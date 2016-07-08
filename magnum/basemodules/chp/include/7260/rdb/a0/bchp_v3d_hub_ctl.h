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
 * Date:           Generated on               Fri Feb 26 13:24:10 2016
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

#ifndef BCHP_V3D_HUB_CTL_H__
#define BCHP_V3D_HUB_CTL_H__

/***************************************************************************
 *V3D_HUB_CTL
 ***************************************************************************/
#define BCHP_V3D_HUB_CTL_AXICFG                  0x21200000 /* [RW] AXI configuration */
#define BCHP_V3D_HUB_CTL_UIFCFG                  0x21200004 /* [RW] UIF configuration */
#define BCHP_V3D_HUB_CTL_IDENT0                  0x21200008 /* [RO] V3D Hub Identification 0 (V3D hub block identity) */
#define BCHP_V3D_HUB_CTL_IDENT1                  0x2120000c /* [RO] V3D Hub Identification 1 (V3D Hub Configuration A) */
#define BCHP_V3D_HUB_CTL_IDENT2                  0x21200010 /* [RO] V3D Hub Identification 1 (V3D Hub Configuration B) */
#define BCHP_V3D_HUB_CTL_IDENT3                  0x21200014 /* [RO] V3D Hub Identification 3 */
#define BCHP_V3D_HUB_CTL_FPGACFG                 0x21200040 /* [RW] (FPGA-ONLY) V3D Hub FPGA dummy slave configuration */
#define BCHP_V3D_HUB_CTL_FPGACFG1                0x21200044 /* [RW] (FPGA-ONLY) V3D Hub FPGA dummy slave configuration 1 */
#define BCHP_V3D_HUB_CTL_INT_STS                 0x21200050 /* [RO] V3D Interrupt Status */
#define BCHP_V3D_HUB_CTL_INT_SET                 0x21200054 /* [WO] V3D Interrupt Set */
#define BCHP_V3D_HUB_CTL_INT_CLR                 0x21200058 /* [WO] V3D Interrupt Clear */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS             0x2120005c /* [RO] V3D Interrupt Mask Status */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET             0x21200060 /* [WO] V3D Interrupt Mask Set */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR             0x21200064 /* [WO] V3D Interrupt Mask Clear */
#define BCHP_V3D_HUB_CTL_INT_STS_PCI             0x21200068 /* [RO] Duplicate V3D Interrupt Status, reads 0 when BCG_INT in IDENT2 is 0 */
#define BCHP_V3D_HUB_CTL_INT_SET_PCI             0x2120006c /* [WO] Duplicate V3D Interrupt Set, write has no effect when BCG_INT in IDENT2 is 0 */
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI             0x21200070 /* [WO] Duplicate V3D Interrupt Clear, write has no effect when BCG_INT in IDENT2 is 0 */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI         0x21200074 /* [RO] Duplicate V3D Interrupt Mask Status, reads 0 when BCG_INT in IDENT2 is 0 */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI         0x21200078 /* [WO] Duplicate V3D Interrupt Mask Set, write has no effect when BCG_INT in IDENT2 is 0 */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI         0x2120007c /* [WO] Duplicate V3D Interrupt Mask Clear, write has noeffect when BCG_INT in IDENT2 is 0 */

/***************************************************************************
 *AXICFG - AXI configuration
 ***************************************************************************/
/* V3D_HUB_CTL :: AXICFG :: reserved0 [31:08] */
#define BCHP_V3D_HUB_CTL_AXICFG_reserved0_MASK                     0xffffff00
#define BCHP_V3D_HUB_CTL_AXICFG_reserved0_SHIFT                    8

/* V3D_HUB_CTL :: AXICFG :: QOS [07:04] */
#define BCHP_V3D_HUB_CTL_AXICFG_QOS_MASK                           0x000000f0
#define BCHP_V3D_HUB_CTL_AXICFG_QOS_SHIFT                          4
#define BCHP_V3D_HUB_CTL_AXICFG_QOS_DEFAULT                        0x00000000

/* V3D_HUB_CTL :: AXICFG :: MAXLEN [03:00] */
#define BCHP_V3D_HUB_CTL_AXICFG_MAXLEN_MASK                        0x0000000f
#define BCHP_V3D_HUB_CTL_AXICFG_MAXLEN_SHIFT                       0
#define BCHP_V3D_HUB_CTL_AXICFG_MAXLEN_DEFAULT                     0x0000000f

/***************************************************************************
 *UIFCFG - UIF configuration
 ***************************************************************************/
/* V3D_HUB_CTL :: UIFCFG :: reserved0 [31:08] */
#define BCHP_V3D_HUB_CTL_UIFCFG_reserved0_MASK                     0xffffff00
#define BCHP_V3D_HUB_CTL_UIFCFG_reserved0_SHIFT                    8

/* V3D_HUB_CTL :: UIFCFG :: XORADDR [07:04] */
#define BCHP_V3D_HUB_CTL_UIFCFG_XORADDR_MASK                       0x000000f0
#define BCHP_V3D_HUB_CTL_UIFCFG_XORADDR_SHIFT                      4
#define BCHP_V3D_HUB_CTL_UIFCFG_XORADDR_DEFAULT                    0x00000004

/* V3D_HUB_CTL :: UIFCFG :: NUMBANKS [03:02] */
#define BCHP_V3D_HUB_CTL_UIFCFG_NUMBANKS_MASK                      0x0000000c
#define BCHP_V3D_HUB_CTL_UIFCFG_NUMBANKS_SHIFT                     2
#define BCHP_V3D_HUB_CTL_UIFCFG_NUMBANKS_DEFAULT                   0x00000001

/* V3D_HUB_CTL :: UIFCFG :: PAGESIZE [01:00] */
#define BCHP_V3D_HUB_CTL_UIFCFG_PAGESIZE_MASK                      0x00000003
#define BCHP_V3D_HUB_CTL_UIFCFG_PAGESIZE_SHIFT                     0
#define BCHP_V3D_HUB_CTL_UIFCFG_PAGESIZE_DEFAULT                   0x00000001

/***************************************************************************
 *IDENT0 - V3D Hub Identification 0 (V3D hub block identity)
 ***************************************************************************/
/* V3D_HUB_CTL :: IDENT0 :: IDSTR [31:00] */
#define BCHP_V3D_HUB_CTL_IDENT0_IDSTR_MASK                         0xffffffff
#define BCHP_V3D_HUB_CTL_IDENT0_IDSTR_SHIFT                        0
#define BCHP_V3D_HUB_CTL_IDENT0_IDSTR_DEFAULT                      0x42554856

/***************************************************************************
 *IDENT1 - V3D Hub Identification 1 (V3D Hub Configuration A)
 ***************************************************************************/
/* V3D_HUB_CTL :: IDENT1 :: reserved0 [31:28] */
#define BCHP_V3D_HUB_CTL_IDENT1_reserved0_MASK                     0xf0000000
#define BCHP_V3D_HUB_CTL_IDENT1_reserved0_SHIFT                    28

/* V3D_HUB_CTL :: IDENT1 :: L3C_BANKS [27:24] */
#define BCHP_V3D_HUB_CTL_IDENT1_L3C_BANKS_MASK                     0x0f000000
#define BCHP_V3D_HUB_CTL_IDENT1_L3C_BANKS_SHIFT                    24
#define BCHP_V3D_HUB_CTL_IDENT1_L3C_BANKS_DEFAULT                  0x00000004

/* V3D_HUB_CTL :: IDENT1 :: L3C_ASSOC [23:20] */
#define BCHP_V3D_HUB_CTL_IDENT1_L3C_ASSOC_MASK                     0x00f00000
#define BCHP_V3D_HUB_CTL_IDENT1_L3C_ASSOC_SHIFT                    20
#define BCHP_V3D_HUB_CTL_IDENT1_L3C_ASSOC_DEFAULT                  0x00000008

/* V3D_HUB_CTL :: IDENT1 :: WITH_MSO [19:19] */
#define BCHP_V3D_HUB_CTL_IDENT1_WITH_MSO_MASK                      0x00080000
#define BCHP_V3D_HUB_CTL_IDENT1_WITH_MSO_SHIFT                     19
#define BCHP_V3D_HUB_CTL_IDENT1_WITH_MSO_DEFAULT                   0x00000001

/* V3D_HUB_CTL :: IDENT1 :: WITH_TSY [18:18] */
#define BCHP_V3D_HUB_CTL_IDENT1_WITH_TSY_MASK                      0x00040000
#define BCHP_V3D_HUB_CTL_IDENT1_WITH_TSY_SHIFT                     18
#define BCHP_V3D_HUB_CTL_IDENT1_WITH_TSY_DEFAULT                   0x00000001

/* V3D_HUB_CTL :: IDENT1 :: WITH_TFU [17:17] */
#define BCHP_V3D_HUB_CTL_IDENT1_WITH_TFU_MASK                      0x00020000
#define BCHP_V3D_HUB_CTL_IDENT1_WITH_TFU_SHIFT                     17
#define BCHP_V3D_HUB_CTL_IDENT1_WITH_TFU_DEFAULT                   0x00000001

/* V3D_HUB_CTL :: IDENT1 :: WITH_L3C [16:16] */
#define BCHP_V3D_HUB_CTL_IDENT1_WITH_L3C_MASK                      0x00010000
#define BCHP_V3D_HUB_CTL_IDENT1_WITH_L3C_SHIFT                     16
#define BCHP_V3D_HUB_CTL_IDENT1_WITH_L3C_DEFAULT                   0x00000000

/* V3D_HUB_CTL :: IDENT1 :: NHOSTS [15:12] */
#define BCHP_V3D_HUB_CTL_IDENT1_NHOSTS_MASK                        0x0000f000
#define BCHP_V3D_HUB_CTL_IDENT1_NHOSTS_SHIFT                       12
#define BCHP_V3D_HUB_CTL_IDENT1_NHOSTS_DEFAULT                     0x00000001

/* V3D_HUB_CTL :: IDENT1 :: NCORES [11:08] */
#define BCHP_V3D_HUB_CTL_IDENT1_NCORES_MASK                        0x00000f00
#define BCHP_V3D_HUB_CTL_IDENT1_NCORES_SHIFT                       8
#define BCHP_V3D_HUB_CTL_IDENT1_NCORES_DEFAULT                     0x00000001

/* V3D_HUB_CTL :: IDENT1 :: REV [07:04] */
#define BCHP_V3D_HUB_CTL_IDENT1_REV_MASK                           0x000000f0
#define BCHP_V3D_HUB_CTL_IDENT1_REV_SHIFT                          4
#define BCHP_V3D_HUB_CTL_IDENT1_REV_DEFAULT                        0x00000003

/* V3D_HUB_CTL :: IDENT1 :: TVER [03:00] */
#define BCHP_V3D_HUB_CTL_IDENT1_TVER_MASK                          0x0000000f
#define BCHP_V3D_HUB_CTL_IDENT1_TVER_SHIFT                         0
#define BCHP_V3D_HUB_CTL_IDENT1_TVER_DEFAULT                       0x00000003

/***************************************************************************
 *IDENT2 - V3D Hub Identification 1 (V3D Hub Configuration B)
 ***************************************************************************/
/* V3D_HUB_CTL :: IDENT2 :: reserved0 [31:09] */
#define BCHP_V3D_HUB_CTL_IDENT2_reserved0_MASK                     0xfffffe00
#define BCHP_V3D_HUB_CTL_IDENT2_reserved0_SHIFT                    9

/* V3D_HUB_CTL :: IDENT2 :: WITH_MMU [08:08] */
#define BCHP_V3D_HUB_CTL_IDENT2_WITH_MMU_MASK                      0x00000100
#define BCHP_V3D_HUB_CTL_IDENT2_WITH_MMU_SHIFT                     8
#define BCHP_V3D_HUB_CTL_IDENT2_WITH_MMU_DEFAULT                   0x00000001

/* V3D_HUB_CTL :: IDENT2 :: L3C_NKB [07:00] */
#define BCHP_V3D_HUB_CTL_IDENT2_L3C_NKB_MASK                       0x000000ff
#define BCHP_V3D_HUB_CTL_IDENT2_L3C_NKB_SHIFT                      0
#define BCHP_V3D_HUB_CTL_IDENT2_L3C_NKB_DEFAULT                    0x00000004

/***************************************************************************
 *IDENT3 - V3D Hub Identification 3
 ***************************************************************************/
/* V3D_HUB_CTL :: IDENT3 :: reserved0 [31:16] */
#define BCHP_V3D_HUB_CTL_IDENT3_reserved0_MASK                     0xffff0000
#define BCHP_V3D_HUB_CTL_IDENT3_reserved0_SHIFT                    16

/* V3D_HUB_CTL :: IDENT3 :: IPREV [15:08] */
#define BCHP_V3D_HUB_CTL_IDENT3_IPREV_MASK                         0x0000ff00
#define BCHP_V3D_HUB_CTL_IDENT3_IPREV_SHIFT                        8
#define BCHP_V3D_HUB_CTL_IDENT3_IPREV_DEFAULT                      0x00000000

/* V3D_HUB_CTL :: IDENT3 :: IPIDX [07:00] */
#define BCHP_V3D_HUB_CTL_IDENT3_IPIDX_MASK                         0x000000ff
#define BCHP_V3D_HUB_CTL_IDENT3_IPIDX_SHIFT                        0
#define BCHP_V3D_HUB_CTL_IDENT3_IPIDX_DEFAULT                      0x00000001

/***************************************************************************
 *FPGACFG - (FPGA-ONLY) V3D Hub FPGA dummy slave configuration
 ***************************************************************************/
/* V3D_HUB_CTL :: FPGACFG :: DUMMY_SLV_REGION [31:02] */
#define BCHP_V3D_HUB_CTL_FPGACFG_DUMMY_SLV_REGION_MASK             0xfffffffc
#define BCHP_V3D_HUB_CTL_FPGACFG_DUMMY_SLV_REGION_SHIFT            2
#define BCHP_V3D_HUB_CTL_FPGACFG_DUMMY_SLV_REGION_DEFAULT          0x00000000

/* V3D_HUB_CTL :: FPGACFG :: reserved0 [01:01] */
#define BCHP_V3D_HUB_CTL_FPGACFG_reserved0_MASK                    0x00000002
#define BCHP_V3D_HUB_CTL_FPGACFG_reserved0_SHIFT                   1

/* V3D_HUB_CTL :: FPGACFG :: DUMMY_SLV_EN [00:00] */
#define BCHP_V3D_HUB_CTL_FPGACFG_DUMMY_SLV_EN_MASK                 0x00000001
#define BCHP_V3D_HUB_CTL_FPGACFG_DUMMY_SLV_EN_SHIFT                0
#define BCHP_V3D_HUB_CTL_FPGACFG_DUMMY_SLV_EN_DEFAULT              0x00000001

/***************************************************************************
 *FPGACFG1 - (FPGA-ONLY) V3D Hub FPGA dummy slave configuration 1
 ***************************************************************************/
/* V3D_HUB_CTL :: FPGACFG1 :: DUMMY_SLV_MASK [31:00] */
#define BCHP_V3D_HUB_CTL_FPGACFG1_DUMMY_SLV_MASK_MASK              0xffffffff
#define BCHP_V3D_HUB_CTL_FPGACFG1_DUMMY_SLV_MASK_SHIFT             0
#define BCHP_V3D_HUB_CTL_FPGACFG1_DUMMY_SLV_MASK_DEFAULT           0xffffffff

/***************************************************************************
 *INT_STS - V3D Interrupt Status
 ***************************************************************************/
/* V3D_HUB_CTL :: INT_STS :: reserved0 [31:06] */
#define BCHP_V3D_HUB_CTL_INT_STS_reserved0_MASK                    0xffffffc0
#define BCHP_V3D_HUB_CTL_INT_STS_reserved0_SHIFT                   6

/* V3D_HUB_CTL :: INT_STS :: INT_MMU_WRV [05:05] */
#define BCHP_V3D_HUB_CTL_INT_STS_INT_MMU_WRV_MASK                  0x00000020
#define BCHP_V3D_HUB_CTL_INT_STS_INT_MMU_WRV_SHIFT                 5
#define BCHP_V3D_HUB_CTL_INT_STS_INT_MMU_WRV_DEFAULT               0x00000000

/* V3D_HUB_CTL :: INT_STS :: INT_MMU_PTI [04:04] */
#define BCHP_V3D_HUB_CTL_INT_STS_INT_MMU_PTI_MASK                  0x00000010
#define BCHP_V3D_HUB_CTL_INT_STS_INT_MMU_PTI_SHIFT                 4
#define BCHP_V3D_HUB_CTL_INT_STS_INT_MMU_PTI_DEFAULT               0x00000000

/* V3D_HUB_CTL :: INT_STS :: INT_MMU_CAP [03:03] */
#define BCHP_V3D_HUB_CTL_INT_STS_INT_MMU_CAP_MASK                  0x00000008
#define BCHP_V3D_HUB_CTL_INT_STS_INT_MMU_CAP_SHIFT                 3
#define BCHP_V3D_HUB_CTL_INT_STS_INT_MMU_CAP_DEFAULT               0x00000000

/* V3D_HUB_CTL :: INT_STS :: INT_MSO [02:02] */
#define BCHP_V3D_HUB_CTL_INT_STS_INT_MSO_MASK                      0x00000004
#define BCHP_V3D_HUB_CTL_INT_STS_INT_MSO_SHIFT                     2

/* V3D_HUB_CTL :: INT_STS :: INT_TFUC [01:01] */
#define BCHP_V3D_HUB_CTL_INT_STS_INT_TFUC_MASK                     0x00000002
#define BCHP_V3D_HUB_CTL_INT_STS_INT_TFUC_SHIFT                    1
#define BCHP_V3D_HUB_CTL_INT_STS_INT_TFUC_DEFAULT                  0x00000000

/* V3D_HUB_CTL :: INT_STS :: INT_TFUF [00:00] */
#define BCHP_V3D_HUB_CTL_INT_STS_INT_TFUF_MASK                     0x00000001
#define BCHP_V3D_HUB_CTL_INT_STS_INT_TFUF_SHIFT                    0
#define BCHP_V3D_HUB_CTL_INT_STS_INT_TFUF_DEFAULT                  0x00000000

/***************************************************************************
 *INT_SET - V3D Interrupt Set
 ***************************************************************************/
/* V3D_HUB_CTL :: INT_SET :: reserved0 [31:06] */
#define BCHP_V3D_HUB_CTL_INT_SET_reserved0_MASK                    0xffffffc0
#define BCHP_V3D_HUB_CTL_INT_SET_reserved0_SHIFT                   6

/* V3D_HUB_CTL :: INT_SET :: INT_MMU_WRV [05:05] */
#define BCHP_V3D_HUB_CTL_INT_SET_INT_MMU_WRV_MASK                  0x00000020
#define BCHP_V3D_HUB_CTL_INT_SET_INT_MMU_WRV_SHIFT                 5
#define BCHP_V3D_HUB_CTL_INT_SET_INT_MMU_WRV_DEFAULT               0x00000000

/* V3D_HUB_CTL :: INT_SET :: INT_MMU_PTI [04:04] */
#define BCHP_V3D_HUB_CTL_INT_SET_INT_MMU_PTI_MASK                  0x00000010
#define BCHP_V3D_HUB_CTL_INT_SET_INT_MMU_PTI_SHIFT                 4
#define BCHP_V3D_HUB_CTL_INT_SET_INT_MMU_PTI_DEFAULT               0x00000000

/* V3D_HUB_CTL :: INT_SET :: INT_MMU_CAP [03:03] */
#define BCHP_V3D_HUB_CTL_INT_SET_INT_MMU_CAP_MASK                  0x00000008
#define BCHP_V3D_HUB_CTL_INT_SET_INT_MMU_CAP_SHIFT                 3
#define BCHP_V3D_HUB_CTL_INT_SET_INT_MMU_CAP_DEFAULT               0x00000000

/* V3D_HUB_CTL :: INT_SET :: INT_MSO [02:02] */
#define BCHP_V3D_HUB_CTL_INT_SET_INT_MSO_MASK                      0x00000004
#define BCHP_V3D_HUB_CTL_INT_SET_INT_MSO_SHIFT                     2

/* V3D_HUB_CTL :: INT_SET :: INT_TFUC [01:01] */
#define BCHP_V3D_HUB_CTL_INT_SET_INT_TFUC_MASK                     0x00000002
#define BCHP_V3D_HUB_CTL_INT_SET_INT_TFUC_SHIFT                    1

/* V3D_HUB_CTL :: INT_SET :: INT_TFUF [00:00] */
#define BCHP_V3D_HUB_CTL_INT_SET_INT_TFUF_MASK                     0x00000001
#define BCHP_V3D_HUB_CTL_INT_SET_INT_TFUF_SHIFT                    0

/***************************************************************************
 *INT_CLR - V3D Interrupt Clear
 ***************************************************************************/
/* V3D_HUB_CTL :: INT_CLR :: reserved0 [31:06] */
#define BCHP_V3D_HUB_CTL_INT_CLR_reserved0_MASK                    0xffffffc0
#define BCHP_V3D_HUB_CTL_INT_CLR_reserved0_SHIFT                   6

/* V3D_HUB_CTL :: INT_CLR :: INT_MMU_WRV [05:05] */
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_MMU_WRV_MASK                  0x00000020
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_MMU_WRV_SHIFT                 5
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_MMU_WRV_DEFAULT               0x00000000

/* V3D_HUB_CTL :: INT_CLR :: INT_MMU_PTI [04:04] */
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_MMU_PTI_MASK                  0x00000010
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_MMU_PTI_SHIFT                 4
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_MMU_PTI_DEFAULT               0x00000000

/* V3D_HUB_CTL :: INT_CLR :: INT_MMU_CAP [03:03] */
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_MMU_CAP_MASK                  0x00000008
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_MMU_CAP_SHIFT                 3

/* V3D_HUB_CTL :: INT_CLR :: INT_MSO [02:02] */
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_MSO_MASK                      0x00000004
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_MSO_SHIFT                     2

/* V3D_HUB_CTL :: INT_CLR :: INT_TFUC [01:01] */
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_TFUC_MASK                     0x00000002
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_TFUC_SHIFT                    1

/* V3D_HUB_CTL :: INT_CLR :: INT_TFUF [00:00] */
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_TFUF_MASK                     0x00000001
#define BCHP_V3D_HUB_CTL_INT_CLR_INT_TFUF_SHIFT                    0

/***************************************************************************
 *INT_MSK_STS - V3D Interrupt Mask Status
 ***************************************************************************/
/* V3D_HUB_CTL :: INT_MSK_STS :: reserved0 [31:06] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_reserved0_MASK                0xffffffc0
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_reserved0_SHIFT               6

/* V3D_HUB_CTL :: INT_MSK_STS :: INT_MMU_WRV [05:05] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_MMU_WRV_MASK              0x00000020
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_MMU_WRV_SHIFT             5
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_MMU_WRV_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_MSK_STS :: INT_MMU_PTI [04:04] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_MMU_PTI_MASK              0x00000010
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_MMU_PTI_SHIFT             4
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_MMU_PTI_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_MSK_STS :: INT_MMU_CAP [03:03] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_MMU_CAP_MASK              0x00000008
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_MMU_CAP_SHIFT             3

/* V3D_HUB_CTL :: INT_MSK_STS :: INT_MSO [02:02] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_MSO_MASK                  0x00000004
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_MSO_SHIFT                 2

/* V3D_HUB_CTL :: INT_MSK_STS :: INT_TFUC [01:01] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_TFUC_MASK                 0x00000002
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_TFUC_SHIFT                1
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_TFUC_DEFAULT              0x00000000

/* V3D_HUB_CTL :: INT_MSK_STS :: INT_TFUF [00:00] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_TFUF_MASK                 0x00000001
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_TFUF_SHIFT                0
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_INT_TFUF_DEFAULT              0x00000000

/***************************************************************************
 *INT_MSK_SET - V3D Interrupt Mask Set
 ***************************************************************************/
/* V3D_HUB_CTL :: INT_MSK_SET :: reserved0 [31:06] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_reserved0_MASK                0xffffffc0
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_reserved0_SHIFT               6

/* V3D_HUB_CTL :: INT_MSK_SET :: INT_MMU_WRV [05:05] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_MMU_WRV_MASK              0x00000020
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_MMU_WRV_SHIFT             5
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_MMU_WRV_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_MSK_SET :: INT_MMU_PTI [04:04] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_MMU_PTI_MASK              0x00000010
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_MMU_PTI_SHIFT             4
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_MMU_PTI_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_MSK_SET :: INT_MMU_CAP [03:03] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_MMU_CAP_MASK              0x00000008
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_MMU_CAP_SHIFT             3

/* V3D_HUB_CTL :: INT_MSK_SET :: INT_MSO [02:02] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_MSO_MASK                  0x00000004
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_MSO_SHIFT                 2

/* V3D_HUB_CTL :: INT_MSK_SET :: INT_TFUC [01:01] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_TFUC_MASK                 0x00000002
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_TFUC_SHIFT                1

/* V3D_HUB_CTL :: INT_MSK_SET :: INT_TFUF [00:00] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_TFUF_MASK                 0x00000001
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_INT_TFUF_SHIFT                0

/***************************************************************************
 *INT_MSK_CLR - V3D Interrupt Mask Clear
 ***************************************************************************/
/* V3D_HUB_CTL :: INT_MSK_CLR :: reserved0 [31:06] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_reserved0_MASK                0xffffffc0
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_reserved0_SHIFT               6

/* V3D_HUB_CTL :: INT_MSK_CLR :: INT_MMU_WRV [05:05] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_MMU_WRV_MASK              0x00000020
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_MMU_WRV_SHIFT             5
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_MMU_WRV_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_MSK_CLR :: INT_MMU_PTI [04:04] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_MMU_PTI_MASK              0x00000010
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_MMU_PTI_SHIFT             4
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_MMU_PTI_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_MSK_CLR :: INT_MMU_CAP [03:03] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_MMU_CAP_MASK              0x00000008
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_MMU_CAP_SHIFT             3

/* V3D_HUB_CTL :: INT_MSK_CLR :: INT_MSO [02:02] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_MSO_MASK                  0x00000004
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_MSO_SHIFT                 2

/* V3D_HUB_CTL :: INT_MSK_CLR :: INT_TFUC [01:01] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_TFUC_MASK                 0x00000002
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_TFUC_SHIFT                1

/* V3D_HUB_CTL :: INT_MSK_CLR :: INT_TFUF [00:00] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_TFUF_MASK                 0x00000001
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_INT_TFUF_SHIFT                0

/***************************************************************************
 *INT_STS_PCI - Duplicate V3D Interrupt Status, reads 0 when BCG_INT in IDENT2 is 0
 ***************************************************************************/
/* V3D_HUB_CTL :: INT_STS_PCI :: reserved0 [31:06] */
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_reserved0_MASK                0xffffffc0
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_reserved0_SHIFT               6

/* V3D_HUB_CTL :: INT_STS_PCI :: INT_MMU_WRV [05:05] */
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_MMU_WRV_MASK              0x00000020
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_MMU_WRV_SHIFT             5
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_MMU_WRV_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_STS_PCI :: INT_MMU_PTI [04:04] */
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_MMU_PTI_MASK              0x00000010
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_MMU_PTI_SHIFT             4
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_MMU_PTI_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_STS_PCI :: INT_MMU_CAP [03:03] */
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_MMU_CAP_MASK              0x00000008
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_MMU_CAP_SHIFT             3
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_MMU_CAP_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_STS_PCI :: INT_MSO [02:02] */
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_MSO_MASK                  0x00000004
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_MSO_SHIFT                 2

/* V3D_HUB_CTL :: INT_STS_PCI :: INT_TFUC [01:01] */
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_TFUC_MASK                 0x00000002
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_TFUC_SHIFT                1
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_TFUC_DEFAULT              0x00000000

/* V3D_HUB_CTL :: INT_STS_PCI :: INT_TFUF [00:00] */
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_TFUF_MASK                 0x00000001
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_TFUF_SHIFT                0
#define BCHP_V3D_HUB_CTL_INT_STS_PCI_INT_TFUF_DEFAULT              0x00000000

/***************************************************************************
 *INT_SET_PCI - Duplicate V3D Interrupt Set, write has no effect when BCG_INT in IDENT2 is 0
 ***************************************************************************/
/* V3D_HUB_CTL :: INT_SET_PCI :: reserved0 [31:06] */
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_reserved0_MASK                0xffffffc0
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_reserved0_SHIFT               6

/* V3D_HUB_CTL :: INT_SET_PCI :: INT_MMU_WRV [05:05] */
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_MMU_WRV_MASK              0x00000020
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_MMU_WRV_SHIFT             5
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_MMU_WRV_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_SET_PCI :: INT_MMU_PTI [04:04] */
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_MMU_PTI_MASK              0x00000010
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_MMU_PTI_SHIFT             4
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_MMU_PTI_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_SET_PCI :: INT_MMU_CAP [03:03] */
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_MMU_CAP_MASK              0x00000008
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_MMU_CAP_SHIFT             3
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_MMU_CAP_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_SET_PCI :: INT_MSO [02:02] */
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_MSO_MASK                  0x00000004
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_MSO_SHIFT                 2

/* V3D_HUB_CTL :: INT_SET_PCI :: INT_TFUC [01:01] */
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_TFUC_MASK                 0x00000002
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_TFUC_SHIFT                1

/* V3D_HUB_CTL :: INT_SET_PCI :: INT_TFUF [00:00] */
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_TFUF_MASK                 0x00000001
#define BCHP_V3D_HUB_CTL_INT_SET_PCI_INT_TFUF_SHIFT                0

/***************************************************************************
 *INT_CLR_PCI - Duplicate V3D Interrupt Clear, write has no effect when BCG_INT in IDENT2 is 0
 ***************************************************************************/
/* V3D_HUB_CTL :: INT_CLR_PCI :: reserved0 [31:06] */
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_reserved0_MASK                0xffffffc0
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_reserved0_SHIFT               6

/* V3D_HUB_CTL :: INT_CLR_PCI :: INT_MMU_WRV [05:05] */
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_MMU_WRV_MASK              0x00000020
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_MMU_WRV_SHIFT             5
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_MMU_WRV_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_CLR_PCI :: INT_MMU_PTI [04:04] */
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_MMU_PTI_MASK              0x00000010
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_MMU_PTI_SHIFT             4
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_MMU_PTI_DEFAULT           0x00000000

/* V3D_HUB_CTL :: INT_CLR_PCI :: INT_MMU_CAP [03:03] */
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_MMU_CAP_MASK              0x00000008
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_MMU_CAP_SHIFT             3

/* V3D_HUB_CTL :: INT_CLR_PCI :: INT_MSO [02:02] */
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_MSO_MASK                  0x00000004
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_MSO_SHIFT                 2

/* V3D_HUB_CTL :: INT_CLR_PCI :: INT_TFUC [01:01] */
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_TFUC_MASK                 0x00000002
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_TFUC_SHIFT                1

/* V3D_HUB_CTL :: INT_CLR_PCI :: INT_TFUF [00:00] */
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_TFUF_MASK                 0x00000001
#define BCHP_V3D_HUB_CTL_INT_CLR_PCI_INT_TFUF_SHIFT                0

/***************************************************************************
 *INT_MSK_STS_PCI - Duplicate V3D Interrupt Mask Status, reads 0 when BCG_INT in IDENT2 is 0
 ***************************************************************************/
/* V3D_HUB_CTL :: INT_MSK_STS_PCI :: reserved0 [31:06] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_reserved0_MASK            0xffffffc0
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_reserved0_SHIFT           6

/* V3D_HUB_CTL :: INT_MSK_STS_PCI :: INT_MMU_WRV [05:05] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_MMU_WRV_MASK          0x00000020
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_MMU_WRV_SHIFT         5
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_MMU_WRV_DEFAULT       0x00000000

/* V3D_HUB_CTL :: INT_MSK_STS_PCI :: INT_MMU_PTI [04:04] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_MMU_PTI_MASK          0x00000010
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_MMU_PTI_SHIFT         4
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_MMU_PTI_DEFAULT       0x00000000

/* V3D_HUB_CTL :: INT_MSK_STS_PCI :: INT_MMU_CAP [03:03] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_MMU_CAP_MASK          0x00000008
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_MMU_CAP_SHIFT         3

/* V3D_HUB_CTL :: INT_MSK_STS_PCI :: INT_MSO [02:02] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_MSO_MASK              0x00000004
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_MSO_SHIFT             2

/* V3D_HUB_CTL :: INT_MSK_STS_PCI :: INT_TFUC [01:01] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_TFUC_MASK             0x00000002
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_TFUC_SHIFT            1
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_TFUC_DEFAULT          0x00000000

/* V3D_HUB_CTL :: INT_MSK_STS_PCI :: INT_TFUF [00:00] */
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_TFUF_MASK             0x00000001
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_TFUF_SHIFT            0
#define BCHP_V3D_HUB_CTL_INT_MSK_STS_PCI_INT_TFUF_DEFAULT          0x00000000

/***************************************************************************
 *INT_MSK_SET_PCI - Duplicate V3D Interrupt Mask Set, write has no effect when BCG_INT in IDENT2 is 0
 ***************************************************************************/
/* V3D_HUB_CTL :: INT_MSK_SET_PCI :: reserved0 [31:06] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_reserved0_MASK            0xffffffc0
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_reserved0_SHIFT           6

/* V3D_HUB_CTL :: INT_MSK_SET_PCI :: INT_MMU_WRV [05:05] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_MMU_WRV_MASK          0x00000020
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_MMU_WRV_SHIFT         5
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_MMU_WRV_DEFAULT       0x00000000

/* V3D_HUB_CTL :: INT_MSK_SET_PCI :: INT_MMU_PTI [04:04] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_MMU_PTI_MASK          0x00000010
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_MMU_PTI_SHIFT         4
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_MMU_PTI_DEFAULT       0x00000000

/* V3D_HUB_CTL :: INT_MSK_SET_PCI :: INT_MMU_CAP [03:03] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_MMU_CAP_MASK          0x00000008
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_MMU_CAP_SHIFT         3

/* V3D_HUB_CTL :: INT_MSK_SET_PCI :: INT_MSO [02:02] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_MSO_MASK              0x00000004
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_MSO_SHIFT             2

/* V3D_HUB_CTL :: INT_MSK_SET_PCI :: INT_TFUC [01:01] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_TFUC_MASK             0x00000002
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_TFUC_SHIFT            1

/* V3D_HUB_CTL :: INT_MSK_SET_PCI :: INT_TFUF [00:00] */
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_TFUF_MASK             0x00000001
#define BCHP_V3D_HUB_CTL_INT_MSK_SET_PCI_INT_TFUF_SHIFT            0

/***************************************************************************
 *INT_MSK_CLR_PCI - Duplicate V3D Interrupt Mask Clear, write has noeffect when BCG_INT in IDENT2 is 0
 ***************************************************************************/
/* V3D_HUB_CTL :: INT_MSK_CLR_PCI :: reserved0 [31:06] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_reserved0_MASK            0xffffffc0
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_reserved0_SHIFT           6

/* V3D_HUB_CTL :: INT_MSK_CLR_PCI :: INT_MMU_WRV [05:05] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_MMU_WRV_MASK          0x00000020
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_MMU_WRV_SHIFT         5
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_MMU_WRV_DEFAULT       0x00000000

/* V3D_HUB_CTL :: INT_MSK_CLR_PCI :: INT_MMU_PTI [04:04] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_MMU_PTI_MASK          0x00000010
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_MMU_PTI_SHIFT         4
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_MMU_PTI_DEFAULT       0x00000000

/* V3D_HUB_CTL :: INT_MSK_CLR_PCI :: INT_MMU_CAP [03:03] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_MMU_CAP_MASK          0x00000008
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_MMU_CAP_SHIFT         3

/* V3D_HUB_CTL :: INT_MSK_CLR_PCI :: INT_MSO [02:02] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_MSO_MASK              0x00000004
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_MSO_SHIFT             2

/* V3D_HUB_CTL :: INT_MSK_CLR_PCI :: INT_TFUC [01:01] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_TFUC_MASK             0x00000002
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_TFUC_SHIFT            1

/* V3D_HUB_CTL :: INT_MSK_CLR_PCI :: INT_TFUF [00:00] */
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_TFUF_MASK             0x00000001
#define BCHP_V3D_HUB_CTL_INT_MSK_CLR_PCI_INT_TFUF_SHIFT            0

#endif /* #ifndef BCHP_V3D_HUB_CTL_H__ */

/* End of File */
