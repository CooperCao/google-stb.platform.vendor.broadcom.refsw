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
 * Date:           Generated on               Tue Feb 23 15:26:06 2016
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

#ifndef BCHP_JTAG_OTP_H__
#define BCHP_JTAG_OTP_H__

/***************************************************************************
 *JTAG_OTP - JTAG OTP Registers
 ***************************************************************************/
#define BCHP_JTAG_OTP_GENERAL_CTRL_0             0x04121000 /* [RW] General control register 0 */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1             0x04121004 /* [RW] General control register 1 */
#define BCHP_JTAG_OTP_GENERAL_CTRL_2             0x04121008 /* [RW] General control register 2 */
#define BCHP_JTAG_OTP_GENERAL_CTRL_3             0x0412100c /* [RW] General control register 2 */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4             0x04121010 /* [RW] General control register 2 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_0           0x04121014 /* [RO] General status register 0 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_1           0x04121018 /* [RO] General status register 1 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_2           0x0412101c /* [RO] General status register 2 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_3           0x04121020 /* [RO] General status register 3 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_4           0x04121024 /* [RO] General status register 4 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_5           0x04121028 /* [RO] General status register 5 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_6           0x0412102c /* [RO] General status register 6 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_7           0x04121030 /* [RO] General status register 7 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_8           0x04121034 /* [RO] General status register 8 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_9           0x04121038 /* [RO] General status register 9 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_10          0x0412103c /* [RO] General status register 10 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_11          0x04121040 /* [RO] General status register 11 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_12          0x04121044 /* [RO] General status register 12 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_13          0x04121048 /* [RO] General status register 13 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_14          0x0412104c /* [RO] General status register 14 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_15          0x04121050 /* [RO] General status register 15 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_16          0x04121054 /* [RO] General status register 16 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_17          0x04121058 /* [RO] General status register 17 */
#define BCHP_JTAG_OTP_GENERAL_STATUS_18          0x0412105c /* [RO] General status register 18 */

/***************************************************************************
 *GENERAL_CTRL_0 - General control register 0
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_31 [31:31] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_31_MASK         0x80000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_31_SHIFT        31
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_31_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_30 [30:30] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_30_MASK         0x40000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_30_SHIFT        30
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_30_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_29 [29:29] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_29_MASK         0x20000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_29_SHIFT        29
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_29_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_28 [28:28] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_28_MASK         0x10000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_28_SHIFT        28
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_28_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_27 [27:27] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_27_MASK         0x08000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_27_SHIFT        27
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_27_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_26 [26:26] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_26_MASK         0x04000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_26_SHIFT        26
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_26_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_25 [25:25] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_25_MASK         0x02000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_25_SHIFT        25
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_25_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_24 [24:24] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_24_MASK         0x01000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_24_SHIFT        24
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_24_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_23 [23:23] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_23_MASK         0x00800000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_23_SHIFT        23
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_23_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_22 [22:22] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_22_MASK         0x00400000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_22_SHIFT        22
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_22_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_21 [21:21] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_21_MASK         0x00200000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_21_SHIFT        21
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_21_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_20 [20:20] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_20_MASK         0x00100000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_20_SHIFT        20
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_20_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_19 [19:19] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_19_MASK         0x00080000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_19_SHIFT        19
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_19_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_18 [18:18] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_18_MASK         0x00040000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_18_SHIFT        18
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_18_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_17 [17:17] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_17_MASK         0x00020000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_17_SHIFT        17
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_17_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_16 [16:16] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_16_MASK         0x00010000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_16_SHIFT        16
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_16_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_15 [15:15] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_15_MASK         0x00008000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_15_SHIFT        15
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_15_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_14 [14:14] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_14_MASK         0x00004000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_14_SHIFT        14
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_14_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_13 [13:13] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_13_MASK         0x00002000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_13_SHIFT        13
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_13_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_12 [12:12] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_12_MASK         0x00001000
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_12_SHIFT        12
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_12_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_11 [11:11] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_11_MASK         0x00000800
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_11_SHIFT        11
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_11_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_10 [10:10] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_10_MASK         0x00000400
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_10_SHIFT        10
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_10_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_9 [09:09] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_9_MASK          0x00000200
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_9_SHIFT         9
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_9_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_8 [08:08] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_8_MASK          0x00000100
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_8_SHIFT         8
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_8_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_7 [07:07] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_7_MASK          0x00000080
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_7_SHIFT         7
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_7_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: general_ctrl0_6 [06:06] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_6_MASK          0x00000040
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_6_SHIFT         6
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_general_ctrl0_6_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: jtag_otp_ctrl_command [05:01] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_jtag_otp_ctrl_command_MASK    0x0000003e
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_jtag_otp_ctrl_command_SHIFT   1
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_jtag_otp_ctrl_command_DEFAULT 0x00000000

/* JTAG_OTP :: GENERAL_CTRL_0 :: cpu_cmd_wr_en [00:00] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_cpu_cmd_wr_en_MASK            0x00000001
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_cpu_cmd_wr_en_SHIFT           0
#define BCHP_JTAG_OTP_GENERAL_CTRL_0_cpu_cmd_wr_en_DEFAULT         0x00000000

/***************************************************************************
 *GENERAL_CTRL_1 - General control register 1
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_CTRL_1 :: reserved0 [31:16] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_reserved0_MASK                0xffff0000
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_reserved0_SHIFT               16

/* JTAG_OTP :: GENERAL_CTRL_1 :: general_ctrl1_15 [15:15] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_15_MASK         0x00008000
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_15_SHIFT        15
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_15_DEFAULT      0x00000001

/* JTAG_OTP :: GENERAL_CTRL_1 :: general_ctrl1_14 [14:14] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_14_MASK         0x00004000
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_14_SHIFT        14
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_14_DEFAULT      0x00000001

/* JTAG_OTP :: GENERAL_CTRL_1 :: general_ctrl1_13 [13:13] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_13_MASK         0x00002000
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_13_SHIFT        13
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_13_DEFAULT      0x00000001

/* JTAG_OTP :: GENERAL_CTRL_1 :: general_ctrl1_12 [12:12] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_12_MASK         0x00001000
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_12_SHIFT        12
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_12_DEFAULT      0x00000001

/* JTAG_OTP :: GENERAL_CTRL_1 :: general_ctrl1_11 [11:11] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_11_MASK         0x00000800
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_11_SHIFT        11
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_11_DEFAULT      0x00000001

/* JTAG_OTP :: GENERAL_CTRL_1 :: general_ctrl1_10 [10:10] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_10_MASK         0x00000400
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_10_SHIFT        10
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_10_DEFAULT      0x00000001

/* JTAG_OTP :: GENERAL_CTRL_1 :: general_ctrl1_9 [09:09] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_9_MASK          0x00000200
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_9_SHIFT         9
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_9_DEFAULT       0x00000001

/* JTAG_OTP :: GENERAL_CTRL_1 :: general_ctrl1_8 [08:08] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_8_MASK          0x00000100
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_8_SHIFT         8
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_8_DEFAULT       0x00000001

/* JTAG_OTP :: GENERAL_CTRL_1 :: general_ctrl1_7 [07:07] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_7_MASK          0x00000080
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_7_SHIFT         7
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_7_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_1 :: general_ctrl1_6 [06:06] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_6_MASK          0x00000040
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_6_SHIFT         6
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_general_ctrl1_6_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_1 :: cpu_sft_otp_reset [05:05] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_cpu_sft_otp_reset_MASK        0x00000020
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_cpu_sft_otp_reset_SHIFT       5
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_cpu_sft_otp_reset_DEFAULT     0x00000000

/* JTAG_OTP :: GENERAL_CTRL_1 :: cpu_sft_bisr_reset [04:04] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_cpu_sft_bisr_reset_MASK       0x00000010
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_cpu_sft_bisr_reset_SHIFT      4
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_cpu_sft_bisr_reset_DEFAULT    0x00000000

/* JTAG_OTP :: GENERAL_CTRL_1 :: sel_alternate_FuseBoxClock [03:03] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_sel_alternate_FuseBoxClock_MASK 0x00000008
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_sel_alternate_FuseBoxClock_SHIFT 3
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_sel_alternate_FuseBoxClock_DEFAULT 0x00000000

/* JTAG_OTP :: GENERAL_CTRL_1 :: cpu_disable_FuseBoxSelect [02:02] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_cpu_disable_FuseBoxSelect_MASK 0x00000004
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_cpu_disable_FuseBoxSelect_SHIFT 2
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_cpu_disable_FuseBoxSelect_DEFAULT 0x00000000

/* JTAG_OTP :: GENERAL_CTRL_1 :: cpu_disable_bisr_mode [01:01] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_cpu_disable_bisr_mode_MASK    0x00000002
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_cpu_disable_bisr_mode_SHIFT   1
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_cpu_disable_bisr_mode_DEFAULT 0x00000000

/* JTAG_OTP :: GENERAL_CTRL_1 :: jtag_otp_cpu_mode [00:00] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_jtag_otp_cpu_mode_MASK        0x00000001
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_jtag_otp_cpu_mode_SHIFT       0
#define BCHP_JTAG_OTP_GENERAL_CTRL_1_jtag_otp_cpu_mode_DEFAULT     0x00000000

/***************************************************************************
 *GENERAL_CTRL_2 - General control register 2
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_CTRL_2 :: jtag_otp_cpu_data [31:00] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_2_jtag_otp_cpu_data_MASK        0xffffffff
#define BCHP_JTAG_OTP_GENERAL_CTRL_2_jtag_otp_cpu_data_SHIFT       0
#define BCHP_JTAG_OTP_GENERAL_CTRL_2_jtag_otp_cpu_data_DEFAULT     0x00000000

/***************************************************************************
 *GENERAL_CTRL_3 - General control register 2
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_CTRL_3 :: reserved0 [31:16] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_3_reserved0_MASK                0xffff0000
#define BCHP_JTAG_OTP_GENERAL_CTRL_3_reserved0_SHIFT               16

/* JTAG_OTP :: GENERAL_CTRL_3 :: jtag_otp_cpu_addr [15:00] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_3_jtag_otp_cpu_addr_MASK        0x0000ffff
#define BCHP_JTAG_OTP_GENERAL_CTRL_3_jtag_otp_cpu_addr_SHIFT       0
#define BCHP_JTAG_OTP_GENERAL_CTRL_3_jtag_otp_cpu_addr_DEFAULT     0x00000000

/***************************************************************************
 *GENERAL_CTRL_4 - General control register 2
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_31 [31:31] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_31_MASK         0x80000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_31_SHIFT        31
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_31_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_30 [30:30] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_30_MASK         0x40000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_30_SHIFT        30
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_30_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_29 [29:29] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_29_MASK         0x20000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_29_SHIFT        29
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_29_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_28 [28:28] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_28_MASK         0x10000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_28_SHIFT        28
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_28_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_27 [27:27] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_27_MASK         0x08000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_27_SHIFT        27
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_27_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_26 [26:26] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_26_MASK         0x04000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_26_SHIFT        26
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_26_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_25 [25:25] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_25_MASK         0x02000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_25_SHIFT        25
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_25_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_24 [24:24] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_24_MASK         0x01000000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_24_SHIFT        24
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_24_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_23 [23:23] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_23_MASK         0x00800000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_23_SHIFT        23
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_23_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_22 [22:22] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_22_MASK         0x00400000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_22_SHIFT        22
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_22_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_21 [21:21] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_21_MASK         0x00200000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_21_SHIFT        21
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_21_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_20 [20:20] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_20_MASK         0x00100000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_20_SHIFT        20
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_20_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_19 [19:19] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_19_MASK         0x00080000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_19_SHIFT        19
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_19_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_18 [18:18] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_18_MASK         0x00040000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_18_SHIFT        18
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_18_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_17 [17:17] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_17_MASK         0x00020000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_17_SHIFT        17
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_17_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_16 [16:16] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_16_MASK         0x00010000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_16_SHIFT        16
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_16_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_15 [15:15] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_15_MASK         0x00008000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_15_SHIFT        15
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_15_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_14 [14:14] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_14_MASK         0x00004000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_14_SHIFT        14
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_14_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_13 [13:13] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_13_MASK         0x00002000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_13_SHIFT        13
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_13_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_12 [12:12] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_12_MASK         0x00001000
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_12_SHIFT        12
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_12_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_11 [11:11] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_11_MASK         0x00000800
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_11_SHIFT        11
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_11_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_10 [10:10] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_10_MASK         0x00000400
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_10_SHIFT        10
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_10_DEFAULT      0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_9 [09:09] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_9_MASK          0x00000200
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_9_SHIFT         9
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_9_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_8 [08:08] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_8_MASK          0x00000100
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_8_SHIFT         8
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_8_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_7 [07:07] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_7_MASK          0x00000080
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_7_SHIFT         7
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_7_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_6 [06:06] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_6_MASK          0x00000040
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_6_SHIFT         6
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_6_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_5 [05:05] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_5_MASK          0x00000020
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_5_SHIFT         5
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_5_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_4 [04:04] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_4_MASK          0x00000010
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_4_SHIFT         4
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_4_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_3 [03:03] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_3_MASK          0x00000008
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_3_SHIFT         3
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_3_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_2 [02:02] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_2_MASK          0x00000004
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_2_SHIFT         2
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_2_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_1 [01:01] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_1_MASK          0x00000002
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_1_SHIFT         1
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_1_DEFAULT       0x00000000

/* JTAG_OTP :: GENERAL_CTRL_4 :: general_ctrl4_0 [00:00] */
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_0_MASK          0x00000001
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_0_SHIFT         0
#define BCHP_JTAG_OTP_GENERAL_CTRL_4_general_ctrl4_0_DEFAULT       0x00000000

/***************************************************************************
 *GENERAL_STATUS_0 - General status register 0
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_0 :: jtag_otp_data_out [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_0_jtag_otp_data_out_MASK      0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_0_jtag_otp_data_out_SHIFT     0

/***************************************************************************
 *GENERAL_STATUS_1 - General status register 1
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_1 :: jtag_otp_cpu_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_1_jtag_otp_cpu_status_MASK    0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_1_jtag_otp_cpu_status_SHIFT   0

/***************************************************************************
 *GENERAL_STATUS_2 - General status register 2
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_2 :: jtag_otp_data_out [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_2_jtag_otp_data_out_MASK      0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_2_jtag_otp_data_out_SHIFT     0

/***************************************************************************
 *GENERAL_STATUS_3 - General status register 3
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_3 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_3_jtag_otp_status_MASK        0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_3_jtag_otp_status_SHIFT       0

/***************************************************************************
 *GENERAL_STATUS_4 - General status register 4
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_4 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_4_jtag_otp_status_MASK        0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_4_jtag_otp_status_SHIFT       0

/***************************************************************************
 *GENERAL_STATUS_5 - General status register 5
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_5 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_5_jtag_otp_status_MASK        0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_5_jtag_otp_status_SHIFT       0

/***************************************************************************
 *GENERAL_STATUS_6 - General status register 6
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_6 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_6_jtag_otp_status_MASK        0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_6_jtag_otp_status_SHIFT       0

/***************************************************************************
 *GENERAL_STATUS_7 - General status register 7
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_7 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_7_jtag_otp_status_MASK        0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_7_jtag_otp_status_SHIFT       0

/***************************************************************************
 *GENERAL_STATUS_8 - General status register 8
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_8 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_8_jtag_otp_status_MASK        0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_8_jtag_otp_status_SHIFT       0

/***************************************************************************
 *GENERAL_STATUS_9 - General status register 9
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_9 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_9_jtag_otp_status_MASK        0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_9_jtag_otp_status_SHIFT       0

/***************************************************************************
 *GENERAL_STATUS_10 - General status register 10
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_10 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_10_jtag_otp_status_MASK       0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_10_jtag_otp_status_SHIFT      0

/***************************************************************************
 *GENERAL_STATUS_11 - General status register 11
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_11 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_11_jtag_otp_status_MASK       0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_11_jtag_otp_status_SHIFT      0

/***************************************************************************
 *GENERAL_STATUS_12 - General status register 12
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_12 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_12_jtag_otp_status_MASK       0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_12_jtag_otp_status_SHIFT      0

/***************************************************************************
 *GENERAL_STATUS_13 - General status register 13
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_13 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_13_jtag_otp_status_MASK       0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_13_jtag_otp_status_SHIFT      0

/***************************************************************************
 *GENERAL_STATUS_14 - General status register 14
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_14 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_14_jtag_otp_status_MASK       0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_14_jtag_otp_status_SHIFT      0

/***************************************************************************
 *GENERAL_STATUS_15 - General status register 15
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_15 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_15_jtag_otp_status_MASK       0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_15_jtag_otp_status_SHIFT      0

/***************************************************************************
 *GENERAL_STATUS_16 - General status register 16
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_16 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_16_jtag_otp_status_MASK       0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_16_jtag_otp_status_SHIFT      0

/***************************************************************************
 *GENERAL_STATUS_17 - General status register 17
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_17 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_17_jtag_otp_status_MASK       0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_17_jtag_otp_status_SHIFT      0

/***************************************************************************
 *GENERAL_STATUS_18 - General status register 18
 ***************************************************************************/
/* JTAG_OTP :: GENERAL_STATUS_18 :: jtag_otp_status [31:00] */
#define BCHP_JTAG_OTP_GENERAL_STATUS_18_jtag_otp_status_MASK       0xffffffff
#define BCHP_JTAG_OTP_GENERAL_STATUS_18_jtag_otp_status_SHIFT      0

#endif /* #ifndef BCHP_JTAG_OTP_H__ */

/* End of File */
