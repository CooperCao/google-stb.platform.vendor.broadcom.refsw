//****************************************************************************
//
// Copyright (c) 2003-2013 Broadcom Corporation
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
//  Filename:       dsg_api.h
//  Author:         
//  Creation Date:  April 24, 2001
//
//****************************************************************************
//  Description: 
//                  
//
//****************************************************************************

#ifndef DSGAPI_H
#define DSGAPI_H

//********************** Include Files ***************************************
#if defined __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"
#include "EnetPkt.h"

/* Please note that DUAL PROCESS case is obsolete and not supported */
#if ( BCM_DSG_DUAL_PROCESSOR_INTERFACE )
#include "ecm_estb_net_if.h"
#endif

//#if (SETTOP_API_INCLUDED)
	#if defined(__linux__)  
		#include <LnxInterface.h>
	#elif defined(TARGETOS_vxWorks)
		#include "VxInterface.h"
	#elif defined(WIN32)
		#include "Win32Interface.h"
	#endif
//#endif
//********************** Global Types ****************************************

#define kDsgApiVersion	0x104b5

#if (BFC_INCLUDE_TAGGING_CHECK)
#define kDefaultValue_TaggingEnabled true
#else
#define kDefaultValue_TaggingEnabled false
#endif
typedef
void (*PFN_DSG_DCD_BYPASS_HANDLER) ( uint8 *pDcdTlv, unsigned int DcdLen );

// ++++++++++++++++++ ATTENTION: THIS IS MOVED TO ecm_estb_net_if.h file !!!!!!!!!! +++++++++++++++++++++++++++++++++++++++ //


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@	
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ eCM - eSTB COMMUNICATION CONFIGURATION @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@	
/*#if ( BCM_DSG_DUAL_PROCESSOR_INTERFACE )

	#define kDefault_eStbSocketMaxRxBufferSize 6144
	#define kDefaultValue_EcmComAttemptMaxCount 30

	// Default eCM-eSTB communication method uses MULTICAST sockets. You can search for
	// USE_MULTICAST_SOCKET to trace it.
	// An optional method uses a private network between eCM and eSTB. This is a build option (pvnet)
	// You can search for USE_PRIVATE_NETWORK to trace it.
	// If none is selected, eSTB will get its IP address from DHCP server running on IP Stack 2.

	// eCM-eSTB interface
	#define USE_USB_INTERFACE
	//#define USE_ETH_INTERFACE

	#if defined(TARGETOS_vxWorks)
		#define kDefaultEcmInterfaceName			"bcmEnet0"					
	#else

		//#define kEnvVar_EcmInterfaceName			"ECM_IF"

		#define IPC_INTERFACE_NAME_MAX_LEN  16	// including the null character
					
		#if defined(USE_INTERFACE_ETH2)
			#define kDefaultEcmInterfaceName		"eth2:0"
		#else					
			#define kDefaultEcmInterfaceName		"eth0:0"
		#endif					
	#endif

	#if defined(USE_MULTICAST_SOCKET)

		#define kLocalRxIpAddress		0xE00001FF				// 224.0.1.255    JOIN MULTICAST SESSION TO RECEIVE PACKETS
		#define kRemoteEcmIpAddress		0xE00001FE				// 224.0.1.254	  SEND PACKETS TO MULTICAST ADDRESS AT eCM
		#define kRemoteEcmSrcIpAddress	0x0a0a0a01 //xC0A86401				// 192.168.100.1	  RECEIVE FROM

		#define kRemoteEcmMacAddress	{ 0x00, 0x10, 0x18, 0x00, 0xef, 0xdb }	

	#elif defined(USE_PRIVATE_NETWORK)

#if defined(IP_NONVOL_OVER_ETHERNET)
		#define kLocalRxIpAddress		0x0a18046a // 10.24.4.106 0x0a0a0a0a				// 10.10.10.10    STATIC IP ADDRESS OF eSTB
		#define kRemoteEcmIpAddress		0x0a1804de // 10.24.4.222 0x0a0a0a01				// 10.10.10.1     STATIC IP ADDRESS OF eCM
		#define kRemoteEcmSrcIpAddress  0x0a1804de // 10.24.4.222 0x0a0a0a01				// 10.10.10.1     RECEIVE FROM
#else
		#define kLocalRxIpAddress		0xC0A8110A				// 192.168.17.10    STATIC IP ADDRESS OF eSTB
		#define kRemoteEcmIpAddress		0xC0A81101				// 192.168.17.1     STATIC IP ADDRESS OF eCM
		#define kRemoteEcmSrcIpAddress  kRemoteEcmIpAddress
#endif
		
		#define kRemoteEcmMacAddress	{ 0x00, 0x10, 0x18, 0x00, 0xef, 0xdb }	

	#else

		// Unless otherwise specified, eSTB RI will communicate to Remote IP Stack on eCM.
		#define kLocalRxIpAddress			0						// Default case
		#define kRemoteEcmSrcIpAddress		0xC0A86401				// 192.168.100.1  RECEIVE FROM
		
		#define kRemoteEcmIpAddress			0xC0A86401 				// 192.168.100.1  SEND TO

	#endif

	// DUAL PROCESS CASE
#else
	#define kDefaultEcmInterfaceName			"lo0"					// loopback

	#define kLocalRxIpAddress			INADDR_LOOPBACK			// 127.0.0.1	  
	#define kRemoteEcmIpAddress			INADDR_LOOPBACK			// 127.0.0.1	  RECEIVE FROM
	#define kRemoteEcmSrcIpAddress		INADDR_LOOPBACK			// 127.0.0.1	  SEND TO
#endif

#if (BCM_CABLEHOME_SUPPORT)
	#define	kEstbIpStackNumber			8
#else
	#define	kEstbIpStackNumber			3
#endif

// These are the socket port numbers used for eCM-eSTB communication
#define ECM_DSG_REQUEST_PORT			0x4246		// Send commands to eCM
#define ESTB_DSG_RESPONSE_PORT			0x8887		// Wait for eCM response

#define ESTB_DSG_REQUEST_PORT			0x4247		// Wait on Notification from eCM

#define ESTB_DSG_DATA_PORT				0x8889		// Wait on DSG DATA

#define	kEcmReplyPktSize				16			// Size of the packet eCM uses to reply to eSTB requests.
 */
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

#define DSG_MAX_ADRS 32

#if defined(WIN32)
#define DsgPrint(s)              printf(s)
#else
#define DsgPrint(s...)              printf(s)
#endif

#ifndef __cplusplus
#if !defined (TARGETOS_eCos)
typedef int bool;
#define true 1
#define false 0
#endif
#endif
typedef struct
{
    unsigned char macaddress[6];
} t_mac_adr;

typedef struct 
{
    unsigned char number_of_adrs;
    /* although this structure includes the maximum number of adrs, this member tells how many are actually used */
    t_mac_adr adrs[DSG_MAX_ADRS];
    uint8 typ[DSG_MAX_ADRS];
} DsgTunnelAdrs;

typedef struct
{ 
	uint16 cfr_id;
	uint8 priority;
    uint8 unused;

	uint8 source_ip_used;
	uint8 source_ip_mask_used;
	uint8 dest_ip_used;
	uint8 port_used;
	uint32 source_ip_adr;
	uint32 source_ip_mask;
	uint32 dest_ip_adr;
	uint16 port_start;
	uint16 port_end; 
} t_dsg_classifier;
typedef struct
{
    t_mac_adr			*pTunnelAddress;
	uint32				clientIdType;
	uint32				clientIdValue;
	uint32				numOfClassifiers;
	t_dsg_classifier	*pClassifierList;
    t_mac_adr			*pMacAddressClientIdVal;
} DsgTunnelSettings;

typedef struct
{
    uint8 enableEStb;
	uint8 mode;
    uint8 docsisUs;
    DsgTunnelAdrs clientIds;
} DsgPersistentData;

typedef struct
{
	bool upstreamState;
	int   ipMode;
} DsgEcmUsSettings;

/* API functions provided by eSTB to eCM */
typedef
void (*PFN_DSG_SCAN_SUCCESS_HANDLER) ( void );

typedef
void (*PFN_DSG_FULLSCAN_FAIL_HANDLER) ( void );

typedef
void (*PFN_DSG_RX_DCDMSG_HANDLER) ( int mes_len, uint8 * );

typedef
void (*PFN_DSG_UPSTREAM_CHANGE_HANDLER) ( DsgEcmUsSettings *pDsgEcmUsSettings );

typedef
void (*PFN_DSG_USID_HANDLER) ( uint8 uc_id );

typedef
void (*PFN_DSG_TIMER_EVENT_HANDLER) ( uint8 tm_event );

typedef
bool (*PFN_DSG_RX_TUNNELDATA_HANDLER) ( ETHERNET_PACKET* );

typedef
void (*PFN_DSG_DCC_INIT_TYPE_HANDLER) ( uint8 dccInitType );

typedef
void (*PFN_DSG_ECM_MAC_REINIT_HANDLER) ( void );

typedef struct eSTB_DSG_CONTROL_INTERFACE {
    PFN_DSG_SCAN_SUCCESS_HANDLER            eSTBDsgScanSuccessHandler;
    PFN_DSG_FULLSCAN_FAIL_HANDLER           eSTBDsgFullScanFailHandler;
    PFN_DSG_RX_DCDMSG_HANDLER               eSTBDsgRxDcdMessageHandler;
    PFN_DSG_UPSTREAM_CHANGE_HANDLER         eSTBDsgUpstreamChangeHandler;
    PFN_DSG_USID_HANDLER                    eSTBDsgUsIdHandler;
    PFN_DSG_TIMER_EVENT_HANDLER             eSTBDsgTimerEventHandler;
    PFN_DSG_RX_TUNNELDATA_HANDLER           eSTBDsgRxTunnelDataHandler;
    PFN_DSG_DCC_INIT_TYPE_HANDLER           eSTBDsgDccInitTypeHandler;
	PFN_DSG_ECM_MAC_REINIT_HANDLER			eSTBDsgEcmMacReinitHandler;
} eSTB_DSG_API_INTERFACE;


/* API functions provided by eCM to eSTB */

typedef
int (*PFN_DSG_SET_MODE_HANDLER) ( int mode, BcmHandle handle );

typedef
bool (*PFN_DSG_ENABLE_DOCSIS_HANDLER) ( bool enable, BcmHandle handle );

typedef
bool (*PFN_DSG_SET_BASIC_TUNNEL_HANDLER) ( DsgTunnelAdrs * my_adrs, BcmHandle handle );

typedef
int (*PFN_DSG_SET_ADVANCED_TUNNEL_HANDLER) ( DsgTunnelSettings *pTunnelSettings, BcmHandle handle );

typedef
bool (*PFN_DSG_CLOSE_ADVANCED_TUNNEL_HANDLER) ( int tunnel_num, BcmHandle handle );

typedef
int (*PFN_DSG_SET_TIMER_HANDLER) ( uint8 timer_id, uint16 timeout, BcmHandle handle );

typedef
int (*PFN_DSG_GET_TIMER_HANDLER) ( uint8 timer_id, uint16 *timeout, BcmHandle handle );

typedef
bool (*PFN_DSG_SET_CHANNELS_LIST_HANDLER) ( uint16 num_chans, uint32 *channel_list, BcmHandle handle );

typedef
bool (*PFN_DSG_RESET_CHANNELS_LIST_HANDLER) ( BcmHandle handle );

typedef
bool (*PFN_DSG_SCAN_HANDLER) ( bool enable, BcmHandle handle );

typedef
int (*PFN_DSG_STATUS_HANDLER) ( BcmHandle handle );

typedef
int (*PFN_DSG_DOCSIS_STATUS_HANDLER) ( BcmHandle handle );

typedef
int (*PFN_DSG_TUNNELDATA_FREE_HANDLER) ( ETHERNET_PACKET*, BcmHandle handle );

typedef
bool (*PFN_DSG_REGISTER_eSTB_API_HANDLER) ( eSTB_DSG_API_INTERFACE *, BcmHandle handle );


typedef struct eCM_DSG_CONTROL_INTERFACE {
   unsigned long hMac;
   PFN_DSG_SET_MODE_HANDLER                 eCMDsgSetModeHandler;
   PFN_DSG_ENABLE_DOCSIS_HANDLER            eCMDsgEnableDocsisHandler;
   PFN_DSG_SET_BASIC_TUNNEL_HANDLER         eCMDsgSetBasicTunnelHandler;
   PFN_DSG_SET_ADVANCED_TUNNEL_HANDLER      eCMDsgSetAdvancedTunnelHandler;
   PFN_DSG_CLOSE_ADVANCED_TUNNEL_HANDLER    eCMDsgCloseAdvancedTunnelHandler;
   PFN_DSG_SET_TIMER_HANDLER                eCMDsgSetTimerHandler;
   PFN_DSG_GET_TIMER_HANDLER                eCMDsgGetTimerHandler;
   PFN_DSG_SET_CHANNELS_LIST_HANDLER        eCMDsgSetChannelsListHandler;
   PFN_DSG_RESET_CHANNELS_LIST_HANDLER      eCMDsgResetChannelsListHandler;
   PFN_DSG_SCAN_HANDLER                     eCMDsgScanHandler;
   PFN_DSG_STATUS_HANDLER                   eCMDsgStatusHandler;
   PFN_DSG_DOCSIS_STATUS_HANDLER            eCMDsgDocsisStatusHandler;
   PFN_DSG_TUNNELDATA_FREE_HANDLER          eCMDsgTunnelDataFreeHandler;
   PFN_DSG_REGISTER_eSTB_API_HANDLER        eCMDsgRegistereSTBApiHandler;
} eCM_DSG_API_INTERFACE;


//********************** Global Constants ************************************
enum
{
	kDsgDisabledMode = 0,
	kDsgBasicMode,
	kDsgAdvanceMode
};

enum
{
    kUndefined = 0,
    kTdsg1Timeout,
    kTdsg2Timeout,
    kTdsg3Timeout,
    kTdsg4Timeout,
    kTdsg4TimerStop
};

enum
{
    kFuncInvalid = -1,
    kFuncFail = 0,
    kFuncSuccess,
    kDocsisOnlyTestMode = 254
};

enum 
{
    kDhcpGotNewLease = 65,          //why 65? why not
    kDhcpLeaseInvalid,
    kReceivedDsgPacket, 
    kReceivedCmStatus
};


typedef enum
{
	kEcmReset					= 1,	//(eCM reset is detected via heartbeat )
    kEcmDocsDevResetIssued		= 2,	//(eCM is about to reset due to docsDevReset)
	kEcmSwUpgradeResetIssued	= 3		//(eCM upgraded its sw and is about to reset)
}EcmResetType;


#if (BFC_INCLUDE_DNAC_SUPPORT)
typedef struct
{ 
    int erouter_enabled; 
    int web_intfc_enabled; 
    int ipv6_address_assignment_method; // see arguments of ErouterIpv6AddressAssign() 
    unsigned char snmpIpv4Addr[4]; // eRouter IPv4 address used by the eSTB for SNMP queries in network byte order 
    char txt[41]; // zero-terminated ASCII string describing the general status of the eRouter 
} t_erouter_status;
#endif

//********************** Global Variables ************************************


//********************** Class Declaration ***********************************

enum
{
    kEventDsgScanSuccess = 0,
    kEventDsgFullScanFail,
    kEventDsgRxDCD,
    kEventDsgUpstreamChange,
    kEventDsgUcid,
    kEventDsgTimerEvent,
    kEventDsgDccInitType,
    kEventDsgTwoWayForwardingDisabled,

    kEventDsgCableCardDhcpReply,
    kEventDsgCableCardIpDsData,
    kEventDocsisIsResetting,

	// Common Download
	kEventDsgCommonDownloadNotification,

	kEventLogEvent,

	kEventDavicNotification,

    kEventEcmHeartBeat,

    kEventDsgEcmEventNotification,

	kEventDsgEcmMacReinit

	// TEST
	,kEventSebNotification	= 99
};

//API code: These enum should be the same as the Dsg Host defines
enum
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
    kDsgCableCardDhcpReq,
    kDsgCableCardIpUsData, 
    kDsgRemoteLnaGain,
    kDsgCableCardShutDownIpu,
    kDsgGetEcmHeartBeat,

    // Common Download
    kCommonDloadProcessEstbReq,
    kCommonDloadAuthenticateOrUpgrade,
	kCommonDloadVerifyImageShaDigest,

    kDsgReportDsgError,
    kDsgSetEstbPrimaryAddr,
    kEsafeProvisioningStatus,
    kEsafeDevStatus,

	kDavicApi,

   	kDsgSetDhcpV4VendorSpecificOption,
	kDsgSetDhcpV6VendorSpecificOption,
	kDsgSetCCMac,

	kSetApiVersion,

#if (BFC_INCLUDE_DNAC_SUPPORT)
	kErouterApi,
#endif

	// IMPORTANT: Add new API below here and 
	// remember to move this comment after the new API

	// TEST
	kSebApi = 99
};

#if (BFC_INCLUDE_DNAC_SUPPORT)
// Constants for the commands.
enum
{
	kErouterEnable,
	kErouterWebIntfcEnable,
	kErouterIpv6AddressAssign,
	kGetErouterStatus
};
#endif

// "C" callable Dsg Host(eCM) API functions
int  DsgSetDSGMode( int mode );
bool DsgEnableDOCSIS (bool enable);

int  DsgOpenTunnel( DsgTunnelSettings *pTunnelSettings);
bool DsgCloseTunnel( int tunnel_num );
int  DsgSetTimer( uint8 timer_id, uint16 timeout );
int  DsgGetTimer( uint8 timer_id, uint16 *timeout );
bool DsgSetChannelList (uint16 num_chans, uint32 * channel_list);

bool DsgResetChannelList ( void ) ;
bool DsgScan ( bool enable );
int  DsgStatus( void );
int  DsgDocsisStatus( void );
int  DsgFreeEthernetPkt( ETHERNET_PACKET* pFreePkt );

bool DsgSetRIAdr( void );
int DsgGetEcmHeartBeat( uint16 *pHbeatCount );
int  DsgReportDsgError( uint8 error_status );
bool DsgSetEstbPrimaryIpV4Addr( uint32 ipAddr, uint8 *macAddr );
bool DsgSetEstbPrimaryIpV6Addr( uint8 *ipAddr, uint8 *macAddr );
bool DsgSetDhcpOption43(void *data, int dataLen);
bool DsgSetDhcpV6Option17(void *data, int dataLen);
bool DsgSetCCMacAddr (uint8 * macAddr);
bool SetEsafeDevStatus( uint8 intImpact, char *intImpactInfo );
bool SetEsafeProvisioningStatus( uint8 progress, uint8 failureFound, uint32 eventId );
bool SetEsafeProvisioningStatusAndFailure( uint8 progress, uint8 failureFound, uint32 eventId, uint8 provFailureFlowLen, char *provFailureFlow,
																				               uint8 provErrorTextLen, char* provErrorText);

// Dsg Client functions

// This is the main published entry funciton for the C++ code to call and preform init 
// and functions exchange and registeration
//
void DsgClientFuncInterfaceEntry( eCM_DSG_API_INTERFACE *pDsgApiInterface);

// Dsg Client functions

// The eCM calls this function to notify the eSTB that the eCM has locked on a downstream 
// with DSG data. For Basic Mode, this means that the eCM has received traffic on at least one 
// of the configured DSG tunnels. For Advanced Mode, this means that the eCM has received a 
// DCD message.
//
// Parameters:  N/A
//
// Returns:  N/A
//
void DsgScanSuccess( void );

// The eCM calls this function to notify the eSTB that the eCM has scanned every downstream in 
// its spectrum (or channel list) without finding suitable DSG traffic. 
//
// Parameters:  N/A
//
// Returns:  N/A
//
void DsgFullScanFail( void );

// This method will send DCD msg encapsulated with UDP packet to MA via Halif interface. 
// Will wait for packet sent completion
//
// Parameters:
//      pData is the Dcd Mac msg pointer including the header
//      msg_len is the entire mac msg length
//
// Returns:
//     n/a
//
void DsgRxDCD( int msg_len, uint8 * pData );

// The eCM will call this function to notify the eSTB when DOCSIS registration is complete 
// (upstream_state = true) or when the upstream is lost (upstream_state = false).
//
// Parameters:
//		pDsgEcmUsSettings.upstreamState 	true (2-way mode)
//                  false (1-way mode)
//		pDsgEcmUsSettings.ipMode			0 (IPv4 only)
//											1 (IPv6 only)
//											2 (Both IPv4 and IPv6)
//
// Returns:  N/A
//
void DsgUpstreamChange( DsgEcmUsSettings *pDsgEcmUsSettings );

// When the eCM establishes DOCSIS registration, it will provide its Upstream Channel ID to 
// the eSTB using this function.
//
// Parameters:
//      uint8 uc_id
//
// Returns:  N/A
//
void DsgUcid( uint8 uc_id  );

// The eCM will call this function to notify the eSTB that a relevant timer event has occurred.
// The value event will be encoded as follows:
// Value	Event	Meaning
//  1	Tdsg1 timeout	Did not find DSG info on current channel - continuing scan
//  2	Tdsg2 timeout	DSG info lost from current channel - starting Tdsg4
//  3	Tdsg3 timeout	Beginning attempt to establish DOCSIS two way connection
//  4	Tdsg4 timeout	DSG info gone from current channel - starting downstream scan
//  5	Tdsg4 stop	    DSG info regained on current channel
//
// Parameters:
//      uint8 tim_event - 1-5 encosed above
//
// Returns:  N/A
//
void DsgTimerEvent( uint8 tm_event ); 

// This method is to forward DSG msg packet out to HalIf interface. 
// Will wait for packet sent completion
//
// Parameters:
//      ETHERNET_PACKET *pData
//
// Returns:
//      success or fail
//
bool DsgTunnelPkt( ETHERNET_PACKET *pEthPacket );

// When the eCM sends a "DCC-RSP (Depart)" message, the eCM must also send a 
// "DCC Depart, Initialization Type <IT>" (where IT = "DCC Initialization Type")
// message to the Client Controller.
//
// Parameters:
//      uint8 dccInitType
//
// Returns:  N/A
//
void DsgDccInitType( uint8 dccInitType );

// eCM will call this function if one of the eSTB MAC addresses cannot be supported for data forwarding 
// because of Max CPE limitations or NACO is configured to "off" in eCM configuration file (see [eDOCSIS]). 
// These provisioning limitations MUST be reported to DSG Client Controller after 2-Way notification.
//
// disabled_forwarding_type: specifies what type of eCM provisioning limitations impact eCM 2-Way
// forwarding. Values below are bit fields that can be ORed to indicate multiple conditions.
//
//	0x01 Network access disabled (NACO=0)
//	0x02 Max CPE limit exhausted
//	0x04 Forwarding interface administratively down
//
// Parameters:
//      uint8 disabled_forwarding_type
//		uint8 maxCpe
//
// Returns:  N/A
void DsgTwoWayForwardingDisabled( uint8 disabled_forwarding_type, uint8 maxCpe );

// This callback function will notify eSTB of eCM's reset status.
//
// Parameters:
//      uint8 eCmResetType - 1: kEcmReset (eCM reset is detected via heartbeat )
//		                     2: kEcmDocsDevResetIssued   (eCM is about to reset due to docsDevReset)
//							 3: kEcmSwUpgradeResetIssued (eCM upgraded its sw and is about to reset)
// 
// Returns:  N/A
void DsgEcmResetIndication( uint8 eCmResetType );

// eCM will call this function whenever any event happens in eCM side
// Parameters:
//      eventCode
//      pEventText
//
// Returns:  N/A
void DsgDocsisEvent (int eventCode, char *pEventText);

// The eCM calls this function to notify the eSTB that the eCM MAC reinitialized.
//
// Parameters:  N/A
//
// Returns:  N/A
//
void DsgEcmMacReinitEvent( void );

// Extended DSG API for erouter
#if (BFC_INCLUDE_DNAC_SUPPORT)
int ErouterEnable ( bool enable );
int ErouterWebIntfcEnable ( bool enable );
int ErouterIpv6AddressAssign ( int assign_method );
int ErouterStatus (t_erouter_status* pStatus);
#endif 
#if defined __cplusplus
}
#endif

#endif
