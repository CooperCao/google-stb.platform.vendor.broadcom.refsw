#!/bin/bash
#
# Script for running cstyle tool on one or multiple files
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: run_cstyle.sh,v 12.23 2011/02/21 02:57:34 Exp $
#
# Usage:
#   run_cstyle.sh [-s] [-m address] [-f file_list]
#
# To run it on tools/misc/cstyle-filelist.txt
#   run_cstyle.sh
#
# To override the list through a file and -f option
#
# To get a summary of just passed/failed use -s option
#
# To get email provide  email address with -m option
#

# Temporary change to get underlying scripts working away from cron launch
if [ -f "/home/hwnbuild/.bash_profile" ]; then
	. /home/hwnbuild/.bash_profile
fi

MAILTO=
MAILCC=
CSTYLE_FILE_LIST=src/tools/misc/cstyle-filelist.txt
SUMMARY=0
tool_name=src/tools/misc/cstyle
svn_who=src/tools/misc/svnwho

# Usage
usage ()
{
cat <<EOF
Usage: $0 [-s] [-m  address] [-f cstyle_file_list]

-d source_base
	Directory containing sources that need to be checked for cstyle

-f cstyle_file_list
	File containing names and optional arguments for the files

-h
	Show help/usage screen

-m address
	Send mail when complete (default: none).

-s summary
	Just show passed or failed

EOF
    exit 1
}

while getopts 'd:f:m:shv' OPT ; do
	case "$OPT" in
	d)
	    CSTYLE_SRC_BASE=${OPTARG}
	    ;;
	f)
	    CSTYLE_FILE_LIST=${OPTARG}
	    ;;
	m)
	    MAILTO=${OPTARG}
	    ;;
	s)
	    SUMMARY=1
	    ;;
	h)
	    usage
	    ;;
	v)
	    verbose=1
	    dbg=1
	    ;;
	?)
	    usage
	    ;;
	esac
done

if [ -d "${CSTYLE_SRC_BASE}" ]; then
	cd ${CSTYLE_SRC_BASE}
fi

if [ ! -e ${tool_name} ]
then
	echo ${tool_name} "not found. Please run it from top of work area"
	exit
fi

if [ "$verbose" != "" ]
then
	svn_who="$svn_who -p '/usr/bin:/bin:$PATH'"
fi

err_log=$(mktemp /tmp/cstyle_svn.elog.XXXXXX)
tmp_mail_log=$(mktemp /tmp/cstyle_svn.tlog.XXXXXX)
mail_log=$(mktemp /tmp/cstyle_svn.mlog.XXXXXX)

exec 3> ${err_log}
exec 4> ${mail_log}
exec 5> ${tmp_mail_log}

total=0
failed=0

if [ "${CSTYLE_FILE_LIST}" != "" ]
then
	if [ ! -e ${CSTYLE_FILE_LIST} ]
	then
		echo ${CSTYLE_FILE_LIST} "not found1. Please run it from top of work area"
		exit
	fi
	${tool_name} -f ${CSTYLE_FILE_LIST} >& ${err_log}
	# Iterate through a list of entries in CSTYLE_FILE_LIST
	for entry in `cat ${CSTYLE_FILE_LIST}`
	do
		if [ -e $entry ]
		then
			files=$entry;
			# if entry is a dir, then list all .c/.h files
			if [ -d $entry ]
			then
				files=`ls $entry*.[ch]`
			fi
			# process all .c and .h files
			for file in $files
			do
				cstyled=`grep 'FILE_CSTYLED' $file | wc -l `
				if [ $cstyled -eq 0 ]
				then
					total=$((${total} + 1))
					errors=$(grep -cF $file ${err_log})
					if [ "${errors}" != "0" ]
					then
						failed_files="$failed_files $file"
						failed=$((${failed} + 1))
						tmp_line_num=`grep -F $file ${err_log} | sed 's/[^(]\+(\([^ ]\+\).*/\1/' | sed "s/://g" | cut -d' ' -f2 | awk '{printf "%s ",$1}'`
						file_loc=`wc -l $file | cut -d' ' -f1`
						# tmp_line_num shouldn't be greater file_loc. Debugging which files are producing 
						# this erroneous condition
						for n in $tmp_line_num; do
							if [ "$n" -gt "$file_loc" ]; then
								echo "WARN: cstyle error line ($n) derived is greater than $file loc ($file_loc)"
							fi
						done
						echo -n  "$file" " " >> $mail_log
						svnuser=`${svn_who} -s $file ${tmp_line_num}`
						if [ "$svnuser" != "" ]; then
						   MAILCC="$MAILCC $svnuser"
						fi
						if [ "$verbose" != "" ]
						then
							echo "DBG: $file"
							echo "DBG: Derived svnuser=$svnuser from '${svn_who} -s $file ${tmp_line_num}'"
						fi
						# If svnuser can't be found, derive reason for failure
						if [ "$svnuser" == "" ]
						then
							cstyle_error=`grep -F $file ${err_log}`
							case $cstyle_error in
								*"bad mode: execute bit set"*)
									svnuser="error:execute_bit_set"
									;;
								*"contains CR+LF terminator"*)
									svnuser="error:found_CR+LF_chars"
									;;
								*"missing Broadcom Copyright notice"*)
									svnuser="error:no_brcm_copyright"
									;;
								*)
									svnuser="error:unknown"
									;;
								esac;
						fi #svnuser
						echo ${svnuser} >> $mail_log
					fi #errors
				fi #cstyled
			done #files
		fi #-e entry
	done #entry
fi

perl -e ' my $file_name, $userids;
format STDOUT_TOP=
==== ERROR SUMMARY ====================================================
File Name|Version                               UserIds|Error
------------------------------------------------------------------
.

format = 
@<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
$file_name,                $userids
.

while (<>) {
	($file_name,@userids_list)=split(" ",$_);
	$userids ="";
	foreach $name (grep { !$seen_user{$_}++ } @userids_list) {
	    $userids .= $name." ";
	}
	write ;
} ' $mail_log >> $tmp_mail_log

if [ ${failed} != "0" ]; then
	subject="Nightly cstyle summary (${failed} of ${total} failed) [TRUNK]"
else
	echo "All files passed"
	3>&-
	4>&-
	5>&-
	exit
fi


(
	exec 5>> $tmp_mail_log
	exec 1>&5 2>&5

	if [ "${SUMMARY}" = "0" ]
	then
		echo ""
		echo "==== ERROR DETAILS [`date +'%Y/%m/%d'`] ========================================"
		echo "Trunk Gallery Used: $CSTYLE_SRC_BASE"
		for file in $failed_files
		do
			filerev=$(svn info $file | grep "^Revision: " | \
						awk -F: '{printf "%s", $NF}')
			filetime=$(svn info $file | grep "^Last Changed Date: " | \
						awk '{printf "%s %s", $4, $5}')
			echo ""
			echo "FILE: $file (r$filerev, $filetime)"
			echo ""
			${tool_name} $file
			echo "------------------------------------------------------------"
		done
		echo ""
		echo "TWIKI: http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/CodeStyle#How_to_run_cstyle_and_run_cstyle";
	fi # SUMMARY
)

3>&-
4>&-
5>&-

if [ ${#MAILTO} -ne 0 ]
then
	MAILCC=`echo $MAILCC | sed -e 's/\n/ /g'`
	cat ${tmp_mail_log} | mail -s "${subject}" "${MAILCC} ${MAILTO}"
else
	cat ${tmp_mail_log}
fi

rm -f ${err_log} ${mail_log} ${tmp_mail_log}
