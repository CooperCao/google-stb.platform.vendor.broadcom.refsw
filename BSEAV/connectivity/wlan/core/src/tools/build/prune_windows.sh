#! /bin/bash
#
# prune_windows.sh - remove older directories (tagged and nightly)
#
# $Copyright (C) 2004 Broadcom Corporation
#
# $Id: prune_windows.sh,v 12.13 2010-10-27 20:49:16 $
#
# SVN: $HeadURL$
#

# Keep some options similar to build_windows, which used to prune
# (but only in what it was building, so it never got tagged dirs)

function usage {
cat <<EOF
    Usage: $0 [ -x N ] [-t] [-b brand | -p prefix]
      -x N  Remove build directories older than N days (default 2)
      -t    Test: don't actually remove
      -b    Specify single brand to prune in
      -p    Specify a prefix (prunes <prefix>*)
    In all cases, only considers NIGHTLY or <word>_BRANCH/<word>_TWIG/REL_<num>_<num>
    If unspecified, prunes NIGHTLY and all BRANCH/TWIG/REL subdirs
EOF
}

# Use the standard BUILD_BASE from build_windows.sh
: ${BUILD_BASE:="e:/build"}
ts=`date '+%Y%m%d'`
mktmp=`mktemp XXXXXX`; rm -f ${mktmp};
tempscript="c:/temp/prune/prune_windows_${ts}_${mktmp}.bat"
mkdir -p c:/temp/prune
rm -v -f ${tempscript}

PATH=/usr/bin:/bin:${PATH}
AGE=3
testing=
verbose=
while getopts 'hvtx:d:b:p:l:' OPT
    do
	case "$OPT" in
        x)
            AGE=$OPTARG;
            ;;
        b)
            BRAND=$OPTARG;
            ;;
        p)
            PREFIX=$OPTARG;
            ;;
	t)
	    testing=true
	    verbose=true
	    ;;
	v)
	    verbose=true
	    ;;
	d)
	    BUILD_BASE=$OPTARG
	    ;;
	l)
	    LOGFILE=$OPTARG
	    ;;
	h)
	    usage;
	    exit;
	    ;;
	:)
	    echo "Missing required argument for option \"-$OPTARG\".\n";
	    usage;
	    ;;
	?)
	    echo "Unrecognized option - \"$OPTARG\".\n";
	    usage;
	    ;;
	esac
done

if [ -n "${BRAND}" ] && [ -n "${PREFIX}" ]; then
    echo "Cannot specify both brand and prefix\n"
    usage;
fi

# Go to build_base to do the work
cd ${BUILD_BASE}
if [ $? != 0 ]; then
    echo "Cannot cd to ${BUILD_BASE}"
    exit 1
fi

echo "Current location: $(pwd)"
echo "BRAND=${BRAND}"
echo "PREFIX=${PREFIX}"
checked=0
removed=0

  # Find subdirs that are older than 1 day to delete,
  # unless the name contains keep string.
  for bdir in $(find . -mindepth 2 -maxdepth 2 -type d \
               -daystart -not -mtime -${AGE} | grep -vi keep)
  do
    if [ -n "$testing" ]; then
	[ -n "$verbose" ] && echo "Would remove ${bdir}"
    else
	[ -n "$verbose" ] && echo "Mark ${bdir} for removal"
	[ -n "$LOGFILE" ] && echo "`date`: Mark ${bdir} for removal" >> ${LOGFILE}
	# Cygwin locks up deleting folders, use doscmds to prune
	bdir_dos="`cygpath -a -w ${bdir}`"
	echo "@title 'Pruning ${bdir_dos} on ${ts}'"     >> ${tempscript}
	echo "@echo [%date% %time%] Start ${bdir_dos}"   >> ${tempscript}
	echo "@df -k \"`dirname $bdir`\""                >> ${tempscript}
	echo "@echo rmdir /S /Q \"${bdir_dos}\""         >> ${tempscript}
	echo "@rmdir /S /Q \"${bdir_dos}\""              >> ${tempscript}
	echo "@df -k \"`dirname $bdir`\""                >> ${tempscript}
	echo "@echo [%date% %time%] End ${bdir_dos}"     >> ${tempscript}
	echo "@echo ===================================" >> ${tempscript}
    fi
    removed=$(($removed + 1))
  done

if [ -s "${tempscript}" ]; then
   echo "Running ${tempscript}"
   cmd /c "${tempscript//\//\\}"
else
   echo "Nothing to prune in $(pwd)"
fi

# Summary if requested
if [ -n "$verbose" ]; then
    echo
    echo "Removed $removed build subdirectories."
    echo
fi
