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
 * Date:           Generated on               Mon Mar 21 13:44:45 2016
 *                 Full Compile MD5 Checksum  48e7e549bb13082ab30187cb156f35ed
 *                     (minus title and desc)
 *                 MD5 Checksum               949df837b98c31b52074d06d129f7b79
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     880
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_CMP_2_H__
#define BCHP_CMP_2_H__

/***************************************************************************
 *CMP_2 - Video Compositor 2 Registers
 ***************************************************************************/
#define BCHP_CMP_2_REVISION                      0x00047000 /* [RO] Compositor Revision ID */
#define BCHP_CMP_2_HW_CONFIGURATION              0x00047004 /* [RO] Compositor HW Configuration */
#define BCHP_CMP_2_CANVAS_CTRL                   0x00047008 /* [XRW] Canvas control */
#define BCHP_CMP_2_CANVAS_SIZE                   0x0004700c /* [RW] Canvas Vertical and Horizontal Size */
#define BCHP_CMP_2_BG_COLOR                      0x00047010 /* [RW] Background color register */
#define BCHP_CMP_2_BLEND_0_CTRL                  0x00047014 /* [RW] Blending Control for First stage Blender */
#define BCHP_CMP_2_BLEND_1_CTRL                  0x00047018 /* [RW] Blending Control for Second stage Blender */
#define BCHP_CMP_2_CMP_CTRL                      0x00047028 /* [RW] Compositor Control */
#define BCHP_CMP_2_CMP_OUT_CTRL                  0x00047040 /* [RW] Compositor Output Control */
#define BCHP_CMP_2_CRC_CTRL                      0x00047044 /* [RW] BVN CRC Control Register */
#define BCHP_CMP_2_CRC_Y_STATUS                  0x00047048 /* [RO] BVN CRC Luma Status Register */
#define BCHP_CMP_2_CRC_CB_STATUS                 0x0004704c /* [RO] BVN CRC Chroma(Cb) Status Register */
#define BCHP_CMP_2_CRC_CR_STATUS                 0x00047050 /* [RO] BVN CRC Chroma(Cr) Status Register */
#define BCHP_CMP_2_READBACK_POSITION             0x00047054 /* [RW] Compositor Position For CMP Output Readback */
#define BCHP_CMP_2_READBACK_VALUE                0x00047058 /* [RO] Compositor Readback Value at Specified Position */
#define BCHP_CMP_2_CSC_DEMO_SETTING              0x0004705c /* [RW] Compositor Color Space Converter Demo Setting */
#define BCHP_CMP_2_SCRATCH_REGISTER              0x00047060 /* [RW] Compositor Scratch Register */
#define BCHP_CMP_2_TEST_REGISTER                 0x00047068 /* [RW] Compositor Test Register */
#define BCHP_CMP_2_TEST_REGISTER1                0x0004706c /* [RO] Compositor Test Register1 */
#define BCHP_CMP_2_CMP_STATUS_CLEAR              0x0004707c /* [WO] CMP Status Clear */
#define BCHP_CMP_2_CMP_STATUS                    0x00047080 /* [RO] CMP Status */
#define BCHP_CMP_2_G0_SURFACE_SIZE               0x00047180 /* [RW] Graphics Surface 0 Vertical and Horizontal Size */
#define BCHP_CMP_2_G0_SURFACE_OFFSET             0x00047184 /* [RW] Graphics Surface 0 Vertical and Horizontal Offset */
#define BCHP_CMP_2_G0_DISPLAY_SIZE               0x00047188 /* [RW] Graphics Surface 0 Display Vertical and Horizontal Size1 */
#define BCHP_CMP_2_G0_CANVAS_OFFSET              0x0004718c /* [RW] Graphics Surface 0 Canvas Vertical and Horizontal Offset */
#define BCHP_CMP_2_G0_CANVAS_X_OFFSET_R          0x00047190 /* [RW] Graphics Surface 0 Canvas Horizontal Offset For Right or Under Window in 3D mode */
#define BCHP_CMP_2_G0_SURFACE_CTRL               0x00047194 /* [RW] Graphics Surface 0 Control */
#define BCHP_CMP_2_G0_BVB_IN_STATUS_CLEAR        0x00047198 /* [WO] Graphics Surface 0 BVB Input Status Clear */
#define BCHP_CMP_2_G0_BVB_IN_STATUS              0x0004719c /* [RO] Graphics Surface 0 BVB Input Status */
#define BCHP_CMP_2_V0_SURFACE_SIZE               0x00047200 /* [RW] Video Surface 0 Vertical and Horizontal Size */
#define BCHP_CMP_2_V0_SURFACE_OFFSET             0x00047204 /* [RW] Video Surface 0 Vertical and Horizontal Offset */
#define BCHP_CMP_2_V0_DISPLAY_SIZE               0x00047208 /* [RW] Video Surface 0 Display Vertical and Horizontal Size */
#define BCHP_CMP_2_V0_CANVAS_OFFSET              0x0004720c /* [RW] Video Surface 0 Canvas Vertical and Horizontal Offset */
#define BCHP_CMP_2_V0_CANVAS_X_OFFSET_R          0x00047210 /* [RW] Video Surface 0 Canvas Horizontal Offset For Right or Under Window in 3D mode */
#define BCHP_CMP_2_V0_SURFACE_CTRL               0x00047214 /* [RW] Video Surface 0 Control */
#define BCHP_CMP_2_V0_BVB_IN_STATUS_CLEAR        0x00047218 /* [WO] Video Surface 0 BVB Input Status Clear */
#define BCHP_CMP_2_V0_BVB_IN_STATUS              0x0004721c /* [RO] Video Surface 0 BVB Input Status */
#define BCHP_CMP_2_V0_CONST_COLOR                0x00047220 /* [RW] Video Surface 0 Constant Color Register */
#define BCHP_CMP_2_V0_CB_KEYING                  0x00047224 /* [RW] Video Surface 0 Chroma (Blue) Key */
#define BCHP_CMP_2_V0_CR_KEYING                  0x00047228 /* [RW] Video Surface 0 Chroma (Red) Key */
#define BCHP_CMP_2_V0_LUMA_KEYING                0x0004722c /* [RW] Video Surface 0 Luma Key */
#define BCHP_CMP_2_V0_RECT_CSC_INDEX_0           0x00047230 /* [RW] Video Surface 0 rectangle CSC index 0 */
#define BCHP_CMP_2_V0_RECT_CSC_INDEX_1           0x00047234 /* [RW] Video Surface 0 rectangle CSC index 1 */
#define BCHP_CMP_2_V0_RECT_COLOR                 0x00047238 /* [RW] Video Surface 0 Rectangle Function Color Register */
#define BCHP_CMP_2_V0_RECT_TOP_CTRL              0x0004723c /* [RW] Video Surface 0 Top Level Rectangle Control for Mosaic or See Through. */
#define BCHP_CMP_2_V0_RECT_ENABLE_MASK           0x00047240 /* [RW] Video Surface 0 Rectangle Function Enable Mask */
#define BCHP_CMP_2_V0_NL_CSC_CTRL                0x000472d0 /* [RW] Video Surface 0 Nonconstant Luminance CSC Control */
#define BCHP_CMP_2_V0_REV_CCA_CTRL               0x000472d4 /* [RW] Video Surface 0 Reverse CCA Control */
#define BCHP_CMP_2_V0_R0_MC_COEFF_C00            0x00047500 /* [RW] Video Surface 0 Color Matrix C R0 coefficient c00 */
#define BCHP_CMP_2_V0_R0_MC_COEFF_C01            0x00047504 /* [RW] Video Surface 0 Color Matrix C R0 coefficient c01 */
#define BCHP_CMP_2_V0_R0_MC_COEFF_C02            0x00047508 /* [RW] Video Surface 0 Color Matrix C R0 coefficient c02 */
#define BCHP_CMP_2_V0_R0_MC_COEFF_C03            0x0004750c /* [RW] Video Surface 0 Color Matrix C R0 coefficient c03 */
#define BCHP_CMP_2_V0_R0_MC_COEFF_C10            0x00047510 /* [RW] Video Surface 0 Color Matrix C R0 coefficient c10 */
#define BCHP_CMP_2_V0_R0_MC_COEFF_C11            0x00047514 /* [RW] Video Surface 0 Color Matrix C R0 coefficient c11 */
#define BCHP_CMP_2_V0_R0_MC_COEFF_C12            0x00047518 /* [RW] Video Surface 0 Color Matrix C R0 coefficient c12 */
#define BCHP_CMP_2_V0_R0_MC_COEFF_C13            0x0004751c /* [RW] Video Surface 0 Color Matrix C R0 coefficient c13 */
#define BCHP_CMP_2_V0_R0_MC_COEFF_C20            0x00047520 /* [RW] Video Surface 0 Color Matrix C R0 coefficient c20 */
#define BCHP_CMP_2_V0_R0_MC_COEFF_C21            0x00047524 /* [RW] Video Surface 0 Color Matrix C R0 coefficient c21 */
#define BCHP_CMP_2_V0_R0_MC_COEFF_C22            0x00047528 /* [RW] Video Surface 0 Color Matrix C R0 coefficient c22 */
#define BCHP_CMP_2_V0_R0_MC_COEFF_C23            0x0004752c /* [RW] Video Surface 0 Color Matrix C R0 coefficient c23 */
#define BCHP_CMP_2_V0_R1_MC_COEFF_C00            0x00047530 /* [RW] Video Surface 0 Color Matrix C R1 coefficient c00 */
#define BCHP_CMP_2_V0_R1_MC_COEFF_C01            0x00047534 /* [RW] Video Surface 0 Color Matrix C R1 coefficient c01 */
#define BCHP_CMP_2_V0_R1_MC_COEFF_C02            0x00047538 /* [RW] Video Surface 0 Color Matrix C R1 coefficient c02 */
#define BCHP_CMP_2_V0_R1_MC_COEFF_C03            0x0004753c /* [RW] Video Surface 0 Color Matrix C R1 coefficient c03 */
#define BCHP_CMP_2_V0_R1_MC_COEFF_C10            0x00047540 /* [RW] Video Surface 0 Color Matrix C R1 coefficient c10 */
#define BCHP_CMP_2_V0_R1_MC_COEFF_C11            0x00047544 /* [RW] Video Surface 0 Color Matrix C R1 coefficient c11 */
#define BCHP_CMP_2_V0_R1_MC_COEFF_C12            0x00047548 /* [RW] Video Surface 0 Color Matrix C R1 coefficient c12 */
#define BCHP_CMP_2_V0_R1_MC_COEFF_C13            0x0004754c /* [RW] Video Surface 0 Color Matrix C R1 coefficient c13 */
#define BCHP_CMP_2_V0_R1_MC_COEFF_C20            0x00047550 /* [RW] Video Surface 0 Color Matrix C R1 coefficient c20 */
#define BCHP_CMP_2_V0_R1_MC_COEFF_C21            0x00047554 /* [RW] Video Surface 0 Color Matrix C R1 coefficient c21 */
#define BCHP_CMP_2_V0_R1_MC_COEFF_C22            0x00047558 /* [RW] Video Surface 0 Color Matrix C R1 coefficient c22 */
#define BCHP_CMP_2_V0_R1_MC_COEFF_C23            0x0004755c /* [RW] Video Surface 0 Color Matrix C R1 coefficient c23 */
#define BCHP_CMP_2_V0_R2_MC_COEFF_C00            0x00047560 /* [RW] Video Surface 0 Color Matrix C R2 coefficient c00 */
#define BCHP_CMP_2_V0_R2_MC_COEFF_C01            0x00047564 /* [RW] Video Surface 0 Color Matrix C R2 coefficient c01 */
#define BCHP_CMP_2_V0_R2_MC_COEFF_C02            0x00047568 /* [RW] Video Surface 0 Color Matrix C R2 coefficient c02 */
#define BCHP_CMP_2_V0_R2_MC_COEFF_C03            0x0004756c /* [RW] Video Surface 0 Color Matrix C R2 coefficient c03 */
#define BCHP_CMP_2_V0_R2_MC_COEFF_C10            0x00047570 /* [RW] Video Surface 0 Color Matrix C R2 coefficient c10 */
#define BCHP_CMP_2_V0_R2_MC_COEFF_C11            0x00047574 /* [RW] Video Surface 0 Color Matrix C R2 coefficient c11 */
#define BCHP_CMP_2_V0_R2_MC_COEFF_C12            0x00047578 /* [RW] Video Surface 0 Color Matrix C R2 coefficient c12 */
#define BCHP_CMP_2_V0_R2_MC_COEFF_C13            0x0004757c /* [RW] Video Surface 0 Color Matrix C R2 coefficient c13 */
#define BCHP_CMP_2_V0_R2_MC_COEFF_C20            0x00047580 /* [RW] Video Surface 0 Color Matrix C R2 coefficient c20 */
#define BCHP_CMP_2_V0_R2_MC_COEFF_C21            0x00047584 /* [RW] Video Surface 0 Color Matrix C R2 coefficient c21 */
#define BCHP_CMP_2_V0_R2_MC_COEFF_C22            0x00047588 /* [RW] Video Surface 0 Color Matrix C R2 coefficient c22 */
#define BCHP_CMP_2_V0_R2_MC_COEFF_C23            0x0004758c /* [RW] Video Surface 0 Color Matrix C R2 coefficient c23 */
#define BCHP_CMP_2_V0_R3_MC_COEFF_C00            0x00047590 /* [RW] Video Surface 0 Color Matrix C R3 coefficient c00 */
#define BCHP_CMP_2_V0_R3_MC_COEFF_C01            0x00047594 /* [RW] Video Surface 0 Color Matrix C R3 coefficient c01 */
#define BCHP_CMP_2_V0_R3_MC_COEFF_C02            0x00047598 /* [RW] Video Surface 0 Color Matrix C R3 coefficient c02 */
#define BCHP_CMP_2_V0_R3_MC_COEFF_C03            0x0004759c /* [RW] Video Surface 0 Color Matrix C R3 coefficient c03 */
#define BCHP_CMP_2_V0_R3_MC_COEFF_C10            0x000475a0 /* [RW] Video Surface 0 Color Matrix C R3 coefficient c10 */
#define BCHP_CMP_2_V0_R3_MC_COEFF_C11            0x000475a4 /* [RW] Video Surface 0 Color Matrix C R3 coefficient c11 */
#define BCHP_CMP_2_V0_R3_MC_COEFF_C12            0x000475a8 /* [RW] Video Surface 0 Color Matrix C R3 coefficient c12 */
#define BCHP_CMP_2_V0_R3_MC_COEFF_C13            0x000475ac /* [RW] Video Surface 0 Color Matrix C R3 coefficient c13 */
#define BCHP_CMP_2_V0_R3_MC_COEFF_C20            0x000475b0 /* [RW] Video Surface 0 Color Matrix C R3 coefficient c20 */
#define BCHP_CMP_2_V0_R3_MC_COEFF_C21            0x000475b4 /* [RW] Video Surface 0 Color Matrix C R3 coefficient c21 */
#define BCHP_CMP_2_V0_R3_MC_COEFF_C22            0x000475b8 /* [RW] Video Surface 0 Color Matrix C R3 coefficient c22 */
#define BCHP_CMP_2_V0_R3_MC_COEFF_C23            0x000475bc /* [RW] Video Surface 0 Color Matrix C R3 coefficient c23 */
#define BCHP_CMP_2_V0_R4_MC_COEFF_C00            0x000475c0 /* [RW] Video Surface 0 Color Matrix C R4 coefficient c00 */
#define BCHP_CMP_2_V0_R4_MC_COEFF_C01            0x000475c4 /* [RW] Video Surface 0 Color Matrix C R4 coefficient c01 */
#define BCHP_CMP_2_V0_R4_MC_COEFF_C02            0x000475c8 /* [RW] Video Surface 0 Color Matrix C R4 coefficient c02 */
#define BCHP_CMP_2_V0_R4_MC_COEFF_C03            0x000475cc /* [RW] Video Surface 0 Color Matrix C R4 coefficient c03 */
#define BCHP_CMP_2_V0_R4_MC_COEFF_C10            0x000475d0 /* [RW] Video Surface 0 Color Matrix C R4 coefficient c10 */
#define BCHP_CMP_2_V0_R4_MC_COEFF_C11            0x000475d4 /* [RW] Video Surface 0 Color Matrix C R4 coefficient c11 */
#define BCHP_CMP_2_V0_R4_MC_COEFF_C12            0x000475d8 /* [RW] Video Surface 0 Color Matrix C R4 coefficient c12 */
#define BCHP_CMP_2_V0_R4_MC_COEFF_C13            0x000475dc /* [RW] Video Surface 0 Color Matrix C R4 coefficient c13 */
#define BCHP_CMP_2_V0_R4_MC_COEFF_C20            0x000475e0 /* [RW] Video Surface 0 Color Matrix C R4 coefficient c20 */
#define BCHP_CMP_2_V0_R4_MC_COEFF_C21            0x000475e4 /* [RW] Video Surface 0 Color Matrix C R4 coefficient c21 */
#define BCHP_CMP_2_V0_R4_MC_COEFF_C22            0x000475e8 /* [RW] Video Surface 0 Color Matrix C R4 coefficient c22 */
#define BCHP_CMP_2_V0_R4_MC_COEFF_C23            0x000475ec /* [RW] Video Surface 0 Color Matrix C R4 coefficient c23 */
#define BCHP_CMP_2_V0_R5_MC_COEFF_C00            0x000475f0 /* [RW] Video Surface 0 Color Matrix C R5 coefficient c00 */
#define BCHP_CMP_2_V0_R5_MC_COEFF_C01            0x000475f4 /* [RW] Video Surface 0 Color Matrix C R5 coefficient c01 */
#define BCHP_CMP_2_V0_R5_MC_COEFF_C02            0x000475f8 /* [RW] Video Surface 0 Color Matrix C R5 coefficient c02 */
#define BCHP_CMP_2_V0_R5_MC_COEFF_C03            0x000475fc /* [RW] Video Surface 0 Color Matrix C R5 coefficient c03 */
#define BCHP_CMP_2_V0_R5_MC_COEFF_C10            0x00047600 /* [RW] Video Surface 0 Color Matrix C R5 coefficient c10 */
#define BCHP_CMP_2_V0_R5_MC_COEFF_C11            0x00047604 /* [RW] Video Surface 0 Color Matrix C R5 coefficient c11 */
#define BCHP_CMP_2_V0_R5_MC_COEFF_C12            0x00047608 /* [RW] Video Surface 0 Color Matrix C R5 coefficient c12 */
#define BCHP_CMP_2_V0_R5_MC_COEFF_C13            0x0004760c /* [RW] Video Surface 0 Color Matrix C R5 coefficient c13 */
#define BCHP_CMP_2_V0_R5_MC_COEFF_C20            0x00047610 /* [RW] Video Surface 0 Color Matrix C R5 coefficient c20 */
#define BCHP_CMP_2_V0_R5_MC_COEFF_C21            0x00047614 /* [RW] Video Surface 0 Color Matrix C R5 coefficient c21 */
#define BCHP_CMP_2_V0_R5_MC_COEFF_C22            0x00047618 /* [RW] Video Surface 0 Color Matrix C R5 coefficient c22 */
#define BCHP_CMP_2_V0_R5_MC_COEFF_C23            0x0004761c /* [RW] Video Surface 0 Color Matrix C R5 coefficient c23 */

/***************************************************************************
 *V0_RECT_SIZE%i - Rectangle Vertical and Horizontal Size.
 ***************************************************************************/
#define BCHP_CMP_2_V0_RECT_SIZEi_ARRAY_BASE                        0x00047250
#define BCHP_CMP_2_V0_RECT_SIZEi_ARRAY_START                       0
#define BCHP_CMP_2_V0_RECT_SIZEi_ARRAY_END                         15
#define BCHP_CMP_2_V0_RECT_SIZEi_ARRAY_ELEMENT_SIZE                32

/***************************************************************************
 *V0_RECT_SIZE%i - Rectangle Vertical and Horizontal Size.
 ***************************************************************************/
/* CMP_2 :: V0_RECT_SIZEi :: reserved0 [31:29] */
#define BCHP_CMP_2_V0_RECT_SIZEi_reserved0_MASK                    0xe0000000
#define BCHP_CMP_2_V0_RECT_SIZEi_reserved0_SHIFT                   29

/* CMP_2 :: V0_RECT_SIZEi :: HSIZE [28:16] */
#define BCHP_CMP_2_V0_RECT_SIZEi_HSIZE_MASK                        0x1fff0000
#define BCHP_CMP_2_V0_RECT_SIZEi_HSIZE_SHIFT                       16
#define BCHP_CMP_2_V0_RECT_SIZEi_HSIZE_DEFAULT                     0x00000000

/* CMP_2 :: V0_RECT_SIZEi :: reserved1 [15:12] */
#define BCHP_CMP_2_V0_RECT_SIZEi_reserved1_MASK                    0x0000f000
#define BCHP_CMP_2_V0_RECT_SIZEi_reserved1_SHIFT                   12

/* CMP_2 :: V0_RECT_SIZEi :: VSIZE [11:00] */
#define BCHP_CMP_2_V0_RECT_SIZEi_VSIZE_MASK                        0x00000fff
#define BCHP_CMP_2_V0_RECT_SIZEi_VSIZE_SHIFT                       0
#define BCHP_CMP_2_V0_RECT_SIZEi_VSIZE_DEFAULT                     0x00000000


/***************************************************************************
 *V0_RECT_OFFSET%i - Rectangle Starting Point Offset from Surface Origin.
 ***************************************************************************/
#define BCHP_CMP_2_V0_RECT_OFFSETi_ARRAY_BASE                      0x00047290
#define BCHP_CMP_2_V0_RECT_OFFSETi_ARRAY_START                     0
#define BCHP_CMP_2_V0_RECT_OFFSETi_ARRAY_END                       15
#define BCHP_CMP_2_V0_RECT_OFFSETi_ARRAY_ELEMENT_SIZE              32

/***************************************************************************
 *V0_RECT_OFFSET%i - Rectangle Starting Point Offset from Surface Origin.
 ***************************************************************************/
/* CMP_2 :: V0_RECT_OFFSETi :: reserved0 [31:29] */
#define BCHP_CMP_2_V0_RECT_OFFSETi_reserved0_MASK                  0xe0000000
#define BCHP_CMP_2_V0_RECT_OFFSETi_reserved0_SHIFT                 29

/* CMP_2 :: V0_RECT_OFFSETi :: X_OFFSET [28:16] */
#define BCHP_CMP_2_V0_RECT_OFFSETi_X_OFFSET_MASK                   0x1fff0000
#define BCHP_CMP_2_V0_RECT_OFFSETi_X_OFFSET_SHIFT                  16
#define BCHP_CMP_2_V0_RECT_OFFSETi_X_OFFSET_DEFAULT                0x00000000

/* CMP_2 :: V0_RECT_OFFSETi :: reserved1 [15:12] */
#define BCHP_CMP_2_V0_RECT_OFFSETi_reserved1_MASK                  0x0000f000
#define BCHP_CMP_2_V0_RECT_OFFSETi_reserved1_SHIFT                 12

/* CMP_2 :: V0_RECT_OFFSETi :: Y_OFFSET [11:00] */
#define BCHP_CMP_2_V0_RECT_OFFSETi_Y_OFFSET_MASK                   0x00000fff
#define BCHP_CMP_2_V0_RECT_OFFSETi_Y_OFFSET_SHIFT                  0
#define BCHP_CMP_2_V0_RECT_OFFSETi_Y_OFFSET_DEFAULT                0x00000000


#endif /* #ifndef BCHP_CMP_2_H__ */

/* End of File */
