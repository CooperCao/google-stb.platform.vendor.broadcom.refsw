//****************************************************************************
//
//  Copyright (c) 2003  Broadcom Corporation
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
//  Filename:       MibFactory.h
//  Author:         Kevin O'Neal
//  Creation Date:  June 4, 2003
//
//****************************************************************************
//  Description:
//      This class is responsible for creating the correct type of
//      various MIB objects, based on the OS selection.  In general these
//      are MIB which come from the OS or IP stack, and which vary per
//      OS
//
//****************************************************************************

#ifndef MIBFACTORY_H
#define MIBFACTORY_H

#include "ipBridge.h"
#include "udpBridge.h"

class BcmMibFactory
{
  public:
    // RFC-2011 objects
    static ipGroupBridge *NewIpGroupBridge (BcmSnmpAgent *pAgent);
    static icmpGroupBridge *NewIcmpGroupBridge (BcmSnmpAgent *pAgent);
    
    // RFC-2013 objects
    static udpGroupBridge *NewUdpGroupBridge (BcmSnmpAgent *pAgent);
    static void UpdateUdpTable (BcmSnmpAgent *pAgent);
    
  protected:
    // I have to declare all of these as protected in order to keep this class
    // from being used inappropriately.  The compiler will generate these
    // methods for you if you don't declare them yourself!

    // Default Constructor.  Not supported.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmMibFactory( void );

    // Destructor.  Not supported.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmMibFactory();
};

#endif
