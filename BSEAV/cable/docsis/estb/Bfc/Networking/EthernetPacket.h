//****************************************************************************
//
// Copyright (c) 2000-2009 Broadcom Corporation
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
//
//  Filename:       EthernetPacket.h
//  Author:         David Pullen
//  Creation Date:  March 15, 2000
//
//****************************************************************************
//  Description:
//      This is a wrapper class for various forms of ethernet packets.  The
//      underlying data structures come from the HAL (ETHERNET_PACKET).  This
//      class is able to present the fields of the packet in a more usable form,
//      allowing get/set operations on them.
//
//      In general, you will hand an ETHERNET_PACKET to this class, which will
//      parse it into the data members.  Then, you can use the accessors to read
//      the fields out of the packet.
//
//      NOTE:  This class does not have the ability to create its own
//             ETHERNET_PACKET!  You must give it one before you can operate on
//             it!  This is especially important when using operator = ...
//
//      The accessors have both get and set versions.  The "gets" are fast
//      inline methods that simply return references to the data members of this
//      class.  The "sets" are real method calls that not only update the data
//      members, but also write through to the underlying ETHERNET_PACKET
//      memory, and potentially invalidate other members!
//
//      NOTE:  Before calling a "get" accessor, you must call the Parse...()
//             method for that group; e.g. before calling DsapParsed(), you
//             must call ParseLlc().  The Parse...() methods do the work of
//             parsing the packet data and storing them in the class members.
//
//             The "get" accessor names were changed to ensure that the client
//             is aware of this, though there is nothing that enforces that the
//             client has actually called Parse...().
//
//      This class handles all of the buffer chaining aspects, allowing clients
//      to treat the separate physical buffers a a single logical buffer.
//
//      See ftp://ftp.ed.ac.uk/pub/EdLAN/provan-enet and "TCP/IP Illustrated" 
//      by Stevens for more information on Ethernet and IP frame formats.
//
//****************************************************************************

#ifndef ETHERNETPACKET_H
#define ETHERNETPACKET_H

//********************** Include Files ***************************************

#include "typedefs.h"

#include "IpAddress.h"
#include "MacAddress.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmHalIf;

// These are actually in BfcHal.h, but I hate exposing the V2 app to
// HAL-specific structures.  Declare a forward reference here to satisfy
// the compiler; anyone who actually cares about what ETHERNET_PACKET is
// will have to include BfcHal.h themselves, which is more reasonable.
extern "C" {

struct _ETHERNET_PACKET_;
typedef struct _ETHERNET_PACKET_ ETHERNET_PACKET;

}

//********************** Class Declaration ***********************************


class BcmEthernetPacket
{
public:

    // Default Constructor.  Sets the ETHERNET_PACKET pointer to NULL, and sets
    // all of the members to 0's.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmEthernetPacket(void);

    // Destructor.  Sets the ETHERNET_PACKET pointer to NULL (doesn't delete
    // it!), and cleans up everything else.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmEthernetPacket();

    // Causes the fields of the specified packet to be parsed into the data
    // members.
    //
    // If a packet was previously stored, then the pointer to it is returned (we
    // don't delete it).  You must then release/free it yourself.
    //
    // Parameters:
    //      pPacket - the packet to be parsed.
    //
    // Returns:  Nothing.
    //
    void SetPacket(ETHERNET_PACKET *pPacket);

    // Causes the currently stored packet (if any) to be released, and my data
    // members are reset to an initial state.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A pointer to the packet that was previous owned by this class, or
    //      NULL if there wasn't one.
    //
    ETHERNET_PACKET *ReleasePacket(void);

    // Allows access to the underlying ETHERNET_PACKET structure, primarily so
    // that it can be sent to a HAL.
    //
    // NOTE - do not modify the packet contents!  This will cause a loss of
    //        sync between the members of this class and the data buffer!
    //
    // Parameters:  None.
    //
    // Returns:
    //      A pointer to the packet.
    //
    inline ETHERNET_PACKET *PacketStruct(void);

    // Allows access to the underlying buffer, primarily so that it can be
    // used/examined directly.
    //
    // NOTE - do not modify the packet contents!  This will cause a loss of
    //        sync between the members of this class and the data buffer!
    //
    // PR12290 - inlined this method.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A pointer to the buffer.
    //
    inline const uint8 *Buffer(void) const;

    // Allows a client to query/set the total number of bytes in the packet,
    // starting at the MAC DA, going through the end of the PDU, but not
    // inluding the CRC32 at the end.
    //
    // This is useful because the packet may not exist in a single buffer.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The total packet length.
    //
    inline unsigned int PacketLength(void) const;
    void SetPacketLength(unsigned int length);

    // Allows the client to set/get the usage count associated with the packet.
    // Since the same packet can be sent to multiple interfaces, it can only
    // be released once the usage count goes to so.
    //
    // Parameters:
    //      count - the usage count to set.
    //
    // Returns:
    //      The current usage count.
    //
    void UsageCount(unsigned int count);
    unsigned int UsageCount(void) const;
    void DecrementUsageCount(void);
    void IncrementUsageCount(void);

    // Tells whether or not the packet has been modified since the last time
    // this method was called.  This is usually used for debugging, where we
    // only want to print the packet contents if the packet changed from the
    // last time it printed.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the packet has been changed since the last time this method
    //          was called.
    //      false if the packet has not been changed.
    //
    inline bool PacketChanged(void);

    // This dumps the packet out in a human-readable form.  This is not a const
    // method because it needs to parse the buffer (if not already parsed) in
    // order to print.
    //
    // Parameters:
    //      outStream - the ostream to which the packet should be printed.
    //
    // Returns:
    //      A reference to the ostream to support operation chaining.
    //
    ostream &Print(ostream &outStream, bool verbose=true);
    

public:

    // Accessors for various Ethernet Header fields.  Note that some fields are
    // not valid for some frame formats.  The caller must first check the frame
    // format (via GetFrameFormat()) in order to know whether or not they should
    // check the fields below.  Reading/writing fields that are not valid for
    // the current frame format will result in unpredictable results.
    
    // Pre-parses the LLC header so that you can call the ...Parsed() accessors.
    inline void ParseLlc(void);

    // Returns the LLC header length. 
    inline unsigned int EthernetHeaderLengthParsed(void);

    typedef enum
    {
        kUnicast,
        kMulticast,
        kBroadcast
    } MacAddrType;

    // Returns the type of the MAC DA.
    //
    // Parameters:  None.
    //
    // Returns:
    //      One of the MacAddrType enum values.
    //
    inline MacAddrType MacDaType(void) const;

    // Checks to see if the current packet has a unicast MAC DA.  This is much
    // faster than other comparisons using the actual MAC DA object.  A packet
    // is unicast if it isn't broadcast and it isn't multicast, i.e. the lsb of
    // the first byte of the MAC DA is 0.
    //
    // This is actually calculated in SetPacket() and the accessor that sets
    // the MAC DA.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the packet has a unicast MAC DA.
    //      false if the packet does not have a unicast MAC DA.
    //
    inline bool IsUnicast(void) const;

    // Checks to see if the current packet has a multicast MAC DA, but is NOT
    // broadcast (a broadcast packet is a type of multicast).  A packet is
    // considered to be multicast by this method if the lsb of the first byte
    // of the MAC DA is 1 AND the MAC DA is not broadcast (ff:ff:ff:ff:ff:ff).
    //
    // This is actually calculated in SetPacket() and the accessor that sets
    // the MAC DA.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the packet has a multicast MAC DA.
    //      false if the packet does not have a multicast MAC DA.
    //
    inline bool IsMulticast(void) const;

    // Checks to see if the current packet has a broadcast MAC DA.  A packet is
    // considered to be broadcast if the MAC DA is ff:ff:ff:ff:ff:ff.
    //
    // This is actually calculated in SetPacket() and the accessor that sets
    // the MAC DA.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the packet has a broadcast MAC DA.
    //      false if the packet does not have a broadcast MAC DA.
    //
    inline bool IsBroadcast(void) const;

    // These are the various Ethernet frame formats that we know about.  See the
    // comments at the bottom of this file for details on each one.
    typedef enum
    {
        kUnknown = 0,   // This one means that we don't recognize the contents
                        // or format of the frame, so any other queries will
                        // yield undefined results.
        kEthernetII,
        kRaw802_3,
        kIeee802_2,
        kSnap
    } FrameFormat;

    // this is used when you are constructing an ethernet packet.  
    // If we are starting from scratch, you must call this function to 
    // tell me whether I am IEEE 802.2/802.3 (RFC 1042) or Ethernet (RFC894)
    void SetFrameFormat(FrameFormat frameFormat);

    // Returns the format of the frame, so that clients can know which of the
    // possible Ethernet Header fields are valid.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The Ethernet frame format.
    //
    inline FrameFormat GetFrameFormatParsed(void) const;

    // This is useful for doing loopback operation.  It swaps the MAC SA with
    // the MAC DA, allowing you to send the packet back to the originator.
    // If this is an IP packet, you can also have it swap the dest and source
    // IP address.  If it is a TCP or UDP IP packet, you can also swap the
    // dest and source port.
    //
    // Parameters:
    //      swapIp - tells it whether or not to swap the IP addresses.  This
    //               only happens for IP packets.
    //      swapPort - tells it whether or not to swap the TCP/UDP ports.  This
    //                 only happens for TCP/UDP packets.
    //
    // Returns:  Nothing.
    //
    void SwapAddresses(bool swapIp = false, bool swapPort = false);

    // Accessors for the Destination MAC address.  This is always bytes 0..5 of
    // the packet.
    inline const BcmMacAddress &MacDa(void) const;
    void MacDa(const BcmMacAddress &macDa);

    // Accessors for the Source MAC address.  This is always bytes 6..11 of the
    // packet.
    inline const BcmMacAddress &MacSa(void) const;
    void MacSa(const BcmMacAddress &macSa);

    // Common values for the EtherType field.
    enum
    {
        kEtherTypeIp = 0x0800,
        kEtherTypeArp = 0x0806,
        kEtherTypeRarp = 0x8035,        
        kEtherTypeIpx = 0x8137,
        kEtherTypeIpV6 = 0x86dd
    } CommonEtherTypes;

    // Accessors for the EtherType field.  This is only valid for frame formats
    // of kEthernetII, and kSnap.
    inline uint16 EtherTypeParsed(void) const;
    void EtherType(uint16 etherType);

    // Accessors for the EtherLength field.  This is only valid for frame
    // formats other than kEthernetII.
    inline uint16 EtherLengthParsed(void) const;
    void EtherLength(uint16 etherLength);

    // Accessors for the DSAP field.  This is only valid for frame formats other
    // than kEthernetII.
    inline uint8 DsapParsed(void) const;
    void Dsap(uint8 dsap);

    // Accessors for the SSAP field.  This is only valid for frame formats other
    // than kEthernetII.
    inline uint8 SsapParsed(void) const;
    void Ssap(uint8 ssap);

    enum
    {
      kLlcProtocolHasEthertype = 0x00010000,
      kLlcProtocolHasDsap = 0x00020000
    } LlcProtocolType;
    
    // PR 12181:  Accessor for the special optimized merged protocol value used 
    // for LLC filtering.
    inline uint32 GetLlcMergedProtocolParsed(void) const;

    // Accessors for the Control field.  This is only valid for frame formats
    // other than kEthernetII.
    inline uint8 ControlParsed(void) const;
    void Control(uint8 control);

    // Accessors for the OrgCode bytes.  The offset must be 0..2.
    inline uint8 OrgCodeParsed(unsigned int offset) const;
    void OrgCode(unsigned int offset, uint8 orgCode);

    // When generating a packet, after you fill in all of the LLC header
    // fields, call this method to lock it in.  This simulates what would happen
    // if you called ParseLlc(), but it is faster.
    inline void LlcGenerated(void);

public:

    // Accessors for various ARP header fields.  Note that these are only valid
    // if the EtherType is kEtherTypeArp.  Reading/writing these fields for non
    // ARP packets will yield unpredictable results.

    // Pre-parses the ARP packet so that you can call the ...Parsed() accessors.
    //
    // Returns false if this is not an ARP packet.
    inline bool ParseArp(void);

    // Is this an ARP packet anyway?
    inline bool IsArpPktParsed(void) const;

    // Accessors for the hardware type field.
    inline uint16 ArpHardwareTypeParsed(void) const;
    void ArpHardwareType(uint16 hardwareType);

    // Values for the hardware type field.
    enum
    {
        kArpHwTypeEthernet = 1
    };

    // Accessors for the protocol type field.  This is identical to the
    // EtherType field, though we use a different variable to store it.  Most
    // of the time, this will be 0x0800 (IP).
    inline uint16 ArpProtocolTypeParsed(void) const;
    void ArpProtocolType(uint16 protocolType);

    // Accessors for the hardware size field - the number of octets in the
    // hardware address.  For hardware type of 1 (Ethernet), this will be 6.
    inline uint8 ArpHardwareSizeParsed(void) const;
    void ArpHardwareSize(uint8 hardwareSize);

    // Accessors for the protocol size field - the number of octets in the
    // protocol address.  For protocol type of 0x0800 (IP), this will be 4.
    // IPv6 will probably be more.
    inline uint8 ArpProtocolSizeParsed(void) const;
    void ArpProtocolSize(uint8 protocolSize);

    // Accessors for the opcode field.
    inline uint16 ArpOpcodeParsed(void) const;
    void ArpOpcode(uint16 opcode);

    // Values for the opcode field.
    enum
    {
        kArpRequest = 1,
        kArpReply = 2
    };

    // Accessors for the sender's h/w address.  Because we deal with a hardware
    // type of Ethernet (1) most of the time, this is all that we will provide
    // for now.  If we start dealing with other hardware types, then we can add
    // more general octet buffer accessors to get/set the h/w address.
    inline const BcmMacAddress &ArpSenderHardwareAddressParsed(void) const;
    void ArpSenderHardwareAddress(const BcmMacAddress &hardwareAddress);

    // Accessors for the sender's protocol address.  Because we deal with IP
    // (0x0800) most of the time, this is all that we will provide for now.  If
    // we start dealing with other types, then we can add more general octet
    // buffer accessors to get/set the protocol address.
    inline const BcmIpAddress &ArpSenderProtocolAddressParsed(void) const;
    void ArpSenderProtocolAddress(const BcmIpAddress &protocolAddress);

    // Accessors for the target's h/w address.
    inline const BcmMacAddress &ArpTargetHardwareAddressParsed(void) const;
    void ArpTargetHardwareAddress(const BcmMacAddress &hardwareAddress);

    // Accessors for the target's protocol address.
    inline const BcmIpAddress &ArpTargetProtocolAddressParsed(void) const;
    void ArpTargetProtocolAddress(const BcmIpAddress &protocolAddress);

    // When generating a packet, after you fill in all of the ARP header
    // fields, call this method to lock it in.  This simulates what would happen
    // if you called ParseArp(), but it is faster.
    inline void ArpGenerated(void);

public:

    // Accessors for various IP Header fields.  Note that these are only valid
    // if the EtherType is kEtherTypeIp.  Reading/writing these fields for non
    // IP packets will yield unpredictable results.
    
    // Pre-parses the IP header so that you can call the ...Parsed() accessors.
    //
    // Returns false if this is not an IP packet.
    inline bool ParseIp(void);

    // Pre-parses the IPV6 header so that you can call the ...Parsed() accessors.
    //
    // Returns false if this is not an IP packet.
    inline bool ParseIpV6(void);

    // This is a specialized version of ParseIp() that only does the source and
    // destination IP addresses.  After calling this, you can call the
    // SourceIpAddressParsed() and DestIpAddressParsed() methods.
    //
    // Returns false if this is not an IP packet.
    inline bool ParseIpAddr(void);

    // This is a specialized version of ParseIp() that only does the source and
    // destination IPv6 addresses.  After calling this, you can call the
    // SourceIpAddressParsed() and DestIpAddressParsed() methods.
    //
    // Returns false if this is not an IP packet.
    inline bool ParseIpV6Addr(void);

    // Is this an IP packet anyway?
    inline bool IsIpPktParsed(void) const;

    // Is this an IPv6 packet anyway?
    inline bool IsIpV6PktParsed(void) const;

    // Accessors for the 4-bit IP Version number field.
    inline uint8 IpVersionParsed(void) const;
    void IpVersion(uint8 ipVersion);

    // Accessors for the IP Header length field.  These specify the value in
    // bytes, even though it is stored in the header in units of 4-byte words.
    // Note that when you sent this value, it must be a multiple of 4 (the lower
    // 2 bits will be discarded).
    inline unsigned int IpHeaderLengthBytesParsed(void) const;
    void IpHeaderLengthBytes(unsigned int headerLength);

    // the total length of the IP packet including IP header in bytes
    inline unsigned int IpTotalLengthBytesParsed(void) const;
    void IpTotalLengthBytes(uint16 totalLength);

    // IP header identification field
    inline uint16 IdentificationParsed(void) const;
    void Identification(uint16 ident);

    // IP header flags
    inline uint8 FlagsParsed(void) const;
    void Flags(uint8 flags);

    // IP header fragment offset
    inline uint16 FragmentOffsetParsed(void) const;
    void FragmentOffset(uint16 offset);

    // IP header time-to-live (typically 32 or 64 router hops)
    uint8 TimeToLive();
    void TimeToLive(uint8 ttl);

    // Accessors for the ToS bits.
    inline uint8 IpTypeOfServiceParsed(void) const;
    void IpTypeOfService(uint8 tos);

    // Common values for the IP Protocol field.
    enum
    {
        kIcmp = 0x01,
        kIgmp = 0x02,
        kTcp = 0x06,
        kUdp = 0x11,
        kEsp = 0x32,
        kAh  = 0x33,
        kGre = 0x2f,
        kIcmpV6 = 0x3a,
    } CommonIpProtocolTypes;

    // Accessors for the IP Protocol Type field.
    inline uint8 IpProtocolTypeParsed(void) const;
    void IpProtocolType(uint8 protocolType);

    // Simple accessor that tells you if the Protocol Type field is either TCP
    // or UDP.  This is useful for checking whether or not the Source/Dest Port
    // numbers should be checked.
    inline bool IpProtocolTypeTcpUdpParsed(void) const;

    // Accessors for the source IP address.
    inline const BcmIpAddress &SourceIpAddressParsed(void) const;
    void SourceIpAddress(const BcmIpAddress &sourceIp);
    void SourceIpAddressFixup(const BcmIpAddress &sourceIp);
    void SourceIpAddressFixupNoCheckSum(const BcmIpAddress &sourceIp);

    // Accessors for the destination IP address.
    inline const BcmIpAddress &DestIpAddressParsed(void) const;
    void DestIpAddress(const BcmIpAddress &destIp);
    void DestIpAddressFixup(const BcmIpAddress &destIp);
    void DestIpAddressFixupNoCheckSum(const BcmIpAddress &destIp);
    
    // Returns a pointer to the first byte of the IP options, if present
    const uint8 *IpOptions(void) const;

    // Helper method to check the IP Header checksum.  Before calling this
    // method, you must call ParseLlc() (and verify that this is an IP packet),
    // but you don't have to call ParseIp().
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the IP header checksum is valid.
    //      false if the IP header checksum is not valid.
    //
    bool IpHcsIsValid(void) const;

    // When generating a packet, after you fill in all of the IP header
    // fields, call this method to lock it in.  This simulates what would happen
    // if you called ParseIp(), but it is faster.
    inline void IpGenerated(void);

    /// Does basic initialization of an IPv6 header, setting the version to 6,
    /// and setting other fields to 0.  It also sets the offset of the TCP/UDP
    /// header.
    ///
    /// Call this method before calling other methods to generate the IPv6
    /// header.
    ///
    void IpV6Init(void);

    // Accessors for IPv6 header fields
    void IpV6TrafficClass(uint8 trafficClass);
    uint8  IpV6TrafficClass();

    void IpV6FlowLabel(uint32 flowLabel);
    uint32 IpV6FlowLabel();

    void IpV6PayloadLength(uint16 payloadLength);
    uint16 IpV6PayloadLength();

    void IpV6NextHeader(uint8 nextHeader);
    uint8 IpV6NextHeader();

    /// Returns the last next header value (typically the next protocol header,
    /// like TCP, UDP, ICMP, etc).
    uint8 IpV6LastNextHeader();

    void IpV6HopLimit(uint8 hopLimit);
    uint8 IpV6HopLimit();

    // When generating a packet, after you fill in all of the IPv6 header
    // fields, call this method to lock it in.  This simulates what would happen
    // if you called ParseIpV6(), but it is faster.
    inline void IpV6Generated(void);

    // IPV6 header identification field
    inline uint32 IpV6IdentificationParsed(void) const;

    // Length of unframentable part of the IPV6 packet.
    inline unsigned int IpV6UnfragmentLengthBytesParsed(void) const;

    // accessor for IPv6 Reassembly Next Header field.  After fragments has been
    // reassembled to its original packet, the next header field of the last extended
    // header of the unfragmentable part must be updated back to its original value.
    void UpdateIpV6ReassemblyNextHeaderField(void);

    // accessor for last header of unfragmentable part of IPV6 extension header
    void IpV6UnfragmentableLastHeader(uint8 nextHeader);
    uint8 IpV6UnfragmentableLastHeader();

private:

    // If any of the IP header's contents get modified, the HCS must be
    // recalculated.
    void UpdateIpHcs (void);

    // fast way to recalculate the checksum
    void ChecksumFixup (uint8 *bptr_checksum, uint8 *bptr_old_data,
		uint16 old_data_length, uint8 *bptr_new_data, uint16  new_data_length);

public:

    // Accessors for TCP/UDP port numbers.  Note that these are only valid if
    // the EtherType() is kEtherTypeIp, and IpProtocolTypeTcpUdp() is true.
    // Reading/writing these fields for non-TCP/UDP packets will yield
    // unpredictable results.

    // Pre-parses the TCP/UDP port numbers so that you can call the ...Parsed()
    // accessors.
    //
    // Returns false if this is not a TCP or UDP packet.
    inline bool ParseTcpUdp(void);

    // Accessors for the source port.  This works for both TCP and UDP packets.
    inline uint16 SourceTcpUdpPortParsed(void) const;
    void SourceTcpUdpPortFixup(uint16 sourcePort);
    void SourceTcpUdpPort(uint16 sourcePort);

    // Accessors for the destination port.  This works for both TCP and UDP
    // packets.
    inline uint16 DestTcpUdpPortParsed(void) const;
    void DestTcpUdpPortFixup(uint16 destPort);
    void DestTcpUdpPort(uint16 destPort);

    // Allows you to read/write the UDP length field if you are constructing
    // your  own packet from scratch.  This field includes the UDP header and
    // the data. 
    uint16 UdpLengthBytes(void) const;
    void UdpLengthBytes(uint16 totalLength);

    // Allows you to write to the UDP checksum field.  useful when constructing
    // a packet from scratch, and the field is unused.  You can zero it.
    void UdpChecksum(uint16 checksum);
    uint16 UdpChecksum(void) const;

    // Helper method to check the UDP Header checksum.  Before calling this
    // method, you must call ParseLlc() and ParseIp() (verifying that this is
    // a UDP/IP packet), but you don't have to call ParseTcpUdp().
    //
    // Note that the UDP checksum covers the UDP header, the UDP payload, and
    // a pseudo header for some IP header fields.  Because the checksum covers
    // the payload, it can be time consuming!
    //
    // If the sender didn't calculate a checksum (value == 0), then this will
    // return true.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the UDP checksum is valid.
    //      false if the UDP checksum is not valid.
    //
    bool UdpChecksumIsValid(void) const;

    // Causes the UDP checksum to be calculated and stored.
    void UpdateUdpChecksum(void);

    // Returns a pointer to the first byte of the payload following the UDP
    // header.  Note that if this isn't a UDP/IP packet (or if the IP header
    // hasn't been parsed yet), then this returns NULL!
    const uint8 *UdpPdu(void) const;

    // Same as above, but allows the PDU to be modified.  Note that you are
    // responsible for updating the UDP checksum after the PDU is modified (you
    // can just set it to 0, since the checksum is not required for UDP).
    uint8 *UdpPduWritable(void);

    // When generating a packet, after you fill in all of the UDP header
    // fields, call this method to lock it in.  This simulates what would happen
    // if you called ParseTcpUdp(), but it is faster.
    inline void UdpGenerated(void);

    // Returns a pointer to the first byte of the payload following the LLC
    // header.  Note that if this isn't a Ethernet packet (or if the LLC header
    // hasn't been parsed yet), then this returns NULL!
    const uint8 *LlcPdu(void) const;

    // Same as above, but allows the PDU to be modified.  
    uint8 *LlcPduWritable(void);

    // Returns a pointer to the first byte of the payload following the IP
    // header.  Note that if this isn't a Ethernet packet (or if the IP header
    // hasn't been parsed yet), then this returns NULL!
    const uint8 *IpPdu(void) const;

    // Same as above, but allows the PDU to be modified.  
    uint8 *IpPduWritable(void);
#ifdef BCM_STANDALONE_DHCP_CLIENT
    // Returns a pointer to the first byte of the payload including the LLC
    // header.  Note that if this isn't a Ethernet packet (or if the LLC header
    // hasn't been parsed yet), then this returns NULL!
    const uint8 *PduHeader(void) const;

    // Same as above, but allows the PDU to be modified.  
    uint8 *PduHeaderWritable(void);
#endif    

public:

    // Accessors for TCP header elements.  Note that these are only valid if
    // the EtherType() is kEtherTypeIp, and IpProtocolType() is kTcp.
    // Reading/writing these fields for non-TCP packets will yield unpredicatble
    // results.

    // Pre-parses the TCP header so that you can call the ...Parsed() accessors.
    //
    // Returns false if this is not a TCP packet.
    inline bool ParseTcp(void);

	// Returns a pointer to the first byte of the TCP
	// header.  Note that if this isn't a TCP/IP packet (or if the IP header
	// hasn't been parsed yet), then this returns NULL!
	const uint8 *TcpHeader(void) const;

    // Accessors for the TCP sequence number.
    inline uint32 TcpSequenceNumberParsed(void) const;
    void TcpSequenceNumber(uint32 sequenceNumber);
    void TcpSequenceNumberFixup(uint32 sequenceNumber);

    // Accessors for the TCP ack number.
    inline uint32 TcpAckNumberParsed(void) const;
    void TcpAckNumber(uint32 ackNumber);
    void TcpAckNumberFixup(uint32 ackNumber);

    // Accessors for the TCP header length (in bytes).
    inline unsigned int TcpHeaderLengthBytesParsed(void) const;
    void TcpHeaderLengthBytes(unsigned int lengthBytes);
    inline bool TcpOptionsPresentParsed(void) const;
	inline uint8 *TcpOptionsPointer(void) const;

    // Flag values for the TCP flags field.
    typedef enum
    {
        kTcp_FIN = 0x01,
        kTcp_SYN = 0x02,
        kTcp_RST = 0x04,
        kTcp_PSH = 0x08,
        kTcp_ACK = 0x10,
        kTcp_URG = 0x20
    } TcpFlagBits;

    // Accessors for the TCP flags.
    inline uint8 TcpFlagsParsed(void) const;
    inline bool TcpFlagIsSetParsed(TcpFlagBits flag) const;
    void TcpFlags(uint8 flags);
    void SetTcpFlag(TcpFlagBits flag);
    void ClearTcpFlag(TcpFlagBits flag);

    // Accessors for the TCP window size.
    inline uint16 TcpWindowSizeParsed(void) const;
    void TcpWindowSize(uint16 windowSize);

    // Accessors for the TCP checksum.
    uint16 TcpChecksum(void) const;
    void TcpChecksum(uint16 checksum);

	// Helper method to check the TCP Header checksum.  Before calling this
	// method, you must call ParseLlc() and ParseIp() (verifying that this is
	// a TCP/IP packet), but you don't have to call ParseTcpUdp().
	//
	// Note that the TCP checksum covers the TCP header, the TCP payload, and
	// a pseudo header for some IP header fields.  Because the checksum covers
	// the payload, it can be time consuming!
	//
	// If the sender didn't calculate a checksum (value == 0), then this will
	// return true.
	//
	// Parameters:  None.
	//
	// Returns:
	//      true if the TCP checksum is valid.
	//      false if the TCP checksum is not valid.
	//
	bool TcpChecksumIsValid(void) const;
    
    // Causes the TCP checksum to be calculated and stored.
    void UpdateTcpChecksum(void);
	
	// Accessors for the TCP urgent pointer.
    inline uint16 TcpUrgentPointerParsed(void) const;
    void TcpUrgentPointer(uint16 urgentPointer);

    // Returns a pointer to the first byte of the payload following the TCP
    // header.  Note that if this isn't a TCP/IP packet (or if the IP header
    // hasn't been parsed yet), then this returns NULL!
    const uint8 *TcpPdu(void) const;

    // Same as above, but allows the PDU to be modified.  Note that you are
    // responsible for updating the TCP checksum after the PDU is modified.
    uint8 *TcpPduWritable(void);

    // When generating a packet, after you fill in all of the TCP header
    // fields, call this method to lock it in.  This simulates what would happen
    // if you called ParseTcp(), but it is faster.
    inline void TcpGenerated(void);

public:
    // Icmp Types
    enum
    {
	    kIcmpTypeEchoReply                      = 0,
	    kIcmpTypeDestinationUnreachable         = 3, 
	    kIcmpTypeSourceQuench                   = 4,
	    kIcmpTypeRedirect                       = 5,
	    kIcmpTypeEchoRequest                    = 8,
	    kIcmpTypeTimeExceeded                   = 11,
	    kIcmpTypeParameterProblem               = 12,
	    kIcmpTypeTimeStampRequest               = 13,
	    kIcmpTypeTimeStampReply                 = 14,
	    kIcmpTypeInfoRequest                    = 15,
	    kIcmpTypeInfoReply                      = 16,
	    kIcmpTypeAddressMaskRequest             = 17,
	    kIcmpTypeAddressMaskReply               = 18
    }; 

    // IcmpV6 Types
    enum
    {
	    kIcmpV6TypeDestinationUnreachable                 = 1,    // RFC2463
	    kIcmpV6TypePacketTooBig                           = 2,    // RFC2463 
	    kIcmpV6TypeTimeExceeded                           = 3,    // RFC2463
	    kIcmpV6TypeParameterProblem                       = 4,    // RFC2463

	    kIcmpV6TypeEchoRequest                            = 128,  // RFC2463
	    kIcmpV6TypeEchoReply                              = 129,  // RFC2463
	    kIcmpV6TypeGroupMembershipQuery                   = 130, 
	    kIcmpV6TypeGroupMembershipReport                  = 131, 
	    kIcmpV6TypeGroupMembershipReduction               = 132, 
	    kIcmpV6TypeRouterSolicitation                     = 133,  // RFC2461
	    kIcmpV6TypeRouterAdvertisement                    = 134,  // RFC2461
	    kIcmpV6TypeNeighborSolicitation                   = 135,  // RFC2461
	    kIcmpV6TypeNeighborAdvertisement                  = 136,  // RFC2461
	    kIcmpV6TypeRedirect                               = 137,  // RFC2461
	    kIcmpV6TypeRouterRenumbering                      = 138,  // RFC2894
	    kIcmpV6TypeInformationQuery                       = 139,  
	    kIcmpV6TypeInformationResponse                    = 140,  
	    kIcmpV6TypeInverseNeighborDiscoverySolicitation   = 141,  // RFC3122
	    kIcmpV6TypeInverseNeighborDiscoveryAdvertisement  = 142,  // RFC3122
	    kIcmpV6TypeVersion2MulticastListenerReport        = 143,  // RFC3810
	    kIcmpV6TypeHomeAgentAddressDiscoveryRequest       = 144,  // RFC3775
	    kIcmpV6TypeHomeAgentAddressDiscoveryReply         = 145,  // RFC3775
	    kIcmpV6TypeMobilePrefixSolicitation               = 146,  // RFC3775
	    kIcmpV6TypeMobilePrefixAdvertisement              = 147,  // RFC3775
	    kIcmpV6TypeCertificationPathSolicitation          = 148,  // RFC3971
	    kIcmpV6TypeCertificationPathAdvertisement         = 149,  // RFC3971
        kIcmpV6TypeExperimentalMobilityProtocols          = 150,  
        kIcmpV6TypeMulticastRouterAdvertisement           = 151,  // RFC4286
        kIcmpV6TypeMulticastRouterSolicitation            = 152,  // RFC4286
        kIcmpV6TypeMulticastRouterTermination             = 153,  // RFC4286
    }; 


    // Returns false if this is not a Icmp packet.
    inline bool ParseIcmp(void);

    // Returns false if this is not a IcmpV6 packet.
    inline bool ParseIcmpV6(void);

    // Accessors for the Icmp Type
    inline uint8 IcmpTypeParsed(void) const;

    // Accessors for the Icmp Code
    inline uint8 IcmpCodeParsed(void) const;

    // Accessors for the Icmp Idenfier
    inline uint16 IcmpIdentifierParsed(void) const;

    // Accessors for the Icmp Sequence Number
    inline uint16 IcmpSequenceNumberParsed(void) const;

    // Fixup Identifier and checksum
    void IcmpIdentifierFixup(uint16 identifier);

    // Fixup the seqeunce number and checksum
    void IcmpSequenceNumberFixup(uint16 sequenceNumber);

    // These are used by the ParseHttpXXX() methods to prevent parsing multiple
    // times.
    enum
    {
		// this isn't a HARD limit
		kHttpMinGetRequestSize = 14,	// "GET / HTTP/1.X"
		kHttpMinReplySize = 16			// "HTTP /1.X XXX OK"
    };

    // HTTP Method Types
    enum
    {
	    kGet	= 1,
	    kPost	= 2, 
		kHead	= 3
	};

    // Returns false if this is not an HTTP packet.
    bool ParseHttp(void);

    // Accessors for the HTTP packet length (in bytes).
    unsigned int HttpTotalLengthBytesParsed(void) const;
    void HttpTotalLengthBytes(unsigned int lengthBytes);

	// Accessor for the HTTP Method Type
    uint8 HttpMethodTypeParsed(void) const;
	char *HttpMethodTypePointer(void) const;
    
    // Accessor for BcmPacketId - this is a semi-unique (rolls over @ 0xFFFFFFFF)
    // identifier for each packet.  There is a global counter which starts at
    // 0 and is incremented for each new packet, or when a packet changes.
    inline unsigned long BcmPacketId(void) const;

private:

    // These are the implementations for the packet parsing functions.  They are
    // called by the public inline methods if necessary.
    void ParseLlcImpl(void);
    void ParseArpImpl(void);
    void ParseIpAddrImpl(void);
    void ParseIpV6AddrImpl(void);
    void ParseIpImpl(void);
    void ParseIpV6Impl(void);
    void ParseTcpUdpImpl(void);
    void ParseTcpImpl(void);
    void ParseIcmpImpl(void);
    void ParseIcmpV6Impl(void);
	void ParseHttpImpl(void);

private:

    static unsigned long gGlobalBcmPacketId;
    unsigned long fBcmPacketId;


    // This is the pointer to the structure that refers to ethernet packet.
    ETHERNET_PACKET *pfEthernetPacket;

    // PR12290 - pointer to the buffer, from pfEthernetPacket.
    uint8 *pfBuffer;

    // Keeps track of when the packet has changed.
    bool fPacketChanged;

    // This is the total number of bytes in the packet, starting at the MAC DA,
    // but not including the CRC32.
    unsigned int fPacketLength;

    // The format of the Ethernet packet that we detected.
    FrameFormat fFrameFormat;

    // Offsets into the packet where we can find various key starting points.
    // These vary because the frame formats and contents can vary.
    unsigned int fEtherTypeOffset;
    unsigned int fIpHeaderOffset;
    unsigned int fTcpUdpHeaderOffset;

    // These are used by the ParseXXX() methods to prevent parsing multiple
    // times.
    enum
    {
        kLlcWasParsed       = 0x01,
        kArpWasParsed       = 0x02,
        kIpWasParsed        = 0x04,
        kIpV6WasParsed      = 0x08,
        kTcpUdpWasParsed    = 0x10,
        kTcpWasParsed       = 0x20,
        kIcmpWasParsed      = 0x40,
        kIcmpV6WasParsed    = 0x80,
        kIpAddrWasParsed    = 0x100,
        kIpV6AddrWasParsed  = 0x200,
		kHttpWasParsed	    = 0x400,
    };

    uint32 fWasParsedMask;

    // This is calculated from the fMacDa parameter when it changes.
    MacAddrType fMacDaType;

    // Storage for various Ethernet Header fields.  Note that some fields are
    // not valid for some frame formats.
    BcmMacAddress fMacDa;
    BcmMacAddress fMacSa;
    uint16 fEtherType;
    uint16 fEtherLength;
    uint8 fDsap;
    uint8 fSsap;
    uint8 fControl;
    uint8 fOrgCode[3];

    // PR 12181:  Special optimized merged value for LLC filtering.  This 
    // consists of the lower 16 bits being the ethertype OR DSAP and the upper 
    // 16 bits containing a flag to indicate which protocol type is being used.
    uint32 fMergedLlcProtocol;

    // Storage for various ARP packet fields.  Note that these fields are only
    // valid if the fEtherType field is kEtherTypeArp.
    uint16 fArpHardwareType;
    uint16 fArpProtocolType;
    uint8 fArpHardwareSize;
    uint8 fArpProtocolSize;
    uint16 fArpOpcode;
    BcmMacAddress fArpSenderHardwareAddress;
    BcmIpAddress fArpSenderProtocolAddress;
    BcmMacAddress fArpTargetHardwareAddress;
    BcmIpAddress fArpTargetProtocolAddress;

    // Storage for various IP Header fields.  Note that these fields are only
    // valid if the fEtherType field is kEtherTypeIp.
    uint8 fIpVersion;
    unsigned int fIpHeaderLengthBytes;
    unsigned int fIpTotalLengthBytes;
	uint8 fIpFlags;
	uint16 fIpFragmentOffset;
	uint16 fIpIdentification;
    uint8 fIpTypeOfService;
    uint8 fIpProtocolType;
    bool fIpProtocolTypeIsTcpUdp;
    BcmIpAddress fSourceIpAddress;
    BcmIpAddress fDestIpAddress;

    // Storage for IPV6 header fields
    uint8 fIpV6TrafficClass;
    uint32 fIpV6FlowLabel;
    uint16 fIpV6PayloadLength;

    // parameters need for IPV6 fragment reassembly
	uint32 fIpV6Identification;
    unsigned int fIpV6UnfragmentableLengthBytes;
    unsigned int fIpV6UnfragmentableLastHeaderOffset;
    uint8 fIpV6FragmentNextHeader;
    
    enum
    {
        kHopByHopOptionsHdr    = 0,
        kRoutingHdr            = 43, 
        kFragmentHdr           = 44, 
        kEncapSecPayloadHdr    = 50,
        kAuthenticationHdr     = 51,
        kDestinationOptionsHdr = 60,
    } IpV6NextHeaderTypes;

    uint8 fIpV6NextHeader;
    uint8 fIpV6HopLimit;

    // Storage for TCP/UDP port numbers.  Note that these fields are only valid
    // if the fEtherType field is kEtherTypeIp, and fIpProtocolTypeIsTcpUdp is
    // true.
    uint16 fSourceTcpUdpPort;
    uint16 fDestTcpUdpPort;

    // Storage for TCP header fields.  Note that these fields are only valid
    // if the fEtherType field is kEtherTypeIp, and fIpProtocolType is kTcp.
    uint32 fTcpSequenceNumber;
    uint32 fTcpAckNumber;
    unsigned int fTcpHeaderLengthBytes;
    uint8 fTcpFlags;
    uint16 fTcpWindowSize;
    //uint16 fTcpChecksum;      // Skipping this for now.
    uint16 fTcpUrgentPointer;

    // storage for ICMP identifiers (Type, Code, Sequence Number and Identifier)
    uint8  fIcmpType; 
    uint8  fIcmpCode;
    uint16 fIcmpIdentifier;
    uint16 fIcmpSequenceNumber;

	// storage for HTTP identifiers 
    unsigned int  fHttpTotalLengthBytes; 
    uint8  fHttpMethodType;
	char  *fHttpMethodTypePointer;

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmEthernetPacket(const BcmEthernetPacket &otherInstance);

    // Assignment operator.  Not supported.
    //
    // NOTE that this object MUST already have a pointer to a valid
    // ETHERNET_PACKET before you call this.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmEthernetPacket & operator = (const BcmEthernetPacket &otherInstance);

};


/*
Note: the following is from Don Provan, (Novell)
at ftp://ftp.ed.ac.uk/pub/EdLAN/provan-enet

=====================================================================
 
Novell Frame Type Designation: Ethernet_II
Common name: Ethernet
Layout:
 
    6 bytes       6 bytes       2 bytes         Up to 1500 bytes
+-------------+-------------+-------------+-------------------------+
| Destination |   Source    |    E-type   | Network Protocol Packet |
| MAC Address | MAC Address | (IPX: 8137) |                         |
+-------------+-------------+-------------+-------------------------+
 
Comments: 1. Used for TCP/IP as well as many other protocols.
          2. Most common frame type in general, although
             Ethernet_802.3 might possibly carry more
             packets world wide.
 
=====================================================================
 
Novell Frame Type Designation: Ethernet_802.3
Common name: "Raw" 802.3
Layout:
 
    6 bytes       6 bytes       2 bytes         Up to 1500 bytes
+-------------+-------------+--------------+------------------------+
| Destination |   Source    | Total packet |       IPX Packet       |
| MAC Address | MAC Address |    length    | first two bytes: FF,FF |
+-------------+-------------+--------------+------------------------+
 
Comments: 1. No protocol ID: Can only carry IPX packets.
          2. Distinguishable from Ethernet_802.2 only because the
             first two bytes of all IPX packets carried on
             Ethernet_802.3 must be all ones, which makes no sense in
             Ethernet_802.2.
          3. The default frame type for NetWare software until
             NetWare v4.0 was released.
 
=====================================================================
 
Novell Frame Type Designation: Ethernet_802.2
Common Name: 802.3 (the 802.2 header is implied by the 802.3 standard).
Also Known As: 802.3/802.2, to distinguish from "raw" 802.3
Layout:
 
6 bytes 6 bytes 2 bytes 1 byte 1 byte  1 byte    Up to 1497 bytes
+------+------+--------+------+------+--------+---------------------+
| Dest | Src  | length | DSAP | SSAP | Control|   Network Packet    |
| Addr | Addr |        | (E0) | (E0) |  (03)  |                     |
+------+------+--------+------+------+--------+---------------------+
 
Comments: 1. Used for OSI packets on 802.3 networks.
          2. Numbers in parentheses are the values used by IPX.
          3. The default frame type for the NetWare v4.0 release.
 
=====================================================================
 
Novell Frame Type Designation: Ethernet_SNAP
Common name: 802.3/SNAP or 802.3/802.2/SNAP
Layout:
 
 6 b  6 b  2 b 1 byte 1 byte 1 byte     5 bytes     Up to 1492 bytes
+----+----+---+------+------+------+---------------+----------------+
|Dst |Src |len| DSAP | SSAP | Ctrl |    SNAP ID    | Network Packet |
|Addr|Addr|   | 0xAA | 0xAA | 0x03 | (0,0,0,81,37) |                |
+----+----+---+------+------+------+---------------+----------------+
 
Comments: 1. Extension to 802.2, indicated by SAP value of hex AA.
          2. Number in parentheses is the value used for IPX.
          3. Used by AppleTalk.  Almost never used for IPX.
 
=====================================================================
*/


//********************** Inline Method Implementations ***********************


inline ostream & operator << (ostream &outStream, BcmEthernetPacket &packet)
{
    return packet.Print(outStream,true);
}


// Allows access to the underlying ETHERNET_PACKET structure, primarily so
// that it can be send to a HAL.
//
// NOTE - do not modify the packet contents!  This will cause a loss of
//        sync between the members of this class and the data buffer!
//
// Parameters:  None.
//
// Returns:
//      A pointer to the packet.
//
inline ETHERNET_PACKET *BcmEthernetPacket::PacketStruct(void)
{
    return pfEthernetPacket;
}


inline const uint8 *BcmEthernetPacket::Buffer(void) const
{
    return pfBuffer;
}


// Allows a client to query/set the total number of bytes in the packet,
// starting at the MAC DA, going through the end of the PDU, but not
// inluding the CRC32 at the end.
//
// This is useful because the packet may not exist in a single buffer.
//
// Parameters:  None.
//
// Returns:
//      The total packet length.
//
inline unsigned int BcmEthernetPacket::PacketLength(void) const
{
    return fPacketLength;
}


// Tells whether or not the packet has been modified since the last time
// this method was called.  This is usually used for debugging, where we
// only want to print the packet contents if the packet changed from the
// last time it printed.
//
// Parameters:  None.
//
// Returns:
//      true if the packet has been changed since the last time this method
//          was called.
//      false if the packet has not been changed.
//
inline bool BcmEthernetPacket::PacketChanged(void)
{
    bool temp = fPacketChanged;

    fPacketChanged = false;

    return temp;
}


// Pre-parses the LLC header so that you can call the ...Parsed() accessors.
inline void BcmEthernetPacket::ParseLlc(void)
{
    // Only do this if we have not already parsed the LLC header.
    if ((fWasParsedMask & kLlcWasParsed) == 0)
    {
        ParseLlcImpl();
    }
}

// Returns the LLC header length. 
inline unsigned int BcmEthernetPacket::EthernetHeaderLengthParsed(void)
{
    return fIpHeaderOffset;
}

// Returns the type of the MAC DA.
//
// Parameters:  None.
//
// Returns:
//      One of the MacAddrType enum values.
//
inline BcmEthernetPacket::MacAddrType BcmEthernetPacket::MacDaType(void) const
{
    return fMacDaType;
}


// Checks to see if the current packet has a unicast MAC DA.  This is much
// faster than other comparisons using the actual MAC DA object.  A packet
// is unicast if it isn't broadcast and it isn't multicast, i.e. the lsb of
// the first byte of the MAC DA is 0.
//
// This is actually calculated in SetPacket() and the accessor that sets
// the MAC DA.
//
// Parameters:  None.
//
// Returns:
//      true if the packet has a unicast MAC DA.
//      false if the packet does not have a unicast MAC DA.
//
inline bool BcmEthernetPacket::IsUnicast(void) const
{
    return (fMacDaType == kUnicast);
}


// Checks to see if the current packet has a multicast MAC DA, but is NOT
// broadcast (a broadcast packet is a type of multicast).  A packet is
// considered to be multicast by this method if the lsb of the first byte
// of the MAC DA is 1 AND the MAC DA is not broadcast (ff:ff:ff:ff:ff:ff).
//
// This is actually calculated in SetPacket() and the accessor that sets
// the MAC DA.
//
// Parameters:  None.
//
// Returns:
//      true if the packet has a multicast MAC DA.
//      false if the packet does not have a multicast MAC DA.
//
inline bool BcmEthernetPacket::IsMulticast(void) const
{
    return (fMacDaType == kMulticast);
}


// Checks to see if the current packet has a broadcast MAC DA.  A packet is
// considered to be broadcast if the MAC DA is ff:ff:ff:ff:ff:ff.
//
// This is actually calculated in SetPacket() and the accessor that sets
// the MAC DA.
//
// Parameters:  None.
//
// Returns:
//      true if the packet has a broadcast MAC DA.
//      false if the packet does not have a broadcast MAC DA.
//
inline bool BcmEthernetPacket::IsBroadcast(void) const
{
    return (fMacDaType == kBroadcast);
}


// Returns the format of the frame, so that clients can know which of the
// possible Ethernet Header fields are valid.
//
// Parameters:  None.
//
// Returns:
//      The Ethernet frame format.
//
inline BcmEthernetPacket::FrameFormat BcmEthernetPacket::GetFrameFormatParsed(void) const
{
    return fFrameFormat;
}


inline const BcmMacAddress &BcmEthernetPacket::MacDa(void) const
{
    // This one is always parsed when the frames is set, so there is no
    // validation required.

    return fMacDa;
}


inline const BcmMacAddress &BcmEthernetPacket::MacSa(void) const
{
    // This one is always parsed when the frames is set, so there is no
    // validation required.

    return fMacSa;
}


inline uint16 BcmEthernetPacket::EtherTypeParsed(void) const
{
    return fEtherType;
}


inline uint16 BcmEthernetPacket::EtherLengthParsed(void) const
{
    return fEtherLength;
}


inline uint8 BcmEthernetPacket::DsapParsed(void) const
{
    return fDsap;
}


inline uint8 BcmEthernetPacket::SsapParsed(void) const
{
    return fSsap;
}


// PR 12181:  Special optimized merged value for LLC filtering.  This 
// consists of the lower 16 bits being the ethertype OR DSAP and the upper 
// 16 bits containing a flag to indicate which protocol type is being used.
inline uint32 BcmEthernetPacket::GetLlcMergedProtocolParsed(void) const
{
    return fMergedLlcProtocol;
}


inline uint8 BcmEthernetPacket::ControlParsed(void) const
{
    return fControl;
}


inline uint8 BcmEthernetPacket::OrgCodeParsed(unsigned int offset) const
{
    // Check to see if the offset is valid.
    if (offset > 2)
    {
        return 0;
    }

    return fOrgCode[offset];
}


// When generating a packet, after you fill in all of the LLC header
// fields, call this method to lock it in.  This simulates what would happen
// if you called ParseLlc(), but it is faster.
inline void BcmEthernetPacket::LlcGenerated(void)
{
    fWasParsedMask |= kLlcWasParsed;
}


// Pre-parses the ARP packet so that you can call the ...Parsed() accessors.
//
// Returns false if this is not an ARP packet.
inline bool BcmEthernetPacket::ParseArp(void)
{
    if ((fWasParsedMask & kArpWasParsed) == 0)
    {
        ParseArpImpl();
    }

    return IsArpPktParsed();
}


inline bool BcmEthernetPacket::IsArpPktParsed(void) const
{
    if (EtherTypeParsed() != kEtherTypeArp)
    {
        return false;
    }

    return true;
}


inline uint16 BcmEthernetPacket::ArpHardwareTypeParsed(void) const
{
    return fArpHardwareType;
}


inline uint16 BcmEthernetPacket::ArpProtocolTypeParsed(void) const
{
    return fArpProtocolType;
}


inline uint8 BcmEthernetPacket::ArpHardwareSizeParsed(void) const
{
    return fArpHardwareSize;
}


inline uint8 BcmEthernetPacket::ArpProtocolSizeParsed(void) const
{
    return fArpProtocolSize;
}


inline uint16 BcmEthernetPacket::ArpOpcodeParsed(void) const
{
    return fArpOpcode;
}


inline const BcmMacAddress &BcmEthernetPacket::ArpSenderHardwareAddressParsed(void) const
{
    return fArpSenderHardwareAddress;
}


inline const BcmIpAddress &BcmEthernetPacket::ArpSenderProtocolAddressParsed(void) const
{
    return fArpSenderProtocolAddress;
}


inline const BcmMacAddress &BcmEthernetPacket::ArpTargetHardwareAddressParsed(void) const
{
    return fArpTargetHardwareAddress;
}


inline const BcmIpAddress &BcmEthernetPacket::ArpTargetProtocolAddressParsed(void) const
{
    return fArpTargetProtocolAddress;
}


// When generating a packet, after you fill in all of the ARP header
// fields, call this method to lock it in.  This simulates what would happen
// if you called ParseArp(), but it is faster.
inline void BcmEthernetPacket::ArpGenerated(void)
{
    fWasParsedMask |= kArpWasParsed;
}


// Pre-parses the IP header so that you can call the ...Parsed() accessors.
//
// Returns false if this is not an IP packet.
inline bool BcmEthernetPacket::ParseIp(void)
{
    // Only do this if we have not already parsed the IP header.
    if ((fWasParsedMask & kIpWasParsed) == 0)
    {
        ParseIpImpl();
    }

    // Return true if this is an IP packet.
    return IsIpPktParsed();
}


// This is a specialized version of ParseIp() that only does the source and
// destination IP addresses.
//
// Returns false if this is not an IP packet.
inline bool BcmEthernetPacket::ParseIpAddr(void)
{
    // Only do this if we have not already parsed the IP addresses.
    if ((fWasParsedMask & kIpAddrWasParsed) == 0)
    {
        ParseIpAddrImpl();
    }

    // Return true if this is an IP packet.
    return IsIpPktParsed();
}

// Pre-parses the IP header so that you can call the ...Parsed() accessors.
//
// Returns false if this is not an IP packet.
inline bool BcmEthernetPacket::ParseIpV6(void)
{
    // Only do this if we have not already parsed the IP header.
    if ((fWasParsedMask & kIpV6WasParsed) == 0)
    {
        ParseIpV6Impl();
    }

    // Return true if this is an IP packet.
    return IsIpV6PktParsed();
}

// This is a specialized version of ParseIpV6() that only does the source and
// destination IP addresses.
//
// Returns false if this is not an IP packet.
inline bool BcmEthernetPacket::ParseIpV6Addr(void)
{
    // Only do this if we have not already parsed the IP addresses.
    if ((fWasParsedMask & kIpV6AddrWasParsed) == 0)
    {
        ParseIpV6AddrImpl();
    }

    // Return true if this is an IP packet.
    return IsIpV6PktParsed();
}



inline bool BcmEthernetPacket::IsIpPktParsed(void) const
{
    // IP may only exist within EtherII or SNAP frames; the parsing functions
    // will set the EtherType field to 0 if it isn't one of these types, so
    // we don't have to check this.
    if (EtherTypeParsed() != kEtherTypeIp)
        return false;

    return true;
}

inline bool BcmEthernetPacket::IsIpV6PktParsed(void) const
{
    // IPV6 may only exist within EtherII or SNAP frames; the parsing functions
    // will set the EtherType field to 0 if it isn't one of these types, so
    // we don't have to check this.
    if (EtherTypeParsed() != kEtherTypeIpV6)
        return false;

    return true;
}

inline uint8 BcmEthernetPacket::IpVersionParsed(void) const
{
    return fIpVersion;
}


inline unsigned int BcmEthernetPacket::IpHeaderLengthBytesParsed(void) const
{
    return fIpHeaderLengthBytes;
}

inline unsigned int BcmEthernetPacket::IpTotalLengthBytesParsed(void) const
{
    return fIpTotalLengthBytes;
}


inline uint16 BcmEthernetPacket::IdentificationParsed(void) const
{
	return fIpIdentification;
}


inline uint8 BcmEthernetPacket::FlagsParsed(void) const
{
	return fIpFlags;
}


inline uint16 BcmEthernetPacket::FragmentOffsetParsed(void) const
{
	return fIpFragmentOffset;
}


inline uint8 BcmEthernetPacket::IpTypeOfServiceParsed(void) const
{
    return fIpTypeOfService;
}


inline uint8 BcmEthernetPacket::IpProtocolTypeParsed(void) const
{
    return fIpProtocolType;
}

inline bool BcmEthernetPacket::IpProtocolTypeTcpUdpParsed(void) const
{
    return fIpProtocolTypeIsTcpUdp;
}


inline const BcmIpAddress &BcmEthernetPacket::SourceIpAddressParsed(void) const
{
    return fSourceIpAddress;
}


inline const BcmIpAddress &BcmEthernetPacket::DestIpAddressParsed(void) const
{
    return fDestIpAddress;
}

inline void BcmEthernetPacket::IpGenerated(void)
{
    fWasParsedMask |= kIpWasParsed;
}


inline void BcmEthernetPacket::IpV6Generated(void)
{
    fWasParsedMask |= kIpV6WasParsed;
}


inline bool BcmEthernetPacket::ParseTcpUdp(void)
{
    // Only do this if we have not already parsed the TCP/UDP header.
    if ((fWasParsedMask & kTcpUdpWasParsed) == 0)
    {
        ParseTcpUdpImpl();
    }
    
    // Return true if this is a TCP or UDP packet.
    //
    // PR3280 - We can only call this if the IP header was parsed successfully,
    // though.  If this is an ARP packet, for example, then we can't call this.
    if (fWasParsedMask & (kIpWasParsed | kIpV6WasParsed))
    {
        return IpProtocolTypeTcpUdpParsed();
    }

    return false;
}


inline uint16 BcmEthernetPacket::SourceTcpUdpPortParsed(void) const
{
    return fSourceTcpUdpPort;
}


inline uint16 BcmEthernetPacket::DestTcpUdpPortParsed(void) const
{
    return fDestTcpUdpPort;
}


// When generating a packet, after you fill in all of the UDP header
// fields, call this method to lock it in.  This simulates what would happen
// if you called ParseTcpUdp(), but it is faster.
inline void BcmEthernetPacket::UdpGenerated(void)
{
    fWasParsedMask |= kTcpUdpWasParsed;
}


// Pre-parses the TCP header so that you can call the ...Parsed() accessors.
//
// Returns false if this is not a TCP packet.
inline bool BcmEthernetPacket::ParseTcp(void)
{
    // Only do this if we have not already parsed the TCP header.
    if ((fWasParsedMask & kTcpWasParsed) == 0)
    {
        ParseTcpImpl();
    }

    // Return true if this is a TCP packet.
    //
    // PR3280 - We can only call this if the IP header was parsed successfully,
    // though.  If this is an ARP packet, for example, then we can't call this.
    if (fWasParsedMask & kIpWasParsed)
    {
        return (IpProtocolTypeParsed() == kTcp);
    }

    return false;
}

inline uint32 BcmEthernetPacket::TcpSequenceNumberParsed(void) const
{
    return fTcpSequenceNumber;
}

inline uint32 BcmEthernetPacket::TcpAckNumberParsed(void) const
{
    return fTcpAckNumber;
}

inline unsigned int BcmEthernetPacket::TcpHeaderLengthBytesParsed(void) const
{
    return fTcpHeaderLengthBytes;
}

inline bool BcmEthernetPacket::TcpOptionsPresentParsed(void) const
{
    if (fTcpHeaderLengthBytes > 20)
    {
        return true;
    }

    return false;
}

inline uint8 *BcmEthernetPacket::TcpOptionsPointer(void) const
{
    if (fTcpHeaderLengthBytes > 20)
    {
		return((uint8*)&Buffer()[fTcpUdpHeaderOffset+20]);
	}
	else
	{
		return(NULL);
	}
}

inline uint8 BcmEthernetPacket::TcpFlagsParsed(void) const
{
    return fTcpFlags;
}


inline bool BcmEthernetPacket::TcpFlagIsSetParsed(TcpFlagBits flag) const
{
    if ((fTcpFlags & flag) != 0)
    {
        return true;
    }

    return false;
}


inline uint16 BcmEthernetPacket::TcpWindowSizeParsed(void) const
{
    return fTcpWindowSize;
}


inline uint16 BcmEthernetPacket::TcpUrgentPointerParsed(void) const
{
    return fTcpUrgentPointer;
}


// When generating a packet, after you fill in all of the TCP header
// fields, call this method to lock it in.  This simulates what would happen
// if you called ParseTcp(), but it is faster.
inline void BcmEthernetPacket::TcpGenerated(void)
{
    fWasParsedMask |= kTcpWasParsed;
}


inline uint8 BcmEthernetPacket::IcmpTypeParsed(void) const
{
    return fIcmpType;
}

inline uint8 BcmEthernetPacket::IcmpCodeParsed(void) const
{
    return fIcmpCode;
}

inline uint16 BcmEthernetPacket::IcmpIdentifierParsed(void) const
{
    return fIcmpIdentifier;
}

inline uint16 BcmEthernetPacket::IcmpSequenceNumberParsed(void) const
{
    return fIcmpSequenceNumber;
}

inline unsigned long BcmEthernetPacket::BcmPacketId(void) const
{
    return fBcmPacketId;
}

// Pre-parses the ICMP header so that you can call the ...Parsed() accessors.
//
// Returns false if this is not a TCP packet.
inline bool BcmEthernetPacket::ParseIcmp(void)
{
    // Only do this if we have not already parsed the TCP header.
    if ((fWasParsedMask & kIcmpWasParsed) == 0)
    {
        ParseIcmpImpl();
    }

    // Return true if this is a TCP packet.
    //
    // PR3280 - We can only call this if the IP header was parsed successfully,
    // though.  If this is an ARP packet, for example, then we can't call this.
    if (fWasParsedMask & kIpWasParsed)
    {
        return (IpProtocolTypeParsed() == kIcmp);
    }
    return false;
}

// Pre-parses the ICMP header so that you can call the ...Parsed() accessors.
//
// Returns false if this is not a TCP packet.
inline bool BcmEthernetPacket::ParseIcmpV6(void)
{
    // Only do this if we have not already parsed the TCP header.
    if ((fWasParsedMask & kIcmpV6WasParsed) == 0)
    {
        ParseIcmpV6Impl();
    }

    // Return true if this is an ICMP packet.
    //
    // PR3280 - We can only call this if the IP header was parsed successfully,
    // though.  If this is an ARP packet, for example, then we can't call this.
    if (fWasParsedMask & kIpV6WasParsed)
    {
        return (IpProtocolTypeParsed() == kIcmpV6);
    }
    return false;
}

inline unsigned int BcmEthernetPacket::IpV6UnfragmentLengthBytesParsed(void) const
{
    return fIpV6UnfragmentableLengthBytes;
}

inline uint32 BcmEthernetPacket::IpV6IdentificationParsed(void) const
{
	return fIpV6Identification;
}

#endif


