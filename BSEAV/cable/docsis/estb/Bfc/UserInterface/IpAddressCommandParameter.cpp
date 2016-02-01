//****************************************************************************
//
//  Copyright (c) 2001  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id: IpAddressCommandParameter.cpp 1.6 2006/06/27 17:05:49Z sdunlap Release $
//
//  Filename:       IpAddressCommandParameter.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 22, 2001
//
//****************************************************************************
//  Description:
//      This is the derived class for MAC Address parameters.  It handles
//      parsing a MAC address.
//
//      NOTE:  All of the string pointers in the class MUST be static strings
//             that will not change or be deleted, etc., because only the string
//             pointers are copied and stored!
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "IpAddressCommandParameter.h"

#include "MessageLog.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default/Initializing Constructor.  Initializes the state of the object,
// storing the values specified.
//
// If pParameterName is NULL, then "IpAddress" will be used.
//
// Parameters:
//      clientId - the id that the client wants to associate with this
//                 parameter.
//      isOptional - true if the parameter is optional, false if it is
//                   required.
//      pParameterName - the name of this parameter (for printing).
//
// Returns:  N/A
//
BcmIpAddressCommandParameter::BcmIpAddressCommandParameter(unsigned int clientId, 
                                                           bool isOptional,
                                                           const char *pParameterName) :
    BcmCommandParameter(clientId, isOptional, pParameterName, "IpAddress"),
    fValue(kAllZerosIp)
{
    // Nothing else to do.
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmIpAddressCommandParameter::~BcmIpAddressCommandParameter()
{
    // Nothing else to do.
}


// Prints the value that is currently stored (regardless of whether or not
// parsing was successful) to the specified ostream.
//
// This method must be implemented by the derived class.
//
// Parameters:
//      outstream - the ostream to be used.
//
// Returns:
//      A reference to the ostream for chaining insertion operators.
//
ostream &BcmIpAddressCommandParameter::PrintValue(ostream &outstream) const
{
    return outstream << fValue;
}


// Prints the value that is currently stored (regardless of whether or not
// parsing was successful) to the specified string.
//
// This method must be implemented by the derived class.
//
// Parameters:
//      outstream - the ostream to be used.
//
// Returns:  Nothing.
//
void BcmIpAddressCommandParameter::PrintValue(char string[64]) const
{
    fValue.Get(string, 64);
}


// These methods are object cast handlers that allow a pointer to the base
// class to be cast as the specified type, and to provide safe access to
// the data in the derived class.  If the client uses these methods, then
// it is guaranteed not to crash due to an incorrect typecast, because the
// base class will catch any incorrect casts and handle them safely.
//
// There is one of these for every type of data that derived classes handle.

// Only implement the one that I care about.
const BcmIpAddress &BcmIpAddressCommandParameter::AsIpAddress(void) const
{
    return fValue;
}


// These are similar to the above methods, except that they allow a client
// to set the current value rather than getting it.  Again, these methods
// provide type safety, etc.
//
// There isn't a Set method for the flag parameter value; you must get the
// object associated with the flag parameter and call it's Set method.

// IP Address.
bool BcmIpAddressCommandParameter::SetAsIpAddress(const BcmIpAddress &value)
{
    fValue = value;

    return true;
}


// This must be implemented by the derived class.  It is a "template method"
// that allows the derived class to do the real work of parsing the string
// and updating its state.
//
// The derived class can assume that the pParameterString pointer is not
// NULL.
//
// Parameters:
//      pParameterString - pointer to the string containing the parameters
//                         to be parsed.  It will not be modified.
//      offset - the offset into the string where parsing should begin.
//
// Returns:
//      true if the parameter was successfully parsed.
//      false if the parameter was not successfully parsed.
//
bool BcmIpAddressCommandParameter::ParseImpl(const char *pParameterString, 
                                             unsigned int &offset)
{
    unsigned int newOffset = 0;

    // A IP address knows how to parse itself from a string, so let it do its
    // job.
    //
    // NOTE - IP address will not destroy its data if this fails; it does its
    // work in a temporary buffer and only updates itself if the parsing is
    // successful.
    if (fValue.Set(pParameterString + offset, &newOffset))
    {
        offset += newOffset;

        return true;
    }

    // Don't print an error if this is an optional parameter.
    if (!IsOptional())
    {
        PrintError(pParameterString + offset, "Must be a valid IP Address!");
    }

    return false;
}


