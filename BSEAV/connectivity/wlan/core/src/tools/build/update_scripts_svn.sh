#!/bin/sh
#
# $Id$
#

PATH=/projects/hnd/tools/bin:/tools/bin:/bin:/usr/bin
export PATH

SVNROOT="http://pegasus.syd.broadcom.com/repo/systems/software/tools/auto_merger/sparse_checkout/trunk"
ADMIN="hnd-software-scm-list@broadcom.com"
TS=`date '+%Y_%m_%d__%H_%M_%S'`

LDIR=/projects/tmp/hwnbuild/
LOG=$LDIR/svn_gallery_bsub_$TS.log
GDIR=/projects/hnd_software/gallery-svn/trunk
UPDATE_SCRIPT=/projects/hnd_software/gallery-svn/svn_update_session.sh

# Catch if update failed to create lock
ERRORSTRINGS="svn: Working copy '.' locked"
# Catch if update exits with non-zero error code
ERRORSTRINGS="${ERRORSTRINGS}\|svn: run 'svn cleanup' to remove locks"

touch $LOG
bsub -o $LOG -q sj-hnd $UPDATE_SCRIPT

cd $LDIR

find $LDIR -mtime +2 -name "*svn_gallery*" -print | xargs rm -f 2> /dev/null
