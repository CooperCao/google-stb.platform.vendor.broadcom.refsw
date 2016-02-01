//****************************************************************************
//
//  Copyright (c) 2000  Broadcom Corporation
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
//  $Id: IpHelperFactory.h 1.6 2002/06/20 01:21:13Z dpullen Release $
//
//  Filename:       IpHelperFactory.h
//  Author:         David Pullen
//  Creation Date:  Jan 26, 2000
//
//****************************************************************************
//  Description:
//      This is a functor factory class who's job is to construct the
//      appropriate OS-specific IP helper derived class.
//
//      This class only has static methods; you don't need to (and can't)
//      create an instance of it.
//
//****************************************************************************

#ifndef IPHELPERFACTORY_H
#define IPHELPERFACTORY_H

//********************** Include Files ***************************************

#include "OperatingSystem.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

// These are the classes that I know how to create.
class BcmTftp;

#if (BCM_DHCPS_SUPPORT)
class BcmDhcpServer;
#endif


//********************** Class Declaration ***********************************


class BcmIpHelperFactory
{
public:


#if (BCM_DHCPS_SUPPORT)
    static BcmDhcpServer *NewDhcpServer(void);
#endif

    // Creates a new instance of a BcmTftp for the target environment.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A pointer to the object that was created, or NULL if there was a
    //      problem creating it.
    //
    static BcmTftp *NewTftp(void);

protected:

    // Default Constructor.  Not supported.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmIpHelperFactory(void);

    // Destructor.  Not supported.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmIpHelperFactory();

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmIpHelperFactory(const BcmIpHelperFactory &otherInstance);

    
    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmIpHelperFactory & operator = (const BcmIpHelperFactory &otherInstance);

};


//********************** Inline Method Implementations ***********************

#endif


