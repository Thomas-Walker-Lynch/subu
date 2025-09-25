#!/bin/env bash
# masu__map_own_all.sh

set -euo pipefail
masu="${1:?Usage: $0 <masu> [--suid=subu1,subu2] }"
suid_list="${2-}"  # optional: --suid=a,b,c

# Build a set for quick membership checks
want_suid() {
  [[ -n "$suid_list" ]] || return 1
  [[ "$suid_list" =~ ^--suid= ]] || return 1
  IFS=',' read -r -a arr <<< "${suid_list#--suid=}"
  for n in "${arr[@]}"; do [[ "$n" == "$1" ]] && return 0; done
  return 1
}

subus=$(./masu__subu_dir_list.sh "$masu")
[[ -n "$subus" ]] || { echo "No sub-users found for $masu"; exit 1; }

while IFS= read -r s; do
  [[ -n "$s" ]] || continue
  echo "Opening sub-user: $s"
  if want_suid "$s"; then
    sudo ./masu_subu__map_own.sh "$masu" "$s" --suid
  else
    sudo ./masu_subu__map_own.sh "$masu" "$s"
  fi
done <<< "$subus"
