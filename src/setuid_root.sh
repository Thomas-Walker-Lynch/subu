#!/bin/bash
# must be run under sudo
# be sure to cat it before running it, better yet just type this in manually
#

chown root $1 && \
chmod u+rsx,u-w,go+rx-s-w $1

