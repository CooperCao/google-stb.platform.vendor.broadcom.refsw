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
//
//****************************************************************************
//  $Id: TimerMux.h 1.3 2003/09/22 13:32:55Z dpullen Release $
//
//  Filename:       TimerMux.h
//  Author:         Sean Dunlap
//  Creation Date:  Feb 15, 2000
//
//****************************************************************************

#ifndef TIMERMUX_H
#define TIMERMUX_H

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#include <list>
using namespace std;
#else
#include <iostream.h>
#include <list.h>
#endif

// forward declarations
class BcmTimer;
class BcmTimerMuxACT;

/** \ingroup OsWrappers
*
*     Manages multiplexing of timers into a single physical timer.  
*     It keeps a list of TimerMuxACTs (asynchronous completion tokens) 
*     corresponding to active timers.  When something wants to start 
*     it's timer, it creates a TimerMuxACT and adds calls Start().
*     To stop the timer before it is expired, call Stop().
*     When the timer expires the ACTs HandleEvent() method will be 
*     invoked.  It is up to you to specialize the ACT for your application.
*     The use of the ACT makes this a general purpose tool that can be used 
*     in various applications, even allowing the same timer to be 
*     multiplexed for various applications at the same time. 
*/
class BcmTimerMux
{
public:

    /// Constructor. 
    ///
    /// pTimer is a pointer to the timer we are multiplexing onto
    /// value is the fixed restart value of this timer
    ///
    /// \param
    ///          pTimer - pointer to the BcmTimer we are multiplexing onto
    ///
    BcmTimerMux(BcmTimer * pTimer);

    /// Destructor.  
    ///
    ~BcmTimerMux();

    /// Call this to start your timer
    ///
    /// \param
    ///          pTransaction - pointer to your BcmTimerMuxACT
    ///
    void StartTimer(BcmTimerMuxACT * pTransaction);

    /// Call this to stop your previously started timer. 
    /// It is OK to call this if it is not, in fact, started. 
    ///
    /// \param
    ///      pTransaction - pointer to the ACT for the Timer Mux
    ///
    void StopTimer(BcmTimerMuxACT * pTransaction);

    /// Called from the thread who owns the timer to indicate timer expiration.
    /// This will gracefully handle the race condition that will occur if the 
    /// timer is Stopped at the same time it is expiring. 
    ///
    void TimerExpired();

    /// Print formatted object contents to specified ostream.
    ///
    /// \param
    ///      ostrm - reference to the destination output stream.
    ///
    /// \return
    ///      reference to the destination output stream.
    ///
	ostream& Print( ostream& ostrm ) const;

private:
    /// the list of BcmTimerMuxACTs that we are multiplexing
    list< BcmTimerMuxACT * > fTimerList;

    /// pointer to the BcmTimer we are multiplexing onto
    BcmTimer * pfTimer;

    /// any timer a timer expires, is started, or stopped, this will 
    /// set the state of out multiplexed timer properly.
    void ResetTimer(unsigned long system_time);
};

inline ostream& operator<<( ostream& ostrm, const BcmTimerMux& rhs )
{
    return rhs.Print( ostrm );
}

#endif
