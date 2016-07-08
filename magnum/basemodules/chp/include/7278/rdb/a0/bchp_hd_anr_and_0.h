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
 * Date:           Generated on               Mon Mar 21 13:44:46 2016
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

#ifndef BCHP_HD_ANR_AND_0_H__
#define BCHP_HD_ANR_AND_0_H__

/***************************************************************************
 *HD_ANR_AND_0 - MCVP AND Registers
 ***************************************************************************/
#define BCHP_HD_ANR_AND_0_REVISION_ID            0x00089800 /* [RO] AND Revision register */
#define BCHP_HD_ANR_AND_0_SW_RESET               0x00089804 /* [WO] AND Software Reset */
#define BCHP_HD_ANR_AND_0_AND_ENABLE             0x00089808 /* [CFG] AND Enable */
#define BCHP_HD_ANR_AND_0_AND_MODE               0x0008980c /* [CFG] AND Window Size */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_0       0x00089810 /* [CFG] Noise Range Lower Threshold 0 */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_1       0x00089814 /* [CFG] Noise Range Lower Threshold 1 */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_2       0x00089818 /* [CFG] Noise Range Lower Threshold 2 */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_3       0x0008981c /* [CFG] Noise Range Lower Threshold 3 */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_4       0x00089820 /* [CFG] Noise Range Lower Threshold 4 */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_0       0x00089824 /* [CFG] Noise Range upper Threshold 0 */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_1       0x00089828 /* [CFG] Noise Range upper Threshold 1 */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_2       0x0008982c /* [CFG] Noise Range upper Threshold 2 */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_3       0x00089830 /* [CFG] Noise Range upper Threshold 3 */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_4       0x00089834 /* [CFG] Noise Range upper Threshold 4 */
#define BCHP_HD_ANR_AND_0_EDGE_TH                0x00089838 /* [CFG] Edge gradient threshold */
#define BCHP_HD_ANR_AND_0_CONTENT_TH             0x0008983c /* [CFG] Content level threshold */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_0 0x00089840 /* [RO] Number of noisy pixel in frame for noise level category 0 */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_1 0x00089844 /* [RO] Number of noisy pixel in frame for noise level category 1 */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_2 0x00089848 /* [RO] Number of noisy pixel in frame for noise level category 2 */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_3 0x0008984c /* [RO] Number of noisy pixel in frame for noise level category 3 */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_4 0x00089850 /* [RO] Number of noisy pixel in frame for noise level category 4 */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_MSB      0x00089854 /* [RO] The means result of noise category 0 (MSB) */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_LSB      0x00089858 /* [RO] The means result of noise category 0 (LSB) */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_1_MSB      0x0008985c /* [RO] The means result of noise category 1 (MSB) */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_1_LSB      0x00089860 /* [RO] The means result of noise category 1 (LSB) */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_2_MSB      0x00089864 /* [RO] The means result of noise category 2 (MSB) */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_2_LSB      0x00089868 /* [RO] The means result of noise category 2 (LSB) */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_3_MSB      0x0008986c /* [RO] The means result of noise category 3 (MSB) */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_3_LSB      0x00089870 /* [RO] The means result of noise category 3 (LSB) */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_4_MSB      0x00089874 /* [RO] The means result of noise category 4 (MSB) */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_4_LSB      0x00089878 /* [RO] The means result of noise category 4 (LSB) */
#define BCHP_HD_ANR_AND_0_X_LOWER_0              0x0008987c /* [CFG] The lower boundary for statistics collection at horizontal direction */
#define BCHP_HD_ANR_AND_0_X_UPPER_0              0x00089880 /* [CFG] The upper boundary for statistics collection at horizontal direction */
#define BCHP_HD_ANR_AND_0_Y_LOWER_0              0x00089884 /* [CFG] The lower boundary for statistics collection at vertical direction */
#define BCHP_HD_ANR_AND_0_Y_UPPER_0              0x00089888 /* [CFG] The upper boundary for statistics collection at vertical direction */

/***************************************************************************
 *REVISION_ID - AND Revision register
 ***************************************************************************/
/* HD_ANR_AND_0 :: REVISION_ID :: reserved0 [31:16] */
#define BCHP_HD_ANR_AND_0_REVISION_ID_reserved0_MASK               0xffff0000
#define BCHP_HD_ANR_AND_0_REVISION_ID_reserved0_SHIFT              16

/* HD_ANR_AND_0 :: REVISION_ID :: MAJOR [15:08] */
#define BCHP_HD_ANR_AND_0_REVISION_ID_MAJOR_MASK                   0x0000ff00
#define BCHP_HD_ANR_AND_0_REVISION_ID_MAJOR_SHIFT                  8
#define BCHP_HD_ANR_AND_0_REVISION_ID_MAJOR_DEFAULT                0x00000003

/* HD_ANR_AND_0 :: REVISION_ID :: MINOR [07:00] */
#define BCHP_HD_ANR_AND_0_REVISION_ID_MINOR_MASK                   0x000000ff
#define BCHP_HD_ANR_AND_0_REVISION_ID_MINOR_SHIFT                  0
#define BCHP_HD_ANR_AND_0_REVISION_ID_MINOR_DEFAULT                0x00000010

/***************************************************************************
 *SW_RESET - AND Software Reset
 ***************************************************************************/
/* HD_ANR_AND_0 :: SW_RESET :: reserved0 [31:01] */
#define BCHP_HD_ANR_AND_0_SW_RESET_reserved0_MASK                  0xfffffffe
#define BCHP_HD_ANR_AND_0_SW_RESET_reserved0_SHIFT                 1

/* HD_ANR_AND_0 :: SW_RESET :: SW_RESET [00:00] */
#define BCHP_HD_ANR_AND_0_SW_RESET_SW_RESET_MASK                   0x00000001
#define BCHP_HD_ANR_AND_0_SW_RESET_SW_RESET_SHIFT                  0
#define BCHP_HD_ANR_AND_0_SW_RESET_SW_RESET_DEFAULT                0x00000000

/***************************************************************************
 *AND_ENABLE - AND Enable
 ***************************************************************************/
/* HD_ANR_AND_0 :: AND_ENABLE :: reserved0 [31:02] */
#define BCHP_HD_ANR_AND_0_AND_ENABLE_reserved0_MASK                0xfffffffc
#define BCHP_HD_ANR_AND_0_AND_ENABLE_reserved0_SHIFT               2

/* HD_ANR_AND_0 :: AND_ENABLE :: AND_DRAIN [01:01] */
#define BCHP_HD_ANR_AND_0_AND_ENABLE_AND_DRAIN_MASK                0x00000002
#define BCHP_HD_ANR_AND_0_AND_ENABLE_AND_DRAIN_SHIFT               1
#define BCHP_HD_ANR_AND_0_AND_ENABLE_AND_DRAIN_DEFAULT             0x00000000

/* HD_ANR_AND_0 :: AND_ENABLE :: AND_ENABLE [00:00] */
#define BCHP_HD_ANR_AND_0_AND_ENABLE_AND_ENABLE_MASK               0x00000001
#define BCHP_HD_ANR_AND_0_AND_ENABLE_AND_ENABLE_SHIFT              0
#define BCHP_HD_ANR_AND_0_AND_ENABLE_AND_ENABLE_DEFAULT            0x00000000

/***************************************************************************
 *AND_MODE - AND Window Size
 ***************************************************************************/
/* HD_ANR_AND_0 :: AND_MODE :: reserved0 [31:02] */
#define BCHP_HD_ANR_AND_0_AND_MODE_reserved0_MASK                  0xfffffffc
#define BCHP_HD_ANR_AND_0_AND_MODE_reserved0_SHIFT                 2

/* HD_ANR_AND_0 :: AND_MODE :: AND_WIN_SIZE [01:00] */
#define BCHP_HD_ANR_AND_0_AND_MODE_AND_WIN_SIZE_MASK               0x00000003
#define BCHP_HD_ANR_AND_0_AND_MODE_AND_WIN_SIZE_SHIFT              0
#define BCHP_HD_ANR_AND_0_AND_MODE_AND_WIN_SIZE_DEFAULT            0x00000001
#define BCHP_HD_ANR_AND_0_AND_MODE_AND_WIN_SIZE_WIN_SIZE_3X3       0
#define BCHP_HD_ANR_AND_0_AND_MODE_AND_WIN_SIZE_WIN_SIZE_5X3       1
#define BCHP_HD_ANR_AND_0_AND_MODE_AND_WIN_SIZE_WIN_SIZE_7X3       2

/***************************************************************************
 *NOISE_LOWER_TH_0 - Noise Range Lower Threshold 0
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LOWER_TH_0 :: reserved0 [31:14] */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_0_reserved0_MASK          0xffffc000
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_0_reserved0_SHIFT         14

/* HD_ANR_AND_0 :: NOISE_LOWER_TH_0 :: NOISE_LTH [13:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_0_NOISE_LTH_MASK          0x00003fff
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_0_NOISE_LTH_SHIFT         0
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_0_NOISE_LTH_DEFAULT       0x00000000

/***************************************************************************
 *NOISE_LOWER_TH_1 - Noise Range Lower Threshold 1
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LOWER_TH_1 :: reserved0 [31:14] */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_1_reserved0_MASK          0xffffc000
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_1_reserved0_SHIFT         14

/* HD_ANR_AND_0 :: NOISE_LOWER_TH_1 :: NOISE_LTH [13:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_1_NOISE_LTH_MASK          0x00003fff
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_1_NOISE_LTH_SHIFT         0
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_1_NOISE_LTH_DEFAULT       0x00000069

/***************************************************************************
 *NOISE_LOWER_TH_2 - Noise Range Lower Threshold 2
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LOWER_TH_2 :: reserved0 [31:14] */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_2_reserved0_MASK          0xffffc000
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_2_reserved0_SHIFT         14

/* HD_ANR_AND_0 :: NOISE_LOWER_TH_2 :: NOISE_LTH [13:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_2_NOISE_LTH_MASK          0x00003fff
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_2_NOISE_LTH_SHIFT         0
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_2_NOISE_LTH_DEFAULT       0x000000d1

/***************************************************************************
 *NOISE_LOWER_TH_3 - Noise Range Lower Threshold 3
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LOWER_TH_3 :: reserved0 [31:14] */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_3_reserved0_MASK          0xffffc000
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_3_reserved0_SHIFT         14

/* HD_ANR_AND_0 :: NOISE_LOWER_TH_3 :: NOISE_LTH [13:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_3_NOISE_LTH_MASK          0x00003fff
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_3_NOISE_LTH_SHIFT         0
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_3_NOISE_LTH_DEFAULT       0x000001a2

/***************************************************************************
 *NOISE_LOWER_TH_4 - Noise Range Lower Threshold 4
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LOWER_TH_4 :: reserved0 [31:14] */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_4_reserved0_MASK          0xffffc000
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_4_reserved0_SHIFT         14

/* HD_ANR_AND_0 :: NOISE_LOWER_TH_4 :: NOISE_LTH [13:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_4_NOISE_LTH_MASK          0x00003fff
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_4_NOISE_LTH_SHIFT         0
#define BCHP_HD_ANR_AND_0_NOISE_LOWER_TH_4_NOISE_LTH_DEFAULT       0x00000342

/***************************************************************************
 *NOISE_UPPER_TH_0 - Noise Range upper Threshold 0
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_UPPER_TH_0 :: reserved0 [31:14] */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_0_reserved0_MASK          0xffffc000
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_0_reserved0_SHIFT         14

/* HD_ANR_AND_0 :: NOISE_UPPER_TH_0 :: NOISE_UTH [13:00] */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_0_NOISE_UTH_MASK          0x00003fff
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_0_NOISE_UTH_SHIFT         0
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_0_NOISE_UTH_DEFAULT       0x000000dd

/***************************************************************************
 *NOISE_UPPER_TH_1 - Noise Range upper Threshold 1
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_UPPER_TH_1 :: reserved0 [31:14] */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_1_reserved0_MASK          0xffffc000
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_1_reserved0_SHIFT         14

/* HD_ANR_AND_0 :: NOISE_UPPER_TH_1 :: NOISE_UTH [13:00] */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_1_NOISE_UTH_MASK          0x00003fff
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_1_NOISE_UTH_SHIFT         0
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_1_NOISE_UTH_DEFAULT       0x000001bf

/***************************************************************************
 *NOISE_UPPER_TH_2 - Noise Range upper Threshold 2
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_UPPER_TH_2 :: reserved0 [31:14] */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_2_reserved0_MASK          0xffffc000
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_2_reserved0_SHIFT         14

/* HD_ANR_AND_0 :: NOISE_UPPER_TH_2 :: NOISE_UTH [13:00] */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_2_NOISE_UTH_MASK          0x00003fff
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_2_NOISE_UTH_SHIFT         0
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_2_NOISE_UTH_DEFAULT       0x0000037c

/***************************************************************************
 *NOISE_UPPER_TH_3 - Noise Range upper Threshold 3
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_UPPER_TH_3 :: reserved0 [31:14] */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_3_reserved0_MASK          0xffffc000
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_3_reserved0_SHIFT         14

/* HD_ANR_AND_0 :: NOISE_UPPER_TH_3 :: NOISE_UTH [13:00] */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_3_NOISE_UTH_MASK          0x00003fff
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_3_NOISE_UTH_SHIFT         0
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_3_NOISE_UTH_DEFAULT       0x000006f5

/***************************************************************************
 *NOISE_UPPER_TH_4 - Noise Range upper Threshold 4
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_UPPER_TH_4 :: reserved0 [31:14] */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_4_reserved0_MASK          0xffffc000
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_4_reserved0_SHIFT         14

/* HD_ANR_AND_0 :: NOISE_UPPER_TH_4 :: NOISE_UTH [13:00] */
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_4_NOISE_UTH_MASK          0x00003fff
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_4_NOISE_UTH_SHIFT         0
#define BCHP_HD_ANR_AND_0_NOISE_UPPER_TH_4_NOISE_UTH_DEFAULT       0x00000de0

/***************************************************************************
 *EDGE_TH - Edge gradient threshold
 ***************************************************************************/
/* HD_ANR_AND_0 :: EDGE_TH :: reserved0 [31:10] */
#define BCHP_HD_ANR_AND_0_EDGE_TH_reserved0_MASK                   0xfffffc00
#define BCHP_HD_ANR_AND_0_EDGE_TH_reserved0_SHIFT                  10

/* HD_ANR_AND_0 :: EDGE_TH :: EDGE_TH [09:00] */
#define BCHP_HD_ANR_AND_0_EDGE_TH_EDGE_TH_MASK                     0x000003ff
#define BCHP_HD_ANR_AND_0_EDGE_TH_EDGE_TH_SHIFT                    0
#define BCHP_HD_ANR_AND_0_EDGE_TH_EDGE_TH_DEFAULT                  0x0000003c

/***************************************************************************
 *CONTENT_TH - Content level threshold
 ***************************************************************************/
/* HD_ANR_AND_0 :: CONTENT_TH :: reserved0 [31:10] */
#define BCHP_HD_ANR_AND_0_CONTENT_TH_reserved0_MASK                0xfffffc00
#define BCHP_HD_ANR_AND_0_CONTENT_TH_reserved0_SHIFT               10

/* HD_ANR_AND_0 :: CONTENT_TH :: CONTENT_TH [09:00] */
#define BCHP_HD_ANR_AND_0_CONTENT_TH_CONTENT_TH_MASK               0x000003ff
#define BCHP_HD_ANR_AND_0_CONTENT_TH_CONTENT_TH_SHIFT              0
#define BCHP_HD_ANR_AND_0_CONTENT_TH_CONTENT_TH_DEFAULT            0x000000a8

/***************************************************************************
 *NOISY_SAMPLE_NUM_BIN_0 - Number of noisy pixel in frame for noise level category 0
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISY_SAMPLE_NUM_BIN_0 :: reserved0 [31:23] */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_0_reserved0_MASK    0xff800000
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_0_reserved0_SHIFT   23

/* HD_ANR_AND_0 :: NOISY_SAMPLE_NUM_BIN_0 :: noisy_sample_num_bin [22:00] */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_0_noisy_sample_num_bin_MASK 0x007fffff
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_0_noisy_sample_num_bin_SHIFT 0
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_0_noisy_sample_num_bin_DEFAULT 0x00000000

/***************************************************************************
 *NOISY_SAMPLE_NUM_BIN_1 - Number of noisy pixel in frame for noise level category 1
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISY_SAMPLE_NUM_BIN_1 :: reserved0 [31:23] */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_1_reserved0_MASK    0xff800000
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_1_reserved0_SHIFT   23

/* HD_ANR_AND_0 :: NOISY_SAMPLE_NUM_BIN_1 :: noisy_sample_num_bin [22:00] */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_1_noisy_sample_num_bin_MASK 0x007fffff
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_1_noisy_sample_num_bin_SHIFT 0
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_1_noisy_sample_num_bin_DEFAULT 0x00000000

/***************************************************************************
 *NOISY_SAMPLE_NUM_BIN_2 - Number of noisy pixel in frame for noise level category 2
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISY_SAMPLE_NUM_BIN_2 :: reserved0 [31:23] */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_2_reserved0_MASK    0xff800000
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_2_reserved0_SHIFT   23

/* HD_ANR_AND_0 :: NOISY_SAMPLE_NUM_BIN_2 :: noisy_sample_num_bin [22:00] */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_2_noisy_sample_num_bin_MASK 0x007fffff
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_2_noisy_sample_num_bin_SHIFT 0
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_2_noisy_sample_num_bin_DEFAULT 0x00000000

/***************************************************************************
 *NOISY_SAMPLE_NUM_BIN_3 - Number of noisy pixel in frame for noise level category 3
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISY_SAMPLE_NUM_BIN_3 :: reserved0 [31:23] */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_3_reserved0_MASK    0xff800000
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_3_reserved0_SHIFT   23

/* HD_ANR_AND_0 :: NOISY_SAMPLE_NUM_BIN_3 :: noisy_sample_num_bin [22:00] */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_3_noisy_sample_num_bin_MASK 0x007fffff
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_3_noisy_sample_num_bin_SHIFT 0
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_3_noisy_sample_num_bin_DEFAULT 0x00000000

/***************************************************************************
 *NOISY_SAMPLE_NUM_BIN_4 - Number of noisy pixel in frame for noise level category 4
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISY_SAMPLE_NUM_BIN_4 :: reserved0 [31:23] */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_4_reserved0_MASK    0xff800000
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_4_reserved0_SHIFT   23

/* HD_ANR_AND_0 :: NOISY_SAMPLE_NUM_BIN_4 :: noisy_sample_num_bin [22:00] */
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_4_noisy_sample_num_bin_MASK 0x007fffff
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_4_noisy_sample_num_bin_SHIFT 0
#define BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_4_noisy_sample_num_bin_DEFAULT 0x00000000

/***************************************************************************
 *NOISE_LEVEL_0_MSB - The means result of noise category 0 (MSB)
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LEVEL_0_MSB :: reserved0 [31:07] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_MSB_reserved0_MASK         0xffffff80
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_MSB_reserved0_SHIFT        7

/* HD_ANR_AND_0 :: NOISE_LEVEL_0_MSB :: noise_level_msb [06:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_MSB_noise_level_msb_MASK   0x0000007f
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_MSB_noise_level_msb_SHIFT  0
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_MSB_noise_level_msb_DEFAULT 0x00000000

/***************************************************************************
 *NOISE_LEVEL_0_LSB - The means result of noise category 0 (LSB)
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LEVEL_0_LSB :: noise_level_lsb [31:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_LSB_noise_level_lsb_MASK   0xffffffff
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_LSB_noise_level_lsb_SHIFT  0
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_LSB_noise_level_lsb_DEFAULT 0x00000000

/***************************************************************************
 *NOISE_LEVEL_1_MSB - The means result of noise category 1 (MSB)
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LEVEL_1_MSB :: reserved0 [31:07] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_1_MSB_reserved0_MASK         0xffffff80
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_1_MSB_reserved0_SHIFT        7

/* HD_ANR_AND_0 :: NOISE_LEVEL_1_MSB :: noise_level_msb [06:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_1_MSB_noise_level_msb_MASK   0x0000007f
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_1_MSB_noise_level_msb_SHIFT  0
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_1_MSB_noise_level_msb_DEFAULT 0x00000000

/***************************************************************************
 *NOISE_LEVEL_1_LSB - The means result of noise category 1 (LSB)
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LEVEL_1_LSB :: noise_level_lsb [31:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_1_LSB_noise_level_lsb_MASK   0xffffffff
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_1_LSB_noise_level_lsb_SHIFT  0
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_1_LSB_noise_level_lsb_DEFAULT 0x00000000

/***************************************************************************
 *NOISE_LEVEL_2_MSB - The means result of noise category 2 (MSB)
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LEVEL_2_MSB :: reserved0 [31:07] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_2_MSB_reserved0_MASK         0xffffff80
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_2_MSB_reserved0_SHIFT        7

/* HD_ANR_AND_0 :: NOISE_LEVEL_2_MSB :: noise_level_msb [06:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_2_MSB_noise_level_msb_MASK   0x0000007f
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_2_MSB_noise_level_msb_SHIFT  0
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_2_MSB_noise_level_msb_DEFAULT 0x00000000

/***************************************************************************
 *NOISE_LEVEL_2_LSB - The means result of noise category 2 (LSB)
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LEVEL_2_LSB :: noise_level_lsb [31:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_2_LSB_noise_level_lsb_MASK   0xffffffff
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_2_LSB_noise_level_lsb_SHIFT  0
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_2_LSB_noise_level_lsb_DEFAULT 0x00000000

/***************************************************************************
 *NOISE_LEVEL_3_MSB - The means result of noise category 3 (MSB)
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LEVEL_3_MSB :: reserved0 [31:07] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_3_MSB_reserved0_MASK         0xffffff80
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_3_MSB_reserved0_SHIFT        7

/* HD_ANR_AND_0 :: NOISE_LEVEL_3_MSB :: noise_level_msb [06:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_3_MSB_noise_level_msb_MASK   0x0000007f
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_3_MSB_noise_level_msb_SHIFT  0
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_3_MSB_noise_level_msb_DEFAULT 0x00000000

/***************************************************************************
 *NOISE_LEVEL_3_LSB - The means result of noise category 3 (LSB)
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LEVEL_3_LSB :: noise_level_lsb [31:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_3_LSB_noise_level_lsb_MASK   0xffffffff
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_3_LSB_noise_level_lsb_SHIFT  0
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_3_LSB_noise_level_lsb_DEFAULT 0x00000000

/***************************************************************************
 *NOISE_LEVEL_4_MSB - The means result of noise category 4 (MSB)
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LEVEL_4_MSB :: reserved0 [31:07] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_4_MSB_reserved0_MASK         0xffffff80
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_4_MSB_reserved0_SHIFT        7

/* HD_ANR_AND_0 :: NOISE_LEVEL_4_MSB :: noise_level_msb [06:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_4_MSB_noise_level_msb_MASK   0x0000007f
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_4_MSB_noise_level_msb_SHIFT  0
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_4_MSB_noise_level_msb_DEFAULT 0x00000000

/***************************************************************************
 *NOISE_LEVEL_4_LSB - The means result of noise category 4 (LSB)
 ***************************************************************************/
/* HD_ANR_AND_0 :: NOISE_LEVEL_4_LSB :: noise_level_lsb [31:00] */
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_4_LSB_noise_level_lsb_MASK   0xffffffff
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_4_LSB_noise_level_lsb_SHIFT  0
#define BCHP_HD_ANR_AND_0_NOISE_LEVEL_4_LSB_noise_level_lsb_DEFAULT 0x00000000

/***************************************************************************
 *X_LOWER_0 - The lower boundary for statistics collection at horizontal direction
 ***************************************************************************/
/* HD_ANR_AND_0 :: X_LOWER_0 :: reserved0 [31:11] */
#define BCHP_HD_ANR_AND_0_X_LOWER_0_reserved0_MASK                 0xfffff800
#define BCHP_HD_ANR_AND_0_X_LOWER_0_reserved0_SHIFT                11

/* HD_ANR_AND_0 :: X_LOWER_0 :: X_LOWER [10:00] */
#define BCHP_HD_ANR_AND_0_X_LOWER_0_X_LOWER_MASK                   0x000007ff
#define BCHP_HD_ANR_AND_0_X_LOWER_0_X_LOWER_SHIFT                  0
#define BCHP_HD_ANR_AND_0_X_LOWER_0_X_LOWER_DEFAULT                0x00000000

/***************************************************************************
 *X_UPPER_0 - The upper boundary for statistics collection at horizontal direction
 ***************************************************************************/
/* HD_ANR_AND_0 :: X_UPPER_0 :: reserved0 [31:11] */
#define BCHP_HD_ANR_AND_0_X_UPPER_0_reserved0_MASK                 0xfffff800
#define BCHP_HD_ANR_AND_0_X_UPPER_0_reserved0_SHIFT                11

/* HD_ANR_AND_0 :: X_UPPER_0 :: X_UPPER [10:00] */
#define BCHP_HD_ANR_AND_0_X_UPPER_0_X_UPPER_MASK                   0x000007ff
#define BCHP_HD_ANR_AND_0_X_UPPER_0_X_UPPER_SHIFT                  0
#define BCHP_HD_ANR_AND_0_X_UPPER_0_X_UPPER_DEFAULT                0x00000780

/***************************************************************************
 *Y_LOWER_0 - The lower boundary for statistics collection at vertical direction
 ***************************************************************************/
/* HD_ANR_AND_0 :: Y_LOWER_0 :: reserved0 [31:12] */
#define BCHP_HD_ANR_AND_0_Y_LOWER_0_reserved0_MASK                 0xfffff000
#define BCHP_HD_ANR_AND_0_Y_LOWER_0_reserved0_SHIFT                12

/* HD_ANR_AND_0 :: Y_LOWER_0 :: Y_LOWER [11:00] */
#define BCHP_HD_ANR_AND_0_Y_LOWER_0_Y_LOWER_MASK                   0x00000fff
#define BCHP_HD_ANR_AND_0_Y_LOWER_0_Y_LOWER_SHIFT                  0
#define BCHP_HD_ANR_AND_0_Y_LOWER_0_Y_LOWER_DEFAULT                0x00000000

/***************************************************************************
 *Y_UPPER_0 - The upper boundary for statistics collection at vertical direction
 ***************************************************************************/
/* HD_ANR_AND_0 :: Y_UPPER_0 :: reserved0 [31:12] */
#define BCHP_HD_ANR_AND_0_Y_UPPER_0_reserved0_MASK                 0xfffff000
#define BCHP_HD_ANR_AND_0_Y_UPPER_0_reserved0_SHIFT                12

/* HD_ANR_AND_0 :: Y_UPPER_0 :: Y_UPPER [11:00] */
#define BCHP_HD_ANR_AND_0_Y_UPPER_0_Y_UPPER_MASK                   0x00000fff
#define BCHP_HD_ANR_AND_0_Y_UPPER_0_Y_UPPER_SHIFT                  0
#define BCHP_HD_ANR_AND_0_Y_UPPER_0_Y_UPPER_DEFAULT                0x00000438

#endif /* #ifndef BCHP_HD_ANR_AND_0_H__ */

/* End of File */
