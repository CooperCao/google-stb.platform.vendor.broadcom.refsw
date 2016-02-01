//****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16251 Laguna Canyon Road
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       IpHelperDhcpV6ClientEventACT.h
//  Author:         David Pullen
//  Creation Date:  June 15, 2006
//
//****************************************************************************
//  Description:
//      This class specializes the base class to allow the CmDocsisIpThread
//      to receive and act on events published by the DHCPv6 client.
//
//      Refer to POSA2 book, page 268. 
//
//****************************************************************************

#ifndef IpHelperDhcpV6ClientEventACT_H
#define IpHelperDhcpV6ClientEventACT_H

//********************** Include Files ***************************************

#include "CompletionHandlerACT.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmEstbIpThread;

//********************** Class Declaration ***********************************


class BcmIpHelperDhcpV6ClientEventACT : public BcmCompletionHandlerACT
{
public:
   
    // Initializing Constructor. 
    //
    // Parameters:
    //      pIpThread - pointer to the IP thread that I must notify.
    //
    // Returns:  N/A
    //
    BcmIpHelperDhcpV6ClientEventACT(BcmEstbIpThread *pIpThread);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmIpHelperDhcpV6ClientEventACT();

    // HandleEvent - in Asynchronous Completion Token pattern, this function
    //      can be used by the Initiator object (or it's BcmCallbackHandler
    //      helper object) to dispatch a specific completion handler that
    //      processes the response from an asynchronous operation.
    //
    // Parameters:
    //      eventCode - integer event code which identifies the asynchronous
    //                  event type that occurred.
    //
    // Returns:  Nothing.
    //
    virtual void HandleEvent(const BcmCompletionEvent &eventCode);

private:

    // Pointer to the client object.
    BcmEstbIpThread *pfIpThread;

private:

    // Default Constructor.  Not supported.
    BcmIpHelperDhcpV6ClientEventACT(void);

    // Copy Constructor.  Not supported.
    BcmIpHelperDhcpV6ClientEventACT(const BcmIpHelperDhcpV6ClientEventACT &otherInstance);

    // Assignment operator.  Not supported.
    BcmIpHelperDhcpV6ClientEventACT & operator = (const BcmIpHelperDhcpV6ClientEventACT &otherInstance);

};


//********************** Inline Method Implementations ***********************

#endif


