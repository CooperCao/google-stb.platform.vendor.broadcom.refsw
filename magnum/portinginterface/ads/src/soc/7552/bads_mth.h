/***************************************************************************
 *     (c)2003-2010 Broadcom Corporation
 *  
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *   
 *  Except as expressly set forth in the Authorized License,
 *   
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *   
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *  
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 *  ANY LIMITED REMEDY.
 * 
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 ***************************************************************************/


#ifndef __BADS_MTH_H__
#define __BADS_MTH_H__


#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
* Define Math 
****************************************************************************/

/*Function to get twos complement of a 32 bit register*/
#define Twos_Complement32(x) ((uint32_t)((x ^ 0xFFFFFFFF) + 1))

#define POWER2_0 1
#define POWER2_1 2
#define POWER2_2 4
#define POWER2_3 8
#define POWER2_4 16
#define POWER2_5 32
#define POWER2_6 64
#define POWER2_7 128
#define POWER2_8 256
#define POWER2_9 512
#define POWER2_10 1024
#define POWER2_11 2048
#define POWER2_12 4096
#define POWER2_13 8192
#define POWER2_14 16384
#define POWER2_15 32768
#define POWER2_16 65536
#define POWER2_17 131072
#define POWER2_18 262144
#define POWER2_19 524288
#define POWER2_20 1048576
#define POWER2_21 2097152
#define POWER2_22 4194304
#define POWER2_23 8388608
#define POWER2_24 16777216
#define POWER2_25 33554432
#define POWER2_26 67108864
#define POWER2_27 134217728
#define POWER2_28 268435456
#define POWER2_29 536870912
#define POWER2_30 1073741824
#define POWER2_31 2147483648UL



#define POWER2_0_M1 0
#define POWER2_1_M1 1
#define POWER2_2_M1 3
#define POWER2_3_M1 7
#define POWER2_4_M1 15
#define POWER2_5_M1 31
#define POWER2_6_M1 63
#define POWER2_7_M1 127
#define POWER2_8_M1 255
#define POWER2_9_M1 511
#define POWER2_10_M1 1023
#define POWER2_11_M1 2047
#define POWER2_12_M1 4095
#define POWER2_13_M1 8191
#define POWER2_14_M1 16383
#define POWER2_15_M1 32767
#define POWER2_16_M1 65535
#define POWER2_17_M1 131071
#define POWER2_18_M1 262143
#define POWER2_19_M1 524287
#define POWER2_20_M1 1048575
#define POWER2_21_M1 2097151
#define POWER2_22_M1 4194303
#define POWER2_23_M1 8388607
#define POWER2_24_M1 16777215
#define POWER2_25_M1 33554431
#define POWER2_26_M1 67108863
#define POWER2_27_M1 134217727
#define POWER2_28_M1 268435455
#define POWER2_29_M1 536870911
#define POWER2_30_M1 1073741823
#define POWER2_31_M1 2147483647
#define POWER2_32_M1 4294967295UL


#define LOG10_POWER2_0_X5120 0
#define LOG10_POWER2_1_X5120 1541
#define LOG10_POWER2_2_X5120 3083
#define LOG10_POWER2_3_X5120 4624
#define LOG10_POWER2_4_X5120 6165
#define LOG10_POWER2_5_X5120 7706
#define LOG10_POWER2_6_X5120 9248
#define LOG10_POWER2_7_X5120 10789
#define LOG10_POWER2_8_X5120 12330
#define LOG10_POWER2_9_X5120 13871
#define LOG10_POWER2_10_X5120 15413
#define LOG10_POWER2_11_X5120 16954
#define LOG10_POWER2_12_X5120 18495
#define LOG10_POWER2_13_X5120 20037
#define LOG10_POWER2_14_X5120 21578
#define LOG10_POWER2_15_X5120 23119
#define LOG10_POWER2_16_X5120 24660
#define LOG10_POWER2_17_X5120 26202
#define LOG10_POWER2_18_X5120 27743
#define LOG10_POWER2_19_X5120 29284
#define LOG10_POWER2_20_X5120 30825
#define LOG10_POWER2_21_X5120 32367
#define LOG10_POWER2_22_X5120 33908
#define LOG10_POWER2_23_X5120 35449
#define LOG10_POWER2_24_X5120 36991
#define LOG10_POWER2_25_X5120 38532
#define LOG10_POWER2_26_X5120 40073
#define LOG10_POWER2_27_X5120 41614
#define LOG10_POWER2_28_X5120 43156
#define LOG10_POWER2_29_X5120 44697
#define LOG10_POWER2_30_X5120 46238
#define LOG10_POWER2_31_X5120 47779
#define LOG10_POWER2_32_X5120 49321  



#ifdef __cplusplus
}
#endif
 
#endif



