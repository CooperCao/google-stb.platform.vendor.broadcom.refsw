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
//  $Id: MsgACT.h 1.1 2005/04/01 19:02:30Z cliffd Release $
//
//
//  Filename:       MsgACT.h
//  Author:         Cliff Danielson
//  Creation Date:  Jan 10, 2005
//
//****************************************************************************
//  Description:
//      Derived class BcmMsgACT specializes the BcmCompletionHandlerACT base 
//      class by adding a void* message parameter data member.
//
//****************************************************************************
//
// MsgACT.h
//
#ifndef MSGACT_H
#define MSGACT_H

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iostream>
#else
#include <iostream.h>
#endif

#include "CompletionHandlerACT.h"

#if (__GNUC__ >= 3)
using namespace std;
#endif


class BcmMsgACT : public BcmCompletionHandlerACT
{
public:
 
    // Constructor. 
    //
    BcmMsgACT( void* pMsgParam = NULL );

    // Destructor.  
    //
    virtual ~BcmMsgACT();

    // Print formatted object contents to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& Print( ostream& ostrm ) const;

    // HandleEvent - in Asynchronous Completion Token pattern, this function
    //      can be used by the Initiator object (or it's BcmCallbackHandler
    //      helper object to dispatch a specific completion handler that
    //      processes the response from an asynchronous operation.
    //
    // Parameters:
    //      event_code - unsigned int event code which identifies the 
    //          asynchronous event type which occurred.  

    virtual void HandleEvent( const BcmCompletionEvent &event_code );

    // misc accessors:
    //
    void MsgParam( void* pMsgParam ) { pfMsgParam = pMsgParam; }
    void* MsgParam() const { return pfMsgParam; }
    
protected:
    // void pointer which either contains or points to data associated 
    // with the message.  usage depends on msg type (a.k.a. event_code).
    void* pfMsgParam;
};

inline ostream& operator<<( ostream& ostrm, const BcmMsgACT& rhs )
{
    return rhs.Print( ostrm );
}

#endif
