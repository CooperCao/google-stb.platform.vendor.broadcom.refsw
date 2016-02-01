//****************************************************************************
//
// Copyright (c) 2003-2011 Broadcom Corporation
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
//  Filename:       davic_api.h
//  Author:         Pinar Taskiran-Cyr
//  Creation Date:  June 13, 2011
//
//****************************************************************************
//  Description: 
//                  
//
//****************************************************************************

#ifndef DAVICAPI_H
#define DAVICAPI_H

//********************** Include Files ***************************************
#if defined __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"

#include "EnetPkt.h"

#ifndef __cplusplus
#if (!defined (TARGETOS_eCos) && !defined (DSGAPI_H))
typedef int bool;
#define true 1
#define false 0
#endif
#endif

/* BROADCOM API functions provided by eSTB to eCM */
#define MAXNUMOOBSTREAMS 5

#define MAXNUM_STATECHANGE_ENTRIES 50

//      mode -  0: kLegacyMode
//			    1: kCableCardMode
typedef enum 
{
	kLegacyMode,	
	kCableCardMode,

	kDavicDisabled = 99
} DavicMode;

typedef enum
{
    // initial state: have not yet attempted to obtain upstream settings.
    kUsSetsInitial = 0,
    //
    // Waiting to receive a Provisioning Channel Message.
    kWaitingForProvisioningCh,
	//
    // One-way transmission established.
    kOneWayStateEstablished,
	//
    // Waiting to receive a Default Config Message.
    kWaitingForDefaultConfig,
	//
    // Waiting to receive a Sign-On Request Message.
    kWaitingForSignOnReq,    
	//
    // Receivd a Sign-On Req Message and Waiting to send a Sign-On Response Message.
    kWaitingToSendSignOnRsp,   		
    //
    // Waiting to receive the first Ranging And Calibration Message.
    kWaitingForInitialRngCalb,     
    //
    // Waiting to receive a Ranging And Calibration or Init Complete Message.
    kStartedRngCalbCycle,     
    //
    // Waiting to receive a Default Connection Message.
    kWaitingForDefaultConnect,     
	//
    // Waiting to receive a Connect Confirm Message.
    kWaitingForDefaultConnectConfirm,   
    //
    // Waiting to receive a Reservation ID Assignment Message.
    kWaitingForDefaultResId,
	//
    // Waiting to complete IP initialization.
    kInitializingIP,   
	//
    // Upstream settings have been obtained, stored and applied to hardware.
	kOperational
	
}DavicState;

typedef struct   									
{
	uint32 fCmUsSetsState;	
	uint32 fCurrentDsFreqHz;
	uint32 fUsFrequencyHz;
	uint32 fBackupFrequencyHz;
						
    uint32 fUpstreamBitRate;
	uint32 fCurrentSignOnRetryCount;
				
} DavicStateInfo;

typedef struct   									
{
	uint32 timeTicksSinceStart;	
	uint32 davicState;					
} DavicStateChangeMapEntry;

typedef struct   									
{
	uint32 numEntries;	
	DavicStateChangeMapEntry davicStateChangeMap[MAXNUM_STATECHANGE_ENTRIES];					
} DavicStateChangeMapSets;

typedef struct   									
{
    // Information pertaining to the connection.
	uint32 fConnectionId;	
	uint32 fReservationId;
	uint32 fGrantProtocolTimeout;
						
    uint8  fConnectionCtrlField;
	uint8  fUpstreamChannelId;			
    uint8  fMaxContMsgLen;				
	uint8  fMaxResvMsgLen;
	
	// Downstream ATM Descriptor
	uint32 fDsFrequencyHz;
	uint32 fDsVPI;
	uint32 fDsVCI;
	uint32 fDsType;				 // enum{ QAM, QPSK_1.544, QPSK_3.088, reserved 3..255}

	// Downstream MPEG Descriptor
	uint32 fQamFrequencyHz;
	uint32 fProgramNum;

	// Upstream ATM Descriptor
	uint32 fUsFrequencyHz;
	uint32 fUsVPI;
	uint32 fUsVCI;
	uint32  fUsParams;
	//MacFlagSet
	//UpstreamBitRate           // enum{ QAM, QPSK_1.544, QPSK_3.088, reserved 3..255}
				
} DavicConnectionSettings;

typedef enum 
{
	kDavicGetDefaultConnSets,       		// data is pointer to DavicConnectionSettings structure
	kDavicGetStateInfo,						// data is pointer to DavicStateInfo structure
	kDavicGetStateChangeInfo				// data is pointer to DavicStateChangeMapSets structure

} DavicGetConfigSets;

typedef enum 
{
	kDavicSetParamProviderIDValue,
	kDavicSetParamProviderIDCheck,
	kDavicSetNSAPAddr,					/* Broadcom DAVIC API config parameter*/
	kDavicSetParamNSAPTxEnb

} DavicSetConfigSets;

typedef enum 
{
	kEventDavicHuntingForDsChannel,
    kEventDavicDsScanOk,
    kEventDavicDsLockLost,
    kEventDavicFullScanFail,				/* Full spectrum scan failed without detecting PROV-CH msg */
    kEventDavicOneWayStateEstablished,
    kEventDavicTwoWayStateEstablished,
    kEventDavicUpstreamChange				/* 0: one-way-state 1: two */

} DavicEventNotification;

typedef enum
{
	kDavicStart			=1,
	kDavicStop,
	kDavicEnableUpstream,
	kDavicSetMode,
	kDavicOpenTunnel,
	kDavicSetConfig,
	kDavicGetConfig,
	kDavicCloseTunnel

} DavicApiTypeCode;

typedef
bool (*PFN_DAVIC_RX_TUNNELDATA_HANDLER) ( ETHERNET_PACKET* );

typedef
void (*PFN_DAVIC_STATUS_DATA_HANDLER) ( DavicEventNotification eventCode, void *data, int dataLen );

typedef struct eSTB_DAVIC_CONTROL_INTERFACE {

    PFN_DAVIC_RX_TUNNELDATA_HANDLER           DavicRxTunnelDataHandler;
	PFN_DAVIC_STATUS_DATA_HANDLER			  DavicStatusDataHandler;

} DAVIC_CALLBACK;


/* API functions provided by eCM to eSTB */

typedef
int (*PFN_DAVIC_START_HANDLER) ( BcmHandle handle );

typedef
int (*PFN_DAVIC_STOP_HANDLER) ( BcmHandle handle );

typedef
int (*PFN_DAVIC_ENABLE_UPSTREAM_HANDLER) ( unsigned int twoWayMode, BcmHandle handle );

typedef
int (*PFN_DAVIC_SET_MODE_HANDLER) ( int mode, BcmHandle handle );

typedef
int (*PFN_DAVIC_OPEN_TUNNEL_HANDLER) ( int vpivci, BcmHandle handle );

typedef
int (*PFN_DAVIC_CLOSE_TUNNEL_HANDLER) ( int vpivci, BcmHandle handle );

typedef
int (*PFN_DAVIC_SET_CONFIG_HANDLER) (unsigned int param, void *data, int dataLen, BcmHandle handle);

typedef
int (*PFN_DAVIC_GET_CONFIG_HANDLER) (unsigned int param, void *data, int dataLen, BcmHandle handle);

typedef
bool (*PFN_DAVIC_REGISTER_API_HANDLER ) ( DAVIC_CALLBACK *, BcmHandle handle );


typedef struct eCM_DAVIC_CONTROL_INTERFACE 
{
	unsigned long hMac;
	PFN_DAVIC_START_HANDLER					DavicStartHandler;
	PFN_DAVIC_STOP_HANDLER					DavicStopHandler;
	PFN_DAVIC_ENABLE_UPSTREAM_HANDLER		DavicEnableUpstreamHandler;
	PFN_DAVIC_SET_MODE_HANDLER				DavicSetModeHandler;
	PFN_DAVIC_SET_CONFIG_HANDLER			DavicSetConfigHandler;
	PFN_DAVIC_GET_CONFIG_HANDLER			DavicGetConfigHandler;
	PFN_DAVIC_OPEN_TUNNEL_HANDLER			DavicOpenTunnelHandler;
	PFN_DAVIC_CLOSE_TUNNEL_HANDLER			DavicCloseTunnelHandler;
	PFN_DAVIC_REGISTER_API_HANDLER			RegisterCallback;
} DAVIC_API;

//********************** Global Constants ************************************

// "C" callable DAVIC API functions

           
#if defined __cplusplus
}
#endif

#endif
