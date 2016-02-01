//****************************************************************************
//
// Copyright (c) 2005-2012 Broadcom Corporation
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
//  Filename:       DsgccCablecardIf.cpp
//  Author:         Kenny Lee
//  Creation Date:  July 22, 2005
//
//****************************************************************************
//  Description:
//		This is the Cablecard Interface object that manage the cablecard operation
//		and receive any DS packet destined to cablecard.
//		This is also the cpp to c interface code where the cablcecard driver will 
//		call these functions to register the DSG tunnel with the DSGCC
//
//****************************************************************************

//********************** Include Files ***************************************
#include "OperatingSystemFactory.h"
#include "MutexSemaphore.h"
#include "DsgClientCtlThread.h"
#include "DsgCableCardClient.h"

#include "DsgccCablecardIf.h"
#include "DsgConfigSettings.h"
#include "EstbRemoteInterfaceStub.h"
#include "DcdTlvCodes.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <ifaddrs.h>


//********************** Local Types *****************************************
typedef struct _IP_HDR
{
    UCHAR  ucVerLen;
    UCHAR  ucToS;
    USHORT usLength;
    USHORT usId;
    USHORT usFFrag;
    USHORT usTtlProto;
    USHORT usHCS;
    ULONG  ulSrcIp;
    ULONG  ulDesIp;
} IP_HDR, *PIP_HDR;

//********************** Local Constants *************************************

#define INVALID_FLOW_ID     0xFF000000      // 24 bit valid flow ID. Use MSB to indicate non-zero invalid flow ID

// debug defines
//#define IPU_REQ_WAIT       0
#define ENABLE_IPU_US_TEST 0
#define SOCKET_REQ_WAIT       0
#define ENABLE_SOCKET_US_TEST 0

//********************** Local Variables *************************************
static unsigned long pkt_count = 0, ipu_count=0;    //debug
void CableCardIpUTestThread ( void );               //debug
void CableCardSocketFlowUsTestThread ( void );      //debug

static BcmCableCardIfThread *pgCableCardIf = NULL;

extern uint32 gTunnelDebugLevel;

//
//  Global helper function to get cablecard ip address
//
uint32 BcmGetCableCardIpAddress()
{
    if( pgCableCardIf )
        return pgCableCardIf->GetCableCardIpAddress();
    else
        return 0;
}

//
// installable routine to the client controller thread
//
static void BcmPassDcdToCableCard ( uint8 *pDcdTlv, unsigned int DcdLen )
{
    POD_Api_Send_DCD_Info((void *)pDcdTlv, (unsigned short) DcdLen);
}

//********************** Class Method Implementations ************************

// Constructor.  Initializes and starts the thread.
//
// Parameters:
//      initialPriority - the thread priority that should be used.
//
// Returns:  N/A
//
BcmCableCardIfThread::BcmCableCardIfThread(BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmThread("CableCard DS Thread ", false, initialPriority),
    need_to_rescan(false),
    vct_id(0),
    dsg_directory_version(0x5500), // invalid value as initial value
	fLastCableCardIpAddress(0),
	fLastDsgMsgApdu(0)
{
    fpMutex = BcmOperatingSystemFactory::NewMutexSemaphore("BcmCableCardIfThread mutex");

    if (fpMutex == NULL)
        gLogMessageRaw << "WARNING: failed to create mutex for BcmCableCardIfThread" << endl;

    //initialize varibles
    gDsgFlowId = INVALID_FLOW_ID;
    gIpUFlowId = INVALID_FLOW_ID;
    gSocketFlowId = INVALID_FLOW_ID;
    DocsisRangedRegistered = false;
    Remove_Header_Bytes = 0;
    CableCardMacAddress = kAllZerosMac;
    CableCardIpAddress = 0;
    gUdpTcpSocket = 0;

    NumOpenTunnel = 0;
    DhcpOptionLen = 0;

    EcmResetRecoveryForIpu = false;

    dsg_oper_mode = DSG_INVALID_MODE;
}


// Destructor.  If the thread is still running, it will be stopped, allowing
// OS objects to be destroyed (note - this is actually done by the derived
// class when it calls WaitForThread()).
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmCableCardIfThread::~BcmCableCardIfThread()
{
    // Nothing to do...?
}

// Returns the pointer to the singleton instance for this class.  Most
// objects in the system will use this method rather than being passed a
// pointer to it.  The singleton pointer will be set up in the base-class
// constructor.
//
// NOTES:  This can return NULL if a singleton has not been set up for the
//         system, so you must check for this condition.
//
//         You must not delete this object!
//
//         You should not store a pointer to the object that is returned,
//         since it may be deleted and replaced with a new one.
//
// Parameters:  None.
//
// Returns:
//      A pointer to the instance that should be used by the system.
//
BcmCableCardIfThread *BcmCableCardIfThread::GetSingletonInstance(void)
{
    if (pgCableCardIf == NULL)
    {
        gLogMessageRaw
            << "BcmCableCardIfThread::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
    }
    
    return pgCableCardIf;
}

// Thread constructor - this is the first method called after the thread has
// been spawned, and is where the thread should create all OS objects.  This
// has to be done here, rather than in the object's constructor, because
// some OS objects must be created in the context of the thread that will
// use them.  The object's constructor is still running in the context of
// the thread that created this object.
//
// The default implementation simply returns true, allowing derived classes
// that don't have any initialization to use the default.
//
// Parameters:  None.
//
// Returns:
//      true if successful and ThreadMain() should be called.
//      false if there was a problem (couldn't create an OS object, etc.)
//          and ThreadMain() should not be called.
//
bool BcmCableCardIfThread::Initialize(void)
{
    // Nothing to do...
    return true;
}

// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// The default implementation does nothing, allowing derived classes that
// don't have any deinitialization to use the default.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmCableCardIfThread::Deinitialize(void)
{
    // Nothing to do!
}

// This is the main body of the thread's code.  This is the only method
// that absolutely must be provided in the derived class (since there is no
// reasonable default behavior).
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmCableCardIfThread::ThreadMain(void)
{
    // This is now a socket flow thread that will listen to incoming packet
    struct sockaddr_in6 frominet6;
    struct sockaddr_in frominet;
    int maxfdp1;
    fd_set rfds;
    int retcode, cnt;
    int inetsize = sizeof(frominet);
    int inetsize6 = sizeof(frominet6);
    int buflen = sizeof(packet_buf);
    struct timeval tv;
    int lost_flow_sent=0;

    gLogMessageRaw << "\nIn CableCard Socket Flow DS RX Thread - listening on port = 0x" << hex << (int)SocFlowInfo.remote_port_number << dec << endl;

    while(1)
    {
        // if DS_Socket is closed, loop here
        while ( gUdpTcpSocket == 0)
        {
            //gLogMessageRaw << "Socket flow is closed! DS tread is sleeping..." << endl ;
            pfOperatingSystem->Sleep(1000);     //sleep 1 sec
        }

        // Set 3 sec timeout loop
        tv.tv_sec = 3;
        tv.tv_usec = 0;

        maxfdp1 = gUdpTcpSocket + 1;

    	FD_ZERO(&rfds);
       	FD_SET(gUdpTcpSocket, &rfds);
    	
        // set loop every 3 seconds to handle cases where the flow is deleted and open again
    	retcode = select(maxfdp1, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)&tv);
    
        // If successful, select() returns the number of ready descriptors that are contained in the bit masks. 
        // If the time limit expires, select returns zero and sets errno to EINTR. 
        // On failure, it returns -1 and sets errno to one of the following values: 
        if( retcode > 0 )
    	{
            if(FD_ISSET(gUdpTcpSocket, &rfds))
            {
                // recvfrom should not block now
                // Upon successful completion, recvfrom() returns the length of the message in bytes. 
                // If no messages are available to be received and the peer has performed an orderly shutdown, recvfrom() returns 0. 
                // Otherwise the function returns -1 and sets errno to indicate the error
                /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if ( SocFlowInfo.remote_address_type == SF_IPV6)
                    cnt = recvfrom(gUdpTcpSocket, packet_buf, buflen, 0, (struct sockaddr*)&frominet6, (socklen_t*) &inetsize6);
                else
                    cnt = recvfrom(gUdpTcpSocket, packet_buf, buflen, 0, (struct sockaddr*)&frominet, (socklen_t*) &inetsize);

                if( cnt > 0 )
                {
                    if (gTunnelDebugLevel & 0x10)
                    {
                        DsgPrint("DS_SocketFlow pkt with len %d\n", cnt);
                        DumpBytes("DS_SocketFlow", packet_buf, cnt);
                    }
                
                    lost_flow_sent = 0;

                    // For CableCard Driver, this function block until data is sent out to the POD or timeout. 
                    // When the function returns, the packet can always be released
                    BcmSendDataToPOD( packet_buf, cnt, gSocketFlowId );
                }
                else if( cnt == 0 )
                {
                    // remote TCP socket close
                    if( lost_flow_sent == 0 )
                    {
                        POD_Api_Lost_Flow_Ind( gSocketFlowId , LOSTFLOW_RTCP_CLOSED );
                        lost_flow_sent = 1;
                        gLogMessageRaw	<< "gUdpTcpSocket: Remote server closed. Wait for Host to delete flow..." << endl;
                    }
                    pfOperatingSystem->Sleep(1000);     //sleep 1 sec
                }
                else
                {
                    if( lost_flow_sent == 0 )
                    {
                        POD_Api_Lost_Flow_Ind( gSocketFlowId , LOSTFLOW_SOCKET_RD_ERR );    //socket read error 
                        lost_flow_sent = 1;
                    }
                    gLogMessageRaw	<< "gUdpTcpSocket receive error = " << strerror(errno) << endl;
                }
            }
        }
        else if (retcode == -1)
        {
            POD_Api_Lost_Flow_Ind( gSocketFlowId , LOSTFLOW_UNKNOWN );    //unknown or unspecified reason
            gLogMessageRaw << "select(): " << strerror(errno) << endl;
        }
        else if( retcode == 0 )
        {
            //gLogMessageRaw << "+" << endl;
            if( EcmResetRecoveryForIpu == true  && DocsisRangedRegistered == true )
            {
                // reset flag
                EcmResetRecoveryForIpu = false;

                // First zero out any cableCard Ipaddress
                CableCardIpAddress = 0;
                DhcpOptionLen = 0;

                // Docsis ranged and registered, Send proxy DHCP request to eCM
                DsgStartProxyDhcpRequest( saved_opt_len, saved_mac, saved_opt_data, NULL ); 

                // Now wait for the DHCP reply to come back
                while( CableCardIpAddress == 0 )
                {
                    cout << "eCM reset recovery IP_U: Waiting DHCP response from eCM..." << endl;
                    BcmOperatingSystemFactory::ThreadSleep(2000);
                    // During IP_U dhcp, at least one tunnel is opened.
                    // NumOpenTunnel suddenly goes to 0 indicate card unplugged
                    if( NumOpenTunnel == 0 )    
                    {
                        cout << "CableCard IP_U: Card removed!" << endl;
                        break;
                    }
                }

                BcmIpAddress ipAddress;
                ipAddress.Set(CableCardIpAddress);
                cout << "IP_U got an IP address = " << ipAddress << endl;
            }
        }
    }

    gLogMessageRaw << "\nError! CableCard Socket Flow DS RX Thread Exit!" << endl;

}

// Handle various DSG modes
//
void BcmCableCardIfThread::HandleSetDsgMode( unsigned char oper_mode, unsigned char *pmac_struct )
{
    BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();
    BcmDsgCableCardClient *pClient;
	unsigned char i;
    unsigned char enable = 0xff;

    if( pDsgClientCtlThread == NULL )
    {
        cout << "HandleSetDsgMode:: Error! No instance of client Controller Thread object!" << endl;
        return;
    }

    // fix for handling cablecard sending back to back set_mode & it cannot handle back to back same DCD pass into it.
    if( dsg_oper_mode == oper_mode )
    {
        // only apply to advance mode
    	if( (oper_mode == ADVANCED_DSG_MODE) || (oper_mode == ADVANCED_DSG_ONEWAY_MODE) )
        {
            cout << "HandleSetDsgMode:: Already in Operational Mode " << (int)oper_mode << ". Ignoring this Set_DSG_mode!" << endl;
            return;
        }
    }

    // Mainly for ATP DSG-03 test: Check transition from two_way to one-way or vice versa
    if( (dsg_oper_mode==ADVANCED_DSG_MODE) && (oper_mode==ADVANCED_DSG_ONEWAY_MODE) )
        enable = 0;
    else if( (dsg_oper_mode==ADVANCED_DSG_ONEWAY_MODE) && (oper_mode==ADVANCED_DSG_MODE) )
        enable = 1;
    else if( (dsg_oper_mode==BASIC_DSG_MODE) && (oper_mode==BASIC_DSG_ONEWAY_MODE) )
        enable = 0;
    else if( (dsg_oper_mode==BASIC_DSG_ONEWAY_MODE) && (oper_mode==BASIC_DSG_MODE) )
        enable = 1;

    if( enable != 0xff )
    {
        dsg_oper_mode = oper_mode;
		pDsgClientCtlThread->CableCardEnableTwoWayMode(enable);
        return;
    }

    // Dynamic switching mode support: Closed all opened tunnels if any
	CloseAllCableCardClients( pDsgClientCtlThread );

    // Advance Mode
	if( (oper_mode == ADVANCED_DSG_MODE) || (oper_mode == ADVANCED_DSG_ONEWAY_MODE) )
	{
        /* Wait for interface to comes up then Set DSG mode */
        while( pDsgClientCtlThread->Mode((uint8) kDsgAdvanceMode) == kFuncFail )
            pfOperatingSystem->Sleep(500);

        /* Send 1 way or 2 way mode */
        if( oper_mode == ADVANCED_DSG_MODE )
            enable = 1;
        else
            enable = 0;
		pDsgClientCtlThread->CableCardEnableTwoWayMode(enable);

		pDsgClientCtlThread->InstallDcdPassThroughHandler( (PFN_DSG_DCD_BYPASS_HANDLER) BcmPassDcdToCableCard );
        cout << "Installed DcdPassThroughHandler!" << endl;

        /* start scanning for new DCD */
        if( need_to_rescan == true )
        {
            // Do not need to rescan until the next cablecard removal
            need_to_rescan = false;
            DocsisRangedRegistered = false;
        }
        pDsgClientCtlThread->Start();

	}
#if 1 // Basic Mode Deprecated
    // Basic Mode
	else if( (oper_mode == BASIC_DSG_MODE) || (oper_mode == BASIC_DSG_ONEWAY_MODE) )
    {
        dsgClientRegInfo regInfo;

        memset(&regInfo, 0, sizeof(regInfo));

        /* Wait for interface to comes up then Set DSG mode */
        while( pDsgClientCtlThread->Mode((uint8) kDsgBasicMode) == kFuncFail )
            pfOperatingSystem->Sleep(500);

        /* Send 1 way or 2 way mode */
        if( oper_mode == BASIC_DSG_MODE )
            enable = 1;
        else
            enable = 0;
		pDsgClientCtlThread->CableCardEnableTwoWayMode(enable);

		pDsgClientCtlThread->InstallDcdPassThroughHandler( (PFN_DSG_DCD_BYPASS_HANDLER) BcmPassDcdToCableCard );
        cout << "Installed DcdPassThroughHandler!" << endl;

        // Dhcp will wait
        DocsisRangedRegistered = false;

        // Now, open new tunnels. First byte is the number of mac addresses that follow
        NumOpenTunnel = *pmac_struct++;
        for (i=0; i<NumOpenTunnel; i++ ) 
        {
            regInfo.clientPort = kClientPort+i;
            // Copy tunnel mac address.
            memcpy( regInfo.clientName, pmac_struct, 6 );
            pmac_struct += 6;
            regInfo.idType = kMacAddressType;

            // create one cablecard client for each mac address
            pClient = new BcmDsgCableCardClient(&regInfo, this);
            pClient->Id( (const uint32)CARD_ENTRY_CLIENT_ID_START+i );

            pDsgClientCtlThread->RegisterClient(pClient);	//Serialize calls
        }

        // follow by 16bit remove header byte
        Remove_Header_Bytes = *pmac_struct++;
        Remove_Header_Bytes <<= 8;
        Remove_Header_Bytes |= *pmac_struct;
        cout << "Remove Header Byte = " << Remove_Header_Bytes << endl;

        pDsgClientCtlThread->Start();

    }
#endif
    else if ( oper_mode == SCTE55_MODE )
    {
        // Stop any on-going Docsis scanning if any
        DsgScan(false);

        /* Wait for interface to comes up then Set DSG mode */
        while( pDsgClientCtlThread->Mode((uint8) kDsgDisabledMode) == kFuncFail )
            pfOperatingSystem->Sleep(500);

        need_to_rescan = true;

    }
	else
        cout << "!PANIC! Unsupported CableCard mode (" << oper_mode << "). Do not know how to handle." << endl;

    // saved of current cablecard dsg mode
    dsg_oper_mode = oper_mode;

	return;

}

//
// Handle DSG AdvanceMode Tunnels configuration
//
// typedef struct _CCARD_CONFIGURE_ADVANCED_DSG_APDU
// {
// 	unsigned char num_filters;
//     // {
//     unsigned char tunnel_id;
//     unsigned short app_id;
//     unsigned char dsg_mac_address[6];
//     unsigned long src_ip_address;
//     unsigned long src_ip_mask;
//     unsigned long dest_ip_address;
//     unsigned char num_ports;
//         // {
//     unsigned short dest_port;       // * num_ports
//         // }
//     unsigned short remove_header_bytes;
//     // }    * num_filters
//     unsigned char num_rx_freq;
//     // {
//     unsigned long rx_freq;      // * num_rx_freq
//     // }
//     unsigned short init_timeout;
//     unsigned short operational_timeout;
//     unsigned short twoway_retry_timeout;
//     unsigned short oneway_retry_timeout;
// } CCARD_CONFIGURE_ADVANCED_DSG_APDU;
//
void BcmCableCardIfThread::HandleConfigureAdvanceDsg(unsigned char *pkt_obj_ptr, unsigned short len )
{
    BcmDsgClassifierSettings cfr, cfr0;
    BcmDsgConfigSettings freqchan_timers;
    BcmIpAddress ip_address;
    BcmDsgCableCardClient *pClient;
    unsigned char *pCfgAdvDsg = pkt_obj_ptr;
    int i, j;
    unsigned long rx_freq;
    unsigned char num_port, num_rx_freq;
    unsigned short port;
    uint32 card_client_id = CARD_ENTRY_CLIENT_ID_START;

    BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();

    if( pDsgClientCtlThread == NULL )
    {
        cout << "HandleSetDsgMode:: Error! No instance of client Controller Thread object!" << endl;
        return;
    }

    // Closed all opened tunnels if any
	CloseAllCableCardClients( pDsgClientCtlThread );

    if (!fpMutex->Lock())
    {
        gErrorMsg(fMessageLogSettings, "HandleConfigureAdvanceDsg")
            << "Failed to lock my mutex!" << endl;

        return;
    }

    // Now, open new tunnels. First byte is the number of filters
    NumOpenTunnel = *pCfgAdvDsg++;
    for (i=0; i<NumOpenTunnel; i++ ) 
    {
        // clear cfr
        cfr = cfr0;

        // create one cablecard client for each mac address
        pClient = new BcmDsgCableCardClient(this);

        // Unique client port for Deregister purposes
        pClient->ClientPort(kClientPort+i);

        // populate tunnel info
        pClient->Id( (const uint32)*pCfgAdvDsg );
        pClient->CableCardTunnelSetting().fRuleId = *pCfgAdvDsg++;    //Also save to here
        // parse 16bit application_id
        port = *pCfgAdvDsg++;
        port <<= 8;
        port |= *pCfgAdvDsg++;
        if( port == 0 )
        {
            // tunnel request by Card, use my own define for card-entry
            pClient->Id( (const uint32)card_client_id++ );
            pClient->IdType( (const uint32)kCardEntryClientType );
        }
        else
        {
            // tunnel request by Host
            pClient->Id( (const uint32)port );
            pClient->IdType( (const uint32)kApplicationType );
        }

        // parse mac address
		BcmMacAddress dsg_mac_address((const byte *) pCfgAdvDsg);
        pClient->ClientAddress(dsg_mac_address);
        pClient->CableCardTunnelSetting().TunnelAddress(dsg_mac_address); // also put in tunnelsetting class
        pCfgAdvDsg += 6;

        // For all valid Ip and mask, makesure it is non-zero
	    ip_address.Set((uint8 *) pCfgAdvDsg);
        if( ip_address == kAllZerosIp )
        {
            // OC-SP-CCIF2.0-I20-091211: ..A value of all zeros implies all values of SourceIP Address, i.e., 
            // this parameter was not specified in the DCD message.
            // To make it NOT specified, do not write anythng to the cfr, skip the cfr SourceIpMask processing as well
            pCfgAdvDsg += 4;
        }
        else
        {
            // Store non-zero Source Ip Address into cfr
            cfr.SourceIpAddr( ip_address );

            // Get SourceIp mask into placeholder
            pCfgAdvDsg += 4;
            ip_address.Set((uint8 *) pCfgAdvDsg);
            // OC-SP-CCIF2.0-I20-091211: A value of all ones implies that all 32 bits of the Source IP Address are 
            // to be used for filtering.
            // Just makesure it is not all 0's. treat it as NOT specficied
            if( ip_address != kAllZerosIp )
            {
                cfr.SourceIpMask( ip_address );
                // Apply subnet mask to ip address for layer 3 filtering
                ip_address.Set( cfr.SourceIpAddr().Get() & cfr.SourceIpMask().Get() );
                cfr.SourceIpAddr( ip_address );
            }
        }
        pCfgAdvDsg += 4;

	    ip_address.Set((uint8 *) pCfgAdvDsg);
        if( ip_address != kAllZerosIp )
            cfr.DestIpAddr( ip_address );
        pCfgAdvDsg += 4;
        
        num_port = *pCfgAdvDsg++;
        for(j=0; j<num_port; j++ )
        {
            port = *pCfgAdvDsg++;
            port <<= 8;
            port |= *pCfgAdvDsg++;
            if( j==0 )
                cfr.DestPortStart( port );
            else
                cfr.DestPortEnd( port );
        }

        // follow by 16bit remove header byte
        Remove_Header_Bytes = *pCfgAdvDsg++;
        Remove_Header_Bytes <<= 8;
        Remove_Header_Bytes |= *pCfgAdvDsg++;
        // cout << "Remove Header Byte = " << Remove_Header_Bytes << endl;
        // cout << "cfr = " << cfr << endl;

        pClient->CableCardTunnelSetting().add(cfr);

        pDsgClientCtlThread->RegisterCableCardAdvancedModeClient(pClient);	//Direct calls
        cout << "ConfigureAdvanceDsg: Tunnel Mac=" << pClient->CableCardTunnelSetting().TunnelAddress() << *pClient << endl;
    }

    if (!fpMutex->Unlock())
    {
        gWarningMsg(fMessageLogSettings, "HandleConfigureAdvanceDsg")
            << "Failed to unlock my mutex!" << endl;
    }

    // Now get the freq channel list if any
    num_rx_freq = *pCfgAdvDsg++;
    if( num_rx_freq )
    {
        for(j=0; j<num_rx_freq; j++ )
        {
            rx_freq = *pCfgAdvDsg++;
            rx_freq <<= 8;
            rx_freq |= *pCfgAdvDsg++;
            rx_freq <<= 8;
            rx_freq |= *pCfgAdvDsg++;
            rx_freq <<= 8;
            rx_freq |= *pCfgAdvDsg++;

            freqchan_timers.SetFreqChannelList(rx_freq);
        }
    }

    // Now set the 4 timeouts
    port = *pCfgAdvDsg++;
    port <<= 8;
    port |= *pCfgAdvDsg++;
    // Valid Tdsg1 timer range value is 1-65535
    if( port )
        freqchan_timers.SetTdsg1Timeout(port);

    port = *pCfgAdvDsg++;
    port <<= 8;
    port |= *pCfgAdvDsg++;
    // Valid Tdsg2 timer range value is 1-65535
    if( port )
        freqchan_timers.SetTdsg2Timeout(port);

    port = *pCfgAdvDsg++;
    port <<= 8;
    port |= *pCfgAdvDsg++;
    // Valid Tdsg3 timer range value is 0-65535
    freqchan_timers.SetTdsg3Timeout(port);

    port = *pCfgAdvDsg++;
    port <<= 8;
    port |= *pCfgAdvDsg++;
    // Valid Tdsg4 timer range value is 0-65535
    freqchan_timers.SetTdsg4Timeout(port);

    // copy this into the client controller object
    // Also serialize call to be sent to eCM
    pDsgClientCtlThread->SetDsgConfigSetting( freqchan_timers );

    // DONE parsing ADVANCED_DSG_APDU!
}

// For dual processor platform: Handle cablecard Ip Unicast flow request
// This is supposed to implement as a dhcp proxy for cablecard
//
// Parameters:  
//	unsigned short flowid - IP unicast flow id to use when forwarding data to cablecard
//	unsigned char *mac - CableCard Mac address to use for this flow
//	unsigned char opt_len - dhcp request option length
//	unsigned char *opt_data - actual dhcp option string
//	unsigned char *ret_option - buffer to contain any dhcp reply option string
//	int *ropt_len - dhcp reply option length
//
// Returns:  Nothing.
//
uint32 BcmCableCardIfThread::HandleIpUnicastWithDhcpReq( unsigned long flowid, unsigned char *mac, unsigned char opt_len, unsigned char *opt_data, unsigned char *ret_option, int *ropt_len )
{
    BcmIpAddress ipAddress;
    uint8 i = 0;
    uint8 dhcploop = 0;

    // If Docsis has not ranged and registered, just return....
    if( !DocsisRangedRegistered ) 
    {
        // OC-ATP 14.1.1 step 8:
        // return 0 and let pod_driver indicating an error (status_field == 0x03, Request denied, network unavailable or not responding)
        cout << "\nCableCard IP_U: Docsis is not Registered! return ip_address of 0!" << endl;
        return 0;
        #if 0
        // on some of the cablecard, return 0 will not make the card resend IPU flow req later
        //#if (IPU_REQ_WAIT )
        cout << "CableCard IP_U: Waiting 10sec for Docsis Registration complete..." << endl;
        
        // Wait 10 sec for Docsis ranged and registered
        while( !DocsisRangedRegistered && (i < 10) ) 
        {
            BcmOperatingSystemFactory::ThreadSleep(1000);
            i++;
        }

        //#else
        if( !DocsisRangedRegistered )
        {
            cout << "\nCableCard IP_U: Docsis is not Registered! return ip_address of 0!" << endl;
            return 0;
        }
        #endif
    }

    //ATP test (OC-TP-HOST2.1-ATP-D04-081218.doc ) section 14.1.2.3 CARD INIT - invalid DHCPACK tests 
    do
    {
        cout << "\nCableCard IP_U: Starting Proxy DHCP...." << endl;

		// PR20830: 
		// Store last IP address.
		fLastCableCardIpAddress = 0;
		if ( CableCardIpAddress != 0xEFEFEFEF )
			fLastCableCardIpAddress = CableCardIpAddress;

        // First zero out any cableCard Ipaddress
        CableCardIpAddress = 0;
        DhcpOptionLen = 0;

        //Saved a copy of the mac address and flow id
        gIpUFlowId  = flowid;
        CableCardMacAddress.Set((const uint8 *)mac);

        // saved off paramters for eCM reset recovery
        for ( i=0; i<6; i++ )
            saved_mac[i] = mac[i];
        saved_opt_len = opt_len;
        for ( i=0; i<saved_opt_len; i++ )
            saved_opt_data[i] = opt_data[i];

		//PR20830: 
		cout << "CableCard IP_U: ** Last acquired IP address is 0x" << hex << fLastCableCardIpAddress << endl;

        // Docsis ranged and registered, Send proxy DHCP request to eCM
        DsgStartProxyDhcpRequest( opt_len, mac, opt_data, fLastCableCardIpAddress ); //ret_option ); PR20830

        // Now wait for the DHCP reply to come back
        while ( CableCardIpAddress == 0 )
        {
            cout << "CableCard IP_U: Waiting DHCP response from eCM..." << endl;
            BcmOperatingSystemFactory::ThreadSleep(2000);
            // During IP_U dhcp, at least one tunnel is opened.
            // NumOpenTunnel suddenly goes to 0 indicate card unplugged
            if ( NumOpenTunnel == 0 )
            {
                cout << "CableCard IP_U: Card removed!" << endl;
                return 0;
            }
        }

        // all F's return by eCM indicates one of the DHCP field required in REQ2597 (OCAP Host spec) is missing
        // Restart DHCP Card IP address acquisition
        if ( CableCardIpAddress == 0xEFEFEFEF )
        {
            cout << (int)dhcploop << ".)Error! One of the required DHCP fileds is missing in the DHCPACK message, restart Proxy DHCP\n" << endl;
            DsgShutDownProxyIpuStack();
            // delay a bit for completion
            BcmOperatingSystemFactory::ThreadSleep(1000);
            continue;
        }
        else
            break;

    } while (++dhcploop < 5);

    if ( dhcploop == 5 )
    {
        cout << "IP_U DHCP Fail after 5 trials! return ip_address of 0!" << endl;
        return 0;
    }

    ipAddress.Set(CableCardIpAddress);
    cout << "CableCard IP_U: Got an IP address = " << ipAddress << endl;

    // DhcpOptionLen will be 0-256
    if( DhcpOptionLen )
    {
        cout << "CableCard IP_U: Got Dhcp Reply Option byte length " << (int)DhcpOptionLen << endl;
        if( ret_option && *ropt_len )
        {
            if( DhcpOptionLen < *ropt_len )
            {
                memcpy( ret_option, DhcpOptionByte, DhcpOptionLen );
                *ropt_len = DhcpOptionLen;  // update new length
            }
            else
            {
                cout << " Truncation: Return buffer length is too short (" << *ropt_len << ")" << endl;
                memcpy( ret_option, DhcpOptionByte, *ropt_len );
            }
        }
        else
            cout << "ret_option ptr(" << (int)ret_option << ") is NULL or *ropt_len(" << (int)*ropt_len <<  ") is 0! cannot copied!" << endl;
    }
    else if( ropt_len )  
        *ropt_len = DhcpOptionLen;  // no dhcp reply option


    return CableCardIpAddress;
}

// Handle cablecard removal clean up
//
void BcmCableCardIfThread::HandleCardRemovedCleanUp()
{
	BcmDsgCableCardClient *pClient;
	BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();

    // Always send Shut down Proxy IP_U stack to Ecm
    DsgShutDownProxyIpuStack();
    // delay a bit for completion
    BcmOperatingSystemFactory::ThreadSleep(1000);
    CableCardIpAddress = 0;

    // Closed all opened tunnels if any
	CloseAllCableCardClients( pDsgClientCtlThread );

    // Closed any opened flows
    if( gSocketFlowId )
    {
        cout << "Delete Socket Flow." << endl;
        if( gUdpTcpSocket )
            close(gUdpTcpSocket);
        gUdpTcpSocket = 0;  // Set to 0 so that the RX thread will not wait for this socket anymore
        gSocketFlowId = INVALID_FLOW_ID;
    }
    if( gIpUFlowId )
    {
        cout << "Delete IPU Flow." << endl;
        gIpUFlowId = INVALID_FLOW_ID;
    }

    // indicate removal
    if( pDsgClientCtlThread->Mode() == kDsgAdvanceMode)
        need_to_rescan = true;

    // Spec did not say whats the behavior after card is removed.
    // For now, just set dsg mode to disable and not rescan.
    pDsgClientCtlThread->Mode((uint8) kDsgDisabledMode);

    // reset
    ResetDsgDirVersion();
    dsg_oper_mode = DSG_INVALID_MODE;

}

//
// service_type == 04 /* Socket*/
// 
// typedef struct _CCARD_CONFIGURE_SOCKET_FLOW_APDU
// {
//      protocol_flag       8 uimsbf
//      local_port_number   16 uimsbf
//      remote_port_number  16 uimsbf
//      remote_address_type 8 uimsbf
//
//      if(remote_address_type==0x00) {
//          name_length     8 uimsbf
//          for(int i=0;i<name_length;++i)
//              name_byte   8 uimsbf
//      }
//      if(remote_address_type == 0x01)
//          ipv4_address    32 uimsbf
//      if(remote_address_type == 0x02)
//          ipv6_address    128 uimsbf
//
//      connection_timeout 8 uimsbf
// } CCARD_CONFIGURE_SOCKET_FLOW_APDU;
//
unsigned char BcmCableCardIfThread::HandleSocketFlowConfig( unsigned long flowid, unsigned char *pkt_obj_ptr, unsigned short len )
{
    struct sockaddr_in sockaddr_param;
    uint32 myIpaddress;
    int i, arg = 1;

    // At this time, this may be private ip address, 
    // Need to wait for a non private ip, which means Docsis is on 2-way mode
    GetEstbIpAddress(&myIpaddress);     //return in network byte order
    #if ( SOCKET_REQ_WAIT )
	// Wait until the ESTB Ip address changed....
	while( (myIpaddress & 0x00ffffff) == 0x0064a8c0  )  //x.100.168.192
	{
        cout << "Wait for ESTB ip address changed..." << endl;
		BcmOperatingSystemFactory::ThreadSleep(3000);
        GetEstbIpAddress(&myIpaddress);
	}
    #else
    if( (myIpaddress & 0x00ffffff) == 0x0064a8c0 )
    {
        cout << "STB ip address has not changed. Return Network unavailable(3)!..." << endl;
        return 0x03;    //Network unavailable
    }
    #endif

    // For now, we only support one socket flow
    if( gUdpTcpSocket )
    {
        // ATP test case 170.1 - step 10
        uint16 rport;
        rport = ((uint16)*(pkt_obj_ptr+4) << 8);
        rport += ((uint16)*(pkt_obj_ptr+5) << 0);
        if( rport == SocFlowInfo.remote_port_number )
        {
            cout << "Request denied, remote port number already in use." << endl;
            return 0x08;    //0x08 Request denied, local port already in use or invalid
        }
        cout << "Only support 1 opened socket flow. Please delete flow before open another one." << endl;
        return 0x01;    //0x01 Request denied, number of flows exceeded
    }

	//Saved the socket flow id
	gSocketFlowId  = flowid;

    //clear out the structure first
    memset( (void *)&SocFlowInfo, 0x0, sizeof(CCARD_CONFIGURE_SOCKET_FLOW_APDU) );

    // Copy raw APDU into struct
    // pkt_obj_ptr point to 1 byte service_type == 04(socket), skip it
    pkt_obj_ptr++;
    SocFlowInfo.protocol_flag = *(pkt_obj_ptr++);
    SocFlowInfo.local_port_number = ((uint16)*(pkt_obj_ptr++) << 8);
    SocFlowInfo.local_port_number += ((uint16)*(pkt_obj_ptr++) << 0);
    SocFlowInfo.remote_port_number = ((uint16)*(pkt_obj_ptr++) << 8);
    SocFlowInfo.remote_port_number += ((uint16)*(pkt_obj_ptr++) << 0);
    SocFlowInfo.remote_address_type = *(pkt_obj_ptr++);

    if( SocFlowInfo.remote_address_type == SF_DOMAIN_NAME )
    {
        struct hostent	*tp_hostent;	/* ptr to temp host entry */
        struct in_addr fIpAddress;
        uint8 namelen = *(pkt_obj_ptr++);
        uint8 *pdst = (uint8 *)malloc(namelen+2);

        if( pdst == NULL)
        {
            cout << "Error: Cannot allocate "<< (int)(namelen+2) << " byte buffer for DNS name" << endl;
            return 0x06;    //DNS not supported
        }

        memcpy( pdst, pkt_obj_ptr, namelen ); 
        *(pdst+namelen) = 0;    //terminate string

        cout << "Looking up ipaddress for DNS name: " << pdst << endl;
        if((tp_hostent = gethostbyname((char *) pdst)) == (struct hostent *)NULL)
        {
            /* gethostbyname() failed */
            cout << "Error: gethostbyname failed" << endl;
            return 0x06;    //DNS not supported
        }

        /* check address is the correct size - NetBSD bug */
        if(tp_hostent->h_length != sizeof(struct in_addr))
        {
            /* address isn't a struct in_addr's worth */
            cout << "Error: ip address length (" << tp_hostent->h_length << ") != " << sizeof(struct in_addr) << endl;
            return 0x06;    //DNS not supported
        }

        if(!(tp_hostent->h_addr_list[0]))
        {
            cout << "Error: DNS ipaddress is NULL" << endl;
            return 0x06;    //DNS not supported
        }

        cout << "Storing resolved IP address." << endl;

        /* save the address */
        (void)memcpy( &fIpAddress, tp_hostent->h_addr_list[0], sizeof(struct in_addr));
        /* convert to host byte order */
        fIpAddress.s_addr = (uint32) ntohl((uint32)(fIpAddress.s_addr));

        printf("Resolved %s\n", inet_ntoa(fIpAddress));

        free(pdst);

        SocFlowInfo.Var.ipv4_address = fIpAddress.s_addr;
    }
    else if( SocFlowInfo.remote_address_type == SF_IPV4 )
    {
        SocFlowInfo.Var.ipv4_address  = ((uint32)*(pkt_obj_ptr++) << 24);
        SocFlowInfo.Var.ipv4_address += ((uint32)*(pkt_obj_ptr++) << 16);
        SocFlowInfo.Var.ipv4_address += ((uint32)*(pkt_obj_ptr++) << 8);
        SocFlowInfo.Var.ipv4_address += ((uint32)*(pkt_obj_ptr++) << 0);
    }
    else if( SocFlowInfo.remote_address_type == SF_IPV6 )
    {
        //cout << "TBD: Do not know how to handle socket flow remote-addrees-type ipv6 " << 
        //    (int)SocFlowInfo.remote_address_type << endl;
        SocFlowInfo.Var.pNameOrIpv6addr = pkt_obj_ptr;
        pkt_obj_ptr += 16;      //exactly 16 bytes according to CCIF spec.
        SocFlowInfo.connection_timeout = *(pkt_obj_ptr++);  // Number of seconds the Host will attempt to establish a TCP connection
        return ConfigureIpV6SocketFlow();
    }

    SocFlowInfo.connection_timeout = *(pkt_obj_ptr++);  // Number of seconds the Host will attempt to establish a TCP connection

    // filter on packet destin to me with my eSTB ipaddress and my port
    memset( &sockaddr_param, 0, sizeof(sockaddr_param) );
    sockaddr_param.sin_family = AF_INET;

    gLogMessageRaw << "local port=0x" << hex << (int)SocFlowInfo.local_port_number 
         << " eSTB PriIp=0x" << (int)htonl(myIpaddress)
         << " remote port=0x" << (int)SocFlowInfo.remote_port_number
         << " ipv4 address=0x" << (int)SocFlowInfo.Var.ipv4_address << dec << endl;

    if( SocFlowInfo.local_port_number == 0 )
        gLogMessageRaw << "Warning! local port is 0! Socket recvfrom may not works!" << endl;


    if( SocFlowInfo.protocol_flag == SF_UDP )
    {
        // sockaddr needs netowrk byte order
        sockaddr_param.sin_addr.s_addr = myIpaddress;
        sockaddr_param.sin_port = htons(SocFlowInfo.local_port_number);

        // Creating DS sockets
        if ((gUdpTcpSocket = socket(AF_INET, SOCK_DGRAM, 0/*usually 0 for UDP*/)) < 0)
        {
            gLogMessageRaw << "BcmCableCardIfThread::HandleSocketFlowConfig "
                << "dsgcc failed to create DS-UDP socket flow." << endl;
            gUdpTcpSocket = 0;  // Set to 0 so that the RX thread will not wait for this socket anymore
            return 0x02;    //service_type not available
        }

        // bind to the local port
        if (bind(gUdpTcpSocket, (struct sockaddr *)&sockaddr_param, sizeof(sockaddr_param)) < 0)
        {
            gLogMessageRaw << "BcmCableCardIfThread::HandleSocketFlowConfig "
                << "dsgcc: DS-UDP socket failed to bind!" << endl;
            close(gUdpTcpSocket);
            gUdpTcpSocket = 0;  // Set to 0 so that the RX thread will not wait for this socket anymore
            return 0x02;    //service_type not available
        }

        // Set the socket to be non-blocking.
        ioctl(gUdpTcpSocket, FIONBIO, &arg);

        gLogMessageRaw << "gUdpTcpSocket(UDP) bind to interface(ip=0x" << hex << htonl(myIpaddress)
            << ") and listen on port 0x" << SocFlowInfo.local_port_number << dec << endl;

        gLogMessageRaw << "gUdpTcpSocket(UDP) will send to remote ip=0x" << hex << SocFlowInfo.Var.ipv4_address
            << " with remote port 0x" << SocFlowInfo.remote_port_number << dec << endl;

    }
    else if( SocFlowInfo.protocol_flag == SF_TCP )   // SF_TCP
    {
        int success=0;

        // Creating DS sockets
        if ((gUdpTcpSocket = socket(AF_INET, SOCK_STREAM, 0/*usually 0 for TCP*/)) < 0)
        {
            gLogMessageRaw << "BcmCableCardIfThread::HandleSocketFlowConfig "
                << "dsgcc failed to create DS-TCP socket flow." << endl;
            gUdpTcpSocket = 0;  // Set to 0 so that the RX thread will not wait for this socket anymore
            return 0x02;    //service_type not available
        }

        // bind to the local port
        sockaddr_param.sin_addr.s_addr = myIpaddress;
        sockaddr_param.sin_port = htons(SocFlowInfo.local_port_number);
        if (bind(gUdpTcpSocket, (struct sockaddr *)&sockaddr_param, sizeof(sockaddr_param)) < 0)
        {
            gLogMessageRaw << "BcmCableCardIfThread::HandleSocketFlowConfig "
                << "dsgcc: DS-TCP socket failed to bind!" << endl;
            close(gUdpTcpSocket);
            gUdpTcpSocket = 0;  // Set to 0 so that the RX thread will not wait for this socket anymore
            return 0x02;    //service_type not available
        }

        // Set the socket to be non-blocking for handling timeout parameter properly.
        ioctl(gUdpTcpSocket, FIONBIO, &arg);

        // For TCP, use remote Ip address and port number
        sockaddr_param.sin_addr.s_addr = htonl(SocFlowInfo.Var.ipv4_address);
        sockaddr_param.sin_port = htons(SocFlowInfo.remote_port_number);

		// first Connect
		int rc = connect(gUdpTcpSocket, (struct sockaddr *)&sockaddr_param, sizeof(sockaddr_param));
        // If the connection succeeds, zero is returned. On error, -1 is returned, and errno is set appropriately.   
        if( rc == 0)
        {
            success = 1;
            gLogMessageRaw << "Success after 1st connect, errno = " << (int)errno << "::" << strerror(errno) << endl;
        }
        else
        {
            gLogMessageRaw << "After 1st connect, errno = " << (int)errno << "::" << strerror(errno) << endl;
            
			if ((errno == EINPROGRESS) || (errno == EALREADY) || (errno == EWOULDBLOCK)) 			
			{
                /* test to see if socket is writable before we try */
				struct timeval sock_to; 
				fd_set fdwrite;
				int optval = 0;
				socklen_t optlen = 0;
                sock_to.tv_sec = SocFlowInfo.connection_timeout;
                sock_to.tv_usec = 0;
                FD_ZERO(&fdwrite);
                FD_SET(gUdpTcpSocket, &fdwrite);
				
				// Select returns the number of sockets recently became active.
                if (0 != select(gUdpTcpSocket+1, NULL, &fdwrite, NULL, &sock_to)) 
				{
                    /* double check that the connect completed properly */
                    optlen = sizeof(optval);
                    rc = getsockopt(gUdpTcpSocket, SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen);
                    if (rc == 0 && optval == 0) 
					{
                        success = 1;
                    }
                }
			}
        }

        if( success == 1 )
        {
            /* connect successfull */
            gLogMessageRaw << "gUdpTcpSocket(TCP) connect to remote ip=0x" << hex << SocFlowInfo.Var.ipv4_address
                << " with remote port 0x" << SocFlowInfo.remote_port_number << dec << endl;
        }
        else
        {
            gLogMessageRaw << "BcmCableCardIfThread::HandleSocketFlowConfig "
                << "dsgcc: DS-TCP socket failed to connect!" << endl;
            close(gUdpTcpSocket);
            gUdpTcpSocket = 0;  // Set to 0 so that the RX thread will not wait for this socket anymore
            return 0x09;    //could not establish TCP connection
        }
    }
    else
    {
        gLogMessageRaw << "BcmCableCardIfThread::HandleSocketFlowConfig "
            << "Protocol flag not supported! Only support UDP or TCP" << endl;
        return 0x02;    //service_type not available
    }

    // Setup upstream socket parameter one time here:
    memset(&gUs_sockaddr, 0, sizeof(gUs_sockaddr));
    gUs_sockaddr.sin_addr.s_addr = htonl(SocFlowInfo.Var.ipv4_address);
    gUs_sockaddr.sin_port = htons(SocFlowInfo.remote_port_number);
    gUs_sockaddr.sin_family = AF_INET;

    // if this is the first time, start the receive thread
    if( pfOperatingSystem->GetThreadState() == 0 /*kNotStarted*/ )
    {
    	cout << "\nStarting cablecard socket flow DS receive thread..." << endl;
        pfOperatingSystem->BeginThread(this);
    }

        #if (ENABLE_SOCKET_US_TEST)
    	// debug. start a periodic US test to exercise callback
    	{
	    	int taskId = -1;
		    DsgPrint("\nStarting cablecard socket flow US test thread\n");
    		StartMyThread( "CableCard US TEST", &taskId, (void*(*)(void*)) CableCardSocketFlowUsTestThread); 
	    }
        #endif

    return 0;
}

//
//  Receive flow delete command from cablecard
//
unsigned char BcmCableCardIfThread::HandleDeleteFlowRequest( unsigned long flowid )
{
    if( flowid == gSocketFlowId )
    {
        cout << "Delete Socket Flow." << endl;
        if( gUdpTcpSocket )
            close(gUdpTcpSocket);
        gUdpTcpSocket = 0;  // Set to 0 so that the RX thread will not wait for this socket anymore
        gSocketFlowId = INVALID_FLOW_ID;
    }
    else if( flowid == gIpUFlowId )
    {
		// PR20830:
		if( (fLastDsgMsgApdu == 0x02) || // Entering_One-Way_mode
			(fLastDsgMsgApdu == 0x05) || // eCM Reset
			(fLastDsgMsgApdu == 0x07) )  // eCM cannot forward 2-Way traffic
		{
			// Handle the following cases: One-way mode, ifAdminStatus is (interface 17 on eCM) is down, eCM reset.
			// PR20830: Do not shutdown proxy IP stack, especially if a CableCardIpAddress is already acquired.
			cout << "HandleDeleteFlowRequest - Last DSG_msg APDU value is " << hex << fLastDsgMsgApdu << " .No need to release the IP address." << endl;
		}
		else
		{			
			cout << "Delete IPU Flow & shutdown Proxy Ip stack." << endl;
			// For now: Shut down Proxy IP_U stack.
			// TBD: Need to implement a non-desstructive way by just caching all DHCP data
			// and just prevent IPU data from over thru. eCm will still have IPU stack up.
			if( CableCardIpAddress )
			{
				DsgShutDownProxyIpuStack();
				// delay a bit for completion
				BcmOperatingSystemFactory::ThreadSleep(1000);
				CableCardIpAddress = 0;
			}
		}

        gIpUFlowId = INVALID_FLOW_ID;
    }
    else if( flowid == gDsgFlowId )
    {
        cout << "Delete DSG FlowId TBD: Do not know what to do yet" << endl;
        return 0x06;    // Not authorized
    }
    else
    {
        cout << "HandleDeleteFlowRequest Error! flowId did not match any of the opened flowId." << endl;
        return 0x05;    //flow id does not exist
    }

    return 0;   // return granted

}

//
//  Configure DSG tunnel classifier and parameters
//
void BcmCableCardIfThread::HandleConfigureDsgDirectory(unsigned char *pkt_obj_ptr, unsigned short len )
{
    BcmDsgClassifierSettings cfr, cfr0;
    BcmDsgConfigSettings freqchan_timers;
    BcmIpAddress ip_address;
    BcmDsgCableCardClient *pClient;
    unsigned char *pCfgAdvDsg = pkt_obj_ptr;
    int i, j, ExtChMpegFlow=0;
    unsigned long rx_freq;
    unsigned char num_port, num_rx_freq, vct_id_included, cidT, cidL, dir_entry_type;
    unsigned short port;
    uint32 card_client_id = CARD_ENTRY_CLIENT_ID_START;

    BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();

    if( pDsgClientCtlThread == NULL )
    {
        cout << "HandleConfigureDsgDirectory:: Error! No instance of client Controller Thread object!" << endl;
        return;
    }

    // Now, parse the APDU. First byte is the number of filters
    vct_id_included = (*pCfgAdvDsg++ & 0x01);   //only b0 valid

    if( dsg_directory_version == (uint16)*pCfgAdvDsg )
    {
        cout << "HandleConfigureDsgDirectory:: dir version is the same. No further processing." << endl;
        return;
    }
        
    // Multiple Dsg Dir support: Closed all opened tunnels first
	CloseAllCableCardClients( pDsgClientCtlThread );

    // debug
    DumpBytes( "DSG_Dir Raw:", pkt_obj_ptr, len );

    if (!fpMutex->Lock())
    {
        gErrorMsg(fMessageLogSettings, "HandleConfigureDsgDirectory")
            << "Failed to lock my mutex!" << endl;

        return;
    }

    // update dir_version
    dsg_directory_version = (uint16)*pCfgAdvDsg++;
    cout << "(MuLock)HandleConfigureDsgDirectory:: Parsing dir version " << (int)dsg_directory_version << endl;

    //Process Host entries
    NumOpenTunnel = *pCfgAdvDsg++;
    for (i=0; i<NumOpenTunnel; i++ ) 
    {
        // clear cfr
        cfr = cfr0;

        // create one cablecard client for each mac address
        pClient = new BcmDsgCableCardClient(this);

        // Unique client port for Deregister purposes
        pClient->ClientPort(kClientPort+i-ExtChMpegFlow);

        // 12/07/07: Host now assume IP/UDP header are all removed.
        // For host entry, remove IP/UDP header + BT header if present
        pClient->fRemoveUdpHeader = true;

        // parse dsg_client_id TLV
        cidT = *pCfgAdvDsg++;
        cidL = *pCfgAdvDsg++;

        switch( cidT )
        {
        case kBroadcast:
        case kCaSystemId:
        case kApplicationId:
            {
                uint32 cid = 0;
                cout << "dsg_dir parse dsg_client_id TLV: T=" << pClient->IdTypeName( (const uint32) cidT ) << " L=" << (int)cidL;
                if( cidL <= 4 )
                {
                    while( cidL-- )
                    {
                        cid <<= 8;
                        cid += *pCfgAdvDsg++;
                    }
                    cout << " V=0x" << hex << (int) cid << dec << "(client ID)" << endl;
                    pClient->Id( (const uint32)cid );
                    pClient->IdType( (const uint32) cidT );
                    if( cidT==kBroadcast && cidL==2 && cid == 0 )
                        cout << "Warning! DSG Broadcast ID of 0 is prohibited!" << endl;
                }
                else
                {
                    cout << " invalid Len " << (int)cidL << " dsg_dir parse error! no tunnel opend for this entry!" << endl;
                    pCfgAdvDsg += cidL;
                }
            }
            break;

        case kWellKnownMacAddress:
            {
                cout << "dsg_dir parse dsg_client_id TLV: T=[WellKnown Mac address] " << (int)cidT;
                if( cidL == 6 )
                {
                    BcmMacAddress dsg_mac_address((const byte *) pCfgAdvDsg);
                    pClient->ClientAddress(dsg_mac_address);
                    cout << " L=6, V=" << dsg_mac_address << endl;
                    pCfgAdvDsg += 6;
                    //pClient->IdType( (const uint32)cid2pclientTypeMap[cidT] );
                    pClient->IdType( (const uint32) cidT );
                }
                else
                {
                    cout << " Len is NOT=6! dsg_dir parse error! no tunnel opend for this entry!" << endl;
                    pCfgAdvDsg += cidL;
                }
            }
            break;

        default:
            cout << "dsg_dir parse error in dsg_client_id (unkownType)" << endl;
            break;

        }   //end switch

        dir_entry_type = *pCfgAdvDsg++;

        if( dir_entry_type == 0x01 )    //DSG filter
        {
            cout << "dsg_dir parsing dsg filter.." << endl;
            // Do not used TunnelId as it will be assigned by eCM when openTunnel
            //pClient->TunnelId( (const uint32)*pCfgAdvDsg++);
            pClient->CableCardTunnelSetting().fRuleId = *pCfgAdvDsg++;
            pClient->CableCardTunnelSetting().fRuleIdSpecified = true;
            pClient->CableCardTunnelSetting().fRulePriority = *pCfgAdvDsg++;
            pClient->CableCardTunnelSetting().fRulePrioritySpecified = true;
            BcmMacAddress dsg_mac_address((const byte *) pCfgAdvDsg);
            pClient->CableCardTunnelSetting().TunnelAddress(dsg_mac_address); // also put in tunnelsetting class
            pCfgAdvDsg += 6;

            // For all valid Ip and mask, makesure it is non-zero
            ip_address.Set((uint8 *) pCfgAdvDsg);
            if( ip_address == kAllZerosIp )
            {
                // OC-SP-CCIF2.0-I20-091211: ..A value of all zeros implies all values of SourceIP Address, i.e., 
                // this parameter was not specified in the DCD message.
                // To make it NOT specified, do not write anythng to the cfr, skip the cfr SourceIpMask processing as well
                pCfgAdvDsg += 4;
            }
            else
            {
                // Store non-zero Source Ip Address into cfr
                cfr.SourceIpAddr( ip_address );

                // Get SourceIp mask into placeholder
            pCfgAdvDsg += 4;
            ip_address.Set((uint8 *) pCfgAdvDsg);
                // OC-SP-CCIF2.0-I20-091211: A value of all ones implies that all 32 bits of the Source IP Address are 
                // to be used for filtering. When source_IP_address is present in the DCD message and
                // source_IP_mask is not present, a value of all ones is to be used for source_IP_mask.
                // In any case, cablecard will send the correct ipMask
                // Just makesure it is not all 0's. treat it as NOT specficied
            if( ip_address != kAllZerosIp )
            {
                cfr.SourceIpMask( ip_address );
                // Apply subnet mask to ip address for layer 3 filtering
                ip_address.Set( cfr.SourceIpAddr().Get() & cfr.SourceIpMask().Get() );
                cfr.SourceIpAddr( ip_address );
            }
            }
            pCfgAdvDsg += 4;

            ip_address.Set((uint8 *) pCfgAdvDsg);
            if( ip_address != kAllZerosIp )
                cfr.DestIpAddr( ip_address );
            pCfgAdvDsg += 4;

            num_port = 2;
            for(j=0; j<num_port; j++ )
            {
                port = *pCfgAdvDsg++;
                port <<= 8;
                port |= *pCfgAdvDsg++;
                if( j==0 )
                    cfr.DestPortStart( port );
                else
                    cfr.DestPortEnd( port );
            }

            // how to use this ucid ????
            pClient->SetDsgDirUcid( *pCfgAdvDsg++ );
        }
        else
        {
            // what to do with Extended Channel MPEG Flow????
            cout << "dsg_dir parsing Extended Channel MPEG flow. What to do???..\n" << endl;
            delete pClient;     //OK??
            ExtChMpegFlow++;    //Since we do not open this flow, increment to makesure 1st client port opened is kClientPort
            continue;
        }

        pClient->CableCardTunnelSetting().add(cfr);

        pDsgClientCtlThread->RegisterCableCardAdvancedModeClient(pClient);	//direct calls
        cout << "Host_entry ClientInfo: Tunnel Mac=" << pClient->CableCardTunnelSetting().TunnelAddress() << *pClient << endl;
    }

    //Process card entries and use the leftover i value as starting point
    NumOpenTunnel += *pCfgAdvDsg++;
    for ( ; i<NumOpenTunnel; i++ ) 
    {
        // clear cfr
        cfr = cfr0;

        // create one cablecard client for each mac address
        pClient = new BcmDsgCableCardClient(this);

        // Unique client port for Deregister purposes
        pClient->ClientPort(kClientPort+i-ExtChMpegFlow);

        // No specify type for dsg_dir, use my own define for card-entry
        pClient->Id( (const uint32)card_client_id++ );
        pClient->IdType( (const uint32)kCardEntryClientType );

        {
            //pClient->TunnelId( (const uint32)*pCfgAdvDsg++);
            //skip tunnel priority, no place to hold it
            pClient->CableCardTunnelSetting().fRuleId = *pCfgAdvDsg++;
            pClient->CableCardTunnelSetting().fRuleIdSpecified = true;
            pClient->CableCardTunnelSetting().fRulePriority = *pCfgAdvDsg++;
            pClient->CableCardTunnelSetting().fRulePrioritySpecified = true;
            BcmMacAddress dsg_mac_address((const byte *) pCfgAdvDsg);
            pClient->CableCardTunnelSetting().TunnelAddress(dsg_mac_address); // also put in tunnelsetting class
            pCfgAdvDsg += 6;

            // For all valid Ip and mask, makesure it is non-zero
            ip_address.Set((uint8 *) pCfgAdvDsg);
            if( ip_address == kAllZerosIp )
            {
                // OC-SP-CCIF2.0-I20-091211: ..A value of all zeros implies all values of SourceIP Address, i.e., 
                // this parameter was not specified in the DCD message.
                // To make it NOT specified, do not write anythng to the cfr, skip the cfr SourceIpMask processing as well
                pCfgAdvDsg += 4;
            }
            else
            {
                // Store non-zero Source Ip Address into cfr
                cfr.SourceIpAddr( ip_address );

                // Get SourceIp mask into placeholder
            pCfgAdvDsg += 4;
            ip_address.Set((uint8 *) pCfgAdvDsg);
                // OC-SP-CCIF2.0-I20-091211: A value of all ones implies that all 32 bits of the Source IP Address are 
                // to be used for filtering. When source_IP_address is present in the DCD message and
                // source_IP_mask is not present, a value of all ones is to be used for source_IP_mask.
                // In any case, cablecard will send the correct ipMask
                // Just makesure it is not all 0's. treat it as NOT specficied
            if( ip_address != kAllZerosIp )
            {
                cfr.SourceIpMask( ip_address );
                // Apply subnet mask to ip address for layer 3 filtering
                ip_address.Set( cfr.SourceIpAddr().Get() & cfr.SourceIpMask().Get() );
                cfr.SourceIpAddr( ip_address );
            }
            }
            pCfgAdvDsg += 4;

            ip_address.Set((uint8 *) pCfgAdvDsg);
            if( ip_address != kAllZerosIp )
                cfr.DestIpAddr( ip_address );
            pCfgAdvDsg += 4;

            num_port = 2;
            for(j=0; j<num_port; j++ )
            {
                port = *pCfgAdvDsg++;
                port <<= 8;
                port |= *pCfgAdvDsg++;
                if( j==0 )
                    cfr.DestPortStart( port );
                else
                    cfr.DestPortEnd( port );
            }

        }

        pClient->CableCardTunnelSetting().add(cfr);

        pDsgClientCtlThread->RegisterCableCardAdvancedModeClient(pClient);	//Direct calls
        cout << "Card_entry ClientInfo: Tunnel Mac=" << pClient->CableCardTunnelSetting().TunnelAddress() << *pClient << endl;

    }

    if (!fpMutex->Unlock())
    {
        gWarningMsg(fMessageLogSettings, "HandleConfigureDsgDirectory")
            << "Failed to unlock my mutex!" << endl;
    }

    // Now get the freq channel list if any
    num_rx_freq = *pCfgAdvDsg++;
    if( num_rx_freq )
    {
        for(j=0; j<num_rx_freq; j++ )
        {
            rx_freq = *pCfgAdvDsg++;
            rx_freq <<= 8;
            rx_freq |= *pCfgAdvDsg++;
            rx_freq <<= 8;
            rx_freq |= *pCfgAdvDsg++;
            rx_freq <<= 8;
            rx_freq |= *pCfgAdvDsg++;

            freqchan_timers.SetFreqChannelList(rx_freq);
        }
    }

    // Now set the 4 timeouts
    port = *pCfgAdvDsg++;
    port <<= 8;
    port |= *pCfgAdvDsg++;
    // Valid Tdsg1 timer range value is 1-65535
    if( port )
        freqchan_timers.SetTdsg1Timeout(port);

    port = *pCfgAdvDsg++;
    port <<= 8;
    port |= *pCfgAdvDsg++;
    // Valid Tdsg2 timer range value is 1-65535
    if( port )
        freqchan_timers.SetTdsg2Timeout(port);

    port = *pCfgAdvDsg++;
    port <<= 8;
    port |= *pCfgAdvDsg++;
    // Valid Tdsg3 timer range value is 0-65535
    freqchan_timers.SetTdsg3Timeout(port);

    port = *pCfgAdvDsg++;
    port <<= 8;
    port |= *pCfgAdvDsg++;
    // Valid Tdsg4 timer range value is 0-65535
    freqchan_timers.SetTdsg4Timeout(port);

    // copy this into the client controller object
    // Also serialize call to be sent to eCM
    pDsgClientCtlThread->SetDsgConfigSetting( freqchan_timers );

    if( vct_id_included )
    {
        port = *pCfgAdvDsg++;
        port <<= 8;
        port |= *pCfgAdvDsg++;
        vct_id = port;
    }

    cout << "(MuUnlock)HandleConfigureDsgDirectory:: DONE Parsing DsgDirectory.\n" << endl;
}

// Send tunnel pkt to Host
int BcmCableCardIfThread::sendTunnelDataToHost( const unsigned char *pBufData, uint16 pktLen, uint32 clientType, uint32 clientId )
{
    //pkt_count++;
	// Send DSG pacekt to Host instead of cablecard
    if (gTunnelDebugLevel & 0x01)
        DsgPrint("Rx DsgTunnel pkt for Host! cType=%d, cId=0x%08lx\n", clientType, clientId);
    // Remove_Header_Bytes does not applied. Send full Ethernet packet
    return BcmSendDSGTunnelDataToHost( (unsigned char *)pBufData, pktLen, clientType, clientId );
}

// Send tunnel pkt to POD driver
int BcmCableCardIfThread::sendTunnelDataToCableCard( const unsigned char *pBufData, uint16 pktLen )
{
    pkt_count++;
	// Send potential DSG pacekt to final destination
    // For CableCard POD SW, it block until data is sent out to the POD or timeout. 
    // When the function returns, the packet can always be released
    return BcmSendDSGTunnelDataToPOD( (unsigned char *)pBufData + Remove_Header_Bytes, pktLen - Remove_Header_Bytes, gDsgFlowId );
}

// Send DS IPU data pkt to POD driver
void BcmCableCardIfThread::SendDsDataToCableCard( unsigned char *pBufData, uint16 pktLen )
{
    ipu_count++;
    if (gTunnelDebugLevel & 0x10)
    {
    	DsgPrint("DS_IPU pkt with len %d\n", pktLen);
	    DumpBytes("DS_IPU", pBufData, pktLen);
    }

    // For CableCard POD SW, this function block until data is sent out to the POD or timeout. 
    // When the function returns, the packet can always be released
    BcmSendDataToPOD( (unsigned char *)pBufData, pktLen, gIpUFlowId );
}

// 
void BcmCableCardIfThread::sendFixedAppsTunnelRequestToCableCard( void )
{
    // open 1 app tunnel with Application ID 3
    const char dsg_message[] = { 0x00, 0x01, 0x00, 0x03 };
    POD_Api_Send_DSG_Message( (void *)dsg_message, 4 );
}

// 
void BcmCableCardIfThread::sendUcidToCableCard( unsigned char ucid )
{
    // tell cablecard about UCID
    char dsg_message[2];

    dsg_message[0] = 0x01;

	// PR20830: Remember the last DSG_message APDU value.
	fLastDsgMsgApdu = (uint8) dsg_message[0];

    dsg_message[1] = ucid;
    POD_Api_Send_DSG_Message( (void *)dsg_message, 2 );
}

// 
void BcmCableCardIfThread::sendTwoWayModeToCableCard( int mode )
{
	//(0=1way/1=2way)
	if( mode )
		DocsisRangedRegistered = true;
	else
    {
        // tell cablecard about one-way mode
        const char dsg_message = 0x02;

		// PR20830: Remember the last DSG_message APDU value.
		fLastDsgMsgApdu = (uint8) dsg_message;

		DocsisRangedRegistered = false;

        POD_Api_Send_DSG_Message( (void *)&dsg_message, 1 );

		// OC-ATP 14.1.1 step 27,45:
		if( CableCardIpAddress )
		{
			// Send lost_flow_ind, card will send delete_flow_request
			cout << "OneWayMode: Send Lost_Flow_Ind (flowid=0x" << hex << pgCableCardIf->GetIpUFlowId() << dec << ") to cablecard" << endl;
			POD_Api_Lost_Flow_Ind( pgCableCardIf->GetIpUFlowId() , LOSTFLOW_NET_BUSY );	//netowork down
		}
    }
}

// 
void BcmCableCardIfThread::sendDsScanCompleteToCableCard( void )
{
    const char dsg_message = 0x03;

	// PR20830: Remember the last DSG_message APDU value.
	fLastDsgMsgApdu = (uint8) dsg_message;

    POD_Api_Send_DSG_Message( (void *)&dsg_message, 1 );
}

// 
void BcmCableCardIfThread::sendDccDepartToCableCard( unsigned char init_type )
{
    // tell cablecard about init_type
    char dsg_message[2];

    dsg_message[0] = 0x04;

	// PR20830: Remember the last DSG_message APDU value.
	fLastDsgMsgApdu = (uint8) dsg_message[0];

    dsg_message[1] = init_type;
    POD_Api_Send_DSG_Message( (void *)dsg_message, 2 );

}

// 
void BcmCableCardIfThread::sendDisabledForwardingTypeToCableCard( unsigned char disabled_fwd_type )
{
    // tell cablecard about init_type
    char dsg_message[2];

    dsg_message[0] = 0x07;

	// PR20830: Remember the last DSG_message APDU value.
	fLastDsgMsgApdu = (uint8) dsg_message[0];

    // I0-9 implementation
    dsg_message[1] = disabled_fwd_type;     //eCM send with bitmask

    POD_Api_Send_DSG_Message( (void *)dsg_message, 2 );

    // OC-ATP 14.1.1 step 81:
    if( disabled_fwd_type == 0x04 && CableCardIpAddress ) 	// Forwarding interface administratively down
    {
        gLogMessageRaw << "eCM ifAdminStatus.17 is down!" << endl;
        POD_Api_Lost_Flow_Ind( pgCableCardIf->GetIpUFlowId() , LOSTFLOW_NET_BUSY );	//netowork down
    }
}

// 
void BcmCableCardIfThread::sendEcmResetToCableCard( void )
{
    char dsg_message, i;
    BcmDsgCableCardClient *pClient;
    BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();

    gLogMessageRaw << "Send DSG_message APDU (eCM_Reset) to cablecard!" << endl;
    dsg_message = 0x05;     //eCM Reset message_type

	// PR20830: Remember the last DSG_message APDU value.
	fLastDsgMsgApdu = (uint8) dsg_message;

    POD_Api_Send_DSG_Message( (void *)&dsg_message, 1 );

    // eCM has reset, Docsis is no longer ranged and registered. Process the next dsg_dir coming in
    DocsisRangedRegistered = false;
    ResetDsgDirVersion();

    // eCM has reset, so all opened tunnels are gone in the eCM, clean up on the dsgcc side
    CloseAllCableCardClients( pDsgClientCtlThread, false );

    // OCAP test expected behavior
    if( CableCardIpAddress )
    {
        // OC-ATP 14.1.1 step 65:
        // Send lost_flow_ind
        cout << "Send Lost_Flow_Ind (flowid=0x" << hex << pgCableCardIf->GetIpUFlowId() << dec << ") to cablecard" << endl;
        POD_Api_Lost_Flow_Ind( pgCableCardIf->GetIpUFlowId() , LOSTFLOW_NET_BUSY );	//netowork down
        // eCM has reset, so the proxy Ip stack is gone. Go ahead and reset Cablecard Ip Address
        // PR20830: Do not reset CableCard IP address. Remember it for DHCP INIT-REBOOT. CableCardIpAddress = 0;
		cout << "sendEcmResetToCableCard: Current CableCardIpAddress is 0x" << hex << CableCardIpAddress << endl;

        // OC-ATP 14.1.1 step 65, cablecard will delete the flow, no need to auto recover.
        EcmResetRecoveryForIpu = false;


        #if 0    //deprecated ipu auto recovery for above
        cout << "Suspend IPU Flow & restart Proxy Ip stack when eCM is up." << endl;
        // eCM has reset, so the proxy Ip stack is gone
        // After Docsis is ranged and registered, redo proxy dhcp for cablecard
        // reset and set some flags
        CableCardIpAddress = 0;
        DocsisRangedRegistered = false;
        EcmResetRecoveryForIpu = true;

        // Need to use thread to continue recovery. if Socket Thread is not started, start the receive thread
        if( pfOperatingSystem->GetThreadState() == 0 /*kNotStarted*/ )
            pfOperatingSystem->BeginThread(this);
        #endif
    }
}

// Debug helper function
#define DPL 16
void BcmCableCardIfThread::DumpBytes(const char *pname, unsigned char *buf,int length)
{
	int k = 0;
	int i;
	for(i=0;i<length;i++)
	{
		if(i%DPL == 0)
			DsgPrint("[%s].%d : ", pname, k++);
		DsgPrint("%02x ",buf[i]);
		if(i%DPL == DPL-1)
			DsgPrint("\n");
	}
	DsgPrint("\n\n");
}

//
void BcmCableCardIfThread::StartDsRxThread( void )
{
	cout << "\nStarting cablecard IP_Unicast DS receive thread..." << endl;
    pfOperatingSystem->BeginThread(this);
}

//
void BcmCableCardIfThread::SetCableCardDhcpReplyOption( char * pDhcpResponse, unsigned char optionlen )
{
    // being unsigned char, Max optionlen is 255, it will fit in DhcpOptionByte[]
    DhcpOptionLen = optionlen;
    memcpy( DhcpOptionByte, pDhcpResponse, DhcpOptionLen );
}

// Helper function
void BcmCableCardIfThread::CloseAllCableCardClients( BcmDsgClientCtlThread *pDsgClientCtrl, bool CloseEcmTunnels )
{
    // Create a mutex auto-lock object.  This will automatically release
    // the lock when it goes out of scope (ie when we return).
    BcmMutexSemaphoreAutoLock Lock(fpMutex);

    BcmDsgCableCardClient *pClient;
    int i;

    // Sanity check
    if( NumOpenTunnel == 0 ) 
        return;

    // Sync call to close all eCM tunnels FIRST BEFORE deleting cablecard client objects (pClient)
    if( CloseEcmTunnels == true )
        pDsgClientCtrl->ResetEcmTunnels();

    cout << "\n(MuLock)Close/Delete " << dec << (int)NumOpenTunnel << " CableCard Clients Objects..." << endl;
    for (i=0; i<NumOpenTunnel; i++ ) 
    {
        uint32 port = kClientPort+i;
        pClient = (BcmDsgCableCardClient *)pDsgClientCtrl->DsgClientManager().FindByPort(port);
        if (pClient)
        {
            // Added
            if( pDsgClientCtrl->DsgClientManager().UnSubscribe( pClient->TunnelId(), pClient) == false )
            {
                gErrorMsg( fMessageLogSettings, "CloseAllCableCardClients" )
                    <<"Failed to unsubscribe Cablecard Client with TunnelId = " << pClient->TunnelId() << endl;
            }
            cout << "Unsubscribe & Del client port " << dec << (int)port << ". ptr=0x" << hex << (int)pClient << dec << endl;
            delete pClient;
        }
    }
    NumOpenTunnel = 0;

    // Reset dsgcc internal states
    pDsgClientCtrl->ResetTunnels();
}

// IpV6 stuff
unsigned char BcmCableCardIfThread::ConfigureIpV6SocketFlow( void )
{
    sockaddr_in6 sockaddr_param;
    BcmIpV6Address ipAddress, myIpAddress;
    int i, arg = 1;
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[INET6_ADDRSTRLEN];

    ipAddress.Set((const uint8*)SocFlowInfo.Var.pNameOrIpv6addr);
    gLogMessageRaw << "\nFrom CC:Remote ipv6 address=0x" << ipAddress << hex;
    #if 0   //raw print 16 byte ipv6 address
    for( i=0; i<16; i++)
    {
        cout << (int)*(uint8 *)(SocFlowInfo.Var.pNameOrIpv6addr+i);
        if( i%2 )
            cout << ":";
    }
    #endif
    gLogMessageRaw << ", local port=0x" << (int)SocFlowInfo.local_port_number 
         << ", remote port=0x" << (int)SocFlowInfo.remote_port_number << dec << endl << endl;


    if (getifaddrs(&ifaddr) == -1) 
    {
        perror("getifaddrs");
        return 0x02;    //service_type not available
    }

    // Pre-set here bind to any
    sockaddr_param.sin6_addr = in6addr_any;         // bind to any
    myIpAddress.Set((uint8 *) &(sockaddr_param.sin6_addr)); 

    /* Walk through linked list, maintaining head pointer so we
       can free list later */
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

     #if 0
        /* Debug: Display interface name and family (including symbolic
           form of the latter for the common families) */
        printf("%s  address family: %d%s\n",
                ifa->ifa_name, family,
                (family == AF_PACKET) ? " (AF_PACKET)" :
                (family == AF_INET) ?   " (AF_INET)" :
                (family == AF_INET6) ?  " (AF_INET6)" : "");

        /* For an AF_INET* interface address, display the address */
        if (family == AF_INET || family == AF_INET6) 
        {
            s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                          sizeof(struct sockaddr_in6),
                    host, INET6_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
            if (s != 0) 
            {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("\taddress: <%s>\n", host);
        }
     #endif

         if( (strncmp(ifa->ifa_name, "eth0", 4)==0) && (ifa->ifa_addr->sa_family == AF_INET6) )
         {
             uint8 *pipv6=(uint8*)(&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr);
             ipAddress.Set((const uint8*)pipv6);
             gLogMessageRaw << ifa->ifa_name << ": Found Ipv6 address 0x" << ipAddress << endl;

             // TBD which one ot use???
             //if( (pipv6[0]==0xfe) && (pipv6[1]==0x80) )      // Use my link lock ipv6 address
             if( (pipv6[0]==0x40) && (pipv6[1]==0x00) )      // Use my Global assigned ipv6 address
             {
                 myIpAddress.Set((const uint8*)pipv6);
                 gLogMessageRaw << ">>Pick My Ipv6 address as 0x" << myIpAddress << " from IF " << ifa->ifa_name << endl;
             }
          }
    }
    cout << endl;
    freeifaddrs(ifaddr);

    // Setup bind to the local port
    memset(&sockaddr_param, 0, sizeof(sockaddr_param));
    sockaddr_param.sin6_family = AF_INET6;
    myIpAddress.Get((uint8 *) &(sockaddr_param.sin6_addr));   // TBD: Use my local ipv6 address cause bind failed!
    sockaddr_param.sin6_port = htons(SocFlowInfo.local_port_number);

    if( SocFlowInfo.local_port_number == 0 )
        gLogMessageRaw << "Warning! local port is 0! Socket recvfrom may not works!" << endl;

    // populate remote ip into our ipv6 object
    ipAddress.Set((const uint8*)SocFlowInfo.Var.pNameOrIpv6addr);

    if( SocFlowInfo.protocol_flag == SF_UDP )
    {
        // Creating DS sockets
        if ((gUdpTcpSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        {
            gLogMessageRaw << "BcmCableCardIfThread::HandleSocketFlowConfig "
                << "dsgcc failed to create DS-UDP ipv6 socket flow." << endl;
            gUdpTcpSocket = 0;  // Set to 0 so that the RX thread will not wait for this socket anymore
            return 0x02;    //service_type not available
        }

        // bind to the local port                  
        if (bind(gUdpTcpSocket, (struct sockaddr *)&sockaddr_param, sizeof(sockaddr_param)) < 0)
        {
            perror("DS-UDP ipv6 bind failed");
            gLogMessageRaw << "BcmCableCardIfThread::HandleSocketFlowConfig " << "errno=" << errno << endl;
            close(gUdpTcpSocket);
            gUdpTcpSocket = 0;  // Set to 0 so that the RX thread will not wait for this socket anymore
            return 0x02;    //service_type not available
        }

        // Set the socket to be non-blocking.
        ioctl(gUdpTcpSocket, FIONBIO, &arg);

        gLogMessageRaw << "gUdpTcpSocket(UDP) bind to local(ip=0x" << hex << myIpAddress
            << ") and listen on port 0x" << SocFlowInfo.local_port_number << dec << endl;

        gLogMessageRaw << "gUdpTcpSocket(UDP) will send to remote ip=0x" << hex << ipAddress
            << " with remote port 0x" << SocFlowInfo.remote_port_number << dec << endl;

    }
    else if( SocFlowInfo.protocol_flag == SF_TCP )   // SF_TCP
    {
        int success=0;

        // Creating DS sockets
        if ((gUdpTcpSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0)
        {
            gLogMessageRaw << "BcmCableCardIfThread::HandleSocketFlowConfig "
                << "dsgcc failed to create DS-TCP ipv6 socket flow." << endl;
            gUdpTcpSocket = 0;  // Set to 0 so that the RX thread will not wait for this socket anymore
            return 0x02;    //service_type not available
        }

        gLogMessageRaw << "gUdpTcpSocket(TCP) bind to interface(ip=0x" << hex << myIpAddress
            << ") and listen on port 0x" << SocFlowInfo.local_port_number << dec << endl;

        if (bind(gUdpTcpSocket, (struct sockaddr *)&sockaddr_param, sizeof(sockaddr_param)) < 0)
        {
            perror("DS-TCP ipv6 bind failed");
            gLogMessageRaw << "BcmCableCardIfThread::HandleSocketFlowConfig " << "errno=" << errno << endl;
            close(gUdpTcpSocket);
            gUdpTcpSocket = 0;  // Set to 0 so that the RX thread will not wait for this socket anymore
            return 0x02;    //service_type not available
        }

        // Set the socket to be non-blocking for handling timeout parameter properly.
        ioctl(gUdpTcpSocket, FIONBIO, &arg);

        // For TCP, use remote Ip address and port number
        ipAddress.Get((uint8 *) &(sockaddr_param.sin6_addr));   // remote
        sockaddr_param.sin6_port = htons(SocFlowInfo.remote_port_number);

        gLogMessageRaw << "gUdpTcpSocket(TCP) connect to remote(ip=0x" << hex << ipAddress
            << ") and remote port 0x" << SocFlowInfo.remote_port_number << dec << endl;

		// first Connect
		int rc = connect(gUdpTcpSocket, (struct sockaddr *)&sockaddr_param, sizeof(sockaddr_param));
        // If the connection succeeds, zero is returned. On error, -1 is returned, and errno is set appropriately.   
        if( rc == 0)
        {
            success = 1;
            gLogMessageRaw << "Success after 1st connect, errno = " << (int)errno << "::" << strerror(errno) << endl;
        }
        else
        {
            gLogMessageRaw << "After 1st connect, errno = " << (int)errno << "::" << strerror(errno) << endl;
            
			if ((errno == EINPROGRESS) || (errno == EALREADY) || (errno == EWOULDBLOCK)) 			
			{
                /* test to see if socket is writable before we try */
				struct timeval sock_to; 
				fd_set fdwrite;
				int optval = 0;
				socklen_t optlen = 0;
                sock_to.tv_sec = SocFlowInfo.connection_timeout;
                sock_to.tv_usec = 0;
                FD_ZERO(&fdwrite);
                FD_SET(gUdpTcpSocket, &fdwrite);
				
				// Select returns the number of sockets recently became active.
                if (0 != select(gUdpTcpSocket+1, NULL, &fdwrite, NULL, &sock_to)) 
				{
                    /* double check that the connect completed properly */
                    optlen = sizeof(optval);
                    rc = getsockopt(gUdpTcpSocket, SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen);
                    if (rc == 0 && optval == 0) 
					{
                        success = 1;
                    }
                }
			}
        }

        if( success == 1 )
        {
            /* connect successfull */
            gLogMessageRaw << "gUdpTcpSocket(TCP) connect to remote ip=0x" << hex << ipAddress
                << " with remote port 0x" << SocFlowInfo.remote_port_number << dec << endl;
        }
        else
        {
            gLogMessageRaw << "BcmCableCardIfThread::HandleSocketFlowConfig "
                << "dsgcc: DS-TCP ipv6 socket failed to connect!" << endl;
            close(gUdpTcpSocket);
            gUdpTcpSocket = 0;  // Set to 0 so that the RX thread will not wait for this socket anymore
            return 0x09;    //could not establish TCP connection
        }
    }
    else
    {
        gLogMessageRaw << "BcmCableCardIfThread::HandleSocketFlowConfig "
            << "Protocol flag not supported! Only support UDP or TCP" << endl;
        return 0x02;    //service_type not available
    }

    // Setup upstream socket parameter one time here:
    memset(&gUs_sock6addr, 0, sizeof(gUs_sock6addr));
    ipAddress.Get((uint8 *) &(gUs_sock6addr.sin6_addr));   // remote
    gUs_sock6addr.sin6_port = htons(SocFlowInfo.remote_port_number);
    gUs_sock6addr.sin6_family = AF_INET6;

    // if this is the first time, start the receive thread
    if( pfOperatingSystem->GetThreadState() == 0 /*kNotStarted*/ )
    {
    	cout << "\nStarting cablecard socket flow DS receive thread..." << endl;
        pfOperatingSystem->BeginThread(this);
    }

        #if (ENABLE_SOCKET_US_TEST)
    	// debug. start a periodic US test to exercise callback
    	{
	    	int taskId = -1;
		    DsgPrint("\nStarting cablecard socket flow US test thread\n");
    		StartMyThread( "CableCard US TEST", &taskId, (void*(*)(void*)) CableCardSocketFlowUsTestThread); 
	    }
        #endif

    return 0;

}
//********************** Export local procedures to POD driver without name-mangling ***********************
// CableCardCallbackXXXXXXX()
// These are the functions called by the cablecard driver directly
//

//
// First function called by cablecard driver to set basic mode and DSG tunnels
//
// pkt_obj_ptr format:
// operational_mode - 8bit
// if ((operation_mode == DSG_mode) || (operation_mode == DSG_one-way_mode)) 
// {
//   number_MAC_addresses - 8it
//   for (i=0; i<number_MAC_addresses; i++) 
//   {
//     DSG_MAC_address - 48bit
//   }
//   remove_header_bytes - 16bit
// }
//
void CableCardCallbackSetDSGMode( unsigned char * pkt_obj_ptr, unsigned short len )
{
    unsigned char oper_mode = *pkt_obj_ptr;
    unsigned char *pmac_struct = pkt_obj_ptr+1;

    DsgPrint("!!! Received Set_DSG_mode from CableCard !!!\n");
    DsgPrint("Operational Mode (OOB=0, DSG=1, DSG1way=2 DSGAdvMode=3) = %x\n", oper_mode );
    if( len > 1 )
        DsgPrint("Number of MacAddress = %d\n", *pmac_struct );

    // First time?
	if( pgCableCardIf == NULL )
    {
        if( (pgCableCardIf = new BcmCableCardIfThread) == NULL )
        {
            DsgPrint("!PANIC! Cannot create CableCard If Thread!\n");
            return;
        }
    }

   	pgCableCardIf->HandleSetDsgMode( oper_mode, pmac_struct );

}

//
// Cablecard driver provides the flow ID for use in forwarding DSG tunnel pkt
//
void CableCardCallbackDSGFlowID( unsigned long flow_id )
{
    // First time?
	if( pgCableCardIf == NULL )
    {
        if( (pgCableCardIf = new BcmCableCardIfThread) == NULL )
        {
            DsgPrint("!PANIC! Cannot create CableCard If Thread!\n");
            return;
        }
    }

	pgCableCardIf->SetDsgFlowId(flow_id);
	DsgPrint("Received DSG flow id=0x%08lx\n", pgCableCardIf->GetDsgFlowId() );


}

//
void CableCardCallbackConfig_Advanced_DSG( unsigned char *pkt_obj_ptr, unsigned short len )
{
    if( pgCableCardIf )
	{
        // debug
        pgCableCardIf->DumpBytes( "CfgAdvDsg Raw:", pkt_obj_ptr, len );
        pgCableCardIf->HandleConfigureAdvanceDsg( pkt_obj_ptr, len );
    }
}

//
// DSG_error_tag 0x9F8E08
// err_status Indicates the type of error that occurred
// 0x00 Byte count error  The Card did not receive the same number of bytes
//   in the DSG packet as was signaled by the Host.
// 0x01 Invalid_DSG_channel 
//   Advanced Mode: The Current DCD message transmitted to the
//   Card is not valid or does not contain the requested DSG tunnel(s).
//   The Host SHALL acquire a new DCD on a different downstream
//   and pass this DCD to the Card. Sent from the Card to the Host
//   during initial tunnel acquisition or when a DCD no longer contains
//   a required tunnel.
//   Basic Mode: The current DSG channel is not valid. The Host
//   SHALL find another DSG channel that contain DSG tunnels with
//   the well-known MAC address(es).
// 0x02 Application_ID_error  The current DCD message transmitted to the
//   Card does not contain a valid entry for an application ID requested by the
//   Host. The Host MAY choose to not to wait for data intended for the
//   specified application from that tunnel if Application_ID is invalid.
// 0x03-0xFF Reserved
//
void CableCardCallbackDSGPacketError( unsigned char err_status )
{
    BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();

    DsgPrint("\nCableCard Client received DSG_error_tag with error status=%d\n", err_status );

    if( err_status == 0x01 )    //Invalid_DSG_channel
    {
        // Tell Docsis about this Dsg Error
        DsgReportDsgError( err_status );

        // Tell Dsg Client Controleer to start Hunt for another DCD
        pDsgClientCtlThread->Start();
        
    }
}

//
// Cablecard request of open IP Unicast flow
//
unsigned long CableCardCallbackIPUDhcp( unsigned long flowid, unsigned char *mac, unsigned char opt_len, unsigned char *opt_data, unsigned char *ret_option, int *ropt_len )
{
#if ( BCM_DSG_DUAL_PROCESSOR_INTERFACE )

    unsigned long ret_ipaddr = 0;

    if( pgCableCardIf )
	{
		// In Dual processor model, the following function will send the dhcp param
		// to the eCM proxy dhcp client and will wait for the DHCP done. 
		// The DHCP parameter will arrive asyncronous via eCM notification and set CableCardIpaddress.
		ret_ipaddr = pgCableCardIf->HandleIpUnicastWithDhcpReq( flowid, mac, opt_len, opt_data, ret_option, ropt_len );

        #if (ENABLE_IPU_US_TEST)
    	// debug. start a periodic US test to exercise callback
        if( ret_ipaddr )
    	{
	    	int taskId = -1;
		    DsgPrint("\nStarting cablecard IP_U US test thread\n");
    		StartMyThread( "CableCard US TEST", &taskId, (void*(*)(void*)) CableCardIpUTestThread); 
	    }
        #endif

	}

	// return ip address to Pod Driver in network byte order
	return ret_ipaddr;


#else

	DsgPrint("\nCableCard IP_U flow request DHCP configure....\n");

    // In single process model, the following function will configure the DHCP option
	// into the proper ip stack and return. DHCP for this stack will automatically start
	// after Docsis ranged and registered!
    *ropt_len = DocsisCpeIpconfig( opt_len, mac, opt_data, ret_option ); 

    if( !DocsisRangedRegistered ) 
	{
		DsgPrint("\n------Starting Docsis Scan...\n");
		//DsgScan();
		BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();
		if(pDsgClientCtlThread)
		{
			// serialize the function call by sending kTestDsgRxDCD to our msg q.
			pDsgClientCtlThread->Start();
		}
	}

    //Wait until Docsis ranged and registered
    while( !DocsisRangedRegistered ) 
		BcmOperatingSystemFactory::ThreadSleep(2000);
	DsgPrint(".....DONE\n");

	// Wait until the Ip address changed....
	GetIpStackAddress(&CableCardIpAddress, IPU_STACK_NAME);
	while( CableCardIpAddress == preIpAddress )
	{
		BcmOperatingSystemFactory::ThreadSleep(3000);
		GetIpStackAddress(&CableCardIpAddress, IPU_STACK_NAME);
	}

	// Now start the DS data receive thread
	pgCableCardIf->StartDsRxThread();

#endif

}

//
// This function is called by Cablecard driver when it has IP Unicast packet to send US
// From the cablecard spec, the format of the pdata is in IP packet meaning 20-byte IP_header + payload
//
void CableCardCallbackSendIPUData( unsigned char *pdata, unsigned long len )
{
#if ( BCM_DSG_DUAL_PROCESSOR_INTERFACE )

	// pad to minimum length
	if( len < 64 )
		len = 64;

	// debug
    if (gTunnelDebugLevel & 0x8)
    	pgCableCardIf->DumpBytes( "US_IPU", pdata, len );

	if( DsgSendCableCardIpUsData( pdata, (unsigned short) len ) == kFuncFail)
        POD_Api_Lost_Flow_Ind( pgCableCardIf->GetIpUFlowId() , LOSTFLOW_NET_BUSY );

#else

    //Use socket to send out to Ip_U stack interface
	int rc = 0;
    struct sockaddr_in name;
	PIP_HDR pIpHdr = (PIP_HDR)pdata;

   	// Setup socket
    memset(&name, 0, sizeof(name));

    // extract the addressing information into the sockaddr struct
	// fish out the dest ip address
	name.sin_addr.s_addr = pIpHdr->ulDesIp;
    name.sin_family = AF_INET;

	// TBD: May have to snoop and consume ping reply packet here since bcm2 already reply it.

	// pad to minimum length
	if( len < 64 )
		len = 64;

	pgCableCardIf->DumpBytes( "US_IPU", pdata, len );
	rc = sendto(pgCableCardIf->IpUsSocket(), pdata, len, 0,  (struct sockaddr *)&name, sizeof(name));

	if ( rc < 0 )
	{
		DsgPrint("Socket sendto() error: %d \n", errno);
	}
	else
		DsgPrint("Socket sendto() OK return %d\n\n", rc);

#endif
}

//
// This function is called by Cablecard driver when the card is removed
//
void CableCardRemovedCallbackCleanUp( void )
{
	if( pgCableCardIf )
    	pgCableCardIf->HandleCardRemovedCleanUp();
}

//
// Cablecard request of open Socket Flow for IP traffic
//
unsigned char CableCardCallbackSocketFlowConfig( unsigned long flowid, unsigned char *pkt_obj_ptr, unsigned short len  )
{
    if( pgCableCardIf )
	{
        // debug
        pgCableCardIf->DumpBytes( "SocketFlowConfig Raw:", pkt_obj_ptr, len );
        return pgCableCardIf->HandleSocketFlowConfig( flowid, pkt_obj_ptr, len );
    }
    return 0x03;    // network unavailable or not responding
}

//
// This function is called by Cablecard driver to send US Ip packet
// using Socket Flow
//
void CableCardCallbackSendSocketFlowUsData( unsigned long flow_id, unsigned char *pdata, unsigned long len )
{
	// pad to minimum length
	if( len < 64 )
		len = 64;

    // Use socket to send out to thru eSTB ip stack
    if( pgCableCardIf->UdpTcpSocket() /*&& pgCableCardIf->DocsisRangedRegistered*/ )
    {
        // debug
        //if (gTunnelDebugLevel & 0x8)
    	pgCableCardIf->DumpBytes( "US_SocketFlow", pdata, len );
        if( pgCableCardIf->SocketFlowIpType() == SF_IPV6 )
        {
            gLogMessageRaw << "sendto() using US_socket6" << endl;
            if( sendto(pgCableCardIf->UdpTcpSocket(), pdata, len, 0,  (struct sockaddr *)&(pgCableCardIf->gUs_sock6addr), sizeof(pgCableCardIf->gUs_sock6addr)) < len )
            {
                gLogMessageRaw << "US_socket6 sendto() error: " << errno << endl;
                POD_Api_Lost_Flow_Ind( flow_id , LOSTFLOW_SOCKET_WR_ERR );    //socket write error
            }
        }
        else
        {
    	    if( sendto(pgCableCardIf->UdpTcpSocket(), pdata, len, 0,  (struct sockaddr *)&(pgCableCardIf->gUs_sockaddr), sizeof(pgCableCardIf->gUs_sockaddr)) < len )
            {
                gLogMessageRaw << "US_socket sendto() error: " << errno << endl;
                POD_Api_Lost_Flow_Ind( flow_id , LOSTFLOW_SOCKET_WR_ERR );    //socket write error
            }
        }

    }
    else
    {
        gLogMessageRaw << "Docsis not in 2-way mode or gUdpTcpSocket is closed, cannot send data!" << endl;
        POD_Api_Lost_Flow_Ind( flow_id , LOSTFLOW_NET_BUSY );    //Network down or busy
    }
}
//
//  This function is called by the Cablecard driver to delete a flow that 
//  has been opened
//
unsigned char CableCardCallbackDeleteFlowReq(unsigned long flowid)
{
    return pgCableCardIf->HandleDeleteFlowRequest( flowid );
}

//
//  new for I0-8 spec: 
//
void CableCardCallbackDSG_Directory( unsigned char *pkt_obj_ptr, unsigned short len ) 
{
    if( pgCableCardIf )
	{
        pgCableCardIf->HandleConfigureDsgDirectory( pkt_obj_ptr, len );
    }
}


//**********Export local procedures to EstbRemoteInterfaceStub.c without name-mangling ****************
//
//  Proxy DHCP reply from eCM
//
void BcmEstbIndicateCableCardProxyDhcpResponse( unsigned long ccIpAddr, char * pDhcpResponse, unsigned char optionlen )
{
	// ccIpAddr comes in host byte order
	pgCableCardIf->SetCableCardIpAddress( ccIpAddr );

	// copy the option byte over
    if( optionlen )
        pgCableCardIf->SetCableCardDhcpReplyOption( pDhcpResponse, optionlen );

}

//
// DS IPU data pkt from eCM proxy Ip stack
//
void BcmSendDsDataToCableCard( int data_len, unsigned char *pDataStart )
{
    // Forward this to cablecard
	pgCableCardIf->SendDsDataToCableCard( pDataStart, data_len );
}

//
// eCM reset detection from KeepAlive() when eCM is reset or received docDevResetNow SNMP command
//
void BcmIndicateEcmReset( void )
{
    if( pgCableCardIf )
        pgCableCardIf->sendEcmResetToCableCard();
}


#if (ENABLE_IPU_US_TEST)
// **************************** DEBUG:: IP_U US test :: DEBUG **************************************
//
// Current cablecard does not send out any unicast packet Upstream
// For test and debug purposes, simulate a thread to send ping packet Upsteam
//

// Calculate the 16 bit checksum over the buffer passed in for
// the number of BYTES passed in as a length.  Code will actually
// compute the checksum over (Length/2) USHORTS, but because we are
// little endian we'll take the buffer in as UCHAR's and convert.
// Also, this is necessary for odd Length...
USHORT CalculateChecksum (PUCHAR pucData, int Length)
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

//
// Fixed Ping routine
//
void Ping(void)
{
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
	  IP_HDR		  IpHdr;  
	  ICMP_HDR        IcmpHdr;
	  UCHAR           pucEchoData[64]; // maximum for our purposes. Really this can be as big as you like.
	} ICMP_ECHO_PKT, *PICMP_ECHO_PKT;

    static ICMP_ECHO_PKT PingPkt;
    int PktSize;
    USHORT usChecksum;
    UCHAR pucPingData[] =
	  // Data from captured ping packet, above.
	  {
		0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
		0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,
		0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x61,
		0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
		0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71,
		0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x61, 0x62,
		0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x70
	  };

    if( pgCableCardIf->GetCableCardIpAddress() == 0 )
		return;

    // Fill in the IP header fields.
	PingPkt.IpHdr.ucVerLen = 0x45;
	PingPkt.IpHdr.ucToS = 0x0;
	PingPkt.IpHdr.usLength = htons(sizeof (IP_HDR) + sizeof (ICMP_HDR)+ sizeof (pucPingData));
	PingPkt.IpHdr.usId = htons(0xbeef);
	PingPkt.IpHdr.usFFrag = htons(0x0000);
	PingPkt.IpHdr.usTtlProto = htons(((64)<<8) + 0x01);	//ICMP
	PingPkt.IpHdr.usHCS = 0;
	PingPkt.IpHdr.ulSrcIp = htonl(pgCableCardIf->GetCableCardIpAddress());		//in host byte order
	// harcode target addresss here x.x.x.1
	PingPkt.IpHdr.ulDesIp = htonl((pgCableCardIf->GetCableCardIpAddress()&0xffffff00)+1);	//11.24.71.1
	// Generate IP HCS.
	usChecksum = htons( CalculateChecksum( (PUCHAR) &PingPkt.IpHdr, sizeof (IP_HDR) ) );
	PingPkt.IpHdr.usHCS = usChecksum;

	// Fill in the ICMP header
	PingPkt.IcmpHdr.ucType = 0x08; // ECHO=8
	PingPkt.IcmpHdr.ucCode = 0x00;
	PingPkt.IcmpHdr.usChecksum = 0x0000; // initially for cksm calculation
	PingPkt.IcmpHdr.usId = htons (0x0100); //(0xDADA); // arbitrary ID
	PingPkt.IcmpHdr.usSequence = htons (0x0C00); //(usSequence); // just count

	memcpy (PingPkt.pucEchoData, pucPingData, sizeof (pucPingData));

	usChecksum = htons (CalculateChecksum ((PUCHAR) &PingPkt.IcmpHdr, sizeof (ICMP_HDR) + sizeof (pucPingData)));
	PingPkt.IcmpHdr.usChecksum = usChecksum;

	PktSize = sizeof (IP_HDR) + sizeof (ICMP_HDR)+ sizeof (pucPingData);

	DsgPrint("Ping %08lx: Sending US ping packet using cablecard IP_U flow\n", ntohl(PingPkt.IpHdr.ulDesIp) );
	CableCardCallbackSendIPUData((unsigned char *)&PingPkt, PktSize);

}

//
// Fixed Ping routine
//
void UdpSend(void)
{
	typedef struct _UDP_HDR
	{
	  USHORT srcport;
	  USHORT dstport;
	  USHORT length;
      USHORT chksum;
	} UDP_HDR, *PUDP_HDR;

	typedef struct _UDP_PKT
	{
	  IP_HDR		  IpHdr;  
	  UDP_HDR         UdpHdr;
	  UCHAR           pucUdpData[0x80]; // maximum for our purposes. Really this can be as big as you like.
	} UDP_PKT, *PUDP_PKT;

    static UDP_PKT UdpPkt;
    int PktSize, i;
    USHORT usChecksum;

    if( pgCableCardIf->GetCableCardIpAddress() == 0 )
		return;

    // Fill in the IP header fields.
	UdpPkt.IpHdr.ucVerLen = 0x45;
	UdpPkt.IpHdr.ucToS = 0x0;
	UdpPkt.IpHdr.usLength = htons(sizeof (IP_HDR) + sizeof (UDP_HDR)+ sizeof (UdpPkt.pucUdpData));
	UdpPkt.IpHdr.usId = htons(0xcafe);
	UdpPkt.IpHdr.usFFrag = htons(0x0000);
	UdpPkt.IpHdr.usTtlProto = htons(((64)<<8) + 0x11);	//UDP
	UdpPkt.IpHdr.usHCS = 0;
	UdpPkt.IpHdr.ulSrcIp = htonl(pgCableCardIf->GetCableCardIpAddress());		//in host byte order
	// harcode target addresss here x.x.x.1
	UdpPkt.IpHdr.ulDesIp = htonl((pgCableCardIf->GetCableCardIpAddress()&0xffff0000)+0x040b);	//11.24.4.11
	// Generate IP HCS.
	usChecksum = htons( CalculateChecksum( (PUCHAR) &UdpPkt.IpHdr, sizeof (IP_HDR) ) );
	UdpPkt.IpHdr.usHCS = usChecksum;

	// Fill in the UDP header
	UdpPkt.UdpHdr.srcport = htons(0xe000);
	UdpPkt.UdpHdr.dstport = htons(0x193d);
    UdpPkt.UdpHdr.length = htons(sizeof (UDP_HDR)+ sizeof (UdpPkt.pucUdpData));
	UdpPkt.UdpHdr.chksum = 0x0000; // initially for cksm calculation

	//memcpy (UdpPkt.pucEchoData, pucPingData, sizeof (pucPingData));
    // increment pattern
    for (i=0; i<sizeof(UdpPkt.pucUdpData); i++)
        UdpPkt.pucUdpData[i] = i;

	usChecksum = htons (CalculateChecksum ((PUCHAR) &UdpPkt.UdpHdr, sizeof (UDP_HDR) + sizeof (UdpPkt.pucUdpData)));
	UdpPkt.UdpHdr.chksum = usChecksum;

	PktSize = sizeof (IP_HDR) + sizeof (UDP_HDR)+ sizeof (UdpPkt.pucUdpData);

	DsgPrint("UdpSend %08lx: Sending US Udp packet using cablecard IP_U flow\n", ntohl(UdpPkt.IpHdr.ulDesIp) );
	CableCardCallbackSendIPUData((unsigned char *)&UdpPkt, PktSize);

}

//
// Test thread
//
void CableCardIpUTestThread ( void )
{
    static int toggle = 0;
	DsgPrint("CableCardIpUTestThread starting...\n");

	while( 1 )
	{
		//pkt_count = 0;
		BcmOperatingSystemFactory::ThreadSleep(5000);
		DsgPrint("DsgFlowId Stat: RX %d pkt\n", pkt_count );
		DsgPrint("IpuFlowId Stat: RX %d pkt\n", ipu_count );
		// ICMP test or UDP test
        if( (toggle++) % 2 )
    		Ping();
        else
		    UdpSend();

		//add ttcp test later
	}

}
#endif

#if (ENABLE_SOCKET_US_TEST)
void CableCardSocketFlowUsTestThread ( void )
{
    UCHAR pucData[80], i;

    // incremental pattern
    for (i=0; i<80; i++)
        pucData[i] = i;

	DsgPrint("CableCardSocketFlowUsTestThread starting...\n");

	while( 1 )
	{
		BcmOperatingSystemFactory::ThreadSleep(5000);
		// UDP test
        DsgPrint("Sending US Udp packet using cablecard socket flow\n" );
        CableCardCallbackSendSocketFlowUsData( 0, pucData, sizeof (pucData) );
	}

}
#endif

#if !(DSGCC_BUILT_AS_LIB)
//
// For standalone dsgcc build with cablecard enabled, Stub out these functions
// 
unsigned char BcmSendDSGTunnelDataToHost( unsigned char *pBufData, unsigned int pktlen, unsigned long client_type, unsigned long client_id)
{
    DsgPrint("-->In Stub BcmSendDSGTunnelDataToHost: Just return 0!" );
    return 0;
}

unsigned char BcmSendDSGTunnelDataToPOD( unsigned char *pBufData, unsigned int pktlen, unsigned long flow_id )
{
    DsgPrint("-->In Stub BcmSendDSGTunnelDataToPOD: Just return 0!" );
    return 0;
}

unsigned char BcmSendDataToPOD( unsigned char *pBufData, unsigned int pktlen, unsigned long flow_id )
{
    DsgPrint("-->In Stub BcmSendDataToPOD: Just return 0!" );
    return 0;
}

void POD_Api_Send_DCD_Info(void *dcd_ptr, unsigned short dcd_len)
{
    DsgPrint("-->In Stub POD_Api_Send_DCD_Info: Just return!" );
    return;
}

void POD_Api_Send_DSG_Message(void *dsg_message_ptr, unsigned short dsg_message_len)
{
    DsgPrint("-->In Stub POD_Api_Send_DSG_Message: Just return!" );
    return;
}

void POD_Api_Lost_Flow_Ind(unsigned long id,unsigned char status)
{
    DsgPrint("-->In Stub POD_Api_Lost_Flow_Ind: Just return!" );
    return;
}
#endif



