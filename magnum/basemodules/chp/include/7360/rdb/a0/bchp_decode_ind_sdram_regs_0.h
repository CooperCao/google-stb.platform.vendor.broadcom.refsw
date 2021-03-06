/***************************************************************************
 *     Copyright (c) 1999-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on         Tue Nov 22 11:54:26 2011
 *                 MD5 Checksum         255434854c147509a2505b55dcf1a8f7
 *
 * Compiled with:  RDB Utility          combo_header.pl
 *                 RDB Parser           3.0
 *                 unknown              unknown
 *                 Perl Interpreter     5.008008
 *                 Operating System     linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_DECODE_IND_SDRAM_REGS_0_H__
#define BCHP_DECODE_IND_SDRAM_REGS_0_H__

/***************************************************************************
 *DECODE_IND_SDRAM_REGS_0
 ***************************************************************************/
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_INC 0x00041000 /* REG_SDRAM_INC */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_ADDR 0x00041004 /* REG_SDRAM_ADDR */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_DATA 0x00041008 /* REG_SDRAM_DATA */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG 0x00041010 /* REG_CPU_DBG */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_STAT 0x00041014 /* REG_SDRAM_STAT */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_END 0x0004107c /* REG_SDRAM_END */

/***************************************************************************
 *REG_SDRAM_INC - REG_SDRAM_INC
 ***************************************************************************/
/* DECODE_IND_SDRAM_REGS_0 :: REG_SDRAM_INC :: reserved0 [31:01] */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_INC_reserved0_MASK  0xfffffffe
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_INC_reserved0_SHIFT 1

/* DECODE_IND_SDRAM_REGS_0 :: REG_SDRAM_INC :: Inc [00:00] */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_INC_Inc_MASK        0x00000001
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_INC_Inc_SHIFT       0

/***************************************************************************
 *REG_SDRAM_ADDR - REG_SDRAM_ADDR
 ***************************************************************************/
/* DECODE_IND_SDRAM_REGS_0 :: REG_SDRAM_ADDR :: Addr [31:02] */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_ADDR_Addr_MASK      0xfffffffc
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_ADDR_Addr_SHIFT     2

/* DECODE_IND_SDRAM_REGS_0 :: REG_SDRAM_ADDR :: reserved0 [01:00] */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_ADDR_reserved0_MASK 0x00000003
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_ADDR_reserved0_SHIFT 0

/***************************************************************************
 *REG_SDRAM_DATA - REG_SDRAM_DATA
 ***************************************************************************/
/* DECODE_IND_SDRAM_REGS_0 :: REG_SDRAM_DATA :: Data [31:00] */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_DATA_Data_MASK      0xffffffff
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_DATA_Data_SHIFT     0

/***************************************************************************
 *REG_CPU_DBG - REG_CPU_DBG
 ***************************************************************************/
/* DECODE_IND_SDRAM_REGS_0 :: REG_CPU_DBG :: reserved0 [31:01] */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG_reserved0_MASK    0xfffffffe
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG_reserved0_SHIFT   1

/* DECODE_IND_SDRAM_REGS_0 :: REG_CPU_DBG :: Hst [00:00] */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG_Hst_MASK          0x00000001
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG_Hst_SHIFT         0

/***************************************************************************
 *REG_SDRAM_STAT - REG_SDRAM_STAT
 ***************************************************************************/
/* DECODE_IND_SDRAM_REGS_0 :: REG_SDRAM_STAT :: reserved0 [31:00] */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_STAT_reserved0_MASK 0xffffffff
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_STAT_reserved0_SHIFT 0

/***************************************************************************
 *REG_SDRAM_END - REG_SDRAM_END
 ***************************************************************************/
/* DECODE_IND_SDRAM_REGS_0 :: REG_SDRAM_END :: reserved0 [31:00] */
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_END_reserved0_MASK  0xffffffff
#define BCHP_DECODE_IND_SDRAM_REGS_0_REG_SDRAM_END_reserved0_SHIFT 0

#endif /* #ifndef BCHP_DECODE_IND_SDRAM_REGS_0_H__ */

/* End of File */
