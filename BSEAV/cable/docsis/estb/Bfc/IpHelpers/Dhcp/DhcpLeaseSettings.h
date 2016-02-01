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
//  Filename:       DhcpLeaseSettings.h
//  Author:         David Pullen
//  Creation Date:  June 8, 2002
//
//****************************************************************************

#ifndef DHCPLEASESETTINGS_H
#define DHCPLEASESETTINGS_H

//********************** Include Files ***************************************

#include "IpAddress.h"
#include "IpAddressList.h"
#include "MacAddress.h"
#include "OctetBuffer.h"

#include "DhcpOptionsRfc2132.h"

#include "DhcpClientId.h"

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iostream>
#else
#include <iostream.h>
#endif

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


/** \ingroup DhcpClientServer
*
*   This is the class that stores all of the relevant information that appears
*   inside a DHCP packet, including most of the fixed fields in the
*   BOOTP-compatible header (starting with ciaddr), and TLV encoded Option
*   fields described in RFC2132.
*
*   Dec 12, 2003 - added support for RFC3396, which specifies requirements
*   for handling DHCP options that are larger than 255 bytes.  This object
*   automatically splits the option values up for you when an option is
*   being added or replaced that is larger than 255 bytes.  On the parsing
*   side, we automatically concatenate multiple option values with the
*   same type code together into a single large buffer.
*/
class BcmDhcpLeaseSettings
{
public:

    /// Default constructor.  Just sets everything to a nice clean state.
    ///
    BcmDhcpLeaseSettings(void);

    /// Copy constructor.  Does a deep copy of the specified instance's data
    /// members.
    ///
    /// \param
    ///      otherInstance - reference to the object to copy.
    /// 
    BcmDhcpLeaseSettings(const BcmDhcpLeaseSettings &otherInstance);

    /// Destructor.  Frees memory that was previously allocated.
    ///
    virtual ~BcmDhcpLeaseSettings();

    /// Assignment operator.  Does a deep copy of the specified instance's data
    /// members.
    ///
    /// \param
    ///      otherInstance - reference to the object to copy.
    ///
    /// \return
    ///      A reference to "this" so that operator = can be chained.
    ///
    BcmDhcpLeaseSettings & operator = (const BcmDhcpLeaseSettings &otherInstance);

    /// Equality comparison operation.  Checks to see if this object has the
    /// same values as the instance specified.
    ///
    /// \param
    ///      otherInstance - reference to the instance to be compared against.
    ///
    /// \retval
    ///      true if they are equal.
    /// \retval
    ///      false if not equal.
    ///
    bool operator == (const BcmDhcpLeaseSettings &otherInstance) const;

    /// Non-equality comparison operation.  Checks to see if this object does
    /// not have the same values as the instance specified.
    ///
    /// \param
    ///      otherInstance - reference to the instance to be compared against.
    ///
    /// \retval
    ///      true if they are not equal.
    /// \retval
    ///      false if they are equal.
    ///
    inline bool operator != (const BcmDhcpLeaseSettings &otherInstance) const;

    /// Resets the object to its default state, as though it had just been
    /// created.
    ///
    void Reset(void);

    /// Checks a few things to see if the configured settings are valid.  For
    /// example, Option 53 (DHCP Message Type) must be present.  Also, we can
    /// do some basic checking of the option fields for validity.
    ///
    /// Note that this says nothing about whether or not the contents of the
    /// settings are meaningful or usable, just that they are formatted
    /// correctly.
    ///
    /// \retval
    ///      true if the settings are valid.
    /// \retval
    ///      false if the settings are not valid.
    ///
    bool IsValid(void) const;

    /// Make sure the size of the buffer is legal.  In some cases may have
    /// restrictions (such as must be 4, or must be multiple of 4).
    ///
    /// Parameters:  DhcpRfc2132OptionCodes optionCode.
    ///              unsigned int optionLen.
    ///
    /// Returns:
    ///      true if the settings are valid.
    ///      false if the settings are not valid.
    ///
    bool IsOptionLenValid(DhcpRfc2132OptionCodes optionCode,
                          unsigned int optionLen) const;

    /// Does formatted output of the settings to the specified ostream.
    ///
    /// \param
    ///      outputStream - the ostream to be used.
    ///
    /// \return
    ///      The reference to the ostream that was passed in.
    ///
    ostream & Print(ostream &outputStream) const;

public:  // Accessors for the BOOTP-compatible fields.  In all cases, I have
         // provided versions that are consistent with RFC2131 along with
         // versions that are more human-readable.

    //@{
    /// Accessors for the htype (hardware type) field.  This affects
    /// interpretation of the chaddr field.  For now, this should always be set
    /// to 1 (ethernet).  If we decide to support others later, then we will
    /// change it.
    ///
    /// \param
    ///      hardwareType - the hardware type to use.
    ///
    /// \return
    ///      The hardware type.
    ///
    void HardwareType(uint8 hardwareType);
    inline uint8 HardwareType(void) const;

    inline void htype(uint8 hardwareType);
    inline uint8 htype(void) const;
    //@}

    //@{
    /// Accessors for the hlen (hardware length) field.  This affects
    /// interpretation of the chaddr field.  For now, this should always be set
    /// to 6 (length of a mac address).  If we decide to support others later,
    /// then we will change it.
    ///
    /// \param
    ///      hardwareLength - the hlen value to use.
    ///
    /// \return
    ///      The hlen value.
    ///
    void HardwareLength(uint8 hardwareLength);
    inline uint8 HardwareLength(void) const;

    inline void hlen(uint8 hardwareLength);
    inline uint8 hlen(void) const;
    //@}

    //@{
    /// Accessors for the ciaddr (Client IP Address) field.  This should only
    /// be set by the DHCP Client thread when sending a packet to the DHCP
    /// server (and only when this is not a DISCOVER message).
    ///
    /// \param
    ///      ipAddr - the IP address to use.
    ///
    /// \return
    ///      The IP address.
    ///
    void ClientIpAddress(const BcmIpAddress &ipAddr);
    inline const BcmIpAddress &ClientIpAddress(void) const;

    inline void ciaddr(const BcmIpAddress &ipAddr);
    inline const BcmIpAddress &ciaddr(void) const;
    //@}

    //@{
    /// Accessors for the yiaddr (Your IP Address) field.  This will be set by
    /// the DHCP server when sending OFFER and BIND packets.
    ///
    /// \param
    ///      ipAddr - the IP address to use.
    ///
    /// \return
    ///      The IP address.
    ///
    void YourIpAddress(const BcmIpAddress &ipAddr);
    inline const BcmIpAddress &YourIpAddress(void) const;

    inline void yiaddr(const BcmIpAddress &ipAddr);
    inline const BcmIpAddress &yiaddr(void) const;
    //@}

    //@{
    /// Accessors for the siaddr (Server IP Address) field.
    ///
    /// \param
    ///      ipAddr - the IP address to use.
    ///
    /// \return
    ///      The IP address.
    ///
    void ServerIpAddress(const BcmIpAddress &ipAddr);
    inline const BcmIpAddress &ServerIpAddress(void) const;

    inline void siaddr(const BcmIpAddress &ipAddr);
    inline const BcmIpAddress &siaddr(void) const;
    //@}

    //@{
    /// Accessors for the TFTP server IP address list.
    ///
    /// \param
    ///      ipAddr - the IP address list to use.
    ///
    /// \return
    ///      The IP address.
    ///
    void TftpServerIpAddressList(const BcmIpAddressList &ipAddr);
    inline const BcmIpAddressList &TftpServerIpAddressList(void) const;
    //@}

    //@{
    /// Accessors for the giaddr (Gateway/Relay Agent IP Address) field.
    ///
    /// \param
    ///      ipAddr - the IP address to use.
    ///
    /// \return
    ///      The IP address.
    ///
    void RelayAgentIpAddress(const BcmIpAddress &ipAddr);
    inline const BcmIpAddress &RelayAgentIpAddress(void) const;

    inline void giaddr(const BcmIpAddress &ipAddr);
    inline const BcmIpAddress &giaddr(void) const;
    //@}

    //@{
    /// Accessors for the chaddr (Client Hardware Address) field.
    ///
    /// Because we deal exclusively with ethernet-based networks, we will only
    /// handle ethernet MAC addresses for the chaddr field.  If we want to make
    /// this more flexible later, we can do so.
    ///
    /// \param
    ///      hardwareAddress - the hardware address to use.
    ///
    /// \return
    ///      The hardware address.
    ///
    void ClientHardwareAddress(const BcmMacAddress &hardwareAddress);
    inline const BcmMacAddress &ClientHardwareAddress(void) const;

    inline void chaddr(const BcmMacAddress &hardwareAddress);
    inline const BcmMacAddress & chaddr(void) const;
    //@}

    //@{
    /// Accessors for the sname (Server Name) field.  This is an ASCII string
    /// with NULL terminator, up to 64 chars (including terminator).
    ///
    /// \param
    ///      pServerName - string containing the server name to use.
    ///
    /// \return
    ///      The server name, either as a string or as the octet buffer in which
    ///      the name is stored.
    ///
    void ServerName(const char *pServerName);
    inline const char *ServerName(void) const;
    inline const BcmOctetBuffer &ServerNameBuffer(void) const;

    inline void sname(const char *pServerName);
    inline const char *sname(void) const;
    //@}

    //@{
    /// Accessors for the file field.  This is an ASCII string with NULL
    /// terminator up to 128 chars (including terminator).
    ///
    /// Due to popular demand, the const char *FileName(void) and file(void)
    /// accessors have been made a tad more intelligent.  The "bootfile name"
    /// can be contained either here or within option 67.  Since the client
    /// really only cares about reading the filename (and some DHCP servers
    /// prohibit using the file field), we will return the value from the file
    /// field if present, or the value in option 67, if present.  In this case,
    /// the returned string will still be NULL terminated, but can be up to 255
    /// characters.
    ///
    /// Note that the const BcmOctetBuffer &FileNameBuffer(void) accessor has
    /// not changed; it always returns the octet buffer corresponding to the
    /// file field.
    ///
    /// \param
    ///      pFileName - string containing the file name to use.
    ///
    /// \return
    ///      The file name, either as a string or as the octet buffer in which
    ///      the name is stored.
    ///
    void FileName(const char *pFileName);
    inline const char *FileName(void) const;
    inline const BcmOctetBuffer &FileNameBuffer(void) const;

    inline void file(const char *pFileName);
    inline const char *file(void) const;
    //@}

    //@{
    /// Accessors for the entire options field.  When setting, the required
    /// magic cookie and 0xff end option must be present.  When getting, these
    /// values will be present.
    ///
    /// \param
    ///      options - octet buffer containing the option values to be used.
    ///
    /// \return
    ///      The option values to be used.
    ///
    void Options(const BcmOctetBuffer &options);
    inline const BcmOctetBuffer &Options(void) const;
    //@}

    /// This method will clear the options field and reset the buffer to still
    /// contain the DHCP magic cookie and 0xff end option
    ///
    void ResetOptions(void);

    /// This is a static helper method that can be called externally to parse
    /// the DHCP options and return the offset of the desired option.  This is
    /// used by several Snoop objects that need to look at the type of DHCP
    /// packet (Discover, Offer, etc.).
    ///
    /// Note that this only returns the offset of the first instance of the
    /// specified option; RFC3396 allows multiple instances to be present.
    ///
    /// \param
    ///      optionCode - one of the DhcpRfc2132OptionCodes enum values.
    /// \param
    ///      pBuffer - pointer to the buffer.  This is the start of the options,
    ///                and must contain the magic cookie value.
    /// \param
    ///      numberOfBytes - the number of bytes of option data in the buffer.
    ///
    /// \return
    ///      The offset within the buffer that the specified option exists.  If
    ///      the option isn't found, then this returns numberOfBytes.
    ///
    static unsigned int FindOptionOffset(DhcpRfc2132OptionCodes optionCode,
                                         const uint8 *pBuffer,
                                         unsigned int numberOfBytes);

    /// This is generally called only by BcmDhcpLeaseSettings after it has
    /// finished parsing the DHCP packet and storing the values into my data
    /// members.
    ///
    /// It causes this object to look for and process Option 52.  This option
    /// allows the "file" and "sname" fields of the bootp header to be used to
    /// carry extra options data.  If Option 52 is found in the options buffer,
    /// then we check the bits to see which of these fields have been overloaded
    /// and move the option data into the options buffer (since it isn't size
    /// constrained).  The overloaded field will then be emptied and Option 52
    /// will be removed from the options list.
    ///
    /// Additionally, we check for the bootfile name in either the file field or
    /// in Option 67, and copy it (NULL terminating as needed) to a separate
    /// buffer that is returned by the FileName() accessor.
    ///
    /// It also does a validity check on the settings.
    ///
    /// \retval
    ///      true if the stored settings are reasonably valid.
    /// \retval
    ///      false if the settings are not valid or we failed to process Option
    ///          52 for some reason.
    ///
    bool ParseComplete(void);

public:  // Accessors for the DHCP options.  The option codes can be found in
         // the enum DhcpRfc2132OptionCodes in DhcpOptionsRfc2132.h.

    //@{
    /// Adds an option (or replaces the existing value if the option already
    /// exists).  There are multiple formats to the option data; generically, it
    /// will be stored as an octet buffer, but it can be accessed via several
    /// common formats (IP address, uint8/16/32, etc).
    ///
    /// We have now added support for RFC3396, which allows a single option to
    /// be longer than 255 bytes.  To handle this case, we will automatically
    /// split the option value up into chunks of 255 bytes or less, and will
    /// add multiple instances of the option type code, each one containing part
    /// of the value.
    ///
    /// If the option already exists and is split, all of the existing instances
    /// will be removed, and the new value will be split (if necessary) and
    /// added.
    ///
    /// \param
    ///      optionCode - one of the DhcpRfc2132OptionCodes enum values.
    /// \param
    ///      optionValue - the value to be set.
    ///
    /// \retval
    ///      true if successful.
    /// \retval
    ///      false if there was a problem; usually this means the length is
    ///          too long or not valid for the option code.
    ///
    bool AddReplaceOption(DhcpRfc2132OptionCodes optionCode,
                          const BcmOctetBuffer &optionValue);

    bool AddReplaceOption(DhcpRfc2132OptionCodes optionCode,
                          const BcmIpAddress &optionValue);
    
    bool AddReplaceOption(DhcpRfc2132OptionCodes optionCode,
                          const BcmIpAddressList &optionValue);
    
    bool AddReplaceOption(DhcpRfc2132OptionCodes optionCode,
                          const BcmMacAddress &optionValue);
    
    bool AddReplaceOption(DhcpRfc2132OptionCodes optionCode,
                          uint8 optionValue);
    
    bool AddReplaceOption(DhcpRfc2132OptionCodes optionCode,
                          uint16 optionValue);
    
    bool AddReplaceOption(DhcpRfc2132OptionCodes optionCode,
                          uint32 optionValue);
    
    bool AddReplaceOption(DhcpRfc2132OptionCodes optionCode,
                          int32 optionValue);
    //@}

    /// Helper method for option 61 (Client-identifier).
    ///
    /// \param
    ///      clientId - the value to be used as the client id for option 61.
    ///
    /// \retval
    ///      true if successful.
    /// \retval
    ///      false if there was a problem.
    ///
    bool AddReplaceOption61(const BcmDhcpClientId &clientId);

    /// Removes an option if it exists.
    ///
    /// For RFC3396 support, if multiple instances of the option exist (each
    /// carrying part of the option value), then all instances will be removed.
    ///
    /// \param
    ///      optionCode - one of the DhcpRfc2132OptionCodes enum values.
    ///
    /// \retval
    ///      true if the option was present and was removed.
    /// \retval
    ///      false if the option was not present.
    ///
    bool RemoveOption(DhcpRfc2132OptionCodes optionCode);

    /// Returns true if the specified option exists.
    ///
    /// \param
    ///      optionCode - one of the DhcpRfc2132OptionCodes enum values.
    ///
    /// \retval
    ///      true if the option is present.
    /// \retval
    ///      false if the option is not present.
    ///
    bool OptionIsPresent(DhcpRfc2132OptionCodes optionCode) const;

    /// Returns true if the specified suboption of the specified option exists.
    ///
    /// \param
    ///      optionCode - one of the DhcpRfc2132OptionCodes enum values.
    /// \param
    ///      suboptionCode - one of the DhcpRfc2132SubOptionCodes enum values.
    ///
    /// \retval
    ///      true if the option and suboption are present.
    /// \retval
    ///      false if the option is not present, or if the option is present but
    ///          the suboption is not present.
    ///
    bool SubOptionIsPresent(DhcpRfc2132OptionCodes optionCode,
                            DhcpRfc2132SubOptionCodes suboptionCode) const;

    //@{
    /// Retrieves the data associated with the specified option code, if it
    /// exists.  There are multiple formats to the option data; generically, it
    /// can be returns as an octet buffer, but it can also be accessed via
    /// several common formats (IP address, uint8/16/32, etc).
    ///
    /// For RFC3396 support, note that the option value can be more than 255
    /// bytes in length, and can span multiple instances of the option code.
    /// This method automatically concatenates all of the instances and treats
    /// them as if they were a single option value.
    ///
    /// \param
    ///      optionCode - one of the DhcpRfc2132OptionCodes enum values.
    /// \param
    ///      optionValue - storage for the value to be returned.
    ///
    /// \retval
    ///      true if successful.
    /// \retval
    ///      false if there was a problem; usually this either means the option
    ///          was not present, or the length of the value is not right
    ///          given the data variable that was passed in.
    ///
    bool GetOption(DhcpRfc2132OptionCodes optionCode,
                   BcmOctetBuffer &optionValue) const;

    bool GetOption(DhcpRfc2132OptionCodes optionCode,
                   BcmIpAddress &optionValue) const;
    
    bool GetOption(DhcpRfc2132OptionCodes optionCode,
                   BcmIpAddressList &optionValue) const;

    bool GetOption(DhcpRfc2132OptionCodes optionCode,
                   BcmMacAddress &optionValue) const;
    
    bool GetOption(DhcpRfc2132OptionCodes optionCode,
                   uint8 &optionValue) const;
    
    bool GetOption(DhcpRfc2132OptionCodes optionCode,
                   uint16 &optionValue) const;
    
    bool GetOption(DhcpRfc2132OptionCodes optionCode,
                   uint32 &optionValue) const;
    
    bool GetOption(DhcpRfc2132OptionCodes optionCode,
                   int32 &optionValue) const;
    //@}

    /// Helper method for option 61 (Client-identifier).
    ///
    /// \param
    ///      clientId - storage for the value to be returned
    ///
    /// \retval
    ///      true if successful.
    /// \retval
    ///      false if there was a problem; usually this either means the option
    ///          was not present, or the length is of the value is not right
    ///          given the data variable that was passed in.
    ///
    bool GetOption61(BcmDhcpClientId &clientId) const;

    /// For option 1 (Subnet Mask Option), we often want to retrieve the value
    /// from the settings or select the default subnet mask based on the class
    /// of the IP address that we have been given.  This method helps with this.
    ///
    /// \note  The yiaddr/fYourIpAddress field must be configured in order for
    ///        this to work.
    ///
    /// \return
    ///      The subnet mask, either the value from option 1 or the value
    ///      calculated from the yiaddr field.
    ///
    BcmIpAddress GetOption1SubnetMask(void) const;

    /// For option 3 (Router Option), we often seen the servers provisioned with
    /// all of the routers that are available, regardless of whether they are
    /// on-net or off-net.  The client usually just wants the first router that
    /// is on its subnet so that it can set the default gatway address.  This
    /// method helps with this.
    ///
    /// \note  The yiaddr/fYourIpAddress field and subnet mask must be
    ///        configured in order for this to work.  If they aren't configured,
    ///        then you are likely to get just any old router address back.
    ///
    /// \param
    ///      ipAddress - storage for the value to be returned.
    ///
    /// \retval
    ///      true if successful.
    /// \retval
    ///      false if there was a problem; usually this means the option was not
    ///          present.
    ///
    bool GetOption3OnNetRouter(BcmIpAddress &ipAddress) const;

    //@{
    /// For some DHCP options, the value is a set of suboptions (Option 122/177
    /// is a prime example for PacketCable/CableHome).  This method helps
    /// clients fish around for the suboptions.
    ///
    /// \param
    ///      optionCode - one of the DhcpRfc2132OptionCodes enum values.
    /// \param
    ///      suboptionCode - one of the DhcpRfc2132SubOptionCodes enum values.
    /// \param
    ///      optionValue - storage for the value to be returned.
    ///
    /// \retval
    ///      true if successful.
    /// \retval
    ///      false if there was a problem; usually this either means the option
    ///          or suboption was not present, or the length of the value is not
    ///          right given the data variable that was passed in.
    ///
    bool GetSubOption(DhcpRfc2132OptionCodes optionCode,
                      DhcpRfc2132SubOptionCodes suboptionCode,
                      BcmOctetBuffer &optionValue) const;
    
    bool GetSubOption(DhcpRfc2132OptionCodes optionCode,
                      DhcpRfc2132SubOptionCodes suboptionCode,
                      BcmIpAddressList &optionValue) const;
    //@}

    /// Helper for option 53 - the DHCP message type.
    ///
    /// \return
    ///      One of the DhcpMessageTypeOption53 enum values.
    ///
    DhcpMessageTypeOption53 GetDhcpMessageType(void) const;


    //@{
    /// Accessors for data member fIsYourIpThePrimaryIpAddress.  
    ///
    inline void IsYourIpThePrimaryIpAddress( bool isPrimary );
    inline bool IsYourIpThePrimaryIpAddress( void ) const;
    void AssignButPreservePrimaryFlag(BcmDhcpLeaseSettings *pDhcpSettings);
    //@}
    
    inline unsigned long CreationTime( void ) const;
    inline void CreationTime( unsigned long Time );

private:

    // These are the BOOTP-compatible fields.

    /// The hardware type and hardware length (htype/hlen) values.  These
    /// affect the interpretation of the chaddr field.
    uint8 fHardwareType;
    uint8 fHardwareLength;

    /// The ciaddr field.  Set by the client during messages in bound/renew/
    /// rebinding state, otherwise 0.0.0.0
    BcmIpAddress fClientIpAddress;

    /// The yiaddr field.  Returned by the DHCP server.
    BcmIpAddress fYourIpAddress;

    /// The siaddr field.  The server address to be used for the next boot
    /// stage.  Most often, this is a TFTP server used to get some image or
    /// config file.
    BcmIpAddress fServerIpAddress;

    /// The IP address list contained in the TFTP Servers Addresses option 
    BcmIpAddressList fTftpServerIpAddressList;

    /// The giaddr field.  This is the gateway/relay agent address.  Not very
    /// useful for DHCP clients, but critical for DHCP servers.
    BcmIpAddress fRelayAgentIpAddress;

    /// The chaddr field.  This will be the hardware address of the client
    /// (usually the ethernet mac address).  This is a fixed 16 byte field, but
    /// only hlen bytes of it are valid.  The rest are 0-padded.
    ///
    /// Because we deal exclusively with ethernet-based networks, we will only
    /// handle ethernet MAC addresses for the chaddr field.
    BcmMacAddress fClientHardwareAddress;

    /// The sname field.  ASCII string containing the name of the server
    /// specified as the siaddr field.  This is a fixed 64 byte field.
    BcmOctetBuffer fServerName;

    /// The file field.  This is the name of the file that is to be loaded in
    /// the next stage of booting, either the image to load or the name of a
    /// config file to load from the siaddr server.  This is a fixed 128 byte
    /// field.
    BcmOctetBuffer fFileName;

    /// The bootfile name.  This value comes either from the fFileName field
    /// (if specified), or from Option 67 (if present).  This is the value
    /// that is returned by the const char *FileName(void) accessor.
    BcmOctetBuffer fBootfileName;

    /// These are the option fields.
    BcmOctetBuffer fDhcpOptions;

    /// True if fYourIpAddress is to be configured as the primary IP address 
    /// for the network interface.  The primary IP address is configured via 
    /// the "set IP address" ioctl command.  Additional non-primary IP addresses 
    /// may be added to the network interface via the "add IP address" 
    /// ioctl command.  Subnet mask, broadcast IP address, interface flags 
    /// and default router may be configured with the primary IP address.  
    /// default value is false.
    bool fIsYourIpThePrimaryIpAddress;
    
    unsigned long fCreationTime;
};


//********************** Inline Method Implementations ***********************


/// This allows you to do the following:
///
///    BcmDhcpLeaseSettings dhcpSettings;
///
///    cout << dhcpSettings << endl;
///
inline ostream & operator << (ostream &outputStream, const BcmDhcpLeaseSettings &settingsInstance)
{
    return settingsInstance.Print(outputStream);
}


inline bool BcmDhcpLeaseSettings::operator != (const BcmDhcpLeaseSettings &otherInstance) const
{
    // Return the opposite of operator ==
    if (*this == otherInstance)
    {
        return false;
    }

    return true;
}


inline uint8 BcmDhcpLeaseSettings::HardwareType(void) const
{
    return fHardwareType;
}

inline void BcmDhcpLeaseSettings::htype(uint8 hardwareType)
{
    HardwareType(hardwareType);
}

inline uint8 BcmDhcpLeaseSettings::htype(void) const
{
    return HardwareType();
}


inline uint8 BcmDhcpLeaseSettings::HardwareLength(void) const
{
    return fHardwareLength;
}

inline void BcmDhcpLeaseSettings::hlen(uint8 hardwareLength)
{
    HardwareLength(hardwareLength);
}

inline uint8 BcmDhcpLeaseSettings::hlen(void) const
{
    return HardwareLength();
}


inline const BcmIpAddress &BcmDhcpLeaseSettings::ClientIpAddress(void) const
{
    return fClientIpAddress;
}

inline void BcmDhcpLeaseSettings::ciaddr(const BcmIpAddress &ipAddr)
{
    ClientIpAddress(ipAddr);
}

inline const BcmIpAddress &BcmDhcpLeaseSettings::ciaddr(void) const
{
    return ClientIpAddress();
}


inline const BcmIpAddress &BcmDhcpLeaseSettings::YourIpAddress(void) const
{
    return fYourIpAddress;
}

inline void BcmDhcpLeaseSettings::yiaddr(const BcmIpAddress &ipAddr)
{
    YourIpAddress(ipAddr);
}

inline const BcmIpAddress &BcmDhcpLeaseSettings::yiaddr(void) const
{
    return YourIpAddress();
}

inline const BcmIpAddress &BcmDhcpLeaseSettings::ServerIpAddress(void) const
{
    return fServerIpAddress;
}

inline void BcmDhcpLeaseSettings::siaddr(const BcmIpAddress &ipAddr)
{
    ServerIpAddress(ipAddr);
}

inline const BcmIpAddress &BcmDhcpLeaseSettings::siaddr(void) const
{
    return ServerIpAddress();
}

inline const BcmIpAddressList &BcmDhcpLeaseSettings::TftpServerIpAddressList(void) const
{
    return fTftpServerIpAddressList;
}

inline const BcmIpAddress &BcmDhcpLeaseSettings::RelayAgentIpAddress(void) const
{
    return fRelayAgentIpAddress;
}

inline void BcmDhcpLeaseSettings::giaddr(const BcmIpAddress &ipAddr)
{
    RelayAgentIpAddress(ipAddr);
}

inline const BcmIpAddress &BcmDhcpLeaseSettings::giaddr(void) const
{
    return RelayAgentIpAddress();
}


inline const BcmMacAddress &BcmDhcpLeaseSettings::ClientHardwareAddress(void) const
{
    return fClientHardwareAddress;
}

inline void BcmDhcpLeaseSettings::chaddr(const BcmMacAddress &hardwareAddress)
{
    ClientHardwareAddress(hardwareAddress);
}

inline const BcmMacAddress & BcmDhcpLeaseSettings::chaddr(void) const
{
    return ClientHardwareAddress();
}


inline const char *BcmDhcpLeaseSettings::ServerName(void) const
{
    // This is relatively safe.  Internally, we ensure that the pointer is
    // valid and that there is a NULL terminator.
    return (char *) (fServerName.AccessBuffer());
}

inline const BcmOctetBuffer &BcmDhcpLeaseSettings::ServerNameBuffer(void) const
{
    return fServerName;
}

inline void BcmDhcpLeaseSettings::sname(const char *pServerName)
{
    ServerName(pServerName);
}

inline const char *BcmDhcpLeaseSettings::sname(void) const
{
    return ServerName();
}


inline const char *BcmDhcpLeaseSettings::FileName(void) const
{
    // This is relatively safe.  Internally, we ensure that the pointer is
    // valid and that there is a NULL terminator.
    return (char *) (fBootfileName.AccessBuffer());
}

inline const BcmOctetBuffer &BcmDhcpLeaseSettings::FileNameBuffer(void) const
{
    // This one always returns the actual buffer for the file field in the bootp
    // header.
    return fFileName;
}

inline void BcmDhcpLeaseSettings::file(const char *pFileName)
{
    FileName(pFileName);
}

inline const char *BcmDhcpLeaseSettings::file(void) const
{
    return FileName();
}


inline const BcmOctetBuffer &BcmDhcpLeaseSettings::Options(void) const
{
    return fDhcpOptions;
}

inline void BcmDhcpLeaseSettings::IsYourIpThePrimaryIpAddress( bool isPrimary )
{
    fIsYourIpThePrimaryIpAddress = isPrimary;
}

inline bool BcmDhcpLeaseSettings::IsYourIpThePrimaryIpAddress( void ) const
{
    return fIsYourIpThePrimaryIpAddress;
}


inline unsigned long BcmDhcpLeaseSettings::CreationTime( void ) const
{
    return fCreationTime;
}

inline void BcmDhcpLeaseSettings::CreationTime( unsigned long Time )
{
    fCreationTime = Time;
}



// Special doxygen documentation tag for the DHCP Client/Server objects.
/** \defgroup DhcpClientServer Portable DHCP Client and Server

This section describes the architecture of the Portable DHCP Client and Server.

The portable DHCP Client was implemented after we ported the system from 
pSOS to VxWorks.  pSOS provided its own implementation, which had a number 
of bugs, RFC non-compliance issues, and inadequacies with respect to the 
needs of our applications.  Since our use of the client was considered 
"non-standard", we received almost no support and had to waste numerous 
days working around the issues ourselves.  Then came VxWorks, bringing its 
implementation of the DHCP Client with a completely different set of bugs 
and inadequacies.  Their response to our support requests was to throw the 
source code at us and say "have at it, boys." 

We were faced with several weeks of effort to hack the VxWorks DHCP 
Client, plus a similar amount of effort looming as we contemplated porting 
to Linux and eCos.  Doing a full-blown RFC implementation was a 
no-brainer, and made the port to Linux and eCos laughingly simple.  It 
also allowed us to support the very complex requirements of CableHome with 
little effort.  

The portable DHCP Server uses many of the same components that were 
implemented for the DHCP Client, yielding minimal code size increase.  

While it is possible to have both the DHCP Client and DHCP Server running 
within the same device, note that it is not normally desirable to let the 
DHCP Server allocate addresses for the DHCP Client.  The DHCP Client 
should only be allowed to allocate addresses for the device's IP stacks 
from an external DHCP Server; likewise, the DHCP Server should only be 
allowed to serve IP addresses to clients attached to the device.  These 
restrictions are usually achieved through clever use of forwarding rules.

In some cases, the DHCP Client and Server will collaborate in allocation 
and assignment of IP addresses - the DHCP Client can allocate some number 
of IP addresses from an external server, and the built-in DHCP Server can 
then hand these addresses out to clients attached to the device.  This 
DHCP proxy arrangement is common in residential gateway applications such 
as CableHome.

<H2>DHCP Client</H2>

The DHCP Client performs dynamic IP address allocation as defined in 
RFC2131 and RFC2132, with extensions defined by RFC3396 for handling long 
option values.  Additionally, it supports configuration of static 
addresses for environments where a DHCP server is not available or where 
use of a fixed, well-known IP address is required.  

The DHCP Client can manage multiple leases for each IP stack interface, and
can handle multiple IP stack interfaces.  We have successfully maintained
over one hundred leases across 5 IP stack interfaces for extended periods
of time.

\image html DhcpClient_HighLevel1.png "High Level DHCP Client Relationship, Part 1"

Each IP stack interface has its own instance of a BcmDhcpClientIf object.  This
diagram only shows 2, but the system can support up to 8.  The BcmDhcpClientIf 
object manages the set of leases associated with the interface, and implements 
the state machines and operations described in RFC2131.

The leases are created at the request of external agent, as are operations on
the leases.  The DHCP Client will automatically renew leases that have been
successfully acquired, but otherwise will lie dormant until commanded to do
something.

In order to minimize OS resource usage, there is a single DHCP Client thread 
within the system.  The thread runs a single timer which is used to drive 
various timeout events associated with the leases.  It doesn't do much of 
the actual DHCP Client work itself; instead, it calls the BcmDhcpClientIf
objects.

\image html DhcpClient_HighLevel2.png "High Level DHCP Client Relationship, Part 2"

This diagram shows the DHCP Client object relationship from the perspective of
a single IP stack interface (BcmBaseIpHalIf) and other networking components.

The BcmDhcpClientIf uses the IP stack interface object that it is registerd 
with to send DHCP and ARP packets to the network.  It also uses the object's
configuration API to add and remove leases from the underlying IP stack.  There 
are derived classes from BcmBaseIpHalIf to handle the peculiarities of each
operating system and IP stack.

As packets come in from the network, the DHCP Client Snoop inspects them, 
redirecting DHCP Client packets and ARP Reply packets to the DHCP Client thread.
The thread uses the contents of the packets (MAC address and/or Client Id) to
identify one of the BcmDhcpClientIf objects and dispatch the packet to it for
processing per RFC2131.

The BcmDhcpClientIf object sends timeout requests to the DHCP Client thread.
When the timeout occurs, the thread will call back to the BcmDhcpClientIf
object that requested the timeout, allowing the appropriate action to be 
performed based on the current lease state (send a packet, retransmit a
packet, start the renewal process, etc.).

<H3>How to use the DHCP Client</H3>

The information above is only of moderate interest to most people; the real
question is "How do I use it?"

First and foremost, you have to decide which IP stack interface you are 
going to control.  In many cases, this comes from a nonvol setting; 
regardless of how you make this decision, be aware that certain application 
components will try to take complete control over the IP stack interface, 
and that it is inappropriate for you to add leases or modify the DHCP control 
on top of what they are doing.  If your application component only needs 
to make use of an IP stack (via sockets), then you almost certainly don't 
need to use the DHCP Client.  Just wait for the stack to be configured and 
then use the allocated IP address.

Once you know the IP stack number, get access to the IP stack interface 
object via the BcmIpStackManager object.  This returns a pointer to the 
specified IP stack object, or NULL if that instance wasn't created.  

From there, you can access the DHCP Client If object, allocate or configure 
leases, and can command it to start DHCP or configure a static lease.  

The following code snippet shows a simple example for starting DHCP on an 
interface, stopping a lease that had previously been acquired, and 
configuring a static address.  Note that DHCP will operate asynchronously, 
and you will need to subscribe to lease events in order to be notified 
when it completes (or fails).  

\code
void StartDhcp(unsigned int stackNumber)
{
    BcmBaseIpHalIf *pIpStack = BcmIpStackManager::GetSingleton().Find(stackNumber);

    if (pIpStack != NULL)
    {
        BcmDhcpClientId clientId(ipHalIf.MacAddress());
        BcmDhcpClientIf *pDhcpClient;
        BcmDhcpClientLease *pDhcpLease;
    

#if (BFC_MULPI_SUPPORT || BFC_INCLUDE_EDVA_SUPPORT)
        // Set our clientId to the RFC4361 style value based on our MAC address
        clientId.Set4361Type(pIpStack->MacAddress());
#endif
        
        // Get a pointer to the DHCP Client If object, and get it to create a
        // new lease object based on the client id.
        pDhcpClient = pIpStack->DhcpClientIf();
        pDhcpLease = pDhcpClient->NewLease(clientId);

        // The default settings are good enough for simple DHCP operation; we
        // will request an IP address, subnet mask, and router address.  If
        // you need something more complex (such as adding specific option 
        // values or requesting specific options to be returned), this would be
        // a good place to configure the lease object.

        // Let's kick the process off.  This will run asynchronously.
        pDhcpClient->AcquireLease(clientId);
    }
}

void ShutDownDhcp(unsigned int stackNumber)
{
    BcmBaseIpHalIf *pIpStack = BcmIpStackManager::GetSingleton().Find(stackNumber);

    if (pIpStack != NULL)
    {
        BcmDhcpClientId clientId(ipHalIf.MacAddress());
        BcmDhcpClientIf *pDhcpClient;
    
#if (BFC_MULPI_SUPPORT || BFC_INCLUDE_EDVA_SUPPORT)
        // Set our clientId to the RFC4361 style value based on our MAC address
        clientId.Set4361Type(pIpStack->MacAddress());
#endif

        // Shut down the lease that we previously acquired.
        pDhcpClient->ReleaseLease(clientId);
    }
}

void ConfigureStaticAddress(unsigned int stackNumber,
                            const BcmIpAddress &ipAddress,
                            const BcmIpAddress &subnetMask,
                            const BcmIpAddress &router)
{
    BcmBaseIpHalIf *pIpStack = BcmIpStackManager::GetSingleton().Find(stackNumber);

    if (pIpStack != NULL)
    {
        BcmDhcpClientId clientId(ipHalIf.MacAddress());
        BcmDhcpClientIf *pDhcpClient;
        BcmDhcpClientLease *pDhcpLease;
    
#if (BFC_MULPI_SUPPORT || BFC_INCLUDE_EDVA_SUPPORT)
        // Set our clientId to the RFC4361 style value based on our MAC address
        clientId.Set4361Type(pIpStack->MacAddress());
#endif

        // Get a pointer to the DHCP Client If object, and get it to create a
        // new lease object based on the client id.
        pDhcpClient = pIpStack->DhcpClientIf();
        pDhcpLease = pDhcpClient->NewLease(clientId);

        // Configure the lease with the values that are specified.
        pDhcpLease->ServerLeaseSettings().YourIpAddress(ipAddress);
        pDhcpLease->ServerLeaseSettings().AddReplaceOption(kDhcpSubnetMask, 
                                                           subnetMask);
        pDhcpLease->ServerLeaseSettings().AddReplaceOption(kDhcpRouter,
                                                           router);

        // Add the lease to the IP stack.
        pDhcpClient->ConfigureStaticLease(clientId);
    }
}
\endcode

<H2>DHCP Server</H2>

The DHCP Server is the devil incarnate.  Even the worst virus won't cause this
much disruption to your network.  One minute, some poor unsuspecting slob plugs
this puppy in to the network; next thing you know the corporate IT hit squad is
doing an office-to-office manhunt to locate the rogue server.

There is no console command table or nonvol to control or disable it, and there
is very little indication that it is handing out leases; once it starts up, it 
will keep on quietly polluting the network until somebody unplugs it.  What's 
worse - the DHCP Server is usually associated with a static IP address 
(192.168.100.1 or 192.168.0.1) and a fixed MAC address, both of which are used 
identically on every single device, so you can't even use a sniffer to identify 
which one is the culprit.  You wind up having to troll around the office to 
find it and shut it off.

Even with the potential for evil such as it is, when used as intended it is 
very beneficial.

UNFINISHED - I had thought that it would be easy to document the DHCP Server 
architecture, but much of the class documentation is either incomplete or 
incorrect (copy/paste from the DHCP Client), and I don't have time to reverse
engineer the code to figure it out...

*/

#endif


