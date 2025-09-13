#!/usr/bin/env bash
# Usage: db_wipe.sh [--force] [--dry-run] [--include-hidden]
# Job: Remove regular non-hidden files in ./db (e.g., store, store-wal, store-shm), keeping the directory.
# Safety:
#   - Refuses to run if ./db does not exist or is not named exactly "db".
#   - Prints a plan, then asks: "Are you sure? [y/N]" unless --force is used.
#   - --dry-run prints what would be removed without deleting.
#   - Hidden files (names starting with '.') are preserved by default (e.g., .gitignore).
# Notes:
#   - Comments avoid possessive pronouns.

set -euo pipefail

SELF_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
DB_DIR="$SELF_DIR/db"

FORCE=0
DRYRUN=0
INCLUDE_HIDDEN=0

while (($#)); do
  case "$1" in
    --force)           FORCE=1 ;;
    --dry-run)         DRYRUN=1 ;;
    --include-hidden)  INCLUDE_HIDDEN=1 ;;
    -h|--help)         sed -n '2,30p' "$0"; exit 0 ;;
    *) echo "unknown option: $1" >&2; exit 2 ;;
  esac
  shift || true
done

# Guards
[[ -d "$DB_DIR" ]] || { echo "❌ not found: $DB_DIR"; exit 1; }
[[ "$(basename -- "$DB_DIR")" == "db" ]] || { echo "❌ expected directory named 'db', got: $(basename -- "$DB_DIR")"; exit 1; }

# Build find expression
if (( INCLUDE_HIDDEN )); then
  # include all regular files
  mapfile -t TARGETS < <(find "$DB_DIR" -maxdepth 1 -type f -print | sort)
else
  # exclude dotfiles (preserve .gitignore and other hidden files)
  mapfile -t TARGETS < <(find "$DB_DIR" -maxdepth 1 -type f ! -name '.*' -print | sort)
fi

if ((${#TARGETS[@]}==0)); then
  echo "db_wipe: no matching files in: ${DB_DIR#$SELF_DIR/}"
  exit 0
fi

echo "db_wipe: plan"
for f in "${TARGETS[@]}"; do
  echo "  delete: ${f#$SELF_DIR/}"
done

if (( DRYRUN )); then
  echo "db_wipe: dry-run; no changes made"
  exit 0
fi

if (( ! FORCE )); then
  printf "Are you sure? [y/N] "
  read -r ans || true
  case "${ans,,}" in y|yes) ;; *) echo "db_wipe: aborted"; exit 0 ;; esac
fi

# Delete
for f in "${TARGETS[@]}"; do
  rm -f -- "$f"
done

echo "db_wipe: deleted ${#TARGETS[@]} file(s) from ${DB_DIR#$SELF_DIR/}"
