//****************************************************************************
//
// Copyright (c) 2002-2009 Broadcom Corporation
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
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: ipMib.h
//    Creation Date: Thursday, April 19, 2007 at 16:18:15
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.0b
//    from input file ip-mib_rfc4293.mib
//
//**************************************************************************

#ifndef IPMIB_H
#define IPMIB_H

#include "MibObjs.h"
#include "MibBridge.h"

#include "HalIfFactory.h" // for BCM_NUM_IP_STACKS
//#include "ipBridge.h"
class ipv6InterfaceEntryBridge;

/*\
 *  ipGroup group
\*/

// OID constants
#define kOID_ipGroup  "1.3.6.1.2.1.4"
#define kOID_ipForwarding  "1.3.6.1.2.1.4.1.0"
#define kOID_ipDefaultTTL  "1.3.6.1.2.1.4.2.0"
#define kOID_ipReasmTimeout  "1.3.6.1.2.1.4.13.0"
#define kOID_ipv6IpForwarding  "1.3.6.1.2.1.4.25.0"
#define kOID_ipv6IpDefaultHopLimit  "1.3.6.1.2.1.4.26.0"
#define kOID_ipv4InterfaceTableLastChange  "1.3.6.1.2.1.4.27.0"
#define kOID_ipv6InterfaceTableLastChange  "1.3.6.1.2.1.4.29.0"
#define kOID_ipAddressSpinLock  "1.3.6.1.2.1.4.33.0"
#define kOID_ipv6RouterAdvertSpinLock  "1.3.6.1.2.1.4.38.0"
#define kOID_ipInReceives  "1.3.6.1.2.1.4.3.0"
#define kOID_ipInHdrErrors  "1.3.6.1.2.1.4.4.0"
#define kOID_ipInAddrErrors  "1.3.6.1.2.1.4.5.0"
#define kOID_ipForwDatagrams  "1.3.6.1.2.1.4.6.0"
#define kOID_ipInUnknownProtos  "1.3.6.1.2.1.4.7.0"
#define kOID_ipInDiscards  "1.3.6.1.2.1.4.8.0"
#define kOID_ipInDelivers  "1.3.6.1.2.1.4.9.0"
#define kOID_ipOutRequests  "1.3.6.1.2.1.4.10.0"
#define kOID_ipOutDiscards  "1.3.6.1.2.1.4.11.0"
#define kOID_ipOutNoRoutes  "1.3.6.1.2.1.4.12.0"
#define kOID_ipReasmReqds  "1.3.6.1.2.1.4.14.0"
#define kOID_ipReasmOKs  "1.3.6.1.2.1.4.15.0"
#define kOID_ipReasmFails  "1.3.6.1.2.1.4.16.0"
#define kOID_ipFragOKs  "1.3.6.1.2.1.4.17.0"
#define kOID_ipFragFails  "1.3.6.1.2.1.4.18.0"
#define kOID_ipFragCreates  "1.3.6.1.2.1.4.19.0"
#define kOID_ipRoutingDiscards  "1.3.6.1.2.1.4.23.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipGroup  "ipGroup"
  #define kNAME_ipForwarding  "ipForwarding"
  #define kNAME_ipDefaultTTL  "ipDefaultTTL"
  #define kNAME_ipReasmTimeout  "ipReasmTimeout"
  #define kNAME_ipv6IpForwarding  "ipv6IpForwarding"
  #define kNAME_ipv6IpDefaultHopLimit  "ipv6IpDefaultHopLimit"
  #define kNAME_ipv4InterfaceTableLastChange  "ipv4InterfaceTableLastChange"
  #define kNAME_ipv6InterfaceTableLastChange  "ipv6InterfaceTableLastChange"
  #define kNAME_ipAddressSpinLock  "ipAddressSpinLock"
  #define kNAME_ipv6RouterAdvertSpinLock  "ipv6RouterAdvertSpinLock"
  #define kNAME_ipInReceives  "ipInReceives"
  #define kNAME_ipInHdrErrors  "ipInHdrErrors"
  #define kNAME_ipInAddrErrors  "ipInAddrErrors"
  #define kNAME_ipForwDatagrams  "ipForwDatagrams"
  #define kNAME_ipInUnknownProtos  "ipInUnknownProtos"
  #define kNAME_ipInDiscards  "ipInDiscards"
  #define kNAME_ipInDelivers  "ipInDelivers"
  #define kNAME_ipOutRequests  "ipOutRequests"
  #define kNAME_ipOutDiscards  "ipOutDiscards"
  #define kNAME_ipOutNoRoutes  "ipOutNoRoutes"
  #define kNAME_ipReasmReqds  "ipReasmReqds"
  #define kNAME_ipReasmOKs  "ipReasmOKs"
  #define kNAME_ipReasmFails  "ipReasmFails"
  #define kNAME_ipFragOKs  "ipFragOKs"
  #define kNAME_ipFragFails  "ipFragFails"
  #define kNAME_ipFragCreates  "ipFragCreates"
  #define kNAME_ipRoutingDiscards  "ipRoutingDiscards"
#else
  #define kNAME_ipGroup  kOID_ipGroup
  #define kNAME_ipForwarding  kOID_ipForwarding
  #define kNAME_ipDefaultTTL  kOID_ipDefaultTTL
  #define kNAME_ipReasmTimeout  kOID_ipReasmTimeout
  #define kNAME_ipv6IpForwarding  kOID_ipv6IpForwarding
  #define kNAME_ipv6IpDefaultHopLimit  kOID_ipv6IpDefaultHopLimit
  #define kNAME_ipv4InterfaceTableLastChange  kOID_ipv4InterfaceTableLastChange
  #define kNAME_ipv6InterfaceTableLastChange  kOID_ipv6InterfaceTableLastChange
  #define kNAME_ipAddressSpinLock  kOID_ipAddressSpinLock
  #define kNAME_ipv6RouterAdvertSpinLock  kOID_ipv6RouterAdvertSpinLock
  #define kNAME_ipInReceives  kOID_ipInReceives
  #define kNAME_ipInHdrErrors  kOID_ipInHdrErrors
  #define kNAME_ipInAddrErrors  kOID_ipInAddrErrors
  #define kNAME_ipForwDatagrams  kOID_ipForwDatagrams
  #define kNAME_ipInUnknownProtos  kOID_ipInUnknownProtos
  #define kNAME_ipInDiscards  kOID_ipInDiscards
  #define kNAME_ipInDelivers  kOID_ipInDelivers
  #define kNAME_ipOutRequests  kOID_ipOutRequests
  #define kNAME_ipOutDiscards  kOID_ipOutDiscards
  #define kNAME_ipOutNoRoutes  kOID_ipOutNoRoutes
  #define kNAME_ipReasmReqds  kOID_ipReasmReqds
  #define kNAME_ipReasmOKs  kOID_ipReasmOKs
  #define kNAME_ipReasmFails  kOID_ipReasmFails
  #define kNAME_ipFragOKs  kOID_ipFragOKs
  #define kNAME_ipFragFails  kOID_ipFragFails
  #define kNAME_ipFragCreates  kOID_ipFragCreates
  #define kNAME_ipRoutingDiscards  kOID_ipRoutingDiscards
#endif

class ipGroupBridge;
class ipGroup : public BcmMibGroup
{
  friend class ipGroupBridge;

  public:
    ipGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ipGroupBridge *fpBridge;

    enum
    {
      kLEAF_ipForwarding = 1,
      kLEAF_ipDefaultTTL = 2,
      kLEAF_ipReasmTimeout = 13,
      kLEAF_ipv6IpForwarding = 25,
      kLEAF_ipv6IpDefaultHopLimit = 26,
      kLEAF_ipv4InterfaceTableLastChange = 27,
      kLEAF_ipv6InterfaceTableLastChange = 29,
      kLEAF_ipAddressSpinLock = 33,
      kLEAF_ipv6RouterAdvertSpinLock = 38,
      kLEAF_ipInReceives = 3,
      kLEAF_ipInHdrErrors = 4,
      kLEAF_ipInAddrErrors = 5,
      kLEAF_ipForwDatagrams = 6,
      kLEAF_ipInUnknownProtos = 7,
      kLEAF_ipInDiscards = 8,
      kLEAF_ipInDelivers = 9,
      kLEAF_ipOutRequests = 10,
      kLEAF_ipOutDiscards = 11,
      kLEAF_ipOutNoRoutes = 12,
      kLEAF_ipReasmReqds = 14,
      kLEAF_ipReasmOKs = 15,
      kLEAF_ipReasmFails = 16,
      kLEAF_ipFragOKs = 17,
      kLEAF_ipFragFails = 18,
      kLEAF_ipFragCreates = 19,
      kLEAF_ipRoutingDiscards = 23
    };
};


/* ipv4InterfaceTable and related objects */

// OID constants
#define kOID_ipv4InterfaceTable  "1.3.6.1.2.1.4.28"
#define kOID_ipv4InterfaceEntry  "1.3.6.1.2.1.4.28.1"
#define kOID_ipv4InterfaceIfIndex  "1.3.6.1.2.1.4.28.1.1"
#define kOID_ipv4InterfaceReasmMaxSize  "1.3.6.1.2.1.4.28.1.2"
#define kOID_ipv4InterfaceEnableStatus  "1.3.6.1.2.1.4.28.1.3"
#define kOID_ipv4InterfaceRetransmitTime  "1.3.6.1.2.1.4.28.1.4"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipv4InterfaceTable  "ipv4InterfaceTable"
  #define kNAME_ipv4InterfaceEntry  "ipv4InterfaceEntry"
  #define kNAME_ipv4InterfaceIfIndex  "ipv4InterfaceIfIndex"
  #define kNAME_ipv4InterfaceReasmMaxSize  "ipv4InterfaceReasmMaxSize"
  #define kNAME_ipv4InterfaceEnableStatus  "ipv4InterfaceEnableStatus"
  #define kNAME_ipv4InterfaceRetransmitTime  "ipv4InterfaceRetransmitTime"
#else
  #define kNAME_ipv4InterfaceTable  kOID_ipv4InterfaceTable
  #define kNAME_ipv4InterfaceEntry  kOID_ipv4InterfaceEntry
  #define kNAME_ipv4InterfaceIfIndex  kOID_ipv4InterfaceIfIndex
  #define kNAME_ipv4InterfaceReasmMaxSize  kOID_ipv4InterfaceReasmMaxSize
  #define kNAME_ipv4InterfaceEnableStatus  kOID_ipv4InterfaceEnableStatus
  #define kNAME_ipv4InterfaceRetransmitTime  kOID_ipv4InterfaceRetransmitTime
#endif

class BcmBaseIpHalIf;

class ipv4InterfaceTable : public BcmMibTable
{
  public:
    ipv4InterfaceTable ();
    
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    
    // Methods to map an IP stack HAL to an ifIndex
    bool IpIfIndex (const BcmBaseIpHalIf *pIpHal, int ifIndex);
    int IpIfIndex (const BcmBaseIpHalIf *pIpHal) const;
    
  protected:
    
    // Override virtual Update method to allow us to be self-updating.
    void Update ();
    
  protected:
    
    // Simple array to do the IP hal / ifIndex mapping.  No need for anything
    // more sophisticated since BCM_NUM_IP_STACKS is small.
    int fIpHalMap[8];
    bool fInitialized;
};

class ipv4InterfaceEntryBridge;
class ipv4InterfaceEntry : public BcmMibEntry
{
  friend class ipv4InterfaceEntryBridge;
  friend class ipv4InterfaceTable;

  public:
    ipv4InterfaceEntry (ipv4InterfaceTable *pTable, ipv4InterfaceEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    
  protected:
    ipv4InterfaceEntryBridge *fpBridge;

    enum
    {
      kLEAF_ipv4InterfaceIfIndex = 1,
      kLEAF_ipv4InterfaceReasmMaxSize = 2,
      kLEAF_ipv4InterfaceEnableStatus = 3,
      kLEAF_ipv4InterfaceRetransmitTime = 4
    };
};


#if (BCM_IPV6_SUPPORT)

/* ipv6InterfaceTable and related objects */

// OID constants
#define kOID_ipv6InterfaceTable  "1.3.6.1.2.1.4.30"
#define kOID_ipv6InterfaceEntry  "1.3.6.1.2.1.4.30.1"
#define kOID_ipv6InterfaceIfIndex  "1.3.6.1.2.1.4.30.1.1"
#define kOID_ipv6InterfaceReasmMaxSize  "1.3.6.1.2.1.4.30.1.2"
#define kOID_ipv6InterfaceIdentifier  "1.3.6.1.2.1.4.30.1.3"
#define kOID_ipv6InterfaceEnableStatus  "1.3.6.1.2.1.4.30.1.5"
#define kOID_ipv6InterfaceReachableTime  "1.3.6.1.2.1.4.30.1.6"
#define kOID_ipv6InterfaceRetransmitTime  "1.3.6.1.2.1.4.30.1.7"
#define kOID_ipv6InterfaceForwarding  "1.3.6.1.2.1.4.30.1.8"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipv6InterfaceTable  "ipv6InterfaceTable"
  #define kNAME_ipv6InterfaceEntry  "ipv6InterfaceEntry"
  #define kNAME_ipv6InterfaceIfIndex  "ipv6InterfaceIfIndex"
  #define kNAME_ipv6InterfaceReasmMaxSize  "ipv6InterfaceReasmMaxSize"
  #define kNAME_ipv6InterfaceIdentifier  "ipv6InterfaceIdentifier"
  #define kNAME_ipv6InterfaceEnableStatus  "ipv6InterfaceEnableStatus"
  #define kNAME_ipv6InterfaceReachableTime  "ipv6InterfaceReachableTime"
  #define kNAME_ipv6InterfaceRetransmitTime  "ipv6InterfaceRetransmitTime"
  #define kNAME_ipv6InterfaceForwarding  "ipv6InterfaceForwarding"
#else
  #define kNAME_ipv6InterfaceTable  kOID_ipv6InterfaceTable
  #define kNAME_ipv6InterfaceEntry  kOID_ipv6InterfaceEntry
  #define kNAME_ipv6InterfaceIfIndex  kOID_ipv6InterfaceIfIndex
  #define kNAME_ipv6InterfaceReasmMaxSize  kOID_ipv6InterfaceReasmMaxSize
  #define kNAME_ipv6InterfaceIdentifier  kOID_ipv6InterfaceIdentifier
  #define kNAME_ipv6InterfaceEnableStatus  kOID_ipv6InterfaceEnableStatus
  #define kNAME_ipv6InterfaceReachableTime  kOID_ipv6InterfaceReachableTime
  #define kNAME_ipv6InterfaceRetransmitTime  kOID_ipv6InterfaceRetransmitTime
  #define kNAME_ipv6InterfaceForwarding  kOID_ipv6InterfaceForwarding
#endif

class ipv6InterfaceTable : public BcmMibTable
{
  public:
    ipv6InterfaceTable ();
    
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    
    // Methods to map an IP stack HAL to an ifIndex
    bool IpIfIndex (const BcmBaseIpHalIf *pIpHal, int ifIndex);
    int IpIfIndex (const BcmBaseIpHalIf *pIpHal) const;
    
  protected:
    
    // Override virtual Update method to allow us to be self-updating.
    void Update ();
    
  protected:
    
    // Simple array to do the IP hal / ifIndex mapping.  No need for anything
    // more sophisticated since BCM_NUM_IP_STACKS is small.
    int fIpHalMap[8];
    bool fInitialized;
};

class ipv6InterfaceEntry : public BcmMibEntry
{
  friend class ipv6InterfaceEntryBridge;
  friend class ipv6InterfaceTable;

  public:
    ipv6InterfaceEntry (ipv6InterfaceTable *pTable, ipv6InterfaceEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ipv6InterfaceEntryBridge *fpBridge;

    enum
    {
      kLEAF_ipv6InterfaceIfIndex = 1,
      kLEAF_ipv6InterfaceReasmMaxSize = 2,
      kLEAF_ipv6InterfaceIdentifier = 3,
      kLEAF_ipv6InterfaceEnableStatus = 5,
      kLEAF_ipv6InterfaceReachableTime = 6,
      kLEAF_ipv6InterfaceRetransmitTime = 7,
      kLEAF_ipv6InterfaceForwarding = 8
    };
};

#endif

#if 0

/* ipAddressPrefixTable and related objects */

// OID constants
#define kOID_ipAddressPrefixTable  "1.3.6.1.2.1.4.32"
#define kOID_ipAddressPrefixEntry  "1.3.6.1.2.1.4.32.1"
#define kOID_ipAddressPrefixIfIndex  "1.3.6.1.2.1.4.32.1.1"
#define kOID_ipAddressPrefixType  "1.3.6.1.2.1.4.32.1.2"
#define kOID_ipAddressPrefixPrefix  "1.3.6.1.2.1.4.32.1.3"
#define kOID_ipAddressPrefixLength  "1.3.6.1.2.1.4.32.1.4"
#define kOID_ipAddressPrefixOrigin  "1.3.6.1.2.1.4.32.1.5"
#define kOID_ipAddressPrefixOnLinkFlag  "1.3.6.1.2.1.4.32.1.6"
#define kOID_ipAddressPrefixAutonomousFlag  "1.3.6.1.2.1.4.32.1.7"
#define kOID_ipAddressPrefixAdvPreferredLifetime  "1.3.6.1.2.1.4.32.1.8"
#define kOID_ipAddressPrefixAdvValidLifetime  "1.3.6.1.2.1.4.32.1.9"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipAddressPrefixTable  "ipAddressPrefixTable"
  #define kNAME_ipAddressPrefixEntry  "ipAddressPrefixEntry"
  #define kNAME_ipAddressPrefixIfIndex  "ipAddressPrefixIfIndex"
  #define kNAME_ipAddressPrefixType  "ipAddressPrefixType"
  #define kNAME_ipAddressPrefixPrefix  "ipAddressPrefixPrefix"
  #define kNAME_ipAddressPrefixLength  "ipAddressPrefixLength"
  #define kNAME_ipAddressPrefixOrigin  "ipAddressPrefixOrigin"
  #define kNAME_ipAddressPrefixOnLinkFlag  "ipAddressPrefixOnLinkFlag"
  #define kNAME_ipAddressPrefixAutonomousFlag  "ipAddressPrefixAutonomousFlag"
  #define kNAME_ipAddressPrefixAdvPreferredLifetime  "ipAddressPrefixAdvPreferredLifetime"
  #define kNAME_ipAddressPrefixAdvValidLifetime  "ipAddressPrefixAdvValidLifetime"
#else
  #define kNAME_ipAddressPrefixTable  kOID_ipAddressPrefixTable
  #define kNAME_ipAddressPrefixEntry  kOID_ipAddressPrefixEntry
  #define kNAME_ipAddressPrefixIfIndex  kOID_ipAddressPrefixIfIndex
  #define kNAME_ipAddressPrefixType  kOID_ipAddressPrefixType
  #define kNAME_ipAddressPrefixPrefix  kOID_ipAddressPrefixPrefix
  #define kNAME_ipAddressPrefixLength  kOID_ipAddressPrefixLength
  #define kNAME_ipAddressPrefixOrigin  kOID_ipAddressPrefixOrigin
  #define kNAME_ipAddressPrefixOnLinkFlag  kOID_ipAddressPrefixOnLinkFlag
  #define kNAME_ipAddressPrefixAutonomousFlag  kOID_ipAddressPrefixAutonomousFlag
  #define kNAME_ipAddressPrefixAdvPreferredLifetime  kOID_ipAddressPrefixAdvPreferredLifetime
  #define kNAME_ipAddressPrefixAdvValidLifetime  kOID_ipAddressPrefixAdvValidLifetime
#endif

class ipAddressPrefixTable : public BcmMibTable
{
  public:
    ipAddressPrefixTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ipAddressPrefixEntry : public BcmMibEntry
{
  friend class ipAddressPrefixEntryBridge;
  friend class ipAddressPrefixTable;

  public:
    ipAddressPrefixEntry (ipAddressPrefixTable *pTable, ipAddressPrefixEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ipAddressPrefixEntryBridge *fpBridge;

    enum
    {
      kLEAF_ipAddressPrefixIfIndex = 1,
      kLEAF_ipAddressPrefixType = 2,
      kLEAF_ipAddressPrefixPrefix = 3,
      kLEAF_ipAddressPrefixLength = 4,
      kLEAF_ipAddressPrefixOrigin = 5,
      kLEAF_ipAddressPrefixOnLinkFlag = 6,
      kLEAF_ipAddressPrefixAutonomousFlag = 7,
      kLEAF_ipAddressPrefixAdvPreferredLifetime = 8,
      kLEAF_ipAddressPrefixAdvValidLifetime = 9
    };
};

#endif


/* ipAddressTable and related objects */

// OID constants
#define kOID_ipAddressTable  "1.3.6.1.2.1.4.34"
#define kOID_ipAddressEntry  "1.3.6.1.2.1.4.34.1"
#define kOID_ipAddressAddrType  "1.3.6.1.2.1.4.34.1.1"
#define kOID_ipAddressAddr  "1.3.6.1.2.1.4.34.1.2"
#define kOID_ipAddressIfIndex  "1.3.6.1.2.1.4.34.1.3"
#define kOID_ipAddressType  "1.3.6.1.2.1.4.34.1.4"
#define kOID_ipAddressPrefix  "1.3.6.1.2.1.4.34.1.5"
#define kOID_ipAddressOrigin  "1.3.6.1.2.1.4.34.1.6"
#define kOID_ipAddressStatus  "1.3.6.1.2.1.4.34.1.7"
#define kOID_ipAddressCreated  "1.3.6.1.2.1.4.34.1.8"
#define kOID_ipAddressLastChanged  "1.3.6.1.2.1.4.34.1.9"
#define kOID_ipAddressRowStatus  "1.3.6.1.2.1.4.34.1.10"
#define kOID_ipAddressStorageType  "1.3.6.1.2.1.4.34.1.11"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipAddressTable  "ipAddressTable"
  #define kNAME_ipAddressEntry  "ipAddressEntry"
  #define kNAME_ipAddressAddrType  "ipAddressAddrType"
  #define kNAME_ipAddressAddr  "ipAddressAddr"
  #define kNAME_ipAddressIfIndex  "ipAddressIfIndex"
  #define kNAME_ipAddressType  "ipAddressType"
  #define kNAME_ipAddressPrefix  "ipAddressPrefix"
  #define kNAME_ipAddressOrigin  "ipAddressOrigin"
  #define kNAME_ipAddressStatus  "ipAddressStatus"
  #define kNAME_ipAddressCreated  "ipAddressCreated"
  #define kNAME_ipAddressLastChanged  "ipAddressLastChanged"
  #define kNAME_ipAddressRowStatus  "ipAddressRowStatus"
  #define kNAME_ipAddressStorageType  "ipAddressStorageType"
#else
  #define kNAME_ipAddressTable  kOID_ipAddressTable
  #define kNAME_ipAddressEntry  kOID_ipAddressEntry
  #define kNAME_ipAddressAddrType  kOID_ipAddressAddrType
  #define kNAME_ipAddressAddr  kOID_ipAddressAddr
  #define kNAME_ipAddressIfIndex  kOID_ipAddressIfIndex
  #define kNAME_ipAddressType  kOID_ipAddressType
  #define kNAME_ipAddressPrefix  kOID_ipAddressPrefix
  #define kNAME_ipAddressOrigin  kOID_ipAddressOrigin
  #define kNAME_ipAddressStatus  kOID_ipAddressStatus
  #define kNAME_ipAddressCreated  kOID_ipAddressCreated
  #define kNAME_ipAddressLastChanged  kOID_ipAddressLastChanged
  #define kNAME_ipAddressRowStatus  kOID_ipAddressRowStatus
  #define kNAME_ipAddressStorageType  kOID_ipAddressStorageType
#endif

class ipAddressTable : public BcmMibTable
{
  public:
    #if (BCM_IPV6_SUPPORT)
    ipAddressTable (ipv4InterfaceTable *pIpv4InterfaceTable, ipv6InterfaceTable *pIpv6InterfaceTable);
    #else
    ipAddressTable (ipv4InterfaceTable *pIpv4InterfaceTable);
    #endif

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    
  protected:
    
    // Override virtual Update method to allow us to be self-updating.
    void Update ();
    
  protected:
    ipv4InterfaceTable *fpIpv4InterfaceTable;
    #if (BCM_IPV6_SUPPORT)
    ipv6InterfaceTable *fpIpv6InterfaceTable;
    #endif
};

class ipAddressEntryBridge;
class ipAddressEntry : public BcmMibEntry
{
  friend class ipAddressEntryBridge;
  friend class ipAddressTable;

  public:
    ipAddressEntry (ipAddressTable *pTable, ipAddressEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ipAddressEntryBridge *fpBridge;

    enum
    {
      kLEAF_ipAddressAddrType = 1,
      kLEAF_ipAddressAddr = 2,
      kLEAF_ipAddressIfIndex = 3,
      kLEAF_ipAddressType = 4,
      kLEAF_ipAddressPrefix = 5,
      kLEAF_ipAddressOrigin = 6,
      kLEAF_ipAddressStatus = 7,
      kLEAF_ipAddressCreated = 8,
      kLEAF_ipAddressLastChanged = 9,
      kLEAF_ipAddressRowStatus = 10,
      kLEAF_ipAddressStorageType = 11
    };
};


/* ipNetToPhysicalTable and related objects */

// OID constants
#define kOID_ipNetToPhysicalTable  "1.3.6.1.2.1.4.35"
#define kOID_ipNetToPhysicalEntry  "1.3.6.1.2.1.4.35.1"
#define kOID_ipNetToPhysicalIfIndex  "1.3.6.1.2.1.4.35.1.1"
#define kOID_ipNetToPhysicalNetAddressType  "1.3.6.1.2.1.4.35.1.2"
#define kOID_ipNetToPhysicalNetAddress  "1.3.6.1.2.1.4.35.1.3"
#define kOID_ipNetToPhysicalPhysAddress  "1.3.6.1.2.1.4.35.1.4"
#define kOID_ipNetToPhysicalLastUpdated  "1.3.6.1.2.1.4.35.1.5"
#define kOID_ipNetToPhysicalType  "1.3.6.1.2.1.4.35.1.6"
#define kOID_ipNetToPhysicalState  "1.3.6.1.2.1.4.35.1.7"
#define kOID_ipNetToPhysicalRowStatus  "1.3.6.1.2.1.4.35.1.8"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipNetToPhysicalTable  "ipNetToPhysicalTable"
  #define kNAME_ipNetToPhysicalEntry  "ipNetToPhysicalEntry"
  #define kNAME_ipNetToPhysicalIfIndex  "ipNetToPhysicalIfIndex"
  #define kNAME_ipNetToPhysicalNetAddressType  "ipNetToPhysicalNetAddressType"
  #define kNAME_ipNetToPhysicalNetAddress  "ipNetToPhysicalNetAddress"
  #define kNAME_ipNetToPhysicalPhysAddress  "ipNetToPhysicalPhysAddress"
  #define kNAME_ipNetToPhysicalLastUpdated  "ipNetToPhysicalLastUpdated"
  #define kNAME_ipNetToPhysicalType  "ipNetToPhysicalType"
  #define kNAME_ipNetToPhysicalState  "ipNetToPhysicalState"
  #define kNAME_ipNetToPhysicalRowStatus  "ipNetToPhysicalRowStatus"
#else
  #define kNAME_ipNetToPhysicalTable  kOID_ipNetToPhysicalTable
  #define kNAME_ipNetToPhysicalEntry  kOID_ipNetToPhysicalEntry
  #define kNAME_ipNetToPhysicalIfIndex  kOID_ipNetToPhysicalIfIndex
  #define kNAME_ipNetToPhysicalNetAddressType  kOID_ipNetToPhysicalNetAddressType
  #define kNAME_ipNetToPhysicalNetAddress  kOID_ipNetToPhysicalNetAddress
  #define kNAME_ipNetToPhysicalPhysAddress  kOID_ipNetToPhysicalPhysAddress
  #define kNAME_ipNetToPhysicalLastUpdated  kOID_ipNetToPhysicalLastUpdated
  #define kNAME_ipNetToPhysicalType  kOID_ipNetToPhysicalType
  #define kNAME_ipNetToPhysicalState  kOID_ipNetToPhysicalState
  #define kNAME_ipNetToPhysicalRowStatus  kOID_ipNetToPhysicalRowStatus
#endif

class ipNetToPhysicalTable : public BcmMibTable
{
  public:
    ipNetToPhysicalTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ipNetToPhysicalEntryBridge;
class ipNetToPhysicalEntry : public BcmMibEntry
{
  friend class ipNetToPhysicalEntryBridge;
  friend class ipNetToPhysicalTable;

  public:
    ipNetToPhysicalEntry (ipNetToPhysicalTable *pTable, ipNetToPhysicalEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ipNetToPhysicalEntryBridge *fpBridge;

    enum
    {
      kLEAF_ipNetToPhysicalIfIndex = 1,
      kLEAF_ipNetToPhysicalNetAddressType = 2,
      kLEAF_ipNetToPhysicalNetAddress = 3,
      kLEAF_ipNetToPhysicalPhysAddress = 4,
      kLEAF_ipNetToPhysicalLastUpdated = 5,
      kLEAF_ipNetToPhysicalType = 6,
      kLEAF_ipNetToPhysicalState = 7,
      kLEAF_ipNetToPhysicalRowStatus = 8
    };
};

#if 0

/* ipv6ScopeZoneIndexTable and related objects */

// OID constants
#define kOID_ipv6ScopeZoneIndexTable  "1.3.6.1.2.1.4.36"
#define kOID_ipv6ScopeZoneIndexEntry  "1.3.6.1.2.1.4.36.1"
#define kOID_ipv6ScopeZoneIndexIfIndex  "1.3.6.1.2.1.4.36.1.1"
#define kOID_ipv6ScopeZoneIndexLinkLocal  "1.3.6.1.2.1.4.36.1.2"
#define kOID_ipv6ScopeZoneIndex3  "1.3.6.1.2.1.4.36.1.3"
#define kOID_ipv6ScopeZoneIndexAdminLocal  "1.3.6.1.2.1.4.36.1.4"
#define kOID_ipv6ScopeZoneIndexSiteLocal  "1.3.6.1.2.1.4.36.1.5"
#define kOID_ipv6ScopeZoneIndex6  "1.3.6.1.2.1.4.36.1.6"
#define kOID_ipv6ScopeZoneIndex7  "1.3.6.1.2.1.4.36.1.7"
#define kOID_ipv6ScopeZoneIndexOrganizationLocal  "1.3.6.1.2.1.4.36.1.8"
#define kOID_ipv6ScopeZoneIndex9  "1.3.6.1.2.1.4.36.1.9"
#define kOID_ipv6ScopeZoneIndexA  "1.3.6.1.2.1.4.36.1.10"
#define kOID_ipv6ScopeZoneIndexB  "1.3.6.1.2.1.4.36.1.11"
#define kOID_ipv6ScopeZoneIndexC  "1.3.6.1.2.1.4.36.1.12"
#define kOID_ipv6ScopeZoneIndexD  "1.3.6.1.2.1.4.36.1.13"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipv6ScopeZoneIndexTable  "ipv6ScopeZoneIndexTable"
  #define kNAME_ipv6ScopeZoneIndexEntry  "ipv6ScopeZoneIndexEntry"
  #define kNAME_ipv6ScopeZoneIndexIfIndex  "ipv6ScopeZoneIndexIfIndex"
  #define kNAME_ipv6ScopeZoneIndexLinkLocal  "ipv6ScopeZoneIndexLinkLocal"
  #define kNAME_ipv6ScopeZoneIndex3  "ipv6ScopeZoneIndex3"
  #define kNAME_ipv6ScopeZoneIndexAdminLocal  "ipv6ScopeZoneIndexAdminLocal"
  #define kNAME_ipv6ScopeZoneIndexSiteLocal  "ipv6ScopeZoneIndexSiteLocal"
  #define kNAME_ipv6ScopeZoneIndex6  "ipv6ScopeZoneIndex6"
  #define kNAME_ipv6ScopeZoneIndex7  "ipv6ScopeZoneIndex7"
  #define kNAME_ipv6ScopeZoneIndexOrganizationLocal  "ipv6ScopeZoneIndexOrganizationLocal"
  #define kNAME_ipv6ScopeZoneIndex9  "ipv6ScopeZoneIndex9"
  #define kNAME_ipv6ScopeZoneIndexA  "ipv6ScopeZoneIndexA"
  #define kNAME_ipv6ScopeZoneIndexB  "ipv6ScopeZoneIndexB"
  #define kNAME_ipv6ScopeZoneIndexC  "ipv6ScopeZoneIndexC"
  #define kNAME_ipv6ScopeZoneIndexD  "ipv6ScopeZoneIndexD"
#else
  #define kNAME_ipv6ScopeZoneIndexTable  kOID_ipv6ScopeZoneIndexTable
  #define kNAME_ipv6ScopeZoneIndexEntry  kOID_ipv6ScopeZoneIndexEntry
  #define kNAME_ipv6ScopeZoneIndexIfIndex  kOID_ipv6ScopeZoneIndexIfIndex
  #define kNAME_ipv6ScopeZoneIndexLinkLocal  kOID_ipv6ScopeZoneIndexLinkLocal
  #define kNAME_ipv6ScopeZoneIndex3  kOID_ipv6ScopeZoneIndex3
  #define kNAME_ipv6ScopeZoneIndexAdminLocal  kOID_ipv6ScopeZoneIndexAdminLocal
  #define kNAME_ipv6ScopeZoneIndexSiteLocal  kOID_ipv6ScopeZoneIndexSiteLocal
  #define kNAME_ipv6ScopeZoneIndex6  kOID_ipv6ScopeZoneIndex6
  #define kNAME_ipv6ScopeZoneIndex7  kOID_ipv6ScopeZoneIndex7
  #define kNAME_ipv6ScopeZoneIndexOrganizationLocal  kOID_ipv6ScopeZoneIndexOrganizationLocal
  #define kNAME_ipv6ScopeZoneIndex9  kOID_ipv6ScopeZoneIndex9
  #define kNAME_ipv6ScopeZoneIndexA  kOID_ipv6ScopeZoneIndexA
  #define kNAME_ipv6ScopeZoneIndexB  kOID_ipv6ScopeZoneIndexB
  #define kNAME_ipv6ScopeZoneIndexC  kOID_ipv6ScopeZoneIndexC
  #define kNAME_ipv6ScopeZoneIndexD  kOID_ipv6ScopeZoneIndexD
#endif

class ipv6ScopeZoneIndexTable : public BcmMibTable
{
  public:
    ipv6ScopeZoneIndexTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ipv6ScopeZoneIndexEntry : public BcmMibEntry
{
  friend class ipv6ScopeZoneIndexEntryBridge;
  friend class ipv6ScopeZoneIndexTable;

  public:
    ipv6ScopeZoneIndexEntry (ipv6ScopeZoneIndexTable *pTable, ipv6ScopeZoneIndexEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ipv6ScopeZoneIndexEntryBridge *fpBridge;

    enum
    {
      kLEAF_ipv6ScopeZoneIndexIfIndex = 1,
      kLEAF_ipv6ScopeZoneIndexLinkLocal = 2,
      kLEAF_ipv6ScopeZoneIndex3 = 3,
      kLEAF_ipv6ScopeZoneIndexAdminLocal = 4,
      kLEAF_ipv6ScopeZoneIndexSiteLocal = 5,
      kLEAF_ipv6ScopeZoneIndex6 = 6,
      kLEAF_ipv6ScopeZoneIndex7 = 7,
      kLEAF_ipv6ScopeZoneIndexOrganizationLocal = 8,
      kLEAF_ipv6ScopeZoneIndex9 = 9,
      kLEAF_ipv6ScopeZoneIndexA = 10,
      kLEAF_ipv6ScopeZoneIndexB = 11,
      kLEAF_ipv6ScopeZoneIndexC = 12,
      kLEAF_ipv6ScopeZoneIndexD = 13
    };
};

#endif

/* ipDefaultRouterTable and related objects */

// OID constants
#define kOID_ipDefaultRouterTable  "1.3.6.1.2.1.4.37"
#define kOID_ipDefaultRouterEntry  "1.3.6.1.2.1.4.37.1"
#define kOID_ipDefaultRouterAddressType  "1.3.6.1.2.1.4.37.1.1"
#define kOID_ipDefaultRouterAddress  "1.3.6.1.2.1.4.37.1.2"
#define kOID_ipDefaultRouterIfIndex  "1.3.6.1.2.1.4.37.1.3"
#define kOID_ipDefaultRouterLifetime  "1.3.6.1.2.1.4.37.1.4"
#define kOID_ipDefaultRouterPreference  "1.3.6.1.2.1.4.37.1.5"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipDefaultRouterTable  "ipDefaultRouterTable"
  #define kNAME_ipDefaultRouterEntry  "ipDefaultRouterEntry"
  #define kNAME_ipDefaultRouterAddressType  "ipDefaultRouterAddressType"
  #define kNAME_ipDefaultRouterAddress  "ipDefaultRouterAddress"
  #define kNAME_ipDefaultRouterIfIndex  "ipDefaultRouterIfIndex"
  #define kNAME_ipDefaultRouterLifetime  "ipDefaultRouterLifetime"
  #define kNAME_ipDefaultRouterPreference  "ipDefaultRouterPreference"
#else
  #define kNAME_ipDefaultRouterTable  kOID_ipDefaultRouterTable
  #define kNAME_ipDefaultRouterEntry  kOID_ipDefaultRouterEntry
  #define kNAME_ipDefaultRouterAddressType  kOID_ipDefaultRouterAddressType
  #define kNAME_ipDefaultRouterAddress  kOID_ipDefaultRouterAddress
  #define kNAME_ipDefaultRouterIfIndex  kOID_ipDefaultRouterIfIndex
  #define kNAME_ipDefaultRouterLifetime  kOID_ipDefaultRouterLifetime
  #define kNAME_ipDefaultRouterPreference  kOID_ipDefaultRouterPreference
#endif

class ipDefaultRouterTable : public BcmMibTable
{
  public:
    ipDefaultRouterTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ipDefaultRouterEntryBridge;
class ipDefaultRouterEntry : public BcmMibEntry
{
  friend class ipDefaultRouterEntryBridge;
  friend class ipDefaultRouterTable;

  public:
    ipDefaultRouterEntry (ipDefaultRouterTable *pTable, ipDefaultRouterEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ipDefaultRouterEntryBridge *fpBridge;

    enum
    {
      kLEAF_ipDefaultRouterAddressType = 1,
      kLEAF_ipDefaultRouterAddress = 2,
      kLEAF_ipDefaultRouterIfIndex = 3,
      kLEAF_ipDefaultRouterLifetime = 4,
      kLEAF_ipDefaultRouterPreference = 5
    };
};

#if 0

/* ipv6RouterAdvertTable and related objects */

// OID constants
#define kOID_ipv6RouterAdvertTable  "1.3.6.1.2.1.4.39"
#define kOID_ipv6RouterAdvertEntry  "1.3.6.1.2.1.4.39.1"
#define kOID_ipv6RouterAdvertIfIndex  "1.3.6.1.2.1.4.39.1.1"
#define kOID_ipv6RouterAdvertSendAdverts  "1.3.6.1.2.1.4.39.1.2"
#define kOID_ipv6RouterAdvertMaxInterval  "1.3.6.1.2.1.4.39.1.3"
#define kOID_ipv6RouterAdvertMinInterval  "1.3.6.1.2.1.4.39.1.4"
#define kOID_ipv6RouterAdvertManagedFlag  "1.3.6.1.2.1.4.39.1.5"
#define kOID_ipv6RouterAdvertOtherConfigFlag  "1.3.6.1.2.1.4.39.1.6"
#define kOID_ipv6RouterAdvertLinkMTU  "1.3.6.1.2.1.4.39.1.7"
#define kOID_ipv6RouterAdvertReachableTime  "1.3.6.1.2.1.4.39.1.8"
#define kOID_ipv6RouterAdvertRetransmitTime  "1.3.6.1.2.1.4.39.1.9"
#define kOID_ipv6RouterAdvertCurHopLimit  "1.3.6.1.2.1.4.39.1.10"
#define kOID_ipv6RouterAdvertDefaultLifetime  "1.3.6.1.2.1.4.39.1.11"
#define kOID_ipv6RouterAdvertRowStatus  "1.3.6.1.2.1.4.39.1.12"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipv6RouterAdvertTable  "ipv6RouterAdvertTable"
  #define kNAME_ipv6RouterAdvertEntry  "ipv6RouterAdvertEntry"
  #define kNAME_ipv6RouterAdvertIfIndex  "ipv6RouterAdvertIfIndex"
  #define kNAME_ipv6RouterAdvertSendAdverts  "ipv6RouterAdvertSendAdverts"
  #define kNAME_ipv6RouterAdvertMaxInterval  "ipv6RouterAdvertMaxInterval"
  #define kNAME_ipv6RouterAdvertMinInterval  "ipv6RouterAdvertMinInterval"
  #define kNAME_ipv6RouterAdvertManagedFlag  "ipv6RouterAdvertManagedFlag"
  #define kNAME_ipv6RouterAdvertOtherConfigFlag  "ipv6RouterAdvertOtherConfigFlag"
  #define kNAME_ipv6RouterAdvertLinkMTU  "ipv6RouterAdvertLinkMTU"
  #define kNAME_ipv6RouterAdvertReachableTime  "ipv6RouterAdvertReachableTime"
  #define kNAME_ipv6RouterAdvertRetransmitTime  "ipv6RouterAdvertRetransmitTime"
  #define kNAME_ipv6RouterAdvertCurHopLimit  "ipv6RouterAdvertCurHopLimit"
  #define kNAME_ipv6RouterAdvertDefaultLifetime  "ipv6RouterAdvertDefaultLifetime"
  #define kNAME_ipv6RouterAdvertRowStatus  "ipv6RouterAdvertRowStatus"
#else
  #define kNAME_ipv6RouterAdvertTable  kOID_ipv6RouterAdvertTable
  #define kNAME_ipv6RouterAdvertEntry  kOID_ipv6RouterAdvertEntry
  #define kNAME_ipv6RouterAdvertIfIndex  kOID_ipv6RouterAdvertIfIndex
  #define kNAME_ipv6RouterAdvertSendAdverts  kOID_ipv6RouterAdvertSendAdverts
  #define kNAME_ipv6RouterAdvertMaxInterval  kOID_ipv6RouterAdvertMaxInterval
  #define kNAME_ipv6RouterAdvertMinInterval  kOID_ipv6RouterAdvertMinInterval
  #define kNAME_ipv6RouterAdvertManagedFlag  kOID_ipv6RouterAdvertManagedFlag
  #define kNAME_ipv6RouterAdvertOtherConfigFlag  kOID_ipv6RouterAdvertOtherConfigFlag
  #define kNAME_ipv6RouterAdvertLinkMTU  kOID_ipv6RouterAdvertLinkMTU
  #define kNAME_ipv6RouterAdvertReachableTime  kOID_ipv6RouterAdvertReachableTime
  #define kNAME_ipv6RouterAdvertRetransmitTime  kOID_ipv6RouterAdvertRetransmitTime
  #define kNAME_ipv6RouterAdvertCurHopLimit  kOID_ipv6RouterAdvertCurHopLimit
  #define kNAME_ipv6RouterAdvertDefaultLifetime  kOID_ipv6RouterAdvertDefaultLifetime
  #define kNAME_ipv6RouterAdvertRowStatus  kOID_ipv6RouterAdvertRowStatus
#endif

class ipv6RouterAdvertTable : public BcmMibTable
{
  public:
    ipv6RouterAdvertTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    BcmMibEntry *NewDefaultEntry (BcmObjectId &Index);
};

class ipv6RouterAdvertEntry : public BcmMibEntry
{
  friend class ipv6RouterAdvertEntryBridge;
  friend class ipv6RouterAdvertTable;

  public:
    ipv6RouterAdvertEntry (ipv6RouterAdvertTable *pTable, ipv6RouterAdvertEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ipv6RouterAdvertEntryBridge *fpBridge;

    enum
    {
      kLEAF_ipv6RouterAdvertIfIndex = 1,
      kLEAF_ipv6RouterAdvertSendAdverts = 2,
      kLEAF_ipv6RouterAdvertMaxInterval = 3,
      kLEAF_ipv6RouterAdvertMinInterval = 4,
      kLEAF_ipv6RouterAdvertManagedFlag = 5,
      kLEAF_ipv6RouterAdvertOtherConfigFlag = 6,
      kLEAF_ipv6RouterAdvertLinkMTU = 7,
      kLEAF_ipv6RouterAdvertReachableTime = 8,
      kLEAF_ipv6RouterAdvertRetransmitTime = 9,
      kLEAF_ipv6RouterAdvertCurHopLimit = 10,
      kLEAF_ipv6RouterAdvertDefaultLifetime = 11,
      kLEAF_ipv6RouterAdvertRowStatus = 12
    };
};

#endif

/* ipAddrTable and related objects */

// OID constants
#define kOID_ipAddrTable  "1.3.6.1.2.1.4.20"
#define kOID_ipAddrEntry  "1.3.6.1.2.1.4.20.1"
#define kOID_ipAdEntAddr  "1.3.6.1.2.1.4.20.1.1"
#define kOID_ipAdEntIfIndex  "1.3.6.1.2.1.4.20.1.2"
#define kOID_ipAdEntNetMask  "1.3.6.1.2.1.4.20.1.3"
#define kOID_ipAdEntBcastAddr  "1.3.6.1.2.1.4.20.1.4"
#define kOID_ipAdEntReasmMaxSize  "1.3.6.1.2.1.4.20.1.5"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipAddrTable  "ipAddrTable"
  #define kNAME_ipAddrEntry  "ipAddrEntry"
  #define kNAME_ipAdEntAddr  "ipAdEntAddr"
  #define kNAME_ipAdEntIfIndex  "ipAdEntIfIndex"
  #define kNAME_ipAdEntNetMask  "ipAdEntNetMask"
  #define kNAME_ipAdEntBcastAddr  "ipAdEntBcastAddr"
  #define kNAME_ipAdEntReasmMaxSize  "ipAdEntReasmMaxSize"
#else
  #define kNAME_ipAddrTable  kOID_ipAddrTable
  #define kNAME_ipAddrEntry  kOID_ipAddrEntry
  #define kNAME_ipAdEntAddr  kOID_ipAdEntAddr
  #define kNAME_ipAdEntIfIndex  kOID_ipAdEntIfIndex
  #define kNAME_ipAdEntNetMask  kOID_ipAdEntNetMask
  #define kNAME_ipAdEntBcastAddr  kOID_ipAdEntBcastAddr
  #define kNAME_ipAdEntReasmMaxSize  kOID_ipAdEntReasmMaxSize
#endif

class ipAddrTable : public BcmMibTable
{
  public:
    ipAddrTable (ipv4InterfaceTable *pIpv4InterfaceTable);

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    
    // Override virtual Update method to allow us to be self-updating.
    void Update ();
    
  protected:
    ipv4InterfaceTable *fpIpv4InterfaceTable;
};

class ipAddrEntryBridge;
class ipAddrEntry : public BcmMibEntry
{
  friend class ipAddrEntryBridge;
  friend class ipAddrTable;

  public:
    ipAddrEntry (ipAddrTable *pTable, ipAddrEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ipAddrEntryBridge *fpBridge;

    enum
    {
      kLEAF_ipAdEntAddr = 1,
      kLEAF_ipAdEntIfIndex = 2,
      kLEAF_ipAdEntNetMask = 3,
      kLEAF_ipAdEntBcastAddr = 4,
      kLEAF_ipAdEntReasmMaxSize = 5
    };
};


/* ipNetToMediaTable and related objects */

// OID constants
#define kOID_ipNetToMediaTable  "1.3.6.1.2.1.4.22"
#define kOID_ipNetToMediaEntry  "1.3.6.1.2.1.4.22.1"
#define kOID_ipNetToMediaIfIndex  "1.3.6.1.2.1.4.22.1.1"
#define kOID_ipNetToMediaPhysAddress  "1.3.6.1.2.1.4.22.1.2"
#define kOID_ipNetToMediaNetAddress  "1.3.6.1.2.1.4.22.1.3"
#define kOID_ipNetToMediaType  "1.3.6.1.2.1.4.22.1.4"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipNetToMediaTable  "ipNetToMediaTable"
  #define kNAME_ipNetToMediaEntry  "ipNetToMediaEntry"
  #define kNAME_ipNetToMediaIfIndex  "ipNetToMediaIfIndex"
  #define kNAME_ipNetToMediaPhysAddress  "ipNetToMediaPhysAddress"
  #define kNAME_ipNetToMediaNetAddress  "ipNetToMediaNetAddress"
  #define kNAME_ipNetToMediaType  "ipNetToMediaType"
#else
  #define kNAME_ipNetToMediaTable  kOID_ipNetToMediaTable
  #define kNAME_ipNetToMediaEntry  kOID_ipNetToMediaEntry
  #define kNAME_ipNetToMediaIfIndex  kOID_ipNetToMediaIfIndex
  #define kNAME_ipNetToMediaPhysAddress  kOID_ipNetToMediaPhysAddress
  #define kNAME_ipNetToMediaNetAddress  kOID_ipNetToMediaNetAddress
  #define kNAME_ipNetToMediaType  kOID_ipNetToMediaType
#endif

class ipNetToMediaTable : public BcmMibTable
{
  public:
    ipNetToMediaTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ipNetToMediaEntryBridge;
class ipNetToMediaEntry : public BcmMibEntry
{
  friend class ipNetToMediaEntryBridge;
  friend class ipNetToMediaTable;

  public:
    ipNetToMediaEntry (ipNetToMediaTable *pTable, ipNetToMediaEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    
    #if (BCM_IPV6_SUPPORT)
    bool IsAccessible (BcmMibRequest *pReq = NULL) const;
    #endif

  protected:
    ipNetToMediaEntryBridge *fpBridge;

    enum
    {
      kLEAF_ipNetToMediaIfIndex = 1,
      kLEAF_ipNetToMediaPhysAddress = 2,
      kLEAF_ipNetToMediaNetAddress = 3,
      kLEAF_ipNetToMediaType = 4
    };
};

#if 0

/*\
 *  ipTrafficStatsGroup group
\*/

// OID constants
#define kOID_ipTrafficStatsGroup  "1.3.6.1.2.1.4.31"
#define kOID_ipIfStatsTableLastChange  "1.3.6.1.2.1.4.31.2.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipTrafficStatsGroup  "ipTrafficStatsGroup"
  #define kNAME_ipIfStatsTableLastChange  "ipIfStatsTableLastChange"
#else
  #define kNAME_ipTrafficStatsGroup  kOID_ipTrafficStatsGroup
  #define kNAME_ipIfStatsTableLastChange  kOID_ipIfStatsTableLastChange
#endif

class ipTrafficStatsGroup : public BcmMibGroup
{
  friend class ipTrafficStatsGroupBridge;

  public:
    ipTrafficStatsGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ipTrafficStatsGroupBridge *fpBridge;

    enum
    {
      kLEAF_ipIfStatsTableLastChange = 2
    };
};


/* ipSystemStatsTable and related objects */

// OID constants
#define kOID_ipSystemStatsTable  "1.3.6.1.2.1.4.31.1"
#define kOID_ipSystemStatsEntry  "1.3.6.1.2.1.4.31.1.1"
#define kOID_ipSystemStatsIPVersion  "1.3.6.1.2.1.4.31.1.1.1"
#define kOID_ipSystemStatsInReceives  "1.3.6.1.2.1.4.31.1.1.3"
#define kOID_ipSystemStatsHCInReceives  "1.3.6.1.2.1.4.31.1.1.4"
#define kOID_ipSystemStatsInOctets  "1.3.6.1.2.1.4.31.1.1.5"
#define kOID_ipSystemStatsHCInOctets  "1.3.6.1.2.1.4.31.1.1.6"
#define kOID_ipSystemStatsInHdrErrors  "1.3.6.1.2.1.4.31.1.1.7"
#define kOID_ipSystemStatsInNoRoutes  "1.3.6.1.2.1.4.31.1.1.8"
#define kOID_ipSystemStatsInAddrErrors  "1.3.6.1.2.1.4.31.1.1.9"
#define kOID_ipSystemStatsInUnknownProtos  "1.3.6.1.2.1.4.31.1.1.10"
#define kOID_ipSystemStatsInTruncatedPkts  "1.3.6.1.2.1.4.31.1.1.11"
#define kOID_ipSystemStatsInForwDatagrams  "1.3.6.1.2.1.4.31.1.1.12"
#define kOID_ipSystemStatsHCInForwDatagrams  "1.3.6.1.2.1.4.31.1.1.13"
#define kOID_ipSystemStatsReasmReqds  "1.3.6.1.2.1.4.31.1.1.14"
#define kOID_ipSystemStatsReasmOKs  "1.3.6.1.2.1.4.31.1.1.15"
#define kOID_ipSystemStatsReasmFails  "1.3.6.1.2.1.4.31.1.1.16"
#define kOID_ipSystemStatsInDiscards  "1.3.6.1.2.1.4.31.1.1.17"
#define kOID_ipSystemStatsInDelivers  "1.3.6.1.2.1.4.31.1.1.18"
#define kOID_ipSystemStatsHCInDelivers  "1.3.6.1.2.1.4.31.1.1.19"
#define kOID_ipSystemStatsOutRequests  "1.3.6.1.2.1.4.31.1.1.20"
#define kOID_ipSystemStatsHCOutRequests  "1.3.6.1.2.1.4.31.1.1.21"
#define kOID_ipSystemStatsOutNoRoutes  "1.3.6.1.2.1.4.31.1.1.22"
#define kOID_ipSystemStatsOutForwDatagrams  "1.3.6.1.2.1.4.31.1.1.23"
#define kOID_ipSystemStatsHCOutForwDatagrams  "1.3.6.1.2.1.4.31.1.1.24"
#define kOID_ipSystemStatsOutDiscards  "1.3.6.1.2.1.4.31.1.1.25"
#define kOID_ipSystemStatsOutFragReqds  "1.3.6.1.2.1.4.31.1.1.26"
#define kOID_ipSystemStatsOutFragOKs  "1.3.6.1.2.1.4.31.1.1.27"
#define kOID_ipSystemStatsOutFragFails  "1.3.6.1.2.1.4.31.1.1.28"
#define kOID_ipSystemStatsOutFragCreates  "1.3.6.1.2.1.4.31.1.1.29"
#define kOID_ipSystemStatsOutTransmits  "1.3.6.1.2.1.4.31.1.1.30"
#define kOID_ipSystemStatsHCOutTransmits  "1.3.6.1.2.1.4.31.1.1.31"
#define kOID_ipSystemStatsOutOctets  "1.3.6.1.2.1.4.31.1.1.32"
#define kOID_ipSystemStatsHCOutOctets  "1.3.6.1.2.1.4.31.1.1.33"
#define kOID_ipSystemStatsInMcastPkts  "1.3.6.1.2.1.4.31.1.1.34"
#define kOID_ipSystemStatsHCInMcastPkts  "1.3.6.1.2.1.4.31.1.1.35"
#define kOID_ipSystemStatsInMcastOctets  "1.3.6.1.2.1.4.31.1.1.36"
#define kOID_ipSystemStatsHCInMcastOctets  "1.3.6.1.2.1.4.31.1.1.37"
#define kOID_ipSystemStatsOutMcastPkts  "1.3.6.1.2.1.4.31.1.1.38"
#define kOID_ipSystemStatsHCOutMcastPkts  "1.3.6.1.2.1.4.31.1.1.39"
#define kOID_ipSystemStatsOutMcastOctets  "1.3.6.1.2.1.4.31.1.1.40"
#define kOID_ipSystemStatsHCOutMcastOctets  "1.3.6.1.2.1.4.31.1.1.41"
#define kOID_ipSystemStatsInBcastPkts  "1.3.6.1.2.1.4.31.1.1.42"
#define kOID_ipSystemStatsHCInBcastPkts  "1.3.6.1.2.1.4.31.1.1.43"
#define kOID_ipSystemStatsOutBcastPkts  "1.3.6.1.2.1.4.31.1.1.44"
#define kOID_ipSystemStatsHCOutBcastPkts  "1.3.6.1.2.1.4.31.1.1.45"
#define kOID_ipSystemStatsDiscontinuityTime  "1.3.6.1.2.1.4.31.1.1.46"
#define kOID_ipSystemStatsRefreshRate  "1.3.6.1.2.1.4.31.1.1.47"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipSystemStatsTable  "ipSystemStatsTable"
  #define kNAME_ipSystemStatsEntry  "ipSystemStatsEntry"
  #define kNAME_ipSystemStatsIPVersion  "ipSystemStatsIPVersion"
  #define kNAME_ipSystemStatsInReceives  "ipSystemStatsInReceives"
  #define kNAME_ipSystemStatsHCInReceives  "ipSystemStatsHCInReceives"
  #define kNAME_ipSystemStatsInOctets  "ipSystemStatsInOctets"
  #define kNAME_ipSystemStatsHCInOctets  "ipSystemStatsHCInOctets"
  #define kNAME_ipSystemStatsInHdrErrors  "ipSystemStatsInHdrErrors"
  #define kNAME_ipSystemStatsInNoRoutes  "ipSystemStatsInNoRoutes"
  #define kNAME_ipSystemStatsInAddrErrors  "ipSystemStatsInAddrErrors"
  #define kNAME_ipSystemStatsInUnknownProtos  "ipSystemStatsInUnknownProtos"
  #define kNAME_ipSystemStatsInTruncatedPkts  "ipSystemStatsInTruncatedPkts"
  #define kNAME_ipSystemStatsInForwDatagrams  "ipSystemStatsInForwDatagrams"
  #define kNAME_ipSystemStatsHCInForwDatagrams  "ipSystemStatsHCInForwDatagrams"
  #define kNAME_ipSystemStatsReasmReqds  "ipSystemStatsReasmReqds"
  #define kNAME_ipSystemStatsReasmOKs  "ipSystemStatsReasmOKs"
  #define kNAME_ipSystemStatsReasmFails  "ipSystemStatsReasmFails"
  #define kNAME_ipSystemStatsInDiscards  "ipSystemStatsInDiscards"
  #define kNAME_ipSystemStatsInDelivers  "ipSystemStatsInDelivers"
  #define kNAME_ipSystemStatsHCInDelivers  "ipSystemStatsHCInDelivers"
  #define kNAME_ipSystemStatsOutRequests  "ipSystemStatsOutRequests"
  #define kNAME_ipSystemStatsHCOutRequests  "ipSystemStatsHCOutRequests"
  #define kNAME_ipSystemStatsOutNoRoutes  "ipSystemStatsOutNoRoutes"
  #define kNAME_ipSystemStatsOutForwDatagrams  "ipSystemStatsOutForwDatagrams"
  #define kNAME_ipSystemStatsHCOutForwDatagrams  "ipSystemStatsHCOutForwDatagrams"
  #define kNAME_ipSystemStatsOutDiscards  "ipSystemStatsOutDiscards"
  #define kNAME_ipSystemStatsOutFragReqds  "ipSystemStatsOutFragReqds"
  #define kNAME_ipSystemStatsOutFragOKs  "ipSystemStatsOutFragOKs"
  #define kNAME_ipSystemStatsOutFragFails  "ipSystemStatsOutFragFails"
  #define kNAME_ipSystemStatsOutFragCreates  "ipSystemStatsOutFragCreates"
  #define kNAME_ipSystemStatsOutTransmits  "ipSystemStatsOutTransmits"
  #define kNAME_ipSystemStatsHCOutTransmits  "ipSystemStatsHCOutTransmits"
  #define kNAME_ipSystemStatsOutOctets  "ipSystemStatsOutOctets"
  #define kNAME_ipSystemStatsHCOutOctets  "ipSystemStatsHCOutOctets"
  #define kNAME_ipSystemStatsInMcastPkts  "ipSystemStatsInMcastPkts"
  #define kNAME_ipSystemStatsHCInMcastPkts  "ipSystemStatsHCInMcastPkts"
  #define kNAME_ipSystemStatsInMcastOctets  "ipSystemStatsInMcastOctets"
  #define kNAME_ipSystemStatsHCInMcastOctets  "ipSystemStatsHCInMcastOctets"
  #define kNAME_ipSystemStatsOutMcastPkts  "ipSystemStatsOutMcastPkts"
  #define kNAME_ipSystemStatsHCOutMcastPkts  "ipSystemStatsHCOutMcastPkts"
  #define kNAME_ipSystemStatsOutMcastOctets  "ipSystemStatsOutMcastOctets"
  #define kNAME_ipSystemStatsHCOutMcastOctets  "ipSystemStatsHCOutMcastOctets"
  #define kNAME_ipSystemStatsInBcastPkts  "ipSystemStatsInBcastPkts"
  #define kNAME_ipSystemStatsHCInBcastPkts  "ipSystemStatsHCInBcastPkts"
  #define kNAME_ipSystemStatsOutBcastPkts  "ipSystemStatsOutBcastPkts"
  #define kNAME_ipSystemStatsHCOutBcastPkts  "ipSystemStatsHCOutBcastPkts"
  #define kNAME_ipSystemStatsDiscontinuityTime  "ipSystemStatsDiscontinuityTime"
  #define kNAME_ipSystemStatsRefreshRate  "ipSystemStatsRefreshRate"
#else
  #define kNAME_ipSystemStatsTable  kOID_ipSystemStatsTable
  #define kNAME_ipSystemStatsEntry  kOID_ipSystemStatsEntry
  #define kNAME_ipSystemStatsIPVersion  kOID_ipSystemStatsIPVersion
  #define kNAME_ipSystemStatsInReceives  kOID_ipSystemStatsInReceives
  #define kNAME_ipSystemStatsHCInReceives  kOID_ipSystemStatsHCInReceives
  #define kNAME_ipSystemStatsInOctets  kOID_ipSystemStatsInOctets
  #define kNAME_ipSystemStatsHCInOctets  kOID_ipSystemStatsHCInOctets
  #define kNAME_ipSystemStatsInHdrErrors  kOID_ipSystemStatsInHdrErrors
  #define kNAME_ipSystemStatsInNoRoutes  kOID_ipSystemStatsInNoRoutes
  #define kNAME_ipSystemStatsInAddrErrors  kOID_ipSystemStatsInAddrErrors
  #define kNAME_ipSystemStatsInUnknownProtos  kOID_ipSystemStatsInUnknownProtos
  #define kNAME_ipSystemStatsInTruncatedPkts  kOID_ipSystemStatsInTruncatedPkts
  #define kNAME_ipSystemStatsInForwDatagrams  kOID_ipSystemStatsInForwDatagrams
  #define kNAME_ipSystemStatsHCInForwDatagrams  kOID_ipSystemStatsHCInForwDatagrams
  #define kNAME_ipSystemStatsReasmReqds  kOID_ipSystemStatsReasmReqds
  #define kNAME_ipSystemStatsReasmOKs  kOID_ipSystemStatsReasmOKs
  #define kNAME_ipSystemStatsReasmFails  kOID_ipSystemStatsReasmFails
  #define kNAME_ipSystemStatsInDiscards  kOID_ipSystemStatsInDiscards
  #define kNAME_ipSystemStatsInDelivers  kOID_ipSystemStatsInDelivers
  #define kNAME_ipSystemStatsHCInDelivers  kOID_ipSystemStatsHCInDelivers
  #define kNAME_ipSystemStatsOutRequests  kOID_ipSystemStatsOutRequests
  #define kNAME_ipSystemStatsHCOutRequests  kOID_ipSystemStatsHCOutRequests
  #define kNAME_ipSystemStatsOutNoRoutes  kOID_ipSystemStatsOutNoRoutes
  #define kNAME_ipSystemStatsOutForwDatagrams  kOID_ipSystemStatsOutForwDatagrams
  #define kNAME_ipSystemStatsHCOutForwDatagrams  kOID_ipSystemStatsHCOutForwDatagrams
  #define kNAME_ipSystemStatsOutDiscards  kOID_ipSystemStatsOutDiscards
  #define kNAME_ipSystemStatsOutFragReqds  kOID_ipSystemStatsOutFragReqds
  #define kNAME_ipSystemStatsOutFragOKs  kOID_ipSystemStatsOutFragOKs
  #define kNAME_ipSystemStatsOutFragFails  kOID_ipSystemStatsOutFragFails
  #define kNAME_ipSystemStatsOutFragCreates  kOID_ipSystemStatsOutFragCreates
  #define kNAME_ipSystemStatsOutTransmits  kOID_ipSystemStatsOutTransmits
  #define kNAME_ipSystemStatsHCOutTransmits  kOID_ipSystemStatsHCOutTransmits
  #define kNAME_ipSystemStatsOutOctets  kOID_ipSystemStatsOutOctets
  #define kNAME_ipSystemStatsHCOutOctets  kOID_ipSystemStatsHCOutOctets
  #define kNAME_ipSystemStatsInMcastPkts  kOID_ipSystemStatsInMcastPkts
  #define kNAME_ipSystemStatsHCInMcastPkts  kOID_ipSystemStatsHCInMcastPkts
  #define kNAME_ipSystemStatsInMcastOctets  kOID_ipSystemStatsInMcastOctets
  #define kNAME_ipSystemStatsHCInMcastOctets  kOID_ipSystemStatsHCInMcastOctets
  #define kNAME_ipSystemStatsOutMcastPkts  kOID_ipSystemStatsOutMcastPkts
  #define kNAME_ipSystemStatsHCOutMcastPkts  kOID_ipSystemStatsHCOutMcastPkts
  #define kNAME_ipSystemStatsOutMcastOctets  kOID_ipSystemStatsOutMcastOctets
  #define kNAME_ipSystemStatsHCOutMcastOctets  kOID_ipSystemStatsHCOutMcastOctets
  #define kNAME_ipSystemStatsInBcastPkts  kOID_ipSystemStatsInBcastPkts
  #define kNAME_ipSystemStatsHCInBcastPkts  kOID_ipSystemStatsHCInBcastPkts
  #define kNAME_ipSystemStatsOutBcastPkts  kOID_ipSystemStatsOutBcastPkts
  #define kNAME_ipSystemStatsHCOutBcastPkts  kOID_ipSystemStatsHCOutBcastPkts
  #define kNAME_ipSystemStatsDiscontinuityTime  kOID_ipSystemStatsDiscontinuityTime
  #define kNAME_ipSystemStatsRefreshRate  kOID_ipSystemStatsRefreshRate
#endif

class ipSystemStatsTable : public BcmMibTable
{
  public:
    ipSystemStatsTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    
  protected:
    unsigned long fColumnMask2;
};

class ipSystemStatsEntry : public BcmMibEntry
{
  friend class ipSystemStatsEntryBridge;
  friend class ipSystemStatsTable;

  public:
    ipSystemStatsEntry (ipSystemStatsTable *pTable, ipSystemStatsEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ipSystemStatsEntryBridge *fpBridge;

    enum
    {
      kLEAF_ipSystemStatsIPVersion = 1,
      kLEAF_ipSystemStatsInReceives = 3,
      kLEAF_ipSystemStatsHCInReceives = 4,
      kLEAF_ipSystemStatsInOctets = 5,
      kLEAF_ipSystemStatsHCInOctets = 6,
      kLEAF_ipSystemStatsInHdrErrors = 7,
      kLEAF_ipSystemStatsInNoRoutes = 8,
      kLEAF_ipSystemStatsInAddrErrors = 9,
      kLEAF_ipSystemStatsInUnknownProtos = 10,
      kLEAF_ipSystemStatsInTruncatedPkts = 11,
      kLEAF_ipSystemStatsInForwDatagrams = 12,
      kLEAF_ipSystemStatsHCInForwDatagrams = 13,
      kLEAF_ipSystemStatsReasmReqds = 14,
      kLEAF_ipSystemStatsReasmOKs = 15,
      kLEAF_ipSystemStatsReasmFails = 16,
      kLEAF_ipSystemStatsInDiscards = 17,
      kLEAF_ipSystemStatsInDelivers = 18,
      kLEAF_ipSystemStatsHCInDelivers = 19,
      kLEAF_ipSystemStatsOutRequests = 20,
      kLEAF_ipSystemStatsHCOutRequests = 21,
      kLEAF_ipSystemStatsOutNoRoutes = 22,
      kLEAF_ipSystemStatsOutForwDatagrams = 23,
      kLEAF_ipSystemStatsHCOutForwDatagrams = 24,
      kLEAF_ipSystemStatsOutDiscards = 25,
      kLEAF_ipSystemStatsOutFragReqds = 26,
      kLEAF_ipSystemStatsOutFragOKs = 27,
      kLEAF_ipSystemStatsOutFragFails = 28,
      kLEAF_ipSystemStatsOutFragCreates = 29,
      kLEAF_ipSystemStatsOutTransmits = 30,
      kLEAF_ipSystemStatsHCOutTransmits = 31,
      kLEAF_ipSystemStatsOutOctets = 32,
      kLEAF_ipSystemStatsHCOutOctets = 33,
      kLEAF_ipSystemStatsInMcastPkts = 34,
      kLEAF_ipSystemStatsHCInMcastPkts = 35,
      kLEAF_ipSystemStatsInMcastOctets = 36,
      kLEAF_ipSystemStatsHCInMcastOctets = 37,
      kLEAF_ipSystemStatsOutMcastPkts = 38,
      kLEAF_ipSystemStatsHCOutMcastPkts = 39,
      kLEAF_ipSystemStatsOutMcastOctets = 40,
      kLEAF_ipSystemStatsHCOutMcastOctets = 41,
      kLEAF_ipSystemStatsInBcastPkts = 42,
      kLEAF_ipSystemStatsHCInBcastPkts = 43,
      kLEAF_ipSystemStatsOutBcastPkts = 44,
      kLEAF_ipSystemStatsHCOutBcastPkts = 45,
      kLEAF_ipSystemStatsDiscontinuityTime = 46,
      kLEAF_ipSystemStatsRefreshRate = 47
    };
};


/* ipIfStatsTable and related objects */

// OID constants
#define kOID_ipIfStatsTable  "1.3.6.1.2.1.4.31.3"
#define kOID_ipIfStatsEntry  "1.3.6.1.2.1.4.31.3.1"
#define kOID_ipIfStatsIPVersion  "1.3.6.1.2.1.4.31.3.1.1"
#define kOID_ipIfStatsIfIndex  "1.3.6.1.2.1.4.31.3.1.2"
#define kOID_ipIfStatsInReceives  "1.3.6.1.2.1.4.31.3.1.3"
#define kOID_ipIfStatsHCInReceives  "1.3.6.1.2.1.4.31.3.1.4"
#define kOID_ipIfStatsInOctets  "1.3.6.1.2.1.4.31.3.1.5"
#define kOID_ipIfStatsHCInOctets  "1.3.6.1.2.1.4.31.3.1.6"
#define kOID_ipIfStatsInHdrErrors  "1.3.6.1.2.1.4.31.3.1.7"
#define kOID_ipIfStatsInNoRoutes  "1.3.6.1.2.1.4.31.3.1.8"
#define kOID_ipIfStatsInAddrErrors  "1.3.6.1.2.1.4.31.3.1.9"
#define kOID_ipIfStatsInUnknownProtos  "1.3.6.1.2.1.4.31.3.1.10"
#define kOID_ipIfStatsInTruncatedPkts  "1.3.6.1.2.1.4.31.3.1.11"
#define kOID_ipIfStatsInForwDatagrams  "1.3.6.1.2.1.4.31.3.1.12"
#define kOID_ipIfStatsHCInForwDatagrams  "1.3.6.1.2.1.4.31.3.1.13"
#define kOID_ipIfStatsReasmReqds  "1.3.6.1.2.1.4.31.3.1.14"
#define kOID_ipIfStatsReasmOKs  "1.3.6.1.2.1.4.31.3.1.15"
#define kOID_ipIfStatsReasmFails  "1.3.6.1.2.1.4.31.3.1.16"
#define kOID_ipIfStatsInDiscards  "1.3.6.1.2.1.4.31.3.1.17"
#define kOID_ipIfStatsInDelivers  "1.3.6.1.2.1.4.31.3.1.18"
#define kOID_ipIfStatsHCInDelivers  "1.3.6.1.2.1.4.31.3.1.19"
#define kOID_ipIfStatsOutRequests  "1.3.6.1.2.1.4.31.3.1.20"
#define kOID_ipIfStatsHCOutRequests  "1.3.6.1.2.1.4.31.3.1.21"
#define kOID_ipIfStatsOutForwDatagrams  "1.3.6.1.2.1.4.31.3.1.23"
#define kOID_ipIfStatsHCOutForwDatagrams  "1.3.6.1.2.1.4.31.3.1.24"
#define kOID_ipIfStatsOutDiscards  "1.3.6.1.2.1.4.31.3.1.25"
#define kOID_ipIfStatsOutFragReqds  "1.3.6.1.2.1.4.31.3.1.26"
#define kOID_ipIfStatsOutFragOKs  "1.3.6.1.2.1.4.31.3.1.27"
#define kOID_ipIfStatsOutFragFails  "1.3.6.1.2.1.4.31.3.1.28"
#define kOID_ipIfStatsOutFragCreates  "1.3.6.1.2.1.4.31.3.1.29"
#define kOID_ipIfStatsOutTransmits  "1.3.6.1.2.1.4.31.3.1.30"
#define kOID_ipIfStatsHCOutTransmits  "1.3.6.1.2.1.4.31.3.1.31"
#define kOID_ipIfStatsOutOctets  "1.3.6.1.2.1.4.31.3.1.32"
#define kOID_ipIfStatsHCOutOctets  "1.3.6.1.2.1.4.31.3.1.33"
#define kOID_ipIfStatsInMcastPkts  "1.3.6.1.2.1.4.31.3.1.34"
#define kOID_ipIfStatsHCInMcastPkts  "1.3.6.1.2.1.4.31.3.1.35"
#define kOID_ipIfStatsInMcastOctets  "1.3.6.1.2.1.4.31.3.1.36"
#define kOID_ipIfStatsHCInMcastOctets  "1.3.6.1.2.1.4.31.3.1.37"
#define kOID_ipIfStatsOutMcastPkts  "1.3.6.1.2.1.4.31.3.1.38"
#define kOID_ipIfStatsHCOutMcastPkts  "1.3.6.1.2.1.4.31.3.1.39"
#define kOID_ipIfStatsOutMcastOctets  "1.3.6.1.2.1.4.31.3.1.40"
#define kOID_ipIfStatsHCOutMcastOctets  "1.3.6.1.2.1.4.31.3.1.41"
#define kOID_ipIfStatsInBcastPkts  "1.3.6.1.2.1.4.31.3.1.42"
#define kOID_ipIfStatsHCInBcastPkts  "1.3.6.1.2.1.4.31.3.1.43"
#define kOID_ipIfStatsOutBcastPkts  "1.3.6.1.2.1.4.31.3.1.44"
#define kOID_ipIfStatsHCOutBcastPkts  "1.3.6.1.2.1.4.31.3.1.45"
#define kOID_ipIfStatsDiscontinuityTime  "1.3.6.1.2.1.4.31.3.1.46"
#define kOID_ipIfStatsRefreshRate  "1.3.6.1.2.1.4.31.3.1.47"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ipIfStatsTable  "ipIfStatsTable"
  #define kNAME_ipIfStatsEntry  "ipIfStatsEntry"
  #define kNAME_ipIfStatsIPVersion  "ipIfStatsIPVersion"
  #define kNAME_ipIfStatsIfIndex  "ipIfStatsIfIndex"
  #define kNAME_ipIfStatsInReceives  "ipIfStatsInReceives"
  #define kNAME_ipIfStatsHCInReceives  "ipIfStatsHCInReceives"
  #define kNAME_ipIfStatsInOctets  "ipIfStatsInOctets"
  #define kNAME_ipIfStatsHCInOctets  "ipIfStatsHCInOctets"
  #define kNAME_ipIfStatsInHdrErrors  "ipIfStatsInHdrErrors"
  #define kNAME_ipIfStatsInNoRoutes  "ipIfStatsInNoRoutes"
  #define kNAME_ipIfStatsInAddrErrors  "ipIfStatsInAddrErrors"
  #define kNAME_ipIfStatsInUnknownProtos  "ipIfStatsInUnknownProtos"
  #define kNAME_ipIfStatsInTruncatedPkts  "ipIfStatsInTruncatedPkts"
  #define kNAME_ipIfStatsInForwDatagrams  "ipIfStatsInForwDatagrams"
  #define kNAME_ipIfStatsHCInForwDatagrams  "ipIfStatsHCInForwDatagrams"
  #define kNAME_ipIfStatsReasmReqds  "ipIfStatsReasmReqds"
  #define kNAME_ipIfStatsReasmOKs  "ipIfStatsReasmOKs"
  #define kNAME_ipIfStatsReasmFails  "ipIfStatsReasmFails"
  #define kNAME_ipIfStatsInDiscards  "ipIfStatsInDiscards"
  #define kNAME_ipIfStatsInDelivers  "ipIfStatsInDelivers"
  #define kNAME_ipIfStatsHCInDelivers  "ipIfStatsHCInDelivers"
  #define kNAME_ipIfStatsOutRequests  "ipIfStatsOutRequests"
  #define kNAME_ipIfStatsHCOutRequests  "ipIfStatsHCOutRequests"
  #define kNAME_ipIfStatsOutForwDatagrams  "ipIfStatsOutForwDatagrams"
  #define kNAME_ipIfStatsHCOutForwDatagrams  "ipIfStatsHCOutForwDatagrams"
  #define kNAME_ipIfStatsOutDiscards  "ipIfStatsOutDiscards"
  #define kNAME_ipIfStatsOutFragReqds  "ipIfStatsOutFragReqds"
  #define kNAME_ipIfStatsOutFragOKs  "ipIfStatsOutFragOKs"
  #define kNAME_ipIfStatsOutFragFails  "ipIfStatsOutFragFails"
  #define kNAME_ipIfStatsOutFragCreates  "ipIfStatsOutFragCreates"
  #define kNAME_ipIfStatsOutTransmits  "ipIfStatsOutTransmits"
  #define kNAME_ipIfStatsHCOutTransmits  "ipIfStatsHCOutTransmits"
  #define kNAME_ipIfStatsOutOctets  "ipIfStatsOutOctets"
  #define kNAME_ipIfStatsHCOutOctets  "ipIfStatsHCOutOctets"
  #define kNAME_ipIfStatsInMcastPkts  "ipIfStatsInMcastPkts"
  #define kNAME_ipIfStatsHCInMcastPkts  "ipIfStatsHCInMcastPkts"
  #define kNAME_ipIfStatsInMcastOctets  "ipIfStatsInMcastOctets"
  #define kNAME_ipIfStatsHCInMcastOctets  "ipIfStatsHCInMcastOctets"
  #define kNAME_ipIfStatsOutMcastPkts  "ipIfStatsOutMcastPkts"
  #define kNAME_ipIfStatsHCOutMcastPkts  "ipIfStatsHCOutMcastPkts"
  #define kNAME_ipIfStatsOutMcastOctets  "ipIfStatsOutMcastOctets"
  #define kNAME_ipIfStatsHCOutMcastOctets  "ipIfStatsHCOutMcastOctets"
  #define kNAME_ipIfStatsInBcastPkts  "ipIfStatsInBcastPkts"
  #define kNAME_ipIfStatsHCInBcastPkts  "ipIfStatsHCInBcastPkts"
  #define kNAME_ipIfStatsOutBcastPkts  "ipIfStatsOutBcastPkts"
  #define kNAME_ipIfStatsHCOutBcastPkts  "ipIfStatsHCOutBcastPkts"
  #define kNAME_ipIfStatsDiscontinuityTime  "ipIfStatsDiscontinuityTime"
  #define kNAME_ipIfStatsRefreshRate  "ipIfStatsRefreshRate"
#else
  #define kNAME_ipIfStatsTable  kOID_ipIfStatsTable
  #define kNAME_ipIfStatsEntry  kOID_ipIfStatsEntry
  #define kNAME_ipIfStatsIPVersion  kOID_ipIfStatsIPVersion
  #define kNAME_ipIfStatsIfIndex  kOID_ipIfStatsIfIndex
  #define kNAME_ipIfStatsInReceives  kOID_ipIfStatsInReceives
  #define kNAME_ipIfStatsHCInReceives  kOID_ipIfStatsHCInReceives
  #define kNAME_ipIfStatsInOctets  kOID_ipIfStatsInOctets
  #define kNAME_ipIfStatsHCInOctets  kOID_ipIfStatsHCInOctets
  #define kNAME_ipIfStatsInHdrErrors  kOID_ipIfStatsInHdrErrors
  #define kNAME_ipIfStatsInNoRoutes  kOID_ipIfStatsInNoRoutes
  #define kNAME_ipIfStatsInAddrErrors  kOID_ipIfStatsInAddrErrors
  #define kNAME_ipIfStatsInUnknownProtos  kOID_ipIfStatsInUnknownProtos
  #define kNAME_ipIfStatsInTruncatedPkts  kOID_ipIfStatsInTruncatedPkts
  #define kNAME_ipIfStatsInForwDatagrams  kOID_ipIfStatsInForwDatagrams
  #define kNAME_ipIfStatsHCInForwDatagrams  kOID_ipIfStatsHCInForwDatagrams
  #define kNAME_ipIfStatsReasmReqds  kOID_ipIfStatsReasmReqds
  #define kNAME_ipIfStatsReasmOKs  kOID_ipIfStatsReasmOKs
  #define kNAME_ipIfStatsReasmFails  kOID_ipIfStatsReasmFails
  #define kNAME_ipIfStatsInDiscards  kOID_ipIfStatsInDiscards
  #define kNAME_ipIfStatsInDelivers  kOID_ipIfStatsInDelivers
  #define kNAME_ipIfStatsHCInDelivers  kOID_ipIfStatsHCInDelivers
  #define kNAME_ipIfStatsOutRequests  kOID_ipIfStatsOutRequests
  #define kNAME_ipIfStatsHCOutRequests  kOID_ipIfStatsHCOutRequests
  #define kNAME_ipIfStatsOutForwDatagrams  kOID_ipIfStatsOutForwDatagrams
  #define kNAME_ipIfStatsHCOutForwDatagrams  kOID_ipIfStatsHCOutForwDatagrams
  #define kNAME_ipIfStatsOutDiscards  kOID_ipIfStatsOutDiscards
  #define kNAME_ipIfStatsOutFragReqds  kOID_ipIfStatsOutFragReqds
  #define kNAME_ipIfStatsOutFragOKs  kOID_ipIfStatsOutFragOKs
  #define kNAME_ipIfStatsOutFragFails  kOID_ipIfStatsOutFragFails
  #define kNAME_ipIfStatsOutFragCreates  kOID_ipIfStatsOutFragCreates
  #define kNAME_ipIfStatsOutTransmits  kOID_ipIfStatsOutTransmits
  #define kNAME_ipIfStatsHCOutTransmits  kOID_ipIfStatsHCOutTransmits
  #define kNAME_ipIfStatsOutOctets  kOID_ipIfStatsOutOctets
  #define kNAME_ipIfStatsHCOutOctets  kOID_ipIfStatsHCOutOctets
  #define kNAME_ipIfStatsInMcastPkts  kOID_ipIfStatsInMcastPkts
  #define kNAME_ipIfStatsHCInMcastPkts  kOID_ipIfStatsHCInMcastPkts
  #define kNAME_ipIfStatsInMcastOctets  kOID_ipIfStatsInMcastOctets
  #define kNAME_ipIfStatsHCInMcastOctets  kOID_ipIfStatsHCInMcastOctets
  #define kNAME_ipIfStatsOutMcastPkts  kOID_ipIfStatsOutMcastPkts
  #define kNAME_ipIfStatsHCOutMcastPkts  kOID_ipIfStatsHCOutMcastPkts
  #define kNAME_ipIfStatsOutMcastOctets  kOID_ipIfStatsOutMcastOctets
  #define kNAME_ipIfStatsHCOutMcastOctets  kOID_ipIfStatsHCOutMcastOctets
  #define kNAME_ipIfStatsInBcastPkts  kOID_ipIfStatsInBcastPkts
  #define kNAME_ipIfStatsHCInBcastPkts  kOID_ipIfStatsHCInBcastPkts
  #define kNAME_ipIfStatsOutBcastPkts  kOID_ipIfStatsOutBcastPkts
  #define kNAME_ipIfStatsHCOutBcastPkts  kOID_ipIfStatsHCOutBcastPkts
  #define kNAME_ipIfStatsDiscontinuityTime  kOID_ipIfStatsDiscontinuityTime
  #define kNAME_ipIfStatsRefreshRate  kOID_ipIfStatsRefreshRate
#endif

class ipIfStatsTable : public BcmMibTable
{
  public:
    ipIfStatsTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    
  protected:
    unsigned long fColumnMask2;
};

class ipIfStatsEntry : public BcmMibEntry
{
  friend class ipIfStatsEntryBridge;
  friend class ipIfStatsTable;

  public:
    ipIfStatsEntry (ipIfStatsTable *pTable, ipIfStatsEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ipIfStatsEntryBridge *fpBridge;

    enum
    {
      kLEAF_ipIfStatsIPVersion = 1,
      kLEAF_ipIfStatsIfIndex = 2,
      kLEAF_ipIfStatsInReceives = 3,
      kLEAF_ipIfStatsHCInReceives = 4,
      kLEAF_ipIfStatsInOctets = 5,
      kLEAF_ipIfStatsHCInOctets = 6,
      kLEAF_ipIfStatsInHdrErrors = 7,
      kLEAF_ipIfStatsInNoRoutes = 8,
      kLEAF_ipIfStatsInAddrErrors = 9,
      kLEAF_ipIfStatsInUnknownProtos = 10,
      kLEAF_ipIfStatsInTruncatedPkts = 11,
      kLEAF_ipIfStatsInForwDatagrams = 12,
      kLEAF_ipIfStatsHCInForwDatagrams = 13,
      kLEAF_ipIfStatsReasmReqds = 14,
      kLEAF_ipIfStatsReasmOKs = 15,
      kLEAF_ipIfStatsReasmFails = 16,
      kLEAF_ipIfStatsInDiscards = 17,
      kLEAF_ipIfStatsInDelivers = 18,
      kLEAF_ipIfStatsHCInDelivers = 19,
      kLEAF_ipIfStatsOutRequests = 20,
      kLEAF_ipIfStatsHCOutRequests = 21,
      kLEAF_ipIfStatsOutForwDatagrams = 23,
      kLEAF_ipIfStatsHCOutForwDatagrams = 24,
      kLEAF_ipIfStatsOutDiscards = 25,
      kLEAF_ipIfStatsOutFragReqds = 26,
      kLEAF_ipIfStatsOutFragOKs = 27,
      kLEAF_ipIfStatsOutFragFails = 28,
      kLEAF_ipIfStatsOutFragCreates = 29,
      kLEAF_ipIfStatsOutTransmits = 30,
      kLEAF_ipIfStatsHCOutTransmits = 31,
      kLEAF_ipIfStatsOutOctets = 32,
      kLEAF_ipIfStatsHCOutOctets = 33,
      kLEAF_ipIfStatsInMcastPkts = 34,
      kLEAF_ipIfStatsHCInMcastPkts = 35,
      kLEAF_ipIfStatsInMcastOctets = 36,
      kLEAF_ipIfStatsHCInMcastOctets = 37,
      kLEAF_ipIfStatsOutMcastPkts = 38,
      kLEAF_ipIfStatsHCOutMcastPkts = 39,
      kLEAF_ipIfStatsOutMcastOctets = 40,
      kLEAF_ipIfStatsHCOutMcastOctets = 41,
      kLEAF_ipIfStatsInBcastPkts = 42,
      kLEAF_ipIfStatsHCInBcastPkts = 43,
      kLEAF_ipIfStatsOutBcastPkts = 44,
      kLEAF_ipIfStatsHCOutBcastPkts = 45,
      kLEAF_ipIfStatsDiscontinuityTime = 46,
      kLEAF_ipIfStatsRefreshRate = 47
    };
};

#endif

/*\
 *  icmpGroup group
\*/

// OID constants
#define kOID_icmpGroup  "1.3.6.1.2.1.5"
#define kOID_icmpInMsgs  "1.3.6.1.2.1.5.1.0"
#define kOID_icmpInErrors  "1.3.6.1.2.1.5.2.0"
#define kOID_icmpInDestUnreachs  "1.3.6.1.2.1.5.3.0"
#define kOID_icmpInTimeExcds  "1.3.6.1.2.1.5.4.0"
#define kOID_icmpInParmProbs  "1.3.6.1.2.1.5.5.0"
#define kOID_icmpInSrcQuenchs  "1.3.6.1.2.1.5.6.0"
#define kOID_icmpInRedirects  "1.3.6.1.2.1.5.7.0"
#define kOID_icmpInEchos  "1.3.6.1.2.1.5.8.0"
#define kOID_icmpInEchoReps  "1.3.6.1.2.1.5.9.0"
#define kOID_icmpInTimestamps  "1.3.6.1.2.1.5.10.0"
#define kOID_icmpInTimestampReps  "1.3.6.1.2.1.5.11.0"
#define kOID_icmpInAddrMasks  "1.3.6.1.2.1.5.12.0"
#define kOID_icmpInAddrMaskReps  "1.3.6.1.2.1.5.13.0"
#define kOID_icmpOutMsgs  "1.3.6.1.2.1.5.14.0"
#define kOID_icmpOutErrors  "1.3.6.1.2.1.5.15.0"
#define kOID_icmpOutDestUnreachs  "1.3.6.1.2.1.5.16.0"
#define kOID_icmpOutTimeExcds  "1.3.6.1.2.1.5.17.0"
#define kOID_icmpOutParmProbs  "1.3.6.1.2.1.5.18.0"
#define kOID_icmpOutSrcQuenchs  "1.3.6.1.2.1.5.19.0"
#define kOID_icmpOutRedirects  "1.3.6.1.2.1.5.20.0"
#define kOID_icmpOutEchos  "1.3.6.1.2.1.5.21.0"
#define kOID_icmpOutEchoReps  "1.3.6.1.2.1.5.22.0"
#define kOID_icmpOutTimestamps  "1.3.6.1.2.1.5.23.0"
#define kOID_icmpOutTimestampReps  "1.3.6.1.2.1.5.24.0"
#define kOID_icmpOutAddrMasks  "1.3.6.1.2.1.5.25.0"
#define kOID_icmpOutAddrMaskReps  "1.3.6.1.2.1.5.26.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_icmpGroup  "icmpGroup"
  #define kNAME_icmpInMsgs  "icmpInMsgs"
  #define kNAME_icmpInErrors  "icmpInErrors"
  #define kNAME_icmpInDestUnreachs  "icmpInDestUnreachs"
  #define kNAME_icmpInTimeExcds  "icmpInTimeExcds"
  #define kNAME_icmpInParmProbs  "icmpInParmProbs"
  #define kNAME_icmpInSrcQuenchs  "icmpInSrcQuenchs"
  #define kNAME_icmpInRedirects  "icmpInRedirects"
  #define kNAME_icmpInEchos  "icmpInEchos"
  #define kNAME_icmpInEchoReps  "icmpInEchoReps"
  #define kNAME_icmpInTimestamps  "icmpInTimestamps"
  #define kNAME_icmpInTimestampReps  "icmpInTimestampReps"
  #define kNAME_icmpInAddrMasks  "icmpInAddrMasks"
  #define kNAME_icmpInAddrMaskReps  "icmpInAddrMaskReps"
  #define kNAME_icmpOutMsgs  "icmpOutMsgs"
  #define kNAME_icmpOutErrors  "icmpOutErrors"
  #define kNAME_icmpOutDestUnreachs  "icmpOutDestUnreachs"
  #define kNAME_icmpOutTimeExcds  "icmpOutTimeExcds"
  #define kNAME_icmpOutParmProbs  "icmpOutParmProbs"
  #define kNAME_icmpOutSrcQuenchs  "icmpOutSrcQuenchs"
  #define kNAME_icmpOutRedirects  "icmpOutRedirects"
  #define kNAME_icmpOutEchos  "icmpOutEchos"
  #define kNAME_icmpOutEchoReps  "icmpOutEchoReps"
  #define kNAME_icmpOutTimestamps  "icmpOutTimestamps"
  #define kNAME_icmpOutTimestampReps  "icmpOutTimestampReps"
  #define kNAME_icmpOutAddrMasks  "icmpOutAddrMasks"
  #define kNAME_icmpOutAddrMaskReps  "icmpOutAddrMaskReps"
#else
  #define kNAME_icmpGroup  kOID_icmpGroup
  #define kNAME_icmpInMsgs  kOID_icmpInMsgs
  #define kNAME_icmpInErrors  kOID_icmpInErrors
  #define kNAME_icmpInDestUnreachs  kOID_icmpInDestUnreachs
  #define kNAME_icmpInTimeExcds  kOID_icmpInTimeExcds
  #define kNAME_icmpInParmProbs  kOID_icmpInParmProbs
  #define kNAME_icmpInSrcQuenchs  kOID_icmpInSrcQuenchs
  #define kNAME_icmpInRedirects  kOID_icmpInRedirects
  #define kNAME_icmpInEchos  kOID_icmpInEchos
  #define kNAME_icmpInEchoReps  kOID_icmpInEchoReps
  #define kNAME_icmpInTimestamps  kOID_icmpInTimestamps
  #define kNAME_icmpInTimestampReps  kOID_icmpInTimestampReps
  #define kNAME_icmpInAddrMasks  kOID_icmpInAddrMasks
  #define kNAME_icmpInAddrMaskReps  kOID_icmpInAddrMaskReps
  #define kNAME_icmpOutMsgs  kOID_icmpOutMsgs
  #define kNAME_icmpOutErrors  kOID_icmpOutErrors
  #define kNAME_icmpOutDestUnreachs  kOID_icmpOutDestUnreachs
  #define kNAME_icmpOutTimeExcds  kOID_icmpOutTimeExcds
  #define kNAME_icmpOutParmProbs  kOID_icmpOutParmProbs
  #define kNAME_icmpOutSrcQuenchs  kOID_icmpOutSrcQuenchs
  #define kNAME_icmpOutRedirects  kOID_icmpOutRedirects
  #define kNAME_icmpOutEchos  kOID_icmpOutEchos
  #define kNAME_icmpOutEchoReps  kOID_icmpOutEchoReps
  #define kNAME_icmpOutTimestamps  kOID_icmpOutTimestamps
  #define kNAME_icmpOutTimestampReps  kOID_icmpOutTimestampReps
  #define kNAME_icmpOutAddrMasks  kOID_icmpOutAddrMasks
  #define kNAME_icmpOutAddrMaskReps  kOID_icmpOutAddrMaskReps
#endif

class icmpGroupBridge;
class icmpGroup : public BcmMibGroup
{
  friend class icmpGroupBridge;
  friend class icmpMsgStatsTable; 

  public:
    icmpGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    icmpGroupBridge *fpBridge;

    enum
    {
      kLEAF_icmpInMsgs = 1,
      kLEAF_icmpInErrors = 2,
      kLEAF_icmpInDestUnreachs = 3,
      kLEAF_icmpInTimeExcds = 4,
      kLEAF_icmpInParmProbs = 5,
      kLEAF_icmpInSrcQuenchs = 6,
      kLEAF_icmpInRedirects = 7,
      kLEAF_icmpInEchos = 8,
      kLEAF_icmpInEchoReps = 9,
      kLEAF_icmpInTimestamps = 10,
      kLEAF_icmpInTimestampReps = 11,
      kLEAF_icmpInAddrMasks = 12,
      kLEAF_icmpInAddrMaskReps = 13,
      kLEAF_icmpOutMsgs = 14,
      kLEAF_icmpOutErrors = 15,
      kLEAF_icmpOutDestUnreachs = 16,
      kLEAF_icmpOutTimeExcds = 17,
      kLEAF_icmpOutParmProbs = 18,
      kLEAF_icmpOutSrcQuenchs = 19,
      kLEAF_icmpOutRedirects = 20,
      kLEAF_icmpOutEchos = 21,
      kLEAF_icmpOutEchoReps = 22,
      kLEAF_icmpOutTimestamps = 23,
      kLEAF_icmpOutTimestampReps = 24,
      kLEAF_icmpOutAddrMasks = 25,
      kLEAF_icmpOutAddrMaskReps = 26
    };
};


/* icmpStatsTable and related objects */

// OID constants
#define kOID_icmpStatsTable  "1.3.6.1.2.1.5.29"
#define kOID_icmpStatsEntry  "1.3.6.1.2.1.5.29.1"
#define kOID_icmpStatsIPVersion  "1.3.6.1.2.1.5.29.1.1"
#define kOID_icmpStatsInMsgs  "1.3.6.1.2.1.5.29.1.2"
#define kOID_icmpStatsInErrors  "1.3.6.1.2.1.5.29.1.3"
#define kOID_icmpStatsOutMsgs  "1.3.6.1.2.1.5.29.1.4"
#define kOID_icmpStatsOutErrors  "1.3.6.1.2.1.5.29.1.5"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_icmpStatsTable  "icmpStatsTable"
  #define kNAME_icmpStatsEntry  "icmpStatsEntry"
  #define kNAME_icmpStatsIPVersion  "icmpStatsIPVersion"
  #define kNAME_icmpStatsInMsgs  "icmpStatsInMsgs"
  #define kNAME_icmpStatsInErrors  "icmpStatsInErrors"
  #define kNAME_icmpStatsOutMsgs  "icmpStatsOutMsgs"
  #define kNAME_icmpStatsOutErrors  "icmpStatsOutErrors"
#else
  #define kNAME_icmpStatsTable  kOID_icmpStatsTable
  #define kNAME_icmpStatsEntry  kOID_icmpStatsEntry
  #define kNAME_icmpStatsIPVersion  kOID_icmpStatsIPVersion
  #define kNAME_icmpStatsInMsgs  kOID_icmpStatsInMsgs
  #define kNAME_icmpStatsInErrors  kOID_icmpStatsInErrors
  #define kNAME_icmpStatsOutMsgs  kOID_icmpStatsOutMsgs
  #define kNAME_icmpStatsOutErrors  kOID_icmpStatsOutErrors
#endif

class icmpStatsTable : public BcmMibTable
{
  public:
    icmpStatsTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class icmpStatsEntryBridge;
class icmpStatsEntry : public BcmMibEntry
{
  friend class icmpStatsEntryBridge;
  friend class icmpStatsTable;

  public:
    icmpStatsEntry (icmpStatsTable *pTable, icmpStatsEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    icmpStatsEntryBridge *fpBridge;

    enum
    {
      kLEAF_icmpStatsIPVersion = 1,
      kLEAF_icmpStatsInMsgs = 2,
      kLEAF_icmpStatsInErrors = 3,
      kLEAF_icmpStatsOutMsgs = 4,
      kLEAF_icmpStatsOutErrors = 5
    };
};


/* icmpMsgStatsTable and related objects */

// OID constants
#define kOID_icmpMsgStatsTable  "1.3.6.1.2.1.5.30"
#define kOID_icmpMsgStatsEntry  "1.3.6.1.2.1.5.30.1"
#define kOID_icmpMsgStatsIPVersion  "1.3.6.1.2.1.5.30.1.1"
#define kOID_icmpMsgStatsType  "1.3.6.1.2.1.5.30.1.2"
#define kOID_icmpMsgStatsInPkts  "1.3.6.1.2.1.5.30.1.3"
#define kOID_icmpMsgStatsOutPkts  "1.3.6.1.2.1.5.30.1.4"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_icmpMsgStatsTable  "icmpMsgStatsTable"
  #define kNAME_icmpMsgStatsEntry  "icmpMsgStatsEntry"
  #define kNAME_icmpMsgStatsIPVersion  "icmpMsgStatsIPVersion"
  #define kNAME_icmpMsgStatsType  "icmpMsgStatsType"
  #define kNAME_icmpMsgStatsInPkts  "icmpMsgStatsInPkts"
  #define kNAME_icmpMsgStatsOutPkts  "icmpMsgStatsOutPkts"
#else
  #define kNAME_icmpMsgStatsTable  kOID_icmpMsgStatsTable
  #define kNAME_icmpMsgStatsEntry  kOID_icmpMsgStatsEntry
  #define kNAME_icmpMsgStatsIPVersion  kOID_icmpMsgStatsIPVersion
  #define kNAME_icmpMsgStatsType  kOID_icmpMsgStatsType
  #define kNAME_icmpMsgStatsInPkts  kOID_icmpMsgStatsInPkts
  #define kNAME_icmpMsgStatsOutPkts  kOID_icmpMsgStatsOutPkts
#endif

class icmpMsgStatsTable : public BcmMibTable
{
  public:
    icmpMsgStatsTable (icmpGroup * icmpgroup);
   ~icmpMsgStatsTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    // Override virtual Update method to allow us to be self-updating.
    void Update ();
    icmpGroup *fpIcmpGroup;

  protected:
    icmpGroup *fpGroup;
    int *fpMsgTypesV4;
    int *fpMsgTypesV6;
};

class icmpMsgStatsEntryBridge;
class icmpMsgStatsEntry : public BcmMibEntry
{
  friend class icmpMsgStatsEntryBridge;
  friend class icmpMsgStatsTable;

  public:
    icmpMsgStatsEntry (icmpMsgStatsTable *pTable, icmpMsgStatsEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    icmpMsgStatsEntryBridge *fpBridge;

    enum
    {
      kLEAF_icmpMsgStatsIPVersion = 1,
      kLEAF_icmpMsgStatsType = 2,
      kLEAF_icmpMsgStatsInPkts = 3,
      kLEAF_icmpMsgStatsOutPkts = 4
    };
};



#endif
