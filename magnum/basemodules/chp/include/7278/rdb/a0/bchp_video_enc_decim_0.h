/********************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Mon Mar 21 13:44:44 2016
 *                 Full Compile MD5 Checksum  48e7e549bb13082ab30187cb156f35ed
 *                     (minus title and desc)
 *                 MD5 Checksum               949df837b98c31b52074d06d129f7b79
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     880
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_VIDEO_ENC_DECIM_0_H__
#define BCHP_VIDEO_ENC_DECIM_0_H__

/***************************************************************************
 *VIDEO_ENC_DECIM_0 - VEC Decimator  Controls
 ***************************************************************************/
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_REV_ID      0x000e6600 /* [RO] Decimator Revision ID register */
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL     0x000e6608 /* [RW] Control Register for decimator */

/***************************************************************************
 *DECIM_REV_ID - Decimator Revision ID register
 ***************************************************************************/
/* VIDEO_ENC_DECIM_0 :: DECIM_REV_ID :: reserved0 [31:16] */
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_REV_ID_reserved0_MASK         0xffff0000
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_REV_ID_reserved0_SHIFT        16

/* VIDEO_ENC_DECIM_0 :: DECIM_REV_ID :: REVISION_ID [15:00] */
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_REV_ID_REVISION_ID_MASK       0x0000ffff
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_REV_ID_REVISION_ID_SHIFT      0
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_REV_ID_REVISION_ID_DEFAULT    0x00004000

/***************************************************************************
 *DECIM_CONTROL - Control Register for decimator
 ***************************************************************************/
/* VIDEO_ENC_DECIM_0 :: DECIM_CONTROL :: reserved0 [31:13] */
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_reserved0_MASK        0xffffe000
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_reserved0_SHIFT       13

/* VIDEO_ENC_DECIM_0 :: DECIM_CONTROL :: PASSTHROUGH_COUNT [12:03] */
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_PASSTHROUGH_COUNT_MASK 0x00001ff8
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_PASSTHROUGH_COUNT_SHIFT 3
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_PASSTHROUGH_COUNT_DEFAULT 0x00000000

/* VIDEO_ENC_DECIM_0 :: DECIM_CONTROL :: DECIMATE_RATIO [02:02] */
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_RATIO_MASK   0x00000004
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_RATIO_SHIFT  2
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_RATIO_DEFAULT 0x00000001
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_RATIO_BY2    1
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_RATIO_BY4    0

/* VIDEO_ENC_DECIM_0 :: DECIM_CONTROL :: DECIMATE_SAMPLING_EN [01:01] */
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_SAMPLING_EN_MASK 0x00000002
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_SAMPLING_EN_SHIFT 1
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_SAMPLING_EN_DEFAULT 0x00000000
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_SAMPLING_EN_ON 1
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_SAMPLING_EN_OFF 0

/* VIDEO_ENC_DECIM_0 :: DECIM_CONTROL :: DECIMATE_FILTER_EN [00:00] */
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_FILTER_EN_MASK 0x00000001
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_FILTER_EN_SHIFT 0
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_FILTER_EN_DEFAULT 0x00000000
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_FILTER_EN_ON 1
#define BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL_DECIMATE_FILTER_EN_OFF 0

#endif /* #ifndef BCHP_VIDEO_ENC_DECIM_0_H__ */

/* End of File */
