#!/bin/bash
# masu_subu__map_own.sh <user> <subu> [--suid]
#
# Examples:
#   subu_bind Thomas developer           # default (nosuid)
#   subu_bind Thomas developer --suid    # enable setuid on this mount

set -euo pipefail

subu_bind() {
  local user="$1"
  local subu="$2"
  local want_suid="${3-}"   # optional third arg

  if ! command -v bindfs &>/dev/null; then
    echo "Error: bindfs is not installed!" >&2
    return 1
  fi

  # identities
  local master_user_name="$user"
  local master_group="$user"
  local subu_user_name="${user}-${subu}"
  local subu_group="${user}-${subu}"

  id "$master_user_name" &>/dev/null || { echo "Error: user '$master_user_name' not found!" >&2; return 1; }
  id "$subu_user_name"   &>/dev/null || { echo "Error: sub-user '$subu_user_name' not found!"   >&2; return 1; }

  # paths
  local subu_data_path="/home/$user/subu_data/$subu"
  local subu_mount_point_path="/home/$user/subu/$subu"

  [[ -d "$subu_data_path" ]] || { echo "Error: source dir '$subu_data_path' does not exist!" >&2; return 1; }
  mkdir -p "$subu_mount_point_path"

  # mount options
  # - allow_other/default_permissions: kernel does POSIX perms
  # - exec: allow execution on the mount
  # - suid: only when explicitly requested (and only works when mounted as root)
  local base_opts="allow_other,default_permissions,exec"
  local opts="$base_opts,nosuid"
  if [[ "$want_suid" == "--suid" ]]; then
    opts="$base_opts,suid"
  fi

  # The UID/GID mapping you already use
  local map_opt="--map=${subu_user_name}/${master_user_name}:@${subu_group}/@${master_group}"

  # If already mounted, decide whether to keep or remount
  if findmnt -n -T "$subu_mount_point_path" >/dev/null 2>&1; then
    current_opts="$(findmnt -no OPTIONS -T "$subu_mount_point_path" || true)"
    if [[ ",$current_opts," != *",$opts,"* ]]; then
      echo "remounting $subu_mount_point_path with opts: $opts"
      # clean remount: unmount then mount with new opts
      sudo umount "$subu_mount_point_path"
      sudo bindfs -o "$opts" $map_opt "$subu_data_path" "$subu_mount_point_path"
    else
      echo "already mounted with compatible options: $current_opts"
    fi
  else
    echo "mounting $subu_data_path -> $subu_mount_point_path with opts: $opts"
    sudo bindfs -o "$opts" $map_opt "$subu_data_path" "$subu_mount_point_path"
  fi

  # Verify outcome
  findmnt "$subu_mount_point_path" -o TARGET,FSTYPE,OPTIONS
  echo "OK: $subu_data_path -> $subu_mount_point_path"
  if [[ "$want_suid" == "--suid" ]]; then
    echo "note: suid is enabled; setuid binaries (e.g., the gasket) can take effect on this mount."
  else
    echo "note: nosuid (default) — setuid will NOT take effect on this mount."
  fi
}

subu_bind "${1:-}" "${2:-}" "${3:-}"
