//****************************************************************************
//
//  Copyright (c) 2003  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16251 Laguna Canyon Road
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       EventLogNonVolSettingsCommandTable.h
//  Author:         Kevin O'Neal
//  Creation Date:  February 13, 2003
//
//****************************************************************************
//  Description:
//      This derived class handles set-up and configuration of the DOCSIS
//      EventLog non-vol settings.
//
//****************************************************************************

#ifndef EVENTLOGNONVOLSETTINGSCOMMANDTABLE_H
#define EVENTLOGNONVOLSETTINGSCOMMANDTABLE_H

//********************** Include Files ***************************************

// My base class.
#include "NonVolSettingsCommandTable.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


class BcmEventLogNonVolSettingsCommandTable : public BcmNonVolSettingsCommandTable
{
public:

    // Default Constructor.  Creates the command table, adds all of the
    // commands to it and sets the command handler function.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmEventLogNonVolSettingsCommandTable(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmEventLogNonVolSettingsCommandTable();

protected:

private:

    // This is the entrypoint that is called whenever one of my commands is
    // entered.
    //
    // Parameters:
    //      pInstanceValue - the instance value to which the command should be
    //                       applied.  Instances must be registered with the
    //                       command table in order for them to receive the
    //                       command.
    //      command - the command (with parameters) that was entered by the user.
    //
    // Returns:  Nothing.
    //
    static void CommandHandler(void *pInstanceValue, const BcmCommand &command);

private:

    // Copy Constructor.  Not supported.
    BcmEventLogNonVolSettingsCommandTable(const BcmEventLogNonVolSettingsCommandTable &otherInstance);

    // Assignment operator.  Not supported.
    BcmEventLogNonVolSettingsCommandTable & operator = (const BcmEventLogNonVolSettingsCommandTable &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif


