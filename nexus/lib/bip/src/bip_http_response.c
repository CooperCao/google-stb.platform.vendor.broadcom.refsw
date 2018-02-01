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
#include <limits.h>
#include <inttypes.h>                                      /* For PRIu64 printf format. */

#include "bip_priv.h"
#include "blst_queue.h"
#include "bioatom.h"

BDBG_MODULE( bip_http_response );
BDBG_OBJECT_ID( BIP_HttpResponse );

BIP_SETTINGS_ID( BIP_HttpResponseCreateSettings );
BIP_SETTINGS_ID( BIP_HttpResponseClearSettings );
BIP_SETTINGS_ID( BIP_HttpResponsePrintSettings );

/*****************************************************************************
*  Deserializer states:
 *****************************************************************************/
typedef enum BIP_HttpResponseDeserializeState {
    deserializeState_eIdle = 0,
    deserializeState_eHttpVersion,
    deserializeState_eStatusCode,
    deserializeState_eReasonPhrase,
    deserializeState_eResponseLineTerm,
    deserializeState_eHeaders,
    deserializeState_eDone
} BIP_HttpResponseDeserializeState;

static const char *toStr_deserializeState(
    int value
    )
{
    const namevalue_t myStrings[] = {
        {"Idle",             deserializeState_eIdle            },
        {"HttpVersion",      deserializeState_eHttpVersion     },
        {"StatusCode",       deserializeState_eStatusCode      },
        {"ReasonPhrase",     deserializeState_eReasonPhrase    },
        {"ResponseLineTerm", deserializeState_eResponseLineTerm},
        {"Headers",          deserializeState_eHeaders         },
        {"Done",             deserializeState_eDone            },
        {NULL,               0                                 }
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
typedef enum BIP_HttpResponseSerializeState {
    serializeState_eIdle = 0,
    serializeState_eVersionPrefix,
    serializeState_eVersionMajor,
    serializeState_eVersionDot,
    serializeState_eVersionMinor,
    serializeState_eVersionSpace,
    serializeState_eStatusCode,
    serializeState_eStatusCodeSpace,
    serializeState_eReasonPhrase,
    serializeState_eResponseLineTerm,
    serializeState_eHeaders,
    serializeState_eDone
} BIP_HttpResponseSerializeState;

static const char *toStr_serializeState(
    int value
    )
{
    const namevalue_t myStrings[] = {
        {"Idle",             serializeState_eIdle            },
        {"VersionPrefix",    serializeState_eVersionPrefix   },
        {"VersionMajor",     serializeState_eVersionMajor    },
        {"VersionDot",       serializeState_eVersionDot      },
        {"VersionMinor",     serializeState_eVersionMinor    },
        {"VersionSpace",     serializeState_eVersionSpace    },
        {"StatusCode",       serializeState_eStatusCode      },
        {"StatusCodeSpace",  serializeState_eStatusCodeSpace },
        {"ReasonPhrase",     serializeState_eReasonPhrase    },
        {"ResponseLineTerm", serializeState_eResponseLineTerm},
        {"Headers",          serializeState_eHeaders         },
        {"Done",             serializeState_eDone            },
        {NULL,               0                               }
    };

    return( lookup_name( myStrings, ( value )));
}                                                          /* toStr_serializeState */

/*****************************************************************************
 *  Object structures:
 *****************************************************************************/
typedef struct BIP_HttpResponse
{
    BDBG_OBJECT( BIP_HttpResponse )

    void    *ownerContext;                                 /* Any pointer passed to Create(), must also be passed to Destroy(). */
    BIP_HttpResponseCreateSettings   createSettings;

    BIP_StringHandle                 hStatus;
    BIP_StringHandle                 hReasonPhrase;
    BIP_HttpResponseHttpVersion      httpVersion;
    BIP_HttpHeaderListHandle         hHeaderList;
    void                            *pUserData;

    BIP_HttpResponseDeserializeState deserializeState;
    BIP_HttpResponseSerializeState   serializeState;

    size_t                           byteIndex;            /* Byte index within current object being serialized/deserialized. */
    B_MutexHandle                    hMutex;
    batom_factory_t                  factory;
} BIP_HttpResponse;

/*****************************************************************************
 *  Sets the HTTP status-code (and a corresponding reason-phrase) into an
 *  Response object, overwriting any previous status-code and
 *  reason-phrase that may have been previously set.
 *
 *  If the HTTP status-code is unrecognized by BIP, the status-code will
 *  be set as requested, but the reason-phrase will be left empty.  To
 *  specify a custom reason-phrase use BIP_HttpResponse_SetCustomStatus()
 *****************************************************************************/
BIP_Status BIP_HttpResponse_SetStatus(
    BIP_HttpResponseHandle hResponse,
    BIP_HttpResponseStatus responseStatus                  /*!< [in] response staus.This API will internally add status phrase.*/
    )
{
    BIP_Status  rc = BIP_SUCCESS;
    const char *pReasonPhrase;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    B_Mutex_Lock( hResponse->hMutex );

    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hResponse->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    rc = BIP_String_StrcpyPrintf( hResponse->hStatus, "%3u", responseStatus ); /* Set the 3-digit status code as a string. */
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_StrcpyPrintf() failed" ), error, rc, rc );

    pReasonPhrase = BIP_ToStr_BIP_HttpResponseStatus( responseStatus );               /* Set the matching reason phrase if we have one. */
    rc            = BIP_String_StrcpyChar( hResponse->hReasonPhrase, pReasonPhrase ); /* Handles pReasonPhrase == NULL. */
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_StrcpyChar() failed" ), error, rc, rc );

error:
    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_SetStatus */

/*****************************************************************************
 *  Sets a "custom" HTTP status-code and reason-phrase into an
 *  HttpResponse object, overwriting any previous status-code and
 *  reason-phrase that may have been previously set.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_SetCustomStatus(
    BIP_HttpResponseHandle hResponse,
    const char            *pStatusCodeString,              /*!< [in] Response status code .*/
    const char            *pReasonPhraseString             /*!< [in] Response status phrase.*/
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    B_Mutex_Lock( hResponse->hMutex );

    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hResponse->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    rc = BIP_String_StrcpyChar( hResponse->hStatus, pStatusCodeString ); /* Set the 3-digit status code as a string. */
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_StrcpyChar() failed" ), error, rc, rc );

    rc = BIP_String_StrcpyChar( hResponse->hReasonPhrase, pReasonPhraseString ); /* Set the reason-phrase3-digit status code as a string. */
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_StrcpyChar() failed" ), error, rc, rc );

error:
    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_SetCustomStatus */

/*****************************************************************************
 *  Retrieves the HTTP status-code and reason-phrase from the specified
 *  BIP_HttpResponse object.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_GetStatus(
    BIP_HttpResponseHandle  hResponse,                     /*!< [in]  Handle of BIP_HttpResponse to be accessd. */
    BIP_HttpResponseStatus *pResponseStatus,               /*!< [out] Address of caller's variable to receive the BIP_HttpResponseStatus enum value. */
                                                           /*!<       If method not recognized, it will be set to BIP_HttpResponseStatus_eInvalid. */
    const char            **ppResponseStatusString,        /*!< [out] Optional address of caller's pointer to be set to the null-terminated status-code. */
    const char            **ppReasonPhraseString           /*!< [out] Optional address of caller's pointer to be set to the null-terminated reason-phrase. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    B_Mutex_Lock( hResponse->hMutex );
    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hResponse->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    if (pResponseStatus)
    {
        *pResponseStatus = BIP_FromStr_BIP_HttpResponseStatus( BIP_String_GetString( hResponse->hStatus ));
    }

    if (ppResponseStatusString)
    {
        *ppResponseStatusString = BIP_String_GetString( hResponse->hStatus );
    }

    if (ppReasonPhraseString)
    {
        *ppReasonPhraseString = BIP_String_GetString( hResponse->hReasonPhrase );
    }

error:
    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_GetStatus */

/*****************************************************************************
 *  Sets the HTTP version into an HttpResponse object, overwriting any
 *  previous version that may have been previously set.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_SetHttpVersion(
    BIP_HttpResponseHandle       hResponse,                /*!< [in] Handle of BIP_HttpResponse to updated. */
    BIP_HttpResponseHttpVersion *pHttpVersion              /*!< [in] Pointer to caller's BIP_HttpResponseHttpVersion structure. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hResponse );
    BDBG_ASSERT( pHttpVersion );

    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    B_Mutex_Lock( hResponse->hMutex );
    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hResponse->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

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

    hResponse->httpVersion = *pHttpVersion;

error:
    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_SetHttpVersion */

/*****************************************************************************
 *  Retrieves the HTTP version from an HttpResponse object.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_GetHttpVersion(
    BIP_HttpResponseHandle       hResponse,                /*!< [in] Handle of BIP_HttpResponse to be accessd. */
    BIP_HttpResponseHttpVersion *pHttpVersion              /*!< [out] Pointer to caller's BIP_HttpResponseHttpVersion structure. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );
    BDBG_ASSERT( pHttpVersion );

    B_Mutex_Lock( hResponse->hMutex );
    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hResponse->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    BIP_CHECK_GOTO(( pHttpVersion ),
        ( "Invalid NULL pointer" ),
        error, BIP_ERR_INVALID_PARAMETER, rc );

    *pHttpVersion = hResponse->httpVersion;

error:
    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_GetHttpVersion */

/*****************************************************************************
 *  Populate the "status-line" fields of the specified BIP_HttpResponse.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_SetStatusLine(
    BIP_HttpResponseHandle       hResponse,                /*!< [in] Handle of BIP_HttpResponse to be updated. */
    BIP_HttpResponseHttpVersion *pHttpVersion,             /*!< [in] Optional pointer to a BIP_HttpReqestHttpVersion structure. */
                                                           /*!<      Overrides value specified in BIP_HttpResponseCreateSettings.httpVersion */
                                                           /*!<      if specified here. */
    const char                  *pStatusCodeString,        /*!< [in] Optional response status code.  If NULL, then value */
                                                           /*!<      from statusEnum will be used. */
    BIP_HttpResponseStatus       statusEnum,               /*!< [in] Optional response status.  Only used if pStatusCodeString is NULL.*/
    const char                  *pReasonPhraseString       /*!< [in] Optional response reason-phrase.  If NULL, then a matching */
                                                           /*!<      reason-phrase will be automatically selected, if possible. */
    )
{
    BIP_Status  rc = BIP_SUCCESS;
    const char *pReasonPhraseFromStatusEnum = NULL;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    B_Mutex_Lock( hResponse->hMutex );
    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hResponse->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    /* Start by validating all fields, because if anything is bad, we
     * don't want to change anything. */

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

    /* If StatusCode string is passed, use it. But if not, then we need
       to use the statusEnum and convert it to a String. */
    if (!pStatusCodeString)
    {
        pReasonPhraseFromStatusEnum = BIP_ToStr_BIP_HttpResponseStatus( statusEnum );

        BIP_CHECK_GOTO(( pReasonPhraseFromStatusEnum!=NULL ),
            ( "Invalid value for BIP_HttpResponseStatus: %u.", statusEnum ),
            error, BIP_ERR_INVALID_PARAMETER, rc );
    }

    /* Everything has been validated, now make the changes. */

    hResponse->httpVersion = *pHttpVersion;

    /* If they passed a pStatusCodeString, go ahead and use it. */
    if (pStatusCodeString)
    {
        rc = BIP_String_StrcpyChar( hResponse->hStatus, pStatusCodeString );
        BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_StrcpyChar() failed" ), error, rc, rc );
    }
    else                                                   /* The used an statusEnum that should convert to a 3-digit string. */
    {
        rc = BIP_String_StrcpyPrintf( hResponse->hStatus, "%u", statusEnum );
        BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_StrcpyPrintf() failed" ), error, rc, rc );
    }

    /* Now try to come up with a Reason Phrase. */
    if (pReasonPhraseString)
    {
        rc = BIP_String_StrcpyChar( hResponse->hReasonPhrase, pReasonPhraseString );
        BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_StrcpyChar() failed" ), error, rc, rc );
    }
    else if (pReasonPhraseFromStatusEnum)
    {
        rc = BIP_String_StrcpyChar( hResponse->hReasonPhrase, pReasonPhraseFromStatusEnum );
        BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_StrcpyChar() failed" ), error, rc, rc );
    }
    else if (pStatusCodeString)
    {
        statusEnum = BIP_FromStr_BIP_HttpResponseStatus( pStatusCodeString );
        pReasonPhraseFromStatusEnum = BIP_ToStr_BIP_HttpResponseStatus( statusEnum );

        rc = BIP_String_StrcpyChar( hResponse->hReasonPhrase, pReasonPhraseFromStatusEnum );
        BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_StrcpyChar() failed" ), error, rc, rc );
    }

error:
    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_SetStatusLine */

/*****************************************************************************
 *  Retrieve the values of the "status-line" fields of the specified
 *  BIP_HttpResponse.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_GetStatusLine(
    BIP_HttpResponseHandle       hResponse,                /*!< [in]  Handle of BIP_HttpResponse to be accessed. */
    BIP_HttpResponseHttpVersion *pHttpVersion,             /*!< [out] Pointer to caller's BIP_HttpResponseHttpVersion structure. */
    const char                 **ppStatusCodeString,       /*!< [out] Optional address of caller's pointer to be set */
                                                           /*!<       to the null-terminated status-code. */
    BIP_HttpResponseStatus      *pStatusEnum,              /*!< [out] Address of caller's variable to receive the */
                                                           /*!<       BIP_HttpResponseStatus enum value. */
                                                           /*!<       Unrecognized status-code values will be indicated */
                                                           /*!<       as BIP_HttpResponseStatus_eInvalid. */
    const char                 **ppReasonPhraseString      /*!< [out] Optional address of caller's pointer to be set */
                                                           /*!<       to the null-terminated reason-phrase. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    B_Mutex_Lock( hResponse->hMutex );
    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hResponse->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    /* Give them the HttpVersion if they gave us somewhere to put it. */
    if (pHttpVersion)
    {
        *pHttpVersion = hResponse->httpVersion;
    }

    /* Give them the StatusString poiner if they gave us somewhere to put it. */
    if (ppStatusCodeString)
    {
        *ppStatusCodeString = BIP_String_GetString( hResponse->hStatus );
    }

    /* Give them the StatusEnum if they gave us somewhere to put it. */
    if (pStatusEnum)
    {
        *pStatusEnum = BIP_FromStr_BIP_HttpResponseStatus( BIP_String_GetString( hResponse->hStatus ));
    }

    /* Give them the Reason Phrase String pointer if they gave us somewhere to put it. */
    if (ppReasonPhraseString)
    {
        *ppReasonPhraseString = BIP_String_GetString( hResponse->hReasonPhrase );
    }

error:
    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_GetStatusLine */

/*****************************************************************************
 *  Attach user-defined data to an HttpResponse.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_SetUserData(
    BIP_HttpResponseHandle hResponse,                      /*!< [in] Handle of BIP_HttpResponse to be printed. */
    void                  *pUserData                       /*!< [in]: Pointer to the user specific data. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    /* We don't do anything with the user-data, so don't worry about states. */

    B_Mutex_Lock( hResponse->hMutex );

    hResponse->pUserData = pUserData;

    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_SetUserData */

/*****************************************************************************
 *  Retrieve user-defined data from an HttpResponse.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_GetUserData(
    BIP_HttpResponseHandle hResponse,
    void                 **ppUserData                      /*!< [out] Address of caller's variable where the user-data pointer will be placed. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    /* We don't do anything with the user-data, so don't worry about states. */

    B_Mutex_Lock( hResponse->hMutex );

    *ppUserData = hResponse->pUserData;

    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_GetUserData */

/*****************************************************************************
 *  Get the first or next HttpHeader following a given Header in the
 *  specified HttpResponse's list of headers.  Optionally, a header
 *  name can be specified to return the next header with the specified name.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_GetNextHeader(
    BIP_HttpResponseHandle hResponse,                      /*!< [in] Handle of the BIP_HttpResponse to be accessed. */
    BIP_HttpHeaderHandle   hCurrentHeader,                 /*!< [in] The Header from which to start.  Pass NULL to start from the beginning of the list. */
    const char            *pName,                          /*!< [in] If non-NULL, then return the next Header with a matching name.. */
    BIP_HttpHeaderHandle  *phNextHeader,                   /*!< [out] Address of variable to receive the handle of the next header with the specified name. */
    const char           **ppNextHeaderValue               /*!< [out] Optional address of pointer that will be point to the header's value.
                                                           Pass NULL if not interested in the header's value. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    BDBG_ASSERT( phNextHeader );

    B_Mutex_Lock( hResponse->hMutex );
    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hResponse->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    rc = BIP_HttpHeaderList_GetNextHeader( hResponse->hHeaderList, hCurrentHeader, pName, phNextHeader, ppNextHeaderValue );

error:
    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_GetNextHeader */

/*****************************************************************************
 *  Get the name & value of a Response's HttpHeader.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_GetHeaderNameValue(
    BIP_HttpResponseHandle hResponse,                      /*!< [in]  Handle of the BIP_HttpResponse to be accessed. */
    BIP_HttpHeaderHandle   hHeader,                        /*!< [in]  The header of interest. */
    const char           **ppName,                         /*!< [out] The address of a pointer that will be updated to point the Header's name as a null-terminated string.
                                                     This pointer will become invalid when it's Header is destroyed. */
    const char           **ppValue                         /*!< [out] The address of a pointer that will be updated to point the Header's value as a null-terminated string.
                                                     This pointer will become invalid when it's Header is destroyed. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    B_Mutex_Lock( hResponse->hMutex );
    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hResponse->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    if (ppName)
    {
        rc = BIP_HttpHeaderList_GetHeaderName( hHeader, ppName );
    }
    if (ppValue)
    {
        rc = BIP_HttpHeaderList_GetHeaderValue( hHeader, ppValue );
    }

error:
    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_GetHeaderNameValue */

/*****************************************************************************
 *  Remove (and destroy) an HttpHeader from an HttpResponse.
 *****************************************************************************/
void BIP_HttpResponse_RemoveHeader(
    BIP_HttpResponseHandle hResponse,                      /*!< [in] Handle of BIP_HttpResponse to modified. */
    BIP_HttpHeaderHandle   hHeader                         /*!< [in] Handle of the Header to be removed/destroyed. */
    )
{
    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    BDBG_ASSERT( hHeader );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHeader %p: Entry..." BIP_MSG_PRE_ARG, (void *)hResponse ));

    BIP_HttpHeaderList_DestroyHeader( hHeader );

    return;
}

/*****************************************************************************
 *  Move an HttpHeader and place it before a specified HttpHeader.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_MoveHeader(
    BIP_HttpResponseHandle hResponse,                      /*!< [in] Handle of the BIP_HttpResponse to be updated. */
    BIP_HttpHeaderHandle   hHeader,                        /*!< [in] The header to be moved. */
    BIP_HttpHeaderHandle   hPutBeforeThisHeader            /*!< [in] The Header that hHeader will moved in front of. */
                                                           /*!<      If NULL, then hHeader will be placed at the end of the HttpResponse's header list. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    BDBG_ASSERT( hHeader );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHeader %p: Entry..." BIP_MSG_PRE_ARG, (void *)hResponse ));

    rc = BIP_HttpHeaderList_MoveHeader( hHeader, hPutBeforeThisHeader );

    return( rc );
}

/*****************************************************************************
 *  Set the value of a HttpResponse's HttpHeader.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_SetHeaderValue(
    BIP_HttpResponseHandle hResponse,                      /*!< [in] Handle of the BIP_HttpResponse to be updated. */
    BIP_HttpHeaderHandle   hHeader,                        /*!< [in] The header that will have its value changed. */
    const char            *pValue                          /*!< [in] A pointer to a null-terminated string containing the Header's new value. */
                                                           /*!<      Passing NULL will result in a header without a value.  */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    B_Mutex_Lock( hResponse->hMutex );
    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hResponse->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    rc = BIP_HttpHeaderList_SetHeaderValue( hHeader, pValue );

error:
    B_Mutex_Unlock( hResponse->hMutex );
    return( rc );
}                                                          /* BIP_HttpResponse_SetHeaderValue */

/*****************************************************************************
 *  Add a new HttpHeader to an HttpResponse.
 *****************************************************************************/
BIP_HttpHeaderHandle BIP_HttpResponse_AddHeader(
    BIP_HttpResponseHandle hResponse,                      /*!< [in] Handle of BIP_HttpResponse to modified. */
    const char            *pName,                          /*!< [in] Pointer to a null-terminated string with the header's name. */
    const char            *pValue,                         /*!< [in] Optional pointer to a null-terminated string will be copied to the header's value. */
                                                           /*!<      If NULL, then the header will be created without a value. */
    BIP_HttpHeaderHandle   hAddBeforeThisHeader            /*!< [in] If a BIP_HeaderHandle is passed, and it exists in the specified hHttpResponse, */
                                                           /*!<      the newly created header will be positioned immediately in front of it. */
                                                           /*!<      If NULL, the new header will be placed at the end of the HttpResponse's headers. */
    )
{
    BIP_Status           rc      = BIP_SUCCESS;
    BIP_HttpHeaderHandle hHeader = NULL;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    BDBG_ASSERT( pName );
    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    B_Mutex_Lock( hResponse->hMutex );
    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hResponse->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    hHeader = BIP_HttpHeaderList_CreateHeader( hResponse->hHeaderList, pName, pValue, hAddBeforeThisHeader );

error:
    B_Mutex_Unlock( hResponse->hMutex );
    if (rc) BDBG_ERR(( BIP_MSG_PRE_FMT "Response state error 0x%x" BIP_MSG_PRE_ARG,rc ));

    return( hHeader );
}                                                          /* BIP_HttpResponse_AddHeader */

/*****************************************************************************
 *  Clear an HttpResponse object.
 *****************************************************************************/
static void BIP_HttpResponse_Clear_locked(
    BIP_HttpResponseHandle         hResponse,              /*!< [in] Handle of BIP_HttpResponse to be cleared. */
    BIP_HttpResponseClearSettings *pSettings               /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hResponse %p: Entry..." BIP_MSG_PRE_ARG, (void *)hResponse ));

    BIP_SETTINGS_ASSERT( pSettings, BIP_HttpResponseClearSettings );

    /* Don't need to check for deserialize or serialize busy because
     * we're going to reset their states back to idle. */

    hResponse->httpVersion = hResponse->createSettings.httpVersion;
    rc = BIP_String_StrcpyPrintf( hResponse->hStatus, "%03u", BIP_HttpResponseStatus_e200_OK );
    BIP_CHECK_LOGERR(( rc==BIP_SUCCESS ), ( "BIP_String_StrcpyPrintf() failed" ), rc, rc );

    rc = BIP_String_StrcpyChar( hResponse->hReasonPhrase, BIP_ToStr_BIP_HttpResponseStatus( BIP_HttpResponseStatus_e200_OK ));
    BIP_CHECK_LOGERR(( rc==BIP_SUCCESS ), ( "BIP_String_StrcpyChar() failed" ), rc, rc );

    /* Destroy any BIP_HttpHeader objects that are in the Response's header list. */
    BIP_HttpHeaderList_Clear( hResponse->hHeaderList );

    hResponse->deserializeState = deserializeState_eIdle;
    hResponse->serializeState   = serializeState_eIdle;

    return;
}                                                          /* BIP_HttpResponse_Clear_locked */

/*****************************************************************************
 *  Clear an HttpResponse object.
 *****************************************************************************/
void BIP_HttpResponse_Clear(
    BIP_HttpResponseHandle         hResponse,              /*!< [in] Handle of BIP_HttpResponse to be cleared. */
    BIP_HttpResponseClearSettings *pSettings               /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    )
{
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hResponse %p: Entry..." BIP_MSG_PRE_ARG, (void *)hResponse ));

    B_Mutex_Lock( hResponse->hMutex );

    BIP_SETTINGS_ASSERT( pSettings, BIP_HttpResponseClearSettings );

    BIP_HttpResponse_Clear_locked( hResponse, pSettings );

    B_Mutex_Unlock( hResponse->hMutex );
    return;
}

/*****************************************************************************
 *  Destroy a HeaderList, including any Headers that it contains.
 *****************************************************************************/
void BIP_HttpResponse_Destroy(
    BIP_HttpResponseHandle hResponse,
    void                  *ownerContext                    /*!< Same ownerContext used to Create the HeaderList. */
    )
{
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hResponse %p: Entry..." BIP_MSG_PRE_ARG, (void *)hResponse ));

    if (ownerContext != hResponse->ownerContext)
    {
        BDBG_WRN(( BIP_MSG_PRE_FMT "hResponse %p: Mismatched ownerContext: got %p, expected %p.  Ignoring Destroy request!"
                   BIP_MSG_PRE_ARG, (void *)hResponse, (void *)ownerContext, (void *)hResponse->ownerContext ));
        return;
    }

    batom_factory_destroy( hResponse->factory );

    if (hResponse->hMutex)      {B_Mutex_Destroy( hResponse->hMutex ); }

    /* Destroy the HttpResponse's HttpHeaderList.*/
    if (hResponse->hHeaderList)
    {
        BIP_HttpHeaderList_Destroy(
            hResponse->hHeaderList,                        /* HeaderList handle. */
            &hResponse->createSettings );                  /* ownerContext, same as we used for Creating the HeaderList. */
    }
    if (hResponse->hStatus)         {BIP_String_Destroy( hResponse->hStatus ); }
    if (hResponse->hReasonPhrase)   {BIP_String_Destroy( hResponse->hReasonPhrase ); }

    BDBG_OBJECT_DESTROY( hResponse, BIP_HttpResponse );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hResponse %p: Freeing object memory" BIP_MSG_PRE_ARG, (void *)hResponse ));
    B_Os_Free( hResponse );

    return;
}                                                          /* BIP_HttpResponse_Destroy */

/*****************************************************************************
 *  Create a new, empty HttpResponse.
 *****************************************************************************/
BIP_HttpResponseHandle BIP_HttpResponse_Create(
    void                           *ownerContext,          /*!< [in] Any pointer, but the same pointer will be needed for destroying the Response. */
    BIP_HttpResponseCreateSettings *pCreateSettings        /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    )
{
    int                    rc;
    BIP_HttpResponseHandle hResponse = NULL;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    BIP_SETTINGS_ASSERT( pCreateSettings, BIP_HttpResponseCreateSettings );

    /* Allocate memory for the object. */
    hResponse = B_Os_Calloc( 1, sizeof( *hResponse ));
    BIP_CHECK_GOTO(( hResponse != NULL ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_OBJECT_SET( hResponse, BIP_HttpResponse );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hResponse %p: Allocated " BIP_MSG_PRE_ARG, (void *)hResponse ));

    hResponse->ownerContext = ownerContext;

    /* Set our Create settings... either from caller or use defaults. */
    {
        BIP_HttpResponseCreateSettings defaultSettings;

        if (NULL == pCreateSettings)
        {
            BIP_HttpResponse_GetDefaultCreateSettings( &defaultSettings );
            pCreateSettings = &defaultSettings;
        }
        hResponse->createSettings = *pCreateSettings;
    }

    hResponse->hStatus = BIP_String_Create();
    BIP_CHECK_GOTO(( hResponse->hStatus ),        ( "BIP_String_Create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hResponse->hReasonPhrase = BIP_String_Create();
    BIP_CHECK_GOTO(( hResponse->hReasonPhrase ),  ( "BIP_String_Create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hResponse->hHeaderList = BIP_HttpHeaderList_Create( &hResponse->createSettings );
    BIP_CHECK_GOTO(( hResponse->hHeaderList ),    ( "BIP_HttpHeaderList_Create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BIP_HttpResponse_Clear_locked( hResponse, NULL );

    /* coverity[missing_lock] */
    hResponse->deserializeState = deserializeState_eIdle;
    hResponse->serializeState   = serializeState_eIdle;

    hResponse->hMutex = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( hResponse->hMutex ),         ( "B_Mutex_Create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hResponse->factory = batom_factory_create( bkni_alloc, 3 ); /* 3 pre allocating atoms */
    BIP_CHECK_GOTO(( hResponse->factory ),       ( "batom_factory_create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    return( hResponse );

error:
    if (hResponse)
    {
        if (hResponse->factory)       {batom_factory_destroy( hResponse->factory ); }
        if (hResponse->hMutex)        {B_Mutex_Destroy( hResponse->hMutex ); }
        if (hResponse->hHeaderList)   {BIP_HttpHeaderList_Destroy( hResponse->hHeaderList, &hResponse->createSettings ); }
        if (hResponse->hReasonPhrase) {BIP_String_Destroy( hResponse->hReasonPhrase ); }
        if (hResponse->hStatus)       {BIP_String_Destroy( hResponse->hStatus ); }

        BDBG_OBJECT_DESTROY( hResponse, BIP_HttpResponse );

        B_Os_Free( hResponse );
    }

    return( NULL );
}                                                          /* BIP_HttpResponse_Create */

/*****************************************************************************
 *  Populate a BIP_HttpResponse object from an HTTP response message
 *  that is contained in one or more batom_cursors. See "HTTP-message" in RFC7230).
 *
 *  Refer to /BSEAV/lib/utils/bioatom.h for information on the
 *  usage of the "batom" APIs and data types.
 *
 *  An single HTTP response can be spread across multiple batom_cursors.
 *  This API can be called sequentially to feed each of the cursors to
 *  the deserializer.  When an entire HTTP response has been deserialized, the
 *  *pDeserializeComplete output argument will be set to true, and the
 *  cursor will be left pointing to the first character after the
 *  HTTP response (or the cursor will be at EOF).
 *****************************************************************************/
BIP_Status BIP_HttpResponse_DeserializeFromAtom(
    BIP_HttpResponseHandle hResponse,                      /*!< [in]  The BIP_HttpResponse to be populated with the deserialized data. */
    batom_cursor          *pCursor,                        /*!< [in]  Cursor that contains incoming data. */
    bool                  *pDeserializeComplete            /*!< [out] Set to true if the http response has been completely deserialized. */
                                                           /*!<       i.e., it has terminating <CRNL><CRNL> or <NL><NL>. */
    )
{
    BIP_Status rc = BIP_SUCCESS;
    int        byte;                                       /* Byte from cursor or BATOM_EOF. */
    char       ch = '\0';
    bool       relook;                                     /* Set this to true to make another pass through the state machine. */

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );
    BDBG_ASSERT( pCursor );
    BDBG_ASSERT( pDeserializeComplete );

    B_Mutex_Lock( hResponse->hMutex );

    /* Can't start deserializing if we're in the middle of serializing. */
    if (hResponse->serializeState != serializeState_eIdle) {rc = BIP_ERR_SERIALIZE_IN_PROGRESS; goto error; }

    *pDeserializeComplete = false;                         /* Assume that the deserialize will not complete. */

    /* Loop for the state machine.  Setting "relook=true" will run the run
     * the state machine again to process a new state.
     * */
    for (relook = true; relook==true; ) {
        relook = false;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hResponse %p: deserializeState=0x%x \"%s\"  byteIndex=%zu"
                   BIP_MSG_PRE_ARG, (void *)hResponse, hResponse->deserializeState, toStr_deserializeState( hResponse->deserializeState ), hResponse->byteIndex ));

        switch (hResponse->deserializeState) {
            /* State: Idle
             * -----------
             * No deserialize in progress.  It's time to start a new deserialize session.
             * Input (ch) is the first byte of a message's header section. */
            case deserializeState_eIdle:
            {
                BIP_HttpResponse_Clear_locked( hResponse, NULL );

                /* Change to new state and process the input byte there. */
                hResponse->deserializeState = deserializeState_eHttpVersion;
                hResponse->byteIndex        = 0;
                relook = true;                             /* Make another pass through the state machine. */
                break;
            }

            /* State: HttpVersion
             * -------------------------
             * Deserializing the Response's "HTTP_Version" (see "HTTP-version" in RFC7320).
             */
            case deserializeState_eHttpVersion:
            {
                while (( byte = batom_cursor_next( pCursor )) != BATOM_EOF)
                {
                    const char httpVersionString[] = {'H', 'T', 'T', 'P', '/', 'x', '.', 'y', ' '};
                    size_t     byteIndex           = hResponse->byteIndex;

                    ch = byte;                             /* Convert from int to char. */

                    /* Make sure our byteIndex is in range. */
                    BIP_CHECK_GOTO(( byteIndex<sizeof( httpVersionString )), ( "byteIndex=%zu is out of range during deserialize!", byteIndex ), error, BIP_ERR_INTERNAL, rc );

                    if (httpVersionString[byteIndex] == 'x')
                    {
                        BIP_CHECK_GOTO(( ch>='0' && ch<='9' ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) for HTTP major version number.", ch, ch ), error, BIP_ERR_INVALID_PARAMETER, rc );
                        hResponse->httpVersion.major = ch - '0';
                    }
                    else if (httpVersionString[byteIndex] == 'y')
                    {
                        BIP_CHECK_GOTO(( ch>='0' && ch<='9' ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) for HTTP minor version number.", ch, ch ), error, BIP_ERR_INVALID_PARAMETER, rc );
                        hResponse->httpVersion.minor = ch - '0';
                    }
                    else
                    {
                        BIP_CHECK_GOTO(( ch==httpVersionString[byteIndex] ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) at index %zu of HTTP Version.", ch, ch, byteIndex ), error, BIP_ERR_INVALID_PARAMETER, rc );
                    }

                    byteIndex++;
                    hResponse->byteIndex = byteIndex;

                    if (byteIndex >= sizeof( httpVersionString ))
                    {
                        BDBG_MSG(( BIP_MSG_PRE_FMT "hResponse %p: Got HTTP Version=" BIP_HTTP_RESPONSE_VERSION_PRINTF_FMT
                                   BIP_MSG_PRE_ARG, (void *)hResponse, BIP_HTTP_RESPONSE_VERSION_PRINTF_ARG( &hResponse->httpVersion )));

                        hResponse->deserializeState = deserializeState_eStatusCode;
                        hResponse->byteIndex        = 0;

                        rc = BIP_String_Clear( hResponse->hStatus );
                        BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_Clear() failed" ), error, rc, rc );

                        relook = true;
                        break;
                    }
                }                                          /* End: For each byte in cursor... */
                break;
            }

            /* State: StatusCode
             * -------------------
             * Deserializing the Response's status code (see "status-code" in RFC7320).
             */
            case deserializeState_eStatusCode:
            {
                while (( byte = batom_cursor_next( pCursor )) != BATOM_EOF)
                {
                    ch = byte;                             /* Convert from int to char. */

                    /* If input belongs to the status-code, append it to the hStatus BIP_String.  */
                    if (isdigit( ch ))
                    {
                        rc = BIP_String_StrcatCharN( hResponse->hStatus, &ch, 1 );
                        BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_StrcatCharN() failed" ), error, rc, rc );
                    }
                    /* If ch is the expected terminator (space) and the hStatus is non-empty, move to next state  */
                    else if (ch == ' ')                    /* Found end of "status-code". */
                    {
                        size_t length = BIP_String_GetLength( hResponse->hStatus );
                        BIP_CHECK_GOTO(( length>0 ), ( "Deserialize error: zero-length HTTP Response status-code." ), error, BIP_ERR_INVALID_PARAMETER, rc );

                        BDBG_MSG(( BIP_MSG_PRE_FMT "hResponse %p: Got status-code=%s" BIP_MSG_PRE_ARG, (void *)hResponse, BIP_String_GetString( hResponse->hStatus )));

                        rc = BIP_String_Clear( hResponse->hReasonPhrase );
                        BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_Clear() failed" ), error, rc, rc );

                        hResponse->deserializeState = deserializeState_eReasonPhrase;
                        relook = true;
                        break;
                    }
                    /* Anything else is invalid. */
                    else
                    {
                        BIP_CHECK_GOTO(( false ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) in HTTP Response Status Code.", ch, ch ), error, BIP_ERR_INVALID_PARAMETER, rc );
                    }
                }                                          /* End: For each byte in cursor... */
                break;
            }

            /* State: ReasonPhrase
             * --------------------
             * Deserializing the Response's reason phrase,  (see "reason-phrase" in RFC7320).
             */
            case deserializeState_eReasonPhrase:
            {
                while (( byte = batom_cursor_next( pCursor )) != BATOM_EOF)
                {
                    ch = byte;                             /* Convert from int to char. */
                    /* If input belongs to the reason-phrase, append it to the hReasonPhrase BIP_String.  */
                    if (( ch > 0x21 ) || ( ch==HTAB ) || ( ch==SP ))
                    {
                        rc = BIP_String_StrcatCharN( hResponse->hReasonPhrase, &ch, 1 );
                        BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_StrcatCharN() failed" ), error, rc, rc );
                    }
                    /* If ch is the expected terminator (SP) move to next state, even if the reason-phrase is empty. */
                    else if (ch == CR)                     /* Found end of "reason-phrase". */
                    {
                        BDBG_MSG(( BIP_MSG_PRE_FMT "hResponse %p: Got Reason Phrase=\"%s\"" BIP_MSG_PRE_ARG, (void *)hResponse, BIP_String_GetString( hResponse->hReasonPhrase )));
                        hResponse->deserializeState = deserializeState_eResponseLineTerm;
                        hResponse->byteIndex        = 0;
                        relook = true;
                        break;
                    }
                    /* Anything else is invalid. */
                    else
                    {
                        BIP_CHECK_GOTO(( false ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) in HTTP Reason Phrase.", ch, ch ), error, BIP_ERR_INVALID_PARAMETER, rc );
                    }
                }                                          /* End: For each byte in cursor... */
                break;
            }

            /* State: StatusLineTerm
             * -------------------------
             * Already got a CR after the Reason Phrase.  Now we just need to wait for the LF.
             */
            case deserializeState_eResponseLineTerm:
            {
                while (( byte = batom_cursor_next( pCursor )) != BATOM_EOF)
                {
                    const char termChars[] = {LF};
                    size_t     byteIndex   = hResponse->byteIndex;

                    ch = byte;                             /* Convert from int to char. */

                    /* Make sure our byteIndex is in range. */
                    BIP_CHECK_GOTO(( byteIndex<sizeof( termChars )), ( "byteIndex=%zu is out of range during deserialize!", byteIndex ), error, BIP_ERR_INTERNAL, rc );

                    BIP_CHECK_GOTO(( ch==termChars[byteIndex] ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) at index %zu of HTTP status-line terminator.", ch, ch, byteIndex ), error, BIP_ERR_INVALID_PARAMETER, rc );

                    byteIndex++;
                    hResponse->byteIndex = byteIndex;

                    if (byteIndex >= sizeof( termChars ))
                    {
                        hResponse->deserializeState = deserializeState_eHeaders;
                        relook = true;
                        break;
                    }
                }                                          /* End: For each byte in cursor... */
                break;
            }

            /* State: Headers
             * -------------------------
             * Now for the Headers... deserialize them into an HttpHeader object that's a part of the HttpResponse.  */
            case deserializeState_eHeaders:
            {
                bool deserializeComplete;

                rc = BIP_HttpHeaderList_DeserializeFromAtom( hResponse->hHeaderList, pCursor, &deserializeComplete );
                BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpHeader_DeserializeFromAtom() failed." ), error, rc, rc );
                if (deserializeComplete)
                {
                    hResponse->deserializeState = deserializeState_eDone;
                    relook = true;
                }
                break;
            }

            /* State: Done
             * -------------------------
             * Not much to do but stay here. */
            case deserializeState_eDone:
            {
                *pDeserializeComplete       = true;
                hResponse->deserializeState = deserializeState_eIdle;
                /* Don't set relook or we'll start deserializing again! */
                break;
            }

            /* State: Anything not handled.
             * -------------------------
             * This should never happen. */
            default:
            {
                BIP_CHECK_GOTO(( false ), ( "hResponse %p: Unhandled state=%s (%d)", (void *)hResponse, toStr_deserializeState( hResponse->deserializeState ), hResponse->deserializeState ),
                    error, BIP_ERR_INTERNAL, rc );

                break;
            }
        }                                                  /* End switch/case. */
    }

error:
    if (rc != BIP_SUCCESS)
    {
        hResponse->deserializeState = deserializeState_eIdle;
    }

    B_Mutex_Unlock( hResponse->hMutex );
    return( rc );
}                                                          /* BIP_HttpResponse_DeserializeFromAtom */

/*****************************************************************************
 *  Populate a BIP_HttpResponse object from an HTTP response message
 *  that is contained in one or more memory buffers (char * arrays).
 *****************************************************************************/
BIP_Status BIP_HttpResponse_DeserializeFromBuffer(
    BIP_HttpResponseHandle hResponse,                      /*!< [in]  The HttpResponse to be populated with the deserialized data. */
    const char            *buffer,                         /*!< [in]  Buffer that contains incoming data. */
    size_t                 bufferSize,                     /*!< [in]  Input buffer size. */
    bool                  *pDeserializeComplete,           /*!< [out] Set to true if the http response is complete, ie it has */
                                                           /*!<       end of header(<CRNL><CRNL or <NL><NL>). */
    size_t                *pConsumedBytes                  /*!< [out] If response is complete then this specifies the consumed */
                                                           /*!<       data size. If message is not complete then this must be equal to bufferSize. */
    )
{
    BIP_Status   rc     = BIP_SUCCESS;
    batom_t      myAtom = NULL;
    batom_cursor myCursor;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );
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
    myAtom = batom_from_range( hResponse->factory, buffer, bufferSize, NULL, NULL );
    batom_cursor_from_atom( &myCursor, myAtom );

    rc = BIP_HttpResponse_DeserializeFromAtom( hResponse, &myCursor, pDeserializeComplete );

    /* Tell the caller how many bytes we've consumed here! */
    *pConsumedBytes = batom_cursor_pos( &myCursor );

    batom_release( myAtom );
    return( rc );
}                                                          /* BIP_HttpResponse_DeserializeFromBuffer */

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
static BIP_Status BIP_HttpResponse_SerializeChars(
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
}                                                          /* BIP_HttpResponse_SerializeChars */

/*****************************************************************************
 *  Populate the caller's memory buffer (char array) with a "serialized"
 *  version of the HttpResponse that is suitable for sending over the network.
 *****************************************************************************/
static BIP_Status BIP_HttpResponse_SerializeToBuffer_locked(
    BIP_HttpResponseHandle hResponse,                      /*!< Handle of the HttpResponse to be serialized. */
    char                  *pBuffer,                        /*!< Buffer to receive the serialized output. */
    size_t                 bufferSize,                     /*!< Buffer size. */
    bool                  *pSerializeComplete,             /*!< Set to true if serializing is complete. Otherwise, */
                                                           /*!< the buffer is full, call this again with a new buffer.  */
    size_t                *pSerializedBytes                /*!< The number of bytes that have been placed in the buffer. */
    )
{
    BIP_Status rc        = BIP_SUCCESS;
    size_t     destIndex = 0;                              /* Index into callers buffer (pBuffer). */
    bool       relook;                                     /* Set this to true to make another pass through the state machine. */

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    /* BIP_HttpResponse_GetSerializeBufferSize() calls this function with
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

    B_MUTEX_ASSERT_LOCKED( hResponse->hMutex );

    /* Can't start serializing if we're in the middle of deserializing. */
    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }

    if (pSerializeComplete) {*pSerializeComplete = false; } /* Assume that we won't finish the serialize in one call. */

    /*************************************************************************
     * Loop for the state machine.  Setting "relook=true" will run the run
     * the state machine again to process a new state.
     *************************************************************************/
    for (relook = true; relook==true; ) {
        relook = false;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hResponse %p: serializeState=0x%x \"%s\"  byteIndex=%zu"
                   BIP_MSG_PRE_ARG, (void *)hResponse, hResponse->serializeState, toStr_serializeState( hResponse->serializeState ), hResponse->byteIndex ));

        switch (hResponse->serializeState) {
            /* State: Idle
             * -----------
             * No serialize in progress.  It's time to start a new serializing session.
             * Initialize the serialize state. */
            case serializeState_eIdle:
            {
                hResponse->serializeState = serializeState_eVersionPrefix;
                hResponse->byteIndex      = 0;
                relook = true;
                break;
            }

            /* State: VersionPrefix
             * --------------------
             * Serialize the constant prefix of the HTTP Version string ("HTTP/"). */
            case serializeState_eVersionPrefix:
            {
                const char pChars[] = {'H', 'T', 'T', 'P', '/'};
                size_t     nChars   = sizeof( pChars );

                rc = BIP_HttpResponse_SerializeChars( pChars, nChars, &hResponse->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hResponse->serializeState = serializeState_eVersionMajor;
                    relook = true;
                }
                break;
            }

            /* State: VersionMajor
             * -------------------
             * Serialize the HTTP major version number: hResponse->httpVersion.major. */
            case serializeState_eVersionMajor:
            {
                char   pChars[1];
                size_t nChars = sizeof( pChars );

                int major = hResponse->httpVersion.major;

                if (( major<0 ) || ( major>9 ))
                {
                    BDBG_ASSERT( false );
                    pChars[0] = '?';                       /* Yikes! Invalid major version!. */
                }
                else
                {
                    pChars[0] = major + '0';
                }

                rc = BIP_HttpResponse_SerializeChars( pChars, nChars, &hResponse->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hResponse->serializeState = serializeState_eVersionDot;
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

                rc = BIP_HttpResponse_SerializeChars( pChars, nChars, &hResponse->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hResponse->serializeState = serializeState_eVersionMinor;
                    relook = true;
                }
                break;
            }

            /* State: VersionMinor
             * -------------------
             * Serialize the HTTP minor version number: hResponse->httpVersion.minor. */
            case serializeState_eVersionMinor:
            {
                char   pChars[1];
                size_t nChars = sizeof( pChars );

                int minor = hResponse->httpVersion.minor;

                if (( minor<0 ) || ( minor>9 ))
                {
                    BDBG_ASSERT( false );
                    pChars[0] = '?';                       /* Yikes! Invalid minor version!. */
                }
                else
                {
                    pChars[0] = minor + '0';
                }

                rc = BIP_HttpResponse_SerializeChars( pChars, nChars, &hResponse->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hResponse->serializeState = serializeState_eVersionSpace;
                    relook = true;
                }
                break;
            }

            /* State: VersionSpace
             * ------------------
             * Add a space after the HTTP Method. */
            case serializeState_eVersionSpace:
            {
                const char pChars[] = {SP};                /* ASCII space character. */
                size_t     nChars   = sizeof( pChars );

                rc = BIP_HttpResponse_SerializeChars( pChars, nChars, &hResponse->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hResponse->serializeState = serializeState_eStatusCode;
                    relook = true;
                }
                break;
            }

            /* State: StatusCode
             * -------------
             * Serialize the Response's HTTP status-code: hResponse->hStatus.  */
            case serializeState_eStatusCode:
            {
                /* coverity[var_deref_op: FALSE] */
                const char *pChars = BIP_String_GetString( hResponse->hStatus );
                size_t      nChars = BIP_String_GetLength( hResponse->hStatus );

                rc = BIP_HttpResponse_SerializeChars( pChars, nChars, &hResponse->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hResponse->serializeState = serializeState_eStatusCodeSpace;
                    relook = true;
                }
                break;
            }

            /* State: StatusCodeSpace
             * ------------------
             * Add a space after the HTTP status-code. */
            case serializeState_eStatusCodeSpace:
            {
                const char pChars[] = {SP};                /* ASCII space character. */
                size_t     nChars   = sizeof( pChars );

                rc = BIP_HttpResponse_SerializeChars( pChars, nChars, &hResponse->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hResponse->serializeState = serializeState_eReasonPhrase;
                    relook = true;
                }
                break;
            }

            /* State: ReasonPhrase
             * -------------
             * Serialize the ReasonPhrase: hResponse->hReasonPhrase.  */
            case serializeState_eReasonPhrase:
            {
                /* coverity[var_deref_op: FALSE] */
                const char *pChars = BIP_String_GetString( hResponse->hReasonPhrase );
                size_t      nChars = BIP_String_GetLength( hResponse->hReasonPhrase );

                rc = BIP_HttpResponse_SerializeChars( pChars, nChars, &hResponse->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hResponse->serializeState = serializeState_eResponseLineTerm;
                    relook = true;
                }
                break;
            }

            /* State: ResponseLineTerm
             * ----------------------
             * Serialize the terminating CRLF at the end of the response-line. */
            case serializeState_eResponseLineTerm:
            {
                const char pChars[] = {CR, LF};
                size_t     nChars   = sizeof( pChars );

                rc = BIP_HttpResponse_SerializeChars( pChars, nChars, &hResponse->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hResponse->serializeState = serializeState_eHeaders;
                    relook = true;
                }
                break;
            }

            /* State: Headers
             * --------------
             * Serializing the Response's list of BIP_HttpHeaders . */
            case serializeState_eHeaders:
            {
                bool   serializeComplete;
                size_t serializedBytes;

                if (pBuffer)                               /* If we have a real buffer, then serialize the headers into it. */
                {
                    rc = BIP_HttpHeaderList_SerializeToBuffer( hResponse->hHeaderList, pBuffer+destIndex, bufferSize-destIndex, &serializeComplete, &serializedBytes );
                    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpHeader_SerializeToBuffer() failed" ), error, rc, rc );
                }
                else                                       /* If no buffer, we just need the size of the serialize. */
                {
                    rc                = BIP_HttpHeaderList_GetSerializeBufferSize( hResponse->hHeaderList, &serializedBytes );
                    serializeComplete = true;              /* _GetSerializeBufferSize() always completes in one call. */
                }

                destIndex += serializedBytes;

                if (serializeComplete)
                {
                    hResponse->serializeState = serializeState_eDone;
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
                hResponse->serializeState = serializeState_eIdle;
                break;
            }

            /* State: Anything not handled.
             * ----------------------------
             * This should never happen. */
            default:
            {
                BIP_CHECK_GOTO(( false ), ( "hResponse %p: Unhandled state=%s (%d)", (void *)hResponse, toStr_serializeState( hResponse->serializeState ), hResponse->serializeState ),
                    error, BIP_ERR_INTERNAL, rc );
                break;
            }
        }                                                  /* End switch/case. */
    }                                                      /* End Do while relook is true. */

error:
    if (pSerializedBytes) {*pSerializedBytes = destIndex; }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hResponse %p: returning *pSerializeComplete=%s *pSerializedBytes=%zu"
               BIP_MSG_PRE_ARG, (void *)hResponse,
               ( pSerializeComplete==NULL ) ? "N/A" : ( *pSerializeComplete ) ? "TRUE" : "FALSE",
               pSerializedBytes ? *pSerializedBytes : 0 ));

    return( BIP_SUCCESS );
}                                                          /* BIP_HttpResponse_SerializeToBuffer_locked */

/*****************************************************************************
 *  Populate the caller's memory buffer (char array) with a "serialized"
 *  version of the HttpResponse that is suitable for sending over the network.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_SerializeToBuffer(
    BIP_HttpResponseHandle hResponse,                      /*!< [in]  Handle of the HttpResponse to be serialized. */
    char                  *pBuffer,                        /*!< [out] Buffer to receive the serialized output. */
    size_t                 bufferSize,                     /*!< [in]  Buffer size. */
    bool                  *pSerializeComplete,             /*!< [out] Set to true if serializing is complete. Otherwise, */
                                                           /*!<       the buffer is full, call this again with a new buffer.  */
    size_t                *pSerializedBytes                /*!< [out] The number of bytes that have been placed in the buffer. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    B_Mutex_Lock( hResponse->hMutex );

    rc = BIP_HttpResponse_SerializeToBuffer_locked( hResponse, pBuffer, bufferSize, pSerializeComplete, pSerializedBytes );

    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}

/*****************************************************************************
 *  Determine the number of bytes that a BIP_HttpResponse will occupy
 *  when it is serialized (converted to one long character string for
 *  transmission over the network).
 *****************************************************************************/
BIP_Status BIP_HttpResponse_GetSerializeBufferSize(
    BIP_HttpResponseHandle hResponse,                      /*!< Handle of HttpResponse to be serialized. */
    size_t                *pSerializeBufferSize            /*!< Address of variable to receive the buffer size
                                                          required for serializing the HttpResponse. */
    )
{
    BIP_Status rc;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );
    BDBG_ASSERT( pSerializeBufferSize );

    rc = BIP_HttpResponse_SerializeToBuffer( hResponse, NULL, 0, NULL, pSerializeBufferSize );
    BIP_CHECK_LOGERR(( rc==BIP_SUCCESS ), ( "BIP_HttpHeader_SerializeToBuffer failed" ), rc, rc );

    return( rc );
}

/*****************************************************************************
 *  API to serialize a BIP_HttpResponse  to an atom.
 *
 *  Produce a "batom" that contains a serialized version of the HttpResponse
 *  that is suitable for sending over the network.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_SerializeToAtom(
    BIP_HttpResponseHandle hResponse,                      /*!< [in]  Handle of HttpResponse to be serialized. */
    batom_factory_t        factory,                        /*!< [in]  Factory to use for creating atom . */
    batom_t               *pAtom                           /*!< [out] Where to put the atom containing the serialized data. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    size_t  serializeLen      = 0;
    batom_t myAtom            = NULL;
    char   *pBuffer           = NULL;
    bool    serializeComplete = false;
    size_t  serializedBytes;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );
    BDBG_ASSERT( pAtom );

    B_Mutex_Lock( hResponse->hMutex );

    /* Can't start serializing if we're in the middle of deserializing. */
    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }

    /*************************************************************************
     *  Figure out how long the serialized response will be.
     *************************************************************************/
    rc = BIP_HttpResponse_SerializeToBuffer_locked( hResponse, NULL, 0, NULL, &serializeLen );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpResponse_SerializeToBuffer_locked failed." ), error, rc, rc );

    /*************************************************************************
     *  Malloc some memory, then create an atom out of it.
     *************************************************************************/
    pBuffer = B_Os_Malloc( serializeLen );
    BIP_CHECK_GOTO(( pBuffer ), ( "B_Os_Malloc (%zu bytes) failed.", serializeLen ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    myAtom = BIP_Atom_AtomFromBOsMallocRange( factory, pBuffer, serializeLen );

    /*************************************************************************
     *  Now serialize into the atom.
     *************************************************************************/
    rc = BIP_HttpResponse_SerializeToBuffer_locked( hResponse, pBuffer, serializeLen, &serializeComplete, &serializedBytes );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpResponse_SerializeToBuffer failed." ), error, rc, rc );

    BDBG_ASSERT( serializeComplete );                      /* The buffer was big enough, serialize should be complete. */
    BDBG_ASSERT( serializedBytes == serializeLen );        /* We should have filled the whole buffer. */

    *pAtom = myAtom;
    B_Mutex_Unlock( hResponse->hMutex );
    return( rc );

error:

    if (myAtom) {batom_release( myAtom ); }                /* This will free the buffer as well. */
    else if (pBuffer)
    {
        B_Os_Free( pBuffer );
    }

    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_SerializeToAtom */

/*****************************************************************************
 *  API to print BIP_HttpResponse using BDBG_LOG.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_Print(
    BIP_HttpResponseHandle         hResponse,              /*!< [in] Handle of BIP_HttpResponse to be printed. */
    const char                    *pLineHeading,           /*!< [in] If non-NULL, will be prefixed to each line printed. */
    BIP_HttpResponsePrintSettings *pSettings               /*!< [in] Optional settings structure. Pass NULL to use default settings. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    size_t serializeLen      = 0;
    char  *pBuffer           = NULL;
    bool   serializeComplete = false;
    size_t serializedBytes;

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );
    BIP_SETTINGS_ASSERT( pSettings, BIP_HttpResponsePrintSettings );

    B_Mutex_Lock( hResponse->hMutex );

    /* Skip the check of the deserialize state.  Even if we're in the middle of deserializing, just
     * print what we have so far. */

    if (pLineHeading == NULL)
    {
        pLineHeading = "HTTP Response:";
    }

    /*************************************************************************
     *  Figure out how long the serialized response will be.
     *************************************************************************/
    rc = BIP_HttpResponse_SerializeToBuffer_locked( hResponse, NULL, 0, NULL, &serializeLen );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpResponse_SerializeToBuffer_locked failed." ), error, rc, rc );

    serializeLen += 1;                                     /* Allow for NULL-terminator. */

    pBuffer = B_Os_Malloc( serializeLen );
    BIP_CHECK_GOTO(( pBuffer ), ( "B_Os_Malloc (%zu bytes) failed.", serializeLen ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    /*************************************************************************
     *  Now serialize into the buffer.
     *************************************************************************/
    rc = BIP_HttpResponse_SerializeToBuffer_locked( hResponse, pBuffer, serializeLen, &serializeComplete, &serializedBytes );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpResponse_SerializeToBuffer_locked failed." ), error, rc, rc );

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
    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_Print */

/*****************************************************************************
 *  API to add a "Content-Range" header to the HttpResponse.
 *****************************************************************************/
BIP_Status BIP_HttpResponse_SetContentRangeHeaderByteRange(
    BIP_HttpResponseHandle hResponse,                      /*!< [in]  Handle of the HttpResponse to be modified. */
    uint64_t               firstBytePos,                   /*!< [in]  First byte position. Use BIP_U64_MAX to indicate an unsatisfied range. */
    uint64_t               lastBytePos,                    /*!< [in]  Last byte position. Use BIP_U64_MAX to indicate an unsatisfied range. */
    uint64_t               completeLength                  /*!< [in]  Complete length of representation. Use BIP_U64_MAX if length is unknown. */
    )
{
    BIP_Status           rc                = BIP_SUCCESS;
    BIP_HttpHeaderHandle hHeader           = NULL;
    BIP_StringHandle     hString           = NULL;
    const char          *pContentRangeName = "Content-Range";

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    BDBG_ASSERT( hResponse );
    BDBG_OBJECT_ASSERT( hResponse, BIP_HttpResponse );

    B_Mutex_Lock( hResponse->hMutex );
    if (hResponse->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS;  goto error; }
    if (hResponse->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;    goto error; }

    /* First, decide if we're building a "byte-range-resp" or an "unsatisfied-range"
     * style of "Content-Range" header. */
    if (( firstBytePos == BIP_U64_MAX ) && ( lastBytePos == BIP_U64_MAX ))
    {
        /* Build an "unsatisfied-range" type of Content-Range header. */
        hString = BIP_String_CreateFromPrintf( "bytes */" BIP_U64_FMT, BIP_U64_ARG( completeLength ));
    }
    else
    {
        /* Build an "byte-range-resp" type of Content-Range header. */

        /* If last-byte-pos is less than first-byte-pos, that's invalid. */
        BIP_CHECK_GOTO(( firstBytePos <= lastBytePos ),
            ( "Content-Range header: firstBytePos:%" PRIu64 " is greater than lastBytePos:%" PRIu64 "", firstBytePos, lastBytePos ),
            error, BIP_ERR_INVALID_PARAMETER, rc );

        if (completeLength == BIP_U64_MAX)                 /* If completeLength is unknown... */
        {
            hString = BIP_String_CreateFromPrintf( "bytes " BIP_U64_FMT "-" BIP_U64_FMT "/*",
                    BIP_U64_ARG( firstBytePos ),
                    BIP_U64_ARG( lastBytePos ));
        }
        else                                               /* Else completeLength is known. */
        {
            /* Make sure that last-byte-pos is less than complete-length. */
            BIP_CHECK_GOTO(( lastBytePos < completeLength ),
                ( "Content-Range header: lastBytePos:%" PRIu64 " is not less than completeLength:%" PRIu64 "", lastBytePos, completeLength ),
                error, BIP_ERR_INVALID_PARAMETER, rc );

            hString = BIP_String_CreateFromPrintf( "bytes "BIP_U64_FMT "-" BIP_U64_FMT "/" BIP_U64_FMT,
                    BIP_U64_ARG( firstBytePos ),
                    BIP_U64_ARG( lastBytePos ),
                    BIP_U64_ARG( completeLength ));
        }
    }
    BIP_CHECK_GOTO(( hString!=NULL ), ( "BIP_String_CreateFromPrintf() failed." ), error, BIP_ERR_CREATE_FAILED, rc );

    /* We don't want multiple "Content-Range" headers, so delete any that might already exist. */
    for (;; ) {
        rc = BIP_HttpHeaderList_GetNextHeader( hResponse->hHeaderList, NULL, pContentRangeName, &hHeader, NULL );
        if (rc==BIP_INF_NOT_AVAILABLE) {break; }
        BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpResponse_GetNextHeader() failed" ), error, rc, rc );

        BIP_HttpHeaderList_DestroyHeader( hHeader );
    }

    hHeader = BIP_HttpHeaderList_CreateHeader( hResponse->hHeaderList, pContentRangeName, BIP_String_GetString( hString ), NULL );
    BIP_CHECK_GOTO(( hHeader!=NULL ), ( "BIP_HttpHeaderList_CreateHeader() failed." ), error, BIP_ERR_CREATE_FAILED, rc );

    BDBG_MSG(( BIP_MSG_PRE_FMT "Added header: %s: %s" BIP_MSG_PRE_ARG, pContentRangeName, BIP_String_GetString( hString )));

    rc = BIP_SUCCESS;

error:
    if (hString) {BIP_String_Destroy( hString ); }
    B_Mutex_Unlock( hResponse->hMutex );

    return( rc );
}                                                          /* BIP_HttpResponse_SetContentRangeHeaderByteRange */
