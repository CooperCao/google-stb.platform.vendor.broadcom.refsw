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
 * Date:           Generated on               Tue Feb 23 15:26:07 2016
 *                 Full Compile MD5 Checksum  4b84f30a4b3665aac5b824a1ed76e56c
 *                     (minus title and desc)
 *                 MD5 Checksum               4894bba0ec078aee10b5b5954262d56e
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     804
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_AVS_PMB_S_000_H__
#define BCHP_AVS_PMB_S_000_H__

/***************************************************************************
 *AVS_PMB_S_000 - AVS_PMB_SLAVE_000 Registers
 ***************************************************************************/
#define BCHP_AVS_PMB_S_000_BPCM_ID               0x04114000 /* [RO] BPCM ID Register */
#define BCHP_AVS_PMB_S_000_BPCM_CAPABILITY       0x04114004 /* [RO] BPCM Capability Register */
#define BCHP_AVS_PMB_S_000_BPCM_CONTROL          0x04114008 /* [RW] BPCM Control Register */
#define BCHP_AVS_PMB_S_000_BPCM_STATUS           0x0411400c /* [RO] BPCM Status Register */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL      0x04114010 /* [RW] Ring Ocsillator Control Register */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_H_THRESHOLD  0x04114014 /* [RW] Event Counter Threshold Register for ROSC_H */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_S_THRESHOLD  0x04114018 /* [RW] Event Counter Threshold Register for ROSC_S */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT        0x0411401c /* [RO] Event Counter Count Register */
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL       0x04114020 /* [RW] PWD control Register */
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL   0x04114024 /* [RW] PWD Accumulator Control and Status Register */

/***************************************************************************
 *BPCM_ID - BPCM ID Register
 ***************************************************************************/
/* AVS_PMB_S_000 :: BPCM_ID :: SW_strap [31:16] */
#define BCHP_AVS_PMB_S_000_BPCM_ID_SW_strap_MASK                   0xffff0000
#define BCHP_AVS_PMB_S_000_BPCM_ID_SW_strap_SHIFT                  16

/* AVS_PMB_S_000 :: BPCM_ID :: HW_revision [15:08] */
#define BCHP_AVS_PMB_S_000_BPCM_ID_HW_revision_MASK                0x0000ff00
#define BCHP_AVS_PMB_S_000_BPCM_ID_HW_revision_SHIFT               8

/* AVS_PMB_S_000 :: BPCM_ID :: PMB_ADDR [07:00] */
#define BCHP_AVS_PMB_S_000_BPCM_ID_PMB_ADDR_MASK                   0x000000ff
#define BCHP_AVS_PMB_S_000_BPCM_ID_PMB_ADDR_SHIFT                  0

/***************************************************************************
 *BPCM_CAPABILITY - BPCM Capability Register
 ***************************************************************************/
/* AVS_PMB_S_000 :: BPCM_CAPABILITY :: reserved0 [31:08] */
#define BCHP_AVS_PMB_S_000_BPCM_CAPABILITY_reserved0_MASK          0xffffff00
#define BCHP_AVS_PMB_S_000_BPCM_CAPABILITY_reserved0_SHIFT         8

/* AVS_PMB_S_000 :: BPCM_CAPABILITY :: Number_of_zones [07:00] */
#define BCHP_AVS_PMB_S_000_BPCM_CAPABILITY_Number_of_zones_MASK    0x000000ff
#define BCHP_AVS_PMB_S_000_BPCM_CAPABILITY_Number_of_zones_SHIFT   0

/***************************************************************************
 *BPCM_CONTROL - BPCM Control Register
 ***************************************************************************/
/* AVS_PMB_S_000 :: BPCM_CONTROL :: reserved_for_eco0 [31:00] */
#define BCHP_AVS_PMB_S_000_BPCM_CONTROL_reserved_for_eco0_MASK     0xffffffff
#define BCHP_AVS_PMB_S_000_BPCM_CONTROL_reserved_for_eco0_SHIFT    0
#define BCHP_AVS_PMB_S_000_BPCM_CONTROL_reserved_for_eco0_DEFAULT  0x00000000

/***************************************************************************
 *BPCM_STATUS - BPCM Status Register
 ***************************************************************************/
/* AVS_PMB_S_000 :: BPCM_STATUS :: reserved0 [31:01] */
#define BCHP_AVS_PMB_S_000_BPCM_STATUS_reserved0_MASK              0xfffffffe
#define BCHP_AVS_PMB_S_000_BPCM_STATUS_reserved0_SHIFT             1

/* AVS_PMB_S_000 :: BPCM_STATUS :: PWD_Alert [00:00] */
#define BCHP_AVS_PMB_S_000_BPCM_STATUS_PWD_Alert_MASK              0x00000001
#define BCHP_AVS_PMB_S_000_BPCM_STATUS_PWD_Alert_SHIFT             0

/***************************************************************************
 *AVS_ROSC_CONTROL - Ring Ocsillator Control Register
 ***************************************************************************/
/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: Test_interval [31:16] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_Test_interval_MASK     0xffff0000
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_Test_interval_SHIFT    16
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_Test_interval_DEFAULT  0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: ALERT_H [15:15] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ALERT_H_MASK           0x00008000
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ALERT_H_SHIFT          15
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ALERT_H_DEFAULT        0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: VALID_H [14:14] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_VALID_H_MASK           0x00004000
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_VALID_H_SHIFT          14
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_VALID_H_DEFAULT        0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: ALERT_S [13:13] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ALERT_S_MASK           0x00002000
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ALERT_S_SHIFT          13
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ALERT_S_DEFAULT        0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: VALID_S [12:12] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_VALID_S_MASK           0x00001000
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_VALID_S_SHIFT          12
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_VALID_S_DEFAULT        0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: reserved_for_eco0 [11:08] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_reserved_for_eco0_MASK 0x00000f00
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_reserved_for_eco0_SHIFT 8
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_reserved_for_eco0_DEFAULT 0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: CONTINUOUS_H [07:07] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_CONTINUOUS_H_MASK      0x00000080
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_CONTINUOUS_H_SHIFT     7
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_CONTINUOUS_H_DEFAULT   0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: CONTINUOUS_S [06:06] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_CONTINUOUS_S_MASK      0x00000040
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_CONTINUOUS_S_SHIFT     6
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_CONTINUOUS_S_DEFAULT   0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: THRSH_EN_H [05:05] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_THRSH_EN_H_MASK        0x00000020
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_THRSH_EN_H_SHIFT       5
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_THRSH_EN_H_DEFAULT     0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: THRSH_EN_S [04:04] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_THRSH_EN_S_MASK        0x00000010
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_THRSH_EN_S_SHIFT       4
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_THRSH_EN_S_DEFAULT     0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: ECTR_EN_H [03:03] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ECTR_EN_H_MASK         0x00000008
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ECTR_EN_H_SHIFT        3
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ECTR_EN_H_DEFAULT      0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: ECTR_EN_S [02:02] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ECTR_EN_S_MASK         0x00000004
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ECTR_EN_S_SHIFT        2
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_ECTR_EN_S_DEFAULT      0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: RO_EN_H [01:01] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_RO_EN_H_MASK           0x00000002
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_RO_EN_H_SHIFT          1
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_RO_EN_H_DEFAULT        0x00000001

/* AVS_PMB_S_000 :: AVS_ROSC_CONTROL :: RO_EN_S [00:00] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_RO_EN_S_MASK           0x00000001
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_RO_EN_S_SHIFT          0
#define BCHP_AVS_PMB_S_000_AVS_ROSC_CONTROL_RO_EN_S_DEFAULT        0x00000001

/***************************************************************************
 *AVS_ROSC_H_THRESHOLD - Event Counter Threshold Register for ROSC_H
 ***************************************************************************/
/* AVS_PMB_S_000 :: AVS_ROSC_H_THRESHOLD :: THRESH_HI [31:16] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_H_THRESHOLD_THRESH_HI_MASK     0xffff0000
#define BCHP_AVS_PMB_S_000_AVS_ROSC_H_THRESHOLD_THRESH_HI_SHIFT    16
#define BCHP_AVS_PMB_S_000_AVS_ROSC_H_THRESHOLD_THRESH_HI_DEFAULT  0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_H_THRESHOLD :: THRESH_LO [15:00] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_H_THRESHOLD_THRESH_LO_MASK     0x0000ffff
#define BCHP_AVS_PMB_S_000_AVS_ROSC_H_THRESHOLD_THRESH_LO_SHIFT    0
#define BCHP_AVS_PMB_S_000_AVS_ROSC_H_THRESHOLD_THRESH_LO_DEFAULT  0x00000000

/***************************************************************************
 *AVS_ROSC_S_THRESHOLD - Event Counter Threshold Register for ROSC_S
 ***************************************************************************/
/* AVS_PMB_S_000 :: AVS_ROSC_S_THRESHOLD :: THRESH_HI [31:16] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_S_THRESHOLD_THRESH_HI_MASK     0xffff0000
#define BCHP_AVS_PMB_S_000_AVS_ROSC_S_THRESHOLD_THRESH_HI_SHIFT    16
#define BCHP_AVS_PMB_S_000_AVS_ROSC_S_THRESHOLD_THRESH_HI_DEFAULT  0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_S_THRESHOLD :: THRESH_LO [15:00] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_S_THRESHOLD_THRESH_LO_MASK     0x0000ffff
#define BCHP_AVS_PMB_S_000_AVS_ROSC_S_THRESHOLD_THRESH_LO_SHIFT    0
#define BCHP_AVS_PMB_S_000_AVS_ROSC_S_THRESHOLD_THRESH_LO_DEFAULT  0x00000000

/***************************************************************************
 *AVS_ROSC_COUNT - Event Counter Count Register
 ***************************************************************************/
/* AVS_PMB_S_000 :: AVS_ROSC_COUNT :: COUNT_H [31:16] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT_COUNT_H_MASK             0xffff0000
#define BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT_COUNT_H_SHIFT            16
#define BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT_COUNT_H_DEFAULT          0x00000000

/* AVS_PMB_S_000 :: AVS_ROSC_COUNT :: COUNT_S [15:00] */
#define BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT_COUNT_S_MASK             0x0000ffff
#define BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT_COUNT_S_SHIFT            0
#define BCHP_AVS_PMB_S_000_AVS_ROSC_COUNT_COUNT_S_DEFAULT          0x00000000

/***************************************************************************
 *AVS_PWD_CONTROL - PWD control Register
 ***************************************************************************/
/* AVS_PMB_S_000 :: AVS_PWD_CONTROL :: reserved_for_eco0 [31:30] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_reserved_for_eco0_MASK  0xc0000000
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_reserved_for_eco0_SHIFT 30
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_reserved_for_eco0_DEFAULT 0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_CONTROL :: CLRCFG [29:27] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_CLRCFG_MASK             0x38000000
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_CLRCFG_SHIFT            27
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_CLRCFG_DEFAULT          0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_CONTROL :: RSEL [26:24] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_RSEL_MASK               0x07000000
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_RSEL_SHIFT              24
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_RSEL_DEFAULT            0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_CONTROL :: CGFG [23:16] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_CGFG_MASK               0x00ff0000
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_CGFG_SHIFT              16
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_CGFG_DEFAULT            0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_CONTROL :: ALERT [15:15] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_ALERT_MASK              0x00008000
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_ALERT_SHIFT             15
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_ALERT_DEFAULT           0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_CONTROL :: reserved1 [14:10] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_reserved1_MASK          0x00007c00
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_reserved1_SHIFT         10

/* AVS_PMB_S_000 :: AVS_PWD_CONTROL :: PWD_TST_STROBE [09:09] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_PWD_TST_STROBE_MASK     0x00000200
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_PWD_TST_STROBE_SHIFT    9
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_PWD_TST_STROBE_DEFAULT  0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_CONTROL :: PWD_TM_EN [08:08] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_PWD_TM_EN_MASK          0x00000100
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_PWD_TM_EN_SHIFT         8
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_PWD_TM_EN_DEFAULT       0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_CONTROL :: START [07:02] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_START_MASK              0x000000fc
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_START_SHIFT             2
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_START_DEFAULT           0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_CONTROL :: PWD_ALERT_SEL [01:01] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_PWD_ALERT_SEL_MASK      0x00000002
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_PWD_ALERT_SEL_SHIFT     1
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_PWD_ALERT_SEL_DEFAULT   0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_CONTROL :: PWD_EN [00:00] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_PWD_EN_MASK             0x00000001
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_PWD_EN_SHIFT            0
#define BCHP_AVS_PMB_S_000_AVS_PWD_CONTROL_PWD_EN_DEFAULT          0x00000000

/***************************************************************************
 *AVS_PWD_ACC_CONTROL - PWD Accumulator Control and Status Register
 ***************************************************************************/
/* AVS_PMB_S_000 :: AVS_PWD_ACC_CONTROL :: reserved0 [31:28] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_reserved0_MASK      0xf0000000
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_reserved0_SHIFT     28

/* AVS_PMB_S_000 :: AVS_PWD_ACC_CONTROL :: DONE [27:27] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_DONE_MASK           0x08000000
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_DONE_SHIFT          27
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_DONE_DEFAULT        0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_ACC_CONTROL :: GOOD_WAS_LOW [26:26] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_GOOD_WAS_LOW_MASK   0x04000000
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_GOOD_WAS_LOW_SHIFT  26
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_GOOD_WAS_LOW_DEFAULT 0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_ACC_CONTROL :: SUM_GOOD [25:16] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_SUM_GOOD_MASK       0x03ff0000
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_SUM_GOOD_SHIFT      16
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_SUM_GOOD_DEFAULT    0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_ACC_CONTROL :: reserved1 [15:05] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_reserved1_MASK      0x0000ffe0
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_reserved1_SHIFT     5

/* AVS_PMB_S_000 :: AVS_PWD_ACC_CONTROL :: SKIP [04:03] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_SKIP_MASK           0x00000018
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_SKIP_SHIFT          3
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_SKIP_DEFAULT        0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_ACC_CONTROL :: SKIP_START [02:01] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_SKIP_START_MASK     0x00000006
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_SKIP_START_SHIFT    1
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_SKIP_START_DEFAULT  0x00000000

/* AVS_PMB_S_000 :: AVS_PWD_ACC_CONTROL :: RUN [00:00] */
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_RUN_MASK            0x00000001
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_RUN_SHIFT           0
#define BCHP_AVS_PMB_S_000_AVS_PWD_ACC_CONTROL_RUN_DEFAULT         0x00000000

#endif /* #ifndef BCHP_AVS_PMB_S_000_H__ */

/* End of File */
