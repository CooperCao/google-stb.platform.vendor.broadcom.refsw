//**************************************************************************
//                                                                          
//    Copyright 2001 Broadcom Corporation                                   
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
//    Filename: systemBridge.h
//    Creation Date: Wednesday, June 18, 2003 at 10:46:34
//    Created by Broadcom V2/BFC/BOSS Mib Compiler BCMIBC.EXE version 1.4.0c
//    from input file rfc3418.mib
//
//**************************************************************************

#ifndef SYSTEMBRIDGE_H
#define SYSTEMBRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

#include "systemMib.h"


/*\
 *  systemGroup 
\*/

#define MINLEN_SYSDESCR  0
#define MAXLEN_SYSDESCR  255
#define MINLEN_SYSCONTACT  0
#define MAXLEN_SYSCONTACT  255
#define MINLEN_SYSNAME  0
#define MAXLEN_SYSNAME  255
#define MINLEN_SYSLOCATION  0
#define MAXLEN_SYSLOCATION  255
#define MINVAL_SYSSERVICES  0
#define MAXVAL_SYSSERVICES  127

class systemGroupBridge : public ScalarMibBridge
{
  friend class systemGroup;

  public:
    systemGroupBridge (BcmSnmpAgent *pAgent);
   ~systemGroupBridge ();

    /* GET methods */

    int Get_sysDescr (BcmString &sysDescr);
    int Get_sysObjectID (BcmObjectId &sysObjectID);
    unsigned long Get_sysUpTime ();
    int Get_sysContact (BcmString &sysContact);
    int Get_sysName (BcmString &sysName);
    int Get_sysLocation (BcmString &sysLocation);
    int Get_sysServices ();
    unsigned long Get_sysORLastChange ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    SNMP_STATUS Set_sysContact (const BcmString &sysContact);
    SNMP_STATUS Set_sysName (const BcmString &sysName);
    SNMP_STATUS Set_sysLocation (const BcmString &sysLocation);
};



/*\
 *  sysOREntry 
\*/

#define MINVAL_SYSORINDEX  1
#define MAXVAL_SYSORINDEX  2147483647
#define MINLEN_SYSORDESCR  0
#define MAXLEN_SYSORDESCR  255

class sysOREntryBridge : public TabularMibBridge
{
  friend class sysOREntry;

  public:
    sysOREntryBridge (int sysORIndex, BcmSnmpAgent *pAgent);
   ~sysOREntryBridge ();

    /* GET methods */

    int Get_sysORIndex ();
    int Get_sysORID (BcmObjectId &sysORID);
    int Get_sysORDescr (BcmString &sysORDescr);
    unsigned long Get_sysORUpTime ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

};


#endif
