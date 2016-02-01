//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//  $Id$
//
//  Filename:       EstbTargetMain.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      This file contains the target startup code (i.e. code related to the
//      embedded system target, as opposed to a host-based simulator target).
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbTargetMain.h"

// Handles message logging stuff.
#include "MessageLog.h"

// PR1465 Various drivers.
// #include "FlashDeviceDriver.h"
// #include "ProgramStoreDeviceDriver.h"
// #include "BootloaderStoreDeviceDriver.h"
#include "NonVolDeviceDriver.h"

#include "bcmAlloc.h"

#if (TARGETOS_Linux)
// This variable must be defined!  If changing this variable, include the leading forward
// slash (for eg:  /home/sw/).  The non-vol file name just gets appended to this path.
#define PATH_TO_NONVOL "./"
#endif

extern "C"
{
    // #include "BOARDHAL.H"

    // PR1465
    #if (POWER_MGMT)
    #include <VoipPowerApi.h>
    #endif
}

// #include "FlashDriverCommandTable.h"

#ifdef BCM_SETTOP_SUPPORT
  // #include "StbProgramStoreDeviceDriverBridge.h"
  #if (TARGETOS_Linux)
    #include "DiskFileNonVolDevice.h"
  #else
    // #include "NonVolDeviceDriverBridge.h"
  #endif
#else
  // Singleton objects that wrap the drivers.
  // #include "NonVolDeviceDriverBridge.h"
  // #include "ProgramStoreDeviceDriverBridge.h"
  // #include "BootloaderStoreDeviceDriverBridge.h"
#endif

#include "MergedNonVolDefaults.h"

#if (BCM_REDUCED_IMAGE_SIZE != 1)
// Comment this out to disable the heap monitor; comment it back in to enable
// the heap monitor.  Note that the heap monitor is not available in the slim
// load.
//    #define RUN_HEAP_MONITOR 1
#endif

#if defined (RUN_HEAP_MONITOR)
    #include "HeapMonitorThread.h"
#endif

// #include "HalIfNonVolSettings.h"

#include "CommandParameterList.h"
#include "BitmaskCommandParameter.h"
#include "UnsignedIntCommandParameter.h"

#if (TARGETOS_Linux)
	#include "DiskFileNonVolDevice.h"
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

enum
{
    kHalDebugZone = 1,
    kVpmPowerdownChannel,
};

enum
{
    kPrimaryParameter = 1
};


#if defined(TARGETOS_Linux)

// For Linux, the thread priority is 99 for some reason.
#define kWatchdogThreadPriority 99

#endif

//********************** Local Variables *************************************

// static unsigned int gCommandIdBase = 0;

//********************** Local Functions *************************************

// static void CommandHandler(void *pInstanceValue, const BcmCommand &command);

//********************** Global Variables ************************************

extern "C"
{
    // This is the HAL's debug control variable.
    // extern uint16 DebugZone; 

    // Global mac address use by the ultra-lame HPNA driver, and quite possibly
    // others.  This is legacy stuff and shouldn't be used any more, but we
    // need to keep it around for now.
    extern unsigned char EthernetAddress[6];
}

//********************** Global Functions ************************************

//********************** Function Implementations ****************************


// This function does the target-specific initialization for CM hardware
// (loading low-level drivers for flash, nonvol, etc), and also creates
// singletons like the target-specific BcmNonVolDevice, etc.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void EstbTargetLowLevelInit(void)
{
    gLogMessageRaw << "EstbTargetLowLevelInit:  Loading NonVol drivers..." << endl;


    gLogMessageRaw << "EstbTargetLowLevelInit:  Creating singletons for NonVol devices...\n" << endl;


    // Create the non-vol device for the target.  This also sets up the
    // singleton.
	#if (TARGETOS_Linux)
       // Create the full path to the dynamic non-vol file
        char *dynFileNameWithPath = new char [strlen(PATH_TO_NONVOL) + 20];
        strcpy(dynFileNameWithPath, (char *)PATH_TO_NONVOL);
        strcat(dynFileNameWithPath, "estbsnmpdynamic.bin");

        // Create the full path to the permanent non-vol file
        char *permFileNameWithPath = new char [strlen(PATH_TO_NONVOL) + 22];
        strcpy(permFileNameWithPath, (char *)PATH_TO_NONVOL);
        strcat(permFileNameWithPath, "estbsnmppermanent.bin");

        BcmDiskFileNonVolDevice *pNonVolDevice = new BcmDiskFileNonVolDevice(dynFileNameWithPath, permFileNameWithPath);
        delete []dynFileNameWithPath;
        delete []permFileNameWithPath;
	#endif


    // Start the heap monitor thread if compiled in.
    #if defined (RUN_HEAP_MONITOR)
    
    BcmHeapMonitorThread *pHeapMonitorThread = new BcmHeapMonitorThread;

    #endif
}


// This function does the target-specific startup.  This is called after nonvol
// settings have been read and validated.  This does stuff like configure DMA
// buffer allocations, start the watchdog, and other board/target initialization
// that requires nonvol settings.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void EstbTargetPostNonvolInit(void)
{
// Nothin' to do here for this App.
}

