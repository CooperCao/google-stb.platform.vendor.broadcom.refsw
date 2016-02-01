//****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       EstbEventLogNonVolSettings.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  14 JUN 06
//
//****************************************************************************
//  Description:
//      This module simply overrides the interesting members of the 
//      BcmEventLogNonVolSettings base class for Estb Snmp Agent specific
//      characteristics.
//
//      Note that the settings are independant of the device to which they are
//      being stored, allowing the same application settings to be used on
//      multiple target platforms and devices!
//
//****************************************************************************
 
//********************** Include Files ***************************************

// My api and definitions...
#include "EstbEventLogNonVolSettings.h"

#include "MergedNonVolDefaults.h"

#if (SNMP_SUPPORT)
#include "EstbSnmpAgent.h"
#include "EstbEventLog.h"
#endif


//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************

// Set this to NULL initially.  This will be set up in my constructor.
EstbEventLogNonVolSettings *EstbEventLogNonVolSettings::pfSingletonInstance = NULL;


// Default Constructor.  Initializes the state of the object, setting all
// fields to good default values.  Set our instance name and number of
// events stored, create our singleton instance.
//
// Parameters:  None.
//
// Returns:  N/A
//
EstbEventLogNonVolSettings::EstbEventLogNonVolSettings(void) :
    BcmEventLogNonVolSettings("CableModem EventLog NonVol Settings", kDefaultValue_EventLogMaxEntries)
{
    // "CMEV"
    fMagicNumber = 0x434d4556;

    // Override the class name in the message log settings that was set by my
    // base class.
    fMessageLogSettings.SetModuleName("EstbEventLogNonVolSettings");

    // Set myself as the singleton.
    if (pfSingletonInstance == NULL)
    {
        gInfoMsg(fMessageLogSettings, "EstbEventLogNonVolSettings")
            << "Setting up the singleton pointer." << endl;
        pfSingletonInstance = this;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "EstbEventLogNonVolSettings")
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
EstbEventLogNonVolSettings::~EstbEventLogNonVolSettings()
{
    // Clear the singleton pointer.
    if (pfSingletonInstance == this)
    {
        gInfoMsg(fMessageLogSettings, "~EstbEventLogNonVolSettings")
            << "Clearing the singleton pointer." << endl;

        pfSingletonInstance = NULL;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "~EstbEventLogNonVolSettings")
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
EstbEventLogNonVolSettings *EstbEventLogNonVolSettings::GetSingletonInstance(void)
{
    if (pfSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "EstbEventLogNonVolSettings::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
    }
    
    return pfSingletonInstance;
}



// Allows the derived class to set its values to a known good state.  This
// MUST be implemented by the derived class.
//
// Note that the derived class should set the version numbers to the desired
// values in case they were changed (by ReadFrom(), etc).
//
// This method may be called if ReadFrom() returns false in order to ensure
// that there are valid settings in the object.
//
// Parameters:
//      section - the section that should be reset.
//
// Returns:  Nothing.
//
void EstbEventLogNonVolSettings::ResetDefaults(NonVolSection section)
{
    // "CMEV"
    fMagicNumber = 0x434d4556;

    // Let the base class do his thing
    BcmEventLogNonVolSettings::ResetDefaults (section);
    
    // Also clear the docsDevEventTable, if we have SNMP support.
#if (CM_HAL_INCLUDED)
#if (SNMP_SUPPORT)
    // Flush the event log.  Call the event log object to do this so that
    // the MIB table will be erased along with flash.  Do this such that
    // we only clear the contents of this object, don't actually write
    // flash yet.
    if (EstbSnmpAgent::Exists())
      EstbEventLog::Singleton().ResetLog();
#else
    gLogMessageRaw << "NO-SNMP: resetting NV defaults clears event log from NV, but does not\n"
                      "         flush the current log.  Do /Event Log/flush to clear." << endl;
#endif
#endif
}
