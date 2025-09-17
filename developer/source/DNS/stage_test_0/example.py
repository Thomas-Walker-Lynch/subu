#!/usr/bin/env -S python3 -B
import Stage

# You can compute these with arbitrary Python if you like
svc = "unbound"
zone = "US"
fname = f"unbound-{zone}.conf"

Stage.init(
  write_file_name="."                 # '.' → use basename of this file -> 'example_dns.py'
, write_file_directory_path="/etc/unbound"
, write_file_owner="root"
, write_file_permissions=0o644        # or "0644"
, read_file_contents="""\
# generated config (example)
server:
  verbosity: 1
  interface: 127.0.0.1
"""
)

# declare the desired operations (no effect in 'noop'/'dry' without a copier)
Stage.displace()
Stage.copy()
