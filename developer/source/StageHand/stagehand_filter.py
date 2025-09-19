# StageHand acceptance filter (default template)
# Return True to include a config file, False to skip it.
# You receive a PlanProvenance object named `prov`.
#
# prov fields commonly used here:
#   prov.stage_root_dpath : Path   → absolute path to the stage root
#   prov.config_abs_fpath : Path   → absolute path to the candidate file
#   prov.config_rel_fpath : Path   → path relative to the stage root
#   prov.read_dir_dpath   : Path   → directory of the candidate file
#   prov.read_fname       : str    → filename with trailing '.py' stripped (if present)
#
# Examples:
#
# 1) Accept everything (default behavior):
# def accept(prov):
#   return True
#
# 2) Only accept configs in a 'dns/' namespace under the stage:
# def accept(prov):
#   return prov.config_rel_fpath.as_posix().startswith("dns/")
#
# 3) Exclude editor backup files:
# def accept(prov):
#   rel = prov.config_rel_fpath.as_posix()
#   return not (rel.endswith("~") or rel.endswith(".swp"))
#
# 4) Only accept Python files + a few non-Python names:
# def accept(prov):
#   name = prov.config_abs_fpath.name
#   return name.endswith(".py") or name in {"hosts", "resolv.conf"}
#
# Choose ONE 'accept' definition. Below is the default:

def accept(prov):
  return True
