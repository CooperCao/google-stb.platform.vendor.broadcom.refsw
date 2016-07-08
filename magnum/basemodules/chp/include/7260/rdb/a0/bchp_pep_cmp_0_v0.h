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
 * Date:           Generated on               Fri Feb 26 13:24:11 2016
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

#ifndef BCHP_PEP_CMP_0_V0_H__
#define BCHP_PEP_CMP_0_V0_H__

/***************************************************************************
 *PEP_CMP_0_V0 - Picture Enhancement Processing Unit in Video Compositor 0/Video Intra Surface 0
 ***************************************************************************/
#define BCHP_PEP_CMP_0_V0_REVISION               0x20646400 /* [RO] PEP Revision ID */
#define BCHP_PEP_CMP_0_V0_SCRATCH_REGISTER       0x20646404 /* [RW] PEP Scratch Register */
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING       0x20646408 /* [RW] CAB Demo Setting */
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING       0x2064640c /* [RW] LAB Demo Setting */
#define BCHP_PEP_CMP_0_V0_CAB_CTRL               0x20646410 /* [RW] Color Adjust Block Control */
#define BCHP_PEP_CMP_0_V0_LAB_CTRL               0x20646414 /* [RW] Luma Adjust Block Control */
#define BCHP_PEP_CMP_0_V0_HISTO_CTRL             0x20646418 /* [RW] Luma Histogram Control */
#define BCHP_PEP_CMP_0_V0_MIN_MAX_CTRL           0x2064641c /* [RW] Luma Histogram Control */
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_OFFSET       0x20646434 /* [RW] Histogram Window X and Y Offset */
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_SIZE         0x20646438 /* [RW] Histogram Window Vertical and Horizontal Size */
#define BCHP_PEP_CMP_0_V0_HISTO_MIN_MAX          0x2064643c /* [RO] Histogram Min and Max value */
#define BCHP_PEP_CMP_0_V0_HISTO_RESET            0x20646480 /* [WO] Luma histogram reset */
#define BCHP_PEP_CMP_0_V0_MIN_MAX_RESET          0x20646484 /* [WO] Luma min/max value reset */

/***************************************************************************
 *REVISION - PEP Revision ID
 ***************************************************************************/
/* PEP_CMP_0_V0 :: REVISION :: reserved0 [31:16] */
#define BCHP_PEP_CMP_0_V0_REVISION_reserved0_MASK                  0xffff0000
#define BCHP_PEP_CMP_0_V0_REVISION_reserved0_SHIFT                 16

/* PEP_CMP_0_V0 :: REVISION :: MAJOR [15:08] */
#define BCHP_PEP_CMP_0_V0_REVISION_MAJOR_MASK                      0x0000ff00
#define BCHP_PEP_CMP_0_V0_REVISION_MAJOR_SHIFT                     8
#define BCHP_PEP_CMP_0_V0_REVISION_MAJOR_DEFAULT                   0x00000000

/* PEP_CMP_0_V0 :: REVISION :: MINOR [07:00] */
#define BCHP_PEP_CMP_0_V0_REVISION_MINOR_MASK                      0x000000ff
#define BCHP_PEP_CMP_0_V0_REVISION_MINOR_SHIFT                     0
#define BCHP_PEP_CMP_0_V0_REVISION_MINOR_DEFAULT                   0x00000039

/***************************************************************************
 *SCRATCH_REGISTER - PEP Scratch Register
 ***************************************************************************/
/* PEP_CMP_0_V0 :: SCRATCH_REGISTER :: reserved0 [31:16] */
#define BCHP_PEP_CMP_0_V0_SCRATCH_REGISTER_reserved0_MASK          0xffff0000
#define BCHP_PEP_CMP_0_V0_SCRATCH_REGISTER_reserved0_SHIFT         16

/* PEP_CMP_0_V0 :: SCRATCH_REGISTER :: VALUE [15:00] */
#define BCHP_PEP_CMP_0_V0_SCRATCH_REGISTER_VALUE_MASK              0x0000ffff
#define BCHP_PEP_CMP_0_V0_SCRATCH_REGISTER_VALUE_SHIFT             0
#define BCHP_PEP_CMP_0_V0_SCRATCH_REGISTER_VALUE_DEFAULT           0x00000000

/***************************************************************************
 *CAB_DEMO_SETTING - CAB Demo Setting
 ***************************************************************************/
/* PEP_CMP_0_V0 :: CAB_DEMO_SETTING :: reserved0 [31:17] */
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_reserved0_MASK          0xfffe0000
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_reserved0_SHIFT         17

/* PEP_CMP_0_V0 :: CAB_DEMO_SETTING :: DEMO_L_R [16:16] */
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_L_R_MASK           0x00010000
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_L_R_SHIFT          16
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_L_R_DEFAULT        0x00000000
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_L_R_LEFT           1
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_L_R_RIGHT          0

/* PEP_CMP_0_V0 :: CAB_DEMO_SETTING :: reserved1 [15:13] */
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_reserved1_MASK          0x0000e000
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_reserved1_SHIFT         13

/* PEP_CMP_0_V0 :: CAB_DEMO_SETTING :: DEMO_BOUNDARY [12:00] */
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_BOUNDARY_MASK      0x00001fff
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_BOUNDARY_SHIFT     0
#define BCHP_PEP_CMP_0_V0_CAB_DEMO_SETTING_DEMO_BOUNDARY_DEFAULT   0x00000168

/***************************************************************************
 *LAB_DEMO_SETTING - LAB Demo Setting
 ***************************************************************************/
/* PEP_CMP_0_V0 :: LAB_DEMO_SETTING :: reserved0 [31:17] */
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_reserved0_MASK          0xfffe0000
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_reserved0_SHIFT         17

/* PEP_CMP_0_V0 :: LAB_DEMO_SETTING :: DEMO_L_R [16:16] */
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_DEMO_L_R_MASK           0x00010000
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_DEMO_L_R_SHIFT          16
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_DEMO_L_R_DEFAULT        0x00000000
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_DEMO_L_R_LEFT           1
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_DEMO_L_R_RIGHT          0

/* PEP_CMP_0_V0 :: LAB_DEMO_SETTING :: reserved1 [15:13] */
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_reserved1_MASK          0x0000e000
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_reserved1_SHIFT         13

/* PEP_CMP_0_V0 :: LAB_DEMO_SETTING :: DEMO_BOUNDARY [12:00] */
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_DEMO_BOUNDARY_MASK      0x00001fff
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_DEMO_BOUNDARY_SHIFT     0
#define BCHP_PEP_CMP_0_V0_LAB_DEMO_SETTING_DEMO_BOUNDARY_DEFAULT   0x00000168

/***************************************************************************
 *CAB_CTRL - Color Adjust Block Control
 ***************************************************************************/
/* PEP_CMP_0_V0 :: CAB_CTRL :: reserved0 [31:03] */
#define BCHP_PEP_CMP_0_V0_CAB_CTRL_reserved0_MASK                  0xfffffff8
#define BCHP_PEP_CMP_0_V0_CAB_CTRL_reserved0_SHIFT                 3

/* PEP_CMP_0_V0 :: CAB_CTRL :: CAB_DEMO_ENABLE [02:02] */
#define BCHP_PEP_CMP_0_V0_CAB_CTRL_CAB_DEMO_ENABLE_MASK            0x00000004
#define BCHP_PEP_CMP_0_V0_CAB_CTRL_CAB_DEMO_ENABLE_SHIFT           2
#define BCHP_PEP_CMP_0_V0_CAB_CTRL_CAB_DEMO_ENABLE_DEFAULT         0x00000000

/* PEP_CMP_0_V0 :: CAB_CTRL :: LUMA_OFFSET_ENABLE [01:01] */
#define BCHP_PEP_CMP_0_V0_CAB_CTRL_LUMA_OFFSET_ENABLE_MASK         0x00000002
#define BCHP_PEP_CMP_0_V0_CAB_CTRL_LUMA_OFFSET_ENABLE_SHIFT        1
#define BCHP_PEP_CMP_0_V0_CAB_CTRL_LUMA_OFFSET_ENABLE_DEFAULT      0x00000000

/* PEP_CMP_0_V0 :: CAB_CTRL :: CAB_ENABLE [00:00] */
#define BCHP_PEP_CMP_0_V0_CAB_CTRL_CAB_ENABLE_MASK                 0x00000001
#define BCHP_PEP_CMP_0_V0_CAB_CTRL_CAB_ENABLE_SHIFT                0
#define BCHP_PEP_CMP_0_V0_CAB_CTRL_CAB_ENABLE_DEFAULT              0x00000000

/***************************************************************************
 *LAB_CTRL - Luma Adjust Block Control
 ***************************************************************************/
/* PEP_CMP_0_V0 :: LAB_CTRL :: reserved0 [31:02] */
#define BCHP_PEP_CMP_0_V0_LAB_CTRL_reserved0_MASK                  0xfffffffc
#define BCHP_PEP_CMP_0_V0_LAB_CTRL_reserved0_SHIFT                 2

/* PEP_CMP_0_V0 :: LAB_CTRL :: LAB_DEMO_ENABLE [01:01] */
#define BCHP_PEP_CMP_0_V0_LAB_CTRL_LAB_DEMO_ENABLE_MASK            0x00000002
#define BCHP_PEP_CMP_0_V0_LAB_CTRL_LAB_DEMO_ENABLE_SHIFT           1
#define BCHP_PEP_CMP_0_V0_LAB_CTRL_LAB_DEMO_ENABLE_DEFAULT         0x00000000

/* PEP_CMP_0_V0 :: LAB_CTRL :: ENABLE [00:00] */
#define BCHP_PEP_CMP_0_V0_LAB_CTRL_ENABLE_MASK                     0x00000001
#define BCHP_PEP_CMP_0_V0_LAB_CTRL_ENABLE_SHIFT                    0
#define BCHP_PEP_CMP_0_V0_LAB_CTRL_ENABLE_DEFAULT                  0x00000000

/***************************************************************************
 *HISTO_CTRL - Luma Histogram Control
 ***************************************************************************/
/* PEP_CMP_0_V0 :: HISTO_CTRL :: reserved0 [31:02] */
#define BCHP_PEP_CMP_0_V0_HISTO_CTRL_reserved0_MASK                0xfffffffc
#define BCHP_PEP_CMP_0_V0_HISTO_CTRL_reserved0_SHIFT               2

/* PEP_CMP_0_V0 :: HISTO_CTRL :: HISTO_CTRL [01:00] */
#define BCHP_PEP_CMP_0_V0_HISTO_CTRL_HISTO_CTRL_MASK               0x00000003
#define BCHP_PEP_CMP_0_V0_HISTO_CTRL_HISTO_CTRL_SHIFT              0
#define BCHP_PEP_CMP_0_V0_HISTO_CTRL_HISTO_CTRL_DEFAULT            0x00000000
#define BCHP_PEP_CMP_0_V0_HISTO_CTRL_HISTO_CTRL_DISABLE            0
#define BCHP_PEP_CMP_0_V0_HISTO_CTRL_HISTO_CTRL_SINGLE_PICTURE     1
#define BCHP_PEP_CMP_0_V0_HISTO_CTRL_HISTO_CTRL_DOUBLE_PICTURES    2
#define BCHP_PEP_CMP_0_V0_HISTO_CTRL_HISTO_CTRL_RESERVED           3

/***************************************************************************
 *MIN_MAX_CTRL - Luma Histogram Control
 ***************************************************************************/
/* PEP_CMP_0_V0 :: MIN_MAX_CTRL :: reserved0 [31:02] */
#define BCHP_PEP_CMP_0_V0_MIN_MAX_CTRL_reserved0_MASK              0xfffffffc
#define BCHP_PEP_CMP_0_V0_MIN_MAX_CTRL_reserved0_SHIFT             2

/* PEP_CMP_0_V0 :: MIN_MAX_CTRL :: MIN_MAX_CTRL [01:00] */
#define BCHP_PEP_CMP_0_V0_MIN_MAX_CTRL_MIN_MAX_CTRL_MASK           0x00000003
#define BCHP_PEP_CMP_0_V0_MIN_MAX_CTRL_MIN_MAX_CTRL_SHIFT          0
#define BCHP_PEP_CMP_0_V0_MIN_MAX_CTRL_MIN_MAX_CTRL_DEFAULT        0x00000000
#define BCHP_PEP_CMP_0_V0_MIN_MAX_CTRL_MIN_MAX_CTRL_DISABLE        0
#define BCHP_PEP_CMP_0_V0_MIN_MAX_CTRL_MIN_MAX_CTRL_SINGLE_PICTURE 1
#define BCHP_PEP_CMP_0_V0_MIN_MAX_CTRL_MIN_MAX_CTRL_DOUBLE_PICTURES 2
#define BCHP_PEP_CMP_0_V0_MIN_MAX_CTRL_MIN_MAX_CTRL_UNLIMITED_PICTURES 3

/***************************************************************************
 *HISTO_WIN_OFFSET - Histogram Window X and Y Offset
 ***************************************************************************/
/* PEP_CMP_0_V0 :: HISTO_WIN_OFFSET :: reserved0 [31:29] */
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_OFFSET_reserved0_MASK          0xe0000000
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_OFFSET_reserved0_SHIFT         29

/* PEP_CMP_0_V0 :: HISTO_WIN_OFFSET :: X_OFFSET [28:16] */
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_OFFSET_X_OFFSET_MASK           0x1fff0000
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_OFFSET_X_OFFSET_SHIFT          16
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_OFFSET_X_OFFSET_DEFAULT        0x00000000

/* PEP_CMP_0_V0 :: HISTO_WIN_OFFSET :: reserved1 [15:12] */
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_OFFSET_reserved1_MASK          0x0000f000
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_OFFSET_reserved1_SHIFT         12

/* PEP_CMP_0_V0 :: HISTO_WIN_OFFSET :: Y_OFFSET [11:00] */
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_OFFSET_Y_OFFSET_MASK           0x00000fff
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_OFFSET_Y_OFFSET_SHIFT          0
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_OFFSET_Y_OFFSET_DEFAULT        0x00000000

/***************************************************************************
 *HISTO_WIN_SIZE - Histogram Window Vertical and Horizontal Size
 ***************************************************************************/
/* PEP_CMP_0_V0 :: HISTO_WIN_SIZE :: reserved0 [31:29] */
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_SIZE_reserved0_MASK            0xe0000000
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_SIZE_reserved0_SHIFT           29

/* PEP_CMP_0_V0 :: HISTO_WIN_SIZE :: HSIZE [28:16] */
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_SIZE_HSIZE_MASK                0x1fff0000
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_SIZE_HSIZE_SHIFT               16
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_SIZE_HSIZE_DEFAULT             0x00000000

/* PEP_CMP_0_V0 :: HISTO_WIN_SIZE :: reserved1 [15:12] */
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_SIZE_reserved1_MASK            0x0000f000
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_SIZE_reserved1_SHIFT           12

/* PEP_CMP_0_V0 :: HISTO_WIN_SIZE :: VSIZE [11:00] */
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_SIZE_VSIZE_MASK                0x00000fff
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_SIZE_VSIZE_SHIFT               0
#define BCHP_PEP_CMP_0_V0_HISTO_WIN_SIZE_VSIZE_DEFAULT             0x00000000

/***************************************************************************
 *HISTO_MIN_MAX - Histogram Min and Max value
 ***************************************************************************/
/* PEP_CMP_0_V0 :: HISTO_MIN_MAX :: reserved0 [31:26] */
#define BCHP_PEP_CMP_0_V0_HISTO_MIN_MAX_reserved0_MASK             0xfc000000
#define BCHP_PEP_CMP_0_V0_HISTO_MIN_MAX_reserved0_SHIFT            26

/* PEP_CMP_0_V0 :: HISTO_MIN_MAX :: MAX [25:16] */
#define BCHP_PEP_CMP_0_V0_HISTO_MIN_MAX_MAX_MASK                   0x03ff0000
#define BCHP_PEP_CMP_0_V0_HISTO_MIN_MAX_MAX_SHIFT                  16
#define BCHP_PEP_CMP_0_V0_HISTO_MIN_MAX_MAX_DEFAULT                0x00000000

/* PEP_CMP_0_V0 :: HISTO_MIN_MAX :: reserved1 [15:10] */
#define BCHP_PEP_CMP_0_V0_HISTO_MIN_MAX_reserved1_MASK             0x0000fc00
#define BCHP_PEP_CMP_0_V0_HISTO_MIN_MAX_reserved1_SHIFT            10

/* PEP_CMP_0_V0 :: HISTO_MIN_MAX :: MIN [09:00] */
#define BCHP_PEP_CMP_0_V0_HISTO_MIN_MAX_MIN_MASK                   0x000003ff
#define BCHP_PEP_CMP_0_V0_HISTO_MIN_MAX_MIN_SHIFT                  0
#define BCHP_PEP_CMP_0_V0_HISTO_MIN_MAX_MIN_DEFAULT                0x000003ff

/***************************************************************************
 *HISTO_DATA_COUNT_%i - Histogram Data Occurrence Count
 ***************************************************************************/
#define BCHP_PEP_CMP_0_V0_HISTO_DATA_COUNT_i_ARRAY_BASE            0x20646440
#define BCHP_PEP_CMP_0_V0_HISTO_DATA_COUNT_i_ARRAY_START           0
#define BCHP_PEP_CMP_0_V0_HISTO_DATA_COUNT_i_ARRAY_END             15
#define BCHP_PEP_CMP_0_V0_HISTO_DATA_COUNT_i_ARRAY_ELEMENT_SIZE    32

/***************************************************************************
 *HISTO_DATA_COUNT_%i - Histogram Data Occurrence Count
 ***************************************************************************/
/* PEP_CMP_0_V0 :: HISTO_DATA_COUNT_i :: reserved0 [31:25] */
#define BCHP_PEP_CMP_0_V0_HISTO_DATA_COUNT_i_reserved0_MASK        0xfe000000
#define BCHP_PEP_CMP_0_V0_HISTO_DATA_COUNT_i_reserved0_SHIFT       25

/* PEP_CMP_0_V0 :: HISTO_DATA_COUNT_i :: COUNT [24:00] */
#define BCHP_PEP_CMP_0_V0_HISTO_DATA_COUNT_i_COUNT_MASK            0x01ffffff
#define BCHP_PEP_CMP_0_V0_HISTO_DATA_COUNT_i_COUNT_SHIFT           0


/***************************************************************************
 *HISTO_RESET - Luma histogram reset
 ***************************************************************************/
/* PEP_CMP_0_V0 :: HISTO_RESET :: reserved0 [31:01] */
#define BCHP_PEP_CMP_0_V0_HISTO_RESET_reserved0_MASK               0xfffffffe
#define BCHP_PEP_CMP_0_V0_HISTO_RESET_reserved0_SHIFT              1

/* PEP_CMP_0_V0 :: HISTO_RESET :: HISTO_RESET [00:00] */
#define BCHP_PEP_CMP_0_V0_HISTO_RESET_HISTO_RESET_MASK             0x00000001
#define BCHP_PEP_CMP_0_V0_HISTO_RESET_HISTO_RESET_SHIFT            0
#define BCHP_PEP_CMP_0_V0_HISTO_RESET_HISTO_RESET_DEFAULT          0x00000000

/***************************************************************************
 *MIN_MAX_RESET - Luma min/max value reset
 ***************************************************************************/
/* PEP_CMP_0_V0 :: MIN_MAX_RESET :: reserved0 [31:01] */
#define BCHP_PEP_CMP_0_V0_MIN_MAX_RESET_reserved0_MASK             0xfffffffe
#define BCHP_PEP_CMP_0_V0_MIN_MAX_RESET_reserved0_SHIFT            1

/* PEP_CMP_0_V0 :: MIN_MAX_RESET :: MIN_MAX_RESET [00:00] */
#define BCHP_PEP_CMP_0_V0_MIN_MAX_RESET_MIN_MAX_RESET_MASK         0x00000001
#define BCHP_PEP_CMP_0_V0_MIN_MAX_RESET_MIN_MAX_RESET_SHIFT        0
#define BCHP_PEP_CMP_0_V0_MIN_MAX_RESET_MIN_MAX_RESET_DEFAULT      0x00000000

/***************************************************************************
 *CAB_LUT_DATA_%i - CAB Lookup Table Data
 ***************************************************************************/
#define BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_ARRAY_BASE                0x20646488
#define BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_ARRAY_START               0
#define BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_ARRAY_END                 1023
#define BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *CAB_LUT_DATA_%i - CAB Lookup Table Data
 ***************************************************************************/
/* PEP_CMP_0_V0 :: CAB_LUT_DATA_i :: reserved0 [31:21] */
#define BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_reserved0_MASK            0xffe00000
#define BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_reserved0_SHIFT           21

/* PEP_CMP_0_V0 :: CAB_LUT_DATA_i :: BYPASS [20:20] */
#define BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_BYPASS_MASK               0x00100000
#define BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_BYPASS_SHIFT              20

/* PEP_CMP_0_V0 :: CAB_LUT_DATA_i :: CR_DATA [19:10] */
#define BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_CR_DATA_MASK              0x000ffc00
#define BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_CR_DATA_SHIFT             10

/* PEP_CMP_0_V0 :: CAB_LUT_DATA_i :: CB_DATA [09:00] */
#define BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_CB_DATA_MASK              0x000003ff
#define BCHP_PEP_CMP_0_V0_CAB_LUT_DATA_i_CB_DATA_SHIFT             0


/***************************************************************************
 *LAB_LUT_DATA_%i - LAB Lookup Table Data
 ***************************************************************************/
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_ARRAY_BASE                0x20647488
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_ARRAY_START               0
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_ARRAY_END                 127
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_ARRAY_ELEMENT_SIZE        32

/***************************************************************************
 *LAB_LUT_DATA_%i - LAB Lookup Table Data
 ***************************************************************************/
/* PEP_CMP_0_V0 :: LAB_LUT_DATA_i :: reserved0 [31:20] */
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_reserved0_MASK            0xfff00000
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_reserved0_SHIFT           20

/* PEP_CMP_0_V0 :: LAB_LUT_DATA_i :: LUMA_BYPASS [19:19] */
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_LUMA_BYPASS_MASK          0x00080000
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_LUMA_BYPASS_SHIFT         19

/* PEP_CMP_0_V0 :: LAB_LUT_DATA_i :: SAT_BYPASS [18:18] */
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_SAT_BYPASS_MASK           0x00040000
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_SAT_BYPASS_SHIFT          18

/* PEP_CMP_0_V0 :: LAB_LUT_DATA_i :: LUMA_DATA [17:08] */
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_LUMA_DATA_MASK            0x0003ff00
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_LUMA_DATA_SHIFT           8

/* PEP_CMP_0_V0 :: LAB_LUT_DATA_i :: SAT_ADJ [07:00] */
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_SAT_ADJ_MASK              0x000000ff
#define BCHP_PEP_CMP_0_V0_LAB_LUT_DATA_i_SAT_ADJ_SHIFT             0


#endif /* #ifndef BCHP_PEP_CMP_0_V0_H__ */

/* End of File */
