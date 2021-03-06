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
 * Date:           Generated on         Fri Jan 22 20:19:12 2010
 *                 MD5 Checksum         a2d1f2163f65e87d228a0fb491cb442d
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

#ifndef BCHP_SMARTCARD_PLL_H__
#define BCHP_SMARTCARD_PLL_H__

/***************************************************************************
 *SMARTCARD_PLL - Smart Card PLL
 ***************************************************************************/
#define BCHP_SMARTCARD_PLL_SC_MACRO              0x00471180 /* Smart Card PLL Sample Rate Macro Select */
#define BCHP_SMARTCARD_PLL_SC_CONTROL            0x00471184 /* Smart Card PLL Control */
#define BCHP_SMARTCARD_PLL_SC_USER_DIV1          0x00471188 /* Smart Card PLL User Divider Settings 1 */
#define BCHP_SMARTCARD_PLL_SC_USER_DIV2          0x0047118c /* Smart Card PLL User Divider Settings 2 */
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV1        0x00471194 /* Smart Card PLL Active Divider Settings 1 */
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2        0x00471198 /* Smart Card PLL Active Divider Settings 2 */
#define BCHP_SMARTCARD_PLL_SC_CTRL               0x004711a0 /* smart card PLL reset, ndiv_mode and powerdown */
#define BCHP_SMARTCARD_PLL_SC_PM_CLOCK_ENA       0x004711a4 /* smart card PLL  clock outputs enable */
#define BCHP_SMARTCARD_PLL_SC_PM_DIS_CHL_1       0x004711a8 /* smart card PLL channel 1 powerdown post divider */

/***************************************************************************
 *SC_MACRO - Smart Card PLL Sample Rate Macro Select
 ***************************************************************************/
/* SMARTCARD_PLL :: SC_MACRO :: reserved0 [31:03] */
#define BCHP_SMARTCARD_PLL_SC_MACRO_reserved0_MASK                 0xfffffff8
#define BCHP_SMARTCARD_PLL_SC_MACRO_reserved0_SHIFT                3

/* SMARTCARD_PLL :: SC_MACRO :: MACRO_SELECT [02:00] */
#define BCHP_SMARTCARD_PLL_SC_MACRO_MACRO_SELECT_MASK              0x00000007
#define BCHP_SMARTCARD_PLL_SC_MACRO_MACRO_SELECT_SHIFT             0
#define BCHP_SMARTCARD_PLL_SC_MACRO_MACRO_SELECT_freq_36p864MHz    0
#define BCHP_SMARTCARD_PLL_SC_MACRO_MACRO_SELECT_freq_27MHz        1
#define BCHP_SMARTCARD_PLL_SC_MACRO_MACRO_SELECT_freq_24MHz        2
#define BCHP_SMARTCARD_PLL_SC_MACRO_MACRO_SELECT_freq_36MHz        3
#define BCHP_SMARTCARD_PLL_SC_MACRO_MACRO_SELECT_User              7

/***************************************************************************
 *SC_CONTROL - Smart Card PLL Control
 ***************************************************************************/
/* SMARTCARD_PLL :: SC_CONTROL :: reserved0 [31:01] */
#define BCHP_SMARTCARD_PLL_SC_CONTROL_reserved0_MASK               0xfffffffe
#define BCHP_SMARTCARD_PLL_SC_CONTROL_reserved0_SHIFT              1

/* SMARTCARD_PLL :: SC_CONTROL :: USER_UPDATE_DIVIDERS [00:00] */
#define BCHP_SMARTCARD_PLL_SC_CONTROL_USER_UPDATE_DIVIDERS_MASK    0x00000001
#define BCHP_SMARTCARD_PLL_SC_CONTROL_USER_UPDATE_DIVIDERS_SHIFT   0
#define BCHP_SMARTCARD_PLL_SC_CONTROL_USER_UPDATE_DIVIDERS_Update_now 1
#define BCHP_SMARTCARD_PLL_SC_CONTROL_USER_UPDATE_DIVIDERS_Dont_update 0

/***************************************************************************
 *SC_USER_DIV1 - Smart Card PLL User Divider Settings 1
 ***************************************************************************/
/* SMARTCARD_PLL :: SC_USER_DIV1 :: reserved0 [31:24] */
#define BCHP_SMARTCARD_PLL_SC_USER_DIV1_reserved0_MASK             0xff000000
#define BCHP_SMARTCARD_PLL_SC_USER_DIV1_reserved0_SHIFT            24

/* SMARTCARD_PLL :: SC_USER_DIV1 :: M3DIV [23:16] */
#define BCHP_SMARTCARD_PLL_SC_USER_DIV1_M3DIV_MASK                 0x00ff0000
#define BCHP_SMARTCARD_PLL_SC_USER_DIV1_M3DIV_SHIFT                16

/* SMARTCARD_PLL :: SC_USER_DIV1 :: M2DIV [15:08] */
#define BCHP_SMARTCARD_PLL_SC_USER_DIV1_M2DIV_MASK                 0x0000ff00
#define BCHP_SMARTCARD_PLL_SC_USER_DIV1_M2DIV_SHIFT                8

/* SMARTCARD_PLL :: SC_USER_DIV1 :: M1DIV [07:00] */
#define BCHP_SMARTCARD_PLL_SC_USER_DIV1_M1DIV_MASK                 0x000000ff
#define BCHP_SMARTCARD_PLL_SC_USER_DIV1_M1DIV_SHIFT                0

/***************************************************************************
 *SC_USER_DIV2 - Smart Card PLL User Divider Settings 2
 ***************************************************************************/
/* SMARTCARD_PLL :: SC_USER_DIV2 :: reserved0 [31:19] */
#define BCHP_SMARTCARD_PLL_SC_USER_DIV2_reserved0_MASK             0xfff80000
#define BCHP_SMARTCARD_PLL_SC_USER_DIV2_reserved0_SHIFT            19

/* SMARTCARD_PLL :: SC_USER_DIV2 :: LCPX [18:14] */
#define BCHP_SMARTCARD_PLL_SC_USER_DIV2_LCPX_MASK                  0x0007c000
#define BCHP_SMARTCARD_PLL_SC_USER_DIV2_LCPX_SHIFT                 14

/* SMARTCARD_PLL :: SC_USER_DIV2 :: VCORNG [13:12] */
#define BCHP_SMARTCARD_PLL_SC_USER_DIV2_VCORNG_MASK                0x00003000
#define BCHP_SMARTCARD_PLL_SC_USER_DIV2_VCORNG_SHIFT               12

/* SMARTCARD_PLL :: SC_USER_DIV2 :: reserved1 [11:09] */
#define BCHP_SMARTCARD_PLL_SC_USER_DIV2_reserved1_MASK             0x00000e00
#define BCHP_SMARTCARD_PLL_SC_USER_DIV2_reserved1_SHIFT            9

/* SMARTCARD_PLL :: SC_USER_DIV2 :: NDIV_INT [08:00] */
#define BCHP_SMARTCARD_PLL_SC_USER_DIV2_NDIV_INT_MASK              0x000001ff
#define BCHP_SMARTCARD_PLL_SC_USER_DIV2_NDIV_INT_SHIFT             0

/***************************************************************************
 *SC_ACTIVE_DIV1 - Smart Card PLL Active Divider Settings 1
 ***************************************************************************/
/* SMARTCARD_PLL :: SC_ACTIVE_DIV1 :: reserved0 [31:24] */
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV1_reserved0_MASK           0xff000000
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV1_reserved0_SHIFT          24

/* SMARTCARD_PLL :: SC_ACTIVE_DIV1 :: M3DIV [23:16] */
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV1_M3DIV_MASK               0x00ff0000
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV1_M3DIV_SHIFT              16

/* SMARTCARD_PLL :: SC_ACTIVE_DIV1 :: M2DIV [15:08] */
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV1_M2DIV_MASK               0x0000ff00
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV1_M2DIV_SHIFT              8

/* SMARTCARD_PLL :: SC_ACTIVE_DIV1 :: M1DIV [07:00] */
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV1_M1DIV_MASK               0x000000ff
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV1_M1DIV_SHIFT              0

/***************************************************************************
 *SC_ACTIVE_DIV2 - Smart Card PLL Active Divider Settings 2
 ***************************************************************************/
/* SMARTCARD_PLL :: SC_ACTIVE_DIV2 :: reserved0 [31:20] */
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_reserved0_MASK           0xfff00000
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_reserved0_SHIFT          20

/* SMARTCARD_PLL :: SC_ACTIVE_DIV2 :: LOCK [19:19] */
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_LOCK_MASK                0x00080000
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_LOCK_SHIFT               19
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_LOCK_Unlocked            0
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_LOCK_Locked              1

/* SMARTCARD_PLL :: SC_ACTIVE_DIV2 :: LCPX [18:14] */
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_LCPX_MASK                0x0007c000
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_LCPX_SHIFT               14

/* SMARTCARD_PLL :: SC_ACTIVE_DIV2 :: VCORNG [13:12] */
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_VCORNG_MASK              0x00003000
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_VCORNG_SHIFT             12

/* SMARTCARD_PLL :: SC_ACTIVE_DIV2 :: reserved1 [11:09] */
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_reserved1_MASK           0x00000e00
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_reserved1_SHIFT          9

/* SMARTCARD_PLL :: SC_ACTIVE_DIV2 :: NDIV_INT [08:00] */
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_NDIV_INT_MASK            0x000001ff
#define BCHP_SMARTCARD_PLL_SC_ACTIVE_DIV2_NDIV_INT_SHIFT           0

/***************************************************************************
 *SC_CTRL - smart card PLL reset, ndiv_mode and powerdown
 ***************************************************************************/
/* SMARTCARD_PLL :: SC_CTRL :: reserved0 [31:04] */
#define BCHP_SMARTCARD_PLL_SC_CTRL_reserved0_MASK                  0xfffffff0
#define BCHP_SMARTCARD_PLL_SC_CTRL_reserved0_SHIFT                 4

/* SMARTCARD_PLL :: SC_CTRL :: POWERDOWN [03:03] */
#define BCHP_SMARTCARD_PLL_SC_CTRL_POWERDOWN_MASK                  0x00000008
#define BCHP_SMARTCARD_PLL_SC_CTRL_POWERDOWN_SHIFT                 3
#define BCHP_SMARTCARD_PLL_SC_CTRL_POWERDOWN_Powerdown             1
#define BCHP_SMARTCARD_PLL_SC_CTRL_POWERDOWN_Normal                0

/* SMARTCARD_PLL :: SC_CTRL :: reserved1 [02:02] */
#define BCHP_SMARTCARD_PLL_SC_CTRL_reserved1_MASK                  0x00000004
#define BCHP_SMARTCARD_PLL_SC_CTRL_reserved1_SHIFT                 2

/* SMARTCARD_PLL :: SC_CTRL :: DRESET [01:01] */
#define BCHP_SMARTCARD_PLL_SC_CTRL_DRESET_MASK                     0x00000002
#define BCHP_SMARTCARD_PLL_SC_CTRL_DRESET_SHIFT                    1
#define BCHP_SMARTCARD_PLL_SC_CTRL_DRESET_Reset                    1
#define BCHP_SMARTCARD_PLL_SC_CTRL_DRESET_Normal                   0

/* SMARTCARD_PLL :: SC_CTRL :: ARESET [00:00] */
#define BCHP_SMARTCARD_PLL_SC_CTRL_ARESET_MASK                     0x00000001
#define BCHP_SMARTCARD_PLL_SC_CTRL_ARESET_SHIFT                    0
#define BCHP_SMARTCARD_PLL_SC_CTRL_ARESET_Reset                    1
#define BCHP_SMARTCARD_PLL_SC_CTRL_ARESET_Normal                   0

/***************************************************************************
 *SC_PM_CLOCK_ENA - smart card PLL  clock outputs enable
 ***************************************************************************/
/* SMARTCARD_PLL :: SC_PM_CLOCK_ENA :: reserved0 [31:01] */
#define BCHP_SMARTCARD_PLL_SC_PM_CLOCK_ENA_reserved0_MASK          0xfffffffe
#define BCHP_SMARTCARD_PLL_SC_PM_CLOCK_ENA_reserved0_SHIFT         1

/* SMARTCARD_PLL :: SC_PM_CLOCK_ENA :: CLOCK_ENA [00:00] */
#define BCHP_SMARTCARD_PLL_SC_PM_CLOCK_ENA_CLOCK_ENA_MASK          0x00000001
#define BCHP_SMARTCARD_PLL_SC_PM_CLOCK_ENA_CLOCK_ENA_SHIFT         0
#define BCHP_SMARTCARD_PLL_SC_PM_CLOCK_ENA_CLOCK_ENA_Enable        1
#define BCHP_SMARTCARD_PLL_SC_PM_CLOCK_ENA_CLOCK_ENA_Disable       0

/***************************************************************************
 *SC_PM_DIS_CHL_1 - smart card PLL channel 1 powerdown post divider
 ***************************************************************************/
/* SMARTCARD_PLL :: SC_PM_DIS_CHL_1 :: reserved0 [31:01] */
#define BCHP_SMARTCARD_PLL_SC_PM_DIS_CHL_1_reserved0_MASK          0xfffffffe
#define BCHP_SMARTCARD_PLL_SC_PM_DIS_CHL_1_reserved0_SHIFT         1

/* SMARTCARD_PLL :: SC_PM_DIS_CHL_1 :: DIS_CH [00:00] */
#define BCHP_SMARTCARD_PLL_SC_PM_DIS_CHL_1_DIS_CH_MASK             0x00000001
#define BCHP_SMARTCARD_PLL_SC_PM_DIS_CHL_1_DIS_CH_SHIFT            0

#endif /* #ifndef BCHP_SMARTCARD_PLL_H__ */

/* End of File */
