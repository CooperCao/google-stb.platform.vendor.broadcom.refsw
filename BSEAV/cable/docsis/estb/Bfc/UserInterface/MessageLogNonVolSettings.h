//****************************************************************************
//
//  Copyright (c) 2000-2004  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       MessageLogNonVolSettings.h
//  Author:         David Pullen
//  Creation Date:  March 7, 2000
//
//****************************************************************************

#ifndef MESSAGELOGNONVOLSETTINGS_H
#define MESSAGELOGNONVOLSETTINGS_H

//********************** Include Files ***************************************

// My base class.
#include "NonVolSettings.h"

#include "IpAddress.h"
#include "IpAddressList.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


/** \ingroup BfcNonVol
*
*   These are the Non-Volatile settings for the Message Logging subsystem.
*   This is derived from the abstract base NonVolSettings class, which
*   provides hooks for loading and storing the settings.
*
*   Note that the settings are independant of the device to which they are
*   being stored, allowing the same application settings to be used on
*   multiple target platforms and devices!
*/
class BcmMessageLogNonVolSettings : public BcmNonVolSettings
{
public:

    /// Default Constructor.  Initializes the state of the object, setting all
    /// fields to good default values.
    ///
    BcmMessageLogNonVolSettings(void);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmMessageLogNonVolSettings();

    /// Set all values to a known good state.
    ///
    /// Note that the derived class should set the version numbers to the desired
    /// values in case they were changed (by ReadFrom(), etc).
    ///
    /// This method may be called if ReadFrom() returns false in order to ensure
    /// that there are valid settings in the object.
    ///
    /// \param
    ///      section - the section that should be reset.
    ///
    virtual void ResetDefaults(NonVolSection section);

    /// Returns the pointer to the singleton instance for this class.  Most
    /// objects in the system will use this method rather than being passed a
    /// pointer to it.  The singleton pointer will be set up in the base-class
    /// constructor.
    ///
    /// NOTES:  This can return NULL if a singleton has not been set up for the
    ///         system, so you must check for this condition.
    ///
    ///         You must not delete this object!
    ///
    ///         You should not store a pointer to the object that is returned,
    ///         since it may be deleted and replaced with a new one.
    ///
    /// \return
    ///      A pointer to the instance that should be used by the system.
    ///
    static BcmMessageLogNonVolSettings *GetSingletonInstance(void);

public:

    //@{
    /// Accessors for the severities value.
    ///
    /// \param
    ///      severities - the new severities bitmask to be used.
    ///
    /// \return
    ///      The severities bitmask.
    ///
    void Severities(uint32 severities);
    uint32 Severities(void) const;
    //@}

    //@{
    /// Accessors for the fields value.
    ///
    /// \param
    ///      fields - the new fields bitmask to be used.
    ///
    /// \return
    ///      The fields bitmask.
    ///
    void Fields(uint8 fields);
    uint8 Fields(void) const;
    //@}

    //@{
    /// Accessors for the HTTP remote access enable flag.
    ///
    /// \param
    ///      enable - set to true to enable this, false to disable.
    ///
    /// \return
    ///      true if enabled.
    /// \return
    ///      false if disabled.
    ///
    void HttpRemoteAccessEnabled(bool enable);
    bool HttpRemoteAccessEnabled(void) const;
    //@}

    //@{
    /// Accessors for the SSH remote access enable flag.
    ///
    /// \param
    ///      enable - set to true to enable this, false to disable.
    ///
    /// \return
    ///      true if enabled.
    /// \return
    ///      false if disabled.
    ///
    void SshRemoteAccessEnabled(bool enable);
    bool SshRemoteAccessEnabled(void) const;
    //@}

    //@{
    /// Accessors for the remote access DHCP settings values.
    ///
    /// \param
    ///      ipAddress - the IP address value to use.
    ///
    /// \return
    ///      The IP address value that is being used.
    ///
    void IpAddress(const BcmIpAddress &ipAddress);
    const BcmIpAddress &IpAddress(void) const;

    void SubnetMask(const BcmIpAddress &ipAddress);
    const BcmIpAddress &SubnetMask(void) const;

    void Router(const BcmIpAddress &ipAddress);
    const BcmIpAddress &Router(void) const;
    //@}

    //@{
    /// Acessors for the IP stack number.  The is the IP stack number that the
    /// remote access server will use when sending and receiving packets.
    ///
    /// \param
    ///      ipStackNumber - the IP stack number to be used.
    ///
    /// \return
    ///      The IP stack number that is being used.
    ///
    void RemoteAccessIpStackNumber(uint8 ipStackNumber);
    uint8 RemoteAccessIpStackNumber(void) const;
    //@}

public:     // PR4517 - Settings for the Telnet Server.

    //@{
    /// Accessors for the telnet remote access enable flag.  Note that this only
    /// affects whether or not the telnet server will be started when the system
    /// boots up.  If disabled, the server can be started from the serial
    /// console or other mechanism (e.g. MIB) after the system is running.
    ///
    /// \param
    ///      enable - set to true to enable this, false to disable.
    ///
    /// \return
    ///      true if enabled.
    /// \return
    ///      false if disabled.
    ///
    void TelnetRemoteAccessEnabled(bool enable);
    bool TelnetRemoteAccessEnabled(void) const;
    //@}

    //@{
    /// Accessors for the remote access user name.  This field is limited to
    /// 16 chars, including the \0 terminator.
    ///
    /// \param
    ///      pUserName - pointer to the new user name string to be stored.
    ///
    /// \return
    ///      The current user name.
    ///
    void RemoteAccessUserName(const char *pUserName);
    const char *RemoteAccessUserName(void) const;
    //@}

    //@{
    /// Accessors for the remote access password.  This field is limited to
    /// 16 chars, including the \0 terminator.
    ///
    /// \param
    ///      pPassword - pointer to the new password string to be stored.
    ///
    /// \return
    ///      The current password.
    ///
    void RemoteAccessPassword(const char *pPassword);
    const char *RemoteAccessPassword(void) const;
    //@}
    
    // PR 9961
    //@{
    /// Accessors for the remote access inactivity timeout.  A session which
    /// has no activity for greater than the number of seconds specified will
    /// be terminated.
    ///
    /// \param
    ///      Seconds - the number of seconds after which an inactive session
    ///                will be terminated.
    ///
    /// \return
    ///      The current inactivity timeout.
    ///
    void RemoteAccessInactivityTimeout(int Seconds);
    const int RemoteAccessInactivityTimeout(void) const;
    //@}

    /// Bits for the IP stacks that Telnet should allow connections on.
    enum
    {
        kTelnetIpStack1 = 0x01,
        kTelnetIpStack2 = 0x02,
        kTelnetIpStack3 = 0x04,
        kTelnetIpStack4 = 0x08,
        kTelnetIpStack5 = 0x10,
        kTelnetIpStack6 = 0x20,
        kTelnetIpStack7 = 0x40,
        kTelnetIpStack8 = 0x80,

        kTelnetIpStackAll = 0xff
    };

    enum
    {
        kSshIpStack1 = 0x01,
        kSshIpStack2 = 0x02,
        kSshIpStack3 = 0x04,
        kSshIpStack4 = 0x08,
        kSshIpStack5 = 0x10,
        kSshIpStack6 = 0x20,
        kSshIpStack7 = 0x40,
        kSshIpStack8 = 0x80,

        kSshIpStackAll = 0xff
    };

    //@{
    /// Accessors for the IP stacks that you want the Telnet server to accept
    /// connections on.
    ///
    /// \param
    ///      ipStackBitmask - bitmask of the stacks that the Telnet server
    ///                       should accept connections on.
    ///
    /// \return
    ///      The bits corresponding to the IP stacks that we will accept
    ///      connections on.
    ///
    void TelnetIpStackBitmask(uint8 ipStackBitmask);
    uint8 TelnetIpStackBitmask(void) const;
    //@}

    //@{
    /// Accessors for the IP stacks that you want the SSH server to accept
    /// connections on.
    ///
    /// \param
    ///      ipStackBitmask - bitmask of the stacks that the SSH server
    ///                       should accept connections on.
    ///
    /// \return
    ///      The bits corresponding to the IP stacks that we will accept
    ///      connections on.
    ///
    void SshIpStackBitmask(uint8 ipStackBitmask);
    uint8 SshIpStackBitmask(void) const;
    //@}

    enum
    {
       kHttpIpStack1 = 0x01,
       kHttpIpStack2 = 0x02,
       kHttpIpStack3 = 0x04,
       kHttpIpStack4 = 0x08,
       kHttpIpStack5 = 0x10,
       kHttpIpStack6 = 0x20,
       kHttpIpStack7 = 0x40,
       kHttpIpStack8 = 0x80,

       kHttpIpStackAll = 0xff
    };
    
    // Added with version 0.4
    void HttpIpStackBitmask(uint8 ipStackBitmask);
    uint8 HttpIpStackBitmask(void) const;
    
    void HttpAdvancedIpStackBitmask(uint8 ipStackBitmask);
    uint8 HttpAdvancedIpStackBitmask(void) const;
    
    void HttpPasswordSeed (const BcmOctetBuffer &Seed);
    const BcmOctetBuffer &HttpPasswordSeed (void) const;
    
    void HttpRemoteIpAcl(BcmIpAddressList &remoteHosts);
    BcmIpAddressList &HttpRemoteIpAcl(void);

    bool HttpRemoteHostIsAuthorized(BcmIpAddress host);

    bool HttpAddAclHost(BcmIpAddress host);
    bool HttpAddAclHostRange(BcmIpAddress start, BcmIpAddress end);
    bool HttpRemoveAclHost(BcmIpAddress host);
    bool HttpRemoveAclHostRange(BcmIpAddress start, BcmIpAddress end);
    void HttpClearRemoteIpAcl(void);
    
    void HttpPasswordIdleTimeout(uint32 timeout);
    uint32 HttpPasswordIdleTimeout(void) const;
    
    /// Helper function to convert the bitmask to string names.
    ///
    /// \param
    ///      pNamesString - pointer to the string into which the names should
    ///                     be copied.
    /// \param
    ///      maxLength - the max number of chars (including the \0) that can be
    ///                  placed in the string.
    ///
    void GetTelnetIpStackBitmaskNames(char *pNameString, unsigned int maxLength) const;

    /// Helper function to convert the bitmask to string names.
    ///
    /// \param
    ///      pNamesString - pointer to the string into which the names should
    ///                     be copied.
    /// \param
    ///      maxLength - the max number of chars (including the \0) that can be
    ///                  placed in the string.
    ///
    void GetSshIpStackBitmaskNames(char *pNameString, unsigned int maxLength) const;

    /// Helper function to convert the bitmask to string names.
    ///
    /// \param
    ///      pNamesString - pointer to the string into which the names should
    ///                     be copied.
    /// \param
    ///      maxLength - the max number of chars (including the \0) that can be
    ///                  placed in the string.
    ///
    void GetHttpIpStackBitmaskNames(char *pNameString, unsigned int maxLength) const;

    /// Helper function to convert the bitmask to string names.
    ///
    /// \param
    ///      pNamesString - pointer to the string into which the names should
    ///                     be copied.
    /// \param
    ///      maxLength - the max number of chars (including the \0) that can be
    ///                  placed in the string.
    ///
    void GetHttpAdvancedIpStackBitmaskNames(char *pNameString, unsigned int maxLength) const;

    //@{
    /// Accessors for HTTP user IDs and passwords.  These were previously 
    /// hard-coded in VendorHttpControls.h, from which we still get the
    /// default values.
    ///
    /// \param
    ///   Id, Password:  The user ID or password.
    ///
    /// \return
    ///   The user ID or password.
    ///
    const BcmOctetBuffer &HttpUserId (void) const;
    void HttpUserId (const BcmOctetBuffer &Id);
    const BcmOctetBuffer &HttpAdminId (void) const;
    void HttpAdminId (const BcmOctetBuffer &Id);
    
    const BcmOctetBuffer &HttpUserPassword (void) const;
    void HttpUserPassword (const BcmOctetBuffer &Password);
    const BcmOctetBuffer &HttpAdminPassword (void) const;
    void HttpAdminPassword (const BcmOctetBuffer &Id);
    //@}

    // Accessors to enable/disable the Exception Logging.
    //
    // Parameters:
    //      enable - true to enable Exception Logging, false to disable.
    //
    // Returns:
    //      The enable state.
    //
    bool ExceptionLogEnable(void) const;
    void ExceptionLogEnable(bool enable);


protected:

    /// Here's where I write my members to the buffer in a flat format, in the
    /// order described at the top of this file.  All multibyte values are
    /// written in network byte order.
    ///
    /// \param
    ///      outputBuffer - the buffer into which the settings should be written.
    /// \param
    ///      section - the section that should be written.
    ///
    /// \return
    ///      true if successful.
    /// \return
    ///      false if there was a problem (usually the buffer is too small).
    ///
    virtual bool WriteToImpl(BcmOctetBuffer &outputBuffer, NonVolSection section) const;

    /// Here's where I read my members from the buffer in a flat format, in the
    /// order described at the top of this file.  All multibyte values are read
    /// in network byte order.
    ///
    /// The currentOffset parameter tells the derived class where the app data
    /// starts in the buffer.  It can be used directly in the calls to Read() or
    /// NetworkRead() in BcmOctetBuffer (which expects an offset value).
    ///
    /// \param
    ///      inputBuffer - the buffer from which the settings should be read.
    /// \param
    ///      currentOffset - the offset into the buffer from which the settings
    ///                      should be read.  This is necessary to allow the
    ///                      derived class to skip the header.
    /// \param
    ///      section - the section that should be read.
    ///
    /// \return
    ///      true if successful.
    /// \return
    ///      false if there was a problem (version number not supported, not
    ///          enough bytes in the buffer, etc.).
    ///
    virtual bool ReadFromImpl(const BcmOctetBuffer &inputBuffer, 
                              unsigned int &currentOffset, NonVolSection section);

    /// Causes me to do formatted output of my settings to the specified ostream,
    /// after the base class does its stuff.
    ///
    /// \param
    ///      outputStream - the ostream to be used.
    ///
    /// \return
    ///      The reference to the ostream that was passed in.
    ///
    virtual ostream & PrintImpl(ostream & outputStream) const;

protected:  // Permanent nonvol settings.

    /// These determine which severity levels will be displayed and which will
    /// be suppressed.  These correspond to the SeverityBits enum.
    uint32 fDefaultEnabledSeverities;

    /// These determine which fields (timestamp, module name, function name,
    /// etc.), are displayed.  These correspond to the Fields enum.
    uint8 fDefaultEnabledFields;

    /// Flags for remote access enable/disable.  If all are disabled, then no
    /// remote access will be allowed.
    enum
    {
        kTelnetRemoteAccessEnabled = 0x01,
        kHttpRemoteAccessEnabled   = 0x02,
        kSshRemoteAccessEnabled   = 0x04,
    };

    /// Store the flags.
    uint8 fRemoteAccessEnable;
    uint8 fPermanentRemoteAccessEnable;

    /// Remote access user name.
    char fRemoteAccessUserName[16];
    char fPermanentRemoteAccessUserName[16];

    /// Remote access password.
    char fRemoteAccessPassword[16];
    char fPermanentRemoteAccessPassword[16];
    
    /// Remote access inactivity timeout
    int fRemoteAccessInactivityTimeout; // PR 9961

    //@{
    /// Remote access IP stack settings (IP address, subnet mask, router address).
    BcmIpAddress fIpAddress;
    BcmIpAddress fSubnetMask;
    BcmIpAddress fRouter;
    //@}

    /// Remote access IP stack number.
    uint8 fRemoteAccessIpStackNumber;

    /// PR4517 - Added with version 0.2
    ///
    /// These control the IP stacks that Telnet will use.
    uint8 fTelnetIpStackBitmask;

    /// Added with version 0.3
    ///
    /// These control the IP stacks that SSH will use.
    uint8 fSshIpStackBitmask;

    //@{
    /// For CW26 - need username and password that are settable for CM/RG products
    /// User ID's and passwords.
    /// These are in the Dynamic Section (version 0.1)
    BcmOctetBuffer fHttpUserId;
    BcmOctetBuffer fHttpUserPassword;
    BcmOctetBuffer fHttpAdminId;
    BcmOctetBuffer fHttpAdminPassword;
    //@}

    // Added with version 0.4
    uint8 fHttpIpStackBitmask;
    uint8 fHttpAdvancedIpStackBitmask;
    BcmOctetBuffer fHttpPasswordSeed;
    BcmIpAddressList fHttpRemoteIpAcl;
    uint32 fHttpPasswordIdleTimeout;

    // Added with version 0.5
    // Other Logs enable/disable.  
    enum
    {
        kExceptionLogEnabled = 0x01
    };

    /// Store the flags.
    uint8 fOtherLogsEnable;


private:

    /// This is the singleton instance of this class.
    static BcmMessageLogNonVolSettings *pfSingletonInstance;

private:

    /// Copy Constructor.  Not supported.
    BcmMessageLogNonVolSettings(const BcmMessageLogNonVolSettings &otherInstance);

    /// Assignment operator.  Not supported.
    BcmMessageLogNonVolSettings & operator = (const BcmMessageLogNonVolSettings &otherInstance);

};


//********************** Inline Method Implementations ***********************


// This allows you to do the following:
//
//    cout << settingsInstance << endl;
//
inline ostream & operator << (ostream &outputStream, const BcmMessageLogNonVolSettings &settingsInstance)
{
    return settingsInstance.Print(outputStream);
}


#endif


