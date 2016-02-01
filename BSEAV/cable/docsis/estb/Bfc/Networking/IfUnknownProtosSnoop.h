//****************************************************************************
//
//  Copyright (c) 2002  Broadcom Corporation
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
//  $Id: IfUnknownProtosSnoop.h 1.2 2002/12/04 13:05:40Z dpullen Release $
//
//  Filename:       IfUnknownProtosSnoop.h
//  Author:         Kevin O'Neal
//  Creation Date:  September 11, 2002
//
//****************************************************************************
//  Description:
//      This is the Snoop class that counts packets with 'unknown' protocols
//      for the benefit of the ifInUnknownProtos counter.  It operates as an
//      outbound snoop on any IP stack interface, and examines each packet
//      passed to the IP stack for a protocol type that the IP stack doesn't
//      understand.  The IP stack itself counts unknown protocols in the
//      object ipInUnknownProtos.  If this object is implemented correctly,
//      the sum of all ifInUnknownProtos should add up to equal the scalar
//      ipInUnknownProtos object.  Some experimentation may be required to
//      determine exactly what kinds of packets are counted by the IP
//      stack as unknown, and our counter should be counting the same things.
//
//****************************************************************************

#ifndef IFUNKNOWNPROTOSSNOOP_H
#define IFUNKNOWNPROTOSSNOOP_H

//********************** Include Files ***************************************

// My base class.
#include "Snoop.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


class BcmIfUnknownProtosSnoop : public BcmSnoop
{
public:

    // Initializing Constructor.  Initializes the state of the object...
    //
    // Parameters:
    //      canBeDeleted - true if the object was "newed" and can be deleted.
    //                     false if the object was created statically or is
    //                     associated with some object that will delete it.
    //                     This is primarily to support the console, which
    //                     allows the user to create/delete Snoops.
    //
    // Returns:  N/A
    //
    BcmIfUnknownProtosSnoop(bool canBeDeleted);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmIfUnknownProtosSnoop();

protected:

    // This method is called by SnoopOutbound when the base class has finished
    // all of its pre-processing.
    //
    // The derived class can Snoop the packet here, overriding the base class
    // method, if it needs to know the source/dest HalIfs and the packet to do
    // its job.
    //
    // The base class simply returns kPassPacket without any other processing.
    // This is for the convenience of Snoops that only implement an Inbound
    // Snoop.
    //
    // Parameters:
    //      pPacket - the packet to be Snooped.
    //      pSrcHalIf - the HalIf that the packet was received on.
    //      pDestHalIf - the HalIf that the packet is being sent to.
    //
    // Returns:
    //      One of the SnoopResult enum values.
    //
    virtual SnoopResult SnoopOutboundImpl(BcmEthernetPacket *pPacket,
                                          BcmHalIf *pSrcHalIf,
                                          BcmHalIf *pDestHalIf);

private:

    // Default Constructor.  Not supported.
    BcmIfUnknownProtosSnoop(void);

    // Copy Constructor.  Not supported.
    BcmIfUnknownProtosSnoop(const BcmIfUnknownProtosSnoop &otherInstance);

    // Assignment operator.  Not supported.
    BcmIfUnknownProtosSnoop & operator = (const BcmIfUnknownProtosSnoop &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif


