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
//  $Id$
//
//  Filename:       TimerMuxACT.h
//  Author:         Sean Dunlap
//  Creation Date:  Feb 16, 2000
//
//****************************************************************************
//
#ifndef TIMERMUXACT_H
#define TIMERMUXACT_H

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
using namespace std;
#else
#include <iostream.h>
#endif
#include "CompletionHandlerACT.h"

class BcmTimerMux;

/** \ingroup OsWrappers
*
*   Derived class BcmTimerMuxACT specializes the BcmCompletionHandlerACT base
*   class for use with BcmTimerMux transactions.  Refer to POSA2 book, page 268.
*/
class BcmTimerMuxACT : public BcmCompletionHandlerACT
{
public:

    /// Constructor. 
    ///
    BcmTimerMuxACT(unsigned long timerDuration = 0);

    /// Copy Constructor.  
    ///
    BcmTimerMuxACT( const BcmTimerMuxACT& rhs );

    /// Destructor.  
    ///
    virtual ~BcmTimerMuxACT();
            
    /// Assignment operator.
    ///
    /// \param
    ///     rhs - source object, a.k.a. right hand side.
    ///
    /// \return
    ///     reference to this object
    ///
    BcmTimerMuxACT& operator=( const BcmTimerMuxACT& rhs );

    /// Equality operator.
    ///
    /// \param
    ///     rhs - comparison object, a.k.a. right hand side.
    ///
    /// \return
    // Returns:
    ///     bool - true if objects are equal, else false.
    ///
	bool operator==( const BcmTimerMuxACT& rhs ) const;

    /// Print formatted object contents to specified ostream.
    ///
    /// \param
    ///      ostrm - reference to the destination output stream.
    ///
    /// \return
    ///      reference to the destination output stream.
    ///
	ostream& Print( ostream& ostrm ) const;

    /// DerivedPrint - This function is implemented by classes derived
    ///      from BcmMacManagementMessage to print formatted derived 
    ///      object contents to the specified ostream.  It is called 
    ///      by BcmMacManagementMessage::Print() (a.k.a. the base class
    ///      Print() function).
    ///
    /// \param
    ///      ostrm - reference to the destination output stream.
    ///
    /// \return
    ///      reference to the destination output stream.
    ///
    virtual ostream& DerivedPrint( ostream& ostrm ) const;

    /// HandleEvent - in Asynchronous Completion Token pattern, this function
    ///      can be used by the Initiator object (or it's BcmCallbackHandler
    ///      helper object to dispatch a specific completion handler that
    ///      processes the response from an asynchronous operation.
    ///
    /// \param
    ///      event_code - integer event code which identifies the asynchronous
    ///          event type which occurred.  

    virtual void HandleEvent(const BcmCompletionEvent &event_code) = 0;

    /// accessors for client to the Timer Mux
    inline unsigned long TimerDuration() const { return fTimerDuration; }
    inline void TimerDuration(unsigned long time) 
    { 
        fTimerDuration = time;
    }

private:

    /// the time that the timer should expire
    unsigned int fExpirationTime;

    /// the time that the timer was started
    unsigned int fStartTime;

    /// the total duration of the timer
    unsigned int fTimerDuration;


protected:
    /// only accessible by friend class BcmTimerMux
    inline unsigned long StartTime() const { return fStartTime; }
    inline void StartTime(unsigned long time) 
    { 
        fStartTime = time;
    }

    inline unsigned long ExpirationTime() const { return fExpirationTime; }
    inline void ExpirationTime(unsigned long time) 
    { 
        fExpirationTime = time;
    }

    /// we want to give the TimerMux access to our StartTime and EndEnd time,
    /// but we do not want the end user to have access. 
    friend class BcmTimerMux;

};

inline ostream& operator<<( ostream& ostrm, const BcmTimerMuxACT& rhs )
{
    return rhs.Print( ostrm );
}

#endif
