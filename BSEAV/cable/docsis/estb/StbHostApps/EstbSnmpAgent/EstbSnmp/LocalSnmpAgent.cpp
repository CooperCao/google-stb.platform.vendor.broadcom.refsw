//****************************************************************************
//
// Copyright (c) 2010 Broadcom Corporation
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
//****************************************************************************
//    Filename: LocalSnmpAgent.cpp
//    Author:   Maurice Turcotte
//    Creation Date: 9 NOV 10
//
//**************************************************************************
//    Description:
//
//    The local SNMP agent - listens on loopback address and has full access
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "SnmpType.h"
#include "SnmpLog.h"
#include "LocalSnmpAgent.h"
#include "SnmpApi.h"
#include "EventLog.h"
#include "EstbEvents.h"
#include "EstbSnmpAgent.h"
#include "MessageLogNonVolSettings.h"
#include "EstbSnmpNonVolSettings.h"
#include "HalIfNonVolSettings.h"
#include "MibFactory.h"

#if (BFC_CM_VENDOR_SUPPORT)
#include "VendorCmAppFactory.h"
#endif

#if (BCM_FACTORY_SUPPORT)
#include "LocalFactorySnmpAgent.h"
#endif

LocalSnmpAgent *LocalSnmpAgent::fpSingletonInstance = NULL;

/* Local SNMP agent */

LocalSnmpAgent::LocalSnmpAgent () :
  DefaultSnmpAgentClass ((char *) LOCAL_AGENT_NAME, &EstbSnmpAgent::Singleton())
{
  SnmpLogStart << "Creating Local SNMP agent" << endl;
  fpNmAccessTable = NULL;
}

LocalSnmpAgent::~LocalSnmpAgent ()
{
}

#include "OperatingSystemFactory.h"
LocalSnmpAgent &LocalSnmpAgent::Singleton()
{
  // PR 9604: Prevent race condition instantiating singleton object
  static BcmMutexSemaphore *pMutex = BcmOperatingSystemFactory::NewMutexSemaphore("LocalSnmpAgent::Singleton mutex");
  BcmMutexSemaphoreAutoLock Lock(pMutex);

  if (fpSingletonInstance)
    return *fpSingletonInstance;
    
  fpSingletonInstance =  new LocalSnmpAgent();
  
  return *fpSingletonInstance;
}

bool LocalSnmpAgent::PopulateMibs ()
{
  // Local agent supports no MIBs directly, only via the eSTB SNMP agent.
  return true;
}

void LocalSnmpAgent::SetupSnoops ()
{
  // No snoops 
}

SNMP_STATUS LocalSnmpAgent::Start ()
{
  SNMP_STATUS Status;
  BcmSnmpNonVolSettings *pSnmpSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  
  
  SnmpLogStart << "Starting Local SNMP agent" << endl;
  
  Status = DefaultSnmpAgentClass::Start ();
  
  SetV1V2View ("docsisLocalView");
  
  fpNmAccessTable = (docsDevNmAccessTable *)FindTable ("docsDevNmAccessTable");
  
  // Go into factory mode if NV is set.
  if ((pSnmpSettings) && (pSnmpSettings->AllowConfiguration() == true))
    EnableFactoryMode (true);
  
  return Status;
}

SNMP_STATUS LocalSnmpAgent::EnableFactoryMode (bool Enable)
{
  SNMP_STATUS Status = DefaultSnmpAgentClass::EnableFactoryMode (Enable);
  if (Status != SNMP_NO_ERROR)
    return Status;
    
  if (EstbSnmpAgent::Singleton().ManagementEnabled() == false)
  {
    if (Enable == true)
      SetV1V2View ("docsisNmAccessView");
    //else
      //SetV1V2View ("docsisLocalView");
  }
  
  return SNMP_NO_ERROR;
}

SNMP_STATUS LocalSnmpAgent::InstallViews ()
{
  // Include EVERY-thing!
  unsigned long subtree_entireMib[]                     = {1}; 
  AgentConfigView ((char *) "docsisLocalView", subtree_entireMib, sizeof(subtree_entireMib)/sizeof(unsigned long), VIEW_STATUS_INCLUDE);
  
  return SNMP_NO_ERROR;
}

void LocalSnmpAgent::InformRegistrationComplete ()
{

  EnableManagement (true);
  
  char pView[32] = "docsisLocalView";
  
  SetV1V2View (pView);
}

void LocalSnmpAgent::InformRestartPending ()
{
}

SNMP_STATUS LocalSnmpAgent::InstallUsers ()
{
  return SNMP_NO_ERROR;
}

SNMP_STATUS LocalSnmpAgent::AuthenticateRequest (int RequestType, int SnmpVersion, int &SrcIfNum, const BcmIpAddress &SrcIp, const BcmString &Community)
{
  return SNMP_NO_ERROR;
}


SNMP_STATUS LocalSnmpAgent::TestObject (BcmObjectId &OID, BcmVarBind &VB, bool FromConfig, BcmMibRequest *pReq)
{
  return DefaultSnmpAgentClass::TestObject (OID, VB, FromConfig, pReq);
}

SNMP_STATUS LocalSnmpAgent::SetObject (BcmObjectId &OID, BcmVarBind &VB, bool FromConfig, BcmMibRequest *pReq)
{
  return DefaultSnmpAgentClass::SetObject (OID, VB, FromConfig, pReq);
}
