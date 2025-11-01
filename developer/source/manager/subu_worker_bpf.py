# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-
"""
subu_worker_bpf.py — build, load, and manage eBPF steering for a Subu

What it does:
  * Compiles subu_bpf_force_egress.c -> /var/lib/subu/bpf/subu_force_egress.bpf.o
  * Creates pins under /sys/fs/bpf/subu/<Subu_ID>:
        force_connect4, force_sendmsg4, force_ifindex_map
  * Creates a cgroup v2 node at /sys/fs/cgroup/subu/<Subu_ID>
  * Attaches programs (connect4, sendmsg4) to that cgroup
  * Writes the target ifindex into map[0]
  * Idempotent: re-running updates ifindex and ensures attachments

Requirements:
  * bpffs mounted at /sys/fs/bpf
  * cgroup v2 mounted at /sys/fs/cgroup
  * tools: clang, bpftool
  * privileges: CAP_BPF + CAP_SYS_ADMIN
"""

import os
import shutil
import subprocess
from pathlib import Path
from typing import Dict

BPF_SRC = Path("subu_bpf_force_egress.c")
BUILD_DIR = Path("/var/lib/subu/bpf")
BPFFS_DIR = Path("/sys/fs/bpf")
BPF_PIN_BASE = BPFFS_DIR / "subu"            # /sys/fs/bpf/subu/<Subu_ID>/*
CGROOT = Path("/sys/fs/cgroup/subu")         # /sys/fs/cgroup/subu/<Subu_ID>

OBJ_NAME = "subu_force_egress.bpf.o"
PROG_CONNECT_PIN = "force_connect4"
PROG_SENDMSG_PIN = "force_sendmsg4"
MAP_IFINDEX_PIN   = "force_ifindex_map"      # matches map name in C

class BpfError(RuntimeError):
  pass

def _run(cmd, check=True):
  r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
  if check and r.returncode != 0:
    raise BpfError(f"cmd failed: {' '.join(cmd)}\nstdout:\n{r.stdout}\nstderr:\n{r.stderr}")
  return r

def _which_or_die(tool: str):
  if not shutil.which(tool):
    raise BpfError(f"Missing required tool: {tool}")

def ensure_prereqs():
  _which_or_die("clang")
  _which_or_die("bpftool")
  # bpffs?
  if not BPFFS_DIR.exists():
    raise BpfError(f"{BPFFS_DIR} not mounted; try: mount -t bpf bpf {BPFFS_DIR}")
  # cgroup v2?
  cgroot = Path("/sys/fs/cgroup")
  if not (cgroot / "cgroup.controllers").exists():
    raise BpfError("cgroup v2 not mounted; e.g.: mount -t cgroup2 none /sys/fs/cgroup")

def ensure_dirs(subu_id: str) -> Dict[str, Path]:
  BUILD_DIR.mkdir(parents=True, exist_ok=True)
  (BPF_PIN_BASE).mkdir(parents=True, exist_ok=True)
  pin_dir = BPF_PIN_BASE / subu_id
  pin_dir.mkdir(parents=True, exist_ok=True)

  CGROOT.mkdir(parents=True, exist_ok=True)
  cgdir = CGROOT / subu_id
  cgdir.mkdir(parents=True, exist_ok=True)

  return {
    "build_obj": BUILD_DIR / OBJ_NAME,
    "pin_dir": pin_dir,
    "pin_prog_connect": pin_dir / PROG_CONNECT_PIN,
    "pin_prog_sendmsg": pin_dir / PROG_SENDMSG_PIN,
    "pin_map_ifindex": pin_dir / MAP_IFINDEX_PIN,
    "cgdir": cgdir,
  }

def compile_bpf(obj_path: Path):
  if not BPF_SRC.exists():
    raise BpfError(f"BPF source not found: {BPF_SRC}")
  cmd = [
    "clang", "-O2", "-g",
    "-target", "bpf",
    "-D__TARGET_ARCH_x86",
    "-c", str(BPF_SRC),
    "-o", str(obj_path),
  ]
  _run(cmd)

def _load_prog_with_pinmaps(obj: Path, section: str, prog_pin: Path, maps_dir: Path):
  # bpftool prog load OBJ PIN_PATH section <section> pinmaps <maps_dir>
  _run([
    "bpftool", "prog", "load",
    str(obj), str(prog_pin),
    "section", section,
    "pinmaps", str(maps_dir),
  ])

def load_and_pin_all(p: Dict[str, Path]):
  obj = p["build_obj"]
  maps_dir = p["pin_dir"]
  # Load two sections; maps get pinned into maps_dir once (first load).
  _load_prog_with_pinmaps(obj, "cgroup/connect4", p["pin_prog_connect"], maps_dir)
  _load_prog_with_pinmaps(obj, "cgroup/sendmsg4", p["pin_prog_sendmsg"], maps_dir)

  # Ensure map exists where we expect it (pinned by name from C)
  if not p["pin_map_ifindex"].exists():
    # Some bpftool/libbpf combos pin maps directly as <maps_dir>/<map_name>.
    # If not present, try to locate by name and pin.
    # Find map id by name:
    out = _run(["bpftool", "map", "show"]).stdout.splitlines()
    target_id = None
    for line in out:
      # sample: "123: array  name force_ifindex_map  flags 0x0 ..."
      if " name " + MAP_IFINDEX_PIN + " " in line:
        # id is before colon
        try:
          target_id = line.strip().split(":", 1)[0]
          int(target_id)  # validate
          break
        except Exception:
          pass
    if not target_id:
      raise BpfError(f"Unable to find map '{MAP_IFINDEX_PIN}' to pin")
    _run(["bpftool", "map", "pin", "id", target_id, str(p["pin_map_ifindex"])])

def attach_to_cgroup(p: Dict[str, Path]):
  # Attach programs to the subu-specific cgroup
  _run(["bpftool", "cgroup", "attach", str(p["cgdir"]), "connect4", "pinned", str(p["pin_prog_connect"])])
  _run(["bpftool", "cgroup", "attach", str(p["cgdir"]), "sendmsg4", "pinned", str(p["pin_prog_sendmsg"])])

def detach_from_cgroup(p: Dict[str, Path]):
  _run(["bpftool", "cgroup", "detach", str(p["cgdir"]), "connect4"], check=False)
  _run(["bpftool", "cgroup", "detach", str(p["cgdir"]), "sendmsg4"], check=False)

def set_ifindex(p: Dict[str, Path], ifindex: int):
  # bpftool map update pinned <map> key <00 00 00 00> value <ifindex_le>
  key_hex = "00 00 00 00".split()
  val_hex = ifindex.to_bytes(4, "little").hex(" ").split()
  _run(["bpftool", "map", "update", "pinned", str(p["pin_map_ifindex"]), "key", *key_hex, "value", *val_hex])

def _ifindex_in_netns(netns_name: str, ifname: str) -> int:
  # ip -n <ns> -o link show <ifname> -> "7: subu_0: <...>"
  r = _run(["ip", "-n", netns_name, "-o", "link", "show", ifname])
  first = r.stdout.strip().split(":", 1)[0]
  return int(first)

def install_steering(subu_id: str, netns_name: str, wg_ifname: str):
  """
  Build/load eBPF programs, pin them under /sys/fs/bpf/subu/<subu_id>,
  attach to /sys/fs/cgroup/subu/<subu_id>, and set map[0]=ifindex(of wg_ifname in netns).
  Idempotent across repeated calls.
  """
  ensure_prereqs()
  paths = ensure_dirs(subu_id)
  # compile if missing or stale
  if (not paths["build_obj"].exists()) or (paths["build_obj"].stat().st_mtime < BPF_SRC.stat().st_mtime):
    compile_bpf(paths["build_obj"])

  # if pins already exist, keep them and just ensure attached + value updated
  pins_exist = all(paths[k].exists() for k in ("pin_prog_connect", "pin_prog_sendmsg"))
  if not pins_exist:
    load_and_pin_all(paths)

  # compute ifindex inside the subu netns
  ifindex = _ifindex_in_netns(netns_name, wg_ifname)
  set_ifindex(paths, ifindex)

  # ensure cgroup attachments in place
  attach_to_cgroup(paths)

def remove_steering(subu_id: str):
  """
  Detach cgroup hooks and unpin programs/maps. Leaves the cgroup dir.
  """
  ensure_prereqs()
  paths = ensure_dirs(subu_id)
  # detach (ignore failure)
  detach_from_cgroup(paths)
  # unpin objects
  for key in ("pin_prog_connect", "pin_prog_sendmsg", "pin_map_ifindex"):
    try:
      p = paths[key]
      if p.exists():
        p.unlink()
    except Exception:
      pass
  # try to remove empty pin dir
  try:
    paths["pin_dir"].rmdir()
  except Exception:
    pass
