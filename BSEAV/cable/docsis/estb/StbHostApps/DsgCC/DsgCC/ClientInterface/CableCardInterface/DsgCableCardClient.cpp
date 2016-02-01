//****************************************************************************
//
// Copyright (c) 2005-2010 Broadcom Corporation
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
//  Filename:       DsgCableCardClient.cpp
//  Author:         Kenny Lee
//  Creation Date:  July 27, 2005
//
//****************************************************************************
//  Description:
//      This is the implementation of the CableCard Client as a derived DsgClient class.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DsgClientCtlThread.h"
#include "DsgCableCardClient.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Initializes the state of the object...
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmDsgCableCardClient::BcmDsgCableCardClient(BcmCableCardIfThread *pCCIfThread) :
	BcmDsgClient(),
    fDsgDirUcid(0),
    fTunnelCfrSetting()
{
	pfCableCardIfThread = pCCIfThread;

    // Init Client ID & type object into the TunnelCfrSettign object
    // This is for client controller to retrival Client ID & type when opening tunnel
	BcmDsgClientId dsgClientId; 
    fTunnelCfrSetting.fClientIdList.push_back( dsgClientId );   //make a copy into list

}

// Registration Constructor.  Sets the state of the object based on
// values passed in registration
//
// Parameters:  struct dsgClientRegInfo.
//
// Returns:  N/A
//
BcmDsgCableCardClient::BcmDsgCableCardClient(const struct dsgClientRegInfo *regInfo, BcmCableCardIfThread *pCCIfThread ) :
	BcmDsgClient(regInfo),
    fDsgDirUcid(0),
    fTunnelCfrSetting()
{
	pfCableCardIfThread = pCCIfThread;

    // Init Client ID & type object into the TunnelCfrSettign object
    // This is for client controller to retrival Client ID & type when opening tunnel
	BcmDsgClientId dsgClientId; 
    fTunnelCfrSetting.fClientIdList.push_back( dsgClientId );  //make a copy into list

}

// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDsgCableCardClient::~BcmDsgCableCardClient()
{
	// debug code to check if this object is being deleted in the multimap element
	//cout << "~BcmDsgCableCardClient::Deleting this Client obj ptr " << this << ". TunnelID is " << TunnelId() << endl;
	// Nothing to do...
}


// These methods are available to notify the client object that
// some status has changed. The client object will 
// notify the actual client of the change.
int BcmDsgCableCardClient::twoWayModeChangeNotification(int mode)
{
    // Only send one notification to cablecard on multiple cablecard clients opened
    // Use the first card entry client ID. Not working if card only open Host entry
    //if( Id() == CARD_ENTRY_CLIENT_ID_START )
    // Use the first clientPort which can be either a Host entry or Card entry opened by cablecard.
    if( ClientPort() == kClientPort )
		pfCableCardIfThread->sendTwoWayModeToCableCard( mode );
    return (1);
}

int BcmDsgCableCardClient::advancedModeChangeNotification(int mode)
{
	//cout << "-->In derived class BcmDsgCableCardClient::advancedModeChangeNotification... " << endl;
	return (1);
}


int BcmDsgCableCardClient::tunnelPresenceChangeNotification(int presence)
{
	//cout << "-->In derived class BcmDsgCableCardClient::tunnelPresenceChangeNotification... " << endl;
	return (1);
}

int BcmDsgCableCardClient::TunnelPacket(const unsigned char *pkt, uint16 pktLen)
{
    BcmDsgClientCtlThread *pDsgClientCtlThread = BcmDsgClientCtlThread::GetSingletonInstance();

	fPacketCount++;
	fNumberOfOctets += pktLen;

	//cout << "-->In derived class BcmDsgCableCardClient::TunnelPacket... (" << dec << pktLen << ") MacAddr=" << fTunnelCfrSetting.TunnelAddress() << endl;
	//Restore the first 4 bytes of Client ID back to tunnel mac address
    fTunnelCfrSetting.TunnelAddress().getAddr( (uint8 *)pkt );

    // DSG advance mode: check if this tunnel terminate at Host or Card
    if( (pDsgClientCtlThread->Mode() == kDsgAdvanceMode) && (IdType() != kCardEntryClientType) )
        return pfCableCardIfThread->sendTunnelDataToHost( pkt, pktLen, IdType(), Id() );
    else
    	return pfCableCardIfThread->sendTunnelDataToCableCard( pkt, pktLen );
}

int BcmDsgCableCardClient::DsgDsScanCompleteNotification( void )
{
    // Only send one notification to cablecard on multiple cablecard clients opened
    // Use the first clientPort which can be either a Host entry or Card entry opened by cablecard.
    if( ClientPort() == kClientPort )
        pfCableCardIfThread->sendDsScanCompleteToCableCard();
    return 1;
}

int BcmDsgCableCardClient::DsgUcid2wayOKNotification( uint8 ucid )
{
    // Only send one notification to cablecard on multiple cablecard clients opened
    // Use the first clientPort which can be either a Host entry or Card entry opened by cablecard.
    if( ClientPort() == kClientPort )
        pfCableCardIfThread->sendUcidToCableCard( ucid );

    // Process UCID here for Host tunnels
    if( IdType() != kCardEntryClientType )
    {
        if( fDsgDirUcid==0 )
                cout << "(DsgDirUCID Not specified!) Host Tunnel ID " << Id() << " is now UCID Certified with Docsis UCID = " << (int)ucid << endl;
        else
        { 
            if( fDsgDirUcid == ucid )
                cout << "Host Tunnel ID " << Id() << " is now UCID Certified with Docsis UCID = " << ucid << endl;
            else
                cout << "Warning! Host Tunnel ID " << Id() << " DsgDirUCID(" << fDsgDirUcid << ") != with DocsisUCID(" << (int)ucid << "). Will Forward Tunnel traffic regardless!" << endl;
        }
    }

    return 1;
}

int BcmDsgCableCardClient::DsgDccDepartNotification( uint8 init_type )
{
    // Only send one notification to cablecard on multiple cablecard clients opened
    // Use the first clientPort which can be either a Host entry or Card entry opened by cablecard.
    if( ClientPort() == kClientPort )
        pfCableCardIfThread->sendDccDepartToCableCard( init_type );
    return 1;
}

// This method tells the client what type of eCM provisioning 
// limitations impact eCM 2-Way forwarding. Values below are bit fields that can 
// be ORed to indicate multiple conditions.
// 0x01  Network access disabled (NACO=0)
// 0x02  Max CPE limit exhausted
int BcmDsgCableCardClient::DsgTwoWayForwardingDisabledNotification( uint8 twoWayForwardingLimitations )
{
    // Only send one notification to cablecard on multiple cablecard clients opened
    // Use the first clientPort which can be either a Host entry or Card entry opened by cablecard.
    if( ClientPort() == kClientPort )
        pfCableCardIfThread->sendDisabledForwardingTypeToCableCard( twoWayForwardingLimitations );
    return 1;
}
