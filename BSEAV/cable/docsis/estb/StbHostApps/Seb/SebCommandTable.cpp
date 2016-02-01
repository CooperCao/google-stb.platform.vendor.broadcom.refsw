//****************************************************************************
//
// Copyright (c) 2006-2012 Broadcom Corporation
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
//  Filename:       SebCommandTable.cpp
//  Author:         Pinar Taskiran-Cyr
//  Creation Date:  July 22, 2011
//
//****************************************************************************
//  Description:
//      Command table for testing of SEB API on eCM side.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "SebCommandTable.h"

#include "BitmaskCommandParameter.h"
#include "BoolCommandParameter.h"
#include "FlagCommandParameter.h"
#include "IpAddressCommandParameter.h"
#include "StringCommandParameter.h"
#include "StringSetCommandParameter.h"
#include "UnsignedIntCommandParameter.h"
#include "MacAddressCommandParameter.h"

#include "OperatingSystemFactory.h"

#if SEB_API_FUNC_IF
#include "EcmSebApiFunctionInterface.h"
#else
#include "EcmSebApiSocketInterface.h"
#endif
  
//********************** Local Types *****************************************
  
//********************** Local Constants *************************************

static const char *pgTableDescr = ".";


// Constants for the commands.
// Constants for the commands.
enum
{
	kTestSebApiInit,
	kTestSebClientConnect,
	kTestSebClientAddDevice,
	kTestSebClientRemove,
	kTestSebClientJoin,
	kTestSebClientLeave,
	kTestSebFwdData,
	kTestSebClientTunnelAcqComplete,
	kTestSebClientTunnelLost,
	kSendCannedDhcpOffer,
	kSendCannedDhcpAck,
	kJoinMcast
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
static int sock_ctl_in = 0;
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
SebBfcCommandTable::SebBfcCommandTable(void) :
    BcmCommandTable(" SEB API Commands", "seb_ctl", "SebCtl", pgTableDescr)
{
    BcmCommand command;

    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fIsHidden = true;
    command.fCommandFunctionWithParms = CommandHandler;

	// kTestSebApiInit
    command.fCommandId = kTestSebApiInit;
	command.fIsHidden = false;
    command.pfCommandName = "init_api";
    command.pfCommandHelp = "Sets up the seb socket interface ";
    command.pfExamples = "	";
    AddCommand(command);

	// kTestSebClientConnect
    command.fCommandId = kTestSebClientConnect;
	command.fIsHidden = false;
    command.pfCommandName = "add_client";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmMacAddressCommandParameter(kPrimaryParameter, true));
    command.pfCommandHelp = "add_client 00:10:18:11:22:33 "
							"";
    command.pfExamples = "	";
    AddCommand(command);

    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fIsHidden = true;
    command.fCommandFunctionWithParms = CommandHandler;

    command.fCommandId = kTestSebClientRemove;
	command.fIsHidden = false;
    command.pfCommandName = "remove_client";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmMacAddressCommandParameter(kPrimaryParameter, true));
    command.pfCommandHelp = "remove_client 00:10:18:11:22:33 "
							"";
    command.pfExamples = "	";
    AddCommand(command);


	// kTestSebClientJoin
    command.fCommandId = kTestSebClientJoin;
	command.fIsHidden = false;
    command.pfCommandName = "join_client";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kPrimaryParameter, true));
	command.pfCommandParameterList->AddOrderDependent(new BcmMacAddressCommandParameter(kSecondParameter, true));
    command.pfCommandHelp = "join_client 234.12.12.187 00:10:18:aa:bb:cc "
							"";
    command.pfExamples = "	";
    AddCommand(command);

	// kJoinMcast
    command.fCommandId = kJoinMcast;
	command.fIsHidden = false;
    command.pfCommandName = "join_mcast";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kPrimaryParameter, true));
    command.pfCommandHelp = "join_mcast 234.12.12.187  "
							"";
    command.pfExamples = "	";
    AddCommand(command);

	// kTestSebClientLeave
    command.fCommandId = kTestSebClientLeave;
	command.fIsHidden = false;
    command.pfCommandName = "leave_client";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kPrimaryParameter, true));
	command.pfCommandParameterList->AddOrderDependent(new BcmMacAddressCommandParameter(kSecondParameter, true));
    command.pfCommandHelp = "leave_client 234.12.12.187 00:10:18:aa:bb:cc "
							"";
    command.pfExamples = "	";
    AddCommand(command);

	// kTestSebFwdData
    command.fCommandId = kTestSebFwdData;
	command.fIsHidden = false;
    command.pfCommandName = "fwd_data_to_if21";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmMacAddressCommandParameter(kPrimaryParameter, true));
    command.pfCommandHelp = "Sends a canned ethernet packet to IF 21.\n"
							"";
    command.pfExamples = "fwd_data_to_if21	";
    AddCommand(command);

	// kTestSebClientTunnelAcqComplete
    command.fCommandId = kTestSebClientTunnelAcqComplete;
	command.fIsHidden = false;
    command.pfCommandName = "tunnel_acq_complete";
    command.pfCommandHelp = "Sends tunnel_acq_complete event to eCM ";
    command.pfExamples = "	";
    AddCommand(command);

	// kTestSebClientTunnelLost
    command.fCommandId = kTestSebClientTunnelLost;
	command.fIsHidden = false;
    command.pfCommandName = "sebc_tunnel_lost";
    command.pfCommandHelp = "Sends kSebcSebTunnelLost event to eCM ";
    command.pfExamples = "	";
    AddCommand(command);

	// kSendCannedDhcpOffer
    command.fCommandId = kSendCannedDhcpOffer;
	command.fIsHidden = true;
    command.pfCommandName = "offer_dhcp_to_if21";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, "Xid"));
    command.pfCommandHelp = "Sends a canned DHCP OFFER to IF 21.\n"
							"";
    command.pfExamples = "offer_dhcp_to_if21	";
    AddCommand(command);

	// kSendCannedDhcpAck
    command.fCommandId = kSendCannedDhcpAck;
	command.fIsHidden = true;
    command.pfCommandName = "ack_dhcp_to_if21";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, "Xid"));
    command.pfCommandHelp = "Sends a canned DHCP ACK to IF 21.\n"
							"";
    command.pfExamples = "ack_dhcp_to_if21	";
    AddCommand(command);

}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
SebBfcCommandTable::~SebBfcCommandTable()
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
void SebBfcCommandTable::CommandHandler(void *pInstanceValue, 
                                             const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = NULL;

    // By convention, the settings object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    //BcmCmDavicCtlThread *pCmDavicCtlThread = (BcmCmDavicCtlThread *) pInstanceValue;
    
    // Validate this just for sanity.
    /*if (pCmDavicCtlThread == NULL)
    {
        gLogMessageRaw 
            << "SebBfcCommandTable::CommandHandler:  ERROR - "
            << "User value (pointer to CM DAVIC Ctl Thread instance) is NULL!  Can't process the command!\n";

        return;
    }*/

    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        case kTestSebApiInit:
			{
				gLogMessageRaw 
					<< "ATTENTION: Do not initialize SEB API sockets if they are previously initialized by a standalone eSTB SEB application." << endl;

				SEB_CALLBACK sebClientCallback;
				memset(&sebClientCallback, 0, sizeof(sebClientCallback));
				sebClientCallback.SebRxTunnelDataHandler = ClientSebRxTunnelDataHandler;
				sebClientCallback.SebStatusDataHandler = ClientSebStatusDataHandler;
				BcmSebApiInit(&sebClientCallback);
			}
			break;

        case kTestSebClientConnect:
            {
				// MAC ADDRESS
				uint8 sebClMac[6] = {0x01, 0x00, 0x5e, 0x0a, 0x0a, 0x65};
				SebClientConnectMsg clientConnectData;
				memset(&clientConnectData, 0, sizeof(clientConnectData));

				BcmMacAddress Mac;              
				if (pParameter->WasParsed())
				{
					Mac = pParameter->AsMacAddress();
					Mac.getAddr(clientConnectData.sebConnectedDeviceMacAddress);
				}
				else
				{
					gLogMessageRaw 
						<< "No MAC address parameter specified. Using default 01:0x00:0x5e:0x0a:0x0a:0x65" << endl;
					memcpy(clientConnectData.sebConnectedDeviceMacAddress, sebClMac, sizeof(sebClMac));
				}
				
				// IP ADDR TYPE
				BcmIpAddress sebIpAddress(0x11, 0x22, 0x33, 0x44);
				uint8 sebConnIpAddress[16];												
				sebIpAddress.Get(sebConnIpAddress, sizeof(sebConnIpAddress));
				uint32 ipAddrType = 0;
				if(sebIpAddress.IsIpV6Address())
					clientConnectData.sebConnectedDeviceIpAddrType = 6;
				else
					clientConnectData.sebConnectedDeviceIpAddrType = 4;
	
				// IP ADDRESS
				memcpy(clientConnectData.sebConnectedDeviceIpAddr, sebConnIpAddress, sizeof(sebConnIpAddress));				

				// DEVICE TYPE
				clientConnectData.sebConnectedDeviceType = kSebEcm;
				BcmSebClientConnect(&clientConnectData);
            }
            break;

        case kTestSebClientRemove:
            {
				// MAC ADDRESS
				uint8 defSebClMac[6] = {0x01, 0x00, 0x5e, 0x0a, 0x0a, 0x65};
				uint8 sebClMac[6];

				BcmMacAddress Mac;              
				if (pParameter->WasParsed())
				{
					Mac = pParameter->AsMacAddress();
					Mac.getAddr(sebClMac);
					BcmSebClientRemoveDevice(sebClMac);
				}
				else
				{
					gLogMessageRaw 
						<< "No MAC address parameter specified. Using default 01:0x00:0x5e:0x0a:0x0a:0x65" << endl;
					BcmSebClientRemoveDevice(defSebClMac);
				}
            }
            break;

        case kTestSebClientJoin:
            {
				// Create API cargo.
				SebClientJoinMsg clientJoinMsg;
				memset(&clientJoinMsg, 0, sizeof(clientJoinMsg));

				// Default MAC/IP storage.
				BcmMacAddress multicastMac(0x01, 0x00, 0x5e, 0x0c, 0x0c, 0xbb);
				BcmIpAddress multicastIp(0xea, 0x0c, 0x0c, 0xbb);				

				// Read user input.
				if (pParameter->WasParsed())
				{
					multicastIp = pParameter->AsIpAddress();
					MapMcastIpAddressToMacAddress(multicastIp, &multicastMac);

					gLogMessageRaw 
						<< multicastIp <<" -> " << multicastMac << endl;			
				}
				else
				{
					gLogMessageRaw 
						<< "No multicast IP address parameter specified. Using default 234.12.12.187" << endl;					
				}

				// Populate cargo
				multicastMac.getAddr(clientJoinMsg.multicastDestMacAddress);

				// MAC ADDRESS
				//uint8 sebClMulticastMac[6] = {0x01, 0x00, 0x5e, 0x0a, 0x0a, 0x65};
				

				/*           
				if (pParameter->WasParsed())
				{
					Mac = pParameter->AsMacAddress();
					Mac.getAddr(clientJoinMsg.multicastDestMacAddress);
				}
				else
				{
					gLogMessageRaw 
						<< "No multicast MAC address parameter specified. Using default 01:0x00:0x5e:0x0a:0x0a:0x65" << endl;
					memcpy(clientJoinMsg.multicastDestMacAddress, sebClMulticastMac, sizeof(sebClMulticastMac));
				}*/
				
				// Default storage.
				BcmMacAddress Mac;   
				uint8 sebClMac[6] = {0x00, 0x10, 0x18, 0x0a, 0x0a, 0x65};

				// Read user input. Populate cargo.
				const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);
				if (pParameter2->WasParsed())
				{
					Mac = pParameter2->AsMacAddress();
					Mac.getAddr(clientJoinMsg.sebClientMacAddress);
				}
				else
				{
					gLogMessageRaw 
						<< "No SEBC MAC address parameter specified. Using default 01:0x00:0x10:0x18:0x0a:0x65" << endl;
					memcpy(clientJoinMsg.sebClientMacAddress, sebClMac, sizeof(sebClMac));
				}					

				// API
				BcmSebClientJoin(&clientJoinMsg);

				//
				gLogMessageRaw 
					<< "ATTENTION!! Use join_mcast command to start a multicast session for this IP address: " << multicastIp << endl;
				
				//SebTestJoinMulticastSession(multicastIp.Get());
            }
            break;

        case kTestSebClientLeave:
            {
				SebClientLeaveMsg clientLeaveMsg;
				memset(&clientLeaveMsg, 0, sizeof(clientLeaveMsg));

				BcmMacAddress multicastMac(0x01, 0x00, 0x5e, 0x0c, 0x0c, 0xbb);
				BcmIpAddress multicastIp(0xea, 0x0c, 0x0c, 0xbb);				

				if (pParameter->WasParsed())
				{
					multicastIp = pParameter->AsIpAddress();
					MapMcastIpAddressToMacAddress(multicastIp, &multicastMac);

					gLogMessageRaw 
						<< multicastIp <<" -> " << multicastMac << endl;	
				}
				else
				{
					gLogMessageRaw 
						<< "No multicast IP address parameter specified. Using default 234.12.12.187" << endl;					
				}

				//
				multicastMac.getAddr(clientLeaveMsg.multicastDestMacAddress);

				// MAC ADDRESS
				//uint8 sebClMulticastMac[6] = {0x01, 0x00, 0x5e, 0x0a, 0x0a, 0x65};

				/*BcmMacAddress Mac;              
				if (pParameter->WasParsed())
				{
					Mac = pParameter->AsMacAddress();
					Mac.getAddr(clientLeaveMsg.multicastDestMacAddress);
				}
				else
				{
					gLogMessageRaw 
						<< "No multicast MAC address parameter specified. Using default 01:0x00:0x5e:0x0a:0x0a:0x65" << endl;
					memcpy(clientLeaveMsg.multicastDestMacAddress, sebClMulticastMac, sizeof(sebClMulticastMac));
				}*/
				
				BcmMacAddress Mac;   
				uint8 sebClMac[6] = {0x00, 0x10, 0x18, 0x0a, 0x0a, 0x65};
				const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);
				if (pParameter2->WasParsed())
				{
					Mac = pParameter2->AsMacAddress();
					Mac.getAddr(clientLeaveMsg.sebClientMacAddress);
				}
				else
				{
					gLogMessageRaw 
						<< "No SEBC MAC address parameter specified. Using default 01:0x00:0x10:0x18:0x0a:0x65" << endl;
					memcpy(clientLeaveMsg.sebClientMacAddress, sebClMac, sizeof(sebClMac));
				}					

				BcmSebClientLeave(&clientLeaveMsg);
            }
            break;

			
        case kTestSebFwdData:
            {
				uint8 sebdata[] = 
				{
					0x01, 0x00, 0x5e, 0x0c, 0x0c, 0xbb, 0x00, 0xdd, 0xaa, 0xbb, 0xee, 0xcc, 0x08, 0x00, 0x45, 0x00,  
					0x00, 0x32, 0xba, 0xbe, 0x80, 0x00, 0xff, 0x11, 0x8a, 0x34, 0x00, 0x00, 0x00, 0x00, 0xea, 0x0c, 
					0x0c, 0xbb, 0x04, 0x00, 0x88, 0x89, 0x00, 0x1e, 0x1c, 0x01, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab,  
					0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab, 0xab
				};

				BcmMacAddress Mac;   
				if (pParameter->WasParsed())
				{
					Mac = pParameter->AsMacAddress();
					Mac.getAddr(&sebdata[6]);
				}

				BcmSebTxTunnelPktToIf21( sebdata, sizeof(sebdata) );
				//EstbToEcmSocketSendSebData( sizeof(sebdata), sebdata, false, false );
			}
			break;

        case kTestSebClientTunnelAcqComplete:
            {
				SebcSebTunnelSettings sebcSebTunnelSettings;
				memset(&sebcSebTunnelSettings, 0, sizeof(SebcSebTunnelSettings));

				{
					// MAC ADDRESS
					/*uint8 sebClMac[6] = {0x00, 0x10, 0x18, 0xDD, 0xEE, 0xFF};

					BcmMacAddress Mac;              
					if (pParameter->WasParsed())
					{
						Mac = pParameter->AsMacAddress();
						Mac.getAddr(sebcSebTunnelSettings.sebConnectedDeviceMacAddress);
					}
					else
					{
						gLogMessageRaw 
							<< "No MAC address parameter specified. Using default 0x00, 0x10, 0x18, 0xDD, 0xEE, 0xFF" << endl;
						memcpy(sebcSebTunnelSettings.sebConnectedDeviceMacAddress, sebClMac, sizeof(sebClMac));
					}*/
					
					// SEB tunnel client IP ADDR TYPE
					BcmIpAddress sebTunnelClientIpAddress(0x11, 0x22, 0x33, 0x44);
					//uint8 sebConnIpAddress[4];												
					//sebTunnelClientIpAddress.Get(sebConnIpAddress, sizeof(sebConnIpAddress));
					uint32 ipAddrType = 0;
					if(sebTunnelClientIpAddress.IsIpV6Address())
						sebcSebTunnelSettings.sebTunnelClientIpAddrType = 6;
					else
						sebcSebTunnelSettings.sebTunnelClientIpAddrType = 4;
		
					// IP ADDRESS
					//memcpy(sebcSebTunnelSettings.sebTunnelClientIpAddr, sebConnIpAddress, sizeof(sebConnIpAddress));				
					sebTunnelClientIpAddress.Get(sebcSebTunnelSettings.sebTunnelClientIpAddr);

					// SEB Tunnel server IP ADDR TYPE
					BcmIpAddress sebTunnelServerIpAddress(0x55, 0x66, 0x77, 0x88);
					//sebConnIpAddress[16];												
					//sebTunnelServerIpAddress.Get(sebConnIpAddress, sizeof(sebConnIpAddress));
					ipAddrType = 0;
					if(sebTunnelServerIpAddress.IsIpV6Address())
						sebcSebTunnelSettings.sebTunnelServerIpAddrType = 6;
					else
						sebcSebTunnelSettings.sebTunnelServerIpAddrType = 4;
		
					// IP ADDRESS
					//memcpy(sebcSebTunnelSettings.sebTunnelServerIpAddr, sebConnIpAddress, sizeof(sebConnIpAddress));	
					sebTunnelServerIpAddress.Get(sebcSebTunnelSettings.sebTunnelServerIpAddr);

					// ports
					sebcSebTunnelSettings.sebTunnelClientPort = 0xDEAD;
					sebcSebTunnelSettings.sebTunnelServerPort = 0xBEEF;

					// DEVICE TYPE
					//sebcSebTunnelSettings.sebConnectedDeviceType = kSebEcm;
				}

				BcmSebcTunnelAcquisitionComplete(&sebcSebTunnelSettings);
			}
			break;

        case kTestSebClientTunnelLost:
            {
				BcmSebcTunnelLost();
			}
			break;

        case kSendCannedDhcpOffer:
            {
				uint8 sebdata[] = 
				{
					0x02, 0x10, 0x18, 0xdb, 0x23, 0xb6, 0x00, 0x17, 0x10, 0x00, 0x53, 0x87, 0x08, 0x00, 0x45, 0x00,  
					0x01, 0x34, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11, 0x50, 0xe9, 0x0c, 0x1b, 0x01, 0x05, 0x06, 0x0d,  
					0x15, 0xa4, 0x00, 0x43, 0x00, 0x44, 0x01, 0x20, 0x34, 0x57, 0x02, 0x01, 0x06, 0x01, 0x69, 0xc3,  
					0x31, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x0d, 0x15, 0xa4, 0x0c, 0x1b,  
					0x01, 0x05, 0x06, 0x0d, 0x15, 0x01, 0x02, 0x10, 0x18, 0xdb, 0x23, 0xb6, 0x00, 0x00, 0x00, 0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x2f, 0x67, 0x6f, 0x6c,  
					0x64, 0x65, 0x6e, 0x2f, 0x62, 0x61, 0x73, 0x69, 0x63, 0x31, 0x31, 0x2e, 0x63, 0x66, 0x67, 0x00, 
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x82, 0x53, 0x63, 0x35, 0x01, 0x02, 0x36, 0x04, 0x0c,  
					0x1b, 0x01, 0x05, 0x33, 0x04, 0x00, 0x01, 0x51, 0x80, 0x01, 0x04, 0xff, 0xff, 0xff, 0x00, 0x02,  
					0x04, 0xff, 0xff, 0xc7, 0xc0, 0x03, 0x04, 0x06, 0x0d, 0x15, 0x01, 0x04, 0x04, 0x0c, 0x1b, 0x01,  
					0x05, 0xff 
				};  

				uint32 Xid = 0;
				
				if (pParameter->WasParsed())
				{
					Xid = pParameter->AsUnsignedInt();
				}

				memcpy(&sebdata[46], &Xid, sizeof(uint32));

				BcmSebTxTunnelPktToIf21( sebdata, sizeof(sebdata) );				
			}
			break;

        case kSendCannedDhcpAck:
            {
				uint8 sebdata[] = 
				{
					0x02, 0x10, 0x18, 0xdb, 0x23, 0xb6, 0x00, 0x17,   0x10, 0x00, 0x53, 0x87   ,0x08 ,0x00 ,0x45 ,0x00,  
					0x01, 0x34, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11,   0x50, 0xe9, 0x0c, 0x1b   ,0x01 ,0x05 ,0x06 ,0x0d,  
					0x15, 0xa4, 0x00, 0x43, 0x00, 0x44, 0x01, 0x20,   0x31, 0x25, 0x02, 0x01   ,0x06 ,0x01 ,0x69 ,0xc3,  
					0x32, 0x0c, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x06, 0x0d   ,0x15 ,0xa4 ,0x0c ,0x1b,  
					0x01, 0x05, 0x06, 0x0d, 0x15, 0x01, 0x02, 0x10,   0x18, 0xdb, 0x23, 0xb6   ,0x00 ,0x00 ,0x00 ,0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00   ,0x00 ,0x00 ,0x00 ,0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00   ,0x00 ,0x00 ,0x00 ,0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00   ,0x00 ,0x00 ,0x00 ,0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00   ,0x00 ,0x00 ,0x00 ,0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x6f,   0x6e, 0x66, 0x69, 0x67   ,0x2f ,0x67 ,0x6f ,0x6c,  
					0x64, 0x65, 0x6e, 0x2f, 0x62, 0x61, 0x73, 0x69,   0x63, 0x31, 0x31 ,0x2e   ,0x63 ,0x66 ,0x67 ,0x00, 
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00 ,0x00   ,0x00 ,0x00 ,0x00 ,0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00 ,0x00   ,0x00 ,0x00 ,0x00 ,0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00 ,0x00   ,0x00 ,0x00 ,0x00 ,0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00 ,0x00   ,0x00 ,0x00 ,0x00 ,0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00 ,0x00   ,0x00 ,0x00 ,0x00 ,0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00 ,0x00   ,0x00 ,0x00 ,0x00 ,0x00,  
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x82,   0x53, 0x63, 0x35 ,0x01   ,0x05 ,0x36 ,0x04 ,0x0c,  
					0x1b, 0x01, 0x05, 0x33, 0x04, 0x00, 0x01, 0x51,   0x80, 0x01, 0x04 ,0xff   ,0xff ,0xff ,0x00 ,0x02,  
					0x04, 0xff, 0xff, 0xc7, 0xc0, 0x03, 0x04, 0x06,   0x0d, 0x15, 0x01 ,0x04   ,0x04 ,0x0c ,0x1b ,0x01, 
					0x05, 0xff 
				};  

				uint32 Xid = 0;
				
				if (pParameter->WasParsed())
				{
					Xid = pParameter->AsUnsignedInt();
				}

				memcpy(&sebdata[46], &Xid, sizeof(uint32));

				BcmSebTxTunnelPktToIf21( sebdata, sizeof(sebdata) );				
			}
			break;

        case kJoinMcast:
            {
				// Default MAC/IP storage.
				BcmMacAddress multicastMac(0x01, 0x00, 0x5e, 0x0c, 0x0c, 0xbb);
				BcmIpAddress multicastIp(0xea, 0x0c, 0x0c, 0xbb);				

				// Read user input.
				if (pParameter->WasParsed())
				{
					multicastIp = pParameter->AsIpAddress();
					MapMcastIpAddressToMacAddress(multicastIp, &multicastMac);

					gLogMessageRaw 
						<< multicastIp <<" -> " << multicastMac << endl;			
				}
				else
				{
					gLogMessageRaw 
						<< "No multicast IP address parameter specified. Using default 234.12.12.187" << endl;					
				}
				
				//
				gLogMessageRaw 
					<< "ATTENTION!! Joining a multicast session for this IP address: " << multicastIp << endl;
				
				SebTestJoinMulticastSession(multicastIp.Get());
            }
            break;

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }

}


void SebBfcCommandTable::SebTestJoinMulticastSession(uint32 multicastIpAddress)
{
	struct sockaddr_in sinme;
	int i, optVal;
	static int mcastPortIncrement = 5;

	// Create CONTROL RX Socket
	bzero((char*)&sinme, sizeof(sinme));

    sinme.sin_family = AF_INET;
	sinme.sin_port = htons(ESTB_DSG_RESPONSE_PORT + mcastPortIncrement++);
	sinme.sin_addr.s_addr = htonl(multicastIpAddress); 				// 234.10.10.101

	// Create socket
    if ((sock_ctl_in = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
		printf("SebTestJoinMulticastSession: socket failure! %s\n", strerror(errno));
		return;
    }

	// Reuse address
    optVal = 1;
    if (setsockopt(sock_ctl_in, SOL_SOCKET, SO_REUSEADDR, (char *) &optVal, sizeof(optVal)) != 0)
	{
		printf ("setsockopt SO_REUSEADDR error:  %s \n", strerror(errno));  
	}

	// Bind
    if (bind(sock_ctl_in, (struct sockaddr *)&sinme, sizeof(sinme)) < 0) 
    {
		printf("SebTestJoinMulticastSession: bind failure! %s\n", strerror(errno));
		return;
    }

	{	// Use 192.168.17.10
		uint32 ipAddress = htonl(0xC0A8110A); //

		/* fill in the argument structure to join the multicast group */ 
		/* initialize the multicast address to join */ 
		struct ip_mreq      ipMreq; 
		ipMreq.imr_multiaddr.s_addr = htonl(multicastIpAddress); 

		// Set unicast interface addr from which to receive the multicast packets 
		//if( GetEstbIpAddress(&ipAddress) == 0 )		
		//{		
		//	return false;
		//}

		ipMreq.imr_interface.s_addr = ipAddress; 

		/* set the socket option to join the MULTICAST group */ 
		printf(" Joining multicast session at port %08x !\n", sinme.sin_port);
		if (setsockopt (sock_ctl_in, IPPROTO_IP, IP_ADD_MEMBERSHIP,  
						(char *)&ipMreq,  
						sizeof (ipMreq)) < 0) 
		{  
			printf ("setsockopt IP_ADD_MEMBERSHIP error:  %s %08lx %08lx\n"
					, strerror(errno), (uint32)ipMreq.imr_multiaddr.s_addr, (uint32)ipMreq.imr_interface.s_addr);  
			return;     
		}	

		// Turn loopback OFF.
		{
			uint8 loop = 0;
			if( setsockopt(sock_ctl_in, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0 )
			{
				printf ("setsockopt IP_MULTICAST_LOOP error:  %s \n"
						, strerror(errno));  
				return;
			}
		}
	}
}

void SebBfcCommandTable::MapMcastIpAddressToMacAddress(BcmIpAddress &mcastIpaddress, BcmMacAddress *pMcastMac )
{
	uint8 macAddress[] = { 0x01, 0x00, 0x5E, 0x00, 0x00, 0x00 };
	mcastIpaddress.Get( macAddress[2], macAddress[3], macAddress[4], macAddress[5] );
	macAddress[2] = 0x5E;
	macAddress[3] &= 0x07F;
	
	BcmMacAddress mCastMac;
	pMcastMac->Set(macAddress);
	//gLogMessageRaw << *pMcastMac << endl;
}


bool SebBfcCommandTable::ClientSebRxTunnelDataHandler(void *data, int dataLen )
{
	gLogMessageRaw << "ClientSebRxTunnelDataHandler" << endl;

	return true;
}

void SebBfcCommandTable::ClientSebStatusDataHandler( SebEcmEventNotification eventCode, void *data, int dataLen )
{
	gLogMessageRaw << "ClientSebStatusDataHandler" << endl;
}