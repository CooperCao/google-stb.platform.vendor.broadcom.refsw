//**************************************************************************
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
//
//
//**************************************************************************
//  $Id: PublisherOneToOne.h 1.1 2005/04/01 19:02:30Z cliffd Release $
//
//  Filename:       PublisherOneToOne.h
//  Author:         Cliff Danielson
//  Creation Date:  Jan 10, 2005
//
//****************************************************************************
//  Description:
//      Class BcmPublisherOneToOne defines an interface that can be used 
//      to publish asynchronous notifications to subscribers.  The "OneToOne"
//      portion of the name refers to the association of one published key 
//      to one subscriber.  Refer to POSA2 book, page 268. 
//
//      BcmPublisherOneToOne is implemented as an container adapter for the 
//      standard map container.
//****************************************************************************
//
// PublisherOneToOne.h
//
#ifndef PUBLISHERONETOONE_H
#define PUBLISHERONETOONE_H

#include "typedefs.h"

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <map>
#include <utility>
#include <algorithm>
#else
#include <map.h>
#include <pair.h>
#include <algo.h>
#endif
#include "MessageLog.h"


template < class TPublishedKey, class TSubscriber >
class BcmPublisherOneToOne
{
public:
    typedef pair< TPublishedKey, TSubscriber > KeySubscriberPair;
    typedef map< TPublishedKey, TSubscriber, less< TPublishedKey > > SubscriberMap;

#if (__GNUC__ >= 3)
    typedef typename SubscriberMap::iterator SubscriberMapIt;
    typedef typename SubscriberMap::const_iterator SubscriberMapConstIt;
#else
    typedef SubscriberMap::iterator SubscriberMapIt;
    typedef SubscriberMap::const_iterator SubscriberMapConstIt;
#endif

    typedef pair< SubscriberMapIt, SubscriberMapIt > SubscriberMapItRange;
    typedef pair< SubscriberMapConstIt, SubscriberMapConstIt > SubscriberMapConstItRange;
    
    // Default constructor.
    //
    BcmPublisherOneToOne()
      :
        fSubscriberMap()
    {}


    // Copy constructor.
    //
    BcmPublisherOneToOne( const BcmPublisherOneToOne<TPublishedKey, TSubscriber>& rhs ) 
      :
        fSubscriberMap( rhs.fSubscriberMap )
    {}

    // Destructor.  
    //
    virtual ~BcmPublisherOneToOne();


    // Assignment operator.
    //
    BcmPublisherOneToOne<TPublishedKey, TSubscriber>& operator=( const BcmPublisherOneToOne<TPublishedKey, TSubscriber>& rhs )
	{
	    if( this == &rhs )
        {
            // special case: assignment to self
            return *this;
        }

        fSubscriberMap = rhs.fSubscriberMap;
        return *this;
    }

    // Less than operator.
    //
    bool operator<( const BcmPublisherOneToOne<TPublishedKey, TSubscriber>& rhs ) const
    {
        return (fSubscriberMap < rhs.fSubscriberMap);
    }

    // Equality operator.
    //
    bool operator==( const BcmPublisherOneToOne<TPublishedKey, TSubscriber>& rhs ) const
    {
        return (fSubscriberMap == rhs.fSubscriberMap);
    }

    // Subscribe() - Subscribe for notification when the specified key
    //      value is published.
    //
    // Parameters:
    //      publishedKey - identifier for published type.
    //
    //      subscriber - pointer or reference to subscriber object.
    //
    void Subscribe( TPublishedKey const& publishedKey, TSubscriber const& subscriber )
    {
        CallTrace("BcmPublisherOneToOne<>", "Subscribe");

        fSubscriberMap.insert( KeySubscriberPair( publishedKey, subscriber ) );
    }

    // UnSubscribe() - End subscription for key value publication.
    //
    // Parameters:
    //      publishedKey - identifier for published type.
    //
    void UnSubscribe( TPublishedKey const& publishedKey )
    {
        CallTrace("BcmPublisherOneToOne<>", "UnSubscribe");

        fSubscriberMap.erase( publishedKey );
    }

    // Publish() - Publish the specified key value to interested subscriber.
    // 
    // Template parameters:
    //      class Functor - function object which publishes the
    //          publishedKey to the interested subscriber.    
    //
    // Function parameters:
    //      publishedKey - identifier for published type.
    //
    // Returns:
    //      bool - true is returned if interested subscriber is found.
    //          otherwise false is returned.
    //
    template < class Functor >
    bool Publish( TPublishedKey const& publishedKey, Functor functor )
    {
        CallTrace("BcmPublisherOneToOne<>", "Publish");
        bool ret_val = false;
        if( !fSubscriberMap.empty() )
        {
            SubscriberMapIt it = fSubscriberMap.find( publishedKey );
            if( it != fSubscriberMap.end() )
            {
                // found subscriber for this publishedKey.
                // --> execute the specified function object for  
                // matching subscriber.
                ret_val = functor( *it );
            }
        }
        return ret_val;
    }

protected:
    SubscriberMap fSubscriberMap;
};

template < class TPublishedKey, class TSubscriber >
BcmPublisherOneToOne< TPublishedKey, TSubscriber >::~BcmPublisherOneToOne()
{}

#endif
