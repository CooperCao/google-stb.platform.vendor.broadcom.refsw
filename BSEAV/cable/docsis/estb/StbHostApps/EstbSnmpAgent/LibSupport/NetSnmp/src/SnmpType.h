//**************************************************************************
//
//    Copyright (c) 2008 Broadcom Corporation
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
//    Filename: SnmpType.h
//    Author:   Kevin O'Neal
//    Creation Date: 21-Jan-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Portable types for SNMP agent
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef SNMPTYPE_H
#define SNMPTYPE_H

#include "SnmpStatus.h"
#include "halmibs.h" // for Counter64 type
//#include "MessageTypeCodes.h" // for DOCSIS version

#include <stdlib.h>
#include <string.h>

/* Conditional support for specific MIB objects */
/* Note that makefile settings might already define these */

typedef unsigned long DOCSIS_MODE;
typedef unsigned long SNMP_MODE;
typedef unsigned long SNMP_VIEW_STATUS;
typedef unsigned long SNMP_SEC_LEVEL;
typedef unsigned long SNMP_SEC_MODEL;

#ifndef SUPPORT_CM_ONLY_MIBS
#define SUPPORT_CM_ONLY_MIBS        1
#endif

#ifndef SUPPORT_CMTS_ONLY_MIBS
#define SUPPORT_CMTS_ONLY_MIBS      0
#endif

#ifndef SUPPORT_DOCSIS_1_1_MIBS
#define SUPPORT_DOCSIS_1_1_MIBS     1
#endif

#ifndef SUPPORT_PACKETCABLE_MIBS
#define SUPPORT_PACKETCABLE_MIBS    0
#endif

/* SNMP types: */

/* ASN.1 class & form flags */
#define ASN_1_IDCF_MASK         0xE0 /* ID CLASS | ID FORM */
#define ASN_1_IDC_MASK          0xC0 /* ID CLASS */
#define ASN_1_IDF_MASK          0x20 /* ID FORM */
#define ASN_1_FLAG_MASK         0xE0
#define ASN_1_UNIVERSAL         0x00
#define ASN_1_APPLICATION       0x40
#define ASN_1_CONTEXT           0x80
#define ASN_1_PRIVATE           0xC0
#define ASN_1_PRIMITIVE         0x00
#define ASN_1_CONSTRUCTOR       0x20

#define ASN_1_DEFAULT_SCOPE     ASN_1_CONTEXT

/* ASN.1 basic types: (UNIVERSAL scope) */
#define ASN_1_NO_ID             0x00
#define ASN_1_BOOLEAN           0x01
#define ASN_1_INTEGER           0x02
#define ASN_1_BITSTRING         0x03
#define ASN_1_OCTETSTRING       0x04
#define ASN_1_NULL              0x05
#define ASN_1_OBJECTID          0x06
#define ASN_1_SEQUENCE          0x10
#define ASN_1_SET               0x11

/* Variable binding types are ASN.1 basic types w/ class & form flags applied */
#define VBT_NUMBER            ASN_1_INTEGER
#define VBT_STRING            ASN_1_OCTETSTRING
#define VBT_BITS              ASN_1_OCTETSTRING
#define VBT_OBJECT            ASN_1_OBJECTID
#define VBT_EMPTY             ASN_1_NULL
#define VBT_IPADDRESS        (ASN_1_APPLICATION | 0)
#define VBT_COUNTER          (ASN_1_APPLICATION | 1)
#define VBT_GAUGE            (ASN_1_APPLICATION | 2)
#define VBT_UNSIGNED32       (ASN_1_APPLICATION | 2)
#define VBT_TIMETICKS        (ASN_1_APPLICATION | 3)
#define VBT_OPAQUE           (ASN_1_APPLICATION | 4)
#define VBT_COUNTER64        (ASN_1_APPLICATION | 6)
#define VBT_UINTEGER32       (ASN_1_APPLICATION | 7)

// Big change, 7/8/2003.  Make pOctets a pointer that is allocated instead
// of a fixed length array so we can handle large strings.  The SNMPv2-SMI
// (RFC-2578) allows octet strings to be as large as 65535 bytes.
typedef struct _OctetString
{
  int            Length;
  unsigned char *pOctets;
} OctetString;

// Note: MAX_OID_SIZE increased from 128 to 129 to accomodate conversion from the
// old 1.3.6.1.3.83 MIB location for docsDev to the
// new 1.3.6.1.2.1.69 location.  Note that the new location has 1 extra identifier
// compared to the experimental location!  So, we need to bump our max size up
// from 128 to 129 to handle that.
#define MAX_OID_SIZE      129
typedef struct _ObjectId
{
  int            Length;
  unsigned long  pObjects[MAX_OID_SIZE];
} ObjectId;


// Simple variable binding that encapsulates the
// basic ASN.1 variable types.  Macros to convert this
// portable variable binding to OS/Agent specific ones
// must be provided in the OS-specific code.
#define VB_PRIVATES 4
typedef struct _VarBind
{    
  unsigned char Type;     
                    
  union 
  {
    long          Number32;  
    unsigned long Counter32; 
    OctetString  String;    
    ObjectId     OID;       
    Counter64    C64;
  } Value;
  
  // Anything you like.
  void *pPrivate[VB_PRIVATES];
  
  // For chaining:
  struct _VarBind *pNext;
  
} VarBind;

typedef struct _MibObject
{
  int Index;
  ObjectId OID;
  VarBind  VB;
  SNMP_STATUS Status;
  
  // For chaining:
  struct _MibObject *pNext;
} MibObject;


// Some macros to make extracting info from the VarBind a bit easier...
#define VARBIND_SET_EMPTY(V) ((V)->Type = VBT_EMPTY)
#define VARBIND_GET_TYPE(V) ((V)->Type)

#define VARBIND_GET_INT32(V) ((V)->Value.Number32)
#define VARBIND_SET_INT32(V,I) ((V)->Value.Number32=I, (V)->Type=VBT_NUMBER)

#define VARBIND_GET_UINT32(V) ((V)->Value.Counter32)
#define VARBIND_SET_UINT32(V,C,T) ((V)->Value.Counter32=C, (V)->Type=T)

#define VARBIND_GET_UINT64(V) (&((V)->Value.C64))
#define VARBIND_SET_UINT64(V,H,L) ((V)->Value.C64.Hi=H, (V)->Value.C64.Lo=L, (V)->Type=VBT_COUNTER64)

#define VARBIND_GET_STRING(V) (&((V)->Value.String))
#define VARBIND_SET_STRING(V,S,L,T) (memcpy((V)->Value.String.pOctets,S,L), (V)->Value.String.Length=L, (V)->Type=T)

#define VARBIND_GET_OID(V) (&((V)->Value.OID))
#define VARBIND_SET_OID(V,O) (memcpy((V)->Value.OID.pObjects,(O)->pObjects,(O)->Length*sizeof(unsigned long)), (V)->Value.OID.Length=(O)->Length, (V)->Type=VBT_OBJECT)

#define VARBIND_GET_IPADDR(V,A) (memcpy(A,(V)->Value.String.pOctets,4))
#define VARBIND_SET_IPADDR(V,A) (memcpy((V)->Value.String.pOctets,A,4), (V)->Value.String.Length=4, (V)->Type=VBT_IPADDRESS)

#endif
