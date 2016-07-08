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
 * Date:           Generated on               Mon Feb  8 12:53:15 2016
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

#ifndef BCHP_DVI_FC_0_H__
#define BCHP_DVI_FC_0_H__

/***************************************************************************
 *DVI_FC_0 - DVI Frontend Format Conversion 0
 ***************************************************************************/
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL        0x006a4a00 /* [RW] FORMAT CONVERSION CONTROL REGISTER */
#define BCHP_DVI_FC_0_FC_REV_ID                  0x006a4a04 /* [RO] Revision ID register */

/***************************************************************************
 *FORMAT_CONV_CONTROL - FORMAT CONVERSION CONTROL REGISTER
 ***************************************************************************/
/* DVI_FC_0 :: FORMAT_CONV_CONTROL :: reserved0 [31:25] */
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_reserved0_MASK           0xfe000000
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_reserved0_SHIFT          25

/* DVI_FC_0 :: FORMAT_CONV_CONTROL :: HTOTAL_SIZE [24:13] */
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_HTOTAL_SIZE_MASK         0x01ffe000
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_HTOTAL_SIZE_SHIFT        13
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_HTOTAL_SIZE_DEFAULT      0x00000898

/* DVI_FC_0 :: FORMAT_CONV_CONTROL :: COEFF_1 [12:09] */
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_COEFF_1_MASK             0x00001e00
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_COEFF_1_SHIFT            9
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_COEFF_1_DEFAULT          0x00000004

/* DVI_FC_0 :: FORMAT_CONV_CONTROL :: COEFF_0 [08:05] */
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_COEFF_0_MASK             0x000001e0
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_COEFF_0_SHIFT            5
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_COEFF_0_DEFAULT          0x00000004

/* DVI_FC_0 :: FORMAT_CONV_CONTROL :: FORMAT [04:03] */
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FORMAT_MASK              0x00000018
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FORMAT_SHIFT             3
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FORMAT_DEFAULT           0x00000000
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FORMAT_FORMAT_444        0
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FORMAT_FORMAT_422        1
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FORMAT_FORMAT_420        2

/* DVI_FC_0 :: FORMAT_CONV_CONTROL :: FILTER_MODE [02:01] */
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FILTER_MODE_MASK         0x00000006
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FILTER_MODE_SHIFT        1
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FILTER_MODE_DEFAULT      0x00000003
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FILTER_MODE_FILTER1      0
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FILTER_MODE_FILTER2      1
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FILTER_MODE_FILTER3      2
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FILTER_MODE_BYPASS       3

/* DVI_FC_0 :: FORMAT_CONV_CONTROL :: DERING_EN [00:00] */
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_DERING_EN_MASK           0x00000001
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_DERING_EN_SHIFT          0
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_DERING_EN_DEFAULT        0x00000000
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_DERING_EN_OFF            0
#define BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_DERING_EN_ON             1

/***************************************************************************
 *FC_REV_ID - Revision ID register
 ***************************************************************************/
/* DVI_FC_0 :: FC_REV_ID :: reserved0 [31:16] */
#define BCHP_DVI_FC_0_FC_REV_ID_reserved0_MASK                     0xffff0000
#define BCHP_DVI_FC_0_FC_REV_ID_reserved0_SHIFT                    16

/* DVI_FC_0 :: FC_REV_ID :: REVISION_ID [15:00] */
#define BCHP_DVI_FC_0_FC_REV_ID_REVISION_ID_MASK                   0x0000ffff
#define BCHP_DVI_FC_0_FC_REV_ID_REVISION_ID_SHIFT                  0
#define BCHP_DVI_FC_0_FC_REV_ID_REVISION_ID_DEFAULT                0x00001000

#endif /* #ifndef BCHP_DVI_FC_0_H__ */

/* End of File */
