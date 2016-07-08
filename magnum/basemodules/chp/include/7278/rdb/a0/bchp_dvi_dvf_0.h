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
 * Date:           Generated on               Mon Mar 21 13:44:46 2016
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

#ifndef BCHP_DVI_DVF_0_H__
#define BCHP_DVI_DVF_0_H__

/***************************************************************************
 *DVI_DVF_0 - DVI Frontend Sync Insertion 0
 ***************************************************************************/
#define BCHP_DVI_DVF_0_DVF_REV_ID                0x000e4b00 /* [RO] Revision ID register */
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL         0x000e4b04 /* [RW] Blank Control register */
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL          0x000e4b08 /* [RW] Flag Width Control register */
#define BCHP_DVI_DVF_0_DVF_CONFIG                0x000e4b0c /* [RW] Configuration register */
#define BCHP_DVI_DVF_0_DVF_VALUES                0x000e4b10 /* [RW] Misc. Value register */
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS            0x000e4b14 /* [RW] DVF's BVB Status Register */
#define BCHP_DVI_DVF_0_DVF_VBI_BLANK             0x000e4b18 /* [RW] VBI Blank Register */

/***************************************************************************
 *DVF_REV_ID - Revision ID register
 ***************************************************************************/
/* DVI_DVF_0 :: DVF_REV_ID :: reserved0 [31:16] */
#define BCHP_DVI_DVF_0_DVF_REV_ID_reserved0_MASK                   0xffff0000
#define BCHP_DVI_DVF_0_DVF_REV_ID_reserved0_SHIFT                  16

/* DVI_DVF_0 :: DVF_REV_ID :: REVISION_ID [15:00] */
#define BCHP_DVI_DVF_0_DVF_REV_ID_REVISION_ID_MASK                 0x0000ffff
#define BCHP_DVI_DVF_0_DVF_REV_ID_REVISION_ID_SHIFT                0
#define BCHP_DVI_DVF_0_DVF_REV_ID_REVISION_ID_DEFAULT              0x00005000

/***************************************************************************
 *DVF_BLANK_CONTROL - Blank Control register
 ***************************************************************************/
/* DVI_DVF_0 :: DVF_BLANK_CONTROL :: reserved0 [31:28] */
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_reserved0_MASK            0xf0000000
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_reserved0_SHIFT           28

/* DVI_DVF_0 :: DVF_BLANK_CONTROL :: LINE [27:15] */
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_LINE_MASK                 0x0fff8000
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_LINE_SHIFT                15
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_LINE_DEFAULT              0x00000005

/* DVI_DVF_0 :: DVF_BLANK_CONTROL :: PIXEL [14:02] */
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_PIXEL_MASK                0x00007ffc
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_PIXEL_SHIFT               2
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_PIXEL_DEFAULT             0x00000005

/* DVI_DVF_0 :: DVF_BLANK_CONTROL :: MODE [01:00] */
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_MODE_MASK                 0x00000003
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_MODE_SHIFT                0
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_MODE_DEFAULT              0x00000000
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_MODE_REG                  1
#define BCHP_DVI_DVF_0_DVF_BLANK_CONTROL_MODE_DELAYED_REG          2

/***************************************************************************
 *DVF_FLAG_CONTROL - Flag Width Control register
 ***************************************************************************/
/* DVI_DVF_0 :: DVF_FLAG_CONTROL :: reserved0 [31:15] */
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_reserved0_MASK             0xffff8000
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_reserved0_SHIFT            15

/* DVI_DVF_0 :: DVF_FLAG_CONTROL :: DE_POL [14:14] */
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_DE_POL_MASK                0x00004000
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_DE_POL_SHIFT               14
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_DE_POL_DEFAULT             0x00000000

/* DVI_DVF_0 :: DVF_FLAG_CONTROL :: DE_WIDTH [13:10] */
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_DE_WIDTH_MASK              0x00003c00
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_DE_WIDTH_SHIFT             10
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_DE_WIDTH_DEFAULT           0x00000000

/* DVI_DVF_0 :: DVF_FLAG_CONTROL :: V_POL [09:09] */
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_V_POL_MASK                 0x00000200
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_V_POL_SHIFT                9
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_V_POL_DEFAULT              0x00000000

/* DVI_DVF_0 :: DVF_FLAG_CONTROL :: V_WIDTH [08:05] */
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_V_WIDTH_MASK               0x000001e0
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_V_WIDTH_SHIFT              5
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_V_WIDTH_DEFAULT            0x00000000

/* DVI_DVF_0 :: DVF_FLAG_CONTROL :: H_POL [04:04] */
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_H_POL_MASK                 0x00000010
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_H_POL_SHIFT                4
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_H_POL_DEFAULT              0x00000000

/* DVI_DVF_0 :: DVF_FLAG_CONTROL :: H_WIDTH [03:00] */
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_H_WIDTH_MASK               0x0000000f
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_H_WIDTH_SHIFT              0
#define BCHP_DVI_DVF_0_DVF_FLAG_CONTROL_H_WIDTH_DEFAULT            0x00000000

/***************************************************************************
 *DVF_CONFIG - Configuration register
 ***************************************************************************/
/* DVI_DVF_0 :: DVF_CONFIG :: reserved0 [31:28] */
#define BCHP_DVI_DVF_0_DVF_CONFIG_reserved0_MASK                   0xf0000000
#define BCHP_DVI_DVF_0_DVF_CONFIG_reserved0_SHIFT                  28

/* DVI_DVF_0 :: DVF_CONFIG :: DE_MASK [27:26] */
#define BCHP_DVI_DVF_0_DVF_CONFIG_DE_MASK_MASK                     0x0c000000
#define BCHP_DVI_DVF_0_DVF_CONFIG_DE_MASK_SHIFT                    26
#define BCHP_DVI_DVF_0_DVF_CONFIG_DE_MASK_DEFAULT                  0x00000000
#define BCHP_DVI_DVF_0_DVF_CONFIG_DE_MASK_UNMASKED                 3
#define BCHP_DVI_DVF_0_DVF_CONFIG_DE_MASK_VBI_LINE_MASKED_INV      2
#define BCHP_DVI_DVF_0_DVF_CONFIG_DE_MASK_VBI_LINE_MASKED          1
#define BCHP_DVI_DVF_0_DVF_CONFIG_DE_MASK_V_AV_MASKED              0

/* DVI_DVF_0 :: DVF_CONFIG :: BVB_LINE_REMOVE_BOTTOM [25:21] */
#define BCHP_DVI_DVF_0_DVF_CONFIG_BVB_LINE_REMOVE_BOTTOM_MASK      0x03e00000
#define BCHP_DVI_DVF_0_DVF_CONFIG_BVB_LINE_REMOVE_BOTTOM_SHIFT     21
#define BCHP_DVI_DVF_0_DVF_CONFIG_BVB_LINE_REMOVE_BOTTOM_DEFAULT   0x00000000

/* DVI_DVF_0 :: DVF_CONFIG :: BVB_LINE_REMOVE_TOP [20:16] */
#define BCHP_DVI_DVF_0_DVF_CONFIG_BVB_LINE_REMOVE_TOP_MASK         0x001f0000
#define BCHP_DVI_DVF_0_DVF_CONFIG_BVB_LINE_REMOVE_TOP_SHIFT        16
#define BCHP_DVI_DVF_0_DVF_CONFIG_BVB_LINE_REMOVE_TOP_DEFAULT      0x00000000

/* DVI_DVF_0 :: DVF_CONFIG :: reserved_for_eco1 [15:14] */
#define BCHP_DVI_DVF_0_DVF_CONFIG_reserved_for_eco1_MASK           0x0000c000
#define BCHP_DVI_DVF_0_DVF_CONFIG_reserved_for_eco1_SHIFT          14
#define BCHP_DVI_DVF_0_DVF_CONFIG_reserved_for_eco1_DEFAULT        0x00000000

/* DVI_DVF_0 :: DVF_CONFIG :: DOWNSAMPLE2X [13:13] */
#define BCHP_DVI_DVF_0_DVF_CONFIG_DOWNSAMPLE2X_MASK                0x00002000
#define BCHP_DVI_DVF_0_DVF_CONFIG_DOWNSAMPLE2X_SHIFT               13
#define BCHP_DVI_DVF_0_DVF_CONFIG_DOWNSAMPLE2X_DEFAULT             0x00000000
#define BCHP_DVI_DVF_0_DVF_CONFIG_DOWNSAMPLE2X_OFF                 0
#define BCHP_DVI_DVF_0_DVF_CONFIG_DOWNSAMPLE2X_ON                  1

/* DVI_DVF_0 :: DVF_CONFIG :: PASSTHROUGH_COUNT [12:03] */
#define BCHP_DVI_DVF_0_DVF_CONFIG_PASSTHROUGH_COUNT_MASK           0x00001ff8
#define BCHP_DVI_DVF_0_DVF_CONFIG_PASSTHROUGH_COUNT_SHIFT          3
#define BCHP_DVI_DVF_0_DVF_CONFIG_PASSTHROUGH_COUNT_DEFAULT        0x00000000

/* DVI_DVF_0 :: DVF_CONFIG :: VBI_PREFERRED [02:02] */
#define BCHP_DVI_DVF_0_DVF_CONFIG_VBI_PREFERRED_MASK               0x00000004
#define BCHP_DVI_DVF_0_DVF_CONFIG_VBI_PREFERRED_SHIFT              2
#define BCHP_DVI_DVF_0_DVF_CONFIG_VBI_PREFERRED_DEFAULT            0x00000000
#define BCHP_DVI_DVF_0_DVF_CONFIG_VBI_PREFERRED_OFF                0
#define BCHP_DVI_DVF_0_DVF_CONFIG_VBI_PREFERRED_ON                 1

/* DVI_DVF_0 :: DVF_CONFIG :: VBI_ENABLE [01:01] */
#define BCHP_DVI_DVF_0_DVF_CONFIG_VBI_ENABLE_MASK                  0x00000002
#define BCHP_DVI_DVF_0_DVF_CONFIG_VBI_ENABLE_SHIFT                 1
#define BCHP_DVI_DVF_0_DVF_CONFIG_VBI_ENABLE_DEFAULT               0x00000001
#define BCHP_DVI_DVF_0_DVF_CONFIG_VBI_ENABLE_OFF                   0
#define BCHP_DVI_DVF_0_DVF_CONFIG_VBI_ENABLE_ON                    1

/* DVI_DVF_0 :: DVF_CONFIG :: UPSAMPLE2X [00:00] */
#define BCHP_DVI_DVF_0_DVF_CONFIG_UPSAMPLE2X_MASK                  0x00000001
#define BCHP_DVI_DVF_0_DVF_CONFIG_UPSAMPLE2X_SHIFT                 0
#define BCHP_DVI_DVF_0_DVF_CONFIG_UPSAMPLE2X_DEFAULT               0x00000000
#define BCHP_DVI_DVF_0_DVF_CONFIG_UPSAMPLE2X_OFF                   0
#define BCHP_DVI_DVF_0_DVF_CONFIG_UPSAMPLE2X_ON                    1

/***************************************************************************
 *DVF_VALUES - Misc. Value register
 ***************************************************************************/
/* DVI_DVF_0 :: DVF_VALUES :: CH0_VBI_OFFSET [31:24] */
#define BCHP_DVI_DVF_0_DVF_VALUES_CH0_VBI_OFFSET_MASK              0xff000000
#define BCHP_DVI_DVF_0_DVF_VALUES_CH0_VBI_OFFSET_SHIFT             24
#define BCHP_DVI_DVF_0_DVF_VALUES_CH0_VBI_OFFSET_DEFAULT           0x00000000

/* DVI_DVF_0 :: DVF_VALUES :: CH2_BLANK [23:16] */
#define BCHP_DVI_DVF_0_DVF_VALUES_CH2_BLANK_MASK                   0x00ff0000
#define BCHP_DVI_DVF_0_DVF_VALUES_CH2_BLANK_SHIFT                  16
#define BCHP_DVI_DVF_0_DVF_VALUES_CH2_BLANK_DEFAULT                0x00000080

/* DVI_DVF_0 :: DVF_VALUES :: CH1_BLANK [15:08] */
#define BCHP_DVI_DVF_0_DVF_VALUES_CH1_BLANK_MASK                   0x0000ff00
#define BCHP_DVI_DVF_0_DVF_VALUES_CH1_BLANK_SHIFT                  8
#define BCHP_DVI_DVF_0_DVF_VALUES_CH1_BLANK_DEFAULT                0x00000080

/* DVI_DVF_0 :: DVF_VALUES :: CH0_BLANK [07:00] */
#define BCHP_DVI_DVF_0_DVF_VALUES_CH0_BLANK_MASK                   0x000000ff
#define BCHP_DVI_DVF_0_DVF_VALUES_CH0_BLANK_SHIFT                  0
#define BCHP_DVI_DVF_0_DVF_VALUES_CH0_BLANK_DEFAULT                0x00000010

/***************************************************************************
 *DVF_BVB_STATUS - DVF's BVB Status Register
 ***************************************************************************/
/* DVI_DVF_0 :: DVF_BVB_STATUS :: reserved0 [31:09] */
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_reserved0_MASK               0xfffffe00
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_reserved0_SHIFT              9

/* DVI_DVF_0 :: DVF_BVB_STATUS :: LATE_BF_SOURCE [08:08] */
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_LATE_BF_SOURCE_MASK          0x00000100
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_LATE_BF_SOURCE_SHIFT         8

/* DVI_DVF_0 :: DVF_BVB_STATUS :: LONG_BF_SOURCE [07:07] */
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_LONG_BF_SOURCE_MASK          0x00000080
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_LONG_BF_SOURCE_SHIFT         7

/* DVI_DVF_0 :: DVF_BVB_STATUS :: SHORT_BF_SOURCE [06:06] */
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_SHORT_BF_SOURCE_MASK         0x00000040
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_SHORT_BF_SOURCE_SHIFT        6

/* DVI_DVF_0 :: DVF_BVB_STATUS :: LATE_TF_SOURCE [05:05] */
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_LATE_TF_SOURCE_MASK          0x00000020
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_LATE_TF_SOURCE_SHIFT         5

/* DVI_DVF_0 :: DVF_BVB_STATUS :: LONG_TF_SOURCE [04:04] */
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_LONG_TF_SOURCE_MASK          0x00000010
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_LONG_TF_SOURCE_SHIFT         4

/* DVI_DVF_0 :: DVF_BVB_STATUS :: SHORT_TF_SOURCE [03:03] */
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_SHORT_TF_SOURCE_MASK         0x00000008
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_SHORT_TF_SOURCE_SHIFT        3

/* DVI_DVF_0 :: DVF_BVB_STATUS :: LATE_LINE [02:02] */
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_LATE_LINE_MASK               0x00000004
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_LATE_LINE_SHIFT              2

/* DVI_DVF_0 :: DVF_BVB_STATUS :: LONG_LINE [01:01] */
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_LONG_LINE_MASK               0x00000002
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_LONG_LINE_SHIFT              1

/* DVI_DVF_0 :: DVF_BVB_STATUS :: SHORT_LINE [00:00] */
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_SHORT_LINE_MASK              0x00000001
#define BCHP_DVI_DVF_0_DVF_BVB_STATUS_SHORT_LINE_SHIFT             0

/***************************************************************************
 *DVF_VBI_BLANK - VBI Blank Register
 ***************************************************************************/
/* DVI_DVF_0 :: DVF_VBI_BLANK :: OFFSET [31:00] */
#define BCHP_DVI_DVF_0_DVF_VBI_BLANK_OFFSET_MASK                   0xffffffff
#define BCHP_DVI_DVF_0_DVF_VBI_BLANK_OFFSET_SHIFT                  0
#define BCHP_DVI_DVF_0_DVF_VBI_BLANK_OFFSET_DEFAULT                0x00000000

#endif /* #ifndef BCHP_DVI_DVF_0_H__ */

/* End of File */
