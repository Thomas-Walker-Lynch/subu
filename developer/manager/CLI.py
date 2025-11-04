#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-
"""
1. CLI.py
 dispatch.

Role: parse argv, choose command, call
CLI should not do any work beyond:

  * figure out program_name (for example, manager/CLI.py or wrapper name)
  * call the right function in dispatch
  * print text from text.py when needed
  * exit with the returned status code
"""

import sys, argparse
from text import make_text
import dispatch


def build_arg_parser(program_name):
  """
  Build the top level argument parser for the subu manager.
  """
  parser = argparse.ArgumentParser(prog=program_name, add_help=False)
  parser.add_argument("-V","--Version", action="store_true", help="print version")

  subparsers = parser.add_subparsers(dest="verb")

  register_subu_commands(subparsers)
  register_wireguard_commands(subparsers)
  register_attach_commands(subparsers)
  register_network_commands(subparsers)
  register_option_commands(subparsers)
  register_exec_commands(subparsers)

  return parser


def register_subu_commands(subparsers):
  """
  Register subu related commands:
    init, make, list, info, information, lo
  """
  # init
  ap = subparsers.add_parser("init")
  ap.add_argument("token", nargs="?")

  # make
  ap = subparsers.add_parser("make")
  ap.add_argument("owner")
  ap.add_argument("name")

  # list
  subparsers.add_parser("list")

  # info / information
  ap = subparsers.add_parser("info")
  ap.add_argument("subu_id")
  ap = subparsers.add_parser("information")
  ap.add_argument("subu_id")

  # lo
  ap = subparsers.add_parser("lo")
  ap.add_argument("state", choices=["up","down"])
  ap.add_argument("subu_id")


def register_wireguard_commands(subparsers):
  """
  Register WireGuard related commands, grouped under 'WG':
    WG global <BaseCIDR>
    WG make <host:port>
    WG server_provided_public_key <WG_ID> <Base64Key>
    WG info|information <WG_ID>
    WG up|down <WG_ID>
  """
  ap = subparsers.add_parser("WG")
  ap.add_argument(
    "wg_verb",
    choices=[
      "global",
      "make",
      "server_provided_public_key",
      "info",
      "information",
      "up",
      "down",
    ],
  )
  ap.add_argument("arg1", nargs="?")
  ap.add_argument("arg2", nargs="?")


def register_attach_commands(subparsers):
  """
  Register attach and detach commands:
    attach WG <Subu_ID> <WG_ID>
    detach WG <Subu_ID>
  """
  ap = subparsers.add_parser("attach")
  ap.add_argument("what", choices=["WG"])
  ap.add_argument("subu_id")
  ap.add_argument("wg_id")

  ap = subparsers.add_parser("detach")
  ap.add_argument("what", choices=["WG"])
  ap.add_argument("subu_id")


def register_network_commands(subparsers):
  """
  Register network aggregate commands:
    network up|down <Subu_ID>
  """
  ap = subparsers.add_parser("network")
  ap.add_argument("state", choices=["up","down"])
  ap.add_argument("subu_id")


def register_option_commands(subparsers):
  """
  Register option commands:
    option set|get|list ...
  """
  ap = subparsers.add_parser("option")
  ap.add_argument("action", choices=["set","get","list"])
  ap.add_argument("subu_id")
  ap.add_argument("name", nargs="?")
  ap.add_argument("value", nargs="?")


def register_exec_commands(subparsers):
  """
  Register exec command:
    exec <Subu_ID> -- <cmd> ...
  """
  ap = subparsers.add_parser("exec")
  ap.add_argument("subu_id")
  # Use a dedicated "--" argument so that:
  #   subu exec subu_7 -- curl -4v https://ifconfig.me
  # works as before.
  ap.add_argument("--", dest="cmd", nargs=argparse.REMAINDER, default=[])


def CLI(argv=None) -> int:
  """
  Top level entry point for the subu manager CLI.
  """
  if argv is None:
    argv = sys.argv[1:]

  # For now we fix the program name to "subu".
  # A release wrapper can later pass a different program name.
  program_name = "subu"
  text = make_text(program_name)

  # No arguments is the same as "help".
  if not argv:
    print(text.help(), end="")
    return 0

  # Simple verbs that bypass argparse so they always work.
  simple = {
    "help": text.help,
    "--help": text.help,
    "-h": text.help,
    "usage": text.usage,
    "example": text.example,
    "version": text.version,
  }
  if argv[0] in simple:
    print(simple[argv[0]](), end="")
    return 0

  parser = build_arg_parser(program_name)
  ns = parser.parse_args(argv)

  if getattr(ns, "Version", False):
    print(text.version(), end="")
    return 0

  try:
    if ns.verb == "init":
      return dispatch.init(ns.token)

    if ns.verb == "make":
      return dispatch.subu_make(ns.owner, ns.name)

    if ns.verb == "list":
      return dispatch.subu_list()

    if ns.verb in ("info","information"):
      return dispatch.subu_info(ns.subu_id)

    if ns.verb == "lo":
      return dispatch.lo_toggle(ns.subu_id, ns.state)

    if ns.verb == "WG":
      v = ns.wg_verb
      if v in ("info","information") and ns.arg1 is None:
        print("WG info requires WG_ID", file=sys.stderr)
        return 2
      if v == "global":
        return dispatch.wg_global(ns.arg1)
      if v == "make":
        return dispatch.wg_make(ns.arg1)
      if v == "server_provided_public_key":
        return dispatch.wg_server_public_key(ns.arg1, ns.arg2)
      if v in ("info","information"):
        return dispatch.wg_info(ns.arg1)
      if v == "up":
        return dispatch.wg_up(ns.arg1)
      if v == "down":
        return dispatch.wg_down(ns.arg1)

    if ns.verb == "attach":
      if ns.what == "WG":
        return dispatch.attach_wg(ns.subu_id, ns.wg_id)

    if ns.verb == "detach":
      if ns.what == "WG":
        return dispatch.detach_wg(ns.subu_id)

    if ns.verb == "network":
      return dispatch.network_toggle(ns.subu_id, ns.state)

    if ns.verb == "option":
      if ns.action == "set":
        return dispatch.option_set(ns.subu_id, ns.name, ns.value)
      if ns.action == "get":
        return dispatch.option_get(ns.subu_id, ns.name)
      if ns.action == "list":
        return dispatch.option_list(ns.subu_id)

    if ns.verb == "exec":
      if not ns.cmd:
        print(f"{program_name} exec <Subu_ID> -- <cmd> ...", file=sys.stderr)
        return 2
      return dispatch.exec(ns.subu_id, ns.cmd)

    # If we reach here, the verb was not recognised.
    print(text.usage(), end="")
    return 2

  except Exception as e:
    print(f"error: {e}", file=sys.stderr)
    return 1


if __name__ == "__main__":
  sys.exit(CLI())
