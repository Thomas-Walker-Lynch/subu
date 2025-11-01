# ===== File: subu_db.py =====
#!/usr/bin/env python3
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-

import os, sqlite3, json
import subu_utils as U

SCHEMA = {
  "meta": "CREATE TABLE IF NOT EXISTS meta (k TEXT PRIMARY KEY, v TEXT)" ,
  "subu": """
    CREATE TABLE IF NOT EXISTS subu (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      masu TEXT NOT NULL,
      subu TEXT NOT NULL,
      uid INTEGER,
      netns TEXT,
      cgroup_path TEXT,
      UNIQUE(masu, subu)
    )""",
  "wg": """
    CREATE TABLE IF NOT EXISTS wg (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      remote TEXT NOT NULL,
      pubkey TEXT,
      dev TEXT,
      addr TEXT,
      state TEXT DEFAULT 'down'
    )""",
  "attach": """
    CREATE TABLE IF NOT EXISTS attach (
      subu_id INTEGER NOT NULL,
      wg_id INTEGER NOT NULL,
      PRIMARY KEY (subu_id, wg_id)
    )""",
}

class NotInitializedError(Exception):
  pass


def require_initialized():
  if not os.path.exists(U.path_db()):
    raise NotInitializedError()


def connect():
  return sqlite3.connect(U.path_db())


def init_db():
  if os.path.exists(U.path_db()):
    return False
  con = sqlite3.connect(U.path_db())
  try:
    cur = con.cursor()
    for sql in SCHEMA.values():
      cur.execute(sql)
    con.commit()
    return True
  finally:
    con.close()


def put_meta(k, v):
  con = connect(); cur = con.cursor()
  cur.execute("INSERT OR REPLACE INTO meta(k,v) VALUES(?,?)", (k, v))
  con.commit(); con.close()

def get_meta(k, default=None):
  con = connect(); cur = con.cursor()
  cur.execute("SELECT v FROM meta WHERE k=?", (k,))
  row = cur.fetchone(); con.close()
  return row[0] if row else default


def create_subu(masu, subu):
  con = connect(); cur = con.cursor()
  cur.execute("INSERT INTO subu(masu,subu) VALUES(?,?)", (masu, subu))
  con.commit()
  sid = cur.lastrowid
  con.close()
  return f"subu_{sid}"


def list_subu():
  con = connect(); cur = con.cursor()
  cur.execute("SELECT id,masu,subu,uid,netns,cgroup_path FROM subu ORDER BY id")
  rows = cur.fetchall(); con.close(); return rows


def subu_by_id(subu_id):
  if not subu_id.startswith("subu_"):
    raise ValueError("bad subu id")
  sid = int(subu_id.split("_")[1])
  con = connect(); cur = con.cursor()
  cur.execute("SELECT id,masu,subu,uid,netns,cgroup_path FROM subu WHERE id=?", (sid,))
  row = cur.fetchone(); con.close(); return row


def update_subu_netns(subu_id, netns):
  sid = int(subu_id.split("_")[1])
  con = connect(); cur = con.cursor()
  cur.execute("UPDATE subu SET netns=? WHERE id=?", (netns, sid))
  con.commit(); con.close()


def update_subu_uid(subu_id, uid):
  sid = int(subu_id.split("_")[1])
  con = connect(); cur = con.cursor()
  cur.execute("UPDATE subu SET uid=? WHERE id=?", (uid, sid))
  con.commit(); con.close()


def update_subu_cgroup(subu_id, path):
  sid = int(subu_id.split("_")[1])
  con = connect(); cur = con.cursor()
  cur.execute("UPDATE subu SET cgroup_path=? WHERE id=?", (path, sid))
  con.commit(); con.close()

# WG

def wg_set_global_base(cidr):
  put_meta("wg_base_cidr", cidr)


def wg_create(remote):
  con = connect(); cur = con.cursor()
  cur.execute("INSERT INTO wg(remote) VALUES(?)", (remote,))
  con.commit(); wid = cur.lastrowid; con.close(); return f"WG_{wid}"


def wg_list():
  con = connect(); cur = con.cursor()
  cur.execute("SELECT id,remote,pubkey,dev,addr,state FROM wg ORDER BY id")
  rows = cur.fetchall(); con.close(); return rows


def wg_by_id(wg_id):
  if not wg_id.startswith("WG_"):
    raise ValueError("bad WG id")
  wid = int(wg_id.split("_")[1])
  con = connect(); cur = con.cursor()
  cur.execute("SELECT id,remote,pubkey,dev,addr,state FROM wg WHERE id=?", (wid,))
  row = cur.fetchone(); con.close(); return row


def wg_update(wg_id, **kv):
  wid = int(wg_id.split("_")[1])
  con = connect(); cur = con.cursor()
  cols = ",".join([f"{k}=?" for k in kv.keys()])
  cur.execute(f"UPDATE wg SET {cols} WHERE id=?", [*kv.values(), wid])
  con.commit(); con.close()


def attach(subu_id, wg_id):
  sid = int(subu_id.split("_")[1])
  wid = int(wg_id.split("_")[1])
  con = connect(); cur = con.cursor()
  cur.execute("INSERT OR REPLACE INTO attach(subu_id,wg_id) VALUES(?,?)", (sid, wid))
  con.commit(); con.close()


def detach(subu_id, wg_id):
  sid = int(subu_id.split("_")[1])
  wid = int(wg_id.split("_")[1])
  con = connect(); cur = con.cursor()
  cur.execute("DELETE FROM attach WHERE subu_id=? AND wg_id=?", (sid, wid))
  con.commit(); con.close()


def attached_wg_ids(sid: int):
  con = connect(); cur = con.cursor()
  cur.execute("SELECT wg_id FROM attach WHERE subu_id=?", (sid,))
  rows = [f"WG_{r[0]}" for r in cur.fetchall()]
  con.close(); return rows

