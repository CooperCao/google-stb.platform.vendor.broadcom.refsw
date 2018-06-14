/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
//****************************************************************************
//  $Id: Timer.cpp 1.8 2000/07/20 21:06:15Z dpullen Release $
//
//  Filename:       Timer.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 18, 1999
//
//****************************************************************************
//  Description:
//      This is the abstract base class for operating system timers.  A timer
//      causes the OS to send an event (BcmEvent) to the thread after the
//      specified timeout interval has expired; thus BcmTimer has-a BcmEvent.
//
//      NOTE:  For some operating systems, a timer must be started by the thread
//             that expects to be notified.  Thus, when a thread creates a
//             timer, it must ensure that the timer will not be used by any
//             other thread.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "Timer.h"

// Other objects of interest.
#include "Event.h"
#include "OperatingSystem.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default Constructor.  Sets everything to a quiescent state; note that
// the event to trigger is not created here - it must be created by the
// derived class.
//
// Parameters:
//      pName - the text name that is to be given to the object, for
//              debugging purposes.
//
// Returns:  N/A
//
BcmTimer::BcmTimer(const char *pName) :
    fMessageLogSettings("BcmTimer")
{
    // Call the helper method to create my object name.
    pfName = BcmOperatingSystem::GenerateName(pName, "Timer");
    
    // Set my instance name.
    fMessageLogSettings.SetInstanceName(pfName);

    fMessageLogSettings.Register();

    // Defer creation of the event to the derived class - they know what type
    // of event to create (and how to create it).
    pfEventToTrigger = NULL;

    // Set the rest of these to good default values.
    fHasBeenStarted = false;
    fIsRunning = false;
    fLastTimeoutMS = 0;
    fLastTimerMode = kOnce;
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.  The most-derived class must call Stop() in order to ensure that
// the timer isn't running.
//
// pfEventToTrigger is deleted here.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmTimer::~BcmTimer()
{
    fMessageLogSettings.Deregister();

    // Assume that the timer is stopped by now.  Delete the event object.
    delete pfEventToTrigger;
    pfEventToTrigger = NULL;

    // Delete the memory associated with the name.
    delete [] pfName;
    pfName = NULL;
}


// Stops the timer and restarts it with the timeout value and mode that
// were sent in on the last call to Start().  If Start() has not been
// called, then the timer is not restarted.
//
// This is useful because it eliminates extra calls to the object, and you
// don't have to know what the previous values were in order to restart it.
//
// Parameters:  None.
//
// Returns:
//      true if the timer was restarted successfully.
//      false if the timer was not restarted (Start() has not bee called
//          previously, or there was a problem stopping and/or starting it).
//
bool BcmTimer::Restart(void)
{
    // See if Start() has been called; if not, then we can't do this.
    if (!fHasBeenStarted)
    {
        gInfoMsg(fMessageLogSettings, "Restart") << "Timer not started.  Can't restart..." << endl;

        return false;
    }

    // Start the timer with the previous values.  I don't need to stop it 
    // before calling Start (the derived class will do so if necessary).
    return Start(fLastTimeoutMS, fLastTimerMode);
}


// Causes the calling thread to be blocked until the timer expires.  If the
// timer was not started, then it returns false immediately.
//
// Parameters:  None.
//
// Returns:
//      true if the timer expired.
//      false if the timer was not started.
//
bool BcmTimer::Wait(void)
{
    if (!fIsRunning)
    {
        gInfoMsg(fMessageLogSettings, "Wait") << "Timer not running.  Can't wait for it..." << endl;

        return false;
    }

    // Wait forever for the event to be triggered.  We don't need to have the
    // event time out on the wait, because the timer will generate the timeout.
    return pfEventToTrigger->Wait();
}
