#!/bin/bash
#
# Script to compress archived build LOGS folder (run only by hwnbuild build user)
#
# Usage: $0 <notify-flag> <verbose-flag>
#
# $Id$
#
# SVN: $HeadURL$
#
notify=$1
verbose=$2
currentyear=`date '+%Y'`
admin="hnd-software-scm-list@broadcom.com"
elog=/home/hwnbuild/tmp/compress/compress.errors
slog=/home/hwnbuild/tmp/compress/compress.size
rm -f $elog

FIND=/tools/bin/find
GZIP=/tools/bin/gzip
DATE=/bin/date
WC=/usr/bin/wc
DU=/tools/oss/bin/du

DEFAULT_MIN_DAYS=0
DEFAULT_MAX_DAYS=3

MIN_DAYS=$DEFAULT_MIN_DAYS
MAX_DAYS=$DEFAULT_MAX_DAYS

for platform in linux macos netbsd window
do
	[ $verbose ] && echo "START: Platform: [$platform]"

	cd /projects/hnd/swbuild/build_${platform}/LOGS &&
	$FIND * -maxdepth 2 -mindepth 2 -type d \
		-mtime +$MIN_DAYS -mtime -$MAX_DAYS | \
		xargs -t -l1 $GZIP -9 -r

	rc=$?
	if [ "$rc" != "0" ]; then
		notify=yes
		echo "`date`: ERROR: [ec=$rc;pf=$platform] compress log failed" >> $elog
	fi
	[ $verbose ] && echo "END  : Platform: [$platform]"
done
echo "`date`: All LOGS compressions done"

[ $notify ] && [ -s "$elog" ] && mail -s "Compress Log Errors on `date`" $admin < $elog

${DATE}                                        >> $slog
${WC} -l /var/tmp/hnd/compress*.log            >> $slog 2>&1
${DU} -sch /projects/hnd/swbuild/build_*/LOGS/ >> $slog 2>&1
