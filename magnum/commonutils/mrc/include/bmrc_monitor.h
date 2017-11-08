/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * Implementation of the Realtime Memory Monitor for 7038
 *
 ***************************************************************************/
#ifndef BMRC_MONITOR_H_
#define BMRC_MONITOR_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "bmrc.h"
#include "bint.h"
#include "bmma_types.h"

/***************************************************************************
Description:
This structure is used to describe interface used to monitor memory  allocations and deallocations in a heap. This interface provides
alloc and free hooks which will be called after when block is allocated or freed from the heap.
Single interface could be used to monitor several heaps.

****************************************************************************/
typedef struct BMRC_MonitorInterface
{
	void *cnxt; /*  User specified context */
	void (*alloc)(void *cnxt, BSTD_DeviceOffset addr, size_t size, const char *fname, int line); /* callback function called when new block was allocated */
	void (*free)(void *cnxt, BSTD_DeviceOffset addr); /* callback function called when block was deallocated */
} BMRC_MonitorInterface;


/*=Module Overview: ********************************************************
BMRC_Monitor provides a way to monitor which hardware clients access certain blocks of memory.
BMRC_Monitor uses the Address Range Checker (ARC) hardware which is a feature of the memory controller (MEMC).
It does this via the BMRC module.

This module easily integrates with magnum's memory manager (BMEM), which automatically configures
clients based on BMEM_Alloc allocations, using the debug tag filename to associate a memory block
with a certain set of MEMC clients.

Because there are limited ARC's on each chip, BMRC_Monitor performs a combine operation on all BMRC_Monitor
regions and clients. The end result is that multiple blocks and hardware clients may be combined into a
single region. This limits the accuracy of the check. Not every bad memory access can be caught.

To print the final BMRC_Monitor configuration, enable DBG's BDBG_eMsg Level for the BMRC_Monitor module and
call BMRC_Monitor_JailUpdate.  For Settop API, set msg_modules=mem_monitor.

Example:
This shows the default configuration for BMRC_Monitor using the automatic configuration
from BMEM. Error checking code is ommited for the sake of simplicity.

void main(void)
{
	BMRC_Monitor_Handle hMonitor;
	BMEM_Handle heap;
	BREG_Handle reg;
	BINT_Handle isr;
	BCHP_Handle chp;
	BMRC_Handle mrc;
	BERR_Code rc;
	static BMEM_MonitorInterface interface;

	.... Initialize MemoryManager, RegisterInterface, InterruptInterface, ChipInterface, and MemoryRangeChecker ...

	rc = BMRC_Monitor_Open(&hMonitor, reg, isr, chp, mrc, 0, 256 * 1024 * 1024); ... Control all 256MBytes window ...
	rc = BMRC_Monitor_GetMemoryInterface(hMonitor, &interface);
	rc = BMEM_InstallMonitor(heap, &interface);
}

This shows the use of a jail for debugging a specific client:
	BMRC_Monitor_Handle hMonitor;
	BMEM_Handle heap;
	BREG_Handle reg;
	BINT_Handle isr;
	BCHP_Handle chp;
	BERR_Code rc;
	static BMEM_MonitorInterface interface;

	.... Initialize MemoryManager, RegisterInterface, InterruptInterface and ChipInterface ...

	rc = BMRC_Monitor_Open(&hMonitor, reg, isr, chp, mrc, 0, 256 * 1024 * 1024); ... Control all 256MBytes window ...
	rc = BMRC_Monitor_GetMemoryInterface(hMonitor, &interface);
	rc = BMEM_InstallMonitor(heap, &interface);
	... Performing AVD debug, so optimize range checking for AVD ...
	BMRC_Monitor_JailAdd(hMonitor, BRMM_CLIENT_AVD_BLK_0);
	BMRC_Monitor_JailAdd(hMonitor, BRMM_CLIENT_AVD_ILA_0);
	BMRC_Monitor_JailAdd(hMonitor, BRMM_CLIENT_AVD_OLA_0);
	BMRC_Monitor_JailAdd(hMonitor, BRMM_CLIENT_AVD_CAB_0);
	BMRC_Monitor_JailAdd(hMonitor, BRMM_CLIENT_AVD_SYM_0);
	BMRC_Monitor_JailUpdate(hMonitor);
}


See Also:
	BMEM_InstallMonitor
	BRMM_Open
****************************************************************************/

/*
 * Summary this type is used to specify instance of the memory monitor
 */
typedef struct BMRC_P_MonitorContext *BMRC_Monitor_Handle;

typedef struct BMRC_Monitor_Settings
{
	BMRC_AccessType eKernelBlockMode;    /* blocking mode when violations occur for kernel memory */
	BMRC_AccessType eBlockMode;          /* blocking mode when violations occur for non-kernel memory */
	unsigned ulNumCheckersToUse;  /* number of hardware checkers used by this instance of mrc monitor */
	bool startDisabled;
} BMRC_Monitor_Settings;

/***************************************************************************
Summary:
	Opens a realtime memory monitor

Description:
	You may create only one instance of RMM per system.

Returns:
	BERR_SUCCESS - Memory monitor was opened.

See Also:
	BMRC_Close
****************************************************************************/
BERR_Code BMRC_Monitor_Open(
		BMRC_Monitor_Handle *phMonitor,   /* [out] this argument is used to return instance (handle) of memory monitor */
		BREG_Handle hReg,                 /* RegisterInterface handle */
		BINT_Handle iIsr,                 /* InterruptInterface handle */
		BCHP_Handle hChp,                 /* ChipInterface handle */
		BMRC_Handle hMrc,                 /* Memory Range Checker Handle */
		BMMA_DeviceOffset ulMemLow,                /* lowest address to control by the memory monitor, usually 0 */
		BMMA_DeviceOffset ulMemHigh,               /* highest address to control by the memory monitor, usualy 256*2^20 - 1 */
		BMRC_Monitor_Settings *pSettings  /* monitor configuration settings.  use NULL for default settings. */
		);

/***************************************************************************
Summary:
	Closes memory monitor

Description:
	This function is used to release resources allocated by
	BMRC_Monitor_Open.

Returns:
	N/A

See Also:
	BMRC_Open
****************************************************************************/
void BMRC_Monitor_Close(
		BMRC_Monitor_Handle hMonitor /* Instance of the memory monitor */
		);

/***************************************************************************
Summary:
	Enables interrupts which were disabled

Description:
    To reduce amount of noise, once violation is detected and printed, interrupts are disabled from this particular region.
    This function would enable interrupts which were previously disabled

Returns:
	N/A
****************************************************************************/
void BMRC_Monitor_RestoreInterrupts(
    BMRC_Monitor_Handle hMonitor /* Instance of the memory monitor */
    );

/***************************************************************************
Summary:
	Gets default settings for a MRC Monitor

Description:
	This function gets the default configuration settings for a MRC
	Monitor by filling in pDefSettings.  It will fill the usMaxNumCheckers
	with the value of -1 which indicates the monitor will use the maximum
	available number of hardware checkers.  It can be overridden to specify
	a number less than that.

Returns:
	N/A

See Also:
	BMRC_Monitor_Open
****************************************************************************/
BERR_Code
BMRC_Monitor_GetDefaultSettings(
		BMRC_Monitor_Settings *pDefSettings
		);

/***************************************************************************
Summary:
	Gets current settings for a MRC Monitor

Description:
	This function gets the current configuration settings for a MRC
	Monitor by filling in pDefSettings.

Returns:
	N/A

See Also:
	BMRC_Monitor_Open
****************************************************************************/
BERR_Code
BMRC_Monitor_GetSettings(
		BMRC_Monitor_Handle hMonitor, /* Instance of the memory monitor */
		BMRC_Monitor_Settings *pSettings
		);

/***************************************************************************
Summary:
	Returns BMEM_MonitorInterface which is passed into BMEM_InstallMonitor
	for automatic configuration.

Description:
	If your system uses multiple instances of BMEM, you can pass the
	BMEM_MonitorInterface into each one.

	BMEM_MonitorInterface is passed to BMEM by reference and so it is the
	application's responsibility to insure that the instance of
	BMEM_MonitorInterface remains intact all the time while the interface
	can be referenced. For example, do not use an instance of
	BMEM_MonitorInterface which is allocated as a function's local variable.


Returns:
	BERR_SUCCESS - Memory monitor was opened.

****************************************************************************/
BERR_Code
BMRC_Monitor_GetMemoryInterface(
		BMRC_Monitor_Handle hMonitor, /* Instance of the memory monitor */
		BMRC_MonitorInterface *pInterface /* [out] memory interface */
		);

/***************************************************************************
Summary:
Print configuration of memory range checkers
****************************************************************************/
void BMRC_Monitor_Print(BMRC_Monitor_Handle hMonitor);

/***************************************************************************
Summary:
HW blocks available on the system
****************************************************************************/
typedef enum BMRC_Monitor_HwBlock {
#define BCHP_P_MEMC_DEFINE_HWBLOCK(block) BMRC_Monitor_HwBlock_e##block,
#include "memc/bchp_memc_hwblock.h"
#undef BCHP_P_MEMC_DEFINE_HWBLOCK
    BMRC_Monitor_HwBlock_eInvalid,
    BMRC_Monitor_HwBlock_eMax = BMRC_Monitor_HwBlock_eInvalid
} BMRC_Monitor_HwBlock;


/***************************************************************************
Summary:
Specifies how to use the client list
****************************************************************************/
typedef enum BMRC_Monitor_ListType {
    BMRC_Monitor_ListType_eSpecifiedClients,
    BMRC_Monitor_ListType_eOtherClients,
    BMRC_Monitor_ListType_eMax
} BMRC_Monitor_ListType;


/***************************************************************************
Summary:
Configuration for BMRC_MonitorRegion
****************************************************************************/
typedef struct BMRC_MonitorRegion_Settings {
    BMMA_DeviceOffset addr; /*  start of range */
    size_t length;
    bool exclusive; /* this controls how range [addr .. addr+length] used, if 'exclusive' set to true it matches transactions outside of the range, otherwise it matches transactions inside the range */
    bool blockRead;
    bool blockWrite;
    BMRC_Monitor_ListType listType; /* this control clients selection, 'specifiedClients' is used when matching access against known clients, and 'otherClients' is used when known list of clients which should bypass match */
} BMRC_MonitorRegion_Settings;

/***************************************************************************
Summary:
Set default values for BMRC_MonitorRegion_Settings
****************************************************************************/
void BMRC_MonitorRegion_GetDefaultSettings(BMRC_MonitorRegion_Settings *settings);


/***************************************************************************
Summary:
Instance of BMRC_MonitorRegion
****************************************************************************/
typedef struct BMRC_MonitorRegion *BMRC_MonitorRegion_Handle;

/***************************************************************************
Summary:
Add new BMRC_MonitorRegion instance to BMRC_Monitor
****************************************************************************/
BERR_Code BMRC_MonitorRegion_Add(
    BMRC_Monitor_Handle hMonitor,
    BMRC_MonitorRegion_Handle *phRegion,
    const BMRC_MonitorRegion_Settings *settings,
    const BMRC_Monitor_HwBlock *clientList,
    size_t clientListLength /* number of entries in the client list */
    );

/***************************************************************************
Summary:
Remove instance BMRC_MonitorRegion from BMRC_Monitor
****************************************************************************/
void BMRC_MonitorRegion_Remove(BMRC_Monitor_Handle hMonitor, BMRC_MonitorRegion_Handle hRegion);

/**************************************************************************
Summary:
    Gets name of SCB request type
**************************************************************************/
const char * BMRC_Monitor_GetRequestTypeName_isrsafe( unsigned requestType);

void BMRC_Monitor_SetEnabled(
    BMRC_Monitor_Handle hMonitor,
    bool enabled
    );

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* BMRC_MONITOR_H_ */

/* End of File */
