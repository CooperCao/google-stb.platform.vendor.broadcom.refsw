/***************************************************************************
 *     Copyright (c) 1999-2012, Broadcom Corporation
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
 * Date:           Generated on         Tue Jan 17 13:26:48 2012
 *                 MD5 Checksum         d41d8cd98f00b204e9800998ecf8427e
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

#ifndef BCHP_VFD_1_H__
#define BCHP_VFD_1_H__

/***************************************************************************
 *VFD_1 - Video Feeder 1 Registers
 ***************************************************************************/
#define BCHP_VFD_1_REVISION_ID                   0x00601200 /* MPEG/Video Feeder Revision Register */
#define BCHP_VFD_1_FEEDER_CNTL                   0x00601204 /* MPEG/Video Feeder Control Register */
#define BCHP_VFD_1_FIXED_COLOUR                  0x00601208 /* MPEG/Video Feeder Fixed Colour Value Register */
#define BCHP_VFD_1_LAC_CNTL                      0x0060120c /* MPEG/Video Feeder LAC Control Register */
#define BCHP_VFD_1_STRIDE                        0x00601210 /* MPEG/Video Feeder Stride Register */
#define BCHP_VFD_1_DISP_HSIZE                    0x00601214 /* MPEG/Video Feeder Horizontal Display Size Register */
#define BCHP_VFD_1_DISP_VSIZE                    0x00601218 /* MPEG/Video Feeder Vertical Display Size Register */
#define BCHP_VFD_1_PICTURE0_LINE_ADDR_0          0x0060121c /* MPEG/Video Feeder Line Address0 Register */
#define BCHP_VFD_1_US_422_TO_444_DERING          0x00601220 /* MPEG/Video Feeder 4:2:2 to 4:4:4 Conversion Register */
#define BCHP_VFD_1_DATA_MODE                     0x00601224 /* MPEG/Video Feeder Data Mode Register */
#define BCHP_VFD_1_PIC_OFFSET                    0x00601228 /* MPEG/Video Feeder Picture Offset Register */
#define BCHP_VFD_1_BYTE_ORDER                    0x0060122c /* 8bit YCbCr PACKED_NEW Format byte order control */
#define BCHP_VFD_1_PIC_FEED_CMD                  0x00601230 /* MPEG/Video Feeder Picture Feed Command Register */
#define BCHP_VFD_1_FEED_STATUS                   0x00601258 /* MPEG/Video Feeder Feed Status Register */
#define BCHP_VFD_1_PICTURE0_LAC_LINE_ADDR_0      0x0060125c /* MPEG/Video Feeder Line Address Computer Line Address0 Register */
#define BCHP_VFD_1_LAC_LINE_FEED_CNTL            0x00601268 /* MPEG/Video Feeder Line Address Computer Line Feed Control Register */
#define BCHP_VFD_1_FEEDER_TIMEOUT_REPEAT_PIC_CNTL 0x0060126c /* MPEG/Video Feeder Timeout and Repeat Picture Control Register */
#define BCHP_VFD_1_BVB_RX_STALL_TIMEOUT_CNTL     0x00601270 /* MPEG/Video Feeder BVB Receiver Stall Timeout Control Register */
#define BCHP_VFD_1_FEEDER_ERROR_INTERRUPT_STATUS 0x00601274 /* MPEG/Video Feeder Error Interrupt Status Register */
#define BCHP_VFD_1_FEEDER_ERROR_INTERRUPT_STATUS_CLR 0x00601278 /* MPEG/Video Feeder Error Interrupt Status Clear Register */
#define BCHP_VFD_1_FEEDER_BVB_STATUS             0x0060127c /* MPEG/Video Feeder BVB Status Register */
#define BCHP_VFD_1_FEEDER_BVB_STATUS_CLR         0x00601280 /* MPEG/Video Feeder BVB Status Clear Register */
#define BCHP_VFD_1_TEST_MODE_CNTL                0x00601284 /* MPEG/Video Feeder Test Mode Control Register */
#define BCHP_VFD_1_BVB_SAMPLE_DATA               0x00601288 /* MPEG/Video Feeder BVB Output Sample Data Register */
#define BCHP_VFD_1_TEST_PORT_CNTL                0x0060128c /* MPEG/Video Feeder Test port Control Register */
#define BCHP_VFD_1_TEST_PORT_DATA                0x00601290 /* MPEG/Video Feeder Test port Data Register */
#define BCHP_VFD_1_CFG_STATUS                    0x00601294 /* MPEG/Video Feeder Hardware Configuration Register */
#define BCHP_VFD_1_PICTURE0_LINE_ADDR_0_R        0x00601350 /* MPEG/Video Feeder Line Address0 Register */
#define BCHP_VFD_1_SCRATCH_REGISTER_1            0x006013fc /* MPEG/Video Feeder Scratch 1 Register */
#define BCHP_VFD_1_SCRATCH_REGISTER_0            0x006013f8 /* Video Feeder Scratch 0 Register */

#endif /* #ifndef BCHP_VFD_1_H__ */

/* End of File */
