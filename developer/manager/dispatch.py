# dispatch.py
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-

import os, sys
import env
from domain import subu as subu_domain
from infrastructure.db import open_db, ensure_schema
from infrastructure.options_store import set_option


def init():
  """
  Handle: subu init <TOKEN>

  For now, TOKEN is unused. The behavior is:

    * if subu.db exists, refuse to overwrite it
    * if it does not exist, make it and apply schema.sql
  """
  db_path = env.db_path()
  if os.path.exists(db_path):
    print("subu.db already exists; refusing to overwrite", file =sys.stderr)
    return 1

  conn = open_db(db_path)
  try:
    ensure_schema(conn)
  finally:
    conn.close()
  return 0

def subu_make(path_tokens: list[str]) -> int:
  """
  Handle: subu make <masu> <subu> [<subu> ...]

  path_tokens is:
    [masu, subu, subu, ...]

  Example:
    subu make Thomas S0
    subu make Thomas S0 S1
  """
  if not path_tokens or len(path_tokens) < 2:
    print(
      "subu: make requires at least <masu> and one <subu> component",
      file =sys.stderr,
    )
    return 2

  masu = path_tokens[0]
  subu_path = path_tokens[1:]

  try:
    username = subu_domain.make_subu(masu, subu_path)
    print(f"made subu unix user '{username}'")
    return 0
  except SystemExit as e:
    # domain layer uses SystemExit for user-facing validation errors
    print(str(e), file =sys.stderr)
    return 2
  except Exception as e:
    print(f"error making subu: {e}", file =sys.stderr)
    return 1

def subu_remove(path_tokens: list[str]) -> int:
  """
  Handle: subu remove <masu> <subu> [<subu> ...]

  path_tokens is:
    [masu, subu, subu, ...]
  """
  if not path_tokens or len(path_tokens) < 2:
    print(
      "subu: remove requires at least <masu> and one <subu> component",
      file =sys.stderr,
    )
    return 2

  masu = path_tokens[0]
  subu_path = path_tokens[1:]

  try:
    username = subu_domain.remove_subu(masu, subu_path)
    print(f"removed subu unix user '{username}'")
    return 0
  except SystemExit as e:
    print(str(e), file =sys.stderr)
    return 2
  except Exception as e:
    print(f"error removing subu: {e}", file =sys.stderr)
    return 1

def option_unix(mode: str) -> int:
  """
  Handle: subu option Unix dry|run

  Example:
    subu option Unix dry
    subu option Unix run
  """
  if mode not in ("dry","run"):
    print(f"unknown Unix mode '{mode}', expected 'dry' or 'run'", file =sys.stderr)
    return 2
  set_option("Unix.mode", mode)
  print(f"Unix mode set to {mode}")
  return 0


# The remaining commands can stay as stubs for now.
# They are left so the CLI imports succeed, but will raise if used.

def subu_list():
  raise NotImplementedError("subu_list is not yet made")


def subu_info(subu_id):
  raise NotImplementedError("subu_info is not yet made")


def lo_toggle(subu_id, state):
  raise NotImplementedError("lo_toggle is not yet made")


def wg_global(base_cidr):
  raise NotImplementedError("wg_global is not yet made")


def wg_make(endpoint):
  raise NotImplementedError("wg_make is not yet made")


def wg_server_public_key(wg_id, key):
  raise NotImplementedError("wg_server_public_key is not yet made")


def wg_info(wg_id):
  raise NotImplementedError("wg_info is not yet made")


def wg_up(wg_id):
  raise NotImplementedError("wg_up is not yet made")


def wg_down(wg_id):
  raise NotImplementedError("wg_down is not yet made")


def attach_wg(subu_id, wg_id):
  raise NotImplementedError("attach_wg is not yet made")


def detach_wg(subu_id):
  raise NotImplementedError("detach_wg is not yet made")


def network_toggle(subu_id, state):
  raise NotImplementedError("network_toggle is not yet made")


def option_set(subu_id, name, value):
  raise NotImplementedError("option_set is not yet made")


def option_get(subu_id, name):
  raise NotImplementedError("option_get is not yet made")


def option_list(subu_id):
  raise NotImplementedError("option_list is not yet made")


def exec(subu_id, cmd_argv):
  raise NotImplementedError("exec is not yet made")
