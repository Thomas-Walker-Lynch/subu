#!/usr/bin/env python3
"""
deploy_dns.py — installs staged DNS artifacts (Unbound, nftables snippet)
without starting/stopping services. Prints next-step commands.
"""

from __future__ import annotations
from pathlib import Path
import os, sys

def main(argv=None) -> int:
  root = Path(__file__).resolve().parent
  stage = root / "stage"
  issues = []
  if os.geteuid() != 0:
    issues.append("must be run as root (sudo)")

  for rel in [
    "etc/unbound/unbound-US.conf",
    "etc/unbound/unbound-x6.conf",
    "etc/systemd/system/unbound@.service",
    "etc/nftables.d/30-dnsredir.nft",
  ]:
    if not (stage / rel).exists():
      issues.append(f"missing staged file: stage/{rel}")

  try:
    import install_staged_tree as ist
  except Exception as e:
    issues.append(f"failed to import install_staged_tree: {e}")

  if issues:
    print("❌ deploy preflight found issue(s):")
    for i in issues: print(f"  - {i}")
    return 2

  dest_root = Path("/")
  staged = ist.install_staged_tree(stage_root=stage, dest_root=dest_root)
  # Paths printed by install_staged_tree; keep our output short.
  print("\nNext steps:")
  print("  sudo systemctl daemon-reload")
  print('  # ensure nft snippet included in /etc/nftables.conf:')
  print('  #   include "/etc/nftables.d/30-dnsredir.nft"')
  print("  sudo nft -f /etc/nftables.conf")
  print("  sudo install -d -m 0755 /var/lib/unbound")
  print("  sudo unbound-anchor -a /var/lib/unbound/root.key")
  print("  sudo systemctl enable --now unbound@US unbound@x6")
  print("\nVerify:")
  print("  sudo ss -ltnup '( sport = :5301 or sport = :5302 )'")
  print("  sudo -u Thomas-US dig example.com +short")
  print("  sudo -u Thomas-x6 dig example.com +short")
  return 0

if __name__ == "__main__":
  sys.exit(main())
