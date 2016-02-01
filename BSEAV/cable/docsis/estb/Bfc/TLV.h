//****************************************************************************
//
// Copyright (c) 1999-2009 Broadcom Corporation
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
//  Filename:       TLV.h
//  Author:         David Pullen
//  Creation Date:  April 28, 1999
//
//****************************************************************************

#ifndef TLV_H
#define TLV_H

/** \file TLV.h
*
*   This file contains helper functions for dealing with the creation and
*   parsing of TLVs.  The parser uses a table-based lookup approach that is
*   very flexible and easy to configure.  The generator supports numerous
*   different data types.
*
*   The TLV engine allows each client to configure the parsing/generation
*   behavior, including the size of the Type value, the size of the Length
*   value, whether or not the Length includes the T and L bytes, as well
*   as debug output behavior.  This is all controlled via the TlvConfig
*   structure.
*/

//********************** Include Files ***************************************

// The octet buffer storage wrapper class.
#include "OctetBuffer.h"

#include "IpAddress.h"
#include "MacAddress.h"

#if (BFC_INCLUDE_DOCSIS_SUPPORT)
#include "Dsid.h"
#include "Cmim.h"
#endif
#include "BcmBasicString.h"

//********************** Forward Declarations ********************************

class BcmMessageLogSettings;

//********************** Global Types ****************************************

/// TLV-related types.  These are type hints that tell me how to interprit the
/// the value portion of the TLV and to convert it from the flat octet-based
/// network format to the C data member in host format.
typedef enum
{
    kTLV_uint8,             ///< Unsigned 8-bit integer.
    kTLV_int8,              ///< Signed 8-bit integer.
    kTLV_uint16,            ///< Unsigned 16-bit integer.
    kTLV_int16,             ///< Signed 16-bit integer.
    kTLV_uint32,            ///< Unsigned 32-bit integer.
    kTLV_int32,             ///< Signed 32-bit integer.

    kTLV_string,            ///< \0-terminated C character string.
    kTLV_OctetBuffer,       ///< store a variable length array of bytes
                            ///< to target var of type BcmOctetBuffer.
                            ///< length is unknown at compile time and
                            ///< determined at run-time from L item of TLV.
    kTLV_StringOctetBuffer, ///< This is a combination of the kTLV_string and
                            ///< kTLV_OctetBuffer types.  It behaves like
                            ///< kTLV_string, doing the \0 string termination
                            ///< stuff, but the data is stored in an OctetBuffer
                            ///< rather than a char *.
    kTLV_FixedByteArray,    ///< a fixed length array of byte values.
                            ///< length is known at compile time and specified
                            ///< in TLV input table.

    kTLV_nested,            ///< The V portion is a set of 1 or more TLVs that
                            ///< need to be parsed.

    kTLV_IpAddress,         ///< A BcmIpAddress object.  If you only want to
                            ///< parse IPv4 addresses, use 4 for the length.
                            ///< If you only want to parse IPv6 addresses, use
                            ///< 16 for the length.  If you want to support
                            ///< either, use kIpv4OrIpv6Length.

    kTLV_MacAddress,        ///< A BcmMacAddress object.
    kTLV_CMIM,              ///< A L2VPN or DOCSIS 3.0 CMIM
    kTLV_DOCS3_DSID,        ///< DOCSIS 3.0 downstream service id (DSID)

    kTLV_BcmBasicString,    ///< A BcmBasicString object. 

    kTLV_end_of_data        ///< Special end-of-file/table marker.  This helps the
                            ///< parsing engine determine where the end of the
                            ///< table is.
} TLV_ValueEncoding;


/// This is the master structure that contains the TLV triplet, along with other
/// associated values.  This is only used for parsing TLVs; the client will
/// define a table (contiguous array) of these, initializing them appropriately,
/// to form the table of interesting TLVs that should be parsed.  This table will
/// be handed to the parsing engine, along with the octet buffer containing the
/// raw TLV data, and the V's will be parsed.
typedef struct _TLVTable
{
    /// The type and length fields associated with this TLV.  If you know the
    /// length of the value, then set it correctly and the parser will validate
    /// that the received length is correct.  If you don't know (for strings,
    /// uint8 arrays, etc.), then set it to kUnknownLength (0).
    ///
    /// Note that we now handle 8-bit, 16-bit and 32-bit type values.
    uint32 type;

    /// The length field is only an input parameter for the parser (used for
    /// validation and error checking of the TLV).  The parser does not update
    /// this with the actual length that was read.  It is assumed that you either
    /// know the actual length (a uint8 is always 1 byte) or can calculate it
    /// easily (strlen for strings, and BytesUsed for BcmOctetBuffer).
    ///
    /// Note that we now handle 8-bit, 16-bit and 32-bit lengths.
    uint32 length;

    /// This tells the parsing engine how to interprit the value field (union).
    TLV_ValueEncoding valueEncoding;

    /// Overlaid mapping of the possible values.  These are pointers to the
    /// actual variables (supplied by the client) where the parsed value will be
    /// stored.  NOTE:  The client who sets up the table MUST ensure that the
    /// pointer below refers to the appropriate memory/object!  We have not
    /// attempted to make this type-safe or idiot proof.
    union
    {
        /// This is only intended as a place-holder for doing table
        /// initialization.  There is no way for the compiler to know which of
        /// the types in this union is being initialized, so it uses the first
        /// one.  A void * type can be used for all pointer types, so it is
        /// placed first here.  It should not be used for any other purpose.
        void *pUnknown;

        /// The rest of these correspond to one of the TLV_ValueEncoding types.
        uint8 *pUint8Value;

        int8 *pInt8Value;

        uint16 *pUint16Value;

        int16 *pInt16Value;

        uint32 *pUint32Value;

        int32 *pInt32Value;

        char *pStringValue;

        /// This is used both for kTLV_StringOctetBuffer and kTLV_OctetBuffer.
        BcmOctetBuffer *pOctetBuffer;   

        BcmIpAddress *pIpAddressValue;

        BcmMacAddress *pMacAddressValue;

        #if (BFC_INCLUDE_DOCSIS_SUPPORT)
        
        BcmCmim *pCmimValue;

        BcmDsid *pDsidValue;

        #endif
        
        struct _TLVTable *pTLVTable;

    } uTLV;

    /// Allows the client to tell the parsing engine to stop gracefully when it
    /// encounters the specified type code.
    bool stopParsing;

    /// Tells whether or not this TLV was encountered.
    bool wasFound;

    /// The text name of the TLV, for debugging.
    const char *pName;

    /// This can be useful in addition to the wasFound member above.  Many
    /// settings structures have a requirement to test/store whether or not a
    /// given value was specified.  If not, then special rules may apply.  If
    /// this pointer is not NULL, then the TLV parser will store true here if
    /// the TLV is found.
    bool *pValueWasSpecified;

} TLVTable;



/// These macros are for the union fields inside the TLV Table.
/// Originally, the union was unnamed until we reached the port to vxWorks
/// where the GNU compiler needed the union to be named in order to compile
/// properly.
//@{
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

#if (BFC_INCLUDE_DOCSIS_SUPPORT)
#define uTLV_pCmimValue          uTLV.pCmimValue
#define uTLV_pDsidValue          uTLV.pDsidValue
#endif
#define uTLV_pBasicStringValue   uTLV.pBasicStringValue
#define uTLV_pTLVTable           uTLV.pTLVTable
//@}


/// These are the values that are returned by the parser.
typedef enum
{
    // These are both valid/successful reasons for the parser exiting.
    kTLVParserEndOfData,                ///< No more data to parse.  All TLVs
                                        ///< parsed successfully.
    kTLVParserStopParsingFound,         ///< The parser found a T code which had
                                        ///< its stopParsing flag set.

    // These are the error codes.
    kTLVParserError,                    ///< General (unspecified) error.
    kTLVParserErrorNULLTable,           ///< The table parameter was NULL.
    kTLVParserErrorInvalidLength,       ///< The length field of the TLV didn't
                                        ///< match the expected length specified
                                        ///< in the table.
    kTLVParserErrorInsufficientData,    ///< Not enough data in the buffer to
                                        ///< satisfy the length field.
    kTLVParserErrorNULLValuePointer,    ///< The pointer to the value in the table
                                        ///< is NULL!  Can't store the value.
    kTLVParserErrorSmallOctetBuffer,    ///< For type kTLV_OctetBuffer, the length
                                        ///< of the specified octet buffer is not
                                        ///< enough to hold all of the data.
    kTLVParserErrorDataLeftOver,        ///< There were some bytes left over in
                                        ///< the input buffer; may indicate loss
                                        ///< of synchronization or invalid TLVs.
    kTLVParserErrorSmallBcmBasicString  ///< For type kTLV_BcmBasicString, the 
                                        ///< capacity of the specified BcmBasicString
                                        ///< is not enough to hold all of the data.

} TLVParserResult;


/// Used in the TlvConfig struct.
typedef enum
{
    /// Indicates that the T part of the TLV is 8 bits.
    kTlvTypeIs8Bits = 1,

    /// Indicates that the T part of the TLV is 16 bits.
    kTlvTypeIs16Bits = 2,

    /// Indicates that the T part of the TLV is 32 bits.
    kTlvTypeIs32Bits = 4

} TlvTypeSize;

/// Used in the TlvConfig struct.
typedef enum
{
    /// Indicates that the L part of the TLV is 8 bits.
    kTlvLengthIs8Bits = 1,

    /// Indicates that the L part of the TLV is 16 bits.
    kTlvLengthIs16Bits = 2,

    /// Indicates that the L part of the TLV is 32 bits.
    kTlvLengthIs32Bits = 4

} TlvLengthSize;

/// Used in the TlvConfig struct.
typedef enum
{
    /// Indicates that the length field of the TLV does not include the length
    /// of the T and L.
    kTlvLengthExcludesTL = 0,

    /// Indicates that the length field of the TLV includes the length of the
    /// T and L.
    kTlvLengthIncludesTL = 1

} TlvLengthAttribute;

/// Used in the TlvConfig struct.
typedef enum
{
    /// Indicates that the length field of the TLV can be 0, meaning there is no
    /// Value field in the TLV.
    kTlvLength0Allowed = 0,

    /// Indicates that the length field of the TLV can not be 0.
    kTlvLength0Illegal

} TlvLengthCanBe0;

/// Used in the TlvConfig struct.
typedef enum
{
    /// When generating a TLV containing a string, this tells the generator
    /// to include the \0 terminator in the TLV.
    kTlvStringIncludes0 = 0,
    
    /// When generating a TLV string, this tells it to exclude the \0 terminator
    /// from the TLV.
    kTlvStringExcludes0

} TlvStringAttribute;

/// This is the new method for passing configuration information to the TLV
/// parser/generator engine.  Previously, these were handled as separate
/// parameters to the API functions.  This more convenient packaging makes the
/// API less clunky, and makes it easier to add more parameters in the future.
///
/// In fact, the first change is to add the option of having a TLV Type code
/// that is larger than 8 bits (this is useful for DHCPv6).  We have also
/// added the ability to deal with a TLV Length field of 32 bits.
///
/// \note The legacy API (which has config parameters specified separately)
///       still exists.  They simply create this structure on the stack, fill
///       it in, and call the new API.
///
/// It's amazing how many different permutations of TLV things there are.
/// These parameters allow the TLV engine to accommodate all of the ones we
/// know of and care about.  The great thing about standards is that there are
/// so many of them to choose from.
typedef struct
{
    /// Causes the parser/generator to use 1, 2, or 4 bytes for the TLV Type
    /// code.
    TlvTypeSize fTlvTypeSize;

    /// Causes the parser/generator to use 1, 2, or 4 bytes for the TLV Length
    /// field.
    TlvLengthSize fTlvLengthSize;

    /// Determines whether the parser/generator includes the size T and L values
    /// in the TLV Length field.
    TlvLengthAttribute fTlvLengthAttribute;

    /// This is only used in the parser; Length value read from the TLV is
    /// multiplied by this value before comparing it to the expected length
    /// specified in the TLV table.  You will normally want to set this to 1.
    ///
    /// \note This is applied to all TLVs that are parsed from the octet buffer.
    ///       It is not intended to be used as a per-TLV kind of thing.
    ///
    /// \note Setting this to 0 will causes a value of 1 to be used.
    unsigned int fLengthMultiplier;

    /// Determines whether an L value of 0 is considered valid.
    TlvLengthCanBe0 fLength0Valid;

    /// Determines whether strings include the \0 terminator or not.  DHCPv6
    /// excludes it, most others seem include it.
    ///
    /// \note This only impacts the storage of the string value in the TLV.
    ///       The string you pass in must still have a \0, and the value parsed
    ///       out will have a \0 appended.
    ///
    /// This setting is ignored for non-string related operations.
    TlvStringAttribute fTlvString;

    /// For verbose debug output.  If non-NULL, debug messages will be
    /// generated using this object to control the output (you must enable
    /// info/warning/error severities in order to get fully verbose output).
    /// This allows re-entrant use of the TLV engine by multiple clients, and
    /// allows only relevant debug info to be displayed (rather than debug
    /// output for all possible uses of the engine, which produces far too much
    /// information to be useful).
    ///
    /// If this is NULL, the TLV engine has one of its own that will be used
    /// to output warning/error messages.
    BcmMessageLogSettings *pfMessageLogSettings;

} TlvConfig;

//********************** Global Constants ************************************

/// Use this as the length field in the TLVTable if you don't want length
/// validation to be done for this type during parsing.
#define kUnknownLength 0

/// Use this as the length field in the TLVTable for a kTLV_IpAddress type
/// if you want to accept either IPv4 or IPv6 addresses.  If you only want
/// one type or the other, use 4 for IPv4, and 16 for IPv6.
#define kIpv4OrIpv6Length 0xfffffffe

/// Define kEndOfDataMarker if it has not been already.  This is also
/// defined in TLVCodes.h, but that is application specific so we would
/// like to avoid using it here.
#ifndef kEndOfDataMarker
#define kEndOfDataMarker 255
#endif

/// This is the TLVTable entry that must be the last item in every table.  Note
/// that I would have preferred to use the const form that is commented out, but
/// the stupid Microsoft compiler considers it an error, so I have to use the
/// defined version.
///const TLVTable kEndOfTLVTable = {kEndOfDataMarker, kUnknownLength, kTLV_end_of_data, (void *) 0, false, false, "Unknown", NULL};
#define kEndOfTLVTable {kEndOfDataMarker, kUnknownLength, kTLV_end_of_data, {(void *) 0}, false, false, "Unknown", NULL}

/// PR12948 - this is needed to handle a new DOCSIS 3.0 requirement where
/// unrecognized TLVs from the config file must be passed up in the REG-REQ.
/// This feature is generically useful as part of the TLV parser (not really
/// DOCSIS-specific), so it's being included here rather than in a DOCSIS
/// header file.
/// 
/// If you put this at the end of your TLV table (instead of kEndOfTLVTable),
/// the TLV parser will stop and return kTLVParserStopParsingFound when it comes
/// across an unrecognized TLV type.  Your code (usually a switch statement
/// inside of a while loop) must have a "default:" or "else" branch in order to
/// catch the unrecognized TLV.
#define kEndOfTLVTableStopParsing {kEndOfDataMarker, kUnknownLength, kTLV_end_of_data, {(void *) 0}, true, false, "Unknown", NULL}

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


/** \name The new TLV engine API

The TLV generation/parsing API has been updated to reduce the number of
parameters that need to be individually specified, and to make it easier
to add new options/values in the future.  Now, clients that use this code
can define/configure their desired TLV engine config once and pass it in
to each call.  Only the parameters that actually change from one call to
the next need to be specified individually (such as type codes, values, etc).

The underlying engine has been updated to allow more flexibility.

There is a single API for parsing TLVs from an octet buffer.  Information
about the TLVs being parsed is specified in a table.

There are multiple functions for generating TLVs (writing them to the end of
an octet buffer).  These functions handle specific data types, and call a
generic function that deals with a void * pointer and a length.

The T and L values can be 8, 16, or 32 bits (each individually selectable).
The typeCode and numberOfBytes parameters are specified as 32-bit values to
allow maximum flexibility.

\note The typeCode and numberOfBytes parameters will be stored using the
      size specified in the TlvConfig parameter, discarding upper bits as
      necessary.  Thus, if the type code is 8 bits, and you specify a
      typeCode value of 257 (0x0101), a value of 1 will be stored.

In the case of (u)int16 and (u)int32, these values are assumed to be in host
byte order, and will be converted to network byte order (if necessary for
the current processor) before they are stored.  The BcmMacAddress and
BcmIpAddress types also convert themselves to network order.  Other data
types are not converted.

There are 3 parameters that control message logging/debug output (2 fields
in the TlvConfig struct, one parameter in the functions).  If your TlvConfig
provides a pointer to a BcmMessageLogSettings object that has informational
messages enabled, then these functions will display detailed information
about the TLVs as they are generated/parsed.  Along with the settings
instance, you can provide a text name of the TLV, and a type hint for the
value, which are used as part of the message log display.

If you don't provide a BcmMessageLogSettings instance, then these functions
will only display error and warning messages, and will use a default
settings instance associated with this module.

Here is a sample of how to set up a TlvConfig struct for basic operation:

\code
void MyFunc(void)
{
    TlvConfig config;

    TlvConfigInit(config);

    // Modify config parameters to suit your needs.

    // Call GenerateTLV() or ParseTLVs() using the config struct.
}
\endcode
*/

//@{

/// This is the generic prototype for the set of functions that can convert an
/// arbitrary value in network order (along with its associated type code) to
/// binary TLV format in an octet array.
///
/// This function takes a pointer to a value, as well as the number of bytes
/// that are in the value.  We can only hope that you have given us the correct
/// number of bytes; there's nothing we can do to validate it.  We do check to
/// make sure that the value pointer isn't NULL.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      pValue - pointer to the value to be stored.
/// \param
///      numberOfBytes - the number of valid bytes in pValue.
/// \param
///      config - the TLV generation engine configuration parameters to be used.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
/// \param
///      valueType - also for message logging; a hint as to what the real type of
///                  the value is.  I use this to display "String", "MacAddress", etc.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                         const void *pValue, uint32 numberOfBytes,
                         const TlvConfig &config,
                         const char *pTlvName = NULL,
                         TLV_ValueEncoding valueType = kTLV_FixedByteArray);

/// This one handles a string value.  The length of the string is calculated for
/// you.  Byte ordering is not changed.  The \0 string terminator is included
/// in the value that is stored.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      pStringValue - pointer to the value to be stored.
/// \param
///      config - the TLV generation engine configuration parameters to be used.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                         const char *pStringValue,
                         const TlvConfig &config,
                         const char *pTlvName = NULL);

/// This one handles a uint8 value.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      value -  the value to be stored.
/// \param
///      config - the TLV generation engine configuration parameters to be used.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                uint8 value, 
                                const TlvConfig &config,
                                const char *pTlvName = NULL);

/// This one handles a uint16 value.  The bytes are swapped to network order
/// (if appropriate for the processor).
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      value -  the value to be stored.
/// \param
///      config - the TLV generation engine configuration parameters to be used.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                uint16 value, 
                                const TlvConfig &config,
                                const char *pTlvName = NULL);

/// This one handles a uint32 value.  The bytes are swapped to network order
/// (if appropriate for the processor).
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      value -  the value to be stored.
/// \param
///      config - the TLV generation engine configuration parameters to be used.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                uint32 value, 
                                const TlvConfig &config,
                                const char *pTlvName = NULL);

/// This one handles an int8 value.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      value - the value to be stored.
/// \param
///      config - the TLV generation engine configuration parameters to be used.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                int8 value, 
                                const TlvConfig &config,
                                const char *pTlvName = NULL);

/// This one handles an int16 value.  The bytes are swapped to network order
/// (if appropriate for the processor).
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      value - the value to be stored.
/// \param
///      config - the TLV generation engine configuration parameters to be used.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                int16 value, 
                                const TlvConfig &config,
                                const char *pTlvName = NULL);

/// This one handles an int32 value.  The bytes are swapped to network order
/// (if appropriate for the processor).
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      value - the value to be stored.
/// \param
///      config - the TLV generation engine configuration parameters to be used.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                int32 value, 
                                const TlvConfig &config,
                                const char *pTlvName = NULL);

/// This one handles an OctetBuffer.  The length comes from the BytesUsed field
/// of the octet buffer.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      octetValue - the value to be stored.
/// \param
///      config - the TLV generation engine configuration parameters to be used.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmOctetBuffer &octetValue, 
                                const TlvConfig &config,
                                const char *pTlvName = NULL);

/// This one handles a MAC address.  The 6 bytes of the MAC address will be
/// written in network order.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      macAddress - the value to be stored.
/// \param
///      config - the TLV generation engine configuration parameters to be used.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmMacAddress &macAddress, 
                                const TlvConfig &config,
                                const char *pTlvName = NULL);

/// This one handles an IP address (both IPv4 and IPv6).  The bytes of the IP
/// address will be written in network order.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      ipAddress - the value to be stored.
/// \param
///      config - the TLV generation engine configuration parameters to be used.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmIpAddress &ipAddress, 
                                const TlvConfig &config,
                                const char *pTlvName = NULL);

#if (BFC_INCLUDE_DOCSIS_SUPPORT)

/// This one handles a DOCSIS 3.0 Downstream Service ID (DSID).
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode,
                                const BcmDsid &dsid, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

#endif

/// This function is the entrypoint for the TLV parsing engine.  It accepts a
/// reference to an octet buffer (along with an offset into the buffer), and a
/// TLV table.  It reads a T and L value from the buffer, and searches through
/// the table for the specified T code.  If found, and if the table entry has
/// a non-0 length value in it, then it validates that the length matches what
/// is in the table.  If the length validation passes, then it stores the value
/// in the table.  The type of the value, and how it is handled/stored depends
/// on the type hint that you provide in the table.
///
/// If the T code is not found in the table, then it silently skips over the V,
/// and then continues with the next TLV.
///
/// You can make the parser abort operation when it encounters a given type code
/// by setting the appropriate flag in the table.  When this happens, the offset
/// is "rewound" so that it refers to the T code that caused the parser to stop.
///
/// Note that, on return, the offset parameter is modified to refer to the next
/// un-parsed byte of data in the buffer.
///
/// The behavior of the parser is controlled by the TlvConfig struct.  You can
/// specify how many bytes the Type codes and Length values use (they don't have
/// to match), whether the Length value includes the size T and L fields, etc.
///
/// \param
///      buffer - reference to the buffer from which the TLVs should be parsed.
/// \param
///      offset - the offset into the buffer of the first TLV byte.  On return,
///               it contains the offset of the next un-parsed byte of data in
///               the buffer.
/// \param
///      pTLVParseTable - pointer to the first TLVTable element in the array.
/// \param
///      config - the TLV parser configuration information to be use for the
///               specified buffer and TLVTable.
///
/// \return
///      One of the TLVParserResult enum values.
///
TLVParserResult ParseTLVs(const BcmOctetBuffer &buffer, unsigned int &offset, 
                          TLVTable *pTLVParseTable, const TlvConfig &config);

/// Initializes all fields in the specified TlvConfig struct with reasonable
/// default values.  You can then tweak individual fields to suit your fancy.
///
/// \param
///     config - the configuration structure to initialize.
///
void TlvConfigInit(TlvConfig &config);

/// Initializes all fields in the struct with values specified (defaults for
/// others).  This is most commonly used by the legacy API to convert to the
/// new API.
///
/// \param
///     config - the configuration structure to initialize.
/// \param
///     pMessageLogSettings - the message log settings object you want to use
///                           to control debug output.  This can be NULL.
/// \param
///     lengthIsUint16 - set this to true if the length field of the TLV is
///                      16 bits, or false if it is 8 bits.
/// \param
///     lengthIncludesTypeAndLength - set this to true if the length field of
///                                   the TLV includes the length of the T and
///                                   L fields, or false if it only includes
///                                   the number of bytes in the value.
///
void TlvConfigInit(TlvConfig &config,
                   BcmMessageLogSettings *pMessageLogSettings,
                   bool lengthIsUint16,
                   bool lengthIncludesTypeAndLength);

//@}

/* \name The legacy TLV engine API

These methods are part of the legacy API.  All new code should be written
to use the new API instead of this, and existing code should be updated to
use the new API.  These functions may be removed in the future.

All of these methods have been made inline, and they simply convert the
configuration parameters to a TlvConfig struct on the stack, calling the
new API to get the job done.
*/

//@{


/// Packages up the relevant parameters in a TlvConfig struct and calls the
/// equivalent new API.
///
/// \deprecated This version of the function will likely be removed at some
///             point.  Please modify existing code to use the new version.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      pValue - pointer to the value to be stored.
/// \param
///      numberOfBytes - the number of valid bytes in pValue.
/// \param
///      pMessageLogSettings - pointer to the message log settings instance to
///                            be used when outputting message log info.  If NULL,
///                            then it will use its own instance.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
/// \param
///      valueType - also for message logging; a hint as to what the real type of
///                  the value is.  I use this to display "String", "MacAddress", etc.
/// \param
///      lengthIsUint16 - tells me how many bytes are to be used for the Length
///                       field of the TLV.  If false, this will be the normal
///                       uint8 representation; if true, it will be a uint16.
/// \param
///      lengthIncludesTypeAndLength - length field includes the type and length itself
///                       in its value.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const void *pValue, uint32 numberOfBytes,
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                TLV_ValueEncoding valueType = kTLV_FixedByteArray,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

/// Packages up the relevant parameters in a TlvConfig struct and calls the
/// equivalent new API.
///
/// \deprecated This version of the function will likely be removed at some
///             point.  Please modify existing code to use the new version.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      pStringValue - pointer to the value to be stored.
/// \param
///      pMessageLogSettings - pointer to the message log settings instance to
///                            be used when outputting message log info.  If NULL,
///                            then it will use its own instance.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
/// \param
///      lengthIsUint16 - tells me how many bytes are to be used for the Length
///                       field of the TLV.  If false, this will be the normal
///                       uint8 representation; if true, it will be a uint16.
/// \param
///      lengthIncludesTypeAndLength - length field includes the type and length itself
///                       in its value.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const char *pStringValue, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

/// Packages up the relevant parameters in a TlvConfig struct and calls the
/// equivalent new API.
///
/// \deprecated This version of the function will likely be removed at some
///             point.  Please modify existing code to use the new version.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      value - the value to be stored.
/// \param
///      pMessageLogSettings - pointer to the message log settings instance to
///                            be used when outputting message log info.  If NULL,
///                            then it will use its own instance.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
/// \param
///      lengthIsUint16 - tells me how many bytes are to be used for the Length
///                       field of the TLV.  If false, this will be the normal
///                       uint8 representation; if true, it will be a uint16.
/// \param
///      lengthIncludesTypeAndLength - length field includes the type and length itself
///                       in its value.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                uint8 value, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

/// Packages up the relevant parameters in a TlvConfig struct and calls the
/// equivalent new API.
///
/// \deprecated This version of the function will likely be removed at some
///             point.  Please modify existing code to use the new version.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      value - the value to be stored.
/// \param
///      pMessageLogSettings - pointer to the message log settings instance to
///                            be used when outputting message log info.  If NULL,
///                            then it will use its own instance.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
/// \param
///      lengthIsUint16 - tells me how many bytes are to be used for the Length
///                       field of the TLV.  If false, this will be the normal
///                       uint8 representation; if true, it will be a uint16.
/// \param
///      lengthIncludesTypeAndLength - length field includes the type and length itself
///                       in its value.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                uint16 value, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

/// Packages up the relevant parameters in a TlvConfig struct and calls the
/// equivalent new API.
///
/// \deprecated This version of the function will likely be removed at some
///             point.  Please modify existing code to use the new version.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      value - the value to be stored.
/// \param
///      pMessageLogSettings - pointer to the message log settings instance to
///                            be used when outputting message log info.  If NULL,
///                            then it will use its own instance.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
/// \param
///      lengthIsUint16 - tells me how many bytes are to be used for the Length
///                       field of the TLV.  If false, this will be the normal
///                       uint8 representation; if true, it will be a uint16.
/// \param
///      lengthIncludesTypeAndLength - length field includes the type and length itself
///                       in its value.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                uint32 value, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

/// Packages up the relevant parameters in a TlvConfig struct and calls the
/// equivalent new API.
///
/// \deprecated This version of the function will likely be removed at some
///             point.  Please modify existing code to use the new version.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      value - the value to be stored.
/// \param
///      pMessageLogSettings - pointer to the message log settings instance to
///                            be used when outputting message log info.  If NULL,
///                            then it will use its own instance.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
/// \param
///      lengthIsUint16 - tells me how many bytes are to be used for the Length
///                       field of the TLV.  If false, this will be the normal
///                       uint8 representation; if true, it will be a uint16.
/// \param
///      lengthIncludesTypeAndLength - length field includes the type and length itself
///                       in its value.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                int8 value, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

/// Packages up the relevant parameters in a TlvConfig struct and calls the
/// equivalent new API.
///
/// \deprecated This version of the function will likely be removed at some
///             point.  Please modify existing code to use the new version.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      value - the value to be stored.
/// \param
///      pMessageLogSettings - pointer to the message log settings instance to
///                            be used when outputting message log info.  If NULL,
///                            then it will use its own instance.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
/// \param
///      lengthIsUint16 - tells me how many bytes are to be used for the Length
///                       field of the TLV.  If false, this will be the normal
///                       uint8 representation; if true, it will be a uint16.
/// \param
///      lengthIncludesTypeAndLength - length field includes the type and length itself
///                       in its value.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                int16 value, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

/// Packages up the relevant parameters in a TlvConfig struct and calls the
/// equivalent new API.
///
/// \deprecated This version of the function will likely be removed at some
///             point.  Please modify existing code to use the new version.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      value - the value to be stored.
/// \param
///      pMessageLogSettings - pointer to the message log settings instance to
///                            be used when outputting message log info.  If NULL,
///                            then it will use its own instance.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
/// \param
///      lengthIsUint16 - tells me how many bytes are to be used for the Length
///                       field of the TLV.  If false, this will be the normal
///                       uint8 representation; if true, it will be a uint16.
/// \param
///      lengthIncludesTypeAndLength - length field includes the type and length itself
///                       in its value.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                int32 value, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

/// Packages up the relevant parameters in a TlvConfig struct and calls the
/// equivalent new API.
///
/// \deprecated This version of the function will likely be removed at some
///             point.  Please modify existing code to use the new version.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      octetValue - the value to be stored.
/// \param
///      pMessageLogSettings - pointer to the message log settings instance to
///                            be used when outputting message log info.  If NULL,
///                            then it will use its own instance.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
/// \param
///      lengthIsUint16 - tells me how many bytes are to be used for the Length
///                       field of the TLV.  If false, this will be the normal
///                       uint8 representation; if true, it will be a uint16.
/// \param
///      lengthIncludesTypeAndLength - length field includes the type and length itself
///                       in its value.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmOctetBuffer &octetValue, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

/// This one handles a BcmBasicString.
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmBasicString &basicString, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

/// Packages up the relevant parameters in a TlvConfig struct and calls the
/// equivalent new API.
///
/// \deprecated This version of the function will likely be removed at some
///             point.  Please modify existing code to use the new version.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      macAddress - the value to be stored.
/// \param
///      pMessageLogSettings - pointer to the message log settings instance to
///                            be used when outputting message log info.  If NULL,
///                            then it will use its own instance.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
/// \param
///      lengthIsUint16 - tells me how many bytes are to be used for the Length
///                       field of the TLV.  If false, this will be the normal
///                       uint8 representation; if true, it will be a uint16.
/// \param
///      lengthIncludesTypeAndLength - length field includes the type and length itself
///                       in its value.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmMacAddress &macAddress, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

/// Packages up the relevant parameters in a TlvConfig struct and calls the
/// equivalent new API.
///
/// \deprecated This version of the function will likely be removed at some
///             point.  Please modify existing code to use the new version.
///
/// \param
///      buffer - reference to the OctetBuffer where the TLV bytes should be
///               placed.  The bytes are appended to the end.
/// \param
///      typeCode - the type code for the T portion of the TLV.
/// \param
///      ipAddress - the value to be stored.
/// \param
///      pMessageLogSettings - pointer to the message log settings instance to
///                            be used when outputting message log info.  If NULL,
///                            then it will use its own instance.
/// \param
///      pTlvName - pointer to an optional string containing the name of the TLV;
///                 this is only used for message logging purposes.
/// \param
///      lengthIsUint16 - tells me how many bytes are to be used for the Length
///                       field of the TLV.  If false, this will be the normal
///                       uint8 representation; if true, it will be a uint16.
/// \param
///      lengthIncludesTypeAndLength - length field includes the type and length itself
///                       in its value.
///
/// \return
///      The number of bytes that were consumed (including the bytes for the
///      T and L parts of the TLV).  If there was an error, then it returns 0.
///
inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmIpAddress &ipAddress, 
                                BcmMessageLogSettings *pMessageLogSettings = NULL,
                                const char *pTlvName = NULL,
                                bool lengthIsUint16 = false,
                                bool lengthIncludesTypeAndLength = false);

/// This is the legacy API for the TLV parsing engine.  It has been made inline;
/// it simply creates a TlvConfig struct on the stack, fills in the fields,
/// and calls the new API.
///
/// \deprecated This version of the function will likely be removed at some
///             point.  Please modify existing code to use the new version.
///
/// \param
///      buffer - reference to the buffer from which the TLVs should be parsed.
/// \param
///      offset - the offset into the buffer of the first TLV byte.
/// \param
///      pTLVParseTable - pointer to the first TLVTable element in the array.
/// \param
///      pMessageLogSettings - pointer to the message log settings instance to
///                            be used when outputting message log info.  If NULL,
///                            then it will use its own instance.
/// \param
///      lengthIsUint16 - tells me how many bytes are to be used for the Length
///                       field of the TLV.  If false, this will be the normal
///                       uint8 representation; if true, it will be a uint16.
/// \param
///      lengthIncludesTypeAndLength - length field includes the type and length itself
///                       in its value.
///
/// \param
///      lengthMultiplier - multiplier if the Length field of all TLVs in the
///                         buffer are in units of multiple bytes
///
/// \return
///      One of the TLVParserResult enum values.
///
inline TLVParserResult ParseTLVs(const BcmOctetBuffer &buffer, 
                                 unsigned int &offset,
                                 TLVTable *pTLVParseTable, 
                                 BcmMessageLogSettings *pMessageLogSettings = NULL,
                                 bool lengthIsUint16 = false,
                                 bool lengthIncludesTypeAndLength = false,
                                 unsigned int lengthMultiplier = 1);

//@}

/// This function scans the specified TLV table, looking for the specified type
/// code.  If found, the function then sets the 2 pointer members (the value
/// pointer and the was specified bool pointer) to the addresses specified.
///
/// Note that either (or both) of the pointers can be NULL if you aren't
/// interested in storing the results.
///
/// This is a helpful function that eliminates the need to keep track of the
/// index of a given member in the table, and is necessary because the compiler
/// doesn't allow you to initialize the pointers when you declare the tables.
///
/// \param
///      pTLVTable - pointer to the table to scan.
/// \param
///      type - the type code to look for.
/// \param
///      pValue - the address to be placed in the union pointer set.
/// \param
///      pValueWasSpecified - the address to be placed in the was-specified bool
///                           pointer member.
///
/// \retval
///      true if the type code was found and the addresses were stored.
/// \retval
///      false if the table entry wasn't found.
///
bool SetTLVTableEntryAddresses(TLVTable *pTLVTable, uint32 type, void *pValue,
                               bool *pValueWasSpecified);

/// This function scans the specified TLV table, looking for the specified type
/// code.  If found, it returns the contents of the wasFound member.
///
/// This is a helpful function that eliminates the need to keep track of the
/// index of a given member in the table.
///
/// \param
///      pTLVTable - pointer to the table to scan.
/// \param
///      type - the type code to look for.
///
/// \retval
///      true if the wasFound member of the table entry was set.
/// \retval
///      false if the table entry wasn't found, or if the wasFound member was
///          false.
///
bool GetTLVTableWasFound(TLVTable *pTLVTable, uint32 type);

/// Regression tests for this module.  These are all of the tests that I
/// used for validation when developing the module, examples of typical
/// usage scenarios, and whatever additional tests that were added when
/// bugs were found and fixed.
///
/// The body of this method is normally compiled out; edit the .cpp file
/// and enable the REGRESSION_TEST define to build it in to your image.
///
/// This code also serves as a pretty comprehensive set of examples showing
/// how this class can/should be used.  Any usage scenario not shown here
/// is not supported, and may not work if this class changes.
///
/// \note If you don't see your usage scenario in the regression suite,
///       go ahead and add it!  This serves a number of purposes:
///         - It shows the original designer/implementer how the class is
///           being used, which can lead to design improvements (new helper
///           methods that make things easier, etc).
///         - It provides more examples to other programmers that can be
///           copied and reused, rather than re-inventing the wheel.
///         - It ensures that code changes to this class won't break your
///           usage scenario.  Code changes are valid only if all
///           regression tests pass; if your usage scenario isn't
///           represented in the tests, then we have no way of knowing
///           whether or not the change will break your use.  This will
///           reduce the amount of debugging that you have to do, which is
///           always a good thing.
///
/// \retval
///     true if the regression tests all passed.
/// \retval
///     false if one or more regression tests failed.
///
bool RegressTlvs(void);

//********************** Inline Method Implementations ***********************


inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                uint8 value, 
                                const TlvConfig &config,
                                const char *pTlvName)
{
    return GenerateTLV(buffer, typeCode, &value, sizeof(value), config, pTlvName, kTLV_uint8);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                uint16 value, 
                                const TlvConfig &config,
                                const char *pTlvName)
{
    // Convert the value to network order.
    value = HostToNetwork(value);

    return GenerateTLV(buffer, typeCode, &value, sizeof(value), config, pTlvName, kTLV_uint16);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                uint32 value, 
                                const TlvConfig &config,
                                const char *pTlvName)
{
    // Convert the value to network order.
    value = HostToNetwork(value);

    return GenerateTLV(buffer, typeCode, &value, sizeof(value), config, pTlvName, kTLV_uint32);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                int8 value, 
                                const TlvConfig &config,
                                const char *pTlvName)
{
    return GenerateTLV(buffer, typeCode, &value, sizeof(value), config, pTlvName, kTLV_int8);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                int16 value, 
                                const TlvConfig &config,
                                const char *pTlvName)
{
    // Convert the value to network order.
    value = HostToNetwork(value);

    return GenerateTLV(buffer, typeCode, &value, sizeof(value), config, pTlvName, kTLV_int16);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                int32 value, 
                                const TlvConfig &config,
                                const char *pTlvName)
{
    // Convert the value to network order.
    value = HostToNetwork(value);

    return GenerateTLV(buffer, typeCode, &value, sizeof(value), config, pTlvName, kTLV_int32);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmOctetBuffer &octetValue, 
                                const TlvConfig &config,
                                const char *pTlvName)
{
    return GenerateTLV(buffer, typeCode, octetValue.AccessBuffer(), octetValue.BytesUsed(), config, pTlvName, kTLV_OctetBuffer);
}


inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmMacAddress &macAddress, 
                                const TlvConfig &config,
                                const char *pTlvName)
{
    uint8 octets[6];

    // Get the values from the object, then vector to the real function.
    macAddress.Get(octets[0], octets[1], octets[2], octets[3], octets[4], octets[5]);

    return GenerateTLV(buffer, typeCode, octets, sizeof(octets), config, pTlvName, kTLV_MacAddress);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmIpAddress &ipAddress, 
                                const TlvConfig &config,
                                const char *pTlvName)
{
    if (ipAddress.IsIpV6Address())
    {
        uint8 octets[BcmIpV6Address::ADDR_LEN];

        // Get the values from the object, then vector to the real function.
        ipAddress.GetIpV6Address().Get(octets);

        return GenerateTLV(buffer, typeCode, octets, sizeof(octets), config, pTlvName, kTLV_IpAddress);
    }
    else
    {
        uint8 octets[BcmIpV4Address::ADDR_LEN];

        // Get the values from the object, then vector to the real function.
        ipAddress.GetIpV4Address().Get(octets[0], octets[1], octets[2], octets[3]);

        return GenerateTLV(buffer, typeCode, octets, sizeof(octets), config, pTlvName, kTLV_IpAddress);
    }
}




inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const void *pValue, uint32 numberOfBytes,
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                TLV_ValueEncoding valueType,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    TlvConfig config;

    // Convert individual parameters to the config struct.
    TlvConfigInit(config, pMessageLogSettings, lengthIsUint16, lengthIncludesTypeAndLength);

    // Call the new-style generic version of the function.
    return GenerateTLV(buffer, typeCode, pValue, numberOfBytes, config,
                       pTlvName, valueType);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const char *pStringValue, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    TlvConfig config;

    // Convert individual parameters to the config struct.
    TlvConfigInit(config, pMessageLogSettings, lengthIsUint16, lengthIncludesTypeAndLength);

    // Call the new-style string version of the function (since it does
    // non-trivial work).
    return GenerateTLV(buffer, typeCode, pStringValue, config, pTlvName);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                uint8 value, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    // Continue to call the old-style generic version, which will process the
    // parameters and call the new-style API.
    return GenerateTLV(buffer, typeCode, &value, sizeof(value), pMessageLogSettings, pTlvName, kTLV_uint8, lengthIsUint16, lengthIncludesTypeAndLength);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                uint16 value, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    // Convert the value to network order.
    value = HostToNetwork(value);

    // Continue to call the old-style generic version, which will process the
    // parameters and call the new-style API.
    return GenerateTLV(buffer, typeCode, &value, sizeof(value), pMessageLogSettings, pTlvName, kTLV_uint16, lengthIsUint16, lengthIncludesTypeAndLength);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                uint32 value, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    // Convert the value to network order.
    value = HostToNetwork(value);

    // Continue to call the old-style generic version, which will process the
    // parameters and call the new-style API.
    return GenerateTLV(buffer, typeCode, &value, sizeof(value), pMessageLogSettings, pTlvName, kTLV_uint32, lengthIsUint16, lengthIncludesTypeAndLength);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                int8 value, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    // Continue to call the old-style generic version, which will process the
    // parameters and call the new-style API.
    return GenerateTLV(buffer, typeCode, &value, sizeof(value), pMessageLogSettings, pTlvName, kTLV_int8, lengthIsUint16, lengthIncludesTypeAndLength);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                int16 value, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    // Convert the value to network order.
    value = HostToNetwork(value);

    // Continue to call the old-style generic version, which will process the
    // parameters and call the new-style API.
    return GenerateTLV(buffer, typeCode, &value, sizeof(value), pMessageLogSettings, pTlvName, kTLV_int16, lengthIsUint16, lengthIncludesTypeAndLength);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode, 
                                int32 value, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    // Convert the value to network order.
    value = HostToNetwork(value);

    // Continue to call the old-style generic version, which will process the
    // parameters and call the new-style API.
    return GenerateTLV(buffer, typeCode, &value, sizeof(value), pMessageLogSettings, pTlvName, kTLV_int32, lengthIsUint16, lengthIncludesTypeAndLength);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmOctetBuffer &octetValue, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    // Continue to call the old-style generic version, which will process the
    // parameters and call the new-style API.
    return GenerateTLV(buffer, typeCode, octetValue.AccessBuffer(), octetValue.BytesUsed(), pMessageLogSettings, pTlvName, kTLV_OctetBuffer, lengthIsUint16, lengthIncludesTypeAndLength);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmBasicString &basicString, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    // Continue to call the old-style generic version, which will process the
    // parameters and call the new-style API.
    return GenerateTLV(buffer, typeCode, basicString.c_str(), basicString.length(), pMessageLogSettings, pTlvName, kTLV_BcmBasicString, lengthIsUint16, lengthIncludesTypeAndLength);
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmMacAddress &macAddress, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    TlvConfig config;

    // Convert individual parameters to the config struct.
    TlvConfigInit(config, pMessageLogSettings, lengthIsUint16, lengthIncludesTypeAndLength);

    // Call the new-style string version of the function (since it does
    // non-trivial work).
    return GenerateTLV(buffer, typeCode, macAddress, config, pTlvName);

/*    uint8 octets[6];

    // Get the values from the object, then vector to the real function.
    macAddress.Get(octets[0], octets[1], octets[2], octets[3], octets[4], octets[5]);

    return GenerateTLV(buffer, typeCode, octets, sizeof(octets), pMessageLogSettings, pTlvName, kTLV_MacAddress, lengthIsUint16, lengthIncludesTypeAndLength);
*/    
}

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint32 typeCode,
                                const BcmIpAddress &ipAddress, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    TlvConfig config;

    // Convert individual parameters to the config struct.
    TlvConfigInit(config, pMessageLogSettings, lengthIsUint16, lengthIncludesTypeAndLength);

    // Call the new-style string version of the function (since it does
    // non-trivial work).
    return GenerateTLV(buffer, typeCode, ipAddress, config, pTlvName);

/*
    // UNFINISHED - this will not work for IPv6!!!
    uint8 octets[4];

    // Get the values from the object, then vector to the real function.
    ipAddress.Get(octets[0], octets[1], octets[2], octets[3]);

    return GenerateTLV(buffer, typeCode, octets, sizeof(octets), pMessageLogSettings, pTlvName, kTLV_IpAddress, lengthIsUint16, lengthIncludesTypeAndLength);
*/
}


inline TLVParserResult ParseTLVs(const BcmOctetBuffer &buffer, 
                                 unsigned int &offset,
                                 TLVTable *pTLVParseTable, 
                                 BcmMessageLogSettings *pMessageLogSettings,
                                 bool lengthIsUint16,
                                 bool lengthIncludesTypeAndLength,
                                 unsigned int lengthMultiplier)
{
    TlvConfig config;

    // Convert individual parameters to the config struct.
    TlvConfigInit(config, pMessageLogSettings, lengthIsUint16, lengthIncludesTypeAndLength);

    config.fLengthMultiplier = lengthMultiplier;

    return ParseTLVs(buffer, offset, pTLVParseTable, config);
}

#if (BFC_INCLUDE_DOCSIS_SUPPORT)

inline unsigned int GenerateTLV(BcmOctetBuffer &buffer, uint8 typeCode,
                                const BcmDsid &dsid, 
                                BcmMessageLogSettings *pMessageLogSettings,
                                const char *pTlvName,
                                bool lengthIsUint16,
                                bool lengthIncludesTypeAndLength)
{
    uint8 octets[3] = { 0, 0, 0 };

    // Get the values from the object, then vector to the real function.
    dsid.Get(octets[0], octets[1], octets[2]);

    return GenerateTLV(buffer, typeCode, octets, sizeof(octets), pMessageLogSettings, pTlvName, kTLV_DOCS3_DSID, lengthIsUint16, lengthIncludesTypeAndLength);
}

#endif


#endif


