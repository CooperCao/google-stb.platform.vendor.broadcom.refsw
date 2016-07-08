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

#ifndef BCHP_AIF_WB_CAB_CORE_INTR2_H__
#define BCHP_AIF_WB_CAB_CORE_INTR2_H__

/***************************************************************************
 *AIF_WB_CAB_CORE_INTR2 - AIF WB CAB Interrupt Register Set
 ***************************************************************************/
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS    0x04200800 /* [RO] CPU interrupt Status Register */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET       0x04200804 /* [WO] CPU interrupt Set Register */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR     0x04200808 /* [WO] CPU interrupt Clear Register */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS 0x0420080c /* [RO] CPU interrupt Mask Status Register */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET  0x04200810 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR 0x04200814 /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS    0x04200818 /* [RO] PCI interrupt Status Register */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET       0x0420081c /* [WO] PCI interrupt Set Register */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR     0x04200820 /* [WO] PCI interrupt Clear Register */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS 0x04200824 /* [RO] PCI interrupt Mask Status Register */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET  0x04200828 /* [WO] PCI interrupt Mask Set Register */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR 0x0420082c /* [WO] PCI interrupt Mask Clear Register */

/***************************************************************************
 *CPU_STATUS - CPU interrupt Status Register
 ***************************************************************************/
/* AIF_WB_CAB_CORE_INTR2 :: CPU_STATUS :: reserved0 [31:06] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_reserved0_MASK       0xffffffc0
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_reserved0_SHIFT      6

/* AIF_WB_CAB_CORE_INTR2 :: CPU_STATUS :: RFAGC_INTR [05:05] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_RFAGC_INTR_MASK      0x00000020
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_RFAGC_INTR_SHIFT     5
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_RFAGC_INTR_DEFAULT   0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_STATUS :: CLPDTR_PONG_INTR [04:04] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_CLPDTR_PONG_INTR_MASK 0x00000010
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_CLPDTR_PONG_INTR_SHIFT 4
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_CLPDTR_PONG_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_STATUS :: CLPDTR_PING_INTR [03:03] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_CLPDTR_PING_INTR_MASK 0x00000008
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_CLPDTR_PING_INTR_SHIFT 3
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_CLPDTR_PING_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_STATUS :: CORR_CNT_DONE_INTR [02:02] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_CORR_CNT_DONE_INTR_MASK 0x00000004
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_CORR_CNT_DONE_INTR_SHIFT 2
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_CORR_CNT_DONE_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_STATUS :: TIMER1_INTR [01:01] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_TIMER1_INTR_MASK     0x00000002
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_TIMER1_INTR_SHIFT    1
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_TIMER1_INTR_DEFAULT  0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_STATUS :: TIMER0_INTR [00:00] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_TIMER0_INTR_MASK     0x00000001
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_TIMER0_INTR_SHIFT    0
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_STATUS_TIMER0_INTR_DEFAULT  0x00000000

/***************************************************************************
 *CPU_SET - CPU interrupt Set Register
 ***************************************************************************/
/* AIF_WB_CAB_CORE_INTR2 :: CPU_SET :: reserved0 [31:06] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_reserved0_MASK          0xffffffc0
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_reserved0_SHIFT         6

/* AIF_WB_CAB_CORE_INTR2 :: CPU_SET :: RFAGC_INTR [05:05] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_RFAGC_INTR_MASK         0x00000020
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_RFAGC_INTR_SHIFT        5
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_RFAGC_INTR_DEFAULT      0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_SET :: CLPDTR_PONG_INTR [04:04] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_CLPDTR_PONG_INTR_MASK   0x00000010
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_CLPDTR_PONG_INTR_SHIFT  4
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_CLPDTR_PONG_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_SET :: CLPDTR_PING_INTR [03:03] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_CLPDTR_PING_INTR_MASK   0x00000008
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_CLPDTR_PING_INTR_SHIFT  3
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_CLPDTR_PING_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_SET :: CORR_CNT_DONE_INTR [02:02] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_CORR_CNT_DONE_INTR_MASK 0x00000004
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_CORR_CNT_DONE_INTR_SHIFT 2
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_CORR_CNT_DONE_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_SET :: TIMER1_INTR [01:01] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_TIMER1_INTR_MASK        0x00000002
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_TIMER1_INTR_SHIFT       1
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_TIMER1_INTR_DEFAULT     0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_SET :: TIMER0_INTR [00:00] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_TIMER0_INTR_MASK        0x00000001
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_TIMER0_INTR_SHIFT       0
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_SET_TIMER0_INTR_DEFAULT     0x00000000

/***************************************************************************
 *CPU_CLEAR - CPU interrupt Clear Register
 ***************************************************************************/
/* AIF_WB_CAB_CORE_INTR2 :: CPU_CLEAR :: reserved0 [31:06] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_reserved0_MASK        0xffffffc0
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_reserved0_SHIFT       6

/* AIF_WB_CAB_CORE_INTR2 :: CPU_CLEAR :: RFAGC_INTR [05:05] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_RFAGC_INTR_MASK       0x00000020
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_RFAGC_INTR_SHIFT      5
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_RFAGC_INTR_DEFAULT    0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_CLEAR :: CLPDTR_PONG_INTR [04:04] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_CLPDTR_PONG_INTR_MASK 0x00000010
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_CLPDTR_PONG_INTR_SHIFT 4
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_CLPDTR_PONG_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_CLEAR :: CLPDTR_PING_INTR [03:03] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_CLPDTR_PING_INTR_MASK 0x00000008
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_CLPDTR_PING_INTR_SHIFT 3
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_CLPDTR_PING_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_CLEAR :: CORR_CNT_DONE_INTR [02:02] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_CORR_CNT_DONE_INTR_MASK 0x00000004
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_CORR_CNT_DONE_INTR_SHIFT 2
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_CORR_CNT_DONE_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_CLEAR :: TIMER1_INTR [01:01] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_TIMER1_INTR_MASK      0x00000002
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_TIMER1_INTR_SHIFT     1
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_TIMER1_INTR_DEFAULT   0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: CPU_CLEAR :: TIMER0_INTR [00:00] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_TIMER0_INTR_MASK      0x00000001
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_TIMER0_INTR_SHIFT     0
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_CLEAR_TIMER0_INTR_DEFAULT   0x00000000

/***************************************************************************
 *CPU_MASK_STATUS - CPU interrupt Mask Status Register
 ***************************************************************************/
/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_STATUS :: reserved0 [31:06] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_reserved0_MASK  0xffffffc0
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_reserved0_SHIFT 6

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_STATUS :: RFAGC_INTR [05:05] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_RFAGC_INTR_MASK 0x00000020
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_RFAGC_INTR_SHIFT 5
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_RFAGC_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_STATUS :: CLPDTR_PONG_INTR [04:04] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_CLPDTR_PONG_INTR_MASK 0x00000010
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_CLPDTR_PONG_INTR_SHIFT 4
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_CLPDTR_PONG_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_STATUS :: CLPDTR_PING_INTR [03:03] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_CLPDTR_PING_INTR_MASK 0x00000008
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_CLPDTR_PING_INTR_SHIFT 3
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_CLPDTR_PING_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_STATUS :: CORR_CNT_DONE_INTR [02:02] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_CORR_CNT_DONE_INTR_MASK 0x00000004
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_CORR_CNT_DONE_INTR_SHIFT 2
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_CORR_CNT_DONE_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_STATUS :: TIMER1_INTR [01:01] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_TIMER1_INTR_MASK 0x00000002
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_TIMER1_INTR_SHIFT 1
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_TIMER1_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_STATUS :: TIMER0_INTR [00:00] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_TIMER0_INTR_MASK 0x00000001
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_TIMER0_INTR_SHIFT 0
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_STATUS_TIMER0_INTR_DEFAULT 0x00000001

/***************************************************************************
 *CPU_MASK_SET - CPU interrupt Mask Set Register
 ***************************************************************************/
/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_SET :: reserved0 [31:06] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_reserved0_MASK     0xffffffc0
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_reserved0_SHIFT    6

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_SET :: RFAGC_INTR [05:05] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_RFAGC_INTR_MASK    0x00000020
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_RFAGC_INTR_SHIFT   5
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_RFAGC_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_SET :: CLPDTR_PONG_INTR [04:04] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_CLPDTR_PONG_INTR_MASK 0x00000010
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_CLPDTR_PONG_INTR_SHIFT 4
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_CLPDTR_PONG_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_SET :: CLPDTR_PING_INTR [03:03] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_CLPDTR_PING_INTR_MASK 0x00000008
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_CLPDTR_PING_INTR_SHIFT 3
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_CLPDTR_PING_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_SET :: CORR_CNT_DONE_INTR [02:02] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_CORR_CNT_DONE_INTR_MASK 0x00000004
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_CORR_CNT_DONE_INTR_SHIFT 2
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_CORR_CNT_DONE_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_SET :: TIMER1_INTR [01:01] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_TIMER1_INTR_MASK   0x00000002
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_TIMER1_INTR_SHIFT  1
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_TIMER1_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_SET :: TIMER0_INTR [00:00] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_TIMER0_INTR_MASK   0x00000001
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_TIMER0_INTR_SHIFT  0
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_SET_TIMER0_INTR_DEFAULT 0x00000001

/***************************************************************************
 *CPU_MASK_CLEAR - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_CLEAR :: reserved0 [31:06] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_reserved0_MASK   0xffffffc0
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_reserved0_SHIFT  6

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_CLEAR :: RFAGC_INTR [05:05] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_RFAGC_INTR_MASK  0x00000020
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_RFAGC_INTR_SHIFT 5
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_RFAGC_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_CLEAR :: CLPDTR_PONG_INTR [04:04] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_CLPDTR_PONG_INTR_MASK 0x00000010
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_CLPDTR_PONG_INTR_SHIFT 4
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_CLPDTR_PONG_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_CLEAR :: CLPDTR_PING_INTR [03:03] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_CLPDTR_PING_INTR_MASK 0x00000008
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_CLPDTR_PING_INTR_SHIFT 3
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_CLPDTR_PING_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_CLEAR :: CORR_CNT_DONE_INTR [02:02] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_CORR_CNT_DONE_INTR_MASK 0x00000004
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_CORR_CNT_DONE_INTR_SHIFT 2
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_CORR_CNT_DONE_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_CLEAR :: TIMER1_INTR [01:01] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_TIMER1_INTR_MASK 0x00000002
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_TIMER1_INTR_SHIFT 1
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_TIMER1_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: CPU_MASK_CLEAR :: TIMER0_INTR [00:00] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_TIMER0_INTR_MASK 0x00000001
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_TIMER0_INTR_SHIFT 0
#define BCHP_AIF_WB_CAB_CORE_INTR2_CPU_MASK_CLEAR_TIMER0_INTR_DEFAULT 0x00000001

/***************************************************************************
 *PCI_STATUS - PCI interrupt Status Register
 ***************************************************************************/
/* AIF_WB_CAB_CORE_INTR2 :: PCI_STATUS :: reserved0 [31:06] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_reserved0_MASK       0xffffffc0
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_reserved0_SHIFT      6

/* AIF_WB_CAB_CORE_INTR2 :: PCI_STATUS :: RFAGC_INTR [05:05] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_RFAGC_INTR_MASK      0x00000020
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_RFAGC_INTR_SHIFT     5
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_RFAGC_INTR_DEFAULT   0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_STATUS :: CLPDTR_PONG_INTR [04:04] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_CLPDTR_PONG_INTR_MASK 0x00000010
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_CLPDTR_PONG_INTR_SHIFT 4
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_CLPDTR_PONG_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_STATUS :: CLPDTR_PING_INTR [03:03] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_CLPDTR_PING_INTR_MASK 0x00000008
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_CLPDTR_PING_INTR_SHIFT 3
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_CLPDTR_PING_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_STATUS :: CORR_CNT_DONE_INTR [02:02] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_CORR_CNT_DONE_INTR_MASK 0x00000004
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_CORR_CNT_DONE_INTR_SHIFT 2
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_CORR_CNT_DONE_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_STATUS :: TIMER1_INTR [01:01] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_TIMER1_INTR_MASK     0x00000002
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_TIMER1_INTR_SHIFT    1
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_TIMER1_INTR_DEFAULT  0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_STATUS :: TIMER0_INTR [00:00] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_TIMER0_INTR_MASK     0x00000001
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_TIMER0_INTR_SHIFT    0
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_STATUS_TIMER0_INTR_DEFAULT  0x00000000

/***************************************************************************
 *PCI_SET - PCI interrupt Set Register
 ***************************************************************************/
/* AIF_WB_CAB_CORE_INTR2 :: PCI_SET :: reserved0 [31:06] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_reserved0_MASK          0xffffffc0
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_reserved0_SHIFT         6

/* AIF_WB_CAB_CORE_INTR2 :: PCI_SET :: RFAGC_INTR [05:05] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_RFAGC_INTR_MASK         0x00000020
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_RFAGC_INTR_SHIFT        5
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_RFAGC_INTR_DEFAULT      0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_SET :: CLPDTR_PONG_INTR [04:04] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_CLPDTR_PONG_INTR_MASK   0x00000010
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_CLPDTR_PONG_INTR_SHIFT  4
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_CLPDTR_PONG_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_SET :: CLPDTR_PING_INTR [03:03] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_CLPDTR_PING_INTR_MASK   0x00000008
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_CLPDTR_PING_INTR_SHIFT  3
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_CLPDTR_PING_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_SET :: CORR_CNT_DONE_INTR [02:02] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_CORR_CNT_DONE_INTR_MASK 0x00000004
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_CORR_CNT_DONE_INTR_SHIFT 2
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_CORR_CNT_DONE_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_SET :: TIMER1_INTR [01:01] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_TIMER1_INTR_MASK        0x00000002
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_TIMER1_INTR_SHIFT       1
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_TIMER1_INTR_DEFAULT     0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_SET :: TIMER0_INTR [00:00] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_TIMER0_INTR_MASK        0x00000001
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_TIMER0_INTR_SHIFT       0
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_SET_TIMER0_INTR_DEFAULT     0x00000000

/***************************************************************************
 *PCI_CLEAR - PCI interrupt Clear Register
 ***************************************************************************/
/* AIF_WB_CAB_CORE_INTR2 :: PCI_CLEAR :: reserved0 [31:06] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_reserved0_MASK        0xffffffc0
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_reserved0_SHIFT       6

/* AIF_WB_CAB_CORE_INTR2 :: PCI_CLEAR :: RFAGC_INTR [05:05] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_RFAGC_INTR_MASK       0x00000020
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_RFAGC_INTR_SHIFT      5
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_RFAGC_INTR_DEFAULT    0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_CLEAR :: CLPDTR_PONG_INTR [04:04] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_CLPDTR_PONG_INTR_MASK 0x00000010
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_CLPDTR_PONG_INTR_SHIFT 4
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_CLPDTR_PONG_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_CLEAR :: CLPDTR_PING_INTR [03:03] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_CLPDTR_PING_INTR_MASK 0x00000008
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_CLPDTR_PING_INTR_SHIFT 3
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_CLPDTR_PING_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_CLEAR :: CORR_CNT_DONE_INTR [02:02] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_CORR_CNT_DONE_INTR_MASK 0x00000004
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_CORR_CNT_DONE_INTR_SHIFT 2
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_CORR_CNT_DONE_INTR_DEFAULT 0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_CLEAR :: TIMER1_INTR [01:01] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_TIMER1_INTR_MASK      0x00000002
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_TIMER1_INTR_SHIFT     1
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_TIMER1_INTR_DEFAULT   0x00000000

/* AIF_WB_CAB_CORE_INTR2 :: PCI_CLEAR :: TIMER0_INTR [00:00] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_TIMER0_INTR_MASK      0x00000001
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_TIMER0_INTR_SHIFT     0
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_CLEAR_TIMER0_INTR_DEFAULT   0x00000000

/***************************************************************************
 *PCI_MASK_STATUS - PCI interrupt Mask Status Register
 ***************************************************************************/
/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_STATUS :: reserved0 [31:06] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_reserved0_MASK  0xffffffc0
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_reserved0_SHIFT 6

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_STATUS :: RFAGC_INTR [05:05] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_RFAGC_INTR_MASK 0x00000020
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_RFAGC_INTR_SHIFT 5
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_RFAGC_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_STATUS :: CLPDTR_PONG_INTR [04:04] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_CLPDTR_PONG_INTR_MASK 0x00000010
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_CLPDTR_PONG_INTR_SHIFT 4
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_CLPDTR_PONG_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_STATUS :: CLPDTR_PING_INTR [03:03] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_CLPDTR_PING_INTR_MASK 0x00000008
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_CLPDTR_PING_INTR_SHIFT 3
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_CLPDTR_PING_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_STATUS :: CORR_CNT_DONE_INTR [02:02] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_CORR_CNT_DONE_INTR_MASK 0x00000004
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_CORR_CNT_DONE_INTR_SHIFT 2
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_CORR_CNT_DONE_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_STATUS :: TIMER1_INTR [01:01] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_TIMER1_INTR_MASK 0x00000002
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_TIMER1_INTR_SHIFT 1
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_TIMER1_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_STATUS :: TIMER0_INTR [00:00] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_TIMER0_INTR_MASK 0x00000001
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_TIMER0_INTR_SHIFT 0
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_STATUS_TIMER0_INTR_DEFAULT 0x00000001

/***************************************************************************
 *PCI_MASK_SET - PCI interrupt Mask Set Register
 ***************************************************************************/
/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_SET :: reserved0 [31:06] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_reserved0_MASK     0xffffffc0
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_reserved0_SHIFT    6

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_SET :: RFAGC_INTR [05:05] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_RFAGC_INTR_MASK    0x00000020
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_RFAGC_INTR_SHIFT   5
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_RFAGC_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_SET :: CLPDTR_PONG_INTR [04:04] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_CLPDTR_PONG_INTR_MASK 0x00000010
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_CLPDTR_PONG_INTR_SHIFT 4
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_CLPDTR_PONG_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_SET :: CLPDTR_PING_INTR [03:03] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_CLPDTR_PING_INTR_MASK 0x00000008
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_CLPDTR_PING_INTR_SHIFT 3
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_CLPDTR_PING_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_SET :: CORR_CNT_DONE_INTR [02:02] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_CORR_CNT_DONE_INTR_MASK 0x00000004
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_CORR_CNT_DONE_INTR_SHIFT 2
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_CORR_CNT_DONE_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_SET :: TIMER1_INTR [01:01] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_TIMER1_INTR_MASK   0x00000002
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_TIMER1_INTR_SHIFT  1
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_TIMER1_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_SET :: TIMER0_INTR [00:00] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_TIMER0_INTR_MASK   0x00000001
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_TIMER0_INTR_SHIFT  0
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_SET_TIMER0_INTR_DEFAULT 0x00000001

/***************************************************************************
 *PCI_MASK_CLEAR - PCI interrupt Mask Clear Register
 ***************************************************************************/
/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_CLEAR :: reserved0 [31:06] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_reserved0_MASK   0xffffffc0
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_reserved0_SHIFT  6

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_CLEAR :: RFAGC_INTR [05:05] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_RFAGC_INTR_MASK  0x00000020
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_RFAGC_INTR_SHIFT 5
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_RFAGC_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_CLEAR :: CLPDTR_PONG_INTR [04:04] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_CLPDTR_PONG_INTR_MASK 0x00000010
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_CLPDTR_PONG_INTR_SHIFT 4
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_CLPDTR_PONG_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_CLEAR :: CLPDTR_PING_INTR [03:03] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_CLPDTR_PING_INTR_MASK 0x00000008
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_CLPDTR_PING_INTR_SHIFT 3
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_CLPDTR_PING_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_CLEAR :: CORR_CNT_DONE_INTR [02:02] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_CORR_CNT_DONE_INTR_MASK 0x00000004
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_CORR_CNT_DONE_INTR_SHIFT 2
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_CORR_CNT_DONE_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_CLEAR :: TIMER1_INTR [01:01] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_TIMER1_INTR_MASK 0x00000002
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_TIMER1_INTR_SHIFT 1
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_TIMER1_INTR_DEFAULT 0x00000001

/* AIF_WB_CAB_CORE_INTR2 :: PCI_MASK_CLEAR :: TIMER0_INTR [00:00] */
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_TIMER0_INTR_MASK 0x00000001
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_TIMER0_INTR_SHIFT 0
#define BCHP_AIF_WB_CAB_CORE_INTR2_PCI_MASK_CLEAR_TIMER0_INTR_DEFAULT 0x00000001

#endif /* #ifndef BCHP_AIF_WB_CAB_CORE_INTR2_H__ */

/* End of File */
