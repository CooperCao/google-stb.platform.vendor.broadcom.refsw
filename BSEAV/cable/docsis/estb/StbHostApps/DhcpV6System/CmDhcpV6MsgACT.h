//****************************************************************************
//
//  Copyright (c) 2007  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       CmDhcpV6MsgACT.h
//  Author:         David Pullen
//  Creation Date:  May 17, 2007
//
//****************************************************************************

#ifndef CmDhcpV6MsgACT_H
#define CmDhcpV6MsgACT_H

//********************** Include Files ***************************************

#include "MsgACT.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmDhcpV6ClientIf;

//********************** Class Declaration ***********************************


class BcmCmDhcpV6MsgACT : public BcmMsgACT
{
public:
   
    /// Constructor. 
    ///
    BcmCmDhcpV6MsgACT();

    /// Destructor.  
    ///
    virtual ~BcmCmDhcpV6MsgACT();


    /// HandleEvent - in Asynchronous Completion Token pattern, this function
    ///      can be used by the Initiator object (or it's BcmCallbackHandler
    ///      helper object to dispatch a specific completion handler that
    ///      processes the response from an asynchronous operation.
    ///
    /// \param
    ///      event_code - integer event code which identifies the asynchronous
    ///          event type which occurred.  
    ///
    virtual void HandleEvent( const BcmCompletionEvent &event_code );

    /// misc accessors.
    ///
    void DhcpV6ClientIf(BcmDhcpV6ClientIf *pClientIf) { pfDhcpV6ClientIf = pClientIf; }

private:

    /// Pointer to the DHCPv6 client object.
    BcmDhcpV6ClientIf *pfDhcpV6ClientIf;

private:

    /// Copy Constructor.  Not supported.
    BcmCmDhcpV6MsgACT( const BcmCmDhcpV6MsgACT& rhs );

    /// Assignment operator.  Not supported.
    BcmCmDhcpV6MsgACT& operator=( const BcmCmDhcpV6MsgACT& rhs );
};


#endif
