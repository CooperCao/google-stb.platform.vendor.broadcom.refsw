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
//  Filename:       EstbEventLogNonVolSettings.h
//  Author:         Maurice Turcotte
//  Creation Date:  14 JUN 06
//
//****************************************************************************
//  Description:
//      These are the Non-Volatile settings specific to the Estb Snmp Agent 
//      event log.
//
//      Note that the settings are independant of the device to which they are
//      being stored, allowing the same application settings to be used on
//      multiple target platforms and devices!
//
//****************************************************************************

#ifndef ESTBEVENTLOGNONVOLSETTINGS_H
#define ESTBEVENTLOGNONVOLSETTINGS_H

//********************** Include Files ***************************************

//********************** Include Files ***************************************

// My base class.
#include "EventLogNonVolSettings.h"

//********************** Global Types ****************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


class EstbEventLogNonVolSettings : public BcmEventLogNonVolSettings
{
public:

    // Default Constructor.  Initializes the state of the object, setting all
    // fields to good default values.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    EstbEventLogNonVolSettings(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~EstbEventLogNonVolSettings();

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
    static EstbEventLogNonVolSettings *GetSingletonInstance(void);
    
    void ResetDefaults(NonVolSection section);

private:

    // Used to keep Reset() from printing warnings when it is called from the
    // constructor.
    bool fConstructorIsRunning;

private:

   // This is the singleton instance of this class.
    static EstbEventLogNonVolSettings *pfSingletonInstance;

private:
    
    // Copy Constructor.  Not supported.
    EstbEventLogNonVolSettings(const EstbEventLogNonVolSettings &otherInstance);

    // Assignment operator.  Not supported.
    EstbEventLogNonVolSettings & operator = (const EstbEventLogNonVolSettings &otherInstance);
};

#endif

