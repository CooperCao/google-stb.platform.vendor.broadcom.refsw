//****************************************************************************
//
//  Copyright (c) 2000  Broadcom Corporation
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
//
//  Filename:       EstbNonVolSettingsCommandTable.h
//  Author:         Pinar Taskiran
//  Creation Date:  July, 03 2005
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the RG non-volatile
//      settings commands in a command table.
//
//****************************************************************************

#ifndef EstbTestNonVolSettingsCommandTable_H
#define EstbTestNonVolSettingsCommandTable_H

//********************** Include Files ***************************************

// My base class.
#include "NonVolSettingsCommandTable.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


class BcmEstbTestNonVolSettingsCommandTable : public BcmNonVolSettingsCommandTable
{
public:

    // Default Constructor.  Creates the command table, adds all of the
    // commands to it and sets the command handler function.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmEstbTestNonVolSettingsCommandTable(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmEstbTestNonVolSettingsCommandTable();

protected:

private:

    // This is the entrypoint that is called whenever one of my commands is
    // entered.
    //
    // Parameters:
    //      pUserValue - the instance value to which the command should be
    //                   applied.  Instances must be registered with the
    //                   command table in order for them to receive the
    //                   command.
    //      commandId - the id of the command that was entered (from the
    //                  Command structure).
    //      pParameters - the string containing any parameters that were entered
    //                    along with the command.  You must not modify this
    //                    string (e.g. with strtok) to parse it, and you must
    //                    not free it.
    //
    // Returns:  Nothing.
    //
    static void CommandHandler(void *pInstanceValue, const BcmCommand &command);

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmEstbTestNonVolSettingsCommandTable(const BcmEstbTestNonVolSettingsCommandTable &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmEstbTestNonVolSettingsCommandTable & operator = (const BcmEstbTestNonVolSettingsCommandTable &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif


