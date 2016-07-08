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
 * Date:           Generated on               Fri Aug 21 14:43:24 2015
 *                 Full Compile MD5 Checksum  6f40c93fa7adf1b7b596c84d59590a10
 *                     (minus title and desc)
 *                 MD5 Checksum               b1b8c76af39c441b8e9ab1ae2930543d
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     88
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /tools/dvtsw/current/Linux/combo_header.pl
 *                 DVTSWVER                   current
 *
 *
 ***************************************************************************/

#ifndef BCHP_AUD_FMM_IOP_LOOPBACK_0_H__
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_H__

/***************************************************************************
 *AUD_FMM_IOP_LOOPBACK_0
 ***************************************************************************/
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE 0x20cb1500 /* [RO] Loopback Capture FCI_ID table */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS   0x20cb1510 /* [RO] Error Status Register */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_SET 0x20cb1514 /* [WO] Error Set Register */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_CLEAR 0x20cb1518 /* [WO] Error Clear Register */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK     0x20cb151c /* [RO] Mask Status Register */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_SET 0x20cb1520 /* [WO] Mask Set Register */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_CLEAR 0x20cb1524 /* [WO] Mask Clear Register */

/***************************************************************************
 *STREAM_CFG_%i - Stream configuration
 ***************************************************************************/
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_ARRAY_BASE        0x20cb1400
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_ARRAY_START       0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_ARRAY_END         3
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *STREAM_CFG_%i - Stream configuration
 ***************************************************************************/
/* AUD_FMM_IOP_LOOPBACK_0 :: STREAM_CFG_i :: ENA [31:31] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_ENA_MASK          0x80000000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_ENA_SHIFT         31
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_ENA_DEFAULT       0x00000000

/* AUD_FMM_IOP_LOOPBACK_0 :: STREAM_CFG_i :: reserved0 [30:28] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_reserved0_MASK    0x70000000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_reserved0_SHIFT   28

/* AUD_FMM_IOP_LOOPBACK_0 :: STREAM_CFG_i :: CHANNEL_GROUPING [27:24] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_CHANNEL_GROUPING_MASK 0x0f000000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_CHANNEL_GROUPING_SHIFT 24
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_CHANNEL_GROUPING_DEFAULT 0x00000001

/* AUD_FMM_IOP_LOOPBACK_0 :: STREAM_CFG_i :: GROUP_ID [23:20] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_GROUP_ID_MASK     0x00f00000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_GROUP_ID_SHIFT    20
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_GROUP_ID_DEFAULT  0x00000000

/* AUD_FMM_IOP_LOOPBACK_0 :: STREAM_CFG_i :: STREAM_BIT_RESOLUTION [19:16] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_STREAM_BIT_RESOLUTION_MASK 0x000f0000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_STREAM_BIT_RESOLUTION_SHIFT 16
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_STREAM_BIT_RESOLUTION_DEFAULT 0x00000008
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_STREAM_BIT_RESOLUTION_Res_16_Bit 0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_STREAM_BIT_RESOLUTION_Res_17_Bit 1
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_STREAM_BIT_RESOLUTION_Res_18_Bit 2
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_STREAM_BIT_RESOLUTION_Res_19_Bit 3
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_STREAM_BIT_RESOLUTION_Res_20_Bit 4
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_STREAM_BIT_RESOLUTION_Res_21_Bit 5
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_STREAM_BIT_RESOLUTION_Res_22_Bit 6
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_STREAM_BIT_RESOLUTION_Res_23_Bit 7
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_STREAM_BIT_RESOLUTION_Res_24_Bit 8

/* AUD_FMM_IOP_LOOPBACK_0 :: STREAM_CFG_i :: WAIT_FOR_VALID [15:15] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_WAIT_FOR_VALID_MASK 0x00008000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_WAIT_FOR_VALID_SHIFT 15
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_WAIT_FOR_VALID_DEFAULT 0x00000000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_WAIT_FOR_VALID_Holdoff_request 1
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_WAIT_FOR_VALID_Keep_requesting 0

/* AUD_FMM_IOP_LOOPBACK_0 :: STREAM_CFG_i :: IGNORE_FIRST_UNDERFLOW [14:14] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_IGNORE_FIRST_UNDERFLOW_MASK 0x00004000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_IGNORE_FIRST_UNDERFLOW_SHIFT 14
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_IGNORE_FIRST_UNDERFLOW_DEFAULT 0x00000001
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_IGNORE_FIRST_UNDERFLOW_Ignore 1
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_IGNORE_FIRST_UNDERFLOW_Dont_ignore 0

/* AUD_FMM_IOP_LOOPBACK_0 :: STREAM_CFG_i :: INIT_SM [13:13] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_INIT_SM_MASK      0x00002000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_INIT_SM_SHIFT     13
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_INIT_SM_DEFAULT   0x00000000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_INIT_SM_Init      1
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_INIT_SM_Normal    0

/* AUD_FMM_IOP_LOOPBACK_0 :: STREAM_CFG_i :: INS_INVAL [12:12] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_INS_INVAL_MASK    0x00001000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_INS_INVAL_SHIFT   12
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_INS_INVAL_DEFAULT 0x00000000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_INS_INVAL_Invalid 1
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_INS_INVAL_Valid   0

/* AUD_FMM_IOP_LOOPBACK_0 :: STREAM_CFG_i :: reserved1 [11:10] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_reserved1_MASK    0x00000c00
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_reserved1_SHIFT   10

/* AUD_FMM_IOP_LOOPBACK_0 :: STREAM_CFG_i :: FCI_ID [09:00] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_FCI_ID_MASK       0x000003ff
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_FCI_ID_SHIFT      0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_STREAM_CFG_i_FCI_ID_DEFAULT    0x000003ff


/***************************************************************************
 *CAP_STREAM_CFG_%i - Capture Stream configuration
 ***************************************************************************/
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_ARRAY_BASE    0x20cb1440
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_ARRAY_START   0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_ARRAY_END     3
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *CAP_STREAM_CFG_%i - Capture Stream configuration
 ***************************************************************************/
/* AUD_FMM_IOP_LOOPBACK_0 :: CAP_STREAM_CFG_i :: CAP_ENA [31:31] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_CAP_ENA_MASK  0x80000000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_CAP_ENA_SHIFT 31
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_CAP_ENA_DEFAULT 0x00000000

/* AUD_FMM_IOP_LOOPBACK_0 :: CAP_STREAM_CFG_i :: reserved0 [30:08] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_reserved0_MASK 0x7fffff00
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_reserved0_SHIFT 8

/* AUD_FMM_IOP_LOOPBACK_0 :: CAP_STREAM_CFG_i :: CAP_GROUP_ID [07:04] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_CAP_GROUP_ID_MASK 0x000000f0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_CAP_GROUP_ID_SHIFT 4
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_CAP_GROUP_ID_DEFAULT 0x00000000

/* AUD_FMM_IOP_LOOPBACK_0 :: CAP_STREAM_CFG_i :: reserved1 [03:01] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_reserved1_MASK 0x0000000e
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_reserved1_SHIFT 1

/* AUD_FMM_IOP_LOOPBACK_0 :: CAP_STREAM_CFG_i :: IGNORE_FIRST_OVERFLOW [00:00] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_IGNORE_FIRST_OVERFLOW_MASK 0x00000001
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_IGNORE_FIRST_OVERFLOW_SHIFT 0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_IGNORE_FIRST_OVERFLOW_DEFAULT 0x00000001
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_IGNORE_FIRST_OVERFLOW_Ignore 1
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAP_STREAM_CFG_i_IGNORE_FIRST_OVERFLOW_Dont_ignore 0


/***************************************************************************
 *LOOPBACK_CFG_%i - Loopback configuration
 ***************************************************************************/
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_ARRAY_BASE      0x20cb1480
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_ARRAY_START     0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_ARRAY_END       3
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *LOOPBACK_CFG_%i - Loopback configuration
 ***************************************************************************/
/* AUD_FMM_IOP_LOOPBACK_0 :: LOOPBACK_CFG_i :: reserved0 [31:06] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_reserved0_MASK  0xffffffc0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_reserved0_SHIFT 6

/* AUD_FMM_IOP_LOOPBACK_0 :: LOOPBACK_CFG_i :: INS_NOACK [05:05] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_INS_NOACK_MASK  0x00000020
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_INS_NOACK_SHIFT 5
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_INS_NOACK_DEFAULT 0x00000000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_INS_NOACK_Enable 1
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_INS_NOACK_Disable 0

/* AUD_FMM_IOP_LOOPBACK_0 :: LOOPBACK_CFG_i :: reserved1 [04:04] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_reserved1_MASK  0x00000010
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_reserved1_SHIFT 4

/* AUD_FMM_IOP_LOOPBACK_0 :: LOOPBACK_CFG_i :: INSERT_CTL [03:02] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_INSERT_CTL_MASK 0x0000000c
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_INSERT_CTL_SHIFT 2
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_INSERT_CTL_DEFAULT 0x00000000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_INSERT_CTL_Repeat 3
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_INSERT_CTL_Insert_zero 2
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_INSERT_CTL_No_insert 0

/* AUD_FMM_IOP_LOOPBACK_0 :: LOOPBACK_CFG_i :: reserved2 [01:00] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_reserved2_MASK  0x00000003
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_LOOPBACK_CFG_i_reserved2_SHIFT 0


/***************************************************************************
 *MCLK_CFG_%i - Loopback MCLK configuration
 ***************************************************************************/
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_ARRAY_BASE          0x20cb14c0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_ARRAY_START         0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_ARRAY_END           3
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_ARRAY_ELEMENT_SIZE  32

/***************************************************************************
 *MCLK_CFG_%i - Loopback MCLK configuration
 ***************************************************************************/
/* AUD_FMM_IOP_LOOPBACK_0 :: MCLK_CFG_i :: reserved0 [31:20] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_reserved0_MASK      0xfff00000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_reserved0_SHIFT     20

/* AUD_FMM_IOP_LOOPBACK_0 :: MCLK_CFG_i :: MCLK_RATE [19:16] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_MCLK_RATE_MASK      0x000f0000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_MCLK_RATE_SHIFT     16
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_MCLK_RATE_DEFAULT   0x00000002
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_MCLK_RATE_MCLK_512fs_SCLK_64fs 4
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_MCLK_RATE_MCLK_384fs_SCLK_64fs 3
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_MCLK_RATE_MCLK_256fs_SCLK_64fs 2
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_MCLK_RATE_MCLK_128fs_SCLK_64fs 1

/* AUD_FMM_IOP_LOOPBACK_0 :: MCLK_CFG_i :: reserved1 [15:04] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_reserved1_MASK      0x0000fff0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_reserved1_SHIFT     4

/* AUD_FMM_IOP_LOOPBACK_0 :: MCLK_CFG_i :: PLLCLKSEL [03:00] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_MASK      0x0000000f
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_SHIFT     0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_DEFAULT   0x00000001
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_PLL0_ch1  0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_PLL0_ch2  1
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_PLL0_ch3  2
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_PLL1_ch1  3
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_PLL1_ch2  4
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_PLL1_ch3  5
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen0 6
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen1 7
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen2 8
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen3 9
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen4 10


/***************************************************************************
 *CAPTURE_FCI_ID_TABLE - Loopback Capture FCI_ID table
 ***************************************************************************/
/* AUD_FMM_IOP_LOOPBACK_0 :: CAPTURE_FCI_ID_TABLE :: reserved0 [31:20] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE_reserved0_MASK 0xfff00000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE_reserved0_SHIFT 20

/* AUD_FMM_IOP_LOOPBACK_0 :: CAPTURE_FCI_ID_TABLE :: NUM_CAP_CHANNELS [19:16] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE_NUM_CAP_CHANNELS_MASK 0x000f0000
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE_NUM_CAP_CHANNELS_SHIFT 16
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE_NUM_CAP_CHANNELS_DEFAULT 0x00000004

/* AUD_FMM_IOP_LOOPBACK_0 :: CAPTURE_FCI_ID_TABLE :: reserved1 [15:10] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE_reserved1_MASK 0x0000fc00
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE_reserved1_SHIFT 10

/* AUD_FMM_IOP_LOOPBACK_0 :: CAPTURE_FCI_ID_TABLE :: START_FCI_ID [09:00] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE_START_FCI_ID_MASK 0x000003ff
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE_START_FCI_ID_SHIFT 0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_CAPTURE_FCI_ID_TABLE_START_FCI_ID_DEFAULT 0x00000181

/***************************************************************************
 *ESR_STATUS - Error Status Register
 ***************************************************************************/
/* AUD_FMM_IOP_LOOPBACK_0 :: ESR_STATUS :: reserved0 [31:04] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_reserved0_MASK      0xfffffff0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_reserved0_SHIFT     4

/* AUD_FMM_IOP_LOOPBACK_0 :: ESR_STATUS :: STREAM_UNDERFLOW [03:00] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_STREAM_UNDERFLOW_MASK 0x0000000f
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_STREAM_UNDERFLOW_SHIFT 0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_STREAM_UNDERFLOW_DEFAULT 0x00000000

/***************************************************************************
 *ESR_STATUS_SET - Error Set Register
 ***************************************************************************/
/* AUD_FMM_IOP_LOOPBACK_0 :: ESR_STATUS_SET :: reserved0 [31:04] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_SET_reserved0_MASK  0xfffffff0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_SET_reserved0_SHIFT 4

/* AUD_FMM_IOP_LOOPBACK_0 :: ESR_STATUS_SET :: STREAM_UNDERFLOW [03:00] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_SET_STREAM_UNDERFLOW_MASK 0x0000000f
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_SET_STREAM_UNDERFLOW_SHIFT 0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_SET_STREAM_UNDERFLOW_DEFAULT 0x00000000

/***************************************************************************
 *ESR_STATUS_CLEAR - Error Clear Register
 ***************************************************************************/
/* AUD_FMM_IOP_LOOPBACK_0 :: ESR_STATUS_CLEAR :: reserved0 [31:04] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_CLEAR_reserved0_MASK 0xfffffff0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_CLEAR_reserved0_SHIFT 4

/* AUD_FMM_IOP_LOOPBACK_0 :: ESR_STATUS_CLEAR :: STREAM_UNDERFLOW [03:00] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_CLEAR_STREAM_UNDERFLOW_MASK 0x0000000f
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_CLEAR_STREAM_UNDERFLOW_SHIFT 0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_STATUS_CLEAR_STREAM_UNDERFLOW_DEFAULT 0x00000000

/***************************************************************************
 *ESR_MASK - Mask Status Register
 ***************************************************************************/
/* AUD_FMM_IOP_LOOPBACK_0 :: ESR_MASK :: reserved0 [31:04] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_reserved0_MASK        0xfffffff0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_reserved0_SHIFT       4

/* AUD_FMM_IOP_LOOPBACK_0 :: ESR_MASK :: STREAM_UNDERFLOW [03:00] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_STREAM_UNDERFLOW_MASK 0x0000000f
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_STREAM_UNDERFLOW_SHIFT 0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_STREAM_UNDERFLOW_DEFAULT 0x0000000f

/***************************************************************************
 *ESR_MASK_SET - Mask Set Register
 ***************************************************************************/
/* AUD_FMM_IOP_LOOPBACK_0 :: ESR_MASK_SET :: reserved0 [31:04] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_SET_reserved0_MASK    0xfffffff0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_SET_reserved0_SHIFT   4

/* AUD_FMM_IOP_LOOPBACK_0 :: ESR_MASK_SET :: STREAM_UNDERFLOW [03:00] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_SET_STREAM_UNDERFLOW_MASK 0x0000000f
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_SET_STREAM_UNDERFLOW_SHIFT 0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_SET_STREAM_UNDERFLOW_DEFAULT 0x0000000f

/***************************************************************************
 *ESR_MASK_CLEAR - Mask Clear Register
 ***************************************************************************/
/* AUD_FMM_IOP_LOOPBACK_0 :: ESR_MASK_CLEAR :: reserved0 [31:04] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_CLEAR_reserved0_MASK  0xfffffff0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_CLEAR_reserved0_SHIFT 4

/* AUD_FMM_IOP_LOOPBACK_0 :: ESR_MASK_CLEAR :: STREAM_UNDERFLOW [03:00] */
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_CLEAR_STREAM_UNDERFLOW_MASK 0x0000000f
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_CLEAR_STREAM_UNDERFLOW_SHIFT 0
#define BCHP_AUD_FMM_IOP_LOOPBACK_0_ESR_MASK_CLEAR_STREAM_UNDERFLOW_DEFAULT 0x0000000f

#endif /* #ifndef BCHP_AUD_FMM_IOP_LOOPBACK_0_H__ */

/* End of File */
