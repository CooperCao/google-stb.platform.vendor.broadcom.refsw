//****************************************************************************
//
// Copyright (c) 2004-2012 Broadcom Corporation
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
//  Filename:       CmDavicApiSocketInterface.c
//  Author:         Pinar Taskiran-Cyr
//  Creation Date:  July 14, 2011
//
//****************************************************************************
//  Description:
//
//****************************************************************************

//********************** Include Files ***************************************

// My api.
#include "CmDavicApiSocketInterface.h"
#include "stdio.h"
#include <string.h>

//********************** Extern variables ************************************
extern uint32 displayMode;
//********************** Extern functions ************************************

extern int ProcessDavicApiSocketTx( uint8 apiType, unsigned int apiParam, uint32 apiLen, uint8* apiBuffer, bool readFromBuffer);

//********************** Local Types *****************************************
static PFN_DAVIC_RX_TUNNELDATA_HANDLER rxTunneldataHandler = 0;

static PFN_DAVIC_STATUS_DATA_HANDLER statusCallbackHandler = NULL;
//********************** Local Constants *************************************

//********************** Class Method Implementations ************************


// RETURNS
// 0 success
// negative number failed
int BcmDavicStart(void)
{
	SetOobMode(kOobModeDavic);

	// SEND VIA SOCKET
	if(ProcessDavicApiSocketTx( kDavicStart, 0, 0, 0, false)== 1)
		return 0;

	return -1;
}
  
// RETURNS
// 0 success
// negative number failed
int BcmDavicStop(void)
{
	SetOobMode(kOobModeNone);

	// SEND VIA SOCKET
	if(ProcessDavicApiSocketTx( kDavicStop, 0, 0, 0, false)== 1)
		return 0;

	return -1;
}

// RETURNS
// 0 success
// negative number failed
int BcmDavicEnableUpstream(unsigned int twoWayMode)
{
	// SEND VIA SOCKET
	if(ProcessDavicApiSocketTx( kDavicEnableUpstream, 0, sizeof(twoWayMode), (uint8*) (&twoWayMode), false)== 1)
		return 0;

	return -1;
}

// RETURNS
// 0 success
// negative number failed
int BcmDavicSetMode ( unsigned int mode )
{
	unsigned int davicMode = mode;

	// SEND VIA SOCKET
	if(ProcessDavicApiSocketTx( kDavicSetMode, 0, sizeof(davicMode), (uint8*) (&davicMode), false)== 1)
		return 0;

	return -1;
}

// RETURNS
// 0 success
// negative number failed
int BcmDavicOpenTunnel(unsigned int vpivci)
{
	unsigned int vpiVci = vpivci;

	// SEND VIA SOCKET
	if(ProcessDavicApiSocketTx( kDavicOpenTunnel, 0, sizeof(vpivci), (uint8*) (&vpiVci), false)== 1)
		return 0;

	return -1;
}

// RETURNS
// 0 success
// negative number failed
int BcmDavicCloseTunnel(unsigned int vpivci)
{
	unsigned int vpiVci = vpivci;

	// SEND VIA SOCKET
	if(ProcessDavicApiSocketTx( kDavicCloseTunnel, 0, sizeof(vpivci), (uint8*) (&vpiVci), false)== 1)
		return 0;

	return -1;
}

// RETURNS
// 0 success
// negative number failed
int BcmDavicSetConfig ( unsigned int param, void *data, int dataLen )
{
	// SEND VIA SOCKET
	if(ProcessDavicApiSocketTx( kDavicSetConfig, param, dataLen, (uint8*) data, false)== 1)
		return 0;

	return -1;
}

// RETURNS
// 0 success
// negative number failed
// IMPORTANT: The calling function MUST handle endianness.
int BcmDavicGetConfig ( unsigned int param, void *data, int dataLen )
{
	memset(data, 0, dataLen);

	// SEND VIA SOCKET
	if(ProcessDavicApiSocketTx( kDavicGetConfig, param, dataLen, (uint8*) data, true)== 1)
	{
		/* ntohl */
		unsigned int *pNtohlData = NULL;

		if( dataLen == sizeof(unsigned int))
		{
			pNtohlData = (unsigned int*) data;
			*pNtohlData = ntohl(*pNtohlData);

			if(displayMode & kDavicLevel2)
				LogRawBuffer ("DavicGetConfig", data, dataLen, 0);
		}

		switch(param)
		{
			case kDavicGetDefaultConnSets:
				{				
					if(displayMode & kDavicLevel1)
					{
						printf("\n************************************"
						   "\n  GET DEFAULT CONNECTION SETTINGS   "
						   "\n**********************************\n");
					}

					NtohlDavicConnectionSettings((DavicConnectionSettings*) data);

					if(displayMode & kDavicLevel2)
						LogRawBuffer ("DavicGetConfig", data, dataLen, 0);
				}
				break;

			case kDavicGetStateInfo:
				{				
					if(displayMode & kDavicLevel1)
					{
						printf("\n************************************"
							   "\n  GET LOCK STATE INFO   "
							   "\n**********************************\n");	
					}
					
					NtohlDavicStateInfo((DavicStateInfo*) data);

					if(displayMode & kDavicLevel2)
						LogRawBuffer ("DavicGetConfig", data, dataLen, 0);
				}
				break;

			case kDavicGetStateChangeInfo:
				{				
					if(displayMode & kDavicLevel1)
					{
						printf("\n************************************"
							   "\n  GET DAVIC STATE CHANGE INFO   "
							   "\n**********************************\n");	
					}
					
					NtohlDavicStateChangeInfo((DavicStateChangeMapSets*) data);

					if(displayMode & kDavicLevel2)
						LogRawBuffer ("DavicGetConfig", data, dataLen, 0);
				}
				break;

			default:
				{
				}
				break;
		}

		return 0;
	}

	return -1;
}

void BcmDavicSetStatusCallback( PFN_DAVIC_STATUS_DATA_HANDLER statusCallback)
{
	statusCallbackHandler = statusCallback;
}

void BcmDavicStatusDataHandler( DavicEventNotification eventCode, void *data, int dataLen )
{
	if(displayMode & kDavicLevel2)
		LogRawBuffer ("BcmDavicStatusDataHandler-Enter", data, dataLen, 0);

	unsigned int *pNtohlData = NULL;

	// ntohl
	if( dataLen == sizeof(unsigned int))
	{
		pNtohlData = (unsigned int*) data;
		*pNtohlData = ntohl(*pNtohlData);
	}

	switch(eventCode)
	{
		case kEventDavicHuntingForDsChannel:
			{				
				if(displayMode & kDavicLevel1)
				{
					printf("\n***************************"
						   "\n  HUNTING FOR DS CHANNEL   "
						   "\n***************************\n");
					printf("data: %d Hz! dataLen: %d \n", *pNtohlData, dataLen);
				}
			}
			break;

		case kEventDavicFullScanFail:
			{
				if(displayMode & kDavicLevel1)
				{
					printf("\n***********************"
						   "\n   FULL SCAN FAILED    "
						   "\n***********************\n");		
				}
			}
			break;

		case kEventDavicOneWayStateEstablished:
			{
				if(displayMode & kDavicLevel1)
				{
					printf("\n******************************"
						   "\n   ONE-WAY STATE ESTABLISHED  "
						   "\n******************************\n");
				}
			}
			break;

		case kEventDavicDsScanOk:
			{				
				if(displayMode & kDavicLevel1)
				{
					printf("\n******************"
						   "\n   DS SCAN OK		"
						   "\n******************\n");
					printf("data: %d Hz! dataLen: %d \n", *pNtohlData, dataLen);
				}
			}
			break;

		case kEventDavicDsLockLost:
			{			
				if(displayMode & kDavicLevel1)
				{
					printf("\n******************"
						   "\n   DS LOCK LOST   "
						   "\n******************\n");
					printf("data: %d Hz! dataLen: %d \n", *pNtohlData, dataLen);
				}
			}
			break;

		case kEventDavicTwoWayStateEstablished:
			{
				if( dataLen < sizeof(DavicConnectionSettings) )
				{
					printf("ERROR - kEventDavicTwoWayStateEstablished: dataLen %d < sizeof(DavicConnectionSettings) %d\n",
								dataLen, sizeof(DavicConnectionSettings));
					return;
				}

				NtohlDavicConnectionSettings((DavicConnectionSettings*) data);				

				if(displayMode & kDavicLevel1)
				{
					printf("\n******************************"
						   "\n   TWO-WAY STATE ESTABLISHED  "
						   "\n******************************\n");	
				}
			}
			break;

		case kEventDavicUpstreamChange:
			{
				if(displayMode & kDavicLevel1)
				{
					printf("\n**************************"
						   "\n   %s  "
						   "\n**************************\n", (*pNtohlData ? "ENTERED TWO-WAY STATE":"ENTERED ONE-WAY STATE"));
					printf("data: %d ! dataLen: %d \n", *pNtohlData, dataLen);
				}
			}
			break;

		default:
			printf("Received unknown event code: %d \n", eventCode);           
			break;
	}

	if(displayMode & kDavicLevel2)
		LogRawBuffer ("BcmDavicStatusDataHandler-Exit", data, dataLen, 0);

	/* Pass the data to the client's callback function */
	if( statusCallbackHandler)
		statusCallbackHandler( eventCode, data, dataLen);

	//printf("BcmDavicStatusDataHandler: Exit\n");
}

// Callback functions 
void BcmDavicSetTunnelPktCallback( PFN_DAVIC_RX_TUNNELDATA_HANDLER rxPktCallback)
{
	rxTunneldataHandler = rxPktCallback;
}

bool BcmDavicRxTunnelPkt(ETHERNET_PACKET* pBuffer )
{
	if(displayMode & kDavicLevel1)
		printf("DavicRxTunnelPkt: dataLen: %d \n", pBuffer->pDataBufferHead->uiLen);

	if( rxTunneldataHandler )
		if( rxTunneldataHandler( pBuffer) == 0 )
		{
			DsgFreeEthernetPkt(pBuffer);	
			return true;
		}

	return false;
}



/* This is the socket function that waits on DAVIC status information */
void BcmDavicProcessEcmNotification( uint8* pBuf )
{
	unsigned int *pEventCode = NULL;
	unsigned int *pDataLen = NULL;
	
	// Skip over event code:kEventDavicNotification
	pBuf++;

	// get the dataLen of DAVIC notification buffer.
	// Includes: DavicEventNotification (4 bytes) + length of notification value
	pDataLen = (unsigned int*) pBuf;
	*pDataLen = ntohl(*pDataLen);	

	// get the eventCode
	pEventCode = (unsigned int*) (pBuf+4);
	*pEventCode = ntohl(*pEventCode);

	if(displayMode & kDavicLevel1)
		LogInfoMsg2Parm("", " dataLen = %d  eventCode = %d \n", *pDataLen, *pEventCode);

	BcmDavicStatusDataHandler( *pEventCode, (pBuf+8), (*pDataLen-4) );
}

/*
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
*/
void NtohlDavicConnectionSettings(DavicConnectionSettings *data)
{
	data->fConnectionId = ntohl(data->fConnectionId);
	data->fReservationId = ntohl(data->fReservationId);
	data->fGrantProtocolTimeout = ntohl(data->fGrantProtocolTimeout);

	data->fDsFrequencyHz = ntohl(data->fDsFrequencyHz);
	data->fDsVPI = ntohl(data->fDsVPI);
	data->fDsVCI = ntohl(data->fDsVCI);
	data->fDsType = ntohl(data->fDsType);
	data->fQamFrequencyHz = ntohl(data->fQamFrequencyHz);
	data->fProgramNum = ntohl(data->fProgramNum);
	data->fUsFrequencyHz = ntohl(data->fUsFrequencyHz);
	data->fUsVPI = ntohl(data->fUsVPI);
	data->fUsVCI = ntohl(data->fUsVCI);
	data->fUsParams = ntohl(data->fUsParams);
}

/*
typedef struct   									
{
	uint32 fCmUsSetsState;	
	uint32 fCurrentDsFreqHz;
	uint32 fUsFrequencyHz;
	uint32 fBackupFrequencyHz;
						
    uint32 fUpstreamBitRate;
	uint32 fCurrentSignOnRetryCount;
				
} DavicStateInfo;
*/
void NtohlDavicStateInfo(DavicStateInfo *data)
{
	data->fCmUsSetsState = ntohl(data->fCmUsSetsState);
	data->fCurrentDsFreqHz = ntohl(data->fCurrentDsFreqHz);
	data->fUsFrequencyHz = ntohl(data->fUsFrequencyHz);
	data->fBackupFrequencyHz = ntohl(data->fBackupFrequencyHz);
	data->fUpstreamBitRate = ntohl(data->fUpstreamBitRate);
	data->fCurrentSignOnRetryCount = ntohl(data->fCurrentSignOnRetryCount);
}

/*
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
*/
void NtohlDavicStateChangeInfo(DavicStateChangeMapSets *data)
{
	uint32 i=0;
	data->numEntries = ntohl(data->numEntries);

	for(i=0; i< data->numEntries; i++)
	{
		data->davicStateChangeMap[i].timeTicksSinceStart = ntohl(data->davicStateChangeMap[i].timeTicksSinceStart);
		data->davicStateChangeMap[i].davicState = ntohl(data->davicStateChangeMap[i].davicState);
	}
}
