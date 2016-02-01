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
//  $Id: MsgPublisherOneToOne.h 1.1 2005/04/01 19:02:31Z cliffd Release $
//
//  Filename:       MsgPublisherOneToOne.h
//  Author:         Cliff Danielson
//  Creation Date:  Mar 22, 2005
//
//****************************************************************************
//  Description:
//      Class BcmMsgPublisherOneToOne supports publication of asynchronous 
//      messages of a particular type to a particular subscriber.  Message 
//      refers to status event plus parameter data.  Interpretation of 
//      parameter data depends on the particular message type.  Deletion 
//      of dynamically allocated message parameter is the responsibility 
//      of the subscriber.  The "OneToOne" portion of the name refers 
//      to the one to one association of published message code to subscriber.  
//
//****************************************************************************
//
// MsgPublisherOneToOne.h
//
#ifndef MSGPUBLISHERONETOONE_H
#define MSGPUBLISHERONETOONE_H

#include "typedefs.h"
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <utility>
#else
#include <pair.h>
#endif
#include "PublisherOneToOne.h"

class BcmMsgACT;
class BcmOctetBuffer;

class BcmMsgPublisherOneToOne
{
public:
    // Default constructor.
    //
    BcmMsgPublisherOneToOne();

    // Copy constructor.
    //
    BcmMsgPublisherOneToOne( const BcmMsgPublisherOneToOne& rhs );

    // Destructor.  
    //
    virtual ~BcmMsgPublisherOneToOne();

    // Assignment operator.
    //
    BcmMsgPublisherOneToOne& operator=( const BcmMsgPublisherOneToOne& rhs );

    // Less than operator.
    //
    bool operator<( const BcmMsgPublisherOneToOne& rhs ) const;

    // Equality operator.
    //
    bool operator==( const BcmMsgPublisherOneToOne& rhs ) const;

    // PublishMsg() - Published specified message to interested subscriber.
    //
    // Parameters:
    //      msg_code - message/event code of interest.
    //
    //      pmsg_param - void* message parameter.  Interpretation of value
    //          depends on msg_code value.
    //
    // Returns:
    //      bool - true is returned if interested subscriber exists.
    //          otherwise returns false.
    //          
    bool PublishMsg( const unsigned int msg_code, void* pmsg_param );

    // Subscribe() - Subscribe for CM DOCSIS status message notification.
    //
    // Parameters:
    //      msg_code - message/event code of interest.
    //
    //      pMsgACT - pointer to object derived from BcmMsgACT.  
    //          BcmMsgACT adds a void* data member to class BcmCompletionHandlerACT.
    //          The derived class implements HandleEvent() in a manner 
    //          which dispatches the message to the client object.
    //
    void Subscribe( const unsigned int msg_code, BcmMsgACT* pMsgACT );
    
    // UnSubscribe() - End subscription for CM DOCSIS status message 
    //      notification.
    //
    // Parameters:
    //      msg_code - message type identifier.
    //
    void UnSubscribe( const unsigned int msg_code );

    typedef pair< const unsigned int, BcmMsgACT* > KeySubscriberPair;

protected:

    typedef BcmPublisherOneToOne< const unsigned int, BcmMsgACT* > SubscriberMap;

    SubscriberMap fSubscriberMap;
};



#endif
