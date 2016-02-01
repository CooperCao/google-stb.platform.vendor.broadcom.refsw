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
//  $Id: IpAddressList.cpp 1.8 2006/11/16 21:09:49Z dpullen Release $
//
//  Filename:       IpAddressList.cpp
//  Author:         David Pullen
//  Creation Date:  June 21, 2002
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "IpAddressList.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************

//********************** Class Method Implementations ************************


BcmIpAddressList::BcmIpAddressList(void) :

    // Initially, don't allocate any memory.
    fIpAddressBuffer(0),
    fIpv6AddressBuffer(0)
{
    // Set protected and private members to a known state.
}


BcmIpAddressList::BcmIpAddressList(const BcmIpAddressList &otherInstance) :

    // Initially, don't allocate any memory.
    fIpAddressBuffer(0),
    fIpv6AddressBuffer(0)
{
    // Usually, you can just call the assignment operator to do the work.
    *this = otherInstance;
}


BcmIpAddressList::~BcmIpAddressList()
{
    // Nothing to do...
}


BcmIpAddressList & BcmIpAddressList::operator = (const BcmIpAddressList &otherInstance)
{
    // Copy protected and private members of otherInstance to my members.
    fIpAddressBuffer = otherInstance.fIpAddressBuffer;
    fIpv6AddressBuffer = otherInstance.fIpv6AddressBuffer;

    return *this;
}


BcmIpAddressList & BcmIpAddressList::operator = (const BcmOctetBuffer &buffer)
{
    // UNFINISHED - what about IPv6?
    fIpAddressBuffer = buffer;

    return *this;
}

BcmIpAddress BcmIpAddressList::operator [] (unsigned int index) const
{
    BcmIpAddress ipAddress;

    if ((index * BcmIpV4Address::ADDR_LEN) < fIpAddressBuffer.BytesUsed())
    {
        // Convert the index to a byte offset.
        index *= BcmIpV4Address::ADDR_LEN;

        // Try to read from that offset.  If this fails, then the IP address will
        // still be 0.0.0.0, and this is what we will return.
        ipAddress.ReadFrom(fIpAddressBuffer, index);
    }
    else if ((index * BcmIpV6Address::ADDR_LEN) < fIpv6AddressBuffer.BytesUsed())
    {
        BcmIpV6Address ipv6Address;

        index *= BcmIpV6Address::ADDR_LEN;

        ipv6Address.ReadFrom(fIpv6AddressBuffer, index);

        ipAddress = ipv6Address;
    }

    return ipAddress;
}

bool BcmIpAddressList::operator == (const BcmIpAddressList &otherInstance) const
{
    // Very simplistic comparison; this will return false if the other
    // instance has the same IP addresses, but in a different order.  May want
    // to beef this up if we decide that this is more generally useful.
    if ((fIpAddressBuffer == otherInstance.fIpAddressBuffer) &&
        (fIpv6AddressBuffer == otherInstance.fIpv6AddressBuffer))
    {
        return true;
    }

    return false;
}


void BcmIpAddressList::Add(const BcmIpAddress &ipAddress)
{
    // First, check to see if this address is already in the list.  Don't add
    // the address multiple times.
    if (IsPresent(ipAddress))
    {
        return;
    }

    if (ipAddress.IsIpV6Address())
    {
        // Next, check to see if there's room in the octet buffer to hold the new
        // address.  If not, increase the buffer size.
        if (fIpv6AddressBuffer.BytesAvailable() < BcmIpV6Address::ADDR_LEN)
        {
            // Create a larger buffer.
            BcmOctetBuffer tempBuffer(fIpv6AddressBuffer.BufferSize() + (BcmIpV6Address::ADDR_LEN * 8));

            // Copy the data from the original buffer.
            tempBuffer.Append(fIpv6AddressBuffer);

            // Move the new buffer over to my data member.  Its previous buffer
            // will be released back to the heap.
            fIpv6AddressBuffer.AdoptBuffer(tempBuffer.AccessBuffer(),
                                           tempBuffer.BufferSize(),
                                           tempBuffer.BytesUsed());

            // Make the temp object forget that it had a buffer.  It is now owned
            // by my object, and will be released when it is deleted.
            tempBuffer.ReleaseBuffer();
        }

        // Finally, add the IP address to the buffer.
        ipAddress.WriteTo(fIpv6AddressBuffer);
    }
    else
    {
        // Next, check to see if there's room in the octet buffer to hold the new
        // address.  If not, increase the buffer size.
        if (fIpAddressBuffer.BytesAvailable() < BcmIpV4Address::ADDR_LEN)
        {
            // Create a larger buffer.
            BcmOctetBuffer tempBuffer(fIpAddressBuffer.BufferSize() + (BcmIpV4Address::ADDR_LEN * 8));

            // Copy the data from the original buffer.
            tempBuffer.Append(fIpAddressBuffer);

            // Move the new buffer over to my data member.  Its previous buffer
            // will be released back to the heap.
            fIpAddressBuffer.AdoptBuffer(tempBuffer.AccessBuffer(),
                                         tempBuffer.BufferSize(),
                                         tempBuffer.BytesUsed());

            // Make the temp object forget that it had a buffer.  It is now owned
            // by my object, and will be released when it is deleted.
            tempBuffer.ReleaseBuffer();
        }

        // Finally, add the IP address to the buffer.
        ipAddress.WriteTo(fIpAddressBuffer);
    }
}


bool BcmIpAddressList::Remove(const BcmIpAddress &ipAddress)
{
    BcmIpAddress listAddress;
    unsigned int readOffset, writeOffset;
    bool foundAndRemoved = false;

    // Iterate through the octet buffer, reading IP addresses out.
    readOffset = 0;
    writeOffset = 0;
    while (listAddress.ReadFrom(fIpAddressBuffer, readOffset))
    {
        // Compare the address from the list to the one passed in.  If it
        // doesn't match, write it back at the current write offset.
        if (listAddress != ipAddress)
        {
            fIpAddressBuffer.Overwrite(writeOffset, listAddress.NetworkGet());
            writeOffset += BcmIpV4Address::ADDR_LEN;
        }
        else
        {
            foundAndRemoved = true;
        }
    }

    // Trim bytes from the end corresponding to addresses that were removed.
    fIpAddressBuffer.SetBytesUsed(writeOffset);

    // If we removed any addresses from the list, then we need to update the
    // number of valid bytes in the buffer.  The write offset indicates the
    // end of the valid data.
    fIpAddressBuffer.SetBytesUsed(writeOffset);

    if (!foundAndRemoved)
    {
        BcmIpV6Address v6ListAddress;

        // Iterate through the octet buffer, reading IP addresses out.
        readOffset = 0;
        writeOffset = 0;
        while (v6ListAddress.ReadFrom(fIpv6AddressBuffer, readOffset))
        {
            // Compare the address from the list to the one passed in.  If it
            // doesn't match, write it back at the current write offset.
            if (ipAddress != v6ListAddress)
            {
                uint8 ipv6AddressBuffer[BcmIpV6Address::ADDR_LEN];

                v6ListAddress.Get(ipv6AddressBuffer);

                fIpv6AddressBuffer.Overwrite(writeOffset, ipv6AddressBuffer, sizeof(ipv6AddressBuffer));
                writeOffset += BcmIpV6Address::ADDR_LEN;
            }
            else
            {
                foundAndRemoved = true;
            }
        }

        // Trim bytes from the end corresponding to addresses that were removed.
       fIpv6AddressBuffer.SetBytesUsed(writeOffset);
    }

    return foundAndRemoved;
}


bool BcmIpAddressList::IsPresent(const BcmIpAddress &ipAddress) const
{
    BcmIpAddress listAddress;
    unsigned int offset;

    // Iterate through the octet buffer, reading IP addresses out.
    offset = 0;
    while (listAddress.ReadFrom(fIpAddressBuffer, offset))
    {
        // Compare the address from the list to the one passed in.  If it
        // matches, return true.
        if (listAddress == ipAddress)
        {
            return true;
        }
    }

    {
        BcmIpV6Address v6ListAddress;

        // Iterate through the octet buffer, reading IP addresses out.
        offset = 0;
        while (v6ListAddress.ReadFrom(fIpv6AddressBuffer, offset))
        {
            // Compare the address from the list to the one passed in.  If it
            // matches, return true.
            if (ipAddress == v6ListAddress)
            {
                return true;
            }
        }
    }

    // If we get this far, the list is empty or there isn't a matching address
    // in the list.
    return false;
}


void BcmIpAddressList::Reset(void)
{
    fIpAddressBuffer.Empty();
    fIpv6AddressBuffer.Empty();
}


ostream &BcmIpAddressList::Print(ostream &outStream) const
{
    unsigned int index = 0;
    bool firstTime = true;

    while (index < Size())
    {
        if (!firstTime)
        {
            outStream << "; ";
        }

        firstTime = false;

        outStream << (*this)[index];

        index++;
    }

    return outStream;
}


