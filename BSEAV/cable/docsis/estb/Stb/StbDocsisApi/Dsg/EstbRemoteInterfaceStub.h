/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef ESTB_RMT_INTERFACE_STUB_H
#define ESTB_RMT_INTERFACE_STUB_H

//********************** Include Files ***************************************
#if defined __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"
#include "EnetPkt.h"
#include "dsg_api.h"

#if defined (COMMON_DOWNLOAD_SUPPORT)
#include "EstbCommonDownloadInterface.h"
#endif

/*  Helper Function returns the IP address that is currently associated with the IP stack
 *  for this HalIf and the interface name passed in.
 *  
 *  Multiple IP addresses in Linux is not supported at this time.
 *  
 *  Added this method for PR2256.
 *  
 *   Parameters:
 *        pEstbIpAddress - the IP address will be returned in this parameter.
 * 
 *   Returns:
 *        true if the IP address was successfully querried.
 *       false if there was a problem acquiring the IP address.
 */
bool GetEstbIpAddress(uint32* pEstbIpAddress);
bool GetEstbIpAddressIPV6(void);

/*
 * This Thread will listen on socket (0x4247) for UDP packets
 * to receive eCM status notifications and DCD-MGMT messages
 * and call the appropriate API 
 */
void EstbDsgRemoteInterfaceHelperThread ( void );

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
ETHERNET_PACKET* WrapEthernetPacket( char *buf, uint32 length );

void ProcessEcmNotifications(char * buf);

// Error codes for SetupRemoteInterface()
typedef enum
{
    kRIsetupNoError			= 1,       
    kRIsetupTxSockFailed		= 2,             
    kRIsetupTxSockBindFailed	= 3,              
    kRIsetupRxSockFailed		= 4,              
    kRIsetupRxSockBindFailed	= 5,
	kRIsetupEcmNotResponding	= 6

} SetupRemoteInterfaceReturnCodes;

// Set up the sock interface for the test thread
SetupRemoteInterfaceReturnCodes SetupRemoteInterface(char *pIpcInterfaceName);
char* GetEnvVarEcmInterface(void);
int FindEcmInterface(char *pIpcInterfaceName);

bool JoinMulticastSession( int socketDescriptor );

bool SetApiVersion(uint32 *pEcmVersion);
void CheckApiVersion(void);

#if (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
void ConfigureEcmInterface(void);
void SetMulticastInterface(void);
void CheckNewEstbPrimaryIpV4Address( bool forgetCurrIpAddr );
void PrintNewEstbPrimaryIpAddress( void );
void FlushRxSocketBuffer( void);
#endif

#if (CABLECARD_SUPPORT)
// For cablecard IP_U flow that needs an IP address by using DHCP client in the eCM
int DsgStartProxyDhcpRequest( unsigned char opt_len, unsigned char *mac, unsigned char *opt_data, uint32 ipaddress); //unsigned char *ret_option )
int DsgShutDownProxyIpuStack( void );
int DsgSendCableCardIpUsData( unsigned char *pData, unsigned short len );
#endif

#if defined(ESTB_SWDLOAD_SUPPORT)
STATUS swdl_tudp_tx( int tpkt_size, unsigned char *pBuf, unsigned short port);
#endif

#if defined (COMMON_DOWNLOAD_SUPPORT)
int CommonDownloadProcessEstbRequest(uint8 apiType, uint32 apiLen, uint8* apiBuffer, CommonDownloadResponseMsg* pCommonDownloadResponseMsg);
#endif

void LogRawBuffer (char * str, uint8 *pBuffer, int BufLen, BOOL wacky);

#if (BFC_INCLUDE_DAVIC_API_SUPPORT) 
int ProcessDavicApiSocketTx( uint8 apiType, unsigned int apiParam, uint32 apiLen, uint8* apiBuffer, bool readFromBuffer);
#endif

typedef enum
{
	kOobModeNone,
	kOobModeDsg,
	kOobModeDavic
}OobModeType;

void SetOobMode(uint32 mode);


#if (BFC_INCLUDE_SEB_API_SUPPORT) 
int ProcessSebApiSocketTx( uint8 apiType, unsigned int apiParam, uint32 apiLen, uint8* apiBuffer, bool readFromBuffer);
void CheckSebApiVersion(void);
#endif

// These are done as macros for compile efficiency.  They are helpers for doing
// debug logging.
typedef enum
{
    kLevel0			= 0x00000000,              
    kLevel1			= 0x00000001,              
    kLevel2			= 0x00000002,              
    kLevel3			= 0x00000004,              
    kLevel4			= 0x00000008,
	kDsg			= 0x00000010,
    kCdl			= 0x00000020,
	kDavic			= 0x00000040,
	kDavicLevel1	= 0x00000080,
	kDavicLevel2	= 0x00000100,
	kDavicVendor	= 0x00000200, 	
	kSebLevel1		= 0x00000400,
	kSebLevel2		= 0x00000800
} DisplayMode;

void SetDisplayMode(DisplayMode mode);

bool ConfigureTaggingMode(bool onoff);

void HtonDsgSetChannelList (uint16 *pNum_chans, uint32 *pChannel_list);

#define HalPrint(s...)                printf(s)

#if ( BCM_CONSOLE_SUPPORT )
    #define LogRawMsg(pFunctionName, pErrorCode, pMessageString) HalPrint("ProgramStoreDeviceDriver::%s:  %s - %s\n", pFunctionName, pErrorCode, pMessageString)
    #define LogRawMinimal(pMessageString) HalPrint(pMessageString); fflush(stdout)
#else
    // For the 'slim' build, disable all printing.
    #define LogRawMsg(pFunctionName, pErrorCode, pMessageString)
    #define LogRawMinimal(pMessageString)
#endif

#define ENABLE_INFO 1
#define ENABLE_INFO_DETAILED 1
#define ENABLE_STATUS 1
#define ENABLE_WARNING 1
#define ENABLE_ERROR 1

#if ENABLE_INFO
	#define LogInfoMsg(pFunctionName, pMessageString) \
        HalPrint("%s:  %s - " pMessageString "\n", pFunctionName, "INFO")
#else
	#define LogInfoMsg(pFunctionName, pMessageString)
#endif

#if ENABLE_INFO_DETAILED
	#define LogInfoMsg1Parm(pFunctionName, pMessageString, parm1) \
        HalPrint("%s:  %s - " pMessageString "\n", pFunctionName, "INFO", parm1)
	#define LogInfoMsg2Parm(pFunctionName, pMessageString, parm1, parm2) \
        HalPrint("%s:  %s - " pMessageString "\n", pFunctionName, "INFO", parm1, parm2)
    #define LogInfoMsg3Parm(pFunctionName, pMessageString, parm1, parm2, parm3) \
        HalPrint("%s:  %s - " pMessageString "\n", pFunctionName, "INFO", parm1, parm2, parm3)
#else	
	#define LogInfoMsg1Parm(pFunctionName, pMessageString, parm1)
	#define LogInfoMsg2Parm(pFunctionName, pMessageString, parm1, parm2)
    #define LogInfoMsg3Parm(pFunctionName, pMessageString, parm1, parm2, parm3)
#endif

#if ENABLE_STATUS
    #define LogStatusMsg(pMessageString) LogRawMinimal(pMessageString)
#else
    #define LogStatusMsg(pMessageString)
#endif

#if ENABLE_WARNING
    #define LogWarningMsg(pFunctionName, pMessageString) LogRawMsg(pFunctionName, "WARNING", pMessageString)
#else
    #define LogWarningMsg(pFunctionName, pMessageString)
#endif
        
#if ENABLE_ERROR
    #define LogErrorMsg(pFunctionName, pMessageString) LogRawMsg(pFunctionName, "ERROR", pMessageString)
#else
    #define LogErrorMsg(pFunctionName, pMessageString)
#endif        


#if defined __cplusplus
	}
#endif


#endif
