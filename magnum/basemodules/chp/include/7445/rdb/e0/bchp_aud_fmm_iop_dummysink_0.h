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
 * Date:           Generated on               Thu Dec  4 13:58:58 2014
 *                 Full Compile MD5 Checksum  7f8620d6db569529bdb0529f9fa1fcf2
 *                     (minus title and desc)
 *                 MD5 Checksum               48c8fdf2af4291bac0fa2b0d5245d05c
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     15262
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_AUD_FMM_IOP_DUMMYSINK_0_H__
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_H__

/***************************************************************************
 *AUD_FMM_IOP_DUMMYSINK_0
 ***************************************************************************/

/***************************************************************************
 *STREAM_CFG_0_%i - Stream configuration
 ***************************************************************************/
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_ARRAY_BASE     0x00cb1a00
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_ARRAY_START    0
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_ARRAY_END      7
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *STREAM_CFG_0_%i - Stream configuration
 ***************************************************************************/
/* AUD_FMM_IOP_DUMMYSINK_0 :: STREAM_CFG_0_i :: ENA [31:31] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_ENA_MASK       0x80000000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_ENA_SHIFT      31
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_ENA_DEFAULT    0x00000000

/* AUD_FMM_IOP_DUMMYSINK_0 :: STREAM_CFG_0_i :: reserved0 [30:28] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_reserved0_MASK 0x70000000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_reserved0_SHIFT 28

/* AUD_FMM_IOP_DUMMYSINK_0 :: STREAM_CFG_0_i :: CHANNEL_GROUPING [27:24] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_CHANNEL_GROUPING_MASK 0x0f000000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_CHANNEL_GROUPING_SHIFT 24
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_CHANNEL_GROUPING_DEFAULT 0x00000001

/* AUD_FMM_IOP_DUMMYSINK_0 :: STREAM_CFG_0_i :: GROUP_ID [23:20] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_GROUP_ID_MASK  0x00f00000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_GROUP_ID_SHIFT 20
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_GROUP_ID_DEFAULT 0x00000000

/* AUD_FMM_IOP_DUMMYSINK_0 :: STREAM_CFG_0_i :: STREAM_BIT_RESOLUTION [19:16] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_STREAM_BIT_RESOLUTION_MASK 0x000f0000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_STREAM_BIT_RESOLUTION_SHIFT 16
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_STREAM_BIT_RESOLUTION_DEFAULT 0x00000008
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_STREAM_BIT_RESOLUTION_Res_16_Bit 0
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_STREAM_BIT_RESOLUTION_Res_17_Bit 1
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_STREAM_BIT_RESOLUTION_Res_18_Bit 2
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_STREAM_BIT_RESOLUTION_Res_19_Bit 3
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_STREAM_BIT_RESOLUTION_Res_20_Bit 4
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_STREAM_BIT_RESOLUTION_Res_21_Bit 5
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_STREAM_BIT_RESOLUTION_Res_22_Bit 6
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_STREAM_BIT_RESOLUTION_Res_23_Bit 7
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_STREAM_BIT_RESOLUTION_Res_24_Bit 8

/* AUD_FMM_IOP_DUMMYSINK_0 :: STREAM_CFG_0_i :: WAIT_FOR_VALID [15:15] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_WAIT_FOR_VALID_MASK 0x00008000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_WAIT_FOR_VALID_SHIFT 15
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_WAIT_FOR_VALID_DEFAULT 0x00000000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_WAIT_FOR_VALID_Holdoff_request 1
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_WAIT_FOR_VALID_Keep_requesting 0

/* AUD_FMM_IOP_DUMMYSINK_0 :: STREAM_CFG_0_i :: IGNORE_FIRST_UNDERFLOW [14:14] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_IGNORE_FIRST_UNDERFLOW_MASK 0x00004000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_IGNORE_FIRST_UNDERFLOW_SHIFT 14
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_IGNORE_FIRST_UNDERFLOW_DEFAULT 0x00000001
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_IGNORE_FIRST_UNDERFLOW_Ignore 1
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_IGNORE_FIRST_UNDERFLOW_Dont_ignore 0

/* AUD_FMM_IOP_DUMMYSINK_0 :: STREAM_CFG_0_i :: INIT_SM [13:13] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_INIT_SM_MASK   0x00002000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_INIT_SM_SHIFT  13
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_INIT_SM_DEFAULT 0x00000000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_INIT_SM_Init   1
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_INIT_SM_Normal 0

/* AUD_FMM_IOP_DUMMYSINK_0 :: STREAM_CFG_0_i :: INS_INVAL [12:12] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_INS_INVAL_MASK 0x00001000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_INS_INVAL_SHIFT 12
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_INS_INVAL_DEFAULT 0x00000000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_INS_INVAL_Invalid 1
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_INS_INVAL_Valid 0

/* AUD_FMM_IOP_DUMMYSINK_0 :: STREAM_CFG_0_i :: reserved1 [11:10] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_reserved1_MASK 0x00000c00
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_reserved1_SHIFT 10

/* AUD_FMM_IOP_DUMMYSINK_0 :: STREAM_CFG_0_i :: FCI_ID [09:00] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_FCI_ID_MASK    0x000003ff
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_FCI_ID_SHIFT   0
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_STREAM_CFG_0_i_FCI_ID_DEFAULT 0x000003ff


/***************************************************************************
 *MCLK_CFG_%i - DummySink MCLK configuration
 ***************************************************************************/
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_ARRAY_BASE         0x00cb1a40
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_ARRAY_START        0
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_ARRAY_END          7
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *MCLK_CFG_%i - DummySink MCLK configuration
 ***************************************************************************/
/* AUD_FMM_IOP_DUMMYSINK_0 :: MCLK_CFG_i :: reserved0 [31:20] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_reserved0_MASK     0xfff00000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_reserved0_SHIFT    20

/* AUD_FMM_IOP_DUMMYSINK_0 :: MCLK_CFG_i :: MCLK_RATE [19:16] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_MCLK_RATE_MASK     0x000f0000
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_MCLK_RATE_SHIFT    16
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_MCLK_RATE_DEFAULT  0x00000002
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_MCLK_RATE_MCLK_512fs_SCLK_64fs 4
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_MCLK_RATE_MCLK_384fs_SCLK_64fs 3
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_MCLK_RATE_MCLK_256fs_SCLK_64fs 2
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_MCLK_RATE_MCLK_128fs_SCLK_64fs 1

/* AUD_FMM_IOP_DUMMYSINK_0 :: MCLK_CFG_i :: reserved1 [15:04] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_reserved1_MASK     0x0000fff0
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_reserved1_SHIFT    4

/* AUD_FMM_IOP_DUMMYSINK_0 :: MCLK_CFG_i :: PLLCLKSEL [03:00] */
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_MASK     0x0000000f
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_SHIFT    0
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_DEFAULT  0x00000001
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_PLL0_ch1 0
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_PLL0_ch2 1
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_PLL0_ch3 2
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_PLL1_ch1 3
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_PLL1_ch2 4
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_PLL1_ch3 5
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_PLL2_ch1 6
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_PLL2_ch2 7
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_PLL2_ch3 8
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen0 9
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen1 10
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen2 11
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen3 12
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen4 13
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen5 14
#define BCHP_AUD_FMM_IOP_DUMMYSINK_0_MCLK_CFG_i_PLLCLKSEL_Mclk_gen6 15


#endif /* #ifndef BCHP_AUD_FMM_IOP_DUMMYSINK_0_H__ */

/* End of File */
