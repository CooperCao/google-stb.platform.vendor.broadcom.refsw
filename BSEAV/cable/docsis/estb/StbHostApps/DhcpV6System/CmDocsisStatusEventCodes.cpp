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
//  Filename:       CmDocsisStatusEventCodes.cpp
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
// CmDocsisStatusEventCodes.cpp
//

#include "CmDocsisStatusEventCodes.h"

const char* CmDocsisStatusEventCodeName( unsigned int event_code )
{
    const char* names[] = 
    {
        "kCmDsScanStarted",

        "kCmDsLockOk",
        "kCmDsLockFail",

        "kCmPriDsMddSetsOk",
        "kCmOtfPriDsMddChangeDetected",
        "kCmPriDsMddSetsReset",
        "kCmPriDsMddKaFail",
        "kCmPriDsMddKaRescue",

        "kCmDsLockSyncOk",
		"kCmDsSyncFail",
        "kCmUcdSetsOk",
        "kCmUsRangingStarted",
        "kCmFirstRngSuccess",
        "kCmRngRspReceived",
        "kCmOperAnyUsTimeRefRescue",

        "kCmDhcpInitStarted",
        "kCmDhcpv6IpAddressAcquiredTentative",
        "kCmDhcpInitOk",
        "kCmDhcpInitFailed",
        "kCmDhcpRenewStarting",     // PR5736
        "kCmDhcpRenewSuccess",      // PR5736
        "kCmDhcpRenewFailed",       // PR2182

        "kCmTftpInitStarted",
        "kCmTftpInitOk",
        "kCmTftpInitFailed",

        "kCmTodInitStarted",
        "kCmTodInitOk",
        "kCmTodInitFailed",

        "kCmSwDownloadStarted",
        "kCmSwDownloadOk",
        "kCmSwDownloadFailed",

        "kCmPreRegConfigFileOk",
        "kCmRegOk",
        "kCmRegFailed",
        "kCmIsOperational",
        "kCmIsNotOperational",
        "kCmResetIp",

        "kCmPostRegXccChanChangeStarted",
        "kCmPostRegXccChanChangeOk",
        
        "kCmDocsisRstErrBeginDsgErrHandling",
        "kCmIsShuttingDown",
        "kCmIsShuttingDownDataPath",

        "kCmDbcOtfRccChange",
        "kCmDbcOtfTccChange",
        "kCmDbcOtfDsidChange",
        "kCmDbcOtfSaidChange",

        "kCmAcPowerFail",
        "kCmEvAcPowerRescue",

        "kCmRxDsgDcdMsg",
        "kCmTxDepartureDccRsp",
        "kCmTxArrivalDccRsp",
        "kCmValidManufCvcObtained",

        "kCmDsLockOkMsg",
        "kCmDsLockFailMsg",

        "kCmUcdSetsOkMsg",
        "kCmUsTimeRefOkMsg",
        "kCmFirstRngSuccessMsg",

        "kCmNonPriDsLockFailMsg",
        "kCmNonPriDsLockRescueMsg",

        "kCmNonPriDsMddFailMsg",
        "kCmNonPriDsMddRescueMsg",
        "kCmDsDataReseqFailMsg",
        "kCmDsDataReseqRescueMsg",

        "kCmT4NoStationMaintOpFailMsg",
        "kCmT4NoStationMaintOpRescueMsg",
        "kCmMaxT3NoRngRspFailMsg",
        "kCmMaxT3NoRngRspRescueMsg"
    };

    if( (event_code >= BcmCmDocsisStatusEventCodes::kCmDsScanStarted) 
        && (event_code <= BcmCmDocsisStatusEventCodes::kCmMaxT3NoRngRspRescueMsg) )
    {
        // event_code value is valid.
        return names[ event_code ];
    }
    else
    {
        return "";
    }
}


bool BcmCmDocsisStatusEventCodes::IsDocsisIpInitStatusEvent( 
    unsigned int event_code ) 
{
    bool ret_val = false;

    switch( event_code )
    {
        case kCmDhcpInitStarted:
        case kCmDhcpv6IpAddressAcquiredTentative:
        case kCmDhcpInitOk:
        case kCmDhcpInitFailed:
        case kCmDhcpRenewStarting:  // PR5736
        case kCmDhcpRenewSuccess:   // PR5736
        case kCmDhcpRenewFailed:    // PR2182
        case kCmTftpInitStarted:
        case kCmTftpInitOk:
        case kCmTftpInitFailed:
        case kCmTodInitStarted:
        case kCmTodInitOk:
        case kCmTodInitFailed:
        case kCmSwDownloadStarted:
        case kCmSwDownloadOk:
        case kCmSwDownloadFailed:
            ret_val = true;
    }

    return ret_val;
}
