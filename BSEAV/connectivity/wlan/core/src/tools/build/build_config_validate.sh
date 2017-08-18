#!/bin/bash
#
# Script to test out changes to build_config.sh
#
# build_config.sh is a database of release default brands that are enabled
# for any given production branch. It is in the form of a script and its
# operations are read-only. So this test script, runs tests for various
# usage scenarios between an existing script and a new script.
#
# Existing production script is expected to be at $OLD_SCRIPT below
# and new one (where the user is making changes and want to test correctness)
# is expected to be pointed to by $NEW_SCRIPT or needs to be passed on
# command line "build_config_validate.sh <my-new-build-config.sh-script>"
#
# ------------------------------------------------------------------
# Usage scenarios of build_config.sh
#
# Example: To see script help and available cmd line options
#           sh build_config.sh --help
#           sh build_config.sh -h
# Example: Show default windows build brands for TOT
#           sh build_config.sh --platform windows
#           sh build_config.sh -p windows
# Example: Show default linux build brands for TOT
#           sh build_config.sh --platform linux
#           sh build_config.sh -p linux
# Example: Show default windows brands for PHOENIX2_BRANCH_6_10
#           sh build_config.sh --revision PHOENIX2_BRANCH_6_10 --platform window
#           sh build_config.sh -r PHOENIX2_BRANCH_6_10 -p window
# Example: Show default brands across all platforms
#           sh build_config.sh --show_brands
#           sh build_config.sh -b
# Example: Show all active branches and twigs
#           sh build_config.sh --show_active
#           sh build_config.sh -a
# Example: Show all default brands for AKASHI_BRANCH_5_110 program
#           sh build_config.sh --revision AKASHI_BRANCH_5_110
#           sh build_config.sh -r AKASHI_BRANCH_5_110
# Example: Show linux build brands that need 32bit resources
#           sh build_config.sh --show_32bit_brands
#           sh build_config.sh --32
# Example: Show all active coverity branches and twigs
#           sh build_config.sh --show_coverity_active
#           sh build_config.sh -c
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#

NEW_SCRIPT=$1
OLD_SCRIPT=/home/hwnbuild/src/tools/build/build_config.sh

if [ ! -x "$NEW_SCRIPT" ]; then
	echo "ERROR: New script/file to test is missing"
	echo "USAGE:"
	echo "	$0 <your-new-build_config.sh-script>"
	echo ""
	exit 1
fi

# ------------------------------------------------------------------
unittests=( \
	"bash %SCRIPT% -r tot" \
	"bash %SCRIPT% -r TRUNK" \
	"bash %SCRIPT% -r DAILY" \
	"bash %SCRIPT% -r DAILY -p macos" \
	"bash %SCRIPT% -p windows" \
	"bash %SCRIPT% -p linux" \
	"bash %SCRIPT% -r PHOENIX2_BRANCH_6_10 -p window" \
	"bash %SCRIPT% -b" \
	"bash %SCRIPT% -a" \
	"bash %SCRIPT% -r AKASHI_BRANCH_5_110" \
	"bash %SCRIPT% --32" \
	"bash %SCRIPT% --64" \
	"bash %SCRIPT% -c" \
)

# Index to show test number
n=1
for test in "${unittests[@]}"
do
	echo "====== TEST[$n] : $test ======="
	old_command=$(echo $test | sed -e "s@%SCRIPT%@$OLD_SCRIPT@g")
	old_output=$($old_command 2>&1)	

	new_command=$(echo $test | sed -e "s@%SCRIPT%@$NEW_SCRIPT@g")
	new_output=$($new_command 2>&1)	

	# Compare outputs (which is list of brands)
	if [ "$old_output" != "$new_output" ]; then
		echo "ERROR: FAILED"
		echo "	OLD: $old_command"
		echo "	NEW: $new_command"
		failedtests=("${failedtests[@]}" "$test")
		echo "Review above failure";
		echo "Press Enter to Continue:"
		read ans
	else
		echo "INFO: PASSED"
		echo "	OLD: $old_command"
		echo "	NEW: $new_command"
	fi
	n=$(expr $n + 1)
	echo ""
done

# Show failed test cases
if [ "${failedtests[*]}" != "" ]; then
	echo "ERROR: Failed Test Cases"
	for test in "${failedtests[@]}"; do
		new_command=$(echo $test | sed -e "s@%SCRIPT%@$NEW_SCRIPT@g")
		echo "ERROR: Failed: $new_command"
	done
fi
