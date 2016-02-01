//****************************************************************************
//
// Copyright (c) 2000-2010 Broadcom Corporation
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
//  $Id$
//
//  Filename:       MessageLogNonVolSettings.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 10, 2000
//
//****************************************************************************
//  Description:
//      These are the Non-Volatile settings for the Message Logging subsystem.
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
#include "MessageLogNonVolSettings.h"

#include "MessageLogSettings.h"

// PR1969
#include "MergedNonVolDefaults.h"

#include <string.h>
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iomanip>
#else
#include <iomanip.h>
#endif
#include "BcmString.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

// Dynamic rev = 0.4
// Permanent rev = 0.2
const uint8 kCurrentVersionMajor[2] =
{// Dyn  Perm
     0,    0
};

const uint8 kCurrentVersionMinor[2] =
{// Dyn  Perm
     5,    2
};

// limit the max HTTP remote acl size
#define kMaxRemoteHosts 20
//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Set this to NULL initially.  This will be set up in my constructor.
BcmMessageLogNonVolSettings *BcmMessageLogNonVolSettings::pfSingletonInstance = NULL;


// Default Constructor.  Initializes the state of the object, setting all
// fields to good default values.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmMessageLogNonVolSettings::BcmMessageLogNonVolSettings(void) :
    BcmNonVolSettings("User Interface NonVol Settings")
{
    // Override the class name in the message log settings that was set by my
    // base class.
    fMessageLogSettings.SetModuleName("BcmMessageLogNonVolSettings");

    // Just let this do the work.
    ResetDefaults(kPermanentSection);
    ResetDefaults(kDynamicSection);

    // Set myself as the singleton.
    if (pfSingletonInstance == NULL)
    {
        gInfoMsg(fMessageLogSettings, "BcmMessageLogNonVolSettings")
            << "Setting up the singleton pointer." << endl;
        pfSingletonInstance = this;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "BcmMessageLogNonVolSettings")
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
BcmMessageLogNonVolSettings::~BcmMessageLogNonVolSettings()
{
    // Clear the singleton pointer.
    if (pfSingletonInstance == this)
    {
        gInfoMsg(fMessageLogSettings, "~BcmMessageLogNonVolSettings")
            << "Clearing the singleton pointer." << endl;

        pfSingletonInstance = NULL;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "~BcmMessageLogNonVolSettings")
            << "I'm not the singleton instance!  Leaving the singleton pointer alone..." << endl;
    }
}


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
void BcmMessageLogNonVolSettings::ResetDefaults(NonVolSection section)
{
    // Set the magic number.  This ASCII-hex for MLog.
    fMagicNumber = 0x4d4c6f67;

    fVersionMajor[section] = kCurrentVersionMajor[section];
    fVersionMinor[section] = kCurrentVersionMinor[section];
    
    // There are no dynamic settings to reset.
    if (section == kDynamicSection)
    {
        // Get user and admin ID's and passwords from VendorHttpControls.h
        // Use a BcmString to ease conversion...
        BcmString String;
        
        String = kDefaultValue_RemoteAccessHttpUserId;
        String.Get(fHttpUserId);
        String = kDefaultValue_RemoteAccessHttpUserPassword;
        String.Get(fHttpUserPassword);
        
        String = kDefaultValue_RemoteAccessHttpAdminId;
        String.Get (fHttpAdminId);
        String = kDefaultValue_RemoteAccessHttpAdminPassword;
        String.Get (fHttpAdminPassword);
        
        fRemoteAccessEnable = 0;

        // PR4517
        if (kDefaultValue_TelnetServerEnabled)
        {
            fRemoteAccessEnable |= kTelnetRemoteAccessEnabled;
        }
    
        if (kDefaultValue_SshServerEnabled)
        {
            fRemoteAccessEnable |= kSshRemoteAccessEnabled;
        }
    
        // PR4517
        memset(fRemoteAccessUserName, 0, sizeof(fRemoteAccessUserName));
        strncpy(fRemoteAccessUserName, kDefaultValue_TelnetUsername, sizeof(fRemoteAccessUserName) - 1);
    
        // PR4517
        memset(fRemoteAccessPassword, 0, sizeof(fRemoteAccessPassword));
        strncpy(fRemoteAccessPassword, kDefaultValue_TelnetPassword, sizeof(fRemoteAccessPassword) - 1);
        
        // PR4517 - Added with version 0.2; sets the IP stacks that we will accept
        // telnet connections on.
        fTelnetIpStackBitmask = kDefaultValue_TelnetIpStackEnable;
        fSshIpStackBitmask = kDefaultValue_SshIpStackEnable;
        
        // PR 9961
        fRemoteAccessInactivityTimeout = 0; // no timeout by default

        // PR 8884
        // TRW - the local stack is always enabled.  It will be
        // the same stack as the remote access stack (2 - CM/RG/3-play, 3-EMTA)
        fHttpIpStackBitmask  = 1 << (kDefaultValue_RemoteAccessIpStackNumber-1);
        fHttpAdvancedIpStackBitmask = 0;
        
        // TRW - add any remote stacks that are enabled
        
        #if (kDefaultValue_HttpRemoteAccessStack1Enabled)
        fHttpIpStackBitmask |= kHttpIpStack1;
        // by default, advanced access only allowed on remote stacks
        fHttpAdvancedIpStackBitmask |= kHttpIpStack1;
        #endif

        // trw - this will only be defined for a cablehome build
        #if (kDefaultValue_HttpRemoteAccessStack3Enabled)
        fHttpIpStackBitmask |= kHttpIpStack3;
        // by default, advanced access only allowed on remote stacks
        fHttpAdvancedIpStackBitmask |= kHttpIpStack3;
        #endif
        
        #if (BCM_CABLEHOME_SUPPORT)
        // also set the CH private stack
        fHttpIpStackBitmask |= kHttpIpStack5;
        #endif

        fHttpPasswordSeed.Empty();
        fHttpRemoteIpAcl.Reset();
        fHttpPasswordIdleTimeout = 0; // no timeout by default

        fOtherLogsEnable = kExceptionLogEnabled; // Exception Logging Enabled by Default.
        return;
    }

    // Set the other values to defaults.
    fDefaultEnabledSeverities = BcmMessageLogSettings::kWarningMessages | 
                                BcmMessageLogSettings::kErrorMessages | 
                                BcmMessageLogSettings::kFatalErrorMessages;
    fDefaultEnabledFields = BcmMessageLogSettings::kAllFields;

    // Store the defaults in the message log settings object.
    BcmMessageLogSettings::SetDefaultSeverities(fDefaultEnabledSeverities);
    BcmMessageLogSettings::SetDefaultFields(fDefaultEnabledFields);

    // PR1969 - use default values specified in the customer config file.
    fIpAddress.Set(kDefaultValue_RemoteAccessIpAddress);
    fSubnetMask.Set(kDefaultValue_RemoteAccessSubnetMask);
    fRouter.Set(kDefaultValue_RemoteAccessRouterAddress);

    fRemoteAccessIpStackNumber = kDefaultValue_RemoteAccessIpStackNumber;
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
BcmMessageLogNonVolSettings *BcmMessageLogNonVolSettings::GetSingletonInstance(void)
{
    // Disable this warning; it happens quite a bit at startup, and our
    // customers don't like to see it.
    /*if (pfSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "BcmMessageLogNonVolSettings::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!\n"
               "This may be normal, but if the system crashes after this, then you'll know why..." << endl;
    }*/
    
    return pfSingletonInstance;
}


// Accessors for the severities value.
//
// Parameters:
//      severities - the new severities bitmask to be used.
//
// Returns:
//      The severities bitmask.
//
void BcmMessageLogNonVolSettings::Severities(uint32 severities)
{
    fDefaultEnabledSeverities = severities;

    // Store the defaults in the message log settings object.
    BcmMessageLogSettings::SetDefaultSeverities(fDefaultEnabledSeverities);
}


uint32 BcmMessageLogNonVolSettings::Severities(void) const
{
    return fDefaultEnabledSeverities;
}


// Accessors for the fields value.
//
// Parameters:
//      fields - the new fields bitmask to be used.
//
// Returns:
//      The fields bitmask.
//
void BcmMessageLogNonVolSettings::Fields(uint8 fields)
{
    fDefaultEnabledFields = fields;

    // Store the defaults in the message log settings object.
    BcmMessageLogSettings::SetDefaultFields(fDefaultEnabledFields);
}


uint8 BcmMessageLogNonVolSettings::Fields(void) const
{
    return fDefaultEnabledFields;
}


// Accessors for the HTTP remote access enable flag.
//
// Parameters:
//      enable - set to true to enable this, false to disable.
//
// Returns:
//      true if enabled.
//      false if disabled.
//
void BcmMessageLogNonVolSettings::HttpRemoteAccessEnabled(bool enable)
{
    if (enable)
    {
        fRemoteAccessEnable |= kHttpRemoteAccessEnabled;
    }
    else
    {
        fRemoteAccessEnable &= ~kHttpRemoteAccessEnabled;
    }
}


bool BcmMessageLogNonVolSettings::HttpRemoteAccessEnabled(void) const
{
    if (fRemoteAccessEnable & kHttpRemoteAccessEnabled)
    {
        return true;
    }

    return false;
}

// Accessors for the remote access DHCP settings values.
//
// Parameters:
//      ipAddress - the IP address value to use.
//
// Returns:
//      The IP address value that is being used.
//
void BcmMessageLogNonVolSettings::IpAddress(const BcmIpAddress &ipAddress)
{
    fIpAddress = ipAddress;
}

const BcmIpAddress &BcmMessageLogNonVolSettings::IpAddress(void) const
{
    return fIpAddress;
}


void BcmMessageLogNonVolSettings::SubnetMask(const BcmIpAddress &ipAddress)
{
    fSubnetMask = ipAddress;
}

const BcmIpAddress &BcmMessageLogNonVolSettings::SubnetMask(void) const
{
    return fSubnetMask;
}


void BcmMessageLogNonVolSettings::Router(const BcmIpAddress &ipAddress)
{
    fRouter = ipAddress;
}

const BcmIpAddress &BcmMessageLogNonVolSettings::Router(void) const
{
    return fRouter;
}


// Acessors for the IP stack number.  The is the IP stack number that the
// remote access server will use when sending and receiving packets.
//
// Parameters:
//      ipStackNumber - the IP stack number to be used.
//
// Returns:
//      The IP stack number that is being used.
//
void BcmMessageLogNonVolSettings::RemoteAccessIpStackNumber(uint8 ipStackNumber)
{
    fRemoteAccessIpStackNumber = ipStackNumber;
}


uint8 BcmMessageLogNonVolSettings::RemoteAccessIpStackNumber(void) const
{
    return fRemoteAccessIpStackNumber;
}

// Accessors for the telnet remote access enable flag.  Note that this only
// affects whether or not the telnet server will be started when the system
// boots up.  If disabled, the server can be started from the serial
// console or other mechanism (e.g. MIB) after the system is running.
//
// Parameters:
//      enable - set to true to enable this, false to disable.
//
// Returns:
//      true if enabled.
//      false if disabled.
//
void BcmMessageLogNonVolSettings::TelnetRemoteAccessEnabled(bool enable)
{
    if (enable)
    {
        fRemoteAccessEnable |= kTelnetRemoteAccessEnabled;
    }
    else
    {
        fRemoteAccessEnable &= ~kTelnetRemoteAccessEnabled;
    }
}


bool BcmMessageLogNonVolSettings::TelnetRemoteAccessEnabled(void) const
{
    if (fRemoteAccessEnable & kTelnetRemoteAccessEnabled)
    {
        return true;
    }

    return false;
}

// Accessors for the SSH remote access enable flag.  Note that this only
// affects whether or not the SSH server will be started when the system
// boots up.  If disabled, the server can be started from the serial
// console or other mechanism (e.g. MIB) after the system is running.
//
// Parameters:
//      enable - set to true to enable this, false to disable.
//
// Returns:
//      true if enabled.
//      false if disabled.
//
void BcmMessageLogNonVolSettings::SshRemoteAccessEnabled(bool enable)
{
    if (enable)
    {
        fRemoteAccessEnable |= kSshRemoteAccessEnabled;
    }
    else
    {
        fRemoteAccessEnable &= ~kSshRemoteAccessEnabled;
    }
}

bool BcmMessageLogNonVolSettings::SshRemoteAccessEnabled(void) const
{
    if (fRemoteAccessEnable & kSshRemoteAccessEnabled)
    {
        return true;
    }

    return false;
}

// Accessors for the remote access user name.  This field is limited to
// 16 chars, including the \0 terminator.
//
// Parameters:
//      pUserName - pointer to the new user name string to be stored.
//
// Returns:
//      The current user name.
//
void BcmMessageLogNonVolSettings::RemoteAccessUserName(const char *pUserName)
{
    if (pUserName != NULL)
    {
        memset(fRemoteAccessUserName, 0, sizeof(fRemoteAccessUserName));
        strncpy(fRemoteAccessUserName, pUserName, sizeof(fRemoteAccessUserName) - 1);
    }
}


const char *BcmMessageLogNonVolSettings::RemoteAccessUserName(void) const
{
    return fRemoteAccessUserName;
}


// Accessors for the remote access password.  This field is limited to
// 16 chars, including the \0 terminator.
//
// Parameters:
//      pPassword - pointer to the new password string to be stored.
//
// Returns:
//      The current password.
//
void BcmMessageLogNonVolSettings::RemoteAccessPassword(const char *pPassword)
{
#ifdef BCM_TELNET_SSH_POTD
    gErrorMsg(fMessageLogSettings, "RemoteAccessPassword") << "This is not supported in our current mode." << endl;
#else    
    if (pPassword != NULL)
    {
        memset(fRemoteAccessPassword, 0, sizeof(fRemoteAccessPassword));
        strncpy(fRemoteAccessPassword, pPassword, sizeof(fRemoteAccessPassword) - 1);
    }
#endif
}


const char *BcmMessageLogNonVolSettings::RemoteAccessPassword(void) const
{
#ifdef BCM_TELNET_SSH_POTD
    gErrorMsg(fMessageLogSettings, "RemoteAccessPassword") << "This is not supported in our current mode, returning default pw." << endl;
    return kDefaultValue_TelnetPassword;
#else    
    return fRemoteAccessPassword;
#endif
}


// PR 9961
// Accessors for the remote access inactivity timeout.  A session which
// has no activity for greater than the number of seconds specified will
// be terminated.
//
// \param
//      Seconds - the number of seconds after which an inactive session
//                will be terminated.
//
// \return
//      The current inactivity timeout.
//
void BcmMessageLogNonVolSettings::RemoteAccessInactivityTimeout(int Seconds)
{
  fRemoteAccessInactivityTimeout = Seconds;
}

const int BcmMessageLogNonVolSettings::RemoteAccessInactivityTimeout(void) const
{
  return fRemoteAccessInactivityTimeout;
}


// Accessors for the IP stacks that you want the Telnet server to accept
// connections on.
//
// PR4517
//
// Parameters:
//      ipStackBitmask - bitmask of the stacks that the Telnet server
//                       should accept connections on.
//
// Returns:
//      The bits corresponding to the IP stacks that we will accept
//      connections on.
//
void BcmMessageLogNonVolSettings::TelnetIpStackBitmask(uint8 ipStackBitmask)
{
    fTelnetIpStackBitmask = ipStackBitmask;
}


uint8 BcmMessageLogNonVolSettings::TelnetIpStackBitmask(void) const
{
    return fTelnetIpStackBitmask;
}

// Accessors for the IP stacks that you want the SSH server to accept
// connections on.
//
// PR4517
//
// Parameters:
//      ipStackBitmask - bitmask of the stacks that the SSH server
//                       should accept connections on.
//
// Returns:
//      The bits corresponding to the IP stacks that we will accept
//      connections on.
//
void BcmMessageLogNonVolSettings::SshIpStackBitmask(uint8 ipStackBitmask)
{
    fSshIpStackBitmask = ipStackBitmask;
}

uint8 BcmMessageLogNonVolSettings::SshIpStackBitmask(void) const
{
    return fSshIpStackBitmask;
}

// Added with version 0.4
void BcmMessageLogNonVolSettings::HttpIpStackBitmask(uint8 ipStackBitmask)
{
   fHttpIpStackBitmask = ipStackBitmask;
}

uint8 BcmMessageLogNonVolSettings::HttpIpStackBitmask(void) const
{
   return fHttpIpStackBitmask;
}

void BcmMessageLogNonVolSettings::HttpAdvancedIpStackBitmask(uint8 ipStackBitmask)
{
   fHttpAdvancedIpStackBitmask = ipStackBitmask;
}

uint8 BcmMessageLogNonVolSettings::HttpAdvancedIpStackBitmask(void) const
{
   return fHttpAdvancedIpStackBitmask;
}

void BcmMessageLogNonVolSettings::HttpPasswordSeed (const BcmOctetBuffer &Seed)
{
   fHttpPasswordSeed = Seed;
}

const BcmOctetBuffer &BcmMessageLogNonVolSettings::HttpPasswordSeed (void) const
{
   return fHttpPasswordSeed;
}

void BcmMessageLogNonVolSettings::HttpRemoteIpAcl(BcmIpAddressList &remoteHosts)
{
   fHttpRemoteIpAcl = remoteHosts;
}

BcmIpAddressList &BcmMessageLogNonVolSettings::HttpRemoteIpAcl(void)
{
   return fHttpRemoteIpAcl;
}

bool BcmMessageLogNonVolSettings::HttpRemoteHostIsAuthorized(BcmIpAddress host)
{
   if(fHttpRemoteIpAcl.Size() == 0)
   {
      // list is empty - everyone is authorized
      return true;
   }

   for(unsigned int i=0; i<fHttpRemoteIpAcl.Size(); i+=2)
   {
      // if [i] and [+1] are same address, it is a single IP
      if(fHttpRemoteIpAcl[i] == fHttpRemoteIpAcl[i+1])
      {
         if(host == fHttpRemoteIpAcl[i])
         {
            return true;
         }
      }
      else
      {
         // is IP address within the range of [i] and [i+1]?
         if((!(host < fHttpRemoteIpAcl[i])) && (host <= fHttpRemoteIpAcl[i+1]))
         {
            return true;
         }
      }
   }

   // nothing matched
   return false;
}

bool BcmMessageLogNonVolSettings::HttpAddAclHost(BcmIpAddress host)
{
   if((fHttpRemoteIpAcl.Size()/2) >= kMaxRemoteHosts)
   {
      return false;
   }

   BcmOctetBuffer newHostList(fHttpRemoteIpAcl.AsBuffer().BytesUsed()+8);
   newHostList.Append(fHttpRemoteIpAcl.AsBuffer());
   
   // single address - add it twice
   host.WriteTo(newHostList);
   host.WriteTo(newHostList);

   fHttpRemoteIpAcl = newHostList;
   return true;
}

bool BcmMessageLogNonVolSettings::HttpAddAclHostRange(BcmIpAddress start, BcmIpAddress end)
{
   if((fHttpRemoteIpAcl.Size()/2) >= kMaxRemoteHosts)
   {
      return false;
   }
   
   BcmOctetBuffer newHostList(fHttpRemoteIpAcl.AsBuffer().BytesUsed()+8);
   newHostList.Append(fHttpRemoteIpAcl.AsBuffer());
   
   // add range
   start.WriteTo(newHostList);
   end.WriteTo(newHostList);

   fHttpRemoteIpAcl = newHostList;
   return true;
}

bool BcmMessageLogNonVolSettings::HttpRemoveAclHost(BcmIpAddress host)
{
   // search for it
   for(unsigned int i=0; i<fHttpRemoteIpAcl.Size(); i+=2)
   {
      if((fHttpRemoteIpAcl[i] == fHttpRemoteIpAcl[i+1]) && (host == fHttpRemoteIpAcl[i]))
      {
         // remove it
         BcmOctetBuffer newHostList;
         unsigned int j;
         for(j=0; j<i; j++)
         {
            fHttpRemoteIpAcl[j].WriteTo(newHostList);
         }
         for(j=i+2; j<fHttpRemoteIpAcl.Size(); j++)
         {
            fHttpRemoteIpAcl[j].WriteTo(newHostList);
         }
         fHttpRemoteIpAcl = newHostList;
         return true;
      }
   }
   // not found
   return false;
}

bool BcmMessageLogNonVolSettings::HttpRemoveAclHostRange(BcmIpAddress start, BcmIpAddress end)
{
   // search for it
   for(unsigned int i=0; i<fHttpRemoteIpAcl.Size(); i+=2)
   {
      if((fHttpRemoteIpAcl[i] == start) &&
         (fHttpRemoteIpAcl[i+1]==end))
      {
         // remove it
         BcmOctetBuffer newHostList;
         unsigned int j;
         for(j=0; j<i; j++)
         {
            fHttpRemoteIpAcl[j].WriteTo(newHostList);
         }
         for(j=i+2; j<fHttpRemoteIpAcl.Size(); j++)
         {
            fHttpRemoteIpAcl[j].WriteTo(newHostList);
         }
         fHttpRemoteIpAcl = newHostList;
         return true;
      }
   }
   // not found
   return false;
}

void BcmMessageLogNonVolSettings::HttpClearRemoteIpAcl(void)
{
   fHttpRemoteIpAcl.Reset();
}

void BcmMessageLogNonVolSettings::HttpPasswordIdleTimeout(uint32 timeout)
{
   fHttpPasswordIdleTimeout = timeout;
}

uint32 BcmMessageLogNonVolSettings::HttpPasswordIdleTimeout(void) const
{
   return fHttpPasswordIdleTimeout;
}

// Accessors for HTTP user IDs and passwords.  These were previously 
// hard-coded in VendorHttpControls.h, from which we still get the
// default values.
//
// Parameters:
//   Id, Password:  The user ID or password.
//
// Returns:
//   The user ID or password.
const BcmOctetBuffer &BcmMessageLogNonVolSettings::HttpUserId (void) const
{
  return fHttpUserId;
}

void BcmMessageLogNonVolSettings::HttpUserId (const BcmOctetBuffer &Id)
{
  fHttpUserId = Id;
}

const BcmOctetBuffer &BcmMessageLogNonVolSettings::HttpAdminId (void) const
{
  return fHttpAdminId;
}

void BcmMessageLogNonVolSettings::HttpAdminId (const BcmOctetBuffer &Id)
{
  fHttpAdminId = Id;
}

const BcmOctetBuffer &BcmMessageLogNonVolSettings::HttpUserPassword (void) const
{
  return fHttpUserPassword;
}

void BcmMessageLogNonVolSettings::HttpUserPassword (const BcmOctetBuffer &Password)
{
  fHttpUserPassword = Password;
}

const BcmOctetBuffer &BcmMessageLogNonVolSettings::HttpAdminPassword (void) const
{
  return fHttpAdminPassword;
}

void BcmMessageLogNonVolSettings::HttpAdminPassword (const BcmOctetBuffer &Password)
{
  fHttpAdminPassword = Password;
}


// Accessors to enable/disable the Exception Logging.
//
// Parameters:
//      enable - true to enable Exception Logging, false to disable.
//
// Returns:
//      The enable state.
//
bool BcmMessageLogNonVolSettings::ExceptionLogEnable(void) const
{
    if (fOtherLogsEnable & kExceptionLogEnabled)
    {
        return true;
    }

    return false;
}


void BcmMessageLogNonVolSettings::ExceptionLogEnable(bool enable)
{
    if (enable)
    {
        fOtherLogsEnable |= kExceptionLogEnabled;
    }
    else
    {
        fOtherLogsEnable &= ~kExceptionLogEnabled;
    }
}


// Helper function to convert the bitmask to string names.
//
// PR4517
//
// Parameters:
//      pNamesString - pointer to the string into which the names should
//                     be copied.
//      maxLength - the max number of chars (including the \0) that can be
//                  placed in the string.
//
// Returns:  Nothing.
//
void BcmMessageLogNonVolSettings::GetTelnetIpStackBitmaskNames(char *pNameString,
                                                               unsigned int maxLength) const
{
    static const char *pIpStackNames[] =
    {
        "IP1",
        "IP2",
        "IP3",
        "IP4",
        "IP5",
        "IP6",
        "IP7",
        "IP8"
    };

    ConvertBitmaskToString(fTelnetIpStackBitmask, pNameString, maxLength,
                           pIpStackNames, sizeof(pIpStackNames) / sizeof(char *));
}

// Helper function to convert the bitmask to string names.
//
// Parameters:
//      pNamesString - pointer to the string into which the names should
//                     be copied.
//      maxLength - the max number of chars (including the \0) that can be
//                  placed in the string.
//
// Returns:  Nothing.
//
void BcmMessageLogNonVolSettings::GetSshIpStackBitmaskNames(char *pNameString,
                                                            unsigned int maxLength) const
{
    static const char *pIpStackNames[] =
    {
        "IP1",
        "IP2",
        "IP3",
        "IP4",
        "IP5",
        "IP6",
        "IP7",
        "IP8"
    };

    ConvertBitmaskToString(fSshIpStackBitmask, pNameString, maxLength,
                           pIpStackNames, sizeof(pIpStackNames) / sizeof(char *));
}

// Helper function to convert the bitmask to string names.
//
// Parameters:
//      pNamesString - pointer to the string into which the names should
//                     be copied.
//      maxLength - the max number of chars (including the \0) that can be
//                  placed in the string.
//
// Returns:  Nothing.
//
void BcmMessageLogNonVolSettings::GetHttpIpStackBitmaskNames(char *pNameString,
                                                            unsigned int maxLength) const
{
    static const char *pIpStackNames[] =
    {
        "IP1",
        "IP2",
        "IP3",
        "IP4",
        "IP5",
        "IP6",
        "IP7",
        "IP8"
    };

    ConvertBitmaskToString(fHttpIpStackBitmask, pNameString, maxLength,
                           pIpStackNames, sizeof(pIpStackNames) / sizeof(char *));
}

// Helper function to convert the bitmask to string names.
//
// Parameters:
//      pNamesString - pointer to the string into which the names should
//                     be copied.
//      maxLength - the max number of chars (including the \0) that can be
//                  placed in the string.
//
// Returns:  Nothing.
//
void BcmMessageLogNonVolSettings::GetHttpAdvancedIpStackBitmaskNames(char *pNameString,
                                                            unsigned int maxLength) const
{
    static const char *pIpStackNames[] =
    {
        "IP1",
        "IP2",
        "IP3",
        "IP4",
        "IP5",
        "IP6",
        "IP7",
        "IP8"
    };

    ConvertBitmaskToString(fHttpAdvancedIpStackBitmask, pNameString, maxLength,
                           pIpStackNames, sizeof(pIpStackNames) / sizeof(char *));
}

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
bool BcmMessageLogNonVolSettings::WriteToImpl(BcmOctetBuffer &outputBuffer, NonVolSection section) const
{
    // There are no dynamic settings, so just return "true" since there were
    // no problems.
    if (section == kDynamicSection)
    {
        if (!outputBuffer.NetworkAddToEnd((uint16)fHttpUserId.BytesUsed())
        ||  !outputBuffer.Append(fHttpUserId)
        ||  !outputBuffer.NetworkAddToEnd((uint16)fHttpUserPassword.BytesUsed())
        ||  !outputBuffer.Append(fHttpUserPassword)
        ||  !outputBuffer.NetworkAddToEnd((uint16)fHttpAdminId.BytesUsed())
        ||  !outputBuffer.Append(fHttpAdminId)
        ||  !outputBuffer.NetworkAddToEnd((uint16)fHttpAdminPassword.BytesUsed())
        ||  !outputBuffer.Append(fHttpAdminPassword)

        // Added with version 0.2
        ||  !outputBuffer.NetworkAddToEnd(fRemoteAccessEnable )
        ||  !outputBuffer.AddToEnd((uint8 *) fRemoteAccessUserName, sizeof(fRemoteAccessUserName)) 
        ||  !outputBuffer.AddToEnd((uint8 *) fRemoteAccessPassword, sizeof(fRemoteAccessPassword)) 
        ||  !outputBuffer.NetworkAddToEnd(fTelnetIpStackBitmask)
        ||  !outputBuffer.NetworkAddToEnd(fSshIpStackBitmask)
        
        // Added with version 0.3 (PR 9961)
        ||  !outputBuffer.NetworkAddToEnd(fRemoteAccessInactivityTimeout ) )
        {
            gErrorMsg(fMessageLogSettings, "WriteToImpl") << "Failed to write Dynamic settings; buffer too small!" << endl;
            return false;
        }
        
        // Added with version 0.4 (PR 8884)
        if (!outputBuffer.NetworkAddToEnd(fHttpIpStackBitmask) ||
            !outputBuffer.NetworkAddToEnd(fHttpAdvancedIpStackBitmask) ||
            !outputBuffer.NetworkAddToEnd((uint16)fHttpPasswordSeed.BytesUsed()) ||
            !outputBuffer.Append(fHttpPasswordSeed))
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "WriteToImpl") 
            << "Buffer too small to hold all of the Dynamic settings!" << endl;

            return false;
        }

        // remote host list size
        if (!outputBuffer.NetworkAddToEnd(uint16(fHttpRemoteIpAcl.AsBuffer().BytesUsed())))
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "WriteToImpl") 
            << "Buffer too small to hold all of the Dynamic settings!" << endl;

            return false;
        }
        
        // remote host list
        if (!outputBuffer.Append(fHttpRemoteIpAcl.AsBuffer()))
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "WriteToImpl") 
            << "Buffer too small to hold all of the Dynamic settings!" << endl;

            return false;
        }
        // password idle timeout
        if (!outputBuffer.NetworkAddToEnd(fHttpPasswordIdleTimeout))
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "WriteToImpl") 
            << "Buffer too small to hold all of the Dynamic settings!" << endl;

            return false;
        }

        // Added with version 0.5
        if (!outputBuffer.NetworkAddToEnd(fOtherLogsEnable))
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "WriteToImpl") 
            << "Buffer too small to hold all of the Dynamic settings!" << endl;

            return false;
        }

        return true;
    }

    // Append the settings one at a time; if any of these fails, then the buffer
    // is probably too small.
    if (!outputBuffer.NetworkAddToEnd(fDefaultEnabledSeverities) ||
        !outputBuffer.NetworkAddToEnd(fDefaultEnabledFields) ||
        !outputBuffer.NetworkAddToEnd(fPermanentRemoteAccessEnable) ||
        !outputBuffer.AddToEnd((uint8 *) fPermanentRemoteAccessUserName, sizeof(fPermanentRemoteAccessUserName)) ||
        !outputBuffer.AddToEnd((uint8 *) fPermanentRemoteAccessPassword, sizeof(fPermanentRemoteAccessPassword)) ||

        // Only need to store these 3 parts of the DHCP settings.
        !fIpAddress.WriteTo(outputBuffer) ||
        !fSubnetMask.WriteTo(outputBuffer) ||
        !fRouter.WriteTo(outputBuffer) ||

        !outputBuffer.NetworkAddToEnd(fRemoteAccessIpStackNumber) ||

        // PR4517 - added with version 0.2
        !outputBuffer.NetworkAddToEnd(fTelnetIpStackBitmask))
    {
        // Log the error.
        gErrorMsg(fMessageLogSettings, "WriteToImpl") << "Buffer too small to hold all of the settings!" << endl;

        return false;
    }

    // If we got this far, then it worked!
    return true;
}


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
bool BcmMessageLogNonVolSettings::ReadFromImpl(const BcmOctetBuffer &inputBuffer, 
                                               unsigned int &currentOffset, NonVolSection section)
{
    // There are no dynamic settings at this point.
    if (section == kPermanentSection)
    {
        // For this object, we skipped 0.0, so this should never be read.  If it
        // is 0.0, then this is a problem, so we bail with comment.
        if ((fVersionMajor[section] == 0) && (fVersionMinor[section] == 0))
        {
            gErrorMsg(fMessageLogSettings, "ReadFromImpl")
                << "Read unsupported version (0.0)!  These settings are not valid!" << endl;

            return false;
        }

        // Read the settings one at a time; if any of these fails, then the buffer
        // is probably too small.
        if (!inputBuffer.NetworkRead(currentOffset, fDefaultEnabledSeverities) ||
            !inputBuffer.NetworkRead(currentOffset, fDefaultEnabledFields) ||
            !inputBuffer.NetworkRead(currentOffset, fPermanentRemoteAccessEnable) ||
            !inputBuffer.Read(currentOffset, (uint8 *) fPermanentRemoteAccessUserName, sizeof(fPermanentRemoteAccessUserName)) ||
            !inputBuffer.Read(currentOffset, (uint8 *) fPermanentRemoteAccessPassword, sizeof(fPermanentRemoteAccessPassword)) ||
            
            // These are the only 3 settings that we read.
            !fIpAddress.ReadFrom(inputBuffer, currentOffset) ||
            !fSubnetMask.ReadFrom(inputBuffer, currentOffset) ||
            !fRouter.ReadFrom(inputBuffer, currentOffset) ||
            
            !inputBuffer.NetworkRead(currentOffset, fRemoteAccessIpStackNumber) )
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                << "Buffer too small; doesn't have all of the settings!" << endl;

            return false;
        }

        // Store the defaults in the message log settings object.
        BcmMessageLogSettings::SetDefaultSeverities(fDefaultEnabledSeverities);
        BcmMessageLogSettings::SetDefaultFields(fDefaultEnabledFields);

        // PR4517 - added with version 0.2
        if (MyVersionIsGreaterThanOrEqualTo(0, 2, section))
        {
            // this has been moved to dynamic non-vol
            uint8 dummyTelnetIpStackBitmask;
            if (!inputBuffer.NetworkRead(currentOffset, dummyTelnetIpStackBitmask))
            {
                gErrorMsg(fMessageLogSettings, "ReadFromImpl")
                    << "Buffer too small; doesn't have all of the settings!" << endl;

                return false;
            }
        }

        // Do whatever validation is necessary for the values.
    }
    else // Dynamic
    {
        // Read 0.1 settings
        if (MyVersionIsGreaterThanOrEqualTo(0, 1, section))
        {
            uint16 Length;
            
            /* read HTTP user ID */
            
            // Read the length
            if (!inputBuffer.NetworkRead(currentOffset, Length))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP user ID size." << endl;
                return false;
            }
            
            // Resize the buffer for the value.
            fHttpUserId.ResizeBuffer(Length);
            
            // Read the value.
            if (!inputBuffer.Read(currentOffset, fHttpUserId.AccessBuffer(), fHttpUserId.BufferSize()))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP user ID value." << endl;
                return false;
            }
            
            // Lock in new size
            fHttpUserId.SetBytesUsed(fHttpUserId.BufferSize());
          
            /* read user password */
            
            // Read the length
            if (!inputBuffer.NetworkRead(currentOffset, Length))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP user password size." << endl;
                return false;
            }
            
            // Resize the buffer for the value.
            fHttpUserPassword.ResizeBuffer(Length);
            
            // Read the value.
            if (!inputBuffer.Read(currentOffset, fHttpUserPassword.AccessBuffer(), fHttpUserPassword.BufferSize()))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP user password value." << endl;
                return false;
            }
            
            // Lock in new size
            fHttpUserPassword.SetBytesUsed(fHttpUserPassword.BufferSize());
            
            /* read admin ID */
            
            // Read the length
            if (!inputBuffer.NetworkRead(currentOffset, Length))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP admin ID size." << endl;
                return false;
            }
            
            // Resize the buffer for the value.
            fHttpAdminId.ResizeBuffer(Length);
            
            // Read the value.
            if (!inputBuffer.Read(currentOffset, fHttpAdminId.AccessBuffer(), fHttpAdminId.BufferSize()))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP admin ID value." << endl;
                return false;
            }
            
            // Lock in new size
            fHttpAdminId.SetBytesUsed(fHttpAdminId.BufferSize());
            
            /* read admin password */
            
            // Read the length
            if (!inputBuffer.NetworkRead(currentOffset, Length))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP admin password size." << endl;
                return false;
            }
            
            // Resize the buffer for the value.
            fHttpAdminPassword.ResizeBuffer(Length);
            
            // Read the value.
            if (!inputBuffer.Read(currentOffset, fHttpAdminPassword.AccessBuffer(), fHttpAdminPassword.BufferSize()))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP admin password value." << endl;
                return false;
            }
            
            // Lock in new size
            fHttpAdminPassword.SetBytesUsed(fHttpAdminPassword.BufferSize());
        }

        // Read 0.2 settings
        if (MyVersionIsGreaterThanOrEqualTo(0, 2, section))
        {
            if ( !inputBuffer.NetworkRead(currentOffset, fRemoteAccessEnable) ||
                 !inputBuffer.Read(currentOffset, (uint8 *) fRemoteAccessUserName, sizeof(fRemoteAccessUserName)) ||
                 !inputBuffer.Read(currentOffset, (uint8 *) fRemoteAccessPassword, sizeof(fRemoteAccessPassword)) ||
                 !inputBuffer.Read(currentOffset, fTelnetIpStackBitmask) ||
                 !inputBuffer.Read(currentOffset, fSshIpStackBitmask) )
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read remote access enable/username/password." << endl;
                return false;
            }
        }
        
        // Read 0.3 settings (PR 9961)
        if (MyVersionIsGreaterThanOrEqualTo(0, 3, section))
        {
            if ( !inputBuffer.NetworkRead(currentOffset, fRemoteAccessInactivityTimeout) )
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read remote access inactivity timeout." << endl;
                return false;
            }
        }
        
        // Read 0.4 settings (PR 8884)
        if (MyVersionIsGreaterThanOrEqualTo(0, 4, section))
        {
           uint16 Length;
           
           if ( !inputBuffer.NetworkRead(currentOffset, fHttpIpStackBitmask) ||
                !inputBuffer.NetworkRead(currentOffset, fHttpAdvancedIpStackBitmask))
           {
              gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP interfaces/advanced bitmasks." << endl;
              return false;
           }


           /* read HTTP password seed */

           // Read the length
           if (!inputBuffer.NetworkRead(currentOffset, Length))
           {
               // Log the error.
               gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP password seed size." << endl;
               return false;
           }

           // Resize the buffer for the value.
           if(Length > 0)
           {
              fHttpPasswordSeed.ResizeBuffer(Length);
           }

           // Read the value.
           if (!inputBuffer.Read(currentOffset, fHttpPasswordSeed.AccessBuffer(), Length))
           {
               // Log the error.
               gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP password seed value." << endl;
               return false;
           }

           // Lock in new size
           fHttpPasswordSeed.SetBytesUsed(Length);



           if ( !inputBuffer.NetworkRead(currentOffset, Length) /* remote hosts buffer size */ )
           {
              gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP remote IP acl length." << endl;
              return false;
           }

           // resize the buffer as appropriate
           BcmOctetBuffer remoteHosts;
           // TRW - if the length is zero, NO memory is allocated for the buffer, and Append() with the
           // resize flag doesn't work!  So only resize if we have a non-zero length
           if(Length > 0)
           {
              remoteHosts.ResizeBuffer(Length);
           }

           if ( !inputBuffer.Read(currentOffset,remoteHosts.AccessBuffer(),Length) )
           {
               // Log the error.
              gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP remote IP acl." << endl;
               return false;
           }

           remoteHosts.SetBytesUsed(Length);
           // now convert to IP address list
           fHttpRemoteIpAcl = remoteHosts;
           
           if ( !inputBuffer.NetworkRead(currentOffset, fHttpPasswordIdleTimeout))
           {
              gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read HTTP password inactivity timeout." << endl;
              return false;
           }
        }

        // Read 0.5 settings 
        if (MyVersionIsGreaterThanOrEqualTo(0, 5, section))
        {
            if ( !inputBuffer.NetworkRead(currentOffset, fOtherLogsEnable) )
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") << "Failed to read OtherLogs Enable/Disable flags." << endl;
                return false;
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
ostream & BcmMessageLogNonVolSettings::PrintImpl(ostream & outputStream) const
{
    #if !(BCM_REDUCED_IMAGE_SIZE)

    char severitiesNames[256], fieldsNames[128];
    char telnetIpStackBitmaskNames[128];    // PR4517
    char sshIpStackBitmaskNames[128];    // PR4517
    char httpIpStackBitmaskNames[128];
    char httpAdvancedIpStackBitmaskNames[128];

    BcmMessageLogSettings::GetEnabledSeveritiesNames(fDefaultEnabledSeverities, severitiesNames, sizeof(severitiesNames));
    BcmMessageLogSettings::GetEnabledFieldsNames(fDefaultEnabledFields, fieldsNames, sizeof(fieldsNames));

    // PR4517
    GetTelnetIpStackBitmaskNames(telnetIpStackBitmaskNames, sizeof(telnetIpStackBitmaskNames));
    GetSshIpStackBitmaskNames(sshIpStackBitmaskNames, sizeof(sshIpStackBitmaskNames));
    GetHttpIpStackBitmaskNames(httpIpStackBitmaskNames, sizeof(httpIpStackBitmaskNames));
    GetHttpAdvancedIpStackBitmaskNames(httpAdvancedIpStackBitmaskNames, sizeof(httpAdvancedIpStackBitmaskNames));

    // Dump the settings out one at a time.

    outputStream    << "\nUser Interface Dynamic NonVol Settings:\n\n"
         << setw(26) << "HTTP User ID = '" << BcmString(HttpUserId()) << "'\n"
         << setw(26) << "HTTP User Password = '" << BcmString(HttpUserPassword()) << "'\n";
    outputStream
         << setw(26) << "HTTP Admin ID = '" << ((kDefaultValue_HttpAdminAccessEnabled)?BcmString(HttpAdminId()):"(DISABLED)") << "'\n"
         << setw(26) << "HTTP Admin Password = '" << ((kDefaultValue_HttpAdminAccessEnabled)?BcmString(HttpAdminPassword()):"(DISABLED)") << "'\n"

         // Added with version 0.2
         << setw(35) << "Remote Access User Name = " << "'" << fRemoteAccessUserName << "'\n"
#ifdef BCM_TELNET_SSH_POTD
         << setw(35) << "Remote Access Password = " << "'" << "<Not Supported in this Mode>" << "'\n"
#else         
         << setw(35) << "Remote Access Password = " << "'" << fRemoteAccessPassword << "'\n"
#endif
         << setw(35) << "Remote Access inactivity timeout = " << "'" << RemoteAccessInactivityTimeout() << "'\n" // 0.3, PR 9961
         << setw(35) << "Telnet Access Enabled = " << TelnetRemoteAccessEnabled() << '\n'
         << setw(35) << "Telnet IP Stack enable = " << inHex(fTelnetIpStackBitmask) << " (" << telnetIpStackBitmaskNames << ")\n"
         << setw(35) << "SSH Access Enabled = " << SshRemoteAccessEnabled() << '\n'
         << setw(35) << "SSH IP Stack enable = " << inHex(fSshIpStackBitmask) << " (" << sshIpStackBitmaskNames << ")\n"
         << setw(35) << "HTTP Access Enabled = " << HttpRemoteAccessEnabled() << '\n';
        // Added with version 0.4
    outputStream
         << setw(35) << "HTTP IP stack enable = " << inHex(fHttpIpStackBitmask) << " (" << httpIpStackBitmaskNames << ")\n"
         << setw(35) << "HTTP Advanced IP stack enable = " << inHex(fHttpAdvancedIpStackBitmask) << " (" << httpAdvancedIpStackBitmaskNames << ")\n"
         << setw(35) << "HTTP Password Seed = '" << BcmString(HttpPasswordSeed()) << "'\n"
         << setw(35) << "HTTP Remote IP ACL (emtpy == allow all)::\n" << fHttpRemoteIpAcl << '\n'
         << setw(35) << "HTTP Password Idle Timeout = " << fHttpPasswordIdleTimeout << '\n';

    outputStream
         << setw(35) << "Exception Logging Enabled = " << ExceptionLogEnable() << '\n';

    outputStream 
        << "\nUser Interface Permanent NonVol Settings:\n\n"
        << setw(35) << "Default Severities = " << inHex(fDefaultEnabledSeverities) << " (" << severitiesNames << ")\n"
        << setw(35) << "Default Fields = " << inHex(fDefaultEnabledFields) << " (" << fieldsNames << ")\n"
        << setw(35) << "Remote Access IP address = " << fIpAddress << '\n'
        << setw(35) << "Remote Access Subnet Mask = " << fSubnetMask << '\n'
        << setw(35) << "Remote Access Router address = " << fRouter << '\n'
        << setw(35) << "Remote Access IP Stack Number = " << (int) fRemoteAccessIpStackNumber << '\n';

    #else

    outputStream
        << "\nNonvol Dump:\n\n"
        << BcmString(HttpUserId()) << '\n'
        << BcmString(HttpUserPassword()) << '\n'
        << BcmString(HttpAdminId()) << '\n'
        << BcmString(HttpAdminPassword()) << '\n'
        << fRemoteAccessUserName << '\n'
#ifdef BCM_TELNET_SSH_POTD
#else        
        << fRemoteAccessPassword << '\n'
#endif
        << RemoteAccessInactivityTimeout() << '\n' // 0.3, PR 9961
        << TelnetRemoteAccessEnabled() << '\n'
        << inHex(fTelnetIpStackBitmask) << '\n'
        << SshRemoteAccessEnabled() << '\n'
        << inHex(fSshIpStackBitmask) << '\n'
        << HttpRemoteAccessEnabled() << '\n'
        << inHex(fHttpIpStackBitmask) << '\n'
        << inHex(fHttpAdvancedIpStackBitmask) << '\n'
        << BcmString(HttpPasswordSeed()) << '\n'
        << fHttpRemoteIpAcl << '\n'
        << fHttpPasswordIdleTimeout << '\n'
        << ExceptionLogEnable() << '\n'
        << inHex(fDefaultEnabledSeverities) << '\n'
        << inHex(fDefaultEnabledFields) << '\n'
        << fIpAddress << '\n'
        << fSubnetMask << '\n'
        << fRouter << '\n'
        << fRemoteAccessIpStackNumber << '\n';

    #endif
    
    return outputStream;
}



