//****************************************************************************
//
// Copyright (c) 2009-2010 Broadcom Corporation
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
//  Filename:       DhcpV5ClientSnoopThread.cpp
//  Author:         Henry Shi
//  Creation Date:  Aug 14, 2009
//
//****************************************************************************
//  Description:

//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DhcpV4ClientSnoopThread.h"
#include "OperatingSystemFactory.h"

#if defined(TARGETOS_vxWorks)
    #include "vxIpHalIf.h"
#elif defined(TARGETOS_Linux)
    #include "LnxIpStackIf.h"	
	#include <net/if.h>	
#elif defined(PSOS)
    #include "PsosIpHalIf.h"
#elif defined(TARGETOS_eCos)
    #include "ecosIpHalIf.h"
#else
    #include "BaseIpHalIf.h"
#endif

#include "IpHalIfFactory.h"
#include "DhcpV6SocketIf.h"
#include "DhcpClientThread.h"
#include "DhcpPacket.h"
#include<linux/if_packet.h>
#include<linux/if_ether.h>

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************
static void DhcpV4HelperThread ( void );
extern unsigned int dhcpVersion;

//********************** Class Method Implementations ************************
extern "C"
{
	extern int SetupRemoteInterface(void);
}

// Default Constructor.  Starts the thread and waits for it to initialze.
// When the constructor returns, you can successfully manipulate the thread.
// Note that it is the most-derived class who actually spawns the thread and
// waits for it.
//
// For now, the thread name is hard-wired to be "Time Of Day Thread", and
// it will not start suspended.  These may be changed if we find that it is
// desireable.
//
// You can set the thread priority as desired (defaults to normal) so that
// the thread consumes the appropriate amount of processor time.
//
// Parameters:
//      initialPriority - the priority that you want the thread to have.
//
// Returns:  N/A
//
BcmDhcpV4ClientSnoopThread::BcmDhcpV4ClientSnoopThread( BcmOperatingSystem::ThreadPriority initialPriority) :
BcmThread("eSTB Remote Interface Thread", false, initialPriority)
{
    CallTrace("BcmDhcpV6ClientSnoopThread", "BcmDhcpV6ClientSnoopThread");

    fMessageLogSettings.SetModuleName("BcmDhcpV4ClientSnoopThread");

    // Set some clean default values.
    fExitThread = false;
 
	// Setup sockets for Broadcom Remote Interface.
	/*if(!SetupRemoteInterface())
	{
		gFatalErrorMsg(fMessageLogSettings, "BcmDhcpV6ClientSnoopThread") << "Failed to initialize the socket interface!" << endl;
	}*/

    // Just kick off the thread.  PR1613 - VxWorks uses almost 4k of stack for
    // some reason; not sure why.  Give it 6k so we have lots of margin.
	#if defined(TARGETOS_vxWorks) || defined(WIN32)
	if (pfOperatingSystem->BeginThread(this, 16 * 1024) == false)
	#else
	if (pfOperatingSystem->BeginThread(this, 8 * 1024) == false)
	#endif
    {
        gFatalErrorMsg(fMessageLogSettings, "BcmDhcpV6ClientSnoopThread") << "Failed to spawn my thread!" << endl;

        assert(0);
    }
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
BcmDhcpV4ClientSnoopThread::~BcmDhcpV4ClientSnoopThread()
{
    CallTrace("BcmDhcpV6ClientSnoopThread", "~BcmDhcpV6ClientSnoopThread");

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
bool BcmDhcpV4ClientSnoopThread::Initialize(void)
{
    CallTrace("BcmDhcpV4ClientSnoopThread", "Initialize");

    return true;
}


// This is the main body of the thread's code.  This is the only method
// that absolutely must be provided in the derived class (since there is no
// reasonable default behavior).
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmDhcpV4ClientSnoopThread::ThreadMain(void)
{
    CallTrace("BcmDhcpV4ClientSnoopThread", "ThreadMain");
	// Broadcom Remote Interface.
	DhcpV4HelperThread();
}


// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmDhcpV4ClientSnoopThread::Deinitialize(void)
{
    CallTrace("BcmDhcpV6ClientSnoopThread", "Deinitialize");

    // Get rid of the OS objects that I created.
}


#if 1

/*******************************************************************************
******************* DSG PACKET HELPERS ***************************************** 
******************************************************************************** 
********************************************************************************
*    Ethernet packet structure.
*
********************************************************************************
typedef struct _ETHERNET_PACKET_
{

  BcmHandle      hPacket;
  PDATA_BUFFER   pDataBufferHead;
  PDATA_BUFFER   pDataBufferTail;
  ULONG          ulCount;                // Packet usage counter
  USHORT         uiInterface;            // interface this packet came from
  USHORT         uiDestination;          // interface this packet is being sent to
  DOCSIS_EXTENDED_HEADER eHdr;           // DOCSIS Extended Header object (used for Downstream)
  struct _ETHERNET_PACKET_ *pNext;       // Reserved field for CM HAL

}    ETHERNET_PACKET
*******************************************************************************/
ETHERNET_PACKET* AllocatePacket( unsigned char *buf, uint32 length )
{          
	unsigned char			*pPayload = NULL;
	DATA_BUFFER		*pDataBuffer = NULL;
	ETHERNET_PACKET *pEthernetPacketBuffer = NULL;

	// Send this packet to final destination such as the cablecard
	// Now wrap the data buffer in ETHERNET_PACKET struct to pass to eSTB.
	//
	// Allocate memory for the data structs.
	//
	// IMPORTANT: eSTB is RESPONSIBLE for releasing the buffers by calling
	//			  DsgFreeEthernetPkt()
	//
	
	// If 'buf' is NULL then the caller wants us to create a buffer of size 'length'.
	pPayload = buf;
	if( pPayload == NULL )
	{
		if( (pPayload = (unsigned char*)malloc( length)) == NULL)
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

/*
 * This Function check whether the received packet is IP or ARP.
 */

int ParseEthernetHeader(unsigned char *packet, int len)
{
	struct ethhdr *ethernet_header;

	if(len > (int) sizeof(struct ethhdr))
	{
		ethernet_header = (struct ethhdr *)packet;

		/* First set of 6 bytes are Destination MAC */
	   if(ntohs (ethernet_header->h_proto) == 0x0806)
	   {
	    	return 1;
	    }
	    else if(ntohs(ethernet_header->h_proto) == 0x0800)
	    {
	    	return 2;
	    }
	    else
	    {
	     return -1;
	    }	
	}
	else
	{
		printf("Packet size too small !\n");
		return -1;
	}
}

/*
 * This Thread will listen on socket (68) for DHCPv4 pkts.
 */
static void DhcpV4HelperThread ( void )
{
    register int cnt;

	BcmLnxIpStackIf *pHal = NULL;
	int maxfdp1 = 0;
	struct sockaddr_ll packet_info;
	int packet_info_size;
	int result;
 	unsigned char *buffer;
	unsigned char *dhcpPacket, *arpPacket;

	struct sockaddr_ll sll;
	struct ifreq ifr;
	BcmEthernetPacket ethPacket,ethArp, ethDhcp;

	dhcpPacket = NULL;
	arpPacket = NULL;
 	ETHERNET_PACKET *pEthBuffer, *pEthArp, *pEthDhcp;

	packet_info_size = sizeof(packet_info);

    gLogMessageRaw <<"Starting DhcpHelperThread. Opening sockets and listening on port "<< inHex(67) << endl;	
	
	// Create my HALIF. I am the SOURCE HAL IF for all packets
 	BcmDhcpV6SocketIf socketHalIf( NULL );
		/*(	118, // interface number - arbitrary.  Use 118 since we have to be ifIndex 18 (why not).
								BcmHalIf::kIanaTypeOpenCableDsgClientInterface, // ifType, internal value, really return '0' per eDOCSIS section 5.2.3.1, table 5-2
								"DHCPv6 Interface",// ifDescr, per eDOCSIS section 5.2.3.1, table 5-2
								0, // ifSpeed, per eDOCSIS section 5.2.3.1, table 5-2
								0, // ifMtu, per eDOCSIS section 5.2.3.1, table 5-2
								NULL, // Wrapped HAL, if any
								BcmMacAddress(0,0,0,0,0,0));// ifPhysAddress, per eDOCSIS section 5.2.3.1, table 5-2)*/
	/////////////////  Get socket descriptors of Interfaces and set up socket set////////////////////

	gLogMessageRaw <<"*** DhcpV4HelperThread ***. Setting up socket set.."<< endl;	
    pHal = (BcmLnxIpStackIf*) BcmIpHalIfFactory::GetSingleton()->First();
    while (pHal)
    {
		// Create transmit socket.
  		pHal->CreateV4NetworkRxSocket();

		if( pHal->NetworkV4RxSocket() > 0 )
		{
			maxfdp1 = ((maxfdp1 > pHal->NetworkV4RxSocket()) ? maxfdp1 : pHal->NetworkRxSocket()) + 1;
		}

		pHal = (BcmLnxIpStackIf*) BcmIpHalIfFactory::GetSingleton()->Next(pHal);
	}
	////////////////////////////////////////////////////////////////////////////
	bzero(&sll, sizeof(sll));
	bzero(&ifr, sizeof(ifr));
    pHal = (BcmLnxIpStackIf*) BcmIpHalIfFactory::GetSingleton()->First();
	
	/* First Get the Interface Index  */
	strncpy((char *)ifr.ifr_name, "eth1", IFNAMSIZ);

	if((ioctl(pHal->NetworkV4RxSocket(), SIOCGIFINDEX, &ifr)) == -1)
	{
		gLogMessageRaw <<" Error: getting Interface index !" << (int) pHal->NetworkV4RxSocket() << endl;
	}

	/* Bind our raw socket to this interface */
	sll.sll_family = AF_PACKET;
	sll.sll_ifindex = ifr.ifr_ifindex;
	sll.sll_protocol = htons(ETH_P_ALL); 

	if((bind(pHal->NetworkV4RxSocket(), (struct sockaddr *)&sll, sizeof(sll)))== -1)
	{
		gLogMessageRaw << "Error binding raw socket to interface !" << endl;
	}

	if( (buffer = (unsigned char *)malloc(1518)) == NULL)
	{
		printf("malloc failure!\n");
	}
 	pEthBuffer = AllocatePacket(buffer, 1518);

	if( (dhcpPacket = (unsigned char *)malloc(1518)) == NULL)
	{
		printf("malloc failure!\n");
	}
	pEthDhcp =  AllocatePacket(dhcpPacket, 1518);

	if( (arpPacket = (unsigned char *)malloc(1518)) == NULL)
	{
		printf("malloc failure!\n");
	}
	pEthArp =  AllocatePacket(arpPacket, 1518);
	bzero(&packet_info, sizeof(packet_info));

	printf("*** Dhcpv4 HelperThread ***. Starting to listen on sockets...\n" );	
	while(1)
	{
		/////////////////  Process  ////////////////////
		BcmLnxIpStackIf *pIpHalIf;

		// Check whether DHCP V4 or V6 is starting
    	if(dhcpVersion != 1)
		{
			// If V6 started, do nothing and yield to another thread.
			BcmOperatingSystemFactory::ThreadSleep(0);
			//gLogMessageRaw <<"Dhcp V4 not started, return. "<< endl;	
		}
		else
		{
			pIpHalIf = (BcmLnxIpStackIf*) BcmIpHalIfFactory::GetSingleton()->First();

			cnt = recvfrom(pIpHalIf->NetworkV4RxSocket(), buffer,1518,0, (struct sockaddr *) &packet_info, (socklen_t *) &packet_info_size); //  buf, buflen);					
			if (cnt <= 0 ) 
			{ 
				gLogMessageRaw << "Dhcp V4 client: Socket recv() failure detected." << endl;																				
			}
			else
			{
				/**************** WRAP THE BUFFER IN ETHERNET PACKET STRUCTURE **********/
				BcmIpAddress ipAddress;
			  
				result = ParseEthernetHeader(buffer, cnt);
 		 	 	ethPacket.SetPacket(pEthBuffer);
		 	 	ethPacket.ParseLlc();
		 	 	ethPacket.ParseIp();
		  	 	ethPacket.ParseTcpUdp();
				// For DHCP packet
				if(result == 2 && ethPacket.DestTcpUdpPortParsed() == 68 && ethPacket.IpProtocolTypeParsed() == BcmEthernetPacket::kUdp)
				{
					memcpy(dhcpPacket,buffer,cnt);	
 					ethDhcp.SetPacket(pEthDhcp);
					ethDhcp.ParseLlc();
					ethDhcp.ParseIp();
					ethDhcp.ParseTcpUdp();
					pIpHalIf->DhcpClientIf()->DhcpClientThread()->AnnounceDhcpPacket(&ethDhcp, &socketHalIf, pIpHalIf);
				}
				// For ARP packets
			   	else if(result == 1 && ethPacket.IsArpPktParsed())
				{
					memcpy(arpPacket,buffer,cnt);	
 					ethArp.SetPacket(pEthArp);
					ethArp.ParseLlc();
					ethArp.ParseIp();
					pIpHalIf->DhcpClientIf()->DhcpClientThread()->AnnounceArpPacket(&ethArp, pIpHalIf);
				}
				else
				{
					// need release the memory
					ethPacket.ReleasePacket();
				}
			}
			// Give other thread change to run
			BcmOperatingSystemFactory::ThreadSleep(0);
		  //		pIpHalIf = (BcmLnxIpStackIf*) BcmIpHalIfFactory::GetSingleton()->Next(pIpHalIf);
		}
	}
	return;
}

#endif
