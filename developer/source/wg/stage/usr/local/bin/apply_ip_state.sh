#!/usr/bin/env bash
# apply IP state for selected interfaces (addresses, routes, rules) — idempotent
set -euo pipefail

ALL_ARGS=("$@")

want_iface(){
  local t=$1
  if [ ${#ALL_ARGS[@]} -eq 0 ]; then return 0; fi
  for a in "${ALL_ARGS[@]}"; do [ "$a" = "$t" ] && return 0; done
  return 1
}

exists_iface(){ ip -o link show dev "$1" >/dev/null 2>&1; }

ensure_addr(){
  local iface=$1; local cidr=$2
  if ip -4 -o addr show dev "$iface" | awk '{print $4}' | grep -Fxq "$cidr"; then
    logger "addr ok: $iface $cidr"
  else
    ip -4 addr add "$cidr" dev "$iface"
    logger "addr add: $iface $cidr"
  fi
}

ensure_route(){
  local table=$1; local cidr=$2; local dev=$3; local via=${4:-}; local metric=${5:-}
  if [ -n "$via" ] && [ -n "$metric" ]; then
    ip -4 route replace "$cidr" via "$via" dev "$dev" table "$table" metric "$metric"
  elif [ -n "$via" ]; then
    ip -4 route replace "$cidr" via "$via" dev "$dev" table "$table"
  elif [ -n "$metric" ]; then
    ip -4 route replace "$cidr" dev "$dev" table "$table" metric "$metric"
  else
    ip -4 route replace "$cidr" dev "$dev" table "$table"
  fi
  logger "route ensure: table=$table cidr=$cidr dev=$dev${via:+ via=$via}${metric:+ metric=$metric}"
}

add_ip_rule_if_absent(){
  local needle=$1; shift
  if ! ip -4 rule show | grep -F -q -- "$needle"; then
    ip -4 rule add "$@"
    logger "rule add: $*"
  else
    logger "rule ok: $needle"
  fi
}

if want_iface x6; then
  if exists_iface x6; then ensure_addr x6 10.8.0.2/32; else logger "skip: iface missing: x6"; fi
fi
if want_iface US; then
  if exists_iface US; then ensure_addr US 10.0.0.1/32; else logger "skip: iface missing: US"; fi
fi
if want_iface x6; then
  add_ip_rule_if_absent "from 10.8.0.2/32 lookup x6" from "10.8.0.2/32" lookup "x6" pref 17000
fi
if want_iface x6; then
  add_ip_rule_if_absent "uidrange 2018-2018 lookup x6" uidrange "2018-2018" lookup "x6" pref 17010
fi
if want_iface US; then
  add_ip_rule_if_absent "from 10.0.0.1/32 lookup US" from "10.0.0.1/32" lookup "US" pref 17000
fi
if want_iface US; then
  add_ip_rule_if_absent "uidrange 2017-2017 lookup US" uidrange "2017-2017" lookup "US" pref 17010
fi
add_ip_rule_if_absent "from 10.0.0.0/24 prohibit" from "10.0.0.0/24" prohibit pref 18050
