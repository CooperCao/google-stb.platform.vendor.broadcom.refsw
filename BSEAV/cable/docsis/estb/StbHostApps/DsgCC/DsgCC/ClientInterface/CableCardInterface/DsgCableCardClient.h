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
//  Filename:       DsgCableCardClient.h
//  Author:         Kenny Lee
//  Creation Date:  July 27, 2005
//
//****************************************************************************
//  Description:
//      This is the Cablecard Client as a derived class of Client class.
//
//****************************************************************************

#ifndef DSGCABLECARDCLIENT_H
#define DSGCABLECARDCLIENT_H

//********************** Include Files ***************************************
// My base class.
#include "DsgClient.h"

#include "DsgccCablecardIf.h"
#include "DsgEstbTunnelSettings.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************
// Pick up 8 bit number that is not used
#define CARD_ENTRY_CLIENT_ID_START  0xC0

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************

class BcmDsgCableCardClient : public BcmDsgClient
{
public:

    // Default Constructor.  Initializes the state of the object...
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmDsgCableCardClient(BcmCableCardIfThread *pCCIfThread);

    // Registration Constructor.  Initializes the state of this object
    // to the registration information passed in.
    //
    // Parameters:
    //      struct dsgClientRegInfo - the registration info.
    //
    // Returns:  N/A
    //
    BcmDsgCableCardClient(const struct dsgClientRegInfo *regInfo, BcmCableCardIfThread *pCCIfThread);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmDsgCableCardClient();

    // These methods are available to notify the client object that
    // some status has changed. The client object will 
    // notify the actual client of the change.
    int twoWayModeChangeNotification(int mode);
    int advancedModeChangeNotification(int mode);
    int tunnelPresenceChangeNotification(int presence);
	int TunnelPacket(const unsigned char *pkt, uint16 pktLen);
    int DsgDsScanCompleteNotification( void );
    int DsgUcid2wayOKNotification( uint8 ucid );
    int DsgDccDepartNotification( uint8 init_type );

	// This method sends a notification to each client what type of eCM provisioning 
	// limitations impact eCM 2-Way forwarding. Values below are bit fields that can 
	// be ORed to indicate multiple conditions.
	// 0x01  Network access disabled (NACO=0)
	// 0x02  Max CPE limit exhausted
	int DsgTwoWayForwardingDisabledNotification( uint8 twoWayForwardingLimitations );

    BcmDsgEstbTunnelSettings &CableCardTunnelSetting() { return fTunnelCfrSetting; }

    void SetDsgDirUcid( unsigned char ucid ) { fDsgDirUcid = ucid; }

private:

   	// the forwarded packet count
	uint32 fPacketCount;
	uint32 fNumberOfOctets;
    unsigned char fDsgDirUcid;      //Specified for host entry in dsg_dir

	BcmCableCardIfThread *pfCableCardIfThread;

    // Tunnel classifier settings
    BcmDsgEstbTunnelSettings fTunnelCfrSetting;

};

#endif


