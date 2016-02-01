############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
#
############################################################
#!/bin/bash

# boxmode_nexus_playback.sh
RET=0
if [ $PLATFORM == 97250 ]; then
  list="1 2 3 4 6 7 8";
elif [ $PLATFORM == 97364 ]; then
  list="1 2 6";
elif [ $PLATFORM == 97366 ]; then
  list="1 2 3 4";
elif [ $PLATFORM == 97445 ]; then
  list="1 3 9 12 13 14 1000";
elif [ $PLATFORM == 97252 ]; then
  list="2 4 5 6 10 1001";
else
  list="0";
fi;

for boxmode in $list ; do
  echo
  echo ---------------- begin box mode [$boxmode] ----------------
  echo
  if [ $boxmode != 0 ]; then
    export B_REFSW_BOXMODE=$boxmode;
  fi;
  nexus unittests/utils/playback $1;
  RET=$(($RET+$?))
  echo ---------------- end box mode [$boxmode] returned [$RET] ----------------
done

exit $RET;
