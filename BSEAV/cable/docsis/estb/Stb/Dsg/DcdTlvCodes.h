//****************************************************************************
//
// Copyright (c) 2007-2011 Broadcom Corporation
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
//  Filename:       DcdTlvCodes.h
//  Author:         Pinar Taskiran
//  Creation Date:  Aug 12, 2004
//
//****************************************************************************
//  Description:
//      This file contains the API that handles
//      parsing of DCD messages, including TLVs.
//
//****************************************************************************

#ifndef DCDTLVCODES_H
#define DCDTLVCODES_H

//********************** Include Files ***************************************

//#include "DcdTlvParser.h"

//********************** Global Constants ************************************

// Use this as the length field in the TLVTable if you don't want length
// validation to be done for this type during parsing.
#define kUnknownLength 0

// Other type values.
#define kEndOfDataMarker 255

#ifndef TLVCODES_H
// This is the TLVTable entry that must be the last item in every table.  Note
// that I would have preferred to use the const form that is commented out, but
// the stupid Microsoft compiler considers it an error, so I have to use the
// #defined version.
//const TLVTable kEndOfTLVTable = {kEndOfDataMarker, kUnknownLength, kTLV_end_of_data, (void *) 0, false, false, "Unknown", NULL};
#define kEndOfTLVTable {kEndOfDataMarker, kUnknownLength, kTLV_end_of_data, "Unknown", kFalse, (void *) 0, NULL}
#endif

// Top level type codes for DCD message.  
// Refer to DOCSIS DSG Specification.
typedef enum
{
    kDsPacketClassificationSets = 23, // sub-type
	kDsgRuleSets				= 50, // sub-type
	kDsgConfigurationSets		= 51  // sub-type
} DcdTopLevelTypeCodes;

// Sub-Type codes common to Downstream Packet Classifier settings
typedef enum
{
    kClassifierId		= 2,
#ifndef TLVCODES_H
    kRulePriority		= 5,
    kIpClassifierSets	= 9 // sub-type
#endif
} DcdSubLevelTypeCodes;

#ifndef TLVCODES_H
// Sub-Type codes for IP Classifier settings
typedef enum
{
    kSourceIpAddr = 3,
    kSourceIpMask = 4,
    kDestIpAddr = 5,
    kDestPortStart = 9,     //ECR-81804
    kDestPortEnd = 10       //ECR-81804
} IpClassifierCodes;
#endif

// Sub-Type codes common to Dsg Rule Settings
typedef enum
{
    kDsgRuleId			= 1,
    kDsgRulePriority	= 2,
    kDsgUcidRange		= 3,
	kDsgClientIdSets	= 4, // sub-type
	kDsgTunnelAddress	= 5,
	kDsgClassifierId	= 6,
	kDsgVendorSpec		= 43
} DsgRuleCodes;

// Sub-Type codes common to DsgClientId  Settings
typedef enum
{
    kUnknown                = 0,
    kBroadcast				= 1,
    kWellKnownMacAddress	= 2,
    kCaSystemId				= 3,
	kApplicationId			= 4
} DsgClientIdCodes;

// Sub-Type codes common to Dsg Configuration Settings
typedef enum
{
    kChannelList			= 1,
    kInitTimeout			= 2,
    kOperationalTimeout		= 3,
	kTwoWayRetryTimeout		= 4, // sub-type
	kOneWayRetryTimeout		= 5,
	kVendorSpec				= 43
} DsgConfigurationCodes;


#endif

