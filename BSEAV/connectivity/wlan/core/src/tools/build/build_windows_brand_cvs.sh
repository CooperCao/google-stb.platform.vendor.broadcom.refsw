#!/bin/bash
#
# build_windows.sh - Do a checkout/release for bcm43xx drivers & tools
#
# $Copyright (C) 2004 Broadcom Corporation
# 
# $Id$
#
# SVN: $HeadURL$
#

SCRIPT_VERSION='$Id$'
SCRIPT_NAME=$0
NULL=/dev/null
HOSTOS=$(uname -s)
SERVERNAME=$(hostname)
MISC_DIR=misc
RLSLOG=,release.log
ENVLOG=${MISC_DIR}/,env.log
MAILLOG=${MISC_DIR}/,mail.log
MOVELOG=${MISC_DIR}/,move.log
DONGLE_RLSLOG=,dongle_image_release.log
DONGLE_ERRORLOG=,dongle_image_errors.log
SIGNTS_ERRORLOG=Z:/projects/hnd_swbuild/build_admin/logs/signing/timestamp_errors.log
ERRORLOG=,build_errors.log
IGNORED_ERRORLOG=,build_errors_ignored.log
WARN_FILE=_WARNING_PARTIAL_CONTENTS_DO_NOT_USE
WARN_VCTOOL_FILE=_WARNING_WRONG_CHECKOUT_FOUND
IGNORE_FILE=_SOME_BUILD_ERRORS_IGNORED
SVNCMD="svn --non-interactive"
SVNTZ="`date '+%z'`"
export VCTOOL=svn

PRESERVE_LIST="Z:/home/hwnbuild/src/tools/release/preserve_built_objects.txt"
PRESERVE_CTIME=120
PRESERVE_LOGS="$RLSLOG"
PRESERVE_LOGS="$PRESERVE_LOGS $ERRORLOG"
PRESERVE_LOGS="$PRESERVE_LOGS $IGNORE_ERRORLOG"
PRESERVE_LOGS="$PRESERVE_LOGS ,succeeded"
PRESERVE_LOGS="$PRESERVE_LOGS $ENVLOG"
PRESERVE_LOGS="$PRESERVE_LOGS $DONGLE_RLSLOG"
PRESERVE_LOGS="$PRESERVE_LOGS $DONGLE_ERRORLOG"

SVN_URL="http://svn.sj.broadcom.com/svn/wlansvn"
WEB_URL="http://home.sj.broadcom.com"
EA_URL="http://wc-sjca-e001.sj.broadcom.com/accelerator/builds.php"

# Given a util needed by build script, search it in gallery
# followed by build user's staging area and then in build script launch dir
# Most of the cases, the first hit is successful
function search_util {
	util_to_find=$1
	path_preference=$2
	util_found_status=1
	util_path_found=""

	net_path1=Z:/projects/hnd/software/gallery/src/tools/build
	net_path2=Z:/home/hwnbuild/src/tools/build
	net_path3=$(dirname $SCRIPT_NAME)

	loc_path1=C:/tools/build

	if echo "$path_preference" | grep -qi "local"; then
	   search_paths="$loc_path1 $net_path1 $net_path2 $net_path3"
	else
	   search_paths="$net_path1 $net_path2 $loc_path1 $net_path3"
	fi

	for tool_path in $search_paths
	do
		if [ -s "${tool_path}/${util_to_find}" ]; then
			util_path_found="${tool_path}/${util_to_find}"
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
		echo "ERROR: or $loc_path1"
		exit 1
	fi
	return $util_found_status
}

# Search utils used by this script in multiple network path heirarchy
# This takes care of stale NFS issues, if any that used to cause
# build scripts to fail.
# TO-DO: Move this to a common script template for all
# TO-DO: build scripts to use or source from

search_util "find_build_errors"
FIND_BUILD_ERRORS=$util_path_found
echo "FIND_BUILD_ERRORS=$FIND_BUILD_ERRORS"

search_util "build_config.sh"
BUILD_CONFIG=$util_path_found
echo "BUILD_CONFIG=$BUILD_CONFIG"

search_util "show_old_builditers.pl"
SHOW_OLD_BUILDITERS=$util_path_found
SHOW_OLD_BUILDITERS="perl $SHOW_OLD_BUILDITERS"

search_util "ecFindLastAgent.tcl"
FIND_ECAGENT=$util_path_found
FIND_ECAGENT="tclsh84 $FIND_ECAGENT"

search_util "hndmake.sh" "local"
HNDMAKE=$util_path_found
echo "HNDMAKE=$HNDMAKE"

function usage {
cat <<EOF

    Usage: $0 -b brand [other options]
    
    For more help, run '$0 -h'

EOF
    exit 1;
}

function help {
cat <<EOF

    Usage: $0 -b brand [other options as shown below]

    * You may want to refer to usage examples below

    -b brand  win_internal_wl or win_external_wl etc.(REQUIRED)
              The corresponding brand.mk must exist at src/tools/release 
              directory. ONLY one brand can be built at a time
    
    -c cutoff_time|svnrev
              This switch takes either cutoff time or svn revision number.
               * Cvs cutoff time (yyyy-mm-dd hh:mm format or other cvs/svn formats)
                 for checking out from tot/tob (default: none)
                 For SVN current timezone is appended by default during checkout
                 ('nightly' refers to midnight and 'now' or 'fix refer current time)
               * SVN revision can be r1234 or just 1234 format

    -d base_dir 
              The base directory where the source will be extracted to and 
              the release will be built. The default base_dir is local
              ${BUILD_DRV}:/build. [optional]

    -f        Gnu make flags
              Use this flag to build with HNDCVS using CVS repository

    -g        Force use gmake
              Default is gmake on non-emake nodes [optional]
              
    -h        Show this help text [optional]

    -j
              Use emake (parallel make) [default]

    -l        For non-gmake parallel build, provide a build label
              Default build-label is automatically constructed if this option
              is not provided [optional]

    -m email_address or "none"
              Send out status mail to email_address.
              If the -m option is not provided, defaults to hnd-build-list
              Specifying "none" as the email address suppresses the email

    -n        Skip copying to server folder [optional]

    -p private_release_makefile
              Provide path to custom or private release.mk file. This is
              copied as release.mk instead of cvs checkout [optional]

    -r tag    Use this option if you want to build a particular CVS tag
              e.g. "-r SPURS_REL_3_60_RC6". [optional]

    -s server_dir 
              The server directory where all the build files will be copied to.
              The default server_dir is Z:/projects/hnd_swbuild/build_window for
              tagged builds and ${BUILD_DRV}:/build_window for others

    -x builds_to_keep
              Remove old builds (default: keep last ${BUILDS_TO_KEEP_TOB} TOB builds; keep last ${BUILDS_TO_KEEP_TOT} TOT builds)
              [optional]
    
Usage Examples:
---------------
NOTE:It is preferable to use build_windows.bat script instead using this
     script directly. Build_windows.bat extends this scripts functionality
     by allowing multiple brand build and detailed logging and scheduling.

  1. Build OR respin win_external_wl for MY_BRANCH_3_120 tag and notify 
     prakashd only instead of hnd-build-list:
     build_windows_brand.sh -r MY_BRANCH_3_120 -b win_external_wl -m prakashd

  2. Build or respin win_external_wl TOT nightly build brand:
     build_windows_brand.sh -b win_external_wl

  3. Respin TOT nightly win_external_wl with cutoff time:
     build_window_brand.sh -c "2007-05-30" -b win_external_wl

  4. Respin TOT nightly win_external_wl with specific SVN revision
     build_window_brand.sh -c "r23456" -b win_external_wl

  5. Respin TOT nightly win_external_wl but not copy to server:
     build_window_brand.sh -n -b win_external_wl -m prakashd

  6. Build with CVS/HNDCVS instead of SVN repository
     build_window_brand.sh -b win_external_wl -f "CVSROOT=/projects/cvsroot"

  7. Build with a custom or private SVN URL, other than official SVN URL
     build_window_brand.sh -b win_external_wl -f "REPO_URL=${SVN_URL}/users/prakashd/xxx"
EOF
    exit 1;
}

# Given a dirname, compute the date-wise next iteration number for $base
function uniquedir () {
    local base="$1";
    local lastiter="$2";
    local -i i=0
    local -i j=0
    local iter=""

    # lastiter is optional argument this uniquedir()
    # if it is present, it is used to set starting iter num
    if [ -n "$lastiter" ]; then
       iter=$(basename $2) 
       iter=$(echo $iter | cut -d. -f4)
       j=$(($iter+1))
    fi

    # The next iteration number should be higher than any existing iteration number
    for i in $(seq $j 20); do
        if [ -d ${base}${i} ]; then
           j=$i
        fi
    done

    # So start with latest existing iteration number
    while [ -d ${base}${j} ]
      do
      if (( $j < 20 ))
      then
        let j=j+1;
      else 
        echo "${FUNCNAME} Giving up: cannot create unique directory in ${base}"
        return 1;
      fi;
    done
    echo ${base}${j};
    return 0;
}

function copy_to_server {

    if [ -f "${MISC_DIR}/serverdir.txt" ]; then
       echo "INFO: Server copy has already been completed" >> ${MOVELOG}
       echo "INFO: at $(cat ${MISC_DIR}/serverdir.txt)"    >> ${MOVELOG}
       return 0	
    fi

    (
      # Redirect stdout and stderr to ${MOVELOG}
      exec 3>> ${MOVELOG}
      exec 1>&3 2>&3

      local copysrc;

      # Copying the build to SERVER_BASE
      echo "mkdir -p ${serverdir}"
      mkdir -pv ${serverdir}

      copysrc=${BUILD_BASE}/${TAGDIR}/${BRAND_DIR}
      du=`df -k $copysrc | grep -v "Filesystem" | awk '{print $5}'`  
      du=`echo $du | sed -e 's/%//g'`
      echo "START:   copy build to server, `date '+%m/%d/%y %H:%M:%S'`"
      echo "xcopy /E /H /I /Q /Y ${copysrc//\//\\} ${serverdir//\//\\}"
      xcopy /E /H /I /Q /Y ${copysrc//\//\\} ${serverdir//\//\\}
      XCOPYRC=$?
      echo "xcopy finished (xcopyrc ${XCOPYRC})"
      echo "Local $copysrc disk-usage: ${du}%"
      ls ${serverdir}
      echo "END:   copy build to server, `date '+%m/%d/%y %H:%M:%S'`"
     
      if [ "${XCOPYRC}" != "0" -o ! -d "${serverdir}" ]; then
         echo "" >> ${RLSLOG}
         echo "ERROR: XCOPY from ${copysrc} to ${serverdir} failed" >> \
              ${RLSLOG}
         echo "ERROR: XCOPY exit code: $XCOPYRC" >> \
              ${RLSLOG}
         echo "INFO: Refer to ${copysrc}/${MOVELOG} on `hostname`" >> \
		${RLSLOG}
         echo "" >> ${RLSLOG}
	 touch $copysrc/_WARNING_PARTIAL_CONTENTS_DO_NOT_USE
	 rm -fv $copysrc/,succeeded
	 rm -fv $serverdir/,succeeded
         echo "ERROR: XCOPY from ${copysrc} to ${serverdir} failed" >> \
		$copysrc/$ERRORLOG
         echo "ERROR: XCOPY exit code: $XCOPYRC" >> \
		$copysrc/$ERRORLOG
         echo "INFO: Refer to ${copysrc}/${MOVELOG} on `hostname`" >> \
		$copysrc/$ERRORLOG
         echo "" >> $copysrc/$ERRORLOG
	 cp -p $copysrc/$ERRORLOG $serverdir/$ERRORLOG
      fi

      if echo ${RELDIR} | grep -i -q "_BRANCH_\|_TWIG_\|NIGHTLY"; then
         if [  -d "${SERVER_BASE}/PRESERVED" -a -f "${PRESERVE_LIST}" ]; then
           preservedir="${SERVER_BASE}/PRESERVED"
           preservedir="${preservedir}/${RELDIR}"
           preserved_brand_flag=0
           PRESERVELOG=${preservedir}/,preserve.log

           # Remove old expired preserved build items
           if [ -d "${preservedir}/${BRAND}" ]; then
              find ${preservedir}/${BRAND} -maxdepth 1 -mindepth 1 -not -mtime -${PRESERVE_CTIME} | xargs -t -l1 -r rm -rf
           fi

           iteration=$(basename ${serverdir})
           preservedir="${preservedir}/${BRAND}/${iteration}"
           for keep in $(cat ${PRESERVE_LIST} | egrep -v "^#" | sed -e "s/[[:space:]]//g" | egrep "build_window:${BRAND}:")
           do
               preserved_brand_flag=1
               mkdir -pv ${preservedir}
               ## PRESERVERLIST format build_window : BRAND : file_to_preserve
               file=$(echo $keep | awk -F: '{print $3}') 
               echo "copy $file => ${preservedir}/$file" >> ${PRESERVELOG}
               (tar cf - $file | ( cd ${preservedir}; tar xf - )) >> ${PRESERVELOG} 2>&1
           done
           if [ "$preserved_brand_flag" != "0" ]; then
              for logf in $PRESERVE_LOGS; do
                  if [ -f "$logf" ]; then
                     cp -v $logf $preservedir
                  fi
              done
              gzip -f -r -q -9 $preservedir
              echo "==================================================" >> ${PRESERVELOG}
           fi
         fi
      fi

      # Reset the permissions to avoid accidental writes
      # For performance reasons this is spawned as a separate process
      echo "Resetting permissions on ${serverdir} to make it read-only"
      tempscript=`mktemp C:/temp/temp_${RELDIR}.XXXXXXXX`
      echo "Running $tempscript (chmod -R -f go-w $serverdir)"
      if [ -f "$tempscript" ]; then
           mv $tempscript ${tempscript}.bat
           tempscript="${tempscript}.bat"
           echo "@echo off" > $tempscript
           echo "@echo Running chmod on $RELDIR on `date`" >> $tempscript
           echo "START \"chmod -R -f go-w $serverdir\" /min /low chmod -R -f go-w $serverdir" >> $tempscript
           cmd /c "${tempscript//\//\\}"
      fi

    ## Preserve log files indefinitely
    logsdir="$SERVER_BASE/LOGS"
    if [ -d "${logsdir}" ]; then
       logsdir="$logsdir/${serverdir#$SERVER_BASE}"
       echo "Copying build logs to $logsdir"

       # create the log directory
       mkdir -pv $logsdir/${MISC_DIR}

       # copy the logs
       cp -p ,* $logsdir 2>${NULL}
       cp -p _* $logsdir 2>${NULL}
       cp -p profile.log $logsdir 2>${NULL}
       cp -p cfiles $logsdir 2>${NULL}
       cp -p hfiles $logsdir 2>${NULL}
       cp -p *.mk $logsdir 2>${NULL}
       cp -p mogrify $logsdir 2>${NULL}
       cp -p list $logsdir 2>${NULL}
       cp -p dongle_image_temp_build_spawned $logsdir 2>${NULL}
       cp -p ${MISC_DIR}/,*.log $logsdir/${MISC_DIR} 2>${NULL}

    fi

    #gzip -f -9 ${serverdir}/${MOVELOG}
    if [ "${EMAKE}" != "" ]; then
         gzip -f -9 ${serverdir}/${MISC_DIR}/*.xml
         if [ -f "${serverdir}/${MISC_DIR}/,emake-${BRAND}_debug.log" ]; then
         	gzip -f -9 ${serverdir}/${MISC_DIR}/*_debug.log
         fi
    fi

    ## Since all the stdout is directed to ${MOVELOG}, xcopy it over again
    rm -f ${serverdir}/${MOVELOG}
    echo "copy again ${copysrc}/${MOVELOG} => ${serverdir}/${MOVELOG}"
    cp ${copysrc}/${MOVELOG} ${serverdir}/${MOVELOG}
    echo ${serverdir} > ${MISC_DIR}/serverdir.txt

    # Record serverdir that was picked in this brand build
    echo ${serverdir} >> ${RLSLOG}

    ) # copy_to_server

} # copy_to_server

function show_build_errors {
   local options="$1";
   local rlog="$2";

   # Show errors found in build log by removing all the full path clutter
   ${FIND_BUILD_ERRORS} $options $rlog | \
        perl -ne "s/$BDIR//gi; s/$BDIR_WIN//gi; print" | \
        perl -ne 's/^(\d+)[:-]/[$1] /g; print';
}

function remove_old_builds {

   # Redirect stdout and stderr to ${MOVELOG}
   echo "Redirecting output messages to ${MOVELOG}"
   (
     exec 3>> ${MOVELOG}
     exec 1>&3 2>&3

     # Default build keep counts are  $BUILDS_TO_KEEP_TOT or BUILDS_TO_KEEP_TOB
     # If current build fails, then keep count is extended automatically below
     if [ "${BUILDS_TO_KEEP}" != "" ] ; then
       echo "Old Builds To delete:"
       if [ ! -f ",succeeded" ]; then
          BUILDS_TO_KEEP=$(expr ${BUILDS_TO_KEEP} + $(expr ${BUILDS_TO_KEEP} / 2))
          echo "Build preserve time (extended): ${BUILDS_TO_KEEP}"
       else
          echo "Build preserve time : ${BUILDS_TO_KEEP}"
       fi

       oldbuilds="$(${SHOW_OLD_BUILDITERS} -keep_count ${BUILDS_TO_KEEP} -brand_dir ${SERVER_BASE}/${RELDIR}/${BRAND})"
       for bld in ${oldbuilds}; do
           if echo $bld | egrep -q -i -v "/${TAG:-NIGHTLY}/${BRAND}/"; then
              echo "WARN: $bld not a build"
              continue;
           fi
           echo "`date`: Delete old iteration $bld"
           blditer=`basename $bld | sed 's/\./_/g'`
           delbat=${MISC_DIR}/delete_${blditer}.bat
           echo "@echo off"                              > ${delbat}
           echo "echo %date% %time% Start Delete: $bld" >> ${delbat}
           echo "title START RMDIR /S /Q ${bld//\//\\}" >> ${delbat}
           echo "RMDIR /S /Q ${bld//\//\\}"             >> ${delbat}
           echo "title END   RMDIR /S /Q ${bld//\//\\}" >> ${delbat}
           echo "echo %date% %time% End   Delete: $bld" >> ${delbat}
           echo "sleep 2"                               >> ${delbat}
           echo "exit"                                  >> ${delbat}
           cmd /c "START /MIN ${delbat//\//\\}"
       done
       echo "`date`: Done with old build iteration cleanup"
     fi

     # remove old failed iterations (from today) from ${SERVER_BASE}/$RELDIR/$BRAND/
     # Format version string like epivers.sh
     yyyy=$(date '+%Y')
     mm=$(date '+%m')
     dd=$(date '+%d')
     m=${mm/#0/}
     d=${dd/#0/}
     iteration=0
     for j in $(seq 0 20) ; do
         BUILD_DCHECK="${SERVER_BASE}/$RELDIR/$BRAND/${yyyy}.${m}.${d}.${j}"
         errorlog="${BUILD_DCHECK}/${ERRORLOG}"
         # delete previous failed build
         if [ -f "${errorlog}" ]
         then
            echo "================================================="
            echo "* Removing failed ${BUILD_DCHECK}:"
            echo "* '${yyyy}.${m}.${d}.${j}' build history:"
            stat ${errorlog} | grep "Access:\|Modify:\|Change:"
            echo "* Errors Found: "
            cat ${errorlog}
            rm -rf "${BUILD_DCHECK}"
         fi

         [ "$j" == "20" ] && break
     done
     echo "================================================="
   )
   # Done removing old builds
   echo "Done removing old or failed builds. Check ${MOVELOG} for details"
}

# TEMP on some systems has white-space in path and its contents change
# from machine to machine causing occasional hangs in temporary scripts
# TEMP. So set it to c:/temp
export TEMP="C:/temp"
[ -d "$TEMP" ] || mkdir -pv $TEMP

MAILSTATUS=true
MAILTO="hnd-build-list@broadcom.com";
ADMINMAILTO="hnd-software-scm-list@broadcom.com";
NULL=/dev/null
TAG=
BRAND=
BUILDS_TO_KEEP=
# default old build keep count for tot and tob builds if builds don't fail
BUILDS_TO_KEEP_TOB=2
BUILDS_TO_KEEP_TOT=7
SERVER_BASE_WINDOW=Z:/projects/hnd_swbuild/build_window
# On windows we can't find target location of a symlink'd dir easily
# hence use absolute path below
SERVER_BASE_SCRATCH=/projects/hnd_swbuild_ext7_scratch/build_window_ext7_scratch
## For tagged and nightly builds, server_base is network drive by default
: ${SERVER_BASE:="$SERVER_BASE_WINDOW"}

SERVER_BASE_WINDOW_MOUNT=Z:/projects/hnd/swbuild/build_window

# If the SVN repository path pointed to 'users' area, the builds
# are redirected to following path
SERVER_BASE_WINDOW_USERS=Z:/projects/hnd_swbuild/build_window/USERS
SERVER_BASE_WINDOW_USERS_RE=Z:/projects.hnd.swbuild.build_window.USERS

# Custom firmware build dirs for GCLIENT transition
BUILD_BASE_LINUX_GCLIENT=/projects/hnd_swbuild/build_linux/GCLIENT_TRANSITION/

# Force microsoft wdk tools not to launch oacr monitor after ndis driver build
# It is not needed on servers. oacr is microsoft code review tool
export WDK_OACR=no_oacr

if [ "${RSHHOST}" == "" ]; then RSHHOST=xlinux.sj.broadcom.com; fi
RSHCMD="rsh -l hwnbuild ${RSHHOST}"

## Determine the local build drive, default is C drive
if   [ "${COMPUTERNAME}" == "NT-SJCA-0514" ]; then
     BUILD_DRV=E
elif [ "${COMPUTERNAME}" == "NT-SJCA-0516" ]; then
     BUILD_DRV=C
elif [ "${COMPUTERNAME}" == "NT-SJCA-0517" ]; then
     BUILD_DRV=C
else
     BUILD_DRV=C
fi

PATH=/cygdrive/${BUILD_DRV}/tools/Python:/cygdrive/${BUILD_DRV}/tools/Subversion/cygdrive/z/projects/hnd/tools/win/Python:/cygdrive/z/projects/hnd/tools/win/Subversion:/usr/bin:/bin:$(cygpath -u ${HNDEMAKE_DIR}/i686_win32/bin):/cygdrive/${BUILD_DRV}/tools/build:${PATH}

# Command line switches this script takes
# WARN: Ensure that cmd line option blocks below are arranged alphabetically.
while getopts 'b:c:d:e:f:ghj:kl:m:nop:r:s:vx:' OPT 
    do
        case "$OPT" in
        b)
            BRAND=$OPTARG;
            ;;        
        c)
	    # Cutoff time or svn revision to use during checkout
	    # CVSCUTOFF is exported into the caller's environment
	    export CVSCUTOFF="${OPTARG}"
	    # SVNCUTOFF is exported into the caller's environment
	    export SVNCUTOFF="${OPTARG}"
            ;;
        d)
            BUILD_BASE=$OPTARG;
            ;;
        e)
            EXTRA_LOG_STATUS=$OPTARG;
            ;;
        f)
            GMAKEFLAGS="$OPTARG";
            ;;
        g)
            GMAKE=1
            ;;
        h)
            help;
            ;;
        j)
            EMAKE=1
            ;;
        k)
            # keep going
            FORCE=1
            ;;
        l)
            BUILD_LABEL=$OPTARG;
            ;;
        m)
            if [ "$OPTARG" == "none" ]; then
               MAILSTATUS=;
            else
               MAILTO=$OPTARG;
               export MAILTO;
            fi
            ;;
        n)
            export SKIP_SERVER_COPY=1
            ;;
        p)
            export PVT_MAKEFILE=$OPTARG;
            ;;
        r)
            TAG="$OPTARG";
            ;;
        s)
            SERVER_BASE=$OPTARG;
            if echo $SERVER_BASE | grep -q -i /projects/hnd/swbuild; then
               SERVER_BASE=$(echo $SERVER_BASE | sed -e 's/hnd\/swbuild/hnd_swbuild/')
            fi
            ;;
        v)
            export VERBOSE=1;
            ;;
        x)  
            BUILDS_TO_KEEP=$OPTARG;
            ;; 
        :)
            echo "Missing required argument for option \"-$OPTARG\".\n";
            usage;
            ;;
        ?)
            makeargs="$makeargs -$OPTARG"
            echo "Unrecognized option - \"$OPTARG\".\n";
            echo "Passing \"$OPTARG\" option to GNU make.\n";
            ;;
        esac
done

CVSROOT_PROD=":pserver:hwnbuild@cvsps-sj1-1.sj.broadcom.com:/projects/cvsroot"

if [ -z "$CVSROOT" ]; then
	export CVSROOT=${CVSROOT:-${CVSROOT_PROD}}
fi

# Default SVN Repo root, if not set in environment
if [ -z "$SVNROOT" ]; then
        # WLAN Production Repository
        export SVNROOT="${SVN_URL}/proj"
        # NOTE: WLAN Production Repository - but accessed from alternate server
        # NOTE: Following is not used currently, is an alternate to production

        # Sample out new SVN server, pointing to production repo to load balance
        # WARN: New server needs svn credentials cached across all systems
        # WARN: Talk to SVN Admins before using this server
        # WARN: Using this server needs credentials to be cached
        #if [[ "$BRAND" == *mfgtest* ]] && [ "$TAG" == "" ]; then
        #   export SVNROOT="http://engcm-sj1-06.sj.broadcom.com/svn/wlansvn/proj"
        #fi
fi
export SVNBASE=$(echo $SVNROOT | sed -e 's%/proj%%')

case "$GMAKEFLAGS" in
     *VCTOOL=cvs*|*CVSROOT=*)
           # If custom CVSROOT is supplied, use it override default setting
           # If CVSROOT is set to dynamically changing CVS_SNAPSHOT, let it 
           # be inherited from environment
           export VCTOOL=cvs
           FIND_BUILD_ERRORS="$FIND_BUILD_ERRORS -cvs"

           export CVSROOT=$(echo "$GMAKEFLAGS" | fmt -1 | grep CVSROOT | awk -F= '{print $2}')

           if echo $CVSROOT | grep -qi -v ":pserver:"; then
              CVSROOT_OLD=$CVSROOT
              export CVSROOT=":pserver:hwnbuild@cvsps-sj1-1.sj.broadcom.com:$CVSROOT"
              GMAKEFLAGS=$(echo "${GMAKEFLAGS}" | sed -e "s%CVSROOT=$CVSROOT_OLD%CVSROOT=$CVSROOT%g")
           fi
           bldlabel_suffix="CVS"
           ;;

     *VCTOOL=svn*)
           export VCTOOL=svn
           ;;

     *COTOOL=gclient*)
           # If GMAKEFLAGS contains COTOOL=gclient, export it to env
           # to pass down to $(MAKE)
           export COTOOL=gclient
           bldlabel_suffix="GCLIENT"
           # If build submitted for GCLIENT, forcefully redirect it to
           # GCLIENT_TRANSITION folder
           if [ -n "$SERVER_BASE" ]; then
             if echo $SERVER_BASE | egrep -qv "GCLIENT_TRANSITION"; then
                SERVER_BASE="${SERVER_BASE}/GCLIENT_TRANSITION"
                echo "INFO: GCLIENT server build copy redirected to $SERVER_BASE"
             fi
          fi
          # BUILD_LINUX_DIR is exported by DHD build into Firmware build
          # process. Separate out GCLIENT fw builds from production SVN fw
          # builds
          export BUILD_LINUX_DIR=$BUILD_BASE_LINUX_GCLIENT
          ;;

     *REPO_URL=*)
           # Extract repository URL if specified on command line flag
           export REPO_URL=$(echo "$GMAKEFLAGS" | fmt -1 | awk -F= '/REPO_URL/{print $2}')
           echo "INFO: Setting custom SVN Repository URL to $REPO_URL"
           ;;

     *DEPS_URL=*)
           # Extract repository URL if specified on command line flag
           export DEPS_URL=$(echo "$GMAKEFLAGS" | fmt -1 | awk -F= '/DEPS_URL/{print $2}')
           echo "INFO: Setting custom GCLIENT Deps URL to $DEPS_URL"
           ;;
esac # GMAKEFLAGS

# For special cvs and svn builds, suffix bldlabel_suffix
# to easily identify them from rest of production builds
if [ -n "$bldlabel_suffix" ]; then
   export BUILD_LABEL="$BUILD_LABEL$bldlabel_suffix"
fi

# Centralized SVN checkout rules and definitions
CHECKOUT_DEFS_CVS="checkout-defs-cvs.mk"
CHECKOUT_RULES_CVS="checkout-rules-cvs.mk"
CHECKOUT_DEFS="checkout-defs.mk"
CHECKOUT_RULES="checkout-rules.mk"

# Default repo url is trunk/tot if not set by user
if [ ! -n "$REPO_URL" ]; then
   export REPO_URL="${SVNROOT}/trunk"
   export REPO_URL_TRUNK="${SVNROOT}/trunk"

   case "${TAG}" in
     *_BRANCH_*|*_TWIG_*)
           export REPO_URL="${SVNROOT}/branches/${TAG}"
           ;;
     *_REL_*)
           export TAG_PREFIX=$(echo $TAG | awk -F_ '{print $1}')
           export REPO_URL="${SVNROOT}/tags/${TAG_PREFIX}/${TAG}"
           ;;
   esac
fi

# Validate if a given tag or branch exists or not in SVN
# No point continuing, if tag or branch is invalid
if [ "$VCTOOL" == "svn" ]; then
   foundtaginfo=$($SVNCMD ls $REPO_URL 2>&1)
   foundtagstatus=$?

   if [ "$foundtagstatus" != "0" ]; then
      echo "ERROR: $foundtaginfo"
      exit 1
   fi
fi

# Redirect user's private builds away from production dirs
# First check supplied SERVER_BASE matches production SERVER_BASE dirs
# irrespective of drive letter prefix
if [ "${SERVER_BASE/*:/}" == "${SERVER_BASE_WINDOW/*:/}" -o "${SERVER_BASE/*:/}" == "${SERVER_BASE_WINDOW_MOUNT/*:/}" ]; then
   # User's full builds are redirected to a separate scratch disk
   # they are identified by users svn area name
   if echo "${REPO_URL}" | grep -q "wlansvn/users"; then
      # Convert wlansvn/users/xxx/yyy/zzz folder to xxx_yyy_zzz format
      export BUILD_USERDIR=$(echo $REPO_URL | sed -e "s%${SVNBASE}/%%" -e "s%^users/%%" -e "s%/%_%g")
      export SERVER_BASE=$SERVER_BASE_WINDOW_USERS/$BUILD_USERDIR

      if ! mkdir -p "${SERVER_BASE}" ; then
         echo "ERROR: $(hostname): Creation of ${SERVER_BASE} failed"
         exit 1
      else
         chmod ug+w $SERVER_BASE
      fi
      echo "INFO: Overriding SERVER_BASE to $SERVER_BASE"
   fi # REPO_URL=wlansvn/users
fi # SERVER_BASE

# Default make is emake on emake cluster build servers
if [[ "$COMPUTERNAME" == *WC-SJCA-E* ]]; then
   if [ "${GMAKE}" != "" ]; then
      MAKE=gmake
   else
      EMAKE=1
      MAKE=$HNDMAKE
   fi
   if [ -f "${SYSTEMDRIVE}/tools/build/set_buildenv.sh" ]; then
      . ${SYSTEMDRIVE}/tools/build/set_buildenv.sh
   fi
else
   MAKE=gmake
   if [ "$FORCE" == "" ]; then
       echo "WARN:"
       echo "WARN: '$COMPUTERNAME' is an old/slower standalone build server"
       echo "WARN: Please use one of the following servers"
       echo "WARN: WC-SJCA-EB02/EB03/EB04 or WC-SJCA-EB12/EB13/EB14"
       echo "WARN: to launch your windows builds"
       echo ""
       echo -n "* WARN: Do you want to continue? [Yes|No|Quit]: "
       read ans
       if echo ${ans} | grep -qi "q\|n"; then echo "Exiting"; exit 0; fi
   else
       echo ""
       echo "Force build enabled on $COMPUTERNAME"
       echo ""
   fi
fi

# BRAND is required
if [ "${BRAND}" == "" ]; then
   usage
fi

if echo "${TAG}" | grep -q -i "_BRANCH_\|_TWIG_"; then
   if [ ! -n "${BUILDS_TO_KEEP}" ]; then
      BUILDS_TO_KEEP=${BUILDS_TO_KEEP_TOB}
      echo "INFO: Set default build keep count to ${BUILDS_TO_KEEP_TOB} builds"
   fi
fi

if [ "${TAG}" == "" -o "${TAG}" == "HEAD" ]; then
   if [ ! -n "${BUILDS_TO_KEEP}" ]; then
      BUILDS_TO_KEEP=${BUILDS_TO_KEEP_TOT}
      echo "INFO: Set default build keep count to ${BUILDS_TO_KEEP_TOT} builds"
   fi
fi

if [ "${CVSCUTOFF}" != "" -o "${CVSCUTOFF}" != "" ] && [ "${TAG}" != "" ]; then
   if echo "${TAG}" | grep -q -v "_BRANCH_\|_TWIG_\|HEAD"; then
      echo "ERROR: -cvscutoff and cvstag are mutually exclusive"
      usage
   fi
fi

if [ "${CVSCUTOFF}" == "nightly" ]; then
   CVSCUTOFF="`date '+%Y-%m-%d 00:00'`"
   SVNCUTOFF="`date '+%Y-%m-%d 00:00'`"
fi

if echo "${SVNCUTOFF}" | egrep -q '^[[:digit:]][[:digit:]]:[[:digit:]][[:digit:]]$' ; then
   SVNCUTOFF="`date "+%Y-%m-%d $SVNCUTOFF"`"
fi

if [ "${CVSCUTOFF}" == "now" -o "${CVSCUTOFF}" == "fix" ]; then
   CVSCUTOFF="`date '+%Y-%m-%d %H:%M:%S'`"
   SVNCUTOFF="`date '+%Y-%m-%d %H:%M:%S'`"
fi

# If -c specified an SVN revision, fetch it and reset cutoff to null
if echo "${SVNCUTOFF}" | egrep -q "^(r|)[[:digit:]]+$"; then
   SVNREV=$(echo $SVNCUTOFF | sed -e 's/^r//g')
   unset SVNCUTOFF
   unset CVSCUTOFF
fi

if [ -n "$PVT_MAKEFILE" ] && [ ! -s ${PVT_MAKEFILE} ]; then
   echo "ERROR: $SERVERNAME: Private makefile '${PVT_MAKEFILE}' not found or empty."
   exit 1
fi

# Any remaining arguments are passed to MAKE
shift $(($OPTIND-1))
makeargs="$makeargs $@"

## By default only one build server can write onto
## NIGHTLY folder. However for problem fixing and
## for partial builds, one can set RELDIR to match
## a TAG or appropriate nightly directory from anyother
## build server
if [ "$RELDIR" == "" ]; then
   ## NT-SJCA-0516 or NT-SJCA-0517 are the old build servers
   ## WC-SJCA-EB* are the nightly parallel build servers
   if [[ "$COMPUTERNAME" == *NT-SJCA-051* ]]; then
      RELDIR=${TAG:-"NIGHTLY"};
   elif [[ "$COMPUTERNAME" == *WC-SJCA-E* ]]; then
      RELDIR=${TAG:-"NIGHTLY"};
   else
      ## For all others, push their builds to OTHER_NIGHTLY inside SERVER_BASE
      RELDIR=${TAG:-"OTHER_NIGHTLY"};
   fi
fi

# Temporary workaround until vxworks makefiles are cleaned up to work
# with longer relative paths and bigger environment space by router team.
# Some microsoft and tornado are very sensitive to env and path length,
# hence trim the directory path. TAGDIR is shortened name for RELDIR
# when build is performed locally for certain types of vx-router builds
case "$RELDIR" in
     COMANCHE_*)
                 TAGDIR=$(echo $RELDIR | sed -e 's/COMANCHE_/C_/g')
                 ;;        
     COMANCHE2_*)
                 TAGDIR=$(echo $RELDIR | sed -e 's/COMANCHE2_/C2_/g')
                 ;;        
              *)
                 TAGDIR=$RELDIR
                 ;;        
esac

mkdir -p "${SYSTEMDRIVE}/build_logs/${RELDIR}"

echo -e "\n======= START LOG `date '+%Y-%m-%d %H:%M:%S'` =========\n"

# Redirect top of branch builds rooted at SERVER_BASE_WINDOW to scratch disk
if echo "${TAG}" | grep -q -i "_BRANCH_\|_TWIG_"; then
   if [ "${SERVER_BASE/*:/}" == "${SERVER_BASE_WINDOW/*:/}" ]; then
      # Under windows cifs filesystem, unix symlinks appear as directories
      if [ ! -d "${SERVER_BASE}/${TAG}" ]; then
         echo "INFO: Redirecting top of branch builds to scratch area"
         mkdir -pv ${SERVER_BASE_SCRATCH}/${TAG}
         ${RSHCMD} ln -sv ${SERVER_BASE_SCRATCH/*:/}/${TAG} ${SERVER_BASE/*:/}/${TAG}
         sleep 5;
      fi
   fi
fi

# make the TAG build directory name shorter
if [ "${TAG}" == "" ]; then
  BRAND_DIR=${BRAND}_`date '+%y%m%d'`;
else
  BRAND_DIR=${BRAND};
fi
BRAND_DIR=${BRAND_DIR:-${TAG:-"BRAND_"`date '+%y%m%d'`}}

echo "BUILD_BASE     = $BUILD_BASE";
echo "SERVER_BASE    = $SERVER_BASE";
echo "BRAND_DIR      = $BRAND_DIR";
echo "BUILD_TIME     = `date`";
echo "BRAND          = $BRAND";
echo "TAG            = $TAG";
echo "TAGDIR         = $TAGDIR";
echo "RELDIR         = $RELDIR";
echo "BUILDS_TO_KEEP = $BUILDS_TO_KEEP";
echo "HOME           = $HOME";
echo "TEMP           = $TEMP";
echo "GMAKEFLAGS     = $GMAKEFLAGS";
echo "REQUESTER INFO = $EXTRA_LOG_STATUS"

: ${BUILD_BASE:="${BUILD_DRV}:/build"}

cd ${BUILD_BASE}
if [ $? != 0 ]; then
    echo "Cannot cd to ${BUILD_BASE}"
    exit 1
fi;

# Add a level of indirection (TAG or date) to the release
if [ ! -d ${TAGDIR} ]; then
   mkdir ${TAGDIR}
else
    echo "${TAGDIR} directory exists"
fi

echo "cd ${TAGDIR}"
cd ${TAGDIR}
if [ $? != 0 ]; then
    echo "Cannot cd to ${TAGDIR}"
    exit 1
fi

BRAND_DIR_ORIG=${BRAND_DIR}
let i=0
let j=0

echo "Trying to create ${BRAND_DIR}"

# If BRAND_DIR exists, cycle through available BRAND_DIR_<iter>
# New dir created is ALWAYS BRAND_DIR_<iter+1>

if [ -d "${BRAND_DIR}" ]; then
   for i in $(seq 0 20); do
       if [ -d ${BRAND_DIR}_${i} ]; then
          j=$(($i+1))
       fi
   done
fi

# Not j contains next iteration of the BRAND_DIR that needs to be created

let i=$j

while ! mkdir $BRAND_DIR > ${NULL} 2>&1
do
    echo "Cannot create new ${BRAND_DIR}"
    if (( $j < 20 ))
    then
       BRAND_DIR=${BRAND_DIR_ORIG}_${i}
       echo "Trying ${BRAND_DIR} instead"
       let i=i+1;
    else 
       echo "Giving up."
       exit 1;
    fi;
done

echo "cd $BRAND_DIR"
cd $BRAND_DIR
if [ $? != 0 ]; then
    echo Cannot cd to ${BRAND_DIR}
    exit 1
fi;

# Format version string like epivers.sh
date=$(date '+%Y.%-m.%-d.');
# Get an unique dir on server, before removing old builds and copying new ones
serverdir=$(uniquedir "${SERVER_BASE}/${RELDIR}/${BRAND}/${date}")
if [ "${SKIP_SERVER_COPY}" != "1" -a ! -d "${serverdir}" ]; then
	echo "INFO: [$(date '+%Y/%m/%d %H:%M:%S')] Pre-creating $serverdir"
	echo "INFO: [$(date '+%Y/%m/%d %H:%M:%S')] Pre-creating $serverdir" >> $RLSLOG
	if ! mkdir -pv "${serverdir}" ; then
		echo "ERROR: Failed to create $serverdir on $(hostname)" > $RLSLOG
   		blat ${RLSLOG} -mailfrom hwnbuild@broadcom.com \
			-t ${ADMINMAILTO} \
			-s "ERROR: Pre-creation FAILED on $(hostname) for $serverdir"
		# Letting the build continue to build locally in $BRAND_DIR after alerting
		# admins instead of exiting as subsequently there is one more serverdir
		# creation attempt after the longer local build is completed
	else
		echo "START_TIME = $(date '+%Y-%m-%d %H:%M:%S')" >  $serverdir/$RLSLOG
		echo "INFO: Build in progress on $(hostname)"    >> $serverdir/$RLSLOG
		echo "INFO: Lookup ${EA_URL} after few minutes"  >> $serverdir/$RLSLOG
	fi
fi

mkdir -pv ${MISC_DIR}

# Emake sometimes exits with zero error code, but emake crashes
EMAKEERRORS="Application encountered an unexpected error.*Stopping"

## Mount any missing drives. This ensures that build continues
## even when there is no hwnbuild console login on the server console
(
    # Redirect stdout and stderr to ${RLSLOG}
    exec 3>> ${RLSLOG}
    exec 1>&3 2>&3

    projdrive=Z
    if echo ${SERVER_BASE} | grep -q -i "${projdrive}:"; then
       if [ "${USERNAME}" == "hwnbuild" -a ! -d "${SERVER_BASE}" ]; then
          echo "Attempting to mount drive ${projdrive}: on $SERVERNAME"
          drvstat=$(net use 2>&1 | egrep -i "unavailable.*${projdrive}:")
          if [ "$drvstat" != "" ]; then
             echo "Deleting bad drive mount ${projdrive}: first"
             echo "net use ${projdrive}: /d"
             echo Y | net use ${projdrive}: /d
          fi
          echo "net use ${projdrive}: \\\\brcm-sj\\dfs /persistent:yes"
          echo Y | net use ${projdrive}: \\\\brcm-sj\\dfs /persistent:yes
          net use ${projdrive}:
       fi
    fi


    # MYHOMEDRIVE=$(cygpath -m ${HOME} | awk -F: '{print $1}')
    myhomedrive=P
    if [ "${USERNAME}" == "hwnbuild" -a ! -f "${myhomedrive}:/.bashrc" ]; then
       echo "Attempting to mount drive ${myhomedrive}: on $SERVERNAME"
       drvstat=$(net use 2>&1 | egrep -i "unavailable.*${myhomedrive}:")
       if [ "$drvstat" != "" ]; then
          echo "Deleting bad drive mount ${myhomedrive}: first"
          echo "net use ${myhomedrive}: /d"
          echo Y | net use ${myhomedrive}: /d
       fi
       echo "net use ${myhomedrive}: \\\\brcm-sj\\dfs\\home\\${USERNAME} /persistent:yes"
       echo Y | net use ${myhomedrive}: \\\\brcm-sj\\dfs\\home\\${USERNAME} /persistent:yes
       net use ${myhomedrive}:
    fi
)

if [ -z "$NTICE" ]; then
   export NTICE="z:/projects/hnd_tools/win/numega/SoftICE/"
fi;

# Build Start Time
starttime=`date`

if [ "$PVT_MAKEFILE" -a -s "$PVT_MAKEFILE" ]; then
        echo "Copying private release makefile:"
        cp -pv $PVT_MAKEFILE release.mk
        export vctoolrc=$?
else
        # If version control tool requested is svn, then get release 
        # makefile from SVN
        echo "Checkout src/tools/release/${BRAND}.mk as release.mk"
        if [ "$VCTOOL" == "svn" ]; then
          # Checkout checkout rules before release makefile is checked out
          echo "Checkout dependent included makefiles before hand"
          $SVNCMD export -q ${REPO_URL_TRUNK}/src/tools/release/${CHECKOUT_DEFS} ${CHECKOUT_DEFS} 2> $NULL
          $SVNCMD export -q ${REPO_URL_TRUNK}/src/tools/release/${CHECKOUT_RULES} ${CHECKOUT_RULES} 2> $NULL

          vctoolcmd="$SVNCMD export ${SVNCUTOFF:+-r \{'$SVNCUTOFF $SVNTZ'\}} ${SVNREV:+-r $SVNREV} ${REPO_URL}/src/tools/release/${BRAND}.mk release.mk"
          echo "$vctoolcmd"
          $SVNCMD export ${SVNCUTOFF:+-r \{"$SVNCUTOFF $SVNTZ"\}} ${SVNREV:+-r $SVNREV} ${REPO_URL}/src/tools/release/${BRAND}.mk release.mk 2> ,${VCTOOL}error.log
          export vctoolrc=$?
        else
          # Checkout checkout rules before release makefile is checked out
          $VCTOOL -Q -d $CVSROOT_PROD co -p src/tools/release/${CHECKOUT_DEFS_CVS} > ${CHECKOUT_DEFS} 2> $NULL
          $VCTOOL -Q -d $CVSROOT_PROD co -p src/tools/release/${CHECKOUT_RULES_CVS} > ${CHECKOUT_RULES} 2> $NULL
           vctoolcmd="cvs -q co ${TAG:+-r $TAG} ${CVSCUTOFF:+-D \"$CVSCUTOFF\"} -p src/tools/release/${BRAND}.mk >release.mk"
           $VCTOOL -q co ${TAG:+-r $TAG} ${CVSCUTOFF:+-D "$CVSCUTOFF"} -p src/tools/release/${BRAND}.mk >release.mk 2> ,${VCTOOL}error.log
           export vctoolrc=$?
        fi
fi

cmd /c title START=$starttime ${TAG:-NIGHTLY} ${BRAND}
cmd /c color 1a

# Strip out cluttered lengthy build path prefixes when generating
# error messages. Build email/message and RLSLOG has that info already.
BDIR=$(pwd)
BDIR_WIN=$(cygpath -w ${BDIR})
BDIR=$(echo ${BDIR} | sed -e 's/\//./g' -e 's/\\/./g').
BDIR_WIN=$(echo ${BDIR_WIN} | sed -e 's/\//./g' -e 's/\\/./g').

(
    # Redirect stdout and stderr to ${RLSLOG}
    exec 3>> ${RLSLOG}
    exec 1>&3 2>&3
    
    # For debugging set following and it overrides value in checkout-*.mk
    # export HNDSVN_CMD=sparse_dbg.bat

    if [[ $vctoolrc == 0 ]]; then
        rm -f ,${VCTOOL}error.log
        date
        echo "Building ${BRAND}"
        echo "START_TIME      = $(date '+%Y-%m-%d %H:%M:%S')"
        echo "PWD             = ${PWD}"
        echo "BUILD_BASE      = ${BUILD_BASE}"
        echo "BRAND           = ${BRAND}"
        echo "BRAND_DIR       = ${BRAND_DIR}"
        echo "BUILD_HOSTOS    = ${HOSTOS}"
        echo "BUILD_HOST      = ${COMPUTERNAME}"
        echo "BUILD_USER      = ${USERNAME}"
        echo "TAG             = ${TAG}"
        if [ "${SVNCUTOFF}" != "" ]; then
           echo "SVN CUTOFF   = ${SVNCUTOFF} ${SVNTZ}"
        fi
        if [ "${SVNREV}" != "" ]; then
           echo "SVNREV       = ${SVNREV}"
        fi
        if [ "${GMAKEFLAGS}" != "" ]; then
           echo "GMAKEFLAGS      = ${GMAKEFLAGS}"
        fi
        if [ -s "${PVT_MAKEFILE}" ]; then
           echo "*PVT MAKEFILE   = ${PVT_MAKEFILE}"
        fi
        if [ "${EXTRA_LOG_STATUS}" != "" ]; then
           echo "-----------------------------------------"
           echo "EXTRA LOG STATUS:"
           echo "${EXTRA_LOG_STATUS}"
        fi

        echo ""
        env | sort | awk -F= '{printf "%20s = %-s\n",$1,$2}' >> ${ENVLOG}
        echo "--------------------------------------------"  >> ${ENVLOG}
        echo "Make version used: `which $MAKE`"              >> ${ENVLOG}
        $MAKE --version                                      >> ${ENVLOG}
        echo "--------------------------------------------"  >> ${ENVLOG}
        grep BUILD_CONFIG_VERSION $BUILD_CONFIG              >> ${ENVLOG}
        echo "SCRIPT VERSION = $SCRIPT_VERSION"              >> ${ENVLOG}
        echo "--------------------------------------------"  >> ${ENVLOG}

        export TARGETENV=win32

        touch $WARN_FILE
        # CVSCUTOFF is exported to the env instead of passing explicitly
        MAKEOPTS="${TAG:+TAG=$TAG} ${BRAND:+BRAND=$BRAND} ${COTOOL:+COTOOL=$COTOOL} ${SVNREV:+SVNREV=$SVNREV} ${GMAKEFLAGS:+$GMAKEFLAGS}"
        echo "exec ${MAKE} -w -f release.mk ${MAKEOPTS} ${makeargs}"
        exec ${MAKE} -w -f release.mk ${MAKEOPTS} ${makeargs}
    else
        echo "Cannot find makefile \'src/tools/release/${BRAND}.mk\'"
        echo "Are you sure you supplied the correct brand name or release tag?"
        echo "$VCTOOL exit code: vctoolrc = $vctoolrc"
        echo ""
    fi
)
export makerc=$?

if [ -f ",${VCTOOL}error.log" ]; then
   echo "ERROR: $VCTOOL failed to get src/tools/release/${BRAND}.mk"
   echo "ERROR: $VCTOOL command used : \"$vctoolcmd\""

   echo "ERROR: $VCTOOL failed to get src/tools/release/${BRAND}.mk"  >>$RLSLOG
   echo "ERROR: $VCTOOL command used : \"$vctoolcmd\""                >>$RLSLOG
   echo "ERROR: $VCTOOL error message: \"`cat ,${VCTOOL}error.log`\"" >>$RLSLOG
   echo ""
   makerc=$vctoolrc
fi

echo "[$(date '+%Y-%m-%d %H:%M:%S')] ${MAKE} exit code: $makerc"
echo "[$(date '+%Y-%m-%d %H:%M:%S')] ${MAKE} exit code: $makerc" >> ${RLSLOG}

# Mark build as failed, when emake fails and exits with zero error code
if [ "${EMAKE}" != "" ]; then
    errors=$(grep "${EMAKEERRORS}" ${RLSLOG} | wc -l | xargs printf "%d")
    if [ "${errors}" != "0" ]; then
        echo "[`date`] ERROR: $EMAKE error detected. Marking build as failed"
        makerc=1
    fi
fi

# If a user explicitly asks for CVS build and new checkout-rules haven't
# been included to create _CVS_BUILD flag file, let build script create 
# those flag files to easily identify builds
if echo "${GMAKEFLAGS}" | grep -q "CVSROOT="; then
   if [ ! -f "_CVS_BUILD" ]; then 
      echo "INFO: Marking $TAG $BRAND as a CVS build explicitly"
      echo "$CVSROOT" > _CVS_BUILD
   fi 
fi

if [[ $makerc == 0 ]]; then
    echo "[`date`] Build succeeded with exit code: $makerc"
    touch ,succeeded
    rm -f $WARN_FILE

    ## Sometimes make exits with '0', even when there are errors 
    show_build_errors "-p windows ${TAG:+-r $TAG}" ${RLSLOG} > ${IGNORED_ERRORLOG} 2> ${NULL}
    if [ -s "${IGNORED_ERRORLOG}" ]; then
       echo "[`date`] Build succeeded but with some errors which were ignored"
       buildresult="(ERRORS IGNORED)"
       touch $IGNORE_FILE
       [ -d "release" ] && touch release/$IGNORE_FILE
       unix2dos ${IGNORED_ERRORLOG} > ${NULL} 2>&1
    else
       if [ -f "$WARN_VCTOOL_FILE" ]; then
          echo "$(hostname): WARN: Built with wrong cvs or svn checkout"
       fi
       # There are no errors that are ignored, cleanup IGNORED_ERRORLOG file
       rm -f ${IGNORED_ERRORLOG}
       buildresult=""
    fi
else
    echo "[`date`] Build failed with exit code: $makerc"
    buildresult="(FAILED)"
    [ -d "release" ] && touch release/$WARN_FILE
fi

# Build failed
if [ ! -f ",succeeded" ] ; then
   # Generate ${ERRORLOG} for use by build_summary script
   rm -f ${ERRORLOG}
   echo "${TAG:-TOT} $BRAND full build log at: ${RLSLOG}" > ${ERRORLOG}
   echo "Filtered Errors:"                                >> ${ERRORLOG}
   echo ""                                                >> ${ERRORLOG} 
   show_build_errors "-p windows ${TAG:+-r $TAG}" ${RLSLOG} >> ${ERRORLOG} 2> ${NULL}
   # If find_build_errors can't filter out errors, when the build
   # fails, show last 15 lines from ,release.log
   if [ ! -s "${ERRORLOG}" ]; then
      tail -15 $RLSLOG | \
        perl -ne "s/$BDIR//gi; s/$BDIR_WIN//gi; print" | \
        perl -ne 's/^(\d+)[:-]/[$1] /g; print' >> $ERRORLOG
   fi
   if [ -f "${MISC_DIR}/,emake_${BRAND}_anno.xml" ]
   then
      echo "${FIND_ECAGENT} --anno=${MISC_DIR}/,emake_${BRAND}_anno.xml"
      sleep 10 # wait until .xml file is flushed after the build
      failedagent=$(${FIND_ECAGENT} --anno=${MISC_DIR}/,emake_${BRAND}_anno.xml)
      echo "*** Last Build Failure on \"$failedagent\"" >> ${ERRORLOG}
   fi

   if [ -s "${DONGLE_ERRORLOG}" ]; then
      echo -e "\n  === Start: Dongle Build Errors ===\n" >> ${ERRORLOG}
      cat  "${DONGLE_ERRORLOG}"                            >> ${ERRORLOG}
      echo -e "\n  === End  : Dongle Build Errors ===\n" >> ${ERRORLOG}
   fi

   echo "Build FAILED at $(date) with errors: $makerc"
   if [ -s "${ERRORLOG}" ]; then
      unix2dos ${ERRORLOG} > ${NULL} 2>&1
      cat "${ERRORLOG}" | col -b
   fi
fi

echo "ANALYSIS_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

skipsrvmsg=""

if [ "$?" != "0" ]; then
   skipsrvmsg="\nERROR: Max build attempts in ${SERVER_BASE}/${RELDIR}/${BRAND}\n"
   skipsrvmsg="${skipsrvmsg}ERROR: Can not create unique iteration dir\n"
   echo -e "$skipsrvmsg"
   echo -e "$skipsrvmsg" >> ${ERRORLOG}
   SKIP_SERVER_COPY=1
fi

if [ ! -f ",succeeded" -a -f "${MISC_DIR}/,emake_${BRAND}_anno.xml" ]
then
   echo "${FIND_ECAGENT} --anno=${MISC_DIR}/,emake_${BRAND}_anno.xml"
   sleep 10 # wait until .xml file is flushed after the build
   failedagent=$(${FIND_ECAGENT} --anno=${MISC_DIR}/,emake_${BRAND}_anno.xml)
   echo "*** Last Build Failure on \"$failedagent\"" >> ${RLSLOG}
fi

echo "Removing old builds or previous failed build iterations if any"
remove_old_builds;

echo "OLD_BUILDS_REMOVED = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

if [ ! -d "$serverdir" ]; then
   serverdir=$(uniquedir "${SERVER_BASE}/${RELDIR}/${BRAND}/${date}" "$serverdir")
   echo "WARN: Previous $serverdir missing. Recreating"
fi

# keep a copy of E.C history/cache file for debugging on local build host
if [ "${EMAKE}" != "" -a -f "${ECHIST_FILE}" ]; then
   cp -p ${ECHIST_FILE} ${MISC_DIR}/emake.data
   gzip -f -9 ${MISC_DIR}/emake.data
fi

echo "BUILD_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}
echo "MAKE_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

# For external and mfgtest builds, see if any .svn folders are still
# remaining in src or release folders
echo "[`date`] DBG: Checking if .svn folders are present in release package"
if echo "$BRAND" | egrep -i "external|mfgtest"; then
       echo "[`date`] DBG: Found external build, checking if .svn folders exist"
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
          if [ -f ",succeeded" ]; then
             cat ${MISC_DIR}/src_dot_svn_found.log \
                 ${MISC_DIR}/release_dot_svn_found.log \
                 ${MISC_DIR}/tar_dot_svn_found.log \
                 ${MISC_DIR}/targz_dot_svn_found.log 2> $NULL | col -b | \
             mail -s "ERROR: .svn folders found at $BRAND_DIR" $ADMINMAILTO
          fi
       fi
fi

# When build is interrupted, append to build error log
function trap_func {
        signal_found=$(($?-128))

        echo "${TAG:-TOT} $BRAND full build log at: ${RLSLOG}" >> $ERRORLOG
        tail -10 ${RLSLOG}                                     >> $ERRORLOG
        echo ""                                                >> $ERRORLOG
        echo "WARN: Build termination detected at `date`"      >> $ERRORLOG
        echo "WARN: signal=$signal_found on $(hostname)"       >> $ERRORLOG
        if [ ! -f "${MISC_DIR}/serverdir.txt" ]; then
           echo "WARN: Copying the local build to"        >> $ERRORLOG
           echo "WARN: `cat ${MISC_DIR}/serverdir.txt`"   >> $ERRORLOG
           copy_to_server;
        fi
        exit 1
}

trap trap_func SIGHUP SIGINT SIGTERM

# copy the build up to the sever.
if [ "${SKIP_SERVER_COPY}" == "" ]; then
   echo "makerc ${makerc}; copy to server"
   echo "New server directory selected for copying:"
   echo " ${serverdir}"
   copy_to_server;
else
   echo "makerc ${makerc}"
   echo "WARN:"
   echo "WARN: Skipping copy to server"
   echo "WARN:"
fi

(
    serverdir_unix=$(echo $serverdir | sed -e 's#.:##g' -e 's#\\#/#g')

    # Redirect stdout and stderr to ${MAILLOG}
    exec 3> ${MAILLOG}
    exec 1>&3 2>&3

    # Build End Time
    endtime=`date`

    date
    echo "Build Start     = $starttime"
    echo "Build End       = $endtime"
    echo "Brand           = ${BRAND}"
    echo "TAG             = ${TAG}"
    echo "Build Host      = ${COMPUTERNAME}"
    echo "Build User      = ${USERNAME}"
    if [ "${EXTRA_LOG_STATUS}" != "" ]; then
       echo "Requester Info  = ${EXTRA_LOG_STATUS}"
    fi

    if [ "${CVSCUTOFF}" != "" ]; then
       echo "Cvs Cutoff      = ${CVSCUTOFF}"
    fi
    if [ "${SVNCUTOFF}" != "" ]; then
       echo "SVN Cutoff      = ${SVNCUTOFF} ${SVNTZ}"
    fi
    if [ "${SVNREV}" != "" ]; then
       echo "SVNREV          = ${SVNREV}"
    fi
    if [ "${GMAKEFLAGS}" != "" ]; then
       echo "Gmake Flags     = ${GMAKEFLAGS}"
    fi
    echo "Build Directory = ${BUILD_BASE}/${TAGDIR}/${BRAND_DIR}"

    if [ -f "${serverdir}/${RLSLOG}" ];  then
       serverdrive=$(echo $serverdir | cut -c1,2)
       if net use $serverdrive > $NULL 2>&1; then
          echo "Build Log       = ${WEB_URL}${serverdir_unix}/${RLSLOG}"
          if [ -s "${DONGLE_RLSLOG}" ]; then
             echo "Dongle Log      = ${WEB_URL}${serverdir_unix}/${DONGLE_RLSLOG}"
          fi
       fi
    fi

    SERVER_BASE_UNC=$(echo ${SERVER_BASE} | sed -e 's%z:%//brcm-sj/dfs%gi')

    if [ "${SKIP_SERVER_COPY}" == "" ]; then
       if [ -d "${SERVER_BASE}/${RELDIR}/${BRAND}" ]; then
          echo "Server Directory= ${SERVER_BASE}/${RELDIR}/${BRAND}"
       elif [ -d "${SERVER_BASE_UNC}/${RELDIR}/${BRAND}" ]; then
          echo "Server Directory= ${SERVER_BASE_UNC}/${RELDIR}/${BRAND}"
       else
          if [[ $vctoolrc == 0 ]]; then
             echo "ERROR:"
             echo "ERROR: Server dir [${SERVER_BASE}/${RELDIR}/${BRAND}]"
             echo "ERROR: not accessible for copying the build.  A local copy of"
             echo "ERROR: build is retained at ${PWD} on ${COMPUTERNAME} temporarily"
             echo "ERROR:"
          fi
       fi
    else
       echo "Server Directory= SKIPPED SERVER DIR COPY"
       [ -n "${skipsrvmsg}" ] && echo -e "\nSkip Message: ${skipsrvmsg}\n"
    fi

    echo ""

    if [ -f ",succeeded" ]; then
       echo \'${MAKE}\' succeeded: $makerc
    else
       echo "Build FAILED at $(date) with $makerc exit code"
       if [ -s "${ERRORLOG}" ]; then
          cat "${ERRORLOG}" | col -b
       else
          tail -30 ${RLSLOG} | col -b | \
               perl -ne "s/$BDIR//gi; s/$BDIR_WIN//gi; print" | \
               perl -ne 's/^(\d+)[:-]/[$1] /g; print'
       fi
    fi
)

echo -e "\n======= END LOG `date '+%Y-%m-%d %H:%M:%S'` ===========\n"

# Set subject line for email notification
mailsubject="Build $buildresult for ${RELDIR}:${BRAND} done on ${COMPUTERNAME}"

# Tag the subject if build runs on non regular build servers
if echo "${COMPUTERNAME}" | grep -i -q "NT-SJCA-051"; then
      mailsubject="$mailsubject [BUILD_SERVER: OLD]"
elif echo "${COMPUTERNAME}" | grep -i -q "WC-SJCA-EB01"; then
      mailsubject="$mailsubject [BUILD_SERVER: TEST]"
elif echo "${COMPUTERNAME}" | grep -i -q "WC-SJCA-E031"; then
      mailsubject="$mailsubject [BUILD_SERVER: TEST]"
elif echo "${COMPUTERNAME}" | grep -i -q "WC-SJCA-E0"; then
      mailsubject="$mailsubject [BUILD_SERVER: NODE]"
fi

# Send status mail if flag is set. '-m none; is rarely set. If set alert admin
# user if build fails for any reason
if [ -n "$MAILSTATUS" ]; then
   unix2dos ${MAILLOG} > ${NULL} 2>&1
   if [ -s "${MISC_DIR}/serverdir.txt" ]; then
      serverdir=`cat ${MISC_DIR}/serverdir.txt`
      echo "Copying $MAILLOG to $serverdir/${MISC_DIR}"
      cp ${MAILLOG} $serverdir/${MISC_DIR}
      echo "${TAG}" | grep "_REL_"
      if echo "${TAG}" | grep -qi "_REL_"; then
         if [ -s "${SYSTEMDRIVE}/build_logs/${RELDIR}/${BRAND}.log" ]; then
            echo "Copying build console log to $serverdir/${MISC_DIR}"
            cp ${SYSTEMDRIVE}/build_logs/${RELDIR}/${BRAND}.log $serverdir/${MISC_DIR}
         fi
      fi
   fi

   sed -e "s/^\[/\n\[/g" ${MAILLOG} | blat - -mailfrom hwnbuild@broadcom.com -t "${MAILTO}" -s "$mailsubject"
   mailrc=$?
   if [ "${mailrc}" != "0" ] ; then
      echo
      echo "$0:  Failed to send status mail"
   fi
elif [ -f ",succeeded" ]; then
   # If build succeeds, discard large debug logfiles
   rm -f ${serverdir}/${MISC_DIR}/*_debug.log*
elif [ -f "$ERRORLOG" ]; then
   # When builds fail, notify admins if any infra related errors are found
   # The list of patterns come from find_build_errors pattern database
   if cat "$ERRORLOG" | egrep -qi "ERROR EC.*:|handle.*exception called|not enough space|: No space left on device|cvs.*failed.*connection.*refused|checkout aborted|Failed in an LSF library call|specified timestamp server could not be reached"; then
      echo "ERROR: Build failed. Alerting $ADMINMAILTO"
      mailsubject="$mailsubject [ADMIN EMAIL]"
      sed -e "s/^\[/\n\[/g" ${MAILLOG} | blat - -mailfrom hwnbuild@broadcom.com -t "${ADMINMAILTO}" -s "$mailsubject"
   fi
   if egrep -qi "specified timestamp server could not be reached" $ERRORLOG; then
      echo "INFO: Recording signtool timestamp server issues in:"
      echo "INFO: Log = $SIGNTS_ERRORLOG.log"
      echo "[`date '+%Y/%m/%d %H:%M:%S'`] tag=$TAG brand=$BRAND error=timestamp" >> $SIGNTS_ERRORLOG
   fi
fi

if [ -s "${MISC_DIR}/,network_errors.log" ]; then
   mailsubject="Build network ERRORS on ${COMPUTERNAME} for  ${RELDIR}:${BRAND}"
   blat ${MISC_DIR}/,network_errors.log -mailfrom hwnbuild@broadcom.com -t ${ADMINMAILTO} -s "$mailsubject"
fi

# Typically pruning is implemented by a prune script, but some times
# when we have too many builds and respins, the build server disk-space
# fills up very fast. The following tries to cleanup local build dirs
# when disk-space usage goes beyond 60%
if [ "${SKIP_SERVER_COPY}" == "" ]; then
   copysrc=${BUILD_BASE}/${TAGDIR}/${BRAND_DIR}
   blddelbat="${TEMP}/purgelocal_${TAGDIR}_${BRAND_DIR}_$(date '+%y%m%d_%H%M%S').bat"
   du=`df -k $copysrc | grep -v "Filesystem" | awk '{print $5}'`  
   du=`echo $du | sed -e 's/%//g'`
   if [ -d "${serverdir}" -a -f "$serverdir/,succeeded" -a -f "$copysrc/,succeeded" ]; then
       if [ "$du" -gt "60" ]; then
          echo "Removing temp build dir contents: $copysrc"
          echo "Local disk-usage: ${du}%"
          echo "Removal script  : $blddelbat"
          echo "@echo on"                                   > ${blddelbat}
          echo "@echo Local disk-usage ${du}%"             >> ${blddelbat}
          lmisc=${copysrc}/misc
          echo "c:\\tools\\utils\\handle.exe $lmisc"       >> ${blddelbat}
          lsrc=${copysrc}/src
          echo "c:\\tools\\utils\\handle.exe $lsrc"        >> ${blddelbat}
          echo "echo %date% %time% Start Delete: $lsrc"    >> ${blddelbat}
          echo "rm -rf $lsrc"                              >> ${blddelbat}
          echo "echo %date% %time% End   Delete: $lsrc"    >> ${blddelbat}
          lrls=${copysrc}/release
          echo "c:\\tools\\utils\\handle.exe $lrls"        >> ${blddelbat}
          echo "echo %date% %time% Start Delete: $lrls"    >> ${blddelbat}
          echo "title START rm -rf ${lrls//\//\\}"         >> ${blddelbat}
          echo "rm -rf $lrls"                              >> ${blddelbat}
          echo "title END   rm -rf ${lrls//\//\\}"         >> ${blddelbat}
          echo "echo %date% %time% End   Delete: $lrls"    >> ${blddelbat}
          echo "sleep 2"                                   >> ${blddelbat}
          echo "exit"                                      >> ${blddelbat}
          cp $blddelbat ${serverdir}/misc/purgedir.log
          cmd /c "START /MIN $(cygpath -w ${blddelbat})"
          #sleep 10
          #rm -f $blddelbat
       fi
   else
          echo "Local disk-usage: ${du}%"
          echo "Local build dir $copysrc will be cleaned up by prune service"
   fi
fi

endtime=`date`
cmd /c title END=$endtime ${TAG:-NIGHTLY} ${BRAND}

# Set final exit code same as what returned by last make command
exit $makerc	
