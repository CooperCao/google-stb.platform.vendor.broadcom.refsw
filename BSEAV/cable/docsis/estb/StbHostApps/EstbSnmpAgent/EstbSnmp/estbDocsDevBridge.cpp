//****************************************************************************
//
// Copyright (c) 2007-2009 Broadcom Corporation
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
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: estbDocsDevBridge.cpp
//    Creation Date: 15 DEC 2006
//    Created by BCMIBC.EXE version 1.1.0b from input file rfc2669.mib
//
//**************************************************************************

#include <stdio.h>
#include <string.h>

#include "IpAddress.h"
#include "IpAddressList.h"

// This has to be before the others because of the type conflicts.
#include "NonVolDevice.h"
#include "EstbAppVersionInfo.h"
#include "SnmpNonVolSettings.h"
//#include "EstbAppNonVolSettings.h"

#include "MibObjs.h"
#include "SnmpLog.h"
#include "estbDocsDevBridge.h"
#include "ifBridge.h"

#include "NmAccess.h"
#include "IpFilter.h"

#include "Event.h"

#include "SnmpAgnt.h"
#include "EventLog.h"
#include "EstbEvents.h"

#include "SnmpApi.h"
#include "SystemTime.h"

/*\
 *
 *  docsDevSoftwareGroupBridge for bridging scalars from the docsDevSoftwareGroup group 
 *
\*/

/* 
<No description available.>
*/ 

/*\ 
 *  construction / destruction
\*/ 

// docsDevSoftwareGroupBridge::docsDevSoftwareGroupBridge (BcmCmDocsisCtlThread *pFriend, BcmSnmpAgent *pAgent)
docsDevSoftwareGroupBridge::docsDevSoftwareGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge ("1.3.6.1.2.1.69.1.3", pAgent)
{
    // Just store this for future reference.
    // pfFriend = pFriend;
}

docsDevSoftwareGroupBridge::~docsDevSoftwareGroupBridge ()
{
    // Just forget my pointer.
    // pfFriend = NULL;
}

/*\ 
 *  GET methods
\*/

/* 
The address of the TFTP server used for software upgrades.
If the TFTP server is unknown, return 0.0.0.0."
*/ 
void docsDevSoftwareGroupBridge::Get_docsDevSwServer (BcmIpAddress &docsDevSwServer)
{
  // If we don't know nothin', then set it to 0's.
  docsDevSwServer.Set(0,0,0,0);
    
  BcmSnmpNonVolSettings *pSnmpNonVolSettings = BcmSnmpNonVolSettings::GetSingletonInstance();

  // If we are in the process of downloading, then the real IP address is
  // the one in non-vol settings.
  if ((pSnmpNonVolSettings != NULL) &&
      ((pSnmpNonVolSettings->DocsDevSwOperStatus() == CONSTVAL_DOCSDEVSWOPERSTATUS_INPROGRESS) ||
       (pSnmpNonVolSettings->DocsDevSwOperStatus() == CONSTVAL_DOCSDEVSWOPERSTATUS_FAILED) ||
       (pSnmpNonVolSettings->DocsDevSwOperStatus() == CONSTVAL_DOCSDEVSWOPERSTATUS_OTHER)))
  {
      docsDevSwServer = pSnmpNonVolSettings->DocsDevSwServer();
  }
}


/* 
The file name of the software image to be loaded into this
device. Unless set via SNMP, this is the file name
specified by the provisioning server that corresponds to
the software version that is desired for this device.
If unknown, the string '(unknown)' is returned."
*/ 
int docsDevSoftwareGroupBridge::Get_docsDevSwFilename (BcmString &docsDevSwFilename)
{
  bool wasFound = false;

  // Pre-set this with string terminators.
  docsDevSwFilename = "";

  BcmSnmpNonVolSettings *pSnmpNonVolSettings = BcmSnmpNonVolSettings::GetSingletonInstance();

  // If we are in the process of downloading, then the real filename is
  // the one in non-vol settings.
  if (pSnmpNonVolSettings != NULL) {
    docsDevSwFilename = pSnmpNonVolSettings->DocsDevSwFilename();
    wasFound = true;
  }

  if (wasFound == false) {
    // If we don't know nothin', then set it to "(unknown)".
    docsDevSwFilename = "(unknown)";
  }
  return docsDevSwFilename.length();
}


/* 
If set to upgradeFromMgt(1), the device will initiate a
TFTP software image download using docsDevSwFilename.
After successfully receiving an image, the device will
set its state to ignoreProvisioningUpgrade(3) and reboot.
If the download process is interrupted by a reset or
power failure, the device will load the previous image
and, after re-initialization, continue to attempt loading
the image specified in docsDevSwFilename.
If set to allowProvisioningUpgrade(2), the device will
use the software version information supplied by the
provisioning server when next rebooting (this does not
cause a reboot).
When set to ignoreProvisioningUpgrade(3), the device
will disregard software image upgrade information from the
provisioning server.
Note that reading this object can return upgradeFromMgt(1).
This indicates that a software download is currently in
progress, and that the device will reboot after
successfully receiving an image.
At initial startup, this object has the default value of
allowProvisioningUpgrade(2)."
*/ 
int docsDevSoftwareGroupBridge::Get_docsDevSwAdminStatus ()
{
    int status = CONSTVAL_DOCSDEVSWADMINSTATUS_ALLOWPROVISIONINGUPGRADE;

    // This is always in the non vol settings, if there is one.
    BcmSnmpNonVolSettings *pSnmpNonVolSettings = BcmSnmpNonVolSettings::GetSingletonInstance();

    if (pSnmpNonVolSettings != NULL)
    {
        status = pSnmpNonVolSettings->DocsDevSwAdminStatus();
    }

    return status;
}


/* 
InProgress(1) indicates that a TFTP download is underway,
either as a result of a version mismatch at provisioning
or as a result of a upgradeFromMgt request.
CompleteFromProvisioning(2) indicates that the last
software upgrade was a result of version mismatch at
provisioning. CompleteFromMgt(3) indicates that the last
software upgrade was a result of setting
docsDevSwAdminStatus to upgradeFromMgt.
Failed(4) indicates that the last attempted download
failed, ordinarily due to TFTP timeout."
*/ 
int docsDevSoftwareGroupBridge::Get_docsDevSwOperStatus ()
{
    int status = CONSTVAL_DOCSDEVSWOPERSTATUS_OTHER;

    // This is always in the non vol settings, if there is one.
    BcmSnmpNonVolSettings *pSnmpNonVolSettings = BcmSnmpNonVolSettings::GetSingletonInstance();

    if (pSnmpNonVolSettings != NULL)
    {
        // fShadowDocsDevSwOperStatus is provided to handle a specific corner
        // case in MAC-14 (SW download).  The problem is, we finish the
        // download, set docsDevSwOperStatus to completeFromProvisioning, and
        // reboot.  Sadly, we have seen a case where the test script comes
        // along and queries docsDevSwOperStatus in that tiny window between
        // the set and the reboot, and thinks we're back up again.  Then we
        // reboot and start timing out, and fail the test.  So, this shadow
        // variable provides a mechanism to return a value that's different
        // from what's in non-vol to avoid that problem.
        status = pSnmpNonVolSettings->ShadowDocsDevSwOperStatus();
    }

    return status;
}


/* 
The software version currently operating in this device.
This object should be in the syntax used by the individual
vendor to identify software versions.  Any eSTB MUST return a
string descriptive of the current software load.  
*/ 
int docsDevSoftwareGroupBridge::Get_docsDevSwCurrentVers (BcmString &docsDevSwCurrentVers)
{
    BcmSnmpNonVolSettings *pSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
    

    // Get this from non-vol if available.  If it has an empty string in there,
    // just return the default version from the BcmCmApplication object.
    if ((pSettings != NULL)
    &&  ((pSettings->DocsDevSwCurrentVers())[0] != 0))
      docsDevSwCurrentVers = pSettings->DocsDevSwCurrentVers();
    else
      docsDevSwCurrentVers = gGetVersionString();
      
    return docsDevSwCurrentVers.length();
}


/*\ 
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\ 
 *  SET methods
\*/

/* 
The address of the TFTP server used for software upgrades.
If the TFTP server is unknown, return 0.0.0.0."
*/ 
SNMP_STATUS docsDevSoftwareGroupBridge::Set_docsDevSwServer (const BcmIpAddress &docsDevSwServer)
{
  SNMP_STATUS Status = IpAddrMibObject::TestDefaults (docsDevSwServer);

  if (Status == SNMP_NO_ERROR)
  {
    // This is always in the non vol settings, if there is one.
    BcmSnmpNonVolSettings *pSnmpNonVolSettings = BcmSnmpNonVolSettings::GetSingletonInstance();

    // Should this be rejected if the oper status is "inProgress"?  The spec is
    // silent...  We will reject it.
    if ((pSnmpNonVolSettings == NULL) ||
        (pSnmpNonVolSettings->ShadowDocsDevSwOperStatus() == CONSTVAL_DOCSDEVSWOPERSTATUS_INPROGRESS))
    {
        return SNMP_NOT_WRITABLE;
    }

    // This should get stuffed into the CmConfig member.  This allows future
    // queries to pull from there.
    // pfFriend->fProvCmConfig.fSwUpgradeServerIpAddr = docsDevSwServer;
    // pfFriend->fProvCmConfig.fSwUpgradeServerIpAddrSpecified = true;

    // PR2920 - Also update the value in nonvol.
    if (pSnmpNonVolSettings != NULL)
    {
        pSnmpNonVolSettings->DocsDevSwServer(docsDevSwServer);
    }

    return SNMP_NO_ERROR;
  }

  return Status;
}


/* 
The file name of the software image to be loaded into this
device. Unless set via SNMP, this is the file name
specified by the provisioning server that corresponds to
the software version that is desired for this device.
If unknown, the string '(unknown)' is returned."
*/ 
SNMP_STATUS docsDevSoftwareGroupBridge::Set_docsDevSwFilename (const BcmString &docsDevSwFilename)
{
  // This is always in the non vol settings, if there is one.
    BcmSnmpNonVolSettings *pSnmpNonVolSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
    
    
    // Restrict filename length to 64 bytes per RFC2669
    if (docsDevSwFilename.length() > MAXLEN_DOCSDEVSWFILENAME)
      return SNMP_WRONG_LENGTH;

    // Should this be rejected if the oper status is "inProgress"?  The spec is
    // silent...  We will reject it.
    if ((pSnmpNonVolSettings == NULL) ||
        (pSnmpNonVolSettings->ShadowDocsDevSwOperStatus() == CONSTVAL_DOCSDEVSWOPERSTATUS_INPROGRESS))
    {
        return SNMP_NOT_WRITABLE;
    }

    // This should get stuffed into the CmConfig member.  This allows future
    // queries to pull from there.
    // pfFriend->fProvCmConfig.fSwFilename.Empty();
    // pfFriend->fProvCmConfig.fSwFilename.AddToEnd((uint8 *) docsDevSwFilename.c_str(), docsDevSwFilename.length() + 1);
    
    return SNMP_NO_ERROR;
}


/* 
If set to upgradeFromMgt(1), the device will initiate a
TFTP software image download using docsDevSwFilename.
After successfully receiving an image, the device will
set its state to ignoreProvisioningUpgrade(3) and reboot.
If the download process is interrupted by a reset or
power failure, the device will load the previous image
and, after re-initialization, continue to attempt loading
the image specified in docsDevSwFilename.
If set to allowProvisioningUpgrade(2), the device will
use the software version information supplied by the
provisioning server when next rebooting (this does not
cause a reboot).
When set to ignoreProvisioningUpgrade(3), the device
will disregard software image upgrade information from the
provisioning server.
Note that reading this object can return upgradeFromMgt(1).
This indicates that a software download is currently in
progress, and that the device will reboot after
successfully receiving an image.
At initial startup, this object has the default value of
allowProvisioningUpgrade(2)."
*/ 
SNMP_STATUS docsDevSoftwareGroupBridge::Set_docsDevSwAdminStatus (int docsDevSwAdminStatus)
{
#ifdef NOTDEF
  // This is always in the non vol settings, if there is one.
  BcmSnmpNonVolSettings *pSnmpNonVolSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  BcmCmDocsisNonVolSettings *pDocsisSettings = BcmCmDocsisNonVolSettings::GetSingletonInstance();

  if ((pSnmpNonVolSettings == NULL) 
  ||  (pDocsisSettings == NULL))
      return SNMP_RESOURCE_UNAVAILABLE;

  // Should this be rejected if the oper status is "inProgress"?  The spec is
  // silent...  We will reject it.      
  if (pSnmpNonVolSettings->ShadowDocsDevSwOperStatus() == CONSTVAL_DOCSDEVSWOPERSTATUS_INPROGRESS)
      return SNMP_INCONSISTENT_VALUE;

  // If they are kicking the process off, then start software upgrade.
  if ((docsDevSwAdminStatus != CONSTVAL_DOCSDEVSWADMINSTATUS_ALLOWPROVISIONINGUPGRADE)
  &&  (docsDevSwAdminStatus != CONSTVAL_DOCSDEVSWADMINSTATUS_IGNOREPROVISIONINGUPGRADE)
  &&  (docsDevSwAdminStatus != CONSTVAL_DOCSDEVSWADMINSTATUS_UPGRADEFROMMGT))
    return SNMP_WRONG_VALUE;

  // Check for CVC only if we have DOCSIS 1.1 and BPI23 enabled in NV
  if (docsDevSwAdminStatus == CONSTVAL_DOCSDEVSWADMINSTATUS_UPGRADEFROMMGT)
  {
      bool result;
      bool useSecureDload = false;
      BcmIpAddress ipAddress;
      BcmCmDocsisNonVolSettings *pDocsisSettings = BcmCmDocsisNonVolSettings::GetSingletonInstance();
      unsigned int imageNumber = 0;  // default image.
      const char *pSwFilename;      // PRxxx (not yet assigned)

      
        if (pfFriend->fDocsCodeUpgradeEnabled == false)
        {
          CmLogEvent (kE207_0);
          return SNMP_INCONSISTENT_VALUE;
        }
          
        useSecureDload = true;

      // PRxxxx (not yet assigned) - it is possible that the s/w filename is
      // not specified anywhere, in which case we need to set it to an empty
      // string, since the buffer that would contain the string in not
      // initialized as a string.
      if (!pfFriend->fProvCmConfig.SwFilenameSpecified())
      {
          // PR2245 - need to use the value in nonvol if not specified in the
          // config file.
          pSwFilename = pSnmpNonVolSettings->DocsDevSwFilename();
      }
      else
      {
          // Note that is somebody sets docsDevSwFilename via SNMP, then it
          // overwrites the value from the config file.
          pSwFilename = (const char *) pfFriend->fProvCmConfig.fSwFilename.AccessBuffer();
      }

      // Get the server IP address that we are supposed to use.
      // Start the helper object...Only specify 1 try; we only
      // retry if the image is detected to be corrupt in some
      // fashion, and that generally won't happen.  Even so,
      // retrying is optional.
      //
      // PR1769 changed the API (don't specify the event any more).
      Get_docsDevSwServer(ipAddress);
      result = pfFriend->pfIpHelperThd->StartSoftwareUpdate(pSwFilename, 
                                                            ipAddress, 
                                                            true, // started by mgmt
                                                            imageNumber,
                                                            useSecureDload);

      if (result == false)
          return SNMP_GEN_ERR;
  }
  else
  {
      BcmNonVolDevice *pNonVolDevice;
      bool result;

      // See if a NonVol Device is available on this target.
      pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();

      if (pNonVolDevice == NULL)
      {
          // Is this the correct return code?
          return SNMP_RESOURCE_UNAVAILABLE;
      }

      // Store the value, and write it to the device.
      pSnmpNonVolSettings->DocsDevSwAdminStatus(docsDevSwAdminStatus);

      result = pNonVolDevice->Write(*pSnmpNonVolSettings);

      if (result == false)
      {
          // Is this the correct return code?
          return SNMP_GEN_ERR;
      }
  }
    
  return SNMP_NO_ERROR;
#else
      return SNMP_RESOURCE_UNAVAILABLE;
#endif
}


/* 
The software version currently operating in this device.
This object should be in the syntax used by the individual
vendor to identify software versions.  Any CM MUST return a
string descriptive of the current software load.  For a
CMTS, this object SHOULD contain either a human readable
representation of the vendor specific designation of the
software for the chassis, or of the software for the
control processor. If neither of these is  applicable,
this MUST contain an empty string."
*/ 
SNMP_STATUS docsDevSoftwareGroupBridge::Set_docsDevSwCurrentVers (const BcmString &docsDevSwCurrentVers)
{
  // This is always in the non vol settings, if there is one.
  BcmSnmpNonVolSettings *pSettings = BcmSnmpNonVolSettings::GetSingletonInstance();
  
  
  if (pSettings == NULL)
    return SNMP_NOT_WRITABLE;
  
  if (pSettings->AllowConfiguration() == false)
    return SNMP_NOT_WRITABLE;

  SNMP_STATUS Status = StringMibObject::TestDefaults (docsDevSwCurrentVers);

  if (Status == SNMP_NO_ERROR)
  {
    pSettings->DocsDevSwCurrentVers(docsDevSwCurrentVers.c_str());
    fpAgent->WriteNonVolSettings();
    
    return SNMP_NO_ERROR;
  }

  return Status;
}

/*\
 *
 *  docsDevFilterGroupBridge for bridging scalars from the docsDevFilterGroup group
 *
\*/

/*
<No description available.>
*/

/*\
 *  construction / destruction
\*/

docsDevFilterGroupBridge::docsDevFilterGroupBridge (BcmSnmpAgent *pAgent)
  : ScalarMibBridge ("1.3.6.1.2.1.69.1.6", pAgent)
{
}

docsDevFilterGroupBridge::~docsDevFilterGroupBridge ()
{
}

/*\
 *  GET methods
\*/

/*
If set to discard(1), all packets not matching an IP filter
will be discarded. If set to accept(2), all packets not
matching an IP filter will be accepted for further
processing (e.g., bridging).
At initial system startup, this object returns accept(2)."
*/
int docsDevFilterGroupBridge::Get_docsDevFilterIpDefault ()
{
  return IpFilter::GetDefault();
}


/*\
 *  TEST methods - not required because this is not a READ-CREATE table
\*/

/*\
 *  SET methods
\*/

/*
If set to discard(1), all packets not matching an IP filter
will be discarded. If set to accept(2), all packets not
matching an IP filter will be accepted for further
processing (e.g., bridging).
At initial system startup, this object returns accept(2)."
*/
SNMP_STATUS docsDevFilterGroupBridge::Set_docsDevFilterIpDefault (int docsDevFilterIpDefault)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevFilterIpDefault);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevFilterIpDefault != CONSTVAL_DOCSDEVFILTERIPDEFAULT_DISCARD)
    &&  (docsDevFilterIpDefault != CONSTVAL_DOCSDEVFILTERIPDEFAULT_ACCEPT))
      return SNMP_WRONG_VALUE;

    return IpFilter::SetDefault(docsDevFilterIpDefault);
  }

  return Status;
}





/*\
 *
 *  docsDevFilterIpEntryBridge for bridging elements in the docsDevFilterIpTable
 *
\*/

/*
An ordered list of filters or classifiers to apply to
IP traffic. Filter application is ordered by the filter
index, rather than by a best match algorithm (Note that
this implies that the filter table may have gaps in the
index values). Packets which match no filters will have
policy 0 in the docsDevFilterPolicyTable applied to them if
it exists. Otherwise, Packets which match no filters
are discarded or forwarded according to the setting of
docsDevFilterIpDefault.
Any IP packet can theoretically match multiple rows of
this table.  When considering a packet, the table is
scanned in row index order (e.g. filter 10 is checked
before filter 20).  If the packet matches that filter
(which means that it matches ALL criteria for that row),
actions appropriate to docsDevFilterIpControl and
docsDevFilterPolicyId are taken.  If the packet was
discarded processing is complete.  If
docsDevFilterIpContinue is set to true, the filter
comparison continues with the next row in the table
looking for additional matches.
If the packet matches no filter in the table, the packet
is accepted or dropped for further processing based on
the setting of docsDevFilterIpDefault. If the packet is
accepted, the actions specified by policy group 0
(e.g. the rows in docsDevFilterPolicyTable which have a
value of 0 for docsDevFilterPolicyId) are taken if that
policy group exists.
Logically, this table is consulted twice during the
processing of any IP packet - once upon its acceptance
from the L2 entity, and once upon its transmission to the
L2 entity.  In actuality, for cable modems, IP filtering
is generally the only IP processing done for transit
traffic.  This means that inbound and outbound filtering
can generally be done at the same time with one pass
through the filter table."

INDEX { docsDevFilterIpIndex }
*/

/*\
 *  construction / destruction
\*/

docsDevFilterIpEntryBridge::docsDevFilterIpEntryBridge (int Index, BcmSnmpAgent *pAgent)
  : TabularMibBridge ("1.3.6.1.2.1.69.1.6.4", Index, pAgent)
{
  fpFilter = new IpFilter (Index);

  // Find the first CPE interface and set the filter's index to that.
  if (fpAgent)
  {
    ifEntryBridge *pIf = fpAgent->IfBridge().FirstIfEntry();

    while (pIf)
    {
      BcmString Descr;
      pIf->Get_ifDescr(Descr);

      // In our bridging model, a CPE interface is defined as any
      // interface that's not the default one, except for the IP
      // stack.  Possibly we could have multiple CPE interfaces, so
      // we'll just use the first one we find.
      if ((pIf->IsDefaultInterface() == false)
      &&  (pIf->Get_ifType() != BcmHalIf::kIanaTypeIpStack)
      &&  (pIf->Get_ifType() != BcmHalIf::kIanaTypeSoftwareLoopback))
      {
        fpFilter->SetIfIndex (pIf->Get_ifIndex());
        break;
      }

      pIf = fpAgent->IfBridge().NextIfEntry(pIf->Get_ifIndex());
    }
  }

  // If we are building this IP filter before all the HAL interfaces
  // get built and registered, then we won't have a CPE interface yet.
  // In that case, hard-code the interface to 1, which by DOCSIS convention
  // must point to the main CPE interface anyway.
  if (fpFilter->GetIfIndex() == 0)
    fpFilter->SetIfIndex (1);
}

docsDevFilterIpEntryBridge::~docsDevFilterIpEntryBridge ()
{
  delete fpFilter;
}

/*\
 *  GET methods
\*/

/*
Index used to order the application of filters.
The filter with the lowest index is always applied
first."
*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpIndex ()
{
  return fpFilter->GetIndex();
}


/*
Controls and reflects the status of rows in this
table. Specifying only this object (with the appropriate
index) on a CM is sufficient to create a filter row which
matches all inbound packets on the ethernet interface,
and results in the packets being
discarded. docsDevFilterIpIfIndex (at least) must be
specified on a CMTS to create a row.  Creation of the
rows may be done via either create-and-wait or
create-and-go, but the filter is not applied until this
object is set to (or changes to) active. There is no
restriction in changing any object in a row while this
object is set to active."
*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpStatus ()
{
  return fpFilter->GetStatus ();
}


/*
If set to discard(1), all packets matching this filter
will be discarded and scanning of the remainder of the
filter list will be aborted. If set to accept(2), all
packets matching this filter will be accepted for further
processing (e.g., bridging). If docsDevFilterIpContinue
is set to true, see if there are other matches, otherwise
done. If set to policy (3), execute the policy entries
matched by docsDevIpFilterPolicyId in
docsDevIpFilterPolicyTable.
If is docsDevFilterIpContinue is set to true, continue
scanning the table for other matches, otherwise done."

DEFVAL { discard }

*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpControl ()
{
  return fpFilter->GetControl();
}


/*
The entry interface to which this filter applies. The
value corresponds to ifIndex for either a CATV MAC or
another network interface. If the value is zero, the
filter applies to all interfaces. Default value in Cable
Modems is the index of the customer-side (e.g. ethernet)
interface. In Cable Modem Termination Systems, this
object MUST be specified to create a row in this table."

-- DEFVAL added for V2 mib compiler. Not required for row creation.

*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpIfIndex ()
{
  return fpFilter->GetIfIndex();
}


/*
Determines whether the filter is applied to inbound(1)
traffic, outbound(2) traffic, or traffic in both(3)
directions."

DEFVAL { inbound }

*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpDirection ()
{
  return fpFilter->GetDirection();
}


/*
If set to true(1), the filter only applies to multicast
and broadcast traffic. If set to false(2), the filter
applies to all traffic."

DEFVAL { false }

*/
bool docsDevFilterIpEntryBridge::Get_docsDevFilterIpBroadcast ()
{
  return fpFilter->GetBroadcast();
}


/*
The source IP address, or portion thereof, that is to be
matched for this filter.  The source address is first
masked (and'ed) against docsDevFilterIpSmask before being
compared  to this value.  A value of 0 for this object
and 0 for the mask matches all IP addresses."

DEFVAL { '00000000'h }

*/
void docsDevFilterIpEntryBridge::Get_docsDevFilterIpSaddr (BcmIpAddress &docsDevFilterIpSaddr)
{
  docsDevFilterIpSaddr = fpFilter->GetSrcIp();
}


/*
A bit mask that is to be applied to the source address
prior to matching. This mask is not necessarily the same
as a subnet mask, but 1's bits must be leftmost and
contiguous."

DEFVAL { '00000000'h }

*/
void docsDevFilterIpEntryBridge::Get_docsDevFilterIpSmask (BcmIpAddress &docsDevFilterIpSmask)
{
  docsDevFilterIpSmask = fpFilter->GetSrcMask();
}


/*
The destination IP address, or portion thereof, that is
to be matched for this filter. The destination address is
first masked (and'ed) against docsDevFilterIpDmask before
being compared  to this value.  A value of 0 for this
object and 0 for the mask matches all IP addresses."

DEFVAL { '00000000'h }

*/
void docsDevFilterIpEntryBridge::Get_docsDevFilterIpDaddr (BcmIpAddress &docsDevFilterIpDaddr)
{
  docsDevFilterIpDaddr = fpFilter->GetDstIp();
}


/*
A bit mask that is to be applied to the destination
address prior to matching. This mask is not necessarily
the same as a subnet mask, but 1's bits must be leftmost
and contiguous."

DEFVAL { '00000000'h }

*/
void docsDevFilterIpEntryBridge::Get_docsDevFilterIpDmask (BcmIpAddress &docsDevFilterIpDmask)
{
  docsDevFilterIpDmask = fpFilter->GetDstMask();
}


/*
The IP protocol value that is to be matched. For example:
icmp is 1, tcp is 6, udp is 17. A value of 256 matches
ANY protocol."

DEFVAL { 256 }

*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpProtocol ()
{
  return fpFilter->GetProtocol();
}


/*
If docsDevFilterIpProtocol is udp or tcp, this is the
inclusive lower bound of the transport-layer source port
range that is to be matched, otherwise it is ignored
during matching."

DEFVAL { 0 }

*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpSourcePortLow ()
{
  return fpFilter->GetSrcPortLo();
}


/*
If docsDevFilterIpProtocol is udp or tcp, this is the
inclusive upper bound of the transport-layer source port
range that is to be matched, otherwise it is ignored
during matching."

DEFVAL { 65535 }

*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpSourcePortHigh ()
{
  return fpFilter->GetSrcPortHi();
}


/*
If docsDevFilterIpProtocol is udp or tcp, this is the
inclusive lower bound of the transport-layer destination
port range that is to be matched, otherwise it is ignored
during matching."

DEFVAL { 0 }

*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpDestPortLow ()
{
  return fpFilter->GetDstPortLo();
}


/*
If docsDevFilterIpProtocol is udp or tcp, this is the
inclusive upper bound of the transport-layer destination
port range that is to be matched, otherwise it is ignored
during matching."

DEFVAL { 65535 }

*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpDestPortHigh ()
{
  return fpFilter->GetDstPortHi();
}


/*
Counts the number of times this filter was matched.
This object is initialized to 0 at boot, or at row
creation, and is reset only upon reboot."
*/
unsigned int docsDevFilterIpEntryBridge::Get_docsDevFilterIpMatches ()
{
  return fpFilter->GetMatches();
}


/*
This is the value to be matched to the packet's
TOS (Type of Service) value (after the TOS value
is AND'd with docsDevFilterIpTosMask).  A value for this
object of 0 and a mask of 0 matches all TOS values."

DEFVAL { '00'h }

*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpTos (BcmString &docsDevFilterIpTos)
{
  unsigned char Tos = fpFilter->GetTos();
  docsDevFilterIpTos.assign ((char *)&Tos, 1);

  return 1;
}


/*
The mask to be applied to the packet's TOS value before
matching."

DEFVAL { '00'h }

*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpTosMask (BcmString &docsDevFilterIpTosMask)
{
  unsigned char Tos = fpFilter->GetTos();
  docsDevFilterIpTosMask.assign ((char *)&Tos, 1);

  return 1;
}


/*
If this value is set to true, and docsDevFilterIpControl
is anything but discard (1), continue scanning and
applying policies."

DEFVAL { false }

*/
bool docsDevFilterIpEntryBridge::Get_docsDevFilterIpContinue ()
{
  return fpFilter->GetContinue();
}


/*
This object points to an entry in docsDevFilterPolicyTable.
If docsDevFilterIpControl is set to policy (3), execute
all matching policies in docsDevFilterPolicyTable.
If no matching policy exists, treat as if
docsDevFilterIpControl were set to accept (1).
If this object is set to the value of 0, there is no
matching policy, and docsDevFilterPolicyTable MUST NOT be
consulted."

DEFVAL { 0 }

*/
int docsDevFilterIpEntryBridge::Get_docsDevFilterIpPolicyId ()
{
  return fpFilter->GetPolicyId();
}


/*\
 *  TEST methods
\*/

/*
Controls and reflects the status of rows in this
table. Specifying only this object (with the appropriate
index) on a CM is sufficient to create a filter row which
matches all inbound packets on the ethernet interface,
and results in the packets being
discarded. docsDevFilterIpIfIndex (at least) must be
specified on a CMTS to create a row.  Creation of the
rows may be done via either create-and-wait or
create-and-go, but the filter is not applied until this
object is set to (or changes to) active. There is no
restriction in changing any object in a row while this
object is set to active."
*/
SNMP_STATUS docsDevFilterIpEntryBridge::Test_docsDevFilterIpStatus (int docsDevFilterIpStatus)
{
  SNMP_STATUS Status = RowStatusMibObject::TestDefaults (docsDevFilterIpStatus, Get_docsDevFilterIpStatus());

  if (Status == SNMP_NO_ERROR)
  {
    fObjectMask |= kBIT_docsDevFilterIpStatus;
  }

  return Status;
}


/*\
 *  SET methods
\*/

/*
Controls and reflects the status of rows in this
table. Specifying only this object (with the appropriate
index) on a CM is sufficient to create a filter row which
matches all inbound packets on the ethernet interface,
and results in the packets being
discarded. docsDevFilterIpIfIndex (at least) must be
specified on a CMTS to create a row.  Creation of the
rows may be done via either create-and-wait or
create-and-go, but the filter is not applied until this
object is set to (or changes to) active. There is no
restriction in changing any object in a row while this
object is set to active."
*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpStatus (int &docsDevFilterIpStatus)
{
  SNMP_STATUS Status = SetRowStatus (docsDevFilterIpStatus);

  if (Status == SNMP_NO_ERROR)
  {
    fObjectMask |= kBIT_docsDevFilterIpStatus;
    return fpFilter->SetStatus (docsDevFilterIpStatus);
  }

  return Status;
}


/*
If set to discard(1), all packets matching this filter
will be discarded and scanning of the remainder of the
filter list will be aborted. If set to accept(2), all
packets matching this filter will be accepted for further
processing (e.g., bridging). If docsDevFilterIpContinue
is set to true, see if there are other matches, otherwise
done. If set to policy (3), execute the policy entries
matched by docsDevIpFilterPolicyId in
docsDevIpFilterPolicyTable.
If is docsDevFilterIpContinue is set to true, continue
scanning the table for other matches, otherwise done."

DEFVAL { discard }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpControl (int docsDevFilterIpControl)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevFilterIpControl);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevFilterIpControl != CONSTVAL_DOCSDEVFILTERIPCONTROL_DISCARD)
    &&  (docsDevFilterIpControl != CONSTVAL_DOCSDEVFILTERIPCONTROL_ACCEPT)
    &&  (docsDevFilterIpControl != CONSTVAL_DOCSDEVFILTERIPCONTROL_POLICY))
      return SNMP_WRONG_VALUE;

    return fpFilter->SetControl(docsDevFilterIpControl);
  }

  return Status;
}


/*
The entry interface to which this filter applies. The
value corresponds to ifIndex for either a CATV MAC or
another network interface. If the value is zero, the
filter applies to all interfaces. Default value in Cable
Modems is the index of the customer-side (e.g. ethernet)
interface. In Cable Modem Termination Systems, this
object MUST be specified to create a row in this table."

-- DEFVAL added for V2 mib compiler. Not required for row creation.

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpIfIndex (int docsDevFilterIpIfIndex)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevFilterIpIfIndex);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevFilterIpIfIndex != 0)
    &&  (fpAgent->IfBridge().FindIfEntry (docsDevFilterIpIfIndex) == NULL))
      return SNMP_WRONG_VALUE;

    return fpFilter->SetIfIndex(docsDevFilterIpIfIndex);
  }

  return Status;
}


/*
Determines whether the filter is applied to inbound(1)
traffic, outbound(2) traffic, or traffic in both(3)
directions."

DEFVAL { inbound }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpDirection (int docsDevFilterIpDirection)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevFilterIpDirection);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevFilterIpDirection != CONSTVAL_DOCSDEVFILTERIPDIRECTION_INBOUND)
    &&  (docsDevFilterIpDirection != CONSTVAL_DOCSDEVFILTERIPDIRECTION_OUTBOUND)
    &&  (docsDevFilterIpDirection != CONSTVAL_DOCSDEVFILTERIPDIRECTION_BOTH))
      return SNMP_WRONG_VALUE;

    return fpFilter->SetDirection(docsDevFilterIpDirection);
  }

  return Status;
}


/*
If set to true(1), the filter only applies to multicast
and broadcast traffic. If set to false(2), the filter
applies to all traffic."

DEFVAL { false }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpBroadcast (bool docsDevFilterIpBroadcast)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  if ((docsDevFilterIpBroadcast != true) && (docsDevFilterIpBroadcast != false))
    return SNMP_WRONG_VALUE;

  return fpFilter->SetBroadcast(docsDevFilterIpBroadcast);

  return Status;
}


/*
The source IP address, or portion thereof, that is to be
matched for this filter.  The source address is first
masked (and'ed) against docsDevFilterIpSmask before being
compared  to this value.  A value of 0 for this object
and 0 for the mask matches all IP addresses."

DEFVAL { '00000000'h }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpSaddr (const BcmIpAddress &docsDevFilterIpSaddr)
{
  SNMP_STATUS Status = IpAddrMibObject::TestDefaults (docsDevFilterIpSaddr);

  if (Status == SNMP_NO_ERROR)
  {
    return fpFilter->SetSrcIp(docsDevFilterIpSaddr);
  }

  return Status;
}


/*
A bit mask that is to be applied to the source address
prior to matching. This mask is not necessarily the same
as a subnet mask, but 1's bits must be leftmost and
contiguous."

DEFVAL { '00000000'h }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpSmask (const BcmIpAddress &docsDevFilterIpSmask)
{
  SNMP_STATUS Status = IpAddrMibObject::TestDefaults (docsDevFilterIpSmask);

  if (Status == SNMP_NO_ERROR)
  {
    return fpFilter->SetSrcMask(docsDevFilterIpSmask);
  }

  return Status;
}


/*
The destination IP address, or portion thereof, that is
to be matched for this filter. The destination address is
first masked (and'ed) against docsDevFilterIpDmask before
being compared  to this value.  A value of 0 for this
object and 0 for the mask matches all IP addresses."

DEFVAL { '00000000'h }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpDaddr (const BcmIpAddress &docsDevFilterIpDaddr)
{
  SNMP_STATUS Status = IpAddrMibObject::TestDefaults (docsDevFilterIpDaddr);

  if (Status == SNMP_NO_ERROR)
  {
    return fpFilter->SetDstIp(docsDevFilterIpDaddr);
  }

  return Status;
}


/*
A bit mask that is to be applied to the destination
address prior to matching. This mask is not necessarily
the same as a subnet mask, but 1's bits must be leftmost
and contiguous."

DEFVAL { '00000000'h }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpDmask (const BcmIpAddress &docsDevFilterIpDmask)
{
  SNMP_STATUS Status = IpAddrMibObject::TestDefaults (docsDevFilterIpDmask);

  if (Status == SNMP_NO_ERROR)
  {
    return fpFilter->SetDstMask(docsDevFilterIpDmask);
  }

  return Status;
}


/*
The IP protocol value that is to be matched. For example:
icmp is 1, tcp is 6, udp is 17. A value of 256 matches
ANY protocol."

DEFVAL { 256 }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpProtocol (int docsDevFilterIpProtocol)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevFilterIpProtocol);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevFilterIpProtocol < MINVAL_DOCSDEVFILTERIPPROTOCOL)
    ||  (docsDevFilterIpProtocol > MAXVAL_DOCSDEVFILTERIPPROTOCOL))
      return SNMP_WRONG_VALUE;

    return fpFilter->SetProtocol (docsDevFilterIpProtocol);
  }

  return Status;
}


/*
If docsDevFilterIpProtocol is udp or tcp, this is the
inclusive lower bound of the transport-layer source port
range that is to be matched, otherwise it is ignored
during matching."

DEFVAL { 0 }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpSourcePortLow (int docsDevFilterIpSourcePortLow)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevFilterIpSourcePortLow);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevFilterIpSourcePortLow < MINVAL_DOCSDEVFILTERIPSOURCEPORTLOW)
    ||  (docsDevFilterIpSourcePortLow > MAXVAL_DOCSDEVFILTERIPSOURCEPORTLOW))
      return SNMP_WRONG_VALUE;

    return fpFilter->SetSrcPortLo (docsDevFilterIpSourcePortLow);
  }

  return Status;
}


/*
If docsDevFilterIpProtocol is udp or tcp, this is the
inclusive upper bound of the transport-layer source port
range that is to be matched, otherwise it is ignored
during matching."

DEFVAL { 65535 }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpSourcePortHigh (int docsDevFilterIpSourcePortHigh)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevFilterIpSourcePortHigh);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevFilterIpSourcePortHigh < MINVAL_DOCSDEVFILTERIPSOURCEPORTHIGH)
    ||  (docsDevFilterIpSourcePortHigh > MAXVAL_DOCSDEVFILTERIPSOURCEPORTHIGH))
      return SNMP_WRONG_VALUE;

    return fpFilter->SetSrcPortHi(docsDevFilterIpSourcePortHigh);
  }

  return Status;
}


/*
If docsDevFilterIpProtocol is udp or tcp, this is the
inclusive lower bound of the transport-layer destination
port range that is to be matched, otherwise it is ignored
during matching."

DEFVAL { 0 }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpDestPortLow (int docsDevFilterIpDestPortLow)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevFilterIpDestPortLow);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevFilterIpDestPortLow < MINVAL_DOCSDEVFILTERIPDESTPORTLOW)
    ||  (docsDevFilterIpDestPortLow > MAXVAL_DOCSDEVFILTERIPDESTPORTLOW))
      return SNMP_WRONG_VALUE;

    return fpFilter->SetDstPortLo(docsDevFilterIpDestPortLow);
  }

  return Status;
}


/*
If docsDevFilterIpProtocol is udp or tcp, this is the
inclusive upper bound of the transport-layer destination
port range that is to be matched, otherwise it is ignored
during matching."

DEFVAL { 65535 }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpDestPortHigh (int docsDevFilterIpDestPortHigh)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevFilterIpDestPortHigh);

  if (Status == SNMP_NO_ERROR)
  {
    if ((docsDevFilterIpDestPortHigh < MINVAL_DOCSDEVFILTERIPDESTPORTHIGH)
    ||  (docsDevFilterIpDestPortHigh > MAXVAL_DOCSDEVFILTERIPDESTPORTHIGH))
      return SNMP_WRONG_VALUE;

    return fpFilter->SetDstPortHi(docsDevFilterIpDestPortHigh);
  }

  return Status;
}


/*
This is the value to be matched to the packet's
TOS (Type of Service) value (after the TOS value
is AND'd with docsDevFilterIpTosMask).  A value for this
object of 0 and a mask of 0 matches all TOS values."

DEFVAL { '00'h }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpTos (const BcmString &docsDevFilterIpTos)
{
  if (docsDevFilterIpTos.length() != 1)
    return SNMP_WRONG_LENGTH;

  return fpFilter->SetTos (docsDevFilterIpTos[0]);
}


/*
The mask to be applied to the packet's TOS value before
matching."

DEFVAL { '00'h }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpTosMask (const BcmString &docsDevFilterIpTosMask)
{
  if (docsDevFilterIpTosMask.length() != 1)
    return SNMP_WRONG_LENGTH;

  return fpFilter->SetTosMask (docsDevFilterIpTosMask[0]);
}


/*
If this value is set to true, and docsDevFilterIpControl
is anything but discard (1), continue scanning and
applying policies."

DEFVAL { false }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpContinue (bool docsDevFilterIpContinue)
{
  SNMP_STATUS Status = SNMP_NO_ERROR;

  if ((docsDevFilterIpContinue != true) && (docsDevFilterIpContinue != false))
    return SNMP_WRONG_VALUE;

  return fpFilter->SetContinue(docsDevFilterIpContinue);

  return Status;
}


/*
This object points to an entry in docsDevFilterPolicyTable.
If docsDevFilterIpControl is set to policy (3), execute
all matching policies in docsDevFilterPolicyTable.
If no matching policy exists, treat as if
docsDevFilterIpControl were set to accept (1).
If this object is set to the value of 0, there is no
matching policy, and docsDevFilterPolicyTable MUST NOT be
consulted."

DEFVAL { 0 }

*/
SNMP_STATUS docsDevFilterIpEntryBridge::Set_docsDevFilterIpPolicyId (int docsDevFilterIpPolicyId)
{
  SNMP_STATUS Status = Int32MibObject::TestDefaults (docsDevFilterIpPolicyId);

  if (Status == SNMP_NO_ERROR)
  {
    if (docsDevFilterIpPolicyId < 0)
      return SNMP_WRONG_VALUE;

    return fpFilter->SetPolicyId(docsDevFilterIpPolicyId);
  }

  return Status;
}


