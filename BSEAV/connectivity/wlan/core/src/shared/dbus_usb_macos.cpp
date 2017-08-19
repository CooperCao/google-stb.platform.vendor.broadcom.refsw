/*
 * Dongle BUS interface
 * USB Mac OS X Implementation
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include <IOKit/usb/IOUSBDevice.h>

extern "C"
{
#include "epivers.h"
#include "dbus.h"


typedef struct {
	dbus_pub_t *pub;
	void *cbarg;
	dbus_intf_callbacks_t *cbs;
} usbos_info_t;


/* Shared Function prototypes */
bool dbus_usbos_dl_cmd(usbos_info_t *usbinfo, uint8 cmd, void *buffer, int buflen);
int dbus_usbos_wait(usbos_info_t *usbinfo, uint16 ms);
int dbus_write_membytes(usbos_info_t *usbinfo, bool set, uint32 address, uint8 *data, uint size);

bool dbus_usbos_dl_send_bulk(usbos_info_t *usbinfo, void *buffer, int len);

} /* extern "C" */


extern "C"
{

int
dbus_bus_osl_register(int vid, int pid, probe_cb_t prcb,
	disconnect_cb_t discb, void *prarg, dbus_intf_t **intf, void *param1, void *param2)
{
	return DBUS_ERR_NODEVICE;
}

int
dbus_bus_osl_deregister()
{
	return DBUS_OK;
}

bool
dbus_usbos_dl_cmd(usbos_info_t *usbinfo, uint8 cmd, void *buffer, int buflen)
{
	return FALSE;
}

int
dbus_write_membytes(usbos_info_t* usbinfo, bool set, uint32 address, uint8 *data, uint size)
{

	DBUSTRACE(("Enter:%s\n", __FUNCTION__));
	return -1;
}
int
dbus_usbos_wait(usbos_info_t *usbinfo, uint16 ms)
{
	return DBUS_OK;
}

bool
dbus_usbos_dl_send_bulk(usbos_info_t *usbinfo, void *buffer, int len)
{
	return FALSE;
}

} /* extern C */
