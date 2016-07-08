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
 * Date:           Generated on               Mon Feb  8 12:53:12 2016
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

#ifndef BCHP_AFEC_GR_BRIDGE_0_H__
#define BCHP_AFEC_GR_BRIDGE_0_H__

/***************************************************************************
 *AFEC_GR_BRIDGE_0 - AFEC_GR_BRIDGE Register Set
 ***************************************************************************/
#define BCHP_AFEC_GR_BRIDGE_0_REVISION           0x01254000 /* [RO] GR Bridge Revision */
#define BCHP_AFEC_GR_BRIDGE_0_CTRL               0x01254004 /* [RW] GR Bridge Control Register */
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0         0x01254008 /* [RW] GR Bridge Software Reset 0 Register */
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1         0x0125400c /* [RW] GR Bridge Software Reset 1 Register */

/***************************************************************************
 *REVISION - GR Bridge Revision
 ***************************************************************************/
/* AFEC_GR_BRIDGE_0 :: REVISION :: reserved0 [31:16] */
#define BCHP_AFEC_GR_BRIDGE_0_REVISION_reserved0_MASK              0xffff0000
#define BCHP_AFEC_GR_BRIDGE_0_REVISION_reserved0_SHIFT             16

/* AFEC_GR_BRIDGE_0 :: REVISION :: MAJOR [15:08] */
#define BCHP_AFEC_GR_BRIDGE_0_REVISION_MAJOR_MASK                  0x0000ff00
#define BCHP_AFEC_GR_BRIDGE_0_REVISION_MAJOR_SHIFT                 8
#define BCHP_AFEC_GR_BRIDGE_0_REVISION_MAJOR_DEFAULT               0x00000001

/* AFEC_GR_BRIDGE_0 :: REVISION :: MINOR [07:00] */
#define BCHP_AFEC_GR_BRIDGE_0_REVISION_MINOR_MASK                  0x000000ff
#define BCHP_AFEC_GR_BRIDGE_0_REVISION_MINOR_SHIFT                 0
#define BCHP_AFEC_GR_BRIDGE_0_REVISION_MINOR_DEFAULT               0x00000000

/***************************************************************************
 *CTRL - GR Bridge Control Register
 ***************************************************************************/
/* AFEC_GR_BRIDGE_0 :: CTRL :: reserved0 [31:01] */
#define BCHP_AFEC_GR_BRIDGE_0_CTRL_reserved0_MASK                  0xfffffffe
#define BCHP_AFEC_GR_BRIDGE_0_CTRL_reserved0_SHIFT                 1

/* AFEC_GR_BRIDGE_0 :: CTRL :: gisb_error_intr [00:00] */
#define BCHP_AFEC_GR_BRIDGE_0_CTRL_gisb_error_intr_MASK            0x00000001
#define BCHP_AFEC_GR_BRIDGE_0_CTRL_gisb_error_intr_SHIFT           0
#define BCHP_AFEC_GR_BRIDGE_0_CTRL_gisb_error_intr_DEFAULT         0x00000000
#define BCHP_AFEC_GR_BRIDGE_0_CTRL_gisb_error_intr_INTR_DISABLE    0
#define BCHP_AFEC_GR_BRIDGE_0_CTRL_gisb_error_intr_INTR_ENABLE     1

/***************************************************************************
 *SW_RESET_0 - GR Bridge Software Reset 0 Register
 ***************************************************************************/
/* AFEC_GR_BRIDGE_0 :: SW_RESET_0 :: reserved0 [31:03] */
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_reserved0_MASK            0xfffffff8
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_reserved0_SHIFT           3

/* AFEC_GR_BRIDGE_0 :: SW_RESET_0 :: YOUR_NAME02_SW_RESET [02:02] */
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME02_SW_RESET_MASK 0x00000004
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME02_SW_RESET_SHIFT 2
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME02_SW_RESET_DEFAULT 0x00000000
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME02_SW_RESET_DEASSERT 0
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME02_SW_RESET_ASSERT 1

/* AFEC_GR_BRIDGE_0 :: SW_RESET_0 :: YOUR_NAME01_SW_RESET [01:01] */
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME01_SW_RESET_MASK 0x00000002
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME01_SW_RESET_SHIFT 1
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME01_SW_RESET_DEFAULT 0x00000000
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME01_SW_RESET_DEASSERT 0
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME01_SW_RESET_ASSERT 1

/* AFEC_GR_BRIDGE_0 :: SW_RESET_0 :: YOUR_NAME00_SW_RESET [00:00] */
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME00_SW_RESET_MASK 0x00000001
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME00_SW_RESET_SHIFT 0
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME00_SW_RESET_DEFAULT 0x00000000
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME00_SW_RESET_DEASSERT 0
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_0_YOUR_NAME00_SW_RESET_ASSERT 1

/***************************************************************************
 *SW_RESET_1 - GR Bridge Software Reset 1 Register
 ***************************************************************************/
/* AFEC_GR_BRIDGE_0 :: SW_RESET_1 :: reserved0 [31:03] */
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_reserved0_MASK            0xfffffff8
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_reserved0_SHIFT           3

/* AFEC_GR_BRIDGE_0 :: SW_RESET_1 :: YOUR_NAME02_SW_RESET [02:02] */
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME02_SW_RESET_MASK 0x00000004
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME02_SW_RESET_SHIFT 2
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME02_SW_RESET_DEFAULT 0x00000001
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME02_SW_RESET_DEASSERT 0
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME02_SW_RESET_ASSERT 1

/* AFEC_GR_BRIDGE_0 :: SW_RESET_1 :: YOUR_NAME01_SW_RESET [01:01] */
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME01_SW_RESET_MASK 0x00000002
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME01_SW_RESET_SHIFT 1
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME01_SW_RESET_DEFAULT 0x00000001
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME01_SW_RESET_DEASSERT 0
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME01_SW_RESET_ASSERT 1

/* AFEC_GR_BRIDGE_0 :: SW_RESET_1 :: YOUR_NAME00_SW_RESET [00:00] */
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME00_SW_RESET_MASK 0x00000001
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME00_SW_RESET_SHIFT 0
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME00_SW_RESET_DEFAULT 0x00000001
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME00_SW_RESET_DEASSERT 0
#define BCHP_AFEC_GR_BRIDGE_0_SW_RESET_1_YOUR_NAME00_SW_RESET_ASSERT 1

#endif /* #ifndef BCHP_AFEC_GR_BRIDGE_0_H__ */

/* End of File */
