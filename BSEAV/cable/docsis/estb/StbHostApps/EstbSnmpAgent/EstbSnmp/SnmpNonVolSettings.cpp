//****************************************************************************
//
//  Copyright (c) 2000-2002  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       SnmpNonVolSettings.cpp
//  Author:         Kevin O'Neal
//  Creation Date:  September 3, 2003
//
//****************************************************************************
//  Description:
//      These are the Non-Volatile settings for the SNMP Subsystem.
//      This is derived from the abstract base NonVolSettings class, which
//      provides hooks for loading and storing the settings.
//
//      Note that the settings are independant of the device to which they are
//      being stored, allowing the same application settings to be used on
//      multiple target platforms and devices!
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "SnmpNonVolSettings.h"
#include "HalIfNonVolSettings.h"
#include "PktFilterController.h"
#include "docsDevBridge.h"

#include "OperatingSystemFactory.h"

#include "MergedNonVolDefaults.h"

#if (CM_HAL_INCLUDED)
#if (SNMP_SUPPORT)
#include "CmSnmpAgent.h"
#endif
#endif

#include <string.h>
#include <iomanip.h>



//********************** Local Types *****************************************

//********************** Local Constants *************************************

// Dynamic rev = 0.6
// Permanent rev = 0.4
const uint8 kCurrentVersionMajor[2] =
{
    0, 0
};

const uint8 kCurrentVersionMinor[2] =
{
    6, 4
};


//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Set this to NULL initially.  This will be set up in my constructor.
BcmSnmpNonVolSettings *BcmSnmpNonVolSettings::pfSingletonInstance = NULL;


// Default Constructor.  Initializes the state of the object, setting all
// fields to good default values.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmSnmpNonVolSettings::BcmSnmpNonVolSettings(void) :
    BcmNonVolSettings("CM SNMP NonVol Settings")
{
    // Override the class name in the message log settings that was set by my
    // base class.
    fMessageLogSettings.SetModuleName("BcmSnmpNonVolSettings");

    fConstructorRunning = true;

    // Just let this do the work.
    ResetDefaults(kDynamicSection);
    ResetDefaults(kPermanentSection);

    fConstructorRunning = false;

    // Set myself as the singleton.
    if (pfSingletonInstance == NULL)
    {
        gInfoMsg(fMessageLogSettings, "BcmSnmpNonVolSettings")
            << "Setting up the singleton pointer." << endl;
        pfSingletonInstance = this;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "BcmSnmpNonVolSettings")
            << "Singleton pointer is not NULL!  There are multiple instances!  Leaving the singleton pointer alone..." << endl;
    }
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmSnmpNonVolSettings::~BcmSnmpNonVolSettings()
{
    // Clear the singleton pointer.
    if (pfSingletonInstance == this)
    {
        gInfoMsg(fMessageLogSettings, "~BcmSnmpNonVolSettings")
            << "Clearing the singleton pointer." << endl;

        pfSingletonInstance = NULL;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "~BcmSnmpNonVolSettings")
            << "I'm not the singleton instance!  Leaving the singleton pointer alone..." << endl;
    }
}


// Allows the derived class to set its values to a known good state.  This
// MUST be implemented by the derived class.
//
// Note that the derived class should set the version numbers to the desired
// values in case they were changed (by ReadFrom(), etc).
//
// This method may be called if ReadFrom() returns false in order to ensure
// that there are valid settings in the object.
//
// Parameters:
//      section - the section that should be reset.
//
// Returns:  Nothing.
//
void BcmSnmpNonVolSettings::ResetDefaults(NonVolSection section)
{
    // Set the magic numbers.  Ascii for 's' 'n' 'm' 'p'
    // Note that this was chosen way back when there was ONLY CM-SNMP,
    // hence the myopic choice of 'snmp' as the magic number.  It's left
    // that way for backwards-compatibility.
    fMagicNumber = 0x736e6d70;

    // Set the current version number.
    fVersionMajor[section] = kCurrentVersionMajor[section];
    fVersionMinor[section] = kCurrentVersionMinor[section];

    // PR1969 - many default values come from the customer config file now.
    if (section == kDynamicSection)
    {
        if (!fConstructorRunning)
        {
            gWarningMsg(fMessageLogSettings, "ResetDefaults")
                << "Resetting Dynamic Settings!" << endl;
        }

        fDocsDevSwAdminStatus = CONSTVAL_DOCSDEVSWADMINSTATUS_ALLOWPROVISIONINGUPGRADE;
        fDocsDevSwOperStatus = CONSTVAL_DOCSDEVSWOPERSTATUS_OTHER;

        // fShadowDocsDevSwOperStatus is provided to handle a specific corner
        // case in MAC-14 (SW download).  The problem is, we finish the
        // download, set docsDevSwOperStatus to completeFromProvisioning, and
        // reboot.  Sadly, we have seen a case where the test script comes
        // along and queries docsDevSwOperStatus in that tiny window between
        // the set and the reboot, and thinks we're back up again.  Then we
        // reboot and start timing out, and fail the test.  So, this shadow
        // variable provides a mechanism to return a value that's different
        // from what's in non-vol to avoid that problem.
        fShadowDocsDevSwOperStatus = fDocsDevSwOperStatus;

        DocsDevSwFilename("(unknown)");
        fDocsDevSwServer.Set("0.0.0.0");

        // Load up writable system info
        SysContact(NULL);
        SysName(NULL);
        SysLocation(NULL);
        
        fDownloadTries = 0;

        fSnmpEngineBoots = 0;
    }
    else
    {
        if (!fConstructorRunning)
        {
            gWarningMsg(fMessageLogSettings, "ResetDefaults")
                << "Resetting all Permanent Settings!  The serial number and other Sys Info items are probably invalid!" << endl;
        }

        fAllowConfiguration = false; 

        // Set the compiled-in value.
        VendorName(NULL);

        // System group; set the compiled-in values.
        SysDescr(NULL);
        SysObjectId(NULL);
        SysORID(NULL);
        SysORDescr(NULL);

        fSysServices = kDefaultValue_SysServices;

        // DocsDev group; set the compiled-in value.
        DocsDevSwCurrentVers(NULL);

        // This needs to be manufactured.
        DocsDevSerialNumber("");

        // The ATP wants to see 3 tries total; we think 4 tries (1 try and 3
        // retries) is what the spec really says, but the ATP is always "right".
        //
        // 2-Oct-2001 Cable Labs changed the ATP again, setting the expected
        // behavior to 4, which is what it should have been in the first place.
        // Go figure; sure would have been nice for them to tell us that they
        // agreed that it was wrong and were fixing the TEP, but no, they find
        // it much more interesting to mislead you and the fail you when you
        // change your code to do what they want...
        fMaxDownloadTries = 4;

        // Default feature mask should always be 0.  See note in the
        // header file.
        fFeatureMask = 0;
    }
}


// Returns the pointer to the singleton instance for this class.  Most
// objects in the system will use this method rather than being passed a
// pointer to it.  The singleton pointer will be set up in the base-class
// constructor.
//
// NOTES:  This can return NULL if a singleton has not been set up for the
//         system, so you must check for this condition.
//
//         You must not delete this object!
//
//         You should not store a pointer to the object that is returned,
//         since it may be deleted and replaced with a new one.
//
// Parameters:  None.
//
// Returns:
//      A pointer to the instance that should be used by the system.
//
BcmSnmpNonVolSettings *BcmSnmpNonVolSettings::GetSingletonInstance(void)
{
    if (pfSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "BcmSnmpNonVolSettings::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
    }

    return pfSingletonInstance;
}


// Accessors for the AllowConfiguration item.
//
// Parameters:
//      enable - set to true to allow configuration via SNMP, false to
//               disable this feature.
//
// Returns:
//      true if enabled.
//      false if disabled.
//
void BcmSnmpNonVolSettings::AllowConfiguration(bool enable)
{
    fAllowConfiguration = enable;
}


bool BcmSnmpNonVolSettings::AllowConfiguration(void) const
{
    return fAllowConfiguration;
}


// Accessors for the Vendor Name item.
//
// Parameters:
//      pVendorName - string containing the vendor name to be used.
//
// Returns:
//      Pointer to the string containing the vendor name.
//
void BcmSnmpNonVolSettings::VendorName(const char *pVendorName)
{
    // PR1969 - Store this value only for backward compatibility (older software
    // reads and returns the value from nonvol).  Ignore the value passed in,
    // just use our hard coded value.
    pVendorName = kDefaultValue_VendorName;

    memset(fVendorName, 0, sizeof(fVendorName));

    strncpy(fVendorName, pVendorName, sizeof(fVendorName) - 1);
}


const char *BcmSnmpNonVolSettings::VendorName(void) const
{
    // PR1969 - New code always returns the compiled-in value.
    return kDefaultValue_VendorName;
}


// Accessors for the SysDescr item.
//
// Parameters:
//      pString - string containing the value to be used.
//
// Returns:
//      Pointer to the string containing the SysDescr value.
//
void BcmSnmpNonVolSettings::SysDescr(const char *pString)
{
    unsigned int length;

#ifdef kDefaultValue_SysDescr
    // PR1969 - Store this value only for backward compatibility (older software
    // reads and returns the value from nonvol).  Ignore the value passed in,
    // just use our hard coded value.
    pString = kDefaultValue_SysDescr;
#else
    if (pString == NULL)
      pString = SysDescr();
#endif

    length = strlen(pString);

    // The sysDescr value was split into 2 strings, each 128 bytes.  We had to
    // do this because we wanted to increase the size, but we couldn't change
    // the footprint (this was a fixed length field), so we added a "part 2"
    // value to hold any overflow.
    memset(fSysDescr1, 0, sizeof(fSysDescr1));
    memset(fSysDescr2, 0, sizeof(fSysDescr2));

    // Copy as much as I can into the first string.
    strncpy(fSysDescr1, pString, sizeof(fSysDescr1) - 1);

    // If there is anything left over, then copy it to the second string.
    if (length >= sizeof(fSysDescr1))
    {
        strncpy(fSysDescr2, pString + sizeof(fSysDescr1) - 1, sizeof(fSysDescr2) - 1);
    }
}


const char *BcmSnmpNonVolSettings::SysDescr(void) const
{
#ifdef kDefaultValue_SysDescr

    // PR1969 - New code always returns the compiled-in value.
    return kDefaultValue_SysDescr;
    
#else
    static char sysDescr[255];
#if BFC
    const char *pSystem = "BFC";
#else
    const char *pSystem = "V2";
#endif

    gLogMessageRaw << "WARNING: kDefaultValue_SysDescr not defined!  Building sysDescr..." << endl;

    sprintf (sysDescr, 
             "%s cablemodem reference design <<HW_REV: V{TBD} US; VENDOR: Broadcom; BOOTR: {TBD}; SW_REV: %s; MODEL: BCM933xx>>",
             pSystem, CmVersion());
             
    return sysDescr;

#endif
}


// Accessors for the SysObjectId item.
//
// Parameters:
//      pString - string containing the value to be used.
//
// Returns:
//      Pointer to the string containing the SysObjectId value.
//
void BcmSnmpNonVolSettings::SysObjectId(const char *pString)
{
    // PR1969
    // Store this value only for backward compatibility (older software reads
    // and returns the value from nonvol).  Ignore the value passed in, just
    // use our hard coded value.
    pString = kDefaultValue_SysObjectId;

    memset(fSysObjectId, 0, sizeof(fSysObjectId));

    strncpy(fSysObjectId, pString, sizeof(fSysObjectId) - 1);
}


const char *BcmSnmpNonVolSettings::SysObjectId(void) const
{
    // PR1969
    // New code always returns the compiled-in value.
    return kDefaultValue_SysObjectId;
}


// Accessors for the SysContact item.
//
// Parameters:
//      pString - string containing the value to be used.
//
// Returns:
//      Pointer to the string containing the SysContact value.
//
void BcmSnmpNonVolSettings::SysContact(const char *pString)
{
    if (pString == NULL)
    {
        pString = "";
    }

    memset(fSysContact, 0, sizeof(fSysContact));

    strncpy(fSysContact, pString, sizeof(fSysContact) - 1);
}


const char *BcmSnmpNonVolSettings::SysContact(void) const
{
    return fSysContact;
}


// Accessors for the SysName item.
//
// Parameters:
//      pString - string containing the value to be used.
//
// Returns:
//      Pointer to the string containing the SysName value.
//
void BcmSnmpNonVolSettings::SysName(const char *pString)
{
    if (pString == NULL)
    {
        pString = "";
    }

    memset(fSysName, 0, sizeof(fSysName));

    strncpy(fSysName, pString, sizeof(fSysName) - 1);
}


const char *BcmSnmpNonVolSettings::SysName(void) const
{
    return fSysName;
}


// Accessors for the SysLocation item.
//
// Parameters:
//      pString - string containing the value to be used.
//
// Returns:
//      Pointer to the string containing the SysLocation value.
//
void BcmSnmpNonVolSettings::SysLocation(const char *pString)
{
    if (pString == NULL)
    {
        pString = "";
    }

    memset(fSysLocation, 0, sizeof(fSysLocation));

    strncpy(fSysLocation, pString, sizeof(fSysLocation) - 1);
}


const char *BcmSnmpNonVolSettings::SysLocation(void) const
{
    return fSysLocation;
}


// Accessors for the SysORID item.
//
// Parameters:
//      pString - string containing the value to be used.
//
// Returns:
//      Pointer to the string containing the SysORID value.
//
void BcmSnmpNonVolSettings::SysORID(const char *pString)
{
    // PR1969
    // Store this value only for backward compatibility (older software reads
    // and returns the value from nonvol).  Ignore the value passed in, just
    // use our hard coded value.
    pString = kDefaultValue_SysOrId;

    memset(fSysORID, 0, sizeof(fSysORID));

    strncpy(fSysORID, pString, sizeof(fSysORID) - 1);
}


const char *BcmSnmpNonVolSettings::SysORID(void) const
{
    // PR1969
    // New code always returns the compiled-in value.
    return kDefaultValue_SysOrId;
}


// Accessors for the SysORDescr item.
//
// Parameters:
//      pString - string containing the value to be used.
//
// Returns:
//      Pointer to the string containing the SysORDescr value.
//
void BcmSnmpNonVolSettings::SysORDescr(const char *pString)
{
    // PR1969
    // Store this value only for backward compatibility (older software reads
    // and returns the value from nonvol).  Ignore the value passed in, just
    // use our hard coded value.
    pString = kDefaultValue_SysOrDescr;

    memset(fSysORDescr, 0, sizeof(fSysORDescr));

    strncpy(fSysORDescr, pString, sizeof(fSysORDescr) - 1);
}

const char *BcmSnmpNonVolSettings::SysORDescr(void) const
{
    // PR1969
    // New code always returns the compiled-in value.
    return kDefaultValue_SysOrDescr;
}


// Accessors for the SysServices item.
//
// Parameters:
//      value - the SysServices value to be used.
//
// Returns:
//      The SysServices value being used.
//
void BcmSnmpNonVolSettings::SysServices(uint8 value)
{
    // PR1969
    // Ignore the value passed in, just use our hard coded value.
    fSysServices = kDefaultValue_SysServices;
}


uint8 BcmSnmpNonVolSettings::SysServices(void) const
{
    // PR1969
    // New code always returns the compiled-in value.
    return (uint8) kDefaultValue_SysServices;
}


// Accessors for the DocsDevSwCurrentVers item.
//
// Parameters:
//      pString - string containing the value to be used.
//
// Returns:
//      Pointer to the string containing the DocsDevSwCurrentVers value.
//
void BcmSnmpNonVolSettings::DocsDevSwCurrentVers(const char *pString)
{
#ifdef kDefaultValue_DocsDevSwCurrentVers
    // PR1969
    // Store this value only for backward compatibility (older software reads
    // and returns the value from nonvol).  Ignore the value passed in, just
    // use our hard coded value.
    pString = kDefaultValue_DocsDevSwCurrentVers;
#else
    if (pString == NULL)
      pString = DocsDevSwCurrentVers();
#endif

    memset(fDocsDevSwCurrentVers, 0, sizeof(fDocsDevSwCurrentVers));

    strncpy(fDocsDevSwCurrentVers, pString, sizeof(fDocsDevSwCurrentVers) - 1);
}


const char *BcmSnmpNonVolSettings::DocsDevSwCurrentVers(void) const
{
#ifdef kDefaultValue_DocsDevSwCurrentVers
    
    // PR1969
    // New code always returns the compiled-in value.
    return kDefaultValue_DocsDevSwCurrentVers;
    
#else

    gLogMessageRaw << "WARNING: kDefaultValue_DocsDevSwCurrentVers not defined!  Building docsDevSwCurrentVers..." << endl;

    // If no default value is defined, use the version handed to us
    // in the CmVersion method, if one has been.
    return CmVersion();

#endif
}


void BcmSnmpNonVolSettings::CmVersion (const char *pString)
{
    if (pString)
      fCmVersion = pString;
    else
      fCmVersion = "";
}


const char *BcmSnmpNonVolSettings::CmVersion (void) const
{
    if (fCmVersion != "")
      return fCmVersion.c_str();
    else
    {
    #if BFC
      return "3.x.x";
    #else
      return "2.x.x";
    #endif
    }
}


// Accessors for the DocsDevSerialNumber item.
//
// Parameters:
//      pString - string containing the value to be used.
//
// Returns:
//      Pointer to the string containing the DocsDevSerialNumber value.
//
void BcmSnmpNonVolSettings::DocsDevSerialNumber(const char *pString)
{
    if (pString != NULL)
    {
        memset(fDocsDevSerialNumber, 0, sizeof(fDocsDevSerialNumber));

        strncpy(fDocsDevSerialNumber, pString, sizeof(fDocsDevSerialNumber) - 1);
    }
}


const char *BcmSnmpNonVolSettings::DocsDevSerialNumber(void) const
{
    return fDocsDevSerialNumber;
}


// Accessors for the DocsDevSwAdminStatus item.
//
// Parameters:
//      value - the DocsDevSwAdminStatus value to be used.
//
// Returns:
//      The DocsDevSwAdminStatus value being used.
//
void BcmSnmpNonVolSettings::DocsDevSwAdminStatus(uint8 value)
{
    fDocsDevSwAdminStatus = value;
}


uint8 BcmSnmpNonVolSettings::DocsDevSwAdminStatus(void) const
{
    return fDocsDevSwAdminStatus;
}


// Accessors for the DocsDevSwOperStatus item.
//
// Parameters:
//      value - the DocsDevSwOperStatus value to be used.
//
// Returns:
//      The DocsDevSwOperStatus value being used.
//
void BcmSnmpNonVolSettings::DocsDevSwOperStatus(uint8 value, bool UpdateShadow)
{
    fDocsDevSwOperStatus = value;

    // fShadowDocsDevSwOperStatus is provided to handle a specific corner
    // case in MAC-14 (SW download).  The problem is, we finish the
    // download, set docsDevSwOperStatus to completeFromProvisioning, and
    // reboot.  Sadly, we have seen a case where the test script comes
    // along and queries docsDevSwOperStatus in that tiny window between
    // the set and the reboot, and thinks we're back up again.  Then we
    // reboot and start timing out, and fail the test.  So, this shadow
    // variable provides a mechanism to return a value that's different
    // from what's in non-vol to avoid that problem.
    if (UpdateShadow == true)
        fShadowDocsDevSwOperStatus = fDocsDevSwOperStatus;
}


uint8 BcmSnmpNonVolSettings::DocsDevSwOperStatus() const
{
    return fDocsDevSwOperStatus;
}

// ShadowDocsDevSwOperStatus is provided to handle a specific corner
// case in MAC-14 (SW download).  The problem is, we finish the
// download, set docsDevSwOperStatus to completeFromProvisioning, and
// reboot.  Sadly, we have seen a case where the test script comes
// along and queries docsDevSwOperStatus in that tiny window between
// the set and the reboot, and thinks we're back up again.  Then we
// reboot and start timing out, and fail the test.  So, this shadow
// variable provides a mechanism to return a value that's different
// from what's in non-vol to avoid that problem.
uint8 BcmSnmpNonVolSettings::ShadowDocsDevSwOperStatus() const
{
    return fShadowDocsDevSwOperStatus;
}


// Accessors for the DocsDevSwFilename item.
//
// Parameters:
//      pString - string containing the value to be used.
//
// Returns:
//      Pointer to the string containing the DocsDevSwFilename value.
//
void BcmSnmpNonVolSettings::DocsDevSwFilename(const char *pString)
{
    if (pString != NULL)
    {
        memset(fDocsDevSwFilename, 0, sizeof(fDocsDevSwFilename));

        strncpy(fDocsDevSwFilename, pString, sizeof(fDocsDevSwFilename) - 1);
    }
}


const char *BcmSnmpNonVolSettings::DocsDevSwFilename(void) const
{
    return fDocsDevSwFilename;
}


// Accessors for the DocsDevSwServer item.
//
// Parameters:
//      value - the DocsDevSwServer value to be used.
//
// Returns:
//      The DocsDevSwServer value being used.
//
void BcmSnmpNonVolSettings::DocsDevSwServer(const BcmIpAddress &value)
{
    fDocsDevSwServer = value;
}


const BcmIpAddress &BcmSnmpNonVolSettings::DocsDevSwServer(void) const
{
    return fDocsDevSwServer;
}


// Accessors for the number of s/w download tries.
//
// Parameters:
//      value - the number of download tries to be stored.
//
// Returns:
//      The number of download tries.
//
void BcmSnmpNonVolSettings::DownloadTries(uint8 value)
{
    fDownloadTries = value;
}


uint8 BcmSnmpNonVolSettings::DownloadTries(void) const
{
    return fDownloadTries;
}


// Accessors for the number of times the SNMP agent has been booted.
//
// Parameters:
//      value - the new boot value.
//
// Returns:
//      The number of boots.
//
void BcmSnmpNonVolSettings::SnmpEngineBoots (uint32 value)
{
    fSnmpEngineBoots = value;
}

uint32 BcmSnmpNonVolSettings::SnmpEngineBoots (void) const
{
    return fSnmpEngineBoots;
}


// Accessors for the maximum number of s/w download tries that are allowed.
//
// Parameters:
//      value - the max number of download tries to be stored.
//
// Returns:
//      The max number of download tries.
//
void BcmSnmpNonVolSettings::MaxDownloadTries(uint8 value)
{
    fMaxDownloadTries = value;
}


uint8 BcmSnmpNonVolSettings::MaxDownloadTries(void) const
{
    return fMaxDownloadTries;
}


bool BcmSnmpNonVolSettings::PktfilterEmptyLlcUnmatchedIgnore () const
{
    return((fFeatureMask & PKTFILTER_EMPTY_LLC_UNMATCHED_IGNORE) != 0);
}

void BcmSnmpNonVolSettings::PktfilterEmptyLlcUnmatchedIgnore (bool Ignore)
{
    if (Ignore)
        fFeatureMask |= PKTFILTER_EMPTY_LLC_UNMATCHED_IGNORE;
    else
        fFeatureMask &= ~PKTFILTER_EMPTY_LLC_UNMATCHED_IGNORE;

#if ((SNMP_SUPPORT) && (CM_HAL_INCLUDED))
    BcmPacketFilterController::Singleton().IgnoreLlcUnmatchedWhenNoFilters(Ignore);
#endif
}

bool BcmSnmpNonVolSettings::PktfilterEmptyIpDefaultIgnore () const
{
    return((fFeatureMask & PKTFILTER_EMPTY_IP_DEFAULT_IGNORE) != 0);
}

void BcmSnmpNonVolSettings::PktfilterEmptyIpDefaultIgnore (bool Ignore)
{
    if (Ignore)
        fFeatureMask |= PKTFILTER_EMPTY_IP_DEFAULT_IGNORE;
    else
        fFeatureMask &= ~PKTFILTER_EMPTY_IP_DEFAULT_IGNORE;

#if ((SNMP_SUPPORT) && (CM_HAL_INCLUDED))
    BcmPacketFilterController::Singleton().IgnoreIpDefaultWhenNoFilters(Ignore);
#endif
}

bool BcmSnmpNonVolSettings::PktfilterSingleIpFilterSufficient () const
{
    return((fFeatureMask & PKTFILTER_SINGLE_IP_FILTER_SUFFICIENT) != 0);
}

void BcmSnmpNonVolSettings::PktfilterSingleIpFilterSufficient (bool Single)
{
    if (Single)
        fFeatureMask |= PKTFILTER_SINGLE_IP_FILTER_SUFFICIENT;
    else
        fFeatureMask &= ~PKTFILTER_SINGLE_IP_FILTER_SUFFICIENT;

#if ((SNMP_SUPPORT) && (CM_HAL_INCLUDED))
    BcmPacketFilterController::Singleton().SingleIpFilterSufficient(Single);
#endif
}

bool BcmSnmpNonVolSettings::DiagAgentDisabledPostReg () const
{
    return((fFeatureMask & DIAG_AGENT_DISABLED_POST_REG) != 0);
}

void BcmSnmpNonVolSettings::DiagAgentDisabledPostReg (bool Disable)
{
    if (Disable)
        fFeatureMask |= DIAG_AGENT_DISABLED_POST_REG;
    else
        fFeatureMask &= ~DIAG_AGENT_DISABLED_POST_REG;
}

bool BcmSnmpNonVolSettings::HideIpStackIfEntries () const
{
    return((fFeatureMask & SHOW_IP_STACK_IF_ENTRIES) == 0);
}

void BcmSnmpNonVolSettings::HideIpStackIfEntries (bool Hide)
{
    if (Hide == false)
        fFeatureMask |= SHOW_IP_STACK_IF_ENTRIES;
    else
        fFeatureMask &= ~SHOW_IP_STACK_IF_ENTRIES;
}


// This is where the derived class writes its members to the buffer in a
// flat format.  It is up to the derived class to determine what order,
// what data size, whether to use network or host byte order, etc.
//
// This MUST be implemented by the derived class.
//
// Parameters:
//      outputBuffer - the buffer into which the settings should be written.
//      section - the section that should be written.
//
// Returns:
//      true if successful.
//      false if there was a problem (usually the buffer is too small).
//
bool BcmSnmpNonVolSettings::WriteToImpl(BcmOctetBuffer &outputBuffer, 
                                        NonVolSection section) const
{
    if (section == kDynamicSection)
    {
        uint16 zero=0;

        // Append the settings one at a time; if any of these fails, then the
        // buffer is probably too small.
        //
        // NOTE: the 'zero' here indicates that we have no 16 or 32 bit
        // events stored.  This is OK for the CM settings because older
        // versions of code can restore the storage to 32 events if we have
        // as situation where we had new code on a device, then we reverted
        // back to an older version.  This is just to save some flash space,
        // which can be considerable for the event log.  Event log is now
        // handled by its own NV object.
        if (!outputBuffer.NetworkAddToEnd(fDocsDevSwAdminStatus) ||
            !outputBuffer.NetworkAddToEnd(fDocsDevSwOperStatus) ||
            !outputBuffer.AddToEnd((uint8 *) fDocsDevSwFilename, sizeof(fDocsDevSwFilename)) ||
            !fDocsDevSwServer.WriteTo(outputBuffer) ||
            !outputBuffer.NetworkAddToEnd(zero) || // no 16 bit events
            !outputBuffer.NetworkAddToEnd(zero))   // no 32 bit events either.
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "WriteToImpl") 
                << "Buffer too small to hold all of the Dynamic settings!" << endl;

            return false;
        }

        // Just trash any old-style events. We won't waste valuable
        // flash bits on them.
        
        // Trash new style events too.  Flash bits are still valuable!

        // Moved the system info to the dynamic (this) section and made it larger.
        if (!outputBuffer.AddToEnd((uint8 *) fSysContact, sizeof(fSysContact)) ||
            !outputBuffer.AddToEnd((uint8 *) fSysName, sizeof(fSysName)) ||
            !outputBuffer.AddToEnd((uint8 *) fSysLocation, sizeof(fSysLocation)) ||

            // New in 0.2
            !outputBuffer.NetworkAddToEnd(fDownloadTries) ||

            // New in 0.5
            !outputBuffer.NetworkAddToEnd(fSnmpEngineBoots))
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "WriteToImpl") 
                << "Buffer too small to hold all of the Dynamic settings!  "
                   "Failed to write system objects!" << endl;

            return false;
        }
        
        // Don't write event text here any more, it's in event log NV
    }
    else
    {
        bool FactoryMode = false;

#if (kDefaultValue_StoreFactoryModeToNonVol == true)        
        // PR6642: If our NV defaults tells us to, we will store fAllowConfig to
        // NV.  Otherwise, we will always write false for this object
        // to avoid accidental (or nefarious) factory mode remaining after
        // reboot.
        FactoryMode = fAllowConfiguration;
#endif
          
        // Append the settings one at a time; if any of these fails, then the
        // buffer is probably too small.
        if (!outputBuffer.NetworkAddToEnd(FactoryMode) ||

            !outputBuffer.AddToEnd((uint8 *) fVendorName, sizeof(fVendorName)) ||

            !outputBuffer.AddToEnd((uint8 *) fSysDescr1, sizeof(fSysDescr1)) ||
            !outputBuffer.AddToEnd((uint8 *) fSysObjectId, sizeof(fSysObjectId)) ||

            // We have to pad these (128*3) bytes still even though they're
            // not really stored here anymore, so we may as well use the
            // real data.  Plus this way we'll be backwards
            // compatible to the old way of leaving them here.  Just truncate
            // to 128 bytes and put them here as well.
            !outputBuffer.AddToEnd((uint8 *) fSysContact, 128) ||
            !outputBuffer.AddToEnd((uint8 *) fSysName, 128) ||
            !outputBuffer.AddToEnd((uint8 *) fSysLocation, 128) ||

            !outputBuffer.AddToEnd((uint8 *) fSysORID, sizeof(fSysORID)) ||
            !outputBuffer.AddToEnd((uint8 *) fSysORDescr, sizeof(fSysORDescr)) ||

            !outputBuffer.NetworkAddToEnd(fSysServices) ||

            !outputBuffer.AddToEnd((uint8 *) fDocsDevSwCurrentVers, sizeof(fDocsDevSwCurrentVers)) ||
            !outputBuffer.AddToEnd((uint8 *) fDocsDevSerialNumber, sizeof(fDocsDevSerialNumber)) ||

            // New in 0.2
            !outputBuffer.NetworkAddToEnd(fMaxDownloadTries) ||

            // New in 0.3
            !outputBuffer.NetworkAddToEnd(fFeatureMask) ||

            // New in 0.4, 1/25/02
            !outputBuffer.AddToEnd((uint8 *) fSysDescr2, sizeof(fSysDescr2)))
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "WriteToImpl") 
                << "Buffer too small to hold all of the Permanent settings!" << endl;

            return false;
        }
    }

    // If we got this far, then it worked!
    return true;
}


// This is where the derived class reads its members from the buffer in a
// flat format.  It is up to the derived class to determine what ordering,
// etc., should be used.
//
// The currentOffset parameter tells the derived class where the app data
// starts in the buffer.  It can be used directly in the calls to Read() or
// NetworkRead() in BcmOctetBuffer (which expects an offset value).
//
// This MUST be implemented by the derived class.
//
// Parameters:
//      inputBuffer - the buffer from which the settings should be read.
//      currentOffset - the offset into the buffer from which the settings
//                      should be read.  This is necessary to allow the
//                      derived class to skip the header.
//      section - the section that should be read.
//
// Returns:
//      true if successful.
//      false if there was a problem (version number not supported, not
//          enough bytes in the buffer, etc.).
//
bool BcmSnmpNonVolSettings::ReadFromImpl(const BcmOctetBuffer &inputBuffer, 
                                         unsigned int &currentOffset, 
                                         NonVolSection section)
{
    // For this object, we skipped 0.0 for both dynamic and permanent, so this
    // should never be read.  If it is 0.0, then this is a problem, so we bail
    // with comment.
    if ((fVersionMajor[section] == 0) && (fVersionMinor[section] == 0))
    {
        gErrorMsg(fMessageLogSettings, "ReadFromImpl")
            << "Read unsupported version (0.0)!  These settings are not valid!" << endl;

        return false;
    }

    if (section == kDynamicSection)
    {
        uint16 maxIndex;
        int Loop;

        // Read the settings one at a time; if any of these fails, then the
        // buffer is probably too small.
        if (!inputBuffer.NetworkRead(currentOffset, fDocsDevSwAdminStatus) ||
            !inputBuffer.NetworkRead(currentOffset, fDocsDevSwOperStatus) ||
            !inputBuffer.Read(currentOffset, (uint8 *) fDocsDevSwFilename, sizeof(fDocsDevSwFilename)) ||
            !fDocsDevSwServer.ReadFrom(inputBuffer, currentOffset) ||
            !inputBuffer.NetworkRead(currentOffset, maxIndex)) // 16 bit events
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                << "Buffer too small; doesn't have all of the Dynamic settings!" << endl;

            return false;
        }

        // Validate one item; if the OperStatus is "in progress", then we need
        // to set it to "other".  This would happen if we lost power or
        // otherwise rebooted during a software download.  Since we are just
        // now reading the settings again (probably after rebooting), we are
        // not yet "in progress"; we need to start the download again to
        // become "in progress", and the code will automatically do this for us.
        //
        // UNFINISHED - this may not be the best place to do this; if nonvol
        // is read for any reason while a dload is in progress, then this would
        // incorrectly change the state, and would report an incorrect value to
        // SNMP.  As it happens, nothing reads nonvol except a user command from
        // the console, so this won't normally happen.
        //
        // Although, thinking about this further...if the user does do a read,
        // then it might undo this if it was done elsewhere, so maybe this is
        // the best place to do it...
        if (fDocsDevSwOperStatus == CONSTVAL_DOCSDEVSWOPERSTATUS_INPROGRESS)
        {
            fDocsDevSwOperStatus = CONSTVAL_DOCSDEVSWOPERSTATUS_OTHER;
        }

        // Make the shadow value match the real one.
        fShadowDocsDevSwOperStatus = fDocsDevSwOperStatus;

        // If we read in version 0.1, then we need to upgrade to the current
        // rev.  We can't translate the event ID's because we don't have enough
        // info, so we'll just flush them down the drain and start over.
        if (MyVersionIsGreaterThanOrEqualTo(0, 2, section) == false)
        {
            maxIndex = 0;
        }

        // Newer versions:
        else
        {
            // skip any old style event records that might be there.  We can't
            // just do maxIndex * sizeof (EventLogStorageRec16) because things
            // in NV aren't necessarily aligned that way.  So break them out
            // individually...
            currentOffset += maxIndex * 
                             (   sizeof(unsigned short) // Id
                                 + sizeof(unsigned short) // Count
                                 + sizeof (time_t) // FirstTime
                                 + sizeof(time_t) // LastTime
                             ); 

            // Now read the maxIndex for the new style events.
            if (!inputBuffer.NetworkRead(currentOffset, maxIndex)) // 32 bit events
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                    << "Buffer too small; doesn't have all of the Dynamic settings!" << endl;

                return false;
            }
        }

        // Do whatever validation is necessary for the values.
        //
        // If the number of events in non-vol is smaller than what I support,
        // then upgrade it to support more.  If the number of events in non-vol
        // is greater, then downgrade it.  Either way, the number that we
        // support is definitive, and non-vol needs to be adjusted to reflect
        // it.
        //
        // Now that NV exists just for the event log, we will treat any
        // events that are there as extra, and just read them into
        // dummy storage which we will discard.
        for (Loop = 0; Loop < maxIndex; Loop++)
        {
            DummyEventLogStorageRec dummyEvent;
            
            inputBuffer.NetworkRead(currentOffset, dummyEvent.Id);
            inputBuffer.NetworkRead(currentOffset, dummyEvent.Count);
            inputBuffer.NetworkRead(currentOffset, dummyEvent.FirstTime);
            inputBuffer.NetworkRead(currentOffset, dummyEvent.LastTime);
        }

        // If we read in version 0.1 or 0.2, then the writable system objects
        // are stored in the permanent section, not here in the dynamic ones.
        // Use the ones we read from permanent storage.
        if (MyVersionIsGreaterThanOrEqualTo(0, 3, section) == false)
        {
            // We will read these later when we read from permanent storage.
            // We have nothing to do now.
        }

        else
        {
            memset (fSysContact, 0, sizeof(fSysContact));
            memset (fSysName, 0, sizeof(fSysName));
            memset (fSysLocation, 0, sizeof(fSysLocation));

            if (!inputBuffer.Read(currentOffset, (uint8 *) fSysContact, sizeof(fSysContact)) ||
                !inputBuffer.Read(currentOffset, (uint8 *) fSysName, sizeof(fSysName)) ||
                !inputBuffer.Read(currentOffset, (uint8 *) fSysLocation, sizeof(fSysLocation)))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                    << "Buffer too small; doesn't have all of the Dynamic settings!  "
                    << "Failed to read system group objects!" << endl;

                return false;
            }
        }

        // Read items introduced by version 0.4.
        if (MyVersionIsGreaterThanOrEqualTo(0, 4, section))
        {
            if (!inputBuffer.NetworkRead(currentOffset, fDownloadTries))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                    << "Buffer too small; doesn't have all of the Dynamic settings!" << endl;

                return false;
            }
        }

        // Read items introduced by version 0.5.
        if (MyVersionIsGreaterThanOrEqualTo(0, 5, section))
        {
            if (!inputBuffer.NetworkRead(currentOffset, fSnmpEngineBoots))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                    << "Buffer too small; doesn't have all of the Dynamic settings!" << endl;

                return false;
            }
        }
        
        // Read items introduced by version 0.6.
        if (MyVersionIsGreaterThanOrEqualTo(0, 6, section))
        {
            unsigned short Length;
            
            // Now that we have seperate storage for the event log, we will
            // just discard any event text that is stored here.  Read any
            // stored events into dummy storage and ditch it!
            for (Loop = 0; Loop < maxIndex; Loop++)
            {
                BcmOctetBuffer dummyText;
                
                // Read the length field.
                inputBuffer.NetworkRead(currentOffset, Length);
                
                // Resize the buffer for the text.
                dummyText.ResizeBuffer(Length);
                
                // Read the value.
                inputBuffer.Read(currentOffset, dummyText.AccessBuffer(), dummyText.BufferSize());
            }
        }
    }
    else
    {
        char Dummy128[128];
        char *pSysContact, *pSysName, *pSysLocation;

        // If this storage indicates it was written by an older version
        // of code, read writable system objects into the class members
        // and use them for real.  Note that we are checking the version
        // numbers of the dynamic section here even though we are reading
        // from permanent.  That's because the permanent versions didn't
        // change, we just added these settings to the dynamic section.
        if (MyVersionIsGreaterThanOrEqualTo(0, 3, kDynamicSection) == false)
        {
            memset (fSysContact, 0, sizeof(fSysContact));
            memset (fSysName, 0, sizeof(fSysName));
            memset (fSysLocation, 0, sizeof(fSysLocation));

            pSysContact = fSysContact;
            pSysName = fSysName;
            pSysLocation = fSysLocation;
        }
        // Otherwise, just chuck any data for writable system objects from
        // here.
        else
        {
            pSysContact = Dummy128;
            pSysName = Dummy128;
            pSysLocation = Dummy128;
        }

        // Read the settings one at a time; if any of these fails, then the
        // buffer is probably too small.
        if (!inputBuffer.NetworkRead(currentOffset, fAllowConfiguration) ||

            !inputBuffer.Read(currentOffset, (uint8 *) fVendorName, sizeof(fVendorName)) ||

            !inputBuffer.Read(currentOffset, (uint8 *) fSysDescr1, sizeof(fSysDescr1)) ||
            !inputBuffer.Read(currentOffset, (uint8 *) fSysObjectId, sizeof(fSysObjectId)) ||

            // Moved these to dynamic section.  But, since we read from here first,
            // go ahead and read them in anyway.  They'll be overwritten by the
            // stuff in dynamic storage if we have a recent enough version of NV.
            // That way, if we have the old NV version, we'll still read from perm
            // storage so we handle the transition the first time.
            !inputBuffer.Read(currentOffset, (uint8 *) pSysContact, 128) ||
            !inputBuffer.Read(currentOffset, (uint8 *) pSysName, 128) ||
            !inputBuffer.Read(currentOffset, (uint8 *) pSysLocation, 128) ||

            !inputBuffer.Read(currentOffset, (uint8 *) fSysORID, sizeof(fSysORID)) ||
            !inputBuffer.Read(currentOffset, (uint8 *) fSysORDescr, sizeof(fSysORDescr)) ||

            !inputBuffer.NetworkRead(currentOffset, fSysServices) ||

            !inputBuffer.Read(currentOffset, (uint8 *) fDocsDevSwCurrentVers, sizeof(fDocsDevSwCurrentVers)) ||
            !inputBuffer.Read(currentOffset, (uint8 *) fDocsDevSerialNumber, sizeof(fDocsDevSerialNumber))
           )
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                << "Buffer too small; doesn't have all of the Permanent settings!" << endl;

            return false;
        }

        if (MyVersionIsGreaterThanOrEqualTo(0, 2, section))
        {
            // Read items introduced by version 0.2.
            if (!inputBuffer.NetworkRead(currentOffset, fMaxDownloadTries))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                    << "Buffer too small; doesn't have all of the Permanent settings!" << endl;

                return false;
            }
        }

        if (MyVersionIsGreaterThanOrEqualTo(0, 3, section))
        {
            // Read items introduced by version 0.3.
            if (!inputBuffer.NetworkRead(currentOffset, fFeatureMask))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                    << "Buffer too small; doesn't have all of the Permanent settings!" << endl;

                return false;
            }
        }

        if (MyVersionIsGreaterThanOrEqualTo(0, 4, section))
        {
            // Read items introduced by version 0.4
            if (!inputBuffer.Read(currentOffset, (uint8 *) fSysDescr2, sizeof(fSysDescr2)))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                    << "Buffer too small; doesn't have all of the Permanent settings!" << endl;

                return false;
            }
        }

        // Do whatever validation is necessary for the values.
        //
        // PR1969
        // We ignore the contents of many of these values now that they are
        // compiled in to the code.
        VendorName(NULL);
        SysDescr(NULL);
        SysObjectId(NULL);
        SysORID(NULL);
        SysORDescr(NULL);
        DocsDevSwCurrentVers(NULL);
        fSysServices = kDefaultValue_SysServices;

        // PR1665 - print large warning banner if allow config is enabled.  This
        // bit us during CW20 because the manufacturing process changed and the
        // script didn't disable this.  Make sure this prints even in the slim
        // load.
        if (fAllowConfiguration)
        {
            unsigned int loop;

            for (loop = 0; loop < 3; loop++)
            {
                // No delays or dings here, it creates the impression that the
                // app has hung.

                gLogMessageRaw 
                    << "\n****\n*\n* WARNING:  SNMP AllowConfig is enabled!!!\n*\n****\n\n" << endl;
                //<< "\n\a****\n\a*\n\a* WARNING:  SNMP AllowConfig is enabled!!!\n\a*\n\a****\n\a\n\a" << endl;

                //BcmOperatingSystemFactory::ThreadSleep(1000);
            }
        }
    }

    // Call the helper code in the base class to check for version mismatch.
    CheckForVersionUpgrade(kCurrentVersionMajor[section],
                           kCurrentVersionMinor[section], section);

    // If we got this far, then if worked!
    return true;
}


// Causes me to do formatted output of my settings to the specified ostream,
// after the base class does its stuff.
//
// Parameters:
//      outputStream - the ostream to be used.
//
// Returns:
//      The reference to the ostream that was passed in.
//
ostream & BcmSnmpNonVolSettings::PrintImpl(ostream &outputStream) const
{
    #if !(BCM_REDUCED_IMAGE_SIZE)
    
    uint8 operValue, adminValue;

    char *pAdminNames[] = 
    {
        "Unknown",
        "upgradeFromMgt",
        "allowProvisioningUpgrade",
        "ignoreProvisioningUpgrade",
    };

    char *pOperNames[] =
    {
        "Unknown",
        "inProgress",
        "completeFromProvisioning",
        "completeFromMgt",
        "failed",
        "other",
    };

    // Clamp these values just in case they are out of bounds.
    adminValue = fDocsDevSwAdminStatus;
    if (adminValue >= (sizeof(pAdminNames) / sizeof(char *)))
    {
        adminValue = 0;
    }

    operValue = fDocsDevSwOperStatus;
    if (operValue >= (sizeof(pOperNames) / sizeof(char *)))
    {
        operValue = 0;
    }

    // PR1969 - many of these values are now specified in the customer config
    // file.
    outputStream 
        << "\nCM SNMP Permanent NonVol Settings:\n\n"

        << setw(30) << "AllowConfiguration = " << fAllowConfiguration << "\n"
        << setw(30) << "Vendor Name = " << kDefaultValue_VendorName << "\n"
        << setw(30) << "sysDescr = " << SysDescr() << "\n"
        << setw(30) << "sysObjectID = " << kDefaultValue_SysObjectId << "\n"
        << setw(30) << "sysORID.1 = " << kDefaultValue_SysOrId << "\n"
        << setw(30) << "sysORDescr.1 = " << kDefaultValue_SysOrDescr << "\n"
        << setw(30) << "sysServices = " << (void *) kDefaultValue_SysServices << "\n"
        << setw(30) << "docsDevSwCurrentVers = " << DocsDevSwCurrentVers() << "\n"
        << setw(30) << "docsDevSerialNumber = " << fDocsDevSerialNumber << "\n"
        << setw(30) << "Max Download Tries = " << (int) fMaxDownloadTries << "\n"
        << setw(30) << "Ignore LLC unmatched if no filters = " << PktfilterEmptyLlcUnmatchedIgnore() << "\n"
        << setw(30) << "Ignore IP default if no filters = " << PktfilterEmptyIpDefaultIgnore() << "\n"
        << setw(30) << "Single IP filter sufficient = " << PktfilterSingleIpFilterSufficient() << "\n"
        << setw(30) << "Disable diag agent post-reg = " << DiagAgentDisabledPostReg() << "\n"
        << setw(30) << "Hide IP stack ifEntries = " << HideIpStackIfEntries() << "\n"

        << "\nCM SNMP Dynamic NonVol Settings:\n\n"

        << setw(30) << "sysContact = " << fSysContact << "\n"
        << setw(30) << "sysName = " << fSysName << "\n"
        << setw(30) << "sysLocation = " << fSysLocation << "\n"
        << setw(30) << "docsDevSwAdminStatus = " << (int) fDocsDevSwAdminStatus << " (" << pAdminNames[adminValue] << ")\n"
        << setw(30) << "docsDevSwOperStatus = " << (int) fDocsDevSwOperStatus << " (" << pOperNames[operValue] << ")\n"
        << setw(30) << "docsDevSwFilename = " << fDocsDevSwFilename << "\n"
        << setw(30) << "docsDevSwServer = " << fDocsDevSwServer << "\n"
        << setw(30) << "Number of Download tries = " << (int) fDownloadTries << "\n"
        << setw(30) << "snmpEngineBoots = " << (int) fSnmpEngineBoots << "\n"

        << "\n    EVENT LOG NO LONGER STORED HERE!  See NonVol/EventLog." << endl;
    #endif
    
    return outputStream;                        
}

