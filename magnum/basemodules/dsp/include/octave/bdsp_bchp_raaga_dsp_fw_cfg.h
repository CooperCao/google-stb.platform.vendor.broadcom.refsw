/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

/*
The following file is not completely auto-generated but manually edited also.
Offsets such as 0x00C23000 for some chips and 0x00823000 for some other chips were there.
For chips 7145 and 7344, these offsets were very different and automated versions wouldn't work rightaway with a simple #if #else
So I manually edited to have the right offset to accomodate all chips.
The offset can be arrived at based on the one of the members of the bchp_raaga_dsp_fw_cfg.h which is BCHP_RAAGA_DSP_FW_CFG_SW_UNDEFINED_SPAREi_ARRAY_BASE
*/

#include "bchp_raaga_dsp_fw_cfg.h"
#ifndef BCHP_RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO0_BASEADDR

/***************************************************************************
 *RAAGA_DSP_FW_CFG - Raaga DSP FW Configuration Registers
 ***************************************************************************/
#define BCHP_RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO0_BASEADDR 0x01023000 /* [RW][64] FIFO 0 BASEADDRESS */
#define BCHP_RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO_ID 0x01023008 /* [RW][64] FIFO ID Containing HOST2DSP Command */
#define BCHP_RAAGA_DSP_FW_CFG_SW_UNUSED1         0x01023010 /* [RW][64] UNUSED REGISTER SPACE */
#define BCHP_RAAGA_DSP_FW_CFG_SW_UNUSED2         0x01023018 /* [RW][64] UNUSED REGISTER SPACE */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR   0x01023020 /* [RW][64] Base Address for FIFO 0 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_0_END_ADDR    0x01023028 /* [RW][64] End Address for FIFO 0 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_0_WRITE_ADDR  0x01023030 /* [RW][64] Write Pointer for FIFO 0 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_0_READ_ADDR   0x01023038 /* [RW][64] Read Pointer for FIFO 0 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR   0x01023040 /* [RW][64] Base Address for FIFO 1 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_1_END_ADDR    0x01023048 /* [RW][64] End Address for FIFO 1 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_1_WRITE_ADDR  0x01023050 /* [RW][64] Write Pointer for FIFO 1 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_1_READ_ADDR   0x01023058 /* [RW][64] Read Pointer for FIFO 1 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_2_BASE_ADDR   0x01023060 /* [RW][64] Base Address for FIFO 2 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_2_END_ADDR    0x01023068 /* [RW][64] End Address for FIFO 2 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_2_WRITE_ADDR  0x01023070 /* [RW][64] Write Pointer for FIFO 2 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_2_READ_ADDR   0x01023078 /* [RW][64] Read Pointer for FIFO 2 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_3_BASE_ADDR   0x01023080 /* [RW][64] Base Address for FIFO 3 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_3_END_ADDR    0x01023088 /* [RW][64] End Address for FIFO 3 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_3_WRITE_ADDR  0x01023090 /* [RW][64] Write Pointer for FIFO 3 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_3_READ_ADDR   0x01023098 /* [RW][64] Read Pointer for FIFO 3 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_4_BASE_ADDR   0x010230a0 /* [RW][64] Base Address for FIFO 4 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_4_END_ADDR    0x010230a8 /* [RW][64] End Address for FIFO 4 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_4_WRITE_ADDR  0x010230b0 /* [RW][64] Write Pointer for FIFO 4 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_4_READ_ADDR   0x010230b8 /* [RW][64] Read Pointer for FIFO 4 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_5_BASE_ADDR   0x010230c0 /* [RW][64] Base Address for FIFO 5 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_5_END_ADDR    0x010230c8 /* [RW][64] End Address for FIFO 5 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_5_WRITE_ADDR  0x010230d0 /* [RW][64] Write Pointer for FIFO 5 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_5_READ_ADDR   0x010230d8 /* [RW][64] Read Pointer for FIFO 5 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_6_BASE_ADDR   0x010230e0 /* [RW][64] Base Address for FIFO 6 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_6_END_ADDR    0x010230e8 /* [RW][64] End Address for FIFO 6 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_6_WRITE_ADDR  0x010230f0 /* [RW][64] Write Pointer for FIFO 6 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_6_READ_ADDR   0x010230f8 /* [RW][64] Read Pointer for FIFO 6 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_7_BASE_ADDR   0x01023100 /* [RW][64] Base Address for FIFO 7 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_7_END_ADDR    0x01023108 /* [RW][64] End Address for FIFO 7 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_7_WRITE_ADDR  0x01023110 /* [RW][64] Write Pointer for FIFO 7 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_7_READ_ADDR   0x01023118 /* [RW][64] Read Pointer for FIFO 7 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_8_BASE_ADDR   0x01023120 /* [RW][64] Base Address for FIFO 8 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_8_END_ADDR    0x01023128 /* [RW][64] End Address for FIFO 8 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_8_WRITE_ADDR  0x01023130 /* [RW][64] Write Pointer for FIFO 8 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_8_READ_ADDR   0x01023138 /* [RW][64] Read Pointer for FIFO 8 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_9_BASE_ADDR   0x01023140 /* [RW][64] Base Address for FIFO 9 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_9_END_ADDR    0x01023148 /* [RW][64] End Address for FIFO 9 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_9_WRITE_ADDR  0x01023150 /* [RW][64] Write Pointer for FIFO 9 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_9_READ_ADDR   0x01023158 /* [RW][64] Read Pointer for FIFO 9 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_10_BASE_ADDR  0x01023160 /* [RW][64] Base Address for FIFO 10 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_10_END_ADDR   0x01023168 /* [RW][64] End Address for FIFO 10 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_10_WRITE_ADDR 0x01023170 /* [RW][64] Write Pointer for FIFO 10 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_10_READ_ADDR  0x01023178 /* [RW][64] Read Pointer for FIFO 10 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_11_BASE_ADDR  0x01023180 /* [RW][64] Base Address for FIFO 11 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_11_END_ADDR   0x01023188 /* [RW][64] End Address for FIFO 11 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_11_WRITE_ADDR 0x01023190 /* [RW][64] Write Pointer for FIFO 11 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_11_READ_ADDR  0x01023198 /* [RW][64] Read Pointer for FIFO 11 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_12_BASE_ADDR  0x010231a0 /* [RW][64] Base Address for FIFO 12 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_12_END_ADDR   0x010231a8 /* [RW][64] End Address for FIFO 12 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_12_WRITE_ADDR 0x010231b0 /* [RW][64] Write Pointer for FIFO 12 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_12_READ_ADDR  0x010231b8 /* [RW][64] Read Pointer for FIFO 12 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_13_BASE_ADDR  0x010231c0 /* [RW][64] Base Address for FIFO 13 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_13_END_ADDR   0x010231c8 /* [RW][64] End Address for FIFO 13 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_13_WRITE_ADDR 0x010231d0 /* [RW][64] Write Pointer for FIFO 13 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_13_READ_ADDR  0x010231d8 /* [RW][64] Read Pointer for FIFO 13 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_14_BASE_ADDR  0x010231e0 /* [RW][64] Base Address for FIFO 14 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_14_END_ADDR   0x010231e8 /* [RW][64] End Address for FIFO 14 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_14_WRITE_ADDR 0x010231f0 /* [RW][64] Write Pointer for FIFO 14 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_14_READ_ADDR  0x010231f8 /* [RW][64] Read Pointer for FIFO 14 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_15_BASE_ADDR  0x01023200 /* [RW][64] Base Address for FIFO 15 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_15_END_ADDR   0x01023208 /* [RW][64] End Address for FIFO 15 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_15_WRITE_ADDR 0x01023210 /* [RW][64] Write Pointer for FIFO 15 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_15_READ_ADDR  0x01023218 /* [RW][64] Read Pointer for FIFO 15 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_16_BASE_ADDR  0x01023220 /* [RW][64] Base Address for FIFO 16 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_16_END_ADDR   0x01023228 /* [RW][64] End Address for FIFO 16 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_16_WRITE_ADDR 0x01023230 /* [RW][64] Write Pointer for FIFO 16 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_16_READ_ADDR  0x01023238 /* [RW][64] Read Pointer for FIFO 16 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_17_BASE_ADDR  0x01023240 /* [RW][64] Base Address for FIFO 17 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_17_END_ADDR   0x01023248 /* [RW][64] End Address for FIFO 17 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_17_WRITE_ADDR 0x01023250 /* [RW][64] Write Pointer for FIFO 17 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_17_READ_ADDR  0x01023258 /* [RW][64] Read Pointer for FIFO 17 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_18_BASE_ADDR  0x01023260 /* [RW][64] Base Address for FIFO 18 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_18_END_ADDR   0x01023268 /* [RW][64] End Address for FIFO 18 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_18_WRITE_ADDR 0x01023270 /* [RW][64] Write Pointer for FIFO 18 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_18_READ_ADDR  0x01023278 /* [RW][64] Read Pointer for FIFO 18 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_19_BASE_ADDR  0x01023280 /* [RW][64] Base Address for FIFO 19 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_19_END_ADDR   0x01023288 /* [RW][64] End Address for FIFO 19 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_19_WRITE_ADDR 0x01023290 /* [RW][64] Write Pointer for FIFO 19 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_19_READ_ADDR  0x01023298 /* [RW][64] Read Pointer for FIFO 19 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_20_BASE_ADDR  0x010232a0 /* [RW][64] Base Address for FIFO 20 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_20_END_ADDR   0x010232a8 /* [RW][64] End Address for FIFO 20 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_20_WRITE_ADDR 0x010232b0 /* [RW][64] Write Pointer for FIFO 20 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_20_READ_ADDR  0x010232b8 /* [RW][64] Read Pointer for FIFO 20 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_21_BASE_ADDR  0x010232c0 /* [RW][64] Base Address for FIFO 21 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_21_END_ADDR   0x010232c8 /* [RW][64] End Address for FIFO 21 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_21_WRITE_ADDR 0x010232d0 /* [RW][64] Write Pointer for FIFO 21 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_21_READ_ADDR  0x010232d8 /* [RW][64] Read Pointer for FIFO 21 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_22_BASE_ADDR  0x010232e0 /* [RW][64] Base Address for FIFO 22 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_22_END_ADDR   0x010232e8 /* [RW][64] End Address for FIFO 22 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_22_WRITE_ADDR 0x010232f0 /* [RW][64] Write Pointer for FIFO 22 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_22_READ_ADDR  0x010232f8 /* [RW][64] Read Pointer for FIFO 22 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_23_BASE_ADDR  0x01023300 /* [RW][64] Base Address for FIFO 23 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_23_END_ADDR   0x01023308 /* [RW][64] End Address for FIFO 23 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_23_WRITE_ADDR 0x01023310 /* [RW][64] Write Pointer for FIFO 23 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_23_READ_ADDR  0x01023318 /* [RW][64] Read Pointer for FIFO 23 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_24_BASE_ADDR  0x01023320 /* [RW][64] Base Address for FIFO 24 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_24_END_ADDR   0x01023328 /* [RW][64] End Address for FIFO 24 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_24_WRITE_ADDR 0x01023330 /* [RW][64] Write Pointer for FIFO 24 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_24_READ_ADDR  0x01023338 /* [RW][64] Read Pointer for FIFO 24 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_25_BASE_ADDR  0x01023340 /* [RW][64] Base Address for FIFO 25 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_25_END_ADDR   0x01023348 /* [RW][64] End Address for FIFO 25 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_25_WRITE_ADDR 0x01023350 /* [RW][64] Write Pointer for FIFO 25 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_25_READ_ADDR  0x01023358 /* [RW][64] Read Pointer for FIFO 25 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_26_BASE_ADDR  0x01023360 /* [RW][64] Base Address for FIFO 26 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_26_END_ADDR   0x01023368 /* [RW][64] End Address for FIFO 26 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_26_WRITE_ADDR 0x01023370 /* [RW][64] Write Pointer for FIFO 26 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_26_READ_ADDR  0x01023378 /* [RW][64] Read Pointer for FIFO 26 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_27_BASE_ADDR  0x01023380 /* [RW][64] Base Address for FIFO 27 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_27_END_ADDR   0x01023388 /* [RW][64] End Address for FIFO 27 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_27_WRITE_ADDR 0x01023390 /* [RW][64] Write Pointer for FIFO 27 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_27_READ_ADDR  0x01023398 /* [RW][64] Read Pointer for FIFO 27 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_28_BASE_ADDR  0x010233a0 /* [RW][64] Base Address for FIFO 28 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_28_END_ADDR   0x010233a8 /* [RW][64] End Address for FIFO 28 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_28_WRITE_ADDR 0x010233b0 /* [RW][64] Write Pointer for FIFO 28 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_28_READ_ADDR  0x010233b8 /* [RW][64] Read Pointer for FIFO 28 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_29_BASE_ADDR  0x010233c0 /* [RW][64] Base Address for FIFO 29 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_29_END_ADDR   0x010233c8 /* [RW][64] End Address for FIFO 29 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_29_WRITE_ADDR 0x010233d0 /* [RW][64] Write Pointer for FIFO 29 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_29_READ_ADDR  0x010233d8 /* [RW][64] Read Pointer for FIFO 29 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_30_BASE_ADDR  0x010233e0 /* [RW][64] Base Address for FIFO 30 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_30_END_ADDR   0x010233e8 /* [RW][64] End Address for FIFO 30 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_30_WRITE_ADDR 0x010233f0 /* [RW][64] Write Pointer for FIFO 30 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_30_READ_ADDR  0x010233f8 /* [RW][64] Read Pointer for FIFO 30 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_31_BASE_ADDR  0x01023400 /* [RW][64] Base Address for FIFO 31 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_31_END_ADDR   0x01023408 /* [RW][64] End Address for FIFO 31 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_31_WRITE_ADDR 0x01023410 /* [RW][64] Write Pointer for FIFO 31 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_31_READ_ADDR  0x01023418 /* [RW][64] Read Pointer for FIFO 31 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_32_BASE_ADDR  0x01023420 /* [RW][64] Base Address for FIFO 32 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_32_END_ADDR   0x01023428 /* [RW][64] End Address for FIFO 32 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_32_WRITE_ADDR 0x01023430 /* [RW][64] Write Pointer for FIFO 32 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_32_READ_ADDR  0x01023438 /* [RW][64] Read Pointer for FIFO 32 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_33_BASE_ADDR  0x01023440 /* [RW][64] Base Address for FIFO 33 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_33_END_ADDR   0x01023448 /* [RW][64] End Address for FIFO 33 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_33_WRITE_ADDR 0x01023450 /* [RW][64] Write Pointer for FIFO 33 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_33_READ_ADDR  0x01023458 /* [RW][64] Read Pointer for FIFO 33 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_34_BASE_ADDR  0x01023460 /* [RW][64] Base Address for FIFO 34 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_34_END_ADDR   0x01023468 /* [RW][64] End Address for FIFO 34 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_34_WRITE_ADDR 0x01023470 /* [RW][64] Write Pointer for FIFO 34 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_34_READ_ADDR  0x01023478 /* [RW][64] Read Pointer for FIFO 34 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_35_BASE_ADDR  0x01023480 /* [RW][64] Base Address for FIFO 35 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_35_END_ADDR   0x01023488 /* [RW][64] End Address for FIFO 35 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_35_WRITE_ADDR 0x01023490 /* [RW][64] Write Pointer for FIFO 35370 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_35_READ_ADDR  0x01023498 /* [RW][64] Read Pointer for FIFO 35 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_36_BASE_ADDR  0x010234a0 /* [RW][64] Base Address for FIFO 36 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_36_END_ADDR   0x010234a8 /* [RW][64] End Address for FIFO 36 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_36_WRITE_ADDR 0x010234b0 /* [RW][64] Write Pointer for FIFO 36 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_36_READ_ADDR  0x010234b8 /* [RW][64] Read Pointer for FIFO 36 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_37_BASE_ADDR  0x010234c0 /* [RW][64] Base Address for FIFO 37 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_37_END_ADDR   0x010234c8 /* [RW][64] End Address for FIFO 37 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_37_WRITE_ADDR 0x010234d0 /* [RW][64] Write Pointer for FIFO 37 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_37_READ_ADDR  0x010234d8 /* [RW][64] Read Pointer for FIFO 37 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_38_BASE_ADDR  0x010234e0 /* [RW][64] Base Address for FIFO 38 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_38_END_ADDR   0x010234e8 /* [RW][64] End Address for FIFO 38 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_38_WRITE_ADDR 0x010234f0 /* [RW][64] Write Pointer for FIFO 38 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_38_READ_ADDR  0x010234f8 /* [RW][64] Read Pointer for FIFO 38 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_39_BASE_ADDR  0x01023500 /* [RW][64] Base Address for FIFO 39 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_39_END_ADDR   0x01023508 /* [RW][64] End Address for FIFO 39 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_39_WRITE_ADDR 0x01023510 /* [RW][64] Write Pointer for FIFO 39 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_39_READ_ADDR  0x01023518 /* [RW][64] Read Pointer for FIFO 39 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_40_BASE_ADDR  0x01023520 /* [RW][64] Base Address for FIFO 40 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_40_END_ADDR   0x01023528 /* [RW][64] End Address for FIFO 40 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_40_WRITE_ADDR 0x01023530 /* [RW][64] Write Pointer for FIFO 40 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_40_READ_ADDR  0x01023538 /* [RW][64] Read Pointer for FIFO 40 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_41_BASE_ADDR  0x01023540 /* [RW][64] Base Address for FIFO 41 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_41_END_ADDR   0x01023548 /* [RW][64] End Address for FIFO 41 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_41_WRITE_ADDR 0x01023550 /* [RW][64] Write Pointer for FIFO 41 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_41_READ_ADDR  0x01023558 /* [RW][64] Read Pointer for FIFO 41 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_42_BASE_ADDR  0x01023560 /* [RW][64] Base Address for FIFO 42 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_42_END_ADDR   0x01023568 /* [RW][64] End Address for FIFO 42 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_42_WRITE_ADDR 0x01023570 /* [RW][64] Write Pointer for FIFO 42 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_42_READ_ADDR  0x01023578 /* [RW][64] Read Pointer for FIFO 42 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_43_BASE_ADDR  0x01023580 /* [RW][64] Base Address for FIFO 43 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_43_END_ADDR   0x01023588 /* [RW][64] End Address for FIFO 43 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_43_WRITE_ADDR 0x01023590 /* [RW][64] Write Pointer for FIFO 43 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_43_READ_ADDR  0x01023598 /* [RW][64] Read Pointer for FIFO 43 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_44_BASE_ADDR  0x010235a0 /* [RW][64] Base Address for FIFO 44 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_44_END_ADDR   0x010235a8 /* [RW][64] End Address for FIFO 44 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_44_WRITE_ADDR 0x010235b0 /* [RW][64] Write Pointer for FIFO 44 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_44_READ_ADDR  0x010235b8 /* [RW][64] Read Pointer for FIFO 44 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_45_BASE_ADDR  0x010235c0 /* [RW][64] Base Address for FIFO 45 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_45_END_ADDR   0x010235c8 /* [RW][64] End Address for FIFO 45 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_45_WRITE_ADDR 0x010235d0 /* [RW][64] Write Pointer for FIFO 45 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_45_READ_ADDR  0x010235d8 /* [RW][64] Read Pointer for FIFO 45 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_46_BASE_ADDR  0x010235e0 /* [RW][64] Base Address for FIFO 46 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_46_END_ADDR   0x010235e8 /* [RW][64] End Address for FIFO 46 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_46_WRITE_ADDR 0x010235f0 /* [RW][64] Write Pointer for FIFO 46 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_46_READ_ADDR  0x010235f8 /* [RW][64] Read Pointer for FIFO 46 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_47_BASE_ADDR  0x01023600 /* [RW][64] Base Address for FIFO 47 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_47_END_ADDR   0x01023608 /* [RW][64] End Address for FIFO 47 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_47_WRITE_ADDR 0x01023610 /* [RW][64] Write Pointer for FIFO 47 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_47_READ_ADDR  0x01023618 /* [RW][64] Read Pointer for FIFO 47 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_48_BASE_ADDR  0x01023620 /* [RW][64] Base Address for FIFO 48 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_48_END_ADDR   0x01023628 /* [RW][64] End Address for FIFO 48 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_48_WRITE_ADDR 0x01023630 /* [RW][64] Write Pointer for FIFO 48 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_48_READ_ADDR  0x01023638 /* [RW][64] Read Pointer for FIFO 48 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_49_BASE_ADDR  0x01023640 /* [RW][64] Base Address for FIFO 49 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_49_END_ADDR   0x01023648 /* [RW][64] End Address for FIFO 49 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_49_WRITE_ADDR 0x01023650 /* [RW][64] Write Pointer for FIFO 49 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_49_READ_ADDR  0x01023658 /* [RW][64] Read Pointer for FIFO 49 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_50_BASE_ADDR  0x01023660 /* [RW][64] Base Address for FIFO 50 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_50_END_ADDR   0x01023668 /* [RW][64] End Address for FIFO 50 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_50_WRITE_ADDR 0x01023670 /* [RW][64] Write Pointer for FIFO 50 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_50_READ_ADDR  0x01023678 /* [RW][64] Read Pointer for FIFO 50 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_51_BASE_ADDR  0x01023680 /* [RW][64] Base Address for FIFO 51 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_51_END_ADDR   0x01023688 /* [RW][64] End Address for FIFO 51 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_51_WRITE_ADDR 0x01023690 /* [RW][64] Write Pointer for FIFO 51 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_51_READ_ADDR  0x01023698 /* [RW][64] Read Pointer for FIFO 51 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_52_BASE_ADDR  0x010236a0 /* [RW][64] Base Address for FIFO 52 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_52_END_ADDR   0x010236a8 /* [RW][64] End Address for FIFO 52 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_52_WRITE_ADDR 0x010236b0 /* [RW][64] Write Pointer for FIFO 52 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_52_READ_ADDR  0x010236b8 /* [RW][64] Read Pointer for FIFO 52 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_53_BASE_ADDR  0x010236c0 /* [RW][64] Base Address for FIFO 53 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_53_END_ADDR   0x010236c8 /* [RW][64] End Address for FIFO 53 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_53_WRITE_ADDR 0x010236d0 /* [RW][64] Write Pointer for FIFO 53 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_53_READ_ADDR  0x010236d8 /* [RW][64] Read Pointer for FIFO 53 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_54_BASE_ADDR  0x010236e0 /* [RW][64] Base Address for FIFO 54 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_54_END_ADDR   0x010236e8 /* [RW][64] End Address for FIFO 54 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_54_WRITE_ADDR 0x010236f0 /* [RW][64] Write Pointer for FIFO 54 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_54_READ_ADDR  0x010236f8 /* [RW][64] Read Pointer for FIFO 54 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_55_BASE_ADDR  0x01023700 /* [RW][64] Base Address for FIFO 55 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_55_END_ADDR   0x01023708 /* [RW][64] End Address for FIFO 55 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_55_WRITE_ADDR 0x01023710 /* [RW][64] Write Pointer for FIFO 55 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_55_READ_ADDR  0x01023718 /* [RW][64] Read Pointer for FIFO 55 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_56_BASE_ADDR  0x01023720 /* [RW][64] Base Address for FIFO 56 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_56_END_ADDR   0x01023728 /* [RW][64] End Address for FIFO 56 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_56_WRITE_ADDR 0x01023730 /* [RW][64] Write Pointer for FIFO 56 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_56_READ_ADDR  0x01023738 /* [RW][64] Read Pointer for FIFO 56 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_57_BASE_ADDR  0x01023740 /* [RW][64] Base Address for FIFO 57 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_57_END_ADDR   0x01023748 /* [RW][64] End Address for FIFO 57 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_57_WRITE_ADDR 0x01023750 /* [RW][64] Write Pointer for FIFO 57 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_57_READ_ADDR  0x01023758 /* [RW][64] Read Pointer for FIFO 57 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_58_BASE_ADDR  0x01023760 /* [RW][64] Base Address for FIFO 58 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_58_END_ADDR   0x01023768 /* [RW][64] End Address for FIFO 58 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_58_WRITE_ADDR 0x01023770 /* [RW][64] Write Pointer for FIFO 58 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_58_READ_ADDR  0x01023778 /* [RW][64] Read Pointer for FIFO 58 */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_BASE_ADDR 0x01023780 /* [RW][64] Base Address for FIFO DRAMLOGS */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_END_ADDR 0x01023788 /* [RW][64] End Address for FIFO DRAMLOGS */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_WRITE_ADDR 0x01023790 /* [RW][64] Write Pointer for FIFO DRAMLOGS */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_READ_ADDR 0x01023798 /* [RW][64] Read Pointer for FIFO DRAMLOGS */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_BASE_ADDR 0x010237a0 /* [RW][64] Base Address for FIFO UART */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_END_ADDR 0x010237a8 /* [RW][64] End Address for FIFO UART */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_WRITE_ADDR 0x010237b0 /* [RW][64] Write Pointer for FIFO UART */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_READ_ADDR 0x010237b8 /* [RW][64] Read Pointer for FIFO UART */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_BASE_ADDR 0x010237c0 /* [RW][64] Base Address for FIFO COREDUMP */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_END_ADDR 0x010237c8 /* [RW][64] End Address for FIFO COREDUMP */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_WRITE_ADDR 0x010237d0 /* [RW][64] Write Pointer for FIFO COREDUMP */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_READ_ADDR 0x010237d8 /* [RW][64] Read Pointer for FIFO COREDUMP */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_BASE_ADDR 0x010237e0 /* [RW][64] Base Address for FIFO TARGETPRINT */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_END_ADDR 0x010237e8 /* [RW][64] End Address for FIFO TARGETPRINT */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_WRITE_ADDR 0x010237f0 /* [RW][64] Write Pointer for FIFO TARGETPRINT */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_READ_ADDR 0x010237f8 /* [RW][64] Read Pointer for FIFO TARGETPRINT */
#define BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_0_DEBUG 0x01023800 /* [RW][64] Base Address for FIFO 0 DEBUG */
#define BCHP_RAAGA_DSP_FW_CFG_END_ADDR_FIFO_0_DEBUG 0x01023808 /* [RW][64] End Address for FIFO 0 DEBUG */
#define BCHP_RAAGA_DSP_FW_CFG_WRITE_ADDR_FIFO_0_DEBUG 0x01023810 /* [RW][64] Write Pointer for FIFO 0 DEBUG */
#define BCHP_RAAGA_DSP_FW_CFG_READ_ADDR_FIFO_0_DEBUG 0x01023818 /* [RW][64] Read Pointer for FIFO 0 DEBUG */
#define BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_1_DEBUG 0x01023820 /* [RW][64] Base Address for FIFO 1 DEBUG */
#define BCHP_RAAGA_DSP_FW_CFG_END_ADDR_FIFO_1_DEBUG 0x01023828 /* [RW][64] End Address for FIFO 1 DEBUG */
#define BCHP_RAAGA_DSP_FW_CFG_WRITE_ADDR_FIFO_1_DEBUG 0x01023830 /* [RW][64] Write Pointer for FIFO 1 DEBUG */
#define BCHP_RAAGA_DSP_FW_CFG_READ_ADDR_FIFO_1_DEBUG 0x01023838 /* [RW][64] Read Pointer for FIFO 1 DEBUG */
#define BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_2_DEBUG 0x01023840 /* [RW][64] Base Address for FIFO 2 DEBUG */
#define BCHP_RAAGA_DSP_FW_CFG_END_ADDR_FIFO_2_DEBUG 0x01023848 /* [RW][64] End Address for FIFO 2 DEBUG */
#define BCHP_RAAGA_DSP_FW_CFG_WRITE_ADDR_FIFO_2_DEBUG 0x01023850 /* [RW][64] Write Pointer for FIFO 2 DEBUG */
#define BCHP_RAAGA_DSP_FW_CFG_READ_ADDR_FIFO_2_DEBUG 0x01023858 /* [RW][64] Read Pointer for FIFO 2 DEBUG */

/***************************************************************************
 *HOST2DSPCMD_FIFO0_BASEADDR - FIFO 0 BASEADDRESS
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: HOST2DSPCMD_FIFO0_BASEADDR :: HOST2DSP_COMMAND_FIFO_0_ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO0_BASEADDR_HOST2DSP_COMMAND_FIFO_0_ADDR_MASK BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO0_BASEADDR_HOST2DSP_COMMAND_FIFO_0_ADDR_SHIFT 0

/***************************************************************************
 *HOST2DSPCMD_FIFO_ID - FIFO ID Containing HOST2DSP Command
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: HOST2DSPCMD_FIFO_ID :: HOST2DSP_COMMAND_FIFO_INDEX [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO_ID_HOST2DSP_COMMAND_FIFO_INDEX_MASK BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO_ID_HOST2DSP_COMMAND_FIFO_INDEX_SHIFT 0

/***************************************************************************
 *SW_UNUSED1 - UNUSED REGISTER SPACE
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: SW_UNUSED1 :: SW_UNUSED [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_SW_UNUSED1_SW_UNUSED_MASK            BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_SW_UNUSED1_SW_UNUSED_SHIFT           0

/***************************************************************************
 *SW_UNUSED2 - UNUSED REGISTER SPACE
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: SW_UNUSED2 :: SW_UNUSED [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_SW_UNUSED2_SW_UNUSED_MASK            BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_SW_UNUSED2_SW_UNUSED_SHIFT           0

/***************************************************************************
 *FIFO_0_BASE_ADDR - Base Address for FIFO 0
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_0_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_0_END_ADDR - End Address for FIFO 0
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_0_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_0_END_ADDR_ADDR_MASK            BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_0_END_ADDR_ADDR_SHIFT           0

/***************************************************************************
 *FIFO_0_WRITE_ADDR - Write Pointer for FIFO 0
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_0_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_0_WRITE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_0_WRITE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_0_READ_ADDR - Read Pointer for FIFO 0
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_0_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_0_READ_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_0_READ_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_1_BASE_ADDR - Base Address for FIFO 1
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_1_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_1_END_ADDR - End Address for FIFO 1
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_1_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_1_END_ADDR_ADDR_MASK            BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_1_END_ADDR_ADDR_SHIFT           0

/***************************************************************************
 *FIFO_1_WRITE_ADDR - Write Pointer for FIFO 1
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_1_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_1_WRITE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_1_WRITE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_1_READ_ADDR - Read Pointer for FIFO 1
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_1_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_1_READ_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_1_READ_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_2_BASE_ADDR - Base Address for FIFO 2
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_2_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_2_BASE_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_2_BASE_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_2_END_ADDR - End Address for FIFO 2
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_2_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_2_END_ADDR_ADDR_MASK            BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_2_END_ADDR_ADDR_SHIFT           0

/***************************************************************************
 *FIFO_2_WRITE_ADDR - Write Pointer for FIFO 2
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_2_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_2_WRITE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_2_WRITE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_2_READ_ADDR - Read Pointer for FIFO 2
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_2_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_2_READ_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_2_READ_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_3_BASE_ADDR - Base Address for FIFO 3
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_3_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_3_BASE_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_3_BASE_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_3_END_ADDR - End Address for FIFO 3
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_3_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_3_END_ADDR_ADDR_MASK            BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_3_END_ADDR_ADDR_SHIFT           0

/***************************************************************************
 *FIFO_3_WRITE_ADDR - Write Pointer for FIFO 3
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_3_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_3_WRITE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_3_WRITE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_3_READ_ADDR - Read Pointer for FIFO 3
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_3_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_3_READ_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_3_READ_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_4_BASE_ADDR - Base Address for FIFO 4
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_4_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_4_BASE_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_4_BASE_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_4_END_ADDR - End Address for FIFO 4
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_4_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_4_END_ADDR_ADDR_MASK            BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_4_END_ADDR_ADDR_SHIFT           0

/***************************************************************************
 *FIFO_4_WRITE_ADDR - Write Pointer for FIFO 4
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_4_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_4_WRITE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_4_WRITE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_4_READ_ADDR - Read Pointer for FIFO 4
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_4_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_4_READ_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_4_READ_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_5_BASE_ADDR - Base Address for FIFO 5
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_5_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_5_BASE_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_5_BASE_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_5_END_ADDR - End Address for FIFO 5
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_5_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_5_END_ADDR_ADDR_MASK            BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_5_END_ADDR_ADDR_SHIFT           0

/***************************************************************************
 *FIFO_5_WRITE_ADDR - Write Pointer for FIFO 5
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_5_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_5_WRITE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_5_WRITE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_5_READ_ADDR - Read Pointer for FIFO 5
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_5_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_5_READ_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_5_READ_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_6_BASE_ADDR - Base Address for FIFO 6
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_6_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_6_BASE_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_6_BASE_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_6_END_ADDR - End Address for FIFO 6
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_6_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_6_END_ADDR_ADDR_MASK            BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_6_END_ADDR_ADDR_SHIFT           0

/***************************************************************************
 *FIFO_6_WRITE_ADDR - Write Pointer for FIFO 6
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_6_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_6_WRITE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_6_WRITE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_6_READ_ADDR - Read Pointer for FIFO 6
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_6_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_6_READ_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_6_READ_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_7_BASE_ADDR - Base Address for FIFO 7
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_7_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_7_BASE_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_7_BASE_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_7_END_ADDR - End Address for FIFO 7
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_7_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_7_END_ADDR_ADDR_MASK            BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_7_END_ADDR_ADDR_SHIFT           0

/***************************************************************************
 *FIFO_7_WRITE_ADDR - Write Pointer for FIFO 7
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_7_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_7_WRITE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_7_WRITE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_7_READ_ADDR - Read Pointer for FIFO 7
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_7_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_7_READ_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_7_READ_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_8_BASE_ADDR - Base Address for FIFO 8
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_8_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_8_BASE_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_8_BASE_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_8_END_ADDR - End Address for FIFO 8
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_8_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_8_END_ADDR_ADDR_MASK            BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_8_END_ADDR_ADDR_SHIFT           0

/***************************************************************************
 *FIFO_8_WRITE_ADDR - Write Pointer for FIFO 8
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_8_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_8_WRITE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_8_WRITE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_8_READ_ADDR - Read Pointer for FIFO 8
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_8_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_8_READ_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_8_READ_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_9_BASE_ADDR - Base Address for FIFO 9
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_9_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_9_BASE_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_9_BASE_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_9_END_ADDR - End Address for FIFO 9
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_9_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_9_END_ADDR_ADDR_MASK            BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_9_END_ADDR_ADDR_SHIFT           0

/***************************************************************************
 *FIFO_9_WRITE_ADDR - Write Pointer for FIFO 9
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_9_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_9_WRITE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_9_WRITE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_9_READ_ADDR - Read Pointer for FIFO 9
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_9_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_9_READ_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_9_READ_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_10_BASE_ADDR - Base Address for FIFO 10
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_10_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_10_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_10_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_10_END_ADDR - End Address for FIFO 10
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_10_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_10_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_10_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_10_WRITE_ADDR - Write Pointer for FIFO 10
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_10_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_10_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_10_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_10_READ_ADDR - Read Pointer for FIFO 10
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_10_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_10_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_10_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_11_BASE_ADDR - Base Address for FIFO 11
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_11_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_11_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_11_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_11_END_ADDR - End Address for FIFO 11
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_11_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_11_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_11_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_11_WRITE_ADDR - Write Pointer for FIFO 11
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_11_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_11_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_11_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_11_READ_ADDR - Read Pointer for FIFO 11
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_11_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_11_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_11_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_12_BASE_ADDR - Base Address for FIFO 12
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_12_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_12_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_12_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_12_END_ADDR - End Address for FIFO 12
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_12_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_12_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_12_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_12_WRITE_ADDR - Write Pointer for FIFO 12
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_12_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_12_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_12_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_12_READ_ADDR - Read Pointer for FIFO 12
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_12_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_12_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_12_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_13_BASE_ADDR - Base Address for FIFO 13
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_13_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_13_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_13_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_13_END_ADDR - End Address for FIFO 13
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_13_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_13_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_13_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_13_WRITE_ADDR - Write Pointer for FIFO 13
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_13_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_13_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_13_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_13_READ_ADDR - Read Pointer for FIFO 13
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_13_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_13_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_13_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_14_BASE_ADDR - Base Address for FIFO 14
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_14_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_14_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_14_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_14_END_ADDR - End Address for FIFO 14
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_14_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_14_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_14_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_14_WRITE_ADDR - Write Pointer for FIFO 14
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_14_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_14_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_14_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_14_READ_ADDR - Read Pointer for FIFO 14
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_14_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_14_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_14_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_15_BASE_ADDR - Base Address for FIFO 15
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_15_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_15_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_15_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_15_END_ADDR - End Address for FIFO 15
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_15_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_15_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_15_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_15_WRITE_ADDR - Write Pointer for FIFO 15
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_15_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_15_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_15_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_15_READ_ADDR - Read Pointer for FIFO 15
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_15_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_15_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_15_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_16_BASE_ADDR - Base Address for FIFO 16
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_16_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_16_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_16_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_16_END_ADDR - End Address for FIFO 16
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_16_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_16_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_16_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_16_WRITE_ADDR - Write Pointer for FIFO 16
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_16_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_16_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_16_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_16_READ_ADDR - Read Pointer for FIFO 16
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_16_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_16_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_16_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_17_BASE_ADDR - Base Address for FIFO 17
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_17_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_17_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_17_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_17_END_ADDR - End Address for FIFO 17
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_17_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_17_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_17_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_17_WRITE_ADDR - Write Pointer for FIFO 17
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_17_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_17_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_17_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_17_READ_ADDR - Read Pointer for FIFO 17
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_17_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_17_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_17_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_18_BASE_ADDR - Base Address for FIFO 18
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_18_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_18_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_18_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_18_END_ADDR - End Address for FIFO 18
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_18_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_18_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_18_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_18_WRITE_ADDR - Write Pointer for FIFO 18
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_18_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_18_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_18_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_18_READ_ADDR - Read Pointer for FIFO 18
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_18_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_18_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_18_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_19_BASE_ADDR - Base Address for FIFO 19
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_19_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_19_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_19_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_19_END_ADDR - End Address for FIFO 19
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_19_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_19_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_19_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_19_WRITE_ADDR - Write Pointer for FIFO 19
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_19_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_19_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_19_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_19_READ_ADDR - Read Pointer for FIFO 19
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_19_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_19_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_19_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_20_BASE_ADDR - Base Address for FIFO 20
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_20_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_20_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_20_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_20_END_ADDR - End Address for FIFO 20
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_20_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_20_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_20_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_20_WRITE_ADDR - Write Pointer for FIFO 20
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_20_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_20_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_20_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_20_READ_ADDR - Read Pointer for FIFO 20
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_20_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_20_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_20_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_21_BASE_ADDR - Base Address for FIFO 21
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_21_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_21_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_21_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_21_END_ADDR - End Address for FIFO 21
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_21_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_21_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_21_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_21_WRITE_ADDR - Write Pointer for FIFO 21
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_21_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_21_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_21_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_21_READ_ADDR - Read Pointer for FIFO 21
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_21_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_21_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_21_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_22_BASE_ADDR - Base Address for FIFO 22
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_22_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_22_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_22_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_22_END_ADDR - End Address for FIFO 22
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_22_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_22_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_22_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_22_WRITE_ADDR - Write Pointer for FIFO 22
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_22_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_22_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_22_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_22_READ_ADDR - Read Pointer for FIFO 22
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_22_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_22_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_22_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_23_BASE_ADDR - Base Address for FIFO 23
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_23_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_23_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_23_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_23_END_ADDR - End Address for FIFO 23
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_23_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_23_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_23_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_23_WRITE_ADDR - Write Pointer for FIFO 23
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_23_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_23_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_23_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_23_READ_ADDR - Read Pointer for FIFO 23
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_23_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_23_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_23_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_24_BASE_ADDR - Base Address for FIFO 24
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_24_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_24_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_24_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_24_END_ADDR - End Address for FIFO 24
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_24_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_24_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_24_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_24_WRITE_ADDR - Write Pointer for FIFO 24
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_24_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_24_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_24_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_24_READ_ADDR - Read Pointer for FIFO 24
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_24_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_24_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_24_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_25_BASE_ADDR - Base Address for FIFO 25
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_25_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_25_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_25_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_25_END_ADDR - End Address for FIFO 25
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_25_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_25_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_25_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_25_WRITE_ADDR - Write Pointer for FIFO 25
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_25_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_25_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_25_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_25_READ_ADDR - Read Pointer for FIFO 25
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_25_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_25_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_25_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_26_BASE_ADDR - Base Address for FIFO 26
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_26_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_26_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_26_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_26_END_ADDR - End Address for FIFO 26
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_26_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_26_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_26_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_26_WRITE_ADDR - Write Pointer for FIFO 26
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_26_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_26_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_26_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_26_READ_ADDR - Read Pointer for FIFO 26
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_26_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_26_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_26_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_27_BASE_ADDR - Base Address for FIFO 27
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_27_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_27_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_27_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_27_END_ADDR - End Address for FIFO 27
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_27_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_27_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_27_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_27_WRITE_ADDR - Write Pointer for FIFO 27
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_27_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_27_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_27_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_27_READ_ADDR - Read Pointer for FIFO 27
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_27_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_27_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_27_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_28_BASE_ADDR - Base Address for FIFO 28
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_28_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_28_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_28_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_28_END_ADDR - End Address for FIFO 28
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_28_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_28_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_28_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_28_WRITE_ADDR - Write Pointer for FIFO 28
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_28_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_28_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_28_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_28_READ_ADDR - Read Pointer for FIFO 28
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_28_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_28_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_28_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_29_BASE_ADDR - Base Address for FIFO 29
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_29_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_29_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_29_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_29_END_ADDR - End Address for FIFO 29
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_29_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_29_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_29_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_29_WRITE_ADDR - Write Pointer for FIFO 29
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_29_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_29_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_29_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_29_READ_ADDR - Read Pointer for FIFO 29
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_29_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_29_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_29_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_30_BASE_ADDR - Base Address for FIFO 30
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_30_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_30_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_30_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_30_END_ADDR - End Address for FIFO 30
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_30_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_30_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_30_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_30_WRITE_ADDR - Write Pointer for FIFO 30
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_30_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_30_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_30_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_30_READ_ADDR - Read Pointer for FIFO 30
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_30_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_30_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_30_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_31_BASE_ADDR - Base Address for FIFO 31
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_31_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_31_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_31_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_31_END_ADDR - End Address for FIFO 31
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_31_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_31_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_31_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_31_WRITE_ADDR - Write Pointer for FIFO 31
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_31_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_31_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_31_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_31_READ_ADDR - Read Pointer for FIFO 31
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_31_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_31_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_31_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_32_BASE_ADDR - Base Address for FIFO 32
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_32_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_32_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_32_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_32_END_ADDR - End Address for FIFO 32
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_32_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_32_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_32_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_32_WRITE_ADDR - Write Pointer for FIFO 32
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_32_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_32_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_32_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_32_READ_ADDR - Read Pointer for FIFO 32
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_32_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_32_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_32_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_33_BASE_ADDR - Base Address for FIFO 33
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_33_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_33_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_33_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_33_END_ADDR - End Address for FIFO 33
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_33_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_33_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_33_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_33_WRITE_ADDR - Write Pointer for FIFO 33
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_33_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_33_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_33_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_33_READ_ADDR - Read Pointer for FIFO 33
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_33_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_33_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_33_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_34_BASE_ADDR - Base Address for FIFO 34
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_34_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_34_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_34_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_34_END_ADDR - End Address for FIFO 34
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_34_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_34_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_34_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_34_WRITE_ADDR - Write Pointer for FIFO 34
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_34_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_34_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_34_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_34_READ_ADDR - Read Pointer for FIFO 34
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_34_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_34_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_34_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_35_BASE_ADDR - Base Address for FIFO 35
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_35_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_35_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_35_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_35_END_ADDR - End Address for FIFO 35
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_35_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_35_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_35_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_35_WRITE_ADDR - Write Pointer for FIFO 35370
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_35_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_35_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_35_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_35_READ_ADDR - Read Pointer for FIFO 35
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_35_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_35_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_35_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_36_BASE_ADDR - Base Address for FIFO 36
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_36_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_36_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_36_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_36_END_ADDR - End Address for FIFO 36
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_36_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_36_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_36_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_36_WRITE_ADDR - Write Pointer for FIFO 36
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_36_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_36_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_36_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_36_READ_ADDR - Read Pointer for FIFO 36
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_36_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_36_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_36_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_37_BASE_ADDR - Base Address for FIFO 37
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_37_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_37_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_37_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_37_END_ADDR - End Address for FIFO 37
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_37_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_37_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_37_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_37_WRITE_ADDR - Write Pointer for FIFO 37
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_37_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_37_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_37_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_37_READ_ADDR - Read Pointer for FIFO 37
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_37_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_37_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_37_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_38_BASE_ADDR - Base Address for FIFO 38
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_38_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_38_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_38_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_38_END_ADDR - End Address for FIFO 38
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_38_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_38_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_38_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_38_WRITE_ADDR - Write Pointer for FIFO 38
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_38_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_38_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_38_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_38_READ_ADDR - Read Pointer for FIFO 38
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_38_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_38_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_38_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_39_BASE_ADDR - Base Address for FIFO 39
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_39_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_39_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_39_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_39_END_ADDR - End Address for FIFO 39
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_39_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_39_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_39_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_39_WRITE_ADDR - Write Pointer for FIFO 39
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_39_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_39_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_39_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_39_READ_ADDR - Read Pointer for FIFO 39
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_39_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_39_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_39_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_40_BASE_ADDR - Base Address for FIFO 40
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_40_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_40_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_40_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_40_END_ADDR - End Address for FIFO 40
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_40_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_40_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_40_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_40_WRITE_ADDR - Write Pointer for FIFO 40
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_40_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_40_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_40_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_40_READ_ADDR - Read Pointer for FIFO 40
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_40_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_40_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_40_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_41_BASE_ADDR - Base Address for FIFO 41
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_41_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_41_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_41_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_41_END_ADDR - End Address for FIFO 41
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_41_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_41_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_41_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_41_WRITE_ADDR - Write Pointer for FIFO 41
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_41_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_41_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_41_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_41_READ_ADDR - Read Pointer for FIFO 41
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_41_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_41_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_41_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_42_BASE_ADDR - Base Address for FIFO 42
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_42_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_42_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_42_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_42_END_ADDR - End Address for FIFO 42
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_42_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_42_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_42_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_42_WRITE_ADDR - Write Pointer for FIFO 42
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_42_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_42_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_42_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_42_READ_ADDR - Read Pointer for FIFO 42
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_42_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_42_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_42_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_43_BASE_ADDR - Base Address for FIFO 43
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_43_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_43_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_43_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_43_END_ADDR - End Address for FIFO 43
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_43_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_43_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_43_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_43_WRITE_ADDR - Write Pointer for FIFO 43
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_43_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_43_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_43_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_43_READ_ADDR - Read Pointer for FIFO 43
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_43_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_43_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_43_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_44_BASE_ADDR - Base Address for FIFO 44
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_44_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_44_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_44_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_44_END_ADDR - End Address for FIFO 44
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_44_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_44_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_44_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_44_WRITE_ADDR - Write Pointer for FIFO 44
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_44_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_44_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_44_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_44_READ_ADDR - Read Pointer for FIFO 44
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_44_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_44_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_44_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_45_BASE_ADDR - Base Address for FIFO 45
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_45_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_45_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_45_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_45_END_ADDR - End Address for FIFO 45
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_45_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_45_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_45_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_45_WRITE_ADDR - Write Pointer for FIFO 45
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_45_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_45_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_45_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_45_READ_ADDR - Read Pointer for FIFO 45
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_45_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_45_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_45_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_46_BASE_ADDR - Base Address for FIFO 46
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_46_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_46_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_46_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_46_END_ADDR - End Address for FIFO 46
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_46_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_46_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_46_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_46_WRITE_ADDR - Write Pointer for FIFO 46
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_46_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_46_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_46_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_46_READ_ADDR - Read Pointer for FIFO 46
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_46_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_46_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_46_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_47_BASE_ADDR - Base Address for FIFO 47
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_47_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_47_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_47_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_47_END_ADDR - End Address for FIFO 47
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_47_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_47_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_47_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_47_WRITE_ADDR - Write Pointer for FIFO 47
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_47_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_47_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_47_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_47_READ_ADDR - Read Pointer for FIFO 47
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_47_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_47_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_47_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_48_BASE_ADDR - Base Address for FIFO 48
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_48_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_48_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_48_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_48_END_ADDR - End Address for FIFO 48
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_48_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_48_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_48_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_48_WRITE_ADDR - Write Pointer for FIFO 48
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_48_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_48_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_48_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_48_READ_ADDR - Read Pointer for FIFO 48
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_48_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_48_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_48_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_49_BASE_ADDR - Base Address for FIFO 49
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_49_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_49_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_49_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_49_END_ADDR - End Address for FIFO 49
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_49_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_49_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_49_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_49_WRITE_ADDR - Write Pointer for FIFO 49
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_49_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_49_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_49_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_49_READ_ADDR - Read Pointer for FIFO 49
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_49_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_49_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_49_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_50_BASE_ADDR - Base Address for FIFO 50
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_50_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_50_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_50_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_50_END_ADDR - End Address for FIFO 50
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_50_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_50_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_50_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_50_WRITE_ADDR - Write Pointer for FIFO 50
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_50_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_50_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_50_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_50_READ_ADDR - Read Pointer for FIFO 50
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_50_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_50_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_50_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_51_BASE_ADDR - Base Address for FIFO 51
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_51_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_51_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_51_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_51_END_ADDR - End Address for FIFO 51
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_51_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_51_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_51_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_51_WRITE_ADDR - Write Pointer for FIFO 51
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_51_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_51_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_51_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_51_READ_ADDR - Read Pointer for FIFO 51
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_51_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_51_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_51_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_52_BASE_ADDR - Base Address for FIFO 52
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_52_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_52_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_52_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_52_END_ADDR - End Address for FIFO 52
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_52_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_52_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_52_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_52_WRITE_ADDR - Write Pointer for FIFO 52
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_52_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_52_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_52_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_52_READ_ADDR - Read Pointer for FIFO 52
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_52_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_52_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_52_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_53_BASE_ADDR - Base Address for FIFO 53
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_53_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_53_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_53_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_53_END_ADDR - End Address for FIFO 53
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_53_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_53_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_53_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_53_WRITE_ADDR - Write Pointer for FIFO 53
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_53_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_53_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_53_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_53_READ_ADDR - Read Pointer for FIFO 53
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_53_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_53_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_53_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_54_BASE_ADDR - Base Address for FIFO 54
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_54_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_54_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_54_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_54_END_ADDR - End Address for FIFO 54
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_54_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_54_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_54_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_54_WRITE_ADDR - Write Pointer for FIFO 54
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_54_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_54_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_54_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_54_READ_ADDR - Read Pointer for FIFO 54
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_54_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_54_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_54_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_55_BASE_ADDR - Base Address for FIFO 55
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_55_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_55_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_55_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_55_END_ADDR - End Address for FIFO 55
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_55_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_55_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_55_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_55_WRITE_ADDR - Write Pointer for FIFO 55
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_55_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_55_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_55_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_55_READ_ADDR - Read Pointer for FIFO 55
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_55_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_55_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_55_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_56_BASE_ADDR - Base Address for FIFO 56
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_56_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_56_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_56_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_56_END_ADDR - End Address for FIFO 56
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_56_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_56_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_56_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_56_WRITE_ADDR - Write Pointer for FIFO 56
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_56_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_56_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_56_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_56_READ_ADDR - Read Pointer for FIFO 56
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_56_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_56_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_56_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_57_BASE_ADDR - Base Address for FIFO 57
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_57_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_57_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_57_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_57_END_ADDR - End Address for FIFO 57
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_57_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_57_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_57_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_57_WRITE_ADDR - Write Pointer for FIFO 57
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_57_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_57_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_57_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_57_READ_ADDR - Read Pointer for FIFO 57
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_57_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_57_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_57_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_58_BASE_ADDR - Base Address for FIFO 58
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_58_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_58_BASE_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_58_BASE_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_58_END_ADDR - End Address for FIFO 58
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_58_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_58_END_ADDR_ADDR_MASK           BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_58_END_ADDR_ADDR_SHIFT          0

/***************************************************************************
 *FIFO_58_WRITE_ADDR - Write Pointer for FIFO 58
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_58_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_58_WRITE_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_58_WRITE_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_58_READ_ADDR - Read Pointer for FIFO 58
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_58_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_58_READ_ADDR_ADDR_MASK          BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_58_READ_ADDR_ADDR_SHIFT         0

/***************************************************************************
 *FIFO_DRAMLOGS_BASE_ADDR - Base Address for FIFO DRAMLOGS
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_DRAMLOGS_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_BASE_ADDR_ADDR_MASK    BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_BASE_ADDR_ADDR_SHIFT   0

/***************************************************************************
 *FIFO_DRAMLOGS_END_ADDR - End Address for FIFO DRAMLOGS
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_DRAMLOGS_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_END_ADDR_ADDR_MASK     BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_END_ADDR_ADDR_SHIFT    0

/***************************************************************************
 *FIFO_DRAMLOGS_WRITE_ADDR - Write Pointer for FIFO DRAMLOGS
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_DRAMLOGS_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_WRITE_ADDR_ADDR_MASK   BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_WRITE_ADDR_ADDR_SHIFT  0

/***************************************************************************
 *FIFO_DRAMLOGS_READ_ADDR - Read Pointer for FIFO DRAMLOGS
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_DRAMLOGS_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_READ_ADDR_ADDR_MASK    BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_DRAMLOGS_READ_ADDR_ADDR_SHIFT   0

/***************************************************************************
 *FIFO_UART_BASE_ADDR - Base Address for FIFO UART
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_UART_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_BASE_ADDR_ADDR_MASK        BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_BASE_ADDR_ADDR_SHIFT       0

/***************************************************************************
 *FIFO_UART_END_ADDR - End Address for FIFO UART
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_UART_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_END_ADDR_ADDR_MASK         BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_END_ADDR_ADDR_SHIFT        0

/***************************************************************************
 *FIFO_UART_WRITE_ADDR - Write Pointer for FIFO UART
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_UART_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_WRITE_ADDR_ADDR_MASK       BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_WRITE_ADDR_ADDR_SHIFT      0

/***************************************************************************
 *FIFO_UART_READ_ADDR - Read Pointer for FIFO UART
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_UART_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_READ_ADDR_ADDR_MASK        BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_UART_READ_ADDR_ADDR_SHIFT       0

/***************************************************************************
 *FIFO_COREDUMP_BASE_ADDR - Base Address for FIFO COREDUMP
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_COREDUMP_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_BASE_ADDR_ADDR_MASK    BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_BASE_ADDR_ADDR_SHIFT   0

/***************************************************************************
 *FIFO_COREDUMP_END_ADDR - End Address for FIFO COREDUMP
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_COREDUMP_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_END_ADDR_ADDR_MASK     BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_END_ADDR_ADDR_SHIFT    0

/***************************************************************************
 *FIFO_COREDUMP_WRITE_ADDR - Write Pointer for FIFO COREDUMP
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_COREDUMP_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_WRITE_ADDR_ADDR_MASK   BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_WRITE_ADDR_ADDR_SHIFT  0

/***************************************************************************
 *FIFO_COREDUMP_READ_ADDR - Read Pointer for FIFO COREDUMP
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_COREDUMP_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_READ_ADDR_ADDR_MASK    BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_COREDUMP_READ_ADDR_ADDR_SHIFT   0

/***************************************************************************
 *FIFO_TARGETPRINT_BASE_ADDR - Base Address for FIFO TARGETPRINT
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_TARGETPRINT_BASE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_BASE_ADDR_ADDR_MASK BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_BASE_ADDR_ADDR_SHIFT 0

/***************************************************************************
 *FIFO_TARGETPRINT_END_ADDR - End Address for FIFO TARGETPRINT
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_TARGETPRINT_END_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_END_ADDR_ADDR_MASK  BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_END_ADDR_ADDR_SHIFT 0

/***************************************************************************
 *FIFO_TARGETPRINT_WRITE_ADDR - Write Pointer for FIFO TARGETPRINT
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_TARGETPRINT_WRITE_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_WRITE_ADDR_ADDR_MASK BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_WRITE_ADDR_ADDR_SHIFT 0

/***************************************************************************
 *FIFO_TARGETPRINT_READ_ADDR - Read Pointer for FIFO TARGETPRINT
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: FIFO_TARGETPRINT_READ_ADDR :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_READ_ADDR_ADDR_MASK BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_FIFO_TARGETPRINT_READ_ADDR_ADDR_SHIFT 0

/***************************************************************************
 *BASE_ADDR_FIFO_0_DEBUG - Base Address for FIFO 0 DEBUG
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: BASE_ADDR_FIFO_0_DEBUG :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_0_DEBUG_ADDR_MASK     BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_0_DEBUG_ADDR_SHIFT    0

/***************************************************************************
 *END_ADDR_FIFO_0_DEBUG - End Address for FIFO 0 DEBUG
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: END_ADDR_FIFO_0_DEBUG :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_END_ADDR_FIFO_0_DEBUG_ADDR_MASK      BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_END_ADDR_FIFO_0_DEBUG_ADDR_SHIFT     0

/***************************************************************************
 *WRITE_ADDR_FIFO_0_DEBUG - Write Pointer for FIFO 0 DEBUG
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: WRITE_ADDR_FIFO_0_DEBUG :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_WRITE_ADDR_FIFO_0_DEBUG_ADDR_MASK    BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_WRITE_ADDR_FIFO_0_DEBUG_ADDR_SHIFT   0

/***************************************************************************
 *READ_ADDR_FIFO_0_DEBUG - Read Pointer for FIFO 0 DEBUG
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: READ_ADDR_FIFO_0_DEBUG :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_READ_ADDR_FIFO_0_DEBUG_ADDR_MASK     BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_READ_ADDR_FIFO_0_DEBUG_ADDR_SHIFT    0

/***************************************************************************
 *BASE_ADDR_FIFO_1_DEBUG - Base Address for FIFO 1 DEBUG
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: BASE_ADDR_FIFO_1_DEBUG :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_1_DEBUG_ADDR_MASK     BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_1_DEBUG_ADDR_SHIFT    0

/***************************************************************************
 *END_ADDR_FIFO_1_DEBUG - End Address for FIFO 1 DEBUG
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: END_ADDR_FIFO_1_DEBUG :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_END_ADDR_FIFO_1_DEBUG_ADDR_MASK      BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_END_ADDR_FIFO_1_DEBUG_ADDR_SHIFT     0

/***************************************************************************
 *WRITE_ADDR_FIFO_1_DEBUG - Write Pointer for FIFO 1 DEBUG
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: WRITE_ADDR_FIFO_1_DEBUG :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_WRITE_ADDR_FIFO_1_DEBUG_ADDR_MASK    BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_WRITE_ADDR_FIFO_1_DEBUG_ADDR_SHIFT   0

/***************************************************************************
 *READ_ADDR_FIFO_1_DEBUG - Read Pointer for FIFO 1 DEBUG
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: READ_ADDR_FIFO_1_DEBUG :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_READ_ADDR_FIFO_1_DEBUG_ADDR_MASK     BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_READ_ADDR_FIFO_1_DEBUG_ADDR_SHIFT    0

/***************************************************************************
 *BASE_ADDR_FIFO_2_DEBUG - Base Address for FIFO 2 DEBUG
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: BASE_ADDR_FIFO_2_DEBUG :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_2_DEBUG_ADDR_MASK     BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_BASE_ADDR_FIFO_2_DEBUG_ADDR_SHIFT    0

/***************************************************************************
 *END_ADDR_FIFO_2_DEBUG - End Address for FIFO 2 DEBUG
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: END_ADDR_FIFO_2_DEBUG :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_END_ADDR_FIFO_2_DEBUG_ADDR_MASK      BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_END_ADDR_FIFO_2_DEBUG_ADDR_SHIFT     0

/***************************************************************************
 *WRITE_ADDR_FIFO_2_DEBUG - Write Pointer for FIFO 2 DEBUG
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: WRITE_ADDR_FIFO_2_DEBUG :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_WRITE_ADDR_FIFO_2_DEBUG_ADDR_MASK    BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_WRITE_ADDR_FIFO_2_DEBUG_ADDR_SHIFT   0

/***************************************************************************
 *READ_ADDR_FIFO_2_DEBUG - Read Pointer for FIFO 2 DEBUG
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: READ_ADDR_FIFO_2_DEBUG :: ADDR [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_READ_ADDR_FIFO_2_DEBUG_ADDR_MASK     BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_READ_ADDR_FIFO_2_DEBUG_ADDR_SHIFT    0

/***************************************************************************
 *SW_VIDEO_RAW_PIC_DESC%i - Reserved Register 0..31
 ***************************************************************************/
#define BCHP_RAAGA_DSP_FW_CFG_SW_VIDEO_RAW_PIC_DESCi_ARRAY_BASE    0x01023860
#define BCHP_RAAGA_DSP_FW_CFG_SW_VIDEO_RAW_PIC_DESCi_ARRAY_START   0
#define BCHP_RAAGA_DSP_FW_CFG_SW_VIDEO_RAW_PIC_DESCi_ARRAY_END     31
#define BCHP_RAAGA_DSP_FW_CFG_SW_VIDEO_RAW_PIC_DESCi_ARRAY_ELEMENT_SIZE 64

/***************************************************************************
 *SW_VIDEO_RAW_PIC_DESC%i - Reserved Register 0..31
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: SW_VIDEO_RAW_PIC_DESCi :: SW_VIDEO_REGISTER [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_SW_VIDEO_RAW_PIC_DESCi_SW_VIDEO_REGISTER_MASK BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_SW_VIDEO_RAW_PIC_DESCi_SW_VIDEO_REGISTER_SHIFT 0


/***************************************************************************
 *SW_SDK_DEBUG_REGISTERS%i - Reserved Register 0..19
 ***************************************************************************/
#define BCHP_RAAGA_DSP_FW_CFG_SW_SDK_DEBUG_REGISTERSi_ARRAY_BASE   0x01023960
#define BCHP_RAAGA_DSP_FW_CFG_SW_SDK_DEBUG_REGISTERSi_ARRAY_START  0
#define BCHP_RAAGA_DSP_FW_CFG_SW_SDK_DEBUG_REGISTERSi_ARRAY_END    19
#define BCHP_RAAGA_DSP_FW_CFG_SW_SDK_DEBUG_REGISTERSi_ARRAY_ELEMENT_SIZE 64

/***************************************************************************
 *SW_SDK_DEBUG_REGISTERS%i - Reserved Register 0..19
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: SW_SDK_DEBUG_REGISTERSi :: SW_UNDEFINED [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_SW_SDK_DEBUG_REGISTERSi_SW_UNDEFINED_MASK BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_SW_SDK_DEBUG_REGISTERSi_SW_UNDEFINED_SHIFT 0


/***************************************************************************
 *SW_DEBUG_SPARE%i - Reserved Register 0..95
 ***************************************************************************/
#define BCHP_RAAGA_DSP_FW_CFG_SW_DEBUG_SPAREi_ARRAY_BASE           0x01023a00
#define BCHP_RAAGA_DSP_FW_CFG_SW_DEBUG_SPAREi_ARRAY_START          0
#define BCHP_RAAGA_DSP_FW_CFG_SW_DEBUG_SPAREi_ARRAY_END            95
#define BCHP_RAAGA_DSP_FW_CFG_SW_DEBUG_SPAREi_ARRAY_ELEMENT_SIZE   64

/***************************************************************************
 *SW_DEBUG_SPARE%i - Reserved Register 0..95
 ***************************************************************************/
/* RAAGA_DSP_FW_CFG :: SW_DEBUG_SPAREi :: SW_UNDEFINED [63:00] */
#define BCHP_RAAGA_DSP_FW_CFG_SW_DEBUG_SPAREi_SW_UNDEFINED_MASK    BCHP_UINT64_C(0xffffffff, 0xffffffff)
#define BCHP_RAAGA_DSP_FW_CFG_SW_DEBUG_SPAREi_SW_UNDEFINED_SHIFT   0

#endif /* #ifndef BCHP_RAAGA_DSP_FW_CFG_HOST2DSPCMD_FIFO0_BASEADDR */

/* End of File */
