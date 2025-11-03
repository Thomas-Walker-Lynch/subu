# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-
"""
core.py — worker API for subu manager
Version: 0.2.0
"""
import os, sqlite3, subprocess
from pathlib import Path
from contextlib import closing
from text import VERSION
from worker_bpf import ensure_mounts, install_steering, remove_steering, BpfError

DB_FILE = Path("./subu.db")
WG_GLOBAL_FILE = Path("./WG_GLOBAL")

def run(cmd, check=True):
  r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
  if check and r.returncode != 0:
    raise RuntimeError(f"cmd failed: {' '.join(cmd)}\n{r.stderr}")
  return r.stdout.strip()

# ---------------- DB ----------------
def _db():
  if not DB_FILE.exists():
    raise FileNotFoundError("subu.db not found; run `subu init <token>` first")
  return sqlite3.connect(DB_FILE)

def cmd_init(token: str|None):
  if DB_FILE.exists():
    raise FileExistsError("db already exists")
  if not token or len(token) < 6:
    raise ValueError("init requires a 6+ char token")
  with closing(sqlite3.connect(DB_FILE)) as db:
    c = db.cursor()
    c.executescript("""
      CREATE TABLE subu (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        owner TEXT,
        name TEXT,
        netns TEXT,
        lo_state TEXT DEFAULT 'down',
        wg_id INTEGER,
        network_state TEXT DEFAULT 'down'
      );
      CREATE TABLE wg (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        endpoint TEXT,
        local_ip TEXT,
        allowed_ips TEXT,
        pubkey TEXT,
        state TEXT DEFAULT 'down'
      );
      CREATE TABLE options (
        subu_id INTEGER,
        name TEXT,
        value TEXT,
        PRIMARY KEY (subu_id, name)
      );
    """)
    db.commit()
  print(f"created subu.db (v{VERSION})")

# ------------- Subu ops -------------
def create_subu(owner: str, name: str) -> str:
  with closing(_db()) as db:
    c = db.cursor()
    subu_netns = f"ns-subu_tmp"  # temp; we rename after ID known
    c.execute("INSERT INTO subu (owner, name, netns) VALUES (?, ?, ?)",
              (owner, name, subu_netns))
    sid = c.lastrowid
    netns = f"ns-subu_{sid}"
    c.execute("UPDATE subu SET netns=? WHERE id=?", (netns, sid))
    db.commit()

  # create netns
  run(["ip", "netns", "add", netns])
  run(["ip", "-n", netns, "link", "set", "lo", "down"])
  print(f"Created subu_{sid} ({owner}:{name}) with netns {netns}")
  return f"subu_{sid}"

def list_subu():
  with closing(_db()) as db:
    for row in db.execute("SELECT id, owner, name, netns, lo_state, wg_id, network_state FROM subu"):
      print(row)

def info_subu(subu_id: str):
  sid = int(subu_id.split("_")[1])
  with closing(_db()) as db:
    row = db.execute("SELECT * FROM subu WHERE id=?", (sid,)).fetchone()
    if not row:
      print("not found"); return
    print(row)
    wg = db.execute("SELECT wg_id FROM subu WHERE id=?", (sid,)).fetchone()[0]
    if wg is not None:
      wrow = db.execute("SELECT * FROM wg WHERE id=?", (wg,)).fetchone()
      print("WG:", wrow)
    opts = db.execute("SELECT name,value FROM options WHERE subu_id=?", (sid,)).fetchall()
    print("Options:", opts)

def lo_toggle(subu_id: str, state: str):
  sid = int(subu_id.split("_")[1])
  with closing(_db()) as db:
    ns = db.execute("SELECT netns FROM subu WHERE id=?", (sid,)).fetchone()
    if not ns: raise ValueError("subu not found")
    ns = ns[0]
    run(["ip", "netns", "exec", ns, "ip", "link", "set", "lo", state])
    db.execute("UPDATE subu SET lo_state=? WHERE id=?", (state, sid))
    db.commit()
  print(f"{subu_id}: lo {state}")

# ------------- WG ops ---------------
def wg_global(basecidr: str):
  WG_GLOBAL_FILE.write_text(basecidr.strip()+"\n")
  print(f"WG pool base = {basecidr}")

def _alloc_ip(idx: int, base: str) -> str:
  # simplistic /24 allocator: base must be x.y.z.0/24
  prefix = base.split("/")[0].rsplit(".", 1)[0]
  host = 2 + idx
  return f"{prefix}.{host}/32"

def wg_create(endpoint: str) -> str:
  if not WG_GLOBAL_FILE.exists():
    raise RuntimeError("set WG base with `subu WG global <CIDR>` first")
  base = WG_GLOBAL_FILE.read_text().strip()
  with closing(_db()) as db:
    c = db.cursor()
    idx = c.execute("SELECT COUNT(*) FROM wg").fetchone()[0]
    local_ip = _alloc_ip(idx, base)
    c.execute("INSERT INTO wg (endpoint, local_ip, allowed_ips) VALUES (?, ?, ?)",
              (endpoint, local_ip, "0.0.0.0/0"))
    wid = c.lastrowid
    db.commit()
  print(f"WG_{wid} endpoint={endpoint} ip={local_ip}")
  return f"WG_{wid}"

def wg_set_pubkey(wg_id: str, key: str):
  wid = int(wg_id.split("_")[1])
  with closing(_db()) as db:
    db.execute("UPDATE wg SET pubkey=? WHERE id=?", (key, wid))
    db.commit()
  print("ok")

def wg_info(wg_id: str):
  wid = int(wg_id.split("_")[1])
  with closing(_db()) as db:
    row = db.execute("SELECT * FROM wg WHERE id=?", (wid,)).fetchone()
    print(row if row else "not found")

def wg_up(wg_id: str):
  wid = int(wg_id.split("_")[1])
  # Admin-up of WG device handled via network_toggle once attached.
  print(f"{wg_id}: up (noop until attached)")

def wg_down(wg_id: str):
  wid = int(wg_id.split("_")[1])
  print(f"{wg_id}: down (noop until attached)")

# ---------- attach/detach + BPF ----------
def attach_wg(subu_id: str, wg_id: str):
  ensure_mounts()
  sid = int(subu_id.split("_")[1]); wid = int(wg_id.split("_")[1])
  with closing(_db()) as db:
    r = db.execute("SELECT netns FROM subu WHERE id=?", (sid,)).fetchone()
    if not r: raise ValueError("subu not found")
    ns = r[0]
    w = db.execute("SELECT endpoint, local_ip, pubkey FROM wg WHERE id=?", (wid,)).fetchone()
    if not w: raise ValueError("WG not found")
    endpoint, local_ip, pubkey = w

  ifname = f"subu_{wid}"
  # create WG link in init ns, move to netns
  run(["ip", "link", "add", ifname, "type", "wireguard"])
  run(["ip", "link", "set", ifname, "netns", ns])
  run(["ip", "-n", ns, "addr", "add", local_ip, "dev", ifname], check=False)
  run(["ip", "-n", ns, "link", "set", "dev", ifname, "mtu", "1420"])
  run(["ip", "-n", ns, "link", "set", "dev", ifname, "down"])  # keep engine down until `network up`

  # install steering (MVP: create cgroup + attach bpf program)
  try:
    install_steering(subu_id, ns, ifname)
    print(f"{subu_id}: eBPF steering installed -> {ifname}")
  except BpfError as e:
    print(f"{subu_id}: steering warning: {e}")

  with closing(_db()) as db:
    db.execute("UPDATE subu SET wg_id=? WHERE id=?", (wid, sid))
    db.commit()
  print(f"attached {wg_id} to {subu_id} in {ns} as {ifname}")

def detach_wg(subu_id: str):
  ensure_mounts()
  sid = int(subu_id.split("_")[1])
  with closing(_db()) as db:
    r = db.execute("SELECT netns,wg_id FROM subu WHERE id=?", (sid,)).fetchone()
    if not r: print("not found"); return
    ns, wid = r
    if wid is None:
      print("nothing attached"); return
  ifname = f"subu_{wid}"
  run(["ip", "-n", ns, "link", "del", ifname], check=False)
  try:
    remove_steering(subu_id)
  except BpfError as e:
    print(f"steering remove warn: {e}")
  with closing(_db()) as db:
    db.execute("UPDATE subu SET wg_id=NULL WHERE id=?", (sid,))
    db.commit()
  print(f"detached WG_{wid} from {subu_id}")

# ------------- network up/down -------------
def network_toggle(subu_id: str, state: str):
  sid = int(subu_id.split("_")[1])
  with closing(_db()) as db:
    ns, wid = db.execute("SELECT netns,wg_id FROM subu WHERE id=?", (sid,)).fetchone()
  # always make sure lo up on 'up'
  if state == "up":
    run(["ip", "netns", "exec", ns, "ip", "link", "set", "lo", "up"], check=False)
  if wid is not None:
    ifname = f"subu_{wid}"
    run(["ip", "-n", ns, "link", "set", "dev", ifname, state], check=False)
  with closing(_db()) as db:
    db.execute("UPDATE subu SET network_state=? WHERE id=?", (state, sid))
    db.commit()
  print(f"{subu_id}: network {state}")

# ------------- options ----------------
def option_set(subu_id: str, name: str, value: str):
  sid = int(subu_id.split("_")[1])
  with closing(_db()) as db:
    db.execute("INSERT INTO options (subu_id,name,value) VALUES(?,?,?) "
               "ON CONFLICT(subu_id,name) DO UPDATE SET value=excluded.value",
               (sid, name, value))
    db.commit()
  print("ok")

def option_get(subu_id: str, name: str):
  sid = int(subu_id.split("_")[1])
  with closing(_db()) as db:
    row = db.execute("SELECT value FROM options WHERE subu_id=? AND name=?", (sid,name)).fetchone()
  print(row[0] if row else "")

def option_list(subu_id: str):
  sid = int(subu_id.split("_")[1])
  with closing(_db()) as db:
    rows = db.execute("SELECT name,value FROM options WHERE subu_id=?", (sid,)).fetchall()
  for n,v in rows:
    print(f"{n}={v}")

# ------------- exec -------------------
def exec_in_subu(subu_id: str, cmd: list):
  sid = int(subu_id.split("_")[1])
  with closing(_db()) as db:
    ns = db.execute("SELECT netns FROM subu WHERE id=?", (sid,)).fetchone()[0]
  os.execvp("ip", ["ip","netns","exec", ns] + cmd)
