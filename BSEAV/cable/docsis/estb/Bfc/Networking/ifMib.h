//**************************************************************************
//                                                                          
//    Copyright 2004 Broadcom Corporation                                   
//    All Rights Reserved                                                   
//    No portions of this material may be reproduced in any form without the
//    written permission of:                                                
//             Broadcom Corporation                                         
//             16251 Laguna Canyon Road                                     
//             Irvine, California  92618                                    
//    All information contained in this document is Broadcom Corporation    
//    company private, proprietary, and trade secret.                       
//                                                                          
//                                                                          
//                                                                          
//**************************************************************************
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: ifMib.h
//    Creation Date: Thursday, September 16, 2004 at 16:26:56
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0j
//    from input file rfc2863.mib
//
//**************************************************************************

#ifndef IFMIB_H
#define IFMIB_H

#include "MibObjs.h"
#include "MibBridge.h"



/*\
 *  ifMIBGroup group
\*/

// OID constants
#define kOID_ifMIBGroup  "1.3.6.1.2.1.31"
#define kOID_ifMIBObjectsGroup  "1.3.6.1.2.1.31.1"
#define kOID_ifConformanceGroup  "1.3.6.1.2.1.31.2"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ifMIBGroup  "ifMIBGroup"
  #define kNAME_ifMIBObjectsGroup  "ifMIBObjectsGroup"
  #define kNAME_ifConformanceGroup  "ifConformanceGroup"
#else
  #define kNAME_ifMIBGroup  kOID_ifMIBGroup
  #define kNAME_ifMIBObjectsGroup  kOID_ifMIBObjectsGroup
  #define kNAME_ifConformanceGroup  kOID_ifConformanceGroup
#endif

class ifMIBGroup : public BcmMibGroup
{
  public:
    ifMIBGroup ();

    bool Populate ();
};

/*\
 *  interfacesGroup group
\*/

// OID constants
#define kOID_interfacesGroup  "1.3.6.1.2.1.2"
#define kOID_ifNumber  "1.3.6.1.2.1.2.1.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_interfacesGroup  "interfacesGroup"
  #define kNAME_ifNumber  "ifNumber"
#else
  #define kNAME_interfacesGroup  kOID_interfacesGroup
  #define kNAME_ifNumber  kOID_ifNumber
#endif

class interfacesGroupBridge;
class interfacesGroup : public BcmMibGroup
{
  friend class interfacesGroupBridge;

  public:
    interfacesGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    interfacesGroupBridge *fpBridge;

    enum
    {
      kLEAF_ifNumber = 1
    };
};


/* ifTable and related objects */

// OID constants
#define kOID_ifTable  "1.3.6.1.2.1.2.2"
#define kOID_ifEntry  "1.3.6.1.2.1.2.2.1"
#define kOID_ifIndex  "1.3.6.1.2.1.2.2.1.1"
#define kOID_ifDescr  "1.3.6.1.2.1.2.2.1.2"
#define kOID_ifType  "1.3.6.1.2.1.2.2.1.3"
#define kOID_ifMtu  "1.3.6.1.2.1.2.2.1.4"
#define kOID_ifSpeed  "1.3.6.1.2.1.2.2.1.5"
#define kOID_ifPhysAddress  "1.3.6.1.2.1.2.2.1.6"
#define kOID_ifAdminStatus  "1.3.6.1.2.1.2.2.1.7"
#define kOID_ifOperStatus  "1.3.6.1.2.1.2.2.1.8"
#define kOID_ifLastChange  "1.3.6.1.2.1.2.2.1.9"
#define kOID_ifInOctets  "1.3.6.1.2.1.2.2.1.10"
#define kOID_ifInUcastPkts  "1.3.6.1.2.1.2.2.1.11"
#define kOID_ifInNUcastPkts  "1.3.6.1.2.1.2.2.1.12"
#define kOID_ifInDiscards  "1.3.6.1.2.1.2.2.1.13"
#define kOID_ifInErrors  "1.3.6.1.2.1.2.2.1.14"
#define kOID_ifInUnknownProtos  "1.3.6.1.2.1.2.2.1.15"
#define kOID_ifOutOctets  "1.3.6.1.2.1.2.2.1.16"
#define kOID_ifOutUcastPkts  "1.3.6.1.2.1.2.2.1.17"
#define kOID_ifOutNUcastPkts  "1.3.6.1.2.1.2.2.1.18"
#define kOID_ifOutDiscards  "1.3.6.1.2.1.2.2.1.19"
#define kOID_ifOutErrors  "1.3.6.1.2.1.2.2.1.20"
#define kOID_ifOutQLen  "1.3.6.1.2.1.2.2.1.21"
#define kOID_ifSpecific  "1.3.6.1.2.1.2.2.1.22"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ifTable  "ifTable"
  #define kNAME_ifEntry  "ifEntry"
  #define kNAME_ifIndex  "ifIndex"
  #define kNAME_ifDescr  "ifDescr"
  #define kNAME_ifType  "ifType"
  #define kNAME_ifMtu  "ifMtu"
  #define kNAME_ifSpeed  "ifSpeed"
  #define kNAME_ifPhysAddress  "ifPhysAddress"
  #define kNAME_ifAdminStatus  "ifAdminStatus"
  #define kNAME_ifOperStatus  "ifOperStatus"
  #define kNAME_ifLastChange  "ifLastChange"
  #define kNAME_ifInOctets  "ifInOctets"
  #define kNAME_ifInUcastPkts  "ifInUcastPkts"
  #define kNAME_ifInNUcastPkts  "ifInNUcastPkts"
  #define kNAME_ifInDiscards  "ifInDiscards"
  #define kNAME_ifInErrors  "ifInErrors"
  #define kNAME_ifInUnknownProtos  "ifInUnknownProtos"
  #define kNAME_ifOutOctets  "ifOutOctets"
  #define kNAME_ifOutUcastPkts  "ifOutUcastPkts"
  #define kNAME_ifOutNUcastPkts  "ifOutNUcastPkts"
  #define kNAME_ifOutDiscards  "ifOutDiscards"
  #define kNAME_ifOutErrors  "ifOutErrors"
  #define kNAME_ifOutQLen  "ifOutQLen"
  #define kNAME_ifSpecific  "ifSpecific"
#else
  #define kNAME_ifTable  kOID_ifTable
  #define kNAME_ifEntry  kOID_ifEntry
  #define kNAME_ifIndex  kOID_ifIndex
  #define kNAME_ifDescr  kOID_ifDescr
  #define kNAME_ifType  kOID_ifType
  #define kNAME_ifMtu  kOID_ifMtu
  #define kNAME_ifSpeed  kOID_ifSpeed
  #define kNAME_ifPhysAddress  kOID_ifPhysAddress
  #define kNAME_ifAdminStatus  kOID_ifAdminStatus
  #define kNAME_ifOperStatus  kOID_ifOperStatus
  #define kNAME_ifLastChange  kOID_ifLastChange
  #define kNAME_ifInOctets  kOID_ifInOctets
  #define kNAME_ifInUcastPkts  kOID_ifInUcastPkts
  #define kNAME_ifInNUcastPkts  kOID_ifInNUcastPkts
  #define kNAME_ifInDiscards  kOID_ifInDiscards
  #define kNAME_ifInErrors  kOID_ifInErrors
  #define kNAME_ifInUnknownProtos  kOID_ifInUnknownProtos
  #define kNAME_ifOutOctets  kOID_ifOutOctets
  #define kNAME_ifOutUcastPkts  kOID_ifOutUcastPkts
  #define kNAME_ifOutNUcastPkts  kOID_ifOutNUcastPkts
  #define kNAME_ifOutDiscards  kOID_ifOutDiscards
  #define kNAME_ifOutErrors  kOID_ifOutErrors
  #define kNAME_ifOutQLen  kOID_ifOutQLen
  #define kNAME_ifSpecific  kOID_ifSpecific
#endif

class ifTable : public BcmMibTable
{
  public:
    ifTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    
  protected:
    friend class ifEntry;
};

class ifEntry : public BcmMibEntry
{
  friend class ifEntryBridge;
  friend class ifTable;
  friend class interfacesGroupBridge;

  public:
    ifEntry (ifTable *pTable, ifEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    
    virtual ostream &Print(ostream &outStream);

  protected:
    ifEntryBridge *fpBridge;

    enum
    {
      kLEAF_ifIndex = 1,
      kLEAF_ifDescr = 2,
      kLEAF_ifType = 3,
      kLEAF_ifMtu = 4,
      kLEAF_ifSpeed = 5,
      kLEAF_ifPhysAddress = 6,
      kLEAF_ifAdminStatus = 7,
      kLEAF_ifOperStatus = 8,
      kLEAF_ifLastChange = 9,
      kLEAF_ifInOctets = 10,
      kLEAF_ifInUcastPkts = 11,
      kLEAF_ifInNUcastPkts = 12,
      kLEAF_ifInDiscards = 13,
      kLEAF_ifInErrors = 14,
      kLEAF_ifInUnknownProtos = 15,
      kLEAF_ifOutOctets = 16,
      kLEAF_ifOutUcastPkts = 17,
      kLEAF_ifOutNUcastPkts = 18,
      kLEAF_ifOutDiscards = 19,
      kLEAF_ifOutErrors = 20,
      kLEAF_ifOutQLen = 21,
      kLEAF_ifSpecific = 22
    };
};

/*\
 *  ifMIBObjectsGroup group
\*/

// OID constants
#define kOID_ifMIBObjectsGroup  "1.3.6.1.2.1.31.1"
#define kOID_ifTableLastChange  "1.3.6.1.2.1.31.1.5.0"
#define kOID_ifStackLastChange  "1.3.6.1.2.1.31.1.6.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ifMIBObjectsGroup  "ifMIBObjectsGroup"
  #define kNAME_ifTableLastChange  "ifTableLastChange"
  #define kNAME_ifStackLastChange  "ifStackLastChange"
#else
  #define kNAME_ifMIBObjectsGroup  kOID_ifMIBObjectsGroup
  #define kNAME_ifTableLastChange  kOID_ifTableLastChange
  #define kNAME_ifStackLastChange  kOID_ifStackLastChange
#endif

class ifMIBObjectsGroupBridge;
class ifMIBObjectsGroup : public BcmMibGroup
{
  friend class ifMIBObjectsGroupBridge;

  public:
    ifMIBObjectsGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ifMIBObjectsGroupBridge *fpBridge;

    enum
    {
      kLEAF_ifTableLastChange = 5,
      kLEAF_ifStackLastChange = 6
    };
};


/* ifXTable and related objects */

// OID constants
#define kOID_ifXTable  "1.3.6.1.2.1.31.1.1"
#define kOID_ifXEntry  "1.3.6.1.2.1.31.1.1.1"
#define kOID_ifName  "1.3.6.1.2.1.31.1.1.1.1"
#define kOID_ifInMulticastPkts  "1.3.6.1.2.1.31.1.1.1.2"
#define kOID_ifInBroadcastPkts  "1.3.6.1.2.1.31.1.1.1.3"
#define kOID_ifOutMulticastPkts  "1.3.6.1.2.1.31.1.1.1.4"
#define kOID_ifOutBroadcastPkts  "1.3.6.1.2.1.31.1.1.1.5"
#define kOID_ifHCInOctets  "1.3.6.1.2.1.31.1.1.1.6"
#define kOID_ifHCInUcastPkts  "1.3.6.1.2.1.31.1.1.1.7"
#define kOID_ifHCInMulticastPkts  "1.3.6.1.2.1.31.1.1.1.8"
#define kOID_ifHCInBroadcastPkts  "1.3.6.1.2.1.31.1.1.1.9"
#define kOID_ifHCOutOctets  "1.3.6.1.2.1.31.1.1.1.10"
#define kOID_ifHCOutUcastPkts  "1.3.6.1.2.1.31.1.1.1.11"
#define kOID_ifHCOutMulticastPkts  "1.3.6.1.2.1.31.1.1.1.12"
#define kOID_ifHCOutBroadcastPkts  "1.3.6.1.2.1.31.1.1.1.13"
#define kOID_ifLinkUpDownTrapEnable  "1.3.6.1.2.1.31.1.1.1.14"
#define kOID_ifHighSpeed  "1.3.6.1.2.1.31.1.1.1.15"
#define kOID_ifPromiscuousMode  "1.3.6.1.2.1.31.1.1.1.16"
#define kOID_ifConnectorPresent  "1.3.6.1.2.1.31.1.1.1.17"
#define kOID_ifAlias  "1.3.6.1.2.1.31.1.1.1.18"
#define kOID_ifCounterDiscontinuityTime  "1.3.6.1.2.1.31.1.1.1.19"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ifXTable  "ifXTable"
  #define kNAME_ifXEntry  "ifXEntry"
  #define kNAME_ifName  "ifName"
  #define kNAME_ifInMulticastPkts  "ifInMulticastPkts"
  #define kNAME_ifInBroadcastPkts  "ifInBroadcastPkts"
  #define kNAME_ifOutMulticastPkts  "ifOutMulticastPkts"
  #define kNAME_ifOutBroadcastPkts  "ifOutBroadcastPkts"
  #define kNAME_ifHCInOctets  "ifHCInOctets"
  #define kNAME_ifHCInUcastPkts  "ifHCInUcastPkts"
  #define kNAME_ifHCInMulticastPkts  "ifHCInMulticastPkts"
  #define kNAME_ifHCInBroadcastPkts  "ifHCInBroadcastPkts"
  #define kNAME_ifHCOutOctets  "ifHCOutOctets"
  #define kNAME_ifHCOutUcastPkts  "ifHCOutUcastPkts"
  #define kNAME_ifHCOutMulticastPkts  "ifHCOutMulticastPkts"
  #define kNAME_ifHCOutBroadcastPkts  "ifHCOutBroadcastPkts"
  #define kNAME_ifLinkUpDownTrapEnable  "ifLinkUpDownTrapEnable"
  #define kNAME_ifHighSpeed  "ifHighSpeed"
  #define kNAME_ifPromiscuousMode  "ifPromiscuousMode"
  #define kNAME_ifConnectorPresent  "ifConnectorPresent"
  #define kNAME_ifAlias  "ifAlias"
  #define kNAME_ifCounterDiscontinuityTime  "ifCounterDiscontinuityTime"
#else
  #define kNAME_ifXTable  kOID_ifXTable
  #define kNAME_ifXEntry  kOID_ifXEntry
  #define kNAME_ifName  kOID_ifName
  #define kNAME_ifInMulticastPkts  kOID_ifInMulticastPkts
  #define kNAME_ifInBroadcastPkts  kOID_ifInBroadcastPkts
  #define kNAME_ifOutMulticastPkts  kOID_ifOutMulticastPkts
  #define kNAME_ifOutBroadcastPkts  kOID_ifOutBroadcastPkts
  #define kNAME_ifHCInOctets  kOID_ifHCInOctets
  #define kNAME_ifHCInUcastPkts  kOID_ifHCInUcastPkts
  #define kNAME_ifHCInMulticastPkts  kOID_ifHCInMulticastPkts
  #define kNAME_ifHCInBroadcastPkts  kOID_ifHCInBroadcastPkts
  #define kNAME_ifHCOutOctets  kOID_ifHCOutOctets
  #define kNAME_ifHCOutUcastPkts  kOID_ifHCOutUcastPkts
  #define kNAME_ifHCOutMulticastPkts  kOID_ifHCOutMulticastPkts
  #define kNAME_ifHCOutBroadcastPkts  kOID_ifHCOutBroadcastPkts
  #define kNAME_ifLinkUpDownTrapEnable  kOID_ifLinkUpDownTrapEnable
  #define kNAME_ifHighSpeed  kOID_ifHighSpeed
  #define kNAME_ifPromiscuousMode  kOID_ifPromiscuousMode
  #define kNAME_ifConnectorPresent  kOID_ifConnectorPresent
  #define kNAME_ifAlias  kOID_ifAlias
  #define kNAME_ifCounterDiscontinuityTime  kOID_ifCounterDiscontinuityTime
#endif

class ifXTable : public BcmMibTable
{
  public:
    ifXTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ifXEntryBridge;
class ifXEntry : public BcmMibEntry
{
  friend class ifXEntryBridge;
  friend class ifXTable;

  public:
    ifXEntry (ifXTable *pTable, ifXEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ifXEntryBridge *fpBridge;

    enum
    {
      kLEAF_ifName = 1,
      kLEAF_ifInMulticastPkts = 2,
      kLEAF_ifInBroadcastPkts = 3,
      kLEAF_ifOutMulticastPkts = 4,
      kLEAF_ifOutBroadcastPkts = 5,
      kLEAF_ifHCInOctets = 6,
      kLEAF_ifHCInUcastPkts = 7,
      kLEAF_ifHCInMulticastPkts = 8,
      kLEAF_ifHCInBroadcastPkts = 9,
      kLEAF_ifHCOutOctets = 10,
      kLEAF_ifHCOutUcastPkts = 11,
      kLEAF_ifHCOutMulticastPkts = 12,
      kLEAF_ifHCOutBroadcastPkts = 13,
      kLEAF_ifLinkUpDownTrapEnable = 14,
      kLEAF_ifHighSpeed = 15,
      kLEAF_ifPromiscuousMode = 16,
      kLEAF_ifConnectorPresent = 17,
      kLEAF_ifAlias = 18,
      kLEAF_ifCounterDiscontinuityTime = 19
    };
};


/* ifStackTable and related objects */

// OID constants
#define kOID_ifStackTable  "1.3.6.1.2.1.31.1.2"
#define kOID_ifStackEntry  "1.3.6.1.2.1.31.1.2.1"
#define kOID_ifStackHigherLayer  "1.3.6.1.2.1.31.1.2.1.1"
#define kOID_ifStackLowerLayer  "1.3.6.1.2.1.31.1.2.1.2"
#define kOID_ifStackStatus  "1.3.6.1.2.1.31.1.2.1.3"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ifStackTable  "ifStackTable"
  #define kNAME_ifStackEntry  "ifStackEntry"
  #define kNAME_ifStackHigherLayer  "ifStackHigherLayer"
  #define kNAME_ifStackLowerLayer  "ifStackLowerLayer"
  #define kNAME_ifStackStatus  "ifStackStatus"
#else
  #define kNAME_ifStackTable  kOID_ifStackTable
  #define kNAME_ifStackEntry  kOID_ifStackEntry
  #define kNAME_ifStackHigherLayer  kOID_ifStackHigherLayer
  #define kNAME_ifStackLowerLayer  kOID_ifStackLowerLayer
  #define kNAME_ifStackStatus  kOID_ifStackStatus
#endif

class ifStackTable : public BcmMibTable
{
  public:
    ifStackTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ifStackEntryBridge;
class ifStackEntry : public BcmMibEntry
{
  friend class ifStackEntryBridge;
  friend class ifStackTable;

  public:
    ifStackEntry (ifStackTable *pTable, ifStackEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ifStackEntryBridge *fpBridge;

    enum
    {
      kLEAF_ifStackHigherLayer = 1,
      kLEAF_ifStackLowerLayer = 2,
      kLEAF_ifStackStatus = 3
    };
};

#if 0 // NOT REQUIRED OR SUPPORTED

/* ifRcvAddressTable and related objects */

// OID constants
#define kOID_ifRcvAddressTable  "1.3.6.1.2.1.31.1.4"
#define kOID_ifRcvAddressEntry  "1.3.6.1.2.1.31.1.4.1"
#define kOID_ifRcvAddressAddress  "1.3.6.1.2.1.31.1.4.1.1"
#define kOID_ifRcvAddressStatus  "1.3.6.1.2.1.31.1.4.1.2"
#define kOID_ifRcvAddressType  "1.3.6.1.2.1.31.1.4.1.3"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ifRcvAddressTable  "ifRcvAddressTable"
  #define kNAME_ifRcvAddressEntry  "ifRcvAddressEntry"
  #define kNAME_ifRcvAddressAddress  "ifRcvAddressAddress"
  #define kNAME_ifRcvAddressStatus  "ifRcvAddressStatus"
  #define kNAME_ifRcvAddressType  "ifRcvAddressType"
#else
  #define kNAME_ifRcvAddressTable  kOID_ifRcvAddressTable
  #define kNAME_ifRcvAddressEntry  kOID_ifRcvAddressEntry
  #define kNAME_ifRcvAddressAddress  kOID_ifRcvAddressAddress
  #define kNAME_ifRcvAddressStatus  kOID_ifRcvAddressStatus
  #define kNAME_ifRcvAddressType  kOID_ifRcvAddressType
#endif

class ifRcvAddressTable : public BcmMibTable
{
  public:
    ifRcvAddressTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    BcmMibEntry *NewDefaultEntry (BcmObjectId &Index);
};

class ifRcvAddressEntry : public BcmMibEntry
{
  friend class ifRcvAddressEntryBridge;
  friend class ifRcvAddressTable;

  public:
    ifRcvAddressEntry (ifRcvAddressTable *pTable, ifRcvAddressEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS TestLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ifRcvAddressEntryBridge *fpBridge;

    enum
    {
      kLEAF_ifRcvAddressAddress = 1,
      kLEAF_ifRcvAddressStatus = 2,
      kLEAF_ifRcvAddressType = 3
    };
};


/* ifTestTable and related objects */

// OID constants
#define kOID_ifTestTable  "1.3.6.1.2.1.31.1.3"
#define kOID_ifTestEntry  "1.3.6.1.2.1.31.1.3.1"
#define kOID_ifTestId  "1.3.6.1.2.1.31.1.3.1.1"
#define kOID_ifTestStatus  "1.3.6.1.2.1.31.1.3.1.2"
#define kOID_ifTestType  "1.3.6.1.2.1.31.1.3.1.3"
#define kOID_ifTestResult  "1.3.6.1.2.1.31.1.3.1.4"
#define kOID_ifTestCode  "1.3.6.1.2.1.31.1.3.1.5"
#define kOID_ifTestOwner  "1.3.6.1.2.1.31.1.3.1.6"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_ifTestTable  "ifTestTable"
  #define kNAME_ifTestEntry  "ifTestEntry"
  #define kNAME_ifTestId  "ifTestId"
  #define kNAME_ifTestStatus  "ifTestStatus"
  #define kNAME_ifTestType  "ifTestType"
  #define kNAME_ifTestResult  "ifTestResult"
  #define kNAME_ifTestCode  "ifTestCode"
  #define kNAME_ifTestOwner  "ifTestOwner"
#else
  #define kNAME_ifTestTable  kOID_ifTestTable
  #define kNAME_ifTestEntry  kOID_ifTestEntry
  #define kNAME_ifTestId  kOID_ifTestId
  #define kNAME_ifTestStatus  kOID_ifTestStatus
  #define kNAME_ifTestType  kOID_ifTestType
  #define kNAME_ifTestResult  kOID_ifTestResult
  #define kNAME_ifTestCode  kOID_ifTestCode
  #define kNAME_ifTestOwner  kOID_ifTestOwner
#endif

class ifTestTable : public BcmMibTable
{
  public:
    ifTestTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class ifTestEntry : public BcmMibEntry
{
  friend class ifTestEntryBridge;
  friend class ifTestTable;

  public:
    ifTestEntry (ifTestTable *pTable, ifTestEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    ifTestEntryBridge *fpBridge;

    enum
    {
      kLEAF_ifTestId = 1,
      kLEAF_ifTestStatus = 2,
      kLEAF_ifTestType = 3,
      kLEAF_ifTestResult = 4,
      kLEAF_ifTestCode = 5,
      kLEAF_ifTestOwner = 6
    };
};

#endif

#endif
