#!/usr/bin/env python3
"""
deploy_DNS.py — Deploy staged DNS bundle (Unbound per-subu + nft redirect)
RT-v2025.09.15.1

Given:
  - A staged tree under ./stage with:
      * etc/unbound/unbound-{US,x6}.conf
      * etc/systemd/system/unbound@.service
      * etc/systemd/system/DNS-redirect.service
      * etc/nftables.d/DNS-redirect.nft
  - install_staged_tree.py available in PYTHONPATH or alongside this script.
  - Instance names provided via --instances (default: US x6).

Does:
  - Validates root, prints a plan with short 'stage:/' paths.
  - Installs staged files into / (preserving backups) via install_staged_tree.install_staged_tree().
  - systemctl daemon-reload
  - Enables & starts: DNS-redirect.service, unbound@<instance>.service for each instance.

Returns:
  - Exit 0 on success, 2 on errors.
"""

from __future__ import annotations
from pathlib import Path
import argparse, importlib, os, sys, subprocess

ROOT = Path(__file__).resolve().parent
STAGE = ROOT / "stage"

def _short(p: Path) -> str:
  try:
    return "stage:/" + str(p.relative_to(STAGE)).replace('\\\','/')
  except Exception:
    return str(p)

def _run(cmd: list[str]) -> tuple[int,str,str]:
  cp = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  return (cp.returncode, cp.stdout.strip(), cp.stderr.strip())

def _require_root() -> None:
  errs = []
  if os.geteuid() != 0:
    errs.append("must be run as root (sudo)")
  if not STAGE.exists():
    errs.append(f"stage dir missing: {STAGE}")
  if errs:
    raise RuntimeError("; ".join(errs))

def deploy(instances: list[str]) -> list[str]:
  logs: list[str] = []
  # Import installer
  try:
    ist = importlib.import_module("install_staged_tree")
  except Exception as e:
    raise RuntimeError(f"failed to import install_staged_tree: {e}")

  # Plan
  logs.append("Deploy DNS plan:")
  logs.append(f"  instances: {', '.join(instances)}")
  logs.append(f"  stage: {STAGE}")
  logs.append(f"  root:  /")
  logs.append("")
  logs.append("Installing staged artifacts…")

  # Install
  paths = ist.install_staged_tree(STAGE, dry_run=False)  # expects function signature from your project
  for item in paths:
    try:
      action, src, dst = item
    except Exception:
      logs.append(str(item))
      continue
    if action == "backup":
      logs.append(f"backup: {dst} -> {src}")
    elif action == "install":
      logs.append(f"install: stage:/{Path(src).relative_to(STAGE)} -> {dst}")
    elif action == "identical":
      logs.append(f"identical: skip stage:/{Path(src).relative_to(STAGE)}")
    else:
      logs.append(f"{action}: {src} -> {dst}")

  # Reload and (enable|start) units
  _run(["systemctl","daemon-reload"])

  # DNS redirect service
  _run(["systemctl","enable","--now","DNS-redirect.service"])
  rc, out, err = _run(["systemctl","is-active","DNS-redirect.service"])
  logs.append(f"DNS-redirect.service: {'active' if rc==0 else 'inactive'}")

  # Unbound instances
  for inst in instances:
    unit = f"unbound@{inst}.service"
    _run(["systemctl","enable","--now", unit])
    rc, out, err = _run(["systemctl","is-active", unit])
    logs.append(f"{unit}: {'active' if rc==0 else 'inactive'}")
  logs.append("")
  logs.append("✓ DNS deploy complete.")
  return logs

def main(argv=None) -> int:
  ap = argparse.ArgumentParser(description="Deploy staged DNS (Unbound per-subu + nft redirect).")
  ap.add_argument("--instances", nargs="+", default=["US","x6"], help="Unbound instances to enable (default: US x6)")
  args = ap.parse_args(argv)

  try:
    _require_root()
  except Exception as e:
    print(f"❌ deploy preflight found issue(s):\n  - {e}", file=sys.stderr)
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
