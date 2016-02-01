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
//  $Id$
//
//  Filename:       CmAppCompositeNonVolCommandTable.h
//  Author:         David Pullen
//  Creation Date:  Nov 2, 2000
//
//****************************************************************************
//  Description:
//      This derived class sets up all of the command subtables associated with
//      the CmAppCompositeNonVolSettings object.
//
//****************************************************************************

#ifndef CMAPPCOMPOSITENONVOLCOMMANDTABLE_H
#define CMAPPCOMPOSITENONVOLCOMMANDTABLE_H

//********************** Include Files ***************************************

// My base class.
#include "NonVolSettingsCommandTable.h"

// The command subtables that I contain.  This mirrors the settings objects that
// are contained in CmAppNonVolSettings.h.
#include "BfcAppCompositeNonVolCommandTable.h"
#include "CmAppNonVolCommandTable.h"
#include "DsgccNonVolSettingsCommandTable.h"
#include "EstbTestNonVolSettingsCommandTable.h"

#if (CM_HAL_INCLUDED)
#include "CmDocsisNonVolSettingsCommandTable.h"
#include "DocsisCmUpstreamCalibrationNonVolSettingsCommandTable.h"
#include "DocsisCmDownstreamCalibrationNonVolSettingsCommandTable.h"
#endif

#if (BCM_FACTORY_SUPPORT)
#include "FactoryNonVolSettingsCommandTable.h"
#endif


//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


class BcmCmAppCompositeNonVolCommandTable : public BcmBfcAppCompositeNonVolCommandTable
{
public:

    // Default Constructor.  Creates the command table, adds all of the
    // commands to it and sets the command handler function.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmCmAppCompositeNonVolCommandTable(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmCmAppCompositeNonVolCommandTable();

protected:

private:

    BcmCmAppNonVolCommandTable            fCmAppCommandTable;
#if (CM_HAL_INCLUDED)
    BcmCmDocsisNonVolSettingsCommandTable fDocsisCommandTable;
    BcmDocsisCmUpstreamCalibrationNonVolSettingsCommandTable fUsCalCommandTable;
    BcmDocsisCmDownstreamCalibrationNonVolSettingsCommandTable fDsCalCommandTable;
#endif

    BcmDsgccNonVolSettingsCommandTable    fDsgccCommandTable;
	BcmEstbTestNonVolSettingsCommandTable fEstbTestCommandTable;
//    BcmSnmpNonVolSettingsCommandTable     fSnmpCommandTable;

#if (EMTA_SUPPORT)
    BcmEmtaNonVolSettingsCommandTable           fEmtaCommandTable;
    BcmEmtaBlindDataNonVolSettingsCommandTable  fEmtaBlindDataCommandTable;
    BcmEmtaSecurityNonVolSettingsCommandTable   fEmtaSecurityCommandTable;
    BcmEmtaSnmpNonVolSettingsCommandTable       fEmtaSnmpCommandTable;
#endif

#if (IPSEC_SUPPORT)
    BcmIpsecNonVolSettingsCommandTable  fIpsecCommandTable;
#endif 
    
#if (WIFI80211_HAL_INCLUDED)
    BcmWiFi80211NonVolSettingsCommandTable  f80211CommandTable;
#endif    

#if (BCM_HTTPS_SUPPORT)
#if (VENDOR_WEB_PAGES_NONVOL)
	BcmHttpServerNonVolSettingsCommandTable fHttpServerCommandTable;
#endif
#endif

#if (PROPANE_SUPPORT)
    BcmCmPropaneNonVolSettingsCommandTable  fCmPropaneNonVolSettingsCommandTable;
#endif    

#if (BCM_CABLEHOME_SUPPORT)
    BcmCdpNonVolSettingsCommandTable       fCdpCommandTable;
    BcmPsDevNonVolSettingsCommandTable     fPsDevCommandTable;
    BcmCapNonVolSettingsCommandTable       fCapCommandTable;
    BcmCmpNonVolSettingsCommandTable       fCmpCommandTable;
    BcmCspNonVolSettingsCommandTable       fCspCommandTable;
    BcmRgNonVolSettingsCommandTable        fRgCommandTable;
#endif

#if (BCM_FIREWALL_ENABLED || BCM_PARENTALCTRL_ENABLED)
    BcmFirewallNonVolSettingsCommandTable  fFirewallNonVolSettingsCommandTable;

    #if (BCM_PARENTALCTRL_ENABLED)
    BcmParentalCtrlNonVolSettingsCommandTable  fParentalCtrlNonVolSettingsCommandTable;
    #endif
#endif

#if (BCM_FACTORY_SUPPORT)
    BcmFactoryNonVolSettingsCommandTable fFactoryCommandTable;
#endif

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmCmAppCompositeNonVolCommandTable(const BcmCmAppCompositeNonVolCommandTable &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmCmAppCompositeNonVolCommandTable & operator = (const BcmCmAppCompositeNonVolCommandTable &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif


