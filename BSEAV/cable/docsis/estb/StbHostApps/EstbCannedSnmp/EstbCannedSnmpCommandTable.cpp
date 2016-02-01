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
//  Filename:       EstbCannedSnmpCommandTable.cpp
//  Author:         Pinar Taskiran Cyr
//  Creation Date:  September 22, 2008
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the SNMP 
//      commands in a command table.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbCannedSnmpCommandTable.h"
#include "EstbCannedSnmp.h"

#include "BoolCommandParameter.h"
#include "IpAddressCommandParameter.h"
#include "SignedIntCommandParameter.h"
#include "StringCommandParameter.h"
#include "UnsignedIntCommandParameter.h"

#include <string.h>

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = "Commands to show/change the ESTB CANNED SNMP NonVol settings.";

// The command ids for the commands I can handle.
enum
{
    //kChangeAllowConfiguration = BcmNonVolSettingsCommandTable::kDerivedClassCommands,
    //kSetSysContact,
    //kSetSysName,
    //kSetSysLocation,
    //kSetDocsDevSerialNumber,
    kSetDocsDevSwAdminStatus,
    kGetDocsDevSwOperStatus,
    kSetDocsDevSwFilename,
    kSetDocsDevSwServer,
	kSetDocsDevResetNow
};

// Constants for the command parameters.
enum
{
    kPrimaryParameter = 1,
    kSecondParameter,
    kThirdParameter,
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
BcmEstbCannedSnmpCommandTable::BcmEstbCannedSnmpCommandTable(void) :
    BcmCommandTable("ESTB CANNED SNMP Commands", "cannedsnmp", "cannedsnmp", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fIsHidden = false;
    command.fCommandFunctionWithParms = CommandHandler;

    command.fCommandId = kSetDocsDevSwAdminStatus;
    command.pfCommandName = "docsDevSwAdminStatus";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, NULL, 1, 3));
    command.pfCommandHelp = 
        "Sets the docsDevSwAdminStatus value that will be reported via SNMP.  Valid "
        "values are:\n\n"
        "\t1\tupgradeFromMgt\n"
        "\t2\tallowProvisioningUpgrade\n"
        "\t3\tignoreProvisioningUpgrade";
    command.pfExamples = 
        "docsDevSwAdminStatus 2\t";
    AddCommand(command);

    command.fCommandId = kGetDocsDevSwOperStatus;
    command.pfCommandName = "docsDevSwOperStatus";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, NULL, 1, 5));
    command.pfCommandHelp = 
        "Gets the docsDevSwOperStatus value that is reported via SNMP. This is a READ-ONLY value. Valid "
        "values are:\n\n"
        "\t1\tinProgress\n"
        "\t2\tcompleteFromProvisioning\n"
        "\t3\tcompleteFromMgt\n"
        "\t4\tfailed\n"
        "\t5\tother";
    command.pfExamples = 
        "docsDevSwOperStatus\t";
    AddCommand(command);

    command.fCommandId = kSetDocsDevSwFilename;
    command.pfCommandName = "docsDevSwFilename";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, NULL, 128, true));
    command.pfCommandHelp = 
        "Sets the docsDevSwFilename string that will be reported via SNMP.";
    command.pfExamples = 
        "docsDevSwFilename myfile.bin\t";
    AddCommand(command);

    command.fCommandId = kSetDocsDevSwServer;
    command.pfCommandName = "docsDevSwServer";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kPrimaryParameter, true));
    command.pfCommandHelp = 
        "Sets the docsDevSwServer IP address that will be reported via SNMP.";
    command.pfExamples = 
        "docsDevSwServer 11.24.4.3\t";
    AddCommand(command);

    command.fCommandId = kSetDocsDevResetNow;
    command.pfCommandName = "docsDevResetNow";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, NULL, 1, 2));
    command.pfCommandHelp = 
        "Sets the docsDevResetNow value that will be reported via SNMP.  Valid "
        "values are:\n\n"
        "\t2\tfalse\n"
        "\t1\true\n";
    command.pfExamples = 
        "docsDevResetNow 1\t";
    AddCommand(command);
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmEstbCannedSnmpCommandTable::~BcmEstbCannedSnmpCommandTable()
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
void BcmEstbCannedSnmpCommandTable::CommandHandler(void *pInstanceValue, 
                                                       const BcmCommand &command)
{
    // By convention, the settings object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmEstbCannedSnmp *pSettings = (BcmEstbCannedSnmp *) pInstanceValue;

    if (pSettings == NULL)
    {
        gLogMessageRaw << "BcmEstbCannedSnmpCommandTable::CommandHandler: ERROR - settings pointer is NULL!\n";

        return;
    }

    const BcmCommandParameter *pParameter = NULL;
    
    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        case kSetDocsDevSwFilename:
			{
				if (pParameter->WasParsed())
				{
					pSettings->Set_docsDevSwFilename(pParameter->AsString());
				}

				char fileName[32];
				pSettings->Get_docsDevSwFilename(fileName);
				gLogMessageRaw << "docsDevSwFilename = " << fileName << "\n";
			}
            break;

        case kSetDocsDevSwAdminStatus:
			{
				if (pParameter->WasParsed())
				{
					pSettings->Set_docsDevSwAdminStatus((uint8) pParameter->AsUnsignedInt());
				}

				uint16 status;
				pSettings->Get_docsDevSwAdminStatus(status);

				gLogMessageRaw << "docsDevSwAdminStatus = " << (int) status<< "\n";
			}
            break;

        case kGetDocsDevSwOperStatus:
			{
				uint16 status;
				pSettings->Get_docsDevSwOperStatus(status);

				gLogMessageRaw << "docsDevSwOperStatus = " << (int) status<< "\n";
			}
            break;

        case kSetDocsDevSwServer:
			{
				unsigned long ipaddress;
				if (pParameter->WasParsed())
				{				
					(pParameter->AsIpAddress()).Get(ipaddress);
					pSettings->Set_docsDevSwServer(ipaddress);					
				}			
				
				BcmIpAddress swServer;
				pSettings->Get_docsDevSwServer(ipaddress);
				swServer.Set(ipaddress);
				gLogMessageRaw << "docsDevSwServer = " << swServer << "\n";
				
			}
            break;
          
        case kSetDocsDevResetNow:
			{
				if (pParameter->WasParsed())
				{
					pSettings->Set_docsDevResetNow((uint8) pParameter->AsUnsignedInt());
				}
			}
            break;

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }
}



