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
//  $Id: MsgPublisherOneToOne.cpp 1.1 2005/04/01 19:02:31Z cliffd Release $
//
//  Filename:       MsgPublisherOneToOne.cpp
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
// MsgPublisherOneToOne.cpp
//
#include "MsgPublisherOneToOne.h"
#include "MsgACT.h"
#include "OctetBuffer.h"
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <functional>
#else
#include <function.h>
#endif

#include "MessageLog.h"

BcmMsgPublisherOneToOne::BcmMsgPublisherOneToOne()
  :
    fSubscriberMap()
{
    CallTrace("BcmMsgPublisherOneToOne", "BcmMsgPublisherOneToOne");
}


BcmMsgPublisherOneToOne::BcmMsgPublisherOneToOne( const BcmMsgPublisherOneToOne& rhs )
  :
    fSubscriberMap( rhs.fSubscriberMap )
{
    CallTrace("BcmMsgPublisherOneToOne", "BcmMsgPublisherOneToOne copy constr");
    
}


BcmMsgPublisherOneToOne::~BcmMsgPublisherOneToOne()
{
    CallTrace("BcmMsgPublisherOneToOne", "~BcmMsgPublisherOneToOne");
}


BcmMsgPublisherOneToOne& BcmMsgPublisherOneToOne::operator=( const BcmMsgPublisherOneToOne& rhs )
{
    CallTrace("BcmMsgPublisherOneToOne", "operator=");

    if( this == &rhs )
    {
        // special case: assignment to self
        return *this;
    }

    fSubscriberMap = rhs.fSubscriberMap;
    return *this;
}


bool BcmMsgPublisherOneToOne::operator<( const BcmMsgPublisherOneToOne& rhs ) const
{
    CallTrace("BcmMsgPublisherOneToOne", "operator<");
    
    return (fSubscriberMap < rhs.fSubscriberMap);
}

bool BcmMsgPublisherOneToOne::operator==( const BcmMsgPublisherOneToOne& rhs ) const
{
    CallTrace("BcmMsgPublisherOneToOne", "operator==");
    
    return (fSubscriberMap == rhs.fSubscriberMap);
}


template< class KeySubscriberPair >
class BcmFuncyMsgPublisherOneToOne: public unary_function< KeySubscriberPair, bool >
{
public:
    BcmFuncyMsgPublisherOneToOne( void* pMsgParam = NULL )
      :
        pfMsgParam( pMsgParam )
    {
        CallTrace("BcmFuncyMsgPublisherOneToOne<>", "constructor");
    }

// UNFINISHED - most books use compiler generated copy constructor, operator=
// and destructor.  I am implementing for CallTrace() got here debugging!!!!
    BcmFuncyMsgPublisherOneToOne( const BcmFuncyMsgPublisherOneToOne& rhs )
      :
        pfMsgParam( rhs.pfMsgParam )
    {
        CallTrace("BcmFuncyMsgPublisherOneToOne<>", "copy constructor");
    }

    BcmFuncyMsgPublisherOneToOne& operator=( const BcmFuncyMsgPublisherOneToOne& rhs )
    {
        CallTrace("BcmFuncyMsgPublisherOneToOne<>", "operator=");
	    if( this == &rhs )
        {
            // special case: assignment to self
            return *this;
        }

        pfMsgParam = rhs.pfMsgParam;
        return *this;
    }

    ~BcmFuncyMsgPublisherOneToOne()
    {
        CallTrace("BcmFuncyMsgPublisherOneToOne<>", "destructor");
    }
// UNFINISHED - most books use compiler generated copy constructor, operator=
// and destructor.  I am implementing for CallTrace() got here debugging!!!!

    
    bool operator()( const KeySubscriberPair& keySubscriberPair )
    {
        CallTrace("BcmFuncyMsgPublisherOneToOne<>", "operator()");
        bool ret_val = false;
        if( keySubscriberPair.second != NULL )
        {
            // load the message buffer param into the ACT...
            BcmMsgACT* pMsgACT = keySubscriberPair.second;
            pMsgACT->MsgParam( MsgParam() );

            // delegate...
            pMsgACT->HandleEvent( keySubscriberPair.first );
            ret_val = true;
        }
        return ret_val;
    }

    void MsgParam( void* pMsgParam ) { pfMsgParam = pMsgParam; }
    void* MsgParam() { return pfMsgParam; }

protected:
    // message parameter.  interpretation depends on published message type.
    void* pfMsgParam;
};

bool BcmMsgPublisherOneToOne::PublishMsg( const unsigned int msg_code, void* pmsg_param )
{
    CallTrace("BcmMsgPublisherOneToOne", "PublishMsg");

/*
    BcmFuncyMsgPublisherOneToOne< KeySubscriberPair > FuncyMsgDelegator( pbuf );
    
    return fSubscriberMap.Publish( msg_code, FuncyMsgDelegator );
*/
    return fSubscriberMap.Publish( msg_code, 
        BcmFuncyMsgPublisherOneToOne< KeySubscriberPair >( pmsg_param ) );
}

void BcmMsgPublisherOneToOne::Subscribe( const unsigned int msg_code, BcmMsgACT* pMsgACT )
{
    CallTrace("BcmMsgPublisherOneToOne", "Subscribe");
    
    fSubscriberMap.Subscribe( msg_code, pMsgACT );
}

void BcmMsgPublisherOneToOne::UnSubscribe( const unsigned int msg_code )
{
    CallTrace("BcmMsgPublisherOneToOne", "UnSubscribe");

    fSubscriberMap.UnSubscribe( msg_code );
}
