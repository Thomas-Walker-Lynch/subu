#!/usr/bin/env python3
# db_init_StanleyPark.py — initialize the DB for the StanleyPark client

from __future__ import annotations
import sys, subprocess, sqlite3
from pathlib import Path
import incommon as ic

# Use existing business functions (no duplication)
from db_init_iface_x6 import init_iface_x6
from db_init_iface_US  import init_iface_US
from db_init_server_x6 import init_server_x6
from db_init_server_US  import init_server_US
from db_bind_user_to_iface import bind_user_to_iface
from db_init_ip_table_registration import assign_missing_rt_table_ids
from db_init_ip_iface_addr_assign import reconcile_kernel_and_db_ipv4_addresses

ROOT = Path(__file__).resolve().parent
DB   = ic.DB_PATH

def msg_wrapped_call(title: str, fn=None, *args, **kwargs):
  """Print a before/after status line around calling `fn(*args, **kwargs)`.
  Returns the function’s return value."""
  print(f"→ {title}", flush=True)
  res = fn(*args, **kwargs) if fn else None
  print(f"✔ {title}" + (f": {res}" if res not in (None, "") else ""), flush=True)
  return res

def _run_local(script: str, *argv: str):
  subprocess.run([str(ROOT / script), *argv], check=True)

def db_init_StanleyPark() -> int:
  """
  Given the local SQLite DB at ic.DB_PATH,
  it loads schema, upserts ifaces (x6, US), upserts server (x6, US),
  binds users (Thomas-x6→x6, Thomas-US→US), generates missing keypairs,
  commits, and prints public keys. Returns 0 on success (raises on failure).
  """
  # 1) Schema
  msg_wrapped_call("db_schema_load.sh", _run_local, "db_schema_load.sh")

  # 2) DB work in one connection/commit
  with ic.open_db(DB) as conn:
    msg_wrapped_call("db_init_iface_x6.py (init_iface_x6)", init_iface_x6, conn)
    msg_wrapped_call("db_init_server_x6.py (init_server_x6)", init_server_x6, conn)
    msg_wrapped_call("bind_user_to_iface: Thomas-x6 → x6", bind_user_to_iface, conn, "x6", "Thomas-x6")

    msg_wrapped_call("db_init_iface_US.py (init_iface_US)", init_iface_US, conn)
    msg_wrapped_call("db_init_server_US.py (init_server_US)", init_server_US, conn)
    msg_wrapped_call("bind_user_to_iface: Thomas-US → US", bind_user_to_iface, conn, "US", "Thomas-US")

    msg_wrapped_call(
      "db_init_ip_table_registration"
      ,lambda: assign_missing_rt_table_ids(conn ,low=20000 ,high=29999 ,dry_run=False)
    )

    msg_wrapped_call(
      "db_init_ip_iface_addr_assign"
      ,lambda: reconcile_kernel_and_db_ipv4_addresses(conn ,pool_cidr="10.0.0.0/16" ,assign_prefix=32 ,reserve_first=0 ,dry_run=False)
    )

    conn.commit()
    print("✔ commit: database updated")

  return 0

def main(argv):
  if argv:
    print(f"Usage: {Path(sys.argv[0]).name}", file=sys.stderr)
    return 2
  try:
    return db_init_StanleyPark()
  except (subprocess.CalledProcessError, sqlite3.Error, FileNotFoundError) as e:
    print(f"❌ {e}", file=sys.stderr)
    return 1

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
