//**************************************************************************
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
//
//
//****************************************************************************
//  $Id$
//
//  Filename:       EstbTLV.h
//  Author:         Maurice Turcotte
//  Creation Date:  8 NOV 2006
//
//****************************************************************************
//  Description:
//      This file contains helper functions for dealing with the creation and
//      parsing of TLVs.  
//
//****************************************************************************

#ifndef ESTBTLV_H
#define ESTBTLV_H

//********************** Include Files ***************************************

// The octet buffer storage wrapper class.
#include "OctetBuffer.h"

// These are the DOCSIS-defined values for the T portion of the TLV (from
// appendix C and elsewhere in the spec).
#include "EstbTLVcodes.h"

#include "IpAddress.h"
#include "MacAddress.h"

// define this to do a test of all the TLVs when a forced config file is read
// #define TEST_TLVS 1

//********************** Global Types ****************************************

// TLV-related types.  These are type hints that tell me how to interprit the
// the value portion of the TLV and to convert it from the flat octet-based
// network format to the C data member in host format.
typedef enum
{
    kTLV_uint8,             // Unsigned 8-bit integer.
    kTLV_int8,              // Signed 8-bit integer.
    kTLV_uint16,            // Unsigned 16-bit integer.
    kTLV_int16,             // Signed 16-bit integer.
    kTLV_uint32,            // Unsigned 32-bit integer.
    kTLV_int32,             // Signed 32-bit integer.

    kTLV_string,            // \0-terminated C character string.
    kTLV_OctetBuffer,       // store a variable length array of bytes
                            // to target var of type BcmOctetBuffer.
                            // length is unknown at compile time and
                            // determined at run-time from L item of TLV.
    kTLV_StringOctetBuffer, // This is a combination of the kTLV_string and
                            // kTLV_OctetBuffer types.  It behaves like
                            // kTLV_string, doing the \0 string termination
                            // stuff, but the data is stored in an OctetBuffer
                            // rather than a char *.
    kTLV_FixedByteArray,    // a fixed length array of byte values.
                            // length is known at compile time and specified
                            // in TLV input table.

    kTLV_nested,            // The V portion is a set of 1 or more TLVs that
                            // need to be parsed.
    kTLV_IpAddress,         
    kTLV_MacAddress,        
    kTLV_end_of_data        // Special end-of-file/table marker.  This helps the
                            // parsing engine determine where the end of the
                            // table is.
} TLV_ValueEncoding;


// This is the master structure that contains the TLV triplet, along with other
// associated values.  This is only used for parsing TLVs; the client will
// define a table (contiguous array) of these, initializing them appropriately,
// to form the table of interesting TLVs that should be parsed.  This table will
// be handed to the parsing engine, along with the octet buffer containing the
// raw TLV data, and the V's will be parsed.
typedef struct _TLVTable
{
    // The type and length fields associated with this TLV.  If you know the
    // length of the value, then set it correctly and the parser will validate
    // that the received length is correct.  If you don't know (for strings,
    // uint8 arrays, etc.), then set it to kUnknownLength (0).
    uint8 type;

    // The length field is only an input parameter for the parser (used for
    // validation and error checking of the TLV).  The parser does not update
    // this with the actual length that was read.  It is assumed that you either
    // know the actual length (a uint8 is always 1 byte) or can calculate it
    // easily (strlen for strings, and BytesUsed for BcmOctetBuffer).
    //
    // Note that we now handle both 8-bit and 16-bit lengths (mainly for BPI in
    // DOCSIS).  If you want the length to be treated by the parser as a 16-bit
    // value, then you must call ParseTLVs with the lengthIsUint16 parameter
    // set to true.
    unsigned int length;

    // This tells the parsing engine how to interprit the value field (union).
    TLV_ValueEncoding valueEncoding;

    // Overlaid mapping of the possible values.  These are pointers to the
    // actual variables (supplied by the client) where the parsed value will be
    // stored.  NOTE:  The client who sets up the table MUST ensure that the
    // pointer below refers to the appropriate memory/object!  We have not
    // attempted to make this type-safe or idiot proof.

    
    union
    {
        // This is only intended as a place-holder for doing table
        // initialization.  There is no way for the compiler to know which of
        // the types in this union is being initialized, so it uses the first
        // one.  A void * type can be used for all pointer types, so it is
        // placed first here.  It should not be used for any other purpose.
        void *pUnknown;

        // The rest of these correspond to one of the TLV_ValueEncoding types.
        uint8 *pUint8Value;

        int8 *pInt8Value;

        uint16 *pUint16Value;

        int16 *pInt16Value;

        uint32 *pUint32Value;

        int32 *pInt32Value;

        char *pStringValue;

        // This is used both for kTLV_StringOctetBuffer and kTLV_OctetBuffer.
        BcmOctetBuffer *pOctetBuffer;   

        BcmIpAddress *pIpAddressValue;

        BcmMacAddress *pMacAddressValue;

        struct _TLVTable *pTLVTable;

    } uTLV;

    // Allows the client to tell the parsing engine to stop gracefully when it
    // encounters the specified type code.
    bool stopParsing;

    // Tells whether or not this TLV was encountered.
    bool wasFound;

    // The text name of the TLV, for debugging.
    char *pName;

    // This can be useful in addition to the wasFound member above.  Many
    // settings structures have a requirement to test/store whether or not a
    // given value was specified.  If not, then special rules may apply.  If
    // this pointer is not NULL, then the TLV parser will store true here if
    // the TLV is found.
    bool *pValueWasSpecified;

    // the TLV is unique, that is, there can only be one of these per table.
    bool isUnique;

} TLVTable;



// These macros are for the union fields inside the TLV Table.
// Originally, the union was unnamed until we reached the port to vxWorks
// where the GNU compiler needed the union to be named in order to compile properly.
#define	uTLV_pUnknown		     uTLV.pUnknown
#define uTLV_pUint8Value         uTLV.pUint8Value
#define uTLV_pInt8Value          uTLV.pInt8Value
#define uTLV_pUint16Value        uTLV.pUint16Value
#define uTLV_pInt16Value         uTLV.pInt16Value
#define uTLV_pUint32Value        uTLV.pUint32Value
#define uTLV_pInt32Value         uTLV.pInt32Value
#define uTLV_pStringValue        uTLV.pStringValue
#define uTLV_pOctetBuffer        uTLV.pOctetBuffer
#define uTLV_pIpAddressValue     uTLV.pIpAddressValue
#define uTLV_pMacAddressValue    uTLV.pMacAddressValue
#define uTLV_pTLVTable           uTLV.pTLVTable



// These are the values that are returned by the parser.
typedef enum
{
    // These are both valid/successful reasons for the parser exiting.
    kTLVParserEndOfData,                // No more data to parse.  All TLVs
                                        // parsed successfully.
    kTLVParserStopParsingFound,         // The parser found a T code which had
                                        // its stopParsing flag set.

    // This is not an error, but we need to pause and log the event
    kTLVParserDuplicateTLV,             // There were two (or more) sub-TLVs 
                                        // found where there should be just one
    // These are the error codes.
    kTLVParserError,                    // General (unspecified) error.
    kTLVParserErrorNULLTable,           // The table parameter was NULL.
    kTLVParserErrorInvalidLength,       // The length field of the TLV didn't
                                        // match the expected length specified
                                        // in the table.
    kTLVParserErrorInsufficientData,    // Not enough data in the buffer to
                                        // satisfy the length field.
    kTLVParserErrorNULLValuePointer,    // The pointer to the value in the table
                                        // is NULL!  Can't store the value.
    kTLVParserErrorSmallOctetBuffer,    // For type kTLV_OctetBuffer, the length
                                        // of the specified octet buffer is not
                                        // enough to hold all of the data.
    kTLVParserErrorDataLeftOver         // There were some bytes left over in
                                        // the input buffer; may indicate loss
                                        // of synchronization or invalid TLVs.
} TLVParserResult;


//********************** Global Constants ************************************

// Use this as the length field in the TLVTable if you don't want length
// validation to be done for this type during parsing.
#define kUnknownLength 0

// This is the TLVTable entry that must be the last item in every table.  Note
// that I would have preferred to use the const form that is commented out, but
// the stupid Microsoft compiler considers it an error, so I have to use the
// #defined version.
//const TLVTable kEndOfTLVTable = {kEndOfDataMarker, kUnknownLength, kTLV_end_of_data, (void *) 0, false, false, "Unknown", NULL};
#define kEndOfTLVTable {kEndOfDataMarker, kUnknownLength, kTLV_end_of_data, (void *) 0, false, false, (char *) "Unknown", NULL, false}
#define kEndOfTLVTableStopParsing {kEndOfDataMarker, kUnknownLength, kTLV_end_of_data, {(void *) 0}, true, false, "Unknown", NULL, false}

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmMessageLogSettings;

//********************** Class Declaration ***********************************



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
                          BcmMessageLogSettings *pMessageLogSettings = NULL,
                          bool lengthIsUint16 = false,
                          bool lengthIncludesTypeAndLength = false);

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
                               bool *pValueWasSpecified);

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
bool GetTLVTableWasFound(TLVTable *pTLVTable, uint8 type);


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
                         BcmMessageLogSettings *pMessageLogSettings = NULL,
                         const char *pTlvName = NULL,
                         TLV_ValueEncoding valueType = kTLV_FixedByteArray,
                         bool lengthIsUint16 = false,
                         bool lengthIncludesTypeAndLength = false);

// This one handles a string value.  The length of the string is calculated for
// you.
unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode,
                         const char *pStringValue, 
                         BcmMessageLogSettings *pMessageLogSettings = NULL,
                         const char *pTlvName = NULL,
                         bool lengthIsUint16 = false,
                         bool lengthIncludesTypeAndLength = false);

// This one handles a uint8 value.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                                uint8 value, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

// This one handles a uint16 value.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                                uint16 value, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

// This one handles a uint32 value.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                                uint32 value, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

// This one handles an int8 value.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                                int8 value, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

// This one handles an int16 value.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                                int16 value, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

// This one handles an int32 value.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                                int32 value, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

// This one handles an OctetBuffer.  The length comes from the BytesUsed field
// of the octet buffer.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode,
                                const BcmOctetBuffer &octetValue, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

// This one handles a MAC address.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode,
                                const BcmMacAddress &macAddress, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

// This one handles an IP address.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode,
                                const BcmIpAddress &ipAddress, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

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
bool TestTLVs(void);

//********************** Inline Method Implementations ***********************

// This one handles a uint8 value.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                                uint8 value, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    return GenerateTLV(buffer, typeCode, &value, sizeof(value), pMessageLogSettings, pTlvName, kTLV_uint8, lengthIsUint16, lengthIncludesTypeAndLength);
}

// This one handles a uint16 value.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                                uint16 value, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    // Convert the value to network order.
    value = HostToNetwork(value);

    return GenerateTLV(buffer, typeCode, &value, sizeof(value), pMessageLogSettings, pTlvName, kTLV_uint16, lengthIsUint16, lengthIncludesTypeAndLength);
}

// This one handles a uint32 value.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                                uint32 value, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    // Convert the value to network order.
    value = HostToNetwork(value);

    return GenerateTLV(buffer, typeCode, &value, sizeof(value), pMessageLogSettings, pTlvName, kTLV_uint32, lengthIsUint16, lengthIncludesTypeAndLength);
}

// This one handles an int8 value.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                                int8 value, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    return GenerateTLV(buffer, typeCode, &value, sizeof(value), pMessageLogSettings, pTlvName, kTLV_int8, lengthIsUint16, lengthIncludesTypeAndLength);
}

// This one handles an int16 value.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                                int16 value, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    // Convert the value to network order.
    value = HostToNetwork(value);

    return GenerateTLV(buffer, typeCode, &value, sizeof(value), pMessageLogSettings, pTlvName, kTLV_int16, lengthIsUint16, lengthIncludesTypeAndLength);
}

// This one handles an int32 value.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode, 
                                int32 value, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    // Convert the value to network order.
    value = HostToNetwork(value);

    return GenerateTLV(buffer, typeCode, &value, sizeof(value), pMessageLogSettings, pTlvName, kTLV_int32, lengthIsUint16, lengthIncludesTypeAndLength);
}

// This one handles an OctetBuffer.  The length comes from the BytesUsed field
// of the octet buffer.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode,
                                const BcmOctetBuffer &octetValue, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    return GenerateTLV(buffer, typeCode, octetValue.AccessBuffer(), octetValue.BytesUsed(), pMessageLogSettings, pTlvName, kTLV_OctetBuffer, lengthIsUint16, lengthIncludesTypeAndLength);
}

// This one handles a MAC address.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode,
                                const BcmMacAddress &macAddress, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    uint8 octets[6];

    // Get the values from the object, then vector to the real function.
    macAddress.Get(octets[0], octets[1], octets[2], octets[3], octets[4], octets[5]);

    return GenerateTLV(buffer, typeCode, octets, sizeof(octets), pMessageLogSettings, pTlvName, kTLV_MacAddress, lengthIsUint16, lengthIncludesTypeAndLength);
}

// This one handles an IP address.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode,
                                const BcmIpAddress &ipAddress, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    uint8 octets[4];

    // Get the values from the object, then vector to the real function.
    ipAddress.Get(octets[0], octets[1], octets[2], octets[3]);

    return GenerateTLV(buffer, typeCode, octets, sizeof(octets), pMessageLogSettings, pTlvName, kTLV_IpAddress, lengthIsUint16, lengthIncludesTypeAndLength);
}

#endif

#endif


