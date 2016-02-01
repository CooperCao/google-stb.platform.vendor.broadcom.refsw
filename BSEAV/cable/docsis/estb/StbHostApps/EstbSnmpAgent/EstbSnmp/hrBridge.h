
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
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: hrBridge.h
//    Creation Date: Wednesday, April 09, 2008 at 15:28:58
//    Created by Broadcom BFC / V2 Mib Compiler BCMIBC.EXE version 3.0.0k
//    from input file ocHostHr.mib
//
//**************************************************************************

#ifndef HRBRIDGE_H
#define HRBRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

#include "hrMib.h"


/*\
 *  hrStorageGroup 
\*/

#define MINVAL_HRMEMORYSIZE  0
#define MAXVAL_HRMEMORYSIZE  2147483647

class hrStorageGroupBridge : public ScalarMibBridge
{
  friend class hrStorageGroup;

  public:
    hrStorageGroupBridge (BcmSnmpAgent *pAgent);
   ~hrStorageGroupBridge ();

    /* GET methods */

    virtual int Get_hrMemorySize ();

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

class hrStorageEntryBridge : public TabularMibBridge
{
  friend class hrStorageEntry;

  public:
    hrStorageEntryBridge (int hrStorageIndex, BcmSnmpAgent *pAgent);
   ~hrStorageEntryBridge ();

    /* GET methods */

    virtual int Get_hrStorageIndex ();
    virtual int Get_hrStorageType (BcmObjectId &hrStorageType);
    virtual int Get_hrStorageDescr (BcmString &hrStorageDescr);
    virtual int Get_hrStorageAllocationUnits ();
    virtual int Get_hrStorageSize ();
    virtual int Get_hrStorageUsed ();
    virtual unsigned int Get_hrStorageAllocationFailures ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    SNMP_STATUS Set_hrStorageSize (int hrStorageSize);
};



/*\
 *  hrDeviceEntry 
\*/

#define MINVAL_HRDEVICEINDEX  1
#define MAXVAL_HRDEVICEINDEX  2147483647
#define MINLEN_HRDEVICEDESCR  0
#define MAXLEN_HRDEVICEDESCR  64
#define CONSTVAL_HRDEVICESTATUS_UNKNOWN  1
#define CONSTVAL_HRDEVICESTATUS_RUNNING  2
#define CONSTVAL_HRDEVICESTATUS_WARNING  3
#define CONSTVAL_HRDEVICESTATUS_TESTING  4
#define CONSTVAL_HRDEVICESTATUS_DOWN  5

class hrDeviceEntryBridge : public TabularMibBridge
{
  friend class hrDeviceEntry;

  public:
    hrDeviceEntryBridge (int hrDeviceIndex, BcmSnmpAgent *pAgent);
   ~hrDeviceEntryBridge ();

    /* GET methods */

    virtual int Get_hrDeviceIndex ();
    virtual int Get_hrDeviceType (BcmObjectId &hrDeviceType);
    virtual int Get_hrDeviceDescr (BcmString &hrDeviceDescr);
    virtual int Get_hrDeviceID (BcmObjectId &hrDeviceID);
    virtual int Get_hrDeviceStatus ();
    virtual unsigned int Get_hrDeviceErrors ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

};



/*\
 *  hrProcessorEntry 
\*/

#define MINVAL_HRPROCESSORLOAD  0
#define MAXVAL_HRPROCESSORLOAD  100

class hrProcessorEntryBridge : public TabularMibBridge
{
  friend class hrProcessorEntry;

  public:
    // UNFINISHED: index hrDeviceIndex not from this table, verify type = ULONG
    hrProcessorEntryBridge (unsigned long hrDeviceIndex, BcmSnmpAgent *pAgent);
   ~hrProcessorEntryBridge ();

    /* GET methods */

    virtual int Get_hrProcessorFrwID (BcmObjectId &hrProcessorFrwID);
    virtual int Get_hrProcessorLoad ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

};



/*\
 *  hrSWRunGroup 
\*/

#define MINVAL_HRSWOSINDEX  1
#define MAXVAL_HRSWOSINDEX  2147483647

class hrSWRunGroupBridge : public ScalarMibBridge
{
  friend class hrSWRunGroup;

  public:
    hrSWRunGroupBridge (BcmSnmpAgent *pAgent);
   ~hrSWRunGroupBridge ();

    /* GET methods */

    int Get_hrSWOSIndex ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

};



/*\
 *  hrSWRunEntry 
\*/

#define MINVAL_HRSWRUNINDEX  1
#define MAXVAL_HRSWRUNINDEX  2147483647
#define MINLEN_HRSWRUNNAME  0
#define MAXLEN_HRSWRUNNAME  64
#define MINLEN_HRSWRUNPATH  0
#define MAXLEN_HRSWRUNPATH  128
#define MINLEN_HRSWRUNPARAMETERS  0
#define MAXLEN_HRSWRUNPARAMETERS  128
#define CONSTVAL_HRSWRUNTYPE_UNKNOWN  1
#define CONSTVAL_HRSWRUNTYPE_OPERATINGSYSTEM  2
#define CONSTVAL_HRSWRUNTYPE_DEVICEDRIVER  3
#define CONSTVAL_HRSWRUNTYPE_APPLICATION  4
#define CONSTVAL_HRSWRUNSTATUS_RUNNING  1
#define CONSTVAL_HRSWRUNSTATUS_RUNNABLE  2
#define CONSTVAL_HRSWRUNSTATUS_NOTRUNNABLE  3
#define CONSTVAL_HRSWRUNSTATUS_INVALID  4

class hrSWRunEntryBridge : public TabularMibBridge
{
  friend class hrSWRunEntry;

  public:
    hrSWRunEntryBridge (int hrSWRunIndex, BcmSnmpAgent *pAgent);
   ~hrSWRunEntryBridge ();

    /* GET methods */

    virtual int Get_hrSWRunIndex ();
    virtual int Get_hrSWRunName (BcmString &hrSWRunName);
    virtual int Get_hrSWRunID (BcmObjectId &hrSWRunID);
    virtual int Get_hrSWRunPath (BcmString &hrSWRunPath);
    virtual int Get_hrSWRunParameters (BcmString &hrSWRunParameters);
    virtual int Get_hrSWRunType ();
    virtual int Get_hrSWRunStatus ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    virtual SNMP_STATUS Set_hrSWRunStatus (int hrSWRunStatus);
};



/*\
 *  hrSWRunPerfEntry 
\*/

#define MINVAL_HRSWRUNPERFCPU  0
#define MAXVAL_HRSWRUNPERFCPU  2147483647
#define MINVAL_HRSWRUNPERFMEM  0
#define MAXVAL_HRSWRUNPERFMEM  2147483647

class hrSWRunPerfEntryBridge : public TabularMibBridge
{
  friend class hrSWRunPerfEntry;

  public:
    // UNFINISHED: unknown index, verify type = ULONG
    hrSWRunPerfEntryBridge (unsigned long Index, BcmSnmpAgent *pAgent);
   ~hrSWRunPerfEntryBridge ();

    /* GET methods */

    virtual int Get_hrSWRunPerfCPU ();
    virtual int Get_hrSWRunPerfMem ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

};



#endif
