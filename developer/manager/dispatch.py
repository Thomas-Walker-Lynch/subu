# dispatch.py
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-

import os, sys, sqlite3
import env
from domain import subu as subu_domain
from infrastructure.db import open_db, ensure_schema
from infrastructure.options_store import set_option


def _require_root(action: str) -> bool:
  """Return True if running as root, else print error and return False."""
  try:
    euid = os.geteuid()
  except AttributeError:
    # Non-POSIX; be permissive.
    return True
  if euid != 0:
    print(f"{action}: must be run as root", file=sys.stderr)
    return False
  return True


def _db_path() -> str:
  return env.db_path()


def _open_existing_db() -> sqlite3.Connection | None:
  """Open the existing manager DB or print an error and return None.

  This does *not* create the DB; callers should ensure that
  'db load schema' has been run first.
  """
  path = _db_path()
  if not os.path.exists(path):
    print(
      f"subu: database does not exist at '{path}'.\n"
      f"       Run 'db load schema' as root first.",
      file=sys.stderr,
    )
    return None
  try:
    conn = open_db(path)
  except Exception as e:
    print(f"subu: unable to open database '{path}': {e}", file=sys.stderr)
    return None

  # Use row objects so we can access columns by name.
  conn.row_factory = sqlite3.Row
  return conn


def db_load_schema() -> int:
  """Handle: CLI.py db load schema

  Ensure the DB directory exists, open the DB, and apply schema.sql.
  """
  if not _require_root("db load schema"):
    return 1

  path = _db_path()
  db_dir = os.path.dirname(path) or "."

  try:
    os.makedirs(db_dir, mode=0o750, exist_ok=True)
  except PermissionError as e:
    print(f"subu: cannot create db directory '{db_dir}': {e}", file=sys.stderr)
    return 1

  try:
    conn = open_db(path)
  except Exception as e:
    print(f"subu: unable to open database '{path}': {e}", file=sys.stderr)
    return 1

  try:
    ensure_schema(conn)
  finally:
    conn.close()

  print(f"subu: schema loaded into {path}")
  return 0


def subu_make(path_tokens: list[str]) -> int:
  """Handle: CLI.py subu make <masu> <subu> [<subu> ...]

  path_tokens is:
    [masu, subu, subu, ...]

  Example:
    CLI.py subu make Thomas developer
    CLI.py subu make Thomas developer bolt
  """
  if not path_tokens or len(path_tokens) < 2:
    print(
      "subu: make requires at least <masu> and one <subu> component",
      file=sys.stderr,
    )
    return 2

  if not _require_root("subu make"):
    return 1

  masu = path_tokens[0]
  subu_path = path_tokens[1:]

  # 1) Create Unix user + groups.
  try:
    username = subu_domain.make_subu(masu, subu_path)
  except SystemExit as e:
    # Domain layer uses SystemExit for validation errors.
    print(f"subu: {e}", file=sys.stderr)
    return 2
  except Exception as e:
    print(f"subu: error creating Unix user for {path_tokens}: {e}", file=sys.stderr)
    return 1

  # 2) Record in SQLite.
  conn = _open_existing_db()
  if conn is None:
    # Unix side succeeded but DB is missing; report and stop.
    return 1

  owner = masu
  leaf_name = subu_path[-1]
  full_unix_name = username
  path_str = " ".join([masu] + subu_path)
  netns_name = full_unix_name  # simple deterministic choice for now

  from datetime import datetime, timezone

  now = datetime.now(timezone.utc).isoformat()

  try:
    cur = conn.execute(
      """INSERT INTO subu
            (owner, name, full_unix_name, path, netns_name, wg_id, created_at, updated_at)
            VALUES (?, ?, ?, ?, ?, NULL, ?, ?)""",
      (owner, leaf_name, full_unix_name, path_str, netns_name, now, now),
    )
    conn.commit()
    subu_id = cur.lastrowid
  except sqlite3.IntegrityError as e:
    print(f"subu: database already has an entry for '{full_unix_name}': {e}", file=sys.stderr)
    conn.close()
    return 1
  except Exception as e:
    print(f"subu: error recording subu in database: {e}", file=sys.stderr)
    conn.close()
    return 1

  conn.close()

  print(f"subu_{subu_id}")
  return 0


def _resolve_subu(conn: sqlite3.Connection, target: str, rest: list[str]) -> sqlite3.Row | None:
  """Resolve a subu either by ID (subu_7) or by path.

  ID form:
    target = 'subu_7', rest = []

  Path form:
    target = masu, rest = [subu, subu, ...]
  """
  # ID form: subu_7
  if target.startswith("subu_") and not rest:
    try:
      subu_numeric_id = int(target.split("_", 1)[1])
    except ValueError:
      print(f"subu: invalid Subu_ID '{target}'", file=sys.stderr)
      return None

    row = conn.execute("SELECT * FROM subu WHERE id = ?", (subu_numeric_id,)).fetchone()
    if row is None:
      print(f"subu: no such subu with id {subu_numeric_id}", file=sys.stderr)
    return row

  # Path form
  path_tokens = [target] + list(rest)
  if len(path_tokens) < 2:
    print(
      "subu: path form requires at least <masu> and one <subu> component",
      file=sys.stderr,
    )
    return None

  owner = path_tokens[0]
  path_str = " ".join(path_tokens)

  row = conn.execute(
    "SELECT * FROM subu WHERE owner = ? AND path = ?",
    (owner, path_str),
  ).fetchone()

  if row is None:
    print(f"subu: no such subu with owner='{owner}' and path='{path_str}'", file=sys.stderr)
  return row


def subu_list() -> int:
  """Handle: CLI.py subu list"""
  conn = _open_existing_db()
  if conn is None:
    return 1

  cur = conn.execute(
    "SELECT id, owner, path, full_unix_name, netns_name, wg_id FROM subu ORDER BY id"
  )

  rows = cur.fetchall()
  conn.close()

  if not rows:
    print("(no subu in database)")
    return 0

  for row in rows:
    subu_id = row[0]
    owner = row[1]
    path = row[2]
    full_unix_name = row[3]
    netns_name = row[4]
    wg_id = row[5]
    wg_display = "-" if wg_id is None else f"WG_{wg_id}"
    print(f"subu_{subu_id}\t{owner}\t{path}\t{full_unix_name}\t{netns_name}\t{wg_display}")

  return 0


def subu_info(target: str, rest: list[str]) -> int:
  """Handle: CLI.py subu info <Subu_ID>|<masu> <subu> [<subu> ...]

  Examples:
    CLI.py subu info subu_3
    CLI.py subu info Thomas developer bolt
  """
  conn = _open_existing_db()
  if conn is None:
    return 1

  row = _resolve_subu(conn, target, rest)
  if row is None:
    conn.close()
    return 1

  subu_id = row["id"]
  owner = row["owner"]
  name = row["name"]
  full_unix_name = row["full_unix_name"]
  path = row["path"]
  netns_name = row["netns_name"]
  wg_id = row["wg_id"]
  created_at = row["created_at"]
  updated_at = row["updated_at"]

  conn.close()

  print(f"Subu_ID:    subu_{subu_id}")
  print(f"Owner:      {owner}")
  print(f"Name:       {name}")
  print(f"Path:       {path}")
  print(f"Unix user:  {full_unix_name}")
  print(f"Netns:      {netns_name}")
  print(f"WG_ID:      {wg_id if wg_id is not None else '-'}")
  print(f"Created:    {created_at}")
  print(f"Updated:    {updated_at}")
  return 0


def subu_remove(target: str, rest: list[str]) -> int:
  """Handle: CLI.py subu remove <Subu_ID>|<masu> <subu> [<subu> ...]

  This removes both:
    - the Unix user/group associated with the subu, and
    - the corresponding row from the database.
  """
  if not _require_root("subu remove"):
    return 1

  conn = _open_existing_db()
  if conn is None:
    return 1

  row = _resolve_subu(conn, target, rest)
  if row is None:
    conn.close()
    return 1

  subu_id = row["id"]
  owner = row["owner"]
  path_str = row["path"]
  path_tokens = path_str.split(" ")
  if not path_tokens or len(path_tokens) < 2:
    print(f"subu: stored path is invalid for id {subu_id}: '{path_str}'", file=sys.stderr)
    conn.close()
    return 1

  masu = path_tokens[0]
  subu_path = path_tokens[1:]

  # 1) Remove Unix user + group.
  try:
    username = subu_domain.remove_subu(masu, subu_path)
  except SystemExit as e:
    print(f"subu: {e}", file=sys.stderr)
    conn.close()
    return 2
  except Exception as e:
    print(f"subu: error removing Unix user for id subu_{subu_id}: {e}", file=sys.stderr)
    conn.close()
    return 1

  # 2) Remove from DB.
  try:
    conn.execute("DELETE FROM subu WHERE id = ?", (subu_id,))
    conn.commit()
  except Exception as e:
    print(f"subu: error removing database row for id subu_{subu_id}: {e}", file=sys.stderr)
    conn.close()
    return 1

  conn.close()

  print(f"removed subu_{subu_id} {username}")
  return 0


# Placeholder stubs for existing option / WG / network / exec wiring.
# These keep the module importable while we focus on subu + db.

def wg_global(arg1: str | None) -> int:
  print("WG global: not yet implemented", file=sys.stderr)
  return 1


def wg_make(arg1: str | None) -> int:
  print("WG make: not yet implemented", file=sys.stderr)
  return 1


def wg_server_public_key(arg1: str | None, arg2: str | None) -> int:
  print("WG server_provided_public_key: not yet implemented", file=sys.stderr)
  return 1


def wg_info(arg1: str | None) -> int:
  print("WG info: not yet implemented", file=sys.stderr)
  return 1


def wg_up(arg1: str | None) -> int:
  print("WG up: not yet implemented", file=sys.stderr)
  return 1


def wg_down(arg1: str | None) -> int:
  print("WG down: not yet implemented", file=sys.stderr)
  return 1


def attach_wg(subu_id: str, wg_id: str) -> int:
  print("attach WG: not yet implemented", file=sys.stderr)
  return 1


def detach_wg(subu_id: str) -> int:
  print("detach WG: not yet implemented", file=sys.stderr)
  return 1


def network_toggle(subu_id: str, state: str) -> int:
  print("network up/down: not yet implemented", file=sys.stderr)
  return 1


def option_unix(mode: str) -> int:
  # example: store a Unix handling mode into options_store
  set_option("Unix.mode", mode)
  print(f"Unix mode set to {mode}")
  return 0


def exec(subu_id: str, cmd_argv: list[str]) -> int:
  print("exec: not yet implemented", file=sys.stderr)
  return 1
