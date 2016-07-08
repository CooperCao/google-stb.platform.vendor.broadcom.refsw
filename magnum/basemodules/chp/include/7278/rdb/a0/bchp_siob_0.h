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

#ifndef BCHP_SIOB_0_H__
#define BCHP_SIOB_0_H__

/***************************************************************************
 *SIOB_0 - MCVP SIOB Control Registers
 ***************************************************************************/
#define BCHP_SIOB_0_DCXS_CFG                     0x00088200 /* [CFG] DCXS2 Configuration */
#define BCHP_SIOB_0_DCXS_RCTRL                   0x00088204 /* [CFG] DCXS2 Rate Control */
#define BCHP_SIOB_0_DITHER_CTRL                  0x00088208 /* [RW] MCVP-SIOB Pixel Capture Dither Control */
#define BCHP_SIOB_0_DITHER_LFSR_INIT             0x0008820c /* [RW] MCVP_SIOB Pixel Capture Dither LFSR Initialization */
#define BCHP_SIOB_0_DITHER_LFSR_CTRL             0x00088210 /* [RW] MCVP-SIOB Pixel Capture Dither LFSR Control */
#define BCHP_SIOB_0_SCB_MODE_CONTROL             0x00088214 /* [CFG] MCVP-SIOB SCB Mode Control */
#define BCHP_SIOB_0_DCES2_DIAG                   0x000882e8 /* [RO] Diagnostic port */
#define BCHP_SIOB_0_DCDS2_FEEDER_B_DIAG          0x000882ec /* [RO] Diagnostic port */
#define BCHP_SIOB_0_DCDS2_FEEDER_J_DIAG          0x000882f0 /* [RO] Diagnostic port */
#define BCHP_SIOB_0_DCDS2_FEEDER_G_DIAG          0x000882f4 /* [RO] Diagnostic port */
#define BCHP_SIOB_0_TEST_PORT_CONTROL            0x000882f8 /* [CFG] Test port control register */
#define BCHP_SIOB_0_TEST_PORT_DATA               0x000882fc /* [RO] Test port data register */

/***************************************************************************
 *DCXS_CFG - DCXS2 Configuration
 ***************************************************************************/
/* SIOB_0 :: DCXS_CFG :: ENABLE [31:31] */
#define BCHP_SIOB_0_DCXS_CFG_ENABLE_MASK                           0x80000000
#define BCHP_SIOB_0_DCXS_CFG_ENABLE_SHIFT                          31
#define BCHP_SIOB_0_DCXS_CFG_ENABLE_DEFAULT                        0x00000001
#define BCHP_SIOB_0_DCXS_CFG_ENABLE_Disable                        0
#define BCHP_SIOB_0_DCXS_CFG_ENABLE_Enable                         1

/* SIOB_0 :: DCXS_CFG :: reserved0 [30:05] */
#define BCHP_SIOB_0_DCXS_CFG_reserved0_MASK                        0x7fffffe0
#define BCHP_SIOB_0_DCXS_CFG_reserved0_SHIFT                       5

/* SIOB_0 :: DCXS_CFG :: APPLY_QERR [04:03] */
#define BCHP_SIOB_0_DCXS_CFG_APPLY_QERR_MASK                       0x00000018
#define BCHP_SIOB_0_DCXS_CFG_APPLY_QERR_SHIFT                      3
#define BCHP_SIOB_0_DCXS_CFG_APPLY_QERR_DEFAULT                    0x00000001
#define BCHP_SIOB_0_DCXS_CFG_APPLY_QERR_Apply_No_Qerr              0
#define BCHP_SIOB_0_DCXS_CFG_APPLY_QERR_Apply_Qerr                 1
#define BCHP_SIOB_0_DCXS_CFG_APPLY_QERR_Apply_Half_Qerr            2
#define BCHP_SIOB_0_DCXS_CFG_APPLY_QERR_Reserved                   3

/* SIOB_0 :: DCXS_CFG :: FIXED_RATE [02:02] */
#define BCHP_SIOB_0_DCXS_CFG_FIXED_RATE_MASK                       0x00000004
#define BCHP_SIOB_0_DCXS_CFG_FIXED_RATE_SHIFT                      2
#define BCHP_SIOB_0_DCXS_CFG_FIXED_RATE_DEFAULT                    0x00000000
#define BCHP_SIOB_0_DCXS_CFG_FIXED_RATE_Variable                   0
#define BCHP_SIOB_0_DCXS_CFG_FIXED_RATE_Fixed                      1

/* SIOB_0 :: DCXS_CFG :: COMPRESSION [01:00] */
#define BCHP_SIOB_0_DCXS_CFG_COMPRESSION_MASK                      0x00000003
#define BCHP_SIOB_0_DCXS_CFG_COMPRESSION_SHIFT                     0
#define BCHP_SIOB_0_DCXS_CFG_COMPRESSION_DEFAULT                   0x00000002
#define BCHP_SIOB_0_DCXS_CFG_COMPRESSION_Reserved                  0
#define BCHP_SIOB_0_DCXS_CFG_COMPRESSION_BPP_9p25_OR_9             1
#define BCHP_SIOB_0_DCXS_CFG_COMPRESSION_BPP_11p25_OR_11           2
#define BCHP_SIOB_0_DCXS_CFG_COMPRESSION_BPP_13p25                 3

/***************************************************************************
 *DCXS_RCTRL - DCXS2 Rate Control
 ***************************************************************************/
/* SIOB_0 :: DCXS_RCTRL :: reserved0 [31:24] */
#define BCHP_SIOB_0_DCXS_RCTRL_reserved0_MASK                      0xff000000
#define BCHP_SIOB_0_DCXS_RCTRL_reserved0_SHIFT                     24

/* SIOB_0 :: DCXS_RCTRL :: SPEND_QP_3 [23:21] */
#define BCHP_SIOB_0_DCXS_RCTRL_SPEND_QP_3_MASK                     0x00e00000
#define BCHP_SIOB_0_DCXS_RCTRL_SPEND_QP_3_SHIFT                    21
#define BCHP_SIOB_0_DCXS_RCTRL_SPEND_QP_3_DEFAULT                  0x00000000

/* SIOB_0 :: DCXS_RCTRL :: SPEND_QP_2 [20:18] */
#define BCHP_SIOB_0_DCXS_RCTRL_SPEND_QP_2_MASK                     0x001c0000
#define BCHP_SIOB_0_DCXS_RCTRL_SPEND_QP_2_SHIFT                    18
#define BCHP_SIOB_0_DCXS_RCTRL_SPEND_QP_2_DEFAULT                  0x00000001

/* SIOB_0 :: DCXS_RCTRL :: SPEND_QP_1 [17:15] */
#define BCHP_SIOB_0_DCXS_RCTRL_SPEND_QP_1_MASK                     0x00038000
#define BCHP_SIOB_0_DCXS_RCTRL_SPEND_QP_1_SHIFT                    15
#define BCHP_SIOB_0_DCXS_RCTRL_SPEND_QP_1_DEFAULT                  0x00000002

/* SIOB_0 :: DCXS_RCTRL :: SPEND_QP_0 [14:12] */
#define BCHP_SIOB_0_DCXS_RCTRL_SPEND_QP_0_MASK                     0x00007000
#define BCHP_SIOB_0_DCXS_RCTRL_SPEND_QP_0_SHIFT                    12
#define BCHP_SIOB_0_DCXS_RCTRL_SPEND_QP_0_DEFAULT                  0x00000003

/* SIOB_0 :: DCXS_RCTRL :: SAVE_QP_3 [11:09] */
#define BCHP_SIOB_0_DCXS_RCTRL_SAVE_QP_3_MASK                      0x00000e00
#define BCHP_SIOB_0_DCXS_RCTRL_SAVE_QP_3_SHIFT                     9
#define BCHP_SIOB_0_DCXS_RCTRL_SAVE_QP_3_DEFAULT                   0x00000000

/* SIOB_0 :: DCXS_RCTRL :: SAVE_QP_2 [08:06] */
#define BCHP_SIOB_0_DCXS_RCTRL_SAVE_QP_2_MASK                      0x000001c0
#define BCHP_SIOB_0_DCXS_RCTRL_SAVE_QP_2_SHIFT                     6
#define BCHP_SIOB_0_DCXS_RCTRL_SAVE_QP_2_DEFAULT                   0x00000001

/* SIOB_0 :: DCXS_RCTRL :: SAVE_QP_1 [05:03] */
#define BCHP_SIOB_0_DCXS_RCTRL_SAVE_QP_1_MASK                      0x00000038
#define BCHP_SIOB_0_DCXS_RCTRL_SAVE_QP_1_SHIFT                     3
#define BCHP_SIOB_0_DCXS_RCTRL_SAVE_QP_1_DEFAULT                   0x00000002

/* SIOB_0 :: DCXS_RCTRL :: SAVE_QP_0 [02:00] */
#define BCHP_SIOB_0_DCXS_RCTRL_SAVE_QP_0_MASK                      0x00000007
#define BCHP_SIOB_0_DCXS_RCTRL_SAVE_QP_0_SHIFT                     0
#define BCHP_SIOB_0_DCXS_RCTRL_SAVE_QP_0_DEFAULT                   0x00000003

/***************************************************************************
 *DITHER_CTRL - MCVP-SIOB Pixel Capture Dither Control
 ***************************************************************************/
/* SIOB_0 :: DITHER_CTRL :: MODE [31:30] */
#define BCHP_SIOB_0_DITHER_CTRL_MODE_MASK                          0xc0000000
#define BCHP_SIOB_0_DITHER_CTRL_MODE_SHIFT                         30
#define BCHP_SIOB_0_DITHER_CTRL_MODE_DEFAULT                       0x00000000
#define BCHP_SIOB_0_DITHER_CTRL_MODE_ROUNDING                      0
#define BCHP_SIOB_0_DITHER_CTRL_MODE_TRUNCATE                      1
#define BCHP_SIOB_0_DITHER_CTRL_MODE_DITHER                        2

/* SIOB_0 :: DITHER_CTRL :: reserved0 [29:20] */
#define BCHP_SIOB_0_DITHER_CTRL_reserved0_MASK                     0x3ff00000
#define BCHP_SIOB_0_DITHER_CTRL_reserved0_SHIFT                    20

/* SIOB_0 :: DITHER_CTRL :: OFFSET_CH1 [19:15] */
#define BCHP_SIOB_0_DITHER_CTRL_OFFSET_CH1_MASK                    0x000f8000
#define BCHP_SIOB_0_DITHER_CTRL_OFFSET_CH1_SHIFT                   15
#define BCHP_SIOB_0_DITHER_CTRL_OFFSET_CH1_DEFAULT                 0x00000001

/* SIOB_0 :: DITHER_CTRL :: SCALE_CH1 [14:10] */
#define BCHP_SIOB_0_DITHER_CTRL_SCALE_CH1_MASK                     0x00007c00
#define BCHP_SIOB_0_DITHER_CTRL_SCALE_CH1_SHIFT                    10
#define BCHP_SIOB_0_DITHER_CTRL_SCALE_CH1_DEFAULT                  0x00000000

/* SIOB_0 :: DITHER_CTRL :: OFFSET_CH0 [09:05] */
#define BCHP_SIOB_0_DITHER_CTRL_OFFSET_CH0_MASK                    0x000003e0
#define BCHP_SIOB_0_DITHER_CTRL_OFFSET_CH0_SHIFT                   5
#define BCHP_SIOB_0_DITHER_CTRL_OFFSET_CH0_DEFAULT                 0x00000001

/* SIOB_0 :: DITHER_CTRL :: SCALE_CH0 [04:00] */
#define BCHP_SIOB_0_DITHER_CTRL_SCALE_CH0_MASK                     0x0000001f
#define BCHP_SIOB_0_DITHER_CTRL_SCALE_CH0_SHIFT                    0
#define BCHP_SIOB_0_DITHER_CTRL_SCALE_CH0_DEFAULT                  0x00000000

/***************************************************************************
 *DITHER_LFSR_INIT - MCVP_SIOB Pixel Capture Dither LFSR Initialization
 ***************************************************************************/
/* SIOB_0 :: DITHER_LFSR_INIT :: reserved0 [31:22] */
#define BCHP_SIOB_0_DITHER_LFSR_INIT_reserved0_MASK                0xffc00000
#define BCHP_SIOB_0_DITHER_LFSR_INIT_reserved0_SHIFT               22

/* SIOB_0 :: DITHER_LFSR_INIT :: SEQ [21:20] */
#define BCHP_SIOB_0_DITHER_LFSR_INIT_SEQ_MASK                      0x00300000
#define BCHP_SIOB_0_DITHER_LFSR_INIT_SEQ_SHIFT                     20
#define BCHP_SIOB_0_DITHER_LFSR_INIT_SEQ_DEFAULT                   0x00000003
#define BCHP_SIOB_0_DITHER_LFSR_INIT_SEQ_ONCE                      0
#define BCHP_SIOB_0_DITHER_LFSR_INIT_SEQ_ONCE_PER_SOP              1
#define BCHP_SIOB_0_DITHER_LFSR_INIT_SEQ_ONCE_PER_2SOP             2
#define BCHP_SIOB_0_DITHER_LFSR_INIT_SEQ_NEVER                     3

/* SIOB_0 :: DITHER_LFSR_INIT :: VALUE [19:00] */
#define BCHP_SIOB_0_DITHER_LFSR_INIT_VALUE_MASK                    0x000fffff
#define BCHP_SIOB_0_DITHER_LFSR_INIT_VALUE_SHIFT                   0
#define BCHP_SIOB_0_DITHER_LFSR_INIT_VALUE_DEFAULT                 0x00000000

/***************************************************************************
 *DITHER_LFSR_CTRL - MCVP-SIOB Pixel Capture Dither LFSR Control
 ***************************************************************************/
/* SIOB_0 :: DITHER_LFSR_CTRL :: reserved0 [31:11] */
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_reserved0_MASK                0xfffff800
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_reserved0_SHIFT               11

/* SIOB_0 :: DITHER_LFSR_CTRL :: T2 [10:08] */
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T2_MASK                       0x00000700
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T2_SHIFT                      8
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T2_DEFAULT                    0x00000000
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T2_ZERO                       0
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T2_B12                        1
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T2_B13                        2
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T2_B14                        3
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T2_B15                        4
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T2_B16                        5
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T2_B17                        6
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T2_B18                        7

/* SIOB_0 :: DITHER_LFSR_CTRL :: reserved1 [07:07] */
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_reserved1_MASK                0x00000080
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_reserved1_SHIFT               7

/* SIOB_0 :: DITHER_LFSR_CTRL :: T1 [06:04] */
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T1_MASK                       0x00000070
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T1_SHIFT                      4
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T1_DEFAULT                    0x00000000
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T1_ZERO                       0
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T1_B8                         1
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T1_B9                         2
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T1_B10                        3
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T1_B11                        4
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T1_B12                        5
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T1_B13                        6
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T1_B14                        7

/* SIOB_0 :: DITHER_LFSR_CTRL :: reserved2 [03:03] */
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_reserved2_MASK                0x00000008
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_reserved2_SHIFT               3

/* SIOB_0 :: DITHER_LFSR_CTRL :: T0 [02:00] */
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T0_MASK                       0x00000007
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T0_SHIFT                      0
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T0_DEFAULT                    0x00000000
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T0_B2                         0
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T0_B3                         1
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T0_B4                         2
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T0_B6                         3
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T0_B7                         4
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T0_B8                         5
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T0_B9                         6
#define BCHP_SIOB_0_DITHER_LFSR_CTRL_T0_B10                        7

/***************************************************************************
 *SCB_MODE_CONTROL - MCVP-SIOB SCB Mode Control
 ***************************************************************************/
/* SIOB_0 :: SCB_MODE_CONTROL :: reserved0 [31:01] */
#define BCHP_SIOB_0_SCB_MODE_CONTROL_reserved0_MASK                0xfffffffe
#define BCHP_SIOB_0_SCB_MODE_CONTROL_reserved0_SHIFT               1

/* SIOB_0 :: SCB_MODE_CONTROL :: SCB_MODE_SEL [00:00] */
#define BCHP_SIOB_0_SCB_MODE_CONTROL_SCB_MODE_SEL_MASK             0x00000001
#define BCHP_SIOB_0_SCB_MODE_CONTROL_SCB_MODE_SEL_SHIFT            0
#define BCHP_SIOB_0_SCB_MODE_CONTROL_SCB_MODE_SEL_DEFAULT          0x00000000
#define BCHP_SIOB_0_SCB_MODE_CONTROL_SCB_MODE_SEL_MODE_8_BIT       0
#define BCHP_SIOB_0_SCB_MODE_CONTROL_SCB_MODE_SEL_MODE_10_BIT      1

/***************************************************************************
 *DCES2_DIAG - Diagnostic port
 ***************************************************************************/
/* SIOB_0 :: DCES2_DIAG :: DCES2_DIAG [31:00] */
#define BCHP_SIOB_0_DCES2_DIAG_DCES2_DIAG_MASK                     0xffffffff
#define BCHP_SIOB_0_DCES2_DIAG_DCES2_DIAG_SHIFT                    0
#define BCHP_SIOB_0_DCES2_DIAG_DCES2_DIAG_DEFAULT                  0x00000000

/***************************************************************************
 *DCDS2_FEEDER_B_DIAG - Diagnostic port
 ***************************************************************************/
/* SIOB_0 :: DCDS2_FEEDER_B_DIAG :: DCDS2_FEEDER_B_DIAG [31:00] */
#define BCHP_SIOB_0_DCDS2_FEEDER_B_DIAG_DCDS2_FEEDER_B_DIAG_MASK   0xffffffff
#define BCHP_SIOB_0_DCDS2_FEEDER_B_DIAG_DCDS2_FEEDER_B_DIAG_SHIFT  0
#define BCHP_SIOB_0_DCDS2_FEEDER_B_DIAG_DCDS2_FEEDER_B_DIAG_DEFAULT 0x00000000

/***************************************************************************
 *DCDS2_FEEDER_J_DIAG - Diagnostic port
 ***************************************************************************/
/* SIOB_0 :: DCDS2_FEEDER_J_DIAG :: DCDS2_FEEDER_J_DIAG [31:00] */
#define BCHP_SIOB_0_DCDS2_FEEDER_J_DIAG_DCDS2_FEEDER_J_DIAG_MASK   0xffffffff
#define BCHP_SIOB_0_DCDS2_FEEDER_J_DIAG_DCDS2_FEEDER_J_DIAG_SHIFT  0
#define BCHP_SIOB_0_DCDS2_FEEDER_J_DIAG_DCDS2_FEEDER_J_DIAG_DEFAULT 0x00000000

/***************************************************************************
 *DCDS2_FEEDER_G_DIAG - Diagnostic port
 ***************************************************************************/
/* SIOB_0 :: DCDS2_FEEDER_G_DIAG :: DCDS2_FEEDER_G_DIAG [31:00] */
#define BCHP_SIOB_0_DCDS2_FEEDER_G_DIAG_DCDS2_FEEDER_G_DIAG_MASK   0xffffffff
#define BCHP_SIOB_0_DCDS2_FEEDER_G_DIAG_DCDS2_FEEDER_G_DIAG_SHIFT  0
#define BCHP_SIOB_0_DCDS2_FEEDER_G_DIAG_DCDS2_FEEDER_G_DIAG_DEFAULT 0x00000000

/***************************************************************************
 *TEST_PORT_CONTROL - Test port control register
 ***************************************************************************/
/* SIOB_0 :: TEST_PORT_CONTROL :: reserved0 [31:03] */
#define BCHP_SIOB_0_TEST_PORT_CONTROL_reserved0_MASK               0xfffffff8
#define BCHP_SIOB_0_TEST_PORT_CONTROL_reserved0_SHIFT              3

/* SIOB_0 :: TEST_PORT_CONTROL :: TP_ADDR [02:00] */
#define BCHP_SIOB_0_TEST_PORT_CONTROL_TP_ADDR_MASK                 0x00000007
#define BCHP_SIOB_0_TEST_PORT_CONTROL_TP_ADDR_SHIFT                0
#define BCHP_SIOB_0_TEST_PORT_CONTROL_TP_ADDR_DEFAULT              0x00000000
#define BCHP_SIOB_0_TEST_PORT_CONTROL_TP_ADDR_SIOB_SEL0            0
#define BCHP_SIOB_0_TEST_PORT_CONTROL_TP_ADDR_SIOB_SEL1            1
#define BCHP_SIOB_0_TEST_PORT_CONTROL_TP_ADDR_SIOB_SEL2            2
#define BCHP_SIOB_0_TEST_PORT_CONTROL_TP_ADDR_SIOB_SEL3            3
#define BCHP_SIOB_0_TEST_PORT_CONTROL_TP_ADDR_SIOB_SEL4            4
#define BCHP_SIOB_0_TEST_PORT_CONTROL_TP_ADDR_SIOB_SEL5            5
#define BCHP_SIOB_0_TEST_PORT_CONTROL_TP_ADDR_SIOB_SEL6            6
#define BCHP_SIOB_0_TEST_PORT_CONTROL_TP_ADDR_SIOB_SEL7            7

/***************************************************************************
 *TEST_PORT_DATA - Test port data register
 ***************************************************************************/
/* union - case SIOB_TP_SEL0 [31:00] */
/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL0 :: reserved0 [31:30] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL0_reserved0_MASK     0xc0000000
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL0_reserved0_SHIFT    30

/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL0 :: FEEDER_G [29:20] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL0_FEEDER_G_MASK      0x3ff00000
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL0_FEEDER_G_SHIFT     20

/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL0 :: FEEDER_J [19:10] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL0_FEEDER_J_MASK      0x000ffc00
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL0_FEEDER_J_SHIFT     10

/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL0 :: FEEDER_B [09:00] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL0_FEEDER_B_MASK      0x000003ff
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL0_FEEDER_B_SHIFT     0

/* union - case SIOB_TP_SEL1 [31:00] */
/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL1 :: reserved0 [31:08] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL1_reserved0_MASK     0xffffff00
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL1_reserved0_SHIFT    8

/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL1 :: CAPTURE_D [07:00] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL1_CAPTURE_D_MASK     0x000000ff
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL1_CAPTURE_D_SHIFT    0

/* union - case SIOB_TP_SEL2 [31:00] */
/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL2 :: DCE_D_DEBUG [31:00] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL2_DCE_D_DEBUG_MASK   0xffffffff
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL2_DCE_D_DEBUG_SHIFT  0

/* union - case SIOB_TP_SEL3 [31:00] */
/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL3 :: DCD_B_DEBUG [31:00] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL3_DCD_B_DEBUG_MASK   0xffffffff
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL3_DCD_B_DEBUG_SHIFT  0

/* union - case SIOB_TP_SEL4 [31:00] */
/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL4 :: DCD_J_DEBUG [31:00] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL4_DCD_J_DEBUG_MASK   0xffffffff
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL4_DCD_J_DEBUG_SHIFT  0

/* union - case SIOB_TP_SEL5 [31:00] */
/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL5 :: DCD_G_DEBUG [31:00] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL5_DCD_G_DEBUG_MASK   0xffffffff
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL5_DCD_G_DEBUG_SHIFT  0

/* union - case SIOB_TP_SEL6 [31:00] */
/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL6 :: reserved0 [31:24] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL6_reserved0_MASK     0xff000000
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL6_reserved0_SHIFT    24

/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL6 :: AUTO_FEEDER_G [23:16] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL6_AUTO_FEEDER_G_MASK 0x00ff0000
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL6_AUTO_FEEDER_G_SHIFT 16

/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL6 :: AUTO_FEEDER_J [15:08] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL6_AUTO_FEEDER_J_MASK 0x0000ff00
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL6_AUTO_FEEDER_J_SHIFT 8

/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL6 :: AUTO_FEEDER_B [07:00] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL6_AUTO_FEEDER_B_MASK 0x000000ff
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL6_AUTO_FEEDER_B_SHIFT 0

/* union - case SIOB_TP_SEL7 [31:00] */
/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL7 :: reserved0 [31:16] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL7_reserved0_MASK     0xffff0000
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL7_reserved0_SHIFT    16

/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL7 :: QM_FIFO_CTRL [15:12] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL7_QM_FIFO_CTRL_MASK  0x0000f000
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL7_QM_FIFO_CTRL_SHIFT 12

/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL7 :: QM_CAPTURE [11:08] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL7_QM_CAPTURE_MASK    0x00000f00
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL7_QM_CAPTURE_SHIFT   8

/* SIOB_0 :: TEST_PORT_DATA :: SIOB_TP_SEL7 :: QM_FEEDER [07:00] */
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL7_QM_FEEDER_MASK     0x000000ff
#define BCHP_SIOB_0_TEST_PORT_DATA_SIOB_TP_SEL7_QM_FEEDER_SHIFT    0

#endif /* #ifndef BCHP_SIOB_0_H__ */

/* End of File */
