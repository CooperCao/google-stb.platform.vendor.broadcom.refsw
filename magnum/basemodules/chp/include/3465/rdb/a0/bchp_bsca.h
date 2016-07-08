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
 * Date:           Generated on               Thu Mar 10 13:57:24 2016
 *                 Full Compile MD5 Checksum  628af85e282e26d7aa8cb3039beb3dda
 *                     (minus title and desc)
 *                 MD5 Checksum               4a24f3aa9cf80f1b639f46df03606df9
 *
 * lock_release:   n/a
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     871
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *                 Script Source              /home/pntruong/sbin/combo_header.pl
 *                 DVTSWVER                   n/a
 *
 *
********************************************************************************/

#ifndef BCHP_BSCA_H__
#define BCHP_BSCA_H__

/***************************************************************************
 *BSCA - Broadcom Serial Control Master A
 ***************************************************************************/
#define BCHP_BSCA_CHIP_ADDRESS                   0x04821800 /* [RW] BSC Chip Address And Read/Write Control */
#define BCHP_BSCA_DATA_IN0                       0x04821804 /* [RW] BSC Write Data Byte 0 / Word 0 */
#define BCHP_BSCA_DATA_IN1                       0x04821808 /* [RW] BSC Write Data Byte 1 */
#define BCHP_BSCA_DATA_IN2                       0x0482180c /* [RW] BSC Write Data Byte 2 */
#define BCHP_BSCA_DATA_IN3                       0x04821810 /* [RW] BSC Write Data Byte 3 */
#define BCHP_BSCA_DATA_IN4                       0x04821814 /* [RW] BSC Write Data Byte 4 / Word 1 */
#define BCHP_BSCA_DATA_IN5                       0x04821818 /* [RW] BSC Write Data Byte 5 */
#define BCHP_BSCA_DATA_IN6                       0x0482181c /* [RW] BSC Write Data Byte 6 */
#define BCHP_BSCA_DATA_IN7                       0x04821820 /* [RW] BSC Write Data Byte 7 */
#define BCHP_BSCA_CNT_REG                        0x04821824 /* [RW] BSC Transfer Count Register */
#define BCHP_BSCA_CTL_REG                        0x04821828 /* [RW] BSC Control Register */
#define BCHP_BSCA_IIC_ENABLE                     0x0482182c /* [RW] BSC Read/Write Enable And Interrupt */
#define BCHP_BSCA_DATA_OUT0                      0x04821830 /* [RO] BSC Read Data Byte 0 / Word 0 */
#define BCHP_BSCA_DATA_OUT1                      0x04821834 /* [RO] BSC Read Data Byte 1 */
#define BCHP_BSCA_DATA_OUT2                      0x04821838 /* [RO] BSC Read Data Byte 2 */
#define BCHP_BSCA_DATA_OUT3                      0x0482183c /* [RO] BSC Read Data Byte 3 */
#define BCHP_BSCA_DATA_OUT4                      0x04821840 /* [RO] BSC Read Data Byte 4 / Word 1 */
#define BCHP_BSCA_DATA_OUT5                      0x04821844 /* [RO] BSC Read Data Byte 5 */
#define BCHP_BSCA_DATA_OUT6                      0x04821848 /* [RO] BSC Read Data Byte 6 */
#define BCHP_BSCA_DATA_OUT7                      0x0482184c /* [RO] BSC Read Data Byte 7 */
#define BCHP_BSCA_CTLHI_REG                      0x04821850 /* [RW] BSC Control Register */
#define BCHP_BSCA_SCL_PARAM                      0x04821854 /* [RW] BSC SCL Parameter Register */

/***************************************************************************
 *CHIP_ADDRESS - BSC Chip Address And Read/Write Control
 ***************************************************************************/
/* BSCA :: CHIP_ADDRESS :: reserved0 [31:08] */
#define BCHP_BSCA_CHIP_ADDRESS_reserved0_MASK                      0xffffff00
#define BCHP_BSCA_CHIP_ADDRESS_reserved0_SHIFT                     8

/* BSCA :: CHIP_ADDRESS :: CHIP_ADDRESS [07:01] */
#define BCHP_BSCA_CHIP_ADDRESS_CHIP_ADDRESS_MASK                   0x000000fe
#define BCHP_BSCA_CHIP_ADDRESS_CHIP_ADDRESS_SHIFT                  1
#define BCHP_BSCA_CHIP_ADDRESS_CHIP_ADDRESS_DEFAULT                0x00000000

/* BSCA :: CHIP_ADDRESS :: RSVD [00:00] */
#define BCHP_BSCA_CHIP_ADDRESS_RSVD_MASK                           0x00000001
#define BCHP_BSCA_CHIP_ADDRESS_RSVD_SHIFT                          0
#define BCHP_BSCA_CHIP_ADDRESS_RSVD_DEFAULT                        0x00000000

/***************************************************************************
 *DATA_IN0 - BSC Write Data Byte 0 / Word 0
 ***************************************************************************/
/* BSCA :: DATA_IN0 :: DATA_IN3 [31:24] */
#define BCHP_BSCA_DATA_IN0_DATA_IN3_MASK                           0xff000000
#define BCHP_BSCA_DATA_IN0_DATA_IN3_SHIFT                          24
#define BCHP_BSCA_DATA_IN0_DATA_IN3_DEFAULT                        0x00000000

/* BSCA :: DATA_IN0 :: DATA_IN2 [23:16] */
#define BCHP_BSCA_DATA_IN0_DATA_IN2_MASK                           0x00ff0000
#define BCHP_BSCA_DATA_IN0_DATA_IN2_SHIFT                          16
#define BCHP_BSCA_DATA_IN0_DATA_IN2_DEFAULT                        0x00000000

/* BSCA :: DATA_IN0 :: DATA_IN1 [15:08] */
#define BCHP_BSCA_DATA_IN0_DATA_IN1_MASK                           0x0000ff00
#define BCHP_BSCA_DATA_IN0_DATA_IN1_SHIFT                          8
#define BCHP_BSCA_DATA_IN0_DATA_IN1_DEFAULT                        0x00000000

/* BSCA :: DATA_IN0 :: DATA_IN0 [07:00] */
#define BCHP_BSCA_DATA_IN0_DATA_IN0_MASK                           0x000000ff
#define BCHP_BSCA_DATA_IN0_DATA_IN0_SHIFT                          0
#define BCHP_BSCA_DATA_IN0_DATA_IN0_DEFAULT                        0x00000000

/***************************************************************************
 *DATA_IN1 - BSC Write Data Byte 1
 ***************************************************************************/
/* BSCA :: DATA_IN1 :: reserved0 [31:08] */
#define BCHP_BSCA_DATA_IN1_reserved0_MASK                          0xffffff00
#define BCHP_BSCA_DATA_IN1_reserved0_SHIFT                         8

/* BSCA :: DATA_IN1 :: DATA_IN1 [07:00] */
#define BCHP_BSCA_DATA_IN1_DATA_IN1_MASK                           0x000000ff
#define BCHP_BSCA_DATA_IN1_DATA_IN1_SHIFT                          0
#define BCHP_BSCA_DATA_IN1_DATA_IN1_DEFAULT                        0x00000000

/***************************************************************************
 *DATA_IN2 - BSC Write Data Byte 2
 ***************************************************************************/
/* BSCA :: DATA_IN2 :: reserved0 [31:08] */
#define BCHP_BSCA_DATA_IN2_reserved0_MASK                          0xffffff00
#define BCHP_BSCA_DATA_IN2_reserved0_SHIFT                         8

/* BSCA :: DATA_IN2 :: DATA_IN2 [07:00] */
#define BCHP_BSCA_DATA_IN2_DATA_IN2_MASK                           0x000000ff
#define BCHP_BSCA_DATA_IN2_DATA_IN2_SHIFT                          0
#define BCHP_BSCA_DATA_IN2_DATA_IN2_DEFAULT                        0x00000000

/***************************************************************************
 *DATA_IN3 - BSC Write Data Byte 3
 ***************************************************************************/
/* BSCA :: DATA_IN3 :: reserved0 [31:08] */
#define BCHP_BSCA_DATA_IN3_reserved0_MASK                          0xffffff00
#define BCHP_BSCA_DATA_IN3_reserved0_SHIFT                         8

/* BSCA :: DATA_IN3 :: DATA_IN3 [07:00] */
#define BCHP_BSCA_DATA_IN3_DATA_IN3_MASK                           0x000000ff
#define BCHP_BSCA_DATA_IN3_DATA_IN3_SHIFT                          0
#define BCHP_BSCA_DATA_IN3_DATA_IN3_DEFAULT                        0x00000000

/***************************************************************************
 *DATA_IN4 - BSC Write Data Byte 4 / Word 1
 ***************************************************************************/
/* BSCA :: DATA_IN4 :: DATA_IN7 [31:24] */
#define BCHP_BSCA_DATA_IN4_DATA_IN7_MASK                           0xff000000
#define BCHP_BSCA_DATA_IN4_DATA_IN7_SHIFT                          24
#define BCHP_BSCA_DATA_IN4_DATA_IN7_DEFAULT                        0x00000000

/* BSCA :: DATA_IN4 :: DATA_IN6 [23:16] */
#define BCHP_BSCA_DATA_IN4_DATA_IN6_MASK                           0x00ff0000
#define BCHP_BSCA_DATA_IN4_DATA_IN6_SHIFT                          16
#define BCHP_BSCA_DATA_IN4_DATA_IN6_DEFAULT                        0x00000000

/* BSCA :: DATA_IN4 :: DATA_IN5 [15:08] */
#define BCHP_BSCA_DATA_IN4_DATA_IN5_MASK                           0x0000ff00
#define BCHP_BSCA_DATA_IN4_DATA_IN5_SHIFT                          8
#define BCHP_BSCA_DATA_IN4_DATA_IN5_DEFAULT                        0x00000000

/* BSCA :: DATA_IN4 :: DATA_IN4 [07:00] */
#define BCHP_BSCA_DATA_IN4_DATA_IN4_MASK                           0x000000ff
#define BCHP_BSCA_DATA_IN4_DATA_IN4_SHIFT                          0
#define BCHP_BSCA_DATA_IN4_DATA_IN4_DEFAULT                        0x00000000

/***************************************************************************
 *DATA_IN5 - BSC Write Data Byte 5
 ***************************************************************************/
/* BSCA :: DATA_IN5 :: reserved0 [31:08] */
#define BCHP_BSCA_DATA_IN5_reserved0_MASK                          0xffffff00
#define BCHP_BSCA_DATA_IN5_reserved0_SHIFT                         8

/* BSCA :: DATA_IN5 :: DATA_IN5 [07:00] */
#define BCHP_BSCA_DATA_IN5_DATA_IN5_MASK                           0x000000ff
#define BCHP_BSCA_DATA_IN5_DATA_IN5_SHIFT                          0
#define BCHP_BSCA_DATA_IN5_DATA_IN5_DEFAULT                        0x00000000

/***************************************************************************
 *DATA_IN6 - BSC Write Data Byte 6
 ***************************************************************************/
/* BSCA :: DATA_IN6 :: reserved0 [31:08] */
#define BCHP_BSCA_DATA_IN6_reserved0_MASK                          0xffffff00
#define BCHP_BSCA_DATA_IN6_reserved0_SHIFT                         8

/* BSCA :: DATA_IN6 :: DATA_IN6 [07:00] */
#define BCHP_BSCA_DATA_IN6_DATA_IN6_MASK                           0x000000ff
#define BCHP_BSCA_DATA_IN6_DATA_IN6_SHIFT                          0
#define BCHP_BSCA_DATA_IN6_DATA_IN6_DEFAULT                        0x00000000

/***************************************************************************
 *DATA_IN7 - BSC Write Data Byte 7
 ***************************************************************************/
/* BSCA :: DATA_IN7 :: reserved0 [31:08] */
#define BCHP_BSCA_DATA_IN7_reserved0_MASK                          0xffffff00
#define BCHP_BSCA_DATA_IN7_reserved0_SHIFT                         8

/* BSCA :: DATA_IN7 :: DATA_IN7 [07:00] */
#define BCHP_BSCA_DATA_IN7_DATA_IN7_MASK                           0x000000ff
#define BCHP_BSCA_DATA_IN7_DATA_IN7_SHIFT                          0
#define BCHP_BSCA_DATA_IN7_DATA_IN7_DEFAULT                        0x00000000

/***************************************************************************
 *CNT_REG - BSC Transfer Count Register
 ***************************************************************************/
/* BSCA :: CNT_REG :: reserved0 [31:08] */
#define BCHP_BSCA_CNT_REG_reserved0_MASK                           0xffffff00
#define BCHP_BSCA_CNT_REG_reserved0_SHIFT                          8

/* BSCA :: CNT_REG :: CNT_REG2 [07:04] */
#define BCHP_BSCA_CNT_REG_CNT_REG2_MASK                            0x000000f0
#define BCHP_BSCA_CNT_REG_CNT_REG2_SHIFT                           4
#define BCHP_BSCA_CNT_REG_CNT_REG2_DEFAULT                         0x00000000

/* BSCA :: CNT_REG :: CNT_REG1 [03:00] */
#define BCHP_BSCA_CNT_REG_CNT_REG1_MASK                            0x0000000f
#define BCHP_BSCA_CNT_REG_CNT_REG1_SHIFT                           0
#define BCHP_BSCA_CNT_REG_CNT_REG1_DEFAULT                         0x00000000

/***************************************************************************
 *CTL_REG - BSC Control Register
 ***************************************************************************/
/* BSCA :: CTL_REG :: reserved0 [31:14] */
#define BCHP_BSCA_CTL_REG_reserved0_MASK                           0xffffc000
#define BCHP_BSCA_CTL_REG_reserved0_SHIFT                          14

/* BSCA :: CTL_REG :: INT_EN2 [13:13] */
#define BCHP_BSCA_CTL_REG_INT_EN2_MASK                             0x00002000
#define BCHP_BSCA_CTL_REG_INT_EN2_SHIFT                            13
#define BCHP_BSCA_CTL_REG_INT_EN2_DEFAULT                          0x00000000

/* BSCA :: CTL_REG :: INT_EN1 [12:12] */
#define BCHP_BSCA_CTL_REG_INT_EN1_MASK                             0x00001000
#define BCHP_BSCA_CTL_REG_INT_EN1_SHIFT                            12
#define BCHP_BSCA_CTL_REG_INT_EN1_DEFAULT                          0x00000000

/* BSCA :: CTL_REG :: reserved_for_eco1 [11:11] */
#define BCHP_BSCA_CTL_REG_reserved_for_eco1_MASK                   0x00000800
#define BCHP_BSCA_CTL_REG_reserved_for_eco1_SHIFT                  11
#define BCHP_BSCA_CTL_REG_reserved_for_eco1_DEFAULT                0x00000000

/* BSCA :: CTL_REG :: SDA_DELAY_SEL [10:08] */
#define BCHP_BSCA_CTL_REG_SDA_DELAY_SEL_MASK                       0x00000700
#define BCHP_BSCA_CTL_REG_SDA_DELAY_SEL_SHIFT                      8
#define BCHP_BSCA_CTL_REG_SDA_DELAY_SEL_DEFAULT                    0x00000000

/* BSCA :: CTL_REG :: DIV_CLK [07:07] */
#define BCHP_BSCA_CTL_REG_DIV_CLK_MASK                             0x00000080
#define BCHP_BSCA_CTL_REG_DIV_CLK_SHIFT                            7
#define BCHP_BSCA_CTL_REG_DIV_CLK_DEFAULT                          0x00000000

/* BSCA :: CTL_REG :: INT_EN [06:06] */
#define BCHP_BSCA_CTL_REG_INT_EN_MASK                              0x00000040
#define BCHP_BSCA_CTL_REG_INT_EN_SHIFT                             6
#define BCHP_BSCA_CTL_REG_INT_EN_DEFAULT                           0x00000000

/* BSCA :: CTL_REG :: SCL_SEL [05:04] */
#define BCHP_BSCA_CTL_REG_SCL_SEL_MASK                             0x00000030
#define BCHP_BSCA_CTL_REG_SCL_SEL_SHIFT                            4
#define BCHP_BSCA_CTL_REG_SCL_SEL_DEFAULT                          0x00000000

/* BSCA :: CTL_REG :: DELAY_DIS [03:03] */
#define BCHP_BSCA_CTL_REG_DELAY_DIS_MASK                           0x00000008
#define BCHP_BSCA_CTL_REG_DELAY_DIS_SHIFT                          3
#define BCHP_BSCA_CTL_REG_DELAY_DIS_DEFAULT                        0x00000000

/* BSCA :: CTL_REG :: DEGLITCH_DIS [02:02] */
#define BCHP_BSCA_CTL_REG_DEGLITCH_DIS_MASK                        0x00000004
#define BCHP_BSCA_CTL_REG_DEGLITCH_DIS_SHIFT                       2
#define BCHP_BSCA_CTL_REG_DEGLITCH_DIS_DEFAULT                     0x00000000

/* BSCA :: CTL_REG :: DTF [01:00] */
#define BCHP_BSCA_CTL_REG_DTF_MASK                                 0x00000003
#define BCHP_BSCA_CTL_REG_DTF_SHIFT                                0
#define BCHP_BSCA_CTL_REG_DTF_DEFAULT                              0x00000000

/***************************************************************************
 *IIC_ENABLE - BSC Read/Write Enable And Interrupt
 ***************************************************************************/
/* BSCA :: IIC_ENABLE :: BUS_BUSY [31:31] */
#define BCHP_BSCA_IIC_ENABLE_BUS_BUSY_MASK                         0x80000000
#define BCHP_BSCA_IIC_ENABLE_BUS_BUSY_SHIFT                        31
#define BCHP_BSCA_IIC_ENABLE_BUS_BUSY_DEFAULT                      0x00000000

/* BSCA :: IIC_ENABLE :: STATE [30:28] */
#define BCHP_BSCA_IIC_ENABLE_STATE_MASK                            0x70000000
#define BCHP_BSCA_IIC_ENABLE_STATE_SHIFT                           28
#define BCHP_BSCA_IIC_ENABLE_STATE_DEFAULT                         0x00000000

/* BSCA :: IIC_ENABLE :: CNT_SEL [27:24] */
#define BCHP_BSCA_IIC_ENABLE_CNT_SEL_MASK                          0x0f000000
#define BCHP_BSCA_IIC_ENABLE_CNT_SEL_SHIFT                         24
#define BCHP_BSCA_IIC_ENABLE_CNT_SEL_DEFAULT                       0x00000000

/* BSCA :: IIC_ENABLE :: REG_CNT [23:20] */
#define BCHP_BSCA_IIC_ENABLE_REG_CNT_MASK                          0x00f00000
#define BCHP_BSCA_IIC_ENABLE_REG_CNT_SHIFT                         20
#define BCHP_BSCA_IIC_ENABLE_REG_CNT_DEFAULT                       0x00000000

/* BSCA :: IIC_ENABLE :: BIT_CNT [19:16] */
#define BCHP_BSCA_IIC_ENABLE_BIT_CNT_MASK                          0x000f0000
#define BCHP_BSCA_IIC_ENABLE_BIT_CNT_SHIFT                         16
#define BCHP_BSCA_IIC_ENABLE_BIT_CNT_DEFAULT                       0x00000000

/* BSCA :: IIC_ENABLE :: INTRP2_STICKY [15:15] */
#define BCHP_BSCA_IIC_ENABLE_INTRP2_STICKY_MASK                    0x00008000
#define BCHP_BSCA_IIC_ENABLE_INTRP2_STICKY_SHIFT                   15
#define BCHP_BSCA_IIC_ENABLE_INTRP2_STICKY_DEFAULT                 0x00000000

/* BSCA :: IIC_ENABLE :: INTRP1_STICKY [14:14] */
#define BCHP_BSCA_IIC_ENABLE_INTRP1_STICKY_MASK                    0x00004000
#define BCHP_BSCA_IIC_ENABLE_INTRP1_STICKY_SHIFT                   14
#define BCHP_BSCA_IIC_ENABLE_INTRP1_STICKY_DEFAULT                 0x00000000

/* BSCA :: IIC_ENABLE :: INTRP2 [13:13] */
#define BCHP_BSCA_IIC_ENABLE_INTRP2_MASK                           0x00002000
#define BCHP_BSCA_IIC_ENABLE_INTRP2_SHIFT                          13
#define BCHP_BSCA_IIC_ENABLE_INTRP2_DEFAULT                        0x00000000

/* BSCA :: IIC_ENABLE :: INTRP1 [12:12] */
#define BCHP_BSCA_IIC_ENABLE_INTRP1_MASK                           0x00001000
#define BCHP_BSCA_IIC_ENABLE_INTRP1_SHIFT                          12
#define BCHP_BSCA_IIC_ENABLE_INTRP1_DEFAULT                        0x00000000

/* BSCA :: IIC_ENABLE :: RSVD2 [11:10] */
#define BCHP_BSCA_IIC_ENABLE_RSVD2_MASK                            0x00000c00
#define BCHP_BSCA_IIC_ENABLE_RSVD2_SHIFT                           10
#define BCHP_BSCA_IIC_ENABLE_RSVD2_DEFAULT                         0x00000000

/* BSCA :: IIC_ENABLE :: NO_AUTO2 [09:09] */
#define BCHP_BSCA_IIC_ENABLE_NO_AUTO2_MASK                         0x00000200
#define BCHP_BSCA_IIC_ENABLE_NO_AUTO2_SHIFT                        9
#define BCHP_BSCA_IIC_ENABLE_NO_AUTO2_DEFAULT                      0x00000000

/* BSCA :: IIC_ENABLE :: NO_AUTO1 [08:08] */
#define BCHP_BSCA_IIC_ENABLE_NO_AUTO1_MASK                         0x00000100
#define BCHP_BSCA_IIC_ENABLE_NO_AUTO1_SHIFT                        8
#define BCHP_BSCA_IIC_ENABLE_NO_AUTO1_DEFAULT                      0x00000000

/* BSCA :: IIC_ENABLE :: RSVD1 [07:07] */
#define BCHP_BSCA_IIC_ENABLE_RSVD1_MASK                            0x00000080
#define BCHP_BSCA_IIC_ENABLE_RSVD1_SHIFT                           7
#define BCHP_BSCA_IIC_ENABLE_RSVD1_DEFAULT                         0x00000000

/* BSCA :: IIC_ENABLE :: RESTART [06:06] */
#define BCHP_BSCA_IIC_ENABLE_RESTART_MASK                          0x00000040
#define BCHP_BSCA_IIC_ENABLE_RESTART_SHIFT                         6
#define BCHP_BSCA_IIC_ENABLE_RESTART_DEFAULT                       0x00000000

/* BSCA :: IIC_ENABLE :: NO_START [05:05] */
#define BCHP_BSCA_IIC_ENABLE_NO_START_MASK                         0x00000020
#define BCHP_BSCA_IIC_ENABLE_NO_START_SHIFT                        5
#define BCHP_BSCA_IIC_ENABLE_NO_START_DEFAULT                      0x00000000

/* BSCA :: IIC_ENABLE :: NO_STOP [04:04] */
#define BCHP_BSCA_IIC_ENABLE_NO_STOP_MASK                          0x00000010
#define BCHP_BSCA_IIC_ENABLE_NO_STOP_SHIFT                         4
#define BCHP_BSCA_IIC_ENABLE_NO_STOP_DEFAULT                       0x00000000

/* BSCA :: IIC_ENABLE :: RSVD0 [03:03] */
#define BCHP_BSCA_IIC_ENABLE_RSVD0_MASK                            0x00000008
#define BCHP_BSCA_IIC_ENABLE_RSVD0_SHIFT                           3
#define BCHP_BSCA_IIC_ENABLE_RSVD0_DEFAULT                         0x00000000

/* BSCA :: IIC_ENABLE :: NO_ACK [02:02] */
#define BCHP_BSCA_IIC_ENABLE_NO_ACK_MASK                           0x00000004
#define BCHP_BSCA_IIC_ENABLE_NO_ACK_SHIFT                          2
#define BCHP_BSCA_IIC_ENABLE_NO_ACK_DEFAULT                        0x00000000

/* BSCA :: IIC_ENABLE :: INTRP [01:01] */
#define BCHP_BSCA_IIC_ENABLE_INTRP_MASK                            0x00000002
#define BCHP_BSCA_IIC_ENABLE_INTRP_SHIFT                           1
#define BCHP_BSCA_IIC_ENABLE_INTRP_DEFAULT                         0x00000000

/* BSCA :: IIC_ENABLE :: ENABLE [00:00] */
#define BCHP_BSCA_IIC_ENABLE_ENABLE_MASK                           0x00000001
#define BCHP_BSCA_IIC_ENABLE_ENABLE_SHIFT                          0
#define BCHP_BSCA_IIC_ENABLE_ENABLE_DEFAULT                        0x00000000

/***************************************************************************
 *DATA_OUT0 - BSC Read Data Byte 0 / Word 0
 ***************************************************************************/
/* BSCA :: DATA_OUT0 :: DATA_OUT3 [31:24] */
#define BCHP_BSCA_DATA_OUT0_DATA_OUT3_MASK                         0xff000000
#define BCHP_BSCA_DATA_OUT0_DATA_OUT3_SHIFT                        24
#define BCHP_BSCA_DATA_OUT0_DATA_OUT3_DEFAULT                      0x00000000

/* BSCA :: DATA_OUT0 :: DATA_OUT2 [23:16] */
#define BCHP_BSCA_DATA_OUT0_DATA_OUT2_MASK                         0x00ff0000
#define BCHP_BSCA_DATA_OUT0_DATA_OUT2_SHIFT                        16
#define BCHP_BSCA_DATA_OUT0_DATA_OUT2_DEFAULT                      0x00000000

/* BSCA :: DATA_OUT0 :: DATA_OUT1 [15:08] */
#define BCHP_BSCA_DATA_OUT0_DATA_OUT1_MASK                         0x0000ff00
#define BCHP_BSCA_DATA_OUT0_DATA_OUT1_SHIFT                        8
#define BCHP_BSCA_DATA_OUT0_DATA_OUT1_DEFAULT                      0x00000000

/* BSCA :: DATA_OUT0 :: DATA_OUT0 [07:00] */
#define BCHP_BSCA_DATA_OUT0_DATA_OUT0_MASK                         0x000000ff
#define BCHP_BSCA_DATA_OUT0_DATA_OUT0_SHIFT                        0
#define BCHP_BSCA_DATA_OUT0_DATA_OUT0_DEFAULT                      0x00000000

/***************************************************************************
 *DATA_OUT1 - BSC Read Data Byte 1
 ***************************************************************************/
/* BSCA :: DATA_OUT1 :: reserved0 [31:08] */
#define BCHP_BSCA_DATA_OUT1_reserved0_MASK                         0xffffff00
#define BCHP_BSCA_DATA_OUT1_reserved0_SHIFT                        8

/* BSCA :: DATA_OUT1 :: DATA_OUT1 [07:00] */
#define BCHP_BSCA_DATA_OUT1_DATA_OUT1_MASK                         0x000000ff
#define BCHP_BSCA_DATA_OUT1_DATA_OUT1_SHIFT                        0
#define BCHP_BSCA_DATA_OUT1_DATA_OUT1_DEFAULT                      0x00000000

/***************************************************************************
 *DATA_OUT2 - BSC Read Data Byte 2
 ***************************************************************************/
/* BSCA :: DATA_OUT2 :: reserved0 [31:08] */
#define BCHP_BSCA_DATA_OUT2_reserved0_MASK                         0xffffff00
#define BCHP_BSCA_DATA_OUT2_reserved0_SHIFT                        8

/* BSCA :: DATA_OUT2 :: DATA_OUT2 [07:00] */
#define BCHP_BSCA_DATA_OUT2_DATA_OUT2_MASK                         0x000000ff
#define BCHP_BSCA_DATA_OUT2_DATA_OUT2_SHIFT                        0
#define BCHP_BSCA_DATA_OUT2_DATA_OUT2_DEFAULT                      0x00000000

/***************************************************************************
 *DATA_OUT3 - BSC Read Data Byte 3
 ***************************************************************************/
/* BSCA :: DATA_OUT3 :: reserved0 [31:08] */
#define BCHP_BSCA_DATA_OUT3_reserved0_MASK                         0xffffff00
#define BCHP_BSCA_DATA_OUT3_reserved0_SHIFT                        8

/* BSCA :: DATA_OUT3 :: DATA_OUT3 [07:00] */
#define BCHP_BSCA_DATA_OUT3_DATA_OUT3_MASK                         0x000000ff
#define BCHP_BSCA_DATA_OUT3_DATA_OUT3_SHIFT                        0
#define BCHP_BSCA_DATA_OUT3_DATA_OUT3_DEFAULT                      0x00000000

/***************************************************************************
 *DATA_OUT4 - BSC Read Data Byte 4 / Word 1
 ***************************************************************************/
/* BSCA :: DATA_OUT4 :: DATA_OUT7 [31:24] */
#define BCHP_BSCA_DATA_OUT4_DATA_OUT7_MASK                         0xff000000
#define BCHP_BSCA_DATA_OUT4_DATA_OUT7_SHIFT                        24
#define BCHP_BSCA_DATA_OUT4_DATA_OUT7_DEFAULT                      0x00000000

/* BSCA :: DATA_OUT4 :: DATA_OUT6 [23:16] */
#define BCHP_BSCA_DATA_OUT4_DATA_OUT6_MASK                         0x00ff0000
#define BCHP_BSCA_DATA_OUT4_DATA_OUT6_SHIFT                        16
#define BCHP_BSCA_DATA_OUT4_DATA_OUT6_DEFAULT                      0x00000000

/* BSCA :: DATA_OUT4 :: DATA_OUT5 [15:08] */
#define BCHP_BSCA_DATA_OUT4_DATA_OUT5_MASK                         0x0000ff00
#define BCHP_BSCA_DATA_OUT4_DATA_OUT5_SHIFT                        8
#define BCHP_BSCA_DATA_OUT4_DATA_OUT5_DEFAULT                      0x00000000

/* BSCA :: DATA_OUT4 :: DATA_OUT4 [07:00] */
#define BCHP_BSCA_DATA_OUT4_DATA_OUT4_MASK                         0x000000ff
#define BCHP_BSCA_DATA_OUT4_DATA_OUT4_SHIFT                        0
#define BCHP_BSCA_DATA_OUT4_DATA_OUT4_DEFAULT                      0x00000000

/***************************************************************************
 *DATA_OUT5 - BSC Read Data Byte 5
 ***************************************************************************/
/* BSCA :: DATA_OUT5 :: reserved0 [31:08] */
#define BCHP_BSCA_DATA_OUT5_reserved0_MASK                         0xffffff00
#define BCHP_BSCA_DATA_OUT5_reserved0_SHIFT                        8

/* BSCA :: DATA_OUT5 :: DATA_OUT5 [07:00] */
#define BCHP_BSCA_DATA_OUT5_DATA_OUT5_MASK                         0x000000ff
#define BCHP_BSCA_DATA_OUT5_DATA_OUT5_SHIFT                        0
#define BCHP_BSCA_DATA_OUT5_DATA_OUT5_DEFAULT                      0x00000000

/***************************************************************************
 *DATA_OUT6 - BSC Read Data Byte 6
 ***************************************************************************/
/* BSCA :: DATA_OUT6 :: reserved0 [31:08] */
#define BCHP_BSCA_DATA_OUT6_reserved0_MASK                         0xffffff00
#define BCHP_BSCA_DATA_OUT6_reserved0_SHIFT                        8

/* BSCA :: DATA_OUT6 :: DATA_OUT6 [07:00] */
#define BCHP_BSCA_DATA_OUT6_DATA_OUT6_MASK                         0x000000ff
#define BCHP_BSCA_DATA_OUT6_DATA_OUT6_SHIFT                        0
#define BCHP_BSCA_DATA_OUT6_DATA_OUT6_DEFAULT                      0x00000000

/***************************************************************************
 *DATA_OUT7 - BSC Read Data Byte 7
 ***************************************************************************/
/* BSCA :: DATA_OUT7 :: reserved0 [31:08] */
#define BCHP_BSCA_DATA_OUT7_reserved0_MASK                         0xffffff00
#define BCHP_BSCA_DATA_OUT7_reserved0_SHIFT                        8

/* BSCA :: DATA_OUT7 :: DATA_OUT7 [07:00] */
#define BCHP_BSCA_DATA_OUT7_DATA_OUT7_MASK                         0x000000ff
#define BCHP_BSCA_DATA_OUT7_DATA_OUT7_SHIFT                        0
#define BCHP_BSCA_DATA_OUT7_DATA_OUT7_DEFAULT                      0x00000000

/***************************************************************************
 *CTLHI_REG - BSC Control Register
 ***************************************************************************/
/* BSCA :: CTLHI_REG :: reserved0 [31:08] */
#define BCHP_BSCA_CTLHI_REG_reserved0_MASK                         0xffffff00
#define BCHP_BSCA_CTLHI_REG_reserved0_SHIFT                        8

/* BSCA :: CTLHI_REG :: reserved_for_eco1 [07:05] */
#define BCHP_BSCA_CTLHI_REG_reserved_for_eco1_MASK                 0x000000e0
#define BCHP_BSCA_CTLHI_REG_reserved_for_eco1_SHIFT                5
#define BCHP_BSCA_CTLHI_REG_reserved_for_eco1_DEFAULT              0x00000000

/* BSCA :: CTLHI_REG :: DISABLE_IDLE_CLK_SYNC [04:04] */
#define BCHP_BSCA_CTLHI_REG_DISABLE_IDLE_CLK_SYNC_MASK             0x00000010
#define BCHP_BSCA_CTLHI_REG_DISABLE_IDLE_CLK_SYNC_SHIFT            4
#define BCHP_BSCA_CTLHI_REG_DISABLE_IDLE_CLK_SYNC_DEFAULT          0x00000000

/* BSCA :: CTLHI_REG :: DISABLE_MM [03:03] */
#define BCHP_BSCA_CTLHI_REG_DISABLE_MM_MASK                        0x00000008
#define BCHP_BSCA_CTLHI_REG_DISABLE_MM_SHIFT                       3
#define BCHP_BSCA_CTLHI_REG_DISABLE_MM_DEFAULT                     0x00000000

/* BSCA :: CTLHI_REG :: ENABLE_DATA_PACKING [02:02] */
#define BCHP_BSCA_CTLHI_REG_ENABLE_DATA_PACKING_MASK               0x00000004
#define BCHP_BSCA_CTLHI_REG_ENABLE_DATA_PACKING_SHIFT              2
#define BCHP_BSCA_CTLHI_REG_ENABLE_DATA_PACKING_DEFAULT            0x00000000

/* BSCA :: CTLHI_REG :: IGNORE_ACK [01:01] */
#define BCHP_BSCA_CTLHI_REG_IGNORE_ACK_MASK                        0x00000002
#define BCHP_BSCA_CTLHI_REG_IGNORE_ACK_SHIFT                       1
#define BCHP_BSCA_CTLHI_REG_IGNORE_ACK_DEFAULT                     0x00000000

/* BSCA :: CTLHI_REG :: WAIT_DIS [00:00] */
#define BCHP_BSCA_CTLHI_REG_WAIT_DIS_MASK                          0x00000001
#define BCHP_BSCA_CTLHI_REG_WAIT_DIS_SHIFT                         0
#define BCHP_BSCA_CTLHI_REG_WAIT_DIS_DEFAULT                       0x00000000

/***************************************************************************
 *SCL_PARAM - BSC SCL Parameter Register
 ***************************************************************************/
/* BSCA :: SCL_PARAM :: reserved_for_eco0 [31:00] */
#define BCHP_BSCA_SCL_PARAM_reserved_for_eco0_MASK                 0xffffffff
#define BCHP_BSCA_SCL_PARAM_reserved_for_eco0_SHIFT                0
#define BCHP_BSCA_SCL_PARAM_reserved_for_eco0_DEFAULT              0x00000000

#endif /* #ifndef BCHP_BSCA_H__ */

/* End of File */
