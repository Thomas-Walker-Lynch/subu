# ===== File: subu_net.py =====
#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-

import subu_utils as U
import subu_db as DB


def ensure_netns(ns):
  # create netns if missing
  rc, out = U.run("ip netns list", capture=True, check=False)
  if ns not in out.split():
    U.run(f"ip netns add {ns}")


def lo(ns, action: str):
  if action == "up":
    U.run(f"ip -n {ns} link set lo up")
  else:
    U.run(f"ip -n {ns} link set lo down")


def cmd_lo(sid, action) -> int:
  row = DB.subu_by_id(sid)
  if not row:
    return U.err("unknown subu id")
  ns = row[4] or sid
  ensure_netns(ns)
  lo(ns, action)
  return 0


def cmd_network_up(sid) -> int:
  row = DB.subu_by_id(sid)
  if not row:
    return U.err("unknown subu id")
  ns = row[4] or sid
  ensure_netns(ns)
  lo(ns, "up")
  # bring all attached WG up
  sid_int = int(sid.split("_")[1])
  for wid in DB.attached_wg_ids(sid_int):
    import subu_wg as WG
    WG.cmd_wg_up(wid)
  return U.ok(f"network up for {sid}")


def cmd_network_down(sid) -> int:
  row = DB.subu_by_id(sid)
  if not row:
    return U.err("unknown subu id")
  ns = row[4] or sid
  # bring attached WG down first
  sid_int = int(sid.split("_")[1])
  for wid in DB.attached_wg_ids(sid_int):
    import subu_wg as WG
    WG.cmd_wg_down(wid)
  # leave lo state alone per spec (no warning here)
  return U.ok(f"network down for {sid}")

