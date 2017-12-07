/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef LNXEVENTSET_H
#define LNXEVENTSET_H

//********************** Include Files ***************************************

// My base class...
#include "EventSet.h"

class BcmLnxEvent;
class BcmLinuxTimeout;
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <map>
#else
#include <map.h>
#endif

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


class BcmLnxEventSet : public BcmEventSet
{
public:

    // Default Constructor.  Stores the name assigned to the object.
    //
    // Parameters:
    //      pName - the text name that is to be given to the object, for
    //              debugging purposes.
    //
    // Returns:  N/A
    //
    BcmLnxEventSet(const char *pName = NULL);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmLnxEventSet();

    // Blocks the calling thread until the conditions specified in the
    // parameters are satisfied.  By default, the thread will be blocked until
    // any of the events in the set is sent.  Optionally, the thread can be
    // released from waiting after a timeout, and it can wait until all of the
    // events in the set are set.
    //
    // Most threads will use the default parameters in their event handling
    // loop.
    //
    // Parameters:
    //      quantity - tells whether the thread should be released when any
    //                 event is sent, or if it should wait for all of them.
    //      mode - tells whether to wait forever, or to timeout.
    //      timeoutMS - tells how long to wait if mode is kTimeout.
    //
    // Returns:
    //      true if one or more of the events occurred.
    //      false if none of the events occurred (timeout).
    //
    bool Wait(Quantity quantity = kAny,
                      BcmEvent::WaitMode mode = BcmEvent::kForever, 
                      unsigned long timeoutMS = 0);

    // Called by LnxTimeout when my timer expires
    void Timeout (void);


private:


    // This method is called by Add() in order to allow the derived class to do
    // its OS-specific stuff.
    //
    // Parameters:
    //      pEvent - pointer to the event to add.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (too many events, etc.).
    //
    bool DerivedAdd(BcmEvent *pEvent);

    // This method is called by Remove() in order to allow the derived class to
    // do its OS-specific stuff.
    //
    // Parameters:
    //      pEvent - pointer to the event to remove.
    //
    // Returns:
    //      true if successful.
    //      false if the event isn't a member of the set, other problem.
    //
    bool DerivedRemove(BcmEvent *pEvent);

    // This method is called by RemoveAll() in order to allow the derived class
    // to do its OS-specific stuff.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if successful.
    //      false if the event set was already empty.
    //
    bool DerivedRemoveAll(void);

    // This method is called by Occurred() in order to allow the derived class
    // to do its OS-specific stuff.
    //
    // Parameters:
    //      pEvent - the event to check for being set.
    //      clearEvent - if true, resets the event; otherwise, leaves it set.
    //
    // Returns:
    //      true if the event is set.
    //      false if the event is not set.
    //
    bool DerivedOccurred(BcmEvent *pEvent, bool clearEvent);

    // Returns:
    //      true if all of my events have occurred
    //      false otherwise
    bool allOccurred(void);

    // Returns:
    //      true if any of my events have occurred
    //      false otherwise
    bool anyOccurred(void);
    
    // get event reports, wait (maybe forever) if necessary
    //
    // Parameters:
    //      quantity: 
    //            kAny: return true if any event has occurred
    //            kAll: return true if all events have occurred
    //      mode: wait forever or a certain timeout
    //      timeoutms: the amount of time to wait (if any)
    //
    // Returns:
    //      true if "quantity" satisfied
    //      false otherwise
    bool WaitAwhile (BcmEventSet::Quantity quantity, BcmEvent::WaitMode mode, 
                                     unsigned long timeoutms);

    // check if any or all events are ready
    // immediate return, no matter how many Events have occurred
    //
    // Parameters:
    //      quantity: 
    //            kAny: return true if any event has occurred
    //            kAll: return true if all events have occurred
    //
    // Returns:
    //      true if "quantity" satisfied
    //      false otherwise
    bool CheckNow (Quantity quantity);

    // Wait until the specified condition (any event or all events) has occurred
    // return true
    bool InfiniteWait (Quantity quantity);

    // Wait for the specified condition (any event or all events) 
    // Stop waiting after specified interval
    // Parameters:
    //      quantity: 
    //            kAny: return true if any event has occurred
    //            kAll: return true if all events have occurred
    //      timeoutms: the amount of time to wait (if any)
    // return: 
    //      true = condition satisfied
    //      false = timeout
    bool FiniteWait (Quantity quantity, unsigned long timeoutms);

    typedef struct 
    {
        BcmLnxEvent * evPtr;
        bool occurred;
    } tEvInfo;
    // a collection of the Events I hold and their id codes
    typedef map< BcmLnxEvent *, bool, less<BcmLnxEvent *> > MapEventsIds;
	MapEventsIds fMyEvents;

    ulong fTimeoutId;
    BcmLinuxTimeout * fMyTimeout; 

	// Flag to tell the waiting thread one or more events have been posted
	bool fEvtSetOccurred;
	// Flag to tell the waiting thread time is up
	bool fEvtSetTimedout;

	// Open access to my friend
    friend class BcmLnxEvent;
};


//********************** Inline Method Implementations ***********************

#endif


