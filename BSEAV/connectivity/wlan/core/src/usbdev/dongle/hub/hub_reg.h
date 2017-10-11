/*
 * USB Hub register definitions
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#ifndef _HUBREG_H_
#define _HUBREG_H_

#define B_0		0x01
#define B_1		0x02
#define B_2		0x04
#define B_3		0x08
#define B_4		0x10
#define B_5		0x20
#define B_6		0x40
#define B_7		0x80
#define B_8		0x100
#define B_9		0x200
#define B_10	0x400
#define B_11	0x800
#define B_12	0x1000
#define B_13	0x2000
#define B_14	0x4000
#define B_15	0x8000
#define B_16	0x10000
#define B_17	0x20000
#define B_18	0x40000
#define B_19	0x80000
#define B_20	0x100000
#define B_21	0x200000
#define B_22	0x400000
#define B_23	0x800000
#define B_24	0x1000000
#define B_25	0x2000000
#define B_26	0x4000000
#define B_27	0x8000000
#define B_28	0x10000000
#define B_29	0x20000000
#define B_30	0x40000000
#define B_31	0x80000000

/* 0x1800_c000 - 0x1800_07ff	USB HUB SHIM register region */
#define UHUB_CTRL_adr			0x00000000
#define UHUB_STATUS_adr			0x00000004
#define UHUB_CORECAP_adr		0x00000008
#define UHUB_WORKAROUND_adr		0x000001e4
#define UHUB_SPARESTS_adr		0x000001f0
#define UHUB_COREGPIN_adr		0x000001f4
#define UHUB_PHYUTMICTL1_adr	0x00000310

/* 0x1800_c800 - 0x1800_cfff	USB HUB SHIM register region */
#define UHUB_ENUM0_adr			0x00000000
#define UHUB_ENUM1_adr			0x00000004
#define UHUB_ENUM2_adr			0x00000008
#define UHUB_USP_PORTSC_adr		0x00000020

/* UHUB_CTRL Bits */
#define HUB_SOFT_RESET			(0x1 << 0)
#define HUB_MAINCLK_EN			(0x1 << 1)
#define HUB_AUXCLK_EN			(0x1 << 2)
#define HUB_NONDRV_ONRESET		(0x1 << 3)
#define HUB_BUSPOWERED			(0x1 << 4)
#define HUB_SPEED_MASK			(0x7 << 5)
#define HUB_PLLCNTL_UPDATE		(0x1 << 8)
#define HUB_DIS_USB30D_VBUSMON	(0x1 << 9)
#define HUB_PHY_RST				(0x1 << 12)
#define HUB_USBPHYPLL_SUSP		(0x1 << 13)
#define HUB_FORCEGATE_CLKON		(0x1 << 14)
#define HUB_UTMI_SOFTRST		(0x1 << 26)

/* UHUB_STATUS Bits */
#define HUB_VBUS_PRESENT	(0x1 << 11)
#define HUB_BYPASSED		(0x1 << 20)

/* UHUB_USP_PORTSC Bits */
#define HUB_SOFTCONNECT		(0x1 << 8)
#define HUB_SOFTDISCONNECT	(0x1 << 7)

/* UHUB_ENUM2 Bits */
#define HUB_VENDORID_SHIFT	0
#define HUB_VENDORID_MASK	(0xffff << HUB_VENDORID_SHIFT)
#define HUB_PRODUCTID_SHIFT	16
#define HUB_PRODUCTID_MASK	(0xffff << HUB_PRODUCTID_SHIFT)

/* UHUB_PHYUTMICTL1 Bits */
#define HUB_PLL_RESET_B				(0x1 << 0)
#define HUB_PHY_ISO					(0x1 << 2)
#define HUB_PHY_IDDQ				(0x1 << 3)
#define HUB_AFE_LDOCNTLEN_1P2		(0x1 << 4)
#define HUB_AFE_LDO_PWRDWNB			(0x1 << 5)
#define HUB_AFE_COREDYB_3P3			(0x1 << 6)
#define HUB_PHY_P1CTL_I_SHIFT		12
#define HUB_PHY_P1CTL_I_MASK		(0xff << HUB_PHY_P1CTL_I_SHIFT)
#define HUB_AFE_LDOCNTL_1P2_SHIFT	16
#define HUB_AFE_LDOCNTL_1P2_MASK	(0x7 << HUB_AFE_LDOCNTL_1P2_SHIFT)
#define HUB_AFE_LDBG_OUTADJ_SHIFT	23
#define HUB_AFE_LDBG_OUTADJ_MASK	(0xf << HUB_AFE_LDBG_OUTADJ_SHIFT)

#define HUB_RX_SYNC_DETECT_LEN		5

#endif /* _HUBREG_H_ */
