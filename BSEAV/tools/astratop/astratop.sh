# stop any currently running version

RUNNING=`ps -eaf | grep astratop_server | grep -v grep | awk '{print $2;}'`
if [[ -z "$RUNNING" ]]; then
  echo "astratop server is not currently running."
else
  echo "astratop server is currently running as pid $RUNNING; killing it."
  kill -9 $RUNNING
fi

DATE=`date +"%Y%m%d_%H%M%S"`

./astratop_server > /tmp/astratop_${DATE}.log 2>&1 &

RUNNING=`ps -eaf | grep astratop_server | grep -v grep | awk '{print $2;}'`
echo "astratop_server is pid: " ${RUNNING}
