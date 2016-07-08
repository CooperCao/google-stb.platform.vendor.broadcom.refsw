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

#ifndef BCHP_HEVD_OL_SINT_0_H__
#define BCHP_HEVD_OL_SINT_0_H__

/***************************************************************************
 *HEVD_OL_SINT_0
 ***************************************************************************/
#define BCHP_HEVD_OL_SINT_0_DMA_ADDR             0x00101000 /* [RW] DMA Address */
#define BCHP_HEVD_OL_SINT_0_DMA_LEN              0x00101004 /* [RW] DMA length */
#define BCHP_HEVD_OL_SINT_0_DMA_BASE             0x00101008 /* [RW] DMA Base Address */
#define BCHP_HEVD_OL_SINT_0_DMA_END              0x0010100c /* [RW] DMA End Address */
#define BCHP_HEVD_OL_SINT_0_STRM_POS             0x00101010 /* [RO] Stream Position */
#define BCHP_HEVD_OL_SINT_0_STRM_STAT            0x00101014 /* [RW] Stream Status */
#define BCHP_HEVD_OL_SINT_0_IENA                 0x00101018 /* [RW] Interrupt Enable */
#define BCHP_HEVD_OL_SINT_0_STRM_BITS            0x0010101c /* [RW] Stream bit 'peek' */
#define BCHP_HEVD_OL_SINT_0_GET_SYMB             0x00101020 /* [RW] Get Symbol */
#define BCHP_HEVD_OL_SINT_0_VPX_PROB_TABLE       0x00101024 /* [RW] VPX Prob Table */
#define BCHP_HEVD_OL_SINT_0_VPX_DEBUG            0x00101028 /* [RO] VPX Debug */

/***************************************************************************
 *DMA_ADDR - DMA Address
 ***************************************************************************/
/* HEVD_OL_SINT_0 :: DMA_ADDR :: Addr [31:02] */
#define BCHP_HEVD_OL_SINT_0_DMA_ADDR_Addr_MASK                     0xfffffffc
#define BCHP_HEVD_OL_SINT_0_DMA_ADDR_Addr_SHIFT                    2

/* HEVD_OL_SINT_0 :: DMA_ADDR :: reserved0 [01:00] */
#define BCHP_HEVD_OL_SINT_0_DMA_ADDR_reserved0_MASK                0x00000003
#define BCHP_HEVD_OL_SINT_0_DMA_ADDR_reserved0_SHIFT               0

/***************************************************************************
 *DMA_LEN - DMA length
 ***************************************************************************/
/* HEVD_OL_SINT_0 :: DMA_LEN :: Length [31:05] */
#define BCHP_HEVD_OL_SINT_0_DMA_LEN_Length_MASK                    0xffffffe0
#define BCHP_HEVD_OL_SINT_0_DMA_LEN_Length_SHIFT                   5

/* HEVD_OL_SINT_0 :: DMA_LEN :: reserved0 [04:00] */
#define BCHP_HEVD_OL_SINT_0_DMA_LEN_reserved0_MASK                 0x0000001f
#define BCHP_HEVD_OL_SINT_0_DMA_LEN_reserved0_SHIFT                0

/***************************************************************************
 *DMA_BASE - DMA Base Address
 ***************************************************************************/
/* HEVD_OL_SINT_0 :: DMA_BASE :: Base [31:08] */
#define BCHP_HEVD_OL_SINT_0_DMA_BASE_Base_MASK                     0xffffff00
#define BCHP_HEVD_OL_SINT_0_DMA_BASE_Base_SHIFT                    8

/* HEVD_OL_SINT_0 :: DMA_BASE :: reserved0 [07:01] */
#define BCHP_HEVD_OL_SINT_0_DMA_BASE_reserved0_MASK                0x000000fe
#define BCHP_HEVD_OL_SINT_0_DMA_BASE_reserved0_SHIFT               1

/* HEVD_OL_SINT_0 :: DMA_BASE :: Endian [00:00] */
#define BCHP_HEVD_OL_SINT_0_DMA_BASE_Endian_MASK                   0x00000001
#define BCHP_HEVD_OL_SINT_0_DMA_BASE_Endian_SHIFT                  0

/***************************************************************************
 *DMA_END - DMA End Address
 ***************************************************************************/
/* HEVD_OL_SINT_0 :: DMA_END :: End [31:00] */
#define BCHP_HEVD_OL_SINT_0_DMA_END_End_MASK                       0xffffffff
#define BCHP_HEVD_OL_SINT_0_DMA_END_End_SHIFT                      0

/***************************************************************************
 *STRM_POS - Stream Position
 ***************************************************************************/
/* HEVD_OL_SINT_0 :: STRM_POS :: Bit_pos [31:00] */
#define BCHP_HEVD_OL_SINT_0_STRM_POS_Bit_pos_MASK                  0xffffffff
#define BCHP_HEVD_OL_SINT_0_STRM_POS_Bit_pos_SHIFT                 0

/***************************************************************************
 *STRM_STAT - Stream Status
 ***************************************************************************/
/* HEVD_OL_SINT_0 :: STRM_STAT :: reserved0 [31:19] */
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_reserved0_MASK               0xfff80000
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_reserved0_SHIFT              19

/* HEVD_OL_SINT_0 :: STRM_STAT :: FlushInput [18:18] */
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_FlushInput_MASK              0x00040000
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_FlushInput_SHIFT             18

/* HEVD_OL_SINT_0 :: STRM_STAT :: reserved1 [17:17] */
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_reserved1_MASK               0x00020000
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_reserved1_SHIFT              17

/* HEVD_OL_SINT_0 :: STRM_STAT :: Rst [16:16] */
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_Rst_MASK                     0x00010000
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_Rst_SHIFT                    16

/* HEVD_OL_SINT_0 :: STRM_STAT :: reserved2 [15:10] */
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_reserved2_MASK               0x0000fc00
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_reserved2_SHIFT              10

/* HEVD_OL_SINT_0 :: STRM_STAT :: Derr [09:09] */
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_Derr_MASK                    0x00000200
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_Derr_SHIFT                   9

/* HEVD_OL_SINT_0 :: STRM_STAT :: Serr [08:08] */
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_Serr_MASK                    0x00000100
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_Serr_SHIFT                   8

/* HEVD_OL_SINT_0 :: STRM_STAT :: reserved3 [07:04] */
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_reserved3_MASK               0x000000f0
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_reserved3_SHIFT              4

/* HEVD_OL_SINT_0 :: STRM_STAT :: Dact [03:03] */
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_Dact_MASK                    0x00000008
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_Dact_SHIFT                   3

/* HEVD_OL_SINT_0 :: STRM_STAT :: reserved4 [02:01] */
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_reserved4_MASK               0x00000006
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_reserved4_SHIFT              1

/* HEVD_OL_SINT_0 :: STRM_STAT :: Sval [00:00] */
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_Sval_MASK                    0x00000001
#define BCHP_HEVD_OL_SINT_0_STRM_STAT_Sval_SHIFT                   0

/***************************************************************************
 *IENA - Interrupt Enable
 ***************************************************************************/
/* HEVD_OL_SINT_0 :: IENA :: reserved0 [31:10] */
#define BCHP_HEVD_OL_SINT_0_IENA_reserved0_MASK                    0xfffffc00
#define BCHP_HEVD_OL_SINT_0_IENA_reserved0_SHIFT                   10

/* HEVD_OL_SINT_0 :: IENA :: Derr [09:09] */
#define BCHP_HEVD_OL_SINT_0_IENA_Derr_MASK                         0x00000200
#define BCHP_HEVD_OL_SINT_0_IENA_Derr_SHIFT                        9

/* HEVD_OL_SINT_0 :: IENA :: Serr [08:08] */
#define BCHP_HEVD_OL_SINT_0_IENA_Serr_MASK                         0x00000100
#define BCHP_HEVD_OL_SINT_0_IENA_Serr_SHIFT                        8

/* HEVD_OL_SINT_0 :: IENA :: reserved1 [07:00] */
#define BCHP_HEVD_OL_SINT_0_IENA_reserved1_MASK                    0x000000ff
#define BCHP_HEVD_OL_SINT_0_IENA_reserved1_SHIFT                   0

/***************************************************************************
 *STRM_BITS - Stream bit 'peek'
 ***************************************************************************/
/* HEVD_OL_SINT_0 :: STRM_BITS :: Stream_Bits [31:00] */
#define BCHP_HEVD_OL_SINT_0_STRM_BITS_Stream_Bits_MASK             0xffffffff
#define BCHP_HEVD_OL_SINT_0_STRM_BITS_Stream_Bits_SHIFT            0

/***************************************************************************
 *GET_SYMB - Get Symbol
 ***************************************************************************/
/* HEVD_OL_SINT_0 :: GET_SYMB :: reserved0 [31:16] */
#define BCHP_HEVD_OL_SINT_0_GET_SYMB_reserved0_MASK                0xffff0000
#define BCHP_HEVD_OL_SINT_0_GET_SYMB_reserved0_SHIFT               16

/* HEVD_OL_SINT_0 :: GET_SYMB :: Type [15:12] */
#define BCHP_HEVD_OL_SINT_0_GET_SYMB_Type_MASK                     0x0000f000
#define BCHP_HEVD_OL_SINT_0_GET_SYMB_Type_SHIFT                    12

/* HEVD_OL_SINT_0 :: GET_SYMB :: SubType [11:08] */
#define BCHP_HEVD_OL_SINT_0_GET_SYMB_SubType_MASK                  0x00000f00
#define BCHP_HEVD_OL_SINT_0_GET_SYMB_SubType_SHIFT                 8

/* HEVD_OL_SINT_0 :: GET_SYMB :: N [07:00] */
#define BCHP_HEVD_OL_SINT_0_GET_SYMB_N_MASK                        0x000000ff
#define BCHP_HEVD_OL_SINT_0_GET_SYMB_N_SHIFT                       0

/***************************************************************************
 *VPX_PROB_TABLE - VPX Prob Table
 ***************************************************************************/
/* HEVD_OL_SINT_0 :: VPX_PROB_TABLE :: Write [31:31] */
#define BCHP_HEVD_OL_SINT_0_VPX_PROB_TABLE_Write_MASK              0x80000000
#define BCHP_HEVD_OL_SINT_0_VPX_PROB_TABLE_Write_SHIFT             31

/* HEVD_OL_SINT_0 :: VPX_PROB_TABLE :: reserved0 [30:26] */
#define BCHP_HEVD_OL_SINT_0_VPX_PROB_TABLE_reserved0_MASK          0x7c000000
#define BCHP_HEVD_OL_SINT_0_VPX_PROB_TABLE_reserved0_SHIFT         26

/* HEVD_OL_SINT_0 :: VPX_PROB_TABLE :: Addr [25:16] */
#define BCHP_HEVD_OL_SINT_0_VPX_PROB_TABLE_Addr_MASK               0x03ff0000
#define BCHP_HEVD_OL_SINT_0_VPX_PROB_TABLE_Addr_SHIFT              16

/* HEVD_OL_SINT_0 :: VPX_PROB_TABLE :: reserved1 [15:08] */
#define BCHP_HEVD_OL_SINT_0_VPX_PROB_TABLE_reserved1_MASK          0x0000ff00
#define BCHP_HEVD_OL_SINT_0_VPX_PROB_TABLE_reserved1_SHIFT         8

/* HEVD_OL_SINT_0 :: VPX_PROB_TABLE :: Data [07:00] */
#define BCHP_HEVD_OL_SINT_0_VPX_PROB_TABLE_Data_MASK               0x000000ff
#define BCHP_HEVD_OL_SINT_0_VPX_PROB_TABLE_Data_SHIFT              0

/***************************************************************************
 *VPX_DEBUG - VPX Debug
 ***************************************************************************/
/* HEVD_OL_SINT_0 :: VPX_DEBUG :: Bool [31:24] */
#define BCHP_HEVD_OL_SINT_0_VPX_DEBUG_Bool_MASK                    0xff000000
#define BCHP_HEVD_OL_SINT_0_VPX_DEBUG_Bool_SHIFT                   24

/* HEVD_OL_SINT_0 :: VPX_DEBUG :: Prob [23:16] */
#define BCHP_HEVD_OL_SINT_0_VPX_DEBUG_Prob_MASK                    0x00ff0000
#define BCHP_HEVD_OL_SINT_0_VPX_DEBUG_Prob_SHIFT                   16

/* HEVD_OL_SINT_0 :: VPX_DEBUG :: Range [15:08] */
#define BCHP_HEVD_OL_SINT_0_VPX_DEBUG_Range_MASK                   0x0000ff00
#define BCHP_HEVD_OL_SINT_0_VPX_DEBUG_Range_SHIFT                  8

/* HEVD_OL_SINT_0 :: VPX_DEBUG :: Value [07:00] */
#define BCHP_HEVD_OL_SINT_0_VPX_DEBUG_Value_MASK                   0x000000ff
#define BCHP_HEVD_OL_SINT_0_VPX_DEBUG_Value_SHIFT                  0

#endif /* #ifndef BCHP_HEVD_OL_SINT_0_H__ */

/* End of File */
