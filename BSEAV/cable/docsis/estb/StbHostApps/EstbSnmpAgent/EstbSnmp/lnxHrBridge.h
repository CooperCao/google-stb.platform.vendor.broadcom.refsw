//**************************************************************************
//                                                                          
//    Copyright 2006 Broadcom Corporation                                   
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
//    Filename: lnxHrBridge.h
//    Creation Date: Tuesday, December 19, 2006 at 11:25:07
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0k
//    from input file ocHostHr.mib
//
//**************************************************************************

#ifndef LNXHRBRIDGE_H
#define LNXHRBRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

#include "hrMib.h"
#include "hrBridge.h"


/*\
 *  hrStorageGroup 
\*/

#define MINVAL_HRMEMORYSIZE  0
#define MAXVAL_HRMEMORYSIZE  2147483647

class lnxHrStorageGroupBridge : public hrStorageGroupBridge
{
  friend class hrStorageGroup;

  public:
    lnxHrStorageGroupBridge (BcmSnmpAgent *pAgent);
   ~lnxHrStorageGroupBridge ();

    /* GET methods */

    int Get_hrMemorySize ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

};



/*\
 *  hrStorageEntry 
\*/

#define MINVAL_HRSTORAGEINDEX  1
#define MAXVAL_HRSTORAGEINDEX  2147483647
#define MINLEN_HRSTORAGEDESCR  0
#define MAXLEN_HRSTORAGEDESCR  255
#define MINVAL_HRSTORAGEALLOCATIONUNITS  1
#define MAXVAL_HRSTORAGEALLOCATIONUNITS  2147483647
#define MINVAL_HRSTORAGESIZE  0
#define MAXVAL_HRSTORAGESIZE  2147483647
#define MINVAL_HRSTORAGEUSED  0
#define MAXVAL_HRSTORAGEUSED  2147483647


#define HR_STORAGE_INDEX_RAM      1
#define HR_STORAGE_INDEX_VIRTUAL  2

#define HR_STORAGE_TYPE_OTHER    1
#define HR_STORAGE_TYPE_RAM      2
#define HR_STORAGE_TYPE_VIRTUAL  3
#define HR_STORAGE_TYPE_RAMDISK  8
#define HR_STORAGE_TYPE_FLASH    9

#define kOID_hrStorageTypeOther    "1.3.6.1.2.1.25.2.1.1"
#define kOID_hrStorageTypeRam      "1.3.6.1.2.1.25.2.1.2"
#define kOID_hrStorageTypeVirtual  "1.3.6.1.2.1.25.2.1.3"
#define kOID_hrStorageTypeRamDisk  "1.3.6.1.2.1.25.2.1.4"
#define kOID_hrStorageTypeFlash    "1.3.6.1.2.1.25.2.1.5"


class lnxHrStorageEntryBridge : public hrStorageEntryBridge
{
  friend class hrStorageEntry;

  public:
    lnxHrStorageEntryBridge (int hrStorageIndex, BcmSnmpAgent *pAgent);
   ~lnxHrStorageEntryBridge ();

    /* GET methods */

    int Get_hrStorageIndex ();
    int Get_hrStorageType (BcmObjectId &hrStorageType);
    int Get_hrStorageDescr (BcmString &hrStorageDescr);
    int Get_hrStorageAllocationUnits ();
    int Get_hrStorageSize ();
    int Get_hrStorageUsed ();
    unsigned int Get_hrStorageAllocationFailures ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    SNMP_STATUS Set_hrStorageSize (int hrStorageSize);

  private:
    int fHrStorageIndex;

};



/*\
 *  hrProcessorEntry 
\*/

#define MINVAL_HRPROCESSORLOAD  0
#define MAXVAL_HRPROCESSORLOAD  100

// This is from the Broadcom MIB
#define kOID_bcm97401    "1.3.6.1.4.1.4413.2.1.3.2.97401"

class lnxHrProcessorEntryBridge : public hrProcessorEntryBridge
{
  friend class hrProcessorEntry;

  public:
    // UNFINISHED: index hrDeviceIndex not from this table, verify type = ULONG
    lnxHrProcessorEntryBridge (unsigned long hrDeviceIndex, BcmSnmpAgent *pAgent);
   ~lnxHrProcessorEntryBridge ();

    /* GET methods */

    int Get_hrProcessorFrwID (BcmObjectId &hrProcessorFrwID);
    int Get_hrProcessorLoad ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

  private:
    unsigned long fHrDeviceIndex;

};

/*\
 *  hrDeviceEntry
\*/

class lnxHrDeviceEntryBridge : public hrDeviceEntryBridge
{
  friend class hrDeviceEntry;

  public:
    lnxHrDeviceEntryBridge (int hrDeviceIndex, BcmObjectId hrDeviceType,
      BcmString hrDeviceDescr, BcmObjectId hrDeviceId, BcmSnmpAgent *pAgent);
   ~lnxHrDeviceEntryBridge ();

    /* GET methods */

    int Get_hrDeviceIndex ();
    int Get_hrDeviceType (BcmObjectId &hrDeviceType);
    int Get_hrDeviceDescr (BcmString &hrDeviceDescr);
    int Get_hrDeviceID (BcmObjectId &hrDeviceID);
    int Get_hrDeviceStatus ();
    unsigned int Get_hrDeviceErrors ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

  protected:
    int fIndex;
    BcmObjectId fType;
    BcmString fDescr;
    BcmObjectId fId;
    BcmSnmpAgent *fpAgent;

};



/*\
 *  hrSWRunEntry
\*/

class lnxHrSWRunEntryBridge : public hrSWRunEntryBridge
{
  friend class hrSWRunEntry;

  public:
    lnxHrSWRunEntryBridge (int hrSWRunIndex, BcmSnmpAgent *pAgent);
   ~lnxHrSWRunEntryBridge ();

    /* GET methods */

    int Get_hrSWRunIndex ();
    int Get_hrSWRunName (BcmString &hrSWRunName);
    int Get_hrSWRunID (BcmObjectId &hrSWRunID);
    int Get_hrSWRunPath (BcmString &hrSWRunPath);
    int Get_hrSWRunParameters (BcmString &hrSWRunParameters);
    int Get_hrSWRunType ();
    int Get_hrSWRunStatus ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    SNMP_STATUS Set_hrSWRunStatus (int hrSWRunStatus);

    protected:
      int fIndex;
};

/*\
 *  hrSWRunPerfEntry 
\*/

#define MINVAL_HRSWRUNPERFCPU  0
#define MAXVAL_HRSWRUNPERFCPU  2147483647
#define MINVAL_HRSWRUNPERFMEM  0
#define MAXVAL_HRSWRUNPERFMEM  2147483647

class lnxHrSWRunPerfEntryBridge : public hrSWRunPerfEntryBridge
{
  friend class hrSWRunPerfEntry;

  public:
    lnxHrSWRunPerfEntryBridge (unsigned long hrSwIndex, BcmSnmpAgent *pAgent);
   ~lnxHrSWRunPerfEntryBridge ();

    /* GET methods */

    int Get_hrSWRunPerfCPU ();
    int Get_hrSWRunPerfMem ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

  private:
    unsigned long fLinuxPID;

};



#endif
