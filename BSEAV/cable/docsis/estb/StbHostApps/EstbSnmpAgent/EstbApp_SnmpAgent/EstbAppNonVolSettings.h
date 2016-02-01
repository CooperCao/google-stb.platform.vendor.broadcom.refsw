//****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
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
//  Filename:       EstbAppNonVolSettings.h
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      These are the Non-Volatile settings for the eSTB SNMP Agent
//      application.  This is derived from the abstract base NonVolSettings
//      class, which provides hooks for loading and storing the settings.
//
//      This class is the parent of several other settings classes, and is what
//      determines order in which they will be written/read. 
//
//      Note that the settings are independant of the device to which they are
//      being stored, allowing the same application settings to be used on
//      multiple target platforms and devices!
//
//****************************************************************************

#ifndef ESTBAPPNONVOLSETTINGS_H
#define ESTBAPPNONVOLSETTINGS_H

//********************** Include Files ***************************************

// My base class.
#include "NonVolSettings.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


class BcmEstbAppNonVolSettings : public BcmNonVolSettings
{
public:

    // Default Constructor.  Initializes the state of the object, setting all
    // fields to good default values.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmEstbAppNonVolSettings(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmEstbAppNonVolSettings();

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
    static BcmEstbAppNonVolSettings *GetSingletonInstance(void);

public:

    // Accessors for the auto-stop-at-console enabled bool.
    //
    // Parameters:
    //      enable - set to true to enable this feature, false to disable.
    //
    // Returns:
    //      true if enabled.
    //      false if disabled.
    //
    inline bool AutoStopAtConsole(void) const;
    inline void AutoStopAtConsole(bool enable);

    // Accessors for the driver init prompt enabled bool.
    //
    // Parameters:
    //      enable - set to true to enable this feature, false to disable.
    //
    // Returns:
    //      true if enabled.
    //      false if disabled.
    //
    inline bool DriverInitPrompt(void) const;
    inline void DriverInitPrompt(bool enable);

    // Accessors for the stop at console prompt enabled bool.
    //
    // Parameters:
    //      enable - set to true to enable this feature, false to disable.
    //
    // Returns:
    //      true if enabled.
    //      false if disabled.
    //
    inline bool StopAtConsolePrompt(void) const;
    inline void StopAtConsolePrompt(bool enable);

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
    virtual ostream & PrintImpl(ostream & outputStream) const;

private:

    // Permanent settings.

    // Bitmask containing some enable/disable bits.
    uint8 fEnableDisableBitmask;

    // The bits for the bitmask.
    enum
    {
        // If set, the app will automatically stop at the console without user
        // intervention.
        kAutoConsoleEnable = 0x01,

        // If set, the "skip driver initialization" prompt will not be
        // displayed.  This means that you can't skip loading the drivers that
        // are enabled in nonvol, but it speeds up boot time by 1.5 seconds.
        kSkipDriverInitPromptDisable = 0x02,

        // If set, the "stop at console" prompt will not be displayed.  This
        // means that you can't stop the CM app from beginning the downstream
        // scan, but is speeds up boot time by 2 seconds.  Note that if you
        // enable the auto-console bit, then it will still stop automatically.
        kStopAtAppPromptDisable = 0x04

    };

private:

    // This is the singleton instance of this class.
    static BcmEstbAppNonVolSettings *pfSingletonInstance;

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmEstbAppNonVolSettings(const BcmEstbAppNonVolSettings &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmEstbAppNonVolSettings & operator = (const BcmEstbAppNonVolSettings &otherInstance);

};


//********************** Inline Method Implementations ***********************


// Accessors for the auto-stop-at-console enabled bool.
//
// Parameters:
//      enable - set to true to enable this feature, false to disable.
//
// Returns:
//      true if enabled.
//      false if disabled.
//
inline bool BcmEstbAppNonVolSettings::AutoStopAtConsole(void) const
{
    if (fEnableDisableBitmask & kAutoConsoleEnable)
    {
        return true;
    }

    return false;
}


inline void BcmEstbAppNonVolSettings::AutoStopAtConsole(bool enable)
{
    if (enable)
    {
        fEnableDisableBitmask |= kAutoConsoleEnable;
    }
    else
    {
        fEnableDisableBitmask &= ~kAutoConsoleEnable;
    }
}


// Accessors for the driver init prompt enabled bool.
//
// Parameters:
//      enable - set to true to enable this feature, false to disable.
//
// Returns:
//      true if enabled.
//      false if disabled.
//
inline bool BcmEstbAppNonVolSettings::DriverInitPrompt(void) const
{
    // This is a disable bit, so I return the opposite.
    if (fEnableDisableBitmask & kSkipDriverInitPromptDisable)
    {
        return false;
    }

    return true;
}


inline void BcmEstbAppNonVolSettings::DriverInitPrompt(bool enable)
{
    // This is a disable bit, so I set the opposite.
    if (enable == false)
    {
        fEnableDisableBitmask |= kSkipDriverInitPromptDisable;
    }
    else
    {
        fEnableDisableBitmask &= ~kSkipDriverInitPromptDisable;
    }
}


// Accessors for the stop at console prompt enabled bool.
//
// Parameters:
//      enable - set to true to enable this feature, false to disable.
//
// Returns:
//      true if enabled.
//      false if disabled.
//
inline bool BcmEstbAppNonVolSettings::StopAtConsolePrompt(void) const
{
    // This is a disable bit, so I return the opposite.
    if (fEnableDisableBitmask & kStopAtAppPromptDisable)
    {
        return false;
    }

    return true;
}

inline void BcmEstbAppNonVolSettings::StopAtConsolePrompt(bool enable)
{
    // This is a disable bit, so I set the opposite.
    if (enable == false)
    {
        fEnableDisableBitmask |= kStopAtAppPromptDisable;
    }
    else
    {
        fEnableDisableBitmask &= ~kStopAtAppPromptDisable;
    }
}

// This allows you to do the following:
//
//    cout << settingsInstance << endl;
//
inline ostream & operator << (ostream &outputStream, const BcmEstbAppNonVolSettings &settingsInstance)
{
    return settingsInstance.Print(outputStream);
}


#endif


