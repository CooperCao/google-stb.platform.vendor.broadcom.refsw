#!/bin/sh
# This script is used to watch for python processes and save data
# about them. There are reports of runaway python processes on LSF
# servers and this is intended to find and analyze them before the
# OOM killer takes them out. It will run forever until killed and
# invocations after the first are ignored. We depend on IT cleanup
# scripts to remove /tmp/pytrace 10 days after last use.

cd /tmp
umask 002
dir=/tmp/pytrace
if [[ -d $dir ]]; then
    [[ "$1" != "kill" ]] || (kill $(<$dir/pid) && /bin/rm $dir/pid)
    [[ "$2" != "rm" ]] || /bin/rm -rf $dir
    exit 0
fi
delay=${1:-900}
ceiling=${2:-20}
mailto=${3:-hnd-software-scm-list.pdl@broadcom.com}
/bin/mkdir $dir || exit 2
exec 1>$dir/stdout 2>$dir/stderr
echo $$ > $dir/pid
while test -d $dir; do
    for pid in $(/usr/bin/pgrep python); do
	test -f /proc/$pid/oom_score || continue
	score=$(</proc/$pid/oom_score)
	pdir=$dir/$pid
	/bin/mkdir -p $pdir
	/usr/bin/pstree -alp $pid > $pdir/pstree.out
	echo $score >> $pdir/oom_score
	if [[ $score -ge $ceiling ]]; then
	    host=$(/bin/hostname)
	    echo "Check $host:$pdir (oom_score=$score)" |\
		/bin/mailx -s "Python OOM risk on $host" $mailto
	fi
	/bin/tar -C /proc/$pid --exclude='*/net/*' --exclude=clear_refs -czf $pdir/proc.tar.gz .
	(/bin/date; /bin/ps -lf $pid) >> $pdir/ps.out
	/usr/bin/timeout 10 /usr/bin/strace -q -o $pdir/strace.out -p $pid
    done
    sleep $delay
done
