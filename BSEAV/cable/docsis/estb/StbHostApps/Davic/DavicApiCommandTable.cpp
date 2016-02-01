//****************************************************************************
//
// Copyright (c) 2006-2011 Broadcom Corporation
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
//  Filename:       DavicApiCommandTable.cpp
//  Author:         Pinar Taskiran-Cyr
//  Creation Date:  July 22, 2011
//
//****************************************************************************
//  Description:
//      Sub-command table for testing of DAVIC API on eCM side.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DavicApiCommandTable.h"
#include "CmDavicApiSocketInterface.h"
#include "davic_api.h"

#include "BitmaskCommandParameter.h"
#include "BoolCommandParameter.h"
#include "FlagCommandParameter.h"
#include "IpAddressCommandParameter.h"
#include "StringCommandParameter.h"
#include "StringSetCommandParameter.h"
#include "UnsignedIntCommandParameter.h"
#include "MacAddressCommandParameter.h"

#include "OperatingSystemFactory.h"



//********************** Local Types *****************************************

//********************** Local Constants *************************************

static const char *pgTableDescr = "Commands to exercise DAVIC API.";


// Constants for the commands.
// Constants for the commands.
enum
{
	kDavicApiStart,                // Starts the Davic App.
	kDavicApiStop,					// Stops Davic App. Transitions to init state.
	/*(kDavicStartInteractive,
	kSetOperMode,*/
	kDavicApiOpenTunnel,
	/*kInstallStatusCallback,*/
	kDavicApiSetConfig,
	kDavicApiGetConfig
	/*

	kDsScanOk*/
};


// Constants for the command parameters.
enum
{
    kPrimaryParameter = 1,
    kSecondParameter,
    kThirdParameter,
    kFourthParameter,
    kImageFlag,
    kSecureFlag,
    kLargeImageFlag,
    kFilenameParameter,
    kForceImageFlag,        
};


//********************** Local Variables *************************************
//********************** Globals *********************************************
extern "C"
{

}

//********************** Local Functions *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Creates the command table, adds all of the
// commands to it and sets the command handler function.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmDavicApiCommandTable::BcmDavicApiCommandTable(void) :
    BcmCommandTable("CM DAVIC API Commands", "davic_ctl", "DavicCtl", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fIsHidden = false;
    command.fCommandFunctionWithParms = CommandHandler;

    command.fCommandId = kDavicApiStart;
    command.pfCommandName = "start";
    command.pfCommandHelp = "During normal DAVIC bring up, this API is called first so that "
							"the DAVIC implementation can start the sign-on process";
    command.pfExamples = "	";
    AddCommand(command);

    command.fCommandId = kDavicApiStop;
    command.pfCommandName = "stop";
    command.pfCommandHelp = "Stop all frequency scanning or locks. Place DAVIC into an init state. "
							"The DAVIC should remain in the init state until DavicStart() is called. "
							"While in the state all other API shall be ignored.\n";
    command.pfExamples = "stop	";
    AddCommand(command);
/*
    command.fCommandId = kDavicStartInteractive;
    command.pfCommandName = "start_interactive";
    command.pfCommandHelp = "Send the sign-on request message after getting the provision message and attempt to go two-way. " 
							"This API shall return an error if DavicStartBoot() was not called previously.\n";
    command.pfExamples = "start_interactive ";
    AddCommand(command);

    command.fCommandId = kSetOperMode;
    command.pfCommandName = "set_oper_mode";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, false, "oper_mode"));
    command.pfCommandHelp = "Set the operational mode of the DAVIC implementation. " 
							"This should be called before StartBoot(). However, DAVIC "
							"will default to the DavicStandardOpMode. The CableCARD mode "
							"shall be called when the DAVIC implementation connects to the cableCARD device. "
							"The Q55-1 mode is currently unsupported.";
    command.pfExamples = "	0:kDAVICStandardOpMode 1:kCCQPSKMode 2:kDQ551Mode";
    AddCommand(command);
*/
    command.fCommandId = kDavicApiOpenTunnel;
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, false, "vpivci_pair"));
    command.pfCommandName = "open_tunnel";
    command.pfCommandHelp = "Opens an OOB tunnel. VPI: bits 16-23  VCI: bits 0-15";
    command.pfExamples = "open_tunnel 0x00000FA0";
    AddCommand(command);
/*
    command.fCommandId = kInstallStatusCallback;
    command.pfCommandName = "instl_status_callback";
    command.pfCommandHelp = "Installs the TEST callback for DAVIC to report status events. This API should only "
							"be called one time. Any subsequent calls shall replace the previously installed "
							"callback function without error.\n";							

    command.pfExamples = "instl_status_callback";
    AddCommand(command);
*/
    command.fCommandId = kDavicApiSetConfig;
    command.pfCommandName = "set_config";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, false, "providerIdCheck"));
    command.pfCommandHelp = "Exercises ALL parameters in BcmDavicSetConfig() API. \n";
							
																										
    command.pfExamples = "set_param 0  sets providerIdCheck to 0\n";
    AddCommand(command);

    command.fCommandId = kDavicApiGetConfig;
    command.pfCommandName = "get_config";
    command.pfCommandHelp = "Exercises ALL parameters in BcmDavicGetConfig() API. "
							"BcmDavicGetConfig(): Get config settings from the DAVIC implementation. The maxDataLen shall "
							"be checked to make sure it is of the same length or greater than the data being get. "
							"If not, an error should be returned. A successful get should return the length of the get data\n";
    command.pfExamples = "get_param";
    AddCommand(command);


}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDavicApiCommandTable::~BcmDavicApiCommandTable()
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
void BcmDavicApiCommandTable::CommandHandler(void *pInstanceValue, 
                                             const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = NULL;

    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        case kDavicApiStart:
            {
				BcmDavicStart();
            }
            break;

        case kDavicApiStop:
            {
				BcmDavicStop();
            }
            break;
/*
        case kDavicStartInteractive:
            {
				DavicStartInteractive();
            }
            break;
*/
        case kDavicApiOpenTunnel:
            {
                uint32 vpivci = pParameter->AsUnsignedInt();

				gLogMessageRaw << "VPIVCI = " << hex << vpivci << endl;
				BcmDavicOpenTunnel(vpivci);
            }
            break;        

  /*
        case kInstallStatusCallback:
            {
				uint32 userHandle;
				if(!DavicInstallStatusCallback((void*) &userHandle, DavicStatus))
					gLogMessageRaw << "Status callback installed!" << endl;
				else
					gLogMessageRaw << "Failed to install status callback!" << endl;				
            }
            break;
*/
        case kDavicApiSetConfig:
            {
				uint32 checkProviderId = pParameter->AsUnsignedInt();

				DavicSetConfigTest(checkProviderId);
            }
            break;

        case kDavicApiGetConfig:
            {				
				DavicConnectionSettings davicConnectionSettings;
				memset(&davicConnectionSettings, 0, sizeof(davicConnectionSettings));
				if( BcmDavicGetConfig( kDavicGetDefaultConnSets, &davicConnectionSettings, sizeof(davicConnectionSettings) ) == -1)
				{
					cout <<" Error: BcmDavicGetConfig: Failed to get parameters!" << endl;
				}
				BcmOctetBuffer buf(sizeof(davicConnectionSettings));
				buf.AddToEnd((uint8*)&davicConnectionSettings, sizeof(davicConnectionSettings));
				cout << "  davicConnectionSettings = " << buf << endl;
            }
            break;

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }

}


/* TEST functions */
void BcmDavicApiCommandTable::DavicSetConfigTest(unsigned int checkProviderId)
{
	uint32 providerID = 0x00000E00;	
	BcmDavicSetConfig(kDavicSetParamProviderIDValue, (void*) &providerID, sizeof(providerID)); 

	int32 providerIDCheck = (int32) checkProviderId;
	BcmDavicSetConfig(kDavicSetParamProviderIDCheck, (void*) &providerIDCheck, sizeof(providerIDCheck)); 

	uint8 nsapAddress[20] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xfa, 0x0fb, 0x0fc, 0xfd, 0xfe };
	BcmDavicSetConfig(kDavicSetNSAPAddr, (void*) nsapAddress, sizeof(nsapAddress)); 

	int32 nsapAddressEnabled = 1;
	BcmDavicSetConfig(kDavicSetParamNSAPTxEnb, (void*) &nsapAddressEnabled, sizeof(nsapAddressEnabled)); 
}