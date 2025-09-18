#!/usr/bin/env -S python3 -B
"""
executor.py — StageHand outer/inner executor (MVP; UNPRIVILEGED for now)

Phase 1 (outer):
  - Build a combined plan by executing each config's `configure(prov, planner, WriteFileMeta)`.
  - Optionally print the plan via Planner.print().
  - Optionally stop.

Phase 2 (inner shim in same program for now; no privilege yet):
  - Encode combined plan to CBOR and pass to inner path.
  - Inner decodes back to a Journal and optionally prints it.
  - Optionally stop.

Discovery:
  - --stage (default: ./stage) points at the stage directory root.
  - By default, *every file* under --stage (recursively) is executed as a config,
    regardless of extension. Editors can still use .py for highlighting; we strip
    only a trailing ".py" to derive prov.read_fname.

"""

from __future__ import annotations

# no bytecode anywhere
import sys, os
sys.dont_write_bytecode = True
os.environ.setdefault("PYTHONDONTWRITEBYTECODE", "1")

from pathlib import Path
import argparse
import getpass
import tempfile
import runpy
import subprocess
import datetime as _dt
import os, fnmatch, stat


# Local module: Planner.py (same directory)
from Planner import (
  Planner, PlanProvenance, WriteFileMeta, Journal, Command,
)

# -------- utilities --------

def iso_utc_now_str() -> str:
  return _dt.datetime.utcnow().strftime("%Y%m%dT%H%M%SZ")

def _split_globs(glob_arg: str) -> list[str]:
    parts = [g.strip() for g in (glob_arg or "").split(",") if g.strip()]
    # Default includes both deep and top-level files
    return parts or ["**/*", "*"]

def find_config_paths(stage_root: Path, glob_arg: str) -> list[Path]:
    """
    Given stage root and comma-glob string, return sorted list of files (regular or symlink).
    Defaults to match ALL files under stage, including top-level ones.
    """
    root = stage_root.resolve()
    patterns = _split_globs(glob_arg)
    out: set[Path] = set()

    for dirpath, dirnames, filenames in os.walk(root, followlinks=False):
        # (optional) prune symlinked dirs to avoid cycles; files can still be symlinks
        dirnames[:] = [d for d in dirnames if not os.path.islink(os.path.join(dirpath, d))]

        for fname in filenames:
            f_abs = Path(dirpath, fname)
            rel = f_abs.relative_to(root).as_posix()
            if any(fnmatch.fnmatch(rel, pat) for pat in patterns):
                try:
                    st = f_abs.lstat()
                    if stat.S_ISREG(st.st_mode) or stat.S_ISLNK(st.st_mode):
                        out.add(f_abs)
                except Exception:
                    # unreadable/broken entries are skipped
                    pass

    return sorted(out, key=lambda p: p.as_posix())



def _run_one_config(config_path: Path, stage_root: Path) -> Planner:
  """Execute a single config's `configure(prov, planner, WriteFileMeta)` and return that config's Planner."""
  prov = PlanProvenance(stage_root=stage_root, config_path=config_path)
  per_planner = Planner(provenance=prov)  # defaults derive from this file's provenance
  env = runpy.run_path(str(config_path))
  fn = env.get("configure")
  if not callable(fn):
    raise RuntimeError(f"{config_path}: missing callable configure(prov, planner, WriteFileMeta)")
  fn(prov, per_planner, WriteFileMeta)
  return per_planner

def _aggregate_into_master(stage_root: Path, planners: list[Planner]) -> Planner:
  """Create a master Planner and copy all Commands from per-config planners into it."""
  # Synthetic provenance for the master planner (used only for display/meta)
  fake_config = stage_root / "(aggregate).py"
  master = Planner(PlanProvenance(stage_root=stage_root, config_path=fake_config))

  # annotate meta
  master.journal().set_meta(
    generator_prog_str="executor.py",
    generated_at_utc_str=iso_utc_now_str(),
    user_name_str=getpass.getuser(),
    host_name_str=os.uname().nodename if hasattr(os, "uname") else "unknown",
    stage_root_dpath_str=str(stage_root.resolve()),
    configs_list=[p._prov.config_rel_fpath.as_posix() for p in planners],
  )

  # copy commands
  out_j = master.journal()
  for p in planners:
    for cmd in p.journal().command_list:
      out_j.append(cmd)  # keep Command objects as-is
  return master

# ----- CBOR “matchbox” (simple wrapper kept local to executor) -----

def _plan_to_cbor_bytes(planner: Planner) -> bytes:
  """Serialize a Planner's Journal to CBOR bytes."""
  try:
    import cbor2
  except Exception as e:
    raise RuntimeError(f"cbor2 is required: {e}")
  plan_dict = planner.journal().as_dictionary()
  return cbor2.dumps(plan_dict, canonical=True)

def _journal_from_cbor_bytes(data: bytes) -> Journal:
  """Rebuild a Journal from CBOR bytes."""
  try:
    import cbor2
  except Exception as e:
    raise RuntimeError(f"cbor2 is required: {e}")
  obj = cbor2.loads(data)
  if not isinstance(obj, dict):
    raise ValueError("CBOR root must be a dict")
  return Journal(plan_dict=obj)

# -------- inner executor (phase 2) --------

def _inner_main(plan_path: Path, phase2_print: bool, phase2_then_stop: bool) -> int:
  """Inner executor path: decode CBOR → Journal; optionally print; (apply TBD)."""
  try:
    data = Path(plan_path).read_bytes()
  except Exception as e:
    print(f"error: failed to read plan file: {e}", file=sys.stderr)
    return 2

  try:
    journal = _journal_from_cbor_bytes(data)
  except Exception as e:
    print(f"error: failed to decode CBOR: {e}", file=sys.stderr)
    return 2

  if phase2_print:
    journal.print()

  if phase2_then_stop:
    return 0

  # (Stage 3 apply would go here; omitted in MVP)
  return 0

# -------- outer executor (phase 1 & handoff) --------

def _outer_main(args) -> int:
  stage_root = Path(args.stage)
  if not stage_root.is_dir():
    print(f"error: --stage not a directory: {stage_root}", file=sys.stderr)
    return 2

  cfgs = find_config_paths(stage_root, args.glob)
  if not cfgs:
    print("No configuration files found.")
    return 0

  # Execute each config into its own planner
  per_planners: list[Planner] = []
  for cfg in cfgs:
    try:
      per_planners.append(_run_one_config(cfg, stage_root))
    except SystemExit:
      raise
    except Exception as e:
      print(f"error: executing {cfg}: {e}", file=sys.stderr)
      return 2

  # Aggregate into a single master planner for printing/CBOR
  master = _aggregate_into_master(stage_root, per_planners)

  if args.phase_1_print:
    master.print()

  if args.phase_1_then_stop:
    return 0

  # Phase 2: encode CBOR and invoke inner path (same script, --inner)
  try:
    cbor_bytes = _plan_to_cbor_bytes(master)
  except Exception as e:
    print(f"error: CBOR encode failed: {e}", file=sys.stderr)
    return 2

  with tempfile.NamedTemporaryFile(prefix="stagehand_plan_", suffix=".cbor", delete=False) as tf:
    tf.write(cbor_bytes)
    plan_path = tf.name

  try:
    cmd = [
      sys.executable,
      str(Path(__file__).resolve()),
      "--inner",
      "--plan", plan_path,
    ]
    if args.phase_2_print:
      cmd.append("--phase-2-print")
    if args.phase_2_then_stop:
      cmd.append("--phase-2-then-stop")

    proc = subprocess.run(cmd)
    return proc.returncode
  finally:
    try:
      os.unlink(plan_path)
    except Exception:
      pass

# -------- CLI --------

def main(argv: list[str] | None = None) -> int:
  ap = argparse.ArgumentParser(
    prog="executor.py",
    description="StageHand outer/inner executor (plan → CBOR → decode).",
  )
  ap.add_argument("--stage", default="stage", help="stage root directory (default: ./stage)")

  ap.add_argument("--glob", default="**/*",
                  help="glob for config scripts under --stage (default: '**/*' = all files)")

  # ap.add_argument("--glob",
  #                 default="**/*",
  #                 help="comma-separated globs under --stage (default: **/*; every file is a config)")

  # Phase-1 (outer) controls
  ap.add_argument("--phase-1-print", action="store_true", help="print master planner (phase 1)")
  ap.add_argument("--phase-1-then-stop", action="store_true", help="stop after phase 1")

  # Phase-2 (inner) controls (outer forwards these to inner)
  ap.add_argument("--phase-2-print", action="store_true", help="print decoded journal (phase 2)")
  ap.add_argument("--phase-2-then-stop", action="store_true", help="stop after phase 2 decode")

  # Inner-only flags (not for users)
  ap.add_argument("--inner", action="store_true", help=argparse.SUPPRESS)
  ap.add_argument("--plan", default=None, help=argparse.SUPPRESS)

  args = ap.parse_args(argv)

  if args.inner:
    if not args.plan:
      print("error: --inner requires --plan <file>", file=sys.stderr)
      return 2
    return _inner_main(Path(args.plan),
                       phase2_print=args.phase_2_print,
                       phase2_then_stop=args.phase_2_then_stop)

  return _outer_main(args)


if __name__ == "__main__":
  sys.exit(main())
