# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-
VERSION = "0.2.0"

USAGE = """\
subu — Subu manager (v0.2.0)

Usage:
  subu                   # usage
  subu help              # detailed help
  subu example           # example workflow
  subu version           # print version

  subu init <TOKEN>
  subu create <owner> <name>
  subu list
  subu info <Subu_ID> | subu information <Subu_ID>

  subu lo up|down <Subu_ID>

  subu WG global <BaseCIDR>
  subu WG create <host:port>
  subu WG server_provided_public_key <WG_ID> <Base64Key>
  subu WG info|information <WG_ID>
  subu WG up <WG_ID>
  subu WG down <WG_ID>

  subu attach WG <Subu_ID> <WG_ID>
  subu detach WG <Subu_ID>

  subu network up|down <Subu_ID>

  subu option set <Subu_ID> <name> <value>
  subu option get <Subu_ID> <name>
  subu option list <Subu_ID>

  subu exec <Subu_ID> -- <cmd> ...
"""

HELP = """\
Subu manager (v0.2.0)

1) Init
  subu init <TOKEN>
    Creates ./subu.db. Refuses to run if db exists.

2) Subu
  subu create <owner> <name>
  subu list
  subu info <Subu_ID>

3) Loopback
  subu lo up|down <Subu_ID>

4) WireGuard objects (independent of subu)
  subu WG global <BaseCIDR>                 # e.g., 192.168.112.0/24
  subu WG create <host:port>                # allocates next /32
  subu WG server_provided_public_key <WG_ID> <Base64Key>
  subu WG info <WG_ID>
  subu WG up <WG_ID> / subu WG down <WG_ID> # admin toggle after attached

5) Attach/detach + eBPF steering
  subu attach WG <Subu_ID> <WG_ID>
    - Creates WG dev as subu_<M> inside ns-subu_<N>, assigns /32, MTU 1420
    - Installs per-subu cgroup + loads eBPF scaffold (UID check, metadata map)
    - Keeps device admin-down until `subu network up`
  subu detach WG <Subu_ID>
    - Deletes device, removes cgroup + BPF

6) Network aggregate
  subu network up|down <Subu_ID>
    - Ensures lo up on 'up', toggles attached WG ifaces

7) Options
  subu option set|get|list ...

8) Exec
  subu exec <Subu_ID> -- <cmd> ...
"""

EXAMPLE = """\
# 0) Initialise the subu database (once per directory)
subu init
# -> created ./subu.db
# If ./subu.db already exists, init will fail with an error and do nothing.

# 1) Create a Subu “US” owned by user Thomas
subu create Thomas US
# -> Subu_ID: subu_7
# -> netns: ns-subu_7 with lo (down)

# 2) Define a global WireGuard address pool (once per host)
subu WG global 192.168.112.0/24
# -> base set; next free: 192.168.112.2/32

# 3) Create a WG object with endpoint (ReasoningTechnology server)
subu WG create 35.194.71.194:51820
# or: subu WG create ReasoningTechnology.com:51820
# -> WG_ID: WG_0
# -> local IP: 192.168.112.2/32
# -> AllowedIPs: 0.0.0.0/0

# 4) Add server public key (example key)
subu WG server_provided_public_key WG_0 ABCDEFG...xyz=
# -> saved

# 5) Attach WG to the Subu
subu attach WG subu_7 WG_0
# -> creates device ns-subu_7/subu_0
# -> assigns 192.168.112.2/32, MTU 1420, accept_local=1
# -> enforces egress steering via cgroup/eBPF for UID(s) of subu_7
# -> warns if lo is down in the netns

# 6) Bring networking up for the Subu
subu network up subu_7
# -> brings lo up in ns-subu_7
# -> brings subu_0 admin up

# 7) Start the WireGuard engine for this WG
subu WG up WG_0
# -> interface up; handshake should start if keys/endpoint are correct

# 8) Run a command inside the Subu’s netns
subu exec subu_7 -- curl -4v https://ifconfig.me
# Traffic from this process should egress via subu_0/US tunnel.
"""

def VERSION_string():
  return VERSION
