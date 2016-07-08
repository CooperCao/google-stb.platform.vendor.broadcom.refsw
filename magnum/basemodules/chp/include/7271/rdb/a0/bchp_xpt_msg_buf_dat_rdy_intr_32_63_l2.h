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
 * Date:           Generated on               Fri Aug 21 14:43:26 2015
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

#ifndef BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_H__
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_H__

/***************************************************************************
 *XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2
 ***************************************************************************/
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_STATUS 0x20a3fc40 /* [RO] CPU interrupt Status Register */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_SET 0x20a3fc44 /* [WO] CPU interrupt Set Register */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_CLEAR 0x20a3fc48 /* [WO] CPU interrupt Clear Register */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_MASK_STATUS 0x20a3fc4c /* [RO] CPU interrupt Mask Status Register */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_MASK_SET 0x20a3fc50 /* [WO] CPU interrupt Mask Set Register */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_MASK_CLEAR 0x20a3fc54 /* [WO] CPU interrupt Mask Clear Register */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_STATUS 0x20a3fc58 /* [RO] PCI interrupt Status Register */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_SET 0x20a3fc5c /* [WO] PCI interrupt Set Register */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_CLEAR 0x20a3fc60 /* [WO] PCI interrupt Clear Register */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_MASK_STATUS 0x20a3fc64 /* [RO] PCI interrupt Mask Status Register */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_MASK_SET 0x20a3fc68 /* [WO] PCI interrupt Mask Set Register */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_MASK_CLEAR 0x20a3fc6c /* [WO] PCI interrupt Mask Clear Register */

/***************************************************************************
 *W1_CPU_STATUS - CPU interrupt Status Register
 ***************************************************************************/
/* XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2 :: W1_CPU_STATUS :: BUF_DAT_RDY_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_STATUS_BUF_DAT_RDY_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_STATUS_BUF_DAT_RDY_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_STATUS_BUF_DAT_RDY_INTR_DEFAULT 0x00000000

/***************************************************************************
 *W1_CPU_SET - CPU interrupt Set Register
 ***************************************************************************/
/* XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2 :: W1_CPU_SET :: BUF_DAT_RDY_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_SET_BUF_DAT_RDY_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_SET_BUF_DAT_RDY_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_SET_BUF_DAT_RDY_INTR_DEFAULT 0x00000000

/***************************************************************************
 *W1_CPU_CLEAR - CPU interrupt Clear Register
 ***************************************************************************/
/* XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2 :: W1_CPU_CLEAR :: BUF_DAT_RDY_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_CLEAR_BUF_DAT_RDY_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_CLEAR_BUF_DAT_RDY_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_CLEAR_BUF_DAT_RDY_INTR_DEFAULT 0x00000000

/***************************************************************************
 *W1_CPU_MASK_STATUS - CPU interrupt Mask Status Register
 ***************************************************************************/
/* XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2 :: W1_CPU_MASK_STATUS :: BUF_DAT_RDY_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_MASK_STATUS_BUF_DAT_RDY_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_MASK_STATUS_BUF_DAT_RDY_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_MASK_STATUS_BUF_DAT_RDY_INTR_DEFAULT 0x00000001

/***************************************************************************
 *W1_CPU_MASK_SET - CPU interrupt Mask Set Register
 ***************************************************************************/
/* XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2 :: W1_CPU_MASK_SET :: BUF_DAT_RDY_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_MASK_SET_BUF_DAT_RDY_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_MASK_SET_BUF_DAT_RDY_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_MASK_SET_BUF_DAT_RDY_INTR_DEFAULT 0x00000001

/***************************************************************************
 *W1_CPU_MASK_CLEAR - CPU interrupt Mask Clear Register
 ***************************************************************************/
/* XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2 :: W1_CPU_MASK_CLEAR :: BUF_DAT_RDY_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_MASK_CLEAR_BUF_DAT_RDY_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_MASK_CLEAR_BUF_DAT_RDY_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_CPU_MASK_CLEAR_BUF_DAT_RDY_INTR_DEFAULT 0x00000001

/***************************************************************************
 *W1_PCI_STATUS - PCI interrupt Status Register
 ***************************************************************************/
/* XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2 :: W1_PCI_STATUS :: BUF_DAT_RDY_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_STATUS_BUF_DAT_RDY_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_STATUS_BUF_DAT_RDY_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_STATUS_BUF_DAT_RDY_INTR_DEFAULT 0x00000000

/***************************************************************************
 *W1_PCI_SET - PCI interrupt Set Register
 ***************************************************************************/
/* XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2 :: W1_PCI_SET :: BUF_DAT_RDY_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_SET_BUF_DAT_RDY_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_SET_BUF_DAT_RDY_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_SET_BUF_DAT_RDY_INTR_DEFAULT 0x00000000

/***************************************************************************
 *W1_PCI_CLEAR - PCI interrupt Clear Register
 ***************************************************************************/
/* XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2 :: W1_PCI_CLEAR :: BUF_DAT_RDY_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_CLEAR_BUF_DAT_RDY_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_CLEAR_BUF_DAT_RDY_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_CLEAR_BUF_DAT_RDY_INTR_DEFAULT 0x00000000

/***************************************************************************
 *W1_PCI_MASK_STATUS - PCI interrupt Mask Status Register
 ***************************************************************************/
/* XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2 :: W1_PCI_MASK_STATUS :: BUF_DAT_RDY_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_MASK_STATUS_BUF_DAT_RDY_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_MASK_STATUS_BUF_DAT_RDY_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_MASK_STATUS_BUF_DAT_RDY_INTR_DEFAULT 0x00000001

/***************************************************************************
 *W1_PCI_MASK_SET - PCI interrupt Mask Set Register
 ***************************************************************************/
/* XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2 :: W1_PCI_MASK_SET :: BUF_DAT_RDY_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_MASK_SET_BUF_DAT_RDY_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_MASK_SET_BUF_DAT_RDY_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_MASK_SET_BUF_DAT_RDY_INTR_DEFAULT 0x00000001

/***************************************************************************
 *W1_PCI_MASK_CLEAR - PCI interrupt Mask Clear Register
 ***************************************************************************/
/* XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2 :: W1_PCI_MASK_CLEAR :: BUF_DAT_RDY_INTR [31:00] */
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_MASK_CLEAR_BUF_DAT_RDY_INTR_MASK 0xffffffff
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_MASK_CLEAR_BUF_DAT_RDY_INTR_SHIFT 0
#define BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_W1_PCI_MASK_CLEAR_BUF_DAT_RDY_INTR_DEFAULT 0x00000001

#endif /* #ifndef BCHP_XPT_MSG_BUF_DAT_RDY_INTR_32_63_L2_H__ */

/* End of File */
