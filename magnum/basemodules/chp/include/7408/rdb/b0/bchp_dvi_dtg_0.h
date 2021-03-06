/***************************************************************************
 *     Copyright (c) 1999-2010, Broadcom Corporation
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
 * Date:           Generated on         Wed Aug  4 16:34:57 2010
 *                 MD5 Checksum         27c5a1259680e8176595e9e88d9958c6
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

#ifndef BCHP_DVI_DTG_0_H__
#define BCHP_DVI_DTG_0_H__

/***************************************************************************
 *DVI_DTG_0 - DVI Frontend Timing Control 0
 ***************************************************************************/
#define BCHP_DVI_DTG_0_DTG_REV_ID                0x007f4000 /* Revision ID register */
#define BCHP_DVI_DTG_0_DTG_CONFIG                0x007f4008 /* DTG Configuration Register */
#define BCHP_DVI_DTG_0_CCIR_PCL                  0x007f4010 /* PCL Registers for CCIR control singals */
#define BCHP_DVI_DTG_0_DVI_PCL                   0x007f4014 /* PCL Registers for DVI control signals */
#define BCHP_DVI_DTG_0_CNTL_PCL                  0x007f4018 /* PCL Registers for Misc. control singals */
#define BCHP_DVI_DTG_0_RAM_ADDR                  0x007f401c /* DTG Starting Address Register */
#define BCHP_DVI_DTG_0_DTG_BVB_SIZE              0x007f4020 /* BVB Size Register. */
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS           0x007f4024 /* BVB status read Register. */
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS           0x007f4028 /* BVB status clear Register. */
#define BCHP_DVI_DTG_0_DTG_TRIGGER_0             0x007f402c /* DTG Trigger Register 0 */
#define BCHP_DVI_DTG_0_DTG_TRIGGER_1             0x007f4030 /* DTG Trigger Register 1 */
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT             0x007f4034 /* DTG Control Bus Status Register */
#define BCHP_DVI_DTG_0_DTG_LCNTR                 0x007f4038 /* DTG Line Counter Register */
#define BCHP_DVI_DTG_0_DTG_120HZ_CTRL            0x007f403c /* DVI FEEDER trigger control */
#define BCHP_DVI_DTG_0_DTG_MSYNC_CTRL            0x007f4040 /* Master Sync Control */
#define BCHP_DVI_DTG_0_DTG_SSYNC_CTRL            0x007f4044 /* Slave Sync Control */
#define BCHP_DVI_DTG_0_DTG_MS_TIMEOUT            0x007f4048 /* Master Slave Time Out register */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START          0x007f404c /* Master Slave Sync Start */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_PCL            0x007f4050 /* Master Slave flag select PCL */
#define BCHP_DVI_DTG_0_DTG_MSYNC_PHASE           0x007f4054 /* Master Sync Phase */
#define BCHP_DVI_DTG_0_DTG_EOF0_LINE             0x007f4058 /* Field0 End line number for interlaced format */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS         0x007f405c /* "Status register for MSSYNC" */

/***************************************************************************
 *DTG_REV_ID - Revision ID register
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_REV_ID :: reserved0 [31:16] */
#define BCHP_DVI_DTG_0_DTG_REV_ID_reserved0_MASK                   0xffff0000
#define BCHP_DVI_DTG_0_DTG_REV_ID_reserved0_SHIFT                  16

/* DVI_DTG_0 :: DTG_REV_ID :: REVISION_ID [15:00] */
#define BCHP_DVI_DTG_0_DTG_REV_ID_REVISION_ID_MASK                 0x0000ffff
#define BCHP_DVI_DTG_0_DTG_REV_ID_REVISION_ID_SHIFT                0

/***************************************************************************
 *DTG_CONFIG - DTG Configuration Register
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_CONFIG :: reserved0 [31:13] */
#define BCHP_DVI_DTG_0_DTG_CONFIG_reserved0_MASK                   0xffffe000
#define BCHP_DVI_DTG_0_DTG_CONFIG_reserved0_SHIFT                  13

/* DVI_DTG_0 :: DTG_CONFIG :: SUPPRESS_TRIGGER0 [12:12] */
#define BCHP_DVI_DTG_0_DTG_CONFIG_SUPPRESS_TRIGGER0_MASK           0x00001000
#define BCHP_DVI_DTG_0_DTG_CONFIG_SUPPRESS_TRIGGER0_SHIFT          12
#define BCHP_DVI_DTG_0_DTG_CONFIG_SUPPRESS_TRIGGER0_DISABLED       0
#define BCHP_DVI_DTG_0_DTG_CONFIG_SUPPRESS_TRIGGER0_ENABLED        1

/* DVI_DTG_0 :: DTG_CONFIG :: AUTO_RESTART [11:11] */
#define BCHP_DVI_DTG_0_DTG_CONFIG_AUTO_RESTART_MASK                0x00000800
#define BCHP_DVI_DTG_0_DTG_CONFIG_AUTO_RESTART_SHIFT               11
#define BCHP_DVI_DTG_0_DTG_CONFIG_AUTO_RESTART_ON                  1
#define BCHP_DVI_DTG_0_DTG_CONFIG_AUTO_RESTART_OFF                 0

/* DVI_DTG_0 :: DTG_CONFIG :: RESTART_WIN [10:06] */
#define BCHP_DVI_DTG_0_DTG_CONFIG_RESTART_WIN_MASK                 0x000007c0
#define BCHP_DVI_DTG_0_DTG_CONFIG_RESTART_WIN_SHIFT                6

/* DVI_DTG_0 :: DTG_CONFIG :: TOGGLE_DVI_DE [05:05] */
#define BCHP_DVI_DTG_0_DTG_CONFIG_TOGGLE_DVI_DE_MASK               0x00000020
#define BCHP_DVI_DTG_0_DTG_CONFIG_TOGGLE_DVI_DE_SHIFT              5
#define BCHP_DVI_DTG_0_DTG_CONFIG_TOGGLE_DVI_DE_SAME               0
#define BCHP_DVI_DTG_0_DTG_CONFIG_TOGGLE_DVI_DE_INVERT             1

/* DVI_DTG_0 :: DTG_CONFIG :: TOGGLE_DVI_V [04:04] */
#define BCHP_DVI_DTG_0_DTG_CONFIG_TOGGLE_DVI_V_MASK                0x00000010
#define BCHP_DVI_DTG_0_DTG_CONFIG_TOGGLE_DVI_V_SHIFT               4
#define BCHP_DVI_DTG_0_DTG_CONFIG_TOGGLE_DVI_V_SAME                0
#define BCHP_DVI_DTG_0_DTG_CONFIG_TOGGLE_DVI_V_INVERT              1

/* DVI_DTG_0 :: DTG_CONFIG :: TOGGLE_DVI_H [03:03] */
#define BCHP_DVI_DTG_0_DTG_CONFIG_TOGGLE_DVI_H_MASK                0x00000008
#define BCHP_DVI_DTG_0_DTG_CONFIG_TOGGLE_DVI_H_SHIFT               3
#define BCHP_DVI_DTG_0_DTG_CONFIG_TOGGLE_DVI_H_SAME                0
#define BCHP_DVI_DTG_0_DTG_CONFIG_TOGGLE_DVI_H_INVERT              1

/* DVI_DTG_0 :: DTG_CONFIG :: TRIGGER_CNT_CLR_COND [02:02] */
#define BCHP_DVI_DTG_0_DTG_CONFIG_TRIGGER_CNT_CLR_COND_MASK        0x00000004
#define BCHP_DVI_DTG_0_DTG_CONFIG_TRIGGER_CNT_CLR_COND_SHIFT       2

/* DVI_DTG_0 :: DTG_CONFIG :: SLAVE_MODE [01:01] */
#define BCHP_DVI_DTG_0_DTG_CONFIG_SLAVE_MODE_MASK                  0x00000002
#define BCHP_DVI_DTG_0_DTG_CONFIG_SLAVE_MODE_SHIFT                 1
#define BCHP_DVI_DTG_0_DTG_CONFIG_SLAVE_MODE_DISABLED              0
#define BCHP_DVI_DTG_0_DTG_CONFIG_SLAVE_MODE_ENABLED               1

/* DVI_DTG_0 :: DTG_CONFIG :: MCS_ENABLE [00:00] */
#define BCHP_DVI_DTG_0_DTG_CONFIG_MCS_ENABLE_MASK                  0x00000001
#define BCHP_DVI_DTG_0_DTG_CONFIG_MCS_ENABLE_SHIFT                 0

/***************************************************************************
 *CCIR_PCL - PCL Registers for CCIR control singals
 ***************************************************************************/
/* DVI_DTG_0 :: CCIR_PCL :: reserved0 [31:12] */
#define BCHP_DVI_DTG_0_CCIR_PCL_reserved0_MASK                     0xfffff000
#define BCHP_DVI_DTG_0_CCIR_PCL_reserved0_SHIFT                    12

/* DVI_DTG_0 :: CCIR_PCL :: ODD_EVEN_SEL [11:10] */
#define BCHP_DVI_DTG_0_CCIR_PCL_ODD_EVEN_SEL_MASK                  0x00000c00
#define BCHP_DVI_DTG_0_CCIR_PCL_ODD_EVEN_SEL_SHIFT                 10

/* DVI_DTG_0 :: CCIR_PCL :: ODD_EVEN_ENABLE [09:09] */
#define BCHP_DVI_DTG_0_CCIR_PCL_ODD_EVEN_ENABLE_MASK               0x00000200
#define BCHP_DVI_DTG_0_CCIR_PCL_ODD_EVEN_ENABLE_SHIFT              9
#define BCHP_DVI_DTG_0_CCIR_PCL_ODD_EVEN_ENABLE_DISABLED           0
#define BCHP_DVI_DTG_0_CCIR_PCL_ODD_EVEN_ENABLE_ENABLED            1

/* DVI_DTG_0 :: CCIR_PCL :: VBLANK_SEL [08:07] */
#define BCHP_DVI_DTG_0_CCIR_PCL_VBLANK_SEL_MASK                    0x00000180
#define BCHP_DVI_DTG_0_CCIR_PCL_VBLANK_SEL_SHIFT                   7

/* DVI_DTG_0 :: CCIR_PCL :: VBLANK_ENABLE [06:06] */
#define BCHP_DVI_DTG_0_CCIR_PCL_VBLANK_ENABLE_MASK                 0x00000040
#define BCHP_DVI_DTG_0_CCIR_PCL_VBLANK_ENABLE_SHIFT                6
#define BCHP_DVI_DTG_0_CCIR_PCL_VBLANK_ENABLE_DISABLED             0
#define BCHP_DVI_DTG_0_CCIR_PCL_VBLANK_ENABLE_ENABLED              1

/* DVI_DTG_0 :: CCIR_PCL :: VACTIVE_SEL [05:04] */
#define BCHP_DVI_DTG_0_CCIR_PCL_VACTIVE_SEL_MASK                   0x00000030
#define BCHP_DVI_DTG_0_CCIR_PCL_VACTIVE_SEL_SHIFT                  4

/* DVI_DTG_0 :: CCIR_PCL :: VACTIVE_ENABLE [03:03] */
#define BCHP_DVI_DTG_0_CCIR_PCL_VACTIVE_ENABLE_MASK                0x00000008
#define BCHP_DVI_DTG_0_CCIR_PCL_VACTIVE_ENABLE_SHIFT               3
#define BCHP_DVI_DTG_0_CCIR_PCL_VACTIVE_ENABLE_DISABLED            0
#define BCHP_DVI_DTG_0_CCIR_PCL_VACTIVE_ENABLE_ENABLED             1

/* DVI_DTG_0 :: CCIR_PCL :: HACTIVE_SEL [02:01] */
#define BCHP_DVI_DTG_0_CCIR_PCL_HACTIVE_SEL_MASK                   0x00000006
#define BCHP_DVI_DTG_0_CCIR_PCL_HACTIVE_SEL_SHIFT                  1

/* DVI_DTG_0 :: CCIR_PCL :: HACTIVE_ENABLE [00:00] */
#define BCHP_DVI_DTG_0_CCIR_PCL_HACTIVE_ENABLE_MASK                0x00000001
#define BCHP_DVI_DTG_0_CCIR_PCL_HACTIVE_ENABLE_SHIFT               0
#define BCHP_DVI_DTG_0_CCIR_PCL_HACTIVE_ENABLE_DISABLED            0
#define BCHP_DVI_DTG_0_CCIR_PCL_HACTIVE_ENABLE_ENABLED             1

/***************************************************************************
 *DVI_PCL - PCL Registers for DVI control signals
 ***************************************************************************/
/* DVI_DTG_0 :: DVI_PCL :: reserved0 [31:09] */
#define BCHP_DVI_DTG_0_DVI_PCL_reserved0_MASK                      0xfffffe00
#define BCHP_DVI_DTG_0_DVI_PCL_reserved0_SHIFT                     9

/* DVI_DTG_0 :: DVI_PCL :: DVI_DE_SEL [08:07] */
#define BCHP_DVI_DTG_0_DVI_PCL_DVI_DE_SEL_MASK                     0x00000180
#define BCHP_DVI_DTG_0_DVI_PCL_DVI_DE_SEL_SHIFT                    7

/* DVI_DTG_0 :: DVI_PCL :: DVI_DE_ENABLE [06:06] */
#define BCHP_DVI_DTG_0_DVI_PCL_DVI_DE_ENABLE_MASK                  0x00000040
#define BCHP_DVI_DTG_0_DVI_PCL_DVI_DE_ENABLE_SHIFT                 6
#define BCHP_DVI_DTG_0_DVI_PCL_DVI_DE_ENABLE_DISABLED              0
#define BCHP_DVI_DTG_0_DVI_PCL_DVI_DE_ENABLE_ENABLED               1

/* DVI_DTG_0 :: DVI_PCL :: DVI_V_SEL [05:04] */
#define BCHP_DVI_DTG_0_DVI_PCL_DVI_V_SEL_MASK                      0x00000030
#define BCHP_DVI_DTG_0_DVI_PCL_DVI_V_SEL_SHIFT                     4

/* DVI_DTG_0 :: DVI_PCL :: DVI_V_ENABLE [03:03] */
#define BCHP_DVI_DTG_0_DVI_PCL_DVI_V_ENABLE_MASK                   0x00000008
#define BCHP_DVI_DTG_0_DVI_PCL_DVI_V_ENABLE_SHIFT                  3
#define BCHP_DVI_DTG_0_DVI_PCL_DVI_V_ENABLE_DISABLED               0
#define BCHP_DVI_DTG_0_DVI_PCL_DVI_V_ENABLE_ENABLED                1

/* DVI_DTG_0 :: DVI_PCL :: DIGITAL_HSYNC_SEL [02:01] */
#define BCHP_DVI_DTG_0_DVI_PCL_DIGITAL_HSYNC_SEL_MASK              0x00000006
#define BCHP_DVI_DTG_0_DVI_PCL_DIGITAL_HSYNC_SEL_SHIFT             1

/* DVI_DTG_0 :: DVI_PCL :: DIGITAL_HSYNC_ENABLE [00:00] */
#define BCHP_DVI_DTG_0_DVI_PCL_DIGITAL_HSYNC_ENABLE_MASK           0x00000001
#define BCHP_DVI_DTG_0_DVI_PCL_DIGITAL_HSYNC_ENABLE_SHIFT          0
#define BCHP_DVI_DTG_0_DVI_PCL_DIGITAL_HSYNC_ENABLE_DISABLED       0
#define BCHP_DVI_DTG_0_DVI_PCL_DIGITAL_HSYNC_ENABLE_ENABLED        1

/***************************************************************************
 *CNTL_PCL - PCL Registers for Misc. control singals
 ***************************************************************************/
/* DVI_DTG_0 :: CNTL_PCL :: reserved0 [31:02] */
#define BCHP_DVI_DTG_0_CNTL_PCL_reserved0_MASK                     0xfffffffc
#define BCHP_DVI_DTG_0_CNTL_PCL_reserved0_SHIFT                    2

/* DVI_DTG_0 :: CNTL_PCL :: NEW_LINE_CLR_SEL [01:00] */
#define BCHP_DVI_DTG_0_CNTL_PCL_NEW_LINE_CLR_SEL_MASK              0x00000003
#define BCHP_DVI_DTG_0_CNTL_PCL_NEW_LINE_CLR_SEL_SHIFT             0

/***************************************************************************
 *RAM_ADDR - DTG Starting Address Register
 ***************************************************************************/
/* DVI_DTG_0 :: RAM_ADDR :: reserved0 [31:08] */
#define BCHP_DVI_DTG_0_RAM_ADDR_reserved0_MASK                     0xffffff00
#define BCHP_DVI_DTG_0_RAM_ADDR_reserved0_SHIFT                    8

/* DVI_DTG_0 :: RAM_ADDR :: MC_START_ADDR [07:00] */
#define BCHP_DVI_DTG_0_RAM_ADDR_MC_START_ADDR_MASK                 0x000000ff
#define BCHP_DVI_DTG_0_RAM_ADDR_MC_START_ADDR_SHIFT                0

/***************************************************************************
 *DTG_BVB_SIZE - BVB Size Register.
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_BVB_SIZE :: reserved0 [31:27] */
#define BCHP_DVI_DTG_0_DTG_BVB_SIZE_reserved0_MASK                 0xf8000000
#define BCHP_DVI_DTG_0_DTG_BVB_SIZE_reserved0_SHIFT                27

/* DVI_DTG_0 :: DTG_BVB_SIZE :: VERTICAL [26:16] */
#define BCHP_DVI_DTG_0_DTG_BVB_SIZE_VERTICAL_MASK                  0x07ff0000
#define BCHP_DVI_DTG_0_DTG_BVB_SIZE_VERTICAL_SHIFT                 16

/* DVI_DTG_0 :: DTG_BVB_SIZE :: reserved1 [15:11] */
#define BCHP_DVI_DTG_0_DTG_BVB_SIZE_reserved1_MASK                 0x0000f800
#define BCHP_DVI_DTG_0_DTG_BVB_SIZE_reserved1_SHIFT                11

/* DVI_DTG_0 :: DTG_BVB_SIZE :: HORIZONTAL [10:00] */
#define BCHP_DVI_DTG_0_DTG_BVB_SIZE_HORIZONTAL_MASK                0x000007ff
#define BCHP_DVI_DTG_0_DTG_BVB_SIZE_HORIZONTAL_SHIFT               0

/***************************************************************************
 *DTG_BVB_RSTATUS - BVB status read Register.
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_BVB_RSTATUS :: reserved0 [31:05] */
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS_reserved0_MASK              0xffffffe0
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS_reserved0_SHIFT             5

/* DVI_DTG_0 :: DTG_BVB_RSTATUS :: MISSING_SYNC [04:04] */
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS_MISSING_SYNC_MASK           0x00000010
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS_MISSING_SYNC_SHIFT          4

/* DVI_DTG_0 :: DTG_BVB_RSTATUS :: LONG_SOURCE [03:03] */
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS_LONG_SOURCE_MASK            0x00000008
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS_LONG_SOURCE_SHIFT           3

/* DVI_DTG_0 :: DTG_BVB_RSTATUS :: SHORT_SOURCE [02:02] */
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS_SHORT_SOURCE_MASK           0x00000004
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS_SHORT_SOURCE_SHIFT          2

/* DVI_DTG_0 :: DTG_BVB_RSTATUS :: LONG_LINE [01:01] */
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS_LONG_LINE_MASK              0x00000002
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS_LONG_LINE_SHIFT             1

/* DVI_DTG_0 :: DTG_BVB_RSTATUS :: SHORT_LINE [00:00] */
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS_SHORT_LINE_MASK             0x00000001
#define BCHP_DVI_DTG_0_DTG_BVB_RSTATUS_SHORT_LINE_SHIFT            0

/***************************************************************************
 *DTG_BVB_CSTATUS - BVB status clear Register.
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_BVB_CSTATUS :: reserved0 [31:05] */
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS_reserved0_MASK              0xffffffe0
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS_reserved0_SHIFT             5

/* DVI_DTG_0 :: DTG_BVB_CSTATUS :: MISSING_SYNC [04:04] */
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS_MISSING_SYNC_MASK           0x00000010
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS_MISSING_SYNC_SHIFT          4

/* DVI_DTG_0 :: DTG_BVB_CSTATUS :: LONG_SOURCE [03:03] */
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS_LONG_SOURCE_MASK            0x00000008
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS_LONG_SOURCE_SHIFT           3

/* DVI_DTG_0 :: DTG_BVB_CSTATUS :: SHORT_SOURCE [02:02] */
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS_SHORT_SOURCE_MASK           0x00000004
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS_SHORT_SOURCE_SHIFT          2

/* DVI_DTG_0 :: DTG_BVB_CSTATUS :: LONG_LINE [01:01] */
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS_LONG_LINE_MASK              0x00000002
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS_LONG_LINE_SHIFT             1

/* DVI_DTG_0 :: DTG_BVB_CSTATUS :: SHORT_LINE [00:00] */
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS_SHORT_LINE_MASK             0x00000001
#define BCHP_DVI_DTG_0_DTG_BVB_CSTATUS_SHORT_LINE_SHIFT            0

/***************************************************************************
 *DTG_TRIGGER_0 - DTG Trigger Register 0
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_TRIGGER_0 :: reserved0 [31:17] */
#define BCHP_DVI_DTG_0_DTG_TRIGGER_0_reserved0_MASK                0xfffe0000
#define BCHP_DVI_DTG_0_DTG_TRIGGER_0_reserved0_SHIFT               17

/* DVI_DTG_0 :: DTG_TRIGGER_0 :: TRIGGER_SELECT [16:15] */
#define BCHP_DVI_DTG_0_DTG_TRIGGER_0_TRIGGER_SELECT_MASK           0x00018000
#define BCHP_DVI_DTG_0_DTG_TRIGGER_0_TRIGGER_SELECT_SHIFT          15

/* DVI_DTG_0 :: DTG_TRIGGER_0 :: MODULO_COUNT [14:12] */
#define BCHP_DVI_DTG_0_DTG_TRIGGER_0_MODULO_COUNT_MASK             0x00007000
#define BCHP_DVI_DTG_0_DTG_TRIGGER_0_MODULO_COUNT_SHIFT            12

/* DVI_DTG_0 :: DTG_TRIGGER_0 :: ENABLE [11:11] */
#define BCHP_DVI_DTG_0_DTG_TRIGGER_0_ENABLE_MASK                   0x00000800
#define BCHP_DVI_DTG_0_DTG_TRIGGER_0_ENABLE_SHIFT                  11

/* DVI_DTG_0 :: DTG_TRIGGER_0 :: TRIGGER_VALUE [10:00] */
#define BCHP_DVI_DTG_0_DTG_TRIGGER_0_TRIGGER_VALUE_MASK            0x000007ff
#define BCHP_DVI_DTG_0_DTG_TRIGGER_0_TRIGGER_VALUE_SHIFT           0

/***************************************************************************
 *DTG_TRIGGER_1 - DTG Trigger Register 1
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_TRIGGER_1 :: reserved0 [31:17] */
#define BCHP_DVI_DTG_0_DTG_TRIGGER_1_reserved0_MASK                0xfffe0000
#define BCHP_DVI_DTG_0_DTG_TRIGGER_1_reserved0_SHIFT               17

/* DVI_DTG_0 :: DTG_TRIGGER_1 :: TRIGGER_SELECT [16:15] */
#define BCHP_DVI_DTG_0_DTG_TRIGGER_1_TRIGGER_SELECT_MASK           0x00018000
#define BCHP_DVI_DTG_0_DTG_TRIGGER_1_TRIGGER_SELECT_SHIFT          15

/* DVI_DTG_0 :: DTG_TRIGGER_1 :: MODULO_COUNT [14:12] */
#define BCHP_DVI_DTG_0_DTG_TRIGGER_1_MODULO_COUNT_MASK             0x00007000
#define BCHP_DVI_DTG_0_DTG_TRIGGER_1_MODULO_COUNT_SHIFT            12

/* DVI_DTG_0 :: DTG_TRIGGER_1 :: ENABLE [11:11] */
#define BCHP_DVI_DTG_0_DTG_TRIGGER_1_ENABLE_MASK                   0x00000800
#define BCHP_DVI_DTG_0_DTG_TRIGGER_1_ENABLE_SHIFT                  11

/* DVI_DTG_0 :: DTG_TRIGGER_1 :: TRIGGER_VALUE [10:00] */
#define BCHP_DVI_DTG_0_DTG_TRIGGER_1_TRIGGER_VALUE_MASK            0x000007ff
#define BCHP_DVI_DTG_0_DTG_TRIGGER_1_TRIGGER_VALUE_SHIFT           0

/***************************************************************************
 *DTG_CTRL_STAT - DTG Control Bus Status Register
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_CTRL_STAT :: reserved0 [31:07] */
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_reserved0_MASK                0xffffff80
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_reserved0_SHIFT               7

/* DVI_DTG_0 :: DTG_CTRL_STAT :: DVI_DE [06:06] */
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_DVI_DE_MASK                   0x00000040
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_DVI_DE_SHIFT                  6

/* DVI_DTG_0 :: DTG_CTRL_STAT :: DVI_V [05:05] */
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_DVI_V_MASK                    0x00000020
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_DVI_V_SHIFT                   5

/* DVI_DTG_0 :: DTG_CTRL_STAT :: DVI_H [04:04] */
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_DVI_H_MASK                    0x00000010
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_DVI_H_SHIFT                   4

/* DVI_DTG_0 :: DTG_CTRL_STAT :: CCIR_ODD_EVEN [03:03] */
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_CCIR_ODD_EVEN_MASK            0x00000008
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_CCIR_ODD_EVEN_SHIFT           3

/* DVI_DTG_0 :: DTG_CTRL_STAT :: CCIR_VBLANK [02:02] */
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_CCIR_VBLANK_MASK              0x00000004
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_CCIR_VBLANK_SHIFT             2

/* DVI_DTG_0 :: DTG_CTRL_STAT :: CCIR_VACTIVE [01:01] */
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_CCIR_VACTIVE_MASK             0x00000002
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_CCIR_VACTIVE_SHIFT            1

/* DVI_DTG_0 :: DTG_CTRL_STAT :: CCIR_HACTIVE [00:00] */
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_CCIR_HACTIVE_MASK             0x00000001
#define BCHP_DVI_DTG_0_DTG_CTRL_STAT_CCIR_HACTIVE_SHIFT            0

/***************************************************************************
 *DTG_LCNTR - DTG Line Counter Register
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_LCNTR :: reserved0 [31:11] */
#define BCHP_DVI_DTG_0_DTG_LCNTR_reserved0_MASK                    0xfffff800
#define BCHP_DVI_DTG_0_DTG_LCNTR_reserved0_SHIFT                   11

/* DVI_DTG_0 :: DTG_LCNTR :: VALUE [10:00] */
#define BCHP_DVI_DTG_0_DTG_LCNTR_VALUE_MASK                        0x000007ff
#define BCHP_DVI_DTG_0_DTG_LCNTR_VALUE_SHIFT                       0

/***************************************************************************
 *DTG_120HZ_CTRL - DVI FEEDER trigger control
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_120HZ_CTRL :: reserved0 [31:02] */
#define BCHP_DVI_DTG_0_DTG_120HZ_CTRL_reserved0_MASK               0xfffffffc
#define BCHP_DVI_DTG_0_DTG_120HZ_CTRL_reserved0_SHIFT              2

/* DVI_DTG_0 :: DTG_120HZ_CTRL :: FEEDER_TRIGGER0 [01:01] */
#define BCHP_DVI_DTG_0_DTG_120HZ_CTRL_FEEDER_TRIGGER0_MASK         0x00000002
#define BCHP_DVI_DTG_0_DTG_120HZ_CTRL_FEEDER_TRIGGER0_SHIFT        1
#define BCHP_DVI_DTG_0_DTG_120HZ_CTRL_FEEDER_TRIGGER0_ON           1
#define BCHP_DVI_DTG_0_DTG_120HZ_CTRL_FEEDER_TRIGGER0_OFF          0

/* DVI_DTG_0 :: DTG_120HZ_CTRL :: PENDING_FEEDER_ENABLE [00:00] */
#define BCHP_DVI_DTG_0_DTG_120HZ_CTRL_PENDING_FEEDER_ENABLE_MASK   0x00000001
#define BCHP_DVI_DTG_0_DTG_120HZ_CTRL_PENDING_FEEDER_ENABLE_SHIFT  0
#define BCHP_DVI_DTG_0_DTG_120HZ_CTRL_PENDING_FEEDER_ENABLE_ON     1
#define BCHP_DVI_DTG_0_DTG_120HZ_CTRL_PENDING_FEEDER_ENABLE_OFF    0

/***************************************************************************
 *DTG_MSYNC_CTRL - Master Sync Control
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_MSYNC_CTRL :: reserved_for_eco0 [31:30] */
#define BCHP_DVI_DTG_0_DTG_MSYNC_CTRL_reserved_for_eco0_MASK       0xc0000000
#define BCHP_DVI_DTG_0_DTG_MSYNC_CTRL_reserved_for_eco0_SHIFT      30

/* DVI_DTG_0 :: DTG_MSYNC_CTRL :: PH_SHFT_1 [29:25] */
#define BCHP_DVI_DTG_0_DTG_MSYNC_CTRL_PH_SHFT_1_MASK               0x3e000000
#define BCHP_DVI_DTG_0_DTG_MSYNC_CTRL_PH_SHFT_1_SHIFT              25

/* DVI_DTG_0 :: DTG_MSYNC_CTRL :: PH_SHFT_0 [24:20] */
#define BCHP_DVI_DTG_0_DTG_MSYNC_CTRL_PH_SHFT_0_MASK               0x01f00000
#define BCHP_DVI_DTG_0_DTG_MSYNC_CTRL_PH_SHFT_0_SHIFT              20

/* DVI_DTG_0 :: DTG_MSYNC_CTRL :: PH_CNT [19:12] */
#define BCHP_DVI_DTG_0_DTG_MSYNC_CTRL_PH_CNT_MASK                  0x000ff000
#define BCHP_DVI_DTG_0_DTG_MSYNC_CTRL_PH_CNT_SHIFT                 12

/* DVI_DTG_0 :: DTG_MSYNC_CTRL :: INIT_LINE [11:00] */
#define BCHP_DVI_DTG_0_DTG_MSYNC_CTRL_INIT_LINE_MASK               0x00000fff
#define BCHP_DVI_DTG_0_DTG_MSYNC_CTRL_INIT_LINE_SHIFT              0

/***************************************************************************
 *DTG_SSYNC_CTRL - Slave Sync Control
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_SSYNC_CTRL :: reserved0 [31:20] */
#define BCHP_DVI_DTG_0_DTG_SSYNC_CTRL_reserved0_MASK               0xfff00000
#define BCHP_DVI_DTG_0_DTG_SSYNC_CTRL_reserved0_SHIFT              20

/* DVI_DTG_0 :: DTG_SSYNC_CTRL :: INIT_PH_INDEX [19:16] */
#define BCHP_DVI_DTG_0_DTG_SSYNC_CTRL_INIT_PH_INDEX_MASK           0x000f0000
#define BCHP_DVI_DTG_0_DTG_SSYNC_CTRL_INIT_PH_INDEX_SHIFT          16

/* DVI_DTG_0 :: DTG_SSYNC_CTRL :: SYNC_RANGE [15:08] */
#define BCHP_DVI_DTG_0_DTG_SSYNC_CTRL_SYNC_RANGE_MASK              0x0000ff00
#define BCHP_DVI_DTG_0_DTG_SSYNC_CTRL_SYNC_RANGE_SHIFT             8

/* DVI_DTG_0 :: DTG_SSYNC_CTRL :: PH_CNT [07:00] */
#define BCHP_DVI_DTG_0_DTG_SSYNC_CTRL_PH_CNT_MASK                  0x000000ff
#define BCHP_DVI_DTG_0_DTG_SSYNC_CTRL_PH_CNT_SHIFT                 0

/***************************************************************************
 *DTG_MS_TIMEOUT - Master Slave Time Out register
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_MS_TIMEOUT :: H_CNT [31:20] */
#define BCHP_DVI_DTG_0_DTG_MS_TIMEOUT_H_CNT_MASK                   0xfff00000
#define BCHP_DVI_DTG_0_DTG_MS_TIMEOUT_H_CNT_SHIFT                  20

/* DVI_DTG_0 :: DTG_MS_TIMEOUT :: V_CNT [19:09] */
#define BCHP_DVI_DTG_0_DTG_MS_TIMEOUT_V_CNT_MASK                   0x000ffe00
#define BCHP_DVI_DTG_0_DTG_MS_TIMEOUT_V_CNT_SHIFT                  9

/* DVI_DTG_0 :: DTG_MS_TIMEOUT :: FRM_CNT [08:03] */
#define BCHP_DVI_DTG_0_DTG_MS_TIMEOUT_FRM_CNT_MASK                 0x000001f8
#define BCHP_DVI_DTG_0_DTG_MS_TIMEOUT_FRM_CNT_SHIFT                3

/* DVI_DTG_0 :: DTG_MS_TIMEOUT :: CLK_PER_PIX [02:00] */
#define BCHP_DVI_DTG_0_DTG_MS_TIMEOUT_CLK_PER_PIX_MASK             0x00000007
#define BCHP_DVI_DTG_0_DTG_MS_TIMEOUT_CLK_PER_PIX_SHIFT            0

/***************************************************************************
 *DTG_MSSYNC_START - Master Slave Sync Start
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_MSSYNC_START :: reserved0 [31:03] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_reserved0_MASK             0xfffffff8
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_reserved0_SHIFT            3

/* DVI_DTG_0 :: DTG_MSSYNC_START :: CONT_RASTER_EN [02:02] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_CONT_RASTER_EN_MASK        0x00000004
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_CONT_RASTER_EN_SHIFT       2
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_CONT_RASTER_EN_ON          1
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_CONT_RASTER_EN_OFF         0

/* DVI_DTG_0 :: DTG_MSSYNC_START :: MS_EN [01:01] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_MS_EN_MASK                 0x00000002
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_MS_EN_SHIFT                1
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_MS_EN_ON                   1
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_MS_EN_OFF                  0

/* DVI_DTG_0 :: DTG_MSSYNC_START :: START [00:00] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_START_MASK                 0x00000001
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_START_SHIFT                0
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_START_ON                   1
#define BCHP_DVI_DTG_0_DTG_MSSYNC_START_START_OFF                  0

/***************************************************************************
 *DTG_MSSYNC_PCL - Master Slave flag select PCL
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_MSSYNC_PCL :: reserved0 [31:04] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_PCL_reserved0_MASK               0xfffffff0
#define BCHP_DVI_DTG_0_DTG_MSSYNC_PCL_reserved0_SHIFT              4

/* DVI_DTG_0 :: DTG_MSSYNC_PCL :: SEL [03:00] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_PCL_SEL_MASK                     0x0000000f
#define BCHP_DVI_DTG_0_DTG_MSSYNC_PCL_SEL_SHIFT                    0

/***************************************************************************
 *DTG_MSYNC_PHASE - Master Sync Phase
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_MSYNC_PHASE :: VALUE [31:00] */
#define BCHP_DVI_DTG_0_DTG_MSYNC_PHASE_VALUE_MASK                  0xffffffff
#define BCHP_DVI_DTG_0_DTG_MSYNC_PHASE_VALUE_SHIFT                 0

/***************************************************************************
 *DTG_EOF0_LINE - Field0 End line number for interlaced format
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_EOF0_LINE :: reserved0 [31:13] */
#define BCHP_DVI_DTG_0_DTG_EOF0_LINE_reserved0_MASK                0xffffe000
#define BCHP_DVI_DTG_0_DTG_EOF0_LINE_reserved0_SHIFT               13

/* DVI_DTG_0 :: DTG_EOF0_LINE :: COUNT [12:00] */
#define BCHP_DVI_DTG_0_DTG_EOF0_LINE_COUNT_MASK                    0x00001fff
#define BCHP_DVI_DTG_0_DTG_EOF0_LINE_COUNT_SHIFT                   0

/***************************************************************************
 *DTG_MSSYNC_STATUS - "Status register for MSSYNC"
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_MSSYNC_STATUS :: reserved0 [31:30] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_reserved0_MASK            0xc0000000
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_reserved0_SHIFT           30

/* DVI_DTG_0 :: DTG_MSSYNC_STATUS :: SSYNC [29:29] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_SSYNC_MASK                0x20000000
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_SSYNC_SHIFT               29

/* DVI_DTG_0 :: DTG_MSSYNC_STATUS :: MSYNC [28:28] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_MSYNC_MASK                0x10000000
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_MSYNC_SHIFT               28

/* DVI_DTG_0 :: DTG_MSSYNC_STATUS :: MSYNC_COUNT [27:20] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_MSYNC_COUNT_MASK          0x0ff00000
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_MSYNC_COUNT_SHIFT         20

/* DVI_DTG_0 :: DTG_MSSYNC_STATUS :: SSYNC_INDEX [19:12] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_SSYNC_INDEX_MASK          0x000ff000
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_SSYNC_INDEX_SHIFT         12

/* DVI_DTG_0 :: DTG_MSSYNC_STATUS :: SSYNC_LINE_INDEX [11:04] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_SSYNC_LINE_INDEX_MASK     0x00000ff0
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_SSYNC_LINE_INDEX_SHIFT    4

/* DVI_DTG_0 :: DTG_MSSYNC_STATUS :: TIMEOUT [03:03] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_TIMEOUT_MASK              0x00000008
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_TIMEOUT_SHIFT             3

/* DVI_DTG_0 :: DTG_MSSYNC_STATUS :: STATE [02:00] */
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_STATE_MASK                0x00000007
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_STATE_SHIFT               0
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_STATE_INIT                0
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_STATE_RASTER_1FLD         1
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_STATE_SYNC_WAIT           2
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_STATE_IN_SYNC             3
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_STATE_LOST_SYNC           4
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_STATE_TIMEOUT             5
#define BCHP_DVI_DTG_0_DTG_MSSYNC_STATUS_STATE_DISABLE             6

/***************************************************************************
 *DTG_SSYNC_LINE%i - Slave Sync Phase Lines
 ***************************************************************************/
#define BCHP_DVI_DTG_0_DTG_SSYNC_LINEi_ARRAY_BASE                  0x007f4060
#define BCHP_DVI_DTG_0_DTG_SSYNC_LINEi_ARRAY_START                 0
#define BCHP_DVI_DTG_0_DTG_SSYNC_LINEi_ARRAY_END                   15
#define BCHP_DVI_DTG_0_DTG_SSYNC_LINEi_ARRAY_ELEMENT_SIZE          32

/***************************************************************************
 *DTG_SSYNC_LINE%i - Slave Sync Phase Lines
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_SSYNC_LINEi :: reserved0 [31:11] */
#define BCHP_DVI_DTG_0_DTG_SSYNC_LINEi_reserved0_MASK              0xfffff800
#define BCHP_DVI_DTG_0_DTG_SSYNC_LINEi_reserved0_SHIFT             11

/* DVI_DTG_0 :: DTG_SSYNC_LINEi :: COUNT [10:00] */
#define BCHP_DVI_DTG_0_DTG_SSYNC_LINEi_COUNT_MASK                  0x000007ff
#define BCHP_DVI_DTG_0_DTG_SSYNC_LINEi_COUNT_SHIFT                 0


/***************************************************************************
 *DTG_SSYNC_PHASE%i - Slave Sync Phases
 ***************************************************************************/
#define BCHP_DVI_DTG_0_DTG_SSYNC_PHASEi_ARRAY_BASE                 0x007f40a0
#define BCHP_DVI_DTG_0_DTG_SSYNC_PHASEi_ARRAY_START                0
#define BCHP_DVI_DTG_0_DTG_SSYNC_PHASEi_ARRAY_END                  15
#define BCHP_DVI_DTG_0_DTG_SSYNC_PHASEi_ARRAY_ELEMENT_SIZE         32

/***************************************************************************
 *DTG_SSYNC_PHASE%i - Slave Sync Phases
 ***************************************************************************/
/* DVI_DTG_0 :: DTG_SSYNC_PHASEi :: VALUE [31:00] */
#define BCHP_DVI_DTG_0_DTG_SSYNC_PHASEi_VALUE_MASK                 0xffffffff
#define BCHP_DVI_DTG_0_DTG_SSYNC_PHASEi_VALUE_SHIFT                0


/***************************************************************************
 *DMC_IMM_VAL_%i - DTG immediate register for CALLI/SCOUNTI
 ***************************************************************************/
#define BCHP_DVI_DTG_0_DMC_IMM_VAL_i_ARRAY_BASE                    0x007f4100
#define BCHP_DVI_DTG_0_DMC_IMM_VAL_i_ARRAY_START                   0
#define BCHP_DVI_DTG_0_DMC_IMM_VAL_i_ARRAY_END                     15
#define BCHP_DVI_DTG_0_DMC_IMM_VAL_i_ARRAY_ELEMENT_SIZE            32

/***************************************************************************
 *DMC_IMM_VAL_%i - DTG immediate register for CALLI/SCOUNTI
 ***************************************************************************/
/* DVI_DTG_0 :: DMC_IMM_VAL_i :: reserved0 [31:24] */
#define BCHP_DVI_DTG_0_DMC_IMM_VAL_i_reserved0_MASK                0xff000000
#define BCHP_DVI_DTG_0_DMC_IMM_VAL_i_reserved0_SHIFT               24

/* DVI_DTG_0 :: DMC_IMM_VAL_i :: ADDR [23:12] */
#define BCHP_DVI_DTG_0_DMC_IMM_VAL_i_ADDR_MASK                     0x00fff000
#define BCHP_DVI_DTG_0_DMC_IMM_VAL_i_ADDR_SHIFT                    12

/* DVI_DTG_0 :: DMC_IMM_VAL_i :: COUNT [11:00] */
#define BCHP_DVI_DTG_0_DMC_IMM_VAL_i_COUNT_MASK                    0x00000fff
#define BCHP_DVI_DTG_0_DMC_IMM_VAL_i_COUNT_SHIFT                   0


#endif /* #ifndef BCHP_DVI_DTG_0_H__ */

/* End of File */
