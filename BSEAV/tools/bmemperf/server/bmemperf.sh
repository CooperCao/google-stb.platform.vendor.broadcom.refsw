export LD_LIBRARY_PATH=.:

# stop any currently running version

RUNNING=`ps -eaf | grep bmemperf_server | grep -v grep | awk '{print $2;}'`
if [[ -z "$RUNNING" ]]; then
  echo "bmemperf server is not currently running."
else
  echo "bmemperf server is currently running as pid $RUNNING; killing it."
  kill -9 $RUNNING
fi

DATE=`date +"%m%d%Y_%H%M%S"`
TEMPDIR=$B_ANDROID_TEMP

if [[ -z "$TEMPDIR" ]]; then
  ### echo "TEMPDIR is not set"
  ./bmemperf_server > /tmp/bmemperf_${DATE}.log 2>&1 &
else
  ### echo "TEMPDIR is currently set to $TEMPDIR"
  ./bmemperf_server > $TEMPDIR/bmemperf_${DATE}.log 2>&1 &
fi

RUNNING=`ps -eaf | grep bmemperf_server | grep -v grep | awk '{print $2;}'`
echo "bmemperf_server is pid: " ${RUNNING}
