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
//    Filename: BcmSnmpAgent.cpp
//    Author:   Kevin O'Neal
//    Creation Date: 14-Jun-2001
//
//**************************************************************************
//    Description:
//
//		Cablemodem V2 code
//		
//    BcmSnmpAgent base class
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "SnmpLog.h"
#include "SnmpApi.h"
#include "SnmpV3Agent.h"

#include "communityMib.h"
#include "communityBridge.h"
#include "BerObjs.h"


BcmSnmpV3Agent::BcmSnmpV3Agent (char *pName, BcmSnmpAgent *pMasterAgent)
  : BcmSnmpAgent (pName, pMasterAgent)
{
  fpCommunityTable = NULL;
  fpDhThread = NULL;
}

BcmSnmpV3Agent::~BcmSnmpV3Agent ()
{
}

ostream &BcmSnmpV3Agent::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  BcmString EngineId;


  BcmSnmpAgent::Print (outStream);  
  
  EngineId = GetEngineId();

  // Print SNMPv3 things  
  outStream << "  Context: '" << GetContext() << "'" << endl;
  if (EngineId.length())
    outStream << "  Engine ID: " << BcmOctetBuffer ((unsigned char *)EngineId.data(), EngineId.length(), EngineId.length(), false);
  else
    outStream << "  Engine ID: <empty>" << endl;
    
  outStream << "  Engine boots: " << GetEngineBoots() << endl;
#endif

  return outStream;
}

void BcmSnmpV3Agent::SetTLV38Base (const char *TLV38_BaseName)
{
  fTLV38_Base = TLV38_BaseName;
  fTLV38_Base_0 = fTLV38_Base;
  fTLV38_Base_0 += "_0";
  fTLV38_Base_inform = fTLV38_Base;
  fTLV38_Base_inform += "_inform";
  fTLV38_Base_trap = fTLV38_Base;
  fTLV38_Base_trap += "_trap";
  fTLV38_BaseUSM = fTLV38_Base;
  fTLV38_BaseUSM += "USM";
  fTLV38_BaseV1 = fTLV38_Base;
  fTLV38_BaseV1 += "V1";
  fTLV38_BaseV2 = fTLV38_Base;
  fTLV38_BaseV2 += "V2";
}

void BcmSnmpV3Agent::SetTLV53Base (const char *TLV53_BaseName)
{
  fTLV53_Base = TLV53_BaseName;
}

SNMP_STATUS BcmSnmpV3Agent::Register ()
{
  SNMP_STATUS Status = BcmSnmpAgent::Register();
  
  if (Status != SNMP_NO_ERROR)
    return Status;

  // Let the thread know I am a V3 agent.
  BcmSnmpThread::Singleton().RegisterV3Agent(this);
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpV3Agent::Start ()
{
  SNMP_STATUS Status = BcmSnmpAgent::Start();
  
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  // Set up SNMPv3 things...
  InstallEngineId ();
  InstallContext ();
  
  return SNMP_NO_ERROR;
}

bool BcmSnmpV3Agent::PopulateMibs()
{
  if (BcmSnmpAgent::PopulateMibs() == false)
    return false;
    
  LockMib ();
  
/* 
  Should I do this here?  Currently derived objects do this...
  BcmMibGroup pGroup = new snmpCommunityMIBObjectsGroup ();
  AddGroup (pGroup);
*/
  
  UnlockMib ();
  
  return true;
}

bool BcmSnmpV3Agent::PopulateBridges ()
{
  if (BcmSnmpAgent::PopulateBridges() == false)
    return false;

  LockMib ();
  
  // Find the community table if we haven't already.  This should have been
  // installed in our derived object's PopulateMibs method.
  if (fpCommunityTable == NULL)
    fpCommunityTable = (snmpCommunityTable *) FindTable (BcmString("snmpCommunityTable"));  
  
  UnlockMib();
  
  return true;
}


SNMP_STATUS BcmSnmpV3Agent::InstallEngineId ()
{
  unsigned char pMac[6];
  unsigned char pEngineId[12];
  BcmMacAddress MyMac;
  
  SnmpLogRaw << GetName() << " installing engine ID..." << endl;

  MyMac = GetMacAddress();

  MyMac.Get(pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5]);

  pEngineId[0]  =  0x80; //as defined by SNMPv3 architecture.

  pEngineId[1]  =  0x00; //next 3 octets = our IANA enterprise ID (4413).
  pEngineId[2]  =  0x11;
  pEngineId[3]  =  0x3d;

  pEngineId[4]  =  0x05; // indicates that the next 6 octets are 
  		   			     // formatted as administratively assigned hex values.

  pEngineId[5]  =  pMac[0];
  pEngineId[6]  =  pMac[1];
  pEngineId[7]  =  pMac[2];
  pEngineId[8]  =  pMac[3];
  pEngineId[9]  =  pMac[4];
  pEngineId[10] =  pMac[5];

  pEngineId[11] =  0x33; // padding to get 12 octets.  Don't use 0x00 because
                         // it confuses stester.
  
  // get sysObjectId from NV and use element 6 as the enterprise ID.
  BcmObjectId sysObjectId = sysObjectID();
  
  // Small sanity check...
  if (sysObjectId.Length() > 6)
  {
    pEngineId[1] = (sysObjectId[6] >> 16) & 0xFF;
    pEngineId[2] = (sysObjectId[6] >> 8)  & 0xFF;
    pEngineId[3] = (sysObjectId[6] >> 0)  & 0xFF;
  }
  
  return AgentSetEngineId (pEngineId, sizeof(pEngineId));
}

SNMP_STATUS BcmSnmpV3Agent::InstallContext ()
{
  SnmpLogRaw << GetName() << " installing context..." << endl;
  return AgentSetContext ((char *) "");
}

SNMP_STATUS BcmSnmpV3Agent::InstallUsers ()
{
  // No default users
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpV3Agent::EnableUser (const BcmString &UserName)
{
  return AgentEnableUser ((char *)UserName.c_str());
}

SNMP_STATUS BcmSnmpV3Agent::DisableUser (const BcmString &UserName)
{
  return AgentDisableUser ((char *)UserName.c_str());
}

SNMP_STATUS BcmSnmpV3Agent::InstallNotifies ()
{
  // No default notifies
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpV3Agent::DestroyUsers ()
{
  SnmpLogRaw << GetName() << " destroying users..." << endl;
  
  AgentCleanupGroups ();
  AgentCleanupAccess ();
  return AgentCleanupUsers ();
}

SNMP_STATUS BcmSnmpV3Agent::DestroyNotifies ()
{
#if (SNMP_SUPPORT == 3)
  SnmpLogRaw << GetName() << " destroying notifies..." << endl;
  return AgentCleanupNotifies ();
#else
  return SNMP_NO_ERROR;
#endif
}


SNMP_STATUS BcmSnmpV3Agent::ProvisionDHKickstartEntry (BcmOctetBuffer &SecurityName, BcmOctetBuffer &PublicNumber)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  unsigned char SecurityNameWithNull[128];
  
  // Add a NULL terminator to the security name by pre-setting the buffer to
  // all 0's then copying in the data bytes we got passed over.
  memset (SecurityNameWithNull, 0, sizeof(SecurityNameWithNull));
  
  // Shouldn't ever have a DH kickstart value anywhere near 128 bytes long, but
  // check just in case somebody created a bogus config file.
  if (SecurityName.BytesUsed() >= sizeof(SecurityNameWithNull))
    memcpy (SecurityNameWithNull, SecurityName.AccessBuffer(), sizeof(SecurityNameWithNull)-1);
  else
    memcpy (SecurityNameWithNull, SecurityName.AccessBuffer(), SecurityName.BytesUsed());
    
  // call AgentKickstartRegister here...
  SnmpLogRaw << "Registering DH kickstart: security name is <" << (char *)SecurityNameWithNull << ">" << endl;
  
  Status = AgentKickstartRegister(GetIfNum(), (char *)SecurityNameWithNull,
								   (char*)PublicNumber.AccessBuffer(),
								   PublicNumber.BytesUsed());
  
  return Status;
}
   
SNMP_STATUS BcmSnmpV3Agent::ProvisionNotifyEntry (int TrapType, const BcmString &ElementName,const BcmString &SecurityName,
											   const BcmIpAddress &TargetIp,int Port,int TimeOut,int Retries,BcmOctetBuffer &Filter)
{
    unsigned char pTargetIp[4];
	unsigned int Size = 0;
	TargetIp.Get (pTargetIp[0],pTargetIp[1],pTargetIp[2],pTargetIp[3]);
    BcmString SecName = SecurityName;
    
    if (( TimeOut > 65535)||(TimeOut < 1 ))
        TimeOut = 1500;
    if ((Retries > 255)||(Retries < 1))
        Retries = 3;
    
    //A securityName is valid only for types 4 and 5
    //Make sure it is null so as to not cause problems further down the line.
    if ((TrapType != 4) && (TrapType != 5))
        SecName = "";   

    SnmpLogRaw << "TLV-38, provisioning notify for " << TargetIp << endl;
    
    AgentProvTargetParamsTable(TrapType,(unsigned char *)(ElementName.data()),ElementName.length(),
							   (unsigned char *)(SecName.data()),SecName.length(),(unsigned char *)fTLV38_Base.c_str());  
	AgentProvTargetAddrTable(pTargetIp,Port,TrapType,TimeOut,Retries,(unsigned char *)(ElementName.data()),
							 ElementName.length(),(unsigned char *)fTLV38_Base_inform.c_str(), (unsigned char *)fTLV38_Base_trap.c_str());
    
    // Create an entry in the snmpTargetAddrExtTable to extend the one we
    // have just made in the snmpTargetAddrTable.
    snmpTargetAddrExtEntryBridge *pBridge = new snmpTargetAddrExtEntryBridge (BcmString(ElementName),this);
    if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
       delete pBridge;

	//provision the filter tables if we have a filter
	unsigned int  ucFilterOidLen = Filter.BytesUsed();
	if (ucFilterOidLen > 0) {
		// - The filter OID
		if (ucFilterOidLen > MAX_OID_SIZE)
			return SNMP_BAD_VALUE;

		unsigned char ucOid[MAX_OID_SIZE];
		BcmObjectId   bcmOid;
		ObjectId      oOid;

		//Filter is a BcmOctetBuffer - Get the data from Filter into a uchar buffer
		Filter.Read(Size,ucOid,ucFilterOidLen);

		//put the uchar in a AsnBerOid object in order to parse it.
		AsnBerOid berOid(ucOid);
		berOid.Parse();         
		berOid.GetValue(bcmOid);   //get the parsed oid into a BcmObjectId object

        bcmOid.GetOID(&oOid);	  //get the ObjectId from BcmObjectId
        
	    AgentProvNotifyFilterProfileTable((unsigned char *)(ElementName.data()),ElementName.length());
	 	AgentProvNotifyFilterTable((unsigned char *)(ElementName.data()),ElementName.length(),
	    						   oOid.pObjects, oOid.Length);
	}

	if (ElementName == fTLV38_Base_0)
    {
		//This is a cheap way to tell if this is our first time through
		//but it will do for now. Need to execute this section only once
		//given one to many calls into this method.
		AgentProvNotifyTable((unsigned char *)fTLV38_Base_inform.c_str(), (unsigned char *)fTLV38_Base_trap.c_str());  
		
        //community MIB section
        snmpCommunityEntryBridge *pCommunityBridge;
        pCommunityBridge = new snmpCommunityEntryBridge (fTLV38_Base,this);
        if (pCommunityBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
            delete pCommunityBridge;
        else
        {
          int Active = SNMP_STATUS_ACTIVE;
          
          pCommunityBridge->Set_snmpCommunityName (BcmString("public"));
          pCommunityBridge->Set_snmpCommunitySecurityName (fTLV38_Base);
          pCommunityBridge->Set_snmpCommunityStatus (Active);
        }

        //vacmViewTreeFamilyTable
        static unsigned long subtree_notify[] = {1,3}; 
        AgentConfigView ((char *)fTLV38_Base.c_str(), subtree_notify, sizeof(subtree_notify)/sizeof(unsigned long), VIEW_STATUS_INCLUDE);
    
        //vacmSecurityToGroupTable
        AgentInstallGroup ((char *)fTLV38_BaseV1.c_str(), (char *)fTLV38_Base.c_str(), SNMP_SEC_MDL_V1);
        AgentInstallGroup ((char *)fTLV38_BaseV2.c_str(), (char *)fTLV38_Base.c_str(), SNMP_SEC_MDL_V2);
        AgentInstallGroup ((char *)fTLV38_BaseUSM.c_str(), (char *)fTLV38_Base.c_str(), SNMP_SEC_MDL_USM);
    
        //usmUserTable MIB section

        //AgentInstallUser ((char *)fTLV38_Base.c_str(), SNMP_SEC_NONE);
        //EnableUser(fTLV38_Base);

        // We don't need the @config user with our engineId for probes anymore.
        // Install it with engine id of 0x00 per the spec.  PR 6798.
        unsigned char eng[] = {0x00};
        unsigned char *pEng = &eng[0];
        int EngLen = sizeof(eng); 

        AgentInstallUserComplete ((unsigned char*)"@config", //unsigned char *pSecName,
                                              7,             //unsigned int SecNameLen,
                                              SNMP_SEC_NONE, //int SecLevel,
                                              pEng,          //unsigned char *pEngineId, 
                                              EngLen,        //int EngineIdLen,
                                              NULL,          //unsigned char *pAuthKey,
                                              0,             //int AuthKeyLen,
                                              0,             //int AuthType,
                                              NULL,          //unsigned char *pPrivKey,
                                              0,             //int PrivKeyLen,
                                              0,             //int Password2Key, 
                                              0);            //int LocalizePwd)


        AgentInstallAccess ((char *)fTLV38_BaseV1.c_str(), (char *) "", (char *) "", (char *)fTLV38_Base.c_str(), SNMP_SEC_NONE, SNMP_SEC_MDL_V1);
        AgentInstallAccess ((char *)fTLV38_BaseV2.c_str(), (char *) "", (char *) "", (char *)fTLV38_Base.c_str(), SNMP_SEC_NONE, SNMP_SEC_MDL_V2);
        AgentInstallAccess ((char *)fTLV38_BaseUSM.c_str(), (char *) "", (char *) "", (char *)fTLV38_Base.c_str(), SNMP_SEC_NONE, SNMP_SEC_MDL_USM);
    }
   
   return SNMP_NO_ERROR;
}

const BcmString &BcmSnmpV3Agent::GetEngineId () const
{
  static BcmString EngineId;
  unsigned char pEngineId[32];
  int EngineIdLen;
  
  
  EngineIdLen = AgentGetEngineId (pEngineId, sizeof(pEngineId));
  EngineId.assign ((char *)pEngineId, EngineIdLen);
  
  return EngineId;
}

unsigned long BcmSnmpV3Agent::GetEngineBoots () const
{
  // Base class has no idea...
  return 1;
}

const BcmString &BcmSnmpV3Agent::GetContext () const
{
  static BcmString Context;
  char pContext[32];
  int ContextLen;
  
  
  ContextLen = AgentGetContext (pContext, sizeof(pContext));
  Context = pContext;
  
  return Context;
}


// Note that the reason we take an int & for SrcIfNum is because we might do a
// N2M lookup and map it if we get a 0, that way derived objects etc. can
// benefit from the lookup.
SNMP_STATUS BcmSnmpV3Agent::AuthenticateRequest (int RequestType, int SnmpVersion, int &SrcIfNum, const BcmIpAddress &SrcIp, const BcmString &Community)
{
  // For now just return the base class implementation.  We will consult our
  // community table in the future
  return BcmSnmpAgent::AuthenticateRequest (RequestType, SnmpVersion, SrcIfNum, SrcIp, Community);
}


BcmDhKickstartThread *BcmSnmpV3Agent::DhKickstartThread() const
{
  if (fpDhThread == NULL)
    return NULL;
    
  return fpDhThread;
}

SNMP_STATUS BcmSnmpV3Agent::ProvisionCommunityEntry (const BcmString &tableIndex, const BcmString &commName, const BcmString &secName, const BcmString &transportTag) {
  snmpCommunityEntryBridge *pComm;
  SNMP_STATUS retVal;

  pComm = new snmpCommunityEntryBridge(tableIndex, this);

  // First check the values
  if ((retVal = pComm->Test_snmpCommunityName(commName)) != SNMP_NO_ERROR) {
    // SnmpLogRaw << "Bad Community name in TLV-53 from config file..." << endl;
    delete pComm;
    return retVal;
  }
  if ((retVal = pComm->Test_snmpCommunitySecurityName(commName)) != SNMP_NO_ERROR) {
    // SnmpLogRaw << "Bad Security name in TLV-53 from config file..." << endl;
    delete pComm;
    return retVal;
  }

  if ((retVal = pComm->Register (kDestroyMibObject)) != SNMP_NO_ERROR) {
    delete pComm;
    return retVal;
  }

  
  // Now go ahead and set all the values
  pComm->Set_snmpCommunityName(commName);
  pComm->Set_snmpCommunitySecurityName(secName);
  pComm->Set_snmpCommunityTransportTag(transportTag);
  int Status = CONSTVAL_SNMPCOMMUNITYSTATUS_ACTIVE;
  pComm->Set_snmpCommunityStatus(Status);
  return SNMP_NO_ERROR;
}

SNMP_STATUS BcmSnmpV3Agent::ProvisionTargetAddrEntry (const BcmString &elementName, const BcmString &tAddr, const BcmString &tagList) {
  return (AgentProvTLV53TargetAddrTable((unsigned char *) elementName.c_str(),
         (unsigned char *) tAddr.c_str(), (unsigned char *) tagList.c_str()));
}

SNMP_STATUS BcmSnmpV3Agent::ProvisionTargetAddrExtEntry (const BcmString &elementName, const BcmString &tAddrMask) {
  SNMP_STATUS retVal;

  snmpTargetAddrExtEntryBridge *pBridge = new snmpTargetAddrExtEntryBridge (BcmString(elementName),this);

  if (pBridge == NULL)
    return SNMP_GEN_ERR;

  if ((retVal = pBridge->Register (kDestroyMibObject)) != SNMP_NO_ERROR) {
    delete pBridge;
    return retVal;
  }

  if ((retVal = pBridge->Set_snmpTargetAddrTMask (tAddrMask)) != SNMP_NO_ERROR) {
    delete pBridge;
  }

  return retVal;
}

SNMP_STATUS BcmSnmpV3Agent::ProvisionSecurityToGroupEntry (int secModel, const BcmString &secName, const BcmString &groupName) {

  return (AgentInstallGroup ((char *)groupName.c_str(), 
                             (char *)secName.c_str(), secModel));
}

SNMP_STATUS BcmSnmpV3Agent::ProvisionVacmAccessEntry (const BcmString &groupName, int viewType, const BcmString &viewName, int secModel) {
  if (viewType == 1) {   // Read-only
   return (AgentInstallAccess ((char *)groupName.c_str(), (char *)viewName.c_str(),
                    (char *) "", (char *) "", SNMP_SEC_NONE, secModel));
  }
  if (viewType == 2) {   // Read-Write
   return (AgentInstallAccess ((char *)groupName.c_str(), (char *)viewName.c_str(),
           (char *)viewName.c_str(), (char *) "", SNMP_SEC_NONE, secModel));
  }
  return SNMP_GEN_ERR;
}


SNMP_STATUS BcmSnmpV3Agent::ProvisionVacmViewTreeFamilyEntry (const BcmString &viewName, const BcmObjectId &subtree, BcmOctetBuffer &mask, int type) {

  ObjectId oOid;

  subtree.GetOID(&oOid);     //get the ObjectId from BcmObjectId

  // TODO: need to handle the mask
  // SnmpLogRaw << "ProvisionVacmViewTreeFamilyEntry - mask ignored"<<endl;

  return (AgentConfigView ((char *)viewName.c_str(), oOid.pObjects, 
                           oOid.Length, type));
}


