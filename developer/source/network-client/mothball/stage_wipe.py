#!/usr/bin/env python3
# stage_wipe.py — safely wipe ./stage contents (keeps hidden files by default)
# Usage:
#   ./stage_wipe.py [--yes] [--dry-run] [--hard]
#
# Notes:
# - Default (no --hard): removes ONLY non-hidden entries in ./stage, keeps dotfiles like .gitignore.
# - --hard: removes the stage directory itself (this will remove hidden files as well), then recreates it.

from __future__ import annotations
import argparse, shutil, sys, subprocess
from pathlib import Path

def stage_root() -> Path:
  return Path(__file__).resolve().parent / "stage"

def human_count_and_size(p: Path) -> tuple[int, str]:
  try:
    count = sum(1 for _ in p.rglob("*"))
  except Exception:
    count = 0
  try:
    cp = subprocess.run(["du", "-sh", p.as_posix()], text=True, capture_output=True)
    size = cp.stdout.split()[0] if cp.returncode == 0 and cp.stdout else "?"
  except Exception:
    size = "?"
  return count, size

def wipe(yes: bool, dry_run: bool, hard: bool) -> int:
  st = stage_root()
  if not st.exists():
    print(f"Nothing to wipe: {st} does not exist.")
    return 0

  # Path safety guard
  safe_root = Path(__file__).resolve().parent / "stage"
  if st.resolve() != safe_root.resolve():
    print(f"Refusing: STAGE path looks unsafe: {st}", file=sys.stderr)
    return 1

  count, size = human_count_and_size(st)

  if dry_run:
    if hard:
      print(f"DRY RUN — would remove the entire directory: {st} (items: {count}, size: ~{size})")
    else:
      print(f"DRY RUN — would remove NON-HIDDEN contents of: {st} (items: {count}, size: ~{size})")
      for p in sorted(st.iterdir()):
        if not p.name.startswith('.'):
          print("  " + p.as_posix())
    return 0

  if not yes:
    prompt = f"Permanently delete {'ALL of ' if hard else 'non-hidden entries in '}{st} (items: {count}, size: ~{size})? [y/N] "
    try:
      ans = input(prompt).strip()
    except EOFError:
      ans = ""
    if ans.lower() not in ("y", "yes"):
      print("Aborted.")
      return 0

  if hard:
    # Remove entire stage directory (hidden files included), then recreate it
    try:
      shutil.rmtree(st)
      print(f"Removed stage dir: {st}")
    except Exception as e:
      print(f"WARN: rmtree failed: {e}", file=sys.stderr)
    st.mkdir(parents=True, exist_ok=True)
  else:
    # Remove only non-hidden entries; keep dotfiles like .gitignore
    for p in list(st.iterdir()):
      if p.name.startswith('.'):
        continue  # preserve hidden files/dirs
      try:
        if p.is_dir():
          shutil.rmtree(p)
        else:
          p.unlink(missing_ok=True)
      except Exception as e:
        print(f"WARN: failed to remove {p}: {e}", file=sys.stderr)
    print(f"Cleared non-hidden contents of: {st}")

  print("✅ Done.")
  return 0

def main(argv):
  ap = argparse.ArgumentParser(description="Wipe the stage directory (keeps hidden files unless --hard).")
  ap.add_argument("--yes", action="store_true", help="do not prompt")
  ap.add_argument("--dry-run", action="store_true", help="show what would be removed, then exit")
  ap.add_argument("--hard", action="store_true", help="remove the stage dir itself (also removes hidden files)")
  args = ap.parse_args(argv)
  return wipe(args.yes, args.dry_run, args.hard)

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
