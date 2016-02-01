//****************************************************************************
//
// Copyright (c) 2008-2012 Broadcom Corporation
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
//  Filename:       EstbDhcpV6Defaults.h
//  Author:         Pinar Taskiran
//  Creation Date:  January 20, 2008
//
//****************************************************************************
//  Description:
//      This file contains the default values that are to be used for nonvol
//      settings and as compile-time defaults.  This file controls the behavior
//      of the settings (hard-coded vs use nonvol) as well as specifying the
//      default values.
//
//****************************************************************************

#ifndef EstbDhcpV6Defaults_H
#define EstbDhcpV6Defaults_H

//********************** Include Files ***************************************

//********************** Global Types ****************************************

//********************** Global Constants ************************************

#if (BFC_INCLUDE_DHCPV6_SUPPORT)

#define kDefault_NumberOfInterfaces	1

#define kDefault_DhcpInterfaces {"eth0"}

#define kDefault_PrimaryDhcpInterface "eth0"

#define kDefault_NumberOfOptions	5

//kDhcpV6OptionClientIdentifier, kDhcpV6OptionIaNa
#define kDefault_DhcpPolicy {kDhcpV6OptionReconfigureAccept, kDhcpV6OptionRequestOption, \
kDhcpV6OptionVendorClass, kDhcpV6OptionVendorSpecificInfo, kDhcpV6OptionRapidCommit}

#define kDefault_NumberOfOptionRequestOptions	2
#define kDefault_DhcpOptionRequestOptionPolicy {kDhcpV6OptionDNSRecursiveNameServer, kDhcpV6OptionDomainList}

#define kDefault_VendorClassString "rng1.0"

#define kDefault_EstbSysDescr "BFC cablemodem reference design <<HW_REV: V1.0; VENDOR: Broadcom; BOOTR: 0.1.1e; SW_REV: 4.4.1; MODEL: BCM93255eSTB>>";
#define kDefault_DeviceSerialNumber "S/N1234567"

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************

//********************** Inline Method Implementations ***********************
#endif //     #if (BFC_INCLUDE_DHCPV6_SUPPORT)

#endif



