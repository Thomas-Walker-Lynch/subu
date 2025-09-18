# example unbound.conf
def configure(prov, planner, WriteFileMeta):
  # use current user for owner, and use this script’s py-less name for the filename

  # owner defaults to root (this is a configuration file installer)
  # owner "." means owner of the process running Stagehane
  # owner "." is good for testing

  # fname "." means write file has the same name as read file (without .py if it has .py)
  # fname "." is the default, so it is redundant here. "." still works in args, even when wfm changes the fname.

  wfm = WriteFileMeta(dpath="stage_test_0_out", fname=".", owner=".")
  planner.displace(wfm)
  planner.copy(wfm, content="server:\n  do-ip6: no\n")

