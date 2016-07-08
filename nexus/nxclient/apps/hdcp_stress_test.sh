#!/bin/bash

LOGFILE=hdcp_stress_test_log.txt
PIDFILE=hdcp_stress_test_pid.txt

DEFAULT_SLEEP=0
DEFAULT_TEST=100

#*******************************************************************************
# print_help - return help information
#*******************************************************************************
function print_help()
{
	cat <<- EOF

		Options:
		  -s SS   - (def. $DEFAULT_SLEEP) SS is the delay after previous hdcp authentication (sethdcp)
		  -t TT   - (def. $DEFAULT_TEST) NN is the number of sethdcp to execute
		  -svp    - Selects playready_svp playback

		Description:
		  This script executes a sethdcp to force an HDCP authentication. The console
		  log is monitored for completed authentication (or failure).
	EOF

	exit 0
}

#*******************************************************************************
# break_test - handle ctrl-C or end-of-test
#*******************************************************************************
trap break_test INT
function break_test()
{
	echo "*** exiting ***"

	cat ${PIDFILE} |
	while read subprocess ; do
		kill $subprocess > /dev/null 2>&1
	done

	echo ""
	echo ""

	cat ${LOGFILE} | awk '
	BEGIN { start_log = 0; test_started = 0; test_finished = 0; test_cnt = 0; pass_cnt = 0; fail_cnt = 0; last_result="unknown"; }

	start_log == 0 && />>>>>HDCP_STRESS_TEST_START<<<<</ { start_log = 1; next; }
	start_log == 0 { next; }

	/>>>>>SET_HDCP-START<<<<</ { test_started = 1; test_finished = 0; test_cnt++ ; last_result="unknown"; next; }
	test_started == 0 { next; }

	test_finished == 1 { next; }
	/Hdcp2x Authentication status:/ { test_finished = 1; test_started = 0; }
	/Hdcp2x Authentication status: AUTHENTICATED/ { pass_cnt++; last_result="AUTHENTICATED"; }
	/Hdcp2x Authentication status: NOT AUTHENTICATED/ { fail_cnt++; last_result="NOT AUTHENTICATED"; }

	END {
		print "********************************************************"
		print "* Test Cnt = " test_cnt;
		print "* Pass Cnt = " pass_cnt;
		print "* Fail Cnt = " fail_cnt;
		print "*";
		print "* Last authentication was " last_result;
		print "********************************************************"
	}
	'
	exit
}

#*******************************************************************************
# start_server - start the nxserver
#*******************************************************************************
function start_server()
{
	if [[ $use_svp -eq 1 ]] ; then
		svp_string="-svp"
	else
		svp_string=""
	fi
	config="msg_modules=nexus_hdmi_output_hdcp" ./nexus nxserver ${svp_string} -hdcp m -hdcp2x_keys drm.bin 2>> ${LOGFILE} &
	echo "$!" >> ${PIDFILE}
	tail -f ${LOGFILE} &
	echo "$!" >> ${PIDFILE}

# Wait for nxserver (and SAGE) to load and get to "running" state
	sleep 10
}

#*******************************************************************************
# do_playback - start playback
#*******************************************************************************
function do_playback()
{
	if [[ $use_svp -eq 1 ]] ; then
		test_string="playready_svp videos/SuperSpeedway_720_2962-enc.ismv"
	else
		test_string="play -hdcp m videos/big_buck_bunny_1080p_h264.mov"
	fi
	echo "CLIENT:"
	echo "CLIENT: nexus.client $test_string"
	echo "CLIENT:"
	while true ; do nexus.client $test_string 2>&1 ; done &
	echo "$!" >> ${PIDFILE}
}

#*******************************************************************************
# do_sethdcp - start testing hdcp authentication
#*******************************************************************************
function do_sethdcp()
{
	local loop_count=$1
	local sleep_sec=$2
	local select_index=0
	local test_cnt=1

	echo ">>>>>HDCP_STRESS_TEST_START<<<<<" >> ${LOGFILE}

	while true ; do
		let select_index+=1
		if [ $test_cnt -gt $loop_count ] ; then break ; fi
		test_string="sethdcp -timeout $sleep_sec"
		echo ">>> nexus.client $test_string <<<"
		echo ">>>>>SET_HDCP-START<<<<<" >> ${LOGFILE}
		nexus.client $test_string
		let test_cnt+=1
	done
}

#*******************************************************************************
#*******************************************************************************
# MAIN
#*******************************************************************************
#*******************************************************************************

rm -f ${LOGFILE}
rm -f ${PIDFILE}

stress_tests=$DEFAULT_TEST
stress_sleep=$DEFAULT_SLEEP
use_svp=0

while [[ $# > 0 ]] ; do
	key="$1"

	case $key in
		-s )
			stress_sleep="$2"
			shift # past key
			;;
		-t )
			stress_tests="$2"
			shift # past key
			;;
		-svp )
			use_svp=1
			;;
		-h|--help|-?|* )
			print_help
			;;
	esac
	shift # past argument or value
done

start_server
do_playback
do_sethdcp ${stress_tests} ${stress_sleep} | sed 's/^/CLIENT: /'
sleep 5

break_test