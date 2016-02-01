//****************************************************************************
//
// Copyright (c) 2009 Broadcom Corporation
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
//  Filename:       DhcpV6ClientSnoopThread.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Nov 19, 2007
//
//****************************************************************************
//  Description:

//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DhcpV6ClientSnoopThread.h"

// Dibbler lib.
#include "Portable.h"

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
#include "DhcpV6ClientThread.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************
#define DHCPV6_INFINITY (unsigned) 0xffffffff

//********************** Local Variables *************************************

//********************** Local Functions *************************************
static void DhcpHelperThread ( void );
static ETHERNET_PACKET* AllocatePacket( char *buf, uint32 length );
static int DsgFreeEthernetPkt( ETHERNET_PACKET* buffer );
static bool CreateDhcpV6Packet(BcmEthernetPacket &packet);

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
BcmDhcpV6ClientSnoopThread::BcmDhcpV6ClientSnoopThread( BcmOperatingSystem::ThreadPriority initialPriority) :
BcmThread("eSTB Remote Interface Thread", false, initialPriority)
{
    CallTrace("BcmDhcpV6ClientSnoopThread", "BcmDhcpV6ClientSnoopThread");

    fMessageLogSettings.SetModuleName("BcmDhcpV6ClientSnoopThread");

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
BcmDhcpV6ClientSnoopThread::~BcmDhcpV6ClientSnoopThread()
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
bool BcmDhcpV6ClientSnoopThread::Initialize(void)
{
    CallTrace("BcmDhcpV6ClientSnoopThread", "Initialize");

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
void BcmDhcpV6ClientSnoopThread::ThreadMain(void)
{
    CallTrace("BcmDhcpV6ClientSnoopThread", "ThreadMain");



	// Broadcom Remote Interface.
	DhcpHelperThread();
}


// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmDhcpV6ClientSnoopThread::Deinitialize(void)
{
    CallTrace("BcmDhcpV6ClientSnoopThread", "Deinitialize");

    // Get rid of the OS objects that I created.
}

/*void BcmDhcpV6ClientSnoopThread::SetEstbIpAddress(BcmIpAddress &eStbIpAddress)
{
	fEstbIpAddress = eStbIpAddress;
}*/


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
ETHERNET_PACKET* AllocatePacket( char *buf, uint32 length )
{          
	char			*pPayload = NULL;
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

/*
 * When Dsg Client is done processing a DSG packet, call this function to free the packet
 *
 * Parameters:
 *      Ethernet packet struct pointer to be free
 *
 * Returns:
 *      kFuncSuccess: Ethernet packet pointer is freed
 *      kFuncInvalid: cannot reconcile the packet pointer
 *      kFuncFail = function failure
 *
 */
int DsgFreeEthernetPkt( ETHERNET_PACKET* buffer )
{
    //DsgPrint("DsgFreeEthernetPkt: DSG Pkt ptr %08lx\n", (uint32)buffer ); 

	// In WrapEthernetPacket ethernet buffer that is received
	// is wrapped in a newly allocated buffer pEthernetPacketBuffer->pDataBufferHead.
	// We are not freeing the actual buffer because that is the buffer RX DATA socket is using.

	// Release the ethernet buffer;
	if( buffer->pDataBufferHead->pvStart != NULL )
	{
		free(buffer->pDataBufferHead->pvStart);
	}

	// Now release the buffers allocated for wrapper objects.
	if( buffer->pDataBufferHead != NULL )
	{
		free(buffer->pDataBufferHead);
	}

	if( buffer != NULL )
	{
		free(buffer);
	}

	return 1; //kSuccess;
}

static bool CreateDhcpV6Packet(BcmEthernetPacket &packet)
{
    bool result;

    CallTrace("BcmDhcpV6ClientIf", "SendDhcpV6Packet");

    BcmOctetBuffer buffer(0);
    //BcmEthernetPacket packet;
	ETHERNET_PACKET* pEthernetBuffer = NULL;

    // Account for the 14 byte LLC header, the 40 byte IPv6 header, the 8 byte
    // UDP header, and the 4 byte CRC32, as well as the size of the DHCPv6
    // payload.
	unsigned int dhcpPacketSize = 1400;
	#warning "-DHCPv6 MODS - Pre-allocated DHCPv6 payload size - 1400 Bytes"
    unsigned int bufferSize = 14 + 40 + 8 + dhcpPacketSize + 4; //pDhcpPacket->Size() + 4;

    // Get the IP HalIf that I'm associated with to allocate a buffer
    // for me.
    //if (!pfIpHalIf->AllocatePacket(&packet, bufferSize))
	pEthernetBuffer = AllocatePacket(NULL, bufferSize);
	if (!pEthernetBuffer)
    {
        gLogMessageRaw //gWarningMsg(fMessageLogSettings, "SendDhcpV6Packet")
            << "Failed to allocate a h/w buffer for the packet ("
            << bufferSize << " bytes); can't send it!" << endl;

        return false;
    }

    // Store the allocated packet in the BcmEthernetPacket object.
    packet.SetPacket(pEthernetBuffer);
	
	BcmMacAddress macAddress;

    // Fill in the LLC header.
    {
        // Select the appropriate MAC DA based on the type of packet.
        /*if ((pDhcpPacket->MessageType() == kDhcpV6Solicit) ||
            (pDhcpPacket->MessageType() == kDhcpV6Request) ||
            (pDhcpPacket->MessageType() == kDhcpV6Renew) ||
            (pDhcpPacket->MessageType() == kDhcpV6Rebind) ||
            (pDhcpPacket->MessageType() == kDhcpV6Release))*/
        {
            //BcmMacAddress macAddress;

            kAllDhcpRelayAgentsAndServers.GetMulticastDestinationMacAddress(macAddress);

            packet.MacDa(macAddress);
        }
        /*else
        {
            gErrorMsg(fMessageLogSettings, "SendDhcpV6Packet")
                << "Not yet supported!" << endl;

            return false;
        }*/

        // The MAC SA is always my IP stack's address.
        packet.MacSa(macAddress);//pfIpHalIf->MacAddress());

        // We always send EtherII/IPv6.
        packet.SetFrameFormat(BcmEthernetPacket::kEthernetII);
        packet.EtherType(BcmEthernetPacket::kEtherTypeIpV6);

        // Tell the packet that this portion of the header is done.
        packet.LlcGenerated();
    }

    // Fill in the IPv6 header.
    {
        // Initialize the IPv6 header.
        packet.IpV6Init();

        // What traffic class should I use?  The default is 0, which is what
        // RFC2460 says use.
        //packet.IpV6TrafficClass(0);

        // What flow label should I use?  The default is 0, which is what
        // RFC2460 says use.
        //packet.IpV6FlowLabel(0);

        // The length of the payload (starting with the UDP header)
        packet.IpV6PayloadLength(8 + dhcpPacketSize); //pDhcpPacket->Size());

        // The next header is UDP.
        packet.IpV6NextHeader(BcmEthernetPacket::kUdp);

        // The hop limit.  I have no idea what to use here; how does 128 sound?
        packet.IpV6HopLimit(128);

        // The source IP address need to be our link-local address for Solicit
        // packets, or our current IPv6 address for other packets (?).
        /*if ((pDhcpPacket->MessageType() == kDhcpV6Solicit) ||
            (pDhcpPacket->MessageType() == kDhcpV6Request) ||
            (pDhcpPacket->MessageType() == kDhcpV6Renew) ||
            (pDhcpPacket->MessageType() == kDhcpV6Rebind) ||
            (pDhcpPacket->MessageType() == kDhcpV6Release))*/
        {
            BcmIpV6Address linkLocalIpAddress = kLinkLocalUnicastAddressPrefix;

            linkLocalIpAddress.AppendInterfaceToken(macAddress); //pfIpHalIf->MacAddress());

            packet.SourceIpAddress(linkLocalIpAddress);
        }
        /*else
        {
            gErrorMsg(fMessageLogSettings, "SendDhcpV6Packet")
                << "Not yet supported!" << endl;

            return false;
        }*/

        // It seems like the destination address is always supposed to be the
        // All-Servers address.
        packet.DestIpAddress(kAllDhcpRelayAgentsAndServers);

        // Tell the packet that this portion of the header is done.
        packet.IpV6Generated();
    }

    // Fill in the UDP header.
    {
        // We send to the DHCPv6 server's port number and from the DHCPv6
        // client's port number.
        packet.SourceTcpUdpPort(kDhcpV6UdpClientPort);
        packet.DestTcpUdpPort(kDhcpV6UdpServerPort);

        // The length of the UDP header plus payload.
        packet.UdpLengthBytes(8 + dhcpPacketSize); //pDhcpPacket->Size());

        // The checksum will be calculated later.

        // Tell the packet that this portion of the header is done.
        packet.UdpGenerated();
    }

	//{
		BcmDhcpV6Packet dhcpPacket;

		// Set the message type to release.
		dhcpPacket.MessageType(kDhcpV6Solicit);

		// Store the transaction id.
		dhcpPacket.TransactionId(BcmDhcpV6Packet::GenerateNewTransactionId());

		// Include a client identifier option (DUID).  The DUID for our device is
		// stored in the DHCPv6 client thread.
//		dhcpPacket.OptionsModify().Add(pfDhcpV6ClientThread->Duid());

		//gLogMessageRaw //gInfoMsg(fMessageLogSettings, "SendDhcpV6Packet")
		//	<< "Generating DHCPv6 packet with these settings:\n" << dhcpPacket << endl;
	//}

    // Now overlay my buffer on the UDP payload.
    buffer.AdoptBuffer(packet.UdpPduWritable(), dhcpPacket.Size(), 0, false);

    // Write the the DHCPv6 packet data as the UDP payload.
    result = dhcpPacket.WriteTo(buffer, NULL);

    if (!result)
    {
        gLogMessageRaw //gErrorMsg(fMessageLogSettings, "SendDhcpV6Packet")
            << "failed to write DHCP packet to the buffer!" << endl;

        // Release the buffer back to the HAL.
        //pfIpHalIf->ReleasePacket(&packet);

        return false;
    }

    // Calculate the UDP checksum, which includes the payload.  We have to do
    // this after writing the payload data.
    packet.UpdateUdpChecksum();

    // Set the packet length in the underlying structure, not including the
    // 4-byte CRC..
    packet.SetPacketLength(14 + 40 + 8 + buffer.BytesUsed());

    //gLogMessageRaw //gInfoMsg(fMessageLogSettings, "SendDhcpV6Packet")
    //    << "Generated DHCPv6 packet:\n" 
		//<< packet  // IMPORTANT - Commented out this because it caused a crash. Stack overflow??
	//	<< endl;

	return true;
}


/*
 * This Thread will listen on socket (kDhcpV6UdpClientPort) for DHCPv6 pkts.
 */
static void DhcpHelperThread ( void )
{
    register int cnt;
	//struct sockaddr_in frominet;
	//char              *buf;					/* ptr to dynamic buffer */
	//char              *dsgPacketBuffer;		/* ptr to dynamic buffer */
	int		buflen  = 1400;					/* length of buffer */

	BcmBaseIpHalIf *pHal = NULL;
	
	int maxfdp1 = 0;
	fd_set rfds;
	int retcode;
	//int optVal;
	struct timeval czas;
	unsigned int time = DHCPV6_INFINITY;

    if (time > DHCPV6_INFINITY/2)
	time /=2;
    
    czas.tv_sec=time;
    czas.tv_usec=0;

	//ETHERNET_PACKET *pEthernetPacketBuffer = NULL;
	
	//Allocate buffer to receive NOTIFICAION.
   /* if( (buf = (char *)malloc(buflen)) == (char *)NULL)
	{
		printf("Can not allocate buffer %d\n", buflen);
		return; //goto err_malloc;
	}*/
	/*
	//Allocate buffer to receive DSG Packet.
    if( (dsgPacketBuffer = (char *)malloc(buflen)) == (char *)NULL)
	{
		printf("Can not allocate buffer %d\n", buflen);
		return; //goto err_malloc;
	}*/

    gLogMessageRaw <<"Starting DhcpHelperThread. Opening sockets and listening on port "<< inHex(kDhcpV6UdpClientPort) << endl;	
	
	// Preallocate ethernet packet buffer. Socket will read into the UDP PAYLOAD buffer.
	BcmEthernetPacket ethernetPacket;
	if( !CreateDhcpV6Packet(ethernetPacket))
	{
		printf("Can not allocate DHCPv6 ETH buffer\n");
		return; //goto err_malloc;
	}
	//printf("*** DhcpHelperThread ***. Setting up socket IF object...\n" );	
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
	gLogMessageRaw <<"*** DhcpHelperThread ***. Setting up socket set.."<< endl;	
    pHal = (BcmBaseIpHalIf*) BcmIpHalIfFactory::GetSingleton()->First();
    while (pHal)
    {
		if( pHal->Ipv6LinkLocalAddressConfigured() )
		{
			// Get IPv6 Link Local address.
			BcmIpV6Address linkLocalAddress;
			pHal->GetIpv6LinkLocalAddress(linkLocalAddress);				

			// Create transmit socket.
			pHal->CreateNetworkRxSocket(linkLocalAddress, kDhcpV6UdpClientPort, true, true);
		}

		if( pHal->NetworkRxSocket() > 0 )
		{
			maxfdp1 = ((maxfdp1 > pHal->NetworkRxSocket()) ? maxfdp1 : pHal->NetworkRxSocket()) + 1;
			//printf("maxfdp1 = %d\n", maxfdp1);
		}

		pHal = (BcmBaseIpHalIf*) BcmIpHalIfFactory::GetSingleton()->Next(pHal);
	}
	////////////////////////////////////////////////////////////////////////////
	//printf("*** DhcpHelperThread ***. Starting to listen on sockets...\n" );	
	while(1)
	{
		FD_ZERO(&rfds);

		/////////////////  Update socket set ////////////////////
    
		pHal = (BcmBaseIpHalIf*) BcmIpHalIfFactory::GetSingleton()->First();
		while (pHal)
		{
			if( pHal->NetworkRxSocket() > 0 )
			{
				FD_SET(pHal->NetworkRxSocket(), &rfds);
			}

			pHal = (BcmBaseIpHalIf*) BcmIpHalIfFactory::GetSingleton()->Next(pHal);
		}
		////////////////////////////////////////////////////////////////////////////
						
		// Listen
		retcode = select(maxfdp1, &rfds, (fd_set *)0, (fd_set *)0, &czas); //(struct timeval *)0);
		
		if (retcode == -1)
		{
			printf("select(): %s\n", strerror(errno));
		}
		else 
		{
//			int result;
			
			/////////////////  Process  ////////////////////
			BcmBaseIpHalIf *pIpHalIf;
    
			pIpHalIf = (BcmBaseIpHalIf*) BcmIpHalIfFactory::GetSingleton()->First();
			while (pIpHalIf)
			{				
				if(FD_ISSET( pIpHalIf->NetworkRxSocket(), &rfds))
				{												
					//gLogMessageRaw << "FD_ISSET" << endl;		

					char myPlainAddr[48];   // my plain address
					char peerPlainAddr[48]; // peer plain address

					// receive data (pure C function used)
					cnt = sock_recv(pIpHalIf->NetworkRxSocket(), myPlainAddr, peerPlainAddr, (char*)ethernetPacket.UdpPduWritable(), buflen); //  buf, buflen);					
					
					if (cnt == -1) 
					{ 
						gLogMessageRaw << "Socket recv() failure detected." << endl;																				
					}
					else
					{
						//gLogMessageRaw << pIpHalIf->Description() << " Socket RECEIVE detected. " << cnt << endl;

						/**************** WRAP THE BUFFER IN ETHERNET PACKET STRUCTURE **********/
						BcmIpV6Address ipAddress;

	//					ipAddress.Set(peerAddrPacked);
						unsigned int size = 22;
						ipAddress.Set(peerPlainAddr, &size);
						//gLogMessageRaw << ipAddress << size << endl;	
						ethernetPacket.SourceIpAddress(ipAddress);
						//gLogMessageRaw << ipAddress << endl;						
						//					ipAddress.Set(myAddrPacked);
						ipAddress.Set(myPlainAddr, &size);
						//gLogMessageRaw << ipAddress << size << endl;
						ethernetPacket.DestIpAddress(ipAddress);
						//gLogMessageRaw << ipAddress << endl;	

						// The length of the UDP header plus payload.
						ethernetPacket.UdpLengthBytes(8 + cnt); //pDhcpPacket->Size());

						// The checksum will be calculated later.

						// Tell the packet that this portion of the header is done.
						ethernetPacket.UdpGenerated();

						// Calculate the UDP checksum, which includes the payload.  We have to do
						// this after writing the payload data.
						ethernetPacket.UpdateUdpChecksum();

						// Set the packet length in the underlying structure, not including the
						// 4-byte CRC..
						ethernetPacket.SetPacketLength(14 + 40 + 8 + cnt);

						//gLogMessageRaw << ethernetPacket << endl;
						/*if(0)
						{
							BcmOctetBuffer buffer(0);
							// Now overlay my buffer on the UDP payload.
							buffer.AdoptBuffer(ethernetPacket.UdpPduWritable(), cnt, cnt, false);

							unsigned int offset = 0;

							BcmDhcpV6Packet dhcpPacket;
							if (dhcpPacket.ReadFrom(buffer, offset, NULL) == false)
							{
								gLogMessageRaw //gInfoMsg(fMessageLogSettings, "ProcessPacket")
									<< "Failed to parse packet (invalid format); discarding..." << endl;					
							}
							gLogMessageRaw << dhcpPacket << endl;
						}*/
						

						pIpHalIf->DhcpV6ClientIf()->DhcpV6ClientThread()->AnnounceDhcpV6Packet(&ethernetPacket, &socketHalIf, pIpHalIf);
						/*************************************************************************/
					}									
				}

				pIpHalIf = (BcmBaseIpHalIf*) BcmIpHalIfFactory::GetSingleton()->Next(pIpHalIf);
			}
			////////////////////////////////////////////////////////////////////////////
		}	
	}
	printf("DhcpV6HelperThread fell through!\n");				
	
	//close(dhcpRecvSocket);

	return;
}

#endif