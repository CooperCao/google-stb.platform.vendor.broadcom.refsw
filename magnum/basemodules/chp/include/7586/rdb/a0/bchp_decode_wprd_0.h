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
 * Date:           Generated on               Wed Feb 11 10:13:57 2015
 *                 Full Compile MD5 Checksum  f7f4bd55341805fcfe958ba5e47e65f4
 *                     (minus title and desc)
 *                 MD5 Checksum               95b679a9655597a92593cae55222c397
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     15653
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_DECODE_WPRD_0_H__
#define BCHP_DECODE_WPRD_0_H__

/***************************************************************************
 *DECODE_WPRD_0
 ***************************************************************************/
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL          0x00120340 /* [WO] Weighted Prediction Control */
#define BCHP_DECODE_WPRD_0_REG_WPRD_END          0x0012035c /* [RW] REG_WPRD_END */

/***************************************************************************
 *REG_WPRD_CTL - Weighted Prediction Control
 ***************************************************************************/
/* DECODE_WPRD_0 :: REG_WPRD_CTL :: reserved0 [31:15] */
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL_reserved0_MASK             0xffff8000
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL_reserved0_SHIFT            15

/* DECODE_WPRD_0 :: REG_WPRD_CTL :: ChromaDenom [14:12] */
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL_ChromaDenom_MASK           0x00007000
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL_ChromaDenom_SHIFT          12

/* DECODE_WPRD_0 :: REG_WPRD_CTL :: reserved1 [11:11] */
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL_reserved1_MASK             0x00000800
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL_reserved1_SHIFT            11

/* DECODE_WPRD_0 :: REG_WPRD_CTL :: LumDenom [10:08] */
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL_LumDenom_MASK              0x00000700
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL_LumDenom_SHIFT             8

/* DECODE_WPRD_0 :: REG_WPRD_CTL :: reserved2 [07:02] */
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL_reserved2_MASK             0x000000fc
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL_reserved2_SHIFT            2

/* DECODE_WPRD_0 :: REG_WPRD_CTL :: PredType [01:00] */
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL_PredType_MASK              0x00000003
#define BCHP_DECODE_WPRD_0_REG_WPRD_CTL_PredType_SHIFT             0

/***************************************************************************
 *REG_WPRD_END - REG_WPRD_END
 ***************************************************************************/
/* DECODE_WPRD_0 :: REG_WPRD_END :: reserved0 [31:00] */
#define BCHP_DECODE_WPRD_0_REG_WPRD_END_reserved0_MASK             0xffffffff
#define BCHP_DECODE_WPRD_0_REG_WPRD_END_reserved0_SHIFT            0

#endif /* #ifndef BCHP_DECODE_WPRD_0_H__ */

/* End of File */
