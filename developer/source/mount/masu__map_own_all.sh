#!/usr/bin/env bash
# usage: sudo ./masu__map_own_all.sh <masu> [--suid=subu1,subu2]
set -euo pipefail

masu="${1:?Usage: $0 <masu> [--suid=subu1,subu2] }"
suid_list="${2-}"  # optional --suid=a,b,c

want_suid() {
  [[ -n "$suid_list" && "$suid_list" =~ ^--suid= ]] || return 1
  IFS=',' read -r -a arr <<< "${suid_list#--suid=}"
  for n in "${arr[@]}"; do [[ "$n" == "$1" ]] && return 0; done
  return 1
}

# List subu names from authoritative source
subu_root="/home/$masu/subu_data"
[[ -d "$subu_root" ]] || { echo "No subu_data dir for $masu: $subu_root" >&2; exit 1; }
mapfile -t subus < <(find "$subu_root" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort -u)
[[ ${#subus[@]} -gt 0 ]] || { echo "No sub-users found for $masu"; exit 1; }

for s in "${subus[@]}"; do
  echo "Opening sub-user: $s"
  if want_suid "$s"; then
    sudo ./masu_subu__map_own.sh "$masu" "$s" --suid
  else
    sudo ./masu_subu__map_own.sh "$masu" "$s"
  fi
done
