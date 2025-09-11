#!/usr/bin/env python3
"""
stage_IP_rules_script.py — synthesize & stage an ip-rule script (scoped to given users)
and per-interface systemd drop-ins that invoke it after wg-quick@IFACE up.

Inputs (CLI):
  stage_IP_rules_script.py <username1> [<username2> ...]
    - Looks up users in DB, finds their iface bindings & UIDs, and emits:
      1) stage/usr/local/bin/set_subu_IP_rules.sh
      2) stage/etc/systemd/wg-quick@IFACE.service.d/10-postup-IP-rules.conf (per iface)

Notes:
  - Script lines are idempotent via a small helper (grep before ip rule add).
  - We include per-iface source-CIDR rule and per-user UID rules, plus a SUBU containment rule.
  - Only the ifaces required by the provided users are staged.
"""

from __future__ import annotations
import ipaddress
import sqlite3
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Tuple

import incommon as ic  # expected to provide: open_db(), rows()

ROOT = Path(__file__).resolve().parent
STAGE_ROOT = ROOT / "stage"
OUTPUT_SCRIPT_NAME = "set_subu_IP_rules.sh"


# ---------- Data models ----------

@dataclass
class IfaceInfo:
    iface_id: int
    iface_name: str
    rt_table_name_eff: str
    local_address_cidr: str  # e.g., '10.8.0.2/32'


@dataclass
class UserBinding:
    username: str
    uid: int
    iface: IfaceInfo


# ---------- DB access ----------

def load_user_bindings(conn: sqlite3.Connection, usernames: Sequence[str]) -> List[UserBinding]:
    """
    Return one UserBinding per (username, iface) row for the given usernames.
    Requires uid to be non-null; raises if unknown usernames or missing UIDs.
    """
    if not usernames:
        return []

    # Fetch bindings joined with effective iface info
    placeholders = ",".join("?" for _ in usernames)
    sql = f"""
    SELECT
      ub.username,
      ub.uid,
      i.id               AS iface_id,
      i.iface            AS iface_name,
      v.rt_table_name_eff,
      v.local_address_cidr
    FROM User ub
    JOIN Iface i          ON i.id = ub.iface_id
    JOIN v_iface_effective v ON v.id = i.id
    WHERE ub.username IN ({placeholders})
    ORDER BY ub.username, i.iface;
    """
    rows = conn.execute(sql, tuple(usernames)).fetchall()

    found_usernames = {r[0] for r in rows}
    missing = [u for u in usernames if u not in found_usernames]
    if missing:
        raise RuntimeError(f"user(s) not found in User: {', '.join(missing)}")

    bindings: List[UserBinding] = []
    for (username, uid, iface_id, iface_name, rtname, cidr) in rows:
        if uid is None or str(uid) == "":
            raise RuntimeError(f"user '{username}' has no cached UID in DB (User.uid is NULL)")
        bindings.append(
            UserBinding(
                username=str(username),
                uid=int(uid),
                iface=IfaceInfo(
                    iface_id=int(iface_id),
                    iface_name=str(iface_name),
                    rt_table_name_eff=str(rtname),
                    local_address_cidr=str(cidr),
                ),
            )
        )
    return bindings


def load_subu_cidr(conn: sqlite3.Connection, default: str = "10.0.0.0/24") -> str:
    row = conn.execute("SELECT value FROM Meta WHERE key='subu_cidr' LIMIT 1;").fetchone()
    return str(row[0]) if row and row[0] else default


# ---------- Validation ----------

def assert_unique_hosts_in_subu(selected_ifaces: Iterable[IfaceInfo], subu_cidr: str) -> None:
    """
    Ensure there are no duplicate host IPs within the SUBU network among the selected ifaces.
    Only checks addresses that fall inside subu_cidr.
    """
    net = ipaddress.IPv4Network(subu_cidr, strict=False)
    seen: Dict[str, str] = {}
    for info in selected_ifaces:
        ip = str(ipaddress.IPv4Interface(info.local_address_cidr).ip)
        if ipaddress.IPv4Address(ip) in net:
            if ip in seen and seen[ip] != info.iface_name:
                raise RuntimeError(f"duplicate SUBU IP {ip} on {seen[ip]} and {info.iface_name}")
            seen[ip] = info.iface_name


# ---------- Script synthesis ----------

def synthesize_ip_rule_script(bindings: List[UserBinding], subu_cidr: str) -> List[str]:
    """
    Build the shell script lines implementing:
      - per-iface source-based rule:  from <src_cidr> lookup <table>
      - per-user UID rule:           uidrange U-U lookup <table>
      - SUBU containment rule:       from <SUBU> prohibit
    The helper add_IP_rule_if_not_exists makes rules idempotent.
    """
    # Group users by iface
    by_iface: Dict[int, Dict[str, object]] = {}
    for b in bindings:
        key = b.iface.iface_id
        entry = by_iface.setdefault(
            key,
            {
                "iface": b.iface.iface_name,
                "rtname": b.iface.rt_table_name_eff,
                "addr": b.iface.local_address_cidr,
                "uids": set(),  # type: ignore[dict-item]
            },
        )
        entry["uids"].add(b.uid)  # type: ignore[index]

    lines: List[str] = [
        "#!/usr/bin/env bash",
        "# Enforce IPv4 rules for selected users; idempotent per rule.",
        "set -euo pipefail",
        "",
        'add_IP_rule_if_not_exists(){ local search_phrase=$1; shift; if ! ip -4 rule list | grep -F -q -- "$search_phrase"; then ip -4 rule add "$@"; fi; }',
        "",
    ]

    # Emit per-iface blocks
    for _, data in sorted(by_iface.items(), key=lambda kv: kv[1]["iface"]):  # type: ignore[index]
        iface = str(data["iface"])
        table = str(data["rtname"])
        src_cidr = str(data["addr"])
        uids = sorted(int(u) for u in data["uids"])  # type: ignore[index]

        lines.append(f"# iface={iface} table={table} src={src_cidr}")
        # source-based rule
        lines.append(
            f'add_IP_rule_if_not_exists "from {src_cidr} lookup {table}" from "{src_cidr}" lookup "{table}" pref 17000'
        )
        # uid-based rules
        for u in uids:
            lines.append(
                f'add_IP_rule_if_not_exists "uidrange {u}-{u} lookup {table}" uidrange "{u}-{u}" lookup "{table}" pref 17010'
            )
        lines.append("")

    # SUBU containment (keeps traffic within the SUBU prefix from escaping unintended paths)
    lines += [
        "# hard containment for SUBU address space",
        f'add_IP_rule_if_not_exists "from {subu_cidr} prohibit" from "{subu_cidr}" prohibit pref 18050',
        "",
    ]
    return lines


def stage_ip_rule_script(lines: List[str], stage_root: Optional[Path] = None) -> Path:
    sr = stage_root or STAGE_ROOT
    out = sr / "usr" / "local" / "bin" / OUTPUT_SCRIPT_NAME
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text("\n".join(lines))
    out.chmod(0o500)
    return out


# ---------- systemd drop-in synthesis ----------

def stage_wg_systemd_postup_ip_dropin(iface_name: str, script_path_in_unit: str, stage_root: Optional[Path] = None) -> Path:
    """
    Write a per-interface systemd override so wg-quick@IFACE runs the rules script on 'up'.
    """
    sr = stage_root or STAGE_ROOT
    dropdir = sr / "etc" / "systemd" / f"wg-quick@{iface_name}.service.d"
    dropdir.mkdir(parents=True, exist_ok=True)
    path = dropdir / "10-postup-IP-rules.conf"
    content = f"""[Service]
# Ensure our ip rules are applied after wg-quick brings {iface_name} up
ExecStartPost=+{script_path_in_unit}
ExecStartPost=+/usr/bin/logger 'wg-quick@{iface_name} up: ip rules applied'
"""
    path.write_text(content)
    return path


# ---------- Orchestration ----------

def stage_for_users(usernames: Sequence[str], stage_root: Optional[Path] = None) -> Tuple[Path, Dict[str, Path], str]:
    """
    High-level: resolve users → bindings, validate, synthesize script, stage drop-ins (only needed ifaces).
    Returns (script_path, {iface: dropin_path}, summary)
    """
    if not usernames:
        raise RuntimeError("no usernames provided")

    with ic.open_db() as conn:
        bindings = load_user_bindings(conn, usernames)
        # Derive iface set from bindings
        iface_infos = {b.iface.iface_id: b.iface for b in bindings}.values()
        subu = load_subu_cidr(conn, "10.0.0.0/24")
        assert_unique_hosts_in_subu(iface_infos, subu)

    # Synthesize & stage the rules script
    script_lines = synthesize_ip_rule_script(bindings, subu)
    script_path = stage_ip_rule_script(script_lines, stage_root=stage_root)

    # Stage systemd drop-ins for just the ifaces we touched
    dropins: Dict[str, Path] = {}
    for iface in sorted({b.iface.iface_name for b in bindings}):
        dropins[iface] = stage_wg_systemd_postup_ip_dropin(
            iface_name=iface,
            script_path_in_unit=f"/usr/local/bin/{OUTPUT_SCRIPT_NAME}",
            stage_root=stage_root,
        )

    # Build a compact summary
    per_iface = ",".join(
        f"{iface}:{','.join(str(b.uid) for b in sorted({ub.uid for ub in bindings if ub.iface.iface_name==iface})) or '-'}"
        for iface in sorted({b.iface.iface_name for b in bindings})
    )
    summary = f"users={len(set(b.username for b in bindings))}, ifaces={len(dropins)} ({per_iface}), rules_script={script_path.relative_to(stage_root or STAGE_ROOT)}"
    return script_path, dropins, summary


# ---------- CLI ----------

def _print_usage_and_exit() -> None:
    prog = Path(sys.argv[0]).name
    print(f"Usage: {prog} <username1> [<username2> ...]", file=sys.stderr)
    sys.exit(2)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        _print_usage_and_exit()
    try:
        script_path, dropins, summary = stage_for_users(sys.argv[1:])
    except Exception as e:
        print(f"error: {e}", file=sys.stderr)
        sys.exit(1)
    print(f"staged: stage/{script_path.relative_to(STAGE_ROOT)}")
    for iface, p in dropins.items():
        print(f"staged: stage/{p.relative_to(STAGE_ROOT)} (iface {iface})")
    print(summary)
