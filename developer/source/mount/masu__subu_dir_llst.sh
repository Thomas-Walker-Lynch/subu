#!/bin/bash

# Function to list sub-users in /home/<user>/subu
subu_list() {
  local user=$1
  local subu_dir="/home/$user/subu"

  if [ ! -d "/home/$user" ]; then
    echo "Error: /home/$user does not exist!"
    return 1
  fi

  if [ ! -d "$subu_dir" ]; then
    echo "Error: $subu_dir does not exist!"
    return 1
  fi

  # List all sub-users in the subu directory
  find "$subu_dir" -maxdepth 1 -mindepth 1 -type d -exec basename {} \;
}

# Run the function with the user as an argument
subu_list "$1"
