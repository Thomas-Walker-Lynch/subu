#!/usr/bin/env python3
# stage_populate.py — orchestrate stage generation (no business logic here)

from __future__ import annotations
import sys, sqlite3, shutil
from pathlib import Path
import incommon as ic

# imports of our freshly Pythonized helpers
import stage_clean as stclean
import stage_wg_conf as stconf
import stage_preferred_server as stpref
import stage_list_uids as stuids
import stage_IP_route_script as striproute
import stage_IP_rules_script as striprules
import stage_wg_unit_IP_scripts as stdrop

def msg_wrapped_call(title: str, fn=None, *args, **kwargs):
  print(f"→ {title}", flush=True)
  res = fn(*args, **kwargs) if fn else None
  print(f"✔ {title}" + (f": {res}" if res not in (None, "") else ""), flush=True)
  return res

def list_client(conn: sqlite3.Connection) -> list[sqlite3.Row]:
  conn.row_factory = sqlite3.Row
  try:
    sql = """
      SELECT c.id, c.iface, v.rt_table_name_eff AS rtname,
             COALESCE(c.rt_table_id,'') AS rtid,
             c.local_address_cidr AS addr,
             c.private_key AS priv,
             COALESCE(c.mtu,'') AS mtu,
             COALESCE(c.fwmark,'') AS fwmark,
             c.dns_mode AS dns_mode,
             COALESCE(c.dns_servers,'') AS dns_servers,
             c.autostart AS autostart
        FROM Iface c
        JOIN v_client_effective v ON v.id=c.id
       ORDER BY c.id;
    """
    return list(conn.execute(sql))
  except sqlite3.Error:
    # fallback if view missing
    sql = """
      SELECT id, iface, COALESCE(rt_table_name,iface) AS rtname,
             COALESCE(rt_table_id,'') AS rtid,
             local_address_cidr AS addr,
             private_key AS priv,
             COALESCE(mtu,'') AS mtu,
             COALESCE(fwmark,'') AS fwmark,
             dns_mode, COALESCE(dns_servers,'') AS dns_servers,
             autostart
        FROM Iface ORDER BY id;
    """
    return list(conn.execute(sql))

def stage_populate(clean_mode: str | None) -> int:
  # 0) clean stage
  if clean_mode == "--clean":
    msg_wrapped_call("stage clean (--yes)", stclean.clean, yes=True, dry_run=False, hard=False)
  elif clean_mode == "--no-clean":
    Path(stclean.stage_root()).mkdir(parents=True, exist_ok=True)
  else:
    # interactive prompt like original
    msg_wrapped_call("stage clean (interactive)", stclean.clean, yes=False, dry_run=False, hard=False)

  # base dirs
  root = Path(__file__).resolve().parent
  stage_root = root / "stage"
  (stage_root / "wireguard").mkdir(parents=True, exist_ok=True)
  (stage_root / "systemd").mkdir(parents=True, exist_ok=True)
  (stage_root / "usr" / "local" / "bin").mkdir(parents=True, exist_ok=True)

  # 1) optional helper copy
  ip_rule_add = root / "IP_rule_add_UID.sh"
  if ip_rule_add.exists():
    dst = stage_root / "usr" / "local" / "bin" / "IP_rule_add_UID.sh"
    shutil.copy2(ip_rule_add, dst)
    dst.chmod(0o500)
    print(f"staged: {dst.relative_to(root)}")

  # 2) stage global policy script once (replaces per-iface policy_init_*.sh)
  msg_wrapped_call("stage global set_subu_IP_rules.sh", striprules.stage_set_subu_ip_rules)

  # 3) per-client staging
  with ic.open_db() as conn:
    for r in list_client(conn):
      cid   = int(r["id"]); iface = str(r["iface"])
      rt    = str(r["rtname"])
      addr  = str(r["addr"]); priv = str(r["priv"])
      mtu   = str(r["mtu"]);  fw   = str(r["fwmark"])
      dns_m = str(r["dns_mode"]); dns_s = str(r["dns_servers"])

      # 3a) preferred server
      srow = stpref.preferred_server_row(cid)
      if not srow:
        print(f"⚠️  No server for client '{iface}' (id={cid}). Skipping.")
        continue
      (s_name, s_pub, s_psk, s_host, s_port, s_allow, s_ka, s_route) = srow

      # 3b) WG conf
      conf_out = stage_root / "wireguard" / f"{iface}.conf"
      msg_wrapped_call(f"wg conf for {iface}",
        stconf.write_wg_conf, conf_out, addr, priv, mtu, fw, dns_m, dns_s,
        s_pub, s_psk, s_host, str(s_port), s_allow, str(s_ka or "")
      )

      # 3c) route init script
      msg_wrapped_call(f"route_init for {iface}", striproute.stage_ip_route_script, iface)

      # 3d) systemd override referencing global rules + per-iface route
      msg_wrapped_call(f"wg-quick override for {iface}", stdrop.stage_dropin, iface)

      print(f"✔ Staged: {iface}")

  print(f"✅ Stage generation complete in: {stage_root}")
  return 0

def main(argv):
  clean_mode = None
  if argv:
    if argv[0] in ("--clean","--no-clean"):
      clean_mode = argv[0]
      argv = argv[1:]
  if argv:
    print(f"Usage: {Path(sys.argv[0]).name} [--clean|--no-clean]", file=sys.stderr); return 2
  try:
    return stage_populate(clean_mode)
  except (sqlite3.Error, FileNotFoundError, RuntimeError) as e:
    print(f"❌ {e}", file=sys.stderr); return 1

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
