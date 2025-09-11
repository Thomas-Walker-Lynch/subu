#!/usr/bin/env python3
# stage_list_client.py — emit one line per client with fields needed for staging
# Output format (pipe-separated, no header):
#   id|iface|rt_table_name|rt_table_id|addr|priv|mtu|fwmark|dns_mode|dns_servers|autostart

from __future__ import annotations
import sys, sqlite3
from pathlib import Path

def rows(conn: sqlite3.Connection, sql: str, params: tuple = ()) -> list[sqlite3.Row]:
  conn.row_factory = sqlite3.Row
  cur = conn.execute(sql, params)
  return cur.fetchall()

def list_client(db_path: Path) -> int:
  try:
    conn = sqlite3.connect(str(db_path))
  except sqlite3.Error as e:
    print(f"❌ sqlite open failed: {e}", file=sys.stderr)
    return 1

  try:
    # Prefer the view (effective rt_table_name); fall back to COALESCE if view missing.
    try_sql = """
      SELECT c.id,
             c.iface,
             v.rt_table_name_eff               AS rt_table_name,
             COALESCE(c.rt_table_id, '')       AS rt_table_id,
             c.local_address_cidr              AS addr,
             c.private_key                     AS priv,
             COALESCE(c.mtu,    '')            AS mtu,
             COALESCE(c.fwmark, '')            AS fwmark,
             c.dns_mode,
             COALESCE(c.dns_servers, '')       AS dns_servers,
             c.autostart
        FROM Iface c
        JOIN v_client_effective v ON v.id = c.id
       ORDER BY c.id;
    """
    try:
      R = rows(conn, try_sql)
    except sqlite3.Error:
      # Fallback without the view
      fallback_sql = """
        SELECT id,
               iface,
               COALESCE(rt_table_name, iface)   AS rt_table_name,
               COALESCE(rt_table_id, '')        AS rt_table_id,
               local_address_cidr               AS addr,
               private_key                      AS priv,
               COALESCE(mtu,    '')             AS mtu,
               COALESCE(fwmark, '')             AS fwmark,
               dns_mode,
               COALESCE(dns_servers, '')        AS dns_servers,
               autostart
          FROM Iface
         ORDER BY id;
      """
      R = rows(conn, fallback_sql)

    for r in R:
      fields = [
        r["id"],
        r["iface"],
        r["rt_table_name"],
        r["rt_table_id"],
        r["addr"],
        r["priv"],
        r["mtu"],
        r["fwmark"],
        r["dns_mode"],
        r["dns_servers"],
        r["autostart"],
      ]
      print("|".join("" if v is None else str(v) for v in fields))
    return 0
  finally:
    conn.close()

def main(argv: list[str]) -> int:
  if len(argv) != 1:
    prog = Path(sys.argv[0]).name
    print(f"Usage: {prog} /path/to/db", file=sys.stderr)
    return 2
  db_path = Path(argv[0])
  if not db_path.exists():
    print(f"❌ DB not found: {db_path}", file=sys.stderr)
    return 1
  return list_client(db_path)

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
