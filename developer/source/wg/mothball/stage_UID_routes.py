#!/usr/bin/env python3
# stage_UID_route.py — emit /usr/local/bin/set_subu_UID_route.sh from DB

from __future__ import annotations
import sys, sqlite3, ipaddress
from pathlib import Path
import incommon as ic

OUT = Path(__file__).resolve().parent / "stage" / "usr" / "local" / "bin" / "set_subu_UID_route.sh"

def main(argv: list[str]) -> int:
  try:
    with ic.open_db() as conn:
      rows = ic.rows(conn, """
        SELECT c.iface, c.rt_table_name_eff AS rtname, c.local_address_cidr,
               ub.uid
          FROM Iface c
     LEFT JOIN user_binding ub ON ub.iface_id=c.id
      ORDER BY c.iface, ub.uid;
      """)

      meta = dict(ic.rows(conn, "SELECT key, value FROM meta;"))
      subu_cidr = meta.get("subu_cidr", "10.0.0.0/24")
  except (sqlite3.Error, FileNotFoundError) as e:
    print(f"❌ {e}", file=sys.stderr); return 1

  OUT.parent.mkdir(parents=True, exist_ok=True)

  lines = []
  lines.append("#!/usr/bin/env bash")
  lines.append("# Set per-UID policy routing; idempotent.")
  lines.append("set -euo pipefail")
  lines.append('ensure(){ local n=\"$1\"; shift; if ! ip -4 rule list | grep -F -q -- \"$n\"; then ip -4 rule add \"$@\"; fi; }')
  lines.append('ensureroute(){ local tbl=\"$1\"; shift; ip route replace \"$@\" table \"$tbl\"; }')
  lines.append("")

  seen = set()
  for iface, rtname, cidr, uid in rows:
    if not iface: continue
    try: src_ip = str(ipaddress.IPv4Interface(cidr).ip)
    except: continue
    # table name per UID (avoid rt_tables entries by using numeric if you prefer)
    if uid is None: continue
    tname = f"{rtname}_u{uid}"
    key = (iface, uid)
    if key in seen: continue
    seen.add(key)

    # route: default via iface with pinned src
    lines.append(f"# uid {uid} on {iface} → src {src_ip} via table {tname}")
    lines.append(f'ensureroute "{tname}" default dev {iface} src {src_ip}')
    lines.append(f'ensure "uidrange {uid}-{uid} lookup {tname}" uidrange "{uid}-{uid}" lookup "{tname}" pref 17010')
    # symmetry guard for already-sourced packets
    lines.append(f'ensure "from {src_ip}/32 lookup {tname}" from "{src_ip}/32" lookup "{tname}" pref 17000')
    lines.append("")

  # global hard containment for subu space
  lines.append(f'# hard containment for subu space {subu_cidr}')
  lines.append(f'ensure "from {subu_cidr} prohibit" from "{subu_cidr}" prohibit pref 18050')
  content = "\n".join(lines) + "\n"
  OUT.write_text(content)
  OUT.chmod(0o500)
  print(f"staged: {OUT.relative_to(Path(__file__).resolve().parent)}")
  return 0

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
