#!/usr/bin/env python3
# stage_wg_conf.py — write stage/wireguard/<iface>.conf (Table=off)

from __future__ import annotations
from pathlib import Path
from typing import Optional, Union

ROOT = Path(__file__).resolve().parent
STAGE_ROOT = ROOT / "stage"

def write_wg_conf(
  out_path: Path,
  *,
  addr: str,
  private_key: str,
  mtu: Optional[Union[int, str]] = None,
  fwmark: Optional[Union[int, str]] = None,
  dns_mode: str = "none",
  dns_servers: Optional[str] = None,
  peer_pub: str,
  psk: Optional[str],
  host: str,
  port: Union[int, str],
  allowed: str,
  keepalive: Optional[Union[int, str]] = None,
) -> Path:
  """
  Given WG interface params + peer params, writes a config with Table=off.
  Returns the written path.
  """
  out_path.parent.mkdir(parents=True, exist_ok=True)
  lines = []
  lines.append("[Interface]")
  lines.append(f"Address = {addr}")
  lines.append(f"PrivateKey = {private_key}")
  if mtu not in (None, "", 0):    lines.append(f"MTU = {mtu}")
  if fwmark not in (None, "", 0): lines.append(f"FwMark = {fwmark}")
  if dns_mode == "static" and dns_servers:
    lines.append(f"DNS = {dns_servers}")
  # policy routing handled by our scripts, not wg-quick
  lines.append("Table = off")
  lines.append("")
  lines.append("[Peer]")
  lines.append(f"PublicKey = {peer_pub}")
  if psk: lines.append(f"PresharedKey = {psk}")
  lines.append(f"Endpoint = {host}:{port}")
  lines.append(f"AllowedIPs = {allowed}")
  if keepalive not in (None, "", 0): lines.append(f"PersistentKeepalive = {keepalive}")
  out_path.write_text("\n".join(lines) + "\n")
  out_path.chmod(0o400)
  return out_path

# Optional CLI wrapper
if __name__ == "__main__":
  import sys, argparse
  ap = argparse.ArgumentParser()
  ap.add_argument("out")
  ap.add_argument("--addr", required=True)
  ap.add_argument("--priv", required=True)
  ap.add_argument("--mtu")
  ap.add_argument("--fwmark")
  ap.add_argument("--dns-mode", default="none")
  ap.add_argument("--dns-servers")
  ap.add_argument("--peer-pub", required=True)
  ap.add_argument("--psk")
  ap.add_argument("--host", required=True)
  ap.add_argument("--port", required=True)
  ap.add_argument("--allowed", required=True)
  ap.add_argument("--keepalive")
  args = ap.parse_args()
  p = write_wg_conf(
    Path(args.out),
    addr=args.addr,
    private_key=args.priv,
    mtu=args.mtu, fwmark=args.fwmark,
    dns_mode=args.dns_mode, dns_servers=args.dns_servers,
    peer_pub=args.peer_pub, psk=args.psk,
    host=args.host, port=args.port, allowed=args.allowed,
    keepalive=args.keepalive,
  )
  print(f"staged: {p}")
