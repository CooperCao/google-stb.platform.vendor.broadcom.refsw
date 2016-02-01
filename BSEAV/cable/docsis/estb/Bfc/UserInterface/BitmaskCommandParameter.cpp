//****************************************************************************
//
// Copyright (c) 2001-2009 Broadcom Corporation
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
//  Filename:       BitmaskCommandParameter.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 23, 2001
//
//****************************************************************************
//  Description:
//      This is the derived class for bitmask parameters.  It handles
//      parsing a mask of bits with optional operators (OR/AND/XOR/etc), and
//      limiting it to an optional set of valid bits.
//
//      NOTE:  All of the string pointers in the class MUST be static strings
//             that will not change or be deleted, etc., because only the string
//             pointers are copied and stored!
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "BitmaskCommandParameter.h"

#include "MessageLog.h"

#include <stdio.h>

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Default/Initializing Constructor.  Initializes the state of the object,
// storing the values specified.
//
// If pParameterName is NULL, then "Bitmask" will be used.
//
// Parameters:
//      clientId - the id that the client wants to associate with this
//                 parameter.
//      isOptional - true if the parameter is optional, false if it is
//                   required.
//      pParameterName - the name of this parameter (for printing).
//      validBits - the set of bits that can be set to 1.
//
// Returns:  N/A
//
BcmBitmaskCommandParameter::BcmBitmaskCommandParameter(unsigned int clientId, 
                                                       bool isOptional,
                                                       const char *pParameterName,
                                                       uint32 validBits) :
    BcmCommandParameter(clientId, isOptional, pParameterName, "Bitmask"),
    fOperation(kEQUAL),
    fValue(0),
    fValidBits(validBits)
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
BcmBitmaskCommandParameter::~BcmBitmaskCommandParameter()
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
ostream &BcmBitmaskCommandParameter::PrintValue(ostream &outstream) const
{
    return outstream << (void *) fValue;
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
void BcmBitmaskCommandParameter::PrintValue(char string[64]) const
{
    sprintf(string, "0x%lx", fValue);
}


// These methods are object cast handlers that allow a pointer to the base
// class to be cast as the specified type, and to provide safe access to
// the data in the derived class.  If the client uses these methods, then
// it is guaranteed not to crash due to an incorrect typecast, because the
// base class will catch any incorrect casts and handle them safely.
//
// There is one of these for every type of data that derived classes handle.

// Only implement the one that I care about.
uint32 BcmBitmaskCommandParameter::AsBitmask(uint32 currentValue) const
{
    switch (fOperation)
    {
        case kEQUAL:
            currentValue = fValue;
            break;

        case kAND:
            currentValue &= fValue;
            break;

        case kOR:
            currentValue |= fValue;
            break;

        case kXOR:
            currentValue ^= fValue;
            break;

        case kNOTAND:
            // Invert newValue, then AND.
            currentValue &= ~fValue;
            break;
    }

    // Make sure no invalid bits were set.
    return currentValue & fValidBits;
}


// These are similar to the above methods, except that they allow a client
// to set the current value rather than getting it.  Again, these methods
// provide type safety, etc.
//
// There isn't a Set method for the flag parameter value; you must get the
// object associated with the flag parameter and call it's Set method.

// Bitmask.  You pass in the current value, and the derived class performs
// the operation that was specified by the user on it to yield the value
// that is returned.
bool BcmBitmaskCommandParameter::SetAsBitmask(uint32 value)
{
    // UNFINISHED - may want to allow the user to specify the operation too.
    // For now, hard-wire it to assignment.
    fOperation = kEQUAL;

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
bool BcmBitmaskCommandParameter::ParseImpl(const char *pParameterString, 
                                           unsigned int &offset)
{
    int numScanned, newOffset, operationOffset = 1;
    uint32 newValue;
    Operation newOperation;
    const char *formatString = "%u%n";

    // See if the user specified an operation character.  If so, then start
    // parsing at the next char; otherwise, adjust the offset back so we don't
    // skip this char.
    switch (pParameterString[offset])
    {
        case '=':
            newOperation = kEQUAL;
            break;

        case '&':
            newOperation = kAND;
            break;

        case '|':
            newOperation = kOR;
            break;

        case '^':
            newOperation = kXOR;
            break;

        case '~':
            newOperation = kNOTAND;
            break;

        default:
            // Use assignment by default.
            newOperation = kEQUAL;
            operationOffset = 0;
            break;
    }

    // PR6650 - workaround for new GNU sscanf(), which limits "%i" to 7fffffff.
    // When reading the parameter, our default is to treat it as an unsigned
    // decimal string.  If it starts with '0', treat it as octal; or if it
    // starts with '0x', treat it as hexadecimal.
    if (pParameterString[offset + operationOffset] == '0')
    {
        formatString = "%o%n";
        
        if ((pParameterString[offset + operationOffset + 1] == 'x') ||
            (pParameterString[offset + operationOffset + 1] == 'X'))
        {
            formatString = "%x%n";
        }
    }

    // Try to parse the value.  numScanned will be 1 if a value was successfully
    // parsed, -1 if there was no parameter at all, or 0 if there was a
    // non-numerical parameter.
    numScanned = sscanf(pParameterString + offset + operationOffset, formatString, &newValue, &newOffset);

    // Make sure the value is in range (only valid bits are specified).
    if ((numScanned == 1) && ((newValue & (~fValidBits)) == 0))
    {
        fValue = newValue;
        fOperation = newOperation;
        offset += newOffset + operationOffset;

        return true;
    }

    // Don't print an error if this is an optional parameter.
    if (!IsOptional())
    {
        PrintError(pParameterString + offset, "Must be a bitmask with only the specified bits!");
    }

    return false;
}


// Gives the derived class a chance to do some derived-class specific stuff
// during the print.
//
// Parameters:
//      outstream - the ostream to be used.
//
// Returns:
//      A reference to the ostream for chaining insertion operators.
//
ostream &BcmBitmaskCommandParameter::PrintImpl(ostream &outstream) const
{
    // If the range was limited, then print the range.
    if (fValidBits != 0xffffffff)
    {
        outstream << "{" << (void *) fValidBits << "}";
    }

    return outstream;
}


