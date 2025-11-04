
def exec_in_subu(subu_id: str, cmd: list):
  sid = int(subu_id.split("_")[1])
  with closing(_db()) as db:
    ns = db.execute("SELECT netns FROM subu WHERE id=?", (sid,)).fetchone()[0]
  os.execvp("ip", ["ip","netns","exec", ns] + cmd)
