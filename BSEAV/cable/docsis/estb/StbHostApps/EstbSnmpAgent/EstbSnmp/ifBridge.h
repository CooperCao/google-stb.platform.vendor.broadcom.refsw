//****************************************************************************
//
// Copyright (c) 2007-2009 Broadcom Corporation
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
//    Filename: ifBridge.h
//    Creation Date: Wednesday, December 04, 2002 at 14:00:30
//    Created by Broadcom V2 Mib Compiler BCMIBC.EXE version 1.4
//    from input file rfc2233.mib
//
//**************************************************************************

#ifndef IFBRIDGE_H
#define IFBRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

#include "ifMib.h"

#ifdef BCM_STANDALONE_SNMP_AGENT

#include "estbIfBridge.h"

#else

/*\
 *  interfacesGroup 
\*/

// Forward references:
class BcmPacketSwitch;
class ifTable;
class BcmHalIf;
class BcmInterfaceCounters;
class ifXEntryBridge;
class ifStackTable;
class ifEntryBridge;
class BcmStubHalIf;

class interfacesGroupBridge : public ScalarMibBridge
{
  friend class interfacesGroup;

  public:
    interfacesGroupBridge (BcmPacketSwitch *pPktSwitch, BcmSnmpAgent *pAgent);
   ~interfacesGroupBridge ();
   
    SNMP_STATUS Register(CleanupMethod Cleanup, bool SilentFailure=false);
   
    inline bool HideIpStackIfEntries() const { return fHideIpStackIfEntries; }

    /* GET methods */

    int Get_ifNumber ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    /* ifTable accessor methods */
    
    ifEntryBridge *FindIfEntry (int ifIndex);
    ifEntryBridge *FindIfEntry (const BcmHalIf *pHal);
   
    ifEntryBridge *FirstIfEntry ();
    ifEntryBridge *NextIfEntry (int ifIndex);
    
    ifEntryBridge *FirstIfEntryOfType (int Type);
    ifEntryBridge *NextIfEntryOfType (int ifIndex, int Type);
    
    ifEntryBridge *FirstIfEntryOfIfNum (int ifNum);
    ifEntryBridge *NextIfEntryOfIfNum (int ifNum, int Type);
    
  public:
    static bool fHideIpStackIfEntries;

  protected:
    BcmPacketSwitch *fpPktSwitch;
    ifTable *fpIfTable;  
    BcmStubHalIf *fpStubHal;
    int fNextIfIndex;
    
    friend class ifEntryBridge;
};



/*\
 *  ifEntry 
\*/

#define MINVAL_IFINDEX  1
#define MAXVAL_IFINDEX  2147483647
#define MINLEN_IFDESCR  0
#define MAXLEN_IFDESCR  255
// ifType values are in IANAifType.h
#define CONSTVAL_IFADMINSTATUS_UP  1
#define CONSTVAL_IFADMINSTATUS_DOWN  2
#define CONSTVAL_IFADMINSTATUS_TESTING  3
#define CONSTVAL_IFOPERSTATUS_UP  1
#define CONSTVAL_IFOPERSTATUS_DOWN  2
#define CONSTVAL_IFOPERSTATUS_TESTING  3
#define CONSTVAL_IFOPERSTATUS_UNKNOWN  4
#define CONSTVAL_IFOPERSTATUS_DORMANT  5
#define CONSTVAL_IFOPERSTATUS_NOTPRESENT  6
#define CONSTVAL_IFOPERSTATUS_LOWERLAYERDOWN  7

class ifEntryBridge : public TabularMibBridge
{
  friend class ifEntry;

  public:
    ifEntryBridge (BcmInterfaceCounters *pInterfaceCounters, 
                   BcmSnmpAgent *pAgent, int ifIndex = 0, int SubIface = 0, 
                   int ifType = 0);
   ~ifEntryBridge ();

    virtual SNMP_STATUS Register(CleanupMethod Cleanup, bool SilentFailure=false);
    virtual SNMP_STATUS DeRegister();

    /* GET methods */

    inline int Get_ifIndex ();
    virtual int Get_ifDescr (BcmString &ifDescr);
    inline int Get_ifType (bool OtherIfUnassigned = false);
    virtual int Get_ifTypeImpl ();
    virtual int Get_ifMtu ();
    virtual unsigned int Get_ifSpeed ();
    virtual void Get_ifPhysAddress (BcmMacAddress &ifPhysAddress);
    virtual int Get_ifAdminStatus ();
    virtual int Get_ifOperStatus ();
    virtual time_t Get_ifLastChange ();
    virtual unsigned int Get_ifInOctets ();
    virtual unsigned int Get_ifInUcastPkts ();
    virtual unsigned int Get_ifInNUcastPkts ();
    virtual unsigned int Get_ifInDiscards ();
    virtual unsigned int Get_ifInErrors ();
    virtual unsigned int Get_ifInUnknownProtos ();
    virtual unsigned int Get_ifOutOctets ();
    virtual unsigned int Get_ifOutUcastPkts ();
    virtual unsigned int Get_ifOutNUcastPkts ();
    virtual unsigned int Get_ifOutDiscards ();
    virtual unsigned int Get_ifOutErrors ();
    virtual unsigned int Get_ifOutQLen ();
    virtual int Get_ifSpecific (BcmObjectId &ifSpecific);

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    virtual SNMP_STATUS Set_ifAdminStatus (int ifAdminStatus);
    
    /* Accessors for non-mib items, used elsewhere */
    
    virtual int Get_ifNumber ();
    virtual bool IsDefaultInterface ();
    
    ifXEntryBridge *IfXBridge(void) { return fpIfXBridge; }
    BcmInterfaceCounters *InterfaceCounters(void) { return fpInterfaceCounters; }

    /* Method needed to set Interface Counter */
    SNMP_STATUS Set_ifInterfaceCounter (BcmInterfaceCounters * pinterfaceCounters);

  protected:
    ifXEntryBridge *fpIfXBridge;
    BcmInterfaceCounters *fpInterfaceCounters;

  protected:    
    // These are logical numbers used in the SNMP ifTable to index the
    // interfaces as required by DOCSIS OSS-N-99020
    enum
    {
        kEthernetIx = 1,
        kCmMacIx,
        kCmDownstreamIx,
        kCmUpstreamIx,
        kUsbIx,
        kHpnaIx,
        kPciIx,
        
        /* INSERT ANY FUTURE INTERFACE TYPES HERE */
        
        kIpStackIx,
        kOtherIx  // Just in case we get a type we don't recognize
    } IfTableIndex;
    
    ifStackTable   *fpIfStackTable;
    time_t fLastChange;
    
    // Sub-interface info, if there is one:
    int fSubIface;
    int fIfType;
    int fIfIndex;
    
    // Assign an ifIndex based on type.  This is to meet DOCSIS
    // requirements which map an ifIndex to a IANA type.  Note that
    // this could be trouble if we ever have multiple interfaces with the
    // same type!!
    int AssignIfIndex (int RequestedIfIndex);
    
    SNMP_STATUS AddToIfStack ();
    SNMP_STATUS RemoveFromIfStack ();
};

/* 
ifIndex OBJECT-TYPE
       SYNTAX      InterfaceIndex
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "A unique value, greater than zero, for each
               interface.  It is recommended that values are assigned
               contiguously starting from 1.  The value for each
               interface sub-layer must remain constant at least from
               one re-initialization of the entity's network
               management system to the next re-initialization."
       ::= { ifEntry 1 }
*/ 
inline int ifEntryBridge::Get_ifIndex ()
{
  return fIfIndex;
}

/* 
ifType OBJECT-TYPE
       SYNTAX      IANAifType
       MAX-ACCESS  read-only
       STATUS      current
       DESCRIPTION
               "The type of interface.  Additional values for ifType
               are assigned by the Internet Assigned Numbers
               Authority (IANA), through updating the syntax of the
               IANAifType textual convention."
       ::= { ifEntry 3 }
*/ 
inline int ifEntryBridge::Get_ifType (bool OtherIfUnassigned)
{
  // If called with OtherIfUnassigned = true, we'll return a value of
  // Other(1) for any unassigned type.  This is the version called for
  // MIB queries.  Otherwise, we'll return the real value.
  if (OtherIfUnassigned == true)  
    return Get_ifTypeImpl();
    
  return fIfType;
}


/*\
 *  ifMIBObjectsGroup 
\*/


class ifMIBObjectsGroupBridge : public ScalarMibBridge
{
  friend class ifMIBObjectsGroup;

  public:
    ifMIBObjectsGroupBridge (BcmSnmpAgent *pAgent);
   ~ifMIBObjectsGroupBridge ();

    /* GET methods */

    unsigned long Get_ifTableLastChange ();
    unsigned long Get_ifStackLastChange ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

};



/*\
 *  ifXEntry 
\*/

#define MINLEN_IFNAME  0
#define MAXLEN_IFNAME  255
#define CONSTVAL_IFLINKUPDOWNTRAPENABLE_ENABLED  1
#define CONSTVAL_IFLINKUPDOWNTRAPENABLE_DISABLED  2
#define CONSTVAL_IFPROMISCUOUSMODE_TRUE  1
#define CONSTVAL_IFPROMISCUOUSMODE_FALSE  2
#define CONSTVAL_IFCONNECTORPRESENT_TRUE  1
#define CONSTVAL_IFCONNECTORPRESENT_FALSE  2
#define MINLEN_IFALIAS  0
#define MAXLEN_IFALIAS  64

class ifXEntryBridge : public TabularMibBridge
{
  friend class ifXEntry;

  public:
    ifXEntryBridge (BcmInterfaceCounters *pInterfaceCounters,
                    ifEntryBridge *pIfEntry, BcmSnmpAgent *pAgent);
   ~ifXEntryBridge ();

    /* GET methods */

    virtual int Get_ifName (BcmString &ifName);
    virtual unsigned int Get_ifInMulticastPkts ();
    virtual unsigned int Get_ifInBroadcastPkts ();
    virtual unsigned int Get_ifOutMulticastPkts ();
    virtual unsigned int Get_ifOutBroadcastPkts ();
    virtual unsigned int Get_ifHCInOctets (BcmCounter64 &ifHCInOctets);
    virtual unsigned int Get_ifHCInUcastPkts (BcmCounter64 &ifHCInUcastPkts);
    virtual unsigned int Get_ifHCInMulticastPkts (BcmCounter64 &ifHCInMulticastPkts);
    virtual unsigned int Get_ifHCInBroadcastPkts (BcmCounter64 &ifHCInBroadcastPkts);
    virtual unsigned int Get_ifHCOutOctets (BcmCounter64 &ifHCOutOctets);
    virtual unsigned int Get_ifHCOutUcastPkts (BcmCounter64 &ifHCOutUcastPkts);
    virtual unsigned int Get_ifHCOutMulticastPkts (BcmCounter64 &ifHCOutMulticastPkts);
    virtual unsigned int Get_ifHCOutBroadcastPkts (BcmCounter64 &ifHCOutBroadcastPkts);
    virtual int Get_ifLinkUpDownTrapEnable ();
    virtual unsigned int Get_ifHighSpeed ();
    virtual bool Get_ifPromiscuousMode ();
    virtual bool Get_ifConnectorPresent ();
    virtual int Get_ifAlias (BcmString &ifAlias);
    virtual time_t Get_ifCounterDiscontinuityTime ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    virtual SNMP_STATUS Set_ifLinkUpDownTrapEnable (int ifLinkUpDownTrapEnable);
    virtual SNMP_STATUS Set_ifPromiscuousMode (bool ifPromiscuousMode);
    virtual SNMP_STATUS Set_ifAlias (const BcmString &ifAlias);
    /* Method needed to set Interface Counter */
    SNMP_STATUS Set_ifInterfaceCounter (BcmInterfaceCounters * pinterfaceCounters);
  
  protected:
    BcmInterfaceCounters *fpInterfaceCounters;
    ifEntryBridge *fpIfEntry;
    bool fLinkUpDownTrapEnabled;
};



/*\
 *  ifStackEntry 
\*/

#define CONSTVAL_IFSTACKSTATUS_ACTIVE  1
#define CONSTVAL_IFSTACKSTATUS_NOTINSERVICE  2
#define CONSTVAL_IFSTACKSTATUS_NOTREADY  3
#define CONSTVAL_IFSTACKSTATUS_CREATEANDWAIT  5
#define CONSTVAL_IFSTACKSTATUS_CREATEANDGO  4
#define CONSTVAL_IFSTACKSTATUS_DESTROY  6

class ifStackEntryBridge : public TabularMibBridge
{
  friend class ifStackEntry;

  public:
    ifStackEntryBridge (int ifStackHigherLayer, int ifStackLowerLayer, BcmSnmpAgent *pAgent);
   ~ifStackEntryBridge ();

    /* GET methods */

    int Get_ifStackHigherLayer ();
    int Get_ifStackLowerLayer ();
    int Get_ifStackStatus ();
    
#if 0 // Write access not required or supported

    /* TEST methods */

    SNMP_STATUS Test_ifStackStatus (int ifStackStatus);

    /* SET methods */

    SNMP_STATUS Set_ifStackStatus (int &ifStackStatus);

#endif

  protected:
    int fRowStatus;
    int fLo, fHi;

    enum
    {
      kBIT_ifStackHigherLayer = 1 << (ifStackEntry::kLEAF_ifStackHigherLayer - 1),
      kBIT_ifStackLowerLayer = 1 << (ifStackEntry::kLEAF_ifStackLowerLayer - 1),
      kBIT_ifStackStatus = 1 << (ifStackEntry::kLEAF_ifStackStatus - 1),
    };
};


#if 0 // NOT REQUIRED OR SUPPORTED

/*\
 *  ifRcvAddressEntry 
\*/

#define CONSTVAL_IFRCVADDRESSSTATUS_ACTIVE  1
#define CONSTVAL_IFRCVADDRESSSTATUS_NOTINSERVICE  2
#define CONSTVAL_IFRCVADDRESSSTATUS_NOTREADY  3
#define CONSTVAL_IFRCVADDRESSSTATUS_CREATEANDWAIT  5
#define CONSTVAL_IFRCVADDRESSSTATUS_CREATEANDGO  4
#define CONSTVAL_IFRCVADDRESSSTATUS_DESTROY  6
#define CONSTVAL_IFRCVADDRESSTYPE_OTHER  1
#define CONSTVAL_IFRCVADDRESSTYPE_VOLATILE  2
#define CONSTVAL_IFRCVADDRESSTYPE_NONVOLATILE  3

class ifRcvAddressEntryBridge : public TabularMibBridge
{
  friend class ifRcvAddressEntry;

  public:
    ifRcvAddressEntryBridge (int ifIndex, const BcmMacAddress & ifRcvAddressAddress, BcmSnmpAgent *pAgent);
    ifRcvAddressEntryBridge (const BcmObjectId &Index, BcmSnmpAgent *pAgent);
   ~ifRcvAddressEntryBridge ();

    /* GET methods */

    void Get_ifRcvAddressAddress (BcmMacAddress &ifRcvAddressAddress);
    int Get_ifRcvAddressStatus ();
    int Get_ifRcvAddressType ();

    /* TEST methods */

    SNMP_STATUS Test_ifRcvAddressStatus (int ifRcvAddressStatus);

    /* SET methods */

    SNMP_STATUS Set_ifRcvAddressStatus (int &ifRcvAddressStatus);
    SNMP_STATUS Set_ifRcvAddressType (int ifRcvAddressType);


  protected:
    int fRowStatus;

    enum
    {
      kBIT_ifRcvAddressAddress = 1 << (ifRcvAddressEntry::kLEAF_ifRcvAddressAddress - 1),
      kBIT_ifRcvAddressStatus = 1 << (ifRcvAddressEntry::kLEAF_ifRcvAddressStatus - 1),
      kBIT_ifRcvAddressType = 1 << (ifRcvAddressEntry::kLEAF_ifRcvAddressType - 1),
    };

};



/*\
 *  ifTestEntry 
\*/

#define CONSTVAL_IFTESTSTATUS_NOTINUSE  1
#define CONSTVAL_IFTESTSTATUS_INUSE  2
#define CONSTVAL_IFTESTRESULT_NONE  1
#define CONSTVAL_IFTESTRESULT_SUCCESS  2
#define CONSTVAL_IFTESTRESULT_INPROGRESS  3
#define CONSTVAL_IFTESTRESULT_NOTSUPPORTED  4
#define CONSTVAL_IFTESTRESULT_UNABLETORUN  5
#define CONSTVAL_IFTESTRESULT_ABORTED  6
#define CONSTVAL_IFTESTRESULT_FAILED  7
#define MINLEN_IFTESTOWNER  0
#define MAXLEN_IFTESTOWNER  255

class ifTestEntryBridge : public TabularMibBridge
{
  friend class ifTestEntry;

  public:
    ifTestEntryBridge unsigned long Index, BcmSnmpAgent *pAgent);
   ~ifTestEntryBridge ();

    /* GET methods */

    /*
    // NOTE: normally, TestAndIncr objects don't need bridge
    //       object methods for get, test, or set; the MIB
    //       object will just return SNMP_NO_OPINION and let the
    //       TestAndIncrMibObject do its thing.  However, this
    //       method is provided here in case you need to provide
    //       non-standard custom behaviour.
    int Get_ifTestId ();
    */
    int Get_ifTestStatus ();
    int Get_ifTestType (BcmObjectId &ifTestType);
    int Get_ifTestResult ();
    int Get_ifTestCode (BcmObjectId &ifTestCode);
    int Get_ifTestOwner (BcmString &ifTestOwner);

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    /*
    // NOTE: normally, TestAndIncr objects don't need bridge
    //       object methods for get, test, or set; the MIB
    //       object will just return SNMP_NO_OPINION and let the
    //       TestAndIncrMibObject do its thing.  However, this
    //       method is provided here in case you need to provide
    //       non-standard custom behaviour.
    SNMP_STATUS Set_ifTestId (int ifTestId);
    */
    SNMP_STATUS Set_ifTestStatus (int ifTestStatus);
    SNMP_STATUS Set_ifTestType (const BcmObjectId &ifTestType);
    SNMP_STATUS Set_ifTestOwner (const BcmString &ifTestOwner);
};

#endif

#endif

#endif
