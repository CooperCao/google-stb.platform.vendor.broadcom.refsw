#!/bin/bash
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# Creates an open license tarball
#
#
# <<Broadcom-WL-IPTag/Proprietary:.*>>
#
# $Id$
#

FILELIST="gpl-filelist.txt"
FILELISTTEMP="gpl-filelist-temp.txt"

#if defined(WLSRC) || defined(NASSRC)
RELEASE=no
#else
RELEASE=yes
#endif

#if defined(WLSRC) || defined(NASSRC)

# do not redistribute this package under any circumstances
if [ "$RELEASE" = "no" ] ; then
cat <<EOF
This package contains UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom
Corporation; the contents of this package may not be disclosed to
third parties, copied or duplicated in any form, in whole or in part,
without the prior written permission of Broadcom Corporation.
EOF
exit 0
fi

#else

usage ()
{
    echo "usage: $0 [open release]"
    exit 1
}

[ "${#*}" = "0" ] && usage

echo "cat ${FILELIST} | sed -e 's/[[:space:]]*$//g' > ${FILELISTTEMP}"
cat ${FILELIST} | sed -e 's/[[:space:]]*$//g' > ${FILELISTTEMP}
if ! diff ${FILELIST} ${FILELISTTEMP} > /dev/null 2>&1; then
   echo "WARNING: "
   echo "WARNING: Fix trailing whitespace in following entries"
   echo "WARNING: in ${FILELIST} file (ignored)"
   echo "WARNING: "
   diff ${FILELIST} ${FILELISTTEMP} | grep "^>"
fi

# create open license tarball
echo -e "\ntar czf \"$1\" -T ${FILELISTTEMP} --ignore-failed-read --exclude=*/CVS --exclude=*/.svn\n"
tar czf "$1" -T ${FILELISTTEMP} --ignore-failed-read --exclude=*/CVS --exclude=*/.svn
rm -f ${FILELISTTEMP}

#endif
