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
//  Filename:       dhcpV6EvdClient.cpp
//  Author:         Pinar Taskiran Cyr
//  Creation Date:  Nov 7, 2008
//
//****************************************************************************
#include "EstbClientDefaultEventList.h"
#include "EvDispatcherClientIf.h"
  
extern "C" void dhcpV6EvdClientNotification(struct clientNotification *clientNotification);
extern "C" int keepaliveclient(struct dsgClientRegInfo *argp);
static char* EventStatusTypeName( unsigned int value );
  
void initializeDhcpV6ClientRpcInterfaceAndSubscribeToEvents(void);
void dhcpV6EvdClientLogEvent(unsigned int eventCode);

extern unsigned int rpcClientHandle;
                    
void initializeDhcpV6ClientRpcInterfaceAndSubscribeToEvents(void)
{
	EV_DISPATCHER_CLNT_IF clientIf;

	bzero((char *) &clientIf, sizeof(EV_DISPATCHER_CLNT_IF));

	clientIf.NotificationHandler = dhcpV6EvdClientNotification;

	// Get a pointer to the events this client is interested.
	clientIf.numOfEvents = kDefault_NumOfEvents;
	
	unsigned int eventList[kDefault_NumOfEvents] = kDefault_EventList;
	clientIf.pEventList = eventList;

	// Call the API to initialize this client's RPC event notification system.
	initializeEventDispatcherClientRpcInterface(&clientIf);
}


void dhcpV6EvdClientNotification(struct clientNotification *clientNotification) 
{
	printf("DHCPv6 CLIENT: NOTIFICATION from Event Dispatcher -> %s value= %d\n", EventStatusTypeName(clientNotification->eventType), clientNotification->eventValue); 	
	
}

static char* EventStatusTypeName( unsigned int value ) 
{
	char notRecognizedStr[] = "EVENT NOT RECOGNIZED";

	if( (value < kEventDsgScanSuccess) || (value > kEventDsgCommonDownloadNotification))
	{
		return notRecognizedStr;
	}
	/* 
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
			kEventDsgCommonDownloadNotification
		};
	*/

	char* names[] =
	{
		"SCAN SUCCESS",
		"",	/* = 1 */
		"",              /* = 2 */
		"TWO-WAY OK",		/* = 3 */
		"",            /* = 4 */
		"",
		"",
		"TWO-WAY FORWARDING DISABLED", 
		"",
		"",
		"eCM IS RESETTING",
		"COMMON DOWNLOAD NOTIFICATION"

	};

	return names[ value ];
}

void dhcpV6EvdClientLogEvent(unsigned int eventCode) 
{
	//DEBUGGING - TESTING EVENT LOGGING
	struct dsgClientRegInfo regInfoData;
	memset(&regInfoData, 0, sizeof(regInfoData));
	regInfoData.clientPort = kEventLogEvent;
	regInfoData.clientId = eventCode;
	regInfoData.handle = rpcClientHandle;

	printf("DHCPv6 LOG EVENT  regInfoData.handle is %d\n", regInfoData.handle);
	keepaliveclient(&regInfoData);
}