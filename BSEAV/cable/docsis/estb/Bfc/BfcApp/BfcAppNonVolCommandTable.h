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
//  $Id: BfcAppNonVolCommandTable.h 1.4 2003/10/03 19:22:52Z dpullen Release $
//
//  Filename:       BfcAppNonVolCommandTable.h (was CmAppNonVolCommandTable.h)
//  Author:         David Pullen
//  Creation Date:  Feb 18, 2000
//
//****************************************************************************

#ifndef BfcAppNonVolCommandTable_H
#define BfcAppNonVolCommandTable_H

//********************** Include Files ***************************************

// My base class.
#include "NonVolSettingsCommandTable.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


/** \ingroup BfcCore
*
*   This derived class handles set-up of the non-volatile settings
*   commands in a command table.
*/
class BcmBfcAppNonVolCommandTable : public BcmNonVolSettingsCommandTable
{
public:

    /// Default Constructor.  Creates the command table, adds all of the
    /// commands to it and sets the command handler function.
    ///
    BcmBfcAppNonVolCommandTable(void);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmBfcAppNonVolCommandTable();

private:

    /// This is the entrypoint that is called whenever one of my commands is
    /// entered.
    ///
    /// Parameters:
    ///      pInstanceValue - the instance value to which the command should be
    ///                       applied.  Instances must be registered with the
    ///                       command table in order for them to receive the
    ///                       command.
    ///      command - the command (with parameters) that was entered by the user.
    ///
    static void CommandHandler(void *pInstanceValue, const BcmCommand &command);

private:

    /// Copy Constructor.  Not supported.
    BcmBfcAppNonVolCommandTable(const BcmBfcAppNonVolCommandTable &otherInstance);

    /// Assignment operator.  Not supported.
    BcmBfcAppNonVolCommandTable & operator = (const BcmBfcAppNonVolCommandTable &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif


