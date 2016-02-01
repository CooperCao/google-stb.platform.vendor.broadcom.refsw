//****************************************************************************
//
// Copyright (c) 1999-2010 Broadcom Corporation
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
//  Filename:       TLV.cpp
//  Author:         David Pullen
//  Creation Date:  April 28, 1999
//
//****************************************************************************

// Set this to 1 to enable the regression test code.
#define REGRESSION_TEST 0

//********************** Include Files ***************************************

// My api and definitions...
#include "TLV.h"

// For strlen() and dealing with C strings.
#include <string.h>
#include <stdio.h>

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iomanip>
#include <iostream>
#else
#include <iomanip.h>
#include <iostream.h>
#endif

#include "IpAddress.h"
#include "MacAddress.h"

#include "MessageLog.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************

static BcmMessageLogSettings *MessageLogSettings(void);
static void LogTypeLength(uint32 TypeField, const char* TypeName, uint32 LengthField,
                          BcmMessageLogSettings *pMessageLogSettings,
                          bool LeadingNewline = false);

//********************** Class Method Implementations ************************


unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                         const void *pValue, uint32 numberOfBytes, 
                         const TlvConfig &config,
                         const char *pTlvName,
                         TLV_ValueEncoding valueType)
{
    unsigned int bytesNeeded, maximumAllowed, valueOffset;
    unsigned int lengthField;

    BcmMessageLogSettings *pMessageLogSettings = config.pfMessageLogSettings;

    // If the user didn't supply a message log settings instance, then use my
    // own.
    if (pMessageLogSettings == NULL)
    {
        pMessageLogSettings = MessageLogSettings();
    }

    // Validate parameters.
    if (pValue == NULL)
    {
        // A NULL pointer is not valid if the number of bytes is greater than 0.
        // If the number of bytes is 0, we just need to set a dummy pointer
        // value so that the octet buffer is happy.
        if (numberOfBytes > 0)
        {
            // Log this to the debugger.
            gErrorMsg(*pMessageLogSettings, "GenerateTLV")
                << "Pointer to the value is NULL!" << endl;

            return 0;
        }
        else
        {
            pValue = &lengthField;
        }
    }

    // UNFINISHED - should I check config.fLength0Valid and return an error
    // if numberOfBytes is 0?

    // Calculate the number of bytes needed.  This is the size of the T and L
    // fields plus the length of the byte array.
    bytesNeeded = config.fTlvTypeSize + config.fTlvLengthSize + numberOfBytes;

    // Calculate the maximum number of value bytes that can be represented with
    // the length field.
    switch (config.fTlvLengthSize)
    {
        case kTlvLengthIs8Bits:
            maximumAllowed = 0xff;
            break;

        case kTlvLengthIs16Bits:
            maximumAllowed = 0xffff;
            break;

        case kTlvLengthIs32Bits:
        default:
            maximumAllowed = 0xffffffff;
            break;
    }

    // If the length field includes the T and L, then we can't have as many
    // data value bytes.
    if (config.fTlvLengthAttribute == kTlvLengthIncludesTL)
    {
        maximumAllowed -= config.fTlvTypeSize + config.fTlvLengthSize;
    }

    // See if there's enough room.  Make sure the value length doesn't exceed
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

    // If we get here, then we need to store the TLV values.  Store the
    // T and L using an appropriate number of bytes based on the config.
    switch (config.fTlvTypeSize)
    {
        case kTlvTypeIs8Bits:
            buffer.NetworkAddToEnd((uint8) typeCode);
            break;

        case kTlvTypeIs16Bits:
            buffer.NetworkAddToEnd((uint16) typeCode);
            break;

        case kTlvTypeIs32Bits:
        default:
            buffer.NetworkAddToEnd((uint32) typeCode);
            break;
    }

    // Calculate the length value to be stored.  This may include the size of
    // the T and L fields.
    lengthField = numberOfBytes;
    if (config.fTlvLengthAttribute == kTlvLengthIncludesTL)
    {
        lengthField += config.fTlvTypeSize + config.fTlvLengthSize;
    }

    switch (config.fTlvLengthSize)
    {
        case kTlvLengthIs8Bits:
            buffer.NetworkAddToEnd((uint8) lengthField);
            break;

        case kTlvLengthIs16Bits:
            buffer.NetworkAddToEnd((uint16) lengthField);
            break;

        case kTlvLengthIs32Bits:
        default:
            buffer.NetworkAddToEnd((uint32) lengthField);
            break;
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
        gInfoMsg( *pMessageLogSettings, "GenerateTLV" )
            << "\n"
            << pTlvName << ": T=0x" << hex << typeCode
                << " (" << dec << typeCode << hex
            << ")  L=0x" << numberOfBytes
                << " (" << dec << numberOfBytes
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

                    sprintf(valueString, "0x%04x (%d)", (uint16) value, value);
                    
                    gInfoMsgNoFields(*pMessageLogSettings) << valueString << endl;
                }
                break;

            case kTLV_uint32:
                {
                    uint32 value;

                    buffer.NetworkRead(valueOffset, value);

                    sprintf(valueString, "0x%08lx (%lu)", value, value);
                    
                    gInfoMsgNoFields(*pMessageLogSettings) << valueString << endl;
                }
                break;

            case kTLV_int32:
                {
                    int32 value;

                    buffer.NetworkRead(valueOffset, value);

                    sprintf(valueString, "0x%08lx (%ld)", (uint32) value, value);
                    
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

            #if (BFC_INCLUDE_DOCSIS_SUPPORT)
            
            case kTLV_CMIM:
                {
                    BcmCmim cmim;

                    cmim.ReadFrom(buffer, valueOffset, lengthField);

                    gInfoMsgNoFields(*pMessageLogSettings) << cmim << endl;
                }
                break;

            case kTLV_DOCS3_DSID:
                {
                    BcmDsid dsid;

                    dsid.ReadFrom(buffer, valueOffset);

                    gInfoMsgNoFields(*pMessageLogSettings) << dsid << endl;
                }
                break;

            #endif
                
            default:
                gInfoMsgNoFields(*pMessageLogSettings) << "unknown!";
                break;
        }
    }

    // Return the number of bytes consumed.
    return bytesNeeded;
}


unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                         const char *pStringValue, 
                         const TlvConfig &config,
                         const char *pTlvName)
{
    unsigned int stringLength;

    BcmMessageLogSettings *pMessageLogSettings = config.pfMessageLogSettings;

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

    // Calculate the length of the string.
    stringLength = strlen(pStringValue);

    // Include the terminating \0 if requested by the client.
    if (config.fTlvString == kTlvStringIncludes0)
    {
        stringLength++;
    }

    // Now vector to the method that does the real work.
    return GenerateTLV(buffer, typeCode, pStringValue, stringLength, config, pTlvName, kTLV_string);
}


TLVParserResult ParseTLVs(const BcmOctetBuffer &buffer, unsigned int &offset,
                          TLVTable *pTLVParseTable, const TlvConfig &config)
{
    uint8 typeField8, lengthField8;
    uint16 typeField16, lengthField16;
    uint32 typeField, lengthField;
    int index;
    unsigned int sizeOfType, sizeOfLength;

    BcmMessageLogSettings *pMessageLogSettings = config.pfMessageLogSettings;

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

    // Set the expected size of the type and length fields based on the user's
    // config.
    sizeOfType = config.fTlvTypeSize;
    sizeOfLength = config.fTlvLengthSize;

    // Keep processing TLVs until we run out of buffer space, or we run across
    // a condition that keeps us from proceeding.  Note that the test below
    // causes us to break out if there aren't enough bytes of valid data left
    // in the buffer; there MUST be at least enough for the T and L fields for
    // this to succeed at all.
    while (offset + sizeOfType + sizeOfLength <= buffer.BytesUsed())
    {
        // Get the type and length fields out of the buffer.  Since we
        // pre-conditioned the loop test (above) so that there will always be
        // at least 2 bytes available, we don't need to do any validation on
        // the calls to NetworkRead().
        //
        // Read the appropriate number of bytes from the buffer, then move the
        // value to the uint32 version so that the code below doesn't
        // have to care what it is.
        switch (config.fTlvTypeSize)
        {
            case kTlvTypeIs8Bits:
                buffer.NetworkRead(offset, typeField8);
                typeField = typeField8;
                break;

            case kTlvTypeIs16Bits:
                buffer.NetworkRead(offset, typeField16);
                typeField = typeField16;
                break;

            case kTlvTypeIs32Bits:
            default:
                buffer.NetworkRead(offset, typeField);
                break;
        }

        switch (config.fTlvLengthSize)
        {
            case kTlvLengthIs8Bits:
                buffer.NetworkRead(offset, lengthField8);
                lengthField = lengthField8;
                break;

            case kTlvLengthIs16Bits:
                buffer.NetworkRead(offset, lengthField16);
                lengthField = lengthField16;
                break;

            case kTlvLengthIs32Bits:
            default:
                buffer.NetworkRead(offset, lengthField);
                break;
        }

        // Adjust the length value that was read by the length multiplier.
        lengthField *= config.fLengthMultiplier;

        // If the config says that all TLV length fields MUST be greater than
        // zero, then check for this.  Does this mean that we should call this
        // an error, or should we just ignore it and continue?  For now, I'll
        // just flag this as a warning and continue.
        if ((config.fLength0Valid == kTlvLength0Illegal) &&
            (lengthField == 0))
        {
            gWarningMsg(*pMessageLogSettings, "ParseTLVs")
                << "TLV length field is 0!  This is a spec violation!  Continuing anyway..." << endl;
        }

        // Adjust length if the length includes the type and length itself.
        // We want the comparisons that follow to be based on the length of
        // the value only.
        if (config.fTlvLengthAttribute == kTlvLengthIncludesTL)
        {
            // Error check for length field that is too small for this case.
            if (lengthField < (sizeOfType + sizeOfLength))
            {
                gErrorMsg(*pMessageLogSettings, "ParseTLVs")
                    << "Length field must be at least "
                    << (sizeOfType + sizeOfLength) << endl;

                return kTLVParserErrorInvalidLength;
            }

            lengthField -= (sizeOfType + sizeOfLength);
        }

        

        // Look for the type field in our table.  Break out if we find either
        // the type we are looking for, or if we reach the end of the table.
        index = 0;
        while ((pTLVParseTable[index].type != kEndOfDataMarker) &&
               (pTLVParseTable[index].type != typeField))
        {
            index++;
        }

        // See if the user wants us to break out of the parsing engine; this
        // generally happens for nested types where they need to create a new
        // instance of a struct or class before continuing with the parsing.
        if (pTLVParseTable[index].stopParsing)
        {
            LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                           pMessageLogSettings, true );

            gNoSlimInfoMsg(*pMessageLogSettings, "ParseTLVs") << endl;

            // Back the offset up so that it will refer to the T and L codes
            // again.  The caller will likely need these in order to know what
            // to do next.
            offset -= sizeOfType + sizeOfLength;

            return kTLVParserStopParsingFound;
        }

        // If we know how long the value should be, then verify that the length
        // specified matches.  For some TLVs (uint8, etc.) the length is known
        // in advance.
        if (pTLVParseTable[index].length != kUnknownLength)
        {
            // Special case for IP addresses; we support both IPv4 (4 bytes)
            // and IPv6 (16 bytes) if the right table values are used.
            if ((pTLVParseTable[index].valueEncoding == kTLV_IpAddress) &&
                (pTLVParseTable[index].length == kIpv4OrIpv6Length))
            {
                if ((lengthField != BcmIpV4Address::ADDR_LEN) &&
                    (lengthField != BcmIpV6Address::ADDR_LEN))
                {
                    gErrorMsg(*pMessageLogSettings, "ParseTLVs")
                        << " -> IP Address TLV " << typeField << " has a length of "
                        << lengthField << "; expected length of "
                        << BcmIpV4Address::ADDR_LEN << " or "
                        << BcmIpV6Address::ADDR_LEN << "!" << endl;

                    return kTLVParserErrorInvalidLength;
                }
            }
            else if (pTLVParseTable[index].length != lengthField)
            {
                gErrorMsg(*pMessageLogSettings, "ParseTLVs")
                    << " -> TLV " << typeField << " has a length of "
                    << lengthField << "; expected length of "
                    << pTLVParseTable[index].length << '!' << endl;

                return kTLVParserErrorInvalidLength;
            }
        }

        // Make sure we have enough bytes left in the buffer for the value.
        if ((lengthField + offset) > buffer.BytesUsed())
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

                LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                               pMessageLogSettings );

                gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                    << "0x" 
                    << setfill( '0' ) << hex << setw( 2 )
                    << uint16( (*(pTLVParseTable[index].uTLV_pUint8Value)) )
                    << " (" << dec // << setw( 3 ) 
                    << uint16( (*(pTLVParseTable[index].uTLV_pUint8Value)) )
                    << ") " << setfill( ' ' )
                    << endl;
                break;

            case kTLV_int8:
                buffer.NetworkRead(offset, *(pTLVParseTable[index].uTLV_pInt8Value));

                LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                               pMessageLogSettings );

                gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                    << "0x" 
                    << setfill( '0' ) << hex << setw( 2 )
                    << int16( (*(pTLVParseTable[index].uTLV_pInt8Value)) )
                    << " (" << dec // << setw( 3 ) 
                    << int16( (*(pTLVParseTable[index].uTLV_pInt8Value)) )
                    << ") " << setfill( ' ' )
                    << endl;

                break;

            case kTLV_uint16:
                buffer.NetworkRead(offset, *(pTLVParseTable[index].uTLV_pUint16Value));

                LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                               pMessageLogSettings );

                gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                    << "0x" 
                    << setfill( '0' ) << hex << setw( 4 )
                    << uint16( (*(pTLVParseTable[index].uTLV_pUint16Value)) )
                    << " (" << dec // << setw( 5 ) 
                    << uint16( (*(pTLVParseTable[index].uTLV_pUint16Value)) )
                    << ") " << setfill( ' ' )
                    << endl;

                break;

            case kTLV_int16:
                buffer.NetworkRead(offset, *(pTLVParseTable[index].uTLV_pInt16Value));

                LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                               pMessageLogSettings );

                gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                    << "0x" 
                    << setfill( '0' ) << hex << setw( 4 )
                    << int16( (*(pTLVParseTable[index].uTLV_pInt16Value)) )
                    << " (" << dec // << setw( 5 ) 
                    << int16( (*(pTLVParseTable[index].uTLV_pInt16Value)) )
                    << ") " << setfill( ' ' )
                    << endl;

                break;

            case kTLV_uint32:
                buffer.NetworkRead(offset, *(pTLVParseTable[index].uTLV_pUint32Value));

                LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                               pMessageLogSettings );

                gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                    << "0x" 
                    << setfill( '0' ) << hex << setw( 8 )
                    << uint32( (*(pTLVParseTable[index].uTLV_pUint32Value)) )
                    << " (" << dec // << setw( 10 ) 
                    << uint32( (*(pTLVParseTable[index].uTLV_pUint32Value)) )
                    << ") " << setfill( ' ' )
                    << endl;

                break;

            case kTLV_int32:
                buffer.NetworkRead(offset, *(pTLVParseTable[index].uTLV_pInt32Value));

                LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                               pMessageLogSettings );

                gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                    << "0x" 
                    << setfill( '0' ) << hex << setw( 8 )
                    << int32( (*(pTLVParseTable[index].uTLV_pInt32Value)) )
                    << " (" << dec // << setw( 10 ) 
                    << int32( (*(pTLVParseTable[index].uTLV_pInt32Value)) )
                    << ") " << setfill( ' ' )
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

                    // Make sure there is a \0 at the end.
                    if (pString[lengthField - 1] != '\0')
                    {
                        pString[lengthField] = '\0';
                        terminatorPresent = false;
                    }

                    LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                                   pMessageLogSettings );

                    BcmOctetBuffer myOctetBuffer(0);
                    myOctetBuffer.Overlay(buffer, originalOffset, lengthField);

                    gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                        << "'" << pString << "' " << myOctetBuffer;

                    if (!terminatorPresent &&
                        (config.fTlvString == kTlvStringIncludes0))
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

                    LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                                   pMessageLogSettings, true );

                    gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                        << '\n'
                        << " V=nested TLVs; recursing." 
                        << endl;

                    nestedTLVs.Overlay(buffer, offset, lengthField);

                    // I need to recurse, giving a pointer to the nested parsing
                    // table.
                    result = ParseTLVs(nestedTLVs, nestedOffset, 
                                       pTLVParseTable[index].uTLV_pTLVTable,
                                       config);
                    offset += nestedOffset;

                    // add an extra newline to the log in order to more easily
                    // see the end of nested type.
                    gNoSlimInfoMsgNoFields(*pMessageLogSettings) << endl;

                    // If there was a  problem, then silently bail out.
                    if (result != kTLVParserEndOfData)
                    {
                        return result;
                    }

                    gNoSlimInfoMsg(*pMessageLogSettings, "ParseTLVs")
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

                    LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                                   pMessageLogSettings );

                    gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
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

                    LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                                   pMessageLogSettings );

                    gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                        << myOctetBuffer;
                }
                break;


            case kTLV_IpAddress:
                {
                    // Select the IPv4/v6 mode depending on how many bytes are
                    // present (16 = IPv6, other length, assume IPv4).
                    if (lengthField == BcmIpV6Address::ADDR_LEN)
                    {
                        pTLVParseTable[index].uTLV_pIpAddressValue->SetIpV6Mode(true);
                    }
                    else
                    {
                        pTLVParseTable[index].uTLV_pIpAddressValue->SetIpV6Mode(false);
                    }

                    pTLVParseTable[index].uTLV_pIpAddressValue->ReadFrom(buffer, offset);

                    LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                                   pMessageLogSettings );

                    gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                        << *pTLVParseTable[index].uTLV_pIpAddressValue << endl;
                }
                break;

            case kTLV_MacAddress:
                {
                    pTLVParseTable[index].uTLV_pMacAddressValue->ReadFrom(buffer, offset);

                    LogTypeLength( typeField, pTLVParseTable[index].pName, lengthField,
                                   pMessageLogSettings );

                    gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                        << *pTLVParseTable[index].uTLV_pMacAddressValue << endl;
                }
                break;

            #if (BFC_INCLUDE_DOCSIS_SUPPORT)
                
            case kTLV_DOCS3_DSID:
                {
                    // Copy a subsection of the source buffer to the target buffer.
                    pTLVParseTable[index].uTLV_pDsidValue->ReadFrom(buffer, offset);

                    LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                                   pMessageLogSettings );

                    gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                        << *pTLVParseTable[index].uTLV_pDsidValue << endl;
                }
                break;

            case kTLV_CMIM:
                {
                    // Copy a subsection of the source buffer to the target buffer.
                    pTLVParseTable[index].uTLV_pCmimValue->ReadFrom(buffer, offset, lengthField);

                    LogTypeLength( typeField, pTLVParseTable[index].pName, (uint16) lengthField,
                               pMessageLogSettings );

                    gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
                        << *pTLVParseTable[index].uTLV_pCmimValue << endl;
                }
                break;

            #endif

            case kTLV_end_of_data:
            default:
                {
                    BcmOctetBuffer myOctetBuffer(0);

                    myOctetBuffer.Overlay(buffer, offset, lengthField);
                
                    LogTypeLength( typeField, "Unknown type. Ignore!", lengthField,
                                   pMessageLogSettings );

                    gNoSlimInfoMsgNoFields(*pMessageLogSettings) 
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



void TlvConfigInit(TlvConfig &config)
{
    // Clear everything.
    memset(&config, 0, sizeof(config));

    // Set the most common (non-0) parameter values.
    config.fTlvTypeSize = kTlvTypeIs8Bits;
    config.fTlvLengthSize = kTlvLengthIs8Bits;
    config.fTlvLengthAttribute = kTlvLengthExcludesTL;
    config.fLengthMultiplier = 1;
    config.fLength0Valid = kTlvLength0Illegal;

    // The fTlvString field should default to 0 (include the \0).
}


void TlvConfigInit(TlvConfig &config,
                   BcmMessageLogSettings *pMessageLogSettings,
                   bool lengthIsUint16,
                   bool lengthIncludesTypeAndLength)
{
    // Basic config.
    TlvConfigInit(config);

    // Override specific values.
    if (lengthIsUint16)
    {
        config.fTlvLengthSize = kTlvLengthIs16Bits;
    }

    if (lengthIncludesTypeAndLength)
    {
        config.fTlvLengthAttribute = kTlvLengthIncludesTL;
    }

    config.pfMessageLogSettings = pMessageLogSettings;
}


bool SetTLVTableEntryAddresses(TLVTable *pTLVTable, uint32 type, void *pValue,
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
        << "The specified type code (" << type 
        << ") wasn't found in the table!" << endl;

    // If we get here, then the type code wasn't found in the table.
    return false;
}


bool GetTLVTableWasFound(TLVTable *pTLVTable, uint32 type)
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
        << "The specified type code (" << type 
        << ") wasn't found in the table!" << endl;

    // If we get here, then the type code wasn't found in the table.
    return false;
}


#if (REGRESSION_TEST)

// Includes and other regression stuff here...

#endif

bool RegressTlvs(void)
{
    #if (!REGRESSION_TEST)
    {
        // Regression tests compiled out, return false.
        return false;
    }
    #else
    {
        BcmMessageLogSettings messageLogSettings("TLV Regression Tests");
        bool lengthIs2Bytes;

        // Enable full debugging.
        messageLogSettings.SetEnabledFields(0);
        messageLogSettings.SetEnabledSeverities(BcmMessageLogSettings::kAllMessages);

        gInfoMsg(messageLogSettings, "RegressTlvs") << "Beginning regression tests of the TLV functions...\n" << endl;

        uint8 uint8Value = 8, uint8Parsed;
        uint16 uint16Value = 16, uint16Parsed;
        uint32 uint32Value = 32, uint32Parsed;
        int8 int8Value = -8, int8Parsed;
        int16 int16Value = -16, int16Parsed;
        int32 int32Value = -32, int32Parsed;

        char stringValue[32] = "This is a string";
        char stringParsed[32] = "";

        BcmOctetBuffer octetBuffer(32);
        BcmOctetBuffer octetBufferParsed(32);

        for (uint16 x = 0; x < 16; x++)
        {
            octetBuffer.NetworkAddToEnd(x);
        }

        BcmOctetBuffer stringOctetBuffer(32);
        BcmOctetBuffer stringOctetBufferParsed(32);

        for (uint8 x2 = 0; x2 < 16; x2++)
        {
            stringOctetBuffer.NetworkAddToEnd((uint8) ('a' + x2));
        }
        stringOctetBuffer.NetworkAddToEnd((uint8) 0);

        uint8 fixedByteArray[16] =
        {
            0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 
            0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
        };
        uint8 fixedByteArrayParsed[16] = {0};

        BcmIpAddress ipv4Address(11, 24, 4, 101);
        BcmIpAddress ipv4AddressParsed, ipv4AddressParsedEither;

        BcmIpAddress ipv6Address("fe80::0123");
        BcmIpAddress ipv6AddressParsed, ipv6AddressParsedEither;

        BcmMacAddress macAddress(0x00, 0x10, 0x18, 0x00, 0x10, 0x36);
        BcmMacAddress macAddressParsed;

        BcmOctetBuffer tlvBuffer(1024);
        BcmOctetBuffer parentTlvBuffer(1024);
        unsigned int offset;

        // For parsing each value out of the tlv buffer.
        TLVTable tlvTable[] =
        {
            {0, sizeof(uint8Parsed),            kTLV_uint8,             {&uint8Parsed},             false, false, "uint8Parsed", NULL},
            {1, sizeof(uint16Parsed),           kTLV_uint16,            {&uint16Parsed},            false, false, "uint16Parsed", NULL},
            {2, sizeof(uint32Parsed),           kTLV_uint32,            {&uint32Parsed},            false, false, "uint32Parsed", NULL},
            {3, sizeof(int8Parsed),             kTLV_int8,              {&int8Parsed},              false, false, "int8Parsed", NULL},
            {4, sizeof(int16Parsed),            kTLV_int16,             {&int16Parsed},             false, false, "int16Parsed", NULL},
            {5, sizeof(int32Parsed),            kTLV_int32,             {&int32Parsed},             false, false, "int32Parsed", NULL},
            {6, kUnknownLength,                 kTLV_string,            {stringParsed},             false, false, "stringParsed", NULL},
            {7, kUnknownLength,                 kTLV_OctetBuffer,       {&octetBufferParsed},       false, false, "octetBufferParsed", NULL},
            {8, sizeof(fixedByteArrayParsed),   kTLV_FixedByteArray,    {fixedByteArrayParsed},     false, false, "fixedByteArrayParsed", NULL},
            {9, BcmIpV4Address::ADDR_LEN,       kTLV_IpAddress,         {&ipv4AddressParsed},       false, false, "ipv4AddressParsed", NULL},
            {10, 6,                             kTLV_MacAddress,        {&macAddressParsed},        false, false, "macAddressParsed", NULL},
            {11, kUnknownLength,                kTLV_StringOctetBuffer, {&stringOctetBufferParsed}, false, false, "stringOctetBufferParsed", NULL},

            // New for IPv6 support.
            {12, kIpv4OrIpv6Length,             kTLV_IpAddress,         {&ipv4AddressParsedEither}, false, false, "ipv4AddressParsedEither", NULL},
            {13, BcmIpV6Address::ADDR_LEN,      kTLV_IpAddress,         {&ipv6AddressParsed},       false, false, "ipv6AddressParsed", NULL},
            {14, kIpv4OrIpv6Length,             kTLV_IpAddress,         {&ipv6AddressParsedEither}, false, false, "ipv6AddressParsedEither", NULL},

            // This must come last.  It tells the parser when to stop.
            kEndOfTLVTable
        };

        TLVTable parentTable[] =
        {
            {128, kUnknownLength, kTLV_nested, {tlvTable}, false, false, "ParentTlv", NULL},

            // This must come last.  It tells the parser when to stop.
            kEndOfTLVTable
        };

        lengthIs2Bytes = false;
        while (1)
        {
            {
                tlvBuffer.Empty();
                parentTlvBuffer.Empty();

                if (lengthIs2Bytes == false)
                {
                    gInfoMsg(messageLogSettings, "RegressTlvs")
                        << "Testing the TLV functions for single-byte length field:\n" << endl;
                }
                else
                {
                    gInfoMsg(messageLogSettings, "RegressTlvs")
                        << "Testing the TLV functions for 2-byte length field:\n" << endl;
                }

                gInfoMsg(messageLogSettings, "RegressTlvs")
                    << "Generating TLVs into a buffer..." << endl;

                // Put each value into the tlv buffer one at a time.
                if ((GenerateTLV(tlvBuffer, 0, uint8Value, &messageLogSettings, "uint8Value", lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 1, uint16Value, &messageLogSettings, "uint16Value", lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 2, uint32Value, &messageLogSettings, "uint32Value", lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 3, int8Value, &messageLogSettings, "int8Value", lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 4, int16Value, &messageLogSettings, "int16Value", lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 5, int32Value, &messageLogSettings, "int32Value", lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 6, stringValue, &messageLogSettings,  "stringValue", lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 7, octetBuffer, &messageLogSettings, "octetBuffer", lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 8, fixedByteArray, sizeof(fixedByteArray), &messageLogSettings, "fixedByteArray", kTLV_FixedByteArray, lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 9, ipv4Address, &messageLogSettings, "ipv4Address", lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 10, macAddress, &messageLogSettings, "macAddress", lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 11, stringOctetBuffer, &messageLogSettings, "stringOctetBuffer", lengthIs2Bytes) == 0) ||
                    
                    // New for IPv6 support.
                    (GenerateTLV(tlvBuffer, 12, ipv4Address, &messageLogSettings, "ipv4Address", lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 13, ipv6Address, &messageLogSettings, "ipv6Address", lengthIs2Bytes) == 0) ||
                    (GenerateTLV(tlvBuffer, 14, ipv6Address, &messageLogSettings, "ipv6Address", lengthIs2Bytes) == 0))
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "Call to GenerateTLV failed!" << endl;

                    return false;
                }

                gInfoMsg(messageLogSettings, "RegressTlvs")
                    << "Generating parent TLV that contains the other TLVs..." << endl;

                GenerateTLV(parentTlvBuffer, 128, tlvBuffer, &messageLogSettings, "parentOfNested", lengthIs2Bytes);

                gInfoMsg(messageLogSettings, "RegressTlvs")
                    << "Here's the final TLV buffer after adding everything to it:\n"
                    << parentTlvBuffer << endl;
            }

            {
                gInfoMsg(messageLogSettings, "RegressTlvs")
                    << "Parsing TLVs from the buffer..." << endl;

                offset = 0;
                if (ParseTLVs(parentTlvBuffer, offset, parentTable, &messageLogSettings, lengthIs2Bytes) != kTLVParserEndOfData)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "Call to ParseTLVs failed!" << endl;

                    return false;
                }

                gInfoMsgNoFields(messageLogSettings) << endl;
            }

            {
                // Verify the values before declaring victory and moving on.
                if (uint8Value != uint8Parsed)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "uint8 values don't match!" << endl;

                    return false;
                }
                if (uint16Value != uint16Parsed)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "uint16 values don't match!" << endl;

                    return false;
                }
                if (uint32Value != uint32Parsed)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "uint32 values don't match!" << endl;

                    return false;
                }
                if (int8Value != int8Parsed)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "int8 values don't match!" << endl;

                    return false;
                }
                if (int16Value != int16Parsed)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "int16 values don't match!" << endl;

                    return false;
                }
                if (int32Value != int32Parsed)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "int32 values don't match!" << endl;

                    return false;
                }
                if (strcmp(stringValue, stringParsed) != 0)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "string values don't match!" << endl;

                    return false;
                }
                if (octetBuffer != octetBufferParsed)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "octetBuffer values don't match!" << endl;

                    return false;
                }
                if (memcmp(fixedByteArray, fixedByteArrayParsed, sizeof(fixedByteArray)) != 0)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "fixedByteArray values don't match!" << endl;

                    return false;
                }
                if (ipv4Address != ipv4AddressParsed)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "ipv4Address values don't match!" << endl;

                    return false;
                }
                if (ipv4Address != ipv4AddressParsedEither)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "ipv4Address values (either length) don't match!" << endl;

                    return false;
                }
                if (ipv6Address != ipv6AddressParsed)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "ipv6Address values don't match!" << endl;

                    return false;
                }
                if (ipv6Address != ipv6AddressParsedEither)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "ipv6Address values (either length) don't match!" << endl;

                    return false;
                }
                if (macAddress != macAddressParsed)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "macAddress values don't match!" << endl;

                    return false;
                }
                if (stringOctetBuffer != stringOctetBufferParsed)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "stringOctetBuffer values don't match!" << endl;

                    return false;
                }
            }

            // If we've tried byte and word sized length fields, then break out.
            if (lengthIs2Bytes == true)
            {
                break;
            }

            // Otherwise, next time around, we need to try 2-byte length fields.
            lengthIs2Bytes = true;
        }

        // New test code that exercises the new API.
        {
            TlvConfig config;
            unsigned int tlvTypeSizeIndex, tlvLengthSizeIndex;
            unsigned int tlvLengthAttributeIndex;

            TlvTypeSize tlvTypeSizeValue[] =
            {
                kTlvTypeIs8Bits,
                kTlvTypeIs16Bits,
                kTlvTypeIs32Bits
            };

            TlvLengthSize tlvLengthSizeValue[] =
            {
                kTlvLengthIs8Bits,
                kTlvLengthIs16Bits,
                kTlvLengthIs32Bits
            };

            TlvLengthAttribute tlvLengthAttributeValue[] =
            {
                kTlvLengthExcludesTL,
                kTlvLengthIncludesTL
            };

            gInfoMsg(messageLogSettings, "RegressTlvs")
                << "\nTesting new-school TLV engine APIs...\n" << endl;

            TlvConfigInit(config);
            config.pfMessageLogSettings = &messageLogSettings;

            for (tlvTypeSizeIndex = 0; tlvTypeSizeIndex < 3; tlvTypeSizeIndex ++)
            {
                for (tlvLengthSizeIndex = 0; tlvLengthSizeIndex < 3; tlvLengthSizeIndex++)
                {
                    for (tlvLengthAttributeIndex = 0; tlvLengthAttributeIndex < 2; tlvLengthAttributeIndex++)
                    {
                        {
                            // Set the TLV engine config.
                            config.fTlvTypeSize = tlvTypeSizeValue[tlvTypeSizeIndex];
                            config.fTlvLengthSize = tlvLengthSizeValue[tlvLengthSizeIndex];
                            config.fTlvLengthAttribute = tlvLengthAttributeValue[tlvLengthAttributeIndex];

                            gInfoMsg(messageLogSettings, "RegressTlvs")
                                << "Testing the following config:\n"
                                << setfill(' ')
                                << setw(25) << "Type size = " << config.fTlvTypeSize << " bytes\n"
                                << setw(25) << "Length size = " << config.fTlvLengthSize << " bytes\n"
                                << setw(25) << "Length includes TL = " << (bool) config.fTlvLengthAttribute
                                << endl << endl;
                        }

                        {
                            tlvBuffer.Empty();
                            parentTlvBuffer.Empty();

                            gInfoMsg(messageLogSettings, "RegressTlvs")
                                << "Generating TLVs into a buffer..." << endl;

                            // Put each value into the tlv buffer one at a time.
                            if ((GenerateTLV(tlvBuffer, 0, uint8Value, config, "uint8Value") == 0) ||
                                (GenerateTLV(tlvBuffer, 1, uint16Value, config, "uint16Value") == 0) ||
                                (GenerateTLV(tlvBuffer, 2, uint32Value, config, "uint32Value") == 0) ||
                                (GenerateTLV(tlvBuffer, 3, int8Value, config, "int8Value") == 0) ||
                                (GenerateTLV(tlvBuffer, 4, int16Value, config, "int16Value") == 0) ||
                                (GenerateTLV(tlvBuffer, 5, int32Value, config, "int32Value") == 0) ||
                                (GenerateTLV(tlvBuffer, 6, stringValue, config,  "stringValue") == 0) ||
                                (GenerateTLV(tlvBuffer, 7, octetBuffer, config, "octetBuffer") == 0) ||
                                (GenerateTLV(tlvBuffer, 8, fixedByteArray, sizeof(fixedByteArray), config, "fixedByteArray", kTLV_FixedByteArray) == 0) ||
                                (GenerateTLV(tlvBuffer, 9, ipv4Address, config, "ipv4Address") == 0) ||
                                (GenerateTLV(tlvBuffer, 10, macAddress, config, "macAddress") == 0) ||
                                (GenerateTLV(tlvBuffer, 11, stringOctetBuffer, config, "stringOctetBuffer") == 0) ||
                    
                                // New for IPv6 support.
                                (GenerateTLV(tlvBuffer, 12, ipv4Address, config, "ipv4Address") == 0) ||
                                (GenerateTLV(tlvBuffer, 13, ipv6Address, config, "ipv6Address") == 0) ||
                                (GenerateTLV(tlvBuffer, 14, ipv6Address, config, "ipv6Address") == 0))
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "Call to GenerateTLV failed!" << endl;

                                return false;
                            }

                            gInfoMsg(messageLogSettings, "RegressTlvs")
                                << "Generating parent TLV that contains the other TLVs..." << endl;

                            GenerateTLV(parentTlvBuffer, 128, tlvBuffer, config, "parentOfNested");

                            gInfoMsg(messageLogSettings, "RegressTlvs")
                                << "Here's the final TLV buffer after adding everything to it:\n"
                                << parentTlvBuffer << endl;
                        }

                        {
                            gInfoMsg(messageLogSettings, "RegressTlvs")
                                << "Parsing TLVs from the buffer..." << endl;

                            offset = 0;
                            if (ParseTLVs(parentTlvBuffer, offset, parentTable, config) != kTLVParserEndOfData)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "Call to ParseTLVs failed!" << endl;

                                return false;
                            }

                            gInfoMsgNoFields(messageLogSettings) << endl;
                        }

                        {
                            // Compare the results.
                            if (uint8Value != uint8Parsed)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "uint8 values don't match!" << endl;

                                return false;
                            }
                            if (uint16Value != uint16Parsed)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "uint16 values don't match!" << endl;

                                return false;
                            }
                            if (uint32Value != uint32Parsed)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "uint32 values don't match!" << endl;

                                return false;
                            }
                            if (int8Value != int8Parsed)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "int8 values don't match!" << endl;

                                return false;
                            }
                            if (int16Value != int16Parsed)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "int16 values don't match!" << endl;

                                return false;
                            }
                            if (int32Value != int32Parsed)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "int32 values don't match!" << endl;

                                return false;
                            }
                            if (strcmp(stringValue, stringParsed) != 0)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "string values don't match!" << endl;

                                return false;
                            }
                            if (octetBuffer != octetBufferParsed)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "octetBuffer values don't match!" << endl;

                                return false;
                            }
                            if (memcmp(fixedByteArray, fixedByteArrayParsed, sizeof(fixedByteArray)) != 0)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "fixedByteArray values don't match!" << endl;

                                return false;
                            }
                            if (ipv4Address != ipv4AddressParsed)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "ipv4Address values don't match!" << endl;

                                return false;
                            }
                            if (ipv4Address != ipv4AddressParsedEither)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "ipv4Address values (either length) don't match!" << endl;

                                return false;
                            }
                            if (ipv6Address != ipv6AddressParsed)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "ipv6Address values don't match!" << endl;

                                return false;
                            }
                            if (ipv6Address != ipv6AddressParsedEither)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "ipv6Address values (either length) don't match!" << endl;

                                return false;
                            }
                            if (macAddress != macAddressParsed)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "macAddress values don't match!" << endl;

                                return false;
                            }
                            if (stringOctetBuffer != stringOctetBufferParsed)
                            {
                                gErrorMsg(messageLogSettings, "RegressTlvs")
                                    << "stringOctetBuffer values don't match!" << endl;

                                return false;
                            }
                        }
                    }
                }
            }

            gInfoMsg(messageLogSettings, "RegressTlvs")
                << "Special test for string types (include/exclude \\0 terminator)..." << endl;

            {
                char *pMyString = "This is a string!";
                char parsedString[32];

                TLVTable stringParseTable[] =
                {
                    {0, kUnknownLength, kTLV_string, {parsedString}, false, false, "stringParsed", NULL},

                    // This must come last.  It tells the parser when to stop.
                    kEndOfTLVTable
                };

                // Reset the parser/generator config.
                TlvConfigInit(config);
                config.pfMessageLogSettings = &messageLogSettings;

                {
                    tlvBuffer.Empty();

                    if (GenerateTLV(tlvBuffer, 0, pMyString, config, "stringValue (with \\0)") != strlen(pMyString) + 2 + 1)
                    {
                        gErrorMsg(messageLogSettings, "RegressTlvs")
                            << "Failed to write proper number of bytes for string with \\0!" << endl;

                        return false;
                    }

                    offset = 0;
                    if (ParseTLVs(tlvBuffer, offset, stringParseTable, config) != kTLVParserEndOfData)
                    {
                        gErrorMsg(messageLogSettings, "RegressTlvs")
                            << "Call to ParseTLVs failed!" << endl;

                        return false;
                    }

                    if (strcmp(pMyString, parsedString) != 0)
                    {
                        gErrorMsg(messageLogSettings, "RegressTlvs")
                            << "Parsed string doesn't compare to original string!" << endl;

                        return false;
                    }
                }

                {
                    config.fTlvString = kTlvStringExcludes0;

                    tlvBuffer.Empty();

                    if (GenerateTLV(tlvBuffer, 0, pMyString, config, "stringValue (without \\0)") != strlen(pMyString) + 2)
                    {
                        gErrorMsg(messageLogSettings, "RegressTlvs")
                            << "Failed to write proper number of bytes for string without \\0!" << endl;

                        return false;
                    }

                    offset = 0;
                    if (ParseTLVs(tlvBuffer, offset, stringParseTable, config) != kTLVParserEndOfData)
                    {
                        gErrorMsg(messageLogSettings, "RegressTlvs")
                            << "Call to ParseTLVs failed!" << endl;

                        return false;
                    }

                    if (strcmp(pMyString, parsedString) != 0)
                    {
                        gErrorMsg(messageLogSettings, "RegressTlvs")
                            << "Parsed string doesn't compare to original string!" << endl;

                        return false;
                    }
                }
            }

            // Test some corner cases.
            {
                gInfoMsg(messageLogSettings, "RegressTlvs")
                    << "\nCorner cases..." << endl;

                // Reset the parser/generator config.
                TlvConfigInit(config);
                config.pfMessageLogSettings = &messageLogSettings;
                config.fLength0Valid = kTlvLength0Allowed;

                tlvBuffer.Empty();

                if (GenerateTLV(tlvBuffer, 1, NULL, 0, config, "0-length value with NULL pointer", kTLV_FixedByteArray) == false)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "Failed write 0-length TLV with NULL value pointer (value not relevant)!" << endl;

                    return false;
                }

                gInfoMsg(messageLogSettings, "RegressTlvs")
                    << "Non-0 length with NULL pointer; this should generate an error message..." << endl;

                if (GenerateTLV(tlvBuffer, 1, NULL, 1, config, "Non-0-length value with NULL pointer", kTLV_FixedByteArray) == true)
                {
                    gErrorMsg(messageLogSettings, "RegressTlvs")
                        << "Wrote non-0-length TLV with NULL value pointer (should have been rejected)!" << endl;

                    return false;
                }
            }
        }

        gInfoMsg(messageLogSettings, "RegressTlvs")
            << "\nAll tests worked!" << endl;

        return true;
    }
    #endif
}


static void LogTypeLength(uint32 TypeField, const char* TypeName, uint32 LengthField,
                          BcmMessageLogSettings *pMessageLogSettings,
                          bool LeadingNewline)
{
    if( pMessageLogSettings )
    {
        if( LeadingNewline )
        {
            gNoSlimInfoMsg(*pMessageLogSettings, "ParseTLVs") << endl;
        }
        gNoSlimInfoMsg(*pMessageLogSettings, "ParseTLVs") 
            << setfill( ' ' ) << setw( 24 )
            << TypeName
            << setfill( '0' )
            << ": T=0x" << hex << setw( 2 ) << TypeField
                << " (" << dec << setw( 3 ) << TypeField << hex
            << ")  L=0x" << setw( 2 ) << LengthField
                << " (" << dec << setw( 3 ) << LengthField << ")  V=" << setfill( ' ' );
    }
}


/// This is the singleton accessor for this object.  Hopefully, it allows
/// creation to be deferred until non-vol settings are read in so that we can
/// get the correct defaults.
///
/// \retval
///      A reference to the singleton instance.
///
static BcmMessageLogSettings *MessageLogSettings(void)
{
    static BcmMessageLogSettings gMessageLogSettings("TLV");

    return &gMessageLogSettings;
}


