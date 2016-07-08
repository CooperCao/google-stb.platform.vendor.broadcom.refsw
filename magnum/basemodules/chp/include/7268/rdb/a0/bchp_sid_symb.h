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
 * Date:           Generated on               Mon Aug 24 11:29:33 2015
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

#ifndef BCHP_SID_SYMB_H__
#define BCHP_SID_SYMB_H__

/***************************************************************************
 *SID_SYMB
 ***************************************************************************/
#define BCHP_SID_SYMB_LONGEST_CODE               0x20983a00 /* [RW] HuffmanLongestCode */
#define BCHP_SID_SYMB_PNG_XFER_CNT               0x20983a04 /* [RW] PngXferCnt */
#define BCHP_SID_SYMB_JPEG_Y_PRED                0x20983a08 /* [RW] JPEG_Y_Pred */
#define BCHP_SID_SYMB_JPEG_U_PRED                0x20983a0c /* [RW] JPEG_U_Pred */
#define BCHP_SID_SYMB_JPEG_V_PRED                0x20983a10 /* [RW] JPEG_V_Pred */

/***************************************************************************
 *LONGEST_CODE - HuffmanLongestCode
 ***************************************************************************/
/* SID_SYMB :: LONGEST_CODE :: reserved0 [31:29] */
#define BCHP_SID_SYMB_LONGEST_CODE_reserved0_MASK                  0xe0000000
#define BCHP_SID_SYMB_LONGEST_CODE_reserved0_SHIFT                 29

/* SID_SYMB :: LONGEST_CODE :: LastTab0 [28:24] */
#define BCHP_SID_SYMB_LONGEST_CODE_LastTab0_MASK                   0x1f000000
#define BCHP_SID_SYMB_LONGEST_CODE_LastTab0_SHIFT                  24

/* SID_SYMB :: LONGEST_CODE :: reserved1 [23:21] */
#define BCHP_SID_SYMB_LONGEST_CODE_reserved1_MASK                  0x00e00000
#define BCHP_SID_SYMB_LONGEST_CODE_reserved1_SHIFT                 21

/* SID_SYMB :: LONGEST_CODE :: LastTab1 [20:16] */
#define BCHP_SID_SYMB_LONGEST_CODE_LastTab1_MASK                   0x001f0000
#define BCHP_SID_SYMB_LONGEST_CODE_LastTab1_SHIFT                  16

/* SID_SYMB :: LONGEST_CODE :: reserved2 [15:13] */
#define BCHP_SID_SYMB_LONGEST_CODE_reserved2_MASK                  0x0000e000
#define BCHP_SID_SYMB_LONGEST_CODE_reserved2_SHIFT                 13

/* SID_SYMB :: LONGEST_CODE :: LastDCTab0 [12:08] */
#define BCHP_SID_SYMB_LONGEST_CODE_LastDCTab0_MASK                 0x00001f00
#define BCHP_SID_SYMB_LONGEST_CODE_LastDCTab0_SHIFT                8

/* SID_SYMB :: LONGEST_CODE :: reserved3 [07:05] */
#define BCHP_SID_SYMB_LONGEST_CODE_reserved3_MASK                  0x000000e0
#define BCHP_SID_SYMB_LONGEST_CODE_reserved3_SHIFT                 5

/* SID_SYMB :: LONGEST_CODE :: LastDCTab1 [04:00] */
#define BCHP_SID_SYMB_LONGEST_CODE_LastDCTab1_MASK                 0x0000001f
#define BCHP_SID_SYMB_LONGEST_CODE_LastDCTab1_SHIFT                0

/***************************************************************************
 *PNG_XFER_CNT - PngXferCnt
 ***************************************************************************/
/* SID_SYMB :: PNG_XFER_CNT :: Loaded [31:31] */
#define BCHP_SID_SYMB_PNG_XFER_CNT_Loaded_MASK                     0x80000000
#define BCHP_SID_SYMB_PNG_XFER_CNT_Loaded_SHIFT                    31

/* SID_SYMB :: PNG_XFER_CNT :: reserved0 [30:16] */
#define BCHP_SID_SYMB_PNG_XFER_CNT_reserved0_MASK                  0x7fff0000
#define BCHP_SID_SYMB_PNG_XFER_CNT_reserved0_SHIFT                 16

/* SID_SYMB :: PNG_XFER_CNT :: Count [15:00] */
#define BCHP_SID_SYMB_PNG_XFER_CNT_Count_MASK                      0x0000ffff
#define BCHP_SID_SYMB_PNG_XFER_CNT_Count_SHIFT                     0

/***************************************************************************
 *JPEG_Y_PRED - JPEG_Y_Pred
 ***************************************************************************/
/* SID_SYMB :: JPEG_Y_PRED :: reserved0 [31:12] */
#define BCHP_SID_SYMB_JPEG_Y_PRED_reserved0_MASK                   0xfffff000
#define BCHP_SID_SYMB_JPEG_Y_PRED_reserved0_SHIFT                  12

/* SID_SYMB :: JPEG_Y_PRED :: Predicted [11:00] */
#define BCHP_SID_SYMB_JPEG_Y_PRED_Predicted_MASK                   0x00000fff
#define BCHP_SID_SYMB_JPEG_Y_PRED_Predicted_SHIFT                  0

/***************************************************************************
 *JPEG_U_PRED - JPEG_U_Pred
 ***************************************************************************/
/* SID_SYMB :: JPEG_U_PRED :: reserved0 [31:12] */
#define BCHP_SID_SYMB_JPEG_U_PRED_reserved0_MASK                   0xfffff000
#define BCHP_SID_SYMB_JPEG_U_PRED_reserved0_SHIFT                  12

/* SID_SYMB :: JPEG_U_PRED :: Predicted [11:00] */
#define BCHP_SID_SYMB_JPEG_U_PRED_Predicted_MASK                   0x00000fff
#define BCHP_SID_SYMB_JPEG_U_PRED_Predicted_SHIFT                  0

/***************************************************************************
 *JPEG_V_PRED - JPEG_V_Pred
 ***************************************************************************/
/* SID_SYMB :: JPEG_V_PRED :: reserved0 [31:12] */
#define BCHP_SID_SYMB_JPEG_V_PRED_reserved0_MASK                   0xfffff000
#define BCHP_SID_SYMB_JPEG_V_PRED_reserved0_SHIFT                  12

/* SID_SYMB :: JPEG_V_PRED :: Predicted [11:00] */
#define BCHP_SID_SYMB_JPEG_V_PRED_Predicted_MASK                   0x00000fff
#define BCHP_SID_SYMB_JPEG_V_PRED_Predicted_SHIFT                  0

#endif /* #ifndef BCHP_SID_SYMB_H__ */

/* End of File */
