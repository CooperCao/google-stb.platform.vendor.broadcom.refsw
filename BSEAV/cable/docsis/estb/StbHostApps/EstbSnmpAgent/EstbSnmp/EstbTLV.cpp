//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
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
//  Filename:       EstbTLV.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  8 NOV 2006
//
//****************************************************************************
//  Description:
//      This file contains helper functions for dealing with the creation and
//      parsing of TLVs. 
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbTLV.h"

// For strlen() and dealing with C strings.
#include <string.h>
#include <stdio.h>

#include <iostream>
#include <iomanip>

#include "IpAddress.h"
#include "MacAddress.h"

#include "MessageLog.h"

#include "EventLogApi.h"
#include "EstbEvents.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

// This is no longer being used; the code below is commented out.
/*static const char *gTlvTypeName[] =
{
    "uint8",
    "int8",
    "uint16",
    "int16",
    "uint32",
    "int32",
    "String",
    "OctetBuffer",
    "StringOctetBuffer",
    "FixedByteArray",
    "Nested",
    "IpAddress",
    "MacAddress",

    "EndOfTable"
};*/

//********************** Local Variables *************************************

//********************** Local Functions *************************************

static BcmMessageLogSettings *MessageLogSettings(void);
static void LogTypeLength(byte TypeField, char* TypeName, uint16 LengthField,
                          BcmMessageLogSettings *pMessageLogSettings,
                          bool LeadingNewline = false);

//********************** Class Method Implementations ************************


// This function is the entrypoint for the TLV parsing engine.  It accepts a
// reference to an octet buffer (along with an offset into the buffer), and a
// TLV table.  It reads a T and L value from the buffer, and searches through
// the table for the specified T code.  If found, and if the table entry has
// a non-0 length value in it, then it validates that the length matches what
// is in the table.  If the length validation passes, then it stores the value
// in the table.  The type of the value, and how it is handled/stored depends
// on the type hint that you provide in the table.
//
// If the T code is not found in the table, then it displays a warning message
// (along with the TLV octets), skips over the V, and then continues with the
// next TLV.
//
// You can make the parser abort operation when it encounters a given type code
// by setting the appropriate flag in the table.  When this happens, the offset
// is "rewound" so that if refers to the T code that caused the parse to stop.
//
// Note that, on return, the offset parameter is modified to refer to the next
// un-parsed byte of data in the buffer.
//
// If you provide a pointer to a BcmMessageLogSettings instance, then it will
// be used to display verbose parsing information.  Informational messages must
// be enabled for your settings instance for the messages to be written.  If
// you don't provide one, then only errors and warning will be displayed, using
// the default settings object for this module.
//
// Parameters:
//      buffer - reference to the buffer from which the TLVs should be parsed.
//      offset - the offset into the buffer of the first TLV byte.
//      pTLVParseTable - pointer to the first TLVTable element in the array.
//      pMessageLogSettings - pointer to the message log settings instance to
//                            be used when outputting message log info.  If NULL,
//                            then it will use its own instance.
//      lengthIsUint16 - tells me how many bytes are to be used for the Length
//                       field of the TLV.  If false, this will be the normal
//                       uint8 representation; if true, it will be a uint16.
//      lengthIncludesTypeAndLength - length field includes the type and length itself
//                       in its value.
//
// Returns:
//      One of the enum values above.
//
TLVParserResult ParseTLVs(const BcmOctetBuffer &buffer, unsigned int &offset,
                          TLVTable *pTLVParseTable, 
                          BcmMessageLogSettings *pMessageLogSettings,
                          bool lengthIsUint16,
                          bool lengthIncludesTypeAndLength)
{
    uint8 typeField, lengthField8;
    uint16 lengthField16;
    unsigned int lengthField;
    int index;
    unsigned int sizeOfLength;

    // If the user didn't supply a message log settings instance, then use my
    // own.
    if (pMessageLogSettings == NULL)
    {
        pMessageLogSettings = MessageLogSettings();
    }

    // Some error checking.
    if (pTLVParseTable == NULL)
    {
        gErrorMsg(*pMessageLogSettings, "ParseTLVs")
            << "NULL TLV decode table passed in!" << endl;

        return kTLVParserErrorNULLTable;
    }

    // Set the expected size of the length field based on the user's parameter.
    if (lengthIsUint16)
    {
        sizeOfLength = 2;
    }
    else
    {
        sizeOfLength = 1;
    }

    // Keep processing TLVs until we run out of buffer space, or we run across
    // a condition that keeps us from proceeding.  Note that the test below
    // causes us to break out if there aren't enough bytes of valid data left
    // in the buffer; there MUST be at least enough for the T and L fields for
    // this to succeed at all.
    while (offset + sizeOfLength < buffer.BytesUsed())
    {
        // Get the type and length fields out of the buffer.  Since we
        // pre-conditioned the loop test (above) so that there will always be
        // at least 2 bytes available, we don't need to do any validation on
        // the calls to NetworkRead().
        buffer.NetworkRead(offset, typeField);
        // printf ("ParseTLV type %d at offset %d\n", typeField, offset - 1);

        // Pull out the length field with the appropriate number of bytes, then
        // move this to the unsigned int version so that I only have to use it
        // later on.
        if (lengthIsUint16)
        {
            buffer.NetworkRead(offset, lengthField16);
            lengthField = lengthField16;
        }
        else
        {
            buffer.NetworkRead(offset, lengthField8);
            lengthField = lengthField8;
        }

        // printf ("ParseTLV type %d length %d at offset %d\n", typeField, lengthField, offset - 1);
        // Spec says (Section 6.2.1.3.2) that all TLV length fields MUST be
        // greater than zero.  Does this mean that we should call this and error,
        // or should we just ignore it and continue?  For now, I'll just flag
        // this as a warning and continue.
        if ((lengthField == 0) && (typeField != kEndOfDataMarker))
        {
			#if !defined(BCM_STANDALONE_DSGCC)
            gWarningMsg(*pMessageLogSettings, "ParseTLVs")
                << "TLV length field is 0!  This is a spec violation!  Continuing anyway..." << endl;
			#endif
        }

        // adjust length if the length includes the type and length itself
        if (lengthIncludesTypeAndLength)
        {
            unsigned int tlLength = 2;

            if (lengthIsUint16)
            {
                tlLength = 3;
            }

            // error check for length field that is too small for this case
            if (lengthField < tlLength)
            {
                gErrorMsg(*pMessageLogSettings, "ParseTLVs") << "Length field must be at least " << tlLength << endl;
                return kTLVParserErrorInvalidLength;
            }

            lengthField -= tlLength;
        }

        // Look for the type field in our table.  Break out if we find either
        // the type we are looking for, or if we reach the end of the table.
        index = 0;
        while ((pTLVParseTable[index].type != kEndOfDataMarker) &&
               (pTLVParseTable[index].type != typeField))
        {
            index++;
        }

        // If the value was unknown, log an error
        if ((pTLVParseTable[index].type == kEndOfDataMarker) &&
            (typeField != 255)) {
          char tlvStr[32];
          sprintf((char *) tlvStr, "TLV%u", typeField);
          EstbLogEvent(kEE_T40101, tlvStr);
        }

        // If the value was already found,  and there should be just one,
        // then skip it, and return with an indication so we can log it.
        if (pTLVParseTable[index].wasFound && pTLVParseTable[index].isUnique) {
          // printf ("ParseTLV found duplicate sub-TLV type %d at offset %d \n", typeField, offset - 1);
          offset += lengthField;
          // printf ("ParseTLV adjusting offset to %d\n", offset);
          return kTLVParserDuplicateTLV;
        }

        // See if the user wants us to break out of the parsing engine; this
        // generally happens for nested types where they need to create a new
        // instance of a struct or class before continuing with the parsing.
        else if (pTLVParseTable[index].stopParsing)
        {
            LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                           pMessageLogSettings, true );

            gInfoMsg(*pMessageLogSettings, "ParseTLVs") << endl;

            // Back the offset up so that it will refer to the T and L codes
            // again.  The caller will likely need these in order to know what
            // to do next.
            offset -= 1 + sizeOfLength;

            return kTLVParserStopParsingFound;
        }

        // If we know how long the value should be, then verify that the length
        // specified matches.  For some TLVs (uint8, etc.) the length is known
        // in advance.
        if ((pTLVParseTable[index].length != kUnknownLength) &&
            (pTLVParseTable[index].length != lengthField))
        {
            gErrorMsg(*pMessageLogSettings, "ParseTLVs")
                << " -> TLV " << (unsigned int) typeField << " has a length of "
                << lengthField << "; expected length of "
                << pTLVParseTable[index].length << '!' << endl;

            return kTLVParserErrorInvalidLength;
        }

        // Make sure we have enough bytes left in the buffer for the value.
        if (lengthField + offset > buffer.BytesUsed())
        {
            gErrorMsg(*pMessageLogSettings, "ParseTLVs")
                << " -> Not enough bytes left for the value!" << endl;

            return kTLVParserErrorInsufficientData;
        }

        // UNFINISHED - Do we really want to do this?  Maybe a NULL value
        // pointer should mean that the user doesn't care what the value is,
        // but is just interested in whether the value was present or not.
        //
        // Also need to verify that the user-supplied pointer to the variable
        // is not NULL.
        if ((pTLVParseTable[index].type != kEndOfDataMarker) &&
            (pTLVParseTable[index].uTLV_pUnknown == NULL))
        {
            gErrorMsg(*pMessageLogSettings, "ParseTLVs")
                << " -> The pointer to the variable in this table entry is NULL!  Table index = " 
                << index << endl;

            return kTLVParserErrorNULLValuePointer;
        }

        // Set the was-found flags for this table entry.
        pTLVParseTable[index].wasFound = true;

        // If the client set up a was-specified bool pointer, then set
        // it to true as well.
        if (pTLVParseTable[index].pValueWasSpecified != NULL)
        {
            *pTLVParseTable[index].pValueWasSpecified = true;
        }
        
        // Store and/or process the value field based on the type.  I have
        // already verified that the pointer to the variable is not NULL, so I
        // don't do any error checking for that.  Also, we have already verified
        // that there are enough bytes available for the whole value, so I don't
        // do any validation there.
        switch (pTLVParseTable[index].valueEncoding)
        {
            case kTLV_uint8:
                buffer.NetworkRead(offset, *(pTLVParseTable[index].uTLV_pUint8Value));

                LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                               pMessageLogSettings );

                gInfoMsgNoFields(*pMessageLogSettings) 
                    << "0x" 
                    << setfill( '0' ) << hex << setw( 2 )
                    << uint16( (*(pTLVParseTable[index].uTLV_pUint8Value)) )
                    << " (" << dec // << setw( 3 ) 
                    << uint16( (*(pTLVParseTable[index].uTLV_pUint8Value)) )
                    << ") " << hex 
                    << endl;
                break;

            case kTLV_int8:
                buffer.NetworkRead(offset, *(pTLVParseTable[index].uTLV_pInt8Value));

                LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                               pMessageLogSettings );

                gInfoMsgNoFields(*pMessageLogSettings) 
                    << "0x" 
                    << setfill( '0' ) << hex << setw( 2 )
                    << int16( (*(pTLVParseTable[index].uTLV_pInt8Value)) )
                    << " (" << dec // << setw( 3 ) 
                    << int16( (*(pTLVParseTable[index].uTLV_pInt8Value)) )
                    << ") " << hex
                    << endl;

                break;

            case kTLV_uint16:
                buffer.NetworkRead(offset, *(pTLVParseTable[index].uTLV_pUint16Value));

                LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                               pMessageLogSettings );

                gInfoMsgNoFields(*pMessageLogSettings) 
                    << "0x" 
                    << setfill( '0' ) << hex << setw( 4 )
                    << uint16( (*(pTLVParseTable[index].uTLV_pUint16Value)) )
                    << " (" << dec // << setw( 5 ) 
                    << uint16( (*(pTLVParseTable[index].uTLV_pUint16Value)) )
                    << ") " << hex
                    << endl;

                break;

            case kTLV_int16:
                buffer.NetworkRead(offset, *(pTLVParseTable[index].uTLV_pInt16Value));

                LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                               pMessageLogSettings );

                gInfoMsgNoFields(*pMessageLogSettings) 
                    << "0x" 
                    << setfill( '0' ) << hex << setw( 4 )
                    << int16( (*(pTLVParseTable[index].uTLV_pInt16Value)) )
                    << " (" << dec // << setw( 5 ) 
                    << int16( (*(pTLVParseTable[index].uTLV_pInt16Value)) )
                    << ") " << hex
                    << endl;

                break;

            case kTLV_uint32:
                buffer.NetworkRead(offset, *(pTLVParseTable[index].uTLV_pUint32Value));

                LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                               pMessageLogSettings );

                gInfoMsgNoFields(*pMessageLogSettings) 
                    << "0x" 
                    << setfill( '0' ) << hex << setw( 8 )
                    << uint32( (*(pTLVParseTable[index].uTLV_pUint32Value)) )
                    << " (" << dec // << setw( 10 ) 
                    << uint32( (*(pTLVParseTable[index].uTLV_pUint32Value)) )
                    << ") " << hex
                    << endl;

                break;

            case kTLV_int32:
                buffer.NetworkRead(offset, *(pTLVParseTable[index].uTLV_pInt32Value));

                LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                               pMessageLogSettings );

                gInfoMsgNoFields(*pMessageLogSettings) 
                    << "0x" 
                    << setfill( '0' ) << hex << setw( 8 )
                    << int32( (*(pTLVParseTable[index].uTLV_pInt32Value)) )
                    << " (" << dec // << setw( 10 ) 
                    << int32( (*(pTLVParseTable[index].uTLV_pInt32Value)) )
                    << ") " << hex
                    << endl;

                break;

            case kTLV_string:
            case kTLV_StringOctetBuffer:
                {
                    bool terminatorPresent = true;
                    char *pString;
                    unsigned int originalOffset = offset;

                    if (pTLVParseTable[index].valueEncoding == kTLV_string)
                    {
                        pString = pTLVParseTable[index].uTLV_pStringValue;
                    }
                    else
                    {
                        pString = (char *) pTLVParseTable[index].uTLV_pOctetBuffer->AccessBuffer();
                    }

                    // Copy the string bytes.
                    buffer.Read(offset, (uint8 *) pString, lengthField);

                    // UNFINISHED - Note, there was talk at one time about
                    // removing the requirement to include the \0 at the end of
                    // the string.  If this ever comes through, we will need to
                    // append one unconditionally, without complaining.
                    //
                    // Make sure there is a \0 at the end.
                    if (pString[lengthField - 1] != '\0')
                    {
                        pString[lengthField] = '\0';
                        terminatorPresent = false;
                    }

                    LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                                   pMessageLogSettings );

                    BcmOctetBuffer myOctetBuffer(0);
                    myOctetBuffer.Overlay(buffer, originalOffset, lengthField);

                    gInfoMsgNoFields(*pMessageLogSettings) 
                        << "'" << pString << "' " << myOctetBuffer;

                    if (!terminatorPresent)
                    {
                        gWarningMsgNoFields(*pMessageLogSettings) << "\n";
                        gWarningMsg(*pMessageLogSettings, "ParseTLVs")
                            << "0-terminator missing from string TLV!  I added one." << endl;
                    }

                    if (pTLVParseTable[index].valueEncoding == kTLV_StringOctetBuffer)
                    {
                        if (terminatorPresent)
                        {
                            pTLVParseTable[index].uTLV_pOctetBuffer->SetBytesUsed(lengthField);
                        }
                        else
                        {
                            pTLVParseTable[index].uTLV_pOctetBuffer->SetBytesUsed(lengthField + 1);
                        }
                    }
                }
                break;

            case kTLV_nested:
                {
                    TLVParserResult result;
                    BcmOctetBuffer nestedTLVs(0);
                    unsigned int nestedOffset = 0;

                    LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                                   pMessageLogSettings, true );

                    gInfoMsgNoFields(*pMessageLogSettings) 
                        << '\n'
                        << " V=nested TLVs; recursing." 
                        << endl;

                    nestedTLVs.Overlay(buffer, offset, lengthField);

                    // I need to recurse, giving a pointer to the nested parsing
                    // table.
                    result = ParseTLVs(nestedTLVs, nestedOffset, 
                                       pTLVParseTable[index].uTLV_pTLVTable,
                                       pMessageLogSettings,
                                       lengthIsUint16,
                                       lengthIncludesTypeAndLength);
                    offset += nestedOffset;

                    // add an extra newline to the log in order to more easily
                    // see the end of nested type.
                    gInfoMsgNoFields(*pMessageLogSettings) << endl;

                    // If there was a  problem, then silently bail out.
                    if (result != kTLVParserEndOfData)
                    {
                        return result;
                    }

                    gInfoMsg(*pMessageLogSettings, "ParseTLVs")
                        << "Returned from recursion." 
                        << endl;
                }
                break;

            case kTLV_OctetBuffer:
                // Make sure the target buffer is large enough to hold the bytes.
                if (pTLVParseTable[index].uTLV_pOctetBuffer->BufferSize() >= lengthField)
                {
                    // Empty the target buffer.
                    pTLVParseTable[index].uTLV_pOctetBuffer->Empty();

                    // Copy a subsection of the source buffer to the target buffer.
                    pTLVParseTable[index].uTLV_pOctetBuffer->Append(buffer, offset, lengthField);
                    offset += lengthField;

                    LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                                   pMessageLogSettings );

                    gInfoMsgNoFields(*pMessageLogSettings) 
                        << *pTLVParseTable[index].uTLV_pOctetBuffer;
                }
                else
                {
                    gErrorMsg(*pMessageLogSettings, "ParseTLVs")
                        << " -> The target octet buffer is too small to hold the value!\n";

                    return kTLVParserErrorSmallOctetBuffer;
                }
                break;

            case kTLV_FixedByteArray:
                {
                    BcmOctetBuffer myOctetBuffer(0);

                    myOctetBuffer.Overlay(buffer, offset, lengthField);
                    
                    // Copy the specified number of bytes to the dest var.
                    buffer.Read(offset,
                                (uint8 *) pTLVParseTable[index].uTLV_pUint8Value,
                                lengthField);

                    LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                                   pMessageLogSettings );

                    gInfoMsgNoFields(*pMessageLogSettings) 
                        << myOctetBuffer;
                }
                break;


            case kTLV_IpAddress:
                {
                    // Copy a subsection of the source buffer to the target buffer.
                    pTLVParseTable[index].uTLV_pIpAddressValue->ReadFrom(buffer, offset);

                    LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                                   pMessageLogSettings );

                    gInfoMsgNoFields(*pMessageLogSettings) 
                        << *pTLVParseTable[index].uTLV_pIpAddressValue << endl;
                }
                break;

            case kTLV_MacAddress:
                {
                    // Copy a subsection of the source buffer to the target buffer.
                    pTLVParseTable[index].uTLV_pMacAddressValue->ReadFrom(buffer, offset);

                    LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                                   pMessageLogSettings );

                    gInfoMsgNoFields(*pMessageLogSettings) 
                        << *pTLVParseTable[index].uTLV_pMacAddressValue << endl;
                }
                break;

            case kTLV_end_of_data:
            default:
                {
                    BcmOctetBuffer myOctetBuffer(0);

                    myOctetBuffer.Overlay(buffer, offset, lengthField);
                
                    LogTypeLength( typeField, (char *) "Unknown type. Ignore!", (uint16) lengthField,
                                   pMessageLogSettings );

                    gInfoMsgNoFields(*pMessageLogSettings) 
                        << myOctetBuffer;

                    // Increment the offset past this value.
                    offset += lengthField;
                }
                break;
        }
    }

    // If there is leftover data in the buffer then there was a problem that
    // was otherwise undetected.  The TLV data is always set up so that exactly
    // the correct number of bytes are produced and consumed, so there should
    // not be any data left over.
    if (offset != buffer.BytesUsed())
    {
        BcmOctetBuffer myOctetBuffer(0);

        myOctetBuffer.Overlay(buffer, offset, buffer.BytesUsed() - offset);

        gErrorMsg(*pMessageLogSettings, "ParseTLVs")
            << "Partial TLV left in the buffer!  Other TLVs may be invalid." 
            << myOctetBuffer << endl;

        return kTLVParserErrorDataLeftOver;
    }

    return kTLVParserEndOfData;
}



static void LogTypeLength(byte TypeField, char* TypeName, uint16 LengthField,
                          BcmMessageLogSettings *pMessageLogSettings,
                          bool LeadingNewline)
{
    if( pMessageLogSettings )
    {
        if( LeadingNewline )
        {
            gInfoMsg(*pMessageLogSettings, "ParseTLVs") << endl;
        }
        gInfoMsg(*pMessageLogSettings, "ParseTLVs") 
            << setfill( ' ' ) << setw( 24 )
            << TypeName
            << setfill( '0' )
            << ": T=0x" << hex << setw( 2 ) << uint16( TypeField )
                << " (" << dec << setw( 3 ) << uint16( TypeField ) << hex
            << ")  L=0x" << setw( 2 ) << LengthField
                << " (" << dec << setw( 3 ) << LengthField << ")  V=";
    }
}



// This function scans the specified TLV table, looking for the specified type
// code.  If found, the function then sets the 2 pointer members (the value
// pointer and the was specified bool pointer) to the addresses specified.
//
// Note that either (or both) of the pointers can be NULL if you aren't
// interested in storing the results.
//
// This is a helpful function that eliminates the need to keep track of the
// index of a given member in the table, and is necessary because the compiler
// doesn't allow you to initialize the pointers when you declare the tables.
//
// Parameters:
//      pTLVTable - pointer to the table to scan.
//      type - the type code to look for.
//      pValue - the address to be placed in the union pointer set.
//      pValueWasSpecified - the address to be placed in the was-specified bool
//                           pointer member.
//
// Returns:
//      true if the type code was found and the addresses were stored.
//      false if the table entry wasn't found.
//
bool SetTLVTableEntryAddresses(TLVTable *pTLVTable, uint8 type, void *pValue,
                               bool *pValueWasSpecified)
{
    // Make sure the table isn't NULL.
    if (pTLVTable != NULL)
    {
        int index = 0;

        // Search until I reach the end-of-table marker.
        while (pTLVTable[index].type != kEndOfDataMarker)
        {
            // If this entry matches, then store the pointers and exit.
            if (pTLVTable[index].type == type)
            {
                pTLVTable[index].uTLV_pUnknown = pValue;
                pTLVTable[index].pValueWasSpecified = pValueWasSpecified;

                gInfoMsg(*(MessageLogSettings()), "SetTLVTableEntryAddresses")
                    << "Stored address " << pValue << " at table index " 
                    << index << endl;

                return true;
            }

            // Go to the next one.
            index++;
        }
    }
    else
    {
        // Log this to the debugger...
        gErrorMsg(*(MessageLogSettings()), "SetTLVTableEntryAddresses")
            << "Pointer to the TLV table is NULL!" << endl;
    }

    // Log this to the debugger...
    gErrorMsg(*(MessageLogSettings()), "SetTLVTableEntryAddresses")
        << "The specified type code (" << (unsigned int) type 
        << ") wasn't found in the table!" << endl;

    // If we get here, then the type code wasn't found in the table.
    return false;
}


// This function scans the specified TLV table, looking for the specified type
// code.  If found, it returns the contents of the wasFound member.
//
// This is a helpful function that eliminates the need to keep track of the
// index of a given member in the table.
//
// Parameters:
//      pTLVTable - pointer to the table to scan.
//      type - the type code to look for.
//
// Returns:
//      true if the wasFound member of the table entry was set.
//      false if the table entry wasn't found, or if the wasFound member was
//          false.
//
bool GetTLVTableWasFound(TLVTable *pTLVTable, uint8 type)
{
    // Make sure the table isn't NULL.
    if (pTLVTable != NULL)
    {
        int index = 0;

        // Search until I reach the end-of-table marker.
        while (pTLVTable[index].type != kEndOfDataMarker)
        {
            // If this entry matches, then return wasFound.
            if (pTLVTable[index].type == type)
            {
                return pTLVTable[index].wasFound;
            }

            // Go to the next one.
            index++;
        }
    }
    else
    {
        // Log this to the debugger...
        gErrorMsg(*(MessageLogSettings()), "GetTLVTableWasFound")
            << "Pointer to the TLV table is NULL!" << endl;
    }

    // Log this to the debugger...
    gErrorMsg(*(MessageLogSettings()), "GetTLVTableWasFound")
        << "The specified type code (" << (unsigned int) type 
        << ") wasn't found in the table!" << endl;

    // If we get here, then the type code wasn't found in the table.
    return false;
}


#ifdef TEST_TLVS
// This is the generic prototype for the set of functions that can convert a
// value (along with its associated DOCSIS/SNMP/BPI/etc type code) to binary
// TLV format in an octet array.
//
// In the case of (u)int16 and (u)int32, these values are assumed to be in host
// byte order, and will be converted to network byte order (if necessary for the
// current processor) before they are stored.  The BcmMacAddress and
// BcmIpAddress types also convert themselves to network order.  Other data
// types are not converted.
//
// The generic function takes a pointer to some value, as well as the number of
// bytes that are in the value.  We can only hope that you have given us the
// correct number of bytes; there's nothing we can do to validate it.  We do
// check to make sure that the value pointer isn't NULL.
//
// Following this function are a number of overloaded versions of this function
// for various data types.  They generally just do a bit of pre-processing and
// then vector to this function.
//
// There are 3 parameters (2 parameters in the other overloaded functions) that
// control message logging.  If you provide a pointer to a BcmMessageLogSettings
// object that has informational messages enabled, then these functions will
// display detailed information about the TLVs as they are generated.  Along
// with the settings instance, you can provide a text name of the TLV, and
// a type hint for the value, which are used as part of the message log display.
//
// If you don't provide a BcmMessageLogSettings instance, then these functions
// will only display error and warning messages, and will use a default settings
// instance associated with this module.
//
// Note that there is now an option that allows you to specify whether the
// Length field of the TLV should be treated as a uint8 or a uint16.  Some
// settings (primarily BPI in DOCSIS) require a 2-byte length field.
//
// Parameters:
//      buffer - reference to the OctetBuffer where the TLV bytes should be
//               placed.
//      typeCode - the DOCSIS-define type code for the T portion of the TLV.
//      pValue - pointer to the value to be stored.
//      numberOfBytes - the number of valid bytes in pValue.
//      pMessageLogSettings - pointer to the message log settings instance to
//                            be used when outputting message log info.  If NULL,
//                            then it will use its own instance.
//      pTlvName - pointer to an optional string containing the name of the TLV;
//                 this is only used for message logging purposes.
//      valueType - also for message logging; a hint as to what the real type of
//                  the value is.  I use this to display "String", "MacAddress", etc.
//      lengthIsUint16 - tells me how many bytes are to be used for the Length
//                       field of the TLV.  If false, this will be the normal
//                       uint8 representation; if true, it will be a uint16.
//      lengthIncludesTypeAndLength - length field includes the type and length itself
//                       in its value.
//
// Returns:
//      The number of bytes that were consumed (including the 2 bytes for the
//      T and L parts of the TLV).  If there was an error, then it returns 0.
//
unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode,
                         const void *pValue, unsigned int numberOfBytes, 
                         BcmMessageLogSettings *pMessageLogSettings,
                         const char *pTlvName,
                         TLV_ValueEncoding valueType,
                         bool lengthIsUint16,
                         bool lengthIncludesTypeAndLength)
{
    unsigned int bytesNeeded, maximumAllowed, valueOffset;

    // If the user didn't supply a message log settings instance, then use my
    // own.
    if (pMessageLogSettings == NULL)
    {
        pMessageLogSettings = MessageLogSettings();
    }

    // Validate parameters.
    if (pValue == NULL)
    {
        // Log this to the debugger.
        gErrorMsg(*pMessageLogSettings, "GenerateTLV")
            << "Pointer to the value is NULL!" << endl;

        return 0;
    }

    // Calculate the number of bytes needed.  This is 2 bytes (for the T and L
    // fields) plus the length of the byte array.
    bytesNeeded = 2 + numberOfBytes;

    if (lengthIsUint16 == true)
    {
        maximumAllowed = 0xffff;
    }
    else
    {
        maximumAllowed = 0xff;
    }

    // See if there's enough room.  Make sure the string length doesn't exceed
    // the maximum for the length field.
    if (bytesNeeded > buffer.BytesAvailable())
    {
        // Log this to the debugger.
        gErrorMsg(*pMessageLogSettings, "GenerateTLV")
            << "Not enough bytes in octet buffer!  " << buffer.BytesAvailable()
            << " bytes available; " << bytesNeeded << " bytes needed." << endl;

        return 0;
    }
    if (numberOfBytes > maximumAllowed)
    {
        gErrorMsg(*pMessageLogSettings, "GenerateTLV")
            << "The length of the value (" << numberOfBytes 
            << " bytes) exceeds the maximum allowed (" << maximumAllowed 
            << ")!" << endl;

        return 0;
    }

    // If we get here, then we need to store the TLV values.  Do the length as
    // one or two bytes depending on the user request.
    buffer.NetworkAddToEnd(typeCode);

    uint16 lengthField = numberOfBytes;
    if(lengthIncludesTypeAndLength)
    {   
        // add 2 for type and 1 byte length
        lengthField += 2;

        if(lengthIsUint16)
        {
            // increment extra time for 2 byte length
            lengthField ++;
        }
    }

    if (lengthIsUint16 == true)
    {
        buffer.NetworkAddToEnd((uint16) lengthField);
    }
    else
    {
        buffer.NetworkAddToEnd((uint8) lengthField);
    }

    // Store the current offset into the buffer for future reference.
    valueOffset = buffer.BytesUsed();

    buffer.AddToEnd((uint8 *) pValue, numberOfBytes);

    // Display some debug info, if enabled.
    if (pMessageLogSettings->GetEnabledSeverities() & BcmMessageLogSettings::kInformationalMessages)
    {
        char valueString[32];

        // Do some error checking first.
        if (pTlvName == NULL)
        {
            pTlvName = "no name";
        }
        if (valueType > kTLV_end_of_data)
        {
            valueType = kTLV_end_of_data;
        }

        // Set up the known values, and pre-set the fill character to '0' so
        // that hex values will be displayed correctly.
//      gInfoMsg(*pMessageLogSettings, "GenerateTLV")
//          << "\n\t"
//          << gTlvTypeName[valueType] << " TLV ("
//          << pTlvName << "):  T=" << (unsigned int) typeCode << "; L=" << numberOfBytes
//          << "; V=";
        gInfoMsg( *pMessageLogSettings, "GenerateTLV" )
            << "\n"
            << pTlvName << ": T=0x" << hex << setw( 2 ) << uint16( typeCode )
                << " (" << dec << setw( 3 ) << uint16( typeCode ) << hex
            << ")  L=0x" << setw( 2 ) << uint16( numberOfBytes )
                << " (" << dec << setw( 3 ) << uint16( numberOfBytes )
            << ")  V=";

        // The value gets displayed differently depending on the type.
        switch (valueType)
        {
            case kTLV_uint8:
                {
                    uint8 value;

                    buffer.NetworkRead(valueOffset, value);

                    sprintf(valueString, "0x%02x (%u)", value, value);
                    
                    gInfoMsgNoFields(*pMessageLogSettings) << valueString << endl;
                }
                break;

            case kTLV_int8:
                {
                    int8 value;

                    buffer.NetworkRead(valueOffset, value);

                    sprintf(valueString, "0x%02x (%d)", value, value);
                    
                    gInfoMsgNoFields(*pMessageLogSettings) << valueString << endl;
                }
                break;

            case kTLV_uint16:
                {
                    uint16 value;

                    buffer.NetworkRead(valueOffset, value);

                    sprintf(valueString, "0x%04x (%u)", value, value);
                    
                    gInfoMsgNoFields(*pMessageLogSettings) << valueString << endl;
                }
                break;

            case kTLV_int16:
                {
                    int16 value;

                    buffer.NetworkRead(valueOffset, value);

                    sprintf(valueString, "0x%04x (%d)", value, value);
                    
                    gInfoMsgNoFields(*pMessageLogSettings) << valueString << endl;
                }
                break;

            case kTLV_uint32:
                {
                    uint32 value;

                    buffer.NetworkRead(valueOffset, value);

                    sprintf(valueString, "0x%08x (%u)", value, value);
                    
                    gInfoMsgNoFields(*pMessageLogSettings) << valueString << endl;
                }
                break;

            case kTLV_int32:
                {
                    int32 value;

                    buffer.NetworkRead(valueOffset, value);

                    sprintf(valueString, "0x%08x (%d)", value, value);
                    
                    gInfoMsgNoFields(*pMessageLogSettings) << valueString << endl;
                }
                break;

            case kTLV_string:
                gInfoMsgNoFields(*pMessageLogSettings)
                    << "'" << (char *) pValue << "'" << endl;
                break;

            case kTLV_OctetBuffer:
            case kTLV_FixedByteArray:
                {
                    BcmOctetBuffer myOctetBuffer(0);

                    myOctetBuffer.Overlay(buffer, valueOffset, numberOfBytes);
                    
                    gInfoMsgNoFields(*pMessageLogSettings)
                        << "array of octets.\n" << myOctetBuffer << endl;
                }
                break;

            case kTLV_nested:
                {
                    BcmOctetBuffer myOctetBuffer(0);

                    myOctetBuffer.Overlay(buffer, valueOffset, numberOfBytes);

                    // Should I try to display this?
                    gInfoMsgNoFields(*pMessageLogSettings)
                        << "nested TLVs.\n" << myOctetBuffer << endl;
                }
                break;

            case kTLV_IpAddress:
                {
                    BcmIpAddress ipAddress;

                    ipAddress.ReadFrom(buffer, valueOffset);

                    gInfoMsgNoFields(*pMessageLogSettings) << ipAddress << endl;
                }
                break;

            case kTLV_MacAddress:
                {
                    BcmMacAddress macAddress;

                    macAddress.ReadFrom(buffer, valueOffset);
                
                    gInfoMsgNoFields(*pMessageLogSettings) << macAddress << endl;
                }
                break;

            default:
                gInfoMsgNoFields(*pMessageLogSettings) << "unknown!";
                break;
        }
    }

    // Return the number of bytes consumed.
    return bytesNeeded;
}


// This one handles a string value.  The length of the string is calculated for
// you.
unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                         const char *pStringValue, 
                         BcmMessageLogSettings *pMessageLogSettings,
                         const char *pTlvName,
                         bool lengthIsUint16,
                         bool lengthIncludesTypeAndLength)
{
    unsigned int stringLength;

    // If the user didn't supply a message log settings instance, then use my
    // own.
    if (pMessageLogSettings == NULL)
    {
        pMessageLogSettings = MessageLogSettings();
    }

    // Validate parameters.
    if (pStringValue == NULL)
    {
        // Log this to the debugger.
        gErrorMsg(*pMessageLogSettings, "GenerateTLV")
            << "Pointer to the string value is NULL!" << endl;

        return 0;
    }

    // UNFINISHED - Note, there was talk at one time about removing the
    // requirement to include the \0 at the end of the string.  If this ever
    // comes through, we will need to remove the "stringLength++;" line.
    //
    // Calculate the length of the string, including terminating \0.
    stringLength = strlen(pStringValue);
    stringLength++;

    // Now vector to the method that does the real work.
    return GenerateTLV(buffer, typeCode, pStringValue, stringLength, pMessageLogSettings, pTlvName, kTLV_string, lengthIsUint16, lengthIncludesTypeAndLength);
}


// This function tests all of the permutations of TLV generation and parsing.
// It should only be called for testing, after changes are made to the TLV
// functions, in order to verify that the code still works.
//
// Parameters:  None.
//
// Returns:
//      true if the tests worked.
//      false if there was a problem.  The problem will be noted in the message
//          logs.
//
bool TestTLVs(void)
{
    BcmMessageLogSettings messageLogSettings("TLV Tester");
    bool lengthIs2Bytes;

    // Enable full debugging.
    messageLogSettings.SetEnabledFields(0);
    messageLogSettings.SetEnabledSeverities(BcmMessageLogSettings::kAllMessages);

	gInfoMsg(messageLogSettings, "TestTLVs") << "Beginning tests of the TLV functions...\n" << endl;

    uint8 uint8Value = 8;
    uint16 uint16Value = 16;
    uint32 uint32Value = 32;
    int8 int8Value = -8;
    int16 int16Value = -16;
    int32 int32Value = -32;

    char string[32] = "This is a string";

    BcmOctetBuffer octetBuffer(32);
    
    for (uint16 x = 0; x < 16; x++)
    {
        octetBuffer.NetworkAddToEnd(x);
    }

    BcmOctetBuffer stringOctetBuffer(32);

    for (uint8 x2 = 0; x2 < 16; x2++)
    {
        stringOctetBuffer.NetworkAddToEnd((uint8) ('a' + x2));
    }

    uint8 fixedByteArray[16] =
    {
        0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 
        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
    };

    BcmIpAddress ipAddress(11, 24, 4, 101);
    BcmMacAddress macAddress(0x00, 0x10, 0x18, 0x00, 0x10, 0x36);

    BcmOctetBuffer tlvBuffer(255);
    BcmOctetBuffer parentTlvBuffer(255);
    unsigned int offset;

    // Now parse each value out of the tlv buffer.
    TLVTable tlvTable[] =
    {
        {0, sizeof(uint8Value),     kTLV_uint8,             &uint8Value,        false, false, "uint8Value", NULL},
        {1, sizeof(uint16Value),    kTLV_uint16,            &uint16Value,       false, false, "uint16Value", NULL},
        {2, sizeof(uint32Value),    kTLV_uint32,            &uint32Value,       false, false, "uint32Value", NULL},
        {3, sizeof(int8Value),      kTLV_int8,              &int8Value,         false, false, "int8Value", NULL},
        {4, sizeof(int16Value),     kTLV_int16,             &int16Value,        false, false, "int16Value", NULL},
        {5, sizeof(int32Value),     kTLV_int32,             &int32Value,        false, false, "int32Value", NULL},
        {6, kUnknownLength,         kTLV_string,            string,             false, false, "stringValue", NULL},
        {7, kUnknownLength,         kTLV_OctetBuffer,       &octetBuffer,       false, false, "octetBuffer", NULL},
        {8, sizeof(fixedByteArray), kTLV_FixedByteArray,    fixedByteArray,     false, false, "fixedByteArray", NULL},
        {9, BcmIpAddress::ADDR_LEN, kTLV_IpAddress,         &ipAddress,         false, false, "ipAddress", NULL},
        {10, 6,                     kTLV_MacAddress,        &macAddress,        false, false, "macAddress", NULL},
        {11, kUnknownLength,        kTLV_StringOctetBuffer, &stringOctetBuffer, false, false, "stringOctetBuffer", NULL},

        // This must come last.  It tells the parser when to stop.
        kEndOfTLVTable
    };

    TLVTable parentTable[] =
    {
        {12, kUnknownLength, kTLV_nested, tlvTable, false, false, "ParentTlv", NULL},

        // This must come last.  It tells the parser when to stop.
        kEndOfTLVTable
    };

    lengthIs2Bytes = false;
    while (1)
    {
        tlvBuffer.Empty();
        parentTlvBuffer.Empty();

        if (lengthIs2Bytes == false)
        {
            gInfoMsg(messageLogSettings, "TestTLVs")
                << "Testing the TLV functions for single-byte length field:\n" << endl;
        }
        else
        {
            gInfoMsg(messageLogSettings, "TestTLVs")
                << "Testing the TLV functions for 2-byte length field:\n" << endl;
        }

        // Put each value into the tlv buffer one at a time.
        if ((GenerateTLV(tlvBuffer, 0, uint8Value, &messageLogSettings, "uint8Value", lengthIs2Bytes) == 0) ||
            (GenerateTLV(tlvBuffer, 1, uint16Value, &messageLogSettings, "uint16Value", lengthIs2Bytes) == 0) ||
            (GenerateTLV(tlvBuffer, 2, uint32Value, &messageLogSettings, "uint32Value", lengthIs2Bytes) == 0) ||
            (GenerateTLV(tlvBuffer, 3, int8Value, &messageLogSettings, "int8Value", lengthIs2Bytes) == 0) ||
            (GenerateTLV(tlvBuffer, 4, int16Value, &messageLogSettings, "int16Value", lengthIs2Bytes) == 0) ||
            (GenerateTLV(tlvBuffer, 5, int32Value, &messageLogSettings, "int32Value", lengthIs2Bytes) == 0) ||
            (GenerateTLV(tlvBuffer, 6, string, &messageLogSettings,  "stringValue", lengthIs2Bytes) == 0) ||
            (GenerateTLV(tlvBuffer, 7, octetBuffer, &messageLogSettings, "octetBuffer", lengthIs2Bytes) == 0) ||
            (GenerateTLV(tlvBuffer, 8, fixedByteArray, sizeof(fixedByteArray), &messageLogSettings, "fixedByteArray", kTLV_FixedByteArray, lengthIs2Bytes) == 0) ||
            (GenerateTLV(tlvBuffer, 9, ipAddress, &messageLogSettings, "ipAddress", lengthIs2Bytes) == 0) ||
            (GenerateTLV(tlvBuffer, 10, macAddress, &messageLogSettings, "macAddress", lengthIs2Bytes) == 0) ||
            (GenerateTLV(tlvBuffer, 11, stringOctetBuffer, &messageLogSettings, "stringOctetBuffer", lengthIs2Bytes) == 0))
        {
            gErrorMsg(messageLogSettings, "TestTLVs")
                << "Call to GenerateTLV failed!" << endl;

            return false;
        }

        GenerateTLV(parentTlvBuffer, 12, tlvBuffer, &messageLogSettings, "parentOfNested", lengthIs2Bytes);

        gInfoMsg(messageLogSettings, "TestTLVs")
            << "Here's the final TLV buffer after adding everything to it:\n"
            << parentTlvBuffer << endl;

        offset = 0;
        if (ParseTLVs(parentTlvBuffer, offset, parentTable, &messageLogSettings, lengthIs2Bytes) != kTLVParserEndOfData)
        {
            gErrorMsg(messageLogSettings, "TestTLVs")
                << "Call to ParseTLVs failed!" << endl;

            return false;
        }

        gInfoMsgNoFields(messageLogSettings) << endl;

        // If we've tried byte and word sized length fields, then break out.
        if (lengthIs2Bytes == true)
        {
            break;
        }

        // Otherwise, next time around, we need to try 2-byte length fields.
        lengthIs2Bytes = true;
    }

    gInfoMsg(messageLogSettings, "TestTLVs")
        << "All tests worked!" << endl;

	return true;
}
#endif


// This is the singleton accessor for this object.  Hopefully, it allows
// creation to be deferred until non-vol settings are read in so that we can
// get the correct defaults.
//
// Parameters:  None.
//
// Returns:
//      A reference to the singleton instance.
//
static BcmMessageLogSettings *MessageLogSettings(void)
{
    static BcmMessageLogSettings gMessageLogSettings("TLV");

    return &gMessageLogSettings;
}


