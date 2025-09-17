#!/usr/bin/env -S python3 -B
"""
cp_stage.py — copy a staged tree into a root, after showing exactly what will happen.

Behavior:
  - Shows plan (to-copy / ignored) before executing.
  - Requires root (UID 0) by default.
  - Recreates symlinks as symlinks; regular files via shutil.copy2().
  - If a destination exists, it is first renamed in-place to '<name>_YYYYMMDDTHHMMSSZ'.
    The backup preserves the original owner/mode/xattrs.
  - Prints shell-equivalent commands for each operation.
"""

from __future__ import annotations

# no bytecode anywhere (works under sudo/root shells too)
import sys ,os
sys.dont_write_bytecode = True
os.environ.setdefault("PYTHONDONTWRITEBYTECODE" ,"1")

from pathlib import Path
import argparse
import datetime as _dt
import re
import shutil
import shlex as _shlex

# === tiny helpers ===

def _read_file_relative_paths_under(dir_tree_root: Path)-> list[Path]:
  """Return POSIX-relative paths for all file-like entries under `dir_tree_root`.
  Notes:
    - Recurses all levels; includes hidden files.
    - Includes symlinks as entries; does NOT traverse into symlinked directories.
    - Returns paths relative to `dir_tree_root` (no leading slash).
  """
  read_file_relative_path_list: list[Path] = []
  for p in dir_tree_root.rglob("*"):
    try:
      if p.is_symlink() or p.is_file():
        read_file_relative_path_list.append(p.relative_to(dir_tree_root))
    except FileNotFoundError:
      continue
  return sorted(read_file_relative_path_list,key=lambda x: x.as_posix())

def _is_ignored(rel: Path ,ignore_res: list[re.Pattern[str]])-> bool:
  s = rel.as_posix()
  return any(r.search(s) for r in ignore_res)

def _mode_octal(path: Path)-> str:
  try:
    return f"0{(path.stat().st_mode & 0o777):03o}"
  except FileNotFoundError:
    return "0644"

def _cmd_install(src: Path ,dst: Path)-> str:
  return f"install -m {_shlex.quote(_mode_octal(src))} -D {_shlex.quote(str(src))} {_shlex.quote(str(dst))}"

def _cmd_symlink(target: str ,dst: Path)-> str:
  return f"ln -sfn -- {_shlex.quote(target)} {_shlex.quote(str(dst))}"

def _iso_utc()-> str:
  return _dt.datetime.utcnow().strftime("%Y%m%dT%H%M%SZ")

def _maybe_backup(dst: Path ,backups: list[Path])-> None:
  """If dst exists (file or symlink), rename it to dst_<ISO> and record it."""
  try:
    if dst.exists() or dst.is_symlink():
      ts = _iso_utc()
      backup = dst.with_name(dst.name + f"_{ts}")
      dst.rename(backup)  # preserves owner/mode/xattrs
      backups.append(backup)
  except FileNotFoundError:
    pass

def _validate_main_args(stage: Path ,root: Path ,ignore_patterns: list[str] ,require_root: bool)-> list[str]:
  errs: list[str] = []
  if require_root and hasattr(os ,"geteuid") and os.geteuid() != 0:
    errs.append("must run as root (UID 0) to write into the destination root; use sudo")
  if not stage.exists():
    errs.append(f"--stage does not exist: {stage}")
  elif not stage.is_dir():
    errs.append(f"--stage is not a directory: {stage}")
  if not root.exists():
    errs.append(f"--root does not exist: {root}")
  elif not root.is_dir():
    errs.append(f"--root is not a directory: {root}")
  # regex syntax (report all)
  for p in ignore_patterns:
    try:
      re.compile(p)
    except re.error as e:
      errs.append(f"--ignore '{p}': invalid regex: {e}")
  try:
    if stage.resolve() == root.resolve():
      errs.append("--stage and --root resolve to the same path (refusing to copy onto self)")
  except Exception:
    pass
  if not sys.dont_write_bytecode:
    errs.append("internal: bytecode suppression did not engage (sys.dont_write_bytecode is False)")
  return errs


def _read_file_relative_paths_under(dir_tree_root: Path)-> list[Path]:
  """Given:  dir_tree_root.
     Does:   walk all levels under dir_tree_root, collect file-like entries (files or symlinks),
             return their paths relative to dir_tree_root in POSIX form ordering.
     Returns: list of relative Path objects (no leading slash), sorted lexicographically by as_posix().
  """
  read_file_relative_path_list: list[Path] = []
  for p in dir_tree_root.rglob("*"):
    try:
      if p.is_symlink() or p.is_file():
        read_file_relative_path_list.append(p.relative_to(dir_tree_root))
    except FileNotFoundError:
      continue
  return sorted(read_file_relative_path_list,key=lambda x: x.as_posix())

def _select_read_paths(read_file_rel_path_list: list[Path],ignore_patterns: list[re.Pattern[str]]
)-> tuple[list[Path], list[Path]]:
  """Given:  stage-relative read paths, compiled ignore patterns (matched against POSIX-style relpaths).
     Does:   partition into selected vs ignored.
     Returns: (selected_read_rel_paths,ignored_read_rel_paths).
  """
  selected: list[Path] = []
  ignored:  list[Path] = []
  for rel in read_file_rel_path_list:
    (ignored if _is_ignored(rel,ignore_patterns) else selected).append(rel)
  return selected,ignored

def _print_install_plan(stage: Path,root: Path,selected: list[Path],ignored: list[Path]-> None:
  """Given:  stage,root,selected,ignored
     Does:   print plan and notes about displaced-target policy.
     Returns: None.
  """
  print(f"Stage: {stage}")
  print(f"Root:  {root}\n")
  print("If a file already exists at a write point, it is first renamed in place to "
        "'<name>_YYYYMMDDTHHMMSSZ' (UTC); owner/mode/xattrs are preserved.")
  print(f"=== Files to be read ({len(selected)}) ===")
  print("\n".join(p.as_posix() for p in selected) if selected else "(none)")
  print(f"\n=== Files ignored ({len(ignored)}) ===")
  print("\n".join(p.as_posix() for p in ignored) if ignored else "(none)")

def _partition_by_kind(stage: Path,selected_read_rel_paths: list[Path]
)-> tuple[list[Path], list[Path]]:
  """Given:  stage root and selected stage-relative read paths.
     Does:   partition into regular files vs symlinks (based on the staged items).
     Returns: (regular_read_rel_paths,symlink_read_rel_paths).
  """
  regular: list[Path] = []
  symlinks: list[Path] = []
  for rel in selected_read_rel_paths:
    (symlinks if (stage/rel).is_symlink() else regular).append(rel)
  return regular,symlinks

def _rewrite_symlink_target(link_target: str,stage: Path,root: Path)-> tuple[str,bool]:
  """Given:  textual symlink target from a staged link, and the resolved stage/root.
     Does:   if target is absolute and under stage, rewrite to analogous path under root;
             otherwise preserve (absolute outside stage or relative).
     Returns: (new_target_text,is_rewritten).
  """
  try:
    if link_target.startswith("/"):
      stage_str = str(stage)
      if link_target == stage_str or link_target.startswith(stage_str + os.sep):
        rel_str = os.path.relpath(link_target,stage_str)
        return str(root/rel_str),True
      return link_target,False
    else:
      return link_target,False  # relative: preserve
  except Exception:
    return link_target,False

def _copy_regular_files(regular_read_rel_paths: list[Path],stage: Path,root: Path,displaced_targets: list[Path])-> int:
  """Given:  stage-rooted regular read relpaths, stage, root, and displaced_targets accumulator.
     Does:   ensure parents, displace existing targets (rename with UTC suffix), copy via copy2, align mode.
     Returns: number of write operations completed.
  """
  writes = 0
  for rel in regular_read_rel_paths:
    read_abs  = stage/rel
    write_abs = root/rel
    write_abs.parent.mkdir(parents=True,exist_ok=True)
    _maybe_backup(write_abs,displaced_targets)
    print(f"+ {_cmd_install(read_abs,write_abs)}  # (exec: copy2)")
    shutil.copy2(read_abs,write_abs)
    try:
      os.chmod(write_abs,read_abs.stat().st_mode & 0o777)
    except Exception:
      pass
    writes += 1
  return writes

def _install_symlinks(symlink_read_rel_paths: list[Path],stage: Path,root: Path,displaced_targets: list[Path]
)-> tuple[int,list[tuple[Path,str]]]:
  """Given:  staged symlink relpaths, stage, root, and displaced_targets accumulator.
     Does:   ensure parents, displace existing targets, recreate symlinks; rewrite stage-absolute targets to root.
     Returns: (writes_completed,post_link_checks[(installed_path,target_text),...]).
  """
  writes = 0
  post_checks: list[tuple[Path,str]] = []
  for rel in symlink_read_rel_paths:
    read_link_abs  = stage/rel
    write_link_abs = root/rel
    write_link_abs.parent.mkdir(parents=True,exist_ok=True)
    _maybe_backup(write_link_abs,displaced_targets)
    target_txt = os.readlink(read_link_abs)
    new_txt,re_written = _rewrite_symlink_target(target_txt,stage,root)
    note = "  # rewritten from stage-absolute" if re_written else ""
    print(f"+ {_cmd_symlink(new_txt,write_link_abs)}{note}")
    os.symlink(new_txt,write_link_abs)
    writes += 1
    post_checks.append((write_link_abs,new_txt))
  return writes,post_checks

def _report_broken_symlinks(post_link_checks: list[tuple[Path,str]])-> list[Path]:
  """Given:  list of (installed_link_path,target_text).
     Does:   detect links whose targets do not exist at install time; prints warnings.
     Returns: list of broken link paths.
  """
  broken: list[Path] = []
  for link_path,target_txt in post_link_checks:
    try:
      exists = Path(target_txt).exists() if target_txt.startswith("/") else (link_path.parent/target_txt).exists()
      if not exists:
        broken.append(link_path)
    except Exception:
      broken.append(link_path)
  if broken:
    print("\nwarn: the following installed symlinks do not resolve at install time:")
    for p in broken:
      print(f"  - {p}")
  return broken


# === core ===

def cp_stage(
  stage: Path   # directory tree of files to be read, then copied
  ,root: Path   # directory tree of files to be written with root ownership, defaults to '/', hence the name
  ,ignore_patterns: list[re.Pattern[str]]  # match against POSIX-style read *relative* paths
  ,assume_yes: bool=False  # prompt short-circuit
)-> tuple[list[Path], list[Path], int, list[Path]]:
  """
  Given:   stage (read dir tree), root (write dir tree), compiled ignore patterns for POSIX-style relpaths under stage,
           and a non-interactive flag.
  Does:    enumerates stage file-like entries, filters by ignores, shows a plan, optionally prompts, then performs
           writes in two phases: regular files first (copy2), then symlinks (with stage→root rewriting of absolute targets).
           Pre-existing targets are displaced with an in-place rename to '<name>_YYYYMMDDTHHMMSSZ' (UTC).
  Returns: (selected_read_rel_paths,ignored_read_rel_paths,writes_completed_count,displaced_targets_list)
  """
  stage = stage.resolve()
  root  = root.resolve()

  read_file_rel_path_list = _read_file_relative_paths_under(stage)
  selected_read_rel_paths,ignored_read_rel_paths = _select_read_paths(read_file_rel_path_list,ignore_patterns)

  _print_install_plan(stage,root,selected_read_rel_paths,ignored_read_rel_paths)
  if not assume_yes:
    print("")
    ans = input("Proceed with copy? [y/N] ").strip().lower()
    if ans not in ("y","yes"):
      print("Aborted. Nothing copied.")
      return selected_read_rel_paths,ignored_read_rel_paths,0,[]

  regular_read_rel_paths,symlink_read_rel_paths = _partition_by_kind(stage,selected_read_rel_paths)

  displaced_targets: list[Path] = []
  writes_completed = 0

  writes_completed += _copy_regular_files(regular_read_rel_paths,stage,root,displaced_targets)
  link_writes,post_link_checks = _install_symlinks(symlink_read_rel_paths,stage,root,displaced_targets)
  writes_completed += link_writes

  _report_broken_symlinks(post_link_checks)

  print(f"\nDone. Copied {writes_completed} file(s). Backups made: {len(displaced_targets)}.")
  if displaced_targets:
    print("These files were displaced and renamed:")
    for p in displaced_targets:
      print(f"  - {p}")

  return selected_read_rel_paths,ignored_read_rel_paths,writes_completed,displaced_targets


def main(argv: list[str] | None=None)-> int:
  ap = argparse.ArgumentParser(
    prog="cp_stage.py"
    ,description="Copy staged files into a destination root, after showing the exact plan."
  )
  ap.add_argument("--stage" ,default="stage",help="stage directory (default: ./stage)")
  ap.add_argument("--root" , default="/"    ,help="destination root (default: /)")
  ap.add_argument("--ignore",action="append",default=[]
                  ,help="regex matched against POSIX-style relative paths under --stage; can be repeated")

  ap.add_argument("--yes" ,action="store_true" ,help="assume yes; do not prompt")

  args = ap.parse_args(argv)

  stage = Path(args.stage)
  root  = Path(args.root)

  # validate against raw patterns (strings)
  errs = _validate_main_args(stage ,root ,args.ignore ,require_root=True)
  if errs:
    print("error(s):" ,file=sys.stderr)
    for e in errs:
      print(f"  - {e}" ,file=sys.stderr)
    return 2

  # compile patterns
  ignore_res: list[re.Pattern[str]] = [re.compile(p) for p in args.ignore]

  try:
    cp_stage(stage ,root ,ignore_res ,assume_yes=args.yes)
    return 0
  except KeyboardInterrupt:
    print(
      "\nKeyboard interrupt — copy may be left in a partial state.\n"
      "Some targets might have been backed up or replaced; review outputs before retrying.",
      file=sys.stderr,
    )
    return 130
  except Exception as e:
    print(f"error: {e}" ,file=sys.stderr)
    return 2

if __name__ == "__main__":
  sys.exit(main())
