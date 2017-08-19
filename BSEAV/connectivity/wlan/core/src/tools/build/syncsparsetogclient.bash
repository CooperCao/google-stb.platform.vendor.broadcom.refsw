#!/bin/bash

function syncSparseToGclient
{
  wdir=/tmp/tmp_$$
  mkdir $wdir
  pushd $wdir
  svn co http://svn.sj.broadcom.com/svn/wlansvn/groups/software/tools/hnd_depot_tools
  svn co --depth files http://svn.sj.broadcom.com/svn/wlansvn/proj/trunk
  svn co http://svn.sj.broadcom.com/svn/wlansvn/components/deps/trunk deps
    pushd trunk
    for i in *.sparse; do
      base=${i%.sparse}
      [[ $base != "appended" ]] || continue
      python ../hnd_depot_tools/hnd_sparse2deps.py -f -r -s $i -d ../deps/$base/DEPS "$@"
     done
    popd
    pushd deps
     svn add *
     svn ci -m "[DEPSUPDATE]: Auto sync sparse to DEPS from cron at `hostname`"
    popd
  popd
  rm -rf $wdir
}
PATH=/tools/bin:$PATH
syncSparseToGclient
