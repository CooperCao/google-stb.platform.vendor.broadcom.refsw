/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#ifndef BCHP_HDR_CMP_0_H__
#define BCHP_HDR_CMP_0_H__

/***************************************************************************
 *HDR_CMP_0 - High Dynamic Range Unit in Video Compositor 0/Video Intra Surface 0/1
 ***************************************************************************/
#define BCHP_HDR_CMP_0_REVISION                  0x20648400 /* [RO] HDR Revision ID */
#define BCHP_HDR_CMP_0_HW_CONFIGURATION          0x20648404 /* [RO] HDR HW Configuration */
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN 0x20648408 /* [CFG] Canvas control */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C00     0x20648410 /* [CFG] Blender Out Color Matrix coefficient c00 */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C01     0x20648414 /* [CFG] Blender Out Color Matrix coefficient c01 */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C02     0x20648418 /* [CFG] Blender Out Color Matrix coefficient c02 */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C03     0x2064841c /* [CFG] Blender Out Color Matrix coefficient c03 */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C10     0x20648420 /* [CFG] Blender Out Color Matrix coefficient c10 */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C11     0x20648424 /* [CFG] Blender Out Color Matrix coefficient c11 */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C12     0x20648428 /* [CFG] Blender Out Color Matrix coefficient c12 */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C13     0x2064842c /* [CFG] Blender Out Color Matrix coefficient c13 */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C20     0x20648430 /* [CFG] Blender Out Color Matrix coefficient c20 */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C21     0x20648434 /* [CFG] Blender Out Color Matrix coefficient c21 */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C22     0x20648438 /* [CFG] Blender Out Color Matrix coefficient c22 */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C23     0x2064843c /* [CFG] Blender Out Color Matrix coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C00        0x20648500 /* [CFG] Video Surface 0 Color Matrix A R0 coefficient c00 */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C01        0x20648504 /* [CFG] Video Surface 0 Color Matrix A R0 coefficient c01 */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C02        0x20648508 /* [CFG] Video Surface 0 Color Matrix A R0 coefficient c02 */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C03        0x2064850c /* [CFG] Video Surface 0 Color Matrix A R0 coefficient c03 */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C10        0x20648510 /* [CFG] Video Surface 0 Color Matrix A R0 coefficient c10 */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C11        0x20648514 /* [CFG] Video Surface 0 Color Matrix A R0 coefficient c11 */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C12        0x20648518 /* [CFG] Video Surface 0 Color Matrix A R0 coefficient c12 */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C13        0x2064851c /* [CFG] Video Surface 0 Color Matrix A R0 coefficient c13 */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C20        0x20648520 /* [CFG] Video Surface 0 Color Matrix A R0 coefficient c20 */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C21        0x20648524 /* [CFG] Video Surface 0 Color Matrix A R0 coefficient c21 */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C22        0x20648528 /* [CFG] Video Surface 0 Color Matrix A R0 coefficient c22 */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C23        0x2064852c /* [CFG] Video Surface 0 Color Matrix A R0 coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C00        0x20648530 /* [CFG] Video Surface 0 Color Matrix A R1 coefficient c00 */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C01        0x20648534 /* [CFG] Video Surface 0 Color Matrix A R1 coefficient c01 */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C02        0x20648538 /* [CFG] Video Surface 0 Color Matrix A R1 coefficient c02 */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C03        0x2064853c /* [CFG] Video Surface 0 Color Matrix A R1 coefficient c03 */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C10        0x20648540 /* [CFG] Video Surface 0 Color Matrix A R1 coefficient c10 */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C11        0x20648544 /* [CFG] Video Surface 0 Color Matrix A R1 coefficient c11 */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C12        0x20648548 /* [CFG] Video Surface 0 Color Matrix A R1 coefficient c12 */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C13        0x2064854c /* [CFG] Video Surface 0 Color Matrix A R1 coefficient c13 */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C20        0x20648550 /* [CFG] Video Surface 0 Color Matrix A R1 coefficient c20 */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C21        0x20648554 /* [CFG] Video Surface 0 Color Matrix A R1 coefficient c21 */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C22        0x20648558 /* [CFG] Video Surface 0 Color Matrix A R1 coefficient c22 */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C23        0x2064855c /* [CFG] Video Surface 0 Color Matrix A R1 coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C00        0x20648560 /* [CFG] Video Surface 0 Color Matrix A R2 coefficient c00 */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C01        0x20648564 /* [CFG] Video Surface 0 Color Matrix A R2 coefficient c01 */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C02        0x20648568 /* [CFG] Video Surface 0 Color Matrix A R2 coefficient c02 */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C03        0x2064856c /* [CFG] Video Surface 0 Color Matrix A R2 coefficient c03 */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C10        0x20648570 /* [CFG] Video Surface 0 Color Matrix A R2 coefficient c10 */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C11        0x20648574 /* [CFG] Video Surface 0 Color Matrix A R2 coefficient c11 */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C12        0x20648578 /* [CFG] Video Surface 0 Color Matrix A R2 coefficient c12 */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C13        0x2064857c /* [CFG] Video Surface 0 Color Matrix A R2 coefficient c13 */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C20        0x20648580 /* [CFG] Video Surface 0 Color Matrix A R2 coefficient c20 */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C21        0x20648584 /* [CFG] Video Surface 0 Color Matrix A R2 coefficient c21 */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C22        0x20648588 /* [CFG] Video Surface 0 Color Matrix A R2 coefficient c22 */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C23        0x2064858c /* [CFG] Video Surface 0 Color Matrix A R2 coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C00        0x20648590 /* [CFG] Video Surface 0 Color Matrix A R3 coefficient c00 */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C01        0x20648594 /* [CFG] Video Surface 0 Color Matrix A R3 coefficient c01 */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C02        0x20648598 /* [CFG] Video Surface 0 Color Matrix A R3 coefficient c02 */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C03        0x2064859c /* [CFG] Video Surface 0 Color Matrix A R3 coefficient c03 */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C10        0x206485a0 /* [CFG] Video Surface 0 Color Matrix A R3 coefficient c10 */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C11        0x206485a4 /* [CFG] Video Surface 0 Color Matrix A R3 coefficient c11 */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C12        0x206485a8 /* [CFG] Video Surface 0 Color Matrix A R3 coefficient c12 */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C13        0x206485ac /* [CFG] Video Surface 0 Color Matrix A R3 coefficient c13 */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C20        0x206485b0 /* [CFG] Video Surface 0 Color Matrix A R3 coefficient c20 */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C21        0x206485b4 /* [CFG] Video Surface 0 Color Matrix A R3 coefficient c21 */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C22        0x206485b8 /* [CFG] Video Surface 0 Color Matrix A R3 coefficient c22 */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C23        0x206485bc /* [CFG] Video Surface 0 Color Matrix A R3 coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C00        0x206485c0 /* [CFG] Video Surface 0 Color Matrix A R4 coefficient c00 */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C01        0x206485c4 /* [CFG] Video Surface 0 Color Matrix A R4 coefficient c01 */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C02        0x206485c8 /* [CFG] Video Surface 0 Color Matrix A R4 coefficient c02 */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C03        0x206485cc /* [CFG] Video Surface 0 Color Matrix A R4 coefficient c03 */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C10        0x206485d0 /* [CFG] Video Surface 0 Color Matrix A R4 coefficient c10 */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C11        0x206485d4 /* [CFG] Video Surface 0 Color Matrix A R4 coefficient c11 */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C12        0x206485d8 /* [CFG] Video Surface 0 Color Matrix A R4 coefficient c12 */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C13        0x206485dc /* [CFG] Video Surface 0 Color Matrix A R4 coefficient c13 */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C20        0x206485e0 /* [CFG] Video Surface 0 Color Matrix A R4 coefficient c20 */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C21        0x206485e4 /* [CFG] Video Surface 0 Color Matrix A R4 coefficient c21 */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C22        0x206485e8 /* [CFG] Video Surface 0 Color Matrix A R4 coefficient c22 */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C23        0x206485ec /* [CFG] Video Surface 0 Color Matrix A R4 coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C00        0x206485f0 /* [CFG] Video Surface 0 Color Matrix A R5 coefficient c00 */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C01        0x206485f4 /* [CFG] Video Surface 0 Color Matrix A R5 coefficient c01 */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C02        0x206485f8 /* [CFG] Video Surface 0 Color Matrix A R5 coefficient c02 */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C03        0x206485fc /* [CFG] Video Surface 0 Color Matrix A R5 coefficient c03 */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C10        0x20648600 /* [CFG] Video Surface 0 Color Matrix A R5 coefficient c10 */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C11        0x20648604 /* [CFG] Video Surface 0 Color Matrix A R5 coefficient c11 */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C12        0x20648608 /* [CFG] Video Surface 0 Color Matrix A R5 coefficient c12 */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C13        0x2064860c /* [CFG] Video Surface 0 Color Matrix A R5 coefficient c13 */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C20        0x20648610 /* [CFG] Video Surface 0 Color Matrix A R5 coefficient c20 */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C21        0x20648614 /* [CFG] Video Surface 0 Color Matrix A R5 coefficient c21 */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C22        0x20648618 /* [CFG] Video Surface 0 Color Matrix A R5 coefficient c22 */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C23        0x2064861c /* [CFG] Video Surface 0 Color Matrix A R5 coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C00        0x20648700 /* [CFG] Video Surface 0 Color Matrix B R0 coefficient c00 */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C01        0x20648704 /* [CFG] Video Surface 0 Color Matrix B R0 coefficient c01 */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C02        0x20648708 /* [CFG] Video Surface 0 Color Matrix B R0 coefficient c02 */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C03        0x2064870c /* [CFG] Video Surface 0 Color Matrix B R0 coefficient c03 */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C10        0x20648710 /* [CFG] Video Surface 0 Color Matrix B R0 coefficient c10 */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C11        0x20648714 /* [CFG] Video Surface 0 Color Matrix B R0 coefficient c11 */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C12        0x20648718 /* [CFG] Video Surface 0 Color Matrix B R0 coefficient c12 */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C13        0x2064871c /* [CFG] Video Surface 0 Color Matrix B R0 coefficient c13 */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C20        0x20648720 /* [CFG] Video Surface 0 Color Matrix B R0 coefficient c20 */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C21        0x20648724 /* [CFG] Video Surface 0 Color Matrix B R0 coefficient c21 */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C22        0x20648728 /* [CFG] Video Surface 0 Color Matrix B R0 coefficient c22 */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C23        0x2064872c /* [CFG] Video Surface 0 Color Matrix B R0 coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C00        0x20648730 /* [CFG] Video Surface 0 Color Matrix B R1 coefficient c00 */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C01        0x20648734 /* [CFG] Video Surface 0 Color Matrix B R1 coefficient c01 */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C02        0x20648738 /* [CFG] Video Surface 0 Color Matrix B R1 coefficient c02 */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C03        0x2064873c /* [CFG] Video Surface 0 Color Matrix B R1 coefficient c03 */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C10        0x20648740 /* [CFG] Video Surface 0 Color Matrix B R1 coefficient c10 */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C11        0x20648744 /* [CFG] Video Surface 0 Color Matrix B R1 coefficient c11 */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C12        0x20648748 /* [CFG] Video Surface 0 Color Matrix B R1 coefficient c12 */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C13        0x2064874c /* [CFG] Video Surface 0 Color Matrix B R1 coefficient c13 */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C20        0x20648750 /* [CFG] Video Surface 0 Color Matrix B R1 coefficient c20 */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C21        0x20648754 /* [CFG] Video Surface 0 Color Matrix B R1 coefficient c21 */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C22        0x20648758 /* [CFG] Video Surface 0 Color Matrix B R1 coefficient c22 */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C23        0x2064875c /* [CFG] Video Surface 0 Color Matrix B R1 coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C00        0x20648760 /* [CFG] Video Surface 0 Color Matrix B R2 coefficient c00 */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C01        0x20648764 /* [CFG] Video Surface 0 Color Matrix B R2 coefficient c01 */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C02        0x20648768 /* [CFG] Video Surface 0 Color Matrix B R2 coefficient c02 */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C03        0x2064876c /* [CFG] Video Surface 0 Color Matrix B R2 coefficient c03 */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C10        0x20648770 /* [CFG] Video Surface 0 Color Matrix B R2 coefficient c10 */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C11        0x20648774 /* [CFG] Video Surface 0 Color Matrix B R2 coefficient c11 */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C12        0x20648778 /* [CFG] Video Surface 0 Color Matrix B R2 coefficient c12 */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C13        0x2064877c /* [CFG] Video Surface 0 Color Matrix B R2 coefficient c13 */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C20        0x20648780 /* [CFG] Video Surface 0 Color Matrix B R2 coefficient c20 */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C21        0x20648784 /* [CFG] Video Surface 0 Color Matrix B R2 coefficient c21 */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C22        0x20648788 /* [CFG] Video Surface 0 Color Matrix B R2 coefficient c22 */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C23        0x2064878c /* [CFG] Video Surface 0 Color Matrix B R2 coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C00        0x20648790 /* [CFG] Video Surface 0 Color Matrix B R3 coefficient c00 */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C01        0x20648794 /* [CFG] Video Surface 0 Color Matrix B R3 coefficient c01 */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C02        0x20648798 /* [CFG] Video Surface 0 Color Matrix B R3 coefficient c02 */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C03        0x2064879c /* [CFG] Video Surface 0 Color Matrix B R3 coefficient c03 */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C10        0x206487a0 /* [CFG] Video Surface 0 Color Matrix B R3 coefficient c10 */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C11        0x206487a4 /* [CFG] Video Surface 0 Color Matrix B R3 coefficient c11 */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C12        0x206487a8 /* [CFG] Video Surface 0 Color Matrix B R3 coefficient c12 */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C13        0x206487ac /* [CFG] Video Surface 0 Color Matrix B R3 coefficient c13 */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C20        0x206487b0 /* [CFG] Video Surface 0 Color Matrix B R3 coefficient c20 */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C21        0x206487b4 /* [CFG] Video Surface 0 Color Matrix B R3 coefficient c21 */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C22        0x206487b8 /* [CFG] Video Surface 0 Color Matrix B R3 coefficient c22 */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C23        0x206487bc /* [CFG] Video Surface 0 Color Matrix B R3 coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C00        0x206487c0 /* [CFG] Video Surface 0 Color Matrix B R4 coefficient c00 */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C01        0x206487c4 /* [CFG] Video Surface 0 Color Matrix B R4 coefficient c01 */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C02        0x206487c8 /* [CFG] Video Surface 0 Color Matrix B R4 coefficient c02 */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C03        0x206487cc /* [CFG] Video Surface 0 Color Matrix B R4 coefficient c03 */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C10        0x206487d0 /* [CFG] Video Surface 0 Color Matrix B R4 coefficient c10 */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C11        0x206487d4 /* [CFG] Video Surface 0 Color Matrix B R4 coefficient c11 */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C12        0x206487d8 /* [CFG] Video Surface 0 Color Matrix B R4 coefficient c12 */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C13        0x206487dc /* [CFG] Video Surface 0 Color Matrix B R4 coefficient c13 */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C20        0x206487e0 /* [CFG] Video Surface 0 Color Matrix B R4 coefficient c20 */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C21        0x206487e4 /* [CFG] Video Surface 0 Color Matrix B R4 coefficient c21 */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C22        0x206487e8 /* [CFG] Video Surface 0 Color Matrix B R4 coefficient c22 */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C23        0x206487ec /* [CFG] Video Surface 0 Color Matrix B R4 coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C00        0x206487f0 /* [CFG] Video Surface 0 Color Matrix B R5 coefficient c00 */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C01        0x206487f4 /* [CFG] Video Surface 0 Color Matrix B R5 coefficient c01 */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C02        0x206487f8 /* [CFG] Video Surface 0 Color Matrix B R5 coefficient c02 */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C03        0x206487fc /* [CFG] Video Surface 0 Color Matrix B R5 coefficient c03 */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C10        0x20648800 /* [CFG] Video Surface 0 Color Matrix B R5 coefficient c10 */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C11        0x20648804 /* [CFG] Video Surface 0 Color Matrix B R5 coefficient c11 */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C12        0x20648808 /* [CFG] Video Surface 0 Color Matrix B R5 coefficient c12 */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C13        0x2064880c /* [CFG] Video Surface 0 Color Matrix B R5 coefficient c13 */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C20        0x20648810 /* [CFG] Video Surface 0 Color Matrix B R5 coefficient c20 */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C21        0x20648814 /* [CFG] Video Surface 0 Color Matrix B R5 coefficient c21 */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C22        0x20648818 /* [CFG] Video Surface 0 Color Matrix B R5 coefficient c22 */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C23        0x2064881c /* [CFG] Video Surface 0 Color Matrix B R5 coefficient c23 */
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL         0x206488c0 /* [CFG] Video Surface 0 Nonconstant Luminance CSC Control */
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL         0x20648910 /* [CFG] Video Surface 0 Nonconstant Luminance CSC Control */
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL         0x20648960 /* [CFG] Video Surface 0 Nonconstant Luminance CSC Control */
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL         0x206489b0 /* [CFG] Video Surface 0 Nonconstant Luminance CSC Control */
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL         0x20648a10 /* [CFG] Video Surface 0 Nonconstant Luminance CSC Control */
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL         0x20648a60 /* [CFG] Video Surface 0 Nonconstant Luminance CSC Control */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_CSC_EN      0x20648a6c /* [CFG] Video Surface 0 Blender In Color Matrix Enable */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C00   0x20648a70 /* [CFG] Video Surface 0 Blender In Color Matrix coefficient c00 */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C01   0x20648a74 /* [CFG] Video Surface 0 Blender In Color Matrix coefficient c01 */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C02   0x20648a78 /* [CFG] Video Surface 0 Blender In Color Matrix coefficient c02 */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C03   0x20648a7c /* [CFG] Video Surface 0 Blender In Color Matrix coefficient c03 */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C10   0x20648a80 /* [CFG] Video Surface 0 Blender In Color Matrix coefficient c10 */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C11   0x20648a84 /* [CFG] Video Surface 0 Blender In Color Matrix coefficient c11 */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C12   0x20648a88 /* [CFG] Video Surface 0 Blender In Color Matrix coefficient c12 */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C13   0x20648a8c /* [CFG] Video Surface 0 Blender In Color Matrix coefficient c13 */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C20   0x20648a90 /* [CFG] Video Surface 0 Blender In Color Matrix coefficient c20 */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C21   0x20648a94 /* [CFG] Video Surface 0 Blender In Color Matrix coefficient c21 */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C22   0x20648a98 /* [CFG] Video Surface 0 Blender In Color Matrix coefficient c22 */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C23   0x20648a9c /* [CFG] Video Surface 0 Blender In Color Matrix coefficient c23 */
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL            0x20648b00 /* [CFG] Video Surface 0 NL2N LUT Control (Control for TEST_RAM) */
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL            0x20649f00 /* [CFG] Video Surface 0 L2NL LUT Control (Control for TEST_RAM) */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C00        0x2064a500 /* [CFG] Video Surface 1 Color Matrix A R0 coefficient c00 */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C01        0x2064a504 /* [CFG] Video Surface 1 Color Matrix A R0 coefficient c01 */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C02        0x2064a508 /* [CFG] Video Surface 1 Color Matrix A R0 coefficient c02 */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C03        0x2064a50c /* [CFG] Video Surface 1 Color Matrix A R0 coefficient c03 */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C10        0x2064a510 /* [CFG] Video Surface 1 Color Matrix A R0 coefficient c10 */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C11        0x2064a514 /* [CFG] Video Surface 1 Color Matrix A R0 coefficient c11 */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C12        0x2064a518 /* [CFG] Video Surface 1 Color Matrix A R0 coefficient c12 */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C13        0x2064a51c /* [CFG] Video Surface 1 Color Matrix A R0 coefficient c13 */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C20        0x2064a520 /* [CFG] Video Surface 1 Color Matrix A R0 coefficient c20 */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C21        0x2064a524 /* [CFG] Video Surface 1 Color Matrix A R0 coefficient c21 */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C22        0x2064a528 /* [CFG] Video Surface 1 Color Matrix A R0 coefficient c22 */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C23        0x2064a52c /* [CFG] Video Surface 1 Color Matrix A R0 coefficient c23 */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C00        0x2064a530 /* [CFG] Video Surface 1 Color Matrix A R1 coefficient c00 */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C01        0x2064a534 /* [CFG] Video Surface 1 Color Matrix A R1 coefficient c01 */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C02        0x2064a538 /* [CFG] Video Surface 1 Color Matrix A R1 coefficient c02 */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C03        0x2064a53c /* [CFG] Video Surface 1 Color Matrix A R1 coefficient c03 */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C10        0x2064a540 /* [CFG] Video Surface 1 Color Matrix A R1 coefficient c10 */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C11        0x2064a544 /* [CFG] Video Surface 1 Color Matrix A R1 coefficient c11 */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C12        0x2064a548 /* [CFG] Video Surface 1 Color Matrix A R1 coefficient c12 */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C13        0x2064a54c /* [CFG] Video Surface 1 Color Matrix A R1 coefficient c13 */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C20        0x2064a550 /* [CFG] Video Surface 1 Color Matrix A R1 coefficient c20 */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C21        0x2064a554 /* [CFG] Video Surface 1 Color Matrix A R1 coefficient c21 */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C22        0x2064a558 /* [CFG] Video Surface 1 Color Matrix A R1 coefficient c22 */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C23        0x2064a55c /* [CFG] Video Surface 1 Color Matrix A R1 coefficient c23 */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C00        0x2064a560 /* [CFG] Video Surface 1 Color Matrix A R2 coefficient c00 */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C01        0x2064a564 /* [CFG] Video Surface 1 Color Matrix A R2 coefficient c01 */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C02        0x2064a568 /* [CFG] Video Surface 1 Color Matrix A R2 coefficient c02 */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C03        0x2064a56c /* [CFG] Video Surface 1 Color Matrix A R2 coefficient c03 */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C10        0x2064a570 /* [CFG] Video Surface 1 Color Matrix A R2 coefficient c10 */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C11        0x2064a574 /* [CFG] Video Surface 1 Color Matrix A R2 coefficient c11 */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C12        0x2064a578 /* [CFG] Video Surface 1 Color Matrix A R2 coefficient c12 */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C13        0x2064a57c /* [CFG] Video Surface 1 Color Matrix A R2 coefficient c13 */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C20        0x2064a580 /* [CFG] Video Surface 1 Color Matrix A R2 coefficient c20 */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C21        0x2064a584 /* [CFG] Video Surface 1 Color Matrix A R2 coefficient c21 */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C22        0x2064a588 /* [CFG] Video Surface 1 Color Matrix A R2 coefficient c22 */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C23        0x2064a58c /* [CFG] Video Surface 1 Color Matrix A R2 coefficient c23 */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C00        0x2064a590 /* [CFG] Video Surface 1 Color Matrix A R3 coefficient c00 */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C01        0x2064a594 /* [CFG] Video Surface 1 Color Matrix A R3 coefficient c01 */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C02        0x2064a598 /* [CFG] Video Surface 1 Color Matrix A R3 coefficient c02 */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C03        0x2064a59c /* [CFG] Video Surface 1 Color Matrix A R3 coefficient c03 */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C10        0x2064a5a0 /* [CFG] Video Surface 1 Color Matrix A R3 coefficient c10 */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C11        0x2064a5a4 /* [CFG] Video Surface 1 Color Matrix A R3 coefficient c11 */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C12        0x2064a5a8 /* [CFG] Video Surface 1 Color Matrix A R3 coefficient c12 */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C13        0x2064a5ac /* [CFG] Video Surface 1 Color Matrix A R3 coefficient c13 */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C20        0x2064a5b0 /* [CFG] Video Surface 1 Color Matrix A R3 coefficient c20 */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C21        0x2064a5b4 /* [CFG] Video Surface 1 Color Matrix A R3 coefficient c21 */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C22        0x2064a5b8 /* [CFG] Video Surface 1 Color Matrix A R3 coefficient c22 */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C23        0x2064a5bc /* [CFG] Video Surface 1 Color Matrix A R3 coefficient c23 */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C00        0x2064a5c0 /* [CFG] Video Surface 1 Color Matrix A R4 coefficient c00 */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C01        0x2064a5c4 /* [CFG] Video Surface 1 Color Matrix A R4 coefficient c01 */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C02        0x2064a5c8 /* [CFG] Video Surface 1 Color Matrix A R4 coefficient c02 */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C03        0x2064a5cc /* [CFG] Video Surface 1 Color Matrix A R4 coefficient c03 */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C10        0x2064a5d0 /* [CFG] Video Surface 1 Color Matrix A R4 coefficient c10 */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C11        0x2064a5d4 /* [CFG] Video Surface 1 Color Matrix A R4 coefficient c11 */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C12        0x2064a5d8 /* [CFG] Video Surface 1 Color Matrix A R4 coefficient c12 */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C13        0x2064a5dc /* [CFG] Video Surface 1 Color Matrix A R4 coefficient c13 */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C20        0x2064a5e0 /* [CFG] Video Surface 1 Color Matrix A R4 coefficient c20 */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C21        0x2064a5e4 /* [CFG] Video Surface 1 Color Matrix A R4 coefficient c21 */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C22        0x2064a5e8 /* [CFG] Video Surface 1 Color Matrix A R4 coefficient c22 */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C23        0x2064a5ec /* [CFG] Video Surface 1 Color Matrix A R4 coefficient c23 */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C00        0x2064a5f0 /* [CFG] Video Surface 1 Color Matrix A R5 coefficient c00 */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C01        0x2064a5f4 /* [CFG] Video Surface 1 Color Matrix A R5 coefficient c01 */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C02        0x2064a5f8 /* [CFG] Video Surface 1 Color Matrix A R5 coefficient c02 */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C03        0x2064a5fc /* [CFG] Video Surface 1 Color Matrix A R5 coefficient c03 */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C10        0x2064a600 /* [CFG] Video Surface 1 Color Matrix A R5 coefficient c10 */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C11        0x2064a604 /* [CFG] Video Surface 1 Color Matrix A R5 coefficient c11 */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C12        0x2064a608 /* [CFG] Video Surface 1 Color Matrix A R5 coefficient c12 */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C13        0x2064a60c /* [CFG] Video Surface 1 Color Matrix A R5 coefficient c13 */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C20        0x2064a610 /* [CFG] Video Surface 1 Color Matrix A R5 coefficient c20 */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C21        0x2064a614 /* [CFG] Video Surface 1 Color Matrix A R5 coefficient c21 */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C22        0x2064a618 /* [CFG] Video Surface 1 Color Matrix A R5 coefficient c22 */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C23        0x2064a61c /* [CFG] Video Surface 1 Color Matrix A R5 coefficient c23 */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C00        0x2064a700 /* [CFG] Video Surface 1 Color Matrix B R0 coefficient c00 */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C01        0x2064a704 /* [CFG] Video Surface 1 Color Matrix B R0 coefficient c01 */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C02        0x2064a708 /* [CFG] Video Surface 1 Color Matrix B R0 coefficient c02 */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C03        0x2064a70c /* [CFG] Video Surface 1 Color Matrix B R0 coefficient c03 */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C10        0x2064a710 /* [CFG] Video Surface 1 Color Matrix B R0 coefficient c10 */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C11        0x2064a714 /* [CFG] Video Surface 1 Color Matrix B R0 coefficient c11 */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C12        0x2064a718 /* [CFG] Video Surface 1 Color Matrix B R0 coefficient c12 */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C13        0x2064a71c /* [CFG] Video Surface 1 Color Matrix B R0 coefficient c13 */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C20        0x2064a720 /* [CFG] Video Surface 1 Color Matrix B R0 coefficient c20 */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C21        0x2064a724 /* [CFG] Video Surface 1 Color Matrix B R0 coefficient c21 */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C22        0x2064a728 /* [CFG] Video Surface 1 Color Matrix B R0 coefficient c22 */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C23        0x2064a72c /* [CFG] Video Surface 1 Color Matrix B R0 coefficient c23 */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C00        0x2064a730 /* [CFG] Video Surface 1 Color Matrix B R1 coefficient c00 */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C01        0x2064a734 /* [CFG] Video Surface 1 Color Matrix B R1 coefficient c01 */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C02        0x2064a738 /* [CFG] Video Surface 1 Color Matrix B R1 coefficient c02 */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C03        0x2064a73c /* [CFG] Video Surface 1 Color Matrix B R1 coefficient c03 */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C10        0x2064a740 /* [CFG] Video Surface 1 Color Matrix B R1 coefficient c10 */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C11        0x2064a744 /* [CFG] Video Surface 1 Color Matrix B R1 coefficient c11 */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C12        0x2064a748 /* [CFG] Video Surface 1 Color Matrix B R1 coefficient c12 */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C13        0x2064a74c /* [CFG] Video Surface 1 Color Matrix B R1 coefficient c13 */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C20        0x2064a750 /* [CFG] Video Surface 1 Color Matrix B R1 coefficient c20 */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C21        0x2064a754 /* [CFG] Video Surface 1 Color Matrix B R1 coefficient c21 */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C22        0x2064a758 /* [CFG] Video Surface 1 Color Matrix B R1 coefficient c22 */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C23        0x2064a75c /* [CFG] Video Surface 1 Color Matrix B R1 coefficient c23 */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C00        0x2064a760 /* [CFG] Video Surface 1 Color Matrix B R2 coefficient c00 */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C01        0x2064a764 /* [CFG] Video Surface 1 Color Matrix B R2 coefficient c01 */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C02        0x2064a768 /* [CFG] Video Surface 1 Color Matrix B R2 coefficient c02 */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C03        0x2064a76c /* [CFG] Video Surface 1 Color Matrix B R2 coefficient c03 */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C10        0x2064a770 /* [CFG] Video Surface 1 Color Matrix B R2 coefficient c10 */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C11        0x2064a774 /* [CFG] Video Surface 1 Color Matrix B R2 coefficient c11 */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C12        0x2064a778 /* [CFG] Video Surface 1 Color Matrix B R2 coefficient c12 */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C13        0x2064a77c /* [CFG] Video Surface 1 Color Matrix B R2 coefficient c13 */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C20        0x2064a780 /* [CFG] Video Surface 1 Color Matrix B R2 coefficient c20 */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C21        0x2064a784 /* [CFG] Video Surface 1 Color Matrix B R2 coefficient c21 */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C22        0x2064a788 /* [CFG] Video Surface 1 Color Matrix B R2 coefficient c22 */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C23        0x2064a78c /* [CFG] Video Surface 1 Color Matrix B R2 coefficient c23 */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C00        0x2064a790 /* [CFG] Video Surface 1 Color Matrix B R3 coefficient c00 */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C01        0x2064a794 /* [CFG] Video Surface 1 Color Matrix B R3 coefficient c01 */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C02        0x2064a798 /* [CFG] Video Surface 1 Color Matrix B R3 coefficient c02 */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C03        0x2064a79c /* [CFG] Video Surface 1 Color Matrix B R3 coefficient c03 */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C10        0x2064a7a0 /* [CFG] Video Surface 1 Color Matrix B R3 coefficient c10 */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C11        0x2064a7a4 /* [CFG] Video Surface 1 Color Matrix B R3 coefficient c11 */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C12        0x2064a7a8 /* [CFG] Video Surface 1 Color Matrix B R3 coefficient c12 */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C13        0x2064a7ac /* [CFG] Video Surface 1 Color Matrix B R3 coefficient c13 */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C20        0x2064a7b0 /* [CFG] Video Surface 1 Color Matrix B R3 coefficient c20 */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C21        0x2064a7b4 /* [CFG] Video Surface 1 Color Matrix B R3 coefficient c21 */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C22        0x2064a7b8 /* [CFG] Video Surface 1 Color Matrix B R3 coefficient c22 */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C23        0x2064a7bc /* [CFG] Video Surface 1 Color Matrix B R3 coefficient c23 */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C00        0x2064a7c0 /* [CFG] Video Surface 1 Color Matrix B R4 coefficient c00 */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C01        0x2064a7c4 /* [CFG] Video Surface 1 Color Matrix B R4 coefficient c01 */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C02        0x2064a7c8 /* [CFG] Video Surface 1 Color Matrix B R4 coefficient c02 */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C03        0x2064a7cc /* [CFG] Video Surface 1 Color Matrix B R4 coefficient c03 */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C10        0x2064a7d0 /* [CFG] Video Surface 1 Color Matrix B R4 coefficient c10 */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C11        0x2064a7d4 /* [CFG] Video Surface 1 Color Matrix B R4 coefficient c11 */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C12        0x2064a7d8 /* [CFG] Video Surface 1 Color Matrix B R4 coefficient c12 */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C13        0x2064a7dc /* [CFG] Video Surface 1 Color Matrix B R4 coefficient c13 */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C20        0x2064a7e0 /* [CFG] Video Surface 1 Color Matrix B R4 coefficient c20 */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C21        0x2064a7e4 /* [CFG] Video Surface 1 Color Matrix B R4 coefficient c21 */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C22        0x2064a7e8 /* [CFG] Video Surface 1 Color Matrix B R4 coefficient c22 */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C23        0x2064a7ec /* [CFG] Video Surface 1 Color Matrix B R4 coefficient c23 */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C00        0x2064a7f0 /* [CFG] Video Surface 1 Color Matrix B R5 coefficient c00 */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C01        0x2064a7f4 /* [CFG] Video Surface 1 Color Matrix B R5 coefficient c01 */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C02        0x2064a7f8 /* [CFG] Video Surface 1 Color Matrix B R5 coefficient c02 */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C03        0x2064a7fc /* [CFG] Video Surface 1 Color Matrix B R5 coefficient c03 */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C10        0x2064a800 /* [CFG] Video Surface 1 Color Matrix B R5 coefficient c10 */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C11        0x2064a804 /* [CFG] Video Surface 1 Color Matrix B R5 coefficient c11 */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C12        0x2064a808 /* [CFG] Video Surface 1 Color Matrix B R5 coefficient c12 */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C13        0x2064a80c /* [CFG] Video Surface 1 Color Matrix B R5 coefficient c13 */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C20        0x2064a810 /* [CFG] Video Surface 1 Color Matrix B R5 coefficient c20 */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C21        0x2064a814 /* [CFG] Video Surface 1 Color Matrix B R5 coefficient c21 */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C22        0x2064a818 /* [CFG] Video Surface 1 Color Matrix B R5 coefficient c22 */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C23        0x2064a81c /* [CFG] Video Surface 1 Color Matrix B R5 coefficient c23 */
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL         0x2064a8c0 /* [CFG] Video Surface 1 Nonconstant Luminance CSC Control */
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL         0x2064a910 /* [CFG] Video Surface 1 Nonconstant Luminance CSC Control */
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL         0x2064a960 /* [CFG] Video Surface 1 Nonconstant Luminance CSC Control */
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL         0x2064a9b0 /* [CFG] Video Surface 1 Nonconstant Luminance CSC Control */
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL         0x2064aa10 /* [CFG] Video Surface 1 Nonconstant Luminance CSC Control */
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL         0x2064aa60 /* [CFG] Video Surface 1 Nonconstant Luminance CSC Control */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_CSC_EN      0x2064aa6c /* [CFG] Video Surface 1 Blender In Color Matrix Enable */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C00   0x2064aa70 /* [CFG] Video Surface 1 Blender In Color Matrix coefficient c00 */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C01   0x2064aa74 /* [CFG] Video Surface 1 Blender In Color Matrix coefficient c01 */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C02   0x2064aa78 /* [CFG] Video Surface 1 Blender In Color Matrix coefficient c02 */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C03   0x2064aa7c /* [CFG] Video Surface 1 Blender In Color Matrix coefficient c03 */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C10   0x2064aa80 /* [CFG] Video Surface 1 Blender In Color Matrix coefficient c10 */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C11   0x2064aa84 /* [CFG] Video Surface 1 Blender In Color Matrix coefficient c11 */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C12   0x2064aa88 /* [CFG] Video Surface 1 Blender In Color Matrix coefficient c12 */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C13   0x2064aa8c /* [CFG] Video Surface 1 Blender In Color Matrix coefficient c13 */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C20   0x2064aa90 /* [CFG] Video Surface 1 Blender In Color Matrix coefficient c20 */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C21   0x2064aa94 /* [CFG] Video Surface 1 Blender In Color Matrix coefficient c21 */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C22   0x2064aa98 /* [CFG] Video Surface 1 Blender In Color Matrix coefficient c22 */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C23   0x2064aa9c /* [CFG] Video Surface 1 Blender In Color Matrix coefficient c23 */

/***************************************************************************
 *REVISION - HDR Revision ID
 ***************************************************************************/
/* HDR_CMP_0 :: REVISION :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_REVISION_reserved0_MASK                     0xffff0000
#define BCHP_HDR_CMP_0_REVISION_reserved0_SHIFT                    16

/* HDR_CMP_0 :: REVISION :: MAJOR [15:08] */
#define BCHP_HDR_CMP_0_REVISION_MAJOR_MASK                         0x0000ff00
#define BCHP_HDR_CMP_0_REVISION_MAJOR_SHIFT                        8
#define BCHP_HDR_CMP_0_REVISION_MAJOR_DEFAULT                      0x00000000

/* HDR_CMP_0 :: REVISION :: MINOR [07:00] */
#define BCHP_HDR_CMP_0_REVISION_MINOR_MASK                         0x000000ff
#define BCHP_HDR_CMP_0_REVISION_MINOR_SHIFT                        0
#define BCHP_HDR_CMP_0_REVISION_MINOR_DEFAULT                      0x00000001

/***************************************************************************
 *HW_CONFIGURATION - HDR HW Configuration
 ***************************************************************************/
/* HDR_CMP_0 :: HW_CONFIGURATION :: reserved0 [31:21] */
#define BCHP_HDR_CMP_0_HW_CONFIGURATION_reserved0_MASK             0xffe00000
#define BCHP_HDR_CMP_0_HW_CONFIGURATION_reserved0_SHIFT            21

/* HDR_CMP_0 :: HW_CONFIGURATION :: reserved1 [20:03] */
#define BCHP_HDR_CMP_0_HW_CONFIGURATION_reserved1_MASK             0x001ffff8
#define BCHP_HDR_CMP_0_HW_CONFIGURATION_reserved1_SHIFT            3

/* HDR_CMP_0 :: HW_CONFIGURATION :: HDR_Version [02:00] */
#define BCHP_HDR_CMP_0_HW_CONFIGURATION_HDR_Version_MASK           0x00000007
#define BCHP_HDR_CMP_0_HW_CONFIGURATION_HDR_Version_SHIFT          0
#define BCHP_HDR_CMP_0_HW_CONFIGURATION_HDR_Version_DEFAULT        0x00000001
#define BCHP_HDR_CMP_0_HW_CONFIGURATION_HDR_Version_HDR_VERSION_1  1

/***************************************************************************
 *CMP_BLENDER_OUT_PQ_CSC_EN - Canvas control
 ***************************************************************************/
/* HDR_CMP_0 :: CMP_BLENDER_OUT_PQ_CSC_EN :: reserved0 [31:04] */
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_reserved0_MASK    0xfffffff0
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_reserved0_SHIFT   4

/* HDR_CMP_0 :: CMP_BLENDER_OUT_PQ_CSC_EN :: reserved1 [03:02] */
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_reserved1_MASK    0x0000000c
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_reserved1_SHIFT   2

/* HDR_CMP_0 :: CMP_BLENDER_OUT_PQ_CSC_EN :: BLENDER_PQ_ADJ_ENABLE [01:01] */
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_BLENDER_PQ_ADJ_ENABLE_MASK 0x00000002
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_BLENDER_PQ_ADJ_ENABLE_SHIFT 1
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_BLENDER_PQ_ADJ_ENABLE_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_BLENDER_PQ_ADJ_ENABLE_DISABLE 0
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_BLENDER_PQ_ADJ_ENABLE_ENABLE 1

/* HDR_CMP_0 :: CMP_BLENDER_OUT_PQ_CSC_EN :: BLENDER_OUT_CSC_ENABLE [00:00] */
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_BLENDER_OUT_CSC_ENABLE_MASK 0x00000001
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_BLENDER_OUT_CSC_ENABLE_SHIFT 0
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_BLENDER_OUT_CSC_ENABLE_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_BLENDER_OUT_CSC_ENABLE_DISABLE 0
#define BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN_BLENDER_OUT_CSC_ENABLE_ENABLE 1

/***************************************************************************
 *BLENDER_OUT_COEFF_C00 - Blender Out Color Matrix coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C00 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C00_reserved0_MASK        0xffff0000
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C00_reserved0_SHIFT       16

/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C00 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C00_COEFF_MUL_MASK        0x0000ffff
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C00_COEFF_MUL_SHIFT       0
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C00_COEFF_MUL_DEFAULT     0x00000000

/***************************************************************************
 *BLENDER_OUT_COEFF_C01 - Blender Out Color Matrix coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C01 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C01_reserved0_MASK        0xffff0000
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C01_reserved0_SHIFT       16

/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C01 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C01_COEFF_MUL_MASK        0x0000ffff
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C01_COEFF_MUL_SHIFT       0
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C01_COEFF_MUL_DEFAULT     0x00000000

/***************************************************************************
 *BLENDER_OUT_COEFF_C02 - Blender Out Color Matrix coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C02 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C02_reserved0_MASK        0xffff0000
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C02_reserved0_SHIFT       16

/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C02 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C02_COEFF_MUL_MASK        0x0000ffff
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C02_COEFF_MUL_SHIFT       0
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C02_COEFF_MUL_DEFAULT     0x00000000

/***************************************************************************
 *BLENDER_OUT_COEFF_C03 - Blender Out Color Matrix coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C03 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C03_reserved0_MASK        0xffff0000
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C03_reserved0_SHIFT       16

/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C03 :: COEFF_ADD [15:00] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C03_COEFF_ADD_MASK        0x0000ffff
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C03_COEFF_ADD_SHIFT       0
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C03_COEFF_ADD_DEFAULT     0x00000000

/***************************************************************************
 *BLENDER_OUT_COEFF_C10 - Blender Out Color Matrix coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C10 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C10_reserved0_MASK        0xffff0000
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C10_reserved0_SHIFT       16

/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C10 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C10_COEFF_MUL_MASK        0x0000ffff
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C10_COEFF_MUL_SHIFT       0
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C10_COEFF_MUL_DEFAULT     0x00000000

/***************************************************************************
 *BLENDER_OUT_COEFF_C11 - Blender Out Color Matrix coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C11 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C11_reserved0_MASK        0xffff0000
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C11_reserved0_SHIFT       16

/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C11 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C11_COEFF_MUL_MASK        0x0000ffff
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C11_COEFF_MUL_SHIFT       0
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C11_COEFF_MUL_DEFAULT     0x00000000

/***************************************************************************
 *BLENDER_OUT_COEFF_C12 - Blender Out Color Matrix coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C12 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C12_reserved0_MASK        0xffff0000
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C12_reserved0_SHIFT       16

/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C12 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C12_COEFF_MUL_MASK        0x0000ffff
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C12_COEFF_MUL_SHIFT       0
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C12_COEFF_MUL_DEFAULT     0x00000000

/***************************************************************************
 *BLENDER_OUT_COEFF_C13 - Blender Out Color Matrix coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C13 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C13_reserved0_MASK        0xffff0000
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C13_reserved0_SHIFT       16

/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C13 :: COEFF_ADD [15:00] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C13_COEFF_ADD_MASK        0x0000ffff
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C13_COEFF_ADD_SHIFT       0
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C13_COEFF_ADD_DEFAULT     0x00000000

/***************************************************************************
 *BLENDER_OUT_COEFF_C20 - Blender Out Color Matrix coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C20 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C20_reserved0_MASK        0xffff0000
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C20_reserved0_SHIFT       16

/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C20 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C20_COEFF_MUL_MASK        0x0000ffff
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C20_COEFF_MUL_SHIFT       0
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C20_COEFF_MUL_DEFAULT     0x00000000

/***************************************************************************
 *BLENDER_OUT_COEFF_C21 - Blender Out Color Matrix coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C21 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C21_reserved0_MASK        0xffff0000
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C21_reserved0_SHIFT       16

/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C21 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C21_COEFF_MUL_MASK        0x0000ffff
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C21_COEFF_MUL_SHIFT       0
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C21_COEFF_MUL_DEFAULT     0x00000000

/***************************************************************************
 *BLENDER_OUT_COEFF_C22 - Blender Out Color Matrix coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C22 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C22_reserved0_MASK        0xffff0000
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C22_reserved0_SHIFT       16

/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C22 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C22_COEFF_MUL_MASK        0x0000ffff
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C22_COEFF_MUL_SHIFT       0
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C22_COEFF_MUL_DEFAULT     0x00000000

/***************************************************************************
 *BLENDER_OUT_COEFF_C23 - Blender Out Color Matrix coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C23 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C23_reserved0_MASK        0xffff0000
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C23_reserved0_SHIFT       16

/* HDR_CMP_0 :: BLENDER_OUT_COEFF_C23 :: COEFF_ADD [15:00] */
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C23_COEFF_ADD_MASK        0x0000ffff
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C23_COEFF_ADD_SHIFT       0
#define BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C23_COEFF_ADD_DEFAULT     0x00000000

/***************************************************************************
 *V0_R00_TO_R15_NL_CONFIG%i - Video Surface 0 RECT0 to RECT15 NL_CSC config[0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE         0x206484e0
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_START        0
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_END          7
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *V0_R00_TO_R15_NL_CONFIG%i - Video Surface 0 RECT0 to RECT15 NL_CSC config[0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R00_TO_R15_NL_CONFIGi :: reserved0 [31:29] */
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_reserved0_MASK     0xe0000000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_reserved0_SHIFT    29

/* HDR_CMP_0 :: V0_R00_TO_R15_NL_CONFIGi :: RECT1_SEL_L2NL [28:26] */
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_MASK 0x1c000000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_SHIFT 26
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_709 0
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_1886 1
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_PQ  2
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_BBC 3
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_RAM 4
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_BYPASS 5

/* HDR_CMP_0 :: V0_R00_TO_R15_NL_CONFIGi :: RECT1_SEL_LRANGE_ADJ [25:23] */
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_MASK 0x03800000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_SHIFT 23
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_DEFAULT 0x00000007
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_DISABLE 7
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_RESRV0 6
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_USE_R5 5
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_USE_R4 4
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_USE_R3 3
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_USE_R2 2
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_USE_R1 1
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_USE_R0 0

/* HDR_CMP_0 :: V0_R00_TO_R15_NL_CONFIGi :: RECT1_SEL_MB_COEF [22:20] */
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_MASK 0x00700000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_SHIFT 20
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_DEFAULT 0x00000007
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_DISABLE 7
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_RESRV0 6
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_USE_R5 5
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_USE_R4 4
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_USE_R3 3
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_USE_R2 2
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_USE_R1 1
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_USE_R0 0

/* HDR_CMP_0 :: V0_R00_TO_R15_NL_CONFIGi :: RECT1_SEL_NL2L [19:17] */
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_MASK 0x000e0000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_SHIFT 17
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_709 0
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_1886 1
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_PQ  2
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_BBC 3
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_RAM 4
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_BYPASS 5

/* HDR_CMP_0 :: V0_R00_TO_R15_NL_CONFIGi :: RECT1_NL_CSC_EN [16:16] */
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_NL_CSC_EN_MASK 0x00010000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_NL_CSC_EN_SHIFT 16
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_NL_CSC_EN_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_NL_CSC_EN_BYPASS 0
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_NL_CSC_EN_ENABLE 1

/* HDR_CMP_0 :: V0_R00_TO_R15_NL_CONFIGi :: reserved1 [15:13] */
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_reserved1_MASK     0x0000e000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_reserved1_SHIFT    13

/* HDR_CMP_0 :: V0_R00_TO_R15_NL_CONFIGi :: RECT0_SEL_L2NL [12:10] */
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_MASK 0x00001c00
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_SHIFT 10
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_709 0
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_1886 1
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_PQ  2
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_BBC 3
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_RAM 4
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_BYPASS 5

/* HDR_CMP_0 :: V0_R00_TO_R15_NL_CONFIGi :: RECT0_SEL_LRANGE_ADJ [09:07] */
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_MASK 0x00000380
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_SHIFT 7
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_DEFAULT 0x00000007
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_DISABLE 7
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_RESRV0 6
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_USE_R5 5
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_USE_R4 4
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_USE_R3 3
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_USE_R2 2
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_USE_R1 1
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_USE_R0 0

/* HDR_CMP_0 :: V0_R00_TO_R15_NL_CONFIGi :: RECT0_SEL_MB_COEF [06:04] */
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_MASK 0x00000070
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_SHIFT 4
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_DEFAULT 0x00000007
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_DISABLE 7
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_RESRV0 6
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_USE_R5 5
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_USE_R4 4
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_USE_R3 3
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_USE_R2 2
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_USE_R1 1
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_USE_R0 0

/* HDR_CMP_0 :: V0_R00_TO_R15_NL_CONFIGi :: RECT0_SEL_NL2L [03:01] */
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_MASK 0x0000000e
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_SHIFT 1
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_709 0
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_1886 1
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_PQ  2
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_BBC 3
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_RAM 4
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_BYPASS 5

/* HDR_CMP_0 :: V0_R00_TO_R15_NL_CONFIGi :: RECT0_NL_CSC_EN [00:00] */
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_NL_CSC_EN_MASK 0x00000001
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_NL_CSC_EN_SHIFT 0
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_NL_CSC_EN_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_NL_CSC_EN_BYPASS 0
#define BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT0_NL_CSC_EN_ENABLE 1


/***************************************************************************
 *V0_R0_MA_COEFF_C00 - Video Surface 0 Color Matrix A R0 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MA_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MA_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R0_MA_COEFF_C01 - Video Surface 0 Color Matrix A R0 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MA_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MA_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MA_COEFF_C02 - Video Surface 0 Color Matrix A R0 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MA_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MA_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MA_COEFF_C03 - Video Surface 0 Color Matrix A R0 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MA_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MA_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MA_COEFF_C10 - Video Surface 0 Color Matrix A R0 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MA_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MA_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MA_COEFF_C11 - Video Surface 0 Color Matrix A R0 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MA_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MA_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R0_MA_COEFF_C12 - Video Surface 0 Color Matrix A R0 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MA_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MA_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MA_COEFF_C13 - Video Surface 0 Color Matrix A R0 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MA_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MA_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MA_COEFF_C20 - Video Surface 0 Color Matrix A R0 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MA_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MA_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MA_COEFF_C21 - Video Surface 0 Color Matrix A R0 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MA_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MA_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MA_COEFF_C22 - Video Surface 0 Color Matrix A R0 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MA_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MA_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R0_MA_COEFF_C23 - Video Surface 0 Color Matrix A R0 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MA_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MA_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MA_COEFF_C00 - Video Surface 0 Color Matrix A R1 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MA_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MA_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R1_MA_COEFF_C01 - Video Surface 0 Color Matrix A R1 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MA_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MA_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MA_COEFF_C02 - Video Surface 0 Color Matrix A R1 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MA_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MA_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MA_COEFF_C03 - Video Surface 0 Color Matrix A R1 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MA_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MA_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MA_COEFF_C10 - Video Surface 0 Color Matrix A R1 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MA_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MA_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MA_COEFF_C11 - Video Surface 0 Color Matrix A R1 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MA_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MA_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R1_MA_COEFF_C12 - Video Surface 0 Color Matrix A R1 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MA_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MA_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MA_COEFF_C13 - Video Surface 0 Color Matrix A R1 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MA_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MA_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MA_COEFF_C20 - Video Surface 0 Color Matrix A R1 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MA_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MA_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MA_COEFF_C21 - Video Surface 0 Color Matrix A R1 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MA_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MA_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MA_COEFF_C22 - Video Surface 0 Color Matrix A R1 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MA_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MA_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R1_MA_COEFF_C23 - Video Surface 0 Color Matrix A R1 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MA_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MA_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MA_COEFF_C00 - Video Surface 0 Color Matrix A R2 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MA_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MA_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R2_MA_COEFF_C01 - Video Surface 0 Color Matrix A R2 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MA_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MA_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MA_COEFF_C02 - Video Surface 0 Color Matrix A R2 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MA_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MA_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MA_COEFF_C03 - Video Surface 0 Color Matrix A R2 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MA_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MA_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MA_COEFF_C10 - Video Surface 0 Color Matrix A R2 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MA_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MA_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MA_COEFF_C11 - Video Surface 0 Color Matrix A R2 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MA_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MA_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R2_MA_COEFF_C12 - Video Surface 0 Color Matrix A R2 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MA_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MA_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MA_COEFF_C13 - Video Surface 0 Color Matrix A R2 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MA_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MA_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MA_COEFF_C20 - Video Surface 0 Color Matrix A R2 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MA_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MA_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MA_COEFF_C21 - Video Surface 0 Color Matrix A R2 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MA_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MA_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MA_COEFF_C22 - Video Surface 0 Color Matrix A R2 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MA_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MA_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R2_MA_COEFF_C23 - Video Surface 0 Color Matrix A R2 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MA_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MA_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MA_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MA_COEFF_C00 - Video Surface 0 Color Matrix A R3 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MA_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MA_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R3_MA_COEFF_C01 - Video Surface 0 Color Matrix A R3 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MA_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MA_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MA_COEFF_C02 - Video Surface 0 Color Matrix A R3 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MA_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MA_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MA_COEFF_C03 - Video Surface 0 Color Matrix A R3 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MA_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MA_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MA_COEFF_C10 - Video Surface 0 Color Matrix A R3 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MA_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MA_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MA_COEFF_C11 - Video Surface 0 Color Matrix A R3 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MA_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MA_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R3_MA_COEFF_C12 - Video Surface 0 Color Matrix A R3 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MA_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MA_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MA_COEFF_C13 - Video Surface 0 Color Matrix A R3 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MA_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MA_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MA_COEFF_C20 - Video Surface 0 Color Matrix A R3 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MA_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MA_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MA_COEFF_C21 - Video Surface 0 Color Matrix A R3 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MA_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MA_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MA_COEFF_C22 - Video Surface 0 Color Matrix A R3 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MA_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MA_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R3_MA_COEFF_C23 - Video Surface 0 Color Matrix A R3 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MA_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MA_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MA_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MA_COEFF_C00 - Video Surface 0 Color Matrix A R4 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MA_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MA_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R4_MA_COEFF_C01 - Video Surface 0 Color Matrix A R4 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MA_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MA_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MA_COEFF_C02 - Video Surface 0 Color Matrix A R4 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MA_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MA_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MA_COEFF_C03 - Video Surface 0 Color Matrix A R4 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MA_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MA_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MA_COEFF_C10 - Video Surface 0 Color Matrix A R4 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MA_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MA_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MA_COEFF_C11 - Video Surface 0 Color Matrix A R4 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MA_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MA_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R4_MA_COEFF_C12 - Video Surface 0 Color Matrix A R4 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MA_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MA_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MA_COEFF_C13 - Video Surface 0 Color Matrix A R4 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MA_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MA_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MA_COEFF_C20 - Video Surface 0 Color Matrix A R4 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MA_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MA_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MA_COEFF_C21 - Video Surface 0 Color Matrix A R4 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MA_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MA_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MA_COEFF_C22 - Video Surface 0 Color Matrix A R4 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MA_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MA_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R4_MA_COEFF_C23 - Video Surface 0 Color Matrix A R4 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MA_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MA_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MA_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MA_COEFF_C00 - Video Surface 0 Color Matrix A R5 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MA_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MA_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R5_MA_COEFF_C01 - Video Surface 0 Color Matrix A R5 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MA_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MA_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MA_COEFF_C02 - Video Surface 0 Color Matrix A R5 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MA_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MA_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MA_COEFF_C03 - Video Surface 0 Color Matrix A R5 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MA_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MA_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MA_COEFF_C10 - Video Surface 0 Color Matrix A R5 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MA_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MA_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MA_COEFF_C11 - Video Surface 0 Color Matrix A R5 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MA_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MA_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R5_MA_COEFF_C12 - Video Surface 0 Color Matrix A R5 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MA_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MA_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MA_COEFF_C13 - Video Surface 0 Color Matrix A R5 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MA_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MA_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MA_COEFF_C20 - Video Surface 0 Color Matrix A R5 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MA_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MA_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MA_COEFF_C21 - Video Surface 0 Color Matrix A R5 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MA_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MA_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MA_COEFF_C22 - Video Surface 0 Color Matrix A R5 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MA_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MA_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R5_MA_COEFF_C23 - Video Surface 0 Color Matrix A R5 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MA_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MA_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MA_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MB_COEFF_C00 - Video Surface 0 Color Matrix B R0 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MB_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MB_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R0_MB_COEFF_C01 - Video Surface 0 Color Matrix B R0 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MB_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MB_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MB_COEFF_C02 - Video Surface 0 Color Matrix B R0 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MB_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MB_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MB_COEFF_C03 - Video Surface 0 Color Matrix B R0 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MB_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MB_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MB_COEFF_C10 - Video Surface 0 Color Matrix B R0 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MB_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MB_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MB_COEFF_C11 - Video Surface 0 Color Matrix B R0 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MB_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MB_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R0_MB_COEFF_C12 - Video Surface 0 Color Matrix B R0 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MB_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MB_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MB_COEFF_C13 - Video Surface 0 Color Matrix B R0 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MB_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MB_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MB_COEFF_C20 - Video Surface 0 Color Matrix B R0 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MB_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MB_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MB_COEFF_C21 - Video Surface 0 Color Matrix B R0 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MB_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MB_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_MB_COEFF_C22 - Video Surface 0 Color Matrix B R0 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MB_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MB_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R0_MB_COEFF_C23 - Video Surface 0 Color Matrix B R0 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_MB_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R0_MB_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MB_COEFF_C00 - Video Surface 0 Color Matrix B R1 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MB_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MB_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R1_MB_COEFF_C01 - Video Surface 0 Color Matrix B R1 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MB_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MB_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MB_COEFF_C02 - Video Surface 0 Color Matrix B R1 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MB_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MB_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MB_COEFF_C03 - Video Surface 0 Color Matrix B R1 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MB_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MB_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MB_COEFF_C10 - Video Surface 0 Color Matrix B R1 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MB_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MB_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MB_COEFF_C11 - Video Surface 0 Color Matrix B R1 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MB_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MB_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R1_MB_COEFF_C12 - Video Surface 0 Color Matrix B R1 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MB_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MB_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MB_COEFF_C13 - Video Surface 0 Color Matrix B R1 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MB_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MB_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MB_COEFF_C20 - Video Surface 0 Color Matrix B R1 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MB_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MB_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MB_COEFF_C21 - Video Surface 0 Color Matrix B R1 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MB_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MB_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R1_MB_COEFF_C22 - Video Surface 0 Color Matrix B R1 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MB_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MB_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R1_MB_COEFF_C23 - Video Surface 0 Color Matrix B R1 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_MB_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R1_MB_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MB_COEFF_C00 - Video Surface 0 Color Matrix B R2 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MB_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MB_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R2_MB_COEFF_C01 - Video Surface 0 Color Matrix B R2 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MB_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MB_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MB_COEFF_C02 - Video Surface 0 Color Matrix B R2 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MB_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MB_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MB_COEFF_C03 - Video Surface 0 Color Matrix B R2 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MB_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MB_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MB_COEFF_C10 - Video Surface 0 Color Matrix B R2 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MB_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MB_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MB_COEFF_C11 - Video Surface 0 Color Matrix B R2 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MB_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MB_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R2_MB_COEFF_C12 - Video Surface 0 Color Matrix B R2 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MB_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MB_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MB_COEFF_C13 - Video Surface 0 Color Matrix B R2 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MB_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MB_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MB_COEFF_C20 - Video Surface 0 Color Matrix B R2 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MB_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MB_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MB_COEFF_C21 - Video Surface 0 Color Matrix B R2 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MB_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MB_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R2_MB_COEFF_C22 - Video Surface 0 Color Matrix B R2 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MB_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MB_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R2_MB_COEFF_C23 - Video Surface 0 Color Matrix B R2 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_MB_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R2_MB_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R2_MB_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MB_COEFF_C00 - Video Surface 0 Color Matrix B R3 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MB_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MB_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R3_MB_COEFF_C01 - Video Surface 0 Color Matrix B R3 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MB_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MB_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MB_COEFF_C02 - Video Surface 0 Color Matrix B R3 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MB_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MB_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MB_COEFF_C03 - Video Surface 0 Color Matrix B R3 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MB_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MB_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MB_COEFF_C10 - Video Surface 0 Color Matrix B R3 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MB_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MB_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MB_COEFF_C11 - Video Surface 0 Color Matrix B R3 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MB_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MB_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R3_MB_COEFF_C12 - Video Surface 0 Color Matrix B R3 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MB_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MB_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MB_COEFF_C13 - Video Surface 0 Color Matrix B R3 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MB_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MB_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MB_COEFF_C20 - Video Surface 0 Color Matrix B R3 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MB_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MB_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MB_COEFF_C21 - Video Surface 0 Color Matrix B R3 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MB_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MB_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R3_MB_COEFF_C22 - Video Surface 0 Color Matrix B R3 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MB_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MB_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R3_MB_COEFF_C23 - Video Surface 0 Color Matrix B R3 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_MB_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R3_MB_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R3_MB_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MB_COEFF_C00 - Video Surface 0 Color Matrix B R4 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MB_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MB_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R4_MB_COEFF_C01 - Video Surface 0 Color Matrix B R4 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MB_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MB_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MB_COEFF_C02 - Video Surface 0 Color Matrix B R4 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MB_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MB_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MB_COEFF_C03 - Video Surface 0 Color Matrix B R4 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MB_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MB_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MB_COEFF_C10 - Video Surface 0 Color Matrix B R4 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MB_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MB_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MB_COEFF_C11 - Video Surface 0 Color Matrix B R4 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MB_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MB_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R4_MB_COEFF_C12 - Video Surface 0 Color Matrix B R4 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MB_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MB_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MB_COEFF_C13 - Video Surface 0 Color Matrix B R4 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MB_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MB_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MB_COEFF_C20 - Video Surface 0 Color Matrix B R4 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MB_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MB_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MB_COEFF_C21 - Video Surface 0 Color Matrix B R4 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MB_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MB_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R4_MB_COEFF_C22 - Video Surface 0 Color Matrix B R4 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MB_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MB_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R4_MB_COEFF_C23 - Video Surface 0 Color Matrix B R4 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_MB_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R4_MB_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R4_MB_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MB_COEFF_C00 - Video Surface 0 Color Matrix B R5 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MB_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MB_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R5_MB_COEFF_C01 - Video Surface 0 Color Matrix B R5 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MB_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MB_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MB_COEFF_C02 - Video Surface 0 Color Matrix B R5 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MB_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MB_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MB_COEFF_C03 - Video Surface 0 Color Matrix B R5 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MB_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MB_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MB_COEFF_C10 - Video Surface 0 Color Matrix B R5 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MB_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MB_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MB_COEFF_C11 - Video Surface 0 Color Matrix B R5 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MB_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MB_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R5_MB_COEFF_C12 - Video Surface 0 Color Matrix B R5 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MB_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MB_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MB_COEFF_C13 - Video Surface 0 Color Matrix B R5 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MB_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MB_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MB_COEFF_C20 - Video Surface 0 Color Matrix B R5 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MB_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MB_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MB_COEFF_C21 - Video Surface 0 Color Matrix B R5 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MB_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MB_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V0_R5_MB_COEFF_C22 - Video Surface 0 Color Matrix B R5 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MB_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MB_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V0_R5_MB_COEFF_C23 - Video Surface 0 Color Matrix B R5 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_MB_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V0_R5_MB_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V0_R5_MB_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V0_R0_NL_LR_SLOPE%i - Video Surface 0 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_BASE               0x20648880
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_START              0
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_END                7
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *V0_R0_NL_LR_SLOPE%i - Video Surface 0 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_NL_LR_SLOPEi :: SLOPE_M [31:16] */
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_SLOPE_M_MASK             0xffff0000
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_SLOPE_M_SHIFT            16
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_SLOPE_M_DEFAULT          0x00000000

/* HDR_CMP_0 :: V0_R0_NL_LR_SLOPEi :: reserved0 [15:05] */
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_reserved0_MASK           0x0000ffe0
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_reserved0_SHIFT          5

/* HDR_CMP_0 :: V0_R0_NL_LR_SLOPEi :: SLOPE_E [04:00] */
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_SLOPE_E_MASK             0x0000001f
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_SLOPE_E_SHIFT            0
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_SLOPE_E_DEFAULT          0x00000000


/***************************************************************************
 *V0_R0_NL_LR_XY_0%i - Video Surface 0 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_ARRAY_BASE                0x206488a0
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_ARRAY_START               0
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_ARRAY_END                 7
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *V0_R0_NL_LR_XY_0%i - Video Surface 0 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_NL_LR_XY_0i :: LRA_Y [31:16] */
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_Y_MASK                0xffff0000
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_Y_SHIFT               16
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_Y_DEFAULT             0x00000000

/* HDR_CMP_0 :: V0_R0_NL_LR_XY_0i :: LRA_X [15:00] */
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_X_MASK                0x0000ffff
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_X_SHIFT               0
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_XY_0i_LRA_X_DEFAULT             0x00008000


/***************************************************************************
 *V0_R0_NL_CSC_CTRL - Video Surface 0 Nonconstant Luminance CSC Control
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R0_NL_CSC_CTRL :: reserved0 [31:03] */
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL_reserved0_MASK            0xfffffff8
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL_reserved0_SHIFT           3

/* HDR_CMP_0 :: V0_R0_NL_CSC_CTRL :: SEL_CL_IN [02:02] */
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL_SEL_CL_IN_MASK            0x00000004
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL_SEL_CL_IN_SHIFT           2
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL_SEL_CL_IN_DEFAULT         0x00000000

/* HDR_CMP_0 :: V0_R0_NL_CSC_CTRL :: SEL_XVYCC [01:01] */
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL_SEL_XVYCC_MASK            0x00000002
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL_SEL_XVYCC_SHIFT           1
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL_SEL_XVYCC_DEFAULT         0x00000000
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL_SEL_XVYCC_NOT_XVYCC       0
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL_SEL_XVYCC_XVYCC           1

/* HDR_CMP_0 :: V0_R0_NL_CSC_CTRL :: reserved1 [00:00] */
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL_reserved1_MASK            0x00000001
#define BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL_reserved1_SHIFT           0

/***************************************************************************
 *V0_R1_NL_LR_SLOPE%i - Video Surface 0 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_ARRAY_BASE               0x206488d0
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_ARRAY_START              0
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_ARRAY_END                7
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *V0_R1_NL_LR_SLOPE%i - Video Surface 0 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_NL_LR_SLOPEi :: SLOPE_M [31:16] */
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_SLOPE_M_MASK             0xffff0000
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_SLOPE_M_SHIFT            16
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_SLOPE_M_DEFAULT          0x00000000

/* HDR_CMP_0 :: V0_R1_NL_LR_SLOPEi :: reserved0 [15:05] */
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_reserved0_MASK           0x0000ffe0
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_reserved0_SHIFT          5

/* HDR_CMP_0 :: V0_R1_NL_LR_SLOPEi :: SLOPE_E [04:00] */
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_SLOPE_E_MASK             0x0000001f
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_SLOPE_E_SHIFT            0
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_SLOPE_E_DEFAULT          0x00000000


/***************************************************************************
 *V0_R1_NL_LR_XY_0%i - Video Surface 0 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_XY_0i_ARRAY_BASE                0x206488f0
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_XY_0i_ARRAY_START               0
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_XY_0i_ARRAY_END                 7
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_XY_0i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *V0_R1_NL_LR_XY_0%i - Video Surface 0 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_NL_LR_XY_0i :: LRA_Y [31:16] */
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_XY_0i_LRA_Y_MASK                0xffff0000
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_XY_0i_LRA_Y_SHIFT               16
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_XY_0i_LRA_Y_DEFAULT             0x00000000

/* HDR_CMP_0 :: V0_R1_NL_LR_XY_0i :: LRA_X [15:00] */
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_XY_0i_LRA_X_MASK                0x0000ffff
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_XY_0i_LRA_X_SHIFT               0
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_XY_0i_LRA_X_DEFAULT             0x00008000


/***************************************************************************
 *V0_R1_NL_CSC_CTRL - Video Surface 0 Nonconstant Luminance CSC Control
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R1_NL_CSC_CTRL :: reserved0 [31:03] */
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL_reserved0_MASK            0xfffffff8
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL_reserved0_SHIFT           3

/* HDR_CMP_0 :: V0_R1_NL_CSC_CTRL :: SEL_CL_IN [02:02] */
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL_SEL_CL_IN_MASK            0x00000004
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL_SEL_CL_IN_SHIFT           2
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL_SEL_CL_IN_DEFAULT         0x00000000

/* HDR_CMP_0 :: V0_R1_NL_CSC_CTRL :: SEL_XVYCC [01:01] */
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL_SEL_XVYCC_MASK            0x00000002
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL_SEL_XVYCC_SHIFT           1
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL_SEL_XVYCC_DEFAULT         0x00000000
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL_SEL_XVYCC_NOT_XVYCC       0
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL_SEL_XVYCC_XVYCC           1

/* HDR_CMP_0 :: V0_R1_NL_CSC_CTRL :: reserved1 [00:00] */
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL_reserved1_MASK            0x00000001
#define BCHP_HDR_CMP_0_V0_R1_NL_CSC_CTRL_reserved1_SHIFT           0

/***************************************************************************
 *V0_R2_NL_LR_SLOPE%i - Video Surface 0 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_SLOPEi_ARRAY_BASE               0x20648920
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_SLOPEi_ARRAY_START              0
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_SLOPEi_ARRAY_END                7
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_SLOPEi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *V0_R2_NL_LR_SLOPE%i - Video Surface 0 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_NL_LR_SLOPEi :: SLOPE_M [31:16] */
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_SLOPEi_SLOPE_M_MASK             0xffff0000
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_SLOPEi_SLOPE_M_SHIFT            16
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_SLOPEi_SLOPE_M_DEFAULT          0x00000000

/* HDR_CMP_0 :: V0_R2_NL_LR_SLOPEi :: reserved0 [15:05] */
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_SLOPEi_reserved0_MASK           0x0000ffe0
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_SLOPEi_reserved0_SHIFT          5

/* HDR_CMP_0 :: V0_R2_NL_LR_SLOPEi :: SLOPE_E [04:00] */
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_SLOPEi_SLOPE_E_MASK             0x0000001f
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_SLOPEi_SLOPE_E_SHIFT            0
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_SLOPEi_SLOPE_E_DEFAULT          0x00000000


/***************************************************************************
 *V0_R2_NL_LR_XY_0%i - Video Surface 0 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_XY_0i_ARRAY_BASE                0x20648940
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_XY_0i_ARRAY_START               0
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_XY_0i_ARRAY_END                 7
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_XY_0i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *V0_R2_NL_LR_XY_0%i - Video Surface 0 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_NL_LR_XY_0i :: LRA_Y [31:16] */
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_XY_0i_LRA_Y_MASK                0xffff0000
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_XY_0i_LRA_Y_SHIFT               16
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_XY_0i_LRA_Y_DEFAULT             0x00000000

/* HDR_CMP_0 :: V0_R2_NL_LR_XY_0i :: LRA_X [15:00] */
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_XY_0i_LRA_X_MASK                0x0000ffff
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_XY_0i_LRA_X_SHIFT               0
#define BCHP_HDR_CMP_0_V0_R2_NL_LR_XY_0i_LRA_X_DEFAULT             0x00008000


/***************************************************************************
 *V0_R2_NL_CSC_CTRL - Video Surface 0 Nonconstant Luminance CSC Control
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R2_NL_CSC_CTRL :: reserved0 [31:03] */
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL_reserved0_MASK            0xfffffff8
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL_reserved0_SHIFT           3

/* HDR_CMP_0 :: V0_R2_NL_CSC_CTRL :: SEL_CL_IN [02:02] */
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL_SEL_CL_IN_MASK            0x00000004
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL_SEL_CL_IN_SHIFT           2
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL_SEL_CL_IN_DEFAULT         0x00000000

/* HDR_CMP_0 :: V0_R2_NL_CSC_CTRL :: SEL_XVYCC [01:01] */
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL_SEL_XVYCC_MASK            0x00000002
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL_SEL_XVYCC_SHIFT           1
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL_SEL_XVYCC_DEFAULT         0x00000000
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL_SEL_XVYCC_NOT_XVYCC       0
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL_SEL_XVYCC_XVYCC           1

/* HDR_CMP_0 :: V0_R2_NL_CSC_CTRL :: reserved1 [00:00] */
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL_reserved1_MASK            0x00000001
#define BCHP_HDR_CMP_0_V0_R2_NL_CSC_CTRL_reserved1_SHIFT           0

/***************************************************************************
 *V0_R3_NL_LR_SLOPE%i - Video Surface 0 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_SLOPEi_ARRAY_BASE               0x20648970
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_SLOPEi_ARRAY_START              0
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_SLOPEi_ARRAY_END                7
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_SLOPEi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *V0_R3_NL_LR_SLOPE%i - Video Surface 0 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_NL_LR_SLOPEi :: SLOPE_M [31:16] */
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_SLOPEi_SLOPE_M_MASK             0xffff0000
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_SLOPEi_SLOPE_M_SHIFT            16
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_SLOPEi_SLOPE_M_DEFAULT          0x00000000

/* HDR_CMP_0 :: V0_R3_NL_LR_SLOPEi :: reserved0 [15:05] */
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_SLOPEi_reserved0_MASK           0x0000ffe0
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_SLOPEi_reserved0_SHIFT          5

/* HDR_CMP_0 :: V0_R3_NL_LR_SLOPEi :: SLOPE_E [04:00] */
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_SLOPEi_SLOPE_E_MASK             0x0000001f
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_SLOPEi_SLOPE_E_SHIFT            0
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_SLOPEi_SLOPE_E_DEFAULT          0x00000000


/***************************************************************************
 *V0_R3_NL_LR_XY_0%i - Video Surface 0 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_XY_0i_ARRAY_BASE                0x20648990
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_XY_0i_ARRAY_START               0
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_XY_0i_ARRAY_END                 7
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_XY_0i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *V0_R3_NL_LR_XY_0%i - Video Surface 0 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_NL_LR_XY_0i :: LRA_Y [31:16] */
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_XY_0i_LRA_Y_MASK                0xffff0000
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_XY_0i_LRA_Y_SHIFT               16
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_XY_0i_LRA_Y_DEFAULT             0x00000000

/* HDR_CMP_0 :: V0_R3_NL_LR_XY_0i :: LRA_X [15:00] */
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_XY_0i_LRA_X_MASK                0x0000ffff
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_XY_0i_LRA_X_SHIFT               0
#define BCHP_HDR_CMP_0_V0_R3_NL_LR_XY_0i_LRA_X_DEFAULT             0x00008000


/***************************************************************************
 *V0_R3_NL_CSC_CTRL - Video Surface 0 Nonconstant Luminance CSC Control
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R3_NL_CSC_CTRL :: reserved0 [31:03] */
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL_reserved0_MASK            0xfffffff8
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL_reserved0_SHIFT           3

/* HDR_CMP_0 :: V0_R3_NL_CSC_CTRL :: SEL_CL_IN [02:02] */
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL_SEL_CL_IN_MASK            0x00000004
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL_SEL_CL_IN_SHIFT           2
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL_SEL_CL_IN_DEFAULT         0x00000000

/* HDR_CMP_0 :: V0_R3_NL_CSC_CTRL :: SEL_XVYCC [01:01] */
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL_SEL_XVYCC_MASK            0x00000002
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL_SEL_XVYCC_SHIFT           1
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL_SEL_XVYCC_DEFAULT         0x00000000
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL_SEL_XVYCC_NOT_XVYCC       0
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL_SEL_XVYCC_XVYCC           1

/* HDR_CMP_0 :: V0_R3_NL_CSC_CTRL :: reserved1 [00:00] */
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL_reserved1_MASK            0x00000001
#define BCHP_HDR_CMP_0_V0_R3_NL_CSC_CTRL_reserved1_SHIFT           0

/***************************************************************************
 *V0_R4_NL_LR_SLOPE%i - Video Surface 0 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_SLOPEi_ARRAY_BASE               0x206489c0
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_SLOPEi_ARRAY_START              0
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_SLOPEi_ARRAY_END                7
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_SLOPEi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *V0_R4_NL_LR_SLOPE%i - Video Surface 0 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_NL_LR_SLOPEi :: SLOPE_M [31:16] */
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_SLOPEi_SLOPE_M_MASK             0xffff0000
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_SLOPEi_SLOPE_M_SHIFT            16
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_SLOPEi_SLOPE_M_DEFAULT          0x00000000

/* HDR_CMP_0 :: V0_R4_NL_LR_SLOPEi :: reserved0 [15:05] */
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_SLOPEi_reserved0_MASK           0x0000ffe0
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_SLOPEi_reserved0_SHIFT          5

/* HDR_CMP_0 :: V0_R4_NL_LR_SLOPEi :: SLOPE_E [04:00] */
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_SLOPEi_SLOPE_E_MASK             0x0000001f
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_SLOPEi_SLOPE_E_SHIFT            0
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_SLOPEi_SLOPE_E_DEFAULT          0x00000000


/***************************************************************************
 *V0_R4_NL_LR_XY_0%i - Video Surface 0 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_XY_0i_ARRAY_BASE                0x206489e0
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_XY_0i_ARRAY_START               0
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_XY_0i_ARRAY_END                 7
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_XY_0i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *V0_R4_NL_LR_XY_0%i - Video Surface 0 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_NL_LR_XY_0i :: LRA_Y [31:16] */
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_XY_0i_LRA_Y_MASK                0xffff0000
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_XY_0i_LRA_Y_SHIFT               16
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_XY_0i_LRA_Y_DEFAULT             0x00000000

/* HDR_CMP_0 :: V0_R4_NL_LR_XY_0i :: LRA_X [15:00] */
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_XY_0i_LRA_X_MASK                0x0000ffff
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_XY_0i_LRA_X_SHIFT               0
#define BCHP_HDR_CMP_0_V0_R4_NL_LR_XY_0i_LRA_X_DEFAULT             0x00008000


/***************************************************************************
 *V0_R4_NL_CSC_CTRL - Video Surface 0 Nonconstant Luminance CSC Control
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R4_NL_CSC_CTRL :: reserved0 [31:03] */
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL_reserved0_MASK            0xfffffff8
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL_reserved0_SHIFT           3

/* HDR_CMP_0 :: V0_R4_NL_CSC_CTRL :: SEL_CL_IN [02:02] */
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL_SEL_CL_IN_MASK            0x00000004
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL_SEL_CL_IN_SHIFT           2
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL_SEL_CL_IN_DEFAULT         0x00000000

/* HDR_CMP_0 :: V0_R4_NL_CSC_CTRL :: SEL_XVYCC [01:01] */
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL_SEL_XVYCC_MASK            0x00000002
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL_SEL_XVYCC_SHIFT           1
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL_SEL_XVYCC_DEFAULT         0x00000000
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL_SEL_XVYCC_NOT_XVYCC       0
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL_SEL_XVYCC_XVYCC           1

/* HDR_CMP_0 :: V0_R4_NL_CSC_CTRL :: reserved1 [00:00] */
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL_reserved1_MASK            0x00000001
#define BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL_reserved1_SHIFT           0

/***************************************************************************
 *V0_R5_NL_LR_SLOPE%i - Video Surface 0 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_ARRAY_BASE               0x20648a20
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_ARRAY_START              0
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_ARRAY_END                7
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *V0_R5_NL_LR_SLOPE%i - Video Surface 0 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_NL_LR_SLOPEi :: SLOPE_M [31:16] */
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_SLOPE_M_MASK             0xffff0000
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_SLOPE_M_SHIFT            16
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_SLOPE_M_DEFAULT          0x00000000

/* HDR_CMP_0 :: V0_R5_NL_LR_SLOPEi :: reserved0 [15:05] */
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_reserved0_MASK           0x0000ffe0
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_reserved0_SHIFT          5

/* HDR_CMP_0 :: V0_R5_NL_LR_SLOPEi :: SLOPE_E [04:00] */
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_SLOPE_E_MASK             0x0000001f
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_SLOPE_E_SHIFT            0
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_SLOPE_E_DEFAULT          0x00000000


/***************************************************************************
 *V0_R5_NL_LR_XY_0%i - Video Surface 0 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_XY_0i_ARRAY_BASE                0x20648a40
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_XY_0i_ARRAY_START               0
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_XY_0i_ARRAY_END                 7
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_XY_0i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *V0_R5_NL_LR_XY_0%i - Video Surface 0 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_NL_LR_XY_0i :: LRA_Y [31:16] */
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_XY_0i_LRA_Y_MASK                0xffff0000
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_XY_0i_LRA_Y_SHIFT               16
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_XY_0i_LRA_Y_DEFAULT             0x00000000

/* HDR_CMP_0 :: V0_R5_NL_LR_XY_0i :: LRA_X [15:00] */
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_XY_0i_LRA_X_MASK                0x0000ffff
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_XY_0i_LRA_X_SHIFT               0
#define BCHP_HDR_CMP_0_V0_R5_NL_LR_XY_0i_LRA_X_DEFAULT             0x00008000


/***************************************************************************
 *V0_R5_NL_CSC_CTRL - Video Surface 0 Nonconstant Luminance CSC Control
 ***************************************************************************/
/* HDR_CMP_0 :: V0_R5_NL_CSC_CTRL :: reserved0 [31:03] */
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL_reserved0_MASK            0xfffffff8
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL_reserved0_SHIFT           3

/* HDR_CMP_0 :: V0_R5_NL_CSC_CTRL :: SEL_CL_IN [02:02] */
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL_SEL_CL_IN_MASK            0x00000004
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL_SEL_CL_IN_SHIFT           2
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL_SEL_CL_IN_DEFAULT         0x00000000

/* HDR_CMP_0 :: V0_R5_NL_CSC_CTRL :: SEL_XVYCC [01:01] */
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL_SEL_XVYCC_MASK            0x00000002
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL_SEL_XVYCC_SHIFT           1
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL_SEL_XVYCC_DEFAULT         0x00000000
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL_SEL_XVYCC_NOT_XVYCC       0
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL_SEL_XVYCC_XVYCC           1

/* HDR_CMP_0 :: V0_R5_NL_CSC_CTRL :: reserved1 [00:00] */
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL_reserved1_MASK            0x00000001
#define BCHP_HDR_CMP_0_V0_R5_NL_CSC_CTRL_reserved1_SHIFT           0

/***************************************************************************
 *V0_BLENDER_IN_CSC_EN - Video Surface 0 Blender In Color Matrix Enable
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_CSC_EN :: reserved0 [31:01] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_CSC_EN_reserved0_MASK         0xfffffffe
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_CSC_EN_reserved0_SHIFT        1

/* HDR_CMP_0 :: V0_BLENDER_IN_CSC_EN :: BLENDER_IN_CSC_ENABLE [00:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_CSC_EN_BLENDER_IN_CSC_ENABLE_MASK 0x00000001
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_CSC_EN_BLENDER_IN_CSC_ENABLE_SHIFT 0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_CSC_EN_BLENDER_IN_CSC_ENABLE_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_CSC_EN_BLENDER_IN_CSC_ENABLE_DISABLE 0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_CSC_EN_BLENDER_IN_CSC_ENABLE_ENABLE 1

/***************************************************************************
 *V0_BLENDER_IN_COEFF_C00 - Video Surface 0 Blender In Color Matrix coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C00 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C00_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C00_reserved0_SHIFT     16

/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C00 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C00_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C00_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C00_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V0_BLENDER_IN_COEFF_C01 - Video Surface 0 Blender In Color Matrix coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C01 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C01_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C01_reserved0_SHIFT     16

/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C01 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C01_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C01_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C01_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V0_BLENDER_IN_COEFF_C02 - Video Surface 0 Blender In Color Matrix coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C02 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C02_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C02_reserved0_SHIFT     16

/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C02 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C02_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C02_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C02_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V0_BLENDER_IN_COEFF_C03 - Video Surface 0 Blender In Color Matrix coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C03 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C03_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C03_reserved0_SHIFT     16

/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C03 :: COEFF_ADD [15:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C03_COEFF_ADD_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C03_COEFF_ADD_SHIFT     0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C03_COEFF_ADD_DEFAULT   0x00000000

/***************************************************************************
 *V0_BLENDER_IN_COEFF_C10 - Video Surface 0 Blender In Color Matrix coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C10 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C10_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C10_reserved0_SHIFT     16

/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C10 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C10_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C10_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C10_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V0_BLENDER_IN_COEFF_C11 - Video Surface 0 Blender In Color Matrix coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C11 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C11_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C11_reserved0_SHIFT     16

/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C11 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C11_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C11_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C11_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V0_BLENDER_IN_COEFF_C12 - Video Surface 0 Blender In Color Matrix coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C12 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C12_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C12_reserved0_SHIFT     16

/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C12 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C12_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C12_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C12_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V0_BLENDER_IN_COEFF_C13 - Video Surface 0 Blender In Color Matrix coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C13 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C13_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C13_reserved0_SHIFT     16

/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C13 :: COEFF_ADD [15:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C13_COEFF_ADD_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C13_COEFF_ADD_SHIFT     0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C13_COEFF_ADD_DEFAULT   0x00000000

/***************************************************************************
 *V0_BLENDER_IN_COEFF_C20 - Video Surface 0 Blender In Color Matrix coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C20 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C20_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C20_reserved0_SHIFT     16

/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C20 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C20_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C20_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C20_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V0_BLENDER_IN_COEFF_C21 - Video Surface 0 Blender In Color Matrix coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C21 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C21_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C21_reserved0_SHIFT     16

/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C21 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C21_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C21_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C21_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V0_BLENDER_IN_COEFF_C22 - Video Surface 0 Blender In Color Matrix coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C22 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C22_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C22_reserved0_SHIFT     16

/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C22 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C22_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C22_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C22_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V0_BLENDER_IN_COEFF_C23 - Video Surface 0 Blender In Color Matrix coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C23 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C23_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C23_reserved0_SHIFT     16

/* HDR_CMP_0 :: V0_BLENDER_IN_COEFF_C23 :: COEFF_ADD [15:00] */
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C23_COEFF_ADD_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C23_COEFF_ADD_SHIFT     0
#define BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C23_COEFF_ADD_DEFAULT   0x00000000

/***************************************************************************
 *V0_NL_LUT_CTRL - Video Surface 0 NL2N LUT Control (Control for TEST_RAM)
 ***************************************************************************/
/* HDR_CMP_0 :: V0_NL_LUT_CTRL :: reserved0 [31:14] */
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_reserved0_MASK               0xffffc000
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_reserved0_SHIFT              14

/* HDR_CMP_0 :: V0_NL_LUT_CTRL :: NL_READ_RAM_SEL [13:12] */
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_NL_READ_RAM_SEL_MASK         0x00003000
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_NL_READ_RAM_SEL_SHIFT        12
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_NL_READ_RAM_SEL_DEFAULT      0x00000000

/* HDR_CMP_0 :: V0_NL_LUT_CTRL :: NL_LUT_NUM_SEG [11:08] */
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_NL_LUT_NUM_SEG_MASK          0x00000f00
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_NL_LUT_NUM_SEG_SHIFT         8
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_NL_LUT_NUM_SEG_DEFAULT       0x00000000

/* HDR_CMP_0 :: V0_NL_LUT_CTRL :: reserved1 [07:05] */
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_reserved1_MASK               0x000000e0
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_reserved1_SHIFT              5

/* HDR_CMP_0 :: V0_NL_LUT_CTRL :: NL_LUT_OINT_BITS [04:03] */
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_NL_LUT_OINT_BITS_MASK        0x00000018
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_NL_LUT_OINT_BITS_SHIFT       3
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_NL_LUT_OINT_BITS_DEFAULT     0x00000000

/* HDR_CMP_0 :: V0_NL_LUT_CTRL :: NL_LUT_XSCL [02:00] */
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_NL_LUT_XSCL_MASK             0x00000007
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_NL_LUT_XSCL_SHIFT            0
#define BCHP_HDR_CMP_0_V0_NL_LUT_CTRL_NL_LUT_XSCL_DEFAULT          0x00000000

/***************************************************************************
 *V0_NL_LUT_SEG_CTRL%i - Video Surface 0 NL2N LUT Segment control [0..5] (Control for TEST_RAM)
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_ARRAY_BASE              0x20648b04
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_ARRAY_START             0
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_ARRAY_END               5
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_ARRAY_ELEMENT_SIZE      32

/***************************************************************************
 *V0_NL_LUT_SEG_CTRL%i - Video Surface 0 NL2N LUT Segment control [0..5] (Control for TEST_RAM)
 ***************************************************************************/
/* HDR_CMP_0 :: V0_NL_LUT_SEG_CTRLi :: reserved0 [31:29] */
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_reserved0_MASK          0xe0000000
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_reserved0_SHIFT         29

/* HDR_CMP_0 :: V0_NL_LUT_SEG_CTRLi :: NL_LUT_SEG_INT_BITS [28:24] */
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_NL_LUT_SEG_INT_BITS_MASK 0x1f000000
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_NL_LUT_SEG_INT_BITS_SHIFT 24
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_NL_LUT_SEG_INT_BITS_DEFAULT 0x00000000

/* HDR_CMP_0 :: V0_NL_LUT_SEG_CTRLi :: reserved1 [23:22] */
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_reserved1_MASK          0x00c00000
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_reserved1_SHIFT         22

/* HDR_CMP_0 :: V0_NL_LUT_SEG_CTRLi :: NL_LUT_SEG_INT_OFFSET [21:11] */
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_NL_LUT_SEG_INT_OFFSET_MASK 0x003ff800
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_NL_LUT_SEG_INT_OFFSET_SHIFT 11
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_NL_LUT_SEG_INT_OFFSET_DEFAULT 0x00000000

/* HDR_CMP_0 :: V0_NL_LUT_SEG_CTRLi :: NL_LUT_SEG_END [10:00] */
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_NL_LUT_SEG_END_MASK     0x000007ff
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_NL_LUT_SEG_END_SHIFT    0
#define BCHP_HDR_CMP_0_V0_NL_LUT_SEG_CTRLi_NL_LUT_SEG_END_DEFAULT  0x00000000


/***************************************************************************
 *V0_NL2L_TF_LUT%i - Video Surface 0 NL2N NL to L transfer function LUT (TEST_RAM)
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_NL2L_TF_LUTi_ARRAY_BASE                  0x20648c00
#define BCHP_HDR_CMP_0_V0_NL2L_TF_LUTi_ARRAY_START                 0
#define BCHP_HDR_CMP_0_V0_NL2L_TF_LUTi_ARRAY_END                   1200
#define BCHP_HDR_CMP_0_V0_NL2L_TF_LUTi_ARRAY_ELEMENT_SIZE          32

/***************************************************************************
 *V0_NL2L_TF_LUT%i - Video Surface 0 NL2N NL to L transfer function LUT (TEST_RAM)
 ***************************************************************************/
/* HDR_CMP_0 :: V0_NL2L_TF_LUTi :: reserved0 [31:26] */
#define BCHP_HDR_CMP_0_V0_NL2L_TF_LUTi_reserved0_MASK              0xfc000000
#define BCHP_HDR_CMP_0_V0_NL2L_TF_LUTi_reserved0_SHIFT             26

/* HDR_CMP_0 :: V0_NL2L_TF_LUTi :: NL2_L_TF_VAL [25:00] */
#define BCHP_HDR_CMP_0_V0_NL2L_TF_LUTi_NL2_L_TF_VAL_MASK           0x03ffffff
#define BCHP_HDR_CMP_0_V0_NL2L_TF_LUTi_NL2_L_TF_VAL_SHIFT          0


/***************************************************************************
 *V0_LN_LUT_CTRL - Video Surface 0 L2NL LUT Control (Control for TEST_RAM)
 ***************************************************************************/
/* HDR_CMP_0 :: V0_LN_LUT_CTRL :: reserved0 [31:14] */
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_reserved0_MASK               0xffffc000
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_reserved0_SHIFT              14

/* HDR_CMP_0 :: V0_LN_LUT_CTRL :: NL_READ_RAM_SEL [13:12] */
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_NL_READ_RAM_SEL_MASK         0x00003000
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_NL_READ_RAM_SEL_SHIFT        12
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_NL_READ_RAM_SEL_DEFAULT      0x00000000

/* HDR_CMP_0 :: V0_LN_LUT_CTRL :: NL_LUT_NUM_SEG [11:08] */
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_NL_LUT_NUM_SEG_MASK          0x00000f00
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_NL_LUT_NUM_SEG_SHIFT         8
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_NL_LUT_NUM_SEG_DEFAULT       0x00000000

/* HDR_CMP_0 :: V0_LN_LUT_CTRL :: reserved1 [07:05] */
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_reserved1_MASK               0x000000e0
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_reserved1_SHIFT              5

/* HDR_CMP_0 :: V0_LN_LUT_CTRL :: NL_LUT_OINT_BITS [04:03] */
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_NL_LUT_OINT_BITS_MASK        0x00000018
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_NL_LUT_OINT_BITS_SHIFT       3
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_NL_LUT_OINT_BITS_DEFAULT     0x00000000

/* HDR_CMP_0 :: V0_LN_LUT_CTRL :: NL_LUT_XSCL [02:00] */
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_NL_LUT_XSCL_MASK             0x00000007
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_NL_LUT_XSCL_SHIFT            0
#define BCHP_HDR_CMP_0_V0_LN_LUT_CTRL_NL_LUT_XSCL_DEFAULT          0x00000000

/***************************************************************************
 *V0_LN_LUT_SEG_CTRL%i - Video Surface 0 L2NL LUT Segment control [0..5] (Control for TEST_RAM)
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_ARRAY_BASE              0x20649f04
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_ARRAY_START             0
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_ARRAY_END               5
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_ARRAY_ELEMENT_SIZE      32

/***************************************************************************
 *V0_LN_LUT_SEG_CTRL%i - Video Surface 0 L2NL LUT Segment control [0..5] (Control for TEST_RAM)
 ***************************************************************************/
/* HDR_CMP_0 :: V0_LN_LUT_SEG_CTRLi :: reserved0 [31:29] */
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_reserved0_MASK          0xe0000000
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_reserved0_SHIFT         29

/* HDR_CMP_0 :: V0_LN_LUT_SEG_CTRLi :: NL_LUT_SEG_INT_BITS [28:24] */
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_NL_LUT_SEG_INT_BITS_MASK 0x1f000000
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_NL_LUT_SEG_INT_BITS_SHIFT 24
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_NL_LUT_SEG_INT_BITS_DEFAULT 0x00000000

/* HDR_CMP_0 :: V0_LN_LUT_SEG_CTRLi :: reserved1 [23:22] */
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_reserved1_MASK          0x00c00000
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_reserved1_SHIFT         22

/* HDR_CMP_0 :: V0_LN_LUT_SEG_CTRLi :: NL_LUT_SEG_INT_OFFSET [21:11] */
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_NL_LUT_SEG_INT_OFFSET_MASK 0x003ff800
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_NL_LUT_SEG_INT_OFFSET_SHIFT 11
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_NL_LUT_SEG_INT_OFFSET_DEFAULT 0x00000000

/* HDR_CMP_0 :: V0_LN_LUT_SEG_CTRLi :: NL_LUT_SEG_END [10:00] */
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_NL_LUT_SEG_END_MASK     0x000007ff
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_NL_LUT_SEG_END_SHIFT    0
#define BCHP_HDR_CMP_0_V0_LN_LUT_SEG_CTRLi_NL_LUT_SEG_END_DEFAULT  0x00000000


/***************************************************************************
 *V0_L2NL_TF_LUT%i - Video Surface 0 L2NL L to NL transfer function LUT (TEST_RAM)
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V0_L2NL_TF_LUTi_ARRAY_BASE                  0x2064a000
#define BCHP_HDR_CMP_0_V0_L2NL_TF_LUTi_ARRAY_START                 0
#define BCHP_HDR_CMP_0_V0_L2NL_TF_LUTi_ARRAY_END                   275
#define BCHP_HDR_CMP_0_V0_L2NL_TF_LUTi_ARRAY_ELEMENT_SIZE          32

/***************************************************************************
 *V0_L2NL_TF_LUT%i - Video Surface 0 L2NL L to NL transfer function LUT (TEST_RAM)
 ***************************************************************************/
/* HDR_CMP_0 :: V0_L2NL_TF_LUTi :: reserved0 [31:14] */
#define BCHP_HDR_CMP_0_V0_L2NL_TF_LUTi_reserved0_MASK              0xffffc000
#define BCHP_HDR_CMP_0_V0_L2NL_TF_LUTi_reserved0_SHIFT             14

/* HDR_CMP_0 :: V0_L2NL_TF_LUTi :: L2_NL_TF_VAL [13:00] */
#define BCHP_HDR_CMP_0_V0_L2NL_TF_LUTi_L2_NL_TF_VAL_MASK           0x00003fff
#define BCHP_HDR_CMP_0_V0_L2NL_TF_LUTi_L2_NL_TF_VAL_SHIFT          0


/***************************************************************************
 *V1_R00_TO_R15_NL_CONFIG%i - Video Surface 1 RECT0 to RECT15 NL_CSC config[0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_ARRAY_BASE         0x2064a4e0
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_ARRAY_START        0
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_ARRAY_END          7
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_ARRAY_ELEMENT_SIZE 32

/***************************************************************************
 *V1_R00_TO_R15_NL_CONFIG%i - Video Surface 1 RECT0 to RECT15 NL_CSC config[0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R00_TO_R15_NL_CONFIGi :: reserved0 [31:29] */
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_reserved0_MASK     0xe0000000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_reserved0_SHIFT    29

/* HDR_CMP_0 :: V1_R00_TO_R15_NL_CONFIGi :: RECT1_SEL_L2NL [28:26] */
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_MASK 0x1c000000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_SHIFT 26
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_709 0
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_1886 1
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_PQ  2
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_BBC 3
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_RAM 4
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_L2NL_BYPASS 5

/* HDR_CMP_0 :: V1_R00_TO_R15_NL_CONFIGi :: RECT1_SEL_LRANGE_ADJ [25:23] */
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_MASK 0x03800000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_SHIFT 23
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_DEFAULT 0x00000007
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_DISABLE 7
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_RESRV0 6
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_USE_R5 5
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_USE_R4 4
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_USE_R3 3
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_USE_R2 2
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_USE_R1 1
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_LRANGE_ADJ_USE_R0 0

/* HDR_CMP_0 :: V1_R00_TO_R15_NL_CONFIGi :: RECT1_SEL_MB_COEF [22:20] */
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_MASK 0x00700000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_SHIFT 20
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_DEFAULT 0x00000007
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_DISABLE 7
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_RESRV0 6
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_USE_R5 5
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_USE_R4 4
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_USE_R3 3
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_USE_R2 2
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_USE_R1 1
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_MB_COEF_USE_R0 0

/* HDR_CMP_0 :: V1_R00_TO_R15_NL_CONFIGi :: RECT1_SEL_NL2L [19:17] */
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_MASK 0x000e0000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_SHIFT 17
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_709 0
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_1886 1
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_PQ  2
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_BBC 3
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_RAM 4
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_SEL_NL2L_BYPASS 5

/* HDR_CMP_0 :: V1_R00_TO_R15_NL_CONFIGi :: RECT1_NL_CSC_EN [16:16] */
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_NL_CSC_EN_MASK 0x00010000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_NL_CSC_EN_SHIFT 16
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_NL_CSC_EN_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_NL_CSC_EN_BYPASS 0
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT1_NL_CSC_EN_ENABLE 1

/* HDR_CMP_0 :: V1_R00_TO_R15_NL_CONFIGi :: reserved1 [15:13] */
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_reserved1_MASK     0x0000e000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_reserved1_SHIFT    13

/* HDR_CMP_0 :: V1_R00_TO_R15_NL_CONFIGi :: RECT0_SEL_L2NL [12:10] */
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_MASK 0x00001c00
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_SHIFT 10
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_709 0
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_1886 1
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_PQ  2
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_BBC 3
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_RAM 4
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_L2NL_BYPASS 5

/* HDR_CMP_0 :: V1_R00_TO_R15_NL_CONFIGi :: RECT0_SEL_LRANGE_ADJ [09:07] */
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_MASK 0x00000380
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_SHIFT 7
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_DEFAULT 0x00000007
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_DISABLE 7
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_RESRV0 6
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_USE_R5 5
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_USE_R4 4
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_USE_R3 3
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_USE_R2 2
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_USE_R1 1
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_LRANGE_ADJ_USE_R0 0

/* HDR_CMP_0 :: V1_R00_TO_R15_NL_CONFIGi :: RECT0_SEL_MB_COEF [06:04] */
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_MASK 0x00000070
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_SHIFT 4
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_DEFAULT 0x00000007
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_DISABLE 7
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_RESRV0 6
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_USE_R5 5
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_USE_R4 4
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_USE_R3 3
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_USE_R2 2
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_USE_R1 1
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_MB_COEF_USE_R0 0

/* HDR_CMP_0 :: V1_R00_TO_R15_NL_CONFIGi :: RECT0_SEL_NL2L [03:01] */
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_MASK 0x0000000e
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_SHIFT 1
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_709 0
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_1886 1
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_PQ  2
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_BBC 3
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_RAM 4
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_SEL_NL2L_BYPASS 5

/* HDR_CMP_0 :: V1_R00_TO_R15_NL_CONFIGi :: RECT0_NL_CSC_EN [00:00] */
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_NL_CSC_EN_MASK 0x00000001
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_NL_CSC_EN_SHIFT 0
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_NL_CSC_EN_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_NL_CSC_EN_BYPASS 0
#define BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_RECT0_NL_CSC_EN_ENABLE 1


/***************************************************************************
 *V1_R0_MA_COEFF_C00 - Video Surface 1 Color Matrix A R0 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MA_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MA_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R0_MA_COEFF_C01 - Video Surface 1 Color Matrix A R0 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MA_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MA_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MA_COEFF_C02 - Video Surface 1 Color Matrix A R0 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MA_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MA_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MA_COEFF_C03 - Video Surface 1 Color Matrix A R0 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MA_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MA_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MA_COEFF_C10 - Video Surface 1 Color Matrix A R0 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MA_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MA_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MA_COEFF_C11 - Video Surface 1 Color Matrix A R0 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MA_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MA_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R0_MA_COEFF_C12 - Video Surface 1 Color Matrix A R0 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MA_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MA_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MA_COEFF_C13 - Video Surface 1 Color Matrix A R0 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MA_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MA_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MA_COEFF_C20 - Video Surface 1 Color Matrix A R0 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MA_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MA_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MA_COEFF_C21 - Video Surface 1 Color Matrix A R0 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MA_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MA_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MA_COEFF_C22 - Video Surface 1 Color Matrix A R0 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MA_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MA_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R0_MA_COEFF_C23 - Video Surface 1 Color Matrix A R0 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MA_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MA_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MA_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MA_COEFF_C00 - Video Surface 1 Color Matrix A R1 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MA_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MA_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R1_MA_COEFF_C01 - Video Surface 1 Color Matrix A R1 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MA_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MA_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MA_COEFF_C02 - Video Surface 1 Color Matrix A R1 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MA_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MA_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MA_COEFF_C03 - Video Surface 1 Color Matrix A R1 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MA_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MA_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MA_COEFF_C10 - Video Surface 1 Color Matrix A R1 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MA_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MA_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MA_COEFF_C11 - Video Surface 1 Color Matrix A R1 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MA_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MA_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R1_MA_COEFF_C12 - Video Surface 1 Color Matrix A R1 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MA_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MA_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MA_COEFF_C13 - Video Surface 1 Color Matrix A R1 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MA_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MA_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MA_COEFF_C20 - Video Surface 1 Color Matrix A R1 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MA_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MA_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MA_COEFF_C21 - Video Surface 1 Color Matrix A R1 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MA_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MA_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MA_COEFF_C22 - Video Surface 1 Color Matrix A R1 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MA_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MA_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R1_MA_COEFF_C23 - Video Surface 1 Color Matrix A R1 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MA_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MA_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MA_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MA_COEFF_C00 - Video Surface 1 Color Matrix A R2 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MA_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MA_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R2_MA_COEFF_C01 - Video Surface 1 Color Matrix A R2 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MA_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MA_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MA_COEFF_C02 - Video Surface 1 Color Matrix A R2 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MA_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MA_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MA_COEFF_C03 - Video Surface 1 Color Matrix A R2 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MA_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MA_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MA_COEFF_C10 - Video Surface 1 Color Matrix A R2 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MA_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MA_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MA_COEFF_C11 - Video Surface 1 Color Matrix A R2 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MA_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MA_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R2_MA_COEFF_C12 - Video Surface 1 Color Matrix A R2 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MA_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MA_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MA_COEFF_C13 - Video Surface 1 Color Matrix A R2 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MA_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MA_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MA_COEFF_C20 - Video Surface 1 Color Matrix A R2 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MA_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MA_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MA_COEFF_C21 - Video Surface 1 Color Matrix A R2 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MA_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MA_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MA_COEFF_C22 - Video Surface 1 Color Matrix A R2 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MA_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MA_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R2_MA_COEFF_C23 - Video Surface 1 Color Matrix A R2 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MA_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MA_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MA_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MA_COEFF_C00 - Video Surface 1 Color Matrix A R3 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MA_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MA_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R3_MA_COEFF_C01 - Video Surface 1 Color Matrix A R3 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MA_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MA_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MA_COEFF_C02 - Video Surface 1 Color Matrix A R3 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MA_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MA_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MA_COEFF_C03 - Video Surface 1 Color Matrix A R3 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MA_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MA_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MA_COEFF_C10 - Video Surface 1 Color Matrix A R3 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MA_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MA_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MA_COEFF_C11 - Video Surface 1 Color Matrix A R3 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MA_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MA_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R3_MA_COEFF_C12 - Video Surface 1 Color Matrix A R3 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MA_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MA_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MA_COEFF_C13 - Video Surface 1 Color Matrix A R3 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MA_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MA_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MA_COEFF_C20 - Video Surface 1 Color Matrix A R3 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MA_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MA_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MA_COEFF_C21 - Video Surface 1 Color Matrix A R3 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MA_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MA_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MA_COEFF_C22 - Video Surface 1 Color Matrix A R3 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MA_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MA_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R3_MA_COEFF_C23 - Video Surface 1 Color Matrix A R3 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MA_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MA_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MA_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MA_COEFF_C00 - Video Surface 1 Color Matrix A R4 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MA_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MA_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R4_MA_COEFF_C01 - Video Surface 1 Color Matrix A R4 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MA_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MA_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MA_COEFF_C02 - Video Surface 1 Color Matrix A R4 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MA_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MA_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MA_COEFF_C03 - Video Surface 1 Color Matrix A R4 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MA_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MA_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MA_COEFF_C10 - Video Surface 1 Color Matrix A R4 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MA_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MA_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MA_COEFF_C11 - Video Surface 1 Color Matrix A R4 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MA_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MA_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R4_MA_COEFF_C12 - Video Surface 1 Color Matrix A R4 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MA_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MA_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MA_COEFF_C13 - Video Surface 1 Color Matrix A R4 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MA_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MA_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MA_COEFF_C20 - Video Surface 1 Color Matrix A R4 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MA_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MA_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MA_COEFF_C21 - Video Surface 1 Color Matrix A R4 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MA_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MA_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MA_COEFF_C22 - Video Surface 1 Color Matrix A R4 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MA_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MA_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R4_MA_COEFF_C23 - Video Surface 1 Color Matrix A R4 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MA_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MA_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MA_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MA_COEFF_C00 - Video Surface 1 Color Matrix A R5 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MA_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MA_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R5_MA_COEFF_C01 - Video Surface 1 Color Matrix A R5 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MA_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MA_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MA_COEFF_C02 - Video Surface 1 Color Matrix A R5 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MA_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MA_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MA_COEFF_C03 - Video Surface 1 Color Matrix A R5 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MA_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MA_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MA_COEFF_C10 - Video Surface 1 Color Matrix A R5 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MA_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MA_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MA_COEFF_C11 - Video Surface 1 Color Matrix A R5 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MA_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MA_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R5_MA_COEFF_C12 - Video Surface 1 Color Matrix A R5 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MA_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MA_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MA_COEFF_C13 - Video Surface 1 Color Matrix A R5 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MA_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MA_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MA_COEFF_C20 - Video Surface 1 Color Matrix A R5 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MA_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MA_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MA_COEFF_C21 - Video Surface 1 Color Matrix A R5 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MA_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MA_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MA_COEFF_C22 - Video Surface 1 Color Matrix A R5 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MA_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MA_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R5_MA_COEFF_C23 - Video Surface 1 Color Matrix A R5 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MA_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MA_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MA_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MB_COEFF_C00 - Video Surface 1 Color Matrix B R0 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MB_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MB_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R0_MB_COEFF_C01 - Video Surface 1 Color Matrix B R0 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MB_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MB_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MB_COEFF_C02 - Video Surface 1 Color Matrix B R0 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MB_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MB_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MB_COEFF_C03 - Video Surface 1 Color Matrix B R0 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MB_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MB_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MB_COEFF_C10 - Video Surface 1 Color Matrix B R0 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MB_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MB_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MB_COEFF_C11 - Video Surface 1 Color Matrix B R0 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MB_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MB_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R0_MB_COEFF_C12 - Video Surface 1 Color Matrix B R0 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MB_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MB_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MB_COEFF_C13 - Video Surface 1 Color Matrix B R0 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MB_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MB_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MB_COEFF_C20 - Video Surface 1 Color Matrix B R0 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MB_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MB_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MB_COEFF_C21 - Video Surface 1 Color Matrix B R0 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MB_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MB_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_MB_COEFF_C22 - Video Surface 1 Color Matrix B R0 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MB_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MB_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R0_MB_COEFF_C23 - Video Surface 1 Color Matrix B R0 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_MB_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R0_MB_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R0_MB_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MB_COEFF_C00 - Video Surface 1 Color Matrix B R1 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MB_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MB_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R1_MB_COEFF_C01 - Video Surface 1 Color Matrix B R1 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MB_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MB_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MB_COEFF_C02 - Video Surface 1 Color Matrix B R1 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MB_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MB_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MB_COEFF_C03 - Video Surface 1 Color Matrix B R1 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MB_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MB_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MB_COEFF_C10 - Video Surface 1 Color Matrix B R1 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MB_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MB_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MB_COEFF_C11 - Video Surface 1 Color Matrix B R1 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MB_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MB_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R1_MB_COEFF_C12 - Video Surface 1 Color Matrix B R1 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MB_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MB_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MB_COEFF_C13 - Video Surface 1 Color Matrix B R1 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MB_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MB_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MB_COEFF_C20 - Video Surface 1 Color Matrix B R1 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MB_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MB_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MB_COEFF_C21 - Video Surface 1 Color Matrix B R1 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MB_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MB_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R1_MB_COEFF_C22 - Video Surface 1 Color Matrix B R1 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MB_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MB_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R1_MB_COEFF_C23 - Video Surface 1 Color Matrix B R1 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_MB_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R1_MB_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R1_MB_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MB_COEFF_C00 - Video Surface 1 Color Matrix B R2 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MB_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MB_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R2_MB_COEFF_C01 - Video Surface 1 Color Matrix B R2 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MB_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MB_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MB_COEFF_C02 - Video Surface 1 Color Matrix B R2 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MB_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MB_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MB_COEFF_C03 - Video Surface 1 Color Matrix B R2 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MB_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MB_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MB_COEFF_C10 - Video Surface 1 Color Matrix B R2 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MB_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MB_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MB_COEFF_C11 - Video Surface 1 Color Matrix B R2 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MB_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MB_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R2_MB_COEFF_C12 - Video Surface 1 Color Matrix B R2 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MB_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MB_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MB_COEFF_C13 - Video Surface 1 Color Matrix B R2 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MB_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MB_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MB_COEFF_C20 - Video Surface 1 Color Matrix B R2 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MB_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MB_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MB_COEFF_C21 - Video Surface 1 Color Matrix B R2 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MB_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MB_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R2_MB_COEFF_C22 - Video Surface 1 Color Matrix B R2 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MB_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MB_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R2_MB_COEFF_C23 - Video Surface 1 Color Matrix B R2 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_MB_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R2_MB_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R2_MB_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MB_COEFF_C00 - Video Surface 1 Color Matrix B R3 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MB_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MB_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R3_MB_COEFF_C01 - Video Surface 1 Color Matrix B R3 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MB_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MB_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MB_COEFF_C02 - Video Surface 1 Color Matrix B R3 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MB_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MB_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MB_COEFF_C03 - Video Surface 1 Color Matrix B R3 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MB_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MB_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MB_COEFF_C10 - Video Surface 1 Color Matrix B R3 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MB_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MB_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MB_COEFF_C11 - Video Surface 1 Color Matrix B R3 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MB_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MB_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R3_MB_COEFF_C12 - Video Surface 1 Color Matrix B R3 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MB_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MB_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MB_COEFF_C13 - Video Surface 1 Color Matrix B R3 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MB_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MB_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MB_COEFF_C20 - Video Surface 1 Color Matrix B R3 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MB_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MB_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MB_COEFF_C21 - Video Surface 1 Color Matrix B R3 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MB_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MB_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R3_MB_COEFF_C22 - Video Surface 1 Color Matrix B R3 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MB_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MB_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R3_MB_COEFF_C23 - Video Surface 1 Color Matrix B R3 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_MB_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R3_MB_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R3_MB_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MB_COEFF_C00 - Video Surface 1 Color Matrix B R4 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MB_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MB_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R4_MB_COEFF_C01 - Video Surface 1 Color Matrix B R4 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MB_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MB_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MB_COEFF_C02 - Video Surface 1 Color Matrix B R4 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MB_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MB_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MB_COEFF_C03 - Video Surface 1 Color Matrix B R4 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MB_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MB_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MB_COEFF_C10 - Video Surface 1 Color Matrix B R4 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MB_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MB_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MB_COEFF_C11 - Video Surface 1 Color Matrix B R4 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MB_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MB_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R4_MB_COEFF_C12 - Video Surface 1 Color Matrix B R4 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MB_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MB_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MB_COEFF_C13 - Video Surface 1 Color Matrix B R4 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MB_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MB_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MB_COEFF_C20 - Video Surface 1 Color Matrix B R4 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MB_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MB_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MB_COEFF_C21 - Video Surface 1 Color Matrix B R4 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MB_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MB_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R4_MB_COEFF_C22 - Video Surface 1 Color Matrix B R4 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MB_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MB_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R4_MB_COEFF_C23 - Video Surface 1 Color Matrix B R4 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_MB_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R4_MB_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R4_MB_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MB_COEFF_C00 - Video Surface 1 Color Matrix B R5 coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MB_COEFF_C00 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C00_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C00_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MB_COEFF_C00 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C00_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C00_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C00_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R5_MB_COEFF_C01 - Video Surface 1 Color Matrix B R5 coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MB_COEFF_C01 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C01_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C01_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MB_COEFF_C01 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C01_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C01_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C01_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MB_COEFF_C02 - Video Surface 1 Color Matrix B R5 coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MB_COEFF_C02 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C02_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C02_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MB_COEFF_C02 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C02_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C02_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C02_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MB_COEFF_C03 - Video Surface 1 Color Matrix B R5 coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MB_COEFF_C03 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C03_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C03_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MB_COEFF_C03 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C03_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C03_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C03_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MB_COEFF_C10 - Video Surface 1 Color Matrix B R5 coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MB_COEFF_C10 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C10_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C10_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MB_COEFF_C10 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C10_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C10_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C10_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MB_COEFF_C11 - Video Surface 1 Color Matrix B R5 coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MB_COEFF_C11 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C11_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C11_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MB_COEFF_C11 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C11_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C11_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C11_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R5_MB_COEFF_C12 - Video Surface 1 Color Matrix B R5 coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MB_COEFF_C12 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C12_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C12_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MB_COEFF_C12 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C12_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C12_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C12_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MB_COEFF_C13 - Video Surface 1 Color Matrix B R5 coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MB_COEFF_C13 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C13_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C13_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MB_COEFF_C13 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C13_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C13_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C13_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MB_COEFF_C20 - Video Surface 1 Color Matrix B R5 coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MB_COEFF_C20 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C20_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C20_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MB_COEFF_C20 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C20_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C20_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C20_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MB_COEFF_C21 - Video Surface 1 Color Matrix B R5 coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MB_COEFF_C21 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C21_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C21_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MB_COEFF_C21 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C21_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C21_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C21_COEFF_MUL_DEFAULT        0x00000000

/***************************************************************************
 *V1_R5_MB_COEFF_C22 - Video Surface 1 Color Matrix B R5 coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MB_COEFF_C22 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C22_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C22_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MB_COEFF_C22 :: COEFF_MUL [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C22_COEFF_MUL_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C22_COEFF_MUL_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C22_COEFF_MUL_DEFAULT        0x00400000

/***************************************************************************
 *V1_R5_MB_COEFF_C23 - Video Surface 1 Color Matrix B R5 coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_MB_COEFF_C23 :: reserved0 [31:25] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C23_reserved0_MASK           0xfe000000
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C23_reserved0_SHIFT          25

/* HDR_CMP_0 :: V1_R5_MB_COEFF_C23 :: COEFF_ADD [24:00] */
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C23_COEFF_ADD_MASK           0x01ffffff
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C23_COEFF_ADD_SHIFT          0
#define BCHP_HDR_CMP_0_V1_R5_MB_COEFF_C23_COEFF_ADD_DEFAULT        0x00000000

/***************************************************************************
 *V1_R0_NL_LR_SLOPE%i - Video Surface 1 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_ARRAY_BASE               0x2064a880
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_ARRAY_START              0
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_ARRAY_END                7
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *V1_R0_NL_LR_SLOPE%i - Video Surface 1 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_NL_LR_SLOPEi :: SLOPE_M [31:16] */
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_SLOPE_M_MASK             0xffff0000
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_SLOPE_M_SHIFT            16
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_SLOPE_M_DEFAULT          0x00000000

/* HDR_CMP_0 :: V1_R0_NL_LR_SLOPEi :: reserved0 [15:05] */
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_reserved0_MASK           0x0000ffe0
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_reserved0_SHIFT          5

/* HDR_CMP_0 :: V1_R0_NL_LR_SLOPEi :: SLOPE_E [04:00] */
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_SLOPE_E_MASK             0x0000001f
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_SLOPE_E_SHIFT            0
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_SLOPE_E_DEFAULT          0x00000000


/***************************************************************************
 *V1_R0_NL_LR_XY_0%i - Video Surface 1 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_XY_0i_ARRAY_BASE                0x2064a8a0
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_XY_0i_ARRAY_START               0
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_XY_0i_ARRAY_END                 7
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_XY_0i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *V1_R0_NL_LR_XY_0%i - Video Surface 1 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_NL_LR_XY_0i :: LRA_Y [31:16] */
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_XY_0i_LRA_Y_MASK                0xffff0000
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_XY_0i_LRA_Y_SHIFT               16
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_XY_0i_LRA_Y_DEFAULT             0x00000000

/* HDR_CMP_0 :: V1_R0_NL_LR_XY_0i :: LRA_X [15:00] */
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_XY_0i_LRA_X_MASK                0x0000ffff
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_XY_0i_LRA_X_SHIFT               0
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_XY_0i_LRA_X_DEFAULT             0x00008000


/***************************************************************************
 *V1_R0_NL_CSC_CTRL - Video Surface 1 Nonconstant Luminance CSC Control
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R0_NL_CSC_CTRL :: reserved0 [31:03] */
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL_reserved0_MASK            0xfffffff8
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL_reserved0_SHIFT           3

/* HDR_CMP_0 :: V1_R0_NL_CSC_CTRL :: SEL_CL_IN [02:02] */
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL_SEL_CL_IN_MASK            0x00000004
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL_SEL_CL_IN_SHIFT           2
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL_SEL_CL_IN_DEFAULT         0x00000000

/* HDR_CMP_0 :: V1_R0_NL_CSC_CTRL :: SEL_XVYCC [01:01] */
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL_SEL_XVYCC_MASK            0x00000002
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL_SEL_XVYCC_SHIFT           1
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL_SEL_XVYCC_DEFAULT         0x00000000
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL_SEL_XVYCC_NOT_XVYCC       0
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL_SEL_XVYCC_XVYCC           1

/* HDR_CMP_0 :: V1_R0_NL_CSC_CTRL :: reserved1 [00:00] */
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL_reserved1_MASK            0x00000001
#define BCHP_HDR_CMP_0_V1_R0_NL_CSC_CTRL_reserved1_SHIFT           0

/***************************************************************************
 *V1_R1_NL_LR_SLOPE%i - Video Surface 1 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_SLOPEi_ARRAY_BASE               0x2064a8d0
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_SLOPEi_ARRAY_START              0
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_SLOPEi_ARRAY_END                7
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_SLOPEi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *V1_R1_NL_LR_SLOPE%i - Video Surface 1 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_NL_LR_SLOPEi :: SLOPE_M [31:16] */
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_SLOPEi_SLOPE_M_MASK             0xffff0000
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_SLOPEi_SLOPE_M_SHIFT            16
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_SLOPEi_SLOPE_M_DEFAULT          0x00000000

/* HDR_CMP_0 :: V1_R1_NL_LR_SLOPEi :: reserved0 [15:05] */
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_SLOPEi_reserved0_MASK           0x0000ffe0
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_SLOPEi_reserved0_SHIFT          5

/* HDR_CMP_0 :: V1_R1_NL_LR_SLOPEi :: SLOPE_E [04:00] */
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_SLOPEi_SLOPE_E_MASK             0x0000001f
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_SLOPEi_SLOPE_E_SHIFT            0
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_SLOPEi_SLOPE_E_DEFAULT          0x00000000


/***************************************************************************
 *V1_R1_NL_LR_XY_0%i - Video Surface 1 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_XY_0i_ARRAY_BASE                0x2064a8f0
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_XY_0i_ARRAY_START               0
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_XY_0i_ARRAY_END                 7
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_XY_0i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *V1_R1_NL_LR_XY_0%i - Video Surface 1 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_NL_LR_XY_0i :: LRA_Y [31:16] */
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_XY_0i_LRA_Y_MASK                0xffff0000
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_XY_0i_LRA_Y_SHIFT               16
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_XY_0i_LRA_Y_DEFAULT             0x00000000

/* HDR_CMP_0 :: V1_R1_NL_LR_XY_0i :: LRA_X [15:00] */
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_XY_0i_LRA_X_MASK                0x0000ffff
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_XY_0i_LRA_X_SHIFT               0
#define BCHP_HDR_CMP_0_V1_R1_NL_LR_XY_0i_LRA_X_DEFAULT             0x00008000


/***************************************************************************
 *V1_R1_NL_CSC_CTRL - Video Surface 1 Nonconstant Luminance CSC Control
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R1_NL_CSC_CTRL :: reserved0 [31:03] */
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL_reserved0_MASK            0xfffffff8
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL_reserved0_SHIFT           3

/* HDR_CMP_0 :: V1_R1_NL_CSC_CTRL :: SEL_CL_IN [02:02] */
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL_SEL_CL_IN_MASK            0x00000004
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL_SEL_CL_IN_SHIFT           2
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL_SEL_CL_IN_DEFAULT         0x00000000

/* HDR_CMP_0 :: V1_R1_NL_CSC_CTRL :: SEL_XVYCC [01:01] */
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL_SEL_XVYCC_MASK            0x00000002
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL_SEL_XVYCC_SHIFT           1
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL_SEL_XVYCC_DEFAULT         0x00000000
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL_SEL_XVYCC_NOT_XVYCC       0
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL_SEL_XVYCC_XVYCC           1

/* HDR_CMP_0 :: V1_R1_NL_CSC_CTRL :: reserved1 [00:00] */
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL_reserved1_MASK            0x00000001
#define BCHP_HDR_CMP_0_V1_R1_NL_CSC_CTRL_reserved1_SHIFT           0

/***************************************************************************
 *V1_R2_NL_LR_SLOPE%i - Video Surface 1 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_SLOPEi_ARRAY_BASE               0x2064a920
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_SLOPEi_ARRAY_START              0
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_SLOPEi_ARRAY_END                7
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_SLOPEi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *V1_R2_NL_LR_SLOPE%i - Video Surface 1 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_NL_LR_SLOPEi :: SLOPE_M [31:16] */
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_SLOPEi_SLOPE_M_MASK             0xffff0000
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_SLOPEi_SLOPE_M_SHIFT            16
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_SLOPEi_SLOPE_M_DEFAULT          0x00000000

/* HDR_CMP_0 :: V1_R2_NL_LR_SLOPEi :: reserved0 [15:05] */
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_SLOPEi_reserved0_MASK           0x0000ffe0
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_SLOPEi_reserved0_SHIFT          5

/* HDR_CMP_0 :: V1_R2_NL_LR_SLOPEi :: SLOPE_E [04:00] */
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_SLOPEi_SLOPE_E_MASK             0x0000001f
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_SLOPEi_SLOPE_E_SHIFT            0
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_SLOPEi_SLOPE_E_DEFAULT          0x00000000


/***************************************************************************
 *V1_R2_NL_LR_XY_0%i - Video Surface 1 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_XY_0i_ARRAY_BASE                0x2064a940
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_XY_0i_ARRAY_START               0
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_XY_0i_ARRAY_END                 7
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_XY_0i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *V1_R2_NL_LR_XY_0%i - Video Surface 1 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_NL_LR_XY_0i :: LRA_Y [31:16] */
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_XY_0i_LRA_Y_MASK                0xffff0000
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_XY_0i_LRA_Y_SHIFT               16
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_XY_0i_LRA_Y_DEFAULT             0x00000000

/* HDR_CMP_0 :: V1_R2_NL_LR_XY_0i :: LRA_X [15:00] */
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_XY_0i_LRA_X_MASK                0x0000ffff
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_XY_0i_LRA_X_SHIFT               0
#define BCHP_HDR_CMP_0_V1_R2_NL_LR_XY_0i_LRA_X_DEFAULT             0x00008000


/***************************************************************************
 *V1_R2_NL_CSC_CTRL - Video Surface 1 Nonconstant Luminance CSC Control
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R2_NL_CSC_CTRL :: reserved0 [31:03] */
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL_reserved0_MASK            0xfffffff8
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL_reserved0_SHIFT           3

/* HDR_CMP_0 :: V1_R2_NL_CSC_CTRL :: SEL_CL_IN [02:02] */
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL_SEL_CL_IN_MASK            0x00000004
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL_SEL_CL_IN_SHIFT           2
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL_SEL_CL_IN_DEFAULT         0x00000000

/* HDR_CMP_0 :: V1_R2_NL_CSC_CTRL :: SEL_XVYCC [01:01] */
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL_SEL_XVYCC_MASK            0x00000002
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL_SEL_XVYCC_SHIFT           1
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL_SEL_XVYCC_DEFAULT         0x00000000
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL_SEL_XVYCC_NOT_XVYCC       0
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL_SEL_XVYCC_XVYCC           1

/* HDR_CMP_0 :: V1_R2_NL_CSC_CTRL :: reserved1 [00:00] */
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL_reserved1_MASK            0x00000001
#define BCHP_HDR_CMP_0_V1_R2_NL_CSC_CTRL_reserved1_SHIFT           0

/***************************************************************************
 *V1_R3_NL_LR_SLOPE%i - Video Surface 1 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_SLOPEi_ARRAY_BASE               0x2064a970
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_SLOPEi_ARRAY_START              0
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_SLOPEi_ARRAY_END                7
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_SLOPEi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *V1_R3_NL_LR_SLOPE%i - Video Surface 1 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_NL_LR_SLOPEi :: SLOPE_M [31:16] */
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_SLOPEi_SLOPE_M_MASK             0xffff0000
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_SLOPEi_SLOPE_M_SHIFT            16
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_SLOPEi_SLOPE_M_DEFAULT          0x00000000

/* HDR_CMP_0 :: V1_R3_NL_LR_SLOPEi :: reserved0 [15:05] */
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_SLOPEi_reserved0_MASK           0x0000ffe0
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_SLOPEi_reserved0_SHIFT          5

/* HDR_CMP_0 :: V1_R3_NL_LR_SLOPEi :: SLOPE_E [04:00] */
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_SLOPEi_SLOPE_E_MASK             0x0000001f
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_SLOPEi_SLOPE_E_SHIFT            0
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_SLOPEi_SLOPE_E_DEFAULT          0x00000000


/***************************************************************************
 *V1_R3_NL_LR_XY_0%i - Video Surface 1 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_XY_0i_ARRAY_BASE                0x2064a990
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_XY_0i_ARRAY_START               0
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_XY_0i_ARRAY_END                 7
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_XY_0i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *V1_R3_NL_LR_XY_0%i - Video Surface 1 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_NL_LR_XY_0i :: LRA_Y [31:16] */
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_XY_0i_LRA_Y_MASK                0xffff0000
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_XY_0i_LRA_Y_SHIFT               16
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_XY_0i_LRA_Y_DEFAULT             0x00000000

/* HDR_CMP_0 :: V1_R3_NL_LR_XY_0i :: LRA_X [15:00] */
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_XY_0i_LRA_X_MASK                0x0000ffff
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_XY_0i_LRA_X_SHIFT               0
#define BCHP_HDR_CMP_0_V1_R3_NL_LR_XY_0i_LRA_X_DEFAULT             0x00008000


/***************************************************************************
 *V1_R3_NL_CSC_CTRL - Video Surface 1 Nonconstant Luminance CSC Control
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R3_NL_CSC_CTRL :: reserved0 [31:03] */
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL_reserved0_MASK            0xfffffff8
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL_reserved0_SHIFT           3

/* HDR_CMP_0 :: V1_R3_NL_CSC_CTRL :: SEL_CL_IN [02:02] */
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL_SEL_CL_IN_MASK            0x00000004
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL_SEL_CL_IN_SHIFT           2
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL_SEL_CL_IN_DEFAULT         0x00000000

/* HDR_CMP_0 :: V1_R3_NL_CSC_CTRL :: SEL_XVYCC [01:01] */
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL_SEL_XVYCC_MASK            0x00000002
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL_SEL_XVYCC_SHIFT           1
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL_SEL_XVYCC_DEFAULT         0x00000000
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL_SEL_XVYCC_NOT_XVYCC       0
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL_SEL_XVYCC_XVYCC           1

/* HDR_CMP_0 :: V1_R3_NL_CSC_CTRL :: reserved1 [00:00] */
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL_reserved1_MASK            0x00000001
#define BCHP_HDR_CMP_0_V1_R3_NL_CSC_CTRL_reserved1_SHIFT           0

/***************************************************************************
 *V1_R4_NL_LR_SLOPE%i - Video Surface 1 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_SLOPEi_ARRAY_BASE               0x2064a9c0
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_SLOPEi_ARRAY_START              0
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_SLOPEi_ARRAY_END                7
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_SLOPEi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *V1_R4_NL_LR_SLOPE%i - Video Surface 1 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_NL_LR_SLOPEi :: SLOPE_M [31:16] */
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_SLOPEi_SLOPE_M_MASK             0xffff0000
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_SLOPEi_SLOPE_M_SHIFT            16
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_SLOPEi_SLOPE_M_DEFAULT          0x00000000

/* HDR_CMP_0 :: V1_R4_NL_LR_SLOPEi :: reserved0 [15:05] */
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_SLOPEi_reserved0_MASK           0x0000ffe0
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_SLOPEi_reserved0_SHIFT          5

/* HDR_CMP_0 :: V1_R4_NL_LR_SLOPEi :: SLOPE_E [04:00] */
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_SLOPEi_SLOPE_E_MASK             0x0000001f
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_SLOPEi_SLOPE_E_SHIFT            0
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_SLOPEi_SLOPE_E_DEFAULT          0x00000000


/***************************************************************************
 *V1_R4_NL_LR_XY_0%i - Video Surface 1 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_XY_0i_ARRAY_BASE                0x2064a9e0
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_XY_0i_ARRAY_START               0
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_XY_0i_ARRAY_END                 7
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_XY_0i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *V1_R4_NL_LR_XY_0%i - Video Surface 1 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_NL_LR_XY_0i :: LRA_Y [31:16] */
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_XY_0i_LRA_Y_MASK                0xffff0000
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_XY_0i_LRA_Y_SHIFT               16
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_XY_0i_LRA_Y_DEFAULT             0x00000000

/* HDR_CMP_0 :: V1_R4_NL_LR_XY_0i :: LRA_X [15:00] */
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_XY_0i_LRA_X_MASK                0x0000ffff
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_XY_0i_LRA_X_SHIFT               0
#define BCHP_HDR_CMP_0_V1_R4_NL_LR_XY_0i_LRA_X_DEFAULT             0x00008000


/***************************************************************************
 *V1_R4_NL_CSC_CTRL - Video Surface 1 Nonconstant Luminance CSC Control
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R4_NL_CSC_CTRL :: reserved0 [31:03] */
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL_reserved0_MASK            0xfffffff8
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL_reserved0_SHIFT           3

/* HDR_CMP_0 :: V1_R4_NL_CSC_CTRL :: SEL_CL_IN [02:02] */
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL_SEL_CL_IN_MASK            0x00000004
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL_SEL_CL_IN_SHIFT           2
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL_SEL_CL_IN_DEFAULT         0x00000000

/* HDR_CMP_0 :: V1_R4_NL_CSC_CTRL :: SEL_XVYCC [01:01] */
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL_SEL_XVYCC_MASK            0x00000002
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL_SEL_XVYCC_SHIFT           1
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL_SEL_XVYCC_DEFAULT         0x00000000
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL_SEL_XVYCC_NOT_XVYCC       0
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL_SEL_XVYCC_XVYCC           1

/* HDR_CMP_0 :: V1_R4_NL_CSC_CTRL :: reserved1 [00:00] */
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL_reserved1_MASK            0x00000001
#define BCHP_HDR_CMP_0_V1_R4_NL_CSC_CTRL_reserved1_SHIFT           0

/***************************************************************************
 *V1_R5_NL_LR_SLOPE%i - Video Surface 1 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_SLOPEi_ARRAY_BASE               0x2064aa20
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_SLOPEi_ARRAY_START              0
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_SLOPEi_ARRAY_END                7
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_SLOPEi_ARRAY_ELEMENT_SIZE       32

/***************************************************************************
 *V1_R5_NL_LR_SLOPE%i - Video Surface 1 NL_CSC Luminance Range Adjustment Slope [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_NL_LR_SLOPEi :: SLOPE_M [31:16] */
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_SLOPEi_SLOPE_M_MASK             0xffff0000
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_SLOPEi_SLOPE_M_SHIFT            16
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_SLOPEi_SLOPE_M_DEFAULT          0x00000000

/* HDR_CMP_0 :: V1_R5_NL_LR_SLOPEi :: reserved0 [15:05] */
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_SLOPEi_reserved0_MASK           0x0000ffe0
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_SLOPEi_reserved0_SHIFT          5

/* HDR_CMP_0 :: V1_R5_NL_LR_SLOPEi :: SLOPE_E [04:00] */
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_SLOPEi_SLOPE_E_MASK             0x0000001f
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_SLOPEi_SLOPE_E_SHIFT            0
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_SLOPEi_SLOPE_E_DEFAULT          0x00000000


/***************************************************************************
 *V1_R5_NL_LR_XY_0%i - Video Surface 1 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_XY_0i_ARRAY_BASE                0x2064aa40
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_XY_0i_ARRAY_START               0
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_XY_0i_ARRAY_END                 7
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_XY_0i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *V1_R5_NL_LR_XY_0%i - Video Surface 1 NL_CSC Luminance Range Adjustment XY [0..7]
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_NL_LR_XY_0i :: LRA_Y [31:16] */
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_XY_0i_LRA_Y_MASK                0xffff0000
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_XY_0i_LRA_Y_SHIFT               16
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_XY_0i_LRA_Y_DEFAULT             0x00000000

/* HDR_CMP_0 :: V1_R5_NL_LR_XY_0i :: LRA_X [15:00] */
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_XY_0i_LRA_X_MASK                0x0000ffff
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_XY_0i_LRA_X_SHIFT               0
#define BCHP_HDR_CMP_0_V1_R5_NL_LR_XY_0i_LRA_X_DEFAULT             0x00008000


/***************************************************************************
 *V1_R5_NL_CSC_CTRL - Video Surface 1 Nonconstant Luminance CSC Control
 ***************************************************************************/
/* HDR_CMP_0 :: V1_R5_NL_CSC_CTRL :: reserved0 [31:03] */
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL_reserved0_MASK            0xfffffff8
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL_reserved0_SHIFT           3

/* HDR_CMP_0 :: V1_R5_NL_CSC_CTRL :: SEL_CL_IN [02:02] */
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL_SEL_CL_IN_MASK            0x00000004
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL_SEL_CL_IN_SHIFT           2
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL_SEL_CL_IN_DEFAULT         0x00000000

/* HDR_CMP_0 :: V1_R5_NL_CSC_CTRL :: SEL_XVYCC [01:01] */
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL_SEL_XVYCC_MASK            0x00000002
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL_SEL_XVYCC_SHIFT           1
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL_SEL_XVYCC_DEFAULT         0x00000000
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL_SEL_XVYCC_NOT_XVYCC       0
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL_SEL_XVYCC_XVYCC           1

/* HDR_CMP_0 :: V1_R5_NL_CSC_CTRL :: reserved1 [00:00] */
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL_reserved1_MASK            0x00000001
#define BCHP_HDR_CMP_0_V1_R5_NL_CSC_CTRL_reserved1_SHIFT           0

/***************************************************************************
 *V1_BLENDER_IN_CSC_EN - Video Surface 1 Blender In Color Matrix Enable
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_CSC_EN :: reserved0 [31:01] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_CSC_EN_reserved0_MASK         0xfffffffe
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_CSC_EN_reserved0_SHIFT        1

/* HDR_CMP_0 :: V1_BLENDER_IN_CSC_EN :: BLENDER_IN_CSC_ENABLE [00:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_CSC_EN_BLENDER_IN_CSC_ENABLE_MASK 0x00000001
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_CSC_EN_BLENDER_IN_CSC_ENABLE_SHIFT 0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_CSC_EN_BLENDER_IN_CSC_ENABLE_DEFAULT 0x00000000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_CSC_EN_BLENDER_IN_CSC_ENABLE_DISABLE 0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_CSC_EN_BLENDER_IN_CSC_ENABLE_ENABLE 1

/***************************************************************************
 *V1_BLENDER_IN_COEFF_C00 - Video Surface 1 Blender In Color Matrix coefficient c00
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C00 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C00_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C00_reserved0_SHIFT     16

/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C00 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C00_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C00_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C00_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V1_BLENDER_IN_COEFF_C01 - Video Surface 1 Blender In Color Matrix coefficient c01
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C01 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C01_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C01_reserved0_SHIFT     16

/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C01 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C01_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C01_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C01_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V1_BLENDER_IN_COEFF_C02 - Video Surface 1 Blender In Color Matrix coefficient c02
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C02 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C02_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C02_reserved0_SHIFT     16

/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C02 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C02_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C02_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C02_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V1_BLENDER_IN_COEFF_C03 - Video Surface 1 Blender In Color Matrix coefficient c03
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C03 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C03_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C03_reserved0_SHIFT     16

/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C03 :: COEFF_ADD [15:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C03_COEFF_ADD_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C03_COEFF_ADD_SHIFT     0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C03_COEFF_ADD_DEFAULT   0x00000000

/***************************************************************************
 *V1_BLENDER_IN_COEFF_C10 - Video Surface 1 Blender In Color Matrix coefficient c10
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C10 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C10_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C10_reserved0_SHIFT     16

/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C10 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C10_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C10_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C10_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V1_BLENDER_IN_COEFF_C11 - Video Surface 1 Blender In Color Matrix coefficient c11
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C11 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C11_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C11_reserved0_SHIFT     16

/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C11 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C11_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C11_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C11_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V1_BLENDER_IN_COEFF_C12 - Video Surface 1 Blender In Color Matrix coefficient c12
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C12 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C12_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C12_reserved0_SHIFT     16

/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C12 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C12_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C12_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C12_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V1_BLENDER_IN_COEFF_C13 - Video Surface 1 Blender In Color Matrix coefficient c13
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C13 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C13_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C13_reserved0_SHIFT     16

/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C13 :: COEFF_ADD [15:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C13_COEFF_ADD_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C13_COEFF_ADD_SHIFT     0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C13_COEFF_ADD_DEFAULT   0x00000000

/***************************************************************************
 *V1_BLENDER_IN_COEFF_C20 - Video Surface 1 Blender In Color Matrix coefficient c20
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C20 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C20_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C20_reserved0_SHIFT     16

/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C20 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C20_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C20_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C20_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V1_BLENDER_IN_COEFF_C21 - Video Surface 1 Blender In Color Matrix coefficient c21
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C21 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C21_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C21_reserved0_SHIFT     16

/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C21 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C21_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C21_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C21_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V1_BLENDER_IN_COEFF_C22 - Video Surface 1 Blender In Color Matrix coefficient c22
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C22 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C22_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C22_reserved0_SHIFT     16

/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C22 :: COEFF_MUL [15:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C22_COEFF_MUL_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C22_COEFF_MUL_SHIFT     0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C22_COEFF_MUL_DEFAULT   0x00000000

/***************************************************************************
 *V1_BLENDER_IN_COEFF_C23 - Video Surface 1 Blender In Color Matrix coefficient c23
 ***************************************************************************/
/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C23 :: reserved0 [31:16] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C23_reserved0_MASK      0xffff0000
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C23_reserved0_SHIFT     16

/* HDR_CMP_0 :: V1_BLENDER_IN_COEFF_C23 :: COEFF_ADD [15:00] */
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C23_COEFF_ADD_MASK      0x0000ffff
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C23_COEFF_ADD_SHIFT     0
#define BCHP_HDR_CMP_0_V1_BLENDER_IN_COEFF_C23_COEFF_ADD_DEFAULT   0x00000000

#endif /* #ifndef BCHP_HDR_CMP_0_H__ */

/* End of File */
