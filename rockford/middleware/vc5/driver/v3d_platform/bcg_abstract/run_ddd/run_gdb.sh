#!/bin/bash

# Checks that the toolchaine path is in PATH and
# starts the correct version of gdb passing on all arguments.

# Requirements to use this script on your own linux machine (in contrast to the build server):
# - you should have access to fs-bri-01.bri.broadcom.com:/vol/vol02904/stbopt_p from your linx machine
#   To have access to this share you either need to log into your linux machine using Broadcom NIS or
#   to be logged with a user that has the same uid than your user on the build server
# - fs-bri-01.bri.broadcom.com:/vol/vol02904/stbopt_p should be mounted into /opt_p
# - ln -s /opt_p/brcm /opt/brcm
# - ln -s /opt_p/toolchains_303 /opt/toochains (this might have to change sometime?)

if [[ :$PATH: != *:"${TOOLCHAIN_PATH}":* ]] ; then
  export PATH=${TOOLCHAIN_PATH}:$PATH
fi

exec ${gdb_name} "$@"