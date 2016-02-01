//****************************************************************************
//
//  Copyright (c) 1999  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16251 Laguna Canyon Road
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id: Event.cpp 1.10 2000/03/04 15:23:19Z dpullen Release $
//
//  Filename:       Event.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 17, 1999
//
//****************************************************************************
//  Description:
//      This is the abstract base class (and wrapper) for operating system
//      events.  These are used when one thread wants to notify another thread
//      that something has happened.  See the BcmEvent scenario diagram in 
//      OSWrapper.vsd for more information on how this class should be used.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "Event.h"

#include "OperatingSystem.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Stores the name assigned to the object.
//
// Parameters:
//      pName - the text name that is to be given to the object, for
//              debugging purposes.
//
// Returns:  N/A
//
BcmEvent::BcmEvent(const char *pName) :
    fMessageLogSettings("BcmEvent")
{
    // Call the helper method to create my object name.
    pfName = BcmOperatingSystem::GenerateName(pName, "Event");

    // Set my instance name.
    fMessageLogSettings.SetInstanceName(pfName);

    fMessageLogSettings.Register();
}


// Destructor.  Frees up any memory and other resources that were allocated.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmEvent::~BcmEvent()
{
    fMessageLogSettings.Deregister();

    // Delete the memory associated with the name.
    delete pfName;
    pfName = NULL;
}


// This method clears any pending (stale) event that may already have been
// processed.  It is commonly called just before calling Wait().
//
// This is equivalent to calling Wait(kTimeout, 0) and ignoring the result.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEvent::Clear(void)
{
    gInfoMsg(fMessageLogSettings, "Clear") << "Clearing the event..." << endl;

    // True to the above comment, call Wait() and ignore the result.
    Wait(kTimeout, 0);
}



