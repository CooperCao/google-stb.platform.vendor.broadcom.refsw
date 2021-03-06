/****************************************************************************
 *     Copyright (c) 1999-2014, Broadcom Corporation
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
 * Date:           Generated on               Tue Jan 27 12:19:20 2015
 *                 Full Compile MD5 Checksum  3788d4127f6320d7294fc780a1f038a5
 *                     (minus title and desc)
 *                 MD5 Checksum               bc21cc7e43ef60b83e7c02281647c8e6
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     15579
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_FMISC_H__
#define BCHP_FMISC_H__

/***************************************************************************
 *FMISC - BVN Front Control Registers
 ***************************************************************************/
#define BCHP_FMISC_SW_INIT                       0x00604000 /* [RW] BVN Front Soft Init */
#define BCHP_FMISC_TEST_PORT_SEL                 0x00604004 /* [RW] BVN Front Test Port Select */
#define BCHP_FMISC_TEST_PORT_DATA                0x00604008 /* [RO] BVN Front Test Port Status */
#define BCHP_FMISC_BVNF_CLOCK_CTRL               0x00604018 /* [RW] BVN Front clock control register */
#define BCHP_FMISC_SCRATCH_0                     0x00604020 /* [RW] Scratch Register */

/***************************************************************************
 *SW_INIT - BVN Front Soft Init
 ***************************************************************************/
/* FMISC :: SW_INIT :: reserved0 [31:25] */
#define BCHP_FMISC_SW_INIT_reserved0_MASK                          0xfe000000
#define BCHP_FMISC_SW_INIT_reserved0_SHIFT                         25

/* FMISC :: SW_INIT :: RDC [24:24] */
#define BCHP_FMISC_SW_INIT_RDC_MASK                                0x01000000
#define BCHP_FMISC_SW_INIT_RDC_SHIFT                               24
#define BCHP_FMISC_SW_INIT_RDC_DEFAULT                             0x00000000

/* FMISC :: SW_INIT :: reserved1 [23:06] */
#define BCHP_FMISC_SW_INIT_reserved1_MASK                          0x00ffffc0
#define BCHP_FMISC_SW_INIT_reserved1_SHIFT                         6

/* FMISC :: SW_INIT :: VFD_1 [05:05] */
#define BCHP_FMISC_SW_INIT_VFD_1_MASK                              0x00000020
#define BCHP_FMISC_SW_INIT_VFD_1_SHIFT                             5
#define BCHP_FMISC_SW_INIT_VFD_1_DEFAULT                           0x00000000

/* FMISC :: SW_INIT :: VFD_0 [04:04] */
#define BCHP_FMISC_SW_INIT_VFD_0_MASK                              0x00000010
#define BCHP_FMISC_SW_INIT_VFD_0_SHIFT                             4
#define BCHP_FMISC_SW_INIT_VFD_0_DEFAULT                           0x00000000

/* FMISC :: SW_INIT :: reserved2 [03:01] */
#define BCHP_FMISC_SW_INIT_reserved2_MASK                          0x0000000e
#define BCHP_FMISC_SW_INIT_reserved2_SHIFT                         1

/* FMISC :: SW_INIT :: MFD_0 [00:00] */
#define BCHP_FMISC_SW_INIT_MFD_0_MASK                              0x00000001
#define BCHP_FMISC_SW_INIT_MFD_0_SHIFT                             0
#define BCHP_FMISC_SW_INIT_MFD_0_DEFAULT                           0x00000000

/***************************************************************************
 *TEST_PORT_SEL - BVN Front Test Port Select
 ***************************************************************************/
/* FMISC :: TEST_PORT_SEL :: reserved0 [31:01] */
#define BCHP_FMISC_TEST_PORT_SEL_reserved0_MASK                    0xfffffffe
#define BCHP_FMISC_TEST_PORT_SEL_reserved0_SHIFT                   1

/* FMISC :: TEST_PORT_SEL :: TEST_PORT_SEL [00:00] */
#define BCHP_FMISC_TEST_PORT_SEL_TEST_PORT_SEL_MASK                0x00000001
#define BCHP_FMISC_TEST_PORT_SEL_TEST_PORT_SEL_SHIFT               0
#define BCHP_FMISC_TEST_PORT_SEL_TEST_PORT_SEL_TP_OUT_1            1
#define BCHP_FMISC_TEST_PORT_SEL_TEST_PORT_SEL_TP_OUT_0            0

/***************************************************************************
 *TEST_PORT_DATA - BVN Front Test Port Status
 ***************************************************************************/
/* FMISC :: TEST_PORT_DATA :: TEST_PORT_DATA [31:00] */
#define BCHP_FMISC_TEST_PORT_DATA_TEST_PORT_DATA_MASK              0xffffffff
#define BCHP_FMISC_TEST_PORT_DATA_TEST_PORT_DATA_SHIFT             0
#define BCHP_FMISC_TEST_PORT_DATA_TEST_PORT_DATA_DEFAULT           0x00000000

/***************************************************************************
 *BVNF_CLOCK_CTRL - BVN Front clock control register
 ***************************************************************************/
/* FMISC :: BVNF_CLOCK_CTRL :: reserved0 [31:02] */
#define BCHP_FMISC_BVNF_CLOCK_CTRL_reserved0_MASK                  0xfffffffc
#define BCHP_FMISC_BVNF_CLOCK_CTRL_reserved0_SHIFT                 2

/* FMISC :: BVNF_CLOCK_CTRL :: RDC_CLK_FREE_RUN_MODE [01:01] */
#define BCHP_FMISC_BVNF_CLOCK_CTRL_RDC_CLK_FREE_RUN_MODE_MASK      0x00000002
#define BCHP_FMISC_BVNF_CLOCK_CTRL_RDC_CLK_FREE_RUN_MODE_SHIFT     1
#define BCHP_FMISC_BVNF_CLOCK_CTRL_RDC_CLK_FREE_RUN_MODE_DEFAULT   0x00000000

/* FMISC :: BVNF_CLOCK_CTRL :: CLK_FREE_RUN_MODE [00:00] */
#define BCHP_FMISC_BVNF_CLOCK_CTRL_CLK_FREE_RUN_MODE_MASK          0x00000001
#define BCHP_FMISC_BVNF_CLOCK_CTRL_CLK_FREE_RUN_MODE_SHIFT         0
#define BCHP_FMISC_BVNF_CLOCK_CTRL_CLK_FREE_RUN_MODE_DEFAULT       0x00000000

/***************************************************************************
 *SCRATCH_0 - Scratch Register
 ***************************************************************************/
/* FMISC :: SCRATCH_0 :: VALUE [31:00] */
#define BCHP_FMISC_SCRATCH_0_VALUE_MASK                            0xffffffff
#define BCHP_FMISC_SCRATCH_0_VALUE_SHIFT                           0
#define BCHP_FMISC_SCRATCH_0_VALUE_DEFAULT                         0x00000000

#endif /* #ifndef BCHP_FMISC_H__ */

/* End of File */
