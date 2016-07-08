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
 * Date:           Generated on               Mon Aug 24 11:29:32 2015
 *                 Full Compile MD5 Checksum  cecd4eac458fcdc4b77c82d0630f17be
 *                     (minus title and desc)
 *                 MD5 Checksum               c9a18191e1cdbfad4487ef21d91e95fc
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     126
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_H__
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_H__

/***************************************************************************
 *XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 - XPT sub-module soft init done interrupt
 ***************************************************************************/
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_STATUS 0x20a00000 /* [RO] CPU interrupt Status Register */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_SET 0x20a00004 /* [WO] CPU interrupt Set Register */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_CLEAR 0x20a00008 /* [WO] CPU interrupt Clear Register */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_STATUS 0x20a0000c /* [RO] CPU interrupt Mask Status Register */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_SET 0x20a00010 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_CLEAR 0x20a00014 /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_STATUS 0x20a00018 /* [RO] PCI interrupt Status Register */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_SET 0x20a0001c /* [WO] PCI interrupt Set Register */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_CLEAR 0x20a00020 /* [WO] PCI interrupt Clear Register */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_STATUS 0x20a00024 /* [RO] PCI interrupt Mask Status Register */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_SET 0x20a00028 /* [WO] PCI interrupt Mask Set Register */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_CLEAR 0x20a0002c /* [WO] PCI interrupt Mask Clear Register */

/***************************************************************************
 *CPU_STATUS - CPU interrupt Status Register
 ***************************************************************************/
/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_STATUS :: reserved0 [31:02] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_STATUS_reserved0_MASK 0xfffffffc
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_STATUS_reserved0_SHIFT 2

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_STATUS :: MEMDMA_MCPB_SOFT_INIT_DONE_INTR [01:01] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_STATUS_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000002
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_STATUS_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_SHIFT 1

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_STATUS :: XPT_MCPB_SOFT_INIT_DONE_INTR [00:00] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_STATUS_XPT_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_STATUS_XPT_MCPB_SOFT_INIT_DONE_INTR_SHIFT 0
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_STATUS_XPT_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000000

/***************************************************************************
 *CPU_SET - CPU interrupt Set Register
 ***************************************************************************/
/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_SET :: reserved0 [31:02] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_SET_reserved0_MASK 0xfffffffc
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_SET_reserved0_SHIFT 2

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_SET :: MEMDMA_MCPB_SOFT_INIT_DONE_INTR [01:01] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_SET_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000002
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_SET_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_SHIFT 1

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_SET :: XPT_MCPB_SOFT_INIT_DONE_INTR [00:00] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_SET_XPT_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_SET_XPT_MCPB_SOFT_INIT_DONE_INTR_SHIFT 0
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_SET_XPT_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000000

/***************************************************************************
 *CPU_CLEAR - CPU interrupt Clear Register
 ***************************************************************************/
/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_CLEAR :: reserved0 [31:02] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_CLEAR_reserved0_MASK 0xfffffffc
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_CLEAR_reserved0_SHIFT 2

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_CLEAR :: MEMDMA_MCPB_SOFT_INIT_DONE_INTR [01:01] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_CLEAR_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000002
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_CLEAR_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_SHIFT 1

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_CLEAR :: XPT_MCPB_SOFT_INIT_DONE_INTR [00:00] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_CLEAR_XPT_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_CLEAR_XPT_MCPB_SOFT_INIT_DONE_INTR_SHIFT 0
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_CLEAR_XPT_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000000

/***************************************************************************
 *CPU_MASK_STATUS - CPU interrupt Mask Status Register
 ***************************************************************************/
/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_MASK_STATUS :: reserved0 [31:02] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_STATUS_reserved0_MASK 0xfffffffc
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_STATUS_reserved0_SHIFT 2

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_MASK_STATUS :: MEMDMA_MCPB_SOFT_INIT_DONE_INTR [01:01] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_STATUS_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000002
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_STATUS_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_SHIFT 1
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_STATUS_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000001

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_MASK_STATUS :: XPT_MCPB_SOFT_INIT_DONE_INTR [00:00] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_STATUS_XPT_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_STATUS_XPT_MCPB_SOFT_INIT_DONE_INTR_SHIFT 0
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_STATUS_XPT_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000001

/***************************************************************************
 *CPU_MASK_SET - CPU interrupt Mask Set Register
 ***************************************************************************/
/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_MASK_SET :: reserved0 [31:02] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_SET_reserved0_MASK 0xfffffffc
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_SET_reserved0_SHIFT 2

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_MASK_SET :: MEMDMA_MCPB_SOFT_INIT_DONE_INTR [01:01] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_SET_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000002
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_SET_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_SHIFT 1
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_SET_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000001

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_MASK_SET :: XPT_MCPB_SOFT_INIT_DONE_INTR [00:00] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_SET_XPT_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_SET_XPT_MCPB_SOFT_INIT_DONE_INTR_SHIFT 0
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_SET_XPT_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000001

/***************************************************************************
 *CPU_MASK_CLEAR - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_MASK_CLEAR :: reserved0 [31:02] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_CLEAR_reserved0_MASK 0xfffffffc
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_CLEAR_reserved0_SHIFT 2

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_MASK_CLEAR :: MEMDMA_MCPB_SOFT_INIT_DONE_INTR [01:01] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_CLEAR_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000002
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_CLEAR_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_SHIFT 1
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_CLEAR_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000001

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: CPU_MASK_CLEAR :: XPT_MCPB_SOFT_INIT_DONE_INTR [00:00] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_CLEAR_XPT_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_CLEAR_XPT_MCPB_SOFT_INIT_DONE_INTR_SHIFT 0
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_CPU_MASK_CLEAR_XPT_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000001

/***************************************************************************
 *PCI_STATUS - PCI interrupt Status Register
 ***************************************************************************/
/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_STATUS :: reserved0 [31:02] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_STATUS_reserved0_MASK 0xfffffffc
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_STATUS_reserved0_SHIFT 2

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_STATUS :: MEMDMA_MCPB_SOFT_INIT_DONE_INTR [01:01] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_STATUS_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000002
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_STATUS_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_SHIFT 1

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_STATUS :: XPT_MCPB_SOFT_INIT_DONE_INTR [00:00] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_STATUS_XPT_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_STATUS_XPT_MCPB_SOFT_INIT_DONE_INTR_SHIFT 0
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_STATUS_XPT_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000000

/***************************************************************************
 *PCI_SET - PCI interrupt Set Register
 ***************************************************************************/
/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_SET :: reserved0 [31:02] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_SET_reserved0_MASK 0xfffffffc
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_SET_reserved0_SHIFT 2

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_SET :: MEMDMA_MCPB_SOFT_INIT_DONE_INTR [01:01] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_SET_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000002
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_SET_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_SHIFT 1

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_SET :: XPT_MCPB_SOFT_INIT_DONE_INTR [00:00] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_SET_XPT_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_SET_XPT_MCPB_SOFT_INIT_DONE_INTR_SHIFT 0
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_SET_XPT_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000000

/***************************************************************************
 *PCI_CLEAR - PCI interrupt Clear Register
 ***************************************************************************/
/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_CLEAR :: reserved0 [31:02] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_CLEAR_reserved0_MASK 0xfffffffc
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_CLEAR_reserved0_SHIFT 2

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_CLEAR :: MEMDMA_MCPB_SOFT_INIT_DONE_INTR [01:01] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_CLEAR_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000002
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_CLEAR_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_SHIFT 1

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_CLEAR :: XPT_MCPB_SOFT_INIT_DONE_INTR [00:00] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_CLEAR_XPT_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_CLEAR_XPT_MCPB_SOFT_INIT_DONE_INTR_SHIFT 0
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_CLEAR_XPT_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000000

/***************************************************************************
 *PCI_MASK_STATUS - PCI interrupt Mask Status Register
 ***************************************************************************/
/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_MASK_STATUS :: reserved0 [31:02] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_STATUS_reserved0_MASK 0xfffffffc
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_STATUS_reserved0_SHIFT 2

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_MASK_STATUS :: MEMDMA_MCPB_SOFT_INIT_DONE_INTR [01:01] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_STATUS_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000002
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_STATUS_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_SHIFT 1
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_STATUS_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000001

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_MASK_STATUS :: XPT_MCPB_SOFT_INIT_DONE_INTR [00:00] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_STATUS_XPT_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_STATUS_XPT_MCPB_SOFT_INIT_DONE_INTR_SHIFT 0
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_STATUS_XPT_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000001

/***************************************************************************
 *PCI_MASK_SET - PCI interrupt Mask Set Register
 ***************************************************************************/
/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_MASK_SET :: reserved0 [31:02] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_SET_reserved0_MASK 0xfffffffc
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_SET_reserved0_SHIFT 2

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_MASK_SET :: MEMDMA_MCPB_SOFT_INIT_DONE_INTR [01:01] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_SET_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000002
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_SET_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_SHIFT 1
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_SET_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000001

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_MASK_SET :: XPT_MCPB_SOFT_INIT_DONE_INTR [00:00] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_SET_XPT_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_SET_XPT_MCPB_SOFT_INIT_DONE_INTR_SHIFT 0
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_SET_XPT_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000001

/***************************************************************************
 *PCI_MASK_CLEAR - PCI interrupt Mask Clear Register
 ***************************************************************************/
/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_MASK_CLEAR :: reserved0 [31:02] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_CLEAR_reserved0_MASK 0xfffffffc
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_CLEAR_reserved0_SHIFT 2

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_MASK_CLEAR :: MEMDMA_MCPB_SOFT_INIT_DONE_INTR [01:01] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_CLEAR_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000002
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_CLEAR_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_SHIFT 1
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_CLEAR_MEMDMA_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000001

/* XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2 :: PCI_MASK_CLEAR :: XPT_MCPB_SOFT_INIT_DONE_INTR [00:00] */
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_CLEAR_XPT_MCPB_SOFT_INIT_DONE_INTR_MASK 0x00000001
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_CLEAR_XPT_MCPB_SOFT_INIT_DONE_INTR_SHIFT 0
#define BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_PCI_MASK_CLEAR_XPT_MCPB_SOFT_INIT_DONE_INTR_DEFAULT 0x00000001

#endif /* #ifndef BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DONE_INTR2_H__ */

/* End of File */
