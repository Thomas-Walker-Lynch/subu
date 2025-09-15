#!/usr/bin/env python3
# stage_wg_unit_IP_scripts.py — write systemd unit override for wg-quick@IFACE

from __future__ import annotations
import sys
from pathlib import Path

def stage_dropin(iface: str) -> Path:
  root = Path(__file__).resolve().parent
  stage_root = root / "stage"
  dropin_dir = stage_root / "etc" / "systemd" / f"wg-quick@{iface}.service.d"
  dropin_dir.mkdir(parents=True, exist_ok=True)
  conf = dropin_dir / "10-postup-IP-scripts.conf"
  conf.write_text(
    "[Service]\n"
    "Restart=on-failure\n"
    "RestartSec=5\n"
    f"ExecStartPre=-/usr/sbin/ip link delete {iface}\n"
    f"ExecStartPost=+/usr/local/bin/set_subu_IP_rules.sh\n"
    f"ExecStartPost=+/usr/local/bin/route_init_{iface}.sh\n"
    f"ExecStartPost=+/usr/bin/logger 'wg-quick@{iface} up: rules+route applied'\n"
  )
  return conf

def main(argv):
  if len(argv)!=1:
    print(f"Usage: {Path(sys.argv[0]).name} <iface>", file=sys.stderr); return 2
  p = stage_dropin(argv[0])
  # print a "stage/..." relative path for consistency
  root = Path(__file__).resolve().parent
  rel = p.as_posix().replace(root.as_posix() + "/", "")
  print(f"staged: {rel}")
  return 0

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
