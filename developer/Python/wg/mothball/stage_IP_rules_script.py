#!/usr/bin/env python3
"""
stage_IP_rules.py — stage a runtime script to enforce IPv4 rules for all subu

- Reads subu_cidr from DB.meta
- For each client: adds FROM <src_cidr> → <table> and per-UID rules
- Appends a final PROHIBIT for subu_cidr to enforce hard containment
- Writes: stage/usr/local/bin/<OUTPUT_SCRIPT_NAME> (no args at runtime)
"""

from __future__ import annotations
import sys
from pathlib import Path
from typing import Optional, Sequence, Dict, List
import incommon as ic

OUTPUT_SCRIPT_NAME = "set_subu_IP_rules.sh"

def stage_set_subu_ip_rules(ifaces: Optional[Sequence[str]] = None) -> tuple[Path, str]:
  with ic.open_db() as conn:  # ← no path arg
    client = ic.fetch_client(conn, ifaces)  # expects id, iface, rtname, addr from v_client_effective
    if not client: raise RuntimeError("no client selected")
    subu = ic.subu_cidr(conn, "10.0.0.0/24")
    ic.validate_unique_hosts(client, subu)
    uid_map: Dict[int, List[int]] = {int(c["id"]): ic.collect_uids(conn, int(c["id"])) for c in client}

  out = ic.STAGE_ROOT / "usr" / "local" / "bin" / OUTPUT_SCRIPT_NAME

  lines: List[str] = []
  
  lines += [
    "#!/usr/bin/env bash",
    "# Enforce IPv4 rules for all subu; idempotent per rule.",
    "set -euo pipefail",
    "",
    'add_IP_rule_if_not_exists(){ local search_phrase=$1; shift; if ! ip -4 rule list | grep -F -q -- "$search_phrase"; then ip -4 rule add "$@"; fi; }',
    ""
  ]

  for c in client:
    table = c["rtname"]; src_cidr = c["addr"]; cid = int(c["id"])
    lines += [f"# client: iface={c['iface']} table={table} src={src_cidr} id={cid}"]
    lines += [f'add_IP_rule_if_not_exists "from {src_cidr} lookup {table}" from "{src_cidr}" lookup "{table}" pref 17000']
    for u in uid_map[cid]:
      lines += [f'add_IP_rule_if_not_exists "from {src_cidr} lookup {table}" from "{src_cidr}" lookup "{table}" pref 17000']
    lines += [""]

  lines += [
    "# hard containment for subu space",
    f'add_IP_rule_if_not_exists "from {subu} prohibit" from "{subu}" prohibit pref 18050',
    ""
  ]

  ic.write_exec_quiet(out, "\n".join(lines))

  per_iface = ", ".join(
    f"{c['iface']}:[{','.join(str(u) for u in uid_map[int(c['id'])]) or '-'}]"
    for c in client
  )
  total_uid_rules = sum(len(uid_map[int(c["id"])]) for c in client)
  summary = f"client={len(client)}, uid_rules={total_uid_rules} ({per_iface})"
  return out, summary

def main(argv: Sequence[str]) -> int:
  ifaces = list(argv) if argv else None
  try:
    path, summary = stage_set_subu_ip_rules(ifaces)
  except Exception as e:
    print(f"❌ {e}", file=sys.stderr); return 1
  try:
    rel = "stage/" + path.relative_to(ic.STAGE_ROOT).as_posix()
  except Exception:
    rel = path.as_posix().replace(ic.ROOT.as_posix() + "/", "")
  print(f"staged: {rel} — {summary}")
  return 0

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
