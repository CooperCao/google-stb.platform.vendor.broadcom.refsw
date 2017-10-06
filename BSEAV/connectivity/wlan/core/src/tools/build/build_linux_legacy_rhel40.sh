#!/bin/bash
#
# Linux top level build script: submit to LSF, checkout and run makefile
#
# Contact: hnd-software-scm-list
#
# $Copyright (C) 2004 Broadcom Corporation
# 
# $Id: build_linux.sh 376213 2012-12-21 21:15:00Z $
#
# SVN: $HeadURL$
#

# ###########################################################################
# DHD and Dongle Builds use xrhel.sj.broadcom.com for LSF submit and queries
# either via rsh or ssh. If RSHHOST is set, then it overrides what is defined
# in makefiles (src-dongle-image-launch.mk). Even SSHOPTS can be set to specify
# certain ssh options like 'StrictHostKeyChecking no' or 'ConnectTimeout=5'
# Currently 'StrictHostKeyChecking no' is default ssh option.
# Since hwnbuild user account is used for all platforms, setting the options
# here is preferable than doing in $HOME/.ssh/config file.
# export RSHHOST="xl-sj1-14"
# export RSHHOST="lc-sj1-3561"
# export SSHOPTS="-o 'StrictHostKeyChecking=no' -o 'ConnectTimeout=5'"
# Common longer wait-times for private firmware builds by dhd builds, until
# automation to reduce fw build stress is implemented in build scripts
export HNDRTE_WAITTIME=120
export HNDRTE_ITERS=60
# ###########################################################################

# Default options
SCRIPT_VERSION='$Id: build_linux.sh 376213 2012-12-21 21:15:00Z $'
SCRIPT_NAME=$0
BUILD_BASE=
BUILDS_TO_KEEP=
# default old build keep count for tot and tob builds
BUILDS_TO_KEEP_TOB=2
BUILDS_TO_KEEP_TOT=7
OVERRIDE=
LSFUSER=hwnbuild
BLDUSER=hwnbuild
XLINUX="xlinux.sj.broadcom.com"
AUTH=/home/$BLDUSER/.restricted/passwd
HOSTOS=$(uname -s)
TODAY=$(date  '+%Y%m%d')
BUILD_LIST="hnd-build-list"
MAILTO=
# Default LSF queue to use for software builds is sj-wlanswbuild
# 'sj-wlanswbuild' has rhel4 64bit resources and limited 32bit resources
# There are no RHEL5 nodes in this queue yet
WLAN_LSF_QUEUE=wlanswrhel4
LSF=sj-${WLAN_LSF_QUEUE}
LSF32BIT=sj-${WLAN_LSF_QUEUE}

NULL=/dev/null
# Record $0 session history here
SESSIONLOGDIR=/projects/hnd/swbuild/build_admin/logs/nightly/linux
URLPFX="http://home.sj.broadcom.com"
MISC_DIR=misc
TAG=
ADMINMAILTO="hnd-software-scm-list@broadcom.com"
SVNCMD="svn --non-interactive"
SVNTZ="`date '+%z'`"
export VCTOOL=svn

if [ "$TEMP" == "" ]; then
   TEMP=/tmp
fi

## List of logs that are created by build brands launched by this script
## This is full $MAKE log
RLSLOG=,release.log
## This is build user's env at the time $MAKE was launched
ENVLOG=${MISC_DIR}/,env.log
## Flag to indicate the build happened on a RHEL4 or RHEL5 system
# RHEL4 == 2.6.9-89.0.9.ELsmp
RHEL4BUILD=${MISC_DIR}/_RHEL4_BUILD
# RHEL5 == 2.6.18-238.5.1.el5
RHEL5BUILD=${MISC_DIR}/_RHEL5_BUILD
## A copy of email message on build completion preserved here
MAILLOG=${MISC_DIR}/,mail.log
## Details on when aged or failed build iterations are purged
MOVELOG=${MISC_DIR}/,move.log
## LSF job info, to see how long it took to start the build from launch
## LSF jobs are submitted as job arrays (called lsf batch job)
LSFJOBLOG=${MISC_DIR}/,lsf_job.log
## LSF batch is current build brand job
LSFBJOBLOG=${MISC_DIR}/,lsf_bjob.log
SESSIONLOG=${SESSIONLOGDIR}/build_${TODAY}.log
## Firmware (dongle) image build log
DONGLE_RLSLOG=,dongle_image_release.log
## Firmware (dongle) image build error log (if any)
DONGLE_ERRORLOG=,dongle_image_errors.log
## Errors go to this logfile, if a $MAKE exits with an error code
ERRORLOG=,build_errors.log
## Flag to indicate build success
SUCCESSLOG=,succeeded
## If $MAKE exits zero, but ,release.log contains errors record them here
IGNORE_ERRORLOG=,build_errors_ignored.log
IGNORE_FILE=_SOME_BUILD_ERRORS_IGNORED
## If build fails or interrupted, mark it as incomplete with this flag file
WARN_FILE=_WARNING_PARTIAL_CONTENTS_DO_NOT_USE
WIP_FILE=_WARNING_BUILD_IN_PROGRESS
## If CVS is checked out in place of SVN, mark with this warning file
WARN_VCTOOL_FILE=_WARNING_WRONG_CHECKOUT_FOUND

## Preserve important built bits as per preserved_built_objects.txt
## These go to build_<platform>/PRESERVED folder
PRESERVE_LIST="/home/hwnbuild/src/tools/release/preserve_built_objects.txt"
PRESERVE_CTIME=120
PRESERVE_LOGS="$RLSLOG"
PRESERVE_LOGS="$PRESERVE_LOGS $ERRORLOG"
PRESERVE_LOGS="$PRESERVE_LOGS $IGNORE_ERRORLOG"
PRESERVE_LOGS="$PRESERVE_LOGS $SUCCESSLOG"
PRESERVE_LOGS="$PRESERVE_LOGS $ENVLOG"
PRESERVE_LOGS="$PRESERVE_LOGS $DONGLE_RLSLOG"
PRESERVE_LOGS="$PRESERVE_LOGS $DONGLE_ERRORLOG"

## /projects/hnd/swbuild is symlink to /prjects/hnd_swbuild
## When destination is either of them treat them same
BUILD_BASE_LINUX=$(cd /projects/hnd_swbuild/build_linux; pwd -P)
BUILD_BASE_LINUX_MOUNT=/projects/hnd/swbuild/build_linux

# Custom firmware build dirs for GCLIENT transition
BUILD_BASE_LINUX_GCLIENT=/projects/hnd_swbuild/build_linux/GCLIENT_TRANSITION/

## This is used to store privately launched firmware/dongle builds
BUILD_BASE_LINUX_PREBUILD=/projects/hnd.swbuild/build_linux/TEMP/prebuild

## If the SVN repository path pointed to 'users' area, the builds
## are redirected to following user specific path, away from production dirs
BUILD_BASE_LINUX_USERS=/projects/hnd_swbuild/build_linux/USERS
BUILD_BASE_LINUX_USERS_RE=/projects.hnd.swbuild.build_linux.USERS

## Workspace to check resource needs of brand makefiles
## (i.e. whether a build can go to a 32bit or 64bit build node)
RESCHECK_DIR=/projects/hnd/swbuild/build_admin/logs/tmp/brand_check
## Workspace to check if a brand is parallel EC emake compatible
EMAKECHECK_DIR=/projects/hnd/swbuild/build_admin/logs/tmp/emake_check

## REL builds go to a pool of non-scratch disks $BUILD_LINUX_REL_DISKS
## List of disks on which linux REL builds go
## Try looking for disks that are less than 95% full
BUILD_LINUX_REL_DISKS=""
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext11/build_linux_ext11"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext12/build_linux_ext12"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext13/build_linux_ext13"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext14/build_linux_ext14"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext15/build_linux_ext15"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext16/build_linux_ext16"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext17/build_linux_ext17"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext18/build_linux_ext18"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext19/build_linux_ext19"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext20/build_linux_ext20"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext21/build_linux_ext21"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext22/build_linux_ext22"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext23/build_linux_ext23"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext24/build_linux_ext24"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext25/build_linux_ext25"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext26/build_linux_ext26"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext27/build_linux_ext27"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext28/build_linux_ext28"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext29/build_linux_ext29"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext30/build_linux_ext30"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext31/build_linux_ext31"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext32/build_linux_ext32"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext33/build_linux_ext33"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext34/build_linux_ext34"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext35/build_linux_ext35"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext36/build_linux_ext36"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext37/build_linux_ext37"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext38/build_linux_ext38"
BUILD_LINUX_REL_DISKS="${BUILD_LINUX_REL_DISKS} /projects/hnd_swbuild_ext39/build_linux_ext39"

## Currently TOB builds are sent to hnd_swbuild_ext41.
BUILD_BASE_SCRATCH=/projects/hnd_swbuild_ext41/build_linux_ext41

## List of disks on which linux TOB builds go
## TODO: TOB disk choice is made in this BUILD_LINUX_TOB_DISKS
BUILD_LINUX_TOB_DISKS=""
BUILD_LINUX_TOB_DISKS="${BUILD_LINUX_TOB_DISKS} /projects/hnd_swbuild_ext42/build_linux_ext42"
BUILD_LINUX_TOB_DISKS="${BUILD_LINUX_TOB_DISKS} /projects/hnd_swbuild_ext43/build_linux_ext43"
BUILD_LINUX_TOB_DISKS="${BUILD_LINUX_TOB_DISKS} /projects/hnd_swbuild_ext44/build_linux_ext44"
BUILD_LINUX_TOB_DISKS="${BUILD_LINUX_TOB_DISKS} /projects/hnd_swbuild_ext45/build_linux_ext45"

# Threshold of local disk-usage on local disk(MB), below which build
# skips local disk and goes back to network drive build
# Applies only if LOCAL_BUILD flag is set
BUILD_LOCAL_MINDU=25000
# Default local build flag is turned on
# NOTE: For now there is no flag to turn off local build and may not be needed
LOCAL_BUILD=0
# Allow upto 50 unique local temporary build dirs for a given branch
# and brand on a given node. In practive more than 10 are never used
LOCAL_BUILD_ITERS=50
# Local build root
LOCAL_BUILD_ROOT=/tmp/$WLAN_LSF_QUEUE

declare -a GMAKEFLAGS

# Given a util needed by build script, search it in gallery
# followed by build user's staging area and then in build script launch dir
# Most of the cases, the first hit is successful
function search_util {
	util_to_find=$1
	util_found_status=1
	util_path_found=""

	net_path1=/projects/hnd/software/gallery/src/tools/build/
	net_path2=/home/hwnbuild/src/tools/build/
	net_path3=$(dirname $SCRIPT_NAME)

	for net_path in $net_path1 $net_path2 $net_path3
	do
		if [ -s "${net_path}/${util_to_find}" ]; then
			util_path_found="${net_path}/${util_to_find}"
			util_found_status=0
			break
		fi
	done
	# If util isn't found, we can't proceed any further
	if [ ! -s "$util_path_found" ]; then
		echo "ERROR: $util_to_find is needed, but it can't be found"
		echo "ERROR: at $net_path1"
		echo "ERROR: or $net_path2"
		echo "ERROR: or $net_path3"
		exit 1
	fi
	return $util_found_status
}

# When build is interrupted, append to build error log
function trap_func {
        signal_found=$(($?-128))

        echo "${TAG:-TOT} $BRAND full build log at: ${RLSLOG}" >> $ERRORLOG
        tail -10 ${RLSLOG}                                     >> $ERRORLOG
        echo ""                                                >> $ERRORLOG
        echo "WARN: Build termination detected at `date`"
        echo "WARN: signal=$signal_found on $(hostname)"
        echo "WARN: Build termination detected at `date`"      >> $ERRORLOG
        echo "WARN: signal=$signal_found on $(hostname)"       >> $ERRORLOG

        # Cleanup on interrupt
        if [ -n "$LOCAL_BUILD" -a -d "$LOCAL_BASE" ]; then
           if [ -d "$BUILD_DIR" ]; then
              # Since the build is terminated, just copy release and error logs
              # to network path
              cp -pv $LOCAL_BASE/{${RLSLOG},${ERRORLOG}} $BUILD_DIR/
           fi
           echo "rm -rf '$LOCAL_BASE'"
           rm -rf "$LOCAL_BASE"
           local_date_dir=$(dirname $LOCAL_BASE)
           local_random_dir=$(dirname $local_date_dir)
           rmdir $local_date_dir
           rmdir -v $local_random_dir
        fi
        exit 1
}

trap trap_func SIGHUP SIGINT SIGTERM

# Search utils used by this script in multiple network path heirarchy
# This takes care of stale NFS issues, if any that used to cause
# build scripts to fail.
# TO-DO: Move this to a common script template for all
# TO-DO: build scripts to use or source from

## This is used to filter error patterns and simplify error log
## and find relevant svn checkins breaking the build
search_util "find_build_errors"
FIND_BUILD_ERRORS=$util_path_found

## Build brand config file to find brands given a branch or platform
search_util "build_config.sh"
BUILD_CONFIG=$util_path_found

## Utility to sort build iterations within a brand and show aged iterations
## that can be deleted
search_util "show_old_builditers.pl"
SHOW_OLD_BUILDITERS=$util_path_found
SHOW_OLD_BUILDITERS="perl $SHOW_OLD_BUILDITERS"

## Show the agent node, where the last error occured in a clustered E.C build
search_util "ecFindLastAgent.tcl"
FIND_ECAGENT=$util_path_found
FIND_ECAGENT="tclsh $FIND_ECAGENT"

## Electric Make wrapper
search_util "hndmake.sh"
HNDMAKE=$util_path_found

## Ensure that cmd line option blocks below are arranged
## alphabetically. Some internal/debug flags are do not
## appear in help screen. See Options/getopt section below
# Usage
usage ()
{
cat <<EOF
Usage: $0 -d dir [other options]

-b brands
	Brands to build (REQUIRED for tagged builds)
        (default: brands list comes from build_config.sh)
	To build more than one brand use "brand1 brand2 brand3 ... brandX"
	Note: If you rsh to another host to launch builds, use '"brand1 brand2"'
-c cutoff_time|svnrev
        This switch takes either cutoff time or svn revision number.
        * Cvs cutoff time (yyyy-mm-dd hh:mm format or other cvs/svn formats)
          for checking out from tot/tob (default: none)
          For SVN current timezone is appended by default during checkout
          ('nightly' refers to midnight and 'now' or 'fix refer current time)
        * SVN revision can be r1234 or just 1234 format

-d dir
	Base directory in which to build (REQUIRED).
	[When using '-t testdir' option below, do not use official/public build
	directories to store your private built objects]

-f makeflags
        Use this option to specify any specific make/build macros on gmake
        command line. Even gnu make flags themselves can also be specified.
        Remember to enclose these flags in double quotes. May be used
        multiple times. Typical uses are: force dongle build or
        specifying a custom SVN URL to pick sources from, like /users.
        See examples below.

-g
        Use gnu-make to build [default]

-h
        Show help

-i
        Set number of parallel threads as needed by any make -j builds

-j
        Use electric make (emake) to build [optional]
        NOTE: This is still experimental

-m address
        Send mail when LSF job is complete (default: none).
        Use commas to separate multiple addresses.

-n
        Enable email relay of LSF job status to larger group/lists (default: ask)

-o R32BIT|RHEL40
        Force a brand to 32bit or 64bit lsf resource queue. This overrides
        default setting in build_config.sh. If no resource queue is
        specified and a brandname doesn't exist in build_config.sh, then
        R32BIT is used by default

-p private_release_makefile
        Provide path to custom or private release.mk file. This is
        copied as release.mk instead of cvs checkout of src/tools/<BRAND>.mk
        NOTE: Works only if "-b" specifies only one brand to build

-q queue
       LSF queue to use (default: ${LSF}). Specify "none" to build
       locally.

-r tag
       CVS tag to use (default: HEAD).

-s skip-brands
       Brands to skip from default list
        (default: brands list comes from build_config.sh)
       To skip more than one brand use "brand1 brand2 brand3 ... brandX"
       Note: If you rsh to another host to launch builds, use '"brand1 brand2"'

-t testdir
       'src' dir with files to override cvs pull.  Must be a CVS
       checkout.  (The "cvs diff" there is applied to the new checkout.)
       NOTE: (Optionally) Along with -t <testdir>, if -f COPYSRC=1 is
       supplied, <testdir> is just copied over as is for build

-u
       Submit job(s) as current user, rather than hwnbuild

-v
       Display more verbose information (default: none)

-w
       (default: on)
       Use temporary local drive path to build, instead of network path
       Once the build is completed in local drive path, the contents
       are transferred back to network path. User doesn't need to
       specify local path, it is derived automatically by build script

-x builds_to_keep
       Remove old builds (default: keep last ${BUILDS_TO_KEEP_TOB} TOB builds; keep last ${BUILDS_TO_KEEP_TOT} TOT builds)

Usage Examples:
---------------

  1. Build OR respin linux-external-wl for MY_BRANCH_3_120 tag and notify
     prakashd:
       build_linux.sh -r MY_BRANCH_3_120 -m prakashd -b linux-external-wl -d /projects/hnd/swbuild/build_linux

  2. Build OR respin linux-external-wl linux-internal-wl linux-external-router
     for MY_BRANCH_3_120 tag and notify prakashd:
       build_linux.sh -r MY_BRANCH_3_120  -m prakashd -b "linux-external-wl linux-internal-wl linux-external-router" -d /projects/hnd/swbuild/build_linux

  3. Build or respin linux-external-wl TOT nightly build brand:
       build_linux.sh -b linux-external-wl -d /projects/hnd/swbuild/build_linux

  4. Respin TOT nightly linux-external-wl with cutoff time:
       build_linux.sh -c "2007-11-10" -b linux-external-wl -d /projects/hnd/swbuild/build_linux

  5. Respin TOT nightly linux-external-wl with specific svn revision
       build_linux.sh -c "r23456" -b linux-external-wl -d /projects/hnd_software/work/xxx

  6. Build a full brand build from a private work-space (as-is with no
     merge/patching). This is an example. <desination-output-absolute-dir is
     an absolute dir that needs to be an absolute network path in your private
     workspace and must be a writeable directory.
       build_linux.sh -f COPYSRC=1 -b linux-external-dongle-sdio -r RAPTOR_REL_4_216_40 -d <destination-output-absolute-dir> -t <your-workspace-dir>/src

  7. Build DHD builds forcing dongle build (say linux-external-dongle-sdio)
     NOTE: For TOT and TOB builds, dongle image build is forced every *day*
     NOTE: whereas for tag builds, if a previous dongle image build exists for
     NOTE: the same tag, then it is reused. If you respin dhd build and want
     NOTE: dongle image build rebuilt use following example.
       build_linux.sh -b linux-external-dongle-sdio -f "FORCE_HNDRTE_BUILD=1" -r RAPTOR_REL_4_216_40 -d /projects/hnd/swbuild/build_linux

  8. Fetch sources from CVS instead of SVN (that's build with HNDCVS)
       build_linux.sh -b linux-external-wl -f "CVSROOT=/projects/cvsroot" -r KIRIN_BRANCH_5_100 -d /projects/hnd/swbuild/build_linux/

  9. Fetch sources from custom wlansvn/users/<user> area instead of $SVNROOT
     (SVNURL points to SVN Repository URL path)
       build_linux.sh -b linux-external-wl -f "REPO_URL=<SVNURL>/users/prakashd/KIRIN_BRANCH_5_100" -r KIRIN_BRANCH_5_100 -d /projects/hnd/swbuild/build_linux/USERS

  10. Build linux-external-wl on trunk using hnd_gclient
       build_linux.sh -b linux-external-wl -f "COTOOL=gclient" -d /projects/hnd/swbuild/build_linux/GCLIENT_TRANSITION

EOF
    exit 1
}

# Set SUBMIT to any necessary rsh required for bsub
function setsubmit () {
    local cluster=sj1;
    local user=${LSFUSER};
    local mycluster="";
    mycluster=$(/tools/bin/lsid 2> ${NULL} |
               egrep '^My cluster name'            |
               sed -e 's/My cluster name is //');

    SUBMIT=
    if [ "${mycluster}" != "${cluster}" ] || [ -z "$(hostname)" ] ||
       ([ "$(whoami)" != "${user}" ] && ! rsh -l ${user} $(hostname) true 2> ${NULL}); then
       SUBMIT="rsh -l ${user} $XLINUX"
    else
       if [ "$(whoami)" != "${user}" ]; then
          SUBMIT="rsh -l ${user} $(hostname)"
       fi
    fi

    if [ -n "${SUBMIT}" ] && ! ${SUBMIT} true; then
       echo "Error: can't submit as ${LSFUSER}?"
       exit
    fi
}

# Set up environment variables
function setenviron {
    if [ -z "${MAKEVER}" ]; then export MAKEVER=3.81; fi
    UNIX_PATH=/bin:/usr/bin:/usr/X11R6/bin:/sbin:/usr/sbin:/usr/X11R6/sbin:/usr/local/bin:/usr/local/sbin
    CROSSGCC30=/projects/hnd/tools/linux/hndtools-mipsel-linux/bin:/projects/hnd/tools/linux/hndtools-mipsel-uclibc/bin:/projects/hnd/tools/linux/hndtools-mips-wrs-vxworks/bin
    CROSSGCC323=/projects/hnd/tools/linux/hndtools-mipsel-linux-3.2.3/bin:/projects/hnd/tools/linux/hndtools-mipsel-uclibc-3.2.3/bin:/projects/hnd/tools/linux/hndtools-mips-wrs-vxworks-3.2.3/bin
    CROSSGCC323_2=/projects/hnd/tools/linux/hndtools-mipsel-linux-3.2.3-bcm-2/bin:/projects/hnd/tools/linux/hndtools-mipsel-uclibc-3.2.3-bcm-2/bin:/projects/hnd/tools/linux/hndtools-mips-wrs-vxworks-3.2.3/bin
    CROSSGCCLINKS=/projects/hnd/tools/linux/hndtools-mipsel-linux/bin:/projects/hnd/tools/linux/hndtools-mipsel-uclibc/bin:/projects/hnd/tools/linux/hndtools-mips-wrs-vxworks/bin

    CVSROOT_PROD=/projects/cvsroot

    case "`uname`" in
    SunOS)
         export PATH=/usr/ucb:/tools/bin:/projects/hnd/tools/SunOS/bin:${UNIX_PATH}
         if [ -z "$CVSROOT" ]; then
            export CVSROOT=$CVSROOT_PROD
         fi
         ;;
    Linux)
         # remove /tools/bin from PATH to fix for lzma issue on RHEL 4 build
         export PATH=/home/hwnbuild/bin:${UNIX_PATH}:/projects/hnd/tools/linux/bin:${CROSSGCCLINKS}
         if [ -z "$CVSROOT" ]; then
            export CVSROOT=$CVSROOT_PROD
         fi
         ;;
    *)
         # unknown
         if [ -z "$CVSROOT" ]; then
            export CVSROOT=":pserver:$(id -un)@cvsps-sj1-1.sj.broadcom.com:${CVSROOT_PROD}"
         fi
;;
    esac

    # Default SVN Repo root, if not set in environment
    if [ -z "$SVNROOT" ]; then
        # WLAN Production Repository
        export SVNROOT="http://svn.sj.broadcom.com/svn/wlansvn/proj"
        # NOTE: WLAN Production Repository - but accessed from alternate server
        # WARN: Talk to SVN Admins before using this server
        # WARN: Using this server needs credentials to be cached
        # export SVNROOT="http://engcm-sj1-06.sj.broadcom.com/svn/wlansvn/proj"
    fi
    export SVNBASE=$(echo $SVNROOT | sed -e 's%/proj%%')

    # Centralized SVN checkout rules and definitions
    CHECKOUT_DEFS="checkout-defs.mk"
    CHECKOUT_RULES="checkout-rules.mk"

    # Default repo url is trunk/tot
    export REPO_URL="${SVNROOT}/trunk"
    export REPO_URL_TRUNK="${SVNROOT}/trunk"

    # Derive REPO_URL for twigs/branches OR REL tags
    case "${TAG}" in
    *_BRANCH_*|*_TWIG_*)
             export REPO_URL="${SVNROOT}/branches/${TAG}"
             export TAGTYPE="TOB"
             ;;
    *_REL_*)
             export TAG_PREFIX=$(echo $TAG | awk -F_ '{print $1}')
             export REPO_URL="${SVNROOT}/tags/${TAG_PREFIX}/${TAG}"
             export TAGTYPE="REL"
             ;;
    esac

} # setenviron

# checkBrand() is used for following tasks right now
# 1. Find out if a given build brand can be built on with parallel emake
#    (this is implemented by 'isEmakeCompatible' target in release makefile)
# 2. Switching over 32bit build brands one-by-one to 64bit resources
#    (this is implemented by 'is32on64' target in release makefile)
function checkBrand {
    local BRAND=$1
    local CHECK_TARGET=$2
    local CHECK_DIR=$3
    local rc=1
    local PWDIR=""

    # A special temporary $BRANDCHECK_DIR for this temporary operation
    # where release.mk and checkout* or other mandatory included
    # files are checked out temporarily and purged

    BRANDCHECK_DIR="${CHECK_DIR}/${TAG:-NIGHTLY}/${BRAND}"
    [ -d "$BRANDCHECK_DIR" ] || mkdir -p "$BRANDCHECK_DIR"
    [ -d "$BRANDCHECK_DIR" ] && chmod ugo+w $BRANDCHECK_DIR ${BRANDCHECK_DIR}/.. 2> ${NULL}

    if [ ! -d "$BRANDCHECK_DIR" ]; then
       echo "WARN: $BRANDCHECK_DIR can't be created"
       echo "WARN: $BRAND will build on 32bit lsf queue"
    else

       PWDIR=$(pwd)
       cd $BRANDCHECK_DIR

       # Checkout checkout rules before release makefile is checked out
       # NOTE: If gallery is accessible CHECKOUT_RULES and CHECKOUT_DEFS
       # NOTE: can be made to come from there, instead svn repo
       if [ "$VCTOOL" == "svn" ]; then

          $SVNCMD export -q \
                  ${REPO_URL_TRUNK}/src/tools/release/${CHECKOUT_DEFS} \
                  ${CHECKOUT_DEFS} 2> $NULL
          $SVNCMD export -q \
                  ${REPO_URL_TRUNK}/src/tools/release/${CHECKOUT_RULES} \
                  ${CHECKOUT_RULES} 2> $NULL
          $SVNCMD export \
                  ${SVNCUTOFF:+-r \{"$SVNCUTOFF $SVNTZ"\}} \
                  ${SVNREV:+SVNREV=$SVNREV} \
                  -q ${REPO_URL}/src/tools/release/${BRAND}.mk release.mk 2> $NULL
          gmake -s -f release.mk \
                  ${VCTOOL:+VCTOOL=$VCTOOL} \
                  ${COTOOL:+COTOOL=$COTOOL} \
                  ${TAG:+TAG=$TAG} \
                  BRAND=${BRAND} \
                  ${SVNCUTOFF:+SVNCUTOFF="$SVNCUTOFF"} \
                  ${SVNREV:+SVNREV=$SVNREV} \
                  "${GMAKEFLAGS[@]}" \
                  $CHECK_TARGET \
                  > ${NULL} 2>&1

       else # VCTOOL

          $VCTOOL -Q -d $CVSROOT co -p src/tools/release/${CHECKOUT_DEFS} > ${CHECKOUT_DEFS}
          $VCTOOL -Q -d $CVSROOT co -p src/tools/release/${CHECKOUT_RULES} > ${CHECKOUT_RULES}
          $VCTOOL -Q co ${TAG:+-r $TAG} ${CVSCUTOFF:+-D "$CVSCUTOFF"} -p src/tools/release/${BRAND}.mk > release.mk 2> ${NULL}
          chmod +w *.mk > ${NULL} 2>&1
          gmake -s -f release.mk \
                  ${TAG:+TAG=$TAG} \
                  BRAND=${BRAND} \
                  ${CVSCUTOFF:+CVSCUTOFF="$CVSCUTOFF"} \
                  "${GMAKEFLAGS[@]}" \
                  $CHECK_TARGET \
                  > ${NULL} 2>&1

       fi

       rc=$?
       cd $PWDIR

       # Once the work is done, purge the $BRANDCHECK_DIR
       [ -d "$BRANDCHECK_DIR" ] && rm -rf "$BRANDCHECK_DIR"
    fi

    # Return success or failure code, this is used to route the build
    # appropriate 32/64bit lsf node or to EC
    return $rc
}

function usefreedisk
{
   # Don't redirect firmware builds that go to PREBUILD area
   if echo "${BUILD_BASE}" | egrep -qvi "${BUILD_BASE_LINUX_PREBUILD}"; then
      # Redirect builds that point to public build_linux as dest dir
      if [ "${BUILD_BASE}" == "${BUILD_BASE_LINUX}" -o "${BUILD_BASE}" == "${BUILD_BASE_LINUX_MOUNT}" ]; then

         # If it is not a respin of an existing build tag, then
         # Default target disk is BUILD_BASE_LINUX if none of BUILD_LINUX_*_DISKS are
         # lower in disk-space usage than BUILD_LINUX_MINDU
         BUILD_LINUX_MINDU=$1
         shift
         DISK_POOL="$@"
         if [ ! -e "${BUILD_BASE}/${TAG}" -a ! -n "$OVERRIDE" ]; then
            mindu=$BUILD_LINUX_MINDU
            mindisk=${BUILD_BASE_LINUX}
            for blddir in $DISK_POOL
            do
                # Find out disk-space usage on $blddir
                du=$(df -P -k -h  $blddir | awk '/:/{printf "%-s",$5}' | awk -F% '{printf "%s", $1}')
                [ -n "$VERBOSE" ] && echo "Disk Usage: ${du}% for $blddir"
                if echo $du | egrep -qv "^[[:digit:]]+$"; then
                   continue
                fi
                if [ "$du" -lt "$mindu" ]; then
                   mindu=$du
                   mindisk=$blddir
                fi
            done
            # Now mindisk points to disk with lowest usage
            # Create build dir there and symlink it from primary disk
            if [ "$mindisk" != "${BUILD_BASE_LINUX}" -a ! -e "${BUILD_BASE}/${TAG}" ]; then
               RSHCMD="rsh -l ${LSFUSER} $XLINUX"
               [ -n "$VERBOSE" ] && echo "Picked Disk: ${mindu}% $mindisk"
               ${RSHCMD} mkdir -pv ${mindisk}/${TAG} > ${NULL}
               ${RSHCMD} chmod 775 ${mindisk}/${TAG} > ${NULL}
               if [ -e "${BUILD_BASE}/${TAG}" ]; then
                  echo "INFO: ${BUILD_BASE}/${TAG} exists"
               else
                  # Avoid race conditions in mkdir when more than one brand is launched
                  # Sleep random number of seconds (0-5 sec)
                  rand=$(echo | awk '{srand(); print rand()*5}')
                  sleep $rand
                  if [ -e "${BUILD_BASE}/${TAG}" ]; then
                     echo "INFO: ${BUILD_BASE}/${TAG} now exists"
                  else
                     [ -n "$VERBOSE" ] && \
                     echo "[`date '+%Y/%m/%d %H:%M:%S'` `hostname`] Creating ${TAG} symlink"
                     ${RSHCMD} ln -sv ${mindisk}/${TAG} ${BUILD_BASE}/${TAG} > ${NULL}
                  fi
               fi
               [ -n "$VERBOSE" ] && ls -l ${BUILD_BASE}/${TAG} 2> ${NULL}
            fi
         fi # link
      fi # build_base
   fi # prebuild
} # usefreedisk

# Validate the tag for existence
function validate_tag {
           foundtagstatus=0

           foundtaginfo=$($SVNCMD ls $REPO_URL 2>&1)
           foundtagstatus=$?

           if [ "$foundtagstatus" != "0" ]; then
              echo "ERROR: $foundtaginfo"
              exit 1
           fi
} # validate_tag

# NOTE: If any new flag is added here and if it is used by nightly builds, then
# NOTE: build_dslcpe.sh needs to be updated as well, as the same flags pass
# NOTE: from this script to build_dslcpe.sh script (when dslcpe build brands
# NOTE: are built)

# Command line switches this script takes
# WARN: Ensure that cmd line option blocks below are arranged alphabetically.
# Available k,w,y,z

while getopts 'ab:c:d:e:f:ghi:jlm:no:p:q:r:s:t:uvwx:' OPT ; do
    case "$OPT" in
	a)
	    # Force all brand build. Don't use for tob/tag builds
	    ALL_BRANDS=1
	    ;;
	b)
	    # Give specific brand to build
	    BRANDS=(${OPTARG})
	    ;;
	c)
	    # Cutoff time or svn revision to use during checkout
	    # CVSCUTOFF is exported into the caller's environment
	    export CVSCUTOFF=${OPTARG}
	    # SVNCUTOFF is exported into the caller's environment
	    export SVNCUTOFF="${OPTARG}"
	    ;;
	d)
	    # Destinatiomn dir to place the build
	    BUILD_BASE=${OPTARG}
	    ;;
	e)
	    # Destination dir to place the build
	    EXTRA_LOG_STATUS=${OPTARG}
	    ;;
	f)
	    # Make flags to passdown to release.mk
	    # Like CVSROOT or SVN REPO to use or Developer build
	    # or custom firmware image to override in dhd/bmac
        GMAKEFLAGS[${#GMAKEFLAGS[@]}]="${OPTARG}"
	    ;;
	g)
	    # Force GNU make build for all
	    GMAKE=1
	    ;;
	h)
	    usage
	    ;;
	i)
	    # Set number of LSF slots, default is 1
	    LSFSLOTS=${OPTARG}
	    ;;
	j)
	    # Force EC emake build
	    EMAKE=1
	    ;;
	l)
	    # Log the console output from LSF bsub
	    LSFLOG=1
	    ;;
	m)
	    # Notify these
	    MAILTO=${OPTARG}
	    export MAILTO
	    ;;
	n)
	    # User intentionally notify BUILD_LIST LSF job status!
	    NOTIFY_ALL=1
	    ;;
	o)
	    # Internal flag used to specify 32bit or 64bit resource
	    OSFLAG=${OPTARG}
	    ;;
	p)
	    # Override cvs or SVN release.mk with a private release.mk
	    PVT_MAKEFILE=$OPTARG
	    ;;
	q)
	    # LSF queue to build to, default is set above
	    # If set to 'none', but build locally on current node
	    LSF=${OPTARG}
	    ;;
	r)
	    # SVN release tag or branch or twig to use
	    TAG=${OPTARG}
	    ;;
	s)
	    # Skip these build brands from $BRANDS list
	    SKIPBRANDS=(${OPTARG})
	    ;;
	t)
	    # Override CVS or SVN workspace sources to build from
	    # with developer's private workspace
	    OVERRIDE=${OPTARG}
	    LSFUSER=$(whoami)
	    ;;
	u)
	    # Submit to lsf as this user
	    LSFUSER=$(whoami)
	    ;;
	v)
	    VERBOSE=1
	    ;;
	w)
	    # If this flag is turned on, the build will happn on local
	    # drive on compute node and results are copied back to network
	    # This flag is turned ON for now globally
	    LOCAL_BUILD=1
	    ;;
	x)
	    # Build iterations older than these days are purged
	    BUILDS_TO_KEEP=${OPTARG}
	    ;;
	?)
	    usage
	    ;;
    esac
done

# Enable verbosity for firmware builds
if echo "$BUILD_BASE" | egrep -i -q prebuild; then VERBOSE=1; fi

# Set up environment variables in case not running as hwnbuild.
# This sets CVSROOT so it must be done before running any CVS commands.
# This also sets SVNROOT so it must be done before running any SVN commands.
setenviron

for arg in "${GMAKEFLAGS[@]}" ; do
  case $arg in
     VCTOOL=cvs)
           # If custom CVSROOT is supplied, use it override default setting
           # If CVSROOT is set to dynamically changing CVS_SNAPSHOT, let it
           # be inherited from environment
           export VCTOOL=cvs
           FIND_BUILD_ERRORS="$FIND_BUILD_ERRORS -cvs"

           SESSIONLOG=${SESSIONLOGDIR}/cvs_snapshot_build_${TODAY}.log
           export CVSROOT=$(echo "$arg" | fmt -1 | awk -F= '/CVSROOT/{print $2}')
           # Now set OVERRIDE to null to ensure that release makefiles
           # don't try to merge current workspace to $TAG or TOT (if no tag)
           export OVERRIDE=
           ;;

     VCTOOL=svn)
           # If GMAKEFLAGS contains VCTOOL=svn, export it to env
           # to pass down to $(MAKE)
           export VCTOOL=svn
           SESSIONLOG=${SESSIONLOGDIR}/svn_build_${TODAY}.log

           validate_tag
           ;;

     COTOOL=gclient)
           # If GMAKEFLAGS contains COTOOL=gclient, export it to env
           # to pass down to $(MAKE)
           export COTOOL=gclient
           SESSIONLOG=${SESSIONLOGDIR}/gclient_build_${TODAY}.log

           # If build submitted for GCLIENT, forcefully redirect it to
           # GCLIENT_TRANSITION folder
           if [ -n "$BUILD_BASE" ]; then
              if echo $BUILD_BASE | egrep -qv "GCLIENT_TRANSITION"; then
                 BUILD_BASE="${BUILD_BASE}/GCLIENT_TRANSITION"
                 echo "INFO: GCLIENT build redirected to $BUILD_BASE"
              fi
           fi
           # BUILD_LINUX_DIR is exported by DHD build into Firmware build
           # process. Separate out GCLIENT fw builds from production SVN fw
           # builds
           export BUILD_LINUX_DIR=$BUILD_BASE_LINUX_GCLIENT

           validate_tag
           ;;

     REPO_URL=*)
           # Extract repository URL if specified on command line flag
           export REPO_URL=$(echo "$arg" | fmt -1 | awk -F= '/REPO_URL/{print $2}')
           echo "INFO: Setting custom SVN Repository URL to $REPO_URL"
           export LSFUSER=$(whoami)

           validate_tag
           ;;

     DEPS_URL=*)
           # Extract repository URL if specified on command line flag
           export DEPS_URL=$(echo "$arg" | fmt -1 | awk -F= '/DEPS_URL/{print $2}')
           echo "INFO: Setting custom GCLIENT Deps URL to $DEPS_URL"
           export LSFUSER=$(whoami)
           ;;
     http*)
           # Rare cases where users make mistakes like they miss REPO_URL
           # or DEPO_URL prefix
           echo "ERROR: "
           echo "ERROR: Erroneous -f '$arg' gmakeflags specified"
           echo "ERROR: Did you miss REPO_URL or DEPO_URL variable name"
           echo "ERROR: Can't continue, Exiting"
           echo "ERROR: "
           exit 1;
           ;;
  esac # arg
done # GMAKEFLAGS

if uname | grep -q -i -v "linux\|netbsd"; then
   echo "ERROR: Can't submit linux build jobs from $(uname) system $(hostname)"
   exit 1
fi

# Redirect user's private builds (from /proj/users area) away from prod dirs
if [ "${BUILD_BASE}" == "${BUILD_BASE_LINUX}" -o "${BUILD_BASE}" == "${BUILD_BASE_LINUX_MOUNT}" ]; then
   # User's full builds are redirected to a separate scratch disk
   # they are identified by users svn area name
   if echo "${REPO_URL}" | grep -q "wlansvn/users"; then
      umask 002
      export BUILD_USERDIR=$(echo $REPO_URL | perl -ne "s%(http|file):.*?/users/%%gi; s%/%_%g; print")
      export BUILD_BASE=$BUILD_BASE_LINUX_USERS/$BUILD_USERDIR

      if ! mkdir -p "${BUILD_BASE}" ; then
         echo "ERROR: $(hostname): Creation of ${BUILD_BASE} failed"
         exit 1
      fi
      echo "INFO: Overriding BUILD_BASE to $BUILD_BASE"
      # When OVERRIDE is defined, create private workspace for firmware images
      export BUILD_LINUX_TEMP=${BUILD_BASE}
      export RSHCMD="rsh -l $LSFUSER "
   fi # REPO_URL=wlansvn/users
fi # BUILD_BASE

if [ "${BUILD_BASE}" = "" ]; then
   echo "ERROR: $(hostname): Build dir not specified via -d (use -h for help)"
   exit 1
fi

# Ensure that specified destination dirs pre-exist
if [ -d "${BUILD_BASE}" -o -h "${BUILD_BASE}" ]; then
   BUILD_BASE_MOUNT=$BUILD_BASE
   BUILD_BASE=`cd $BUILD_BASE;pwd -P 2> ${NULL}`
else
   echo "ERROR: $(hostname): Build dir ${BUILD_BASE} not specified or doesn't exist! Exiting"
   exit 1
fi

# For TOB builds set their KEEP age to BUILDS_TO_KEEP_TOB
if echo "${TAG}" | grep -q -i "_BRANCH_\|_TWIG_"; then
   if [ ! -n "${BUILDS_TO_KEEP}" ]; then
      BUILDS_TO_KEEP=${BUILDS_TO_KEEP_TOB}
      # echo "INFO: Set default build keep count to ${BUILDS_TO_KEEP_TOB} builds"
   fi
fi

# For TOT builds set their KEEP age to BUILDS_TO_KEEP_TOT
if [ "${TAG}" == "" -o "${TAG}" == "HEAD" ]; then
   if [ ! -n "${BUILDS_TO_KEEP}" ]; then
      BUILDS_TO_KEEP=${BUILDS_TO_KEEP_TOT}
      echo "INFO: Set default build keep count to ${BUILDS_TO_KEEP_TOT} builds"
   fi
fi

if [ "${CVSCUTOFF}" != "" -a "${TAG}" != "" ]; then
   if echo "${TAG}" | grep -q -v "_BRANCH_\|_TWIG_\|HEAD"; then
      echo "ERROR: $(hostname): -cvscutoff and cvstag are mutually exclusive"
      usage
   fi
fi

# If cutoff time is specified as "nightly" take it as midnight
if [ "${CVSCUTOFF}" == "nightly" ]; then
   CVSCUTOFF="`date '+%Y-%m-%d 00:00'`"
   SVNCUTOFF="`date '+%Y-%m-%d 00:00'`"
fi

# SVN always needs date yyyy-mm-dd with HH:MM:SS
if echo "${SVNCUTOFF}" | egrep -q '^[[:digit:]][[:digit:]]:[[:digit:]][[:digit:]]$'; then
   SVNCUTOFF="`date "+%Y-%m-%d $SVNCUTOFF"`"
fi

# If cutoff time is specified as "now" or "fix" take it as current timestamp
if [ "${CVSCUTOFF}" == "now" -o "${CVSCUTOFF}" == "fix" ]; then
   CVSCUTOFF="`date '+%Y-%m-%d %H:%M:%S'`"
   SVNCUTOFF="`date '+%Y-%m-%d %H:%M:%S'`"
fi

# If -c specified a SVN revision fetch it and reset cutoff to null
if echo "${SVNCUTOFF}" | egrep -q "^(r|)[[:digit:]]+$"; then
   SVNREV=$(echo $SVNCUTOFF | sed -e 's/^r//g')
   unset SVNCUTOFF
   unset CVSCUTOFF
fi

# Don't allow developer's private builds in release/production dirs
if [ -n "$OVERRIDE" ]; then
    case "$BUILD_BASE" in
        ${BUILD_BASE_LINUX}|${BUILD_BASE_LINUX_MOUNT})
	  BUILD_BASE="${BUILD_BASE}/CUSTOM"
	  echo "WARNING: /projects/hnd/swbuild/* is for production builds only"
	  echo "WARNING: Redirecting private builds to $BUILD_BASE"
          echo ""
	  if [ ! -d "$BUILD_BASE" ]; then
	     echo "ERROR: Private BUILD_BASE = ${BUILD_BASE} doesn't exist"
	     exit 1
	  else
	     echo "INFO: Force setting BUILD_BASE = ${BUILD_BASE}"
	  fi
    ;;
    esac
    umask 002
    # When OVERRIDE is defined, create private workspace for firmware images
    export BUILD_LINUX_TEMP=${BUILD_BASE}
    export RSHCMD="rsh -l $LSFUSER "
fi

## BRAND list extraction via build_config.sh
BRANDS_REQUESTED=(${BRANDS[*]})

if [ "${ALL_BRANDS}" == "1" -a "${BRANDS[*]}" != "" ]; then
   echo -e "WARN:"
   echo -e "WARN: -a and -b command switches are mutually exclusive"
   echo -e "WARN: Overriding with BRANDS=${BRANDS[*]}"
   echo -e "WARN:"
fi

if [ "${ALL_BRANDS}" == "" -a "${BRANDS[*]}" == "" ]; then
   echo "WARN: No build brands specified with -b \"<brand1> <brand2>\""
   echo "INFO: Default list of brands will be picked"
fi

cfgec=0
if [ "${TAG}" == "" -a "${BRANDS[*]}" == "" ]; then
   if echo " ${GMAKEFLAGS[*]} " | grep -qi " DAILY_BUILD=1 " ; then
     BRANDS=($(${BUILD_CONFIG} -r DAILY_TOT -p Linux 2> ${NULL})); cfgec=$?
     DAILY_BUILD=1
     [ -n "$VERBOSE" ] && \
	echo "INFO: Daily TOT BRANDS[$((${#BRANDS[*]}-1))]=${BRANDS[*]}"
   else
      BRANDS=($(${BUILD_CONFIG} -r TRUNK -p Linux 2> ${NULL})); cfgec=$?
   fi
elif [ "${TAG}" != "" -a "${BRANDS[*]}" == "" ]; then
   [ -n "$VERBOSE" ] && \
	echo "INFO: TAG VCTOOL=$VCTOOL; CVSROOT=$CVSROOT" && \
	echo "INFO: ${BUILD_CONFIG} -r ${TAG} -p Linux"
   BRANDS=($(${BUILD_CONFIG} -r ${TAG} -p Linux 2> ${NULL})); cfgec=$?
elif [ "${BRANDS_REQUESTED[*]}" == "" ]; then
   [ -n "$VERBOSE" ] && \
	echo "INFO: TOT VCTOOL=$VCTOOL; CVSROOT=$CVSROOT" && \
	echo "INFO: ${BUILD_CONFIG} -r TRUNK -p Linux"
   BRANDS=($(${BUILD_CONFIG} -r TRUNK -p Linux 2> ${NULL})); cfgec=$?
fi

#echo "Derived   BRANDS = ${BRANDS[*]}"
#echo "Requested BRANDS_REQUESTED = ${BRANDS_REQUESTED[*]}"

if [ "${BRANDS_REQUESTED[*]}" == "" -a "${BRANDS[*]}" == "" ] || [ "${BRANDS_REQUESTED[*]}" == "" -a "$cfgec" != "0" ]; then
	echo "ERROR:"
        echo "ERROR: Default list of build brands can't be generated"
        echo "ERROR: or no brands were specified on command line with -b <brand>"
	echo "ERROR: Can't continue. Exiting"
	echo "ERROR:"
	exit $cfgec
fi

# Reset pseudo tags to their original values
if [ "$TAG" == "TOTUTF" ]; then
   unset TAG
fi

[ "${BRANDS_REQUESTED}" == "" ] && echo "INFO: DERIVED DEFAULT BUILD BRANDS[${#BRANDS[*]}] = ${BRANDS[*]}"

# For TOT, default build list is too big, so confirm with user before starting
if [ "${TAG}" == "" -a "${BRANDS_REQUESTED}" == "" -a "${ALL_BRANDS}" == "" -a "${DAILY_BUILD}" == "" ]; then
   echo ""
   echo -n "* WARN: Do you really want to build *ALL* brands? [Yes|No|Quit]: "
   read ans
   if echo ${ans} | grep -q -i "q\|n"; then echo -e "\nExiting"; exit 0; fi
fi

# Redirect public REL builds to disk with lowest usage percentage
# To ensure that their chance of failing due to disk-space is reduced
# Any private builds aren't supposed to be in public disks, so this
# automated disk-allocation will not work
if [ "${TAGTYPE}" == "REL" ]; then
      usefreedisk 95 ${BUILD_LINUX_REL_DISKS}
elif [ "${TAGTYPE}" == "TOB" ]; then
      usefreedisk 90 ${BUILD_LINUX_TOB_DISKS}
fi

## Inherit list of 32bit LSF brands from build_config.sh
R32BITBRANDS=($(${BUILD_CONFIG} --show_32bit_brands 2> ${NULL}))
## Inherit list of 64bit LSF brands from build_config.sh
R64BITBRANDS=($(${BUILD_CONFIG} --show_64bit_brands 2> ${NULL}))

# If BRANDS is not supplied as an arg, concatenate 32bit and 64bit brands
[ "${BRANDS}" == "" ] && BRANDS=(${R32BITBRANDS[*]} ${R64BITBRANDS[*]});

# Search for brands that need to be skipped, as they can't be built
# by current script. Filter them out of BRANDS list and append to
# SKIPBRANDS. If it is a dslcpe brand, append to DSLCPEBRANDS in
# addition to skipping it

for i in $(seq 0 $((${#BRANDS[*]}-1))); do
    brand=${BRANDS[${i}]}
    if echo "$brand" | egrep -qi "linux.*dslcpe"; then
       SKIPBRANDS=(${SKIPBRANDS[*]} $brand)
       DSLCPEBRANDS=(${DSLCPEBRANDS[*]} $brand)
       continue
    fi
done

# Convert brand array to scalar to remove duplicate brand info in next pass
for i in $(seq 0 $((${#SKIPBRANDS[*]}-1))); do
    skipbrand=${SKIPBRANDS[${i}]}
    skipbrandsfound=" $skipbrandsfound $skipbrand "
done

# Remove brands to be skipped from brands list
TMPBRANDS=()
for i in $(seq 0 $((${#BRANDS[*]}-1))); do
    brand=${BRANDS[${i}]}
    if echo "$skipbrandsfound" | egrep -q " $brand "; then
       continue
    else
       TMPBRANDS=(${TMPBRANDS[*]} $brand)
    fi
done
# TMPBRANDS Now contains brands actually needing to be built
[ "${TMPBRANDS[*]}" != "" ] && BRANDS=(${TMPBRANDS[*]})

# Show final list of brands
[ "${SKIPBRANDS[*]}" != "" ] && \
     echo "INFO: SKIP  BRANDS SPECIFIED [${#SKIPBRANDS[*]}] = ${SKIPBRANDS[*]}" &&
     echo "INFO: FINAL BRANDS TO BUILD [${#BRANDS[*]}] = ${BRANDS[*]}"

###### Debug #########
# echo "DBG: Exiting 1"
# exit 1

# -p <private-make> can't go with multiple build brands, as each
# brand needs its own brand release makefile
if [ -n "$PVT_MAKEFILE" ] && [ $((${#BRANDS[*]}-1)) -gt 1 ]; then
   echo "ERROR: '-p <private-makefile>' can't be used with '${BRANDS[*]}'"
   echo "ERROR: Specify only one build brand with '-b <build-brand>'"
   exit 1
fi

# Bark on non-existant private release makefile, if specified
if [ -n "$PVT_MAKEFILE" ] && [ ! -s ${PVT_MAKEFILE} ]; then
   echo "ERROR: $(hostname): Private makefile '${PVT_MAKEFILE}' not found or empty"
   exit 1
fi

# Identify brands which can build on 64bit rhel lsf
# 32bit takes precedence
for i in $(seq 0 $((${#BRANDS[*]}-1))) ; do
    foundbrand=0
    BRAND=${BRANDS[${i}]}

    ## Catch any bad/erroneous brand name names specified and skip them.
    if echo $BRAND | egrep -q "'|_REL_|_BRANCH_|_TWIG_"; then
       echo "WARN: Invalid brandname \"$BRAND\" specified. Skipping it"
       echo "WARN: See usage by running \"$0 -h\""
       continue
    fi

    ## If a user forces via cmd line switch a brand to 32/64bit queue
    if   [ "$OSFLAG" == "R32BIT" ]; then
         R32BUILDS=(${R32BUILDS[*]} $BRAND)
         continue
    elif [ "$OSFLAG" == "RHEL40" ]; then
         R64BUILDS=(${R64BUILDS[*]} $BRAND)
         continue
    elif [ "$OSFLAG" == "UBUNTU" ]; then
         UBUNTUBUILDS=(${UBUNTUBUILDS[*]} $BRAND)
         continue
    fi
    # Assign given brands to 32bit or 64bit queue as specified
    # in build_config.sh.
    # Override build_config setting for a given 32bit brand,
    # if it has been updated to build on 64bit resource
    for j in $(seq 0 $((${#R32BITBRANDS[*]}-1))) ; do
        if [ "$BRAND" ==  "${R32BITBRANDS[${j}]}" ]; then
           if checkBrand $BRAND is32on64 $RESCHECK_DIR; then
              [ -n "$VERBOSE" ] && \
		 echo "INFO: Forcing 32bit $BRAND brand to 64bit host"
              R64BUILDS=(${R64BUILDS[*]} $BRAND)
           else
              R32BUILDS=(${R32BUILDS[*]} $BRAND)
           fi
           foundbrand=1; break;
        fi # BRAND
    done

    [ "$foundbrand" == "1" ] && continue;

    for j in $(seq 0 $((${#R64BITBRANDS[*]}-1))) ; do
        if [ "$BRAND" ==  "${R64BITBRANDS[${j}]}" ]; then
           R64BUILDS=(${R64BUILDS[*]} $BRAND)
           foundbrand=1; break;
        fi
    done

    # If 32bit or 64bit segregation can't be determined still
    #     * Set 32bit queue for wl nic builds by default
    #     * Set 64bit queue for router and vxworks builds
    #     * and 32bit for others by default
    if echo $BRAND | grep -qi "linux.*-wl" | grep -v "64"; then
       [ "$foundbrand" == "0" ] && R32BUILDS=(${R32BUILDS[*]} $BRAND)
       continue
    fi
    if echo $BRAND | grep -qi "linux-coverity"; then
       [ "$foundbrand" == "0" ] && R64BUILDS=(${R64BUILDS[*]} $BRAND)
       continue
    fi
    if echo $BRAND | grep -qi "linux-coverity-65"; then
       [ "$foundbrand" == "0" ] && R64BUILDS=(${R64BUILDS[*]} $BRAND)
       continue
    fi
    if echo $BRAND | grep -qi "router\|usbap\|ecos-\|64\|vx\|hndrte-dongle-wl"; then
       [ "$foundbrand" == "0" ] && R64BUILDS=(${R64BUILDS[*]} $BRAND)
       continue
    else
       [ "$foundbrand" == "0" ] && R32BUILDS=(${R32BUILDS[*]} $BRAND)
       continue
    fi

done # for each brand

# If R32BUILDS or R64BUILDS is empty, inherit from build_config
[ "${R32BUILDS}" == "" ] && [ "${BRANDS}" == "" ] && R32BUILDS=(${R32BITBRANDS[*]});
[ "${R64BUILDS}" == "" ] && [ "${BRANDS}" == "" ] && R64BUILDS=(${R64BITBRANDS[*]});

#[ -n "$VERBOSE" ] && [ "${BRANDS}" != "" ] && \
#     echo "ALLBRANDS[${#BRANDS[*]}]=${BRANDS[*]}"
[ -n "$VERBOSE" ] && [ "${R32BUILDS}" != "" ] && \
     echo "R32 BUILDS  [${#R32BUILDS[*]}] = ${R32BUILDS[*]}"
[ -n "$VERBOSE" ] && [ "${R64BUILDS}" != "" ] && \
     echo "R64 BUILDS  [${#R64BUILDS[*]}] = ${R64BUILDS[*]}"

[ -n "$VERBOSE" ] && [ "${BUILD_BASE}" != "" ] && \
     echo "Effective BUILD_BASE = $BUILD_BASE"

if [ "${OPTIND}" -le "$#" ]; then
    echo "ERROR: $(hostname): Some command line options are missing to $0"
    usage
fi

if [ "${BUILD_BASE}" == "" ]; then
    echo "ERROR: BUILD_BASE = ${BUILD_BASE} not defined or not found"
    usage
fi

# Log LSF console output in LSFLOGDIR
if [ -n "$LSFLOG" ]; then
   LSFTIMESTAMP=`date '+%Y%m%d_%H%M%S'`
   LSFDATE=`date '+%Y%m%d'`
   LSFLOGDIR=/projects/hnd/swbuild/build_admin/logs/lsftasks/${TAG:-NIGHTLY}/$LSFDATE
   mkdir -p $LSFLOGDIR 2>$NULL
   if [ -d "$LSFLOGDIR" ]; then
      LSFLOGFILE=${LSFLOGDIR}/lsf_${LSFTIMESTAMP}.log
   else
      LSFLOGFILE=$NULL
   fi
   echo "DEBUG: lsf logout: $LSFLOGFILE"
else
   LSFLOGFILE=$NULL
fi

# Default email gets ignored
if [ "$MAILTO" != "" ]; then
	LSFOUTPUT="-u \"${MAILTO}\""
else
	LSFOUTPUT="-o ${LSFLOGFILE}"
fi

# LSF emails shouldn't go to bigger build lists like hnd-build-list
# Confirm with user the intentions
if echo "${MAILTO}" | grep -q -i "${BUILD_LIST}"; then
	if [ "${NOTIFY_ALL}" == "" ]; then
		echo ""
		echo -n "* WARN: Do you really want to email ALL LSF outputs to ${MAILTO} (default: N)? [Yes|No|Quit]: "
		read ans
		case "${ans}" in
			y*|Y*)
				echo -e "\nWARN: Forcing job status notification to '$MAILTO'\n"
				;;
			q*|Q*)
				echo -e "\nWARN: Exiting. Can't notify job status to a wider list\n"
				exit 0
				;;
			n*|N*)
				LSFOUTPUT="-o ${NULL}"
				echo -e "\nWARN: Job status notification disabled\n"
				;;
			*)
				MAILTO=$(echo "$MAILTO" | sed -e "s/$BUILD_LIST//g" -e "s/^[[:space:]]*$//g")
				if [ "$MAILTO" == "" ]; then
					echo -e "\nWARN: Job status notification disabled\n"
					LSFOUTPUT="-o ${NULL}"
				else
					echo -e "\nWARN: Skipping broader $BUILD_LIST and notifying only to '$MAILTO'\n"
					LSFOUTPUT="-u \"${MAILTO}\""
				fi
				;;
		esac
	fi
fi

if [ "${LSFUSER}" == "" ]; then
    echo "ERROR: $(hostname): Cannot determine current user?"
    exit 1
fi

###### Debug #########
#echo "DBG: Exiting 2"
#exit 2

# Submit myself to LSF
if [ "${LSF}" != "none" ] ; then
    # Quote all arguments
    while [ "$1" ] ; do
        if [ "$1" == "-b" ]; then
           shift; shift;
        else
           if [ "${1%%\"}" == "$1" ] ; then
                args="${args} \"${1}\""
           else
                args="${args} \'${1}\'"
           fi
           shift
       fi
    done

    # Submit myself to the batch daemon.
    # The -J option creates as many jobs as there are brands.
    setsubmit

    # Relative path contexts are not passed from user's script launch
    # environment to build user's logon session
    if echo $0 | grep -q "^/"; then
       build_script=$0
    else
       build_script=`pwd`/$0
    fi
    # Use build_dslcpe.sh for running dslcpe build brands
    build_script_dslcpe=${build_script//build_linux.sh/build_dslcpe.sh}

    # If the requestor is not BLDUSER, record that info in ,release.log
    # through "-e" cmd line switch to submitted job
    if [ "$(whoami)" != "${BLDUSER}" ]; then
	args="${args} \"-e\" \"BUILD_REQUESTER=$(whoami)\""
    fi

    r32args="${args} \"-o\" \"R32BIT\" \"-b\" \"${R32BUILDS[*]}\""
    r64args="${args} \"-o\" \"RHEL40\" \"-b\" \"${R64BUILDS[*]}\""
    ubuntuargs="${args} \"-o\" \"UBUNTU\" \"-b\" \"${UBUNTUBUILDS[*]}\""
    r64args_dslcpe="${args} \"-b\" \"${DSLCPEBRANDS[*]}\""

       r32res="r32bit"
       r64res="rhel40"
       ubuntures="ubuntu"
       r64res_dslcpe="rhel40"

    r32cmd="/tools/bin/bsub -R \"$r32res\" ${LSFSLOTS:+ -n $LSFSLOTS} -L /bin/bash ${LSFOUTPUT} -q ${LSF32BIT} -J\"${LSFUSER}[1-${#R32BUILDS[*]}]\" \"$build_script\" ${r32args} -q none"
    r64cmd="/tools/bin/bsub -R \"$r64res\" ${LSFSLOTS:+ -n $LSFSLOTS} -L /bin/bash ${LSFOUTPUT} -q ${LSF} -J\"${LSFUSER}[1-${#R64BUILDS[*]}]\" \"$build_script\" ${r64args} -q none"
    ubuntucmd="/tools/bin/bsub -R \"$ubuntures\" ${LSFSLOTS:+ -n $LSFSLOTS} -L /bin/bash ${LSFOUTPUT} -q ${LSF} \"$build_script\" ${ubuntuargs} -q none"
    r64cmd_dslcpe="/tools/bin/bsub -R \"$r64res\" -L /bin/bash ${LSFOUTPUT} -q ${LSF} -J\"${LSFUSER}[1-${#DSLCPEBRANDS[*]}]\" \"$build_script_dslcpe\" ${r64args_dslcpe} -q none"

    [ -n "$VERBOSE" ] && echo "INFO: Submitting LSF jobs from $(hostname) to $LSF queue"
    [ -n "$VERBOSE" ] && [ -n "$LSFSLOTS" ] && echo "INFO: $LSFSLOTS LSF slots specified"

    if [ "$VCTOOL" == "cvs" ]; then
       echo "$(hostname): Setting CVSROOT=$CVSROOT"
    fi
    [ -n "$VERBOSE" ] && [ "${DSLCPEBRANDS[*]}" != "" ] && \
         echo "Routing ${DSLCPEBRANDS[*]} to $build_script_dslcpe script: $build_script_dslcpe $r64args_dslcpe"

    # Exit codes from submit step are rarely used as build already through
    # by the time exit code is presented back to shell, however if there is any
    # exit code, it is preserved until after all submit attempts are completed

    submitec=0; submitec32=0; submitec64=0; submitecdsl=0; submitecubuntu=0;

    if [ -n "${SUBMIT}" ]; then

       # Submit 32bit builds
       if [ ${#R32BUILDS[*]} -gt 0 ]; then
          ${SUBMIT} ${r32cmd}; submitec32=$?
       fi

       # Submit 64bit builds
       if [ ${#R64BUILDS[*]} -gt 0 ]; then
          ${SUBMIT} ${r64cmd}; submitec64=$?
       fi

       if [ ${#UBUNTUBUILDS[*]} -gt 0 ]; then
          ${SUBMIT} ${ubuntucmd}; submitecubuntu=$?
       fi
       # Submit any dslcpe brands with build_script_dslcpe script instead
       if [ "${DSLCPEBRANDS[*]}" != "" ]; then
          ${SUBMIT} ${r64cmd_dslcpe}; submitecdsl=$?
       fi

    else # SUBMIT

       # Submit 32bit builds
       if [ ${#R32BUILDS[*]} -gt 0 ]; then
          eval $(echo ${r32cmd}); submitec32=$?
       fi

       # Submit 64bit builds
       if [ ${#R64BUILDS[*]} -gt 0 ]; then
          eval $(echo ${r64cmd}); submitec64=$?
       fi

       if [ "$OSFLAG" == "UBUNTU" ]; then
          eval $(echo ${ubuntucmd}); submitecubuntu=$?
       fi

       # Submit any dslcpe brands with build_script_dslcpe script instead
       if [ "${DSLCPEBRANDS[*]}" != "" ]; then
          eval $(echo ${r64cmd_dslcpe}); submitecdsl=$?
       fi

    fi # SUBMIT

    # Display exit code from individual submissions to user before exiting
    if [ "$submitec32" != "0" ]; then
       echo "ERROR: 32bit build submit exit code: $submitec32"
       submitec=1
    fi
    if [ "$submitec64" != "0" ]; then
       echo "ERROR: 64bit build submit exit code: $submitec64"
       submitec=1
    fi
     if [ "$submitecubuntu" != "0" ]; then
       echo "ERROR: Ubuntu build submit exit code: $submitecubuntu"
       submitec=1
    fi
   if [ "$submitecdsl" != "0" ]; then
       echo "ERROR: dsl build submit exit code: $submitecdsl"
       submitec=1
    fi

    # Finally exit with exit code of 0 or 1 based on individual category
    # results. Build is already completed by this time, so this is just
    # for reference or any higher level wrapper around this build_linux.sh
    if [ "$submitec" != "0" ]; then
       exit 1;
    else
       exit 0
    fi
fi # LSF

# if srcdir specified, get absolute path and generate custom suffix
if [ -n "$OVERRIDE" ]; then
    if ! (cd $OVERRIDE); then
	echo "ERROR: $(hostname): Cannot get to src dir $OVERRIDE"
	exit 1
    fi
    OVERRIDE=$(cd $OVERRIDE && pwd)

    cvs status -l $OVERRIDE > ${NULL} 2>&1; cvs_ovstatus=$?
    svn info $OVERRIDE > ${NULL} 2>&1; svn_ovstatus=$?

    if [ "$cvs_ovstatus" != "0" -a "$svn_ovstatus" != "0" ]; then
	echo "ERROR: $(hostname): $OVERRIDE not a CVS checkout?"
	exit 1
    fi
    #OVSUFFIX=${OVERRIDE//\//_} OVSUFFIX=${OVSUFFIX/#_/-custom-}
    OVSUFFIX="-private-build"
else
    OVSUFFIX=
fi

# once submitted, "build_linux.sh -q none(in bsub)" leads to here. Build one by one
if [ "${LSB_JOBINDEX:-0}" -gt 0 ] ; then
    i=$((${LSB_JOBINDEX}-1))
    if   [ "$OSFLAG" == "R32BIT" ]; then
         BRANDS=(${R32BUILDS[${i}]})
    elif [ "$OSFLAG" == "RHEL40" ]; then
         BRANDS=(${R64BUILDS[${i}]})
    fi
    echo "OSFLAG = $OSFLAG"
    export OSFLAG
fi

if [ -f ${SESSIONLOG} -a "${LSFUSER}" == "hwnbuild" ]; then
   chmod ug+w ${SESSIONLOG}
fi

echo "$(date '+%b %d %H:%M:%S') $(hostname) build_linux[pid=$$,lsf=${LSB_JOBID}]: START brands=${BRANDS}, tag=${TAG:-NIGHTLY}, user=${LSFUSER}, override=${OVERRIDE};" >> ${SESSIONLOG}


###### Debug #########
# echo "DBG: Exiting 1"
# exit 1

# Make sure jobs won't hang on a read from stdin. LSF takes
# care of this automatically but with "-q none" it's needed.
exec < ${NULL}

# For every brand specified, now go ahead and build with $MAKE
for i in $(seq 0 $((${#BRANDS[*]}-1))) ; do
    BRAND=${BRANDS[${i}]}

    # Turn off local builds for firmware build to debug
    # dhd/bmac build synchronization
    if [ "$BRAND" == "hndrte-dongle-wl" ]; then
       LOCAL_BUILD=
    fi

    echo "Building BRAND = $BRAND"
    # Format version string like epivers.sh
    yyyy=$(date '+%Y')
    mm=$(date '+%m')
    dd=$(date '+%d')
    m=${mm/#0/}
    d=${dd/#0/}

    # Make base directory
    TAG_DIR="${BUILD_BASE}/${TAG:-NIGHTLY}"
    BRAND_DIR="${TAG_DIR}/${BRAND}"
    if ! mkdir -p "${BRAND_DIR}" ; then
	echo "$(hostname): Creation of BRAND_DIR ${BRAND_DIR} failed"
	continue
    fi

    # search new build directory and delete previous fail builds
    # try up to 20 times
    iteration=0
    for j in $(seq 0 20); do
	BUILD_DCHECK="${BRAND_DIR}/${yyyy}.${m}.${d}.${j}${OVSUFFIX}"
	BUILD_ITER="${TAG:-NIGHTLY}/${BRAND}/${yyyy}.${m}.${d}.${j}${OVSUFFIX}"
	if [ -e "${BUILD_DCHECK}" ]
	then
		iteration=$(( j + 1 ))
	fi

	# delete previous fail build iteration on current date
	if [ -e "${BUILD_DCHECK}/${RLSLOG}" ]
	then
		log="${BUILD_DCHECK}/${RLSLOG}"
		errorlog="${BUILD_DCHECK}/${ERRORLOG}"
		if [ -f "${errorlog}" ]
		then
			echo "INFO: Deleting failed build: $BUILD_ITER"
			rm -rf "${BUILD_DCHECK}"
			continue
		fi
		if echo ${TAG:-NIGHTLY} | grep -v -i -q "_BRANCH_\|_TWIG_\|NIGHTLY"; then
			if [ -f "${BUILD_DCHECK}/${SUCCESSLOG}" ]
			then
				echo "INFO: Found duplicate build: $BUILD_ITER"
			else
				# These may be builds in progress
				echo "INFO: Found in-complete build: $BUILD_ITER"
			fi
		fi
	fi

	[ "$j" == "20" ] && break
    done

    # For non TOT/TOB builds, delete previous failed build from earlier dates
    if echo ${TAG:-NIGHTLY} | grep -v -i -q "_BRANCH_\|_TWIG_\|NIGHTLY"; then
	BUILD_ITERS=$(find ${BRAND_DIR} -maxdepth 1 -mindepth 1 -type d -not -name "*${yyyy}.${m}.${d}.*")
	for iter in $BUILD_ITERS; do
		BUILD_ITER=$(basename $iter)
		BUILD_ITER="${TAG:-NIGHTLY}/${BRAND}/$BUILD_ITER"
		if [ -e "${iter}/${RLSLOG}" ]
		then
			log="${iter}/${RLSLOG}"
			errorlog="${iter}/${ERRORLOG}"
			if [ -f "${errorlog}" ]
			then
				echo "INFO: Deleting old failed build: ${BUILD_ITER}"
				rm -rf "${iter}"
			elif [ -f "${iter}/${SUCCESSLOG}" ]
			then
				echo "INFO: Found old duplicate build: ${BUILD_ITER}"
			else
				# These may be builds in progress
				echo "INFO: Found old in-complete build: ${BUILD_ITER}"
			fi
		fi
	done
    fi

    BUILD_TIME="${yyyy}.${m}.${d}.${iteration}${OVSUFFIX}"
    BUILD_DIR="${BRAND_DIR}/${BUILD_TIME}"

    # Make and enter new build directory
    if ((${iteration} > 20)) ; then
	echo "$(hostname): Creation of BUILD_DIR ${BUILD_DIR} failed (exceeded iterations)"
	continue
    else
	mkdir "${BUILD_DIR}" > ${NULL} 2>&1
	if [ "$?" != "0" ]; then
	    echo "$(hostname): Creation of BUILD_DIR ${BUILD_DIR} failed"
	    continue
	fi
    fi

    cd "${BUILD_DIR}"

    # Local build flag is suitable only in wlan sw build queue nodes
    if [ -n "$LOCAL_BUILD" ]; then
       wlanlsfnode=$(bhosts $WLAN_LSF_QUEUE 2> $NULL | egrep -q ${HOSTNAME})
       if [ "$wlanlsfnode" != "" -o "$LSF" == "none" ]; then

          echo "INFO: Local build flag specified"
          # If local disk-space usage is lower than BUILD_LOCAL_MINDU MB
          # then disable LOCAL_BUILD
          tmpdu=$(df -hm /tmp | tail -1 | awk '{print $4}')
          echo "INFO: Local disk-usage: $tmpdu"

          if [ "$tmpdu" -ge "$BUILD_LOCAL_MINDU" ]; then
             let i=0; let j=0
             LOCAL_BASE=$LOCAL_BUILD_ROOT/$USER/${TAG:-TRUNK}/${BRAND}_$(date '+%y%m%d')
             LOCAL_BASE_ORIG=${LOCAL_BASE}
             LOCAL_BASE=${LOCAL_BASE_ORIG}_${i}

             echo "Trying to create ${LOCAL_BASE}"

             # If LOCAL_BASE exists, cycle through available LOCAL_BASE_<iter>
             # New dir created is ALWAYS LOCAL_BASE_<iter+1>
             if [ -d "${LOCAL_BASE}" ]; then
                for i in $(seq 0 $LOCAL_BUILD_ITERS); do
                    if [ -d ${LOCAL_BASE}_${i} ]; then
                       j=$(($i+1))
                    fi
                done
             fi

             # Now j contains next iteration of the BRAND_DIR that needs to be created
             let i=$j

             while ! mkdir -pv --mode=777 $LOCAL_BASE > ${NULL} 2>&1
             do
                 echo "Cannot create new ${LOCAL_BASE}"
                 if (( $j < $LOCAL_BUILD_ITERS ))
                 then
                    LOCAL_BASE=${LOCAL_BASE_ORIG}_${i}
                    echo "Trying ${LOCAL_BASE} instead"
                    let i=i+1;
                 fi;
             done

             if [ -d "$LOCAL_BASE" ]; then
                echo "INFO: Local build at $LOCAL_BASE on $HOSTNAME"
                tmpduinfo=$(df -h $LOCAL_BASE | grep -v Filesystem | awk '{printf "Avail %s; Use %s\n", $4, $5}')

                echo "[`date`]"                                            >> ${RLSLOG}
                touch $WIP_FILE
                echo "INFO: Local build flag set for ${TAG:-TRUNK}:$BRAND" >> ${RLSLOG}
                echo "INFO: Build will run on $HOSTNAME at $LOCAL_BASE"    >> ${RLSLOG}
                echo "INFO: Local diskspace info: $tmpduinfo"                  >> ${RLSLOG}
                echo "INFO: Run 'bjobs $LSB_BATCH_JID' to query status or" >> ${RLSLOG}
                echo "INFO: rsh $HOSTNAME tail $LOCAL_BASE/,release.log"   >> ${RLSLOG}
                chmod ugo+w $LOCAL_BUILD_ROOT $LOCAL_BUILD_ROOT/$USER
             else
                LOCAL_BUILD=
                echo "[`date`]"                                            >> ${RLSLOG}
                echo "INFO: Local build flag set for ${TAG:-TRUNK}:$BRAND" >> ${RLSLOG}
                echo "WARN: Creation of $LOCAL_BASE failed on $HOSTNAME"   >> ${RLSLOG}
                echo "WARN: Disabling local build in $LSB_BATCH_JID job"   >> ${RLSLOG}
             fi
          else # tmpdu
             LOCAL_BUILD=
             echo "[`date`]"                                            >> ${RLSLOG}
             echo "INFO: Local build flag set for ${TAG:-TRUNK}:$BRAND" >> ${RLSLOG}
             echo "WARN: But local disk space low on $HOSTNAME"         >> ${RLSLOG}
             echo "WARN: Disabling local build in $LSB_BATCH_JID job"   >> ${RLSLOG}
          fi
       else # bhosts
          LOCAL_BUILD=
          echo "[`date`]"                                          >> ${RLSLOG}
          echo "WARN: Local build flag specified"                  >> ${RLSLOG}
          echo "WARN: However $HOSTNAME is NOT WLAN SW node"       >> ${RLSLOG}
          echo "WARN: Disabling local build in $LSB_BATCH_JID job" >> ${RLSLOG}
       fi
    fi

    # If no sparse files are found, when the build is started
    # without -flag "CVSROOT=/projects/cvsroot", for automated
    # builds use cvs to checkout. This is used by dhd/bmac
    # builds that need firmware build launched
    if echo "${BUILD_DIR}" | egrep -qi "${BUILD_BASE_LINUX_PREBUILD}|PREBUILD_DONGLE"; then
       if [ -s "${AUTH}" ]; then
         password=`cat ${AUTH}`
       fi
       echo "DBG: svn ls ${REPO_URL}"
       if [ "$VERBOSE" != "" ]; then
          echo "DBG: svn ls --username hwnbuild --password '$password' ${REPO_URL} 2> $NULL | egrep '\.sparse'"
       fi
       sparses_found=$(svn ls --username hwnbuild --password "$password" ${REPO_URL} 2> $NULL | egrep "\.sparse")
       if [ "$sparses_found" == "" ]; then
          echo "WARN:"
          echo "WARN: No sparse files are found for $TAG and $BRAND at"
          echo "WARN: $REPO_URL. Defaulting to CVS for private dongle build"
          echo "WARN:"
          export VCTOOL=cvs
          FIND_BUILD_ERRORS="$FIND_BUILD_ERRORS -cvs"
      fi
    fi

    mkdir "${BUILD_DIR}/${MISC_DIR}" > ${NULL} 2>&1

    if [ -n "$LOCAL_BUILD" ]; then
       echo "Changing dir to $LOCAL_BASE"
       cd $LOCAL_BASE
       mkdir ${MISC_DIR}
       # In-case one needs to know, where is intended destination given
       # a temp build dir, put it in this file. We can use it for reporting
       # of any sort in future
       echo "Local build dir: $LOCAL_BASE" >> ${MISC_DIR}/BUILD_DIR_INFO.txt
       echo "Final build dir: $BUILD_DIR"  >> ${MISC_DIR}/BUILD_DIR_INFO.txt
    fi

    # Check out branded Makefile, if no TAG, use TOT
    # If version control tool requested is svn, then get release makefile
    # from SVN
    if [ "$VCTOOL" == "svn" ]; then
       # Checkout checkout rules before release makefile is checked out
       echo "$SVNCMD export ${REPO_URL_TRUNK}/src/tools/release/${CHECKOUT_DEFS}"
       $SVNCMD export ${REPO_URL_TRUNK}/src/tools/release/${CHECKOUT_DEFS} \
               ${CHECKOUT_DEFS}
       echo "$SVNCMD export ${REPO_URL_TRUNK}/src/tools/release/${CHECKOUT_RULES}"
       $SVNCMD export ${REPO_URL_TRUNK}/src/tools/release/${CHECKOUT_RULES} \
               ${CHECKOUT_RULES}
       echo "$SVNCMD export ${SVNCUTOFF:+-r \{'$SVNCUTOFF $SVNTZ'\}} ${SVNREV:+-r $SVNREV} ${REPO_URL}/src/tools/release/${BRAND}.mk release.mk"
       $SVNCMD export ${SVNCUTOFF:+-r \{"$SVNCUTOFF $SVNTZ"\}} ${SVNREV:+-r $SVNREV} ${REPO_URL}/src/tools/release/${BRAND}.mk \
               release.mk
       vctoolrc=$?
    else
       # Checkout checkout rules before release makefile is checked out
       $VCTOOL -Q -d $CVSROOT co -p src/tools/release/${CHECKOUT_DEFS} > ${CHECKOUT_DEFS}
       $VCTOOL -Q -d $CVSROOT co -p src/tools/release/${CHECKOUT_RULES} > ${CHECKOUT_RULES}
       echo "$VCTOOL -Q co ${TAG:+-r $TAG} ${CVSCUTOFF:+-D \"$CVSCUTOFF\"} -p src/tools/release/${BRAND}.mk"
       $VCTOOL -Q co ${TAG:+-r $TAG} ${CVSCUTOFF:+-D "$CVSCUTOFF"} -p src/tools/release/${BRAND}.mk > release.mk
       vctoolrc=$?
    fi

    if [ "$vctoolrc" != "0" -a ! -n "$PVT_MAKEFILE" ] ; then
       if [ -n "$SVNREV" ]; then
          echo "INFO: $(hostname): SVNREV    = $SVNREV"           >> ${RLSLOG}
       elif [ -n "$SVNCUTOFF" ]; then
          echo "INFO: $(hostname): SVNCUTOFF = $SVNCUTOFF $SVNTZ" >> ${RLSLOG}
       fi
       echo "ERROR: $(hostname): Checkout of src/tools/release/${BRAND}.mk"
       echo "$SVNCMD export ${SVNCUTOFF:+-r \{'$SVNCUTOFF $SVNTZ'\}} ${SVNREV:+-r $SVNREV} ${REPO_URL}/src/tools/release/${BRAND}.mk release.mk" >> ${RLSLOG}
       echo "ERROR: $(hostname): Checkout of src/tools/release/${BRAND}.mk" \
            >> ${RLSLOG}
       if [ "$VCTOOL" == "svn" ]; then
          $SVNCMD export ${REPO_URL}/src/tools/release/${BRAND}.mk \
                  release.mk >> ${RLSLOG} 2>&1
       else
          $VCTOOL -Q co ${TAG:+-r $TAG} ${CVSCUTOFF:+-D "$CVSCUTOFF"} -p src/tools/release/${BRAND}.mk > release.mk 2>> ${RLSLOG}
       fi
       continue
    fi

    if [ -n "$OVERRIDE" ] && [ -f ${OVERRIDE}/tools/release/${BRAND}.mk ]; then
        echo "Copy private ${OVERRIDE}/tools/release/${BRAND}.mk -> release.mk"
        cp -pv ${OVERRIDE}/tools/release/${BRAND}.mk release.mk
        if [ "$?" != "0" ] ; then
           echo "WARN: $(hostname): Copy of ${OVERRIDE}/tools/release/${BRAND}.mk failed"
          continue
       fi
    fi

    if [ -n "$PVT_MAKEFILE" ] && [ -s ${PVT_MAKEFILE} ]; then
       echo "Copying private release makefile:"
       cp -pv $PVT_MAKEFILE release.mk
       if [ "$?" != "0" ] ; then
          echo "ERROR: $(hostname): Copy of ${PVT_MAKEFILE} failed"
          exit 1
       fi
    fi

    # For debugging set following and it is overrides value on checkout-*.mk
    # export HNDSVN_CMD=sparse_dbg.sh

    # Echo status information
    echo "START_TIME      = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}
    echo "PWD             = ${PWD}"                    >> ${RLSLOG}
    echo "BUILD_DIR       = ${BUILD_DIR}"              >> ${RLSLOG}
    if [ -n "$LOCAL_BUILD" ]; then
       echo "LOCAL_DIR       = ${LOCAL_BASE}"          >> ${RLSLOG}
       echo "LOCAL DISK INFO = $tmpduinfo"             >> ${RLSLOG}
    fi
    echo "BUILD_HOSTOS    = ${HOSTOS}"                 >> ${RLSLOG}
    echo "BUILD_HOST      = ${HOSTNAME}"               >> ${RLSLOG}
    echo "BUILD_USER      = ${USER}"                   >> ${RLSLOG}
    echo "BUILD_HOST_ARCH = $(uname -p)"               >> ${RLSLOG}
    echo "TAG             = ${TAG}"                    >> ${RLSLOG}
    echo "BRAND           = ${BRAND}"                  >> ${RLSLOG}
    if [ "$VCTOOL" == "cvs" -a "${CVSCUTOFF}" != "" ]; then
       echo "CVS CUTOFF      = ${CVSCUTOFF}"
       echo "CVS CUTOFF      = ${CVSCUTOFF}"           >> ${RLSLOG}
    fi
    if [ "$VCTOOL" == "svn" -a "${SVNCUTOFF}" != "" ]; then
       echo "SVN CUTOFF      = ${SVNCUTOFF} ${SVNTZ}"
       echo "SVN CUTOFF      = ${SVNCUTOFF} ${SVNTZ}"  >> ${RLSLOG}
    fi
    if [ "$VCTOOL" == "svn" -a "${SVNREV}" != "" ]; then
       echo "SVNREV          = ${SVNREV}"
       echo "SVNREV          = ${SVNREV}"              >> ${RLSLOG}
    fi
    if [ "${GMAKEFLAGS[*]}" != "" ]; then
       echo "GMAKEFLAGS      = ${GMAKEFLAGS[*]}"
       echo "GMAKEFLAGS      = ${GMAKEFLAGS[*]}"       >> ${RLSLOG}
    fi
    [ -n "$OVERRIDE" ] && echo "OVERRIDE=${OVERRIDE}"  >> ${RLSLOG}
    echo "PATH            = ${PATH}"                   >> ${RLSLOG}
    if [ -s "${PVT_MAKEFILE}" ]; then
       echo "*PVT MAKEFILE   = ${PVT_MAKEFILE}"        >> ${RLSLOG}
    fi
    if [ "${EXTRA_LOG_STATUS}" != "" ]; then
       echo "-----------------------------------------">> ${RLSLOG}
       echo "EXTRA LOG STATUS:"                        >> ${RLSLOG}
       echo "${EXTRA_LOG_STATUS}"                      >> ${RLSLOG}
    fi
    echo "LSF SLOTS       = ${LSFSLOTS}"               >> ${RLSLOG}
    echo "LSF JOB ID      = ${LSB_BATCH_JID}"          >> ${RLSLOG}
    echo "LSF JOB PID     = ${LSB_JOBRES_PID}"         >> ${RLSLOG}
    echo "--------------------------------------------">> ${RLSLOG}
    env | sort | awk -F= '{printf "%20s = %-s\n",$1,$2}'> ${ENVLOG}
    echo "--------------------------------------------">> ${ENVLOG}
    echo "SCRIPT VERSIONS USED:"                       >> ${ENVLOG}
    echo "SCRIPT VERSION     = $SCRIPT_VERSION"        >> ${ENVLOG}
    echo "FIND_BUILD_ERRORS  = $FIND_BUILD_ERRORS"     >> ${ENVLOG}
    echo "BUILD_CONFIG       = $BUILD_CONFIG"          >> ${ENVLOG}
    echo "SHOW_OLD_BUILDITERS= $SHOW_OLD_BUILDITERS"   >> ${ENVLOG}
    echo "FIND_ECAGENT       = $FIND_ECAGENT"          >> ${ENVLOG}
    echo "HNDMAKE            = $HNDMAKE"               >> ${ENVLOG}
    echo "--------------------------------------------">> ${ENVLOG}

    case "`uname -r`" in
         *2.6.9*)
             echo "Build Machine OS: $(uname -r)" > ${RHEL4BUILD}
             ;;
         *2.6.18*)
             echo "Build Machine OS: $(uname -r)" > ${RHEL5BUILD}
             ;;
    esac

    # For now check for emake compatibility only for firmware and linux*wl
    if checkBrand $BRAND isEmakeCompatible $EMAKECHECK_DIR; then
       [ -n "$VERBOSE" ] && echo "INFO: Found emake build brand: $BRAND"
       if echo $BRAND | egrep -q "hndrte-dongle-wl|linux-.*-wl"; then
          EMAKE=1
       fi
    fi # BRAND

    MYMAKE="gmake"
    if [ "$EMAKE" == "1" -a "$GMAKE" != "1" ]; then
       MYMAKE="$HNDMAKE"
    fi

    if [ -n "$OVERRIDE" ]; then
       # If COPYSRC is set, OVERRIDE is copied as is with no patching
       # with rest of $TAG (or TOT)
       if [ -n "$LOCAL_BUILD" ]; then
          COPY_DIR=$LOCAL_BASE
       else
          COPY_DIR=$BUILD_DIR
       fi
       if echo " ${GMAKEFLAGS[*]} " | grep -qi " COPYSRC=1 " ; then
          export SRC_CHECKOUT_DISABLED=1
          echo "Copying sources from $OVERRIDE to $COPY_DIR"
          echo "Copying sources from $OVERRIDE to $COPY_DIR" >> ${RLSLOG}
          cp -rp $OVERRIDE $COPY_DIR 2>> ${RLSLOG}
          if [ "$?" != "0" ]; then
             echo "ERROR: Copying from private tree to build directory failed"
             exit 1
          fi
          if echo "$BRAND" | egrep -i "external|mfgtest"; then
             echo "Stripping .svn bits from private external build"
             find $COPY_DIR/src -type d -name ".svn" -print0 | \
                  xargs -t -l1 -0 rm -rf
          fi
       fi
    fi

    echo "Make version used: `which $MYMAKE`"            >> ${ENVLOG}
    $MYMAKE --version                                    >> ${ENVLOG}
    echo "--------------------------------------------"  >> ${ENVLOG}

    touch $WARN_FILE
    echo "$(hostname): Build started at $(date)"
    if [ "$VCTOOL" == "cvs" ]; then
       echo "$(hostname): CVSROOT=$CVSROOT"
    fi

    if [ "$VCTOOL" == "svn" ]; then
       # EMAKE Build
       echo "$MYMAKE -w -f release.mk \
             ${VCTOOL:+VCTOOL=$VCTOOL} \
             ${COTOOL:+COTOOL=$COTOOL} \
             ${TAG:+TAG=$TAG} \
             BRAND=${BRAND} \
             ${OVERRIDE:+OVERRIDE=$OVERRIDE} \
             ${SVNCUTOFF:+SVNCUTOFF=\"$SVNCUTOFF\"} \
             ${SVNREV:+SVNREV=\"$SVNREV\"} \
             ${GMAKEFLAGS[*]}" \
             >> ${RLSLOG}
       # SVNCUTOFF is exported into the env and inherited by release.mk
       $MYMAKE -w -f release.mk \
             ${VCTOOL:+VCTOOL=$VCTOOL} \
             ${COTOOL:+COTOOL=$COTOOL} \
             ${TAG:+TAG=$TAG} \
             BRAND=${BRAND} \
             ${OVERRIDE:+OVERRIDE=$OVERRIDE} \
             ${SVNREV:+SVNREV=$SVNREV} \
             "${GMAKEFLAGS[@]}" \
             >> ${RLSLOG} 2>&1
    else
       # EMAKE Build
       echo "$MYMAKE -w -f release.mk \
             ${VCTOOL:+VCTOOL=$VCTOOL} \
             ${TAG:+TAG=$TAG} \
             BRAND=${BRAND} \
             ${OVERRIDE:+OVERRIDE=$OVERRIDE} \
             ${CVSCUTOFF:+CVSCUTOFF=\"$CVSCUTOFF\"} \
             ${GMAKEFLAGS[*]}" \
             >> ${RLSLOG}

       # CVSCUTOFF is exported into the env and inherited by release.mk
       $MYMAKE -w -f release.mk \
             ${VCTOOL:+VCTOOL=$VCTOOL} \
             ${TAG:+TAG=$TAG} \
             BRAND=${BRAND} \
             ${OVERRIDE:+OVERRIDE=$OVERRIDE} \
             "${GMAKEFLAGS[@]}"  \
             >> ${RLSLOG} 2>&1
    fi

    # Exit code from make captured in makerc
    makerc=$?

    # For external and mfgtest builds, see if any .svn folders are still
    # remaining in src or release folders
    if echo "$BRAND" | egrep -i "external|mfgtest"; then
       find src -type d -name ".svn" -print > ${MISC_DIR}/src_dot_svn_found.log 2> $NULL
       find release -type d -name ".svn" -print > ${MISC_DIR}/release_dot_svn_found.log 2> $NULL
       tar_packages=$(find release -type f -name "*.tar" -o -name "*.tar.gz" -print)
       for pkg in $tar_packages
       do
           pkgname=$(basename $pkg)
           if echo $pkg | egrep -q ".tar.gz"; then
              tar tzf $pkg > ${MISC_DIR}/${pkgname}.txt
              if egrep -q "\.svn" ${MISC_DIR}/${pkgname}.txt; then
                 echo "Package $pkg contains .svn folders" ${MISC_DIR}/targz_dot_svn_found.log
              fi
              rm -f ${MISC_DIR}/${pkgname}.txt
           else
              tar tf $pkg > ${MISC_DIR}/${pkgname}.txt
              if egrep -q "\.svn" ${MISC_DIR}/${pkgname}.txt; then
                 echo "Package $pkg contains .svn folders" ${MISC_DIR}/tar_dot_svn_found.log
              fi
              rm -f ${MISC_DIR}/${pkgname}.txt
           fi
       done

       # Notify Build Admin if any .svn folders sneak in when the build is successful
       if [ -s "${MISC_DIR}/src_dot_svn_found.log" -o -s "${MISC_DIR}/release_dot_svn_found.log" -o -s "${MISC_DIR}/tar_dot_svn_found.log" -o -s "${MISC_DIR}/targz_dot_svn_found.log" ]; then
          if [ -f "${SUCCESSLOG}" ]; then
             cat ${MISC_DIR}/src_dot_svn_found.log \
                 ${MISC_DIR}/release_dot_svn_found.log \
                 ${MISC_DIR}/tar_dot_svn_found.log \
                 ${MISC_DIR}/targz_dot_svn_found.log 2> $NULL | col -b | \
             mail -s "ERROR: .svn folders found at $BUILD_DIR" $ADMINMAILTO
          fi
       fi
    fi

    echo "MAKE_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}
    # Strip out cluttered lengthy build path prefixes when generating
    # error messages. Build email/message and RLSLOG has that info already.
    BDIR=$(pwd | sed -e 's/\//./g' -e 's/\\/./g').
    BDIR_UX=$(pwd -P| sed -e 's/\//./g' -e 's/\\/./g').

    # Create 'last' symlink to last build that completed
    #rm -f ${BRAND_DIR}/last
    #ln -s ${BUILD_TIME} ${BRAND_DIR}/last

    # If a user explicitly asks for CVS build and new checkout-rules haven't
    # been included to create _CVS_BUILD flag file, let build script create
    # those flag files to easily identify builds
    if echo " ${GMAKEFLAGS[*]} " | grep -qi " CVSROOT=" ; then
       if [ ! -f "_CVS_BUILD" ]; then
          echo "INFO: Marking $TAG $BRAND as a CVS build explicitly"
          echo "$CVSROOT" > _CVS_BUILD
       fi
    fi

    # Echo status information
    if [ "$makerc" != "0" ] ; then
	echo "$(hostname): Build FAILED at $(date) with $makerc exit code"
        [ -d "release" ] && touch release/$WARN_FILE

        # For emake, find the last agent node where failure occured (if any)
        if [ -f "${MISC_DIR}/,emake_${BRAND}_anno.xml" ]
then
           echo "${FIND_ECAGENT} --anno=${MISC_DIR}/,emake_${BRAND}_anno.xml"
           sleep 10 # wait until .xml file is flushed after the build
           failedagent=$(${FIND_ECAGENT} --anno=${MISC_DIR}/,emake_${BRAND}_anno.xml 2> $NULL)
           gzip -9 ${MISC_DIR}/,emake_${BRAND}_anno.xml 2> $NULL
           echo "*** Last Build Failure on \"$failedagent\"" >> ${RLSLOG}
        fi
    else
        touch ${SUCCESSLOG}.`date +%F%H%M%S`
        touch ${SUCCESSLOG}
        rm -f $WARN_FILE

        ## Sometimes make exits with '0', even when there are errors
        errors=$(grep "Error [0-9]\+[[:space:]]*$" ${RLSLOG} | wc -l | xargs printf "%d")
        if [ "${errors}" != "0" ] ; then
           ${FIND_BUILD_ERRORS} -p linux ${TAG:+-r $TAG} -v ${RLSLOG} | \
                perl -ne "s/$BDIR//gi; s/$BDIR_UX//gi; print" | \
                perl -ne 's/^(\d+)[:-]/[$1] /g; print' > \
                ${IGNORE_ERRORLOG}
           # brand specific customization to mark hndrte* brands
           # module ignored errors as whole-build errors
           # as build has been made to continue in-spite of intermediate errors
           if [ "${BRAND}" == "hndrte" -o "${BRAND}" == "hndrte-dongle-wl" ]; then
              rm -f ${SUCCESSLOG} ${IGNORE_ERRORLOG}
           else
              echo "$(hostname): Build succeeded with ERRORS IGNORED at $(date)"
              touch $IGNORE_FILE
              [ -d "release" ] && touch release/$IGNORE_FILE
           fi
        else
           if [ -f "$WARN_VCTOOL_FILE" ]; then
              echo "$(hostname): WARN: Built with wrong cvs or svn checkout"
           else
              echo "$(hostname): Build succeeded at $(date)"
           fi
        fi
    fi # gmake $?

    # Build succeeded
    #if [ -f "${SUCCESSLOG}" ] ; then
    #   # Create 'good' symlink to last build that completed successfully
    #	rm -f ${BRAND_DIR}/good
    #	ln -s ${BUILD_TIME} ${BRAND_DIR}/good
    #fi

    # Build failed
    if [ ! -f "${SUCCESSLOG}" ] ; then
       echo "  === Build Node: `uname -nrm` ==="
       # Generate ${ERRORLOG} for use by build_summary script
       rm -f ${ERRORLOG}
       echo "${TAG:-TOT} $BRAND full build log at: ${RLSLOG}" > ${ERRORLOG}
       echo "Filtered Errors:"                                >> ${ERRORLOG}
       echo ""                                                >> ${ERRORLOG}
       ${FIND_BUILD_ERRORS} -p linux ${TAG:+-r $TAG} ${RLSLOG} | \
               perl -ne "s/$BDIR//gi; s/$BDIR_UX//gi; print" | \
               perl -ne 's/^(\d+)[:-]/[$1] /g; print' >> ${ERRORLOG}

       # If find_build_errors can't filter out errors, when the build
       # fails, show last 15 lines from ,release.log
       found_errors=$(cat $ERRORLOG)
       if [ ! "$found_errors" ]; then
          tail -15 $RLSLOG | \
               perl -ne "s/$BDIR//gi; s/$BDIR_UX//gi; print" | \
               perl -ne 's/^(\d+)[:-]/[$1] /g; print' >> ${ERRORLOG}
       fi

       # If Dongle build had a ,build_errors.log file, include it
       # If not, sometimes DHD/BMAC builds fail with firmwares that are
       # intentionally skipped. Identify such instances and report them
       # as warnings
       if [ -s "${DONGLE_ERRORLOG}" ]; then
          echo -e "\n  === Start: Dongle Build Errors ===\n" >> ${ERRORLOG}
          cat  "${DONGLE_ERRORLOG}"                          >> ${ERRORLOG}
          echo -e "\n  === End  : Dongle Build Errors ===\n" >> ${ERRORLOG}
       elif [ -s "${DONGLE_RLSLOG}" ]; then
          echo -e "\n  === Start: Dongle Build Warnings ===\n" >> ${ERRORLOG}
          ${FIND_BUILD_ERRORS} -p linux ${TAG:+-r $TAG} ${DONGLE_RLSLOG} | \
               perl -ne "s/$BDIR//gi; s/$BDIR_UX//gi; print" | \
               perl -ne 's/^(\d+)[:-]/[$1] /g; print'        >> ${ERRORLOG}
          echo -e "\n  === End  : Dongle Build Warnings ===\n" >> ${ERRORLOG}
       fi

       echo "Build Errors:"
       if [ -s "${ERRORLOG}" ]; then
          echo "  === Build Node: `uname -nrm` ===" >> ${ERRORLOG}
          unix2dos ${ERRORLOG} > ${NULL} 2>&1
          cat "${ERRORLOG}" | col -b
       fi
    fi

    echo "  === Build Node: `uname -nrm` ===" >> ${RLSLOG}
    echo "  === Node Usage: `uptime` ==="     >> ${RLSLOG}

    echo "ANALYSIS_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

    # Default build keep counts are $BUILDS_TO_KEEP_TOT or BUILDS_TO_KEEP_TOB
    # If current build fails, then keep count is extended automatically below
    if [ "${BUILDS_TO_KEEP}" != "" ] ; then
       echo "Old Builds To delete:" > ${MOVELOG}
       if [ ! -f "${SUCCESSLOG}" ]; then
          BUILDS_TO_KEEP=$(expr ${BUILDS_TO_KEEP} + $(expr ${BUILDS_TO_KEEP} / 2))
          echo "Build preserve time (extended): ${BUILDS_TO_KEEP}" >> ${MOVELOG}
       else
          echo "Build preserve time : ${BUILDS_TO_KEEP}" >> ${MOVELOG}
       fi

       oldbuilds="$(${SHOW_OLD_BUILDITERS} -keep_count ${BUILDS_TO_KEEP} -brand_dir ${BRAND_DIR})"
       for bld in ${oldbuilds}; do
           if echo $bld | egrep -q -i -v "/${TAG:-NIGHTLY}/${BRAND}/"; then
              echo "WARN: '$bld' not a build" >> ${MOVEOG}
              continue;
           fi
           echo "`date '+%Y-%m-%d %H:%M:%S'` Start Delete: $bld" >> ${MOVELOG}
           rm -rf "${bld}" >> ${MOVELOG} 2>&1
           echo "`date '+%Y-%m-%d %H:%M:%S'` End   Delete: $bld" >> ${MOVELOG}
       done
    fi # BUILDS_TO_KEEP

    # Remove links that have become invalid due to removals
    [ ! -d ${BRAND_DIR}/good ] && rm -f ${BRAND_DIR}/good
    [ ! -d ${BRAND_DIR}/last ] && rm -f ${BRAND_DIR}/last

    echo "OLD_ITERATIONS_TO_REMOVE = $oldbuilds"             >> ${RLSLOG}
    echo "OLD_BUILDS_REMOVED = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

    ## Preserve critical files longer than usual nightly recycle duration
    ## for only regular nightly tot/tob builds or respins.
    if echo ${TAG:-NIGHTLY} | grep -i -q "_BRANCH_\|_TWIG_\|NIGHTLY"; then
       if [ "${LSFUSER}" == "hwnbuild" -a -d "${BUILD_BASE}/PRESERVED" -a -f "${PRESERVE_LIST}" ]; then
          preservedir="${BUILD_BASE}/PRESERVED"
          preservedir="${preservedir}/${TAG:-NIGHTLY}"
          preserved_brand_flag=0
          PRESERVELOG=${preservedir}/,preserve.log

          # Remove old preserved build items
          if [ -d "${preservedir}/${BRAND}" ]; then
             rm -rf $(find ${preservedir}/${BRAND} -maxdepth 1 -mindepth 1 -not -mtime -${PRESERVE_CTIME})
             echo "rm -rf $(find ${preservedir}/${BRAND} -maxdepth 1 -mindepth 1 -not -mtime -${PRESERVE_CTIME})" >> ${PRESERVELOG}
          fi

          preservedir="${preservedir}/${BRAND}"
          preservedir="${preservedir}/${yyyy}.${m}.${d}.${iteration}"
          for keep in $(cat "${PRESERVE_LIST}" | sed -e "s/[[:space:]]//g" | egrep -v "^#" | egrep "build_linux:${BRAND}:")
          do
              preserved_brand_flag=1
              mkdir -pv ${preservedir}
              ## PRESERVELIST format build_linux : BRAND : file_to_preserve
              file=$(echo $keep | awk -F: '{print $3}')
              echo "cp $file ${preservedir}/$file" >> ${PRESERVELOG}
              (tar cpf - $file | ( cd $preservedir; tar xpf - )) >> ${PRESERVELOG} 2>&1
          done
          if [ "$preserved_brand_flag" != "0" ]; then
              for logf in $PRESERVE_LOGS; do
                  if [ -f "$logf" ]; then
                     cp -p $logf $preservedir
                  fi
              done
              gzip -f -r -q -9 $preservedir
              echo "==================================================" >> ${PRESERVELOG}
          fi
       fi
    fi

    echo "CRITICAL_FILES_PRESERVED = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

    echo "";
    if echo $BUILD_DIR | grep -q "/projects/hnd"; then
       WEBRLSURL=$(echo ${URLPFX}${BUILD_DIR}/${RLSLOG} | sed -e "s%${BUILD_BASE}%${BUILD_BASE_MOUNT}%")
       WEBDNGLURL=$(echo ${URLPFX}${BUILD_DIR}/${DONGLE_RLSLOG} | sed -e "s%${BUILD_BASE}%${BUILD_BASE_MOUNT}%")
       echo "ReleaseLog = ${WEBRLSURL}"
       if [ -s "${DONGLE_RLSLOG}" ]; then
          echo "DongleLog  = ${WEBDNGLURL}"
       fi
    else
       echo "ReleaseLog = ${BUILD_DIR}/${RLSLOG}"
       if [ -s "${DONGLE_RLSLOG}" ]; then
          echo "DongleLog  = ${BUILD_DIR}/${DONGLE_RLSLOG}"
       fi
    fi

    ## Resetting permissions on $BUILD_DIR or LOCAL_BASE to read-only for others
    if [ -n "$LOCAL_BUILD" ]; then
       echo "Resetting permissions on $LOCAL_BASE to read-only for others"
       find $LOCAL_BASE -perm +22 -print0 | xargs -0 -r chmod -f go-w
    else
       echo "Resetting permissions on $BUILD_DIR to read-only for others"
       find $BUILD_DIR -perm +22 -print0 | xargs -0 -r chmod -f go-w
    fi

    # For public builds mark the current brand dir as readonly for others
    if [ "${BUILD_BASE}" == "${BUILD_BASE_LINUX}" -o "${BUILD_BASE}" == "${BUILD_BASE_LINUX_MOUNT}" ]; then
       chmod go-w .
       if [ -n "$LOCAL_BUILD" ]; then
          chmod go-w $LOCAL_BASE
       fi
    fi

    # Post build compression/cleanup of intermediate parallel build files
    if [ -f "${MISC_DIR}/,emake_${BRAND}_anno.xml" ]; then
       gzip -9 "${MISC_DIR}/,emake_${BRAND}_anno.xml"
    fi

    if [ -f "${MISC_DIR}/emake.dlog" -a -f "${SUCCESSLOG}" ]; then
       rm -f ${MISC_DIR}/emake.dlog
    elif [ -f "${MISC_DIR}/emake.dlog" -a -f "${ERRORLOG}" ]; then
       gzip -9 ${MISC_DIR}/emake.dlog
    fi


    ## Preserve log files indefinitely
    logsdir="${BUILD_BASE}/LOGS";
    if [ "${LSFUSER}" == "hwnbuild" -a -d "${logsdir}" ]; then
       logsdir="$logsdir/${BUILD_DIR#$BUILD_BASE}"
       echo "Copying build logs to $logsdir"

       # create the log directory
       mkdir -pv $logsdir

       # copy the logs
       cp -p _* $logsdir 2>$NULL
       cp -p profile.log $logsdir 2>$NULL
       cp -p cfiles $logsdir 2>$NULL
       cp -p hfiles $logsdir 2>$NULL
       cp -p *.mk $logsdir 2>$NULL
       cp -p mogrify $logsdir 2>$NULL
       cp -p list $logsdir 2>$NULL
       cp -p dongle_image_temp_build_spawned $logsdir 2>$NULL

       echo "BUILD_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

       # copy release log after build complete message is written to it
       cp -p ,* $logsdir 2>$NULL
    else
       echo "BUILD_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}
    fi

    # Once the build is completed, transfer the built bits to network path
    if [ -n "$LOCAL_BUILD" ]; then
       echo "COPYING_STARTED = $(date '+%Y-%m-%d %H:%M:%S')"       >> ${RLSLOG}
       touch $BUILD_DIR/$WARN_FILE
       touch $LOCAL_BASE/$WARN_FILE
       # Build success or failure markers are skipped from copy until very last
       (tar cpf - -C $LOCAL_BASE . --exclude=*/${SUCCESSLOG} --exclude=*/${ERRORLOG} | tar xpf - -C $BUILD_DIR) >> ${RLSLOG} 2>&1
       copyec=$?
       [ -f "$LOCAL_BASE/$SUCCESSLOG" ] && cp -pv $LOCAL_BASE/${SUCCESSLOG} $BUILD_DIR/
       [ -f "$LOCAL_BASE/$ERRORLOG" ] && cp -pv $LOCAL_BASE/${ERRORLOG} $BUILD_DIR/
       if [ "$copyec" == "0" ]; then
          echo "INFO: Copy succeeded with exit code: $copyec"
          echo "COPYING_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}
          rm $BUILD_DIR/$WARN_FILE
          rm $LOCAL_BASE/$WARN_FILE
       else
          echo "ERROR: Copy failed with exit code: $copyec"
          echo "COPYING_FAILED = $(date '+%Y-%m-%d %H:%M:%S')"   >> ${RLSLOG}
          echo "INFO: Wait for 60 seconds and retry"
          sleep 60
          (tar cpf - -C $LOCAL_BASE . --exclude=*/${SUCCESSLOG} --exclude=*/${ERRORLOG} | tar xpf - -C $BUILD_DIR) >> ${RLSLOG} 2>&1
          copyec=$?
          [ -f "$LOCAL_BASE/$SUCCESSLOG" ] && cp -pv $LOCAL_BASE/${SUCCESSLOG} $BUILD_DIR/
          [ -f "$LOCAL_BASE/$ERRORLOG" ] && cp -pv $LOCAL_BASE/${ERRORLOG} $BUILD_DIR/
          if [ "$copyec" == "0" ]; then
             echo "INFO: Copy succeeded with exit code: $copyec"
             echo "COPYING_RETRY_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}
             rm $BUILD_DIR/$WARN_FILE
             rm $LOCAL_BASE/$WARN_FILE
          else
             echo "ERROR: Copy failed again with exit code: $copyec"
             echo "COPYING_RETRY_FAILED = $(date '+%Y-%m-%d %H:%M:%S')"   >> ${RLSLOG}
             rm -f $BUILD_DIR/${SUCCESSLOG}
          fi
       fi
    fi # LOCAL_BUILD
done

echo "$(date '+%b %d %H:%M:%S') $(hostname) build_linux[pid=$$,lsf=${LSB_JOBID}]: brands=${BRANDS}, tag=${TAG:-NIGHTLY}, user=${LSFUSER}, override=${OVERRIDE}; VCTOOL=$VCTOOL CVSROOT=$CVSROOT" >> ${SESSIONLOG}
echo "SESSIONLOG_UPDATED = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

# This lsf job status may not be complete, as it tries to query itself before
# completing. But this is still recorded to debug any resource issues
bjobs -l $LSB_JOBID     > ${LSFJOBLOG} 2>&1
bjobs -l $LSB_BATCH_JID > ${LSFBJOBLOG} 2>&1

echo "LSF_JOB_INFO_RECORDED = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

# Cleanup local temporay space used on completion if build copying is succeeded
# If not alert build admins and build launcher
if [ -n "$LOCAL_BUILD" -a -d "$LOCAL_BASE" ]; then
       # Copy over final version of release.log back to network path
       cp -pv $LOCAL_BASE/$RLSLOG $BUILD_DIR
       rm -fv $BUILD_DIR/$WIP_FILE
       # We don't want to leave temporary build dirs
       # After a successful copy back to network
       if [ ! -f "$LOCAL_BASE/$WARN_FILE" ]; then
          echo "INFO: Removing temporary $LOCAL_BASE"
          rm -rf "$LOCAL_BASE"
          local_date_dir=$(dirname $LOCAL_BASE)
          local_random_dir=$(dirname $local_date_dir)
          rmdir $local_date_dir
          rmdir -v $local_random_dir
       else
          copyerr=$(mktemp ${TEMP}/copyerr.XXXX)
          (
             exec 3>> $copyerr
             exec 1>&3 2>&3
             echo "ERROR: Local dir to network path copy failed with exit code: $copyec"
             echo "Date   : `date`"
             echo "Host   : $HOSTNAME"
             echo "LSF ID : $LSB_BATCH_JID"
             echo ""
             echo "Please copy manually and *delete* local copy"
             echo ""
             echo "% rsh $HOSTNAME \\"
             echo "      cp -rp $LOCAL_BASE \\"
             echo "      $BUILD_DIR"
             echo ""
             echo "Local Disk Usage Info:"
             df -h $LOCAL_BASE
             echo ""
             echo "Destination Disk Usage Info:"
             df -h $BUILD_DIR
          )
          unix2dos $copyerr
          cat $copyerr | \
              mail -s "ERROR: Local Build copying FAILED for ${TAG:-TRUNK}:$BRAND" \
              $MAILTO $ADMINMAILTO
          rm -fv $copyerr
          exit 1
       fi
fi

# Final exit code
exit 0
