#!/bin/bash
##
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##
## Search the build storage directories for any failed build attempts
##
## $Id: find_failed_builds.sh,v 12.18 2011/02/13 07:14:25 Exp $

BUILD_ADMINS="hnd-software-scm-list@broadcom.com"

# Cleanup script can be run on windows side if needed
uname | grep -i cygwin_nt > /dev/null 2>&1

if [ "$?" = "0" ]; then
   DRIVE="Z:"
   HOST="windows"
else
   DRIVE=
   HOST="linux"
fi

PROG=$0
PROGPATH=${PROG%/*}
CALL_DELETE=0
verbose=
while [ $# -gt 0 ]
do
    case $1 in
    -delete)
        CALL_DELETE=1
        ;;
    -verbose)
        verbose=1
        ;;
    *)
        ;;
    esac
    shift
done

# Ignore the non-default brand builds that were moved by build_summary for two
# more days (2 in build_summary + 2) before deleting them.
days2ignore=4
case "`date +%a`" in
Mon)
    days2ignore=$((days2ignore + 2))
    ;;
Tue)
    days2ignore=$((days2ignore + 1))
    ;;
*)
    ;;
esac

BUILD_ROOT=/projects/hnd/swbuild
BUILD_LOG_DIR=/projects/hnd/swbuild/build_admin/logs/diskspace/swbuild_cleanup/FAILED_BUILDS
BUILD_DIRS=(${DRIVE}${BUILD_ROOT}/build_window \
${DRIVE}${BUILD_ROOT}/build_linux \
${DRIVE}${BUILD_ROOT}/build_netbsd \
${DRIVE}${BUILD_ROOT}/build_macos)

## Skip read-only archive folders for now
# ${DRIVE}${BUILD_ROOT}/build_window/ARCHIVED
# ${DRIVE}${BUILD_ROOT}/build_linux/ARCHIVED
# ${DRIVE}${BUILD_ROOT}/build_netbsd/ARCHIVED
# ${DRIVE}${BUILD_ROOT}/build_macos/ARCHIVED

timestamp=`date '+%Y%m%d'`
todayiter=`date '+%Y\.%m\.%d\.'`
if [ "$HOST" == "windows" ]; then
   # TODO: Test previter generation on windows platform
   previter=`date '+%Y\.%m\.%d\.'`
else
   # Generate previous date using touch mechanism
   touch=/home/hwnbuild/bin/newtouch
   tmpfile=`mktemp /tmp/prevday.XXXX`
   # Set previous duration to be exactly 24 hours behind current time
   prevduration=$((24*60*60))
   $touch -B $prevduration $tmpfile
   # 2009-11-09 09:32:32.000000000 -0800
   previter=`stat -c "%y" $tmpfile | awk '{print $1}' | sed -e 's/-/\\\\./g'`
   previter="${previter}\\."
   rm -f $tmpfile
fi
LOGFILE=bld_cleanup_${timestamp}.log

cd $BUILD_LOG_DIR
exec > $LOGFILE 2>&1

echo "NOTE: Skipping build iterations from today=$todayiter and yesterday = $previter"

# Output filenames
# failed is list of build iterations that failed and have ,build_errors.log
failed="${PWD}/failed_builds_${timestamp}.txt"
# duplicate is list of build iterations that are redundant
duplicate="${PWD}/duplicate_builds_${timestamp}.txt"
# verify is list of build iterations that show errors in ,release.log, but
# have been marked as succeeded as errors are ignored intentionally
verify="${PWD}/verify_builds_${timestamp}.txt"
# have been moved by build_summary and need to be deleted
disabled="${PWD}/disabled_builds_${timestamp}.txt"

# Cleanup old history
rm -fv $failed;    touch $failed
rm -fv $duplicate; touch $duplicate
rm -fv $verify;    touch $verify
rm -fv $disabled;  touch $disabled

# Initialize the output files
echo -e "\nFAILED BUILDS TO BE DELETED as of `date`:\n"    > $failed
echo -e "\nDUPLICATE BUILDS TO BE CLEANED as of `date`:\n" > $duplicate
echo -e "\nOTHERS BUILDS TO BE VERIFIED   as of `date`:\n" > $verify
echo -e "\nDISABLED BUILDS TO BE DELETED   as of `date`:\n" > $disabled

# Keep count of number of verify, failed and duplicate build iterations
vindex=1
findex=1
dindex=1

## Search all BUILD_DIRS (production and sw_archive_ro area)

for idir in $(seq 0 ${#BUILD_DIRS[*]}); do
    builddir=${BUILD_DIRS[$idir]}
    [ ! -n "$builddir" ] && continue
    platform=`basename ${BUILD_DIRS[$idir]}`
    BUILD_DIR="${builddir}"
    [ ! -d "$BUILD_DIR" ] && continue
    echo -e "\n===== START: SCAN $(basename $builddir) BUILDS =====\n"
    echo "#==== START: FAILED $(basename $builddir) BUILDS ====" >> ${failed}
    echo "#==== START: DUPLICATE $(basename $builddir) BUILDS ====" >> ${duplicate}
    echo "#==== START: VERIFY $(basename $builddir) BUILDS ====" >> ${verify}
    echo "#==== START: DISABLED $(basename $builddir) BUILDS ====" >> ${disabled}

    ## Search all builds in a build_<dir>

    cd $BUILD_DIR
    # Search for all builds, except for some special dirs
    BUILDS=(`ls -1 | grep "_REL_" | egrep -v "LOGS|SORTED|ARCHIVED|RESTORE|PRESERVED"`)
    for ibuild in $(seq 0 ${#BUILDS[*]}); do
        buildtag=${BUILDS[$ibuild]}
        [ ! -n "$buildtag" ] && continue
        BUILD_DIR="${builddir}/${buildtag}"
        [ ! -d "$BUILD_DIR" ] && continue
        [ -n "$verbose" ] && echo " -- Searching $platform/$buildtag "

        ## Search all brands in a build

        cd $BUILD_DIR
        BRANDS=(`ls -1`)
        for ibrand in $(seq 0 ${#BRANDS[*]}); do
            brandname=${BRANDS[$ibrand]}
            if echo $brandname | egrep -iq _disabled; then
               echo " --     Skip $buildtag/$brandname"
               continue
            fi
            [ ! -n "$brandname" ] && continue
            BUILD_DIR="${builddir}/${buildtag}/${brandname}"
            [ ! -d "$BUILD_DIR" ] && continue

            ## Search all build-attempts/iteration for a brand

            cd $BUILD_DIR
            #echo "   ITERS Orig=`echo 2*`"
            # Ensure that build iterations can be sorted reliably, irrespective
            # of how many digits are used to represent month and day fields
            # Prefix single digit month and date fields with 0
            # Skip build iterations from today and yesterday's dates
            ATTEMPTS=(`echo 2* | fmt -1 | perl -ne 's/\.(\d)\.(\d)\./\.0$1\.0$2\./g; s/\.(\d)\./\.0$1\./g; print $_' | sort -r | egrep -v "$todayiter|$previter" | xargs echo`)
            #echo "   ITERS New=$brandname : ${ATTEMPTS[*]}"
            succeeded=0
            duplicate_build=0
            failed_build=0
            verify_build=0
            keep_build=0
            for iattempt in $(seq 0 ${#ATTEMPTS[*]}); do
                attemptnum=${ATTEMPTS[$iattempt]}
                # take out prefix for single digit month and date fields with 0
                attemptnum=`echo $attemptnum | perl -ne 's/\.0(\d)\.0(\d)\./\.$1\.$2\./g; s/\.0(\d)\./\.$1\./g; print $_'`
                [ ! -n "$attemptnum" ] && continue
                BUILD_DIR="${builddir}/${buildtag}/${brandname}/${attemptnum}"
                BUILD_ITER=`echo $BUILD_DIR | sed -e "s%${BUILD_ROOT}/%%g"`
                [ ! -d "$BUILD_DIR" ] && continue
                cd $BUILD_DIR
                releaselog=",release.log"
                errorlog=",build_errors.log"
                succeededlog=",succeeded"

                [ -s "${releaselog}.gz" ] && gzip -d -v "${releaselog}.gz"
                [ -s "${errorlog}.gz" ] && gzip -d -v "${errorlog}.gz"
                [ -s "${succeededlog}.gz" ] && gzip -d -v "${succeededlog}.gz"

                if [ -s "${releaselog}" -o -s "${releaselog}.gz" ]
                then
                   if [ -f "${errorlog}" -o -f "${errorlog}.gz" ]
                   then
                      # Build iteration failed
                      echo $findex | \
                           xargs printf "%3d: Failed   : ${BUILD_ITER}\n"
                      echo "${BUILD_DIR}" >> ${failed}
                      findex=$(($findex + 1))
                      failed_build=1
                      ## In rare situations, there are errors, but succeeded
                      ## is generated (or it is a manual created build)
                      errors=$(grep "Error [0-9]\+[[:space:]]*$" ${releaselog})
                      if [ -n "${errors}" ]; then
                         echo $vindex | \
                              xargs printf "%3d: Verify   : ${BUILD_ITER}\n"
                         echo "${BUILD_DIR}" >> ${verify}
                         verify_build=1
                         vindex=$(($vindex + 1))
                      fi
                   elif [ -f "${succeededlog}" -o -f "${succeededlog}.gz" ]; then
                      # Build iteration succeeded, check for duplicates
                      if [ $succeeded -gt 0 ]; then
                         if [ "$keep_build" != "1" ]; then
                            echo "   : *Keep*   : ${prev_build_iter}"
                            keep_build=1
                         fi
                         echo $dindex | \
                              xargs printf "%3d: Duplicate: ${BUILD_ITER}\n"
                         echo "${BUILD_DIR}" >> ${duplicate}
                         dindex=$(($dindex + 1))
                         duplicate_build=1
                      fi
                      succeeded=$(($succeeded + 1))
                      prev_build_iter=${BUILD_ITER}
                   else
                      # Neither errorlog nor succeeded log exists!
                      echo $vindex | \
                           xargs printf "%3d: Verify   : ${BUILD_ITER}\n"
                      echo "${BUILD_DIR}" >> ${verify}
                      verify_build=1
                      vindex=$(($vindex + 1))
                   fi # errorlog

                else # releaselog is missing

                   echo $vindex | \
                        xargs printf "%3d: Verify   : ${BUILD_ITER}\n"
                   echo "${BUILD_DIR}" >> ${verify}
                   verify_build=1
                   vindex=$(($vindex + 1))

                fi # releaselog
            done #iterations/attempt

        if [ -n "$verbose" -a "$duplicate_build" == "1" ]; then
           echo "     All Iters=`basename $builddir`/$buildtag/$brandname/`echo ${ATTEMPTS[*]} | perl -ne 's/\.0(\d)\.0(\d)\./\.$1\.$2\./g; s/\.0(\d)\./\.$1\./g; print $_'`"
        fi
        done #brand
    done #build

    # Search for _diabled directory in branch builds
    BUILD_DIR="${builddir}"
	cd $BUILD_DIR
    for disbuildtag in `ls -d1 */_disabled 2> /dev/null| grep -e "_BRANCH_" -e "_TWIG_" -e "NIGHTLY"`
	do
        DIS_BUILD_DIR="${builddir}/$disbuildtag"
        [ -n "$verbose" ] && echo " -- Searching $platform/$disbuildtag "

        ## Search all brands in a disabled build

        cd $DIS_BUILD_DIR
        for disbrand in `ls -1`
        do
            DIS_BUILD_DIR="${builddir}/${disbuildtag}/${disbrand}"
            cd $DIS_BUILD_DIR
            for disabled_bld in `ls -1`
            do
                lastaccessed=`find $disabled_bld -type f -atime $((days2ignore * -1)) -print`
                if [ -z "$lastaccessed" ]
                then
                    echo "${DIS_BUILD_DIR}/${disabled_bld}" >> ${disabled}
                fi
            done # disabled_bld
        done # dis_brand
    done #dis_build

    echo "#==== END  : FAILED $(basename $builddir) BUILDS ====" >> ${failed}
    echo "#==== END  : DUPLICATE $(basename $builddir) BUILDS ====" >> ${duplicate}
    echo "#==== END  : VERIFY $(basename $builddir) BUILDS ====" >> ${verify}
    echo "#==== END  : DISABLED $(basename $builddir) BUILDS ====" >> ${disabled}
    echo -e "\n===== END  : SCAN $(basename $builddir) BUILDS =====\n"
done #dir

unix2dos $failed $duplicate $verify > /dev/null 2>&1
findex=$(($findex - 1))
dindex=$(($dindex - 1))
vindex=$(($vindex - 1))
echo ""
echo "Found $findex FAILED build iterations to delete (in $failed)"
echo "Found $dindex DUPLICATE build iterations to clean (in $duplicate)"
echo "Found $vindex OTHER build iterations to verify (in $verify)"

if [ $CALL_DELETE -eq 1 ]
then
   #disabled#    $PROGPATH/delete_failed_dup_disabled_bld.sh ${failed} ${duplicate} ${disabled}

   echo "WARN: Disabled automatic deletion of failed/duplicate iterations"
   echo "WARN: This is manually done after scanning builds to delete"

   date=$(date '+%Y/%m/%d')
   today=$(date '+%a')

   if [ "$today" == "Thu" ]; then
      cat $failed $duplicate | \
	   mail -s "[$date] Failed and Duplicate builds to delete" $BUILD_ADMINS
   fi
fi
