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
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 * The launch point for all information concerning RDB is found at:
 *   http://bcgbu.broadcom.com/RDB/SitePages/Home.aspx
 *
 * Date:           Generated on               Tue Jun 27 10:52:39 2017
 *                 Full Compile MD5 Checksum  de13a1e8011803b5a40ab14e4d71d071
 *                     (minus title and desc)
 *                 MD5 Checksum               b694fcab41780597392ed5a8f558ad3e
 *
 * lock_release:   r_1255
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     1570
 *                 unknown                    unknown
 *                 Perl Interpreter           5.014001
 *                 Operating System           linux
 *                 Script Source              home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   LOCAL home/pntruong/sbin/combo_header.pl
 *
 *
********************************************************************************/

#ifndef BCHP_XPT_MPOD_H__
#define BCHP_XPT_MPOD_H__

/***************************************************************************
 *XPT_MPOD - XPT MPOD Control Registers
 ***************************************************************************/
#define BCHP_XPT_MPOD_CFG                        0x20a02c00 /* [RW][32] MPOD Configuration Register */
#define BCHP_XPT_MPOD_OCTRL                      0x20a02c04 /* [RW][32] MPOD Output Interface Formatter Control Register */
#define BCHP_XPT_MPOD_ICTRL                      0x20a02c08 /* [RW][32] MPOD Input Interface Formatter Control Register */
#define BCHP_XPT_MPOD_RES_FIELD                  0x20a02c0c /* [RO][32] MPOD Reserved Fields Register */
#define BCHP_XPT_MPOD_OCTRL2                     0x20a02c10 /* [RW][32] MPOD Output Interface Formatter Control2 Register */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP      0x20a02c14 /* [RW][32] MPOD Band ID IBP Disable */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP      0x20a02c18 /* [RW][32] MPOD Band ID PBP Disable */
#define BCHP_XPT_MPOD_TV_STATUS                  0x20a02c20 /* [RW][32] TV Status */

/***************************************************************************
 *CFG - MPOD Configuration Register
 ***************************************************************************/
/* XPT_MPOD :: CFG :: reserved0 [31:16] */
#define BCHP_XPT_MPOD_CFG_reserved0_MASK                           0xffff0000
#define BCHP_XPT_MPOD_CFG_reserved0_SHIFT                          16

/* XPT_MPOD :: CFG :: SPARE_FUNC [15:13] */
#define BCHP_XPT_MPOD_CFG_SPARE_FUNC_MASK                          0x0000e000
#define BCHP_XPT_MPOD_CFG_SPARE_FUNC_SHIFT                         13
#define BCHP_XPT_MPOD_CFG_SPARE_FUNC_DEFAULT                       0x00000000

/* XPT_MPOD :: CFG :: DROP_PSUB_NULL_PKT_EN [12:12] */
#define BCHP_XPT_MPOD_CFG_DROP_PSUB_NULL_PKT_EN_MASK               0x00001000
#define BCHP_XPT_MPOD_CFG_DROP_PSUB_NULL_PKT_EN_SHIFT              12
#define BCHP_XPT_MPOD_CFG_DROP_PSUB_NULL_PKT_EN_DEFAULT            0x00000001

/* XPT_MPOD :: CFG :: SMODE_EN [11:11] */
#define BCHP_XPT_MPOD_CFG_SMODE_EN_MASK                            0x00000800
#define BCHP_XPT_MPOD_CFG_SMODE_EN_SHIFT                           11
#define BCHP_XPT_MPOD_CFG_SMODE_EN_DEFAULT                         0x00000000

/* XPT_MPOD :: CFG :: CRC_INIT_VALUE [10:03] */
#define BCHP_XPT_MPOD_CFG_CRC_INIT_VALUE_MASK                      0x000007f8
#define BCHP_XPT_MPOD_CFG_CRC_INIT_VALUE_SHIFT                     3
#define BCHP_XPT_MPOD_CFG_CRC_INIT_VALUE_DEFAULT                   0x000000ff

/* XPT_MPOD :: CFG :: reserved1 [02:02] */
#define BCHP_XPT_MPOD_CFG_reserved1_MASK                           0x00000004
#define BCHP_XPT_MPOD_CFG_reserved1_SHIFT                          2

/* XPT_MPOD :: CFG :: MPOD_EXT_EN [01:01] */
#define BCHP_XPT_MPOD_CFG_MPOD_EXT_EN_MASK                         0x00000002
#define BCHP_XPT_MPOD_CFG_MPOD_EXT_EN_SHIFT                        1
#define BCHP_XPT_MPOD_CFG_MPOD_EXT_EN_DEFAULT                      0x00000000

/* XPT_MPOD :: CFG :: MPOD_EN [00:00] */
#define BCHP_XPT_MPOD_CFG_MPOD_EN_MASK                             0x00000001
#define BCHP_XPT_MPOD_CFG_MPOD_EN_SHIFT                            0
#define BCHP_XPT_MPOD_CFG_MPOD_EN_DEFAULT                          0x00000000

/***************************************************************************
 *OCTRL - MPOD Output Interface Formatter Control Register
 ***************************************************************************/
/* XPT_MPOD :: OCTRL :: HOST_RSVD [31:16] */
#define BCHP_XPT_MPOD_OCTRL_HOST_RSVD_MASK                         0xffff0000
#define BCHP_XPT_MPOD_OCTRL_HOST_RSVD_SHIFT                        16
#define BCHP_XPT_MPOD_OCTRL_HOST_RSVD_DEFAULT                      0x00000000

/* XPT_MPOD :: OCTRL :: CLK_DELAY [15:12] */
#define BCHP_XPT_MPOD_OCTRL_CLK_DELAY_MASK                         0x0000f000
#define BCHP_XPT_MPOD_OCTRL_CLK_DELAY_SHIFT                        12
#define BCHP_XPT_MPOD_OCTRL_CLK_DELAY_DEFAULT                      0x00000000

/* XPT_MPOD :: OCTRL :: reserved0 [11:11] */
#define BCHP_XPT_MPOD_OCTRL_reserved0_MASK                         0x00000800
#define BCHP_XPT_MPOD_OCTRL_reserved0_SHIFT                        11

/* XPT_MPOD :: OCTRL :: HOST_RSVD_EN [10:10] */
#define BCHP_XPT_MPOD_OCTRL_HOST_RSVD_EN_MASK                      0x00000400
#define BCHP_XPT_MPOD_OCTRL_HOST_RSVD_EN_SHIFT                     10
#define BCHP_XPT_MPOD_OCTRL_HOST_RSVD_EN_DEFAULT                   0x00000000

/* XPT_MPOD :: OCTRL :: INVERT_VALID [09:09] */
#define BCHP_XPT_MPOD_OCTRL_INVERT_VALID_MASK                      0x00000200
#define BCHP_XPT_MPOD_OCTRL_INVERT_VALID_SHIFT                     9
#define BCHP_XPT_MPOD_OCTRL_INVERT_VALID_DEFAULT                   0x00000000

/* XPT_MPOD :: OCTRL :: BYTE_SYNC [08:08] */
#define BCHP_XPT_MPOD_OCTRL_BYTE_SYNC_MASK                         0x00000100
#define BCHP_XPT_MPOD_OCTRL_BYTE_SYNC_SHIFT                        8
#define BCHP_XPT_MPOD_OCTRL_BYTE_SYNC_DEFAULT                      0x00000001

/* XPT_MPOD :: OCTRL :: CLK_NRUN [07:07] */
#define BCHP_XPT_MPOD_OCTRL_CLK_NRUN_MASK                          0x00000080
#define BCHP_XPT_MPOD_OCTRL_CLK_NRUN_SHIFT                         7
#define BCHP_XPT_MPOD_OCTRL_CLK_NRUN_DEFAULT                       0x00000000

/* XPT_MPOD :: OCTRL :: INVERT_CLK [06:06] */
#define BCHP_XPT_MPOD_OCTRL_INVERT_CLK_MASK                        0x00000040
#define BCHP_XPT_MPOD_OCTRL_INVERT_CLK_SHIFT                       6
#define BCHP_XPT_MPOD_OCTRL_INVERT_CLK_DEFAULT                     0x00000000

/* XPT_MPOD :: OCTRL :: NSHIFT_CLK [05:05] */
#define BCHP_XPT_MPOD_OCTRL_NSHIFT_CLK_MASK                        0x00000020
#define BCHP_XPT_MPOD_OCTRL_NSHIFT_CLK_SHIFT                       5
#define BCHP_XPT_MPOD_OCTRL_NSHIFT_CLK_DEFAULT                     0x00000000

/* XPT_MPOD :: OCTRL :: INVERT_SYNC [04:04] */
#define BCHP_XPT_MPOD_OCTRL_INVERT_SYNC_MASK                       0x00000010
#define BCHP_XPT_MPOD_OCTRL_INVERT_SYNC_SHIFT                      4
#define BCHP_XPT_MPOD_OCTRL_INVERT_SYNC_DEFAULT                    0x00000000

/* XPT_MPOD :: OCTRL :: MUTE [03:03] */
#define BCHP_XPT_MPOD_OCTRL_MUTE_MASK                              0x00000008
#define BCHP_XPT_MPOD_OCTRL_MUTE_SHIFT                             3
#define BCHP_XPT_MPOD_OCTRL_MUTE_DEFAULT                           0x00000000

/* XPT_MPOD :: OCTRL :: reserved1 [02:01] */
#define BCHP_XPT_MPOD_OCTRL_reserved1_MASK                         0x00000006
#define BCHP_XPT_MPOD_OCTRL_reserved1_SHIFT                        1

/* XPT_MPOD :: OCTRL :: OUTPUT_FORMATTER_EN [00:00] */
#define BCHP_XPT_MPOD_OCTRL_OUTPUT_FORMATTER_EN_MASK               0x00000001
#define BCHP_XPT_MPOD_OCTRL_OUTPUT_FORMATTER_EN_SHIFT              0
#define BCHP_XPT_MPOD_OCTRL_OUTPUT_FORMATTER_EN_DEFAULT            0x00000000

/***************************************************************************
 *ICTRL - MPOD Input Interface Formatter Control Register
 ***************************************************************************/
/* XPT_MPOD :: ICTRL :: reserved0 [31:18] */
#define BCHP_XPT_MPOD_ICTRL_reserved0_MASK                         0xfffc0000
#define BCHP_XPT_MPOD_ICTRL_reserved0_SHIFT                        18

/* XPT_MPOD :: ICTRL :: BAND_NO [17:12] */
#define BCHP_XPT_MPOD_ICTRL_BAND_NO_MASK                           0x0003f000
#define BCHP_XPT_MPOD_ICTRL_BAND_NO_SHIFT                          12
#define BCHP_XPT_MPOD_ICTRL_BAND_NO_DEFAULT                        0x00000000

/* XPT_MPOD :: ICTRL :: PB_BAND [11:11] */
#define BCHP_XPT_MPOD_ICTRL_PB_BAND_MASK                           0x00000800
#define BCHP_XPT_MPOD_ICTRL_PB_BAND_SHIFT                          11
#define BCHP_XPT_MPOD_ICTRL_PB_BAND_DEFAULT                        0x00000000

/* XPT_MPOD :: ICTRL :: BAND_EN [10:10] */
#define BCHP_XPT_MPOD_ICTRL_BAND_EN_MASK                           0x00000400
#define BCHP_XPT_MPOD_ICTRL_BAND_EN_SHIFT                          10
#define BCHP_XPT_MPOD_ICTRL_BAND_EN_DEFAULT                        0x00000000

/* XPT_MPOD :: ICTRL :: FORCE_VALID [09:09] */
#define BCHP_XPT_MPOD_ICTRL_FORCE_VALID_MASK                       0x00000200
#define BCHP_XPT_MPOD_ICTRL_FORCE_VALID_SHIFT                      9
#define BCHP_XPT_MPOD_ICTRL_FORCE_VALID_DEFAULT                    0x00000000

/* XPT_MPOD :: ICTRL :: TIMESTAMP_INSERT_EN [08:08] */
#define BCHP_XPT_MPOD_ICTRL_TIMESTAMP_INSERT_EN_MASK               0x00000100
#define BCHP_XPT_MPOD_ICTRL_TIMESTAMP_INSERT_EN_SHIFT              8
#define BCHP_XPT_MPOD_ICTRL_TIMESTAMP_INSERT_EN_DEFAULT            0x00000000

/* XPT_MPOD :: ICTRL :: INVERT_VALID [07:07] */
#define BCHP_XPT_MPOD_ICTRL_INVERT_VALID_MASK                      0x00000080
#define BCHP_XPT_MPOD_ICTRL_INVERT_VALID_SHIFT                     7
#define BCHP_XPT_MPOD_ICTRL_INVERT_VALID_DEFAULT                   0x00000000

/* XPT_MPOD :: ICTRL :: INVERT_CLK [06:06] */
#define BCHP_XPT_MPOD_ICTRL_INVERT_CLK_MASK                        0x00000040
#define BCHP_XPT_MPOD_ICTRL_INVERT_CLK_SHIFT                       6
#define BCHP_XPT_MPOD_ICTRL_INVERT_CLK_DEFAULT                     0x00000000

/* XPT_MPOD :: ICTRL :: reserved1 [05:05] */
#define BCHP_XPT_MPOD_ICTRL_reserved1_MASK                         0x00000020
#define BCHP_XPT_MPOD_ICTRL_reserved1_SHIFT                        5

/* XPT_MPOD :: ICTRL :: INVERT_SYNC [04:04] */
#define BCHP_XPT_MPOD_ICTRL_INVERT_SYNC_MASK                       0x00000010
#define BCHP_XPT_MPOD_ICTRL_INVERT_SYNC_SHIFT                      4
#define BCHP_XPT_MPOD_ICTRL_INVERT_SYNC_DEFAULT                    0x00000000

/* XPT_MPOD :: ICTRL :: MUTE [03:03] */
#define BCHP_XPT_MPOD_ICTRL_MUTE_MASK                              0x00000008
#define BCHP_XPT_MPOD_ICTRL_MUTE_SHIFT                             3
#define BCHP_XPT_MPOD_ICTRL_MUTE_DEFAULT                           0x00000000

/* XPT_MPOD :: ICTRL :: reserved2 [02:01] */
#define BCHP_XPT_MPOD_ICTRL_reserved2_MASK                         0x00000006
#define BCHP_XPT_MPOD_ICTRL_reserved2_SHIFT                        1

/* XPT_MPOD :: ICTRL :: INPUT_FORMATTER_EN [00:00] */
#define BCHP_XPT_MPOD_ICTRL_INPUT_FORMATTER_EN_MASK                0x00000001
#define BCHP_XPT_MPOD_ICTRL_INPUT_FORMATTER_EN_SHIFT               0
#define BCHP_XPT_MPOD_ICTRL_INPUT_FORMATTER_EN_DEFAULT             0x00000000

/***************************************************************************
 *RES_FIELD - MPOD Reserved Fields Register
 ***************************************************************************/
/* XPT_MPOD :: RES_FIELD :: reserved0 [31:16] */
#define BCHP_XPT_MPOD_RES_FIELD_reserved0_MASK                     0xffff0000
#define BCHP_XPT_MPOD_RES_FIELD_reserved0_SHIFT                    16

/* XPT_MPOD :: RES_FIELD :: POD_RES [15:00] */
#define BCHP_XPT_MPOD_RES_FIELD_POD_RES_MASK                       0x0000ffff
#define BCHP_XPT_MPOD_RES_FIELD_POD_RES_SHIFT                      0
#define BCHP_XPT_MPOD_RES_FIELD_POD_RES_DEFAULT                    0x00000000

/***************************************************************************
 *OCTRL2 - MPOD Output Interface Formatter Control2 Register
 ***************************************************************************/
/* XPT_MPOD :: OCTRL2 :: MPOD_PKT_DLY_CNT [31:24] */
#define BCHP_XPT_MPOD_OCTRL2_MPOD_PKT_DLY_CNT_MASK                 0xff000000
#define BCHP_XPT_MPOD_OCTRL2_MPOD_PKT_DLY_CNT_SHIFT                24
#define BCHP_XPT_MPOD_OCTRL2_MPOD_PKT_DLY_CNT_DEFAULT              0x00000000

/* XPT_MPOD :: OCTRL2 :: reserved0 [23:08] */
#define BCHP_XPT_MPOD_OCTRL2_reserved0_MASK                        0x00ffff00
#define BCHP_XPT_MPOD_OCTRL2_reserved0_SHIFT                       8

/* XPT_MPOD :: OCTRL2 :: MPOD_CLK_DIV_SEL [07:05] */
#define BCHP_XPT_MPOD_OCTRL2_MPOD_CLK_DIV_SEL_MASK                 0x000000e0
#define BCHP_XPT_MPOD_OCTRL2_MPOD_CLK_DIV_SEL_SHIFT                5
#define BCHP_XPT_MPOD_OCTRL2_MPOD_CLK_DIV_SEL_DEFAULT              0x00000001

/* XPT_MPOD :: OCTRL2 :: MPOD_CLK_SEL [04:00] */
#define BCHP_XPT_MPOD_OCTRL2_MPOD_CLK_SEL_MASK                     0x0000001f
#define BCHP_XPT_MPOD_OCTRL2_MPOD_CLK_SEL_SHIFT                    0
#define BCHP_XPT_MPOD_OCTRL2_MPOD_CLK_SEL_DEFAULT                  0x00000006

/***************************************************************************
 *MPOD_BAND_ID_IBP_DROP - MPOD Band ID IBP Disable
 ***************************************************************************/
/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP31_DROP [31:31] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP31_DROP_MASK 0x80000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP31_DROP_SHIFT 31
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP31_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP30_DROP [30:30] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP30_DROP_MASK 0x40000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP30_DROP_SHIFT 30
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP30_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP29_DROP [29:29] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP29_DROP_MASK 0x20000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP29_DROP_SHIFT 29
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP29_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP28_DROP [28:28] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP28_DROP_MASK 0x10000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP28_DROP_SHIFT 28
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP28_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP27_DROP [27:27] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP27_DROP_MASK 0x08000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP27_DROP_SHIFT 27
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP27_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP26_DROP [26:26] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP26_DROP_MASK 0x04000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP26_DROP_SHIFT 26
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP26_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP25_DROP [25:25] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP25_DROP_MASK 0x02000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP25_DROP_SHIFT 25
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP25_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP24_DROP [24:24] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP24_DROP_MASK 0x01000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP24_DROP_SHIFT 24
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP24_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP23_DROP [23:23] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP23_DROP_MASK 0x00800000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP23_DROP_SHIFT 23
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP23_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP22_DROP [22:22] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP22_DROP_MASK 0x00400000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP22_DROP_SHIFT 22
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP22_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP21_DROP [21:21] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP21_DROP_MASK 0x00200000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP21_DROP_SHIFT 21
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP21_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP20_DROP [20:20] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP20_DROP_MASK 0x00100000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP20_DROP_SHIFT 20
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP20_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP19_DROP [19:19] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP19_DROP_MASK 0x00080000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP19_DROP_SHIFT 19
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP19_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP18_DROP [18:18] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP18_DROP_MASK 0x00040000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP18_DROP_SHIFT 18
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP18_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP17_DROP [17:17] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP17_DROP_MASK 0x00020000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP17_DROP_SHIFT 17
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP17_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP16_DROP [16:16] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP16_DROP_MASK 0x00010000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP16_DROP_SHIFT 16
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP16_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP15_DROP [15:15] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP15_DROP_MASK 0x00008000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP15_DROP_SHIFT 15
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP15_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP14_DROP [14:14] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP14_DROP_MASK 0x00004000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP14_DROP_SHIFT 14
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP14_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP13_DROP [13:13] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP13_DROP_MASK 0x00002000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP13_DROP_SHIFT 13
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP13_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP12_DROP [12:12] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP12_DROP_MASK 0x00001000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP12_DROP_SHIFT 12
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP12_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP11_DROP [11:11] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP11_DROP_MASK 0x00000800
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP11_DROP_SHIFT 11
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP11_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP10_DROP [10:10] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP10_DROP_MASK 0x00000400
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP10_DROP_SHIFT 10
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP10_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP9_DROP [09:09] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP9_DROP_MASK 0x00000200
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP9_DROP_SHIFT 9
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP9_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP8_DROP [08:08] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP8_DROP_MASK 0x00000100
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP8_DROP_SHIFT 8
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP8_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP7_DROP [07:07] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP7_DROP_MASK 0x00000080
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP7_DROP_SHIFT 7
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP7_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP6_DROP [06:06] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP6_DROP_MASK 0x00000040
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP6_DROP_SHIFT 6
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP6_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP5_DROP [05:05] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP5_DROP_MASK 0x00000020
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP5_DROP_SHIFT 5
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP5_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP4_DROP [04:04] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP4_DROP_MASK 0x00000010
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP4_DROP_SHIFT 4
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP4_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP3_DROP [03:03] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP3_DROP_MASK 0x00000008
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP3_DROP_SHIFT 3
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP3_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP2_DROP [02:02] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP2_DROP_MASK 0x00000004
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP2_DROP_SHIFT 2
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP2_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP1_DROP [01:01] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP1_DROP_MASK 0x00000002
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP1_DROP_SHIFT 1
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP1_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_IBP_DROP :: MPOD_BAND_ID_IBP0_DROP [00:00] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP0_DROP_MASK 0x00000001
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP0_DROP_SHIFT 0
#define BCHP_XPT_MPOD_MPOD_BAND_ID_IBP_DROP_MPOD_BAND_ID_IBP0_DROP_DEFAULT 0x00000000

/***************************************************************************
 *MPOD_BAND_ID_PBP_DROP - MPOD Band ID PBP Disable
 ***************************************************************************/
/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP31_DROP [31:31] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP31_DROP_MASK 0x80000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP31_DROP_SHIFT 31
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP31_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP30_DROP [30:30] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP30_DROP_MASK 0x40000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP30_DROP_SHIFT 30
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP30_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP29_DROP [29:29] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP29_DROP_MASK 0x20000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP29_DROP_SHIFT 29
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP29_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP28_DROP [28:28] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP28_DROP_MASK 0x10000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP28_DROP_SHIFT 28
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP28_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP27_DROP [27:27] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP27_DROP_MASK 0x08000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP27_DROP_SHIFT 27
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP27_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP26_DROP [26:26] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP26_DROP_MASK 0x04000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP26_DROP_SHIFT 26
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP26_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP25_DROP [25:25] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP25_DROP_MASK 0x02000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP25_DROP_SHIFT 25
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP25_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP24_DROP [24:24] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP24_DROP_MASK 0x01000000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP24_DROP_SHIFT 24
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP24_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP23_DROP [23:23] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP23_DROP_MASK 0x00800000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP23_DROP_SHIFT 23
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP23_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP22_DROP [22:22] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP22_DROP_MASK 0x00400000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP22_DROP_SHIFT 22
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP22_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP21_DROP [21:21] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP21_DROP_MASK 0x00200000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP21_DROP_SHIFT 21
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP21_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP20_DROP [20:20] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP20_DROP_MASK 0x00100000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP20_DROP_SHIFT 20
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP20_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP19_DROP [19:19] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP19_DROP_MASK 0x00080000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP19_DROP_SHIFT 19
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP19_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP18_DROP [18:18] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP18_DROP_MASK 0x00040000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP18_DROP_SHIFT 18
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP18_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP17_DROP [17:17] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP17_DROP_MASK 0x00020000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP17_DROP_SHIFT 17
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP17_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP16_DROP [16:16] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP16_DROP_MASK 0x00010000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP16_DROP_SHIFT 16
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP16_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP15_DROP [15:15] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP15_DROP_MASK 0x00008000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP15_DROP_SHIFT 15
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP15_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP14_DROP [14:14] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP14_DROP_MASK 0x00004000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP14_DROP_SHIFT 14
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP14_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP13_DROP [13:13] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP13_DROP_MASK 0x00002000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP13_DROP_SHIFT 13
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP13_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP12_DROP [12:12] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP12_DROP_MASK 0x00001000
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP12_DROP_SHIFT 12
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP12_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP11_DROP [11:11] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP11_DROP_MASK 0x00000800
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP11_DROP_SHIFT 11
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP11_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP10_DROP [10:10] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP10_DROP_MASK 0x00000400
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP10_DROP_SHIFT 10
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP10_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP9_DROP [09:09] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP9_DROP_MASK 0x00000200
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP9_DROP_SHIFT 9
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP9_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP8_DROP [08:08] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP8_DROP_MASK 0x00000100
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP8_DROP_SHIFT 8
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP8_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP7_DROP [07:07] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP7_DROP_MASK 0x00000080
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP7_DROP_SHIFT 7
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP7_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP6_DROP [06:06] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP6_DROP_MASK 0x00000040
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP6_DROP_SHIFT 6
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP6_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP5_DROP [05:05] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP5_DROP_MASK 0x00000020
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP5_DROP_SHIFT 5
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP5_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP4_DROP [04:04] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP4_DROP_MASK 0x00000010
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP4_DROP_SHIFT 4
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP4_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP3_DROP [03:03] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP3_DROP_MASK 0x00000008
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP3_DROP_SHIFT 3
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP3_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP2_DROP [02:02] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP2_DROP_MASK 0x00000004
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP2_DROP_SHIFT 2
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP2_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP1_DROP [01:01] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP1_DROP_MASK 0x00000002
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP1_DROP_SHIFT 1
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP1_DROP_DEFAULT 0x00000000

/* XPT_MPOD :: MPOD_BAND_ID_PBP_DROP :: MPOD_BAND_ID_PBP0_DROP [00:00] */
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP0_DROP_MASK 0x00000001
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP0_DROP_SHIFT 0
#define BCHP_XPT_MPOD_MPOD_BAND_ID_PBP_DROP_MPOD_BAND_ID_PBP0_DROP_DEFAULT 0x00000000

/***************************************************************************
 *TV_STATUS - TV Status
 ***************************************************************************/
/* XPT_MPOD :: TV_STATUS :: reserved0 [31:09] */
#define BCHP_XPT_MPOD_TV_STATUS_reserved0_MASK                     0xfffffe00
#define BCHP_XPT_MPOD_TV_STATUS_reserved0_SHIFT                    9

/* XPT_MPOD :: TV_STATUS :: ARM_CLK_EN [08:08] */
#define BCHP_XPT_MPOD_TV_STATUS_ARM_CLK_EN_MASK                    0x00000100
#define BCHP_XPT_MPOD_TV_STATUS_ARM_CLK_EN_SHIFT                   8
#define BCHP_XPT_MPOD_TV_STATUS_ARM_CLK_EN_DEFAULT                 0x00000000

/* XPT_MPOD :: TV_STATUS :: INPUT_CRC [07:00] */
#define BCHP_XPT_MPOD_TV_STATUS_INPUT_CRC_MASK                     0x000000ff
#define BCHP_XPT_MPOD_TV_STATUS_INPUT_CRC_SHIFT                    0
#define BCHP_XPT_MPOD_TV_STATUS_INPUT_CRC_DEFAULT                  0x00000000

#endif /* #ifndef BCHP_XPT_MPOD_H__ */

/* End of File */
