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
//    Filename: usmDhBridge.h
//    Creation Date: Monday, May 12, 2003 at 11:24:18
//    Created by Broadcom V2/BFC Mib Compiler BCMIBC.EXE version 1.4.0b
//    from input file rfc2786.mib
//
//**************************************************************************

#ifndef USMDHBRIDGE_H
#define USMDHBRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

#include "usmDhMib.h"

#include "snmpDH.h"


/*\
 *  usmDHPublicObjectsGroup 
\*/


class usmDHPublicObjectsGroupBridge : public ScalarMibBridge
{
  friend class usmDHPublicObjectsGroup;

  public:
    usmDHPublicObjectsGroupBridge (BcmSnmpAgent *pAgent);
   ~usmDHPublicObjectsGroupBridge ();

    /* GET methods */

    int Get_usmDHParameters (BcmString &usmDHParameters);

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    SNMP_STATUS Set_usmDHParameters (const BcmString &usmDHParameters);
};



/*\
 *  usmDHUserKeyEntry 
\*/


class usmDHUserKeyEntryBridge : public TabularMibBridge
{
  friend class usmDHUserKeyEntry;

  public:
    usmDHUserKeyEntryBridge (BcmString &usmUserIndex, SnmpDh *pDH, BcmSnmpAgent *pAgent);
   ~usmDHUserKeyEntryBridge ();

    /* GET methods */

    int Get_usmDHUserAuthKeyChange (BcmString &usmDHUserAuthKeyChange);
    int Get_usmDHUserOwnAuthKeyChange (BcmString &usmDHUserOwnAuthKeyChange);
    int Get_usmDHUserPrivKeyChange (BcmString &usmDHUserPrivKeyChange);
    int Get_usmDHUserOwnPrivKeyChange (BcmString &usmDHUserOwnPrivKeyChange);

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */

    SNMP_STATUS Set_usmDHUserAuthKeyChange (const BcmString &usmDHUserAuthKeyChange);
    SNMP_STATUS Set_usmDHUserOwnAuthKeyChange (const BcmString &usmDHUserOwnAuthKeyChange);
    SNMP_STATUS Set_usmDHUserPrivKeyChange (const BcmString &usmDHUserPrivKeyChange);
    SNMP_STATUS Set_usmDHUserOwnPrivKeyChange (const BcmString &usmDHUserOwnPrivKeyChange);
    
  private:
	SnmpDh *fDhObject;
};



/*\
 *  usmDHKickstartEntry 
\*/

#define MINVAL_USMDHKICKSTARTINDEX  1
#define MAXVAL_USMDHKICKSTARTINDEX  2147483647
#define MINLEN_USMDHKICKSTARTSECURITYNAME  0
#define MAXLEN_USMDHKICKSTARTSECURITYNAME  255

class usmDHKickstartEntryBridge : public TabularMibBridge
{
  friend class usmDHKickstartEntry;

  public:
    usmDHKickstartEntryBridge (int usmDHKickstartIndex, SnmpDh *pDH, BcmSnmpAgent *pAgent);
   ~usmDHKickstartEntryBridge ();

    /* GET methods */

    int Get_usmDHKickstartIndex ();
    int Get_usmDHKickstartMyPublic (BcmString &usmDHKickstartMyPublic);
    int Get_usmDHKickstartMgrPublic (BcmString &usmDHKickstartMgrPublic);
    int Get_usmDHKickstartSecurityName (BcmString &usmDHKickstartSecurityName);

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */
    
  private:
   	SnmpDh *fDhObject;
};



#endif
