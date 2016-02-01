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
//  $Id: InterfaceCounters.h 1.4 2003/01/16 20:48:58Z dpullen Release $
//
//  Filename:       InterfaceCounters.h
//  Author:         David Pullen
//  Creation Date:  October 29, 2002
//
//****************************************************************************
//  Description:
//      This is a helper class for all kinds of interfaces (BcmHalIf); it keeps
//      track of various counter values, most of which correspond directly to
//      ifTable and ifXTable MIB entries.  This object isn't tied to SNMP or
//      any stinkin' MIBs, but it is the class that the MIBs read the counters
//      from.
//
//      Previously, each interface driver was responsible for keeping these
//      counters; this let to an ugly amount of duplicate code spread out over
//      several dozen files, with lots of calculations (like is-broadcast,
//      is-unicast, etc) being repeated for each packet.
//
//      In order to clean this up, and in order to support driver-less
//      interfaces, we moved the counters here, and made a single point in the
//      code where the counters are all updated, etc.
//
//****************************************************************************

#ifndef InterfaceCounters_H
#define InterfaceCounters_H

//********************** Include Files ***************************************

#include "Counter64.h"

#include "EthernetPacket.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmHalIf;

//********************** Class Declaration ***********************************


class BcmInterfaceCounters
{
public:

    // Initializing Constructor.  Initializes the state of the object...
    //
    // Parameters:
    //      pHalIf - the interface that I am associated with.
    //
    // Returns:  N/A
    //
    BcmInterfaceCounters(BcmHalIf *pHalIf);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmInterfaceCounters();

    // Clears the counters.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    virtual void Clear(void);

    // Accessor for the HalIf object that I am associated with.
    //
    // Parameters:  None.
    //
    // Returns:
    //      Pointer to the HalIf that I am associated with.
    //
    inline BcmHalIf *Interface(void);

    // Prints the counters to the specified ostream.
    //
    // Parameters:
    //      outStream - the ostream to print to.
    //
    // Returns:
    //      A reference to the ostream for chaining.
    //
    virtual ostream & Print(ostream &outStream) const;

public:

    // Accessors to return the counters.  There are two versions of these for
    // most counters in order to handle both 32-bit and 64-bit versions of the
    // values.  These methods are all virtual so that a derived class can
    // override the implementation in order to handle sub-interface cases, where
    // they may return the parent interface counter plus their counter.

    virtual uint32 ReceivedOctets(void) const;
    virtual void ReceivedOctets(BcmCounter64 &counter) const;

    virtual uint32 ReceivedUnicastPackets(void) const;
    virtual void ReceivedUnicastPackets(BcmCounter64 &counter) const;

    virtual uint32 ReceivedMulticastPackets(void) const;
    virtual void ReceivedMulticastPackets(BcmCounter64 &counter) const;

    virtual uint32 ReceivedBroadcastPackets(void) const;
    virtual void ReceivedBroadcastPackets(BcmCounter64 &counter) const;

    virtual uint32 ReceivedPacketsNotForwarded(void) const;


    virtual uint32 TransmittedOctets(void) const;
    virtual void TransmittedOctets(BcmCounter64 &counter) const;

    virtual uint32 TransmittedUnicastPackets(void) const;
    virtual void TransmittedUnicastPackets(BcmCounter64 &counter) const;

    virtual uint32 TransmittedMulticastPackets(void) const;
    virtual void TransmittedMulticastPackets(BcmCounter64 &counter) const;

    virtual uint32 TransmittedBroadcastPackets(void) const;
    virtual void TransmittedBroadcastPackets(BcmCounter64 &counter) const;

    virtual uint32 TransmitPacketsDiscarded(void) const;
    virtual uint32 TransmitPacketsMtuExceeded(void) const;

public:  // Methods to update the counters.

    // Tells the counters that a packet was received on the interface, and how
    // big the packet is.
    //
    // Parameters:
    //      packetSize - the number of bytes in the packet (including CRC32).
    //      packetType - the type of packet (MAC DA).
    //
    // Returns:  Nothing.
    //
    inline void PacketReceived(unsigned int packetSize,
                               BcmEthernetPacket::MacAddrType packetType);

    // Tells the counters that a packet was received on the interface but then
    // was not forwarded for some reason (e.g. filters).
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    inline void ReceivedPacketNotForwarded(void);

    // Tells the counters that a packet was transmitted on the interface, and
    // how big the packet is.  NOTE:  This should be called AFTER the packet
    // is actually transmitted, generally when the buffer is being released.
    //
    // Parameters:
    //      packetSize - the number of bytes in the packet (including CRC32).
    //      packetType - the type of packet (MAC DA).
    //
    // Returns:  Nothing.
    //
    inline void PacketTransmitted(unsigned int packetSize,
                                  BcmEthernetPacket::MacAddrType packetType);

    // Tells the counters that an outbound packet could not be transmitted for
    // some reason specific to the interface, usually because of insufficient
    // resources.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    inline void TransmitPacketWasDiscarded(void);

    // Tells the counters that an outbound packet was discarded for upper layer
    // reasons; this counteracts a call to TransmitPacketWasDiscarded(), causing
    // the counter to be decremented rather than incremented.
    //
    // This is useful when a packet is being discarded for upper-layer reasons
    // (such as rate shaping), and not for low-layer reasons (such as queue
    // full).  When the packet is discarded, the core interface code will call
    // TransmitPacketWasDiscarded(); however, since the derived class knows that
    // the packet is being discarded for upper-layer reasons, we need to make
    // sure that the counter does not increment.  Calling this method reverses
    // what the core code will do.
    //
    // This was added for PR4236.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    inline void TransmitPacketWasDiscardedUpperLayer(void);

    // Tells the counters that an outbound packet exceeds the MTU.  This will
    // generally be called along with TransmitPacketDiscarded(), since an
    // oversize packet will usually have to be dropped.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    inline void TransmitPacketMtuExceeded(void);

protected:

    // Pointer to the HalIf that I am associated with.
    BcmHalIf *pfHalIf;

    // Various counters.  Most of these are stored natively as 64-bit counters,
    // but they can be read as either 32 or 64 bits.
    BcmCounter64 fReceivedOctets;
    BcmCounter64 fReceivedPackets[3];

    uint32 fReceivedPacketsNotForwarded;

    BcmCounter64 fTransmittedOctets;
    BcmCounter64 fTransmittedPackets[3];

    uint32 fTransmitPacketsDiscarded;
    uint32 fTransmitPacketsMtuExceeded;

private:

    // Default Constructor.  Not supported.
    BcmInterfaceCounters(void);

    // Copy Constructor.  Not supported.
    BcmInterfaceCounters(const BcmInterfaceCounters &otherInstance);

    // Assignment operator.  Not supported.
    BcmInterfaceCounters & operator = (const BcmInterfaceCounters &otherInstance);

};


//********************** Inline Method Implementations ***********************


inline ostream & operator << (ostream &outStream, const BcmInterfaceCounters &counters)
{
    return counters.Print(outStream);
}


// Accessor for the HalIf object that I am associated with.
//
// Parameters:  None.
//
// Returns:
//      Pointer to the HalIf that I am associated with.
//
inline BcmHalIf *BcmInterfaceCounters::Interface(void)
{
    return pfHalIf;
}


// Tells the counters that a packet was received on the interface, and how
// big the packet is.
//
// Parameters:
//      packetSize - the number of bytes in the packet (including CRC32).
//      packetType - the type of packet (MAC DA).
//
// Returns:  Nothing.
//
inline void BcmInterfaceCounters::PacketReceived(unsigned int packetSize,
                                                 BcmEthernetPacket::MacAddrType packetType)
{
    // Update the octets counter.
    fReceivedOctets += packetSize;

    // Update the appropriate packets counter.
    fReceivedPackets[packetType]++;
}


// Tells the counters that a packet was received on the interface but then
// was not forwarded for some reason (e.g. filters).
//
// Parameters:  None.
//
// Returns:  Nothing.
//
inline void BcmInterfaceCounters::ReceivedPacketNotForwarded(void)
{
    fReceivedPacketsNotForwarded++;
}


// Tells the counters that a packet was transmitted on the interface, and
// how big the packet is.  NOTE:  This should be called AFTER the packet
// is actually transmitted, generally when the buffer is being released.
//
// Parameters:
//      packetSize - the number of bytes in the packet (including CRC32).
//      packetType - the type of packet (MAC DA).
//
// Returns:  Nothing.
//
inline void BcmInterfaceCounters::PacketTransmitted(unsigned int packetSize,
                                                    BcmEthernetPacket::MacAddrType packetType)
{
    // Update the octets counter.
    fTransmittedOctets += packetSize;

    // Update the appropriate packets counter.
    fTransmittedPackets[packetType]++;
}


// Tells the counters that an outbound packet could not be transmitted for
// some reason specific to the interface, usually because of insufficient
// resources.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
inline void BcmInterfaceCounters::TransmitPacketWasDiscarded(void)
{
    fTransmitPacketsDiscarded++;
}


// Tells the counters that an outbound packet was not discarded; this
// counteracts a call to TransmitPacketWasDiscarded(), causing the counter
// to be decremented rather than incremented.
//
// This is useful when a packet is being discarded for upper-layer reasons
// (such as rate shaping), and not for low-layer reasons (such as queue
// full).  When the packet is discarded, the core interface code will call
// TransmitPacketWasDiscarded(); however, since the derived class knows that
// the packet is being discarded for upper-layer reasons, we need to make
// sure that the counter does not increment.  Calling this method reverses
// what the core code will do.
//
// This was added for PR4236.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
inline void BcmInterfaceCounters::TransmitPacketWasDiscardedUpperLayer(void)
{
    fTransmitPacketsDiscarded--;
}


// Tells the counters that an outbound packet exceeds the MTU.  This will
// generally be called along with TransmitPacketDiscarded(), since an
// oversize packet will usually have to be dropped.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
inline void BcmInterfaceCounters::TransmitPacketMtuExceeded(void)
{
    fTransmitPacketsMtuExceeded++;
}


#endif


