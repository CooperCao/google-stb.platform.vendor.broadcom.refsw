//****************************************************************************
//
//  Copyright (c) 2005  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id: MsgACT.cpp 1.1 2005/04/01 19:02:30Z cliffd Release $
//
//  Filename:       MsgACT.cpp
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
// MsgACT.cpp
//

#include "MsgACT.h"
#include "MessageLog.h"

BcmMsgACT::BcmMsgACT( void* pMsgParam )
  :
    BcmCompletionHandlerACT(),
    pfMsgParam( pMsgParam )
{
    CallTrace("BcmMsgACT", "constructor");
}

BcmMsgACT::~BcmMsgACT()
{
    CallTrace("BcmMsgACT", "destructor");
}


ostream& BcmMsgACT::Print( ostream& ostrm ) const
{    
    CallTrace("BcmMsgACT", "Print");
    ostrm 
        << "\nBcmMsgACT:" 
        << "\n    pfMsgParam = " << pfMsgParam
        << endl;

    return ostrm;
}


void BcmMsgACT::HandleEvent( 
    const BcmCompletionEvent &event_code )
{
    CallTrace("BcmMsgACT", "HandleEvent");
    
    gLogMessageRaw << "\nBcmMsgACT::HandleEvent() event_code = " << event_code 
        << *this;
}

