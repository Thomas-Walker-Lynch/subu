# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-
"""
subu_core.py — main worker layer for Subu management
Version 0.1.6
"""

import os, sqlite3, subprocess
from pathlib import Path
from contextlib import closing
from subu_worker_bpf import install_steering, remove_steering, BpfError

DB_FILE = Path("./subu.db")

# ---------------------------------------------------------------------
# SQLite helpers
# ---------------------------------------------------------------------

def db_connect():
  if not DB_FILE.exists():
    raise FileNotFoundError("subu.db not found; run `subu init <token>` first")
  return sqlite3.connect(DB_FILE)

def db_init():
  if DB_FILE.exists():
    raise FileExistsError("Database already exists")
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
  print("✅ subu.db created")

# ---------------------------------------------------------------------
# System helpers
# ---------------------------------------------------------------------

def run(cmd, check=True):
  r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
  if check and r.returncode != 0:
    raise RuntimeError(f"cmd failed: {' '.join(cmd)}\n{r.stderr}")
  return r.stdout.strip()

def create_netns(nsname: str):
  run(["ip", "netns", "add", nsname])
  run(["ip", "-n", nsname, "link", "set", "lo", "down"])
  return nsname

def delete_netns(nsname: str):
  run(["ip", "netns", "delete", nsname], check=False)

def ifindex_in_netns(nsname: str, ifname: str) -> int:
  out = run(["ip", "-n", nsname, "-o", "link", "show", ifname])
  return int(out.split(":", 1)[0])

# ---------------------------------------------------------------------
# Subu operations
# ---------------------------------------------------------------------

def create_subu(owner: str, name: str) -> str:
  with closing(db_connect()) as db:
    c = db.cursor()
    c.execute("INSERT INTO subu (owner, name, netns) VALUES (?, ?, ?)",
              (owner, name, f"ns-{owner}-{name}"))
    subu_id = c.lastrowid
    db.commit()
  nsname = f"ns-subu_{subu_id}"
  create_netns(nsname)
  print(f"Created subu_{subu_id} ({owner}:{name}) with netns {nsname}")
  return f"subu_{subu_id}"

def list_subu():
  with closing(db_connect()) as db:
    for row in db.execute("SELECT id, owner, name, netns, lo_state, wg_id, network_state FROM subu"):
      print(row)

def info_subu(subu_id: str):
  sid = int(subu_id.split("_")[1])
  with closing(db_connect()) as db:
    for row in db.execute("SELECT * FROM subu WHERE id=?", (sid,)):
      print(row)

def lo_toggle(subu_id: str, state: str):
  sid = int(subu_id.split("_")[1])
  with closing(db_connect()) as db:
    row = db.execute("SELECT netns FROM subu WHERE id=?", (sid,)).fetchone()
    if not row: raise ValueError("subu not found")
    ns = row[0]
    run(["ip", "netns", "exec", ns, "ip", "link", "set", "lo", state])
    db.execute("UPDATE subu SET lo_state=? WHERE id=?", (state, sid))
    db.commit()
  print(f"loopback {state} in {subu_id}")

# ---------------------------------------------------------------------
# WireGuard operations
# ---------------------------------------------------------------------

def wg_global(basecidr: str):
  Path("./WG_GLOBAL").write_text(basecidr.strip() + "\n")
  print(f"Base CIDR set to {basecidr}")

def wg_create(endpoint: str) -> str:
  base = Path("./WG_GLOBAL").read_text().strip() if Path("./WG_GLOBAL").exists() else None
  if not base:
    raise RuntimeError("No WG global base; set with `subu WG global`")
  with closing(db_connect()) as db:
    c = db.cursor()
    # trivial allocator: next /32 by count
    idx = c.execute("SELECT COUNT(*) FROM wg").fetchone()[0]
    octets = base.split(".")
    octets[3] = str(2 + idx)
    local_ip = ".".join(octets) + "/32"
    c.execute("INSERT INTO wg (endpoint, local_ip, allowed_ips) VALUES (?, ?, ?)",
              (endpoint, local_ip, "0.0.0.0/0"))
    wid = c.lastrowid
    db.commit()
  print(f"Created WG_{wid} ({endpoint}) local_ip={local_ip}")
  return f"WG_{wid}"

def wg_set_pubkey(wg_id: str, key: str):
  wid = int(wg_id.split("_")[1])
  with closing(db_connect()) as db:
    db.execute("UPDATE wg SET pubkey=? WHERE id=?", (key, wid))
    db.commit()
  print(f"Public key stored for {wg_id}")

def wg_info(wg_id: str):
  wid = int(wg_id.split("_")[1])
  with closing(db_connect()) as db:
    row = db.execute("SELECT * FROM wg WHERE id=?", (wid,)).fetchone()
    if not row: print("WG not found")
    else: print(row)

# ---------------------------------------------------------------------
# Attach / Detach with eBPF steering
# ---------------------------------------------------------------------

def attach_wg(subu_id: str, wg_id: str):
  sid = int(subu_id.split("_")[1])
  wid = int(wg_id.split("_")[1])
  wg_ifname = f"subu_{wid}"
  netns = f"ns-{subu_id}"

  # Create WG device inside namespace
  run(["ip", "link", "add", wg_ifname, "type", "wireguard"])
  run(["ip", "link", "set", wg_ifname, "netns", netns])
  # Configure MTU + accept_local
  run(["ip", "-n", netns, "link", "set", wg_ifname, "mtu", "1420"])
  run(["ip", "-n", netns, "link", "set", "dev", wg_ifname, "up"])
  print(f"Attached {wg_id} as {wg_ifname} inside {netns}")

  # Install steering
  try:
    install_steering(subu_id, netns, wg_ifname)
    print(f"Installed eBPF steering for {subu_id} via {wg_ifname}")
  except BpfError as e:
    print(f"warning: steering failed: {e}")

  # Update DB linkage
  with closing(db_connect()) as db:
    db.execute("UPDATE subu SET wg_id=? WHERE id=?", (wid, sid))
    db.commit()

def detach_wg(subu_id: str):
  sid = int(subu_id.split("_")[1])
  with closing(db_connect()) as db:
    row = db.execute("SELECT wg_id, netns FROM subu WHERE id=?", (sid,)).fetchone()
    if not row or row[0] is None:
      print("nothing attached")
      return
    wid, ns = row
    wg_ifname = f"subu_{wid}"
    run(["ip", "-n", ns, "link", "del", wg_ifname], check=False)
    db.execute("UPDATE subu SET wg_id=NULL WHERE id=?", (sid,))
    db.commit()
  try:
    remove_steering(subu_id)
    print(f"Removed steering for {subu_id}")
  except BpfError as e:
    print(f"warning: remove steering failed: {e}")

# ---------------------------------------------------------------------
# Network up/down aggregate
# ---------------------------------------------------------------------

def network_toggle(subu_id: str, state: str):
  sid = int(subu_id.split("_")[1])
  with closing(db_connect()) as db:
    row = db.execute("SELECT netns, wg_id FROM subu WHERE id=?", (sid,)).fetchone()
    if not row: raise ValueError("subu not found")
    ns, wid = row
  # bring lo up first if needed
  if state == "up":
    run(["ip", "netns", "exec", ns, "ip", "link", "set", "lo", "up"], check=False)
  # bring attached iface
  if wid:
    ifname = f"subu_{wid}"
    run(["ip", "-n", ns, "link", "set", "dev", ifname, state], check=False)
  with closing(db_connect()) as db:
    db.execute("UPDATE subu SET network_state=? WHERE id=?", (state, sid))
    db.commit()
  print(f"{subu_id}: network {state}")

# ---------------------------------------------------------------------
# Exec inside namespace
# ---------------------------------------------------------------------

def exec_in_subu(subu_id: str, cmd: list):
  sid = int(subu_id.split("_")[1])
  with closing(db_connect()) as db:
    ns = db.execute("SELECT netns FROM subu WHERE id=?", (sid,)).fetchone()[0]
  full = ["ip", "netns", "exec", ns] + cmd
  os.execvp(full[0], full)
