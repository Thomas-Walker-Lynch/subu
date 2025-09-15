#!/usr/bin/env python3
"""
deploy_DNS.py — Deploy staged DNS bundle (Unbound per-subu + nft redirect/egress)
RT-v2025.09.15.2

Model:
  - nftables is the single authority for firewall/NAT.
  - /etc/nftables.conf includes all /etc/nftables.d/*.nft drop-ins.
  - Unbound runs as instances (unbound@US, unbound@x6), each bound to 127.0.0.1:53xx.
  - No separate DNS-redirect.service unit.

Given:
  - stage/ contains:
      * etc/unbound/unbound-<inst>.conf           (e.g. unbound-US.conf, unbound-x6.conf)
      * etc/systemd/system/unbound@.service       (template, if you ship one)
      * etc/nftables.d/20-SUBU-ports.nft          (your combined redirect + egress rules)
  - install_staged_tree.py importable (same dir or PYTHONPATH).
  - Instances list via --instances (default: US x6).

Does:
  - Validates root and presence of stage/.
  - Installs staged tree with backups (via install_staged_tree.install_staged_tree()).
  - Ensures /etc/nftables.conf has a single include line for /etc/nftables.d/*.nft
    (adds it if missing; keeps any existing content, does NOT add `flush ruleset` here).
  - Disables/removes any legacy DNS-redirect.service if present.
  - Enables and reloads nftables.service.
  - Enables/starts unbound@<instance>.service for each instance.
  - Prints concise logs with stage:/… relative paths.

Exit:
  - 0 on success; 2 on preflight/deploy errors.
"""

from __future__ import annotations
from pathlib import Path
import argparse, importlib, os, sys, subprocess, shutil

ROOT = Path(__file__).resolve().parent
STAGE = ROOT / "stage"
NFT_CONF = Path("/etc/nftables.conf")
NFT_INCLUDE_LINE = 'include "/etc/nftables.d/*.nft"'

def _short(p: Path) -> str:
  try:
    return "stage:/" + str(p.relative_to(STAGE)).replace("\\", "/")
  except Exception:
    return str(p)

def _run(cmd: list[str]) -> tuple[int, str, str]:
  cp = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  return (cp.returncode, cp.stdout.strip(), cp.stderr.strip())

def _require_root_and_stage() -> None:
  errs = []
  if os.geteuid() != 0:
    errs.append("must be run as root (sudo)")
  if not STAGE.exists():
    errs.append(f"stage dir missing: {STAGE}")
  if errs:
    raise RuntimeError("; ".join(errs))

def _ensure_nft_include_line(logs: list[str]) -> None:
  # Make sure /etc/nftables.conf includes our drop-ins exactly once.
  # We do NOT force 'flush ruleset' here—your .nft files should be self-contained.
  if not NFT_CONF.exists():
    raise RuntimeError(f"{NFT_CONF} not found (install nftables or create a minimal config)")

  text = NFT_CONF.read_text()
  if NFT_INCLUDE_LINE in text:
    logs.append(f"nftables: include already present in {NFT_CONF}")
    return

  # Append the include at the end with a preceding newline if needed.
  sep = "" if text.endswith("\n") else "\n"
  NFT_CONF.write_text(text + f"{sep}{NFT_INCLUDE_LINE}\n")
  logs.append(f"nftables: appended include to {NFT_CONF}")

def _retire_legacy_unit(unit: str, logs: list[str]) -> None:
  # Best-effort: disable and remove old per-feature unit if present.
  rc, _, _ = _run(["systemctl", "is-enabled", unit])
  if rc == 0:
    _run(["systemctl", "disable", "--now", unit])
    logs.append(f"retired: {unit} (disabled + stopped)")
  unit_path = Path("/etc/systemd/system") / unit
  if unit_path.exists():
    try:
      unit_path.unlink()
      logs.append(f"removed: {unit_path}")
    except Exception as e:
      logs.append(f"warn: could not remove {unit_path}: {e}")

def deploy(instances: list[str]) -> list[str]:
  logs: list[str] = []

  # 1) Import installer
  try:
    ist = importlib.import_module("install_staged_tree")
  except Exception as e:
    raise RuntimeError(f"failed to import install_staged_tree: {e}")

  # 2) Plan
  logs.append("Deploy DNS plan:")
  logs.append(f"  instances: {', '.join(instances)}")
  logs.append(f"  stage: {STAGE}")
  logs.append(f"  root:  /")
  logs.append("")
  logs.append("Installing staged artifacts…")

  # 3) Install staged tree
  results = ist.install_staged_tree(STAGE, dry_run=False)
  for item in results:
    try:
      action, backup_or_src, dst = item
    except Exception:
      logs.append(str(item))
      continue
    if action == "backup":
      logs.append(f"backup: {dst} -> {backup_or_src}")
    elif action == "install":
      logs.append(f"install: {_short(Path(backup_or_src))} -> {dst}")
    elif action == "identical":
      logs.append(f"identical: skip {_short(Path(backup_or_src))}")
    else:
      logs.append(f"{action}: {backup_or_src} -> {dst}")

  # 4) Systemd reload (units may have been installed)
  _run(["systemctl", "daemon-reload"])

  # 5) Retire any old per-feature unit
  _retire_legacy_unit("DNS-redirect.service", logs)

  # 6) Ensure nftables includes drop-ins
  _ensure_nft_include_line(logs)

  # 7) Enable + reload nftables to pick up new rules
  _run(["systemctl", "enable", "--now", "nftables"])
  rc, _, _ = _run(["nft", "-c", "-f", str(NFT_CONF)])
  if rc != 0:
    raise RuntimeError(f"nftables config check failed for {NFT_CONF}")
  _run(["systemctl", "reload", "nftables"])
  rc, out, _ = _run(["systemctl", "is-active", "nftables"])
  logs.append(f"nftables.service: {'active' if rc==0 else 'inactive'}")

  # 8) Unbound instances
  for inst in instances:
    unit = f"unbound@{inst}.service"
    _run(["systemctl", "enable", "--now", unit])
    rc, _, _ = _run(["systemctl", "is-active", unit])
    logs.append(f"{unit}: {'active' if rc==0 else 'inactive'}")

  logs.append("")
  logs.append("✓ DNS deploy complete.")
  return logs

def main(argv=None) -> int:
  ap = argparse.ArgumentParser(description="Deploy staged DNS (Unbound per-subu + nft redirect/egress).")
  ap.add_argument("--instances", nargs="+", default=["US", "x6"], help="Unbound instances to enable (default: US x6)")
  args = ap.parse_args(argv)

  try:
    _require_root_and_stage()
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
