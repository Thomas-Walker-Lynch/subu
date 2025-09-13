#!/usr/bin/env python3
"""
stage_IP_apply_script.py

Given:
  - A SQLite DB (schema you’ve defined), with:
      * Iface(id ,iface ,local_address_cidr ,rt_table_name)
      * v_iface_effective(id ,rt_table_name_eff ,local_address_cidr)
      * Route(iface_id ,cidr ,via ,table_name ,metric ,on_up ,on_down)
      * "User"(iface_id ,username ,uid)  — table formerly User_Binding
      * Meta(key='subu_cidr' ,value)
  - A list of interface names to include (e.g., ["x6","US"]).

Does:
  - Reads DB once and *synthesizes a single* idempotent runtime script
    that, for the selected interfaces, on each `wg-quick@IFACE` start:
      1) ensures IPv4 addresses exist on the iface (if present in DB)
      2) ensures all configured routes exist (using `ip -4 route replace`)
      3) ensures policy rules exist for src-cidr ,uidrange ,and a `prohibit`
  - Stages that script under: stage/usr/local/bin/<script_name>
  - Stages per-iface systemd drop-ins:
      stage/etc/systemd/wg-quick@IFACE.service.d/<prio>-postup-ip-state.conf
    which call the script (default prio = 20).

Returns:
  (script_path ,notes[list of strings])

Errors:
  - Raises RuntimeError if no interfaces provided or there’s nothing to emit.
  - Does not write /etc/iproute2/rt_tables (that’s handled by your registration stager).
  - Does not modify kernel state — this is staging only.

Notes:
  - The generated script is idempotent:
      * addresses: “add if missing”
      * routes:    `ip -4 route replace`
      * rules:     add only if a grep needle is not found
  - It accepts optional IFACE args at runtime to limit application to a subset.
"""

from __future__ import annotations
from pathlib import Path
from typing import Dict ,Iterable ,List ,Optional ,Sequence ,Tuple
import argparse
import sqlite3
import sys

import incommon as ic  # expected: open_db()

ROOT = Path(__file__).resolve().parent
STAGE_ROOT = ROOT / "stage"


# ---------- DB access ----------

def _fetch_meta_subu_cidr(conn: sqlite3.Connection ,default="10.0.0.0/24") -> str:
  row = conn.execute("SELECT value FROM Meta WHERE key='subu_cidr' LIMIT 1;").fetchone()
  return str(row[0]) if row and row[0] else default


def _fetch_iface_meta(conn: sqlite3.Connection ,iface_names: Sequence[str]) -> Dict[str ,Tuple[int ,str ,Optional[str]]]:
  """
  Return {iface_name -> (iface_id ,rt_table_name_eff ,local_address_cidr_or_None)}.
  """
  if not iface_names:
    return {}
  ph = ",".join("?" for _ in iface_names)
  sql = f"""
  SELECT i.id
       , i.iface
       , v.rt_table_name_eff
       , NULLIF(TRIM(v.local_address_cidr),'') AS cidr
  FROM Iface i
  JOIN v_iface_effective v ON v.id = i.id
  WHERE i.iface IN ({ph})
  ORDER BY i.id;
  """
  rows = conn.execute(sql ,tuple(iface_names)).fetchall()
  out: Dict[str ,Tuple[int ,str ,Optional[str]]] = {}
  for r in rows:
    iface_id = int(r[0]); name = str(r[1]); eff = str(r[2]); cidr = (str(r[3]) if r[3] is not None else None)
    out[name] = (iface_id ,eff ,cidr)
  return out


def _fetch_routes_by_iface_id(
  conn: sqlite3.Connection
  ,iface_ids: Sequence[int]
  ,only_on_up: bool = True
) -> Dict[int ,List[Tuple[str ,Optional[str] ,Optional[str] ,Optional[int]]]]:
  """
  Return {iface_id -> [(cidr ,via ,table_name_or_None ,metric_or_None),...]}.
  """
  if not iface_ids:
    return {}
  ph = ",".join("?" for _ in iface_ids)
  sql = f"""
  SELECT iface_id
       , cidr
       , NULLIF(TRIM(via),'')        AS via
       , NULLIF(TRIM(table_name),'') AS table_name
       , metric
       , on_up
  FROM Route
  WHERE iface_id IN ({ph})
  ORDER BY id;
  """
  rows = conn.execute(sql ,tuple(iface_ids)).fetchall()
  out: Dict[int ,List[Tuple[str ,Optional[str] ,Optional[str] ,Optional[int]]]] = {}
  for iface_id ,cidr ,via ,tname ,metric ,on_up in rows:
    if only_on_up and int(on_up) != 1:
      continue
    out.setdefault(int(iface_id) ,[]).append(
      (str(cidr) ,(str(via) if via is not None else None) ,(str(tname) if tname is not None else None)
      ,(int(metric) if metric is not None else None))
    )
  return out


def _fetch_uids_by_iface_id(conn: sqlite3.Connection ,iface_ids: Sequence[int]) -> Dict[int ,List[int]]:
  """
  Return {iface_id -> [uid,...]} using table "User".
  """
  if not iface_ids:
    return {}
  ph = ",".join("?" for _ in iface_ids)
  sql = f"""
  SELECT iface_id
       , uid
  FROM "User"
  WHERE iface_id IN ({ph})
    AND uid IS NOT NULL
    AND CAST(uid AS TEXT) != ''
  ORDER BY iface_id ,uid;
  """
  rows = conn.execute(sql ,tuple(iface_ids)).fetchall()
  out: Dict[int ,List[int]] = {}
  for iface_id ,uid in rows:
    out.setdefault(int(iface_id) ,[]).append(int(uid))
  return out


# ---------- rendering ----------

def _render_composite_script(
  plan_ifaces: List[str]
  ,meta: Dict[str ,Tuple[int ,str ,Optional[str]]]
  ,routes_by_id: Dict[int ,List[Tuple[str ,Optional[str] ,Optional[str] ,Optional[int]]]]
  ,uids_by_id: Dict[int ,List[int]]
  ,subu_cidr: str
) -> str:
  """
  Build a single bash script that ensures addresses → routes → rules.
  """
  lines: List[str] = [
    "#!/usr/bin/env bash"
   ,"# apply IP state for selected interfaces (addresses, routes, rules) — idempotent"
   ,"set -euo pipefail"
   ,""
   ,"ALL_ARGS=(\"$@\")"
   ,""
   ,"want_iface(){"
   ,"  local t=$1"
   ,"  if [ ${#ALL_ARGS[@]} -eq 0 ]; then return 0; fi"
   ,"  for a in \"${ALL_ARGS[@]}\"; do [ \"$a\" = \"$t\" ] && return 0; done"
   ,"  return 1"
   ,"}"
   ,""
   ,"exists_iface(){ ip -o link show dev \"$1\" >/dev/null 2>&1; }"
   ,""
   ,"ensure_addr(){"
   ,"  local iface=$1; local cidr=$2"
   ,"  if ip -4 -o addr show dev \"$iface\" | awk '{print $4}' | grep -Fxq \"$cidr\"; then"
   ,"    logger \"addr ok: $iface $cidr\""
   ,"  else"
   ,"    ip -4 addr add \"$cidr\" dev \"$iface\""
   ,"    logger \"addr add: $iface $cidr\""
   ,"  fi"
   ,"}"
   ,""
   ,"ensure_route(){"
   ,"  local table=$1; local cidr=$2; local dev=$3; local via=${4:-}; local metric=${5:-}"
   ,"  if [ -n \"$via\" ] && [ -n \"$metric\" ]; then"
   ,"    ip -4 route replace \"$cidr\" via \"$via\" dev \"$dev\" table \"$table\" metric \"$metric\""
   ,"  elif [ -n \"$via\" ]; then"
   ,"    ip -4 route replace \"$cidr\" via \"$via\" dev \"$dev\" table \"$table\""
   ,"  elif [ -n \"$metric\" ]; then"
   ,"    ip -4 route replace \"$cidr\" dev \"$dev\" table \"$table\" metric \"$metric\""
   ,"  else"
   ,"    ip -4 route replace \"$cidr\" dev \"$dev\" table \"$table\""
   ,"  fi"
   ,"  logger \"route ensure: table=$table cidr=$cidr dev=$dev${via:+ via=$via}${metric:+ metric=$metric}\""
   ,"}"
   ,""
   ,"add_ip_rule_if_absent(){"
   ,"  local needle=$1; shift"
   ,"  if ! ip -4 rule show | grep -F -q -- \"$needle\"; then"
   ,"    ip -4 rule add \"$@\""
   ,"    logger \"rule add: $*\""
   ,"  else"
   ,"    logger \"rule ok: $needle\""
   ,"  fi"
   ,"}"
   ,""
  ]

  any_action = False

  # 1) Addresses
  for name in plan_ifaces:
    _iid ,rtname ,cidr = meta[name]
    if cidr:
      lines += [
        f'if want_iface {name}; then'
       ,f'  if exists_iface {name}; then ensure_addr {name} {cidr}; else logger "skip: iface missing: {name}"; fi'
       ,'fi'
      ]
      any_action = True

  # 2) Routes
  for name in plan_ifaces:
    iid ,rtname ,_cidr = meta[name]
    rows = routes_by_id.get(iid ,[])
    for cidr ,via ,t_override ,metric in rows:
      table_eff = t_override or rtname
      viastr = (via if via is not None else "")
      mstr = (str(metric) if metric is not None else "")
      lines += [
        f'if want_iface {name}; then'
       ,f'  if exists_iface {name}; then ensure_route "{table_eff}" "{cidr}" "{name}" "{viastr}" "{mstr}"; else logger "skip: iface missing: {name}"; fi'
       ,'fi'
      ]
      any_action = True

  # 3) Rules (src, uids, and one prohibit for the subu block)
  for name in plan_ifaces:
    iid ,rtname ,cidr = meta[name]
    if cidr:
      lines += [
        f'if want_iface {name}; then'
       ,f'  add_ip_rule_if_absent "from {cidr} lookup {rtname}" from "{cidr}" lookup "{rtname}" pref 17000'
       ,'fi'
      ]
      any_action = True
    uids = uids_by_id.get(iid ,[])
    for u in uids:
      lines += [
        f'if want_iface {name}; then'
       ,f'  add_ip_rule_if_absent "uidrange {u}-{u} lookup {rtname}" uidrange "{u}-{u}" lookup "{rtname}" pref 17010'
       ,'fi'
      ]
      any_action = True

  # One global prohibit for subu block (emit once)
  if subu_cidr:
    lines += [
      f'add_ip_rule_if_absent "from {subu_cidr} prohibit" from "{subu_cidr}" prohibit pref 18050'
    ]
    any_action = True

  if not any_action:
    raise RuntimeError("no IP state to emit for requested interfaces")

  lines += [""]  # trailing newline
  return "\n".join(lines)


def _write_dropin_for_iface(stage_root: Path ,iface: str ,script_name: str ,priority: int) -> Path:
  d = stage_root / "etc" / "systemd" / f"wg-quick@{iface}.service.d"
  d.mkdir(parents=True ,exist_ok=True)
  p = d / f"{priority}-postup-ip-state.conf"
  content = (
    "[Service]\n"
    f"ExecStartPost=+/usr/local/bin/{script_name} {iface}\n"
  )
  p.write_text(content)
  return p


# ---------- business ----------

def stage_ip_apply_script(
  conn: sqlite3.Connection
  ,iface_names: Sequence[str]
  ,stage_root: Optional[Path] = None
  ,script_name: str = "apply_ip_state.sh"
  ,dropin_priority: int = 20
  ,only_on_up: bool = True
  ,with_dropins: bool = True
  ,dry_run: bool = False
) -> Tuple[Path ,List[str]]:
  """
  Plan and stage the unified runtime script and per-iface drop-ins.
  """
  if not iface_names:
    raise RuntimeError("no interfaces provided")

  meta = _fetch_iface_meta(conn ,iface_names)
  if not meta:
    raise RuntimeError("none of the requested interfaces exist in DB")

  # preserve caller order but skip unknowns (already handled above)
  ifaces_in_order = [n for n in iface_names if n in meta]
  iface_ids = [meta[n][0] for n in ifaces_in_order]

  routes_by_id = _fetch_routes_by_iface_id(conn ,iface_ids ,only_on_up=only_on_up)
  uids_by_id = _fetch_uids_by_iface_id(conn ,iface_ids)
  subu_cidr = _fetch_meta_subu_cidr(conn ,default="10.0.0.0/24")

  sr = stage_root or STAGE_ROOT
  out = sr / "usr" / "local" / "bin" / script_name
  out.parent.mkdir(parents=True ,exist_ok=True)

  content = _render_composite_script(ifaces_in_order ,meta ,routes_by_id ,uids_by_id ,subu_cidr)

  notes: List[str] = []
  if dry_run:
    notes.append(f"dry-run: would write {out}")
    if with_dropins:
      for n in ifaces_in_order:
        notes.append(f"dry-run: would write drop-in for {n} at priority {dropin_priority}")
    return (out ,notes)

  out.write_text(content)
  out.chmod(0o500)
  notes.append(f"staged: {out}")

  if with_dropins:
    for n in ifaces_in_order:
      dp = _write_dropin_for_iface(sr ,n ,script_name ,dropin_priority)
      notes.append(f"staged: {dp}")

  return (out ,notes)


# ---------- CLI ----------

def main(argv=None) -> int:
  ap = argparse.ArgumentParser(description="Stage one script that applies addresses, routes, and rules for selected ifaces.")
  ap.add_argument("ifaces" ,nargs="+" ,help="interface names to include")
  ap.add_argument("--script-name" ,default="apply_ip_state.sh")
  ap.add_argument("--dropin-priority" ,type=int ,default=20)
  ap.add_argument("--all" ,action="store_true" ,help="include routes where on_up=0 as well")
  ap.add_argument("--no-dropins" ,action="store_true" ,help="do not stage systemd drop-ins")
  ap.add_argument("--dry-run" ,action="store_true")
  args = ap.parse_args(argv)

  with ic.open_db() as conn:
    try:
      out ,notes = stage_ip_apply_script(
        conn
       ,args.ifaces
       ,script_name=args.script_name
       ,dropin_priority=args.dropin_priority
       ,only_on_up=(not args.all)
       ,with_dropins=(not args.no_dropins)
       ,dry_run=args.dry_run
      )
    except Exception as e:
      print(f"error: {e}" ,file=sys.stderr)
      return 2

  if notes:
    print("\n".join(notes))
  return 0


if __name__ == "__main__":
  sys.exit(main())
