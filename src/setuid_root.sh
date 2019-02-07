#!/bin/bash
# must be run under sudo
#

chown root $1 && \
chmod u+rsx,u-w,g+rx-s $1

