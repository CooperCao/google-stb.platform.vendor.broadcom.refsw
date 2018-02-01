/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>

#include "dsg_api.h"
#include "EstbRemoteInterfaceStub.h"

// Packet tagging
#if (kDefaultValue_TaggingEnabled)
#include "if_net.h"
#endif

#if (BFC_INCLUDE_DAVIC_API_SUPPORT)
#include "CmDavicApiSocketInterface.h"
#endif

#if defined (COMMON_DOWNLOAD_SUPPORT)
#include "EstbCommonDownloadInterface.h"
#endif

#if NPTL_SUPPORT
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP \
	{ {0, 0, 0, PTHREAD_MUTEX_RECURSIVE_NP,0} }
#else
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP \
	{0, 0, 0, PTHREAD_MUTEX_RECURSIVE_NP, __LOCK_INITIALIZER}
#endif

static pthread_mutex_t LnxDsgApiMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

#define eCmResponseTimeout  2			// Timeout value for eCM response
#define DSGAPI_CHANNELLIST_HTON_SUPPORT 1
//****************** Extern/Local Functions prototypes ************************
#if (CABLECARD_SUPPORT)
// temp here. Bypassing ClientController
extern void BcmEstbIndicateCableCardProxyDhcpResponse( unsigned long ccIpAddr, char * pDhcpResponse, unsigned char optionlen );
extern void BcmSendDsDataToCableCard( int data_len, unsigned char *pDataStart );
extern void BcmIndicateEcmReset( void );
#endif

#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
extern void ResetKeepAliveTimer(uint32 timeoutms);
#endif

uint32 HostToNetworkUint32(uint32 value);
uint16 HostToNetworkUint16(uint16 value);
//********************** Global Variables *************************************
//********************** Local Variables *************************************
static unsigned char TransId = 0;
static bool socketInitComplete = false;
static int sock_ctl_out = 0;
static int sock_ctl_in = 0;
char EstbPrimaryIpStackName[IPC_INTERFACE_NAME_MAX_LEN] = kDefaultEcmInterfaceName;
char ecmInterfaceName[IPC_INTERFACE_NAME_MAX_LEN];

static uint32 oobMode = kOobModeDsg;
uint32 displayMode = kLevel0;

static bool ecmHasOlderApiVers = false;
static uint32 ecmApiVersion = 0;

static bool taggingEnabled = kDefaultValue_TaggingEnabled;

//***************************************************************************
//	void  LogRawBuffer
//
// Debug Hex dump
//
//	returns  length of print
//
//***************************************************************************
void LogRawBuffer (char * str, uint8 *pBuffer, int BufLen, BOOL wacky)
{
   int Loop;
   char DataLine[96];
   char DataValue[16];

   printf("%s\nLogging %d bytes:\r\n", str, BufLen);

   DataLine[0] = 0;
   for (Loop = 0; Loop < BufLen; Loop++)
   {
      sprintf (DataValue, "%02x ", pBuffer[Loop]);
      strcat (DataLine, DataValue);

      if ((Loop + 1) % 16 == 0)
      {
         strcat (DataLine, "\r\n");
         printf("%s\n", DataLine);
         DataLine[0] = 0;
      }

      else if ((Loop + 1) % 4 == 0)
         strcat (DataLine, "  ");
   }

   strcat (DataLine, "\r\n");
   printf("%s\n", DataLine);
}

static int EstbToEcmSocketSendRx( int tpkt_size, unsigned char *pBuf, bool waitreply, bool readBuffer )
{
	struct sockaddr_in sin_dst;
    register int cnt;
	unsigned int rxBufferSize = 0;

	if( (tpkt_size < kEcmReplyPktSize) || (tpkt_size > kDefault_eStbSocketMaxRxBufferSize)  )
	{
		printf("\nEstbToEcmSocketSendRx() Error: buffer size %d is not within range (%d - %d)\n", tpkt_size, kEcmReplyPktSize, kDefault_eStbSocketMaxRxBufferSize );
		return kFuncFail;
	}

	bzero((char*)&sin_dst, sizeof(sin_dst));
    sin_dst.sin_family = AF_INET;
    sin_dst.sin_port = htons(ECM_DSG_REQUEST_PORT);
	sin_dst.sin_addr.s_addr = htonl(kRemoteEcmIpAddress);

    FlushRxSocketBuffer();

	// Use CONTROL TX Socket
    cnt=sendto(sock_ctl_out, pBuf, tpkt_size, 0, (struct sockaddr*)&sin_dst, sizeof(sin_dst));
	if( cnt != tpkt_size )
	{
		printf("\nError: sendto returns %d  - %s\n", cnt, strerror(errno));
		return kFuncFail;
	}

    if( waitreply == false )
        return kFuncSuccess;

    if(displayMode & kLevel4)
	LogRawBuffer ("EstbToEcmSocketSendRx: SENT: ", pBuf, tpkt_size, 0);

    if( readBuffer )
    {
	rxBufferSize = tpkt_size;
    }
    else
    {
	rxBufferSize = kEcmReplyPktSize;
    }

    // wait for reply
    {
        struct sockaddr_in frominet;
        socklen_t inetsize;
        int retcode;
        fd_set rfds;
        struct timeval tv;
        unsigned char xid = pBuf[0];    // remember transaction ID
        unsigned char api = pBuf[1];    // debug
        unsigned int maxNumRxSystemCalls = 3;

        #if (CABLECARD_SUPPORT)
        maxNumRxSystemCalls = 32;
        #endif

        bzero((char*)&frominet, sizeof(frominet));
        bzero((char*)pBuf, tpkt_size);  // reuse TX buffer for RX buffer.

        while(1)
        {
            FD_ZERO(&rfds);
            FD_SET(sock_ctl_in, &rfds);

            // The linux call to select will modify the timeout
            // value to reflect the time remaining.  Therefore,
            // we have to re-initialize before repeated calls to select.
            tv.tv_sec = eCmResponseTimeout;
            tv.tv_usec = 0;

            retcode = select((sock_ctl_in + 1), &rfds, (fd_set *)0, (fd_set *)0, &tv);
            if (retcode > 0)
            {
                // Listen for NOTIFICATIONS
                if(FD_ISSET(sock_ctl_in, &rfds))
                {
                    // Configure CONTROL RX socket to receive packets from ECM.
                    inetsize = sizeof(frominet);
                    cnt = recvfrom(sock_ctl_in, (char*)pBuf, rxBufferSize, 0, (struct sockaddr*)&frominet, &inetsize);
                    if(  cnt > 0  )
                    {
                        if( cnt != rxBufferSize  )
                            printf("Warning! Rxlen %d != expected len %d\n", cnt, rxBufferSize );

                        if( pBuf[0] != xid )
                            printf("Error! RxXid %d != expectedXid %d\n", pBuf[0], xid );
                        else
                        {
							if(displayMode & kLevel4)
								LogRawBuffer ("EstbToEcmSocketSendRx: RCVD: ", pBuf, cnt, 0);

                            return ((int)pBuf[2]);   // Normal return
                        }
                    }
                    else
                        printf("Error! recvfrom return %d. (errno:%d - %s)\n", cnt, errno, strerror(errno) );
                }
                else
                    printf("Error! Select() return but FD_ISSET is not set\n" );
            }
            else if( retcode == 0 ) // eCmResponseTimeout timeout
            {
                printf("Timeout(%d sec) Waiting for eCmResponse! (errno:%d - %s)\n", eCmResponseTimeout, errno, strerror(errno));
            }
            else    // <0, interrupted signal
            {
                #if defined(TARGETOS_Linux)
                // Each serialize call into client control thread will cause 1 EINTR here,
                // this is not an error from eCM, continue wait for ecm response
                // Pick a number that is more that the max consecutive serialize calls anticipated
                if ( (errno == EINTR) && --maxNumRxSystemCalls )
                {
                    printf("!!!%s\n", strerror(errno));
                    continue;
                }
                if( maxNumRxSystemCalls == 0 )
                {
                    printf("Error(%s):RECEIVED too many SystemCalls during wait_for_ecm_response! Re-send commands\n", strerror(errno));
                    break;
                }
                else
                #endif
                    printf("Error! Select() return negative number! (errno:%d - %s)\n", errno, strerror(errno) );
            }
            // An error has occured when drop down to here
            printf("Xid %d - api %d error\n", xid, api);
            break;
        }   // while loop
    }

    // If arrive here, an error has happened above
    return kFuncFail;

}
#if 0
/*
 *  Socket call to send a message
 *
 *  rpkt_size:  the packet length
 *
 *	pBuf:	pointer to the packet
 *
 *	bIsCtl: 1 to send a control packet, 0 for data.
 *
 */
STATUS tudp_tx( int tpkt_size, unsigned char *pBuf, unsigned short port)
{
	struct sockaddr_in sin_dst;
    register int cnt;

	bzero((char*)&sin_dst, sizeof(sin_dst));
    sin_dst.sin_family = AF_INET;
//    sin_dst.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sin_dst.sin_port = htons(port);
    //inet_pton(AF_INET, "127.0.0.1", &sin_dst.sin_addr);

	// eSTB RI will always communicate to Remote IP Stack on eCM.
	sin_dst.sin_addr.s_addr = htonl(kRemoteEcmIpAddress);

	/*printf("\n: Sending config pkt port %04x tpkt_size=%d socket = %d\n", port, tpkt_size, sock_ctl_out);
	{
		uint32 i = 0;
		for( i=0; i<16 ; i++)
		{
			printf (" %02x ", pBuf[i]);
		}
	}*/

    FlushRxSocketBuffer();

	// Use CONTROL TX Socket
    cnt=sendto(sock_ctl_out, pBuf, tpkt_size, 0, (struct sockaddr*)&sin_dst, sizeof(sin_dst));
	if( cnt != tpkt_size )
	{
		printf("\nError: sendto returns %d  - %s\n", cnt, strerror(errno));
		return kFuncFail;
	}
	/*else
	{
		printf("\nTransmission is complete.\n");
	}*/

	return(kFuncSuccess);
}

/*
 *  Socket call to receive a message
 *
 *  rpkt_size:  expected minimum packet length
 *
 *	pBuf:	pointer to the control request packet
 *
 *	bIsCtl: 1 to receive a control packet, 0 for data.
 *
 */
/*
 *  Socket call to receive a DSG confirm message from DSG CM
 */
STATUS wait_for_ecm_response( int rpkt_size, unsigned char *pBuf)
{
    register int cnt;
	struct sockaddr_in frominet;
	socklen_t       inetsize;
	unsigned char      *buf;
	int retcode;

	fd_set rfds;
	struct timeval tv;
	int		buflen  = rpkt_size;   /* length of buffer */

	unsigned int maxNumRxSystemCalls = 3;


	#if (CABLECARD_SUPPORT)
	maxNumRxSystemCalls = 32;
	#endif

	tv.tv_sec = eCmResponseTimeout;
	tv.tv_usec = 0;

	bzero((char*)&frominet, sizeof(frominet));

    if( (buf = (unsigned char *)malloc(buflen)) == NULL)
	{
		printf("wait_for_ecm_response: malloc failure!\n");
		return kFuncFail;
	}

	while(1)
	{
		FD_ZERO(&rfds);
		FD_SET(sock_ctl_in, &rfds);

        // The linux call to select will modify the timeout
        // value to reflect the time remaining.  Therefore,
        // we have to re-initialize before repeated calls to select.
		tv.tv_sec = eCmResponseTimeout;
		tv.tv_usec = 0;

		retcode = select((sock_ctl_in + 1), &rfds, (fd_set *)0, (fd_set *)0, &tv);
		if (retcode > 0)
		{
			// Listen for NOTIFICATIONS
			if(FD_ISSET(sock_ctl_in, &rfds))
			{
				// Configure NOTIFICATION and DATA sockets to receive packets with eCM address.
				inetsize = sizeof(frominet);

				// recvfrom shoud not block now
				// Use CONTROL RX Socket
				cnt = recvfrom(sock_ctl_in, (char*)buf, buflen, 0, (struct sockaddr*)&frominet, &inetsize);

				// Configure CONTROL RX socket to receive packets with eCM address.
				if(  cnt > 0  )
				{
					//printf("Rx response pkt from [ %s ]: UDP port %04x received packet with len %d\n", inet_ntoa(frominet.sin_addr),  ESTB_DSG_RESPONSE_PORT, cnt);
					//if( frominet.sin_addr.s_addr == htonl(kRemoteEcmSrcIpAddress ) )
					{
						//printf(" Transaction Id(%02x) \n", buf[0] );
						if( cnt == buflen  )
						{
							if( buf[0] == pBuf[0] )
							{
								//printf(" Transaction Id(%02x) matched for TX/RX. cnt=%d\n", buf[0], cnt );

								//printf(" Return result is kSuccess(%02x)\n", buf[2] );
								memcpy(pBuf, buf, cnt);
							}
							else
							    printf(" Rx Transaction Id(0x%02x)!= 0x%02x. API %d\n", buf[0], pBuf[0], pBuf[1] );
						}
						else
						    printf(" Rx len %d != expected len %d\n", cnt, buflen );
					}
					//else
					//{
					//	printf("Rx response pkt from [ %s ]: with len %d\n", inet_ntoa(frominet.sin_addr), cnt);
					//	printf(" Dropping packet\n");
					//}
					// Break out of the while loop.
					break;
				}
				else
				{
					printf("wait_for_ecm_response - recvfrom: %s\n", strerror(errno));
					free(buf);
					return kFuncFail;
				}
			}
		}
		else if( retcode == 0 ) // eCmResponseTimeout timeout
		{
			printf("Timeout Waiting for eCmResponse! (errno:%d - %s)\n", errno, strerror(errno));
			free(buf);
			return kFuncFail;
		}
		else    // <0, interrupted signal
		{
			#if defined(TARGETOS_Linux)
			// Each serialize call into client control thread will cause 1 EINTR here,
			// this is not an error from eCM, continue wait for ecm response
			// Pick a number that is more that the max consecutive serialize calls anticipated
			//printf("!!!%s\n", strerror(errno));
			if ( (errno == EINTR) && maxNumRxSystemCalls )
			{
				if( --maxNumRxSystemCalls == 0 )
				{
					printf("kFuncFail(%s):RECEIVED too many SystemCalls during wait_for_ecm_response! Try increase maxNumRxSystemCalls in wait_for_ecm_response() in EstbRemoteInterfaceStub.c.\n"
						   "Each serialize call into the thread this function is called from will cause 1 EINTR here,\n"
						   "this is not an error from eCM, continue wait for ecm response.\n"
						   "Pick a number that is more that the max consecutive serialize calls anticipated\n", strerror(errno));
					free(buf);
					return kFuncFail;
				}
				continue;
			}
			else
			#endif
			{
				free(buf);
				return kFuncFail;
			}
		}
	}

    free(buf);
	return(kFuncSuccess);
}
#endif
/*****************************************************************************************
  "C" type DSG API functions published for Dsg Client(eSTB) call into Dsg Host(eCM)
  For more detail, pleae refer to Brcm document "Boradcom DSG API.doc"
  prototype in dsg_api.h
******************************************************************************************/
/*
 *  Set DSG operation mode to eCM
 *  Set the eCM's DSG mode according to mode:
 *  0 == DSG disabled
 *  1 == DSG Basic Mode
 *  2 == DSG Advanced Mode
 *
 *  This function returns:
 *  kFuncSuccess = success; DSG mode set
 *  kFuncInvalid = mode out of range
 *  kFuncFail = function failure
 */
int DsgSetDSGMode( int mode )
{
	unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

	// Set OOB mode to DSG. Please note that a DsgStop function does not exist.
	// DSG does not set oob mode to kOobModeNone.
	SetOobMode(kOobModeDsg);

    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

    // Generate the message.
#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgSetMode;
    DsgConfig[2] = mode;

    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send set DSG mode command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;
}

/*
 * Set the eCM's DOCSIS mode according to enable;
 * true - allows bidirectional DOCSIS operation
 * false - disables DOCSIS operation.
 *
 * This function returns:
 * true == success; DOCSIS mode set
 * false == function failure
 *
 */
bool DsgEnableDOCSIS (bool enable)
{
	unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

    pthread_mutex_lock(&LnxDsgApiMutex);
#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

    // Generate the message.
#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgEnableDocsis;
    DsgConfig[2] = enable;

    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

}


/*
 *  Set Advanced mode DSG Info to eCM
 *
 * This function returns:
 * true == success; Advanced tunnel set
 * false == function failure
 *
 */
int  DsgOpenTunnel( DsgTunnelSettings *pTunnelSettings )
{
	unsigned char *DsgConfig;
	unsigned int bufferLen;
	int ret_val;
	uint32 tempVal;
	uint8 sizeOfClassifierSets = 0;

#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

    pthread_mutex_lock(&LnxDsgApiMutex);

	bufferLen = 26 + (pTunnelSettings->numOfClassifiers * sizeof(t_dsg_classifier));

    // Pad out the buffer to minimum expected reply size.
    if (bufferLen < kEcmReplyPktSize)
        bufferLen = kEcmReplyPktSize;

	//Allocate buffer to receive NOTIFICAION.
    if( (DsgConfig = (unsigned char *)malloc(bufferLen)) == NULL)
	{
		printf("Can not allocate buffer %d\n", bufferLen);
        pthread_mutex_unlock(&LnxDsgApiMutex);
		return kFuncFail;
	}
#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, bufferLen);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgOpenTunnel;

	memcpy( &DsgConfig[2], pTunnelSettings->pTunnelAddress, sizeof(t_mac_adr) );

	// Client ID Type
	tempVal = HostToNetworkUint32(pTunnelSettings->clientIdType);
	memcpy( &DsgConfig[8], &tempVal, sizeof(uint32) );

	// Client ID Value
	tempVal = HostToNetworkUint32(pTunnelSettings->clientIdValue);
	memcpy( &DsgConfig[12], &tempVal, sizeof(uint32) );

	// Number Of Classifiers
	tempVal = HostToNetworkUint32(pTunnelSettings->numOfClassifiers);
	memcpy( &DsgConfig[16], &tempVal, sizeof(uint32) );

	// Classifier Settings
	memcpy( &DsgConfig[20], pTunnelSettings->pClassifierList, pTunnelSettings->numOfClassifiers * sizeof(t_dsg_classifier) );

	// Client ID Value for MAC_address type client
	sizeOfClassifierSets = pTunnelSettings->numOfClassifiers * sizeof(t_dsg_classifier);
	if(pTunnelSettings->pMacAddressClientIdVal)
		memcpy( &DsgConfig[20+sizeOfClassifierSets], pTunnelSettings->pMacAddressClientIdVal, sizeof(t_mac_adr) );

    ret_val = EstbToEcmSocketSendRx( bufferLen, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send open tunnel command again!\n");
        iteration--;
    }
    }
#endif
	free(DsgConfig);
    pthread_mutex_unlock(&LnxDsgApiMutex);

	return ret_val;

}

/*
 *
 * This function tells the eCM to close any and all active Advanced Mode tunnels.
 * The eCM will stop forwarding any DSG traffic to the eSTB.
 *
 * This function may be useful if the DCD changes to contain different tunnel identification parameters.
 *
 * This function returns:
 * true == success; Advanced tunnels closed
 * false == function failure
 */
bool DsgCloseTunnel( int tunnel_num )
{
	unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif
    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgCloseTunnel;
	DsgConfig[2] = tunnel_num;

    // Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

}
////////////////////////////////////////////////////////////////////////
//
// Timers
//
////////////////////////////////////////////////////////////////////////
/*
 *  Set Tdsg1 to Tdsg4 timeout other than the default.
 *
 *  timer_id is a value from 1-4. timeout is in unit of seconds
 *
 *  This function returns:
 *  kFuncSuccess = success
 *  kFuncInvalid = out of range
 *  kFuncFail = function failure
 */
int DsgSetTimer( uint8 timer_id, uint16 timeout )
{
	unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif
    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

    //Test the next configuration: Timer: Tdsg1
    //Use a shorter time-out for testing
#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgSetTimer;
    DsgConfig[2] = timer_id;
    DsgConfig[3] = (unsigned char)(timeout >> 8);    //uint16 of timeout unit in 10ms
    DsgConfig[4] = (unsigned char)timeout;

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

}

/*
 *  Get current tdsg1 to Tdsg4 timeout from V2 manager
 *
 *  timer_id is a value from 1-4. timeout is in unit of seconds
 *
 *  This function returns:
 *  kFuncSuccess = success
 *  kFuncInvalid = out of range
 *  kFuncFail = function failure
 */
int DsgGetTimer( uint8 timer_id, uint16 *timeout )
{
	unsigned char DsgConfig[64];
	uint16 timeoutValue;
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif
    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

    //Test the next configuration: Timer: Tdsg1
    //Use a shorter time-out for testing
#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgGetTimer;
	DsgConfig[2] = timer_id;

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);
    if( ret_val != kFuncFail )
    {
        // Retrieve timeout value from the message.
        timeoutValue = DsgConfig[3] << 8;
        timeoutValue |= DsgConfig[4];
        memcpy(timeout, &timeoutValue, sizeof(*timeout));
    }
#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

}

/*
 *
 * This function directs the eCM to employ the indicated channel list when it scans for DSG
 * channels. Note that this list is not used when DSG is disabled and the eCM scans for DOCSIS
 * channels.
 *
 * The argument num_chans indicates the number of channels in the list. The argument channel_list
 * is an array of channel frequencies. Each value is the center frequency of a channel, in units
 * of 62500 Hz.
 *
 * This function returns:
 * true == success; channel list set
 * false == function failure
 */
#if (DSGAPI_CHANNELLIST_HTON_SUPPORT)
bool DsgSetChannelList (uint16 num_chans, uint32 * channel_list)
{
    unsigned char *DsgConfig;
	unsigned int bufferLen = num_chans * sizeof(uint32) + 4 ;    // can have up to 255 freq entry + plus 4 bytes for DSG command, transaction id etc.
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

	uint16 numChansHtonl  = num_chans;
	HtonDsgSetChannelList(&numChansHtonl , channel_list);

    pthread_mutex_lock(&LnxDsgApiMutex);

    // Pad out the buffer to minimum expected reply size.
    if (bufferLen < kEcmReplyPktSize)
        bufferLen = kEcmReplyPktSize;

    // Allocate buffer to hold channel list
    if( (DsgConfig = (unsigned char *)malloc(bufferLen)) == NULL)
    {
        printf("Can not allocate buffer %d\n", bufferLen);
        pthread_mutex_unlock(&LnxDsgApiMutex);
        return kFuncFail;
    }

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, bufferLen);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgSetChannelsList;

	memcpy( &DsgConfig[2], (uint8*) &numChansHtonl , sizeof(uint16) );

    // Number of channels to set.
    memcpy( &DsgConfig[4], (uint8*) channel_list, num_chans * sizeof(uint32) );

    // Send the message
    ret_val = EstbToEcmSocketSendRx( bufferLen, DsgConfig, true, false);
#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send open tunnel command again!\n");
        iteration--;
    }
    }
#endif
    free(DsgConfig);
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;
}

void HtonDsgSetChannelList (uint16 *pNum_chans, uint32 *pChannel_list)
{
	// First htonl channel list.
	uint8 i;
	for( i=0; i<*pNum_chans; i++ )
	{
		*pChannel_list = htonl(*pChannel_list);
		pChannel_list++;
	}

	// number of channels
	*pNum_chans = htons(*pNum_chans);
}

#else
bool DsgSetChannelList (uint16 num_chans, uint32 * channel_list)
{
    unsigned char *DsgConfig;
	unsigned int bufferLen = num_chans * sizeof(uint32) + 4 ;    // can have up to 255 freq entry + plus 4 bytes for DSG command, transaction id etc.
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif
    pthread_mutex_lock(&LnxDsgApiMutex);

    // Pad out the buffer to minimum expected reply size.
    if (bufferLen < kEcmReplyPktSize)
        bufferLen = kEcmReplyPktSize;

    // Allocate buffer to hold channel list
    if( (DsgConfig = (unsigned char *)malloc(bufferLen)) == NULL)
    {
        printf("Can not allocate buffer %d\n", bufferLen);
        pthread_mutex_unlock(&LnxDsgApiMutex);
        return kFuncFail;
    }

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, bufferLen);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgSetChannelsList;

    // Number of channels to set.
    DsgConfig[2] = (unsigned char)(num_chans >> 8);
    DsgConfig[3] = (unsigned char)num_chans;

    memcpy( &DsgConfig[4], channel_list, num_chans * sizeof(uint32) );

    // Send the message
    ret_val = EstbToEcmSocketSendRx( bufferLen, DsgConfig, true, false);
#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send open tunnel command again!\n");
        iteration--;
    }
    }
#endif
    free(DsgConfig);
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;
}
#endif


/*
 *
 * This function directs the eCM to disregard any previous channel list and search all
 * frequencies when scanning for DSG tunnels.
 *
 * This function returns:
 * true == success; channel list reset
 * false == function failure
 *
 */
bool DsgResetChannelList ( void )
{
	unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif
    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgResetChannelsList;

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

}

/*
 *
 * This function directs the eCM to continue scanning downstreams in search of DSG information.
 *
 * This function returns:
 * true == success; scan started
 * false == function failure
 *
 */
bool DsgScan ( bool enable )
{
	unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif
    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgScan;
	DsgConfig[2] = enable;

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

}

/*
	The eSTB calls this function to learn the eCM's status
	// with regard to DSG operation. The possible return values are:

	0 =  function failure
	1 = scanning for DSG info
	2 = DSG info present
	3 = DSG info lost, trying to recover
	4 = DSG not started
*/
int DsgStatus( void )
{
	unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif
    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgStatus;

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

}

/*
 * Allow the Dsg Client to query the DOCSIS Ready state
 *
 *  This function returns:
 *      enum DsgDocsisStatus values per RFC2670
 *      kFuncFail = function failure
 *
	// These are the "CM initialization state values" defined by the
	// CM status MIB.  They are based on the "CM Initialization Overview"
	// flowchart in the DOCSIS RFI spec (see Figure 9-1 in RFIv1.1-I03-991105).
	// KO: changed the state names to match docsIfCmStatusValue names in
	// RFC-2670.  Left the old names as comments.
	typedef enum
	{
		kOther = 1,
		kNotReady = 2,
		kNotSynchronized = 3,
		kPhySynchronized = 4,
		kUsParametersAcquired = 5,
		kRangingComplete = 6,
		kIpComplete = 7,
		kTodEstablished = 8,
		kSecurityEstablished = 9,
		kParamTransferComplete = 10,
		kRegistrationComplete = 11,
		kOperational = 12,
		kAccessDenied = 13
	} CmStatusValue;
 */
int DsgDocsisStatus( void )
{
	unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif
    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgDocsisStatus;

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

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


bool DsgSetRIAdr( void )
{
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
	unsigned char DsgConfig[64];
	uint32 ipaddress;
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif
    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

	ipaddress = kLocalRxIpAddress;

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgSetRIAddr;
    DsgConfig[2] = (unsigned char)(ipaddress >> 24);
    DsgConfig[3] = (unsigned char)(ipaddress >> 16);
    DsgConfig[4] = (unsigned char)(ipaddress >> 8);
    DsgConfig[5] = (unsigned char)(ipaddress);

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

#else

	return true;
#endif
}

/*
 *
 *
 *
 *
 *  This function returns:
 *  kFuncSuccess = success
 *  kFuncInvalid = out of range
 *  kFuncFail = function failure
 */
int DsgGetEcmHeartBeat( uint16 *pHbeatCount )
{
	unsigned char DsgConfig[64];
    int ret_val;

    pthread_mutex_lock(&LnxDsgApiMutex);

    //Reset memory.
    memset( DsgConfig, 0, 64);

    DsgConfig[0] = ++TransId;
    DsgConfig[1] = kDsgGetEcmHeartBeat;

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false );
    if( ret_val != kFuncFail )
    {
        // Retrieve counter value from the message.
        *pHbeatCount = (DsgConfig[3] << 8);
        *pHbeatCount |= (DsgConfig[4] & 0x00FF);
    }
    else if( (DsgConfig[0] == TransId) )
    {
        // if Ecm running older SW, print warning but return success.
        printf("\n!!!Warning:Ecm does not support kDsgGetEcmHeartBeat, cannot detect Ecm reset&recovery.\nPlease update Ecm image!!!\n\n");
        ret_val = kFuncSuccess;
    }

    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

}

/*
 *  Report DSG_error to eCM
 *  Only the following error is being reported to eCM
 *  1 == Invalid_DSG_channel
 *
 *  This function returns:
 *  kFuncSuccess = success
 *  kFuncInvalid = mode out of range
 *  kFuncFail = function failure
 */
int DsgReportDsgError( uint8 error_status )
{
	unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    // Generate the message.
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgReportDsgError;
    DsgConfig[2] = error_status;

    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;
}

bool DsgSetEstbPrimaryIpV4Addr( uint32 ipAddr, uint8 *macAddr )
{
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
    unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgSetEstbPrimaryAddr;
    DsgConfig[2] = 4;   // IPv4
    DsgConfig[3] = 0;
    DsgConfig[4] = (unsigned char)(ipAddr >> 24);
    DsgConfig[5] = (unsigned char)(ipAddr >> 16);
    DsgConfig[6] = (unsigned char)(ipAddr >> 8);
    DsgConfig[7] = (unsigned char)(ipAddr);

    DsgConfig[8] = *macAddr++;
    DsgConfig[9] = *macAddr++;
    DsgConfig[10] = *macAddr++;
    DsgConfig[11] = *macAddr++;
    DsgConfig[12] = *macAddr++;
    DsgConfig[13] = *macAddr;

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

#else

	return true;
#endif
}


bool DsgSetEstbPrimaryIpV6Addr( uint8 *ipAddr, uint8 *macAddr )
{
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
    unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgSetEstbPrimaryAddr;
    DsgConfig[2] = 6;   // IPv6
    DsgConfig[3] = 0;

    // Copy IPv6 IP address (128 bits = 16 Bytes)
	memcpy(&DsgConfig[4], ipAddr, 16);

	// Copy Mac address (48 bits = 6 Bytes)
	memcpy(&DsgConfig[20], macAddr, 6);

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

#else

	return true;
#endif
}


bool DsgSetDhcpOption43(void *data, int dataLen)
{
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
		unsigned char DsgConfig[260];
		int ret_val;
#if (DSG_RECOVER_SUPPORT)
		uint32 iteration = 5;
		unsigned char xid;
#endif
		// We think 256 bytes will be enough for the DHCP option 43 suboption buffer
		// dataLen should be less than 256 bytes.
		if (dataLen > 256) return false;

		pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
		xid = ++TransId;

		while (iteration > 0)
		{
#endif
		//Reset memory.
		memset( DsgConfig, 0, 256);

#if (DSG_RECOVER_SUPPORT)
		DsgConfig[0] = xid;
#else
		DsgConfig[0] = ++TransId;
#endif
		DsgConfig[1] = kDsgSetDhcpV4VendorSpecificOption;
		DsgConfig[2] = (uint8) dataLen; //datalen should be less than 256 bytes
		DsgConfig[3] = 0;

		// Copy DHCP option 43 suboption values to the TX buffer
		memcpy(&DsgConfig[4], data, dataLen);

		// Send the message
		ret_val = EstbToEcmSocketSendRx(dataLen + 4, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
		if (ret_val > 0)
		{
			break;
		}
		else
		{
			printf("+++ Send Enable Docsis command again!\n");
			iteration--;
		}
		}
#endif
		pthread_mutex_unlock(&LnxDsgApiMutex);

		return ret_val;
#else
	return true;
#endif
}

bool DsgSetDhcpV6Option17(void *data, int dataLen)
{
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
	unsigned char DsgConfig[260];
	int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif
	// We think 256 bytes will be enough for the DHCP option 17 suboption buffer
	// dataLen should be less than 256 bytes.
	if (dataLen > 256) return false;

	pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
	xid = ++TransId;

	while (iteration > 0)
	{
#endif
		//Reset memory.
		memset( DsgConfig, 0, 256);

#if (DSG_RECOVER_SUPPORT)
		DsgConfig[0] = xid;
#else
		DsgConfig[0] = ++TransId;
#endif
		DsgConfig[1] = kDsgSetDhcpV6VendorSpecificOption;
		DsgConfig[2] = (uint8) dataLen; //datalen should be less than 256 bytes
		DsgConfig[3] = 0;

		// Copy DHCP option 17 suboption values to the TX buffer
		memcpy(&DsgConfig[4], data, dataLen);

		// Send the message
		ret_val = EstbToEcmSocketSendRx(dataLen + 4, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
		if (ret_val > 0)
		{
			break;
		}
		else
		{
			printf("+++ Send Enable Docsis command again!\n");
			iteration--;
		}
	}
#endif
	pthread_mutex_unlock(&LnxDsgApiMutex);

	return ret_val;
#else
		return true;
#endif
}


bool DsgSetCCMacAddr (uint8 * macAddr)
{
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
    unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kDsgSetCCMac;
    DsgConfig[2] = 0;
    DsgConfig[3] = 0;

	// Copy Mac address (48 bits = 6 Bytes)
	memcpy(&DsgConfig[4], macAddr, 6);

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

#else

	return true;
#endif
}


bool SetEsafeDevStatus( uint8 intImpact, char * intImpactInfo )
{
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
  unsigned char DsgConfig[260];
  int retVal, infoLen;;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

  infoLen = strlen(intImpactInfo);
  // the MIB object that this is destined for is max 255 bytes, so we will truncate it
  infoLen &= 0x000000ff;

  pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 260);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = (unsigned char) kEsafeDevStatus;
    DsgConfig[2] = (unsigned char) intImpact;
    DsgConfig[3] = (unsigned char) infoLen;
    memcpy( &DsgConfig[4], intImpactInfo, infoLen );

	// Send the message
    retVal = EstbToEcmSocketSendRx( 260, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (retVal > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return retVal;

#else

	return true;
#endif
}


// OC-SP-HOST2.1-CFR - Table 15.1-1 - Provisioning steps of the OCHD2.1

//=== Flow Step ==== eSAFE MIB esafeProvisioningStatusProgress value ==

//"OCHD2.1-Prov-0" : notInitiated	 	           	 (1)
//"OCHD2.1-Prov-1",: estbTlvProcessingInProgress	 (2)
//"OCHD2.1-Prov-2",: estbDhcpV4InProgress		     (2)
//"OCHD2.1-Prov-3",: estbDhcpV6InProgress			 (2)
//"OCHD2.1-Prov-4" : estbProvFinished                (3)

// If a failure occurs in processing the eSTB TLVs, the value of the eSAFE MIB object
// esafeProvisioningStatusFailureFlow SHALL be set to OCHD2.1-Prov-1, and the value of the eSAFE MIB
// object esafeProvisioningStatusProgress SHALL be set to (3) finished.
// If a failure occurs in acquiring an IPv4 address, the value of the eSAFE MIB object
// esafeProvisioningStatusFailureFlow SHALL bet set to OCHD2.1-Prov-2, and the eSAFE MIB object
// esafeProvisioningStatusProgress SHALL be set to (3) finished.
// If a failure occurs in acquiring an IPv6 address, the value of the eSAFE MIB object
// esafeProvisioningStatusFailureFlow SHALL be set to OCHD2.1-Prov-3, and the eSAFE MIB object
// esafeProvisioningStatusProgress SHALL be set to (3) finished.
//
// The following eSAFE MIBs are updated:
//
// esafeProvisioningStatusProgress
// esafeProvisioningStatusFailureEventID
// esafeProvisioningStatusFailureFlow
// esafeProvisioningStatusFailureErrorText
//
bool SetEsafeProvisioningStatus( uint8 progress, uint8 failureFound, uint32 eventId )
{
	return SetEsafeProvisioningStatusAndFailure(progress, failureFound, eventId, 0, NULL, 0, NULL);
}

bool SetEsafeProvisioningStatusAndFailure( uint8 progress, uint8 failureFound, uint32 eventId, uint8 provFailureFlowLen, char *provFailureFlow,
								                                                     uint8 provErrorTextLen, char* provErrorText)
{
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
    int ret_val;
	unsigned int bufferLen;
	unsigned char *pDsgConfig;
	unsigned char *pDsgConfigPtr = NULL;

	// the MIB object that this is destined for is max 255 bytes, so we will truncate it
	provFailureFlowLen &= 0x000000ff;
	provErrorTextLen &= 0x000000ff;

	bufferLen = 10 + provFailureFlowLen + provErrorTextLen;

    // Pad out the buffer to minimum expected reply size.
    if (bufferLen < kEcmReplyPktSize)
        bufferLen = kEcmReplyPktSize;

#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

    pthread_mutex_lock(&LnxDsgApiMutex);

    // allocate memory
	pDsgConfig = (unsigned char *)malloc(bufferLen);

	if( pDsgConfig == NULL )
    {
        pthread_mutex_unlock(&LnxDsgApiMutex);
		return kFuncFail;
    }

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( pDsgConfig, 0, bufferLen);
	pDsgConfigPtr = pDsgConfig;

#if (DSG_RECOVER_SUPPORT)
    *pDsgConfigPtr++ = xid;
#else
    *pDsgConfigPtr++ = ++TransId;
#endif
    *pDsgConfigPtr++ = kEsafeProvisioningStatus;
    *pDsgConfigPtr++ = progress;
    *pDsgConfigPtr++ = failureFound;
    *pDsgConfigPtr++ = (unsigned char)(eventId >> 24);
    *pDsgConfigPtr++ = (unsigned char)(eventId >> 16);
    *pDsgConfigPtr++ = (unsigned char)(eventId >> 8);
    *pDsgConfigPtr++ = (unsigned char)(eventId);

	*pDsgConfigPtr++ = provFailureFlowLen;
	if( provFailureFlowLen && provFailureFlow )
		memcpy(pDsgConfigPtr, provFailureFlow, provFailureFlowLen);

	*(pDsgConfigPtr + provFailureFlowLen) = provErrorTextLen;
	if( provErrorTextLen && provErrorText )
		memcpy((pDsgConfigPtr + provFailureFlowLen + 1), provErrorText, provErrorTextLen);

	// Send the message
    ret_val = EstbToEcmSocketSendRx( bufferLen, pDsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif

	free( pDsgConfig );
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

#else

	return true;
#endif
}

bool SetApiVersion(uint32 *pEcmVersion)
{
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
    unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

	uint32 version = htonl(kDsgApiVersion);

    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kSetApiVersion;
    DsgConfig[2] = 0;
    DsgConfig[3] = 0;

	// Copy API version
	memcpy(&DsgConfig[4], &version, sizeof(uint32));

	// Send the message
    ret_val = EstbToEcmSocketSendRx( kEcmReplyPktSize, DsgConfig, true, true);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable Docsis command again!\n");
        iteration--;
    }
    }
#endif

	// Now read the return value:
	memcpy( pEcmVersion, &DsgConfig[8], sizeof(uint32));
	*pEcmVersion = ntohl(*pEcmVersion);

    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

#else

	return true;
#endif
}

// Helper Function returns the IP address of the Primary eSTB IP stack
// for this HalIf and the interface name passed in.
//
// Multiple IP addresses in Linux is not supported at this time.
//
// Added this method for PR2256.
//
// Parameters:
//      pEstbIpAddress - the IP address will be returned in this parameter.
//
// Returns:
//      true if the IP address was successfully querried.
//      false if there was a problem acquiring the IP address.
//
bool GetEstbIpAddress(uint32* pEstbIpAddress)
{
	char *interfaceName = EstbPrimaryIpStackName;

	if(GetIpStackAddress(pEstbIpAddress, interfaceName) == 0 )
	{
		return true;
	}

	return false;
}

// Helper Function returns the MAC address of the Primary eSTB IP stack
// for this HalIf and the interface name passed in.
//
// Parameters:
//      pEstbMacAddress - address to copy the MAC address.
//
// Returns:
//      true if the MAC address was successfully querried.
//      false if there was a problem acquiring the MAC address.
//
bool GetEstbMacAddress(uint8* pEstbMacAddress)
{
	char *interfaceName = EstbPrimaryIpStackName;

	if (GetInterfaceMacAddress(interfaceName, pEstbMacAddress))
	  return true;

  return false;
}

#if !(BFC_INCLUDE_DAVIC_API_SUPPORT)

/*
 * This Thread will listen on socket (0x4247) for UDP packets
 * to receive eCM status notifications and DCD-MGMT messages
 * and call the appropriate API
 */
void EstbDsgRemoteInterfaceHelperThread ( void )
{
	struct sockaddr_in sin_ctl;
	struct sockaddr_in sin_dat;

    register int cnt;
	struct sockaddr_in frominet;
	socklen_t      inetsize;
	char              *buf;					/* ptr to dynamic buffer */
	char              *dsgPacketBuffer;		/* ptr to dynamic buffer */
	int		buflen  = 1518;					/* length of buffer */

	int sock_ctl, sock_dat, maxfdp1;
	fd_set rfds;
	int retcode;
	int optVal;

	ETHERNET_PACKET *pEthernetPacketBuffer = NULL;

	#if defined (USE_MULTICAST_SOCKET)
	// Wait to acquire an IP address to
	// setup multicast sockets.
	uint32 ipAddress = 0;
	while( GetEstbIpAddress(&ipAddress) == 0 )
	{
		// Wait
		osSleep( 5 );
	}
	#endif

	// Create NOTIFICATION Socket
    if ((sock_ctl = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("EstbDsgRemoteInterfaceHelperThread(): Error creating sock_ctl!\n");
		goto err_create_sock_ctl;
    }

    #if !defined(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
	// Create DATA Socket
    if ((sock_dat = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("EstbDsgRemoteInterfaceHelperThread(): Error creating sock_dat!\n");
		goto err_create_sock_dat;
    }

	// Configure DATA address and port
	bzero((char*)&sin_dat, sizeof(sin_dat));

    sin_dat.sin_family = AF_INET;
	sin_dat.sin_port = htons(ESTB_DSG_DATA_PORT);
	sin_dat.sin_addr.s_addr = htonl(kLocalRxIpAddress);

    #endif

	// Configure NOTIFICATION address and port
	bzero((char*)&sin_ctl, sizeof(sin_ctl));
    sin_ctl.sin_family = AF_INET;
	sin_ctl.sin_port = htons(ESTB_DSG_REQUEST_PORT);
	sin_ctl.sin_addr.s_addr = htonl(kLocalRxIpAddress);

	// REUSE ADDRESS
    optVal = 1;

    if (setsockopt(sock_ctl, SOL_SOCKET, SO_REUSEADDR, (char *) &optVal, sizeof(optVal)) != 0)
	{
		printf ("setsockopt SO_REUSEADDR error:  %s \n"
						, strerror(errno));
	}

    #if !defined(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
    if (setsockopt(sock_dat, SOL_SOCKET, SO_REUSEADDR, (char *) &optVal, sizeof(optVal)) != 0)
	{
		printf ("setsockopt SO_REUSEADDR error:  %s \n"
						, strerror(errno));
	}
    #endif

	// Bind to the NOTIFICATION port
    if (bind(sock_ctl, (struct sockaddr *)&sin_ctl, sizeof(sin_ctl)) < 0)
    {
        printf("EstbDsgRemoteInterfaceHelperThread(): Error binding sock_ctl_in!\n");
		goto err_bind_sock_ctl;
    }

    #if !defined(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
	// Bind to the DATA port
    if (bind(sock_dat, (struct sockaddr *)&sin_dat, sizeof(sin_dat)) < 0)
    {
        printf("EstbDsgRemoteInterfaceHelperThread(): Error binding sock_dat_in!\n");
		goto err_bind_sock_dat;
    }
    #endif

	#if (defined (USE_MULTICAST_SOCKET))
		JoinMulticastSession( sock_ctl );
	  #if !defined(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
		JoinMulticastSession( sock_dat );
	  #endif
	#endif

	//Allocate buffer to receive NOTIFICAION.
    if( (buf = (char *)malloc(buflen)) == (char *)NULL)
	{
		printf("Can not allocate buffer %d\n", buflen);
		goto err_malloc;
	}

	//Allocate buffer to receive DSG Packet.
    if( (dsgPacketBuffer = (char *)malloc(buflen)) == (char *)NULL)
	{
		printf("Can not allocate buffer %d\n", buflen);
		goto err_malloc;
	}

    printf("Starting eSTB HelperThread. Opening sockets and listening on ports %04x & %04x...\n", ESTB_DSG_DATA_PORT, ESTB_DSG_REQUEST_PORT );

	#if !defined(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
	maxfdp1 = ((sock_ctl > sock_dat) ? sock_ctl : sock_dat) + 1;
	#else
	    maxfdp1 = sock_ctl + 1;
	#endif
	while(1)
	{
		FD_ZERO(&rfds);
		FD_SET(sock_ctl, &rfds);
		#if !defined(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
		FD_SET(sock_dat, &rfds);
		#endif

		retcode = select(maxfdp1, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);

		if (retcode == -1)
		{
			printf("select(): %s\n", strerror(errno));
		}
		else
		{
			// Listen for NOTIFICATIONS
			if(FD_ISSET(sock_ctl, &rfds))
			{
				// Configure NOTIFICATION and DATA sockets to receive packets with eCM address.
				inetsize = sizeof(frominet);

				// recvfrom should not block now
				cnt = recvfrom(sock_ctl, buf, buflen, 0, (struct sockaddr*)&frominet, &inetsize);
				if( (cnt > 0) )
					//&& ( frominet.sin_addr.s_addr == htonl(kRemoteEcmSrcIpAddress)) )
				{
					// Pass the notification to DSG-CC.
					// printf("NOTIFICATION from [ %s ]: with len = %d on port = %02x \n", inet_ntoa(frominet.sin_addr), cnt, sin_ctl.sin_port);
					if(displayMode & kLevel4)
						LogRawBuffer ("Notification: RCVD: ", (uint8*)buf, cnt, 0);

					// Process the request. Contents of "buf" will be modified when ProcessDsgClientPacketFromHalif()
					// returns. ProcessDsgClientPacketFromHalif will read the request and place the response
					// in the same buffer
					ProcessEcmNotifications( buf );
				}
			}

            #if !defined(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
			// Listen for DATA
			if(FD_ISSET(sock_dat, &rfds))
			{
				inetsize = sizeof(frominet);
				// recvfrom shoud not block now
				cnt = recvfrom(sock_dat, dsgPacketBuffer, buflen, 0, (struct sockaddr*)&frominet, &inetsize);
				if( (cnt > 0) )
					//&& ( frominet.sin_addr.s_addr == htonl(kRemoteEcmSrcIpAddress)) )
				{
					//printf("DATA PKT from [ %s ]: with len = %d on port = %02x \n", inet_ntoa(frominet.sin_addr), cnt, ESTB_DSG_DATA_PORT);

					/**************** WRAP THE BUFFER IN ETHERNET PACKET STRUCTURE **********/

						pEthernetPacketBuffer = WrapEthernetPacket(dsgPacketBuffer, cnt);
						if( pEthernetPacketBuffer != NULL )
						{
							if( oobMode == kOobModeDsg )
							{
								if(displayMode & kSebLevel2)
									LogRawBuffer ("SEB TEST: DATA PKT:", pEthernetPacketBuffer->pDataBufferHead->pvStart, pEthernetPacketBuffer->pDataBufferHead->uiLen, 0);
								// Pass the DATA to DSG-CC.
								// DsgTunnelPkt is a VENDOR method.
								// eSTB is RESPONSIBLE for releasing the buffers
								// by calling DsgFreeEthernetPkt()
								if( DsgTunnelPkt( pEthernetPacketBuffer ) == false)
								{
									// eSTB did NOT consume the buffer.
									// We have to release the buffers NOW.
									DsgFreeEthernetPkt(pEthernetPacketBuffer);
								}
							}
							#if (BFC_INCLUDE_DAVIC_API_SUPPORT)
							else if(oobMode == kOobModeDavic )
							{
								if( BcmDavicRxTunnelPkt( pEthernetPacketBuffer ) == false)
								{
									// eSTB did NOT consume the buffer.
									// We have to release the buffers NOW.
									DsgFreeEthernetPkt(pEthernetPacketBuffer);
								}
							}
							#endif
						}
					/*************************************************************************/
				}
				//else
				//	printf("DATA PKT from [ %s ]:ERROR- Dropping packet. recvfrom returns %d %s\n", inet_ntoa(frominet.sin_addr), cnt, strerror(errno));
			}
			#endif
		}
	}
	printf("DsgRemoteInterfaceHelperThread fell through!\n");

    free(buf);
    free(dsgPacketBuffer);

err_malloc:
err_bind_sock_dat:
err_bind_sock_ctl:
#if defined(WIN32)
	closesocket(sock_dat);
#else
        #if !defined(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
	close(sock_dat);
        #endif
#endif
err_create_sock_dat:
#if defined(WIN32)
	closesocket(sock_ctl);
#else
	close(sock_ctl);
#endif
err_create_sock_ctl:
	return;
}
#endif

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
ETHERNET_PACKET* WrapEthernetPacket( char *buf, uint32 length )
{
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
	memset(pDataBuffer, 0, sizeof(*pDataBuffer));
	memset(pEthernetPacketBuffer, 0, sizeof(*pEthernetPacketBuffer));

	// Set DATA_BUFFER struct
	pDataBuffer->pvStart = (PVOID)buf;
	pDataBuffer->uiLen   = length;

	//Fill in the starting data buffer pointer address for the
	//Ethernet packet we are going to use for this data.
	pEthernetPacketBuffer->pDataBufferHead = pDataBuffer;

	//Use current FreeDataBuffPtr as last buffer in EthPktChain.
	pEthernetPacketBuffer->pDataBufferTail = pDataBuffer;
	pEthernetPacketBuffer->pNext = NULL;

	return pEthernetPacketBuffer;
}

void ProcessEcmNotifications(char * buf)
{
	switch( buf[0] )
	{
	case kEventDsgScanSuccess:
		{
			// Notify eSTB.
			DsgScanSuccess();
		}
		break;

	case kEventDsgFullScanFail:
		{
			// Notify eSTB.
			DsgFullScanFail();
		}
		break;

	case kEventDsgRxDCD:
		{
			int dcd_len;
			unsigned char * pDcdStart;
			unsigned char * pbuf = (unsigned char *)buf;

			//skip the 1st byte event id
			pbuf++;

			// get the packet length
			dcd_len  = ((int)*pbuf++) << 24;
			dcd_len |= ((int)*pbuf++) << 16;
			dcd_len |= ((int)*pbuf++) << 8;
			dcd_len |= ((int)*pbuf++) << 0;

			//get the DCD payload start ptr
			pDcdStart = pbuf;

			// eSTB will copy the buffer and return
			// in DsgRxDCD().
			// Notify eSTB.
			DsgRxDCD( dcd_len, pDcdStart );
		}
		break;

	case kEventDsgUpstreamChange:
		{
			DsgEcmUsSettings dsgEcmUsSettings;

			// Notify eSTB.
			dsgEcmUsSettings.upstreamState = buf[1];
			dsgEcmUsSettings.ipMode = buf[2];
			DsgUpstreamChange( &dsgEcmUsSettings );
			printf("kEventDsgUpstreamChange: us_state %d, ip_mode %d\n", dsgEcmUsSettings.upstreamState, dsgEcmUsSettings.ipMode);
		}
		break;

	case kEventDsgUcid:
		{
			// Notify eSTB.
			DsgUcid( (uint8) buf[1]  );
		}
		break;

	case kEventDsgTimerEvent:
		{
			// Notify eSTB.
			DsgTimerEvent( (uint8) buf[1]);
		}
		break;

	case kEventDsgDccInitType:
		{
			// Notify eSTB.
			DsgDccInitType( (uint8) buf[1]);
		}
		break;

	case kEventDsgTwoWayForwardingDisabled:
		{
			// Notify eSTB.
			DsgTwoWayForwardingDisabled( (uint8) buf[1], (uint8) buf[2]);
			printf(" disabled_forwarding_type: %d  maxCPE: %d (not used)\n", (uint8) buf[1], (uint8) buf[2]);
		}
		break;

#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
	// We received notification that the Docsis app. is resetting.
	// We'll bring down the DSG interface (USB, ETH or PCI) so that
	// there aren't any issues created by someone accessing the interface
	// during a reset.
	// The DSG-CC's keepalive functionality will bring up the interface
	// once the docsis app. comes out of reset.
	case kEventDocsisIsResetting:
		{
			DsgEcmResetIndication( (uint8) buf[1]);
		}
		break;
#endif

#if (CABLECARD_SUPPORT)
	case kEventDsgCableCardDhcpReply:
		{
			unsigned long ipaddr;
			char * pOptionStart;
			char * pbuf = buf;
            unsigned char optionlen;

			//skip the 1st byte event id
			pbuf++;

			// get the ipaddress in. The ipaddr will be in the host byte order
			ipaddr  = ((unsigned char)*pbuf++) << 24;
			ipaddr |= ((unsigned char)*pbuf++) << 16;
			ipaddr |= ((unsigned char)*pbuf++) << 8;
			ipaddr |= ((unsigned char)*pbuf++) << 0;

            // get the dhcp repluy option length to follow
            optionlen = (unsigned char)*pbuf++;

			//get the DHCP option start ptr
			pOptionStart = pbuf;

			printf("*******>>>> Received DHCP reply from eCM 0x%08lx (optionlen=%d)<<<<<***********\n", ipaddr, optionlen );
			// Bypass stubInterface.c and Client controller and send direct to CableCardIf
			BcmEstbIndicateCableCardProxyDhcpResponse( ipaddr, pOptionStart, optionlen );
		}
		break;

	case kEventDsgCableCardIpDsData:
		{
			int data_len;
			unsigned char *pDataStart;
			unsigned char *pbuf = (unsigned char *)buf;

			//skip the 1st byte event id
			pbuf++;

			// get the packet length
			data_len  = ((int)*pbuf++) << 24;
			data_len |= ((int)*pbuf++) << 16;
			data_len |= ((int)*pbuf++) << 8;
			data_len |= ((int)*pbuf++) << 0;

			//get the data payload start ptr
			pDataStart = pbuf;

			// Notify CableCard Driver.
			BcmSendDsDataToCableCard( data_len, pDataStart );
		}
		break;
#endif

#if defined (COMMON_DOWNLOAD_SUPPORT)
	case kEventDsgCommonDownloadNotification:
		{
			CommonDownloadProcessEcmNotification((uint8 *)buf);
		}
		break;
#endif

#if (BFC_INCLUDE_DAVIC_API_SUPPORT)
	case kEventDavicNotification:
		{
			//printf("DAVIC Notification!\n");
			BcmDavicProcessEcmNotification((uint8 *)buf);
		}
		break;
#endif

	case kEventEcmHeartBeat:
		{
			static uint16 lastHeartBeatCount = 0;
			uint16 ecmHeartBeatCount=0;

			// Retrieve counter value from the message.
			ecmHeartBeatCount = (buf[1] << 8);
			ecmHeartBeatCount |= (buf[2] & 0x00FF);

			//printf("lastHeartBeatCount %d, ecmHeartBeatCount %d\n", lastHeartBeatCount, ecmHeartBeatCount );

			// Check if eCM has reset.
			if( lastHeartBeatCount > ecmHeartBeatCount )
			{
				printf("eCM RESET!\n");

				// Send notification
				DsgEcmResetIndication(kEcmReset);

				// CheckApiVersion
				CheckApiVersion();
			}

			// update Count and check for roll over. Reserved 0 for detecting roll over reset
			lastHeartBeatCount = ecmHeartBeatCount;
			if( lastHeartBeatCount == 0xffff )
				lastHeartBeatCount = 1;
		}
		break;

	case kEventDsgEcmEventNotification:
		{
			int eventCode;
			char *pbuf = (char *)buf;

			//skip the 1st~5th bytes, which are event id and packet length
			pbuf+=5;

			memcpy((uint8*)&eventCode, pbuf, 4);
			eventCode = ntohl(eventCode);

			pbuf+=4;

			// pass the event code and event text;
			DsgDocsisEvent (eventCode, pbuf);
		}
		break;

	case kEventDsgEcmMacReinit:
		{
			DsgEcmMacReinitEvent();
		}
		break;

	default:
		{
			printf("NOTIFICATION FROM ECM NOT RECOGNIZED %d\n", buf[0]);
		}
		break;
	}
}

// Set up the sock interface for the test thread
SetupRemoteInterfaceReturnCodes SetupRemoteInterface(char *pIpcInterfaceName)
{
	struct sockaddr_in sinme;
	int optVal;
	char *pStr = NULL;

	if( socketInitComplete == false )
	{
		sock_ctl_out = 0;
		sock_ctl_in = 0;

		if( pIpcInterfaceName != NULL )
		{
			printf("SetupRemoteInterface() passed us eCM interface name %s\n", pIpcInterfaceName);
			strncpy(EstbPrimaryIpStackName, pIpcInterfaceName, (sizeof(EstbPrimaryIpStackName) - 1));
			EstbPrimaryIpStackName[sizeof(EstbPrimaryIpStackName) - 1] = '\0';
		}
		else
		{
			printf("SetupRemoteInterface() did not specify the eCM interface name. Using self-detect... \n");

			memset(ecmInterfaceName, 0, IPC_INTERFACE_NAME_MAX_LEN);
			if( FindEcmInterface(ecmInterfaceName) )
			{
				strncpy(EstbPrimaryIpStackName, ecmInterfaceName, (sizeof(EstbPrimaryIpStackName) - 1));
				EstbPrimaryIpStackName[sizeof(EstbPrimaryIpStackName) - 1] = '\0';
				printf("Detected eCM interface: %s configured with %x \n", EstbPrimaryIpStackName, kLocalRxIpAddress);
			}
			else
				printf("WARNING: Failed to find an interface configured with %d. Using default value from dsg_api.h for EstbPrimaryIpStackName: %s\n", kLocalRxIpAddress, EstbPrimaryIpStackName);
		}

		// Remove ":" from EstbPrimaryIpStackName.
		pStr = strstr(EstbPrimaryIpStackName, ":");
		if(pStr)
			*pStr = 0;

		printf("EstbPrimaryIpStackName is set to %s \n", EstbPrimaryIpStackName);

		// Create CONTROL TX Socket
		if ((sock_ctl_out = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			printf("SetupRemoteInterface(): Error creating sock_ctl_out!\n");
			return kRIsetupTxSockFailed;
		}

		// BIND CONTROL TX Socket
		bzero((char*)&sinme, sizeof(sinme));
		sinme.sin_family = AF_INET;
		sinme.sin_port = htons(0);
		sinme.sin_addr.s_addr = htonl(kLocalRxIpAddress);

		// Bind
		if (bind(sock_ctl_out, (struct sockaddr *)&sinme, sizeof(sinme)) < 0)
		{
			printf("SetupRemoteInterface: bind failure! %s\n", strerror(errno));
			close(sock_ctl_out);
			return kRIsetupTxSockBindFailed;
		}

		// Create CONTROL RX Socket
		bzero((char*)&sinme, sizeof(sinme));

		sinme.sin_family = AF_INET;
		sinme.sin_port = htons(ESTB_DSG_RESPONSE_PORT);
		sinme.sin_addr.s_addr = htonl(kLocalRxIpAddress);

		// Create socket
		if ((sock_ctl_in = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			printf("SetupRemoteInterface: socket failure! %s\n", strerror(errno));
			close(sock_ctl_out);
			return kRIsetupRxSockFailed;
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
			printf("SetupRemoteInterface: bind failure! %s\n", strerror(errno));
			close(sock_ctl_in);
			close(sock_ctl_out);
			return kRIsetupRxSockBindFailed;
		}

		socketInitComplete = true;
	}

	ConfigureEcmInterface();

	#if ( kDefaultValue_TaggingEnabled )
	ConfigureTaggingMode(taggingEnabled);
	#endif

	unsigned int count = 0;
    // Probe interface
    while( SetApiVersion(&ecmApiVersion) == kFuncFail )
	{
		// timeout is eCmResponseTimeout
		printf("tagging[%s] SetupRemoteInterface: LINK IS NOT UP YET %s!\n", (taggingEnabled?"on":"off"), strerror(errno));

		#if ( kDefaultValue_TaggingEnabled )
		// If supported toggle tagging.
		taggingEnabled = !taggingEnabled;
		ConfigureTaggingMode(taggingEnabled);
		#endif

		count++;
		if(count == kDefaultValue_EcmComAttemptMaxCount)
		{
			printf("SetupRemoteInterface: eCM is non-responsive!\n");
			return kRIsetupEcmNotResponding;
		}
	}

	printf("SetupRemoteInterface: LINK IS UP !\n");

	// Now send the API version
	CheckApiVersion();

	#if ( kDefaultValue_TaggingEnabled )
	if( ecmApiVersion > 0x104b3)
	{
		taggingEnabled = true;
	}
	else
	{
		taggingEnabled = false;
	}

	if( ConfigureTaggingMode(taggingEnabled) == true )
		printf("tagging[%s] !\n", (taggingEnabled?"on":"off"));

	#endif

	return kRIsetupNoError;
}

bool JoinMulticastSession( int socketDescriptor )
{
#if defined (USE_MULTICAST_SOCKET)
	uint32 ipAddress = htonl( INADDR_ANY );

	/* fill in the argument structure to join the multicast group */
	/* initialize the multicast address to join */
	struct ip_mreq      ipMreq;
	ipMreq.imr_multiaddr.s_addr = htonl(kLocalRxIpAddress);

	// Set unicast interface addr from which to receive the multicast packets
	if( GetEstbIpAddress(&ipAddress) == 0 )
	{
		return false;
	}

	ipMreq.imr_interface.s_addr = ipAddress;

	/* set the socket option to join the MULTICAST group */
	printf(" Joining multicast session to listen to eCM responses !\n");
	if (setsockopt (socketDescriptor, IPPROTO_IP, IP_ADD_MEMBERSHIP,
					(char *)&ipMreq,
					sizeof (ipMreq)) < 0)
	{
		printf ("setsockopt IP_ADD_MEMBERSHIP error:  %s %08lx %08lx\n"
				, strerror(errno), (uint32)ipMreq.imr_multiaddr.s_addr, (uint32)ipMreq.imr_interface.s_addr);
		return false;
	}

	// Turn loopback OFF.
	{
		uint8 loop = 0;
		if( setsockopt(socketDescriptor, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0 )
		{
			printf ("setsockopt IP_MULTICAST_LOOP error:  %s \n"
					, strerror(errno));
			return false;
		}
	}

	return true;
#endif
	return false;
}


#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
void ConfigureEcmInterface(void)
{
	uint32 flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
	uint8 macAddr[6] = kRemoteEcmMacAddress;

	// Wait until primary interfaces connects.
	if( SetInterfaceFlags(0, EstbPrimaryIpStackName, flags) == ENODEV )
	{
		return;
	}

	CheckApiVersion();

    printf("\nConfig Comm: eSTB <-- PRIVATE_NETWORK --> eCM:  EstbPrimaryIpStackName: %s\n", EstbPrimaryIpStackName);
    printf("-->|eSTB Interface Ip=%2d.%2d.%2d.%2d\n",
            (unsigned char)(kLocalRxIpAddress>>24), (unsigned char)(kLocalRxIpAddress>>16), (unsigned char)(kLocalRxIpAddress>>8), (unsigned char)(kLocalRxIpAddress) );
    printf("-->|eCM Mac=%02x.%02x.%02x.%02x.%02x.%02x Ip=%2d.%2d.%2d.%2d\n\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5],
            (unsigned char)(kRemoteEcmIpAddress>>24), (unsigned char)(kRemoteEcmIpAddress>>16), (unsigned char)(kRemoteEcmIpAddress>>8), (unsigned char)(kRemoteEcmIpAddress) );
}

void SetMulticastInterface(void)
{
	 // Set default interface for out-going multicast IP datagrams.
	struct in_addr mcastOutIfAddress;
	uint32 ipAddress = 0;

	// Get IP address of the interface used to
	// communicate with eCM.
	GetEstbIpAddress(&ipAddress);

	// Set default interface for out-going multicast IP datagrams.
	mcastOutIfAddress.s_addr = ipAddress;

	if( setsockopt(sock_ctl_out, IPPROTO_IP, IP_MULTICAST_IF, &mcastOutIfAddress, sizeof(mcastOutIfAddress)) < 0 )
	{
		printf ("setsockopt IP_MULTICAST_IF error:  %s %08lx\n", strerror(errno), (uint32)mcastOutIfAddress.s_addr);
	}
}

void CheckNewEstbPrimaryIpV4Address( bool forgetCurrIpAddr )
{
  static uint32 currentIpAddr = 0;
  static uint8 currentMacAddr[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  if( forgetCurrIpAddr )
  {
	currentIpAddr = 0;
  }

  uint32 newIpAddr;
  uint8 newMacAddr[6];

  if (GetEstbMacAddress(newMacAddr))
    if ( memcmp(newMacAddr, currentMacAddr, 6) != 0 ) {
      // Got a new MAC address!
      if (DsgSetEstbPrimaryIpV4Addr(currentIpAddr, newMacAddr))
	  {
			printf( "Updating eCM with eSTB IP:%08x and MAC=%02x.%02x.%02x.%02x.%02x.%02x \n"
					, htonl(currentIpAddr), currentMacAddr[0], currentMacAddr[1], currentMacAddr[2], currentMacAddr[3], currentMacAddr[4], currentMacAddr[5]);
			memcpy(currentMacAddr, newMacAddr, 6);
	  }
    }

  if (GetEstbIpAddress(&newIpAddr))
    if ( newIpAddr != currentIpAddr ) {
      // Got a new IP address!
      if (DsgSetEstbPrimaryIpV4Addr(newIpAddr, currentMacAddr))
	  {
			printf( "Updating eCM with eSTB IP:%08x and MAC=%02x.%02x.%02x.%02x.%02x.%02x \n"
					, htonl(currentIpAddr), currentMacAddr[0], currentMacAddr[1], currentMacAddr[2], currentMacAddr[3], currentMacAddr[4], currentMacAddr[5]);
			currentIpAddr = newIpAddr;
	  }
    }
}

void PrintNewEstbPrimaryIpAddress( void )
{
    static uint32 currentIpAddress = 0;
	uint32 newIpAddress;

    if(GetIpStackAddress(&newIpAddress, EstbPrimaryIpStackName) == 0)
    {
        // Get an Ip successfully
        if( newIpAddress != currentIpAddress )
        {
            currentIpAddress = newIpAddress;
            // print LE ipaddress
            printf( "ESTB (%s) new IP Address = %2d.%2d.%2d.%2d\n", EstbPrimaryIpStackName,
                     (unsigned char)(currentIpAddress), (unsigned char)(currentIpAddress>>8), (unsigned char)(currentIpAddress>>16), (unsigned char)(currentIpAddress>>24) );

#if 0
        // SWCMSTB-1508
        // The system calls below cause a crash with the latest toolchain, so
        // don't do this. Besides, it turns out that we don't need this if the
        // default route is set up properly.

            // Maintain 192.168.100.x comm for host SNMP use all the time.
            // If private primary Ip address is replaced with public ip sddress, add an alias interface.
            if( (currentIpAddress & 0x00ffffff) != 0x0064a8c0 ) //x.100.168.192
            {
                char syscmdstring[48];

                // Check if interface already exist
                sprintf( syscmdstring, "%s:1", EstbPrimaryIpStackName );
                if( GetIpStackAddress(&newIpAddress, syscmdstring) )
                {
                    // Get an Ip failed
                    printf( "Adding alias interface for 192.168.100.10 for Host side SNMP use.\n");
                    sprintf( syscmdstring, "ifconfig %s:1 192.168.100.10", EstbPrimaryIpStackName );
                    system(syscmdstring);
                    osSleep( 1 );
                    system("route");
                    osSleep( 1 );
                }
            }
#endif
        }
    }
}

#endif

#if (CABLECARD_SUPPORT)

// For cablecard IP_U flow that needs an IP address by using DHCP client in the eCM
int DsgStartProxyDhcpRequest( unsigned char opt_len, unsigned char *mac, unsigned char *opt_data, uint32 ipaddress) //unsigned char *ret_option )
{
	unsigned total_len = opt_len+9+4;
	unsigned char *pDsgConfig;
    int ret_val;

    pthread_mutex_lock(&LnxDsgApiMutex);

    printf("DsgStartProxyDhcpRequest\n");

    // Pad out the buffer to minimum expected reply size.
    if (total_len < kEcmReplyPktSize)
        total_len = kEcmReplyPktSize;

    // Add extra 9 bytes header for Id|cmd|6-bytes MAc|1-byte length
	pDsgConfig = (unsigned char *)malloc(total_len);

	if( pDsgConfig == NULL )
    {
        pthread_mutex_unlock(&LnxDsgApiMutex);
		return kFuncFail;
    }

    // zero out packet
    memset( pDsgConfig, 0, total_len);

    // Generate the message.
    *(pDsgConfig+0) = ++TransId;
    *(pDsgConfig+1) = kDsgCableCardDhcpReq;

	memcpy( (pDsgConfig+2), mac, 6 );

	// PR20830 - ipaddress
	memcpy( (pDsgConfig+8), &ipaddress, 4 );

    *(pDsgConfig+12) = opt_len;
	memcpy( (pDsgConfig+13), opt_data, opt_len );

	// Send the data
    ret_val = EstbToEcmSocketSendRx( total_len, pDsgConfig, true, false);

    free(pDsgConfig);
    pthread_mutex_unlock(&LnxDsgApiMutex);

	return ret_val;



}

// For cablecard IP_U flow that needs an IP address by using DHCP client in the eCM
int DsgShutDownProxyIpuStack( void )
{
	unsigned char DsgConfig[68];
    int ret_val;

    pthread_mutex_lock(&LnxDsgApiMutex);

    printf("DsgShutDownProxyIpuStack\n");

    //Reset memory.
    memset( DsgConfig, 0, 68);

    //Test the next configuration: Timer: Tdsg1
    //Use a shorter time-out for testing
    DsgConfig[0] = ++TransId;
    DsgConfig[1] = kDsgCableCardShutDownIpu;

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 68, DsgConfig, true, false);

    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

}

int DsgSendCableCardIpUsData( unsigned char *pData, unsigned short len )
{
	unsigned total_len = len + 4;       //Max total lengh should be 1518.
	unsigned char *pDsgConfig;
    int ret_val;

    pthread_mutex_lock(&LnxDsgApiMutex);

    // Pad out the buffer to minimum expected reply size.
    if (total_len < kEcmReplyPktSize)
    {
        total_len = kEcmReplyPktSize;
    }
	pDsgConfig = (unsigned char *)malloc(total_len);

	if( pDsgConfig == NULL )
    {
        pthread_mutex_unlock(&LnxDsgApiMutex);
		return kFuncFail;
    }

    // zero out packet
    memset( pDsgConfig, 0, total_len);

    // build packet header
    *pDsgConfig = ++TransId;
    *(pDsgConfig+1) = kDsgCableCardIpUsData;
	*(pDsgConfig+2) = (unsigned char)(len >> 8);
	*(pDsgConfig+3) = (unsigned char)len;

	// copy packet
	memcpy( pDsgConfig+4, pData, len );

	// Send the data
    ret_val = EstbToEcmSocketSendRx( total_len, pDsgConfig, true, false);

    free(pDsgConfig);
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

}

#endif

#if 0
//
//  Remote LNA access API sample function
//  For platform that does not control LNA, use this API to read or write to the LNA
//  Format:
//      unsigned char rdwr - 0 = read, 1 = write
//      unsigned char *pData -  pData[0] = register offset, pData[1] = rd/wr value
//                              pData[2] = next regisoffset, pData[3] = rd/wr value
//                              ......
//      unsigned char len - length of the above pair pData
//
// Sample calling test code:
//                {
//                    uint8 pData[74], i;
//
//                    // read test. Lna has a total of 37 registers
//                    for( i=0; i<=0x24; i++ )
//                    {
//                        pData[i*2] = i;   //reg offset
//                        pData[i*2+1] = 0; //init to zero
//                    }
//                    BcmEstbAcessRemoteLnaGain( 0, pData, 74 );
//                    for( i=0; i<=0x24; i++ )
//                        printf(" REG 0x%02x = 0x%02x\n", pData[i*2], pData[i*2+1] );
//
//                    // write test. Lna has a total of 37 registers
//                    for( i=0; i<=0x24; i++ )
//                    {
//                        pData[i*2] = i;           //reg offset
//                        pData[i*2+1] = 0x55+i;    //test value
//                    }
//                    BcmEstbAcessRemoteLnaGain( 1, pData, 74 );
//                }
//
int BcmEstbAcessRemoteLnaGain( unsigned char rdwr, unsigned char *pData, unsigned char len )
{
	STATUS status;
	unsigned total_len = len + 4;
	unsigned char *pDsgConfig;
    int ret_val;

    pthread_mutex_lock(&LnxDsgApiMutex);

    // Pad out the buffer to minimum expected reply size.
    if (total_len < kEcmReplyPktSize)
        total_len = kEcmReplyPktSize;

	pDsgConfig = (unsigned char *)malloc(total_len);

	if( pDsgConfig == NULL )
    {
        pthread_mutex_unlock(&LnxDsgApiMutex);
		return kFuncFail;
    }

    // zero out packet
    memset( pDsgConfig, 0, total_len);

    // build packet header
    *pDsgConfig = ++TransId;
    *(pDsgConfig+1) = kDsgRemoteLnaGain;
	*(pDsgConfig+2) = rdwr;
	*(pDsgConfig+3) = len;

   	// copy packet
    memcpy( pDsgConfig+4, pData, len );

	// Send the message
    ret_val = EstbToEcmSocketSendRx( total_len, pDsgConfig, true, false);

    if( (rdwr == 0) && (ret_val != kFuncFail) )     //0 = read, 1 = write
    	// copy read value back
	    memcpy( pData, pDsgConfig+4, len );

	free( pDsgConfig );
    pthread_mutex_unlock(&LnxDsgApiMutex);
	return ret_val;

}
#endif


#if defined(ESTB_SWDLOAD_SUPPORT)
/* Special function for Remote Sw DownLoad thread to call using static sock_ctl_out socket
*/
STATUS swdl_tudp_tx( int tpkt_size, unsigned char *pBuf, unsigned short port)
{
	struct sockaddr_in sin_dst;
    register int cnt;
    STATUS ret_val;

    pthread_mutex_lock(&LnxDsgApiMutex);
    ret_val = kFuncSuccess;

	bzero((char*)&sin_dst, sizeof(sin_dst));
    sin_dst.sin_family = AF_INET;
    sin_dst.sin_port = htons(port);
	sin_dst.sin_addr.s_addr = htonl(kRemoteEcmIpAddress);

	// Use CONTROL TX Socket
    cnt=sendto(sock_ctl_out, pBuf, tpkt_size, 0, (struct sockaddr*)&sin_dst, sizeof(sin_dst));
	if( cnt != tpkt_size )
	{
		printf("\nError: sendto returns %d  - %s\n", cnt, strerror(errno));
		ret_val = kFuncFail;
	}

    pthread_mutex_unlock(&LnxDsgApiMutex);

	return ret_val;
}
#endif

void FlushRxSocketBuffer(void)
{
	unsigned char rxBuffer[1500];
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);
	int bufferSize = 0;

	while( (bufferSize=recvfrom(sock_ctl_in, (char*)rxBuffer, sizeof(rxBuffer), MSG_DONTWAIT, (struct sockaddr*)&from, &fromlen)) > 0  )
		printf("Flushing Rx buffer with Xid %d from [ %s ]: UDP port %04x: pktlen %d\n", rxBuffer[0], inet_ntoa(from.sin_addr),  ntohs(from.sin_port), bufferSize);

}


#if defined (COMMON_DOWNLOAD_SUPPORT)
/*
 * CommonDownloadProcessEstbRequest
 *
 *	apiBuffer - pointer to the buffer
 *	apiLen	- length of the buffer
 *	apiType	-
 *
 * This function returns
 * true == success;
 * false == failed.
 *
 */
int CommonDownloadProcessEstbRequest(uint8 apiType, uint32 apiLen, uint8* apiBuffer, CommonDownloadResponseMsg* pCommonDownloadResponseMsg)
{
	unsigned char *DsgConfig;
	unsigned int bufferLen;
	int ret_val;
	unsigned char* pResponseBuf = NULL;

    pthread_mutex_lock(&LnxDsgApiMutex);

	bufferLen = 8 + apiLen;

    // Pad out the buffer to minimum expected reply size.
    if (bufferLen < kEcmReplyPktSize)
        bufferLen = kEcmReplyPktSize;

	//Allocate buffer to receive NOTIFICAION.
    if( (DsgConfig = (unsigned char *)malloc(bufferLen)) == NULL)
	{
		printf("Can not allocate buffer %d\n", bufferLen);
        pthread_mutex_unlock(&LnxDsgApiMutex);
		return kFuncFail;
	}

	printf("Allocate buffer %d\n", bufferLen);

	pResponseBuf = &DsgConfig[4];
	if( ecmApiVersion < 0x104b2 )
	{
		pResponseBuf = &DsgConfig[2];
	}

    //Reset memory.
    memset( DsgConfig, 0, bufferLen);

    DsgConfig[0] = ++TransId;
    DsgConfig[1] = kCommonDloadProcessEstbReq;
    DsgConfig[2] = apiType;

    DsgConfig[4] = (unsigned char)(apiLen >> 8);
    DsgConfig[5] = (unsigned char)apiLen;

	// secure buffer
	memcpy( &DsgConfig[8], apiBuffer, apiLen );

    ret_val = EstbToEcmSocketSendRx( bufferLen, DsgConfig, true, false);

	if( ret_val == kFuncSuccess)
		// Now read the return code and the info and return to the caller.
		CommonDownloadResponseMessage(pResponseBuf, pCommonDownloadResponseMsg);

	free(DsgConfig);
    pthread_mutex_unlock(&LnxDsgApiMutex);

	return pCommonDownloadResponseMsg->returnCode;
}


void CommonDownloadAuthenticateOrUpgradeFileX( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 tftpServerIpAddress, uint8 fileType, uint8 flags, CommonDownloadResponseMsg *responseMsg )
{
	unsigned char *DsgConfig;
	unsigned int bufferLen;
	int ret_val;

    pthread_mutex_lock(&LnxDsgApiMutex);

	// 2 bytes message header + 1 byte file type + 1 byte flags + 4 bytes file name length +
	// 4 bytes tftp ip address = 12 bytes total
	bufferLen = 12 + filePathOrBufferLen;

    // Pad out the buffer to minimum expected reply size.
    if (bufferLen < kEcmReplyPktSize)
        bufferLen = kEcmReplyPktSize;

	//Allocate buffer
    if( (DsgConfig = (unsigned char *)malloc(bufferLen)) == NULL)
	{
		printf("Can not allocate buffer %d\n", bufferLen);
        pthread_mutex_unlock(&LnxDsgApiMutex);
		return;
	}

    //Reset buffer.
    memset( DsgConfig, 0, bufferLen);

    DsgConfig[0] = ++TransId;
    DsgConfig[1] = kCommonDloadAuthenticateOrUpgrade;
	DsgConfig[2] = fileType;
	DsgConfig[3] = flags;

	if( flags & kUseTftp )
	{
		memcpy( &DsgConfig[4], &tftpServerIpAddress, 4 );
		/*DsgConfig[4] = (unsigned char)(tftpServerIpAddress >> 24);
		DsgConfig[5] = (unsigned char)(tftpServerIpAddress >> 16);
		DsgConfig[6] = (unsigned char)(tftpServerIpAddress >> 8);
		DsgConfig[7] = (unsigned char)(tftpServerIpAddress);*/
	}

	DsgConfig[8]  = (unsigned char)(filePathOrBufferLen >> 24);
	DsgConfig[9]  = (unsigned char)(filePathOrBufferLen >> 16);
	DsgConfig[10] = (unsigned char)(filePathOrBufferLen >> 8);
	DsgConfig[11] = (unsigned char)(filePathOrBufferLen);

	// filename buffer
	memcpy( &DsgConfig[12], filePathOrBuffer, filePathOrBufferLen );

	// DEBUG
	if(displayMode & kCdl)
		LogRawBuffer ("CommonDownloadAuthenticateOrUpgradeFileX:: ", DsgConfig, bufferLen, 0);

    ret_val = EstbToEcmSocketSendRx( bufferLen, DsgConfig, true, false);

	// DEBUG
	if(displayMode & kCdl)
		LogRawBuffer ("CommonDownloadAuthenticateOrUpgradeFileX:: ", DsgConfig, bufferLen, 0);

	/*
	+---------+---------+--------+---------+--------------+---------------+
	| TransId | Command |    unused        |  ReturnCode  |  Return Value |
	+---------+---------+--------+---------+--------------+---------------+
	 1 byte   1 byte      1 byte    1byte    4 bytes              4 bytes
	 */

	//#warning "!!!!!! UNFINISHED  - retVal returns DsgConfig;2 may be set to 0 for success !!!!!!!"

	if( ret_val == kFuncSuccess)
		// Now read the return code and the info and return to the caller.
		CommonDownloadResponseMessage(&DsgConfig[2], responseMsg);

	free(DsgConfig);
    pthread_mutex_unlock(&LnxDsgApiMutex);
}

void CommonDownloadAuthenticateOrUpgradeFile( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType, uint8 *pServerIpAddress, uint8 fileType, uint8 flags, CommonDownloadResponseMsg *responseMsg )
{
	unsigned char *DsgConfig;
	unsigned int bufferLen;
	int ret_val;

	// Use version info for backwards compatibility.
	if( ecmApiVersion < 0x104b2 )
	{
		// Temp fix for backward compatibility
		if(CommonDownloadProcessEstbRequest( kGetCurrEcmImageVers, 0, NULL, responseMsg) == 0)
		{
			if(ipAddrType == 6)
			{
				printf("ERROR - eCM CDL API does not support IPv6!\n");
				return;
			}

			// Use old style CommonDownloadAuthenticateOrUpgradeFile.
			uint32 ipAddress = 0;
			memcpy(&ipAddress, pServerIpAddress, 4);

			memset(responseMsg, 0, sizeof(*responseMsg));
			CommonDownloadAuthenticateOrUpgradeFileX( filePathOrBuffer, filePathOrBufferLen, ipAddress, fileType, flags, responseMsg );
			return;
		}
	}

	memset(responseMsg, 0, sizeof(*responseMsg));

    pthread_mutex_lock(&LnxDsgApiMutex);

	// 2 bytes message header + 1 byte file type + 1 byte flags + 4 bytes server address type +
	// 16 bytes tftp ip address + 4 bytes file name length = 28 bytes total
	bufferLen = 28 + filePathOrBufferLen;

    // Pad out the buffer to minimum expected reply size.
    if (bufferLen < kEcmReplyPktSize)
        bufferLen = kEcmReplyPktSize;

	//Allocate buffer
    if( (DsgConfig = (unsigned char *)malloc(bufferLen)) == NULL)
	{
		printf("Can not allocate buffer %d\n", bufferLen);
        pthread_mutex_unlock(&LnxDsgApiMutex);
		return;
	}

    //Reset buffer.
    memset( DsgConfig, 0, bufferLen);

    DsgConfig[0] = ++TransId;
    DsgConfig[1] = kCommonDloadAuthenticateOrUpgrade;
	DsgConfig[2] = fileType;
	DsgConfig[3] = flags;

	if( flags & kUseTftp )
	{
		DsgConfig[4] = (unsigned char)(ipAddrType >> 24);
		DsgConfig[5] = (unsigned char)(ipAddrType >> 16);
		DsgConfig[6] = (unsigned char)(ipAddrType >> 8);
		DsgConfig[7] = (unsigned char)(ipAddrType);

		if(ipAddrType == 4)
		{
			/*DsgConfig[8] = *pServerIpAddress++;
			DsgConfig[9] = *pServerIpAddress++;
			DsgConfig[10] = *pServerIpAddress++;
			DsgConfig[11] = *pServerIpAddress;*/
			memcpy(&DsgConfig[8], pServerIpAddress, 4 );
		}
		else if(ipAddrType == 6)
		{
			// Copy 16-byte IPv6 address.
			memcpy(&DsgConfig[8], pServerIpAddress, 16 );
		}
	}

	DsgConfig[24]  = (unsigned char)(filePathOrBufferLen >> 24);
	DsgConfig[25]  = (unsigned char)(filePathOrBufferLen >> 16);
	DsgConfig[26] = (unsigned char)(filePathOrBufferLen >> 8);
	DsgConfig[27] = (unsigned char)(filePathOrBufferLen);

	// filename buffer
	memcpy( &DsgConfig[28], filePathOrBuffer, filePathOrBufferLen );

	if(displayMode & kCdl)
		LogRawBuffer ("CommonDownloadAuthenticateOrUpgradeFile:: ", DsgConfig, bufferLen, 0);

    ret_val = EstbToEcmSocketSendRx( bufferLen, DsgConfig, true, false);

	/*
	+---------+---------+--------+---------+--------------+---------------+
	| TransId | Command |    unused        |  ReturnCode  |  Return Value |
	+---------+---------+--------+---------+--------------+---------------+
	 1 byte   1 byte      1 byte    1byte    4 bytes              4 bytes
	 */

	//#warning "!!!!!! UNFINISHED  - retVal returns DsgConfig;2 may be set to 0 for success !!!!!!!"

	if(displayMode & kCdl)
		LogRawBuffer ("CommonDownloadAuthenticateOrUpgradeFile:: ", DsgConfig, bufferLen, 0);

	if( ret_val == kFuncSuccess)
		// Now read the return code and the info and return to the caller.
		CommonDownloadResponseMessage(&DsgConfig[4], responseMsg);

	free(DsgConfig);
    pthread_mutex_unlock(&LnxDsgApiMutex);
}

/*
 * This function send the SHA-digest of the code image
 * to the eCM for authentication.
 *
 *	codeImageShaDigest - pointer to the codeImageShaDigest buffer
 * This function returns
 * true == success; codeImageShaDigest validated
 * false == failed. codeImageShaDigest is invalid.
 *
 */
int CommonDownloadVerifyCodeImageShaDigest( uint8* codeImageShaDigest, CommonDownloadResponseMsg* pCommonDownloadResponseMsg)
{
	unsigned char *DsgConfig;
	unsigned int bufferLen;
	int ret_val;
	unsigned char* pResponseBuf = NULL;

    pthread_mutex_lock(&LnxDsgApiMutex);

	/*
	Diagram of the Header:
														  20 bytes
	+---------+----------------------+---------+ +-------------------+--------------+
	| TransId | Command |            |         | |  SHA DIGEST
	+---------+----------------------+---------+ +-------------------+--------------+
	 Byte 0		Byte 1    Byte 2       Byte 3      Byte 4   Byte 5 ...........Byte 23
	*/

	bufferLen = 4 + 20; // SHA-digest is 20 bytes.

    // Pad out the buffer to minimum expected reply size.
    if (bufferLen < kEcmReplyPktSize)
        bufferLen = kEcmReplyPktSize;

	//Allocate buffer to receive NOTIFICAION.
    if( (DsgConfig = (unsigned char *)malloc(bufferLen)) == NULL)
	{
		printf("Can not allocate buffer %d\n", bufferLen);
        pthread_mutex_unlock(&LnxDsgApiMutex);
		return kFuncFail;
	}

	printf("Allocate buffer %d\n", bufferLen);

	pResponseBuf = &DsgConfig[4];
	if( ecmApiVersion < 0x104b2 )
	{
		pResponseBuf = &DsgConfig[2];
	}

    //Reset memory.
    memset( DsgConfig, 0, bufferLen);

    DsgConfig[0] = ++TransId;
    DsgConfig[1] = kCommonDloadVerifyImageShaDigest;

	// secure buffer
	memcpy( &DsgConfig[4], codeImageShaDigest, 20 );

    ret_val = EstbToEcmSocketSendRx( bufferLen, DsgConfig, true, false);

	if( ret_val == kFuncSuccess)
		// Now read the return code and the info and return to the caller.
		CommonDownloadResponseMessage(pResponseBuf, pCommonDownloadResponseMsg);

	free(DsgConfig);
    pthread_mutex_unlock(&LnxDsgApiMutex);

	return pCommonDownloadResponseMsg->returnCode;
}
#endif

#if (BFC_INCLUDE_DAVIC_API_SUPPORT)

/*
 * This Thread will listen on socket (0x8889) for UDP packets
 * to receive eCM status notifications and DCD-MGMT messages
 * and call the appropriate API
 */
void EstbDsgRemoteInterfaceDataHelperThread ( void )
{
    #if !defined(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
	struct sockaddr_in sin_dat;

    register int cnt;
	struct sockaddr_in frominet;
	socklen_t      inetsize;
	char              *dsgPacketBuffer;		/* ptr to dynamic buffer */
	int		buflen  = 1518;					/* length of buffer */

	int sock_dat, maxfdp1;
	fd_set rfds;
	int retcode;
	int optVal;

	ETHERNET_PACKET *pEthernetPacketBuffer = NULL;

	#if defined (USE_MULTICAST_SOCKET)
	// Wait to acquire an IP address to
	// setup multicast sockets.
	uint32 ipAddress = 0;
	while( GetEstbIpAddress(&ipAddress) == 0 )
	{
		// Wait
		osSleep( 5 );
	}
	#endif

	// Create DATA Socket
    if ((sock_dat = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0)) < 0)
    {
        printf("EstbDsgRemoteInterfaceDataHelperThread(): Error creating sock_dat!\n");
		goto err_create_sock_dat;
    }

	int rc, sizedattx, sizedatrx, szlen = sizeof(int);
    rc = getsockopt(sock_dat, SOL_SOCKET, SO_SNDBUF, (void *)&sizedattx, (socklen_t*)&szlen);
	if (rc == 0) printf("%s(%d): sock_dat %d has SO_SNDBUF size=%d (0x%x)\n", __FUNCTION__, __LINE__, sock_dat, sizedattx, sizedattx);
    rc = getsockopt(sock_dat, SOL_SOCKET, SO_RCVBUF, (void *)&sizedatrx, (socklen_t*)&szlen);
	if (rc == 0) printf("%s(%d): sock_dat %d has SO_RCVBUF size=%d (0x%x)\n", __FUNCTION__, __LINE__, sock_dat, sizedatrx, sizedatrx);

	// Configure DATA address and port
	bzero((char*)&sin_dat, sizeof(sin_dat));

    sin_dat.sin_family = AF_INET;
	sin_dat.sin_port = htons(ESTB_DSG_DATA_PORT);
	sin_dat.sin_addr.s_addr = htonl(kLocalRxIpAddress);

    if (setsockopt(sock_dat, SOL_SOCKET, SO_REUSEADDR, (char *) &optVal, sizeof(optVal)) != 0)
	{
		printf ("setsockopt SO_REUSEADDR error:  %s \n"
						, strerror(errno));
	}

	// Bind to the DATA port
    if (bind(sock_dat, (struct sockaddr *)&sin_dat, sizeof(sin_dat)) < 0)
    {
        printf("EstbDsgRemoteInterfaceDataHelperThread(): Error binding sock_dat_in!\n");
		goto err_bind_sock_dat;
    }

	#if (defined (USE_MULTICAST_SOCKET))
		JoinMulticastSession( sock_dat );
	#endif

	//Allocate buffer to receive DSG Packet.
    if( (dsgPacketBuffer = (char *)malloc(buflen)) == (char *)NULL)
	{
		printf("Can not allocate buffer %d\n", buflen);
		goto err_malloc;
	}

    printf("Starting eSTB DataHelperThread. Opening sockets and listening on ports %04x..\n", ESTB_DSG_DATA_PORT );

	maxfdp1 = sock_dat + 1;

	while(1)
	{
		FD_ZERO(&rfds);
		FD_SET(sock_dat, &rfds);

		retcode = select(maxfdp1, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);

		if (retcode == -1)
		{
			printf("select(): %s\n", strerror(errno));
		}
		else
		{
			// Listen for DATA
			if(FD_ISSET(sock_dat, &rfds))
			{
				inetsize = sizeof(frominet);
				// recvfrom shoud not block now
				cnt = recvfrom(sock_dat, dsgPacketBuffer, buflen, 0, (struct sockaddr*)&frominet, &inetsize);
				if( (cnt > 0) )
					//&& ( frominet.sin_addr.s_addr == htonl(kRemoteEcmSrcIpAddress)) )
				{
					//printf("DATA PKT from [ %s ]: with len = %d on port = %02x \n", inet_ntoa(frominet.sin_addr), cnt, ESTB_DSG_DATA_PORT);

					/**************** WRAP THE BUFFER IN ETHERNET PACKET STRUCTURE **********/

						pEthernetPacketBuffer = WrapEthernetPacket(dsgPacketBuffer, cnt);
						if( pEthernetPacketBuffer != NULL )
						{
							if( oobMode == kOobModeDsg )
							{
								if(displayMode & kSebLevel2)
									LogRawBuffer ("SEB TEST: DATA PKT:", pEthernetPacketBuffer->pDataBufferHead->pvStart, pEthernetPacketBuffer->pDataBufferHead->uiLen, 0);
								// Pass the DATA to DSG-CC.
								// DsgTunnelPkt is a VENDOR method.
								// eSTB is RESPONSIBLE for releasing the buffers
								// by calling DsgFreeEthernetPkt()
								if( DsgTunnelPkt( pEthernetPacketBuffer ) == false)
								{
									// eSTB did NOT consume the buffer.
									// We have to release the buffers NOW.
									DsgFreeEthernetPkt(pEthernetPacketBuffer);
								}
							}
							#if (BFC_INCLUDE_DAVIC_API_SUPPORT)
							else if(oobMode == kOobModeDavic )
							{
								if( BcmDavicRxTunnelPkt( pEthernetPacketBuffer ) == false)
								{
									// eSTB did NOT consume the buffer.
									// We have to release the buffers NOW.
									DsgFreeEthernetPkt(pEthernetPacketBuffer);
								}
							}
							#endif
						}
					/*************************************************************************/
				}
				//else
				//	printf("DATA PKT from [ %s ]:ERROR- Dropping packet. recvfrom returns %d %s\n", inet_ntoa(frominet.sin_addr), cnt, strerror(errno));
			}
		}
	}
	printf("DsgRemoteInterfaceDataHelperThread fell through!\n");

    free(dsgPacketBuffer);

err_malloc:
err_bind_sock_dat:
    close(sock_dat);
err_create_sock_dat:
	return;
    #endif
}

///////////////////////////////////////////////////////////////////
/*
 * This Thread will listen on socket (0x4247) for UDP packets
 * to receive eCM status notifications and DCD-MGMT messages
 * and call the appropriate API
 */
void EstbDsgRemoteInterfaceCtrlHelperThread ( void )
{
	struct sockaddr_in sin_ctl;

    register int cnt;
	struct sockaddr_in frominet;
	socklen_t      inetsize;
	char              *buf;					/* ptr to dynamic buffer */
	int		buflen  = 1518;					/* length of buffer */

	int sock_ctl, maxfdp1;
	fd_set rfds;
	int retcode;
	int optVal;

	#if defined (USE_MULTICAST_SOCKET)
	// Wait to acquire an IP address to
	// setup multicast sockets.
	uint32 ipAddress = 0;
	while( GetEstbIpAddress(&ipAddress) == 0 )
	{
		// Wait
		osSleep( 5 );
	}
	#endif

	// Create NOTIFICATION Socket
    if ((sock_ctl = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0)) < 0)
    {
        printf("EstbDsgRemoteInterfaceCtrlHelperThread(): Error creating sock_ctl!\n");
		goto err_create_sock_ctl;
    }

	int rc, sizectltx,sizectlrx, szlen = sizeof(int);
    rc = getsockopt(sock_ctl, SOL_SOCKET, SO_SNDBUF, (void *)&sizectltx, (socklen_t*)&szlen);
	if (rc == 0) printf("%s(%d): sock_ctl %d has SO_SNDBUF size=%d (0x%x)\n", __FUNCTION__, __LINE__, sock_ctl, sizectltx, sizectltx);
	rc = getsockopt(sock_ctl, SOL_SOCKET, SO_RCVBUF, (void *)&sizectlrx, (socklen_t*)&szlen);
	if (rc == 0) printf("%s(%d): sock_ctl %d has SO_RCVBUF size=%d (0x%x)\n", __FUNCTION__, __LINE__, sock_ctl, sizectlrx, sizectlrx);

	// Configure NOTIFICATION address and port
	bzero((char*)&sin_ctl, sizeof(sin_ctl));
    sin_ctl.sin_family = AF_INET;
	sin_ctl.sin_port = htons(ESTB_DSG_REQUEST_PORT);
	sin_ctl.sin_addr.s_addr = htonl(kLocalRxIpAddress);

	// REUSE ADDRESS
    optVal = 1;

    if (setsockopt(sock_ctl, SOL_SOCKET, SO_REUSEADDR, (char *) &optVal, sizeof(optVal)) != 0)
	{
		printf ("setsockopt SO_REUSEADDR error:  %s \n"
						, strerror(errno));
	}

	// Bind to the NOTIFICATION port
    if (bind(sock_ctl, (struct sockaddr *)&sin_ctl, sizeof(sin_ctl)) < 0)
    {
        printf("EstbDsgRemoteInterfaceCtrlHelperThread(): Error binding sock_ctl_in!\n");
		goto err_bind_sock_ctl;
    }


	#if (defined (USE_MULTICAST_SOCKET))
		JoinMulticastSession( sock_ctl );
	#endif

	//Allocate buffer to receive NOTIFICAION.
    if( (buf = (char *)malloc(buflen)) == (char *)NULL)
	{
		printf("Can not allocate buffer %d\n", buflen);
		goto err_malloc;
	}

    printf("Starting eSTB CtrlHelperThread. Opening sockets and listening on ports %04x...\n", ESTB_DSG_REQUEST_PORT );

	maxfdp1 = sock_ctl + 1;

	while(1)
	{
		FD_ZERO(&rfds);
		FD_SET(sock_ctl, &rfds);

		retcode = select(maxfdp1, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);

		if (retcode == -1)
		{
			printf("select(): %s\n", strerror(errno));
		}
		else
		{
			// Listen for NOTIFICATIONS
			if(FD_ISSET(sock_ctl, &rfds))
			{
				// Configure NOTIFICATION and DATA sockets to receive packets with eCM address.
				inetsize = sizeof(frominet);

				// recvfrom should not block now
				cnt = recvfrom(sock_ctl, buf, buflen, 0, (struct sockaddr*)&frominet, &inetsize);
				if( (cnt > 0) )
					//&& ( frominet.sin_addr.s_addr == htonl(kRemoteEcmSrcIpAddress)) )
				{
					// Pass the notification to DSG-CC.
				    // printf("NOTIFICATION from [ %s ]: with len = %d on port = %02x \n", inet_ntoa(frominet.sin_addr), cnt, sin_ctl.sin_port);
					if(displayMode & kLevel4)
						LogRawBuffer ("Notification: RCVD: ", (uint8*)buf, cnt, 0);

					// Process the request. Contents of "buf" will be modified when ProcessDsgClientPacketFromHalif()
					// returns. ProcessDsgClientPacketFromHalif will read the request and place the response
					// in the same buffer
					ProcessEcmNotifications( buf );
				}
			}
		}
	}
	printf("DsgRemoteInterfaceCtrlHelperThread fell through!\n");

    free(buf);

err_malloc:
err_bind_sock_ctl:
	close(sock_ctl);
err_create_sock_ctl:
	return;
}

int ProcessDavicApiSocketTx( uint8 apiType, unsigned int apiParam, uint32 apiLen, uint8* apiBuffer, bool readFromBuffer)
{
	unsigned char *DavicConfig;
	unsigned int bufferLen;
	int ret_val = kFuncFail;
	uint32 tempVal = 0;

	// DEBUG
	if(displayMode & kDavicLevel2)
		LogRawBuffer ("ProcessDavicApiSocketTx:: apiBuffer: ", apiBuffer, apiLen, 0);

    pthread_mutex_lock(&LnxDsgApiMutex);

	// TransId (1 byte) + kDavicApi (1 byte) + apiType (1 byte) + reserved (1 byte) = 4 bytes
	// apiParam (4 bytes)
	// apiLen (4 bytes)
	bufferLen = 4 + 4 + 4 + apiLen;

    // Pad out the buffer to minimum expected reply size.
    if (bufferLen < kEcmReplyPktSize)
        bufferLen = kEcmReplyPktSize;

	// Allocate buffer.
    if( (DavicConfig = (unsigned char *)malloc(bufferLen)) == NULL)
	{
		printf("Can not allocate buffer %d\n", bufferLen);
        pthread_mutex_unlock(&LnxDsgApiMutex);
		return kFuncFail;
	}

    //Reset memory.
    memset( DavicConfig, 0, bufferLen);

    DavicConfig[0] = ++TransId;
    DavicConfig[1] = kDavicApi;
    DavicConfig[2] = apiType;
	//DavicConfig[3] = 0;

	if(displayMode & kDavicLevel1)
		LogInfoMsg2Parm("ProcessDavicApiSocketTx", "Allocated buffer %d bytes for DAVIC API type: %d\n", bufferLen, DavicConfig[2] );

	if(apiLen)
	{
		tempVal = HostToNetworkUint32(apiParam);
		memcpy( &DavicConfig[4], &tempVal, sizeof(uint32) );

		tempVal = HostToNetworkUint32(apiLen);
		memcpy( &DavicConfig[8], &tempVal, sizeof(uint32) );

		if( apiLen == sizeof(uint32))
		{
			//printf("ProcessDavicApiSocketTx:apiBuffer[0]=%x [1]=%x [2]=%x [3]=%x\n", apiBuffer[0], apiBuffer[1], apiBuffer[2], apiBuffer[3]);
			//tempVal = HostToNetworkUint32((uint32)apiBuffer[3]);
			//memcpy( &DavicConfig[12], &tempVal, sizeof(uint32) );
			DavicConfig[12] = apiBuffer[3];
			DavicConfig[13] = apiBuffer[2];
			DavicConfig[14] = apiBuffer[1];
			DavicConfig[15] = apiBuffer[0];

		}
		else
			memcpy( &DavicConfig[12], apiBuffer, apiLen );
	}

	// DEBUG
	if(displayMode & kDavicLevel2)
		LogRawBuffer ("ProcessDavicApiSocketTx:: Sending ", DavicConfig, bufferLen, 0);

    ret_val = EstbToEcmSocketSendRx( bufferLen, DavicConfig, true, readFromBuffer);

	// DEBUG
	if(displayMode & kDavicLevel2)
		LogRawBuffer ("ProcessDavicApiSocketTx:: Received ", DavicConfig, bufferLen, 0);

	// Make DAVIC API return values compatible with BRCM eCM-eSTB socket API.
	// BRCM eCM-eSTB socket API expects DavicConfig[2] to be 1 for success.
	// For versions earlier than 104b3 eCM returns 0 for success for DAVIC API response.
	if( ecmApiVersion < 0x104b3 )
	{
		if( (ret_val == 0) )
		{
			// zero:success
			if( DavicConfig[2] == 0 )
				ret_val = kFuncSuccess;
		}
	}

	if( ret_val == kFuncSuccess )
	{
		// DEBUG
		if(displayMode & kDavicLevel2)
			LogInfoMsg2Parm("ProcessDavicApiSocketTx", "apiType %d successful TransId=%d\n", apiType, TransId );

		if( readFromBuffer )
		{
			memcpy(apiBuffer, &DavicConfig[12], apiLen);

			// DEBUG
			if(displayMode & kDavicLevel2)
				LogRawBuffer ("ProcessDavicApiSocketTx:: Read apiBuffer: ", apiBuffer, apiLen, 0);
		}
	}

	free(DavicConfig);
    pthread_mutex_unlock(&LnxDsgApiMutex);

	return ret_val;
}
#endif

uint32 HostToNetworkUint32(uint32 value)
{
    uint32 newValue;

    newValue = HostToNetworkUint16((uint16) (value & 0xffff));
    newValue <<= 16;
    newValue |= HostToNetworkUint16((uint16) ((value >> 16) & 0xffff));

    return newValue;
}

uint16 HostToNetworkUint16(uint16 value)
{
    uint16 newValue;

    newValue = (value >> 8) & 0x00ff;
    newValue |= (value << 8) & 0xff00;

    return newValue;
}

void SetOobMode(uint32 mode)
{
	oobMode = mode;
}

void SetDisplayMode(DisplayMode mode)
{
	displayMode = mode;

	LogInfoMsg1Parm("", "display mode = %d", (unsigned int)displayMode);
}

void CheckApiVersion(void)
{
	uint32 learnedEcmApiVersion = ecmApiVersion;

	if( SetApiVersion(&ecmApiVersion) == kFuncFail )
	{
		// Cover the case where eCM previously resonded to SetApiVersion() call but failing
		// to do so due to communication problems.
		if( (learnedEcmApiVersion != 0) && (ecmApiVersion == 0) )
		{
			ecmApiVersion = learnedEcmApiVersion;
			printf("CheckApiVersion: WARNING! ******** eCM does not respond to SetApiVersion: Current ecmVersion 0x%08x!\n"
				   , (unsigned int)ecmApiVersion);
		}
		else
		{
			printf("CheckApiVersion: WARNING! ******** eCM does not respond to SetApiVersion: ecmVersion 0x%08x!\n"
				   "Never learned eCM version. \n", (unsigned int)ecmApiVersion);
			ecmHasOlderApiVers = true;
		}
	}
	else
	{
		if( ecmApiVersion < kDsgApiVersion )
		{
			printf("CheckApiVersion: WARNING! ******** eCM API Version: %08x < eSTB API Version: 0x%08x! **********\n", (unsigned int)ecmApiVersion, (unsigned int)kDsgApiVersion);
			ecmHasOlderApiVers = true;
		}
	}
}

#if (kDefaultValue_TaggingEnabled)
bool ConfigureTaggingMode(bool onoff)
{
	struct ifreq ifr;
	struct sockaddr_in *sin = (struct sockaddr_in *)&(ifr.ifr_addr);
	int retcode = 0;
	int socketDescriptor = 0;

	/* Set the socket options */
	memset(&ifr, 0, sizeof (struct ifreq));
	strcpy(ifr.ifr_name, EstbPrimaryIpStackName);
	sin->sin_family = AF_INET;
	sin->sin_port = 0;

	/* Create the socket. Bail if we're not able to create one */
	socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
	if (!socketDescriptor)
	{
		/* We weren't able to create a socket - something must be
		 * seriously messed up.  Print error message and bail */
		perror("ConfigureTaggingMode: Failed to created socket");
		return false;
	}

	if (onoff)
	{
		retcode = ioctl(socketDescriptor, SIOCTAGGINGON, &ifr);
	}
	else
	{
		retcode = ioctl(socketDescriptor, SIOCTAGGINGOFF, &ifr);
	}

	close(socketDescriptor);

	/* Now check to see if ioctl worked */
	if (retcode)
	{
		perror("ConfigureTaggingMode: Ioctl error:");
		return false;
	}
	else
	{
		return true;
	}
}
#endif


#if (BFC_INCLUDE_DNAC_SUPPORT)
int ErouterEnable ( bool enable )
{
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
    unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

    if (enable != 1 && enable != 0)
    {
        printf("+++ ErouterEnable(), Wrong argument!\n");
        return 	kFuncFail;
    }

	uint32 version = htonl(kDsgApiVersion);

    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kErouterApi;
    DsgConfig[2] = kErouterEnable;
    DsgConfig[3] = enable;

	// Copy API version
	memcpy(&DsgConfig[4], &version, sizeof(uint32));

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable eRouter command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

#else

	return ret_val;
#endif

}

int ErouterWebIntfcEnable ( bool enable )
{
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
    unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

    if (enable !=1 && enable != 0)
    {
        printf("+++ ErouterWebIntfcEnable(), Wrong argument!\n");
        return 	kFuncFail;
    }

	uint32 version = htonl(kDsgApiVersion);

    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kErouterApi;
    DsgConfig[2] = kErouterWebIntfcEnable;
    DsgConfig[3] = enable;

	// Copy API version
	memcpy(&DsgConfig[4], &version, sizeof(uint32));

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable eRouter Web UI command again!\n");
        iteration--;
    }
    }
#endif

    pthread_mutex_unlock(&LnxDsgApiMutex);

    return ret_val;

#else

	return ret_val;
#endif


}

int ErouterIpv6AddressAssign ( int assign_method )
{
#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
    unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif

    if (assign_method > 3 || assign_method < 1)
    {
        printf("+++ Wrong IP assign mode!\n");
        return 	kFuncFail;
    }

	uint32 version = htonl(kDsgApiVersion);

    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kErouterApi;
    DsgConfig[2] = kErouterIpv6AddressAssign;
    DsgConfig[3] = assign_method;

	// Copy API version
	memcpy(&DsgConfig[4], &version, sizeof(uint32));

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, false);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send Enable eRouter IPV6 Address assign method command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);
    return ret_val;

#else

	return ret_val;
#endif



}

int ErouterStatus (t_erouter_status *pStatus)
{
	unsigned char DsgConfig[64];
    int ret_val;
#if (DSG_RECOVER_SUPPORT)
	uint32 iteration = 5;
	unsigned char xid;
#endif
    pthread_mutex_lock(&LnxDsgApiMutex);

#if (DSG_RECOVER_SUPPORT)
    xid = ++TransId;

    while (iteration > 0)
    {
#endif
    //Reset memory.
    memset( DsgConfig, 0, 64);

#if (DSG_RECOVER_SUPPORT)
    DsgConfig[0] = xid;
#else
    DsgConfig[0] = ++TransId;
#endif
    DsgConfig[1] = kErouterApi;
    DsgConfig[2] = kGetErouterStatus;

	// Send the message
    ret_val = EstbToEcmSocketSendRx( 64, DsgConfig, true, true);

#if (DSG_RECOVER_SUPPORT)
    if (ret_val > 0)
    {
        break;
    }
    else
    {
        printf("+++ Send get eRouter status command again!\n");
        iteration--;
    }
    }
#endif
    pthread_mutex_unlock(&LnxDsgApiMutex);

    if (ret_val <= 0)
	    return ret_val;

    pStatus->erouter_enabled   = DsgConfig[3];
    pStatus->web_intfc_enabled = DsgConfig[4];
    pStatus->ipv6_address_assignment_method = DsgConfig[5];
    memcpy( &(pStatus->snmpIpv4Addr[0]),&DsgConfig[6],4);

    switch ( DsgConfig[10] )
    {
        case 0:
        {
            strcpy(pStatus->txt, "Erouter is running in keRouterDisabled mode\n");
            break;
        }
        case 1:
        {
            strcpy(pStatus->txt, "Erouter is running in keRouterIpv4ModeOnly mode\n");
            break;
        }
        case 2:
        {
            strcpy(pStatus->txt, "Erouter is running in keRouterIpv6ModeOnly mode\n");
            break;
		}
        case 3:
        {
            strcpy(pStatus->txt, "Erouter is running in keRouterDualMode mode\n");
            break;
		}
        case 4:
        {
            strcpy(pStatus->txt, "Erouter is running in keRouterIpv4RGModeOnly mode\n");
            break;
		}

        default:
        {
		    printf("Returned an invalid status\n");
            break;


			return false;
        }
    }

    return ret_val;
}
#endif

/*
char* GetEnvVarEcmInterface(void)
{
    char env_name[] = kEnvVar_EcmInterfaceName;
    char *env_value;

    env_value = getenv(env_name);
    if (env_value)
	{
        printf("\neCM interface is read from env var  %s: <%s>\n", env_name, env_value);
		return env_value;
    }
    else
	{
        printf("There is no environment variable named <%s>\n", env_name);
    }

    return NULL;
}
*/

/*
  Example code to obtain IP and MAC for all available interfaces on Linux.
  by Adam Pierce <adam@doctort.org>

http://www.doctort.org/adam/

*/
int FindEcmInterface(char *pIpcInterfaceName)
{
	char          buf[1024];
	struct ifconf ifc;
	struct ifreq *ifr;
	int           sck;
	int           nInterfaces;
	int           i;
	struct sockaddr_in *sa = NULL;

	/* Get a socket handle. */
	sck = socket(AF_INET, SOCK_DGRAM, 0);
	if(sck < 0)
	{
		perror("socket");
		return 1;
	}

	/* Query available interfaces. */
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if(ioctl(sck, SIOCGIFCONF, &ifc) < 0)
	{
		perror("ioctl(SIOCGIFCONF)");
		return 1;
	}

	/* Iterate through the list of interfaces. */
	ifr         = ifc.ifc_req;
	nInterfaces = ifc.ifc_len / sizeof(struct ifreq);
	for(i = 0; i < nInterfaces; i++)
	{
		struct ifreq *item = &ifr[i];

		/* Show the device name and IP address */
		sa = (struct sockaddr_in *)(&item->ifr_addr);
		printf("%s: IP %s \n",
			   item->ifr_name,
			   inet_ntoa(sa->sin_addr));

		if( htonl(sa->sin_addr.s_addr) == kLocalRxIpAddress )
		{
			//printf("%s  %08x\n", iap->ifa_name, htonl(sa->sin_addr.s_addr));
			memcpy( pIpcInterfaceName, item->ifr_name, IPC_INTERFACE_NAME_MAX_LEN);
			return 1;
		}
	}

        return 0;
}
