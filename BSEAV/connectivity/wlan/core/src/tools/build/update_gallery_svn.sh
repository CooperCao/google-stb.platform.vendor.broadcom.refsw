#!/bin/sh
#
# $Id$
#

PATH=/projects/hnd/tools/bin:/bin:/usr/bin:/tools/bin
export PATH

ADMIN="hnd-software-scm-list@broadcom.com"
TS=`date '+%Y_%m_%d__%H_%M_%S'`

LDIR=/projects/tmp/hwnbuild/
LOG=$LDIR/gallery_svn_bsub_$TS.log
GDIR=/projects/hnd_software_misc/swdata/gallery/src

# Sparse file used for gallery svn-gallery-trunk.sparse
UPDATE_SCRIPT=/projects/hnd_software/gallery/svn_update_session.sh

# Catch if update failed to create lock
ERRORSTRINGS="svn: Working copy '.' locked"
# Catch if update exits with non-zero error code
ERRORSTRINGS="${ERRORSTRINGS}\|svn: run 'svn cleanup' to remove locks"

touch $LOG
bsub -o $LOG -q sj-hnd $UPDATE_SCRIPT

ls $LDIR || exit 2
cd $LDIR || exit 2

# Search for any pending gallery update errors and alert admin users
if grep -qi \""${ERRORSTRINGS}\"" `find . -mtime +1 -print`
then
	echo "ERROR: SVN Gallery update failed on $TS"
	echo "ERROR: Notifying admin user $ADMIN"
	grep -i \""${ERRORSTRINGS}\"" `find . -mtime +1 -print` | \
		mailx -s "SVN Gallery update failed on $TS" $ADMIN
fi

find . -mtime +2 -type f -exec /bin/rm -f {} \;
