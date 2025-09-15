#!/usr/bin/env python3
# stage_StanleyPark.py — stage artifacts for this client machine only
# Chooses just the ifaces we run here (x6, US) and reuses existing business funcs.

from __future__ import annotations
import sys, sqlite3, shutil
from pathlib import Path
import incommon as ic

# Reuse business modules (no logic duplication)
import stage_clean as stclean
import stage_wg_conf as stconf
import stage_preferred_server as stpref
import stage_IP_route_script as striproute
import stage_IP_rules_script as striprules
import stage_wg_unit_IP_scripts as stdrop

# Ifaces for THIS machine (adjust if needed)
IFACES = ["x6", "US"]

def msg_wrapped_call(title: str, fn=None, *args, **kwargs):
  print(f"→ {title}", flush=True)
  res = fn(*args, **kwargs) if fn else None
  print(f"✔ {title}" + (f": {res}" if res not in (None, "") else ""), flush=True)
  return res

def fetch_client_by_iface(conn: sqlite3.Connection, iface: str) -> dict | None:
  conn.row_factory = sqlite3.Row
  # Prefer the effective-view; fall back if missing
  try:
    r = conn.execute("""
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
       WHERE c.iface=? LIMIT 1;
    """,(iface,)).fetchone()
  except sqlite3.Error:
    r = conn.execute("""
      SELECT id, iface, COALESCE(rt_table_name,iface) AS rtname,
             COALESCE(rt_table_id,'') AS rtid,
             local_address_cidr AS addr,
             private_key AS priv,
             COALESCE(mtu,'') AS mtu,
             COALESCE(fwmark,'') AS fwmark,
             dns_mode AS dns_mode,
             COALESCE(dns_servers,'') AS dns_servers,
             autostart AS autostart
        FROM Iface WHERE iface=? LIMIT 1;
    """,(iface,)).fetchone()
  return (dict(r) if r else None)

def stage_for_ifaces(ifaces: list[str], clean_mode: str | None) -> int:
  # 0) Clean stage dir
  if clean_mode == "--clean":
    msg_wrapped_call("stage clean (--yes)", stclean.clean, yes=True, dry_run=False, hard=False)
  elif clean_mode == "--no-clean":
    Path(stclean.stage_root()).mkdir(parents=True, exist_ok=True)
  else:
    msg_wrapped_call("stage clean (interactive)", stclean.clean, yes=False, dry_run=False, hard=False)

  root = Path(__file__).resolve().parent
  stage_root = root / "stage"
  (stage_root / "wireguard").mkdir(parents=True, exist_ok=True)
  (stage_root / "systemd").mkdir(parents=True, exist_ok=True)
  (stage_root / "usr" / "local" / "bin").mkdir(parents=True, exist_ok=True)

  # Optional helper carry-over (kept same behavior)
  ip_rule_add = root / "IP_rule_add_UID.sh"
  if ip_rule_add.exists():
    dst = stage_root / "usr" / "local" / "bin" / "IP_rule_add_UID.sh"
    shutil.copy2(ip_rule_add, dst); dst.chmod(0o500)
    print(f"staged: {dst.relative_to(root)}")

  # 1) Global policy script — limit to selected ifaces (so rules are scoped)
  msg_wrapped_call(f"stage global set_subu_IP_rules.sh for {ifaces}",
                   striprules.stage_set_subu_ip_rules, ifaces)

  # 2) Per-iface artifacts
  with ic.open_db() as conn:
    for iface in ifaces:
      c = fetch_client_by_iface(conn, iface)
      if not c:
        print(f"⚠️  iface '{iface}' not in DB; skipping"); continue

      cid   = int(c["id"])
      addr  = str(c["addr"])
      priv  = str(c["priv"])
      mtu   = str(c["mtu"])
      fw    = str(c["fwmark"])
      dns_m = str(c["dns_mode"])
      dns_s = str(c["dns_servers"])

      srow = stpref.preferred_server_row(cid)
      if not srow:
        print(f"⚠️  No server for client '{iface}' (id={cid}). Skipping.")
        continue
      (s_name, s_pub, s_psk, s_host, s_port, s_allow, s_ka, s_route) = srow

      # WG conf
      conf_out = stage_root / "wireguard" / f"{iface}.conf"
      msg_wrapped_call(f"wg conf for {iface}",
        stconf.write_wg_conf, conf_out, addr, priv, mtu, fw, dns_m, dns_s,
        s_pub, s_psk, s_host, str(s_port), s_allow, str(s_ka or "")
      )

      # Per-iface route script
      msg_wrapped_call(f"route_init for {iface}", striproute.stage_ip_route_script, iface)

      # Systemd override referencing global rules + per-iface route
      msg_wrapped_call(f"wg-quick override for {iface}", stdrop.stage_dropin, iface)

      print(f"✔ Staged: {iface}")

  print(f"✅ Stage generation complete in: {stage_root}")
  return 0

def main(argv: list[str]) -> int:
  clean_mode = None
  if argv and argv[0] in ("--clean","--no-clean"):
    clean_mode = argv[0]
    argv = argv[1:]
  if argv:
    print(f"Usage: {Path(sys.argv[0]).name} [--clean|--no-clean]", file=sys.stderr)
    return 2
  try:
    return stage_for_ifaces(IFACES, clean_mode)
  except (sqlite3.Error, FileNotFoundError, RuntimeError) as e:
    print(f"❌ {e}", file=sys.stderr); return 1

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
