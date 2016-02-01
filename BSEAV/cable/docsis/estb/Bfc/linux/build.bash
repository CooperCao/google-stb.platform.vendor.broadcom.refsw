#!/bin/bash

set -a

# Load common function definitions from our own directory.
scriptDir=${0%/*}
source ${scriptDir}/functions.bash

case "$1" in
    "processor")
        shift
        BuildProcessor $*
        BfcBuildResult=$?
        ;;
    "product")
        shift
        BuildProduct $*
        BfcBuildResult=$?
        ;;
esac

exit $BfcBuildResult

