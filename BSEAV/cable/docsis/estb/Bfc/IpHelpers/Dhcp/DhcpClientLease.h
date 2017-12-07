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
//  Filename:       DhcpClientLease.h
//  Author:         David Pullen
//  Creation Date:  June 10, 2002
//
//****************************************************************************

#ifndef DHCPCLIENTLEASE_H
#define DHCPCLIENTLEASE_H

//********************** Include Files ***************************************

#include "DhcpLeaseSettings.h"
#include "DhcpClientId.h"

#include "MacAddress.h"

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#else
#include <iostream.h>
#endif

//********************** Global Types ****************************************

/// This is the prototype for callout functions that can be registered with the
/// lease.  When an offer/ack packet is received, we will evaluate it for
/// "goodness", and if everything checks out then we will call out to the
/// registered client to let it see if it's good enough.
///
/// This is for PR2807.
///
/// This is what your function should look like:
///
/// bool DhcpOfferAckCallout(const BcmDhcpLeaseSettings &settings, void *pHandle);
///
/// \param
///      settings - the settings from the offer/ack that are to be sniffed.
/// \param
///      pHandle - arbitrary handle that was passed in when the client registered
///                the callout function.  It can be whatever you want, including
///                NULL.
///
/// \retval
///      true if the offer/ack is acceptable.
/// \retval
///      false if the offer/ack is not acceptable, and should not be selected or
///          should be NAKed.
///
typedef bool (*DhcpOfferAckCallout)(const BcmDhcpLeaseSettings &settings,
                                    void *pHandle);

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmEventPublisher;
class BcmCompletionHandlerACT;
class BcmMessageLogSettings;

//********************** Class Declaration ***********************************


/** \ingroup DhcpClientServer
*
*   This object represents the DHCP lease from either the client's
*   perspective.  It contains the DHCP lease settings that are requested
*   by the client, and the lease settings that are assigned by the server,
*   plus state information to help the DHCP Client Thread manage the lease.
*/
class BcmDhcpClientLease
{
public:

    /// Initializing Constructor.  Sets the client id to the value specified.
    ///
    /// \param
    ///      clientId - the client id to use.
    ///
    BcmDhcpClientLease(const BcmDhcpClientId &clientId);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    ~BcmDhcpClientLease();

    /// Does formatted output of the object to the specified ostream.  Prints
    /// details about the lease state, as well as a good deal of information
    /// about the lease settings assigned by the DHCP server.
    ///
    /// \param
    ///      outputStream - the ostream to be used.
    ///
    /// \return
    ///      The reference to the ostream that was passed in.
    ///
    ostream & Print(ostream &outputStream) const;

public:  // Lease settings objects.

    /// These are the settings that are being requested by the client.  The
    /// client needs to configure the desired settings/options before the lease
    /// acquisition process starts.  The values in this object will be sent in a
    /// Discover/Request/Inform packet.
    ///
    /// \note  You MUST NOT change the client id option (option 61) in the
    ///        DhcpLeaseSettings objects.  These are set when the lease is
    ///        created, and must remain constant in order for the objects
    ///        to be tracked correctly (the client id is stored elsewhere
    ///        and would not be changed to match the new id).
    ///
    /// \return
    ///      A reference to the client's DHCP Lease Settings.
    ///
    inline BcmDhcpLeaseSettings &ClientLeaseSettings(void);

    /// These are the settings that are allocated for the client by the server.
    ///
    /// From the client's perspective, this is what was received from the server
    /// in an Offer/Ack/Nak packet.
    ///
    /// From the server's perspective, this is what will be sent to the client
    /// in an Offer/Ack/Nak packet.
    ///
    /// If you are configuring a static lease for the IP stack, then you will
    /// need to configure this object rather than the client lease settings.
    ///
    /// \note  You MUST NOT change the client id option (option 61) in the
    ///        DhcpLeaseSettings objects.  These are set when the lease is
    ///        created, and must remain constant in order for the objects
    ///        to be tracked correctly (the client id is stored elsewhere
    ///        and would not be changed to match the new id).
    ///
    /// \return
    ///      A reference to the server's DHCP Lease Settings.
    ///
    inline BcmDhcpLeaseSettings &ServerLeaseSettings(void);

    /// Accessor to set the MAC address of the DHCP server.
    ///
    /// \param
    ///      macAddress - the MAC address of the DHCP server (from the Ack
    ///                   packet).
    ///
    inline void ServerMacAddresss(const BcmMacAddress &macAddress);

    /// Accessor to get the MAC address of the DHCP server.
    ///
    /// \return
    ///      The MAC address of the DHCP server.
    ///
    inline const BcmMacAddress &ServerMacAddresss(void) const;

    //@{
    /// Accessor to specify that this lease is for the primary IP address
    /// of the associated network interface.    
    ///
    inline void IsYourIpThePrimaryIpAddress( bool isPrimary );
    bool IsYourIpThePrimaryIpAddress( void ) const;
    //@}

public:  // Settings that affect the operation of the lease, but that aren't
         // necessarily part of the lease settings object.

    /// There are 3 modes for auto-configure.
    typedef enum
    {
        /// When the lease is bound/lost, don't do anything to the IP stack.
        kAutoConfigDisabled = 0,

        /// When the lease is bound/lost, configure the IP address and subnet
        /// mask only.
        kAutoConfigIpAndSubnet,

        /// When the lease is bound/lost, configure the IP address, subnet mask,
        /// and router.
        kAutoConfigIpSubnetAndRouter

    } AutoConfigureMode;

    /// Tells the DHCP client whether or not the lease should be automatically
    /// used to configure the IP stack.  If set to kAutoConfigIpAndSubnet or
    /// kAutoConfigIpSubnetAndRouter, then the DHCP client thread/DhcpClientIf
    /// object will configure the IP stack whenever an Ack or Nak message is
    /// received.
    ///
    /// By default, this will be set to kAutoConfigIpSubnetAndRouter for the
    /// first lease on each stack, and kAutoConfigDisabled for any following
    /// leases on the stack.
    ///
    /// \param
    ///      mode - one of the AutoConfigureMode enum values.
    ///
    inline void AutoConfigure(AutoConfigureMode mode);

    /// Accessor to return the current auto-config mode.
    ///
    /// \return
    ///      The current auto-config mode.
    ///
    inline AutoConfigureMode AutoConfigure(void) const;

    /// Returns true if the auto-config mode is kAutoConfigIpSubnetAndRouter.
    ///
    /// \retval
    ///      true if the router should be configured.
    /// \retval
    ///      false if the router should not be configured.
    ///
    inline bool AutoConfigureRouter(void) const;

    /// Returns the client id associated with this lease.  This must be set in
    /// the constructor.
    ///
    /// \return
    ///      The client id.
    ///
    inline const BcmDhcpClientId &ClientId(void) const;

    /// The valid bits for the BOOTP-compatible fields parameter in
    /// SetRequiredFields().  Of these, typically only kYourAddr, kServerAddr,
    /// and kFileName are used.
    enum
    {
        /// ciaddr, rarely used
        kClientAddr   = 0x01,

        /// yiaddr
        kYourAddr     = 0x02,   
        
        /// siaddr
        kServerAddr   = 0x04,   
        
        /// giaddr, rarely used
        kGatewayAddr  = 0x08,   
        
        /// chaddr, rarely used
        kClientHwAddr = 0x10,   
        
        /// sname, rarely used
        kServerName   = 0x20,   
        
        /// file
        kFileName     = 0x40    
    };

    /// Sets the DHCP fields that are required in order for the Offer/Ack to be
    /// considered valid.  The client will modify the ClientLeaseSettings to
    /// have the settings that are desirable, but the client may be able to
    /// operate with only a subset of those if they are returned.
    ///
    /// Because some of the fields are part of the BOOTP-compatible fixed header
    /// in the DHCP packet, we need a bitmask to allow these to be checked.  If
    /// the value contained in the field is all 0's, then it is considered not
    /// to be present.
    ///
    /// The rest are DHCP options are TLV encoded.  You specify the desired
    /// options via the octet buffer, which contains a list of the 8-bit option
    /// codes (see DhcpOptionsRfc2132.h for a list of the options).
    ///
    /// \note  The IP stack and DHCP client may impose additional requirements
    ///        above and beyond this list, which may cause a lease to be
    ///        rejected.  You should only set the items that you truly care
    ///        about; don't try to second guess what the stack or DHCP client
    ///        will need - they will take care of themselves.
    ///
    /// \param
    ///      bootpFields - a bitmask of the BOOTP-compatible fields that are
    ///                    required.
    /// \param
    ///      buffer - a buffer containing the DHCP option codes that are
    ///               required.
    ///
    void SetRequiredFields(uint8 bootpFields, const BcmOctetBuffer &buffer);

    /// Same as above, but doesn't require the octet buffer if all you care
    /// about are the fixed BOOTP-compatible fields.
    ///
    /// \param
    ///      bootpFields - a bitmask of the BOOTP-compatible fields that are
    ///                    required.
    ///
    void SetRequiredFields(uint8 bootpFields);

    /// Sets the DHCP fields that are "preferred", but not required.  These are
    /// used when evaluating and comparing Offers received from a server.  The
    /// Offer that has all of the required fields and that has more of the
    /// "preferred" fields will be considered the best and will be selected.
    ///
    /// The parameters are the same as for SetRequiredFields().  The buffer will
    /// usually contain the same list of options that were sent in option 55
    /// (the Parameter Request list); obviously, if you requested the option,
    /// then it would be preferred if it was present.  But, since the client may
    /// actually want more options, or totally not care about others, we allow
    /// this to be specified separately.
    ///
    /// \param
    ///      bootpFields - a bitmask of the BOOTP-compatible fields that are
    ///                    preferred.
    /// \param
    ///      buffer - a buffer containing the DHCP option codes that are
    ///               preferred.
    ///
    void SetPreferredFields(uint8 bootpFields, const BcmOctetBuffer &buffer);

    /// Same as above, but doesn't require the octet buffer if all you care
    /// about are the fixed BOOTP-compatible fields.
    ///
    /// \param
    ///      bootpFields - a bitmask of the BOOTP-compatible fields that are
    ///                    preferred.
    ///
    void SetPreferredFields(uint8 bootpFields);

    /// Sets the DHCP option number to use for PacketCable servers.  The IETF
    /// has issued an official option number for PacketCable, but there are
    /// systems that still only support the older option number, so we need to
    /// be able to choose which one we want to use.
    ///
    /// \param
    ///     packetCableDhcpOptionNumber - the PacketCable option number to use.
    ///
    /// \return
    ///     This method always returns true; I'm not sure why there is a
    ///     return value at all...
    ///
    bool SetPacketCableDhcpOptionNumber(uint8 packetCableDhcpOptionNumber);

public:  // Event subscribe/publish methods.

    /// These are the events that I publish.
    typedef enum
    {
        /// Indicates that the lease object has been configured and that we are
        /// about to start the acquisition process.  This event allows secondary
        /// entities to modify the lease object before it is used; note that
        /// the primary client (who started the acquisition) probably configured
        /// a number of items to meet its requirements, so you should avoid
        /// changing the lease in a way that would break this.
        kEventAcquisitionStarting = 0,

        /// Indicates that the lease has been successfully acquired/renewed/etc
        /// and that the lease settings are different than what they were
        /// previously.  This event will be sent on the first bind, but will
        /// only be sent when renewing/rebinding if there is something
        /// different.
        kEventLeaseBound,

        /// Indicates that we received an Offer that is missing required fields.
        kEventBadOfferMissingRequiredFields,

        /// Indicates that we received an Offer that is valid (has all required
        /// fields) but that is missing one or more preferred fields.
        kEventOfferMissingPreferredFields,

        /// Indicates that we didn't receive any valid Offers after sending a
        /// Discover.  We may or may not retry the Discover.
        kEventNoOffers,

        /// Indicates that we didn't receive an Ack (or Nak) after sending a
        /// Request.
        kEventNoAck,

        /// Indicates that we received an Ack that is missing required fields.
        kEventBadAckMissingRequiredFields,

        /// Indicates that we received an Ack that is valid (has all required
        /// fields) but that is missing one or more preferred fields.
        kEventAckMissingPreferredFields,

        /// Indicates that we failed to acquire the lease (either because we
        /// didn't get any Acks after retrying the Request, or we got a Nak,
        /// or possibly because we didn't like the Ack we got).
        kEventLeaseFailed,

        /// Indicates that we failed to renew/rebind our lease.
        kEventLeaseLost,

        /// Indicates that we successfully released our lease.
        kEventLeaseReleased,

        /// Indicates a T1 timeout has occured and we are starting 
        /// the renewal process on our lease.  Event is published just before 
        /// Request message is sent. 
        kEventRenewing,

        /// Indicates a T2 timeout has occured and the client moves to the 
        /// REBINDING state and sends broadcast request messages
        /// to extend its lease.  Event is published just before broadcast
        /// request message is sent. 
        kEventRebinding,

        /// PR11468 - Indicates that the server sent a NAK in response to our
        /// request packet.  Subscribers of this event must process it
        /// synchronously, since the DHCP client will act on the NAK as soon
        /// as the event publisher returns.  Usually, subscribers will act on
        /// this event by calling IgnoreNak() to enable/disable NAK processing.
        kEventNakReceived,

    } DhcpClientLeaseEvent;

    /// Lets one or more clients sign up for notification of events that occur
    /// to this lease.
    ///
    /// \param
    ///      eventCode - the event code for the status event of interest.
    /// \param
    ///      pAct - pointer to the ACT that will be notified.
    ///
    void Subscribe(DhcpClientLeaseEvent eventCode, 
                   BcmCompletionHandlerACT *pAct);

    /// Lets the clients stop receiving notification of events that occur to
    /// this lease.  Note that if the lease is destroyed (after being released),
    /// then you will automatically be unsubscribed.
    ///
    /// \param
    ///      eventCode - the event code for the status event of interest.
    /// \param
    ///      pAct - pointer to the ACT that was previously subscribed.
    ///
    void Unsubscribe(DhcpClientLeaseEvent eventCode,
                     BcmCompletionHandlerACT *pAct);

    /// Lets a single client register its callout function so that it can
    /// inspect Offer and Ack packets from the DHCP server.  If another client
    /// had been registered, then it will no longer be called (we don't support
    /// a list of callouts).
    ///
    /// To deregister yourself, call this method with NULL as the callout
    /// function address.
    ///
    /// This is for PR2807.
    ///
    /// \param
    ///      pDhcpOfferAckCallout - the function to call.
    /// \param
    ///      pHandle - arbitrary handle to be sent when the callback function is
    ///                called.  You can use this for anything you want.
    ///
    void RegisterOfferAckCallout(DhcpOfferAckCallout pDhcpOfferAckCallout,
                                 void *pHandle = NULL);

public:  // Lease state information.  This should only be called by the
         // DhcpClientIf object.

    /// States that the lease can be in.  These are from the perspective of the
    /// client, who is requesting a lease.  Some of them are directly from
    /// RFC2131, others are internal states to handle intermediate events or
    /// actions that must occur.
    typedef enum
    {
        /// From RFC2131
        kStateInit = 0,         

        /// This is not in the RFC; it's our internal state that handles the
        /// random delay before we send the discover message.
        kStateInitDelay,        

        /// From RFC2131
        kStateSelecting,

        /// Also not in the RFC; we transition to this state after we time out
        /// wait for offers, and no valid offers were received.  We delay for
        /// a bit in this state, then go back to sending the Discover again.
        kStateDiscoverDelay,

        /// From RFC2131
        kStateRequesting,
        /// From RFC2131
        kStateBound,
        /// From RFC2131
        kStateRenewing,
        /// From RFC2131
        kStateRebinding,

        /// This isn't in the RFC either; it's our internal state that handles
        /// the intermediate state between declining an addresss and going back
        /// to initial state.
        kStateDeclining,        

        /// Also not in the RFC; handles the intermediate state between
        /// releasing an address and going back to initial state.  For this
        /// one, we will send a Release packet to the DHCP server.
        kStateReleasingSendPacket,  

        /// Similar to kStateReleasingSendPacket, but we won't send a Release
        /// packet to the DHCP server.
        kStateReleasingNoPacket,    

        // These aren't supported right now, so let's not pretend that they
        // exist sincle we won't handle them.
        kStateInitReboot,
        kStateRebooting

    } LeaseState;

    /// Accessor to set the lease state.
    ///
    /// \param
    ///      state - the new state.
    ///
    inline void State(LeaseState state);

    /// Accessor to get the lease state.
    ///
    /// \return
    ///      The current state.
    ///
    inline LeaseState State(void) const;

    /// Returns true if lease state is consistent with a bound "my IP" address.
    ///
    /// \return
    ///      true if "my IP" address is bound.
    ///
    inline bool IsIpAddressBound(void) const;

    /// Specific method to handle the transition to the bound state.  We need
    /// to know the current time (when the lease was bound) in order to set
    /// the lease time, T1, and T2 correctly.  T1 and T2 are queried from
    /// the DHCP server settings object and are added to the current time.  The
    /// value of T1 becomes the next event time.
    ///
    /// Note that this value has nothing to do with calendar or clock time; it
    /// will often be set to the number of seconds since the system rebooted.
    ///
    /// In fact, we DON'T recommend using the C function time() to set this
    /// value, because time() returns a value based on the time of day, and
    /// that value often changes after DHCP is complete.  We have seen several
    /// DHCP implementations that use the time of day, and they all have
    /// problems.
    ///
    /// \param
    ///      currentTime - the current time (in seconds).
    /// \param
    ///      staticallyProvisioned - true if this is a static lease, false if
    ///                              the lease was acquired via DHCP.
    ///
    void Bound(unsigned int currentTime, bool staticallyProvisioned);

    /// Returns the statically-provisioned state.
    ///
    /// \retval
    ///      true if the lease is statically provisioned.
    /// \retval
    ///      false if the lease is not statically provisioned.
    ///
    inline bool StaticallyProvisioned(void) const;

    /// Accessor to set the XID that is being used in the DHCP packets.  This is
    /// actually assigned by the DHCP client thread, but stored here.  A new
    /// XID will be generated for each major transaction.
    ///
    /// \param
    ///      xid - the XID parameter.
    ///
    inline void Xid(uint32 xid);

    /// Accessor to get the XID that is being used in the DHCP packets.
    ///
    /// \return
    ///      The XID parameter.
    ///
    inline uint32 Xid(void) const;

    //@{
    /// Accessors for the number of tries associated with the current operation
    /// or state.  This is used to determine the backoff amount.
    ///
    /// \param
    ///      numberOfTries - the current number of tries.
    ///
    /// \return
    ///      The current number of tries.
    ///
    inline void NumberOfTries(unsigned int numberOfTries);
    inline unsigned int NumberOfTries(void) const;
    inline void IncrementNumberOfTries(void);
    //@}

    //@{
    /// Accessors for the maximum number of tries that are allowed for Discover
    /// and Request packets.  By default, these are set to 6, but they can be
    /// overridden.
    ///
    /// \param
    ///      maxNumberOfTries - the max number of tries.
    ///
    /// \return
    ///      The max number of tries.
    ///
    inline void MaxNumberOfDiscoverTries(unsigned int maxNumberOfTries);
    inline void MaxNumberOfRequestTries(unsigned int maxNumberOfTries);
    inline unsigned int MaxNumberOfDiscoverTries(void) const;
    inline unsigned int MaxNumberOfRequestTries(void) const;
    //@}

    //@{
    /// Accessors for the lease time, which is used to track elapsed time since
    /// some event or other started.
    ///
    /// \param
    ///      currentTime - the current time (in seconds).
    ///
    /// \return
    ///      The stored time.
    ///
    inline void CurrentTimeSec(unsigned int currentTime);
    inline unsigned int CurrentTimeSec(void) const;
    inline void IncrementCurrentTimeSec(unsigned int elapsedTime);
    //@}

    //@{
    /// Accessors for the next event time.  This is the time (in seconds) when
    /// the next major event needs to happen, and it is usually calculated as
    /// an offset from the current time.  For example, when we are bound to an
    /// address, we store the current time and calculate T1 as an offset from
    /// this.  The expiry of T1 becomes the next event time.
    ///
    /// \param
    ///      nextTime - the time (in seconds) of the next event.
    ///
    /// \return
    ///      The next event time.
    ///
    inline void NextEventTimeSec(unsigned int nextTime);
    inline unsigned int NextEventTimeSec(void) const;
    //@}

    //@{
    /// Returns the time (in seconds) when T1, T2 and the lease expire.  These
    /// values are set when Bound() is called.
    ///
    /// \return
    ///      The time when T1/T2/lease expire.
    ///
    inline unsigned int T1ExpireTimeSec(void) const;
    inline unsigned int T2ExpireTimeSec(void) const;
    inline unsigned int LeaseExpireTimeSec(void) const;
    //@}

    /// Tells whether or not the lease is infinite.
    ///
    /// \retval
    ///      true if infinite
    /// \retval
    ///      false if not infinite
    ///
    inline bool LeaseIsInfinite(void) const;

    /// Returns the number of seconds until the lease expires.  If the lease
    /// is not bound (or renewing), then this returns 0.  If the lease is
    /// infinite, then it returns 0xffffffff.
    ///
    /// \return
    ///     The number of seconds until the lease expires.
    ///
    unsigned int TimeLeftOnLease(void) const;

    /// Causes the specified event to be published to all of the clients that
    /// have subscribed with me.
    ///
    /// \param
    ///      eventCode - the event that occurred.
    ///
    void PublishEvent(DhcpClientLeaseEvent eventCode);

    /// Lets this object sniff the DHCP offer/ack to see if it is acceptable.
    ///
    /// \param
    ///      pDhcpLeaseSettings - the settings from the offer/ack to sniff.
    /// \param
    ///      pMessageLogSettings - optional message log settings object to be
    ///                            used for logging debug info (what fields are
    ///                            required but missing).  If NULL, then no
    ///                            logging will be performed.
    ///
    /// \retval
    ///      true if the settings are acceptable.
    /// \retval
    ///      false if there are one or more fields missing that make it
    ///          unacceptable.
    ///
    bool IsAcceptable(const BcmDhcpLeaseSettings *pDhcpLeaseSettings,
                      BcmMessageLogSettings *pMessageLogSettings = NULL) const;

    /// Lets this object sniff the DHCP offer to see if it is better than the
    /// last one we received and stored.
    ///
    /// \param
    ///      pDhcpLeaseSettings - the settings from the offer to sniff.
    ///
    /// \retval
    ///      true if the new settings are better.
    /// \retval
    ///      false if the current settings are better.
    ///
    bool IsBetter(const BcmDhcpLeaseSettings *pDhcpLeaseSettings) const;

    /// Lets this object sniff the DHCP offer/ack to see if it is "perfect" - in
    /// other words, it has all of the required fields and all of the preferred
    /// fields.  If any of the required or preferred fields are not present,
    /// then it isn't perfect.
    ///
    /// \param
    ///      pDhcpLeaseSettings - the settings from the offer/ack to sniff.
    ///
    /// \retval
    ///      true if the settings are perfect.
    /// \retval
    ///      false if the settings are missing one or more fields.
    ///
    bool IsPerfect(const BcmDhcpLeaseSettings *pDhcpLeaseSettings) const;

    //@{
    /// Controls whether or not the DHCP client ignores NAKs from the server.
    /// By default, this is false (don't ignore NAKs).  If this is enabled,
    /// we will not change the lease state.
    ///
    /// \note Ignoring NAKs violates RFC2131!  There are some environments where
    ///       this violation is required, but it is generally a bad idea to do
    ///       this.
    ///
    /// Added for PR11468
    ///
    /// \param
    ///     ignore - set this to true to ignore NAKs from the server, or false
    ///              to restore RFC2131-compliant behavior.
    ///
    /// \retval
    ///     true if the DHCP client is ignoring NAKs for this lease.
    /// \retval
    ///     false if the DHCP client will act on NAKs according to RFC2131.
    ///
    inline bool IgnoreNak(void) const;
    inline void IgnoreNak(bool ignore);
    //@}

private:

    /// The client id for this lease.  This is used when storing in STL
    /// containers, etc.
    BcmDhcpClientId fClientId;

    /// The state of the lease.
    LeaseState fState;

    /// Tells whether or not the lease was statically provisioned, or acquired
    /// via DHCP.  Only valid if the state is kBound.  This determines whether
    /// or not we will attempt to release the lease (send a RELEASE packet)
    /// later when the lease is not needed.
    bool fStaticallyProvisioned;

    /// Tells whether or not the client wants us to configure the IP stack with
    /// this lease automatically.
    AutoConfigureMode fAutoConfigure;

    /// The XID being used to request the lease.
    uint32 fXid;

    /// The number of tries associated with the current operation/state (sending
    /// a Discover, sending a Request, etc).  This is used to determine the
    /// backoff amount.
    unsigned int fNumberOfTries;

    /// The maximum number of tries allowed for Discover and Request packets.
    unsigned int fMaxDiscoverTries;
    unsigned int fMaxRequestTries;

    /// The client's lease settings.
    BcmDhcpLeaseSettings fClientLeaseSettings;

    /// The server's lease settings.
    BcmDhcpLeaseSettings fServerLeaseSettings;

    /// The MAC address of the server.  I use this for renewals.
    BcmMacAddress fServerMacAddress;

    /// The system time when the lease was bound.
    unsigned int fLeaseBindTimeSec;

    /// Tells whether or not this is an infinite lease.
    bool fLeaseIsInfinite;

    /// A time holder, used by the ClientIf class to determine how long it's
    /// been since something started.
    unsigned int fCurrentTimeSec;

    /// The time when the something needs to happen (renew lease, send Discover,
    /// etc).  This is set by the ClientIf.
    unsigned int fNextEventTimeSec;

    /// The T1 and T2 values for the lease (in seconds).  These are relative to
    /// the lease start time.
    unsigned int fT1, fT2;

    /// The time when the lease will expire.
    unsigned int fLeaseExpireTimeSec;

    /// The fields required for the Offer/Bind to be valid.
    uint8 fRequiredBootpFields;
    BcmOctetBuffer fRequiredDhcpOptions;

    /// The fields that are preferred in an Offer.
    uint8 fPreferredBootpFields;
    BcmOctetBuffer fPreferredDhcpOptions;

    /// Used to provide a publisher-subscriber interface for async status event
    /// notification.
    BcmEventPublisher *pfEventPublisher;

    /// PR2807 - Callout function registered by a client.
    DhcpOfferAckCallout pfDhcpOfferAckCallout;
    void *pfClientHandle;

    /// The PacketCable servers option number to look for.  This is programable
    /// as either 122 (the new IETF assigned number) or 177 (the old temporary
    /// number) to facilitate interoperation with old school DHCP servers.
    uint8 fPacketCableDhcpOptionNumber;

    /// PR11468 - controls whether we act on or ignore a NAK from the server.
    bool fIgnoreNak;

private:

    /// Default Constructor.  Not supported.
    BcmDhcpClientLease(void);

    /// Copy Constructor.  Not supported.
    BcmDhcpClientLease(const BcmDhcpClientLease &otherInstance);

    /// Assignment operator.  Not supported.
    BcmDhcpClientLease & operator = (const BcmDhcpClientLease &otherInstance);

};


//********************** Inline Method Implementations ***********************


/// This allows you to do the following:
///
///    BcmDhcpClientLease lease;
///
///    cout << lease << endl;
///
inline ostream & operator << (ostream &outputStream, const BcmDhcpClientLease &lease)
{
    return lease.Print(outputStream);
}


inline BcmDhcpLeaseSettings &BcmDhcpClientLease::ClientLeaseSettings(void)
{
    return fClientLeaseSettings;
}


inline BcmDhcpLeaseSettings &BcmDhcpClientLease::ServerLeaseSettings(void)
{
    return fServerLeaseSettings;
}


inline void BcmDhcpClientLease::ServerMacAddresss(const BcmMacAddress &macAddress)
{
    fServerMacAddress = macAddress;
}


inline const BcmMacAddress &BcmDhcpClientLease::ServerMacAddresss(void) const
{
    return fServerMacAddress;
}

inline void BcmDhcpClientLease::IsYourIpThePrimaryIpAddress( bool isPrimary )
{
    fClientLeaseSettings.IsYourIpThePrimaryIpAddress( isPrimary );
    fServerLeaseSettings.IsYourIpThePrimaryIpAddress( isPrimary );
}

inline void BcmDhcpClientLease::AutoConfigure(AutoConfigureMode mode)
{
    fAutoConfigure = mode;
}


inline BcmDhcpClientLease::AutoConfigureMode BcmDhcpClientLease::AutoConfigure(void) const
{
    return fAutoConfigure;
}


inline bool BcmDhcpClientLease::AutoConfigureRouter(void) const
{
    if (fAutoConfigure == kAutoConfigIpSubnetAndRouter)
    {
        return true;
    }

    return false;
}


inline const BcmDhcpClientId &BcmDhcpClientLease::ClientId(void) const
{
    return fClientId;
}


inline void BcmDhcpClientLease::State(LeaseState state)
{
    fState = state;
}


inline BcmDhcpClientLease::LeaseState BcmDhcpClientLease::State(void) const
{
    return fState;
}


inline bool BcmDhcpClientLease::IsIpAddressBound(void) const
{
    if( (fState == kStateBound) 
        || (fState == kStateRenewing) 
        || (fState == kStateRebinding) )
    {
        return true;
    }
    return false;
}



inline bool BcmDhcpClientLease::StaticallyProvisioned(void) const
{
    return fStaticallyProvisioned;
}


inline void BcmDhcpClientLease::Xid(uint32 xid)
{
    fXid = xid;
}


inline uint32 BcmDhcpClientLease::Xid(void) const
{
    return fXid;
}


inline void BcmDhcpClientLease::NumberOfTries(unsigned int numberOfTries)
{
    fNumberOfTries = numberOfTries;
}


inline unsigned int BcmDhcpClientLease::NumberOfTries(void) const
{
    return fNumberOfTries;
}


inline void BcmDhcpClientLease::IncrementNumberOfTries(void)
{
    fNumberOfTries++;
}


inline void BcmDhcpClientLease::MaxNumberOfDiscoverTries(unsigned int maxNumberOfTries)
{
    fMaxDiscoverTries = maxNumberOfTries;
}


inline void BcmDhcpClientLease::MaxNumberOfRequestTries(unsigned int maxNumberOfTries)
{
    fMaxRequestTries = maxNumberOfTries;
}


inline unsigned int BcmDhcpClientLease::MaxNumberOfDiscoverTries(void) const
{
    return fMaxDiscoverTries;
}


inline unsigned int BcmDhcpClientLease::MaxNumberOfRequestTries(void) const
{
    return fMaxRequestTries;
}


inline void BcmDhcpClientLease::CurrentTimeSec(unsigned int currentTime)
{
    fCurrentTimeSec = currentTime;
}


inline unsigned int BcmDhcpClientLease::CurrentTimeSec(void) const
{
    return fCurrentTimeSec;
}


inline void BcmDhcpClientLease::IncrementCurrentTimeSec(unsigned int elapsedTime)
{
    fCurrentTimeSec += elapsedTime;
}


inline void BcmDhcpClientLease::NextEventTimeSec(unsigned int nextTime)
{
    fNextEventTimeSec = nextTime;
}


inline unsigned int BcmDhcpClientLease::NextEventTimeSec(void) const
{
    return fNextEventTimeSec;
}


inline unsigned int BcmDhcpClientLease::T1ExpireTimeSec(void) const
{
    return fT1;
}


inline unsigned int BcmDhcpClientLease::T2ExpireTimeSec(void) const
{
    return fT2;
}


inline unsigned int BcmDhcpClientLease::LeaseExpireTimeSec(void) const
{
    return fLeaseExpireTimeSec;
}


inline bool BcmDhcpClientLease::LeaseIsInfinite(void) const
{
    return fLeaseIsInfinite;
}


// PR11468
inline bool BcmDhcpClientLease::IgnoreNak(void) const
{
    return fIgnoreNak;
}


// PR11468
inline void BcmDhcpClientLease::IgnoreNak(bool ignore)
{
    fIgnoreNak = ignore;
}

#endif


