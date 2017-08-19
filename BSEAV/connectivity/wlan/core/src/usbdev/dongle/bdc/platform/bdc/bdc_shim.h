/*
 *  All access to BDC Interface functions are given here (not BDC core)
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

#ifndef _BDC_SHIM_H_
#define _BDC_SHIM_H_

#include <typedefs.h>

/* Core_control register in DMP  0x18106408 */
#define CCTRL_MAINCLK_EN	B_0
#define CCTRL_FORCE_GTCLK_ON	B_1
#define CCTRL_AUXCLK_EN		B_2
#define CCTRL_NDR_EN		B_3
#define CCTRL_BUSPWR		B_4
#define CCTRL_SPEED_HS		(0x2 << CCTRL_SPEED_SHIFT)
#define CCTRL_SPEED_SHIFT	5

#define UTMI_PHYPT_DISABLE	B_15
#define UTMI_RXSYNC_DETLEN_MASK	(B_14 | B_13 | B_12)
#define UTMI_PHY_IDDQ		B_3
#define UTMI_PHY_ISO		B_2
#define UTMI_PLL_RESETB		B_0
#define USB3_PIPE_RESETB	B_11

#define DEV_CTRL_DEVRESET	B_0
#define DEV_CTRL_USB_AUX_RST_B B_2
#define DEV_CTRL_UTMIPWRDWN B_10
#define DEV_CTRL_ANAPWRDWN B_11
#define DEV_CTRL_PHYRESET	B_12
#define DEV_CTRL_UTMISOFTRST B_26

#define PLL_SEQ_START		B_28
#define PLL_REFCLK40_MODE	B_25
#define PLL_CTRL_UPDATE		B_10

#define XDC_MDIO_SLAVE_ADDR	0x0A

/* MDIO addresses */
#define XDCI_MDIO_BASE_REG	0x1F
#define XDCI_MDIO_PLL30_BLOCK	0x8000
#define XDCI_MDIO_RXPMD_BLOCK	0x8020
#define XDCI_MDIO_TXPMD_BLOCK	0x8040
#define XDCI_MDIO_PIPE_BLOCK	0x8060
#define XDCI_MDIO_AFE30_BLOCK	0x8080
#define XDCI_MDIO_UTMI_BLOCK	0x80A0
#define XDCI_MDIO_AFE20_BLOCK	0x80C0
#define XDCI_MDIO_AEQ_BLOCK	0x80E0
/* XDCI_MDIO_TXPMD_BLOCK */
#define XDCI_MDIO_TXPMD_REV_ID_REG		0x00
#define XDCI_MDIO_TXPMD_GEN_CTRL_REG		0x01
#define XDCI_MDIO_TXPMD_FREQ_CTRL1_REG		0x02
#define XDCI_MDIO_TXPMD_FREQ_CTRL2_REG		0x03
#define XDCI_MDIO_TXPMD_FREQ_CTRL3_REG		0x04
#define XDCI_MDIO_TXPMD_FREQ_CTRL4_REG		0x05
#define XDCI_MDIO_TXPMD_FREQ_CTRL5_REG		0x06
#define XDCI_MDIO_TXPMD_FREQ_CTRL_STAT1_REG	0x07
#define XDCI_MDIO_TXPMD_FREQ_CTRL_STAT2_REG	0x08
#define XDCI_MDIO_TXPMD_FREQ_CTRL_STAT3_REG	0x09
#define XDCI_MDIO_TXPMD_FREQ_CTRL_STAT4_REG	0x0A
#define XDCI_MDIO_TXPMD_HSPMD_STAT1_REG		0x0B
/* XDCI_MDIO_TXPMD_GEN_CTRL_REG fields */
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_ssc_en_frc		B_0
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_ssc_en_frc_val	B_1
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_pmd_en_frc		B_2
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_pmd_en_frc_val	B_3
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_phase_rotate_step	B_4
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_phase_rotate_dir	B_5
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_phase_rotate_trig	B_6
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_phase_rotate_tmenab	B_7
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_phs_stp_inv		B_8
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_host_track_en	B_9
#define XDCI_MDIO_TXPMD_GEN_CTRL_hstr_filt_bypass	B_10
#define XDCI_MDIO_TXPMD_GEN_CTRL_ana_farend_lpbk	B_11
#define XDCI_MDIO_TXPMD_GEN_CTRL_tx_phact_8b4bn		B_12

typedef volatile struct {
	/* Device Control */
	uint32 devcontrol;		/* Dev_control 0x000 */
	uint32 devstatus;			/* Dev_status 0x004 */
	uint32 PAD0[16];
	uint32 intrstatus;		/* Interrupt_status 0x048 */
	uint32 intrmaskwlan;		/* Interrupt_mask_wLAN 0x04c */
	uint32 intrmaskbt;		/* Interrupt_mask_bT    0x050 */
	uint32 mficount;			/* Mfi_count	   0x054 */

	uint32 PAD1[98];
	uint32 clkctlstatus;		/* Clk_ctrl_status	 0x01E0 */
	uint32 workaround;		/* Work_around    0x1E4 */
	uint32 PAD2[62];
	uint32 hsicphyctrl1;		/* HSICPhy_ctrl1	0x2e0 */
	uint32 hsicphyctrl2;		/* HSICPhy_ctrl1	0x2e4 */
	uint32 hsicphystat1;		/* HSICPhy_state	0x2e8 */
	uint32 PAD3[5];
	uint32 phybertctrl1;		/* phy_bert_ctrl1 0x300 */
	uint32 phybertctrl2;		/* phy_bert_ctrl2 0x304 */
	uint32 phybertstat1;		/* phy_bert_state1 0x308 */
	uint32 phybertstat2;		/* phy_bert_state2 0x30C */
	uint32 phyutmictl1;		/* phy_utmi_ctl1 0x310 */
	uint32 phytpctl1;		/* phy_tp_ctl1	  0x314 */
	uint32 usb30dgpiosel;			/* Usb30d_gPIOSel	 0x318 */
	uint32 usb30dgpiooe;			/* Usb30d_gPIOoe	 0x31C */
	uint32 usb30dmdioctl;			/* Usb30d_mdio_ctl	  0x320 */
	uint32 usb30dmdiorddata;	/* Usb30Mdio_rd_data 0x324 */
	uint32 phymiscctrl;		/* phy_misc_ctrl  0x328 */
	uint32 PAD4[1];
	uint32 xdcdebugsel1;			/* xdc_debug_sel1	  0x330 */
	uint32 usb30dgpioout;			/* Usb30d_gPIOout	 0x334 */
} usb_bdc_shim_regs_t;

struct usbdev_chip;

int bdci_init_hw(struct usbdev_chip *ubdc);
int bdci_init_mdio(struct usbdev_chip *ubdc);
int bdci_init_phy(struct usbdev_chip *ubdc);
void bdci2wlan_irq_on(struct usbdev_chip *ubdc);
void bdci2wlan_irq_off(struct usbdev_chip *ubdc);
void bdci_dev_reset(struct usbdev_chip *ubdc);
int bdci2wlan_isr(struct usbdev_chip *ubdc);

#endif /* _BDC_SHIM_H_ */
