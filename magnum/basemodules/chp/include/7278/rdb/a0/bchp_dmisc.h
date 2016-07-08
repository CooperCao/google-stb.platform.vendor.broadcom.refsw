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

#ifndef BCHP_DMISC_H__
#define BCHP_DMISC_H__

/***************************************************************************
 *DMISC - BVN Deinterlace Control Registers
 ***************************************************************************/
#define BCHP_DMISC_SW_INIT                       0x00080000 /* [CFG] BVN Deinterlace Soft Init */
#define BCHP_DMISC_BVND_CLOCK_CTRL               0x00080004 /* [CFG] BVN Deinterlace clock control register */
#define BCHP_DMISC_BVND_PDA_OUT_STATUS           0x00080008 /* [RO] BVN Deinterlace PDA Out Status */
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS        0x0008000c /* [RO] BVN Deinterlace PDA Power Up Status */
#define BCHP_DMISC_SCRATCH_0                     0x0008001c /* [CFG] Scratch Register */

/***************************************************************************
 *SW_INIT - BVN Deinterlace Soft Init
 ***************************************************************************/
/* DMISC :: SW_INIT :: reserved0 [31:04] */
#define BCHP_DMISC_SW_INIT_reserved0_MASK                          0xfffffff0
#define BCHP_DMISC_SW_INIT_reserved0_SHIFT                         4

/* DMISC :: SW_INIT :: MVP_3 [03:03] */
#define BCHP_DMISC_SW_INIT_MVP_3_MASK                              0x00000008
#define BCHP_DMISC_SW_INIT_MVP_3_SHIFT                             3
#define BCHP_DMISC_SW_INIT_MVP_3_DEFAULT                           0x00000000

/* DMISC :: SW_INIT :: MVP_2 [02:02] */
#define BCHP_DMISC_SW_INIT_MVP_2_MASK                              0x00000004
#define BCHP_DMISC_SW_INIT_MVP_2_SHIFT                             2
#define BCHP_DMISC_SW_INIT_MVP_2_DEFAULT                           0x00000000

/* DMISC :: SW_INIT :: MVP_1 [01:01] */
#define BCHP_DMISC_SW_INIT_MVP_1_MASK                              0x00000002
#define BCHP_DMISC_SW_INIT_MVP_1_SHIFT                             1
#define BCHP_DMISC_SW_INIT_MVP_1_DEFAULT                           0x00000000

/* DMISC :: SW_INIT :: MVP_0 [00:00] */
#define BCHP_DMISC_SW_INIT_MVP_0_MASK                              0x00000001
#define BCHP_DMISC_SW_INIT_MVP_0_SHIFT                             0
#define BCHP_DMISC_SW_INIT_MVP_0_DEFAULT                           0x00000000

/***************************************************************************
 *BVND_CLOCK_CTRL - BVN Deinterlace clock control register
 ***************************************************************************/
/* DMISC :: BVND_CLOCK_CTRL :: reserved0 [31:01] */
#define BCHP_DMISC_BVND_CLOCK_CTRL_reserved0_MASK                  0xfffffffe
#define BCHP_DMISC_BVND_CLOCK_CTRL_reserved0_SHIFT                 1

/* DMISC :: BVND_CLOCK_CTRL :: CLK_FREE_RUN_MODE [00:00] */
#define BCHP_DMISC_BVND_CLOCK_CTRL_CLK_FREE_RUN_MODE_MASK          0x00000001
#define BCHP_DMISC_BVND_CLOCK_CTRL_CLK_FREE_RUN_MODE_SHIFT         0
#define BCHP_DMISC_BVND_CLOCK_CTRL_CLK_FREE_RUN_MODE_DEFAULT       0x00000000

/***************************************************************************
 *BVND_PDA_OUT_STATUS - BVN Deinterlace PDA Out Status
 ***************************************************************************/
/* DMISC :: BVND_PDA_OUT_STATUS :: reserved0 [31:11] */
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_reserved0_MASK              0xfffff800
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_reserved0_SHIFT             11

/* DMISC :: BVND_PDA_OUT_STATUS :: MCVP_SIOB [10:10] */
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_MCVP_SIOB_MASK              0x00000400
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_MCVP_SIOB_SHIFT             10

/* DMISC :: BVND_PDA_OUT_STATUS :: MCTF [09:09] */
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_MCTF_MASK                   0x00000200
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_MCTF_SHIFT                  9

/* DMISC :: BVND_PDA_OUT_STATUS :: MCDI [08:08] */
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_MCDI_MASK                   0x00000100
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_MCDI_SHIFT                  8

/* DMISC :: BVND_PDA_OUT_STATUS :: MADR_0 [07:07] */
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_MADR_0_MASK                 0x00000080
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_MADR_0_SHIFT                7

/* DMISC :: BVND_PDA_OUT_STATUS :: MADR_1 [06:06] */
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_MADR_1_MASK                 0x00000040
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_MADR_1_SHIFT                6

/* DMISC :: BVND_PDA_OUT_STATUS :: MADR_2 [05:05] */
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_MADR_2_MASK                 0x00000020
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_MADR_2_SHIFT                5

/* DMISC :: BVND_PDA_OUT_STATUS :: reserved1 [04:00] */
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_reserved1_MASK              0x0000001f
#define BCHP_DMISC_BVND_PDA_OUT_STATUS_reserved1_SHIFT             0

/***************************************************************************
 *BVND_PDA_PWR_UP_STATUS - BVN Deinterlace PDA Power Up Status
 ***************************************************************************/
/* DMISC :: BVND_PDA_PWR_UP_STATUS :: reserved0 [31:11] */
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_reserved0_MASK           0xfffff800
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_reserved0_SHIFT          11

/* DMISC :: BVND_PDA_PWR_UP_STATUS :: MCVP_SIOB [10:10] */
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_MCVP_SIOB_MASK           0x00000400
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_MCVP_SIOB_SHIFT          10

/* DMISC :: BVND_PDA_PWR_UP_STATUS :: MCTF [09:09] */
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_MCTF_MASK                0x00000200
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_MCTF_SHIFT               9

/* DMISC :: BVND_PDA_PWR_UP_STATUS :: MCDI [08:08] */
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_MCDI_MASK                0x00000100
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_MCDI_SHIFT               8

/* DMISC :: BVND_PDA_PWR_UP_STATUS :: MADR_0 [07:07] */
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_MADR_0_MASK              0x00000080
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_MADR_0_SHIFT             7

/* DMISC :: BVND_PDA_PWR_UP_STATUS :: MADR_1 [06:06] */
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_MADR_1_MASK              0x00000040
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_MADR_1_SHIFT             6

/* DMISC :: BVND_PDA_PWR_UP_STATUS :: MADR_2 [05:05] */
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_MADR_2_MASK              0x00000020
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_MADR_2_SHIFT             5

/* DMISC :: BVND_PDA_PWR_UP_STATUS :: reserved1 [04:00] */
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_reserved1_MASK           0x0000001f
#define BCHP_DMISC_BVND_PDA_PWR_UP_STATUS_reserved1_SHIFT          0

/***************************************************************************
 *SCRATCH_0 - Scratch Register
 ***************************************************************************/
/* DMISC :: SCRATCH_0 :: VALUE [31:00] */
#define BCHP_DMISC_SCRATCH_0_VALUE_MASK                            0xffffffff
#define BCHP_DMISC_SCRATCH_0_VALUE_SHIFT                           0
#define BCHP_DMISC_SCRATCH_0_VALUE_DEFAULT                         0x00000000

#endif /* #ifndef BCHP_DMISC_H__ */

/* End of File */
