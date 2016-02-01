//****************************************************************************
//
//  Copyright (c) 2003  Broadcom Corporation
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
//  Filename:       BcmSnmpAgentConsole.cpp
//  Author:         Kevin O'Neal
//  Creation Date:  October 3, 2003
//
//****************************************************************************
//  Description:
//  InstallCommands and CommandHandler methods for BcmSnmpAgent class.
//
//****************************************************************************

//********************** Include Files ***************************************

#include "SnmpV3Agent.h"

#include "StringCommandParameter.h"

#include "SnmpLog.h"


// The command ids for the commands I can handle.
enum
{
    kInstallGroup   /* Add a standard DOCSIS 1.1 USM group */ = BcmNonVolSettingsCommandTable::kDerivedClassCommands + 100,
    kNotifySetup,	// Simulate Notify TLVs in config file.
    kKerbTest       // start a Kerberos/SNMPv3 test for debug
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


void BcmSnmpV3Agent::InstallCommands ()
{
  static bool CommandsInstalled = false;
  
  // Only install commands once.
  if (CommandsInstalled == true)
    return;
    
  CommandsInstalled = true;
  
  // Run the base class implementation first.
  BcmSnmpAgent::InstallCommands();
  
  BcmCommand command;
    
  // Set up fields that don't change.
  command.fIsEnabled = true;
  command.fIsHidden = false;
  command.fCommandFunctionWithParms = GetCommandTableSingleton().CommandHandler;
  
  command.fCommandId = kInstallGroup;
  command.pfCommandName = "install_group";
  command.pfCommandParameterList = new BcmCommandParameterList;
  command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, false, "vacmGroupName"));
  command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kSecondParameter, false, "dhPublicKey"));
  command.pfCommandHelp = "Install one of the standard DOCSIS 1.1 groups. Supported groups are:\n\n"
                          "\tdocsisManager\n"
                          "\tdocsisOperator\n"
                          "\tdocsisMonitor\n"
                          "\tdocsisUser\n\n"
                          "vacmGroupName is one of the above names.  dhPublicKey is any old "
                          "text string to use as the public key (no spaces).\n"
                          "NOTE: Only SnmpV3Agent objects support this command.";
  command.pfExamples = "install_group docsisManager broadcom\t";
  GetCommandTableSingleton().AddCommand(command);

  command.fCommandId = kNotifySetup;
  command.pfCommandName = "notify_setup";
  command.pfCommandHelp = "Setup Notify Mibs to enable SNMPv3 Notify.  "
                          "Uses default entries.\n"
                          "NOTE: Only SnmpV3Agent objects support this command.";;
  command.pfExamples = "notify_setup\t";
  GetCommandTableSingleton().AddCommand(command);
}


void BcmSnmpV3Agent::CommandHandler (const BcmCommand &command)
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
        case kInstallGroup:
        {
            const BcmCommandParameter *pKeyParam = command.pfCommandParameterList->Find(kSecondParameter);
            const char *pGroupName = pParameter->AsString();
            const char *pDhPublicKey = pKeyParam->AsString();;
        
        
            if ( AgentInstallProvisionedView ((char *)pGroupName, (unsigned char *)pDhPublicKey, strlen (pDhPublicKey)) == SNMP_NO_ERROR )
                SnmpLogRaw << "Installed provisioned key " << pGroupName << " OK." << endl;
            else
                SnmpLogRaw << "Failed to install provisioned key " << pGroupName << endl;
            
            break;
        }

        case kNotifySetup:
        {
            // ProvisionNotifyEntry should be called with each TLV 38 parameter set.
            // However, It should only be called if any TLV 34 has been encountered
            // in the config file.
          
            // - ElementName should be a concat of the string "@config_n"
            // where n is from 0 to the number of elements (config file entries).
            
            BcmSnmpV3Agent *pV3Agent = BcmSnmpThread::Singleton().FindV3Agent (this);
            if (pV3Agent == NULL)
            {
              SnmpLogRaw << GetName() << " is not an SNMPv3 agent" << endl;
              return;
            }
        
            int TrapType, Port, TimeOut, Retries;
            const BcmString ElementName("@config_0");     //MUST increment for each
            const BcmIpAddress TargetIp(10,24,65,36);    //TLV 38.1
            Port     = 162; //TLV 38.2
            TrapType = 5;   //TLV 38.3
            TimeOut  = 4000; //TLV 38.4
            Retries  = 1;   //TLV 38.5	       
            //unsigned char filter[] = {6,9,43,6,1,6,3,1,1,5,3}; // generic link down trap
            unsigned char filter[] = {6,8,43,6,1,6,3,1,1,5}; // generic link down trap
        
            BcmOctetBuffer Filter(filter,sizeof(filter),sizeof(filter) ,false);  //TLV 38.6
            const BcmString SecurityName("@config"); //TLV 38.7
        
            if (pV3Agent->ProvisionNotifyEntry (TrapType,ElementName,
                                               SecurityName,TargetIp,
                                               Port,TimeOut,Retries,Filter) )
                SnmpLogRaw<<"Failed to provision Notify MIBs"<<endl;
            else
                SnmpLogRaw<<"Notify MIBs provisioned OK"<<endl;   

            break;
        }

        default:
            Handled = false;
            break;
    }
    
    // Run the base class implementation if we have not handled the command.
    if (Handled == false)
      BcmSnmpAgent::CommandHandler(command);
}
