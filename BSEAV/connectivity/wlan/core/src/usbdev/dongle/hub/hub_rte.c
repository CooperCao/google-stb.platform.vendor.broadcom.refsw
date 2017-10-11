/*
 * USB device RTE interface
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#include <usb.h>
#include <usbdev.h>
#include <usbdev_dbg.h>
#include <osl.h>
#include <osl_ext.h>
#include <sbusbd.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <rte_isr.h>
#include <rte_dev.h>
#include <rte_cons.h>
#include <rte_trap.h>
#include <rte_ioctl.h>
#include <rte_gpio.h>
#include <usbhub.h>
#include <hub_reg.h>
#include <bdc.h>
#include <usbrdl.h>

#ifdef HUB_DEBUG
uint32 hub_msg_level = HUB_ERROR_VAL | HUB_DBG_VAL | HUB_TRACE_VAL;
#else
uint32 hub_msg_level = HUB_ERROR_VAL;
#endif

typedef struct {
	int unit;		/* Given by hnd_add_device(), always 0 */
	hnd_dev_t *rtedev;	/* Points usbdev_dev */
	void *ch;		/* Points struct usbhub_chip* as returned by hub_attach */
	osl_t *osh;		/* Points to osl_attach() called from usbdev_probe */
	volatile void *regs;	/* Points to regs for this device core, as obtained by SI utils */
	uint16 device;		/* Points to the device id given in hnd_add_device() */
	void *usb_drv;		/* Points to drv struct of usb controller */
} drv_t;

/* Driver entry points */
static void * usbhub_probe(hnd_dev_t *dev, volatile void *regs, uint bus, uint16 device,
		uint coreid, uint unit);
static int usbhub_open(hnd_dev_t *dev);
static int usbhub_close(hnd_dev_t *dev);
struct usbhub_chip * usbhub_attach(void *drv, uint vendor, uint device, osl_t *osh,
		volatile void *regs,
		uint bus);

static hnd_dev_ops_t usbhub_funcs = {
	probe:		usbhub_probe,
	open:		usbhub_open,
	close:		usbhub_close,
};

hnd_dev_t usbhub_dev = {
	name:		"usbhub",
	ops:		&usbhub_funcs,
};

/*
 * hub_bind() : To store the drv struct pointer for usb controller
 */
void
hub_bind(void *hub_drv, void *usb_drv)
{
	hub_trace(("\n %s", __FUNCTION__));
	drv_t *drv = (drv_t *)hub_drv;
	drv->usb_drv = usb_drv;
}

/*
 *  Open the USB HUB device.
 *  Prepare to receive and send packets.
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *
 *  Return value:
 *		status, 0 = ok
 */
static int
BCMATTACHFN(usbhub_open) (hnd_dev_t * dev)
{
/*	drv_t *drv = dev->softc; */
	hub_trace(("\n %s", __FUNCTION__));

	return 0;
}

/*
 *  Close the USB HUB device.
 *
 *  Input parameters:
 *		dev - device context (includes ptr to our softc)
 *
 *  Return value:
 *		status, 0 = ok
 */
static int
BCMATTACHFN(usbhub_close) (hnd_dev_t * dev)
{
/*	drv_t *drv = dev->softc; */
	hub_trace(("\n %s", __FUNCTION__));

	return 0;
}

/*
 *  Probe and install driver.
 *  Create a context structure and attach to the  specified network device.
 *
 *  Input parameters:
 *		dev - device descriptor
 *		regs - mapped registers
 *		device - device ID
 *		unit - unit number
 *
 *  Return value:
 *		nothing
 */
static void *
BCMATTACHFN(usbhub_probe) (hnd_dev_t * rtedev, volatile void * regs, uint bus, uint16 device,
		uint coreid, uint unit)
{
	drv_t *drv;
	osl_t *osh;

	hub_trace(("\n %s", __FUNCTION__));

	osh = osl_attach(rtedev);

	if (!(drv = (drv_t *)MALLOCZ(osh, sizeof(drv_t)))) {
		hub_err(("malloc failed in hub drv_t"));
		goto fail;
	}

	drv->unit = 0;
	drv->rtedev = rtedev;
	drv->device = device;
	drv->regs = regs;
	drv->osh = osh;

	if (!(drv->ch = usbhub_attach(drv, VENDOR_BROADCOM,
			drv->device, osh, drv->regs, bus))) {
		hub_err(("\n usbhub_attach failed"));
		goto fail;
	}

	return (void *)drv;
fail:
	return NULL;
}

/** Called on firmware initialization */
struct usbhub_chip *
BCMATTACHFN(usbhub_attach) (void * drv, uint vendor, uint device, osl_t * osh, volatile void * regs,
		uint bus)
{
	struct usbhub_chip *uhub;
	void *mem;
	char *vars;
	uint vars_len;

	hub_trace(("\n %s", __FUNCTION__));

	/* allocate usbdev_chip structure */
	if (!(mem = MALLOC_ALIGN(osh, sizeof(struct usbhub_chip), HUB_ALIGN_BITS))) {
		hub_err(("%s: out of memory, malloced %d bytes", __FUNCTION__, MALLOCED(osh)));
		ASSERT(0);
		return NULL;
	}

	uhub = (struct usbhub_chip *)mem;

	hub_dbg(("uhub = %p\n", mem));

	bzero(uhub, sizeof(struct usbhub_chip));
	uhub->drv = drv;
	uhub->osh = osh;

	/* Attach dci to bus */
	if (!(uhub->sih = si_attach(device, osh, regs, bus, NULL, &vars, &vars_len))) {
		hub_err(("\n usbhub_probe si_attach failed"));
		goto err;
	}

	uhub->id = si_coreid(uhub->sih);
	uhub->rev = si_corerev(uhub->sih);

	uhub->hub_shim_reg = (void *)(regs + 0x0);
	uhub->hub_core_reg = (void *)(regs + 0x800);

	hub_dbg(("Hub core rev = 0x%x, id = 0x%x \n", uhub->rev, uhub->id));
	hub_dbg(("Hub addr shim = 0x%p, core = 0x%p \n", uhub->hub_shim_reg, uhub->hub_core_reg));

	usbhub_read_otp(uhub);

	if (uhub->bypass_mode) {
	/*
	 * Todo , find the way to bypass mode through software
	 * http://confluence.broadcom.com/display
	 * /WLAN/4373+Toplevel+Architecture#id-4373ToplevelArchitecture-StrapPins
	 * Todo, find and configure the gpio to drive that pin.
	 */
	}

	usbhub_is_bypassed(uhub);

	if (uhub->vid || uhub->pid) {
		usbhub_set_newid(uhub);
	}

	uhub->non_disconnect = 0;

#ifdef BCMUSB_NODISCONNECT
	uhub->non_disconnect = 1;
#endif

	if (uhub->non_disconnect == 0) {
		/* usbhub_init(uhub); */
	}

	return ((void *)uhub);
err:
	MFREE(osh, uhub, sizeof(struct usbhub_chip));
	hub_err(("\n usbhub_attach failed"));

	return NULL;
}	/* usbhub_attach */

/*
 * usbhub_utmiphy_reset () :
 * This function will be called to issue a reset and initalize UTMI PHY
 */
void
usbhub_utmiphy_reset(void *hub_drv)
{
	uint32 value, delay;
	char *otp_str;

	hub_trace(("%s \n", __FUNCTION__));

	drv_t *drv = (drv_t *)hub_drv;
	struct usbhub_chip *uhub = drv->ch;

	value = HUB_RX_SYNC_DETECT_LEN << HUB_PHY_P1CTL_I_SHIFT;
	if ((otp_str = getvar(NULL, "hubphy_utmi_ctl0")) != NULL)
		value = (uint32)(bcm_strtoul(otp_str, NULL, 0));
	W_REG(uhub->osh, (uint32 *)(uhub->hub_shim_reg + UHUB_PHYUTMICTL1_adr), value);

	delay = 1;
	if ((otp_str = getvar(NULL, "hubphy_sleep0")) != NULL)
		delay = (uint32)(bcm_strtoul(otp_str, NULL, 0));
	OSL_DELAY(delay);

	hub_dbg(("Value read from UHUB_PHYUTMICTL1 (%p) is 0x%x \n",
		(uint32 *)(uhub->hub_shim_reg + UHUB_PHYUTMICTL1_adr),
		R_REG(uhub->osh, (uint32 *)(uhub->hub_shim_reg + UHUB_PHYUTMICTL1_adr))));

	value = (HUB_PHY_ISO | HUB_PHY_IDDQ |
		(HUB_RX_SYNC_DETECT_LEN << HUB_PHY_P1CTL_I_SHIFT));
	if ((otp_str = getvar(NULL, "hubphy_utmi_ctl1")) != NULL)
		value = (uint32)(bcm_strtoul(otp_str, NULL, 0));
	W_REG(uhub->osh, (uint32 *)(uhub->hub_shim_reg + UHUB_PHYUTMICTL1_adr), value);

	delay = 1;
	if ((otp_str = getvar(NULL, "hubphy_sleep1")) != NULL)
		delay = (uint32)(bcm_strtoul(otp_str, NULL, 0));
	OSL_DELAY(delay);

	hub_dbg(("Value read from UHUB_PHYUTMICTL1 (%p) is 0x%x \n",
		(uint32 *)(uhub->hub_shim_reg + UHUB_PHYUTMICTL1_adr),
		R_REG(uhub->osh, (uint32 *)(uhub->hub_shim_reg + UHUB_PHYUTMICTL1_adr))));

	value = (HUB_PLL_RESET_B | HUB_AFE_LDOCNTLEN_1P2 | HUB_AFE_LDO_PWRDWNB |
		HUB_PHY_ISO | HUB_PHY_IDDQ | (HUB_RX_SYNC_DETECT_LEN << HUB_PHY_P1CTL_I_SHIFT));
	if ((otp_str = getvar(NULL, "hubphy_utmi_ctl2")) != NULL)
		value = (uint32)(bcm_strtoul(otp_str, NULL, 0));
	W_REG(uhub->osh, (uint32 *)(uhub->hub_shim_reg + UHUB_PHYUTMICTL1_adr), value);

	delay = 1;
	if ((otp_str = getvar(NULL, "hubphy_sleep2")) != NULL)
		delay = (uint32)(bcm_strtoul(otp_str, NULL, 0));
	OSL_DELAY(delay);

	hub_dbg(("Value read from UHUB_PHYUTMICTL1 (%p) is 0x%x \n",
		(uint32 *)(uhub->hub_shim_reg + UHUB_PHYUTMICTL1_adr),
		R_REG(uhub->osh, (uint32 *)(uhub->hub_shim_reg + UHUB_PHYUTMICTL1_adr))));

	return;
}

/*
 * usbhub_soft_connect() :
 * To initiate a soft connect to the host after hub is configured
 */
void
usbhub_soft_connect(struct usbhub_chip *uhub)
{
/*
 *      0x1800C820  : USB HUB register:  USP_PORTSC (offset 0x20)
 *      Upstream Port's Port Status and Control Register
 *      Please refer Hub Spec:
 *
 *     http://hwnbu-twiki.sj.broadcom.com/twiki/pub/Mwgroup/CurrentUsbHubProgGuide/HSHUB_Spec.pdf
 *      Set bit 8: (SCN) Soft Connect =1
 *      S/w writes to this to initiate a soft connect.
 *      This is a one-time operation after power up.
 *      Once s/w writes to this, Hub will exit non-driving mode and connects to the host.
 */
	uint32 value;

	hub_trace(("\n %s", __FUNCTION__));

	value = R_REG(uhub->osh, (uint32 *)(uhub->hub_core_reg + UHUB_USP_PORTSC_adr));
	value |= HUB_SOFTCONNECT;
	W_REG(uhub->osh, (uint32 *)(uhub->hub_core_reg + UHUB_USP_PORTSC_adr), value);

	return;
}

/*
 * usbhub_init() : To initialize the hub core
 */
void
usbhub_init(void *hub_drv)
{
/*
 *      Address: 0x1800C000   (USB HUB register)
 *      Clear bit 0 to de-assert HUB_Soft_Reset
 *      Set bit 1 for main clock enable
 *      Set bit 2 for Aux clock enable
 *      clear bit 12 to de-assert phy_reset
 *      clear bit 26  to de-assert utmiSoftRst
 *      Retain values of other bits in this register to their default values
 */
	uint32 value;

	hub_trace(("\n %s", __FUNCTION__));

	drv_t *drv = (drv_t *)hub_drv;
	struct usbhub_chip *uhub = (struct usbhub_chip *)drv->ch;

	value = R_REG(uhub->osh, (uint32 *)(uhub->hub_shim_reg + UHUB_CTRL_adr));
	value &= ~(HUB_SOFT_RESET);
	value |= (HUB_MAINCLK_EN | HUB_AUXCLK_EN);
	value &= ~(HUB_PHY_RST);
	value &= ~(HUB_UTMI_SOFTRST);

	hub_dbg(("%p = %x \n", (uint32 *)(uhub->hub_shim_reg + UHUB_CTRL_adr), value));
	W_REG(uhub->osh, (uint32 *)(uhub->hub_shim_reg + UHUB_CTRL_adr), value);

	return;
}

/*
 * usbhub_read_otp(): Read the OTP entries related to Hub
 */
void
usbhub_read_otp(struct usbhub_chip *uhub)
{
	char *otp_str;
	uint32 value;

	hub_trace(("\n %s", __FUNCTION__));

	if ((otp_str = getvar(NULL, "usbhub")) != NULL) {
		value = bcm_strtoul(otp_str, NULL, 0);
		uhub->bypass_mode = (uint8)(value & HUB_OTP_VAR_BYPASS_MASK);
		hub_dbg(("hub bypass_mode = %d \n", uhub->bypass_mode));
	}
	if ((otp_str = getvar(NULL, "usbhubid")) != NULL) {
		value = bcm_strtoul(otp_str, NULL, 0);
		uhub->pid = (uint16)(value & HUB_OTP_VAR_PID_MASK);
		uhub->vid = (uint16)(value & HUB_OTP_VAR_VID_MASK) >> 16;
		hub_dbg(("hub pid 0x%x vid 0x%x \n", uhub->pid, uhub->vid));
	}
	return;
}

/*
 * usbhub_set_newid():
 * Called to configure the with new pid/vid
 */
void
usbhub_set_newid(struct usbhub_chip *uhub)
{
	uint32 value;

	hub_trace(("\n %s", __FUNCTION__));

	if (uhub->pid) {
		value = R_REG(uhub->osh, (uint32 *)(uhub->hub_core_reg + UHUB_ENUM2_adr));
		value &= ~HUB_PRODUCTID_MASK;
		value |= (uhub->pid << HUB_PRODUCTID_SHIFT);
		W_REG(uhub->osh, (uint32 *)(uhub->hub_core_reg + UHUB_ENUM2_adr), value);
	}
	if (uhub->vid) {
		value = R_REG(uhub->osh, (uint32 *)(uhub->hub_core_reg + UHUB_ENUM2_adr));
		value &= ~HUB_VENDORID_MASK;
		value |= (uhub->vid << HUB_VENDORID_SHIFT);
		W_REG(uhub->osh, (uint32 *)(uhub->hub_core_reg + UHUB_ENUM2_adr), value);
	}
	return;
}

/*
 * usbhub_is_bypassed():
 * Called to check whether the hub is by passed or not
 */
void
usbhub_is_bypassed(struct usbhub_chip *uhub)
{
	uint32 value;

	hub_trace(("\n %s", __FUNCTION__));

	value = R_REG(uhub->osh, (uint32 *)(uhub->hub_shim_reg + UHUB_STATUS_adr));

	if (value & HUB_BYPASSED) {
		uhub->isbypassed = 1;
	}
	return;
}
