#!/usr/bin/env python3
# stage_IP_route_script.py — emit /usr/local/bin/route_init_<iface>.sh from DB
# Purpose at runtime of the emitted script:
#   1) Ensure default + blackhole default in the dedicated route table.
#   2) Pin the peer endpoint (/32 via GW on NIC, metric 5) outside the tunnel so the handshake cannot vanish.
#   3) Apply any extra route from the route table (on_up=1).
#
# Usage: stage_IP_route_script.py <iface>
# Output: stage/usr/local/bin/route_init_<iface>.sh (chmod 500)
# Idempotence (runtime): uses `ip -4 route replace`
# Failure modes (runtime): if DNS resolution fails, step (2) is skipped; (1) and (3) still apply.

from __future__ import annotations
import sys, sqlite3
from pathlib import Path
import incommon as ic  # open_db(), rows()

def _bash_single_quote(s: str) -> str:
  # Safe single-quoted literal for bash
  return "'" + s.replace("'", "'\"'\"'") + "'"

def stage_ip_route_script(iface: str) -> Path:
  # Resolve DB data
  with ic.open_db() as conn:
    row = conn.execute(
      "SELECT id, rt_table_name_eff FROM v_client_effective WHERE iface=? LIMIT 1;",
      (iface,)
    ).fetchone()
    if not row:
      raise RuntimeError(f"iface not found in DB: {iface}")
    iface_id, rtname = int(row[0]), str(row[1])

    # Preferred server: lowest priority, then lowest id
    srow = conn.execute(
      """
      SELECT s.endpoint_host, s.endpoint_port
        FROM server s
        JOIN Iface c ON c.id=s.iface_id
       WHERE c.id=?
       ORDER BY s.priority ASC, s.id ASC
       LIMIT 1;
      """,
      (iface_id,)
    ).fetchone()
    ep_host = str(srow[0]) if srow and srow[0] else ""
    ep_port = str(srow[1]) if srow and srow[1] else ""

    # Extra route for on_up
    extra = ic.rows(conn, """
      SELECT cidr, COALESCE(via,''), COALESCE(table_name,''), COALESCE(metric,'')
        FROM route
       WHERE iface_id=? AND on_up=1
       ORDER BY id;
    """, (iface_id,))

  # Paths
  out_path = Path(__file__).resolve().parent / "stage" / "usr" / "local" / "bin" / f"route_init_{iface}.sh"
  out_path.parent.mkdir(parents=True, exist_ok=True)

  # Emit script
  lines: list[str] = []
  lines.append("#!/usr/bin/env bash")
  lines.append("set -euo pipefail")
  lines.append(f"table={_bash_single_quote(rtname)}")
  lines.append(f"dev={_bash_single_quote(iface)}")
  lines.append(f"endpoint_host={_bash_single_quote(ep_host)}")
  lines.append(f"endpoint_port={_bash_single_quote(ep_port)}")
  lines.append("")
  lines.append("# 1) Default in dedicated table")
  lines.append('ip -4 route replace default dev "$dev" table "$table"')
  lines.append('ip -4 route replace blackhole default metric 32767 table "$table"')
  lines.append("")
  lines.append("# 2) Keep peer endpoint reachable outside the tunnel")
  lines.append('ep_ip=$(getent ahostsv4 "$endpoint_host" | awk \'NR==1{print $1}\')')
  lines.append('if [[ -n "$ep_ip" ]]; then')
  lines.append('  gw=$(ip -4 route get "$ep_ip" | awk \'/ via /{print $3; exit}\')')
  lines.append('  nic=$(ip -4 route get "$ep_ip" | awk \'/ dev /{for(i=1;i<=NF;i++) if ($i=="dev"){print $(i+1); exit}}\')')
  lines.append('  if [[ -n "$gw" && -n "$nic" ]]; then')
  lines.append('    ip -4 route replace "${ep_ip}/32" via "$gw" dev "$nic" metric 5')
  lines.append('  fi')
  lines.append('fi')
  lines.append("")
  lines.append("# 3) Extra route from DB")
  for cidr, via, tbl, met in extra:
    cidr = str(cidr)
    via  = str(via or "")
    tbl  = str(tbl or rtname)
    met  = str(met or "")
    cmd = ["ip -4 route replace", cidr]
    if via: cmd += ["via", via]
    cmd += ['table', f'"{tbl}"']
    if met: cmd += ['metric', met]
    lines.append(" ".join(cmd))

  out_path.write_text("\n".join(lines) + "\n")
  out_path.chmod(0o500)

  return out_path

def main(argv: list[str]) -> int:
  if len(argv) != 1:
    print(f"Usage: {Path(sys.argv[0]).name} <iface>", file=sys.stderr)
    return 2
  iface = argv[0]
  try:
    out = stage_ip_route_script(iface)
  except (sqlite3.Error, FileNotFoundError, RuntimeError) as e:
    print(f"❌ {e}", file=sys.stderr); return 1
  # Print relative-to-CWD as requested style: 'stage/...'
  try:
    rel = out.relative_to(Path.cwd())
    print(f"staged: {rel}")
  except ValueError:
    print(f"staged: {out}")
  return 0

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
