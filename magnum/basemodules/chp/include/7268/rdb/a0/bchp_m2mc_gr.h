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

#ifndef BCHP_M2MC_GR_H__
#define BCHP_M2MC_GR_H__

/***************************************************************************
 *M2MC_GR - M2MC GR Bridge Registers
 ***************************************************************************/
#define BCHP_M2MC_GR_REVISION                    0x209b1800 /* [RO] GR Bridge Revision */
#define BCHP_M2MC_GR_CTRL                        0x209b1804 /* [RW] GR Bridge Control Register */
#define BCHP_M2MC_GR_SW_INIT_0                   0x209b1808 /* [RW] GR Bridge Software Init 0 Register */
#define BCHP_M2MC_GR_SW_INIT_1                   0x209b180c /* [RW] GR Bridge Software Init 1 Register */

/***************************************************************************
 *REVISION - GR Bridge Revision
 ***************************************************************************/
/* M2MC_GR :: REVISION :: reserved0 [31:16] */
#define BCHP_M2MC_GR_REVISION_reserved0_MASK                       0xffff0000
#define BCHP_M2MC_GR_REVISION_reserved0_SHIFT                      16

/* M2MC_GR :: REVISION :: MAJOR [15:08] */
#define BCHP_M2MC_GR_REVISION_MAJOR_MASK                           0x0000ff00
#define BCHP_M2MC_GR_REVISION_MAJOR_SHIFT                          8
#define BCHP_M2MC_GR_REVISION_MAJOR_DEFAULT                        0x00000002

/* M2MC_GR :: REVISION :: MINOR [07:00] */
#define BCHP_M2MC_GR_REVISION_MINOR_MASK                           0x000000ff
#define BCHP_M2MC_GR_REVISION_MINOR_SHIFT                          0
#define BCHP_M2MC_GR_REVISION_MINOR_DEFAULT                        0x00000000

/***************************************************************************
 *CTRL - GR Bridge Control Register
 ***************************************************************************/
/* M2MC_GR :: CTRL :: reserved0 [31:01] */
#define BCHP_M2MC_GR_CTRL_reserved0_MASK                           0xfffffffe
#define BCHP_M2MC_GR_CTRL_reserved0_SHIFT                          1

/* M2MC_GR :: CTRL :: gisb_error_intr [00:00] */
#define BCHP_M2MC_GR_CTRL_gisb_error_intr_MASK                     0x00000001
#define BCHP_M2MC_GR_CTRL_gisb_error_intr_SHIFT                    0
#define BCHP_M2MC_GR_CTRL_gisb_error_intr_DEFAULT                  0x00000000
#define BCHP_M2MC_GR_CTRL_gisb_error_intr_INTR_DISABLE             0
#define BCHP_M2MC_GR_CTRL_gisb_error_intr_INTR_ENABLE              1

/***************************************************************************
 *SW_INIT_0 - GR Bridge Software Init 0 Register
 ***************************************************************************/
/* M2MC_GR :: SW_INIT_0 :: reserved0 [31:01] */
#define BCHP_M2MC_GR_SW_INIT_0_reserved0_MASK                      0xfffffffe
#define BCHP_M2MC_GR_SW_INIT_0_reserved0_SHIFT                     1

/* M2MC_GR :: SW_INIT_0 :: M2MC_CLK_108_SW_INIT [00:00] */
#define BCHP_M2MC_GR_SW_INIT_0_M2MC_CLK_108_SW_INIT_MASK           0x00000001
#define BCHP_M2MC_GR_SW_INIT_0_M2MC_CLK_108_SW_INIT_SHIFT          0
#define BCHP_M2MC_GR_SW_INIT_0_M2MC_CLK_108_SW_INIT_DEFAULT        0x00000000
#define BCHP_M2MC_GR_SW_INIT_0_M2MC_CLK_108_SW_INIT_DEASSERT       0
#define BCHP_M2MC_GR_SW_INIT_0_M2MC_CLK_108_SW_INIT_ASSERT         1

/***************************************************************************
 *SW_INIT_1 - GR Bridge Software Init 1 Register
 ***************************************************************************/
/* M2MC_GR :: SW_INIT_1 :: reserved0 [31:01] */
#define BCHP_M2MC_GR_SW_INIT_1_reserved0_MASK                      0xfffffffe
#define BCHP_M2MC_GR_SW_INIT_1_reserved0_SHIFT                     1

/* M2MC_GR :: SW_INIT_1 :: YOUR_NAME00_SW_INIT [00:00] */
#define BCHP_M2MC_GR_SW_INIT_1_YOUR_NAME00_SW_INIT_MASK            0x00000001
#define BCHP_M2MC_GR_SW_INIT_1_YOUR_NAME00_SW_INIT_SHIFT           0
#define BCHP_M2MC_GR_SW_INIT_1_YOUR_NAME00_SW_INIT_DEFAULT         0x00000001
#define BCHP_M2MC_GR_SW_INIT_1_YOUR_NAME00_SW_INIT_DEASSERT        0
#define BCHP_M2MC_GR_SW_INIT_1_YOUR_NAME00_SW_INIT_ASSERT          1

#endif /* #ifndef BCHP_M2MC_GR_H__ */

/* End of File */
