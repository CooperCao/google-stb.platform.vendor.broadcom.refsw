//****************************************************************************
//
// Copyright (c) 2004-2012 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized Licens, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//  $Id$
//
//  Filename:       CmAppCompositeNonVolCommandTable.cpp
//  Author:         David Pullen
//  Creation Date:  Nov 2, 2000
//
//****************************************************************************
//  Description:
//      This derived class sets up all of the command subtables associated with
//      the CmAppCompositeNonVolSettings object.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "CmAppCompositeNonVolCommandTable.h"

// The settings objects that are associated with the command tables.
#include "CmAppCompositeNonVolSettings.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

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
BcmCmAppCompositeNonVolCommandTable::BcmCmAppCompositeNonVolCommandTable(void) 
{
    // Add the subtables that I contain.
    AddSubtable(&fCmAppCommandTable);

#if (CM_HAL_INCLUDED)
    AddSubtable(&fDocsisCommandTable);
    AddSubtable(&fUsCalCommandTable);
    AddSubtable(&fDsCalCommandTable);
#endif

    AddSubtable(&fDsgccCommandTable);
    AddSubtable(&fEstbTestCommandTable);
//    AddSubtable(&fSnmpCommandTable);
//    AddSubtable(&BcmEventLogNonVolSettings::fCommandTable);

#if (EMTA_SUPPORT)
    AddSubtable(&fEmtaCommandTable);
    AddSubtable(&fEmtaBlindDataCommandTable);
    AddSubtable(&fEmtaSecurityCommandTable);
    AddSubtable(&fEmtaSnmpCommandTable);
#endif
    
#if (IPSEC_SUPPORT)
    AddSubtable(&fIpsecCommandTable);
#endif

#if (WIFI80211_HAL_INCLUDED)
    AddSubtable(&f80211CommandTable);
#endif

#if (BCM_HTTPS_SUPPORT)
#if (VENDOR_WEB_PAGES_NONVOL)
    AddSubtable(&fHttpServerCommandTable);
#endif
#endif

#if (PROPANE_SUPPORT)
    AddSubtable(&fCmPropaneNonVolSettingsCommandTable);
#endif

#if (BCM_CABLEHOME_SUPPORT)
    AddSubtable(&fCdpCommandTable);
    AddSubtable(&fPsDevCommandTable);
    AddSubtable(&fCapCommandTable);
    AddSubtable(&fCmpCommandTable);
    AddSubtable(&fCspCommandTable);
    AddSubtable(&fRgCommandTable); 
#endif

#if (BCM_FIREWALL_ENABLED || BCM_PARENTALCTRL_ENABLED)
    AddSubtable(&fFirewallNonVolSettingsCommandTable);

    #if (BCM_PARENTALCTRL_ENABLED)
    AddSubtable(&fParentalCtrlNonVolSettingsCommandTable);
    #endif
#endif

#if (BCM_FACTORY_SUPPORT)
    AddSubtable(&fFactoryCommandTable);
#endif

}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmCmAppCompositeNonVolCommandTable::~BcmCmAppCompositeNonVolCommandTable()
{
    // Remove the subtables that I previously added.
    RemoveSubtable(&fCmAppCommandTable);
    
#if (CM_HAL_INCLUDED)
    RemoveSubtable(&fDocsisCommandTable);
    RemoveSubtable(&fUsCalCommandTable);
    RemoveSubtable(&fDsCalCommandTable);
#endif

    RemoveSubtable(&fDsgccCommandTable);
    RemoveSubtable(&fEstbTestCommandTable);
//    RemoveSubtable(&fSnmpCommandTable);
    
#if (EMTA_SUPPORT)
    RemoveSubtable(&fEmtaCommandTable);
    RemoveSubtable(&fEmtaBlindDataCommandTable);
    RemoveSubtable(&fEmtaSecurityCommandTable);
    RemoveSubtable(&fEmtaSnmpCommandTable);
#endif
    
#if (IPSEC_SUPPORT)
    RemoveSubtable(&fIpsecCommandTable);
#endif

#if (WIFI80211_HAL_INCLUDED)
    RemoveSubtable(&f80211CommandTable);
#endif

#if (BCM_HTTPS_SUPPORT)
#if (VENDOR_WEB_PAGES_NONVOL)
    RemoveSubtable(&fHttpServerCommandTable);
#endif
#endif

#if (PROPANE_SUPPORT)
    RemoveSubtable(&fCmPropaneNonVolSettingsCommandTable);
#endif

#if (BCM_CABLEHOME_SUPPORT)
    RemoveSubtable(&fCdpCommandTable);
    RemoveSubtable(&fPsDevCommandTable);
    RemoveSubtable(&fCapCommandTable);
    RemoveSubtable(&fCmpCommandTable);
    RemoveSubtable(&fCspCommandTable);
#endif

#if (BCM_FIREWALL_ENABLED || BCM_PARENTALCTRL_ENABLED)
    RemoveSubtable(&fFirewallNonVolSettingsCommandTable);

    #if (BCM_PARENTALCTRL_ENABLED)
    RemoveSubtable(&fParentalCtrlNonVolSettingsCommandTable);
    #endif
#endif

#if (BCM_FACTORY_SUPPORT)
    RemoveSubtable(&fFactoryCommandTable);
#endif

}



