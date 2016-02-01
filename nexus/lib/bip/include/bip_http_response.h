/******************************************************************************
 * (c) 2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "bip.h"

#ifndef BIP_HTTP_RESPONSE_H
#define BIP_HTTP_RESPONSE_H

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup bip_http_response
 *
 * BIP_HttpResponse Interface Definition.
 *
 * This class is responsible for creating, manipulating, and accessing HTTP Response
 * messages and their components (status-code, reason-phrase, headers, etc.).
 *
**/

typedef struct BIP_HttpResponse *BIP_HttpResponseHandle;   /*!< HttpResponse object handle. */

/**
Summary:
Structure for storing the HTTP major and minor version numbers.

See Also:
BIP_HttpResponse_Create
BIP_HttpResponse_CreateSettings
BIP_HttpResponse_SetHttpVersion
BIP_HttpResponse_GetHttpVersion
**/
typedef struct BIP_HttpResponseHttpVersion
{
    int major;
    int minor;
} BIP_HttpResponseHttpVersion;

#define BIP_HttpResponseVersionIs_1_0( pVer ) ((( pVer )->major==1 && ( pVer )->minor==0 ) ? true : false )
#define BIP_HttpResponseVersionIs_1_1( pVer ) ((( pVer )->major==1 && ( pVer )->minor==1 ) ? true : false )

#define BIP_HTTP_RESPONSE_VERSION_PRINTF_FMT "HTTP/%d.%d"
#define BIP_HTTP_RESPONSE_VERSION_PRINTF_ARG( pVer ) ( pVer )->major, ( pVer )->minor

/**
Summary:
Settings for BIP_HttpResponse_Create().

See Also:
BIP_HttpResponse_GetDefaultCreateSettings
BIP_HttpResponse_Create
**/
typedef struct BIP_HttpResponseCreateSettings
{
    BIP_HttpResponseHttpVersion httpVersion;
    BIP_SETTINGS( BIP_HttpResponseCreateSettings )         /*!< Internal use... for init verification. */
} BIP_HttpResponseCreateSettings;
BIP_SETTINGS_ID_DECLARE( BIP_HttpResponseCreateSettings );

/**
Summary:
Get default settings for BIP_HttpResponse_Create().

See Also:
BIP_HttpResponseCreateSettings
BIP_HttpResponse_Create
**/
#define BIP_HttpResponse_GetDefaultCreateSettings( pSettings )                  \
    BIP_SETTINGS_GET_DEFAULT_BEGIN( pSettings, BIP_HttpResponseCreateSettings ) \
    /* Set non-zero defaults explicitly. */                                     \
    ( pSettings )->httpVersion.major = 1;                                       \
    ( pSettings )->httpVersion.minor = 1;                  /* "HTTP/1.1" */     \
    BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Create an empty HttpResponse object.

Description:
Allocates resources that are needed by an BIP HttpResponse.
A single BIP_HttpResponse object can be sequentially reused to serialize or
serialize multiple HTTP Responses without having to be Destroyed and re-Created.

Return:
    non-NULL :        The handle of the new HttpResponse to be used for calling subsequent HttpResponse related APIs.
Return:
    NULL     :        Failure, an HttpHeader instance could not be created.

See Also:
BIP_HttpResponse_GetDefaultCreateSettings
BIP_HttpResponseCreateSettings
BIP_HttpResponse_Destroy
**/
BIP_HttpResponseHandle BIP_HttpResponse_Create(
    void                           *ownerContext,          /*!< [in] Any pointer, but the same pointer will be needed for destroying the Response. */
    BIP_HttpResponseCreateSettings *pCreateSettings        /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    );

/**
Summary:
Destroy a BIP HttpResponse instance

Return:
    void :

See Also:
BIP_HttpResponse_Create
**/
void BIP_HttpResponse_Destroy(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in] Handle of BIP_HttpResponse to be destroyed. */
    void                  *ownerContext                    /*!< [in] Same ownerContext used to Create the HeaderList. */
    );

/**
Summary:
Settings for BIP_HttpResponse_Print().

See Also:
BIP_HttpResponse_GetDefaultPrintSettings
BIP_HttpResponse_Print
**/
typedef struct BIP_HttpResponsePrintSettings
{
    BIP_SETTINGS( BIP_HttpResponsePrintSettings )          /*!< Internal use... for init verification. */
} BIP_HttpResponsePrintSettings;
BIP_SETTINGS_ID_DECLARE( BIP_HttpResponsePrintSettings );

/**
Summary:
Get default settings for BIP_HttpResponse_Print().

See Also:
BIP_HttpResponsePrintSettings
BIP_HttpResponse_Print
**/
#define BIP_HttpResponse_GetDefaultPrintSettings( pSettings )                  \
    BIP_SETTINGS_GET_DEFAULT_BEGIN( pSettings, BIP_HttpResponsePrintSettings ) \
    /* Set non-zero defaults explicitly. */                                    \
    BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Print an HttpResponse object.

Description:
Intended for debugging purposes, this prints the current contents of an
BIP_HttpResponse object.

Return:
    BIP_SUCCESS           : The HttpResponse has been printed.

See Also:
BIP_HttpResponse_GetDefaultPrintSettings
BIP_HttpResponsePrintSettings
**/
BIP_Status BIP_HttpResponse_Print(
    BIP_HttpResponseHandle         hResponse,              /*!< [in] Handle of BIP_HttpResponse to be printed. */
    const char                    *pLineHeading,           /*!< [in] If non-NULL, will be prefixed to each line printed. */
    BIP_HttpResponsePrintSettings *pSettings               /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    );

/**
Summary:
Settings for BIP_HttpResponse_Clear().

See Also:
BIP_HttpResponse_GetDefaultClearSettings
BIP_HttpResponse_Clear
**/
typedef struct BIP_HttpResponseClearSettings
{
    BIP_SETTINGS( BIP_HttpResponseClearSettings )          /*!< Internal use... for init verification. */
} BIP_HttpResponseClearSettings;
BIP_SETTINGS_ID_DECLARE( BIP_HttpResponseClearSettings );

/**
Summary:
Get default settings for BIP_HttpResponse_Clear().

See Also:
BIP_HttpResponseClearSettings
BIP_HttpResponse_Clear
**/
#define BIP_HttpResponse_GetDefaultClearSettings( pSettings )                  \
    BIP_SETTINGS_GET_DEFAULT_BEGIN( pSettings, BIP_HttpResponseClearSettings ) \
    /* Set non-zero defaults explicitly. */                                    \
    BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Clear an HttpResponse object.

Description:
Removes everything (except for the HttpVersion) from an HttpResponse, returns it to its initial state.

Return:
    void :

See Also:
BIP_HttpResponse_GetDefaultClearSettings
BIP_HttpResponseClearSettings
**/
void BIP_HttpResponse_Clear(
    BIP_HttpResponseHandle         hHttpResponse,          /*!< [in] Handle of BIP_HttpResponse to be cleared. */
    BIP_HttpResponseClearSettings *pSettings               /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    );

/**
Summary:
Attach user-defined data to an HttpResponse.

Description:
Stores a user-defined pointer in the BIP_HttpResponse object.  The stored pointer
can be accessed at a later time by calling BIP_HttpResponse_GetUserData().

Return:
    BIP_SUCCESS           : The user-data pointer has been stored into the HttpResponse.

See Also:
BIP_HttpResponse_GetUserData
**/
BIP_Status BIP_HttpResponse_SetUserData(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in] Handle of BIP_HttpResponse to be printed. */
    void                  *pUserData                       /*!< [in] Pointer to the user specific data. */
    );

/**
Summary:
Retrieve user-defined data from an HttpResponse.

Description:
Retrieve the user-defined pointer from a BIP_HttpResponse that was previous stored
by BIP_HttpResponse_SetUserData(). If no user-data pointer has been previously stored,
the caller's variable will be set to NULL.

Return:
    BIP_SUCCESS           : The HttpResponse's user-data pointer has been placed into the variable specified by ppUserData.

See Also:
BIP_HttpResponse_SetUserData
**/
BIP_Status BIP_HttpResponse_GetUserData(
    BIP_HttpResponseHandle hHttpResponse,
    void                 **ppUserData                      /*!< [out] Address of caller's variable where the user-data pointer will be placed. */
    );

/**
Summary:
Supported HTTP Response status-codes

See Also:
BIP_HttpResponse_SetStatus
BIP_HttpResponse_GetStatus
**/
typedef enum BIP_HttpResponseStatus
{
    BIP_HttpResponseStatus_e100_Continue                      = 100,
    BIP_HttpResponseStatus_e101_SwitchingProtocols            = 101,
    BIP_HttpResponseStatus_e102_Processing                    = 102,
    BIP_HttpResponseStatus_e200_OK                            = 200,
    BIP_HttpResponseStatus_e201_Created                       = 201,
    BIP_HttpResponseStatus_e202_Accepted                      = 202,
    BIP_HttpResponseStatus_e203_NonAuthoritativeInformation   = 203,
    BIP_HttpResponseStatus_e204_NoContent                     = 204,
    BIP_HttpResponseStatus_e205_ResetContent                  = 205,
    BIP_HttpResponseStatus_e206_PartialContent                = 206,
    BIP_HttpResponseStatus_e207_MultiStatus                   = 207,
    BIP_HttpResponseStatus_e208_AlreadyReported               = 208,
    BIP_HttpResponseStatus_e226_IMUsed                        = 226,
    BIP_HttpResponseStatus_e300_MultipleChoices               = 300,
    BIP_HttpResponseStatus_e301_MovedPermanently              = 301,
    BIP_HttpResponseStatus_e302_Found                         = 302,
    BIP_HttpResponseStatus_e303_SeeOther                      = 303,
    BIP_HttpResponseStatus_e304_NotModified                   = 304,
    BIP_HttpResponseStatus_e305_UseProxy                      = 305,
    BIP_HttpResponseStatus_e306_Unused                        = 306,
    BIP_HttpResponseStatus_e307_TemporaryRedirect             = 307,
    BIP_HttpResponseStatus_e308_PermanentRedirect             = 308,
    BIP_HttpResponseStatus_e400_BadRequest                    = 400,
    BIP_HttpResponseStatus_e401_Unauthorized                  = 401,
    BIP_HttpResponseStatus_e402_PaymentRequired               = 402,
    BIP_HttpResponseStatus_e403_Forbidden                     = 403,
    BIP_HttpResponseStatus_e404_NotFound                      = 404,
    BIP_HttpResponseStatus_e405_MethodNotAllowed              = 405,
    BIP_HttpResponseStatus_e406_NotAcceptable                 = 406,
    BIP_HttpResponseStatus_e407_ProxyAuthenticationRequired   = 407,
    BIP_HttpResponseStatus_e408_RequestTimeout                = 408,
    BIP_HttpResponseStatus_e409_Conflict                      = 409,
    BIP_HttpResponseStatus_e410_Gone                          = 410,
    BIP_HttpResponseStatus_e411_LengthRequired                = 411,
    BIP_HttpResponseStatus_e412_PreconditionFailed            = 412,
    BIP_HttpResponseStatus_e413_PayloadTooLarge               = 413,
    BIP_HttpResponseStatus_e414_URITooLong                    = 414,
    BIP_HttpResponseStatus_e415_UnsupportedMediaType          = 415,
    BIP_HttpResponseStatus_e416_RangeNotSatisfiable           = 416,
    BIP_HttpResponseStatus_e417_ExpectationFailed             = 417,
    BIP_HttpResponseStatus_e421_MisdirectedRequest            = 421,
    BIP_HttpResponseStatus_e422_UnprocessableEntity           = 422,
    BIP_HttpResponseStatus_e423_Locked                        = 423,
    BIP_HttpResponseStatus_e424_FailedDependency              = 424,
    BIP_HttpResponseStatus_e425_Unassigned                    = 425,
    BIP_HttpResponseStatus_e426_UpgradeRequired               = 426,
    BIP_HttpResponseStatus_e427_Unassigned                    = 427,
    BIP_HttpResponseStatus_e428_PreconditionRequired          = 428,
    BIP_HttpResponseStatus_e429_TooManyRequests               = 429,
    BIP_HttpResponseStatus_e430_Unassigned                    = 430,
    BIP_HttpResponseStatus_e431_RequestHeaderFieldsTooLarge   = 431,
    BIP_HttpResponseStatus_e500_InternalServerError           = 500,
    BIP_HttpResponseStatus_e501_NotImplemented                = 501,
    BIP_HttpResponseStatus_e502_BadGateway                    = 502,
    BIP_HttpResponseStatus_e503_ServiceUnavailable            = 503,
    BIP_HttpResponseStatus_e504_GatewayTimeout                = 504,
    BIP_HttpResponseStatus_e505_HTTPVersionNotSupported       = 505,
    BIP_HttpResponseStatus_e506_VariantAlsoNegotiates         = 506,
    BIP_HttpResponseStatus_e507_InsufficientStorage           = 507,
    BIP_HttpResponseStatus_e508_LoopDetected                  = 508,
    BIP_HttpResponseStatus_e509_Unassigned                    = 509,
    BIP_HttpResponseStatus_e510_NotExtended                   = 510,
    BIP_HttpResponseStatus_e511_NetworkAuthenticationRequired = 511,

    BIP_HttpResponseStatus_e9999_Unknown                      = 9999 /* BIP's own unique status-code. For internal use only. */
} BIP_HttpResponseStatus;

/**
Summary:
Populate the "status-line" fields of the specified BIP_HttpResponse.

Description:
The "status-line" consists of the following fields:
- HTTP-version   : For example: "HTTP/1.1"       Can also be set with BIP_HttpResponse_SetHttpVersion()
- status-code    : "200", "404", etc.            Can also be set with BIP_HttpResponse_SetStatus()
- reason-phrase  : "OK", "Not Found", etc.       Can also be set with BIP_HttpResponse_SetCustomStatus()

Return:
    BIP_SUCCESS           : The HttpResponse's version has bee updated.

See Also:
BIP_HttpResponse_SetStatus
BIP_HttpResponse_SetCustomStatus
BIP_HttpResponse_SetHttpVersion
BIP_HttpResponse_GetStatusLine
**/
BIP_Status BIP_HttpResponse_SetStatusLine(
    BIP_HttpResponseHandle       hHttpResponse,            /*!< [in]  Handle of BIP_HttpResponse to be updated. */
    BIP_HttpResponseHttpVersion *pHttpVersion,             /*!< [in]  Optional pointer to a BIP_HttpReqestHttpVersion structure. */
                                                           /*!<       Overrides value specified in BIP_HttpResponseCreateSettings.httpVersion */
                                                           /*!<       if specified here. */
    const char                  *pStatusCodeString,        /*!< [in] Optional response status code.  If NULL, then value */
                                                           /*!<       from statusEnum will be used. */
    BIP_HttpResponseStatus       statusEnum,               /*!< [in]  Optional response status.  Only used if pStatusCodeString is NULL.*/
    const char                  *pReasonPhraseString       /*!< [in]  Optional response reason-phrase.  If NULL, then a matching */
                                                           /*!<       reason-phrase will be automatically selected, if possible. */
    );

/**
Summary:
Retrieve the values of the "status-line" fields of the specified BIP_HttpResponse.

Description:
The "status-line" consists of the following fields:
- HTTP-version   : For example: "HTTP/1.1"       Can also be retrieved with BIP_HttpResponse_GetHttpVersion()
- status-code    : "200", "404", etc.            Can also be retrieved with BIP_HttpResponse_GetStatus()
- reason-phrase  : "OK", "Not Found", etc.

Return:
    BIP_SUCCESS           : The HttpResponse's fields have been returned.

See Also:
BIP_HttpResponse_GetStatus
BIP_HttpResponse_GetHttpVersion
BIP_HttpResponse_SetStatusLine
**/
BIP_Status BIP_HttpResponse_GetStatusLine(
    BIP_HttpResponseHandle       hHttpResponse,            /*!< [in]  Handle of BIP_HttpResponse to be accessed. */
    BIP_HttpResponseHttpVersion *pHttpVersion,             /*!< [out] Pointer to caller's BIP_HttpResponseHttpVersion structure. */
    const char                 **ppStatusCodeString,       /*!< [out] Optional address of caller's pointer to be set */
                                                           /*!<       to the null-terminated status-code. */
    BIP_HttpResponseStatus      *pStatusEnum,              /*!< [out] Address of caller's variable to receive the */
                                                           /*!<       BIP_HttpResponseStatus enum value.  */
                                                           /*!<       Unrecognized status-code values will be indicated */
                                                           /*!<       as BIP_HttpResponseStatus_eInvalid. */
    const char                 **ppReasonPhraseString      /*!< [out] Optional address of caller's pointer to be set */
                                                           /*!<       to the null-terminated reason-phrase. */
    );

/**
Summary:
Sets the HTTP status-code (and a corresponding reason-phrase) into an HttpResponse
object, overwriting any previous status-code and reason-phrase that may have been previously set.

Description:
Updates the status-code and reason-phrase for the specified HttpResponse object.
The Response's "reason-phrase" field will be set to a phrase that corresponds to
the specified responseStatus.

Return:
    BIP_SUCCESS           : The HttpResponse has been updated with the responseStatus.

See Also:
BIP_HttpResponse_SetCustomStatus
BIP_HttpResponse_GetStatus
**/
BIP_Status BIP_HttpResponse_SetStatus(
    BIP_HttpResponseHandle hHttpResponse,
    BIP_HttpResponseStatus responseStatus                  /*!< [in] response staus.This API will internally add status phrase.*/
    );

/**
Summary:
Sets the HTTP status-code and reason-phrase into an HttpResponse
object, overwriting any previous status-code and reason-phrase that may
have been previously set.

Description:
Updates the status-code and reason-phrase for the specified HttpResponse object to
the specified values.

Return:
    BIP_SUCCESS           : The HttpResponse has been updated with the responseStatus.

See Also:
BIP_HttpResponse_SetStatus
BIP_HttpResponse_GetStatus
**/
BIP_Status BIP_HttpResponse_SetCustomStatus(
    BIP_HttpResponseHandle hHttpResponse,
    const char            *pStatusCodeString,              /*!< [in] Response status code .*/
    const char            *pReasonPhraseString             /*!< [in] Response status phrase.*/
    );

/**
Summary:
Retrieves the HTTP status-code and reason-phrase from the specified BIP_HttpResponse
object.

Description:
Retrieves the HttpResponse's HTTP Response status-code and reason-phrase as
pointers to null-terminated strings, and also attempts to return the
corresponding BIP_HttpResponseStatus enum (if one exists).  Any unrecognized
status-code string will be indicated as BIP_HttpResponseStatus_eInvalid.

Return:
    BIP_SUCCESS           : The status-code and reason-phrase have been retrieved.

See Also:
BIP_HttpResponse_SetStatus
BIP_HttpResponse_SetCustomStatus
**/
BIP_Status BIP_HttpResponse_GetStatus(
    BIP_HttpResponseHandle  hHttpResponse,                 /*!< [in]  Handle of BIP_HttpResponse to be accessd. */
    BIP_HttpResponseStatus *pResponseStatus,               /*!< [out] Address of caller's variable to receive the BIP_HttpResponseStatus enum value. */
                                                           /*!<       If method not recognized, it will be set to BIP_HttpResponseStatus_eInvalid. */
    const char            **ppResponseStatusString,        /*!< [out] Optional address of caller's pointer to be set to the null-terminated status-code. */
    const char            **ppReasonPhraseString           /*!< [out] Optional address of caller's pointer to be set to the null-terminated reason-phrase. */
    );

/**
Summary:
Sets the HTTP version into an HttpResponse object, overwriting any previous version
that may have been previously set.

Return:
    BIP_SUCCESS           : The HttpResponse's version has been updated.

See Also:
BIP_HttpResponse_GetHttpVersion
**/
BIP_Status BIP_HttpResponse_SetHttpVersion(
    BIP_HttpResponseHandle       hHttpResponse,            /*!< [in] Handle of BIP_HttpResponse to be updated. */
    BIP_HttpResponseHttpVersion *pHttpVersion              /*!< [in] Pointer to a BIP_HttpResponseHttpVersion structure. */
    );

/**
Summary:
Retrieves the HTTP version from an HttpResponse object.

Return:
    BIP_SUCCESS           : The HTTP version has been stored in the caller's structure.

See Also:
BIP_HttpResponse_SetHttpVersion
**/
BIP_Status BIP_HttpResponse_GetHttpVersion(
    BIP_HttpResponseHandle       hHttpResponse,            /*!< [in]  Handle of BIP_HttpResponse to be accessd. */
    BIP_HttpResponseHttpVersion *pHttpVersion              /*!< [out] Pointer to caller's BIP_HttpResponseHttpVersion structure. */
    );

/**
Summary:
Add a new HttpHeader to an HttpResponse.

Description:
Creates a new HttpHeader with the specified name and value, then adds it
to the HttpResponse's list of headers at the indicated position.

Return:
    non-NULL :        The handle of the new HttpHeader that has been added to the HttpResponse's header list.
Return:
    NULL     :        Failure, an HttpHeader instance could not be created.

See Also:
BIP_HttpResponse_RemoveHeader
**/
BIP_HttpHeaderHandle BIP_HttpResponse_AddHeader(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in]  Handle of BIP_HttpResponse to updated. */
    const char            *pName,                          /*!< [in]  Pointer to a null-terminated string with the header's name. */
    const char            *pValue,                         /*!< [in]  Optional pointer to a null-terminated string that will be copied to the header's value. */
                                                           /*!<       If NULL, then the header will be created without a value. */
    BIP_HttpHeaderHandle   hAddBeforeThisHeader            /*!< [in]  If a BIP_HeaderHandle is passed, and it exists in the specified hHttpResponse, */
                                                           /*!<       the newly created header will be positioned immediately in front of it. */
                                                           /*!<       If NULL, the new header will be placed at the end of the HttpResponse's headers. */
    );

/**
Summary:
Remove (and destroy) an HttpHeader from an HttpResponse.

Description:
The indicated HttpHeader is destroyed after being removed from the specified HttpResponse's list of headers.

Return:
    void :

See Also:
BIP_HttpResponse_AddHeader
**/
void BIP_HttpResponse_RemoveHeader(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in]  Handle of BIP_HttpResponse to be updated. */
    BIP_HttpHeaderHandle   hHttpHeader                     /*!< [in]  Handle of the Header to be removed/destroyed. */
    );

/**
Summary:
Get the first or next HttpHeader following a given Header in the specified HttpResponse's list of headers.  Optionally,
a header name can be specified to return the first or next header with the specified name.

Description:
Starting from a given HttpHeader, or from the start of the HttpResponses's header list,
return the handle of the next Header, optionally, with a specified name.

Return:
    BIP_SUCCESS           : The handle of the next HttpHeader with the given name has been placed in the variable referenced by phNextHeader.
Return:
    BIP_INF_NOT_AVAILABLE : There are no Headers with the specified name following hCurrentHeader.
                            The variable referenced by phNextHeader has been set to NULL.

See Also:
BIP_HttpResponse_GetHeaderNameValue
**/
BIP_Status BIP_HttpResponse_GetNextHeader(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in]  Handle of the BIP_HttpResponse to be accessed. */
    BIP_HttpHeaderHandle   hCurrentHeader,                 /*!< [in]  The Header from which to start.  Pass NULL to start from the beginning of the list. */
    const char            *pName,                          /*!< [in]  If non-NULL, then return the next Header matching this name. */
    BIP_HttpHeaderHandle  *phNextHeader,                   /*!< [out] Address of variable to receive the handle of the next header with the specified name. */
    const char           **ppNextHeaderValue               /*!< [out] Optional address of pointer that will be point to the header's value. */
                                                           /*!<       Pass NULL if not interested in the header's value. */
    );

/**
Summary:
Move an HttpHeader and place it before a specified HttpHeader.

Description:
Moves an HttpHeader to a different position in the HttpResponse's header list, by placing it in front of another
specified header, or at the end of the list.

Return:
    BIP_SUCCESS           : The Header was moved as requested.
**/
BIP_Status BIP_HttpResponse_MoveHeader(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in]  Handle of the BIP_HttpResponse to be updated. */
    BIP_HttpHeaderHandle   hHeader,                        /*!< [in]  The header to be moved. */
    BIP_HttpHeaderHandle   hPutBeforeThisHeader            /*!< [in]  The Header that hHeader will moved in front of. */
                                                           /*!<       If NULL, then hHeader will be placed at the end of the HttpResponse's header list. */
    );

/**
Summary:
Get the name and value of an HttpHeader.

Description:
Returns pointers to the header's null-terminated name and value strings.

Return:
    BIP_SUCCESS           : The address of the Header's name was returned.

See Also:
BIP_HttpResponse_SetHeaderValue
**/
BIP_Status BIP_HttpResponse_GetHeaderNameValue(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in]  Handle of the BIP_HttpResponse to be accessed. */
    BIP_HttpHeaderHandle   hHeader,                        /*!< [in]  The header of interest. */
    const char           **ppName,                         /*!< [out] The address of a pointer that will be updated to point the Header's name as a null-terminated string. */
                                                           /*!<       This pointer will become invalid when it's Header is destroyed. */
    const char           **ppValue                         /*!< [out] The address of a pointer that will be updated to point the Header's value as a null-terminated string. */
                                                           /*!<       This pointer will become invalid when it's Header is destroyed. */
    );

/**
Summary:
Set the value of a HttpResponse's HttpHeader.

Description:
Replaces a header's value with a copy of a specified character string.

Return:
    BIP_SUCCESS       : The Header's value string has been modified as requested.

See Also:
BIP_HttpResponse_GetHeaderNameValue
**/
BIP_Status BIP_HttpResponse_SetHeaderValue(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in]  Handle of the BIP_HttpResponse to be updated. */
    BIP_HttpHeaderHandle   hHeader,                        /*!< [in]  The header that will have its value changed. */
    const char            *pValue                          /*!< [in]  A pointer to a null-terminated string containing the Header's new value. */
                                                           /*!<       Passing NULL will result in a header without a value.  */
    );

/**
Summary:
Populate a BIP_HttpResponse object from an serial HTTP response message that is
contained in one or more batom_cursors.

Description:
Refer to /BSEAV/lib/utils/bioatom.h for information on the usage of the "batom" APIs and data types.

An single HTTP response can be spread across multiple batom_cursors.  This API can be called repeatedly to
feed each of the cursors to the deserializer.  When an entire HTTP response has been deserialized, the *pDeserializeComplete
output argument will be set to true, and the cursor will be left pointing to the first character after the
HTTP response (start of payload) or the cursor will be at EOF.

Return:
    BIP_SUCCESS       : The data from the specified cursor has been deserialized.  Check *pDeserializeComplete
                        to determine completion.
Return:
BIP_ERR_SERIALIZE_IN_PROGRESS : The BIP_HttpResponse object is in the process of serializing output.
                        Either complete the serialize process by calling the appropriate
                        BIP_HttpResponse_Serialize... API until it completes or fails, or
                        call BIP_HttpResponse_Clear() to start over.

See Also:
BIP_HttpResponse_DeserializeFromBuffer
**/
BIP_Status BIP_HttpResponse_DeserializeFromAtom(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in]  The BIP_HttpResponse to be populated with the deserialized data. */
    batom_cursor          *cursor,                         /*!< [in]  Cursor that contains incoming data */
    bool                  *pDeserializeComplete            /*!< [out] Set to true if the http response is complete, */
                                                           /*!<       i.e., it has <CRNL><CRNL> or <NL><NL>. */
    );

/**
Summary:
Populate a BIP_HttpResponse object from a serial HTTP response message that
is contained in one or more memory buffers (char * arrays).

Description:
An single HTTP response can be spread across multiple buffers.  This API can be called iteratively to
feed each of the buffers to the deserializer.  When an entire HTTP response has been deserialized, the *pDeserializeComplete
output argument will be set to true, and *pConsumedBytes will indicate the position of the first character
after the HTTP response (unless *pConsumedBytes == bufferSize, which indicates that the buffer has nothing
after the end of the response).

The memory buffers must not be NULL-terminated.

Return:
    BIP_SUCCESS                : The data from the specified buffer has been deserialized.  Check *pDeserializeComplete
                                 to determine completion.
Return:
    BIP_ERR_SERIALIZE_IN_PROGRESS : The BIP_HttpResponse object is in the process of serializing output.
                                 Either complete the serialize process by calling the appropriate
                                 BIP_HttpResponse_Serialize... API until it completes or fails, or
                                 call BIP_HttpResponse_Clear() to start over.

See Also:
BIP_HttpResponse_DeserializeFromAtom
**/
BIP_Status BIP_HttpResponse_DeserializeFromBuffer(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in]  The HttpResponse to be populated with the deserialized data. */
    const char            *buffer,                         /*!< [in]  Buffer that contains incoming data. */
    size_t                 bufferSize,                     /*!< [in]  Input buffer size. */
    bool                  *pDeserializeComplete,           /*!< [out] Set to true if the http response is complete, ie it has */
                                                           /*!<       end of header(<CRNL><CRNL or <NL><NL>). */
    size_t                *pConsumedBytes                  /*!< [out] If response is complete then this specifies the consumed */
                                                           /*!<       data size. If message is not complete then this must be equal to bufferSize. */
    );

/**
Summary:
Produce a "batom" that contains a serialized version of the HttpResponse
that is suitable for sending over the network.

Description:
After constructing an HttpResponse object (by setting the Start Line and adding headers),
this API can be used convert the HttpResponse into character string to be transmitted.
The resulting character string is encapsulated by a batom and returned to the caller.

Refer to /BSEAV/lib/utils/bioatom.h for information on the usage of the "batom" APIs and data types.

The data in the batom is not NULL-terminated.

Return:
    BIP_SUCCESS                : The data from the HttpResponse has been serialized into the batom,
                                 and the batom has been returned to the caller.
Return:
    BIP_ERR_DESERIALIZE_IN_PROGRESS  : The BIP_HttpResponse object is in the process of being constructed
                                 by deserializing a received HTTP response.  The HttpResponse cannot
                                 be serialized until the deserializing is completed.
                                 Either complete the deserializing process by calling the appropriate
                                 BIP_HttpResponse_Deserialize... API until it completes or fails, or
                                 call BIP_HttpResponse_Clear() to start over.

See Also:
BIP_HttpResponse_SerializeToBuffer
**/
BIP_Status BIP_HttpResponse_SerializeToAtom(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in]  Handle of HttpResponse to be serialized. */
    batom_factory_t        factory,                        /*!< [in]  Factory to use for creating atom .   */
    batom_t               *atom                            /*!< [out] Where to put the atom containing the serialized data. */
    );

/**
Summary:
Determine the number of bytes that a BIP_HttpResponse will occupy when it is serialized (converted to
one long character string for transmission over the network).

Description:
This API allows the caller to allocate a buffer of the proper size before calling
BIP_HttpResponse_SerializeToBuffer().

Return:
    BIP_SUCCESS                : The *pSerializeBufferSize argument contains the size required for
                                 the serialized HttpResponse.
Return:
    BIP_ERR_DESERIALIZE_IN_PROGRESS : The BIP_HttpResponse object is in the process of being constructed
                                by deserializing a received HTTP response.  The HttpResponse cannot
                                be serialized until the deserializing is completed.
                                Either complete the deserializing process by calling the appropriate
                                BIP_HttpResponse_Deserialize... API until it completes or fails, or
                                call BIP_HttpResponse_Clear() to start over.

See Also:
BIP_HttpResponse_SerializeToBuffer
**/
BIP_Status BIP_HttpResponse_GetSerializeBufferSize(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in]  Handle of HttpResponse to be serialized. */
    size_t                *pSerializeBufferSize            /*!< [out] Address of variable to receive the buffer size */
                                                           /*!<       required for serializing the HttpResponse. */
    );

/**
Summary:
Populate the caller's memory buffer (char array) with a "serialized" version of the HttpResponse
that is suitable for sending over the network.

Description:
After constructing an HttpResponse object (by setting the Start Line and adding headers),
this API can be used convert the HttpResponse into character string to be transmitted.
If the caller's buffer is not large enough to hold the entire serialized HttpResponse,
this API will set the *pSerializeComplete output argument to false. The caller is then expected
to call this API repeatedly until *pSerializeComplete is true.

When the serialize is complete, the *pSerializedBytes output argument indicates the number
of bytes (of the caller's buffer) that are occupied by the serialized HttpResponse.

The data in the callers's buffer is not NULL-terminated.

Return:
    BIP_SUCCESS                : The HttpResponse has been serialized into the callers buffer.
                                 Check *pDeserializeComplete to determine completion.
Return:
    BIP_ERR_DESERIALIZE_IN_PROGRESS :  The BIP_HttpResponse object is in the process of being constructed
                                 by deserializing a received HTTP response.  The HttpResponse cannot
                                 be serialized until the deserializing is complete.
                                 Either complete the deserializing process by calling the appropriate
                                 BIP_HttpResponse_Deserialize... API until it completes or fails, or
                                 call BIP_HttpResponse_Clear() to start over.

See Also:
BIP_HttpResponse_GetSerializeBufferSize
**/
BIP_Status BIP_HttpResponse_SerializeToBuffer(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in]  Handle of the HttpResponse to be serialized. */
    char                  *pBuffer,                        /*!< [in]  Buffer to receive the serialized output. */
    size_t                 bufferSize,                     /*!< [in]  Buffer size. */
    bool                  *pSerializeComplete,             /*!< [out] Set to true if serializing is complete. Otherwise, */
                                                           /*!<       the buffer is full, call this again with a new buffer.  */
    size_t                *pSerializedBytes                /*!< [out] The number of bytes that have been placed in the buffer. */
    );

/**
Summary:
Set the "Content-Range" header for an HttpResponse.

Description:
First, any existing "Content-Range" headers will be removed
from the HttpResponse.  Then, a new "Content-Range" header,
constructed from the specified values, will be added.

Note: This API will always overwrite any existing
"Content-Range" headers.

Refer to RFC7233 Appendix D for the ABNF description of the "Content-Range" header.

Return:
    BIP_SUCCESS                : The specified "Content-Range" header has been
                                 added to the HttpResponse as requested.
Return:
    BIP_ERR_PARSE_IN_PROGRESS :
Return:
    BIP_ERR_RENDER_IN_PROGRESS :

See Also:
**/
BIP_Status BIP_HttpResponse_SetContentRangeHeaderByteRange(
    BIP_HttpResponseHandle hHttpResponse,                  /*!< [in]  Handle of the HttpResponse to be accessed. */
    uint64_t               firstBytePos,                   /*!< [in]  First byte position. Use BIP_U64_MAX to indicate an unsatisfied range. */
    uint64_t               lastBytePos,                    /*!< [in]  Last byte position. Use BIP_U64_MAX to indicate an unsatisfied range. */
    uint64_t               completeLength                  /*!< [in]  Complete length of representation. Use BIP_U64_MAX if length is unknown. */
    );

#ifdef __cplusplus
}
#endif

#endif /* BIP_HTTP_RESPONSE_H */
