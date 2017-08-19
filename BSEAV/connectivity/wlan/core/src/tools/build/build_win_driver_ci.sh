#!/bin/bash

WORKSPACE=$1
shift
ARGS=$*
CURRENT_TIME=$(date '+%Y-%m-%d %H:%M:%S')

echo "Calling C:/tools/build/win_map_drives.sh"
C:/tools/build/win_map_drives.sh

echo "Calling Z:/home/hwnbuild/src/tools/build/set_buildenv.sh"
Z:/home/hwnbuild/src/tools/build/set_buildenv.sh

echo "Calling gmake -C ${WORKSPACE} ${ARGS}"
gmake -C ${WORKSPACE} ${ARGS}

export EXITCODE=$?
echo EXITCODE=$EXITCODE

#This bkill is required since windows jobs for some reason don't exit for 10minutes
echo "Calling bkill $LSB_JOBID"
bkill $LSB_JOBID

echo "Exiting with exitcode: $EXITCODE"
exit $EXITCODE
