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
//****************************************************************************
//  $Id$
//
//  Filename:       BfcAppCompositeNonVolSettings.cpp
//  Author:         David Pullen
//  Creation Date:  June 11, 2003
//
//****************************************************************************
//  Description:
//      This class composites all of the individual settings groups together for
//      the BFC-based application.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "BfcAppCompositeNonVolSettings.h"

// The settings that I contain.
#include "BfcAppNonVolSettings.h"
#include "MessageLogNonVolSettings.h"

#if BFC_INCLUDE_NETWORKING_SUPPORT
#include "HalIfNonVolSettings.h"

#if (BFC_INCLUDE_WIFI80211_SUPPORT)
#include "WiFi80211NonVolSettings.h"
#if(BCM_80211N)      // 802.11n functions
#include "WiFi80211SromNonVolSettings.h"
#endif
#endif

#if (BFC_INCLUDE_HOMEPLUG_SUPPORT)
#include "HomePlugNonVolSettings.h"
#endif

#endif

#if BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT
#include "FactoryNonVolSettings.h"
#endif

#if BFC_INCLUDE_BATTERY_SUPPORT
#include "BatteryNonVolSettings.h"
#endif

#if (BCM_RESET_EVENT_LOG_SUPPORT)
#include "ResetSystemEventLogNonVolSettings.h"
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Set this to NULL initially.  This will be set up in my constructor.
BcmBfcAppCompositeNonVolSettings *BcmBfcAppCompositeNonVolSettings::pgSingletonInstance = NULL;


// Default Constructor.  Initializes the state of the object, setting all
// fields to good default values.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmBfcAppCompositeNonVolSettings::BcmBfcAppCompositeNonVolSettings(void) 
{
    BcmNonVolSettings *pSettings;

    // Override the class name in the message log settings that was set by my
    // base class.
    fMessageLogSettings.SetModuleName("BcmBfcAppCompositeNonVolSettings");

    // Create one of each of the settings I contain, set myself as the parent,
    // and add it to the list.

	#if !defined (BCM_STANDALONE_SNMP_AGENT) && !defined (BCM_STANDALONE_DSGCC)
    // BFC App settings.
    pSettings = new BcmBfcAppNonVolSettings;
    if (!AddSettingsGroup(pSettings))
    {
        gErrorMsg(fMessageLogSettings, "BcmBfcAppCompositeNonVolSettings")
            << "Failed to add BcmBfcAppNonVolSettings to the composite!" << endl; 

        delete pSettings;
    }
	#endif

	#if !defined (BCM_STANDALONE_SNMP_AGENT) && !defined (BCM_ESTB_STANDALONE_APP_SUPPORT)
    // Message logging settings.
    pSettings = new BcmMessageLogNonVolSettings;
    if (!AddSettingsGroup(pSettings))
    {
        gErrorMsg(fMessageLogSettings, "BcmBfcAppCompositeNonVolSettings")
            << "Failed to add BcmMessageLogNonVolSettings to the composite!" << endl;

        delete pSettings;
    }
    #endif

#if BFC_INCLUDE_NETWORKING_SUPPORT        
    // Halif settings if we have networking.
    pSettings = new BcmHalIfNonVolSettings;
    if (!AddSettingsGroup(pSettings))
    {
        gErrorMsg(fMessageLogSettings, "BcmBfcAppCompositeNonVolSettings")
            << "Failed to add HalIfNonVolSettings to the composite!" << endl;

        delete pSettings;
    }
    
    #if BFC_INCLUDE_WIFI80211_SUPPORT
    pSettings = new BcmWiFi80211NonVolSettings;
    if (!AddSettingsGroup(pSettings))
    {
        gErrorMsg(fMessageLogSettings, "BcmBfcAppCompositeNonVolSettings")
            << "Failed to add BcmWiFi80211NonVolSettings to the composite!" << endl;

        delete pSettings;
    }

	#if(BCM_80211N)      // 802.11n functions
    pSettings = new BcmWiFi80211SromNonVolSettings;
    if (!AddSettingsGroup(pSettings))
    {
        gErrorMsg(fMessageLogSettings, "BcmBfcAppCompositeNonVolSettings")
            << "Failed to add BcmWiFi80211SromNonVolSettings to the composite!" << endl;

        delete pSettings;
    }
	#endif
    #endif
    
    #if BFC_INCLUDE_HOMEPLUG_SUPPORT
    pSettings = new BcmHomePlugNonVolSettings;
    if (!AddSettingsGroup(pSettings))
    {
        gErrorMsg(fMessageLogSettings, "BcmBfcAppCompositeNonVolSettings")
            << "Failed to add BcmHomePlugNonVolSettings to the composite!" << endl;

        delete pSettings;
    }
    #endif
    
#endif

#if BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT
    // Factory settings if we have factory support.
    pSettings = new BcmFactoryNonVolSettings;
    if (!AddSettingsGroup(pSettings))
    {
        gErrorMsg(fMessageLogSettings, "BcmBfcAppCompositeNonVolSettings")
            << "Failed to add FactoryNonVolSettings to the composite!" << endl;

        delete pSettings;
    }
#endif

#if BFC_INCLUDE_BATTERY_SUPPORT
    // Battery settings if we have factory support.
    pSettings = new BcmBatteryNonVolSettings;
    if (!AddSettingsGroup(pSettings))
    {
        gErrorMsg(fMessageLogSettings, "BcmBfcAppCompositeNonVolSettings")
            << "Failed to add BatteryNonVolSettings to the composite!" << endl;

        delete pSettings;
    }
#endif

#if (BCM_RESET_EVENT_LOG_SUPPORT)
    // Reset event logging if we have reset event log support
    pSettings = new BcmResetSystemEventLogNonVolSettings;
    if (!AddSettingsGroup(pSettings))
    {
        gErrorMsg(fMessageLogSettings, "CreateAndRegisterNonVolSettings")
            << "Failed to add BcmResetSystemEventLogNonVolSettings to the composite!" << endl;

        delete pSettings;
    }
#endif

    // Set myself as the singleton.
    if (pgSingletonInstance == NULL)
    {
        gInfoMsg(fMessageLogSettings, "BcmBfcAppCompositeNonVolSettings")
            << "Setting up the singleton pointer." << endl;

        pgSingletonInstance = this;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "BcmBfcAppCompositeNonVolSettings")
            << "Singleton pointer is not NULL!  There are multiple instances!  Leaving the singleton pointer alone..." << endl;
    }
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmBfcAppCompositeNonVolSettings::~BcmBfcAppCompositeNonVolSettings()
{
    // Clear the singleton pointer.
    if (pgSingletonInstance == this)
    {
        gInfoMsg(fMessageLogSettings, "~BcmBfcAppCompositeNonVolSettings")
            << "Clearing the singleton pointer." << endl;

        pgSingletonInstance = NULL;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "~BcmBfcAppCompositeNonVolSettings")
            << "I'm not the singleton instance!  Leaving the singleton pointer alone..." << endl;
    }
}


// Returns the pointer to the singleton instance for this class.  Most
// objects in the system will use this method rather than being passed a
// pointer to it.  The singleton pointer will be set up in the base-class
// constructor.
//
// NOTES:  This can return NULL if a singleton has not been set up for the
//         system, so you must check for this condition.
//
//         You must not delete this object!
//
//         You should not store a pointer to the object that is returned,
//         since it may be deleted and replaced with a new one.
//
// Parameters:  None.
//
// Returns:
//      A pointer to the instance that should be used by the system.
//
BcmBfcAppCompositeNonVolSettings *BcmBfcAppCompositeNonVolSettings::GetSingletonInstance(void)
{
    if (pgSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "BcmBfcAppCompositeNonVolSettings::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
    }
    
    return pgSingletonInstance;
}




