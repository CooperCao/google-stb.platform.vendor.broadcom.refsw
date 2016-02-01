//****************************************************************************
//
// Copyright (c) 2008 Broadcom Corporation
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
//  $Id$
//
//  Filename:       CmDocsisStatusEventCodes.h
//  Author:         Cliff Danielson
//  Creation Date:  Apr 13, 2001
//
//****************************************************************************
//  Description:
//      Event codes for use with the BcmCmDocsisStatusEventACT client
//      interface used by BcmCmDocsisCtlThread to notify clients of
//      interesting status change events.
//      Refer to POSA2 book, page 268. 
//
//****************************************************************************
//
// CmDocsisStatusEventCodes.h
//
#ifndef CMDOCSISSTATUSEVENTCODES_H
#define CMDOCSISSTATUSEVENTCODES_H


class BcmCmDocsisStatusEventCodes 
{
public:
    enum
    {
        // event codes used in Asynchronous Completion Token client
        // interface.
        
        // CM starting downstream scan.  (customer request).
        kCmDsScanStarted = 0,

        // CM has acquired downstream FEC lock but not time synchronization.
        // (DSG app request).  event published by BcmCmDocsisCtlThread.
        kCmDsLockOk,

        // CM has failed to acquire downstream FEC lock.
        kCmDsLockFail,


        // CM has obtained primary downstream channel MAC Domain Descriptor 
        // settings.
        kCmPriDsMddSetsOk,
        
        // CM has detected on-the-fly MDD change for primary downstream
        // channel MAC Domain Descriptor settings.
        kCmOtfPriDsMddChangeDetected,

        // CM has reset/discarded primary downstream channel MAC Domain 
        // Descriptor settings.
        kCmPriDsMddSetsReset,

        // CM has detected MDD keep alive failure on the primary downstream
        // channel.
        kCmPriDsMddKaFail,

        // MDD keep alive has recovered on the primary downstream
        // channel.
        kCmPriDsMddKaRescue,


        // CM has acquired downstream FEC lock and time synchronization.
        // (customer request).  event published by BcmCmDocsisCtlThread.
        kCmDsLockSyncOk,

        // CM has failed to acquire downstream time synchronization.
        kCmDsSyncFail,

        // CM has gathered and applied UCD settings for the target upstream
        // channel.  event published by BcmCmDocsisCtlThread.
        kCmUcdSetsOk,

        // CM has started the upstream ranging process.  (customer request)
        // event published by BcmCmDocsisCtlThread.
        kCmUsRangingStarted,

        // RNG-RSP with status==SUCCESS received for first time since joining 
        // the current upstream channel.  (customer request)
        // event published by BcmCmDocsisCtlThread.
        kCmFirstRngSuccess,

        // RNG-RSP message received.  This is only notified when it is a "normal"
        // RNG-RSP (continue or success, no channel overrides, not abort, etc).
        kCmRngRspReceived,

        // operational CM has recovered upstream time reference for at least
        // one upstream channel after failure (due to loss of downstream 
        // phy lock or not receiving SYNC messages within threshold limits).
        kCmOperAnyUsTimeRefRescue,


        // begin DOCSIS IP initialization events.
        //
        // DHCP portion of CM IP initialization has started.  (customer request)
        // event published by BcmCmDocsisIpThread
        kCmDhcpInitStarted,
        //
        // DHCPv6 address has been acquired, but duplicate address detection
        // has not yet been performed. 
        kCmDhcpv6IpAddressAcquiredTentative,
        //
        // DHCP portion of CM IP initilization has successfully completed.
        // (customer request).  event published by BcmCmDocsisIpThread.
        kCmDhcpInitOk,
        //
        // DHCP portion of CM IP initilization has failed.
        // event published by BcmCmDocsisIpThread.
        kCmDhcpInitFailed,
        //
        // PR5736
        // The DHCP client is about to start renewing the lease for the CM's
        // public stack.
        kCmDhcpRenewStarting,
        //
        // PR5736
        // The DHCP client (successfully) finished renewing the lease for the
        // CM's public stack.
        kCmDhcpRenewSuccess,
        //
        // PR2182
        // DHCP failed during renewals (this is most often post-reg).
        // event published by BcmCmDocsisIpThread.
        kCmDhcpRenewFailed,
        //
        //
        // TFTP portion of CM IP initialization has started.  
        // event published by BcmCmDocsisIpThread
        kCmTftpInitStarted,
        //
        // TFTP portion of CM IP initilization has successfully completed.
        // (customer request).  event published by BcmCmDocsisIpThread.
        kCmTftpInitOk,
        //
        // TFTP portion of CM IP initilization has failed.
        // event published by BcmCmDocsisIpThread.
        kCmTftpInitFailed,
        //
        //
        // Time of Day portion of CM IP initialization has started.   
        // event published by BcmCmDocsisIpThread
        kCmTodInitStarted,
        //
        // Time of Day portion of CM IP initilization has successfully completed.
        // (customer request).  event published by BcmCmDocsisIpThread.
        kCmTodInitOk,
        //
        // Time of Day portion of CM IP initilization has failed.
        // event published by BcmCmDocsisIpThread.
        kCmTodInitFailed,
        //
        //
        // CM software download has started.  
        // event published by BcmCmDocsisIpThread
        kCmSwDownloadStarted,
        //
        // CM software download has successfully completed.  (customer request)
        // event published by BcmCmDocsisIpThread.
        kCmSwDownloadOk,
        //
        // CM software download has failed.
        // event published by BcmCmDocsisIpThread.
        kCmSwDownloadFailed,
        //
        // end DOCSIS IP initialization events.

        // config file parsed without error and pre-registration config file
        // items that BcmCmDocsisCtlThread handles have been applied.
        // Note that customer vendor specific items have not been applied.
        kCmPreRegConfigFileOk,
        //
        // CM DOCSIS registration has successfully completed.
        kCmRegOk,
        //
        // CM DOCSIS registration has failed.
        kCmRegFailed,
        //
        // CM operational state refers to whether or not client data traffic 
        // can be forwarded over the HFC network.  
        // event published by BcmCmDocsisCtlThread.
        kCmIsOperational,
        //
        // CM can no longer forward client data traffic over the HFC network when
        // registration state is reset.  event published by BcmCmDocsisCtlThread.
        kCmResetRegState,  
        kCmIsNotOperational = kCmResetRegState, // synonym  
        //
        // CM IP protocol layer has been reset to initial conditions.
        // event published by BcmCmDocsisIpThread
        kCmResetIp,


        //
        // CM has started a post-Registration UCC/DCC channel change.
        // event published by BcmCmDocsisCtlThread.
        kCmPostRegXccChanChangeStarted,
        //
        // CM has successfully completed a post-Registration UCC/DCC channel 
        // change.  NOTE: if the UCC/DCC fails this notification will not
        // occur.  The UCC/DCC failure will result in a kCmIsNotOperational 
        // notification.  event published by BcmCmDocsisCtlThread.
        kCmPostRegXccChanChangeOk,
        
        // CM has experienced a DOCSIS "restart upstream MAC" error
        // AND DSG specific error handling is in effect.  
        kCmDocsisRstErrBeginDsgErrHandling,
        kDsgOneWayMode = kCmDocsisRstErrBeginDsgErrHandling,

        // CM is about to reboot itself.
        kCmIsShuttingDown,

        // CM is disabling network access.
        kCmIsShuttingDownDataPath,

        // DOCSIS 3.0 Dynamic Bonding Change MAC management message can 
        // change 
        // 1) downstream channel assignments (a.k.a. Rx Chan Config)
        // 2) upstream channel assignments (a.k.a. Tx Chan Config)
        // 3) DSID (data path id used for bonded packet reassembly and
        //      multicast).
        // 4) SAID (security id)
        //
        // Changes 1), 2) and 3) are of interest to 3df (a.k.a. CM-STATUS
        // messaging subsystem) and may be of interest to others.  
        // BcmCmDocsisCtlThread will publish events to indicate that a 
        // DBC-REQ has changed RCC, TCC, DSIDs or SAIDs.
        kCmDbcOtfRccChange,
        kCmDbcOtfTccChange,
        kCmDbcOtfDsidChange,
        kCmDbcOtfSaidChange,

        // DOCSIS 3.0 CM-STATUS events which do NOT require event instance id
        // parameters (and can therefore be published as a status event
        // rather than a message).
        //
        // CM equipped with battery backup has lost normal AC power and is now
        // running on battery backup.
        kCmAcPowerFail,
        //
        // CM equipped with battery backup is running on normal AC power
        // after running on battery backup.
        kCmAcPowerRescue,            

// begin wacky messages (event plus data) required by and probably 
// only of interest to DSG applications...
//
        // CM has received a DSG-DCD MAC management message.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to BcmOctetBuffer* and be responsible 
        // for deleting the BcmOctetBuffer* message param.  the BcmOctetBuffer*
        // param contains the MAC management message from the beginning
        // of the MAC management header up to but not including the
        // (4) byte MAC CRC.
        kCmRxDsgDcdMsg,

        // CM has transmitted a departure time DCC-RSP message.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // param contains the initialization strategy from the DCC-REQ 
        // message which started the channel change.
        kCmTxDepartureDccRsp,

        // CM has transmitted an arrival time DCC-RSP message.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned char.  the unsigned char
        // param contains the upstream channel id for the channel
        // on which the CM arrived.
        kCmTxArrivalDccRsp,

		// CM has parsed a valid CVC from the config file.
		//
		// NOTE: notification includes data!  at subscription time the 
		// subscriber object must pass a pointer to an object derived 
		// from BcmMsgACT.  at HandleEvent() time subscriber must cast 
		// BcmMsgACT::fpMsgParam to BcmOctetBuffer* and be responsible 
		// for deleting the BcmOctetBuffer* message param.  the BcmOctetBuffer*
		// param contains the valid manufacturer CVC.
		kCmValidManufCvcObtained,
//
// end wacky messages (event plus data) required by and probably 
// only of interest to DSG applications...

        // CM has acquired downstream FEC lock but not time synchronization.
        // message published by BcmCmDocsisCtlThread.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // param contains the CM hardware downstream channel id for the 
        // channel which achieved ds phy lock.
        kCmDsLockOkMsg,

        // CM has failed to acquire or lost downstream FEC lock.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // param contains the CM hardware downstream channel id for the 
        // channel which lost ds phy lock.
        kCmDsLockFailMsg,

        // CM has installed UCD settings for the specified CM upstream
        // transmit channel.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // param contains the CM hardware upstream transmit channel id 
        // for the channel which installed UCD settings.
        kCmUcdSetsOkMsg,

        // CM has established time sync for the specified CM upstream
        // transmit channel.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // param contains the CM hardware upstream transmit channel id 
        // for the channel which established time sync.
        kCmUsTimeRefOkMsg,

        // CM has received RNG-RSP with status==SUCCESS received for 
        // first time since joining the current upstream channel.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // param contains the CM hardware upstream transmit channel id 
        // for the channel which achieved ranging success.
        kCmFirstRngSuccessMsg,

// begin wacky messages (event plus data) required by and probably 
// only of interest to DOCSIS 3.0 CM-STATUS messaging...
//
        // CM has lost downstream phy lock on a non-primary downstream
        // channel.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // param contains the CMTS assigned DOCSIS MAC downstream 
        // channel id for the channel which lost ds phy lock.
        kCmNonPriDsLockFailMsg,

        // CM has recovered downstream phy lock on a non-primary downstream
        // channel.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // param contains the CMTS assigned DOCSIS MAC downstream 
        // channel id for the channel which recovered ds phy lock.
        kCmNonPriDsLockRescueMsg,

        // MDD keep alive failure has occurred on a non-primary downstream
        // channel.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // param contains the CMTS assigned DOCSIS MAC downstream 
        // channel id for the channel on which MDD keep alive failed.
        kCmNonPriDsMddFailMsg,

        // MDD keep alive has recovered on a non-primary downstream
        // channel.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // param contains the CMTS assigned DOCSIS MAC downstream 
        // channel id for the channel on which MDD keep alive recovered.
        kCmNonPriDsMddRescueMsg,

        // CM has received downstream data packet with out of range
        // sequence number for the specified DSID.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // param contains the DSID which experienced the error.
        kCmDsDataReseqFailMsg,

        // CM has recovered from out of range sequence number error
        // for the specified DSID.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // param contains the DSID which experienced the recovery.
        kCmDsDataReseqRescueMsg,

        // T4 no station maintenance timeout failure has occurred 
        // on the specified upstream channel.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // msg param contains the DOCSIS upstream channel id for the 
        // channel on which the T4 failure occurred.
        kCmT4NoStationMaintOpFailMsg,

        // CM has recovered from T4 no station maintenance timeout
        // failure on the specified upstream channel.  Recovery means 
        // that the CM received a unicast ranging opportunity 
        // on a channel which had previously experienced a T4 failure.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // msg param contains the DOCSIS upstream channel id for the 
        // channel on which the T4 recovery occurred.
        kCmT4NoStationMaintOpRescueMsg,

        // max T3 no RNG-RSP retries exceeded failure has occurred 
        // on the specified upstream channel.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // msg param contains the DOCSIS upstream channel id for the 
        // channel on which the T3 failure occurred.
        kCmMaxT3NoRngRspFailMsg,

        // CM has recovered from max T3 no RNG-RSP retries exceeded
        // failure on the specified upstream channel.  Recovery means 
        // that the CM received a RNG-RSP message for a DOCSIS upstream
        // channel which had previously experienced a T3 failure.
        //
        // NOTE: notification includes data!  at subscription time the 
        // subscriber object must pass a pointer to an object derived 
        // from BcmMsgACT.  at HandleEvent() time subscriber must cast 
        // BcmMsgACT::fpMsgParam to unsigned int.  the unsigned int
        // msg param contains the DOCSIS upstream channel id for the 
        // channel on which the T3 recovery occurred.
        kCmMaxT3NoRngRspRescueMsg

// end wacky messages (event plus data) required by and probably 
// only of interest to DOCSIS 3.0 CM-STATUS messaging...
//

    };

    // IsDocsisIpInitStatusEvent() - Returns true if specified event 
    //      is associated with DOCSIS IP initialization.
    //
    static bool IsDocsisIpInitStatusEvent( unsigned int event_code );
};

// returns enum name as char*.  useful for debug output.
const char* CmDocsisStatusEventCodeName( unsigned int event_code );

#endif
