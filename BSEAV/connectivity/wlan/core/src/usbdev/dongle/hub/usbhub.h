/*
 * USB device(target) driver API
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

#ifndef	_usbhub_h_
#define	_usbhub_h_

#include <typedefs.h>
#include <usbstd.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>

extern uint32 hub_msg_level;	/**< Print messages even for non-debug drivers */

#define HUB_ERROR_VAL	0x0001
#define HUB_TRACE_VAL	0x0002
#define HUB_DBG_VAL		0x0004
#define HUB_DBG_LTSSM	0x0008

#ifdef bBCMDBG
#define hub_err(args)	do {if (hub_msg_level & HUB_ERROR_VAL) printf args;} while (0)
#define HUB_DEBUG
#define hub_trace(args)	do {if (hub_msg_level & HUB_TRACE_VAL) printf args;} while (0)
#define hub_dbg(args)	do {if (hub_msg_level & HUB_DBG_VAL) printf args;} while (0)
#else
#undef HUB_DEBUG
#define hub_err(args)	do {if (hub_msg_level & HUB_ERROR_VAL) printf args;} while (0)
#define hub_trace(args)
#define hub_dbg(args)
#endif	/* bBCMDBG */

#define HUB_ALIGN_BITS 4
#define HUB_OTP_VAR_BYPASS_MASK		0x01  /**< Bits 0 for bypass */
#define HUB_OTP_VAR_PID_MASK		0x0000FFFF	/** <Bits 0-15 for hub pid */
#define HUB_OTP_VAR_VID_MASK		0xFFFF0000	/** <Bits 16-31 for hub vid */

struct usbhub_chip {
	void *drv;			/**< Driver handle */
	uint id;			/**< USB device core ID */
	uint rev;			/**< USB device core revision */
	osl_t *osh;			/**< os (Driver PCI) handle */
	si_t *sih;			/**< SiliconBackplane handle */
	struct dngl_bus *bus;		/**< high level USB bus logic handle */
	uint8 non_disconnect;	/**< not re-enumerate after FW download */
	uint8 bypass_mode;		/**< Hub is bypassed mode requested */
	uint8 isbypassed;		/**< Hub bypassed through gpio */
	uint16 vid;			/**< Hub vendor id */
	uint16 pid;			/**< Hub Product id */
	volatile void *hub_shim_reg;
	volatile void *hub_core_reg;
};

void usbhub_soft_connect(struct usbhub_chip *uhub);
void usbhub_init(void *drv);
void usbhub_is_bypassed(struct usbhub_chip *uhub);
void usbhub_set_newid(struct usbhub_chip *uhub);
void usbhub_read_otp(struct usbhub_chip *uhub);
void usbhub_utmiphy_reset(void *drv);
void hub_bind(void *hub_drv, void *usb_drv);
#endif	/* _usbhub_h_ */
