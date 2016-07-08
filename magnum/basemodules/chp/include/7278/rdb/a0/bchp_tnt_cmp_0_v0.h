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

#ifndef BCHP_TNT_CMP_0_V0_H__
#define BCHP_TNT_CMP_0_V0_H__

/***************************************************************************
 *TNT_CMP_0_V0 - Transient Adjustment Block in Video Compositor 0/Video Intra Surface 0
 ***************************************************************************/
#define BCHP_TNT_CMP_0_V0_REVISION_ID            0x00048800 /* [RO] Revision ID */
#define BCHP_TNT_CMP_0_V0_HW_CONFIGURATION       0x00048804 /* [RO] TN2T HW Configuration */
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING           0x00048814 /* [RW] Visual Effects Demo Setting */
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL            0x00048818 /* [RW] Top Control */
#define BCHP_TNT_CMP_0_V0_SCRATCH_0              0x0004881c /* [RW] Scratch Register */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW  0x00048858 /* [RW] Luma Peaking Input Coring Low Band Gain Offsets */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH 0x0004885c /* [RW] Luma Peaking Input Coring High Band Gain Offsets */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW   0x00048860 /* [RW] Luma Peaking Input Coring Low Band Thresholds */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH  0x00048870 /* [RW] Luma Peaking Input Coring High Band Thresholds */
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE         0x00048880 /* [RW] Luma Peaking Output Coring Control */
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID       0x00048884 /* [RW] Luma Peaking Clip Avoidance Control */
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL            0x00048888 /* [RW] LTI Control (Gain/Core/Avoid/Etc) */
#define BCHP_TNT_CMP_0_V0_LTI_FILTER             0x0004888c /* [RW] LTI Filter Control */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_THR         0x00048890 /* [RW] LTI Input Coring Thresholds */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF        0x000488a0 /* [RW] LTI Input Coring Gain Offsets */
#define BCHP_TNT_CMP_0_V0_CTI_H                  0x000488a4 /* [RW] CTI Horizontal Filter Controls */

/***************************************************************************
 *REVISION_ID - Revision ID
 ***************************************************************************/
/* TNT_CMP_0_V0 :: REVISION_ID :: reserved0 [31:16] */
#define BCHP_TNT_CMP_0_V0_REVISION_ID_reserved0_MASK               0xffff0000
#define BCHP_TNT_CMP_0_V0_REVISION_ID_reserved0_SHIFT              16

/* TNT_CMP_0_V0 :: REVISION_ID :: MAJOR [15:08] */
#define BCHP_TNT_CMP_0_V0_REVISION_ID_MAJOR_MASK                   0x0000ff00
#define BCHP_TNT_CMP_0_V0_REVISION_ID_MAJOR_SHIFT                  8
#define BCHP_TNT_CMP_0_V0_REVISION_ID_MAJOR_DEFAULT                0x00000011

/* TNT_CMP_0_V0 :: REVISION_ID :: MINOR [07:00] */
#define BCHP_TNT_CMP_0_V0_REVISION_ID_MINOR_MASK                   0x000000ff
#define BCHP_TNT_CMP_0_V0_REVISION_ID_MINOR_SHIFT                  0
#define BCHP_TNT_CMP_0_V0_REVISION_ID_MINOR_DEFAULT                0x00000005

/***************************************************************************
 *HW_CONFIGURATION - TN2T HW Configuration
 ***************************************************************************/
/* TNT_CMP_0_V0 :: HW_CONFIGURATION :: reserved0 [31:01] */
#define BCHP_TNT_CMP_0_V0_HW_CONFIGURATION_reserved0_MASK          0xfffffffe
#define BCHP_TNT_CMP_0_V0_HW_CONFIGURATION_reserved0_SHIFT         1

/* TNT_CMP_0_V0 :: HW_CONFIGURATION :: Luma_Present [00:00] */
#define BCHP_TNT_CMP_0_V0_HW_CONFIGURATION_Luma_Present_MASK       0x00000001
#define BCHP_TNT_CMP_0_V0_HW_CONFIGURATION_Luma_Present_SHIFT      0
#define BCHP_TNT_CMP_0_V0_HW_CONFIGURATION_Luma_Present_DEFAULT    0x00000001
#define BCHP_TNT_CMP_0_V0_HW_CONFIGURATION_Luma_Present_SUPPORTED  1
#define BCHP_TNT_CMP_0_V0_HW_CONFIGURATION_Luma_Present_NOT_SUPPORTED 0

/***************************************************************************
 *DEMO_SETTING - Visual Effects Demo Setting
 ***************************************************************************/
/* TNT_CMP_0_V0 :: DEMO_SETTING :: reserved0 [31:17] */
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING_reserved0_MASK              0xfffe0000
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING_reserved0_SHIFT             17

/* TNT_CMP_0_V0 :: DEMO_SETTING :: DEMO_L_R [16:16] */
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING_DEMO_L_R_MASK               0x00010000
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING_DEMO_L_R_SHIFT              16
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING_DEMO_L_R_DEFAULT            0x00000000
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING_DEMO_L_R_LEFT               1
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING_DEMO_L_R_RIGHT              0

/* TNT_CMP_0_V0 :: DEMO_SETTING :: reserved1 [15:13] */
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING_reserved1_MASK              0x0000e000
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING_reserved1_SHIFT             13

/* TNT_CMP_0_V0 :: DEMO_SETTING :: DEMO_BOUNDARY [12:00] */
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING_DEMO_BOUNDARY_MASK          0x00001fff
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING_DEMO_BOUNDARY_SHIFT         0
#define BCHP_TNT_CMP_0_V0_DEMO_SETTING_DEMO_BOUNDARY_DEFAULT       0x00000168

/***************************************************************************
 *TOP_CONTROL - Top Control
 ***************************************************************************/
/* TNT_CMP_0_V0 :: TOP_CONTROL :: reserved0 [31:06] */
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_reserved0_MASK               0xffffffc0
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_reserved0_SHIFT              6

/* TNT_CMP_0_V0 :: TOP_CONTROL :: LPEAK_INCORE_EN [05:05] */
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_LPEAK_INCORE_EN_MASK         0x00000020
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_LPEAK_INCORE_EN_SHIFT        5
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_LPEAK_INCORE_EN_DEFAULT      0x00000000
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_LPEAK_INCORE_EN_DISABLE      0
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_LPEAK_INCORE_EN_ENABLE       1

/* TNT_CMP_0_V0 :: TOP_CONTROL :: DEMO_MODE [04:04] */
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_DEMO_MODE_MASK               0x00000010
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_DEMO_MODE_SHIFT              4
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_DEMO_MODE_DEFAULT            0x00000000
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_DEMO_MODE_DISABLE            0
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_DEMO_MODE_ENABLE             1

/* TNT_CMP_0_V0 :: TOP_CONTROL :: H_WINDOW [03:02] */
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_H_WINDOW_MASK                0x0000000c
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_H_WINDOW_SHIFT               2
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_H_WINDOW_DEFAULT             0x00000000

/* TNT_CMP_0_V0 :: TOP_CONTROL :: UPDATE_SEL [01:01] */
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_UPDATE_SEL_MASK              0x00000002
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_UPDATE_SEL_SHIFT             1
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_UPDATE_SEL_DEFAULT           0x00000000
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_UPDATE_SEL_NORMAL            0
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_UPDATE_SEL_EOP               1

/* TNT_CMP_0_V0 :: TOP_CONTROL :: MASTER_EN [00:00] */
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_MASTER_EN_MASK               0x00000001
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_MASTER_EN_SHIFT              0
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_MASTER_EN_DEFAULT            0x00000000
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_MASTER_EN_DISABLE            0
#define BCHP_TNT_CMP_0_V0_TOP_CONTROL_MASTER_EN_ENABLE             1

/***************************************************************************
 *SCRATCH_0 - Scratch Register
 ***************************************************************************/
/* TNT_CMP_0_V0 :: SCRATCH_0 :: VALUE [31:00] */
#define BCHP_TNT_CMP_0_V0_SCRATCH_0_VALUE_MASK                     0xffffffff
#define BCHP_TNT_CMP_0_V0_SCRATCH_0_VALUE_SHIFT                    0
#define BCHP_TNT_CMP_0_V0_SCRATCH_0_VALUE_DEFAULT                  0x00000000

/***************************************************************************
 *LPEAK_GAINS%i - Luma Peaking Overshoot/Undershoot Gains
 ***************************************************************************/
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_ARRAY_BASE                  0x00048820
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_ARRAY_START                 0
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_ARRAY_END                   3
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_ARRAY_ELEMENT_SIZE          32

/***************************************************************************
 *LPEAK_GAINS%i - Luma Peaking Overshoot/Undershoot Gains
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LPEAK_GAINSi :: reserved0 [31:14] */
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_reserved0_MASK              0xffffc000
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_reserved0_SHIFT             14

/* TNT_CMP_0_V0 :: LPEAK_GAINSi :: GAIN_NEG [13:08] */
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_GAIN_NEG_MASK               0x00003f00
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_GAIN_NEG_SHIFT              8
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_GAIN_NEG_DEFAULT            0x00000000

/* TNT_CMP_0_V0 :: LPEAK_GAINSi :: reserved1 [07:06] */
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_reserved1_MASK              0x000000c0
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_reserved1_SHIFT             6

/* TNT_CMP_0_V0 :: LPEAK_GAINSi :: GAIN_POS [05:00] */
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_GAIN_POS_MASK               0x0000003f
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_GAIN_POS_SHIFT              0
#define BCHP_TNT_CMP_0_V0_LPEAK_GAINSi_GAIN_POS_DEFAULT            0x00000000


/***************************************************************************
 *LPEAK_INCORE_GOFF_LOW - Luma Peaking Input Coring Low Band Gain Offsets
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_LOW :: reserved0 [31:31] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_reserved0_MASK     0x80000000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_reserved0_SHIFT    31

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_LOW :: OFFSET4 [30:24] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_OFFSET4_MASK       0x7f000000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_OFFSET4_SHIFT      24
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_OFFSET4_DEFAULT    0x00000000

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_LOW :: reserved1 [23:23] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_reserved1_MASK     0x00800000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_reserved1_SHIFT    23

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_LOW :: OFFSET3 [22:16] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_OFFSET3_MASK       0x007f0000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_OFFSET3_SHIFT      16
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_OFFSET3_DEFAULT    0x00000000

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_LOW :: reserved2 [15:15] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_reserved2_MASK     0x00008000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_reserved2_SHIFT    15

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_LOW :: OFFSET2 [14:08] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_OFFSET2_MASK       0x00007f00
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_OFFSET2_SHIFT      8
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_OFFSET2_DEFAULT    0x00000000

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_LOW :: reserved3 [07:07] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_reserved3_MASK     0x00000080
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_reserved3_SHIFT    7

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_LOW :: OFFSET1 [06:00] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_OFFSET1_MASK       0x0000007f
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_OFFSET1_SHIFT      0
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_LOW_OFFSET1_DEFAULT    0x00000000

/***************************************************************************
 *LPEAK_INCORE_GOFF_HIGH - Luma Peaking Input Coring High Band Gain Offsets
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_HIGH :: reserved0 [31:31] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_reserved0_MASK    0x80000000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_reserved0_SHIFT   31

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_HIGH :: OFFSET4 [30:24] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_OFFSET4_MASK      0x7f000000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_OFFSET4_SHIFT     24
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_OFFSET4_DEFAULT   0x00000000

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_HIGH :: reserved1 [23:23] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_reserved1_MASK    0x00800000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_reserved1_SHIFT   23

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_HIGH :: OFFSET3 [22:16] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_OFFSET3_MASK      0x007f0000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_OFFSET3_SHIFT     16
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_OFFSET3_DEFAULT   0x00000000

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_HIGH :: reserved2 [15:15] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_reserved2_MASK    0x00008000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_reserved2_SHIFT   15

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_HIGH :: OFFSET2 [14:08] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_OFFSET2_MASK      0x00007f00
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_OFFSET2_SHIFT     8
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_OFFSET2_DEFAULT   0x00000000

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_HIGH :: reserved3 [07:07] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_reserved3_MASK    0x00000080
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_reserved3_SHIFT   7

/* TNT_CMP_0_V0 :: LPEAK_INCORE_GOFF_HIGH :: OFFSET1 [06:00] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_OFFSET1_MASK      0x0000007f
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_OFFSET1_SHIFT     0
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_GOFF_HIGH_OFFSET1_DEFAULT   0x00000000

/***************************************************************************
 *LPEAK_INCORE_THR_LOW - Luma Peaking Input Coring Low Band Thresholds
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LPEAK_INCORE_THR_LOW :: reserved0 [31:20] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW_reserved0_MASK      0xfff00000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW_reserved0_SHIFT     20

/* TNT_CMP_0_V0 :: LPEAK_INCORE_THR_LOW :: T2 [19:10] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW_T2_MASK             0x000ffc00
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW_T2_SHIFT            10
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW_T2_DEFAULT          0x00000000

/* TNT_CMP_0_V0 :: LPEAK_INCORE_THR_LOW :: T1 [09:00] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW_T1_MASK             0x000003ff
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW_T1_SHIFT            0
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_LOW_T1_DEFAULT          0x00000000

/***************************************************************************
 *LPEAK_INCORE_DIV_LOW%i - Luma Peaking Input Coring Low Band Threshold Scale Factors
 ***************************************************************************/
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_LOWi_ARRAY_BASE         0x00048864
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_LOWi_ARRAY_START        0
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_LOWi_ARRAY_END          2
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_LOWi_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *LPEAK_INCORE_DIV_LOW%i - Luma Peaking Input Coring Low Band Threshold Scale Factors
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LPEAK_INCORE_DIV_LOWi :: reserved0 [31:16] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_LOWi_reserved0_MASK     0xffff0000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_LOWi_reserved0_SHIFT    16

/* TNT_CMP_0_V0 :: LPEAK_INCORE_DIV_LOWi :: ONE_OVER_T [15:00] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_LOWi_ONE_OVER_T_MASK    0x0000ffff
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_LOWi_ONE_OVER_T_SHIFT   0
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_LOWi_ONE_OVER_T_DEFAULT 0x00000000


/***************************************************************************
 *LPEAK_INCORE_THR_HIGH - Luma Peaking Input Coring High Band Thresholds
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LPEAK_INCORE_THR_HIGH :: reserved0 [31:20] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH_reserved0_MASK     0xfff00000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH_reserved0_SHIFT    20

/* TNT_CMP_0_V0 :: LPEAK_INCORE_THR_HIGH :: T2 [19:10] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH_T2_MASK            0x000ffc00
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH_T2_SHIFT           10
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH_T2_DEFAULT         0x00000000

/* TNT_CMP_0_V0 :: LPEAK_INCORE_THR_HIGH :: T1 [09:00] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH_T1_MASK            0x000003ff
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH_T1_SHIFT           0
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_THR_HIGH_T1_DEFAULT         0x00000000

/***************************************************************************
 *LPEAK_INCORE_DIV_HIGH%i - Luma Peaking Input Coring High Band Threshold Scale Factors
 ***************************************************************************/
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_HIGHi_ARRAY_BASE        0x00048874
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_HIGHi_ARRAY_START       0
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_HIGHi_ARRAY_END         2
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_HIGHi_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *LPEAK_INCORE_DIV_HIGH%i - Luma Peaking Input Coring High Band Threshold Scale Factors
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LPEAK_INCORE_DIV_HIGHi :: reserved0 [31:16] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_HIGHi_reserved0_MASK    0xffff0000
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_HIGHi_reserved0_SHIFT   16

/* TNT_CMP_0_V0 :: LPEAK_INCORE_DIV_HIGHi :: ONE_OVER_T [15:00] */
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_HIGHi_ONE_OVER_T_MASK   0x0000ffff
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_HIGHi_ONE_OVER_T_SHIFT  0
#define BCHP_TNT_CMP_0_V0_LPEAK_INCORE_DIV_HIGHi_ONE_OVER_T_DEFAULT 0x00000000


/***************************************************************************
 *LPEAK_OUT_CORE - Luma Peaking Output Coring Control
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LPEAK_OUT_CORE :: reserved0 [31:26] */
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_reserved0_MASK            0xfc000000
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_reserved0_SHIFT           26

/* TNT_CMP_0_V0 :: LPEAK_OUT_CORE :: MODE [25:24] */
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_MODE_MASK                 0x03000000
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_MODE_SHIFT                24
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_MODE_DEFAULT              0x00000001
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_MODE_LOW_HIGH             1
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_MODE_TOTAL                2
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_MODE_RESERVED             3

/* TNT_CMP_0_V0 :: LPEAK_OUT_CORE :: reserved1 [23:16] */
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_reserved1_MASK            0x00ff0000
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_reserved1_SHIFT           16

/* TNT_CMP_0_V0 :: LPEAK_OUT_CORE :: LCORE_BAND2 [15:08] */
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_LCORE_BAND2_MASK          0x0000ff00
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_LCORE_BAND2_SHIFT         8
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_LCORE_BAND2_DEFAULT       0x00000000

/* TNT_CMP_0_V0 :: LPEAK_OUT_CORE :: LCORE_BAND1 [07:00] */
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_LCORE_BAND1_MASK          0x000000ff
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_LCORE_BAND1_SHIFT         0
#define BCHP_TNT_CMP_0_V0_LPEAK_OUT_CORE_LCORE_BAND1_DEFAULT       0x00000000

/***************************************************************************
 *LPEAK_CLIP_AVOID - Luma Peaking Clip Avoidance Control
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LPEAK_CLIP_AVOID :: reserved0 [31:25] */
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_reserved0_MASK          0xfe000000
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_reserved0_SHIFT         25

/* TNT_CMP_0_V0 :: LPEAK_CLIP_AVOID :: CLIPAVOID_EN [24:24] */
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_CLIPAVOID_EN_MASK       0x01000000
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_CLIPAVOID_EN_SHIFT      24
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_CLIPAVOID_EN_DEFAULT    0x00000000
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_CLIPAVOID_EN_DISABLE    0
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_CLIPAVOID_EN_ENABLE     1

/* TNT_CMP_0_V0 :: LPEAK_CLIP_AVOID :: SLOPE2 [23:22] */
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_SLOPE2_MASK             0x00c00000
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_SLOPE2_SHIFT            22
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_SLOPE2_DEFAULT          0x00000000

/* TNT_CMP_0_V0 :: LPEAK_CLIP_AVOID :: SLOPE1 [21:20] */
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_SLOPE1_MASK             0x00300000
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_SLOPE1_SHIFT            20
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_SLOPE1_DEFAULT          0x00000000

/* TNT_CMP_0_V0 :: LPEAK_CLIP_AVOID :: CLIPVAL2 [19:10] */
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_CLIPVAL2_MASK           0x000ffc00
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_CLIPVAL2_SHIFT          10
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_CLIPVAL2_DEFAULT        0x00000000

/* TNT_CMP_0_V0 :: LPEAK_CLIP_AVOID :: CLIPVAL1 [09:00] */
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_CLIPVAL1_MASK           0x000003ff
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_CLIPVAL1_SHIFT          0
#define BCHP_TNT_CMP_0_V0_LPEAK_CLIP_AVOID_CLIPVAL1_DEFAULT        0x00000000

/***************************************************************************
 *LTI_CONTROL - LTI Control (Gain/Core/Avoid/Etc)
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LTI_CONTROL :: LTI_INCORE_EN [31:31] */
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_LTI_INCORE_EN_MASK           0x80000000
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_LTI_INCORE_EN_SHIFT          31
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_LTI_INCORE_EN_DEFAULT        0x00000000
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_LTI_INCORE_EN_DISABLE        0
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_LTI_INCORE_EN_ENABLE         1

/* TNT_CMP_0_V0 :: LTI_CONTROL :: reserved0 [30:26] */
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_reserved0_MASK               0x7c000000
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_reserved0_SHIFT              26

/* TNT_CMP_0_V0 :: LTI_CONTROL :: GAIN [25:20] */
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_GAIN_MASK                    0x03f00000
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_GAIN_SHIFT                   20
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_GAIN_DEFAULT                 0x00000000

/* TNT_CMP_0_V0 :: LTI_CONTROL :: HAVOID [19:14] */
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_HAVOID_MASK                  0x000fc000
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_HAVOID_SHIFT                 14
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_HAVOID_DEFAULT               0x00000000

/* TNT_CMP_0_V0 :: LTI_CONTROL :: reserved1 [13:08] */
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_reserved1_MASK               0x00003f00
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_reserved1_SHIFT              8

/* TNT_CMP_0_V0 :: LTI_CONTROL :: CORE_LEVEL [07:00] */
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_CORE_LEVEL_MASK              0x000000ff
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_CORE_LEVEL_SHIFT             0
#define BCHP_TNT_CMP_0_V0_LTI_CONTROL_CORE_LEVEL_DEFAULT           0x00000000

/***************************************************************************
 *LTI_FILTER - LTI Filter Control
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LTI_FILTER :: reserved0 [31:05] */
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_reserved0_MASK                0xffffffe0
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_reserved0_SHIFT               5

/* TNT_CMP_0_V0 :: LTI_FILTER :: BLUR_EN [04:04] */
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_BLUR_EN_MASK                  0x00000010
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_BLUR_EN_SHIFT                 4
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_BLUR_EN_DEFAULT               0x00000000
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_BLUR_EN_DISABLE               0
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_BLUR_EN_ENABLE                1

/* TNT_CMP_0_V0 :: LTI_FILTER :: reserved1 [03:02] */
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_reserved1_MASK                0x0000000c
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_reserved1_SHIFT               2

/* TNT_CMP_0_V0 :: LTI_FILTER :: H_FILT_SEL [01:00] */
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_H_FILT_SEL_MASK               0x00000003
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_H_FILT_SEL_SHIFT              0
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_H_FILT_SEL_DEFAULT            0x00000000
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_H_FILT_SEL_TAP5               0
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_H_FILT_SEL_TAP7               1
#define BCHP_TNT_CMP_0_V0_LTI_FILTER_H_FILT_SEL_TAP9               2

/***************************************************************************
 *LTI_INCORE_THR - LTI Input Coring Thresholds
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LTI_INCORE_THR :: reserved0 [31:20] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_THR_reserved0_MASK            0xfff00000
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_THR_reserved0_SHIFT           20

/* TNT_CMP_0_V0 :: LTI_INCORE_THR :: T2 [19:10] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_THR_T2_MASK                   0x000ffc00
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_THR_T2_SHIFT                  10
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_THR_T2_DEFAULT                0x00000000

/* TNT_CMP_0_V0 :: LTI_INCORE_THR :: T1 [09:00] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_THR_T1_MASK                   0x000003ff
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_THR_T1_SHIFT                  0
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_THR_T1_DEFAULT                0x00000000

/***************************************************************************
 *LTI_INCORE_DIV%i - LTI Input Coring Threshold Scale Factors
 ***************************************************************************/
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_DIVi_ARRAY_BASE               0x00048894
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_DIVi_ARRAY_START              0
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_DIVi_ARRAY_END                2
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_DIVi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *LTI_INCORE_DIV%i - LTI Input Coring Threshold Scale Factors
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LTI_INCORE_DIVi :: reserved0 [31:16] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_DIVi_reserved0_MASK           0xffff0000
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_DIVi_reserved0_SHIFT          16

/* TNT_CMP_0_V0 :: LTI_INCORE_DIVi :: ONE_OVER_T [15:00] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_DIVi_ONE_OVER_T_MASK          0x0000ffff
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_DIVi_ONE_OVER_T_SHIFT         0
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_DIVi_ONE_OVER_T_DEFAULT       0x00000000


/***************************************************************************
 *LTI_INCORE_GOFF - LTI Input Coring Gain Offsets
 ***************************************************************************/
/* TNT_CMP_0_V0 :: LTI_INCORE_GOFF :: reserved0 [31:31] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_reserved0_MASK           0x80000000
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_reserved0_SHIFT          31

/* TNT_CMP_0_V0 :: LTI_INCORE_GOFF :: OFFSET4 [30:24] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_OFFSET4_MASK             0x7f000000
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_OFFSET4_SHIFT            24
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_OFFSET4_DEFAULT          0x00000000

/* TNT_CMP_0_V0 :: LTI_INCORE_GOFF :: reserved1 [23:23] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_reserved1_MASK           0x00800000
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_reserved1_SHIFT          23

/* TNT_CMP_0_V0 :: LTI_INCORE_GOFF :: OFFSET3 [22:16] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_OFFSET3_MASK             0x007f0000
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_OFFSET3_SHIFT            16
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_OFFSET3_DEFAULT          0x00000000

/* TNT_CMP_0_V0 :: LTI_INCORE_GOFF :: reserved2 [15:15] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_reserved2_MASK           0x00008000
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_reserved2_SHIFT          15

/* TNT_CMP_0_V0 :: LTI_INCORE_GOFF :: OFFSET2 [14:08] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_OFFSET2_MASK             0x00007f00
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_OFFSET2_SHIFT            8
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_OFFSET2_DEFAULT          0x00000000

/* TNT_CMP_0_V0 :: LTI_INCORE_GOFF :: reserved3 [07:07] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_reserved3_MASK           0x00000080
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_reserved3_SHIFT          7

/* TNT_CMP_0_V0 :: LTI_INCORE_GOFF :: OFFSET1 [06:00] */
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_OFFSET1_MASK             0x0000007f
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_OFFSET1_SHIFT            0
#define BCHP_TNT_CMP_0_V0_LTI_INCORE_GOFF_OFFSET1_DEFAULT          0x00000000

/***************************************************************************
 *CTI_H - CTI Horizontal Filter Controls
 ***************************************************************************/
/* TNT_CMP_0_V0 :: CTI_H :: reserved0 [31:18] */
#define BCHP_TNT_CMP_0_V0_CTI_H_reserved0_MASK                     0xfffc0000
#define BCHP_TNT_CMP_0_V0_CTI_H_reserved0_SHIFT                    18

/* TNT_CMP_0_V0 :: CTI_H :: FILT_SEL [17:16] */
#define BCHP_TNT_CMP_0_V0_CTI_H_FILT_SEL_MASK                      0x00030000
#define BCHP_TNT_CMP_0_V0_CTI_H_FILT_SEL_SHIFT                     16
#define BCHP_TNT_CMP_0_V0_CTI_H_FILT_SEL_DEFAULT                   0x00000000
#define BCHP_TNT_CMP_0_V0_CTI_H_FILT_SEL_TAP5                      0
#define BCHP_TNT_CMP_0_V0_CTI_H_FILT_SEL_TAP7                      1
#define BCHP_TNT_CMP_0_V0_CTI_H_FILT_SEL_TAP9                      2

/* TNT_CMP_0_V0 :: CTI_H :: reserved1 [15:14] */
#define BCHP_TNT_CMP_0_V0_CTI_H_reserved1_MASK                     0x0000c000
#define BCHP_TNT_CMP_0_V0_CTI_H_reserved1_SHIFT                    14

/* TNT_CMP_0_V0 :: CTI_H :: GAIN [13:08] */
#define BCHP_TNT_CMP_0_V0_CTI_H_GAIN_MASK                          0x00003f00
#define BCHP_TNT_CMP_0_V0_CTI_H_GAIN_SHIFT                         8
#define BCHP_TNT_CMP_0_V0_CTI_H_GAIN_DEFAULT                       0x00000000

/* TNT_CMP_0_V0 :: CTI_H :: CORE_LEVEL [07:00] */
#define BCHP_TNT_CMP_0_V0_CTI_H_CORE_LEVEL_MASK                    0x000000ff
#define BCHP_TNT_CMP_0_V0_CTI_H_CORE_LEVEL_SHIFT                   0
#define BCHP_TNT_CMP_0_V0_CTI_H_CORE_LEVEL_DEFAULT                 0x00000000

#endif /* #ifndef BCHP_TNT_CMP_0_V0_H__ */

/* End of File */
