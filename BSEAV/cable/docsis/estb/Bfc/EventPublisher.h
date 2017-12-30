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
//  Filename:       EventPublisher.h
//  Author:         Cliff Danielson
//  Creation Date:  Apr 13, 2001
//
//****************************************************************************
//  Description:
//      Class BcmEventPublisher defines an interface that can be used to publish
//      asynchronous event notifications to multiple subscribers.  Refer to 
//      POSA2 book, page 268. 
//
//****************************************************************************
//
// EventPublisher.h
//
#ifndef EVENTPUBLISHER_H
#define EVENTPUBLISHER_H

#include "typedefs.h"

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <map>
#include <utility>
#else
#include <multimap.h>
#include <pair.h>
#endif

class BcmCompletionHandlerACT;

// UNFINISHED - the SubscriberPair typedef needs to be phased out in favor
// of public typedef BcmEventPublisher::KeySubscriberPair
//
typedef pair< const unsigned int, BcmCompletionHandlerACT* > SubscriberPair;

class BcmEventPublisher
{
public:
    // Default constructor.
    //
    BcmEventPublisher();

    // Copy constructor.
    //
    BcmEventPublisher( const BcmEventPublisher& rhs );

    // Destructor.  
    //
    virtual ~BcmEventPublisher();

    // Assignment operator.
    //
    BcmEventPublisher& operator=( const BcmEventPublisher& rhs );

    // Less than operator.
    //
    bool operator<( const BcmEventPublisher& rhs ) const;

    // Equality operator.
    //
    bool operator==( const BcmEventPublisher& rhs ) const;


    // PublishEvent() - Notify interested subscribers that the specified event
    //      has occurrd.
    //
    // Parameters:
    //      event_code - integer event code which identifies the asynchronous
    //          event type which occurred.  
    //
    virtual void PublishEvent( const unsigned int event_code );

    // Subscribe() - Subscribe for notification when the specified event occurs.
    //
    // Parameters:
    //      SubscriberPair
    //      event_code - event code for event of interest.
    //
    //      pACT - pointer to object derived from BcmCompletionHandlerACT. 
    //          BcmCompletionHandlerACT implements the callback interface.  
    //          The derived class implements HandleEvent() in a manner which
    //          dispatches the event to the client object.
    //
    void Subscribe( const unsigned int event_code, BcmCompletionHandlerACT* pACT );
    
    // UnSubscribe() - End subscription for event notification.
    //
    // Parameters:
    //      event_code - event code for event of interest.
    //
    //      pACT - pointer to object derived from BcmCompletionHandlerACT. 
    //          BcmCompletionHandlerACT implements the callback interface.  
    //          The derived class implements HandleEvent() in a manner which
    //          dispatches the event to the client object.
    //
    void UnSubscribe( const unsigned int event_code, BcmCompletionHandlerACT* pACT );

    // NumSubscribers() - Returns the number of subscribers for the specified 
    //      event code.
    //
    // Parameters:
    //      event_code - event code for event of interest.
    //
    // Returns:
    //      unsigned int - number of subscribers for the specified event code.
    //
    unsigned int NumSubscribers( const unsigned int event_code ) const
    { 
        return ( fSubscriberMmap.count( event_code ) ); 
    }


    typedef pair< const unsigned int, BcmCompletionHandlerACT* > KeySubscriberPair;


protected:
    // DerivedPublishEvent() - Called by PublishEvent for each target
    //      < event code, BcmCompletionHandlerACT* > pair found
    //      in order that a derived class may apply additional filtering 
    //      criteria prior to dispatching the event to the ACT.  
    //      Default implementation unconditionslly dispatches the event 
    //      to the specified ACT.
    //
    // Parameters:
    //      event_code - integer event code which identifies the asynchronous
    //          event type which occurred.  
    //
    //      pACT - pointer to object derived from BcmCompletionHandlerACT. 
    //
    virtual void DerivedPublishEvent( const unsigned int event_code, 
        BcmCompletionHandlerACT* pACT );

    typedef multimap< unsigned int, BcmCompletionHandlerACT*, std::less<unsigned int> > SubscriberMmap;
    typedef SubscriberMmap::iterator SubscriberMmapIt;
    
    SubscriberMmap fSubscriberMmap;
};


#endif
