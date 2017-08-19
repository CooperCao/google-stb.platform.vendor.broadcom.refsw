#!/bin/bash
#
# Show outstanding defects in Coverity targets. Formulate URL queries about defects to
# display the info in the Coverity Connect UI.
#
# Usage: show_coverity_status.sh BRANCH TARGET
#
# Output: URL of Coverity query
#    Can be cut-and-pasted into a browser
#
# Examples:
#   show_coverity_status.sh AARDVARK_BRANCH_6_30
#   show_coverity_status.sh TRUNK build_win7_driver_wl_x64
#
# You can say "NIGHTLY" instead of "TRUNK" if you want.
#
# $Id: $
#
BRANCH=$1
TARGET=$2
COV_TARGET=`echo $TARGET | tr "/" "_"`
COV_STREAM=$BRANCH"__$COV_TARGET"

BASE_URL="http://wlansw-coverity.sj.broadcom.com:8080/query/defects.htm"
QUERY_URL="$BASE_URL?stream=$COV_STREAM&outstanding=true"

echo $QUERY_URL
