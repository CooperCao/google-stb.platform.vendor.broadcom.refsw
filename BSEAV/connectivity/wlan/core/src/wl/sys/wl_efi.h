/*
 * wl_efi.c exported functions and definitions
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

#ifndef _WL_EFI_H_
#define _WL_EFI_H_

#if defined(EFI_WINBLD)
#include "Tiano.h"
#include "EfiDriverLib.h"
#include "Pci22.h"
#include "EfiPrintLib.h"
#include "EfiStdArg.h"
#include "EfiPrintLib.h"

#else /* EFI_WINBLD */
#include <Base.h>
#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Guid/EventGroup.h>

#define EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL_REVISION_31 0x00010001
#define PXE_ROMID_MINORVER_31 0x10

#ifndef PXE_STATCODE_NO_MEDIA
#define PXE_STATCODE_NO_MEDIA 0x0014
#endif
#endif /* EFI_WINBLD */

#include "brcmefi.h"

#ifdef OLYMPIC_API
#ifdef APPLE_BUILD
#include EFI_PROTOCOL_DEFINITION(Apple80211)
#include EFI_PROTOCOL_DEFINITION(AppleLinkStateProtocol)
#include EFI_PROTOCOL_DEFINITION(AppleAip)
#else
#include "apple80211.h"
#endif /* APPLE_BUILD */
#endif /* OLYMPIC_API */

#if defined(EFI_WINBLD)
#define BCMWL_DRIVER_SIGNATURE EFI_SIGNATURE_32 ('4', '3', 'x', 'x')
#else /* EFI_WINBLD */
#define BCMWL_DRIVER_SIGNATURE SIGNATURE_32 ('4', '3', 'x', 'x')
#endif /* EFI_WINBLD */

#define WLC_CONNECTED(wlc) (wlc_bss_connected((wlc)->cfg) && wlc_portopen((wlc)->cfg))

/* This is done to avoid having to organize blocks by #ifdef - #else - #endif */
#if !defined(BCMWLUNDI)
#error "BCMWLUNDI has to be defined"
#endif

#ifdef BCMWLUNDI
#if defined(EFI_WINBLD)
#include "EfiPxe.h"
#endif /* EFI_WINBLD */

#ifndef PXE_OPFLAGS_STATION_ADDRESS_WRITE
#define  PXE_OPFLAGS_STATION_ADDRESS_WRITE 0x0000
#endif

#if defined(EFI_WINBLD)
#include EFI_PROTOCOL_DEFINITION(EfiNetworkInterfaceIdentifier)
#else /* EFI_WINBLD */
#include <Protocol/NetworkInterfaceIdentifier.h>
#endif /* EFI_WINBLD */

#define BCMWL_UNDI_NOT_REQUIRED        0xffff
#define BCMWL_UNDI_SIZE_VARY           0xfffe
#define BCMWL_UNDI_AT_LEAST_STARTED     1
#define BCMWL_UNDI_AT_LEAST_INITIALIZED 2
#define NII_GUID gEfiNetworkInterfaceIdentifierProtocolGuid_31

typedef struct UNDI_API_ENTRY
{
	UINT16 cpbsize;
	UINT16 dbsize;
	UINT16 opflags;
	UINT16 state;
	void (*handler_fn)();
	const char *name;
} UNDI_API_ENTRY, *pUNDI_API_ENTRY;
#endif /* BCMWLUNDI */


#if defined(EFI_WINBLD)
#include EFI_PROTOCOL_DEFINITION(DevicePath)
#include EFI_PROTOCOL_DEFINITION(PciRootBridgeIo)
#include EFI_PROTOCOL_DEFINITION(PciIo)
#include EFI_PROTOCOL_DEFINITION(DriverBinding)
#include EFI_PROTOCOL_DEFINITION(LoadedImage)
#else /* EFI_WINBLD */
#include <Protocol/DevicePath.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciIo.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/ComponentName.h>
#endif /* EFI_WINBLD */

/*  bit fields for the command */
#define PCI_COMMAND_MASTER  0x04    /*  bit 2 */
#define PCI_COMMAND	0x04

/* private tunables */
#define	NTXBUF	(256)	/* # local transmit buffers */
#define	NRXBUF	(256)	/* # local receive buffers */

#define PCI_VENDOR(vid) ((vid) & 0xFFFF)
#define PCI_DEVID(vid) ((vid) >> 16)

#endif /* _WL_EFI_H_ */
