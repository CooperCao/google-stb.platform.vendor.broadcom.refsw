//****************************************************************************
//  
// Copyright (c) 2004-2013 Broadcom Corporation
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
//
//****************************************************************************
//
//  Filename:       DsgCommandTable.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  December 13, 2004
//
//****************************************************************************
//  Description:
//      This derived class handles controlling the DSG-CC objects.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DsgCommandTable.h"

#include "DsgClientCtlThread.h"
#include "DsgClientRegistrar.h"
#include "DsgClientId.h"
#include "EthernetPacket.h"

#include "BitmaskCommandParameter.h"
#include "BoolCommandParameter.h"
#include "FlagCommandParameter.h"
#include "IpAddressCommandParameter.h"
#include "StringCommandParameter.h"
#include "StringSetCommandParameter.h"
#include "UnsignedIntCommandParameter.h"
#include "MacAddressCommandParameter.h"

#include "OperatingSystemFactory.h"

#if (BCM_NONVOL_SUPPORT)
#include "NonVolDevice.h"
#include "EstbTestNonVolSettings.h"
#endif

#if( CABLECARD_SUPPORT )
#include "DsgccCablecardIf.h"
#endif

#include "EstbRemoteInterfaceStub.h"
//****************** Extern Variables**************** ************************
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
extern char EstbPrimaryIpStackName[IPC_INTERFACE_NAME_MAX_LEN];
extern char ecmInterfaceName[IPC_INTERFACE_NAME_MAX_LEN];
#endif
//********************** Local Types *****************************************

//********************** Local Constants *************************************
/*extern "C"
{
	extern SetupRemoteInterfaceReturnCodes SetupRemoteInterface(void);
}*/

#if CABLECARD_SUPPORT
extern uint32 BcmGetCableCardIpAddress();
#endif


static const char *pgTableDescr = "Commands to change the state of the DSG Client Control thread.";

// kDsgTunnel constants
enum
{
	kTunnelAdd,
	kTunnelOpen,
	kTunnelClose,
	kTunnelShow
};

// API code: These enum should be the same as the Dsg Host defines
// defined in dsg_api.h
/*enum
{
	kDsgSetMode			= 0,
    kDsgEnableDocsis	= 1,
    kDsgOpenTunnel		= 2,
    kDsgCloseTunnel		= 3,
    kDsgSetTimer		= 4,
    kDsgGetTimer		= 5,   
    kDsgSetChannelsList,
    kDsgResetChannelsList,
    kDsgScan,
    kDsgStatus,     
    kDsgDocsisStatus,
    kDsgSetRIAddr,
    kDsgCpeIpConfig
};*/


enum
{
	kShowState = 40,
	kDsgTunnel,
	kAddClient,
	kAddClientWithClassifier,
	kRemoveClient,
	kShowClassifiers,
	kShowClientList,
	kShowTunnelPkts,
	kCannedSourceNameSubTable,
	kBcastTunnelIdMac,
	kShowSourceNameSubTable
	#if ( BCM_NONVOL_SUPPORT )
	,kWriteSettings
	#endif
	#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
	,kShowIpAddress,
	kShowEcmCommunication,
	kSetupRemoteInterface,
	kPing
	#endif
	,kDcdTlvShow
	,kSendCannedDcd
	,kSendCannedUcid
	,kSendCannedDsgDir
	,kDsgResetChannelList
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
    kForceImageFlag,        // PR1744
};

enum
{
	kDummyAppNameClientPortNumber = 0x10000,
	kDummyMacAddrClientPortNumber = 0x20000,
	kDummyUint16ClientPortNumber  = 0x40000
};
//********************** Local Variables *************************************

extern uint32 gTunnelDebugLevel;

#ifndef DSG_RPC_INCLUDED //#if !defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) && !defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
unsigned char BcmDsgCommandTable::singleProcessModelPortNumber = 0;
#endif


// This is a helper method that prompts the user for input for the classifier settings
//
// Parameters:
//
// Returns:  Nothing.
//
static bool PromptForClassifierSettings(BcmIpClassifierSettings &ipClassifierSets);

//********************** Local Functions *************************************
#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
extern "C"
{
	bool GetEstbIpAddress(uint32* pEstbIpAddress);
	ETHERNET_PACKET* WrapEthernetPacket( char *buf, uint32 length );
}
#endif
//********************** Class Method Implementations ************************


// Default Constructor.  Creates the command table, adds all of the
// commands to it and sets the command handler function.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmDsgCommandTable::BcmDsgCommandTable(void) :
    BcmCommandTable("STB DSG Client Control Thread Commands", "dsgcc", "dsgcc", pgTableDescr)
{

    BcmCommand command;

    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fIsHidden = false;
    command.fCommandFunctionWithParms = CommandHandler;

    command.fCommandId = kShowState;
    command.pfCommandName = "state";
    command.pfCommandHelp = "Shows the current state of the DSG Client Control Thread.";
    command.pfExamples = "state\t";
    AddCommand(command);

    command.fCommandId = kDsgScan;
    command.pfCommandName = "scan";
    command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmBoolCommandParameter(kPrimaryParameter, true));
    command.pfCommandHelp = "Enable/disable DsgScan";
    command.pfExamples = "scan   \t Start scan. \n"
						 "scan 0 \t Stop scan.\n";
    AddCommand(command);
    
	command.fCommandId = kDsgTunnel;
	command.pfCommandName = "tunnel_dsg";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("add|open|close|show", kPrimaryParameter, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmMacAddressCommandParameter(kSecondParameter, true));
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kThirdParameter, true, "ID"));
	command.pfCommandHelp = "Add, open or close a DSG tunnel for the specified MAC address.";
	command.pfExamples = "tunnel_dsg add 00:10:18:ff:ff:ff\tAdd a closed tunnel for 00:10:18:ff:ff:ff.\n" 
						"tunnel_dsg open 00:10:18:ff:ff:ff\tOpen a tunnel for 00:10:18:ff:ff:ff.\n"
					   "tunnel_dsg close 00:10:18:ff:ff:ff\tClose the tunnel for 00:10:18:ff:ff:ff, if it exists.\n"
					   "tunnel_dsg close 1\tClose the tunnel with ID 1, if it exists.\n"
					   "tunnel_dsg close\tClose all DSG tunnels\n"
					   "tunnel_dsg show 00:10:18:ff:ff:ff\tShow the tunnel for 00:10:18:ff:ff:ff, if it exists.\n"
					   "tunnel_dsg show 1\tShow the tunnel with ID 1, if it exists.\n"
					   "tunnel_dsg show\tShow all DSG tunnels\n";
	AddCommand(command);

    command.fCommandId = kDsgSetMode;
    command.pfCommandName = "mode";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, "Mode"));
    command.pfCommandHelp = "Sets DSG mode.";
    command.pfExamples = "mode 0 (disabled)\n"
						 "mode 1 (basic)\n"
						 "mode 2 (advance)\n";
    AddCommand(command);

    command.fCommandId = kDsgEnableDocsis;
    command.pfCommandName = "twoway_mode";
    command.pfCommandParameterList = new BcmCommandParameterList;
    command.pfCommandParameterList->AddOrderDependent(new BcmBoolCommandParameter(kPrimaryParameter, false));
    command.pfCommandHelp = "Changes upstream transmission mode.";
    command.pfExamples = "twoway_mode 0 (disabled)\n"
						 "twoway_mode 1 (enabled)\n";
    AddCommand(command);

    command.fCommandId = kDsgStatus;
    command.pfCommandName = "dsg_status";
    command.pfCommandParameterList = NULL;
    command.pfCommandHelp = "DSG Status.";
    command.pfExamples = "dsg_status";
    AddCommand(command);

    command.fCommandId = kDsgDocsisStatus;
    command.pfCommandName = "docsis_status";
    command.pfCommandParameterList = NULL;
    command.pfCommandHelp = "Show upstream status.";
    command.pfExamples = "docsis_status";
    AddCommand(command);

    command.fCommandId = kDsgSetTimer;
    command.pfCommandName = "timer_set";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, false, "Timer ID"));
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kSecondParameter, false, "Timer Value"));
    command.pfCommandHelp = "Set timer value.";
    command.pfExamples = "timer_set";
    AddCommand(command);

    command.fCommandId = kDsgSetChannelsList;
    command.pfCommandName = "set_channel";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, "channel"));
    command.pfCommandHelp = "Sets a single frequency channel list.";
    command.pfExamples = "set_channel 765 (Mhz)\n";
    AddCommand(command);

    command.fCommandId = kAddClient;
    command.pfCommandName = "add_client";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("none|bcast|mac|cas|app", kPrimaryParameter, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmMacAddressCommandParameter(kSecondParameter, true, "mac address"));
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kThirdParameter, true, "id", 0, 0xFFFF));
	command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kFourthParameter, true, "application name"));
    command.pfCommandHelp = "add_client <client_type> <client_id> ";
    command.pfExamples = 
						 "Note that if multiple broadcast tunnels are specified in the DCD\n"
						 "this command will open the first broadcast tunnel it finds in the list.\n"
						 "If you want to open a specific broadcast tunnel, set the mac address\n"
						 "of the broadcast tunnel you want to be opened by using the console command\n"
						 " < bcast_tunnel 01:00:5e:00:01:81 > for example, and then use.\n"
						 " <add_client bcast> command. \n\n"
						 
						 "add_client cas 3\n"
						 "add_client app 5\n"
						 "add_client mac 01:00:5e:00:01:13 \n"
						 "add_client mac 01:00:5e:00:01:81 \n"
						 "add_client mac 00:10:18:ff:ff:ff \n"
						 "add_client app TvGuide \n"
						 "add_client bcast \n";
    AddCommand(command);

    command.fCommandId = kAddClientWithClassifier;
    command.pfCommandName = "add_classifier_with_client";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("none|bcast|mac|cas|app", kPrimaryParameter, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmMacAddressCommandParameter(kSecondParameter, true, "mac address"));
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kThirdParameter, true, "id", 0, 0xFFFF));
	command.pfCommandParameterList->AddOrderDependent(new BcmBoolCommandParameter(kFourthParameter, true)); 
    command.pfCommandHelp = "add_classifier_with_client <client_type> <client_id> ";
    command.pfExamples = 						 
						 "add_classifier_with_client cas 3 \n"
						 "add_classifier_with_client app 5 0\n"
						 "add_classifier_with_client mac 01:00:5e:00:01:13 \n"
						 "add_classifier_with_client bcast \n";
    AddCommand(command);

    command.fCommandId = kRemoveClient;
    command.pfCommandName = "remove_client";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, false));
    command.pfCommandHelp = "remove_client <port number>";
    command.pfExamples = "First you need to use <clients_show> command to find the port number of the client to remove \n"
						 "and then enter the port number of the client using <remove_client> command.\n";
    AddCommand(command);

	#if ( BCM_NONVOL_SUPPORT )
    command.fCommandId = kWriteSettings;
    command.pfCommandName = "write";
	command.pfCommandParameterList = NULL;
    command.pfCommandHelp = 
        "Writes the dynamic section of eSTB Test non-volatile settings to non-volatile memory.  "
        "Client List is stored in the dynamic section of eSTB Test non-volatile settings";
    command.pfExamples = 
        "write";
    AddCommand(command);
	#endif

    command.fCommandId = kShowClientList;
    command.pfCommandName = "clients_show";
	command.pfCommandParameterList = NULL;
    command.pfCommandHelp = "Show Client List.";
    command.pfExamples = "clients_show\n";
    AddCommand(command);

    command.fCommandId = kShowClassifiers;
    command.pfCommandName = "classifiers_show";
	command.pfCommandParameterList = NULL;
    command.pfCommandHelp = "Show Classifier List.";
    command.pfExamples = "classifiers_show\n";
    AddCommand(command);

    command.fCommandId = kShowTunnelPkts;
    command.pfCommandName = "show_packets";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, "Debug Level"));
    command.pfCommandHelp = "Set debug level to display tunnel packets.\n"
							"   0: disabled\n"
							"   1: heartbeat\n"
							"   2: min \n"
							"   4: full\n"
                            "   8: IPU US\n"
                            "0x10: IPU DS\n";

	command.pfExamples = "show_packets 1\n";
    AddCommand(command);

#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
    command.fCommandId = kShowIpAddress;
    command.pfCommandName = "ipaddress_show";
	command.pfCommandParameterList = NULL;
    command.pfCommandHelp = "Show IP Address.";
    command.pfExamples = "ipaddress_show\n";
    AddCommand(command);

    command.fCommandId = kShowEcmCommunication;
    command.pfCommandName = "communication_sets_show";
	command.pfCommandParameterList = NULL;
    command.pfCommandHelp = "Shows eCM-eSTB communication settings.";
    command.pfExamples = "communication_sets_show\n";
    AddCommand(command);

    command.fCommandId = kPing;
    command.pfCommandName = "ping";
	command.pfCommandParameterList = new BcmCommandParameterList;;
	command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kPrimaryParameter, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kSecondParameter, true, "number of pings"));
    command.pfCommandHelp = "Sends canned ping messages to the specified destination.\n " "Sends 3 ping messages if a number is not specified.\n";
    command.pfExamples = "ping 11.24.71.122  <number of pings>\n";
    AddCommand(command);

    command.fCommandId = kSetupRemoteInterface;
    command.pfCommandName = "setup_rmt_if";
	command.fIsHidden = true;
	command.pfCommandParameterList = NULL;
    command.pfCommandHelp = "Setups eCM-eSTB communication link.";
    command.pfExamples = "setup_rmt_if\n";
    AddCommand(command);

#endif

    command.fCommandId = kCannedSourceNameSubTable;
    command.pfCommandName = "send_canned_sns";
	command.fIsHidden = true;
	command.pfCommandParameterList = NULL;
    command.pfCommandHelp = "Sends a canned Source Name Sub Table to DSG-CC Thread's message queue.";
    command.pfExamples = "send_canned_sns\n";
    AddCommand(command);

    command.fCommandId = kShowSourceNameSubTable;
    command.pfCommandName = "sns_table_show";
	command.fIsHidden = true;
	command.pfCommandParameterList = NULL;
    command.pfCommandHelp = "Displays Source Name Sub Table.";
    command.pfExamples = "sns_table_show\n";
    AddCommand(command);

    command.fCommandId = kBcastTunnelIdMac;
    command.pfCommandName = "bcast_tunnel";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmMacAddressCommandParameter(kPrimaryParameter, true, "broadcast tunnel mac address"));
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kSecondParameter, true, "broadcast tunnel id"));
    command.pfCommandHelp = "Sets the broadcast tunnel MAC address. This command is useful when multiple broadcast		\n"
							"tunnels are specified on the same DCD for testing purposes. \n";
    command.pfExamples = "bcast_tunnel 01:00:5e:00:01:81\n";
    AddCommand(command);

	command.fCommandId = kDcdTlvShow;
    command.pfCommandName = "dcd_tlv_show";
	command.fIsHidden = true;
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, ""));
    command.pfCommandHelp = "Displays last received NEW DCD buffer.";
    command.pfExamples = "dcd_tlv_show\n";
    AddCommand(command);

	command.fCommandId = kSendCannedDcd;
    command.pfCommandName = "send_canned_dcd";
	command.fIsHidden = true;
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, ""));
    command.pfCommandHelp = "Sends canned DCD msg to DSG-CC.";
    command.pfExamples = "send_canned_dcd\n";
    AddCommand(command);

    command.fCommandId = kSendCannedUcid;
    command.pfCommandName = "send_canned_ucid";
	command.fIsHidden = true;
	command.pfCommandParameterList = new BcmCommandParameterList;;
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, "UCID value"));
    command.pfCommandHelp = "Sends canned UCID event to DSG-CC.\n";
    command.pfExamples = "send_canned_ucid  <UCID>\n";
    AddCommand(command);

	command.fCommandId = kDsgSetEstbPrimaryAddr;
	command.pfCommandName = "set_estb_primary_address";
	command.fIsHidden = false;
	command.pfCommandParameterList = new BcmCommandParameterList;;
	command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kPrimaryParameter, true, "IP Address value"));
	command.pfCommandParameterList->AddOrderDependent(new BcmMacAddressCommandParameter(kSecondParameter, true, "MAC address value"));
	command.pfCommandHelp = "Set primary IP(v4/v6) and MAC address of eSTB to eCM.\n"
							"set_estb_primary_address	<IPv4 or IPv6 address> <MAC address>\n";
	command.pfExamples = "set_estb_primary_address  fe80::211:22ff:fe11:ff33   00:11:22:11:FF:33\n"
						 "set_estb_primary_address  192.168.100.10             00:11:22:11:FF:33\n";
	AddCommand(command);

	command.fCommandId = kDsgSetDhcpV4VendorSpecificOption;
	command.pfCommandName = "test_dhcpv4vendorspecific";
	command.fIsHidden = true;
	command.pfCommandParameterList = NULL;
	command.pfCommandHelp = "test_dhcpv4vendorspecific\n";
	command.pfExamples = "test_dhcpv4vendorspecific\n";
	AddCommand(command);

	command.fCommandId = kDsgSetDhcpV6VendorSpecificOption;
	command.pfCommandName = "test_dhcpv6vendorspecific";
	command.fIsHidden = true;
	command.pfCommandParameterList = NULL;
	command.pfCommandHelp = "test_dhcpv6vendorspecific\n";
	command.pfExamples = "test_dhcpv6vendorspecific\n";
	AddCommand(command);

	command.fCommandId = kDsgSetCCMac;
	command.pfCommandName = "set_cablecard_mac_address";
	command.fIsHidden = false;
	command.pfCommandParameterList = new BcmCommandParameterList;;
	command.pfCommandParameterList->AddOrderDependent(new BcmMacAddressCommandParameter(kPrimaryParameter, true, "MAC Address value"));
	command.pfCommandHelp = "Set CableCard's MAC address (eSafe device MAC address) to eCM.\n"
							"set_cablecard_mac_address	<MAC address>\n";
	command.pfExamples = "set_cablecard_mac_address  00:11:22:11:FF:33\n";
	AddCommand(command);

	#if( CABLECARD_SUPPORT )
    command.fCommandId = kSendCannedDsgDir;
    command.pfCommandName = "send_canned_dsgdir";
	command.fIsHidden = true;
	command.pfCommandParameterList = new BcmCommandParameterList;;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, ""));
    command.pfCommandHelp = "Sends canned dsg-dir.\n";
    command.pfExamples = "send_canned_dsgdir\n";
    AddCommand(command);
	#endif
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDsgCommandTable::~BcmDsgCommandTable()
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
void BcmDsgCommandTable::CommandHandler(void *pInstanceValue, 
                                             const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = NULL;
    YesNo answer = kNo;

    // By convention, the settings object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmDsgClientCtlThread *pDsgClientCtlThread = (BcmDsgClientCtlThread *) pInstanceValue;

    // Validate this just for sanity.
    if (pDsgClientCtlThread == NULL)
    {
        gLogMessageRaw 
            << "BcmDsgCommandTable::CommandHandler:  ERROR - "
            << "User value (pointer to CM DSG Ctl Thread instance) is NULL!  Cant process the command!\n";

        return;
    }

    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

    switch (command.fCommandId)
    {
        case kShowState:
            gLogMessageRaw << *pDsgClientCtlThread << endl;
            break;

		case kDsgScan:
			{
				bool enableScan = true;
				if (pParameter->WasParsed())
				{
					enableScan = pParameter->AsBool();
				}

				BcmDsgCommandTable::Scan(enableScan);
			}
			break;
            
		case kDsgSetMode:
			{
				if (pParameter->WasParsed())
				{
					uint32 mode = pParameter->AsUnsignedInt();
					BcmDsgCommandTable::SetMode(mode);
					
					gLogMessageRaw << "You have to use 'scan' command if you want eCM start scanning in this mode." << endl;
				}
				else
				{
					gLogMessageRaw << "DSG mode = " << pDsgClientCtlThread->Mode() << endl;
				}
			}
			break;

		case kDsgEnableDocsis:
			{
				if (pParameter->WasParsed())
				{
					BcmDsgCommandTable::EnableDocsis(pParameter->AsBool());
				}
			}
			break;

		case kDsgStatus:
			{
				BcmDsgCommandTable::DsgStatus();

				//SetEsafeProvisioningStatusAndFailure( (uint8)2, (uint8)1, 84010300, strlen("OCHD2.1-Prov-1"), "OCHD2.1-Prov-1",
				//					                                                     strlen("OCHD2.1-Prov-1"), "OCHD2.1-Prov-1");
			}
			break;

		case kDsgDocsisStatus:
			{
				BcmDsgCommandTable::DocsisStatus();
			}
			break;

		case kDsgSetTimer:
			{
				if (pParameter->WasParsed())
				{
					uint8 whichTimer = pParameter->AsUnsignedInt();
					
					const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);
					if (pParameter2->WasParsed())
					{
						uint16 valueInMilliSeconds = pParameter2->AsUnsignedInt();
						BcmDsgCommandTable::SetTimer(whichTimer, valueInMilliSeconds);
					}					
				}
				gLogMessageRaw << *pDsgClientCtlThread << endl;
			}
			break;

		case kDsgSetChannelsList:
			{
				if (pParameter->WasParsed())
				{
					uint32 channel = pParameter->AsUnsignedInt();
					BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();
					if(pDsgClientCtlThread)
					{
						// DsgSetChannelList expects uint32 array
						channel = htonl(channel);

						// serialize the function call by sending to our msg q.
						pDsgClientCtlThread->SerializePublicInterfaceCall( kTestDsgSetChannelList, (void*)channel );
					}									
				}
			}
			break;


        case kDsgTunnel:
            {
              const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);
              const BcmCommandParameter *pParameter3 = command.pfCommandParameterList->Find(kThirdParameter);
              BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();
              BcmMacAddress Mac;
              unsigned int ID = 0;
              
              
              if (pParameter2->WasParsed())
                Mac = pParameter2->AsMacAddress();
              
              if (pParameter3->WasParsed())
                ID = pParameter3->AsUnsignedInt();
              
              // Should never happen...
              if (pDsgClientCtlThread == NULL)
              {
                gLogMessageRaw << "No DSG thread!" << endl;
                break;
              }
              
              /* OPEN */
              
              if (pParameter->AsUnsignedInt() == kTunnelOpen)
              {
                // MAC specified...
                if (pParameter2->WasParsed())
                {
                  t_mac_adr tMac;                
              
                  Mac.Get(tMac.macaddress[0], tMac.macaddress[1], tMac.macaddress[2],
                          tMac.macaddress[3], tMac.macaddress[4], tMac.macaddress[5]);
                          
                  int Tunnel = 0 ;

				  if(pDsgClientCtlThread->Mode() == kDsgAdvanceMode)
				  {
					TunnelSettingsListIterator tunnelSet_it = pDsgClientCtlThread->DsgTunnelManager().find(Mac);
					if(tunnelSet_it != pDsgClientCtlThread->DsgTunnelManager().end())
					{
						Tunnel = pDsgClientCtlThread->ActivateTunnel(&(tunnelSet_it->second));
					}
				  }
				  else if(pDsgClientCtlThread->Mode() == kDsgBasicMode)
				  {
					Tunnel = pDsgClientCtlThread->OpenTunnel(Mac);
				  }
 
                  if (Tunnel == 0)
                    gLogMessageRaw << "Failed to open tunnel for " << Mac << endl;
                  else
                    gLogMessageRaw << "Opened tunnel number " << Tunnel << endl;
                }
                else
                  gLogMessageRaw << "You must specify a MAC address to open a tunnel." << endl;
              }
              
			  else if(pParameter->AsUnsignedInt() == kTunnelAdd)
			  {                         
                // MAC specified...
                if (pParameter2->WasParsed())
				{
					t_mac_adr tMac;                

					Mac.Get(tMac.macaddress[0], tMac.macaddress[1], tMac.macaddress[2],
					tMac.macaddress[3], tMac.macaddress[4], tMac.macaddress[5]);

					// No tunnel with this address exists. So create a new one.
					BcmDsgEstbTunnelSettings newTunnel;


					// If UCID range is specified, get it.
					/*if (pParameter3->WasParsed())
					{
						ID = pParameter3->AsUnsignedInt();
						newTunnel.fUcidRange = (uint16) ID;
					}*/

					// Set the mac address. Remember, at this point we failed to find a DCD
					// AND at least one client registered with MAC ADDRESS ID type.
					// In basic mode, eCM checks only MAC ADDRESS of the DSG packet 
					// to lock on a downstream.
					newTunnel.TunnelAddress(Mac);
					pDsgClientCtlThread->DsgTunnelManager().add(newTunnel);
				}
			  }

              /* CLOSE / SHOW */
              
              else 
              {
                TunnelSettingsListIterator Pos;

                // If a MAC address was specified, get the ID from it.
                if (pParameter2->WasParsed())
                {
                  Pos = pDsgClientCtlThread->DsgTunnelManager().find(Mac);
                  if (Pos == pDsgClientCtlThread->DsgTunnelManager().end())
                  {
                    gLogMessageRaw << "Couldnt find tunnel for " << Mac << endl;
                    break;
                  }
                  else
                    ID = Pos->second.TunnelId();
                }
                
                // If an ID was specified, get the MAC from it.
                else if (pParameter3->WasParsed())
                {
                  Pos = pDsgClientCtlThread->DsgTunnelManager().find(ID);
                  if (Pos == pDsgClientCtlThread->DsgTunnelManager().end())
                  {
                    gLogMessageRaw << "Couldn't find tunnel for " << ID << endl;
                    break;
                  }
                  else
                    Mac = Pos->second.TunnelAddress();
                }
                
                // At this point we should have a matching MAC/ID pair,
                // if either was specified.
                
                /* CLOSE */
                
                if (pParameter->AsUnsignedInt() == kTunnelClose)
                {
                  // If no MAC or ID specified, close all.
                  if ((pParameter2->WasParsed() == false)
                  &&  (pParameter3->WasParsed() == false))
                  {
                    gLogMessageRaw << "Closing all tunnels." << endl;
                    pDsgClientCtlThread->ResetTunnels();

                    break;
                  }
                    
                  // Otherwise use the ID that was either directly specified
                  // or obtained from the MAC address.

                  if (pDsgClientCtlThread->CloseTunnel(ID) == true)
                    gLogMessageRaw << "Closed tunnel " << ID << " for " << Mac << endl;
                  else
                    gLogMessageRaw << "Failed to close tunnel " << ID << " for " << Mac << endl;

                }
                
                /* SHOW */
                
                else if (pParameter->AsUnsignedInt() == kTunnelShow)
                {
                  // If no MAC or ID specified, show all.
                  if ((pParameter2->WasParsed() == false) 
                  &&  (pParameter3->WasParsed() == false))
                  {
                    Pos = pDsgClientCtlThread->DsgTunnelManager().begin();
                    
                    while (Pos != pDsgClientCtlThread->DsgTunnelManager().end())
                    {
                      gLogMessageRaw << Pos->second << endl;
                      Pos++;
                    }
                  }
                  
                  // Otherwise show the tunnel.  Note that we should already
                  // have a valid iterator at this point from our MAC/ID
                  // mapping earlier.
                  else
                  {
                    if (Pos == pDsgClientCtlThread->DsgTunnelManager().end())
                      gLogMessageRaw << "Couldn't find tunnel for " << Mac << endl;
                    else
                      gLogMessageRaw << Pos->second << endl;
                  }
                }
              }
            }
            break;

		case kAddClientWithClassifier:
			{
				uint16 ID;
				const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);
				const BcmCommandParameter *pParameter3 = command.pfCommandParameterList->Find(kThirdParameter);
				const BcmCommandParameter *pParameter4 = command.pfCommandParameterList->Find(kFourthParameter);

				dsg_classifier *dsgClassifier = NULL;
				BcmOctetBuffer octetBuffer;
				if (pParameter4->WasParsed())
				{
					gLogMessageRaw << "4th param parsed" << endl;
					if(pParameter4->AsBool())
					{
						gLogMessageRaw << "TRUE" << endl;
						BcmIpClassifierSettings ipClassifierSets;
						if( PromptForClassifierSettings(ipClassifierSets) == false )
						{
							return;
						}

						{
							
							uint8 portUsed = 0;
							uint8 unused = 0;

							uint32 source_ip_adr;
							ipClassifierSets.SourceIpAddr().Get(source_ip_adr);

							uint32 source_ip_mask;
							ipClassifierSets.SourceIpMask().Get(source_ip_mask);

							uint32 dest_ip_adr;
							ipClassifierSets.DestIpAddr().Get(dest_ip_adr);

							if(ipClassifierSets.DestPortStartSpecified() || ipClassifierSets.DestPortEndSpecified() )
							{
								portUsed = 1;
							}

							if( !octetBuffer.NetworkAddToEnd( (uint8)ipClassifierSets.SourceIpAddrSpecified() ) ||
								!octetBuffer.NetworkAddToEnd( (uint8)ipClassifierSets.SourceIpMaskSpecified ()) ||
								!octetBuffer.NetworkAddToEnd( ipClassifierSets.DestIpAddrSpecified ()) ||
								!octetBuffer.NetworkAddToEnd( portUsed ) ||
								!octetBuffer.NetworkAddToEnd( source_ip_adr ) ||
								!octetBuffer.NetworkAddToEnd( source_ip_mask ) ||
								!octetBuffer.NetworkAddToEnd( dest_ip_adr ) ||
								!octetBuffer.NetworkAddToEnd( ipClassifierSets.DestPortStart()) ||
								!octetBuffer.NetworkAddToEnd( ipClassifierSets.DestPortEnd()) 		
							  )
							{
								gLogMessageRaw << "Failed to set dsgClassifier!" << endl;
							}
							else
							{
								//gLogMessageRaw << "octetbuffer = " <<octetBuffer << endl;
								dsgClassifier = (dsg_classifier *) octetBuffer.AccessBuffer();
								//gLogMessageRaw << "dsgClassifier = " << (void*) dsgClassifier << "octetBuffer.AccessBuffer() = " << (void*)octetBuffer.AccessBuffer() << endl;
							}
						}						
					}
				}				

				/* CAS */

				if (pParameter->AsUnsignedInt() == kCaSystemType)
				{
					// ID specified...
					if (pParameter3->WasParsed())
					{
						ID = pParameter3->AsUnsignedInt();
						gLogMessageRaw << "ID  = " << (uint32) ID << endl;
						AddTestClientUint16( (uint32) kCaSystemType, ID, dsgClassifier);
					}
					else
					{
						gLogMessageRaw << "You must specify a client ID to add a client." << endl;
					}
				}

				if (pParameter->AsUnsignedInt() == kApplicationType)
				{
					// ID specified...
					if (pParameter3->WasParsed())
					{
						ID = pParameter3->AsUnsignedInt();
						gLogMessageRaw << "ID  = " << (uint32) ID << endl;
						gLogMessageRaw << "octetbuffer = " <<octetBuffer << endl;
						if(dsgClassifier == NULL)
						{
							gLogMessageRaw << "dsgClassifier is NULL "  << endl;
						}
						AddTestClientUint16( (uint32) kApplicationType, ID, dsgClassifier);
					}
					else
					{
						gLogMessageRaw << "You must specify a Client ID or an Application name." << endl;
					}
				}

				if (pParameter->AsUnsignedInt() == kBroadcastType)
				{
					// ID specified...
					if (pParameter3->WasParsed())
					{
						ID = pParameter3->AsUnsignedInt();
						AddTestClientUint16( (uint32) kBroadcastType, ID, dsgClassifier);
					}
					else
					{
						gLogMessageRaw << "You must specify a Client ID." << endl;
					}
				}

				if (pParameter->AsUnsignedInt() == kMacAddressType)
				{
					// MAC specified...
					if (pParameter2->WasParsed())
					{
						BcmMacAddress Mac = pParameter2->AsMacAddress();
						t_mac_adr tMac;                

						Mac.Get(tMac.macaddress[0], tMac.macaddress[1], tMac.macaddress[2],
							tMac.macaddress[3], tMac.macaddress[4], tMac.macaddress[5]);

						AddTestClientMacAddress(Mac);
					}
					else
						gLogMessageRaw << "You must specify a MAC address." << endl;
				}
			}
			break;

		case kAddClient:
			{
				uint16 ID;
				const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);
				const BcmCommandParameter *pParameter3 = command.pfCommandParameterList->Find(kThirdParameter);
				const BcmCommandParameter *pParameter4 = command.pfCommandParameterList->Find(kFourthParameter);
				/* CAS */

				if (pParameter->AsUnsignedInt() == kCaSystemType)
				{
					// ID specified...
					if (pParameter3->WasParsed())
					{
						ID = pParameter3->AsUnsignedInt();
						gLogMessageRaw << "ID  = " << (uint32) ID << endl;
						AddTestClientUint16( (uint32) kCaSystemType, ID, NULL);
					}
					else
					{
						gLogMessageRaw << "You must specify a client ID to add a client." << endl;
					}
				}

				if (pParameter->AsUnsignedInt() == kApplicationType)
				{
					// ID specified...
					if (pParameter3->WasParsed())
					{
						ID = pParameter3->AsUnsignedInt();
						AddTestClientUint16( (uint32) kApplicationType, ID, NULL);
					}
					else if (pParameter4->WasParsed())
					{
						#ifdef APPNAME_SUPPORT
						gLogMessageRaw << "Broadcast Tunnel MAC address is = " << pDsgClientCtlThread->fBroadcastTunnelMac << endl;
						BcmString AppName = pParameter4->AsString();						
						AddTestClientAppType( AppName );
						#else
						gLogMessageRaw << "Build with APPNAME_SUPPORT to enable Application name ->ID mapping." << endl;
						#endif
					}
					else
					{
						gLogMessageRaw << "You must specify a Client ID or an Application name." << endl;
					}
				}

				if (pParameter->AsUnsignedInt() == kBroadcastType)
				{
                                        // ID specified...
                                        if (pParameter3->WasParsed())
                                        {
                                                ID = pParameter3->AsUnsignedInt();
                                                gLogMessageRaw << "ID  = " << (uint32) ID << endl;
                                                AddTestClientUint16( (uint32) kBroadcastType, ID, NULL);
                                        }
                                        else
                                        {
					  // ID is unspecified.
                                          gLogMessageRaw << "ID  unspecified " << endl;
					  AddTestClientUint16( (uint32) kBroadcastType, 0, NULL);
                                        }
 
				}

				if (pParameter->AsUnsignedInt() == kMacAddressType)
				{
					// MAC specified...
					if (pParameter2->WasParsed())
					{
						BcmMacAddress Mac = pParameter2->AsMacAddress();
						t_mac_adr tMac;                

						Mac.Get(tMac.macaddress[0], tMac.macaddress[1], tMac.macaddress[2],
							tMac.macaddress[3], tMac.macaddress[4], tMac.macaddress[5]);

						AddTestClientMacAddress(Mac);
					}
					else
						gLogMessageRaw << "You must specify a MAC address." << endl;
				}
			}
			break;

		case kRemoveClient:
			{
				if (pParameter->WasParsed())
				{
					uint32 port = pParameter->AsUnsignedInt();
					BcmDsgClient *client = pDsgClientCtlThread->DsgClientManager().FindByPort(port);
					if (client)
					{
						pDsgClientCtlThread->DeregisterClient(client);
					}					
				}
			}
			break;
		
		#if ( BCM_NONVOL_SUPPORT && TARGETOS_Linux )
        case kWriteSettings:
            {
				// By convention, the settings object registers its instance so that the
				// user value is the 'this' pointer.  This cast should be safe.
				BcmEstbTestNonVolSettings *pNonVolSettings = BcmEstbTestNonVolSettings::GetSingletonInstance();

				// Validate this just for sanity.
				if (pNonVolSettings == NULL)
				{
					gLogMessageRaw 
						<< "BcmEstbTestNonVolSettingsCommandTable::CommandHandler:  ERROR - "
						<< "User value (pointer to NonVol Settings instance) is NULL!  Can't process the command!\n";

					return;
				}

                BcmNonVolDevice *pNonVolDevice;

                // If a non-vol device was given to me, then use it.
                pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();			

                if (pNonVolDevice != NULL)
                {
                    bool result;

                    result = pNonVolDevice->Write(*pNonVolSettings, BcmNonVolSettings::kDynamicSection);

                    if (result == false)
                    {
                        gLogMessageRaw << "ERROR - failed to write Dynamic Non-Vol Settings to the device!\n";
                    }
                    else
                    {
                        gLogMessageRaw << "Dynamic Non-Vol Settings successfully written to the device.\n";
                    }                    
                }
                else
                {
                    gLogMessageRaw << "ERROR - No Non-Vol Device is present; can't write settings to the device!\n";
                }
            }
		    break;
		#endif

		case kShowClassifiers:
			{
				pDsgClientCtlThread->PrintClassifierList();
			}
			break;

		case kShowClientList:
			{
				gLogMessageRaw << pDsgClientCtlThread->DsgClientManager() << endl;
			}
			break;


#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
		case kShowIpAddress:
			{				
				#ifndef ECM_STUB_API
				uint32 ifIpAddress = 0;
				BcmIpAddress ipAddress;
				if( GetEstbIpAddress(&ifIpAddress) )
				{
					ipAddress.Set((uint32)htonl(ifIpAddress));
					gLogMessageRaw << "eSTB Primary Ip: " << ipAddress << endl;
				}
				else
				{
					gLogMessageRaw << "Failed to get eSTB IP address." << endl;
				}

				if( GetIpStackAddress(&ifIpAddress, (char*)ecmInterfaceName) == 0 )
				{
					ipAddress.Set((uint32)htonl(ifIpAddress));
					gLogMessageRaw << (char*)ecmInterfaceName <<": " << ipAddress << endl;
				}
				else
				{
					gLogMessageRaw << "Failed to get eSTB-eCM comm IP address." << endl;
				}

				#if CABLECARD_SUPPORT
				ifIpAddress = BcmGetCableCardIpAddress();
				ipAddress.Set(ifIpAddress);
				gLogMessageRaw << "CableCard Ip: " << ipAddress << endl;
				#endif
				#endif
			}
			break;

		case kShowEcmCommunication:
			{
				BcmIpAddress eCMIpAddress;
				eCMIpAddress.Set((uint32)(kRemoteEcmIpAddress));

				BcmIpAddress eSTBIpAddress;
				eSTBIpAddress.Set((uint32)(kLocalRxIpAddress));

				gLogMessageRaw << setw(30)

					<< "            eSTB is listening to eCM on = "<< eSTBIpAddress << "\n"

					<< "            eCM is listening to eSTB on = "<< eCMIpAddress << "\n\n"

					<< "      Sending API requests to eCM Port# = 0x" << hex << ECM_DSG_REQUEST_PORT << "\n"

					<< "     Waiting for API responses on Port# = 0x" << hex << ESTB_DSG_RESPONSE_PORT << "\n"

					<< " Waiting for eCM Notifications on Port# = 0x" << hex << ESTB_DSG_REQUEST_PORT << "\n"

					<< "          Waiting for DSG DATA on Port# = 0x" << hex << ESTB_DSG_DATA_PORT << "\n";


			}
			break;

		case kSetupRemoteInterface:
			{
				#ifndef ECM_STUB_API
				SetupRemoteInterface(NULL);
				#endif
			}
			break;

		case kPing:
			{
                if (pParameter->WasParsed())
				{
					uint32 numberOfPings = 3;

					const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);
					if (pParameter2->WasParsed())
					{
						numberOfPings = pParameter2->AsUnsignedInt();
					}

					while(numberOfPings--)
					{
						if(! BcmDsgCommandTable::Ping( pParameter->AsIpAddress(), numberOfPings ))
						{
							break;
						}

						BcmOperatingSystemFactory::ThreadSleep(1000);
					}
				}
			}
			break;
    
#endif

		case kShowTunnelPkts:
			{
				uint32 mode = gTunnelDebugLevel;
				if (pParameter->WasParsed())
				{
					mode = pParameter->AsUnsignedInt();

					gTunnelDebugLevel = mode;
					if(mode > 0)
					{
						gLogMessageRaw << "Packet Logging enabled."<< endl;
					}
					else if(mode == 0)
					{
						gLogMessageRaw << "Packet Logging disabled."<< endl;
					}					
				}
				else
				{
					gLogMessageRaw << "Current Logging mode is 0x" << hex << mode << endl;
				}
			}
			break;

		#ifdef APPNAME_SUPPORT
		case kCannedSourceNameSubTable:
			{
				unsigned char debugSNSpkt[] = 
						{ 
						  0x00, 0x10, 0x18, 0x01,
						  0x02, 0x03, 0xff, 0xff,
						  0xff, 0xff, 0xff, 0xff,
						  0x08, 0x00, 
						  
						  0x45, 0x00,							
						  0x00, 0x58, 0x02, 0xbd, 
						  0x00, 0x00, 0x1f, 0x11, 
						  0xa1, 0xd3, 0x0b, 0x18, 
						  0x04, 0x6c, 0xe6, 0x00,
						  0x01, 0x81, 
						  
						  0x13, 0x89, 0x13, 0x89, 
						  0x00,	0x44, 0x9d, 0xb0, 
					
						  0xff, 0x30, 0xee, 0x72,
                          0xc3, 0x30, 0x35, 0x00,
                          0x65, 0x6e, 0x67, 0x06,
                          0x03, 0x00, 0x00, 0x01,
                          0x10, 0x87, 0x00, 0x00,
                          0x04, 0x57, 0x43, 0x42,
                          0x53, 0x00, 0x04, 0x57,
                          0x43, 0x42, 0x53, 0x88,
                          0x00, 0x00, 0x80, 0x00,
                          0x03, 0x06, 0x00, 0x04,
                          0x61, 0x70, 0x70, 0x33,
                          0x00, 0x80, 0x00, 0x04,
                          0x06, 0x00, 0x04, 0x63,
                          0x61, 0x73, 0x31, 0x00,
                          0x26, 0x80, 0x2f, 0x88 };
  				
				// Wrap the buffer in a ETHERNET_PACKET structure. 
				ETHERNET_PACKET* snsTable = WrapEthernetPacket((char*)debugSNSpkt, sizeof(debugSNSpkt));

				unsigned char *pBufData = (unsigned char *) snsTable->pDataBufferHead->pvStart;
				uint16 pktlen = snsTable->pDataBufferHead->uiLen;     //get ethernet pkt length

				gLogMessageRaw << "PKTLEN = " << pktlen << endl;

				BcmEthernetPacket pack;
				pack.SetPacket(snsTable);

				if( pack.ParseTcpUdp() == true )
				{
					gLogMessageRaw << pack << endl;
				}
				else
				{
					gLogMessageRaw << "NOT AN IP PACKET \n" << pack<< endl;
				}

				// Send it to DSG-CC Thread's message queue.
				pDsgClientCtlThread->ProcessSourceSubTable(snsTable);
			}
			break;

		case kShowSourceNameSubTable:
			{
				if( pDsgClientCtlThread->pfAppNameTable )
				{
					pDsgClientCtlThread->pfAppNameTable->Dump();
				}
				else
				{
					gLogMessageRaw << "App Name Table is EMPTY."<< endl;
				}
			}
			break;
		#endif

		case kBcastTunnelIdMac:
			{
				const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);

				// MAC specified...
				if (pParameter->WasParsed())
				{
					pDsgClientCtlThread->fBroadcastTunnelMac = pParameter->AsMacAddress();
				}


				if (pParameter2->WasParsed())
				{
					uint32 id = pParameter2->AsUnsignedInt();

					pDsgClientCtlThread->BcastTunnelId(id);		
				}
				

				gLogMessageRaw << "Broadcast Tunnel Id is " << pDsgClientCtlThread->BcastTunnelId()
							   << "Broadcast Tunnel MAC is " << pDsgClientCtlThread->fBroadcastTunnelMac << endl;
				
			}
			break;

		case kDcdTlvShow:
			{
				pDsgClientCtlThread->DcdMsg()->PrintTLVs();
			}
			break;


		case kSendCannedDcd:
			{
				uint8 dcdMsgPdu[] =
				{
					  0x01, 0x01, 0x01, 
					  
					  0x32, 0x18, //0x1d, 
					  0x01, 0x01, 0x01, 
					  0x02, 0x01, 0x01, 
					  //0x03, 0x03, 0x00, 0x01, 0x02,
					  0x04, 0x04, 
							0x03, 0x02, 0x00, 0x01, 
					  0x05, 0x06, 0x01, 0x00, 0x5e, 0x00, 0x00, 0x71, 
					  0x06, 0x02, 0x00, 0x01, 

					  0x32, 0x2a, 
					  0x01, 0x01, 0x02, 
					  0x02, 0x01, 0x01, 
					  0x03, 0x02, 0x01, 0x02,
					  0x04, 0x0e, 
							0x01, 0x02, 0x00, 0x02,
							0x03, 0x02, 0x00, 0x01,
							0x03, 0x02, 0x00, 0x02, 
							0x01, 0x00,
					  0x05, 0x06, 0x01, 0x00, 0x5e, 0x00, 0x01, 0x81, 
					  0x06, 0x02, 0x00, 0x02, 
					  0x06, 0x02, 0x00, 0x03,
 
					  0x17, 0x1d, 
						  0x02, 0x02, 0x00, 0x01, 
						  0x05, 0x01, 0x01,   
						  0x09, 0x14, 
							  0x05, 0x04, 0xef, 0x00, 0x00, 0x71, 
							  0x03, 0x04, 0x0b, 0x18, 0x04, 0x6d, 
							  0x09, 0x02, 0x00, 0xc0, 
							  0x0a, 0x02, 0x00, 0xd0, 
					  
					  0x17, 0x1d, 
						  0x02, 0x02, 0x00, 0x02,   
						  0x05, 0x01, 0x01, 
						  0x09, 0x14, 
							  0x05, 0x04, 0xe6, 0x00, 0x01, 0x81, 
							  0x03, 0x04, 0x0b, 0x18, 0x04, 0x6b, 
							  0x09, 0x02, 0x13, 0x88, 
							  0x0a, 0x02, 0x13, 0x90, 
					  
					  0x17, 0x15, 
						  0x02, 0x02, 0x00, 0x03, 
						  0x05, 0x01, 0x02, 
						  0x09, 0x0c, 
							  0x05, 0x04, 0xe6, 0x00, 0x01, 0x09, 
							  0x03, 0x04, 0x0b, 0x18, 0x04, 0x6b,

					  0x33, 0x16, 
					  0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 
					  0x02, 0x02, 0x00, 0x0F, 
					  0x03, 0x02, 0x00, 0x14, 
					  0x04, 0x02, 0x00, 0x14, 
					  0x05, 0x02, 0x00, 0x14
				};
				pDsgClientCtlThread->DsgRxDCD(sizeof(dcdMsgPdu), dcdMsgPdu);
			}
			break;

		case kSendCannedUcid:
			{
				uint32 ucid = 0;
				if (pParameter->WasParsed())
				{
					ucid = pParameter->AsUnsignedInt();
					pDsgClientCtlThread->DsgUcid((uint8) ucid);
					
				}
				else
				{
					gLogMessageRaw << "Current UCID value is " << (uint32) pDsgClientCtlThread->Ucid() << endl;
				}
			}
			break;

		case kDsgSetEstbPrimaryAddr:
			{
				const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);

				if (pParameter->WasParsed() && pParameter2->WasParsed())
				{
					BcmIpAddress ipAddress = pParameter->AsIpAddress();
					BcmMacAddress macAddress = pParameter2->AsMacAddress();
					uint8 macAddr[6];

					macAddress.Get(macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
					if (ipAddress.IsIpV6Address())
					{
						uint8 ipv6Addr[16];
						ipAddress.Get(ipv6Addr, 16);
						DsgSetEstbPrimaryIpV6Addr((uint8 *)ipv6Addr, (uint8 *)macAddr);
						//ipAddress.Print (gLogMessageRaw);
						//macAddress.Print (gLogMessageRaw);
					}
					else
					{
					    uint32 ipv4Addr = ipAddress.Get();
						DsgSetEstbPrimaryIpV4Addr(htonl(ipv4Addr), (uint8 *)macAddr);
						//ipAddress.Print (gLogMessageRaw);
						//macAddress.Print (gLogMessageRaw);
					}
				}
				else
				{
					gLogMessageRaw << "Invalid IP or Mac address" << endl;
				}
			}
 			break;

		case kDsgSetDhcpV4VendorSpecificOption:
		{
			bool retVal;
			char option43buf[91]= 
			{	0x02, 0x03, 0x45, 0x43, 0x4d, 0x03, 0x08, 0x45, 
				0x43, 0x4d, 0x3a, 0x45, 0x4B, 0x54, 0x42, 0x04,  
				0x00, 0x05, 0x04, 0x56, 0x31, 0x2e, 0x30, 0x06,
				0x0b, 0x34, 0x2e, 0x30, 0x2e, 0x30, 0x61, 0x6c, 
				0x70, 0x68, 0x61, 0x34, 0x07, 0x10, 0x32, 0x2e, 
				0x34, 0x2e, 0x30, 0x2e, 0x73, 0x74, 0x62, 0x2e, 
				0x6B, 0x69, 0x6D, 0x63, 0x73, 0x34, 0x08, 0x06,
				0x30, 0x30, 0x31, 0x30, 0x31, 0x38, 0x09, 0x0b,
				0x42, 0x43, 0x4d, 0x39, 0x33, 0x33, 0x38, 0x33,
				0x45, 0x43, 0x4d, 0x0a, 0x08, 0x42, 0x72, 0x6f, 
				0x6B, 0x69, 0x6D, 0x63, 0x73, 0x12, 0x04, 0x43, 
				0x41, 0x52, 0x44
			};

			retVal = DsgSetDhcpOption43(option43buf, 91);
			
			gLogMessageRaw << "DsgSetDhcpV4Option43: " << retVal << endl;
		}
		break;

		case kDsgSetDhcpV6VendorSpecificOption:
		{
			bool retVal;
			char option17buf[120]= 
			{	0x00, 0x02, 0x00, 0x03, 0x45, 0x43, 0x4d, 0x00, 
				0x03, 0x00, 0x08, 0x45, 0x43, 0x4d, 0x3a, 0x45, 
				0x4B, 0x54, 0x42, 0x00, 0x04, 0x00, 0x00, 0x00, 
				0x05, 0x00, 0x04, 0x56, 0x31, 0x2e, 0x30, 0x00, 
				0x06, 0x00, 0x0b, 0x34, 0x2e, 0x30, 0x2e, 0x30, 
				0x61, 0x6c, 0x70, 0x68, 0x61, 0x34, 0x00, 0x07, 
				0x00, 0x10, 0x32, 0x2e, 0x34, 0x2e, 0x30, 0x2e, 
				0x73, 0x74, 0x62, 0x2e, 0x6B, 0x69, 0x6D, 0x63, 
				0x73, 0x34, 0x00, 0x08, 0x00, 0x06,	0x30, 0x30, 
				0x31, 0x30, 0x31, 0x38, 0x00, 0x09, 0x00, 0x0b,
				0x42, 0x43, 0x4d, 0x39, 0x33, 0x33, 0x38, 0x33,
				0x45, 0x43, 0x4d, 0x00, 0x0a, 0x00, 0x08, 0x42, 
				0x72, 0x6f, 0x6B, 0x69, 0x6D, 0x63, 0x73, 0x00, 
				0x12, 0x00, 0x04, 0x43, 0x41, 0x52, 0x44
			};

			retVal = DsgSetDhcpV6Option17(option17buf, 111);
			
			gLogMessageRaw << "DsgSetDhcpV6Option17: " << retVal << endl;
		}
		break;

		case kDsgSetCCMac:
		{
			if (pParameter->WasParsed())
			{
				BcmMacAddress macAddress = pParameter->AsMacAddress();
				uint8 macAddr[6];

				macAddress.Get(macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
				DsgSetCCMacAddr ((uint8 *)macAddr);					
			}
			else
			{
				gLogMessageRaw << "Invalid Mac address" << endl;
			}
		}
		break;

		#if( CABLECARD_SUPPORT )
		case kSendCannedDsgDir:
			{
				/*uint8 dsgDirMsgPdu[] =
				{
					0x01, 0x03, 0x06, 0x01, 0x02, 0x00, 0x02, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xef, 0x01, 0x01, 0x12, 0x00, 0x00, 0xff, 0xff,
					0x00, 0x01, 0x02, 0x00, 0x05, 0x01, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xef, 0x01, 0x01, 0x64, 0x00, 0x00, 0xff, 0xff, 0x00, 0x04,
					0x02, 0x13, 0x89, 0x01, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x01, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0xef, 0x01, 0x01, 0xc9, 0x00, 0x00, 0xff, 0xff, 0x00, 0x04, 0x02, 0x00,
					0x01, 0x01, 0x06, 0x00, 0x01, 0x00, 0x12, 0x34, 0x50, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0xef, 0x01, 0x01, 0xca, 0x00, 0x00, 0xff, 0xff, 0x00, 0x04, 0x02, 0x00, 0x9f, 0x01,
					0x07, 0x00, 0x01, 0x00, 0x12, 0x35, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0xef, 0x01, 0x01, 0xcb, 0x00, 0x00, 0xff, 0xff, 0x00, 0x01, 0x02, 0x00, 0x01, 0x02, 0x04, 0x03,
					0x00, 0x01, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xef,
					0x01, 0x01, 0x0c, 0xc0, 0x00, 0xc0, 0x01, 0x03, 0x00, 0x01, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xef, 0x01, 0x01, 0x0c, 0xc7, 0x77, 0xc7, 0x77, 0x04,
					0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xef,
					0x01, 0x01, 0x0d, 0xc0, 0x00, 0xc0, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xef, 0x01, 0x01, 0x0d, 0xc7, 0x77, 0xc7, 0x77, 0x02,
					0x18, 0x7f, 0x5c, 0xc0, 0x18, 0xda, 0xea, 0x40, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00
				};*/

				uint8 dsgDirMsgPdu[] =
				{
					0xfe, 0x06, 0x01, 0x03, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x00, 0x5e, 0x0a, 0x0a, 0x65, 0x00, 
					0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00,
					0x00, 0x01, 0x0a, 0x31, 0x40, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				};

				BcmCableCardIfThread *pCableCardIfThread = BcmCableCardIfThread::GetSingletonInstance();
				if(pCableCardIfThread == NULL)
				{
					pCableCardIfThread = new BcmCableCardIfThread;
				}

				pCableCardIfThread->HandleConfigureDsgDirectory(dsgDirMsgPdu, sizeof(dsgDirMsgPdu));
			}
			break;
		#endif

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }

}


void BcmDsgCommandTable::Scan( bool enable )
{
	BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();
	if(pDsgClientCtlThread)
	{
		// serialize the function call by sending kTestDsgRxDCD to our msg q.
		pDsgClientCtlThread->SerializePublicInterfaceCall( kTestDsgScan, (void*) enable);
	}
}

void BcmDsgCommandTable::SetMode( uint32 mode )
{
	BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();
	if(pDsgClientCtlThread)
	{
		// serialize the function call by sending kTestDsgRxDCD to our msg q.
		pDsgClientCtlThread->SerializePublicInterfaceCall( kTestDsgSetDSGMode, (void*) mode);
	}
}

void BcmDsgCommandTable::EnableDocsis( bool enable )
{
	BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();
	if(pDsgClientCtlThread)
	{
		// serialize the function call by sending kTestDsgRxDCD to our msg q.
		pDsgClientCtlThread->SerializePublicInterfaceCall( kTestDsgEnableDocsis, (void*) enable);
	}
}

void BcmDsgCommandTable::SetTimer( uint8 timer_id, uint16 timeout )
{
	BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();
	if(pDsgClientCtlThread)
	{
		uint32 timerSetting = timer_id;
		timerSetting = (timerSetting << 16) | timeout;

		// serialize the function call by sending kTestDsgRxDCD to our msg q.
		pDsgClientCtlThread->SerializePublicInterfaceCall( kTestDsgSetTimer, (void*) timerSetting);
	}
}

void BcmDsgCommandTable::GetTimer( uint8 timer_id )
{
	BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();
	if(pDsgClientCtlThread)
	{
		// serialize the function call by sending kTestDsgRxDCD to our msg q.
		pDsgClientCtlThread->SerializePublicInterfaceCall( kTestDsgGetTimer);
	}
}

void BcmDsgCommandTable::DsgStatus ( void )
{
	BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();
	if(pDsgClientCtlThread)
	{
		// serialize the function call by sending kTestDsgRxDCD to our msg q.
		pDsgClientCtlThread->SerializePublicInterfaceCall( kTestDsgStatus);
	}
}

void BcmDsgCommandTable::DocsisStatus ( void )
{
	BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();
	if(pDsgClientCtlThread)
	{
		// serialize the function call by sending kTestDsgRxDCD to our msg q.
		pDsgClientCtlThread->SerializePublicInterfaceCall( kTestDsgDocsisStatus);
	}
}

#ifdef APPNAME_SUPPORT
bool BcmDsgCommandTable::AddTestClientAppType(BcmString &name)
{
	static uint32 port = kDummyAppNameClientPortNumber;
	dsgClientRegInfo regInfo;
	memset(&regInfo, 0, sizeof(regInfo));	

	regInfo.idType = kApplicationNameType;
	strcpy( regInfo.clientName, name.c_str());

#ifdef DSG_RPC_INCLUDED
	#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
		regInfo.clientPort = port++;
	#endif
#else
	// This will assign a test port number between 0 and 255 which are
	// known not to be valid callback addresses
	regInfo.clientPort = singleProcessModelPortNumber++;
#endif

	if( BcmDsgClientRegistrarThread::GetSingletonInstance()->addDsgClient(&regInfo) == DSGCC_SUCCESS)
	{
		return true;
	}
	
	return false;
}
#endif

bool BcmDsgCommandTable::AddTestClientMacAddress(const BcmMacAddress &macAddress)
{
	static uint32 port = kDummyMacAddrClientPortNumber;
	dsgClientRegInfo regInfo;
	memset(&regInfo, 0, sizeof(regInfo));

#ifdef DSG_RPC_INCLUDED
	#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
		regInfo.clientPort = port++;
	#endif
#else
	// This will assign a test port number between 0 and 255 which are
	// known not to be valid callback addresses
	regInfo.clientPort = singleProcessModelPortNumber++;
#endif
	//BcmMacAddress macAddress(0x01, 0x00, 0x5e, 0x00, 0x01, 0x13);

	uint8 u0, u1, u2, u3, u4, u5;
	macAddress.Get(u0, u1, u2, u3, u4, u5);
	regInfo.clientName[0] = u0;
	regInfo.clientName[1] = u1;
	regInfo.clientName[2] = u2;
    regInfo.clientName[3] = u3;
	regInfo.clientName[4] = u4;
	regInfo.clientName[5] = u5;

	regInfo.idType = kMacAddressType;

	if( BcmDsgClientRegistrarThread::GetSingletonInstance()->addDsgClient(&regInfo) == DSGCC_SUCCESS)
	{
		return true;
	}
	
	return false;
}
 
// These methods are implemented for debugging purposes.

bool BcmDsgCommandTable::AddTestClientUint16(const uint32 type, const uint16 id, dsg_classifier *dsgClassifier)
{
	static uint32 port = kDummyUint16ClientPortNumber;
	dsgClientRegInfo regInfo;
	memset(&regInfo, 0, sizeof(regInfo));	

	regInfo.idType = type;
	regInfo.clientId = id;
#ifdef DSG_RPC_INCLUDED
	#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
		regInfo.clientPort = port++;
	#endif
#else
	// This will assign a test port number between 0 and 255 which are
	// known not to be valid callback addresses
	regInfo.clientPort = singleProcessModelPortNumber++;
#endif

	if( BcmDsgClientRegistrarThread::GetSingletonInstance()->addDsgClientWithClassifier(&regInfo, dsgClassifier))
	{
		return true;
	}

	return false;
}

// This is a helper method that prompts the user for input for the DHCP settings
// to be stored in nonvol.
//
// Parameters:
//      settings - reference to the nonvol settings object to be modified.
//
// Returns:  Nothing.
//
static bool PromptForClassifierSettings(BcmIpClassifierSettings &ipClassifierSets)
{
    BcmIpAddressCommandParameter ipAddrParm;
    BcmBoolCommandParameter answer(0, false, NULL, true);
	BcmUnsignedIntCommandParameter portNumParm;

	
		// Canned classifier
		uint8 ipClassifier[] = {	  0x05, 0x04, 0xe6, 0x00, 0x01, 0x09, 
									  0x03, 0x04, 0x0b, 0x18, 0x04, 0x6b,
									  0x09, 0x02, 0x11, 0x11,
									  0x0a, 0x02, 0x33, 0x33
		};

//		BcmIpClassifierSettings ipClassifierSets;

		// Wrap the message in an Octet buffer so that the receiving entity gets it
		// in a more usable form.
		BcmOctetBuffer *pOctetBuffer;

		// Wrap a BcmOctetBuffer object around the raw buffer.
		pOctetBuffer = new BcmOctetBuffer(sizeof(ipClassifier));

		// Copy the packet data.
		if( pOctetBuffer->AddToEnd((uint8 *) ipClassifier, sizeof(ipClassifier)) )
		{
			pOctetBuffer->SetBytesUsed(sizeof(ipClassifier));
		}


		// For controlling message log output.
		BcmMessageLogSettings messageLogSettings;
		ErrorSetList err_set_list;

		messageLogSettings.SetEnabledSeverities(0xffffffff); 

		// Override the class name given by my parent.
		messageLogSettings.SetModuleName("clsfr_test");
		unsigned int offset = 0;

		byte confCode;

		// Lets parse the message.
		confCode = ipClassifierSets.ParseFrom((*pOctetBuffer), offset, err_set_list, &messageLogSettings, false);

		if(confCode != kConfOk)
		{
			cout <<" FAILED TO PARSE THE DEFAULT CLASSIFIER" << endl;
		}	
		//else
		//{
			//gLogMessageRaw <<ipClassifierSets << endl;
		//}

    gLogMessageRaw 
        << "\nChange them to:  (Just hit enter to leave a setting alone)\n\n";

	// Source IP address
    ipAddrParm.SetAsIpAddress(ipClassifierSets.SourceIpAddr());
    ipAddrParm.PromptUser("Source IP address:");
	if( ipAddrParm.AsIpAddress() != kAllZerosIp )
	{
		// Set classifier.
		ipClassifierSets.SourceIpAddr(ipAddrParm.AsIpAddress());
	}
	else
	{
		ipClassifierSets.ClearSourceIpAddr();
	}

	// Source IP Mask
    ipAddrParm.SetAsIpAddress(ipClassifierSets.SourceIpMask());
    ipAddrParm.PromptUser("Source IP Mask:");
	if( ipAddrParm.AsIpAddress() != kBroadcastIp )
	{
		// Set classifier.
		ipClassifierSets.SourceIpMask(ipAddrParm.AsIpAddress());
	}
	else
	{
		ipClassifierSets.ClearSourceIpMask();
	}

	// Destination IP address
    ipAddrParm.SetAsIpAddress(ipClassifierSets.DestIpAddr());
    ipAddrParm.PromptUser("Destination IP address:");
	if( ipAddrParm.AsIpAddress() != kAllZerosIp )
	{
		// Set classifier.
		ipClassifierSets.DestIpAddr(ipAddrParm.AsIpAddress());
	}
	else
	{
		ipClassifierSets.ClearDestIpAddr();
	}

	// Destination port range
    portNumParm.SetAsUnsignedInt(ipClassifierSets.DestPortStart());
    portNumParm.PromptUser("Destination Port Start:");
	if( portNumParm.AsUnsignedInt() != 0 )
	{
		// Set classifier.
		ipClassifierSets.DestPortStart(portNumParm.AsUnsignedInt());

		portNumParm.SetAsUnsignedInt(ipClassifierSets.DestPortEnd());
		portNumParm.PromptUser("Destination Port End:");

		if( portNumParm.AsUnsignedInt() != 0 )
		{
			if (ipClassifierSets.DestPortStart() > portNumParm.AsUnsignedInt())
			{
				gLogMessageRaw <<"ERROR - DestPortStart > DestPortEnd " << endl;
				return false;
			}

			// Set classifier.
			ipClassifierSets.DestPortEnd(portNumParm.AsUnsignedInt());
		}
		else
		{
			// If starting port is specified, we do not allow port end to be zero.
			ipClassifierSets.DestPortEnd(ipClassifierSets.DestPortStart());
		}
	}
	else
	{
		ipClassifierSets.ClearDestPortStart();
		ipClassifierSets.ClearDestPortEnd();
	}

	//gLogMessageRaw <<ipClassifierSets << endl;
    gLogMessageRaw << endl;

	return true;
}




#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
// Calculate the 16 bit checksum over the buffer passed in for
// the number of BYTES passed in as a length.  Code will actually
// compute the checksum over (Length/2) USHORTS, but because we are
// little endian we'll take the buffer in as UCHAR's and convert.
// Also, this is necessary for odd Length...
static USHORT CalculateChecksum (PUCHAR pucData, int Length)
{
  int Index = 0;
  USHORT usWord, usChecksum;
  ULONG  ulTotal = 0;


  while (Index < Length)
  {
    usWord = (pucData[Index++] << 8) & 0xFF00;

    if (Index < Length) // handle possibility of odd # of bytes
      usWord |= pucData[Index++];

    ulTotal += usWord;
  }

  // Don't understand what is being done here!!! But I found this
  // code fragment on the net and it works...
  ulTotal = (ulTotal & 0xffff) + (ulTotal>>16);
  ulTotal += (ulTotal >> 16);

  usChecksum = ~ulTotal & 0xFFFF;

  return usChecksum;
}

bool BcmDsgCommandTable::Ping( const BcmIpAddress &ipAddress, uint32 numberOfPings)
{
	unsigned long targetIpAddress = 0;
	uint32 rc = 0;
	int sock;
    struct sockaddr_in name;

	typedef struct _ICMP_HDR
	{
	  UCHAR  ucType;
	  UCHAR  ucCode;
	  USHORT usChecksum;
	  USHORT usId;
	  USHORT usSequence;
	} ICMP_HDR, *PICMP_HDR;

	typedef struct _ICMP_ECHO_PKT
	{
	  ICMP_HDR        IcmpHdr;
	  UCHAR           pucEchoData[128]; // maximum for our purposes. Really this can be as big as you like.
	} ICMP_ECHO_PKT, *PICMP_ECHO_PKT;

    ICMP_ECHO_PKT PingPkt;
    int PktSize;
    USHORT usChecksum;
    UCHAR pucPingData[] = //"Broadcom DCT-5000 MAC stub is a-pingin' ya!";
	  // Data from captured ping packet, above.
	  {
		0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
		0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,
		0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x61,
		0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69
	  };

	ipAddress.Get(targetIpAddress);

	PingPkt.IcmpHdr.ucType = 0x08; // ECHO=8
	PingPkt.IcmpHdr.ucCode = 0x00;
	PingPkt.IcmpHdr.usChecksum = 0x0000; // initially for cksm calculation
	PingPkt.IcmpHdr.usId = htons (0x0100); //(0xDADA); // arbitrary ID
	PingPkt.IcmpHdr.usSequence = htons (0x0C00); //(usSequence); // just count

	memcpy (PingPkt.pucEchoData, pucPingData, sizeof (pucPingData));

	usChecksum = htons (CalculateChecksum ((PUCHAR) &PingPkt.IcmpHdr, sizeof (ICMP_HDR) + sizeof (pucPingData)));
	PingPkt.IcmpHdr.usChecksum = usChecksum;

	PktSize = sizeof (ICMP_HDR)+ sizeof (pucPingData);

	// Setup socket
    memset(&name, 0, sizeof(name));

    // extract the addressing information into the sockaddr struct
    name.sin_addr.s_addr = htonl(targetIpAddress);
    name.sin_port = 1400;
    name.sin_family = AF_INET;

    sock = socket (AF_INET, SOCK_RAW, 1); // ICMP = 1
    if (sock < 0)
    {
        return false;
    }

	unsigned char		*buf;
	if( (buf = (unsigned char *)malloc(PktSize)) == NULL)
	{
		printf("malloc failure!\n");
		return false;
	}


	rc = sendto(sock, (char*)&PingPkt, PktSize, 0,  (sockaddr*) &name, sizeof(name));

	if ( rc < 0 )
	{
		gLogMessageRaw << "Socket sendto() error:  " << strerror(errno) << endl;
		return false;
	}
	else
	{
		struct sockaddr_in	frominet;
		int					inetsize;
		int					cnt;
		
		bzero((char*)&frominet, sizeof(frominet));

		inetsize = sizeof(frominet); 

		struct timeval  tv;
		bzero((char *) &tv, sizeof(struct timeval));
		tv.tv_sec = 1;
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));

		// Wait for response
		cnt = recvfrom(sock, (char*)buf, PktSize, 0, (struct sockaddr*)&frominet, (socklen_t*)&inetsize);
		if( cnt == PktSize )
		{
			if( frominet.sin_addr.s_addr == htonl(targetIpAddress ) )
				gLogMessageRaw << "Rx response pkt from " << inet_ntoa(frominet.sin_addr) << " with len = " << cnt << endl;

			free (buf);
			close(sock);
			return true;
		}
		else
		{
			gLogMessageRaw << "No reply received. error:  " << strerror(errno) << endl;
		}

	}

	close(sock);
	return false;

}
#endif
