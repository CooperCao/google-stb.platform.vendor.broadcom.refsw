/***************************************************************************
 *     Copyright (c) 1999-2013, Broadcom Corporation
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
 * Date:           Generated on              Mon Sep 23 09:50:36 2013
 *                 Full Compile MD5 Checksum fcccce298b546dd6a1f4cbad288478da
 *                   (minus title and desc)
 *                 MD5 Checksum              211556602e37a33262598b3d5eeba81c
 *
 * Compiled with:  RDB Utility               combo_header.pl
 *                 RDB Parser                3.0
 *                 unknown                   unknown
 *                 Perl Interpreter          5.008008
 *                 Operating System          linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BCHP_CMP_4_H__
#define BCHP_CMP_4_H__

/***************************************************************************
 *CMP_4 - Video Compositor 4 Registers
 ***************************************************************************/
#define BCHP_CMP_4_REVISION                      0x00644800 /* Compositor Revision ID */
#define BCHP_CMP_4_HW_CONFIGURATION              0x00644804 /* Compositor HW Configuration */
#define BCHP_CMP_4_CANVAS_CTRL                   0x00644808 /* Canvas control */
#define BCHP_CMP_4_CANVAS_SIZE                   0x0064480c /* Canvas Vertical and Horizontal Size */
#define BCHP_CMP_4_BG_COLOR                      0x00644810 /* Background color register */
#define BCHP_CMP_4_BLEND_0_CTRL                  0x00644814 /* Blending Control for First stage Blender */
#define BCHP_CMP_4_BLEND_1_CTRL                  0x00644818 /* Blending Control for Second stage Blender */
#define BCHP_CMP_4_CMP_CTRL                      0x00644828 /* Compositor Control */
#define BCHP_CMP_4_CMP_OUT_CTRL                  0x00644840 /* Compositor Output Control */
#define BCHP_CMP_4_CRC_CTRL                      0x00644844 /* BVN CRC Control Register */
#define BCHP_CMP_4_CRC_Y_STATUS                  0x00644848 /* BVN CRC Luma Status Register */
#define BCHP_CMP_4_CRC_CB_STATUS                 0x0064484c /* BVN CRC Chroma(Cb) Status Register */
#define BCHP_CMP_4_CRC_CR_STATUS                 0x00644850 /* BVN CRC Chroma(Cr) Status Register */
#define BCHP_CMP_4_READBACK_POSITION             0x00644854 /* Compositor Position For CMP Output Readback */
#define BCHP_CMP_4_READBACK_VALUE                0x00644858 /* Compositor Readback Value at Specified Position */
#define BCHP_CMP_4_CSC_DEMO_SETTING              0x0064485c /* Compositor Color Space Converter Demo Setting */
#define BCHP_CMP_4_SCRATCH_REGISTER              0x00644860 /* Compositor Scratch Register */
#define BCHP_CMP_4_TEST_REGISTER                 0x00644868 /* Compositor Test Register */
#define BCHP_CMP_4_TEST_REGISTER1                0x0064486c /* Compositor Test Register1 */
#define BCHP_CMP_4_G0_SURFACE_SIZE               0x00644880 /* Graphics Surface 0 Vertical and Horizontal Size */
#define BCHP_CMP_4_G0_SURFACE_OFFSET             0x00644884 /* Graphics Surface 0 Vertical and Horizontal Offset */
#define BCHP_CMP_4_G0_DISPLAY_SIZE               0x00644888 /* Graphics Surface 0 Display Vertical and Horizontal Size */
#define BCHP_CMP_4_G0_CANVAS_OFFSET              0x0064488c /* Graphics Surface 0 Canvas Vertical and Horizontal Offset */
#define BCHP_CMP_4_G0_CANVAS_X_OFFSET_R          0x00644890 /* Graphics Surface 0 Canvas Horizontal Offset For Right or Under Window in 3D mode */
#define BCHP_CMP_4_G0_SURFACE_CTRL               0x00644894 /* Graphics Surface 0 Control */
#define BCHP_CMP_4_G0_BVB_IN_STATUS_CLEAR        0x00644898 /* Graphics Surface 0 BVB Input Status Clear */
#define BCHP_CMP_4_G0_BVB_IN_STATUS              0x0064489c /* Graphics Surface 0 BVB Input Status */
#define BCHP_CMP_4_V0_SURFACE_SIZE               0x006448b0 /* Video Surface 0 Vertical and Horizontal Size */
#define BCHP_CMP_4_V0_SURFACE_OFFSET             0x006448b4 /* Video Surface 0 Vertical and Horizontal Offset */
#define BCHP_CMP_4_V0_DISPLAY_SIZE               0x006448b8 /* Video Surface 0 Display Vertical and Horizontal Size */
#define BCHP_CMP_4_V0_CANVAS_OFFSET              0x006448bc /* Video Surface 0 Canvas Vertical and Horizontal Offset */
#define BCHP_CMP_4_V0_CANVAS_X_OFFSET_R          0x006448c0 /* Video Surface 0 Canvas Horizontal Offset For Right or Under Window in 3D mode */
#define BCHP_CMP_4_V0_SURFACE_CTRL               0x006448c4 /* Video Surface 0 Control */
#define BCHP_CMP_4_V0_BVB_IN_STATUS_CLEAR        0x006448c8 /* Video Surface 0 BVB Input Status Clear */
#define BCHP_CMP_4_V0_BVB_IN_STATUS              0x006448cc /* Video Surface 0 BVB Input Status */
#define BCHP_CMP_4_V0_CONST_COLOR                0x006448d0 /* Video Surface 0 Constant Color Register */
#define BCHP_CMP_4_V0_CB_KEYING                  0x006448d4 /* Video Surface 0 Chroma (Blue) Key */
#define BCHP_CMP_4_V0_CR_KEYING                  0x006448d8 /* Video Surface 0 Chroma (Red) Key */
#define BCHP_CMP_4_V0_LUMA_KEYING                0x006448dc /* Video Surface 0 Luma Key */
#define BCHP_CMP_4_V0_RECT_CSC_INDEX_0           0x006448e0 /* Video Surface 0 rectangle CSC index 0 */
#define BCHP_CMP_4_V0_RECT_CSC_INDEX_1           0x006448e4 /* Video Surface 0 rectangle CSC index 1 */
#define BCHP_CMP_4_V0_R0_MC_COEFF_C00            0x006448e8 /* Video Surface 0 Color Matrix R0 coefficient c00 */
#define BCHP_CMP_4_V0_R0_MC_COEFF_C01            0x006448ec /* Video Surface 0 Color Matrix R0 coefficient c01 */
#define BCHP_CMP_4_V0_R0_MC_COEFF_C02            0x006448f0 /* Video Surface 0 Color Matrix R0 coefficient c02 */
#define BCHP_CMP_4_V0_R0_MC_COEFF_C03            0x006448f4 /* Video Surface 0 Color Matrix R0 coefficient c03 */
#define BCHP_CMP_4_V0_R0_MC_COEFF_C10            0x006448f8 /* Video Surface 0 Color Matrix R0 coefficient c10 */
#define BCHP_CMP_4_V0_R0_MC_COEFF_C11            0x006448fc /* Video Surface 0 Color Matrix R0 coefficient c11 */
#define BCHP_CMP_4_V0_R0_MC_COEFF_C12            0x00644900 /* Video Surface 0 Color Matrix R0 coefficient c12 */
#define BCHP_CMP_4_V0_R0_MC_COEFF_C13            0x00644904 /* Video Surface 0 Color Matrix R0 coefficient c13 */
#define BCHP_CMP_4_V0_R0_MC_COEFF_C20            0x00644908 /* Video Surface 0 Color Matrix R0 coefficient c20 */
#define BCHP_CMP_4_V0_R0_MC_COEFF_C21            0x0064490c /* Video Surface 0 Color Matrix R0 coefficient c21 */
#define BCHP_CMP_4_V0_R0_MC_COEFF_C22            0x00644910 /* Video Surface 0 Color Matrix R0 coefficient c22 */
#define BCHP_CMP_4_V0_R0_MC_COEFF_C23            0x00644914 /* Video Surface 0 Color Matrix R0 coefficient c23 */
#define BCHP_CMP_4_V0_R1_MC_COEFF_C00            0x00644918 /* Video Surface 0 Color Matrix R1 coefficient c00 */
#define BCHP_CMP_4_V0_R1_MC_COEFF_C01            0x0064491c /* Video Surface 0 Color Matrix R1 coefficient c01 */
#define BCHP_CMP_4_V0_R1_MC_COEFF_C02            0x00644920 /* Video Surface 0 Color Matrix R1 coefficient c02 */
#define BCHP_CMP_4_V0_R1_MC_COEFF_C03            0x00644924 /* Video Surface 0 Color Matrix R1 coefficient c03 */
#define BCHP_CMP_4_V0_R1_MC_COEFF_C10            0x00644928 /* Video Surface 0 Color Matrix R1 coefficient c10 */
#define BCHP_CMP_4_V0_R1_MC_COEFF_C11            0x0064492c /* Video Surface 0 Color Matrix R1 coefficient c11 */
#define BCHP_CMP_4_V0_R1_MC_COEFF_C12            0x00644930 /* Video Surface 0 Color Matrix R1 coefficient c12 */
#define BCHP_CMP_4_V0_R1_MC_COEFF_C13            0x00644934 /* Video Surface 0 Color Matrix R1 coefficient c13 */
#define BCHP_CMP_4_V0_R1_MC_COEFF_C20            0x00644938 /* Video Surface 0 Color Matrix R1 coefficient c20 */
#define BCHP_CMP_4_V0_R1_MC_COEFF_C21            0x0064493c /* Video Surface 0 Color Matrix R1 coefficient c21 */
#define BCHP_CMP_4_V0_R1_MC_COEFF_C22            0x00644940 /* Video Surface 0 Color Matrix R1 coefficient c22 */
#define BCHP_CMP_4_V0_R1_MC_COEFF_C23            0x00644944 /* Video Surface 0 Color Matrix R1 coefficient c23 */
#define BCHP_CMP_4_V0_R2_MC_COEFF_C00            0x00644948 /* Video Surface 0 Color Matrix R2 coefficient c00 */
#define BCHP_CMP_4_V0_R2_MC_COEFF_C01            0x0064494c /* Video Surface 0 Color Matrix R2 coefficient c01 */
#define BCHP_CMP_4_V0_R2_MC_COEFF_C02            0x00644950 /* Video Surface 0 Color Matrix R2 coefficient c02 */
#define BCHP_CMP_4_V0_R2_MC_COEFF_C03            0x00644954 /* Video Surface 0 Color Matrix R2 coefficient c03 */
#define BCHP_CMP_4_V0_R2_MC_COEFF_C10            0x00644958 /* Video Surface 0 Color Matrix R2 coefficient c10 */
#define BCHP_CMP_4_V0_R2_MC_COEFF_C11            0x0064495c /* Video Surface 0 Color Matrix R2 coefficient c11 */
#define BCHP_CMP_4_V0_R2_MC_COEFF_C12            0x00644960 /* Video Surface 0 Color Matrix R2 coefficient c12 */
#define BCHP_CMP_4_V0_R2_MC_COEFF_C13            0x00644964 /* Video Surface 0 Color Matrix R2 coefficient c13 */
#define BCHP_CMP_4_V0_R2_MC_COEFF_C20            0x00644968 /* Video Surface 0 Color Matrix R2 coefficient c20 */
#define BCHP_CMP_4_V0_R2_MC_COEFF_C21            0x0064496c /* Video Surface 0 Color Matrix R2 coefficient c21 */
#define BCHP_CMP_4_V0_R2_MC_COEFF_C22            0x00644970 /* Video Surface 0 Color Matrix R2 coefficient c22 */
#define BCHP_CMP_4_V0_R2_MC_COEFF_C23            0x00644974 /* Video Surface 0 Color Matrix R2 coefficient c23 */
#define BCHP_CMP_4_V0_R3_MC_COEFF_C00            0x00644978 /* Video Surface 0 Color Matrix R3 coefficient c00 */
#define BCHP_CMP_4_V0_R3_MC_COEFF_C01            0x0064497c /* Video Surface 0 Color Matrix R3 coefficient c01 */
#define BCHP_CMP_4_V0_R3_MC_COEFF_C02            0x00644980 /* Video Surface 0 Color Matrix R3 coefficient c02 */
#define BCHP_CMP_4_V0_R3_MC_COEFF_C03            0x00644984 /* Video Surface 0 Color Matrix R3 coefficient c03 */
#define BCHP_CMP_4_V0_R3_MC_COEFF_C10            0x00644988 /* Video Surface 0 Color Matrix R3 coefficient c10 */
#define BCHP_CMP_4_V0_R3_MC_COEFF_C11            0x0064498c /* Video Surface 0 Color Matrix R3 coefficient c11 */
#define BCHP_CMP_4_V0_R3_MC_COEFF_C12            0x00644990 /* Video Surface 0 Color Matrix R3 coefficient c12 */
#define BCHP_CMP_4_V0_R3_MC_COEFF_C13            0x00644994 /* Video Surface 0 Color Matrix R3 coefficient c13 */
#define BCHP_CMP_4_V0_R3_MC_COEFF_C20            0x00644998 /* Video Surface 0 Color Matrix R3 coefficient c20 */
#define BCHP_CMP_4_V0_R3_MC_COEFF_C21            0x0064499c /* Video Surface 0 Color Matrix R3 coefficient c21 */
#define BCHP_CMP_4_V0_R3_MC_COEFF_C22            0x006449a0 /* Video Surface 0 Color Matrix R3 coefficient c22 */
#define BCHP_CMP_4_V0_R3_MC_COEFF_C23            0x006449a4 /* Video Surface 0 Color Matrix R3 coefficient c23 */
#define BCHP_CMP_4_V0_R4_MC_COEFF_C00            0x006449a8 /* Video Surface 0 Color Matrix R4 coefficient c00 */
#define BCHP_CMP_4_V0_R4_MC_COEFF_C01            0x006449ac /* Video Surface 0 Color Matrix R4 coefficient c01 */
#define BCHP_CMP_4_V0_R4_MC_COEFF_C02            0x006449b0 /* Video Surface 0 Color Matrix R4 coefficient c02 */
#define BCHP_CMP_4_V0_R4_MC_COEFF_C03            0x006449b4 /* Video Surface 0 Color Matrix R4 coefficient c03 */
#define BCHP_CMP_4_V0_R4_MC_COEFF_C10            0x006449b8 /* Video Surface 0 Color Matrix R4 coefficient c10 */
#define BCHP_CMP_4_V0_R4_MC_COEFF_C11            0x006449bc /* Video Surface 0 Color Matrix R4 coefficient c11 */
#define BCHP_CMP_4_V0_R4_MC_COEFF_C12            0x006449c0 /* Video Surface 0 Color Matrix R4 coefficient c12 */
#define BCHP_CMP_4_V0_R4_MC_COEFF_C13            0x006449c4 /* Video Surface 0 Color Matrix R4 coefficient c13 */
#define BCHP_CMP_4_V0_R4_MC_COEFF_C20            0x006449c8 /* Video Surface 0 Color Matrix R4 coefficient c20 */
#define BCHP_CMP_4_V0_R4_MC_COEFF_C21            0x006449cc /* Video Surface 0 Color Matrix R4 coefficient c21 */
#define BCHP_CMP_4_V0_R4_MC_COEFF_C22            0x006449d0 /* Video Surface 0 Color Matrix R4 coefficient c22 */
#define BCHP_CMP_4_V0_R4_MC_COEFF_C23            0x006449d4 /* Video Surface 0 Color Matrix R4 coefficient c23 */
#define BCHP_CMP_4_V0_RECT_COLOR                 0x006449d8 /* Video Surface 0 Rectangle Function Color Register */
#define BCHP_CMP_4_V0_RECT_TOP_CTRL              0x006449dc /* Video Surface 0 Top Level Rectangle Control for Mosaic or See Through. */
#define BCHP_CMP_4_V0_RECT_ENABLE_MASK           0x006449e0 /* Video Surface 0 Rectangle Function Enable Mask */

/***************************************************************************
 *V0_RECT_SIZE%i - Rectangle Vertical and Horizontal Size.
 ***************************************************************************/
#define BCHP_CMP_4_V0_RECT_SIZEi_ARRAY_BASE                        0x006449e4
#define BCHP_CMP_4_V0_RECT_SIZEi_ARRAY_START                       0
#define BCHP_CMP_4_V0_RECT_SIZEi_ARRAY_END                         15
#define BCHP_CMP_4_V0_RECT_SIZEi_ARRAY_ELEMENT_SIZE                32

/***************************************************************************
 *V0_RECT_SIZE%i - Rectangle Vertical and Horizontal Size.
 ***************************************************************************/
/* CMP_4 :: V0_RECT_SIZEi :: reserved0 [31:29] */
#define BCHP_CMP_4_V0_RECT_SIZEi_reserved0_MASK                    0xe0000000
#define BCHP_CMP_4_V0_RECT_SIZEi_reserved0_SHIFT                   29

/* CMP_4 :: V0_RECT_SIZEi :: HSIZE [28:16] */
#define BCHP_CMP_4_V0_RECT_SIZEi_HSIZE_MASK                        0x1fff0000
#define BCHP_CMP_4_V0_RECT_SIZEi_HSIZE_SHIFT                       16
#define BCHP_CMP_4_V0_RECT_SIZEi_HSIZE_DEFAULT                     0x00000000

/* CMP_4 :: V0_RECT_SIZEi :: reserved1 [15:12] */
#define BCHP_CMP_4_V0_RECT_SIZEi_reserved1_MASK                    0x0000f000
#define BCHP_CMP_4_V0_RECT_SIZEi_reserved1_SHIFT                   12

/* CMP_4 :: V0_RECT_SIZEi :: VSIZE [11:00] */
#define BCHP_CMP_4_V0_RECT_SIZEi_VSIZE_MASK                        0x00000fff
#define BCHP_CMP_4_V0_RECT_SIZEi_VSIZE_SHIFT                       0
#define BCHP_CMP_4_V0_RECT_SIZEi_VSIZE_DEFAULT                     0x00000000


/***************************************************************************
 *V0_RECT_OFFSET%i - Rectangle Starting Point Offset from Surface Origin.
 ***************************************************************************/
#define BCHP_CMP_4_V0_RECT_OFFSETi_ARRAY_BASE                      0x00644a24
#define BCHP_CMP_4_V0_RECT_OFFSETi_ARRAY_START                     0
#define BCHP_CMP_4_V0_RECT_OFFSETi_ARRAY_END                       15
#define BCHP_CMP_4_V0_RECT_OFFSETi_ARRAY_ELEMENT_SIZE              32

/***************************************************************************
 *V0_RECT_OFFSET%i - Rectangle Starting Point Offset from Surface Origin.
 ***************************************************************************/
/* CMP_4 :: V0_RECT_OFFSETi :: reserved0 [31:29] */
#define BCHP_CMP_4_V0_RECT_OFFSETi_reserved0_MASK                  0xe0000000
#define BCHP_CMP_4_V0_RECT_OFFSETi_reserved0_SHIFT                 29

/* CMP_4 :: V0_RECT_OFFSETi :: X_OFFSET [28:16] */
#define BCHP_CMP_4_V0_RECT_OFFSETi_X_OFFSET_MASK                   0x1fff0000
#define BCHP_CMP_4_V0_RECT_OFFSETi_X_OFFSET_SHIFT                  16
#define BCHP_CMP_4_V0_RECT_OFFSETi_X_OFFSET_DEFAULT                0x00000000

/* CMP_4 :: V0_RECT_OFFSETi :: reserved1 [15:12] */
#define BCHP_CMP_4_V0_RECT_OFFSETi_reserved1_MASK                  0x0000f000
#define BCHP_CMP_4_V0_RECT_OFFSETi_reserved1_SHIFT                 12

/* CMP_4 :: V0_RECT_OFFSETi :: Y_OFFSET [11:00] */
#define BCHP_CMP_4_V0_RECT_OFFSETi_Y_OFFSET_MASK                   0x00000fff
#define BCHP_CMP_4_V0_RECT_OFFSETi_Y_OFFSET_SHIFT                  0
#define BCHP_CMP_4_V0_RECT_OFFSETi_Y_OFFSET_DEFAULT                0x00000000


#endif /* #ifndef BCHP_CMP_4_H__ */

/* End of File */
