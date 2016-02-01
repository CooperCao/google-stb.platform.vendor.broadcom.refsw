//****************************************************************************
//
// Copyright (c) 2009-2012 Broadcom Corporation
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
//  Filename:       StubInterface.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Dec 06, 2004
//
//****************************************************************************
//  Description:
//  C to C++ Interface.
//****************************************************************************

//********************** Include Files ***************************************

// My api.
#include "StubInterface.h"

#if (BCM_DSGCC_INCLUDED)
#include "DsgClientCtlThread.h"
#endif
	
#if (BCM_CANNED_SNMP_SUPPORT)
#include "EstbCannedSnmp.h"
#endif

#if defined (BCM_ESTB_ECM_INTERFACE_INCLUDED)
#include "EcmEventDispatcherThread.h"	
#include "EcmStatusEventCodes.h"
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Class Method Implementations ************************

// These functions must be provided by the eSTB
// eCM expects these functions to be present
void DsgScanSuccess( void )
{
#if (BCM_DSGCC_INCLUDED)
	BcmDsgClientCtlThread *ctl = BcmDsgClientCtlThread::GetSingletonInstance();
	if (ctl)
		ctl->DsgScanSuccess();
#endif
}

void DsgFullScanFail( void )
{
#if (BCM_DSGCC_INCLUDED)
	BcmDsgClientCtlThread *ctl = BcmDsgClientCtlThread::GetSingletonInstance();
	if (ctl)
		ctl->DsgFullScanFail();
#endif
}

void DsgRxDCD( int len, uint8 * dcd )
{
#if (BCM_DSGCC_INCLUDED)
	BcmDsgClientCtlThread *ctl = BcmDsgClientCtlThread::GetSingletonInstance();
	if (ctl)
		ctl->DsgRxDCD( len, dcd );
#endif
}

void DsgUpstreamChange( DsgEcmUsSettings *pDsgEcmUsSettings )
{
	#if (BCM_DSGCC_INCLUDED)
	BcmDsgClientCtlThread *ctl = BcmDsgClientCtlThread::GetSingletonInstance();
	if (ctl)
	{
		// We don't use pDsgEcmUsSettings->ipMode in our DSGCC for now.	
		// But, if someone needs ip_mode information, it may be used. */	
		if( ctl->Mode() != kDsgDisabledMode ) 
			ctl->DsgUpstreamChange( pDsgEcmUsSettings->upstreamState );
	}
	#endif

	#if defined (BCM_ESTB_ECM_INTERFACE_INCLUDED)
	BcmEcmEventDispatcherThread *pEvDispatcher = BcmEcmEventDispatcherThread::GetSingletonInstance();
	if (pEvDispatcher)
	{
		printf(" ########## kEventDsgUpstreamChange ############ VALUE = %d %d\n", pDsgEcmUsSettings->upstreamState, pDsgEcmUsSettings->ipMode);

        // Pass it along to the DSG event handler.  note that 
        // the msg publisher will make a copy for the subscriber.
		unsigned int upstreamStateAndIpMode = pDsgEcmUsSettings->ipMode; 
		upstreamStateAndIpMode <<= 16;
		upstreamStateAndIpMode |= pDsgEcmUsSettings->upstreamState;

        pEvDispatcher->PublishMsg( kEventDsgUpstreamChange, 
            (void*)(&upstreamStateAndIpMode) ); 
	}
	#endif	
}

void DsgUcid( uint8 uc_id  )
{
#if (BCM_DSGCC_INCLUDED)
	BcmDsgClientCtlThread *ctl = BcmDsgClientCtlThread::GetSingletonInstance();
	if (ctl)
		ctl->DsgUcid( uc_id  );
#endif
}

void DsgTimerEvent( uint8 tm_event )
{
#if (BCM_DSGCC_INCLUDED)
	BcmDsgClientCtlThread *ctl = BcmDsgClientCtlThread::GetSingletonInstance();
	if (ctl)
		ctl->DsgTimerEvent( tm_event );
#endif
}

bool DsgTunnelPkt( ETHERNET_PACKET *pkt )
{
#if (BCM_DSGCC_INCLUDED)
	BcmDsgClientCtlThread *ctl = BcmDsgClientCtlThread::GetSingletonInstance();
	if (ctl)
		return ctl->DsgTunnelPkt( pkt );
	return false;
#endif
}

void DsgDccInitType( uint8 tm_event )
{
#if (BCM_DSGCC_INCLUDED)
	BcmDsgClientCtlThread *ctl = BcmDsgClientCtlThread::GetSingletonInstance();
	if (ctl)
		ctl->DsgDccInitType( tm_event );
#endif
}

void DsgTwoWayForwardingDisabled( uint8 twoWayForwardingLimitations, uint8 maxCpe )
{
#if (BCM_DSGCC_INCLUDED)
	BcmDsgClientCtlThread *ctl = BcmDsgClientCtlThread::GetSingletonInstance();
	if (ctl)
		ctl->DsgTwoWayForwardingDisabled( twoWayForwardingLimitations );
#endif
}

void DsgEcmResetIndication(uint8 eCmResetType)
{
#if (BCM_DSGCC_INCLUDED)
	BcmDsgClientCtlThread *ctl = BcmDsgClientCtlThread::GetSingletonInstance();
	if (ctl)
		ctl->DsgEcmResetIndication( eCmResetType );
#endif
}

void DsgEcmMacReinitEvent( void )
{
#if (BCM_DSGCC_INCLUDED)
	BcmDsgClientCtlThread *ctl = BcmDsgClientCtlThread::GetSingletonInstance();
	if (ctl)
		ctl->DsgEcmMacReinitEvent();
#endif	

	#if defined (BCM_ESTB_ECM_INTERFACE_INCLUDED)
	BcmEcmEventDispatcherThread *pEvDispatcher = BcmEcmEventDispatcherThread::GetSingletonInstance();
	if (pEvDispatcher)
	{
		printf(" ########## DsgEcmMacReinitEvent ############ \n");
		
		uint8 dummy = 0;
        pEvDispatcher->PublishMsg( kEventDsgEcmMacReinit, 
            (void*)(&dummy) ); 
	}
	#endif	
}
/////////////////////////////////////////////////////////////////////////////////////
#if (ECM_STUB_API)

//********************** Local Variables *************************************
static uint32 fTunnelIdBitMask = 0;

static uint32 AssignTunnelId( void );
static void RecycleTunnelId( uint32 closedTunnelId );

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
	printf(" ECM_STUB_API - DsgSetDSGMode - mode = %d\n", mode);
	
	// Close the tunnels.
	fTunnelIdBitMask = 0;

	return kFuncSuccess;
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
	printf(" ECM_STUB_API - DsgEnableDOCSIS - twoway_mode = %s\n", (enable?"enabled":"disabled"));
	return true;    
}


/*
 *  Set Advanced mode DSG Info to eCM
 *
 * This function returns:
 * true == success; Advanced tunnel set
 * false == function failure
 *  
 */
int  DsgOpenTunnel( t_mac_adr * tunnel_adr, uint8 num_classifiers, t_dsg_classifier * classifier_list)
{
	printf(" ECM_STUB_API - DsgOpenTunnel \n");
	return AssignTunnelId();
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
	printf(" ECM_STUB_API - DsgCloseTunnel - tunnel_num = %d\n", tunnel_num);	
	RecycleTunnelId(tunnel_num);

	return kFuncSuccess;
    
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
	printf(" ECM_STUB_API - DsgSetTimer - [%d] = %d\n", timer_id, timeout);	
	return kFuncSuccess;    
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
	uint16 timeoutValue = 1800; // Dummy value

	printf(" ECM_STUB_API - DsgGetTimer - [%d] \n", timer_id);	
	memcpy(timeout, &timeoutValue, sizeof(timeout));
	return kFuncSuccess;    
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
bool DsgSetChannelList (uint16 num_chans, uint32 * channel_list)
{
	printf(" ECM_STUB_API - DsgSetChannelList - num_chans = %d \n", num_chans);
	return true;        
}


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
	printf(" ECM_STUB_API - DsgResetChannelList \n");
	return true;        
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
	printf(" ECM_STUB_API - DsgScan \n");
	return true;   
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
	printf(" ECM_STUB_API - DsgStatus - DSG info present \n");
	return 2; // = DSG info present
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
	printf(" ECM_STUB_API - DsgDocsisStatus - DOCSIS operational \n");
	return 12; //kOperational = 12,
}

bool DsgSetRIAdr( void )
{
	return true;
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
	printf(" ECM_STUB_API - DsgFreeEthernetPkt\n");
	return 1; //kSuccess;
}

static uint32 AssignTunnelId( void )
{
	uint8 bitPosition = 0;
	uint32 tunnelId = 0;
	uint32 setBit = fTunnelIdBitMask;
	while(((setBit & 0x1)) && (bitPosition <= 32))
	{
		 setBit >>= 1;
		 bitPosition++;
	}

	if(bitPosition <= 32)
	{
		tunnelId = bitPosition+1;
		setBit = 0x1;
		setBit <<= bitPosition;
		fTunnelIdBitMask |= setBit;
		printf(" AssignTunnelId - tunnelIdBitMask = %d\n",  fTunnelIdBitMask);
	}

	printf(" AssignTunnelId - tunnelId = %d\n",  tunnelId);
	return tunnelId;
}

static void RecycleTunnelId( uint32 closedTunnelId )
{
	uint8 bitPosition = closedTunnelId -1;
	uint32 resetBit = 0x00000001;
	fTunnelIdBitMask &= ~(resetBit << bitPosition); 
	printf(" RecycleTunnelId - tunnelIdBitMask = %d\n",  fTunnelIdBitMask);
}

#endif

#if (BCM_CANNED_SNMP_SUPPORT)
// Set_docsDevSwFilename(const char *fileName)
// Get_docsDevSwFilename(char *fileName)
//
//		Create, send, and verify and SNMP SET/GET for the docsDevSwFilename MIB.
//
// Parameters:
//      fileName - Pointer to null terminated file name ascii string.
//
// Returns:
//      true - All is well
//		False - Something didn't work
//
bool Set_docsDevSwFilename(const char *fileName)
{
	BcmEstbCannedSnmp *pSnmp = BcmEstbCannedSnmp::GetSingletonInstance();
	if (pSnmp)
		return pSnmp->Set_docsDevSwFilename( fileName );

	return false;
}

bool Get_docsDevSwFilename(char *fileName)
{
	BcmEstbCannedSnmp *pSnmp = BcmEstbCannedSnmp::GetSingletonInstance();
	if (pSnmp)
		return pSnmp->Get_docsDevSwFilename( fileName );

	return false;
}

void DsgDocsisEvent (int eventCode, char *pEventText)
{
	printf("EcmEventNotification - ID: %d, TEXT: %s\n", eventCode,  pEventText);
}

#endif

