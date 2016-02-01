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
//    Filename: BcmTrap.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 22-may-2000
//
//**************************************************************************
//    Description:
//
//      Source file for syslog & trap objects, used by event log
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "BcmTrap.h"
#include "MibObjs.h"
#include "SnmpApi.h"
#include "BcmSnmpAgent.h"
#include "SnmpLog.h"
#include "SystemTime.h"


#define MAX_TRAP_VARBINDS   16

bool BcmTrap::fSilent = false;

BcmTrap::BcmTrap (const BcmObjectId &OID, const BcmString &Name, BcmSnmpAgent *pAgent, int TrapType)
{
  fpNext = NULL;

  fName = Name;
  fOID = OID;

  fpAgent = pAgent;
  
  /*\
   *  RFC-2576 explains the proper values for enterprise, generic, and
   *  specific objects for SNMPv1 traps in section 3.2.
   * 
   *  Enterprise: the OID of the trap without the last element.  If the 
   *              next-to-last element is 0, then without the last 2 elements.
   *  Generic:    the trap type for the standard traps from 1907, otherwise
   *              enterpriseSpecific (6)
   *  Specific:   0 for the 1907 traps, otherwise the last element of the trap's
   *              OID.
  \*/
  
  // Enterprise:
  for (int Loop = 0; Loop < fOID.Length()-1; Loop++)
  {
    if (Loop == fOID.Length() - 2)
    {
      if (fOID[Loop] == 0)
        break;
    }
    
    fEnterprise += fOID[Loop];
  }
  
  // Generic:
  fGeneric = TrapType;
  
  // Specific:
  if (fGeneric == TRAP_ID_ENTERPRISE)
    fSpecific = fOID[fOID.Length()-1];
  else
    fSpecific = 0;
}

BcmTrap::~BcmTrap ()
{
  TrapObject *pTrapObj;
  list <TrapObject *>::iterator Pos;
  
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
    pTrapObj = *Pos;
    delete pTrapObj;
    
    Pos++;
  }
  fObjects.clear();
}

SNMP_STATUS BcmTrap::AddObject (const BcmString &Name, const BcmObjectId &Index)
{
  if (fpAgent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  SnmpLogEv << "BcmTrap::AddObject " << Name << "." << Index << endl;
    
  fpAgent->LockMib();
  
  BcmMibObject *pObject = fpAgent->FindObject(Name, Index);
  if (pObject == NULL)
  {
    fpAgent->UnlockMib();
    SnmpLogRaw << "BcmTrap::AddObject failed to find object " << Name << "." << Index << endl;
    return SNMP_NO_SUCH_NAME;
  }
  
  TrapObject *pTrapObj = new TrapObject;
  
  pObject->GetOid(pTrapObj->OID);
  pObject->Get(pTrapObj->Value);
  
  fObjects.push_back (pTrapObj);
    
  fpAgent->UnlockMib();
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmTrap::AddObject (BcmObjectId &OID)
{
  if (fpAgent == NULL)
    return SNMP_RESOURCE_UNAVAILABLE;
    
  fpAgent->LockMib();

  BcmVarBind VB;
  SNMP_STATUS retVal = fpAgent->GetObject(OID, VB, NULL);
  if (retVal == SNMP_NO_ERROR) {
    TrapObject *pTrapObj = new TrapObject;
    pTrapObj->OID = OID;
    pTrapObj->Value = VB;
    fObjects.push_back (pTrapObj);
  }
  // cout << "BcmTrap::TrapAddObject() " << *this << endl;
 
#ifdef NOTDEF 
  BcmMibObject *pObject = fpAgent->FindObject(OID);
  if (pObject == NULL)
  {
    fpAgent->UnlockMib();
    SnmpLogRaw << "BcmTrap::AddObject failed to find object " << OID << endl;
    return SNMP_NO_SUCH_NAME;
  }
  
  TrapObject *pTrapObj = new TrapObject;
  
  pObject->GetOid(pTrapObj->OID);
  pObject->Get(pTrapObj->Value);
#endif
    
  fpAgent->UnlockMib();
  
  return retVal;
}

const TrapObject *BcmTrap ::GetObject (int Index) const
{
  list <TrapObject *>::const_iterator Pos;
  TrapObject *pObj;
  int ObjIx = 0;
  
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
    pObj = *Pos;
    
    if (ObjIx == Index)
      return pObj;
    
    ObjIx++;
    Pos++;
  }
  
  return NULL;
}

ostream &BcmTrap::Print(ostream &outStream) 
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  outStream << "BcmTrap " << fName << " OID = " << fOID << endl;

  outStream << "V1 trap things:"
            << "\n  Enterprise = " << fEnterprise
            << "\n  Generic = " << fGeneric << " Specific = " << fSpecific
            << endl;

  outStream << "VarBinds:" << endl;

  // Spin thru and list varbinds
  list <TrapObject *>::iterator Pos;
  TrapObject *pObj;
  BcmVarBind VB;
  
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
    pObj = *Pos;
    
    outStream << pObj->OID << " = " << pObj->Value << endl;
    
    Pos++;
  }
#endif
  return outStream;
}


SNMP_STATUS BcmTrap::SendV1Trap (const BcmIpAddress &From, const BcmIpAddress &To, const BcmString &Community, int ToPort)
{    
  // These are definitely too big to come off the stack, but is it OK to make it
  // static here or should I have an arry of them that are class members, or
  // should I malloc as many as needed and then free them?  I think static is
  // OK so long as we never try to send multiple traps at once, and I think that
  // could never happen since trapping is serialized by the event log thread.
  static MibObject Obj[MAX_TRAP_VARBINDS]; 
  static ObjectId Enterprise, Oid;

  unsigned char pTo[4], pFrom[4];
  MibObject *pObj = NULL;
  TrapObject *pTrapObj;
  time_t Now;
  int ObjIx = 0;

  if (fSilent == false)
    gLogMessageRaw << "v1Trap: " << Name() << " " << From << "->" << To << ":" << ToPort << ", cmty=\"" << Community << "\"" << endl;
  
  // From IP:
  From.Get(pFrom[0],pFrom[1],pFrom[2],pFrom[3]);
  
  // To IP: Nm's IP
  To.Get(pTo[0],pTo[1],pTo[2],pTo[3]);
  
  // Enterprise: just package as an ObjectId structure
  fEnterprise.GetOID (&Enterprise);
  fOID.GetOID (&Oid);
  
  // Get the timestamp
  Now = fpAgent->sysUpTime();
    
  /* Spin thru object list... */
  list <TrapObject *>::iterator Pos;
  
  //SnmpLogEv << "Loading VB's..." << endl;
  
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
    pTrapObj = *Pos;
    
    //SnmpLogEv << "  VB " << ObjIx << ": " << pTrapObj->OID << " = " << pTrapObj->Value << endl;
    
    memset (&Obj[ObjIx], 0, sizeof(MibObject));
    pTrapObj->OID.GetOID(&Obj[ObjIx].OID);
    pTrapObj->Value.GetVarBind(&Obj[ObjIx].VB);
    
    if (ObjIx == 0)
      pObj = &Obj[0];
    else
      Obj[ObjIx-1].pNext = &Obj[ObjIx];
    
    ObjIx++;
    
    if (ObjIx >= MAX_TRAP_VARBINDS)
    {
      SnmpLogRaw << "WARNING: Trap " << Name() << " has too many varbinds! Truncating." << endl;
      break;
    }
    
    Pos++;
  }
  
  //SnmpLogEv << "All varbinds loaded.  Sending trap..." << endl;
  SnmpLogEv << *this << endl;

  return AgentSendV1Trap ((char *)Community.c_str(), pFrom, pTo, ToPort, &Enterprise, fGeneric, fSpecific, Now, pObj);
}


SNMP_STATUS BcmTrap::SendV2Trap (const BcmIpAddress &From, const BcmIpAddress &To, const BcmString &Community, int ToPort)
{    
  // These are definitely too big to come off the stack, but is it OK to make it
  // static here or should I have an arry of them that are class members, or
  // should I malloc as many as needed and then free them?  I think static is
  // OK so long as we never try to send multiple traps at once, and I think that
  // could never happen since trapping is serialized by the event log thread.
  static MibObject Obj[MAX_TRAP_VARBINDS]; 
  static ObjectId Oid;

  unsigned char pTo[4], pFrom[4];
  MibObject *pObj = NULL;
  TrapObject *pTrapObj;
  time_t Now;
  int ObjIx = 0;

  
  if (fSilent == false)
    gLogMessageRaw << "v2cTrap: " << Name() << " " << From << "->" << To << ":" << ToPort << ", cmty=\"" << Community << "\"" << endl;

  // From IP:
  From.Get(pFrom[0],pFrom[1],pFrom[2],pFrom[3]);
  
  // To IP: Nm's IP
  To.Get(pTo[0],pTo[1],pTo[2],pTo[3]);
  
  fOID.GetOID (&Oid);
  
  // Get the timestamp
  Now = fpAgent->sysUpTime();
    
  /* Spin thru object list... */
  list <TrapObject *>::iterator Pos;
  
  SnmpLogEv << "Loading VB's..." << endl;
  
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
    pTrapObj = *Pos;
    
    SnmpLogEv << "  VB " << ObjIx << ": " << pTrapObj->OID << " = " << pTrapObj->Value << endl;
    
    memset (&Obj[ObjIx], 0, sizeof(MibObject));
    pTrapObj->OID.GetOID(&Obj[ObjIx].OID);
    pTrapObj->Value.GetVarBind(&Obj[ObjIx].VB);
    
    if (ObjIx == 0)
      pObj = &Obj[0];
    else
      Obj[ObjIx-1].pNext = &Obj[ObjIx];
    
    ObjIx++;
    
    if (ObjIx >= MAX_TRAP_VARBINDS)
    {
      SnmpLogRaw << "WARNING: Trap " << Name() << " has too many varbinds! Truncating." << endl;
      break;
    }
    
    Pos++;
  }
  
  SnmpLogEv << "All varbinds loaded.  Sending trap..." << endl;

  return AgentSendV2Trap ((char *)Community.c_str(), pFrom, pTo, ToPort, &Oid, Now, pObj);
}


SNMP_STATUS BcmTrap::SendNotify (const BcmIpAddress &From)
{    
  // These are definitely too big to come off the stack, but is it OK to make it
  // static here or should I have an arry of them that are class members, or
  // should I malloc as many as needed and then free them?  I think static is
  // OK so long as we never try to send multiple traps at once, and I think that
  // could never happen since trapping is serialized by the event log thread.
  static MibObject Obj[MAX_TRAP_VARBINDS]; 
  static ObjectId Oid;

  unsigned char pFrom[4];
  MibObject *pObj = NULL;
  TrapObject *pTrapObj;
  time_t Now;
  int ObjIx = 0;


  if (fSilent == false)
    gLogMessageRaw << "Sending notify " << Name() << " from " << From << endl;
    
  SnmpLogEv << *this << endl;

  // From IP:
  From.Get(pFrom[0],pFrom[1],pFrom[2],pFrom[3]);
  
  fOID.GetOID (&Oid);
  
  // Get the timestamp
  Now = fpAgent->sysUpTime();
    
  /* Spin thru object list... */
  list <TrapObject *>::iterator Pos;
  
  SnmpLogEv << "Loading VB's..." << endl;
  
  Pos = fObjects.begin();
  while (Pos != fObjects.end())
  {
    pTrapObj = *Pos;
    
    SnmpLogEv << "  VB " << ObjIx << ": " << pTrapObj->OID << " = " << pTrapObj->Value << endl;
    
    memset (&Obj[ObjIx], 0, sizeof(MibObject));
    pTrapObj->OID.GetOID(&Obj[ObjIx].OID);
    pTrapObj->Value.GetVarBind(&Obj[ObjIx].VB);
    
    if (ObjIx == 0)
      pObj = &Obj[0];
    else
      Obj[ObjIx-1].pNext = &Obj[ObjIx];
    
    ObjIx++;
    
    if (ObjIx >= MAX_TRAP_VARBINDS)
    {
      SnmpLogRaw << "WARNING: Trap " << Name() << " has too many varbinds! Truncating." << endl;
      break;
    }
    
    Pos++;
  }
  
  SnmpLogEv << "All varbinds loaded.  Sending notify..." << endl;
  return AgentSendNotify (pFrom, &Oid, Now, pObj);
}
