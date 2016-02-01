export LD_LIBRARY_PATH=.:

# stop any currently running version

RUNNING=`ps -eaf | grep bsysperf_server | grep -v grep | awk '{print $2;}'`
if [[ -z "$RUNNING" ]]; then
  echo "bsysperf server is not currently running."
else
  echo "bsysperf server is currently running as pid $RUNNING; killing it."
  kill -9 $RUNNING
fi

DATE=`date +"%m%d%Y_%H%M%S"`
TEMPDIR=$B_ANDROID_TEMP

if [[ -z "$TEMPDIR" ]]; then
  ### echo "TEMPDIR is not set"
  ./bsysperf_server > /tmp/bsysperf_${DATE}.log 2>&1 &
else
  ### echo "TEMPDIR is currently set to $TEMPDIR"
  ./bsysperf_server > $TEMPDIR/bsysperf_${DATE}.log 2>&1 &
fi

RUNNING=`ps -eaf | grep bsysperf_server | grep -v grep | awk '{print $2;}'`
echo "bsysperf_server is pid: " ${RUNNING}
