//****************************************************************************
//
// Copyright (c) 2003-2009 Broadcom Corporation
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
//****************************************************************************//
//  $Id$
//
//  Filename:       BfcAppCompositeNonVolCommandTable.cpp
//  Author:         David Pullen
//  Creation Date:  June 11, 2003
//
//****************************************************************************
//  Description:
//      This derived class sets up all of the command subtables associated with
//      the BfcAppCompositeNonVolSettings object.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "BfcAppCompositeNonVolCommandTable.h"

// My object represents the top level command table for the entire composite of
// nonvol settings.  I need to register myself with the compsite object.
#include "BfcAppCompositeNonVolSettings.h"

// The command subtables that I contain.  This mirrors the settings objects that
// are contained in BfcAppCompositeNonVolSettings.cpp.
#include "BfcAppNonVolCommandTable.h"
#include "MessageLogNonVolSettingsCommandTable.h"

#if BFC_INCLUDE_NETWORKING_SUPPORT
#include "HalIfNonVolSettingsCommandTable.h"

  #if BFC_INCLUDE_WIFI80211_SUPPORT
  #include "WiFi80211NonVolSettingsCommandTable.h"
  #if(BCM_80211N)      // 802.11n functions
  #include "WiFi80211SromNonVolSettingsCommandTable.h"
  #endif
  #endif

  #if (BFC_INCLUDE_HOMEPLUG_SUPPORT)
  #include "HomePlugNonVolSettingsCommandTable.h"
  #endif

#endif

#if BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT
  #include "FactoryNonVolSettingsCommandTable.h"
#endif

#if BFC_INCLUDE_BATTERY_SUPPORT
  #include "BatteryNonVolSettingsCommandTable.h"
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** External Variables **********************************

//********************** Class Method Implementations ************************


// Default Constructor.  Creates the command table, adds all of the
// commands to it and sets the command handler function.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmBfcAppCompositeNonVolCommandTable::BcmBfcAppCompositeNonVolCommandTable(void)
{
    // Try to get the singleton instance of the settings object, and add it as
    // the one-and-only instance to me.
    BcmNonVolSettings *pSingleton = BcmBfcAppCompositeNonVolSettings::GetSingletonInstance();

    if (pSingleton != NULL)
    {
        AddInstance("Application NonVol Settings", pSingleton);

        // Since I know that this is a composite settings type, I will enable
        // my "flush" command.
        EnableCommand(kFlushUnrecognized, true);
    }
    else
    {
        gLogMessageRaw
            << "BcmBfcAppCompositeNonVolCommandTable::BcmBfcAppCompositeNonVolCommandTable: ERROR - Singleton instance of BcmCmAppCompositeNonVolSettings is NULL!" << endl;
    }

    #if BFC_INCLUDE_MESSAGELOG_SUPPORT
    // Create and add the subtables that I contain.
    pfBfcAppNonvolCommandTable = new BcmBfcAppNonVolCommandTable;
    AddSubtable(pfBfcAppNonvolCommandTable);

    pfMessageLogNonvolCommandTable = new BcmMessageLogNonVolSettingsCommandTable;
    AddSubtable(pfMessageLogNonvolCommandTable);
    #endif

#if BFC_INCLUDE_NETWORKING_SUPPORT
    pfHalIfNonvolCommandTable = new BcmHalIfNonVolSettingsCommandTable;
    AddSubtable(pfHalIfNonvolCommandTable);

    #if BFC_INCLUDE_WIFI80211_SUPPORT
    pf80211NonvolCommandTable = new BcmWiFi80211NonVolSettingsCommandTable;
    AddSubtable(pf80211NonvolCommandTable);

	#if(BCM_80211N)      // 802.11n functions
    pf80211NonvolCommandTable = new BcmWiFi80211SromNonVolSettingsCommandTable;
    AddSubtable(pf80211NonvolCommandTable);
	#endif

    #endif

    #if (BFC_INCLUDE_HOMEPLUG_SUPPORT)
    pfHomePlugNonvolCommandTable = new BcmHomePlugNonVolSettingsCommandTable;
    AddSubtable(pfHomePlugNonvolCommandTable);
    #endif

#endif

#if BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT
    pfFactoryNonvolCommandTable = new BcmFactoryNonVolSettingsCommandTable;
    AddSubtable(pfFactoryNonvolCommandTable);
#endif

#if BFC_INCLUDE_BATTERY_SUPPORT
    pfBatteryNonvolCommandTable = new BcmBatteryNonVolSettingsCommandTable;
    AddSubtable(pfBatteryNonvolCommandTable);
#endif
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmBfcAppCompositeNonVolCommandTable::~BcmBfcAppCompositeNonVolCommandTable()
{
    // Remove and delete the subtables that I previously added.
    RemoveSubtable(pfMessageLogNonvolCommandTable);
    delete pfMessageLogNonvolCommandTable;
    pfMessageLogNonvolCommandTable = NULL;

    RemoveSubtable(pfBfcAppNonvolCommandTable);
    delete pfBfcAppNonvolCommandTable;
    pfBfcAppNonvolCommandTable = NULL;

#if BFC_INCLUDE_NETWORKING_SUPPORT
    RemoveSubtable(pfHalIfNonvolCommandTable);
    delete pfHalIfNonvolCommandTable;
    pfHalIfNonvolCommandTable = NULL;

    #if BFC_INCLUDE_WIFI80211_SUPPORT
    RemoveSubtable(pf80211NonvolCommandTable);
    delete pf80211NonvolCommandTable;
    pf80211NonvolCommandTable = NULL;
    #endif

    #if BFC_INCLUDE_HOMEPLUG_SUPPORT
    RemoveSubtable(pfHomePlugNonvolCommandTable);
    delete pfHomePlugNonvolCommandTable;
    pfHomePlugNonvolCommandTable = NULL;
    #endif
#endif

#if BFC_INCLUDE_FACTORY_SUPPORT
    RemoveSubtable(pfFactoryNonvolCommandTable);
    delete pfFactoryNonvolCommandTable;
    pfFactoryNonvolCommandTable = NULL;
#endif

#if BFC_INCLUDE_BATTERY_SUPPORT
    RemoveSubtable(pfBatteryNonvolCommandTable);
    delete pfBatteryNonvolCommandTable;
    pfBatteryNonvolCommandTable = NULL;
#endif
}



