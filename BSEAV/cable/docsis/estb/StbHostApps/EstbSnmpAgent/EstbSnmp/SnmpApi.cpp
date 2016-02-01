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
//    Filename: SnmpApi.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 21-Jan-2000
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    Basic classes for dealing with SNMP objects and concepts
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "SnmpApi.h"
#include "BcmSnmpAgent.h"
#include "SnmpLog.h"
#include "ifBridge.h"
#include "SnmpThread.h"
#include "BcmSocket.h"
#include "OperatingSystemFactory.h"

#if (SNMP_SUPPORT == 3)
#include "communityMib.h"
#include "communityBridge.h"
#endif


// API's called by the non-portable agent:

/*\
 *  Generic API's for getting/setting/nexting/testing all SNMP objects:
\*/

// Utility function to convert an SNMP_STATUS value to a string for display
void AppSnmpStatusAsString (SNMP_STATUS Status, char *pString, int MaxLen)
{
    BcmString String;

    switch (Status)
    {
    case SNMP_NO_ERROR:             // 0
        String = "NO_ERROR";
        break;

    case SNMP_TOO_BIG:              // 1
        String = "TOO_BIG";
        break;

    case SNMP_NO_SUCH_NAME:         // 2
        String = "NO_SUCH_NAME";
        break;

    case SNMP_BAD_VALUE:            // 3
        String = "BAD_VALUE";
        break;

    case SNMP_READ_ONLY:            // 4
        String = "READ_ONLY";
        break;

    case SNMP_GEN_ERR:              // 5
        String = "GEN_ERR";
        break;

    case SNMP_NO_ACCESS:            // 6
        String = "NO_ACCESS";
        break;

    case SNMP_WRONG_TYPE:           // 7
        String = "WRONG_TYPE";
        break;

    case SNMP_WRONG_LENGTH:         // 8
        String = "WRONG_LENGTH";
        break;

    case SNMP_WRONG_ENCODING:       // 9
        String = "WRONG_ENCODING";
        break;

    case SNMP_WRONG_VALUE:          // 10
        String = "WRONG_VALUE";
        break;

    case SNMP_NO_CREATION:          // 11
        String = "NO_CREATION";
        break;

    case SNMP_INCONSISTENT_VALUE:   // 12
        String = "INCONSISTENT_VALUE";
        break;

    case SNMP_RESOURCE_UNAVAILABLE: // 13
        String = "RESOURCE_UNAVAILABLE";
        break;

    case SNMP_COMMIT_FAILED:        // 14
        String = "COMMIT_FAILED";
        break;

    case SNMP_UNDO_FAILED:          // 15
        String = "UNDO_FAILED";
        break;

    case SNMP_AUTHORIZATION_ERROR:  // 16
        String = "AUTHORIZATION_ERROR";
        break;

    case SNMP_NOT_WRITABLE:         // 17
        String = "NOT_WRITABLE";
        break;

    case SNMP_INCONSISTENT_NAME:    // 18
        String = "INCONSISTENT_NAME";
        break;

    case SNMP_LAST_ERROR:           // 19
        String = "LAST_ERROR";
        break;

    default:
        {
            char ErrStr[32];
            sprintf (ErrStr, "Unknown:%d", (int) Status);
            String = ErrStr;
            break;
        }
    }

    memset (pString, 0, MaxLen);
    strncpy (pString, String.c_str(), MaxLen-1);
}

unsigned long AppGetEngineBoots ()
{
#if (SNMP_SUPPORT == 3)
  // Since we really only have one engine, any old BcmSnmpAgent object
  // will suffice.  Just get the first one.
  BcmSnmpV3Agent *pAgent = BcmSnmpThread::Singleton().GetV3Agent (0);
  
  if (pAgent)
    return pAgent->GetEngineBoots ();
#endif
    
  return 1;
}


#ifndef BCM_STANDALONE_SNMP_AGENT 

int SendLinkTrap (int TrapType, BcmHalIf *pHal)
{
  BcmSnmpAgent  *pAgent;
  ifEntryBridge *pBridge;
  int Loop, TrapsSent = 0;

  for (Loop = 0; Loop < BcmSnmpThread::Singleton().NumAgents(); Loop++)
  {
    pAgent = BcmSnmpThread::Singleton().GetAgent(Loop);
    if (pAgent)
    {
      pBridge = pAgent->IfBridge().FindIfEntry (pHal);
      if (pBridge)
        TrapsSent += pAgent->SendStandardTrap (TrapType, pBridge->Get_ifIndex(), true);
    }
  }
  
  return TrapsSent;
}

int SendLinkTrap (int TrapType, BcmHalIf::IanaInterfaceType ifType)
{
  BcmSnmpAgent  *pAgent;
  ifEntryBridge *pBridge;
  int Loop, TrapsSent = 0;

  for (Loop = 0; Loop < BcmSnmpThread::Singleton().NumAgents(); Loop++)
  {
    pAgent = BcmSnmpThread::Singleton().GetAgent(Loop);
    if (pAgent)
    {
      pBridge = pAgent->IfBridge().FirstIfEntryOfType (ifType);
      if (pBridge)
        TrapsSent += pAgent->SendStandardTrap (TrapType, pBridge->Get_ifIndex(), true);
    }
  }
  
  return TrapsSent;
}

#endif // BCM_STANDALONE_SNMP_AGENT 

// Pass in return code, this will make every effort to translate it to the
// correct return code for the query type (SNMPv1 / SNMPv2 / SNMPv3).
// For example, when attempting to set a writable variable to an invalid
// value, SNMPv1 must return BAD_VALUE, while SNMPv2 must return 
// WRONG_VALUE.  Generally, it's better to translate V2 return codes
// to V1, beause there are more V2 return codes than V1 return codes.  But
// we'll attempt to translate either way as best we can.
SNMP_STATUS TranslateReturnValue (SNMP_STATUS Value, int SnmpVersion)
{
    SNMP_STATUS Translated = Value;


    // Don't allow return codes outside the defined range, unless it's one
    // of our special app-specific values.
    if ((Value > SNMP_INCONSISTENT_NAME)
        &&  (Value < SNMP_APP_SPECIFIC_BASE))
        Translated = SNMP_GEN_ERR;

    // Many SNMP agents (including Epilogue) handle V2->V1 translation
    // for us.  In that case, we don't really need to know what kind of
    // query this is and we'll just pass back the same value passed in.
    if (SnmpVersion == SNMP_QUERY)
        return Translated;

    else if (SnmpVersion == SNMP_V1_QUERY)
    {
        // Translate V2 value to V1.  V1 basically only has
        // 3 valid return codes: BAD_VALUE, NO_SUCH_NAME, or NO_ERROR.
        // V2 adds other values that need to be translated.
        switch (Value)
        {
        case SNMP_WRONG_TYPE:
        case SNMP_WRONG_LENGTH:
        case SNMP_WRONG_ENCODING:
        case SNMP_WRONG_VALUE:
            Translated = SNMP_BAD_VALUE;
            break;

        case SNMP_NO_ACCESS:
        case SNMP_NO_CREATION:
        case SNMP_AUTHORIZATION_ERROR:
        case SNMP_NOT_WRITABLE:
        case SNMP_INCONSISTENT_NAME:
            Translated = SNMP_NO_SUCH_NAME;
            break;

        case SNMP_RESOURCE_UNAVAILABLE:
        case SNMP_COMMIT_FAILED:
        case SNMP_UNDO_FAILED:
        case SNMP_LAST_ERROR:
            Translated = SNMP_GEN_ERR;
            break;
        }
    } else if (SnmpVersion == SNMP_V2_QUERY)
    {
        switch (Value)
        {
        case SNMP_BAD_VALUE:
            // Even though BAD_VALUE is a valid SNMPv2 return code,
            // CableLabs will flunk you if you return it instead of 
            // WRONG_VALUE.  So...
            Translated = SNMP_WRONG_VALUE;
            break;
        }
    }

    return Translated;
}

const char *AppGetV1V2View (const unsigned char *AgentIp)
{
  BcmIpAddress IP(AgentIp[0],AgentIp[1],AgentIp[2],AgentIp[3]);
  BcmSnmpAgent *pAgent = BcmSnmpThread::Singleton().FindAgentByIp (IP);
  
  if (pAgent == NULL)
    return NULL;
    
  return (const char *)pAgent->GetV1V2View().c_str();
}

// Called from the non-portable agent at the same time as community
// validation.  Note that this will consult the NmAccess table if we are
// in NmAccess mode, or the snmpCommunityMib if we are in coexistence mode.
SNMP_STATUS AppValidateSnmpRequest (int PduType, int SnmpVersion, const unsigned char *pSA, int SrcPort, const unsigned char *pDA, int DstPort, const char *pCommunity, char *pSecurityName, int MaxLen)
{
    BcmIpAddress SrcAddr(pSA[0],pSA[1],pSA[2],pSA[3]);
    BcmIpAddress DstAddr(pDA[0],pDA[1],pDA[2],pDA[3]);
    BcmSnmpAgent *pAgent = BcmSnmpThread::Singleton().FindAgentByIp (DstAddr);
    SNMP_STATUS Status;
    int SrcIf = 0;
    const char *pVersion;
    const char *pPdu;
    
    switch (SnmpVersion)
    {
      case SNMP_V1_QUERY:
        pVersion = "SNMPv1 ";
        break;
        
      case SNMP_V2_QUERY:
        pVersion = "SNMPv2c ";
        break;
        
      case SNMP_V3_QUERY:
        pVersion = "SNMPv3 ";
        break;
        
      default:
        pVersion = "SNMPv?? ";
        break;
    }
    
    switch (PduType)
    {
      case PDU_GET_REQUEST:
        pPdu = "GET ";
        break;
        
      case PDU_GET_NEXT_REQUEST:
        pPdu = "NEXT ";
        break;
        
      case PDU_GET_BULK_REQUEST:
        pPdu = "GET-BULK ";
        break;
        
      case PDU_SET_REQUEST:
        pPdu = "SET ";
        break;
        
      default:
        pPdu = "UNKNOWN ";
        break;
    }
    
    SnmpLogNm << pVersion << PduType << "request: " << SrcAddr << ":" << SrcPort << "->" << DstAddr << ":" << DstPort << ", Cmty='" << pCommunity << "'" << endl;
    
    if (pSecurityName)
      memset (pSecurityName, 0, MaxLen);
    
    // If this is not a GET or SET request, return NO ERROR.  This is necessary 
    // because if we return an error, the agent will not process the packet 
    // any further which will result in the proper counters (snmpInASNParseErrs
    // or snmpUnknownPDUHandlers) not being incremented.
    //
    // For now, we assume that no agent is going to handle received notifies,
    // or get/set responses, so we can make this check here for convenience.  
    // In the future, if for some weird reason we decide to have an agent that 
    // might handle other PDU types, we would move this check into the derived 
    // BcmSnmpAgent::AuthenticateRequest method.
    if ((PduType != PDU_GET_REQUEST)
    &&  (PduType != PDU_GET_NEXT_REQUEST)
    &&  (PduType != PDU_GET_BULK_REQUEST)
    &&  (PduType != PDU_SET_REQUEST))
    {
      SnmpLogNm << "  Unhandled PDU type (" << PduType << "), NO ERROR so the agent can increment counters." << endl;
      return SNMP_NO_ERROR;
    }

    if (pCommunity == NULL)
    {
      SnmpLogRaw << "ERROR: AppValidateSnmpRequest: pCommunity == NULL" << endl;
      return SNMP_WRONG_VALUE;
    }
    
    BcmString Community(pCommunity);
    BcmString SecurityName;

    if (pAgent == NULL)
    {
      SnmpLogNm << "pAgent = NULL: RESOURCE_UNAVAILABLE" << endl;
      return SNMP_RESOURCE_UNAVAILABLE;
    }
    
    Status = pAgent->AuthenticateRequest (PduType, SnmpVersion, SrcIf, SrcAddr, Community);

#if (SNMP_SUPPORT == 3)    
    // If AuthenticateRequest returned NO_OPINION, then check for
    // snmpCommunityTable information.
    BcmSnmpV3Agent *pV3Agent = BcmSnmpThread::Singleton().FindV3Agent (pAgent);
    
    if ((SnmpVersion != 3) && (Status == SNMP_NO_OPINION) && (pV3Agent->fpCommunityTable))
    {
      Status = pV3Agent->fpCommunityTable->ValidateRequest (Community, SrcAddr, SrcPort, SecurityName);
    
      if ((Status == SNMP_NO_ERROR) && (pSecurityName != NULL))
        strncpy (pSecurityName, SecurityName.c_str(), MaxLen-1);
    }
#endif
    
    // If AuthenticateRequest failed us, then send an authFail trap.
    if ((Status != SNMP_NO_ERROR) 
    &&  (Status != SNMP_NO_OPINION)
    &&  (Status != SNMP_NO_ACCESS)) // we're disabled.  Drop pkt but no trap.
      // Call AppSendTrap here to make sure it's done asynchronously.
      AppSendTrap (pDA, AUTH_FAILURE_TRAP, 0);

    return Status;
}

SNMP_STATUS AppValidateSetRequest (ObjectId *pOID, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, const char *pCommunity, int SnmpVersion)
{
  BcmObjectId OID(pOID);
  BcmVarBind VB(pVB);
  BcmString Community;
  BcmIpAddress SrcAddr(SA[0],SA[1],SA[2],SA[3]), DstAddr(DA[0],DA[1],DA[2],DA[3]);
  BcmSnmpAgent *pAgent = BcmSnmpThread::Singleton().FindAgentByIp (DstAddr);
  
  if (pAgent == NULL)
  {
    SnmpLogRaw << "WARNING - AppValidateSetRequest couldn't find agent for IP " << DstAddr << endl;
    return SNMP_NO_ERROR;
  }
  
  if (pCommunity)
    Community = pCommunity;
  
  return pAgent->ValidateSet (OID, VB, SrcAddr, SnmpVersion, Community);
}

SNMP_STATUS AppGetObject (ObjectId *pOID, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, const char *pCommunity, int SnmpVersion)
{
    static BcmObjectId Index("0");
    BcmObjectId OID(pOID);
    BcmVarBind VB(pVB);
    BcmIpAddress SrcIp(SA[0],SA[1],SA[2],SA[3]), DstIp(DA[0],DA[1],DA[2],DA[3]);
    BcmSnmpAgent *pAgent = BcmSnmpThread::Singleton().FindAgentByIp (DstIp);
    BcmString Community = pCommunity;
    BcmMibRequest Req(BcmMibRequest::kGetRequest, SnmpVersion, 0, OID, Index, SrcIp, DstIp, Community, pAgent);

    // Don't yield the processor...
    // BcmOperatingSystemFactory::ThreadSleep(5);

    if (pAgent == NULL)
    {
        SnmpLogReq << "AppGetObject: pAgent = NULL: RESOURCE_UNAVAILABLE" << endl;
        return TranslateReturnValue (SNMP_RESOURCE_UNAVAILABLE, SnmpVersion);
    }
    
    // Special case: if the source address is 127.0.0.1, treat it like a
    // console or config file set because it's internal
    if (SrcIp == kSoftwareLoopbackIp)
      return TranslateReturnValue (pAgent->GetObject (OID, VB, NULL), SnmpVersion);
    else
      return TranslateReturnValue (pAgent->GetObject (OID, VB, &Req), SnmpVersion);
}


SNMP_STATUS AppNextObject (ObjectId *pOID, ObjectId *pIndex, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, const char *pCommunity, int SnmpVersion)
{
    BcmObjectId OID(pOID);
    BcmObjectId Index(pIndex);
    BcmVarBind VB(pVB);
    BcmIpAddress SrcIp(SA[0],SA[1],SA[2],SA[3]), DstIp(DA[0],DA[1],DA[2],DA[3]);
    BcmSnmpAgent *pAgent = BcmSnmpThread::Singleton().FindAgentByIp (DstIp);
    BcmString Community = pCommunity;
    BcmMibRequest Req(BcmMibRequest::kNextRequest, SnmpVersion, 0, OID, Index, SrcIp, DstIp, Community, pAgent);


    //SnmpLogReq << "NEXT request for OID " << OID << " From " << DstIp << " To " << SrcIp << " cmty=" << Community << endl;
    
    // Don't yield the processor... too slow fow MIB walks
    // BcmOperatingSystemFactory::ThreadSleep(5);

    if (pAgent == NULL)
    {
        SnmpLogReq << "AppNextObject: pAgent = NULL: RESOURCE_UNAVAILABLE" << endl;
        return TranslateReturnValue (SNMP_RESOURCE_UNAVAILABLE, SnmpVersion);
    }

    // Special case: if the source address is 127.0.0.1, treat it like a
    // console or config file set because it's internal
    if (SrcIp == kSoftwareLoopbackIp)
      return TranslateReturnValue (pAgent->NextObject (OID, Index, VB, NULL), SnmpVersion);
    else
      return TranslateReturnValue (pAgent->NextObject (OID, Index, VB, &Req), SnmpVersion);
}


SNMP_STATUS AppTestObject (ObjectId *pOID, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, const char *pCommunity, int SnmpVersion)
{
    static BcmObjectId Index ("0");
    BcmObjectId OID(pOID);
    BcmVarBind VB(pVB);
    BcmIpAddress SrcIp(SA[0],SA[1],SA[2],SA[3]), DstIp(DA[0],DA[1],DA[2],DA[3]);
    BcmSnmpAgent *pAgent = BcmSnmpThread::Singleton().FindAgentByIp (DstIp);
    BcmString Community = pCommunity;
    BcmMibRequest Req(BcmMibRequest::kTestRequest, SnmpVersion, 0, OID, Index, SrcIp, DstIp, Community, pAgent);
    SNMP_STATUS Status;


    //SnmpLogReq << "TEST request for OID " << OID << " From " << DstIp << " To " << SrcIp << " cmty=" << Community << endl;
    
    // Yield the processor... 
    //  PR 8299 - For some reason, Linux won't really yield if this is < 5ms
    BcmOperatingSystemFactory::ThreadSleep(5);

    if (pAgent == NULL)
    {
        SnmpLogReq << "AppTestObject: pAgent = NULL: RESOURCE_UNAVAILABLE" << endl;
        return TranslateReturnValue (SNMP_RESOURCE_UNAVAILABLE, SnmpVersion);
    }

    // Special case: if the source address is 127.0.0.1, treat it like a
    // console or config file set because it's internal. Indicate this with a
    // NULL value for the BcmMibRequest argument.
    if (SrcIp == kSoftwareLoopbackIp)
      Status = pAgent->TestObject (OID, VB, false, NULL);
    else
      Status = pAgent->TestObject (OID, VB, false, &Req);
      
    // Set requests can't return NO_SUCH_NAME or NO_SUCH_INSTANCE, they must
    // return NOT_WRITABLE instead.  Internally, we may have some use for a
    // NO_SUCH_INSTANCE or NO_SUCH_NAME return from a test or set, so we are
    // keeping those values in the BcmSnmpAgent::TestObject method.  However,
    // for an actual request off the wire, translate here.
    if ((Status == SNMP_NO_SUCH_NAME)
    ||  (Status == SNMP_NO_SUCH_INSTANCE))
      Status = SNMP_NOT_WRITABLE;
      
    return TranslateReturnValue (Status, SnmpVersion);
}


SNMP_STATUS AppSetObject (ObjectId *pOID, VarBind *pVB, const unsigned char *DA, const unsigned char *SA, const char *pCommunity, int SnmpVersion)
{
    static BcmObjectId Index ("0");
    BcmObjectId OID(pOID);
    BcmVarBind VB(pVB);
    BcmIpAddress SrcIp(SA[0],SA[1],SA[2],SA[3]), DstIp(DA[0],DA[1],DA[2],DA[3]);
    BcmSnmpAgent *pAgent = BcmSnmpThread::Singleton().FindAgentByIp (DstIp);
    BcmString Community = pCommunity;
    BcmMibRequest Req(BcmMibRequest::kSetRequest, SnmpVersion, 0, OID, Index, SrcIp, DstIp, Community, pAgent);
    SNMP_STATUS Status;


    //SnmpLogReq << "SET request for OID " << OID << " From " << DstIp << " To " << SrcIp << " cmty=" << Community << endl;
    
    // Yield the processor...
    //  PR 8299 - For some reason, Linux won't really yield if this is < 5ms
    BcmOperatingSystemFactory::ThreadSleep(5);

    if (pAgent == NULL)
    {
        SnmpLogReq << "AppSetObject: pAgent = NULL: RESOURCE_UNAVAILABLE" << endl;
        return TranslateReturnValue (SNMP_RESOURCE_UNAVAILABLE, SnmpVersion);
    }

    // Special case: if the source address is 127.0.0.1, treat it like a
    // console or config file set because it's internal.  Indicate this with a
    // NULL value for the BcmMibRequest argument.
    if (SrcIp == kSoftwareLoopbackIp)
      Status = pAgent->SetObject (OID, VB, false, NULL);
    else
      Status = pAgent->SetObject (OID, VB, false, &Req);
      
    // Set requests can't return NO_SUCH_NAME or NO_SUCH_INSTANCE, they must
    // return NOT_WRITABLE instead.  Internally, we may have some use for a
    // NO_SUCH_INSTANCE or NO_SUCH_NAME return from a test or set, so we are
    // keeping those values in the BcmSnmpAgent::TestObject method.  However,
    // for an actual request off the wire, translate here.
    if ((Status == SNMP_NO_SUCH_NAME)
    ||  (Status == SNMP_NO_SUCH_INSTANCE))
      Status = SNMP_NOT_WRITABLE;
      
    return TranslateReturnValue (Status, SnmpVersion);
}


SNMP_STATUS AppSendTrap (const unsigned char *AgentIp, int TrapType, int IfIndex)
{
    BcmIpAddress IP(AgentIp[0],AgentIp[1],AgentIp[2],AgentIp[3]);
    BcmSnmpAgent *pAgent = BcmSnmpThread::Singleton().FindAgentByIp (IP);
    
    if (pAgent == NULL)
      return SNMP_RESOURCE_UNAVAILABLE;
      
    pAgent->SendStandardTrap (TrapType, IfIndex, true);
    
    return SNMP_NO_ERROR;
}

int AppInformSentNotify ()
{
  // We don't do anything with this notification for now.  At one time, 
  // used it for event log throttling, but no more because it can miscount
  // if we have multiple targets.
  return 0;
}

SNMP_STATUS AppProvTargetAddrExtTable (unsigned char *snmpTargetAddrName, int snmpTargetAddrNameLen)
{
#if SNMP_SUPPORT == 3
    // Add an entry into the targetAddrExtTable
    BcmMibBridge *pBridge;
    BcmSnmpV3Agent *pAgent;
    
    // Since the snmpTargetAddrExtTable extends 'shared' table snmpTargetAddrTable,
    // we need to create this entry in any agent that supports it.  So loop.
    for (int Loop = 0; Loop < BcmSnmpThread::Singleton().NumAgents(); Loop++)
    {
      pAgent = BcmSnmpThread::Singleton().GetV3Agent(Loop);
      if (pAgent)
      {
        pBridge = new snmpTargetAddrExtEntryBridge (BcmString ((const char *)snmpTargetAddrName, snmpTargetAddrNameLen), pAgent);
        if (pBridge->Register (kDestroyMibObject, true) != SNMP_NO_ERROR)
        {
          // If this failed, then either we have tried to create an entry on
          // a slave agent, the agent doesn't support this table, or we have
          // already created an entry.  In any case, it's OK, which is why we
          // used the 'silent' argument above.
          delete pBridge;
        }
      }
    }
#endif
    return SNMP_NO_ERROR;
}

SNMP_STATUS AppDestroyTargetAddrExtTable (unsigned char *snmpTargetAddrName, int snmpTargetAddrNameLen)
{
  BcmSnmpAgent *pAgent;
    
  // Since the snmpTargetAddrExtTable extends 'shared' table snmpTargetAddrTable,
  // we need to destroy this entry in any agent that supports it.  So loop.
  for (int Loop = 0; Loop < BcmSnmpThread::Singleton().NumAgents(); Loop++)
  {
    pAgent = BcmSnmpThread::Singleton().GetAgent(Loop);
    if (pAgent)
    {
      BcmMibTable *pTable = pAgent->FindTable (BcmString("snmpTargetAddrExtTable"));
      if (pTable)
      {
        BcmObjectId Index (BcmString ((const char *)snmpTargetAddrName, snmpTargetAddrNameLen), BcmObjectId::kAscii);
        pTable->DestroyEntry (Index);
      }
    }
  }
  
  return SNMP_NO_ERROR;
}


SNMP_STATUS AppResolveSecNameToCommName ( unsigned char *pToIp, int ToPort, unsigned char *pFromIp, char *pCommunity, char *pSecurityName, int MaxLen)
{
#if (SNMP_SUPPORT == 3)
    BcmIpAddress To(pToIp[0],pToIp[1],pToIp[2],pToIp[3]);
    BcmIpAddress From(pFromIp[0],pFromIp[1],pFromIp[2],pFromIp[3]);
    BcmSnmpV3Agent *pAgent = BcmSnmpThread::Singleton().FindV3AgentByIp (From);
    SNMP_STATUS Status = SNMP_BAD_COMMUNITY_NAME;
    
   if (pSecurityName == NULL)
    {
      SnmpLogRaw << "ERROR: AppResolveSecNameToCommName: pSecurityName == NULL" << endl;
      return SNMP_WRONG_VALUE;
    }
    
    if (pCommunity)
      memset (pCommunity, 0, MaxLen);

    BcmString Community;
    BcmString SecurityName(pSecurityName);

    if (pAgent == NULL)
    {
      SnmpLogNm << "pAgent = NULL: RESOURCE_UNAVAILABLE" << endl;
      return SNMP_RESOURCE_UNAVAILABLE;
    }
     
    if (pAgent->fpCommunityTable)
    {  
        Status = pAgent->fpCommunityTable->ResolveSecNameToCommName (Community, To, SecurityName, ToPort);
        if ((Status == SNMP_NO_ERROR) && (pCommunity != NULL))
          strncpy (pCommunity, Community.c_str(), MaxLen-1);
    }
    
    return Status;
#else
    return SNMP_BAD_COMMUNITY_NAME;
#endif
}


int AppGetInterfaceNumber (unsigned char *AgentIp)
{
  BcmIpAddress IP(AgentIp[0],AgentIp[1],AgentIp[2],AgentIp[3]);
  BcmSnmpAgent *pAgent = BcmSnmpThread::Singleton().FindAgentByIp (IP);
  
  if (pAgent)
    return pAgent->GetIfNum();
    
  return 0;
}


int AppOpenSnmpSocket (unsigned long IpAddr, unsigned short Port)
{
  BcmIpAddress Ip;
  BcmSocket *pSocket;
  
  Ip.NetworkSet (IpAddr);
  
  pSocket = BcmSnmpThread::Singleton().OpenSnmpSocket (Ip, Port);
  
  if (pSocket == NULL)
    return 0;
    
  return pSocket->SocketDescriptor();
}

int AppCloseSnmpSocket (int Socket)
{
  return BcmSnmpThread::Singleton().CloseSnmpSocket (Socket);
}


int AppRecvfromSnmpSocket (int Socket, unsigned char *pBuf, int BufSize, unsigned long *pFromIp, unsigned short *pFromPort)
{
  BcmIpAddress Ip;
  
  int Retval = BcmSnmpThread::Singleton().RecvfromSnmpSocket (Socket, pBuf, BufSize, Ip, *pFromPort);
  
  if (Retval != -1)
    Ip.NetworkGet (*pFromIp);
    
  return Retval;
}


int AppSendtoSnmpSocket (int Socket, const unsigned char *pBuf, int BufSize, unsigned long ToIp, unsigned short ToPort)
{
  BcmIpAddress Ip;
  
  Ip.NetworkSet (ToIp);
  
  int Retval = BcmSnmpThread::Singleton().SendtoSnmpSocket (Socket, pBuf, BufSize, Ip, ToPort);
  
  return Retval;
}

void ExpungeVarBind (VarBind *pVB)
{
#ifdef NOTDEF
if (pVB->Type == VBT_STRING)
  printf("ExpungeVarBind Type %d buffersize %d len %d  ptr %p\n", pVB->Type,pVB->Value.String.Size, pVB->Value.String.Length, pVB->Value.String.pOctets);
#endif

    // If allocated string storage, free it.
    if ((pVB->Type == VBT_STRING)
    &&  (pVB->Value.String.DontFree == 0))
    {
        if ( pVB->Value.String.pOctets ) {
            delete [] pVB->Value.String.pOctets;
            pVB->Value.String.pOctets = NULL;
            pVB->Value.String.Size = 0;
            pVB->Value.String.Length = 0;
        }
    }

}

