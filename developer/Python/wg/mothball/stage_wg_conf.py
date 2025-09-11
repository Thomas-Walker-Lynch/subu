#!/usr/bin/env python3
# stage_wg_conf.py — write stage/wireguard/<iface>.conf

from __future__ import annotations
import sys, argparse
from pathlib import Path

def write_wg_conf(out: Path, addr: str, priv: str, mtu: str, fwmark: str,
                  dns_mode: str, dns_servers: str,
                  peer_pub: str, psk: str, host: str, port: str,
                  allowed: str, keepalive: str) -> Path:
  out.parent.mkdir(parents=True, exist_ok=True)
  lines = [
    "[Interface]",
    f"Address = {addr}",
    f"PrivateKey = {priv}",
  ]
  if mtu:    lines.append(f"MTU = {mtu}")
  if fwmark: lines.append(f"FwMark = {fwmark}")
  if dns_mode == "static" and dns_servers:
    lines.append(f"DNS = {dns_servers}")
  lines.append("Table = off")  # policy routing handled outside wg-quick
  lines += [
    "",
    "[Peer]",
    f"PublicKey = {peer_pub}",
  ]
  if psk: lines.append(f"PresharedKey = {psk}")
  lines += [
    f"Endpoint = {host}:{port}",
    f"AllowedIPs = {allowed}",
  ]
  if keepalive: lines.append(f"PersistentKeepalive = {keepalive}")

  out.write_text("\n".join(lines) + "\n")
  out.chmod(0o400)
  return out

def main(argv):
  ap = argparse.ArgumentParser()
  ap.add_argument("out"); ap.add_argument("addr"); ap.add_argument("priv")
  ap.add_argument("mtu"); ap.add_argument("fwmark")
  ap.add_argument("dns_mode"); ap.add_argument("dns_servers")
  ap.add_argument("peer_pub"); ap.add_argument("psk")
  ap.add_argument("host"); ap.add_argument("port")
  ap.add_argument("allowed"); ap.add_argument("keepalive")
  args = ap.parse_args(argv)
  out = write_wg_conf(Path(args.out), args.addr, args.priv, args.mtu, args.fwmark,
                      args.dns_mode, args.dns_servers, args.peer_pub, args.psk,
                      args.host, args.port, args.allowed, args.keepalive)
  print(f"staged: {out}")
  return 0

if __name__ == "__main__":
  sys.exit(main(sys.argv[1:]))
