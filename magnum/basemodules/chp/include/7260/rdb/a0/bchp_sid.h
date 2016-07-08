/********************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Fri Feb 26 13:24:12 2016
 *                 Full Compile MD5 Checksum  1560bfee4f086d6e1d49e6bd3406a38d
 *                     (minus title and desc)
 *                 MD5 Checksum               8d7264bb382089f88abd2b1abb2a6340
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     823
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_SID_H__
#define BCHP_SID_H__

/***************************************************************************
 *SID - Control
 ***************************************************************************/
#define BCHP_SID_MAIN_CTL                        0x20980100 /* [RW] Main Control Register */
#define BCHP_SID_COMMAND                         0x20980104 /* [WO] Command Register */
#define BCHP_SID_STATUS                          0x20980108 /* [RW] Status Register */
#define BCHP_SID_IMAGE_FMT                       0x2098010c /* [RW] Image Format */
#define BCHP_SID_VERSION                         0x20980110 /* [RO] Version Register */
#define BCHP_SID_IMAGE_WIDTH                     0x20980114 /* [RW] Image Width */
#define BCHP_SID_IMAGE_HEIGHT                    0x20980118 /* [RW] Image Height */
#define BCHP_SID_PROCESS_TIMEOUT_CNT             0x2098011c /* [RW] Process Timeout Cnt */
#define BCHP_SID_INT_IS_EDGE                     0x20980120 /* [RW] Interrupt Type (Edge/Level) */
#define BCHP_SID_INT_CLR                         0x20980124 /* [WO] Interrupt Clear */
#define BCHP_SID_FIFO_DATA                       0x20980140 /* [WO] Image Fifo Data */
#define BCHP_SID_DEBUG_IDCT                      0x20980180 /* [RO] IDCT Debug Register */
#define BCHP_SID_DEBUG_RAS_HW                    0x20980184 /* [RO] RAS Debug */
#define BCHP_SID_DEBUG_RAS_FLAG                  0x20980188 /* [RO] Ras Flag Debug */
#define BCHP_SID_DEBUG_HORCOL                    0x2098018c /* [RO] HorCol Debug */
#define BCHP_SID_DEBUG_PNG                       0x20980190 /* [RO] PNG Debug */
#define BCHP_SID_DEBUG_RLE                       0x20980194 /* [RO] RLE Debug */
#define BCHP_SID_DEBUG_GIF                       0x20980198 /* [RO] GIF Debug */
#define BCHP_SID_DEBUG_SYM_DCD                   0x2098019c /* [RO] Symb Dcd Debug */

/***************************************************************************
 *MAIN_CTL - Main Control Register
 ***************************************************************************/
/* SID :: MAIN_CTL :: reserved0 [31:28] */
#define BCHP_SID_MAIN_CTL_reserved0_MASK                           0xf0000000
#define BCHP_SID_MAIN_CTL_reserved0_SHIFT                          28

/* SID :: MAIN_CTL :: VH_TAB [27:26] */
#define BCHP_SID_MAIN_CTL_VH_TAB_MASK                              0x0c000000
#define BCHP_SID_MAIN_CTL_VH_TAB_SHIFT                             26

/* SID :: MAIN_CTL :: UH_TAB [25:24] */
#define BCHP_SID_MAIN_CTL_UH_TAB_MASK                              0x03000000
#define BCHP_SID_MAIN_CTL_UH_TAB_SHIFT                             24

/* SID :: MAIN_CTL :: YH_TAB [23:22] */
#define BCHP_SID_MAIN_CTL_YH_TAB_MASK                              0x00c00000
#define BCHP_SID_MAIN_CTL_YH_TAB_SHIFT                             22

/* SID :: MAIN_CTL :: V_QTAB [21:20] */
#define BCHP_SID_MAIN_CTL_V_QTAB_MASK                              0x00300000
#define BCHP_SID_MAIN_CTL_V_QTAB_SHIFT                             20
#define BCHP_SID_MAIN_CTL_V_QTAB_DEFAULT                           0x00000000

/* SID :: MAIN_CTL :: U_QTAB [19:18] */
#define BCHP_SID_MAIN_CTL_U_QTAB_MASK                              0x000c0000
#define BCHP_SID_MAIN_CTL_U_QTAB_SHIFT                             18
#define BCHP_SID_MAIN_CTL_U_QTAB_DEFAULT                           0x00000000

/* SID :: MAIN_CTL :: Y_QTAB [17:16] */
#define BCHP_SID_MAIN_CTL_Y_QTAB_MASK                              0x00030000
#define BCHP_SID_MAIN_CTL_Y_QTAB_SHIFT                             16
#define BCHP_SID_MAIN_CTL_Y_QTAB_DEFAULT                           0x00000000

/* SID :: MAIN_CTL :: reserved1 [15:12] */
#define BCHP_SID_MAIN_CTL_reserved1_MASK                           0x0000f000
#define BCHP_SID_MAIN_CTL_reserved1_SHIFT                          12

/* SID :: MAIN_CTL :: SPARE [11:11] */
#define BCHP_SID_MAIN_CTL_SPARE_MASK                               0x00000800
#define BCHP_SID_MAIN_CTL_SPARE_SHIFT                              11
#define BCHP_SID_MAIN_CTL_SPARE_DEFAULT                            0x00000000

/* SID :: MAIN_CTL :: V_FILT [10:10] */
#define BCHP_SID_MAIN_CTL_V_FILT_MASK                              0x00000400
#define BCHP_SID_MAIN_CTL_V_FILT_SHIFT                             10
#define BCHP_SID_MAIN_CTL_V_FILT_DEFAULT                           0x00000000

/* SID :: MAIN_CTL :: H_FILT [09:08] */
#define BCHP_SID_MAIN_CTL_H_FILT_MASK                              0x00000300
#define BCHP_SID_MAIN_CTL_H_FILT_SHIFT                             8
#define BCHP_SID_MAIN_CTL_H_FILT_DEFAULT                           0x00000000

/* SID :: MAIN_CTL :: LZW_SIZE [07:04] */
#define BCHP_SID_MAIN_CTL_LZW_SIZE_MASK                            0x000000f0
#define BCHP_SID_MAIN_CTL_LZW_SIZE_SHIFT                           4
#define BCHP_SID_MAIN_CTL_LZW_SIZE_DEFAULT                         0x00000000

/* SID :: MAIN_CTL :: reserved2 [03:02] */
#define BCHP_SID_MAIN_CTL_reserved2_MASK                           0x0000000c
#define BCHP_SID_MAIN_CTL_reserved2_SHIFT                          2

/* SID :: MAIN_CTL :: RLE_LUT [01:01] */
#define BCHP_SID_MAIN_CTL_RLE_LUT_MASK                             0x00000002
#define BCHP_SID_MAIN_CTL_RLE_LUT_SHIFT                            1
#define BCHP_SID_MAIN_CTL_RLE_LUT_DEFAULT                          0x00000000

/* SID :: MAIN_CTL :: PEEK_ENDIAN [00:00] */
#define BCHP_SID_MAIN_CTL_PEEK_ENDIAN_MASK                         0x00000001
#define BCHP_SID_MAIN_CTL_PEEK_ENDIAN_SHIFT                        0
#define BCHP_SID_MAIN_CTL_PEEK_ENDIAN_DEFAULT                      0x00000000

/***************************************************************************
 *COMMAND - Command Register
 ***************************************************************************/
/* SID :: COMMAND :: reserved0 [31:13] */
#define BCHP_SID_COMMAND_reserved0_MASK                            0xffffe000
#define BCHP_SID_COMMAND_reserved0_SHIFT                           13

/* SID :: COMMAND :: IMG_RESTART [12:12] */
#define BCHP_SID_COMMAND_IMG_RESTART_MASK                          0x00001000
#define BCHP_SID_COMMAND_IMG_RESTART_SHIFT                         12
#define BCHP_SID_COMMAND_IMG_RESTART_DEFAULT                       0x00000000

/* SID :: COMMAND :: DMA_CNT_DEC [11:11] */
#define BCHP_SID_COMMAND_DMA_CNT_DEC_MASK                          0x00000800
#define BCHP_SID_COMMAND_DMA_CNT_DEC_SHIFT                         11
#define BCHP_SID_COMMAND_DMA_CNT_DEC_DEFAULT                       0x00000000

/* SID :: COMMAND :: DMA_CNT_CLR [10:10] */
#define BCHP_SID_COMMAND_DMA_CNT_CLR_MASK                          0x00000400
#define BCHP_SID_COMMAND_DMA_CNT_CLR_SHIFT                         10
#define BCHP_SID_COMMAND_DMA_CNT_CLR_DEFAULT                       0x00000000

/* SID :: COMMAND :: TIMEOUT_ENA [09:09] */
#define BCHP_SID_COMMAND_TIMEOUT_ENA_MASK                          0x00000200
#define BCHP_SID_COMMAND_TIMEOUT_ENA_SHIFT                         9
#define BCHP_SID_COMMAND_TIMEOUT_ENA_DEFAULT                       0x00000000

/* SID :: COMMAND :: STREAM_IN_RST [08:08] */
#define BCHP_SID_COMMAND_STREAM_IN_RST_MASK                        0x00000100
#define BCHP_SID_COMMAND_STREAM_IN_RST_SHIFT                       8
#define BCHP_SID_COMMAND_STREAM_IN_RST_DEFAULT                     0x00000000

/* SID :: COMMAND :: STRM_FLUSH [07:07] */
#define BCHP_SID_COMMAND_STRM_FLUSH_MASK                           0x00000080
#define BCHP_SID_COMMAND_STRM_FLUSH_SHIFT                          7
#define BCHP_SID_COMMAND_STRM_FLUSH_DEFAULT                        0x00000000

/* SID :: COMMAND :: OUT_FLUSH [06:06] */
#define BCHP_SID_COMMAND_OUT_FLUSH_MASK                            0x00000040
#define BCHP_SID_COMMAND_OUT_FLUSH_SHIFT                           6
#define BCHP_SID_COMMAND_OUT_FLUSH_DEFAULT                         0x00000000

/* SID :: COMMAND :: BIT_SHIFT_RST [05:05] */
#define BCHP_SID_COMMAND_BIT_SHIFT_RST_MASK                        0x00000020
#define BCHP_SID_COMMAND_BIT_SHIFT_RST_SHIFT                       5
#define BCHP_SID_COMMAND_BIT_SHIFT_RST_DEFAULT                     0x00000000

/* SID :: COMMAND :: JPEG_DC_RST [04:04] */
#define BCHP_SID_COMMAND_JPEG_DC_RST_MASK                          0x00000010
#define BCHP_SID_COMMAND_JPEG_DC_RST_SHIFT                         4
#define BCHP_SID_COMMAND_JPEG_DC_RST_DEFAULT                       0x00000000

/* SID :: COMMAND :: FE_START [03:03] */
#define BCHP_SID_COMMAND_FE_START_MASK                             0x00000008
#define BCHP_SID_COMMAND_FE_START_SHIFT                            3
#define BCHP_SID_COMMAND_FE_START_DEFAULT                          0x00000000

/* SID :: COMMAND :: BE_START [02:02] */
#define BCHP_SID_COMMAND_BE_START_MASK                             0x00000004
#define BCHP_SID_COMMAND_BE_START_SHIFT                            2
#define BCHP_SID_COMMAND_BE_START_DEFAULT                          0x00000000

/* SID :: COMMAND :: PDMA_RST [01:01] */
#define BCHP_SID_COMMAND_PDMA_RST_MASK                             0x00000002
#define BCHP_SID_COMMAND_PDMA_RST_SHIFT                            1
#define BCHP_SID_COMMAND_PDMA_RST_DEFAULT                          0x00000000

/* SID :: COMMAND :: RESET [00:00] */
#define BCHP_SID_COMMAND_RESET_MASK                                0x00000001
#define BCHP_SID_COMMAND_RESET_SHIFT                               0
#define BCHP_SID_COMMAND_RESET_DEFAULT                             0x00000000

/***************************************************************************
 *STATUS - Status Register
 ***************************************************************************/
/* SID :: STATUS :: SYS_BIG_END [31:31] */
#define BCHP_SID_STATUS_SYS_BIG_END_MASK                           0x80000000
#define BCHP_SID_STATUS_SYS_BIG_END_SHIFT                          31
#define BCHP_SID_STATUS_SYS_BIG_END_DEFAULT                        0x00000000

/* SID :: STATUS :: reserved0 [30:30] */
#define BCHP_SID_STATUS_reserved0_MASK                             0x40000000
#define BCHP_SID_STATUS_reserved0_SHIFT                            30

/* SID :: STATUS :: DONE_CNT [29:28] */
#define BCHP_SID_STATUS_DONE_CNT_MASK                              0x30000000
#define BCHP_SID_STATUS_DONE_CNT_SHIFT                             28
#define BCHP_SID_STATUS_DONE_CNT_DEFAULT                           0x00000000

/* SID :: STATUS :: reserved1 [27:25] */
#define BCHP_SID_STATUS_reserved1_MASK                             0x0e000000
#define BCHP_SID_STATUS_reserved1_SHIFT                            25

/* SID :: STATUS :: PIX_DMA_MERR [24:24] */
#define BCHP_SID_STATUS_PIX_DMA_MERR_MASK                          0x01000000
#define BCHP_SID_STATUS_PIX_DMA_MERR_SHIFT                         24
#define BCHP_SID_STATUS_PIX_DMA_MERR_DEFAULT                       0x00000000

/* SID :: STATUS :: SYMB_DCD_ERR [23:23] */
#define BCHP_SID_STATUS_SYMB_DCD_ERR_MASK                          0x00800000
#define BCHP_SID_STATUS_SYMB_DCD_ERR_SHIFT                         23
#define BCHP_SID_STATUS_SYMB_DCD_ERR_DEFAULT                       0x00000000

/* SID :: STATUS :: PIX_DMA_ERR [22:22] */
#define BCHP_SID_STATUS_PIX_DMA_ERR_MASK                           0x00400000
#define BCHP_SID_STATUS_PIX_DMA_ERR_SHIFT                          22
#define BCHP_SID_STATUS_PIX_DMA_ERR_DEFAULT                        0x00000000

/* SID :: STATUS :: RLE_ERR [21:21] */
#define BCHP_SID_STATUS_RLE_ERR_MASK                               0x00200000
#define BCHP_SID_STATUS_RLE_ERR_SHIFT                              21
#define BCHP_SID_STATUS_RLE_ERR_DEFAULT                            0x00000000

/* SID :: STATUS :: GIF_ERR [20:20] */
#define BCHP_SID_STATUS_GIF_ERR_MASK                               0x00100000
#define BCHP_SID_STATUS_GIF_ERR_SHIFT                              20
#define BCHP_SID_STATUS_GIF_ERR_DEFAULT                            0x00000000

/* SID :: STATUS :: PNG_ERR [19:19] */
#define BCHP_SID_STATUS_PNG_ERR_MASK                               0x00080000
#define BCHP_SID_STATUS_PNG_ERR_SHIFT                              19
#define BCHP_SID_STATUS_PNG_ERR_DEFAULT                            0x00000000

/* SID :: STATUS :: PEEK_ERR [18:18] */
#define BCHP_SID_STATUS_PEEK_ERR_MASK                              0x00040000
#define BCHP_SID_STATUS_PEEK_ERR_SHIFT                             18
#define BCHP_SID_STATUS_PEEK_ERR_DEFAULT                           0x00000000

/* SID :: STATUS :: STRM_IN_ERR [17:17] */
#define BCHP_SID_STATUS_STRM_IN_ERR_MASK                           0x00020000
#define BCHP_SID_STATUS_STRM_IN_ERR_SHIFT                          17
#define BCHP_SID_STATUS_STRM_IN_ERR_DEFAULT                        0x00000000

/* SID :: STATUS :: TIMEOUT_ERR [16:16] */
#define BCHP_SID_STATUS_TIMEOUT_ERR_MASK                           0x00010000
#define BCHP_SID_STATUS_TIMEOUT_ERR_SHIFT                          16
#define BCHP_SID_STATUS_TIMEOUT_ERR_DEFAULT                        0x00000000

/* SID :: STATUS :: FIFO_FULL [15:15] */
#define BCHP_SID_STATUS_FIFO_FULL_MASK                             0x00008000
#define BCHP_SID_STATUS_FIFO_FULL_SHIFT                            15
#define BCHP_SID_STATUS_FIFO_FULL_DEFAULT                          0x00000000

/* SID :: STATUS :: PNG_PASS_DONE [14:14] */
#define BCHP_SID_STATUS_PNG_PASS_DONE_MASK                         0x00004000
#define BCHP_SID_STATUS_PNG_PASS_DONE_SHIFT                        14
#define BCHP_SID_STATUS_PNG_PASS_DONE_DEFAULT                      0x00000000

/* SID :: STATUS :: PDMA_FIFO_FULL [13:13] */
#define BCHP_SID_STATUS_PDMA_FIFO_FULL_MASK                        0x00002000
#define BCHP_SID_STATUS_PDMA_FIFO_FULL_SHIFT                       13
#define BCHP_SID_STATUS_PDMA_FIFO_FULL_DEFAULT                     0x00000000

/* SID :: STATUS :: PDMA_FIFO_HALF [12:12] */
#define BCHP_SID_STATUS_PDMA_FIFO_HALF_MASK                        0x00001000
#define BCHP_SID_STATUS_PDMA_FIFO_HALF_SHIFT                       12
#define BCHP_SID_STATUS_PDMA_FIFO_HALF_DEFAULT                     0x00000000

/* SID :: STATUS :: STRM_DMA_DONE [11:11] */
#define BCHP_SID_STATUS_STRM_DMA_DONE_MASK                         0x00000800
#define BCHP_SID_STATUS_STRM_DMA_DONE_SHIFT                        11
#define BCHP_SID_STATUS_STRM_DMA_DONE_DEFAULT                      0x00000000

/* SID :: STATUS :: STREAM_IN_ACTIVE [10:10] */
#define BCHP_SID_STATUS_STREAM_IN_ACTIVE_MASK                      0x00000400
#define BCHP_SID_STATUS_STREAM_IN_ACTIVE_SHIFT                     10
#define BCHP_SID_STATUS_STREAM_IN_ACTIVE_DEFAULT                   0x00000000

/* SID :: STATUS :: BIT_SHIFT_ACTIVE [09:09] */
#define BCHP_SID_STATUS_BIT_SHIFT_ACTIVE_MASK                      0x00000200
#define BCHP_SID_STATUS_BIT_SHIFT_ACTIVE_SHIFT                     9
#define BCHP_SID_STATUS_BIT_SHIFT_ACTIVE_DEFAULT                   0x00000000

/* SID :: STATUS :: SYMB_ACTIVE [08:08] */
#define BCHP_SID_STATUS_SYMB_ACTIVE_MASK                           0x00000100
#define BCHP_SID_STATUS_SYMB_ACTIVE_SHIFT                          8
#define BCHP_SID_STATUS_SYMB_ACTIVE_DEFAULT                        0x00000000

/* SID :: STATUS :: FIFO_VALID [07:07] */
#define BCHP_SID_STATUS_FIFO_VALID_MASK                            0x00000080
#define BCHP_SID_STATUS_FIFO_VALID_SHIFT                           7
#define BCHP_SID_STATUS_FIFO_VALID_DEFAULT                         0x00000000

/* SID :: STATUS :: OUTPUT_ACTIVE [06:06] */
#define BCHP_SID_STATUS_OUTPUT_ACTIVE_MASK                         0x00000040
#define BCHP_SID_STATUS_OUTPUT_ACTIVE_SHIFT                        6
#define BCHP_SID_STATUS_OUTPUT_ACTIVE_DEFAULT                      0x00000000

/* SID :: STATUS :: JPEG_ACTIVE [05:05] */
#define BCHP_SID_STATUS_JPEG_ACTIVE_MASK                           0x00000020
#define BCHP_SID_STATUS_JPEG_ACTIVE_SHIFT                          5
#define BCHP_SID_STATUS_JPEG_ACTIVE_DEFAULT                        0x00000000

/* SID :: STATUS :: GIF_ACTIVE [04:04] */
#define BCHP_SID_STATUS_GIF_ACTIVE_MASK                            0x00000010
#define BCHP_SID_STATUS_GIF_ACTIVE_SHIFT                           4
#define BCHP_SID_STATUS_GIF_ACTIVE_DEFAULT                         0x00000000

/* SID :: STATUS :: PNG_ACTIVE [03:03] */
#define BCHP_SID_STATUS_PNG_ACTIVE_MASK                            0x00000008
#define BCHP_SID_STATUS_PNG_ACTIVE_SHIFT                           3
#define BCHP_SID_STATUS_PNG_ACTIVE_DEFAULT                         0x00000000

/* SID :: STATUS :: RLE_ACTIVE [02:02] */
#define BCHP_SID_STATUS_RLE_ACTIVE_MASK                            0x00000004
#define BCHP_SID_STATUS_RLE_ACTIVE_SHIFT                           2
#define BCHP_SID_STATUS_RLE_ACTIVE_DEFAULT                         0x00000000

/* SID :: STATUS :: FE_ACTIVE [01:01] */
#define BCHP_SID_STATUS_FE_ACTIVE_MASK                             0x00000002
#define BCHP_SID_STATUS_FE_ACTIVE_SHIFT                            1
#define BCHP_SID_STATUS_FE_ACTIVE_DEFAULT                          0x00000000

/* SID :: STATUS :: BE_ACTIVE [00:00] */
#define BCHP_SID_STATUS_BE_ACTIVE_MASK                             0x00000001
#define BCHP_SID_STATUS_BE_ACTIVE_SHIFT                            0
#define BCHP_SID_STATUS_BE_ACTIVE_DEFAULT                          0x00000000

/***************************************************************************
 *IMAGE_FMT - Image Format
 ***************************************************************************/
/* SID :: IMAGE_FMT :: reserved0 [31:24] */
#define BCHP_SID_IMAGE_FMT_reserved0_MASK                          0xff000000
#define BCHP_SID_IMAGE_FMT_reserved0_SHIFT                         24

/* SID :: IMAGE_FMT :: Alpha [23:16] */
#define BCHP_SID_IMAGE_FMT_Alpha_MASK                              0x00ff0000
#define BCHP_SID_IMAGE_FMT_Alpha_SHIFT                             16
#define BCHP_SID_IMAGE_FMT_Alpha_DEFAULT                           0x00000000

/* SID :: IMAGE_FMT :: reserved1 [15:14] */
#define BCHP_SID_IMAGE_FMT_reserved1_MASK                          0x0000c000
#define BCHP_SID_IMAGE_FMT_reserved1_SHIFT                         14

/* SID :: IMAGE_FMT :: MPASS [13:13] */
#define BCHP_SID_IMAGE_FMT_MPASS_MASK                              0x00002000
#define BCHP_SID_IMAGE_FMT_MPASS_SHIFT                             13
#define BCHP_SID_IMAGE_FMT_MPASS_DEFAULT                           0x00000000

/* SID :: IMAGE_FMT :: BPP [12:08] */
#define BCHP_SID_IMAGE_FMT_BPP_MASK                                0x00001f00
#define BCHP_SID_IMAGE_FMT_BPP_SHIFT                               8
#define BCHP_SID_IMAGE_FMT_BPP_DEFAULT                             0x00000000

/* SID :: IMAGE_FMT :: Sub_Fmt [07:04] */
#define BCHP_SID_IMAGE_FMT_Sub_Fmt_MASK                            0x000000f0
#define BCHP_SID_IMAGE_FMT_Sub_Fmt_SHIFT                           4
#define BCHP_SID_IMAGE_FMT_Sub_Fmt_DEFAULT                         0x00000000

/* SID :: IMAGE_FMT :: Format [03:00] */
#define BCHP_SID_IMAGE_FMT_Format_MASK                             0x0000000f
#define BCHP_SID_IMAGE_FMT_Format_SHIFT                            0
#define BCHP_SID_IMAGE_FMT_Format_DEFAULT                          0x00000000
#define BCHP_SID_IMAGE_FMT_Format_RLE                              0
#define BCHP_SID_IMAGE_FMT_Format_PNG                              1
#define BCHP_SID_IMAGE_FMT_Format_GIF                              2
#define BCHP_SID_IMAGE_FMT_Format_JPEG                             3

/***************************************************************************
 *VERSION - Version Register
 ***************************************************************************/
/* SID :: VERSION :: ProcID [31:24] */
#define BCHP_SID_VERSION_ProcID_MASK                               0xff000000
#define BCHP_SID_VERSION_ProcID_SHIFT                              24
#define BCHP_SID_VERSION_ProcID_DEFAULT                            0x00000005

/* SID :: VERSION :: Major [23:16] */
#define BCHP_SID_VERSION_Major_MASK                                0x00ff0000
#define BCHP_SID_VERSION_Major_SHIFT                               16
#define BCHP_SID_VERSION_Major_DEFAULT                             0x00000001

/* SID :: VERSION :: Minor [15:08] */
#define BCHP_SID_VERSION_Minor_MASK                                0x0000ff00
#define BCHP_SID_VERSION_Minor_SHIFT                               8
#define BCHP_SID_VERSION_Minor_DEFAULT                             0x00000002

/* SID :: VERSION :: Debug [07:00] */
#define BCHP_SID_VERSION_Debug_MASK                                0x000000ff
#define BCHP_SID_VERSION_Debug_SHIFT                               0
#define BCHP_SID_VERSION_Debug_DEFAULT                             0x00000001

/***************************************************************************
 *IMAGE_WIDTH - Image Width
 ***************************************************************************/
/* SID :: IMAGE_WIDTH :: reserved0 [31:14] */
#define BCHP_SID_IMAGE_WIDTH_reserved0_MASK                        0xffffc000
#define BCHP_SID_IMAGE_WIDTH_reserved0_SHIFT                       14

/* SID :: IMAGE_WIDTH :: ImageWidth [13:00] */
#define BCHP_SID_IMAGE_WIDTH_ImageWidth_MASK                       0x00003fff
#define BCHP_SID_IMAGE_WIDTH_ImageWidth_SHIFT                      0
#define BCHP_SID_IMAGE_WIDTH_ImageWidth_DEFAULT                    0x00000000

/***************************************************************************
 *IMAGE_HEIGHT - Image Height
 ***************************************************************************/
/* SID :: IMAGE_HEIGHT :: reserved0 [31:14] */
#define BCHP_SID_IMAGE_HEIGHT_reserved0_MASK                       0xffffc000
#define BCHP_SID_IMAGE_HEIGHT_reserved0_SHIFT                      14

/* SID :: IMAGE_HEIGHT :: ImageHeight [13:00] */
#define BCHP_SID_IMAGE_HEIGHT_ImageHeight_MASK                     0x00003fff
#define BCHP_SID_IMAGE_HEIGHT_ImageHeight_SHIFT                    0
#define BCHP_SID_IMAGE_HEIGHT_ImageHeight_DEFAULT                  0x00000000

/***************************************************************************
 *PROCESS_TIMEOUT_CNT - Process Timeout Cnt
 ***************************************************************************/
/* SID :: PROCESS_TIMEOUT_CNT :: reserved0 [31:16] */
#define BCHP_SID_PROCESS_TIMEOUT_CNT_reserved0_MASK                0xffff0000
#define BCHP_SID_PROCESS_TIMEOUT_CNT_reserved0_SHIFT               16

/* SID :: PROCESS_TIMEOUT_CNT :: TimeoutCnt [15:00] */
#define BCHP_SID_PROCESS_TIMEOUT_CNT_TimeoutCnt_MASK               0x0000ffff
#define BCHP_SID_PROCESS_TIMEOUT_CNT_TimeoutCnt_SHIFT              0
#define BCHP_SID_PROCESS_TIMEOUT_CNT_TimeoutCnt_DEFAULT            0x00000000

/***************************************************************************
 *INT_IS_EDGE - Interrupt Type (Edge/Level)
 ***************************************************************************/
/* SID :: INT_IS_EDGE :: reserved0 [31:06] */
#define BCHP_SID_INT_IS_EDGE_reserved0_MASK                        0xffffffc0
#define BCHP_SID_INT_IS_EDGE_reserved0_SHIFT                       6

/* SID :: INT_IS_EDGE :: Error [05:05] */
#define BCHP_SID_INT_IS_EDGE_Error_MASK                            0x00000020
#define BCHP_SID_INT_IS_EDGE_Error_SHIFT                           5

/* SID :: INT_IS_EDGE :: PDmaInAct [04:04] */
#define BCHP_SID_INT_IS_EDGE_PDmaInAct_MASK                        0x00000010
#define BCHP_SID_INT_IS_EDGE_PDmaInAct_SHIFT                       4

/* SID :: INT_IS_EDGE :: FEndInAct [03:03] */
#define BCHP_SID_INT_IS_EDGE_FEndInAct_MASK                        0x00000008
#define BCHP_SID_INT_IS_EDGE_FEndInAct_SHIFT                       3

/* SID :: INT_IS_EDGE :: BEndInAct [02:02] */
#define BCHP_SID_INT_IS_EDGE_BEndInAct_MASK                        0x00000004
#define BCHP_SID_INT_IS_EDGE_BEndInAct_SHIFT                       2

/* SID :: INT_IS_EDGE :: BShiftInAct [01:01] */
#define BCHP_SID_INT_IS_EDGE_BShiftInAct_MASK                      0x00000002
#define BCHP_SID_INT_IS_EDGE_BShiftInAct_SHIFT                     1

/* SID :: INT_IS_EDGE :: InDmaDone [00:00] */
#define BCHP_SID_INT_IS_EDGE_InDmaDone_MASK                        0x00000001
#define BCHP_SID_INT_IS_EDGE_InDmaDone_SHIFT                       0

/***************************************************************************
 *INT_CLR - Interrupt Clear
 ***************************************************************************/
/* SID :: INT_CLR :: reserved0 [31:06] */
#define BCHP_SID_INT_CLR_reserved0_MASK                            0xffffffc0
#define BCHP_SID_INT_CLR_reserved0_SHIFT                           6

/* SID :: INT_CLR :: Error [05:05] */
#define BCHP_SID_INT_CLR_Error_MASK                                0x00000020
#define BCHP_SID_INT_CLR_Error_SHIFT                               5

/* SID :: INT_CLR :: PDmaInAct [04:04] */
#define BCHP_SID_INT_CLR_PDmaInAct_MASK                            0x00000010
#define BCHP_SID_INT_CLR_PDmaInAct_SHIFT                           4

/* SID :: INT_CLR :: FEndInAct [03:03] */
#define BCHP_SID_INT_CLR_FEndInAct_MASK                            0x00000008
#define BCHP_SID_INT_CLR_FEndInAct_SHIFT                           3

/* SID :: INT_CLR :: BEndInAct [02:02] */
#define BCHP_SID_INT_CLR_BEndInAct_MASK                            0x00000004
#define BCHP_SID_INT_CLR_BEndInAct_SHIFT                           2

/* SID :: INT_CLR :: BShiftInAct [01:01] */
#define BCHP_SID_INT_CLR_BShiftInAct_MASK                          0x00000002
#define BCHP_SID_INT_CLR_BShiftInAct_SHIFT                         1

/* SID :: INT_CLR :: InDmaDone [00:00] */
#define BCHP_SID_INT_CLR_InDmaDone_MASK                            0x00000001
#define BCHP_SID_INT_CLR_InDmaDone_SHIFT                           0

/***************************************************************************
 *FIFO_DATA - Image Fifo Data
 ***************************************************************************/
/* union - case JPEG [31:00] */
/* SID :: FIFO_DATA :: JPEG :: Run0 [31:28] */
#define BCHP_SID_FIFO_DATA_JPEG_Run0_MASK                          0xf0000000
#define BCHP_SID_FIFO_DATA_JPEG_Run0_SHIFT                         28
#define BCHP_SID_FIFO_DATA_JPEG_Run0_DEFAULT                       0x00000000

/* SID :: FIFO_DATA :: JPEG :: Level0 [27:16] */
#define BCHP_SID_FIFO_DATA_JPEG_Level0_MASK                        0x0fff0000
#define BCHP_SID_FIFO_DATA_JPEG_Level0_SHIFT                       16
#define BCHP_SID_FIFO_DATA_JPEG_Level0_DEFAULT                     0x00000000

/* SID :: FIFO_DATA :: JPEG :: Run1 [15:12] */
#define BCHP_SID_FIFO_DATA_JPEG_Run1_MASK                          0x0000f000
#define BCHP_SID_FIFO_DATA_JPEG_Run1_SHIFT                         12
#define BCHP_SID_FIFO_DATA_JPEG_Run1_DEFAULT                       0x00000000

/* SID :: FIFO_DATA :: JPEG :: Level1 [11:00] */
#define BCHP_SID_FIFO_DATA_JPEG_Level1_MASK                        0x00000fff
#define BCHP_SID_FIFO_DATA_JPEG_Level1_SHIFT                       0
#define BCHP_SID_FIFO_DATA_JPEG_Level1_DEFAULT                     0x00000000

/* union - case PNG [31:00] */
/* SID :: FIFO_DATA :: PNG :: reserved0 [31:25] */
#define BCHP_SID_FIFO_DATA_PNG_reserved0_MASK                      0xfe000000
#define BCHP_SID_FIFO_DATA_PNG_reserved0_SHIFT                     25

/* SID :: FIFO_DATA :: PNG :: Len [24:16] */
#define BCHP_SID_FIFO_DATA_PNG_Len_MASK                            0x01ff0000
#define BCHP_SID_FIFO_DATA_PNG_Len_SHIFT                           16
#define BCHP_SID_FIFO_DATA_PNG_Len_DEFAULT                         0x00000000

/* SID :: FIFO_DATA :: PNG :: IsDist [15:15] */
#define BCHP_SID_FIFO_DATA_PNG_IsDist_MASK                         0x00008000
#define BCHP_SID_FIFO_DATA_PNG_IsDist_SHIFT                        15
#define BCHP_SID_FIFO_DATA_PNG_IsDist_DEFAULT                      0x00000000

/* SID :: FIFO_DATA :: PNG :: Distance_Literal [14:00] */
#define BCHP_SID_FIFO_DATA_PNG_Distance_Literal_MASK               0x00007fff
#define BCHP_SID_FIFO_DATA_PNG_Distance_Literal_SHIFT              0
#define BCHP_SID_FIFO_DATA_PNG_Distance_Literal_DEFAULT            0x00000000

/* union - case GIF [31:00] */
/* SID :: FIFO_DATA :: GIF :: reserved0 [31:12] */
#define BCHP_SID_FIFO_DATA_GIF_reserved0_MASK                      0xfffff000
#define BCHP_SID_FIFO_DATA_GIF_reserved0_SHIFT                     12

/* SID :: FIFO_DATA :: GIF :: Code [11:00] */
#define BCHP_SID_FIFO_DATA_GIF_Code_MASK                           0x00000fff
#define BCHP_SID_FIFO_DATA_GIF_Code_SHIFT                          0
#define BCHP_SID_FIFO_DATA_GIF_Code_DEFAULT                        0x00000000

/* union - case RLE [31:00] */
/* SID :: FIFO_DATA :: RLE :: reserved0 [31:20] */
#define BCHP_SID_FIFO_DATA_RLE_reserved0_MASK                      0xfff00000
#define BCHP_SID_FIFO_DATA_RLE_reserved0_SHIFT                     20

/* SID :: FIFO_DATA :: RLE :: Repeat [19:08] */
#define BCHP_SID_FIFO_DATA_RLE_Repeat_MASK                         0x000fff00
#define BCHP_SID_FIFO_DATA_RLE_Repeat_SHIFT                        8
#define BCHP_SID_FIFO_DATA_RLE_Repeat_DEFAULT                      0x00000000

/* SID :: FIFO_DATA :: RLE :: Pixel [07:00] */
#define BCHP_SID_FIFO_DATA_RLE_Pixel_MASK                          0x000000ff
#define BCHP_SID_FIFO_DATA_RLE_Pixel_SHIFT                         0
#define BCHP_SID_FIFO_DATA_RLE_Pixel_DEFAULT                       0x00000000

/***************************************************************************
 *DEBUG_IDCT - IDCT Debug Register
 ***************************************************************************/
/* SID :: DEBUG_IDCT :: DbgBits [31:00] */
#define BCHP_SID_DEBUG_IDCT_DbgBits_MASK                           0xffffffff
#define BCHP_SID_DEBUG_IDCT_DbgBits_SHIFT                          0

/***************************************************************************
 *DEBUG_RAS_HW - RAS Debug
 ***************************************************************************/
/* SID :: DEBUG_RAS_HW :: DbgBits [31:00] */
#define BCHP_SID_DEBUG_RAS_HW_DbgBits_MASK                         0xffffffff
#define BCHP_SID_DEBUG_RAS_HW_DbgBits_SHIFT                        0

/***************************************************************************
 *DEBUG_RAS_FLAG - Ras Flag Debug
 ***************************************************************************/
/* SID :: DEBUG_RAS_FLAG :: DbgBits [31:00] */
#define BCHP_SID_DEBUG_RAS_FLAG_DbgBits_MASK                       0xffffffff
#define BCHP_SID_DEBUG_RAS_FLAG_DbgBits_SHIFT                      0

/***************************************************************************
 *DEBUG_HORCOL - HorCol Debug
 ***************************************************************************/
/* SID :: DEBUG_HORCOL :: DbgBits [31:00] */
#define BCHP_SID_DEBUG_HORCOL_DbgBits_MASK                         0xffffffff
#define BCHP_SID_DEBUG_HORCOL_DbgBits_SHIFT                        0

/***************************************************************************
 *DEBUG_PNG - PNG Debug
 ***************************************************************************/
/* SID :: DEBUG_PNG :: DbgBits [31:00] */
#define BCHP_SID_DEBUG_PNG_DbgBits_MASK                            0xffffffff
#define BCHP_SID_DEBUG_PNG_DbgBits_SHIFT                           0

/***************************************************************************
 *DEBUG_RLE - RLE Debug
 ***************************************************************************/
/* SID :: DEBUG_RLE :: DbgBits [31:00] */
#define BCHP_SID_DEBUG_RLE_DbgBits_MASK                            0xffffffff
#define BCHP_SID_DEBUG_RLE_DbgBits_SHIFT                           0

/***************************************************************************
 *DEBUG_GIF - GIF Debug
 ***************************************************************************/
/* SID :: DEBUG_GIF :: DbgBits [31:00] */
#define BCHP_SID_DEBUG_GIF_DbgBits_MASK                            0xffffffff
#define BCHP_SID_DEBUG_GIF_DbgBits_SHIFT                           0

/***************************************************************************
 *DEBUG_SYM_DCD - Symb Dcd Debug
 ***************************************************************************/
/* SID :: DEBUG_SYM_DCD :: DbgBits [31:00] */
#define BCHP_SID_DEBUG_SYM_DCD_DbgBits_MASK                        0xffffffff
#define BCHP_SID_DEBUG_SYM_DCD_DbgBits_SHIFT                       0

#endif /* #ifndef BCHP_SID_H__ */

/* End of File */
