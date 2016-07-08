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
 * Date:           Generated on               Fri Feb 26 13:24:12 2016
 *                 Full Compile MD5 Checksum  1560bfee4f086d6e1d49e6bd3406a38d
 *                     (minus title and desc)
 *                 MD5 Checksum               8d7264bb382089f88abd2b1abb2a6340
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     823
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_H__
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_H__

/***************************************************************************
 *XPT_MSG_BUF_OVFL_INTR_00_31_L2
 ***************************************************************************/
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_STATUS 0x20a3fe00 /* [RO] CPU interrupt Status Register */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_SET 0x20a3fe04 /* [WO] CPU interrupt Set Register */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_CLEAR 0x20a3fe08 /* [WO] CPU interrupt Clear Register */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_MASK_STATUS 0x20a3fe0c /* [RO] CPU interrupt Mask Status Register */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_MASK_SET 0x20a3fe10 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_MASK_CLEAR 0x20a3fe14 /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_STATUS 0x20a3fe18 /* [RO] PCI interrupt Status Register */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_SET 0x20a3fe1c /* [WO] PCI interrupt Set Register */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_CLEAR 0x20a3fe20 /* [WO] PCI interrupt Clear Register */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_MASK_STATUS 0x20a3fe24 /* [RO] PCI interrupt Mask Status Register */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_MASK_SET 0x20a3fe28 /* [WO] PCI interrupt Mask Set Register */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_MASK_CLEAR 0x20a3fe2c /* [WO] PCI interrupt Mask Clear Register */

/***************************************************************************
 *W8_CPU_STATUS - CPU interrupt Status Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_INTR_00_31_L2 :: W8_CPU_STATUS :: BUF_OVFL_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_STATUS_BUF_OVFL_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_STATUS_BUF_OVFL_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_STATUS_BUF_OVFL_INTR_DEFAULT 0x00000000

/***************************************************************************
 *W8_CPU_SET - CPU interrupt Set Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_INTR_00_31_L2 :: W8_CPU_SET :: BUF_OVFL_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_SET_BUF_OVFL_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_SET_BUF_OVFL_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_SET_BUF_OVFL_INTR_DEFAULT 0x00000000

/***************************************************************************
 *W8_CPU_CLEAR - CPU interrupt Clear Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_INTR_00_31_L2 :: W8_CPU_CLEAR :: BUF_OVFL_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_CLEAR_BUF_OVFL_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_CLEAR_BUF_OVFL_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_CLEAR_BUF_OVFL_INTR_DEFAULT 0x00000000

/***************************************************************************
 *W8_CPU_MASK_STATUS - CPU interrupt Mask Status Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_INTR_00_31_L2 :: W8_CPU_MASK_STATUS :: BUF_OVFL_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_MASK_STATUS_BUF_OVFL_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_MASK_STATUS_BUF_OVFL_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_MASK_STATUS_BUF_OVFL_INTR_DEFAULT 0x00000001

/***************************************************************************
 *W8_CPU_MASK_SET - CPU interrupt Mask Set Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_INTR_00_31_L2 :: W8_CPU_MASK_SET :: BUF_OVFL_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_MASK_SET_BUF_OVFL_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_MASK_SET_BUF_OVFL_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_MASK_SET_BUF_OVFL_INTR_DEFAULT 0x00000001

/***************************************************************************
 *W8_CPU_MASK_CLEAR - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_INTR_00_31_L2 :: W8_CPU_MASK_CLEAR :: BUF_OVFL_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_MASK_CLEAR_BUF_OVFL_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_MASK_CLEAR_BUF_OVFL_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_CPU_MASK_CLEAR_BUF_OVFL_INTR_DEFAULT 0x00000001

/***************************************************************************
 *W8_PCI_STATUS - PCI interrupt Status Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_INTR_00_31_L2 :: W8_PCI_STATUS :: BUF_OVFL_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_STATUS_BUF_OVFL_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_STATUS_BUF_OVFL_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_STATUS_BUF_OVFL_INTR_DEFAULT 0x00000000

/***************************************************************************
 *W8_PCI_SET - PCI interrupt Set Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_INTR_00_31_L2 :: W8_PCI_SET :: BUF_OVFL_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_SET_BUF_OVFL_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_SET_BUF_OVFL_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_SET_BUF_OVFL_INTR_DEFAULT 0x00000000

/***************************************************************************
 *W8_PCI_CLEAR - PCI interrupt Clear Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_INTR_00_31_L2 :: W8_PCI_CLEAR :: BUF_OVFL_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_CLEAR_BUF_OVFL_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_CLEAR_BUF_OVFL_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_CLEAR_BUF_OVFL_INTR_DEFAULT 0x00000000

/***************************************************************************
 *W8_PCI_MASK_STATUS - PCI interrupt Mask Status Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_INTR_00_31_L2 :: W8_PCI_MASK_STATUS :: BUF_OVFL_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_MASK_STATUS_BUF_OVFL_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_MASK_STATUS_BUF_OVFL_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_MASK_STATUS_BUF_OVFL_INTR_DEFAULT 0x00000001

/***************************************************************************
 *W8_PCI_MASK_SET - PCI interrupt Mask Set Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_INTR_00_31_L2 :: W8_PCI_MASK_SET :: BUF_OVFL_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_MASK_SET_BUF_OVFL_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_MASK_SET_BUF_OVFL_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_MASK_SET_BUF_OVFL_INTR_DEFAULT 0x00000001

/***************************************************************************
 *W8_PCI_MASK_CLEAR - PCI interrupt Mask Clear Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_INTR_00_31_L2 :: W8_PCI_MASK_CLEAR :: BUF_OVFL_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_MASK_CLEAR_BUF_OVFL_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_MASK_CLEAR_BUF_OVFL_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_W8_PCI_MASK_CLEAR_BUF_OVFL_INTR_DEFAULT 0x00000001

#endif /* #ifndef BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_L2_H__ */

/* End of File */
