//****************************************************************************
//
// Copyright (c) 2010 Broadcom Corporation
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
//  $Id: TimerMux.cpp 1.5 2005/07/06 22:06:00Z sdunlap Release $
//
//  Filename:       TimerMux.cpp
//  Author:         Sean Dunlap
//  Creation Date:  Feb 16, 2000
//
//****************************************************************************
//  Description:
//      Manages multiplexing of timers into a single physical timer.  
//      It keeps a list of TimerMuxACTs (asynchronous completion tokens) 
//      corresponding to active timers.  When something wants to start 
//      it's timer, it creates a TimerMuxACT and adds calls Start().
//      To stop the timer before it is expired, call Stop().
//      When the timer expires the ACTs HandleEvent() method will be 
//      invoked.  It is up to you to specialize the ACT for your application.
//      The use of the ACT makes this a general purpose tool that can be used 
//      in various applications, even allowing the same timer to be 
//      multiplexed for various applications at the same time. 
//
//****************************************************************************

#include "TimerMux.h"
#include "TimerMuxACT.h"
#include "SystemTime.h"
#include "Timer.h"
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <algorithm>
#else
#include <algo.h>
#endif

// Constructor. 
//
// pTimer is a pointer to the timer we are multiplexing onto
// value is the fixed restart value of this timer
//
// Parameters:  
//          pTimer - pointer to the BcmTimer we are multiplexing onto
//
BcmTimerMux::BcmTimerMux(BcmTimer * pTimer) :
    pfTimer(pTimer)
{
}

// Destructor.  
//
BcmTimerMux::~BcmTimerMux()
{
}

// Call this to start your timer
//
// Parameters:  
//          pACT - pointer to your BcmTimerMuxACT
//
// Returns:
//
void BcmTimerMux::StartTimer(BcmTimerMuxACT * pACT)
{
    list< BcmTimerMuxACT * >::iterator it;

    // get system time here and use the same value in the ACT 
    // and the ResetTime function
    unsigned int system_time = SystemTimeMS();

    // set up the start time and the expiration time
    pACT->StartTime(system_time);
    pACT->ExpirationTime(system_time + pACT->TimerDuration());

    it = find(fTimerList.begin(), fTimerList.end(), pACT);

    // if it is NOT on the list, add it to the list
    if(it == fTimerList.end())
    {
        // add the timer ACT to the end of the list
        fTimerList.push_back( pACT );
    }

    // recalculate the timer because we may have just added a 
    // timer that will expire before the current timer, or, it 
    // may be the first on the list, or, it's period may have
    // changed (nothing preventing this)
    ResetTimer(system_time);

}

// Call this to stop your previously started timer. 
// It is OK to call this if it is not, in fact, started. 
//
// Parameters:  
//      BcmTimerMuxACT * pACT - pointer to the ACT for the Timer Mux
//
// Returns:
//
void BcmTimerMux::StopTimer(BcmTimerMuxACT * pACT)
{
    list< BcmTimerMuxACT * >::iterator it;
    it = find(fTimerList.begin(), fTimerList.end(), pACT);

    // remove it from the list only if we found it on the list
    if(it != fTimerList.end())
    { 
        // remove the timer ACT from the list
        fTimerList.erase( it );

        // recalculate the timer because we may have removed the active timer
        // in the list
        ResetTimer(SystemTimeMS());
    }
}

// Called from the thread who owns the timer to indicate timer expiration.
// This will gracefully handle the race condition that will occur if the 
// timer is Stopped at the same time it is expiring. 
//
// Parameters:  None.
//
// Returns:
//
void BcmTimerMux::TimerExpired()
{
    // iterate the list and expire the timers for all timers that were set
    // to expire on or before now
    list< BcmTimerMuxACT * >::iterator it = fTimerList.begin();
    list< BcmTimerMuxACT * >::iterator list_end = fTimerList.end();

    // read this once up front and use the same time 
    // in the expiration iteration and the ResetTimer iteration
    unsigned long current_time = SystemTimeMS();
    
    // expiration time of the current timer that the iterator is pointing to
    unsigned long expiration_time;

    // the time that the timer was started
    unsigned long start_time;

    // the list of BcmTimerMuxACTs that we are multiplexing
    list< BcmTimerMuxACT * > fExpiredTimerList;


    // iterate through the whole list finding expired timers.
    // there are 6 cases here to handle because of timer 
    // waparound every 49.71 days
    //
    // S = start time  E = expiration time  C = current time
    //
    //                       Expired?    
    // Case 1: S <  E <= C    yes      this is the normal case
    // Case 2: C <  S <  E    yes      obscure wraparound case
    // Case 3: E <= C <  S    yes      somewhat less obscure wraparound case 
    // Case 4: S <= C <  E    no
    // Case 5: E <  S <= C    no
    // Case 6: C <  E <  S    no
    while( it != list_end )
    {
        // derefernce the iterator once here
        BcmTimerMuxACT * pACT = *it;

        start_time = pACT->StartTime();
        expiration_time = pACT->ExpirationTime();
    
        // if the timer expired on or before now
        if ( ((start_time < expiration_time) && (expiration_time <= current_time )) ||
             ((current_time < start_time) && (start_time < expiration_time)) ||
             ((expiration_time <= current_time) && (current_time < start_time)) 
           )
        {
            // add this timer to the local expired list
            fExpiredTimerList.push_back(*it);

            // remove "it" from the list first.  
            // erase invalidates the iterator unless post-increment.
            fTimerList.erase(it++);
        }
        else
        {
            // go to the next ACT
            it++;
        }
    }

    // now invoke the expiration handler for each timer that expired
    it = fExpiredTimerList.begin();
    list_end = fExpiredTimerList.end();

    while( it != list_end )
    {
        // indicate to the ACT that the timer has expired.  this call may 
        // result in call to StartTimer or StopTimer. 
        (*it)->HandleEvent(0);
        it++;
    }

    // now that at least 1 timer has expired, we must recalculte when the 
    // shared timer should expire and re-set it
    ResetTimer(current_time);
}


// Calculates what the smallest timer is on the list and restarts the 
// multiplexed timer with the appropriate value.  Or it will stop the 
// timer if the list is NULL.  
//
// Parameters:  
//          current_time - current system time.
//
// Returns:
//
void BcmTimerMux::ResetTimer(unsigned long current_time)
{
    // iterate the list and determine the next timer to expire
    list< BcmTimerMuxACT * >::iterator it = fTimerList.begin();
    list< BcmTimerMuxACT * >::iterator list_end = fTimerList.end();

    unsigned long next_expiration_time = 0;
    unsigned long it_expiration_time;

    while( it != list_end )
    {
        it_expiration_time = (*it)->ExpirationTime();

        // if this is the first one we have found not expired
        if(next_expiration_time == 0)
        {
            next_expiration_time = it_expiration_time;
        }

        // now there are again 6 cases to consider. 
        //
        // next_expiration_time = N  it_expiration_time = I  current_time = C
        // 
        //                       I < N ?    
        // Case 1:  N < C < I     yes           
        // Case 2:  I < N < C     yes
        // Case 3:  C < I < N     yes
        // Case 4:  I < C < N     no 
        // Case 5:  N < I < C     no
        // Case 6:  C < N < I     no
        //
        else if ( ((next_expiration_time < current_time) && (current_time < it_expiration_time)) ||
                  ((it_expiration_time < next_expiration_time) && (next_expiration_time < current_time)) ||
                  ((current_time < it_expiration_time) && (it_expiration_time < next_expiration_time) )
                )
        {
                next_expiration_time = it_expiration_time;
        }

        it++;

    }

    // if there are any timers left, restart our timer with the time of the next to expire
    if(next_expiration_time)
    {

        // make sure the value of the timer we set is at least 1 ms.  A value of zero will not fire. 
        unsigned int timerSetValue = 1;

        if( next_expiration_time > current_time )
        {
            timerSetValue = next_expiration_time - current_time;
        }

        // restart the timer based on the smallest one
        pfTimer->Start( timerSetValue );
    }
    else
    {
        // else stop the timer, because it may still be running
        pfTimer->Stop();
    }
}

// Print formatted object contents to specified ostream.
//
// Parameters:
//      ostream& ostrm - reference to the destination output stream.
//
// Returns:
//      reference to the destination output stream.
//
ostream& BcmTimerMux::Print( ostream& ostrm ) const
{    
    ostrm << "\nBcmTimerMux timer list:\n";

    list< BcmTimerMuxACT * >::const_iterator it = fTimerList.begin();

    // iterate the list and print the timers
    while( it != fTimerList.end() )
    {
      //  ostrm << *it++;
    }

    return ostrm;
}
