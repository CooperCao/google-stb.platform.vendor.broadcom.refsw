//****************************************************************************
//
// Copyright (c) 2009-2012 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
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
//
//  Filename:       DhcpV6System.cpp
//  Author:         Pinar Taskiran Cyr
//  Creation Date:  May 23, 2007
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DhcpV6System.h"

#include "VersionBanner.h"
#include "DhcpV6SystemVersion.h"

#if defined(TARGETOS_vxWorks)
    #include "vxIpHalIf.h"
#elif defined(TARGETOS_Linux)
    #include "LnxIpStackIf.h"	
	#include <net/if.h>	
#elif defined(PSOS)
    #include "PsosIpHalIf.h"
#elif defined(TARGETOS_eCos)
    #include "ecosIpHalIf.h"
#else
    #include "BaseIpHalIf.h"
#endif

#include "IpHalIfFactory.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************
  
//********************** Local Variables *************************************

//********************** Extern Functions ************************************
extern void DhcpClientInit(bool preferredIpVersionIsIPv6, bool dualStackOperationEnabled);

//********************** Local Functions *************************************

//********************** Class Method Implementations ************************


BcmDhcpV6System::BcmDhcpV6System(void) :
    BcmBfcSystem("DhcpV6 Regression Commands", "Regress", "Regress", "DhcpV6 Regression Commands")
{
    CallTrace("BcmDhcpV6System", "Constructor");

    fMessageLogSettings.SetModuleName("BcmDhcpV6System");
}


BcmDhcpV6System::~BcmDhcpV6System()
{
    CallTrace("BcmDhcpV6System", "Destructor");
}


void BcmDhcpV6System::AddVersionInfo(BcmVersionBanner *pVersionBanner)
{
    CallTrace("BcmDhcpV6System", "AddVersionInfo");
#if 0
    pVersionBanner->AddComponentInfo("DhcpV6 Regression System",
                                     kVersion,
                                     pVersionBanner->IsRelease(kReleaseState));
#endif
}

bool BcmDhcpV6System::RunImpl(void)
{

    CallTrace("BcmDhcpV6System", "RunImpl");

	DhcpClientInit(false, false);

	return true;
}


/// This method is called by Initialize() (near the end of the method), and
/// it gives the derived class a final chance to do its own app-specific
/// initialization.
///
/// The base class provides a dummy implementation that just returns true.
///
/// \retval
///      true if initialization succeeded.
/// \retval
///      false if initialization failed for some reason.
///
bool BcmDhcpV6System::InitializeImpl(void)
{
    CallTrace("BcmDhcpV6System", "InitializeImpl");

    // -----------------------------------------------------------------------
    // Start up the networking support objects, and register command tables for
    // each interface.
    // -----------------------------------------------------------------------

    // Start the network interfaces (create objects, load drivers, etc).
    //#if (BFC_INCLUDE_NETWORKING_SUPPORT)
    //if (skipDrivers == false)
    {
        // Instantiate the HalIf factory.
        BcmIpHalIfFactory *pFactory = BcmIpHalIfFactory::GetSingleton();

        // PR6829 - make it load HAL drivers.
        pFactory->Initialize();
/*
        if (pfBfcTarget->InitNetworkInterfaces() == false)
        {
            gErrorMsg(fMessageLogSettings, "Initialize")
                << "The BfcTarget failed to initialize one or more network interface drivers!" << endl;

            return false;
        }
*/
        #if (BFC_INCLUDE_CONSOLE_SUPPORT)
        // Iterate through all HALs and set up their command tables, if
        // appropriate.      
        BcmHalIf *pHalIf = pFactory->First();

        while (pHalIf != NULL)
        {
            // If this interface has a command table, add it as a subtable
            // of the main command table.
            if (pHalIf->CommandTable() != NULL)
            {
                BcmCommandTable *pDummy = NULL;

                // Because multiple interfaces might use the same command table
                // (for example, all IP stacks use the IpHalCommandTable), we
                // need to avoid adding the same table more than once.
                if (pfMainCommandTable->IsValidSubtable(pHalIf->CommandTable()->Name(),
                                                        pDummy) == false)
                {
                    pfMainCommandTable->AddSubtable(pHalIf->CommandTable());
                }
            }

            pHalIf = pFactory->Next(pHalIf);
        }
        #endif // (BFC_INCLUDE_CONSOLE_SUPPORT)
    }

    //------------------------------------------------------------------
 
    return true;
}
