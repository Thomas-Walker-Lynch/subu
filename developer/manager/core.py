# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-
"""
core.py — worker API for subu manager
Version: 0.2.0
"""
import os, sqlite3, subprocess
from pathlib import Path
from contextlib import closing
from text import VERSION
from worker_bpf import ensure_mounts, install_steering, remove_steering, BpfError
import db

DB_FILE = Path("./subu.db")
WG_GLOBAL_FILE = Path("./WG_GLOBAL")

def run(cmd, check=True):
  r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
  if check and r.returncode != 0:
    raise RuntimeError(f"cmd failed: {' '.join(cmd)}\n{r.stderr}")
  return r.stdout.strip()


