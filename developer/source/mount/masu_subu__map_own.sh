#!/usr/bin/env bash
# usage: sudo ./masu_subu__map_own.sh <masu> <subu> [--suid]
set -euo pipefail

need(){ command -v "$1" >/dev/null 2>&1 || { echo "missing: $1" >&2; exit 1; }; }

masu="${1:?usage: $0 <masu> <subu> [--suid] }"
subu="${2:?usage: $0 <masu> <subu> [--suid] }"
want_suid=0
case "${3-}" in
  --suid) want_suid=1 ;;
  "" ) ;;
  * ) echo "unknown option: $3" >&2; exit 2 ;;
esac

need bindfs; need findmnt; need mountpoint; id "$masu" >/dev/null; id "${masu}-${subu}" >/dev/null

src="/home/$masu/subu_data/$subu"
tgt="/home/$masu/subu/$subu"

[[ -d "$src" ]] || { echo "Error: source dir '$src' does not exist" >&2; exit 1; }
mkdir -p "$tgt"

base_opts="allow_other,default_permissions,exec"
desired_opts="$base_opts,$([[ $want_suid -eq 1 ]] && echo suid || echo nosuid)"
map_opt="--map=${masu}-${subu}/${masu}:@${masu}-${subu}/@${masu}"

# Peel any existing stack at $tgt (no matter what it is)
while mountpoint -q "$tgt"; do
  umount "$tgt" 2>/dev/null || umount -l "$tgt" || break
done

echo "mounting $src -> $tgt  (opts: $desired_opts)"
bindfs -o "$desired_opts" $map_opt "$src" "$tgt"

# If, for any reason, multiple identical layers ended up stacked, peel until one remains.
while [ "$(findmnt -nr -T "$tgt" | wc -l)" -gt 1 ]; do
  umount "$tgt" || umount -l "$tgt" || break
done

# Show only the bindfs line (or the only remaining one)
findmnt -nr -T "$tgt" -o TARGET,SOURCE,FSTYPE,OPTIONS | head -n1
echo "OK"
if (( want_suid )); then
  echo "note: suid enabled; setuid binaries can take effect on this mount."
else
  echo "note: nosuid (default) — setuid will NOT take effect on this mount."
fi
