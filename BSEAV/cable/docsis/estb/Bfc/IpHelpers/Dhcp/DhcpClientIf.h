//****************************************************************************
//
// Copyright (c) 2002-2010 Broadcom Corporation
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
//  Filename:       DhcpClientIf.h
//  Author:         David Pullen
//  Creation Date:  June 10, 2002
//
//****************************************************************************

#ifndef DHCPCLIENTIF_H
#define DHCPCLIENTIF_H

//********************** Include Files ***************************************

#include "DhcpClientId.h"

#include "IpAddress.h"
#include "MacAddress.h"

#include "MessageLog.h"

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iostream>
#include <vector>
#else
#include <iostream.h>
#include <vector.h>
#endif

//********************** Global Types ****************************************
typedef vector< BcmIpAddress > IpAddressVector;
typedef IpAddressVector::iterator IpAddressVectorIt;
typedef IpAddressVector::const_iterator IpAddressVectorConstIt;

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmDhcpClientLease;
class BcmDhcpLeaseSettings;

class BcmBaseIpHalIf;
class BcmDhcpClientThread;

class BcmDhcpClientLeaseContainer;

class BcmEthernetPacket;
class BcmDhcpPacket;

class BcmMutexSemaphore;

//********************** Class Declaration ***********************************


/** \ingroup DhcpClientServer
*
*   This is the primary interface between a client object/thread and the
*   DHCP Client Thread.  This object is associated with an IP stack, and
*   manages all of the leases for that stack.  In many cases, there will
*   only be a single lease for each stack.
*
*   There is a single DHCP Client thread in the system, but there can be
*   multiple IP stacks, and there can even be multiple leases for each
*   stack.
*
*   Note that this is the object that actually sends and processes packets,
*   and that services the state diagrams of each lease.  The object will be
*   running in the context of the DHCP client thread when this happens.
*/
class BcmDhcpClientIf
{
public:

    /// Initializing Constructor.  Stores the IP HalIf that I am being
    /// associated with.
    ///
    /// \note  The constructor will automatically create a lease object with
    ///        the client id = IP stack MAC address.  This lease will be set
    ///        so that it will automatically configure the IP stack when a
    ///        lease is acquired.  This will generally be the lease that most
    ///        clients will use when calling this object.
    ///
    /// \param
    ///      ipHalIf - the IP stack that I am being associated with.
    ///
    BcmDhcpClientIf(BcmBaseIpHalIf &ipHalIf);

    //// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    ~BcmDhcpClientIf();

    /// Sets the DHCP client thread object that I need to use.  This is
    /// generally only called at startup.  Setting this on-the-fly, particularly
    /// while a lease is in progress and timers are running, will have bad
    /// consequences.
    ///
    /// \param
    ///      pThread - the thread to use.
    ///
    void DhcpClientThread(BcmDhcpClientThread *pThread);
    
    /// Gets the DHCP client thread object that I am using.
    ///
    /// \return
    ///      The thread I am using.
    ///
    inline BcmDhcpClientThread *DhcpClientThread(void);

    /// Accessor for the IP HalIf that I am registered with.
    ///
    /// \return
    ///      A pointer to the IP HalIf that I am registered with.
    ///
    inline BcmBaseIpHalIf *IpHalIf(void);

    /// Does formatted output of the object to the specified ostream.  It
    /// mostly just prints the client ids for the list of leases I am
    /// managing.
    ///
    /// \param
    ///      outputStream - the ostream to be used.
    ///
    /// \return
    ///      The reference to the ostream that was passed in.
    ///
    ostream & Print(ostream &outputStream) const;

    /// Turns debugging on or off.  When on, excessive detail about DHCP lease
    /// operations will be printed to the message logger.
    ///
    /// \param
    ///      enable - true to enable debugging, false to disable.
    ///
    void Debug(bool enable);

public:  // Lease management methods.

    /// Creates a new lease object with the specified client id.  You will use
    /// this client id when using other methods relating to the lease.
    ///
    /// This method returns a pointer to the lease that was created (based on
    /// the assumption that you will want to configure it before doing anything
    /// else).  If the lease already exists, then this method just returns a
    /// pointer to it.
    ///
    /// \note  You MUST NOT delete the pointer returned.  The object is owned
    ///        by this class, and will be deleted by this class.  If you delete
    ///        it, the system will crash!
    ///
    /// \note  You MUST NOT change the client id option (option 61) in the
    ///        DhcpLeaseSettings objects.  These are set when the lease is
    ///        created, and must remain constant in order for the objects
    ///        to be tracked correctly (the client id is stored elsewhere
    ///        and would not be changed to match the new id).
    ///
    /// \param
    ///      clientId - the client id to be used for this lease.
    ///
    /// \return
    ///      A pointer to the lease that was created if successful.  Returns
    ///      NULL if the lease was not created (failed to lock the mutex).
    ///
    BcmDhcpClientLease *NewLease(const BcmDhcpClientId &clientId);

    /// Destroys the lease object with the specified client id.  The lease must
    /// be in the initial state for this to work; if the lease is bound or
    /// negotiations are in progress, then you must call ReleaseLease() before
    /// calling this.
    ///
    /// \param
    ///      clientId - the client id of the lease to be destroyed.
    ///
    /// \retval
    ///      true if the lease was destroyed.
    /// \retval
    ///      false if the lease was not found, or was not in initial state.
    ///
    bool DestroyLease(const BcmDhcpClientId &clientId);

    /// Returns a pointer to the lease with the specified client id.  You will
    /// use this to configure the client request parameters or to inspect the
    /// server response settings.
    ///
    /// \note  You MUST NOT delete the pointer returned.  The object is owned
    ///        by this class, and will be deleted by this class.  If you delete
    ///        it, the system will crash!
    ///
    /// \note  You MUST NOT change the client id option (option 61) in the
    ///        DhcpLeaseSettings objects.  These are set when the lease is
    ///        created, and must remain constant in order for the objects
    ///        to be tracked correctly (the client id is stored elsewhere
    ///        and would not be changed to match the new id).
    ///
    /// \param
    ///      clientId - the client id to search for.
    ///
    /// \return
    ///      A pointer to the lease, or NULL if one with that client id was not
    ///      found.
    ///
    BcmDhcpClientLease *FindLease(const BcmDhcpClientId &clientId);

public:  // DHCP Client-Server control methods.

    /// Causes the DHCP client thread to acquire the lease specified by the
    /// client id.  You must configure the client request settings in the lease
    /// prior to calling this.
    ///
    /// The process starts as a cold start, going through the full Discover,
    /// Offer, Request, Ack process.
    ///
    /// This method operates asynchronously; you will be notified of events
    /// related to the lease if you subscribed for notification.
    ///
    /// \param
    ///      clientId - the client id of the lease to start acquisition.
    /// \param
    ///      shortDelay - shortens the delay before we send our Discover packet.
    ///                   RFC2131 strongly recommends a delay between 1 and 10
    ///                   seconds to minimize collisions during a massive power
    ///                   failure.  However, in some environments, startup is
    ///                   sufficiently randomized that the delay can be
    ///                   shortened.
    ///
    /// \retval
    ///      true if lease acquisition has started.
    /// \retval
    ///      false if lease acquisition could not be started (lease doesn't
    ///          exist, lease not in initial state, etc).
    ///
    bool AcquireLease(const BcmDhcpClientId &clientId, bool shortDelay = false);

    /// Cause the DHCP client thread to renew the lease specified by the client
    /// id prior to the expiration of the T1/T2 timers.  Normally this is done
    /// for you when the appropriate timers expire, but in some cases it is
    /// desirable to start a renewal manually.
    ///
    /// This method operates asynchronously; you will be notified of events
    /// related to the lease if you subscribed for notification.
    ///
    /// \param
    ///      clientId - the client id of the lease to renew.
    ///
    /// \retval
    ///      true if the lease renewal has started.
    /// \retval
    ///      false if the lease renewal could not be started (lease doesn't
    ///          exist, lease not bound, etc).
    ///
    bool RenewLease(const BcmDhcpClientId &clientId);

    // Do we need this?  Not at this point...
    //bool InformLease(const BcmDhcpClientId &clientId);

    /// Causes the DHCP client thread to release the lease specified by the
    /// client id.  This will also abort any acquisition or renewal that is in
    /// progress.  A Release packet is sent for a non-static lease if it is in
    /// the Bound, Renewing, or Rebinding state, otherwise, it just causes the
    /// state to be reverted to initial state.
    ///
    /// Due to overwhelming request, this has been made synchronous, meaning
    /// that the caller will be blocked until the lease is released and the
    /// work is all done.  You will, however, still be notified of events
    /// related to the lease if you subscribed for notification.
    ///
    /// \param
    ///      clientId - the client id of the lease to release.
    ///
    /// \retval
    ///      true if the lease was released.
    /// \retval
    ///      false if the lease could not be released (lease doesn't exist, etc).
    ///
    bool ReleaseLease(const BcmDhcpClientId &clientId);

    /// Causes the DHCP client thread to release ALL the leases for this
    /// interface.  This will also abort any acquisition or renewal that is in
    /// progress.  A Release packet is sent for a non-static lease if it is in
    /// the Bound, Renewing, or Rebinding state, otherwise, it just causes the
    /// state to be reverted to initial state.
    ///
    /// Due to overwhelming request, this has been made synchronous, meaning
    /// that the caller will be blocked until the leases are all released and
    /// the work is all done.  You will, however, still be notified of events
    /// related to the leases if you subscribed for notification.
    ///
    /// \note  You should only call this if you're sure that all of the leases
    ///        for this IP stack belong to you.  It's pretty rude to shut down
    ///        somebody else's lease.
    ///
    void ReleaseAllLeases(void);

    /// This method supports static provisioning of parameters for the stack.
    /// In some cases, you want to force the stack to use a particular IP
    /// address (and other associated settings).
    ///
    /// To support this, the client will need to configure the server response
    /// settings rather than the client request settings.  The server settings
    /// are stored to the IP stack just like they normally would be after we
    /// receive an Ack packet.  All clients will be notified that the IP stack
    /// is bound if they subscribed for the event.
    ///
    /// \param
    ///      clientId - the client id of the lease to statically configure.
    ///
    /// \retval
    ///      true if the lease was statically provisioned (bound).
    /// \retval
    ///      false if the lease could not be bound (invalid settings, etc).
    ///
    bool ConfigureStaticLease(const BcmDhcpClientId &clientId);

	// Cause the DHCP client thread to move to kStateInitReboot state.
	//
	// Parameters:
	//      clientId - the client id of the lease to renew.
	//
	// Returns:
	//      true if the lease reboot has started.
	//      false if the lease reboot could not be started (lease doesn't
	//          exist, lease not bound, etc).
	//
	bool Reboot(const BcmDhcpClientId &clientId);

public:

    /// Accessor for the number of leases that are currently held by this
    /// object.
    ///
    /// \return
    ///      The number of leases.
    ///
    unsigned int NumberOfLeases(void) const;

    /// Accessor for the number of active/bound leases.  Some leases may have
    /// expired, or may have been created but not acquired.  The active leases
    /// are actively being maintained by the DHCP client thread.
    ///
    /// \return
    ///      The number of active leases.
    ///
    unsigned int NumberOfActiveLeases(void) const;

    /// Checks to see if the specified IP address is in the list of active
    /// leases on this interface.
    ///
    /// \param
    ///     ipAddress - address to search 
    ///
    /// \retval
    ///     true if the IP address is active on this interface
    /// \retval
    ///     false if the IP address is not active on this interface
    ///
    bool IsIpAddressActive(const BcmIpAddress &ipAddress) const;

    /// Checks to see if the specified IP address is in the list of bound
    /// leases on this interface.
    ///
    /// \param
    ///     ipAddress - address to search 
    ///
    /// \retval
    ///     true if the IP address is bound on this interface
    /// \retval
    ///     false if the IP address is not bound on this interface
    ///
    bool IsIpAddressBound(const BcmIpAddress &ipAddress) const;

    /// Find all bound (a.k.a. usable) IP addresses and copy to specified 
    /// destination IpAddressVector object.
    ///
    /// \param
    ///     destIpAddrVec - each active IP address found is added to the end 
    ///         of this container.
    ///
    /// \return
    ///      The number of active IP addresses found.
    ///
    unsigned int GetAllBoundIpAddresses( IpAddressVector& destIpAddrVec );

    /// Returns a pointer to the first lease that this object is managing.  This
    /// can be used to iterate over all of the leases.
    ///
    /// \return
    ///      A pointer to the first lease, or NULL if there are none.
    ///
    BcmDhcpClientLease *FirstLease(void);

    /// Returns the next lease following the one specified.
    ///
    /// \param
    ///      pCurrentLease - the starting point for the search.
    ///
    /// \return
    ///      The next lease that is being controlled by this object, or NULL if
    ///      there are none.
    ///
    BcmDhcpClientLease *NextLease(BcmDhcpClientLease *pCurrentLease);

    /// Returns a pointer to the lease at the 0-based index.  This is mostly for
    /// debugging purposes.  The index is displayed by the Print() method.  Note
    /// that the index of the lease can change when a new one is created.
    ///
    /// \param
    ///      index - the 0-based index of the lease to return.
    ///
    /// \return
    ///      The lease at the specified index, or NULL if there isn't one.
    ///
    BcmDhcpClientLease *FindLease(unsigned int index);

public:  // Methods called ONLY by the DHCP Client thread.  These methods should
         // not be called by any other thread or object.

    /// Notifies the client interface that some number of seconds has elapsed.
    /// This updates the object's notion of the current time.  Also gives the 
    /// object a chance to service any timers or other events related to the 
    /// leases.
    ///
    /// \note  This method should ONLY be called by the DHCP Client thread.
    ///
    /// \param
    ///      seconds - the number of seconds that have elapsed since the last
    ///                call.
    ///
    void ElapsedTimeSec(unsigned int seconds);

    /// Lets the client interface process the specified packet.  I will check my
    /// leases to see if it's meaningful and will modify them accordingly.
    ///
    /// The packet must be released by the caller after this method returns.
    ///
    /// \note  This method should ONLY be called by the DHCP Client thread.
    ///
    /// \param
    ///      pPacket - pointer to the ethernet packet to be processed.
    ///
    void ProcessPacket(BcmEthernetPacket *pPacket);

    /// Lets the client interface process the ARP reply info that was received.
    /// I will check my leases and take appropriate action.
    ///
    /// \note  This method should ONLY be called by the DHCP Client thread.
    ///
    /// \param
    ///      ipAddress - the IP address that was responded to.
    /// \param
    ///      macAddress - the MAC address of the device that responded.
    ///
    void ProcessArpReply(const BcmIpAddress &ipAddress,
                         const BcmMacAddress &macAddress);

private:

    /// Helper method to create and configure a DHCP settings object that will
    /// be sent in a Discover packet.  It uses the context information in the
    /// lease to determine which fields should be filled in (and with what
    /// values).
    ///
    /// This method handles max try checking, and also sets up the retry/timeout
    /// parameters for waiting for offers.  The lease state must be InitDelay on
    /// entry, and will be Selecting on exit.
    ///
    /// This method calls SendDhcpPacket() to do the packet creation and xmit.
    ///
    /// \param
    ///      pLease - the lease object for which the Discover packet is being
    ///               sent.
    ///
    void SendDiscover(BcmDhcpClientLease *pLease);

    /// Helper method to create and configure a DHCP settings object that will
    /// be sent in a Request packet.  It uses the context information in the
    /// lease to determine which fields should be filled in (and with what
    /// values).
    ///
    /// This method handles max try checking, and also sets up the retry/timeout
    /// parameters for waiting for the ack.  The lease state must be Requesting,
    /// Renewing, or Rebinding on entry, and will not change.
    ///
    /// This method calls SendDhcpPacket() to do the packet creation and xmit.
    ///
    /// \param
    ///      pLease - the lease object for which the Request packet is being
    ///               sent.
    ///
    void SendRequest(BcmDhcpClientLease *pLease);

    /// Helper method to create and configure a DHCP settings object that will
    /// be sent in a Decline packet.  It uses context information in the lease
    /// to determine which fields should be filled in (and with what values).
    ///
    /// This method calls SendDhcpPacket() to do the packet creation and xmit.
    ///
    /// \param
    ///      pLease - the lease object for which the Decline packet is being
    ///               sent.
    ///
    void SendDecline(BcmDhcpClientLease *pLease);
    
    /// Helper method to create and configure a DHCP settings object that will
    /// be sent in a Release packet.  It uses context information in the lease
    /// to determine which fields should be filled in (and with what values).
    ///
    /// This method calls SendDhcpPacket() to do the packet creation and xmit.
    ///
    /// \param
    ///      pLease - the lease object for which the Release packet is being
    ///               sent.
    ///
    void SendRelease(BcmDhcpClientLease *pLease);

    /// Helper method to create, configure and send DHCP packets to the network
    /// (per RFC2131).  It uses the context information in the lease and
    /// settings objects to determine which fields should be filled in (and with
    /// what values).
    ///
    /// The MAC addresses are pulled from the pfIpHalIf object.
    ///
    /// \param
    ///      pLease - the lease object for which this packet is being sent.
    /// \param
    ///      pSettings - the DHCP settings object that will determine the
    ///                  majority of the payload.
    ///
    /// \retval
    ///      true if the packet was built and sent.
    /// \retval
    ///      false if we couldn't allocate a buffer for failed to send the
    ///          packet.
    ///
#ifdef BCM_STANDALONE_DHCP_CLIENT
    virtual bool SendDhcpPacket(BcmDhcpClientLease *pLease, 
                        BcmDhcpLeaseSettings *pSettings);
#else
    bool SendDhcpPacket(BcmDhcpClientLease *pLease, 
                        BcmDhcpLeaseSettings *pSettings);
#endif 

    /// Helper method to create, configure and send ARP Request and Reply
    /// packets to the network (per RFC2131).  It uses the ARP type specified
    /// to determine which fields should be filled in (and with what values).
    ///
    /// This should be called when we are selecting/storing an Offer (sending
    /// an ARP request to see if anyone else is using the address), and when
    /// we first bind to an address after receiving the first Ack (sending an
    /// ARP reply to let everyone know that we are using the address).
    ///
    /// The MAC addresses are pulled from the pfIpHalIf object.
    ///
    /// \param
    ///      arpType - this should be either BcmEthernetPacket::kArpRequest or
    ///                BcmEthernetPacket::kArpReply.
    /// \param
    ///      ipAddress - the client's IP address.  This will be the yiaddr
    ///                  field from the Offer or Ack packet.
    ///
    /// \retval
    ///      true if the packet was built and sent.
    /// \retval
    ///      false if we couldn't allocate a buffer for failed to send the
    ///          packet.
    ///
#ifdef BCM_STANDALONE_DHCP_CLIENT
    virtual bool SendArpPacket(uint16 arpType, const BcmIpAddress &ipAddress);
#else
    bool SendArpPacket(uint16 arpType, const BcmIpAddress &ipAddress);
#endif

    /// Helper method to query the parameter request list from the IP stack and
    /// to merge it with the one from the client (eliminating duplicates).
    ///
    /// \param
    ///      pIpHalIf - the IP stack to query.
    /// \param
    ///      pSettings - the settings object that has the client's parameter
    ///                  request list.
    ///
    void QueryAndMergeParamRequestList(BcmBaseIpHalIf *pIpHalIf,
                                       BcmDhcpLeaseSettings *pSettings);

private:

    /// The DHCP client thread; this is who I call to get operations on leases
    /// performed.
    BcmDhcpClientThread *pfDhcpClientThread;

    /// The IP stack for whom I am managing leases.
    BcmBaseIpHalIf *pfIpHalIf;

    /// STL container that holds the leases (probably an STL Map).
    BcmDhcpClientLeaseContainer *pfLeaseContainer;

    /// Used for debugging and debug output control.
    BcmMessageLogSettings fMessageLogSettings;

    /// Protects against re-entrancy.
    mutable BcmMutexSemaphore *pfMutex;

    /// These objects are used when parsing or sending packets.  They are
    /// temporary objects, but we don't want them coming and going all the time
    /// (because they allocate lots of memory and tend to fragment the heap).
#ifdef BCM_STANDALONE_DHCP_CLIENT
#else
    BcmDhcpPacket *pfDhcpPacket;
#endif
    BcmDhcpLeaseSettings *pfDhcpSettings;
    BcmOctetBuffer *pfParamRequestList;
    BcmOctetBuffer *pfIpStackParamRequestList;

#ifdef BCM_STANDALONE_DHCP_CLIENT
protected:
    BcmDhcpPacket *pfDhcpPacket;
	
#endif

private:

    /// Default Constructor.  Not supported.
    BcmDhcpClientIf(void);

    /// Copy Constructor.  Not supported.
    BcmDhcpClientIf(const BcmDhcpClientIf &otherInstance);

    /// Assignment operator.  Not supported.
    BcmDhcpClientIf & operator = (const BcmDhcpClientIf &otherInstance);

};


//********************** Inline Method Implementations ***********************


/// This allows you to do the following:
///
///    BcmDhcpClientIf clientIf;
///
///    cout << clientIf << endl;
///
inline ostream & operator << (ostream &outputStream, const BcmDhcpClientIf &clientIf)
{
    return clientIf.Print(outputStream);
}


inline BcmDhcpClientThread *BcmDhcpClientIf::DhcpClientThread(void)
{
    return pfDhcpClientThread;
}


inline BcmBaseIpHalIf *BcmDhcpClientIf::IpHalIf(void)
{
    return pfIpHalIf;
}


#endif

