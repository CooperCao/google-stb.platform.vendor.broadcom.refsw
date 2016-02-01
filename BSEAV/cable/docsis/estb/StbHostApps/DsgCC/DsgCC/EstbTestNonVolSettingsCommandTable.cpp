//****************************************************************************
//
// Copyright (c) 2005-2009 Broadcom Corporation
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
//  Filename:       EstbNonVolSettingsCommandTable.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  July, 03 2005
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the eSTB Test non-volatile
//      settings commands in a command table.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
// My api and definitions...
#include "EstbTestNonVolSettingsCommandTable.h"

#include "EstbTestNonVolSettings.h"

#include "ConsoleThread.h"
 
#include "BoolCommandParameter.h"
#include "MacAddressCommandParameter.h"
#include "UnsignedIntCommandParameter.h"
#include "SignedIntCommandParameter.h"

#include "IpAddressCommandParameter.h"
#include "StringCommandParameter.h"
#include "StringSetCommandParameter.h"

#include "IpHelperFactory.h"

#if !defined (BCM_STANDALONE_DSGCC)
	#include "MessageLogNonVolSettings.h"
#endif

//********************** Local Variables *************************************

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = "Commands to show/change the ESTB NonVol settings.";

// The command ids for the commands I can handle.

enum
{
    kPrimaryParameter = 1,
    kSecondaryParameter,
    kThirdParameter
};


//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Creates the command table, adds all of the
// commands to it and sets the command handler function.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmEstbTestNonVolSettingsCommandTable::BcmEstbTestNonVolSettingsCommandTable(void) :
    BcmNonVolSettingsCommandTable("ESTB Test NonVol Commands", "test_estb", "ESTB Test NonVol", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fIsHidden = false;
    command.fCommandFunctionWithParms = CommandHandler;

    // Try to get the singleton instance of the settings object, and add it as
    // the one-and-only instance to me.
    BcmEstbTestNonVolSettings *pSingleton = BcmEstbTestNonVolSettings::GetSingletonInstance();

    if (pSingleton != NULL)
    {
        AddInstance("ESTB NonVol Settings", pSingleton);
    }
    else
    {
        gLogMessageRaw 
            << "BcmEstbTestNonVolSettingsCommandTable::BcmEstbTestNonVolSettingsCommandTable: ERROR - Singleton instance of BcmEstbTestNonVolSettings is NULL!" << endl;
    }

}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmEstbTestNonVolSettingsCommandTable::~BcmEstbTestNonVolSettingsCommandTable()
{
    // Nothing to do...
}


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
void BcmEstbTestNonVolSettingsCommandTable::CommandHandler(void *pInstanceValue, 
                                                           const BcmCommand &command)
{
    const char *pRebootString = "\nNOTE:  You must write the settings and reboot for the change to take effect!\n";
    bool somethingChanged = false;
    const BcmCommandParameter *pParameter = NULL;
    
    // By convention, the settings object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmEstbTestNonVolSettings *pSettings = (BcmEstbTestNonVolSettings *) pInstanceValue;

    if (pSettings == NULL)
    {
        gLogMessageRaw << "BcmEstbTestNonVolSettingsCommandTable::CommandHandler: ERROR - settings pointer is NULL!" << endl;

        return;
    }


    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }  

    if (somethingChanged)
    {
        gLogMessageRaw << pRebootString;
    }


}



