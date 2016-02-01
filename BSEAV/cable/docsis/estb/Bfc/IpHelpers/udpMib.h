//**************************************************************************
//                                                                          
//    Copyright 2007 Broadcom Corporation                                   
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
//    Filename: udpMib.h
//    Creation Date: Monday, April 16, 2007 at 15:13:40
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.0b
//    from input file udp-mib_rfc4113.mib
//
//**************************************************************************

#ifndef UDPMIB_H
#define UDPMIB_H

#include "MibObjs.h"
#include "MibBridge.h"



/*\
 *  udpGroup group
\*/

// OID constants
#define kOID_udpGroup  "1.3.6.1.2.1.7"
#define kOID_udpInDatagrams  "1.3.6.1.2.1.7.1.0"
#define kOID_udpNoPorts  "1.3.6.1.2.1.7.2.0"
#define kOID_udpInErrors  "1.3.6.1.2.1.7.3.0"
#define kOID_udpOutDatagrams  "1.3.6.1.2.1.7.4.0"
#define kOID_udpHCInDatagrams  "1.3.6.1.2.1.7.8.0"
#define kOID_udpHCOutDatagrams  "1.3.6.1.2.1.7.9.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_udpGroup  "udpGroup"
  #define kNAME_udpInDatagrams  "udpInDatagrams"
  #define kNAME_udpNoPorts  "udpNoPorts"
  #define kNAME_udpInErrors  "udpInErrors"
  #define kNAME_udpOutDatagrams  "udpOutDatagrams"
  #define kNAME_udpHCInDatagrams  "udpHCInDatagrams"
  #define kNAME_udpHCOutDatagrams  "udpHCOutDatagrams"
#else
  #define kNAME_udpGroup  kOID_udpGroup
  #define kNAME_udpInDatagrams  kOID_udpInDatagrams
  #define kNAME_udpNoPorts  kOID_udpNoPorts
  #define kNAME_udpInErrors  kOID_udpInErrors
  #define kNAME_udpOutDatagrams  kOID_udpOutDatagrams
  #define kNAME_udpHCInDatagrams  kOID_udpHCInDatagrams
  #define kNAME_udpHCOutDatagrams  kOID_udpHCOutDatagrams
#endif

class udpTable;
class udpEndpointTable;
class udpGroupBridge;
class udpEndpointEntryBridge;
class udpEntryBridge;

class udpGroup : public BcmMibGroup
{
  friend class udpGroupBridge;

  public:
    udpGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    udpGroupBridge *fpBridge;

    enum
    {
      kLEAF_udpInDatagrams = 1,
      kLEAF_udpNoPorts = 2,
      kLEAF_udpInErrors = 3,
      kLEAF_udpOutDatagrams = 4,
      kLEAF_udpHCInDatagrams = 8,
      kLEAF_udpHCOutDatagrams = 9
    };
    
    udpTable *fpUdpTable;
    udpEndpointTable  *fpUdpEndpointTable;
};


/* udpEndpointTable and related objects */

// OID constants
#define kOID_udpEndpointTable  "1.3.6.1.2.1.7.7"
#define kOID_udpEndpointEntry  "1.3.6.1.2.1.7.7.1"
#define kOID_udpEndpointLocalAddressType  "1.3.6.1.2.1.7.7.1.1"
#define kOID_udpEndpointLocalAddress  "1.3.6.1.2.1.7.7.1.2"
#define kOID_udpEndpointLocalPort  "1.3.6.1.2.1.7.7.1.3"
#define kOID_udpEndpointRemoteAddressType  "1.3.6.1.2.1.7.7.1.4"
#define kOID_udpEndpointRemoteAddress  "1.3.6.1.2.1.7.7.1.5"
#define kOID_udpEndpointRemotePort  "1.3.6.1.2.1.7.7.1.6"
#define kOID_udpEndpointInstance  "1.3.6.1.2.1.7.7.1.7"
#define kOID_udpEndpointProcess  "1.3.6.1.2.1.7.7.1.8"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_udpEndpointTable  "udpEndpointTable"
  #define kNAME_udpEndpointEntry  "udpEndpointEntry"
  #define kNAME_udpEndpointLocalAddressType  "udpEndpointLocalAddressType"
  #define kNAME_udpEndpointLocalAddress  "udpEndpointLocalAddress"
  #define kNAME_udpEndpointLocalPort  "udpEndpointLocalPort"
  #define kNAME_udpEndpointRemoteAddressType  "udpEndpointRemoteAddressType"
  #define kNAME_udpEndpointRemoteAddress  "udpEndpointRemoteAddress"
  #define kNAME_udpEndpointRemotePort  "udpEndpointRemotePort"
  #define kNAME_udpEndpointInstance  "udpEndpointInstance"
  #define kNAME_udpEndpointProcess  "udpEndpointProcess"
#else
  #define kNAME_udpEndpointTable  kOID_udpEndpointTable
  #define kNAME_udpEndpointEntry  kOID_udpEndpointEntry
  #define kNAME_udpEndpointLocalAddressType  kOID_udpEndpointLocalAddressType
  #define kNAME_udpEndpointLocalAddress  kOID_udpEndpointLocalAddress
  #define kNAME_udpEndpointLocalPort  kOID_udpEndpointLocalPort
  #define kNAME_udpEndpointRemoteAddressType  kOID_udpEndpointRemoteAddressType
  #define kNAME_udpEndpointRemoteAddress  kOID_udpEndpointRemoteAddress
  #define kNAME_udpEndpointRemotePort  kOID_udpEndpointRemotePort
  #define kNAME_udpEndpointInstance  kOID_udpEndpointInstance
  #define kNAME_udpEndpointProcess  kOID_udpEndpointProcess
#endif

class udpEndpointTable : public BcmMibTable
{
  friend class udpGroup;

  public:
    udpEndpointTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    
    void Update ();
    
  protected:
    udpGroupBridge *fpUdpBridge;
};

class udpEndpointEntry : public BcmMibEntry
{
  friend class udpEndpointEntryBridge;
  friend class udpEndpointTable;

  public:
    udpEndpointEntry (udpEndpointTable *pTable, udpEndpointEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    udpEndpointEntryBridge *fpBridge;

    enum
    {
      kLEAF_udpEndpointLocalAddressType = 1,
      kLEAF_udpEndpointLocalAddress = 2,
      kLEAF_udpEndpointLocalPort = 3,
      kLEAF_udpEndpointRemoteAddressType = 4,
      kLEAF_udpEndpointRemoteAddress = 5,
      kLEAF_udpEndpointRemotePort = 6,
      kLEAF_udpEndpointInstance = 7,
      kLEAF_udpEndpointProcess = 8
    };
};


/* udpTable and related objects */

// OID constants
#define kOID_udpTable  "1.3.6.1.2.1.7.5"
#define kOID_udpEntry  "1.3.6.1.2.1.7.5.1"
#define kOID_udpLocalAddress  "1.3.6.1.2.1.7.5.1.1"
#define kOID_udpLocalPort  "1.3.6.1.2.1.7.5.1.2"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_udpTable  "udpTable"
  #define kNAME_udpEntry  "udpEntry"
  #define kNAME_udpLocalAddress  "udpLocalAddress"
  #define kNAME_udpLocalPort  "udpLocalPort"
#else
  #define kNAME_udpTable  kOID_udpTable
  #define kNAME_udpEntry  kOID_udpEntry
  #define kNAME_udpLocalAddress  kOID_udpLocalAddress
  #define kNAME_udpLocalPort  kOID_udpLocalPort
#endif

class udpTable : public BcmMibTable
{
  friend class udpGroup;

  public:
    udpTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
    
    void Update ();
    
  protected:
    udpGroupBridge *fpUdpBridge;
};

class udpEntry : public BcmMibEntry
{
  friend class udpEntryBridge;
  friend class udpTable;

  public:
    udpEntry (udpTable *pTable, udpEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    udpEntryBridge *fpBridge;

    enum
    {
      kLEAF_udpLocalAddress = 1,
      kLEAF_udpLocalPort = 2
    };
};



#endif
