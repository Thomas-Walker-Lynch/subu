#!/usr/bin/env python3
# stage_IP_routes_script.py — emit /usr/local/bin/routes_init_<iface>.sh

from __future__ import annotations
import sqlite3, sys
from pathlib import Path
import incommon as ic  # open_db, rows (works with DB_PATH)

ROOT = Path(__file__).resolve().parent
STAGE_ROOT = ROOT / "stage"

def _sq(s: str) -> str:
  return "'" + s.replace("'", "'\"'\"'") + "'"

def stage_ip_routes_script(iface: str) -> Path:
  """
  Given an iface name, queries DB for its table name and preferred server,
  and writes a runtime script that:
    1) sets default + blackhole default in the iface's route table
    2) pins the server endpoint /32 via the current GW/NIC (metric 5)
    3) applies extra Route rows (on_up=1)
  Returns the path written.
  """
  with ic.open_db() as conn:
    row = conn.execute(
      "SELECT id, rt_table_name_eff FROM v_iface_effective WHERE iface=? LIMIT 1;",
      (iface,)
    ).fetchone()
    if not row:
      raise RuntimeError(f"iface not found in DB: {iface}")
    iface_id, rtname = int(row[0]), str(row[1])

    srow = conn.execute(
      """
      SELECT s.endpoint_host, s.endpoint_port
        FROM Server s
        JOIN Iface c ON c.id = s.iface_id
       WHERE c.id=?
       ORDER BY s.priority ASC, s.id ASC
       LIMIT 1;
      """,
      (iface_id,)
    ).fetchone()
    ep_host = (srow[0] if srow and srow[0] else "")
    ep_port = (srow[1] if srow and srow[1] else "")

    extra = ic.rows(conn, """
      SELECT cidr, COALESCE(via,''), COALESCE(table_name,''), COALESCE(metric,'')
        FROM Route
       WHERE iface_id=? AND on_up=1
       ORDER BY id;
    """, (iface_id,))

  out = STAGE_ROOT / "usr" / "local" / "bin" / f"routes_init_{iface}.sh"
  out.parent.mkdir(parents=True, exist_ok=True)

  lines = [
    "#!/usr/bin/env bash",
    "set -euo pipefail",
    f"table={_sq(rtname)}",
    f"dev={_sq(iface)}",
    f"endpoint_host={_sq(str(ep_host))}",
    f"endpoint_port={_sq(str(ep_port))}",
    "",
    "# 1) Default in dedicated table",
    'ip -4 route replace default dev "$dev" table "$table"',
    'ip -4 route replace blackhole default metric 32767 table "$table"',
    "",
    "# 2) Keep peer endpoint reachable outside the tunnel",
    'ep_ip=$(getent ahostsv4 "$endpoint_host" | awk \'NR==1{print $1}\')',
    'if [[ -n "$ep_ip" ]]; then',
    '  gw=$(ip -4 route get "$ep_ip" | awk \'/ via /{print $3; exit}\')',
    '  nic=$(ip -4 route get "$ep_ip" | awk \'/ dev /{for(i=1;i<=NF;i++) if ($i=="dev"){print $(i+1); exit}}\')',
    '  if [[ -n "$gw" && -n "$nic" ]]; then',
    '    ip -4 route replace "${ep_ip}/32" via "$gw" dev "$nic" metric 5',
    '  fi',
    'fi',
    "",
    "# 3) Extra routes from DB",
  ]

  for cidr, via, tbl, met in extra:
    cidr = str(cidr); via = str(via or ""); tbl = str(tbl or rtname); met = str(met or "")
    cmd = ["ip -4 route replace", cidr]
    if via: cmd += ["via", via]
    cmd += ["table", f'"{tbl}"']
    if met: cmd += ["metric", met]
    lines.append(" ".join(cmd))

  out.write_text("\n".join(lines) + "\n")
  out.chmod(0o500)
  return out

def main(argv):
  if len(argv) != 1:
    print(f"Usage: {Path(sys.argv[0]).name} <iface>", file=sys.stderr); return 2
  try:
    p = stage_ip_routes_script(argv[0])
  except (sqlite3.Error, FileNotFoundError, RuntimeError) as e:
    print(f"❌ {e}", file=sys.stderr); return 1
  print(f"staged: {p.relative_to(ROOT)}")
  return 0

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
