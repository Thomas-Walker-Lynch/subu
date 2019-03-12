#!/bin/bash
chown root "$@" && \
chmod u+rsx,u-w,go+rx-s-w "$@"

