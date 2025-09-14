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

# Reset address: delete the exact CIDR if present, then add it back.
reset_addr(){
  local iface=$1; local cidr=$2
  ip -4 addr del "$cidr" dev "$iface" >/dev/null 2>&1 || true
  if ip -4 addr add "$cidr" dev "$iface"; then
    logger "addr set: $iface $cidr"
  else
    logger "addr add failed (non-fatal): $iface $cidr"
  fi
}

# Ensure route using replace; log but do not fail the unit if kernel says 'exists'.
ensure_route(){
  local table=$1; local cidr=$2; local dev=$3; local via=${4:-}; local metric=${5:-}
  if [ -n "$via" ] && [ -n "$metric" ]; then
    if ip -4 route replace "$cidr" via "$via" dev "$dev" table "$table" metric "$metric" 2>/dev/null; then
      logger "route ensure: table=$table cidr=$cidr dev=$dev via=$via metric=$metric"
    else
      logger "route ensure (tolerated failure): table=$table cidr=$cidr dev=$dev via=$via metric=$metric"
    fi
  elif [ -n "$via" ]; then
    if ip -4 route replace "$cidr" via "$via" dev "$dev" table "$table" 2>/dev/null; then
      logger "route ensure: table=$table cidr=$cidr dev=$dev via=$via"
    else
      logger "route ensure (tolerated failure): table=$table cidr=$cidr dev=$dev via=$via"
    fi
  elif [ -n "$metric" ]; then
    if ip -4 route replace "$cidr" dev "$dev" table "$table" metric "$metric" 2>/dev/null; then
      logger "route ensure: table=$table cidr=$cidr dev=$dev metric=$metric"
    else
      logger "route ensure (tolerated failure): table=$table cidr=$cidr dev=$dev metric=$metric"
    fi
  else
    if ip -4 route replace "$cidr" dev "$dev" table "$table" 2>/dev/null; then
      logger "route ensure: table=$table cidr=$cidr dev=$dev"
    else
      logger "route ensure (tolerated failure): table=$table cidr=$cidr dev=$dev"
    fi
  fi
}

# Reset a policy rule by numeric preference: delete-by-pref, then add.
reset_IP_rule(){
  # Usage: reset_IP_rule <pref> <rule-args...>
  local pref=$1; shift
  ip -4 rule del pref "$pref" >/dev/null 2>&1 || true
  if ip -4 rule add "$@" pref "$pref"; then
    logger "rule set: pref=$pref $*"
  else
    logger "rule add failed (non-fatal): pref=$pref $*"
  fi
}

if want_iface x6; then
  if exists_iface x6; then reset_addr x6 10.8.0.2/32; else logger "skip: iface missing: x6"; fi
fi
if want_iface US; then
  if exists_iface US; then reset_addr US 10.0.0.1/32; else logger "skip: iface missing: US"; fi
fi
if want_iface x6; then
  if exists_iface x6; then ensure_route "x6" "0.0.0.0/0" "x6" "" ""; else logger "skip: iface missing: x6"; fi
fi
if want_iface US; then
  if exists_iface US; then ensure_route "US" "0.0.0.0/0" "US" "" ""; else logger "skip: iface missing: US"; fi
fi
if want_iface x6; then
  reset_IP_rule 17010 from "10.8.0.2/32" lookup "x6"
fi
if want_iface x6; then
  reset_IP_rule 17011 uidrange "2018-2018" lookup "x6"
fi
if want_iface US; then
  reset_IP_rule 17020 from "10.0.0.1/32" lookup "US"
fi
if want_iface US; then
  reset_IP_rule 17021 uidrange "2017-2017" lookup "US"
fi
reset_IP_rule 18050 from "10.0.0.0/24" prohibit
