//****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
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
//  Filename:       EstbAppCompositeNonVolCommandTable.h
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      This derived class sets up all of the command subtables associated with
//      the CmAppCompositeNonVolSettings object.
//
//****************************************************************************

#ifndef ESTBAPPCOMPOSITENONVOLCOMMANDTABLE_H
#define ESTBAPPCOMPOSITENONVOLCOMMANDTABLE_H

//********************** Include Files ***************************************

// My base class.
#include "NonVolSettingsCommandTable.h"

// The command subtables that I contain.  This mirrors the settings objects that
// are contained in CmAppNonVolSettings.h.
// #include "AppCompositeNonVolCommandTable.h"
#include "MessageLogNonVolSettingsCommandTable.h"

#include "EstbSnmpNonVolSettingsCommandTable.h"

#if (BCM_FACTORY_SUPPORT)
#include "FactoryNonVolSettingsCommandTable.h"
#endif

#include "NonVolSettingsCommandTable.h"


//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


class BcmEstbAppCompositeNonVolCommandTable : public BcmNonVolSettingsCommandTable
{
public:

    // Default Constructor.  Creates the command table, adds all of the
    // commands to it and sets the command handler function.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmEstbAppCompositeNonVolCommandTable(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmEstbAppCompositeNonVolCommandTable();

protected:

private:

    // BcmCmAppNonVolCommandTable            fCmAppCommandTable;
    BcmSnmpNonVolSettingsCommandTable     fSnmpCommandTable;


private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmEstbAppCompositeNonVolCommandTable(const BcmEstbAppCompositeNonVolCommandTable &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmEstbAppCompositeNonVolCommandTable & operator = (const BcmEstbAppCompositeNonVolCommandTable &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif


