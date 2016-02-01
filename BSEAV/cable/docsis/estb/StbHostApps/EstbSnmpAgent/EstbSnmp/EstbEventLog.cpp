//****************************************************************************
//
// Copyright (c) 2007-2011 Broadcom Corporation
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
//    Filename: EstbEventLog.cpp
//    Author:   Maurice Turcotte
//    Creation Date: 12 MAY 06
//
//**************************************************************************
//    Description:
//
//    Estb event log objects
//
//		
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "EstbEventLog.h"
#include "EstbEvents.h"
#include "SnmpLog.h"
#include "NonVolDevice.h"
#include "EstbEventLogNonVolSettings.h"
#include "BcmSnmpAgent.h"
#include "EstbSnmpAgent.h"
#include "OperatingSystemFactory.h"
#include "NmAccess.h"

#if (SNMP_SUPPORT)
#include "docsDevTrap.h"
#include "docsDevTrapBridge.h"
#include "docsDevEvMib.h"
#include "docsDevNmBridge.h"
#endif

#if (BFC_CM_VENDOR_SUPPORT)
#include "VendorCmAppFactory.h"
#endif

#if (BCM_FACTORY_SUPPORT)
#include "CmFactoryEventLog.h"
#endif


// UNFINISHED: go constipate the whole chain of event logging so DAVE PULLEN
// will be happy!  For now, though, just cast them away...
SNMP_STATUS EstbLogEvent (EventLogIdCode Id, EventLogSeverityLevel Level, const char *pText, EventLogPolicy Policy)
{
  if (EstbEventLog::Singleton().GetLevelControl(-1) == 0)
  { 
    SnmpLogEv << "Event logging disabled (" << (int)Id << ")" << endl;
    return SNMP_NO_ERROR;
  }
  
  return EstbEventLog::Singleton().LogEvent (Id, Level, (char *)pText, Policy);
}


SNMP_STATUS EstbLogEvent (EventLogIdCode Id, const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5)
{
  if (EstbEventLog::Singleton().GetLevelControl(-1) == 0)
  { 
    SnmpLogEv << "Event logging disabled (" << (int)Id << ")" << endl;
    return SNMP_NO_ERROR;
  }
  
  return EstbEventLog::Singleton().LogEvent (Id, (char *)pParam1, (char *)pParam2, (char *)pParam3, (char *)pParam4, (char *)pParam5);
}


SNMP_STATUS EstbSendTrap (int TrapType, int IfIndex)
{
  return EstbEventLog::Singleton().SendTrap (TrapType, IfIndex);
}


// The following functions are usable by "C" modules.
SNMP_STATUS EstbLogEventIdOnly (EventLogIdCode Id)
{
  return EstbLogEvent (Id);
}

SNMP_STATUS EstbLogEventIdPlusText (EventLogIdCode Id, const char *pParam)
{
  return EstbLogEvent (Id, pParam);
}


/******************************************************************************\
**                                                                            **
* Default arguments for event log ID codes.  These can be specified if you     *
* want, but it's not required.  Then EstbLogEvent can just be called with        *
* the ID code only, and defaults will be filled in.                            *
**                                                                            **
\******************************************************************************/

DocsisEvent EstbEventLog::fEvents [] = 
{
  // EventLogIdCode, EventLogSeverityLevel, Text, EventLogPolicy, WriteToNV
  
  /*\
   *  Standard error codes. 
   *  There are also some vendor-specific codes here.
   *
   *  Updated to OC-SP-HOST2.1-CFR-I08-090508 Annex A
  \*/

  {kEE_T40101, kNotice, (char *) "Unrecognized TLV or sub-TLVs detected <P1>", kDefaultNewRowBehavior, kDefaultNvBehavior, kUnspecifiedTrap, NULL}, // <P1> = TLVnn
  {kEE_T40102, kNotice, (char *) "Invalid TLV Type encoding <P1>", kDefaultNewRowBehavior, kDefaultNvBehavior, kUnspecifiedTrap, NULL}, // <P1> = TLVnn
  {kEE_T40103, kNotice, (char *) "Resource limit reached", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},

  {kEE_T41111, kNotice, (char *) "TLV-1 - Multiple TLVs not allowed", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},

  {kEE_T41101, kNotice, (char *) "TLV-11 - unrecognized OID", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T41102, kWarning, (char *) "TLV-11 - Multiple SET operations of object instance attempted", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T41103, kWarning, (char *) "TLV-11 - Invalid varbind encoding", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},

  {kEE_T43801, kWarning, (char *) "TLV-38 - Duplicate TLV Received", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T43802, kCritical, (char *) "TLV-38 - No Receiver IP Address received in TLV", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T43803, kWarning, (char *) "TLV-38 - No SNMPv2c Trap Notification Receiver specified", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T43804, kWarning, (char *) "TLV-38 - Inconsistent IP address type specified in TLV1", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},

  {kEE_T44301, kCritical, (char *) "TLV43 - Invalid Vendor ID encoding", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},

  {kEE_T45301, kCritical, (char *) "TLV53 - Community Name not present", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T45302, kCritical, (char *) "TLV53 - Transport address not present", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T45303, kWarning, (char *) "TLV53 - Multiple sub-TLVs not allowed", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T45304, kWarning, (char *) "TLV53 - SNMP syntax conflicts detected", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T45305, kWarning, (char *) "TLV53 - Insufficient table resources", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T45306, kWarning, (char *) "TLV53 - Attempt to create duplicate entry not allowed", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},

  {kEE_T45401, kWarning, (char *) "TLV54 - Access View Name not present", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T45402, kWarning, (char *) "TLV54 - Multiple sub-TLVs not allowed", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T45403, kWarning, (char *) "TLV54 SNMP syntax conflicts detected", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T45404, kWarning, (char *) "TLV54 - Insufficient table resources", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_T45405, kWarning, (char *) "TLV54 - Attempt to create duplicate entry not allowed", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},

  {kEE_D00100, kCritical, (char *) "DHCP FAILED - Discover sent, no offer received for <P1>", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL}, // <P1> = eSTB or CC
  {kEE_D00200, kCritical, (char *) "DHCP FAILED - Request sent, No response for <P1>", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL}, // <P1> = eSTB or CC
  {kEE_D00301, kCritical, (char *) "DHCP FAILED - Response doesn't contain ALL the valid fields for <P1>", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL}, // <P1> = eSTB or CC
  {kEE_D01200, kCritical, (char *) "DHCP failed - RS sent, no RA received", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_D01201, kCritical, (char *) "DHCP Failed - Invalid RA", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_D01202, kCritical, (char *) "DHCP failed - DHCP Solicit sent, No DHCP Advertise received", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_D01203, kCritical, (char *) "DHCP failed - DHCP Request sent, No DHCP REPLY received", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_D01301, kCritical, (char *) "Link-Local address failed DAD", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_D01302, kCritical, (char *) "DHCP lease address failed DAD", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},

  {kEE_D10100, kError, (char *) "DHCP RENEW sent - No response for <P1>", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL}, // <P1> = eSTB or CC
  {kEE_D10200, kError, (char *) "DHCP REBIND sent - No response for <P1>", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL}, // <P1> = eSTB or CC
  {kEE_D10300, kError, (char *) "DHCP RENEW sent - Invalid DHCP option for <P1>", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL}, // <P1> = eSTB or CC
  {kEE_D10400, kError, (char *) "DHCP REBIND sent - Invalid DHCP option for <P1>", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL}, // <P1> = eSTB or CC
  {kEE_B01_0, kNotice, (char *) "Hardware and OS UP", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B02_0, kNotice, (char *) "Execution Engine UP", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B03_0, kNotice, (char *) "Copy Protection Resource opened", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B04_0, kNotice, (char *) "Host AuthKey sent", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B05_0, kCritical, (char *) "Binding Failure: Card reasons", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B06_0, kCritical, (char *) "Binding Failure Invalid Host Cert:", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B07_0, kCritical, (char *) "Binding Failure: Invalid Host Signature", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B08_0, kCritical, (char *) "Binding Failure: Invalid Host AuthKey", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B09_0, kCritical, (char *) "Binding Failure: Other", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B10_0, kCritical, (char *) "Card Validation Error: Validation revoked", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B11_0, kCritical, (char *) "Binding Failure: Incompatible module", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B12_0, kNotice, (char *) "Binding Complete: Card/Host Validated", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B13_0, kNotice, (char *) "Copy Protection Initiated", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B14_0, kNotice, (char *) "FDC acquired", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B15_0, kNotice, (char *) "Set QPSK mode", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B16_0, kNotice, (char *) "Set Adv. DSG mode", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B17_0, kNotice, (char *) "Set Adv. DSG 1-way mode", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B18_0, kNotice, (char *) "DSG acquired", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B19_0, kNotice, (char *) "DOCSIS 2-way established", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B20_0, kNotice, (char *) "VCT acquired", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B21_0, kNotice, (char *) "OCAP launched", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B22_0, kError, (char *) "OCAP Startup Error", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B23_0, kNotice, (char *) "XAIT received", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B24_0, kError, (char *) "Error reading XAIT", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B25_0, kNotice, (char *) "Initial Monitor App launched", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B26_0, kNotice, (char *) "Initial Monitor App Startup Error", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B27_0, kNotice, (char *) "Proprietary Condition Met", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B28_0, kNotice, (char *) "Host Image Download Complete", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B29_0, kError, (char *) "Common Download CVT Error", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL},
  {kEE_B30_0, kError, (char *) "Host Image Download Error, <P1>", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL}, // <P1> = Error code 
  {kEE_B31_0, kNotice, (char *) "Card Image Download Complete", kUpdateExisting, kDefaultNvBehavior, kUnspecifiedTrap, NULL}

};


EstbEventLog::EstbEventLog ()
#if (SNMP_SUPPORT)
  : EventLog ("eSTB Event Log", &EstbSnmpAgent::Singleton())
#else
  : EventLog ("eSTB Event Log")
#endif
{
  memset (fCmStatusCode, 0, sizeof (fCmStatusCode));
  fCmStatusCodeTime = 0;
  
  fpSettings = EstbEventLogNonVolSettings::GetSingletonInstance();
}

EstbEventLog::~EstbEventLog()
{
}

EstbEventLog &EstbEventLog::Singleton ()
{
  static EstbEventLog *gSingleton = NULL;
  
  if (gSingleton)
    return *gSingleton;
    
#if (BFC_CM_VENDOR_SUPPORT)
  // If we have vendor support, get the singleton from the factory.
  // If he returns NULL, then fall through to non-vendor behaviour
  // below.
  gSingleton = BcmVendorCmAppFactory::NewCmEventLog();
    
  if (gSingleton == NULL)
#endif
  {
#if (BCM_FACTORY_SUPPORT)
    gSingleton = new CmFactoryEventLog();
#else
    gSingleton = new EstbEventLog();
#endif
  }
  
  return *gSingleton;
}


ostream &EstbEventLog::PrintControl (ostream &outStream) const
{
  EventLog::PrintControl (outStream);

#if (SNMP_SUPPORT)
  outStream << "\nTrap Control:" << endl;
  
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMINITTLVUNKNOWNTRAP))
    outStream << "  CmInitTLVUnknownTrap:   ENABLED" << endl;
  else
    outStream << "  CmInitTLVUnknownTrap:   DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDYNSERVREQFAILTRAP))
    outStream << "  CmDynServReqFailTrap:   ENABLED" << endl;
  else
    outStream << "  CmDynServReqFailTrap:   DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDYNSERVRSPFAILTRAP))
    outStream << "  CmDynServRspFailTrap:   ENABLED" << endl;
  else
    outStream << "  CmDynServRspFailTrap:   DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDYNSERVACKFAILTRAP))
    outStream << "  CmDynServAckFailTrap:   ENABLED" << endl;
  else
    outStream << "  CmDynServAckFailTrap:   DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMBPIINITTRAP))
    outStream << "  CmBpiInitTrap:          ENABLED" << endl;
  else
    outStream << "  CmBpiInitTrap:          DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMBPKMTRAP))
    outStream << "  CmBPKMTrap:             ENABLED" << endl;
  else
    outStream << "  CmBPKMTrap:             DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDYNAMICSATRAP))
    outStream << "  CmDynamicSATrap:        ENABLED" << endl;
  else
    outStream << "  CmDynamicSATrap:        DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDHCPFAILTRAP))
    outStream << "  CmDHCPFailTrap:         ENABLED" << endl;
  else
    outStream << "  CmDHCPFailTrap:         DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMSWUPGRADEINITTRAP))
    outStream << "  CmSwUpgradeInitTrap:    ENABLED" << endl;
  else
    outStream << "  CmSwUpgradeInitTrap:    DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMSWUPGRADEFAILTRAP))
    outStream << "  CmSwUpgradeFailTrap:    ENABLED" << endl;
  else
    outStream << "  CmSwUpgradeFailTrap:    DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMSWUPGRADESUCCESSTRAP))
    outStream << "  CmSwUpgradeSuccessTrap: ENABLED" << endl;
  else
    outStream << "  CmSwUpgradeSuccessTrap: DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMSWUPGRADECVCTRAP))
    outStream << "  CmSwUpgradeCVCFailTrap: ENABLED" << endl;
  else
    outStream << "  CmSwUpgradeCVCFailTrap: DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMTODFAILTRAP))
    outStream << "  CmTODFailTrap:          ENABLED" << endl;
  else
    outStream << "  CmTODFailTrap:          DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDCCREQFAILTRAP))
    outStream << "  CmDCCReqFailTrap:       ENABLED" << endl;
  else
    outStream << "  CmDCCReqFailTrap:       DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDCCRSPFAILTRAP))
    outStream << "  CmDCCRspFailTrap:       ENABLED" << endl;
  else
    outStream << "  CmDCCRspFailTrap:       DISABLED" << endl;
    
  if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDCCACKFAILTRAP))
    outStream << "  CmDCCAckFailTrap:       ENABLED" << endl;
  else
    outStream << "  CmDCCAckFailTrap:       DISABLED" << endl;
#endif
    
  return outStream;
}

bool EstbEventLog::IsThrottled ()
{
  if (EventLog::IsThrottled() == true)
    return true;

#if (SNMP_SUPPORT)  
  // If we are in NmAccess mode, then we are throttled per RFC-2669 if we have
  // no (active) docsDevNmAccessEntries w/ trap permission and also no syslog
  // IP address.  For coexistence we must not apply this because obviously we
  // have no docsDevNmAccessEntries table in that case.  Note that there was a 
  // good bit of reflector traffic on this matter about 12/2002, the requirement
  // to return 'throttled' in this case (never has been tested) may go away.
  if (EstbSnmpAgent::Singleton().GetDocsisSnmpMode() == SNMP_MODE_NMACCESS)
  // &&  (CmSnmpAgent::Singleton().GetDocsisMode() != DOCSIS_MODE_UNREGISTERED))
  {
    NmAccessController *pNm;
    bool GotTraps = false;
    static docsDevNmAccessTable *pNmTable = NULL;
    
    if (fpAgent)
    {
      fpAgent->LockMib();
      
      if (pNmTable == NULL)
        pNmTable = (docsDevNmAccessTable *) fpAgent->FindTable(BcmString("docsDevNmAccessTable"));
      
      // Do we have any (active) NMS's w/ trap permissions?
      if (pNmTable)
      {
        pNm = pNmTable->FirstNmController ();
        while (pNm)
        {
          if ((pNm->GetControl() == CONSTVAL_DOCSDEVNMACCESSCONTROL_ROWITHTRAPS)
          ||  (pNm->GetControl() == CONSTVAL_DOCSDEVNMACCESSCONTROL_RWWITHTRAPS)
          ||  (pNm->GetControl() == CONSTVAL_DOCSDEVNMACCESSCONTROL_TRAPSONLY))
          {
            if (pNm->GetStatus() == CONSTVAL_DOCSDEVNMACCESSSTATUS_ACTIVE)
            {
              GotTraps = true;
              break;
            }
          }
          
          pNm = pNmTable->NextNmController (pNm);
        }
      }
      
      fpAgent->UnlockMib();
      
      // Wha'bout a syslog IP addr?
      if ((GotTraps == false)
      &&  (fSyslogIp == kAllZerosIp))
      {
        SnmpLogEv << "EstbEventLog::IsThrottled - throttled because fSyslogIp is empty and no trap destinations" << endl;
        return true;
      }
    }
  }
#endif
  
  return false;
}

SNMP_STATUS EstbEventLog::InitFromNonVolImpl ()
{
  // Let the base class do his thing:
  SNMP_STATUS Status = EventLog::InitFromNonVolImpl();
  
  if (Status != SNMP_NO_ERROR)
    return Status;

#if (SNMP_SUPPORT)  
  if (fpAgent)  
    fpAgent->LockMib();
#endif
    
  /*\
   *  How should we initialize docsIfCmStatusCode?  Well, here's what section
   *  4.4.2.3 of OSS-I-05 says:
   *  
   *  "Alert event (priority 2)
   *   A serious failure, which causes reporting system to reboot but it is not 
   *   caused by h/w or s/w malfunctioning. After recovering from the critical 
   *   event system MUST send the cold/warm start notification. Alert event 
   *   could not be reported as a Trap or SYSLOG message and MUST be stored in 
   *   the internal log file. The code of this event MUST be saved in 
   *   non-volatile memory and reported later through docsIfCmStatusCode SNMP 
   *   variable [RFC-2670].
   *   
   *   Critical event (priority 3)
   *   A serious failure that requires attention and prevents the device from 
   *   transmitting data but could be recovered without rebooting the system. 
   *   After recovering from the error event Cable Modem Device MUST send the 
   *   Link Up notification. Critical events could not be reported as a Trap or 
   *   SYSLOG message and MUST be stored in the internal log file. The code of 
   *   this event MUST be reported later through docsIfCmStatusCode SNMP 
   *   variable [RFC-2670]. The examples of such events could be configuration
   *   file problems detected by the modem or inability to get IP address from 
   *   DHCP."
   *  
   *  I suppose this means that after reboot docsIfCmStatusCode should be the
   *  value of the last Alert priority event logged, otherwise empty.  And
   *  then it can be overwritten at runtime by a new Critical event or a new
   *  Alert event.  But if it's overwritten by a Critical event, then after
   *  reset we should still show the old Alert event code.  This is a bit
   *  unclear, but this is my best guess.
  \*/
  
  // Only do this the first time we read from NV.
  if (fCmStatusCodeTime == 0)
  {
    char AlertStatusCode[16];
    time_t AlertStatusTime = 0;
    
    EventLogEntry *pEvent = FirstEvent();
    
    while (pEvent)
    {
      if (pEvent->GetLevel() == kAlert)
        UpdateCmStatusCode (pEvent, AlertStatusCode, &AlertStatusTime);
            
      pEvent = NextEvent(pEvent);
    }
    
    memset (fCmStatusCode, 0, sizeof(fCmStatusCode));
    strncpy (fCmStatusCode, AlertStatusCode, sizeof (fCmStatusCode)-1);
    fCmStatusCodeTime = AlertStatusTime; 
  }

#if (SNMP_SUPPORT)      
  if (fpAgent)  
    fpAgent->UnlockMib();
#endif
  
  return SNMP_NO_ERROR;
}


bool EstbEventLog::ShouldWriteNonVolNow (const EventLogEntry *pEvent, bool IsNewEvent, bool IsNewText) const
{
  return EventLog::ShouldWriteNonVolNow (pEvent, IsNewEvent, IsNewText);
}


DocsisEvent *EstbEventLog::GetEvent (EventLogIdCode Id)
{
  int NumEvents = sizeof(fEvents) / sizeof(DocsisEvent);
  int Loop;
  DocsisEvent *pEvent = NULL;

  // If the base class doesn't find one, 
  for (Loop = 0; Loop < NumEvents; Loop++)
  {
    if (fEvents[Loop].Id == Id)
    {
      pEvent = &fEvents[Loop];
      break;
    }
  }
  
  // If we didn't find a match, give the base class a chance:
  if (pEvent == NULL)
    pEvent = EventLog::GetEvent (Id);
  
  return pEvent;
}


EventLogEntry *EstbEventLog::LogEventImpl (EventLogIdCode Id, EventLogSeverityLevel Level, const char *pText, EventLogPolicy Policy, bool PrintToConsole)
{
  EventLogEntry *pEvent = EventLog::LogEventImpl (Id, Level, pText, Policy, PrintToConsole);
  
  if (pEvent)
    UpdateCmStatusCode (pEvent);
    
  return pEvent;
}

EventLogEntry *EstbEventLog::LogEventImpl (EventLogIdCode Id, const char *pParam1, const char *pParam2, const char *pParam3, const char *pParam4, const char *pParam5, bool PrintToConsole)
{
  EventLogEntry *pEvent = EventLog::LogEventImpl (Id, pParam1, pParam2, pParam3, pParam4, pParam5, PrintToConsole);
  
  if (pEvent)
    UpdateCmStatusCode (pEvent);
    
  return pEvent;
}

void EstbEventLog::GetCmStatusCode (char *pCode, int MaxLen) const
{
  memset (pCode, 0, MaxLen);
  strncpy (pCode, fCmStatusCode, MaxLen-1);
}

bool EstbEventLog::UpdateCmStatusCode (const EventLogEntry *pEvent, char *pCodeStr, time_t *pTime)
{
  if (pCodeStr == NULL)
    pCodeStr = fCmStatusCode;
    
  if (pTime == NULL)
    pTime = &fCmStatusCodeTime;

  // SPECIAL CASE: handle Appendix F error codes if our Id is in that range.
  // Decode straight from the Id value.  Note that we are kind of making an
  // assumption here that any time GetEvent gets called that the event is
  // actually going to be logged, but we can get away with that because it's
  // true!
  if (((pEvent->GetId() & VENDOR_SPECIFIC_EVENT_ID_PREFIX) == 0)
  &&   ((pEvent->GetLevel() == kCritical) || (pEvent->GetLevel() == kAlert))
  &&   (pEvent->GetLastTime() > *pTime))
  {
    int Family, Major, Minor;
    
    Family = pEvent->GetId() / 1000000;
    Minor = pEvent->GetId() % 100;
    Major = (pEvent->GetId() / 100) % 10000;
    sprintf (pCodeStr, "%c%d.%d", Family, Major, Minor);
    
    SnmpLogEv << "Docsis event, adjusting CmStatusCode to " << pCodeStr << endl;
    
    *pTime = pEvent->GetLastTime();
    
    return true;
  }
  else
  {
    SnmpLogEv << "Not a critical or alert docsis event, not adjusting CmStatusCode" << endl;
    return false;
  }
}


BcmTrap *EstbEventLog::GetNewTrap (const EventLogEntry *pEvent, DocsDevTrapTypeHint EvTrap)
{
#if (SNMP_SUPPORT)
  BcmTrap *pTrap = NULL;
  
  
  // Give the base class a shot at well-known traps (eg ColdStart, etc)
  // first.
  pTrap = EventLog::GetNewTrap(pEvent, EvTrap);
  if (pTrap)
    return pTrap;
  
  // If the base class has no opinion, check for our own kinds of traps.
  SnmpLogEv << "EstbEventLog::GetNewTrap - creating trap" << endl;
    
  // if (CmSnmpAgent::Singleton().GetDocsisMode() == DOCSIS_MODE_PURE_1_0)
  //   pTrap = new docsDevTrap_10 (fpAgent, pEvent);
    
  // else switch (EvTrap)
  switch (EvTrap)
  {
    case kUnspecifiedTrap:
      SnmpLogEv << "No trap associated with pEvent event, creating no trap." << endl;
      pTrap = NULL;
      /*
      SnmpLogEv << "No recognized trap type for pEvent event, sending a 1.0 style trap." << endl;
      pTrap = new docsDevTrap_10 (fpAgent, pEvent);
      */
      break;
      
#ifdef NOTDEF_MOE
    case kDocsDevCmInitTLVUnknownTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMINITTLVUNKNOWNTRAP))
        pTrap = new docsDevCmInitTLVUnknownTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmInitTLVUnknownTrap, disabled via docsDevCmTrapControl." << endl;
      break;

      
    case kDocsDevCmDynServReqFailTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDYNSERVREQFAILTRAP))
        pTrap = new docsDevCmDynServReqFailTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmDynServReqFailTrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmDynServRspFailTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDYNSERVRSPFAILTRAP))
        pTrap = new docsDevCmDynServRspFailTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmDynServRspFailTrap, disabled via docsDevCmTrapControl." << endl;
      break;
    
    case kDocsDevCmDynServAckFailTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDYNSERVACKFAILTRAP))
        pTrap = new docsDevCmDynServAckFailTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmDynServAckFailTrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmBpiInitTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMBPIINITTRAP))
        pTrap = new docsDevCmBpiInitTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmBpiInitTrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmBPKMTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMBPKMTRAP))
        pTrap = new docsDevCmBPKMTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmBPKMTrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmDynamicSATrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDYNAMICSATRAP))
        pTrap = new docsDevCmDynamicSATrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmDynamicSATrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmDHCPFailTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDHCPFAILTRAP))
        pTrap = new docsDevCmDHCPFailTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmDHCPFailTrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmSwUpgradeInitTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMSWUPGRADEINITTRAP))
        pTrap = new docsDevCmSwUpgradeInitTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmSwUpgradeInitTrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmSwUpgradeFailTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMSWUPGRADEFAILTRAP))
        pTrap = new docsDevCmSwUpgradeFailTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmSwUpgradeFailTrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmSwUpgradeSuccessTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMSWUPGRADESUCCESSTRAP))
        pTrap = new docsDevCmSwUpgradeSuccessTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmSwUpgradeSuccessTrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmSwUpgradeCVCFailTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMSWUPGRADECVCTRAP))
        pTrap = new docsDevCmSwUpgradeCVCFailTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmSwUpgradeCVCFailTrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmTODFailTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMTODFAILTRAP))
        pTrap = new docsDevCmTODFailTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmTODFailTrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmDCCReqFailTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDCCREQFAILTRAP))
        pTrap = new docsDevCmDCCReqFailTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmDCCReqFailTrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmDCCRspFailTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDCCRSPFAILTRAP))
        pTrap = new docsDevCmDCCRspFailTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmDCCRspFailTrap, disabled via docsDevCmTrapControl." << endl;
      break;
      
    case kDocsDevCmDCCAckFailTrap:
      if (GetTrapControl() & (1 << CONSTVAL_DOCSDEVCMTRAPCONTROL_CMDCCACKFAILTRAP))
        pTrap = new docsDevCmDCCAckFailTrap (fpAgent, pEvent);
      else
        SnmpLogEv << "Not sending docsDevCmDCCAckFailTrap, disabled via docsDevCmTrapControl." << endl;
      break;
    
    case kDsgIfStdUpstreamEnabledNotifyTrap:
      pTrap = new dsgIfStdUpstreamEnabledNotify (fpAgent, pEvent);
      break;
    
    case kDsgIfStdUpstreamDisabledNotifyTrap:
      pTrap = new dsgIfStdUpstreamEnabledNotify (fpAgent, pEvent);
      break;
    
    case kDsgIfStdTdsg2TimeoutNotifyTrap:
      pTrap = new dsgIfStdUpstreamEnabledNotify (fpAgent, pEvent);
      break;

#endif // NOTDEF_MOE
    
    default:
      SnmpLogEv << "Unsupported trap type " << (int)EvTrap << endl;
      pTrap = NULL;
      break;
  }
  
  return pTrap;
#else
  return NULL;
#endif
}


docsDevSyslog *EstbEventLog::GetNewSyslog (EventLogEntry *pEvent, int SrcIf)
{
  return new docsDevSyslog (pEvent, SrcIf);
}
