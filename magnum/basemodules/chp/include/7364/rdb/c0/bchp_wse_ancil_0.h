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
 * Date:           Generated on               Mon Feb  8 12:53:14 2016
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

#ifndef BCHP_WSE_ANCIL_0_H__
#define BCHP_WSE_ANCIL_0_H__

/***************************************************************************
 *WSE_ANCIL_0 - WSE_ANCIL_0 registers
 ***************************************************************************/
#define BCHP_WSE_ANCIL_0_control                 0x006a8b04 /* [RW] WSS Control Register */
#define BCHP_WSE_ANCIL_0_wss_data                0x006a8b08 /* [RW] WSS & VPS Data 0 Register */
#define BCHP_WSE_ANCIL_0_wss_revid               0x006a8b0c /* [RO] WSS Module Revision ID register */

/***************************************************************************
 *control - WSS Control Register
 ***************************************************************************/
/* WSE_ANCIL_0 :: control :: reserved0 [31:23] */
#define BCHP_WSE_ANCIL_0_control_reserved0_MASK                    0xff800000
#define BCHP_WSE_ANCIL_0_control_reserved0_SHIFT                   23

/* WSE_ANCIL_0 :: control :: AUTO_PARITY_TYP_656 [22:22] */
#define BCHP_WSE_ANCIL_0_control_AUTO_PARITY_TYP_656_MASK          0x00400000
#define BCHP_WSE_ANCIL_0_control_AUTO_PARITY_TYP_656_SHIFT         22
#define BCHP_WSE_ANCIL_0_control_AUTO_PARITY_TYP_656_DEFAULT       0x00000000
#define BCHP_WSE_ANCIL_0_control_AUTO_PARITY_TYP_656_ODD           0
#define BCHP_WSE_ANCIL_0_control_AUTO_PARITY_TYP_656_EVEN          1

/* WSE_ANCIL_0 :: control :: AUTO_PARITY_EN_656 [21:21] */
#define BCHP_WSE_ANCIL_0_control_AUTO_PARITY_EN_656_MASK           0x00200000
#define BCHP_WSE_ANCIL_0_control_AUTO_PARITY_EN_656_SHIFT          21
#define BCHP_WSE_ANCIL_0_control_AUTO_PARITY_EN_656_DEFAULT        0x00000000

/* WSE_ANCIL_0 :: control :: OUTPUT_ORDER [20:19] */
#define BCHP_WSE_ANCIL_0_control_OUTPUT_ORDER_MASK                 0x00180000
#define BCHP_WSE_ANCIL_0_control_OUTPUT_ORDER_SHIFT                19
#define BCHP_WSE_ANCIL_0_control_OUTPUT_ORDER_DEFAULT              0x00000000
#define BCHP_WSE_ANCIL_0_control_OUTPUT_ORDER_Low_Pad_2nd          0
#define BCHP_WSE_ANCIL_0_control_OUTPUT_ORDER_High_Pad_1st         1
#define BCHP_WSE_ANCIL_0_control_OUTPUT_ORDER_High_Pad_2nd         2
#define BCHP_WSE_ANCIL_0_control_OUTPUT_ORDER_Low_Pad_1st          3

/* WSE_ANCIL_0 :: control :: anci656_enable [18:18] */
#define BCHP_WSE_ANCIL_0_control_anci656_enable_MASK               0x00040000
#define BCHP_WSE_ANCIL_0_control_anci656_enable_SHIFT              18
#define BCHP_WSE_ANCIL_0_control_anci656_enable_DEFAULT            0x00000000

/* WSE_ANCIL_0 :: control :: reserved1 [17:17] */
#define BCHP_WSE_ANCIL_0_control_reserved1_MASK                    0x00020000
#define BCHP_WSE_ANCIL_0_control_reserved1_SHIFT                   17

/* WSE_ANCIL_0 :: control :: active_line [16:08] */
#define BCHP_WSE_ANCIL_0_control_active_line_MASK                  0x0001ff00
#define BCHP_WSE_ANCIL_0_control_active_line_SHIFT                 8
#define BCHP_WSE_ANCIL_0_control_active_line_DEFAULT               0x00000016

/* WSE_ANCIL_0 :: control :: reserved2 [07:00] */
#define BCHP_WSE_ANCIL_0_control_reserved2_MASK                    0x000000ff
#define BCHP_WSE_ANCIL_0_control_reserved2_SHIFT                   0

/***************************************************************************
 *wss_data - WSS & VPS Data 0 Register
 ***************************************************************************/
/* WSE_ANCIL_0 :: wss_data :: reserved0 [31:14] */
#define BCHP_WSE_ANCIL_0_wss_data_reserved0_MASK                   0xffffc000
#define BCHP_WSE_ANCIL_0_wss_data_reserved0_SHIFT                  14

/* WSE_ANCIL_0 :: wss_data :: wss_data [13:00] */
#define BCHP_WSE_ANCIL_0_wss_data_wss_data_MASK                    0x00003fff
#define BCHP_WSE_ANCIL_0_wss_data_wss_data_SHIFT                   0
#define BCHP_WSE_ANCIL_0_wss_data_wss_data_DEFAULT                 0x00000000

/***************************************************************************
 *wss_revid - WSS Module Revision ID register
 ***************************************************************************/
/* WSE_ANCIL_0 :: wss_revid :: reserved0 [31:16] */
#define BCHP_WSE_ANCIL_0_wss_revid_reserved0_MASK                  0xffff0000
#define BCHP_WSE_ANCIL_0_wss_revid_reserved0_SHIFT                 16

/* WSE_ANCIL_0 :: wss_revid :: REVID [15:00] */
#define BCHP_WSE_ANCIL_0_wss_revid_REVID_MASK                      0x0000ffff
#define BCHP_WSE_ANCIL_0_wss_revid_REVID_SHIFT                     0
#define BCHP_WSE_ANCIL_0_wss_revid_REVID_DEFAULT                   0x00004000

#endif /* #ifndef BCHP_WSE_ANCIL_0_H__ */

/* End of File */
