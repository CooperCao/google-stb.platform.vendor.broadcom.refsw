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
//  Filename:       HalIf.h
//  Author:         David Pullen
//  Creation Date:  March 13, 2000
//
//****************************************************************************
//  Description:
//      This is the abstract base class for all of the types of HAL Interface
//      objects.  The HALs are the hardware drivers (or the internal IP stack
//      interface) that handle tx/rx of packets.
//
//      The BcmHalIf objects provide a common, object-based abstraction for
//      the HALs.  They are used  by the Forwarder (or Router) object when
//      bridging packets from one interface to another.
//
//      Each BcmHalIf object contains a bitmask of the other BcmHalIf objects
//      that it is allowed to send packets to.  Using this bitmask, (and the
//      bit assigned to each object), we can selectively control whether or not
//      packets can be sent from one HalIf to any other HalIf.  This control
//      can be unidirectional, or bidirectional.
//
//      By default, the masks are set so that all interfaces are enabled (and
//      can be enabled), and none can be disabled.
//
//****************************************************************************

#ifndef HALIF_H
#define HALIF_H

//********************** Include Files ***************************************

#include "MacAddress.h"

#include "MessageLog.h"

#include "IANAifType.h"

#include "PacketSwitch.h"
#include "InterfaceCounters.h"

#include "BcmString.h"

// PR1880
#include "Snoop.h"

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <vector>
#include <algorithm>
#else
#include <vector.h>
#include <algo.h>
#endif

// PR 12207 - event publisher
#include "EventPublisher.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

class BcmHalIf;

// This is a special constant value that can be used in the association with
// MAC addresses in the learning table.  It means that the HalIf pointer is
// not yet known.
extern BcmHalIf *kUnassociatedHalIf;

//********************** Forward Declarations ********************************

class BcmEthernetPacket;
class BcmSnmpAgent;
class ifEntryBridge;
class BcmCommandTable;

//********************** Class Declaration ***********************************


class BcmHalIf
{
public:

    // These are IANA Interface Type numbers (of interest to us).  For a
    // complete list of the numbers, see Mibs/IANAifType.mib
    typedef enum
    {
        kIanaTypeOther =            CONSTVAL_IANAIFTYPE_OTHER,
        kIanaTypeEthernet =         CONSTVAL_IANAIFTYPE_ETHERNETCSMACD,
        kIanaTypeSoftwareLoopback = CONSTVAL_IANAIFTYPE_SOFTWARELOOPBACK,
        kIanaTypeHal80211 =         CONSTVAL_IANAIFTYPE_IEEE80211,
        kIanaTypeCmMac =            CONSTVAL_IANAIFTYPE_DOCSCABLEMACLAYER,
        kIanaTypeCmDownstream =     CONSTVAL_IANAIFTYPE_DOCSCABLEDOWNSTREAM,
        kIanaTypeCmUpstream =       CONSTVAL_IANAIFTYPE_DOCSCABLEUPSTREAM,
        kIanaTypeUsb =              CONSTVAL_IANAIFTYPE_USB,
        // KO: new IANA type for HPNA, 1/13/2003
        kIanaTypeHpna =             CONSTVAL_IANAIFTYPE_HOMEPNA,
        
        // Start of types unassigned by IANA. These will all return Other (1)    
        // on a MIB query, but internally we need to distinguish among them. 
        // Note that constants here come from IANAifType.h and are BRCM extensions
        // to the IANAifType.mib
        
        kIanaTypeUnassignedBase =   CONSTVAL_IANAIFTYPE_BRCMBASE, 
        kIanaTypeIpStack =          CONSTVAL_IANAIFTYPE_BRCMIPSTACK, 
        kIanaTypePci =              CONSTVAL_IANAIFTYPE_BRCMPCI, 
        // KO: value below is deprecated, IANA assigned an HPNA type on
        // 1/13/2003
        //kIanaTypeHpna =             CONSTVAL_IANAIFTYPE_BRCMHPNA,
        kIanaTypeBluetooth =        CONSTVAL_IANAIFTYPE_BRCMBLUETOOTH, 
        kIanaTypePktGenerator =     CONSTVAL_IANAIFTYPE_BRCMPKTGENERATOR,

        // This was added to support the dual forwarder design for CableHome.
        // There are a pair of HalIf object that aren't really actual
        // interfaces; they are virtual interfaces that just bridge between
        // Forwarders.
        kIanaTypeVirtualInterface = CONSTVAL_IANAIFTYPE_BRCMVIRTUALINTERFACE,

        // These are types related to the above type, but more specifically for
        // Packet Cable and Cable Home.  KevO requested this so he can key off
        // of this value to place ifTable entries at the appropriate index.
        kIanaTypeVirtualCableHomeInterface = CONSTVAL_IANAIFTYPE_BRCMVIRTUALCABLEHOMEINTERFACE,
        kIanaTypeVirtualPacketCableInterface = CONSTVAL_IANAIFTYPE_BRCMVIRTUALPACKETCABLEINTERFACE,
        kIanaTypeCableHomeWanDataInterface = CONSTVAL_IANAIFTYPE_BRCMCABLEHOMEWANDATAINTERFACE,
        kIanaTypeCableHomeAggregatedLanInterface = CONSTVAL_IANAIFTYPE_BRCMCABLEHOMEAGGREGATEDLANINTERFACE,
        kIanaTypeCableHomeAggregatedWirelessLanInterface = CONSTVAL_IANAIFTYPE_BRCMCABLEHOMEAGGREGATEDWIRELESSLANINTERFACE,
        kIanaTypeOpenCableDsgClientInterface = CONSTVAL_IANAIFTYPE_BRCMOPENCABLEDSGCLIENTINTERFACE,
        kIanaTypeOpenCableInteractiveIpInterface = CONSTVAL_IANAIFTYPE_BRCMOPENCABLEINTERACTIVEIPINTERFACE,
        kIanaTypeHomePlugInterface = CONSTVAL_IANAIFTYPE_BRCMHOMEPLUGINTERFACE

    } IanaInterfaceType;

    // These are the interface numbers (essentially hardware port numbers) that
    // are used when addressing a particular HalIf.
    enum
    {
        // The IP Stack's interface number is generally assigned by the stack.
        // This is just a dummy placeholder.
        kIpStackIf = 0,

        kCmMacHalIf = 10,
        kCmUpstreamHalIf = 11,
        kCmDownstreamHalIf = 12,

        // If this value changes, then ETHERNET_BASE_INTERFACENUM in EnetNiHal.h needs to change also
        kEthernetHalIf = 13,
        // The next 15 are saved for the Managed Switch!!!!
        kEthernetHalIf1 = 14,
        kEthernetHalIf2 = 15,
        kEthernetHalIf3 = 16,
        kEthernetHalIf4 = 17,
        kEthernetHalIf5 = 18,
        kEthernetHalIf6 = 19,
        kEthernetHalIf7 = 20,
        kEthernetHalIf8 = 21,
        kEthernetHalIf9 = 22,
        kEthernetHalIf10 = 23,
        kEthernetHalIf11 = 24,
        kEthernetHalIf12 = 25,
        kEthernetHalIf13 = 26,
        kEthernetHalIf14 = 27,
        kEthernetHalIf15 = 28,

        kUsbHalIf = 29,
        kHpnaHalIf = 30,
        kPacketGeneratorHalIf = 31,
        kBluetoothHalIf = 32,
        
        // On some chips (BCM4710/4702) we have 2 ethernet ports, one for LAN
        // and one for WAN.  This one is for the WAN side.
        kEthernetWanHalIf = 38,

        // These were added to support the dual forwarder design for CableHome.
        // There are a pair of HalIf objects that exist between the two
        // Forwarders; one represents a virtual WAN interface, the other
        // represents a virtual LAN/CPE interface.  The job of these objects is
        // to bridge packets between the two forwarders.
        kVirtualLanCpeHalIf = 39,
        kVirtualWanHalIf = 40,

        kPciHalIf = 41,
        kDavicMacHalIf = 42,
        kScbHalIf = 43,

        // If this value changes, then WIFI80211_BASE_INTERFACENUM in Hal80211NiHal.h needs to change also
        kHal80211HalIf   = 100,     // Main SSID (MBSS #1 SSID)
        kHal80211HalIf1  = 101,     // Guest SSID (MBSS #2 SSID)
        kHal80211HalIf2  = 102,     // MBSS #3 SSID
        kHal80211HalIf3  = 103,     // MBSS #4 SSID
        kHal80211HalIf4  = 104,     // WDS 1
        kHal80211HalIf5  = 105,     // WDS 2
        kHal80211HalIf6  = 106,     // WDS 3
        kHal80211HalIf7  = 107,     // WDS 4
        // The next 8 are reserved for future expansion of the WiFi 802.11 virtual interfaces!!!!
        kHal80211HalIf8  = 108,
        kHal80211HalIf9  = 109,
        kHal80211HalIf10 = 110,
        kHal80211HalIf11 = 111,
        kHal80211HalIf12 = 112,
        kHal80211HalIf13 = 113,
        kHal80211HalIf14 = 114,
        kHal80211HalIf15 = 115

    } InterfaceNumbers;

    // Initializing Constructor.  Stores the information specified.
    //
    // Note that some HalIf objects will allow network access to be disabled
    // (via some management command, as in the DOCSIS NACO setting), and some
    // must not be turned off.  The networkAccessCanBeDisabled parameter tells
    // whether or not the HalIf will allow this to happen.
    //
    // Parameters:
    //      interfaceNumber - the interface number that is being assigned.
    //      interfaceType - the type of interface (gives a hint as to what the
    //                      physical device is).
    //      pDescription - text string describing the HalIf, for debugging.
    //
    // Returns:  N/A
    //
    BcmHalIf(unsigned int interfaceNumber, IanaInterfaceType interfaceType,
             const char *pDescription = NULL);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmHalIf();

public:  // Data tx/rx API.

    // This method is called (generally by a forwarder/router Packet Switch
    // object) to cause a packet to be sent to the HAL.
    //
    // PR2257 - the source HalIf is specified now.
    //
    // PR1880 - When a Snoop needs to send a packet, it will often want to skip
    // all of the processing done so far by earlier Snoops.  In this case, the
    // Snoop can specify itself as the pSnoop parameter, and processing will
    // resume with the Snoop that follows it in the list.  This should only be
    // done in cases where there would be recursion if the same Snoop ran on
    // its own packet.
    //
    // Parameters:
    //      pPacket - the packet to be sent.
    //      pSrcHalIf - the HalIf that is sending the packet to me.
    //      pSnoop - the Snoop to resume processing after; this allows
    //               preceeding Snoops to be skipped.
    //
    // Returns:
    //      true if the packet was sent.
    //      false if there was a problem.
    //
    inline bool TransmitPacket(BcmEthernetPacket *pPacket, BcmHalIf *pSrcHalIf,
                               BcmSnoop *pSnoop = NULL);

    // This method is called (generally by the HAL) to cause a packet to be
    // processed, and possibly sent to the forwarder/router Packet Switch object.
    //
    // The HalIf class will process the packet (doing any filtering or other
    // processing that is necessary).
    //
    // PR2412 - changed the parameter from the HAL ETHERNET_PACKET struct to
    // the V2 BcmEthernetPacket object, which helps make the BcmHalIf class
    // more portable.
    //
    // PR1880 - When a Snoop needs to send a packet, it will often want to skip
    // all of the processing done so far by earlier Snoops.  In this case, the
    // Snoop can specify itself as the pSnoop parameter, and processing will
    // resume with the Snoop that follows it in the list.  This should only be
    // done in cases where there would be recursion if the same Snoop ran on
    // its own packet.
    //
    // PR12290 - added support for skipping all inbound Snoops.
    //
    // Parameters:
    //      pPacket - the packet to be processed.
    //      pSnoop - the Snoop to resume processing after; this allows
    //               preceeding Snoops to be skipped.
    //      skipAllSnoops - set this to true to skip all Snoop processing.
    //                      This should almost never be used; it is designed
    //                      to optimize the data path for voice packets.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (the packet must be released by the
    //          caller).
    //
    inline bool ReceivePacket(BcmEthernetPacket *pPacket, 
                              BcmSnoop *pSnoop = NULL,
                              bool skipAllSnoops = false);

    // This method is called (generally by the HAL) to cause a packet to be
    // processed, and possibly sent to the forwarder/router Packet Switch object.
    //
    // The HalIf class will process the packet (doing any filtering or other
    // processing that is necessary).
    //
    // PR2412 - changed the parameter from the HAL ETHERNET_PACKET struct to
    // the V2 BcmEthernetPacket object, which helps make the BcmHalIf class
    // more portable.
    //
    //
    // Parameters:
    //      mask    - the mask applied to the broadcast or multicast packets
    //      pPacket - the packet to be processed.
    //      pSnoop - the Snoop to resume processing after; this allows
    //               preceeding Snoops to be skipped.
    //      skipFirstSnoop - whether to skip the first snoop or not. 
    //              Note:
    //                Currently L2VPN assumes that the first snoop is the IGMP snoop
    //                For Docsis, this is the only snoop on the downstream. If this
    //                changes, then we will need to pass in the snoop to be skipped.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (the packet must be released by the
    //          caller).
    //
    inline bool ReceivePacketWithMask(uint32 mask, BcmEthernetPacket *pPacket, bool skipFirstSnoop);

    // This method is called (generally by the Packet Switch or another HalIf
    // instance) when it has finished using the packet.  This may or may not
    // actually release the packet, depending on whether or not anyone else is
    // using it.
    //
    // NOTE:  Once you call this method, you must NOT attempt to access the
    //        packet!  Doing so could cause a memory exception!
    //
    // Parameters:
    //      pPacket - the packet to be released.
    //
    // Returns:
    //      true if the packet was released.
    //      false if the packet was not released.
    //
    inline bool ReleasePacket(BcmEthernetPacket *pPacket);


public:  // Configuration/management API

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

    // Accessor for the command table object for this interface.  This is
    // created/destroyed by the most derived class, but stored in the base
    // class.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A pointer to the command table object for this interface, or NULL
    //      if there isn't one.
    //
    inline BcmCommandTable *CommandTable(void);

    // This sets the Packet Switch instance.  When a packet is received from the
    // HAL, it will be processed and then sent to this Packet Switch.
    //
    // Parameters:
    //      pPacketSwitch - the Packet Switch instance to whom I should send
    //                      packets that I receive from the HAL.
    //      isDefault - set this to true if this instance is being added as
    //                  the default HalIf.
    //
    // Returns:  Nothing.
    //
    void SetPacketSwitch(BcmPacketSwitch *pPacketSwitch,
                         bool isDefault = false);

    // Returns a pointer to the Packet Switch instance to which I am registered.
    // This can return NULL if I am not registered with one.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The Packet Switch instance to which I am registered.
    //
    inline BcmPacketSwitch *GetPacketSwitch(void);

    // This method enables/disables transmitting from this interface to the
    // specified interface, depending on the enable flag.  This overrides the
    // Can/CantBeEnabled/Disabled flag settings.
    //
    // Parameters:
    //      halIf - the destination HalIf to which transmitting should be
    //              enabled/disabled.
    //      enable - true to enable transmit, false to disable.
    //
    // Returns:  Nothing.
    //
    void EnableTransmitTo(const BcmHalIf &halIf, bool enable);

    // Calling this allows this interface to transmit to all other interfaces
    // except itself, regardless of the Can/CantBeEnabled/Disabled settings.
    // This is a quick way to enable access to all interfaces from this one.
    //
    // This was added for PR2258.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void EnableTransmitToAllOtherInterfaces(void);

    // Calling this keeps this interfaces from transmitting to all other
    // interfaces, regardless of the Can/CantBeEnabled/Disabled settings.  This
    // is a quick way to disable access to all interfaces.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void DisableTransmitToAll(void);

    // Calling this allows all other interfaces to transmit to this interface,
    // regardless of the Can/CantBeEnabled/Disabled settings.  It has to iterate
    // through all of the interfaces registered with the PacketSwitch to do
    // this.
    //
    // This was added for PR2258.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void EnableReceiveFromAllOtherInterfaces(void);

    // Calling this keeps other interfaces from transmitting to this interface,
    // regardless of the Can/CantBeEnabled/Disabled settings.  It has to iterate
    // through all of the interfaces registered with the PacketSwitch to do
    // this.
    //
    // This was added for PR1905.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void DisableReceiveFromAll(void);

    // Tells whether or not this interface is allowed to transmit to the
    // specified desination HalIf.
    //
    // Parameters:
    //      halIf - the destination HalIf to be tested.
    //
    // Returns:
    //      true if transmit is allowed.
    //      false if transmit is NOT allowed.
    //
    inline bool IsTransmitEnabled(const BcmHalIf &halIf) const;

    // Tells whether or not this HalIf instance is the default interface with
    // the packet switch.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if it is the default interface.
    //      false if it is not the default interface.
    //
    inline bool IsDefaultInterface(void) const;

    // Enables/disables packet logging for packets sent to this interface by
    // the specified interface.
    //
    // PR1880 - reversed the sense of this to support the packet logging Snoop.
    // Previously, we would enable logging from this interface to the specified
    // interface, but because the packet logging Snoop is registered with the
    // outbound interface, we needed to reverse the direction of enabling.
    //
    // Parameters:
    //      enable - true to enable, false to disable.
    //      halIf - the source interface.
    //
    // Returns:  Nothing.
    //
    void EnablePacketLogging(bool enable, const BcmHalIf &halIf, bool verbose=true);

    // Tells whether or not packet logging is enabled for packets sent to this
    // by the specified interface.
    //
    // PR1880 - reversed the sense of this to support the packet logging Snoop.
    // Previously, we would enable logging from this interface to the specified
    // interface, but because the packet logging Snoop is registered with the
    // outbound interface, we needed to reverse the direction of enabling.
    //
    // Parameters:
    //      halIf - the source interface.
    //
    // Returns:
    //      true if packet logging is enabled for the interface.
    //      false if packet logging is disabled.
    //
    inline bool IsPacketLoggingEnabled(const BcmHalIf &halIf) const;

    // Accessor for the interface number that was assigned to this instance.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The interface number that was assigned.
    //
    inline unsigned int InterfaceNumber(void) const;
    
    // These are the standard short names for various types of interfaces.
    // They map to 4 character text names that have an index value of 0.  You
    // can find the non-0 instance by simply adding the index to it.
    enum
    {
        // Hardware LAN interfaces (may be used for WAN, but usually not).
        kEth0       = 0x65746830,   // 'eth0' Ethernet interface
        kUsb0       = 0x75736230,   // 'ubs0' USB interface
        kWlan0      = 0x776c6e30,   // 'wln0' 802.11 Wireless LAN interface
        kWlan1      = 0x776c6e31,   // 'wln1' 802.11 Wireless LAN interface
        kWlan2      = 0x776c6e32,   // 'wln2' 802.11 Wireless LAN interface
        kWlan3      = 0x776c6e33,   // 'wln3' 802.11 Wireless LAN interface
        kWlan4      = 0x776c6e34,   // 'wln4' 802.11 Wireless LAN interface
        kWlan5      = 0x776c6e35,   // 'wln5' 802.11 Wireless LAN interface
        kWlan6      = 0x776c6e36,   // 'wln6' 802.11 Wireless LAN interface
        kWlan7      = 0x776c6e37,   // 'wln7' 802.11 Wireless LAN interface
        kHpna0      = 0x68706e30,   // 'hpn0' HPNA interface
        kBluetooth0 = 0x626c7530,   // 'blu0' Bluetooth interface
        kPci0       = 0x70636930,   // 'pci0' PCI interface
        kScb0       = 0x62736230,   // 'scb0' PCI interface

        // Hardware WAN interfaces.
        kCbl0       = 0x63626c30,   // 'cbl0' DOCSIS Cable Modem interface
        kDsl0       = 0x64736c30,   // 'dsl0' DSL interface
        kDavic0     = 0x64617630,   // 'dav0' DAVIC interface

        // Software interfaces.
        kIp0        = 0x69702030,   // 'ip 0' IP stack interface
        kPktGen0    = 0x706b7430,   // 'pkt0' Packet Generator interface
        kSwLoopback0= 0x6c6f2030,   // 'lo 0' Software Loopback interface

        kCpe0       = 0x63706530,   // 'cpe0' Arbitrary CPE interface; used
                                    // mainly for virtual/software interfaces
                                    // not tied to hardware.

        kWan0       = 0x77616e30,   // 'wan0' Arbitrary WAN interface; any of
                                    // the above can be renamed to this as
                                    // needed.  This is used for Firewall rules
                                    // so that we don't need to know the kind of
                                    // interface that is used for the WAN on
                                    // each product.
        
        kHplg0       = 0x68706730,   // 'hpg0' Arbitrary HomePlug interface; any of
                                    // the above can be renamed to this as
                                    // needed.  This is used for Firewall rules
                                    
        kPktSwBr0    = 0x70736230   // 'psb0' For PacketSwitchBridgingHalIf
    };

    // Accessor for the short name that was given for this instance.  This can
    // be just any old number, but most often you want to give it a 4-char
    // ASCII name like 'eth1' or 'usb0', something that can be read by humans.
    //
    // Parameters:
    //      name - the name value to set.
    //
    // Returns:
    //      The short name.
    //
    inline uint32 ShortName(void) const;
    inline void ShortName(uint32 name);

    // Accessor for the description that was given for this instance.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The description string.
    //
    inline const char *Description(void) const;
    
    // Prints the HalIf's settings to the specified ostream.
    //
    // Parameters:
    //      outstream - the ostream to which the settings should be printed.
    //
    // Returns:
    //      A reference to the ostream for operator chaining.
    //
    ostream & Print(ostream &outstream) const;

    // Gives access to my message log settings instance.  This is mainly
    // intended to be used by command tables to change my severity levels and
    // enable/disable logging.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A reference to my message log settings.
    //
    inline BcmMessageLogSettings &MessageLogSettings(void);

public:

    // If the device has a CAM, this will cause it to add the specified MAC
    // address to it.  In general, the default interface is the only one that
    // is not in promiscuous mode, and that needs to have it's CAM programmed.
    //
    // Parameters:
    //      address - the MAC address to be added to the CAM.
    //
    // Returns:
    //      true if successful (or if the device doesn't have a CAM).
    //      false if the address couldn't be added (CAM is full, etc.).
    //
    virtual bool AddAddressToCam(const BcmMacAddress &address);

    // If the device has a CAM, this will cause it to remove the specified MAC
    // address from it.
    //
    // Parameters:
    //      address - the MAC address to be removed from the CAM.
    //
    // Returns:
    //      true if successful (or if the device doesn't have a CAM).
    //      false if the address couldn't be removed (not found, etc).
    //
    virtual bool RemoveAddressFromCam(const BcmMacAddress &address);

    // If the device has a reject CAM, this will cause it to add the specified
    // MAC address to it.  This is helpful to filter out local traffic in
    // hardware, since software would have to filter it out anyway.
    //
    // This is for PR1332.
    //
    // Parameters:
    //      address - the MAC address to be added to the reject CAM.
    //
    // Returns:
    //      true if successful (or if the device doesn't have a reject CAM).
    //      false if the address couldn't be added (CAM is full, etc).
    //
    virtual bool AddAddressToRejectCam(const BcmMacAddress &address);

    // If the device has a reject CAM, this will cause it to remove the
    // specified MAC address from it.
    //
    // This is for PR1332.
    //
    // Parameters:
    //      address - the MAC address to be removed from the reject CAM.
    //
    // Returns:
    //      true if successful (or if the device doesn't have a reject CAM).
    //      false if the address couldn't be removed (not found, etc).
    //
    virtual bool RemoveAddressFromRejectCam(const BcmMacAddress &address);

public:

    // Accessor for the ifTable index number that was assigned to this instance.
    // Note that this is a logical value and, generally, will vary from the 
    // InterfaceNumber used by the forwarder.  The reason that this is necessary
    // is that DOCSIS requires the following indices in the ifTable (per
    // OSS-N-99020
    //
    // CMCI: 1
    // CM MAC: 2
    // CM DS: 3
    // CM US: 4
    // other: 4+n
    // 
    // Unfortunately, the OS often assigns an ifnumber to the IP stack, usually
    // 1.  So this conflicts with the DOCSIS requirement and makes it necessary
    // for us to assign logical ifnumbers for use by the ifTable to satisfy
    // DOCSIS requirements.
    //
    // Parameters:
    //      pAgent - the SNMP agent that my interface is being querried for.
    //
    // Returns:
    //      The interface number that was assigned.
    //
    virtual unsigned int IfTableIndex(const BcmSnmpAgent *pAgent = NULL) const;
    
    // Accessor for the ifDescr value, which is a string describing the
    // interface.
    //
    // The default action taken by the base class is to return the value in
    // Description(), which is the name of the interface for debugging
    // purposes.
    //
    // Parameters:
    //      interfaceNumber - the interface number being querried; this is
    //                        primarily for subinterface support.
    //
    // Returns:
    //      The ifDescr string.
    //
    virtual const char *IfDescr(int interfaceNumber = 0) const;

    // Accessor for the iana type that was assigned to this instance.
    //
    // Parameters:
    //      interfaceNumber - the interface number being querried; this is
    //                        primarily for subinterface support.
    //
    // Returns:
    //      The iana type that was assigned.
    //
    inline unsigned int IanaType(int interfaceNumber = 0) const;
    
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
    virtual bool IfPhysAddress (const BcmMacAddress &ifPhysAddress, int interfaceNumber = 0);

    // Codes for IfAdminStatus and IfOperStatus.
    typedef enum 
    {
        kUp = 1,            // ready to pass packets
        kDown = 2,
        kTesting = 3,       // in some test mode
        kUnknown = 4,       // status can not be determined for some reason.
        kDormant = 5,
        kNotPresent = 6,    // some component is missing
        kLowerLayerDown = 7 // down due to state of lower-layer interface(s)
    } IfStatus;

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

    virtual unsigned int IfCounterDiscontinuityTime(int interfaceNumber = 0) const;

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
    virtual unsigned int IfInUnknownProtos(int interfaceNumber = 0) const;
    inline void IncrementIfInUnknownProtos(void);

    virtual unsigned int IfOutDiscards(int interfaceNumber = 0) const;
    virtual unsigned int IfOutErrors(int interfaceNumber = 0) const;
    
    // Accessor for the ifSpecific object
    //
    // Parameters:
    //      interfaceNumber - the interface number being querried; this is
    //                        primarily for subinterface support.
    //
    // Returns:
    //      An object ID formatted as a C string.
    //      EG: "1.3.6.1"
    //
    virtual const char *IfSpecific (int interfaceNumber = 0) const;

    // Gives access to my interface counters object.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A pointer to the interface counters object.
    //
    virtual inline BcmInterfaceCounters *InterfaceCounters(void);
    
    // HAL info accessors for the BcmHalIf base class are essentially 
    // non-functional, since there is no HAL characteristics structure here
    // for us to communicate with the hardware by.  However, we need these
    // methods here so that code can make calls to BcmHalIf derived objects
    // without needing to cast them as BcmNiHalIf's
    virtual unsigned int GetMibCounter(unsigned int MibOID) {return 0;};
    virtual unsigned int GetMibCounter(unsigned int MibOID, unsigned int Argument) {return 0;};
    virtual bool SetMibCounter (unsigned int MibOID, unsigned int Value) {return false;};
    virtual bool SetMibCounter (unsigned int MibOID, unsigned int Argument, unsigned int Value) {return false;};
    virtual int GetMibInteger(unsigned int MibOID) {return 0;};
    virtual int GetMibInteger(unsigned int MibOID, unsigned int Argument) {return 0;};
    virtual bool SetMibInteger (unsigned int MibOID, int Value) {return false;};
    virtual bool SetMibInteger (unsigned int MibOID, unsigned int Argument, int Value) {return false;};
    virtual int GetMibOctetBuffer (unsigned int MibOID, unsigned char *pBuf, int MaxSize) {return 0;};
    virtual int GetMibOctetBuffer (unsigned int MibOID, unsigned char *pBuf, int MaxSize, unsigned int Argument) {return 0;};
    virtual int GetMibOctetBuffer (unsigned int MibOID, BcmString &Buf, unsigned int Argument) {return 0;};
    virtual int GetMibOctetBuffer (unsigned int MibOID, BcmString &Buf) {return 0;};
    virtual bool SetMibOctetBuffer (unsigned int MibOID, unsigned char *pValue, int Size) {return false;};
    virtual bool SetMibOctetBuffer (unsigned int MibOID, unsigned char *pValue, int Size, unsigned int Argument) {return false;};
    virtual bool SetMibOctetBuffer (unsigned int MibOID, const BcmString &Buf, unsigned int Argument) {return false;};
    virtual bool SetMibOctetBuffer (unsigned int MibOID, const BcmString &Buf) {return false;};
    

public:  // PR1880 - added all methods in this group.

    typedef enum
    {
        kInbound,
        kOutbound,
        kInAndOutbound
    } SnoopDirection;

    // Adds a Snoop to the specified list of inbound and/or outbound Snoops. 
    // The list will be sorted on descending priority.  Note that if the Snoop
    // is already registered, then it will not be registered again.
    //
    // Parameters:
    //      pSnoop - the Snoop to be registered.
    //      direction - the direction that the Snoop is to be registered.
    //
    // Returns:  Nothing.
    //
    void AddSnoop(BcmSnoop *pSnoop, SnoopDirection direction);

    // Causes the specified list of inbound and/or outbound Snoops to be sorted.
    // This is called by AddSnoop(), and can also be called by an external
    // client (such as when the priority of a Snoop has changed).
    //
    // Parameters:
    //      direction - the list of Snoops to be sorted.
    //
    // Returns:  Nothing.
    //
    void SortSnoops(SnoopDirection direction);

    // Checks to see if the specified Snoop has been registered in the specified
    // inbound and/or outbound list.
    //
    // Parameters:
    //      pSnoop - the Snoop to be checked.
    //      direction - the list of Snoops searched.
    //
    // Returns:
    //      true if the specified Snoop has been registered.
    //      false if the specified Snoop has not been registered.
    //
    bool SnoopIsRegistered(BcmSnoop *pSnoop, SnoopDirection direction) const;

    // If the specified Snoop has been registered in the specified inbound
    // and/or outbound list, then this method removes it from the list.  The
    // Snoop is not deleted.
    //
    // Parameters:
    //      pSnoop - the Snoop to be removed.
    //      direction - the list of Snoops searched.
    //
    // Returns:  Nothing.
    //
    void RemoveSnoop(BcmSnoop *pSnoop, SnoopDirection direction);

    // Returns the first Snoop that is registered with this interface for the
    // specified direction; if there are no Snoops, it returns NULL.  Only the
    // kInbound and kOutbound directions are valid; if you specify
    // kInAndOutbound, it will operate on the kInbound list.
    //
    // Parameters:
    //      direction - the list of Snoops to be searched.
    //
    // Returns:
    //      Pointer to the first Snoop, or NULL.
    //
    BcmSnoop *FirstSnoop(SnoopDirection direction) const;

    // Returns the next Snoop in order that is registered with this interface for the
    // specified direction; if there are no Snoops, it returns NULL.  Only the
    // kInbound and kOutbound directions are valid; if you specify
    // kInAndOutbound, it will operate on the kInbound list.
    //
    // Parameters:
    //      direction - the list of Snoops to be searched.
    //      pSnoop - set to NULL for first Snoop, and then send in the
    //      last snoop returned from this call in order to return the next snoop.
    //
    // Example Usage: pSnoop = FirstSnoop(BcmHalIf::kInbound);
    //                pSnoop = NextSnoop(BcmHalIf::kInbound,pSnoop);
    //
    // Returns:
    //      Pointer to the Snoop following the pSnoop sent in, or NULL.
    //
    BcmSnoop *NextSnoop(SnoopDirection direction, BcmSnoop *pSnoop) const;
    
    // Returns the last Snoop that is registered with this interface for the
    // specified direction; if there are no Snoops, it returns NULL.  Only the
    // kInbound and kOutbound directions are valid; if you specify
    // kInAndOutbound, it will operate on the kInbound list.
    //
    // Parameters:
    //      direction - the list of Snoops to be searched.
    //
    // Returns:
    //      Pointer to the last Snoop, or NULL.
    //
    BcmSnoop *LastSnoop(SnoopDirection direction) const;

public:

    // Sets the mask bit.  This is usually only called by the Packet Switch
    // with which I am registered.
    //
    // The mask bit can contain any combination of bits (and can even be 0), but
    // it generally only makes sense if a single bit is set.
    //
    // Parameters:
    //      maskBit - the mask bit I should use.
    //
    // Returns:  Nothing.
    //
    inline void MaskBit(uint32 maskBit);

    // Returns my mask bit.  This is usually only called by other HalIf
    // instances.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The mask bit for this instance.
    //
    inline uint32 MaskBit(void) const;

    // This configures a bitmask of interfaces that should never be enabled,
    // even when told to.  This is done to make it easier to use the helper
    // functions EnableReceiveFromAllOtherInterfaces() and
    // EnableTransmitToAllOtherInterfaces(), since there are often interfaces
    // that we don't want to have enabled.
    //
    // When adding the interface to the list, this method will automatically
    // disable transmit to it.
    //
    // Parameters:
    //      halIf - the destination HalIf of interest.
    //      addToList - true to add this HalIf to the "never enable" list;
    //                  false to remove it from the list.
    //
    // Returns:  Nothing.
    //
    void NeverEnableTransmitTo(const BcmHalIf &halIf, bool addToList);

    // Tells whether or not NeverEnableTransmitTo() was set for the specified
    // destination HalIf.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if NeverEnableTransmitTo is enabled.
    //      false if NeverEnableTransmitTo is not enabled.
    //
    inline bool IsNeverEnableTransmitToEnabled(const BcmHalIf &halIf);
    
public:
    // Events which we publish for interested subscribers.  Note that if
    // derived classes extend this list, care must be taken not to duplicate
    // the enum values.
    typedef enum
    {
      kLinkDown = 0,
      kLinkUp
    } EventCode;

    // Subscribe for status event notification.
    //
    // \param
    //      Code - event code for status event of interest, from enum above
    //
    // \param
    //      pACT - pointer to object derived from BcmCompletionHandlerACT. 
    //          BcmCompletionHandlerACT implements the callback interface.  
    //          The derived class implements HandleEvent() in a manner which
    //          dispatches the event to the client object.
    //
    void SubscribeEventNote (unsigned int EventCode, BcmCompletionHandlerACT* pACT);
    
    //
    // End subscription for status event notification.
    //
    // \param
    //      Code - event code for status event of interest, from enum above
    //
    // \param
    //      pACT - pointer to object derived from BcmCompletionHandlerACT. 
    //          BcmCompletionHandlerACT implements the callback interface.  
    //          The derived class implements HandleEvent() in a manner which
    //          dispatches the event to the client object.
    //
    void UnSubscribeEventNote (unsigned int EventCode, BcmCompletionHandlerACT* pACT);

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
                                    BcmHalIf *pSrcHalIf) = 0;

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
    virtual bool ReleasePacketImpl(BcmEthernetPacket *pPacket) = 0;

    // Gives the derived class a chance to print its settings to the specified
    // ostream.
    //
    // Parameters:
    //      outstream - the ostream to which the settings should be printed.
    //
    // Returns:
    //      A reference to the ostream for operator chaining.
    //
    virtual ostream & PrintImpl(ostream &outstream) const;

protected:

    // These are the application-specific message log severity bits.
    enum
    {
        kTxMessages = 0x00000100,
        kRxMessages = 0x00000200,
        kTxData =     0x00000400,
        kRxData =     0x00000800,
        kSFlows =     0x00001000,
        kMibFilters = 0x00002000,
        kDsScan =     0x00004000,
        kOtherInfo =  0x40000000
    };

    // For controlling message log output.
    BcmMessageLogSettings fMessageLogSettings;

    // The command table object created by the most derived class.
    BcmCommandTable *pfCommandTable;

    // The Packet Switch instance with whom I have been registered.  When I
    // receive a packet from my HAL, I will send it to him after I process it.
    BcmPacketSwitch *pfPacketSwitch;

    // This is the object that wraps the low-level structure and packet that is
    // sent to us by the driver.
    BcmEthernetPacket *pfEthernetPacket;

    // Lets a HalIf class know whether or not it is the "default" interface
    // with the packet switch (forwarder) instance.
    bool fIsDefaultInterface;

    // Other state information.  These generally don't change after they are set
    // in the constructor.
    IanaInterfaceType fInterfaceType;
    unsigned int fInterfaceNumber;
    uint32 fShortName;
    const char *pfDescription;

    // Counters for this interface.  These used to be in the drivers themselves,
    // but now that we have pure s/w interfaces without drivers, we need to keep
    // the counters here.  This also shrinks a bit of code and gets rid of
    // duplicate code.
    BcmInterfaceCounters fInterfaceCounters;

    // These flags and bitmasks all control various aspects of forwarding and
    // logging capabilities of the HalIf classes.  Each HalIf gets its own
    // unique mask bit, and given the mask bit of some other HalIf, it can be
    // asked if forwarding or logging is enabled between the two.
    uint32 fMyMaskBit;

    uint32 fCanTransmitToMask;
    uint32 fLoggingEnabledMask;
    uint32 fNeverEnableTransmitMask;

    // A counter for ifInUnknownProtos MIB object.  This is updated by the
    // BcmIfUnknownProtosSnoop.
    unsigned int fIfInUnknownProtos;

    // PR1880 - added to store the Snoops that are registered with me.
    vector<BcmSnoop *> fInboundSnoops;
    vector<BcmSnoop *> fOutboundSnoops;

    // The desired state of the interface.  This is stored rather than just
    // making a driver call because we could have AdminStatus = up and OperStatus
    // = down, for example if we want ethernet turned on but there is no active
    // link.
    IfStatus fAdminStatus;

    // The actual state of the interface.  This is a combination of the desired
    // state of the interface (fAdminStatus) and the actual h/w state.
    IfStatus fOperStatus;
    
    // Store a MAC address for this interface.  This allows us to accomodate
    // the preference of various applications which might have different
    // expactations of what the value of ifPhysAddress should be.
    BcmMacAddress fPhysAddress;
    
    // Event publisher
    BcmEventPublisher fEventPublisher;

    #if (SNMP_SUPPORT)

    // SNMP stuff.
    ifEntryBridge *pfIfTableBridge;

    #endif

private:

    // Default Constructor.  Not supported.
    BcmHalIf(void);

    // Copy Constructor.  Not supported.
    BcmHalIf(const BcmHalIf &otherInstance);

    // Assignment operator.  Not supported.
    BcmHalIf & operator = (const BcmHalIf &otherInstance);
    
    #if (SNMP_SUPPORT)
    
    friend class ifEntryBridge;
    
    #endif
};


//********************** Inline Method Implementations ***********************


// Accessor for the command table object for this interface.  This is
// created/destroyed by the most derived class, but stored in the base
// class.
//
// Parameters:  None.
//
// Returns:
//      A pointer to the command table object for this interface, or NULL
//      if there isn't one.
//
inline BcmCommandTable *BcmHalIf::CommandTable(void)
{
    return pfCommandTable;
}


// This method is called (generally by a forwarder/router Packet Switch
// object) to cause a packet to be sent to the HAL.
//
// PR2257 - the source HalIf is specified now.
//
// PR1880 - When a Snoop needs to send a packet, it will often want to skip
// all of the processing done so far by earlier Snoops.  In this case, the
// Snoop can specify itself as the pSnoop parameter, and processing will
// resume with the Snoop that follows it in the list.  This should only be
// done in cases where there would be recursion if the same Snoop ran on
// its own packet.
//
// Parameters:
//      pPacket - the packet to be sent.
//      pSrcHalIf - the HalIf that is sending the packet to me.
//      pSnoop - the Snoop to resume processing after; this allows
//               preceeding Snoops to be skipped.
//
// Returns:
//      true if the packet was sent.
//      false if there was a problem.
//
inline bool BcmHalIf::TransmitPacket(BcmEthernetPacket *pPacket,
                                     BcmHalIf *pSrcHalIf, BcmSnoop *pSnoop)
{
    vector<BcmSnoop *>::iterator myIterator;
    BcmSnoop::SnoopResult result;
    bool success;

    CallTraceDatapath("BcmHalIf", "TransmitPacket");

    // UNFINISHED - do I also want to allow kTesting?
    if (fAdminStatus != kUp)
    {
        // UNFINISHED - do I want to update a counter?

        return false;
    }

    // Do NOT update interface counters here.  The counters need to be updated
    // when the packet has actually been transmitted, which means that we need
    // to update them when the packet is released back via the interface-
    // specific send-complete method.  This is necessary because a packet might
    // be queued for a significant amount of time before it is sent, and we
    // don't want the packets to show up in the counters before they are
    // transmitted.

    if (pSnoop == NULL)
    {
        myIterator = fOutboundSnoops.begin();
    }
    else
    {
        // Find the starting point.  If pSnoop is not NULL, and it a valid Snoop
        // pointer in the list, then we will start with the first Snoop after it.
        // Otherwise, we will start at the beginning of the list.
        myIterator = find(fOutboundSnoops.begin(), fOutboundSnoops.end(), pSnoop);

        if (myIterator == fOutboundSnoops.end())
        {
            // Snoop not found, so start at the beginning.
            myIterator = fOutboundSnoops.begin();
        }
        else
        {
            // Skip the Snoop that we found.
            ++myIterator;
        }
    }
    
    // Run each of the outbound Snoops that are registered with me, in order
    // (they are already sorted by priority).
    while (myIterator != fOutboundSnoops.end())
    {
        // Run the Snoop, giving a pointer to myself as the destination HalIf,
        // and the HalIf that sent the packet to me as the source HalIf.
        result = (*myIterator)->SnoopOutbound(pPacket, pSrcHalIf, this);

        // If the Snoop says to discard the packet, then stop all processing and
        // return false, which causes the packet to be released back to the
        // sender.
        if (result == BcmSnoop::kDiscardPacket)
        {
            // No counter update here; the packet is being discarded because of
            // "upper layer" filtering, not because of interface resources.

            return false;
        }
        
        // If the Snoop says that it consumed the packet, then stop all
        // processing on the packet and return true, which simulates what would
        // happen if the packet was successfully queued to the destination
        // interface.  The Snoop (or it's client) is responsible for releasing
        // the packet back to the sender when it is done with it.
        else if (result == BcmSnoop::kPacketConsumed)
        {
            // No counter update here; the packet hasn't been sent out the
            // interface.

            return true;
        }

        // If the Snoop says to pass the packet on, then continue with the next
        // Snoop.
        ++myIterator;
    }

    // Vector to the derived class.  If we got this far, then all of the Snoops
    // said to pass the packet, so we need to do derived class processing.
    success = TransmitPacketImpl(pPacket, pSrcHalIf);

    // If we didn't send the packet, then we need to update the discard counter.
    // The assumption is that this could only be because of insufficient
    // resources, the interface is 'down', etc.  In any case, this is a lower
    // layer issue, which is what this counter counts.
    if (!success)
    {
        // PR 5311: only count if we are 'up'.  This means that this counter
        // will rarely (ever?) be incremented.
        if (fOperStatus == kUp)
          fInterfaceCounters.TransmitPacketWasDiscarded();
    }

    return success;
}


// This method is called (generally by the HAL) to cause a packet to be
// processed, and possibly sent to the forwarder/router Packet Switch object.
//
// The HalIf class will process the packet (doing any filtering or other
// processing that is necessary).
//
// PR2412 - changed the parameter from the HAL ETHERNET_PACKET struct to
// the V2 BcmEthernetPacket object, which helps make the BcmHalIf class
// more portable.
//
// PR1880 - When a Snoop needs to send a packet, it will often want to skip
// all of the processing done so far by earlier Snoops.  In this case, the
// Snoop can specify itself as the pSnoop parameter, and processing will
// resume with the Snoop that follows it in the list.  This should only be
// done in cases where there would be recursion if the same Snoop ran on
// its own packet.
//
// Parameters:
//      pPacket - the packet to be processed.
//      pSnoop - the Snoop to resume processing after; this allows
//               preceeding Snoops to be skipped.
//
// Returns:
//      true if successful.
//      false if there was a problem (the packet must be released by the
//          caller).
//
inline bool BcmHalIf::ReceivePacket(BcmEthernetPacket *pPacket, 
                                    BcmSnoop *pSnoop,
                                    bool skipAllSnoops) // PR12290
{
    bool success;

    CallTraceDatapath("BcmHalIf", "ReceivePacket");

    
    // UNFINISHED - do I also want to allow kTesting?
    if (fAdminStatus != kUp)
    {
        // UNFINISHED - do I want to update a counter?

        return false;
    }
    
    // Update the interface counters for this packet here.  The receive
    // counters reflect what actually came in on the interface, not whether or
    // not we decided to discard them at a higher layer.  The CRC32 has been
    // stripped off, so I need to add those bytes back in.
    fInterfaceCounters.PacketReceived(pPacket->PacketLength() + 4,
                                      pPacket->MacDaType());

    // PR12290 - added support for skipping inbound Snoops.
    if (skipAllSnoops == false)
    {
        vector<BcmSnoop *>::iterator myIterator;
        BcmSnoop::SnoopResult result;

        if (pSnoop == NULL)
        {
            myIterator = fInboundSnoops.begin();
        }
        else
        {
            // Find the starting point.  If pSnoop is not NULL, and it a valid Snoop
            // pointer in the list, then we will start with the first Snoop after it.
            // Otherwise, we will start at the beginning of the list.
            myIterator = find(fInboundSnoops.begin(), fInboundSnoops.end(), pSnoop);

            if (myIterator == fInboundSnoops.end())
            {
                // Snoop not found, so start at the beginning.
                myIterator = fInboundSnoops.begin();
            }
            else
            {
                // Skip the Snoop that we found.
                ++myIterator;
            }
        }

        // Run each of the inbound Snoops that are registered with me, in order
        // (they are already sorted by priority).
        while (myIterator != fInboundSnoops.end())
        {
            // Run the Snoop, giving a pointer to myself as the source HalIf.
            result = (*myIterator)->SnoopInbound(pPacket, this);

            // If the Snoop says to discard the packet, then stop all processing and
            // return false, which causes the packet to be released back to the
            // sender.
            if (result == BcmSnoop::kDiscardPacket)
            {
                // Update the counter indicating that we didn't forward the packet.
                // This is used when the packet is discarded because of the upper
                // layer (e.g. filters).
                fInterfaceCounters.ReceivedPacketNotForwarded();

                return false;
            }

            // If the Snoop says that it consumed the packet, then stop all
            // processing on the packet and return true, which simulates what would
            // happen if the packet was successfully queued to the destination
            // interface.  The Snoop (or it's client) is responsible for releasing
            // the packet back to the sender when it is done with it.
            else if (result == BcmSnoop::kPacketConsumed)
            {
                return true;
            }

            // If the Snoop says to pass the packet on, then continue with the next
            // Snoop.
            ++myIterator;
        }
    }

    // Send the packet to the packet switch.  If we got this far, then all of
    // the Snoops decided to pass the packet, so we need to send it on its way.
    success = pfPacketSwitch->TransmitPacket(pPacket, this);

    if (!success)
    {
        // Update the counter indicating that we didn't forward the packet.
        // Something elsewhere caused the packet to be dropped, which is what
        // this counts.
        fInterfaceCounters.ReceivedPacketNotForwarded();
    }

    return success;
}

// This method is called (generally by the HAL) to cause a packet to be
// processed, and possibly sent to the forwarder/router Packet Switch object.
//
// The HalIf class will process the packet (doing any filtering or other
// processing that is necessary).
//
// PR2412 - changed the parameter from the HAL ETHERNET_PACKET struct to
// the V2 BcmEthernetPacket object, which helps make the BcmHalIf class
// more portable.
//
//
// Parameters:
//      mask    - the mask applied to the broadcast or multicast packets
//      pPacket - the packet to be processed.
//      pSnoop - the Snoop to resume processing after; this allows
//               preceeding Snoops to be skipped.
//      skipFirstSnoop - whether to skip the first snoop or not. 
//              Note:
//                Currently L2VPN assumes that the first snoop is the IGMP snoop
//                For Docsis, this is the only snoop on the downstream. If this
//                changes, then we will need to pass in the snoop to be skipped.
//
// Returns:
//      true if successful.
//      false if there was a problem (the packet must be released by the
//          caller).
//
inline bool BcmHalIf::ReceivePacketWithMask(uint32 mask, BcmEthernetPacket *pPacket, bool skipFirstSnoop)
{
    vector<BcmSnoop *>::iterator myIterator;
    BcmSnoop::SnoopResult result;
    bool success;

    CallTraceDatapath("BcmHalIf", "ReceivePacket");

    
    // UNFINISHED - do I also want to allow kTesting?
    if (fAdminStatus != kUp)
    {
        // UNFINISHED - do I want to update a counter?

        return false;
    }
    
    // Update the interface counters for this packet here.  The receive
    // counters reflect what actually came in on the interface, not whether or
    // not we decided to discard them at a higher layer.  The CRC32 has been
    // stripped off, so I need to add those bytes back in.
    fInterfaceCounters.PacketReceived(pPacket->PacketLength() + 4,
                                      pPacket->MacDaType());

    myIterator = fInboundSnoops.begin();
    if (skipFirstSnoop && myIterator != fInboundSnoops.end())
    {
        ++myIterator; // go to the next snoop
    }
    
    // Run each of the inbound Snoops that are registered with me, in order
    // (they are already sorted by priority).
    while (myIterator != fInboundSnoops.end())
    {
        // Run the Snoop, giving a pointer to myself as the source HalIf.
        result = (*myIterator)->SnoopInbound(pPacket, this);

        // If the Snoop says to discard the packet, then stop all processing and
        // return false, which causes the packet to be released back to the
        // sender.
        if (result == BcmSnoop::kDiscardPacket)
        {
            // Update the counter indicating that we didn't forward the packet.
            // This is used when the packet is discarded because of the upper
            // layer (e.g. filters).
            fInterfaceCounters.ReceivedPacketNotForwarded();

            return false;
        }

        // If the Snoop says that it consumed the packet, then stop all
        // processing on the packet and return true, which simulates what would
        // happen if the packet was successfully queued to the destination
        // interface.  The Snoop (or it's client) is responsible for releasing
        // the packet back to the sender when it is done with it.
        else if (result == BcmSnoop::kPacketConsumed)
        {
            return true;
        }

        // If the Snoop says to pass the packet on, then continue with the next
        // Snoop.
        ++myIterator;
    }

    // Send the packet to the packet switch.  If we got this far, then all of
    // the Snoops decided to pass the packet, so we need to send it on its way.
    success = pfPacketSwitch->ForwardBroadcastPacketWithMask(mask, pPacket, this);

    if (!success)
    {
        // Update the counter indicating that we didn't forward the packet.
        // Something elsewhere caused the packet to be dropped, which is what
        // this counts.
        fInterfaceCounters.ReceivedPacketNotForwarded();
    }

    return success;
}


// This method is called (generally by the Packet Switch or another HalIf
// instance) when it has finished using the packet.  This may or may not
// actually release the packet, depending on whether or not anyone else is
// using it.
//
// NOTE:  Once you call this method, you must NOT attempt to access the
//        packet!  Doing so could cause a memory exception!
//
// Parameters:
//      pPacket - the packet to be released.
//
// Returns:
//      true if the packet was released.
//      false if the packet was not released.
//
inline bool BcmHalIf::ReleasePacket(BcmEthernetPacket *pPacket)
{
    /* Don't do excessive error checking any more.
    if (pPacket == NULL)
    {
        gErrorMsg(fMessageLogSettings, "ReleasePacket")
            << "Packet pointer is NULL!" << endl;

        return false;
    }
    */

    // Vector to the derived class.
    return ReleasePacketImpl(pPacket);
}


// Returns a pointer to the Packet Switch instance to which I am registered.
// This can return NULL if I am not registered with one.
//
// Parameters:  None.
//
// Returns:
//      The Packet Switch instance to which I am registered.
//
inline BcmPacketSwitch *BcmHalIf::GetPacketSwitch(void)
{
    return pfPacketSwitch;
}


// Tells whether or not this HalIf instance is the default interface with
// the packet switch.
//
// Parameters:  None.
//
// Returns:
//      true if it is the default interface.
//      false if it is not the default interface.
//
inline bool BcmHalIf::IsDefaultInterface(void) const
{
    return fIsDefaultInterface;
}


// Tells whether or not packet logging is enabled for packets sent to this
// by the specified interface.
//
// PR1880 - reversed the sense of this to support the packet logging Snoop.
// Previously, we would enable logging from this interface to the specified
// interface, but because the packet logging Snoop is registered with the
// outbound interface, we needed to reverse the direction of enabling.
//
// Parameters:
//      halIf - the source interface.
//
// Returns:
//      true if packet logging is enabled for the interface.
//      false if packet logging is disabled.
//
inline bool BcmHalIf::IsPacketLoggingEnabled(const BcmHalIf &halIf) const
{
    if (fLoggingEnabledMask & halIf.MaskBit())
    {
        return true;
    }

    return false;
}


// Accessor for the interface number that was assigned to this instance.
//
// Parameters:  None.
//
// Returns:
//      The interface number that was assigned.
//
inline unsigned int BcmHalIf::InterfaceNumber(void) const
{
    return fInterfaceNumber;
}


// Accessor for the iana type that was assigned to this instance.
//
// Parameters:
//      interfaceNumber - the interface number being querried; this is
//                        primarily for subinterface support.
//
// Returns:
//      The iana type that was assigned.
//
inline unsigned int BcmHalIf::IanaType(int interfaceNumber) const
{
    return fInterfaceType;
}


// Accessor for the short name that was given for this instance.
//
// Parameters:  None.
//
// Returns:
//      The short name.
//
inline uint32 BcmHalIf::ShortName(void) const
{
    return fShortName;
}


inline void BcmHalIf::ShortName(uint32 name)
{
    fShortName = name;
}


// Accessor for the description that was given for this instance.
//
// Parameters:  None.
//
// Returns:
//      The description string.
//
inline const char *BcmHalIf::Description(void) const
{
    return pfDescription;
}


// Gives access to my message log settings instance.  This is mainly
// intended to be used by command tables to change my severity levels and
// enable/disable logging.
//
// Parameters:  None.
//
// Returns:
//      A reference to my message log settings.
//
inline BcmMessageLogSettings &BcmHalIf::MessageLogSettings(void)
{
    return fMessageLogSettings;
}


// Gives access to my interface counters object.
//
// Parameters:  None.
//
// Returns:
//      A pointer to the interface counters object.
//
inline BcmInterfaceCounters *BcmHalIf::InterfaceCounters(void)
{
    return &fInterfaceCounters;
}


// Sets the mask bit.  This is usually only called by the Packet Switch
// with which I am registered.
//
// The mask bit can contain any combination of bits (and can even be 0), but
// it generally only makes sense if a single bit is set.
//
// Parameters:
//      maskBit - the mask bit I should use.
//
// Returns:  Nothing.
//
inline void BcmHalIf::MaskBit(uint32 maskBit)
{
    fMyMaskBit = maskBit;

    // By default, we want to set things up so that packets coming in to this
    // interface can't be forwarded back out this interface.  There are a few
    // cases where that behavior is desirable, but in most cases it is really
    // bad to loop packets back like that.
    fCanTransmitToMask &= ~fMyMaskBit;
    fNeverEnableTransmitMask |= fMyMaskBit;

    // In order to allow packet loopback on the same interface, you must call
    // the following:
    //
    // pHalIf->NeverEnableTransmitTo(*pHalIf, false);
    // pHalIf->EnableTransmitTo(*pHalIf, true);
}


// Returns my mask bit.  This is usually only called by other HalIf
// instances.
//
// Parameters:  None.
//
// Returns:
//      The mask bit for this instance.
//
inline uint32 BcmHalIf::MaskBit(void) const
{
    return fMyMaskBit;
}


// Tells whether or not this interface is allowed to transmit to the
// specified desination HalIf.
//
// Parameters:
//      halIf - the destination HalIf to be tested.
//
// Returns:
//      true if transmit is allowed.
//      false if transmit is NOT allowed.
//
inline bool BcmHalIf::IsTransmitEnabled(const BcmHalIf &halIf) const
{
    if ((fCanTransmitToMask & halIf.MaskBit()) != 0)
    {
        return true;
    }

    return false;
}


// Tells whether or not NeverEnableTransmitTo() was set for the specified
// destination HalIf.
//
// Parameters:  None.
//
// Returns:
//      true if NeverEnableTransmitTo is enabled.
//      false if NeverEnableTransmitTo is not enabled.
//
inline bool BcmHalIf::IsNeverEnableTransmitToEnabled(const BcmHalIf &halIf)
{
    if ((fNeverEnableTransmitMask & halIf.MaskBit()) != 0)
    {
        return true;
    }

    return false;
}


inline void BcmHalIf::IncrementIfInUnknownProtos(void)
{
    fIfInUnknownProtos++;
}


inline ostream & operator << (ostream &outstream, const BcmHalIf &halIf)
{
    return halIf.Print(outstream);
}


#endif


