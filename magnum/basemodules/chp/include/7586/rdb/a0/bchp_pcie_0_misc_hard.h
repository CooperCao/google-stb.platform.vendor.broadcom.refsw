/****************************************************************************
 *     Copyright (c) 1999-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *                     DO NOT EDIT THIS FILE DIRECTLY
 *
 * This module was generated magically with RDB from a source description
 * file. You must edit the source file for changes to be made to this file.
 *
 *
 * Date:           Generated on               Wed Feb 11 10:13:57 2015
 *                 Full Compile MD5 Checksum  f7f4bd55341805fcfe958ba5e47e65f4
 *                     (minus title and desc)
 *                 MD5 Checksum               95b679a9655597a92593cae55222c397
 *
 * Compiled with:  RDB Utility                combo_header.pl
 *                 RDB.pm                     15653
 *                 unknown                    unknown
 *                 Perl Interpreter           5.008008
 *                 Operating System           linux
 *
 *
 ***************************************************************************/

#ifndef BCHP_PCIE_0_MISC_HARD_H__
#define BCHP_PCIE_0_MISC_HARD_H__

/***************************************************************************
 *PCIE_0_MISC_HARD - PCI-E Miscellaneous Registers (Hard reset)
 ***************************************************************************/
#define BCHP_PCIE_0_MISC_HARD_ECO_CTRL_HARD      0x00474200 /* [RW] ECO Hard Reset Control Register */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG    0x00474204 /* [RW] PCIE Hard Debug Register */

/***************************************************************************
 *ECO_CTRL_HARD - ECO Hard Reset Control Register
 ***************************************************************************/
/* PCIE_0_MISC_HARD :: ECO_CTRL_HARD :: reserved0 [31:16] */
#define BCHP_PCIE_0_MISC_HARD_ECO_CTRL_HARD_reserved0_MASK         0xffff0000
#define BCHP_PCIE_0_MISC_HARD_ECO_CTRL_HARD_reserved0_SHIFT        16

/* PCIE_0_MISC_HARD :: ECO_CTRL_HARD :: ECO_HARD [15:00] */
#define BCHP_PCIE_0_MISC_HARD_ECO_CTRL_HARD_ECO_HARD_MASK          0x0000ffff
#define BCHP_PCIE_0_MISC_HARD_ECO_CTRL_HARD_ECO_HARD_SHIFT         0
#define BCHP_PCIE_0_MISC_HARD_ECO_CTRL_HARD_ECO_HARD_DEFAULT       0x00000000

/***************************************************************************
 *PCIE_HARD_DEBUG - PCIE Hard Debug Register
 ***************************************************************************/
/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: DOWNSTREAM_PERST_DELAY_MODE [31:30] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_DOWNSTREAM_PERST_DELAY_MODE_MASK 0xc0000000
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_DOWNSTREAM_PERST_DELAY_MODE_SHIFT 30
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_DOWNSTREAM_PERST_DELAY_MODE_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: DOWNSTREAM_PERST_DELAY_TIMER [29:28] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_DOWNSTREAM_PERST_DELAY_TIMER_MASK 0x30000000
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_DOWNSTREAM_PERST_DELAY_TIMER_SHIFT 28
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_DOWNSTREAM_PERST_DELAY_TIMER_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: SERDES_IDDQ [27:27] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_SERDES_IDDQ_MASK     0x08000000
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_SERDES_IDDQ_SHIFT    27
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_SERDES_IDDQ_DEFAULT  0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: SERDES_RESET [26:26] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_SERDES_RESET_MASK    0x04000000
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_SERDES_RESET_SHIFT   26
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_SERDES_RESET_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: SERDES_MDIO_RESET [25:25] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_SERDES_MDIO_RESET_MASK 0x02000000
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_SERDES_MDIO_RESET_SHIFT 25
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_SERDES_MDIO_RESET_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: SERDES_TEST_MODE [24:24] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_SERDES_TEST_MODE_MASK 0x01000000
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_SERDES_TEST_MODE_SHIFT 24
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_SERDES_TEST_MODE_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: CORE_CLOCK_SPEED [23:23] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_CORE_CLOCK_SPEED_MASK 0x00800000
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_CORE_CLOCK_SPEED_SHIFT 23
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_CORE_CLOCK_SPEED_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: reserved0 [22:21] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_reserved0_MASK       0x00600000
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_reserved0_SHIFT      21

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: REFCLK_OVERRIDE_OUT [20:20] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_REFCLK_OVERRIDE_OUT_MASK 0x00100000
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_REFCLK_OVERRIDE_OUT_SHIFT 20
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_REFCLK_OVERRIDE_OUT_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: REFCLK_OVERRIDE_IN [19:17] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_REFCLK_OVERRIDE_IN_MASK 0x000e0000
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_REFCLK_OVERRIDE_IN_SHIFT 17
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_REFCLK_OVERRIDE_IN_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: REFCLK_OVERRIDE [16:16] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_REFCLK_OVERRIDE_MASK 0x00010000
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_REFCLK_OVERRIDE_SHIFT 16
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_REFCLK_OVERRIDE_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: PCIE_TMUX_SEL [15:08] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_PCIE_TMUX_SEL_MASK   0x0000ff00
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_PCIE_TMUX_SEL_SHIFT  8
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_PCIE_TMUX_SEL_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: WATCHDOG_PERST_CTRL [07:07] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_WATCHDOG_PERST_CTRL_MASK 0x00000080
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_WATCHDOG_PERST_CTRL_SHIFT 7
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_WATCHDOG_PERST_CTRL_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: LOCAL_PERST_CTRL [06:06] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_LOCAL_PERST_CTRL_MASK 0x00000040
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_LOCAL_PERST_CTRL_SHIFT 6
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_LOCAL_PERST_CTRL_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: LOCAL_BRIDGE_RESET_CTRL [05:05] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_LOCAL_BRIDGE_RESET_CTRL_MASK 0x00000020
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_LOCAL_BRIDGE_RESET_CTRL_SHIFT 5
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_LOCAL_BRIDGE_RESET_CTRL_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: AUXCLK_ENABLE [04:04] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_AUXCLK_ENABLE_MASK   0x00000010
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_AUXCLK_ENABLE_SHIFT  4
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_AUXCLK_ENABLE_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: DOWNSTREAM_PERST_ASSERT [03:03] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_DOWNSTREAM_PERST_ASSERT_MASK 0x00000008
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_DOWNSTREAM_PERST_ASSERT_SHIFT 3
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_DOWNSTREAM_PERST_ASSERT_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: PCIECORE_ADDR_MODE [02:02] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_PCIECORE_ADDR_MODE_MASK 0x00000004
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_PCIECORE_ADDR_MODE_SHIFT 2
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_PCIECORE_ADDR_MODE_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: CLKREQ_DEBUG_ENABLE [01:01] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_CLKREQ_DEBUG_ENABLE_MASK 0x00000002
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_CLKREQ_DEBUG_ENABLE_SHIFT 1
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_CLKREQ_DEBUG_ENABLE_DEFAULT 0x00000000

/* PCIE_0_MISC_HARD :: PCIE_HARD_DEBUG :: HOTRESET_DISABLE [00:00] */
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_HOTRESET_DISABLE_MASK 0x00000001
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_HOTRESET_DISABLE_SHIFT 0
#define BCHP_PCIE_0_MISC_HARD_PCIE_HARD_DEBUG_HOTRESET_DISABLE_DEFAULT 0x00000000

#endif /* #ifndef BCHP_PCIE_0_MISC_HARD_H__ */

/* End of File */
