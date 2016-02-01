//****************************************************************************
//
//  Copyright (c) 2003  Broadcom Corporation
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
//  Filename:       EventLogNonVolSettings.h
//  Author:         Kevin O'Neal
//  Creation Date:  February 13, 2003
//
//****************************************************************************
//  Description:
//      These are the Non-Volatile settings specific to the DOCSIS event log
//
//      Note that the settings are independant of the device to which they are
//      being stored, allowing the same application settings to be used on
//      multiple target platforms and devices!
//
//****************************************************************************

#ifndef EVENTLOGNONVOLSETTINGS_H
#define EVENTLOGNONVOLSETTINGS_H

//********************** Include Files ***************************************

//********************** Include Files ***************************************

// My base class.
#include "NonVolSettings.h"

#if (BCM_CONSOLE_SUPPORT)
// My command table
#include "EventLogNonVolSettingsCommandTable.h"
#endif

#include "OctetBuffer.h"

//********************** Global Types ****************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************

typedef struct _EventLogNvRec
{
  uint32 Id;
  uint32 Count;
  time_t FirstTime;
  time_t LastTime; 
  BcmOctetBuffer Text;
} EventLogNvRec;

class BcmEventLogNonVolSettings : public BcmNonVolSettings
{
public:

    // Default Constructor.  Initializes the state of the object, setting all
    // fields to good default values.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmEventLogNonVolSettings(const char *pInstanceName, int Capacity);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmEventLogNonVolSettings();

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
    virtual void ResetDefaults(NonVolSection section);
    
    // The maximum number of events that can be stored in this object.
    uint16 Capacity (void) const;

public:

    void Event(unsigned int eventNumber, const EventLogNvRec &event);
    const EventLogNvRec &Event(unsigned int eventNumber) const;
    
    void EventText (unsigned int eventNumber, const BcmOctetBuffer &Text);
    const BcmOctetBuffer &EventText (unsigned int eventNumber) const;
    
    int NextAvailableEvent () const;
    
    void ClearAllEvents (void);

protected:

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
    virtual bool WriteToImpl(BcmOctetBuffer &outputBuffer, NonVolSection section) const;

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
    virtual bool ReadFromImpl(const BcmOctetBuffer &inputBuffer, 
                              unsigned int &currentOffset, NonVolSection section);

    // Allows the derived class to do formatted output of its settings to the
    // specified ostream, after the base class does its stuff.
    //
    // Parameters:
    //      outputStream - the ostream to be used.
    //
    // Returns:
    //      The reference to the ostream that was passed in.
    //
    virtual ostream & PrintImpl(ostream & outputStream) const;

protected:

    // Used to keep Reset() from printing warnings when it is called from the
    // constructor.
    bool fConstructorIsRunning;

#if (BCM_CONSOLE_SUPPORT)
    // My command table
public:
    static BcmEventLogNonVolSettingsCommandTable fCommandTable;
#endif

private:  // Dynamic nonvol settings.
    
    // How many events are supported by this instance
    uint16 fCapacity;
    
    // Next event available
    int fNextAvailableEvent;
    
    // Array of event rec's
    EventLogNvRec *pfEvents;

private:  // Permanent nonvol settings.

private:
    
    // Copy Constructor.  Not supported.
    BcmEventLogNonVolSettings(const BcmEventLogNonVolSettings &otherInstance);

    // Assignment operator.  Not supported.
    BcmEventLogNonVolSettings & operator = (const BcmEventLogNonVolSettings &otherInstance);


protected:
};

//********************** Inline Method Implementations ***********************


// This allows you to do the following:
//
//    cout << settingsInstance << endl;
//
inline ostream & operator << (ostream &outputStream, const BcmEventLogNonVolSettings &settingsInstance)
{
    return settingsInstance.Print(outputStream);
}


#endif

