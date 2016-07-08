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
 * Date:           Generated on               Fri Mar 11 11:27:45 2016
 *                 Full Compile MD5 Checksum  bd2422d99eb64ca1a813d44f334c2142
 *                     (minus title and desc)
 *                 MD5 Checksum               4b0080d75b229c7646e83938b11f658f
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

#ifndef BCHP_RF4CE_CPU_HOST_STB_L2_UB_H__
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_H__

/***************************************************************************
 *RF4CE_CPU_HOST_STB_L2_UB - Host STB L2 Interrupt Controller Registers
 ***************************************************************************/
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0 0x03980500 /* [RO] CPU interrupt Status Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0   0x03980504 /* [WO] CPU interrupt Set Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0 0x03980508 /* [WO] CPU interrupt Clear Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0 0x0398050c /* [RO] CPU interrupt Mask Status Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0 0x03980510 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0 0x03980514 /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1 0x03980518 /* [RO] Host Interrupt Status Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1   0x0398051c /* [WO] Host Interrupt Set Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1 0x03980520 /* [WO] Host Interrupt Clear Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1 0x03980524 /* [RO] Host Interrupt Mask Status Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1 0x03980528 /* [WO] Host Interrupt Mask Set Register */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1 0x0398052c /* [WO] Host Interrupt Mask Clear Register */

/***************************************************************************
 *CPU_STATUS0 - CPU interrupt Status Register
 ***************************************************************************/
/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_STATUS0 :: SW_INTR [31:04] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_SW_INTR_MASK     0xfffffff0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_SW_INTR_SHIFT    4
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_SW_INTR_DEFAULT  0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_STATUS0 :: MBOX_SEM_INTR [03:03] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_MBOX_SEM_INTR_MASK 0x00000008
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_MBOX_SEM_INTR_SHIFT 3
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_MBOX_SEM_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_STATUS0 :: WDOG_RST_INTR [02:02] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_WDOG_RST_INTR_MASK 0x00000004
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_WDOG_RST_INTR_SHIFT 2
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_WDOG_RST_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_STATUS0 :: WDOG_INTR [01:01] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_WDOG_INTR_MASK   0x00000002
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_WDOG_INTR_SHIFT  1
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_WDOG_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_STATUS0 :: MBOX_Z2H_FULL_INTR [00:00] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_MBOX_Z2H_FULL_INTR_MASK 0x00000001
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_MBOX_Z2H_FULL_INTR_SHIFT 0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS0_MBOX_Z2H_FULL_INTR_DEFAULT 0x00000000

/***************************************************************************
 *CPU_SET0 - CPU interrupt Set Register
 ***************************************************************************/
/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_SET0 :: SW_INTR [31:04] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_SW_INTR_MASK        0xfffffff0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_SW_INTR_SHIFT       4
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_SW_INTR_DEFAULT     0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_SET0 :: MBOX_SEM_INTR [03:03] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_MBOX_SEM_INTR_MASK  0x00000008
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_MBOX_SEM_INTR_SHIFT 3
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_MBOX_SEM_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_SET0 :: WDOG_RST_INTR [02:02] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_WDOG_RST_INTR_MASK  0x00000004
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_WDOG_RST_INTR_SHIFT 2
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_WDOG_RST_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_SET0 :: WDOG_INTR [01:01] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_WDOG_INTR_MASK      0x00000002
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_WDOG_INTR_SHIFT     1
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_WDOG_INTR_DEFAULT   0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_SET0 :: MBOX_Z2H_FULL_INTR [00:00] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_MBOX_Z2H_FULL_INTR_MASK 0x00000001
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_MBOX_Z2H_FULL_INTR_SHIFT 0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET0_MBOX_Z2H_FULL_INTR_DEFAULT 0x00000000

/***************************************************************************
 *CPU_CLEAR0 - CPU interrupt Clear Register
 ***************************************************************************/
/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_CLEAR0 :: SW_INTR [31:04] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_SW_INTR_MASK      0xfffffff0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_SW_INTR_SHIFT     4
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_SW_INTR_DEFAULT   0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_CLEAR0 :: MBOX_SEM_INTR [03:03] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_MBOX_SEM_INTR_MASK 0x00000008
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_MBOX_SEM_INTR_SHIFT 3
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_MBOX_SEM_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_CLEAR0 :: WDOG_RST_INTR [02:02] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_WDOG_RST_INTR_MASK 0x00000004
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_WDOG_RST_INTR_SHIFT 2
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_WDOG_RST_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_CLEAR0 :: WDOG_INTR [01:01] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_WDOG_INTR_MASK    0x00000002
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_WDOG_INTR_SHIFT   1
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_WDOG_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_CLEAR0 :: MBOX_Z2H_FULL_INTR [00:00] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_MBOX_Z2H_FULL_INTR_MASK 0x00000001
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_MBOX_Z2H_FULL_INTR_SHIFT 0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR0_MBOX_Z2H_FULL_INTR_DEFAULT 0x00000000

/***************************************************************************
 *CPU_MASK_STATUS0 - CPU interrupt Mask Status Register
 ***************************************************************************/
/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_STATUS0 :: SW_INTR [31:04] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_SW_INTR_MASK 0xfffffff0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_SW_INTR_SHIFT 4
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_SW_INTR_DEFAULT 0x000000ff

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_STATUS0 :: MBOX_SEM_INTR [03:03] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_MBOX_SEM_INTR_MASK 0x00000008
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_MBOX_SEM_INTR_SHIFT 3
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_MBOX_SEM_INTR_DEFAULT 0x00000001

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_STATUS0 :: WDOG_RST_INTR [02:02] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_WDOG_RST_INTR_MASK 0x00000004
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_WDOG_RST_INTR_SHIFT 2
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_WDOG_RST_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_STATUS0 :: WDOG_INTR [01:01] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_WDOG_INTR_MASK 0x00000002
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_WDOG_INTR_SHIFT 1
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_WDOG_INTR_DEFAULT 0x00000001

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_STATUS0 :: MBOX_Z2H_FULL_INTR [00:00] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_MBOX_Z2H_FULL_INTR_MASK 0x00000001
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_MBOX_Z2H_FULL_INTR_SHIFT 0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS0_MBOX_Z2H_FULL_INTR_DEFAULT 0x00000001

/***************************************************************************
 *CPU_MASK_SET0 - CPU interrupt Mask Set Register
 ***************************************************************************/
/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_SET0 :: SW_INTR [31:04] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_SW_INTR_MASK   0xfffffff0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_SW_INTR_SHIFT  4
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_SW_INTR_DEFAULT 0x000000ff

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_SET0 :: MBOX_SEM_INTR [03:03] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_MBOX_SEM_INTR_MASK 0x00000008
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_MBOX_SEM_INTR_SHIFT 3
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_MBOX_SEM_INTR_DEFAULT 0x00000001

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_SET0 :: WDOG_RST_INTR [02:02] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_WDOG_RST_INTR_MASK 0x00000004
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_WDOG_RST_INTR_SHIFT 2
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_WDOG_RST_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_SET0 :: WDOG_INTR [01:01] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_WDOG_INTR_MASK 0x00000002
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_WDOG_INTR_SHIFT 1
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_WDOG_INTR_DEFAULT 0x00000001

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_SET0 :: MBOX_Z2H_FULL_INTR [00:00] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_MBOX_Z2H_FULL_INTR_MASK 0x00000001
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_MBOX_Z2H_FULL_INTR_SHIFT 0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET0_MBOX_Z2H_FULL_INTR_DEFAULT 0x00000001

/***************************************************************************
 *CPU_MASK_CLEAR0 - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_CLEAR0 :: SW_INTR [31:04] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_SW_INTR_MASK 0xfffffff0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_SW_INTR_SHIFT 4
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_SW_INTR_DEFAULT 0x000000ff

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_CLEAR0 :: MBOX_SEM_INTR [03:03] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_MBOX_SEM_INTR_MASK 0x00000008
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_MBOX_SEM_INTR_SHIFT 3
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_MBOX_SEM_INTR_DEFAULT 0x00000001

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_CLEAR0 :: WDOG_RST_INTR [02:02] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_WDOG_RST_INTR_MASK 0x00000004
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_WDOG_RST_INTR_SHIFT 2
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_WDOG_RST_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_CLEAR0 :: WDOG_INTR [01:01] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_WDOG_INTR_MASK 0x00000002
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_WDOG_INTR_SHIFT 1
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_WDOG_INTR_DEFAULT 0x00000001

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_CLEAR0 :: MBOX_Z2H_FULL_INTR [00:00] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_MBOX_Z2H_FULL_INTR_MASK 0x00000001
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_MBOX_Z2H_FULL_INTR_SHIFT 0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR0_MBOX_Z2H_FULL_INTR_DEFAULT 0x00000001

/***************************************************************************
 *CPU_STATUS1 - Host Interrupt Status Register
 ***************************************************************************/
/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_STATUS1 :: SW_INTR [31:04] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_SW_INTR_MASK     0xfffffff0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_SW_INTR_SHIFT    4
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_SW_INTR_DEFAULT  0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_STATUS1 :: MBOX_SEM_INTR [03:03] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_MBOX_SEM_INTR_MASK 0x00000008
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_MBOX_SEM_INTR_SHIFT 3
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_MBOX_SEM_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_STATUS1 :: WDOG_RST_INTR [02:02] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_WDOG_RST_INTR_MASK 0x00000004
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_WDOG_RST_INTR_SHIFT 2
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_WDOG_RST_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_STATUS1 :: WDOG_INTR [01:01] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_WDOG_INTR_MASK   0x00000002
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_WDOG_INTR_SHIFT  1
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_WDOG_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_STATUS1 :: MBOX_Z2H_FULL_INTR [00:00] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_MBOX_Z2H_FULL_INTR_MASK 0x00000001
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_MBOX_Z2H_FULL_INTR_SHIFT 0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_STATUS1_MBOX_Z2H_FULL_INTR_DEFAULT 0x00000000

/***************************************************************************
 *CPU_SET1 - Host Interrupt Set Register
 ***************************************************************************/
/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_SET1 :: SW_INTR [31:04] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_SW_INTR_MASK        0xfffffff0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_SW_INTR_SHIFT       4
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_SW_INTR_DEFAULT     0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_SET1 :: MBOX_SEM_INTR [03:03] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_MBOX_SEM_INTR_MASK  0x00000008
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_MBOX_SEM_INTR_SHIFT 3
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_MBOX_SEM_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_SET1 :: WDOG_RST_INTR [02:02] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_WDOG_RST_INTR_MASK  0x00000004
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_WDOG_RST_INTR_SHIFT 2
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_WDOG_RST_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_SET1 :: WDOG_INTR [01:01] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_WDOG_INTR_MASK      0x00000002
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_WDOG_INTR_SHIFT     1
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_WDOG_INTR_DEFAULT   0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_SET1 :: MBOX_Z2H_FULL_INTR [00:00] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_MBOX_Z2H_FULL_INTR_MASK 0x00000001
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_MBOX_Z2H_FULL_INTR_SHIFT 0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_SET1_MBOX_Z2H_FULL_INTR_DEFAULT 0x00000000

/***************************************************************************
 *CPU_CLEAR1 - Host Interrupt Clear Register
 ***************************************************************************/
/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_CLEAR1 :: SW_INTR [31:04] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_SW_INTR_MASK      0xfffffff0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_SW_INTR_SHIFT     4
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_SW_INTR_DEFAULT   0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_CLEAR1 :: MBOX_SEM_INTR [03:03] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_MBOX_SEM_INTR_MASK 0x00000008
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_MBOX_SEM_INTR_SHIFT 3
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_MBOX_SEM_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_CLEAR1 :: WDOG_RST_INTR [02:02] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_WDOG_RST_INTR_MASK 0x00000004
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_WDOG_RST_INTR_SHIFT 2
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_WDOG_RST_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_CLEAR1 :: WDOG_INTR [01:01] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_WDOG_INTR_MASK    0x00000002
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_WDOG_INTR_SHIFT   1
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_WDOG_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_CLEAR1 :: MBOX_Z2H_FULL_INTR [00:00] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_MBOX_Z2H_FULL_INTR_MASK 0x00000001
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_MBOX_Z2H_FULL_INTR_SHIFT 0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_CLEAR1_MBOX_Z2H_FULL_INTR_DEFAULT 0x00000000

/***************************************************************************
 *CPU_MASK_STATUS1 - Host Interrupt Mask Status Register
 ***************************************************************************/
/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_STATUS1 :: SW_INTR [31:04] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_SW_INTR_MASK 0xfffffff0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_SW_INTR_SHIFT 4
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_SW_INTR_DEFAULT 0x000000ff

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_STATUS1 :: MBOX_SEM_INTR [03:03] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_MBOX_SEM_INTR_MASK 0x00000008
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_MBOX_SEM_INTR_SHIFT 3
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_MBOX_SEM_INTR_DEFAULT 0x00000001

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_STATUS1 :: WDOG_RST_INTR [02:02] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_WDOG_RST_INTR_MASK 0x00000004
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_WDOG_RST_INTR_SHIFT 2
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_WDOG_RST_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_STATUS1 :: WDOG_INTR [01:01] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_WDOG_INTR_MASK 0x00000002
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_WDOG_INTR_SHIFT 1
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_WDOG_INTR_DEFAULT 0x00000001

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_STATUS1 :: MBOX_Z2H_FULL_INTR [00:00] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_MBOX_Z2H_FULL_INTR_MASK 0x00000001
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_MBOX_Z2H_FULL_INTR_SHIFT 0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_STATUS1_MBOX_Z2H_FULL_INTR_DEFAULT 0x00000001

/***************************************************************************
 *CPU_MASK_SET1 - Host Interrupt Mask Set Register
 ***************************************************************************/
/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_SET1 :: SW_INTR [31:04] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_SW_INTR_MASK   0xfffffff0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_SW_INTR_SHIFT  4
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_SW_INTR_DEFAULT 0x000000ff

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_SET1 :: MBOX_SEM_INTR [03:03] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_MBOX_SEM_INTR_MASK 0x00000008
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_MBOX_SEM_INTR_SHIFT 3
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_MBOX_SEM_INTR_DEFAULT 0x00000001

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_SET1 :: WDOG_RST_INTR [02:02] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_WDOG_RST_INTR_MASK 0x00000004
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_WDOG_RST_INTR_SHIFT 2
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_WDOG_RST_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_SET1 :: WDOG_INTR [01:01] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_WDOG_INTR_MASK 0x00000002
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_WDOG_INTR_SHIFT 1
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_WDOG_INTR_DEFAULT 0x00000001

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_SET1 :: MBOX_Z2H_FULL_INTR [00:00] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_MBOX_Z2H_FULL_INTR_MASK 0x00000001
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_MBOX_Z2H_FULL_INTR_SHIFT 0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_SET1_MBOX_Z2H_FULL_INTR_DEFAULT 0x00000001

/***************************************************************************
 *CPU_MASK_CLEAR1 - Host Interrupt Mask Clear Register
 ***************************************************************************/
/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_CLEAR1 :: SW_INTR [31:04] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_SW_INTR_MASK 0xfffffff0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_SW_INTR_SHIFT 4
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_SW_INTR_DEFAULT 0x000000ff

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_CLEAR1 :: MBOX_SEM_INTR [03:03] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_MBOX_SEM_INTR_MASK 0x00000008
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_MBOX_SEM_INTR_SHIFT 3
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_MBOX_SEM_INTR_DEFAULT 0x00000001

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_CLEAR1 :: WDOG_RST_INTR [02:02] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_WDOG_RST_INTR_MASK 0x00000004
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_WDOG_RST_INTR_SHIFT 2
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_WDOG_RST_INTR_DEFAULT 0x00000000

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_CLEAR1 :: WDOG_INTR [01:01] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_WDOG_INTR_MASK 0x00000002
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_WDOG_INTR_SHIFT 1
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_WDOG_INTR_DEFAULT 0x00000001

/* RF4CE_CPU_HOST_STB_L2_UB :: CPU_MASK_CLEAR1 :: MBOX_Z2H_FULL_INTR [00:00] */
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_MBOX_Z2H_FULL_INTR_MASK 0x00000001
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_MBOX_Z2H_FULL_INTR_SHIFT 0
#define BCHP_RF4CE_CPU_HOST_STB_L2_UB_CPU_MASK_CLEAR1_MBOX_Z2H_FULL_INTR_DEFAULT 0x00000001

#endif /* #ifndef BCHP_RF4CE_CPU_HOST_STB_L2_UB_H__ */

/* End of File */
