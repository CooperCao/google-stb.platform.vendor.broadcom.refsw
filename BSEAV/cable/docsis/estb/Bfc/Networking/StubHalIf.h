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
//  $Id: StubHalIf.h 1.15 2003/06/18 14:16:18Z koneal Release $
//
//  Filename:       StubHalIf.h
//  Author:         David Pullen
//  Creation Date:  March 21, 2000
//
//****************************************************************************
//  Description:
//      This is a stub implementation for a HalIf object.  It is primarily used
//      on host testing environments (e.g. Win32).
//
//****************************************************************************

#ifndef STUBHALIF_H
#define STUBHALIF_H

//********************** Include Files ***************************************

// My base class...
#include "HalIf.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


class BcmStubHalIf : public BcmHalIf
{
public:

    // Initializing Constructor.  Stores the information specified.
    //
    // Parameters:
    //      interfaceNumber - the interface number that is being assigned.
    //      interfaceType - the type of interface (gives a hint as to what the
    //                      physical device is).
    //      pDescription - text string describing the HalIf, for debugging.
    //
    // Returns:  N/A
    //
    BcmStubHalIf(unsigned int interfaceNumber, IanaInterfaceType interfaceType,
                 const char *pDescription = NULL, int ifSpeed = 0, int ifMtu = 1500,
                 BcmHalIf *pWrappedHal = NULL,
                 const BcmMacAddress &ifPhysAddress = kAllZerosMac);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmStubHalIf();
    
#if (SNMP_SUPPORT)

    // This method is where the HalIf class registers any SNMP bridge objects
    // it might have with the SNMP agent.
    //
    // Parameters:
    //      pAgent - pointer to the SNMP agent that I should register the bridge
    //               objects with.
    //
    // Returns:  Nothing.
    //
    virtual void RegisterSnmpObjects(BcmSnmpAgent *pAgent);

#endif

    // Returns the MTU for the interface.
    //
    // The default for the base class is to return 1500, which is commonly used
    // across numerous interfaces.  Derived classes should only override this
    // if they have a different MTU.
    //
    // Parameters:
    //      interfaceNumber - the interface number being querried; this is
    //                        primarily for subinterface support.
    //
    // Returns:
    //      The Maximum Transmit Unit for the interface.
    //
    virtual int IfMtu(int interfaceNumber = 0) const;

    // Returns the current speed of the interface, in bits per second.
    //
    // Parameters:
    //      interfaceNumber - the interface number being querried; this is
    //                        primarily for subinterface support.
    //
    // Returns:
    //      The interface speed.
    //
    virtual unsigned int IfSpeed(int interfaceNumber = 0) const;
    
    // Accessor for the ifPhyAddress value.
    // 
    // Parameters:
    //      pAgent - the SNMP agent that my interface is being querried for.
    //      interfaceNumber - the interface number being querried; this is
    //                        primarily for subinterface support.
    //
    // Returns:
    //      The MAC address which is this object's ifPhysAddress
    //
    virtual const BcmMacAddress &IfPhysAddress (const BcmSnmpAgent *pAgent = NULL, int interfaceNumber = 0) const;

    // Accessors for the interface's administrative and operational
    // status.
    // 
    // Parameters: 
    //      interfaceNumber: the sub-interfce to operate on for HAL's that 
    //          have sub-interfaces (e.g. DOCSIS CM HAL)
    //
    // Returns:
    //      The current state of the interace.
    //
    virtual IfStatus IfAdminStatus(int interfaceNumber = 0) const;
    virtual IfStatus IfOperStatus(int interfaceNumber = 0);

    // Sets the adminstrative status of the interface; this is the desired
    // state.  Legal values for adminStatus are kUp, kDown, and kTesting.  The
    // operational status of the interface may or may not change based on this;
    // for example if the admin status is set to kUp, but there is no cable
    // attached, then the operational status will remain kDown.
    //
    // Derived classes associated with h/w drivers should do something to
    // physically enable or disable the interface based on this (stop/start
    // DMA, turn the PHY on/off, etc).  If this is successful, then the derived
    // class should call this method so that the base class state is updated.
    //
    // Parameters:
    //      adminStatus - the desired state of the interface.  Must be one of
    //                    kUp, kDown or kTesting.
    //      interfaceNumber - the interface number, used to support
    //                        sub-interfaces.
    //
    // Returns:
    //      true if the admin status was set correctly.
    //      false if the admin status was not allowed, or if the state could
    //          not be set for some reason.
    //
    virtual bool IfAdminStatus(IfStatus adminStatus, int interfaceNumber = 0);

protected:

    // This method is called by the base class after it has done the initial
    // processing of the packet that is sent to us by the Packet Switch.  This
    // must be provided by derived classes.
    //
    // PR2257 - Change the TransmitPacket() API so that the source HalIf
    // is passed in.
    //
    // Parameters:
    //      pPacket - the packet to be sent.
    //      pSrcHalIf - the HalIf that is sending the packet to me.
    //
    // Returns:
    //      true if the packet was sent.
    //      false if there was a problem.
    //
    virtual bool TransmitPacketImpl(BcmEthernetPacket *pPacket,
                                    BcmHalIf *pSrcHalIf);

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

protected:

    BcmHalIf *fpWrappedHal;

    int fIfSpeed;
    int fIfMtu;
    BcmMacAddress fIfPhysAddress;

private:

    // Default Constructor.  Not supported.
    BcmStubHalIf(void);

    // Copy Constructor.  Not supported.
    BcmStubHalIf(const BcmStubHalIf &otherInstance);

    // Assignment operator.  Not supported.
    BcmStubHalIf & operator = (const BcmStubHalIf &otherInstance);
    
};


//********************** Inline Method Implementations ***********************

// This allows you to do the following:
//
//    cout << halIf << endl;
//
inline ostream & operator << (ostream &outputStream, const BcmStubHalIf &rhs)
{
    return rhs.Print(outputStream);
}


// Companion class to the BcmStubHalIf, allows us to share the wrapped HAL's
// counters but associate them with the stub.
class StubInterfaceCounters : public BcmInterfaceCounters
{
public:
    StubInterfaceCounters (BcmHalIf *pHal, BcmInterfaceCounters *pCounters);
   ~StubInterfaceCounters ();
   
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
    
protected:
    BcmInterfaceCounters *fpCounters;
};


#endif


