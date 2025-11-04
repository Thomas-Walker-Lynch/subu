# ---------------- Unix users & groups ----------------

def _group_exists(name: str) -> bool:
  try:
    grp.getgrnam(name)
    return True
  except KeyError:
    return False

def _user_exists(name: str) -> bool:
  try:
    pwd.getpwnam(name)
    return True
  except KeyError:
    return False

def _ensure_group(name: str):
  if not _group_exists(name):
    # groupadd <name>
    run(["groupadd", name])

def _ensure_user(name: str, primary_group: str):
  if not _user_exists(name):
    # useradd -m -g <primary_group> -s /bin/bash <name>
    run(["useradd", "-m", "-g", primary_group, "-s", "/bin/bash", name])

def _add_user_to_group(user: str, group: str):
  run(["usermod", "-aG", group, user])
