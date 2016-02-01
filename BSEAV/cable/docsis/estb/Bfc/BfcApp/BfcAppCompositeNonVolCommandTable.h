//****************************************************************************
//
//  Copyright (c) 2003  Broadcom Corporation
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
//  $Id: BfcAppCompositeNonVolCommandTable.h 1.7 2005/03/03 16:01:14Z koneal Release $
//
//  Filename:       BfcAppCompositeNonVolCommandTable.h
//  Author:         David Pullen
//  Creation Date:  June 11, 2003
//
//****************************************************************************

#ifndef BfcAppCompositeNonVolCommandTable_H
#define BfcAppCompositeNonVolCommandTable_H

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
*   This derived class sets up all of the command subtables associated with
*   the BfcAppCompositeNonVolSettings object.
*/
class BcmBfcAppCompositeNonVolCommandTable : public BcmNonVolSettingsCommandTable
{
public:

    /// Default Constructor.  Creates the command table, adds all of the
    /// commands to it and sets the command handler function.
    ///
    BcmBfcAppCompositeNonVolCommandTable(void);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmBfcAppCompositeNonVolCommandTable();

private:

    /// \todo - UNFINISHED:  I think I want to remove all of these from here.
    /// This class should just be a top level container, and all command tables
    /// should be added to it by the appropriate components.

    BcmNonVolSettingsCommandTable *pfBfcAppNonvolCommandTable;
    BcmNonVolSettingsCommandTable *pfMessageLogNonvolCommandTable;

    /// These two will only be present (non-NULL) if networking support is
    /// enabled, and if 802.11 support is enabled.
    BcmNonVolSettingsCommandTable *pfHalIfNonvolCommandTable;
    BcmNonVolSettingsCommandTable *pf80211NonvolCommandTable;
    BcmNonVolSettingsCommandTable *pfHomePlugNonvolCommandTable;
    
    // If factory support is enabled, this will be non-NULL.
    BcmNonVolSettingsCommandTable *pfFactoryNonvolCommandTable;
    
    // Only present if battery support is enabled.
    BcmNonVolSettingsCommandTable *pfBatteryNonvolCommandTable;
    
private:

    /// Copy Constructor.  Not supported.
    BcmBfcAppCompositeNonVolCommandTable(const BcmBfcAppCompositeNonVolCommandTable &otherInstance);

    /// Assignment operator.  Not supported.
    BcmBfcAppCompositeNonVolCommandTable & operator = (const BcmBfcAppCompositeNonVolCommandTable &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif


