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
//    Filename: ipBridge.h
//    Creation Date: Thursday, April 19, 2007 at 16:18:15
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.0b
//    from input file ip-mib_rfc4293.mib
//
//**************************************************************************

#ifndef IPBRIDGE_H
#define IPBRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

#include "ipMib.h"


/*\
 *  ipGroup 
\*/

#define CONSTVAL_IPFORWARDING_FORWARDING  1
#define CONSTVAL_IPFORWARDING_NOTFORWARDING  2
#define MINVAL_IPDEFAULTTTL  1
#define MAXVAL_IPDEFAULTTTL  255
#define CONSTVAL_IPV6IPFORWARDING_FORWARDING  1
#define CONSTVAL_IPV6IPFORWARDING_NOTFORWARDING  2
#define MINVAL_IPV6IPDEFAULTHOPLIMIT  0
#define MAXVAL_IPV6IPDEFAULTHOPLIMIT  255

class ipGroupBridge : public ScalarMibBridge
{
  friend class ipGroup;

  public:
    ipGroupBridge (BcmSnmpAgent *pAgent);
   ~ipGroupBridge ();

    /* GET methods */

    virtual int Get_ipForwarding ();
    virtual int Get_ipDefaultTTL ();
    virtual int Get_ipReasmTimeout ();
    #if (BCM_IPV6_SUPPORT)
    virtual int Get_ipv6IpForwarding ();
    virtual int Get_ipv6IpDefaultHopLimit ();
    #endif
    unsigned long Get_ipv4InterfaceTableLastChange ();
    #if (BCM_IPV6_SUPPORT)
    unsigned long Get_ipv6InterfaceTableLastChange ();
    #endif
    /*
    // NOTE: normally, TestAndIncr objects don't need bridge
    //       object methods for get, test, or set; the MIB
    //       object will just return SNMP_NO_OPINION and let the
    //       TestAndIncrMibObject do its thing.  However, this
    //       method is provided here in case you need to provide
    //       non-standard custom behaviour.
    int Get_ipAddressSpinLock ();
    */
    /*
    // NOTE: normally, TestAndIncr objects don't need bridge
    //       object methods for get, test, or set; the MIB
    //       object will just return SNMP_NO_OPINION and let the
    //       TestAndIncrMibObject do its thing.  However, this
    //       method is provided here in case you need to provide
    //       non-standard custom behaviour.
    int Get_ipv6RouterAdvertSpinLock ();
    */
    virtual unsigned int Get_ipInReceives ();
    virtual unsigned int Get_ipInHdrErrors ();
    virtual unsigned int Get_ipInAddrErrors ();
    virtual unsigned int Get_ipForwDatagrams ();
    virtual unsigned int Get_ipInUnknownProtos ();
    virtual unsigned int Get_ipInDiscards ();
    virtual unsigned int Get_ipInDelivers ();
    virtual unsigned int Get_ipOutRequests ();
    virtual unsigned int Get_ipOutDiscards ();
    virtual unsigned int Get_ipOutNoRoutes ();
    virtual unsigned int Get_ipReasmReqds ();
    virtual unsigned int Get_ipReasmOKs ();
    virtual unsigned int Get_ipReasmFails ();
    virtual unsigned int Get_ipFragOKs ();
    virtual unsigned int Get_ipFragFails ();
    virtual unsigned int Get_ipFragCreates ();
    virtual unsigned int Get_ipRoutingDiscards ();

    /* TEST methods - not required */


    /* SET methods */

    virtual SNMP_STATUS Set_ipForwarding (int ipForwarding);
    virtual SNMP_STATUS Set_ipDefaultTTL (int ipDefaultTTL);
    #if (BCM_IPV6_SUPPORT)
    SNMP_STATUS Set_ipv6IpForwarding (int ipv6IpForwarding);
    SNMP_STATUS Set_ipv6IpDefaultHopLimit (int ipv6IpDefaultHopLimit);
    #endif
    /*
    // NOTE: normally, TestAndIncr objects don't need bridge
    //       object methods for get, test, or set; the MIB
    //       object will just return SNMP_NO_OPINION and let the
    //       TestAndIncrMibObject do its thing.  However, this
    //       method is provided here in case you need to provide
    //       non-standard custom behaviour.
    SNMP_STATUS Set_ipAddressSpinLock (int ipAddressSpinLock);
    */
    /*
    // NOTE: normally, TestAndIncr objects don't need bridge
    //       object methods for get, test, or set; the MIB
    //       object will just return SNMP_NO_OPINION and let the
    //       TestAndIncrMibObject do its thing.  However, this
    //       method is provided here in case you need to provide
    //       non-standard custom behaviour.
    SNMP_STATUS Set_ipv6RouterAdvertSpinLock (int ipv6RouterAdvertSpinLock);
    */
};


class BcmBaseIpHalIf;

/*\
 *  ipv4InterfaceEntry 
\*/

#define MINVAL_IPV4INTERFACEIFINDEX  1
#define MAXVAL_IPV4INTERFACEIFINDEX  2147483647
#define MINVAL_IPV4INTERFACEREASMMAXSIZE  0
#define MAXVAL_IPV4INTERFACEREASMMAXSIZE  65535
#define CONSTVAL_IPV4INTERFACEENABLESTATUS_UP  1
#define CONSTVAL_IPV4INTERFACEENABLESTATUS_DOWN  2

class ipv4InterfaceEntryBridge : public TabularMibBridge
{
  friend class ipv4InterfaceEntry;

  public:
    ipv4InterfaceEntryBridge (int ipv4InterfaceIfIndex, BcmSnmpAgent *pAgent);
   ~ipv4InterfaceEntryBridge ();

    /* GET methods */

    int Get_ipv4InterfaceIfIndex ();
    int Get_ipv4InterfaceReasmMaxSize ();
    int Get_ipv4InterfaceEnableStatus ();
    unsigned int Get_ipv4InterfaceRetransmitTime ();

    /* TEST methods - not required */


    /* SET methods */

    SNMP_STATUS Set_ipv4InterfaceEnableStatus (int ipv4InterfaceEnableStatus);
};

#if (BCM_IPV6_SUPPORT)

/*\
 *  ipv6InterfaceEntry 
\*/

#define MINVAL_IPV6INTERFACEIFINDEX  1
#define MAXVAL_IPV6INTERFACEIFINDEX  2147483647
#define MINVAL_IPV6INTERFACEREASMMAXSIZE  1500
#define MAXVAL_IPV6INTERFACEREASMMAXSIZE  65535
#define MINLEN_IPV6INTERFACEIDENTIFIER  0
#define MAXLEN_IPV6INTERFACEIDENTIFIER  8
#define CONSTVAL_IPV6INTERFACEENABLESTATUS_UP  1
#define CONSTVAL_IPV6INTERFACEENABLESTATUS_DOWN  2
#define CONSTVAL_IPV6INTERFACEFORWARDING_FORWARDING  1
#define CONSTVAL_IPV6INTERFACEFORWARDING_NOTFORWARDING  2

class BcmNeighborDiscoveryThread;

class ipv6InterfaceEntryBridge : public TabularMibBridge
{
  friend class ipv6InterfaceEntry;

  public:
    ipv6InterfaceEntryBridge (int ipv6InterfaceIfIndex, BcmSnmpAgent *pAgent, BcmNeighborDiscoveryThread *pNDThd, const BcmString &ipv6InterfaceIdentifier);
   ~ipv6InterfaceEntryBridge ();

    /* GET methods */

    int Get_ipv6InterfaceIfIndex ();
    unsigned int Get_ipv6InterfaceReasmMaxSize ();
    int Get_ipv6InterfaceIdentifier (BcmString &ipv6InterfaceIdentifier);
    int Get_ipv6InterfaceEnableStatus ();
    unsigned int Get_ipv6InterfaceReachableTime ();
    unsigned int Get_ipv6InterfaceRetransmitTime ();
    int Get_ipv6InterfaceForwarding ();

    /* TEST methods - not required */


    /* SET methods */

    SNMP_STATUS Set_ipv6InterfaceEnableStatus (int ipv6InterfaceEnableStatus);
    SNMP_STATUS Set_ipv6InterfaceForwarding (int ipv6InterfaceForwarding);
    
  protected:
    BcmNeighborDiscoveryThread *fpNDThd;
    BcmString fInterfaceIdentifier;
};

#endif

#if 0

/*\
 *  ipAddressPrefixEntry 
\*/

#define MINVAL_IPADDRESSPREFIXIFINDEX  1
#define MAXVAL_IPADDRESSPREFIXIFINDEX  2147483647
#define CONSTVAL_IPADDRESSPREFIXTYPE_UNKNOWN  0
#define CONSTVAL_IPADDRESSPREFIXTYPE_IPV4  1
#define CONSTVAL_IPADDRESSPREFIXTYPE_IPV6  2
#define CONSTVAL_IPADDRESSPREFIXTYPE_IPV4Z  3
#define CONSTVAL_IPADDRESSPREFIXTYPE_IPV6Z  4
#define CONSTVAL_IPADDRESSPREFIXTYPE_DNS  16
#define CONSTLEN_IPADDRESSPREFIXPREFIX_UNKNOWN  0
#define CONSTLEN_IPADDRESSPREFIXPREFIX_IPV4  4
#define CONSTLEN_IPADDRESSPREFIXPREFIX_IPV6  16
#define MINVAL_IPADDRESSPREFIXLENGTH  0
#define MAXVAL_IPADDRESSPREFIXLENGTH  32
#define CONSTVAL_IPADDRESSPREFIXORIGIN_OTHER  1
#define CONSTVAL_IPADDRESSPREFIXORIGIN_MANUAL  2
#define CONSTVAL_IPADDRESSPREFIXORIGIN_WELLKNOWN  3
#define CONSTVAL_IPADDRESSPREFIXORIGIN_DHCP  4
#define CONSTVAL_IPADDRESSPREFIXORIGIN_ROUTERADV  5
#define CONSTVAL_IPADDRESSPREFIXONLINKFLAG_TRUE  1
#define CONSTVAL_IPADDRESSPREFIXONLINKFLAG_FALSE  2
#define CONSTVAL_IPADDRESSPREFIXAUTONOMOUSFLAG_TRUE  1
#define CONSTVAL_IPADDRESSPREFIXAUTONOMOUSFLAG_FALSE  2

class ipAddressPrefixEntryBridge : public TabularMibBridge
{
  friend class ipAddressPrefixEntry;

  public:
    ipAddressPrefixEntryBridge (int ipAddressPrefixIfIndex, int ipAddressPrefixType, const BcmString &ipAddressPrefixPrefix, int ipAddressPrefixLength, BcmSnmpAgent *pAgent);
   ~ipAddressPrefixEntryBridge ();

    /* GET methods */

    int Get_ipAddressPrefixIfIndex ();
    int Get_ipAddressPrefixType ();
    int Get_ipAddressPrefixPrefix (BcmString &ipAddressPrefixPrefix);
    int Get_ipAddressPrefixLength ();
    int Get_ipAddressPrefixOrigin ();
    bool Get_ipAddressPrefixOnLinkFlag ();
    bool Get_ipAddressPrefixAutonomousFlag ();
    unsigned int Get_ipAddressPrefixAdvPreferredLifetime ();
    unsigned int Get_ipAddressPrefixAdvValidLifetime ();

    /* TEST methods - not required */


    /* SET methods */

};

#endif

/*\
 *  ipAddressEntry 
\*/

#define CONSTVAL_IPADDRESSADDRTYPE_UNKNOWN  0
#define CONSTVAL_IPADDRESSADDRTYPE_IPV4  1
#define CONSTVAL_IPADDRESSADDRTYPE_IPV6  2
#define CONSTVAL_IPADDRESSADDRTYPE_IPV4Z  3
#define CONSTVAL_IPADDRESSADDRTYPE_IPV6Z  4
#define CONSTVAL_IPADDRESSADDRTYPE_DNS  16
#define CONSTLEN_IPADDRESSADDR_UNKNOWN  0
#define CONSTLEN_IPADDRESSADDR_IPV4  4
#define CONSTLEN_IPADDRESSADDR_IPV6  16
#define MINVAL_IPADDRESSIFINDEX  1
#define MAXVAL_IPADDRESSIFINDEX  2147483647
#define CONSTVAL_IPADDRESSTYPE_UNICAST  1
#define CONSTVAL_IPADDRESSTYPE_ANYCAST  2
#define CONSTVAL_IPADDRESSTYPE_BROADCAST  3
#define CONSTVAL_IPADDRESSORIGIN_OTHER  1
#define CONSTVAL_IPADDRESSORIGIN_MANUAL  2
#define CONSTVAL_IPADDRESSORIGIN_DHCP  4
#define CONSTVAL_IPADDRESSORIGIN_LINKLAYER  5
#define CONSTVAL_IPADDRESSORIGIN_RANDOM  6
#define CONSTVAL_IPADDRESSSTATUS_PREFERRED  1
#define CONSTVAL_IPADDRESSSTATUS_DEPRECATED  2
#define CONSTVAL_IPADDRESSSTATUS_INVALID  3
#define CONSTVAL_IPADDRESSSTATUS_INACCESSIBLE  4
#define CONSTVAL_IPADDRESSSTATUS_UNKNOWN  5
#define CONSTVAL_IPADDRESSSTATUS_TENTATIVE  6
#define CONSTVAL_IPADDRESSSTATUS_DUPLICATE  7
#define CONSTVAL_IPADDRESSSTATUS_OPTIMISTIC  8
#define CONSTVAL_IPADDRESSROWSTATUS_ACTIVE  1
#define CONSTVAL_IPADDRESSROWSTATUS_NOTINSERVICE  2
#define CONSTVAL_IPADDRESSROWSTATUS_NOTREADY  3
#define CONSTVAL_IPADDRESSROWSTATUS_CREATEANDGO  4
#define CONSTVAL_IPADDRESSROWSTATUS_CREATEANDWAIT  5
#define CONSTVAL_IPADDRESSROWSTATUS_DESTROY  6
#define CONSTVAL_IPADDRESSSTORAGETYPE_OTHER  1
#define CONSTVAL_IPADDRESSSTORAGETYPE_VOLATILE  2
#define CONSTVAL_IPADDRESSSTORAGETYPE_NONVOLATILE  3
#define CONSTVAL_IPADDRESSSTORAGETYPE_PERMANENT  4
#define CONSTVAL_IPADDRESSSTORAGETYPE_READONLY  5

class ipAddressEntryBridge : public TabularMibBridge
{
  friend class ipAddressEntry;

  public:
    ipAddressEntryBridge (const BcmIpAddress &ipAddressAddr, int ipAddressIfIndex, int ipAddressType, const BcmObjectId &ipAddressPrefix, int ipAddressOrigin, int ipAddressStatus, unsigned long ipAddressCreated, unsigned long ipAddressLastChanged, unsigned long ipAddressStorageType, BcmSnmpAgent *pAgent);
    ipAddressEntryBridge (const BcmObjectId &Index, BcmSnmpAgent *pAgent);
   ~ipAddressEntryBridge ();

    /* GET methods */

    int Get_ipAddressAddrType ();
    int Get_ipAddressAddr (BcmString &ipAddressAddr);
    int Get_ipAddressIfIndex ();
    int Get_ipAddressType ();
    int Get_ipAddressPrefix (BcmObjectId &ipAddressPrefix);
    int Get_ipAddressOrigin ();
    int Get_ipAddressStatus ();
    unsigned long Get_ipAddressCreated ();
    unsigned long Get_ipAddressLastChanged ();
    int Get_ipAddressRowStatus ();
    int Get_ipAddressStorageType ();

    /* TEST methods */

    SNMP_STATUS Test_ipAddressIfIndex (int ipAddressIfIndex);
    SNMP_STATUS Test_ipAddressRowStatus (int ipAddressRowStatus);

    /* SET methods */

    SNMP_STATUS Set_ipAddressIfIndex (int ipAddressIfIndex);
    SNMP_STATUS Set_ipAddressType (int ipAddressType);
    SNMP_STATUS Set_ipAddressStatus (int ipAddressStatus);
    SNMP_STATUS Set_ipAddressRowStatus (int &ipAddressRowStatus);
    SNMP_STATUS Set_ipAddressStorageType (int ipAddressStorageType);


  protected:
    int fRowStatus;

    enum
    {
      kBIT_ipAddressAddrType = 1 << (ipAddressEntry::kLEAF_ipAddressAddrType - 1),
      kBIT_ipAddressAddr = 1 << (ipAddressEntry::kLEAF_ipAddressAddr - 1),
      kBIT_ipAddressIfIndex = 1 << (ipAddressEntry::kLEAF_ipAddressIfIndex - 1),
      kBIT_ipAddressType = 1 << (ipAddressEntry::kLEAF_ipAddressType - 1),
      kBIT_ipAddressPrefix = 1 << (ipAddressEntry::kLEAF_ipAddressPrefix - 1),
      kBIT_ipAddressOrigin = 1 << (ipAddressEntry::kLEAF_ipAddressOrigin - 1),
      kBIT_ipAddressStatus = 1 << (ipAddressEntry::kLEAF_ipAddressStatus - 1),
      kBIT_ipAddressCreated = 1 << (ipAddressEntry::kLEAF_ipAddressCreated - 1),
      kBIT_ipAddressLastChanged = 1 << (ipAddressEntry::kLEAF_ipAddressLastChanged - 1),
      kBIT_ipAddressRowStatus = 1 << (ipAddressEntry::kLEAF_ipAddressRowStatus - 1),
      kBIT_ipAddressStorageType = 1 << (ipAddressEntry::kLEAF_ipAddressStorageType - 1),
    };
    
    BcmIpAddress fIpAddressAddr;
    int fIpAddressIfIndex;
    int fIpAddressType;
    BcmObjectId fIpAddressPrefix;
    int fIpAddressOrigin;
    int fIpAddressStatus;
    unsigned long fIpAddressCreated;
    unsigned long fIpAddressLastChanged;
    unsigned long fIpAddressStorageType;
};



/*\
 *  ipNetToPhysicalEntry 
\*/

#define MINVAL_IPNETTOPHYSICALIFINDEX  1
#define MAXVAL_IPNETTOPHYSICALIFINDEX  2147483647
#define CONSTVAL_IPNETTOPHYSICALNETADDRESSTYPE_UNKNOWN  0
#define CONSTVAL_IPNETTOPHYSICALNETADDRESSTYPE_IPV4  1
#define CONSTVAL_IPNETTOPHYSICALNETADDRESSTYPE_IPV6  2
#define CONSTVAL_IPNETTOPHYSICALNETADDRESSTYPE_IPV4Z  3
#define CONSTVAL_IPNETTOPHYSICALNETADDRESSTYPE_IPV6Z  4
#define CONSTVAL_IPNETTOPHYSICALNETADDRESSTYPE_DNS  16
#define CONSTLEN_IPNETTOPHYSICALNETADDRESS_UNKNOWN  0
#define CONSTLEN_IPNETTOPHYSICALNETADDRESS_IPV4  4
#define CONSTLEN_IPNETTOPHYSICALNETADDRESS_IPV6  16
#define MINLEN_IPNETTOPHYSICALPHYSADDRESS  0
#define MAXLEN_IPNETTOPHYSICALPHYSADDRESS  65535
#define CONSTVAL_IPNETTOPHYSICALTYPE_OTHER  1
#define CONSTVAL_IPNETTOPHYSICALTYPE_INVALID  2
#define CONSTVAL_IPNETTOPHYSICALTYPE_DYNAMIC  3
#define CONSTVAL_IPNETTOPHYSICALTYPE_STATIC  4
#define CONSTVAL_IPNETTOPHYSICALTYPE_LOCAL  5
#define CONSTVAL_IPNETTOPHYSICALSTATE_REACHABLE  1
#define CONSTVAL_IPNETTOPHYSICALSTATE_STALE  2
#define CONSTVAL_IPNETTOPHYSICALSTATE_DELAY  3
#define CONSTVAL_IPNETTOPHYSICALSTATE_PROBE  4
#define CONSTVAL_IPNETTOPHYSICALSTATE_INVALID  5
#define CONSTVAL_IPNETTOPHYSICALSTATE_UNKNOWN  6
#define CONSTVAL_IPNETTOPHYSICALSTATE_INCOMPLETE  7
#define CONSTVAL_IPNETTOPHYSICALROWSTATUS_ACTIVE  1
#define CONSTVAL_IPNETTOPHYSICALROWSTATUS_NOTINSERVICE  2
#define CONSTVAL_IPNETTOPHYSICALROWSTATUS_NOTREADY  3
#define CONSTVAL_IPNETTOPHYSICALROWSTATUS_CREATEANDGO  4
#define CONSTVAL_IPNETTOPHYSICALROWSTATUS_CREATEANDWAIT  5
#define CONSTVAL_IPNETTOPHYSICALROWSTATUS_DESTROY  6

class ipNetToMediaEntryBridge;
class ipNetToPhysicalEntryBridge : public TabularMibBridge
{
  friend class ipNetToPhysicalEntry;

  public:
    ipNetToPhysicalEntryBridge (ipNetToMediaEntryBridge *pN2M);
   ~ipNetToPhysicalEntryBridge ();

    /* GET methods */

    int Get_ipNetToPhysicalIfIndex ();
    int Get_ipNetToPhysicalNetAddressType ();
    int Get_ipNetToPhysicalNetAddress (BcmString &ipNetToPhysicalNetAddress);
    void Get_ipNetToPhysicalPhysAddress (BcmMacAddress &ipNetToPhysicalPhysAddress);
    unsigned long Get_ipNetToPhysicalLastUpdated ();
    int Get_ipNetToPhysicalType ();
    int Get_ipNetToPhysicalState ();
    int Get_ipNetToPhysicalRowStatus ();

    /* TEST methods */

    SNMP_STATUS Test_ipNetToPhysicalPhysAddress (const BcmMacAddress &ipNetToPhysicalPhysAddress);
    SNMP_STATUS Test_ipNetToPhysicalRowStatus (int ipNetToPhysicalRowStatus);

    /* SET methods */

    SNMP_STATUS Set_ipNetToPhysicalPhysAddress (const BcmMacAddress &ipNetToPhysicalPhysAddress);
    SNMP_STATUS Set_ipNetToPhysicalType (int ipNetToPhysicalType);
    SNMP_STATUS Set_ipNetToPhysicalRowStatus (int &ipNetToPhysicalRowStatus);


  protected:
    int fRowStatus;

    enum
    {
      kBIT_ipNetToPhysicalIfIndex = 1 << (ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalIfIndex - 1),
      kBIT_ipNetToPhysicalNetAddressType = 1 << (ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalNetAddressType - 1),
      kBIT_ipNetToPhysicalNetAddress = 1 << (ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalNetAddress - 1),
      kBIT_ipNetToPhysicalPhysAddress = 1 << (ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalPhysAddress - 1),
      kBIT_ipNetToPhysicalLastUpdated = 1 << (ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalLastUpdated - 1),
      kBIT_ipNetToPhysicalType = 1 << (ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalType - 1),
      kBIT_ipNetToPhysicalState = 1 << (ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalState - 1),
      kBIT_ipNetToPhysicalRowStatus = 1 << (ipNetToPhysicalEntry::kLEAF_ipNetToPhysicalRowStatus - 1),
    };
    
    ipNetToMediaEntryBridge *fpN2M;
};

#if 0

/*\
 *  ipv6ScopeZoneIndexEntry 
\*/

#define MINVAL_IPV6SCOPEZONEINDEXIFINDEX  1
#define MAXVAL_IPV6SCOPEZONEINDEXIFINDEX  2147483647

class ipv6ScopeZoneIndexEntryBridge : public TabularMibBridge
{
  friend class ipv6ScopeZoneIndexEntry;

  public:
    ipv6ScopeZoneIndexEntryBridge (int ipv6ScopeZoneIndexIfIndex, BcmSnmpAgent *pAgent);
   ~ipv6ScopeZoneIndexEntryBridge ();

    /* GET methods */

    int Get_ipv6ScopeZoneIndexIfIndex ();
    unsigned int Get_ipv6ScopeZoneIndexLinkLocal ();
    unsigned int Get_ipv6ScopeZoneIndex3 ();
    unsigned int Get_ipv6ScopeZoneIndexAdminLocal ();
    unsigned int Get_ipv6ScopeZoneIndexSiteLocal ();
    unsigned int Get_ipv6ScopeZoneIndex6 ();
    unsigned int Get_ipv6ScopeZoneIndex7 ();
    unsigned int Get_ipv6ScopeZoneIndexOrganizationLocal ();
    unsigned int Get_ipv6ScopeZoneIndex9 ();
    unsigned int Get_ipv6ScopeZoneIndexA ();
    unsigned int Get_ipv6ScopeZoneIndexB ();
    unsigned int Get_ipv6ScopeZoneIndexC ();
    unsigned int Get_ipv6ScopeZoneIndexD ();

    /* TEST methods - not required */


    /* SET methods */

};

#endif

/*\
 *  ipDefaultRouterEntry 
\*/

#define CONSTVAL_IPDEFAULTROUTERADDRESSTYPE_UNKNOWN  0
#define CONSTVAL_IPDEFAULTROUTERADDRESSTYPE_IPV4  1
#define CONSTVAL_IPDEFAULTROUTERADDRESSTYPE_IPV6  2
#define CONSTVAL_IPDEFAULTROUTERADDRESSTYPE_IPV4Z  3
#define CONSTVAL_IPDEFAULTROUTERADDRESSTYPE_IPV6Z  4
#define CONSTVAL_IPDEFAULTROUTERADDRESSTYPE_DNS  16
#define CONSTLEN_IPDEFAULTROUTERADDRESS_UNKNOWN  0
#define CONSTLEN_IPDEFAULTROUTERADDRESS_IPV4  4
#define CONSTLEN_IPDEFAULTROUTERADDRESS_IPV6  16
#define MINVAL_IPDEFAULTROUTERIFINDEX  1
#define MAXVAL_IPDEFAULTROUTERIFINDEX  2147483647
#define MINVAL_IPDEFAULTROUTERLIFETIME  0
#define MAXVAL_IPDEFAULTROUTERLIFETIME  65535
#define CONSTVAL_IPDEFAULTROUTERPREFERENCE_RESERVED  -2
#define CONSTVAL_IPDEFAULTROUTERPREFERENCE_LOW  -1
#define CONSTVAL_IPDEFAULTROUTERPREFERENCE_MEDIUM  0
#define CONSTVAL_IPDEFAULTROUTERPREFERENCE_HIGH  1

class ipDefaultRouterEntryBridge : public TabularMibBridge
{
  friend class ipDefaultRouterEntry;

  public:
    ipDefaultRouterEntryBridge (const BcmIpAddress &ipDefaultRouterAddress, int ipDefaultRouterIfIndex, BcmSnmpAgent *pAgent);
   ~ipDefaultRouterEntryBridge ();

    /* GET methods */

    int Get_ipDefaultRouterAddressType ();
    int Get_ipDefaultRouterAddress (BcmString &ipDefaultRouterAddress);
    int Get_ipDefaultRouterIfIndex ();
    unsigned int Get_ipDefaultRouterLifetime ();
    int Get_ipDefaultRouterPreference ();

    /* TEST methods - not required */


    /* SET methods */

  protected:
    BcmIpAddress fIpAddress;
};

#if 0

/*\
 *  ipv6RouterAdvertEntry 
\*/

#define MINVAL_IPV6ROUTERADVERTIFINDEX  1
#define MAXVAL_IPV6ROUTERADVERTIFINDEX  2147483647
#define CONSTVAL_IPV6ROUTERADVERTSENDADVERTS_TRUE  1
#define CONSTVAL_IPV6ROUTERADVERTSENDADVERTS_FALSE  2
#define MINVAL_IPV6ROUTERADVERTMAXINTERVAL  4
#define MAXVAL_IPV6ROUTERADVERTMAXINTERVAL  1800
#define MINVAL_IPV6ROUTERADVERTMININTERVAL  3
#define MAXVAL_IPV6ROUTERADVERTMININTERVAL  1350
#define CONSTVAL_IPV6ROUTERADVERTMANAGEDFLAG_TRUE  1
#define CONSTVAL_IPV6ROUTERADVERTMANAGEDFLAG_FALSE  2
#define CONSTVAL_IPV6ROUTERADVERTOTHERCONFIGFLAG_TRUE  1
#define CONSTVAL_IPV6ROUTERADVERTOTHERCONFIGFLAG_FALSE  2
#define MINVAL_IPV6ROUTERADVERTREACHABLETIME  0
#define MAXVAL_IPV6ROUTERADVERTREACHABLETIME  3600000
#define MINVAL_IPV6ROUTERADVERTCURHOPLIMIT  0
#define MAXVAL_IPV6ROUTERADVERTCURHOPLIMIT  255
#define MINVAL_IPV6ROUTERADVERTDEFAULTLIFETIME  0|4
#define MAXVAL_IPV6ROUTERADVERTDEFAULTLIFETIME  9000
#define CONSTVAL_IPV6ROUTERADVERTROWSTATUS_ACTIVE  1
#define CONSTVAL_IPV6ROUTERADVERTROWSTATUS_NOTINSERVICE  2
#define CONSTVAL_IPV6ROUTERADVERTROWSTATUS_NOTREADY  3
#define CONSTVAL_IPV6ROUTERADVERTROWSTATUS_CREATEANDGO  4
#define CONSTVAL_IPV6ROUTERADVERTROWSTATUS_CREATEANDWAIT  5
#define CONSTVAL_IPV6ROUTERADVERTROWSTATUS_DESTROY  6

class ipv6RouterAdvertEntryBridge : public TabularMibBridge
{
  friend class ipv6RouterAdvertEntry;

  public:
    ipv6RouterAdvertEntryBridge (int ipv6RouterAdvertIfIndex, BcmSnmpAgent *pAgent);
    ipv6RouterAdvertEntryBridge (const BcmObjectId &Index, BcmSnmpAgent *pAgent);
   ~ipv6RouterAdvertEntryBridge ();

    /* GET methods */

    int Get_ipv6RouterAdvertIfIndex ();
    bool Get_ipv6RouterAdvertSendAdverts ();
    unsigned int Get_ipv6RouterAdvertMaxInterval ();
    unsigned int Get_ipv6RouterAdvertMinInterval ();
    bool Get_ipv6RouterAdvertManagedFlag ();
    bool Get_ipv6RouterAdvertOtherConfigFlag ();
    unsigned int Get_ipv6RouterAdvertLinkMTU ();
    unsigned int Get_ipv6RouterAdvertReachableTime ();
    unsigned int Get_ipv6RouterAdvertRetransmitTime ();
    unsigned int Get_ipv6RouterAdvertCurHopLimit ();
    unsigned int Get_ipv6RouterAdvertDefaultLifetime ();
    int Get_ipv6RouterAdvertRowStatus ();

    /* TEST methods */

    SNMP_STATUS Test_ipv6RouterAdvertMinInterval (unsigned int ipv6RouterAdvertMinInterval);
    SNMP_STATUS Test_ipv6RouterAdvertCurHopLimit (unsigned int ipv6RouterAdvertCurHopLimit);
    SNMP_STATUS Test_ipv6RouterAdvertDefaultLifetime (unsigned int ipv6RouterAdvertDefaultLifetime);
    SNMP_STATUS Test_ipv6RouterAdvertRowStatus (int ipv6RouterAdvertRowStatus);

    /* SET methods */

    SNMP_STATUS Set_ipv6RouterAdvertSendAdverts (bool ipv6RouterAdvertSendAdverts);
    SNMP_STATUS Set_ipv6RouterAdvertMaxInterval (unsigned int ipv6RouterAdvertMaxInterval);
    SNMP_STATUS Set_ipv6RouterAdvertMinInterval (unsigned int ipv6RouterAdvertMinInterval);
    SNMP_STATUS Set_ipv6RouterAdvertManagedFlag (bool ipv6RouterAdvertManagedFlag);
    SNMP_STATUS Set_ipv6RouterAdvertOtherConfigFlag (bool ipv6RouterAdvertOtherConfigFlag);
    SNMP_STATUS Set_ipv6RouterAdvertLinkMTU (unsigned int ipv6RouterAdvertLinkMTU);
    SNMP_STATUS Set_ipv6RouterAdvertReachableTime (unsigned int ipv6RouterAdvertReachableTime);
    SNMP_STATUS Set_ipv6RouterAdvertRetransmitTime (unsigned int ipv6RouterAdvertRetransmitTime);
    SNMP_STATUS Set_ipv6RouterAdvertCurHopLimit (unsigned int ipv6RouterAdvertCurHopLimit);
    SNMP_STATUS Set_ipv6RouterAdvertDefaultLifetime (unsigned int ipv6RouterAdvertDefaultLifetime);
    SNMP_STATUS Set_ipv6RouterAdvertRowStatus (int &ipv6RouterAdvertRowStatus);


  protected:
    int fRowStatus;

    enum
    {
      kBIT_ipv6RouterAdvertIfIndex = 1 << (ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertIfIndex - 1),
      kBIT_ipv6RouterAdvertSendAdverts = 1 << (ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertSendAdverts - 1),
      kBIT_ipv6RouterAdvertMaxInterval = 1 << (ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertMaxInterval - 1),
      kBIT_ipv6RouterAdvertMinInterval = 1 << (ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertMinInterval - 1),
      kBIT_ipv6RouterAdvertManagedFlag = 1 << (ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertManagedFlag - 1),
      kBIT_ipv6RouterAdvertOtherConfigFlag = 1 << (ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertOtherConfigFlag - 1),
      kBIT_ipv6RouterAdvertLinkMTU = 1 << (ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertLinkMTU - 1),
      kBIT_ipv6RouterAdvertReachableTime = 1 << (ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertReachableTime - 1),
      kBIT_ipv6RouterAdvertRetransmitTime = 1 << (ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertRetransmitTime - 1),
      kBIT_ipv6RouterAdvertCurHopLimit = 1 << (ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertCurHopLimit - 1),
      kBIT_ipv6RouterAdvertDefaultLifetime = 1 << (ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertDefaultLifetime - 1),
      kBIT_ipv6RouterAdvertRowStatus = 1 << (ipv6RouterAdvertEntry::kLEAF_ipv6RouterAdvertRowStatus - 1),
    };
};

#endif

/*\
 *  ipAddrEntry 
\*/

#define MINVAL_IPADENTIFINDEX  1
#define MAXVAL_IPADENTIFINDEX  2147483647
#define MINVAL_IPADENTBCASTADDR  0
#define MAXVAL_IPADENTBCASTADDR  1
#define MINVAL_IPADENTREASMMAXSIZE  0
#define MAXVAL_IPADENTREASMMAXSIZE  65535

class ipAddrEntryBridge : public TabularMibBridge
{
  friend class ipAddrEntry;
  friend class BcmSnmpAgent;

  public:
    ipAddrEntryBridge (const BcmIpAddress &ipAdEntAddr, 
                       int ipAdEntIfIndex,
                       const BcmIpAddress &ipAdEntNetMask,
                       int ipAdEntBcastAddr,
                       int IpAdEntReasmMaxSize, 
                       BcmSnmpAgent *pAgent);
   ~ipAddrEntryBridge ();

    /* GET methods */

    void Get_ipAdEntAddr (BcmIpAddress &ipAdEntAddr);
    int Get_ipAdEntIfIndex ();
    void Get_ipAdEntNetMask (BcmIpAddress &ipAdEntNetMask);
    int Get_ipAdEntBcastAddr ();
    int Get_ipAdEntReasmMaxSize ();

    /* TEST methods - not required */


    /* SET methods */
    
  protected:
    BcmIpAddress fIpAdEntAddr;
    int fIpAdEntIfIndex;
    BcmIpAddress fIpAdEntNetMask;
    int fIpAdEntBcastAddr;
    int fIpAdEntReasmMaxSize;
};



/*\
 *  ipNetToMediaEntry 
\*/

#define MINVAL_IPNETTOMEDIAIFINDEX  1
#define MAXVAL_IPNETTOMEDIAIFINDEX  2147483647
#define MINLEN_IPNETTOMEDIAPHYSADDRESS  0
#define MAXLEN_IPNETTOMEDIAPHYSADDRESS  65535
#define CONSTVAL_IPNETTOMEDIATYPE_OTHER  1
#define CONSTVAL_IPNETTOMEDIATYPE_INVALID  2
#define CONSTVAL_IPNETTOMEDIATYPE_DYNAMIC  3
#define CONSTVAL_IPNETTOMEDIATYPE_STATIC  4

// Forward reference:
class BcmHalIf;

class ipNetToMediaEntryBridge : public TabularMibBridge
{
  friend class ipNetToMediaEntry;
  friend class BcmNetToMediaSnoop;

  public:
    // NOTE: The pHal argument in this constructor points to the BcmHalIf object
    //       which is connected to the IP/MAC address mapping being performed.
    //       The ipNetToMediaIfIndex value for this mapping will be the
    //       ifIndex value for this HAL.  For entries which are learned via
    //       the BcmNetToMediaSnoop, this is the interface which sent the
    //       incoming packet to the IP stack, or to which the IP stack sent the
    //       outgoing packet (eg ethernet).
    //
    //       The IpStackInterface argument, on the other hand, indicates which
    //       IP stack this entry is associated with.  For entries which are
    //       learned via the BcmNetToMediaSnoop, this is the interface
    //       number (NOT ifIndex) of the IP stack which sent or received the 
    //       packet from which we learn the association.
    //
    //       The reason for the IpStackInterface argument is because we might
    //       have a single snoop servicing multiple agents, should we have a
    //       master agent situation.  In that case, when one agent's IP address
    //       changes, for example (see PR 5814), we don't want to lose all the
    //       entries for the other agent.  This argument gives us a way to 
    //       keep track of which entry is related to which IP stack.
    ipNetToMediaEntryBridge (const BcmHalIf *pHal,
                             const BcmMacAddress &ipNetToMediaPhysAddress, 
                             const BcmIpAddress &ipNetToMediaNetAddress, 
                             int ipNetToMediaType, 
                             int IpStackInterface,
                             BcmSnmpAgent *pAgent);
                             
   ~ipNetToMediaEntryBridge ();
   
   SNMP_STATUS Register(CleanupMethod Cleanup, bool SilentFailure=false);
   
    /* GET methods */

    int Get_ipNetToMediaIfIndex ();
    void Get_ipNetToMediaPhysAddress (BcmMacAddress &ipNetToMediaPhysAddress);
    void Get_ipNetToMediaNetAddress (BcmIpAddress &ipNetToMediaNetAddress);
    int Get_ipNetToMediaType ();

    /* TEST methods - not required */


    /* SET methods */

    SNMP_STATUS Set_ipNetToMediaIfIndex (int ipNetToMediaIfIndex);
    SNMP_STATUS Set_ipNetToMediaPhysAddress (const BcmMacAddress &ipNetToMediaPhysAddress);
    SNMP_STATUS Set_ipNetToMediaNetAddress (const BcmIpAddress &ipNetToMediaNetAddress);
    SNMP_STATUS Set_ipNetToMediaType (int ipNetToMediaType);
    
    
    /* Accessor methods */
    
    inline const BcmHalIf *GetHal (void) const { return fpHal; }
    inline int GetIpStackInterface (void) const { return fIpStackIf; }
    inline time_t GetLastUpdate (bool Changed = false) const;
    
  protected:
    const BcmHalIf *fpHal;
    BcmMacAddress fMac;
    BcmIpAddress fIp;
    int fType;
    int fIpStackIf;
    time_t fLastUpdate;
    time_t fLastChange;
    ipNetToPhysicalEntryBridge *fpN2P;
    
    // Called by our friend class BcmNetToMediaSnoop.  Note that we don't 
    // accept a BcmHalIf here since the ifIndex is part of our index, if it
    // changes you must delete the bridge object and create a new one.
    void Update (const BcmMacAddress &Mac, int Type);
};

inline time_t ipNetToMediaEntryBridge::GetLastUpdate (bool Changed) const
{
  if (Changed)
    return fLastChange;
    
  return fLastUpdate;
}

#if 0

/*\
 *  ipTrafficStatsGroup 
\*/


class ipTrafficStatsGroupBridge : public ScalarMibBridge
{
  friend class ipTrafficStatsGroup;

  public:
    ipTrafficStatsGroupBridge (BcmSnmpAgent *pAgent);
   ~ipTrafficStatsGroupBridge ();

    /* GET methods */

    unsigned long Get_ipIfStatsTableLastChange ();

    /* TEST methods - not required */


    /* SET methods */

};



/*\
 *  ipSystemStatsEntry 
\*/

#define CONSTVAL_IPSYSTEMSTATSIPVERSION_UNKNOWN  0
#define CONSTVAL_IPSYSTEMSTATSIPVERSION_IPV4  1
#define CONSTVAL_IPSYSTEMSTATSIPVERSION_IPV6  2

class ipSystemStatsEntryBridge : public TabularMibBridge
{
  friend class ipSystemStatsEntry;

  public:
    ipSystemStatsEntryBridge (int ipSystemStatsIPVersion, BcmSnmpAgent *pAgent);
   ~ipSystemStatsEntryBridge ();

    /* GET methods */

    int Get_ipSystemStatsIPVersion ();
    unsigned int Get_ipSystemStatsInReceives ();
    unsigned int Get_ipSystemStatsHCInReceives (BcmCounter64 &ipSystemStatsHCInReceives);
    unsigned int Get_ipSystemStatsInOctets ();
    unsigned int Get_ipSystemStatsHCInOctets (BcmCounter64 &ipSystemStatsHCInOctets);
    unsigned int Get_ipSystemStatsInHdrErrors ();
    unsigned int Get_ipSystemStatsInNoRoutes ();
    unsigned int Get_ipSystemStatsInAddrErrors ();
    unsigned int Get_ipSystemStatsInUnknownProtos ();
    unsigned int Get_ipSystemStatsInTruncatedPkts ();
    unsigned int Get_ipSystemStatsInForwDatagrams ();
    unsigned int Get_ipSystemStatsHCInForwDatagrams (BcmCounter64 &ipSystemStatsHCInForwDatagrams);
    unsigned int Get_ipSystemStatsReasmReqds ();
    unsigned int Get_ipSystemStatsReasmOKs ();
    unsigned int Get_ipSystemStatsReasmFails ();
    unsigned int Get_ipSystemStatsInDiscards ();
    unsigned int Get_ipSystemStatsInDelivers ();
    unsigned int Get_ipSystemStatsHCInDelivers (BcmCounter64 &ipSystemStatsHCInDelivers);
    unsigned int Get_ipSystemStatsOutRequests ();
    unsigned int Get_ipSystemStatsHCOutRequests (BcmCounter64 &ipSystemStatsHCOutRequests);
    unsigned int Get_ipSystemStatsOutNoRoutes ();
    unsigned int Get_ipSystemStatsOutForwDatagrams ();
    unsigned int Get_ipSystemStatsHCOutForwDatagrams (BcmCounter64 &ipSystemStatsHCOutForwDatagrams);
    unsigned int Get_ipSystemStatsOutDiscards ();
    unsigned int Get_ipSystemStatsOutFragReqds ();
    unsigned int Get_ipSystemStatsOutFragOKs ();
    unsigned int Get_ipSystemStatsOutFragFails ();
    unsigned int Get_ipSystemStatsOutFragCreates ();
    unsigned int Get_ipSystemStatsOutTransmits ();
    unsigned int Get_ipSystemStatsHCOutTransmits (BcmCounter64 &ipSystemStatsHCOutTransmits);
    unsigned int Get_ipSystemStatsOutOctets ();
    unsigned int Get_ipSystemStatsHCOutOctets (BcmCounter64 &ipSystemStatsHCOutOctets);
    unsigned int Get_ipSystemStatsInMcastPkts ();
    unsigned int Get_ipSystemStatsHCInMcastPkts (BcmCounter64 &ipSystemStatsHCInMcastPkts);
    unsigned int Get_ipSystemStatsInMcastOctets ();
    unsigned int Get_ipSystemStatsHCInMcastOctets (BcmCounter64 &ipSystemStatsHCInMcastOctets);
    unsigned int Get_ipSystemStatsOutMcastPkts ();
    unsigned int Get_ipSystemStatsHCOutMcastPkts (BcmCounter64 &ipSystemStatsHCOutMcastPkts);
    unsigned int Get_ipSystemStatsOutMcastOctets ();
    unsigned int Get_ipSystemStatsHCOutMcastOctets (BcmCounter64 &ipSystemStatsHCOutMcastOctets);
    unsigned int Get_ipSystemStatsInBcastPkts ();
    unsigned int Get_ipSystemStatsHCInBcastPkts (BcmCounter64 &ipSystemStatsHCInBcastPkts);
    unsigned int Get_ipSystemStatsOutBcastPkts ();
    unsigned int Get_ipSystemStatsHCOutBcastPkts (BcmCounter64 &ipSystemStatsHCOutBcastPkts);
    unsigned long Get_ipSystemStatsDiscontinuityTime ();
    unsigned int Get_ipSystemStatsRefreshRate ();

    /* TEST methods - not required */


    /* SET methods */

};



/*\
 *  ipIfStatsEntry 
\*/

#define CONSTVAL_IPIFSTATSIPVERSION_UNKNOWN  0
#define CONSTVAL_IPIFSTATSIPVERSION_IPV4  1
#define CONSTVAL_IPIFSTATSIPVERSION_IPV6  2
#define MINVAL_IPIFSTATSIFINDEX  1
#define MAXVAL_IPIFSTATSIFINDEX  2147483647

class ipIfStatsEntryBridge : public TabularMibBridge
{
  friend class ipIfStatsEntry;

  public:
    ipIfStatsEntryBridge (int ipIfStatsIPVersion, int ipIfStatsIfIndex, BcmSnmpAgent *pAgent);
   ~ipIfStatsEntryBridge ();

    /* GET methods */

    int Get_ipIfStatsIPVersion ();
    int Get_ipIfStatsIfIndex ();
    unsigned int Get_ipIfStatsInReceives ();
    unsigned int Get_ipIfStatsHCInReceives (BcmCounter64 &ipIfStatsHCInReceives);
    unsigned int Get_ipIfStatsInOctets ();
    unsigned int Get_ipIfStatsHCInOctets (BcmCounter64 &ipIfStatsHCInOctets);
    unsigned int Get_ipIfStatsInHdrErrors ();
    unsigned int Get_ipIfStatsInNoRoutes ();
    unsigned int Get_ipIfStatsInAddrErrors ();
    unsigned int Get_ipIfStatsInUnknownProtos ();
    unsigned int Get_ipIfStatsInTruncatedPkts ();
    unsigned int Get_ipIfStatsInForwDatagrams ();
    unsigned int Get_ipIfStatsHCInForwDatagrams (BcmCounter64 &ipIfStatsHCInForwDatagrams);
    unsigned int Get_ipIfStatsReasmReqds ();
    unsigned int Get_ipIfStatsReasmOKs ();
    unsigned int Get_ipIfStatsReasmFails ();
    unsigned int Get_ipIfStatsInDiscards ();
    unsigned int Get_ipIfStatsInDelivers ();
    unsigned int Get_ipIfStatsHCInDelivers (BcmCounter64 &ipIfStatsHCInDelivers);
    unsigned int Get_ipIfStatsOutRequests ();
    unsigned int Get_ipIfStatsHCOutRequests (BcmCounter64 &ipIfStatsHCOutRequests);
    unsigned int Get_ipIfStatsOutForwDatagrams ();
    unsigned int Get_ipIfStatsHCOutForwDatagrams (BcmCounter64 &ipIfStatsHCOutForwDatagrams);
    unsigned int Get_ipIfStatsOutDiscards ();
    unsigned int Get_ipIfStatsOutFragReqds ();
    unsigned int Get_ipIfStatsOutFragOKs ();
    unsigned int Get_ipIfStatsOutFragFails ();
    unsigned int Get_ipIfStatsOutFragCreates ();
    unsigned int Get_ipIfStatsOutTransmits ();
    unsigned int Get_ipIfStatsHCOutTransmits (BcmCounter64 &ipIfStatsHCOutTransmits);
    unsigned int Get_ipIfStatsOutOctets ();
    unsigned int Get_ipIfStatsHCOutOctets (BcmCounter64 &ipIfStatsHCOutOctets);
    unsigned int Get_ipIfStatsInMcastPkts ();
    unsigned int Get_ipIfStatsHCInMcastPkts (BcmCounter64 &ipIfStatsHCInMcastPkts);
    unsigned int Get_ipIfStatsInMcastOctets ();
    unsigned int Get_ipIfStatsHCInMcastOctets (BcmCounter64 &ipIfStatsHCInMcastOctets);
    unsigned int Get_ipIfStatsOutMcastPkts ();
    unsigned int Get_ipIfStatsHCOutMcastPkts (BcmCounter64 &ipIfStatsHCOutMcastPkts);
    unsigned int Get_ipIfStatsOutMcastOctets ();
    unsigned int Get_ipIfStatsHCOutMcastOctets (BcmCounter64 &ipIfStatsHCOutMcastOctets);
    unsigned int Get_ipIfStatsInBcastPkts ();
    unsigned int Get_ipIfStatsHCInBcastPkts (BcmCounter64 &ipIfStatsHCInBcastPkts);
    unsigned int Get_ipIfStatsOutBcastPkts ();
    unsigned int Get_ipIfStatsHCOutBcastPkts (BcmCounter64 &ipIfStatsHCOutBcastPkts);
    unsigned long Get_ipIfStatsDiscontinuityTime ();
    unsigned int Get_ipIfStatsRefreshRate ();

    /* TEST methods - not required */


    /* SET methods */

};

#endif

/*\
 *  icmpGroup 
\*/

// From RFC-1700, ICMP message type codes:
#define MINVAL_ICMPMSGTYPE  0
#define MAXVAL_ICMPMSGTYPE  255
#define MAXNUM_ICMPMTYPESV4 24
#define CONSTVAL_ICMPMSGTYPE_ECHOREP  0
#define CONSTVAL_ICMPMSGTYPE_DESTUNREACH  3
#define CONSTVAL_ICMPMSGTYPE_SRCQUENCH  4
#define CONSTVAL_ICMPMSGTYPE_REDIRECT  5
#define CONSTVAL_ICMPMSGTYPE_ALTERNATEHOSTADDRESS  6
#define CONSTVAL_ICMPMSGTYPE_ECHO  8
#define CONSTVAL_ICMPMSGTYPE_ROUTERADVERTISEMENT  9
#define CONSTVAL_ICMPMSGTYPE_ROUTERSELECTION  10
#define CONSTVAL_ICMPMSGTYPE_TIMEEXCD  11
#define CONSTVAL_ICMPMSGTYPE_PARMPROB  12
#define CONSTVAL_ICMPMSGTYPE_TIMESTAMP  13
#define CONSTVAL_ICMPMSGTYPE_TIMESTAMPREP  14
#define CONSTVAL_ICMPMSGTYPE_INFORMATIONREQUEST  15
#define CONSTVAL_ICMPMSGTYPE_INFORMATIONREPLY  16
#define CONSTVAL_ICMPMSGTYPE_ADDRMASK  17
#define CONSTVAL_ICMPMSGTYPE_ADDRMASKREP  18
#define CONSTVAL_ICMPMSGTYPE_TRACEROUTE  30
#define CONSTVAL_ICMPMSGTYPE_DATAGRAMCONVERSIONERROR  31
#define CONSTVAL_ICMPMSGTYPE_MOBILEHOSTREDIRECT  32
#define CONSTVAL_ICMPMSGTYPE_IPV6WHEREAREYOU  33
#define CONSTVAL_ICMPMSGTYPE_IPV6IAMHERE  34
#define CONSTVAL_ICMPMSGTYPE_MOBILEREGISTRATIONREQUEST  35
#define CONSTVAL_ICMPMSGTYPE_MOBILEREGISTRATIONREPLY  36
#define CONSTVAL_ICMPMSGTYPE_DOMAINNAMEREQUEST  37

#if (BCM_IPV6_SUPPORT)
// ICMPv6 Type Numbers
// from www.iana.org/assignments/icmpv6-parameters
#define MAXNUM_ICMPMTYPESV6 29
#define CONSTVAL_ICMPv6MSGTYPE_DESTUNREACH  1
#define CONSTVAL_ICMPv6MSGTYPE_PKTTOOBIG  2
#define CONSTVAL_ICMPv6MSGTYPE_TIMEEXCD  3
#define CONSTVAL_ICMPv6MSGTYPE_PARMPROBLEM  4
#define CONSTVAL_ICMPv6MSGTYPE_ECHO  128
#define CONSTVAL_ICMPv6MSGTYPE_ECHOREP  129
#define CONSTVAL_ICMPv6MSGTYPE_MCLISTENERQUERY  130
#define CONSTVAL_ICMPv6MSGTYPE_MCLISTENERREPORT  131
#define CONSTVAL_ICMPv6MSGTYPE_MCLISTENERDONE  132
#define CONSTVAL_ICMPv6MSGTYPE_ROUTERSOLICITATION  133
#define CONSTVAL_ICMPv6MSGTYPE_ROUTERADVERTISEMENT  134
#define CONSTVAL_ICMPv6MSGTYPE_NEIGHBORSOLICITATION  135
#define CONSTVAL_ICMPv6MSGTYPE_NEIGHBORADVERTISEMENT  136
#define CONSTVAL_ICMPv6MSGTYPE_REDIRECT  137
#define CONSTVAL_ICMPv6MSGTYPE_ROUTERRENUMBER  138
#define CONSTVAL_ICMPv6MSGTYPE_NODEINFOQUERY  139
#define CONSTVAL_ICMPv6MSGTYPE_NODEINFORESPONSE  140
#define CONSTVAL_ICMPv6MSGTYPE_INVNDSOLICITATION  141
#define CONSTVAL_ICMPv6MSGTYPE_INVNDADVERTISEMENT  142
#define CONSTVAL_ICMPv6MSGTYPE_V2MCLISTENERREPORT  143
#define CONSTVAL_ICMPv6MSGTYPE_HAADDRDISCREQUEST  144
#define CONSTVAL_ICMPv6MSGTYPE_HAADDRDISCREPLY  145
#define CONSTVAL_ICMPv6MSGTYPE_MOBILEPRESOLICITATION  146
#define CONSTVAL_ICMPv6MSGTYPE_MOBILEPREADVERTISEMENT  147 
#define CONSTVAL_ICMPv6MSGTYPE_CERTPATHSOLICITATION  148
#define CONSTVAL_ICMPv6MSGTYPE_CERTPATHADVERTISEMENT  149
#define CONSTVAL_ICMPv6MSGTYPE_MCROUTERADVERTISEMENT  151
#define CONSTVAL_ICMPv6MSGTYPE_MCROUTERSOLICITATION  152
#define CONSTVAL_ICMPv6MSGTYPE_MCROUTERTERMINATION  153
#endif 

class icmpGroupBridge : public ScalarMibBridge
{
  friend class icmpGroup;

  public:
    icmpGroupBridge (BcmSnmpAgent *pAgent);
   ~icmpGroupBridge ();
   
    SNMP_STATUS Register(CleanupMethod Cleanup, bool SilentFailure=false);

    /* GET methods */

    unsigned int Get_icmpInMsgs ();
    unsigned int Get_icmpInErrors ();
    unsigned int Get_icmpInDestUnreachs ();
    unsigned int Get_icmpInTimeExcds ();
    unsigned int Get_icmpInParmProbs ();
    unsigned int Get_icmpInSrcQuenchs ();
    unsigned int Get_icmpInRedirects ();
    unsigned int Get_icmpInEchos ();
    unsigned int Get_icmpInEchoReps ();
    unsigned int Get_icmpInTimestamps ();
    unsigned int Get_icmpInTimestampReps ();
    unsigned int Get_icmpInAddrMasks ();
    unsigned int Get_icmpInAddrMaskReps ();
    unsigned int Get_icmpOutMsgs ();
    unsigned int Get_icmpOutErrors ();
    unsigned int Get_icmpOutDestUnreachs ();
    unsigned int Get_icmpOutTimeExcds ();
    unsigned int Get_icmpOutParmProbs ();
    unsigned int Get_icmpOutSrcQuenchs ();
    unsigned int Get_icmpOutRedirects ();
    unsigned int Get_icmpOutEchos ();
    unsigned int Get_icmpOutEchoReps ();
    unsigned int Get_icmpOutTimestamps ();
    unsigned int Get_icmpOutTimestampReps ();
    unsigned int Get_icmpOutAddrMasks ();
    unsigned int Get_icmpOutAddrMaskReps ();

    /* TEST methods - not required */

    /* SET methods */
    
    /* Helper methods - derived objects can / should override these */
    
    virtual unsigned int IcmpInMsgs (int MsgType = -1) const { return 0; }
    virtual unsigned int IcmpInErrors () const { return 0; }
    virtual unsigned int IcmpOutMsgs (int MsgType = -1) const { return 0; }
    virtual unsigned int IcmpOutErrors () const { return 0; }
    
    #if (BCM_IPV6_SUPPORT)
    virtual unsigned int Icmp6InMsgs (int MsgType = -1) const { return 0; }
    virtual unsigned int Icmp6InErrors () const { return 0; }
    virtual unsigned int Icmp6OutMsgs (int MsgType = -1) const { return 0; }
    virtual unsigned int Icmp6OutErrors () const { return 0; }
    #endif
};



/*\
 *  icmpStatsEntry 
\*/

#define CONSTVAL_ICMPSTATSIPVERSION_UNKNOWN  0
#define CONSTVAL_ICMPSTATSIPVERSION_IPV4  1
#define CONSTVAL_ICMPSTATSIPVERSION_IPV6  2

class icmpStatsEntryBridge : public TabularMibBridge
{
  friend class icmpStatsEntry;

  public:
    icmpStatsEntryBridge (int icmpStatsIPVersion, icmpGroupBridge *pIcmpBridge, BcmSnmpAgent *pAgent);
   ~icmpStatsEntryBridge ();

    /* GET methods */

    int Get_icmpStatsIPVersion ();
    unsigned int Get_icmpStatsInMsgs ();
    unsigned int Get_icmpStatsInErrors ();
    unsigned int Get_icmpStatsOutMsgs ();
    unsigned int Get_icmpStatsOutErrors ();

    /* TEST methods - not required */


    /* SET methods */

  protected:
    icmpGroupBridge *fpIcmpBridge;
};



/*\
 *  icmpMsgStatsEntry 
\*/

#define CONSTVAL_ICMPMSGSTATSIPVERSION_UNKNOWN  0
#define CONSTVAL_ICMPMSGSTATSIPVERSION_IPV4  1
#define CONSTVAL_ICMPMSGSTATSIPVERSION_IPV6  2
#define MINVAL_ICMPMSGSTATSTYPE  0
#define MAXVAL_ICMPMSGSTATSTYPE  255

class icmpMsgStatsEntryBridge : public TabularMibBridge
{
  friend class icmpMsgStatsEntry;

  public:
    icmpMsgStatsEntryBridge (int icmpMsgStatsIPVersion, int icmpMsgStatsType, icmpGroupBridge *pIcmpBridge, BcmSnmpAgent *pAgent);
   ~icmpMsgStatsEntryBridge ();

    /* GET methods */

    int Get_icmpMsgStatsIPVersion ();
    int Get_icmpMsgStatsType ();
    unsigned int Get_icmpMsgStatsInPkts ();
    unsigned int Get_icmpMsgStatsOutPkts ();

    /* TEST methods - not required */


    /* SET methods */

  protected:
    icmpGroupBridge *fpIcmpBridge;
};

#endif
