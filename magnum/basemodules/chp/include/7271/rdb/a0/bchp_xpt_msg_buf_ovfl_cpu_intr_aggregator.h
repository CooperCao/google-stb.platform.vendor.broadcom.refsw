/****************************************************************************
 *     Copyright (c) 1999-2015, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Fri Aug 21 14:43:24 2015
 *                 Full Compile MD5 Checksum  6f40c93fa7adf1b7b596c84d59590a10
 *                     (minus title and desc)
 *                 MD5 Checksum               b1b8c76af39c441b8e9ab1ae2930543d
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     88
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

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
