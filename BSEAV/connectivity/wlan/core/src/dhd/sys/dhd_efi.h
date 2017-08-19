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
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: dhd_efi.h 465078 2015-12-03 00:56:24Z $
 */

#ifndef _DHD_EFI_H_
#define _DHD_EFI_H_

#include "sbchipc.h"

#if !defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION < 0x00020000)
#include "Tiano.h"
#include "EfiDriverLib.h"
#include "Pci22.h"
#include "EfiPrintLib.h"
#include "EfiStdArg.h"
#include "EfiPrintLib.h"
#include "LoadFile.h"
#include "SimpleFileSystem.h"
#include "DevicePath.h"
#include "FileInfo.h"

#else /* !(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION < 0x00020000) */

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>

#include <Protocol/LoadFile.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

#define EFI_STRINGIZE(a)            #a
#define EFI_PROTOCOL_DEFINITION(a)  EFI_STRINGIZE (Protocol/a.h)
#define EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL_REVISION_31 0x00010001
#define PXE_ROMID_MINORVER_31 0x10
#ifndef PXE_STATCODE_NO_MEDIA
#define PXE_STATCODE_NO_MEDIA 0x0014
#endif

#endif /* !EDK_RELEASE_VERSION || (EDK_RELEASE_VERSION < 0x00020000) */

#ifdef APPLE_BUILD
#include EFI_PROTOCOL_DEFINITION(Apple80211)
#include EFI_PROTOCOL_DEFINITION(AppleLinkStateProtocol)
#include EFI_PROTOCOL_DEFINITION(AppleAip)
#else
#include "apple80211.h"
#endif

#define WL_LOCK(wl, tpl) \
	do { \
	} while (0);
#define WL_UNLOCK(wl, tpl) \
	do { \
	} while (0);

#define DHD_LOCK_LEVEL_HIGH EFI_TPL_NOTIFY
#define DHD_LOCK_LEVEL_LOW EFI_TPL_CALLBACK

/* private tunables */
#define	NTXBUF	(1024)	/* # local transmit buffers */
#define	NRXBUF	(1024)	/* # local receive buffers */

#define PROGRAM_NAME "sid"
#define FW_PATH "\\WIRELESS\\rtecdc.bin"
#define NV_PATH "\\WIRELESS\\nvram.txt"
#define CLM_PATH "\\WIRELESS\\clm.blob"
#define WIRELESS_PATH "\\WIRELESS"
#define TIMESTAMP_PREFIX "\\[%2u-%2u-%2u_%2u,%2u,%2u.%u]Corecapture"
#define SOCRAM_LOG_DIR "\\StateSnapshots"
#define DRIVER_LOG_DIR "\\DriverLogs"
#define FW_LOG_DIR "\\FirmwareLogs"
#define SOCRAM_FILE_NAME "\\SoC_RAM.bin"
#define MMIO_FILE_NAME "\\MMIO.log"
#define FW_LOG_FILENAME "\\FwConsoleMsgEvents.log"
#ifdef SHOW_LOGTRACE
#define LOGSTRS_FILE_PATH "\\WIRELESS\\logstrs.bin"
#define ST_STR_FILE_PATH "\\WIRELESS\\rtecdc.bin"
#define MAPFILE_PATH "\\WIRELESS\\rtecdc.map"
#define ROM_ST_STR_FILE_PATH "\\WIRELESS\\roml.bin"
#define ROM_MAP_FILE_PATH "\\WIRELESS\\roml.map"
#define RAM_FILE_STR "rtecdc"
#define ROM_FILE_STR "roml"
#endif /* SHOW_LOGTRACE */

#define OTP_ADDRESS(sih) (SI_ENUM_BASE(sih) + CC_SROM_OTP)
#define OTP_USER_AREA_OFFSET 0x80
#define OTP_USER_AREA_ADDR(sih) (OTP_ADDRESS(sih) + OTP_USER_AREA_OFFSET)
#define OTP_VENDOR_TUPLE_ID 0x15
#define OTP_CIS_REGION_END_TUPLE_ID 0XFF
#define SPROM_CTRL_REG_ADDR(sih) (SI_ENUM_BASE(sih) + CC_SROM_CTRL)
#define OTP_CTRL1_REG_ADDR(sih) (SI_ENUM_BASE(sih) + 0xF4)
#define PMU_MINRESMASK_REG_ADDR(sih) (SI_ENUM_BASE(sih) + MINRESMASKREG)
#define SPROM_CHIP_STATUS(sih) (SI_ENUM_BASE(sih) + 0x2C)

#define INTR_POLL_PERIOD 1000	/* 100us, in units of 100ns */
#define WAIT_EVENT_STALL_PERIOD 10000 /* 10ms in units of us */
#define PATH_MAX 256
#define EFI_DHD_MS_NS_CONVERSION_UNIT	10000 /* 1ms, in units of 100ns */
#define BT_BOOT_SROM_READ_PERIOD	50000 /* 50ms in units of us */

/* The least significant 6-bytes will be the NIC MAC address */
#define BCMDHD_GUID \
	{0x21981f75, 0x0e3f, 0x4a04, 0x83, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }

#if defined(EFI_WINBLD)
#define BCMDHD_DRIVER_SIGNATURE EFI_SIGNATURE_32 ('4', '3', 'x', 'x')
#else /* EFI_WINBLD */
#define BCMDHD_DRIVER_SIGNATURE SIGNATURE_32 ('4', '3', 'x', 'x')
#endif /* EFI_WINBLD */

/* This is done to avoid having to organize blocks by #ifdef - #else - #endif */
#if !defined(BCMWLUNDI)
#error "BCMWLUNDI has to be defined"
#endif

#ifdef BCMWLUNDI
#if !defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION < 0x00020000)
#include "EfiPxe.h"
#include EFI_PROTOCOL_DEFINITION(EfiNetworkInterfaceIdentifier)
#else
#include <Protocol/NetworkInterfaceIdentifier.h>
#endif

#ifndef PXE_OPFLAGS_STATION_ADDRESS_WRITE
#define  PXE_OPFLAGS_STATION_ADDRESS_WRITE 0x0000
#endif

#endif /* BCMWLUNDI */

#ifdef EFI_WINBLD
#include EFI_PROTOCOL_DEFINITION(DevicePath)
#include EFI_PROTOCOL_DEFINITION(PciRootBridgeIo)
#include EFI_PROTOCOL_DEFINITION(PciIo)
#include EFI_PROTOCOL_DEFINITION(DriverBinding)
#include EFI_PROTOCOL_DEFINITION(LoadedImage)
#include EFI_PROTOCOL_DEFINITION(FileInfo)
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

int dhd_os_get_cur_priority_level();


#endif /* _DHD_EFI_H_ */
