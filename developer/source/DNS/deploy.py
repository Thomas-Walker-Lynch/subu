#!/usr/bin/env python3
"""
deploy.py — Deploy staged DNS bundle (Unbound per-subu + nft redirect)
RT-v2025.09.15.4

What it does
  - Installs the staged tree under ./stage into /
  - systemctl daemon-reload
  - nft -f /etc/nftables.conf   (relies on: include "/etc/nftables.d/*.nft")
  - enable + restart unbound@<instance> for each instance (default: US x6)

Assumptions
  - This file lives next to install_staged_tree.py
  - Stage contains:
      stage/etc/nftables.d/10-block-IPv6.nft
      stage/etc/nftables.d/20-SUBU-ports.nft
      stage/etc/systemd/system/unbound@.service
      stage/etc/unbound/unbound-US.conf (127.0.0.1@5301)
      stage/etc/unbound/unbound-x6.conf (127.0.0.1@5302)
  - /etc/nftables.conf has:  include "/etc/nftables.d/*.nft"

Exit codes
  0 = success, 2 = preflight/deploy error
"""
from __future__ import annotations
from pathlib import Path
import argparse
import importlib
import os
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
STAGE = ROOT / "stage"

def _run(cmd: list[str]) -> tuple[int, str, str]:
  cp = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  return (cp.returncode, cp.stdout.strip(), cp.stderr.strip())

def _preflight_errors() -> list[str]:
  errs = []
  if os.geteuid() != 0:
    errs.append("must be run as root (sudo)")
  if not STAGE.exists():
    errs.append(f"stage dir missing: {STAGE}")
  return errs

def _install_stage(stage_root: Path) -> list[str]:
  """
  Call install_staged_tree.install_staged_tree(stage_root=..., dest_root=/, create_dirs=True)
  and return its log lines.
  """
  sys.path.insert(0, str(ROOT))
  try:
    ist = importlib.import_module("install_staged_tree")
  except Exception as e:
    raise RuntimeError(f"failed to import install_staged_tree: {e}")

  # Expect signature: (stage_root, dest_root, create_dirs=False, skip_identical=True) -> (logs, ifaces)
  try:
    logs, _ifaces = ist.install_staged_tree(
      stage_root=stage_root,
      dest_root=Path("/"),
      create_dirs=True,
      skip_identical=True,
    )
  except TypeError as te:
    # Fallback for older two-arg signature: install_staged_tree(stage_root, dest_root)
    try:
      logs, _ifaces = ist.install_staged_tree(stage_root, Path("/"))
    except Exception as e2:
      raise RuntimeError(f"install_staged_tree() call failed: {e2}") from te
  return logs

def deploy(instances: list[str]) -> list[str]:
  logs: list[str] = []

  # Plan
  logs.append("Deploy DNS plan:")
  logs.append(f"  instances: {', '.join(instances)}")
  logs.append(f"  stage: {STAGE}")
  logs.append(f"  root:  /")
  logs.append("")
  logs.append("Installing staged artifacts…")

  # Install staged files
  install_logs = _install_stage(STAGE)
  logs.extend(install_logs)

  # Reload systemd units (for unbound@.service changes)
  _run(["systemctl", "daemon-reload"])

  # Apply nftables from the main config (which includes drop-ins)
  rc, out, err = _run(["/usr/sbin/nft", "-f", "/etc/nftables.conf"])
  if rc != 0:
    raise RuntimeError(f"nftables apply failed:\n{err or out}")

  # Sanity: verify our tables are present
  rc2, out2, err2 = _run(["/usr/sbin/nft", "list", "tables"])
  if rc2 != 0:
    raise RuntimeError(f"nftables list tables failed:\n{err2 or out2}")

  required = {"inet NO-IPV6", "inet SUBU-DNS-REDIRECT", "inet SUBU-PORT-EGRESS"}
  present = set()
  for line in out2.splitlines():
    parts = line.strip().split()
    # lines look like: "table inet FOO"
    if len(parts) == 3 and parts[0] == "table":
      present.add(f"{parts[1]} {parts[2]}")
  missing = required - present
  if missing:
    raise RuntimeError(f"nftables missing tables: {', '.join(sorted(missing))}")

  # Enable + restart unbound instances
  for inst in instances:
    unit = f"unbound@{inst}.service"
    _run(["systemctl", "enable", unit])
    _run(["systemctl", "restart", unit])
    rcA, _, _ = _run(["systemctl", "is-active", unit])
    logs.append(f"{unit}: {'active' if rcA == 0 else 'inactive'}")

  logs.append("")
  logs.append("✓ DNS deploy complete.")
  return logs

def main(argv=None) -> int:
  ap = argparse.ArgumentParser(description="Deploy staged DNS (Unbound per-subu + nft redirect).")
  ap.add_argument("--instances", nargs="+", default=["US", "x6"],
                  help="Unbound instances to enable (default: US x6)")
  args = ap.parse_args(argv)

  errs = _preflight_errors()
  if errs:
    print("❌ deploy preflight found issue(s):", file=sys.stderr)
    for e in errs:
      print(f"  - {e}", file=sys.stderr)
    return 2

  try:
    logs = deploy(args.instances)
    print("\n".join(logs))
    return 0
  except Exception as e:
    print(f"❌ deploy failed: {e}", file=sys.stderr)
    return 2

if __name__ == "__main__":
  sys.exit(main())
