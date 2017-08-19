/*
 * Broadcom Host Remote Download Utility
 *
 * MacOS USB OSL routines adapted from Xcode's USBSimple example.
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
#include <stdlib.h>
#include <mach/mach.h>
#include <CoreFoundation/CFNumber.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include "usbrdl.h"
#include "usb_osl.h"

/* 
 * Private data
 */
struct usbinfo {
	struct bcm_device_id *devtable;
	int bulkoutpipe;
	int ctrlinpipe;
	io_service_t usbDeviceRef;
	IOUSBDeviceInterface **usbDev;
	IOUSBInterfaceInterface **usbIntf;
};

/* 
 * Local interfaces
 */
static bool
find_my_device(usbinfo_t *info, struct bcm_device_id **bd)
{
	struct bcm_device_id *bcmdev;
	CFMutableDictionaryRef matchDict;
	SInt32 idVendor, idProduct;
	CFNumberRef nRef;
	io_service_t usbDevRef;

	/* Traverse list and find device */
	for (bcmdev = info->devtable; bcmdev->name; bcmdev++)
	{
		idProduct = bcmdev->prod;
		idVendor = bcmdev->vend;

		matchDict = IOServiceMatching(kIOUSBDeviceClassName);
		if (!matchDict)
			goto err;

		nRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &idVendor);
		if (!nRef)
			goto err;

		CFDictionaryAddValue(matchDict, CFSTR(kUSBVendorID), nRef);
		CFRelease(nRef);
		nRef = 0;

		nRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &idProduct);
		if (!nRef)
			goto err;

		CFDictionaryAddValue(matchDict, CFSTR(kUSBProductID), nRef);
		CFRelease(nRef);
		nRef = 0;

		usbDevRef = IOServiceGetMatchingService(kIOMasterPortDefault,
			matchDict); /* also releases matchDict */
		matchDict = 0;

		if (usbDevRef) {
			*bd = bcmdev; /* Found device */
			info->usbDeviceRef = usbDevRef;
			return TRUE;
		}
	}

err:

	return FALSE;
}

static void
usbdev_intf_open(usbinfo_t *info, io_service_t usbIntfRef)
{
	IOReturn err;
	IOCFPlugInInterface **iodev;
	IOUSBInterfaceInterface **intf;
	UInt8 dir, number, xferType, interval;
	UInt16 mps;
	int i;
	SInt32 score;
	UInt8  numEP;

	err = IOCreatePlugInInterfaceForService(usbIntfRef, kIOUSBInterfaceUserClientTypeID,
		kIOCFPlugInInterfaceID, &iodev, &score);
	if (err || !iodev)
		return;

	err = (*iodev)->QueryInterface(iodev, CFUUIDGetUUIDBytes(kIOUSBInterfaceInterfaceID),
		(LPVOID)&intf);
	IODestroyPlugInInterface(iodev);

	if (err || !intf)
		return;

	err = (*intf)->USBInterfaceOpen(intf);
	if (err)
		return;

	err = (*intf)->GetNumEndpoints(intf, &numEP);
	if (err)
		goto err;

	if (numEP) {
		for (i = 1; i <= numEP; i++) {

			err = (*intf)->GetPipeProperties(intf, i, &dir, &number,
				&xferType, &mps, &interval);
			if (err)
				goto err;

			if ((xferType == kUSBBulk) && (dir == kUSBOut)) {
				/* Found BULK OUT endpoint */
				info->bulkoutpipe = i;
			}
		}
	}

	info->usbIntf = intf;
	return;
err:
	(*intf)->USBInterfaceClose(intf);
	(*intf)->Release(intf);
	info->usbIntf = NULL;
}

static void
usbdev_intf_close(usbinfo_t *info)
{
	if (info->usbIntf) {
		(*(info->usbIntf))->USBInterfaceClose(info->usbIntf);
		(*(info->usbIntf))->Release(info->usbIntf);
	}
}

static void
usbdev_device_open(usbinfo_t *info)
{
	IOReturn err;
	IOCFPlugInInterface **iodev;
	IOUSBDeviceInterface **dev;
	SInt32 score;
	UInt8  numConf;
	IOUSBConfigurationDescriptorPtr confDesc;
	IOUSBFindInterfaceRequest intfReq;
	io_iterator_t iterator;
	io_service_t usbIntfRef;

	err = IOCreatePlugInInterfaceForService(info->usbDeviceRef, kIOUSBDeviceUserClientTypeID,
		kIOCFPlugInInterfaceID, &iodev, &score);
	if (err || !iodev)
		return;

	err = (*iodev)->QueryInterface(iodev, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID),
		(LPVOID)&dev);
	IODestroyPlugInInterface(iodev);

	if (err || !dev)
		return;

	err = (*dev)->USBDeviceOpen(dev);
	if (err)
		return;

	err = (*dev)->GetNumberOfConfigurations(dev, &numConf);
	if (err || !numConf)
		goto err;

	err = (*dev)->GetConfigurationDescriptorPtr(dev, 0, &confDesc);
	if (err)
		goto err;

	err = (*dev)->SetConfiguration(dev, confDesc->bConfigurationValue);
	if (err)
		goto err;

	intfReq.bInterfaceClass = kIOUSBFindInterfaceDontCare;
	intfReq.bInterfaceSubClass = kIOUSBFindInterfaceDontCare;
	intfReq.bInterfaceProtocol = kIOUSBFindInterfaceDontCare;
	intfReq.bAlternateSetting = kIOUSBFindInterfaceDontCare;

	err = (*dev)->CreateInterfaceIterator(dev, &intfReq, &iterator);
	if (err)
		goto err;

	while (usbIntfRef = IOIteratorNext(iterator)) {
		usbdev_intf_open(info, usbIntfRef);
		IOObjectRelease(usbIntfRef);
		break;
	}

	IOObjectRelease(iterator);
	iterator = 0;

	info->usbDev = dev;
	return;
err:
	(*dev)->USBDeviceClose(dev);
	(*dev)->Release(dev);
	info->usbDev = NULL;
}

static void
usbdev_device_close(usbinfo_t *info)
{
	if (info->usbDeviceRef)
		IOObjectRelease(info->usbDeviceRef);

	usbdev_intf_close(info);

	if (info->usbDev) {
		(*(info->usbDev))->USBDeviceClose(info->usbDev);
		(*(info->usbDev))->Release(info->usbDev);
	}
}

/* 
 * Public interfaces
 */
usbinfo_t *
usbdev_init(struct bcm_device_id *devtable, struct bcm_device_id **bcmdev)
{
	usbinfo_t *info;

	if ((info = (usbinfo_t *)malloc(sizeof(usbinfo_t))) == NULL)
		return NULL;

	bzero(info, sizeof(usbinfo_t));
	info->devtable = devtable;

	if (find_my_device(info, bcmdev) == FALSE) {
		printf("No device found\n");
		*bcmdev = NULL;
		goto err;
	}

	usbdev_device_open(info);
	return (info);
err:
	free(info);
	return NULL;
}

int
usbdev_deinit(usbinfo_t *info)
{
	usbdev_device_close(info);
	free(info);
	return 0;
}

int
usbdev_bulk_write(usbinfo_t *info, void *data, int len, int timeout)
{
	IOReturn kr;

	kr = (*(info->usbIntf))->WritePipe(info->usbIntf, info->bulkoutpipe, data, len);
	if (kr != kIOReturnSuccess)
		return -1;

	return 0;
}

int
usbdev_control_read(usbinfo_t *info, int request, int value, int index,
                    void *data, int size, bool interface, int timeout)
{
	IOUSBDevRequest req;
	IOReturn err;

	req.bmRequestType = USBmakebmRequestType(kUSBIn, kUSBVendor, kUSBInterface);
	req.bRequest = request;
	req.wValue = value;
	req.wIndex = index;
	req.wLength = size;
	req.pData = data;

	err = (*(info->usbIntf))->ControlRequest(info->usbIntf, 0, &req);
	if (kIOReturnSuccess != err)
		return -1;

	return 0;
}
