#!/usr/bin/env python3
# stage_list_uid.py — print Uid (one per line) bound to a iface_id

from __future__ import annotations
import sys, sqlite3
from pathlib import Path
import incommon as ic

def list_uid(iface_id: int) -> int:
  try:
    with ic.open_db() as conn:
      rows = conn.execute("""
        SELECT ub.uid
          FROM user_binding ub
         WHERE ub.iface_id=? AND ub.uid IS NOT NULL AND ub.uid!=''
         ORDER BY ub.uid;
      """,(iface_id,)).fetchall()
  except (sqlite3.Error, FileNotFoundError) as e:
    print(f"❌ {e}", file=sys.stderr); return 1
  for (uid,) in rows:
    print(uid)
  return 0

def main(argv):
  if len(argv)!=1:
    print(f"Usage: {Path(sys.argv[0]).name} <iface_id>", file=sys.stderr)
    return 2
  return list_uid(int(argv[0]))

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
