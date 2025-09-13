#!/bin/bash

# Check if the correct number of arguments is passed
if [ $# -ne 1 ]; then
  echo "Usage: $0 <username>"
  exit 1
fi

user=$1

# Get the list of sub-users by calling the user_list_subu_home.sh script
subu_list=$(./user_list_subu_homedir.sh "$user")

# Check if we received any sub-users
if [ -z "$subu_list" ]; then
  echo "No sub-users found for $user."
  exit 1
fi

# Loop through the sub-users and call user_open_subu.sh for each
for subu in $subu_list; do
  echo "Opening sub-user: $subu"
  ./masu_subu__map_own.sh "$user" "$subu"
done
