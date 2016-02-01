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
//**************************************************************************
//    $Id$
//
//    Filename: EstbEvents.h
//    Author:   Maurice Turcotte
//    Creation Date: 12 MAY 06
//
//**************************************************************************
//    Description:
//
//    Estb event log constants 
//
//    From OC-SP-HOST2.1-CFR-I10-091211 Annex A
//		
//**************************************************************************

#ifndef ESTBEVENTS_H
#define ESTBEVENTS_H

#include "EventLogApi.h" // typedefs, enums, etc


#if (BFC_CM_VENDOR_SUPPORT)
#include "CmVendorEvents.h"
#endif


/*\
*  Standard error codes   
*  See EstbEventLog.cpp for textual descriptions.
\*/

// Open Cable Events

// General TLV Errors
#define kEE_T40101  ((EventLogIdCode) 73040101)
#define kEE_T40102  ((EventLogIdCode) 73040102)
#define kEE_T40103  ((EventLogIdCode) 73040103)

// TLV-1 Failures
#define kEE_T41111  ((EventLogIdCode) 73041111)

// TLV-11 Failures
#define kEE_T41101  ((EventLogIdCode) 73041101)
#define kEE_T41102  ((EventLogIdCode) 73041102)
#define kEE_T41103  ((EventLogIdCode) 73041103)

// TLV 38 Failures
#define kEE_T43801  ((EventLogIdCode) 73043801)
#define kEE_T43802  ((EventLogIdCode) 73043802)
#define kEE_T43803  ((EventLogIdCode) 73043803)
#define kEE_T43804  ((EventLogIdCode) 73043804)

// TLV 43 Failures
#define kEE_T44301  ((EventLogIdCode) 73044301)

// TLV 53 Failures
#define kEE_T45301  ((EventLogIdCode) 73045301)
#define kEE_T45302  ((EventLogIdCode) 73045302)
#define kEE_T45303  ((EventLogIdCode) 73045303)
#define kEE_T45304  ((EventLogIdCode) 73045304)
#define kEE_T45305  ((EventLogIdCode) 73045305)
#define kEE_T45306  ((EventLogIdCode) 73045306)

// TLV 54 Failures
#define kEE_T45401  ((EventLogIdCode) 73045401)
#define kEE_T45402  ((EventLogIdCode) 73045402)
#define kEE_T45403  ((EventLogIdCode) 73045403)
#define kEE_T45404  ((EventLogIdCode) 73045404)
#define kEE_T45405  ((EventLogIdCode) 73045405)

// DHCP Events
#define kEE_D00100   ((EventLogIdCode) 68000100)
#define kEE_D00200   ((EventLogIdCode) 68000200)
#define kEE_D00301   ((EventLogIdCode) 68000301)
#define kEE_D01200   ((EventLogIdCode) 68001200)
#define kEE_D01201   ((EventLogIdCode) 68001201)
#define kEE_D01202   ((EventLogIdCode) 68001202)
#define kEE_D01203   ((EventLogIdCode) 68001203)
#define kEE_D01301   ((EventLogIdCode) 68001301)
#define kEE_D01302   ((EventLogIdCode) 68001302)
#define kEE_D10100   ((EventLogIdCode) 68010100)
#define kEE_D10200   ((EventLogIdCode) 68010200)
#define kEE_D10300   ((EventLogIdCode) 68010300)
#define kEE_D10400   ((EventLogIdCode) 68010400)

// OS/HW/Middleware Initialization Events
#define kEE_B01_0   ((EventLogIdCode) 66000100)
#define kEE_B02_0   ((EventLogIdCode) 66000200)
#define kEE_B03_0   ((EventLogIdCode) 66000300)
#define kEE_B04_0   ((EventLogIdCode) 66000400)
#define kEE_B05_0   ((EventLogIdCode) 66000500)
#define kEE_B06_0   ((EventLogIdCode) 66000600)
#define kEE_B07_0   ((EventLogIdCode) 66000700)
#define kEE_B08_0   ((EventLogIdCode) 66000800)
#define kEE_B09_0   ((EventLogIdCode) 66000900)
#define kEE_B10_0   ((EventLogIdCode) 66001000)
#define kEE_B11_0   ((EventLogIdCode) 66001100)
#define kEE_B12_0   ((EventLogIdCode) 66001200)
#define kEE_B13_0   ((EventLogIdCode) 66001300)
#define kEE_B14_0   ((EventLogIdCode) 66001400)
#define kEE_B15_0   ((EventLogIdCode) 66001500)
#define kEE_B16_0   ((EventLogIdCode) 66001600)
#define kEE_B17_0   ((EventLogIdCode) 66001700)
#define kEE_B18_0   ((EventLogIdCode) 66001800)
#define kEE_B19_0   ((EventLogIdCode) 66001900)
#define kEE_B20_0   ((EventLogIdCode) 66002000)
#define kEE_B21_0   ((EventLogIdCode) 66002100)
#define kEE_B22_0   ((EventLogIdCode) 66002200)
#define kEE_B23_0   ((EventLogIdCode) 66002300)
#define kEE_B24_0   ((EventLogIdCode) 66002400)
#define kEE_B25_0   ((EventLogIdCode) 66002500)
#define kEE_B26_0   ((EventLogIdCode) 66002600)
#define kEE_B27_0   ((EventLogIdCode) 66002700)
#define kEE_B28_0   ((EventLogIdCode) 66002800)
#define kEE_B29_0   ((EventLogIdCode) 66002900)
#define kEE_B30_0   ((EventLogIdCode) 66003000)
#define kEE_B31_0   ((EventLogIdCode) 66003100)


/*\
 *  Broadcom Vendor-Specific codes.
\*/

// This is generic for all#define kinds of software download events.
#define kSoftwareDloadFailure                           BROADCOM_EVENT_ID_BASE + 1
#define kSoftwareDloadSuccess                           BROADCOM_EVENT_ID_BASE + 2

#define kSnmpAgentStartup                               BROADCOM_EVENT_ID_BASE + 3
#define kSnmpProvisioningSetFailure                     BROADCOM_EVENT_ID_BASE + 4
#define kSnmpAuthFailure                                BROADCOM_EVENT_ID_BASE + 5

// Specific dload failure types.
#define kSoftwareDloadFailureBufferReadFailed           BROADCOM_EVENT_ID_BASE + 6
#define kSoftwareDloadFailureImageTooSmall              BROADCOM_EVENT_ID_BASE + 7
#define kSoftwareDloadFailureHcsFailure                 BROADCOM_EVENT_ID_BASE + 8
#define kSoftwareDloadFailureInvalidSignature           BROADCOM_EVENT_ID_BASE + 9
#define kSoftwareDloadFailureImageTooBig                BROADCOM_EVENT_ID_BASE + 10
#define kSoftwareDloadFailureCrcFailure                 BROADCOM_EVENT_ID_BASE + 11
#define kSoftwareDloadFailureImageVerifyFailed          BROADCOM_EVENT_ID_BASE + 12
#define kSoftwareDloadFailureUnsupportedCompressionType BROADCOM_EVENT_ID_BASE + 13

#define kResetting                                      BROADCOM_EVENT_ID_BASE + 14

#define kConfigFileMissingFF                            BROADCOM_EVENT_ID_BASE + 15

#define kPreRegDCCCfgFile                               BROADCOM_EVENT_ID_BASE + 16
#define kPreRegDCCRngRsp                                BROADCOM_EVENT_ID_BASE + 17
#define kRescanAbortedInitalRanging                     BROADCOM_EVENT_ID_BASE + 18
#define kRescanCfgFileBufferAllocFailed                 BROADCOM_EVENT_ID_BASE + 19
#define kRescanCfgFileInvalid                           BROADCOM_EVENT_ID_BASE + 20
#define kRescan11Cfg10Modem                             BROADCOM_EVENT_ID_BASE + 21
#define kRescanIpInitFailure                            BROADCOM_EVENT_ID_BASE + 22
#define kRescanSoftwareUpdateDidntStart                 BROADCOM_EVENT_ID_BASE + 23
#define kRescanRegFailed                                BROADCOM_EVENT_ID_BASE + 24
#define kReceivedUpDisMessage                           BROADCOM_EVENT_ID_BASE + 25

#define kSoftwareDloadRejectedSTB                       BROADCOM_EVENT_ID_BASE + 26


// Enum to provide a hint as to what kind of trap is sent for a given event.
// One item in the enum per trap type.
typedef enum 
{
  kDocsDevCmInitTLVUnknownTrap = 10,
  kDocsDevCmDynServReqFailTrap,
  kDocsDevCmDynServRspFailTrap,
  kDocsDevCmDynServAckFailTrap,
  kDocsDevCmDynamicSATrap,
  kDocsDevCmDHCPFailTrap,
  kDocsDevCmSwUpgradeInitTrap,
  kDocsDevCmSwUpgradeFailTrap,
  kDocsDevCmSwUpgradeSuccessTrap,
  kDocsDevCmSwUpgradeCVCFailTrap,
  kDocsDevCmTODFailTrap,
  kDocsDevCmDCCReqFailTrap,
  kDocsDevCmDCCRspFailTrap,
  kDocsDevCmDCCAckFailTrap,
} estbTrapTypeHint;


/*\ 
 *  Global API for logging events to the eSTBEventLog
\*/

#if defined (__cplusplus)
// These API's are C++ only because of the default arguments and overloading.
SNMP_STATUS EstbLogEvent (EventLogIdCode Id, const char *pParam1, const char *pParam2 = NULL, const char *pParam3 = NULL, const char *pParam4 = NULL, const char *pParam5 = NULL);
SNMP_STATUS EstbLogEvent (EventLogIdCode Id, EventLogSeverityLevel Level = kUseDefaultLevel, const char *pText = NULL, EventLogPolicy Policy = kUseDefaultPolicy);
SNMP_STATUS EstbSendTrap (int TrapType, int IfIndex = 0);
#endif

#if defined (__cplusplus)
extern "C"
{
#endif
// These APIs may be used by C++ or C modules.  If we need to, we could
// expand this list of APIs to handle other permutations of the C++ functions.
SNMP_STATUS EstbLogEventIdOnly (EventLogIdCode Id);
SNMP_STATUS EstbLogEventIdPlusText (EventLogIdCode Id, const char *pParam);
#if defined (__cplusplus)
}
#endif

#endif
