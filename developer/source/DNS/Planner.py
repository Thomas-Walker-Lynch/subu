#!/usr/bin/env -S python3 -B
"""
Planner.py — plan builder for staged configuration (UNPRIVILEGED).

The Planner accumulates Command objects into a Journal.

Journal building is orchestrated by the outer runner (e.g., stage_show_plan, stage_cp)
which constructs a Planner per config file and invokes Planner command methods.

Defaults and provenance come from a PlannerContext instance. You can replace the
context at any time via set_context(ctx).

The Journal can be exported as CBOR via Journal.to_CBOR_bytes(), and reconstructed
on the privileged side via Journal.from_CBOR_bytes().

On-wire field names are snake_case and use explicit suffixes (_str,_bytes,_int, etc.)
to avoid ambiguity.
"""

from __future__ import annotations

# no bytecode anywhere (works under sudo/root shells too)
import sys ,os
sys.dont_write_bytecode = True
os.environ.setdefault("PYTHONDONTWRITEBYTECODE","1")

from dataclasses import dataclass ,field
from pathlib import Path
from typing import Any

# ===== Utilities =====

def _norm_perm(value: int|str)-> tuple[int,str]|None:
  "Given int or 4-char octal string. Does validate/normalize. Returns (int,'%04o') or None."
  if isinstance(value ,int):
    if 0 <= value <= 0o7777:
      return value ,f"{value:04o}"
    return None
  if isinstance(value ,str):
    s = value.strip()
    if len(s)==4 and all(ch in "01234567" for ch in s):
      try:
        v = int(s ,8)
        return v ,s
      except Exception:
        return None
  return None

def _is_abs_dpath(dpath_str: str)-> bool:
  "Given path string. Does quick abs dir check. Returns bool."
  return bool(dpath_str) and dpath_str.startswith("/")

def _join_write_file(dpath_str: str ,fname_str: str)-> str:
  "Given dir path string and filename string. Does join. Returns POSIX path string or ''."
  if not _is_abs_dpath(dpath_str): return ""
  if not fname_str or "/" in fname_str: return ""
  return (Path(dpath_str)/fname_str).as_posix()

# ===== Core data types =====

@dataclass(slots=True)
class Command:
  """
  Command — a single planned operation.

  Given a command name and an argument map (native values).
  Does hold the op name, owns a distinct args map, accumulates errors for this op.
  Returns serializable mapping via to_map().
  """
  name_str: str
  args_map: dict[str,Any] = field(default_factory=dict)
  errors_list: list[str] = field(default_factory=list)

  def add_error(self ,msg_str: str)-> None:
    "Given message. Does append to errors_list. Returns None."
    self.errors_list.append(msg_str)

  def to_map(self)-> dict[str,Any]:
    "Given self. Does convert to a plain dict. Returns {'op','args_map','errors_list'}."
    return {
      "op": self.name_str
      ,"args_map": dict(self.args_map)
      ,"errors_list": list(self.errors_list)
    }

@dataclass(slots=True)
class PlannerContext:
  """
  PlannerContext — per-config provenance and defaults.

  Given: stage_root_dpath, read_file_rel_fpath, default write_file location/name,
         default owner name, default permission (int or '0644'), optional default content.
  Does:  provide ambient defaults and provenance to Planner methods.
  Returns: n/a (data holder).
  """
  stage_root_dpath: Path
  read_file_rel_fpath: Path
  default_write_file_dpath_str: str
  default_write_file_fname_str: str
  default_owner_name_str: str
  default_mode_int: int|None = None
  default_mode_octal_str: str|None = None
  default_content_bytes: bytes|None = None

  @staticmethod
  def from_values(stage_root_dpath: Path
                  ,read_file_rel_fpath: Path
                  ,write_file_dpath_str: str
                  ,write_file_fname_str: str
                  ,owner_name_str: str
                  ,perm: int|str
                  ,content: bytes|str|None
  )-> PlannerContext:
    "Given raw values. Does normalize perm and content. Returns PlannerContext."
    if isinstance(content ,str):
      content_b = content.encode("utf-8")
    else:
      content_b = content
    perm_norm = _norm_perm(perm)
    if perm_norm is None:
      m_int ,m_oct = None ,None
    else:
      m_int ,m_oct = perm_norm
    return PlannerContext(
      stage_root_dpath=stage_root_dpath
      ,read_file_rel_fpath=read_file_rel_fpath
      ,default_write_file_dpath_str=write_file_dpath_str
      ,default_write_file_fname_str=write_file_fname_str
      ,default_owner_name_str=owner_name_str
      ,default_mode_int=m_int
      ,default_mode_octal_str=m_oct
      ,default_content_bytes=content_b
    )

@dataclass(slots=True)
class Journal:
  """
  Journal — ordered list of Commands plus provenance metadata.

  Given optional meta map.
  Does append commands, expose entries, produce plain or CBOR encodings, and rebuild from CBOR.
  Returns plain dict via to_map(), bytes via to_CBOR_bytes(), Journal via from_CBOR_bytes().
  """
  meta_map: dict[str,Any] = field(default_factory=dict)
  commands_list: list[Command] = field(default_factory=list)

  def set_meta(self ,**kv)-> None:
    "Given keyword meta. Does merge into meta_map. Returns None."
    self.meta_map.update(kv)

  def append(self ,cmd: Command)-> None:
    "Given Command. Does append to commands_list. Returns None."
    self.commands_list.append(cmd)

  def entries_list(self)-> list[dict[str,Any]]:
    "Given n/a. Does return list of entry dicts (copy). Returns list[dict]."
    return [c.to_map() for c in self.commands_list]

  def to_map(self)-> dict[str,Any]:
    "Given n/a. Does package a plan map (ready for CBOR). Returns dict."
    return {
      "version_int": 1
      ,"meta_map": dict(self.meta_map)
      ,"entries_list": self.entries_list()
    }

  def to_CBOR_bytes(self ,canonical_bool: bool=True)-> bytes:
    "Given n/a. Does CBOR-encode to bytes (requires cbor2). Returns bytes."
    try:
      import cbor2
    except Exception as e:
      raise RuntimeError(f"package cbor2 required for to_CBOR_bytes: {e}")
    return cbor2.dumps(self.to_map() ,canonical=canonical_bool)

  @staticmethod
  def from_CBOR_bytes(data_bytes: bytes)-> Journal:
    "Given CBOR bytes. Does decode and rebuild a Journal (Commands + meta). Returns Journal."
    try:
      import cbor2
    except Exception as e:
      raise RuntimeError(f"package cbor2 required for from_CBOR_bytes: {e}")
    obj = cbor2.loads(data_bytes)
    if not isinstance(obj ,dict): raise ValueError("CBOR root must be a map")
    meta = dict(obj.get("meta_map") or {})
    entries = obj.get("entries_list") or []
    j = Journal(meta_map=meta)
    for e in entries:
      if not isinstance(e ,dict): continue
      op = e.get("op") or "?"
      args = e.get("args_map") or {}
      errs = e.get("errors_list") or []
      j.append(Command(name_str=op ,args_map=dict(args) ,errors_list=list(errs)))
    return j

# ===== Planner =====

class Planner:
  """
  Planner — constructs a Journal of Commands from config scripts.

  Given: PlannerContext (provenance + defaults).
  Does:  maintains a Journal; command methods (copy/displace/delete) create Command objects,
         fill missing args from context defaults, preflight minimal shape checks, then append.
  Returns: accessors for Journal and meta; no I/O or privilege here.
  """
  def __init__(self ,ctx: PlannerContext)-> None:
    self._ctx = ctx
    self._journal = Journal()
    # seed provenance; outer tools can add more later
    self._journal.set_meta(
      source_read_file_rel_fpath_str=ctx.read_file_rel_fpath.as_posix()
      ,stage_root_dpath_str=str(ctx.stage_root_dpath)
    )

  # --- Context management ---

  def set_context(self ,ctx: PlannerContext)-> None:
    "Given PlannerContext. Does replace current context. Returns None."
    self._ctx = ctx

  def context(self)-> PlannerContext:
    "Given n/a. Does return current context. Returns PlannerContext."
    return self._ctx

  # --- Journal access ---

  def journal(self)-> Journal:
    "Given n/a. Does return the Journal (live). Returns Journal."
    return self._journal

  # --- Helpers ---

  def _resolve_write_file(self ,write_file_dpath_str: str|None ,write_file_fname_str: str|None)-> tuple[str,str]:
    "Given optional write_file dpath/fname. Does fill from context; '.' fname → read_file basename. Returns (dpath,fname)."
    dpath_str = write_file_dpath_str if write_file_dpath_str is not None else self._ctx.default_write_file_dpath_str
    fname_str = write_file_fname_str if write_file_fname_str is not None else self._ctx.default_write_file_fname_str
    if fname_str == ".":
      fname_str = self._ctx.read_file_rel_fpath.name
    return dpath_str ,fname_str

  def _resolve_owner(self ,owner_name_str: str|None)-> str:
    "Given optional owner. Does fill from context. Returns owner string."
    return owner_name_str if owner_name_str is not None else self._ctx.default_owner_name_str

  def _resolve_mode(self ,perm: int|str|None)-> tuple[int|None,str|None]:
    "Given optional perm. Does normalize or fall back to context. Returns (mode_int,mode_octal_str)."
    if perm is None:
      return self._ctx.default_mode_int ,self._ctx.default_mode_octal_str
    norm = _norm_perm(perm)
    return (norm if norm is not None else (None ,None))

  def _resolve_content(self ,content: bytes|str|None)-> bytes|None:
    "Given optional content (bytes or str). Does normalize or fall back to context. Returns bytes|None."
    if content is None:
      return self._ctx.default_content_bytes
    if isinstance(content ,str):
      return content.encode("utf-8")
    return content

  # --- Command builders ---

  def copy(self
           ,*
           ,write_file_dpath_str: str|None=None
           ,write_file_fname_str: str|None=None
           ,owner_name_str: str|None=None
           ,perm: int|str|None=None
           ,content: bytes|str|None=None
           ,read_file_rel_fpath: Path|None=None
  )-> Command:
    """
    Given: optional overrides for write_file (dpath,fname,owner,perm), content, and read_file_rel_fpath.
    Does:  build a 'copy' command entry (content is embedded; read_file path kept as provenance).
    Returns: Command (also appended to Journal).
    """
    cmd = Command("copy")
    # resolve basics
    wf_dpath_str ,wf_fname_str = self._resolve_write_file(write_file_dpath_str ,write_file_fname_str)
    owner_str = self._resolve_owner(owner_name_str)
    mode_int ,mode_oct = self._resolve_mode(perm)
    content_b = self._resolve_content(content)
    read_rel = (read_file_rel_fpath if read_file_rel_fpath is not None else self._ctx.read_file_rel_fpath)

    # minimal shape checks (well-formedness, not policy)
    if not _is_abs_dpath(wf_dpath_str):
      cmd.add_error("write_file_dpath_str must be absolute and non-empty")
    if not wf_fname_str or "/" in wf_fname_str:
      cmd.add_error("write_file_fname_str must be a simple filename (no '/')")
    if not owner_str:
      cmd.add_error("owner_name_str must be non-empty")
    if (mode_int ,mode_oct) == (None ,None):
      cmd.add_error("perm must be an int <= 0o7777 or a 4-digit octal string")
    if content_b is None:
      cmd.add_error("content is required for copy() (bytes or str)")

    cmd.args_map.update({
      "write_file_dpath_str": wf_dpath_str
      ,"write_file_fname_str": wf_fname_str
      ,"owner_name_str": owner_str
      ,"mode_int": mode_int
      ,"mode_octal_str": mode_oct
      ,"content_bytes": content_b
      ,"read_file_rel_fpath_str": read_rel.as_posix()
    })
    self._journal.append(cmd)
    return cmd

  def displace(self
               ,*
               ,write_file_dpath_str: str|None=None
               ,write_file_fname_str: str|None=None
  )-> Command:
    """
    Given: optional write_file dpath/fname overrides.
    Does:  build a 'displace' command (rename existing write_file in-place with UTC suffix).
    Returns: Command (appended).
    """
    cmd = Command("displace")
    wf_dpath_str ,wf_fname_str = self._resolve_write_file(write_file_dpath_str ,write_file_fname_str)

    if not _is_abs_dpath(wf_dpath_str):
      cmd.add_error("write_file_dpath_str must be absolute and non-empty")
    if not wf_fname_str or "/" in wf_fname_str:
      cmd.add_error("write_file_fname_str must be a simple filename (no '/')")

    cmd.args_map.update({
      "write_file_dpath_str": wf_dpath_str
      ,"write_file_fname_str": wf_fname_str
    })
    self._journal.append(cmd)
    return cmd

  def delete(self
             ,*
             ,write_file_dpath_str: str|None=None
             ,write_file_fname_str: str|None=None
  )-> Command:
    """
    Given: optional write_file dpath/fname overrides.
    Does:  build a 'delete' command (unlink if present).
    Returns: Command (appended).
    """
    cmd = Command("delete")
    wf_dpath_str ,wf_fname_str = self._resolve_write_file(write_file_dpath_str ,write_file_fname_str)

    if not _is_abs_dpath(wf_dpath_str):
      cmd.add_error("write_file_dpath_str must be absolute and non-empty")
    if not wf_fname_str or "/" in wf_fname_str:
      cmd.add_error("write_file_fname_str must be a simple filename (no '/')")

    cmd.args_map.update({
      "write_file_dpath_str": wf_dpath_str
      ,"write_file_fname_str": wf_fname_str
    })
    self._journal.append(cmd)
    return cmd
