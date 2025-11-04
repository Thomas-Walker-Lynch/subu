# infrastructure/db.py
import os
import pwd
import grp
import subprocess
from contextlib import closing
import sqlite3
from pathlib import Path

"""
5.1 infrastructure/db.py

All SQLite access.

5.1.1 open_db(path: str = "subu.db") -> sqlite3.Connection
5.1.2 ensure_schema(conn) -> None
5.1.3 insert_subu(conn, subu: Subu) -> None
5.1.4 fetch_subu(conn, subu_id: str) -> Subu
5.1.5 list_subu(conn) -> list[Subu]
5.1.6 insert_wg(conn, wg: WG) -> None
5.1.7 fetch_wg(conn, wg_id: str) -> WG
5.1.8 update_wg(conn, wg: WG) -> None
5.1.9 set_option_row(conn, subu_id: str, name: str, value: str) -> None
5.1.10 get_option_row(conn, subu_id: str, name: str) -> str | None
5.1.11 list_option_rows(conn, subu_id: str) -> dict[str, str]

(Exact breakdown can be tuned when we see schema.sql.)
"""

# infrastructure/db.py
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-

def schema_path_default(): return Path(__file__).with_name("schema.sql")
def db_path_default(): return "."

def open_db(path ="subu.db"):
  """
  Return a sqlite3.Connection with sensible pragmas.
  Caller is responsible for closing.
  """
  conn = sqlite3.connect(path)
  conn.row_factory = sqlite3.Row
  conn.execute("PRAGMA foreign_keys = ON")
  conn.execute("PRAGMA journal_mode = WAL")
  conn.execute("PRAGMA synchronous = NORMAL")
  return conn

def ensure_schema(conn):
  """
  Ensure the schema in schema.sql is applied.
  This is idempotent: executing the DDL again is acceptable.
  """
  sql = schema_path_default().read_text(encoding="utf-8")
  conn.executescript(sql)
  conn.commit()

def _db():
  if not DB_FILE.exists():
    raise FileNotFoundError("subu.db not found; run `subu init <token>` first")
  return sqlite3.connect(DB_FILE)

def init_db(path: str = db_path_default()):
  """
  Initialise subu.db if missing; refuse to overwrite existing file.
  """
  if os.path.exists(path):
    print(f"subu: db already exists at {path}")
    return

  with closing(sqlite3.connect(path)) as db:
    db.executescript(SCHEMA_SQL)
    db.execute(
      "INSERT INTO meta(key,value) VALUES ('made_at', datetime('now'))"
    )
    db.commit()
    print(f"subu: made new db at {path}")


def cmd_init(token: str|None):
  if DB_FILE.exists():
    raise FileExistsError("db already exists")
  if not token or len(token) < 6:
    raise ValueError("init requires a 6+ char token")
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
    print(f"made subu.db (v{VERSION})")

def _first_free_id(db, table: str) -> int:
  """
  Return the smallest non-negative integer not in table.id.
  Assumes 'id' INTEGER PRIMARY KEY in that table.
  """
  rows = db.execute(f"SELECT id FROM {table} ORDER BY id ASC").fetchall()
  used = {r[0] for r in rows}
  i = 0
  while i in used:
    i += 1
  return i

def get_subu_by_full_unix_name(full_unix_name: str):
  """
  Return the DB row for a subu with this full_unix_name, or None.
  """
  with closing(open_db()) as db:
    row = db.execute(
      "SELECT id, owner, name, full_unix_name, path, netns_name "
      "FROM subu WHERE full_unix_name = ?",
      (full_unix_name,)
    ).fetchone()
    return row
