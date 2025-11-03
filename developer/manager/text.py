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
# 0) Init
subu init dzkq7b

# 1) Create Subu
subu create Thomas US
# -> subu_1

# 2) WG pool once
subu WG global 192.168.112.0/24

# 3) Create WG object with endpoint
subu WG create ReasoningTechnology.com:51820
# -> WG_1

# 4) Pubkey (placeholder)
subu WG server_provided_public_key WG_1 ABCDEFG...xyz=

# 5) Attach device and install cgroup+BPF steering
subu attach WG subu_1 WG_1

# 6) Bring network up (lo + WG)
subu network up subu_1

# 7) Test inside ns
subu exec subu_1 -- curl -4v https://ifconfig.me
"""

def VERSION_string():
  return VERSION
