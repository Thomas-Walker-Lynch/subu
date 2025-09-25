# masu_subu__map_own.sh
#!/bin/bash
# usage: sudo ./masu_subu__map_own.sh <masu> <subu> [--suid]
set -euo pipefail

need(){ command -v "$1" >/dev/null 2>&1 || { echo "missing: $1" >&2; exit 1; }; }

want_suid=0
case "${3-}" in
  --suid) want_suid=1 ;;
  "" ) ;;
  * ) echo "unknown option: $3" >&2; exit 2 ;;
esac

masu="${1:?usage: $0 <masu> <subu> [--suid] }"
subu="${2:?usage: $0 <masu> <subu> [--suid] }"

need bindfs; need findmnt; need mountpoint; id "$masu" >/dev/null
id "${masu}-${subu}" >/dev/null

src="/home/$masu/subu_data/$subu"
tgt="/home/$masu/subu/$subu"
[[ -d "$src" ]] || { echo "Error: source dir '$src' does not exist" >&2; exit 1; }
mkdir -p "$tgt"

base_opts="allow_other,default_permissions,exec"
desired_opts="$base_opts,$([[ $want_suid -eq 1 ]] && echo suid || echo nosuid)"
map_opt="--map=${masu}-${subu}/${masu}:@${masu}-${subu}/@${masu}"

opts_have() { grep -qw "$1"; }

# Peel off incorrect layers until either:
#  - nothing is mounted on $tgt, or
#  - the top-most layer is a bindfs of $src with desired opts
while mountpoint -q "$tgt"; do
  read -r FSTYPE SOURCE OPTIONS < <(findmnt -T "$tgt" -no FSTYPE,SOURCE,OPTIONS)
  if [[ "$FSTYPE" != fuse*bindfs* && "$FSTYPE" != fuse.bindfs && "$FSTYPE" != fuse3.bindfs ]]; then
    echo "⚠︎ '$tgt' is a mountpoint but not bindfs (fstype=$FSTYPE); unmounting this layer…"
    umount "$tgt" || umount -l "$tgt" || true
    continue
  fi

  # Normalize desired
  want_suid_kw=$([[ $want_suid -eq 1 ]] && echo suid || echo nosuid)
  # Check source and essential flags without brittle full-string compare
  if [[ "$SOURCE" == "$src" ]] \
     && opts_have <<<"$OPTIONS" allow_other \
     && opts_have <<<"$OPTIONS" exec \
     && opts_have <<<"$OPTIONS" "$want_suid_kw" \
     && ! opts_have <<<"$OPTIONS" "$([[ $want_suid -eq 1 ]] && echo nosuid || echo suid)"; then
    echo "already mounted OK: $tgt ← $src ($OPTIONS)"
    exit 0
  fi

  echo "unmounting incorrect layer on $tgt (src=$SOURCE opts=$OPTIONS)…"
  umount "$tgt" || umount -l "$tgt" || true
done

echo "mounting $src -> $tgt  (opts: $desired_opts)"
bindfs -o "$desired_opts" $map_opt "$src" "$tgt"
findmnt "$tgt" -o TARGET,SOURCE,FSTYPE,OPTIONS
echo "OK"
