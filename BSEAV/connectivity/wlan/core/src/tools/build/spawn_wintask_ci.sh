#!/bin/bash
#
# Generic wrapper that spawns CI steps on windows nodes
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#

WORKSPACE=$1
shift
ARGS=$*
MAP_DRIVES="C:/tools/build/win_map_drives.sh"
SET_ENV="Z:/home/hwnbuild/src/tools/build/set_buildenv.sh"

if [ "$WORKSPACE" == "" -o "$ARGS" == "" ]; then
	echo "ERROR: Some arguments missing to $0"
	echo "Usage:"
	echo "$0 <workspace-path> <targets-to-build>"
	echo ""
	exit 1
fi

CURRENT_TIME=$(date '+%Y-%m-%d %H:%M:%S')

if [ ! -s "${MAP_DRIVES}" ]; then
	echo "ERROR: Missing ${MAP_DRIVES}"
	exit 1
fi

echo "Calling ${MAP_DRIVES}"
${MAP_DRIVES}

if [ ! -s "${SET_ENV}" ]; then
	echo "ERROR: Missing ${SET_ENV}"
	exit 1
fi

echo "Calling ${SET_ENV}"
${SET_ENV}

echo "Calling gmake -C ${WORKSPACE} ${ARGS}"
gmake -C ${WORKSPACE} ${ARGS}

export EXITCODE=$?
echo EXITCODE=$EXITCODE

echo "Calling bkill $LSB_JOBID"
bkill $LSB_JOBID
# Ensure commander step shell doesn't complain with exit code of bkill
echo "[$CURRENT_TIME] bkill of $LSB_JOBID done"

echo "Exiting with exitcode: $EXITCODE"
exit $EXITCODE
