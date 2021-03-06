/***************************************************************************
 *     Copyright (c) 1999-2008, Broadcom Corporation
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
 * Date:           Generated on         Mon Mar 10 16:44:35 2008
 *                 MD5 Checksum         cf66689754e95804a8fa320d4ee9eb18
 *
 * Compiled with:  RDB Utility          combo_header.pl
 *                 RDB Parser           3.0
 *                 unknown              unknown
 *                 Perl Interpreter     5.008008
 *                 Operating System     linux
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BCHP_XPT_RMX0_H__
#define BCHP_XPT_RMX0_H__

/***************************************************************************
 *XPT_RMX0 - XPT RMX0 Control Registers
 ***************************************************************************/
#define BCHP_XPT_RMX0_CTRL                       0x00226020 /* RMX Control Register */
#define BCHP_XPT_RMX0_PCR_CTRL                   0x00226024 /* RMX PCR Control Register */
#define BCHP_XPT_RMX0_FORMAT                     0x00226028 /* RMX Format Register */
#define BCHP_XPT_RMX0_PCR_OFFSET                 0x0022602c /* RMX PCR Constant Offset Register */
#define BCHP_XPT_RMX0_PSUB1_CTRL                 0x00226030 /* RMX Packet Substitution Control Register */
#define BCHP_XPT_RMX0_PSUB1_PID                  0x00226034 /* RMX Packet Substitution PID Register */
#define BCHP_XPT_RMX0_PSUB2_CTRL                 0x00226038 /* RMX Packet Substitution Control Register */
#define BCHP_XPT_RMX0_PSUB2_PID                  0x0022603c /* RMX Packet Substitution PID Register */
#define BCHP_XPT_RMX0_PKT_GEN1_CTRL              0x00226044 /* RMX Packet Generation 1 Control Register */
#define BCHP_XPT_RMX0_PKT_GEN2_CTRL              0x00226048 /* RMX Packet Generation 2 Control Register */
#define BCHP_XPT_RMX0_PKT_DLY_CNT                0x0022604c /* RMX Packet Delay Control Register */
#define BCHP_XPT_RMX0_RMX_PACING                 0x00226050 /* RMX Pacing Register */
#define BCHP_XPT_RMX0_RMX_TS_ERR_BOUND           0x00226054 /* Data Transport Remux Timestamp Error Bound Register */
#define BCHP_XPT_RMX0_RMX_DEBUG                  0x00226058 /* RMX DEBUG Register */
#define BCHP_XPT_RMX0_RMX_STATUS                 0x0022605c /* RMX Status Register */

/***************************************************************************
 *CTRL - RMX Control Register
 ***************************************************************************/
/* XPT_RMX0 :: CTRL :: reserved0 [31:16] */
#define BCHP_XPT_RMX0_CTRL_reserved0_MASK                          0xffff0000
#define BCHP_XPT_RMX0_CTRL_reserved0_SHIFT                         16

/* XPT_RMX0 :: CTRL :: RMX_PAUSE_EN [15:15] */
#define BCHP_XPT_RMX0_CTRL_RMX_PAUSE_EN_MASK                       0x00008000
#define BCHP_XPT_RMX0_CTRL_RMX_PAUSE_EN_SHIFT                      15

/* XPT_RMX0 :: CTRL :: RMX_WATERMARK_CLEAR [14:14] */
#define BCHP_XPT_RMX0_CTRL_RMX_WATERMARK_CLEAR_MASK                0x00004000
#define BCHP_XPT_RMX0_CTRL_RMX_WATERMARK_CLEAR_SHIFT               14

/* XPT_RMX0 :: CTRL :: RMX_INTERNAL_PES_STRIP [13:13] */
#define BCHP_XPT_RMX0_CTRL_RMX_INTERNAL_PES_STRIP_MASK             0x00002000
#define BCHP_XPT_RMX0_CTRL_RMX_INTERNAL_PES_STRIP_SHIFT            13

/* XPT_RMX0 :: CTRL :: RMX_ALL_PASS_EN [12:12] */
#define BCHP_XPT_RMX0_CTRL_RMX_ALL_PASS_EN_MASK                    0x00001000
#define BCHP_XPT_RMX0_CTRL_RMX_ALL_PASS_EN_SHIFT                   12

/* XPT_RMX0 :: CTRL :: RMX_PID_MAP_EN [11:11] */
#define BCHP_XPT_RMX0_CTRL_RMX_PID_MAP_EN_MASK                     0x00000800
#define BCHP_XPT_RMX0_CTRL_RMX_PID_MAP_EN_SHIFT                    11

/* XPT_RMX0 :: CTRL :: reserved1 [10:10] */
#define BCHP_XPT_RMX0_CTRL_reserved1_MASK                          0x00000400
#define BCHP_XPT_RMX0_CTRL_reserved1_SHIFT                         10

/* XPT_RMX0 :: CTRL :: RMX_NULL_PKT_DIS [09:09] */
#define BCHP_XPT_RMX0_CTRL_RMX_NULL_PKT_DIS_MASK                   0x00000200
#define BCHP_XPT_RMX0_CTRL_RMX_NULL_PKT_DIS_SHIFT                  9

/* XPT_RMX0 :: CTRL :: RMX_PCR_CORRECT_DIS [08:08] */
#define BCHP_XPT_RMX0_CTRL_RMX_PCR_CORRECT_DIS_MASK                0x00000100
#define BCHP_XPT_RMX0_CTRL_RMX_PCR_CORRECT_DIS_SHIFT               8

/* XPT_RMX0 :: CTRL :: RMX_PSUB2_EN [07:07] */
#define BCHP_XPT_RMX0_CTRL_RMX_PSUB2_EN_MASK                       0x00000080
#define BCHP_XPT_RMX0_CTRL_RMX_PSUB2_EN_SHIFT                      7

/* XPT_RMX0 :: CTRL :: RMX_PSUB1_EN [06:06] */
#define BCHP_XPT_RMX0_CTRL_RMX_PSUB1_EN_MASK                       0x00000040
#define BCHP_XPT_RMX0_CTRL_RMX_PSUB1_EN_SHIFT                      6

/* XPT_RMX0 :: CTRL :: RMX_BYPASS [05:05] */
#define BCHP_XPT_RMX0_CTRL_RMX_BYPASS_MASK                         0x00000020
#define BCHP_XPT_RMX0_CTRL_RMX_BYPASS_SHIFT                        5

/* XPT_RMX0 :: CTRL :: RMX_ENABLE [04:04] */
#define BCHP_XPT_RMX0_CTRL_RMX_ENABLE_MASK                         0x00000010
#define BCHP_XPT_RMX0_CTRL_RMX_ENABLE_SHIFT                        4

/* XPT_RMX0 :: CTRL :: RMX_CLK_SEL [03:00] */
#define BCHP_XPT_RMX0_CTRL_RMX_CLK_SEL_MASK                        0x0000000f
#define BCHP_XPT_RMX0_CTRL_RMX_CLK_SEL_SHIFT                       0

/***************************************************************************
 *PCR_CTRL - RMX PCR Control Register
 ***************************************************************************/
/* XPT_RMX0 :: PCR_CTRL :: READLOAD_RMXB_TS_COUNT [31:31] */
#define BCHP_XPT_RMX0_PCR_CTRL_READLOAD_RMXB_TS_COUNT_MASK         0x80000000
#define BCHP_XPT_RMX0_PCR_CTRL_READLOAD_RMXB_TS_COUNT_SHIFT        31

/* XPT_RMX0 :: PCR_CTRL :: RMXB_TIMEBASE_SEL [30:28] */
#define BCHP_XPT_RMX0_PCR_CTRL_RMXB_TIMEBASE_SEL_MASK              0x70000000
#define BCHP_XPT_RMX0_PCR_CTRL_RMXB_TIMEBASE_SEL_SHIFT             28

/* XPT_RMX0 :: PCR_CTRL :: reserved0 [27:21] */
#define BCHP_XPT_RMX0_PCR_CTRL_reserved0_MASK                      0x0fe00000
#define BCHP_XPT_RMX0_PCR_CTRL_reserved0_SHIFT                     21

/* XPT_RMX0 :: PCR_CTRL :: RMXB_BAND_NUM [20:16] */
#define BCHP_XPT_RMX0_PCR_CTRL_RMXB_BAND_NUM_MASK                  0x001f0000
#define BCHP_XPT_RMX0_PCR_CTRL_RMXB_BAND_NUM_SHIFT                 16

/* XPT_RMX0 :: PCR_CTRL :: READLOAD_RMXA_TS_COUNT [15:15] */
#define BCHP_XPT_RMX0_PCR_CTRL_READLOAD_RMXA_TS_COUNT_MASK         0x00008000
#define BCHP_XPT_RMX0_PCR_CTRL_READLOAD_RMXA_TS_COUNT_SHIFT        15

/* XPT_RMX0 :: PCR_CTRL :: RMXA_TIMEBASE_SEL [14:12] */
#define BCHP_XPT_RMX0_PCR_CTRL_RMXA_TIMEBASE_SEL_MASK              0x00007000
#define BCHP_XPT_RMX0_PCR_CTRL_RMXA_TIMEBASE_SEL_SHIFT             12

/* XPT_RMX0 :: PCR_CTRL :: reserved1 [11:05] */
#define BCHP_XPT_RMX0_PCR_CTRL_reserved1_MASK                      0x00000fe0
#define BCHP_XPT_RMX0_PCR_CTRL_reserved1_SHIFT                     5

/* XPT_RMX0 :: PCR_CTRL :: RMXA_BAND_NUM [04:00] */
#define BCHP_XPT_RMX0_PCR_CTRL_RMXA_BAND_NUM_MASK                  0x0000001f
#define BCHP_XPT_RMX0_PCR_CTRL_RMXA_BAND_NUM_SHIFT                 0

/***************************************************************************
 *FORMAT - RMX Format Register
 ***************************************************************************/
/* XPT_RMX0 :: FORMAT :: RMX_PKT_LENGTH [31:24] */
#define BCHP_XPT_RMX0_FORMAT_RMX_PKT_LENGTH_MASK                   0xff000000
#define BCHP_XPT_RMX0_FORMAT_RMX_PKT_LENGTH_SHIFT                  24

/* XPT_RMX0 :: FORMAT :: reserved0 [23:23] */
#define BCHP_XPT_RMX0_FORMAT_reserved0_MASK                        0x00800000
#define BCHP_XPT_RMX0_FORMAT_reserved0_SHIFT                       23

/* XPT_RMX0 :: FORMAT :: RMXP_INVERT_CLK [22:22] */
#define BCHP_XPT_RMX0_FORMAT_RMXP_INVERT_CLK_MASK                  0x00400000
#define BCHP_XPT_RMX0_FORMAT_RMXP_INVERT_CLK_SHIFT                 22

/* XPT_RMX0 :: FORMAT :: RMXP_INVERT_SYNC [21:21] */
#define BCHP_XPT_RMX0_FORMAT_RMXP_INVERT_SYNC_MASK                 0x00200000
#define BCHP_XPT_RMX0_FORMAT_RMXP_INVERT_SYNC_SHIFT                21

/* XPT_RMX0 :: FORMAT :: RMXP_ENABLE [20:20] */
#define BCHP_XPT_RMX0_FORMAT_RMXP_ENABLE_MASK                      0x00100000
#define BCHP_XPT_RMX0_FORMAT_RMXP_ENABLE_SHIFT                     20

/* XPT_RMX0 :: FORMAT :: reserved1 [19:18] */
#define BCHP_XPT_RMX0_FORMAT_reserved1_MASK                        0x000c0000
#define BCHP_XPT_RMX0_FORMAT_reserved1_SHIFT                       18

/* XPT_RMX0 :: FORMAT :: RMX_B8ZS_EN_ [17:17] */
#define BCHP_XPT_RMX0_FORMAT_RMX_B8ZS_EN__MASK                     0x00020000
#define BCHP_XPT_RMX0_FORMAT_RMX_B8ZS_EN__SHIFT                    17

/* XPT_RMX0 :: FORMAT :: RMX_PKT_MODE [16:16] */
#define BCHP_XPT_RMX0_FORMAT_RMX_PKT_MODE_MASK                     0x00010000
#define BCHP_XPT_RMX0_FORMAT_RMX_PKT_MODE_SHIFT                    16

/* XPT_RMX0 :: FORMAT :: reserved2 [15:07] */
#define BCHP_XPT_RMX0_FORMAT_reserved2_MASK                        0x0000ff80
#define BCHP_XPT_RMX0_FORMAT_reserved2_SHIFT                       7

/* XPT_RMX0 :: FORMAT :: RMX_INVERT_SYNC [06:06] */
#define BCHP_XPT_RMX0_FORMAT_RMX_INVERT_SYNC_MASK                  0x00000040
#define BCHP_XPT_RMX0_FORMAT_RMX_INVERT_SYNC_SHIFT                 6

/* XPT_RMX0 :: FORMAT :: RMX_INVERT_CLK [05:05] */
#define BCHP_XPT_RMX0_FORMAT_RMX_INVERT_CLK_MASK                   0x00000020
#define BCHP_XPT_RMX0_FORMAT_RMX_INVERT_CLK_SHIFT                  5

/* XPT_RMX0 :: FORMAT :: RMX_BYTE_SYNC [04:04] */
#define BCHP_XPT_RMX0_FORMAT_RMX_BYTE_SYNC_MASK                    0x00000010
#define BCHP_XPT_RMX0_FORMAT_RMX_BYTE_SYNC_SHIFT                   4

/* XPT_RMX0 :: FORMAT :: reserved3 [03:02] */
#define BCHP_XPT_RMX0_FORMAT_reserved3_MASK                        0x0000000c
#define BCHP_XPT_RMX0_FORMAT_reserved3_SHIFT                       2

/* XPT_RMX0 :: FORMAT :: RMX_AMI_MODE [01:01] */
#define BCHP_XPT_RMX0_FORMAT_RMX_AMI_MODE_MASK                     0x00000002
#define BCHP_XPT_RMX0_FORMAT_RMX_AMI_MODE_SHIFT                    1

/* XPT_RMX0 :: FORMAT :: RMX_MUTE [00:00] */
#define BCHP_XPT_RMX0_FORMAT_RMX_MUTE_MASK                         0x00000001
#define BCHP_XPT_RMX0_FORMAT_RMX_MUTE_SHIFT                        0

/***************************************************************************
 *PCR_OFFSET - RMX PCR Constant Offset Register
 ***************************************************************************/
/* XPT_RMX0 :: PCR_OFFSET :: reserved0 [31:28] */
#define BCHP_XPT_RMX0_PCR_OFFSET_reserved0_MASK                    0xf0000000
#define BCHP_XPT_RMX0_PCR_OFFSET_reserved0_SHIFT                   28

/* XPT_RMX0 :: PCR_OFFSET :: RMX_PCR_OFFSET [27:00] */
#define BCHP_XPT_RMX0_PCR_OFFSET_RMX_PCR_OFFSET_MASK               0x0fffffff
#define BCHP_XPT_RMX0_PCR_OFFSET_RMX_PCR_OFFSET_SHIFT              0

/***************************************************************************
 *PSUB1_CTRL - RMX Packet Substitution Control Register
 ***************************************************************************/
/* XPT_RMX0 :: PSUB1_CTRL :: RMX_PSUB1_ONCE [31:31] */
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_ONCE_MASK               0x80000000
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_ONCE_SHIFT              31

/* XPT_RMX0 :: PSUB1_CTRL :: RMX_PSUB1_MODE [30:30] */
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_MODE_MASK               0x40000000
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_MODE_SHIFT              30

/* XPT_RMX0 :: PSUB1_CTRL :: RMX_PSUB1_ADAPT_EN [29:29] */
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_ADAPT_EN_MASK           0x20000000
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_ADAPT_EN_SHIFT          29

/* XPT_RMX0 :: PSUB1_CTRL :: RMX_PSUB1_HD_EN [28:28] */
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_HD_EN_MASK              0x10000000
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_HD_EN_SHIFT             28

/* XPT_RMX0 :: PSUB1_CTRL :: RMX_PSUB1_HD [27:24] */
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_HD_MASK                 0x0f000000
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_HD_SHIFT                24

/* XPT_RMX0 :: PSUB1_CTRL :: RMX_PSUB1_LENGTH [23:16] */
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_LENGTH_MASK             0x00ff0000
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_LENGTH_SHIFT            16

/* XPT_RMX0 :: PSUB1_CTRL :: RMX_PSUB1_START [15:08] */
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_START_MASK              0x0000ff00
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_START_SHIFT             8

/* XPT_RMX0 :: PSUB1_CTRL :: RMX_PSUB1_END [07:00] */
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_END_MASK                0x000000ff
#define BCHP_XPT_RMX0_PSUB1_CTRL_RMX_PSUB1_END_SHIFT               0

/***************************************************************************
 *PSUB1_PID - RMX Packet Substitution PID Register
 ***************************************************************************/
/* XPT_RMX0 :: PSUB1_PID :: reserved0 [31:13] */
#define BCHP_XPT_RMX0_PSUB1_PID_reserved0_MASK                     0xffffe000
#define BCHP_XPT_RMX0_PSUB1_PID_reserved0_SHIFT                    13

/* XPT_RMX0 :: PSUB1_PID :: RMX_PSUB1_PID [12:00] */
#define BCHP_XPT_RMX0_PSUB1_PID_RMX_PSUB1_PID_MASK                 0x00001fff
#define BCHP_XPT_RMX0_PSUB1_PID_RMX_PSUB1_PID_SHIFT                0

/***************************************************************************
 *PSUB2_CTRL - RMX Packet Substitution Control Register
 ***************************************************************************/
/* XPT_RMX0 :: PSUB2_CTRL :: RMX_PSUB2_ONCE [31:31] */
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_ONCE_MASK               0x80000000
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_ONCE_SHIFT              31

/* XPT_RMX0 :: PSUB2_CTRL :: RMX_PSUB2_MODE [30:30] */
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_MODE_MASK               0x40000000
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_MODE_SHIFT              30

/* XPT_RMX0 :: PSUB2_CTRL :: RMX_PSUB2_ADAPT_EN [29:29] */
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_ADAPT_EN_MASK           0x20000000
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_ADAPT_EN_SHIFT          29

/* XPT_RMX0 :: PSUB2_CTRL :: RMX_PSUB2_HD_EN [28:28] */
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_HD_EN_MASK              0x10000000
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_HD_EN_SHIFT             28

/* XPT_RMX0 :: PSUB2_CTRL :: RMX_PSUB2_HD [27:24] */
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_HD_MASK                 0x0f000000
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_HD_SHIFT                24

/* XPT_RMX0 :: PSUB2_CTRL :: RMX_PSUB2_LENGTH [23:16] */
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_LENGTH_MASK             0x00ff0000
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_LENGTH_SHIFT            16

/* XPT_RMX0 :: PSUB2_CTRL :: RMX_PSUB2_START [15:08] */
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_START_MASK              0x0000ff00
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_START_SHIFT             8

/* XPT_RMX0 :: PSUB2_CTRL :: RMX_PSUB2_END [07:00] */
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_END_MASK                0x000000ff
#define BCHP_XPT_RMX0_PSUB2_CTRL_RMX_PSUB2_END_SHIFT               0

/***************************************************************************
 *PSUB2_PID - RMX Packet Substitution PID Register
 ***************************************************************************/
/* XPT_RMX0 :: PSUB2_PID :: reserved0 [31:13] */
#define BCHP_XPT_RMX0_PSUB2_PID_reserved0_MASK                     0xffffe000
#define BCHP_XPT_RMX0_PSUB2_PID_reserved0_SHIFT                    13

/* XPT_RMX0 :: PSUB2_PID :: RMX_PSUB2_PID [12:00] */
#define BCHP_XPT_RMX0_PSUB2_PID_RMX_PSUB2_PID_MASK                 0x00001fff
#define BCHP_XPT_RMX0_PSUB2_PID_RMX_PSUB2_PID_SHIFT                0

/***************************************************************************
 *PKT_GEN1_CTRL - RMX Packet Generation 1 Control Register
 ***************************************************************************/
/* XPT_RMX0 :: PKT_GEN1_CTRL :: reserved0 [31:03] */
#define BCHP_XPT_RMX0_PKT_GEN1_CTRL_reserved0_MASK                 0xfffffff8
#define BCHP_XPT_RMX0_PKT_GEN1_CTRL_reserved0_SHIFT                3

/* XPT_RMX0 :: PKT_GEN1_CTRL :: RMX_PKT_SUB1_BUSY [02:02] */
#define BCHP_XPT_RMX0_PKT_GEN1_CTRL_RMX_PKT_SUB1_BUSY_MASK         0x00000004
#define BCHP_XPT_RMX0_PKT_GEN1_CTRL_RMX_PKT_SUB1_BUSY_SHIFT        2

/* XPT_RMX0 :: PKT_GEN1_CTRL :: RMX_PKT_GEN1_BUSY [01:01] */
#define BCHP_XPT_RMX0_PKT_GEN1_CTRL_RMX_PKT_GEN1_BUSY_MASK         0x00000002
#define BCHP_XPT_RMX0_PKT_GEN1_CTRL_RMX_PKT_GEN1_BUSY_SHIFT        1

/* XPT_RMX0 :: PKT_GEN1_CTRL :: RMX_PKT_GEN1_EN [00:00] */
#define BCHP_XPT_RMX0_PKT_GEN1_CTRL_RMX_PKT_GEN1_EN_MASK           0x00000001
#define BCHP_XPT_RMX0_PKT_GEN1_CTRL_RMX_PKT_GEN1_EN_SHIFT          0

/***************************************************************************
 *PKT_GEN2_CTRL - RMX Packet Generation 2 Control Register
 ***************************************************************************/
/* XPT_RMX0 :: PKT_GEN2_CTRL :: reserved0 [31:03] */
#define BCHP_XPT_RMX0_PKT_GEN2_CTRL_reserved0_MASK                 0xfffffff8
#define BCHP_XPT_RMX0_PKT_GEN2_CTRL_reserved0_SHIFT                3

/* XPT_RMX0 :: PKT_GEN2_CTRL :: RMX_PKT_SUB2_BUSY [02:02] */
#define BCHP_XPT_RMX0_PKT_GEN2_CTRL_RMX_PKT_SUB2_BUSY_MASK         0x00000004
#define BCHP_XPT_RMX0_PKT_GEN2_CTRL_RMX_PKT_SUB2_BUSY_SHIFT        2

/* XPT_RMX0 :: PKT_GEN2_CTRL :: RMX_PKT_GEN2_BUSY [01:01] */
#define BCHP_XPT_RMX0_PKT_GEN2_CTRL_RMX_PKT_GEN2_BUSY_MASK         0x00000002
#define BCHP_XPT_RMX0_PKT_GEN2_CTRL_RMX_PKT_GEN2_BUSY_SHIFT        1

/* XPT_RMX0 :: PKT_GEN2_CTRL :: RMX_PKT_GEN2_EN [00:00] */
#define BCHP_XPT_RMX0_PKT_GEN2_CTRL_RMX_PKT_GEN2_EN_MASK           0x00000001
#define BCHP_XPT_RMX0_PKT_GEN2_CTRL_RMX_PKT_GEN2_EN_SHIFT          0

/***************************************************************************
 *PKT_DLY_CNT - RMX Packet Delay Control Register
 ***************************************************************************/
/* XPT_RMX0 :: PKT_DLY_CNT :: reserved0 [31:09] */
#define BCHP_XPT_RMX0_PKT_DLY_CNT_reserved0_MASK                   0xfffffe00
#define BCHP_XPT_RMX0_PKT_DLY_CNT_reserved0_SHIFT                  9

/* XPT_RMX0 :: PKT_DLY_CNT :: RMX_PKT_DLY_CNT [08:00] */
#define BCHP_XPT_RMX0_PKT_DLY_CNT_RMX_PKT_DLY_CNT_MASK             0x000001ff
#define BCHP_XPT_RMX0_PKT_DLY_CNT_RMX_PKT_DLY_CNT_SHIFT            0

/***************************************************************************
 *RMX_PACING - RMX Pacing Register
 ***************************************************************************/
/* XPT_RMX0 :: RMX_PACING :: reserved0 [31:11] */
#define BCHP_XPT_RMX0_RMX_PACING_reserved0_MASK                    0xfffff800
#define BCHP_XPT_RMX0_RMX_PACING_reserved0_SHIFT                   11

/* XPT_RMX0 :: RMX_PACING :: TIMEBASE_SEL [10:08] */
#define BCHP_XPT_RMX0_RMX_PACING_TIMEBASE_SEL_MASK                 0x00000700
#define BCHP_XPT_RMX0_RMX_PACING_TIMEBASE_SEL_SHIFT                8

/* XPT_RMX0 :: RMX_PACING :: TIMESTAMP_MODE [07:06] */
#define BCHP_XPT_RMX0_RMX_PACING_TIMESTAMP_MODE_MASK               0x000000c0
#define BCHP_XPT_RMX0_RMX_PACING_TIMESTAMP_MODE_SHIFT              6

/* XPT_RMX0 :: RMX_PACING :: TS_RANGE_MODE [05:05] */
#define BCHP_XPT_RMX0_RMX_PACING_TS_RANGE_MODE_MASK                0x00000020
#define BCHP_XPT_RMX0_RMX_PACING_TS_RANGE_MODE_SHIFT               5

/* XPT_RMX0 :: RMX_PACING :: PACING_START [04:04] */
#define BCHP_XPT_RMX0_RMX_PACING_PACING_START_MASK                 0x00000010
#define BCHP_XPT_RMX0_RMX_PACING_PACING_START_SHIFT                4

/* XPT_RMX0 :: RMX_PACING :: PACING_EN [03:03] */
#define BCHP_XPT_RMX0_RMX_PACING_PACING_EN_MASK                    0x00000008
#define BCHP_XPT_RMX0_RMX_PACING_PACING_EN_SHIFT                   3

/* XPT_RMX0 :: RMX_PACING :: PACING_OFFSET_ADJ_DIS [02:02] */
#define BCHP_XPT_RMX0_RMX_PACING_PACING_OFFSET_ADJ_DIS_MASK        0x00000004
#define BCHP_XPT_RMX0_RMX_PACING_PACING_OFFSET_ADJ_DIS_SHIFT       2

/* XPT_RMX0 :: RMX_PACING :: PACING_AUTOSTART_EN [01:01] */
#define BCHP_XPT_RMX0_RMX_PACING_PACING_AUTOSTART_EN_MASK          0x00000002
#define BCHP_XPT_RMX0_RMX_PACING_PACING_AUTOSTART_EN_SHIFT         1

/* XPT_RMX0 :: RMX_PACING :: reserved1 [00:00] */
#define BCHP_XPT_RMX0_RMX_PACING_reserved1_MASK                    0x00000001
#define BCHP_XPT_RMX0_RMX_PACING_reserved1_SHIFT                   0

/***************************************************************************
 *RMX_TS_ERR_BOUND - Data Transport Remux Timestamp Error Bound Register
 ***************************************************************************/
/* XPT_RMX0 :: RMX_TS_ERR_BOUND :: reserved0 [31:19] */
#define BCHP_XPT_RMX0_RMX_TS_ERR_BOUND_reserved0_MASK              0xfff80000
#define BCHP_XPT_RMX0_RMX_TS_ERR_BOUND_reserved0_SHIFT             19

/* XPT_RMX0 :: RMX_TS_ERR_BOUND :: RMX_TS_ERR_BOUND [18:00] */
#define BCHP_XPT_RMX0_RMX_TS_ERR_BOUND_RMX_TS_ERR_BOUND_MASK       0x0007ffff
#define BCHP_XPT_RMX0_RMX_TS_ERR_BOUND_RMX_TS_ERR_BOUND_SHIFT      0

/***************************************************************************
 *RMX_DEBUG - RMX DEBUG Register
 ***************************************************************************/
/* XPT_RMX0 :: RMX_DEBUG :: reserved0 [31:02] */
#define BCHP_XPT_RMX0_RMX_DEBUG_reserved0_MASK                     0xfffffffc
#define BCHP_XPT_RMX0_RMX_DEBUG_reserved0_SHIFT                    2

/* XPT_RMX0 :: RMX_DEBUG :: RMX_PIDSUB_RESET [01:01] */
#define BCHP_XPT_RMX0_RMX_DEBUG_RMX_PIDSUB_RESET_MASK              0x00000002
#define BCHP_XPT_RMX0_RMX_DEBUG_RMX_PIDSUB_RESET_SHIFT             1

/* XPT_RMX0 :: RMX_DEBUG :: RMX_THROTTLE_DISABLE [00:00] */
#define BCHP_XPT_RMX0_RMX_DEBUG_RMX_THROTTLE_DISABLE_MASK          0x00000001
#define BCHP_XPT_RMX0_RMX_DEBUG_RMX_THROTTLE_DISABLE_SHIFT         0

/***************************************************************************
 *RMX_STATUS - RMX Status Register
 ***************************************************************************/
/* XPT_RMX0 :: RMX_STATUS :: reserved0 [31:15] */
#define BCHP_XPT_RMX0_RMX_STATUS_reserved0_MASK                    0xffff8000
#define BCHP_XPT_RMX0_RMX_STATUS_reserved0_SHIFT                   15

/* XPT_RMX0 :: RMX_STATUS :: RMX_CSEQ_ST [14:11] */
#define BCHP_XPT_RMX0_RMX_STATUS_RMX_CSEQ_ST_MASK                  0x00007800
#define BCHP_XPT_RMX0_RMX_STATUS_RMX_CSEQ_ST_SHIFT                 11

/* XPT_RMX0 :: RMX_STATUS :: RMX_CRBUS_ST [10:08] */
#define BCHP_XPT_RMX0_RMX_STATUS_RMX_CRBUS_ST_MASK                 0x00000700
#define BCHP_XPT_RMX0_RMX_STATUS_RMX_CRBUS_ST_SHIFT                8

/* XPT_RMX0 :: RMX_STATUS :: RMX_WATERMARK [07:00] */
#define BCHP_XPT_RMX0_RMX_STATUS_RMX_WATERMARK_MASK                0x000000ff
#define BCHP_XPT_RMX0_RMX_STATUS_RMX_WATERMARK_SHIFT               0

/***************************************************************************
 *PID_MAP_TABLE%i - RMX PID MAP Table (32 entries)
 ***************************************************************************/
#define BCHP_XPT_RMX0_PID_MAP_TABLEi_ARRAY_BASE                    0x00226100
#define BCHP_XPT_RMX0_PID_MAP_TABLEi_ARRAY_START                   0
#define BCHP_XPT_RMX0_PID_MAP_TABLEi_ARRAY_END                     31
#define BCHP_XPT_RMX0_PID_MAP_TABLEi_ARRAY_ELEMENT_SIZE            32

/***************************************************************************
 *PID_MAP_TABLE%i - RMX PID MAP Table (32 entries)
 ***************************************************************************/
/* XPT_RMX0 :: PID_MAP_TABLEi :: RMX_MAP_ENABLE [31:31] */
#define BCHP_XPT_RMX0_PID_MAP_TABLEi_RMX_MAP_ENABLE_MASK           0x80000000
#define BCHP_XPT_RMX0_PID_MAP_TABLEi_RMX_MAP_ENABLE_SHIFT          31

/* XPT_RMX0 :: PID_MAP_TABLEi :: RMX_BAND_SELECT [30:26] */
#define BCHP_XPT_RMX0_PID_MAP_TABLEi_RMX_BAND_SELECT_MASK          0x7c000000
#define BCHP_XPT_RMX0_PID_MAP_TABLEi_RMX_BAND_SELECT_SHIFT         26

/* XPT_RMX0 :: PID_MAP_TABLEi :: RMX_OUTPUT_PID [25:13] */
#define BCHP_XPT_RMX0_PID_MAP_TABLEi_RMX_OUTPUT_PID_MASK           0x03ffe000
#define BCHP_XPT_RMX0_PID_MAP_TABLEi_RMX_OUTPUT_PID_SHIFT          13

/* XPT_RMX0 :: PID_MAP_TABLEi :: RMX_INPUT_PID [12:00] */
#define BCHP_XPT_RMX0_PID_MAP_TABLEi_RMX_INPUT_PID_MASK            0x00001fff
#define BCHP_XPT_RMX0_PID_MAP_TABLEi_RMX_INPUT_PID_SHIFT           0


/***************************************************************************
 *PKT_SUB1_TABLE%i - RMX Packet Substitution 1 Table (47 entries)
 ***************************************************************************/
#define BCHP_XPT_RMX0_PKT_SUB1_TABLEi_ARRAY_BASE                   0x00226200
#define BCHP_XPT_RMX0_PKT_SUB1_TABLEi_ARRAY_START                  0
#define BCHP_XPT_RMX0_PKT_SUB1_TABLEi_ARRAY_END                    46
#define BCHP_XPT_RMX0_PKT_SUB1_TABLEi_ARRAY_ELEMENT_SIZE           32

/***************************************************************************
 *PKT_SUB1_TABLE%i - RMX Packet Substitution 1 Table (47 entries)
 ***************************************************************************/
/* XPT_RMX0 :: PKT_SUB1_TABLEi :: RMX_PSUB_BYTE1 [31:24] */
#define BCHP_XPT_RMX0_PKT_SUB1_TABLEi_RMX_PSUB_BYTE1_MASK          0xff000000
#define BCHP_XPT_RMX0_PKT_SUB1_TABLEi_RMX_PSUB_BYTE1_SHIFT         24

/* XPT_RMX0 :: PKT_SUB1_TABLEi :: RMX_PSUB_BYTE2 [23:16] */
#define BCHP_XPT_RMX0_PKT_SUB1_TABLEi_RMX_PSUB_BYTE2_MASK          0x00ff0000
#define BCHP_XPT_RMX0_PKT_SUB1_TABLEi_RMX_PSUB_BYTE2_SHIFT         16

/* XPT_RMX0 :: PKT_SUB1_TABLEi :: RMX_PSUB_BYTE3 [15:08] */
#define BCHP_XPT_RMX0_PKT_SUB1_TABLEi_RMX_PSUB_BYTE3_MASK          0x0000ff00
#define BCHP_XPT_RMX0_PKT_SUB1_TABLEi_RMX_PSUB_BYTE3_SHIFT         8

/* XPT_RMX0 :: PKT_SUB1_TABLEi :: RMX_PSUB_BYTE4 [07:00] */
#define BCHP_XPT_RMX0_PKT_SUB1_TABLEi_RMX_PSUB_BYTE4_MASK          0x000000ff
#define BCHP_XPT_RMX0_PKT_SUB1_TABLEi_RMX_PSUB_BYTE4_SHIFT         0


/***************************************************************************
 *PKT_SUB2_TABLE%i - RMX Packet Substitution 2 Table (47 entries)
 ***************************************************************************/
#define BCHP_XPT_RMX0_PKT_SUB2_TABLEi_ARRAY_BASE                   0x002262bc
#define BCHP_XPT_RMX0_PKT_SUB2_TABLEi_ARRAY_START                  0
#define BCHP_XPT_RMX0_PKT_SUB2_TABLEi_ARRAY_END                    46
#define BCHP_XPT_RMX0_PKT_SUB2_TABLEi_ARRAY_ELEMENT_SIZE           32

/***************************************************************************
 *PKT_SUB2_TABLE%i - RMX Packet Substitution 2 Table (47 entries)
 ***************************************************************************/
/* XPT_RMX0 :: PKT_SUB2_TABLEi :: RMX_PSUB_BYTE1 [31:24] */
#define BCHP_XPT_RMX0_PKT_SUB2_TABLEi_RMX_PSUB_BYTE1_MASK          0xff000000
#define BCHP_XPT_RMX0_PKT_SUB2_TABLEi_RMX_PSUB_BYTE1_SHIFT         24

/* XPT_RMX0 :: PKT_SUB2_TABLEi :: RMX_PSUB_BYTE2 [23:16] */
#define BCHP_XPT_RMX0_PKT_SUB2_TABLEi_RMX_PSUB_BYTE2_MASK          0x00ff0000
#define BCHP_XPT_RMX0_PKT_SUB2_TABLEi_RMX_PSUB_BYTE2_SHIFT         16

/* XPT_RMX0 :: PKT_SUB2_TABLEi :: RMX_PSUB_BYTE3 [15:08] */
#define BCHP_XPT_RMX0_PKT_SUB2_TABLEi_RMX_PSUB_BYTE3_MASK          0x0000ff00
#define BCHP_XPT_RMX0_PKT_SUB2_TABLEi_RMX_PSUB_BYTE3_SHIFT         8

/* XPT_RMX0 :: PKT_SUB2_TABLEi :: RMX_PSUB_BYTE4 [07:00] */
#define BCHP_XPT_RMX0_PKT_SUB2_TABLEi_RMX_PSUB_BYTE4_MASK          0x000000ff
#define BCHP_XPT_RMX0_PKT_SUB2_TABLEi_RMX_PSUB_BYTE4_SHIFT         0


#endif /* #ifndef BCHP_XPT_RMX0_H__ */

/* End of File */
