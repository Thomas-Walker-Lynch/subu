# ===== File: subu_utils.py =====
#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-

import os, sys, subprocess, shlex


def ok(msg: str, code: int = 0) -> int:
  print(msg)
  return code

def err(msg: str, code: int = 2) -> int:
  print(f"❌ {msg}")
  return code


def run(cmd: str, check=True, capture=False, env=None, ns_enter=None):
  """Run shell command. If ns_enter is a netns name, prefix with `ip netns exec`.
  Returns (rc, outstr).
  """
  if ns_enter:
    cmd = f"ip netns exec {shlex.quote(ns_enter)} {cmd}"
  p = subprocess.run(cmd, shell=True, env=env,
                     stdout=subprocess.PIPE if capture else None,
                     stderr=subprocess.STDOUT)
  out = p.stdout.decode() if p.stdout else ""
  if check and p.returncode != 0:
    raise RuntimeError(f"command failed ({p.returncode}): {cmd}\n{out}")
  return p.returncode, out


def path_db():
  return os.path.abspath("subu.db")

