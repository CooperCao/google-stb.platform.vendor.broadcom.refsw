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
//  $Id: MessageQueue.cpp 1.6 2000/07/20 21:06:12Z dpullen Release $
//
//  Filename:       MessageQueue.cpp
//  Author:         David Pullen
//  Creation Date:  Dec 30, 1999
//
//****************************************************************************
//  Description:
//
//      NOTE:  I'm undecided at this point as to whether or not I should use
//             templates for the message parameters, object pointers (derived
//             from an abstract BcmMessage base class), or just a void pointer.
//             They all have their drawbacks, so I may just go with the simplest
//             one (void pointer).
//
//      This is the abstract base class (and wrapper) for operating system
//      message queues.  These are used when one thread wants to send data to
//      another thread (as opposed to events and semaphores, which simply
//      indicate that something happened).  See the BcmMessageQueue scenario
//      diagram in OSWrapper.vsd for more information on how this class should
//      be used.
//
//      This message queue object is thread-safe, meaning that it provides
//      access and contention control against being modified by multiple threads
//      at the same time.  Other types of queues (such as STL and other home-brew
//      objects/structures) may be more efficient, but must provide their own
//      contention control mechanisms (mutexes).
//
//      Note that a message queue generally operates as a FIFO queue, though it
//      does provide a method to allow messages to be sent at the front of the
//      queue.
//
//      When using a message queue one thread will act as the producer, calling
//      Send(), and another will act as the consumer, calling Receive().  There
//      can be multiple producers for a single message queue, and if controlled
//      carefully, there can even be multiple consumers.  Note, however, that
//      if there are multiple consumers, then it may only be possible for one
//      of them to use the queue in the context of an event set.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "MessageQueue.h"

// Other objects of interest.
#include "Event.h"
#include "OperatingSystem.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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
BcmMessageQueue::BcmMessageQueue(const char *pName) :
    fMessageLogSettings("BcmMessageQueue")
{
    // The derived class must create and store this.  I will delete it.
    pfEventToTrigger = NULL;

    // Call the helper method to create my object name.
    pfName = BcmOperatingSystem::GenerateName(pName, "MessageQueue");

    // Set my instance name.
    fMessageLogSettings.SetInstanceName(pfName);

    fMessageLogSettings.Register();
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Warning about possible memory leaks:
//
//      If the message queue has messages in it, then any memory associated
//      with that data will not be freed.  This is because the message queue
//      has no way to know whether the data is just a number, is a pointer
//      to a buffer, or is a pointer to an object (whose destructor needs)
//      to be called.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmMessageQueue::~BcmMessageQueue()
{
    fMessageLogSettings.Deregister();

    // Assume that the derived class has removed all of the messages from the
    // queue and done other high-level cleanup.

    // Get rid of the event.
    delete pfEventToTrigger;
    pfEventToTrigger = NULL;

    // Delete the memory associated with the name.
    delete [] pfName;
    pfName = NULL;
}


// This methods allows the calling thread to wait for a message to be queued
// without forcing it to remove the message from the queue.
//
// Note that this method returns true immediately if there are messages
// already waiting in the queue.  It returns false if the timeout expired
// before a message was queued.
//
// Parameters:
//      mode - the wait mode (forever or timeout).
//      timeoutMs - the number of milliseconds to wait for a message to be
//                  sent; only used if mode is set to timeout.
//
// Returns:
//      true if a message was queued.
//      false if the timeout expired, or some other OS-specific problem
//          occurred.
//
bool BcmMessageQueue::Wait(WaitMode mode, unsigned long timeoutMs)
{
    if (pfEventToTrigger == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Wait") << "My event is NULL!  Can't wait..." << endl;

        return false;
    }

    // Just vector to the event's method.
    return pfEventToTrigger->Wait((BcmEvent::WaitMode) mode, timeoutMs);
}
