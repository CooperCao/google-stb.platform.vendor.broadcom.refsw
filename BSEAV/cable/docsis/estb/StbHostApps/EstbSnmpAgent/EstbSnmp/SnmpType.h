//**************************************************************************
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
#if 0 //BCM_STANDALONE_SNMP_AGENT
/* 64 bit counter type for HC objects in ifXTable */
typedef struct _Counter64
{
  unsigned long Hi;
  unsigned long Lo;
} Counter64;

/* Macros for 64 bit counters */
#define C64_SET(N, H, L)    ((N)->Hi = (H), (N)->Lo = (L))
#define C64_LO(N)           (N)->Lo
#define C64_HI(N)           (N)->Hi
#define C64_ZERO(N)         ((N)->Hi = 0, (N)->Lo = 0)
#define C64_INCR(N)         {if (++((N)->Lo) == 0) (N)->Hi++;}
#define C64_ADD32(N, I)     {(N)->Lo += I; if ((N)->Lo < I) (N)->Hi++;}
#define C64_ADD64(N, M)     {(N)->Lo += (M)->Lo; (N)->Hi += (M)->Hi; \
                            if ((N)->Lo < (M)->Lo) (N)->Hi++;}
#define C64_SUB64(X, Y, Z)  {(X)->Lo = (Y)->Lo - (Z)->Lo; \
                            (X)->Hi = (Y)->Hi - (Z)->Hi; \
                            if ((X)->Lo > (Y)->Lo) (X)->Hi--; }
#define C64_COPY(X, Y)      {(X)->Lo = (Y)->Lo; (X)->Hi = (Y)->Hi;}
#define C64_COMPARE(X, Y)   {(((X)->Hi == (Y)->Hi) ? \
                            (((X)->Lo == (Y)->Lo)  ? (0) : (((X)->Lo > (Y)->Lo) ? (1) : (-1))) : \
                            (((X)->Hi > (Y)->Hi) ? (1) : (-1)));}

#else
#include "halmibs.h" // for Counter64 type
#endif

//#include "MessageTypeCodes.h" // for DOCSIS version

#include <stdlib.h>
#include <string.h>

/* Conditional support for specific MIB objects */
/* Note that makefile settings might already define these */

#ifndef BCM_STANDALONE_SNMP_AGENT
typedef unsigned long DOCSIS_MODE;
#endif
typedef unsigned long SNMP_MODE;
typedef unsigned long SNMP_VIEW_STATUS;
typedef unsigned long SNMP_SEC_LEVEL;
typedef unsigned long SNMP_SEC_MODEL;

#ifndef BCM_STANDALONE_SNMP_AGENT

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
#define VBT_NOSUCHOBJECT     (ASN_1_CONTEXT | ASN_1_PRIMITIVE | 0x0)
#define VBT_NOSUCHINSTANCE   (ASN_1_CONTEXT | ASN_1_PRIMITIVE | 0x1)
#define VBT_ENDOFMIBVIEW     (ASN_1_CONTEXT | ASN_1_PRIMITIVE | 0x2)

// Used to indicate the VB has not been assigned.  Typically used in a 'next'
// call to indicate that the 'get' call still needs to be made.
#define VBT_NOTYPE            ASN_1_NO_ID

// Big change, 7/8/2003.  Make pOctets a pointer that is allocated instead
// of a fixed length array so we can handle large strings.  The SNMPv2-SMI
// (RFC-2578) allows octet strings to be as large as 65535 bytes.
#define UNSPECIFIED_OCTETSTRING_SIZE -1
typedef struct _OctetString
{
  int            Size; // How big is the allocated pOctets buffer, -1 if unknown
  unsigned char *pOctets; // Buffer
  int            Length; // How many bytes are used in the buffer.
  int            DontFree; // 0 if buffer should be freed when finished, otherwise don't free
} OctetString;

// Note: MAX_OID_SIZE increased from 128 to 129 to accomodate conversion from the
// old 1.3.6.1.3.83 MIB location for docsDev to the
// new 1.3.6.1.2.1.69 location.  Note that the new location has 1 extra identifier
// compared to the experimental location!  So, we need to bump our max size up
// from 128 to 129 to handle that.
#define MAX_OID_SIZE              129
#define UNSPECIFIED_INDEX_OFFSET  -1
#define UNSPECIFIED_LEAF          0
typedef struct _ObjectId
{
  int            Length;
  unsigned long  pObjects[MAX_OID_SIZE];
  int            IndexOffset; // 0 based offset if known.  Otherwise -1.
  unsigned long  Leaf; // value of pObjects[IndexOffset-1]
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
    unsigned char pIpAddr[4];
    OctetString  String;    
    ObjectId     OID;       
    Counter64    C64;
  } Value;
  
  // Anything you like.
  void *pPrivate[VB_PRIVATES];
  
  // For freeing above if necessary.  Called on each if not NULL.
  void (*pPrivateFree)(void *);
  
  // Private storage not freed, this might be a function pointer,
  // for example.
  void *pMisc;
  
  // For chaining:
  struct _VarBind *pNext;
  
} VarBind;

typedef struct _MibObject
{
  int Index; // Position in the varbind chain
  ObjectId OID;
  VarBind  VB;
  SNMP_STATUS Status;
  
  // For chaining:
  struct _MibObject *pNext;
} MibObject;

// This structure is NOT intended to fully characterize an SNMP packet.
// Mainly, it just has things that we regard as interesting at the
// BFC level.  I reserve the right to add more things to this structure as
// needed.  Also, be aware that fields in this might be NULL if they are not
// deemed interesting for a given application.
typedef struct _SnmpPacket
{
  int SnmpVersion;         // SNMP_V1_QUERY, etc (from SnmpStatus.h)
  int PduType;             // PDU_GET_REQUEST, etc (from SnmpStatus.h)
  const char *pCommunity;  // for V1 / V2c queries only, NULL for V3
  unsigned char pSrcIp[4]; // source IP (of manager)
  unsigned char pDstIp[4]; // destination IP (of agent)
  int ErrorStatus;         // Standard SNMP error codes
  int ErrorIndex;          // Index of errored varbind
  
  MibObject *pMibObjs;     // Chain of MibObjects in this request.
} SnmpPacket;

#define OID_SIZE(U)  (sizeof(U)/sizeof(unsigned long))
#define OID_COMPARE(U,u,L)  (memcmp(u,U,L*sizeof(unsigned long)))

// Some macros to make extracting info from the VarBind a bit easier...
#define VARBIND_GET_TYPE(V) ((V)->Type)
#define VARBIND_SET_TYPE(V,T) ((V)->Type = T)

#define VARBIND_SET_EMPTY(V) ((V)->Type = VBT_EMPTY)
#define VARBIND_GET_TYPE(V) ((V)->Type)

#define VARBIND_GET_INT32(V) ((V)->Value.Number32)
#define VARBIND_SET_INT32(V,I) ((V)->Value.Number32=I, (V)->Type=VBT_NUMBER)

#define VARBIND_GET_UINT32(V) ((V)->Value.Counter32)
#define VARBIND_SET_UINT32(V,C,T) ((V)->Value.Counter32=C, (V)->Type=T)

#define VARBIND_GET_UINT64(V) (&((V)->Value.C64))
#define VARBIND_SET_UINT64(V,H,L) ((V)->Value.C64.Hi=H, (V)->Value.C64.Lo=L, (V)->Type=VBT_COUNTER64)

#define VARBIND_GET_STRING(V) (&((V)->Value.String))
#define VARBIND_SET_STRING(V,S,L) (memcpy((V)->Value.String.pOctets,S,L), (V)->Value.String.Length=L, (V)->Type=VBT_STRING)

#define VARBIND_GET_OID(V) (&((V)->Value.OID))
#define VARBIND_SET_OID(V,O) (memcpy((V)->Value.OID.pObjects,(O)->pObjects,(O)->Length*sizeof(unsigned long)), (V)->Value.OID.Length=(O)->Length, (V)->Type=VBT_OBJECT)

#define VARBIND_GET_IPADDR(V,A) (memcpy(A,(V)->Value.pIpAddr,4))
#define VARBIND_SET_IPADDR(V,A) (memcpy((V)->Value.pIpAddr,A,4), (V)->Type=VBT_IPADDRESS)

#endif
