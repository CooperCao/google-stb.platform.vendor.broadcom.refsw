//****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       EstbSnmpAgentConsole.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  22 MAY 2006
//
//****************************************************************************
//  Description:
//  InstallCommands and CommandHandler methods for EstbSnmpAgent class.
//
//****************************************************************************

//********************** Include Files ***************************************

#include "EstbSnmpAgent.h"

#include "IpAddressCommandParameter.h"
#include "StringCommandParameter.h"
#include "StringSetCommandParameter.h"

#include "SnmpLog.h"
// #include "PktFilterController.h"
// #include "CmDocsisNonVolSettings.h"


// The command ids for the commands I can handle.
enum
{
    // The first command here is from SnmpV3AgentConsole.cpp.  This is awkward, 
    // should be in a header file...
    kInstallGroup   /* Add a standard DOCSIS 1.1 USM group */ = BcmNonVolSettingsCommandTable::kDerivedClassCommands + 100,
    kFilters        /* enable/disable MIB packet filters */ = BcmNonVolSettingsCommandTable::kDerivedClassCommands + 200,
    kLoadConfig     // load a config file
};

// Constants for the command parameters.
enum
{
    kPrimaryParameter = 1,
    kSecondParameter,
    kThirdParameter,
    kFourthParameter
};

/* String set values: */

// kFilters
enum
{
  kFiltersOn = 0,
  kFiltersOff,
  kFiltersOptimized
};


void EstbSnmpAgent::InstallCommands ()
{
  static bool CommandsInstalled = false;
  
  // Only install commands once.
  if (CommandsInstalled == true)
    return;
    
  CommandsInstalled = true;
  
  // Run the base class implementation first.
  DefaultSnmpAgentClass::InstallCommands();

  BcmCommand command;
    
  // Set up fields that don't change.
  command.fIsEnabled = true;
  command.fIsHidden = false;
  command.fCommandFunctionWithParms = GetCommandTableSingleton().CommandHandler;

  command.fCommandId = kLoadConfig;
  command.pfCommandName = "estb_cfgfile";
  command.pfCommandParameterList = new BcmCommandParameterList;
  // command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kPrimaryParameter, true, "IP"));
  command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kSecondParameter, true, "path", 255));
  command.pfCommandHelp = "Load the specified config file.  If the path is not specified, "
                          "then the settings from SNMP NV / cfg_file will be used.\n";
  command.pfExamples = "estb_cfgfile /home/broadcom/cu.cfg\n";
  GetCommandTableSingleton().AddCommand(command);  
}


void EstbSnmpAgent::CommandHandler (const BcmCommand &command)
{
    bool Handled = true;
    const BcmCommandParameter *pParameter = NULL;
    
    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        // overload base class implementation of kInstallGroup to force
        // coexistence mode.
        case kInstallGroup:
        {
            fSnmpMode = SNMP_MODE_COEXISTENCE;
            
            // Indicate we have not handled this command so we'll still call
            // the base class.
            Handled = false;
            break;
        }
        
        case kLoadConfig:
        {

            const BcmCommandParameter *pPathParam = command.pfCommandParameterList->Find(kSecondParameter);
            BcmString Path = "";
            
            if  ( pPathParam->WasParsed() )
              Path = pPathParam->AsString();
              
            EstbSnmpAgent::Singleton().LoadConfigFile (Path.c_str());
            
            break;
        }

        default:
            Handled = false;
            break;
    }
    
    // Run the base class implementation if we have not handled the command.
    if (Handled == false)
      DefaultSnmpAgentClass::CommandHandler(command);
}
