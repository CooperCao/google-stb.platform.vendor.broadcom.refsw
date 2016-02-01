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
//  Filename:       SnmpNonVolSettings.h
//  Author:         Kevin O'Neal
//  Creation Date:  August 8, 2000
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

#ifndef SNMPNONVOLSETTINGS_H
#define SNMPNONVOLSETTINGS_H

//********************** Include Files ***************************************

// My base class.
#include "NonVolSettings.h"

#include "IpAddress.h"

#include "BcmString.h"

#include <time.h>

//********************** Global Types ****************************************

// Dummy structure to read in any old event log records that might be
// lying around.  We no longer store these here, they're in the
// event log NV settings.
typedef struct _DummyEventLogStorageRec
{
  unsigned long Id;
  unsigned short Count;
  time_t FirstTime;
  time_t LastTime; 
} DummyEventLogStorageRec;


//********************** Global Constants ************************************

/* fFeatureMask bits: */

/*\
 *  A note about these bits: Unused bits in the mask are going to be 0.  So,
 *  whatever the desired default is for a given feature, make a bit that
 *  gets set to 1 if the feature is OPPOSITE of the desired default.  That
 *  way, in the future if we add a new feature bit, it will come up in the
 *  desired default state.
\*/

// Packet filtering modes
#define PKTFILTER_EMPTY_LLC_UNMATCHED_IGNORE  0x0001 // Default is apply
#define PKTFILTER_EMPTY_IP_DEFAULT_IGNORE     0x0002 // Default is apply
#define PKTFILTER_SINGLE_IP_FILTER_SUFFICIENT 0x0004 // Default is 2 required

// Keep the diag agent active after registration
#define DIAG_AGENT_DISABLED_POST_REG          0x0008 // Default is active

#define SHOW_IP_STACK_IF_ENTRIES              0x0010 // Default is to hide them

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


class BcmSnmpNonVolSettings : public BcmNonVolSettings
{
public:

    // Default Constructor.  Initializes the state of the object, setting all
    // fields to good default values.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmSnmpNonVolSettings(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmSnmpNonVolSettings();

    // Set all values to a known good state.
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
    virtual void ResetDefaults(NonVolSection section);

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
    static BcmSnmpNonVolSettings *GetSingletonInstance(void);
    
public:

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
    void AllowConfiguration(bool enable);
    bool AllowConfiguration(void) const;

    // Accessors for the Vendor Name item.
    //
    // Parameters:
    //      pVendorName - string containing the vendor name to be used.
    //
    // Returns:
    //      Pointer to the string containing the vendor name.
    //
    void VendorName(const char *pVendorName);
    const char *VendorName(void) const;

    // Accessors for the SysDescr item.
    //
    // Parameters:
    //      pString - string containing the value to be used.
    //
    // Returns:
    //      Pointer to the string containing the SysDescr value.
    //
    void SysDescr(const char *pString);
    const char *SysDescr(void) const;

    // Accessors for the SysObjectId item.
    //
    // Parameters:
    //      pString - string containing the value to be used.
    //
    // Returns:
    //      Pointer to the string containing the SysObjectId value.
    //
    void SysObjectId(const char *pString);
    const char *SysObjectId(void) const;

    // Accessors for the SysContact item.
    //
    // Parameters:
    //      pString - string containing the value to be used.
    //
    // Returns:
    //      Pointer to the string containing the SysContact value.
    //
    void SysContact(const char *pString);
    const char *SysContact(void) const;

    // Accessors for the SysName item.
    //
    // Parameters:
    //      pString - string containing the value to be used.
    //
    // Returns:
    //      Pointer to the string containing the SysName value.
    //
    void SysName(const char *pString);
    const char *SysName(void) const;

    // Accessors for the SysLocation item.
    //
    // Parameters:
    //      pString - string containing the value to be used.
    //
    // Returns:
    //      Pointer to the string containing the SysLocation value.
    //
    void SysLocation(const char *pString);
    const char *SysLocation(void) const;

    // Accessors for the SysORID item.
    //
    // Parameters:
    //      pString - string containing the value to be used.
    //
    // Returns:
    //      Pointer to the string containing the SysORID value.
    //
    void SysORID(const char *pString);
    const char *SysORID(void) const;

    // Accessors for the SysORDescr item.
    //
    // Parameters:
    //      pString - string containing the value to be used.
    //
    // Returns:
    //      Pointer to the string containing the SysORDescr value.
    //
    void SysORDescr(const char *pString);
    const char *SysORDescr(void) const;

    // Accessors for the SysServices item.
    //
    // Parameters:
    //      value - the SysServices value to be used.
    //
    // Returns:
    //      The SysServices value being used.
    //
    void SysServices(uint8 value);
    uint8 SysServices(void) const;

    // Accessors for the DocsDevSwCurrentVers item.
    //
    // Parameters:
    //      pString - string containing the value to be used.
    //
    // Returns:
    //      Pointer to the string containing the DocsDevSwCurrentVers value.
    //
    void DocsDevSwCurrentVers(const char *pString);
    const char *DocsDevSwCurrentVers(void) const;
    
    // Accessors for the CmVersion item.  Note that this is in fact NOT
    // stored in NV, it's a backup in case we don't have sysDescr or
    // docsDevSwCurrentVers defined, this way we can build it.
    void CmVersion (const char *pString);
    const char *CmVersion (void) const;

    // Accessors for the DocsDevSerialNumber item.
    //
    // Parameters:
    //      pString - string containing the value to be used.
    //
    // Returns:
    //      Pointer to the string containing the DocsDevSerialNumber value.
    //
    void DocsDevSerialNumber(const char *pString);
    const char *DocsDevSerialNumber(void) const;

public:

    // Accessors for the DocsDevSwAdminStatus item.
    //
    // Parameters:
    //      value - the DocsDevSwAdminStatus value to be used.
    //
    // Returns:
    //      The DocsDevSwAdminStatus value being used.
    //
    void DocsDevSwAdminStatus(uint8 value);
    uint8 DocsDevSwAdminStatus(void) const;

    // Accessors for the DocsDevSwOperStatus item.
    //
    // Parameters:
    //      value - the DocsDevSwOperStatus value to be used.
    //
    // Returns:
    //      The DocsDevSwOperStatus value being used.
    //
    void DocsDevSwOperStatus(uint8 value, bool UpdateShadow = true);
    uint8 DocsDevSwOperStatus() const;
    uint8 ShadowDocsDevSwOperStatus() const;

    // Accessors for the DocsDevSwFilename item.
    //
    // Parameters:
    //      pString - string containing the value to be used.
    //
    // Returns:
    //      Pointer to the string containing the DocsDevSwFilename value.
    //
    void DocsDevSwFilename(const char *pString);
    const char *DocsDevSwFilename(void) const;

    // Accessors for the DocsDevSwServer item.
    //
    // Parameters:
    //      value - the DocsDevSwServer value to be used.
    //
    // Returns:
    //      The DocsDevSwServer value being used.
    //
    void DocsDevSwServer(const BcmIpAddress &value);
    const BcmIpAddress &DocsDevSwServer(void) const;

    // Accessors for the number of s/w download tries.
    //
    // Parameters:
    //      value - the number of download tries to be stored.
    //
    // Returns:
    //      The number of download tries.
    //
    void DownloadTries(uint8 value);
    uint8 DownloadTries(void) const;
    
    // Accessors for the number of times the SNMP agent has been booted.
    //
    // Parameters:
    //      value - the new boot value.
    //
    // Returns:
    //      The number of boots.
    //
    void SnmpEngineBoots (uint32 value);
    uint32 SnmpEngineBoots (void) const;

    // Accessors for the maximum number of s/w download tries that are allowed.
    //
    // Parameters:
    //      value - the max number of download tries to be stored.
    //
    // Returns:
    //      The max number of download tries.
    //
    void MaxDownloadTries(uint8 value);
    uint8 MaxDownloadTries(void) const;
    
    /*\
     *  Accessors for bits in the fFeatureMask member
    \*/
    
    // Packet filtering related mode bits
    bool PktfilterEmptyLlcUnmatchedIgnore () const;
    void PktfilterEmptyLlcUnmatchedIgnore (bool Ignore);
    
    bool PktfilterEmptyIpDefaultIgnore () const;
    void PktfilterEmptyIpDefaultIgnore (bool Ignore);
    
    bool PktfilterSingleIpFilterSufficient () const;
    void PktfilterSingleIpFilterSufficient (bool Single);
    
    bool DiagAgentDisabledPostReg () const;
    void DiagAgentDisabledPostReg (bool Disable);
    
    bool HideIpStackIfEntries () const;
    void HideIpStackIfEntries (bool Show);
    
protected:

    // Here's where I write my members to the buffer in a flat format, in the
    // order described at the top of this file.  All multibyte values are
    // written in network byte order.
    //
    // Parameters:
    //      outputBuffer - the buffer into which the settings should be written.
    //      section - the section that should be written.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (usually the buffer is too small).
    //
    virtual bool WriteToImpl(BcmOctetBuffer &outputBuffer, NonVolSection section) const;

    // Here's where I read my members from the buffer in a flat format, in the
    // order described at the top of this file.  All multibyte values are read
    // in network byte order.
    //
    // The currentOffset parameter tells the derived class where the app data
    // starts in the buffer.  It can be used directly in the calls to Read() or
    // NetworkRead() in BcmOctetBuffer (which expects an offset value).
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
    virtual bool ReadFromImpl(const BcmOctetBuffer &inputBuffer, 
                              unsigned int &currentOffset, NonVolSection section);

    // Causes me to do formatted output of my settings to the specified ostream,
    // after the base class does its stuff.
    //
    // Parameters:
    //      outputStream - the ostream to be used.
    //
    // Returns:
    //      The reference to the ostream that was passed in.
    //
    virtual ostream & PrintImpl(ostream &outputStream) const;

private:

    // Settings for the Permanent Group:
    // --------------------------------

    // Configurable MIB objects.  All of these are supposed to be read-only,
    // but should still be stored in NV to support different vendors using this
    // same refrence design directly and without source code mods.
    
    // allow R/W access to various normally R/O objects below
    bool fAllowConfiguration; 
    char fVendorName[32];
    
    // system group
    char fSysDescr1[128];
    char fSysDescr2[128]; // 1/25/02: New CL format sometimes > 128!
    char fSysObjectId[128];
    
    // Made these larger and moved them to NV storage.
    char fSysContact[256];
    char fSysName[256];
    char fSysLocation[256];
    
    char fSysORID[128];
    char fSysORDescr[128];
    uint8 fSysServices;
    
    // docsDev group
    char fDocsDevSwCurrentVers[128];
    char fDocsDevSerialNumber[64];

    // More settings for software upgrade.  We need to store the max number of
    // download attempts that are allowed before we give up.  There is some
    // vagueness in the spec as to whether 3 or 4 tries should be made, and the
    // ATP seems to want 3.  We think it should be 4 (1 try, plus 3 retries).
    // In the end, the ATP is what matters, so we'll make this flexible.
    uint8 fMaxDownloadTries;
    
    // Bitmask for on/off settings, added as needed.  Bits defined above.
    uint32 fFeatureMask;
    
    // Local copy of the CM version, used to construct docsDevSwVersion and
    // sysDescr if we don't have a compile-time value.
    BcmString fCmVersion;
    
private:

    // Settings for the Dynamic Group:
    // ------------------------------

    // These are SNMP-based settings for the software upgrade stuff.  See
    // RFC2669, docsDevSoftware section for details.
    uint8 fDocsDevSwAdminStatus;
    uint8 fDocsDevSwOperStatus;
    char fDocsDevSwFilename[256];
    BcmIpAddress fDocsDevSwServer;
    
    // These values shadow the real NV values.  This is necessary to
    // avoid certain race conditions where we've just set something to
    // NV and are ABOUT to reboot, but haven't just yet and we get an
    // SNMP query in that little window.  So some objects we'll lie about.
    uint8 fShadowDocsDevSwOperStatus;

    // More settings for software upgrade.  We need to store the number of
    // download attempts that have been made.
    uint8 fDownloadTries;
    
    // Number of times this device has been booted, incremented at each
    // boot.
    uint32 fSnmpEngineBoots;
    
    // Not stored to NV, used internally to track the next available event
    // record in the case where the log is not full yet.
    int fNextAvailableEvent;

private:

    // Keeps Reset() from complaining when it is called from the constructor.
    bool fConstructorRunning;

private:

    // This is the singleton instance of this class.
    static BcmSnmpNonVolSettings *pfSingletonInstance;

private:

    // Copy constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmSnmpNonVolSettings(const BcmSnmpNonVolSettings &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmSnmpNonVolSettings & operator = (const BcmSnmpNonVolSettings &otherInstance);

};


//********************** Inline Method Implementations ***********************


// This allows you to do the following:
//
//    cout << settingsInstance << endl;
//
inline ostream & operator << (ostream &outputStream, const BcmSnmpNonVolSettings &settingsInstance)
{
    return settingsInstance.Print(outputStream);
}


#endif


