/****************************************************************************
 *     Copyright (c) 1999-2016, Broadcom Corporation
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
 * Date:           Generated on               Mon Feb  8 12:56:18 2016
 *                 Full Compile MD5 Checksum  7c463a9180016920b3e03273285ff33d
 *                     (minus title and desc)
 *                 MD5 Checksum               30fed0099690880293569d98807ed1d8
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     749
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_H__
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_H__

/***************************************************************************
 *XPT_WDMA_CPU_INTR_AGGREGATOR - CPU Interrupt Aggregator
 ***************************************************************************/
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS 0x00a68020 /* [RO] Interrupt Status Register */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W1_STATUS 0x00a68024 /* [RO] Interrupt Status Register */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS 0x00a68028 /* [RO] Interrupt Mask Status Register */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W1_MASK_STATUS 0x00a6802c /* [RO] Interrupt Mask Status Register */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET 0x00a68030 /* [WO] Interrupt Mask Set Register */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W1_MASK_SET 0x00a68034 /* [WO] Interrupt Mask Set Register */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR 0x00a68038 /* [WO] Interrupt Mask Clear Register */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W1_MASK_CLEAR 0x00a6803c /* [WO] Interrupt Mask Clear Register */

/***************************************************************************
 *INTR_W0_STATUS - Interrupt Status Register
 ***************************************************************************/
/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_STATUS :: reserved0 [31:04] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_reserved0_MASK 0xfffffff0
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_reserved0_SHIFT 4

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_STATUS :: PM_INTR [03:03] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_PM_INTR_MASK 0x00000008
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_PM_INTR_SHIFT 3

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_STATUS :: BTP_INTR [02:02] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_BTP_INTR_MASK 0x00000004
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_BTP_INTR_SHIFT 2
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_BTP_INTR_DEFAULT 0x00000000

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_STATUS :: OVERFLOW_INTR [01:01] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_OVERFLOW_INTR_MASK 0x00000002
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_OVERFLOW_INTR_SHIFT 1
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_OVERFLOW_INTR_DEFAULT 0x00000000

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_STATUS :: DESC_DONE_INTR [00:00] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_DESC_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_DESC_DONE_INTR_SHIFT 0
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_STATUS_DESC_DONE_INTR_DEFAULT 0x00000000

/***************************************************************************
 *INTR_W1_STATUS - Interrupt Status Register
 ***************************************************************************/
/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W1_STATUS :: reserved0 [31:00] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W1_STATUS_reserved0_MASK 0xffffffff
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W1_STATUS_reserved0_SHIFT 0

/***************************************************************************
 *INTR_W0_MASK_STATUS - Interrupt Mask Status Register
 ***************************************************************************/
/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_STATUS :: reserved0 [31:04] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_reserved0_MASK 0xfffffff0
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_reserved0_SHIFT 4

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_STATUS :: PM_INTR [03:03] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_PM_INTR_MASK 0x00000008
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_PM_INTR_SHIFT 3

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_STATUS :: BTP_INTR [02:02] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_BTP_INTR_MASK 0x00000004
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_BTP_INTR_SHIFT 2
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_BTP_INTR_DEFAULT 0x00000001

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_STATUS :: OVERFLOW_INTR [01:01] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_OVERFLOW_INTR_MASK 0x00000002
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_OVERFLOW_INTR_SHIFT 1
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_OVERFLOW_INTR_DEFAULT 0x00000001

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_STATUS :: DESC_DONE_INTR [00:00] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_DESC_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_DESC_DONE_INTR_SHIFT 0
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_STATUS_DESC_DONE_INTR_DEFAULT 0x00000001

/***************************************************************************
 *INTR_W1_MASK_STATUS - Interrupt Mask Status Register
 ***************************************************************************/
/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W1_MASK_STATUS :: reserved0 [31:00] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W1_MASK_STATUS_reserved0_MASK 0xffffffff
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W1_MASK_STATUS_reserved0_SHIFT 0

/***************************************************************************
 *INTR_W0_MASK_SET - Interrupt Mask Set Register
 ***************************************************************************/
/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_SET :: reserved0 [31:04] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_reserved0_MASK 0xfffffff0
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_reserved0_SHIFT 4

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_SET :: PM_INTR [03:03] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_PM_INTR_MASK 0x00000008
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_PM_INTR_SHIFT 3

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_SET :: BTP_INTR [02:02] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_BTP_INTR_MASK 0x00000004
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_BTP_INTR_SHIFT 2
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_BTP_INTR_DEFAULT 0x00000001

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_SET :: OVERFLOW_INTR [01:01] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_OVERFLOW_INTR_MASK 0x00000002
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_OVERFLOW_INTR_SHIFT 1
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_OVERFLOW_INTR_DEFAULT 0x00000001

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_SET :: DESC_DONE_INTR [00:00] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_DESC_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_DESC_DONE_INTR_SHIFT 0
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_SET_DESC_DONE_INTR_DEFAULT 0x00000001

/***************************************************************************
 *INTR_W1_MASK_SET - Interrupt Mask Set Register
 ***************************************************************************/
/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W1_MASK_SET :: reserved0 [31:00] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W1_MASK_SET_reserved0_MASK 0xffffffff
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W1_MASK_SET_reserved0_SHIFT 0

/***************************************************************************
 *INTR_W0_MASK_CLEAR - Interrupt Mask Clear Register
 ***************************************************************************/
/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_CLEAR :: reserved0 [31:04] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_reserved0_MASK 0xfffffff0
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_reserved0_SHIFT 4

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_CLEAR :: PM_INTR [03:03] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_PM_INTR_MASK 0x00000008
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_PM_INTR_SHIFT 3

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_CLEAR :: BTP_INTR [02:02] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_BTP_INTR_MASK 0x00000004
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_BTP_INTR_SHIFT 2
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_BTP_INTR_DEFAULT 0x00000001

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_CLEAR :: OVERFLOW_INTR [01:01] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_OVERFLOW_INTR_MASK 0x00000002
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_OVERFLOW_INTR_SHIFT 1
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_OVERFLOW_INTR_DEFAULT 0x00000001

/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W0_MASK_CLEAR :: DESC_DONE_INTR [00:00] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_DESC_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_DESC_DONE_INTR_SHIFT 0
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_DESC_DONE_INTR_DEFAULT 0x00000001

/***************************************************************************
 *INTR_W1_MASK_CLEAR - Interrupt Mask Clear Register
 ***************************************************************************/
/* XPT_WDMA_CPU_INTR_AGGREGATOR :: INTR_W1_MASK_CLEAR :: reserved0 [31:00] */
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W1_MASK_CLEAR_reserved0_MASK 0xffffffff
#define BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_INTR_W1_MASK_CLEAR_reserved0_SHIFT 0

#endif /* #ifndef BCHP_XPT_WDMA_CPU_INTR_AGGREGATOR_H__ */

/* End of File */
