/***************************************************************************
 *     Copyright (c) 1999-2012, Broadcom Corporation
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
 * Date:           Generated on         Tue Feb 28 11:03:20 2012
 *                 MD5 Checksum         d41d8cd98f00b204e9800998ecf8427e
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

#ifndef BCHP_SSP_RG_H__
#define BCHP_SSP_RG_H__

/***************************************************************************
 *SSP_RG - Registers for the SSP RG bridge
 ***************************************************************************/
#define BCHP_SSP_RG_REVISION                     0x00400600 /* RG Bridge Revision */
#define BCHP_SSP_RG_CTRL                         0x00400604 /* RG Bridge Control Register */
#define BCHP_SSP_RG_SW_INIT_0                    0x00400608 /* RG Bridge Software Init 0 Register */
#define BCHP_SSP_RG_SW_INIT_1                    0x0040060c /* RG Bridge Software Init 1 Register */

/***************************************************************************
 *REVISION - RG Bridge Revision
 ***************************************************************************/
/* SSP_RG :: REVISION :: reserved0 [31:16] */
#define BCHP_SSP_RG_REVISION_reserved0_MASK                        0xffff0000
#define BCHP_SSP_RG_REVISION_reserved0_SHIFT                       16

/* SSP_RG :: REVISION :: MAJOR [15:08] */
#define BCHP_SSP_RG_REVISION_MAJOR_MASK                            0x0000ff00
#define BCHP_SSP_RG_REVISION_MAJOR_SHIFT                           8
#define BCHP_SSP_RG_REVISION_MAJOR_DEFAULT                         0x00000002

/* SSP_RG :: REVISION :: MINOR [07:00] */
#define BCHP_SSP_RG_REVISION_MINOR_MASK                            0x000000ff
#define BCHP_SSP_RG_REVISION_MINOR_SHIFT                           0
#define BCHP_SSP_RG_REVISION_MINOR_DEFAULT                         0x00000000

/***************************************************************************
 *CTRL - RG Bridge Control Register
 ***************************************************************************/
/* SSP_RG :: CTRL :: reserved0 [31:02] */
#define BCHP_SSP_RG_CTRL_reserved0_MASK                            0xfffffffc
#define BCHP_SSP_RG_CTRL_reserved0_SHIFT                           2

/* SSP_RG :: CTRL :: rbus_error_intr [01:01] */
#define BCHP_SSP_RG_CTRL_rbus_error_intr_MASK                      0x00000002
#define BCHP_SSP_RG_CTRL_rbus_error_intr_SHIFT                     1
#define BCHP_SSP_RG_CTRL_rbus_error_intr_DEFAULT                   0x00000000
#define BCHP_SSP_RG_CTRL_rbus_error_intr_INTR_DISABLE              0
#define BCHP_SSP_RG_CTRL_rbus_error_intr_INTR_ENABLE               1

/* SSP_RG :: CTRL :: reserved1 [00:00] */
#define BCHP_SSP_RG_CTRL_reserved1_MASK                            0x00000001
#define BCHP_SSP_RG_CTRL_reserved1_SHIFT                           0

/***************************************************************************
 *SW_INIT_0 - RG Bridge Software Init 0 Register
 ***************************************************************************/
/* SSP_RG :: SW_INIT_0 :: reserved0 [31:01] */
#define BCHP_SSP_RG_SW_INIT_0_reserved0_MASK                       0xfffffffe
#define BCHP_SSP_RG_SW_INIT_0_reserved0_SHIFT                      1

/* SSP_RG :: SW_INIT_0 :: SPARE_SW_INIT [00:00] */
#define BCHP_SSP_RG_SW_INIT_0_SPARE_SW_INIT_MASK                   0x00000001
#define BCHP_SSP_RG_SW_INIT_0_SPARE_SW_INIT_SHIFT                  0
#define BCHP_SSP_RG_SW_INIT_0_SPARE_SW_INIT_DEFAULT                0x00000000
#define BCHP_SSP_RG_SW_INIT_0_SPARE_SW_INIT_DEASSERT               0
#define BCHP_SSP_RG_SW_INIT_0_SPARE_SW_INIT_ASSERT                 1

/***************************************************************************
 *SW_INIT_1 - RG Bridge Software Init 1 Register
 ***************************************************************************/
/* SSP_RG :: SW_INIT_1 :: reserved0 [31:01] */
#define BCHP_SSP_RG_SW_INIT_1_reserved0_MASK                       0xfffffffe
#define BCHP_SSP_RG_SW_INIT_1_reserved0_SHIFT                      1

/* SSP_RG :: SW_INIT_1 :: SPARE_SW_INIT [00:00] */
#define BCHP_SSP_RG_SW_INIT_1_SPARE_SW_INIT_MASK                   0x00000001
#define BCHP_SSP_RG_SW_INIT_1_SPARE_SW_INIT_SHIFT                  0
#define BCHP_SSP_RG_SW_INIT_1_SPARE_SW_INIT_DEFAULT                0x00000001
#define BCHP_SSP_RG_SW_INIT_1_SPARE_SW_INIT_DEASSERT               0
#define BCHP_SSP_RG_SW_INIT_1_SPARE_SW_INIT_ASSERT                 1

#endif /* #ifndef BCHP_SSP_RG_H__ */

/* End of File */
