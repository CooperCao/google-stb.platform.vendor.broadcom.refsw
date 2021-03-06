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
 * Date:           Generated on               Fri Feb 20 00:05:25 2015
 *                 Full Compile MD5 Checksum  f4a546a20d0bd1f244e0d6a139e85ce0
 *                     (minus title and desc)
 *                 MD5 Checksum               a9d9eeea3a1c30a122d08de69d07786c
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     15715
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_HEVD_OUTPUT_0_H__
#define BCHP_HEVD_OUTPUT_0_H__

/***************************************************************************
 *HEVD_OUTPUT_0
 ***************************************************************************/
#define BCHP_HEVD_OUTPUT_0_Y_BASE                0x00124500 /* [WO] Luma base address */
#define BCHP_HEVD_OUTPUT_0_C_BASE                0x00124504 /* [WO] Chroma base address */
#define BCHP_HEVD_OUTPUT_0_STRIPE_INFO           0x00124508 /* [WO] Stripe Info */
#define BCHP_HEVD_OUTPUT_0_CHKSUM_Y              0x00124580 /* [RO] Luma Checksum */
#define BCHP_HEVD_OUTPUT_0_CHKSUM_U              0x00124584 /* [RO] U Checksum */
#define BCHP_HEVD_OUTPUT_0_CHKSUM_V              0x00124588 /* [RO] V Checksum */
#define BCHP_HEVD_OUTPUT_0_STATUS                0x001245fc /* [RO] Block Status */

/***************************************************************************
 *Y_BASE - Luma base address
 ***************************************************************************/
/* HEVD_OUTPUT_0 :: Y_BASE :: Addr [31:12] */
#define BCHP_HEVD_OUTPUT_0_Y_BASE_Addr_MASK                        0xfffff000
#define BCHP_HEVD_OUTPUT_0_Y_BASE_Addr_SHIFT                       12
#define BCHP_HEVD_OUTPUT_0_Y_BASE_Addr_DEFAULT                     0x00000000

/* HEVD_OUTPUT_0 :: Y_BASE :: reserved0 [11:00] */
#define BCHP_HEVD_OUTPUT_0_Y_BASE_reserved0_MASK                   0x00000fff
#define BCHP_HEVD_OUTPUT_0_Y_BASE_reserved0_SHIFT                  0

/***************************************************************************
 *C_BASE - Chroma base address
 ***************************************************************************/
/* HEVD_OUTPUT_0 :: C_BASE :: Addr [31:12] */
#define BCHP_HEVD_OUTPUT_0_C_BASE_Addr_MASK                        0xfffff000
#define BCHP_HEVD_OUTPUT_0_C_BASE_Addr_SHIFT                       12
#define BCHP_HEVD_OUTPUT_0_C_BASE_Addr_DEFAULT                     0x00000000

/* HEVD_OUTPUT_0 :: C_BASE :: reserved0 [11:00] */
#define BCHP_HEVD_OUTPUT_0_C_BASE_reserved0_MASK                   0x00000fff
#define BCHP_HEVD_OUTPUT_0_C_BASE_reserved0_SHIFT                  0

/***************************************************************************
 *STRIPE_INFO - Stripe Info
 ***************************************************************************/
/* HEVD_OUTPUT_0 :: STRIPE_INFO :: reserved0 [31:30] */
#define BCHP_HEVD_OUTPUT_0_STRIPE_INFO_reserved0_MASK              0xc0000000
#define BCHP_HEVD_OUTPUT_0_STRIPE_INFO_reserved0_SHIFT             30

/* HEVD_OUTPUT_0 :: STRIPE_INFO :: Stripe_Width [29:28] */
#define BCHP_HEVD_OUTPUT_0_STRIPE_INFO_Stripe_Width_MASK           0x30000000
#define BCHP_HEVD_OUTPUT_0_STRIPE_INFO_Stripe_Width_SHIFT          28
#define BCHP_HEVD_OUTPUT_0_STRIPE_INFO_Stripe_Width_DEFAULT        0x00000000

/* HEVD_OUTPUT_0 :: STRIPE_INFO :: Chroma_Height [27:16] */
#define BCHP_HEVD_OUTPUT_0_STRIPE_INFO_Chroma_Height_MASK          0x0fff0000
#define BCHP_HEVD_OUTPUT_0_STRIPE_INFO_Chroma_Height_SHIFT         16

/* HEVD_OUTPUT_0 :: STRIPE_INFO :: reserved1 [15:13] */
#define BCHP_HEVD_OUTPUT_0_STRIPE_INFO_reserved1_MASK              0x0000e000
#define BCHP_HEVD_OUTPUT_0_STRIPE_INFO_reserved1_SHIFT             13

/* HEVD_OUTPUT_0 :: STRIPE_INFO :: Height [12:00] */
#define BCHP_HEVD_OUTPUT_0_STRIPE_INFO_Height_MASK                 0x00001fff
#define BCHP_HEVD_OUTPUT_0_STRIPE_INFO_Height_SHIFT                0

/***************************************************************************
 *CHKSUM_Y - Luma Checksum
 ***************************************************************************/
/* HEVD_OUTPUT_0 :: CHKSUM_Y :: Checksum [31:00] */
#define BCHP_HEVD_OUTPUT_0_CHKSUM_Y_Checksum_MASK                  0xffffffff
#define BCHP_HEVD_OUTPUT_0_CHKSUM_Y_Checksum_SHIFT                 0
#define BCHP_HEVD_OUTPUT_0_CHKSUM_Y_Checksum_DEFAULT               0x00000000

/***************************************************************************
 *CHKSUM_U - U Checksum
 ***************************************************************************/
/* HEVD_OUTPUT_0 :: CHKSUM_U :: Checksum [31:00] */
#define BCHP_HEVD_OUTPUT_0_CHKSUM_U_Checksum_MASK                  0xffffffff
#define BCHP_HEVD_OUTPUT_0_CHKSUM_U_Checksum_SHIFT                 0
#define BCHP_HEVD_OUTPUT_0_CHKSUM_U_Checksum_DEFAULT               0x00000000

/***************************************************************************
 *CHKSUM_V - V Checksum
 ***************************************************************************/
/* HEVD_OUTPUT_0 :: CHKSUM_V :: Checksum [31:00] */
#define BCHP_HEVD_OUTPUT_0_CHKSUM_V_Checksum_MASK                  0xffffffff
#define BCHP_HEVD_OUTPUT_0_CHKSUM_V_Checksum_SHIFT                 0
#define BCHP_HEVD_OUTPUT_0_CHKSUM_V_Checksum_DEFAULT               0x00000000

/***************************************************************************
 *STATUS - Block Status
 ***************************************************************************/
/* HEVD_OUTPUT_0 :: STATUS :: reserved0 [31:02] */
#define BCHP_HEVD_OUTPUT_0_STATUS_reserved0_MASK                   0xfffffffc
#define BCHP_HEVD_OUTPUT_0_STATUS_reserved0_SHIFT                  2

/* HEVD_OUTPUT_0 :: STATUS :: InputFull [01:01] */
#define BCHP_HEVD_OUTPUT_0_STATUS_InputFull_MASK                   0x00000002
#define BCHP_HEVD_OUTPUT_0_STATUS_InputFull_SHIFT                  1
#define BCHP_HEVD_OUTPUT_0_STATUS_InputFull_DEFAULT                0x00000000

/* HEVD_OUTPUT_0 :: STATUS :: Active [00:00] */
#define BCHP_HEVD_OUTPUT_0_STATUS_Active_MASK                      0x00000001
#define BCHP_HEVD_OUTPUT_0_STATUS_Active_SHIFT                     0
#define BCHP_HEVD_OUTPUT_0_STATUS_Active_DEFAULT                   0x00000000

#endif /* #ifndef BCHP_HEVD_OUTPUT_0_H__ */

/* End of File */
