import os
import pwd
import grp
import subprocess
from contextlib import closing

def _db():
  if not DB_FILE.exists():
    raise FileNotFoundError("subu.db not found; run `subu init <token>` first")
  return sqlite3.connect(DB_FILE)

def init_db(path: str = DB_PATH):
  """
  Initialise subu.db if missing; refuse to overwrite existing file.
  """
  if os.path.exists(path):
    print(f"subu: db already exists at {path}")
    return

  with closing(sqlite3.connect(path)) as db:
    db.executescript(SCHEMA_SQL)
    db.execute(
      "INSERT INTO meta(key,value) VALUES ('created_at', datetime('now'))"
    )
    db.commit()
    print(f"subu: created new db at {path}")


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
    print(f"created subu.db (v{VERSION})")

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
