#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-

"""
subu.py — CLI only.
- No-args prints USAGE.
- `help` / `usage` / `example` / `version` are handled *before* argparse.
- `-h` / `--help` are mapped to `help`.
- Delegates real work to subu_core.dispatch(args).
"""

from __future__ import annotations
import argparse
import sys

try:
  from subu_version import VERSION
except Exception:
  VERSION = "0.0.0-unknown"

try:
  from subu_text import USAGE, HELP, EXAMPLE
except Exception:
  USAGE = "usage: subu <verb> [args]\n"
  HELP = "help text unavailable (subu_text import failed)\n"
  EXAMPLE = "example text unavailable (subu_text import failed)\n"

# -------------------------------
# Parser construction (verbs that do real work)
# -------------------------------
def _build_parser() -> argparse.ArgumentParser:
  # add_help=False so -h/--help don't get auto-bound; we intercept them manually
  p = argparse.ArgumentParser(
      prog="subu",
      description="Manage subu containers, namespaces, and WireGuard attachments.",
      add_help=False,
  )
  # keep -V only; -h/--help are handled by pre-parse
  p.add_argument("-V", "--version", action="store_true",
                 help="Print version and exit.")

  sub = p.add_subparsers(dest="verb",
                         metavar="{init,create,info,information,WG,attach,detach,network,lo,option,exec}",
                         required=False)

  sub.add_parser("init", help="Initialize new subu database (refuses if exists).")
  sub.add_parser("create", help="Create a subu (defaults only).")
  sub.add_parser("info", help="Show info about a subu.")
  sub.add_parser("information", help="Alias of 'info'.")
  sub.add_parser("WG", help="WireGuard operations.")
  sub.add_parser("attach", help="Attach WG to subu (netns + cgroup/eBPF).")
  sub.add_parser("detach", help="Detach WG from subu.")
  sub.add_parser("network", help="Bring attached ifaces up/down in the subu netns.")
  sub.add_parser("lo", help="Bring loopback up/down in the subu netns.")
  sub.add_parser("option", help="Persisted options (list/get/set).")
  sub.add_parser("exec", help="Execute a command inside the subu netns: subu exec <id> -- <cmd...>")

  return p

def _print_topic_help(parser: argparse.ArgumentParser, topic: str) -> bool:
  """Try to print help for a specific subparser topic. Returns True if found."""
  for action in getattr(parser, "_subparsers", [])._actions:
    if isinstance(action, argparse._SubParsersAction):
      if topic in action.choices:
        action.choices[topic].print_help()
        return True
      if topic == "information" and "info" in action.choices:
        action.choices["info"].print_help()
        return True
  return False

# -------------------------------
# CLI entry (parse only)
# -------------------------------
def CLI(argv=None) -> int:
  argv = sys.argv[1:] if argv is None else argv
  parser = _build_parser()

  # 0) No args => USAGE
  if not argv:
    sys.stdout.write(USAGE)
    return 0

  # 1) Pre-parse intercepts (robust vs. argparse)
  first = argv[0]
  if first in ("-h", "--help", "help"):
    topic = argv[1] if len(argv) > 1 and argv[0] == "help" else None
    if topic:
      # Topic-aware help if possible; else fall back to full HELP
      if not _print_topic_help(parser, topic):
        sys.stdout.write(HELP)
    else:
      sys.stdout.write(HELP)
    return 0

  if first in ("usage",):
    sys.stdout.write(USAGE)
    return 0

  if first in ("example",):
    sys.stdout.write(EXAMPLE)
    return 0

  if first in ("version",):
    print(VERSION)
    return 0

  # 2) Normal parse
  try:
    args = parser.parse_args(argv)
  except SystemExit as e:
    return int(e.code)

  # 3) Global -V/--version
  if getattr(args, "version", False):
    print(VERSION)
    return 0

  # 4) Delegate to worker layer
  try:
    from subu_core import dispatch  # type: ignore
  except Exception as e:
    sys.stderr.write(f"subu: internal error: cannot import subu_core.dispatch: {e}\n")
    return 1

  try:
    rc = dispatch(args)
    return int(rc) if rc is not None else 0
  except KeyboardInterrupt:
    return 130
  except SystemExit as e:
    return int(e.code)
  except Exception as e:
    sys.stderr.write(f"subu: error: {e}\n")
    return 1

if __name__ == "__main__":
  sys.exit(CLI())
