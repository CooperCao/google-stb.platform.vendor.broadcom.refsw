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

#ifndef BIP_HTTP_REQUEST_H
#define BIP_HTTP_REQUEST_H

#include "bip.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup bip_http_request
 *
 * BIP_HttpRequest Interface Definition.
 *
 * This class is responsible for creating, manipulating, and accessing HTTP Request
 * messages and their components (method, request target, headers, etc.).
 *
**/

typedef struct BIP_HttpRequest *BIP_HttpRequestHandle;     /*!< HttpRequest object handle. */

/**
Summary:
Structure for storing the HTTP major and minor version numbers.

See Also:
BIP_HttpRequest_Create
BIP_HttpRequest_CreateSettings
BIP_HttpRequest_SetHttpVersion
BIP_HttpRequest_GetHttpVersion
**/
typedef struct BIP_HttpRequestHttpVersion
{
    int major;
    int minor;
} BIP_HttpRequestHttpVersion;

#define BIP_HttpRequestVersionIs_1_0( pVer ) ((( pVer )->major==1 && ( pVer )->minor==0 ) ? true : false )
#define BIP_HttpRequestVersionIs_1_1( pVer ) ((( pVer )->major==1 && ( pVer )->minor==1 ) ? true : false )

#define BIP_HTTP_REQUEST_VERSION_PRINTF_FMT "HTTP/%d.%d"
#define BIP_HTTP_REQUEST_VERSION_PRINTF_ARG( pVer ) ( pVer )->major, ( pVer )->minor

/**
Summary:
Settings for BIP_HttpRequest_Create().

See Also:
BIP_HttpRequest_GetDefaultCreateSettings
BIP_HttpRequest_Create
**/
typedef struct BIP_HttpRequestCreateSettings
{
    BIP_HttpRequestHttpVersion httpVersion;
    BIP_SETTINGS( BIP_HttpRequestCreateSettings )          /*!< Internal use... for init verification. */
} BIP_HttpRequestCreateSettings;
BIP_SETTINGS_ID_DECLARE( BIP_HttpRequestCreateSettings );

/**
Summary:
Get default settings for BIP_HttpRequest_Create().

See Also:
BIP_HttpRequestCreateSettings
BIP_HttpRequest_Create
**/
#define BIP_HttpRequest_GetDefaultCreateSettings( pSettings )                  \
    BIP_SETTINGS_GET_DEFAULT_BEGIN( pSettings, BIP_HttpRequestCreateSettings ) \
    /* Set non-zero defaults explicitly. */                                    \
    ( pSettings )->httpVersion.major = 1;                                      \
    ( pSettings )->httpVersion.minor = 1;                  /* "HTTP/1.1" */    \
    BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Create an empty HttpRequest object.

Description:
Allocates resources that are needed by an BIP HttpRequest.
A single BIP_HttpRequest object can be reused to serialize or deserialize multiple
HTTP Requests without having to be Destroyed and re-Created for each new request.

Return:
    non-NULL :        The handle of the new HttpRequest to be used for calling subsequent HttpRequest related APIs.
Return:
    NULL     :        Failure, an HttpHeader instance could not be created.

See Also:
BIP_HttpRequest_GetDefaultCreateSettings
BIP_HttpRequestCreateSettings
BIP_HttpRequest_Destroy
**/
BIP_HttpRequestHandle BIP_HttpRequest_Create(
    void                          *ownerContext,           /*!< [in] Any pointer, but the same pointer will be needed for destroying the Request. */
    BIP_HttpRequestCreateSettings *pCreateSettings         /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    );

/**
Summary:
Destroy a BIP HttpRequest instance

Return:
    void :

See Also:
BIP_HttpRequestCreate
**/
void BIP_HttpRequest_Destroy(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in] Handle of BIP_HttpRequest to be destroyed. */
    void                 *ownerContext                     /*!< [in] Same ownerContext used to Create the HeaderList. */
    );

/**
Summary:
Settings for BIP_HttpRequest_Print().

See Also:
BIP_HttpRequest_GetDefaultPrintSettings
BIP_HttpRequest_Print
**/
typedef struct BIP_HttpRequestPrintSettings
{
    BIP_SETTINGS( BIP_HttpRequestPrintSettings )           /*!< Internal use... for init verification. */
} BIP_HttpRequestPrintSettings;
BIP_SETTINGS_ID_DECLARE( BIP_HttpRequestPrintSettings );

/**
Summary:
Get default settings for BIP_HttpRequest_Print().

See Also:
BIP_HttpRequestPrintSettings
BIP_HttpRequest_Print
**/
#define BIP_HttpRequest_GetDefaultPrintSettings( pSettings )                  \
    BIP_SETTINGS_GET_DEFAULT_BEGIN( pSettings, BIP_HttpRequestPrintSettings ) \
    /* Set non-zero defaults explicitly. */                                   \
    BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Print an HttpRequest object.

Description:
Intended for debugging purposes, this prints the current contents of an
BIP_HttpRequest object.

Return:
    BIP_SUCCESS           : The HttpRequest has been printed.

See Also:
BIP_HttpRequest_GetDefaultPrintSettings
BIP_HttpRequestPrintSettings
**/
BIP_Status BIP_HttpRequest_Print(
    BIP_HttpRequestHandle         hRequest,                /*!< [in] Handle of BIP_HttpRequest to be printed. */
    const char                   *pLineHeading,            /*!< [in] If non-NULL, will be prefixed to each line printed. */
    BIP_HttpRequestPrintSettings *pSettings                /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    );

/**
Summary:
Settings for BIP_HttpRequest_Clear().

See Also:
BIP_HttpRequest_GetDefaultClearSettings
BIP_HttpRequest_Clear
**/
typedef struct BIP_HttpRequestClearSettings
{
    BIP_SETTINGS( BIP_HttpRequestClearSettings )           /*!< Internal use... for init verification. */
} BIP_HttpRequestClearSettings;
BIP_SETTINGS_ID_DECLARE( BIP_HttpRequestClearSettings );

/**
Summary:
Get default settings for BIP_HttpRequest_Clear().

See Also:
BIP_HttpRequestClearSettings
BIP_HttpRequest_Clear
**/
#define BIP_HttpRequest_GetDefaultClearSettings( pSettings )                  \
    BIP_SETTINGS_GET_DEFAULT_BEGIN( pSettings, BIP_HttpRequestClearSettings ) \
    /* Set non-zero defaults explicitly. */                                   \
    BIP_SETTINGS_GET_DEFAULT_END

/**
Summary:
Clear an HttpRequest object.

Description:
Removes everything (except for the HttpVersion) from an HttpRequest, returns it to its initial state.

Return:
    void :

See Also:
BIP_HttpRequest_GetDefaultClearSettings
BIP_HttpRequestClearSettings
**/
void BIP_HttpRequest_Clear(
    BIP_HttpRequestHandle         hHttpRequest,            /*!< [in] Handle of BIP_HttpRequest to be cleared. */
    BIP_HttpRequestClearSettings *pSettings                /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    );

/**
Summary:
Attach user-defined data to an HttpRequest.

Description:
Stores a user-defined pointer in the BIP_HttpRequest object.  The stored pointer
can be accessed at a later time by calling BIP_HttpRequest_GetUserData().

Return:
    BIP_SUCCESS           : The user-data pointer has been stored into the HttpRequest.

See Also:
BIP_HttpRequest_GetUserData
**/
BIP_Status BIP_HttpRequest_SetUserData(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in] Handle of BIP_HttpRequest to be printed. */
    void                 *pUserData                        /*!< [in] Pointer to the user specific data. */
    );

/**
Summary:
Retrieve user-defined data from an HttpRequest.

Description:
Retrieve the user-defined pointer from a BIP_HttpRequest that was previous stored
by BIP_HttpRequest_SetUserData(). If no user-data pointer has been previously stored,
the caller's variable will be set to NULL.

Return:
    BIP_SUCCESS           : The HttpRequest's user-data pointer has been placed into the variable referenced by ppUserData.

See Also:
BIP_HttpRequest_SetUserData
**/
BIP_Status BIP_HttpRequest_GetUserData(
    BIP_HttpRequestHandle hHttpRequest,
    void                **ppUserData                       /*!< [out] Address of caller's variable where the user-data pointer will be placed. */
    );

/**
Summary:
Supported HTTP Methods

See Also:
BIP_HttpRequest_SetMethod
BIP_HttpRequest_GetMethod
**/
typedef enum BIP_HttpRequestMethod
{
    BIP_HttpRequestMethod_eGet,                            /*!< "GET" Method */
    BIP_HttpRequestMethod_eHead,                           /*!< "HEAD" Method */
    BIP_HttpRequestMethod_ePost,                           /*!< "POST" Method */
    BIP_HttpRequestMethod_ePut,                            /*!< "PUT" Method */
    BIP_HttpRequestMethod_eDelete,                         /*!< "DELETE" Method */
    BIP_HttpRequestMethod_eConnect,                        /*!< "CONNECT" Method */
    BIP_HttpRequestMethod_eOptions,                        /*!< "OPTIONS" Method */
    BIP_HttpRequestMethod_eTrace,                          /*!< "TRACE" Method */

    BIP_HttpRequestMethod_eUnknown,                        /*!< Method is not one of the above.  Check method's name string. */
    BIP_HttpRequestMethod_eMax
} BIP_HttpRequestMethod;

#define HTTP_METHOD_GET     "GET"
#define HTTP_METHOD_HEAD    "HEAD"
#define HTTP_METHOD_POST    "POST"
#define HTTP_METHOD_PUT     "PUT"
#define HTTP_METHOD_DELETE  "DELETE"
#define HTTP_METHOD_CONNECT "CONNECT"
#define HTTP_METHOD_OPTIONS "OPTIONS"
#define HTTP_METHOD_TRACE   "TRACE"

/**
Summary:
Populate the "request-line" fields of the specified BIP_HttpRequest.

Description:
The "request-line" consists of the following fields:
- method         : "GET", "HEAD", "POST", etc.   Can also be set with BIP_HttpRequest_SetMethod()
- request-target : URL/URI                       Can also be set with BIP_HttpRequest_SetTarget()
- HTTP-version   : For example: "HTTP/1.1"       Can also be set with BIP_HttpRequest_SetHttpVersion()

Return:
    BIP_SUCCESS           : The HttpRequest's version has bee updated.

See Also:
BIP_HttpRequest_SetMethod
BIP_HttpRequest_SetTarget
BIP_HttpRequest_SetHttpVersion
BIP_HttpRequest_GetRequestLine
**/
BIP_Status BIP_HttpRequest_SetRequestLine(
    BIP_HttpRequestHandle       hHttpRequest,              /*!< [in] Handle of BIP_HttpRequest to be updated. */
    const char                 *pMethodName,               /*!< [in] If non-NULL then the null-terminated HTTP method name. */
    BIP_HttpRequestMethod       method,                    /*!< [in] Method name enum (only used if pMethodName is NULL). */
    const char                 *pTarget,                   /*!< [in] Null-terminated request target string. */
    BIP_HttpRequestHttpVersion *pHttpVersion               /*!< [in] Pointer to a BIP_HttpReqestHttpVersion structure. */
                                                           /*!<      Optional if specified in BIP_HttpRequestCreateSettings.httpVersion  */
                                                           /*!<      or overrides that value if specified here. */
    );

/**
Summary:
Retrieve the values of the "request-line" fields of the specified BIP_HttpRequest.

Description:
The "request-line" consists of the following fields:
- method         : "GET", "HEAD", "POST", etc.   Can also be retrieved with BIP_HttpRequest_GetMethod()
- request-target : URL/URI                       Can also be retrieved with BIP_HttpRequest_GetTarget()
- HTTP-version   : For example: "HTTP/1.1"       Can also be retrieved with BIP_HttpRequest_GetHttpVersion()

Return:
    BIP_SUCCESS           : The HttpRequest's fields have been returned.

See Also:
BIP_HttpRequest_GetMethod
BIP_HttpRequest_GetTarget
BIP_HttpRequest_GetHttpVersion
BIP_HttpRequest_SetRequestLine
**/
BIP_Status BIP_HttpRequest_GetRequestLine(
    BIP_HttpRequestHandle       hHttpRequest,              /*!< [in]  Handle of BIP_HttpRequest to be accessed. */
    BIP_HttpRequestMethod      *pMethodEnum,               /*!< [out] Method name enum, if method not recognized then BIP_HttpRequestMethod_eUnknown. */
    const char                **ppMethodName,              /*!< [out] Optional address of caller's pointer to be set to the null-terminated method name. */
    const char                **ppTarget,                  /*!< [out] Address of caller's pointer to be set to the null-terminated request target. */
    BIP_HttpRequestHttpVersion *pHttpVersion               /*!< [out] Pointer to caller's BIP_HttpResponseHttpVersion structure. */
    );

/**
Summary:
Sets the HTTP method into an HttpRequest object, overwriting any previous method
that may have been previously set.

Description:
Updates the method for the specified HttpRequest object.  The method can be specified as
either a BIP_HttpRequestMethod enum value, or as a null-terminated character string.

Return:
    BIP_SUCCESS           : The HttpRequest has been updated with the specified method.

See Also:
BIP_HttpRequest_GetMethod
**/
BIP_Status BIP_HttpRequest_SetMethod(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in] Handle of BIP_HttpRequest to updated. */
    BIP_HttpRequestMethod methodEnum,                      /*!< [in] Method name enum used if pName is NULL. */
    const char           *pMethodName                      /*!< [in] If non-NULL then the null-terminated method name. */
    );

/**
Summary:
Retrieves the HTTP method present in the specified BIP_HttpRequest object.

Description:
Retrieves the HttpRequest's HTTP method as a pointer to a null-terminated string,
and also as a BIP_HttpRequestMethod enum value. Any unrecognized method string will
be indicated as BIP_HttpRequestMethod_eUnknown.

Return:
    BIP_SUCCESS           : The HttpRequest has been updated with the specified method.

See Also:
BIP_HttpRequest_SetMethod
**/
BIP_Status BIP_HttpRequest_GetMethod(
    BIP_HttpRequestHandle  hHttpRequest,                   /*!< [in]  Handle of BIP_HttpRequest to be accessd. */
    BIP_HttpRequestMethod *pMethodEnum,                    /*!< [out] Address of caller's variable to receive the BIP_HttpRequestMethod  enum. */
                                                           /*!<       If method not recognized, it will be set to BIP_HttpRequestMethod_eUnknown. */
    const char           **ppMethodName                    /*!< [out] Optional address of caller's pointer to be set to the null-terminated method name. */
    );

/**
Summary:
Sets the HTTP version into an HttpRequest object, overwriting any previous version
that may have been previously set.

Return:
    BIP_SUCCESS           : The HttpRequest's version has bee updated.

See Also:
BIP_HttpRequest_GetHttpVersion
**/
BIP_Status BIP_HttpRequest_SetHttpVersion(
    BIP_HttpRequestHandle       hHttpRequest,              /*!< [in] Handle of BIP_HttpRequest to be updated. */
    BIP_HttpRequestHttpVersion *pHttpVersion               /*!< [in] Pointer to a BIP_HttpReqestHttpVersion structure. */
    );

/**
Summary:
Retrieves the HTTP version from an HttpRequest object.

Return:
    BIP_SUCCESS           : The HTTP version has been stored in the caller's structure.

See Also:
BIP_HttpRequest_SetHttpVersion
**/
BIP_Status BIP_HttpRequest_GetHttpVersion(
    BIP_HttpRequestHandle       hHttpRequest,              /*!< [in]  Handle of BIP_HttpRequest to be accessd. */
    BIP_HttpRequestHttpVersion *pHttpVersion               /*!< [out] Pointer to caller's BIP_HttpResponseHttpVersion structure. */
    );

/**
Summary:
Sets the HTTP request target (URL) into an HttpRequest object, overwriting any previous target
that may have been previously set.

Description:
Updates the request target for the specified HttpRequest object.  The request target
is specified as a null-terminated character string.

For example, if the first line of the HTTP request is
\code
            "GET http://www.mywebsite.com/mypage.html HTTP/1.1"
\endcode
then the request target string will be:
\code
            "/mypage.html"
\endcode

Return:
    BIP_SUCCESS           : The HttpRequest has been updated with the specified target string.

See Also:
BIP_HttpRequest_GetTarget
**/
BIP_Status BIP_HttpRequest_SetTarget(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in] Handle of BIP_HttpRequest to be updated. */
    const char           *pTarget                          /*!< [in] Null-terminated request target string. */
    );

/**
Summary:
Retrieves the HTTP request target from the specified BIP_HttpRequest object.

Description:
Retrieves the HttpRequest's HTTP request target as a pointer to a null-terminated string.

Return:
    BIP_SUCCESS           : The caller's pointer has been updated with the pointer to the request target string.

See Also:
BIP_HttpRequest_GetTarget
*/
BIP_Status BIP_HttpRequest_GetTarget(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in]  Handle of BIP_HttpRequest to be accessd. */
    const char          **ppTarget                         /*!< [out] Address of caller's pointer to be set to the null-terminated request target. */
    );

/**
Summary:
Add a new HttpHeader to an HttpRequest.

Description:
Creates a new HttpHeader with the specified name and value, then adds it
to the HttpRequest's list of headers at the indicated position.

Return:
    non-NULL :        The handle of the new HttpHeader that has been added to the HttpRequest's header list.
Return:
    NULL     :        Failure, an HttpHeader instance could not be created.

See Also:
BIP_HttpRequest_RemoveHeader
**/
BIP_HttpHeaderHandle BIP_HttpRequest_AddHeader(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in]  Handle of BIP_HttpRequest to updated. */
    const char           *pName,                           /*!< [in]  Pointer to a null-terminated string with the header's name. */
    const char           *pValue,                          /*!< [in]  Optional pointer to a null-terminated string will be copied to the header's value. */
                                                           /*!<       If NULL, then the header will be created without a value. */
    BIP_HttpHeaderHandle  hAddBeforeThisHeader             /*!< [in]  If a BIP_HeaderHandle is passed, and it exists in the specified hHttpRequest, */
                                                           /*!<       the newly created header will be positioned immediately in front of it. */
                                                           /*!<       If NULL, the new header will be placed at the end of the HttpRequest's headers. */
    );

/**
Summary:
Remove (and destroy) an HttpHeader from an HttpRequest.

Description:
The indicated HttpHeader is destroyed after being removed from the specified HttpRequest's list of headers.

Return:
    void :

See Also:
BIP_HttpRequest_AddHeader
**/
void BIP_HttpRequest_RemoveHeader(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in] Handle of BIP_HttpRequest to be updated. */
    BIP_HttpHeaderHandle  hHttpHeader                      /*!< [in] Handle of the Header to be removed/destroyed. */
    );

/**
Summary:
Get the first or next HttpHeader following a given Header in the specified HttpRequest's list of headers.  Optionally,
a header name can be specified to return the next header with the specified name.

Description:
Starting from a given HttpHeader, or from the start of the HttpRequests's header list,
return the handle of the next Header, optionally, with a specified name.

Return:
    BIP_SUCCESS           : The handle of the next HttpHeader with the given name has been placed in the variable referenced by phNextHeader.
Return:
    BIP_INF_NOT_AVAILABLE : There are no Headers with the specified name following hCurrentHeader.
                            The variable referenced by phNextHeader has been set to NULL.
**/
BIP_Status BIP_HttpRequest_GetNextHeader(
    BIP_HttpRequestHandle hRequest,                        /*!< [in] Handle of the BIP_HttpRequest to be accessed. */
    BIP_HttpHeaderHandle  hCurrentHeader,                  /*!< [in] The Header from which to start.  Pass NULL to start from the beginning of the list. */
    const char           *pName,                           /*!< [in] If non-NULL, then return the next Header with a matching name.. */
    BIP_HttpHeaderHandle *phNextHeader,                    /*!< [out] Address of variable to receive the handle of the next header with the specified name. */
    const char          **ppNextHeaderValue                /*!< [out] Optional address of pointer that will be point to the header's value. */
                                                           /*!<       Pass NULL if not interested in the header's value. */
    );

/**
Summary:
Move an HttpHeader and place it before a specified HttpHeader.

Description:
Moves an HttpHeader to a different position in the HttpRequest's header list, by placing it in front of another
specified header, or at the end of the list.

Return:
    BIP_SUCCESS           : The Header was moved as requested.
**/
BIP_Status BIP_HttpRequest_MoveHeader(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in] Handle of the BIP_HttpRequest to be updated. */
    BIP_HttpHeaderHandle  hHeader,                         /*!< [in] The header to be moved. */
    BIP_HttpHeaderHandle  hPutBeforeThisHeader             /*!< [in] The Header that hHeader will moved in front of. */
                                                           /*!<      If NULL, then hHeader will be placed at the end of the HttpRequest's header list. */
    );

/**
Summary:
Get the name & value of an HttpHeader.

Description:
Returns a pointer to the null-terminated header name associated with a HeaderHandle.

Return:
    BIP_SUCCESS           : The address of the Header's name was returned.

See Also:
BIP_HttpRequest_SetHeaderValue
**/
BIP_Status BIP_HttpRequest_GetHeaderNameValue(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in]  Handle of the BIP_HttpRequest to be accessed. */
    BIP_HttpHeaderHandle  hHeader,                         /*!< [in]  The header of interest. */
    const char          **ppName,                          /*!< [out] The address of a pointer that will be updated to point the Header's name as a null-terminated string. */
                                                           /*!<       This pointer will become invalid when it's Header is destroyed. */
    const char          **ppValue                          /*!< [out] The address of a pointer that will be updated to point the Header's value as a null-terminated string. */
                                                           /*!<       This pointer will become invalid when it's Header is destroyed. */
    );

/**
Summary:
Set the value of a HttpRequest's HttpHeader.

Description:
Replaces a header's value with a copy of a specified character string.

Return:
    BIP_SUCCESS       : The Header's value string has been modified as requested.

See Also:
BIP_HttpRequest_GetHeaderNameValue
**/
BIP_Status BIP_HttpRequest_SetHeaderValue(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in] Handle of the BIP_HttpRequest to be updated. */
    BIP_HttpHeaderHandle  hHeader,                         /*!< [in] The header that will have its value changed. */
    const char           *pValue                           /*!< [in] A pointer to a null-terminated string containing the Header's new value. */
                                                           /*!<      Passing NULL will result in a header without a value.  */
    );

/**
Summary:
Populate a BIP_HttpRequest object from an HTTP request message that is contained in one or more batom_cursors.

Description:
Refer to /BSEAV/lib/utils/bioatom.h for information on the usage of the "batom" APIs and data types.

An single HTTP request can be spread across multiple batom_cursors.  This API can be called sequentially to
feed each of the cursors to the deserializer.  When an entire HTTP request has been deserialized, the
pDeserializeComplete output argument will be set to true, and the cursor will be left pointing to the first
character after the HTTP request (start of payload) or the cursor will be at EOF.

Return:
    BIP_SUCCESS       : The data from the specified cursor has been deserialized.  Check *pDeserializeComplete
                        to determine completion.
Return:
    BIP_ERR_SERIALIZE_IN_PROGRESS : The BIP_HttpRequest object is in the process of serializing output.
                        Either complete the serialize process by calling the appropriate
                        BIP_HttpRequest_Serialize... API until it completes or fails, or
                        call BIP_HttpRequest_Clear() to start over.

See Also:
BIP_HttpRequest_DeserializeFromBuffer
**/
BIP_Status BIP_HttpRequest_DeserializeFromAtom(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in]  The BIP_HttpRequest to be populated with the deserialized data. */
    batom_cursor         *cursor,                          /*!< [in]  Cursor that contains incoming data */
    bool                 *pDeserializeComplete             /*!< [out] Set to true if the http request is complete, */
                                                           /*!<             i.e., it has <CRNL><CRNL> or <NL><NL>. */
    );

/**
Summary:
Populate a BIP_HttpRequest object from an HTTP request message that is contained in one or more
memory buffers (char * arrays).

Description:
An single HTTP request can be spread across multiple buffers.  This API can be called sequentially to
feed each of the buffers to the deserializer.  When an entire HTTP request has been deserialized, the
*pDeserializeComplete output argument will be set to true, and *pConsumedBytes will indicate the position
of the first character after the HTTP request (unless *pConsumedBytes == bufferSize, which indicates
that the buffer has nothing after the end of the request).

The memory buffers should not be NULL-terminated.

Return:
    BIP_SUCCESS                : The data from the specified buffer has been deserialized.
                                 Check *pDeserializeComplete to determine completion.
Return:
    BIP_ERR_SERIALIZE_IN_PROGRESS : The BIP_HttpRequest object is in the process of serializing output.
                                 Either complete the serialize process by calling the appropriate
                                 BIP_HttpRequest_Serialize... API until it completes or fails, or
                                 call BIP_HttpRequest_Clear() to start over.

See Also:
BIP_HttpRequest_DeserializeFromAtom
**/
BIP_Status BIP_HttpRequest_DeserializeFromBuffer(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in]  The HttpRequest to be populated with the deserialized data. */
    const char           *buffer,                          /*!< [in]  Buffer that contains incoming data. */
    size_t                bufferSize,                      /*!< [in]  Input buffer size. */
    bool                 *pDeserializeComplete,            /*!< [out] Set to true if the http request is complete, ie it has */
                                                           /*!<       end of header(<CRNL><CRNL or <NL><NL>). */
    size_t               *pConsumedBytes                   /*!< [out] If request is complete then this specifies the consumed */
                                                           /*!<       data size. If message is not complete then this must be equal to bufferSize. */
    );

/**
Summary:
Produce a "batom" that contains a serialized version of the HttpRequest
that is suitable for sending over the network.

Description:
After constructing an HttpRequest object (by setting the Start Line and adding headers),
this API can be used convert the HttpRequest into character string to be transmitted.
The resulting character string is encapsulated by a batom and returned to the caller.

Refer to /BSEAV/lib/utils/bioatom.h for information on the usage of the "batom" APIs and data types.

The data in the batom is not NULL-terminated.

Return:
    BIP_SUCCESS                : The data from the HttpRequest has been serialized into the batom,
                                 and the batom has been returned to the caller.
Return:
    BIP_ERR_DESERIALIZE_IN_PROGRESS  : The BIP_HttpRequest object is in the process of being constructed
                                 by deserializing a received HTTP request.  The HttpRequest cannot
                                 be serialized until the deserializing is completed.
                                 Either complete the deserializing process by calling the appropriate
                                 BIP_HttpRequest_Deserialize... API until it completes or fails, or
                                 call BIP_HttpRequest_Clear() to start over.

See Also:
BIP_HttpRequest_SerializeToBuffer
**/
BIP_Status BIP_HttpRequest_SerializeToAtom(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in]  Handle of HttpRequest to be serialized. */
    batom_factory_t       factory,                         /*!< [in]  Factory to use for creating atom .   */
    batom_t              *atom                             /*!< [out] Where to put the atom containing the serialized data. */
    );

/**
Summary:
Determine the number of bytes that a BIP_HttpRequest will occupy when it is serialized
(converted to one long character string for transmission over the network).

Description:
This API allows the caller to allocate a buffer of the proper size before calling
BIP_HttpRequest_SerializeToBuffer().

Return:
    BIP_SUCCESS                : The *pSerializeBufferSize argument contains the size required for
                                 the serialized HttpRequest.
Return:
    BIP_ERR_DESERIALIZE_IN_PROGRESS : The BIP_HttpRequest object is in the process of being constructed
                                by deserializing a received HTTP request.  The HttpRequest cannot
                                be serialized until the deserializing is completed.
                                Either complete the deserializing process by calling the appropriate
                                BIP_HttpRequest_Deserialize... API until it completes or fails, or
                                call BIP_HttpRequest_Clear() to start over.

See Also:
BIP_HttpRequest_SerializeToBuffer
**/
BIP_Status BIP_HttpRequest_GetSerializeBufferSize(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in]  Handle of HttpRequest to be serialized. */
    size_t               *pSerializeBufferSize             /*!< [out] Address of variable to receive the buffer size */
                                                           /*!<       required for serializing the HttpRequest. */
    );

/**
Summary:
Populate the caller's memory buffer (char array) with a "serialized" version of the HttpRequest
that is suitable for sending over the network.

Description:
After constructing an HttpRequest object (by setting the Start Line and adding headers),
this API can be used convert the HttpRequest into character string to be transmitted.
If the caller's buffer is not large enough to hold the entire serialized HttpRequest,
this API will set the *pSerializeComplete output argument to false. The caller is then expected
to call this API repeatedly until *pSerializeComplete is true.

When the serialize is complete, the *pSerializedBytes output argument indicates the number
of bytes (of the caller's buffer) that are occupied by the serialized HttpRequest.

The data in the callers's buffer is not NULL-terminated.

Return:
    BIP_SUCCESS                : The HttpRequest has been serialized into the callers buffer.
                                 Check *pDeserializeComplete to determine completion.
Return:
    BIP_ERR_DESERIALIZE_IN_PROGRESS :  The BIP_HttpRequest object is in the process of being constructed
                                 by deserializing a received HTTP request.  The HttpRequest cannot
                                 be serialized until the deserializing is complete.
                                 Either complete the deserializing process by calling the appropriate
                                 BIP_HttpRequest_Deserialize... API until it completes or fails, or
                                 call BIP_HttpRequest_Clear() to start over.

See Also:
BIP_HttpRequest_GetSerializeBufferSize
**/
BIP_Status BIP_HttpRequest_SerializeToBuffer(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in]  Handle of the HttpRequest to be serialized. */
    char                 *pBuffer,                         /*!< [in]  Buffer to receive the serialized output. */
    size_t                bufferSize,                      /*!< [in]  Buffer size. */
    bool                 *pSerializeComplete,              /*!< [out] Set to true if serializing is complete. Otherwise, */
                                                           /*!<       the buffer is full, call this again with a new buffer.  */
    size_t               *pSerializedBytes                 /*!< [out] The number of bytes that have been placed in the buffer. */
    );

/**
Summary:
Data structures for holding aggregated "Range" header information for
Range headers with units of "bytes".

See Also:
BIP_HttpRequest_ParseRangeHeaderForBytes
**/

/* First, the sub-structure for representing a single byte-range-spec or
 * suffix-byte-range-spec. */
typedef struct BIP_HttpRequestRangeHeaderByteRange
{
    BIP_HttpHeaderHandle hHeader;
    bool                 isSuffixByteRange;                /*!< Indicates whether to use byteRangeSpec or suffixByteRangeSpec. */
    union
    {
        /** Used when isSuffixByteRange==false */
        struct
        {
            uint64_t firstBytePos;                         /*!< First byte of range. */
            uint64_t lastBytePos;                          /*!< Last byte of range, or BIP_U64_MAX if last byte is unspecified. */
        } byteRangeSpec;

        /** Used when isSuffixByteRange==true */
        struct
        {
            uint64_t suffixLength;                         /*!< Range is the last suffixLength bytes of the file. */
        } suffixByteRangeSpec;
    } specs;
} BIP_HttpRequestRangeHeaderByteRange;

/* Then, a struct that represents a byte-range-set and contains a variable-length
 * array of the RangeHeaderByteRange structs. */
typedef struct BIP_HttpRequestParsedRangeHeaderForBytes
{
    unsigned byteRangeCount;                               /*!< Number of elements in the *pByteRangeSet array. */

    /** An array of structs (with byteRangeCount elements),
     *  with each element describing one range specification. */
    BIP_HttpRequestRangeHeaderByteRange *pByteRangeSet;
} BIP_HttpRequestParsedRangeHeaderForBytes;

/**
Summary:
Retrieve the contents of the HttpRequest's "Range" headers that have units
specified as "bytes=".

Description:
The specified BIP_HttpRequest is scanned for any "Range" headers that have units
specified as "bytes".  These Range headers (as defined in RFC7233) are parsed
and the resulting data is placed into a BIP_HttpRequestParsedRangeHeaderForBytes
structure whose address is then returned to the caller. The caller can then
access the Range header information like this:

\code
void printByteRangeHeaderInfo(BIP_HttpRequestHandle hRequest)
{
    BIP_HttpRequestParsedRangeHeaderForBytes  *pRangeInfo;
    BIP_Status  rc;
    int i;

    rc = BIP_HttpRequest_ParseRangeHeaderForBytes( hRequest, &pRangeInfo );

    printf("Found %u byte ranges\n", pRangeHdr->byteRangeCount);

    for (i=0 ; i<pRangeHdr->byteRangeCount ; i++) {

        if (pRangeHdr->pByteRangeSet[i].isSuffixByteRange)
        {
            printf("Range %u: The last %u bytes\n", i,
                   pRangeHdr->pByteRangeSet[i].specs.suffixByteRangeSpec.suffixLength);
        }
        else
        {
            printf("Range %u: From byte %u to byte %u\n", i,
                   pRangeHdr->pByteRangeSet[i].specs.byteRangeSpec.firstBytePos,
                   pRangeHdr->pByteRangeSet[i].specs.byteRangeSpec.lastBytePos);
        }
    }
}
\endcode

 This returned pointer remains valid until either:

- The HttpRequest object is destroyed, or
- This BIP_HttpRequest_ParseRangeHeaderForBytes() API is called again.

Refer to RFC7233 Appendix D for the ABNF description of the "Range" header.

Return:
    BIP_SUCCESS                : The HttpRequest's Range headers have been parsed and the
                                 results can be accessed using the returned *ppRangeHeaderParsed
                                 pointer.
Return:
    BIP_INF_NOT_AVAILABLE      : There are no "Range" headers specified as "bytes=" for the HttpRequest.
Return:
    BIP_ERR_PARSE_IN_PROGRESS :
Return:
    BIP_ERR_RENDER_IN_PROGRESS :


See Also:
BIP_HttpRequestParsedRangeHeaderForBytes
BIP_HttpRequest_AddRangeHeaderByteRange
BIP_HttpRequest_AddRangeHeaderSuffixByteRange
**/
BIP_Status BIP_HttpRequest_ParseRangeHeaderForBytes(
    BIP_HttpRequestHandle                      hHttpRequest,       /*!< [in]  Handle of the HttpRequest to be accessed. */
    BIP_HttpRequestParsedRangeHeaderForBytes **ppRangeHeaderParsed /*!< [out] Address of caller's pointer variable that */
                                                                   /*!<       will point to the parsed results after return. */
    );

/**
Summary:
API to get startOffset and length for a range spec entry.

Description:
Caller can use this API to validate the range spec entry on the basis of contentLength and get the valid startOffset and Length.
Length specifies the amount of data that need to be send from the startOffset byte position.
This api will perform the following validation on range spec:
\code
{
    if( isSuffixByteRange == false)
    {
        if((firstBytePos > lastBytePos) && (lastBytePos != BIP_U64_MAX) ) :Basic validation, which checks whether firstBytePos is greater than lastBytePos when lastBytePos has a valid value.
        {
            return BIP_ERR_INVALID_PARAMETER;
        }

        if( contentLength != 0)
        {
            if(firstBytePos >= contentLength)
            {
                return BIP_ERR_INVALID_PARAMETER;
            }
            else
            {
                *pStartOffset = firstBytePos;
            }

            if(lastBytePos >= contentLength)    : incase lastBytePos is invalid that is BIP_U64_MAX,then it will be automatically checked here since contentLength can have a max valid value as BIP_U64_MAX.
            {
                *pLength = ( contentLength - firstBytePos);
            }
            else
            {
                *pLength = ( lastBytePos + 1 - firstBytePos);
            }
        }
        else
        {
             pStartOffset = firstBytePos;
             if(lastBytePos != BIP_U64_MAX)
             {
                *pLength = ( lastBytePos + 1 - firstBytePos);
             }
             else
             {
                 pLength = 0;
                 rc = BIP_INF_NOT_AVAILABLE;
             }
        }
    }
    else if( isSuffixByteRange == true)
    {
        if(contentLength != 0 )
        {
            if(suffixLength <= contentLength)
            {
                *pStartOffset = contentLength - suffixLength;
                 pLen = suffixLength;
            }
            else
            {
                 *pStartOffset = 0; :If the selected representation is shorter than the specified suffix-length, the entire representation is used
                 *pLen = contentLength;
            }
        }
        else
        {
            return BIP_INF_NOT_AVAILABLE;
        }
    }
}
\endcode

Truth Table:
\code

 Is this a          RangeEntry has   RangeEntry has   RangeEntry has   Caller passes         Resulting                     Resulting
 SuffixByteRange?   FirstBytePos?    LastBytePos?     suffixLength?    ContentLength        *pStartOffset                  *pLength
 ----------------   --------------   --------------   --------------   ----------------   ----------------------------    -----------------------------
    Yes                 X                X                Yes               0            0 (unknown)                     SuffixLength
    Yes                 X                X                Yes             number         ContentLength - SuffixLength    SuffixLength
    No                 Yes         No (BIP_U64_MAX)        X                0            FirstBytePos                    0 (unknown)
    No                 Yes         No (BIP_U64_MAX)        X              number         FirstBytePos                    ContentLength - FirstBytePos
    No                 Yes              Yes                X                X            FirstBytePos                    LastBytePos - FirstBytePos + 1
\endcode

Return:
   BIP_ERR_INVALID_PARAMETER : Validation failed due to:
   - FirstBytePos > ContentLength  or
   - FirstBytePos > LastBytePos

Return:
    BIP_INF_NOT_AVAILABLE : Complete results cannot be determined.
   - If *pLength == 0, then *pStartOffset is valid, but we don't know length.
   - If *pStartOffset == 0, then *pLength is valid, but we don't know start offset.
**/

BIP_Status BIP_HttpRequest_GetRangeEntryOffset(
    BIP_HttpRequestRangeHeaderByteRange *pRange,           /*!< [in] Caller's struct containing a pointer to a valid range spec entry */
    uint64_t                             contentLength,    /*!< [in] ContentLength from caller, if it is 0 then that indicates caller doesn't know the contentLength */
                                                           /*!<      and this api won't be able to perform validation on range spec's firstBytePos and lastBytePos. */
    uint64_t                            *pStartOffset,     /*!< [out] Callers variable to receive start byte offset. Caller need to send out data from this position. */
    uint64_t                            *pLength           /*!< [out] Caller's variable to receive length of range, i.e the amount of data that need to be sent out from the */
                                                           /*!<       startOffset. If contentLength us unknown and Range spec also doesn't have a valid lastBytePos then this will be set to 0. */
    );

/**
Summary:
Add a specified first/last byte range to an HttpRequest's "Range" header.

Description:
If the HttpRequest does not have a Range header (with "bytes="), one will
be created with the specified range, otherwise, the specified range will
be added to the "byte-range-set" of the first existing Range header.

Refer to RFC7233 Appendix D for the ABNF description of the "Range" header.

Return:
    BIP_SUCCESS                : The specified byte range has been added to the
                                 HttpRequest's Range header(s) as requested.
Return:
    BIP_ERR_PARSE_IN_PROGRESS :
Return:
    BIP_ERR_RENDER_IN_PROGRESS :

See Also:
BIP_HttpRequest_AddRangeHeaderSuffixByteRange
BIP_HttpRequest_ParseRangeHeaderForBytes
**/
BIP_Status BIP_HttpRequest_AddRangeHeaderByteRange(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in]  Handle of the HttpRequest to be accessed. */
    uint64_t              firstBytePos,                    /*!< [in]  First byte position. */
    uint64_t              lastBytePos                      /*!< [in]  Last byte position. Use BIP_U64_MAX if last byte is unspecified. */
    );

/**
Summary:
Add a specified suffix byte range to an HttpRequest's "Range" header.

Description:
If the HttpRequest does not have a Range header (with "bytes="), one will
be created with the specified suffix range, otherwise, the specified suffix
range will be added to the "byte-range-set" of the first existing Range header.

A suffix byte range is represented by a single integer "suffixLength", which
indicates the last "suffixLength" bytes from the end of the representation.
For example, if a representation contains 1000 bytes (0 to 999), then a
"suffixLength" of 10 will indicate the last 10 bytes, or bytes 990 through
999.

Refer to RFC7233 Appendix D for the ABNF description of the "Range" header.

Return:
    BIP_SUCCESS                : The specified suffix byte range has been
                                 added to the HttpRequest's Range header(s)
                                 as requested.
Return:
    BIP_ERR_PARSE_IN_PROGRESS :
Return:
    BIP_ERR_RENDER_IN_PROGRESS :

See Also:
BIP_HttpRequest_AddRangeHeaderByteRange
BIP_HttpRequest_ParseRangeHeaderForBytes
**/
BIP_Status BIP_HttpRequest_AddRangeHeaderSuffixByteRange(
    BIP_HttpRequestHandle hHttpRequest,                    /*!< [in]  Handle of the HttpRequest to be accessed. */
    uint64_t              suffixLength                     /*!< [in]  This many bytes from the end of the representation. */
    );

/**
Summary:
Retrieve the contents of the HttpRequest's "TimeSeekRange.dlna.org" header.

Description:
The specified BIP_HttpRequest is scanned for a "TimeSeekRange.dlna.org" header.
If the header is present, it will be parsed and the resulting data placed into
the caller's variables.

In an HTTP request, the "TimeSeekRange.dlna.org" header may contain these two values:
- npt-start-time
- npt-end-time (optional)

The following is a summary of the logic used to validate and convert those two
value into a startTime and endTime.  If the endTime cannot be determined,
it is returned as -1 (which should be interpreted as "unknown").

    - If scan mode is forward (positive play-speed):
        - If contentDurationInMs is specified by caller
            - Fail if npt-start-time  is >= contentDurationInMs
        - If header contains npt-end-time:
            - Truncate npt-end-time to contentDuration if required (and possible)
            - Fail if npt-end-time < npt-start-time
            - Return *pStartTimeInMs = npt-start-time
            - Return *pEndTimeInMs = npt-end-time
        - Else (header omits npt-end-time)
            - Return *pStartTimeInMs = npt-start-time
            - If contentDurationInMs is specified by caller
                - Return *pEndTimeInMs = contentDurationInMs
            - Else (contentDurationInMs is passed as zero)
                - Return *pDurationInMs = -1

    - If scan mode is backward (negative play-speed):
        - If contentDurationInMs is specified by caller
            - Fail if npt-end-time is > contentDurationInMs
        - If header contains npt-end-time:
            - Fail if npt-start-time  < npt-end-time
            - Truncate npt-start-time  to contentDuration if required (and possible)
            - Return *pStartTimeInMs = npt-start-time
            - Return *pEndTimeInMs = npt-end-time
        - Else (header omits npt-end-time)
            - Return *pStartTimeInMs = npt-start-time
            - Return *pEndTimeInMs = 0

 Truth Table:
\code
                Header has       Caller passed       Resulting              Resulting
 Scan Mode      npt-end-time      contentDuration     *pStartTimeInMs        *pEndTimeInMs
 --------       -----------     --------------      --------------        ----------------------
 Forward          No               No               npt-start-time         -1 (unknown)
 Forward          No               Yes              npt-start-time         contentDurationInMs
 Forward          Yes              X                npt-start-time         npt-end-time
 Backward         No               X                npt-start-time         0
 Backward         Yes              X                npt-start-time         npt-end-time

\endcode

The scan mode is determined by examining the "PlaySpeed.dlna.org" header.

Refer to DLNA Guidelines March 2014 Part 1-1 Section 7.5.4.3.2.24.3 for a
detailed description of the "TimeSeekRange.dlna.org" header.

Refer to DLNA Guidelines March 2014 Part 1-1 Section 7.5.4.3.3.16.3 for a
detailed description of the "PlaySpeed.dlna.org" header.

Return:
    BIP_SUCCESS                : The HttpRequest's "TimeSeekRange.dlna.org" header has been
                                 parsed and the results have been placed into the caller's variables.
Return:
    BIP_INF_NOT_AVAILABLE      : There is no "TimeSeekRange.dlna.org" header for the HttpRequest.
Return:
    BIP_ERR_PARSE_IN_PROGRESS :
Return:
    BIP_ERR_RENDER_IN_PROGRESS :


See Also:
**/
BIP_Status BIP_HttpRequest_ParseTimeSeekRangeDlnaOrgHeader(
    BIP_HttpRequestHandle hRequest,                        /*!< [in]  Handle of the HttpRequest to be accessed. */
    int64_t               contentDurationInMs,             /*!< [in]  Total content duration if known.  Pass -1 if unknown. */
    int64_t              *pStartTimeInMs,                  /*!< [out] Caller's variable to receive npt-start-time as int64_t milliseconds */
    int64_t              *pEndTimeInMs                     /*!< [out] Caller's variable to receive npt-end-time as int64_t milliseconds */
                                                           /*!<       -1 is returned if duration cannot be determined. */
    );

#ifdef __cplusplus
}
#endif

#endif /* BIP_HTTP_REQUEST_H */
