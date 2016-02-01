//****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       EstbAppCompositeNonVolSettings.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 2006
//
//****************************************************************************
//  Description:
//      This class composites all of the individual settings groups together for
//      the eSTB SNMP Agent application.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbAppCompositeNonVolSettings.h"

// The settings that I contain.
#include "EstbAppNonVolSettings.h"
#include "MessageLogNonVolSettings.h"

#include "EstbSnmpNonVolSettings.h"
#include "EstbEventLogNonVolSettings.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Set this to NULL initially.  This will be set up in my constructor.
BcmEstbAppCompositeNonVolSettings *BcmEstbAppCompositeNonVolSettings::pfSingletonInstance = NULL;


// Default Constructor.  Initializes the state of the object, setting all
// fields to good default values.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmEstbAppCompositeNonVolSettings::BcmEstbAppCompositeNonVolSettings(void) 
{
    BcmNonVolSettings *pSettings;

    // Override the class name in the message log settings that was set by my
    // base class.
    fMessageLogSettings.SetModuleName("BcmEstbAppCompositeNonVolSettings");

    // Create one of each of the settings I contain, set myself as the parent,
    // and add it to the list.
    // CM App settings.
    pSettings = new BcmEstbAppNonVolSettings;
    if (!AddSettingsGroup(pSettings))
    {
        gErrorMsg(fMessageLogSettings, "BcmEstbAppCompositeNonVolSettings")
            << "Failed to add BcmEstbAppNonVolSettings to the composite!" << endl; 
        delete pSettings;
    }


    // SNMP settings.
    pSettings = new BcmSnmpNonVolSettings;
    if (!AddSettingsGroup(pSettings))
    {
        gErrorMsg(fMessageLogSettings, "BcmEstbAppCompositeNonVolSettings")
            << "Failed to add BcmSnmpNonVolSettings to the composite!" << endl;

        delete pSettings;
    }

    // Event Log settings.
    pSettings = new EstbEventLogNonVolSettings;
    if (!AddSettingsGroup(pSettings))
    {
        gErrorMsg(fMessageLogSettings, "BcmEstbAppCompositeNonVolSettings")
            << "Failed to add EstbEventLogNonVolSettings to the composite!" << endl;

        delete pSettings;
    }


    // Set myself as the singleton.
    if (pfSingletonInstance == NULL)
    {
        gInfoMsg(fMessageLogSettings, "BcmEstbAppCompositeNonVolSettings")
            << "Setting up the singleton pointer." << endl;
        pfSingletonInstance = this;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "BcmEstbAppCompositeNonVolSettings")
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
BcmEstbAppCompositeNonVolSettings::~BcmEstbAppCompositeNonVolSettings()
{
    // Clear the singleton pointer.
    if (pfSingletonInstance == this)
    {
        gInfoMsg(fMessageLogSettings, "~BcmEstbAppCompositeNonVolSettings")
            << "Clearing the singleton pointer." << endl;

        pfSingletonInstance = NULL;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "~BcmEstbAppCompositeNonVolSettings")
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
BcmEstbAppCompositeNonVolSettings *BcmEstbAppCompositeNonVolSettings::GetSingletonInstance(void)
{
    if (pfSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "BcmEstbAppCompositeNonVolSettings::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
    }
    
    return pfSingletonInstance;
}




