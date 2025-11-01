# ===== File: subu_bpf.py =====
#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-

"""
Stub for eBPF steering (cgroup/connect4+sendmsg4 hooks) to enforce sk_bound_dev_if.
Implementation notes:
  * We will later compile a small eBPF C program (libbpf/bpftool) that:
    - on connect4/sendmsg4: if process UID==subu UID -> sets sk_bound_dev_if to WG ifindex
  * For now, we provide placeholders that pretend success.
"""

import subu_utils as U
import subu_db as DB


def install_steer(subu_id: str, wg_ifindex: int):
  # TODO: load BPF, attach to cgroup v2 path; store cgroup path in DB
  DB.update_subu_cgroup(subu_id, "/sys/fs/cgroup/subu_placeholder")
  return 0


def remove_steer(subu_id: str):
  # TODO: detach and unload
  return 0
