//****************************************************************************
//
// Copyright (c) 2007 Broadcom Corporation
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
//  Filename:       DhcpOptionsRfc2132.h
//  Author:         John McQueen
//  Creation Date:  June 7, 2002
//
//****************************************************************************

#ifndef DHCPOPTIONSRFC2132_H
#define DHCPOPTIONSRFC2132_H

//********************** Include Files ***************************************

//********************** Global Types ****************************************

//********************** Global Constants ************************************


/** \ingroup DhcpClientServer
*
*   This file contains common constants for DHCP, as well as utility
*   functions for converting the constants to text strings (for debugging
*   purposes).
*/

/// Various constants used by the DHCP Client and Server.
enum
{
    /// Always the first 4 bytes of the options field.
    kDhcpMagicCookie = 0x63825363,

    /// Max lengths for some fields per RFC2131/RFC2132
    kMaxServerNameLength = 64,
    kMaxFileNameLength = 128,

    /// The maximum number of bytes that can be sent as options, including the
    /// use of the sname and file fields as option overloads.  This is derived
    /// from the fact that the largest IP datagram that we can send is 1500 bytes
    /// (1518 including LLC header and CRC32); the minimum IP datagram that MUST
    /// be supported is 576 bytes, and with this minimum size, we must be able
    /// to support a minimum of 312 bytes of options.  Additionally, the file
    /// and sname fields can be used to hold options.
    kMaxDhcpOptionsLength = (1500 - 576 + 312 + 64 + 128)
};


/// DHCP Option type codes, mostly from RFC2132.
typedef enum
{
  /* Rfc 1497 Vendor Extensions */
  kDhcpPadOption                = 0,        // 0x00
  kDhcpSubnetMask               = 1,        // 0x01
  kDhcpTimeOffset               = 2,        // 0x02
  kDhcpRouter                   = 3,        // 0x03
  kDhcpTimeServer               = 4,        // 0x04
  kDhcpNameServer               = 5,        // 0x05
  kDhcpDomainNameServer         = 6,        // 0x06
  kDhcpLogServer                = 7,        // 0x07
  kDhcpCookieServer             = 8,        // 0x08
  kDhcpLprServer                = 9,        // 0x09
  kDhcpImpressServer            = 10,       // 0x0a
  kDhcpResourceLocationServer   = 11,       // 0x0b
  kDhcpHostName                 = 12,       // 0x0c
  kDhcpBootFileSize             = 13,       // 0x0d
  kDhcpMeritDumpFile            = 14,       // 0x0e
  kDhcpDomainName               = 15,       // 0x0f
  kDhcpSwapServer               = 16,       // 0x10
  kDhcpRootPath                 = 17,       // 0x11
  kDhcpExtensionsPath           = 18,       // 0x12
  
  /* Ip Layer Parameters per Host */
  kDhcpIpForwarding             = 19,       // 0x13
  kDhcpNonLocalSourceRouting    = 20,       // 0x14
  kDhcpPolicyFilter             = 21,       // 0x15
  kDhcpMaxDataReassemblySize    = 22,       // 0x16
  kDhcpDefaultIpTTL             = 23,       // 0x17
  kDhcpPathMtuAgingTimeout      = 24,       // 0x18
  kDhcpPathMtuPalteauTable      = 25,       // 0x19

  /* Ip Layer Parameters per Interface */
  kDhcpInterfaceMTU             = 26,       // 0x1a
  kDhcpAllSubnetsAreLocal       = 27,       // 0x1b
  kDhcpBroadcastAddress         = 28,       // 0x1c
  kDhcpPerformMaskDiscovery     = 29,       // 0x1d
  kDhcpMaskSupplier             = 30,       // 0x1e
  kDhcpPerformRouterDiscovery   = 31,       // 0x1f
  kDhcpRouterSolicitationAddress = 32,       // 0x20
  kDhcpStaticRoute              = 33,       // 0x21

  /* Link Layer Parameters per Interface */
  kDhcpTrailerEncapsulation     = 34,       // 0x22
  kDhcpArpCacheTimeout          = 35,       // 0x23
  kDhcpEthernetEncapsulation    = 36,       // 0x24

  /* Tcp Parameters */
  kDhcpTcpDefaultTTL            = 37,       // 0x25
  kDhcpTcpKeepaliveInterval     = 38,       // 0x26
  kDhcpTcpKeepaliveGarbage      = 39,       // 0x27

  /* Application and Service Parameters */
  kDhcpNetworkInfoServiceDomain = 40,       // 0x28
  kDhcpNetworkInfoServers       = 41,       // 0x29
  kDhcpNetworkTimeProtoServers  = 42,       // 0x2a
  kDhcpVendorSpecInfo           = 43,       // 0x2b
  kDhcpNetBiosTcpipNameServer   = 44,       // 0x2c
  kDhcpNetBiosTcpipDistServer   = 45,       // 0x2d
  kDhcpNetBiosTcpipNodeType     = 46,       // 0x2e
  kDhcpNetBiosTcpipScope        = 47,       // 0x2f
  kDhcpXWindowSysFontServer     = 48,       // 0x30
  kDhcpXWindowSysDisplayManager = 49,       // 0x31
  kDhcpNetworkInfoServicePlusDomain = 64,   // 0x40
  kDhcpNetworkInfoServicePlusServer = 65,   // 0x41
  kDhcpMobileIpHomeAgent        = 68,       // 0x44
  kDhcpSmtpServer               = 69,       // 0x45
  kDhcpPop3Server               = 70,       // 0x46
  kDhcpNntpServer               = 71,       // 0x47
  kDhcpWwwServer                = 72,       // 0x48
  kDhcpDefaultFingerServer      = 73,       // 0x49
  kDhcpDefaultIrcServer         = 74,       // 0x4a
  kDhcpStreetTalkServer         = 75,       // 0x4b
  kDhcpStdaServer               = 76,       // 0x4c
  
  /* Dhcp Extensions */
  kDhcpRequestedIpAddress       = 50,       // 0x32
  kDhcpIpAddressLeaseTime       = 51,       // 0x33
  kDhcpOptionOverload           = 52,       // 0x34
  kDhcpDhcpMessageType          = 53,       // 0x35
  kDhcpServerIdentifier         = 54,       // 0x36
  kDhcpParamRequestList         = 55,       // 0x37
  kDhcpMessage                  = 56,       // 0x38
  kDhcpMaxDhcpMessageSize       = 57,       // 0x39
  kDhcpRenewalT1TimeValue       = 58,       // 0x3a
  kDhcpRebindingT2TimeValue     = 59,       // 0x3b
  kDhcpVendorClassIdentifier    = 60,       // 0x3c
  kDhcpClientIdentifier         = 61,       // 0x3d
  kDhcpTftpServerName           = 66,       // 0x42
  kDhcpBootFileName             = 67,       // 0x43

  // new to Docsis3.0 - see RFC3925
  kDhcpVendorIdentifyingVendorSpecificInformation = 125,  // 0x7d
        kDhcpCableLabsOptionRequestOption        = 1,
        kDhcpCableLabsTftpServersOption          = 2,
        kDhcpCableLabsModemCapabilitiesOption    = 5,

  /* Additional Option Codes */
  kDhcpRelayAgentInfo           = 82,       // 0x52
  kDhcpPacketCableServers       = 122,      // 0x7a, alias for the application
  kDhcpClV4OptionCccV6          = 123,      // DHCPv4 CableLabs Client Configuration Option for DHCPv6 Server Addresses
  kDhcpClIpAddrModePreference   = 124,      // DHCPv4 CableLabs IP addressing mode preference DHCP Option
  kDhcpOption177                = 177,      // 0xb1
  
  // NOTE: PR9131 CH1.1-N-05.0223-2, Change DHCP option 177 to option 122
  #if 1 //PR9131  Enable option 122 instead of the old option 177
  kDhcpOption122                = 122,      // 0x7a
  kDhcpCableHomeServers         = 122,      // 0x7a, alias for the application
  #else
  kDhcpCableHomeServers         = 177,      // 0xb1, alias for the application
  #endif
  
  kDhcpInterconnection          = 178,      // 0xb2
  kDhcpLogicalElement           = 179,      // 0xb3
  kDhcpSecondaryDns             = 254,      // 0xfe
  kDhcpEndOption                = 255       // 0xff

} DhcpRfc2132OptionCodes;


/// DHCP Sub-option type codes for the PacketCable and CableHome options.
typedef enum
{
    // These are valid for the kDhcpPacketCableServers parent option.
    kDhcpPacketCablePrimaryDhcpServerAddress        = 1,        // 0x01
    kDhcpPacketCableSecondaryDhcpServerAddress      = 2,        // 0x02
    kDhcpPacketCableSnmpEntity                      = 3,        // 0x03
    kDhcpPacketCableASreqestASreply                 = 4,        // 0x04
    kDhcpPacketCableAPrequestAPreply                = 5,        // 0x05
    kDhcpPacketCableKerberosRealmName               = 6,        // 0x06
    kDhcpPacketCableTicketGrantingServerUtilization = 7,        // 0x07
    kDhcpPacketCableProvisioningTimer               = 8,        // 0x08
    kDhcpPacketCableSecurityTicketInvalidations     = 9,        // 0x09
    
    // These are valid for the kDhcpOption177 parent option.
    kDhcpOption177PrimaryDhcpServerAddress    = 1,        // 0x01
    kDhcpOption177SecondaryDhcpServerAddress  = 2,        // 0x02
    kDhcpOption177SnmpEntity                  = 3,        // 0x03
    kDhcpOption177PrimaryDns                  = 4,        // 0x04
    kDhcpOption177SecondaryDns                = 5,        // 0x05
    kDhcpOption177PrimaryKerberosRealm        = 6,        // 0x06
    kDhcpOption177SecondaryKerberosRealm      = 7,        // 0x07
    kDhcpOption177ProvisioningTimer           = 8,        // 0x08
    kDhcpOption177PrimaryLineCms              = 9,        // 0x09

    // These are valid for the kDhcpCableHomeServers parent option.
    kDhcpCableHomeSnmpEntity                    = 3,        // 0x03
    kDhcpCableHomeKerberosRealm                 = 6,        // 0x06
    kDhcpCableHomeDisabledMode                  = 15,       // 0x0f
    #if 1  //PR9131  Enable option 122 instead of the old option 177
    kDhcpCableHomeKerberosServer                = 10,       // 0x0a
    #else 
    kDhcpCableHomeKerberosServer                = 51,       // 0x33
    #endif

    // These are valid for the kDhcpClV4OptionCccV6 parent option.
    kDhcpPrimaryDhcpV6ServerSelectorID          = 1,        // 0x01
    kDhcpSecondaryDhcpV6ServerSelectorID        = 2,        // 0x02

} DhcpRfc2132SubOptionCodes;


/// DHCP Option53 values from RFC2132.
typedef enum
{
    // This is just a place holder for uninitialized messages.
    kUnknownDhcpMessageType = 0,

    // These are in RFC2132.
    kDhcpDiscover = 1,
    kDhcpOffer = 2,
    kDhcpRequest = 3,
    kDhcpDecline = 4,
    kDhcpAck = 5,
    kDhcpNak = 6,
    kDhcpRelease = 7,
    kDhcpInform = 8
} DhcpMessageTypeOption53;


/// Bootp OP Field values.
typedef enum
{
    kDhcpBootRequest = 1,
    kDhcpBootReply = 2
} BootpOpFieldValues;


//********************** Global Functions ************************************

/// Given an enum value from the DHCP Option Codes, this returns the text name.
///
/// \param
///      optionCode - the DHCP Option Codes value.
///
/// \return
///      The text name of the option code.
///
const char *DhcpRfc2131OptionCodeName(DhcpRfc2132OptionCodes optionCode);

/// Given an enum value from the DHCP SubOption Codes, this returns the text name.
///
/// \param
///      subOptionCode - the DHCP SubOption Codes value.
///
/// \return
///      The text name of the suboption code.
///
const char *DhcpRfc2132SubOptionCodeName(DhcpRfc2132SubOptionCodes subOptionCode);

#endif

