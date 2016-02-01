//****************************************************************************
//
//  Copyright (c) 2002  Broadcom Corporation
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
//  Filename:       CmVendorNonVolSettings.cpp
//  Author:         David Pullen
//  Creation Date:  August 28, 2002
//
//****************************************************************************
//  Description:
//      These are the Non-Volatile settings specific to the Thomson Multimedia
//      Cable Modem.
//
//      Note that the settings are independant of the device to which they are
//      being stored, allowing the same application settings to be used on
//      multiple target platforms and devices!
//
//****************************************************************************
 
//********************** Include Files ***************************************

// My api and definitions...
#include "EventLogNonVolSettings.h"
#include "BcmString.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

// Dynamic rev = 0.5
// Permanent rev = 0.3
const uint8 kCurrentVersionMajor[2] =
{
    // Dyn, Perm
    0, 0
};

const uint8 kCurrentVersionMinor[2] =
{
    // Dyn, Perm
    1, 1
};

#if (BCM_CONSOLE_SUPPORT)
// Static command table:
BcmEventLogNonVolSettingsCommandTable BcmEventLogNonVolSettings::fCommandTable;
#endif


//********************** Local Variables *************************************

//********************** Class Method Implementations ************************

// Default Constructor.  Initializes the state of the object, setting all
// fields to good default values.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmEventLogNonVolSettings::BcmEventLogNonVolSettings(const char *pInstanceName, int Capacity) :
    BcmNonVolSettings(pInstanceName)
{
    fConstructorIsRunning = true;
    
    fCapacity = Capacity;
    
    fNextAvailableEvent = -1;
    
    // Instantiate my storage
    pfEvents = new EventLogNvRec[fCapacity];

#if (BCM_CONSOLE_SUPPORT)
    // Add myself to the command table
    fCommandTable.AddInstance(pInstanceName, this);
#endif

    // Set the magic numbers.  This is ASCII-hex for "EVLG".
    // Derived objects should override this.
    fMagicNumber = 0x45564c47;

    // Just let this do the work.
    ResetDefaults(kDynamicSection);
    ResetDefaults(kPermanentSection);

    fConstructorIsRunning = false;
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmEventLogNonVolSettings::~BcmEventLogNonVolSettings()
{
#if (BCM_CONSOLE_SUPPORT)
  // Remove myself from the command table
  fCommandTable.RemoveInstance(this);
#endif
  
  // Delete my storage.
  delete pfEvents;
}


uint16 BcmEventLogNonVolSettings::Capacity () const
{
  return fCapacity;
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
void BcmEventLogNonVolSettings::ResetDefaults(NonVolSection section)
{
/*
    // 6/30/2003: Moved this to the constructor to avoid stepping on derived
    // class magic numbers.
    
    // Set the magic numbers.  This is ASCII-hex for "EVLG".
    // Derived objects should override this.
    fMagicNumber = 0x45564c47;
*/
    // Set the current version number.
    fVersionMajor[section] = kCurrentVersionMajor[section];
    fVersionMinor[section] = kCurrentVersionMinor[section];

    /* Permanent Settings */
    
    if (section == kPermanentSection)
    {
    }
    
    /* Dynamic Settings */
    
    else
    {
        ClearAllEvents();
    }
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
bool BcmEventLogNonVolSettings::ReadFromImpl(const BcmOctetBuffer &inputBuffer, 
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
    
    /*\
     *  Dynamic Settings
    \*/

    if (section == kDynamicSection)
    {
        uint16 StoredEvents = 0, ExtraEvents = 0, Length;
        int Loop;
        
        
        // Read how many events are stored.
        if (!inputBuffer.NetworkRead(currentOffset, StoredEvents))
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                << "Failed reading number of stored events!" << endl;

            return false;
        }
        
        // If number of events stored is different from my capacity,
        // make adjustments as necessary.
        if (StoredEvents < Capacity())
        {
            gLogMessageRaw
                << "CM event log supports " << Capacity() 
                << " events, but non-vol only has " << StoredEvents
                << " events; non-vol storage will be increased to hold " 
                << Capacity() << " events." << endl;
        }
        else if (StoredEvents > Capacity())
        {
            ExtraEvents = StoredEvents - Capacity();

            gLogMessageRaw
                << "CM event log supports " << Capacity()
                << " events, and non-vol has " << StoredEvents
                << " events; non-vol storage will be decreased, and the oldest "
                << ExtraEvents << " events will be lost." << endl;

            StoredEvents = Capacity();
        }
        
        // Clear any existing events
        ClearAllEvents();
        
        // If there were any extras that we couldn't hold, then I need to
        // consume them from the buffer.  I don't care whether or not this
        // succeeds.  Read the extra events first so we preserve the newest
        // ones.
        if (ExtraEvents)
        {
            EventLogNvRec DummyEvent;
            
            for (Loop = 0; Loop < ExtraEvents; Loop++)
            {
                if (!inputBuffer.NetworkRead(currentOffset, DummyEvent.Id)
                ||  !inputBuffer.NetworkRead(currentOffset, DummyEvent.Count)
                ||  !inputBuffer.NetworkRead(currentOffset, DummyEvent.FirstTime)
                ||  !inputBuffer.NetworkRead(currentOffset, DummyEvent.LastTime))
                {
                    gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                              << "Failed reading extra event " << Loop << endl;
                    return false;
                }
                
                // Read the text length field.
                if (!inputBuffer.NetworkRead(currentOffset, Length))
                {
                    gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                              << "Failed reading extra event text length " << Loop << endl;
                    return false;
                }
                
                
                // Resize the buffer for the text.
                DummyEvent.Text.ResizeBuffer(Length);
                
                // Read the value.
                if (!inputBuffer.Read(currentOffset, DummyEvent.Text.AccessBuffer(), DummyEvent.Text.BufferSize()))
                {
                    gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                              << "Failed reading extra event text " << Loop << endl;
                    return false;
                }
            }
        }
        
        // Now read in as many events as either we can hold or as are stored.
        for (Loop = 0; Loop < StoredEvents; Loop++)
        {
            if (!inputBuffer.NetworkRead(currentOffset, pfEvents[Loop].Id)
            ||  !inputBuffer.NetworkRead(currentOffset, pfEvents[Loop].Count)
            ||  !inputBuffer.NetworkRead(currentOffset, pfEvents[Loop].FirstTime)
            ||  !inputBuffer.NetworkRead(currentOffset, pfEvents[Loop].LastTime))
            {
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                          << "Failed reading event " << Loop << endl;
                return false;
            }
            
            // Read the text length field.
            if (!inputBuffer.NetworkRead(currentOffset, Length))
            {
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                          << "Failed reading event text length " << Loop << endl;
                return false;
            }
            
            // Resize the buffer for the text.
            pfEvents[Loop].Text.ResizeBuffer(Length);
            
            // Read the value.
            if (!inputBuffer.Read(currentOffset, pfEvents[Loop].Text.AccessBuffer(), pfEvents[Loop].Text.BufferSize()))
            {
                gErrorMsg(fMessageLogSettings, "ReadFromImpl") 
                          << "Failed reading event text " << Loop << endl;
                return false;
            }
            
            // Lock in new size
            pfEvents[Loop].Text.SetBytesUsed(pfEvents[Loop].Text.BufferSize());
            
            // Note the first available slot.
            if (pfEvents[Loop].Id == 0)
            {
              if (fNextAvailableEvent == -1)
                fNextAvailableEvent = Loop;
            }
            // This would be a weird case where there's an empty event
            // stuck in the middle of the list.  If that happens, reset
            // the next available counter.
            else if (fNextAvailableEvent != -1)
              fNextAvailableEvent = -1;
        }
    }
    
    /*\
     *  Permanent Settings
    \*/
    
    else
    {
    }
     
    // Call the helper code in the base class to check for version mismatch.
    CheckForVersionUpgrade(kCurrentVersionMajor[section],
                           kCurrentVersionMinor[section], section);

    // If we got this far, then if worked!
    return true;
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
bool BcmEventLogNonVolSettings::WriteToImpl(BcmOctetBuffer &outputBuffer, 
                                        NonVolSection section) const
{
    /*\
     *  Dynamic Settings
    \*/
    
    if (section == kDynamicSection)
    {
        if (!outputBuffer.NetworkAddToEnd(Capacity()))
        {
            // Log the error.
            gErrorMsg(fMessageLogSettings, "WriteToImpl") 
                << "Buffer too small to hold all of the Dynamic settings!" << endl;

            return false;
        }
        
        for (int Loop = 0; Loop < Capacity(); Loop++)
        {
            if (!outputBuffer.NetworkAddToEnd(pfEvents[Loop].Id)
            ||  !outputBuffer.NetworkAddToEnd(pfEvents[Loop].Count)
            ||  !outputBuffer.NetworkAddToEnd(pfEvents[Loop].FirstTime)
            ||  !outputBuffer.NetworkAddToEnd(pfEvents[Loop].LastTime)
            ||  !outputBuffer.NetworkAddToEnd((uint16) pfEvents[Loop].Text.BytesUsed())
            ||  !outputBuffer.Append(pfEvents[Loop].Text))
            {
                // Log the error.
                gErrorMsg(fMessageLogSettings, "WriteToImpl") 
                    << "Buffer too small to hold all of the Dynamic settings!" << endl;
    
                return false;
            }
        }
    }
    
    /*\
     *  Permanent Settings
    \*/
    
    else
    {
    }

    // If we got this far, then it worked!
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
ostream & BcmEventLogNonVolSettings::PrintImpl(ostream & outputStream) const
{
#if !(BCM_REDUCED_IMAGE_SIZE)
    int Loop;
    
    outputStream
        << setw(25) << "\nPermanent Settings:" << " \n\n"
        
        << setw(25) << "\t(there are no permanent event log settings)\n"
        
        << setw(25) << "\nDynamic Settings:" << " \n\n"
        
        << setw(25) << "Capacity = " << Capacity() << " events\n" 
        << setw(25) << "Next available = " << fNextAvailableEvent << "\n"
        << "    Event#        Id         Count  FirstTime   LastTime\n"
        << "    ------  --------------   -----  ---------   --------" << endl;

    for (Loop = 0; Loop < Capacity(); Loop++)
    {
        outputStream
            << "    "
            << setw(4) << Loop
            << setw(16) << pfEvents[Loop].Id
            << setw(10) << pfEvents[Loop].Count
            << setw(11) << pfEvents[Loop].FirstTime
            << setw(11) << pfEvents[Loop].LastTime
            << "\n";
    }
    
    outputStream << "Event Text:" << endl;
    for (Loop = 0; Loop < Capacity(); Loop++)
        outputStream << Loop << ": " << BcmString(pfEvents[Loop].Text) << endl;
    
#endif
    
    return outputStream;
}

void BcmEventLogNonVolSettings::Event(unsigned int eventNumber, const EventLogNvRec &event)
{
    if (eventNumber < Capacity())
    {
        pfEvents[eventNumber] = event;
    }
    
    // Update next available
    if (eventNumber == fNextAvailableEvent)
    {
      if (eventNumber == (Capacity() - 1))
        fNextAvailableEvent = -1;
      else
        fNextAvailableEvent++;
    }
}

const EventLogNvRec &BcmEventLogNonVolSettings::Event(unsigned int eventNumber) const
{
    if (eventNumber >= Capacity())
    {
        eventNumber = Capacity() - 1;
    }

    return pfEvents[eventNumber];
}

void BcmEventLogNonVolSettings::EventText (unsigned int eventNumber, const BcmOctetBuffer &Text)
{
    if (eventNumber >= Capacity())
    {
        eventNumber = Capacity() - 1;
    }
    
    pfEvents[eventNumber].Text = Text;
}

const BcmOctetBuffer &BcmEventLogNonVolSettings::EventText (unsigned int eventNumber) const
{
    if (eventNumber >= Capacity())
    {
        eventNumber = Capacity() - 1;
    }
      
    return pfEvents[eventNumber].Text;
}

int BcmEventLogNonVolSettings::NextAvailableEvent () const 
{
  return fNextAvailableEvent;
}
 
void BcmEventLogNonVolSettings::ClearAllEvents(void)
{
    // Clear events
    for (int Loop = 0; Loop < Capacity(); Loop++)
    {
      pfEvents[Loop].Id = 0;
      pfEvents[Loop].Count = 0;
      pfEvents[Loop].FirstTime = 0;
      pfEvents[Loop].LastTime = 0;
      pfEvents[Loop].Text.Empty();
    }
      
    fNextAvailableEvent = 0;
}




