# env.py
# -*- mode: python; coding: utf-8; python-indent-offset: 2; indent-tabs-mode: nil -*-

from pathlib import Path


def version() -> str:
  """
  Software / CLI version.
  """
  return "0.3.3"


def db_schema_version() -> str:
  """
  Database schema version (used in the DB filename).

  This only changes when the DB layout/semantics change,
  not for every CLI code change.
  """
  return "0.1"


def db_root_dir() -> Path:
  """
  Default directory for the system-wide subu database.

  This is intentionally independent of the project/repo location.
  """
  return Path("/opt/subu")


def db_filename() -> str:
  """
  Default SQLite database filename, including schema version.

    subu_<schema>.sqlite3

  Example: subu_0.1.sqlite3
  """
  return f"subu_{db_schema_version()}.sqlite3"


def db_path() -> str:
  """
  Full path to the SQLite database file.

  Currently this is:

    /opt/subu/subu_<schema>.sqlite3

  There is deliberately no environment override here; this path
  defines the canonical system-wide DB used by all manager invocations.
  """
  return str(db_root_dir() / db_filename())
