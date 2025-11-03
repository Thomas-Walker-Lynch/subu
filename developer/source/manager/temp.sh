# from: /home/Thomas/subu_data/developer/project/active/subu/developer/source/manager

set -euo pipefail

echo "== 1) Backup legacy-prefixed modules =="
mkdir -p _old_prefixed
for f in subu_*.py; do
  [ -f "$f" ] && mv -v "$f" _old_prefixed/
done
[ -f subu_worker_bpf.py ] && mv -v subu_worker_bpf.py _old_prefixed/ || true

echo "== 2) Ensure only the new module names remain =="
# Keep these (already present in your tar):
#   CLI.py  core.py  text.py  worker_bpf.py  bpf_force_egress.c
ls -1

echo "== 3) Make CLI runnable as 'subu' =="
# Make sure CLI has a shebang; add if missing
if ! head -n1 CLI.py | grep -q '^#!/usr/bin/env python3'; then
  (printf '%s\n' '#!/usr/bin/env python3' ; cat CLI.py) > .CLI.tmp && mv .CLI.tmp CLI.py
fi
chmod +x CLI.py
ln -sf CLI.py subu
chmod +x subu

echo "== 4) Quick import sanity =="
# Fail if any of the remaining files still import the old module names
bad=$(grep -R --line-number -E 'import +subu_|from +subu_' -- *.py || true)
if [ -n "$bad" ]; then
  echo "Found old-style imports; please fix:" >&2
  echo "$bad" >&2
  exit 1
fi

echo "== 5) Show version and help =="
./subu version || true
./subu help    || true
./subu         || true  # should print usage by default

echo "== Done. If this looks good, you can delete _old_prefixed when ready. =="
