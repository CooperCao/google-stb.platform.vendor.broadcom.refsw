//****************************************************************************
//
//  Copyright (c) 2002-2003  Broadcom Corporation
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
//  $Id: IpAddressList.h 1.8 2006/06/27 17:05:14Z sdunlap Release $
//
//  Filename:       IpAddressList.h
//  Author:         David Pullen
//  Creation Date:  June 21, 2002
//
//****************************************************************************

#ifndef IPADDRESSLIST_H
#define IPADDRESSLIST_H

//********************** Include Files ***************************************

#include "OctetBuffer.h"
#include "IpAddress.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


/**
*      Helper class that deals with lists of IP addresses.  This is used by
*      a number of helper classes where more than one IP address can be
*      specified.  For example, in BcmTimeOfDayThread, we can be provided a
*      list of ToD servers to try (via DHCP), and we need an easy way to pass
*      this set of IP addresses in.
*/
class BcmIpAddressList
{
public:

    /// Default Constructor.  Initializes the state of the object...
    ///
    BcmIpAddressList(void);

    /// Copy Constructor.  Initializes the state of this object to match that
    /// of the instance passed in.
    ///
    /// \param
    ///      otherInstance - reference to the object to copy.
    ///
    BcmIpAddressList(const BcmIpAddressList &otherInstance);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    ~BcmIpAddressList();

    /// Assignment operator.  Copies the state of the instance passed in so that
    /// this object is "identical".
    ///
    /// \param
    ///      otherInstance - reference to the object to copy.
    ///
    /// \return
    ///      A reference to "this" so that operator = can be chained.
    ///
    BcmIpAddressList & operator = (const BcmIpAddressList &otherInstance);

    /// Assignment operator, from an octet buffer.
    ///
    /// \param
    ///      buffer - the octet buffer to copy from.
    ///
    /// \return
    ///      A reference to "this" so that operator = can be chained.
    ///
    BcmIpAddressList & operator = (const BcmOctetBuffer &buffer);

    /// The subscripting operator.  This can only be used to read from a
    /// particular offset.  Attempts to read past the end cause kAllZerosIp to
    /// be returned.
    ///
    /// \param
    ///      index - the 0-based offset to read from.
    ///
    /// \return
    ///      The IP address at the specified index.
    ///
    BcmIpAddress operator [] (unsigned int index) const;

    /// Equality comparison operator.
    ///
    /// \note Not sure how useful this really is, aside from regression
    ///       testing.  It just does simple buffer comparisons; it will
    ///       return false if the two objects have the same IP addresses,
    ///       but they are in different orders.
    ///
    /// \param
    ///     otherInstance - reference to the object to compare against.
    ///
    /// \retval
    ///     true if the two objects are equal.
    /// \retval
    ///     false if the two objects are not equal.
    ///
    bool operator == (const BcmIpAddressList &otherInstance) const;

    /// Inequality comparison operator.  This is the inverse of operator ==.
    ///
    /// \param
    ///     otherInstance - reference to the object to compare against.
    ///
    /// \retval
    ///     true if the two objects are not equal.
    /// \retval
    ///     false if the two objects are equal.
    ///
    inline bool operator != (const BcmIpAddressList &otherInstance) const;

    /// Allows an IP address to be added to the list (to the end).
    ///
    /// \note If the address is already in the list, it will not be added a
    ///       second time.
    ///
    /// \param
    ///     ipAddress - the IP address to add to the list.
    ///
    void Add(const BcmIpAddress &ipAddress);

    /// Allows an IP address to be removed from the list.  IP addresses
    /// following the specified address will be moved forward (so there are no
    /// gaps).
    ///
    /// \note If the address is in the list multiple times, all copies will be
    ///       removed.  This should only happen if this object was created with
    ///       an octet buffer that had multiple copies of the address.
    ///
    /// \param
    ///     ipAddress - the IP address to be removed from the list.
    ///
    /// \retval
    ///     true if the IP address was found and removed.
    /// \retval
    ///     false if the IP address was not found.
    ///
    bool Remove(const BcmIpAddress &ipAddress);

    /// Checks to see if the specified IP address is in the list.
    ///
    /// \retval
    ///     true if the IP address was found.
    /// \retval
    ///     false if the IP address was not found.
    ///
    bool IsPresent(const BcmIpAddress &ipAddress) const;

    /// Resets the internal state.
    ///
    void Reset(void);

    /// Returns the number of IP addresses.  This can be used as the upper
    /// bound of a loop.
    ///
    /// \return
    ///      The number of IP addresses.
    ///
    inline unsigned int Size(void) const;

    /// Accessor for the octet buffer.  Most often used for writing the buffer
    /// out to a packet.
    ///
    /// \return
    ///      The octet buffer.
    ///
    inline const BcmOctetBuffer &AsBuffer(void) const;

    /// Prints (as a series of IP addresses) to the specified ostream.
    ///
    /// \param
    ///      outStream - the ostream that you want me to print to.
    ///
    /// \return
    ///      A reference to the ostream.
    ///
    ostream &Print(ostream &outStream) const;

private:

    /// I actually store the IP addresses in an octet buffer, since the list
    /// is most often gotten from an octet buffer.
    BcmOctetBuffer fIpAddressBuffer;

    /// UNFINISHED - hack for IPv6 support; this is a short-term workaround
    /// to add IPv6.  I think the real method for dealing with this is to
    /// use a list/vector of BcmIpAddress object rather than an octet buffer.
    BcmOctetBuffer fIpv6AddressBuffer;

};


//********************** Inline Method Implementations ***********************


/// Lets you insert this object to an ostream.
inline ostream & operator << (ostream &outStream, const BcmIpAddressList &rhs)
{
    return rhs.Print(outStream);
}


inline bool BcmIpAddressList::operator != (const BcmIpAddressList &otherInstance) const
{
    return !(*this == otherInstance);
}


inline unsigned int BcmIpAddressList::Size(void) const
{
    // There is one IP address for every 4 bytes, and one IPv6 address for
    // every 16 bytes.
    return (fIpAddressBuffer.BytesUsed() / BcmIpV4Address::ADDR_LEN) +
           (fIpv6AddressBuffer.BytesUsed() / BcmIpV6Address::ADDR_LEN);
}


inline const BcmOctetBuffer &BcmIpAddressList::AsBuffer(void) const
{
    // UNFINISHED - what about IPv6?
    return fIpAddressBuffer;
}


#endif


