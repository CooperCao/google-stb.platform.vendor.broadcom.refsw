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
//    Filename: docsDevTranslator.h
//    Creation Date: Friday, February 13, 2004 at 13:13:13
//
//    This module provides the mechanism to translate from the old
//    draft location of the docsDev node (experimental.83) to the new
//    location (transmission.69)
//
//**************************************************************************

#ifndef DOCSDEVTRANSLATOR_H
#define DOCSDEVTRANSLATOR_H

#include "MibObjs.h"


// Derived objects to handle registering old draft MIB handlers with the
// agent core - for the old experimental.83 location instead of the RFC
// location at mib-II.69, as well as finding objects like these by OID.
class docsDevMibGroup : public BcmMibGroup
{
  public:
    docsDevMibGroup (const char *pOID, const char *pName);
   
    SNMP_STATUS AddScalar (BcmMibObject *pObject);
    SNMP_STATUS AddTable (BcmMibTable *pTable);
    
    BcmMibTable *FindTable (const BcmObjectId &OID);
    BcmMibObject *FindObject (const BcmObjectId &OID);
    BcmMibObject *FindObject (const char *pOidStr);
    BcmMibObject *FindOtherInstance (const BcmObjectId &OID, const BcmMibRequest *pReq);
};


class docsDevMibTable : public BcmMibTable
{
  public:
    docsDevMibTable (const char *pOID, const char *pName);
   
    BcmMibObject *FindObject (const BcmObjectId &OID);
    
    SNMP_STATUS ExtractEntryIndex (const BcmObjectId &OID, BcmObjectId &Index);
};


class docsDevMibEntry : public BcmMibEntry
{
  public:
    docsDevMibEntry (BcmMibTable *pTable, const char *pName, const char *pIndex = NULL);
   
    BcmMibObject *FindObject (const BcmObjectId &OID);
};

#endif
