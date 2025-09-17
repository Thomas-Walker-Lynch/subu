#!/usr/bin/env -S python3 -B
"""
stage_cp.py — build a CBOR plan from staged configs; show, validate, and apply with privilege.

Given: a stage root directory.
Does:  (user) run configs → build native plan → WF checks → summarize → encode plan → sudo re-exec
       (root) decode plan → VALID + SANITY → apply ops (displace/copy/delete) safely.
Returns: exit code.

Requires: pip install cbor2
"""
from __future__ import annotations
import sys ,os
sys.dont_write_bytecode = True
os.environ.setdefault("PYTHONDONTWRITEBYTECODE" ,"1")

from pathlib import Path
import argparse ,importlib.util ,runpy ,socket ,getpass ,time ,tempfile ,subprocess ,pwd
from typing import Any
import cbor2

# ---------- small utils ----------

def _load_stage_module(stage_root_dpath: Path):
  "Given: stage root path. Does: load Stage.py as module 'Stage'. Returns: module."
  mod_fpath = stage_root_dpath/"Stage.py"
  if not mod_fpath.exists():
    raise FileNotFoundError(f"Stage.py not found at {mod_fpath}")
  spec = importlib.util.spec_from_file_location("Stage" ,str(mod_fpath))
  mod = importlib.util.module_from_spec(spec)
  sys.modules["Stage"] = mod
  assert spec and spec.loader
  spec.loader.exec_module(mod)  # type: ignore
  return mod

def _config_rel_fpaths(stage_root_dpath: Path)-> list[Path]:
  "Given: stage root. Does: collect *.py (excluding Stage.py) as relative file paths. Returns: list[Path]."
  rel_fpath_list: list[Path] = []
  for p in stage_root_dpath.rglob("*.py"):
    if p.name == "Stage.py": continue
    if p.is_file():
      rel_fpath_list.append(p.relative_to(stage_root_dpath))
  return sorted(rel_fpath_list ,key=lambda x: x.as_posix())

def _sha256_bytes(b: bytes)-> bytes:
  "Given: bytes. Does: sha256. Returns: 32-byte digest."
  return hashlib.sha256(b).digest()

def _dst_fpath_str(dst_dpath_str: str ,dst_fname_str: str)-> str:
  "Given: a directory path string and a filename string. Does: join. Returns: combined POSIX path string."
  if "/" in dst_fname_str:
    return ""  # invalid; WF will flag
  return str((Path(dst_dpath_str)/dst_fname_str))

# ---------- WF / VALID / SANITY ----------

_ALLOWLIST_PREFIXES_LIST = ["/etc" ,"/usr/local" ,"/etc/systemd/system"]

def wf_check(plan_map: dict[str,Any])-> list[str]:
  "Given: plan map. Does: shape/lexical checks only. Returns: list of error strings."
  errs_list: list[str] = []
  if plan_map.get("version_int") != 1:
    errs_list.append("WF_VERSION: unsupported plan version")
  entries_list = plan_map.get("entries_list")
  if not isinstance(entries_list ,list):
    errs_list.append("WF_ENTRIES: 'entries_list' missing or not a list")
    return errs_list
  for i ,e_map in enumerate(entries_list ,1):
    op = e_map.get("op")
    dst_dpath_str = e_map.get("dst_dpath")
    dst_fname_str = e_map.get("dst_fname")
    where = f"entry {i}"
    if op not in ("copy","displace","delete"):
      errs_list.append(f"WF_OP:{where}: invalid op {op!r}")
      continue
    if not isinstance(dst_dpath_str ,str) or not dst_dpath_str:
      errs_list.append(f"WF_DST_DPATH:{where}: dst_dpath missing or not str")
    if not isinstance(dst_fname_str ,str) or not dst_fname_str:
      errs_list.append(f"WF_DST_FNAME:{where}: dst_fname missing or not str")
    if isinstance(dst_fname_str ,str) and "/" in dst_fname_str:
      errs_list.append(f"WF_DST_FNAME:{where}: dst_fname must not contain '/'")
    if isinstance(dst_dpath_str ,str) and not dst_dpath_str.startswith("/"):
      errs_list.append(f"WF_DST_DPATH:{where}: dst_dpath must be absolute")
    full_fpath_str = _dst_fpath_str(dst_dpath_str or "" ,dst_fname_str or "")
    if not full_fpath_str or not full_fpath_str.startswith("/"):
      errs_list.append(f"WF_PATH:{where}: failed to construct absolute path from dst_dpath/fname")
    if op == "copy":
      mode_int = e_map.get("mode_int")
      if not isinstance(mode_int ,int) or not (0 <= mode_int <= 0o7777):
        errs_list.append(f"WF_MODE:{where}: mode_int must be int in [0..0o7777]")
      if isinstance(mode_int ,int) and (mode_int & 0o6000):
        errs_list.append(f"WF_MODE:{where}: suid/sgid bits not allowed in MVP")
      owner_name = e_map.get("owner_name")
      if not isinstance(owner_name ,str) or not owner_name:
        errs_list.append(f"WF_OWNER:{where}: owner_name must be non-empty username string")
      content_bytes = e_map.get("content_bytes")
      if not (isinstance(content_bytes ,(bytes,bytearray)) and len(content_bytes) >= 0):
        errs_list.append(f"WF_CONTENT:{where}: content_bytes must be bytes (may be empty)")
      sha = e_map.get("sha256_bytes")
      if sha is not None:
        if not isinstance(sha ,(bytes,bytearray)) or len(sha)!=32:
          errs_list.append(f"WF_SHA256:{where}: sha256_bytes must be 32-byte digest if present")
        elif isinstance(content_bytes ,(bytes,bytearray)) and sha != _sha256_bytes(content_bytes):
          errs_list.append(f"WF_SHA256_MISMATCH:{where}: sha256_bytes does not match content_bytes")
  return errs_list

def valid_check(plan_map: dict[str,Any])-> list[str]:
  "Given: plan map. Does: environment (read-only) checks. Returns: list of error strings."
  errs_list: list[str] = []
  for i ,e_map in enumerate(plan_map.get("entries_list") or [] ,1):
    op = e_map.get("op")
    dst_fpath_str = _dst_fpath_str(e_map.get("dst_dpath","/") ,e_map.get("dst_fname",""))
    where = f"entry {i}"
    try:
      parent_dpath = Path(dst_fpath_str).parent
      if not parent_dpath.exists():
        errs_list.append(f"VAL_PARENT_MISSING:{where}: parent dir does not exist: {parent_dpath}")
      elif not parent_dpath.is_dir():
        errs_list.append(f"VAL_PARENT_NOT_DIR:{where}: parent is not a directory: {parent_dpath}")
      if Path(dst_fpath_str).is_dir():
        errs_list.append(f"VAL_DST_IS_DIR:{where}: destination exists as a directory: {dst_fpath_str}")
      if op == "copy":
        owner_name = e_map.get("owner_name")
        try:
          pw = pwd.getpwnam(owner_name)  # may raise KeyError
          e_map["_resolved_uid_int"] = pw.pw_uid
          e_map["_resolved_gid_int"] = pw.pw_gid
        except Exception:
          errs_list.append(f"VAL_OWNER_UNKNOWN:{where}: user not found: {owner_name!r}")
    except Exception as x:
      errs_list.append(f"VAL_EXCEPTION:{where}: {x}")
  return errs_list

def sanity_check(plan_map: dict[str,Any])-> list[str]:
  "Given: plan map. Does: policy checks (allowlist, denials). Returns: list of error strings."
  errs_list: list[str] = []
  for i ,e_map in enumerate(plan_map.get("entries_list",[]) ,1):
    dst_fpath_str = _dst_fpath_str(e_map.get("dst_dpath","/") ,e_map.get("dst_fname",""))
    where = f"entry {i}"
    if not any(dst_fpath_str.startswith(pref + "/") or dst_fpath_str==pref for pref in _ALLOWLIST_PREFIXES_LIST):
      errs_list.append(f"POL_PATH_DENY:{where}: destination outside allowlist: {dst_fpath_str}")
  return errs_list

# ---------- APPLY (root) ----------

def _utc_str()-> str:
  "Given: n/a. Does: current UTC compact. Returns: string."
  import datetime as _dt
  return _dt.datetime.utcnow().strftime("%Y%m%dT%H%M%SZ")

def _ensure_parent_dirs(dst_fpath: Path)-> None:
  "Given: destination file path. Does: create parents. Returns: None."
  dst_fpath.parent.mkdir(parents=True ,exist_ok=True)

def _displace_in_place(dst_fpath: Path)-> None:
  "Given: destination file path. Does: rename existing file/symlink to add UTC suffix. Returns: None."
  try:
    if dst_fpath.exists() or dst_fpath.is_symlink():
      suffix = "_" + _utc_str()
      dst_fpath.rename(dst_fpath.with_name(dst_fpath.name + suffix))
  except FileNotFoundError:
    pass

def _apply_copy(dst_fpath: Path ,content_bytes: bytes ,mode_int: int ,uid_int: int ,gid_int: int)-> None:
  "Given: target, bytes, mode, uid, gid. Does: write temp, fsync, chmod/chown, atomic replace. Returns: None."
  _ensure_parent_dirs(dst_fpath)
  _displace_in_place(dst_fpath)
  tmp_fpath = dst_fpath.with_name("." + dst_fpath.name + ".stage_tmp")
  with open(tmp_fpath ,"wb") as fh:
    fh.write(content_bytes)
    fh.flush()
    os.fsync(fh.fileno())
  try:
    os.chmod(tmp_fpath ,mode_int & 0o777)
  except Exception:
    pass
  try:
    os.chown(tmp_fpath ,uid_int ,gid_int)
  except Exception:
    pass
  os.replace(tmp_fpath ,dst_fpath)  # atomic within same dir/device

def _apply_delete(dst_fpath: Path)-> None:
  "Given: target file path. Does: unlink file/symlink if present. Returns: None."
  try:
    if dst_fpath.is_symlink() or dst_fpath.is_file():
      dst_fpath.unlink()
  except FileNotFoundError:
    pass

def apply_plan(plan_map: dict[str,Any] ,dry_run_bool: bool=False)-> int:
  "Given: plan map and dry flag. Does: execute ops sequentially. Returns: exit code."
  for i ,e_map in enumerate(plan_map.get("entries_list") or [] ,1):
    op = e_map.get("op")
    dst_fpath = Path(_dst_fpath_str(e_map.get("dst_dpath","/") ,e_map.get("dst_fname","")))
    if op == "displace":
      print(f"+ displace {dst_fpath}")
      if not dry_run_bool:
        _displace_in_place(dst_fpath)
    elif op == "delete":
      print(f"+ delete   {dst_fpath}")
      if not dry_run_bool:
        _apply_delete(dst_fpath)
    elif op == "copy":
      mode_int = e_map.get("mode_int") or 0o644
      uid_int  = e_map.get("_resolved_uid_int" ,0)
      gid_int  = e_map.get("_resolved_gid_int" ,0)
      content_bytes = e_map.get("content_bytes") or b""
      print(f"+ copy     {dst_fpath}  mode {mode_int:04o} uid {uid_int} gid {gid_int} bytes {len(content_bytes)}")
      if not dry_run_bool:
        _apply_copy(dst_fpath ,content_bytes ,mode_int ,uid_int ,gid_int)
    else:
      print(f"! unknown op {op} (skipping)")
      return 2
  return 0

# ---------- orchestration ----------

def _build_plan_unpriv(stage_root_dpath: Path)-> dict[str,Any]:
  "Given: stage root. Does: execute configs, accumulate entries, add sha256. Returns: plan map."
  StageMod = _load_stage_module(stage_root_dpath)
  Stage = StageMod.Stage
  Stage._reset()
  Stage.set_meta(
    planner_user_name=getpass.getuser()
    ,planner_uid_int=os.getuid()
    ,planner_gid_int=os.getgid()
    ,host_name=socket.gethostname()
    ,created_utc_str=time.strftime("%Y-%m-%dT%H:%M:%SZ",time.gmtime())
  )
  for rel_fpath in _config_rel_fpaths(stage_root_dpath):
    Stage._begin(read_rel_fpath=rel_fpath ,stage_root_dpath=stage_root_dpath)
    runpy.run_path(str(stage_root_dpath/rel_fpath) ,run_name="__main__")
    Stage._end()
  for e_map in Stage.plan_entries():
    if e_map.get("op") == "copy" and isinstance(e_map.get("content_bytes") ,(bytes,bytearray)):
      e_map["sha256_bytes"] = _sha256_bytes(e_map["content_bytes"])
  return Stage.plan_object()

def _sudo_apply_self(plan_fpath: Path ,dry_run_bool: bool)-> int:
  "Given: plan file path and dry flag. Does: sudo re-exec current script with --apply. Returns: exit code."
  cmd_list = ["sudo",sys.executable,os.path.abspath(__file__),"--apply"
             ,"--plan",str(plan_fpath)]
  if dry_run_bool:
    cmd_list.append("--dry-run")
  return subprocess.call(cmd_list)

def main(argv: list[str]|None=None)-> int:
  "Given: CLI. Does: plan, WF (user) then VALID+SANITY+APPLY (root). Returns: exit code."
  ap = argparse.ArgumentParser(prog="stage_cp.py"
   ,description="Plan staged config application and apply with sudo.")
  ap.add_argument("--stage",default="stage",help="stage directory (default: ./stage)")
  ap.add_argument("--dry-run",action="store_true",help="validate and show actions, do not change files")
  ap.add_argument("--apply",action="store_true",help=argparse.SUPPRESS)     # internal (root path)
  ap.add_argument("--plan",default=None,help=argparse.SUPPRESS)             # internal (root path)
  args = ap.parse_args(argv)

  # Root path (apply)
  if args.apply:
    if os.geteuid() != 0:
      print("error: --apply requires root" ,file=sys.stderr)
      return 2
    if not args.plan:
      print("error: --plan path required for --apply" ,file=sys.stderr)
      return 2
    with open(args.plan ,"rb") as fh:
      plan_map = cbor2.load(fh)
    val_errs = valid_check(plan_map)
    pol_errs = sanity_check(plan_map)
    if val_errs or pol_errs:
      print("error(s) during validation/sanity:" ,file=sys.stderr)
      for e in val_errs: print(f"  - {e}" ,file=sys.stderr)
      for e in pol_errs: print(f"  - {e}" ,file=sys.stderr)
      return 2
    rc = apply_plan(plan_map ,dry_run_bool=args.dry_run)
    return rc

  # User path (plan + summarize + escalate)
  stage_root_dpath = Path(args.stage)
  plan_map = _build_plan_unpriv(stage_root_dpath)

  entries_list = plan_map.get("entries_list" ,[])
  print(f"Built plan with {len(entries_list)} entr{'y' if len(entries_list)==1 else 'ies'}")

  total_bytes_int = sum(len(e_map.get("content_bytes") or b"")
                        for e_map in entries_list if e_map.get("op")=="copy")
  print(f"Total bytes to write: {total_bytes_int}")
  if args.dry_run:
    print("\n--dry-run: would perform the following:")

  for i ,e_map in enumerate(entries_list ,1):
    op = e_map.get("op")
    dst_fpath_str = _dst_fpath_str(e_map.get("dst_dpath") ,e_map.get("dst_fname"))
    if op=="copy":
      mode_int = e_map.get("mode_int") or 0o644
      owner_name = e_map.get("owner_name") or "?"
      size = len(e_map.get("content_bytes") or b"")
      print(f"{i:02d}. copy     -> {dst_fpath_str}  mode {mode_int:04o} owner {owner_name} bytes {size}")
    elif op=="displace":
      print(f"{i:02d}. displace -> {dst_fpath_str}")
    elif op=="delete":
      print(f"{i:02d}. delete   -> {dst_fpath_str}")
    else:
      print(f"{i:02d}. ?op?     -> {dst_fpath_str}")

  with tempfile.NamedTemporaryFile(prefix="plan_" ,suffix=".cbor" ,delete=False) as tf:
    cbor2.dump(plan_map ,tf)
    plan_fpath = Path(tf.name)
  try:
    if args.dry_run:
      return _sudo_apply_self(plan_fpath ,dry_run_bool=True)
    ans = input("\nProceed with apply under sudo? [y/N] ").strip().lower()
    if ans not in ("y","yes"):
      print("Aborted.")
      return 0
    return _sudo_apply_self(plan_fpath ,dry_run_bool=False)
  finally:
    try: os.unlink(plan_fpath)
    except Exception: pass

if __name__ == "__main__":
  sys.exit(main())
