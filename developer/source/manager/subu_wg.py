# ===== File: subu_wg.py =====
#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-

import ipaddress
import subu_utils as U
import subu_db as DB

PREFIX_DEV = "subu_"  # device name base; dev = f"{PREFIX_DEV}{WG_id_num}"


def cmd_wg_help() -> int:
  print("WG commands: global <cidr> | create <host:port> | info | server_provided_public_key <WG_id> <key> | up <WG_id> | down <WG_id>")
  return 0


def cmd_wg_global(base_cidr: str) -> int:
  # validate CIDR
  try:
    net = ipaddress.ip_network(base_cidr, strict=False)
    if net.version != 4:
      return U.err("only IPv4 supported for WG base")
  except Exception as e:
    return U.err(f"invalid cidr: {e}")
  DB.wg_set_global_base(base_cidr)
  return U.ok(f"WG base set to {base_cidr}")


def allocate_addr_for(wg_id: str) -> str:
  base = DB.get_meta("wg_base_cidr")
  if not base:
    raise RuntimeError("WG base not set; run 'subu WG global <cidr>'")
  net = ipaddress.ip_network(base, strict=False)
  wid = int(wg_id.split("_")[1])
  host = list(net.hosts())[wid + 1]  # skip .1 for potential gateway
  return f"{host}/32"


def ensure_device(wg_id: str):
  # create device if missing and store dev+addr in DB
  row = DB.wg_by_id(wg_id)
  if not row:
    raise RuntimeError("unknown WG id")
  _, remote, pubkey, dev, addr, state = row
  if not dev:
    dev = f"{PREFIX_DEV}{wg_id.split('_')[1]}"
    DB.wg_update(wg_id, dev=dev)
  if not addr:
    addr = allocate_addr_for(wg_id)
    DB.wg_update(wg_id, addr=addr)
  # ensure link exists in root netns
  rc, out = U.run("ip link show", capture=True, check=False)
  if f": {dev}:" not in out:
    # create WG link skeleton; full wg config is deferred
    U.run(f"ip link add {dev} type wireguard")
    U.run(f"ip addr add {addr} dev {dev}")
  return dev, addr


def move_to_netns(wg_id: str, ns: str):
  dev, _ = ensure_device(wg_id)
  # if already in ns, ip will refuse — treat as ok
  U.run(f"ip link set {dev} netns {ns}", check=False)


def detach_device(wg_id: str):
  row = DB.wg_by_id(wg_id)
  if not row: return
  dev = row[3]
  if not dev: return
  # best effort delete (must run either in owning ns or root if present there)
  # try root first
  rc, out = U.run(f"ip link del {dev}", check=False)
  if rc != 0:
    # try to find owning ns? (skipped for brevity)
    pass


def cmd_wg_create(remote: str) -> int:
  wid = DB.wg_create(remote)
  print(wid)
  return 0


def cmd_wg_info() -> int:
  rows = DB.wg_list()
  for wid, remote, pubkey, dev, addr, state in rows:
    print(f"WG_{wid}: remote={remote} dev={dev} addr={addr} state={state} pubkey={'set' if pubkey else 'unset'}")
  return 0


def cmd_wg_set_server_pub(wg_id: str, pub: str) -> int:
  DB.wg_update(wg_id, pubkey=pub)
  return U.ok(f"Set server pubkey for {wg_id}")


def _ns_of_wg(wg_id: str):
  # discover netns from attachment
  rows = DB.list_subu()
  for sid, *_ in rows:
    attached = DB.attached_wg_ids(sid)
    if wg_id in attached:
      row = DB.subu_by_id(f"subu_{sid}")
      return row[4] or f"subu_{sid}"
  return None


def cmd_wg_up(wg_id: str) -> int:
  row = DB.wg_by_id(wg_id)
  if not row:
    return U.err("unknown WG id")
  dev, addr = ensure_device(wg_id)
  ns = _ns_of_wg(wg_id)
  if ns:
    # bring lo up silently before bringing WG up
    U.run(f"ip -n {ns} link set lo up", check=False)
    U.run(f"ip -n {ns} link set {dev} up")
  else:
    U.run(f"ip link set {dev} up")
  DB.wg_update(wg_id, state="up")
  return U.ok(f"WG {wg_id} up")


def cmd_wg_down(wg_id: str) -> int:
  row = DB.wg_by_id(wg_id)
  if not row:
    return U.err("unknown WG id")
  dev = row[3]
  if not dev:
    return U.err("WG device not created yet")
  ns = _ns_of_wg(wg_id)
  if ns:
    U.run(f"ip -n {ns} link set {dev} down", check=False)
  else:
    U.run(f"ip link set {dev} down", check=False)
  DB.wg_update(wg_id, state="down")
  return U.ok(f"WG {wg_id} down")


