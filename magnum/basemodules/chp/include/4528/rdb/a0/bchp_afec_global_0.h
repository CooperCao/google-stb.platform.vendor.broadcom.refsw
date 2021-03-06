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
 * Date:           Generated on         Wed Jan 12 18:40:50 2011
 *                 MD5 Checksum         6e6727f6c827233acdd395c9a9032c98
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

#ifndef BCHP_AFEC_GLOBAL_0_H__
#define BCHP_AFEC_GLOBAL_0_H__

/***************************************************************************
 *AFEC_GLOBAL_0 - AFEC_GLOBAL Register Set 0
 ***************************************************************************/
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL             0x000b4a00 /* AFEC Clock Control */
#define BCHP_AFEC_GLOBAL_0_PWR_CNTRL             0x000b4a04 /* AFEC Clock Control 1 */
#define BCHP_AFEC_GLOBAL_0_SW_SPARE0             0x000b4a08 /* AFEC SW SPARE0 */
#define BCHP_AFEC_GLOBAL_0_SW_SPARE1             0x000b4a0c /* AFEC SW SPARE1 */

/***************************************************************************
 *CLK_CNTRL - AFEC Clock Control
 ***************************************************************************/
/* AFEC_GLOBAL_0 :: CLK_CNTRL :: reserved0 [31:24] */
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_reserved0_MASK                0xff000000
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_reserved0_SHIFT               24

/* AFEC_GLOBAL_0 :: CLK_CNTRL :: LDPC_CLK_ENABLEB [23:23] */
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_LDPC_CLK_ENABLEB_MASK         0x00800000
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_LDPC_CLK_ENABLEB_SHIFT        23

/* AFEC_GLOBAL_0 :: CLK_CNTRL :: reserved1 [22:20] */
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_reserved1_MASK                0x00700000
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_reserved1_SHIFT               20

/* AFEC_GLOBAL_0 :: CLK_CNTRL :: LDPC_CLK_HOLD [19:19] */
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_LDPC_CLK_HOLD_MASK            0x00080000
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_LDPC_CLK_HOLD_SHIFT           19

/* AFEC_GLOBAL_0 :: CLK_CNTRL :: reserved2 [18:16] */
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_reserved2_MASK                0x00070000
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_reserved2_SHIFT               16

/* AFEC_GLOBAL_0 :: CLK_CNTRL :: LDPC_CLK_LOAD_EN [15:15] */
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_LDPC_CLK_LOAD_EN_MASK         0x00008000
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_LDPC_CLK_LOAD_EN_SHIFT        15

/* AFEC_GLOBAL_0 :: CLK_CNTRL :: reserved3 [14:12] */
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_reserved3_MASK                0x00007000
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_reserved3_SHIFT               12

/* AFEC_GLOBAL_0 :: CLK_CNTRL :: SEL_BYPASS_CLK [11:11] */
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_SEL_BYPASS_CLK_MASK           0x00000800
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_SEL_BYPASS_CLK_SHIFT          11

/* AFEC_GLOBAL_0 :: CLK_CNTRL :: reserved4 [10:08] */
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_reserved4_MASK                0x00000700
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_reserved4_SHIFT               8

/* AFEC_GLOBAL_0 :: CLK_CNTRL :: LDPC_CLK_MDIV [07:00] */
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_LDPC_CLK_MDIV_MASK            0x000000ff
#define BCHP_AFEC_GLOBAL_0_CLK_CNTRL_LDPC_CLK_MDIV_SHIFT           0

/***************************************************************************
 *PWR_CNTRL - AFEC Clock Control 1
 ***************************************************************************/
/* AFEC_GLOBAL_0 :: PWR_CNTRL :: reserved0 [31:01] */
#define BCHP_AFEC_GLOBAL_0_PWR_CNTRL_reserved0_MASK                0xfffffffe
#define BCHP_AFEC_GLOBAL_0_PWR_CNTRL_reserved0_SHIFT               1

/* AFEC_GLOBAL_0 :: PWR_CNTRL :: REG_STBY [00:00] */
#define BCHP_AFEC_GLOBAL_0_PWR_CNTRL_REG_STBY_MASK                 0x00000001
#define BCHP_AFEC_GLOBAL_0_PWR_CNTRL_REG_STBY_SHIFT                0

/***************************************************************************
 *SW_SPARE0 - AFEC SW SPARE0
 ***************************************************************************/
/* AFEC_GLOBAL_0 :: SW_SPARE0 :: SW_SPARE0 [31:00] */
#define BCHP_AFEC_GLOBAL_0_SW_SPARE0_SW_SPARE0_MASK                0xffffffff
#define BCHP_AFEC_GLOBAL_0_SW_SPARE0_SW_SPARE0_SHIFT               0

/***************************************************************************
 *SW_SPARE1 - AFEC SW SPARE1
 ***************************************************************************/
/* AFEC_GLOBAL_0 :: SW_SPARE1 :: SW_SPARE1 [31:00] */
#define BCHP_AFEC_GLOBAL_0_SW_SPARE1_SW_SPARE1_MASK                0xffffffff
#define BCHP_AFEC_GLOBAL_0_SW_SPARE1_SW_SPARE1_SHIFT               0

#endif /* #ifndef BCHP_AFEC_GLOBAL_0_H__ */

/* End of File */
