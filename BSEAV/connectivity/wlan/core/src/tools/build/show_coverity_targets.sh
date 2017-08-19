#!/bin/sh
#
# Show active coverity targets
# This script is not used by any automation, it is used to just show
# some info on enabled coverity branches and targets inside them
#
# $Id$
#

build_config=/home/hwnbuild/src/tools/build/build_config.sh
target_config=/home/hwnbuild/src/tools/release/coverity-target-info.mk
NULL=/dev/null

echo "INFO: Active Coverity Targets"

for branch in `$build_config -c | fmt -1`
do
	for target in `gmake -f $target_config TAG=$branch show_coverity_targets | fmt -1 2> $NULL`
	do
		echo "$branch $target" | awk '{printf "%-30s %s\n", $1, $2}'
	done
done
