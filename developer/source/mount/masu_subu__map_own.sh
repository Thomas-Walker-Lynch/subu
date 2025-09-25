#!/usr/bin/env bash
# usage: sudo ./masu_subu__map_own.sh <masu> <subu> [--suid]
set -euo pipefail

need(){ command -v "$1" >/dev/null 2>&1 || { echo "missing: $1" >&2; exit 1; }; }
need bindfs; need findmnt; need umount

masu="${1:?usage: $0 <masu> <subu> [--suid] }"
subu="${2:?usage: $0 <masu> <subu> [--suid] }"
want_suid=0
[[ "${3-}" == "--suid" ]] && want_suid=1

master_user="$masu"
master_group="$masu"
subu_user="${masu}-${subu}"
subu_group="${masu}-${subu}"

id "$master_user" >/dev/null
id "$subu_user"   >/dev/null

src="/home/$masu/subu_data/$subu"
tgt="/home/$masu/subu/$subu"
[[ -d "$src" ]] || { echo "no source dir: $src" >&2; exit 1; }
mkdir -p "$tgt"

# IMPORTANT: don’t stay inside the target tree while (un)mounting
cd /

base_opts="allow_other,default_permissions,exec"
opts="$base_opts,nosuid"
(( want_suid )) && opts="$base_opts,suid"

map_opt="--map=${subu_user}/${master_user}:@${subu_group}/@${master_group}"

# Peel any existing mount at tgt (use -T to match covering mount)
while findmnt -nr -T "$tgt" >/dev/null 2>&1; do
  umount "$tgt" 2>/dev/null || umount -l "$tgt" || break
done

echo "mounting $src -> $tgt  (opts: $opts)"
bindfs -o "$opts" $map_opt "$src" "$tgt"

# Verify
if findmnt -nr -T "$tgt" -o TARGET,SOURCE,FSTYPE,OPTIONS; then
  echo "OK"
  if (( want_suid )); then
    echo "note: suid is ENABLED at $tgt"
  else
    echo "note: nosuid (default) — setuid will NOT take effect at $tgt"
  fi
else
  echo "❌ bindfs did not mount at $tgt" >&2
  exit 2
fi
