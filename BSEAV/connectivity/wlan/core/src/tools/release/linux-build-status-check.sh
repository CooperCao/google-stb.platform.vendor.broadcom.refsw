#!/bin/bash
#
# $Id$

NULL=/dev/null
UNAME=$(uname)

RSHHOST=xrhel.sj.broadcom.com
SSHOPTS="-o StrictHostKeyChecking=no -o BatchMode=yes"
RSHCMD="ssh -q $SSHOPTS -l hwnbuild"

case $UNAME in
        *CYGWIN*|*Cygwin*|*cygwin*)
                RSH="$RSHCMD $RSHHOST"
		;;
        *)
                RSH=""
		;;
esac

echo "INFO: Calling $0 to check build status: $*"

while [ $# -gt 0 ]; do
    case $1 in
        -d|--dir)
            shift
            BUILD_DIR_PARENT="$1"
            ;;
        -j|--jobid)
            shift
            JOB_ID="$1"
            ;;
        -r|--rshcmd)
            shift
            RSH="$1"
            ;;
    esac
    shift
done

BUILD_DIR_PARENT=$(echo $BUILD_DIR_PARENT | sed -e 's%{.*}%*%g')
echo "INFO: Will check status of build job-id $JOB_ID in $BUILD_DIR_PARENT"

# If regular nightly tot or tob build has not built a dongle image, then
# use BUILD_DIR_PARENT as a temp build space to generate private dongle image

# File to search for LSF job-id in the launched build's environment log
ENV_LOG="misc/,env.log"

echo "Searching in BUILD_DIR_PARENT=$BUILD_DIR_PARENT"

# If a private lsf job is submitted JOBID is defined
BUILD_DIR_OLD=$(find $BUILD_DIR_PARENT -maxdepth 0 -mindepth 0 2> $NULL | xargs ls -1td)

#BUILD_DIR_NEW=$(egrep -l "LSB_JOBID = ${JOB_ID}$" ${BUILD_DIR_PARENT}/${ENV_LOG} 2> $NULL | sed -e "s%$ENV_LOG%%g")

for bld in ${BUILD_DIR_PARENT}
do
	echo "Searching PVT build: $bld for ${JOB_ID} now"
	if egrep -q "LSB_JOBID = ${JOB_ID}$" $bld/${ENV_LOG}; then
		echo "Setting BUILD_DIR_NEW=$bld"
		BUILD_DIR_NEW=$bld
	fi
done

# If a private build is launched, use its lsf jobid for BUILD_DIR
# otherwise take the latest private build
if [ -d "$BUILD_DIR_OLD" ]; then
        echo "Setting OLD BUILD_DIR=$BUILD_DIR_OLD"
	BUILD_DIR=$BUILD_DIR_OLD
else
        echo "Setting NEW BUILD_DIR=$BUILD_DIR_NEW"
	BUILD_DIR=$BUILD_DIR_NEW
fi

# Wait time between lsf query. (Default is 3min. i.e. 180seconds)
if [ "$BUILD_ITER_TIME" == "" ]; then
	#BUILD_ITER_TIME=180
	BUILD_ITER_TIME=15
	echo "Setting BUILD_ITER_TIME=$BUILD_ITER_TIME"
fi

# How many times to check build status, default 90times $(BUILD_ITER_TIME)
# Total wait-time = (BUILD_ITER_TIME * BUILD_ITER_COUNT)
if [ "$BUILD_ITER_COUNT" == "" ]; then
	#BUILD_ITER_COUNT=120
	BUILD_ITER_COUNT=2
	echo "Setting BUILD_ITER_COUNT=$BUILD_ITER_COUNT"
fi


# Loop BUILD_ITER_COUNT iterations until launched build is done
for loop in $(seq 0 ${BUILD_ITER_COUNT}); do
	echo "Waiting ${BUILD_ITER_TIME} secs for job ${JOB_ID} to complete"
	sleep ${BUILD_ITER_TIME}
	if [ -d "${BUILD_DIR}" -a "${BUILD_DIR} != "\." ]; then
		if [ -f "${BUILD_DIR}/,succeeded" ]; then
			echo "   INFO: [$loop] Job ${JOB_ID} finished, build succeeded"
			echo "   INFO: [$loop] Found ${BUILD_DIR}/,succeeded file"
		elif [ -f "${BUILD_DIR}/,build_errors.log" ]; then
			echo "   INFO: [$loop] Job ${JOB_ID} finished, build failed"
			echo "   INFO: [$loop] Found ${BUILD_DIR}/,build_errors.log file"
		else
			echo "   INFO: [$loop] Job ${JOB_ID} is in progress"
			echo "   INFO: [$loop] at ${BUILD_DIR}"
		fi
	else
		echo "   WARN: [$loop] Job ${JOB_ID} not yet started"
	fi

	lsfstat=$($RSH bjobs ${JOB_ID} | grep -v JOBID)
	case "$lsfstat" in
		 *RUN*) echo "WARN: [$loop] ${JOB_ID} is still running";;
		*PEND*) echo "WARN: [$loop] ${JOB_ID} is still pending";;
		*DONE*) echo "INFO: [$loop] ${JOB_ID} is done"; exit 0;;
		*EXIT*) echo "WARN: [$loop] ${JOB_ID} is exited"; exit 0;;
		     *) echo "WARN: [$loop] ${JOB_ID} status unknown";;
	esac
done
