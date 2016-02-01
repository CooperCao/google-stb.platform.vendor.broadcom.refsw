//****************************************************************************
//
// Copyright (c) 2000-2010 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//****************************************************************************
//  $Id$
//
//  Filename:       PacketSwitch.h
//  Author:         David Pullen
//  Creation Date:  March 13, 2000
//
//****************************************************************************
//  Description:
//      This is the abstract base class for all of the types of Packet Switches
//      (Forwarder, Router, etc.).  It accepts a packet from one "port", and
//      sends it out 0 or more ports, depending on the particular algorithm of
//      the object.
//
//      A Packet Switch contains a set of BcmHalIf objects, each on representing
//      a "port".  The BcmHalIf object is generally a C++ wrapper on a network
//      interface driver.
//
//      In addition to the list of HalIf objects, there is one special one that
//      is the default interface.  In general, the Packet Switch will ask each
//      registered interface whether or not the packet is destined for it.  If
//      it doesn't find one, then it sends it to the default interface.
//
//      Note that this object does not delete the HalIf objects that are given
//      to it.  The entity that created it must delete it.  You MUST remove it
//      from this object before deleting it.
//
//****************************************************************************

#ifndef PACKETSWITCH_H
#define PACKETSWITCH_H

//********************** Include Files ***************************************

#include "typedefs.h"

#include "MessageLog.h"

#include "MacAddress.h"

#include "LearningTable.h"

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iostream>
#include <vector>
#include <map>
#else
#include <vector.h>
#include <iostream.h>
#include <map.h>
#endif



//********************** Global Types ****************************************

//********************** Global Constants ************************************

// Set this to 1 if you want the forwarder/packet switch to use a mutex to
// protect itself against reentrancy.  This has a non-trivial impact on
// performance, but is safer.  For the most part, it doesn't seem to be
// necessary.  The main risk is with someone modifying the learning table via
// the console, which is not a normal thing to do.  Aside from that, it doesn't
// look like it is necessary.
//
// For now, we're leaving this as a compile time option so that we can add/remove
// it more easily.  We aren't yet clear that this is completely safe, so I want
// to be able to put it back in.
#define PACKETSWITCH_USE_MUTEX 0

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmMutexSemaphore;
class BcmHalIf;
class BcmEthernetPacket;

//********************** Class Declaration ***********************************

class BcmPacketSwitch
{
public:

    // Default Constructor.  Sets the list of HalIf objects, and the default
    // interface, to NULL.
    //
    // Parameters:
    //      maxLearningTableEntries - sets the maximum number of associations
    //                                that will be allowed in the learning table.
    //                                A value of 0 means that there is no limit.
    //      pInstanceName - name of the derived class (for debugging purposes).
    //
    // Returns:  N/A
    //
    BcmPacketSwitch(unsigned int maxLearningTableEntries = 0,
                    const char *pInstanceName = NULL);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.  Removes all of the HalIf objects from the list, but does not
    // delete them.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmPacketSwitch();

    // Accessor for the instance name given in the constructor.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The name of the object.
    //
    inline const char *Name(void) const;

public:

    // Management routines for the Packet Switch.  This is where interfaces are
    // added/removed, and their states are changed, and where the learning
    // table is modified.

    // Registers the specified HalIf object with me.  I add it to my list of
    // valid HalIf objects, and then register myself with the HalIf (so that it
    // will send received packets to me).
    //
    // Note that HalIf instances registered here are not the default interface.
    // To register the default HalIf object, call RegisterDefault().  A packet
    // will only be sent to these HalIf objects if there is an explicit
    // association to them in the learning table.
    //
    // Parameters:
    //      pHalIf - pointer to the HalIf object to be registered.
    //
    // Returns:
    //      true if the specified HalIf was registered (or was already
    //          registered).
    //      false if there was a problem (NULL pointer, HalIf is the default,
    //          etc.).
    //
    virtual bool Register(BcmHalIf *pHalIf);

    // This allows some control entity to force me to remove all entries from
    // my learning table that are associated with the specified HalIf instance.
    // This is necessary when the HalIf is being deleted (so I don't send any
    // more packets to it!).
    //
    // This also de-registers the specified HalIf instance.
    //
    // Note that this will not work on the default HalIf; you must use 
    // RegisterDefault(NULL) to remove the default HalIf.
    //
    // Parameters:
    //      pHalIf - pointer to the HalIf object to be forgotten.
    //
    // Returns:
    //      true if successful.
    //      false if the specified HalIf was not found.
    //
    bool Deregister(BcmHalIf *pHalIf);

    // Sets the specified HalIf instance as the default interface.  If a packet
    // has no association with any other interface in the learning table, then
    // it will be sent to this one.
    //
    // You can specify NULL as the parameter if you want to deregister the
    // current default HalIf.
    //
    // Parameters:
    //      pHalIf - pointer to the HalIf object to be set as the default.
    //
    // Returns:
    //      A pointer to the previous default HalIf object.
    //
    BcmHalIf *RegisterDefault(BcmHalIf *pHalIf);

    // Returns the default HalIf, if one was registered.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The default (WAN) HalIf, or NULL if there isn't one.
    //
    inline BcmHalIf *DefaultHalIf(void) const;

    // This deregisters all HalIf instances that had been registered, including
    // the default HalIf.  This also clears the learning table.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void DeregisterAll(void);

    // Returns the number of HalIf instances that have been registered.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The number of HalIf instances that have been registered.
    //
    unsigned int NumberOfRegisteredHalIf(void) const;

    // This method searches the list of registered HalIf instances for the one
    // corresponding to the index passed in.  If there is no HalIf for the
    // index, then it returns NULL.
    //
    // Parameters:
    //      index - the index of the HalIf to look for.
    //
    // Returns:
    //      A pointer to the HalIf, or NULL.
    //
    BcmHalIf *Find(unsigned int index) const;
    
    // This method searches the list of registered HalIf instances for the one
    // corresponding to the IfNum passed in.  If there is no HalIf for the
    // index, then it returns NULL.  Similar to Find, except instead of the
    // index into the local array of HAL's, it returns the object with the
    // given interface number.
    //
    // Parameters:
    //      IfNum - the interface number of the HalIf to look for.
    //
    // Returns:
    //      A pointer to the HalIf, or NULL.
    //
    BcmHalIf *FindByIfNum (unsigned int IfNum) const;

    // Modifies the maximum number of associations that will be allowed in the
    // learning table.  Note that this doesn't cause any associations to be
    // removed from the learning table, even if this is less than the number of
    // associations that are already in it.
    //
    // Parameters:
    //      maxLearningTableEntries - the maximum number of associations that
    //                                will be allowed in the learning table.
    //                                A value of 0 means that there is no limit.
    //
    // Returns:  Nothing.
    //
    void SetMaxLearningTableEntries(unsigned int maxLearningTableEntries);

    // Returns the number of learning table entries that will be allowed.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The maximum number of learning table entries.
    //
    inline unsigned int GetMaxLearningTableEntries(void) const;

    // This allows some control entity to force me to add an association
    // between a MAC address and a HalIf object.  This allows the learning
    // table to be populated before any packets are sent/received.
    //
    // Note that this may also be called by TransmitPacket() when the HalIf
    // instance sends a packet to me.
    //
    // Parameters:
    //      address - the MAC address that is to be added.
    //      pHalIf - pointer to the HalIf object that is to be added.
    //      Static - if true, then this MAC address will never be re-associated
    //               from pHalIf, even if a packet with this source address
    //               arrives via another interface. (PR 7840)
    //
    // Returns:
    //      true if the pair was added successfully.
    //      false if there was a problem (invalid HalIf, learning table full,
    //          etc).
    //
    virtual bool Add(const BcmMacAddress &address, BcmHalIf *pHalIf, bool Static = false);

    // Allows some control entity to add an association between a MAC address,
    // and a HalIf object, but using the index to the HalIf instance rather
    // than the address.   The default HalIf (if there is one) is index 0, and
    // the others are increasing from there (they actually match what shows up
    // when you call PrintHalIfs).
    //
    // Parameters:
    //      address - the MAC address that is to be added.
    //      index - the index the HalIf object that is to be added.
    //      Static - if true, then this MAC address will never be re-associated
    //               from pHalIf, even if a packet with this source address
    //               arrives via another interface. (PR 7840)
    //
    // Returns:
    //      true if the pair was added successfully.
    //      false if there was a problem (invalid HalIf, learning table full,
    //          etc).
    //
    bool Add(const BcmMacAddress &address, unsigned int index, bool Static = false);

    // Adds the specified MAC address, but without an association with a HalIf
    // object.  This is useful under some provisioning cases (via config file)
    // where the MAC address is known, but the HalIf is not.  In this case,
    // the association will be learned once the CPE device sends a packet.
    // Until then, packets sent to the unassociated MAC address will be sent
    // to all HalIf objects (like a broadcast packet).
    //
    // Parameters:
    //      address - the MAC address that is to be added.
    //
    // Returns:
    //      true if the address was added successfully.
    //      false if there was a problem (learning table full, etc).
    //
    bool AddUnassociated(const BcmMacAddress &address);

    // This method searches the learning table for the MAC address specified
    // and, if found, returns the HalIf that is associated with it.  If not
    // found, then it returns NULL.
    //
    // Parameters:
    //      address - the MAC address to look for.
    //
    // Returns:
    //      A pointer to the HalIf, or NULL.
    //
    BcmHalIf *Find(const BcmMacAddress &address);
    
    // Accessor which indicates whether a static bridging association exists
    // for a given MAC address.  Note that this method can't be const because
    // it calls Find, which is non-const.
    //
    // Parameters:
    //      address - the MAC address in question
    //
    // Returns:
    //      true if the routing association for this MAC address is static,
    //      false if the association is dynamic or if no association exists.
    //
    bool IsStatic (const BcmMacAddress &address);
    
    // Method to assign a static bridging association for the given address.
    // 
    // Parameters:
    //      address - the MAC address to make static
    //
    // Returns:
    //      true if an association is found
    //      false if no association is found
    //
    bool MakeStatic (const BcmMacAddress &address);

    // This allows some control entity to force me to remove the specified MAC
    // address from my learning table, deleting the association between it and
    // the HalIf.
    //
    // Note that this doesn't de-register the HalIf instance.
    //
    // Parameters:
    //      address - the MAC address that is to be removed.
    //
    // Returns:
    //      true if successful.
    //      false if the specified MAC address was not found.
    //
    bool Remove(const BcmMacAddress &address);

    // Same as above, but allows you to specify the learning table entry based
    // on its index.
    //
    // Parameters:
    //      index - the index of the table entry to be removed.
    //
    // Returns:
    //      true if successful.
    //      false if the specified MAC address was not found.
    //
    bool Remove(unsigned int index);
    
    // Method to remove a static bridging association for the given address.
    // 
    // Parameters:
    //      address - the MAC address to remove the static association for
    //
    // Returns:
    //      true if an association is found
    //      false if no association is found
    //
    bool RemoveStatic (const BcmMacAddress &address);
    
    // This allows some control entity to force me to clear out my learning
    // table.
    //
    // Note that this doesn't de-register any of the HalIf instances, it just
    // clears the learning table elements.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void ClearLearningTable(void);

    // This method prints the contents of the learning table to the specified
    // ostream.
    //
    // Parameters:
    //      outStream - the ostream to be used.
    //
    // Returns:
    //      A reference to the ostream.
    //
    ostream &PrintLearningTable(ostream &outStream);

    // This method prints the registered HalIf classes to the specified ostream.
    //
    // Parameters:
    //      outStream - the ostream to be used.
    //      printSettings - set to true if you want detailed settings to be
    //                      printed, of false for just the description.
    //
    // Returns:
    //      A reference to the ostream.
    //
    ostream &PrintHalIfs(ostream &outStream, bool printSettings = false);

public:
    // Data handling routines.

    // This is called, generally by a HalIf instance, to send a packet to one or
    // more of the other interfaces.  It assumes that the packet has already
    // been processed/filtered/etc.
    //
    // I will create an association in my learning table between the MAC SA in
    // this packet and the HalIf instance that sent it to me.  If my learning
    // table is full, then I will not send the packet (because I would not be
    // able to bridge any packets back to the device; they would always go to
    // the default HalIf).
    //
    // Broadcast packets will be sent to all registered HalIf instances (except
    // the one sending the packet to me).  Unicast packets will only be sent
    // to the HalIf associated in the learning table with the MAC DA in the
    // packet.
    //
    // Note that the packet also will not be sent if the destination HalIf
    // instance is the same as the one sending the packet to me.  This would
    // cause packet replication.
    //
    // If this succeeds, then the packet will be freed by the HalIf object(s)
    // to which it was sent.  If it fails, then the sending HalIf must free the
    // packet.
    //
    // It is highly desirable for the HalIf that is sending the packet to be
    // registered with me so that I can manage it, and so that I can send
    // broadcast packets to it.  Note, however, that this is not necessary for
    // unicast packets.
    //
    // Parameters:
    //      pPacket - pointer to the packet to send.
    //      pSourceHalIf - pointer to the HalIf from which this packet is being
    //                     sent.  This is used to keep the packet from being
    //                     sent back to that interface.  This must NOT be NULL!
    //
    // Returns:
    //      true if the packet was sent to at least one other interface.
    //      false if the packet was not sent on any interface (pSourceHalIf
    //          is NULL, there are no registered interfaces, etc).
    //
    virtual bool TransmitPacket(BcmEthernetPacket *pPacket, 
                                BcmHalIf *pSourceHalIf) = 0;

    // This method handles forwarding of broadcast packets which have a Mask (CM Interface Mask)
    // assocaited with it .  It sends the packet to every registered interface
    // anded with the Mask mask.
    //
    // Parameters:
    //      Mask    - Mask associated with the broadcast packet 
    //      pPacket - pointer to the packet to send.
    //      pSourceHalIf - pointer to the HalIf from which this packet is being
    //                     sent.  This is used to keep the packet from being
    //                     sent back to that interface.  This must NOT be NULL!
    //
    // Returns:
    //      true if the packet was sent to at least one other interface.
    //      false if the packet was not sent on any interface (pSourceHalIf
    //          is NULL, there are no registered interfaces, etc).
    //
    bool ForwardBroadcastPacketWithMask (uint32 Mask,  BcmEthernetPacket *pPacket,
                                              BcmHalIf *pSourceHalIf);


    // This method handles sending packets to the default Halif. If there is
    // an error or no default Halif exists, then this method will issue an
    // the error message  
    //
    // Parameters:
    //      Mask    - Mask for where packet can be sent 
    //      pPacket - pointer to the packet to send.
    //      pSourceHalIf - pointer to the HalIf from which this packet is being
    //                     sent.  
    //
    // Returns:
    //      true if the packet was sent to the default halif
    //      false if the packet was not sent to the default halif
    //
    bool ForwardBroadcastToDefaultHalIfWithMask(uint32 Mask, BcmEthernetPacket *pPacket,
                                                      BcmHalIf *pSourceHalIf);

    // This method returns the interface mask which matches the HalIf passed to this method
    //
    // Parameters:
    //      pHalIf - pointer to the HalIf which is searched for an interface mask
    //
    // Returns:
    //      the interface mask (zero if interface not found)
    //
    uint32  SearchInterfaces(BcmHalIf *pHalIf);

    // This method returns the HalIf which matches the interface mask passed to this method
    //
    // Parameters:
    //      the interface mask (one 1 bit should be set)
    //
    // Returns:
    //      pHalIf - pointer to the HalIf which is searched for an interface mask// 
    //
    BcmHalIf * SearchInterfaces(uint32 mask) const;

public:
    // Accessors for counters.

    // Returns the counter for the number of failed attempts to add an entry
    // to the learning table (because the table is full).
    //
    // Parameters:  None.
    //
    // Returns:
    //      The counter.
    //
    inline unsigned long GetFailedLearningAttempts(void) const;

protected:

    // This is the same as the public version of this method, except that it
    // doesn't lock the mutex.  This is meant to be called by methods of this
    // class which have already locked the mutex.
    //
    // Parameters:
    //      address - the MAC address that is to be added.
    //      pHalIf - pointer to the HalIf object that is to be added.
    //      Static - if true, then this MAC address will never be re-associated
    //               from pHalIf, even if a packet with this source address
    //               arrives via another interface. (PR 7840)
    //
    // Returns:
    //      true if the pair was added successfully.
    //      false if there was a problem (invalid HalIf, learning table full,
    //          etc).
    //
    bool AddImpl(const BcmMacAddress &address, BcmHalIf *pHalIf, bool Static);

    // This method handles the common implementation details for removing a
    // MAC DA/HalIf association from the learning table.  It assumes that the
    // caller has already locked the mutex, and that the learning table iterator
    // has been set to refer to the item to be removed.
    //
    // After returning from this method, the iterator will refer to the next
    // item in the map, regardless of whether or not the item was actually
    // removed.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the item was removed successfully.
    //      false if there was a problem.
    //
    bool RemoveImpl(BcmLearningTable::Iterator &LearningTableIterator);

    // Helper method to search the learning table for the entry associated
    // with the specified MAC address.  Uses the iterator object in the learning
    // table to perform the search and store the result.
    //
    // Parameters:
    //      address - the MAC address to be found.
    //
    // Returns:  Nothing.
    //
    void FindImpl(const BcmMacAddress &address, BcmLearningTable::Iterator &LearningTableIterator);

protected:

    #if (PACKETSWITCH_USE_MUTEX)
    
    // Protects access to the map of HalIf objects and other state info.  This
    // is necessary because multiple HALs (each with its own ISR/thread) can be
    // trying to send packets at the same time.
    BcmMutexSemaphore *pfMutex;
    
    #endif
    
    // The set of associations between MAC addresses and HalIf objects.  This is
    // filled by calls to Add().
    BcmLearningTable *pfLearningTable;

    // This limits the maximum number of associations that will be allowed in
    // the learning table.
    unsigned int fMaxLearningTableEntries;

    // This is the set of HalIf objects that have been registered with me.
    vector<BcmHalIf *> fHalInterfaces;

    // This is the "default" HalIf that gets the packet if none of the others
    // took it.
    BcmHalIf *pfDefaultHalIf;

    // Bitmask used to assign interface bits to each HalIf; this used to be
    // done dynamically, but that caused too many problems.  Now we keep track
    // of the bits statically so that each interface is assigned a bit once,
    // and it never changes after that.
    uint32 fHalIfMaskBits;

    // Controls message logging output.
    BcmMessageLogSettings fMessageLogSettings;

    // Counters and statistics.
    unsigned long fNumberOfFailedLearningAttempts;

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmPacketSwitch(const BcmPacketSwitch &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmPacketSwitch & operator = (const BcmPacketSwitch &otherInstance);

};


//********************** Inline Method Implementations ***********************


// Accessor for the instance name given in the constructor.
//
// Parameters:  None.
//
// Returns:
//      The name of the object.
//
inline const char *BcmPacketSwitch::Name(void) const
{
    return fMessageLogSettings.GetInstanceName();
}


// Returns the default HalIf, if one was registered.
//
// Parameters:  None.
//
// Returns:
//      The default (WAN) HalIf, or NULL if there isn't one.
//
inline BcmHalIf *BcmPacketSwitch::DefaultHalIf(void) const
{
    return pfDefaultHalIf;
}


// Returns the number of learning table entries that will be allowed.
//
// Parameters:  None.
//
// Returns:
//      The maximum number of learning table entries.
//
inline unsigned int BcmPacketSwitch::GetMaxLearningTableEntries(void) const
{
    return fMaxLearningTableEntries;
}


// Returns the counter for the number of failed attempts to add an entry
// to the learning table (because the table is full).
//
// Parameters:  None.
//
// Returns:
//      The counter.
//
inline unsigned long BcmPacketSwitch::GetFailedLearningAttempts(void) const
{
    return fNumberOfFailedLearningAttempts;
}

#endif


