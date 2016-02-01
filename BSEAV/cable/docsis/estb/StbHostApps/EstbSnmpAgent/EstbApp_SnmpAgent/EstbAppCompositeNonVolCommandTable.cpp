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
//  Filename:       EstbAppCompositeNonVolCommandTable.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      This derived class sets up all of the command subtables associated with
//      the EstbAppCompositeNonVolSettings object.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbAppCompositeNonVolCommandTable.h"

// The settings objects that are associated with the command tables.
#include "EstbAppCompositeNonVolSettings.h"

#include "EventLogNonVolSettings.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************
static const char *pgTableDescr = "The root directory for accessing and modifying the non-volatile settings for\nthe eSTB SNMP Agent.";


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
BcmEstbAppCompositeNonVolCommandTable::BcmEstbAppCompositeNonVolCommandTable(void) :
    BcmNonVolSettingsCommandTable("Non-Volatile Settings Commands", "non-vol", "NonVol", pgTableDescr)
{
    // Try to get the singleton instance of the settings object, and add it as
    // the one-and-only instance to me.
    BcmNonVolSettings *pSingleton = BcmEstbAppCompositeNonVolSettings::GetSingletonInstance();

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
            << "BcmEstbAppCompositeNonVolCommandTable::BcmEstbAppCompositeNonVolCommandTable: ERROR - Singleton instance of BcmEstbAppCompositeNonVolSettings is NULL!" << endl;
    }
    
    // Add the subtables that I contain.
    AddSubtable(&fSnmpCommandTable);
    AddSubtable(&BcmEventLogNonVolSettings::fCommandTable);

    // Do we really need this??? AddSubtable(&fEstbNonVolSettingsCommandTable);

}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmEstbAppCompositeNonVolCommandTable::~BcmEstbAppCompositeNonVolCommandTable()
{
    // Remove the subtables that I previously added.
    RemoveSubtable(&fSnmpCommandTable);
    RemoveSubtable(&BcmEventLogNonVolSettings::fCommandTable);
    // Do we really need this??? RemoveSubtable(&fEstbNonVolSettingsCommandTable);
}



