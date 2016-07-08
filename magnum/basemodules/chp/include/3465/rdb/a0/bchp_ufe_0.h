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
 * Date:           Generated on               Fri Apr  8 10:05:33 2016
 *                 Full Compile MD5 Checksum  fedc0c202f37d89a0f031183f2971dd5
 *                     (minus title and desc)
 *                 MD5 Checksum               46ee8b9f88da54e2e54f3751e0c3b177
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     899
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_UFE_0_H__
#define BCHP_UFE_0_H__

/***************************************************************************
 *UFE_0 - UFE core registers
 ***************************************************************************/
#define BCHP_UFE_0_CTRL                          0x04000800 /* [RW] clock/misc control register */
#define BCHP_UFE_0_BYP                           0x04000804 /* [RW] bypass register */
#define BCHP_UFE_0_RST                           0x04000808 /* [RW] reset control register */
#define BCHP_UFE_0_FRZ                           0x0400080c /* [RW] freeze control register */
#define BCHP_UFE_0_AGC1                          0x04000810 /* [RW] AGC1 control register */
#define BCHP_UFE_0_AGC2                          0x04000814 /* [RW] AGC2 control register */
#define BCHP_UFE_0_AGC3                          0x04000818 /* [RW] AGC3 control register */
#define BCHP_UFE_0_AGC1_THRESH                   0x0400081c /* [RW] AGC1 threshold register */
#define BCHP_UFE_0_AGC2_THRESH                   0x04000820 /* [RW] AGC2 threshold register */
#define BCHP_UFE_0_AGC3_THRESH                   0x04000824 /* [RW] AGC3 threshold register */
#define BCHP_UFE_0_AGC1_LF                       0x04000828 /* [RW] AGC1 loop filter register */
#define BCHP_UFE_0_AGC2_LF                       0x0400082c /* [RW] AGC2 loop filter register */
#define BCHP_UFE_0_AGC3_LF                       0x04000830 /* [RW] AGC3 loop filter register */
#define BCHP_UFE_0_IQIMB_AMP_CTRL                0x04000834 /* [RW] IQ-Imbalance amplitude correction control register */
#define BCHP_UFE_0_IQIMB_PHS_CTRL                0x04000838 /* [RW] IQ-Imbalance phase     correction control register */
#define BCHP_UFE_0_IQIMB_AMP_LF                  0x0400083c /* [RW] IQ-Imbalance amplitude loop filter register */
#define BCHP_UFE_0_IQIMB_PHS_LF                  0x04000840 /* [RW] IQ-Imbalance phase     loop filter register */
#define BCHP_UFE_0_DCO_CTRL                      0x04000844 /* [RW] DCO canceller control register */
#define BCHP_UFE_0_DCOINTI                       0x04000848 /* [RW] DCO integrator I */
#define BCHP_UFE_0_DCOINTQ                       0x0400084c /* [RW] DCO integrator Q */
#define BCHP_UFE_0_BMIX_FCW                      0x04000850 /* [RW] FCW register for back mixer */
#define BCHP_UFE_0_FMIX_FCW                      0x04000854 /* [RW] FCW register for front mixer */
#define BCHP_UFE_0_CRC_EN                        0x04000858 /* [RW] CRC enable register */
#define BCHP_UFE_0_CRC                           0x0400085c /* [RO] CRC signature analyzer register */
#define BCHP_UFE_0_LFSR_SEED                     0x04000860 /* [RW] LFSR initial seed */
#define BCHP_UFE_0_TP                            0x04000864 /* [RW] Testport register */
#define BCHP_UFE_0_SPARE                         0x04000868 /* [RW] Software spare register */

/***************************************************************************
 *CTRL - clock/misc control register
 ***************************************************************************/
/* UFE_0 :: CTRL :: INPUT_FMT [31:31] */
#define BCHP_UFE_0_CTRL_INPUT_FMT_MASK                             0x80000000
#define BCHP_UFE_0_CTRL_INPUT_FMT_SHIFT                            31
#define BCHP_UFE_0_CTRL_INPUT_FMT_DEFAULT                          0x00000000

/* UFE_0 :: CTRL :: INPUT_EDGE [30:30] */
#define BCHP_UFE_0_CTRL_INPUT_EDGE_MASK                            0x40000000
#define BCHP_UFE_0_CTRL_INPUT_EDGE_SHIFT                           30
#define BCHP_UFE_0_CTRL_INPUT_EDGE_DEFAULT                         0x00000001

/* UFE_0 :: CTRL :: IQ_SWAP [29:29] */
#define BCHP_UFE_0_CTRL_IQ_SWAP_MASK                               0x20000000
#define BCHP_UFE_0_CTRL_IQ_SWAP_SHIFT                              29
#define BCHP_UFE_0_CTRL_IQ_SWAP_DEFAULT                            0x00000000

/* UFE_0 :: CTRL :: ZERO_Q [28:28] */
#define BCHP_UFE_0_CTRL_ZERO_Q_MASK                                0x10000000
#define BCHP_UFE_0_CTRL_ZERO_Q_SHIFT                               28
#define BCHP_UFE_0_CTRL_ZERO_Q_DEFAULT                             0x00000000

/* UFE_0 :: CTRL :: NEGATE_I [27:27] */
#define BCHP_UFE_0_CTRL_NEGATE_I_MASK                              0x08000000
#define BCHP_UFE_0_CTRL_NEGATE_I_SHIFT                             27
#define BCHP_UFE_0_CTRL_NEGATE_I_DEFAULT                           0x00000000

/* UFE_0 :: CTRL :: NEGATE_Q [26:26] */
#define BCHP_UFE_0_CTRL_NEGATE_Q_MASK                              0x04000000
#define BCHP_UFE_0_CTRL_NEGATE_Q_SHIFT                             26
#define BCHP_UFE_0_CTRL_NEGATE_Q_DEFAULT                           0x00000000

/* UFE_0 :: CTRL :: SPINV_FRONT [25:25] */
#define BCHP_UFE_0_CTRL_SPINV_FRONT_MASK                           0x02000000
#define BCHP_UFE_0_CTRL_SPINV_FRONT_SHIFT                          25
#define BCHP_UFE_0_CTRL_SPINV_FRONT_DEFAULT                        0x00000000

/* UFE_0 :: CTRL :: SPINV_BACK [24:24] */
#define BCHP_UFE_0_CTRL_SPINV_BACK_MASK                            0x01000000
#define BCHP_UFE_0_CTRL_SPINV_BACK_SHIFT                           24
#define BCHP_UFE_0_CTRL_SPINV_BACK_DEFAULT                         0x00000000

/* UFE_0 :: CTRL :: LO_IF [23:23] */
#define BCHP_UFE_0_CTRL_LO_IF_MASK                                 0x00800000
#define BCHP_UFE_0_CTRL_LO_IF_SHIFT                                23
#define BCHP_UFE_0_CTRL_LO_IF_DEFAULT                              0x00000000

/* UFE_0 :: CTRL :: ZERO_I [22:22] */
#define BCHP_UFE_0_CTRL_ZERO_I_MASK                                0x00400000
#define BCHP_UFE_0_CTRL_ZERO_I_SHIFT                               22
#define BCHP_UFE_0_CTRL_ZERO_I_DEFAULT                             0x00000000

/* UFE_0 :: CTRL :: reserved0 [21:21] */
#define BCHP_UFE_0_CTRL_reserved0_MASK                             0x00200000
#define BCHP_UFE_0_CTRL_reserved0_SHIFT                            21

/* UFE_0 :: CTRL :: VID_QUANT [20:18] */
#define BCHP_UFE_0_CTRL_VID_QUANT_MASK                             0x001c0000
#define BCHP_UFE_0_CTRL_VID_QUANT_SHIFT                            18
#define BCHP_UFE_0_CTRL_VID_QUANT_DEFAULT                          0x00000006

/* UFE_0 :: CTRL :: CIC_DEC_RATIO [17:16] */
#define BCHP_UFE_0_CTRL_CIC_DEC_RATIO_MASK                         0x00030000
#define BCHP_UFE_0_CTRL_CIC_DEC_RATIO_SHIFT                        16
#define BCHP_UFE_0_CTRL_CIC_DEC_RATIO_DEFAULT                      0x00000000

/* UFE_0 :: CTRL :: reserved1 [15:15] */
#define BCHP_UFE_0_CTRL_reserved1_MASK                             0x00008000
#define BCHP_UFE_0_CTRL_reserved1_SHIFT                            15

/* UFE_0 :: CTRL :: reserved_for_eco2 [14:13] */
#define BCHP_UFE_0_CTRL_reserved_for_eco2_MASK                     0x00006000
#define BCHP_UFE_0_CTRL_reserved_for_eco2_SHIFT                    13
#define BCHP_UFE_0_CTRL_reserved_for_eco2_DEFAULT                  0x00000000

/* UFE_0 :: CTRL :: VID_MIXER_MODE [12:12] */
#define BCHP_UFE_0_CTRL_VID_MIXER_MODE_MASK                        0x00001000
#define BCHP_UFE_0_CTRL_VID_MIXER_MODE_SHIFT                       12
#define BCHP_UFE_0_CTRL_VID_MIXER_MODE_DEFAULT                     0x00000000

/* UFE_0 :: CTRL :: VID_MIXER_EN [11:11] */
#define BCHP_UFE_0_CTRL_VID_MIXER_EN_MASK                          0x00000800
#define BCHP_UFE_0_CTRL_VID_MIXER_EN_SHIFT                         11
#define BCHP_UFE_0_CTRL_VID_MIXER_EN_DEFAULT                       0x00000000

/* UFE_0 :: CTRL :: USE_EXT_VID_FREQ [10:10] */
#define BCHP_UFE_0_CTRL_USE_EXT_VID_FREQ_MASK                      0x00000400
#define BCHP_UFE_0_CTRL_USE_EXT_VID_FREQ_SHIFT                     10
#define BCHP_UFE_0_CTRL_USE_EXT_VID_FREQ_DEFAULT                   0x00000001

/* UFE_0 :: CTRL :: VID_DIV [09:00] */
#define BCHP_UFE_0_CTRL_VID_DIV_MASK                               0x000003ff
#define BCHP_UFE_0_CTRL_VID_DIV_SHIFT                              0
#define BCHP_UFE_0_CTRL_VID_DIV_DEFAULT                            0x00000020

/***************************************************************************
 *BYP - bypass register
 ***************************************************************************/
/* UFE_0 :: BYP :: reserved0 [31:13] */
#define BCHP_UFE_0_BYP_reserved0_MASK                              0xffffe000
#define BCHP_UFE_0_BYP_reserved0_SHIFT                             13

/* UFE_0 :: BYP :: BACK_MIX [12:12] */
#define BCHP_UFE_0_BYP_BACK_MIX_MASK                               0x00001000
#define BCHP_UFE_0_BYP_BACK_MIX_SHIFT                              12
#define BCHP_UFE_0_BYP_BACK_MIX_DEFAULT                            0x00000001

/* UFE_0 :: BYP :: DCO [11:11] */
#define BCHP_UFE_0_BYP_DCO_MASK                                    0x00000800
#define BCHP_UFE_0_BYP_DCO_SHIFT                                   11
#define BCHP_UFE_0_BYP_DCO_DEFAULT                                 0x00000000

/* UFE_0 :: BYP :: IQ_PHS [10:10] */
#define BCHP_UFE_0_BYP_IQ_PHS_MASK                                 0x00000400
#define BCHP_UFE_0_BYP_IQ_PHS_SHIFT                                10
#define BCHP_UFE_0_BYP_IQ_PHS_DEFAULT                              0x00000000

/* UFE_0 :: BYP :: IQ_AMP [09:09] */
#define BCHP_UFE_0_BYP_IQ_AMP_MASK                                 0x00000200
#define BCHP_UFE_0_BYP_IQ_AMP_SHIFT                                9
#define BCHP_UFE_0_BYP_IQ_AMP_DEFAULT                              0x00000000

/* UFE_0 :: BYP :: HBU [08:08] */
#define BCHP_UFE_0_BYP_HBU_MASK                                    0x00000100
#define BCHP_UFE_0_BYP_HBU_SHIFT                                   8
#define BCHP_UFE_0_BYP_HBU_DEFAULT                                 0x00000000

/* UFE_0 :: BYP :: VID [07:07] */
#define BCHP_UFE_0_BYP_VID_MASK                                    0x00000080
#define BCHP_UFE_0_BYP_VID_SHIFT                                   7
#define BCHP_UFE_0_BYP_VID_DEFAULT                                 0x00000000

/* UFE_0 :: BYP :: AGC [06:04] */
#define BCHP_UFE_0_BYP_AGC_MASK                                    0x00000070
#define BCHP_UFE_0_BYP_AGC_SHIFT                                   4
#define BCHP_UFE_0_BYP_AGC_DEFAULT                                 0x00000000

/* UFE_0 :: BYP :: reserved1 [03:03] */
#define BCHP_UFE_0_BYP_reserved1_MASK                              0x00000008
#define BCHP_UFE_0_BYP_reserved1_SHIFT                             3

/* UFE_0 :: BYP :: HB [02:01] */
#define BCHP_UFE_0_BYP_HB_MASK                                     0x00000006
#define BCHP_UFE_0_BYP_HB_SHIFT                                    1
#define BCHP_UFE_0_BYP_HB_DEFAULT                                  0x00000000

/* UFE_0 :: BYP :: CIC [00:00] */
#define BCHP_UFE_0_BYP_CIC_MASK                                    0x00000001
#define BCHP_UFE_0_BYP_CIC_SHIFT                                   0
#define BCHP_UFE_0_BYP_CIC_DEFAULT                                 0x00000001

/***************************************************************************
 *RST - reset control register
 ***************************************************************************/
/* UFE_0 :: RST :: CLKGEN_RESET [31:31] */
#define BCHP_UFE_0_RST_CLKGEN_RESET_MASK                           0x80000000
#define BCHP_UFE_0_RST_CLKGEN_RESET_SHIFT                          31
#define BCHP_UFE_0_RST_CLKGEN_RESET_DEFAULT                        0x00000000

/* UFE_0 :: RST :: DATA_RESET [30:30] */
#define BCHP_UFE_0_RST_DATA_RESET_MASK                             0x40000000
#define BCHP_UFE_0_RST_DATA_RESET_SHIFT                            30
#define BCHP_UFE_0_RST_DATA_RESET_DEFAULT                          0x00000001

/* UFE_0 :: RST :: reserved0 [29:12] */
#define BCHP_UFE_0_RST_reserved0_MASK                              0x3ffff000
#define BCHP_UFE_0_RST_reserved0_SHIFT                             12

/* UFE_0 :: RST :: DCO [11:11] */
#define BCHP_UFE_0_RST_DCO_MASK                                    0x00000800
#define BCHP_UFE_0_RST_DCO_SHIFT                                   11
#define BCHP_UFE_0_RST_DCO_DEFAULT                                 0x00000001

/* UFE_0 :: RST :: IQ_PHS [10:10] */
#define BCHP_UFE_0_RST_IQ_PHS_MASK                                 0x00000400
#define BCHP_UFE_0_RST_IQ_PHS_SHIFT                                10
#define BCHP_UFE_0_RST_IQ_PHS_DEFAULT                              0x00000001

/* UFE_0 :: RST :: IQ_AMP [09:09] */
#define BCHP_UFE_0_RST_IQ_AMP_MASK                                 0x00000200
#define BCHP_UFE_0_RST_IQ_AMP_SHIFT                                9
#define BCHP_UFE_0_RST_IQ_AMP_DEFAULT                              0x00000001

/* UFE_0 :: RST :: reserved1 [08:07] */
#define BCHP_UFE_0_RST_reserved1_MASK                              0x00000180
#define BCHP_UFE_0_RST_reserved1_SHIFT                             7

/* UFE_0 :: RST :: AGC [06:04] */
#define BCHP_UFE_0_RST_AGC_MASK                                    0x00000070
#define BCHP_UFE_0_RST_AGC_SHIFT                                   4
#define BCHP_UFE_0_RST_AGC_DEFAULT                                 0x00000007

/* UFE_0 :: RST :: reserved2 [03:00] */
#define BCHP_UFE_0_RST_reserved2_MASK                              0x0000000f
#define BCHP_UFE_0_RST_reserved2_SHIFT                             0

/***************************************************************************
 *FRZ - freeze control register
 ***************************************************************************/
/* UFE_0 :: FRZ :: reserved0 [31:13] */
#define BCHP_UFE_0_FRZ_reserved0_MASK                              0xffffe000
#define BCHP_UFE_0_FRZ_reserved0_SHIFT                             13

/* UFE_0 :: FRZ :: DCO [12:12] */
#define BCHP_UFE_0_FRZ_DCO_MASK                                    0x00001000
#define BCHP_UFE_0_FRZ_DCO_SHIFT                                   12
#define BCHP_UFE_0_FRZ_DCO_DEFAULT                                 0x00000001

/* UFE_0 :: FRZ :: IQ_PHS [11:11] */
#define BCHP_UFE_0_FRZ_IQ_PHS_MASK                                 0x00000800
#define BCHP_UFE_0_FRZ_IQ_PHS_SHIFT                                11
#define BCHP_UFE_0_FRZ_IQ_PHS_DEFAULT                              0x00000001

/* UFE_0 :: FRZ :: IQ_AMP [10:10] */
#define BCHP_UFE_0_FRZ_IQ_AMP_MASK                                 0x00000400
#define BCHP_UFE_0_FRZ_IQ_AMP_SHIFT                                10
#define BCHP_UFE_0_FRZ_IQ_AMP_DEFAULT                              0x00000001

/* UFE_0 :: FRZ :: reserved1 [09:07] */
#define BCHP_UFE_0_FRZ_reserved1_MASK                              0x00000380
#define BCHP_UFE_0_FRZ_reserved1_SHIFT                             7

/* UFE_0 :: FRZ :: AGC [06:04] */
#define BCHP_UFE_0_FRZ_AGC_MASK                                    0x00000070
#define BCHP_UFE_0_FRZ_AGC_SHIFT                                   4
#define BCHP_UFE_0_FRZ_AGC_DEFAULT                                 0x00000007

/* UFE_0 :: FRZ :: reserved2 [03:00] */
#define BCHP_UFE_0_FRZ_reserved2_MASK                              0x0000000f
#define BCHP_UFE_0_FRZ_reserved2_SHIFT                             0

/***************************************************************************
 *AGC1 - AGC1 control register
 ***************************************************************************/
/* UFE_0 :: AGC1 :: USE_EXT_GAIN [31:31] */
#define BCHP_UFE_0_AGC1_USE_EXT_GAIN_MASK                          0x80000000
#define BCHP_UFE_0_AGC1_USE_EXT_GAIN_SHIFT                         31
#define BCHP_UFE_0_AGC1_USE_EXT_GAIN_DEFAULT                       0x00000000

/* UFE_0 :: AGC1 :: reserved0 [30:06] */
#define BCHP_UFE_0_AGC1_reserved0_MASK                             0x7fffffc0
#define BCHP_UFE_0_AGC1_reserved0_SHIFT                            6

/* UFE_0 :: AGC1 :: reserved_for_eco1 [05:04] */
#define BCHP_UFE_0_AGC1_reserved_for_eco1_MASK                     0x00000030
#define BCHP_UFE_0_AGC1_reserved_for_eco1_SHIFT                    4
#define BCHP_UFE_0_AGC1_reserved_for_eco1_DEFAULT                  0x00000000

/* UFE_0 :: AGC1 :: BW [03:00] */
#define BCHP_UFE_0_AGC1_BW_MASK                                    0x0000000f
#define BCHP_UFE_0_AGC1_BW_SHIFT                                   0
#define BCHP_UFE_0_AGC1_BW_DEFAULT                                 0x00000000

/***************************************************************************
 *AGC2 - AGC2 control register
 ***************************************************************************/
/* UFE_0 :: AGC2 :: USE_EXT_GAIN [31:31] */
#define BCHP_UFE_0_AGC2_USE_EXT_GAIN_MASK                          0x80000000
#define BCHP_UFE_0_AGC2_USE_EXT_GAIN_SHIFT                         31
#define BCHP_UFE_0_AGC2_USE_EXT_GAIN_DEFAULT                       0x00000000

/* UFE_0 :: AGC2 :: reserved0 [30:06] */
#define BCHP_UFE_0_AGC2_reserved0_MASK                             0x7fffffc0
#define BCHP_UFE_0_AGC2_reserved0_SHIFT                            6

/* UFE_0 :: AGC2 :: reserved_for_eco1 [05:04] */
#define BCHP_UFE_0_AGC2_reserved_for_eco1_MASK                     0x00000030
#define BCHP_UFE_0_AGC2_reserved_for_eco1_SHIFT                    4
#define BCHP_UFE_0_AGC2_reserved_for_eco1_DEFAULT                  0x00000000

/* UFE_0 :: AGC2 :: BW [03:00] */
#define BCHP_UFE_0_AGC2_BW_MASK                                    0x0000000f
#define BCHP_UFE_0_AGC2_BW_SHIFT                                   0
#define BCHP_UFE_0_AGC2_BW_DEFAULT                                 0x00000000

/***************************************************************************
 *AGC3 - AGC3 control register
 ***************************************************************************/
/* UFE_0 :: AGC3 :: USE_EXT_GAIN [31:31] */
#define BCHP_UFE_0_AGC3_USE_EXT_GAIN_MASK                          0x80000000
#define BCHP_UFE_0_AGC3_USE_EXT_GAIN_SHIFT                         31
#define BCHP_UFE_0_AGC3_USE_EXT_GAIN_DEFAULT                       0x00000000

/* UFE_0 :: AGC3 :: reserved0 [30:06] */
#define BCHP_UFE_0_AGC3_reserved0_MASK                             0x7fffffc0
#define BCHP_UFE_0_AGC3_reserved0_SHIFT                            6

/* UFE_0 :: AGC3 :: reserved_for_eco1 [05:04] */
#define BCHP_UFE_0_AGC3_reserved_for_eco1_MASK                     0x00000030
#define BCHP_UFE_0_AGC3_reserved_for_eco1_SHIFT                    4
#define BCHP_UFE_0_AGC3_reserved_for_eco1_DEFAULT                  0x00000000

/* UFE_0 :: AGC3 :: BW [03:00] */
#define BCHP_UFE_0_AGC3_BW_MASK                                    0x0000000f
#define BCHP_UFE_0_AGC3_BW_SHIFT                                   0
#define BCHP_UFE_0_AGC3_BW_DEFAULT                                 0x00000000

/***************************************************************************
 *AGC1_THRESH - AGC1 threshold register
 ***************************************************************************/
/* UFE_0 :: AGC1_THRESH :: reserved0 [31:22] */
#define BCHP_UFE_0_AGC1_THRESH_reserved0_MASK                      0xffc00000
#define BCHP_UFE_0_AGC1_THRESH_reserved0_SHIFT                     22

/* UFE_0 :: AGC1_THRESH :: THRESHOLD [21:00] */
#define BCHP_UFE_0_AGC1_THRESH_THRESHOLD_MASK                      0x003fffff
#define BCHP_UFE_0_AGC1_THRESH_THRESHOLD_SHIFT                     0
#define BCHP_UFE_0_AGC1_THRESH_THRESHOLD_DEFAULT                   0x00000000

/***************************************************************************
 *AGC2_THRESH - AGC2 threshold register
 ***************************************************************************/
/* UFE_0 :: AGC2_THRESH :: reserved0 [31:22] */
#define BCHP_UFE_0_AGC2_THRESH_reserved0_MASK                      0xffc00000
#define BCHP_UFE_0_AGC2_THRESH_reserved0_SHIFT                     22

/* UFE_0 :: AGC2_THRESH :: THRESHOLD [21:00] */
#define BCHP_UFE_0_AGC2_THRESH_THRESHOLD_MASK                      0x003fffff
#define BCHP_UFE_0_AGC2_THRESH_THRESHOLD_SHIFT                     0
#define BCHP_UFE_0_AGC2_THRESH_THRESHOLD_DEFAULT                   0x00000000

/***************************************************************************
 *AGC3_THRESH - AGC3 threshold register
 ***************************************************************************/
/* UFE_0 :: AGC3_THRESH :: reserved0 [31:22] */
#define BCHP_UFE_0_AGC3_THRESH_reserved0_MASK                      0xffc00000
#define BCHP_UFE_0_AGC3_THRESH_reserved0_SHIFT                     22

/* UFE_0 :: AGC3_THRESH :: THRESHOLD [21:00] */
#define BCHP_UFE_0_AGC3_THRESH_THRESHOLD_MASK                      0x003fffff
#define BCHP_UFE_0_AGC3_THRESH_THRESHOLD_SHIFT                     0
#define BCHP_UFE_0_AGC3_THRESH_THRESHOLD_DEFAULT                   0x00000000

/***************************************************************************
 *AGC1_LF - AGC1 loop filter register
 ***************************************************************************/
/* UFE_0 :: AGC1_LF :: LF [31:00] */
#define BCHP_UFE_0_AGC1_LF_LF_MASK                                 0xffffffff
#define BCHP_UFE_0_AGC1_LF_LF_SHIFT                                0
#define BCHP_UFE_0_AGC1_LF_LF_DEFAULT                              0x02000000

/***************************************************************************
 *AGC2_LF - AGC2 loop filter register
 ***************************************************************************/
/* UFE_0 :: AGC2_LF :: LF [31:00] */
#define BCHP_UFE_0_AGC2_LF_LF_MASK                                 0xffffffff
#define BCHP_UFE_0_AGC2_LF_LF_SHIFT                                0
#define BCHP_UFE_0_AGC2_LF_LF_DEFAULT                              0x02000000

/***************************************************************************
 *AGC3_LF - AGC3 loop filter register
 ***************************************************************************/
/* UFE_0 :: AGC3_LF :: LF [31:00] */
#define BCHP_UFE_0_AGC3_LF_LF_MASK                                 0xffffffff
#define BCHP_UFE_0_AGC3_LF_LF_SHIFT                                0
#define BCHP_UFE_0_AGC3_LF_LF_DEFAULT                              0x02000000

/***************************************************************************
 *IQIMB_AMP_CTRL - IQ-Imbalance amplitude correction control register
 ***************************************************************************/
/* UFE_0 :: IQIMB_AMP_CTRL :: reserved0 [31:04] */
#define BCHP_UFE_0_IQIMB_AMP_CTRL_reserved0_MASK                   0xfffffff0
#define BCHP_UFE_0_IQIMB_AMP_CTRL_reserved0_SHIFT                  4

/* UFE_0 :: IQIMB_AMP_CTRL :: BW [03:00] */
#define BCHP_UFE_0_IQIMB_AMP_CTRL_BW_MASK                          0x0000000f
#define BCHP_UFE_0_IQIMB_AMP_CTRL_BW_SHIFT                         0
#define BCHP_UFE_0_IQIMB_AMP_CTRL_BW_DEFAULT                       0x00000000

/***************************************************************************
 *IQIMB_PHS_CTRL - IQ-Imbalance phase     correction control register
 ***************************************************************************/
/* UFE_0 :: IQIMB_PHS_CTRL :: reserved0 [31:04] */
#define BCHP_UFE_0_IQIMB_PHS_CTRL_reserved0_MASK                   0xfffffff0
#define BCHP_UFE_0_IQIMB_PHS_CTRL_reserved0_SHIFT                  4

/* UFE_0 :: IQIMB_PHS_CTRL :: BW [03:00] */
#define BCHP_UFE_0_IQIMB_PHS_CTRL_BW_MASK                          0x0000000f
#define BCHP_UFE_0_IQIMB_PHS_CTRL_BW_SHIFT                         0
#define BCHP_UFE_0_IQIMB_PHS_CTRL_BW_DEFAULT                       0x00000000

/***************************************************************************
 *IQIMB_AMP_LF - IQ-Imbalance amplitude loop filter register
 ***************************************************************************/
/* UFE_0 :: IQIMB_AMP_LF :: LF [31:00] */
#define BCHP_UFE_0_IQIMB_AMP_LF_LF_MASK                            0xffffffff
#define BCHP_UFE_0_IQIMB_AMP_LF_LF_SHIFT                           0
#define BCHP_UFE_0_IQIMB_AMP_LF_LF_DEFAULT                         0x00000000

/***************************************************************************
 *IQIMB_PHS_LF - IQ-Imbalance phase     loop filter register
 ***************************************************************************/
/* UFE_0 :: IQIMB_PHS_LF :: LF [31:00] */
#define BCHP_UFE_0_IQIMB_PHS_LF_LF_MASK                            0xffffffff
#define BCHP_UFE_0_IQIMB_PHS_LF_LF_SHIFT                           0
#define BCHP_UFE_0_IQIMB_PHS_LF_LF_DEFAULT                         0x00000000

/***************************************************************************
 *DCO_CTRL - DCO canceller control register
 ***************************************************************************/
/* UFE_0 :: DCO_CTRL :: reserved0 [31:06] */
#define BCHP_UFE_0_DCO_CTRL_reserved0_MASK                         0xffffffc0
#define BCHP_UFE_0_DCO_CTRL_reserved0_SHIFT                        6

/* UFE_0 :: DCO_CTRL :: BW [05:00] */
#define BCHP_UFE_0_DCO_CTRL_BW_MASK                                0x0000003f
#define BCHP_UFE_0_DCO_CTRL_BW_SHIFT                               0
#define BCHP_UFE_0_DCO_CTRL_BW_DEFAULT                             0x00000000

/***************************************************************************
 *DCOINTI - DCO integrator I
 ***************************************************************************/
/* UFE_0 :: DCOINTI :: INT [31:00] */
#define BCHP_UFE_0_DCOINTI_INT_MASK                                0xffffffff
#define BCHP_UFE_0_DCOINTI_INT_SHIFT                               0
#define BCHP_UFE_0_DCOINTI_INT_DEFAULT                             0x00000000

/***************************************************************************
 *DCOINTQ - DCO integrator Q
 ***************************************************************************/
/* UFE_0 :: DCOINTQ :: INT [31:00] */
#define BCHP_UFE_0_DCOINTQ_INT_MASK                                0xffffffff
#define BCHP_UFE_0_DCOINTQ_INT_SHIFT                               0
#define BCHP_UFE_0_DCOINTQ_INT_DEFAULT                             0x00000000

/***************************************************************************
 *BMIX_FCW - FCW register for back mixer
 ***************************************************************************/
/* UFE_0 :: BMIX_FCW :: FCW [31:00] */
#define BCHP_UFE_0_BMIX_FCW_FCW_MASK                               0xffffffff
#define BCHP_UFE_0_BMIX_FCW_FCW_SHIFT                              0
#define BCHP_UFE_0_BMIX_FCW_FCW_DEFAULT                            0x00000000

/***************************************************************************
 *FMIX_FCW - FCW register for front mixer
 ***************************************************************************/
/* UFE_0 :: FMIX_FCW :: FCW [31:00] */
#define BCHP_UFE_0_FMIX_FCW_FCW_MASK                               0xffffffff
#define BCHP_UFE_0_FMIX_FCW_FCW_SHIFT                              0
#define BCHP_UFE_0_FMIX_FCW_FCW_DEFAULT                            0x00000000

/***************************************************************************
 *CRC_EN - CRC enable register
 ***************************************************************************/
/* UFE_0 :: CRC_EN :: ENABLE [31:31] */
#define BCHP_UFE_0_CRC_EN_ENABLE_MASK                              0x80000000
#define BCHP_UFE_0_CRC_EN_ENABLE_SHIFT                             31
#define BCHP_UFE_0_CRC_EN_ENABLE_DEFAULT                           0x00000000

/* UFE_0 :: CRC_EN :: COUNT [30:00] */
#define BCHP_UFE_0_CRC_EN_COUNT_MASK                               0x7fffffff
#define BCHP_UFE_0_CRC_EN_COUNT_SHIFT                              0
#define BCHP_UFE_0_CRC_EN_COUNT_DEFAULT                            0x00000000

/***************************************************************************
 *CRC - CRC signature analyzer register
 ***************************************************************************/
/* UFE_0 :: CRC :: VALUE [31:00] */
#define BCHP_UFE_0_CRC_VALUE_MASK                                  0xffffffff
#define BCHP_UFE_0_CRC_VALUE_SHIFT                                 0
#define BCHP_UFE_0_CRC_VALUE_DEFAULT                               0x55555555

/***************************************************************************
 *LFSR_SEED - LFSR initial seed
 ***************************************************************************/
/* UFE_0 :: LFSR_SEED :: SEED [31:00] */
#define BCHP_UFE_0_LFSR_SEED_SEED_MASK                             0xffffffff
#define BCHP_UFE_0_LFSR_SEED_SEED_SHIFT                            0
#define BCHP_UFE_0_LFSR_SEED_SEED_DEFAULT                          0x00000001

/***************************************************************************
 *TP - Testport register
 ***************************************************************************/
/* UFE_0 :: TP :: TPOUT_EN [31:31] */
#define BCHP_UFE_0_TP_TPOUT_EN_MASK                                0x80000000
#define BCHP_UFE_0_TP_TPOUT_EN_SHIFT                               31
#define BCHP_UFE_0_TP_TPOUT_EN_DEFAULT                             0x00000000

/* UFE_0 :: TP :: TPOUT_SEL [30:26] */
#define BCHP_UFE_0_TP_TPOUT_SEL_MASK                               0x7c000000
#define BCHP_UFE_0_TP_TPOUT_SEL_SHIFT                              26
#define BCHP_UFE_0_TP_TPOUT_SEL_DEFAULT                            0x00000000

/* UFE_0 :: TP :: reserved0 [25:08] */
#define BCHP_UFE_0_TP_reserved0_MASK                               0x03ffff00
#define BCHP_UFE_0_TP_reserved0_SHIFT                              8

/* UFE_0 :: TP :: AI_CHK_I [07:07] */
#define BCHP_UFE_0_TP_AI_CHK_I_MASK                                0x00000080
#define BCHP_UFE_0_TP_AI_CHK_I_SHIFT                               7
#define BCHP_UFE_0_TP_AI_CHK_I_DEFAULT                             0x00000000

/* UFE_0 :: TP :: AI_CHK_Q [06:06] */
#define BCHP_UFE_0_TP_AI_CHK_Q_MASK                                0x00000040
#define BCHP_UFE_0_TP_AI_CHK_Q_SHIFT                               6
#define BCHP_UFE_0_TP_AI_CHK_Q_DEFAULT                             0x00000000

/* UFE_0 :: TP :: RECORD_ADC_SEL [05:04] */
#define BCHP_UFE_0_TP_RECORD_ADC_SEL_MASK                          0x00000030
#define BCHP_UFE_0_TP_RECORD_ADC_SEL_SHIFT                         4
#define BCHP_UFE_0_TP_RECORD_ADC_SEL_DEFAULT                       0x00000000

/* UFE_0 :: TP :: RECORD_ADC [03:03] */
#define BCHP_UFE_0_TP_RECORD_ADC_MASK                              0x00000008
#define BCHP_UFE_0_TP_RECORD_ADC_SHIFT                             3
#define BCHP_UFE_0_TP_RECORD_ADC_DEFAULT                           0x00000000

/* UFE_0 :: TP :: FIXED_RECORD_ADC [02:02] */
#define BCHP_UFE_0_TP_FIXED_RECORD_ADC_MASK                        0x00000004
#define BCHP_UFE_0_TP_FIXED_RECORD_ADC_SHIFT                       2
#define BCHP_UFE_0_TP_FIXED_RECORD_ADC_DEFAULT                     0x00000000

/* UFE_0 :: TP :: LFSR_EN [01:01] */
#define BCHP_UFE_0_TP_LFSR_EN_MASK                                 0x00000002
#define BCHP_UFE_0_TP_LFSR_EN_SHIFT                                1
#define BCHP_UFE_0_TP_LFSR_EN_DEFAULT                              0x00000000

/* UFE_0 :: TP :: IFC_LFSR_EN [00:00] */
#define BCHP_UFE_0_TP_IFC_LFSR_EN_MASK                             0x00000001
#define BCHP_UFE_0_TP_IFC_LFSR_EN_SHIFT                            0
#define BCHP_UFE_0_TP_IFC_LFSR_EN_DEFAULT                          0x00000000

/***************************************************************************
 *SPARE - Software spare register
 ***************************************************************************/
/* UFE_0 :: SPARE :: spare [31:00] */
#define BCHP_UFE_0_SPARE_spare_MASK                                0xffffffff
#define BCHP_UFE_0_SPARE_spare_SHIFT                               0
#define BCHP_UFE_0_SPARE_spare_DEFAULT                             0x00000000

#endif /* #ifndef BCHP_UFE_0_H__ */

/* End of File */
