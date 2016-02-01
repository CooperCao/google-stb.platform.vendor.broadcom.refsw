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
//  $Id$
//
//  Filename:       NiHalIf.h
//  Author:         David Pullen
//  Creation Date:  March 23, 2000
//
//****************************************************************************
//  Description:
//      This is an implementation of a HalIf that really talks to a HAL from the
//      BCM3310 and BCM3350 V2 BSP.  Since all of those HALs have the same API
//      (but different entrypoints), the only thing I have to do is allow the
//      entrypoint to be specified.
//
//      The BcmNiHalIf class is a Network Interface (Ethernet, HPNA, USB, PCI,
//      etc.) HAL Interface class.
//
//****************************************************************************

#ifndef NIHALIF_H
#define NIHALIF_H

//********************** Include Files ***************************************

// My base class.
#include "HalIf.h"

#include "BcmString.h"

#if (BFC_INCLUDE_BATTERY_SUPPORT && POWER_MGMT)
// PR 9159: power mgmt support at NiHalIf level
#include "NiHalIfPowerMgmtACT.h"
#endif

extern "C"
{
    // PR2412 - For the ETHERNET_PACKET struct.
    #include "BfcHal.h"
}



//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


class BcmNiHalIf : public BcmHalIf
{
public:

    // Initializing Constructor.  Stores the information specified.
    //
    // Parameters:
    //
    // Returns:  N/A
    //
    BcmNiHalIf(unsigned int interfaceNumber, IanaInterfaceType interfaceType,
               const char *pDescription = NULL);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmNiHalIf();

    // This method causes the HalIf class to register itself with the HAL,
    // effectively loading the driver for the interface.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the driver was registered and initialize successfully.
    //      false if there was a problem.
    //
    virtual bool StartDriver(void) = 0;

public:

    // Accessor that returns the driver index.  This is used in cases (like
    // enet or the IP stacks) where there are multiple of them, and we need to
    // know which one is being addressed.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The driver index.
    //
    inline unsigned int DriverIndex(void) const;
    inline void DriverIndex(unsigned int index);

public:

    // Accessor for the ifDescr value, which is a string describing the
    // interface.
    //
    // Parameters:
    //      interfaceNumber - the interface number being querried; this is
    //                        primarily for subinterface support.
    //
    // Returns:
    //      The ifDescr string.
    //
    virtual const char *IfDescr(int interfaceNumber = 0) const;
    
    // Returns the MTU for the interface.
    //
    // The default for this class is to ask the hardware for an answer.
    // Derived classes may override this if they have a different MTU.
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
    virtual IfStatus IfOperStatus(int interfaceNumber = 0);
    virtual bool IfAdminStatus(IfStatus adminStatus, int interfaceNumber = 0);

	// Accessor that allows us to call the AdminStatus function but be able to 
	// differentiate the call between when SNMP calls it versus the software calls
	// it internally upon initialization.
	virtual bool IfInterfaceStatus(IfStatus adminStatus, int interfaceNumber = 0);

    // Dunno why, but the Linux compiler can't handle both of these being
    // named IfPromiscuousMode.  So give them different names so he won't get
    // confused.  Guess you get what you pay for...
    virtual bool GetIfPromiscuousMode(int interfaceNumber = 0) const;
    virtual bool SetIfPromiscuousMode(bool promiscuousMode, int interfaceNumber = 0);

    virtual bool IfConnectorPresent(int interfaceNumber = 0) const;

    // Accessors for interface counter values that aren't countable at my level.
    // These are almost always interface specific, and may even be counted in
    // hardware.
    //
    // Parameters:
    //      interfaceNumber - the interface number being querried; this is
    //                        primarily for subinterface support.
    //
    // Returns:
    //      The counter value.
    //
    virtual unsigned int IfInDiscards(int interfaceNumber = 0) const;
    virtual unsigned int IfInErrors(int interfaceNumber = 0) const;

    virtual unsigned int IfOutDiscards(int interfaceNumber = 0) const;
    virtual unsigned int IfOutErrors(int interfaceNumber = 0) const;

    // This is a more general-purpose MIB counter retrieval method.  You specify
    // the MIB_OID_... counter type, and I'll call the HAL, returning the value
    // that it gives me.  The HAL may hit the hardware to get the value, or it
    // may pull it from its memory.  Note that "counter" here is a bit of a
    // misnomer; this may be used to retrieve any kind of unsigned integer
    // kinds of MIB things.
    //
    // Parameters:
    //      MibId    - one of the MIB_OID_... values defined in halmibs.h.
    //      Argument - this is the argument that gets passed to the HAL in the
    //                 ulParam field of the query info handler.  For HalIf 
    //                 classes which have have more than one sub-interface,
    //                 (mainly the CM HAL), this may specify the interface to 
    //                 be operated on.  However, the meaning of this argument
    //                 really depends on the OID being queried.
    //
    // Returns:
    //      The requested MIB counter.
    //
    virtual unsigned int GetMibCounter(unsigned int MibOID);
    virtual unsigned int GetMibCounter(unsigned int MibOID, unsigned int Argument);
    
    // Just like GetMibCounter, only it attempts to SET the OID to the
    // value specified.  Returns true on success, false on failure.
    virtual bool SetMibCounter (unsigned int MibOID, unsigned int Value);
    virtual bool SetMibCounter (unsigned int MibOID, unsigned int Argument, unsigned int Value);
    
    // This is a more general-purpose MIB counter retrieval method.  You specify
    // the MIB_OID_... counter type, and I'll call the HAL, returning the value
    // that it gives me.  The HAL may hit the hardware to get the value, or it
    // may pull it from its memory.  This method is just like GetMibCounter,
    // except that it returns a signed quantity rather than unsigned.
    //
    // Parameters:
    //      MibId    - one of the MIB_OID_... values defined in halmibs.h.
    //      Argument - this is the argument that gets passed to the HAL in the
    //                 ulParam field of the query info handler.  For HalIf 
    //                 classes which have have more than one sub-interface,
    //                 (mainly the CM HAL), this may specify the interface to 
    //                 be operated on.  However, the meaning of this argument
    //                 really depends on the OID being queried.
    //
    // Returns:
    //      The requested signed integer MIB value.
    //
    virtual int GetMibInteger(unsigned int MibOID);
    virtual int GetMibInteger(unsigned int MibOID, unsigned int Argument);
    
    // Just like GetMibInteger, only it attempts to SET the OID to the
    // value specified.  Returns true on success, false on failure.
    virtual bool SetMibInteger (unsigned int MibOID, int Value);
    virtual bool SetMibInteger (unsigned int MibOID, unsigned int Argument, int Value);
    
    // Another general-purpose MIB retrieval method, only this one gets an
    // octet buffer rather than an integral value.  Between this and GetMibCounter,
    // most MIB types are accounted for.  You specify
    // the MIB_OID_..., and I'll call the HAL, returning the value
    // that it gives me.  The HAL may hit the hardware to get the value, or it
    // may pull it from its memory.
    //
    // Parameters:
    //      MibOID - one of the MIB_OID_... values defined in halmibs.h.
    //      unsigned char *pBuf - buffer passed in to receive the octet buffer
    //      int MaxSize - how big is pBuf?
    //      interfaceNumber - for HalIf classes have have more than one
    //                        sub-interface (mainly the CM HAL), this lets you
    //                        specify the interface to be operated on.
    //
    //      The size (excluding NULL terminator for ASCII strings) of the string 
    //      returned.  -1 if the QueryInfo call fails.
    //
    virtual int GetMibOctetBuffer (unsigned int MibOID, unsigned char *pBuf, int MaxSize);
    virtual int GetMibOctetBuffer (unsigned int MibOID, unsigned char *pBuf, int MaxSize, unsigned int Argument);
    virtual int GetMibOctetBuffer (unsigned int MibOID, BcmString &Buf, unsigned int Argument);
    virtual int GetMibOctetBuffer (unsigned int MibOID, BcmString &Buf);
    
    // Just like GetMibOctetBuffer, only it attempts to SET the OID to the
    // value specified.  Returns true on success, false on failure.
    virtual bool SetMibOctetBuffer (unsigned int MibOID, unsigned char *pValue, int Size);
    virtual bool SetMibOctetBuffer (unsigned int MibOID, unsigned char *pValue, int Size, unsigned int Argument);
    virtual bool SetMibOctetBuffer (unsigned int MibOID, const BcmString &Buf, unsigned int Argument);
    virtual bool SetMibOctetBuffer (unsigned int MibOID, const BcmString &Buf);

protected:

    // This method is called by the base class after it has done the initial
    // processing of the packet that is sent to us by the Packet Switch.  This
    // must be provided by derived classes.
    //
    // PR2257 - the source HalIf is specified now.
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

    // This method is called by the HalIndicateStatus function.  I use it to
    // process status information related to my HAL/HalIf.
    //
    // Parameters:
    //      cmMacOid - the status code OID.
    //      parameter - OID-specific parameter.
    //      pStatusBuffer - OID-specific buffer pointer for status info.
    //      statusBufferLength - the number of bytes in pStatusBuffer.
    //      handle - the "this" pointer for my class.
    //
    // Returns:  Nothing.
    //
    virtual void HandleStatusIndication(BFC_MAC_OID cmMacOid, ULONG parameter,
                                        PVOID pStatusBuffer,
                                        ULONG statusBufferLength);


#if (BFC_INCLUDE_BATTERY_SUPPORT && POWER_MGMT)                                        
    // This method is called in response to a power management event by way
    // of our BcmNiHalIfPowerManagementACT member (PR 9159)
    //
    // Parameters:
    //      PowerMgmtState - true: entering power save mode or remaining
    //                       in power save mode but at a different level
    //                       false: exiting power save mode
    //
    // Returns:  Nothing
    // 
    virtual void PowerMgmtEvent (bool PowerMgmtState);
#endif

protected:

    // This method causes the HalIf class to register itself with the HAL (via
    // the specified entrypoint).
    //
    // PR534 - made this virtual so that the DocsisCmHalIf derived class can
    // overload it.
    //
    // Parameters:
    //      DriverEntryPoint - pointer to the function corresponding to the
    //                         driver entrypoint that I should call.
    //
    // Returns:
    //      true if the driver was registered and initialize successfully.
    //      false if there was a problem.
    //
    virtual bool Register(PFN_BFC_HAL_DRIVER_ENTRY DriverEntryPoint);

protected:

    // These are the static callback functions that I give to the HAL.  In all
    // cases, the handle parameter is the "this" pointer for my class.

    // This is what the HAL calls when I register myself with it.  This is how
    // it gives me it's entrypoints for sending data and returning buffers,
    // etc.
    //
    // This function dereferences the "this" pointer and calls the protected
    // implementation method.
    //
    // Parameters:
    //      pHalChars - pointer to a struct that contains the HAL
    //                  characteristics.
    //      handle - the "this" pointer for my class.
    //
    // Returns:
    //      The result of the operation.
    //
    static BFC_STATUS HalRegisterCallback(PBFC_HAL_CHARACTERISTICS pHalChars, 
                                         BcmHandle handle);

    // This is what the HAL calls when a packet is received from the network.
    //
    // This function dereferences the "this" pointer and calls the protected
    // implementation method.
    //
    // Parameters:
    //      pPacket - pointer to the ETHERNET_PACKET structure, which has the
    //                ethernet frame.
    //      handle - the "this" pointer for my class.
    //
    // Returns:  Nothing.
    //
    static void HalIndicateDataPacket(ETHERNET_PACKET *pPacket, BcmHandle handle);

     // This is what the HAL calls when a packet is received from the network when a Mask is applied to the 
    // Broadcast packets.
    //
    // This function dereferences the "this" pointer and calls the protected
    // implementation method.
    //
    // Parameters:
    //      mask    - the mask used to determine which interface broadcast are sent
    //      pPacket - pointer to the ETHERNET_PACKET structure, which has the
    //                ethernet frame.
    //      handle - the "this" pointer for my class.
    //      skipInbound - whether to skip Inbound snoops 
    //
    // Returns:  Nothing.
    //
    static void HalIndicateDataPacketWithMask(uint32 mask, ETHERNET_PACKET *pPacket, BcmHandle handle, 
                                                                                      bool skipInbound);

 
    // This is what the HAL calls when the specified packet has been transmitted
    // to the network.  Presumably, we had previously called the HAL function
    // that queues the packet for transmission.  This allows us to return the
    // packet to the entity that sent it to us originally.
    //
    // This function dereferences the "this" pointer and calls the protected
    // implementation method.
    //
    // Parameters:
    //      pPacket - pointer to the ETHERNET_PACKET structure, which has the
    //                ethernet frame.
    //      handle - the "this" pointer for my class.
    //
    // Returns:  Nothing.
    //
    static void HalIndicateDataPacketSendComplete(ETHERNET_PACKET *pPacket, 
                                                  BcmHandle handle);

    // This is what the HAL calls when it needs to provide status information
    // to us.
    //
    // This function dereferences the "this" pointer and calls the protected
    // implementation method.
    //
    // Parameters:
    //      cmMacOid - the status code OID.
    //      parameter - OID-specific parameter.
    //      pStatusBuffer - OID-specific buffer pointer for status info.
    //      statusBufferLength - the number of bytes in pStatusBuffer.
    //      handle - the "this" pointer for my class.
    //
    // Returns:  Nothing.
    //
    static void HalIndicateStatus(BFC_MAC_OID cmMacOid, ULONG parameter,
                                  PVOID pStatusBuffer, ULONG statusBufferLength,
                                  BcmHandle handle);

protected:

    // These are my callback functions and other data, which I give to the
    // HAL in Register().
    BFC_MAC_CHARACTERISTICS fMacChars;
    
    // These are the callback functions and other data that are given to me
    // by the HAL when it calls HalRegisterCallback.
    BFC_HAL_CHARACTERISTICS fHalChars;

    // This is the index value of the driver (for example, when there are
    // multiple ethernet drivers/ports, we need to tell the driver which one
    // we are addressing).
    unsigned int fDriverIndex;
    
#if (BFC_INCLUDE_BATTERY_SUPPORT && POWER_MGMT)
    // PR 9159: power mgmt support at NiHalIf level
    BcmNiHalIfPowerMgmtACT fPowerMgmtACT;
    
    bool fPowerMgmtState;
    int  fPowerMgmtLevel;
#endif

private:

    // Default Constructor.  Not supported.
    BcmNiHalIf(void);

    // Copy Constructor.  Not supported.
    BcmNiHalIf(const BcmNiHalIf &otherInstance);

    // Assignment operator.  Not supported.
    BcmNiHalIf & operator = (const BcmNiHalIf &otherInstance);

public:

    // These are helper methods for debugging/console purposes.  They simply
    // call the equivalent HAL Mode function on behalf of the client.  These are
    // not meant to be called by the application, because the app should not be
    // aware of HAL specifics.
    //
    // Parameters:
    //      mode - the #define for the mode that is being called.
    //      param - any parameters relevant to this mode.
    //      flag - true to enable the mode, false to disable.
    //      pFlag - point to a bool into which the mode value is to be stored.
    //
    // Returns:
    //      A status value for the mode call (success/fail/reason for failure).
    //
    virtual BFC_STATUS HalSetMode(BFC_HAL_MODE mode, uint32 param, bool flag);
    virtual BFC_STATUS HalQueryMode(BFC_HAL_MODE mode, uint32 param, bool *pFlag) const;

    // These are helper methods for debugging/console purposes.  They simply
    // call the equivalent HAL Info function on behalf of the client.  These are
    // not meant to be called by the application, because the app should not be
    // aware of HAL specifics.
    //
    // Parameters:
    //      oid - the #define for the info that is being called.
    //      param - any parameters relevant to this info.
    //      pInfoBuffer - pointer to the buffer containing the info data to be
    //                    written or read.
    //      infoBufferLength - the number of bytes in pInfoBuffer to be written.
    //      pInfoBufferLength - set this to the max size of pInfoBuffer on
    //                         input; the HAL will set it to the number of bytes
    //                         of data on output.
    //
    // Returns:
    //      A status value for the mode call (success/fail/reason for failure).
    //
    virtual BFC_STATUS HalSetInfo(BFC_HAL_OID oid, uint32 param, void *pInfoBuffer, uint32 infoBufferLength);
    virtual BFC_STATUS HalQueryInfo(BFC_HAL_OID oid, uint32 param, void *pInfoBuffer, uint32 *pInfoBufferLength) const;
    
#if (BFC_INCLUDE_BATTERY_SUPPORT && POWER_MGMT)
// PR 9159: power mgmt support at NiHalIf level
friend class BcmNiHalIfPowerMgmtSoftTimer;
friend class BcmNiHalIfPowerMgmtACT;
#endif
};


//********************** Inline Method Implementations ***********************


// This allows you to do the following:
//
//    cout << halIf << endl;
//
inline ostream & operator << (ostream &outputStream, const BcmNiHalIf &rhs)
{
    return rhs.Print(outputStream);
}


// Accessor that returns the driver index.  This is used in cases (like
// enet or the IP stacks) where there are multiple of them, and we need to
// know which one is being addressed.
//
// Parameters:  None.
//
// Returns:
//      The driver index.
//
inline unsigned int BcmNiHalIf::DriverIndex(void) const
{
    return fDriverIndex;
}


inline void BcmNiHalIf::DriverIndex(unsigned int index) 
{
    fDriverIndex = index;
}

#endif


