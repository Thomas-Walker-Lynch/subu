#!/usr/bin/env python3
# stage_wg_systemd_postup_ip_dropin.py — stage a systemd unit override ("dropin")
# for wg-quick@IFACE to run IP policy + route scripts after the interface comes up.

from __future__ import annotations
from pathlib import Path
from typing import Optional
import argparse
import re

ROOT = Path(__file__).resolve().parent
STAGE_ROOT = ROOT / "stage"

_IFACE_WELLFORMED_RE = re.compile(r"^[A-Za-z0-9_.-]+$")

def wellformed_iface_name_guard(iface: str)
  if not iface or not _IFACE_WELLFORMED_RE.match(iface):
    raise ValueError(f"Invalid iface '{iface}': allowed chars are A–Z, a–z, 0–9, _ . -")

def stage_wg_systemd_postup_ip_dropin(iface: str ,*) -> Path:
  """
  Create and stage a systemd droppin for setting wg rules and routes
    - ExecStartPre: delete a stale dev 'iface' before wg-quick runs.
    - ExecStartPost: set the tunnel ip rules and routes
        /usr/local/bin/wg_IP_rules.sh
    - Emit a log line via logger.

  Returns: Path to the staged drop-in file.
  """
  wellformed_iface_name_guard(iface)
  sr = stage_root or STAGE_ROOT
  dropin_dir = sr / "etc" / "systemd" / f"wg-quick@{iface}.service.d"
  dropin_dir.mkdir(parents=True, exist_ok=True)
  dropin_conf_path = dropin_dir / "10-postup-IP-scripts.conf"

  restart_lines = "Restart=on-failure\nRestartSec=5" if restart_on_failure else ""
  pre_line = f"ExecStartPre=-/usr/sbin/ip link delete {iface}" if delete_iface_pre else ""
  policy_line = (
    "ExecStartPost=+/usr/local/bin/set_subu_IP_rules.sh"
    if use_global_policy_script
    else f"ExecStartPost=+/usr/local/bin/policy_init_{iface}.sh"
  )

  content = f"""[Service]
{restart_lines}
{pre_line}
{policy_line}
ExecStartPost=+/usr/local/bin/routes_init_{iface}.sh
ExecStartPost=+/usr/bin/logger 'wg-quick@{iface} up: rules+routes applied'
"""
  # Remove blank lines if some options are disabled
  content = "\n".join(ln for ln in content.splitlines() if ln.strip()) + "\n"
  dropin_conf_path.write_text(content)
  return dropin_conf_path

# ----- CLI -----
if __name__ == "__main__":
  p = argparse.ArgumentParser(
    description="Stage a systemd drop-in to run IP policy+routes after wg-quick@IFACE up"
  )
  p.add_argument("iface", help="WireGuard interface name (e.g., x6)")
  p.add_argument("--per-iface-policy", action="store_true",
                 help="Use /usr/local/bin/policy_init_<iface>.sh instead of global set_subu_IP_rules.sh")
  p.add_argument("--stage-root", type=Path, default=None,
                 help="Custom staging root (default: ./stage next to this script)")
  p.add_argument("--no-delete-pre", action="store_true",
                 help="Do not delete a stale iface before wg-quick runs")
  p.add_argument("--no-restart", action="store_true",
                 help="Do not set Restart=on-failure/RestartSec=5")
  args = p.parse_args()

  out = stage_wg_systemd_postup_ip_dropin(
    args.iface,
    use_global_policy_script=not args.per_iface_policy,
    stage_root=args.stage_root,
    delete_iface_pre=not args.no_delete_pre,
    restart_on_failure=not args.no_restart,
  )
  print(f"staged: {out.relative_to(ROOT)}")

  
