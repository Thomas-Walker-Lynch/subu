#!/usr/bin/env python3
# stage_preferred_server.py — emit the preferred server row for a iface_id
# Output: name|peer_pub|psk|endpoint_host|endpoint_port|allowed_ips|keepalive|route_allowed_ips

from __future__ import annotations
import sys, sqlite3
from pathlib import Path
import incommon as ic

def preferred_server_row(iface_id: int) -> tuple | None:
  with ic.open_db() as conn:
    r = conn.execute("""
      SELECT name, public_key, COALESCE(preshared_key,''),
             endpoint_host, endpoint_port, allowed_ips,
             COALESCE(keepalive_s,''), route_allowed_ips
        FROM server
       WHERE iface_id=?
       ORDER BY priority ASC, id ASC
       LIMIT 1;
    """,(iface_id,)).fetchone()
    return tuple(r) if r else None

def main(argv):
  if len(argv)!=1:
    print(f"Usage: {Path(sys.argv[0]).name} <iface_id>", file=sys.stderr)
    return 2
  row = preferred_server_row(int(argv[0]))
  if not row:
    # empty stdout on "no server" just like the shell version
    return 0
  print("|".join("" if v is None else str(v) for v in row))
  return 0

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
