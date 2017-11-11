/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
//
//****************************************************************************
//
//  Filename:       BaseIpHalIf.h
//  Author:         David Pullen
//  Creation Date:  April 22, 2002
//
//****************************************************************************
//  Description:
//      This is a specialization of a BcmNiHalIf that contains helper methods
//      and state that is common to many IP stack-specific derived classes
//      (pSOS, VxWorks, etc).
//
//      This file was added to support PR2256.
//
//****************************************************************************

#ifndef BASEIPHALIF_H
#define BASEIPHALIF_H

//********************** Include Files ***************************************

// My base class.
#include "NiHalIf.h"

#include "MacAddress.h"
#include "IpAddress.h"
#include "IpAddressList.h"

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <vector>
#else
#include <vector.h>
#endif

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************
#if (BFC_INCLUDE_DHCPV4_SUPPORT)
class BcmDhcpClientIf;
class BcmDhcpLeaseSettings;
#endif
class BcmEventPublisher;            // PR4517
class BcmCompletionHandlerACT;      // PR4517

//#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT)
class BcmNeighborDiscoveryThread;
class BcmNeighborDiscoveryACT;
//#endif
#if (BFC_INCLUDE_DHCPV6_SUPPORT)
class BcmDhcpV6ClientIf;
#endif
//********************** Class Declaration ***********************************


class BcmBaseIpHalIf : public BcmNiHalIf
{
public:

    /// Initializing Constructor.  Stores the information specified.
    ///
    /// \param
    ///      macAddress - the MAC address that is to be used for this IP stack.
    /// \param
    ///      pDescription - text string describing the HalIf, for debugging.
    ///
    BcmBaseIpHalIf(const BcmMacAddress &macAddress,
                   const char *pDescription = NULL);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmBaseIpHalIf();
    
    /// This method causes the HalIf class to register itself with the HAL,
    /// effectively loading the driver for the interface.
    ///
    /// \retval
    ///      true if the driver was registered and initialize successfully.
    /// \retval
    ///      false if there was a problem.
    ///
    virtual bool StartDriver(void);

    /// Gets the HAL-level ETHERNET_PACKET and DATA_BUFFER structures, along
    /// with the appropriate packet buffer, and fills in the fields of the
    /// specified BcmEthernetPacket object.
    ///
    /// The packet will be configured so that it is released back to this HalIf
    /// normally (as though the packet were announced by the IP stack).
    ///
    /// \param
    ///      pPacket - pointer to the BcmEthernetPacket object to configure.
    /// \param
    ///      numBytes - the PDU size to allocate.
    ///
    /// \retval
    ///      true if the packet was allocated and configured correctly.
    /// \retval
    ///      false if there was no memory to allocate.
    ///
    bool AllocatePacket(BcmEthernetPacket *pPacket, unsigned int numBytes);

public:     // PR4517 - added enum and methods.

    /// These are the events that I publish.
    typedef enum
    {
        /// This event is published when the first IP address is configured in
        /// to the stack.  Subsequent addresses do not cause this to be
        /// published.  You should be able to query the IP address that was
        /// configured via GetCurrentIpAddress().
        kEventIpAddressConfigured = 0,

        /// This event is published when the last IP address is removed.
        kEventIpAddressRemoved

    } IpStackEvent;

    /// Lets one or more clients sign up for notification of the events that I
    /// publish.
    ///
    /// \param
    ///      eventCode - the event code for the status event of interest.
    /// \param
    ///      pAct - pointer to the ACT that will be notified.
    ///
    void Subscribe(IpStackEvent eventCode, 
                   BcmCompletionHandlerACT *pAct);

    /// Lets the clients stop receiving notification of events that were
    /// previously subscribed to.
    ///
    /// \param
    ///      eventCode - the event code for the status event of interest.
    /// \param
    ///      pAct - pointer to the ACT that was previously subscribed.
    ///
    void Unsubscribe(IpStackEvent eventCode,
                     BcmCompletionHandlerACT *pAct);

public:

    /// Returns the physical address of the interface, if any.  Most of our
    /// interfaces don't have an address (promiscuous mode), so this returns
    /// all 0's.
    ///
    /// \param
    ///      interfaceNumber - the interface number being querried; this is
    ///                        primarily for subinterface support.
    ///
    /// \return
    ///      The physical address of the interface.
    ///
    virtual const BcmMacAddress &IfPhysAddress(int interfaceNumber = 0) const;
    
    /// Accessor for the ifPhyAddress value.
    /// 
    /// \param
    ///      pAgent - the SNMP agent that my interface is being querried for.
    /// \param
    ///      interfaceNumber - the interface number being querried; this is
    ///                        primarily for subinterface support.
    ///
    /// \return
    ///      The MAC address which is this object's ifPhysAddress
    ///
    virtual const BcmMacAddress &IfPhysAddress (const BcmSnmpAgent *pAgent = NULL, int interfaceNumber = 0) const
    {
        return fMyMacAddress;
    }
    
    /// An accessor for this HAL's IP stack number
    ///
    /// \param
    ///      none
    ///
    /// \return
    ///      The stack number of this HAL
    ///
    inline int StackNumber (void) const;

    /// A different accessor for the MacAddress associated with this IP stack.
    ///
    /// \return
    ///      The MAC address.
    ///
    inline const BcmMacAddress &MacAddress(void) const;
    inline void SetMacAddress( BcmMacAddress & macAdr );
    
public:

    /** \name Dual mode IPv4/IPv6 support methods
    *
    * These methods can reasonably support either/both IPv4 and IPv6 for
    * client who don't really care what mode we're in.
    *
    * \note It's not entirely clear what we should do if both modes are active.
    *       We need to discuss this and evaluate use-case models.
    */
    //@{

    /// Returns the IP address that is currently associated with the IP stack
    /// for this HalIf.  Note that the stack can have multiple IP addresses
    /// associated with it, in which case this returns the first one.
    ///
    /// \note UNFINISHED - what if the stack has both an IPv4 and IPv6 address?
    ///       Or multiple IPv6 addresses?  What should be returned?
    ///
    /// \param
    ///      ipAddress - the IP address will be returned in this parameter.
    ///
    /// \retval
    ///      true if the IP address was successfully querried.
    /// \retval
    ///      false if there was a problem acquiring the IP address.
    ///
    inline bool GetCurrentIpAddress(BcmIpAddress &ipAddress);

    /// Returns the first IP address configured for a network interface.
    /// GetCurrentIpAddress() implements the same concept, though the function
    /// name is less precise.
    ///
    /// \note UNFINISHED - what if the stack has both an IPv4 and IPv6 address?
    ///       Or multiple IPv6 addresses?  What should be returned?
    ///
    /// \return
    ///     The first IP address configured for the interface.
    ///
    inline const BcmIpAddress& PrimaryIpAddress() const;

    /// Checks to see if the specified IP address is in the list of bound
    /// leases on this interface.
    ///
    /// \note It seems like this one should work fine for either IPv4 or IPv6.
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
    /// destination vector< BcmIpAddress > object.
    ///
    /// \note The list can contain a combination of IPv4 and/or IPv6 addresses.
    ///
    /// \param
    ///     destIpAddrVec - each active IP address found is added to the end 
    ///         of this container.
    ///
    /// \return
    ///      The number of active IP addresses found.
    ///
    unsigned int GetAllBoundIpAddresses( vector< BcmIpAddress >& destIpAddrVec );
    unsigned int GetAllBoundIpV4Addresses( vector< BcmIpAddress >& destIpAddrVec );
    unsigned int GetAllBoundIpV6Addresses( vector< BcmIpAddress >& destIpAddrVec );

    /// Allows a client to query whether or not we have been configured with
    /// one or more IP addresses/DHCP leases.
    ///
    /// \retval
    ///      true if we are configured.
    /// \retval
    ///      false if we are not configured.
    ///
    inline bool IpAddressConfigured(void) const;

    /// Allows a client to set whether or not we have been configured with
    /// one or more IP addresses/DHCP leases.
    ///
    /// \param
    ///     configured - true if configured.
    ///
    inline void IpAddressConfigured(bool configured);
    
    /// Returns the default gateway associated with the supplied IP address.
    ///
    /// \return
    ///     true unless the Address is not bound to this interface.
    ///
    bool DefaultGateway (const BcmIpAddress &Address, BcmIpAddress &Gateway) const;

    /// This shuts down all leases/IP addresses that are configured with the
    /// stack, and removes all routers, cleaning up the routing table.
    ///
    /// \retval
    ///      true if the IP stack was shut down successfully.
    /// \retval
    ///      false if the IP stack was not shut down.
    ///
    bool ShutdownIpStack(void);

    //@}

public:

    /** \name IPv4 support methods
    *
    * These methods support only IPv4 operation for the IP stack.  They are
    * separate from the IPv6 support methods due to fundamental differences in
    * state and operation.
    *
    */
    //@{

    /// Returns the subnet mask that is currently associated with the IP stack
    /// for this HalIf.  
    ///
    /// \param
    ///      subnetMask - the subnet mask will be returned in this parameter.
    ///
    /// \retval
    ///      true if the subnet mask was successfully querried.
    /// \retval
    ///      false if there was a problem acquiring the subnet mask.
    ///
    inline bool GetCurrentSubnetMask(BcmIpAddress &subnetMask);

    /// Returns the subnet mask stored when the primary IP address of the
    /// network interface was configured.   GetCurrentSubnetMask() implements
    /// the same concept.
    ///
    /// \return
    ///     The subnet mask associated with the primary IPv4 address.
    ///
    inline const BcmIpAddress& SubnetMask() const;
    
    /// Returns the subnet mask associated with the supplied IP address.
    ///
    /// \return
    ///     true unless the Address argument is IPv6 or is not bound to this
    ///     interface
    ///
    bool SubnetMask(const BcmIpAddress &Address, BcmIpAddress &Subnet) const;

#if (BFC_INCLUDE_DHCPV4_SUPPORT)
    /// Returns the DHCP ClientIf object.  Clients should use this object to
    /// configure or acquire an IP address for this IP stack.
    ///
    /// \return
    ///      The DHCP ClientIf object.
    ///
    inline BcmDhcpClientIf *DhcpClientIf(void) const;

#endif
#if (BFC_INCLUDE_DHCPV6_SUPPORT)
    /// Returns the DHCPv6 ClientIf object.  Clients should use this object to
    /// configure or acquire an IPv6 address for this IP stack.
    ///
    /// \return
    ///      The DHCPv6 ClientIf object.
    ///
    inline BcmDhcpV6ClientIf *DhcpV6ClientIf(void) const;
#endif
    /// Searches the ARP table, looking for the IP address specified.  If found,
    /// it returns the MAC address.
    ///
    /// This needs to be implemented by the derived class; this class provides
    /// a dummy implementation for stacks that don't support it.
    ///
    /// \param
    ///      ipAddress - the IP address to search for.
    /// \param
    ///      macAddress - the MAC address corresponding to the IP address.
    ///
    /// \retval
    ///      true if the entry was found and the MAC address is valid.
    /// \retval
    ///      false if the entry was not found.
    ///
    virtual bool ArpTableSearch(const BcmIpAddress &ipAddress,
                                BcmMacAddress &macAddress) { return false; }

    /// This method adds the specified entry (IP address <--> MAC address) to
    /// the ARP table.  This is for entries that can't otherwise be learned
    /// automatically.  The entry will be added as a permanent entry, meaning
    /// that it won't age out, and won't be removed unless manually told to.
    ///
    /// This needs to be implemented by the derived class; this class provides
    /// a dummy implementation for stacks that don't support it.
    ///
    /// \param
    ///      ipAddress - the IP address to add to the ARP table.
    /// \param
    ///      macAddress - the MAC address to be associated with the IP address.
    ///
    /// \retval
    ///      true if the entry was added successfully.
    /// \retval
    ///      false if the entry was not added for some reason.
    ///
    virtual bool AddPermanentArpTableEntry(const BcmIpAddress &ipAddress,
                                           const BcmMacAddress &macAddress);

    /// This method removes the entry from the ARP table specified via the
    /// IP address.  This can be used to remove any entry from the ARP table,
    /// and it is the only way to remove entries added via the
    /// AddPermanentArpTableEntry() method.
    ///
    /// This needs to be implemented by the derived class; this class provides
    /// a dummy implementation for stacks that don't support it.
    ///
    /// \param
    ///      ipAddress - the IP address of the ARP table entry to remove.
    ///
    /// \retval
    ///      true if the entry was removed (or was not present).
    /// \retval
    ///      false if there was a problem.
    ///
    virtual bool RemoveArpTableEntry(const BcmIpAddress &ipAddress);

	/// Deletes the route for the given interface
	///
	/// \param
	///      ipAddress - the IP address of the router (will be returned in this parameter)
	/// \param
	///      interfaceName - the name of the given interface
	///
	/// \retval
	///      true if the router was successfully added
	/// \retval
	///      false if there was a problem adding the route
	///
	virtual bool DeleteRoute(BcmIpAddress &router, char *pInterfaceName);

	/// Adds the default route for the given interface
	/// Use 0,0,GATEWAY (router) for the default route
	///
	/// \param
	///      ipAddress - the IP address of the router (will be returned in this parameter)
	/// \param
	///      interfaceName - the name of the given interface
	///
	/// \retval
	///      true if the router was successfully added
	/// \retval
	///      false if there was a problem adding the route
	///
	virtual bool AddDefaultRoute(BcmIpAddress &router, char *pInterfaceName);
    
    //@}

public:
	//(BFC_INCLUDE_DHCPV6_SUPPORT)
    /** \name IPv6 support methods
    *
    * These methods support only IPv6 operation for the IP stack.  They are
    * separate from the IPv4 support methods due to fundamental differences in
    * state and operation.
    *
    */
    //@{

    /// Allows a new IPv6 address to be associated with the IP stack.  This
    /// address can be gotten via DHCPv6, stateless auto-config, manual
    /// configuration, etc.
    ///
    /// PR12451 - added support for passing the lifetime parameters to the
    /// stack.  The default value of -1 means "infinity".
    ///
    /// \param
    ///     ipAddress - the IPv6 address to be associated with the IP stack.
    /// \param
    ///     preferredLifetimeSecs - the preferred lifetime of the IP address
    ///                             (this comes from the IA_ADDR).
    /// \param
    ///     validLifetimeSecs - the valid lifetime of the IP address (this comes
    ///                         from the IA_ADDR).
    ///
    /// \retval
    ///     true if the address was associated successfully.
    /// \retval
    ///     false if the address was not associated successfully (invalid
    ///         value, IPv6 not supported by derived class, etc).
    ///
    virtual bool AddIpv6Address(const BcmIpV6Address &ipAddress,
                                uint32 preferredLifetimeSecs = (uint32) -1,
                                uint32 validLifetimeSecs = (uint32) -1);

    /// Allows the preferred/valid lifetime values for an existing IPv6 address
    /// updated.  This will generally be done when the DHCPv6 lease is renewed.
    ///
    /// Added for PR12451.
    ///
    /// \param
    ///     ipAddress - the IPv6 address to be associated with the IP stack.
    /// \param
    ///     preferredLifetimeSecs - the preferred lifetime of the IP address
    ///                             (this comes from the IA_ADDR).
    /// \param
    ///     validLifetimeSecs - the valid lifetime of the IP address (this comes
    ///                         from the IA_ADDR).
    ///
    /// \retval
    ///     true if the lifetime values were updated successfully.
    /// \retval
    ///     false if the lifetime values were not updated successfully (IPv6
    ///         address doesn't exist, IPv6 not supported by derived class,
    ///         etc).
    ///
    virtual bool UpdateIpv6Lifetimes(const BcmIpV6Address &ipAddress,
                                     uint32 preferredLifetimeSecs,
                                     uint32 validLifetimeSecs);

    /// Helper method that computes the link-local address from the well-known
    /// link-local prefix, then appends the interface token (i.e. the MAC
    /// address), and then calls AddIpv6Address().
    ///
    /// \retval
    ///     true if the address was added successfully.
    /// \retval
    ///     false if the address was not added successfully.
    ///
    bool AddIpv6LinkLocalAddress(void);
    inline bool Ipv6LinkLocalAddressConfigured(void) const;
    virtual void GetIpv6LinkLocalAddress(BcmIpV6Address &linkLocalAddress) const;

	inline const BcmIpV6Address& Ipv6LinkLocalAddress(void) const;
	inline void Ipv6LinkLocalAddress(BcmIpV6Address& linkLocal) ;

    /// Removes the specified IPv6 address from the IP stack.  This address
    /// must have been added by AddIpv6Address() previously.
    ///
    /// \param
    ///     ipAddress - the IPv6 address to remove from the IP stack.
    ///
    /// \retval
    ///     true if the address was removed successfully.
    /// \retval
    ///     false if the address was not removed successfully (invalid value,
    ///         address not found, IPv6 not supported by derived class, etc.).
    ///
    virtual bool RemoveIpv6Address(const BcmIpV6Address &ipAddress);

	virtual int NetworkRxSocket(void);
	virtual int CreateNetworkTxSocket(BcmIpV6Address &ipAddress, int port, bool ifaceonly, bool reuse);	
	virtual int CreateNetworkRxSocket(BcmIpV6Address &ipAddress, int port, bool ifaceonly, bool reuse);	
	virtual bool TransmitViaNetworkSocket( BcmOctetBuffer &buffer, const BcmIpAddress &ipAddress, uint16 port);
	virtual bool GetInterfaceFlags(uint32 *pFlags);
	virtual bool AddIpv6DnsServer(const BcmIpV6Address &ipAddress);
	virtual bool AddIpv6DomainName(const char* domain);
	virtual bool RemoveIpv6DnsServer(const BcmIpV6Address &ipAddress);	
	virtual bool RemoveIpv6DomainName(const char* domain);
	virtual bool IsIpV6AddressTentative(const BcmIpV6Address &ipAddress);

    ///
    /// Performs the address resolution described in RFC2461 sec. 5.2 
    /// First we determine if the desination is on-net or off-net
    /// by using a combination of the destination cache, prefix list, 
    /// and default router list.  This gives us the next-hop neighbor IP
    /// corresponding to the specified destination IP address.  
    /// If the next-hop is the same as the desitnation, the destination is
    /// on net.  It it is different, the next-hop is the router with the 
    /// longest prefix match.
    ///
    /// Once the next-hop IP is known, the the negihbor cache is checked for 
    /// for an entry.  If no cache entry exists, address resolution using 
    /// neighbor discovery will be performed. The client will be notified 
    /// asynchronously of the result through his ACT. 
    ///
    /// \param
    ///     pACT - Neighbor Discovery ACT 
    ///
    /// \return - bool
    ///     true - if the entry was resolved in cache, the pACT->DestinationMacAddress()
    ///            will contain the resolved address
    ///     false - entry was NOT resolved in cache, ND will be performed and 
    ///             will use ACT to callback to calling application.
    ///
	#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT)
    bool ResolveIpV6Address( BcmNeighborDiscoveryACT * pACT );
	#endif
    
    ///
    /// Performs duplicate address detection as described in RFC2461
    ///
    /// \param
    ///     pACT - Neighbor Discovery ACT 
    ///
	//#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT)
    virtual void PerformDuplicateAddressDetection( BcmNeighborDiscoveryACT * pACT );
	//#endif

    /// Returns the prefix associated with the supplied IP address.
    ///
    /// \return
    ///     the prefix length, unless the Address argument is IPv4 or is not 
    ///     bound to this interface - in which case -1 is returned.
    ///
    int IpV6Prefix ( const BcmIpAddress &Address, BcmIpAddress &Prefix ) const;

    /// 
    /// Registers a neighbor discovery thread with the IP interface.  This
    /// will make the interface IPv6 capable, and enables IP address 
    /// auto-configuration. 
    ///
    /// \param
    ///     pNeighborDisvoveryThread - Neighbor Discovery thread 
    ///
	#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT)
    void RegisterNeighborDiscoveryThread( BcmNeighborDiscoveryThread * pNeighborDisvoveryThread );
	#endif
    /// Accessor for the Neighbor Discovery Thread
    ///
    /// \retval
    ///     pointer to the Neighbor Discovery Thread
	#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT)
    inline BcmNeighborDiscoveryThread * NeighborDiscoveryThread() const;
	#endif    
    //@}

public:

    /** \name DHCPv4 support methods
    *
    * Methods used by the IPv4 version of the DHCP client to add/remove/change
    * addresses in the IP stack.
    *
    * \note These methods must only be called by the BcmDhcpClientIf object!
    *
    */
    //@{

    /// This is called by the DHCP Client thread so that the derived class can
    /// request certain options to be returned by the server; for example, if
    /// the stack cares about the MTU, then it can request option 26.
    ///
    /// The option codes are single byte values, as specified in RFC2132.  The
    /// derived class can append option codes to the buffer specified.
    ///
    /// The options will be passed in via the BcmDhcpLeaseSettings object at
    /// various points of the DHCP process.
    ///
    /// \param
    ///      buffer - the octet buffer into which the derived class can place
    ///               the DHCP option codes that it wants requested.
    ///
    virtual void DhcpParameterRequestList(BcmOctetBuffer &dhcpOptions);
#if (BFC_INCLUDE_DHCPV4_SUPPORT)
    /// Gives the derived class a chance to sniff the DHCP OFFER before the
    /// DHCP client thread requests a lease.  If the stack doesn't like
    /// something, then the offer will not be requested.  This method will be
    /// called once for each offer that is received.
    ///
    /// \param
    ///      dhcpLeaseSettings - the settings to sniff.
    ///
    /// \retval
    ///      true if the offer is acceptable.
    /// \retval
    ///      false if the offer is bogus and should be discarded.
    ///
    virtual bool DhcpOfferIsAcceptable(const BcmDhcpLeaseSettings &dhcpLeaseSettings) { return true; }

    /// Configures the IP stack based on the settings passed in.  These may have
    /// been gotten via DHCP, or they may have been manually configured.  Either
    /// way, the derived class needs to extract the relevant options and
    /// parameters, and configure the IP stack for network operation.
    ///
    /// If this is the first time this lease has been configured, the
    /// pCurrentSettings MUST be NULL.  We will just store whatever is in the
    /// pNewSettings parameter.
    ///
    /// If this is a lease renewal, the the pCurrentSettings MUST be the
    /// settings that were previously configured.  The pNewSettings will be
    /// the new settings that are to be configured.  We use the old and new
    /// settings to determine which of the interesting items in the lease has
    /// changed so that we can be selective in what we do.  This minimizes
    /// disturbances to the IP stack and any active sockets, etc.
    ///
    /// Note that you can call this with different leases, and each lease will
    /// cause the address to be added to the stack.  Then, you can renew each
    /// lease by passing in the current and new settings for the lease that has
    /// been renewed.
    ///
    /// \param
    ///      pCurrentSettings - the settings that are currently in use.
    /// \param
    ///      pNewSettings - the new settings to use.
    /// \param
    ///      configureRouter - tells whether or not the router should be
    ///                        configured.
    ///
    /// \retval
    ///      true if the IP stack was configured successfully.
    /// \retval
    ///      false if the IP stack was not configured.
    ///
    bool ConfigureLease(const BcmDhcpLeaseSettings *pCurrentSettings,
                        BcmDhcpLeaseSettings *pNewSettings,
                        bool configureRouter);

    /// This removes a lease that was previously configured, removing the IP
    /// address from the stack.  If this is the only address, then doing this
    /// shuts down the IP stack, reverting it back to some default state.  If
    /// this is one of many addresses, then the address is simply removed from
    /// the stack.
    ///
    /// \param
    ///      pSettings - the lease settings that are to be removed from the
    ///                  stack.
    /// \param
    ///      removeRouter - tells whether or not the router should be removed.
    ///
    /// \retval
    ///      true if the lease address was found and removed.
    /// \retval
    ///      false if the lease address was not found or could not be removed.
    ///
    bool RemoveLease(const BcmDhcpLeaseSettings *pSettings,
                     bool removeRouter);

    //@}
#endif

protected:
#if (BFC_INCLUDE_DHCPV4_SUPPORT)
    /** \name Internal DHCPv4 support methods.
    *
    * These methods are called by the similarly-named public methods that deal
    * with DHCPv4.  These give the derived class a chance to implement
    * OS-specific behavior.
    *
    * It appears that we decided to provide a default/dummy implementation in
    * the base class which does nothing, in order to avoid modifying derived
    * classes that don't need to do anything for these methods.
    *
    */
    //@{

    /// This is the method the derived class must implement when a new lease is
    /// being configured or an existing lease is being renewed.
    ///
    /// If the IP stack doesn't support multiple IP addresses associated with the
    /// stack, then it should return false rather than replacing the existing
    /// lease.
    ///
    /// This method is called by ConfigureLease().
    ///
    /// \param
    ///      pCurrentSettings - the settings that are currently in use.
    /// \param
    ///      pNewSettings - the new settings to use.
    /// \param
    ///      configureRouter - tells whether or not the router should be
    ///                        configured.
    ///
    /// \retval
    ///      true if the IP stack was configured successfully.
    /// \retval
    ///      false if the IP stack was not configured.
    ///
    virtual bool ConfigureLeaseImpl(const BcmDhcpLeaseSettings *pCurrentSettings,
                                    const BcmDhcpLeaseSettings *pNewSettings,
                                    bool configureRouter) { return false; }

    /// Values returned by RemoveLeaseImpl().
    typedef enum
    {
        kError,
        kNoMoreLeases,
        kMoreLeases
    } RemoveResult;

    /// This is the method the derived class must implement when an existing
    /// lease is being removed.
    ///
    /// This method is called by RemoveLease().
    ///
    /// \param
    ///      pSettings - the lease settings that are to be removed from the
    ///                  stack.
    /// \param
    ///      removeRouter - tells whether or not the router should be removed.
    ///
    /// \retval
    ///      kError if the lease address was not found or could not be removed.
    /// \retval
    ///      kNoMoreLeases if the lease was found and removed, and this was the
    ///          last lease.
    /// \retval
    ///      kMoreLeases if the lease was found and removed, but there are more
    ///          leases associated with the stack.
    ///
    virtual RemoveResult RemoveLeaseImpl(const BcmDhcpLeaseSettings *pSettings,
                                         bool removeRouter) { return kError; }
#endif
    /// This is the method the derived class must implement when all addresses
    /// associated with the stack need to be removed.
    ///
    /// \retval
    ///      true if the IP stack was shut down successfully.
    /// \retval
    ///      false if the IP stack was not shut down.
    ///
    virtual bool ShutdownIpStackImpl(void) { return false; }

    //@}

public:
      
    /** \name BcmIpStackManager support method
    *
    * This method should only be called by BcmIpStackManager; all of the
    * instances of this class need to be configured, and the manager
    * class will do this for you.
    *
    */
    //@{

    /// Tells this object whether or not it is the primary IP stack interface.
    /// Only one of the IP HalIf's should be primary.  The derived class can use
    /// this to decide whether or not certain actions can be performed.
    ///
    /// \param
    ///      isPrimary - true if this is the primary interface, false otherwise.
    ///
    inline void SelectPrimary(bool isPrimary);
    
    //@}

protected:

    /// This method causes the HalIf class to register itself with the HAL (via
    /// the specified entrypoint).
    ///
    /// PR534 - made this virtual so that the DocsisCmHalIf derived class can
    /// overload it.
    ///
    /// \param
    ///      DriverEntryPoint - pointer to the function corresponding to the
    ///                         driver entrypoint that I should call.
    ///
    /// \retval
    ///      true if the driver was registered and initialize successfully.
    /// \retval
    ///      false if there was a problem.
    ///
    virtual bool Register(PFN_BFC_HAL_DRIVER_ENTRY DriverEntryPoint);
    
protected:

    /// This method is called by the base class after it has done the initial
    /// processing of the packet that is sent to us by the Packet Switch.  This
    /// must be provided by derived classes.
    ///
    /// PR2257 - the source HalIf is specified now.
    ///
    /// \param
    ///      pPacket - the packet to be sent.
    /// \param
    ///      pSrcHalIf - the HalIf that is sending the packet to me.
    ///
    /// \retval
    ///      true if the packet was sent.
    /// \retval
    ///      false if there was a problem.
    ///
    virtual bool TransmitPacketImpl(BcmEthernetPacket *pPacket,
                                    BcmHalIf *pSrcHalIf);

protected:

    /// Used as an index into ipv4/6InterfaceTables from RFC-4293
    int fIpInterfaceIfIndex;

    /// Assigned when I was created.  Usually gotten from nonvol.
    BcmMacAddress fMyMacAddress;
#if (BFC_INCLUDE_DHCPV4_SUPPORT)    
    /// The DHCP ClientIf associated with me.  It manages my lease(s).
    BcmDhcpClientIf *pfDhcpClientIf;
#endif

#if (BFC_INCLUDE_DHCPV6_SUPPORT)    
    /// The DHCPv6 ClientIf associated with me.  It manages my IPv6 addresses.
    BcmDhcpV6ClientIf *pfDhcpV6ClientIf;
#endif
    /// Tells whether or not we are configured with one or more leases/IP
    /// addresses.
    bool fIpV4AddressConfigured;
    bool fIpV6AddressConfigured;
    bool fIpV6LinkLocalAddressConfigured;

    /// The first IP address configured for a network interface is called the
    /// primary IP address and is configured via the "set IP address" ioctl
    /// command.  Additional non-primary IP addresses may be added to the
    /// network interface via the "add IP address" ioctl command.  Subnet mask, 
    /// broadcast IP address, interface flags and default router may be
    /// configured with the primary IP address.  Primary IP address and 
    /// subnet mask are stored in data members
    ///      BcmIpAddress fPrimaryIpAddress;
    ///      BcmIpAddress fSubnetMask;
    /// 
    /// Whether or not a particular IP address is primary is determined at 
    /// DHCP lease configuration time, in virtual member function 
    /// ConfigureLeaseImpl().  The derived class implementation of 
    /// ConfigureLeaseImpl() will load data members fPrimaryIpAddress and
    /// fSubnetMask if it is configuring the primary IP address.  If the lease 
    /// for non-zero fPrimaryIpAddress is removed via RemoveLease(), 
    /// then fPrimaryIpAddress and fSubnetMask are set to kAllZerosIp.
    BcmIpAddress fPrimaryIpAddress;
    BcmIpAddress fSubnetMask;

    BcmIpV6Address fIpV6LinkLocalAddress;

    /// Used to provide a publisher-subscriber interface for async status event
    /// notification.
    /// PR4517
    BcmEventPublisher *pfEventPublisher;

    /// Tells whether or not this is the primary IP stack interface.  This can
    /// be used by the derived class to determine whether or not this interface
    /// is allowed to configure the underlying IP stack in certain ways.
    bool fIsPrimaryIpInterface;
	//#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT)
    /// Implements the Neighbor Discovery protocol for IP
    /// Version 6 described in RFC2461.
    BcmNeighborDiscoveryThread * pfNeighborDiscoveryThread;
	//#endif
private:

    /// Default Constructor.  Not supported.
    BcmBaseIpHalIf(void);

    /// Copy Constructor.  Not supported.
    BcmBaseIpHalIf(const BcmBaseIpHalIf &otherInstance);

    /// Assignment operator.  Not supported.
    BcmBaseIpHalIf & operator = (const BcmBaseIpHalIf &otherInstance);

};


//********************** Inline Method Implementations ***********************


// This allows you to do the following:
//
//    cout << halIf << endl;
//
inline ostream & operator << (ostream &outputStream, const BcmBaseIpHalIf &rhs)
{
    return rhs.Print(outputStream);
}


/// An accessor for this HAL's IP stack number
///
/// \param
///      none
///
/// \return
///      The stack number of this HAL
///
inline int BcmBaseIpHalIf::StackNumber (void) const
{
  // When this object registers with the IP stack manager (in the Register
  // method), the value passed as the stack number arg is our driver index.
  return fDriverIndex;
}

inline const BcmMacAddress &BcmBaseIpHalIf::MacAddress(void) const
{
    return fMyMacAddress;
}

inline void BcmBaseIpHalIf::SetMacAddress( BcmMacAddress & macAdr )
{
    fMyMacAddress = macAdr;
    return;
}

inline bool BcmBaseIpHalIf::GetCurrentIpAddress(BcmIpAddress &ipAddress)
{
    ipAddress = fPrimaryIpAddress;
    return true;
}


inline const BcmIpAddress& BcmBaseIpHalIf::PrimaryIpAddress() const
{
    return fPrimaryIpAddress;
}


inline const BcmIpAddress& BcmBaseIpHalIf::SubnetMask() const
{
    return fSubnetMask;
}


inline bool BcmBaseIpHalIf::GetCurrentSubnetMask(BcmIpAddress &subnetMask)
{
    subnetMask = fSubnetMask;
    return true;
}

#if (BFC_INCLUDE_DHCPV4_SUPPORT)
inline BcmDhcpClientIf *BcmBaseIpHalIf::DhcpClientIf(void) const
{
    return pfDhcpClientIf;
}
#endif
#if (BFC_INCLUDE_DHCPV6_SUPPORT)
inline BcmDhcpV6ClientIf *BcmBaseIpHalIf::DhcpV6ClientIf(void) const
{
    return pfDhcpV6ClientIf;
}
#endif

inline bool BcmBaseIpHalIf::IpAddressConfigured(void) const
{
    return fIpV4AddressConfigured || fIpV6AddressConfigured;
}

inline void BcmBaseIpHalIf::IpAddressConfigured(bool configured)
{
    fIpV4AddressConfigured = configured;
}

inline bool BcmBaseIpHalIf::Ipv6LinkLocalAddressConfigured(void) const
{
    return fIpV6LinkLocalAddressConfigured;
}

inline const BcmIpV6Address& BcmBaseIpHalIf::Ipv6LinkLocalAddress(void) const
{
    return fIpV6LinkLocalAddress;
}

inline void BcmBaseIpHalIf::Ipv6LinkLocalAddress(BcmIpV6Address& linkLocal) 
{
    fIpV6LinkLocalAddress = linkLocal;
	fIpV6LinkLocalAddressConfigured = true;
}

inline void BcmBaseIpHalIf::SelectPrimary(bool isPrimary)
{
    fIsPrimaryIpInterface = isPrimary;
}

#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT)
/// Accessor for the Neighbor Discovery Thread
///
/// \retval
///     pointer to the Neighbor Discovery Thread
inline BcmNeighborDiscoveryThread * BcmBaseIpHalIf::NeighborDiscoveryThread() const
{
    return pfNeighborDiscoveryThread;
}
#endif


#endif

