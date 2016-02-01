//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
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
//  $Id$
//
//  Filename:       EstbSnmpNonVolSettingsCommandTable.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  February 7, 2006
//
//****************************************************************************
//  Description:
//      This derived class handles set-up of the eSTB SNMP non-volatile settings
//      commands in a command table.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbSnmpNonVolSettingsCommandTable.h"

#include "EstbSnmpNonVolSettings.h"

#include "CoreObjs.h"

#include "BoolCommandParameter.h"
#include "IpAddressCommandParameter.h"
#include "SignedIntCommandParameter.h"
#include "StringCommandParameter.h"
#include "UnsignedIntCommandParameter.h"

#include "EstbSnmpAgent.h"

#if (BCM_FACTORY_SUPPORT)
#include "FactoryNonVolSettings.h"
#endif

#include <string.h>

//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = "Commands to show/change the SNMP NonVol settings.";

// The command ids for the commands I can handle.
enum
{
    kChangeAllowConfiguration = BcmNonVolSettingsCommandTable::kDerivedClassCommands,
    kSetVendorName,
    kSetSysDescr,
    kSetSysObjectId,
    kSetSysContact,
    kSetSysName,
    kSetSysLocation,
    kSetSysORID,
    kSetSysORDescr,
    kSetSysServices,
    kSetDocsDevSwCurrentVers,
    kSetDocsDevSerialNumber,
    kSetDocsDevSwAdminStatus,
    kSetDocsDevSwOperStatus,
    kSetDocsDevSwFilename,
    kSetDocsDevSwServer,
    kDownloadTries,
    kMaxDownloadTries,
    
    // fFeatureMask bits
    kIgnoreLlcUnmatched,
    kIgnoreIpDefault,
    kSingleIpFilter,
    kDiagAgentDisabledPostReg,
    kHideIpStackIfEntries,
    kInterfaceName
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
BcmSnmpNonVolSettingsCommandTable::BcmSnmpNonVolSettingsCommandTable(void) :
    BcmNonVolSettingsCommandTable("ESTB SNMP NonVol Commands", "snmp_estb", "ESTB SNMP NonVol", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fIsHidden = false;
    command.fCommandFunctionWithParms = CommandHandler;

    command.fCommandId = kChangeAllowConfiguration;
    command.pfCommandName = "allow_config";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmBoolCommandParameter(kPrimaryParameter, true));
    command.pfCommandHelp = 
        "Enables/disables configuration of various sytem components which are\n"
        "not normally configurable in standard operational mode.  Also in some\n"
        "ways affects system behaviour.  This should more properly be called\n"
        "'factory_mode' because in general it is used to support factory mode\n"
        "things like US/DS calibration, etc.\n"
#if (BCM_FACTORY_SUPPORT)
      "\nNote that setting this to 'true' will set the Broadcom private MIB\n"
        "object cdPrivateMibEnable to engineering(2), which will expose both\n"
        "the cableDataPrivateFactory and cableDataPrivateEngineering branches.\n"
        "Setting it to 'false' will set cdPrivateMibEnable to disabled(0), which\n"
        "will result in both of these branches being hidden.\n"
#endif
        ;
    command.pfExamples = 
        "allow_config true\tEnables configuration / factory mode.";
    AddCommand(command);

    command.fCommandId = kSetVendorName;
    command.pfCommandName = "vendor_name";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, "Name", 128, true));
    command.pfCommandHelp = 
        "Sets the vendor name that will be reported for events and syslog messages.";
    command.pfExamples = 
        "vendor_name Broadcom Corporation\t";
    AddCommand(command);

    command.fCommandId = kSetSysDescr;
    command.pfCommandName = "sysDescr";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, NULL, 255, true));
    command.pfCommandHelp = 
        "Sets the sysDescr string that will be reported via SNMP.";
    command.pfExamples = 
        "sysDescr Best thing since sliced bread\t";
    AddCommand(command);

    command.fCommandId = kSetSysObjectId;
    command.pfCommandName = "sysObjectId";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, NULL, 128, true));
    command.pfCommandHelp = 
        "Sets the sysObjectId string that will be reported via SNMP.";
    command.pfExamples = 
        "sysObjectId 1.2.3.4.5.6\t";
    AddCommand(command);

    command.fCommandId = kSetSysContact;
    command.pfCommandName = "sysContact";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, NULL, 128, true));
    command.pfCommandHelp = 
        "Sets the sysContact string that will be reported via SNMP.";
    command.pfExamples = 
        "sysContact me myself and I\t";
    AddCommand(command);

    command.fCommandId = kSetSysName;
    command.pfCommandName = "sysName";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, NULL, 128, true));
    command.pfCommandHelp = 
        "Sets the sysName string that will be reported via SNMP.";
    command.pfExamples = 
        "sysName whoever you are\t";
    AddCommand(command);

    command.fCommandId = kSetSysLocation;
    command.pfCommandName = "sysLocation";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, NULL, 128, true));
    command.pfCommandHelp = 
        "Sets the sysLocation string that will be reported via SNMP.";
    command.pfExamples = 
        "sysLocation wherever you are\t";
    AddCommand(command);

    command.fCommandId = kSetSysORID;
    command.pfCommandName = "sysORID";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, NULL, 128, true));
    command.pfCommandHelp = 
        "Sets the sysORID string that will be reported via SNMP.";
    command.pfExamples = 
        "sysORID 1.2.3.4.5.6.1\t";
    AddCommand(command);

    command.fCommandId = kSetSysORDescr;
    command.pfCommandName = "sysORDescr";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, NULL, 128, true));
    command.pfCommandHelp = 
        "Sets the sysORDescr string that will be reported via SNMP.";
    command.pfExamples = 
        "sysORDescr who I am\t";
    AddCommand(command);

    command.fCommandId = kSetSysServices;
    command.pfCommandName = "sysServices";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, NULL, 0, 0xff));
    command.pfCommandHelp = 
        "Sets the sysServices value that will be reported via SNMP.";
    command.pfExamples = 
        "sysServices 0x4f\t";
    AddCommand(command);

    command.fCommandId = kInterfaceName;
    command.pfCommandName = "interface_name";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, NULL, 16, true));
    command.pfCommandHelp = 
        "The name of the interface that the eSTB SNMP Agent will use.\n"
        "Setting this to 'any' will cause the Agent to listen on all interfaces.\n";
    command.pfExamples = 
        "interface_name eth1";
    AddCommand(command);
    
    // Try to get the singleton instance of the settings object, and add it as
    // the one-and-only instance to me.
    BcmNonVolSettings *pSingleton = BcmSnmpNonVolSettings::GetSingletonInstance();

    if (pSingleton != NULL)
    {
        AddInstance("SNMP NonVol Settings", pSingleton);
    }
    else
    {
        gLogMessageRaw 
            << "BcmSnmpNonVolSettingsCommandTable::BcmSnmpNonVolSettingsCommandTable: ERROR - Singleton instance of BcmSnmpNonVolSettings is NULL!" << endl;
    }
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmSnmpNonVolSettingsCommandTable::~BcmSnmpNonVolSettingsCommandTable()
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
void BcmSnmpNonVolSettingsCommandTable::CommandHandler(void *pInstanceValue, 
                                                       const BcmCommand &command)
{
    // By convention, the settings object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmSnmpNonVolSettings *pSettings = (BcmSnmpNonVolSettings *) pInstanceValue;

    if (pSettings == NULL)
    {
        gLogMessageRaw << "BcmSnmpNonVolSettingsCommandTable::CommandHandler: ERROR - settings pointer is NULL!\n";

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
        case kChangeAllowConfiguration:
        {
            gLogMessageRaw << "AllowConfiguration = " << pSettings->AllowConfiguration() << "\n";
            
#if (BCM_FACTORY_SUPPORT)
            BcmFactoryNonVolSettings *pFactorySettings = BcmFactoryNonVolSettings::GetSingletonInstance();
            if (pFactorySettings)
              gLogMessageRaw << "(FYI, cdPrivateMibEnable = " << pFactorySettings->PrivateMibEnable() << ")\n";
#endif
            break;
        }

        case kSetVendorName:
            if (pParameter->WasParsed())
            {
                pSettings->VendorName(pParameter->AsString());
            }
            
            gLogMessageRaw << "Vendor Name = " << pSettings->VendorName() << "\n";
            break;

        case kSetSysDescr:
            if (pParameter->WasParsed())
            {
                pSettings->SysDescr(pParameter->AsString());
            }
            
            gLogMessageRaw << "sysDescr = " << pSettings->SysDescr() << "\n";
            break;

        case kSetSysObjectId:
            if (pParameter->WasParsed())
            {
                BcmObjectId oid;
                BcmString oidString = pParameter->AsString();

                // Validate this as a valid OID.
                if (oid.Set(oidString) == SNMP_NO_ERROR)
                {
                    pSettings->SysObjectId(pParameter->AsString());
                }
                else
                {
                    gLogMessageRaw << "ERROR - '" << pParameter->AsString() << "' is not a valid OID!\n";
                }
            }
            
            gLogMessageRaw << "sysObjectId = " << pSettings->SysObjectId() << "\n";
            break;

        case kSetSysContact:
            if (pParameter->WasParsed())
            {
                pSettings->SysContact(pParameter->AsString());
            }
            
            gLogMessageRaw << "sysContact = " << pSettings->SysContact() << "\n";
            break;

        case kSetSysName:
            if (pParameter->WasParsed())
            {
                pSettings->SysName(pParameter->AsString());
            }
            
            gLogMessageRaw << "sysName = " << pSettings->SysName() << "\n";
            break;

        case kSetSysLocation:
            if (pParameter->WasParsed())
            {
                pSettings->SysLocation(pParameter->AsString());
            }
            
            gLogMessageRaw << "sysLocation = " << pSettings->SysLocation() << "\n";
            break;

        case kSetSysORID:
            if (pParameter->WasParsed())
            {
                BcmObjectId oid;
                BcmString oidString = pParameter->AsString();

                // Validate this as a valid OID.
                if (oid.Set(oidString) == SNMP_NO_ERROR)
                {
                    pSettings->SysORID(pParameter->AsString());
                }
                else
                {
                    gLogMessageRaw << "ERROR - '" << pParameter->AsString() << "' is not a valid OID!\n";
                }
            }
            
            gLogMessageRaw << "sysORID.1 = " << pSettings->SysORID() << "\n";
            break;

        case kSetSysORDescr:
            if (pParameter->WasParsed())
            {
                pSettings->SysORDescr(pParameter->AsString());
            }
            
            gLogMessageRaw << "sysORDescr.1 = " << pSettings->SysORDescr() << "\n";
            break;

        case kSetDocsDevSwCurrentVers:
            if (pParameter->WasParsed())
            {
                pSettings->DocsDevSwCurrentVers(pParameter->AsString());
            }
            
            gLogMessageRaw << "docsDevSwCurrentVers = " << pSettings->DocsDevSwCurrentVers() << "\n";
            break;

        case kSetDocsDevSerialNumber:
            if (pParameter->WasParsed())
            {
                pSettings->DocsDevSerialNumber(pParameter->AsString());
            }
            
            gLogMessageRaw << "docsDevSerialNumber = " << pSettings->DocsDevSerialNumber() << "\n";
            break;

        case kSetSysServices:
            if (pParameter->WasParsed())
            {
                pSettings->SysServices((uint8) pParameter->AsUnsignedInt());
            }
            
            gLogMessageRaw << "sysServices = " << inHex(pSettings->SysServices()) << "\n";
            break;

        case kDiagAgentDisabledPostReg:
            if (pParameter->WasParsed())
            {
                pSettings->DiagAgentDisabledPostReg(pParameter->AsBool());
            }

            gLogMessageRaw << "Diag agent disabled post-registration = " << pSettings->DiagAgentDisabledPostReg() << "\n";
            break;
            
        case kHideIpStackIfEntries:
            if (pParameter->WasParsed())
            {
                pSettings->HideIpStackIfEntries(pParameter->AsBool());
            }

            gLogMessageRaw << "IP stack ifTable entries hidden = " << pSettings->HideIpStackIfEntries() << "\n";
            break;
            
        case kInterfaceName:
            if (pParameter->WasParsed())
            {
                pSettings->InterfaceName(pParameter->AsString());
            }

            gLogMessageRaw << "eSTB SNMP Agent interface name = " << pSettings->InterfaceName() << "\n";
            break;
            
        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }
}



