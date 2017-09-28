/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/



#ifndef BCHP_MM_M2MC0_H__
#define BCHP_MM_M2MC0_H__

/***************************************************************************
 *MM_M2MC0 - Mipmap M2MC0 Registers
 ***************************************************************************/
#define BCHP_MM_M2MC0_REVISION                   0x002e8000 /* [RO][32] M2MC Revision register */
#define BCHP_MM_M2MC0_BLIT_GO                    0x002e8004 /* [WO][32] Blit GO bit */
#define BCHP_MM_M2MC0_SCRATCH_NOT_LIST           0x002e8008 /* [RW][32] M2MC Scratch register (not included in DMA list structure) */
#define BCHP_MM_M2MC0_LIST_CTRL                  0x002e800c /* [RW][32] RDMA Linked List Control Register */
#define BCHP_MM_M2MC0_LIST_STATUS                0x002e8010 /* [RO][32] RDMA Linked List Status Register */
#define BCHP_MM_M2MC0_LIST_FIRST_PKT_ADDR        0x002e8018 /* [RW][64] RDMA Linked List First Packet Address Register */
#define BCHP_MM_M2MC0_LIST_CURR_PKT_ADDR         0x002e8020 /* [RO][64] RDMA Linked List Current Packet Address Register */
#define BCHP_MM_M2MC0_BLIT_STATUS                0x002e8028 /* [RO][32] Blit status */
#define BCHP_MM_M2MC0_BLIT_SRC_ADDRESS           0x002e8030 /* [RO][64] Blit status source feeder current address */
#define BCHP_MM_M2MC0_BLIT_SRC_S1_ADDRESS        0x002e8038 /* [RO][64] Blit status source feeder current address */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_ADDRESS        0x002e8048 /* [RO][64] Blit status output feeder current address */
#define BCHP_MM_M2MC0_BLIT_MEM_HI                0x002e8050 /* [RW][64] Blit memory protection address high */
#define BCHP_MM_M2MC0_BLIT_MEM_LO                0x002e8058 /* [RW][64] Blit memory protection address low */
#define BCHP_MM_M2MC0_BLIT_EXT_MEM_HI            0x002e8060 /* [RW][64] Blit memory protection address high for Extension area */
#define BCHP_MM_M2MC0_BLIT_EXT_MEM_LO            0x002e8068 /* [RW][64] Blit memory protection address low for Extension area */
#define BCHP_MM_M2MC0_BLIT_MIPMAP_ADDRESS        0x002e8070 /* [RO][64] Blit status mipmap current address */
#define BCHP_MM_M2MC0_BLIT_SRC_S2_ADDRESS        0x002e8078 /* [RO][64] Blit status source feeder S2 current address */
#define BCHP_MM_M2MC0_DITHER_CONTROL_0           0x002e8080 /* [RW][32] Source Feeder 10 to 8 bit conversion Dither Control for ch0, ch1 and ch2 */
#define BCHP_MM_M2MC0_DITHER_CONTROL_1           0x002e8084 /* [RW][32] Source Feeder 10 to 8 bit conversion Dither Control for ch3 */
#define BCHP_MM_M2MC0_DITHER_LFSR                0x002e8088 /* [RW][32] Source Feeder 10 to 8 bit conversion Dither LFSR Control */
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT           0x002e808c /* [RW][32] Source Feeder 10 to 8 bit conversion Dither LFSR Init value and control */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_REVISION     0x002e8090 /* [RO][32] BSTC Compression Revision ID register */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL      0x002e8094 /* [RW][32] BSTC Compression Control */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL 0x002e8098 /* [RW][32] BSTC Compression Checksum Control */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_STATUS 0x002e809c /* [RO][32] BSTC Compression Checksum Status */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CLEAR 0x002e80a0 /* [RW][32] BSTC Compression Checksum Clear */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CONTROL 0x002e80a4 /* [RW][32] BSTC Compression Debug Control */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_STATUS 0x002e80a8 /* [RO][32] BSTC Compression Debug Status */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CLEAR  0x002e80ac /* [RW][32] BSTC Compression Debug Clear */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_REVISION   0x002e80b0 /* [RO][32] BSTC Decompression Revision ID register */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_STATUS_MUX 0x002e80b4 /* [RW][32] BSTC Decompression Status Mux */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL 0x002e80b8 /* [RW][32] BSTC Decompression Checksum Control */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_STATUS 0x002e80bc /* [RO][32] BSTC Decompression Checksum Status */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CLEAR 0x002e80c0 /* [RW][32] BSTC Decompression Checksum Status */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CONTROL 0x002e80c4 /* [RW][32] BSTC Decompression Debug Control */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_STATUS 0x002e80c8 /* [RO][32] BSTC Decompression Debug Status */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CLEAR 0x002e80cc /* [RW][32] BSTC Decompression Debug Clear */
#define BCHP_MM_M2MC0_DRAM_MAP                   0x002e80d0 /* [RW][32] DRAM MAP Version */
#define BCHP_MM_M2MC0_DEBUG_STATUS               0x002e80d4 /* [RO][32] M2MC Debug Status register */
#define BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG      0x002e80d8 /* [RW][32] Mipmap M2MC System Config */
#define BCHP_MM_M2MC0_MM_M2MC_MIPMAP_STATUS      0x002e80dc /* [RO][32] Mipmap M2MC MipMap Status */
#define BCHP_MM_M2MC0_SCRATCH_LIST               0x002e8100 /* [RW][32] M2MC Scratch register (Included in DMA list structure) */
#define BCHP_MM_M2MC0_SRC_FEEDER_ENABLE          0x002e8104 /* [RW][32] Source plane enable */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_0         0x002e8108 /* [RW][64] Source surface 0 address */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_0_BOT_FLD 0x002e8110 /* [RW][64] Source surface 0, field 1  address */
#define BCHP_MM_M2MC0_SRC_SURFACE_STRIDE_0       0x002e8118 /* [RW][32] Source surface 0 STRIDE */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_1         0x002e8120 /* [RW][64] Source surface 1 address */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_1_BOT_FLD 0x002e8128 /* [RW][64] Source surface 1, field 1 address */
#define BCHP_MM_M2MC0_SRC_SURFACE_STRIDE_1       0x002e8130 /* [RW][32] Source surface 1 STRIDE */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1 0x002e8134 /* [RW][32] Source pixel format 1 for surface 0 */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2 0x002e8138 /* [RW][32] Source pixel format 2 for surface 0 */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3 0x002e813c /* [RW][32] Source pixel format 3 for surface 0 */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1 0x002e8140 /* [RW][32] Source pixel format 1 for surface 1 */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2 0x002e8144 /* [RW][32] Source pixel format 2 for surface 1 */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3 0x002e8148 /* [RW][32] Source pixel format 3 for surface 1 */
#define BCHP_MM_M2MC0_SRC_W_ALPHA                0x002e814c /* [RW][32] Source Alpha W format */
#define BCHP_MM_M2MC0_SRC_CONSTANT_COLOR         0x002e8150 /* [RW][32] Source constant color */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_2         0x002e8158 /* [RW][64] Source surface 2 address */
#define BCHP_MM_M2MC0_OUTPUT_FEEDER_ENABLE       0x002e8198 /* [RW][32] Output plane enable */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_ADDR_0      0x002e81a0 /* [RW][64] Output surface 0 address */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_STRIDE_0    0x002e81a8 /* [RW][32] Output surface 0 STRIDE */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_ADDR_1      0x002e81b0 /* [RW][64] Output surface 1 address */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_STRIDE_1    0x002e81b8 /* [RW][32] Output surface 1 STRIDE */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1 0x002e81bc /* [RW][32] Output pixel format 1 */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2 0x002e81c0 /* [RW][32] Output pixel format 2 */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3 0x002e81c4 /* [RW][32] Output pixel format 3 */
#define BCHP_MM_M2MC0_BLIT_HEADER                0x002e81c8 /* [RW][32] Blit header and control */
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_0        0x002e81cc /* [RW][32] Top left pixel coordinate in the Source surface 0. */
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_0            0x002e81d0 /* [RW][32] Pixel width and height of operation in Source surface 0. */
#define BCHP_MM_M2MC0_BLIT_SRC_UIF_FULL_HEIGHT   0x002e81d4 /* [RW][32] Source UIF Full Vertical Size */
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_1        0x002e81d8 /* [RW][32] Top left pixel coordinate in the Source surface 1. */
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_1            0x002e81dc /* [RW][32] Pixel width and height of operation in Source surface 1. */
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0 0x002e81e0 /* [RW][32] width and height of stripe for surface 0 for the decoded frame image format. */
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1 0x002e81e4 /* [RW][32] width and height of stripe for surface 1 for the decoded frame image format. */
#define BCHP_MM_M2MC0_SCRATCH_LIST_1             0x002e81e8 /* [RW][32] M2MC Scratch register (Included in DMA list structure) */
#define BCHP_MM_M2MC0_SCRATCH_LIST_2             0x002e81ec /* [RW][32] M2MC Scratch register (Included in DMA list structure) */
#define BCHP_MM_M2MC0_SCRATCH_LIST_3             0x002e81f0 /* [RW][32] M2MC Scratch register (Included in DMA list structure) */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_TOP_LEFT       0x002e81f4 /* [RW][32] Top left pixel coordinate in the Output */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_SIZE           0x002e81f8 /* [RW][32] Pixel width and height of operation in the Output */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_UIF_FULL_HEIGHT 0x002e81fc /* [RW][32] Output UIF Full Vertical Size */
#define BCHP_MM_M2MC0_BLIT_INPUT_STRIPE_WIDTH_0  0x002e8200 /* [RW][32] Pixel width of input stripe when striping for surface 0 */
#define BCHP_MM_M2MC0_BLIT_INPUT_STRIPE_WIDTH_1  0x002e8204 /* [RW][32] Pixel width of input stripe when striping for surface 1 */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_STRIPE_WIDTH   0x002e8208 /* [RW][32] Pixel width of output stripe when striping */
#define BCHP_MM_M2MC0_BLIT_STRIPE_OVERLAP_0      0x002e820c /* [RW][32] Pixel width of stripe overlap when striping for surface 0. */
#define BCHP_MM_M2MC0_BLIT_STRIPE_OVERLAP_1      0x002e8210 /* [RW][32] Pixel width of stripe overlap when striping for surface 1. */
#define BCHP_MM_M2MC0_BLIT_CTRL                  0x002e8214 /* [RW][32] Blit control */
#define BCHP_MM_M2MC0_SCALER_CTRL                0x002e8218 /* [RW][32] Scaler control */
#define BCHP_MM_M2MC0_HORIZ_AVERAGER_COUNT       0x002e821c /* [RW][32] Horizontal averager control count */
#define BCHP_MM_M2MC0_HORIZ_AVERAGER_COEFF       0x002e8220 /* [RW][32] Horizontal averager control coefficient */
#define BCHP_MM_M2MC0_VERT_AVERAGER_COUNT        0x002e8224 /* [RW][32] Vertical averager control count */
#define BCHP_MM_M2MC0_VERT_AVERAGER_COEFF        0x002e8228 /* [RW][32] Vertical averager control coefficient */
#define BCHP_MM_M2MC0_HORIZ_SCALER_0_INITIAL_PHASE 0x002e822c /* [RW][32] Horizontal scaler 0 initial phase */
#define BCHP_MM_M2MC0_HORIZ_SCALER_0_STEP        0x002e8230 /* [RW][32] Horizontal scaler 0 step */
#define BCHP_MM_M2MC0_HORIZ_SCALER_1_INITIAL_PHASE 0x002e8234 /* [RW][32] Horizontal scaler 1 initial phase */
#define BCHP_MM_M2MC0_HORIZ_SCALER_1_STEP        0x002e8238 /* [RW][32] Horizontal scaler 1 step */
#define BCHP_MM_M2MC0_VERT_SCALER_0_INITIAL_PHASE 0x002e823c /* [RW][32] Vertical scaler 0 initial phase */
#define BCHP_MM_M2MC0_VERT_SCALER_0_STEP         0x002e8240 /* [RW][32] Vertical scaler 0 step */
#define BCHP_MM_M2MC0_VERT_SCALER_1_INITIAL_PHASE 0x002e8244 /* [RW][32] Vertical scaler 1 initial phase */
#define BCHP_MM_M2MC0_VERT_SCALER_1_STEP         0x002e8248 /* [RW][32] Vertical scaler 1 step */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_01 0x002e8298 /* [RW][32] Horizontal Scaler 0 Poly-Phase Filter Phase 0 Coefficients */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_2 0x002e829c /* [RW][32] Horizontal Scaler 0 Poly-Phase Filter Phase 0 Coefficients */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_01 0x002e82a0 /* [RW][32] Horizontal Scaler 0 Poly-Phase Filter Phase 1 Coefficients */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_2 0x002e82a4 /* [RW][32] Horizontal Scaler 0 Poly-Phase Filter Phase 1 Coefficients */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_01 0x002e82a8 /* [RW][32] Horizontal Scaler 1 Poly-Phase Filter Phase 0 Coefficients */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_2 0x002e82ac /* [RW][32] Horizontal Scaler 1 Poly-Phase Filter Phase 0 Coefficients */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_01 0x002e82b0 /* [RW][32] Horizontal Scaler 1 Poly-Phase Filter Phase 1 Coefficients */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_2 0x002e82b4 /* [RW][32] Horizontal Scaler 1 Poly-Phase Filter Phase 1 Coefficients */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_01 0x002e82b8 /* [RW][32] Vertical Scaler 0 Poly-Phase Filter Phase 0 Coefficients */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_2  0x002e82bc /* [RW][32] Vertical Scaler 0 Poly-Phase Filter Phase 0 Coefficients */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_01 0x002e82c0 /* [RW][32] Vertical Scaler 0 Poly-Phase Filter Phase 1 Coefficients */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_2  0x002e82c4 /* [RW][32] Vertical Scaler 0 Poly-Phase Filter Phase 1 Coefficients */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_01 0x002e82c8 /* [RW][32] Vertical Scaler 1 Poly-Phase Filter Phase 0 Coefficients */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_2  0x002e82cc /* [RW][32] Vertical Scaler 1 Poly-Phase Filter Phase 0 Coefficients */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_01 0x002e82d0 /* [RW][32] Vertical Scaler 1 Poly-Phase Filter Phase 1 Coefficients */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_2  0x002e82d4 /* [RW][32] Vertical Scaler 1 Poly-Phase Filter Phase 1 Coefficients */
#define BCHP_MM_M2MC0_SRC_CM_C00_C01             0x002e82d8 /* [RW][32] Color Conversion Matrix Coefficients C00 and C01 */
#define BCHP_MM_M2MC0_SRC_CM_C02_C03             0x002e82dc /* [RW][32] Color Conversion Matrix Coefficients C02 and C03 */
#define BCHP_MM_M2MC0_SRC_CM_C04                 0x002e82e0 /* [RW][32] Color Conversion Matrix Coefficients C04 */
#define BCHP_MM_M2MC0_SRC_CM_C10_C11             0x002e82e4 /* [RW][32] Color Conversion Matrix Coefficients C10 and C11 */
#define BCHP_MM_M2MC0_SRC_CM_C12_C13             0x002e82e8 /* [RW][32] Color Conversion Matrix Coefficients C12 and C13 */
#define BCHP_MM_M2MC0_SRC_CM_C14                 0x002e82ec /* [RW][32] Color Conversion Matrix Coefficients C14 */
#define BCHP_MM_M2MC0_SRC_CM_C20_C21             0x002e82f0 /* [RW][32] Color Conversion Matrix Coefficients C20 and C21 */
#define BCHP_MM_M2MC0_SRC_CM_C22_C23             0x002e82f4 /* [RW][32] Color Conversion Matrix Coefficients C22 and C23 */
#define BCHP_MM_M2MC0_SRC_CM_C24                 0x002e82f8 /* [RW][32] Color Conversion Matrix Coefficients C24 */
#define BCHP_MM_M2MC0_SRC_CM_C30_C31             0x002e82fc /* [RW][32] Color Conversion Matrix Coefficients C30 and C31 */
#define BCHP_MM_M2MC0_SRC_CM_C32_C33             0x002e8300 /* [RW][32] Color Conversion Matrix Coefficients C32 and C33 */
#define BCHP_MM_M2MC0_SRC_CM_C34                 0x002e8304 /* [RW][32] Color Conversion Matrix Coefficients C34 */
#define BCHP_MM_M2MC0_TIMEOUT_COUNTER_CONTROL    0x002e8308 /* [RW][32] M2MC timeout counter control register */
#define BCHP_MM_M2MC0_CLK_GATE_AND_SW_INIT_CONTROL 0x002e830c /* [RW][32] M2MC clock gating and software init control register */
#define BCHP_MM_M2MC0_DEADLOCK_DETECT_CONTROL    0x002e8310 /* [RW][32] Deadlock Detect Control register */
#define BCHP_MM_M2MC0_TIMEOUT_DEBUG              0x002e8314 /* [RO][32] Timeout Debug Register */
#define BCHP_MM_M2MC0_ARBITRATION_DELAY          0x002e8318 /* [RO][32] Arbitration Delay Register */
#define BCHP_MM_M2MC0_RDMA_DEBUG_CLEAR           0x002e831c /* [RW][32] M2MC RDMA Debug Clear */
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS          0x002e8320 /* [RO][32] M2MC RDMA Debug Status */

/***************************************************************************
 *REVISION - M2MC Revision register
 ***************************************************************************/
/* MM_M2MC0 :: REVISION :: reserved0 [31:16] */
#define BCHP_MM_M2MC0_REVISION_reserved0_MASK                      0xffff0000
#define BCHP_MM_M2MC0_REVISION_reserved0_SHIFT                     16

/* MM_M2MC0 :: REVISION :: MAJOR [15:08] */
#define BCHP_MM_M2MC0_REVISION_MAJOR_MASK                          0x0000ff00
#define BCHP_MM_M2MC0_REVISION_MAJOR_SHIFT                         8
#define BCHP_MM_M2MC0_REVISION_MAJOR_DEFAULT                       0x00000001

/* MM_M2MC0 :: REVISION :: MINOR [07:00] */
#define BCHP_MM_M2MC0_REVISION_MINOR_MASK                          0x000000ff
#define BCHP_MM_M2MC0_REVISION_MINOR_SHIFT                         0
#define BCHP_MM_M2MC0_REVISION_MINOR_DEFAULT                       0x00000000

/***************************************************************************
 *BLIT_GO - Blit GO bit
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_GO :: reserved0 [31:01] */
#define BCHP_MM_M2MC0_BLIT_GO_reserved0_MASK                       0xfffffffe
#define BCHP_MM_M2MC0_BLIT_GO_reserved0_SHIFT                      1

/* MM_M2MC0 :: BLIT_GO :: GO [00:00] */
#define BCHP_MM_M2MC0_BLIT_GO_GO_MASK                              0x00000001
#define BCHP_MM_M2MC0_BLIT_GO_GO_SHIFT                             0
#define BCHP_MM_M2MC0_BLIT_GO_GO_DEFAULT                           0x00000000

/***************************************************************************
 *SCRATCH_NOT_LIST - M2MC Scratch register (not included in DMA list structure)
 ***************************************************************************/
/* MM_M2MC0 :: SCRATCH_NOT_LIST :: VALUE [31:00] */
#define BCHP_MM_M2MC0_SCRATCH_NOT_LIST_VALUE_MASK                  0xffffffff
#define BCHP_MM_M2MC0_SCRATCH_NOT_LIST_VALUE_SHIFT                 0
#define BCHP_MM_M2MC0_SCRATCH_NOT_LIST_VALUE_DEFAULT               0x00000000

/***************************************************************************
 *LIST_CTRL - RDMA Linked List Control Register
 ***************************************************************************/
/* MM_M2MC0 :: LIST_CTRL :: reserved0 [31:03] */
#define BCHP_MM_M2MC0_LIST_CTRL_reserved0_MASK                     0xfffffff8
#define BCHP_MM_M2MC0_LIST_CTRL_reserved0_SHIFT                    3

/* MM_M2MC0 :: LIST_CTRL :: WAKE_MODE [02:02] */
#define BCHP_MM_M2MC0_LIST_CTRL_WAKE_MODE_MASK                     0x00000004
#define BCHP_MM_M2MC0_LIST_CTRL_WAKE_MODE_SHIFT                    2
#define BCHP_MM_M2MC0_LIST_CTRL_WAKE_MODE_DEFAULT                  0x00000000
#define BCHP_MM_M2MC0_LIST_CTRL_WAKE_MODE_ResumeFromLast           0
#define BCHP_MM_M2MC0_LIST_CTRL_WAKE_MODE_ResumeFromFirst          1

/* MM_M2MC0 :: LIST_CTRL :: RUN [01:01] */
#define BCHP_MM_M2MC0_LIST_CTRL_RUN_MASK                           0x00000002
#define BCHP_MM_M2MC0_LIST_CTRL_RUN_SHIFT                          1
#define BCHP_MM_M2MC0_LIST_CTRL_RUN_DEFAULT                        0x00000000
#define BCHP_MM_M2MC0_LIST_CTRL_RUN_Stop                           0
#define BCHP_MM_M2MC0_LIST_CTRL_RUN_Run                            1

/* MM_M2MC0 :: LIST_CTRL :: WAKE [00:00] */
#define BCHP_MM_M2MC0_LIST_CTRL_WAKE_MASK                          0x00000001
#define BCHP_MM_M2MC0_LIST_CTRL_WAKE_SHIFT                         0
#define BCHP_MM_M2MC0_LIST_CTRL_WAKE_DEFAULT                       0x00000000
#define BCHP_MM_M2MC0_LIST_CTRL_WAKE_Ack                           0
#define BCHP_MM_M2MC0_LIST_CTRL_WAKE_WakeUp                        1

/***************************************************************************
 *LIST_STATUS - RDMA Linked List Status Register
 ***************************************************************************/
/* MM_M2MC0 :: LIST_STATUS :: reserved0 [31:02] */
#define BCHP_MM_M2MC0_LIST_STATUS_reserved0_MASK                   0xfffffffc
#define BCHP_MM_M2MC0_LIST_STATUS_reserved0_SHIFT                  2

/* MM_M2MC0 :: LIST_STATUS :: FINISHED [01:01] */
#define BCHP_MM_M2MC0_LIST_STATUS_FINISHED_MASK                    0x00000002
#define BCHP_MM_M2MC0_LIST_STATUS_FINISHED_SHIFT                   1
#define BCHP_MM_M2MC0_LIST_STATUS_FINISHED_DEFAULT                 0x00000000
#define BCHP_MM_M2MC0_LIST_STATUS_FINISHED_NotFinished             0
#define BCHP_MM_M2MC0_LIST_STATUS_FINISHED_Finished                1

/* MM_M2MC0 :: LIST_STATUS :: BUSY [00:00] */
#define BCHP_MM_M2MC0_LIST_STATUS_BUSY_MASK                        0x00000001
#define BCHP_MM_M2MC0_LIST_STATUS_BUSY_SHIFT                       0
#define BCHP_MM_M2MC0_LIST_STATUS_BUSY_DEFAULT                     0x00000000
#define BCHP_MM_M2MC0_LIST_STATUS_BUSY_Idle                        0
#define BCHP_MM_M2MC0_LIST_STATUS_BUSY_Busy                        1

/***************************************************************************
 *LIST_FIRST_PKT_ADDR - RDMA Linked List First Packet Address Register
 ***************************************************************************/
/* MM_M2MC0 :: LIST_FIRST_PKT_ADDR :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_LIST_FIRST_PKT_ADDR_reserved0_MASK           BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_LIST_FIRST_PKT_ADDR_reserved0_SHIFT          40

/* MM_M2MC0 :: LIST_FIRST_PKT_ADDR :: FIRST_PKT_ADDR [39:05] */
#define BCHP_MM_M2MC0_LIST_FIRST_PKT_ADDR_FIRST_PKT_ADDR_MASK      BCHP_UINT64_C(0x000000ff, 0xffffffe0)
#define BCHP_MM_M2MC0_LIST_FIRST_PKT_ADDR_FIRST_PKT_ADDR_SHIFT     5
#define BCHP_MM_M2MC0_LIST_FIRST_PKT_ADDR_FIRST_PKT_ADDR_DEFAULT   0

/* MM_M2MC0 :: LIST_FIRST_PKT_ADDR :: reserved1 [04:00] */
#define BCHP_MM_M2MC0_LIST_FIRST_PKT_ADDR_reserved1_MASK           BCHP_UINT64_C(0x00000000, 0x0000001f)
#define BCHP_MM_M2MC0_LIST_FIRST_PKT_ADDR_reserved1_SHIFT          0

/***************************************************************************
 *LIST_CURR_PKT_ADDR - RDMA Linked List Current Packet Address Register
 ***************************************************************************/
/* MM_M2MC0 :: LIST_CURR_PKT_ADDR :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_LIST_CURR_PKT_ADDR_reserved0_MASK            BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_LIST_CURR_PKT_ADDR_reserved0_SHIFT           40

/* MM_M2MC0 :: LIST_CURR_PKT_ADDR :: CURR_PKT_ADDR [39:05] */
#define BCHP_MM_M2MC0_LIST_CURR_PKT_ADDR_CURR_PKT_ADDR_MASK        BCHP_UINT64_C(0x000000ff, 0xffffffe0)
#define BCHP_MM_M2MC0_LIST_CURR_PKT_ADDR_CURR_PKT_ADDR_SHIFT       5
#define BCHP_MM_M2MC0_LIST_CURR_PKT_ADDR_CURR_PKT_ADDR_DEFAULT     0

/* MM_M2MC0 :: LIST_CURR_PKT_ADDR :: reserved1 [04:00] */
#define BCHP_MM_M2MC0_LIST_CURR_PKT_ADDR_reserved1_MASK            BCHP_UINT64_C(0x00000000, 0x0000001f)
#define BCHP_MM_M2MC0_LIST_CURR_PKT_ADDR_reserved1_SHIFT           0

/***************************************************************************
 *BLIT_STATUS - Blit status
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_STATUS :: reserved0 [31:25] */
#define BCHP_MM_M2MC0_BLIT_STATUS_reserved0_MASK                   0xfe000000
#define BCHP_MM_M2MC0_BLIT_STATUS_reserved0_SHIFT                  25

/* MM_M2MC0 :: BLIT_STATUS :: RDY_FOR_SW_INIT [24:24] */
#define BCHP_MM_M2MC0_BLIT_STATUS_RDY_FOR_SW_INIT_MASK             0x01000000
#define BCHP_MM_M2MC0_BLIT_STATUS_RDY_FOR_SW_INIT_SHIFT            24

/* MM_M2MC0 :: BLIT_STATUS :: reserved1 [23:18] */
#define BCHP_MM_M2MC0_BLIT_STATUS_reserved1_MASK                   0x00fc0000
#define BCHP_MM_M2MC0_BLIT_STATUS_reserved1_SHIFT                  18

/* MM_M2MC0 :: BLIT_STATUS :: MIPMAP_MEMORY_PROTECTION [17:17] */
#define BCHP_MM_M2MC0_BLIT_STATUS_MIPMAP_MEMORY_PROTECTION_MASK    0x00020000
#define BCHP_MM_M2MC0_BLIT_STATUS_MIPMAP_MEMORY_PROTECTION_SHIFT   17
#define BCHP_MM_M2MC0_BLIT_STATUS_MIPMAP_MEMORY_PROTECTION_NORMAL  0
#define BCHP_MM_M2MC0_BLIT_STATUS_MIPMAP_MEMORY_PROTECTION_VIOLATION 1

/* MM_M2MC0 :: BLIT_STATUS :: MEMORY_PROTECTION [16:16] */
#define BCHP_MM_M2MC0_BLIT_STATUS_MEMORY_PROTECTION_MASK           0x00010000
#define BCHP_MM_M2MC0_BLIT_STATUS_MEMORY_PROTECTION_SHIFT          16
#define BCHP_MM_M2MC0_BLIT_STATUS_MEMORY_PROTECTION_NORMAL         0
#define BCHP_MM_M2MC0_BLIT_STATUS_MEMORY_PROTECTION_VIOLATION      1

/* MM_M2MC0 :: BLIT_STATUS :: reserved2 [15:09] */
#define BCHP_MM_M2MC0_BLIT_STATUS_reserved2_MASK                   0x0000fe00
#define BCHP_MM_M2MC0_BLIT_STATUS_reserved2_SHIFT                  9

/* MM_M2MC0 :: BLIT_STATUS :: TIME_OUT_ERROR [08:08] */
#define BCHP_MM_M2MC0_BLIT_STATUS_TIME_OUT_ERROR_MASK              0x00000100
#define BCHP_MM_M2MC0_BLIT_STATUS_TIME_OUT_ERROR_SHIFT             8

/* MM_M2MC0 :: BLIT_STATUS :: reserved3 [07:01] */
#define BCHP_MM_M2MC0_BLIT_STATUS_reserved3_MASK                   0x000000fe
#define BCHP_MM_M2MC0_BLIT_STATUS_reserved3_SHIFT                  1

/* MM_M2MC0 :: BLIT_STATUS :: STATUS [00:00] */
#define BCHP_MM_M2MC0_BLIT_STATUS_STATUS_MASK                      0x00000001
#define BCHP_MM_M2MC0_BLIT_STATUS_STATUS_SHIFT                     0
#define BCHP_MM_M2MC0_BLIT_STATUS_STATUS_IDLE                      0
#define BCHP_MM_M2MC0_BLIT_STATUS_STATUS_RUNNING                   1

/***************************************************************************
 *BLIT_SRC_ADDRESS - Blit status source feeder current address
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_SRC_ADDRESS :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_BLIT_SRC_ADDRESS_reserved0_MASK              BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_BLIT_SRC_ADDRESS_reserved0_SHIFT             40

/* MM_M2MC0 :: BLIT_SRC_ADDRESS :: ADDR [39:00] */
#define BCHP_MM_M2MC0_BLIT_SRC_ADDRESS_ADDR_MASK                   BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_BLIT_SRC_ADDRESS_ADDR_SHIFT                  0
#define BCHP_MM_M2MC0_BLIT_SRC_ADDRESS_ADDR_DEFAULT                0

/***************************************************************************
 *BLIT_SRC_S1_ADDRESS - Blit status source feeder current address
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_SRC_S1_ADDRESS :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_BLIT_SRC_S1_ADDRESS_reserved0_MASK           BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_BLIT_SRC_S1_ADDRESS_reserved0_SHIFT          40

/* MM_M2MC0 :: BLIT_SRC_S1_ADDRESS :: ADDR [39:00] */
#define BCHP_MM_M2MC0_BLIT_SRC_S1_ADDRESS_ADDR_MASK                BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_BLIT_SRC_S1_ADDRESS_ADDR_SHIFT               0
#define BCHP_MM_M2MC0_BLIT_SRC_S1_ADDRESS_ADDR_DEFAULT             0

/***************************************************************************
 *BLIT_OUTPUT_ADDRESS - Blit status output feeder current address
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_OUTPUT_ADDRESS :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_ADDRESS_reserved0_MASK           BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_BLIT_OUTPUT_ADDRESS_reserved0_SHIFT          40

/* MM_M2MC0 :: BLIT_OUTPUT_ADDRESS :: ADDR [39:00] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_ADDRESS_ADDR_MASK                BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_BLIT_OUTPUT_ADDRESS_ADDR_SHIFT               0
#define BCHP_MM_M2MC0_BLIT_OUTPUT_ADDRESS_ADDR_DEFAULT             0

/***************************************************************************
 *BLIT_MEM_HI - Blit memory protection address high
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_MEM_HI :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_BLIT_MEM_HI_reserved0_MASK                   BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_BLIT_MEM_HI_reserved0_SHIFT                  40

/* MM_M2MC0 :: BLIT_MEM_HI :: ADDR [39:00] */
#define BCHP_MM_M2MC0_BLIT_MEM_HI_ADDR_MASK                        BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_BLIT_MEM_HI_ADDR_SHIFT                       0
#define BCHP_MM_M2MC0_BLIT_MEM_HI_ADDR_DEFAULT                     1099511627775

/***************************************************************************
 *BLIT_MEM_LO - Blit memory protection address low
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_MEM_LO :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_BLIT_MEM_LO_reserved0_MASK                   BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_BLIT_MEM_LO_reserved0_SHIFT                  40

/* MM_M2MC0 :: BLIT_MEM_LO :: ADDR [39:00] */
#define BCHP_MM_M2MC0_BLIT_MEM_LO_ADDR_MASK                        BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_BLIT_MEM_LO_ADDR_SHIFT                       0
#define BCHP_MM_M2MC0_BLIT_MEM_LO_ADDR_DEFAULT                     0

/***************************************************************************
 *BLIT_EXT_MEM_HI - Blit memory protection address high for Extension area
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_EXT_MEM_HI :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_BLIT_EXT_MEM_HI_reserved0_MASK               BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_BLIT_EXT_MEM_HI_reserved0_SHIFT              40

/* MM_M2MC0 :: BLIT_EXT_MEM_HI :: ADDR [39:00] */
#define BCHP_MM_M2MC0_BLIT_EXT_MEM_HI_ADDR_MASK                    BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_BLIT_EXT_MEM_HI_ADDR_SHIFT                   0
#define BCHP_MM_M2MC0_BLIT_EXT_MEM_HI_ADDR_DEFAULT                 1099511627775

/***************************************************************************
 *BLIT_EXT_MEM_LO - Blit memory protection address low for Extension area
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_EXT_MEM_LO :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_BLIT_EXT_MEM_LO_reserved0_MASK               BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_BLIT_EXT_MEM_LO_reserved0_SHIFT              40

/* MM_M2MC0 :: BLIT_EXT_MEM_LO :: ADDR [39:00] */
#define BCHP_MM_M2MC0_BLIT_EXT_MEM_LO_ADDR_MASK                    BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_BLIT_EXT_MEM_LO_ADDR_SHIFT                   0
#define BCHP_MM_M2MC0_BLIT_EXT_MEM_LO_ADDR_DEFAULT                 0

/***************************************************************************
 *BLIT_MIPMAP_ADDRESS - Blit status mipmap current address
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_MIPMAP_ADDRESS :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_BLIT_MIPMAP_ADDRESS_reserved0_MASK           BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_BLIT_MIPMAP_ADDRESS_reserved0_SHIFT          40

/* MM_M2MC0 :: BLIT_MIPMAP_ADDRESS :: ADDR [39:00] */
#define BCHP_MM_M2MC0_BLIT_MIPMAP_ADDRESS_ADDR_MASK                BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_BLIT_MIPMAP_ADDRESS_ADDR_SHIFT               0
#define BCHP_MM_M2MC0_BLIT_MIPMAP_ADDRESS_ADDR_DEFAULT             0

/***************************************************************************
 *BLIT_SRC_S2_ADDRESS - Blit status source feeder S2 current address
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_SRC_S2_ADDRESS :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_BLIT_SRC_S2_ADDRESS_reserved0_MASK           BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_BLIT_SRC_S2_ADDRESS_reserved0_SHIFT          40

/* MM_M2MC0 :: BLIT_SRC_S2_ADDRESS :: ADDR [39:00] */
#define BCHP_MM_M2MC0_BLIT_SRC_S2_ADDRESS_ADDR_MASK                BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_BLIT_SRC_S2_ADDRESS_ADDR_SHIFT               0
#define BCHP_MM_M2MC0_BLIT_SRC_S2_ADDRESS_ADDR_DEFAULT             0

/***************************************************************************
 *DITHER_CONTROL_0 - Source Feeder 10 to 8 bit conversion Dither Control for ch0, ch1 and ch2
 ***************************************************************************/
/* MM_M2MC0 :: DITHER_CONTROL_0 :: MODE [31:30] */
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_MODE_MASK                   0xc0000000
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_MODE_SHIFT                  30
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_MODE_DEFAULT                0x00000000
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_MODE_ROUNDING               0
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_MODE_TRUNCATE               1
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_MODE_DITHER                 2

/* MM_M2MC0 :: DITHER_CONTROL_0 :: OFFSET_CH2 [29:25] */
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_OFFSET_CH2_MASK             0x3e000000
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_OFFSET_CH2_SHIFT            25
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_OFFSET_CH2_DEFAULT          0x00000001

/* MM_M2MC0 :: DITHER_CONTROL_0 :: SCALE_CH2 [24:20] */
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_SCALE_CH2_MASK              0x01f00000
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_SCALE_CH2_SHIFT             20
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_SCALE_CH2_DEFAULT           0x00000000

/* MM_M2MC0 :: DITHER_CONTROL_0 :: OFFSET_CH1 [19:15] */
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_OFFSET_CH1_MASK             0x000f8000
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_OFFSET_CH1_SHIFT            15
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_OFFSET_CH1_DEFAULT          0x00000001

/* MM_M2MC0 :: DITHER_CONTROL_0 :: SCALE_CH1 [14:10] */
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_SCALE_CH1_MASK              0x00007c00
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_SCALE_CH1_SHIFT             10
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_SCALE_CH1_DEFAULT           0x00000000

/* MM_M2MC0 :: DITHER_CONTROL_0 :: OFFSET_CH0 [09:05] */
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_OFFSET_CH0_MASK             0x000003e0
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_OFFSET_CH0_SHIFT            5
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_OFFSET_CH0_DEFAULT          0x00000001

/* MM_M2MC0 :: DITHER_CONTROL_0 :: SCALE_CH0 [04:00] */
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_SCALE_CH0_MASK              0x0000001f
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_SCALE_CH0_SHIFT             0
#define BCHP_MM_M2MC0_DITHER_CONTROL_0_SCALE_CH0_DEFAULT           0x00000000

/***************************************************************************
 *DITHER_CONTROL_1 - Source Feeder 10 to 8 bit conversion Dither Control for ch3
 ***************************************************************************/
/* MM_M2MC0 :: DITHER_CONTROL_1 :: reserved0 [31:10] */
#define BCHP_MM_M2MC0_DITHER_CONTROL_1_reserved0_MASK              0xfffffc00
#define BCHP_MM_M2MC0_DITHER_CONTROL_1_reserved0_SHIFT             10

/* MM_M2MC0 :: DITHER_CONTROL_1 :: OFFSET_CH3 [09:05] */
#define BCHP_MM_M2MC0_DITHER_CONTROL_1_OFFSET_CH3_MASK             0x000003e0
#define BCHP_MM_M2MC0_DITHER_CONTROL_1_OFFSET_CH3_SHIFT            5
#define BCHP_MM_M2MC0_DITHER_CONTROL_1_OFFSET_CH3_DEFAULT          0x00000001

/* MM_M2MC0 :: DITHER_CONTROL_1 :: SCALE_CH3 [04:00] */
#define BCHP_MM_M2MC0_DITHER_CONTROL_1_SCALE_CH3_MASK              0x0000001f
#define BCHP_MM_M2MC0_DITHER_CONTROL_1_SCALE_CH3_SHIFT             0
#define BCHP_MM_M2MC0_DITHER_CONTROL_1_SCALE_CH3_DEFAULT           0x00000000

/***************************************************************************
 *DITHER_LFSR - Source Feeder 10 to 8 bit conversion Dither LFSR Control
 ***************************************************************************/
/* MM_M2MC0 :: DITHER_LFSR :: reserved0 [31:11] */
#define BCHP_MM_M2MC0_DITHER_LFSR_reserved0_MASK                   0xfffff800
#define BCHP_MM_M2MC0_DITHER_LFSR_reserved0_SHIFT                  11

/* MM_M2MC0 :: DITHER_LFSR :: T2 [10:08] */
#define BCHP_MM_M2MC0_DITHER_LFSR_T2_MASK                          0x00000700
#define BCHP_MM_M2MC0_DITHER_LFSR_T2_SHIFT                         8
#define BCHP_MM_M2MC0_DITHER_LFSR_T2_DEFAULT                       0x00000000
#define BCHP_MM_M2MC0_DITHER_LFSR_T2_ZERO                          0
#define BCHP_MM_M2MC0_DITHER_LFSR_T2_B12                           1
#define BCHP_MM_M2MC0_DITHER_LFSR_T2_B13                           2
#define BCHP_MM_M2MC0_DITHER_LFSR_T2_B14                           3
#define BCHP_MM_M2MC0_DITHER_LFSR_T2_B15                           4
#define BCHP_MM_M2MC0_DITHER_LFSR_T2_B16                           5
#define BCHP_MM_M2MC0_DITHER_LFSR_T2_B17                           6
#define BCHP_MM_M2MC0_DITHER_LFSR_T2_B18                           7

/* MM_M2MC0 :: DITHER_LFSR :: reserved1 [07:07] */
#define BCHP_MM_M2MC0_DITHER_LFSR_reserved1_MASK                   0x00000080
#define BCHP_MM_M2MC0_DITHER_LFSR_reserved1_SHIFT                  7

/* MM_M2MC0 :: DITHER_LFSR :: T1 [06:04] */
#define BCHP_MM_M2MC0_DITHER_LFSR_T1_MASK                          0x00000070
#define BCHP_MM_M2MC0_DITHER_LFSR_T1_SHIFT                         4
#define BCHP_MM_M2MC0_DITHER_LFSR_T1_DEFAULT                       0x00000000
#define BCHP_MM_M2MC0_DITHER_LFSR_T1_ZERO                          0
#define BCHP_MM_M2MC0_DITHER_LFSR_T1_B8                            1
#define BCHP_MM_M2MC0_DITHER_LFSR_T1_B9                            2
#define BCHP_MM_M2MC0_DITHER_LFSR_T1_B10                           3
#define BCHP_MM_M2MC0_DITHER_LFSR_T1_B11                           4
#define BCHP_MM_M2MC0_DITHER_LFSR_T1_B12                           5
#define BCHP_MM_M2MC0_DITHER_LFSR_T1_B13                           6
#define BCHP_MM_M2MC0_DITHER_LFSR_T1_B14                           7

/* MM_M2MC0 :: DITHER_LFSR :: reserved2 [03:03] */
#define BCHP_MM_M2MC0_DITHER_LFSR_reserved2_MASK                   0x00000008
#define BCHP_MM_M2MC0_DITHER_LFSR_reserved2_SHIFT                  3

/* MM_M2MC0 :: DITHER_LFSR :: T0 [02:00] */
#define BCHP_MM_M2MC0_DITHER_LFSR_T0_MASK                          0x00000007
#define BCHP_MM_M2MC0_DITHER_LFSR_T0_SHIFT                         0
#define BCHP_MM_M2MC0_DITHER_LFSR_T0_DEFAULT                       0x00000000
#define BCHP_MM_M2MC0_DITHER_LFSR_T0_B2                            0
#define BCHP_MM_M2MC0_DITHER_LFSR_T0_B3                            1
#define BCHP_MM_M2MC0_DITHER_LFSR_T0_B4                            2
#define BCHP_MM_M2MC0_DITHER_LFSR_T0_B6                            3
#define BCHP_MM_M2MC0_DITHER_LFSR_T0_B7                            4
#define BCHP_MM_M2MC0_DITHER_LFSR_T0_B8                            5
#define BCHP_MM_M2MC0_DITHER_LFSR_T0_B9                            6
#define BCHP_MM_M2MC0_DITHER_LFSR_T0_B10                           7

/***************************************************************************
 *DITHER_LFSR_INIT - Source Feeder 10 to 8 bit conversion Dither LFSR Init value and control
 ***************************************************************************/
/* MM_M2MC0 :: DITHER_LFSR_INIT :: reserved0 [31:22] */
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT_reserved0_MASK              0xffc00000
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT_reserved0_SHIFT             22

/* MM_M2MC0 :: DITHER_LFSR_INIT :: SEQ [21:20] */
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT_SEQ_MASK                    0x00300000
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT_SEQ_SHIFT                   20
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT_SEQ_DEFAULT                 0x00000003
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT_SEQ_ONCE                    0
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT_SEQ_ONCE_PER_SOP            1
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT_SEQ_ONCE_PER_2SOP           2
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT_SEQ_NEVER                   3

/* MM_M2MC0 :: DITHER_LFSR_INIT :: VALUE [19:00] */
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT_VALUE_MASK                  0x000fffff
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT_VALUE_SHIFT                 0
#define BCHP_MM_M2MC0_DITHER_LFSR_INIT_VALUE_DEFAULT               0x00000000

/***************************************************************************
 *BSTC_COMPRESS_REVISION - BSTC Compression Revision ID register
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_COMPRESS_REVISION :: reserved0 [31:16] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_REVISION_reserved0_MASK        0xffff0000
#define BCHP_MM_M2MC0_BSTC_COMPRESS_REVISION_reserved0_SHIFT       16

/* MM_M2MC0 :: BSTC_COMPRESS_REVISION :: MAJOR [15:08] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_REVISION_MAJOR_MASK            0x0000ff00
#define BCHP_MM_M2MC0_BSTC_COMPRESS_REVISION_MAJOR_SHIFT           8
#define BCHP_MM_M2MC0_BSTC_COMPRESS_REVISION_MAJOR_DEFAULT         0x00000000

/* MM_M2MC0 :: BSTC_COMPRESS_REVISION :: MINOR [07:00] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_REVISION_MINOR_MASK            0x000000ff
#define BCHP_MM_M2MC0_BSTC_COMPRESS_REVISION_MINOR_SHIFT           0
#define BCHP_MM_M2MC0_BSTC_COMPRESS_REVISION_MINOR_DEFAULT         0x00000000

/***************************************************************************
 *BSTC_COMPRESS_CONTROL - BSTC Compression Control
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_COMPRESS_CONTROL :: reserved0 [31:20] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_reserved0_MASK         0xfff00000
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_reserved0_SHIFT        20

/* MM_M2MC0 :: BSTC_COMPRESS_CONTROL :: MAX_NUM [19:16] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_MAX_NUM_MASK           0x000f0000
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_MAX_NUM_SHIFT          16
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_MAX_NUM_DEFAULT        0x00000009

/* MM_M2MC0 :: BSTC_COMPRESS_CONTROL :: MAX_THRESH [15:08] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_MAX_THRESH_MASK        0x0000ff00
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_MAX_THRESH_SHIFT       8
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_MAX_THRESH_DEFAULT     0x0000008c

/* MM_M2MC0 :: BSTC_COMPRESS_CONTROL :: reserved1 [07:01] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_reserved1_MASK         0x000000fe
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_reserved1_SHIFT        1

/* MM_M2MC0 :: BSTC_COMPRESS_CONTROL :: MAX_RANGE_BA_PLANE [00:00] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_MAX_RANGE_BA_PLANE_MASK 0x00000001
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_MAX_RANGE_BA_PLANE_SHIFT 0
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_MAX_RANGE_BA_PLANE_DEFAULT 0x00000001
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_MAX_RANGE_BA_PLANE_DISABLE 0
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CONTROL_MAX_RANGE_BA_PLANE_ENABLE 1

/***************************************************************************
 *BSTC_COMPRESS_CHECKSUM_CONTROL - BSTC Compression Checksum Control
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_COMPRESS_CHECKSUM_CONTROL :: NUM_OF_BLKS [31:08] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_NUM_OF_BLKS_MASK 0xffffff00
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_NUM_OF_BLKS_SHIFT 8

/* MM_M2MC0 :: BSTC_COMPRESS_CHECKSUM_CONTROL :: reserved0 [07:02] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_reserved0_MASK 0x000000fc
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_reserved0_SHIFT 2

/* MM_M2MC0 :: BSTC_COMPRESS_CHECKSUM_CONTROL :: DATA_SELECT [01:01] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_DATA_SELECT_MASK 0x00000002
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_DATA_SELECT_SHIFT 1
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_DATA_SELECT_INPUT 0
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_DATA_SELECT_OUTPUT 1

/* MM_M2MC0 :: BSTC_COMPRESS_CHECKSUM_CONTROL :: CHECKSUM_ENABLE [00:00] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_CHECKSUM_ENABLE_MASK 0x00000001
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_CHECKSUM_ENABLE_SHIFT 0
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_CHECKSUM_ENABLE_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_CHECKSUM_ENABLE_DISABLE 0
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CONTROL_CHECKSUM_ENABLE_ENABLE 1

/***************************************************************************
 *BSTC_COMPRESS_CHECKSUM_STATUS - BSTC Compression Checksum Status
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_COMPRESS_CHECKSUM_STATUS :: CHECKSUM [31:00] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_STATUS_CHECKSUM_MASK  0xffffffff
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_STATUS_CHECKSUM_SHIFT 0
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_STATUS_CHECKSUM_DEFAULT 0x00000000

/***************************************************************************
 *BSTC_COMPRESS_CHECKSUM_CLEAR - BSTC Compression Checksum Clear
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_COMPRESS_CHECKSUM_CLEAR :: reserved0 [31:01] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CLEAR_reserved0_MASK  0xfffffffe
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CLEAR_reserved0_SHIFT 1

/* MM_M2MC0 :: BSTC_COMPRESS_CHECKSUM_CLEAR :: CHECKSUM_CLEAR [00:00] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CLEAR_CHECKSUM_CLEAR_MASK 0x00000001
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CLEAR_CHECKSUM_CLEAR_SHIFT 0
#define BCHP_MM_M2MC0_BSTC_COMPRESS_CHECKSUM_CLEAR_CHECKSUM_CLEAR_DEFAULT 0x00000000

/***************************************************************************
 *BSTC_COMPRESS_DEBUG_CONTROL - BSTC Compression Debug Control
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_COMPRESS_DEBUG_CONTROL :: INCOMPLETE_4X4_TIMEOUT_COUNT [31:16] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CONTROL_INCOMPLETE_4X4_TIMEOUT_COUNT_MASK 0xffff0000
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CONTROL_INCOMPLETE_4X4_TIMEOUT_COUNT_SHIFT 16
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CONTROL_INCOMPLETE_4X4_TIMEOUT_COUNT_DEFAULT 0x00000000

/* MM_M2MC0 :: BSTC_COMPRESS_DEBUG_CONTROL :: OUTPUT_TIMEOUT_COUNT [15:00] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CONTROL_OUTPUT_TIMEOUT_COUNT_MASK 0x0000ffff
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CONTROL_OUTPUT_TIMEOUT_COUNT_SHIFT 0
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CONTROL_OUTPUT_TIMEOUT_COUNT_DEFAULT 0x00000000

/***************************************************************************
 *BSTC_COMPRESS_DEBUG_STATUS - BSTC Compression Debug Status
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_COMPRESS_DEBUG_STATUS :: BLK_COUNT [31:08] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_STATUS_BLK_COUNT_MASK    0xffffff00
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_STATUS_BLK_COUNT_SHIFT   8

/* MM_M2MC0 :: BSTC_COMPRESS_DEBUG_STATUS :: PIXEL_SET_COUNT [07:04] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_STATUS_PIXEL_SET_COUNT_MASK 0x000000f0
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_STATUS_PIXEL_SET_COUNT_SHIFT 4

/* MM_M2MC0 :: BSTC_COMPRESS_DEBUG_STATUS :: reserved0 [03:02] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_STATUS_reserved0_MASK    0x0000000c
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_STATUS_reserved0_SHIFT   2

/* MM_M2MC0 :: BSTC_COMPRESS_DEBUG_STATUS :: INCOMPLETE_4X4_BLK_INPUT [01:01] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_STATUS_INCOMPLETE_4X4_BLK_INPUT_MASK 0x00000002
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_STATUS_INCOMPLETE_4X4_BLK_INPUT_SHIFT 1

/* MM_M2MC0 :: BSTC_COMPRESS_DEBUG_STATUS :: TIMEOUT_AT_OUTPUT [00:00] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_STATUS_TIMEOUT_AT_OUTPUT_MASK 0x00000001
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_STATUS_TIMEOUT_AT_OUTPUT_SHIFT 0

/***************************************************************************
 *BSTC_COMPRESS_DEBUG_CLEAR - BSTC Compression Debug Clear
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_COMPRESS_DEBUG_CLEAR :: reserved0 [31:02] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CLEAR_reserved0_MASK     0xfffffffc
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CLEAR_reserved0_SHIFT    2

/* MM_M2MC0 :: BSTC_COMPRESS_DEBUG_CLEAR :: INCOMPLETE_4X4_BLK_INPUT_CLEAR [01:01] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CLEAR_INCOMPLETE_4X4_BLK_INPUT_CLEAR_MASK 0x00000002
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CLEAR_INCOMPLETE_4X4_BLK_INPUT_CLEAR_SHIFT 1

/* MM_M2MC0 :: BSTC_COMPRESS_DEBUG_CLEAR :: TIMEOUT_AT_OUTPUT_CLEAR [00:00] */
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CLEAR_TIMEOUT_AT_OUTPUT_CLEAR_MASK 0x00000001
#define BCHP_MM_M2MC0_BSTC_COMPRESS_DEBUG_CLEAR_TIMEOUT_AT_OUTPUT_CLEAR_SHIFT 0

/***************************************************************************
 *BSTC_DECOMPRESS_REVISION - BSTC Decompression Revision ID register
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_DECOMPRESS_REVISION :: reserved0 [31:16] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_REVISION_reserved0_MASK      0xffff0000
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_REVISION_reserved0_SHIFT     16

/* MM_M2MC0 :: BSTC_DECOMPRESS_REVISION :: MAJOR [15:08] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_REVISION_MAJOR_MASK          0x0000ff00
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_REVISION_MAJOR_SHIFT         8
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_REVISION_MAJOR_DEFAULT       0x00000000

/* MM_M2MC0 :: BSTC_DECOMPRESS_REVISION :: MINOR [07:00] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_REVISION_MINOR_MASK          0x000000ff
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_REVISION_MINOR_SHIFT         0
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_REVISION_MINOR_DEFAULT       0x00000000

/***************************************************************************
 *BSTC_DECOMPRESS_STATUS_MUX - BSTC Decompression Status Mux
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_DECOMPRESS_STATUS_MUX :: reserved0 [31:02] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_STATUS_MUX_reserved0_MASK    0xfffffffc
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_STATUS_MUX_reserved0_SHIFT   2

/* MM_M2MC0 :: BSTC_DECOMPRESS_STATUS_MUX :: MUX [01:00] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_STATUS_MUX_MUX_MASK          0x00000003
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_STATUS_MUX_MUX_SHIFT         0
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_STATUS_MUX_MUX_DEFAULT       0x00000000
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_STATUS_MUX_MUX_SELECT_SRC    0
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_STATUS_MUX_MUX_SELECT_DEST   1
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_STATUS_MUX_MUX_SELECT_OUTPUT 2

/***************************************************************************
 *BSTC_DECOMPRESS_CHECKSUM_CONTROL - BSTC Decompression Checksum Control
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_DECOMPRESS_CHECKSUM_CONTROL :: NUM_OF_BLKS [31:08] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_NUM_OF_BLKS_MASK 0xffffff00
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_NUM_OF_BLKS_SHIFT 8

/* MM_M2MC0 :: BSTC_DECOMPRESS_CHECKSUM_CONTROL :: reserved0 [07:02] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_reserved0_MASK 0x000000fc
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_reserved0_SHIFT 2

/* MM_M2MC0 :: BSTC_DECOMPRESS_CHECKSUM_CONTROL :: DATA_SELECT [01:01] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_DATA_SELECT_MASK 0x00000002
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_DATA_SELECT_SHIFT 1
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_DATA_SELECT_INPUT 0
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_DATA_SELECT_OUTPUT 1

/* MM_M2MC0 :: BSTC_DECOMPRESS_CHECKSUM_CONTROL :: CHECKSUM_ENABLE [00:00] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_CHECKSUM_ENABLE_MASK 0x00000001
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_CHECKSUM_ENABLE_SHIFT 0
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_CHECKSUM_ENABLE_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_CHECKSUM_ENABLE_DISABLE 0
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CONTROL_CHECKSUM_ENABLE_ENABLE 1

/***************************************************************************
 *BSTC_DECOMPRESS_CHECKSUM_STATUS - BSTC Decompression Checksum Status
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_DECOMPRESS_CHECKSUM_STATUS :: CHECKSUM [31:00] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_STATUS_CHECKSUM_MASK 0xffffffff
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_STATUS_CHECKSUM_SHIFT 0
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_STATUS_CHECKSUM_DEFAULT 0x00000000

/***************************************************************************
 *BSTC_DECOMPRESS_CHECKSUM_CLEAR - BSTC Decompression Checksum Status
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_DECOMPRESS_CHECKSUM_CLEAR :: reserved0 [31:01] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CLEAR_reserved0_MASK 0xfffffffe
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CLEAR_reserved0_SHIFT 1

/* MM_M2MC0 :: BSTC_DECOMPRESS_CHECKSUM_CLEAR :: CHECKSUM_CLEAR [00:00] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CLEAR_CHECKSUM_CLEAR_MASK 0x00000001
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CLEAR_CHECKSUM_CLEAR_SHIFT 0
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_CHECKSUM_CLEAR_CHECKSUM_CLEAR_DEFAULT 0x00000000

/***************************************************************************
 *BSTC_DECOMPRESS_DEBUG_CONTROL - BSTC Decompression Debug Control
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_DECOMPRESS_DEBUG_CONTROL :: reserved0 [31:16] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CONTROL_reserved0_MASK 0xffff0000
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CONTROL_reserved0_SHIFT 16

/* MM_M2MC0 :: BSTC_DECOMPRESS_DEBUG_CONTROL :: TIMEOUT_COUNT [15:00] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CONTROL_TIMEOUT_COUNT_MASK 0x0000ffff
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CONTROL_TIMEOUT_COUNT_SHIFT 0
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CONTROL_TIMEOUT_COUNT_DEFAULT 0x00000000

/***************************************************************************
 *BSTC_DECOMPRESS_DEBUG_STATUS - BSTC Decompression Debug Status
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_DECOMPRESS_DEBUG_STATUS :: BLK_COUNT [31:08] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_STATUS_BLK_COUNT_MASK  0xffffff00
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_STATUS_BLK_COUNT_SHIFT 8

/* MM_M2MC0 :: BSTC_DECOMPRESS_DEBUG_STATUS :: PIXEL_SET_COUNT [07:04] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_STATUS_PIXEL_SET_COUNT_MASK 0x000000f0
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_STATUS_PIXEL_SET_COUNT_SHIFT 4

/* MM_M2MC0 :: BSTC_DECOMPRESS_DEBUG_STATUS :: reserved0 [03:02] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_STATUS_reserved0_MASK  0x0000000c
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_STATUS_reserved0_SHIFT 2

/* MM_M2MC0 :: BSTC_DECOMPRESS_DEBUG_STATUS :: MIN_GT_MAX [01:01] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_STATUS_MIN_GT_MAX_MASK 0x00000002
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_STATUS_MIN_GT_MAX_SHIFT 1

/* MM_M2MC0 :: BSTC_DECOMPRESS_DEBUG_STATUS :: TIMEOUT [00:00] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_STATUS_TIMEOUT_MASK    0x00000001
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_STATUS_TIMEOUT_SHIFT   0

/***************************************************************************
 *BSTC_DECOMPRESS_DEBUG_CLEAR - BSTC Decompression Debug Clear
 ***************************************************************************/
/* MM_M2MC0 :: BSTC_DECOMPRESS_DEBUG_CLEAR :: reserved0 [31:02] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CLEAR_reserved0_MASK   0xfffffffc
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CLEAR_reserved0_SHIFT  2

/* MM_M2MC0 :: BSTC_DECOMPRESS_DEBUG_CLEAR :: MIN_GT_MAX_CLEAR [01:01] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CLEAR_MIN_GT_MAX_CLEAR_MASK 0x00000002
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CLEAR_MIN_GT_MAX_CLEAR_SHIFT 1

/* MM_M2MC0 :: BSTC_DECOMPRESS_DEBUG_CLEAR :: TIMEOUT_CLEAR [00:00] */
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CLEAR_TIMEOUT_CLEAR_MASK 0x00000001
#define BCHP_MM_M2MC0_BSTC_DECOMPRESS_DEBUG_CLEAR_TIMEOUT_CLEAR_SHIFT 0

/***************************************************************************
 *DRAM_MAP - DRAM MAP Version
 ***************************************************************************/
/* MM_M2MC0 :: DRAM_MAP :: reserved0 [31:04] */
#define BCHP_MM_M2MC0_DRAM_MAP_reserved0_MASK                      0xfffffff0
#define BCHP_MM_M2MC0_DRAM_MAP_reserved0_SHIFT                     4

/* MM_M2MC0 :: DRAM_MAP :: MAP [03:00] */
#define BCHP_MM_M2MC0_DRAM_MAP_MAP_MASK                            0x0000000f
#define BCHP_MM_M2MC0_DRAM_MAP_MAP_SHIFT                           0
#define BCHP_MM_M2MC0_DRAM_MAP_MAP_DEFAULT                         0x00000002
#define BCHP_MM_M2MC0_DRAM_MAP_MAP_MAP_2P0                         0
#define BCHP_MM_M2MC0_DRAM_MAP_MAP_MAP_5P0                         1
#define BCHP_MM_M2MC0_DRAM_MAP_MAP_MAP_8P0                         2

/***************************************************************************
 *DEBUG_STATUS - M2MC Debug Status register
 ***************************************************************************/
/* MM_M2MC0 :: DEBUG_STATUS :: reserved0 [31:01] */
#define BCHP_MM_M2MC0_DEBUG_STATUS_reserved0_MASK                  0xfffffffe
#define BCHP_MM_M2MC0_DEBUG_STATUS_reserved0_SHIFT                 1

/* MM_M2MC0 :: DEBUG_STATUS :: BLIT_STARTED [00:00] */
#define BCHP_MM_M2MC0_DEBUG_STATUS_BLIT_STARTED_MASK               0x00000001
#define BCHP_MM_M2MC0_DEBUG_STATUS_BLIT_STARTED_SHIFT              0
#define BCHP_MM_M2MC0_DEBUG_STATUS_BLIT_STARTED_WAITING            0
#define BCHP_MM_M2MC0_DEBUG_STATUS_BLIT_STARTED_STARTED            1

/***************************************************************************
 *MM_M2MC_SYSTEM_CONFIG - Mipmap M2MC System Config
 ***************************************************************************/
/* MM_M2MC0 :: MM_M2MC_SYSTEM_CONFIG :: reserved0 [31:12] */
#define BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG_reserved0_MASK         0xfffff000
#define BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG_reserved0_SHIFT        12

/* MM_M2MC0 :: MM_M2MC_SYSTEM_CONFIG :: XOR_ADDR [11:04] */
#define BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG_XOR_ADDR_MASK          0x00000ff0
#define BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG_XOR_ADDR_SHIFT         4
#define BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG_XOR_ADDR_DEFAULT       0x00000010

/* MM_M2MC0 :: MM_M2MC_SYSTEM_CONFIG :: PAGE_SIZE [03:02] */
#define BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG_PAGE_SIZE_MASK         0x0000000c
#define BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG_PAGE_SIZE_SHIFT        2
#define BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG_PAGE_SIZE_DEFAULT      0x00000001

/* MM_M2MC0 :: MM_M2MC_SYSTEM_CONFIG :: NUM_BANKS [01:00] */
#define BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG_NUM_BANKS_MASK         0x00000003
#define BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG_NUM_BANKS_SHIFT        0
#define BCHP_MM_M2MC0_MM_M2MC_SYSTEM_CONFIG_NUM_BANKS_DEFAULT      0x00000001

/***************************************************************************
 *MM_M2MC_MIPMAP_STATUS - Mipmap M2MC MipMap Status
 ***************************************************************************/
/* MM_M2MC0 :: MM_M2MC_MIPMAP_STATUS :: reserved0 [31:04] */
#define BCHP_MM_M2MC0_MM_M2MC_MIPMAP_STATUS_reserved0_MASK         0xfffffff0
#define BCHP_MM_M2MC0_MM_M2MC_MIPMAP_STATUS_reserved0_SHIFT        4

/* MM_M2MC0 :: MM_M2MC_MIPMAP_STATUS :: MAX_MIPMAP_LEVEL [03:00] */
#define BCHP_MM_M2MC0_MM_M2MC_MIPMAP_STATUS_MAX_MIPMAP_LEVEL_MASK  0x0000000f
#define BCHP_MM_M2MC0_MM_M2MC_MIPMAP_STATUS_MAX_MIPMAP_LEVEL_SHIFT 0

/***************************************************************************
 *SCRATCH_LIST - M2MC Scratch register (Included in DMA list structure)
 ***************************************************************************/
/* MM_M2MC0 :: SCRATCH_LIST :: VALUE [31:00] */
#define BCHP_MM_M2MC0_SCRATCH_LIST_VALUE_MASK                      0xffffffff
#define BCHP_MM_M2MC0_SCRATCH_LIST_VALUE_SHIFT                     0
#define BCHP_MM_M2MC0_SCRATCH_LIST_VALUE_DEFAULT                   0x00000000

/***************************************************************************
 *SRC_FEEDER_ENABLE - Source plane enable
 ***************************************************************************/
/* MM_M2MC0 :: SRC_FEEDER_ENABLE :: reserved0 [31:01] */
#define BCHP_MM_M2MC0_SRC_FEEDER_ENABLE_reserved0_MASK             0xfffffffe
#define BCHP_MM_M2MC0_SRC_FEEDER_ENABLE_reserved0_SHIFT            1

/* MM_M2MC0 :: SRC_FEEDER_ENABLE :: ENABLE [00:00] */
#define BCHP_MM_M2MC0_SRC_FEEDER_ENABLE_ENABLE_MASK                0x00000001
#define BCHP_MM_M2MC0_SRC_FEEDER_ENABLE_ENABLE_SHIFT               0
#define BCHP_MM_M2MC0_SRC_FEEDER_ENABLE_ENABLE_DEFAULT             0x00000000
#define BCHP_MM_M2MC0_SRC_FEEDER_ENABLE_ENABLE_DISABLE             0
#define BCHP_MM_M2MC0_SRC_FEEDER_ENABLE_ENABLE_ENABLE              1

/***************************************************************************
 *SRC_SURFACE_ADDR_0 - Source surface 0 address
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_ADDR_0 :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_0_reserved0_MASK            BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_0_reserved0_SHIFT           40

/* MM_M2MC0 :: SRC_SURFACE_ADDR_0 :: ADDR [39:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_0_ADDR_MASK                 BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_0_ADDR_SHIFT                0
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_0_ADDR_DEFAULT              0

/***************************************************************************
 *SRC_SURFACE_ADDR_0_BOT_FLD - Source surface 0, field 1  address
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_ADDR_0_BOT_FLD :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_0_BOT_FLD_reserved0_MASK    BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_0_BOT_FLD_reserved0_SHIFT   40

/* MM_M2MC0 :: SRC_SURFACE_ADDR_0_BOT_FLD :: ADDR [39:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_0_BOT_FLD_ADDR_MASK         BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_0_BOT_FLD_ADDR_SHIFT        0
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_0_BOT_FLD_ADDR_DEFAULT      0

/***************************************************************************
 *SRC_SURFACE_STRIDE_0 - Source surface 0 STRIDE
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_STRIDE_0 :: reserved0 [31:17] */
#define BCHP_MM_M2MC0_SRC_SURFACE_STRIDE_0_reserved0_MASK          0xfffe0000
#define BCHP_MM_M2MC0_SRC_SURFACE_STRIDE_0_reserved0_SHIFT         17

/* MM_M2MC0 :: SRC_SURFACE_STRIDE_0 :: STRIDE [16:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_STRIDE_0_STRIDE_MASK             0x0001ffff
#define BCHP_MM_M2MC0_SRC_SURFACE_STRIDE_0_STRIDE_SHIFT            0
#define BCHP_MM_M2MC0_SRC_SURFACE_STRIDE_0_STRIDE_DEFAULT          0x00000000

/***************************************************************************
 *SRC_SURFACE_ADDR_1 - Source surface 1 address
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_ADDR_1 :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_1_reserved0_MASK            BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_1_reserved0_SHIFT           40

/* MM_M2MC0 :: SRC_SURFACE_ADDR_1 :: ADDR [39:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_1_ADDR_MASK                 BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_1_ADDR_SHIFT                0
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_1_ADDR_DEFAULT              0

/***************************************************************************
 *SRC_SURFACE_ADDR_1_BOT_FLD - Source surface 1, field 1 address
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_ADDR_1_BOT_FLD :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_1_BOT_FLD_reserved0_MASK    BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_1_BOT_FLD_reserved0_SHIFT   40

/* MM_M2MC0 :: SRC_SURFACE_ADDR_1_BOT_FLD :: ADDR [39:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_1_BOT_FLD_ADDR_MASK         BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_1_BOT_FLD_ADDR_SHIFT        0
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_1_BOT_FLD_ADDR_DEFAULT      0

/***************************************************************************
 *SRC_SURFACE_STRIDE_1 - Source surface 1 STRIDE
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_STRIDE_1 :: reserved0 [31:17] */
#define BCHP_MM_M2MC0_SRC_SURFACE_STRIDE_1_reserved0_MASK          0xfffe0000
#define BCHP_MM_M2MC0_SRC_SURFACE_STRIDE_1_reserved0_SHIFT         17

/* MM_M2MC0 :: SRC_SURFACE_STRIDE_1 :: STRIDE [16:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_STRIDE_1_STRIDE_MASK             0x0001ffff
#define BCHP_MM_M2MC0_SRC_SURFACE_STRIDE_1_STRIDE_SHIFT            0
#define BCHP_MM_M2MC0_SRC_SURFACE_STRIDE_1_STRIDE_DEFAULT          0x00000000

/***************************************************************************
 *SRC_SURFACE_0_FORMAT_DEF_1 - Source pixel format 1 for surface 0
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_1 :: reserved0 [31:20] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_reserved0_MASK    0xfff00000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_reserved0_SHIFT   20

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_1 :: FORMAT_TYPE [19:16] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_FORMAT_TYPE_MASK  0x000f0000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_FORMAT_TYPE_SHIFT 16
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_FORMAT_TYPE_DEFAULT 0x00000000

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_1 :: CH3_NUM_BITS [15:12] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_CH3_NUM_BITS_MASK 0x0000f000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_CH3_NUM_BITS_SHIFT 12
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_CH3_NUM_BITS_DEFAULT 0x00000008

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_1 :: CH2_NUM_BITS [11:08] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_CH2_NUM_BITS_MASK 0x00000f00
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_CH2_NUM_BITS_SHIFT 8
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_CH2_NUM_BITS_DEFAULT 0x00000008

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_1 :: CH1_NUM_BITS [07:04] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_CH1_NUM_BITS_MASK 0x000000f0
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_CH1_NUM_BITS_SHIFT 4
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_CH1_NUM_BITS_DEFAULT 0x00000008

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_1 :: CH0_NUM_BITS [03:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_CH0_NUM_BITS_MASK 0x0000000f
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_CH0_NUM_BITS_SHIFT 0
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_1_CH0_NUM_BITS_DEFAULT 0x00000008

/***************************************************************************
 *SRC_SURFACE_0_FORMAT_DEF_2 - Source pixel format 2 for surface 0
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_2 :: reserved0 [31:29] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_reserved0_MASK    0xe0000000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_reserved0_SHIFT   29

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_2 :: CH3_LSB_POS [28:24] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_CH3_LSB_POS_MASK  0x1f000000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_CH3_LSB_POS_SHIFT 24
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_CH3_LSB_POS_DEFAULT 0x00000018

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_2 :: reserved1 [23:21] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_reserved1_MASK    0x00e00000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_reserved1_SHIFT   21

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_2 :: CH2_LSB_POS [20:16] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_CH2_LSB_POS_MASK  0x001f0000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_CH2_LSB_POS_SHIFT 16
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_CH2_LSB_POS_DEFAULT 0x00000010

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_2 :: reserved2 [15:13] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_reserved2_MASK    0x0000e000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_reserved2_SHIFT   13

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_2 :: CH1_LSB_POS [12:08] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_CH1_LSB_POS_MASK  0x00001f00
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_CH1_LSB_POS_SHIFT 8
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_CH1_LSB_POS_DEFAULT 0x00000008

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_2 :: reserved3 [07:05] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_reserved3_MASK    0x000000e0
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_reserved3_SHIFT   5

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_2 :: CH0_LSB_POS [04:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_CH0_LSB_POS_MASK  0x0000001f
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_CH0_LSB_POS_SHIFT 0
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_2_CH0_LSB_POS_DEFAULT 0x00000000

/***************************************************************************
 *SRC_SURFACE_0_FORMAT_DEF_3 - Source pixel format 3 for surface 0
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_3 :: reserved0 [31:23] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_reserved0_MASK    0xff800000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_reserved0_SHIFT   23

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_3 :: RANGE_EXP_MAP_SCALE_FACTOR_C [22:18] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_C_MASK 0x007c0000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_C_SHIFT 18
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_C_DEFAULT 0x00000008

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_3 :: RANGE_EXP_MAP_SCALE_FACTOR_Y [17:13] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_Y_MASK 0x0003e000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_Y_SHIFT 13
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_Y_DEFAULT 0x00000008

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_3 :: CH3_DISABLE [12:12] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CH3_DISABLE_MASK  0x00001000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CH3_DISABLE_SHIFT 12
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CH3_DISABLE_DEFAULT 0x00000000

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_3 :: CH2_DISABLE [11:11] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CH2_DISABLE_MASK  0x00000800
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CH2_DISABLE_SHIFT 11
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CH2_DISABLE_DEFAULT 0x00000000

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_3 :: CH1_DISABLE [10:10] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CH1_DISABLE_MASK  0x00000400
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CH1_DISABLE_SHIFT 10
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CH1_DISABLE_DEFAULT 0x00000000

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_3 :: CH0_DISABLE [09:09] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CH0_DISABLE_MASK  0x00000200
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CH0_DISABLE_SHIFT 9
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CH0_DISABLE_DEFAULT 0x00000000

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_3 :: ZERO_PAD [08:08] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_ZERO_PAD_MASK     0x00000100
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_ZERO_PAD_SHIFT    8
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_ZERO_PAD_DEFAULT  0x00000000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_ZERO_PAD_ZERO_PAD 1
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_ZERO_PAD_REPLICATE 0

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_3 :: reserved1 [07:05] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_reserved1_MASK    0x000000e0
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_reserved1_SHIFT   5

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_3 :: PALETTE_BYPASS [04:04] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_PALETTE_BYPASS_MASK 0x00000010
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_PALETTE_BYPASS_SHIFT 4
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_PALETTE_BYPASS_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_PALETTE_BYPASS_DONT_LOOKUP 0
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_PALETTE_BYPASS_LOOKUP 1

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_3 :: reserved2 [03:01] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_reserved2_MASK    0x0000000e
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_reserved2_SHIFT   1

/* MM_M2MC0 :: SRC_SURFACE_0_FORMAT_DEF_3 :: CHROMA_FILTER [00:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CHROMA_FILTER_MASK 0x00000001
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CHROMA_FILTER_SHIFT 0
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CHROMA_FILTER_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CHROMA_FILTER_REPLICATE 0
#define BCHP_MM_M2MC0_SRC_SURFACE_0_FORMAT_DEF_3_CHROMA_FILTER_FILTER 1

/***************************************************************************
 *SRC_SURFACE_1_FORMAT_DEF_1 - Source pixel format 1 for surface 1
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_1 :: reserved0 [31:20] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_reserved0_MASK    0xfff00000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_reserved0_SHIFT   20

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_1 :: FORMAT_TYPE [19:16] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_FORMAT_TYPE_MASK  0x000f0000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_FORMAT_TYPE_SHIFT 16
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_FORMAT_TYPE_DEFAULT 0x00000000

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_1 :: CH3_NUM_BITS [15:12] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_CH3_NUM_BITS_MASK 0x0000f000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_CH3_NUM_BITS_SHIFT 12
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_CH3_NUM_BITS_DEFAULT 0x00000008

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_1 :: CH2_NUM_BITS [11:08] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_CH2_NUM_BITS_MASK 0x00000f00
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_CH2_NUM_BITS_SHIFT 8
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_CH2_NUM_BITS_DEFAULT 0x00000008

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_1 :: CH1_NUM_BITS [07:04] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_CH1_NUM_BITS_MASK 0x000000f0
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_CH1_NUM_BITS_SHIFT 4
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_CH1_NUM_BITS_DEFAULT 0x00000008

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_1 :: CH0_NUM_BITS [03:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_CH0_NUM_BITS_MASK 0x0000000f
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_CH0_NUM_BITS_SHIFT 0
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_1_CH0_NUM_BITS_DEFAULT 0x00000008

/***************************************************************************
 *SRC_SURFACE_1_FORMAT_DEF_2 - Source pixel format 2 for surface 1
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_2 :: reserved0 [31:29] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_reserved0_MASK    0xe0000000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_reserved0_SHIFT   29

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_2 :: CH3_LSB_POS [28:24] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_CH3_LSB_POS_MASK  0x1f000000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_CH3_LSB_POS_SHIFT 24
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_CH3_LSB_POS_DEFAULT 0x00000018

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_2 :: reserved1 [23:21] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_reserved1_MASK    0x00e00000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_reserved1_SHIFT   21

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_2 :: CH2_LSB_POS [20:16] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_CH2_LSB_POS_MASK  0x001f0000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_CH2_LSB_POS_SHIFT 16
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_CH2_LSB_POS_DEFAULT 0x00000010

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_2 :: reserved2 [15:13] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_reserved2_MASK    0x0000e000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_reserved2_SHIFT   13

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_2 :: CH1_LSB_POS [12:08] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_CH1_LSB_POS_MASK  0x00001f00
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_CH1_LSB_POS_SHIFT 8
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_CH1_LSB_POS_DEFAULT 0x00000008

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_2 :: reserved3 [07:05] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_reserved3_MASK    0x000000e0
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_reserved3_SHIFT   5

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_2 :: CH0_LSB_POS [04:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_CH0_LSB_POS_MASK  0x0000001f
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_CH0_LSB_POS_SHIFT 0
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_2_CH0_LSB_POS_DEFAULT 0x00000000

/***************************************************************************
 *SRC_SURFACE_1_FORMAT_DEF_3 - Source pixel format 3 for surface 1
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_3 :: reserved0 [31:23] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_reserved0_MASK    0xff800000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_reserved0_SHIFT   23

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_3 :: RANGE_EXP_MAP_SCALE_FACTOR_C [22:18] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_C_MASK 0x007c0000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_C_SHIFT 18
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_C_DEFAULT 0x00000008

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_3 :: RANGE_EXP_MAP_SCALE_FACTOR_Y [17:13] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_Y_MASK 0x0003e000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_Y_SHIFT 13
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_RANGE_EXP_MAP_SCALE_FACTOR_Y_DEFAULT 0x00000008

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_3 :: CH3_DISABLE [12:12] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CH3_DISABLE_MASK  0x00001000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CH3_DISABLE_SHIFT 12
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CH3_DISABLE_DEFAULT 0x00000000

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_3 :: CH2_DISABLE [11:11] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CH2_DISABLE_MASK  0x00000800
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CH2_DISABLE_SHIFT 11
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CH2_DISABLE_DEFAULT 0x00000000

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_3 :: CH1_DISABLE [10:10] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CH1_DISABLE_MASK  0x00000400
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CH1_DISABLE_SHIFT 10
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CH1_DISABLE_DEFAULT 0x00000000

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_3 :: CH0_DISABLE [09:09] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CH0_DISABLE_MASK  0x00000200
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CH0_DISABLE_SHIFT 9
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CH0_DISABLE_DEFAULT 0x00000000

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_3 :: ZERO_PAD [08:08] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_ZERO_PAD_MASK     0x00000100
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_ZERO_PAD_SHIFT    8
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_ZERO_PAD_DEFAULT  0x00000000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_ZERO_PAD_ZERO_PAD 1
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_ZERO_PAD_REPLICATE 0

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_3 :: reserved1 [07:05] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_reserved1_MASK    0x000000e0
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_reserved1_SHIFT   5

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_3 :: PALETTE_BYPASS [04:04] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_PALETTE_BYPASS_MASK 0x00000010
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_PALETTE_BYPASS_SHIFT 4
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_PALETTE_BYPASS_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_PALETTE_BYPASS_DONT_LOOKUP 0
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_PALETTE_BYPASS_LOOKUP 1

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_3 :: reserved2 [03:01] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_reserved2_MASK    0x0000000e
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_reserved2_SHIFT   1

/* MM_M2MC0 :: SRC_SURFACE_1_FORMAT_DEF_3 :: CHROMA_FILTER [00:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CHROMA_FILTER_MASK 0x00000001
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CHROMA_FILTER_SHIFT 0
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CHROMA_FILTER_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CHROMA_FILTER_REPLICATE 0
#define BCHP_MM_M2MC0_SRC_SURFACE_1_FORMAT_DEF_3_CHROMA_FILTER_FILTER 1

/***************************************************************************
 *SRC_W_ALPHA - Source Alpha W format
 ***************************************************************************/
/* MM_M2MC0 :: SRC_W_ALPHA :: reserved0 [31:24] */
#define BCHP_MM_M2MC0_SRC_W_ALPHA_reserved0_MASK                   0xff000000
#define BCHP_MM_M2MC0_SRC_W_ALPHA_reserved0_SHIFT                  24

/* MM_M2MC0 :: SRC_W_ALPHA :: W1_ALPHA [23:16] */
#define BCHP_MM_M2MC0_SRC_W_ALPHA_W1_ALPHA_MASK                    0x00ff0000
#define BCHP_MM_M2MC0_SRC_W_ALPHA_W1_ALPHA_SHIFT                   16

/* MM_M2MC0 :: SRC_W_ALPHA :: reserved1 [15:08] */
#define BCHP_MM_M2MC0_SRC_W_ALPHA_reserved1_MASK                   0x0000ff00
#define BCHP_MM_M2MC0_SRC_W_ALPHA_reserved1_SHIFT                  8

/* MM_M2MC0 :: SRC_W_ALPHA :: W0_ALPHA [07:00] */
#define BCHP_MM_M2MC0_SRC_W_ALPHA_W0_ALPHA_MASK                    0x000000ff
#define BCHP_MM_M2MC0_SRC_W_ALPHA_W0_ALPHA_SHIFT                   0

/***************************************************************************
 *SRC_CONSTANT_COLOR - Source constant color
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CONSTANT_COLOR :: ALPHA [31:24] */
#define BCHP_MM_M2MC0_SRC_CONSTANT_COLOR_ALPHA_MASK                0xff000000
#define BCHP_MM_M2MC0_SRC_CONSTANT_COLOR_ALPHA_SHIFT               24

/* MM_M2MC0 :: SRC_CONSTANT_COLOR :: RED [23:16] */
#define BCHP_MM_M2MC0_SRC_CONSTANT_COLOR_RED_MASK                  0x00ff0000
#define BCHP_MM_M2MC0_SRC_CONSTANT_COLOR_RED_SHIFT                 16

/* MM_M2MC0 :: SRC_CONSTANT_COLOR :: GREEN [15:08] */
#define BCHP_MM_M2MC0_SRC_CONSTANT_COLOR_GREEN_MASK                0x0000ff00
#define BCHP_MM_M2MC0_SRC_CONSTANT_COLOR_GREEN_SHIFT               8

/* MM_M2MC0 :: SRC_CONSTANT_COLOR :: BLUE [07:00] */
#define BCHP_MM_M2MC0_SRC_CONSTANT_COLOR_BLUE_MASK                 0x000000ff
#define BCHP_MM_M2MC0_SRC_CONSTANT_COLOR_BLUE_SHIFT                0

/***************************************************************************
 *SRC_SURFACE_ADDR_2 - Source surface 2 address
 ***************************************************************************/
/* MM_M2MC0 :: SRC_SURFACE_ADDR_2 :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_2_reserved0_MASK            BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_2_reserved0_SHIFT           40

/* MM_M2MC0 :: SRC_SURFACE_ADDR_2 :: ADDR [39:00] */
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_2_ADDR_MASK                 BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_2_ADDR_SHIFT                0
#define BCHP_MM_M2MC0_SRC_SURFACE_ADDR_2_ADDR_DEFAULT              0

/***************************************************************************
 *OUTPUT_FEEDER_ENABLE - Output plane enable
 ***************************************************************************/
/* MM_M2MC0 :: OUTPUT_FEEDER_ENABLE :: reserved0 [31:01] */
#define BCHP_MM_M2MC0_OUTPUT_FEEDER_ENABLE_reserved0_MASK          0xfffffffe
#define BCHP_MM_M2MC0_OUTPUT_FEEDER_ENABLE_reserved0_SHIFT         1

/* MM_M2MC0 :: OUTPUT_FEEDER_ENABLE :: ENABLE [00:00] */
#define BCHP_MM_M2MC0_OUTPUT_FEEDER_ENABLE_ENABLE_MASK             0x00000001
#define BCHP_MM_M2MC0_OUTPUT_FEEDER_ENABLE_ENABLE_SHIFT            0
#define BCHP_MM_M2MC0_OUTPUT_FEEDER_ENABLE_ENABLE_DEFAULT          0x00000000
#define BCHP_MM_M2MC0_OUTPUT_FEEDER_ENABLE_ENABLE_DISABLE          0
#define BCHP_MM_M2MC0_OUTPUT_FEEDER_ENABLE_ENABLE_ENABLE           1

/***************************************************************************
 *OUTPUT_SURFACE_ADDR_0 - Output surface 0 address
 ***************************************************************************/
/* MM_M2MC0 :: OUTPUT_SURFACE_ADDR_0 :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_ADDR_0_reserved0_MASK         BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_ADDR_0_reserved0_SHIFT        40

/* MM_M2MC0 :: OUTPUT_SURFACE_ADDR_0 :: ADDR [39:00] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_ADDR_0_ADDR_MASK              BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_ADDR_0_ADDR_SHIFT             0
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_ADDR_0_ADDR_DEFAULT           0

/***************************************************************************
 *OUTPUT_SURFACE_STRIDE_0 - Output surface 0 STRIDE
 ***************************************************************************/
/* MM_M2MC0 :: OUTPUT_SURFACE_STRIDE_0 :: reserved0 [31:17] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_STRIDE_0_reserved0_MASK       0xfffe0000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_STRIDE_0_reserved0_SHIFT      17

/* MM_M2MC0 :: OUTPUT_SURFACE_STRIDE_0 :: STRIDE [16:00] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_STRIDE_0_STRIDE_MASK          0x0001ffff
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_STRIDE_0_STRIDE_SHIFT         0
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_STRIDE_0_STRIDE_DEFAULT       0x00000000

/***************************************************************************
 *OUTPUT_SURFACE_ADDR_1 - Output surface 1 address
 ***************************************************************************/
/* MM_M2MC0 :: OUTPUT_SURFACE_ADDR_1 :: reserved0 [63:40] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_ADDR_1_reserved0_MASK         BCHP_UINT64_C(0xffffff00, 0x00000000)
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_ADDR_1_reserved0_SHIFT        40

/* MM_M2MC0 :: OUTPUT_SURFACE_ADDR_1 :: ADDR [39:00] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_ADDR_1_ADDR_MASK              BCHP_UINT64_C(0x000000ff, 0xffffffff)
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_ADDR_1_ADDR_SHIFT             0
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_ADDR_1_ADDR_DEFAULT           0

/***************************************************************************
 *OUTPUT_SURFACE_STRIDE_1 - Output surface 1 STRIDE
 ***************************************************************************/
/* MM_M2MC0 :: OUTPUT_SURFACE_STRIDE_1 :: reserved0 [31:17] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_STRIDE_1_reserved0_MASK       0xfffe0000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_STRIDE_1_reserved0_SHIFT      17

/* MM_M2MC0 :: OUTPUT_SURFACE_STRIDE_1 :: STRIDE [16:00] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_STRIDE_1_STRIDE_MASK          0x0001ffff
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_STRIDE_1_STRIDE_SHIFT         0
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_STRIDE_1_STRIDE_DEFAULT       0x00000000

/***************************************************************************
 *OUTPUT_SURFACE_FORMAT_DEF_1 - Output pixel format 1
 ***************************************************************************/
/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_1 :: reserved0 [31:20] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_reserved0_MASK   0xfff00000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_reserved0_SHIFT  20

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_1 :: FORMAT_TYPE [19:16] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_FORMAT_TYPE_MASK 0x000f0000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_FORMAT_TYPE_SHIFT 16
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_FORMAT_TYPE_DEFAULT 0x00000000

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_1 :: CH3_NUM_BITS [15:12] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_CH3_NUM_BITS_MASK 0x0000f000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_CH3_NUM_BITS_SHIFT 12
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_CH3_NUM_BITS_DEFAULT 0x00000008

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_1 :: CH2_NUM_BITS [11:08] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_CH2_NUM_BITS_MASK 0x00000f00
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_CH2_NUM_BITS_SHIFT 8
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_CH2_NUM_BITS_DEFAULT 0x00000008

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_1 :: CH1_NUM_BITS [07:04] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_CH1_NUM_BITS_MASK 0x000000f0
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_CH1_NUM_BITS_SHIFT 4
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_CH1_NUM_BITS_DEFAULT 0x00000008

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_1 :: CH0_NUM_BITS [03:00] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_CH0_NUM_BITS_MASK 0x0000000f
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_CH0_NUM_BITS_SHIFT 0
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_1_CH0_NUM_BITS_DEFAULT 0x00000008

/***************************************************************************
 *OUTPUT_SURFACE_FORMAT_DEF_2 - Output pixel format 2
 ***************************************************************************/
/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_2 :: reserved0 [31:29] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_reserved0_MASK   0xe0000000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_reserved0_SHIFT  29

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_2 :: CH3_LSB_POS [28:24] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_CH3_LSB_POS_MASK 0x1f000000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_CH3_LSB_POS_SHIFT 24
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_CH3_LSB_POS_DEFAULT 0x00000018

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_2 :: reserved1 [23:21] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_reserved1_MASK   0x00e00000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_reserved1_SHIFT  21

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_2 :: CH2_LSB_POS [20:16] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_CH2_LSB_POS_MASK 0x001f0000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_CH2_LSB_POS_SHIFT 16
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_CH2_LSB_POS_DEFAULT 0x00000010

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_2 :: reserved2 [15:13] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_reserved2_MASK   0x0000e000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_reserved2_SHIFT  13

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_2 :: CH1_LSB_POS [12:08] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_CH1_LSB_POS_MASK 0x00001f00
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_CH1_LSB_POS_SHIFT 8
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_CH1_LSB_POS_DEFAULT 0x00000008

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_2 :: reserved3 [07:05] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_reserved3_MASK   0x000000e0
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_reserved3_SHIFT  5

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_2 :: CH0_LSB_POS [04:00] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_CH0_LSB_POS_MASK 0x0000001f
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_CH0_LSB_POS_SHIFT 0
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_2_CH0_LSB_POS_DEFAULT 0x00000000

/***************************************************************************
 *OUTPUT_SURFACE_FORMAT_DEF_3 - Output pixel format 3
 ***************************************************************************/
/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_3 :: reserved0 [31:23] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_reserved0_MASK   0xff800000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_reserved0_SHIFT  23

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_3 :: SRGB_CURVE_CORRECTION_ENB [22:22] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_SRGB_CURVE_CORRECTION_ENB_MASK 0x00400000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_SRGB_CURVE_CORRECTION_ENB_SHIFT 22
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_SRGB_CURVE_CORRECTION_ENB_DEFAULT 0x00000000

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_3 :: DISABLE_LINEAR_FILTERING [21:21] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_DISABLE_LINEAR_FILTERING_MASK 0x00200000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_DISABLE_LINEAR_FILTERING_SHIFT 21
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_DISABLE_LINEAR_FILTERING_DEFAULT 0x00000000

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_3 :: DISABLE_L0_WRITE_OUT [20:20] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_DISABLE_L0_WRITE_OUT_MASK 0x00100000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_DISABLE_L0_WRITE_OUT_SHIFT 20
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_DISABLE_L0_WRITE_OUT_DEFAULT 0x00000000

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_3 :: reserved1 [19:17] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_reserved1_MASK   0x000e0000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_reserved1_SHIFT  17

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_3 :: DITHER_ENABLE [16:16] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_DITHER_ENABLE_MASK 0x00010000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_DITHER_ENABLE_SHIFT 16
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_DITHER_ENABLE_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_DITHER_ENABLE_DISABLE 0
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_DITHER_ENABLE_ENABLE 1

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_3 :: MIN_MIPMAP_LEVEL [15:12] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_MIN_MIPMAP_LEVEL_MASK 0x0000f000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_MIN_MIPMAP_LEVEL_SHIFT 12
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_MIN_MIPMAP_LEVEL_DEFAULT 0x00000000

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_3 :: MAX_MIPMAP_LEVEL [11:08] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_MAX_MIPMAP_LEVEL_MASK 0x00000f00
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_MAX_MIPMAP_LEVEL_SHIFT 8
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_MAX_MIPMAP_LEVEL_DEFAULT 0x00000000

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_3 :: reserved2 [07:07] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_reserved2_MASK   0x00000080
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_reserved2_SHIFT  7

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_3 :: L0_LAYOUT [06:04] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_L0_LAYOUT_MASK   0x00000070
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_L0_LAYOUT_SHIFT  4
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_L0_LAYOUT_DEFAULT 0x00000000

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_3 :: reserved3 [03:02] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_reserved3_MASK   0x0000000c
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_reserved3_SHIFT  2

/* MM_M2MC0 :: OUTPUT_SURFACE_FORMAT_DEF_3 :: CHROMA_FILTER [01:00] */
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_CHROMA_FILTER_MASK 0x00000003
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_CHROMA_FILTER_SHIFT 0
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_CHROMA_FILTER_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_CHROMA_FILTER_FILTER 0
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_CHROMA_FILTER_FIRST 1
#define BCHP_MM_M2MC0_OUTPUT_SURFACE_FORMAT_DEF_3_CHROMA_FILTER_SECOND 2

/***************************************************************************
 *BLIT_HEADER - Blit header and control
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_HEADER :: SRC_SCALER_ENABLE [31:31] */
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_SCALER_ENABLE_MASK           0x80000000
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_SCALER_ENABLE_SHIFT          31
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_SCALER_ENABLE_DEFAULT        0x00000000
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_SCALER_ENABLE_DISABLE        0
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_SCALER_ENABLE_ENABLE         1

/* MM_M2MC0 :: BLIT_HEADER :: reserved0 [30:25] */
#define BCHP_MM_M2MC0_BLIT_HEADER_reserved0_MASK                   0x7e000000
#define BCHP_MM_M2MC0_BLIT_HEADER_reserved0_SHIFT                  25

/* MM_M2MC0 :: BLIT_HEADER :: DEST_COLOR_KEY_ENABLE [24:24] */
#define BCHP_MM_M2MC0_BLIT_HEADER_DEST_COLOR_KEY_ENABLE_MASK       0x01000000
#define BCHP_MM_M2MC0_BLIT_HEADER_DEST_COLOR_KEY_ENABLE_SHIFT      24
#define BCHP_MM_M2MC0_BLIT_HEADER_DEST_COLOR_KEY_ENABLE_DEFAULT    0x00000000
#define BCHP_MM_M2MC0_BLIT_HEADER_DEST_COLOR_KEY_ENABLE_DISABLE    0
#define BCHP_MM_M2MC0_BLIT_HEADER_DEST_COLOR_KEY_ENABLE_ENABLE     1

/* MM_M2MC0 :: BLIT_HEADER :: SRC_COLOR_MATRIX_ROUNDING [23:23] */
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_MATRIX_ROUNDING_MASK   0x00800000
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_MATRIX_ROUNDING_SHIFT  23
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_MATRIX_ROUNDING_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_MATRIX_ROUNDING_TRUNCATE 0
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_MATRIX_ROUNDING_NEAREST 1

/* MM_M2MC0 :: BLIT_HEADER :: SRC_COLOR_MATRIX_ENABLE [22:22] */
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_MATRIX_ENABLE_MASK     0x00400000
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_MATRIX_ENABLE_SHIFT    22
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_MATRIX_ENABLE_DEFAULT  0x00000000
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_MATRIX_ENABLE_DISABLE  0
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_MATRIX_ENABLE_ENABLE   1

/* MM_M2MC0 :: BLIT_HEADER :: SRC_COLOR_KEY_ENABLE [21:21] */
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_KEY_ENABLE_MASK        0x00200000
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_KEY_ENABLE_SHIFT       21
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_KEY_ENABLE_DEFAULT     0x00000000
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_KEY_ENABLE_DISABLE     0
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_KEY_ENABLE_ENABLE      1

/* MM_M2MC0 :: BLIT_HEADER :: DEST_COLOR_KEY_COMPARE [20:20] */
#define BCHP_MM_M2MC0_BLIT_HEADER_DEST_COLOR_KEY_COMPARE_MASK      0x00100000
#define BCHP_MM_M2MC0_BLIT_HEADER_DEST_COLOR_KEY_COMPARE_SHIFT     20
#define BCHP_MM_M2MC0_BLIT_HEADER_DEST_COLOR_KEY_COMPARE_DEFAULT   0x00000000
#define BCHP_MM_M2MC0_BLIT_HEADER_DEST_COLOR_KEY_COMPARE_INCLUSIVE 0
#define BCHP_MM_M2MC0_BLIT_HEADER_DEST_COLOR_KEY_COMPARE_EXCUSIVE  1

/* MM_M2MC0 :: BLIT_HEADER :: SRC_COLOR_KEY_COMPARE [19:19] */
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_KEY_COMPARE_MASK       0x00080000
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_KEY_COMPARE_SHIFT      19
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_KEY_COMPARE_DEFAULT    0x00000000
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_KEY_COMPARE_INCLUSIVE  0
#define BCHP_MM_M2MC0_BLIT_HEADER_SRC_COLOR_KEY_COMPARE_EXCUSIVE   1

/* MM_M2MC0 :: BLIT_HEADER :: CBAR_SRC_COLOR [18:16] */
#define BCHP_MM_M2MC0_BLIT_HEADER_CBAR_SRC_COLOR_MASK              0x00070000
#define BCHP_MM_M2MC0_BLIT_HEADER_CBAR_SRC_COLOR_SHIFT             16
#define BCHP_MM_M2MC0_BLIT_HEADER_CBAR_SRC_COLOR_DEFAULT           0x00000002
#define BCHP_MM_M2MC0_BLIT_HEADER_CBAR_SRC_COLOR_SCALE_THEN_KEY_THEN_MATRIX 0
#define BCHP_MM_M2MC0_BLIT_HEADER_CBAR_SRC_COLOR_SCALE_THEN_MATRIX_THEN_KEY 1
#define BCHP_MM_M2MC0_BLIT_HEADER_CBAR_SRC_COLOR_KEY_THEN_MATRIX_THEN_SCALE 2
#define BCHP_MM_M2MC0_BLIT_HEADER_CBAR_SRC_COLOR_KEY_THEN_SCALE_THEN_MATRIX 3
#define BCHP_MM_M2MC0_BLIT_HEADER_CBAR_SRC_COLOR_MATRIX_THEN_SCALE_THEN_KEY 4
#define BCHP_MM_M2MC0_BLIT_HEADER_CBAR_SRC_COLOR_MATRIX_THEN_KEY_THEN_SCALE 5
#define BCHP_MM_M2MC0_BLIT_HEADER_CBAR_SRC_COLOR_BYPASS_ALL        6

/* MM_M2MC0 :: BLIT_HEADER :: REGISTER_DMA_EVENT_ENABLE [15:15] */
#define BCHP_MM_M2MC0_BLIT_HEADER_REGISTER_DMA_EVENT_ENABLE_MASK   0x00008000
#define BCHP_MM_M2MC0_BLIT_HEADER_REGISTER_DMA_EVENT_ENABLE_SHIFT  15
#define BCHP_MM_M2MC0_BLIT_HEADER_REGISTER_DMA_EVENT_ENABLE_DEFAULT 0x00000000

/* MM_M2MC0 :: BLIT_HEADER :: INTERRUPT_ENABLE [14:14] */
#define BCHP_MM_M2MC0_BLIT_HEADER_INTERRUPT_ENABLE_MASK            0x00004000
#define BCHP_MM_M2MC0_BLIT_HEADER_INTERRUPT_ENABLE_SHIFT           14
#define BCHP_MM_M2MC0_BLIT_HEADER_INTERRUPT_ENABLE_DEFAULT         0x00000000

/* MM_M2MC0 :: BLIT_HEADER :: reserved1 [13:01] */
#define BCHP_MM_M2MC0_BLIT_HEADER_reserved1_MASK                   0x00003ffe
#define BCHP_MM_M2MC0_BLIT_HEADER_reserved1_SHIFT                  1

/* MM_M2MC0 :: BLIT_HEADER :: FOLLOW_SRC_CLUT_PTR [00:00] */
#define BCHP_MM_M2MC0_BLIT_HEADER_FOLLOW_SRC_CLUT_PTR_MASK         0x00000001
#define BCHP_MM_M2MC0_BLIT_HEADER_FOLLOW_SRC_CLUT_PTR_SHIFT        0
#define BCHP_MM_M2MC0_BLIT_HEADER_FOLLOW_SRC_CLUT_PTR_DEFAULT      0x00000000
#define BCHP_MM_M2MC0_BLIT_HEADER_FOLLOW_SRC_CLUT_PTR_DISABLE      0
#define BCHP_MM_M2MC0_BLIT_HEADER_FOLLOW_SRC_CLUT_PTR_ENABLE       1

/***************************************************************************
 *BLIT_SRC_TOP_LEFT_0 - Top left pixel coordinate in the Source surface 0.
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_SRC_TOP_LEFT_0 :: reserved0 [31:29] */
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_0_reserved0_MASK           0xe0000000
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_0_reserved0_SHIFT          29

/* MM_M2MC0 :: BLIT_SRC_TOP_LEFT_0 :: TOP [28:16] */
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_0_TOP_MASK                 0x1fff0000
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_0_TOP_SHIFT                16
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_0_TOP_DEFAULT              0x00000000

/* MM_M2MC0 :: BLIT_SRC_TOP_LEFT_0 :: reserved1 [15:13] */
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_0_reserved1_MASK           0x0000e000
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_0_reserved1_SHIFT          13

/* MM_M2MC0 :: BLIT_SRC_TOP_LEFT_0 :: LEFT [12:00] */
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_0_LEFT_MASK                0x00001fff
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_0_LEFT_SHIFT               0
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_0_LEFT_DEFAULT             0x00000000

/***************************************************************************
 *BLIT_SRC_SIZE_0 - Pixel width and height of operation in Source surface 0.
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_SRC_SIZE_0 :: reserved0 [31:29] */
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_0_reserved0_MASK               0xe0000000
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_0_reserved0_SHIFT              29

/* MM_M2MC0 :: BLIT_SRC_SIZE_0 :: SURFACE_WIDTH [28:16] */
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_0_SURFACE_WIDTH_MASK           0x1fff0000
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_0_SURFACE_WIDTH_SHIFT          16
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_0_SURFACE_WIDTH_DEFAULT        0x00000001

/* MM_M2MC0 :: BLIT_SRC_SIZE_0 :: reserved1 [15:13] */
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_0_reserved1_MASK               0x0000e000
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_0_reserved1_SHIFT              13

/* MM_M2MC0 :: BLIT_SRC_SIZE_0 :: SURFACE_HEIGHT [12:00] */
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_0_SURFACE_HEIGHT_MASK          0x00001fff
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_0_SURFACE_HEIGHT_SHIFT         0
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_0_SURFACE_HEIGHT_DEFAULT       0x00000001

/***************************************************************************
 *BLIT_SRC_UIF_FULL_HEIGHT - Source UIF Full Vertical Size
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_SRC_UIF_FULL_HEIGHT :: reserved0 [31:14] */
#define BCHP_MM_M2MC0_BLIT_SRC_UIF_FULL_HEIGHT_reserved0_MASK      0xffffc000
#define BCHP_MM_M2MC0_BLIT_SRC_UIF_FULL_HEIGHT_reserved0_SHIFT     14

/* MM_M2MC0 :: BLIT_SRC_UIF_FULL_HEIGHT :: FULL_HEIGHT [13:00] */
#define BCHP_MM_M2MC0_BLIT_SRC_UIF_FULL_HEIGHT_FULL_HEIGHT_MASK    0x00003fff
#define BCHP_MM_M2MC0_BLIT_SRC_UIF_FULL_HEIGHT_FULL_HEIGHT_SHIFT   0
#define BCHP_MM_M2MC0_BLIT_SRC_UIF_FULL_HEIGHT_FULL_HEIGHT_DEFAULT 0x00000000

/***************************************************************************
 *BLIT_SRC_TOP_LEFT_1 - Top left pixel coordinate in the Source surface 1.
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_SRC_TOP_LEFT_1 :: reserved0 [31:29] */
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_1_reserved0_MASK           0xe0000000
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_1_reserved0_SHIFT          29

/* MM_M2MC0 :: BLIT_SRC_TOP_LEFT_1 :: TOP [28:16] */
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_1_TOP_MASK                 0x1fff0000
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_1_TOP_SHIFT                16
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_1_TOP_DEFAULT              0x00000000

/* MM_M2MC0 :: BLIT_SRC_TOP_LEFT_1 :: reserved1 [15:13] */
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_1_reserved1_MASK           0x0000e000
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_1_reserved1_SHIFT          13

/* MM_M2MC0 :: BLIT_SRC_TOP_LEFT_1 :: LEFT [12:00] */
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_1_LEFT_MASK                0x00001fff
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_1_LEFT_SHIFT               0
#define BCHP_MM_M2MC0_BLIT_SRC_TOP_LEFT_1_LEFT_DEFAULT             0x00000000

/***************************************************************************
 *BLIT_SRC_SIZE_1 - Pixel width and height of operation in Source surface 1.
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_SRC_SIZE_1 :: reserved0 [31:29] */
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_1_reserved0_MASK               0xe0000000
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_1_reserved0_SHIFT              29

/* MM_M2MC0 :: BLIT_SRC_SIZE_1 :: SURFACE_WIDTH [28:16] */
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_1_SURFACE_WIDTH_MASK           0x1fff0000
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_1_SURFACE_WIDTH_SHIFT          16
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_1_SURFACE_WIDTH_DEFAULT        0x00000001

/* MM_M2MC0 :: BLIT_SRC_SIZE_1 :: reserved1 [15:13] */
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_1_reserved1_MASK               0x0000e000
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_1_reserved1_SHIFT              13

/* MM_M2MC0 :: BLIT_SRC_SIZE_1 :: SURFACE_HEIGHT [12:00] */
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_1_SURFACE_HEIGHT_MASK          0x00001fff
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_1_SURFACE_HEIGHT_SHIFT         0
#define BCHP_MM_M2MC0_BLIT_SRC_SIZE_1_SURFACE_HEIGHT_DEFAULT       0x00000001

/***************************************************************************
 *BLIT_SRC_STRIPE_HEIGHT_WIDTH_0 - width and height of stripe for surface 0 for the decoded frame image format.
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_SRC_STRIPE_HEIGHT_WIDTH_0 :: reserved0 [31:30] */
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_reserved0_MASK 0xc0000000
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_reserved0_SHIFT 30

/* MM_M2MC0 :: BLIT_SRC_STRIPE_HEIGHT_WIDTH_0 :: STRIPE_HEIGHT [29:16] */
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_STRIPE_HEIGHT_MASK 0x3fff0000
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_STRIPE_HEIGHT_SHIFT 16
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_STRIPE_HEIGHT_DEFAULT 0x00000040

/* MM_M2MC0 :: BLIT_SRC_STRIPE_HEIGHT_WIDTH_0 :: reserved1 [15:02] */
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_reserved1_MASK 0x0000fffc
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_reserved1_SHIFT 2

/* MM_M2MC0 :: BLIT_SRC_STRIPE_HEIGHT_WIDTH_0 :: STRIPE_WIDTH [01:00] */
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_STRIPE_WIDTH_MASK 0x00000003
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_STRIPE_WIDTH_SHIFT 0
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_0_STRIPE_WIDTH_DEFAULT 0x00000000

/***************************************************************************
 *BLIT_SRC_STRIPE_HEIGHT_WIDTH_1 - width and height of stripe for surface 1 for the decoded frame image format.
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_SRC_STRIPE_HEIGHT_WIDTH_1 :: reserved0 [31:30] */
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1_reserved0_MASK 0xc0000000
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1_reserved0_SHIFT 30

/* MM_M2MC0 :: BLIT_SRC_STRIPE_HEIGHT_WIDTH_1 :: STRIPE_HEIGHT [29:16] */
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1_STRIPE_HEIGHT_MASK 0x3fff0000
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1_STRIPE_HEIGHT_SHIFT 16
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1_STRIPE_HEIGHT_DEFAULT 0x00000040

/* MM_M2MC0 :: BLIT_SRC_STRIPE_HEIGHT_WIDTH_1 :: reserved1 [15:02] */
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1_reserved1_MASK 0x0000fffc
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1_reserved1_SHIFT 2

/* MM_M2MC0 :: BLIT_SRC_STRIPE_HEIGHT_WIDTH_1 :: STRIPE_WIDTH [01:00] */
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1_STRIPE_WIDTH_MASK 0x00000003
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1_STRIPE_WIDTH_SHIFT 0
#define BCHP_MM_M2MC0_BLIT_SRC_STRIPE_HEIGHT_WIDTH_1_STRIPE_WIDTH_DEFAULT 0x00000000

/***************************************************************************
 *SCRATCH_LIST_1 - M2MC Scratch register (Included in DMA list structure)
 ***************************************************************************/
/* MM_M2MC0 :: SCRATCH_LIST_1 :: VALUE [31:00] */
#define BCHP_MM_M2MC0_SCRATCH_LIST_1_VALUE_MASK                    0xffffffff
#define BCHP_MM_M2MC0_SCRATCH_LIST_1_VALUE_SHIFT                   0
#define BCHP_MM_M2MC0_SCRATCH_LIST_1_VALUE_DEFAULT                 0x00000000

/***************************************************************************
 *SCRATCH_LIST_2 - M2MC Scratch register (Included in DMA list structure)
 ***************************************************************************/
/* MM_M2MC0 :: SCRATCH_LIST_2 :: VALUE [31:00] */
#define BCHP_MM_M2MC0_SCRATCH_LIST_2_VALUE_MASK                    0xffffffff
#define BCHP_MM_M2MC0_SCRATCH_LIST_2_VALUE_SHIFT                   0
#define BCHP_MM_M2MC0_SCRATCH_LIST_2_VALUE_DEFAULT                 0x00000000

/***************************************************************************
 *SCRATCH_LIST_3 - M2MC Scratch register (Included in DMA list structure)
 ***************************************************************************/
/* MM_M2MC0 :: SCRATCH_LIST_3 :: VALUE [31:00] */
#define BCHP_MM_M2MC0_SCRATCH_LIST_3_VALUE_MASK                    0xffffffff
#define BCHP_MM_M2MC0_SCRATCH_LIST_3_VALUE_SHIFT                   0
#define BCHP_MM_M2MC0_SCRATCH_LIST_3_VALUE_DEFAULT                 0x00000000

/***************************************************************************
 *BLIT_OUTPUT_TOP_LEFT - Top left pixel coordinate in the Output
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_OUTPUT_TOP_LEFT :: reserved0 [31:29] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_TOP_LEFT_reserved0_MASK          0xe0000000
#define BCHP_MM_M2MC0_BLIT_OUTPUT_TOP_LEFT_reserved0_SHIFT         29

/* MM_M2MC0 :: BLIT_OUTPUT_TOP_LEFT :: TOP [28:16] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_TOP_LEFT_TOP_MASK                0x1fff0000
#define BCHP_MM_M2MC0_BLIT_OUTPUT_TOP_LEFT_TOP_SHIFT               16
#define BCHP_MM_M2MC0_BLIT_OUTPUT_TOP_LEFT_TOP_DEFAULT             0x00000000

/* MM_M2MC0 :: BLIT_OUTPUT_TOP_LEFT :: reserved1 [15:13] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_TOP_LEFT_reserved1_MASK          0x0000e000
#define BCHP_MM_M2MC0_BLIT_OUTPUT_TOP_LEFT_reserved1_SHIFT         13

/* MM_M2MC0 :: BLIT_OUTPUT_TOP_LEFT :: LEFT [12:00] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_TOP_LEFT_LEFT_MASK               0x00001fff
#define BCHP_MM_M2MC0_BLIT_OUTPUT_TOP_LEFT_LEFT_SHIFT              0
#define BCHP_MM_M2MC0_BLIT_OUTPUT_TOP_LEFT_LEFT_DEFAULT            0x00000000

/***************************************************************************
 *BLIT_OUTPUT_SIZE - Pixel width and height of operation in the Output
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_OUTPUT_SIZE :: reserved0 [31:29] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_SIZE_reserved0_MASK              0xe0000000
#define BCHP_MM_M2MC0_BLIT_OUTPUT_SIZE_reserved0_SHIFT             29

/* MM_M2MC0 :: BLIT_OUTPUT_SIZE :: SURFACE_WIDTH [28:16] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_SIZE_SURFACE_WIDTH_MASK          0x1fff0000
#define BCHP_MM_M2MC0_BLIT_OUTPUT_SIZE_SURFACE_WIDTH_SHIFT         16
#define BCHP_MM_M2MC0_BLIT_OUTPUT_SIZE_SURFACE_WIDTH_DEFAULT       0x00000001

/* MM_M2MC0 :: BLIT_OUTPUT_SIZE :: reserved1 [15:13] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_SIZE_reserved1_MASK              0x0000e000
#define BCHP_MM_M2MC0_BLIT_OUTPUT_SIZE_reserved1_SHIFT             13

/* MM_M2MC0 :: BLIT_OUTPUT_SIZE :: SURFACE_HEIGHT [12:00] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_SIZE_SURFACE_HEIGHT_MASK         0x00001fff
#define BCHP_MM_M2MC0_BLIT_OUTPUT_SIZE_SURFACE_HEIGHT_SHIFT        0
#define BCHP_MM_M2MC0_BLIT_OUTPUT_SIZE_SURFACE_HEIGHT_DEFAULT      0x00000001

/***************************************************************************
 *BLIT_OUTPUT_UIF_FULL_HEIGHT - Output UIF Full Vertical Size
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_OUTPUT_UIF_FULL_HEIGHT :: reserved0 [31:14] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_UIF_FULL_HEIGHT_reserved0_MASK   0xffffc000
#define BCHP_MM_M2MC0_BLIT_OUTPUT_UIF_FULL_HEIGHT_reserved0_SHIFT  14

/* MM_M2MC0 :: BLIT_OUTPUT_UIF_FULL_HEIGHT :: FULL_HEIGHT [13:00] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_UIF_FULL_HEIGHT_FULL_HEIGHT_MASK 0x00003fff
#define BCHP_MM_M2MC0_BLIT_OUTPUT_UIF_FULL_HEIGHT_FULL_HEIGHT_SHIFT 0
#define BCHP_MM_M2MC0_BLIT_OUTPUT_UIF_FULL_HEIGHT_FULL_HEIGHT_DEFAULT 0x00000000

/***************************************************************************
 *BLIT_INPUT_STRIPE_WIDTH_0 - Pixel width of input stripe when striping for surface 0
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_INPUT_STRIPE_WIDTH_0 :: reserved0 [31:28] */
#define BCHP_MM_M2MC0_BLIT_INPUT_STRIPE_WIDTH_0_reserved0_MASK     0xf0000000
#define BCHP_MM_M2MC0_BLIT_INPUT_STRIPE_WIDTH_0_reserved0_SHIFT    28

/* MM_M2MC0 :: BLIT_INPUT_STRIPE_WIDTH_0 :: STRIPE_WIDTH [27:00] */
#define BCHP_MM_M2MC0_BLIT_INPUT_STRIPE_WIDTH_0_STRIPE_WIDTH_MASK  0x0fffffff
#define BCHP_MM_M2MC0_BLIT_INPUT_STRIPE_WIDTH_0_STRIPE_WIDTH_SHIFT 0

/***************************************************************************
 *BLIT_INPUT_STRIPE_WIDTH_1 - Pixel width of input stripe when striping for surface 1
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_INPUT_STRIPE_WIDTH_1 :: reserved0 [31:28] */
#define BCHP_MM_M2MC0_BLIT_INPUT_STRIPE_WIDTH_1_reserved0_MASK     0xf0000000
#define BCHP_MM_M2MC0_BLIT_INPUT_STRIPE_WIDTH_1_reserved0_SHIFT    28

/* MM_M2MC0 :: BLIT_INPUT_STRIPE_WIDTH_1 :: STRIPE_WIDTH [27:00] */
#define BCHP_MM_M2MC0_BLIT_INPUT_STRIPE_WIDTH_1_STRIPE_WIDTH_MASK  0x0fffffff
#define BCHP_MM_M2MC0_BLIT_INPUT_STRIPE_WIDTH_1_STRIPE_WIDTH_SHIFT 0

/***************************************************************************
 *BLIT_OUTPUT_STRIPE_WIDTH - Pixel width of output stripe when striping
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_OUTPUT_STRIPE_WIDTH :: reserved0 [31:12] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_STRIPE_WIDTH_reserved0_MASK      0xfffff000
#define BCHP_MM_M2MC0_BLIT_OUTPUT_STRIPE_WIDTH_reserved0_SHIFT     12

/* MM_M2MC0 :: BLIT_OUTPUT_STRIPE_WIDTH :: STRIPE_WIDTH [11:00] */
#define BCHP_MM_M2MC0_BLIT_OUTPUT_STRIPE_WIDTH_STRIPE_WIDTH_MASK   0x00000fff
#define BCHP_MM_M2MC0_BLIT_OUTPUT_STRIPE_WIDTH_STRIPE_WIDTH_SHIFT  0
#define BCHP_MM_M2MC0_BLIT_OUTPUT_STRIPE_WIDTH_STRIPE_WIDTH_DEFAULT 0x00000040

/***************************************************************************
 *BLIT_STRIPE_OVERLAP_0 - Pixel width of stripe overlap when striping for surface 0.
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_STRIPE_OVERLAP_0 :: reserved0 [31:12] */
#define BCHP_MM_M2MC0_BLIT_STRIPE_OVERLAP_0_reserved0_MASK         0xfffff000
#define BCHP_MM_M2MC0_BLIT_STRIPE_OVERLAP_0_reserved0_SHIFT        12

/* MM_M2MC0 :: BLIT_STRIPE_OVERLAP_0 :: STRIPE_WIDTH [11:00] */
#define BCHP_MM_M2MC0_BLIT_STRIPE_OVERLAP_0_STRIPE_WIDTH_MASK      0x00000fff
#define BCHP_MM_M2MC0_BLIT_STRIPE_OVERLAP_0_STRIPE_WIDTH_SHIFT     0
#define BCHP_MM_M2MC0_BLIT_STRIPE_OVERLAP_0_STRIPE_WIDTH_DEFAULT   0x00000040

/***************************************************************************
 *BLIT_STRIPE_OVERLAP_1 - Pixel width of stripe overlap when striping for surface 1.
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_STRIPE_OVERLAP_1 :: reserved0 [31:12] */
#define BCHP_MM_M2MC0_BLIT_STRIPE_OVERLAP_1_reserved0_MASK         0xfffff000
#define BCHP_MM_M2MC0_BLIT_STRIPE_OVERLAP_1_reserved0_SHIFT        12

/* MM_M2MC0 :: BLIT_STRIPE_OVERLAP_1 :: STRIPE_WIDTH [11:00] */
#define BCHP_MM_M2MC0_BLIT_STRIPE_OVERLAP_1_STRIPE_WIDTH_MASK      0x00000fff
#define BCHP_MM_M2MC0_BLIT_STRIPE_OVERLAP_1_STRIPE_WIDTH_SHIFT     0
#define BCHP_MM_M2MC0_BLIT_STRIPE_OVERLAP_1_STRIPE_WIDTH_DEFAULT   0x00000040

/***************************************************************************
 *BLIT_CTRL - Blit control
 ***************************************************************************/
/* MM_M2MC0 :: BLIT_CTRL :: reserved0 [31:26] */
#define BCHP_MM_M2MC0_BLIT_CTRL_reserved0_MASK                     0xfc000000
#define BCHP_MM_M2MC0_BLIT_CTRL_reserved0_SHIFT                    26

/* MM_M2MC0 :: BLIT_CTRL :: reserved1 [25:25] */
#define BCHP_MM_M2MC0_BLIT_CTRL_reserved1_MASK                     0x02000000
#define BCHP_MM_M2MC0_BLIT_CTRL_reserved1_SHIFT                    25

/* MM_M2MC0 :: BLIT_CTRL :: MWORD_ALLIGN_DISABLE [24:24] */
#define BCHP_MM_M2MC0_BLIT_CTRL_MWORD_ALLIGN_DISABLE_MASK          0x01000000
#define BCHP_MM_M2MC0_BLIT_CTRL_MWORD_ALLIGN_DISABLE_SHIFT         24
#define BCHP_MM_M2MC0_BLIT_CTRL_MWORD_ALLIGN_DISABLE_DEFAULT       0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_MWORD_ALLIGN_DISABLE_DISABLE       0
#define BCHP_MM_M2MC0_BLIT_CTRL_MWORD_ALLIGN_DISABLE_ENABLE        1

/* MM_M2MC0 :: BLIT_CTRL :: STRIPE_WIDTH_OVERRIDE [23:23] */
#define BCHP_MM_M2MC0_BLIT_CTRL_STRIPE_WIDTH_OVERRIDE_MASK         0x00800000
#define BCHP_MM_M2MC0_BLIT_CTRL_STRIPE_WIDTH_OVERRIDE_SHIFT        23
#define BCHP_MM_M2MC0_BLIT_CTRL_STRIPE_WIDTH_OVERRIDE_DEFAULT      0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_STRIPE_WIDTH_OVERRIDE_DISABLE      0
#define BCHP_MM_M2MC0_BLIT_CTRL_STRIPE_WIDTH_OVERRIDE_ENABLE       1

/* MM_M2MC0 :: BLIT_CTRL :: READ_420_AS_FIELDS [22:22] */
#define BCHP_MM_M2MC0_BLIT_CTRL_READ_420_AS_FIELDS_MASK            0x00400000
#define BCHP_MM_M2MC0_BLIT_CTRL_READ_420_AS_FIELDS_SHIFT           22
#define BCHP_MM_M2MC0_BLIT_CTRL_READ_420_AS_FIELDS_DEFAULT         0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_READ_420_AS_FIELDS_DISABLE         0
#define BCHP_MM_M2MC0_BLIT_CTRL_READ_420_AS_FIELDS_ENABLE          1

/* MM_M2MC0 :: BLIT_CTRL :: ALPHA_DE_MULTIPLY_ENABLE [21:21] */
#define BCHP_MM_M2MC0_BLIT_CTRL_ALPHA_DE_MULTIPLY_ENABLE_MASK      0x00200000
#define BCHP_MM_M2MC0_BLIT_CTRL_ALPHA_DE_MULTIPLY_ENABLE_SHIFT     21
#define BCHP_MM_M2MC0_BLIT_CTRL_ALPHA_DE_MULTIPLY_ENABLE_DEFAULT   0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_ALPHA_DE_MULTIPLY_ENABLE_DISABLE   0
#define BCHP_MM_M2MC0_BLIT_CTRL_ALPHA_DE_MULTIPLY_ENABLE_ENABLE    1

/* MM_M2MC0 :: BLIT_CTRL :: DEST_ALPHA_PRE_MULTIPLY_ENABLE [20:20] */
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_ALPHA_PRE_MULTIPLY_ENABLE_MASK 0x00100000
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_ALPHA_PRE_MULTIPLY_ENABLE_SHIFT 20
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_ALPHA_PRE_MULTIPLY_ENABLE_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_ALPHA_PRE_MULTIPLY_ENABLE_DISABLE 0
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_ALPHA_PRE_MULTIPLY_ENABLE_ENABLE 1

/* MM_M2MC0 :: BLIT_CTRL :: DEST_ALPHA_PRE_MULTIPLY_OFFSET_EN [19:19] */
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_ALPHA_PRE_MULTIPLY_OFFSET_EN_MASK 0x00080000
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_ALPHA_PRE_MULTIPLY_OFFSET_EN_SHIFT 19
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_ALPHA_PRE_MULTIPLY_OFFSET_EN_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_ALPHA_PRE_MULTIPLY_OFFSET_EN_DISABLE 0
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_ALPHA_PRE_MULTIPLY_OFFSET_EN_ENABLE 1

/* MM_M2MC0 :: BLIT_CTRL :: DEST_CBAR_SELECT [18:18] */
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_CBAR_SELECT_MASK              0x00040000
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_CBAR_SELECT_SHIFT             18
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_CBAR_SELECT_DEFAULT           0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_CBAR_SELECT_CKEY_THEN_ALPHA_PREMULT 0
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_CBAR_SELECT_ALPHA_PREMULT_THEN_CKEY 1

/* MM_M2MC0 :: BLIT_CTRL :: BLOCK_AUTO_SPLIT_FIFO [17:17] */
#define BCHP_MM_M2MC0_BLIT_CTRL_BLOCK_AUTO_SPLIT_FIFO_MASK         0x00020000
#define BCHP_MM_M2MC0_BLIT_CTRL_BLOCK_AUTO_SPLIT_FIFO_SHIFT        17
#define BCHP_MM_M2MC0_BLIT_CTRL_BLOCK_AUTO_SPLIT_FIFO_DEFAULT      0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_BLOCK_AUTO_SPLIT_FIFO_DISABLE      0
#define BCHP_MM_M2MC0_BLIT_CTRL_BLOCK_AUTO_SPLIT_FIFO_ENABLE       1

/* MM_M2MC0 :: BLIT_CTRL :: STRIPE_ENABLE [16:16] */
#define BCHP_MM_M2MC0_BLIT_CTRL_STRIPE_ENABLE_MASK                 0x00010000
#define BCHP_MM_M2MC0_BLIT_CTRL_STRIPE_ENABLE_SHIFT                16
#define BCHP_MM_M2MC0_BLIT_CTRL_STRIPE_ENABLE_DEFAULT              0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_STRIPE_ENABLE_DISABLE              0
#define BCHP_MM_M2MC0_BLIT_CTRL_STRIPE_ENABLE_ENABLE               1

/* MM_M2MC0 :: BLIT_CTRL :: reserved2 [15:06] */
#define BCHP_MM_M2MC0_BLIT_CTRL_reserved2_MASK                     0x0000ffc0
#define BCHP_MM_M2MC0_BLIT_CTRL_reserved2_SHIFT                    6

/* MM_M2MC0 :: BLIT_CTRL :: OUTPUT_V_DIRECTION [05:05] */
#define BCHP_MM_M2MC0_BLIT_CTRL_OUTPUT_V_DIRECTION_MASK            0x00000020
#define BCHP_MM_M2MC0_BLIT_CTRL_OUTPUT_V_DIRECTION_SHIFT           5
#define BCHP_MM_M2MC0_BLIT_CTRL_OUTPUT_V_DIRECTION_DEFAULT         0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_OUTPUT_V_DIRECTION_BOTTOM_TO_TOP   1
#define BCHP_MM_M2MC0_BLIT_CTRL_OUTPUT_V_DIRECTION_TOP_TO_BOTTOM   0

/* MM_M2MC0 :: BLIT_CTRL :: OUTPUT_H_DIRECTION [04:04] */
#define BCHP_MM_M2MC0_BLIT_CTRL_OUTPUT_H_DIRECTION_MASK            0x00000010
#define BCHP_MM_M2MC0_BLIT_CTRL_OUTPUT_H_DIRECTION_SHIFT           4
#define BCHP_MM_M2MC0_BLIT_CTRL_OUTPUT_H_DIRECTION_DEFAULT         0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_OUTPUT_H_DIRECTION_RIGHT_TO_LEFT   1
#define BCHP_MM_M2MC0_BLIT_CTRL_OUTPUT_H_DIRECTION_LEFT_TO_RIGHT   0

/* MM_M2MC0 :: BLIT_CTRL :: DEST_V_DIRECTION [03:03] */
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_V_DIRECTION_MASK              0x00000008
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_V_DIRECTION_SHIFT             3
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_V_DIRECTION_DEFAULT           0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_V_DIRECTION_BOTTOM_TO_TOP     1
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_V_DIRECTION_TOP_TO_BOTTOM     0

/* MM_M2MC0 :: BLIT_CTRL :: DEST_H_DIRECTION [02:02] */
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_H_DIRECTION_MASK              0x00000004
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_H_DIRECTION_SHIFT             2
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_H_DIRECTION_DEFAULT           0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_H_DIRECTION_RIGHT_TO_LEFT     1
#define BCHP_MM_M2MC0_BLIT_CTRL_DEST_H_DIRECTION_LEFT_TO_RIGHT     0

/* MM_M2MC0 :: BLIT_CTRL :: SRC_V_DIRECTION [01:01] */
#define BCHP_MM_M2MC0_BLIT_CTRL_SRC_V_DIRECTION_MASK               0x00000002
#define BCHP_MM_M2MC0_BLIT_CTRL_SRC_V_DIRECTION_SHIFT              1
#define BCHP_MM_M2MC0_BLIT_CTRL_SRC_V_DIRECTION_DEFAULT            0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_SRC_V_DIRECTION_BOTTOM_TO_TOP      1
#define BCHP_MM_M2MC0_BLIT_CTRL_SRC_V_DIRECTION_TOP_TO_BOTTOM      0

/* MM_M2MC0 :: BLIT_CTRL :: SRC_H_DIRECTION [00:00] */
#define BCHP_MM_M2MC0_BLIT_CTRL_SRC_H_DIRECTION_MASK               0x00000001
#define BCHP_MM_M2MC0_BLIT_CTRL_SRC_H_DIRECTION_SHIFT              0
#define BCHP_MM_M2MC0_BLIT_CTRL_SRC_H_DIRECTION_DEFAULT            0x00000000
#define BCHP_MM_M2MC0_BLIT_CTRL_SRC_H_DIRECTION_RIGHT_TO_LEFT      1
#define BCHP_MM_M2MC0_BLIT_CTRL_SRC_H_DIRECTION_LEFT_TO_RIGHT      0

/***************************************************************************
 *SCALER_CTRL - Scaler control
 ***************************************************************************/
/* MM_M2MC0 :: SCALER_CTRL :: reserved0 [31:29] */
#define BCHP_MM_M2MC0_SCALER_CTRL_reserved0_MASK                   0xe0000000
#define BCHP_MM_M2MC0_SCALER_CTRL_reserved0_SHIFT                  29

/* MM_M2MC0 :: SCALER_CTRL :: ALPHA_PRE_MULTIPLY_ENABLE [28:28] */
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_ENABLE_MASK   0x10000000
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_ENABLE_SHIFT  28
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_ENABLE_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_ENABLE_DISABLE 0
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_ENABLE_ENABLE 1

/* MM_M2MC0 :: SCALER_CTRL :: ALPHA_PRE_MULTIPLY_OFFSET_EN [27:27] */
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_OFFSET_EN_MASK 0x08000000
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_OFFSET_EN_SHIFT 27
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_OFFSET_EN_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_OFFSET_EN_DISABLE 0
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_OFFSET_EN_ENABLE 1

/* MM_M2MC0 :: SCALER_CTRL :: CLUT_SCALE_MODE [26:26] */
#define BCHP_MM_M2MC0_SCALER_CTRL_CLUT_SCALE_MODE_MASK             0x04000000
#define BCHP_MM_M2MC0_SCALER_CTRL_CLUT_SCALE_MODE_SHIFT            26
#define BCHP_MM_M2MC0_SCALER_CTRL_CLUT_SCALE_MODE_DEFAULT          0x00000000
#define BCHP_MM_M2MC0_SCALER_CTRL_CLUT_SCALE_MODE_DISABLE          0
#define BCHP_MM_M2MC0_SCALER_CTRL_CLUT_SCALE_MODE_ENABLE           1

/* MM_M2MC0 :: SCALER_CTRL :: ALPHA_PRE_MULTIPLY [25:25] */
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_MASK          0x02000000
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_SHIFT         25
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_DEFAULT       0x00000000
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_DISABLE       0
#define BCHP_MM_M2MC0_SCALER_CTRL_ALPHA_PRE_MULTIPLY_ENABLE        1

/* MM_M2MC0 :: SCALER_CTRL :: OFFSET_ADJUST [24:24] */
#define BCHP_MM_M2MC0_SCALER_CTRL_OFFSET_ADJUST_MASK               0x01000000
#define BCHP_MM_M2MC0_SCALER_CTRL_OFFSET_ADJUST_SHIFT              24
#define BCHP_MM_M2MC0_SCALER_CTRL_OFFSET_ADJUST_DEFAULT            0x00000000
#define BCHP_MM_M2MC0_SCALER_CTRL_OFFSET_ADJUST_DISABLE            0
#define BCHP_MM_M2MC0_SCALER_CTRL_OFFSET_ADJUST_ENABLE             1

/* MM_M2MC0 :: SCALER_CTRL :: reserved1 [23:17] */
#define BCHP_MM_M2MC0_SCALER_CTRL_reserved1_MASK                   0x00fe0000
#define BCHP_MM_M2MC0_SCALER_CTRL_reserved1_SHIFT                  17

/* MM_M2MC0 :: SCALER_CTRL :: ROUNDING_MODE [16:16] */
#define BCHP_MM_M2MC0_SCALER_CTRL_ROUNDING_MODE_MASK               0x00010000
#define BCHP_MM_M2MC0_SCALER_CTRL_ROUNDING_MODE_SHIFT              16
#define BCHP_MM_M2MC0_SCALER_CTRL_ROUNDING_MODE_DEFAULT            0x00000000
#define BCHP_MM_M2MC0_SCALER_CTRL_ROUNDING_MODE_TRUNCATE           0
#define BCHP_MM_M2MC0_SCALER_CTRL_ROUNDING_MODE_NEAREST            1

/* MM_M2MC0 :: SCALER_CTRL :: reserved2 [15:05] */
#define BCHP_MM_M2MC0_SCALER_CTRL_reserved2_MASK                   0x0000ffe0
#define BCHP_MM_M2MC0_SCALER_CTRL_reserved2_SHIFT                  5

/* MM_M2MC0 :: SCALER_CTRL :: HORIZ_SCALER_ENABLE [04:04] */
#define BCHP_MM_M2MC0_SCALER_CTRL_HORIZ_SCALER_ENABLE_MASK         0x00000010
#define BCHP_MM_M2MC0_SCALER_CTRL_HORIZ_SCALER_ENABLE_SHIFT        4
#define BCHP_MM_M2MC0_SCALER_CTRL_HORIZ_SCALER_ENABLE_DEFAULT      0x00000000
#define BCHP_MM_M2MC0_SCALER_CTRL_HORIZ_SCALER_ENABLE_DISABLE      0
#define BCHP_MM_M2MC0_SCALER_CTRL_HORIZ_SCALER_ENABLE_ENABLE       1

/* MM_M2MC0 :: SCALER_CTRL :: reserved3 [03:03] */
#define BCHP_MM_M2MC0_SCALER_CTRL_reserved3_MASK                   0x00000008
#define BCHP_MM_M2MC0_SCALER_CTRL_reserved3_SHIFT                  3

/* MM_M2MC0 :: SCALER_CTRL :: VERT_SCALER_ENABLE [02:02] */
#define BCHP_MM_M2MC0_SCALER_CTRL_VERT_SCALER_ENABLE_MASK          0x00000004
#define BCHP_MM_M2MC0_SCALER_CTRL_VERT_SCALER_ENABLE_SHIFT         2
#define BCHP_MM_M2MC0_SCALER_CTRL_VERT_SCALER_ENABLE_DEFAULT       0x00000000
#define BCHP_MM_M2MC0_SCALER_CTRL_VERT_SCALER_ENABLE_DISABLE       0
#define BCHP_MM_M2MC0_SCALER_CTRL_VERT_SCALER_ENABLE_ENABLE        1

/* MM_M2MC0 :: SCALER_CTRL :: SCALER_ORDER [01:01] */
#define BCHP_MM_M2MC0_SCALER_CTRL_SCALER_ORDER_MASK                0x00000002
#define BCHP_MM_M2MC0_SCALER_CTRL_SCALER_ORDER_SHIFT               1
#define BCHP_MM_M2MC0_SCALER_CTRL_SCALER_ORDER_DEFAULT             0x00000000
#define BCHP_MM_M2MC0_SCALER_CTRL_SCALER_ORDER_VERT_THEN_HORIZ     0
#define BCHP_MM_M2MC0_SCALER_CTRL_SCALER_ORDER_HORIZ_THEN_VERT     1

/* MM_M2MC0 :: SCALER_CTRL :: reserved4 [00:00] */
#define BCHP_MM_M2MC0_SCALER_CTRL_reserved4_MASK                   0x00000001
#define BCHP_MM_M2MC0_SCALER_CTRL_reserved4_SHIFT                  0

/***************************************************************************
 *HORIZ_AVERAGER_COUNT - Horizontal averager control count
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_AVERAGER_COUNT :: reserved0 [31:01] */
#define BCHP_MM_M2MC0_HORIZ_AVERAGER_COUNT_reserved0_MASK          0xfffffffe
#define BCHP_MM_M2MC0_HORIZ_AVERAGER_COUNT_reserved0_SHIFT         1

/* MM_M2MC0 :: HORIZ_AVERAGER_COUNT :: DUMMY_COUNT [00:00] */
#define BCHP_MM_M2MC0_HORIZ_AVERAGER_COUNT_DUMMY_COUNT_MASK        0x00000001
#define BCHP_MM_M2MC0_HORIZ_AVERAGER_COUNT_DUMMY_COUNT_SHIFT       0

/***************************************************************************
 *HORIZ_AVERAGER_COEFF - Horizontal averager control coefficient
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_AVERAGER_COEFF :: reserved0 [31:01] */
#define BCHP_MM_M2MC0_HORIZ_AVERAGER_COEFF_reserved0_MASK          0xfffffffe
#define BCHP_MM_M2MC0_HORIZ_AVERAGER_COEFF_reserved0_SHIFT         1

/* MM_M2MC0 :: HORIZ_AVERAGER_COEFF :: DUMMY_COEFF [00:00] */
#define BCHP_MM_M2MC0_HORIZ_AVERAGER_COEFF_DUMMY_COEFF_MASK        0x00000001
#define BCHP_MM_M2MC0_HORIZ_AVERAGER_COEFF_DUMMY_COEFF_SHIFT       0

/***************************************************************************
 *VERT_AVERAGER_COUNT - Vertical averager control count
 ***************************************************************************/
/* MM_M2MC0 :: VERT_AVERAGER_COUNT :: reserved0 [31:01] */
#define BCHP_MM_M2MC0_VERT_AVERAGER_COUNT_reserved0_MASK           0xfffffffe
#define BCHP_MM_M2MC0_VERT_AVERAGER_COUNT_reserved0_SHIFT          1

/* MM_M2MC0 :: VERT_AVERAGER_COUNT :: DUMMY_COUNT [00:00] */
#define BCHP_MM_M2MC0_VERT_AVERAGER_COUNT_DUMMY_COUNT_MASK         0x00000001
#define BCHP_MM_M2MC0_VERT_AVERAGER_COUNT_DUMMY_COUNT_SHIFT        0

/***************************************************************************
 *VERT_AVERAGER_COEFF - Vertical averager control coefficient
 ***************************************************************************/
/* MM_M2MC0 :: VERT_AVERAGER_COEFF :: reserved0 [31:01] */
#define BCHP_MM_M2MC0_VERT_AVERAGER_COEFF_reserved0_MASK           0xfffffffe
#define BCHP_MM_M2MC0_VERT_AVERAGER_COEFF_reserved0_SHIFT          1

/* MM_M2MC0 :: VERT_AVERAGER_COEFF :: DUMMY_COEFF [00:00] */
#define BCHP_MM_M2MC0_VERT_AVERAGER_COEFF_DUMMY_COEFF_MASK         0x00000001
#define BCHP_MM_M2MC0_VERT_AVERAGER_COEFF_DUMMY_COEFF_SHIFT        0

/***************************************************************************
 *HORIZ_SCALER_0_INITIAL_PHASE - Horizontal scaler 0 initial phase
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_SCALER_0_INITIAL_PHASE :: reserved0 [31:24] */
#define BCHP_MM_M2MC0_HORIZ_SCALER_0_INITIAL_PHASE_reserved0_MASK  0xff000000
#define BCHP_MM_M2MC0_HORIZ_SCALER_0_INITIAL_PHASE_reserved0_SHIFT 24

/* MM_M2MC0 :: HORIZ_SCALER_0_INITIAL_PHASE :: PHASE [23:00] */
#define BCHP_MM_M2MC0_HORIZ_SCALER_0_INITIAL_PHASE_PHASE_MASK      0x00ffffff
#define BCHP_MM_M2MC0_HORIZ_SCALER_0_INITIAL_PHASE_PHASE_SHIFT     0
#define BCHP_MM_M2MC0_HORIZ_SCALER_0_INITIAL_PHASE_PHASE_DEFAULT   0x00000000

/***************************************************************************
 *HORIZ_SCALER_0_STEP - Horizontal scaler 0 step
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_SCALER_0_STEP :: reserved0 [31:24] */
#define BCHP_MM_M2MC0_HORIZ_SCALER_0_STEP_reserved0_MASK           0xff000000
#define BCHP_MM_M2MC0_HORIZ_SCALER_0_STEP_reserved0_SHIFT          24

/* MM_M2MC0 :: HORIZ_SCALER_0_STEP :: STEP [23:00] */
#define BCHP_MM_M2MC0_HORIZ_SCALER_0_STEP_STEP_MASK                0x00ffffff
#define BCHP_MM_M2MC0_HORIZ_SCALER_0_STEP_STEP_SHIFT               0
#define BCHP_MM_M2MC0_HORIZ_SCALER_0_STEP_STEP_DEFAULT             0x00100000

/***************************************************************************
 *HORIZ_SCALER_1_INITIAL_PHASE - Horizontal scaler 1 initial phase
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_SCALER_1_INITIAL_PHASE :: reserved0 [31:24] */
#define BCHP_MM_M2MC0_HORIZ_SCALER_1_INITIAL_PHASE_reserved0_MASK  0xff000000
#define BCHP_MM_M2MC0_HORIZ_SCALER_1_INITIAL_PHASE_reserved0_SHIFT 24

/* MM_M2MC0 :: HORIZ_SCALER_1_INITIAL_PHASE :: PHASE [23:00] */
#define BCHP_MM_M2MC0_HORIZ_SCALER_1_INITIAL_PHASE_PHASE_MASK      0x00ffffff
#define BCHP_MM_M2MC0_HORIZ_SCALER_1_INITIAL_PHASE_PHASE_SHIFT     0
#define BCHP_MM_M2MC0_HORIZ_SCALER_1_INITIAL_PHASE_PHASE_DEFAULT   0x00000000

/***************************************************************************
 *HORIZ_SCALER_1_STEP - Horizontal scaler 1 step
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_SCALER_1_STEP :: reserved0 [31:24] */
#define BCHP_MM_M2MC0_HORIZ_SCALER_1_STEP_reserved0_MASK           0xff000000
#define BCHP_MM_M2MC0_HORIZ_SCALER_1_STEP_reserved0_SHIFT          24

/* MM_M2MC0 :: HORIZ_SCALER_1_STEP :: STEP [23:00] */
#define BCHP_MM_M2MC0_HORIZ_SCALER_1_STEP_STEP_MASK                0x00ffffff
#define BCHP_MM_M2MC0_HORIZ_SCALER_1_STEP_STEP_SHIFT               0
#define BCHP_MM_M2MC0_HORIZ_SCALER_1_STEP_STEP_DEFAULT             0x00100000

/***************************************************************************
 *VERT_SCALER_0_INITIAL_PHASE - Vertical scaler 0 initial phase
 ***************************************************************************/
/* MM_M2MC0 :: VERT_SCALER_0_INITIAL_PHASE :: reserved0 [31:24] */
#define BCHP_MM_M2MC0_VERT_SCALER_0_INITIAL_PHASE_reserved0_MASK   0xff000000
#define BCHP_MM_M2MC0_VERT_SCALER_0_INITIAL_PHASE_reserved0_SHIFT  24

/* MM_M2MC0 :: VERT_SCALER_0_INITIAL_PHASE :: PHASE [23:00] */
#define BCHP_MM_M2MC0_VERT_SCALER_0_INITIAL_PHASE_PHASE_MASK       0x00ffffff
#define BCHP_MM_M2MC0_VERT_SCALER_0_INITIAL_PHASE_PHASE_SHIFT      0
#define BCHP_MM_M2MC0_VERT_SCALER_0_INITIAL_PHASE_PHASE_DEFAULT    0x00000000

/***************************************************************************
 *VERT_SCALER_0_STEP - Vertical scaler 0 step
 ***************************************************************************/
/* MM_M2MC0 :: VERT_SCALER_0_STEP :: reserved0 [31:24] */
#define BCHP_MM_M2MC0_VERT_SCALER_0_STEP_reserved0_MASK            0xff000000
#define BCHP_MM_M2MC0_VERT_SCALER_0_STEP_reserved0_SHIFT           24

/* MM_M2MC0 :: VERT_SCALER_0_STEP :: STEP [23:00] */
#define BCHP_MM_M2MC0_VERT_SCALER_0_STEP_STEP_MASK                 0x00ffffff
#define BCHP_MM_M2MC0_VERT_SCALER_0_STEP_STEP_SHIFT                0
#define BCHP_MM_M2MC0_VERT_SCALER_0_STEP_STEP_DEFAULT              0x00100000

/***************************************************************************
 *VERT_SCALER_1_INITIAL_PHASE - Vertical scaler 1 initial phase
 ***************************************************************************/
/* MM_M2MC0 :: VERT_SCALER_1_INITIAL_PHASE :: reserved0 [31:24] */
#define BCHP_MM_M2MC0_VERT_SCALER_1_INITIAL_PHASE_reserved0_MASK   0xff000000
#define BCHP_MM_M2MC0_VERT_SCALER_1_INITIAL_PHASE_reserved0_SHIFT  24

/* MM_M2MC0 :: VERT_SCALER_1_INITIAL_PHASE :: PHASE [23:00] */
#define BCHP_MM_M2MC0_VERT_SCALER_1_INITIAL_PHASE_PHASE_MASK       0x00ffffff
#define BCHP_MM_M2MC0_VERT_SCALER_1_INITIAL_PHASE_PHASE_SHIFT      0
#define BCHP_MM_M2MC0_VERT_SCALER_1_INITIAL_PHASE_PHASE_DEFAULT    0x00000000

/***************************************************************************
 *VERT_SCALER_1_STEP - Vertical scaler 1 step
 ***************************************************************************/
/* MM_M2MC0 :: VERT_SCALER_1_STEP :: reserved0 [31:24] */
#define BCHP_MM_M2MC0_VERT_SCALER_1_STEP_reserved0_MASK            0xff000000
#define BCHP_MM_M2MC0_VERT_SCALER_1_STEP_reserved0_SHIFT           24

/* MM_M2MC0 :: VERT_SCALER_1_STEP :: STEP [23:00] */
#define BCHP_MM_M2MC0_VERT_SCALER_1_STEP_STEP_MASK                 0x00ffffff
#define BCHP_MM_M2MC0_VERT_SCALER_1_STEP_STEP_SHIFT                0
#define BCHP_MM_M2MC0_VERT_SCALER_1_STEP_STEP_DEFAULT              0x00100000

/***************************************************************************
 *HORIZ_FIR_0_COEFF_PHASE0_01 - Horizontal Scaler 0 Poly-Phase Filter Phase 0 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE0_01 :: reserved0 [31:28] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_01_reserved0_MASK   0xf0000000
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_01_reserved0_SHIFT  28

/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE0_01 :: COEFF_0 [27:18] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_01_COEFF_0_MASK     0x0ffc0000
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_01_COEFF_0_SHIFT    18

/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE0_01 :: reserved1 [17:12] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_01_reserved1_MASK   0x0003f000
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_01_reserved1_SHIFT  12

/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE0_01 :: COEFF_1 [11:02] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_01_COEFF_1_MASK     0x00000ffc
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_01_COEFF_1_SHIFT    2

/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE0_01 :: reserved2 [01:00] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_01_reserved2_MASK   0x00000003
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_01_reserved2_SHIFT  0

/***************************************************************************
 *HORIZ_FIR_0_COEFF_PHASE0_2 - Horizontal Scaler 0 Poly-Phase Filter Phase 0 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE0_2 :: reserved0 [31:12] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_2_reserved0_MASK    0xfffff000
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_2_reserved0_SHIFT   12

/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE0_2 :: COEFF_2 [11:02] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_2_COEFF_2_MASK      0x00000ffc
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_2_COEFF_2_SHIFT     2

/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE0_2 :: reserved1 [01:00] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_2_reserved1_MASK    0x00000003
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE0_2_reserved1_SHIFT   0

/***************************************************************************
 *HORIZ_FIR_0_COEFF_PHASE1_01 - Horizontal Scaler 0 Poly-Phase Filter Phase 1 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE1_01 :: reserved0 [31:28] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_01_reserved0_MASK   0xf0000000
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_01_reserved0_SHIFT  28

/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE1_01 :: COEFF_0 [27:18] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_01_COEFF_0_MASK     0x0ffc0000
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_01_COEFF_0_SHIFT    18

/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE1_01 :: reserved1 [17:12] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_01_reserved1_MASK   0x0003f000
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_01_reserved1_SHIFT  12

/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE1_01 :: COEFF_1 [11:02] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_01_COEFF_1_MASK     0x00000ffc
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_01_COEFF_1_SHIFT    2

/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE1_01 :: reserved2 [01:00] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_01_reserved2_MASK   0x00000003
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_01_reserved2_SHIFT  0

/***************************************************************************
 *HORIZ_FIR_0_COEFF_PHASE1_2 - Horizontal Scaler 0 Poly-Phase Filter Phase 1 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE1_2 :: reserved0 [31:12] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_2_reserved0_MASK    0xfffff000
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_2_reserved0_SHIFT   12

/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE1_2 :: COEFF_2 [11:02] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_2_COEFF_2_MASK      0x00000ffc
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_2_COEFF_2_SHIFT     2

/* MM_M2MC0 :: HORIZ_FIR_0_COEFF_PHASE1_2 :: reserved1 [01:00] */
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_2_reserved1_MASK    0x00000003
#define BCHP_MM_M2MC0_HORIZ_FIR_0_COEFF_PHASE1_2_reserved1_SHIFT   0

/***************************************************************************
 *HORIZ_FIR_1_COEFF_PHASE0_01 - Horizontal Scaler 1 Poly-Phase Filter Phase 0 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE0_01 :: reserved0 [31:28] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_01_reserved0_MASK   0xf0000000
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_01_reserved0_SHIFT  28

/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE0_01 :: COEFF_0 [27:18] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_01_COEFF_0_MASK     0x0ffc0000
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_01_COEFF_0_SHIFT    18

/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE0_01 :: reserved1 [17:12] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_01_reserved1_MASK   0x0003f000
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_01_reserved1_SHIFT  12

/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE0_01 :: COEFF_1 [11:02] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_01_COEFF_1_MASK     0x00000ffc
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_01_COEFF_1_SHIFT    2

/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE0_01 :: reserved2 [01:00] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_01_reserved2_MASK   0x00000003
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_01_reserved2_SHIFT  0

/***************************************************************************
 *HORIZ_FIR_1_COEFF_PHASE0_2 - Horizontal Scaler 1 Poly-Phase Filter Phase 0 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE0_2 :: reserved0 [31:12] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_2_reserved0_MASK    0xfffff000
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_2_reserved0_SHIFT   12

/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE0_2 :: COEFF_2 [11:02] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_2_COEFF_2_MASK      0x00000ffc
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_2_COEFF_2_SHIFT     2

/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE0_2 :: reserved1 [01:00] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_2_reserved1_MASK    0x00000003
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE0_2_reserved1_SHIFT   0

/***************************************************************************
 *HORIZ_FIR_1_COEFF_PHASE1_01 - Horizontal Scaler 1 Poly-Phase Filter Phase 1 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE1_01 :: reserved0 [31:28] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_01_reserved0_MASK   0xf0000000
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_01_reserved0_SHIFT  28

/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE1_01 :: COEFF_0 [27:18] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_01_COEFF_0_MASK     0x0ffc0000
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_01_COEFF_0_SHIFT    18

/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE1_01 :: reserved1 [17:12] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_01_reserved1_MASK   0x0003f000
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_01_reserved1_SHIFT  12

/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE1_01 :: COEFF_1 [11:02] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_01_COEFF_1_MASK     0x00000ffc
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_01_COEFF_1_SHIFT    2

/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE1_01 :: reserved2 [01:00] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_01_reserved2_MASK   0x00000003
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_01_reserved2_SHIFT  0

/***************************************************************************
 *HORIZ_FIR_1_COEFF_PHASE1_2 - Horizontal Scaler 1 Poly-Phase Filter Phase 1 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE1_2 :: reserved0 [31:12] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_2_reserved0_MASK    0xfffff000
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_2_reserved0_SHIFT   12

/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE1_2 :: COEFF_2 [11:02] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_2_COEFF_2_MASK      0x00000ffc
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_2_COEFF_2_SHIFT     2

/* MM_M2MC0 :: HORIZ_FIR_1_COEFF_PHASE1_2 :: reserved1 [01:00] */
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_2_reserved1_MASK    0x00000003
#define BCHP_MM_M2MC0_HORIZ_FIR_1_COEFF_PHASE1_2_reserved1_SHIFT   0

/***************************************************************************
 *VERT_FIR_0_COEFF_PHASE0_01 - Vertical Scaler 0 Poly-Phase Filter Phase 0 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE0_01 :: reserved0 [31:28] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_01_reserved0_MASK    0xf0000000
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_01_reserved0_SHIFT   28

/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE0_01 :: COEFF_0 [27:18] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_01_COEFF_0_MASK      0x0ffc0000
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_01_COEFF_0_SHIFT     18

/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE0_01 :: reserved1 [17:12] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_01_reserved1_MASK    0x0003f000
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_01_reserved1_SHIFT   12

/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE0_01 :: COEFF_1 [11:02] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_01_COEFF_1_MASK      0x00000ffc
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_01_COEFF_1_SHIFT     2

/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE0_01 :: reserved2 [01:00] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_01_reserved2_MASK    0x00000003
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_01_reserved2_SHIFT   0

/***************************************************************************
 *VERT_FIR_0_COEFF_PHASE0_2 - Vertical Scaler 0 Poly-Phase Filter Phase 0 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE0_2 :: reserved0 [31:12] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_2_reserved0_MASK     0xfffff000
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_2_reserved0_SHIFT    12

/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE0_2 :: COEFF_2 [11:02] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_2_COEFF_2_MASK       0x00000ffc
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_2_COEFF_2_SHIFT      2

/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE0_2 :: reserved1 [01:00] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_2_reserved1_MASK     0x00000003
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE0_2_reserved1_SHIFT    0

/***************************************************************************
 *VERT_FIR_0_COEFF_PHASE1_01 - Vertical Scaler 0 Poly-Phase Filter Phase 1 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE1_01 :: reserved0 [31:28] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_01_reserved0_MASK    0xf0000000
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_01_reserved0_SHIFT   28

/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE1_01 :: COEFF_0 [27:18] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_01_COEFF_0_MASK      0x0ffc0000
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_01_COEFF_0_SHIFT     18

/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE1_01 :: reserved1 [17:12] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_01_reserved1_MASK    0x0003f000
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_01_reserved1_SHIFT   12

/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE1_01 :: COEFF_1 [11:02] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_01_COEFF_1_MASK      0x00000ffc
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_01_COEFF_1_SHIFT     2

/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE1_01 :: reserved2 [01:00] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_01_reserved2_MASK    0x00000003
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_01_reserved2_SHIFT   0

/***************************************************************************
 *VERT_FIR_0_COEFF_PHASE1_2 - Vertical Scaler 0 Poly-Phase Filter Phase 1 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE1_2 :: reserved0 [31:12] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_2_reserved0_MASK     0xfffff000
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_2_reserved0_SHIFT    12

/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE1_2 :: COEFF_2 [11:02] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_2_COEFF_2_MASK       0x00000ffc
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_2_COEFF_2_SHIFT      2

/* MM_M2MC0 :: VERT_FIR_0_COEFF_PHASE1_2 :: reserved1 [01:00] */
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_2_reserved1_MASK     0x00000003
#define BCHP_MM_M2MC0_VERT_FIR_0_COEFF_PHASE1_2_reserved1_SHIFT    0

/***************************************************************************
 *VERT_FIR_1_COEFF_PHASE0_01 - Vertical Scaler 1 Poly-Phase Filter Phase 0 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE0_01 :: reserved0 [31:28] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_01_reserved0_MASK    0xf0000000
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_01_reserved0_SHIFT   28

/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE0_01 :: COEFF_0 [27:18] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_01_COEFF_0_MASK      0x0ffc0000
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_01_COEFF_0_SHIFT     18

/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE0_01 :: reserved1 [17:12] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_01_reserved1_MASK    0x0003f000
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_01_reserved1_SHIFT   12

/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE0_01 :: COEFF_1 [11:02] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_01_COEFF_1_MASK      0x00000ffc
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_01_COEFF_1_SHIFT     2

/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE0_01 :: reserved2 [01:00] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_01_reserved2_MASK    0x00000003
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_01_reserved2_SHIFT   0

/***************************************************************************
 *VERT_FIR_1_COEFF_PHASE0_2 - Vertical Scaler 1 Poly-Phase Filter Phase 0 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE0_2 :: reserved0 [31:12] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_2_reserved0_MASK     0xfffff000
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_2_reserved0_SHIFT    12

/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE0_2 :: COEFF_2 [11:02] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_2_COEFF_2_MASK       0x00000ffc
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_2_COEFF_2_SHIFT      2

/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE0_2 :: reserved1 [01:00] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_2_reserved1_MASK     0x00000003
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE0_2_reserved1_SHIFT    0

/***************************************************************************
 *VERT_FIR_1_COEFF_PHASE1_01 - Vertical Scaler 1 Poly-Phase Filter Phase 1 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE1_01 :: reserved0 [31:28] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_01_reserved0_MASK    0xf0000000
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_01_reserved0_SHIFT   28

/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE1_01 :: COEFF_0 [27:18] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_01_COEFF_0_MASK      0x0ffc0000
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_01_COEFF_0_SHIFT     18

/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE1_01 :: reserved1 [17:12] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_01_reserved1_MASK    0x0003f000
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_01_reserved1_SHIFT   12

/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE1_01 :: COEFF_1 [11:02] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_01_COEFF_1_MASK      0x00000ffc
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_01_COEFF_1_SHIFT     2

/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE1_01 :: reserved2 [01:00] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_01_reserved2_MASK    0x00000003
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_01_reserved2_SHIFT   0

/***************************************************************************
 *VERT_FIR_1_COEFF_PHASE1_2 - Vertical Scaler 1 Poly-Phase Filter Phase 1 Coefficients
 ***************************************************************************/
/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE1_2 :: reserved0 [31:12] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_2_reserved0_MASK     0xfffff000
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_2_reserved0_SHIFT    12

/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE1_2 :: COEFF_2 [11:02] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_2_COEFF_2_MASK       0x00000ffc
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_2_COEFF_2_SHIFT      2

/* MM_M2MC0 :: VERT_FIR_1_COEFF_PHASE1_2 :: reserved1 [01:00] */
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_2_reserved1_MASK     0x00000003
#define BCHP_MM_M2MC0_VERT_FIR_1_COEFF_PHASE1_2_reserved1_SHIFT    0

/***************************************************************************
 *SRC_CM_C00_C01 - Color Conversion Matrix Coefficients C00 and C01
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CM_C00_C01 :: reserved0 [31:30] */
#define BCHP_MM_M2MC0_SRC_CM_C00_C01_reserved0_MASK                0xc0000000
#define BCHP_MM_M2MC0_SRC_CM_C00_C01_reserved0_SHIFT               30

/* MM_M2MC0 :: SRC_CM_C00_C01 :: CM_C00 [29:16] */
#define BCHP_MM_M2MC0_SRC_CM_C00_C01_CM_C00_MASK                   0x3fff0000
#define BCHP_MM_M2MC0_SRC_CM_C00_C01_CM_C00_SHIFT                  16

/* MM_M2MC0 :: SRC_CM_C00_C01 :: reserved1 [15:14] */
#define BCHP_MM_M2MC0_SRC_CM_C00_C01_reserved1_MASK                0x0000c000
#define BCHP_MM_M2MC0_SRC_CM_C00_C01_reserved1_SHIFT               14

/* MM_M2MC0 :: SRC_CM_C00_C01 :: CM_C01 [13:00] */
#define BCHP_MM_M2MC0_SRC_CM_C00_C01_CM_C01_MASK                   0x00003fff
#define BCHP_MM_M2MC0_SRC_CM_C00_C01_CM_C01_SHIFT                  0

/***************************************************************************
 *SRC_CM_C02_C03 - Color Conversion Matrix Coefficients C02 and C03
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CM_C02_C03 :: reserved0 [31:30] */
#define BCHP_MM_M2MC0_SRC_CM_C02_C03_reserved0_MASK                0xc0000000
#define BCHP_MM_M2MC0_SRC_CM_C02_C03_reserved0_SHIFT               30

/* MM_M2MC0 :: SRC_CM_C02_C03 :: CM_C02 [29:16] */
#define BCHP_MM_M2MC0_SRC_CM_C02_C03_CM_C02_MASK                   0x3fff0000
#define BCHP_MM_M2MC0_SRC_CM_C02_C03_CM_C02_SHIFT                  16

/* MM_M2MC0 :: SRC_CM_C02_C03 :: reserved1 [15:14] */
#define BCHP_MM_M2MC0_SRC_CM_C02_C03_reserved1_MASK                0x0000c000
#define BCHP_MM_M2MC0_SRC_CM_C02_C03_reserved1_SHIFT               14

/* MM_M2MC0 :: SRC_CM_C02_C03 :: CM_C03 [13:00] */
#define BCHP_MM_M2MC0_SRC_CM_C02_C03_CM_C03_MASK                   0x00003fff
#define BCHP_MM_M2MC0_SRC_CM_C02_C03_CM_C03_SHIFT                  0

/***************************************************************************
 *SRC_CM_C04 - Color Conversion Matrix Coefficients C04
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CM_C04 :: reserved0 [31:14] */
#define BCHP_MM_M2MC0_SRC_CM_C04_reserved0_MASK                    0xffffc000
#define BCHP_MM_M2MC0_SRC_CM_C04_reserved0_SHIFT                   14

/* MM_M2MC0 :: SRC_CM_C04 :: CM_C04 [13:00] */
#define BCHP_MM_M2MC0_SRC_CM_C04_CM_C04_MASK                       0x00003fff
#define BCHP_MM_M2MC0_SRC_CM_C04_CM_C04_SHIFT                      0

/***************************************************************************
 *SRC_CM_C10_C11 - Color Conversion Matrix Coefficients C10 and C11
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CM_C10_C11 :: reserved0 [31:30] */
#define BCHP_MM_M2MC0_SRC_CM_C10_C11_reserved0_MASK                0xc0000000
#define BCHP_MM_M2MC0_SRC_CM_C10_C11_reserved0_SHIFT               30

/* MM_M2MC0 :: SRC_CM_C10_C11 :: CM_C10 [29:16] */
#define BCHP_MM_M2MC0_SRC_CM_C10_C11_CM_C10_MASK                   0x3fff0000
#define BCHP_MM_M2MC0_SRC_CM_C10_C11_CM_C10_SHIFT                  16

/* MM_M2MC0 :: SRC_CM_C10_C11 :: reserved1 [15:14] */
#define BCHP_MM_M2MC0_SRC_CM_C10_C11_reserved1_MASK                0x0000c000
#define BCHP_MM_M2MC0_SRC_CM_C10_C11_reserved1_SHIFT               14

/* MM_M2MC0 :: SRC_CM_C10_C11 :: CM_C11 [13:00] */
#define BCHP_MM_M2MC0_SRC_CM_C10_C11_CM_C11_MASK                   0x00003fff
#define BCHP_MM_M2MC0_SRC_CM_C10_C11_CM_C11_SHIFT                  0

/***************************************************************************
 *SRC_CM_C12_C13 - Color Conversion Matrix Coefficients C12 and C13
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CM_C12_C13 :: reserved0 [31:30] */
#define BCHP_MM_M2MC0_SRC_CM_C12_C13_reserved0_MASK                0xc0000000
#define BCHP_MM_M2MC0_SRC_CM_C12_C13_reserved0_SHIFT               30

/* MM_M2MC0 :: SRC_CM_C12_C13 :: CM_C12 [29:16] */
#define BCHP_MM_M2MC0_SRC_CM_C12_C13_CM_C12_MASK                   0x3fff0000
#define BCHP_MM_M2MC0_SRC_CM_C12_C13_CM_C12_SHIFT                  16

/* MM_M2MC0 :: SRC_CM_C12_C13 :: reserved1 [15:14] */
#define BCHP_MM_M2MC0_SRC_CM_C12_C13_reserved1_MASK                0x0000c000
#define BCHP_MM_M2MC0_SRC_CM_C12_C13_reserved1_SHIFT               14

/* MM_M2MC0 :: SRC_CM_C12_C13 :: CM_C13 [13:00] */
#define BCHP_MM_M2MC0_SRC_CM_C12_C13_CM_C13_MASK                   0x00003fff
#define BCHP_MM_M2MC0_SRC_CM_C12_C13_CM_C13_SHIFT                  0

/***************************************************************************
 *SRC_CM_C14 - Color Conversion Matrix Coefficients C14
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CM_C14 :: reserved0 [31:14] */
#define BCHP_MM_M2MC0_SRC_CM_C14_reserved0_MASK                    0xffffc000
#define BCHP_MM_M2MC0_SRC_CM_C14_reserved0_SHIFT                   14

/* MM_M2MC0 :: SRC_CM_C14 :: CM_C14 [13:00] */
#define BCHP_MM_M2MC0_SRC_CM_C14_CM_C14_MASK                       0x00003fff
#define BCHP_MM_M2MC0_SRC_CM_C14_CM_C14_SHIFT                      0

/***************************************************************************
 *SRC_CM_C20_C21 - Color Conversion Matrix Coefficients C20 and C21
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CM_C20_C21 :: reserved0 [31:30] */
#define BCHP_MM_M2MC0_SRC_CM_C20_C21_reserved0_MASK                0xc0000000
#define BCHP_MM_M2MC0_SRC_CM_C20_C21_reserved0_SHIFT               30

/* MM_M2MC0 :: SRC_CM_C20_C21 :: CM_C20 [29:16] */
#define BCHP_MM_M2MC0_SRC_CM_C20_C21_CM_C20_MASK                   0x3fff0000
#define BCHP_MM_M2MC0_SRC_CM_C20_C21_CM_C20_SHIFT                  16

/* MM_M2MC0 :: SRC_CM_C20_C21 :: reserved1 [15:14] */
#define BCHP_MM_M2MC0_SRC_CM_C20_C21_reserved1_MASK                0x0000c000
#define BCHP_MM_M2MC0_SRC_CM_C20_C21_reserved1_SHIFT               14

/* MM_M2MC0 :: SRC_CM_C20_C21 :: CM_C21 [13:00] */
#define BCHP_MM_M2MC0_SRC_CM_C20_C21_CM_C21_MASK                   0x00003fff
#define BCHP_MM_M2MC0_SRC_CM_C20_C21_CM_C21_SHIFT                  0

/***************************************************************************
 *SRC_CM_C22_C23 - Color Conversion Matrix Coefficients C22 and C23
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CM_C22_C23 :: reserved0 [31:30] */
#define BCHP_MM_M2MC0_SRC_CM_C22_C23_reserved0_MASK                0xc0000000
#define BCHP_MM_M2MC0_SRC_CM_C22_C23_reserved0_SHIFT               30

/* MM_M2MC0 :: SRC_CM_C22_C23 :: CM_C22 [29:16] */
#define BCHP_MM_M2MC0_SRC_CM_C22_C23_CM_C22_MASK                   0x3fff0000
#define BCHP_MM_M2MC0_SRC_CM_C22_C23_CM_C22_SHIFT                  16

/* MM_M2MC0 :: SRC_CM_C22_C23 :: reserved1 [15:14] */
#define BCHP_MM_M2MC0_SRC_CM_C22_C23_reserved1_MASK                0x0000c000
#define BCHP_MM_M2MC0_SRC_CM_C22_C23_reserved1_SHIFT               14

/* MM_M2MC0 :: SRC_CM_C22_C23 :: CM_C23 [13:00] */
#define BCHP_MM_M2MC0_SRC_CM_C22_C23_CM_C23_MASK                   0x00003fff
#define BCHP_MM_M2MC0_SRC_CM_C22_C23_CM_C23_SHIFT                  0

/***************************************************************************
 *SRC_CM_C24 - Color Conversion Matrix Coefficients C24
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CM_C24 :: reserved0 [31:14] */
#define BCHP_MM_M2MC0_SRC_CM_C24_reserved0_MASK                    0xffffc000
#define BCHP_MM_M2MC0_SRC_CM_C24_reserved0_SHIFT                   14

/* MM_M2MC0 :: SRC_CM_C24 :: CM_C24 [13:00] */
#define BCHP_MM_M2MC0_SRC_CM_C24_CM_C24_MASK                       0x00003fff
#define BCHP_MM_M2MC0_SRC_CM_C24_CM_C24_SHIFT                      0

/***************************************************************************
 *SRC_CM_C30_C31 - Color Conversion Matrix Coefficients C30 and C31
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CM_C30_C31 :: reserved0 [31:30] */
#define BCHP_MM_M2MC0_SRC_CM_C30_C31_reserved0_MASK                0xc0000000
#define BCHP_MM_M2MC0_SRC_CM_C30_C31_reserved0_SHIFT               30

/* MM_M2MC0 :: SRC_CM_C30_C31 :: CM_C30 [29:16] */
#define BCHP_MM_M2MC0_SRC_CM_C30_C31_CM_C30_MASK                   0x3fff0000
#define BCHP_MM_M2MC0_SRC_CM_C30_C31_CM_C30_SHIFT                  16

/* MM_M2MC0 :: SRC_CM_C30_C31 :: reserved1 [15:14] */
#define BCHP_MM_M2MC0_SRC_CM_C30_C31_reserved1_MASK                0x0000c000
#define BCHP_MM_M2MC0_SRC_CM_C30_C31_reserved1_SHIFT               14

/* MM_M2MC0 :: SRC_CM_C30_C31 :: CM_C31 [13:00] */
#define BCHP_MM_M2MC0_SRC_CM_C30_C31_CM_C31_MASK                   0x00003fff
#define BCHP_MM_M2MC0_SRC_CM_C30_C31_CM_C31_SHIFT                  0

/***************************************************************************
 *SRC_CM_C32_C33 - Color Conversion Matrix Coefficients C32 and C33
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CM_C32_C33 :: reserved0 [31:30] */
#define BCHP_MM_M2MC0_SRC_CM_C32_C33_reserved0_MASK                0xc0000000
#define BCHP_MM_M2MC0_SRC_CM_C32_C33_reserved0_SHIFT               30

/* MM_M2MC0 :: SRC_CM_C32_C33 :: CM_C32 [29:16] */
#define BCHP_MM_M2MC0_SRC_CM_C32_C33_CM_C32_MASK                   0x3fff0000
#define BCHP_MM_M2MC0_SRC_CM_C32_C33_CM_C32_SHIFT                  16

/* MM_M2MC0 :: SRC_CM_C32_C33 :: reserved1 [15:14] */
#define BCHP_MM_M2MC0_SRC_CM_C32_C33_reserved1_MASK                0x0000c000
#define BCHP_MM_M2MC0_SRC_CM_C32_C33_reserved1_SHIFT               14

/* MM_M2MC0 :: SRC_CM_C32_C33 :: CM_C33 [13:00] */
#define BCHP_MM_M2MC0_SRC_CM_C32_C33_CM_C33_MASK                   0x00003fff
#define BCHP_MM_M2MC0_SRC_CM_C32_C33_CM_C33_SHIFT                  0

/***************************************************************************
 *SRC_CM_C34 - Color Conversion Matrix Coefficients C34
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CM_C34 :: reserved0 [31:14] */
#define BCHP_MM_M2MC0_SRC_CM_C34_reserved0_MASK                    0xffffc000
#define BCHP_MM_M2MC0_SRC_CM_C34_reserved0_SHIFT                   14

/* MM_M2MC0 :: SRC_CM_C34 :: CM_C34 [13:00] */
#define BCHP_MM_M2MC0_SRC_CM_C34_CM_C34_MASK                       0x00003fff
#define BCHP_MM_M2MC0_SRC_CM_C34_CM_C34_SHIFT                      0

/***************************************************************************
 *TIMEOUT_COUNTER_CONTROL - M2MC timeout counter control register
 ***************************************************************************/
/* MM_M2MC0 :: TIMEOUT_COUNTER_CONTROL :: TIMEOUT_COUNTER_ENABLE [31:31] */
#define BCHP_MM_M2MC0_TIMEOUT_COUNTER_CONTROL_TIMEOUT_COUNTER_ENABLE_MASK 0x80000000
#define BCHP_MM_M2MC0_TIMEOUT_COUNTER_CONTROL_TIMEOUT_COUNTER_ENABLE_SHIFT 31
#define BCHP_MM_M2MC0_TIMEOUT_COUNTER_CONTROL_TIMEOUT_COUNTER_ENABLE_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_TIMEOUT_COUNTER_CONTROL_TIMEOUT_COUNTER_ENABLE_DISABLE 0
#define BCHP_MM_M2MC0_TIMEOUT_COUNTER_CONTROL_TIMEOUT_COUNTER_ENABLE_ENABLE 1

/* MM_M2MC0 :: TIMEOUT_COUNTER_CONTROL :: TIMEOUT_COUNT_VALUE [30:00] */
#define BCHP_MM_M2MC0_TIMEOUT_COUNTER_CONTROL_TIMEOUT_COUNT_VALUE_MASK 0x7fffffff
#define BCHP_MM_M2MC0_TIMEOUT_COUNTER_CONTROL_TIMEOUT_COUNT_VALUE_SHIFT 0

/***************************************************************************
 *CLK_GATE_AND_SW_INIT_CONTROL - M2MC clock gating and software init control register
 ***************************************************************************/
/* MM_M2MC0 :: CLK_GATE_AND_SW_INIT_CONTROL :: reserved0 [31:09] */
#define BCHP_MM_M2MC0_CLK_GATE_AND_SW_INIT_CONTROL_reserved0_MASK  0xfffffe00
#define BCHP_MM_M2MC0_CLK_GATE_AND_SW_INIT_CONTROL_reserved0_SHIFT 9

/* MM_M2MC0 :: CLK_GATE_AND_SW_INIT_CONTROL :: INTER_BLT_CLK_GATE_ENABLE [08:08] */
#define BCHP_MM_M2MC0_CLK_GATE_AND_SW_INIT_CONTROL_INTER_BLT_CLK_GATE_ENABLE_MASK 0x00000100
#define BCHP_MM_M2MC0_CLK_GATE_AND_SW_INIT_CONTROL_INTER_BLT_CLK_GATE_ENABLE_SHIFT 8
#define BCHP_MM_M2MC0_CLK_GATE_AND_SW_INIT_CONTROL_INTER_BLT_CLK_GATE_ENABLE_DEFAULT 0x00000000

/* MM_M2MC0 :: CLK_GATE_AND_SW_INIT_CONTROL :: reserved1 [07:01] */
#define BCHP_MM_M2MC0_CLK_GATE_AND_SW_INIT_CONTROL_reserved1_MASK  0x000000fe
#define BCHP_MM_M2MC0_CLK_GATE_AND_SW_INIT_CONTROL_reserved1_SHIFT 1

/* MM_M2MC0 :: CLK_GATE_AND_SW_INIT_CONTROL :: START_SW_INIT [00:00] */
#define BCHP_MM_M2MC0_CLK_GATE_AND_SW_INIT_CONTROL_START_SW_INIT_MASK 0x00000001
#define BCHP_MM_M2MC0_CLK_GATE_AND_SW_INIT_CONTROL_START_SW_INIT_SHIFT 0
#define BCHP_MM_M2MC0_CLK_GATE_AND_SW_INIT_CONTROL_START_SW_INIT_DEFAULT 0x00000000

/***************************************************************************
 *DEADLOCK_DETECT_CONTROL - Deadlock Detect Control register
 ***************************************************************************/
/* MM_M2MC0 :: DEADLOCK_DETECT_CONTROL :: reserved0 [31:16] */
#define BCHP_MM_M2MC0_DEADLOCK_DETECT_CONTROL_reserved0_MASK       0xffff0000
#define BCHP_MM_M2MC0_DEADLOCK_DETECT_CONTROL_reserved0_SHIFT      16

/* MM_M2MC0 :: DEADLOCK_DETECT_CONTROL :: COUNT [15:01] */
#define BCHP_MM_M2MC0_DEADLOCK_DETECT_CONTROL_COUNT_MASK           0x0000fffe
#define BCHP_MM_M2MC0_DEADLOCK_DETECT_CONTROL_COUNT_SHIFT          1
#define BCHP_MM_M2MC0_DEADLOCK_DETECT_CONTROL_COUNT_DEFAULT        0x00000000

/* MM_M2MC0 :: DEADLOCK_DETECT_CONTROL :: ENABLE [00:00] */
#define BCHP_MM_M2MC0_DEADLOCK_DETECT_CONTROL_ENABLE_MASK          0x00000001
#define BCHP_MM_M2MC0_DEADLOCK_DETECT_CONTROL_ENABLE_SHIFT         0
#define BCHP_MM_M2MC0_DEADLOCK_DETECT_CONTROL_ENABLE_DEFAULT       0x00000000
#define BCHP_MM_M2MC0_DEADLOCK_DETECT_CONTROL_ENABLE_DISABLE       0
#define BCHP_MM_M2MC0_DEADLOCK_DETECT_CONTROL_ENABLE_ENABLE        1

/***************************************************************************
 *TIMEOUT_DEBUG - Timeout Debug Register
 ***************************************************************************/
/* MM_M2MC0 :: TIMEOUT_DEBUG :: DEADLOCK_DETECT [31:31] */
#define BCHP_MM_M2MC0_TIMEOUT_DEBUG_DEADLOCK_DETECT_MASK           0x80000000
#define BCHP_MM_M2MC0_TIMEOUT_DEBUG_DEADLOCK_DETECT_SHIFT          31

/* MM_M2MC0 :: TIMEOUT_DEBUG :: EXECUTION_TIME [30:00] */
#define BCHP_MM_M2MC0_TIMEOUT_DEBUG_EXECUTION_TIME_MASK            0x7fffffff
#define BCHP_MM_M2MC0_TIMEOUT_DEBUG_EXECUTION_TIME_SHIFT           0

/***************************************************************************
 *ARBITRATION_DELAY - Arbitration Delay Register
 ***************************************************************************/
/* MM_M2MC0 :: ARBITRATION_DELAY :: reserved0 [31:31] */
#define BCHP_MM_M2MC0_ARBITRATION_DELAY_reserved0_MASK             0x80000000
#define BCHP_MM_M2MC0_ARBITRATION_DELAY_reserved0_SHIFT            31

/* MM_M2MC0 :: ARBITRATION_DELAY :: VALUE [30:00] */
#define BCHP_MM_M2MC0_ARBITRATION_DELAY_VALUE_MASK                 0x7fffffff
#define BCHP_MM_M2MC0_ARBITRATION_DELAY_VALUE_SHIFT                0

/***************************************************************************
 *RDMA_DEBUG_CLEAR - M2MC RDMA Debug Clear
 ***************************************************************************/
/* MM_M2MC0 :: RDMA_DEBUG_CLEAR :: reserved0 [31:01] */
#define BCHP_MM_M2MC0_RDMA_DEBUG_CLEAR_reserved0_MASK              0xfffffffe
#define BCHP_MM_M2MC0_RDMA_DEBUG_CLEAR_reserved0_SHIFT             1

/* MM_M2MC0 :: RDMA_DEBUG_CLEAR :: RBUS_MASTER_ERR_CLEAR [00:00] */
#define BCHP_MM_M2MC0_RDMA_DEBUG_CLEAR_RBUS_MASTER_ERR_CLEAR_MASK  0x00000001
#define BCHP_MM_M2MC0_RDMA_DEBUG_CLEAR_RBUS_MASTER_ERR_CLEAR_SHIFT 0
#define BCHP_MM_M2MC0_RDMA_DEBUG_CLEAR_RBUS_MASTER_ERR_CLEAR_DEFAULT 0x00000000

/***************************************************************************
 *RDMA_DEBUG_STATUS - M2MC RDMA Debug Status
 ***************************************************************************/
/* MM_M2MC0 :: RDMA_DEBUG_STATUS :: reserved0 [31:30] */
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_reserved0_MASK             0xc0000000
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_reserved0_SHIFT            30

/* MM_M2MC0 :: RDMA_DEBUG_STATUS :: RBUS_MASTER_ERR_ADDR [29:18] */
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_RBUS_MASTER_ERR_ADDR_MASK  0x3ffc0000
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_RBUS_MASTER_ERR_ADDR_SHIFT 18
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_RBUS_MASTER_ERR_ADDR_DEFAULT 0x00000000

/* MM_M2MC0 :: RDMA_DEBUG_STATUS :: RBUS_MASTER_ERR_RSP [17:16] */
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_RBUS_MASTER_ERR_RSP_MASK   0x00030000
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_RBUS_MASTER_ERR_RSP_SHIFT  16
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_RBUS_MASTER_ERR_RSP_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_RBUS_MASTER_ERR_RSP_OKAY   0
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_RBUS_MASTER_ERR_RSP_WARNG  1
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_RBUS_MASTER_ERR_RSP_SLVERR 2
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_RBUS_MASTER_ERR_RSP_DECERR 3

/* MM_M2MC0 :: RDMA_DEBUG_STATUS :: LIST_HANDLER_STATE [15:14] */
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_LIST_HANDLER_STATE_MASK    0x0000c000
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_LIST_HANDLER_STATE_SHIFT   14
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_LIST_HANDLER_STATE_DEFAULT 0x00000000
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_LIST_HANDLER_STATE_LIST_HANDLER_IDLE 0
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_LIST_HANDLER_STATE_WAIT_FOR_ACCEPT 1
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_LIST_HANDLER_STATE_WAIT_FOR_VALID 2
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_LIST_HANDLER_STATE_SLEEP   3

/* MM_M2MC0 :: RDMA_DEBUG_STATUS :: PKT_FETCHER_STATE [13:12] */
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_FETCHER_STATE_MASK     0x00003000
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_FETCHER_STATE_SHIFT    12
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_FETCHER_STATE_DEFAULT  0x00000000
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_FETCHER_STATE_PKT_FETCHER_IDLE 0
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_FETCHER_STATE_DRIVE_FETCH_CMD 1
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_FETCHER_STATE_WAIT_FOR_JW 2
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_FETCHER_STATE_PREPARE_FOR_FETCH 3

/* MM_M2MC0 :: RDMA_DEBUG_STATUS :: PKT_READER_STATE [11:10] */
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_READER_STATE_MASK      0x00000c00
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_READER_STATE_SHIFT     10
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_READER_STATE_DEFAULT   0x00000000
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_READER_STATE_PKT_READER_IDLE 0
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_READER_STATE_DRIVE_RBUS_MASTER 1
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_READER_STATE_WAIT_FOR_A_CYCLE 2
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_READER_STATE_WAIT_FOR_M2MC_EXEC 3

/* MM_M2MC0 :: RDMA_DEBUG_STATUS :: BUF_AVAIL_WR [09:08] */
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_BUF_AVAIL_WR_MASK          0x00000300
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_BUF_AVAIL_WR_SHIFT         8
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_BUF_AVAIL_WR_DEFAULT       0x00000002

/* MM_M2MC0 :: RDMA_DEBUG_STATUS :: BUF_AVAIL_RD [07:06] */
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_BUF_AVAIL_RD_MASK          0x000000c0
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_BUF_AVAIL_RD_SHIFT         6
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_BUF_AVAIL_RD_DEFAULT       0x00000000

/* MM_M2MC0 :: RDMA_DEBUG_STATUS :: PKT_COUNTER [05:00] */
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_COUNTER_MASK           0x0000003f
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_COUNTER_SHIFT          0
#define BCHP_MM_M2MC0_RDMA_DEBUG_STATUS_PKT_COUNTER_DEFAULT        0x00000000

/***************************************************************************
 *SRC_CLUT_ENTRY_%i - Source CLUT RAM entry 000..255
 ***************************************************************************/
#define BCHP_MM_M2MC0_SRC_CLUT_ENTRY_i_ARRAY_BASE                  0x002e8400
#define BCHP_MM_M2MC0_SRC_CLUT_ENTRY_i_ARRAY_START                 0
#define BCHP_MM_M2MC0_SRC_CLUT_ENTRY_i_ARRAY_END                   255
#define BCHP_MM_M2MC0_SRC_CLUT_ENTRY_i_ARRAY_ELEMENT_SIZE          32

/***************************************************************************
 *SRC_CLUT_ENTRY_%i - Source CLUT RAM entry 000..255
 ***************************************************************************/
/* MM_M2MC0 :: SRC_CLUT_ENTRY_i :: ALPHA [31:24] */
#define BCHP_MM_M2MC0_SRC_CLUT_ENTRY_i_ALPHA_MASK                  0xff000000
#define BCHP_MM_M2MC0_SRC_CLUT_ENTRY_i_ALPHA_SHIFT                 24

/* MM_M2MC0 :: SRC_CLUT_ENTRY_i :: RED [23:16] */
#define BCHP_MM_M2MC0_SRC_CLUT_ENTRY_i_RED_MASK                    0x00ff0000
#define BCHP_MM_M2MC0_SRC_CLUT_ENTRY_i_RED_SHIFT                   16

/* MM_M2MC0 :: SRC_CLUT_ENTRY_i :: GREEN [15:08] */
#define BCHP_MM_M2MC0_SRC_CLUT_ENTRY_i_GREEN_MASK                  0x0000ff00
#define BCHP_MM_M2MC0_SRC_CLUT_ENTRY_i_GREEN_SHIFT                 8

/* MM_M2MC0 :: SRC_CLUT_ENTRY_i :: BLUE [07:00] */
#define BCHP_MM_M2MC0_SRC_CLUT_ENTRY_i_BLUE_MASK                   0x000000ff
#define BCHP_MM_M2MC0_SRC_CLUT_ENTRY_i_BLUE_SHIFT                  0


#endif /* #ifndef BCHP_MM_M2MC0_H__ */

/* End of File */
