# text.py

from env import version as current_version


class Text:
  """
  Program text bound to a specific command name.

  Usage:
    text_1 = Text("subu")
    text_2 = Text("manager")

    print(text_1.usage())
    print(text_2.help())
  """

  def __init__(self, program_name ="subu"):
    self.program_name = program_name

  def usage(self):
    program_name = self.program_name
    return f"""{program_name} — Subu manager (v{current_version()})

Usage:
  {program_name}                   # usage
  {program_name} help              # detailed help
  {program_name} example           # example workflow
  {program_name} version           # print version

  {program_name} init <TOKEN>
  {program_name} make <masu> <subu> [_<subu>]*
  {program_name} list
  {program_name} info <Subu_ID> | {program_name} information <Subu_ID>

  {program_name} lo up|down <Subu_ID>

  {program_name} WG global <BaseCIDR>
  {program_name} WG make <host:port>
  {program_name} WG server_provided_public_key <WG_ID> <Base64Key>
  {program_name} WG info|information <WG_ID>
  {program_name} WG up <WG_ID>
  {program_name} WG down <WG_ID>

  {program_name} attach WG <Subu_ID> <WG_ID>
  {program_name} detach WG <Subu_ID>

  {program_name} network up|down <Subu_ID>

  {program_name} option set <Subu_ID> <subu> <value>
  {program_name} option get <Subu_ID> <subu>
  {program_name} option list <Subu_ID>

  {program_name} exec <Subu_ID> -- <cmd> ...
"""

  def help(self, verbose =False):
    program_name = self.program_name
    return f"""Subu manager (v{current_version()})

1) Init
  {program_name} init <TOKEN>
    Makes ./subu.db. Refuses to run if db exists.

2) Subu
  {program_name} make <masu> <subu> [_<subu>]*
  {program_name} list
  {program_name} info <Subu_ID>

3) Loopback
  {program_name} lo up|down <Subu_ID>

4) WireGuard objects (independent of subu)
  {program_name} WG global <BaseCIDR>                 # for example, 192.168.112.0/24
  {program_name} WG make <host:port>                # allocates next /32
  {program_name} WG server_provided_public_key <WG_ID> <Base64Key>
  {program_name} WG info <WG_ID>
  {program_name} WG up <WG_ID> / {program_name} WG down <WG_ID> # administrative toggle after attached

5) Attach or detach and eBPF steering
  {program_name} attach WG <Subu_ID> <WG_ID>
    - Makes WireGuard device as subu_<M> inside ns-subu_<N>, assigns /32, MTU 1420
    - Installs per-subu cgroup and loads eBPF scaffold (user identifier check, metadata map)
    - Keeps device administrative-down until `{program_name} network up`
  {program_name} detach WG <Subu_ID>
    - Deletes device, removes cgroup and eBPF program

6) Network aggregate
  {program_name} network up|down <Subu_ID>
    - Ensures loopback is up on 'up', toggles attached WireGuard interfaces

7) Options
  {program_name} option set|get|list ...

8) Exec
  {program_name} exec <Subu_ID> -- <cmd> ...
"""

  def example(self):
    program_name = self.program_name
    return f"""# 0) Initialise the subu database (once per directory)
{program_name} init dzkq7b

# 1) Make Subu
{program_name} make Thomas US
# -> subu_1

# 2) WireGuard pool once
{program_name} WG global 192.168.112.0/24

# 3) Make WireGuard object with endpoint
{program_name} WG make ReasoningTechnology.com:51820
# -> WG_1

# 4) Server public key (placeholder)
{program_name} WG server_provided_public_key WG_1 ABCDEFG...xyz=

# 5) Attach device and install cgroup and eBPF steering
{program_name} attach WG subu_1 WG_1

# 6) Bring network up (loopback and WireGuard)
{program_name} network up subu_1

# 7) Test inside namespace
{program_name} exec subu_1 -- curl -4v https://ifconfig.me
"""

  def version(self):
    return current_version()


def make_text(program_name ="subu"):
  return Text(program_name)
