#!/bin/bash
# masu_subu__map_own.sh <user> <subu> [--suid]
#
# Examples:
#   sudo ./masu_subu__map_own.sh Thomas developer
#   sudo ./masu_subu__map_own.sh Thomas developer --suid

set -euo pipefail

need() { command -v "$1" >/dev/null 2>&1 || { echo "missing: $1" >&2; exit 1; }; }

subu_bind() {
  local user="$1"
  local subu="$2"
  local want_suid="${3-}"   # optional third arg

  need bindfs
  need findmnt
  need mountpoint

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

  # desired options
  local base_opts="allow_other,default_permissions,exec"
  local opts="$base_opts,nosuid"
  if [[ "$want_suid" == "--suid" ]]; then
    opts="$base_opts,suid"
  fi

  # map rule
  local map_opt="--map=${subu_user_name}/${master_user_name}:@${subu_group}/@${master_group}"

  # check if *this path* is already a mountpoint
  if mountpoint -q "$subu_mount_point_path"; then
    # only care about a few options, avoid brittle whole-string compare
    local current_opts
    current_opts="$(findmnt -no OPTIONS -T "$subu_mount_point_path" 2>/dev/null || true)"

    have_exec=$(grep -qw exec <<<"$current_opts" && echo 1 || echo 0)
    have_suid=$(grep -qw suid <<<"$current_opts" && echo 1 || echo 0)
    have_nosuid=$(grep -qw nosuid <<<"$current_opts" && echo 1 || echo 0)
    have_allow=$(grep -qw allow_other <<<"$current_opts" && echo 1 || echo 0)

    want_suid_flag=$([[ "$want_suid" == "--suid" ]] && echo 1 || echo 0)

    needs_change=0
    [[ $have_exec -eq 1 ]] || needs_change=1
    [[ $have_allow -eq 1 ]] || needs_change=1
    if [[ $want_suid_flag -eq 1 ]]; then
      [[ $have_suid -eq 1 && $have_nosuid -eq 0 ]] || needs_change=1
    else
      [[ $have_nosuid -eq 1 && $have_suid -eq 0 ]] || needs_change=1
    fi

    if [[ $needs_change -eq 1 ]]; then
      echo "remounting $subu_mount_point_path with opts: $opts"
      umount "$subu_mount_point_path" || { echo "⚠︎ umount failed; trying lazy"; umount -l "$subu_mount_point_path" || true; }
      bindfs -o "$opts" $map_opt "$subu_data_path" "$subu_mount_point_path"
    else
      echo "already mounted with compatible options: $current_opts"
    fi
  else
    echo "mounting $subu_data_path -> $subu_mount_point_path with opts: $opts"
    bindfs -o "$opts" $map_opt "$subu_data_path" "$subu_mount_point_path"
  fi

  # verify
  findmnt "$subu_mount_point_path" -o TARGET,FSTYPE,OPTIONS
  echo "OK: $subu_data_path -> $subu_mount_point_path"
  if [[ "$want_suid" == "--suid" ]]; then
    echo "note: suid is ENABLED; setuid binaries (e.g., the gasket) can take effect on this mount."
  else
    echo "note: nosuid (default) — setuid will NOT take effect on this mount."
  fi
}

# run (sudo not needed if already root)
subu_bind "${1:-}" "${2:-}" "${3:-}"
