#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-
"""
CLI.py — thin command-line harness
Version: 0.2.0
"""
import sys, argparse
from text import USAGE, HELP, EXAMPLE, VERSION
import core

def CLI(argv=None) -> int:
  argv = argv or sys.argv[1:]
  if not argv:
    print(USAGE)
    return 0

  # simple verbs that bypass argparse (so `help/version/example` always work)
  simple = {"help": HELP, "--help": HELP, "-h": HELP, "usage": USAGE, "example": EXAMPLE, "version": VERSION}
  if argv[0] in simple:
    out = simple[argv[0]]
    print(out if isinstance(out, str) else out())
    return 0

  p = argparse.ArgumentParser(prog="subu", add_help=False)
  p.add_argument("-V", "--Version", action="store_true", help="print version")
  sub = p.add_subparsers(dest="verb")

  # init
  ap = sub.add_parser("init")
  ap.add_argument("token", nargs="?")

  # create/list/info
  ap = sub.add_parser("create")
  ap.add_argument("owner")
  ap.add_argument("name")

  sub.add_parser("list")
  ap = sub.add_parser("info"); ap.add_argument("subu_id")
  ap = sub.add_parser("information"); ap.add_argument("subu_id")

  # lo
  ap = sub.add_parser("lo")
  ap.add_argument("state", choices=["up","down"])
  ap.add_argument("subu_id")

  # WG
  ap = sub.add_parser("WG")
  ap.add_argument("verb", choices=["global","create","server_provided_public_key","info","information","up","down"])
  ap.add_argument("arg1", nargs="?")
  ap.add_argument("arg2", nargs="?")

  # attach/detach
  ap = sub.add_parser("attach")
  ap.add_argument("what", choices=["WG"])
  ap.add_argument("subu_id")
  ap.add_argument("wg_id")

  ap = sub.add_parser("detach")
  ap.add_argument("what", choices=["WG"])
  ap.add_argument("subu_id")

  # network
  ap = sub.add_parser("network")
  ap.add_argument("state", choices=["up","down"])
  ap.add_argument("subu_id")

  # option
  ap = sub.add_parser("option")
  ap.add_argument("verb", choices=["set","get","list"])
  ap.add_argument("subu_id")
  ap.add_argument("name", nargs="?")
  ap.add_argument("value", nargs="?")

  # exec
  ap = sub.add_parser("exec")
  ap.add_argument("subu_id")
  ap.add_argument("--", dest="cmd", nargs=argparse.REMAINDER, default=[])

  ns = p.parse_args(argv)
  if ns.Version:
    print(VERSION); return 0

  try:
    if ns.verb == "init":
      return core.cmd_init(ns.token)

    if ns.verb == "create":
      core.create_subu(ns.owner, ns.name); return 0
    if ns.verb == "list":
      core.list_subu(); return 0
    if ns.verb in ("info","information"):
      core.info_subu(ns.subu_id); return 0

    if ns.verb == "lo":
      core.lo_toggle(ns.subu_id, ns.state); return 0

    if ns.verb == "WG":
      v = ns.verb
      if ns.arg1 is None and v in ("info","information"):
        print("WG info requires WG_ID"); return 2
      if v == "global":
        core.wg_global(ns.arg1); return 0
      if v == "create":
        wid = core.wg_create(ns.arg1); print(wid); return 0
      if v == "server_provided_public_key":
        core.wg_set_pubkey(ns.arg1, ns.arg2); return 0
      if v in ("info","information"):
        core.wg_info(ns.arg1); return 0
      if v == "up":
        core.wg_up(ns.arg1); return 0
      if v == "down":
        core.wg_down(ns.arg1); return 0

    if ns.verb == "attach":
      if ns.what == "WG":
        core.attach_wg(ns.subu_id, ns.wg_id); return 0

    if ns.verb == "detach":
      if ns.what == "WG":
        core.detach_wg(ns.subu_id); return 0

    if ns.verb == "network":
      core.network_toggle(ns.subu_id, ns.state); return 0

    if ns.verb == "option":
      if ns.verb == "option" and ns.name is None and ns.value is None and ns.verb == "list":
        core.option_list(ns.subu_id); return 0
      if ns.verb == "set":
        core.option_set(ns.subu_id, ns.name, ns.value); return 0
      if ns.verb == "get":
        core.option_get(ns.subu_id, ns.name); return 0
      if ns.verb == "list":
        core.option_list(ns.subu_id); return 0

    if ns.verb == "exec":
      if not ns.cmd:
        print("subu exec <Subu_ID> -- <cmd> ..."); return 2
      core.exec_in_subu(ns.subu_id, ns.cmd); return 0

    print(USAGE); return 2
  except Exception as e:
    print(f"error: {e}")
    return 1

if __name__ == "__main__":
  sys.exit(CLI())
