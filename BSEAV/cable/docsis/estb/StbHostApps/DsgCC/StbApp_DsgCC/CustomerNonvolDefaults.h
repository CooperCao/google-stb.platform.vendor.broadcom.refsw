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
//  $Id$
//
//  Filename:       CustomerNonvolDefaults.h
//  Author:         David Pullen
//  Creation Date:  March 25, 2002
//
//****************************************************************************
//  Description:
//      This file contains the default values that are to be used for nonvol
//      settings.  This file controls the behavior of the settings (hard-coded
//      vs use nonvol) as well as specifying the default values.
//
//      This file needs to be modified for each customer so that it reflects
//      their unique settings, and then the entire code set needs to be rebuilt
//      so that they take effect.
//
//****************************************************************************

#ifndef CUSTOMERNONVOLDEFAULTS_H
#define CUSTOMERNONVOLDEFAULTS_H

//********************** Include Files ***************************************

//#include "BspConstants.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

// +----------------+
// | Snoop priority |
// +----------------+
//
// A Snoop is an object that is registered with 0 or more interfaces (USB,
// ethernet, etc), and has an opportunity to look at/process/modify/discard
// the packet.  Inbound Snoops are called before the packet is sent to the
// forwarder, and Outbound Snoops are called after the packet comes from the
// forwarder, but before being sent to the interface driver.
//
// Snoop objects are stored in a list (an STL vector, to be precise), which is
// sorted on decreasing priority.  Thus, a Snoop with a higher priority will be
// run before one with a lower priority.
//
// Because different systems can have different requirements for the ordering
// of Snoops, we allow the default priority to be set here.
//

// Packet logging is done on the outbound side of each interface (when enabled).
// We generally want it to run last so that it shows the packet that is actually
// being sent to the driver, and only if prior Snoops don't drop the packet or
// otherwise consume it.
#define kSnoopPriority_Outbound_PacketLogging 0

// The Spanning Tree Protocol Filter drops STP packets (this is a DOCSIS
// requirement).  It is registered with the inbound interface of CPE ports.
// Because it drops all STP packets, we want it to run early on so that other
// processing isn't wasted.
#define kSnoopPriority_Inbound_StpFilter 256

// The IP Routing Filter Snoop looks for packets destined to an IP stack (with
// the stack's MAC DA) or coming from an IP stack (with the stack's MAC SA).  If
// the IP address doesn't match the stack's address, then the packet was routed
// (or will be routed) by the stack, and we must discard it.  This only runs on
// the IP stacks associated with the Cable Modem (public and private stacks).
// This is necessary because packets to/from the CM's public stack get special
// treatment, like bypassing filters, max CPE, and NACO, and we don't want CPEs
// using the routing features in the stack to get free access to the WAN.
// PR4263
#define kSnoopPriority_Inbound_IpRoutingFilter 1
#define kSnoopPriority_Outbound_IpRoutingFilter 1

// The dot1dStaticSnoop is an outbound snoop registered on all interfaces
// in the ifTable which has a real HAL (that is, the HAL is registered with
// a packet switch - no stub HALs like SW loopback).  This also means that any
// HAL that doesn't show up in the ifTable (like IP stacks) won't get the snoop.
// It provides destination MAC address filtering per the dot1dStaticTable (see
// RFC-1493).  It might drop the packet, and it runs fast compared to the 
// DOCSIS MIB filters, so lets run it before the MIB filters.
#define kSnoopPriority_Outbound_Dot1dStatic 48

// The MIB Filter is an outbound Snoop registered with the CPE interfaces and
// the WAN/DOCSIS CM interface.  It runs the MIB filters for the src/dest pair.
#define kSnoopPriority_Outbound_DocsisMibFilter 32

// The IGMP/Mcast Snoop is both inbound and outbound registered with the WAN/CM
// interface.  It handles the BPI+ stuff for encrypted multicast streams.
// UNFINISHED - should this be before or after MIB filters?
#define kSnoopPriority_Inbound_DocsisMcastIgmp 64
#define kSnoopPriority_Outbound_DocsisMcastIgmp 64

// The WPA-NAS is an inbound snoop on the 802.11 interface only.  It should run
// after the MIB filters so we can use the filters to trap EAP packets if we
// choose to.
#define kSnoopPriority_Inbound_WpaNas 16

// The NetToMedia Snoop is both inbound and outbound registered with IP stack
// interfaces.  It handles learning IP addresses in the SNMP NetToMedia table.
// Since the IP stacks don't have MIB filter Snoops associated with them, we
// can use the same priority.
#define kSnoopPriority_Inbound_NetToMedia 32
#define kSnoopPriority_Outbound_NetToMedia 32

// The IfUnknownProtos snoop counts packets heading to an IP stack which are 
// of 'unknown protocol' type.  It never changes or drops a packet, just counts.
// So it can go late.  16, why not?
#define kSnoopPriority_Outbound_IfUnknownProtos 16

// The DhcpDiscoverDirect Snoop is inbound registered with WAN-MAN
// WAN-DATA IP interfaces.  It will only send DHCP discover messages
// to the default Halif, and not to any other interface. It needs
// to be one of the last snoops, because it can send the packet
// directly to an interface (skipping other snoops).  
#define kSnoopPriority_Inbound_DhcpDiscoverDirect 2

// The DHCP Client Snoop is outbound on all IP HAL interfaces; it looks for
// packets from a DHCP server that are destined to the IP stack, and routes
// them to the V2 DHCP Client thread for processing.
// PR4263 - changed this from 1 to 2.
#define kSnoopPriority_Outbound_DhcpClient 2

// The DhcpServer Snoop is outbound registered with 192.168.0.1 IP HAL
// interface.  It handles listening for DHCP discovers from the Lan-Side 
// network.
#define kSnoopPriority_Outbound_DhcpServerSnoop 120


// The IpForwDatagrams snoop simply counts all IP packets which are bridged from
// LAN to WAN or vice-versa.  It exists as an inbound and outbound snoop on the
// default HAL interface, whatever that is for a given packet switch.  It will
// never change or drop a packet, and we don't want to count packets which are
// dropped, so let's run it pretty late.  Why not 16?
#define kSnoopPriority_Inbound_IpForwDatagrams 16
#define kSnoopPriority_Outbound_IpForwDatagrams 16

// The Nat Snoop is inbound and outbound registered with CM HAL
// interface.  It handles address translation.
// NOTE:  on the downstream, NAT is done FIRST automatically because
// the FIREWALL is applied on the outbound LAN interface.  On the UPSTREAM,
// we want NAT to be AFTER the firewall, so the Outbound Priority needs to be
// LOWER than the FW
#define kSnoopPriority_Outbound_NatSnoop 100
#define kSnoopPriority_Inbound_NatSnoop  120

// The DhcpcOffer Snoop is inbound registered with CM HAL
// interfaces.  It handles learning IP addresses by gleaning 
// DHCP DISCOVER/REQUEST/OFFERS/ACK messages
// from/to the DHCP server.
// This snoop needs to run before NAT in both directions 
#define kSnoopPriority_Inbound_DhcpcOfferSnoop  130
#define kSnoopPriority_Outbound_DhcpcOfferSnoop 130

// The Dynamic NAT Snoop is outbound on the WAN interface (usually CM HalIf).
// It handles dynamic mapping of CPE addresses to public addresses in NAT mode
// (not NAPT mode).  It needs to run before NAT, so it is a higher priority.
#define kSnoopPriority_Outbound_DynamicNat 110

// The NAT MAC Address Spoofing Snoop needs to run before NAT on the downstream
// (inbound), meaning it needs to be a higher priority.  It needs to run after
// NAT on the upstream (outbound), meaning it needs to be a lower priority.
#define kSnoopPriority_Outbound_NatMacAddrSpoof kSnoopPriority_Outbound_NatSnoop -1
#define kSnoopPriority_Inbound_NatMacAddrSpoof  kSnoopPriority_Inbound_NatSnoop +1

// The Firewall Snoop is outbound registered with ALL HalIfs.
// It handles packet filtering and SPI.
#define kSnoopPriority_Outbound_FirewallSnoop 140
#define kSnoopPriority_Inbound_FirewallSnoop  110 

// The HttpContentFilter Snoop is inbound and outbound registered with CM HAL
// interface.  It handles HTTP content filtering and domain blocking.  Note that
// on the outbound side, it has to see the packet BEFORE ANY address translations so
// that it can spoof the response back to the CPE address for blocked sites.
#define kSnoopPriority_Outbound_HttpContentFilterSnoop 130
#define kSnoopPriority_Inbound_HttpContentFilterSnoop 1

// The Routing Subnet Snoop is inbound on all the LAN and IP stack interfaces.
// This snoop needs to run before all the other inbound snoops.
#define kSnoopPriority_Inbound_RoutedSubnet kSnoopPriority_Inbound_FirewallWanLanSnoop+1

// The Wan Routing Subnet Snoop is inbound and outbound on the default Halif.
// This snoop needs to run after the Nat snoop on outbound packets, and
// before the Nat Snoop on inbound packets..
#define kSnoopPriority_Inbound_WanRoutedSubnet kSnoopPriority_Inbound_NatSnoop + 10
#define kSnoopPriority_Outbound_WanRoutedSubnet kSnoopPriority_Outbound_NatSnoop - 10

// The USFS Snoop is inbound on all the LAN and IP stack interfaces.
// The USFS Snoop will route traffic to CPE and Stacks which are on different subnets
#define kSnoopPriority_Inbound_Usfs 64

// The lanNatSnoop will call the NAT code when necessary.
// This snoop needs to run after the USFS snoop
#define kSnoopPriority_Inbound_lanNatSnoop 60

// The NAT Relay Snoop is inbound on all LAN interfaces.
// It helps with LAN->WAN traffic performance by skipping routing by the IP
// stack whenever possible.  It needs to run after USFS.
#define kSnoopPriority_Inbound_NatRelay 63

// The Rg Static snoop is an snoop on all inbound LAN devices. This snoop will drop packets which 
// are not within the specified range and are being sent to the WAN.
#define kSnoopPriority_Inbound_RgStatic 0

// The FireWallLan Snoop will isolate the LAN from the WAN when it is enabled. It
// should be disabled when the firewall file is downloaded or we are running in RG mode
#define kSnoopPriority_Inbound_FirewallWanLanSnoop   150
#define kSnoopPriority_OutBound_FirewallWanLanSnoop  150

#define kSnoopPriority_InBound_CtpSnoop   65
#define kSnoopPriority_OutBound_CtpSnoop  65

// The ARP snoop runs on the OutBound of the WAN-MAN, WAN-DATA, and CableHome Stack
// It needs to run before the firewall snoop, because potentially the packet
// doesn't need to be sent to a stack. 
#define kSnoopPriority_Outbound_Arp 130

// The ARP Filter Snoops runs on the Outbound of the RG Ip Stack instances.
// It will assure that ARP Requests for Ip Addresses that the Ip Stack is not
// bound to will be dropped.
#define kSnoopPriority_Outbound_ArpFilter 130

// The ManageSwitch Snoop will run OutBound on the Ethernet ManageSwitch port.
// The Snoop is responsibe for making sure that a broadcasted packet will get
// to one of the switch interfaces.
#define kSnoopPriority_Outbound_ManageSwitch  200

// The DropBroadcast Snoop will drop broadcasted packets from
// the WAN to LAN. The snoop will assure that there are no
// pass-through addresses before dropping the packets.
#define kSnoopPriority_OutBound_DropBroadcastSnoop 150

// The Upnp Snoop will run on each of the LAN interfaces looking for
// frames that are related to Upnp discovery and provisioning. 
#define kSnoopPriority_Inbound_UpnpSnoop 1


// +--------------------+
// | NonVol memory size |
// +--------------------+
//
// If non-0, these values configure the number of bytes of flash memory will be
// reserved for Permanent and Dynamic nonvol memory.  The flash driver will
// set aside enough flash blocks to hold at least this many bytes.
//
// NOTE:  Changing the default value can cause the amount of flash memory
//        reserved for Image1 and Image2 to be reduced.  It may also cause
//        Permanent Nonvol to be placed in a different location, moving it from
//        the boot block (before Image1) to after Image1.
//
#define kDefaultValue_PermanentNonvolSizeBytes 0
#define kDefaultValue_DynamicNonvolSizeBytes   0

// +---------------------------------+
// | Message Logging NonVol Settings |
// +---------------------------------+
// 
// The customer can change the "CPE Access" IP stack address and subnet to suit
// the needs of their MSO.  Most of the time, you will want to use the
// 192.168.100.1 address and the default subnet mask.  If each of your MSOs
// have different values that must be used, then this needs to be a
// manufacturing item associated with a MIB.
//
// For the router address, if the OS is pSOS, then you will need to leave the
// address set to 0.0.0.0; otherwise it will need to be set to an on-net address.
// 
// None of the other settings can be modified.

// These are all IP addresses in string values.
#define kDefaultValue_RemoteAccessIpAddress     "192.168.100.1"
#define kDefaultValue_RemoteAccessSubnetMask    "255.255.255.0"

#if defined(TARGETOS_vxWorks)
#define kDefaultValue_RemoteAccessRouterAddress "192.168.100.254"
#else
#define kDefaultValue_RemoteAccessRouterAddress "0.0.0.0"
#endif

#define kDefaultValue_RemoteAccessHttpUserId        ""
#define kDefaultValue_RemoteAccessHttpUserPassword  "Broadcom"
#define kDefaultValue_RemoteAccessHttpAdminId       ""
#define kDefaultValue_RemoteAccessHttpAdminPassword "aDm1n$TR8r"

// Unsigned integer value from 2-4.  Other values are reserved and must not
// be used.  Typically, you will want to set this to 2.  This controls the
// stack affinity in cases where multiple stacks have the same subnet, and
// tells the CPE Access agent which stack to initialize with the IP address
// and subnet specified above.  For the EMTA/Packet Cable build, this needs to
// be 3, since the EMTA uses stack 2.
#define kDefaultValue_RemoteAccessIpStackNumber 2

// PR4517 - new default controls and values for the telnet server.  The telnet
// server provides access to the console via telnet.  The telnet server is an
// optional feature that is compile in by default for fat images, and compiled
// out by default for slim images, though this can be overriden via build
// options "telnet" and "notelnet".  The following default values are used if
// the telnet server is compiled in to the image.
//
// This determines whether the telnet server will be enabled or not at startup.
// If not enabled, it can be started at runtime via the serial console or other
// hook (we will likely implement a MIB to do this).  This is a boolean value.
#define kDefaultValue_TelnetServerEnabled false

// These set the default values for the user name and password that must be
// entered when logging in via telnet.  Again, we may implement MIBs to control
// these; if an empty string is used, then you can just hit enter when prompted
// to log in.  They are both strings, up to 15 characters.
#define kDefaultValue_TelnetUsername ""
#define kDefaultValue_TelnetPassword ""

// This is the bitmask of IP stacks that the telnet server will listen for
// connections over.  The telnet server must create a TCP socket and bind it
// to the IP address for each stack in order for clients to connect.  There are
// 8 IP stacks currently available.  Use values kTelnetIpStackX, logically ORed
// together, where X is a value 1-8.
#define kDefaultValue_TelnetIpStackEnable (kTelnetIpStack1 | kTelnetIpStack2)

// +-----------------------+
// | HalIf NonVol Settings |
// +-----------------------+
 
// The customer needs to specify the default dummy MAC addresses for the IP
// stacks and USB.  These items need to be manufactured into the modem (via
// MIBs), but there should be reasonable defaults.  Mainly, we need to know the
// customer's vendor id, which comprise the first 3 bytes of the MAC addresses.
// 
// The customer MUST specify the USB Vendor and Device IDs, so they match the
// USB driver.

// This is a string value, corresponding to the first 3 bytes of the MAC
// address.  This is for Broadcom Corporation.
#define kCustomerVendorId "00:10:18"

// PR5058 - Previously, we always used "de:ad:xx" as the last part of the
// default MAC addresses.  However, one of our customers pointed out that this
// range of MAC addresses actually is valid; in fact everything from 00:00:00
// to ff:ff:ff is valid for each OUI (kCustomerVendorId).  Different customers
// may have different values that are reserved for the special "not yet
// manufactured" values, and we want to allow them to specify this range rather
// than being stuck with our "de:ad:xx" range.
//
// This is the value that will now be used to determine the default MAC address
// for IP stack 1.  Other addresses will be generated by incrementing from this
// value (we will generate at least 10 addresses beyond this one).  This is a
// string value, and will be used like this:
//
//     macAddress.Set(kCustomerVendorId ":" kMacAddressBase);
//
#define kMacAddressBase "de:ad:01"

// These are both 16-bit hex numbers.  Broadcom Corporation uses 0x0a5c for the
// vendor id, and our devices all report themselves as the chip ID (the same USB
// driver can be used on any modem).
#define kDefaultValue_UsbVendorId       0x0a5c
#if (PROGRAM_STORE_SIGNATURE == 0x3348) || \
    (PROGRAM_STORE_SIGNATURE == 0x3350) || \
    (PROGRAM_STORE_SIGNATURE == 0x3351) || \
    (PROGRAM_STORE_SIGNATURE == 0x3352) || \
    (PROGRAM_STORE_SIGNATURE == 0x3360)
#define kDefaultValue_UsbDeviceId       PROGRAM_STORE_SIGNATURE
#else
#define kDefaultValue_UsbDeviceId       0x3345
#endif
#define kDefaultValue_UsbVendorName     "Broadcom Corporation"
#define kDefaultValue_UsbProductName    "USB Cable Modem"

// These set the interfaces that are enabled or disabled by default.  At a
// minimum, the DocsisCm, IP1 and IP2 interfaces must be enabled.  Others are
// optional.  These are all bool values.
#define kDefaultValue_EthernetEnabled    true
#define kDefaultValue_HpnaEnabled        false
#define kDefaultValue_UsbEnabled         true
#define kDefaultValue_IpStack3Enabled    false
#define kDefaultValue_IpStack4Enabled    false
#define kDefaultValue_BlueToothEnabled   false
#define kDefaultValue_802_11Enabled      false
#define kDefaultValue_IpStack5Enabled    false
#define kDefaultValue_IpStack6Enabled    false
#define kDefaultValue_IpStack7Enabled    false
#define kDefaultValue_IpStack8Enabled    false
#define kDefaultValue_WanEthernetEnabled false

// Sets the Board Rev, which is used by various drivers to determine what h/w
// is present or not (for h/w that can't be detected automatically).  See
// BspConstants.h for legal values.
#define kDefaultValue_BoardRev      kHalIf_BoardRev_BCM93345A

// Parameters for the CM Interface.  You can select the tuner, amp, reference
// frequencies, annex mode, etc.  See external documentation for legal values
// and the appropriate use.  These are all unsigned integers.
#define kDefaultValue_CmTuner       kHalIf_CmTuner_BCM3416
#define kDefaultValue_CmAmp         kHalIf_CmAmp_BCM_Internal
#define kDefaultValue_AnnexMode     kHalIf_CmAnnex_B

#define kDefaultValue_DsRefFreq     24500000
#define kDefaultValue_UsRefFreq     28000000
#define kDefaultValue_PhyRefFreq    43750000
#define kDefaultValue_UsSampleFreq  432000000       // Added for PR3576

// More CM HAL parameters, added for PR4111 (which seems to have vanished from
// GNATs).  These obscure names match some wacky registers in the h/w; if you
// don't know what they mean, then you probably have no business reading or
// writing new values anyway...
//
// These default values came from Ray.
//
// PR5610 - new values from Ray for the 3348/3416.  The old values are
// commented out.
#define kDefaultValue_DsAgi     0x0d        // (was 0x08)       // uint8 0..255
#define kDefaultValue_DsAgt     0x1d        // (was 0x38)       // uint8 0..255
#define kDefaultValue_Stathr    0x0480                          // uint16 0..65535
#define kDefaultValue_Stagi     0xa8000000  // (was 0x38000000) // uint32 0..2^32
#define kDefaultValue_Stpga1    0x020042    // (was 0x000000)   // uint32 0..2^24
#define kDefaultValue_Stagt     0xfff00000  // (was 0xe0000000) // uint32 0..2^32
#define kDefaultValue_Stabw1    0x0d0b                          // uint16 0..65535
#define kDefaultValue_Stabw2    0x0a09                          // uint16 0..65535

// Buffer allocation settings - all network interfaces use these buffers for
// DMA.  Depending on the type and number of interfaces (and performance or
// memory constraints), you may want to tune these values.  See external
// documentation for reasonable values and tuning guidelines.
//
#if BCM_CABLEHOME_SUPPORT

    // This is the new way to specify the BcmAlloc configration.  You tell it the
    // number of different buffer sizes, the size of each buffer, as well as the
    // number of buffers of each size.
    //
    // Here's the reason for the different buffer sizes:
    //
    //  16 - used for the DOCSIS MAC Header/ehdr for upstream packets
    //  48 - used for the MAC_TX_HDR struct for upstream packets
    //  64 - IP stack; small packets.
    //  288 - Stores DOCSIS MAC Header/ehdr for downstream DMA
    //  1536 - USB Receive DMA
    //  1632 - CM Downstream MAC/Data DMA, Ethernet Receive DMA, 802.11 Receive DMA
    //
    // UNFINISHED - need to profile for real numbers!
    #if (HPNA_HAL_INCLUDED || WIFI80211_HAL_INCLUDED)

        // If HPNA and/or 802.11 is included, we need additional buffer sizes:
        //
        //  1024, 1904, 2048 - don't know how/why these are used.
        //
        // PR5740 - changed the 1904 buffer size to 1920 (increased by one cache
        // line) to accommodate the new 802.11 driver.
        //
        #define kDefaultValue_NumberOfBufferSizes 9
        #define kDefaultValue_BufferSizes     {  16,   48,   64,  288, 1024, 1536, 1632, 1920, 2048}
        #define kDefaultValue_NumberOfBuffers { 350,  350,  128,  200,   64,   80,  700,  256,    8}

    #else

        // No HPNA support.
        #define kDefaultValue_NumberOfBufferSizes 6
        #define kDefaultValue_BufferSizes     {  16,   48,   64,  288, 1536, 1632}
        #define kDefaultValue_NumberOfBuffers { 350,  350,  128,  150,   80,  700}

    #endif


    // PR4822 - these are the current "best" values for a CableMedea/RG CM.
    //
    // NOTE:  These values are no longer directly used, but MUST remain.  The nonvol
    //        corresponding to these is still present (in order to support
    //        downgrading to older code) and must be populated with reasonable
    //        values.
    #define kDefaultValue_BufferSize16   256
    #define kDefaultValue_BufferSize32   512
    #define kDefaultValue_BufferSize64   512
    #define kDefaultValue_BufferSize128  200
    #define kDefaultValue_BufferSize256  200
    #define kDefaultValue_BufferSize512  0
    #define kDefaultValue_BufferSize1024 200
    #define kDefaultValue_BufferSize1600 400
    #define kDefaultValue_BufferSize2048 425

#elif EMTA_SUPPORT

    // This is the new way to specify the BcmAlloc configration.  You tell it the
    // number of different buffer sizes, the size of each buffer, as well as the
    // number of buffers of each size.
    //
    // Here's the reason for the different buffer sizes:
    //
    //  16 - used for the DOCSIS MAC Header/ehdr for upstream packets
    //  48 - used for the MAC_TX_HDR struct for upstream packets
    //  64 - IP stack; small packets.
    //  288 - Stores DOCSIS MAC Header/ehdr for downstream DMA
    //  1536 - USB Receive DMA
    //  1632 - CM Downstream MAC/Data DMA, Ethernet Receive DMA, 802.11 Receive DMA
    //
    // UNFINISHED - need to profile for real numbers!
    #if (HPNA_HAL_INCLUDED || WIFI80211_HAL_INCLUDED)

        // If HPNA and/or 802.11 is included, we need additional buffer sizes:
        //
        //  1024, 1904, 2048 - don't know how/why these are used.
        //
        // PR5740 - changed the 1904 buffer size to 1920 (increased by one cache
        // line) to accommodate the new 802.11 driver.
        //
        #define kDefaultValue_NumberOfBufferSizes 9
        #define kDefaultValue_BufferSizes     {  16,   48,   64,  288, 1024, 1536, 1632, 1920, 2048}
        #define kDefaultValue_NumberOfBuffers { 350,  350,  128,  200,   64,   80,  700,  256,    8}

    #else

        // No HPNA support.
        #define kDefaultValue_NumberOfBufferSizes 6
        #define kDefaultValue_BufferSizes     {  16,   48,   64,  288, 1536, 1632}
        #define kDefaultValue_NumberOfBuffers { 350,  350,  128,  150,   80,  700}

    #endif


    // PR4822 - these are the current "best" values for an EMTA/CM.
    //
    // NOTE:  These values are no longer directly used, but MUST remain.  The nonvol
    //        corresponding to these is still present (in order to support
    //        downgrading to older code) and must be populated with reasonable
    //        values.
    #define kDefaultValue_BufferSize16   256
    #define kDefaultValue_BufferSize32   512
    #define kDefaultValue_BufferSize64   512
    #define kDefaultValue_BufferSize128  100
    #define kDefaultValue_BufferSize256  100
    #define kDefaultValue_BufferSize512  0
    #define kDefaultValue_BufferSize1024 150
    #define kDefaultValue_BufferSize1600 375
    #define kDefaultValue_BufferSize2048 400

#elif BCM97110

    // This is the new way to specify the BcmAlloc configration.  You tell it the
    // number of different buffer sizes, the size of each buffer, as well as the
    // number of buffers of each size.
    //
    // Here's the reason for the different buffer sizes:
    //
    //  16 - used for the DOCSIS MAC Header/ehdr for upstream packets
    //  48 - used for the MAC_TX_HDR struct for upstream packets
    //  64 - IP stack; small packets.
    //  288 - Stores DOCSIS MAC Header/ehdr for downstream DMA
    //  1536 - USB Receive DMA
    //  1632 - CM Downstream MAC/Data DMA, Ethernet Receive DMA, 802.11 Receive DMA
    //
        #define kDefaultValue_NumberOfBufferSizes 12
        #define kDefaultValue_BufferSizes     {  16,  32,  64, 128, 256, 512, 1024, 1600, 2048, 4096, 8192, 16384}
        #define kDefaultValue_NumberOfBuffers { 256, 512, 512,  25,  25,   0,   25,  270,  300,  200,   20,    20 }



    // PR4822 - these are the current "best" values for a bridging CM.
    //
    // NOTE:  These values are no longer directly used, but MUST remain.  The nonvol
    //        corresponding to these is still present (in order to support
    //        downgrading to older code) and must be populated with reasonable
    //        values.
    #define kDefaultValue_BufferSize16   256
    #define kDefaultValue_BufferSize32   512
    #define kDefaultValue_BufferSize64   512
    #define kDefaultValue_BufferSize128  25
    #define kDefaultValue_BufferSize256  25
    #define kDefaultValue_BufferSize512  0
    #define kDefaultValue_BufferSize1024 25
    #define kDefaultValue_BufferSize1600 270
    #define kDefaultValue_BufferSize2048 300
    #define kDefaultValue_BufferSize4096 200
    #define kDefaultValue_BufferSize8192 20
    #define kDefaultValue_BufferSize16384 20

#else

    // This is the new way to specify the BcmAlloc configration.  You tell it the
    // number of different buffer sizes, the size of each buffer, as well as the
    // number of buffers of each size.
    //
    // Here's the reason for the different buffer sizes:
    //
    //  16 - used for the DOCSIS MAC Header/ehdr for upstream packets
    //  48 - used for the MAC_TX_HDR struct for upstream packets
    //  64 - IP stack; small packets.
    //  288 - Stores DOCSIS MAC Header/ehdr for downstream DMA
    //  1536 - USB Receive DMA
    //  1632 - CM Downstream MAC/Data DMA, Ethernet Receive DMA, 802.11 Receive DMA
    //
    #if (HPNA_HAL_INCLUDED || WIFI80211_HAL_INCLUDED)

        // If HPNA and/or 802.11 is included, we need additional buffer sizes:
        //
        //  1024, 1904, 2048 - don't know how/why these are used.
        //
        // PR5740 - changed the 1904 buffer size to 1920 (increased by one cache
        // line) to accommodate the new 802.11 driver.
        //
        #define kDefaultValue_NumberOfBufferSizes 9
        #define kDefaultValue_BufferSizes     {  16,   48,   64,  288, 1024, 1536, 1632, 1920, 2048}
        #define kDefaultValue_NumberOfBuffers { 350,  350,  128,  200,   64,   80,  700,  256,    8}

    #else

        // No HPNA support.
        #define kDefaultValue_NumberOfBufferSizes 6
        #define kDefaultValue_BufferSizes     {  16,   48,   64,  288, 1536, 1632}
        #define kDefaultValue_NumberOfBuffers { 350,  350,  128,  150,   80,  700}

    #endif


    // PR4822 - these are the current "best" values for a bridging CM.
    //
    // NOTE:  These values are no longer directly used, but MUST remain.  The nonvol
    //        corresponding to these is still present (in order to support
    //        downgrading to older code) and must be populated with reasonable
    //        values.
    #define kDefaultValue_BufferSize16   256
    #define kDefaultValue_BufferSize32   512
    #define kDefaultValue_BufferSize64   512
    #define kDefaultValue_BufferSize128  25
    #define kDefaultValue_BufferSize256  25
    #define kDefaultValue_BufferSize512  0
    #define kDefaultValue_BufferSize1024 150
    #define kDefaultValue_BufferSize1600 350
    #define kDefaultValue_BufferSize2048 375

#endif

// PR4822 - allow the BcmAlloc settings to be compile-time or run-time.
// By default, they will continue to be run-time, as they always have been.
// This can be overridden in the product-specific nonvol defaults file.
// This is a boolean value.
#define kDefaultValue_BcmAllocIsCompileTime false

// +---------------------------+
// | CM DOCSIS NonVol Settings |
// +---------------------------+
// 
// The customer can taylor the product to be DOCSIS 1.0 only or DOCSIS 1.1,
// depending on how the product is being submitted for certification.
// 
// The customer can also specify the default downstream frequency that will be
// the starting point the first time the modem boots up.

// This is a boolean true/false value.  Set this to true for DOCSIS 1.1
// submissions or false for DOCSIS 1.0 submissions.
#define kDefaultValue_Docsis11Support true

// This is a bool true/false value.  If kDefaultValue_Docsis11Support is false
// (meaning the CM is to operate in pure DOCSIS 1.0 mode), then this determines
// whether or not Fragmentation will be supported.
#define kDefaultValue_Docsis10FragmentationSupport true

// This is a bool true/false value.  If kDefaultValue_Docsis11Support is false
// (meaning the CM is to operate in pure DOCSIS 1.0 mode), then this determines
// whether or not IGMP will be supported.
#define kDefaultValue_Docsis10IgmpSupport false

// This is a bool true/false value. If this flag only applies to DOCSIS 1.0 and
// it will only apply when someone is running IGMP in DOCSIS 1.0 mode. This flag
// will enable/disabled multicast promisc. support when IGMP is enabled.
// If IGMP is disabled in DOCSIS 1.0 mode, multicast promisc is ALWAYS enabled!             
#define kDefaultValue_Docsis10IgmpMulticastPromiscSupport false

// This is a bool true/false value.  If the h/w is capable of DOCSIS 2.0
// advanced phy, then this determines whether or not that hardware will be
// used.
#define kDefaultValue_AdvancedPhySupport false

// This is a bool true/false value.  There is a problem with doing DOCSIS 2.0
// stuff on some 1.x CMTSs (namely Arris).  This determines our mode of
// operation when 2.0 is enabled and supported by h/w.  Setting this to true
// violates the DOCSIS 2.0 spec (we will only do 1.x stuff on 1.x channels, but
// will do 2.0 stuff on 2.0 channels), but allows the CM to register with 1.x
// CMTSs.
//
// This value should be set to true when submitting for 1.x certwaves, but
// should be set to false for 2.0 certwaves.
//
// This is for PR2519.
// PR4259 - changed the default value to false.
#define kDefaultValue_Docsis1xCmtsHackForDocsis20 false

// This is a bool true/false value.  It controls the algorithm that we use to
// select transmit power levels during initial ranging.  The "standard"
// algorithm for this is to start at 8 dBmV and increase power by 3 dBmV during
// each ranging attempt until we receive a RNG-RSP from the CMTS.  This method
// tends to be very slow, especially if the target power is high (40-50 dBmV).
//
// The optimized method is to use a bisection algorithm that tries the middle
// of the power range, steps up half way, then down half way, etc.  There is
// a possible problem with this in that if we start transmitting at power levels
// that are too high, then we risk saturating the lasers in the upstream, and
// clobbering other transmissions during the burst.
//
// It has not yet been shown that using the optimized algorithm actually causes
// problems, it's just a theoretical concern by one our PHY-heads.  If you feel
// that "safety" is more important than speed, then set this value to false and
// we will do the safe thing.
//
// This is for PR2524.
#define kDefaultValue_OptimizedInitialRanging true

// Unsigned integer value from 1..255.  This determines the value of T that will
// be used in the DOCSIS 1.1 Max(T) rate shaping equation.  This value is in
// units of one second.  2 seconds seems to be a good value, but the customer
// may want to change this to 1 second if they are worried about failing the
// ATP.
#define kDefaultValue_RateShapingT 2

// Unsigned integer value from 1..255.  This determines the maximum number of
// packets that will be allowed to "back up" on an UGS flow before we start
// dropping packets.  Note that once we reach 50% of this number, we set the QI
// bit in the UGS header.  Also note that this number is multiplied by the
// number of Grants Per Interval, so if you have 2 subflows (2 Grants Per
// Interval), then we will allow double the number specified, and will set the
// QI bit at the 50% mark of the doubled amount.
#define kDefaultValue_MaxUgsDepth 3

// Unsigned integer value from 2..255.  This determines the maximum number of
// packets that will be allowed in a single concatenated burst.  This primarily
// affects concatenation of small packets; the total concatenation is still
// limited by MaxBurst/MaxConcatBurst/UCD limitation for number of bytes.
#define kDefaultValue_MaxConcatPackets 128

// +----------------------+
// | SNMP NonVol Settings |
// +----------------------+
// 
// Most of these values are hard-coded in the build, and are not stored in
// nonvol at all.
// 
// The only value that is unique per modem is the docsDevSerialNumber, which
// must be manufactured via a MIB.

// If true, then manufacturing (via the MIBs) will be allowed by default.  If
// false, then manufacturing is disabled and must be enabled via some other
// mechanism.
#define kDefaultValue_AllowConfig false

// String, preferably 32 characters or less (for backward compatibility).
#define kDefaultValue_VendorName "Broadcom"

// String, preferably 128 characters or less (for backward compatibility).
#define kDefaultValue_SysDescr "V2 cablemodem reference design <<HW_REV: V{TBD} US; VENDOR: Broadcom; BOOTR: {TBD}; SW_REV: {TBD}; MODEL: BCM933xx>>"

// String, preferably 128 characters or less (for backward compatibility).
#define kDefaultValue_SysObjectId "1.3.6.1.4.1.4413.2.1.6"

// String, preferably 128 characters or less (for backward compatibility).
#define kDefaultValue_SysOrId "1.3.6.1.4.1.4413.2.3.1.1.2.1"

// String, preferably 128 characters or less (for backward compatibility).
#define kDefaultValue_SysOrDescr "Broadcom DOCSIS 1.1 cablemodem agent version 2"

// String, preferably 128 characters or less (for backward compatibility).
#define kDefaultValue_DocsDevSwCurrentVers "{TBD}"

// This is an 8-bit hex number.
#define kDefaultValue_SysServices 0x4f

#define kDefaultValue_SysContact  "(unknown)";
#define kDefaultValue_SysName     "CableModem";
#define kDefaultValue_SysLocation "(unknown)";

// Number of event log entries
#define kDefaultValue_EventLogMaxEntries   32

// +-------------------------------+
// | SNMP NonVol Settings for EMTA |
// +-------------------------------+

// String, preferably 128 characters or less (for backward compatibility).
#define kDefaultValue_EmtaSysDescr     kDefaultValue_SysDescr
// String, preferably 128 characters or less (for backward compatibility).
#define kDefaultValue_EmtaSysObjectId  "1.3.6.1.4.1.4413.2.1.7"
#define kDefaultValue_EmtaSysContact   "Eugene Nechamkin"
#define kDefaultValue_EmtaSysName      "MTA.broadcom.com"
#define kDefaultValue_EmtaSysLocation  "Richmond, BC, Canada"

// +----------------------+
// | CM DHCP Server Settings |
// +----------------------+
#define kDefaultValue_CmDhcpServerLanIpAddress "192.168.100.1"
#define kDefaultValue_CmDhcpServerLanPoolStart "192.168.100.10"
#define kDefaultValue_CmDhcpServerLanPoolEnd "192.168.100.41"
#define kDefaultValue_CmDhcpServerLanSubnetMask "255.255.255.0"
#define kDefaultValue_CmDhcpServerLeaseTime 30 // in seconds

// +----------------------+
// | CMP NonVol Settings |
// +----------------------+
// 
// Most of these values are hard-coded in the build, and are not stored in
// nonvol at all.
// 
// This is an 8-bit hex number.
#define kCableHome_DefaultValue_SysServices 0x3     // Internet Gateway  

// String, preferably 128 characters or less (for backward compatibility).
#define kCableHome_DefaultValue_SysDescr "V2 cablemodem reference design <<HW_REV: V{TBD} US; VENDOR: Broadcom; BOOTR: {TBD}; SW_REV: {TBD}; MODEL: BCM933xx>>"

// String, preferably 128 characters or less (for backward compatibility).
#define kCableHome_DefaultValue_SysObjectId "1.3.6.1.4.1.4413.2.1.6"

#define kCableHome_DefaultValue_EventLogMaxEntries   64


// +------------------------+
// | 802.11 NonVol Settings |
// +------------------------+

// NOTE:  These all used to be _Cm80211, but they really don't have anything
// to do with a Cable Modem.  In our efforts to have the names make sense, we
// renamed these to _WiFi80211.  However, because vendors may have already
// created their nonvol defaults files based on the previous names, we need to
// warn them of this so that they know to change their files.  This is done in
// makeapp.bat, and causes a warning to be displayed when the build is starting.

// The default SSID value to be stored in nonvol.  This is a string.
#define kDefaultValue_WiFi80211Ssid                   "BROADCOM"

// The default channel number for 802.11.  This is an unsigned integer.
#define kDefaultValue_WiFi80211ChannelNumber          0

// The default interval (in secs?  millisecs?) between beacon transmissions.
#define kDefaultValue_WiFi80211BeaconInterval         100

#define kDefaultValue_WiFi80211MfgIdList1  {0x01,0x02,0xd0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define kDefaultValue_WiFi80211MfgIdList2  {0x02,0x02,0x74,0x00,0x0b,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define kDefaultValue_WiFi80211ProdIdList1 {0x02,0x04,0x06,0x04,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#define kDefaultValue_WiFi80211ProdIdList2 {0x01,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}

#define kDefaultValue_WiFi80211CSMap                  0x34 

#define kDefaultValue_WiFi80211OnBoard                0x00
#define kDefaultValue_WiFi80211ManufacturerId         0x02d0
#define kDefaultValue_WiFi80211ProductId              0x0406
#define kDefaultValue_WiFi80211VendorId               0x14e4
#define kDefaultValue_WiFi80211DeviceId               0x4301
#define kDefaultValue_WiFi80211BoardRevision          0x70
#define kDefaultValue_WiFi80211DefaultOem             {0,0,0,0,0,0,0,0}
#define kDefaultValue_WiFi80211MaxPower               0x42
#define kDefaultValue_WiFi80211TempTable              {0x45,0xfb,0x67,0xd0,0x35,0xf5,0x46,0xc4,0x4d,0x12}
#define kDefaultValue_WiFi80211ChipRevision           0x0001

#define kDefaultValue_WiFi80211BasicRates             0x0003
#define kDefaultValue_WiFi80211RTSThreshold           2347
#define kDefaultValue_WiFi80211FragThreshold          2346
#define kDefaultValue_WiFi80211SupportedRates         0x000f
#define kDefaultValue_WiFi80211EncryptionMode         0
#define kDefaultValue_WiFi80211Authentication         1
#define kDefaultValue_WiFi80211DtimInterval           1
#define kDefaultValue_WiFi80211TxPowerLevel           1
#define kDefaultValue_WiFi80211DefaultKey             1

#define kDefaultValue_WiFi80211ShortRetryLimit        7
#define kDefaultValue_WiFi80211LongRetryLimit         4

#define kDefaultValue_WiFi80211aChannelNumber         36
#define kDefaultValue_WiFi80211gMode                  5
#define kDefaultValue_WiFi80211WirelessEnable         true
#define kDefaultValue_WiFi80211AvailableAntennas      0x03
#define kDefaultValue_WiFi80211AntennaGain            0x00

#define kDefaultValue_WiFi80211gProtection            1
#define kDefaultValue_WiFi80211gRate                  0
#define kDefaultValue_WiFi80211OutputPower            100

#define kDefaultValue_WiFi80211WpaGroupRekeyInterval  0
#define kDefaultValue_WiFi80211RadiusServerIp         "0.0.0.0"
#define kDefaultValue_WiFi80211RadiusPort             1812
#define kDefaultValue_WiFi80211Country                  6
#define kDefaultValue_WiFi80211NetworkAuthentication    0
#define kDefaultValue_WiFi80211SharedKeyAuth            0
#define kDefaultValue_WiFiFrameBursting                 1
#define kDefaultValue_WiFi80211BoardFlags               0x000f
// TRW - use dynamic location by default (0 = default, 1 = permanent, 2 = dynamic)
#define kDefaultValue_WiFi80211LocationCountry          2


// +--------------------------------------------------+
// | CM CableHome/Residential Gateway NonVol Settings |
// +--------------------------------------------------+

// Enables which CableHome mode we are running in 0 = Disabled 1 - CableHome1.0
#define kDefaultValue_CableHomeMode 1

// Enables/disables residential gateway features.  This is a boolean value
#define kDefaultValue_RgEnable false

// WAN-blocking: for ICSA we default this to TRUE
#define kDefaultValue_WanBlockingEnabled true

// The default for whether we get a Wan-Man DHCP address in RG Passthrough Mode        
#define kDefaultValue_RgPassthroughWanDhcpEnabled false


// The default password for managing the RG settings via HTTP.  This is a string.
#define kDefaultValue_RgPassword "Broadcom"

// The default realm name.  This is a string.
#define kDefaultValue_RgRealmName "Broadcom"

// The default start IP address to be served by the DHCP server.  This is an IP
// address in a string.
#define kDefaultValue_CdsLanPoolStart "192.168.0.10"
#define kDefaultValue_CdsLanPoolEnd "192.168.0.254"
#define kDefaultValue_CdsLanSubnetMask "255.255.255.0"
#define kDefaultValue_CdsLanPoolSize 253
#define kDefaultValue_CdsDomainName ""

// The default number of SNMP cabhCapCapMappingEntries to be displayed. This number
// can be capped to be less that the actual session table for performance and attack reasons. 
#define kDefaultMaxCapMappingEntries  128 

//
// UPnP (Universal Plug-n-Play) and CH 1.1 Device Discovery Strings  
// 
#define kDefaultValue_UpnpEnabled       false
#define kProductManufacturerUrl         "http://www.broadcom.com"
#define kProductModelUrl                "http://www.broadcom.com" 
#define kProductModelUpc                "0"
#define kPsDevDeviceType                "CableHome Residential Gateway"


//
// Default RIP and Routed Subnet setting
//
#define kDefaultValue_RipEnabled false
#define kDefaultValue_RipRoutedSubnetEnabled   false
#define kDefaultValue_DhcpRoutedSubnetEnabled  false


#define kDefaultValue_FirewallFileSize (32 * 1024)

// this is the maximum size that the filewall file can be
#define kMAXFIREFILESIZE (32 * 1024)

#define kDefaultValue_RipSendInterval   30  // In seconds
#define kDefaultValue_HttpRemoteAccessPort   8080

// +--------------------------------------------------+
// | PARENTAL CONTROL Settings                        |
// +--------------------------------------------------+
// The default password for unblocking web pages. This is a string.
#define kDefaultValue_ParentalCtrlPassword "Broadcom"

// Default CCID: Customer contract identifier to establish parental control service.. 
#define kDefaultValue_ParentalCtrlCCID "1111-2222-3333-4444"

// Primary category name server. 
#define kDefaultValue_PrimaryCategoryNameServer "http://www.broadcom.com"

// Secondary category name server. 
#define kDefaultValue_SecondaryCategoryNameServer "http://www.broadcom.com"

// Default category list. Always place a comma to the end.
#define kDefaultValue_CategoryList "Abortion,Alcohol,Criminal Skills,Cult,Occult,Drugs,Glamour,Gambling,Hacking,Hate,Hate Site,Hate Speech,Lingerie,Mature Content,Nudism and Naturism,Personals and Dating,Pornography,Tobacco,Violence,Weapons,Sexual Education,Sexual Advice,Sexual Orientation,Finance and Investing,Job Search,News,Shopping,Sports,Travel," // Leave comma at the end

// Subscription URL.
#define kDefaultValue_SubscriptionURL "http://www.broadcom.com"

// +--------------------------------------------------+
// | BPI Settings                                     |
// +--------------------------------------------------+
#define kDefaultValue_BpiMfgVendorName kDefaultValue_VendorName

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************

//********************** Inline Method Implementations ***********************

#include "EstbNonVolDefaults.h"

#endif


