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

#include <typedefs.h>
#include <bcmdefs.h>
#include <sbchipc.h>
#include <hndsoc.h>
#include <siutils.h>
#include <osl.h>
#include <bdc_shim.h>
#include <bdc_rte.h>
#include <sbusbd.h>
#ifdef USB_HUB
#include <usbhub.h>
#endif /* USB_HUB */

int
bdci_init_mdio(struct usbdev_chip *ubdc)
{
	int ret = 0;
	usb_bdc_shim_regs_t *bdc_shim_regs = ubdc->bdc->bdc_shim_regs;

	bdc_fn(("%s\n", __FUNCTION__));

	bdc_dbg(("Address &bdc_shim_regs->usb30dmdioctl %p\n", &bdc_shim_regs->usb30dmdioctl));

	W_REG(ubdc->osh, &bdc_shim_regs->usb30dmdioctl, 0x15);
	OSL_DELAY(1);

	bdc_dbg(("r %p=%x \n", &bdc_shim_regs->usb30dmdioctl,
			R_REG(ubdc->osh, &bdc_shim_regs->usb30dmdioctl)));

	return ret;
}

int
bdci_init_hw(struct usbdev_chip *ubdc)
{
	int ret = 0;
	uint32 rdval, orig_coreid;
	usb_bdc_shim_regs_t *bdc_shim_regs = ubdc->bdc->bdc_shim_regs;
#ifdef USB_HUB
	void *hub_drv;
#endif /* USB_HUB */

	bdc_fn(("%s\n", __FUNCTION__));
	bdc_dbg(("cc status %x\n", ubdc->sih->chipst));

	orig_coreid = si_coreid(ubdc->sih);
	si_setcore(ubdc->sih, USB30D_CORE_ID, 0);


	/* w 0x18106800.b[0] = 1 to reset */
	si_wrapperreg(ubdc->sih, AI_RESETCTRL, ~0, AIRC_RESET);
	OSL_DELAY(1);

	/* USB30d DMP Register programming */
	/* w 0x18103408.b[0,2] = 0x07 to enable clk, rd back value */
	rdval = si_wrapperreg(ubdc->sih, AI_IOCTRL, ~0,
			(CCTRL_SPEED_HS | CCTRL_BUSPWR | CCTRL_NDR_EN |
			CCTRL_AUXCLK_EN | CCTRL_FORCE_GTCLK_ON | CCTRL_MAINCLK_EN));
	bdc_dbg(("18103408 = %x \n", rdval));
	OSL_DELAY(1);

	/* w 0x18103800.b[0] = 0 to clear reset */
	si_wrapperreg(ubdc->sih, AI_RESETCTRL, ~0, 0x00);
	OSL_DELAY(1);

	/* w 0x18106408.b[0,2] = 0x05 to clear Force_gate_clk_on, rd back value */
	rdval = si_wrapperreg(ubdc->sih, AI_IOCTRL, ~0,
			(CCTRL_SPEED_HS | CCTRL_BUSPWR | CCTRL_NDR_EN |
			CCTRL_AUXCLK_EN | CCTRL_MAINCLK_EN));
	bdc_dbg(("18103408 = %x \n", rdval));
	OSL_DELAY(1);


	si_setcoreidx(ubdc->sih, orig_coreid);

#ifdef USB_HUB
	hub_drv = rdl_get_hub_drv(ubdc->drv);
	usbhub_utmiphy_reset(hub_drv);
	usbhub_init(hub_drv);
#endif /* USB_HUB */

	//bdci_init_mdio(ubdc);

	/* Deassert XDC reset bit 0x18007000.b[0] Dev_reset = 0 */
	rdval = R_REG(ubdc->osh, &(bdc_shim_regs->devcontrol));

	bdc_dbg(("r Dev_reset = %x\n", rdval));

	/* Clear bits 0,2, 10 to 12,  26, to bring BDC out of reset
		bit 0 : dev_reset = 0
		bit 2 : aux_rst_b = 0
		bit 10: utmi_pwr_dwn = 0
		bit 11: ana_pwr_dwn = 0
		bit 12 : phy_rst  = 0
		bit 26 : utmi_soft_rst = 0
	*/

	if (rdval & DEV_CTRL_DEVRESET) {
		rdval &= ~DEV_CTRL_DEVRESET;
		rdval &= ~DEV_CTRL_USB_AUX_RST_B;
		rdval &= ~DEV_CTRL_UTMIPWRDWN;
		rdval &= ~DEV_CTRL_ANAPWRDWN;
		rdval &= ~DEV_CTRL_PHYRESET;
		rdval &= ~DEV_CTRL_UTMISOFTRST;

		W_REG(ubdc->osh, &bdc_shim_regs->devcontrol, rdval);
		bdc_dbg(("w Dev_reset %p = %x\n", &bdc_shim_regs->devcontrol, rdval));
	}
	OSL_DELAY(1);

	rdval = R_REG(ubdc->osh, &bdc_shim_regs->devcontrol);
	bdc_dbg(("r Dev_reset = %x\n", rdval));

	return ret;
}	/* xdc_init_phy */

void
bdci_dev_reset(struct usbdev_chip *ubdc)
{
	uint32 value;
	usb_bdc_shim_regs_t *bdc_shim_regs = ubdc->bdc->bdc_shim_regs;

	bdc_fn(("%s\n", __FUNCTION__));

	/* Assert Dev Reset */
	value = R_REG(ubdc->osh, &bdc_shim_regs->devcontrol);

	if (value & DEV_CTRL_DEVRESET) {
		value |= B_0;
		W_REG(ubdc->osh, &bdc_shim_regs->devcontrol, value);

		OSL_DELAY(400);
	}
}

/** This route is ISR for USB interrupt to WLAN ARM */
int
bdci2wlan_isr(struct usbdev_chip *ubdc)
{
	uint32 usbintstatus;
	uint32 usbintwlanmask = 0x0F;	/* b[0, 3] is core interrpt for rings */
	usb_bdc_shim_regs_t *bdc_shim_regs = ubdc->bdc->bdc_shim_regs;

	usbintstatus = R_REG(ubdc->osh, &bdc_shim_regs->intrstatus);

	if (!(usbintstatus & usbintwlanmask))
		return FALSE;

	W_REG(ubdc->osh, &bdc_shim_regs->intrstatus, usbintstatus);

	bdc_trace(("bdci2wlan_isr = %x\n", usbintstatus));

	if (!ubdc->up) {
		return FALSE;
	}

	return TRUE;
}	/* xdci2wlan_isr */

void
bdci2wlan_irq_on(struct usbdev_chip *ubdc)
{
	uint32 value;
	usb_bdc_shim_regs_t *bdc_shim_regs = ubdc->bdc->bdc_shim_regs;

	bdc_fn(("%s\n", __FUNCTION__));

	value = R_REG(ubdc->osh, &bdc_shim_regs->intrmaskwlan);
	/* b[0, 3] is core interrpt for rings */
	value |= 0x0F;

	W_REG(ubdc->osh, &bdc_shim_regs->intrmaskwlan, value);
}

void
bdci2wlan_irq_off(struct usbdev_chip *ubdc)
{
	uint32 value;
	usb_bdc_shim_regs_t *bdc_shim_regs = ubdc->bdc->bdc_shim_regs;

	bdc_fn(("%s\n", __FUNCTION__));

	value = R_REG(ubdc->osh, &bdc_shim_regs->intrmaskwlan);
	value &= ~0x0F;	/* b[0, 3] is core interrpt for rings */

	W_REG(ubdc->osh, &bdc_shim_regs->intrmaskwlan, value);
}
