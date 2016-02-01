#!/bin/bash
#############################################################################
#    (c)2010 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
# This script clears out the .strtab section of an ELF file (without removing it).
# This effectively hides all symbol strings without affecting the ability to relocate the file.
#
# Revision History:
#
# $brcm_Log: $
#
#############################################################################
READELF=$2
#
# Declare our variables as integers
#
declare -i pReverse_Jumptable
declare -i B_Forward_Jumptable
declare -i start
declare -i end
#
# Extract offset and size of .strtab section from the elf file (in hex)
#
offset=`$READELF -e $1 | grep "\.strtab" | awk '{print toupper($5)}'`
size=`$READELF -e $1 | grep "\.strtab" | awk '{print toupper($6)}'`
#
# Convert to decimal
#
offset=`echo "ibase=16;" $offset |bc`
size=`echo "ibase=16;" $size |bc`

#
# NFE specific: Find our two key symbols and preserve them
#
str=`strings -a -td $1 | grep pReverse_Jumptable`
if [ -z "$str" ]; then
echo ERROR: Cannot find pReverse_Jumptable in $1
exit 1
else
let pReverse_Jumptable=`echo $str|cut --delimiter=" " --fields=1`
fi

str=`strings -a -td  $1 | grep B_Forward_Jumptable`
if [ -z "$str" ]; then
echo ERROR: Cannot find B_Forward_Jumptable in $1
exit 1
else
let B_Forward_Jumptable=`echo $str|cut --delimiter=" " --fields=1`
fi

#
# If build server has dd conv=nocreat feature, we do this the easy way
#
if dd --help | grep -q nocreat ; then
   # Now zero out the section in the file
   #
   dd if=/dev/zero of=$1 bs=1 seek=$offset count=$size conv=notrunc,nocreat 2>/dev/null
   #
   # NFE: Put our two symbols back
   echo -n pReverse_Jumptable | dd of=$1 bs=1 seek=$pReverse_Jumptable  count=18 conv=notrunc,nocreat  2>/dev/null
   echo -n B_Forward_Jumptable  | dd of=$1 bs=1 seek=$B_Forward_Jumptable count=19 conv=notrunc,nocreat 2>/dev/null
else
# Hard way algorithm - We create three temp files and cat them together
# First temp file contains all data upto the strtab section start
# Second temp file contains strtab section start zero'ed up to our first symbol(either pReverse_Jumptable or B_Forward_Jumptable),
# the first symbol, zeros to the next symbol, the next symbol, zeros to the end of the section
# Third temp file is data from end of the strtab section to EOF
#
   dd if=$1 of=tmp1 bs=1 count=$offset 2>/dev/null
   if [ $pReverse_Jumptable -gt $B_Forward_Jumptable ] ; then
      let start=$B_Forward_Jumptable-$offset
      dd if=/dev/zero of=tmp2 bs=1 count=$start 2>/dev/null
      echo -n B_Forward_Jumptable>>tmp2
      let start=$pReverse_Jumptable-$B_Forward_Jumptable-19
      dd if=/dev/zero bs=1 count=$start >> tmp2 2>/dev/null
      echo -n pReverse_Jumptable>>tmp2
      let start=$pReverse_Jumptable+18
   else
      let start=$pReverse_Jumptable-$offset
      dd if=/dev/zero of=tmp2 bs=1 count=$start 2>/dev/null
      echo -n pReverse_Jumptable>>tmp2
      let start=$B_Forward_Jumptable-pReverse_Jumptable-18
      dd if=/dev/zero bs=1 count=$start >> tmp2 2>/dev/null
      echo -n B_Forward_Jumptable>>tmp2
      let start=$B_Forward_Jumptable+19
   fi
   let start=($offset+$size)-$start
   dd if=/dev/zero bs=1 count=$start >> tmp2 2>/dev/null
   let start=$offset+$size
   dd if=$1 of=tmp3 bs=1 skip=$start 2>/dev/null
   cat tmp1 tmp2 tmp3 > $1
   rm -f tmp1 tmp2 tmp3
fi

