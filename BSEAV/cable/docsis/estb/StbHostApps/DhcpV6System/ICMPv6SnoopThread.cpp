//****************************************************************************
//
// Copyright (c) 2008-2012 Broadcom Corporation
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
//  Filename:       ICMPv6SnoopThread.cpp
//  Author:         Karthik Balasubramanian
//  Creation Date:  March 31, 2008
//
//  Description:
//
//  Thread that uses libpcap to snoop on ICMPv6 packets.
//
//****************************************************************************


//********************** Include Files ***************************************

// My api and definitions...
#include "ICMPv6SnoopThread.h"

#include "NeighborDiscoveryThread.h"
#include "BaseIpHalIf.h"
#include "DhcpV6ClientThread.h"
#include "DhcpV6ClientSoftTimer.h"
#include "DhcpV6StatusCodeOption.h"
#include "DhcpV6Packet.h"

#include "OperatingSystemFactory.h"
#include "MutexSemaphore.h"
#include "EventPublisher.h"
// PR13544
#include "MsgPublisherOneToMany.h"
#include <stdlib.h>

//********************** defines ***************************************
#define IP_HDR_TYPE_ICMP  0x3a

static BcmNeighborDiscoveryThread * ptrNeighborDiscoveryThread = NULL;
//********************** structure definitions ***************************************
#if 0
typedef struct
{
	unsigned char  source_mac_addr[6];
	unsigned char  dest_mac_addr[6];
	unsigned short type;
}ethHdr;

typedef struct
{
	union 
	{
		struct ip6_hdrctl 
		{
	        unsigned int   tflow_tflowlabel;
            unsigned short payload_length;
	        unsigned char  next_hdr;
	        unsigned char  hop_limit;		
		} ip6_un1;
		unsigned char version;	// 4 bits version, top 4 bits class
	} ip6_ctlun;
	unsigned char  source_addr[16];
	unsigned char  dest_addr[16];
} ip6Hdr;

// ICMPv6 Packet
typedef struct
{
	unsigned char  type;			// Type
	unsigned char  code;			// Code
	unsigned short checksum;		// ICMP Checksum
	unsigned char  cur_hop_limit;	
	unsigned char  flags;
	unsigned short router_life_time;
	unsigned int   reachable_time;
	unsigned int   retrans_time;
	void *         icmpv6_options;
} icmp6Pkt;


typedef struct
{
	unsigned short int reserved;
	unsigned int mtu;
} icmp6_mtu_option;

typedef struct
{
	unsigned char prefixLength;
	unsigned char flags;
	unsigned int validLifetime;
	unsigned int preferredLifetime;
	unsigned int reserved;
	void *prefix;
}icmp6_prefix_option;


typedef struct
{
	unsigned char type;   // ICMP Option type
	unsigned char length; // Length in units of 8 bytes

	union
	{
		void *pLLAddress;    // Source Link address
		icmp6_mtu_option mtu_option;    // MTU option
		icmp6_prefix_option prefix_option; // Prefix option
	}options_un;
} icmp6_option;
#endif

static ETHERNET_PACKET* AllocatePacket( char *buf, uint32 length );

// Constructor.  Takes a string passed in and uses it as packet filtering
// criterion.
//
// You can set the thread priority as desired (defaults to normal) so that
// the thread consumes the appropriate amount of processor time.
//
// Parameters:
//      initialPriority - the priority that you want the thread to have.
//
// Returns:  N/A
//
BcmIcmpV6SnoopThread::BcmIcmpV6SnoopThread(char *snoopThreadName, char *ethInterface, BcmNeighborDiscoveryThread * pNeighborDiscoveryThread, 
										   BcmOperatingSystem::ThreadPriority initialPriority) :
											BcmThread(snoopThreadName, false, initialPriority)
{
	strcpy(fEthInterface,ethInterface);
	pfNeighborDiscoveryThread = pNeighborDiscoveryThread;
	if (pfOperatingSystem->BeginThread(this, 6144) == false)
	{
		gFatalErrorMsg( fMessageLogSettings, "BcmNeighborDiscoveryThread" ) 
			<< "Failed to spawn my thread!" << endl;
		assert(0);
	}      
	// Set this for the callback function to use.
	ptrNeighborDiscoveryThread = pNeighborDiscoveryThread;

	gAlwaysMsgNoFields(fMessageLogSettings) << " Created ICMPv6 Snoop Thread for " << ethInterface << endl;
}
// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Note that the event that was passed to us via SetNotificationEvent() will
// not be deleted!  It is the owner's responsibility to do so!
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmIcmpV6SnoopThread::~BcmIcmpV6SnoopThread()
{
    CallTrace("BcmIcmpV6SnoopThread", "~BcmIcmpV6SnoopThread");

    fExitThread = true;

    // Make sure the thread has exited.
    WaitForThread();    
}

// Thread constructor - this is the first method called after the thread has
// been spawned, and is where the thread should create all OS objects.  This
// has to be done here, rather than in the object's constructor, because
// some OS objects must be created in the context of the thread that will
// use them.  The object's constructor is still running in the context of
// the thread that created this object.
//
// Parameters:  None.
//
// Returns:
//      true if successful and ThreadMain() should be called.
//      false if there was a problem (couldn't create an OS object, etc.)
//          and ThreadMain() should not be called.
//
bool BcmIcmpV6SnoopThread::Initialize(void)
{
	// pcap stuff
    struct bpf_program fp;      
    bpf_u_int32 maskp;          
    bpf_u_int32 netp;        
    char errbuf[PCAP_ERRBUF_SIZE];
    
    CallTrace("BcmIcmpV6SnoopThread", "Initialize");

    // Grab the device to peak into
    char *ethDev = pcap_lookupdev(fEthInterface);
    if(ethDev == NULL)
    {
        gFatalErrorMsg(fMessageLogSettings, "BcmIcmpV6SnoopThread") << "Failed to open device" << fEthInterface
			<< "to capture packets" << endl;
        assert(0);
	}

    // ask pcap for the network address and mask of the device
    pcap_lookupnet(fEthInterface,&netp,&maskp,errbuf);

    // Open device for reading 
	// KB-TODO: What size packets should we wait to read?  Currently hardcoded to 1536
    pcapDeviceDescr = pcap_open_live(fEthInterface,1536,0,-1,errbuf);
    if(pcapDeviceDescr == NULL)
    { 
		gFatalErrorMsg(fMessageLogSettings, "BcmIcmpV6SnoopThread") << "pcap_open_live failed on" << fEthInterface << endl;
		return false;
	}

    // Lets try and compile the program
	// KB-TODO: Fix the filter string
    if(pcap_compile(pcapDeviceDescr,&fp,"ip6",0,netp) == -1)
    {
		gFatalErrorMsg(fMessageLogSettings, "BcmIcmpV6SnoopThread") << "pcap_compile failed on" << fEthInterface << endl;
		return false; 
	}

    // Set the compiled program as the filter
    if(pcap_setfilter(pcapDeviceDescr,&fp) == -1)
    { 
		gFatalErrorMsg(fMessageLogSettings, "BcmIcmpV6SnoopThread") << "pcap_setfilter failed on" << fEthInterface << endl;
		return false; 
	}

    return true;
}

// This is the callback function that is invoked by the pcap library when a packet that
// matches the filter we set comes in.  This function will handle the Router Advertisements
// and Duplicate Address Detect responses.  With Router Advertisements, it will extract the
// prefix length and pass it on to the core DHCP client.  With DAD messages, who knows??
void Icmpv6Handler(u_char *pIcmpV6SnoopThread, const struct pcap_pkthdr* pktHdr, const u_char* capturedPacket)
{
	BcmEthernetPacket ethPacket;
	BcmEthernetPacket *pPacket = &ethPacket;
	ETHERNET_PACKET *pEthBuffer = AllocatePacket((char *)capturedPacket, pktHdr->len);

	ethPacket.SetPacket(pEthBuffer);

    if( pPacket->ParseIcmpV6() == true )
    {
        switch( pPacket->IcmpTypeParsed() )
        {
            case BcmEthernetPacket::kIcmpV6TypeRouterAdvertisement:
				gLogMessageRaw << "Received a Router Advertisement Message!!" << endl;

				if( ptrNeighborDiscoveryThread )
					((BcmNeighborDiscoveryThread *)ptrNeighborDiscoveryThread)->AnnounceRouterAdvertisementMessage( pPacket );
				//gLogMessageRaw << *ptrNeighborDiscoveryThread << endl;

				//((BcmIcmpV6SnoopThread *)pIcmpV6SnoopThread)->ParseRouterAdvertisement(ethPacket);
                break;

			
            case BcmEthernetPacket::kIcmpV6TypeNeighborAdvertisement:
				gLogMessageRaw << "Received a Neighbour Advertisement Message!!" << endl;

				//if( ptrNeighborDiscoveryThread )
				//	((BcmNeighborDiscoveryThread *)ptrNeighborDiscoveryThread)->AnnounceNeighborAdvertisementMessage( pPacket );
                break;
		
                
			default:
				//gLogMessageRaw << "ICMPV6SNOOP: Received an Unknown Message!!" << endl;
                break;
        }
	}

	//ptrNeighborDiscoveryThread->IpHalIf()->DhcpV6ClientIf()->DhcpV6ClientThread()->ResyncTimers();
}

// This is the main body of the thread's code.  This is the only method
// that absolutely must be provided in the derived class (since there is no
// reasonable default behavior).
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmIcmpV6SnoopThread::ThreadMain(void)
{
    bool timeToExit = false;

	//gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Entering..." << endl;

    while (1)
    {
		// Loop indefinitely.  Each time a packet is received,
		// the callback function will be invoked.  However, if
		// an error occurs, this function will return -1, so
		// check for and handle error condition.
		if (pcap_loop(pcapDeviceDescr,-1,(pcap_handler)Icmpv6Handler,(u_char *)this) < 0 )
		{
			gErrorMsg(fMessageLogSettings, "BcmIcmpV6SnoopThread") << "pcap_loop returned an error on "
				<< fEthInterface << endl;
			// Not sure if we need to take some kind of reparative action here...
		}
		else
			cout << " PKT" << endl;
	}
}


// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmIcmpV6SnoopThread::Deinitialize(void)
{
    CallTrace("BcmIcmpV6SnoopThread", "Deinitialize");

    // Get rid of the OS objects that I created.
	// Nothing to do here (yet)...
}

void BcmIcmpV6SnoopThread::ParseRouterAdvertisement(BcmEthernetPacket &packet)
{
	BcmIpV6Address routerIpAddress = packet.SourceIpAddressParsed().GetIpV6Address();
	BcmRouterEntry *pRouterEntry;
	BcmBaseIpHalIf *pfIpHalIf = (BcmBaseIpHalIf *)pfNeighborDiscoveryThread->IpHalIf();

	// Create a router entry object to parse the router advertisement message.
	pRouterEntry = new BcmRouterEntry( routerIpAddress, pfIpHalIf, 0 );
	pRouterEntry->ParseRouterAdvertisement(packet);
	pRouterEntry->Print(cout);
}

ETHERNET_PACKET* AllocatePacket( char *buf, uint32 length )
{          
	char			*pPayload = NULL;
	DATA_BUFFER		*pDataBuffer = NULL;
	ETHERNET_PACKET *pEthernetPacketBuffer = NULL;

	// If 'buf' is NULL then the caller wants us to create a buffer of size 'length'.
	pPayload = buf;
	if( pPayload == NULL )
	{
		if( (pPayload = (char*)malloc( length)) == NULL)
		{
			gLogMessageRaw << " Failed to allocate ethernet buffer." << endl;
			return NULL;
		}

		memset(pPayload, 0, length);
	}

	if( ( pDataBuffer = (DATA_BUFFER *) malloc( sizeof( DATA_BUFFER )) ) == NULL)
	{
		// Ethernet driver will free the buffer 'buf' upon return.
		return NULL;
	}

	if( ( pEthernetPacketBuffer = (ETHERNET_PACKET *) malloc( sizeof( ETHERNET_PACKET )) ) == NULL)
	{
		free (pDataBuffer);

		// Ethernet driver will free the buffer 'buf' upon return.
		return NULL;
	}

	// reset buffer
	memset(pDataBuffer, 0, sizeof(pDataBuffer));
	memset(pEthernetPacketBuffer, 0, sizeof(pEthernetPacketBuffer));

	// Set DATA_BUFFER struct
	pDataBuffer->pvStart = (PVOID)pPayload; //buf;
	pDataBuffer->uiLen   = length;

	//Fill in the starting data buffer pointer address for the
	//Ethernet packet we are going to use for this data.
	pEthernetPacketBuffer->pDataBufferHead = pDataBuffer;

	//Use current FreeDataBuffPtr as last buffer in EthPktChain.
	pEthernetPacketBuffer->pDataBufferTail = pDataBuffer;
	pEthernetPacketBuffer->pNext = NULL;

	return pEthernetPacketBuffer;
}
