#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-
"""
dispatch.py

Role: provide one function for each CLI verb so that:

  * CLI.py can call these functions
  * Other Python code can also import and call them directly

Each function should return an integer status code where practical.

Implementation note:

  At this stage of the refactor, the functions are stubs. They define the
  public interface and may raise NotImplementedError. As the domain modules
  under domain/ are completed (subu.py, wg.py, network.py, options.py,
  exec.py), these functions should be updated to call into those modules.
"""
# dispatch.py
from domain import subu as subu_domain

def init(token=None):
  """
  Initialize ./subu.db using schema.sql
  token is currently unused but kept for CLI compatibility.
  """
  # open_db + ensure_schema via domain layer convenience
  from infrastructure.db import open_db, ensure_schema
  conn = open_db("subu.db")
  try:
    ensure_schema(conn)
    return 0
  finally:
    conn.close()


def subu_make(owner, name):
  try:
    s = subu_domain.make_subu(owner, name)
    # print the made ID or username like your older CLI did
    print(f"made subu: id={s.id} username={s.username}")
    return 0
  except Exception as e:
    print(f"error creating subu: {e}", file=sys.stderr)
    return 1


def subu_list():
  try:
    subs = subu_domain.list_subu()
    if not subs:
      print("no subu found")
      return 0
    # simple table
    print("ID  OWNER    NAME    USERNAME    CREATED_AT")
    for s in subs:
      print(f"{s.id}  {s.owner}  {s.name}  {s.username}  {s.made_at}")
    return 0
  except Exception as e:
    print(f"error listing subu: {e}", file=sys.stderr)
    return 1


def subu_info(subu_id):
  """
  Handle: subu info|information <Subu_ID>
  """
  raise NotImplementedError("subu_info is not yet implemented")


def lo_toggle(subu_id, state):
  """
  Handle: subu lo up|down <Subu_ID>
  """
  raise NotImplementedError("lo_toggle is not yet implemented")


def wg_global(base_cidr):
  """
  Handle: subu WG global <BaseCIDR>
  """
  raise NotImplementedError("wg_global is not yet implemented")


def wg_make(endpoint):
  """
  Handle: subu WG make <host:port>
  """
  raise NotImplementedError("wg_make is not yet implemented")


def wg_server_public_key(wg_id, key):
  """
  Handle: subu WG server_provided_public_key <WG_ID> <Base64Key>
  """
  raise NotImplementedError("wg_server_public_key is not yet implemented")


def wg_info(wg_id):
  """
  Handle: subu WG info|information <WG_ID>
  """
  raise NotImplementedError("wg_info is not yet implemented")


def wg_up(wg_id):
  """
  Handle: subu WG up <WG_ID>
  """
  raise NotImplementedError("wg_up is not yet implemented")


def wg_down(wg_id):
  """
  Handle: subu WG down <WG_ID>
  """
  raise NotImplementedError("wg_down is not yet implemented")


def attach_wg(subu_id, wg_id):
  """
  Handle: subu attach WG <Subu_ID> <WG_ID>
  """
  raise NotImplementedError("attach_wg is not yet implemented")


def detach_wg(subu_id):
  """
  Handle: subu detach WG <Subu_ID>
  """
  raise NotImplementedError("detach_wg is not yet implemented")


def network_toggle(subu_id, state):
  """
  Handle: subu network up|down <Subu_ID>
  """
  raise NotImplementedError("network_toggle is not yet implemented")


def option_set(subu_id, name, value):
  """
  Handle: subu option set <Subu_ID> <name> <value>
  """
  raise NotImplementedError("option_set is not yet implemented")


def option_get(subu_id, name):
  """
  Handle: subu option get <Subu_ID> <name>
  """
  raise NotImplementedError("option_get is not yet implemented")


def option_list(subu_id):
  """
  Handle: subu option list <Subu_ID>
  """
  raise NotImplementedError("option_list is not yet implemented")


def exec(subu_id, cmd_argv):
  """
  Handle: subu exec <Subu_ID> -- <cmd> ...
  """
  raise NotImplementedError("exec is not yet implemented")
