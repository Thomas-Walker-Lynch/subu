# ------------- Subu ops -------------
def create_subu(owner: str, name: str) -> str:
  with closing(_db()) as db:
    c = db.cursor()
    subu_netns = f"ns-subu_tmp"  # temp; we rename after ID known
    c.execute("INSERT INTO subu (owner, name, netns) VALUES (?, ?, ?)",
              (owner, name, subu_netns))
    sid = c.lastrowid
    netns = f"ns-subu_{sid}"
    c.execute("UPDATE subu SET netns=? WHERE id=?", (netns, sid))
    db.commit()

  # create netns
  run(["ip", "netns", "add", netns])
  run(["ip", "-n", netns, "link", "set", "lo", "down"])
  print(f"Created subu_{sid} ({owner}:{name}) with netns {netns}")
  return f"subu_{sid}"

def list_subu():
  with closing(_db()) as db:
    for row in db.execute("SELECT id, owner, name, netns, lo_state, wg_id, network_state FROM subu"):
      print(row)

def info_subu(subu_id: str):
  sid = int(subu_id.split("_")[1])
  with closing(_db()) as db:
    row = db.execute("SELECT * FROM subu WHERE id=?", (sid,)).fetchone()
    if not row:
      print("not found"); return
    print(row)
    wg = db.execute("SELECT wg_id FROM subu WHERE id=?", (sid,)).fetchone()[0]
    if wg is not None:
      wrow = db.execute("SELECT * FROM wg WHERE id=?", (wg,)).fetchone()
      print("WG:", wrow)
    opts = db.execute("SELECT name,value FROM options WHERE subu_id=?", (sid,)).fetchall()
    print("Options:", opts)

def lo_toggle(subu_id: str, state: str):
  sid = int(subu_id.split("_")[1])
  with closing(_db()) as db:
    ns = db.execute("SELECT netns FROM subu WHERE id=?", (sid,)).fetchone()
    if not ns: raise ValueError("subu not found")
    ns = ns[0]
    run(["ip", "netns", "exec", ns, "ip", "link", "set", "lo", state])
    db.execute("UPDATE subu SET lo_state=? WHERE id=?", (state, sid))
    db.commit()
  print(f"{subu_id}: lo {state}")

# ---------------- High-level Subu factory ----------------

def make_subu(path_tokens: list[str]) -> str:
  """
  Create a new Subu with hierarchical name and full wiring:

    path_tokens: ['Thomas', 'US'] or ['Thomas', 'new-subu', 'Rabbit']

  Rules:
    - len(path_tokens) >= 2
    - parent path (everything except last token) must already exist
      as:
        * a Unix user (for len==2: just the top-level user, e.g. 'Thomas')
        * and as a Subu in our DB if len > 2 (e.g. 'Thomas_new-subu')
    - new Unix user name is path joined by '_', e.g. 'Thomas_new-subu_Rabbit'
    - mas u(root) is path_tokens[0]
    - groups:
        <masu>
        <masu>-incommon

  Side effects:
    - DB row in 'subu' (id, owner, name, full_unix_name, path, netns_name, ...)
    - netns ns-subu_<id> created with lo down
    - Unix user created/ensured
    - Unix groups ensured and membership updated

  Returns: textual Subu_ID, e.g. 'subu_7'.
  """
  if not path_tokens or len(path_tokens) < 2:
    raise SystemExit("subu: make requires at least two path elements, e.g. 'Thomas US'")

  # Normalised pieces
  path_tokens = [p.strip() for p in path_tokens if p.strip()]
  if len(path_tokens) < 2:
    raise SystemExit("subu: make requires at least two non-empty path elements")

  masu = path_tokens[0]                    # root user / owner
  leaf = path_tokens[-1]                   # new subu leaf
  parent_tokens = path_tokens[:-1]         # parent path
  full_unix_name = "_".join(path_tokens)   # e.g. 'Thomas_new-subu_Rabbit'
  parent_unix_name = "_".join(parent_tokens)
  path_str = " ".join(path_tokens)         # e.g. 'Thomas new-subu Rabbit'

  # 1) Enforce parent existing

  # Case A: top-level subu (e.g. ['Thomas', 'US'])
  if len(path_tokens) == 2:
    # Require the root user to exist as a Unix user
    if not _user_exists(masu):
      raise SystemExit(
        f"subu: cannot make '{path_str}': root user '{masu}' does not exist"
      )
  else:
    # Case B: deeper subu: require parent subu exists in our DB
    parent_row = get_subu_by_full_unix_name(parent_unix_name)
    if not parent_row:
      raise SystemExit(
        f"subu: cannot make '{path_str}': parent subu '{parent_unix_name}' does not exist"
      )

  # Also forbid duplicate full_unix_name
  existing = get_subu_by_full_unix_name(full_unix_name)
  if existing:
    raise SystemExit(
      f"subu: subu with name '{full_unix_name}' already exists (id=subu_{existing[0]})"
    )

  # 2) Insert DB row and allocate ID + netns_name

  with closing(open_db()) as db:
    subu_id_num = _first_free_id(db, "subu")
    netns_name = f"ns-subu_{subu_id_num}"

    db.execute(
      "INSERT INTO subu(id, owner, name, full_unix_name, path, netns_name, wg_id, created_at, updated_at) "
      "VALUES (?, ?, ?, ?, ?, ?, NULL, datetime('now'), datetime('now'))",
      (subu_id_num, masu, leaf, full_unix_name, path_str, netns_name)
    )
    db.commit()

  subu_id = f"subu_{subu_id_num}"

  # 3) Create netns + lo down
  _create_netns_for_subu(subu_id_num, netns_name)

  # 4) Ensure Unix user + groups

  unix_user = full_unix_name
  group_masu = masu
  group_incommon = f"{masu}-incommon"

  _ensure_group(group_masu)
  _ensure_group(group_incommon)

  _ensure_user(unix_user, group_masu)
  _add_user_to_group(unix_user, group_masu)       # mostly redundant but explicit
  _add_user_to_group(unix_user, group_incommon)

  print(f"Created Subu {subu_id} for path '{path_str}' with Unix user '{unix_user}' "
        f"and netns '{netns_name}'")

  return subu_id
