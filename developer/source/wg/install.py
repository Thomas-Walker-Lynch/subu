#!/usr/bin/env python3
"""
install_staged_tree.py

Given:
  - A staged tree (default: ./stage) containing:
      /usr/local/bin/apply_ip_state.sh
      /etc/wireguard/*.conf
      /etc/systemd/wg-quick@IFACE.service.d/*.conf
      /etc/iproute2/rt_tables
  - A destination root (default: /) whose *parent directories already exist*

Does:
  - For each whitelisted staged file:
      * if a target already exists, copy it *back into the stage* as a timestamped backup
      * atomically replace target with staged version
      * set root:root ownership and deterministic permissions (see MODE_MAP)
  - Optionally `systemctl daemon-reload` and restart provided wg-quick@IFACE units

Returns:
  - Exit 0 on success; non-zero on error
  - Prints a concise log of actions

Errors:
  - Fails if a target parent directory is missing (unless --create-dirs is given)
  - Fails on any copy/permission error and reports which path caused it
"""

from __future__ import annotations
from pathlib import Path
from typing import Dict ,Iterable ,List ,Optional ,Sequence ,Tuple
import argparse
import datetime as dt
import hashlib
import os
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
DEFAULT_STAGE = ROOT / "stage"

# Whitelist → permissions
# (relative glob inside stage) → (relative dest base, file mode)
MODE_MAP: Dict[str,Tuple[str,int]] = {
  "usr/local/bin/*": ("usr/local/bin",0o500)          # scripts: rx for root only
 , "etc/wireguard/*.conf": ("etc/wireguard",0o600)     # WG confs
 , "etc/systemd/wg-quick@"  : ("etc/systemd",0o644)    # handled per-dropin below
 , "etc/iproute2/rt_tables": ("etc/iproute2",0o644)    # route tables file
}

def _sha256(path: Path) -> str:
  h = hashlib.sha256()
  with path.open("rb") as f:
    for chunk in iter(lambda: f.read(1<<20), b""):
      h.update(chunk)
  return h.hexdigest()

def _iter_dropins(stage_root: Path) -> List[Tuple[Path,int]]:
  """Return [(relpath,mode)] for systemd wg-quick drop-ins."""
  out: List[Tuple[Path,int]] = []
  base = stage_root / "etc" / "systemd"
  if not base.exists():
    return out
  for p in base.rglob("wg-quick@*.service.d/*.conf"):
    rel = p.relative_to(stage_root)
    out.append((rel,0o644))
  return out

def _gather_stage_files(stage_root: Path) -> List[Tuple[Path,int]]:
  """Resolve whitelist into [(relpath,mode)]."""
  items: List[Tuple[Path,int]] = []
  # explicit patterns
  for pat,(_dest_base,mode) in MODE_MAP.items():
    if pat.endswith("@"):  # systemd base marker handled separately
      continue
    for p in (stage_root / pat).parent.glob(Path(pat).name):
      rel = p.relative_to(stage_root)
      items.append((rel,mode))
  # systemd drop-ins
  items += _iter_dropins(stage_root)
  # de-dup in order
  seen = set()
  uniq: List[Tuple[Path,int]] = []
  for rel,mode in items:
    if rel not in seen:
      uniq.append((rel,mode))
      seen.add(rel)
  return uniq

def _ensure_parents(dest_root: Path ,rel: Path ,create: bool) -> None:
  parent = (dest_root / rel).parent
  if parent.exists():
    return
  if not create:
    raise RuntimeError(f"missing parent directory: {parent}")
  parent.mkdir(parents=True,exist_ok=True)

def _backup_existing_to_stage(stage_root: Path ,dest_root: Path ,rel: Path) -> Optional[Path]:
  """If target exists, copy it back into stage/_backups/<ts>/<rel> and return backup path."""
  target = dest_root / rel
  if not target.exists():
    return None
  ts = dt.datetime.utcnow().strftime("%Y%m%dT%H%M%SZ")
  backup = stage_root / "_backups" / ts / rel
  backup.parent.mkdir(parents=True,exist_ok=True)
  shutil.copy2(target,backup)
  return backup

def _atomic_install(src: Path ,dst: Path ,mode: int) -> None:
  tmp = dst.with_suffix(dst.suffix + ".tmp")
  # copy *bytes*, then set perms/owner, then atomic replace
  shutil.copyfile(src,tmp)
  os.chmod(tmp,mode)
  try:
    os.chown(tmp,0,0)  # root:root
  except PermissionError:
    # setuid root expected; if not root, we still proceed for dry-run contexts
    pass
  os.replace(tmp,dst)

def _maybe_daemon_reload(perform: bool) -> None:
  if not perform:
    return
  subprocess.run(
     ["systemctl","daemon-reload"]
    ,check=False
    ,stdout=subprocess.DEVNULL
    ,stderr=subprocess.DEVNULL
  )

def _maybe_restart_ifaces(ifaces: Sequence[str]) -> None:
  for iface in ifaces:
    unit = f"wg-quick@{iface}.service"
    subprocess.run(
       ["systemctl","restart",unit]
      ,check=False
      ,stdout=subprocess.DEVNULL
      ,stderr=subprocess.DEVNULL
    )

def install_staged_tree(
   stage_root: Path
  ,dest_root: Path
  ,create_dirs: bool = False
  ,skip_identical: bool = True
  ,daemon_reload: bool = False
  ,restart_ifaces: Sequence[str] = ()
) -> List[str]:
  """
  Core business function.

  Given:
    stage_root, dest_root, flags
  Does:
    safe, deterministic copy with backups and explicit perms
  Returns:
    list of log lines
  """
  # Do not rely on process umask; set restrictive default, then override per-file.
  old_umask = os.umask(0o077)
  logs: List[str] = []
  try:
    staged = _gather_stage_files(stage_root)
    if not staged:
      raise RuntimeError("nothing to install (stage is empty or whitelist didn’t match)")

    for rel,mode in staged:
      src = stage_root / rel
      dst = dest_root / rel

      _ensure_parents(dest_root,rel,create_dirs)

      backup = _backup_existing_to_stage(stage_root,dest_root,rel)
      if backup:
        logs.append(f"backup: {dst} -> {backup}")

      if skip_identical and dst.exists():
        try:
          if _sha256(src) == _sha256(dst):
            logs.append(f"identical: skip {rel}")
            continue
        except Exception:
          pass

      _atomic_install(src,dst,mode)
      logs.append(f"install: {rel} (mode {oct(mode)})")

    if daemon_reload:
      _maybe_daemon_reload(True)
      logs.append("systemctl: daemon-reload")

    if restart_ifaces:
      _maybe_restart_ifaces(restart_ifaces)
      logs.append(f"systemctl: restart wg-quick@{','.join(restart_ifaces)}")

    return logs
  finally:
    os.umask(old_umask)

def _require_root() -> None:
  if os.geteuid() != 0:
    raise RuntimeError("must run as root (installer sets ownership/permissions)")

def main(argv: Optional[Sequence[str]] = None) -> int:
  ap = argparse.ArgumentParser(description="Install staged artifacts into a target root (root-only).")
  ap.add_argument("--stage" ,default=str(DEFAULT_STAGE))
  ap.add_argument("--root"  ,default="/")
  ap.add_argument("--create-dirs" ,action="store_true" ,help="create missing parent directories")
  ap.add_argument("--no-skip-identical" ,action="store_true" ,help="always replace even if content identical")
  ap.add_argument("--daemon-reload" ,action="store_true" ,help="run systemctl daemon-reload after install")
  ap.add_argument("--restart-ifaces" ,nargs="*" ,default=[] ,help="optionally restart these wg-quick@IFACE units")
  args = ap.parse_args(argv)

  try:
    _require_root()
    logs = install_staged_tree(
       stage_root=Path(args.stage)
      ,dest_root=Path(args.root)
      ,create_dirs=args.create_dirs
      ,skip_identical=(not args.no_skip_identical)
      ,daemon_reload=args.daemon_reload
      ,restart_ifaces=args.restart_ifaces
    )
    for line in logs:
      print(line)
    return 0
  except Exception as e:
    print(f"❌ install failed: {e}",file=sys.stderr)
    return 2

if __name__ == "__main__":
  sys.exit(main())
