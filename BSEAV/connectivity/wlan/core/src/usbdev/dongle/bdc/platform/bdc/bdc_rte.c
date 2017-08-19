/*
 *  Defines BDC RTE integration functions
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
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmstdlib.h>

#include <bdc.h>
#include <wlrdl.h>
#include <bdc_rte.h>
#include <bdc_shim.h>
#include <usbdevice.h>
#include <usbclass.h>

#ifdef BDC_DEBUG
uint32 bdc_msg_level = BDC_ERROR_VAL | BDC_DBG_VAL | BDC_TRACE_VAL | BDC_DBG_FUNCTION;
#else
uint32 bdc_msg_level = BDC_ERROR_VAL;
#endif

bool
bdci_hsic(struct usbdev_chip *chip)
{
	return FALSE;
}

si_t *
bdci_get_sih(struct usbdev_chip *ch)
{
	return (ch->sih);
}

/** Called on firmware initialization */
struct usbdev_chip *
BCMATTACHFN(bdci_attach) (void * drv, uint vendor, uint device, osl_t * osh, volatile void * regs,
		uint bus)
{
	struct usbdev_chip *ubdc;
	usb_class_t *usb_cls;
	usb_device_t *usb_device;
	char *vars;
	uint32 vars_len;

	bdc_dbg(("bdci_attach regs %p\n", regs));

	/* allocate usbdev_chip structure */
	/* TODO: CHECK SIZE OF UBDC_ALIGN_BITS */
	if (!(ubdc = (struct usbdev_chip *)MALLOC_ALIGN(osh,
		sizeof(*ubdc), UBDC_ALIGN_BITS))) {
		bdc_err(("%s: out of memory, malloced %d bytes", __FUNCTION__, MALLOCED(osh)));
		ASSERT(0);
		return NULL;
	}

	bdc_dbg(("ubdc = %p\n", ubdc));

	bzero(ubdc, sizeof(struct usbdev_chip));
	ubdc->drv = drv;
	ubdc->osh = osh;
	ubdc->up = FALSE;

	/* allocate bdc structure */
	/* TODO: CHECK SIZE OF BDC_ALIGN_BITS */
	if (!(ubdc->bdc = MALLOC_ALIGN(osh, sizeof(*(ubdc->bdc)), BDC_ALIGN_BITS))) {
		bdc_err(("%s: out of memory, malloced %d bytes", __FUNCTION__, MALLOCED(osh)));
		MFREE(osh, ubdc, sizeof(*ubdc));
		ASSERT(0);

		return NULL;
	}
	memset(ubdc->bdc, 0, sizeof(struct _bdcd));
	bdc_adapter_init();
	bdc_init(ubdc->bdc);

	ubdc->bdc->osh = osh;
	ubdc->bdc->ubdc = ubdc;


	/* allocate usb_device_t structure */
	/* TODO: CHECK SIZE OF BDC_ALIGN_BITS */
	if (!(usb_device = MALLOC_ALIGN(osh, sizeof(*usb_device), BDC_ALIGN_BITS))) {
		bdc_err(("%s: out of memory, malloced %d bytes", __FUNCTION__, MALLOCED(osh)));
		MFREE(osh, ubdc->bdc, sizeof(*(ubdc->bdc)));
		MFREE(osh, ubdc, sizeof(*ubdc));
		ASSERT(0);
		return NULL;
	}

	/* Attach dci to bus */
	if (!(ubdc->sih = si_attach(device, osh, regs, bus, NULL, &vars, &vars_len)))
		goto err;

	ubdc->id = si_coreid(ubdc->sih);
	ubdc->rev = si_corerev(ubdc->sih);
	bdc_dbg(("core rev = %x, core id = %x \n", ubdc->rev, ubdc->id));

	ubdc->bdc->bdc_shim_regs = (usb_bdc_shim_regs_t *)regs;
	ubdc->bdc->bdc_base_reg = si_addrspace(ubdc->sih, CORE_SLAVE_PORT_1, CORE_BASE_ADDR_0);

	bdc_dbg(("bdc_shim_regs = %p, bdc_base_reg = %x \n",
		ubdc->bdc->bdc_shim_regs, ubdc->bdc->bdc_base_reg));

	usbdevice_init(usb_device, (usb_controller *)ubdc->bdc);

	usb_cls = (usb_class_t *)usb_wl_create(usb_device);
	ubdc->u_class = usb_cls;

	/* Set USB is not yet detected */
	ubdc->bdc->attribute &= ~USBCONTROLLER_ATTRIBUTE_TRAN_DETECT;

	ubdc->non_disconnect = 0;

#ifdef BCMUSB_NODISCONNECT
	ubdc->non_disconnect = 1;
#endif

	return ubdc;

err:
	if (usb_device)
		MFREE(osh, usb_device, (sizeof(*usb_device)));
	if (ubdc->bdc)
		MFREE(osh, ubdc->bdc, sizeof(*(ubdc->bdc)));
	if (ubdc)
		MFREE(osh, ubdc, sizeof(*(ubdc)));

	ubdc->up = FALSE;

	return NULL;
}	/* xdci_attach */

void
BCMATTACHFN(bdci_detach) (struct usbdev_chip * ubdc, bool disable)
{
	/* uint32 devcontrl; */
	bdcd *bdc = ubdc->bdc;

	/* This function deletes all the EP created so far except EP0 */
	ubdc->u_class->ft->bus_reset(ubdc->u_class);

	bdci2wlan_irq_off(ubdc);

	if (ubdc->non_disconnect == 0) {
		/* Put the core back into reset */
		if (disable)
			si_core_disable(ubdc->sih, 0);

		/* Detach from SB bus */
		si_detach(ubdc->sih);

		/* Free chip state */
		bdc_disconnect_from_host((usb_controller *)bdc);

		bdc_power_off(ubdc->bdc);
		bdci_dev_reset(ubdc);
	} else {
		bdc_bus_reset((usb_controller *)bdc);
		/*
		 * todo: do we need stop also?
		 * bdc_stop_controller((usb_controller*)ubdc->bdc);
		 */
		ubdc->u_class->ft->stop(ubdc->u_class);

	}

	MFREE(ubdc->osh, ubdc->u_class->device, (sizeof(*ubdc->u_class->device)));
	MFREE(ubdc->osh, bdc, sizeof(*bdc));
	MFREE(ubdc->osh, ubdc, sizeof(*ubdc));

	bdc_trace(("bdci_detach done"));
}

/** Called when the BDC core signals an interrupt */
int
bdci_dpc(struct usbdev_chip *ch)
{
	bdc_interrupt_handler((usb_controller *)ch->bdc);
	return 0;
}

uint32
BCMATTACHFN(bdci_init) (struct usbdev_chip * ch, bool disconnect)
{
	usb_class_t *u_class = ch->u_class;

	if (ch->non_disconnect == 0) {
		bdci_init_hw(ch);
		OSL_DELAY(1000);
	}

	/* This calls bdc_start() */
	u_class->ft->start(ch->u_class);
	ch->up = TRUE;

	return 0;
}

int
bdci_tx(struct usbdev_chip *ch)
{
	usb_class_t *u_class = ch->u_class;

	/* Check if we have data to send to host */
	while (u_class) {
		u_class->ft->send_next_transfer(u_class);
		u_class = u_class->next;
	}

	return 0;
}
