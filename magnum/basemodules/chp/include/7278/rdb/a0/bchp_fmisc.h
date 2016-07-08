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
 * Date:           Generated on               Mon Mar 21 13:44:44 2016
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

#ifndef BCHP_FMISC_H__
#define BCHP_FMISC_H__

/***************************************************************************
 *FMISC - BVN Front Control Registers
 ***************************************************************************/
#define BCHP_FMISC_SW_INIT                       0x00006000 /* [CFG] BVN Front Soft Init */
#define BCHP_FMISC_BVN_PDA_CTRL                  0x0000600c /* [CFG] BVN PDA Control */
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS           0x00006010 /* [RO] BVN Front PDA Out Status */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS        0x00006014 /* [RO] BVN Front PDA Power Up Status */
#define BCHP_FMISC_BVNF_CLOCK_CTRL               0x00006018 /* [CFG] BVN Front clock control register */
#define BCHP_FMISC_SCRATCH_0                     0x00006020 /* [CFG] Scratch Register */

/***************************************************************************
 *SW_INIT - BVN Front Soft Init
 ***************************************************************************/
/* FMISC :: SW_INIT :: reserved0 [31:25] */
#define BCHP_FMISC_SW_INIT_reserved0_MASK                          0xfe000000
#define BCHP_FMISC_SW_INIT_reserved0_SHIFT                         25

/* FMISC :: SW_INIT :: RDC [24:24] */
#define BCHP_FMISC_SW_INIT_RDC_MASK                                0x01000000
#define BCHP_FMISC_SW_INIT_RDC_SHIFT                               24
#define BCHP_FMISC_SW_INIT_RDC_DEFAULT                             0x00000000

/* FMISC :: SW_INIT :: reserved1 [23:12] */
#define BCHP_FMISC_SW_INIT_reserved1_MASK                          0x00fff000
#define BCHP_FMISC_SW_INIT_reserved1_SHIFT                         12

/* FMISC :: SW_INIT :: VFD_3 [11:11] */
#define BCHP_FMISC_SW_INIT_VFD_3_MASK                              0x00000800
#define BCHP_FMISC_SW_INIT_VFD_3_SHIFT                             11
#define BCHP_FMISC_SW_INIT_VFD_3_DEFAULT                           0x00000000

/* FMISC :: SW_INIT :: VFD_2 [10:10] */
#define BCHP_FMISC_SW_INIT_VFD_2_MASK                              0x00000400
#define BCHP_FMISC_SW_INIT_VFD_2_SHIFT                             10
#define BCHP_FMISC_SW_INIT_VFD_2_DEFAULT                           0x00000000

/* FMISC :: SW_INIT :: VFD_1 [09:09] */
#define BCHP_FMISC_SW_INIT_VFD_1_MASK                              0x00000200
#define BCHP_FMISC_SW_INIT_VFD_1_SHIFT                             9
#define BCHP_FMISC_SW_INIT_VFD_1_DEFAULT                           0x00000000

/* FMISC :: SW_INIT :: VFD_0 [08:08] */
#define BCHP_FMISC_SW_INIT_VFD_0_MASK                              0x00000100
#define BCHP_FMISC_SW_INIT_VFD_0_SHIFT                             8
#define BCHP_FMISC_SW_INIT_VFD_0_DEFAULT                           0x00000000

/* FMISC :: SW_INIT :: reserved2 [07:04] */
#define BCHP_FMISC_SW_INIT_reserved2_MASK                          0x000000f0
#define BCHP_FMISC_SW_INIT_reserved2_SHIFT                         4

/* FMISC :: SW_INIT :: MFD_3 [03:03] */
#define BCHP_FMISC_SW_INIT_MFD_3_MASK                              0x00000008
#define BCHP_FMISC_SW_INIT_MFD_3_SHIFT                             3
#define BCHP_FMISC_SW_INIT_MFD_3_DEFAULT                           0x00000000

/* FMISC :: SW_INIT :: MFD_2 [02:02] */
#define BCHP_FMISC_SW_INIT_MFD_2_MASK                              0x00000004
#define BCHP_FMISC_SW_INIT_MFD_2_SHIFT                             2
#define BCHP_FMISC_SW_INIT_MFD_2_DEFAULT                           0x00000000

/* FMISC :: SW_INIT :: MFD_1 [01:01] */
#define BCHP_FMISC_SW_INIT_MFD_1_MASK                              0x00000002
#define BCHP_FMISC_SW_INIT_MFD_1_SHIFT                             1
#define BCHP_FMISC_SW_INIT_MFD_1_DEFAULT                           0x00000000

/* FMISC :: SW_INIT :: MFD_0 [00:00] */
#define BCHP_FMISC_SW_INIT_MFD_0_MASK                              0x00000001
#define BCHP_FMISC_SW_INIT_MFD_0_SHIFT                             0
#define BCHP_FMISC_SW_INIT_MFD_0_DEFAULT                           0x00000000

/***************************************************************************
 *BVN_PDA_CTRL - BVN PDA Control
 ***************************************************************************/
/* FMISC :: BVN_PDA_CTRL :: reserved0 [31:01] */
#define BCHP_FMISC_BVN_PDA_CTRL_reserved0_MASK                     0xfffffffe
#define BCHP_FMISC_BVN_PDA_CTRL_reserved0_SHIFT                    1

/* FMISC :: BVN_PDA_CTRL :: DMPG_EN [00:00] */
#define BCHP_FMISC_BVN_PDA_CTRL_DMPG_EN_MASK                       0x00000001
#define BCHP_FMISC_BVN_PDA_CTRL_DMPG_EN_SHIFT                      0
#define BCHP_FMISC_BVN_PDA_CTRL_DMPG_EN_DEFAULT                    0x00000000
#define BCHP_FMISC_BVN_PDA_CTRL_DMPG_EN_DISABLE                    0
#define BCHP_FMISC_BVN_PDA_CTRL_DMPG_EN_ENABLE                     1

/***************************************************************************
 *BVNF_PDA_OUT_STATUS - BVN Front PDA Out Status
 ***************************************************************************/
/* FMISC :: BVNF_PDA_OUT_STATUS :: MFD_0 [31:31] */
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_MFD_0_MASK                  0x80000000
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_MFD_0_SHIFT                 31

/* FMISC :: BVNF_PDA_OUT_STATUS :: MFD_1 [30:30] */
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_MFD_1_MASK                  0x40000000
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_MFD_1_SHIFT                 30

/* FMISC :: BVNF_PDA_OUT_STATUS :: MFD_2 [29:29] */
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_MFD_2_MASK                  0x20000000
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_MFD_2_SHIFT                 29

/* FMISC :: BVNF_PDA_OUT_STATUS :: MFD_3 [28:28] */
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_MFD_3_MASK                  0x10000000
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_MFD_3_SHIFT                 28

/* FMISC :: BVNF_PDA_OUT_STATUS :: reserved0 [27:24] */
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_reserved0_MASK              0x0f000000
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_reserved0_SHIFT             24

/* FMISC :: BVNF_PDA_OUT_STATUS :: VFD_0 [23:23] */
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_VFD_0_MASK                  0x00800000
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_VFD_0_SHIFT                 23

/* FMISC :: BVNF_PDA_OUT_STATUS :: VFD_1 [22:22] */
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_VFD_1_MASK                  0x00400000
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_VFD_1_SHIFT                 22

/* FMISC :: BVNF_PDA_OUT_STATUS :: VFD_2 [21:21] */
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_VFD_2_MASK                  0x00200000
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_VFD_2_SHIFT                 21

/* FMISC :: BVNF_PDA_OUT_STATUS :: VFD_3 [20:20] */
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_VFD_3_MASK                  0x00100000
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_VFD_3_SHIFT                 20

/* FMISC :: BVNF_PDA_OUT_STATUS :: reserved1 [19:01] */
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_reserved1_MASK              0x000ffffe
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_reserved1_SHIFT             1

/* FMISC :: BVNF_PDA_OUT_STATUS :: RDC_LUT [00:00] */
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_RDC_LUT_MASK                0x00000001
#define BCHP_FMISC_BVNF_PDA_OUT_STATUS_RDC_LUT_SHIFT               0

/***************************************************************************
 *BVNF_PDA_PWR_UP_STATUS - BVN Front PDA Power Up Status
 ***************************************************************************/
/* FMISC :: BVNF_PDA_PWR_UP_STATUS :: MFD_0 [31:31] */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_MFD_0_MASK               0x80000000
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_MFD_0_SHIFT              31

/* FMISC :: BVNF_PDA_PWR_UP_STATUS :: MFD_1 [30:30] */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_MFD_1_MASK               0x40000000
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_MFD_1_SHIFT              30

/* FMISC :: BVNF_PDA_PWR_UP_STATUS :: MFD_2 [29:29] */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_MFD_2_MASK               0x20000000
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_MFD_2_SHIFT              29

/* FMISC :: BVNF_PDA_PWR_UP_STATUS :: MFD_3 [28:28] */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_MFD_3_MASK               0x10000000
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_MFD_3_SHIFT              28

/* FMISC :: BVNF_PDA_PWR_UP_STATUS :: reserved0 [27:24] */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_reserved0_MASK           0x0f000000
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_reserved0_SHIFT          24

/* FMISC :: BVNF_PDA_PWR_UP_STATUS :: VFD_0 [23:23] */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_VFD_0_MASK               0x00800000
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_VFD_0_SHIFT              23

/* FMISC :: BVNF_PDA_PWR_UP_STATUS :: VFD_1 [22:22] */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_VFD_1_MASK               0x00400000
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_VFD_1_SHIFT              22

/* FMISC :: BVNF_PDA_PWR_UP_STATUS :: VFD_2 [21:21] */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_VFD_2_MASK               0x00200000
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_VFD_2_SHIFT              21

/* FMISC :: BVNF_PDA_PWR_UP_STATUS :: VFD_3 [20:20] */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_VFD_3_MASK               0x00100000
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_VFD_3_SHIFT              20

/* FMISC :: BVNF_PDA_PWR_UP_STATUS :: reserved1 [19:19] */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_reserved1_MASK           0x00080000
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_reserved1_SHIFT          19

/* FMISC :: BVNF_PDA_PWR_UP_STATUS :: reserved2 [18:01] */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_reserved2_MASK           0x0007fffe
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_reserved2_SHIFT          1

/* FMISC :: BVNF_PDA_PWR_UP_STATUS :: RDC_LUT [00:00] */
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_RDC_LUT_MASK             0x00000001
#define BCHP_FMISC_BVNF_PDA_PWR_UP_STATUS_RDC_LUT_SHIFT            0

/***************************************************************************
 *BVNF_CLOCK_CTRL - BVN Front clock control register
 ***************************************************************************/
/* FMISC :: BVNF_CLOCK_CTRL :: reserved0 [31:02] */
#define BCHP_FMISC_BVNF_CLOCK_CTRL_reserved0_MASK                  0xfffffffc
#define BCHP_FMISC_BVNF_CLOCK_CTRL_reserved0_SHIFT                 2

/* FMISC :: BVNF_CLOCK_CTRL :: RDC_CLK_FREE_RUN_MODE [01:01] */
#define BCHP_FMISC_BVNF_CLOCK_CTRL_RDC_CLK_FREE_RUN_MODE_MASK      0x00000002
#define BCHP_FMISC_BVNF_CLOCK_CTRL_RDC_CLK_FREE_RUN_MODE_SHIFT     1
#define BCHP_FMISC_BVNF_CLOCK_CTRL_RDC_CLK_FREE_RUN_MODE_DEFAULT   0x00000000

/* FMISC :: BVNF_CLOCK_CTRL :: CLK_FREE_RUN_MODE [00:00] */
#define BCHP_FMISC_BVNF_CLOCK_CTRL_CLK_FREE_RUN_MODE_MASK          0x00000001
#define BCHP_FMISC_BVNF_CLOCK_CTRL_CLK_FREE_RUN_MODE_SHIFT         0
#define BCHP_FMISC_BVNF_CLOCK_CTRL_CLK_FREE_RUN_MODE_DEFAULT       0x00000000

/***************************************************************************
 *SCRATCH_0 - Scratch Register
 ***************************************************************************/
/* FMISC :: SCRATCH_0 :: VALUE [31:00] */
#define BCHP_FMISC_SCRATCH_0_VALUE_MASK                            0xffffffff
#define BCHP_FMISC_SCRATCH_0_VALUE_SHIFT                           0
#define BCHP_FMISC_SCRATCH_0_VALUE_DEFAULT                         0x00000000

#endif /* #ifndef BCHP_FMISC_H__ */

/* End of File */
