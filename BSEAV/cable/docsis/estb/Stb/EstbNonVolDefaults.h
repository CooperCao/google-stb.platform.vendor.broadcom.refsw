//****************************************************************************
//
// Copyright (c) 2004-2009 Broadcom Corporation
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
//  Filename:       EstbNonVolDefaults.h
//  Author:         Kevin O'Neal
//  Creation Date:  October 14, 2004
//
//****************************************************************************
//  Description:
//      This file contains the OpenCable-specific default values that are to be
//      used for nonvol settings and as compile-time defaults.  This file adds
//      to or overrides the values in BfcNonVolDefaults.h.  The build script
//      will merge this file with that one when producing MergedNonVolDefaults.h
//
//****************************************************************************
 
#ifndef EstbNonVolDefaults_H
#define EstbNonVolDefaults_H

//********************** Include Files ***************************************

//#include "EstbNonVolSettings.h"
//#include "dsg_api.h"
//********************** Global Constants ************************************

// If using the CM default run-time sysDescr function (default for BRCM builds)
// we can specify an additional model tag to append to the model number here.
#undef kCmSysDescrModelTag
#define kCmSysDescrModelTag "eSTB"


// PR 8572
// DSG mode: 
//	kDsgDisabledMode = 0,
//	kDsgBasicMode,
//	kDsgAdvanceMode
//
#define kDefaultValue_dsgMode kDsgAdvanceMode

// The customer can change the "eCM" IP stack address and subnet to suit
// their needs.  Most of the time, you will want to use the
// 192.168.100.1 address and the default subnet mask.  
//
// For the router address, if the OS is pSOS, then you will need to leave the
// address set to 0.0.0.0; otherwise it will need to be set to an on-net address.
// 
// None of the other settings can be modified.

// These are all IP addresses in string values.


// Unsigned integer value from 2-4.  Other values are reserved and must not
// be used.  This is the stack number eCM will use setup sockets to communicate 
// with eSTB. 

#if defined(IP_NONVOL_OVER_ETHERNET)
#define kDefaultValue_ecmRemoteAccessSubnetMask    "255.255.252.0"
#define kDefaultValue_ecmRemoteAccessRouterAddress "10.24.4.1"
#else
#define kDefaultValue_ecmRemoteAccessSubnetMask    "255.255.255.0"
#define kDefaultValue_ecmRemoteAccessRouterAddress "10.10.10.254"
#endif

// IP Stack 3 is the designated IP Stack for eCM-eSTB socket communication.
#undef kDefaultValue_IpStack3Enabled
#define kDefaultValue_IpStack3Enabled    			true

// DSG IP Stack number is 3 (defined in dsg_api.h)
#define kDefaultValue_ecmRemoteAccessIpStackNumber	kEstbIpStackNumber

// " ********************************************************** "
// " ATTENTION: NONVOL DEFAULTS FOR eCM-eSTB PHYSICAL INTERFACE "
// " ********************************************************** "
// These are only useful in Dual Processor design.
#define kUsb				0
#define kEth				1
#define kPci				2
#define kScb				3
#define kVirtual			4
// Select the Interface to be used for eCM-eSTB communication (USB or ETH)
// USB: Use kUsb	
// ETH: Use kEth

#define kDefaultValue_dsgTransportHal				kUsb					
#define kDefaultValue_ipTransportHal				kUsb	

// If the platform includes a managed switch,
// the ethernet port number to be used by the IP Transport Hal 
// is kDefaultValue_EthernetPortNumber.
// Default value is zero for data-only eCMs.
#define kDefaultValue_EthernetPortNumber			0

// IP Stack 4 is the designated IP Stack to proxy Cablecard IP traffic 
// from eSTB to eCM using socket communication.
#undef kDefaultValue_IpStack4Enabled
#define kDefaultValue_IpStack4Enabled    			true

#define kDefaultValue_downloadMode					0 // kEnableCarousel | kEnableSecure;

#define kDefaultValue_preRegSnmpDloadEnabled		true

//#define kDefaultValue_OpenCableBpiMfgVendorName		"Broadcom Corporation"

#define kDefaultValue_EstbConfigFilePath			"estbconfigsettings.bin"

// Ecm Mode:
//	enum
//	{
//	    kEcmDocsisMode = 0,
//  	kEcmReservedMode,
//  	kEcmNormalMode
//	};
#define kDefaultValue_EcmMode						kEcmNormalMode

#define kDefaultValue_VideoSecurityType				"CARD"	//	SM, EMBED

// Optimized fast scan default symbol rate check range
#define kDefaultValue_FastScanRateOffsetCheckRange  1000

// Overwrite the default and disable this feature
#undef kDefaultValue_NumberOfFailedScansBeforeReboot
#define kDefaultValue_NumberOfFailedScansBeforeReboot 0

#endif
