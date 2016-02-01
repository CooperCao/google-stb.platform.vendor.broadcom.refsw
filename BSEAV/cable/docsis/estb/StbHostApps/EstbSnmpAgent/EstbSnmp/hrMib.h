//**************************************************************************
//                                                                          
//    Copyright 2008 Broadcom Corporation                                   
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
//    Filename: hrMib.h
//    Creation Date: Wednesday, April 09, 2008 at 15:28:58
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0k
//    from input file ocHostHr.mib
//
//**************************************************************************

#ifndef HRMIB_H
#define HRMIB_H

#include "MibObjs.h"
#include "MibBridge.h"

class hrStorageGroupBridge;
class hrStorageEntryBridge;
class hrDeviceEntryBridge;
class hrProcessorEntryBridge;
class hrSWRunGroupBridge;
class hrSWRunEntryBridge;
class hrSWRunPerfEntryBridge;

/*\
 *  hostGroup group
\*/

// OID constants
#define kOID_hostGroup  "1.3.6.1.2.1.25"
#define kOID_hrStorageGroup  "1.3.6.1.2.1.25.2"
#define kOID_hrDeviceGroup  "1.3.6.1.2.1.25.3"
#define kOID_hrSWRunGroup  "1.3.6.1.2.1.25.4"
#define kOID_hrSWRunPerfGroup  "1.3.6.1.2.1.25.5"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_hostGroup  "hostGroup"
  #define kNAME_hrStorageGroup  "hrStorageGroup"
  #define kNAME_hrDeviceGroup  "hrDeviceGroup"
  #define kNAME_hrSWRunGroup  "hrSWRunGroup"
  #define kNAME_hrSWRunPerfGroup  "hrSWRunPerfGroup"
#else
  #define kNAME_hostGroup  kOID_hostGroup
  #define kNAME_hrStorageGroup  kOID_hrStorageGroup
  #define kNAME_hrDeviceGroup  kOID_hrDeviceGroup
  #define kNAME_hrSWRunGroup  kOID_hrSWRunGroup
  #define kNAME_hrSWRunPerfGroup  kOID_hrSWRunPerfGroup
#endif

class hostGroup : public BcmMibGroup
{
  public:
    hostGroup ();

    bool Populate ();
};



/*\
 *  hrStorageGroup group
\*/

// OID constants
#define kOID_hrStorageGroup  "1.3.6.1.2.1.25.2"
#define kOID_hrMemorySize  "1.3.6.1.2.1.25.2.2.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_hrStorageGroup  "hrStorageGroup"
  #define kNAME_hrMemorySize  "hrMemorySize"
#else
  #define kNAME_hrStorageGroup  kOID_hrStorageGroup
  #define kNAME_hrMemorySize  kOID_hrMemorySize
#endif

class hrStorageGroup : public BcmMibGroup
{
  friend class hrStorageGroupBridge;

  public:
    hrStorageGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    hrStorageGroupBridge *fpBridge;

    enum
    {
      kLEAF_hrMemorySize = 2
    };
};


/* hrStorageTable and related objects */

// OID constants
#define kOID_hrStorageTable  "1.3.6.1.2.1.25.2.3"
#define kOID_hrStorageEntry  "1.3.6.1.2.1.25.2.3.1"
#define kOID_hrStorageIndex  "1.3.6.1.2.1.25.2.3.1.1"
#define kOID_hrStorageType  "1.3.6.1.2.1.25.2.3.1.2"
#define kOID_hrStorageDescr  "1.3.6.1.2.1.25.2.3.1.3"
#define kOID_hrStorageAllocationUnits  "1.3.6.1.2.1.25.2.3.1.4"
#define kOID_hrStorageSize  "1.3.6.1.2.1.25.2.3.1.5"
#define kOID_hrStorageUsed  "1.3.6.1.2.1.25.2.3.1.6"
#define kOID_hrStorageAllocationFailures  "1.3.6.1.2.1.25.2.3.1.7"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_hrStorageTable  "hrStorageTable"
  #define kNAME_hrStorageEntry  "hrStorageEntry"
  #define kNAME_hrStorageIndex  "hrStorageIndex"
  #define kNAME_hrStorageType  "hrStorageType"
  #define kNAME_hrStorageDescr  "hrStorageDescr"
  #define kNAME_hrStorageAllocationUnits  "hrStorageAllocationUnits"
  #define kNAME_hrStorageSize  "hrStorageSize"
  #define kNAME_hrStorageUsed  "hrStorageUsed"
  #define kNAME_hrStorageAllocationFailures  "hrStorageAllocationFailures"
#else
  #define kNAME_hrStorageTable  kOID_hrStorageTable
  #define kNAME_hrStorageEntry  kOID_hrStorageEntry
  #define kNAME_hrStorageIndex  kOID_hrStorageIndex
  #define kNAME_hrStorageType  kOID_hrStorageType
  #define kNAME_hrStorageDescr  kOID_hrStorageDescr
  #define kNAME_hrStorageAllocationUnits  kOID_hrStorageAllocationUnits
  #define kNAME_hrStorageSize  kOID_hrStorageSize
  #define kNAME_hrStorageUsed  kOID_hrStorageUsed
  #define kNAME_hrStorageAllocationFailures  kOID_hrStorageAllocationFailures
#endif

class hrStorageTable : public BcmMibTable
{
  public:
    hrStorageTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class hrStorageEntry : public BcmMibEntry
{
  friend class hrStorageEntryBridge;
  friend class hrStorageTable;

  public:
    hrStorageEntry (hrStorageTable *pTable, hrStorageEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    hrStorageEntryBridge *fpBridge;

    enum
    {
      kLEAF_hrStorageIndex = 1,
      kLEAF_hrStorageType = 2,
      kLEAF_hrStorageDescr = 3,
      kLEAF_hrStorageAllocationUnits = 4,
      kLEAF_hrStorageSize = 5,
      kLEAF_hrStorageUsed = 6,
      kLEAF_hrStorageAllocationFailures = 7
    };
};



/*\
 *  hrDeviceGroup group
\*/

// OID constants
#define kOID_hrDeviceGroup  "1.3.6.1.2.1.25.3"
#define kOID_hrDeviceTypesGroup  "1.3.6.1.2.1.25.3.1"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_hrDeviceGroup  "hrDeviceGroup"
  #define kNAME_hrDeviceTypesGroup  "hrDeviceTypesGroup"
#else
  #define kNAME_hrDeviceGroup  kOID_hrDeviceGroup
  #define kNAME_hrDeviceTypesGroup  kOID_hrDeviceTypesGroup
#endif

class hrDeviceGroup : public BcmMibGroup
{
  public:
    hrDeviceGroup ();

    bool Populate ();
};


/* hrDeviceTable and related objects */

// OID constants
#define kOID_hrDeviceTable  "1.3.6.1.2.1.25.3.2"
#define kOID_hrDeviceEntry  "1.3.6.1.2.1.25.3.2.1"
#define kOID_hrDeviceIndex  "1.3.6.1.2.1.25.3.2.1.1"
#define kOID_hrDeviceType  "1.3.6.1.2.1.25.3.2.1.2"
#define kOID_hrDeviceDescr  "1.3.6.1.2.1.25.3.2.1.3"
#define kOID_hrDeviceID  "1.3.6.1.2.1.25.3.2.1.4"
#define kOID_hrDeviceStatus  "1.3.6.1.2.1.25.3.2.1.5"
#define kOID_hrDeviceErrors  "1.3.6.1.2.1.25.3.2.1.6"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_hrDeviceTable  "hrDeviceTable"
  #define kNAME_hrDeviceEntry  "hrDeviceEntry"
  #define kNAME_hrDeviceIndex  "hrDeviceIndex"
  #define kNAME_hrDeviceType  "hrDeviceType"
  #define kNAME_hrDeviceDescr  "hrDeviceDescr"
  #define kNAME_hrDeviceID  "hrDeviceID"
  #define kNAME_hrDeviceStatus  "hrDeviceStatus"
  #define kNAME_hrDeviceErrors  "hrDeviceErrors"
#else
  #define kNAME_hrDeviceTable  kOID_hrDeviceTable
  #define kNAME_hrDeviceEntry  kOID_hrDeviceEntry
  #define kNAME_hrDeviceIndex  kOID_hrDeviceIndex
  #define kNAME_hrDeviceType  kOID_hrDeviceType
  #define kNAME_hrDeviceDescr  kOID_hrDeviceDescr
  #define kNAME_hrDeviceID  kOID_hrDeviceID
  #define kNAME_hrDeviceStatus  kOID_hrDeviceStatus
  #define kNAME_hrDeviceErrors  kOID_hrDeviceErrors
#endif

class hrDeviceTable : public BcmMibTable
{
  public:
    hrDeviceTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class hrDeviceEntry : public BcmMibEntry
{
  friend class hrDeviceEntryBridge;
  friend class hrDeviceTable;

  public:
    hrDeviceEntry (hrDeviceTable *pTable, hrDeviceEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    hrDeviceEntryBridge *fpBridge;

    enum
    {
      kLEAF_hrDeviceIndex = 1,
      kLEAF_hrDeviceType = 2,
      kLEAF_hrDeviceDescr = 3,
      kLEAF_hrDeviceID = 4,
      kLEAF_hrDeviceStatus = 5,
      kLEAF_hrDeviceErrors = 6
    };
};


/* hrProcessorTable and related objects */

// OID constants
#define kOID_hrProcessorTable  "1.3.6.1.2.1.25.3.3"
#define kOID_hrProcessorEntry  "1.3.6.1.2.1.25.3.3.1"
#define kOID_hrProcessorFrwID  "1.3.6.1.2.1.25.3.3.1.1"
#define kOID_hrProcessorLoad  "1.3.6.1.2.1.25.3.3.1.2"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_hrProcessorTable  "hrProcessorTable"
  #define kNAME_hrProcessorEntry  "hrProcessorEntry"
  #define kNAME_hrProcessorFrwID  "hrProcessorFrwID"
  #define kNAME_hrProcessorLoad  "hrProcessorLoad"
#else
  #define kNAME_hrProcessorTable  kOID_hrProcessorTable
  #define kNAME_hrProcessorEntry  kOID_hrProcessorEntry
  #define kNAME_hrProcessorFrwID  kOID_hrProcessorFrwID
  #define kNAME_hrProcessorLoad  kOID_hrProcessorLoad
#endif

class hrProcessorTable : public BcmMibTable
{
  public:
    hrProcessorTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class hrProcessorEntry : public BcmMibEntry
{
  friend class hrProcessorEntryBridge;
  friend class hrProcessorTable;

  public:
    hrProcessorEntry (hrProcessorTable *pTable, hrProcessorEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    hrProcessorEntryBridge *fpBridge;

    enum
    {
      kLEAF_hrProcessorFrwID = 1,
      kLEAF_hrProcessorLoad = 2
    };
};



/*\
 *  hrSWRunGroup group
\*/

// OID constants
#define kOID_hrSWRunGroup  "1.3.6.1.2.1.25.4"
#define kOID_hrSWOSIndex  "1.3.6.1.2.1.25.4.1.0"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_hrSWRunGroup  "hrSWRunGroup"
  #define kNAME_hrSWOSIndex  "hrSWOSIndex"
#else
  #define kNAME_hrSWRunGroup  kOID_hrSWRunGroup
  #define kNAME_hrSWOSIndex  kOID_hrSWOSIndex
#endif

class hrSWRunGroup : public BcmMibGroup
{
  friend class hrSWRunGroupBridge;

  public:
    hrSWRunGroup ();

    bool Populate ();
    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    hrSWRunGroupBridge *fpBridge;

    enum
    {
      kLEAF_hrSWOSIndex = 1
    };
};


/* hrSWRunTable and related objects */

// OID constants
#define kOID_hrSWRunTable  "1.3.6.1.2.1.25.4.2"
#define kOID_hrSWRunEntry  "1.3.6.1.2.1.25.4.2.1"
#define kOID_hrSWRunIndex  "1.3.6.1.2.1.25.4.2.1.1"
#define kOID_hrSWRunName  "1.3.6.1.2.1.25.4.2.1.2"
#define kOID_hrSWRunID  "1.3.6.1.2.1.25.4.2.1.3"
#define kOID_hrSWRunPath  "1.3.6.1.2.1.25.4.2.1.4"
#define kOID_hrSWRunParameters  "1.3.6.1.2.1.25.4.2.1.5"
#define kOID_hrSWRunType  "1.3.6.1.2.1.25.4.2.1.6"
#define kOID_hrSWRunStatus  "1.3.6.1.2.1.25.4.2.1.7"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_hrSWRunTable  "hrSWRunTable"
  #define kNAME_hrSWRunEntry  "hrSWRunEntry"
  #define kNAME_hrSWRunIndex  "hrSWRunIndex"
  #define kNAME_hrSWRunName  "hrSWRunName"
  #define kNAME_hrSWRunID  "hrSWRunID"
  #define kNAME_hrSWRunPath  "hrSWRunPath"
  #define kNAME_hrSWRunParameters  "hrSWRunParameters"
  #define kNAME_hrSWRunType  "hrSWRunType"
  #define kNAME_hrSWRunStatus  "hrSWRunStatus"
#else
  #define kNAME_hrSWRunTable  kOID_hrSWRunTable
  #define kNAME_hrSWRunEntry  kOID_hrSWRunEntry
  #define kNAME_hrSWRunIndex  kOID_hrSWRunIndex
  #define kNAME_hrSWRunName  kOID_hrSWRunName
  #define kNAME_hrSWRunID  kOID_hrSWRunID
  #define kNAME_hrSWRunPath  kOID_hrSWRunPath
  #define kNAME_hrSWRunParameters  kOID_hrSWRunParameters
  #define kNAME_hrSWRunType  kOID_hrSWRunType
  #define kNAME_hrSWRunStatus  kOID_hrSWRunStatus
#endif

class hrSWRunTable : public BcmMibTable
{
  public:
    hrSWRunTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class hrSWRunEntry : public BcmMibEntry
{
  friend class hrSWRunEntryBridge;
  friend class hrSWRunTable;

  public:
    hrSWRunEntry (hrSWRunTable *pTable, hrSWRunEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);
    SNMP_STATUS SetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    hrSWRunEntryBridge *fpBridge;

    enum
    {
      kLEAF_hrSWRunIndex = 1,
      kLEAF_hrSWRunName = 2,
      kLEAF_hrSWRunID = 3,
      kLEAF_hrSWRunPath = 4,
      kLEAF_hrSWRunParameters = 5,
      kLEAF_hrSWRunType = 6,
      kLEAF_hrSWRunStatus = 7
    };
};



/*\
 *  hrSWRunPerfGroup group
\*/

// OID constants
#define kOID_hrSWRunPerfGroup  "1.3.6.1.2.1.25.5"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_hrSWRunPerfGroup  "hrSWRunPerfGroup"
#else
  #define kNAME_hrSWRunPerfGroup  kOID_hrSWRunPerfGroup
#endif

class hrSWRunPerfGroup : public BcmMibGroup
{
  public:
    hrSWRunPerfGroup ();

    bool Populate ();
};


/* hrSWRunPerfTable and related objects */

// OID constants
#define kOID_hrSWRunPerfTable  "1.3.6.1.2.1.25.5.1"
#define kOID_hrSWRunPerfEntry  "1.3.6.1.2.1.25.5.1.1"
#define kOID_hrSWRunPerfCPU  "1.3.6.1.2.1.25.5.1.1.1"
#define kOID_hrSWRunPerfMem  "1.3.6.1.2.1.25.5.1.1.2"

// Object name constants (can be compiled out for code size optimization)
#ifndef BFC_SUPRESS_SNMP_OBJECT_NAMES
  #define kNAME_hrSWRunPerfTable  "hrSWRunPerfTable"
  #define kNAME_hrSWRunPerfEntry  "hrSWRunPerfEntry"
  #define kNAME_hrSWRunPerfCPU  "hrSWRunPerfCPU"
  #define kNAME_hrSWRunPerfMem  "hrSWRunPerfMem"
#else
  #define kNAME_hrSWRunPerfTable  kOID_hrSWRunPerfTable
  #define kNAME_hrSWRunPerfEntry  kOID_hrSWRunPerfEntry
  #define kNAME_hrSWRunPerfCPU  kOID_hrSWRunPerfCPU
  #define kNAME_hrSWRunPerfMem  kOID_hrSWRunPerfMem
#endif

class hrSWRunPerfTable : public BcmMibTable
{
  public:
    hrSWRunPerfTable ();

    SNMP_STATUS RegisterBridge (BcmMibBridge *pBridge, bool DeleteUponDestruction);
};

class hrSWRunPerfEntry : public BcmMibEntry
{
  friend class hrSWRunPerfEntryBridge;
  friend class hrSWRunPerfTable;

  public:
    hrSWRunPerfEntry (hrSWRunPerfTable *pTable, hrSWRunPerfEntryBridge *pBridge);

    SNMP_STATUS GetLeafValue (const BcmMibObject &Leaf, BcmVarBind &VB);

  protected:
    hrSWRunPerfEntryBridge *fpBridge;

    enum
    {
      kLEAF_hrSWRunPerfCPU = 1,
      kLEAF_hrSWRunPerfMem = 2
    };
};



#endif
