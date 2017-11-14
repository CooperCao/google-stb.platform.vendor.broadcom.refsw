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
//  Filename:       ClassifierSettings.h
//  Author:         Cliff Danielson
//  Creation Date:  {unknown}
//
//****************************************************************************

#ifndef CLASSIFIERSETTINGS_H
#define CLASSIFIERSETTINGS_H

#include "typedefs.h"

#include "TLVCodes.h"
#include "Cmim.h"
#include "MacAddress.h"
#include "IpAddress.h"
#include "ErrorSetList.h"

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iomanip>
#else
#include <iomanip.h>
#endif

class BcmOctetBuffer;
class BcmMessageLogSettings;
class BcmQosErrorEncodingList;

/**# :[Description = "DOCSIS 1.1 IP classifier settings."] */
class BcmIpClassifierSettings
{
public:
    // begin functions required to be param for STL containers...
    //

    // Default constructor.
    //
	BcmIpClassifierSettings();

    // Copy constructor.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
	BcmIpClassifierSettings( const BcmIpClassifierSettings& rhs );

    // Destructor.
    //
    ~BcmIpClassifierSettings() {}

    // Assignment operator.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
    // Returns:
    //      reference to this object
    //
    BcmIpClassifierSettings& operator=( const BcmIpClassifierSettings& rhs );

    // Equality operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if objects are equal, else false.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.
    //
	bool operator==( const BcmIpClassifierSettings& rhs ) const;
    // end functions required to be param for STL containers...

    // Inequality operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if objects are not equal, else false.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.
    //
	bool operator!=( const BcmIpClassifierSettings& rhs ) const;
    // end functions required to be param for STL containers...

    // Reset to default values.
    //
	void Reset();

    // Print formatted object contents to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& Print( ostream& ostrm ) const;

    // This method causes the object to write itself out to the octet buffer in
    // TLV format.  This function writes the type 25 specific sub-type TLV
    // attributes, but not the parent type 25 attribute.  The caller
    // of this function must handle the parent TLV.
    //
    // Parameters:
    //      octetBuffer - the buffer into which the TLVs should be written.
    //
    // Returns:
    //      true if the TLVs were written correctly.
    //      false if there was a problem (not enough room in the buffer, etc.).
    //
	bool WriteTo( BcmOctetBuffer &octetBuffer, BcmMessageLogSettings* log_sets = NULL ) const;

	// Load object with the decoded TLV attributes contained in the specified
    // input buffer.
    //
    // Parameters:
    //      octetBuffer - input buffer.
    //
    //      offset - offset into input buffer at which to begin parsing.
    //
    //      err_set_list - reference to ErrorSetList object of "parent"
    //          BcmClassifierSettings object.  the object being parsed 
    //          may encounter an error, create a BcmErrorSet object, 
    //          and add it to this list.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which are
    //          used to control debug output.
    //
    //      strict_checking - if true, perform strict error checking during
    //          and after parsing, else do not validate setting values
    //          after parsing.  default value is true.  value of false
    //          is useful when encoding/decoding DOCSIS messages with
    //          known errors for test scenarios.
    //
    // Returns:
    //      "Confirmation code" result, as described in DOCSIS RFI v1.1-IO5
    //      spec App C.4.  A result == 0 is success.  Non-zero result is an
    //      error.
    //
    // Note:
    // It is assumed that the parent T and L codes have already been read
    // from the input buffer.  The starting offset for subsequent processing
    // will be the T code of the first sub-type.
    //
	byte ParseFrom( const BcmOctetBuffer &octetBuffer, unsigned int &offset, 
        ErrorSetList& err_set_list, BcmMessageLogSettings* log_sets = NULL, 
        bool strict_checking = true );

    // Helper method to check all of the "...Specified" members, telling whether
    // any of them is specified.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if at least one "...Specified" member is true.
    //      false if all of the "...Specified" members are false.
    //
    bool AnythingSpecified(void) const;

    // accessor for the ToS range and mask.
    byte TosRangeAndMask(unsigned int offset) const { return fTosRangeAndMask[offset]; }

    // accessor for IP protocol type.  Type [22/23/60].9.2  no default value.
    uint16 ProtocolType() const { return fProtocolType; };
    void ProtocolType(uint16 type) { fProtocolType = type;
                                     fProtocolTypeSpecified = true;
                                     fIpv4NonTcpUdpPortSpecified = true;
                                   };
    void ClearProtocolType() { fProtocolType = 0;
                                     fProtocolTypeSpecified = false;
                                   };

    // accessor for source IP address.  Type [22/23/60].9.3  no default value.
    const BcmIpAddress &SourceIpAddr() const { return fSourceIpAddr; };
    void SourceIpAddr(BcmIpAddress addr) { fSourceIpAddr = addr;
                                           fSourceIpAddrSpecified = true;
                                           fIpv4NonTcpUdpPortSpecified = true;
                                         };
    void ClearSourceIpAddr() { fSourceIpAddr = kAllZerosIp;
                                           fSourceIpAddrSpecified = false;
                                         };

    // accessor for source IP mask.  Type [22/23/60].9.4  default = 255.255.255.255
    const BcmIpAddress &SourceIpMask() const { return fSourceIpMask; };
    void SourceIpMask(BcmIpAddress mask) { fSourceIpMask = mask;
                                            fSourceIpMaskSpecified = true;
                                            fIpv4NonTcpUdpPortSpecified = true;
                                          };
    void ClearSourceIpMask() { fSourceIpMask = kBroadcastIp;
                                            fSourceIpMaskSpecified = false;
                                          };

    // accessor for dest IP address.  Type [22/23/60].9.5  no default value.
    const BcmIpAddress &DestIpAddr() const { return fDestIpAddr; };
    void DestIpAddr(BcmIpAddress addr) { fDestIpAddr = addr;
                                          fDestIpAddrSpecified = true;
                                          fIpv4NonTcpUdpPortSpecified = true;
                                        };
    void ClearDestIpAddr() { fDestIpAddr = kAllZerosIp;
                                          fDestIpAddrSpecified = false;
                                        };

    // accessor for dest IP mask.  Type [22/23/60].9.6  default = 255.255.255.255
    const BcmIpAddress &DestIpMask() const { return fDestIpMask; };
    void DestIpMask(BcmIpAddress addr) { fDestIpMask = addr;
                                         fDestIpMaskSpecified = true;
                                         fIpv4NonTcpUdpPortSpecified = true;
                                       };
    void ClearDestIpMask() { fDestIpMask = kBroadcastIp;
                                         fDestIpMaskSpecified = false;
                                       };

    // accessor for source TCP/UDP port start.  Type [22/23/60].9.7  default value = 0
    uint16 SourcePortStart() const { return fSourcePortStart; };
    void SourcePortStart(uint16 port) { fSourcePortStart = port;
                                        fSourcePortStartSpecified = true;
                                        fTcpUdpPortSpecified = true;
                                      };
    void ClearSourcePortStart() { fSourcePortStart = 0;
                                        fSourcePortStartSpecified = false;
                                      };

    // accessor for source TCP/UDP port end.  Type [22/23/60].9.8  default value = 65535
    uint16 SourcePortEnd() const { return fSourcePortEnd; };
    void SourcePortEnd(uint16 port) { fSourcePortEnd = port;
                                      fSourcePortEndSpecified = true;
                                      fTcpUdpPortSpecified = true;
                                    };
    void ClearSourcePortEnd() { fSourcePortEnd = 0;
                                      fSourcePortEndSpecified = false;
                                    };

    // accessor for dest TCP/UDP port start.  Type [22/23/60].9.9  default value = 0
    uint16 DestPortStart() const { return fDestPortStart; };
    void DestPortStart(uint16 port) { fDestPortStart = port;
                                      fDestPortStartSpecified = true;
                                      fTcpUdpPortSpecified = true;
                                    };
    void ClearDestPortStart() { fDestPortStart = 0;
                                      fDestPortStartSpecified = false;
                                    };

    // accessor for dest TCP/UDP port end.  Type [22/23/60].9.10  default value = 65535
    uint16 DestPortEnd() const { return fDestPortEnd; };
    void DestPortEnd(uint16 port) { fDestPortEnd = port;
                                    fDestPortEndSpecified = true;
                                    fTcpUdpPortSpecified = true;
                                  };
    void ClearDestPortEnd() { fDestPortEnd = 0;
                                    fDestPortEndSpecified = false;
                                  };

    // accessor for the boolean that indicates whether or not any of the IPv4
    // values (other than TCP/UDP source/dest port) were specified.
    inline bool Ipv4NonTcpUdpPortSpecified(void) const { return fIpv4NonTcpUdpPortSpecified; }

    // Accessors for the booleans that indicate whether or not the non-TCP/UDP
    // port values are specified.
    inline bool TosRangeAndMaskSpecified(void) const { return fTosRangeAndMaskSpecified; }
    inline bool ProtocolTypeSpecified(void) const { return fProtocolTypeSpecified; }
    inline bool SourceIpAddrSpecified(void) const { return fSourceIpAddrSpecified; }
    inline bool SourceIpMaskSpecified(void) const { return fSourceIpMaskSpecified; }
    inline bool DestIpAddrSpecified(void) const { return fDestIpAddrSpecified; }
    inline bool DestIpMaskSpecified(void) const { return fDestIpMaskSpecified; }

    // accessor for the boolean that indicates whether or not any of the
    // TCP/UDP source/dest port values were specified.
    inline bool TcpUdpPortSpecified(void) const { return fTcpUdpPortSpecified; }

    // Accessors for the booleans that indicate whether or not the source/dest
    // TCP/UDP ports are specified.
    inline bool SourcePortStartSpecified(void) const { return fSourcePortStartSpecified; }
    inline bool SourcePortEndSpecified(void) const { return fSourcePortEndSpecified; }
    inline bool DestPortStartSpecified(void) const { return fDestPortStartSpecified; }
    inline bool DestPortEndSpecified(void) const { return fDestPortEndSpecified; }
    inline bool SourcePortStartOrEndSpecified(void) const { return (fSourcePortStartSpecified || fSourcePortEndSpecified); }
    inline bool DestPortStartOrEndSpecified(void) const { return (fDestPortStartSpecified || fDestPortEndSpecified); }


private:

    // Make all of these private so that we can be sure fIpv4NonTcpUdpPortSpecified
    // is updated properly.

	// IP Type Of Service range and mask.  Type [22/23/60].9.1  no default value.
    // data modeling is wacky.  the V consists of (3) eight bit values.
    byte fTosRangeAndMask[ 3 ];
    bool fTosRangeAndMaskSpecified;

	// IP protocol type.  Type [22/23/60].9.2  no default value.
    uint16 fProtocolType;
    bool fProtocolTypeSpecified;

	// source IP address.  Type [22/23/60].9.3  no default value.
    BcmIpAddress fSourceIpAddr;
    bool fSourceIpAddrSpecified;

	// source IP mask.  Type [22/23/60].9.4  default = 255.255.255.255
    BcmIpAddress fSourceIpMask;
    bool fSourceIpMaskSpecified;

	// dest IP address.  Type [22/23/60].9.5  no default value.
    BcmIpAddress fDestIpAddr;
    bool fDestIpAddrSpecified;

	// dest IP mask.  Type [22/23/60].9.6  default = 255.255.255.255
    BcmIpAddress fDestIpMask;
    bool fDestIpMaskSpecified;

    // Indicates whether any of the IPv4 values (other than TCP/UDP ports) were
    // specified.
    bool fIpv4NonTcpUdpPortSpecified;

private:

    // Make all of these private so that we can be sure fTcpUdpPortSpecified
    // is updated properly.

    // This allows access to private members via helper methods in the containing
    // class.
    friend class BcmClassifierSettings;

    // source TCP/UDP port start.  Type [22/23/60].9.7  default value = 0
    uint16 fSourcePortStart;
    bool fSourcePortStartSpecified;

    // source TCP/UDP port end.  Type [22/23/60].9.8  default value = 65535
    uint16 fSourcePortEnd;
    bool fSourcePortEndSpecified;

    // dest TCP/UDP port start.  Type [22/23/60].9.9  default value = 0
    uint16 fDestPortStart;
    bool fDestPortStartSpecified;

    // dest TCP/UDP port end.  Type [22/23/60].9.10  default value = 65535
    uint16 fDestPortEnd;
    bool fDestPortEndSpecified;

    // Indicates whether any of the TCP/UDP port values were specified.
    bool fTcpUdpPortSpecified;

};

inline ostream& operator<<( ostream& ostrm, const BcmIpClassifierSettings& rhs )
{
    return rhs.Print( ostrm );
}


#if (BFC_INCLUDE_IPV6_SUPPORT)

/**# :[Description = "DOCSIS 3.0 IPv6 classifier settings."] */
// Added for PR13079
class BcmIpv6ClassifierSettings
{
public:
    // begin functions required to be param for STL containers...
    //

    // Default constructor.
    //
	BcmIpv6ClassifierSettings();

    // Copy constructor.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
	BcmIpv6ClassifierSettings( const BcmIpv6ClassifierSettings& rhs );

    // Destructor.
    //
    ~BcmIpv6ClassifierSettings() {}

    // Assignment operator.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
    // Returns:
    //      reference to this object
    //
    BcmIpv6ClassifierSettings& operator=( const BcmIpv6ClassifierSettings& rhs );

    // Equality operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if objects are equal, else false.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.
    //
	bool operator==( const BcmIpv6ClassifierSettings& rhs ) const;
    // end functions required to be param for STL containers...

    // Inequality operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if objects are not equal, else false.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.
    //
	bool operator!=( const BcmIpv6ClassifierSettings& rhs ) const;
    // end functions required to be param for STL containers...

    // Reset to default values.
    //
	void Reset();

    // Print formatted object contents to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& Print( ostream& ostrm ) const;

    // This method causes the object to write itself out to the octet buffer in
    // TLV format.  This function writes the type 25 specific sub-type TLV
    // attributes, but not the parent type 25 attribute.  The caller
    // of this function must handle the parent TLV.
    //
    // Parameters:
    //      octetBuffer - the buffer into which the TLVs should be written.
    //
    // Returns:
    //      true if the TLVs were written correctly.
    //      false if there was a problem (not enough room in the buffer, etc.).
    //
	bool WriteTo( BcmOctetBuffer &octetBuffer, BcmMessageLogSettings* log_sets = NULL ) const;

	// Load object with the decoded TLV attributes contained in the specified
    // input buffer.
    //
    // Parameters:
    //      octetBuffer - input buffer.
    //
    //      offset - offset into input buffer at which to begin parsing.
    //
    //      err_set_list - reference to ErrorSetList object of "parent"
    //          BcmClassifierSettings object.  the object being parsed 
    //          may encounter an error, create a BcmErrorSet object, 
    //          and add it to this list.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which are
    //          used to control debug output.
    //
    //      strict_checking - if true, perform strict error checking during
    //          and after parsing, else do not validate setting values
    //          after parsing.  default value is true.  value of false
    //          is useful when encoding/decoding DOCSIS messages with
    //          known errors for test scenarios.
    //
    // Returns:
    //      "Confirmation code" result, as described in DOCSIS RFI v1.1-IO5
    //      spec App C.4.  A result == 0 is success.  Non-zero result is an
    //      error.
    //
    // Note:
    // It is assumed that the parent T and L codes have already been read
    // from the input buffer.  The starting offset for subsequent processing
    // will be the T code of the first sub-type.
    //
	byte ParseFrom( const BcmOctetBuffer &octetBuffer, unsigned int &offset, 
        ErrorSetList& err_set_list, BcmMessageLogSettings* log_sets = NULL, 
        bool strict_checking = true );

    // Helper method to check all of the "...Specified" members, telling whether
    // any of them is specified.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if at least one "...Specified" member is true.
    //      false if all of the "...Specified" members are false.
    //
    bool AnythingSpecified(void) const;

	// IPv6 Traffic Class range and mask.  Type [22/23/60].12.1  no default value.
    // data modeling is wacky.  the V consists of (3) eight bit values.
    byte fTrafficClassRangeAndMask[ 3 ];
    bool fTrafficClassRangeAndMaskSpecified;

	// IPv6 Flow Label.  Type [22/23/60].12.2  no default value.  Only the lower
    // 20 bits are relevant; the upper 12 bits are ignored.
    uint32 fFlowLabel;
    bool fFlowLabelSpecified;

    // IPv6 Next Header Type.  Type [22/23/60].12.3  no default value.
    uint16 fNextHeaderType;
    bool fNextHeaderTypeSpecified;

	// source IPv6 address.  Type [22/23/60].12.4  no default value.
    BcmIpAddress fSourceIpv6Addr;
    bool fSourceIpv6AddrSpecified;

	// source IPv6 Prefix Length.  Type [22/23/60].12.5  default = 128
    uint8 fSourceIpv6PrefixLength;
    bool fSourceIpv6PrefixLengthSpecified;

	// dest IPv6 address.  Type [22/23/60].12.6  no default value.
    BcmIpAddress fDestIpv6Addr;
    bool fDestIpv6AddrSpecified;

	// dest IPv6 Prefix Length.  Type [22/23/60].12.7  default = 128
    uint8 fDestIpv6PrefixLength;
    bool fDestIpv6PrefixLengthSpecified;

};

inline ostream& operator<<( ostream& ostrm, const BcmIpv6ClassifierSettings& rhs )
{
    return rhs.Print( ostrm );
}

#endif // BFC_INCLUDE_IPV6_SUPPORT


/**# :[Description = "DOCSIS 1.1 Ethernet LLC classifier settings."] */
class BcmEthernetClassifierSettings
{
public:
    // begin functions required to be param for STL containers...
    //

    // Default constructor.
    //
	BcmEthernetClassifierSettings();

    // Copy constructor.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
	BcmEthernetClassifierSettings( const BcmEthernetClassifierSettings& rhs );

    // Destructor.
    //
    ~BcmEthernetClassifierSettings() {}

    // Assignment operator.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
    // Returns:
    //      reference to this object
    //
    BcmEthernetClassifierSettings& operator=( const BcmEthernetClassifierSettings& rhs );

    // Equality operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if objects are equal, else false.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.  
    //
	bool operator==( const BcmEthernetClassifierSettings& rhs ) const;
    // end functions required to be param for STL containers...

    // Inequality operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if objects not are equal, else false.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.  
    //
	bool operator!=( const BcmEthernetClassifierSettings& rhs ) const;
    // end functions required to be param for STL containers...
    // Reset to default values.
    //
	void Reset();


    // Print formatted object contents to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& Print( ostream& ostrm ) const;

    // This method causes the object to write itself out to the octet buffer in
    // TLV format.  This function writes the type 25 specific sub-type TLV
    // attributes, but not the parent type 25 attribute.  The caller
    // of this function must handle the parent TLV.
    //
    // Parameters:
    //      octetBuffer - the buffer into which the TLVs should be written.
    //
    // Returns:
    //      true if the TLVs were written correctly.
    //      false if there was a problem (not enough room in the buffer, etc.).
    //
	bool WriteTo( BcmOctetBuffer &octetBuffer, BcmMessageLogSettings* log_sets = NULL ) const;

	// Load object with the decoded TLV attributes contained in the specified
    // input buffer.
    //
    // Parameters:
    //      octetBuffer - input buffer.
    //
    //      offset - offset into input buffer at which to begin parsing.
    //
    //      err_set_list - reference to ErrorSetList object of "parent"
    //          BcmClassifierSettings object.  the object being parsed 
    //          may encounter an error, create a BcmErrorSet object, 
    //          and add it to this list.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which are
    //          used to control debug output.
    //
    //      strict_checking - if true, perform strict error checking during
    //          and after parsing, else do not validate setting values
    //          after parsing.  default value is true.  value of false
    //          is useful when encoding/decoding DOCSIS messages with
    //          known errors for test scenarios.
    //
    // Returns:
    //      "Confirmation code" result, as described in DOCSIS RFI v1.1-IO5
    //      spec App C.4.  A result == 0 is success.  Non-zero result is an
    //      error.
    //
    // Note:
    // It is assumed that the parent T and L codes have already been read
    // from the input buffer.  The starting offset for subsequent processing
    // will be the T code of the first sub-type.
    //
	byte ParseFrom( const BcmOctetBuffer &octetBuffer, unsigned int &offset, 
        ErrorSetList& err_set_list, BcmMessageLogSettings* log_sets = NULL, 
        bool strict_checking = true );
    
    // Helper method to check all of the "...Specified" members, telling whether
    // any of them is specified.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if at least one "...Specified" member is true.
    //      false if all of the "...Specified" members are false.
    //
    bool AnythingSpecified(void) const;


	// dest MAC address and mask.  Type [22/23/60].10.1  no default value.
    // data modeling is wacky.  the V consists of (2) MAC addresses
    // but is specified as a single sub-type.  We have split this out to 2
    // BcmMacAddress instances for public consumption, but convert to/from the
    // 12-byte representation for write/parse operations.
    BcmMacAddress fDestinationMacAddr;
    BcmMacAddress fDestinationMacAddrMask;
    bool fDestMacAddrAndMaskSpecified;

	// source MAC address.  Type [22/23/60].10.2  no default value.
    BcmMacAddress fSourceMacAddr;
    bool fSourceMacAddrSpecified;

	// Ethertype/IEEE 802.2 SAP  Type [22/23/60].10.3  no default value.
    // data modeling is extra wacky!  the V consists of (3) eight
    // bit values.
    byte fEtherType[ 3 ];
    bool fEtherTypeSpecified;
};

inline ostream& operator<<( ostream& ostrm, const BcmEthernetClassifierSettings& rhs )
{
    return rhs.Print( ostrm );
}

/**# :[Description = "DOCSIS 1.1 IEEE 802.1P/Q classifier settings."] */
class Bcm8021PqClassifierSettings
{
public:
    // begin functions required to be param for STL containers...
    //

    // Default constructor.
    //
	Bcm8021PqClassifierSettings()
    {
        fUserPrioritySpecified = fVLanIdSpecified = false;
        fUserPriority[ 0 ] = fUserPriority[ 1 ] = fVLanId[ 0 ] 
            = fVLanId[ 1 ] = 0;
    }

    // Copy constructor.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
	Bcm8021PqClassifierSettings( const Bcm8021PqClassifierSettings& rhs )
    {
        fUserPrioritySpecified = rhs.fUserPrioritySpecified;
        fUserPriority[ 0 ] = rhs.fUserPriority[ 0 ];
        fUserPriority[ 1 ] = rhs.fUserPriority[ 1 ];

        fVLanIdSpecified = rhs.fVLanIdSpecified;
        fVLanId[ 0 ] = rhs.fVLanId[ 0 ];
        fVLanId[ 1 ] = rhs.fVLanId[ 1 ];
    }

    // Destructor.
    //
    ~Bcm8021PqClassifierSettings() {}

    // Assignment operator.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
    // Returns:
    //      reference to this object
    //
    Bcm8021PqClassifierSettings& operator=( const Bcm8021PqClassifierSettings& rhs )
    {
        // special case: assignment to self can skip the work.
        if (this != &rhs)
        {
            fUserPrioritySpecified = rhs.fUserPrioritySpecified;
            fUserPriority[ 0 ] = rhs.fUserPriority[ 0 ];
            fUserPriority[ 1 ] = rhs.fUserPriority[ 1 ];
    
            fVLanIdSpecified = rhs.fVLanIdSpecified;
            fVLanId[ 0 ] = rhs.fVLanId[ 0 ];
            fVLanId[ 1 ] = rhs.fVLanId[ 1 ];
        }

        return *this;
    }

    // Equality operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if objects are equal, else false.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.  
    //
	bool operator==( const Bcm8021PqClassifierSettings& rhs ) const;
    // end functions required to be param for STL containers...

    // Inequality operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if objects are not equal, else false.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.  
    //
	bool operator!=( const Bcm8021PqClassifierSettings& rhs ) const;
    // end functions required to be param for STL containers...

    // Reset to default values.
    //
	void Reset()
    {
        fUserPrioritySpecified = fVLanIdSpecified = false;
        fUserPriority[ 0 ] = fUserPriority[ 1 ] = fVLanId[ 0 ] = fVLanId[ 1 ] = 0;
    }


    // Print formatted object contents to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& Print( ostream& ostrm ) const;

    // This method causes the object to write itself out to the octet buffer in
    // TLV format.  This function writes the type 25 specific sub-type TLV
    // attributes, but not the parent type 25 attribute.  The caller
    // of this function must handle the parent TLV.
    //
    // Parameters:
    //      octetBuffer - the buffer into which the TLVs should be written.
    //
    // Returns:
    //      true if the TLVs were written correctly.
    //      false if there was a problem (not enough room in the buffer, etc.).
    //
	bool WriteTo( BcmOctetBuffer &octetBuffer, BcmMessageLogSettings* log_sets = NULL ) const;

	// Load object with the decoded TLV attributes contained in the specified
    // input buffer.
    //
    // Parameters:
    //      octetBuffer - input buffer.
    //
    //      offset - offset into input buffer at which to begin parsing.
    //
    //      err_set_list - reference to ErrorSetList object of "parent"
    //          BcmClassifierSettings object.  the object being parsed 
    //          may encounter an error, create a BcmErrorSet object, 
    //          and add it to this list.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which are
    //          used to control debug output.
    //
    //      strict_checking - if true, perform strict error checking during
    //          and after parsing, else do not validate setting values
    //          after parsing.  default value is true.  value of false
    //          is useful when encoding/decoding DOCSIS messages with
    //          known errors for test scenarios.
    //
    // Returns:
    //      "Confirmation code" result, as described in DOCSIS RFI v1.1-IO5
    //      spec App C.4.  A result == 0 is success.  Non-zero result is an
    //      error.
    //
    // Note:
    // It is assumed that the parent T and L codes have already been read
    // from the input buffer.  The starting offset for subsequent processing
    // will be the T code of the first sub-type.
    //
	byte ParseFrom( const BcmOctetBuffer &octetBuffer, unsigned int &offset, 
        ErrorSetList& err_set_list, BcmMessageLogSettings* log_sets = NULL, 
        bool strict_checking = true );

    // Helper method to check all of the "...Specified" members, telling whether
    // any of them is specified.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if at least one "...Specified" member is true.
    //      false if all of the "...Specified" members are false.
    //
    bool AnythingSpecified(void) const;

	
    // 802.1P User priority.  Type [22/23/60].11.1
    // no default.  valid range 0 - 7.  data modeling is wacky.  the V
    // consists of (2) eight bit values.
    byte fUserPriority[ 2 ];
    bool fUserPrioritySpecified;

	// 802.1Q VLan id.  Type [22/23/60].11.2
    // no default.  data modeling is wacky.  the V consists of (2) eight
    // bit values.
    byte fVLanId[ 2 ];
    bool fVLanIdSpecified;
};

inline ostream& operator<<( ostream& ostrm, const Bcm8021PqClassifierSettings& rhs )
{
    return rhs.Print( ostrm );
}



/**# :[Description = "DOCSIS 1.1 Packet Classifier Settings."]
[Level = Analysis] */
class BcmClassifierSettings
{
public:

    // begin functions required to be param for STL containers...
    //

    // Default constructor.
    //
	BcmClassifierSettings( byte starting_type_code = 0 );

    // Copy constructor.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
	BcmClassifierSettings( const BcmClassifierSettings& rhs );

    // Destructor.
    //
    ~BcmClassifierSettings() {}

    // Assignment operator.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
    // Returns:
    //      reference to this object
    //
    BcmClassifierSettings& operator=( const BcmClassifierSettings& rhs );

    // Less than operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if this object is less than the comparison object.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.  It is used for ordering lists.
    //
	bool operator<( const BcmClassifierSettings& rhs ) const;

    // Equality operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if objects are equal, else false.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.  
    //
	bool operator==( const BcmClassifierSettings& rhs ) const;
    // end functions required to be param for STL containers...

    // Print formatted object contents to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
    ostream& Print( ostream& ostrm ) const;

    // This method causes the object to write itself out to the octet buffer in
    // TLV format.  This function writes the type 22/23 sub-type TLV
    // attributes, but not the parent type 22/23 attribute.  The caller
    // of this function must handle the parent TLV.
    //
    // Parameters:
    //      octetBuffer - the buffer into which the TLVs should be written.
    //
    // Returns:
    //      true if the TLVs were written correctly.
    //      false if there was a problem (not enough room in the buffer, etc.).
    //
    bool WriteTo( BcmOctetBuffer &octetBuffer, BcmMessageLogSettings* log_sets = NULL ) const;

	// Load object with the decoded TLV attributes contained in the specified
    // input buffer.
    //
    // Parameters:
    //      octetBuffer - input buffer.
    //
    //      offset - offset into input buffer at which to begin parsing.
    //
    //      errorEncodingList - BcmQosErrorEncodingList objects.  if ParseFrom()
    //          encounters errors, resulting error encoding objects are added
    //          to this object.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    //      strict_checking - if true, perform strict error checking during
    //          and after parsing, else do not validate setting values
    //          after parsing.  default value is true.  value of false
    //          is useful when encoding/decoding DOCSIS messages with
    //          known errors for test scenarios.
    //
    // Returns:
    //      "Confirmation code" result, as described in DOCSIS RFI v1.1-IO5
    //      spec App C.4.  A result == 0 is success.  Non-zero result is an
    //      error.
    //
    // Note:
    // It is assumed that the parent T and L codes have already been read
    // from the input buffer.  The starting offset for subsequent processing
    // will be the T code of the first sub-type.
    //
	byte ParseFrom( const BcmOctetBuffer &octetBuffer, unsigned int &offset,
        BcmQosErrorEncodingList& errorEncodingList, 
        BcmMessageLogSettings* log_sets = NULL, bool strict_checking = true );
    
    // AddErrorSet() - Helper function that creates a new error param set object
    //      with the specified TLV type code, conf code, and optional string,
    //      and adds it to the specified list of QoS error encoding objects.  
    //      It also logs meaningful information to the console.
    //
    // Parameters:
    //      pMsgLogSettings - used for logging to the console.
    //      errorEncodingList - destination list of error encoding objects.
    //      errSubTypeCode - the TLV subtype code that is being errored.
    //      confCode - error code (a.k.a. confirmation code) value from RFI 
    //          spec App. C.4
    //      pErrorString - optional error string giving more info about the error.
    //
    // Returns:
    //      The conf code that was specified.
    //
    byte AddErrorSet( BcmMessageLogSettings *pMsgLogSettings,
        BcmQosErrorEncodingList &errorEncodingList, byte errSubTypeCode, byte confCode, 
        const char *pErrorString = NULL );

public:
    // misc accessors
    //
    int UsageContext() const { return fUsageContext; }
    void UsageContext( int new_val ) { fUsageContext = new_val; }
    //
    // returns starting type code (a.k.a kUsClassifierSettings or
    // kDsClassifierSettings)
    byte StartingTypeCode() const { return fStartingTypeCode; }
    //
    byte CmClassifierRef() const { return fCmClassifierRef; }
    void CmClassifierRef( byte new_val ) { fCmClassifierRef = new_val; }
    //
    uint16 CmtsClassifierId() const { return fCmtsClassifierId; }
    void CmtsClassifierId( uint16 new_val ) { fCmtsClassifierId = new_val; }
    //
    uint32 Sfid() const { return fSfid; }
    void Sfid( uint32 new_val ) { fSfid = new_val; }
    //
    uint16 CmFlowRef() const { return fCmFlowRef; }
    void CmFlowRef( uint16 new_val ) { fCmFlowRef = new_val; }
    //
    //
    // Returns true if classifier direction is upstream.
    // PR12948 - updated to handle Drop Classifiers (which are always upstream).
    bool IsUpstreamClassifier() const 
    {
        if ((fStartingTypeCode == kUsClassifierSettings) ||
            (fStartingTypeCode == kUsDropClassifierSettings))
        {
            return true;
        }

        return false;
    }

    //
    bool IsDownstreamClassifier() const 
        { return( fStartingTypeCode == kDsClassifierSettings ); }

    bool DscActionSpecified() const { return fDscActionSpecified; }
    byte DscAction() const { return fDscAction; }
    void DscAction( byte new_val ) 
    { 
        fDscAction = new_val; 
        fDscActionSpecified = true;
    }

    // accessor for classifier activation state [22/23/60].6 - default value 1
    byte ActivationState() const { return fIsActive; }
    void ActivationState( byte new_val ) 
    { 
        fIsActive = new_val; 
        fIsActiveSpecified = true;
    }

    // accessor for rule priority [22/23/60].5 - default value 64
    byte RulePriority() const { return fRulePriority; }
    void RulePriority( byte new_val ) 
    { 
        fRulePriority = new_val; 
        fRulePrioritySpecified = true;
    }

    // accessors for IP Classifier Settings
    //
    // accessor for the ToS range and mask.
    byte TosRangeAndMask(unsigned int offset) const { return fIpClassifierSets.fTosRangeAndMask[offset]; }

    // accessor for IP protocol type.  Type [22/23/60].9.2  no default value.
    uint16 ProtocolType() const { return fIpClassifierSets.fProtocolType; };
    void ProtocolType(uint16 type) { fIpClassifierSets.fProtocolType = type;
                                     fIpClassifierSets.fProtocolTypeSpecified = true;
                                     fIpClassifierSets.fIpv4NonTcpUdpPortSpecified = true;
                                     fIpClassifierSetsSpecified = true;
                                   };

    // accessor for source IP address.  Type [22/23/60].9.3  no default value.
    const BcmIpAddress &SourceIpAddr() const { return fIpClassifierSets.fSourceIpAddr; };
    void SourceIpAddr(BcmIpAddress addr) { fIpClassifierSets.fSourceIpAddr = addr;
                                           fIpClassifierSets.fSourceIpAddrSpecified = true;
                                           fIpClassifierSets.fIpv4NonTcpUdpPortSpecified = true;
                                           fIpClassifierSetsSpecified = true;
                                         };

    // accessor for source IP mask.  Type [22/23/60].9.4  default = 255.255.255.255
    const BcmIpAddress &SourceIpMask() const { return fIpClassifierSets.fSourceIpMask; };
    void SourceIpMask(BcmIpAddress mask) { fIpClassifierSets.fSourceIpMask = mask;
                                            fIpClassifierSets.fSourceIpMaskSpecified = true;
                                            fIpClassifierSets.fIpv4NonTcpUdpPortSpecified = true;
                                            fIpClassifierSetsSpecified = true;
                                          };

    // accessor for dest IP address.  Type [22/23/60].9.5  no default value.
    const BcmIpAddress &DestIpAddr() const { return fIpClassifierSets.fDestIpAddr; };
    void DestIpAddr(BcmIpAddress addr) { fIpClassifierSets.fDestIpAddr = addr;
                                          fIpClassifierSets.fDestIpAddrSpecified = true;
                                          fIpClassifierSets.fIpv4NonTcpUdpPortSpecified = true;
                                          fIpClassifierSetsSpecified = true;
                                        };

    // accessor for dest IP mask.  Type [22/23/60].9.6  default = 255.255.255.255
    const BcmIpAddress &DestIpMask() const { return fIpClassifierSets.fDestIpMask; };
    void DestIpMask(BcmIpAddress addr) { fIpClassifierSets.fDestIpMask = addr;
                                         fIpClassifierSets.fDestIpMaskSpecified = true;
                                         fIpClassifierSets.fIpv4NonTcpUdpPortSpecified = true;
                                         fIpClassifierSetsSpecified = true;
                                       };

    // accessor for source TCP/UDP port start.  Type [22/23/60].9.7  default value = 0
    uint16 SourcePortStart() const { return fIpClassifierSets.fSourcePortStart; };
    void SourcePortStart(uint16 port) { fIpClassifierSets.fSourcePortStart = port;
                                        fIpClassifierSets.fSourcePortStartSpecified = true;
                                        fIpClassifierSets.fTcpUdpPortSpecified = true;
                                        fIpClassifierSetsSpecified = true;
                                      };

    // accessor for source TCP/UDP port end.  Type [22/23/60].9.8  default value = 65535
    uint16 SourcePortEnd() const { return fIpClassifierSets.fSourcePortEnd; };
    void SourcePortEnd(uint16 port) { fIpClassifierSets.fSourcePortEnd = port;
                                      fIpClassifierSets.fSourcePortEndSpecified = true;
                                      fIpClassifierSets.fTcpUdpPortSpecified = true;
                                      fIpClassifierSetsSpecified = true;
                                    };

    // accessor for dest TCP/UDP port start.  Type [22/23/60].9.9  default value = 0
    uint16 DestPortStart() const { return fIpClassifierSets.fDestPortStart; };
    void DestPortStart(uint16 port) { fIpClassifierSets.fDestPortStart = port;
                                      fIpClassifierSets.fDestPortStartSpecified = true;
                                      fIpClassifierSets.fTcpUdpPortSpecified = true;
                                      fIpClassifierSetsSpecified = true;
                                    };

    // accessor for dest TCP/UDP port end.  Type [22/23/60].9.10  default value = 65535
    uint16 DestPortEnd() const { return fIpClassifierSets.fDestPortEnd; };
    void DestPortEnd(uint16 port) { fIpClassifierSets.fDestPortEnd = port;
                                    fIpClassifierSets.fDestPortEndSpecified = true;
                                    fIpClassifierSets.fTcpUdpPortSpecified = true;
                                    fIpClassifierSetsSpecified = true;
                                  };

    // accessor for the boolean that indicates whether or not any of the IPv4
    // values (other than TCP/UDP source/dest port) were specified.
    inline bool Ipv4NonTcpUdpPortSpecified(void) const { return fIpClassifierSets.fIpv4NonTcpUdpPortSpecified; }

    // Accessors for the booleans that indicate whether or not the non-TCP/UDP
    // port values are specified.
    inline bool TosRangeAndMaskSpecified(void) const { return fIpClassifierSets.fTosRangeAndMaskSpecified; }
    inline bool ProtocolTypeSpecified(void) const { return fIpClassifierSets.fProtocolTypeSpecified; }
    inline bool SourceIpAddrSpecified(void) const { return fIpClassifierSets.fSourceIpAddrSpecified; }
    inline bool SourceIpMaskSpecified(void) const { return fIpClassifierSets.fSourceIpMaskSpecified; }
    inline bool DestIpAddrSpecified(void) const { return fIpClassifierSets.fDestIpAddrSpecified; }
    inline bool DestIpMaskSpecified(void) const { return fIpClassifierSets.fDestIpMaskSpecified; }

    // accessor for the boolean that indicates whether or not any of the
    // TCP/UDP source/dest port values were specified.
    inline bool TcpUdpPortSpecified(void) const { return fIpClassifierSets.fTcpUdpPortSpecified; }

    // Accessors for the booleans that indicate whether or not the source/dest
    // TCP/UDP ports are specified.
    inline bool SourcePortStartSpecified(void) const { return fIpClassifierSets.fSourcePortStartSpecified; }
    inline bool SourcePortEndSpecified(void) const { return fIpClassifierSets.fSourcePortEndSpecified; }
    inline bool DestPortStartSpecified(void) const { return fIpClassifierSets.fDestPortStartSpecified; }
    inline bool DestPortEndSpecified(void) const { return fIpClassifierSets.fDestPortEndSpecified; }
    inline bool SourcePortStartOrEndSpecified(void) const { return (fIpClassifierSets.fSourcePortStartSpecified || fIpClassifierSets.fSourcePortEndSpecified); }
    inline bool DestPortStartOrEndSpecified(void) const { return (fIpClassifierSets.fDestPortStartSpecified || fIpClassifierSets.fDestPortEndSpecified); }

    //
    // IsMatchingClassifierId - Returns true if the specified classifier 
    //      identifiers match the classifier identifiers of this object.
    //
    // Parameters:
    //      cmts_cls_id - 16 bit CMTS assigned classifier id
    //
    //      cm_cls_ref - 8 bit CM classifier reference
    //
    bool IsMatchingClassifierId( uint16 cmts_cls_id, byte cm_cls_ref = 0 ) const;




	// The TLV type code which started the classifier setting TLV.
    // Default = 0. Valid values are: Type 22/23 (DOCSIS 1.1)
    // Note: used to indicate whether classifier is for upstream (22) or
    // downstream (23).  there is very little difference between an
    // US vs. DS classifier.  an US classifier has (1) extra setting:
    // classifier activation signal.
    // 
    // PR12948 - DOCSIS 3.0 introduces Us Drop Classifiers (Type 60).  This
    // is identical to a Type 22 Us Classifier, minus the flow ref/id.
    byte fStartingTypeCode;

    // NOTE: the many
    //      bool < data member name >Specified
    // data members are required in order to know which data members have
    // been specified (and therefore need to be included in DSx messages).

    // UNFINISHED - there is a proposal to define 0 a reserved value
    // for the various ref and id items.  if this is accepted, we can
    // test for non-zero value for the ref/id data members rather than
    // maintaining ...IdSpecified data members.  make change soon!

	// CM classifier reference, used in CM config file, REG and DSA messages.
    // Assigned by provisioning server (REG-REQ) or CM (DSA-REQ).  Of no
    // interest once CMTS assigns an SFID and classifier id.  Type [22/23/60].1
    // (DOCSIS 1.1).  Default = 0.
	byte fCmClassifierRef;

	// CMTS assigned classifier id, used in REG-RSP and DSx messages.
    // Type [22/23/60].2  Default = 0.
	uint16 fCmtsClassifierId;

	// CM flow reference, used in CM config file, REG and DSA messages.
    // Assigned by provisioning server (REG-REQ) or CM (DSA-REQ).  Of no
    // interest once CMTS assigns a SID or an SFID.  Type [22/23].3
    // Default = 0.  
	uint16 fCmFlowRef;
    
	// Service flow identifier, a.k.a. sfid.  Used in REG and DSx messages.
    // Assigned by CMTS. Type [22/23].4  Default = 0.
    uint32 fSfid;

    // Rule priority.  Type [22/23/60].5  Context sensitive default(sic).
    byte fRulePriority;
    bool fRulePrioritySpecified;

    // Activation state.  non-zero value is active.  Type [22/23/60].6
    // default = 1.
    byte fIsActive;
    bool fIsActiveSpecified;

    // DSC message action.  Type [22/23/60].7
    // no default.  only relevant when context is DSC message exchange.
    byte fDscAction;
    bool fDscActionSpecified;

    // IP classifier settings.  Type [22/23/60].9  no default.
    BcmIpClassifierSettings fIpClassifierSets;
    bool fIpClassifierSetsSpecified;

    #if (BFC_INCLUDE_IPV6_SUPPORT)

    // PR13079 - IPv6 classifier settings.  Type [22/23/60].12  no default.
    // Added for PR13079.
    BcmIpv6ClassifierSettings fIpv6ClassifierSets;
    bool fIpv6ClassifierSetsSpecified;

    #endif

    // Ethernet LLC classifier settings.  Type [22/23/60].10  no default.
    BcmEthernetClassifierSettings fEthernetClassifierSets;
    bool fEthernetClassifierSetsSpecified;

    // IEEE 802.1 P/Q classifier settings.  Type [22/23/60].11  no default.
    Bcm8021PqClassifierSettings f8021PqClassifierSets;
    bool f8021PqClassifierSetsSpecified;

    // DSC activation signal.  Type 22.12
    // Note: pathetic logic rules for assigning a default value... we will
    // use 2 for now.
    // this is the only US specific classifier setting.  there are
    // no DS specific classifier settings.
    byte fDscActivationSignal;
    bool fDscActivationSignalSpecified;

    // CM Interface Mask Classification Criteria.  Type [22/60].13
    // Note:  For L2VPN and DOCSIS 3.0
    BcmCmim fCmim;
    bool fCmimSpecified;
    uint32 fInterfaceMask;

    // Used by clients to specify context info which determines some print
    // formatting decisions.  Possible values defined in TLVCodes.h
    int fUsageContext;

};

inline ostream& operator<<( ostream& ostrm, const BcmClassifierSettings& rhs )
{
    return rhs.Print( ostrm );
}

#endif
