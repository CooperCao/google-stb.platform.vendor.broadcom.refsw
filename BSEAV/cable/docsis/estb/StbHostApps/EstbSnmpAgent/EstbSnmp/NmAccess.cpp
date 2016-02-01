//**************************************************************************
//
//    Copyright 2000 Broadcom Corporation
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
//    Filename: NmAccess.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 28-Mar-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Objects for network manaagement access control
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include <string.h>

#include "MibObjs.h"
#include "SnmpLog.h"
#include "NmAccess.h"
#include "docsDevNmMib.h"
#include "docsDevNmBridge.h"
#include "ifBridge.h"
#include "SnmpApi.h" // for UNSPECIFIED_IF_INDEX
#include "MergedNonVolDefaults.h"


// PR 7557:  This is a definite requirement for DOCSIS, just define this
// constant here.  Otherwise it's in PktFilterController.h, but that include
// file is not reliably included for all build types as it is a CM-specific
// file.
#ifndef SUPPORT_CMCI_IF_CONVERSION
#define SUPPORT_CMCI_IF_CONVERSION
#endif

NmAccessController::NmAccessController (int Index, BcmSnmpAgent *pAgent)
{
  fIndex = Index;
  fInterfaces = 0;
  
  // NOTE: pure DOCSIS 1.0 support used to be a NV setting, but now it is
  // compiled in.  To avoid platform-specific NV use here, use the macro
  // directly.
  bool Docsis11 = kDefaultValue_Docsis11Support;

  
  // Initialize values to defaults per MIB.
  fControl = CONSTVAL_DOCSDEVNMACCESSCONTROL_READ;
  fStatus = SNMP_STATUS_INVISIBLE;
  fTrapType = CONSTVAL_DOCSDEVNMACCESSTRAPVERSION_DISABLESNMPV2TRAP;
  
  fIpAddr.Set(255,255,255,255);


  if (Docsis11 == true)
    // From section 4.3.4 of SP-OSSIv1_1-I03-001220, default value for
    // IP mask must be 0.0.0.0, not 255.255.255.255 as specified in RFC-2669
    fIpMask.Set(0,0,0,0);
  else
    // 9/28/01: CL failure in pure 1.0 mode for this. In pure 1.0 mode, use
    // 255.255.255.255.  0.0.0.0 otherwise
    fIpMask.Set(255,255,255,255);
  
  fCommunity = "public";
  
  fpAgent = pAgent;
  
  // Initialize interfaces to all.
  {
    int Count = 0;
    ifEntryBridge *pIf;
    
    pIf = pAgent->IfBridge().FirstIfEntry ();
    while (pIf)
    {
#ifndef BCM_STANDALONE_SNMP_AGENT 
      /*\
       *   - from RFC 2669:
       *   
       *  "Note that entries in this table apply only to link-layer
       *   interfaces (e.g., Ethernet and CATV MAC). Upstream and
       *   downstream channel interfaces must not be specified."
       *
       *   My own note: likewise w/ IP stack interface(s).
      \*/
      
      if ((pIf->Get_ifType() != BcmHalIf::kIanaTypeCmUpstream)
      &&  (pIf->Get_ifType() != BcmHalIf::kIanaTypeCmDownstream)
      &&  (pIf->Get_ifType() != BcmHalIf::kIanaTypeIpStack)
      &&  (pIf->Get_ifType() != BcmHalIf::kIanaTypeSoftwareLoopback))
        fInterfaces |= (1 << Count);
#endif
      
      Count++;
      pIf = pAgent->IfBridge().NextIfEntry (pIf->Get_ifIndex());
    }
  }
  
//fpBridge = new DocsDevNmAccessTableBridge (this, pAgent);
//fpBridge->Register (kDeleteMibObject);
}


NmAccessController::~NmAccessController ()
{
  //delete (fpBridge);
}

int NmAccessController::GetIndex () const
{
  return fIndex;
}

int NmAccessController::GetControl () const
{
  return fControl;
}

unsigned long NmAccessController::GetInterfaces () const
{
  return fInterfaces;
}

int NmAccessController::GetStatus () const
{
  return fStatus;
}

int NmAccessController::GetTrapType () const
{
  return fTrapType;
}

const BcmIpAddress &NmAccessController::GetIpAddr () const
{
  return fIpAddr;
}

const BcmIpAddress &NmAccessController::GetIpMask () const
{
  return fIpMask;
}

const BcmString &NmAccessController::GetCommunity () const
{
  return fCommunity;
}

SNMP_STATUS NmAccessController::SetControl (int Control)
{
  // UNFINISHED? Should I test for good values here? Already done in
  // docsDevNmAccessTableBridge class...
  fControl = Control;
  return SNMP_NO_ERROR;
}

SNMP_STATUS NmAccessController::SetInterfaces (unsigned long Interfaces)
{
  // UNFINISHED? Should I test for good values here? Already done in
  // docsDevNmAccessTableBridge class...
  fInterfaces = Interfaces;
  return SNMP_NO_ERROR;
}

SNMP_STATUS NmAccessController::SetStatus (int Status)
{
  fStatus = Status;
  return SNMP_NO_ERROR;
}

SNMP_STATUS NmAccessController::SetTrapType (int TrapType)
{
  fTrapType = TrapType;
    
  return SNMP_NO_ERROR;
}

SNMP_STATUS NmAccessController::SetIpAddr (const BcmIpAddress &Ip)
{
  fIpAddr = Ip;
  return SNMP_NO_ERROR;
}

SNMP_STATUS NmAccessController::SetIpMask (const BcmIpAddress &Mask)
{
  fIpMask = Mask;
  return SNMP_NO_ERROR;
}

SNMP_STATUS NmAccessController::SetCommunity (const BcmString &Community)
{
  fCommunity = Community;
  return SNMP_NO_ERROR;
}

SNMP_STATUS NmAccessController::ValidateRequest (int PduType, int IfNum, const BcmIpAddress &SA, const BcmString &Community) const
{
  SNMP_STATUS Status;
  
  
  SnmpLogNm << "\nNmAccessController::ValidateRequest controller " << fIndex << endl;
  
  if (fStatus != SNMP_STATUS_ACTIVE)
  {
    SnmpLogNm << "  Controller is not active (" << fIndex <<"), skipping..." << endl;
    return SNMP_NO_OPINION;
  }
  
  Status = ValidateReqCommunity (Community);
  if (Status != SNMP_NO_ERROR)
    return Status;
  
  Status = ValidateReqPduType (PduType);
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  Status = ValidateReqAddr (SA);
  if (Status != SNMP_NO_ERROR)
    return Status;

  // If we have a real interface number here, go ahead and apply
  // the interfaces validation.  Otherwise, this'll be handled in the
  // MIB filters section.
  if (IfNum != UNSPECIFIED_IF_INDEX)
  {
    Status = ValidateReqInterfaces (IfNum);
    if (Status != SNMP_NO_ERROR)
      return Status;
  }

  return SNMP_NO_ERROR;
}

SNMP_STATUS NmAccessController::ValidateReqPduType (int PduType) const
{
  SNMP_STATUS Status = SNMP_NO_ERROR;
  
  
  SnmpLogNm << "NmAccessController::ValidateReqPduType" << endl;
  
  if ((fControl == CONSTVAL_DOCSDEVNMACCESSCONTROL_NONE)
  ||  (fControl == CONSTVAL_DOCSDEVNMACCESSCONTROL_TRAPSONLY))
  {
    SnmpLogNm << "  FAILURE due to control being none or traps only" << endl;
    return SNMP_BAD_COMMUNITY_USE;
  }
  
  switch (PduType)
  {
    // Everybody has read permission, unless they're none or traps only, above
    case PDU_GET_REQUEST:
    case PDU_GET_NEXT_REQUEST:
    case PDU_GET_BULK_REQUEST:
      break;
      
    case PDU_SET_REQUEST:
      if ((fControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_RWWITHTRAPS)
      &&  (fControl != CONSTVAL_DOCSDEVNMACCESSCONTROL_READWRITE))
      {
        SnmpLogNm << "  FAILURE due to read-only control" << endl;
        Status = SNMP_BAD_COMMUNITY_USE;
      }
      break;

    default:
      SnmpLogNm << "  unknown PDU type " << PduType << endl;
      break;
  }
  
  SnmpLogNm << "  MATCH, control/PDU type OK (" << fControl << "/" << PduType << ")" << endl;
  
  return Status;
}

SNMP_STATUS NmAccessController::ValidateReqAddr (const BcmIpAddress &PktIp) const
{
  unsigned long pktSA, nmSA, nmMask;
  
  
  SnmpLogNm << "NmAccessController::ValidateReqAddr\n"
            << "  pkt addr = " << PktIp << ", NM addr = " << fIpAddr << ", NM mask = " << fIpMask << endl;

  // special case: 255.255.255.255 matches everything.
  if (fIpAddr == kBroadcastIp)
  {
    SnmpLogNm << "  MATCH due to 255.255.255.255 NM IP" << endl;
    return SNMP_NO_ERROR;
  }

/* 
  No need to check this, it falls out from the 
  if ((pktSA & nmMask) != (nmSA & nmMask))
  logic below.  (Anything & 0) always equals (AnythingElse & 0)
  
  // New special case for DOCSIS 1.1, from section 4.3.4 of 
  // SP-OSSIv1_1-I03-001220: mask of 0.0.0.0 matches everything.
  // RFC-2669 requirement from above is superceded by the OSS spec.
  if (fIpMask == kAllZerosIp)
  {
    SnmpLogNm << "  MATCH due to 0.0.0.0 NM IP mask" << endl;
    return SNMP_NO_ERROR;
  }
*/
  
  // Convert the IP addresses to more easily maskable forms.
  PktIp.Get(pktSA);
  fIpAddr.Get(nmSA);
  fIpMask.Get(nmMask);

  if ((pktSA & nmMask) != (nmSA & nmMask))
  {
    SnmpLogNm << "  FAILURE, (pkt addr & NM mask) != (NM addr & NM mask)" << endl;
    return SNMP_BAD_COMMUNITY_USE;
  }
  
  SnmpLogNm << "  MATCH, (pkt addr & NM mask) = (NM addr & NM mask)" << endl;
  return SNMP_NO_ERROR;
}


#if 0 // Changed approach here to handle CableHome not using CMCI conversion,
      // plus clean up logic a bit.
SNMP_STATUS NmAccessController::ValidateReqInterfaces (int IfNum) const
{
  ifEntryBridge *pIf;
  int Count = 0, Bit = -1;


  SnmpLogNm << "NmAccessController::ValidateReqInterfaces\n"
            << "  pkt If = " << IfNum << ", NM interface mask = " << (void *)fInterfaces << endl;
  
  /*\
   *   - from RFC 2669:
   *   
   *  "Note that entries in this table apply only to link-layer
   *   interfaces (e.g., Ethernet and CATV MAC). Upstream and
   *   downstream channel interfaces must not be specified."
   *
   *   So, if IfNum here is of type CmUpstream or CmDownstream, change
   *   it to be the number of interface of type CmMac.
  \*/
  
  pIf = fpAgent->IfBridge().FindIfEntry (IfNum);
  if (pIf == NULL)
  {
    SnmpLogNm << "  Couldn't find interface " << IfNum << ", AUTH_ERROR" << endl;
    return SNMP_BAD_COMMUNITY_USE;
  }
 
#ifndef BCM_STANDALONE_SNMP_AGENT 
 
  if ((pIf->Get_ifType() == BcmHalIf::kIanaTypeCmUpstream)
  ||  (pIf->Get_ifType() == BcmHalIf::kIanaTypeCmDownstream))
  {
    pIf = fpAgent->IfBridge().FirstIfEntryOfType (BcmHalIf::kIanaTypeCmMac);
    
    if (pIf == NULL)
    {
      SnmpLogNm << "  Couldn't find interface of type CM_MAC, AUTH_ERROR" << endl;
      return SNMP_BAD_COMMUNITY_USE;
    }
    
    IfNum = pIf->Get_ifIndex();
  }

#ifdef SUPPORT_CMCI_IF_CONVERSION  
  else if ((fInterfaces & 0x01)
  &&       (pIf->Get_ifType() != BcmHalIf::kIanaTypeCmMac)
  &&       (pIf->Get_ifType() != BcmHalIf::kIanaTypeIpStack)
  &&       (pIf->Get_ifType() != BcmHalIf::kIanaTypeSoftwareLoopback)
  // PR 7470: eDOCSIS-N-04.0153-2 requires that we only apply filters
  // for ifIndex 1 to interfaces 5-15 and 32+, not to 16-31 which are reserved
  // for eSAFEs.
  &&       ((pIf->Get_ifIndex() < 16) || (pIf->Get_ifIndex() > 31)))
  {
    SnmpLogNm << "  Pkt interface number is a CMCI interface and mask includes 0x01.\n"
                 "  Performing CMCI IF index conversion to IF 1." << endl;
    IfNum = 1;
  }
#endif
#endif

  pIf = fpAgent->IfBridge().FirstIfEntry ();
  while (pIf)
  {
    if (pIf->Get_ifIndex() == IfNum)
    {
      Bit = Count;
      break;
    }
    
    Count++;  
    pIf = fpAgent->IfBridge().NextIfEntry (pIf->Get_ifIndex());
  }
  
  if (Bit == -1)
    return SNMP_BAD_COMMUNITY_USE;
    
  if (fInterfaces & (1 << Bit))
  {
    SnmpLogNm << "  MATCH, (PktIf & NmIf) != 0" << endl;
    return SNMP_NO_ERROR;
  }

  SnmpLogNm << "  MISMATCH, (PktIf & NmIf) = 0" << endl;
  return SNMP_BAD_COMMUNITY_USE;
}

#else

SNMP_STATUS NmAccessController::ValidateReqInterfaces (int IfNum) const
{
  unsigned long Bit;


  SnmpLogNm << "NmAccessController::ValidateReqInterfaces\n"
            << "  pkt If=" << IfNum << ", NM interface mask = " << (void *)fInterfaces << endl;
 
#ifndef BCM_STANDALONE_SNMP_AGENT  
#ifdef SUPPORT_CMCI_IF_CONVERSION  
  // If our fInterfaces mask has bit 0 (0x00000001) set then we must treat all
  // CPE interfaces as matching.  Check for that condition and adjust if
  // necessary.
  if ((fInterfaces & 0x01) 
  &&  (IfNum != 1)
  // PR 7470: eDOCSIS-N-04.0153-2 requires that we only apply filters
  // for ifIndex 1 to interfaces 5-15 and 32+, not to 16-31 which are reserved
  // for eSAFEs.
  &&  ((IfNum < 16) || (IfNum > 31)))
  {
    int DefaultIf = 0;
    
    // We must NOT apply this rule if interface #1 is not a CPE interface; 
    // that is, if #1 is the WAN / default interface as is the case with
    // CableHome. So figure out the interface number of the 'default' interface.  
    // We can get this from the packet switch pointer that our agent has.
    if (fpAgent)
    {
      BcmHalIf *pDefHal;
      BcmPacketSwitch *pSwitch = fpAgent->GetPacketSwitch();
      
      if (pSwitch)
      {
        pDefHal = pSwitch->DefaultHalIf();
        if (pDefHal)
          DefaultIf = pDefHal->IfTableIndex (fpAgent);
      }
    }
    
    // Apply the conversion if we should.  Note that I am assuming here that
    // any interface which is not the default is a CPE.  This would not work
    // right if the interface passed in is US or DS, but we know because of
    // the way the whole NmAccess system works that that will never be the
    // case.  Mainly, because the interface number comes from the BcmHalIf
    // object that received the packet, and there is no BcmHalIf object for
    // US and DS, only MAC.  
    if ((IfNum != DefaultIf) // Packet came in on a CPE interface...
    &&  (DefaultIf != 1))    // ...WAN / default interface is not #1
    {
      SnmpLogNm << "  Controller has IF 1 authorized, query is from CPE, treating as IF 1" << endl;
      IfNum = 1; // pretend the packet came in on IF 1.
    }
    else
      SnmpLogNm << "  Either IfNum == DefaultIf, or DefaultIf = 1.  DefaultIf=" << DefaultIf << endl;
  }
  else
    SnmpLogNm << "  No CMCI conversion." << endl;
#endif
#endif

  // Convert interface number to a bit in the mask.
  Bit = (1 << (IfNum-1));

  if (fInterfaces & Bit)
  {
    SnmpLogNm << "  MATCH, (PktIf & NmIf) != 0" << endl;
    return SNMP_NO_ERROR;
  }

  SnmpLogNm << "  MISMATCH, (PktIf & NmIf) = 0" << endl;
  return SNMP_BAD_COMMUNITY_USE;
}
#endif


SNMP_STATUS NmAccessController::ValidateReqCommunity (const BcmString &Community) const
{
  SnmpLogNm << "NmAccessController::ValidateCommunity\n"
            << "  pkt community = '" << Community << "', NM community = '" << fCommunity << "'" << endl;
    
  if (fCommunity.empty())
  {
    SnmpLogNm << "  MATCH due to empty NM community" << endl;
    return SNMP_NO_ERROR;
  }
    
  if (Community == fCommunity)
  {
    SnmpLogNm << "  MATCH, pkt community = NM community" << endl;
    return SNMP_NO_ERROR;
  }
    
  SnmpLogNm << "  FAILURE, pkt community (" << Community << ") != NM community (" << fCommunity << ")" << endl;
  return SNMP_BAD_COMMUNITY_NAME;  
}
