#!/bin/bash

svntrunk=http://svn.sj.broadcom.com/svn/wlansvn/proj/trunk

srcrepo=$svntrunk

function usage () {
    echo "Usage: merge_range -c rev [ svn-src-path ]"
    echo "Usage: merge_range -r rev1:rev2 [ svn-src-path ]"
    echo "  Perform svn merge commands on each changed file in the give SVN range from"
    echo "  the SVN src path, either a work area or URL."
    echo "  Current directory should be parent of \'src\' in a checkout area."
    echo ""
    echo "Options:"
    echo "  -c rev       : the change made by revision ARG (like -r ARG-1:ARG)"
    echo "                 If ARG is negative this is like -r ARG:ARG-1"
    echo "  -r rev1:rev2 : the changes made from rev1 to rev2"
}

function check_nargs () {
    if [ $1 -lt $2 ]; then
	echo "too few args"
	usage;
	exit -1
    fi
}

# sets vars startrev, endrev to the revsions in given range value "r1:r2"
function split_range () {
    # find the ':' separator
    local r=$1
    local sep=`expr index $r ':'`

    # sep == 0 if not found
    if [ $sep -eq 0 ]; then
	echo "Error: unable to parse revision range \"$r\" as r1:r2"
	exit -2
    fi

    startrev=${r:0:$((sep-1))}
    endrev=${r:$sep}
}

# make sure we have at least 1 parameter
check_nargs $# 1

opt=$1
if [ $opt == "-h" -o $opt == "--help" ]; then
    usage;
    exit 0;
elif [ $opt == "-c" -o $opt == "--change" ]; then
    # make sure we have a value for the -c option
    check_nargs $# 2

    if [ `expr index $2 ':'` -gt 0 ]; then
	echo "Error: $opt take a single revision, found \"$2\""
	usage;
	exit -2
    fi

    echo "Merging Revision: $2"
    range_arg="$opt $2"
    startrev=""
    endrev=$2

    # shift off the 2 parameters
    shift 2
elif [ $opt == "-r" -o $opt == "--revision" ]; then
    # make sure we have a value for the -r option
    check_nargs $# 2

    echo "Merging Revision Range: $2"
    range_arg="$opt $2"

    # set up startrev, endrev
    split_range $2

    # shift off the 2 parameters
    shift 2
else
    echo "Error: unknow option: $opt"
    usage;
    exit -1
fi

# take a repostory argument
if [ $# -ge 1 ]; then
    srcrepo=$1
    shift 1

    # Check for a valid repository
    badrepo=0
    svn info $srcrepo > /dev/null 2>&1 || badrepo=1

    if [ $badrepo -eq 1 ]; then
	echo "Error: problem with repository argument \"$srcrepo\""
	usage;
	exit -1
    fi
fi

# check for extra params
if [ $# -gt 1 ]; then
    echo "too many args"
    usage;
    exit -1
fi

#echo "DBG: range_arg $range_arg"
#echo "DBG: startrev $startrev"
#echo "DBG: endrev $endrev"

dest=.

function fileset () {
    local r=$1
    # find all files changed in the range of revisions,
    # in the /src subdir of the src repository
    svn diff --summarize $r $srcrepo/src | \
	awk "/^[M|A].*http:/ { sub(\"$srcrepo/\", \"\"); print \$2 }"
#	awk "/^[^D].*http:/ { sub(\"$srcrepo/\", \"\"); print \$2 }"
}

echo "Finding fileset ..."
fileset=`fileset "$range_arg"`
echo "Found `echo $fileset | wc -w` files."
echo ""

for f in $fileset; do
    if [ -e $dest/$f ]; then
	echo "Apply change $range to $f"
	svn merge --accept postpone $range_arg $srcrepo/$f $dest/$f
	echo ""
    else
	echo "Adding $f @$endrev"
	svn copy $srcrepo/$f -r $endrev $dest/$f
	echo ""
    fi
#    echo $f
done
