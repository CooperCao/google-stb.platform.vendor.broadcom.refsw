#!/bin/bash

SERVER_ERROR_LOG=${SERVER_ERROR_LOG:=server_error.log}
TMP_ERROR_LOG=tmp_error.txt
SUMMARY_LOG=${SUMMARY_LOG:=summary.log}

declare -a PLAYFILE
PLAYFILE[1]=${PLAYFILE1:=videos/cnnticker.mpg}
PLAYFILE[2]=${PLAYFILE2:=videos/japan480p.mpg}
PLAYFILE[3]=${PLAYFILE3:=videos/riddick_avc_720p.mpg}
PLAYFILE[4]=${PLAYFILE4:=videos/discoveryAvcHD.mpg}
PLAYFILE[5]=${PLAYFILE5:=videos/spider_cc.mpg}
PLAYFILE[6]=${PLAYFILE6:=videos/herbie1AvcHD.mpg}
PLAYFILE[7]=${PLAYFILE7:=videos/big_buck_bunny_1080p_h264.mov}
PLAYFILE[8]=${PLAYFILE8:=videos/mosaic_6avc_6mp2_cif.ts}

declare -a SATPARM
SATPARM[1]=${SATPARM1:=-freq 1148 -sat 4 -sym 21500000 -tone on -program 0}
SATPARM[2]=${SATPARM1:=-freq 1148 -sat 4 -sym 21500000 -tone on -program 1}
SATPARM[3]=${SATPARM1:=-freq 1119 -sat dvb -program 0}
SATPARM[4]=${SATPARM1:=-freq 1119 -sat dvb -program 1}

declare -a RECORDPARM
RECORDPARM[1]=${RECORDPARM1:=${SATPARM[1]} videos/freq1148.mpg -timeout 30}

declare server_pid=999
declare num_of_video_windows
declare num_of_video_encoders
declare num_of_video_decoders

#*******************************************************************************
# Function:	print_help
#*******************************************************************************
function print_help()
{
	cat <<- EOF

		Options:
		  -b|--box NN    - NN is the selected boxmode (may be comma-separated list)
		  -t|--test NN   - NN is the selected test
		  -a|--all       - Finish test series (do not exit on FAIL)
		  -h|--help|-?   - This help text

		Description:
		  This script runs a test over all (or selected) box modes.

		  During the build, script "list_all_boxmodes" is generated, which contains a
		  list of all available boxmodes for the given chip.  This script is used in
		  this script.

		Current tests are:
		  0              - Launch nxserver, PASS if server is up
		  1              - Query capabilities of server
		  2              - Launch transcode for all encoders
		  3              - Launch play for all decoders
		  4              - Launch transcode/record_sat (simultaneous transcode and record_sat)

		Examples:
		  $ $0 -a -t 0
		                 - Reports if nxserver could start for each box mode

		  $ $0 -t 2
		                 - FAILS on first boxmode transcode failure

		  $ $0 -b 1 -t 1
		                 - Starts server in boxmode #1 and runs test #1

		  $ $0  -a -b 1,3,12,13,1000 -t 0
		                 - Determines if server can start in boxmodes 1,3,12,13 and 1000

	EOF

	exit 0
}

#*******************************************************************************
# Function:	start_server
#*******************************************************************************
function start_server()
{
	local box_mode=$1
	local server_name

	export B_REFSW_BOXMODE=${box_mode}
	nexus nxserver 2> $SERVER_ERROR_LOG 1>> $SERVER_ERROR_LOG &
	server_pid=$!
	sleep 5
	server_name="`ps -p $server_pid -o comm=`"
	if [[ "$server_name" == "" ]] ; then
		return 1
	fi
	if [[ "$server_name" != "nxserver" ]] ; then
		return 1
	fi
	if [[ "`ps -C logger -o comm=`" != "logger" ]] ; then
		return 1
	fi
	return 0
}

#*******************************************************************************
# Function:	wait_for_server
#*******************************************************************************
function wait_for_server()
{
	nexus.client wait_for_server > /dev/null 2> /dev/null
	if [ $? -ne "0" ]; then
		return 1
	fi

	return 0
}

#*******************************************************************************
# Function:	clean_modules
#*******************************************************************************
function clean_modules()
{
	local ret_code

	ret_code=0
	cat /proc/modules |
	while read modname rest_of_line ; do
		if [[ "$modname" == "wakeup_drv" || \
			  "$modname" == "bcmdriver" ]] ; then
			rmmod $modname
			if [ $? -ne 0 ] ; then
				echo "FAIL: unable to unload \"$modname\""
				ret_code=1
			fi
		fi
	done

	return $ret_code
}

#*******************************************************************************
# Function:	stop_server
#*******************************************************************************
function stop_server()
{
	local nxserver_pid
	local logger_pid

	if [[ "`ps -p $server_pid -o comm=`" == "nxserver" ]] ; then
		kill $server_pid
		wait $server_pid
	fi

	nxserver_pid=`ps -C nxserver -o pid=`
	if [[ "$nxserver_pid" != "" ]] ; then
		kill $nxserver_pid
		wait $nxserver_pid
	fi

	logger_pid=`ps -C logger -o pid=`
	if [[ "$logger_pid" != "" ]] ; then
		kill $logger_pid
		wait $logger_pid
	fi

	clean_modules
	if [ $? -ne 0 ] ; then
		return 1
	fi

	if [[ "`ps -C nxserver -o pid=`" != "" ]] ; then
		return 1
	fi
	if [[ "`ps -C logger -o pid=`" != "" ]] ; then
		return 1
	fi

	return 0
}

#*******************************************************************************
# Function:	report_server_errors
#*******************************************************************************
function report_server_errors()
{
	egrep '^(!!!Error|BKNI_Fail)' $SERVER_ERROR_LOG > $TMP_ERROR_LOG
	cat $TMP_ERROR_LOG
	if [[ $(cat $TMP_ERROR_LOG | wc -l) -ne 0 ]] ; then
		return 1
	fi

	return 0
}

#*******************************************************************************
# Function:	run_empty_test
#*******************************************************************************
function run_empty_test()
{
	local ret_code

	report_server_errors
	if [ $? -ne 0 ] ; then
		return 1
	fi

	return 0
}

#*******************************************************************************
# Function:	run_cap_test
# Notes:
#   This function sets the global variables:
#       num_of_video_windows
#       num_of_video_encoders
#       num_of_video_decoders
#*******************************************************************************
function run_cap_test()
{
	num_of_video_windows=$(nexus.client cap -video_windows)
	if [ $? -ne 0 ] ; then
		report_server_errors
		return 1
	fi
	num_of_video_encoders=$(nexus.client cap -video_encoders)
	if [ $? -ne 0 ] ; then
		report_server_errors
		return 1
	fi
	num_of_video_decoders=$(nexus.client cap -video_decoders)
	if [ $? -ne 0 ] ; then
		report_server_errors
		return 1
	fi
	num_of_rec_pumps=$(nexus.client cap -rec_pumps)
	if [ $? -ne 0 ] ; then
		report_server_errors
		return 1
	fi
	echo "CAP results: windows=${num_of_video_windows} \
		encoders=${num_of_video_encoders} \
		decoders=${num_of_video_decoders} \
		recpumps=${num_of_rec_pumps}"
	report_server_errors
	if [ $? -ne 0 ] ; then
		return 1
	fi

	return 0
}

#*******************************************************************************
# Function:	run_transcode_test
#*******************************************************************************
function run_transcode_test()
{
	local ret_code
	local -a transcode_pid

	run_cap_test
	if [ $? -ne 0 ] ; then
		echo "FAIL: run_cap_test"
		return 1
	fi

	transcode_pid=()
	if [[ $num_of_video_encoders -gt $num_of_video_decoders ]] ; then
		num_of_transcode_tests=$num_of_video_decoders
	else
		num_of_transcode_tests=$num_of_video_encoders
	fi
	for (( i=1 ; i<=$num_of_transcode_tests ; i++ )) ; do
		test_string="${PLAYFILE[$i]}"
		echo ">>> transcode ${test_string}"
		nexus.client transcode ${test_string} &
		transcode_pid[$i]=$!
	done

	sleep 20

	for (( i=1 ; i<=$num_of_transcode_tests ; i++ )) ; do
		kill ${transcode_pid[$i]}
	done
	wait ${transcode_pid[*]} 2> /dev/null

	report_server_errors
	ret_code=$?

	return $ret_code
}

#*******************************************************************************
# Function:	run_play_test
#*******************************************************************************
VRECT_SIZE=100
WIN_SIZE=50
function run_play_test()
{
	local ret_code
	local vrect_pos
	local pip
	local -a play_pid

	run_cap_test
	if [ $? -ne 0 ] ; then
		echo "FAIL: run_cap_test"
		return 1
	fi

	if [ $num_of_video_windows -eq 0 ] ; then
		echo ">>> NOTICE.  No video windows, test not applicable"
		return 0
	fi
	play_pid=()
	let vrect_pos=0
	for (( i=1 ; i<=$num_of_video_decoders ; i++ )) ; do
		if [[ $num_of_video_windows -gt 1 && $i -eq 2 ]] ; then
			pip="-pip"
		else
			pip=""
		fi
		test_string="${PLAYFILE[$i]} -vrect $VRECT_SIZE,$VRECT_SIZE:$vrect_pos,$vrect_pos,$WIN_SIZE,$WIN_SIZE $pip -audio_primers"
		echo ">>> play ${test_string}"
		nexus.client play ${test_string} &
		play_pid[$i]=$!
		let vrect_pos=$vrect_pos+$WIN_SIZE/$num_of_video_decoders
	done

	sleep 20

	for (( i=1 ; i<=$num_of_video_decoders ; i++ )) ; do
		kill ${play_pid[$i]}
	done
	wait ${play_pid[*]} 2> /dev/null

	report_server_errors
	ret_code=$?

	return $ret_code
}

#*******************************************************************************
# Function:	run_transcode_recordsat_test
#*******************************************************************************
function run_transcode_recordsat_test()
{
	local ret_code
	local -a transcode_pid
	local -a record_pid
	local gui_flag

	run_cap_test
	if [ $? -ne 0 ] ; then
		echo "FAIL: run_cap_test"
		return 1
	fi

	transcode_pid=()
	record_pid=()

	if [[ $num_of_video_encoders -lt 1 ]] ; then
		echo "FAIL: no resource for test, num_of_video_encoders=$num_of_video_encoders"
		return 2
	fi

	if [[ $num_of_video_decoders -lt 2 ]] ; then
		echo "FAIL: no resource for test, num_of_video_decoders=$num_of_video_decoders"
		return 2
	fi

	gui_flag=""
	if [[ $num_of_video_windows -lt 1 ]] ; then
		gui_flag="-gui off"
	fi

	test_string="$gui_flag ${PLAYFILE[2]}"
	echo ">>> transcode ${test_string}"
	nexus.client transcode ${test_string} &
	transcode_pid[1]=$!
	test_string="$gui_flag ${RECORDPARM[1]}"
	echo ">>> record ${test_string}"
	nexus.client record ${test_string} &
	record_pid[1]=$!

	sleep 20

	kill ${record_pid[1]}
	kill ${transcode_pid[1]}

	wait ${record_pid[*]} ${transcode_pid[*]} 2> /dev/null

	report_server_errors
	ret_code=$?

	return $ret_code
}

#*******************************************************************************
# Function:	run_boxmode_test
#*******************************************************************************
function run_boxmode_test()
{
	local box_mode=$1
	local test_case=$2
	local ret_code

	echo ""
	echo "********************************"
	echo "* Test boxmode ${box_mode} case ${test_case}"
	echo "********************************"

	num_of_video_windows=0
	num_of_video_encoders=0
	num_of_video_decoders=0

	start_server $box_mode
	if [ $? -ne 0 ] ; then
		echo "FAIL: start_server"
		report_server_errors
		stop_server
		return 11
	fi

	wait_for_server
	if [ $? -ne 0 ] ; then
		echo "FAIL: wait_for_server"
		report_server_errors
		stop_server
		return 12
	fi

	case $test_case in
	0 )
		run_empty_test
		;;
	1 )
		run_cap_test
		;;
	2 )
		run_transcode_test
		;;
	3 )
		run_play_test
		;;
	4 )
		run_transcode_recordsat_test
		;;
	* )
		return 1
		;;
	esac
	ret_code=$?

	stop_server
	return $ret_code
}

#*******************************************************************************
# Function:	get_ret_code_string
#*******************************************************************************
function get_ret_code_string()
{
	local ret_code=$1

	case $ret_code in
	1 )	echo -n "test"		;;
	2 )	echo -n "resource not available"		;;
	11 )	echo -n "server-start"		;;
	12 )	echo -n "server-ipc"		;;
	* )	echo -n "???"		;;
	esac
}

#*******************************************************************************
# Function:	add_spaces
#*******************************************************************************
function add_spaces()
{
	local field_width=$1
	local parm=$2

	local parm_size=${#parm}
	let needed_space=$field_width-$parm_size
	case $needed_space in
	0 )	;;
	1 )	echo -n " "	;;
	2 )	echo -n "  "	;;
	3 )	echo -n "   "	;;
	4 )	echo -n "    "	;;
	5 )	echo -n "     "	;;
	6 )	echo -n "      "	;;
	7 )	echo -n "       "	;;
	8 )	echo -n "        "	;;
	9 )	echo -n "         "	;;
	* )	echo -n "          "	;;
	esac
}

#*******************************************************************************
#*******************************************************************************
# MAIN
#*******************************************************************************
#*******************************************************************************
box_select="$(list_all_boxmodes)"
test_select=0
never_fail=0

while [[ $# > 0 ]] ; do
	key="$1"

	case $key in
		-b|--box )
			tmp_select="$2"
			box_select="${tmp_select//,/ }"
			shift # past argument
			;;
		-t|--test )
			test_select="$2"
			shift # past argument
			;;
		-a|--all )
			never_fail=1
			;;
		-h|--help|-?|* )
			print_help
			;;
	esac
	shift # past argument or value
done

rm -f $SERVER_ERROR_LOG
rm -f $SUMMARY_LOG
stop_server

for box_mode in ${box_select} ; do
	run_boxmode_test $box_mode $test_select
	ret_code=$?
	case $ret_code in
	0 )
		echo ">>> PASS boxmode $box_mode case $test_select <<<"
		if [ "$never_fail" == "1" ] ; then
			echo "Boxmode $(add_spaces 5 $box_mode)$box_mode	Case $test_select	PASS" >> $SUMMARY_LOG
		fi
		;;
	2 )
		echo ">>> N/A  boxmode $box_mode case $test_select <<<"
		if [ "$never_fail" == "1" ] ; then
			echo "Boxmode $(add_spaces 5 $box_mode)$box_mode	Case $test_select	 N/A($(get_ret_code_string $ret_code))" >> $SUMMARY_LOG
		fi
		;;
	* )
		echo ">>> FAIL boxmode $box_mode case $test_select ($(get_ret_code_string $ret_code))<<<"
		if [ "$never_fail" == 0 ] ; then
			exit 1
		else
			echo "Boxmode $(add_spaces 5 $box_mode)$box_mode	Case $test_select	FAIL($(get_ret_code_string $ret_code))" >> $SUMMARY_LOG
		fi
		;;
	esac
done
if [ "$never_fail" == "1" ] ; then
	echo ""
	echo "Test Summary"
	echo "============"
	cat $SUMMARY_LOG
fi

exit 0