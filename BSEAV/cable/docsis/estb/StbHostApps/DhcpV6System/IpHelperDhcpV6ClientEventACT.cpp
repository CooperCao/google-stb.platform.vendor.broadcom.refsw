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
//  Filename:       IpHelperDhcpV6ClientEventACT.cpp
//  Author:         David Pullen
//  Creation Date:  June 15, 2006
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "IpHelperDhcpV6ClientEventACT.h"

#include "EstbIpThread.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


BcmIpHelperDhcpV6ClientEventACT::BcmIpHelperDhcpV6ClientEventACT(BcmEstbIpThread *pIpThread) :
    BcmCompletionHandlerACT(),
    pfIpThread(pIpThread)
{
    // Nothing else to do.
}


BcmIpHelperDhcpV6ClientEventACT::~BcmIpHelperDhcpV6ClientEventACT()
{
    // Clear the pointer, but don't delete the object.
    pfIpThread = NULL;
}


void BcmIpHelperDhcpV6ClientEventACT::HandleEvent(const BcmCompletionEvent &eventCode)
{
    // Make sure somebody was registered with me.
    if (pfIpThread != NULL)
    {
        pfIpThread->ProcessDhcpV6Event(eventCode);
    }
}

