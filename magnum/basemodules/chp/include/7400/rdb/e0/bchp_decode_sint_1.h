/***************************************************************************
 *     Copyright (c) 1999-2008, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on         Wed Jan  9 09:01:49 2008
 *                 MD5 Checksum         847dc12a9d71c4c68a648bbf19a883e3
 *
 * Compiled with:  RDB Utility          combo_header.pl
 *                 RDB Parser           3.0
 *                 unknown              unknown
 *                 Perl Interpreter     5.008008
 *                 Operating System     linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_DECODE_SINT_1_H__
#define BCHP_DECODE_SINT_1_H__

/***************************************************************************
 *DECODE_SINT_1
 ***************************************************************************/
#define BCHP_DECODE_SINT_1_REG_SINT_DMA_ADDR     0x00e00c00 /* REG_SINT_DMA_ADDR */
#define BCHP_DECODE_SINT_1_REG_SINT_DMA_LEN      0x00e00c04 /* REG_SINT_DMA_LEN */
#define BCHP_DECODE_SINT_1_REG_SINT_DMA_BASE     0x00e00c08 /* REG_SINT_DMA_BASE */
#define BCHP_DECODE_SINT_1_REG_SINT_DMA_END      0x00e00c0c /* REG_SINT_DMA_END */
#define BCHP_DECODE_SINT_1_REG_SINT_STRM_POS     0x00e00c10 /* REG_SINT_STRM_POS */
#define BCHP_DECODE_SINT_1_REG_SINT_STRM_STAT    0x00e00c14 /* REG_SINT_STRM_STAT */
#define BCHP_DECODE_SINT_1_REG_SINT_IENA         0x00e00c18 /* REG_SINT_IENA */
#define BCHP_DECODE_SINT_1_REG_SINT_STRM_BITS    0x00e00c1c /* REG_SINT_STRM_BITS */
#define BCHP_DECODE_SINT_1_REG_SINT_GET_SYMB     0x00e00c20 /* REG_SINT_GET_SYMB */
#define BCHP_DECODE_SINT_1_REG_SINT_MPEG_DC      0x00e00c24 /* REG_SINT_MPEG_DC */
#define BCHP_DECODE_SINT_1_REG_SINT_DO_RESID     0x00e00c28 /* REG_SINT_DO_RESID */
#define BCHP_DECODE_SINT_1_REG_SINT_XNZERO       0x00e00c2c /* REG_SINT_XNZERO */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_MBTYPE   0x00e00c30 /* REG_SINT_VEC_MBTYPE */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_RESID    0x00e00c34 /* REG_SINT_VEC_RESID */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_DMODE    0x00e00c38 /* REG_SINT_VEC_DMODE */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_TOP_LD   0x00e00c3c /* REG_SINT_VEC_TOP_LD */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_DO_CONST 0x00e00c40 /* REG_SINT_VEC_DO_CONST */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_MVDIFF   0x00e00c44 /* Deblocking Motion Vector Difference */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_REFIDX   0x00e00c48 /* REG_SINT_VEC_REFIDX */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_TOPREF   0x00e00c4c /* REG_SINT_VEC_TOPREF */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_TOPTOPREF 0x00e00c50 /* REG_SINT_VEC_TOPTOPREF */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_COL_TYPE 0x00e00c54 /* REG_SINT_VEC_COL_TYPE */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_COL_REFID 0x00e00c58 /* REG_SINT_VEC_COL_REFID */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_TOPPIC   0x00e00c5c /* REG_SINT_VEC_TOPPIC */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_VC1_INFO 0x00e00c60 /* REG_SINT_VEC_VC1_INFO */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_REFPIC   0x00e00c64 /* REG_SINT_VEC_REFPIC - H.264 only */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_COUNT    0x00e00c68 /* REG_SINT_VEC_COUNT */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_MVD_FIFO 0x00e00c6c /* REG_SINT_VEC_MVD_FIFO */
#define BCHP_DECODE_SINT_1_REG_SINT_DIVX_TABSEL  0x00e00c70 /* REG_SINT_DIVX_TABSEL */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_REGEND   0x00e00c7c /* REG_SINT_VEC_REGEND */
#define BCHP_DECODE_SINT_1_REG_SINT_CTL          0x00e00c80 /* REG_SINT_CTL */
#define BCHP_DECODE_SINT_1_REG_SINT_VLC_TOPCTX   0x00e00c84 /* REG_SINT_VLC_TOPCTX */
#define BCHP_DECODE_SINT_1_REG_SINT_SLICE_ID     0x00e00c88 /* REG_SINT_SLICE_ID */
#define BCHP_DECODE_SINT_1_REG_SINT_QP           0x00e00c8c /* REG_SINT_QP */
#define BCHP_DECODE_SINT_1_REG_SINT_TOP_BASE_ADDR 0x00e00c90 /* REG_SINT_TOP_BASE_ADDR */
#define BCHP_DECODE_SINT_1_REG_SINT_DIRCTX_WR_ADDR 0x00e00c94 /* REG_SINT_DIRCTX_WR_ADDR */
#define BCHP_DECODE_SINT_1_REG_SINT_TOPCTX_DATA  0x00e00c98 /* REG_SINT_TOPCTX_DATA */
#define BCHP_DECODE_SINT_1_REG_SINT_XFER_SYMB    0x00e00c9c /* REG_SINT_XFER_SYMB */
#define BCHP_DECODE_SINT_1_REG_SINT_SMODE_BASE   0x00e00ca0 /* REG_SINT_SMODE_BASE */
#define BCHP_DECODE_SINT_1_REG_SINT_SMODE_LEFT   0x00e00ca4 /* REG_SINT_SMODE_LEFT */
#define BCHP_DECODE_SINT_1_REG_SINT_SMODE_TOP    0x00e00ca8 /* REG_SINT_SMODE_TOP */
#define BCHP_DECODE_SINT_1_REG_SINT_SMODE_END    0x00e00cac /* REG_SINT_SMODE_END */
#define BCHP_DECODE_SINT_1_REG_SINT_CTX_INIT     0x00e00cb0 /* REG_SINT_CTX_INIT */
#define BCHP_DECODE_SINT_1_REG_SINT_TOP_CTX      0x00e00cb4 /* REG_SINT_TOP_CTX */
#define BCHP_DECODE_SINT_1_REG_SINT_VC1_TABSEL   0x00e00cb8 /* REG_SINT_VC1_TABSEL */
#define BCHP_DECODE_SINT_1_REG_SINT_CNST_INTRA   0x00e00cbc /* REG_SINT_CNST_INTRA */
#define BCHP_DECODE_SINT_1_REG_SINT_OPIC_MEM_BASE 0x00e00cc0 /* Outpic Lookup */
#define BCHP_DECODE_SINT_1_REG_SINT_OPIC_MEM_END 0x00e00cfc /* REG_SINT_OPIC_MEM_END */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_MEM_BASE 0x00e00d00 /* Vector Memory */
#define BCHP_DECODE_SINT_1_REG_SINT_VEC_MEM_END  0x00e00dfc /* REG_SINT_VEC_MEM_END */

#endif /* #ifndef BCHP_DECODE_SINT_1_H__ */

/* End of File */
