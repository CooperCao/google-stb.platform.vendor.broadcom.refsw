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

#ifndef BCHP_MMISC_H__
#define BCHP_MMISC_H__

/***************************************************************************
 *MMISC - BVN Middle Control Registers
 ***************************************************************************/
#define BCHP_MMISC_SW_INIT                       0x20622800 /* [RW] BVN Middle Soft Init */
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT        0x2062280c /* [RW] BVN Video Network Switch Frontend Channels Init */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT        0x20622810 /* [RW] BVN Video Network Switch Backend Channels Init */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1      0x20622814 /* [RW] BVN Video Network Switch Backend Channels Init */
#define BCHP_MMISC_SCRATCH_0                     0x20622818 /* [RW] Scratch Register */
#define BCHP_MMISC_BVNM_CLOCK_CTRL               0x20622820 /* [RW] BVN Middle clock control register */
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS           0x20622824 /* [RO] BVN Middle PDA Out Status */
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS        0x20622828 /* [RO] BVN Middle PDA Power Up Status */

/***************************************************************************
 *SW_INIT - BVN Middle Soft Init
 ***************************************************************************/
/* MMISC :: SW_INIT :: VNET_B [31:31] */
#define BCHP_MMISC_SW_INIT_VNET_B_MASK                             0x80000000
#define BCHP_MMISC_SW_INIT_VNET_B_SHIFT                            31
#define BCHP_MMISC_SW_INIT_VNET_B_DEFAULT                          0x00000000

/* MMISC :: SW_INIT :: VNET_F [30:30] */
#define BCHP_MMISC_SW_INIT_VNET_F_MASK                             0x40000000
#define BCHP_MMISC_SW_INIT_VNET_F_SHIFT                            30
#define BCHP_MMISC_SW_INIT_VNET_F_DEFAULT                          0x00000000

/* MMISC :: SW_INIT :: reserved0 [29:26] */
#define BCHP_MMISC_SW_INIT_reserved0_MASK                          0x3c000000
#define BCHP_MMISC_SW_INIT_reserved0_SHIFT                         26

/* MMISC :: SW_INIT :: DNR_1 [25:25] */
#define BCHP_MMISC_SW_INIT_DNR_1_MASK                              0x02000000
#define BCHP_MMISC_SW_INIT_DNR_1_SHIFT                             25
#define BCHP_MMISC_SW_INIT_DNR_1_DEFAULT                           0x00000000

/* MMISC :: SW_INIT :: DNR_0 [24:24] */
#define BCHP_MMISC_SW_INIT_DNR_0_MASK                              0x01000000
#define BCHP_MMISC_SW_INIT_DNR_0_SHIFT                             24
#define BCHP_MMISC_SW_INIT_DNR_0_DEFAULT                           0x00000000

/* MMISC :: SW_INIT :: reserved1 [23:17] */
#define BCHP_MMISC_SW_INIT_reserved1_MASK                          0x00fe0000
#define BCHP_MMISC_SW_INIT_reserved1_SHIFT                         17

/* MMISC :: SW_INIT :: LBOX_0 [16:16] */
#define BCHP_MMISC_SW_INIT_LBOX_0_MASK                             0x00010000
#define BCHP_MMISC_SW_INIT_LBOX_0_SHIFT                            16
#define BCHP_MMISC_SW_INIT_LBOX_0_DEFAULT                          0x00000000

/* MMISC :: SW_INIT :: reserved2 [15:09] */
#define BCHP_MMISC_SW_INIT_reserved2_MASK                          0x0000fe00
#define BCHP_MMISC_SW_INIT_reserved2_SHIFT                         9

/* MMISC :: SW_INIT :: XSRC_0 [08:08] */
#define BCHP_MMISC_SW_INIT_XSRC_0_MASK                             0x00000100
#define BCHP_MMISC_SW_INIT_XSRC_0_SHIFT                            8
#define BCHP_MMISC_SW_INIT_XSRC_0_DEFAULT                          0x00000000

/* MMISC :: SW_INIT :: reserved3 [07:02] */
#define BCHP_MMISC_SW_INIT_reserved3_MASK                          0x000000fc
#define BCHP_MMISC_SW_INIT_reserved3_SHIFT                         2

/* MMISC :: SW_INIT :: SCL_1 [01:01] */
#define BCHP_MMISC_SW_INIT_SCL_1_MASK                              0x00000002
#define BCHP_MMISC_SW_INIT_SCL_1_SHIFT                             1
#define BCHP_MMISC_SW_INIT_SCL_1_DEFAULT                           0x00000000

/* MMISC :: SW_INIT :: SCL_0 [00:00] */
#define BCHP_MMISC_SW_INIT_SCL_0_MASK                              0x00000001
#define BCHP_MMISC_SW_INIT_SCL_0_SHIFT                             0
#define BCHP_MMISC_SW_INIT_SCL_0_DEFAULT                           0x00000000

/***************************************************************************
 *VNET_F_CHANNEL_SW_INIT - BVN Video Network Switch Frontend Channels Init
 ***************************************************************************/
/* MMISC :: VNET_F_CHANNEL_SW_INIT :: reserved0 [31:20] */
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_reserved0_MASK           0xfff00000
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_reserved0_SHIFT          20

/* MMISC :: VNET_F_CHANNEL_SW_INIT :: LOOP_3 [19:19] */
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_3_MASK              0x00080000
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_3_SHIFT             19
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_3_DEFAULT           0x00000000

/* MMISC :: VNET_F_CHANNEL_SW_INIT :: LOOP_2 [18:18] */
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_2_MASK              0x00040000
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_2_SHIFT             18
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_2_DEFAULT           0x00000000

/* MMISC :: VNET_F_CHANNEL_SW_INIT :: LOOP_1 [17:17] */
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_1_MASK              0x00020000
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_1_SHIFT             17
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_1_DEFAULT           0x00000000

/* MMISC :: VNET_F_CHANNEL_SW_INIT :: LOOP_0 [16:16] */
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_0_MASK              0x00010000
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_0_SHIFT             16
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_LOOP_0_DEFAULT           0x00000000

/* MMISC :: VNET_F_CHANNEL_SW_INIT :: reserved1 [15:10] */
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_reserved1_MASK           0x0000fc00
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_reserved1_SHIFT          10

/* MMISC :: VNET_F_CHANNEL_SW_INIT :: VFD_1 [09:09] */
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_VFD_1_MASK               0x00000200
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_VFD_1_SHIFT              9
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_VFD_1_DEFAULT            0x00000000

/* MMISC :: VNET_F_CHANNEL_SW_INIT :: VFD_0 [08:08] */
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_VFD_0_MASK               0x00000100
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_VFD_0_SHIFT              8
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_VFD_0_DEFAULT            0x00000000

/* MMISC :: VNET_F_CHANNEL_SW_INIT :: reserved2 [07:02] */
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_reserved2_MASK           0x000000fc
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_reserved2_SHIFT          2

/* MMISC :: VNET_F_CHANNEL_SW_INIT :: MFD_1 [01:01] */
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_MFD_1_MASK               0x00000002
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_MFD_1_SHIFT              1
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_MFD_1_DEFAULT            0x00000000

/* MMISC :: VNET_F_CHANNEL_SW_INIT :: MFD_0 [00:00] */
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_MFD_0_MASK               0x00000001
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_MFD_0_SHIFT              0
#define BCHP_MMISC_VNET_F_CHANNEL_SW_INIT_MFD_0_DEFAULT            0x00000000

/***************************************************************************
 *VNET_B_CHANNEL_SW_INIT - BVN Video Network Switch Backend Channels Init
 ***************************************************************************/
/* MMISC :: VNET_B_CHANNEL_SW_INIT :: reserved0 [31:26] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_reserved0_MASK           0xfc000000
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_reserved0_SHIFT          26

/* MMISC :: VNET_B_CHANNEL_SW_INIT :: DNR_1 [25:25] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_DNR_1_MASK               0x02000000
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_DNR_1_SHIFT              25
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_DNR_1_DEFAULT            0x00000000

/* MMISC :: VNET_B_CHANNEL_SW_INIT :: DNR_0 [24:24] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_DNR_0_MASK               0x01000000
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_DNR_0_SHIFT              24
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_DNR_0_DEFAULT            0x00000000

/* MMISC :: VNET_B_CHANNEL_SW_INIT :: reserved1 [23:17] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_reserved1_MASK           0x00fe0000
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_reserved1_SHIFT          17

/* MMISC :: VNET_B_CHANNEL_SW_INIT :: FCH_3 [16:16] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_3_MASK               0x00010000
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_3_SHIFT              16
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_3_DEFAULT            0x00000000

/* MMISC :: VNET_B_CHANNEL_SW_INIT :: FCH_2 [15:15] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_2_MASK               0x00008000
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_2_SHIFT              15
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_2_DEFAULT            0x00000000

/* MMISC :: VNET_B_CHANNEL_SW_INIT :: FCH_1 [14:14] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_1_MASK               0x00004000
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_1_SHIFT              14
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_1_DEFAULT            0x00000000

/* MMISC :: VNET_B_CHANNEL_SW_INIT :: FCH_0 [13:13] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_0_MASK               0x00002000
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_0_SHIFT              13
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_FCH_0_DEFAULT            0x00000000

/* MMISC :: VNET_B_CHANNEL_SW_INIT :: reserved2 [12:09] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_reserved2_MASK           0x00001e00
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_reserved2_SHIFT          9

/* MMISC :: VNET_B_CHANNEL_SW_INIT :: MVP_0 [08:08] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_MVP_0_MASK               0x00000100
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_MVP_0_SHIFT              8
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_MVP_0_DEFAULT            0x00000000

/* MMISC :: VNET_B_CHANNEL_SW_INIT :: reserved3 [07:02] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_reserved3_MASK           0x000000fc
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_reserved3_SHIFT          2

/* MMISC :: VNET_B_CHANNEL_SW_INIT :: SCL_1 [01:01] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_SCL_1_MASK               0x00000002
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_SCL_1_SHIFT              1
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_SCL_1_DEFAULT            0x00000000

/* MMISC :: VNET_B_CHANNEL_SW_INIT :: SCL_0 [00:00] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_SCL_0_MASK               0x00000001
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_SCL_0_SHIFT              0
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_SCL_0_DEFAULT            0x00000000

/***************************************************************************
 *VNET_B_CHANNEL_SW_INIT_1 - BVN Video Network Switch Backend Channels Init
 ***************************************************************************/
/* MMISC :: VNET_B_CHANNEL_SW_INIT_1 :: reserved0 [31:09] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1_reserved0_MASK         0xfffffe00
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1_reserved0_SHIFT        9

/* MMISC :: VNET_B_CHANNEL_SW_INIT_1 :: XSRC_0 [08:08] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1_XSRC_0_MASK            0x00000100
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1_XSRC_0_SHIFT           8
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1_XSRC_0_DEFAULT         0x00000000

/* MMISC :: VNET_B_CHANNEL_SW_INIT_1 :: reserved1 [07:00] */
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1_reserved1_MASK         0x000000ff
#define BCHP_MMISC_VNET_B_CHANNEL_SW_INIT_1_reserved1_SHIFT        0

/***************************************************************************
 *SCRATCH_0 - Scratch Register
 ***************************************************************************/
/* MMISC :: SCRATCH_0 :: VALUE [31:00] */
#define BCHP_MMISC_SCRATCH_0_VALUE_MASK                            0xffffffff
#define BCHP_MMISC_SCRATCH_0_VALUE_SHIFT                           0
#define BCHP_MMISC_SCRATCH_0_VALUE_DEFAULT                         0x00000000

/***************************************************************************
 *BVNM_CLOCK_CTRL - BVN Middle clock control register
 ***************************************************************************/
/* MMISC :: BVNM_CLOCK_CTRL :: reserved0 [31:01] */
#define BCHP_MMISC_BVNM_CLOCK_CTRL_reserved0_MASK                  0xfffffffe
#define BCHP_MMISC_BVNM_CLOCK_CTRL_reserved0_SHIFT                 1

/* MMISC :: BVNM_CLOCK_CTRL :: CLK_FREE_RUN_MODE [00:00] */
#define BCHP_MMISC_BVNM_CLOCK_CTRL_CLK_FREE_RUN_MODE_MASK          0x00000001
#define BCHP_MMISC_BVNM_CLOCK_CTRL_CLK_FREE_RUN_MODE_SHIFT         0
#define BCHP_MMISC_BVNM_CLOCK_CTRL_CLK_FREE_RUN_MODE_DEFAULT       0x00000000

/***************************************************************************
 *BVNM_PDA_OUT_STATUS - BVN Middle PDA Out Status
 ***************************************************************************/
/* MMISC :: BVNM_PDA_OUT_STATUS :: DNR_0 [31:31] */
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS_DNR_0_MASK                  0x80000000
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS_DNR_0_SHIFT                 31

/* MMISC :: BVNM_PDA_OUT_STATUS :: DNR_1 [30:30] */
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS_DNR_1_MASK                  0x40000000
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS_DNR_1_SHIFT                 30

/* MMISC :: BVNM_PDA_OUT_STATUS :: reserved0 [29:24] */
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS_reserved0_MASK              0x3f000000
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS_reserved0_SHIFT             24

/* MMISC :: BVNM_PDA_OUT_STATUS :: SCL_0 [23:23] */
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS_SCL_0_MASK                  0x00800000
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS_SCL_0_SHIFT                 23

/* MMISC :: BVNM_PDA_OUT_STATUS :: SCL_1 [22:22] */
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS_SCL_1_MASK                  0x00400000
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS_SCL_1_SHIFT                 22

/* MMISC :: BVNM_PDA_OUT_STATUS :: reserved1 [21:00] */
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS_reserved1_MASK              0x003fffff
#define BCHP_MMISC_BVNM_PDA_OUT_STATUS_reserved1_SHIFT             0

/***************************************************************************
 *BVNM_PDA_PWR_UP_STATUS - BVN Middle PDA Power Up Status
 ***************************************************************************/
/* MMISC :: BVNM_PDA_PWR_UP_STATUS :: DNR_0 [31:31] */
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS_DNR_0_MASK               0x80000000
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS_DNR_0_SHIFT              31

/* MMISC :: BVNM_PDA_PWR_UP_STATUS :: DNR_1 [30:30] */
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS_DNR_1_MASK               0x40000000
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS_DNR_1_SHIFT              30

/* MMISC :: BVNM_PDA_PWR_UP_STATUS :: reserved0 [29:24] */
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS_reserved0_MASK           0x3f000000
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS_reserved0_SHIFT          24

/* MMISC :: BVNM_PDA_PWR_UP_STATUS :: SCL_0 [23:23] */
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS_SCL_0_MASK               0x00800000
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS_SCL_0_SHIFT              23

/* MMISC :: BVNM_PDA_PWR_UP_STATUS :: SCL_1 [22:22] */
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS_SCL_1_MASK               0x00400000
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS_SCL_1_SHIFT              22

/* MMISC :: BVNM_PDA_PWR_UP_STATUS :: reserved1 [21:00] */
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS_reserved1_MASK           0x003fffff
#define BCHP_MMISC_BVNM_PDA_PWR_UP_STATUS_reserved1_SHIFT          0

#endif /* #ifndef BCHP_MMISC_H__ */

/* End of File */
