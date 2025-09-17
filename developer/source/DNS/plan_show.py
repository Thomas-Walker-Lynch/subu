#!/usr/bin/env -S python3 -B
"""
plan_show.py — build and display a staged plan (UNPRIVILEGED).

Given: a stage directory of config scripts (*.stage.py by default).
Does:  executes each script with a pre-created Planner (P) and PlannerContext,
       aggregates Commands into a single Journal, by default prints from the CBOR
       round-trip (encode→decode) so the human view matches what will be shipped
       to stage_cp; runs well-formed (WF) invariant checks. Can emit CBOR if requested.
Returns: exit status 0 on success; 2 on WF errors or usage errors.
"""

from __future__ import annotations

# no bytecode anywhere
import sys ,os
sys.dont_write_bytecode = True
os.environ.setdefault("PYTHONDONTWRITEBYTECODE" ,"1")

from pathlib import Path
import argparse
import datetime as _dt
import getpass
import runpy

# local module (same dir): Planner
from Planner import Planner ,PlannerContext ,Journal ,Command

# ===== Utilities (general / reusable) =====

def iso_utc_now_str()-> str:
  "Given n/a. Does return compact UTC timestamp. Returns YYYYMMDDTHHMMSSZ."
  return _dt.datetime.utcnow().strftime("%Y%m%dT%H%M%SZ")

def find_configs(stage_root_dpath: Path ,glob_pat_str: str)-> list[Path]:
  "Given stage root and glob. Does find matching files under stage. Returns list of absolute Paths."
  root = stage_root_dpath.resolve()
  return sorted((p for p in root.glob(glob_pat_str) if p.is_file()) ,key=lambda p: p.as_posix())

def human_size(n: int)-> str:
  "Given byte count. Does format human size. Returns string."
  units = ["B","KB","MB","GB","TB"]
  i = 0
  x = float(max(0 ,n))
  while x >= 1024 and i < len(units)-1:
    x /= 1024.0
    i += 1
  return f"{x:.1f} {units[i]}"

def _dst_path_str(args_map: dict)-> str:
  "Given args map. Does join write_file path. Returns POSIX path or '?'."
  d = args_map.get("write_file_dpath_str") or ""
  f = args_map.get("write_file_fname_str") or ""
  try:
    if d and f and "/" not in f:
      return (Path(d)/f).as_posix()
  except Exception:
    pass
  return "?"

# ===== WF invariants (MVP) =====
# These are “well-formedness” rules (shape/encoding/domain), not policy or privilege checks.

def wf_check(journal: Journal)-> list[str]:
  """
  Given Journal. Does run invariant checks on meta and each Command entry. Returns list of error strings.
  Invariants (MVP):
    - meta_map: must include generator identity and stage_root_dpath_str.
    - entry.op ∈ {'copy','displace','delete'}
    - all ops: write_file_dpath_str absolute; write_file_fname_str is bare filename.
    - copy: owner_name_str non-empty; mode_int ∈ [0..0o7777] and no suid/sgid; content_bytes present (bytes).
  """
  errs: list[str] = []
  meta = journal.meta_map or {}

  # meta presence (light placeholder)
  if not isinstance(meta ,dict):
    errs.append("WF_META: meta_map must be a map")
  else:
    if not meta.get("stage_root_dpath_str"):
      errs.append("WF_META: missing stage_root_dpath_str")
    if not meta.get("generator_prog_str"):
      errs.append("WF_META: missing generator_prog_str")

  # entries
  for idx ,cmd in enumerate(journal.commands_list ,1):
    prefix = f"WF[{idx:02d}]"
    if not isinstance(cmd ,Command):
      errs.append(f"{prefix}: entry is not Command")
      continue
    op = cmd.name_str
    if op not in {"copy","displace","delete"}:
      errs.append(f"{prefix}: unknown op '{op}'")
      continue
    am = cmd.args_map or {}
    dpath = am.get("write_file_dpath_str")
    fname = am.get("write_file_fname_str")
    if not isinstance(dpath ,str) or not dpath.startswith("/"):
      errs.append(f"{prefix}: write_file_dpath_str must be absolute")
    if not isinstance(fname ,str) or not fname or "/" in fname:
      errs.append(f"{prefix}: write_file_fname_str must be a bare filename")

    if op == "copy":
      owner = am.get("owner_name_str")
      mode  = am.get("mode_int")
      data  = am.get("content_bytes")
      if not isinstance(owner ,str) or not owner.strip():
        errs.append(f"{prefix}: owner_name_str must be non-empty")
      if not isinstance(mode ,int) or not (0 <= mode <= 0o7777):
        errs.append(f"{prefix}: mode_int must be int in [0..0o7777]")
      elif (mode & 0o6000):
        errs.append(f"{prefix}: mode_int suid/sgid not allowed in MVP")
      if not isinstance(data ,(bytes,bytearray)):
        errs.append(f"{prefix}: content_bytes must be bytes")
  return errs

# ===== Planner execution =====

def _run_one_config(config_abs_fpath: Path ,stage_root_dpath: Path)-> Planner:
  """
  Given abs path to a config script and stage root. Does construct a PlannerContext and Planner,
  then executes the script with 'P' (Planner instance) bound in globals. Returns Planner with Journal.
  Notes:
    - Defaults are intentionally spartan; config should refine them via P.set_context(...).
    - This is UNPRIVILEGED; no filesystem changes are performed here.
  """
  read_rel = config_abs_fpath.resolve().relative_to(stage_root_dpath.resolve())
  ctx = PlannerContext.from_values(
    stage_root_dpath=stage_root_dpath
    ,read_file_rel_fpath=read_rel
    ,write_file_dpath_str="/"
    ,write_file_fname_str="."
    ,owner_name_str=getpass.getuser()
    ,perm=0o644
    ,content=None
  )
  P = Planner(ctx)
  g = {"Planner": Planner ,"PlannerContext": PlannerContext ,"P": P}
  runpy.run_path(str(config_abs_fpath) ,init_globals=g)
  return P

def _aggregate_journal(planners_list: list[Planner] ,stage_root_dpath: Path)-> Journal:
  "Given planners and stage root. Does aggregate Commands into a single Journal with meta. Returns Journal."
  J = Journal()
  J.set_meta(
    version_int=1
    ,generator_prog_str="plan_show.py"
    ,generated_at_utc_str=iso_utc_now_str()
    ,user_name_str=getpass.getuser()
    ,host_name_str=os.uname().nodename if hasattr(os ,"uname") else "unknown"
    ,stage_root_dpath_str=str(stage_root_dpath.resolve())
    ,configs_list=[p.context().read_file_rel_fpath.as_posix() for p in planners_list]
  )
  for p in planners_list:
    for cmd in p.journal().commands_list:
      J.append(cmd)
  return J

def _print_plan(journal: Journal)-> None:
  "Given Journal. Does print a readable summary. Returns None."
  meta = journal.meta_map or {}
  print(f"Stage: {meta.get('stage_root_dpath_str','?')}")
  print(f"Generated: {meta.get('generated_at_utc_str','?')} by {meta.get('user_name_str','?')}@{meta.get('host_name_str','?')}\n")

  entries = journal.commands_list
  if not entries:
    print("(plan is empty)")
    return

  n_copy = sum(1 for c in entries if c.name_str=="copy")
  n_disp = sum(1 for c in entries if c.name_str=="displace")
  n_del  = sum(1 for c in entries if c.name_str=="delete")
  print(f"Entries: {len(entries)}  copy:{n_copy}  displace:{n_disp}  delete:{n_del}\n")

  for i ,cmd in enumerate(entries ,1):
    am = cmd.args_map
    dst = _dst_path_str(am)
    if cmd.name_str == "copy":
      size = len(am.get("content_bytes") or b"")
      mode = am.get("mode_int")
      owner = am.get("owner_name_str")
      print(f"{i:02d}. copy     -> {dst}  mode {mode:04o} owner {owner} bytes {size} ({human_size(size)})")
    elif cmd.name_str == "displace":
      print(f"{i:02d}. displace -> {dst}")
    elif cmd.name_str == "delete":
      print(f"{i:02d}. delete   -> {dst}")
    else:
      print(f"{i:02d}. ?op?     -> {dst}")

def _maybe_emit_CBOR(journal: Journal ,emit_CBOR_fpath: Path|None)-> None:
  "Given Journal and optional path. Does write CBOR if requested. Returns None."
  if not emit_CBOR_fpath:
    return
  try:
    data = journal.to_CBOR_bytes(canonical_bool=True)
  except Exception as e:
    print(f"error: CBOR encode failed: {e}" ,file=sys.stderr)
    raise
  emit_CBOR_fpath.parent.mkdir(parents=True ,exist_ok=True)
  with open(emit_CBOR_fpath ,"wb") as fh:
    fh.write(data)
  print(f"\nWrote CBOR plan: {emit_CBOR_fpath}  ({len(data)} bytes)")

# ===== CLI =====

def main(argv: list[str]|None=None)-> int:
  "Given CLI. Does discover configs, build plan, (optionally) CBOR round-trip before printing, run WF, optionally emit CBOR. Returns exit code."
  ap = argparse.ArgumentParser(prog="plan_show.py"
   ,description="Build and show a staged plan (no privilege, no apply).")
  ap.add_argument("--stage",default="stage",help="stage directory root (default: ./stage)")
  ap.add_argument("--glob",default="**/*.stage.py",help="glob for config scripts under --stage")
  ap.add_argument("--emit-CBOR",default=None,help="write CBOR plan to this path (optional)")
  ap.add_argument("--print-from-journal",action="store_true"
                 ,help="print directly from in-memory Journal (skip CBOR round-trip)")
  args = ap.parse_args(argv)

  stage_root_dpath = Path(args.stage)
  if not stage_root_dpath.is_dir():
    print(f"error: --stage not a directory: {stage_root_dpath}" ,file=sys.stderr)
    return 2

  configs = find_configs(stage_root_dpath ,args.glob)
  if not configs:
    print("No config scripts found.")
    return 0

  planners: list[Planner] = []
  for cfg in configs:
    try:
      planners.append(_run_one_config(cfg ,stage_root_dpath))
    except SystemExit:
      raise
    except Exception as e:
      print(f"error: executing {cfg}: {e}" ,file=sys.stderr)
      return 2

  journal_src = _aggregate_journal(planners ,stage_root_dpath)

  if not args.print_from_journal:
    try:
      cbor_bytes = journal_src.to_CBOR_bytes(canonical_bool=True)
      journal = Journal.from_CBOR_bytes(cbor_bytes)
    except Exception as e:
      print(f"error: CBOR round-trip failed: {e}" ,file=sys.stderr)
      return 2
  else:
    journal = journal_src

  _print_plan(journal)

  errs = wf_check(journal)
  if errs:
    print("\nerror(s):" ,file=sys.stderr)
    for e in errs:
      print(f"  - {e}" ,file=sys.stderr)
    return 2

  emit = Path(args.emit_CBOR) if args.emit_CBOR else None
  if emit:
    try:
      data = (cbor_bytes if not args.print_from_journal else journal_src.to_CBOR_bytes(canonical_bool=True))
      emit.parent.mkdir(parents=True ,exist_ok=True)
      with open(emit ,"wb") as fh:
        fh.write(data)
      print(f"\nWrote CBOR plan: {emit}  ({len(data)} bytes)")
    except Exception as e:
      print(f"error: failed to write CBOR: {e}" ,file=sys.stderr)
      return 2

  return 0

if __name__ == "__main__":
  sys.exit(main())
