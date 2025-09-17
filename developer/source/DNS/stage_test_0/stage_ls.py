#!/usr/bin/env -S python3 -B
"""
stage_ls.py — execute staged Python programs with Stage in 'noop' mode and list metadata.

For each *.py under --stage (recursively, excluding Stage.py), this tool:
  1) loads Stage.py from the stage root,
  2) switches mode to 'noop' (no side effects, no printing),
  3) executes the program via runpy.run_path(...) with the proper __file__,
  4) collects the resolved write_file_* metadata and declared ops,
  5) prints either list or aligned table,
  6) reports any collected errors.

This lets admins compute metadata with arbitrary Python while guaranteeing no writes.
"""

from __future__ import annotations

import sys ,os
sys.dont_write_bytecode = True
os.environ.setdefault("PYTHONDONTWRITEBYTECODE" ,"1")

from dataclasses import dataclass
from pathlib import Path
import argparse
import importlib.util ,runpy
import traceback

# --- utility dataclass (for printing) ---

@dataclass
class Row:
  read_rel: Path
  owner: str|None
  perm: str|None
  write_name: str|None
  target_dir: Path|None
  ops: list[str]
  errors: list[str]

# --- helpers ---

def _load_stage_module(stage_root: Path):
  """Load Stage.py from stage_root into sys.modules['Stage'] (overwriting if present). Returns the Stage module."""
  stage_py = stage_root/"Stage.py"
  if not stage_py.exists():
    raise FileNotFoundError(f"Stage.py not found at {stage_py} — place Stage.py in the stage root.")
  spec = importlib.util.spec_from_file_location("Stage" ,str(stage_py))
  if spec is None or spec.loader is None:
    raise RuntimeError(f"cannot load Stage module from {stage_py}")
  mod = importlib.util.module_from_spec(spec)
  sys.modules["Stage"] = mod
  spec.loader.exec_module(mod)  # type: ignore[union-attr]
  return mod

def _stage_program_paths(stage_root: Path)-> list[Path]:
  rels: list[Path] = []
  for p in stage_root.rglob("*.py"):
    if p.name == "Stage.py":
      continue
    try:
      if p.is_file():
        rels.append(p.relative_to(stage_root))
    except Exception:
      continue
  return sorted(rels ,key=lambda x: x.as_posix())

def print_list(rows: list[Row])-> None:
  for r in rows:
    owner = r.owner or "?"
    perm  = r.perm or "????"
    name  = r.write_name or "?"
    tdir  = str(r.target_dir) if r.target_dir is not None else "?"
    print(f"{r.read_rel.as_posix()}: {owner} {perm} {name} {tdir}")

def print_table(rows: list[Row])-> None:
  if not rows:
    return
  a = [r.read_rel.as_posix() for r in rows]
  b = [(r.owner or "?") for r in rows]
  c = [(r.perm or "????") for r in rows]
  d = [(r.write_name or "?") for r in rows]
  e = [str(r.target_dir) if r.target_dir is not None else "?" for r in rows]
  wa = max(len(s) for s in a)
  wb = max(len(s) for s in b)
  wc = max(len(s) for s in c)
  wd = max(len(s) for s in d)
  for sa ,sb ,sc ,sd ,se in zip(a ,b ,c ,d ,e):
    print(f"{sa:<{wa}}  {sb:<{wb}}  {sc:<{wc}}  {sd:<{wd}}  {se}")

# --- core ---

def ls_stage(stage_root: Path ,fmt: str="list")-> int:
  Stage = _load_stage_module(stage_root)
  Stage.Stage.set_mode("noop")  # hard safety for this tool

  rows: list[Row] = []
  errs: list[str] = []

  for rel in _stage_program_paths(stage_root):
    abs_path = stage_root/rel
    try:
      # isolate per-run state
      Stage.Stage._current = None
      Stage.Stage._all_records.clear()
      Stage.Stage._begin(read_rel=rel ,stage_root=stage_root)

      # execute the staged program under its real path
      runpy.run_path(str(abs_path) ,run_name="__main__")

      rec = Stage.Stage._end()
      if rec is None:
        errs.append(f"{rel}: program executed but Stage.init(...) was never called")
        continue

      rows.append(
        Row(
          read_rel=rel
        , owner=rec.owner
        , perm=rec.perm_octal_str
        , write_name=rec.write_name
        , target_dir=rec.target_dir
        , ops=list(rec.ops)
        , errors=list(rec.errors)
        )
      )

    except SystemExit as e:
      errs.append(f"{rel}: program called sys.exit({e.code}) during listing")
    except Exception:
      tb = traceback.format_exc(limit=2)
      errs.append(f"{rel}: exception during execution:\n{tb}")

  # print data
  if fmt == "table":
    print_table(rows)
  else:
    print_list(rows)

  # print per-row Stage errors
  row_errs = [f"{r.read_rel}: {msg}" for r in rows for msg in r.errors]
  all_errs = row_errs + errs
  if all_errs:
    print("\nerror(s):" ,file=sys.stderr)
    for e in all_errs:
      print(f"  - {e}" ,file=sys.stderr)
    return 1
  return 0

# --- CLI ---

def main(argv: list[str] | None=None)-> int:
  import argparse
  ap = argparse.ArgumentParser(
    prog="stage_ls.py"
  , description="Execute staged Python configs with Stage in 'noop' mode and list resolved metadata."
  )
  ap.add_argument("--stage" ,default="stage" ,help="stage directory (default: ./stage)")
  ap.add_argument("--format" ,choices=["list","table"] ,default="list" ,help="output format")
  args = ap.parse_args(argv)

  stage_root = Path(args.stage)
  if not stage_root.exists() or not stage_root.is_dir():
    print(f"error: stage directory not found or not a directory: {stage_root}" ,file=sys.stderr)
    return 2

  return ls_stage(stage_root ,fmt=args.format)

if __name__ == "__main__":
  sys.exit(main())
