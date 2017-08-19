/*
 * RSock User Mode Host Bus Implementation for libusb
 *
 * Sits below the rsock application library.
 * Provides attach, close, read and output functions.
 *
 * The Broadcom version of libusb has been modified to support
 * multiple outstanding reads and writes from different threads.
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

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/time.h>

#include "libusb/usb.h"
#include "os.h"
#include "../hostbus.h"
#include "hostbus_usb.h"

#define VERBOSE			0

#define DPRINTF			if (0) printf

#define DONGLE_VENDOR		0x0a5c
#define DONGLE_PRODUCT		0x0cdc

#define CDC_EP_DATA_IN		0x82
#define CDC_EP_DATA_OUT		0x03

#define READ_TIMEOUT		60000		/* msec */
#define WRITE_TIMEOUT		5000		/* msec */

static usb_dev_handle *udev;

void
hostbus_attach(void)
{
	struct usb_bus *bus;
	struct usb_device *dev = NULL;
	char buf[256];

	usb_init();

	usb_find_busses();
	usb_find_devices();

	for (bus = usb_busses; bus; bus = bus->next)
		for (dev = bus->devices; dev; dev = dev->next)
			if (dev->descriptor.idVendor == DONGLE_VENDOR &&
			    dev->descriptor.idProduct == DONGLE_PRODUCT)
				break;

	if (!dev) {
		printf("Device not found\n");
		exit(1);
	}

	if ((udev = usb_open(dev)) == NULL) {
		fprintf(stderr, "Could not open device (permission?)\n");
		exit(1);
	}

	if (dev->descriptor.iManufacturer) {
		if (usb_get_string_simple(udev, dev->descriptor.iManufacturer,
		                          buf, sizeof(buf)) <= 0)
			fprintf(stderr, "Unable to determine manufacturer\n");
		else if (VERBOSE)
			printf("Manufacturer: %s\n", buf);
	}

	if (dev->descriptor.iProduct) {
		if (usb_get_string_simple(udev, dev->descriptor.iProduct,
		                          buf, sizeof(buf)) <= 0)
			fprintf(stderr, "Unable to determine product\n");
		else if (VERBOSE)
			printf("Product: %s\n", buf);
	}

	if (dev->descriptor.iSerialNumber) {
		if (usb_get_string_simple(udev, dev->descriptor.iSerialNumber,
		                          buf, sizeof(buf)) <= 0)
			fprintf(stderr, "Unable to determine product\n");
		else if (VERBOSE)
			printf("Serial Number: %s\n", buf);
	}

	if (VERBOSE)
		printf("Number of configurations: %d\n", dev->descriptor.bNumConfigurations);

	if (VERBOSE)
		usb_set_debug(10);

	if (VERBOSE)
		printf("Claim interfaces\n");

	if (usb_claim_interface(udev, 0) < 0) {
		fprintf(stderr, "Could not claim interface 0\n");
		exit(1);
	}

	if (VERBOSE)
		printf("USB attached\n");
}

void
hostbus_close(void)
{
	if (udev) {
		usb_close(udev);
		udev = NULL;
	}
}

/*
 * Read a response, simulating SDIO packet transport.
 * Returns length in bytes.
 */

int
hostbus_read(void **resp_buf)
{
	int rv;
	char *buf;

	if ((buf = malloc(HOSTBUS_MTU)) == NULL) {
		fprintf(stderr,
		        "hostbus_read: out of memory (len=%d)\n", HOSTBUS_MTU);
		exit(1);
	}

	for (;;) {
		DPRINTF("hostbus_read: submit bulk read buf=%p max=%d\n",
		        (void *)buf, HOSTBUS_MTU);

		rv = usb_bulk_read(udev, CDC_EP_DATA_IN,
		                   buf, HOSTBUS_MTU,
		                   READ_TIMEOUT);

		DPRINTF("hostbus_read: result=%d\n", rv);

		if (rv == 0) {
			fprintf(stderr,
			        "hostbus_read: zero length read from USB\n");
			free(buf);
			exit(1);
		}

		if (rv > 0)
			break;

		if (errno != ETIMEDOUT && errno != EINTR) {
			fprintf(stderr,
			        "hostbus_read: usb_bulk_read failed: errno=%d (%s)\n",
			        errno, strerror(errno));
			free(buf);
			exit(1);
		}
	}

	*resp_buf = buf;

	DPRINTF("hostbus_read: len=%d {%02x%02x %02x%02x %02x%02x %02x%02x}\n",
	        rv,
	        ((uint8 *)buf)[0], ((uint8 *)buf)[1], ((uint8 *)buf)[2], ((uint8 *)buf)[3],
	        ((uint8 *)buf)[4], ((uint8 *)buf)[5], ((uint8 *)buf)[6], ((uint8 *)buf)[7]);

	return rv;
}

int
hostbus_output(void *msg, int msg_len)
{
	int rv;

	DPRINTF("hostbus_output: len=%d {%02x%02x %02x%02x %02x%02x %02x%02x}\n",
	        msg_len,
	        ((uint8 *)msg)[0], ((uint8 *)msg)[1], ((uint8 *)msg)[2], ((uint8 *)msg)[3],
	        ((uint8 *)msg)[4], ((uint8 *)msg)[5], ((uint8 *)msg)[6], ((uint8 *)msg)[7]);

	ASSERT(msg_len > 0);
	ASSERT(msg_len <= HOSTBUS_MTU);

	if ((rv = usb_bulk_write(udev, CDC_EP_DATA_OUT,
	                         msg, msg_len,
	                         WRITE_TIMEOUT)) < 0) {
		fprintf(stderr,
		        "usb_bulk_write failed: rv=%d errno=%d (%s)\n",
		        rv, errno, strerror(errno));
		exit(1);
	}

	return 0;
}
