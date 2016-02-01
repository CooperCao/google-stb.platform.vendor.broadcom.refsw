//****************************************************************************
//  
// Copyright (c) 2004-2009 Broadcom Corporation
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
//  Filename:       DsgClientId.cpp
//  Author:         Pinar Taskiran, Moe Turcotte
//  Creation Date:  Dec 1, 2004
//
//****************************************************************************
//  Description:
//      This is the implementation of the Client ID class.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DsgClient.h"

#include "DsgClientId.h"
#include "EthernetPacket.h"

// Other objects of interest.
#include "Event.h"
#include "Timer.h"
#include "CountingSemaphore.h"
#include "OperatingSystem.h"
#include "OperatingSystemFactory.h"

#include "SystemTime.h"
#include "SocketFactory.h"

//#include "dsgClientCallback.h"

#if defined(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC)
extern "C"
{
    extern int8 BcmSharedMemoryCreate(uint32 key, uint8 owner);
    extern int8 BcmSharedMemoryDelete(uint32 shm_id);
    extern int8 BcmSharedMemoryWrite(uint32 shm_id, char *buffer, uint32 size);
    extern int8 BcmSharedMemoryRead(uint32 shm_id, char *buffer, uint8 nowait);
}
#endif

#ifdef DSG_RPC_INCLUDED
#include "dsgccClientCallback_rpc.h"
#include "dsgccClientCallback_rpcClient.h"
#endif
//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Initializes the state of the object...
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmDsgClient::BcmDsgClient(void) :
   
    fClientId( 0 ),
	fIdType (0),
	fClientAddr(kAllZerosMac),
	fClientProgNum( 0 ),
	fClientPort( 0 ),
	fActive(false),
	fTunnelId(0),
	fToken(0),
	pfSocket(NULL),
	fPacketCount(0),
	fNumberOfOctets(0),
    fIpClassifierSets(),
    fIpClassifierSetsSpecified( false ),
	fRemoveUdpHeader( false ),
	fTunnelStatus( kDsgccTunnelRequestPending )
{
}

// Registration Constructor.  Sets the state of the object based on
// values passed in registration
//
// Parameters:  struct dsgClientRegInfo.
//
// Returns:  N/A
//
//  
BcmDsgClient::BcmDsgClient(const struct dsgClientRegInfo *regInfo, dsg_classifier* dsgClassifier) :

    fClientId( 0 ),
	fIdType (0),
	fClientAddr(kAllZerosMac),
	fClientProgNum( 0 ),
	fClientPort( 0 ),
	fActive(false),
	fTunnelId(0),
	fToken(0),
	pfSocket(NULL),
	fPacketCount(0),
	fNumberOfOctets(0),
    fIpClassifierSets(),
    fIpClassifierSetsSpecified(false),
	fRemoveUdpHeader(false),
	fTunnelStatus( kDsgccTunnelRequestPending )
{
	// Start with a clean object and fill in values
	fClientPort = regInfo->clientPort;
	fClientProgNum = regInfo->clientProgNum;

	#if defined(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC)
			uint32 sharedMemoryKey = (uint32) fClientPort;
	#endif

	// Get the classifier settings
	if(dsgClassifier)
	{
		// Now store the classifier's settings for this tunnel.
		BcmIpAddress ipAddress(kAllZerosIp);

		// Source IP Address
		if( dsgClassifier->source_ip_used)
		{
			// Set ipAddress object 
			ipAddress.NetworkSet((uint32)dsgClassifier->source_ip_adr);

			// Set classifier.
			fIpClassifierSets.SourceIpAddr(ipAddress);
		}

		// Source IP Mask
		if( dsgClassifier->source_ip_mask_used)
		{
			// Set ipAddress object 
			ipAddress.NetworkSet((uint32)dsgClassifier->source_ip_mask);

			// Set classifier.
			fIpClassifierSets.SourceIpMask(ipAddress);
		}

		// Destination IP Address
		if( dsgClassifier->dest_ip_used)
		{
			// Set ipAddress object 
			ipAddress.NetworkSet((uint32)dsgClassifier->dest_ip_adr);

			// Set classifier.
			fIpClassifierSets.DestIpAddr(ipAddress);
		}

		// Destination TCP/UDP Port Start
		if( dsgClassifier->port_used)
		{
			fIpClassifierSets.DestPortStart(NetworkToHost(dsgClassifier->port_start));
			
			fIpClassifierSets.DestPortEnd(NetworkToHost(dsgClassifier->port_end));
		}

		if( fIpClassifierSets.AnythingSpecified() )
			fIpClassifierSetsSpecified = true;			
	}

	// 16 MSBs of idType indicate the packet format the client expects to receive its data in.
	uint32 removeUdpHeader = regInfo->idType;
	removeUdpHeader = (removeUdpHeader >> 16);
	if( removeUdpHeader == 1 )
		fRemoveUdpHeader = true;

	// 16 LSBs of idType indicate the client's ID type.
	fIdType = (uint16) regInfo->idType;
	switch (fIdType) 
	{

		case kMacAddressType:
			{
				#ifdef NOTDEF
					printf("DSGCC Registrar: registering a Client using well-known MAC addr");
					printf(" %02x:%02x:%02x:%02x:%02x:%02x\n",
					  regInfo->clientName[0], regInfo->clientName[1],
					  regInfo->clientName[2], regInfo->clientName[3],
					  regInfo->clientName[4], regInfo->clientName[5]) ;
					printf("\t to receive data on UDP Port %d.\n", regInfo->clientPort);
				#endif

				BcmMacAddress address((const byte *) regInfo->clientName);
				fClientAddr = address;
			}

		break;

		case kCaSystemType:
			{
				#ifdef NOTDEF
					printf("DSGCC Registrar: registering a Client using CAS ID %d, \n",
					  regInfo->clientId);
					printf("\t to receive data on UDP Port %d.\n", regInfo->clientPort);
				#endif

				fClientId = regInfo->clientId;
			}

		break;

		case kApplicationType:
			{
				#ifdef NOTDEF
					printf("DSGCC Registrar: registering a Client using Application ID %d\n",
					  regInfo->clientId);
					printf("\t to receive data on UDP Port %d.\n", regInfo->clientPort);
				#endif

				fClientId = regInfo->clientId;
			}
		break;

		case kBroadcastType:
			{
				#ifdef NOTDEF
					printf("DSGCC Registrar: registering a Client for Broadcast traffic\n");
					printf("\t to receive data on UDP Port %d.\n", regInfo->clientPort);
				#endif
				fClientId = regInfo->clientId;
			}
		break;

		case kApplicationNameType:
		{
			#ifdef NOTDEF
				printf("DSGCC Registrar: registering a Client using Application name %s\n",
				  regInfo->clientName);
				printf("\t to receive data on UDP Port %d.\n", regInfo->clientPort);
			#endif
			#ifdef APPNAME_SUPPORT
			BcmString name(regInfo->clientName);
			fClientName = name;
			#else
			printf("DSGCC Registrar: Application name type client not supported.\n");
			#endif
		}
		break;

		case kVendorType:
			{
					#ifdef NOTDEF
					printf("DSGCC Registrar: registering a Client using Vendor ID %d\n",
					  regInfo->clientId);
					printf("\t to receive data on UDP Port %d.\n", regInfo->clientPort );
					#endif

				fClientId = regInfo->clientId;
			}
		break;

		default:
			cout <<"Client ID Type is not recognized" << endl;
	}

	#if defined(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC)
		//  Initialize shared memory only for real clients.
		//  If client port is > 65535 then the client is a dummy console client.
		if( fClientPort <= 65535 )
			fSharedMemoryId = BcmSharedMemoryCreate(sharedMemoryKey, 0);
	#endif
}

// Copy Constructor.  Initializes the state of this object to match that
// of the instance passed in.
//
// Parameters:
//      otherInstance - reference to the object to copy.
//
// Returns:  N/A
//
BcmDsgClient::BcmDsgClient(const BcmDsgClient &otherInstance) 
{
    // Copy the members from the other instance.
    // Copy the members from the other instance.
    fClientId = otherInstance.fClientId;
	fIdType = otherInstance.fIdType;
	fClientAddr = otherInstance.fClientAddr;
	fClientProgNum = otherInstance.fClientProgNum;
	fClientPort = otherInstance.fClientPort;
	fActive = otherInstance.fActive;

	fClientName = otherInstance.fClientName;
	fTunnelId = otherInstance.fTunnelId;
	fToken = otherInstance.fToken;

	pfSocket = otherInstance.pfSocket;
	fPacketCount = otherInstance.fPacketCount;
	fNumberOfOctets = otherInstance.fNumberOfOctets;

    fIpClassifierSets = otherInstance.fIpClassifierSets;
    fIpClassifierSetsSpecified = otherInstance.fIpClassifierSetsSpecified;
	fRemoveUdpHeader = otherInstance.fRemoveUdpHeader;
	fTunnelStatus = otherInstance.fTunnelStatus;
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDsgClient::~BcmDsgClient()
{
    #if defined(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC)
		//  Initialize shared memory only for real clients.
		//  If client port is > 65535 then the client is a dummy console client.
		if( fClientPort <= 65535 )
			BcmSharedMemoryDelete(fSharedMemoryId);
    #else
		CloseSocket();
    #endif
}


// Assignment operator.  Copies the state of the instance passed in so that
// this object is "identical".
//
// Parameters:
//      otherInstance - reference to the object to copy.
//
// Returns:
//      A reference to "this" so that operator = can be chained.
//
BcmDsgClient & BcmDsgClient::operator = (const BcmDsgClient &otherInstance)
{
    // Copy protected and private members of otherInstance to my members.
    fClientId = otherInstance.fClientId;
	fIdType = otherInstance.fIdType;
	fClientAddr = otherInstance.fClientAddr;
	fClientProgNum = otherInstance.fClientProgNum;
	fClientPort = otherInstance.fClientPort;
	fActive = otherInstance.fActive;

	fClientName = otherInstance.fClientName;
	fTunnelId = otherInstance.fTunnelId;
	fToken = otherInstance.fToken;

	pfSocket = otherInstance.pfSocket;

	fPacketCount = otherInstance.fPacketCount;
	fNumberOfOctets = otherInstance.fNumberOfOctets;

    fIpClassifierSets = otherInstance.fIpClassifierSets;
    fIpClassifierSetsSpecified = otherInstance.fIpClassifierSetsSpecified;
	fRemoveUdpHeader = otherInstance.fRemoveUdpHeader;
	fTunnelStatus = otherInstance.fTunnelStatus;

	cout <<"Assignment Operator - pfSocket = " << hex << pfSocket 
							<< "otherInstance.pfSocket = " << hex << otherInstance.pfSocket << endl;

    return *this;
}

bool BcmDsgClient::operator==(
  const BcmDsgClient& rhs ) const
{
    if( fIdType != (uint16)rhs.IdType() )
    {	
        return false;
    }	

	if( fClientPort != rhs.ClientPort() )
    {
        return false;
    }

	if( fIdType == kMacAddressType )
	{
		if( fClientAddr != (rhs.ClientAddress()) )
		{
			return false;
		}
	}
	else if( fIdType == kApplicationNameType )
	{
		if( fClientAddr != (rhs.ClientAddress()) )
		{
			return false;
		}
	}
	else
	{
		if( fClientId != (uint16)rhs.Id() )
		{

			return false;
		}
	}

	if( fIpClassifierSetsSpecified )
	{
		cout
			<<" Client ID = " << fClientId
			<<" Client has fIpClassifierSetsSpecified" <<endl;
	}
	//#warning "Client comparison operator - Compare fIpClassifierSetsSpecified?? "

    return true;
}
  

bool BcmDsgClient::NameMapped(void) const
{
	if( (fIdType ==kApplicationNameType) && (fClientId == 0) )
	{
		return false;
	}

	return true;
}

ostream& BcmDsgClient::Print( ostream& ostrm ) const
{    
       
#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm 
        << "\n=========================\n"
        << "CLIENT SETTINGS:" << endl;
        

	if( fIdType == kMacAddressType )
	{
		ostrm
			<< "\n TYPE = " << IdTypeName((uint32)fIdType) <<" ID = " << fClientAddr;
	}
	else if( fIdType == kApplicationNameType) 
	{
		ostrm
			<< "\n TYPE = " << IdTypeName((uint32)fIdType) <<" Name = " << fClientName
			<<"\n\t\t ID = " << (uint32)fClientId;
	}
	else
	{
		ostrm
			<< "\n TYPE = " << IdTypeName((uint32)fIdType)  <<" ID = 0x" << hex << (uint32)fClientId << dec;
	}

	ostrm
		<< "\n Port         = " << (uint32)fClientPort  
		//<< "\n IsActive     = " << (uint32)fActive  
		<< "\n Tunnel ID    = " << (uint32)fTunnelId  
		<< "\n Name         = " << (!fClientName.empty()?fClientName.c_str():"N/A")
		//<< "\n Token        = " << (uint32)fToken 
		<< "\n Packet count = " << fPacketCount
		<< "\n Octet count  = " << fNumberOfOctets	
		<< "\n Classifier Settings = " << (fIpClassifierSetsSpecified?"specified":"not specified")
		<< "\n Remove Udp Header = " << (fRemoveUdpHeader?"true":"false") 
		<< "\n Tunnel Status = " << TunnelStatusTypeName(fTunnelStatus) 

		<< endl;

	if(fIpClassifierSetsSpecified)
	{
		ostrm << fIpClassifierSets << endl;
	}

#endif

    return ostrm;
}

const char* BcmDsgClient::IdTypeName( const uint32 value ) const
{
	char* names[] = 
	{
		(char *) "",
		(char *) "[ Broadcast ]",			// = 1
		(char *) "[ Mac Address ]",			// = 2 
		(char *) "[ Ca System ]",			// = 3
		(char *) "[ Application ]",			// = 4
		(char *) "[ Application Name ]",		// = 5
		(char *) "[ Vendor ]",				// = 6
		(char *) "[ CableCard Client ]"      // = 7
	};

	return names[ value ];
}

int BcmDsgClient::UpdateTunnelStatusSendNotification(int status)
{
	fTunnelStatus = status;
	return SendNotification(status);
}
         
// 
int BcmDsgClient::SendNotification(int event, int value)
{

#ifdef DSG_RPC_INCLUDED

	#if !(BCM_REDUCED_IMAGE_SIZE)
	//  If this is a dummy client added from console then just return.
	if( fClientPort > 65535 )
	{
		return 1;
	}
	#endif

  if (fClientProgNum > 0)  // If client registers with ProgNum 0, it doesn't want notification
  {
	dsgccClientNotification clientNotification;
	clientNotification.eventType = event;
	clientNotification.eventValue = value;
	clientNotification.clientPort = fClientPort;
	
    rpc_dsgccClientNotification(fClientProgNum, &clientNotification);
  }
  
  return (1);
#else

	if ((fClientProgNum > 0) && (fClientPort >= 256) )  // If client registers with ProgNum 0, it doesn't want notification
	{
		dsgccClientNotification clientNotification;
		clientNotification.eventType = event;
		clientNotification.eventValue = value;
		clientNotification.clientPort = 0;

		void (*callback)(struct dsgccClientNotification *) = (void (*)(struct dsgccClientNotification *))fClientProgNum;
		if (callback)
				callback((struct dsgccClientNotification *) &clientNotification);
	}

  return (1);
#endif
}

// These methods are available to notify the client object that
// some status has changed. The client object will 
// notify the actual client of the change.
// Base class 
int BcmDsgClient::twoWayModeChangeNotification(int mode)
{
	return SendNotification(kTwoWayModeChange, mode);
}

// Base class 
int BcmDsgClient::advancedModeChangeNotification(int mode)
{
	return SendNotification(kAdvancedModeChange, mode);
}

// Base class 
int BcmDsgClient::tunnelPresenceChangeNotification(int presence)
{
	//return SendNotification(kTunnelPresenceChange, presence);
}

// Base class 
int BcmDsgClient::DsgDsScanCompleteNotification( void )
{
    return 1;
}

// Base class 
int BcmDsgClient::DsgUcid2wayOKNotification( uint8 ucid )
{
    return 1;
}

// Base class 
int BcmDsgClient::DsgDccDepartNotification( uint8 init_type )
{
    return 1;
}

int BcmDsgClient::DsgTwoWayForwardingDisabledNotification( uint8 twoWayForwardingLimitations )
{
    return 1;
}

void BcmDsgClient::Token(uint8 &token) 
{
	fToken = token;
}

uint8 &BcmDsgClient::Token(void) 
{
	return fToken;
}

void BcmDsgClient::TunnelId( const uint32 new_val ) 
{ 
  fTunnelId = new_val; 
}
  

int BcmDsgClient::ProcessTunnelPacket(BcmEthernetPacket &ethernetPacket) 
{
	const uint8 *pkt = NULL;
	uint16 pktLen = 0;

	#ifdef DCAS_BT_SUPPORT

		// Check if client requested header stripping.
		if( fRemoveUdpHeader )
		{
			if( ethernetPacket.ParseTcpUdp() && (ethernetPacket.IpProtocolTypeParsed() == BcmEthernetPacket::kUdp))
			{
				// Remove BT header if this is a broadcast tunnel packet
				if( fIdType == kBroadcastType )
				{
					pkt = ethernetPacket.UdpPdu() + 4;					// Broadcast tunnel header is 4 bytes.
					pktLen = ethernetPacket.UdpLengthBytes() - (8 + 4); //ethernetPacket.UdpLengthBytes() includes UDP header length. 
																		// 8 = UDP header lenght   4 = BT header length
				}
				else
				{
					pkt = ethernetPacket.UdpPdu();
					pktLen = ethernetPacket.UdpLengthBytes() - 8;
				}
			}
			else
			{
				return 0;
			}
		}
		else

	#endif

		{
			pkt = ethernetPacket.Buffer();
			pktLen = ethernetPacket.PacketLength();
		}
	
	TunnelPacket(pkt, pktLen);
}


int BcmDsgClient::TunnelPacket(const unsigned char *pkt, uint16 pktLen)
{
	fPacketCount++;
	fNumberOfOctets += pktLen;

	#if defined(CLIENT_INTERFACE_SUPPORT)
		
		#if !(BCM_REDUCED_IMAGE_SIZE)
		#ifdef DSG_RPC_INCLUDED
		//  If this is a dummy client added from console then just return.
		if( fClientPort > 65535 )
		{
			return 1;
		}
		#endif
		#endif

		#ifdef DSG_RPC_INCLUDED
			#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE) || defined(BCM_DSG_SINGLE_PROCESS_INTERFACE)

				#if defined(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC)
					(void)BcmSharedMemoryWrite(fSharedMemoryId, (char *) pkt, (uint32) pktLen);
				#else
				// If dual processor or dual process then use the socket interface
				BcmIpAddress dsgClientIpAddress(0x7f, 0x00, 0x00, 0x01);

				// if we don't have a socket, get one
				if (pfSocket== NULL) 
				{	
					pfSocket = BcmSocketFactory::NewSocket(AF_INET, SOCK_DGRAM, 0, 1);
					int rc = pfSocket->Connect(dsgClientIpAddress, (uint16)fClientPort); 

					fActive = true;
				}

				return (pfSocket->Send((char *) pkt, (int) pktLen, 0));
				// if (n < 0)
				// I think that ECONNREFUSED is the only errno that we could
				// possibly get, so why bother checking for it? Just delete
				// me from the client list. This will actually be done by
				// whoever called me
				#endif
			#endif

		#else

			// If single process (vxWorks) then assume the port number actually contains the
			// call back address. NOTE: In the single process model, port numbers below 256
			// are used for test purposes and do not represent actual callback addresses.
			if (fClientPort >= 256) {
				int (*callback)(char *, int) = (int (*)(char *, int))fClientPort;
				if (callback)
					return callback((char *) pkt, (int) pktLen);
			}
			return 1;

		#endif

	#else 
		
		//cout <<"Sending DSG PKT to PORT = " << fClientPort << endl;
		return 1;
	#endif
}

void BcmDsgClient::CloseSocket(void)
{
	if(pfSocket)
	{
		cout <<"pfSocket NOT NULL. Deleting  " << hex << pfSocket << endl;
		delete pfSocket;
		pfSocket = NULL;
	}
}

void BcmDsgClient::ClientAddress(BcmMacAddress &id)
{
	fClientAddr = id;
}

void BcmDsgClient::ClientName(const BcmOctetBuffer &clientName)
{
    fClientName = (char*) clientName.AccessBuffer();
}

void BcmDsgClient::TunnelStatus( const uint32 new_val ) 
{ 
	fTunnelStatus = new_val;
}

const char* BcmDsgClient::TunnelStatusTypeName( uint32 value ) const
{
	char* names[] =
	{
		(char *) "",
		(char *) "TUNNEL REQUEST PENDING",	// = 1
		(char *) "TUNNEL OPEN",              // = 2
		(char *) "TUNNEL NOT AVAILABLE",		// = 3
		(char *) "TUNNEL PENDING"            // = 4
	};

	return names[ value ];
}
	
