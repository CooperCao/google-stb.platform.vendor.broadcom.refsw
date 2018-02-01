/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#include <ctype.h>
#include <inttypes.h>

#include "bip_priv.h"
#include "blst_queue.h"
#include "bioatom.h"
#include "limits.h"

BDBG_MODULE( bip_http_request );
BDBG_OBJECT_ID( BIP_HttpRequest );

BIP_SETTINGS_ID( BIP_HttpRequestCreateSettings );
BIP_SETTINGS_ID( BIP_HttpRequestClearSettings );
BIP_SETTINGS_ID( BIP_HttpRequestPrintSettings );

/*****************************************************************************
 *  Deserializer states:
 *****************************************************************************/
typedef enum BIP_HttpRequestDeserializeState {
    deserializeState_eIdle = 0,
    deserializeState_eMethod,
    deserializeState_eRequestTarget,
    deserializeState_eHttpVersion,
    deserializeState_eRequestLineTerm,
    deserializeState_eHeaders,
    deserializeState_eDone
} BIP_HttpRequestDeserializeState;

static const char *toStr_deserializeState(
    int value
    )
{
    const namevalue_t myStrings[] = {
        {"Idle",            deserializeState_eIdle           },
        {"Method",          deserializeState_eMethod         },
        {"RequestTarget",   deserializeState_eRequestTarget  },
        {"HttpVersion",     deserializeState_eHttpVersion    },
        {"RequestLineTerm", deserializeState_eRequestLineTerm},
        {"Headers",         deserializeState_eHeaders        },
        {"Done",            deserializeState_eDone           },
        {NULL,              0                                }
    };

    return( lookup_name( myStrings, ( value )));
}

/* Define some non-printable ASCII characters as they are named in RFC5234. */
#define SP   ( 0x20 )
#define HTAB ( 0x09 )
#define CR   ( 0x0d )
#define LF   ( 0x0a )

/*****************************************************************************
 *  Serializer states:
 *****************************************************************************/
typedef enum BIP_HttpRequestSerializeState {
    serializeState_eIdle = 0,
    serializeState_eMethod,
    serializeState_eMethodSpace,
    serializeState_eTarget,
    serializeState_eTargetSpace,
    serializeState_eVersionPrefix,
    serializeState_eVersionMajor,
    serializeState_eVersionDot,
    serializeState_eVersionMinor,
    serializeState_eRequestLineTerm,
    serializeState_eHeaders,
    serializeState_eDone
} BIP_HttpRequestSerializeState;

static const char *toStr_serializeState(
    int value
    )
{
    const namevalue_t myStrings[] = {
        {"Idle",            serializeState_eIdle           },
        {"Method",          serializeState_eMethod         },
        {"MethodSpace",     serializeState_eMethodSpace    },
        {"Target",          serializeState_eTarget         },
        {"TargetSpace",     serializeState_eTargetSpace    },
        {"VersionPrefix",   serializeState_eVersionPrefix  },
        {"VersionMajor",    serializeState_eVersionMajor   },
        {"VersionDot",      serializeState_eVersionDot     },
        {"VersionMinor",    serializeState_eVersionMinor   },
        {"RequestLineTerm", serializeState_eRequestLineTerm},
        {"Headers",         serializeState_eHeaders        },
        {"Done",            serializeState_eDone           },
        {NULL,              0                              }
    };

    return( lookup_name( myStrings, ( value )));
}                                                          /* toStr_serializeState */

/*****************************************************************************
 *  Object structures:
 *****************************************************************************/
typedef struct BIP_HttpRequest
{
    BDBG_OBJECT( BIP_HttpRequest )

    void    *ownerContext;                                 /* Any pointer passed to Create(), must also be passed to Destroy(). */
    BIP_HttpRequestCreateSettings   createSettings;

    BIP_StringHandle                hMethod;
    BIP_StringHandle                hRequestTarget;
    BIP_HttpRequestHttpVersion      httpVersion;
    BIP_HttpHeaderListHandle        hHeaderList;
    void                           *pUserData;

    BIP_HttpRequestDeserializeState deserializeState;
    BIP_HttpRequestSerializeState   serializeState;

    size_t                          byteIndex;             /* Byte index within current object being serialized/deserialized. */
    B_MutexHandle                   hMutex;
    batom_factory_t                 factory;

    /* Pointers to custom header parse results. */
    BIP_HttpRequestParsedRangeHeaderForBytes *pRangeHeaderParsed;
} BIP_HttpRequest;

/*****************************************************************************
 *  Sets the HTTP method into an HttpRequest object, overwriting any
 *  previous method that may have been previously set.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_SetMethod(
    BIP_HttpRequestHandle hRequest,                        /*!< [in] Handle of BIP_HttpRequest to updated. */
    BIP_HttpRequestMethod methodEnum,                      /*!< [in] Method name enum used if pName is NULL. */
    const char           *pMethodName                      /*!< [in] If non-NULL then the null-terminated method name. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    /* If MethodName string is passed, use it. So we need to find the matching enum. */
    if (pMethodName)
    {
        methodEnum = BIP_FromStr_BIP_HttpRequestMethod( pMethodName );
    }
    /* Else (no MethodName string) use the enum, if valid. */
    else
    {
        BIP_CHECK_GOTO(( methodEnum <  BIP_HttpRequestMethod_eMax &&
                         methodEnum != BIP_HttpRequestMethod_eUnknown ),
            ( "Invalid value for BIP_HttpRequestMethod enum: %u", methodEnum ),
            error, BIP_ERR_INVALID_PARAMETER, rc );

        pMethodName = BIP_ToStr_BIP_HttpRequestMethod( methodEnum );
    }

    BIP_String_StrcpyChar( hRequest->hMethod, pMethodName );

error:
    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_SetMethod */

/*****************************************************************************
 *  Retrieves the HTTP method present in the specified BIP_HttpRequest
 *  object.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_GetMethod(
    BIP_HttpRequestHandle  hRequest,                       /*!< [in]  Handle of BIP_HttpRequest to be accessd. */
    BIP_HttpRequestMethod *pMethodEnum,                    /*!< [out] Address of caller's variable to receive the BIP_HttpRequestMethod  enum.
                                                           If method not recognized, it will be set to BIP_HttpRequestMethod_eUnknown. */
    const char           **ppMethodName                    /*!< [out] Optional address of caller's pointer to be set to the null-terminated method name. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    if (pMethodEnum)
    {
        *pMethodEnum = BIP_FromStr_BIP_HttpRequestMethod( BIP_String_GetString( hRequest->hMethod ));
    }

    if (ppMethodName)
    {
        *ppMethodName = BIP_String_GetString( hRequest->hMethod );
    }

error:
    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_GetMethod */

/*****************************************************************************
 *  Sets the HTTP request target (URL) into an HttpRequest object,
 *  overwriting any target that may have been previously set.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_SetTarget(
    BIP_HttpRequestHandle hRequest,                        /*!< [in] Handle of BIP_HttpRequest to updated. */
    const char           *pTarget                          /*!< [in] Null-terminated request target string. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );
    BDBG_ASSERT( pTarget );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    BIP_CHECK_GOTO(( pTarget ),
        ( "Invalid NULL pointer" ),
        error, BIP_ERR_INVALID_PARAMETER, rc );

    BIP_String_StrcpyChar( hRequest->hRequestTarget, pTarget );

error:
    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_SetTarget */

/*****************************************************************************
 *  Retrieves the HTTP request target from the specified BIP_HttpRequest
 *  object.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_GetTarget(
    BIP_HttpRequestHandle hRequest,                        /*!< [in]  Handle of BIP_HttpRequest to be accessd. */
    const char          **ppTarget                         /*!< [out] Address of caller's pointer to be set to the null-terminated request target. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );
    BDBG_ASSERT( ppTarget );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    BIP_CHECK_GOTO(( ppTarget ),
        ( "Invalid NULL pointer" ),
        error, BIP_ERR_INVALID_PARAMETER, rc );

    *ppTarget = BIP_String_GetString( hRequest->hRequestTarget );

error:
    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_GetTarget */

/*****************************************************************************
 *  Sets the HTTP version into an HttpRequest object, overwriting any
 *  previous version that may have been previously set.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_SetHttpVersion(
    BIP_HttpRequestHandle       hRequest,                  /*!< [in] Handle of BIP_HttpRequest to updated. */
    BIP_HttpRequestHttpVersion *pHttpVersion               /*!< [in] Pointer to a BIP_HttpReqestHttpVersion structure. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_ASSERT( pHttpVersion );

    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    /* Validate the HttpVersion structure.  */
    BIP_CHECK_GOTO(( pHttpVersion ),
        ( "Invalid NULL pointer" ),
        error, BIP_ERR_INVALID_PARAMETER, rc );

    BIP_CHECK_GOTO(( pHttpVersion->major>=0 && pHttpVersion->major<=9 ),
        ( "HTTP major version: %u exceeds single digit range.", pHttpVersion->major ),
        error, BIP_ERR_INVALID_PARAMETER, rc );

    BIP_CHECK_GOTO(( pHttpVersion->minor>=0 && pHttpVersion->minor<=9 ),
        ( "HTTP minor version: %u exceeds single digit range.", pHttpVersion->minor ),
        error, BIP_ERR_INVALID_PARAMETER, rc );

    hRequest->httpVersion = *pHttpVersion;

error:
    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_SetHttpVersion */

/*****************************************************************************
 *  Retrieves the HTTP version from an HttpRequest object.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_GetHttpVersion(
    BIP_HttpRequestHandle       hRequest,                  /*!< [in] Handle of BIP_HttpRequest to be accessd. */
    BIP_HttpRequestHttpVersion *pHttpVersion               /*!< [out] Pointer to caller's BIP_HttpResponseHttpVersion structure. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );
    BDBG_ASSERT( pHttpVersion );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    BIP_CHECK_GOTO(( pHttpVersion ),
        ( "Invalid NULL pointer" ),
        error, BIP_ERR_INVALID_PARAMETER, rc );

    *pHttpVersion = hRequest->httpVersion;

error:
    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_GetHttpVersion */

/*****************************************************************************
 *  Populate the "request-line" fields of the specified BIP_HttpRequest.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_SetRequestLine(
    BIP_HttpRequestHandle       hRequest,                  /*!< [in] Handle of BIP_HttpRequest to be updated. */
    const char                 *pMethodName,               /*!< [in] If non-NULL then the null-terminated HTTP method name. */
    BIP_HttpRequestMethod       methodEnum,                /*!< [in] Method name enum (only used if pMethodName is NULL). */
    const char                 *pTarget,                   /*!< [in] Null-terminated request target string. */
    BIP_HttpRequestHttpVersion *pHttpVersion               /*!< [in] Pointer to a BIP_HttpReqestHttpVersion structure. */
                                                           /*!<      Optional if specified in BIP_HttpRequestCreateSettings.httpVersion or */
                                                           /*!<      overrides that value if specified here. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );
    BDBG_ASSERT( pTarget );
    BDBG_ASSERT( pHttpVersion );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    /* Start by validating all fields, because if anything is bad, we
     * don't want to change anything. */

    /* If MethodName string is passed, use it. So we need to find the matching enum. */
    if (pMethodName)
    {
        methodEnum = BIP_FromStr_BIP_HttpRequestMethod( pMethodName );
    }
    /* Else (no MethodName string) use the enum, if valid. */
    else
    {
        BIP_CHECK_GOTO(( methodEnum <  BIP_HttpRequestMethod_eMax &&
                         methodEnum != BIP_HttpRequestMethod_eUnknown ),
            ( "Invalid value for BIP_HttpRequestMethod enum: %u", methodEnum ),
            error, BIP_ERR_INVALID_PARAMETER, rc );

        pMethodName = BIP_ToStr_BIP_HttpRequestMethod( methodEnum );
    }

    /* Validate the RequestTarget structure.  */
    BIP_CHECK_GOTO(( pTarget ),
        ( "Invalid NULL pointer" ),
        error, BIP_ERR_INVALID_PARAMETER, rc );

    /* Validate the HttpVersion structure.  */
    BIP_CHECK_GOTO(( pHttpVersion ),
        ( "Invalid NULL pointer" ),
        error, BIP_ERR_INVALID_PARAMETER, rc );

    BIP_CHECK_GOTO(( pHttpVersion->major>=0 && pHttpVersion->major<=9 ),
        ( "HTTP major version: %u exceeds single digit range.", pHttpVersion->major ),
        error, BIP_ERR_INVALID_PARAMETER, rc );

    BIP_CHECK_GOTO(( pHttpVersion->minor>=0 && pHttpVersion->minor<=9 ),
        ( "HTTP minor version: %u exceeds single digit range.", pHttpVersion->minor ),
        error, BIP_ERR_INVALID_PARAMETER, rc );

    /* Everything has been validated, now make the changes. */

    BIP_String_StrcpyChar( hRequest->hMethod, pMethodName );

    BIP_String_StrcpyChar( hRequest->hRequestTarget, pTarget );

    hRequest->httpVersion = *pHttpVersion;

error:
    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_SetRequestLine */

/*****************************************************************************
 *  Retrieve the values of the "request-line" fields of the specified
 *  BIP_HttpRequest.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_GetRequestLine(
    BIP_HttpRequestHandle       hRequest,                  /*!< [in] Handle of BIP_HttpRequest to be accessed. */
    BIP_HttpRequestMethod      *pMethodEnum,               /*!< [out] Method name enum, if method not recognized then BIP_HttpRequestMethod_eUnknown. */
    const char                **ppMethodName,              /*!< [out] Optional address of caller's pointer to be set to the null-terminated method name. */
    const char                **ppTarget,                  /*!< [out] Address of caller's pointer to be set to the null-terminated request target. */
    BIP_HttpRequestHttpVersion *pHttpVersion               /*!< [out] Pointer to caller's BIP_HttpResponseHttpVersion structure. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    /* Give them the MethodEnum if they gave us somewhere to put it. */
    if (pMethodEnum)
    {
        *pMethodEnum = BIP_FromStr_BIP_HttpRequestMethod( BIP_String_GetString( hRequest->hMethod ));
    }

    /* Give them the MethodName poiner if they gave us somewhere to put it. */
    if (ppMethodName)
    {
        *ppMethodName = BIP_String_GetString( hRequest->hMethod );
    }

    /* Give them the RequestTarget pointer if they gave us somewhere to put it. */
    if (ppTarget)
    {
        *ppTarget = BIP_String_GetString( hRequest->hRequestTarget );
    }

    /* Give them the HttpVersion if they gave us somewhere to put it. */
    if (pHttpVersion)
    {
        *pHttpVersion = hRequest->httpVersion;
    }

error:
    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_GetRequestLine */

/*****************************************************************************
 *  Attach user-defined data to an HttpRequest.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_SetUserData(
    BIP_HttpRequestHandle hRequest,                        /*!< [in] Handle of BIP_HttpRequest to be printed. */
    void                 *pUserData                        /*!< [in]: Pointer to the user specific data. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    /* We don't do anything with the user-data, so don't worry about states. */

    B_Mutex_Lock( hRequest->hMutex );

    hRequest->pUserData = pUserData;

    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_SetUserData */

/*****************************************************************************
 *  Retrieve user-defined data from an HttpRequest.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_GetUserData(
    BIP_HttpRequestHandle hRequest,
    void                **ppUserData                       /*!< [out] Address of caller's variable where the user-data pointer will be placed. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    /* We don't do anything with the user-data, so don't worry about states. */

    B_Mutex_Lock( hRequest->hMutex );

    *ppUserData = hRequest->pUserData;

    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_GetUserData */

/*****************************************************************************
 *  Get the first or next HttpHeader following a given Header in the
 *  specified HttpRequest's list of headers.  Optionally, a header
 *  name can be specified to return the next header with the specified name.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_GetNextHeader(
    BIP_HttpRequestHandle hRequest,                        /*!< [in] Handle of the BIP_HttpRequest to be accessed. */
    BIP_HttpHeaderHandle  hCurrentHeader,                  /*!< [in] The Header from which to start.  Pass NULL to start from the beginning of the list. */
    const char           *pName,                           /*!< [in] If non-NULL, then return the next Header with a matching name.. */
    BIP_HttpHeaderHandle *phNextHeader,                    /*!< [out] Address of variable to receive the handle of the next header with the specified name. */
    const char          **ppNextHeaderValue                /*!< [out] Optional address of pointer that will be point to the header's value.
                                                          Pass NULL if not interested in the header's value. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    BDBG_ASSERT( phNextHeader );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    rc = BIP_HttpHeaderList_GetNextHeader( hRequest->hHeaderList, hCurrentHeader, pName, phNextHeader, ppNextHeaderValue );

error:
    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_GetNextHeader */

/*****************************************************************************
 *  Get the name & value of a Request's HttpHeader.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_GetHeaderNameValue(
    BIP_HttpRequestHandle hRequest,                        /*!< [in]  Handle of the BIP_HttpRequest to be accessed. */
    BIP_HttpHeaderHandle  hHeader,                         /*!< [in]  The header of interest. */
    const char          **ppName,                          /*!< [out] The address of a pointer that will be updated to point the Header's name as a null-terminated string. */
                                                           /*!<      This pointer will become invalid when it's Header is destroyed. */
    const char          **ppValue                          /*!< [out] The address of a pointer that will be updated to point the Header's value as a null-terminated string. */
                                                           /*!<      This pointer will become invalid when it's Header is destroyed. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    if (ppName)
    {
        rc = BIP_HttpHeaderList_GetHeaderName( hHeader, ppName );
    }
    if (ppValue)
    {
        rc = BIP_HttpHeaderList_GetHeaderValue( hHeader, ppValue );
    }

error:
    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_GetHeaderNameValue */

/*****************************************************************************
 *  Remove (and destroy) an HttpHeader from an HttpRequest.
 *****************************************************************************/
void BIP_HttpRequest_RemoveHeader(
    BIP_HttpRequestHandle hRequest,                        /*!< [in] Handle of BIP_HttpRequest to modified. */
    BIP_HttpHeaderHandle  hHeader                          /*!< [in] Handle of the Header to be removed/destroyed. */
    )
{
    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    BDBG_ASSERT( hHeader );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHeader %p: Entry..." BIP_MSG_PRE_ARG, (void *)hRequest ));

    BIP_HttpHeaderList_DestroyHeader( hHeader );

    return;
}

/*****************************************************************************
 *  Move an HttpHeader and place it before a specified HttpHeader.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_MoveHeader(
    BIP_HttpRequestHandle hRequest,                        /*!< [in] Handle of the BIP_HttpRequest to be updated. */
    BIP_HttpHeaderHandle  hHeader,                         /*!< [in] The header to be moved. */
    BIP_HttpHeaderHandle  hPutBeforeThisHeader             /*!< [in] The Header that hHeader will moved in front of. */
                                                           /*!<      If NULL, then hHeader will be placed at the end of the HttpRequest's header list. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    BDBG_ASSERT( hHeader );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHeader %p: Entry..." BIP_MSG_PRE_ARG, (void *)hRequest ));

    rc = BIP_HttpHeaderList_MoveHeader( hHeader, hPutBeforeThisHeader );

    return( rc );
}

/*****************************************************************************
 *  Set the value of a HttpRequest's HttpHeader.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_SetHeaderValue(
    BIP_HttpRequestHandle hRequest,                        /*!< [in] Handle of the BIP_HttpRequest to be updated. */
    BIP_HttpHeaderHandle  hHeader,                         /*!< [in] The header that will have its value changed. */
    const char           *pValue                           /*!< [in] A pointer to a null-terminated string containing the Header's new value. */
                                                           /*!<      Passing NULL will result in a header without a value.  */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    rc = BIP_HttpHeaderList_SetHeaderValue( hHeader, pValue );

error:
    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_SetHeaderValue */

/*****************************************************************************
 *  Add a new HttpHeader to an HttpRequest.
 *****************************************************************************/
BIP_HttpHeaderHandle BIP_HttpRequest_AddHeader(
    BIP_HttpRequestHandle hRequest,                        /*!< [in] Handle of BIP_HttpRequest to modified. */
    const char           *pName,                           /*!< [in] Pointer to a null-terminated string with the header's name. */
    const char           *pValue,                          /*!< [in] Optional pointer to a null-terminated string will be copied to the header's value. */
                                                           /*!<      If NULL, then the header will be created without a value. */
    BIP_HttpHeaderHandle  hAddBeforeThisHeader             /*!< [in] If a BIP_HeaderHandle is passed, and it exists in the specified hHttpRequest, */
                                                           /*!<      the newly created header will be positioned immediately in front of it. */
                                                           /*!<      If NULL, the new header will be placed at the end of the HttpRequest's headers. */
    )
{
    BIP_Status           rc      = BIP_SUCCESS;
    BIP_HttpHeaderHandle hHeader = NULL;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    BDBG_ASSERT( pName );
    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    hHeader = BIP_HttpHeaderList_CreateHeader( hRequest->hHeaderList, pName, pValue, hAddBeforeThisHeader );

error:
    B_Mutex_Unlock( hRequest->hMutex );
    if (rc) BDBG_ERR(( BIP_MSG_PRE_FMT "Request state error 0x%x" BIP_MSG_PRE_ARG,rc ));

    return( hHeader );
}                                                          /* BIP_HttpRequest_AddHeader */

/*****************************************************************************
 *  Clear an HttpRequest object.
 *****************************************************************************/
void BIP_HttpRequest_Clear_locked(
    BIP_HttpRequestHandle         hRequest,                /*!< [in] Handle of BIP_HttpRequest to be cleared. */
    BIP_HttpRequestClearSettings *pSettings                /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    )
{
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Entry..." BIP_MSG_PRE_ARG, (void *)hRequest ));

    BIP_SETTINGS_ASSERT( pSettings, BIP_HttpRequestClearSettings );

    /* Don't need to check for deserialize or serialize busy because
     * we're going to reset their states back to idle. */

    BIP_String_Clear( hRequest->hMethod );
    BIP_String_Clear( hRequest->hRequestTarget );
    hRequest->httpVersion = hRequest->createSettings.httpVersion;

    /* Destroy any BIP_HttpHeader objects that are in the Request's header list. */
    BIP_HttpHeaderList_Clear( hRequest->hHeaderList );

    hRequest->deserializeState = deserializeState_eIdle;
    hRequest->serializeState   = serializeState_eIdle;

    return;
}                                                          /* BIP_HttpRequest_Clear_locked */

/*****************************************************************************
 *  Clear an HttpRequest object.
 *****************************************************************************/
void BIP_HttpRequest_Clear(
    BIP_HttpRequestHandle         hRequest,                /*!< [in] Handle of BIP_HttpRequest to be cleared. */
    BIP_HttpRequestClearSettings *pSettings                /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    )
{
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Entry..." BIP_MSG_PRE_ARG, (void *)hRequest ));

    B_Mutex_Lock( hRequest->hMutex );

    BIP_SETTINGS_ASSERT( pSettings, BIP_HttpRequestClearSettings );

    BIP_HttpRequest_Clear_locked( hRequest, pSettings );

    B_Mutex_Unlock( hRequest->hMutex );
    return;
}

/*****************************************************************************
 *  Destroy a HeaderList, including any Headers that it contains.
 *****************************************************************************/
void BIP_HttpRequest_Destroy(
    BIP_HttpRequestHandle hRequest,
    void                 *ownerContext                     /*!< Same ownerContext used to Create the HeaderList. */
    )
{
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Entry..." BIP_MSG_PRE_ARG, (void *)hRequest ));

    if (ownerContext != hRequest->ownerContext)
    {
        BDBG_WRN(( BIP_MSG_PRE_FMT "hRequest %p: Mismatched ownerContext: got %p, expected %p.  Ignoring Destroy request!"
                   BIP_MSG_PRE_ARG, (void *)hRequest, ownerContext, hRequest->ownerContext ));
        return;
    }

    /* Free any custom header parsed structures. */
    if (hRequest->pRangeHeaderParsed) {B_Os_Free( hRequest->pRangeHeaderParsed ); }

    batom_factory_destroy( hRequest->factory );

    if (hRequest->hMutex)      {B_Mutex_Destroy( hRequest->hMutex ); }

    /* Destroy the HttpRequest's HttpHeaderList.*/
    if (hRequest->hHeaderList)
    {
        BIP_HttpHeaderList_Destroy(
            hRequest->hHeaderList,                         /* HeaderList handle. */
            &hRequest->createSettings );                   /* ownerContext, same as we used for Creating the HeaderList. */
    }
    if (hRequest->hRequestTarget)  {BIP_String_Destroy( hRequest->hRequestTarget ); }
    if (hRequest->hMethod)         {BIP_String_Destroy( hRequest->hMethod ); }

    BDBG_OBJECT_DESTROY( hRequest, BIP_HttpRequest );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Freeing object memory" BIP_MSG_PRE_ARG, (void *)hRequest ));
    B_Os_Free( hRequest );

    return;
}                                                          /* BIP_HttpRequest_Destroy */

/*****************************************************************************
 *  Create a new, empty HttpRequest.
 *****************************************************************************/
BIP_HttpRequestHandle BIP_HttpRequest_Create(
    void                          *ownerContext,           /*!< Any pointer, but the same pointer will be needed for destroying the Request. */
    BIP_HttpRequestCreateSettings *pCreateSettings         /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    )
{
    int                           rc;
    BIP_HttpRequestHandle         hRequest = NULL;
    BIP_HttpRequestCreateSettings defaultSettings;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    BIP_SETTINGS_ASSERT( pCreateSettings, BIP_HttpRequestCreateSettings );

    /* Allocate memory for the object. */
    hRequest = B_Os_Calloc( 1, sizeof( *hRequest ));
    BIP_CHECK_GOTO(( hRequest != NULL ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_OBJECT_SET( hRequest, BIP_HttpRequest );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Allocated " BIP_MSG_PRE_ARG, (void *)hRequest ));

    hRequest->ownerContext = ownerContext;

    /* Set our Create settings... either from caller or use defaults. */
    if (NULL == pCreateSettings)
    {
        BIP_HttpRequest_GetDefaultCreateSettings( &defaultSettings );
        pCreateSettings = &defaultSettings;
    }
    hRequest->createSettings = *pCreateSettings;

    /* Don't create BIP_Strings for hMethod or hRequestTarget and just
     * leave those handles NULL to indicate that they have not been set. */

    hRequest->hMethod = BIP_String_Create();
    BIP_CHECK_GOTO(( hRequest->hMethod ),         ( "BIP_String_Create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hRequest->hRequestTarget = BIP_String_Create();
    BIP_CHECK_GOTO(( hRequest->hRequestTarget ),  ( "BIP_String_Create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hRequest->httpVersion = hRequest->createSettings.httpVersion;
    hRequest->hHeaderList = BIP_HttpHeaderList_Create(
            &hRequest->createSettings );                   /* Pointer for ownerContext to prevent inadvertent destruction. */
    BIP_CHECK_GOTO(( hRequest->hHeaderList ),     ( "BIP_HttpHeaderList_Create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* coverity[missing_lock] */
    hRequest->deserializeState = deserializeState_eIdle;
    hRequest->serializeState   = serializeState_eIdle;

    hRequest->hMutex = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( hRequest->hMutex ),          ( "B_Mutex_Create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hRequest->factory = batom_factory_create( bkni_alloc, 3 ); /* 3 pre allocating atoms */
    BIP_CHECK_GOTO(( hRequest->factory ),        ( "batom_factory_create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    return( hRequest );

error:
    if (hRequest)
    {
        if (hRequest->factory)        {batom_factory_destroy( hRequest->factory ); }
        if (hRequest->hMutex)         {B_Mutex_Destroy( hRequest->hMutex ); }
        if (hRequest->hHeaderList)    {BIP_HttpHeaderList_Destroy( hRequest->hHeaderList, &hRequest->createSettings ); }
        if (hRequest->hRequestTarget) {BIP_String_Destroy( hRequest->hRequestTarget ); }
        if (hRequest->hMethod)        {BIP_String_Destroy( hRequest->hMethod ); }

        BDBG_OBJECT_DESTROY( hRequest, BIP_HttpRequest );

        B_Os_Free( hRequest );
    }

    return( NULL );
}                                                          /* BIP_HttpRequest_Create */

/*****************************************************************************
 *  Some private (static) functions used by the deserializer.
 *****************************************************************************/
static bool BIP_HttpRequest_IsTChar(
    char ch
    )
{
    char *tchar = "!#$%&'*+-.^_`|~";
    char *pCh;

    if (isalnum( ch )) {return( true ); }

    for (pCh = tchar; *pCh; pCh++) {
        if (*pCh == ch) {return( true ); }
    }
    return( false );
}

static bool BIP_HttpRequest_IsPChar(
    char ch
    )
{
    /*
     *  Make sure that the specified character is one allowed
     *  for the "pchar" type.  Note that this does not validate
     *  the sequence of "pct-encoded" characters.
     *
     *  pchar = unreserved / pct-encoded / sub-delims / ":" / "@"
     *      unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
     *      pct-encoded = "%" HEXDIG HEXDIG
     *      sub-delims =    "!" / "$" / "&" / "?" /
     *                       "(" / ")" / "*" / "+" /
     *                       "," / ";" / "="
     */

    /*            unreserved    pct-encoded    sub-delims      extra pchar */
    char *pchar =   "-._~"         "%"        "!$&?()*+,;="       ":@";
    char *pCh;

    if (isalnum( ch )) {return( true ); }                  /* Covers ALPHA, DIGIT, HEXDIG. */

    for (pCh = pchar; *pCh; pCh++) {
        if (*pCh == ch) {return( true ); }
    }
    return( false );
}

/*****************************************************************************
 *  Populate a BIP_HttpRequest object from an HTTP request message
 *  that is contained in one or more batom_cursors.
 *
 *  Refer to "HTTP-message" in RFC7230 for the structure of an HTTP
 *  request message.
 *
 *  Refer to /BSEAV/lib/utils/bioatom.h for information on the
 *  usage of the "batom" APIs and data types.
 *
 *  An single HTTP request can be spread across multiple batom_cursors.
 *  This API can be called sequentially to feed each of the cursors to
 *  the deserializer.  When an entire HTTP request has been deserialized,
 *  the *pDeserializeComplete output argument will be set to true, and the
 *  cursor will be left pointing to the first character of the message
 *  body (or payload) or the cursor will be at EOF.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_DeserializeFromAtom(
    BIP_HttpRequestHandle hRequest,                        /*!< [in]  The BIP_HttpRequest to be populated with the deserialized data. */
    batom_cursor         *pCursor,                         /*!< [in]  Cursor that contains incoming data. */
    bool                 *pDeserializeComplete             /*!< [out] Set to true if the http request has been completely deserialized. */
                                                           /*!<       i.e., it has terminating <CRNL><CRNL> or <NL><NL>. */
    )
{
    BIP_Status rc = BIP_SUCCESS;
    int        byte;                                       /* Byte from cursor or BATOM_EOF. */
    char       ch = '\0';
    bool       relook;                                     /* Set this to true to make another pass through the state machine. */

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );
    BDBG_ASSERT( pCursor );
    BDBG_ASSERT( pDeserializeComplete );

    B_Mutex_Lock( hRequest->hMutex );

    /* Can't start deserializing if we're in the middle of serializing. */
    if (hRequest->serializeState != serializeState_eIdle) {rc = BIP_ERR_SERIALIZE_IN_PROGRESS; goto error; }

    *pDeserializeComplete = false;                         /* Assume that the deserialize will not complete. */

    /* Loop for the state machine.  Setting "relook=true" will run the run
     * the state machine again to process a new state.
     * */
    for (relook = true; relook==true; ) {
        relook = false;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: deserializeState=0x%x \"%s\"  byteIndex=%zu"
                   BIP_MSG_PRE_ARG, (void *)hRequest, hRequest->deserializeState, toStr_deserializeState( hRequest->deserializeState ), hRequest->byteIndex ));

        switch (hRequest->deserializeState) {
            /* State: Idle
             * -----------
             * No deserialize in progress.  It's time to start a new deserialize session.
             * Input (ch) is the first byte of a message's header section. */
            case deserializeState_eIdle:
            {
                BIP_HttpRequest_Clear_locked( hRequest, NULL );

                /* Create our accumulator strings. */
                BIP_String_Clear( hRequest->hMethod );
                BIP_String_Clear( hRequest->hRequestTarget );

                /* Change to new state and process the input byte there. */
                hRequest->deserializeState = deserializeState_eMethod;
                relook = true;                             /* Make another pass through the state machine. */
                break;
            }

            /* State: Method
             * -------------------
             * Deserializing the Request's method (see "method" in RFC7320). */
            case deserializeState_eMethod:
            {
                while (( byte = batom_cursor_next( pCursor )) != BATOM_EOF)
                {
                    ch = byte;                             /* Convert from int to char. */

                    /* If input belongs to the method, append it to the hMethod BIP_String.  */
                    if (BIP_HttpRequest_IsTChar( ch ))
                    {
                        BIP_String_StrcatCharN( hRequest->hMethod, &ch, 1 );
                    }
                    /* If ch is the expected terminator (space) and the hMethod is non-empty, move to next state  */
                    else if (ch == ' ')                    /* Found end of "method". */
                    {
                        size_t length = BIP_String_GetLength( hRequest->hMethod );
                        BIP_CHECK_GOTO(( length>0 ), ( "Deserialize error: zero-length HTTP Request Method name." ), error, BIP_ERR_INVALID_PARAMETER, rc );

                        BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Got Method name=%s" BIP_MSG_PRE_ARG, (void *)hRequest, BIP_String_GetString( hRequest->hMethod )));

                        hRequest->deserializeState = deserializeState_eRequestTarget;
                        relook = true;
                        break;
                    }
                    /* Anything else is invalid. */
                    else
                    {
                        BIP_CHECK_GOTO(( false ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) in HTTP Request Method name.", ch, ch ), error, BIP_ERR_INVALID_PARAMETER, rc );
                    }
                }                                          /* End: For each byte in cursor... */
                break;
            }

            /* State: RequestTarget
             * --------------------
             * Deserializing the Request's request target (see "request-target" in RFC7320). */
            case deserializeState_eRequestTarget:
            {
                while (( byte = batom_cursor_next( pCursor )) != BATOM_EOF)
                {
                    ch = byte;                             /* Convert from int to char. */
                    /* If input belongs to the RequestTarget, append it to the hRequestTarget BIP_String.  */
                    if (BIP_HttpRequest_IsPChar( ch ) || ( ch == '/' ))
                    {
                        BIP_String_StrcatCharN( hRequest->hRequestTarget, &ch, 1 );
                    }
                    /* If ch is the expected terminator (SP) move to next state, even if the RequestTarget is
                       empty, which seems to be allowed (or if it's not allowed, we'll check for it later. */
                    else if (ch == ' ')                    /* Found end of "request-target". */
                    {
                        BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Got Request Target=\"%s\"" BIP_MSG_PRE_ARG, (void *)hRequest, BIP_String_GetString( hRequest->hRequestTarget )));

                        hRequest->deserializeState = deserializeState_eHttpVersion;
                        hRequest->byteIndex        = 0;
                        relook = true;
                        break;
                    }
                    /* Anything else is invalid. */
                    else
                    {
                        BIP_CHECK_GOTO(( false ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) in HTTP Request Target.", ch, ch ), error, BIP_ERR_INVALID_PARAMETER, rc );
                    }
                }                                          /* End: For each byte in cursor... */
                break;
            }

            /* State: HttpVersion
             * -------------------------
             * Deserializing the Request's HTTP version (see "HTTP-version" in RFC7320). */
            case deserializeState_eHttpVersion:
            {
                while (( byte = batom_cursor_next( pCursor )) != BATOM_EOF)
                {
                    const char httpVersionString[] = {'H', 'T', 'T', 'P', '/', 'x', '.', 'y'};
                    size_t     byteIndex           = hRequest->byteIndex;

                    ch = byte;                             /* Convert from int to char. */

                    /* Make sure our byteIndex is in range. */
                    BIP_CHECK_GOTO(( byteIndex<sizeof( httpVersionString )), ( "byteIndex=%zu is out of range during deserialize!", byteIndex ), error, BIP_ERR_INTERNAL, rc );

                    if (httpVersionString[byteIndex] == 'x')
                    {
                        BIP_CHECK_GOTO(( ch>='0' && ch<='9' ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) for HTTP major version number.", ch, ch ), error, BIP_ERR_INVALID_PARAMETER, rc );
                        hRequest->httpVersion.major = ch - '0';
                    }
                    else if (httpVersionString[byteIndex] == 'y')
                    {
                        BIP_CHECK_GOTO(( ch>='0' && ch<='9' ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) for HTTP minor version number.", ch, ch ), error, BIP_ERR_INVALID_PARAMETER, rc );
                        hRequest->httpVersion.minor = ch - '0';
                    }
                    else
                    {
                        BIP_CHECK_GOTO(( ch==httpVersionString[byteIndex] ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) at index %zu of HTTP Version.", ch, ch, byteIndex ), error, BIP_ERR_INVALID_PARAMETER, rc );
                    }

                    byteIndex++;
                    hRequest->byteIndex = byteIndex;

                    if (byteIndex >= sizeof( httpVersionString ))
                    {
                        hRequest->deserializeState = deserializeState_eRequestLineTerm;
                        hRequest->byteIndex        = 0;
                        relook = true;
                        break;
                    }
                }                                          /* End: For each byte in cursor... */
                break;
            }

            /* State: RequestLineTerm
             * -------------------------
             * Got a CR after the HTTP version. */
            case deserializeState_eRequestLineTerm:
            {
                while (( byte = batom_cursor_next( pCursor )) != BATOM_EOF)
                {
                    const char termChars[] = {CR, LF};
                    size_t     byteIndex   = hRequest->byteIndex;

                    ch = byte;                             /* Convert from int to char. */

                    /* Make sure our byteIndex is in range. */
                    BIP_CHECK_GOTO(( byteIndex<sizeof( termChars )), ( "byteIndex=%zu is out of range during deserialize!", byteIndex ), error, BIP_ERR_INTERNAL, rc );

                    BIP_CHECK_GOTO(( ch==termChars[byteIndex] ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) at index %zu of HTTP request-line terminator.", ch, ch, byteIndex ), error, BIP_ERR_INVALID_PARAMETER, rc );

                    byteIndex++;
                    hRequest->byteIndex = byteIndex;

                    if (byteIndex >= sizeof( termChars ))
                    {
                        hRequest->deserializeState = deserializeState_eHeaders;
                        relook = true;
                        break;
                    }
                }                                          /* End: For each byte in cursor... */
                break;
            }

            /* State: Headers
             * -------------------------
             * Now for the Headers... deserialize them into an HttpHeader object that's a part of the HttpRequest.  */
            case deserializeState_eHeaders:
            {
                bool deserializeComplete;

                rc = BIP_HttpHeaderList_DeserializeFromAtom( hRequest->hHeaderList, pCursor, &deserializeComplete );
                BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpHeader_DeserializeFromAtom() failed." ), error, rc, rc );
                if (deserializeComplete)
                {
                    hRequest->deserializeState = deserializeState_eDone;
                    relook = true;
                }
                break;
            }

            /* State: Done
             * -------------------------
             * Not much to do but stay here. */
            case deserializeState_eDone:
            {
                *pDeserializeComplete      = true;
                hRequest->deserializeState = deserializeState_eIdle;
                /* Don't set relook or we'll start deserializing again! */
                break;
            }

            /* State: Anything not handled.
             * -------------------------
             * This should never happen. */
            default:
            {
                BIP_CHECK_GOTO(( false ), ( "hRequest %p: Unhandled state=%s (%d)", (void *)hRequest, toStr_deserializeState( hRequest->deserializeState ), hRequest->deserializeState ),
                    error, BIP_ERR_INTERNAL, rc );

                break;
            }
        }                                                  /* End switch/case. */
    }

error:
    if (rc != BIP_SUCCESS)
    {
        BIP_String_Clear( hRequest->hMethod       );
        BIP_String_Clear( hRequest->hRequestTarget );
        hRequest->deserializeState = deserializeState_eIdle;
    }

    B_Mutex_Unlock( hRequest->hMutex );
    return( rc );
}                                                          /* BIP_HttpRequest_DeserializeFromAtom */

/*****************************************************************************
 *  API to deserialize a list of HTTP headers from a buffer into a
 *  BIP_HttpHeaderList.
 *
 *  Refer to:  BIP_HttpHeader_DeserializeFromAtom()
 *****************************************************************************/
BIP_Status BIP_HttpRequest_DeserializeFromBuffer(
    BIP_HttpRequestHandle hRequest,                        /*!< [in] The HttpRequest to be populated with the deserialized data. */
    const char           *buffer,                          /*!< [in] Buffer that contains incoming data. */
    size_t                bufferSize,                      /*!< [in] Input buffer size. */
    bool                 *pDeserializeComplete,            /*!< [out] Set to true if the http request is complete, ie it has */
                                                           /*!<       end of header(<CRNL><CRNL or <NL><NL>). */
    size_t               *pConsumedBytes                   /*!< [out] If request is complete then this specifies the consumed */
                                                           /*!<       data size.If message is not complete then this must be equal to bufferSize. */
    )
{
    BIP_Status   rc     = BIP_SUCCESS;
    batom_t      myAtom = NULL;
    batom_cursor myCursor;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );
    BDBG_ASSERT( buffer );
    BDBG_ASSERT( pDeserializeComplete );
    BDBG_ASSERT( pConsumedBytes );

    /*************************************************************************
     *  Set output args in case of early return..
     *************************************************************************/
    *pDeserializeComplete = false;                         /* Assume that the deserialize will not complete. */
    *pConsumedBytes       = 0;

    if (bufferSize == 0) {return( rc ); }                  /* Success!  Nothing to deserialize. */

    /*************************************************************************
     *  Create an atom from the buffer.
     *************************************************************************/
    myAtom = batom_from_range( hRequest->factory, buffer, bufferSize, NULL, NULL );
    batom_cursor_from_atom( &myCursor, myAtom );

    rc = BIP_HttpRequest_DeserializeFromAtom( hRequest, &myCursor, pDeserializeComplete );

    /* Tell the caller how many bytes we've consumed here! */
    *pConsumedBytes = batom_cursor_pos( &myCursor );

    batom_release( myAtom );
    return( rc );
}                                                          /* BIP_HttpRequest_DeserializeFromBuffer */

/*****************************************************************************
 *  Private function to assist with serializing.  It will copy as many bytes
 *  as possible from a source buffer to a destination buffer.  It copies
 *  until either the source is empty, or until the destination is full.
 *
 *  Also, if no destination buffer is specified (pDstbfr==NULL), nothing
 *  is copied to the destination, but the destination index (*pCurDstIdx)
 *  is incremented as usual.  This behavior is used to calculate the space
 *  required for serializing without having to generate the serial output.
 *
 *  Returns:
 *      BIP_SUCCESSS:         : all bytes of the source have been copied
 *                              to the destination.
 *
 *      BIP_INF_NOT_AVAILABLE : some source bytes were not copied
 *                              because the destination is full.
 *****************************************************************************/
static BIP_Status BIP_HttpRequest_SerializeChars(
    const char *pSrcBfr,                                   /* Pointer to source buffer to be output. */
    size_t      srcLen,                                    /* Size of source buffer.                 */
    size_t     *pCurSrcIdx,                                /* Index into source buffer for next byte */
    char       *pDstBfr,                                   /* Pointer to dest buffer to be filled.  */
    size_t      dstLen,                                    /* Size of destination buffer. */
    size_t     *pCurDstIdx
    )                                                      /* Index into dest buffer for next byte. */
{
    BIP_Status rc =  BIP_SUCCESS;
    size_t     bytesToMove;
    size_t     spaceLeft;

    bytesToMove = srcLen - *pCurSrcIdx;                    /* How man bytes left in source. */
    if (bytesToMove == 0)
    {
        *pCurSrcIdx = 0;
    }
    else
    {
        BDBG_ASSERT( *pCurSrcIdx < srcLen );

        spaceLeft = dstLen - *pCurDstIdx;                  /* How many bytes left in output buffer. */
        if (spaceLeft >= bytesToMove)                      /* If there's space for all of it... */
        {
            if (pDstBfr)
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "Copying \"%.*s\" to destIdx=%zu"
                           BIP_MSG_PRE_ARG, (int)bytesToMove, pSrcBfr + *pCurSrcIdx, *pCurDstIdx ));

                BKNI_Memcpy( pDstBfr + *pCurDstIdx, pSrcBfr+ *pCurSrcIdx, bytesToMove );
            }
            *pCurDstIdx += bytesToMove;
            *pCurSrcIdx  = 0;
        }
        else                                               /* Can't do all, just do what we can. */
        {
            if (spaceLeft > 0)
            {
                if (pDstBfr)
                {
                    BDBG_MSG(( BIP_MSG_PRE_FMT "Copying \"%.*s\" to destIdx=%zu"
                               BIP_MSG_PRE_ARG, (int)spaceLeft, pSrcBfr + *pCurSrcIdx, *pCurDstIdx ));
                    BKNI_Memcpy( pDstBfr + *pCurDstIdx, pSrcBfr + *pCurSrcIdx, spaceLeft );
                }
                *pCurDstIdx += spaceLeft;
                *pCurSrcIdx += spaceLeft;
            }
            rc = BIP_INF_NOT_AVAILABLE;                    /* No space available in output buffer */
        }
    }
    return( rc );
}                                                          /* BIP_HttpRequest_SerializeChars */

/*****************************************************************************
 *  Populate the caller's memory buffer (char array) with a "serialized"
 *  version of the HttpRequest that is suitable for sending over the network.
 *****************************************************************************/
static BIP_Status BIP_HttpRequest_SerializeToBuffer_locked(
    BIP_HttpRequestHandle hRequest,                        /*!< Handle of the HttpRequest to be serialized. */
    char                 *pBuffer,                         /*!< Buffer to receive the serialized output. */
    size_t                bufferSize,                      /*!< Buffer size. */
    bool                 *pSerializeComplete,              /*!< Set to true if serializing is complete. Otherwise, */
                                                           /*!< the buffer is full, call this again with a new buffer.  */
    size_t               *pSerializedBytes                 /*!< The number of bytes that have been placed in the buffer. */
    )
{
    BIP_Status rc        = BIP_SUCCESS;
    size_t     destIndex = 0;                              /* Index into callers buffer (pBuffer). */
    bool       relook;                                     /* Set this to true to make another pass through the state machine. */

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    /* BIP_HttpRequest_GetSerializeBufferSize() calls this function with
     * pBuffer set to NULL, so we'll go through the motions to compute the
     * final serialized size, without actually serializing anything. */
    if (pBuffer == NULL)
    {
        BDBG_ASSERT( bufferSize==0 );
        BDBG_ASSERT( pSerializedBytes );
        bufferSize = SSIZE_MAX;
    }
    else
    {
        BDBG_ASSERT( pSerializeComplete );
    }

    B_MUTEX_ASSERT_LOCKED( hRequest->hMutex );

    /* Can''t start serializing if we're in the middle of deserializing. */
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }

    if (pSerializeComplete) {*pSerializeComplete = false; } /* Assume that we won't finish the serialize in one call. */

    /*************************************************************************
     * Loop for the state machine.  Setting "relook=true" will run the run
     * the state machine again to process a new state.
     *************************************************************************/
    for (relook = true; relook==true; ) {
        relook = false;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: serializeState=0x%x \"%s\"  byteIndex=%zu"
                   BIP_MSG_PRE_ARG, (void *)hRequest, hRequest->serializeState, toStr_serializeState( hRequest->serializeState ), hRequest->byteIndex ));

        switch (hRequest->serializeState) {
            /* State: Idle
             * -----------
             * No serialize in progress.  It's time to start a new serializing session.
             * Initialize the serialize state. */
            case serializeState_eIdle:
            {
                hRequest->serializeState = serializeState_eMethod;
                hRequest->byteIndex      = 0;
                relook = true;
                break;
            }

            /* State: Method
             * -------------
             * Serialize the Request's HTTP Method: hRequest->hMethod.  */
            case serializeState_eMethod:
            {
                /* coverity[var_deref_op: FALSE] */
                const char *pChars = BIP_String_GetString( hRequest->hMethod );
                size_t      nChars = BIP_String_GetLength( hRequest->hMethod );

                rc = BIP_HttpRequest_SerializeChars( pChars, nChars, &hRequest->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hRequest->serializeState = serializeState_eMethodSpace;
                    relook = true;
                }
                break;
            }

            /* State: MethodSpace
             * ------------------
             * Add a space after the HTTP Method. */
            case serializeState_eMethodSpace:
            {
                const char pChars[] = {SP};                /* ASCII space character. */
                size_t     nChars   = sizeof( pChars );

                rc = BIP_HttpRequest_SerializeChars( pChars, nChars, &hRequest->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hRequest->serializeState = serializeState_eTarget;
                    relook = true;
                }
                break;
            }

            /* State: Target
             * -------------
             * Serialize the Request Target: hRequest->hRequestTarget.  */
            case serializeState_eTarget:
            {
                /* coverity[var_deref_op: FALSE] */
                const char *pChars = BIP_String_GetString( hRequest->hRequestTarget );
                size_t      nChars = BIP_String_GetLength( hRequest->hRequestTarget );

                rc = BIP_HttpRequest_SerializeChars( pChars, nChars, &hRequest->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hRequest->serializeState = serializeState_eTargetSpace;
                    relook = true;
                }
                break;
            }

            /* State: TargetSpace
             * ------------------
             * Serialize the space after the RequestTarget */
            case serializeState_eTargetSpace:
            {
                const char pChars[] = {SP};                /* ASCII space character. */
                size_t     nChars   = sizeof( pChars );

                rc = BIP_HttpRequest_SerializeChars( pChars, nChars, &hRequest->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hRequest->serializeState = serializeState_eVersionPrefix;
                    relook = true;
                }
                break;
            }

            /* State: VersionPrefix
             * --------------------
             * Serialize the constant prefix of the HTTP Version string ("HTTP/"). */
            case serializeState_eVersionPrefix:
            {
                const char pChars[] = {'H', 'T', 'T', 'P', '/'};
                size_t     nChars   = sizeof( pChars );

                rc = BIP_HttpRequest_SerializeChars( pChars, nChars, &hRequest->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hRequest->serializeState = serializeState_eVersionMajor;
                    relook = true;
                }
                break;
            }

            /* State: VersionMajor
             * -------------------
             * Serialize the HTTP major version number: hRequest->httpVersion.major. */
            case serializeState_eVersionMajor:
            {
                char   pChars[1];
                size_t nChars = sizeof( pChars );

                int major = hRequest->httpVersion.major;

                if (( major<0 ) || ( major>9 ))
                {
                    BDBG_ASSERT( false );
                    pChars[0] = '?';                       /* Yikes! Invalid major version!. */
                }
                else
                {
                    pChars[0] = major + '0';
                }

                rc = BIP_HttpRequest_SerializeChars( pChars, nChars, &hRequest->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hRequest->serializeState = serializeState_eVersionDot;
                    relook = true;
                }
                break;
            }

            /* State: VersionDot
             * -----------------
             * Serializing the dot (period) between the major and minor version numbers. */
            case serializeState_eVersionDot:
            {
                char   pChars[] = {'.'};
                size_t nChars   = sizeof( pChars );

                rc = BIP_HttpRequest_SerializeChars( pChars, nChars, &hRequest->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hRequest->serializeState = serializeState_eVersionMinor;
                    relook = true;
                }
                break;
            }

            /* State: VersionMinor
             * -------------------
             * Serialize the HTTP minor version number: hRequest->httpVersion.minor. */
            case serializeState_eVersionMinor:
            {
                char   pChars[1];
                size_t nChars = sizeof( pChars );

                int minor = hRequest->httpVersion.minor;

                if (( minor<0 ) || ( minor>9 ))
                {
                    BDBG_ASSERT( false );
                    pChars[0] = '?';                       /* Yikes! Invalid major version!. */
                }
                else
                {
                    pChars[0] = minor + '0';
                }

                rc = BIP_HttpRequest_SerializeChars( pChars, nChars, &hRequest->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hRequest->serializeState = serializeState_eRequestLineTerm;
                    relook = true;
                }
                break;
            }

            /* State: RequestLineTerm
             * ----------------------
             * Serialize the terminating CRLF at the end of the request-line. */
            case serializeState_eRequestLineTerm:
            {
                const char pChars[] = {CR, LF};
                size_t     nChars   = sizeof( pChars );

                rc = BIP_HttpRequest_SerializeChars( pChars, nChars, &hRequest->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hRequest->serializeState = serializeState_eHeaders;
                    relook = true;
                }
                break;
            }

            /* State: Headers
             * --------------
             * Serializing the Request's list of BIP_HttpHeaders . */
            case serializeState_eHeaders:
            {
                bool   serializeComplete;
                size_t serializedBytes;

                if (pBuffer)                               /* If we have a real buffer, then serialize the headers into it. */
                {
                    rc = BIP_HttpHeaderList_SerializeToBuffer( hRequest->hHeaderList, pBuffer+destIndex, bufferSize-destIndex, &serializeComplete, &serializedBytes );
                    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpHeader_SerializeToBuffer() failed" ), error, rc, rc );
                }
                else                                       /* If no buffer, we just need the size of the serialize. */
                {
                    rc                = BIP_HttpHeaderList_GetSerializeBufferSize( hRequest->hHeaderList, &serializedBytes );
                    serializeComplete = true;              /* _GetSerializeBufferSize() always completes in one call. */
                }

                destIndex += serializedBytes;

                if (serializeComplete)
                {
                    hRequest->serializeState = serializeState_eDone;
                    relook = true;
                }
                break;
            }

            /* State: Done
             * -----------
             * Serializing is complete. */
            case serializeState_eDone:
            {
                if (pSerializeComplete) {*pSerializeComplete = true; }
                hRequest->serializeState = serializeState_eIdle;
                break;
            }

            /* State: Anything not handled.
             * ----------------------------
             * This should never happen. */
            default:
            {
                BIP_CHECK_GOTO(( false ), ( "hRequest %p: Unhandled state=%s (%d)", (void *)hRequest, toStr_serializeState( hRequest->serializeState ), hRequest->serializeState ),
                    error, BIP_ERR_INTERNAL, rc );
                break;
            }
        }                                                  /* End switch/case. */
    }                                                      /* End Do while relook is true. */

error:                                                     /* todo: Return BIP_Status on error!. */
    if (pSerializedBytes) {*pSerializedBytes = destIndex; }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: returning *pSerializeComplete=%s *pSerializedBytes=%zu"
               BIP_MSG_PRE_ARG, (void *)hRequest,
               ( pSerializeComplete==NULL ) ? "N/A" : ( *pSerializeComplete ) ? "TRUE" : "FALSE",
               pSerializedBytes ? *pSerializedBytes : 0 ));

    return( BIP_SUCCESS );
}                                                          /* BIP_HttpRequest_SerializeToBuffer_locked */

/*****************************************************************************
 *  Populate the caller's memory buffer (char array) with a "serialized"
 *  version of the HttpRequest that is suitable for sending over the network.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_SerializeToBuffer(
    BIP_HttpRequestHandle hRequest,                        /*!< Handle of the HttpRequest to be serialized. */
    char                 *pBuffer,                         /*!< Buffer to receive the serialized output. */
    size_t                bufferSize,                      /*!< Buffer size. */
    bool                 *pSerializeComplete,              /*!< Set to true if serializing is complete. Otherwise, */
                                                           /*!< the buffer is full, call this again with a new buffer.  */
    size_t               *pSerializedBytes                 /*!< The number of bytes that have been placed in the buffer. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    B_Mutex_Lock( hRequest->hMutex );

    rc = BIP_HttpRequest_SerializeToBuffer_locked( hRequest, pBuffer, bufferSize, pSerializeComplete, pSerializedBytes );

    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}

/*****************************************************************************
 *  Determine the number of bytes that a BIP_HttpRequest will occupy
 *  when it is serialized (converted to one long character string for
 *  transmission over the network).
 *****************************************************************************/
BIP_Status BIP_HttpRequest_GetSerializeBufferSize(
    BIP_HttpRequestHandle hRequest,                        /*!< Handle of HttpRequest to be serialized. */
    size_t               *pSerializeBufferSize             /*!< Address of variable to receive the buffer size */
                                                           /*!< required for serializing the HttpRequest. */
    )
{
    BIP_Status rc;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );
    BDBG_ASSERT( pSerializeBufferSize );

    rc = BIP_HttpRequest_SerializeToBuffer( hRequest, NULL, 0, NULL, pSerializeBufferSize );
    BIP_CHECK_LOGERR(( rc==BIP_SUCCESS ), ( "BIP_HttpHeader_SerializeToBuffer failed" ), rc, rc );

    return( rc );
}

/*****************************************************************************
 *  API to serialize a BIP_HttpRequest to an atom.
 *
 *  Produce a "batom" that contains a serialized version of the HttpRequest
 *  that is suitable for sending over the network.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_SerializeToAtom(
    BIP_HttpRequestHandle hRequest,                        /*!< [in]  Handle of HttpRequest to be serialized. */
    batom_factory_t       factory,                         /*!< [in]  Factory to use for creating atom . */
    batom_t              *pAtom                            /*!< [out] Where to put the atom containing the serialized data. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    size_t  serializeLen      = 0;
    batom_t myAtom            = NULL;
    char   *pBuffer           = NULL;
    bool    serializeComplete = false;
    size_t  serializedBytes;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );
    BDBG_ASSERT( pAtom );

    B_Mutex_Lock( hRequest->hMutex );

    /* Can''t start serializing if we're in the middle of deserializing. */
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }

    /*************************************************************************
     *  Figure out how long the serialized request will be.
     *************************************************************************/
    rc = BIP_HttpRequest_SerializeToBuffer_locked( hRequest, NULL, 0, NULL, &serializeLen );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpRequest_SerializeToBuffer_locked failed." ), error, rc, rc );

    /*************************************************************************
     *  Malloc some memory, then create an atom out of it.
     *************************************************************************/
    pBuffer = B_Os_Malloc( serializeLen );
    BIP_CHECK_GOTO(( pBuffer ), ( "B_Os_Malloc (%zu bytes) failed.", serializeLen ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    myAtom = BIP_Atom_AtomFromBOsMallocRange( factory, pBuffer, serializeLen );

    /*************************************************************************
     *  Now serialize into the atom.
     *************************************************************************/
    rc = BIP_HttpRequest_SerializeToBuffer_locked( hRequest, pBuffer, serializeLen, &serializeComplete, &serializedBytes );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpRequest_SerializeToBuffer failed." ), error, rc, rc );

    BDBG_ASSERT( serializeComplete );                      /* The buffer was big enough, serialize should be complete. */
    BDBG_ASSERT( serializedBytes == serializeLen );        /* We should have filled the whole buffer. */

    *pAtom = myAtom;
    B_Mutex_Unlock( hRequest->hMutex );
    return( BIP_SUCCESS );

error:

    if (myAtom) {batom_release( myAtom ); }                /* This will free the buffer as well. */
    else if (pBuffer)
    {
        B_Os_Free( pBuffer );
    }

    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_SerializeToAtom */

/*****************************************************************************
 *  API to print BIP_HttpRequest using BDBG_LOG.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_Print(
    BIP_HttpRequestHandle         hRequest,                /*!< [in] Handle of BIP_HttpRequest to be printed. */
    const char                   *pLineHeading,            /*!< [in] If non-NULL, will be prefixed to each line printed. */
    BIP_HttpRequestPrintSettings *pSettings                /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    size_t serializeLen      = 0;
    char  *pBuffer           = NULL;
    bool   serializeComplete = false;
    size_t serializedBytes;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );
    BIP_SETTINGS_ASSERT( pSettings, BIP_HttpRequestPrintSettings );

    B_Mutex_Lock( hRequest->hMutex );

    /* Skip the check of the deserialize state.  Even if we're in the middle of deserializing, just
     * print what we have so far. */

    if (pLineHeading == NULL)
    {
        pLineHeading = "HTTP Request:";
    }

    /*************************************************************************
     *  Figure out how long the serialized request will be.
     *************************************************************************/
    rc = BIP_HttpRequest_SerializeToBuffer_locked( hRequest, NULL, 0, NULL, &serializeLen );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpRequest_SerializeToBuffer_locked failed." ), error, rc, rc );

    serializeLen += 1;                                     /* Allow for NULL-terminator. */

    pBuffer = B_Os_Malloc( serializeLen );
    BIP_CHECK_GOTO(( pBuffer ), ( "B_Os_Malloc (%zu bytes) failed.", serializeLen ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /*************************************************************************
     *  Now serialize into the buffer.
     *************************************************************************/
    rc = BIP_HttpRequest_SerializeToBuffer_locked( hRequest, pBuffer, serializeLen, &serializeComplete, &serializedBytes );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpRequest_SerializeToBuffer failed." ), error, rc, rc );

    BDBG_ASSERT( serializeComplete );                      /* The buffer was big enough, serialize should be complete. */
    BDBG_ASSERT( serializedBytes == serializeLen - 1 );    /* We should have filled the whole buffer. */

    pBuffer[serializedBytes] = '\0';                       /* NULL-terminate the serialize buffer. */

    {
        char        *pLineStart = pBuffer;
        size_t       len;
        char         lineEndChars[]  = "\r\n";
        char         separatorLine[] =     "------------------------------------------------------------------------";
        const size_t maxLineLen      = 70;                                                      /* Add artificial linebreak after this many characters. */

        BIP_MSG_LOG(( BIP_MSG_PRE_FMT "%s %s" BIP_MSG_PRE_ARG, pLineHeading, separatorLine  )); /* Print header line. */
        while (*pLineStart != '\0') {
            bool truncated = false;

            len = strcspn( pLineStart, lineEndChars );     /* Find length of printable line. */

            if (len > maxLineLen)
            {
                len       = maxLineLen - 4;                /* Allow for trailing "...". */
                truncated = true;
            }
            if (len > 0)                                   /* Indicate wrapped lines by appending "...". */
            {
                BIP_MSG_LOG(( BIP_MSG_PRE_FMT "%s \"%.*s\"%s" BIP_MSG_PRE_ARG, pLineHeading, (int)len, pLineStart, truncated ? "..." : "" ));
            }
            pLineStart += len;
            len         = strspn( pLineStart, lineEndChars );                                   /* Find length of line ending. */
            pLineStart += len;                                                                  /* Skip over line ending.      */
        }
        BIP_MSG_LOG(( BIP_MSG_PRE_FMT "%s %s" BIP_MSG_PRE_ARG, pLineHeading, separatorLine  )); /* Print header line. */
    }

error:
    if (pBuffer) {B_Os_Free( pBuffer ); }
    B_Mutex_Unlock( hRequest->hMutex );

    return( rc );
}                                                          /* BIP_HttpRequest_Print */

/*****************************************************************************
 *  Some private (static) functions used by the deserializer.
 *****************************************************************************/
static bool BIP_HttpRequest_IsOws(
    char ch
    )                                                      /* OWS: Optional White Space. */
{
    if (( ch==SP ) ||                                      /* ASCII space */
        ( ch==HTAB ))                                      /* ASCII tab   */
    {
        return( true );
    }

    return( false );
}

/*****************************************************************************
 *  Local function to parse one range spec from a byte Range header.
 *  Refer to "Range" in RFC7233.
 *****************************************************************************/
static BIP_Status BIP_HttpRequest_ParseRangeSpec(
    const char                                 *pCh,       /* Pointer to string to parse (might not be null-terminated). */
    size_t                                      lenCh,     /* Length of string that pCh points to. */
    size_t                                     *pIdx,      /* [in/out] Current index in to pCh. */
    struct BIP_HttpRequestRangeHeaderByteRange *pByteRange /* Optional pointer to struct to be populated with next byte range spec. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    size_t   idx          = *pIdx;
    uint64_t suffixLength = 0;
    uint64_t firstBytePos = 0;
    uint64_t lastBytePos  = 0;

    /* pCh is pointing somewhere in a "byte-range-set".
     * Skip over any leading OWS (optional white space) or commas. */
    for (; idx<lenCh; idx++) {
        if (BIP_HttpRequest_IsOws( pCh[idx] )) {continue; }
        if (pCh[idx] == ',')                    {continue; }

        break;                                             /* Found some other character, time to stop. */
    }

    /* If we came to the end of the string, this is
     * the normal way to end the parsing of the range-set. */
    if (idx >= lenCh)
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Found END OF STRING at idx=%zu"
                   BIP_MSG_PRE_ARG, idx ));
        rc = BIP_INF_NOT_AVAILABLE;
        goto done;
    }

    /* Now we should be at the start of a "byte-range-spec" or a
     * "suffix-byte-range-spec".
     * A byte-range-spec starts with a digit. */
    if (isdigit( pCh[idx] ))
    {
        /* byte-range-spec:
         * ---------------- */
        BDBG_MSG(( BIP_MSG_PRE_FMT "Found byte-range-spec at idx=%zu"
                   BIP_MSG_PRE_ARG, idx ));

        /* Found a byte-range-spec.  First field is first-byte-pos.
         * Convert the digits to binary and save for later. */
        for (; idx<lenCh; idx++) {
            uint64_t val;
            if (!isdigit( pCh[idx] )) {break; }            /* Non-digit, end of first-byte-pos. */

            val = pCh[idx] - '0';
            if (firstBytePos > ( BIP_U64_MAX-val )/10)     /* Check for integer overflow. */
            {
                goto parseError;
            }
            firstBytePos = ( firstBytePos*10 ) + val;
        }

        /* Came to non-digit, must be done with this first-byte-pos. */
        BDBG_MSG(( BIP_MSG_PRE_FMT "first-byte-pos value=%"PRIu64
                   BIP_MSG_PRE_ARG, firstBytePos ));

        if (idx >= lenCh) {goto parseError; }              /* byte-range-spec can't end with "first-byte-pos". */

        /* After first-byte-pos, a "-" is required. */
        if (pCh[idx] != '-') {goto parseError; }           /* Need the minus sign. */
        idx++;                                             /* Skip over "-". */

        /* Now we might be at the optional last-byte-pos
         * (if it's there).  */
        if (( idx >= lenCh ) || !isdigit( pCh[idx] ))
        {
            lastBytePos = BIP_U64_MAX;                     /* Indicate that last-byte-pos is not specified. */
        }
        else
        {
            for (; idx<lenCh; idx++) {
                uint64_t val;
                if (!isdigit( pCh[idx] )) {break; }

                val = pCh[idx] - '0';
                if (lastBytePos > ( BIP_U64_MAX-val )/10)  /* Check for integer overflow. */
                {
                    goto parseError;
                }
                lastBytePos = ( lastBytePos*10 ) + val;
            }
        }
        /* Came to non-digit, must be done with this last-byte-pos. */
        BDBG_MSG(( BIP_MSG_PRE_FMT "last-byte-pos value=%"PRIu64 "(0x%016"PRIu64 ")"
                   BIP_MSG_PRE_ARG, lastBytePos, lastBytePos ));

        if (pByteRange)
        {
            pByteRange->isSuffixByteRange                = false;
            pByteRange->specs.byteRangeSpec.firstBytePos = firstBytePos;
            pByteRange->specs.byteRangeSpec.lastBytePos  = lastBytePos;
        }
    }
    /* See if we have a suffix-byte-range-spec. */
    else if (pCh[idx] == '-')
    {
        /* suffix-byte-range-spec:
         * ---------------- */
        BDBG_MSG(( BIP_MSG_PRE_FMT "Found suffix-byte-range-spec at idx=%zu"
                   BIP_MSG_PRE_ARG, idx ));

        idx++;                                             /* Skip over leading "-". */
        if (idx >= lenCh) {goto parseError; }              /* "-" without digits not allowed. */
        if (!isdigit( pCh[idx] )) {goto parseError; }  /* Need at least 1 digit. */

        for (; idx<lenCh; idx++) {
            uint64_t val;
            if (!isdigit( pCh[idx] )) {break; }

            val = pCh[idx] - '0';
            if (suffixLength > ( BIP_U64_MAX-val )/10)     /* Check for integer overflow. */
            {
                goto parseError;
            }
            suffixLength = ( suffixLength*10 ) + val;
        }

        /* Came to non-digit, must be finished with this range-spec. */
        BDBG_MSG(( BIP_MSG_PRE_FMT "suffix-byte-range-spec value=%"PRIu64
                   BIP_MSG_PRE_ARG, suffixLength ));
        if (pByteRange)
        {
            pByteRange->isSuffixByteRange = true;
            pByteRange->specs.suffixByteRangeSpec.suffixLength = suffixLength;
        }
    }
    else
    {
        goto parseError;
    }

done:
    *pIdx = idx;
    return( rc );

parseError:

    BDBG_ERR(( "Error parsing Range header: \"%.*s\"", (int)lenCh, pCh ));
    BDBG_ERR(( "Parse failed here:          %*s^", (int)idx, "" ));
    *pIdx = idx;
    return( BIP_ERR_HTTP_MESSAGE_INVALID );
}                                                          /* BIP_HttpRequest_ParseRangeSpec */

/*****************************************************************************
 *  API to either count or parse range specs.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_ParseRangeHeaderForBytes_locked(
    BIP_HttpRequestHandle                     hRequest,           /*!< [in]  Handle of the HttpRequest to be accessed. */
    BIP_HttpRequestParsedRangeHeaderForBytes *pRangeHeaderParsed, /*!< [out] Address of caller's pointer variable that
                                                                                  will point to the parsed results after return. */
    unsigned                                 *pRangeSpecCount     /*!< [out] Address of caller's unsigned variable that
                                                                                  will be set to the number range specs. */
    )
{
    BIP_Status           rc             = BIP_SUCCESS;
    BIP_HttpHeaderHandle hHeader        = NULL;
    unsigned             rangeSpecIndex = 0;
    const char           bytesUnits[]   = "bytes=";
    size_t               bytesUnitsLen  = sizeof ( bytesUnits ) - 1; /* -1 to exclude null terminator. */
    size_t               rangeHeaderLen;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    /* Loop through each Range header that we have. */
    while (true) {
        const char *pRangeHeader = NULL;
        size_t      idxCh        = 0;                      /* Index into the Range Header string. */

        rc = BIP_HttpHeaderList_GetNextHeader( hRequest->hHeaderList, hHeader, "Range", &hHeader, &pRangeHeader );
        if (rc == BIP_INF_NOT_AVAILABLE) {break; }
        BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_HttpRequest_GetHeader_locked() failed" ), error, rc, rc );

        BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Found Range header %p : \"%s\""
                   BIP_MSG_PRE_ARG, (void *)hRequest, (void *)hHeader, pRangeHeader ));

        if (strncmp( bytesUnits, &pRangeHeader[idxCh], bytesUnitsLen ) != 0)
        {
            BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Skipping non-byte Range header %p : \"%s\""
                       BIP_MSG_PRE_ARG, (void *)hRequest, (void *)hHeader, pRangeHeader ));
            continue;
        }

        /* Find the length of the header. */
        rangeHeaderLen = strlen( pRangeHeader );

        /* Move past the "bytes=" prefix. */
        idxCh += bytesUnitsLen;

        /* Loop through each byte-range-spec in the byte-range-set. */
        while (true) {
            struct BIP_HttpRequestRangeHeaderByteRange *pRangeSpec = NULL;

            if (pRangeHeaderParsed)
            {
                pRangeSpec = &( pRangeHeaderParsed->pByteRangeSet[rangeSpecIndex] );
                pRangeHeaderParsed->pByteRangeSet[rangeSpecIndex].hHeader = hHeader;
            }

            rc = BIP_HttpRequest_ParseRangeSpec( pRangeHeader, rangeHeaderLen, &idxCh, pRangeSpec );
            if (rc == BIP_INF_NOT_AVAILABLE) {break; }
            BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "parseRangeSpec() failed" ), error, rc, rc );
            rangeSpecIndex++;
        }
    }

    if (pRangeSpecCount)
    {
        *pRangeSpecCount = rangeSpecIndex;
    }

    return( BIP_SUCCESS );

error:
    return( rc );
}                                                          /* BIP_HttpRequest_ParseRangeHeaderForBytes_locked */

/*****************************************************************************
 *  API to parse byte range headers into structure.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_ParseRangeHeaderForBytes(
    BIP_HttpRequestHandle                      hRequest,           /*!< [in]  Handle of the HttpRequest to be accessed. */
    BIP_HttpRequestParsedRangeHeaderForBytes **ppRangeHeaderParsed /*!< [out] Address of caller's pointer variable that
                                                                                   will point to the parsed results after return. */
    )
{
    BIP_Status rc             = BIP_SUCCESS;
    unsigned   rangeSpecCount = 0;
    BIP_HttpRequestParsedRangeHeaderForBytes *pRangeHeaderParsed = NULL;

    BDBG_ASSERT( hRequest );
    BDBG_OBJECT_ASSERT( hRequest, BIP_HttpRequest );

    B_Mutex_Lock( hRequest->hMutex );
    if (hRequest->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hRequest->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    /* If this HttpRequest already has a parsed structure, free it. */
    if (hRequest->pRangeHeaderParsed)
    {
        B_Os_Free( hRequest->pRangeHeaderParsed ); hRequest->pRangeHeaderParsed = NULL;
    }

    /* Parse the Range headers so we can find the count of range specs. */
    rc = BIP_HttpRequest_ParseRangeHeaderForBytes_locked( hRequest, NULL, &rangeSpecCount );
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_HttpRequest_ParseRangeHeaderForBytes_locked() failed" ), error, rc, rc );

    if (rangeSpecCount == 0)
    {
        rc = BIP_INF_NOT_AVAILABLE;
        goto done;
    }

    /* Allocate the BIP_HttpRequestParsedRangeHeaderForBytes structure
     * that we'll give back to the caller. */
    pRangeHeaderParsed = B_Os_Calloc( 1, sizeof( *pRangeHeaderParsed ) +
            rangeSpecCount * sizeof( *( pRangeHeaderParsed->pByteRangeSet )));
    BIP_CHECK_GOTO(( pRangeHeaderParsed != NULL ), ( "B_Os_Malloc() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /* Set the pointer to the variable array at the end of the struct. */
    pRangeHeaderParsed->pByteRangeSet = (void *)( pRangeHeaderParsed + 1 );

    /* Set the count of range specs. */
    pRangeHeaderParsed->byteRangeCount = rangeSpecCount;

    /* Now parse again to populate the structure. */
    rc = BIP_HttpRequest_ParseRangeHeaderForBytes_locked( hRequest, pRangeHeaderParsed, NULL );
    BIP_CHECK_GOTO(( rc == BIP_SUCCESS ), ( "BIP_HttpRequest_ParseRangeHeaderForBytes_locked() failed" ), error, rc, rc );

    if (ppRangeHeaderParsed)
    {
        *ppRangeHeaderParsed = pRangeHeaderParsed;
    }

    hRequest->pRangeHeaderParsed = pRangeHeaderParsed;

done:
    B_Mutex_Unlock( hRequest->hMutex );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Returning: "BIP_STATUS_FMT "" BIP_MSG_PRE_ARG, (void *)hRequest, BIP_STATUS_ARG( rc )));
    return( rc );

error:
    if (pRangeHeaderParsed) {B_Os_Free( pRangeHeaderParsed ); }

    B_Mutex_Unlock( hRequest->hMutex );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: [Error] Returning: "BIP_STATUS_FMT "" BIP_MSG_PRE_ARG, (void *)hRequest, BIP_STATUS_ARG( rc )));
    return( rc );
}                                                          /* BIP_HttpRequest_ParseRangeHeaderForBytes */

/*****************************************************************************
 *  API to get a valid startOffset and length for a ByteRange entry.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_GetRangeEntryOffset(
    BIP_HttpRequestRangeHeaderByteRange *pRange,           /*!< [in] Caller's struct containing a pointer to a valid range spec entry */
    uint64_t                             contentLength,    /*!< [in] ContentLength from caller, if it is 0 then that indicates caller doesn't know the contentLength */
                                                           /*!<      and this api won't be able to perform validation on range spec's firstBytePos and lastBytePos. */
    uint64_t                            *pStartOffset,     /*!< [out] Callers variable to receive start byte offset. Caller need to send out data from this position. */
    uint64_t                            *pLength           /*!< [out] Caller's variable to receive length of range, i.e the amount of data that need to be sent out from the */
                                                           /*!<       startOffset. If contentLength us unknown and Range spec also doesn't have a valid lastBytePos then this will be set to 0. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    if (pRange->isSuffixByteRange == false)
    {
        if (( pRange->specs.byteRangeSpec.firstBytePos > pRange->specs.byteRangeSpec.lastBytePos ) && ( pRange->specs.byteRangeSpec.lastBytePos != BIP_U64_MAX ))
        {
            /* Basic validation to check whether firstBytePos is greater than lastBytesPos when lastBytePos has a valid value. */
            rc = BIP_ERR_INVALID_PARAMETER;
            goto error;
        }

        if (contentLength != 0)
        {
            /* Evaluate firstBytePos */
            if (pRange->specs.byteRangeSpec.firstBytePos >= contentLength)
            {
                rc = BIP_ERR_INVALID_PARAMETER;
                goto error;
            }
            else
            {
                *pStartOffset = pRange->specs.byteRangeSpec.firstBytePos;
            }

            /* Now evaluate lastBytePos and set pLength appropriately. */
            if (pRange->specs.byteRangeSpec.lastBytePos >= contentLength)
            {
                /* Incase lastBytePos is invalid that is BIP_U64_MAX,then it will be automatically checked here since contentLength can have a max valid value as BIP_U64_MAX.*/
                *pLength = ( contentLength - pRange->specs.byteRangeSpec.firstBytePos );
            }
            else
            {
                *pLength = ( pRange->specs.byteRangeSpec.lastBytePos + 1 - pRange->specs.byteRangeSpec.firstBytePos );
            }
        }
        else                                               /* contentLength is 0, that is at that calling instant contentLength is unknown to the caller.*/
        {
            /* In this case we can't perform any validation.*/
            *pStartOffset =  pRange->specs.byteRangeSpec.firstBytePos;
            if (pRange->specs.byteRangeSpec.lastBytePos != BIP_U64_MAX)
            {
                *pLength = ( pRange->specs.byteRangeSpec.lastBytePos + 1 - pRange->specs.byteRangeSpec.firstBytePos );
            }
            else                                           /* lastBytePos == BIP_U64_MAX */
            {
                /* If lastBytePos is unknown then in this case though we know thye startOffset but we can't determine the *pLength.*/
                *pLength = 0;
                rc       = BIP_INF_NOT_AVAILABLE;
            }
        }
    }
    else                                                   /* pRange->isSuffixByteRange == true */
    {
        if (contentLength != 0)
        {
            if (pRange->specs.suffixByteRangeSpec.suffixLength <= contentLength)
            {
                *pStartOffset = ( contentLength - pRange->specs.suffixByteRangeSpec.suffixLength );
                *pLength      = pRange->specs.suffixByteRangeSpec.suffixLength;
            }
            else
            {
                /*Refer to rfc7233: If the selected representation is shorter than the specified suffix-length, the entire representation is used.*/
                *pStartOffset = 0;
                *pLength      = contentLength;
            }
        }
        else
        {
            /* In this case we don't have contentLength so we can't derive a startOffset and Length. */
            *pStartOffset = 0;
            *pLength      = 0;
            rc            = BIP_INF_NOT_AVAILABLE;
        }
    }

error:
    return( rc );
}                                                          /* BIP_HttpRequest_GetRangeEntryOffset */

/*****************************************************************************
 *  Local function to parse an "npt-time" from a "TimeSeekRange.dlna.org"                                        .
 *  header.
 *****************************************************************************/
static BIP_Status BIP_HttpRequest_ParseNptTime(
    batom_cursor *pCursor,                                 /* Pointer to string to parse (might not be null-terminated). */
    int64_t      *pNptTimeInMs                              /* Caller's variable to receive the result. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    int64_t parsedValue;

    int64_t hoursValue    = 0;
    int64_t minutesValue  = 0;
    int64_t secondsValue  = 0;
    int64_t msecondsValue = 0;

    int          ch;
    batom_cursor startCursor;

    batom_cursor_clone( &startCursor, pCursor );

    /* Now we should be at the start of an "npt-start-time". The format might be either
     * <seconds>[.<fraction>] or <hours>:<minutes>:<seconds>[.fraction>].   We won't know
     * which format until after we parse the first integer. */
    parsedValue = 0;
    for (; isdigit( ch = batom_cursor_next( pCursor )); )
    {
        int64_t val;

        val = ch - '0';
        /* BDBG_MSG(( BIP_MSG_PRE_FMT "parsedValue: %"PRId64"  (BIP_I64_MAX-val)/10 : %"PRId64"" BIP_MSG_PRE_ARG, parsedValue, (BIP_I64_MAX-val)/10 )); */

        if (parsedValue > (( BIP_I64_MAX-val )/10 ))       /* Check for integer overflow. */
        {
            goto overflow;
        }
        parsedValue = ( parsedValue*10 ) + val;
    }
    BDBG_MSG(( BIP_MSG_PRE_FMT "parsedValue: %" PRId64 "" BIP_MSG_PRE_ARG, parsedValue ));

    /* If we've come to a period, we must have this format: "<seconds>[.<fraction>]". */
    if (ch == '.')
    {
        batom_cursor decimalCursor;
        int32_t     decimalFactor = 100;

        batom_cursor_clone( &decimalCursor, pCursor );

        BDBG_MSG(( BIP_MSG_PRE_FMT "Found '.', npt format is \"<seconds>.<fraction>\"" BIP_MSG_PRE_ARG ));

        secondsValue = parsedValue;                        /* parsedValue is seconds. */

        /* We have seconds, get the fractional seconds. */
        parsedValue = 0;
        for (; isdigit( ch = batom_cursor_next( pCursor )); )
        {
            int64_t val;

            if (batom_cursor_distance( &decimalCursor, pCursor ) >3) {break; }

            val = ch - '0';
            BDBG_MSG(( BIP_MSG_PRE_FMT "parsedValue: %" PRId64 "  (BIP_I64_MAX-val)/10 : %" PRId64 "" BIP_MSG_PRE_ARG, parsedValue, ( BIP_I64_MAX-val )/10 ));
            if (parsedValue > ( BIP_I64_MAX-( val*decimalFactor ))) /* Check for integer overflow. */
            {
                goto overflow;
            }
            parsedValue    = ( parsedValue ) + val*decimalFactor;
            decimalFactor /= 10;
        }

        msecondsValue = parsedValue;                       /* parsedValue is in milliseconds, add them to result. */
        BDBG_MSG(( BIP_MSG_PRE_FMT "msecondsValue: %" PRId64 "" BIP_MSG_PRE_ARG, msecondsValue ));
    }
    /* If we've come to a colon, we must have this format: "<hours>:<minutes>:<seconds>[.fraction>]". */
    else if (ch  == ':')
    {
        batom_cursor colonCursor;
        batom_cursor decimalCursor;

        batom_cursor_clone( &colonCursor, pCursor );

        BDBG_MSG(( BIP_MSG_PRE_FMT "Found ':', npt format is \"<hours>:<minutes>:<seconds>[.fraction>]\"" BIP_MSG_PRE_ARG ));

        hoursValue = parsedValue;

        /* We already have the hours, not get the minutes. */
        parsedValue = 0;
        for (; isdigit( ch = batom_cursor_next( pCursor )); )
        {
            int64_t val;

            if (batom_cursor_distance( &colonCursor, pCursor ) >2) {break; } /* Only allow 2 digits after colon. */

            val = ch - '0';
            if (parsedValue > ( BIP_I64_MAX-val )/10)      /* Check for integer overflow. */
            {
                goto overflow;
            }
            parsedValue = ( parsedValue*10 ) + val;
            if (parsedValue > 59)
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT  "Error: NPT-time minutes exceeds max of 59" BIP_MSG_PRE_ARG ));
                goto parseError;
            }
        }

        /* There should be a colon after the minutes. */
        if (ch != ':') {goto parseError; }

        batom_cursor_clone( &colonCursor, pCursor );

        minutesValue = parsedValue;

        /* We have hours and minutes, now get the seconds. */
        parsedValue = 0;
        for (; isdigit( ch = batom_cursor_next( pCursor )); )
        {
            int64_t val;

            if (batom_cursor_distance( &colonCursor, pCursor ) >2) {break; } /* Only allow 2 digits after colon. */

            val = ch - '0';
            if (parsedValue > ( BIP_I64_MAX-val )/10)      /* Check for integer overflow. */
            {
                goto overflow;
            }
            parsedValue = ( parsedValue*10 ) + val;
            if (parsedValue > 59)
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT  "Error: NPT-time seconds exceeds max of 59" BIP_MSG_PRE_ARG ));
                goto parseError;
            }
        }

        secondsValue = parsedValue;

        /* There might be a decimal point after the seconds, if so, then get the fractional seconds. */
        if (ch  == '.')
        {
            int32_t decimalFactor = 100;

            batom_cursor_clone( &decimalCursor, pCursor );

            parsedValue = 0;
            for (; isdigit( ch = batom_cursor_next( pCursor )); )
            {
                int64_t val;

                if (batom_cursor_distance( &decimalCursor, pCursor ) >3) {break; } /* Only allow 3 digits after decimal point. */

                val = ch - '0';
                if (parsedValue > ( BIP_I64_MAX-val )/10)  /* Check for integer overflow. */
                {
                    goto overflow;
                }
                parsedValue    = ( parsedValue ) + val*decimalFactor;
                decimalFactor /= 10;
            }

            msecondsValue = parsedValue;
        }
    }
    /* If we've come to some other non-digit, we must have this format: "<seconds>[.<fraction>]",
     * without the optional decimal point.  And we're at the end of the NPT time. */
    else
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "Found end of npt-time string, npt format is \"<seconds>\"" BIP_MSG_PRE_ARG ));

        secondsValue = parsedValue;
    }

    batom_cursor_skip( &startCursor, batom_cursor_distance( &startCursor, pCursor ) -1 );
    batom_cursor_clone( pCursor, &startCursor );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Summing up: hoursValue: %" PRId64 " minutesValue: %" PRId64 " secondsValue: %" PRId64 " msecondsValue: %" PRId64 ""
               BIP_MSG_PRE_ARG, hoursValue, minutesValue, secondsValue, msecondsValue ));

    /* Now combine the different units into a total number of milliseconds. */
    if (hoursValue > ( BIP_I64_MAX-minutesValue )/60) {goto overflow; }
    minutesValue = ( hoursValue * 60 ) + minutesValue;

    if (minutesValue > ( BIP_I64_MAX-secondsValue )/60) {goto overflow; }
    secondsValue = ( minutesValue * 60 ) + secondsValue;

    if (secondsValue > ( BIP_I64_MAX-msecondsValue )/1000) {goto overflow; }
    msecondsValue = ( secondsValue * 1000 ) + msecondsValue;

    {
        unsigned fraction;
        fraction = msecondsValue%1000;
        BDBG_MSG(( BIP_MSG_PRE_FMT "Final answer. NPT time in milliseconds: %" PRId64 ".%03u"
                   BIP_MSG_PRE_ARG, msecondsValue/1000, fraction ));
    }

    *pNptTimeInMs = msecondsValue;
    return( rc );

overflow:
    BDBG_ERR(( BIP_MSG_PRE_FMT  "Error: 64-bit overflow while parsing NPT-time" BIP_MSG_PRE_ARG ));

parseError:
    return( BIP_ERR_HTTP_MESSAGE_INVALID );
}                                                          /* BIP_HttpRequest_ParseNptTime */

/*****************************************************************************
 *  API to get a start time and duration for a DLNA TimeSeekRange header.
 *****************************************************************************/
BIP_Status BIP_HttpRequest_ParseTimeSeekRangeDlnaOrgHeader(
   BIP_HttpRequestHandle hRequest,                        /*!< [in]  Handle of the HttpRequest to be accessed. */
   int64_t               contentDurationInMs,             /*!< [in]  Total content duration if known.  Pass -1 if unknown. */
   int64_t              *pStartTimeInMs,                  /*!< [out] Caller's variable to receive npt-start-time as int64_t milliseconds */
   int64_t              *pEndTimeInMs                     /*!< [out] Caller's variable to receive npt-end-time as int64_t milliseconds */
                                                          /*!<       Set to -1 if duration cannot be determined. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    int64_t startTimeInMs;
    int64_t endTimeInMs;

    const char *pPlaySpeedName  = "PlaySpeed.dlna.org";
    const char *pPlaySpeedValue = NULL;

    const char *pTimeSeekRangeName  = "TimeSeekRange.dlna.org";
    const char *pTimeSeekRangeValue = NULL;

    bool scanModeIsForward = true;
    bool valueParseError;

    batom_vec    vec;
    batom_cursor cursor;

    /* Look for the "PlaySpeed.dlna.org" header so we can find the scan mode. */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Getting %s header... " BIP_MSG_PRE_ARG, (void *)hRequest, pPlaySpeedName ));
    rc = BIP_HttpHeaderList_GetNextHeader( hRequest->hHeaderList, NULL, pPlaySpeedName, NULL, &pPlaySpeedValue );
    BIP_CHECK_LOGERR(( rc==BIP_SUCCESS || rc==BIP_INF_NOT_AVAILABLE ), ( "BIP_HttpHeaderList_GetNextHeader() failed" ), rc, rc );

    if (rc==BIP_SUCCESS)                                   /* If we found the PlaySpeed header... */
    {
        const char *speedEquals = "speed=";
        const char *pSpeed;

        batom_vec_init( &vec, pPlaySpeedValue, strlen( pPlaySpeedValue ));
        batom_cursor_from_vec( &cursor, &vec, 1 );

        valueParseError = false;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Got %s header: \"%s\" " BIP_MSG_PRE_ARG, (void *)hRequest, pPlaySpeedName, pPlaySpeedValue ));

        for (pSpeed = speedEquals; *pSpeed!='\0'; pSpeed++) {
            if (batom_cursor_byte( &cursor ) != *pSpeed)
            {
                valueParseError = true;
                break;
            }
        }

        if (valueParseError)
        {
            if (batom_cursor_size( &cursor ) == 0)
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT "%s header has no value." BIP_MSG_PRE_ARG, pPlaySpeedName ));
            }
            else
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT "Error parsing PlaySpeed.dlna.org header: \"%s\"" BIP_MSG_PRE_ARG, pPlaySpeedValue ));
                BDBG_WRN(( BIP_MSG_PRE_FMT "Parse failed here:                       %*s^" BIP_MSG_PRE_ARG, (int)batom_cursor_pos( &cursor ), "" ));
            }
        }
        else                                               /* Playspeed.dlna.org header parsed successfully. */
        {
            int ch = batom_cursor_next( &cursor );
            if (ch == '-')
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Found minus sign on PlaySpeed, scan mode is backwards." BIP_MSG_PRE_ARG, (void *)hRequest ));
                scanModeIsForward = false;
            }
            else if (isdigit( ch ))
            {
                BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: No minus sign on PlaySpeed, scan mode is forward." BIP_MSG_PRE_ARG, (void *)hRequest ));
            }
        }
    }

    /* Look for the "TimeSeekRange.dlna.org" . */
    BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Getting %s header... " BIP_MSG_PRE_ARG, (void *)hRequest, pTimeSeekRangeName ));
    rc = BIP_HttpHeaderList_GetNextHeader( hRequest->hHeaderList, NULL, pTimeSeekRangeName, NULL, &pTimeSeekRangeValue );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS || rc==BIP_INF_NOT_AVAILABLE ), ( "BIP_HttpHeaderList_GetNextHeader() failed" ), error,  rc, rc );

    if (rc==BIP_INF_NOT_AVAILABLE) {goto error; }          /* No TimeSeekRange header return INF status. */

    batom_vec_init( &vec, pTimeSeekRangeValue, strlen( pTimeSeekRangeValue ));
    batom_cursor_from_vec( &cursor, &vec, 1 );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hRequest %p: Got %s header: \"%s\" " BIP_MSG_PRE_ARG, (void *)hRequest, pTimeSeekRangeName, pTimeSeekRangeValue ));

    BIP_CHECK_GOTO(( batom_cursor_size( &cursor ) != 0 ), ( "%s header has no value.", pTimeSeekRangeName ), error,  BIP_ERR_HTTP_MESSAGE_INVALID, rc );

    /* TimeSeekRange header should start with "npt=". */
    {
        const char *nptEquals = "npt=";
        const char *pNpt;

        for (pNpt = nptEquals; *pNpt!='\0'; pNpt++) {
            if (batom_cursor_byte( &cursor ) != *pNpt)
            {
                goto parseError;
            }
        }
    }

    /* We've moved past the "npt=", so we shoud be at the  npt-start-time. */
    rc = BIP_HttpRequest_ParseNptTime( &cursor, &startTimeInMs );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpRequest_ParseNptTime() failed" ), parseError,  rc, rc );

    /* We're past the npt-start-time, now there should be a hyphen ("-").*/
    if (batom_cursor_next( &cursor ) != '-')
    {
        BDBG_ERR(( BIP_MSG_PRE_FMT "Missing hyphen after npt-start-time." BIP_MSG_PRE_ARG ));
        goto parseError;
    }

    /* If anything left in the cursor, it must be the npt-end-time. */
    if (batom_cursor_size( &cursor ) > 0)
    {
        rc = BIP_HttpRequest_ParseNptTime( &cursor, &endTimeInMs );
        BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpRequest_ParseNptTime() failed" ), parseError,  rc, rc );
    }
    else                                                   /* There's nothing after the "-".  npt-end-time not specified. */
    {
        endTimeInMs = -1;                                  /* End time is end or start of content (depending on scan mode). */
    }

    /* We're done parsing the header, now convert the npt-start-time, optional npt-end-time, the
     * caller's specified content duration and the scan mode (forward or backward) into a
     * startTime and endTime that we can give back to the caller. */

    if (scanModeIsForward)
    {
        /* Make sure the npt-start-time is before the caller-specified content duration. */
        if (( contentDurationInMs >= 0 ) && ( startTimeInMs > contentDurationInMs ))
        {
            BDBG_ERR(( BIP_MSG_PRE_FMT "Forward Scan and npt-start-time:%" PRId64 " exceeds contentDurationInMs:%" PRId64 ""
                       BIP_MSG_PRE_ARG, startTimeInMs, contentDurationInMs ));
            goto parseError;
        }

        if (endTimeInMs >= 0)                           /* Header included end time. */
        {
            if ( endTimeInMs < startTimeInMs)
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "Forward Scan and npt-end-time:%" PRId64 " is before npt-start-time:%" PRId64 ""
                           BIP_MSG_PRE_ARG, endTimeInMs, startTimeInMs ));
                goto parseError;
            }

            if (( contentDurationInMs >= 0 ) && ( endTimeInMs > contentDurationInMs ))
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT "npt-end-time:%" PRId64 " exceeds contentDuration:%" PRId64 ", compensating..."
                           BIP_MSG_PRE_ARG, endTimeInMs, contentDurationInMs ));
                *pEndTimeInMs = contentDurationInMs;       /* DLNA Guidelines Mar 2014: 7.5.4.3.2.24.4 */
            }
            else
            {
                *pEndTimeInMs = endTimeInMs;
            }

            *pStartTimeInMs = startTimeInMs;
        }
        else                                               /* Header omitted end time, end is at content duration. */
        {
            if (contentDurationInMs >= 0)                   /* Caller passed content duration. */
            {
                *pEndTimeInMs = contentDurationInMs;
            }
            else                                           /* Caller didn't pass content duration. */
            {                                              /* Can't determine duration of range. */
                *pEndTimeInMs = -1;
            }

            *pStartTimeInMs = startTimeInMs;
        }
    }
    else                                                   /* Scan mode is backwards. */
    {
        /* Make sure the npt-end-time is before the caller-specified content duration. */
        if (endTimeInMs >= 0)                    /* Header included end time. */
        {
            if (( contentDurationInMs >= 0 ) && ( endTimeInMs > contentDurationInMs ))
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "Backward Scan and npt-end-time:%" PRId64 " exceeds contentDurationInMs:%" PRId64 ""
                           BIP_MSG_PRE_ARG, endTimeInMs, contentDurationInMs ));
                goto parseError;
            }

            if (startTimeInMs < endTimeInMs)               /* DLNA Guidelines Mar 2014: 7.5.4.3.2.24.12 */
            {
                BDBG_ERR(( BIP_MSG_PRE_FMT "Backward Scan and npt-start-time:%" PRId64 " is before npt-end-time:%" PRId64 ""
                           BIP_MSG_PRE_ARG, startTimeInMs, endTimeInMs ));
                goto parseError;
            }

            if (( contentDurationInMs >= 0 ) && ( startTimeInMs > contentDurationInMs ))
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT "npt-start-time:%" PRId64 " exceeds contentDuration:%" PRId64 ", compensating..."
                           BIP_MSG_PRE_ARG, startTimeInMs, contentDurationInMs ));
                *pStartTimeInMs = contentDurationInMs;     /* DLNA Guidelines Mar 2014: 7.5.4.3.2.24.4 */
            }
            else
            {
                *pStartTimeInMs = startTimeInMs;
            }

            *pEndTimeInMs = endTimeInMs;
        }
        else                                               /* Header omitted end time, end is at time:0. */
        {
            if (( contentDurationInMs >= 0 ) && ( startTimeInMs > contentDurationInMs ))
            {
                BDBG_WRN(( BIP_MSG_PRE_FMT "npt-start-time:%" PRId64 " exceeds contentDuration:%" PRId64 ", compensating..."
                           BIP_MSG_PRE_ARG, startTimeInMs, contentDurationInMs ));
                *pStartTimeInMs = contentDurationInMs;     /* DLNA Guidelines Mar 2014: 7.5.4.3.2.24.4 */
            }
            else
            {
                *pStartTimeInMs = startTimeInMs;
            }

            *pEndTimeInMs = 0;
        }
    }

error:
    return( rc );

parseError:
    BDBG_ERR(( BIP_MSG_PRE_FMT "Error parsing TimeSeekRange.dlna.org: \"%s\"" BIP_MSG_PRE_ARG, pTimeSeekRangeValue ));
    BDBG_ERR(( BIP_MSG_PRE_FMT "Parse failed here:                    %*s^" BIP_MSG_PRE_ARG, (int)batom_cursor_pos( &cursor ), "" ));

    BIP_ERR_TRACE( BIP_ERR_HTTP_MESSAGE_INVALID );

    return( BIP_ERR_HTTP_MESSAGE_INVALID );
}                                                          /* BIP_HttpRequest_ParseTimeSeekRangeDlnaOrgHeader */
