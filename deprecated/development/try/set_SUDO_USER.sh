#!/usr/bin/bash
#
#  Shows that a regular user can set environment variables with the same names as
#  sudo does.  Hence it is possible to confuse a script that relies upon the sudo
#  variables.  
#

export SUDO_USER=jones
./SUDO_USER_1.sh



