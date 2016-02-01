//****************************************************************************
//
//  Copyright (c) 2008  Broadcom Corporation
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
//  $Id$
//
//  Filename:       DhcpV6SocketIf.h
//  Author:         Pinar Taskiran Cyr
//  Creation Date:  April 29, 2008
//
//****************************************************************************
// 
//
//****************************************************************************

#ifndef DHCPV6SOCKETIF_H
#define DHCPV6SOCKETIF_H

#include "StubHalIf.h"

class BcmDhcpV6SocketIf : public BcmStubHalIf
{
public:

    // Initializing Constructor.  Stores the information specified.
    //
    // Parameters:
    //      pWrappedHal - if the design is using a real physical medium
    //                    (ie Ethernet, USB, etc) to transmit the data, a 
    //                    pointer to that HAL.  If it's a pure software
    //                    design, then this will be NULL.
    //
    // Returns:  N/A
    //
    BcmDhcpV6SocketIf (BcmHalIf *pWrappedHal = NULL);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmDhcpV6SocketIf();
	
protected:

	    // This method is called by the base class after it has done initial
    // processing of the packet that is being released.  If the base class
    // calls this, then the packet really needs to be released.  This must be
    // provided by the derived classes.
    //
    // Parameters:
    //      pPacket - the packet to be released.
    //
    // Returns:
    //      true if the packet was released.
    //      false if the packet was not released.
    //
    virtual bool ReleasePacketImpl(BcmEthernetPacket *pPacket);

private:

    // Default Constructor.  Not supported.
    BcmDhcpV6SocketIf(void);

    // Copy Constructor.  Not supported.
    BcmDhcpV6SocketIf(const BcmDhcpV6SocketIf &otherInstance);

    // Assignment operator.  Not supported.
    BcmDhcpV6SocketIf & operator = (const BcmDhcpV6SocketIf &otherInstance);
};

#endif
