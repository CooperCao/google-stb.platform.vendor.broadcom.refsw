//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
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
//  Filename:       EstbTLVcodes.h
//  Author:         Maurice Turcotte
//  Creation Date:  8 NOV 2006
//
//****************************************************************************
//  Description:
//      This file contains the eSTB type values that are
//      defined for the T portion of TLV codes.
//
//****************************************************************************

#ifndef ESTBTLVCODES_H
#define ESTBTLVCODES_H

//********************** Include Files ***************************************

//********************** Global Types ****************************************

//********************** Global Constants ************************************

// Top level type codes specific to Estb configuration file.
typedef enum
{
    kEndOfData = 255,
    kPad = 0,
    kSnmpMibObject = 11,
    kSnmpV3NotifyReceiver = 38,
        kSnmpNotifyIp = 1,
        kSnmpNotifyPort = 2,
        kSnmpNotifyType = 3,
        kSnmpNotifyTimeout = 4,
        kSnmpNotifyRetries = 5,
        kSnmpNotifyFilterParams = 6,
        kSnmpNotifySecurityName = 7,
    kSnmpV1V2Coexistence = 53, 
        kSnmpV1V2CommunityName = 1, 
        kSnmpV1V2Transport = 2, 
            kSnmpV1V2TransportAddress = 1, 
            kSnmpV1V2TransportAddressMask = 2, 
        kSnmpV1V2ViewType = 3, 
        kSnmpV1V2ViewName = 4, 
    kSnmpV3AccessView = 54, 
        kSnmpV3AccessViewName = 1, 
        kSnmpV3AccessViewSubtree = 2, 
        kSnmpV3AccessViewMask = 3, 
        kSnmpV3AccessViewType = 4,
    kIpModeControl = 1, 
    kVendorSpecific = 43, 
        kVendorId = 8, 
    kEstbConfig = 217
} CmConfigSpecificTypeCodes;

// value codes for Config TLV parsing...
typedef enum
{
    kConfOK = 0,
    kConfMissingTLV217 = 1,
    kConfTooBig = 2,
    kConfNoEOF = 3,
    kConfDupeOID = 4,
    kConfBadValue = 5,
    kConfBadOID = 6,
    kConfBadType = 7,
    kConfMissingParam = 8,
    kConfSyntaxError = 9,
    kConfRejOther = 10

} ConfigParseCodes;



// Usage context codes used for configuration settings used in 
// CM configuration files, registration, and DSx messaging.
typedef enum
{
    // for contexts kCmConfigFile thru kRegAck, the context dictates
    // which side tx the msg and which side rx the msg.
    kEstbConfigFile = 1,
    kUcEndOfList
} UsageContext;


// returns enum name as char*.  useful for debug output.
const char* UsageContextName( int context );
const char* ConfigErrorCodeName( int conf_code );

#ifdef NOTDEF
//
//
// EstbConfCodeToSnmpEventLogCode() - Converts Estb config code
//      to SNMP event log code for.
//
int EstbConfCodeToSnmpEventLogCode( int docsis_conf_code );
#endif

// common name strings for debug output.
extern char* spec;                 // setting value specified
extern char* no_spec;              // setting value not specified
extern char* reserved_value;      
extern char* undefined_value;  

// Other type values.
#define kEndOfDataMarker 255

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************

//********************** Inline Method Implementations ***********************

#endif


