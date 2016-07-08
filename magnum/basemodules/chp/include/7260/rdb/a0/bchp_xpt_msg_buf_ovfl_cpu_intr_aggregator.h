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
 * Date:           Generated on               Fri Feb 26 13:24:10 2016
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

#ifndef BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_H__
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_H__

/***************************************************************************
 *XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR
 ***************************************************************************/
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS 0x20a3fb20 /* [RO] Interrupt Status Register */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W3_STATUS 0x20a3fb24 /* [RO] Interrupt Status Register */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS 0x20a3fb28 /* [RO] Interrupt Mask Status Register */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W3_MASK_STATUS 0x20a3fb2c /* [RO] Interrupt Mask Status Register */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET 0x20a3fb30 /* [WO] Interrupt Mask Set Register */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W3_MASK_SET 0x20a3fb34 /* [WO] Interrupt Mask Set Register */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR 0x20a3fb38 /* [WO] Interrupt Mask Clear Register */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W3_MASK_CLEAR 0x20a3fb3c /* [WO] Interrupt Mask Clear Register */

/***************************************************************************
 *INTR_W2_STATUS - Interrupt Status Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_STATUS :: reserved0 [31:04] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_reserved0_MASK 0xfffffff0
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_reserved0_SHIFT 4

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_STATUS :: MSG_BUF_OVFL_INTR_96_127 [03:03] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_96_127_MASK 0x00000008
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_96_127_SHIFT 3
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_96_127_DEFAULT 0x00000000

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_STATUS :: MSG_BUF_OVFL_INTR_64_95 [02:02] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_64_95_MASK 0x00000004
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_64_95_SHIFT 2
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_64_95_DEFAULT 0x00000000

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_STATUS :: MSG_BUF_OVFL_INTR_32_63 [01:01] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_32_63_MASK 0x00000002
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_32_63_SHIFT 1
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_32_63_DEFAULT 0x00000000

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_STATUS :: MSG_BUF_OVFL_INTR_00_31 [00:00] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_00_31_MASK 0x00000001
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_00_31_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_STATUS_MSG_BUF_OVFL_INTR_00_31_DEFAULT 0x00000000

/***************************************************************************
 *INTR_W3_STATUS - Interrupt Status Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W3_STATUS :: reserved0 [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W3_STATUS_reserved0_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W3_STATUS_reserved0_SHIFT 0

/***************************************************************************
 *INTR_W2_MASK_STATUS - Interrupt Mask Status Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_STATUS :: reserved0 [31:04] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_reserved0_MASK 0xfffffff0
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_reserved0_SHIFT 4

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_STATUS :: MSG_BUF_OVFL_INTR_96_127 [03:03] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_MSG_BUF_OVFL_INTR_96_127_MASK 0x00000008
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_MSG_BUF_OVFL_INTR_96_127_SHIFT 3
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_MSG_BUF_OVFL_INTR_96_127_DEFAULT 0x00000001

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_STATUS :: MSG_BUF_OVFL_INTR_64_95 [02:02] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_MSG_BUF_OVFL_INTR_64_95_MASK 0x00000004
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_MSG_BUF_OVFL_INTR_64_95_SHIFT 2
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_MSG_BUF_OVFL_INTR_64_95_DEFAULT 0x00000001

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_STATUS :: MSG_BUF_OVFL_INTR_32_63 [01:01] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_MSG_BUF_OVFL_INTR_32_63_MASK 0x00000002
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_MSG_BUF_OVFL_INTR_32_63_SHIFT 1
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_MSG_BUF_OVFL_INTR_32_63_DEFAULT 0x00000001

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_STATUS :: MSG_BUF_OVFL_INTR_00_31 [00:00] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_MSG_BUF_OVFL_INTR_00_31_MASK 0x00000001
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_MSG_BUF_OVFL_INTR_00_31_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_STATUS_MSG_BUF_OVFL_INTR_00_31_DEFAULT 0x00000001

/***************************************************************************
 *INTR_W3_MASK_STATUS - Interrupt Mask Status Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W3_MASK_STATUS :: reserved0 [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W3_MASK_STATUS_reserved0_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W3_MASK_STATUS_reserved0_SHIFT 0

/***************************************************************************
 *INTR_W2_MASK_SET - Interrupt Mask Set Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_SET :: reserved0 [31:04] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_reserved0_MASK 0xfffffff0
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_reserved0_SHIFT 4

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_SET :: MSG_BUF_OVFL_INTR_96_127 [03:03] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_MSG_BUF_OVFL_INTR_96_127_MASK 0x00000008
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_MSG_BUF_OVFL_INTR_96_127_SHIFT 3
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_MSG_BUF_OVFL_INTR_96_127_DEFAULT 0x00000001

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_SET :: MSG_BUF_OVFL_INTR_64_95 [02:02] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_MSG_BUF_OVFL_INTR_64_95_MASK 0x00000004
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_MSG_BUF_OVFL_INTR_64_95_SHIFT 2
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_MSG_BUF_OVFL_INTR_64_95_DEFAULT 0x00000001

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_SET :: MSG_BUF_OVFL_INTR_32_63 [01:01] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_MSG_BUF_OVFL_INTR_32_63_MASK 0x00000002
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_MSG_BUF_OVFL_INTR_32_63_SHIFT 1
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_MSG_BUF_OVFL_INTR_32_63_DEFAULT 0x00000001

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_SET :: MSG_BUF_OVFL_INTR_00_31 [00:00] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_MSG_BUF_OVFL_INTR_00_31_MASK 0x00000001
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_MSG_BUF_OVFL_INTR_00_31_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_SET_MSG_BUF_OVFL_INTR_00_31_DEFAULT 0x00000001

/***************************************************************************
 *INTR_W3_MASK_SET - Interrupt Mask Set Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W3_MASK_SET :: reserved0 [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W3_MASK_SET_reserved0_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W3_MASK_SET_reserved0_SHIFT 0

/***************************************************************************
 *INTR_W2_MASK_CLEAR - Interrupt Mask Clear Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_CLEAR :: reserved0 [31:04] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_reserved0_MASK 0xfffffff0
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_reserved0_SHIFT 4

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_CLEAR :: MSG_BUF_OVFL_INTR_96_127 [03:03] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_MSG_BUF_OVFL_INTR_96_127_MASK 0x00000008
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_MSG_BUF_OVFL_INTR_96_127_SHIFT 3
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_MSG_BUF_OVFL_INTR_96_127_DEFAULT 0x00000001

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_CLEAR :: MSG_BUF_OVFL_INTR_64_95 [02:02] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_MSG_BUF_OVFL_INTR_64_95_MASK 0x00000004
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_MSG_BUF_OVFL_INTR_64_95_SHIFT 2
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_MSG_BUF_OVFL_INTR_64_95_DEFAULT 0x00000001

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_CLEAR :: MSG_BUF_OVFL_INTR_32_63 [01:01] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_MSG_BUF_OVFL_INTR_32_63_MASK 0x00000002
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_MSG_BUF_OVFL_INTR_32_63_SHIFT 1
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_MSG_BUF_OVFL_INTR_32_63_DEFAULT 0x00000001

/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W2_MASK_CLEAR :: MSG_BUF_OVFL_INTR_00_31 [00:00] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_MSG_BUF_OVFL_INTR_00_31_MASK 0x00000001
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_MSG_BUF_OVFL_INTR_00_31_SHIFT 0
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W2_MASK_CLEAR_MSG_BUF_OVFL_INTR_00_31_DEFAULT 0x00000001

/***************************************************************************
 *INTR_W3_MASK_CLEAR - Interrupt Mask Clear Register
 ***************************************************************************/
/* XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR :: INTR_W3_MASK_CLEAR :: reserved0 [31:00] */
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W3_MASK_CLEAR_reserved0_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_INTR_W3_MASK_CLEAR_reserved0_SHIFT 0

#endif /* #ifndef BCHP_XPT_MSG_BUF_OVFL_CPU_INTR_AGGREGATOR_H__ */

/* End of File */
