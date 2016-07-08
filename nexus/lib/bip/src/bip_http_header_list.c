/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "bip_priv.h"
#include "blst_queue.h"
#include "bioatom.h"

BDBG_MODULE( bip_http_header_list );
BDBG_OBJECT_ID( BIP_HttpHeader );
BDBG_OBJECT_ID( BIP_HttpHeaderList );

#define BIP_HTTP_HEADER_PRINTF_FMT "[hHandle=%p hList=%p Name=\"%s\" Value=\"%s\"]"
#define BIP_HTTP_HEADER_PRINTF_ARG( h )                           \
    (void*)(( h ) ? ( h ) : 0),                                \
    (void *)(( h ) ? ( h )->hList : 0),                                     \
    ( h ) ?  BIP_String_GetString(( h )->hName ) : "NULL handle", \
    ( h ) ?  BIP_String_GetString(( h )->hValue ) : ""

/*****************************************************************************
 *  Deserializer states:
 *****************************************************************************/
typedef enum BIP_HttpHeaderListDeserializeState {
    deserializeState_eIdle = 0,
    deserializeState_eNextHeader,
    deserializeState_eHdrFieldName,
    deserializeState_ePreFieldOws,
    deserializeState_eFieldValuePostOws,
    deserializeState_eHeaderCr,
    deserializeState_eHeaderCrLf,
    deserializeState_eObsFoldSpaceTab,
    deserializeState_eEndOfListCr,
    deserializeState_eDone
} BIP_HttpHeaderListDeserializeState;

static const char *toStr_deserializeState(
    int value
    )
{
    const namevalue_t myStrings[] = {
        {"Idle",            deserializeState_eIdle                  },
        {"NextHeader",      deserializeState_eNextHeader            },
        {"HdrFieldName",    deserializeState_eHdrFieldName          },
        {"PreFieldOws",     deserializeState_ePreFieldOws           },
        {"FieldValue",      deserializeState_eFieldValuePostOws     },
        {"HeaderCr",        deserializeState_eHeaderCr              },
        {"HeaderCrLf",      deserializeState_eHeaderCrLf            },
        {"ObsFoldSpaceTab", deserializeState_eObsFoldSpaceTab       },
        {"EndOfListCr",     deserializeState_eEndOfListCr           },
        {"Done",            deserializeState_eDone                  },
        {NULL,              0                                       }
    };

    return( lookup_name( myStrings, ( value )));
}                                                          /* toStr_deserializeState */

/* Define some non-printable ASCII characters as they are named in RFC5234. */
#define SP   ( 0x20 )
#define HTAB ( 0x09 )
#define CR   ( 0x0d )
#define LF   ( 0x0a )

/*****************************************************************************
 *  Serializer states:
 *****************************************************************************/
typedef enum BIP_HttpHeaderListSerializeState {
    serializeState_eIdle = 0,
    serializeState_eHdrName,
    serializeState_eSeparator,
    serializeState_eHdrValue,
    serializeState_eHdrTerm,
    serializeState_eNextHdr,
    serializeState_eListTerm,
    serializeState_eDone
} BIP_HttpHeaderListSerializeState;

static const char *toStr_serializeState(
    int value
    )
{
    const namevalue_t myStrings[] = {
        {"Idle",      serializeState_eIdle     },
        {"HdrName",   serializeState_eHdrName  },
        {"Separator", serializeState_eSeparator},
        {"HdrValue",  serializeState_eHdrValue },
        {"HdrTerm",   serializeState_eHdrTerm  },
        {"NextHdr",   serializeState_eNextHdr  },
        {"ListTerm",  serializeState_eListTerm },
        {"Done",      serializeState_eDone     },
        {NULL,        0                        }
    };

    return( lookup_name( myStrings, ( value )));
}

/*****************************************************************************
 *  Object structures:
 *****************************************************************************/
typedef struct BIP_HttpHeaderList
{
    BDBG_OBJECT( BIP_HttpHeaderList )

    void    *ownerContext;                                 /* Any pointer passed to Create(), must also be passed to Destroy(). */

    BLST_Q_HEAD( headerListHead, BIP_HttpHeader )       headerListHead;

    BIP_HttpHeaderListDeserializeState deserializeState;
    BIP_HttpHeaderListSerializeState   serializeState;

    size_t                             byteIndex;          /* Byte index within current header being serialized. */
    BIP_StringHandle                   hName;
    BIP_StringHandle                   hValue;
    BIP_HttpHeaderHandle               hHeader;            /* Header currently being serialized. */

    B_MutexHandle                      hMutex;

    batom_factory_t                    factory;
} BIP_HttpHeaderList;

typedef struct BIP_HttpHeader
{
    BDBG_OBJECT( BIP_HttpHeader )

    BIP_HttpHeaderListHandle hList;                        /* The handle of the HttpHeaderList that this header belongs to. */
    BIP_StringHandle hName;
    BIP_StringHandle hValue;

    BLST_Q_ENTRY( BIP_HttpHeader )   headerListNext;
} BIP_HttpHeader;

/*****************************************************************************
 *  Set a Header's value.
 *****************************************************************************/
BIP_Status BIP_HttpHeaderList_SetHeaderValue(
    BIP_HttpHeaderHandle hHeader,
    const char          *pValue
    )
{
    BIP_Status               rc = BIP_SUCCESS;
    BIP_HttpHeaderListHandle hList;

    BDBG_OBJECT_ASSERT( hHeader, BIP_HttpHeader );

    hList = hHeader->hList;
    BDBG_ASSERT( hList );

    B_Mutex_Lock( hList->hMutex );
    if (hList->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }
    if (hList->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;   goto error; }

    rc = BIP_String_StrcpyChar( hHeader->hValue, pValue );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_String_StrcpyChar() Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

error:
    B_Mutex_Unlock( hList->hMutex );

    return( rc );
}                                                          /* BIP_HttpHeaderList_SetHeaderValue */

/*****************************************************************************
 *  Get a Header's value.
 *****************************************************************************/
BIP_Status BIP_HttpHeaderList_GetHeaderValue(
    BIP_HttpHeaderHandle hHeader,
    const char         **ppValue
    )
{
    BIP_Status               rc = BIP_SUCCESS;
    BIP_HttpHeaderListHandle hList;

    BDBG_ASSERT( ppValue );
    BDBG_OBJECT_ASSERT( hHeader, BIP_HttpHeader );

    hList = hHeader->hList;
    BDBG_ASSERT( hList );

    B_Mutex_Lock( hList->hMutex );
    if (hList->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }
    if (hList->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;   goto error; }

    *ppValue = BIP_String_GetString( hHeader->hValue );

error:
    B_Mutex_Unlock( hList->hMutex );

    return( rc );
}                                                          /* BIP_HttpHeaderList_GetHeaderValue */

/*****************************************************************************
 *  Get a Header's name.
 *****************************************************************************/
BIP_Status BIP_HttpHeaderList_GetHeaderName(
    BIP_HttpHeaderHandle hHeader,
    const char         **ppName
    )
{
    BIP_Status               rc = BIP_SUCCESS;
    BIP_HttpHeaderListHandle hList;

    BDBG_ASSERT( ppName );
    BDBG_OBJECT_ASSERT( hHeader, BIP_HttpHeader );

    hList = hHeader->hList;
    BDBG_ASSERT( hList );

    B_Mutex_Lock( hList->hMutex );
    if (hList->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }
    if (hList->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;   goto error; }

    *ppName = BIP_String_GetString( hHeader->hName );

error:
    B_Mutex_Unlock( hList->hMutex );

    return( rc );
}                                                          /* BIP_HttpHeaderList_GetHeaderName */

/*****************************************************************************
 *  Move a Header and place it before a specified Header (or at the end
 *  of its list).
 *****************************************************************************/
BIP_Status BIP_HttpHeaderList_MoveHeader(
    BIP_HttpHeaderHandle hHeader,
    BIP_HttpHeaderHandle hPutBeforeThisHeader
    )
{
    BIP_Status               rc = BIP_SUCCESS;
    BIP_HttpHeaderListHandle hList;

    BDBG_OBJECT_ASSERT( hHeader, BIP_HttpHeader );

    hList = hHeader->hList;
    BDBG_ASSERT( hList );

    B_Mutex_Lock( hList->hMutex );
    if (hList->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }
    if (hList->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;   goto error; }

    BLST_Q_REMOVE( &hList->headerListHead, hHeader, headerListNext );

    if (hPutBeforeThisHeader)
    {
        BDBG_OBJECT_ASSERT( hPutBeforeThisHeader, BIP_HttpHeader );
        BLST_Q_INSERT_BEFORE( &hList->headerListHead, hPutBeforeThisHeader, hHeader, headerListNext );
    }
    else
    {
        BLST_Q_INSERT_TAIL( &hList->headerListHead, hHeader, headerListNext );
    }

error:
    B_Mutex_Unlock( hList->hMutex );

    return( rc );
}                                                          /* BIP_HttpHeaderList_MoveHeader */

/*****************************************************************************
 *  Get the handle of the Header (optionally with a specified name) that
 *  follows a given Header in a HeaderList.
 *****************************************************************************/
BIP_Status BIP_HttpHeaderList_GetNextHeader(
    BIP_HttpHeaderListHandle hList,
    BIP_HttpHeaderHandle     hCurrentHeader,
    const char              *pName,
    BIP_HttpHeaderHandle    *phNextHeader,
    const char             **ppNextHeaderValue
    )
{
    BIP_Status           rc = BIP_SUCCESS;
    BIP_HttpHeaderHandle hHeader;

    BDBG_ASSERT( hList );
    BDBG_OBJECT_ASSERT( hList, BIP_HttpHeaderList );

    if (hCurrentHeader)
    {
        BDBG_OBJECT_ASSERT( hCurrentHeader, BIP_HttpHeader );
    }

    B_Mutex_Lock( hList->hMutex );
    if (hList->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }
    if (hList->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;   goto error; }

    hHeader = ( hCurrentHeader ) ? BLST_Q_NEXT( hCurrentHeader, headerListNext ) : BLST_Q_FIRST( &hList->headerListHead );

    if (pName)
    {
        while (hHeader != NULL) {
            /* RFC7230 Sec 3.2: Header names are case insensitive. */
            if (strcasecmp( pName, BIP_String_GetString( hHeader->hName ))==0)
            {
                break;
            }
            hHeader = BLST_Q_NEXT( hHeader, headerListNext );
        }
    }

    if (phNextHeader)
    {
        *phNextHeader = hHeader;
    }

    if (hHeader == NULL)
    {
        rc = BIP_INF_NOT_AVAILABLE;
        if (ppNextHeaderValue)
        {
            *ppNextHeaderValue = NULL;
        }
    }
    else                                                   /* We found a header. */
    {
        if (ppNextHeaderValue)
        {
            *ppNextHeaderValue = BIP_String_GetString( hHeader->hValue );
        }
    }

    BDBG_MSG(( BIP_MSG_PRE_FMT "Got %s Header: "BIP_HTTP_HEADER_PRINTF_FMT
               BIP_MSG_PRE_ARG, hCurrentHeader ? "Next" : "First", BIP_HTTP_HEADER_PRINTF_ARG( hHeader )));

error:
    B_Mutex_Unlock( hList->hMutex );

    return( rc );
}                                                          /* BIP_HttpHeaderList_GetNextHeader */

/*****************************************************************************
 *  Destroy a Header, after removing it from its HeaderList.
 *  Note: The HeaderList mutex must be locked before calling this function.
 *****************************************************************************/
static BIP_Status BIP_HttpHeader_Destroy_locked(
    BIP_HttpHeaderHandle hHeader
    )
{
    BIP_Status               rc = BIP_SUCCESS;
    BIP_HttpHeaderListHandle hList;

    BDBG_OBJECT_ASSERT( hHeader, BIP_HttpHeader );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHeader %p: Entry..." BIP_MSG_PRE_ARG, (void *)hHeader ));

    hList = hHeader->hList;
    BDBG_ASSERT( hList );
    B_MUTEX_ASSERT_LOCKED( hList->hMutex );

    /* Remove ourself from the HeaderList. */
    if (hHeader->hList)
    {
        BLST_Q_REMOVE( &hHeader->hList->headerListHead, hHeader, headerListNext );
    }

    if (hHeader->hName)
    {
        BIP_String_Destroy( hHeader->hName );
    }
    if (hHeader->hValue)
    {
        BIP_String_Destroy( hHeader->hValue );
    }

    BDBG_OBJECT_DESTROY( hHeader, BIP_HttpHeader );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHeader %p: Freeing object memory" BIP_MSG_PRE_ARG, (void *)hHeader ));
    B_Os_Free( hHeader );

    return( rc );
}                                                          /* BIP_HttpHeader_Destroy_locked */

/*****************************************************************************
 *  Destroy a Header, after removing it from its HeaderList.
 *****************************************************************************/
BIP_Status BIP_HttpHeaderList_DestroyHeader(
    BIP_HttpHeaderHandle hHeader
    )
{
    BIP_Status               rc = BIP_SUCCESS;
    BIP_HttpHeaderListHandle hList;

    BDBG_OBJECT_ASSERT( hHeader, BIP_HttpHeader );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hHeader %p: Entry..." BIP_MSG_PRE_ARG, (void *)hHeader ));

    hList = hHeader->hList;
    BDBG_ASSERT( hList );

    B_Mutex_Lock( hList->hMutex );
    if (hList->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }
    if (hList->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;   goto error; }

    rc = BIP_HttpHeader_Destroy_locked( hHeader );

error:
    B_Mutex_Unlock( hList->hMutex );

    return( rc );
}                                                          /* BIP_HttpHeaderList_DestroyHeader */

/*****************************************************************************
 *  Private API to create an HttpHeader by internal deserializing functions.
 *  Note: The HeaderList mutex must be locked before calling this function.
 *****************************************************************************/
static BIP_HttpHeaderHandle BIP_HttpHeader_Create_locked(
    BIP_HttpHeaderListHandle hList,
    const char              *pName,
    const char              *pValue,
    BIP_HttpHeaderHandle     hAddBeforeThisHeader
    )
{
    BIP_Status           rc      = BIP_SUCCESS;
    BIP_HttpHeaderHandle hHeader = NULL;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    BDBG_ASSERT( pName );
    BDBG_ASSERT( hList );
    BDBG_OBJECT_ASSERT( hList, BIP_HttpHeaderList );
    B_MUTEX_ASSERT_LOCKED( hList->hMutex );

    /* Allocate memory for the object. */
    hHeader = B_Os_Calloc( 1, sizeof( *hHeader ));
    BIP_CHECK_GOTO(( hHeader !=NULL ), ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_OBJECT_SET( hHeader, BIP_HttpHeader );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hHeader %p: Allocated " BIP_MSG_PRE_ARG, (void *)hHeader ));

    hHeader->hList = hList;
    hHeader->hName = BIP_String_CreateFromChar( pName );
    BIP_CHECK_GOTO(( hHeader->hName ), ( "BIP_String_CreateFromChar() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hHeader->hValue = BIP_String_CreateFromChar( pValue ); /* Works even if pValue is NULL. */
    BIP_CHECK_GOTO(( hHeader->hValue ), ( "BIP_String_CreateFromChar() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    if (hAddBeforeThisHeader)
    {
        BDBG_OBJECT_ASSERT( hAddBeforeThisHeader, BIP_HttpHeader );

        BDBG_MSG(( BIP_MSG_PRE_FMT "hHeader %p: Inserting before hHeader: %p " BIP_MSG_PRE_ARG, (void *)hHeader, (void *)hAddBeforeThisHeader ));
        BLST_Q_INSERT_BEFORE( &hList->headerListHead, hAddBeforeThisHeader, hHeader, headerListNext );
    }
    else
    {
        BDBG_MSG(( BIP_MSG_PRE_FMT "hHeader %p: Inserting at end of list" BIP_MSG_PRE_ARG, (void *)hHeader ));
        BLST_Q_INSERT_TAIL( &hList->headerListHead, hHeader, headerListNext );
    }
    return( hHeader );

error:
    if (hHeader)
    {
        if (hHeader->hValue) {BIP_String_Destroy( hHeader->hValue ); }
        if (hHeader->hName)  {BIP_String_Destroy( hHeader->hName ); }

        BDBG_OBJECT_DESTROY( hHeader, BIP_HttpHeader );

        B_Os_Free( hHeader );
    }

    return( NULL );
}                                                          /* BIP_HttpHeader_Create_locked */

/*****************************************************************************
 *  Create a Header with a name and value, and insert it into a HeaderList.
 *****************************************************************************/
BIP_HttpHeaderHandle BIP_HttpHeaderList_CreateHeader(
    BIP_HttpHeaderListHandle hList,
    const char              *pName,
    const char              *pValue,
    BIP_HttpHeaderHandle     hAddBeforeThisHeader
    )
{
    BIP_Status           rc      = BIP_SUCCESS;
    BIP_HttpHeaderHandle hHeader = NULL;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    BDBG_ASSERT( pName );
    BDBG_ASSERT( hList );
    BDBG_OBJECT_ASSERT( hList, BIP_HttpHeaderList );

    B_Mutex_Lock( hList->hMutex );
    if (hList->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }
    if (hList->serializeState   != serializeState_eIdle)   {rc = BIP_ERR_SERIALIZE_IN_PROGRESS;   goto error; }

    hHeader = BIP_HttpHeader_Create_locked( hList, pName, pValue, hAddBeforeThisHeader );

error:
    B_Mutex_Unlock( hList->hMutex );

    return( hHeader );
}                                                          /* BIP_HttpHeaderList_CreateHeader */

/*****************************************************************************
 *  Clear (reinitialize) a HeaderList.
 *  Note: The HeaderList mutex must be locked before calling this function.
 *****************************************************************************/
static void BIP_HttpHeaderList_Clear_locked(
    BIP_HttpHeaderListHandle hList
    )
{
    BIP_HttpHeaderHandle hHeader;

    BDBG_OBJECT_ASSERT( hList, BIP_HttpHeaderList );
    B_MUTEX_ASSERT_LOCKED( hList->hMutex );

    /* Don't need to check for serialize or deserialize busy because
     * we're going to reset their states back to idle. */

    /* Destroy any BIP_HttpHeader objects that are in the list. */
    hHeader = BLST_Q_FIRST( &hList->headerListHead );
    while (hHeader != NULL) {
        BIP_HttpHeaderHandle hNext = BLST_Q_NEXT( hHeader, headerListNext );

        BDBG_MSG(( BIP_MSG_PRE_FMT "hList %p: Removing hHeader=%p" BIP_MSG_PRE_ARG, (void *)hList, (void *)hHeader ));
        BIP_HttpHeader_Destroy_locked( hHeader );
        hHeader = hNext;
    }

    hList->deserializeState = deserializeState_eIdle;
    hList->serializeState   = serializeState_eIdle;

    hList->byteIndex = 0;

    if (hList->hName) {BIP_String_Clear( hList->hName ); }
    if (hList->hValue) {BIP_String_Clear( hList->hValue ); }
    hList->hHeader = NULL;

    return;
}                                                          /* BIP_HttpHeaderList_Clear_locked */

/*****************************************************************************
 *  Clear (reinitialize) a HeaderList.
 *****************************************************************************/
void BIP_HttpHeaderList_Clear(
    BIP_HttpHeaderListHandle hList
    )
{
    BDBG_OBJECT_ASSERT( hList, BIP_HttpHeaderList );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hList %p: Entry..." BIP_MSG_PRE_ARG, (void *)hList ));

    B_Mutex_Lock( hList->hMutex );

    BIP_HttpHeaderList_Clear_locked( hList );

    B_Mutex_Unlock( hList->hMutex );
    return;
}

/*****************************************************************************
 *  Destroy a HeaderList, including any Headers that it contains.
 *****************************************************************************/
void BIP_HttpHeaderList_Destroy(
    BIP_HttpHeaderListHandle hList,
    void                    *ownerContext                  /*!< Same ownerContext used to Create the HeaderList. */
    )
{
    BDBG_OBJECT_ASSERT( hList, BIP_HttpHeaderList );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hList %p: Entry..." BIP_MSG_PRE_ARG, (void *)hList ));

    if (ownerContext != hList->ownerContext)
    {
        BDBG_WRN(( BIP_MSG_PRE_FMT "hList %p: Mismatched ownerContext: got %p, expected %p.  Ignoring Destroy request!"
                   BIP_MSG_PRE_ARG, (void *)hList, (void *)ownerContext, (void *)hList->ownerContext ));
    }

    BIP_HttpHeaderList_Clear( hList );                     /* Delete any Headers from the HeaderList. */

    batom_factory_destroy( hList->factory );

    if (hList->hName) {BIP_String_Destroy( hList->hName ); }
    if (hList->hValue) {BIP_String_Destroy( hList->hValue ); }

    B_Mutex_Destroy( hList->hMutex );

    BDBG_OBJECT_DESTROY( hList, BIP_HttpHeaderList );

    BDBG_MSG(( BIP_MSG_PRE_FMT "hList %p: Freeing object memory" BIP_MSG_PRE_ARG, (void *)hList ));
    B_Os_Free( hList );

    return;
}                                                          /* BIP_HttpHeaderList_Destroy */

/*****************************************************************************
 *  Create a new, empty HeaderList.
 *****************************************************************************/
BIP_HttpHeaderListHandle BIP_HttpHeaderList_Create(
    void *ownerContext                                     /*!< Any pointer, but the same pointer will be needed for destroying the HeaderList. */
    )
{
    int rc;
    BIP_HttpHeaderListHandle hList = NULL;

    BDBG_MSG(( BIP_MSG_PRE_FMT "Entry..." BIP_MSG_PRE_ARG ));

    /* Allocate memory for the object. */
    hList = B_Os_Calloc( 1, sizeof( *hList ));
    BIP_CHECK_GOTO(( hList ),            ( "Memory Allocation Failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BDBG_OBJECT_SET( hList, BIP_HttpHeaderList );
    BDBG_MSG(( BIP_MSG_PRE_FMT "hList %p: Allocated " BIP_MSG_PRE_ARG, (void *)hList ));

    hList->ownerContext = ownerContext;

    hList->hMutex = B_Mutex_Create( NULL );
    BIP_CHECK_GOTO(( hList->hMutex ),   ( "B_Mutex_Create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    BLST_Q_INIT( &hList->headerListHead );

    hList->hName = BIP_String_Create();
    BIP_CHECK_GOTO(( hList->hName ),    ( "BIP_String_Create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hList->hValue = BIP_String_Create();                   /* Works even if pValue is NULL. */
    BIP_CHECK_GOTO(( hList->hValue ),   ( "BIP_String_Create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    hList->factory = batom_factory_create( bkni_alloc, 3 ); /* 3 pre allocating atoms */
    BIP_CHECK_GOTO(( hList->factory ),   ( "batom_factory_create() failed" ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    return( hList );

error:
    if (hList)
    {
        if (hList->factory) {batom_factory_destroy( hList->factory ); }
        if (hList->hName)   {BIP_String_Destroy( hList->hName ); }
        if (hList->hValue)  {BIP_String_Destroy( hList->hValue ); }
        if (hList->hMutex)  {B_Mutex_Destroy( hList->hMutex ); }
        B_Os_Free( hList );
    }
    return( NULL );
}                                                          /* BIP_HttpHeaderList_Create */

/*****************************************************************************
 *  Some private (static) functions used by the deserializer.
 *****************************************************************************/
static bool BIP_HttpHeader_IsTChar(
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

static bool BIP_HttpHeader_IsOws(
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

static bool BIP_HttpHeader_IsFieldVchar(
    char ch
    )                                                      /* VCHAR: visible chars. */
{
    if (ch>=0x21 /* && uch<=0xFF */)
    {
        return( true );
    }

    return( false );
}

/*****************************************************************************
 *  API to deserialize a list of HTTP headers from a batom cursor into a
 *  BIP_HttpHeaderList.
 *****************************************************************************/
BIP_Status BIP_HttpHeaderList_DeserializeFromAtom(
    BIP_HttpHeaderListHandle hList,
    batom_cursor            *pCursor,                      /* [in]  Cursor that contains incoming data */
    bool                    *pDeserializeComplete          /* [out] Set to true if the http message is complete, ie it has <CRNL><CRNL or <NL><NL>. */
    )
{
    BIP_Status   rc = BIP_SUCCESS;
    char         ch;
    batom_cursor cursorSave;

    BDBG_ASSERT( hList );
    BDBG_OBJECT_ASSERT( hList, BIP_HttpHeaderList );
    BDBG_ASSERT( pCursor );
    BDBG_ASSERT( pDeserializeComplete );

    B_Mutex_Lock( hList->hMutex );

    /* Can't start deserializing if we're in the middle of serializing. */
    if (hList->serializeState != serializeState_eIdle) {rc = BIP_ERR_SERIALIZE_IN_PROGRESS; goto error; }

    *pDeserializeComplete = false;

    batom_cursor_clone( &cursorSave, pCursor );

    /* Loop through each byte in the cursor, until:
     *   1. The cursor is empty,
     *   2. Deserializing is complete,
     *   3. Deserializing fails.
     * */
    while (true) {
        bool relook;                                       /* Set this to true to make another pass through the state machine. */

        ch = batom_cursor_next( pCursor );
        if (batom_cursor_eof( pCursor )) {break; }

        /* Loop for the state machine.  Setting "relook=true" will run the run
         * the state machine again to process a new state.
         * */
        for (relook = true; relook==true; ) {
            relook = false;

            BDBG_MSG(( BIP_MSG_PRE_FMT "hList %p: deserializeState=0x%x \"%s\"  byteIndex=%zu"
                       BIP_MSG_PRE_ARG, (void *)hList, hList->deserializeState, toStr_deserializeState( hList->deserializeState ), hList->byteIndex ));

            switch (hList->deserializeState) {
                /* State: Idle
                 * -----------
                 * No deserialize in progress.  It's time to start a new deserialize session.
                 * Input (ch) is the first byte of a message's header section. */
                case deserializeState_eIdle:
                {
                    BIP_HttpHeaderList_Clear_locked( hList );

                    /* Create our accumulator strings. */
                    BIP_String_Clear( hList->hName );
                    BIP_String_Clear( hList->hValue );

                    /* Change to new state and process the input byte there. */
                    hList->deserializeState = deserializeState_eNextHeader;
                    relook = true;                         /* process this char in the new state. */
                    break;
                }

                /* State: NextHeader
                 * -------------------
                 * See if we're at the start of a header name, or at the end
                 * of the header list. */
                case deserializeState_eNextHeader:
                {
                    /* Change to new state and process the input byte there. */
                    if (ch == CR)
                    {
                        hList->deserializeState = deserializeState_eEndOfListCr;
                    }
                    else
                    {
                        hList->deserializeState = deserializeState_eHdrFieldName;
                        relook = true;                     /* process this char in the new state. */
                    }
                    break;
                }

                /* State: HdrFieldName
                 * -------------------
                 * Deserializing the header's Field Name (see "field-name" in RFC7230).  */
                case deserializeState_eHdrFieldName:
                {
                    /* If input belongs to the name, append it to the hName BIP_String.  */
                    if (BIP_HttpHeader_IsTChar( ch ))
                    {
                        BIP_String_StrcatCharN( hList->hName, &ch, 1 );
                    }
                    /* If ch is the expected terminator ":" and the hName is non-empty, move to next state  */
                    else if (ch == ':')                    /* Found end of "field-name". */
                    {
                        size_t length = BIP_String_GetLength( hList->hName );
                        BIP_CHECK_GOTO(( length>0 ), ( "Deserialize error: zero-length header name." ), error, BIP_ERR_INVALID_PARAMETER, rc );

                        BDBG_MSG(( BIP_MSG_PRE_FMT "hList %p: Got header name=%s" BIP_MSG_PRE_ARG, (void *)hList, BIP_String_GetString( hList->hName )));

                        hList->deserializeState = deserializeState_ePreFieldOws;
                    }
                    /* Anything else is invalid. */
                    else
                    {
                        BIP_CHECK_GOTO(( false ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) in header name.", ch, ch ), error, BIP_ERR_INVALID_PARAMETER, rc );
                    }
                    break;
                }

                /* State: PreFieldOws
                 * -------------------
                 * Deserializing any optional white space before the header's Field Name (see "pre-field-OWS in RFC7230). */
                case deserializeState_ePreFieldOws:
                {
                    /* If ch is still OWS, ignore it.  Else move to field-value state and handle it there. */
                    if (!BIP_HttpHeader_IsOws( ch ))
                    {
                        hList->deserializeState = deserializeState_eFieldValuePostOws;
                        relook = true;
                    }
                    break;
                }

                /* State: FieldValuePostOws
                 * -------------------------
                 * Deserializing any optional white space after the header's Field Value and optional
                 * trailing white space (see "field-value-post-OWS" in RFC7320). */
                case deserializeState_eFieldValuePostOws:
                {
                    /* Check for CR terminator. It may be end of header value, or start of obs-fold. */
                    if (ch == CR)
                    {
                        hList->deserializeState = deserializeState_eHeaderCr;
                    }
                    /* If a valid field-content char, append it to the hValue BIP_String. */
                    else if (BIP_HttpHeader_IsFieldVchar( ch ) || ( ch == HTAB ) || ( ch == SP ))
                    {
                        BIP_String_StrcatCharN( hList->hValue, &ch, 1 );
                    }
                    else
                    {
                        BIP_CHECK_GOTO(( false ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) in header value.", ch, ch ), error, BIP_ERR_INVALID_PARAMETER, rc );
                    }
                    break;
                }

                /* State: HeaderCr
                 * -------------------------
                 * Got a CR in the field-value-post-OWS.  Might be end of header or start of obs-fold...
                 * we can't tell yet.  */
                case deserializeState_eHeaderCr:
                {
                    /* Next char should be LF. */
                    if (ch == LF)
                    {
                        hList->deserializeState = deserializeState_eHeaderCrLf;
                    }
                    else
                    {
                        BIP_CHECK_GOTO(( false ), ( "Deserialize error: Invalid character (\"%c\"=0x%02x) in header terminator.", ch, ch ), error, BIP_ERR_INVALID_PARAMETER, rc );
                    }
                    break;
                }

                /* State: HeaderCrLf
                 * -------------------------
                 * Got a CRLF in the field-value-post-OWS.  Still might be end of header or start of obs-fold.
                 * The next byte will tell.  */
                case deserializeState_eHeaderCrLf:
                {
                    bool headerDone = false;

                    /* See if we are starting an obs-fold sequence. If so, condense the obs-fold
                     * into a single space. */
                    if (( ch == SP ) || ( ch == HTAB ))
                    {
                        BIP_String_StrcatCharN( hList->hValue, " ", 1 );
                        hList->deserializeState = deserializeState_eObsFoldSpaceTab;
                    }
                    else
                    {
                        headerDone              = true;
                        hList->deserializeState = deserializeState_eNextHeader;
                        relook = true;
                    }

                    if (headerDone)
                    {
                        const char *pStart = BIP_String_GetString( hList->hValue );
                        const char *pEnd   = pStart + BIP_String_GetLength( hList->hValue );
                        const char *pCh    = pEnd;
                        while (pCh > pStart) {
                            if (!BIP_HttpHeader_IsOws( *( pCh-1 ))) {break; }
                            pCh--;
                        }
                        if (pCh < pEnd)
                        {
                            BIP_String_Trim( hList->hValue, pCh, 0 ); /* Trim from pCh to end. */
                        }

                        BDBG_MSG(( BIP_MSG_PRE_FMT "hList %p: Adding header: name=\"%s\" value=\"%s\""
                                   BIP_MSG_PRE_ARG, (void *)hList, BIP_String_GetString( hList->hName ), BIP_String_GetString( hList->hValue )));
                        {
                            BIP_HttpHeaderHandle hMyHandle;
                            hMyHandle = BIP_HttpHeader_Create_locked( hList,
                                    BIP_String_GetString( hList->hName ),
                                    BIP_String_GetString( hList->hValue ),
                                    NULL );                /* Add to end of HeaderList. */
                            BIP_CHECK_GOTO(( hMyHandle ), ( "Can't create HttpHeader during deserialize." ), error, BIP_ERR_INTERNAL, rc );
                        }

                        rc = BIP_String_Clear( hList->hName );
                        rc = BIP_String_Clear( hList->hValue );
                    }
                    break;
                }

                /* State: ObsFoldSpaceTab
                 * -------------------------
                 * We're in the deprecated line-folding state.  We've already put a SP
                 * in the value, so just throw away any following folding chars. */
                case deserializeState_eObsFoldSpaceTab:
                {
                    /* See if we're done with the folding sequence.. */
                    if (( ch != SP ) && ( ch != HTAB ))
                    {
                        hList->deserializeState = deserializeState_eFieldValuePostOws;
                        relook = true;
                    }
                    break;
                }

                /* State: EndOfListCr
                 * -------------------------
                 * We've found a CR after the end of a header.  This must be the end of the header list.
                 * But we still need the following LF to be sure. */
                case deserializeState_eEndOfListCr:
                {
                    if (ch == LF)
                    {
                        hList->deserializeState = deserializeState_eDone;
                        *pDeserializeComplete   = true;
                    }
                    else
                    {
                        BIP_Atom_CursorDump( &cursorSave, "Cursor Dump" );

                        BIP_CHECK_GOTO(( false ), ( "Deserialize error: At offset 0x%zx: Invalid character (\"%c\"=0x%02x) in header list terminator.",
                                                    batom_cursor_pos( pCursor ) - batom_cursor_pos( &cursorSave ), ch, ch ),
                            error, BIP_ERR_INVALID_PARAMETER, rc );
                    }
                    break;
                }

                /* State: Done
                 * -------------------------
                 * Not much to do but stay here. */
                case deserializeState_eDone:
                {
                    BIP_CHECK_GOTO(( false ), ( "Deserialize error: Deserialize is done! Ignoring character (\"%c\"=0x%02x) in header list terminator.", ch, ch ), error, BIP_ERR_INVALID_PARAMETER, rc );
                    break;
                }

                /* State: Anything not handled.
                 * -------------------------
                 * This should never happen. */
                default:
                {
                    BIP_CHECK_GOTO(( false ), ( "hList %p: Unhandled state=%s (%d)", (void *)hList, toStr_deserializeState( hList->deserializeState ), hList->deserializeState ),
                        error, BIP_ERR_INTERNAL, rc );

                    break;
                }
            }                                              /* End switch/case. */
        }

        if (hList->deserializeState == deserializeState_eDone)
        {
            hList->deserializeState = deserializeState_eIdle;
            break;
        }
    }

error:
    if (rc != BIP_SUCCESS)
    {
        hList->deserializeState = deserializeState_eIdle;
    }

    B_Mutex_Unlock( hList->hMutex );
    return( rc );
}                                                          /* BIP_HttpHeaderList_DeserializeFromAtom */

/*****************************************************************************
 *  API to deserialize a list of HTTP headers from a buffer into a
 *  BIP_HttpHeaderList object.
 *
 *  Refer to:  BIP_HttpHeader_DeserializeFromAtom()
 *****************************************************************************/
BIP_Status BIP_HttpHeaderList_DeserializeFromBuffer(
    BIP_HttpHeaderListHandle hList,                        /*!< [in]  The HttpHeaderList to be populated with the deserialized Headers. */
    const char              *buffer,                       /*!< [in]  Buffer that contains incoming serial data. */
    size_t                   bufferSize,                   /*!< [in]  Input buffer size. */
    bool                    *pDeserializeComplete,         /*!< [out] Set to true if the http message is complete, ie it has end of header(<CRNL><CRNL or <NL><NL>). */
    size_t                  *pConsumedBytes                /*!< [out] If request is complete then this specifies the consumed data size. If message is not complete then this will equal the bufferSize. */
    )
{
    BIP_Status   rc     = BIP_SUCCESS;
    batom_t      myAtom = NULL;
    batom_cursor myCursor;

    BDBG_ASSERT( hList );
    BDBG_OBJECT_ASSERT( hList, BIP_HttpHeaderList );
    BDBG_ASSERT( buffer );
    BDBG_ASSERT( pDeserializeComplete );

    if (bufferSize == 0) {return( rc ); }                  /* Nothing to deserialize. */

    /*************************************************************************
     *  Create an atom from the buffer.
     *************************************************************************/
    myAtom = batom_from_range( hList->factory, buffer, bufferSize, NULL, NULL );
    batom_cursor_from_atom( &myCursor, myAtom );

    rc = BIP_HttpHeaderList_DeserializeFromAtom( hList, &myCursor, pDeserializeComplete );

    /* Tell the caller how many bytes we've consumed here! */
    *pConsumedBytes = batom_cursor_pos( &myCursor );

    batom_release( myAtom );
    return( rc );
}                                                          /* BIP_HttpHeaderList_DeserializeFromBuffer */

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
static BIP_Status BIP_HttpHeader_SerializeChars(
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
    if (bytesToMove > 0)
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
}                                                          /* BIP_HttpHeader_SerializeChars */

/*****************************************************************************
 *  API to serialize a BIP_HttpHeaderList of HttpHeaders to a buffer
 *  specified by the caller.
 *****************************************************************************/
static BIP_Status BIP_HttpHeader_SerializeToBuffer_locked(
    BIP_HttpHeaderListHandle hList,                        /* In: handle of HeaderList to be serialized. */
    char                    *pBuffer,                      /* In: Buffer to receive the serialized output. */
    size_t                   bufferSize,                   /* In: Buffer size. */
    bool                    *pSerializeComplete,           /* Out: Set to true if serializing is complete. Otherwise, the buffer if full, call this again with a new buffer.  */
    size_t                  *pSerializedBytes              /* Out: The number of bytes that have been placed in the buffer. */
    )
{
    BIP_Status rc        = BIP_SUCCESS;
    size_t     destIndex = 0;
    bool       relook;                                     /* Set this to true to make another pass through the state machine. */

    BDBG_ASSERT( hList );
    BDBG_OBJECT_ASSERT( hList, BIP_HttpHeaderList );

    /* BIP_HttpHeader_GetSerializeBufferSize() calls this function with
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

    B_MUTEX_ASSERT_LOCKED( hList->hMutex );

    /* Can't start serializing if we're in the middle of deserializing. */
    if (hList->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }

    if (pSerializeComplete) {*pSerializeComplete = false; } /* Assume that we won't finish the serialize in one call. */

    /*************************************************************************
     * Loop for the state machine.  Setting "relook=true" will run the run
     * the state machine again to process a new state.
     *************************************************************************/
    for (relook = true; relook==true; ) {
        relook = false;

        BDBG_MSG(( BIP_MSG_PRE_FMT "hList %p: serializeState=0x%x \"%s\"  byteIndex=%zu"
                   BIP_MSG_PRE_ARG, (void *)hList, hList->serializeState, toStr_serializeState( hList->serializeState ), hList->byteIndex ));

        switch (hList->serializeState) {
            /* State: Idle
             * -----------
             * No serialize in progress.  It's time to start a new serializing session.
             * Initialize the serialize state. */
            case serializeState_eIdle:
            {
                hList->hHeader = BLST_Q_FIRST( &hList->headerListHead );
                if (hList->hHeader == NULL)                /* No headers to serialize. */
                {
                    hList->serializeState = serializeState_eListTerm;
                    hList->byteIndex      = 0;
                    relook                = true;
                }
                else
                {
                    hList->serializeState = serializeState_eHdrName;
                    hList->byteIndex      = 0;
                    relook                = true;
                }
                break;
            }

            /* State: NextHdr
             * --------------
             * Look for the next header,  */
            case serializeState_eNextHdr:
            {
                /* coverity[var_deref_op: FALSE] */
                hList->hHeader = BLST_Q_NEXT( hList->hHeader, headerListNext );
                if (hList->hHeader == NULL)
                {
                    hList->serializeState = serializeState_eListTerm;
                    hList->byteIndex      = 0;
                    relook                = true;
                }
                else
                {
                    hList->serializeState = serializeState_eHdrName;
                    hList->byteIndex      = 0;
                    relook                = true;
                }
                break;
            }

            /* State: HdrName
             * --------------
             * Serializing the current Header's name. */
            case serializeState_eHdrName:
            {
                /* coverity[var_deref_op: FALSE] */
                const char *pChars = BIP_String_GetString( hList->hHeader->hName );
                size_t      nChars = BIP_String_GetLength( hList->hHeader->hName );

                rc = BIP_HttpHeader_SerializeChars( pChars, nChars, &hList->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hList->serializeState = serializeState_eSeparator;
                    relook                = true;
                }
                break;
            }

            /* State: Separator
             * ----------------
             * Serializing the separator between name and value (": ")*/
            case serializeState_eSeparator:
            {
                const char pChars[] = {':', ' '};
                size_t     nChars   = sizeof( pChars );

                rc = BIP_HttpHeader_SerializeChars( pChars, nChars, &hList->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hList->serializeState = serializeState_eHdrValue;
                    relook                = true;
                }
                break;
            }

            /* State: HdrValue
             * -----------------
             * Serializing the current Header's value. */
            case serializeState_eHdrValue:
            {
                /* coverity[var_deref_op: FALSE] */
                const char *pChars = BIP_String_GetString( hList->hHeader->hValue );
                size_t      nChars = BIP_String_GetLength( hList->hHeader->hValue );

                rc = BIP_HttpHeader_SerializeChars( pChars, nChars, &hList->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hList->serializeState = serializeState_eHdrTerm;
                    relook                = true;
                }
                break;
            }

            /* State: HdrTerm
             * --------------
             * Serializing the terminating CRLF at the end of each header. */
            case serializeState_eHdrTerm:
            {
                const char pChars[] = {CR, LF};
                size_t     nChars   = sizeof( pChars );

                rc = BIP_HttpHeader_SerializeChars( pChars, nChars, &hList->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hList->serializeState = serializeState_eNextHdr;
                    relook                = true;
                }
                break;
            }

            /* State: ListTerm
             * ---------------
             * Serializing the current Header's name. */
            case serializeState_eListTerm:
            {
                const char pChars[] = {CR, LF};
                size_t     nChars   = sizeof( pChars );

                rc = BIP_HttpHeader_SerializeChars( pChars, nChars, &hList->byteIndex, pBuffer, bufferSize, &destIndex );
                if (rc == BIP_SUCCESS)
                {
                    hList->serializeState = serializeState_eDone;
                    relook                = true;
                }
                break;
            }

            /* State: Done
             * ------------
             * Serializing is complete. */
            case serializeState_eDone:
            {
                if (pSerializeComplete) {*pSerializeComplete = true; }
                hList->serializeState = serializeState_eIdle;
                break;
            }

            /* State: Anything not handled.
             * ----------------------------
             * This should never happen. */
            default:
            {
                BIP_CHECK_GOTO(( false ), ( "hList %p: Unhandled state=%s (%d)", (void *)hList, toStr_serializeState( hList->serializeState ), hList->serializeState ),
                    error, BIP_ERR_INTERNAL, rc );

                break;
            }
        }                                                  /* End switch/case. */
    }                                                      /* End Do while relook is true. */

    if (pSerializedBytes) {*pSerializedBytes = destIndex; }

    BDBG_MSG(( BIP_MSG_PRE_FMT "hList %p: returning *pSerializeComplete=%s *pSerializedBytes=%zu"
               BIP_MSG_PRE_ARG, (void *)hList,
               ( pSerializeComplete==NULL ) ? "N/A" : ( *pSerializeComplete ) ? "TRUE" : "FALSE",
               pSerializedBytes ? *pSerializedBytes : 0 ));

    return( BIP_SUCCESS );

error:
    if (pSerializedBytes) {*pSerializedBytes = destIndex; }

    hList->serializeState = serializeState_eIdle;

    BDBG_MSG(( BIP_MSG_PRE_FMT "hList %p: Error: returning *pRenderComplete=%s *pSerializedBytes=%zu"
               BIP_MSG_PRE_ARG, (void *)hList,
               ( pSerializeComplete==NULL ) ? "N/A" : ( *pSerializeComplete ) ? "TRUE" : "FALSE",
               pSerializedBytes ? *pSerializedBytes : 0 ));

    return( rc );
}                                                          /* BIP_HttpHeader_SerializeToBuffer_locked */

/*****************************************************************************
 *  API to serialize a BIP_HttpHeaderList of HttpHeaders to a buffer specified by
 *  the caller.
 *****************************************************************************/
BIP_Status BIP_HttpHeaderList_SerializeToBuffer(
    BIP_HttpHeaderListHandle hList,                        /* In: handle of HeaderList to be serialized. */
    char                    *pBuffer,                      /* In: Buffer to receive the serialized output. */
    size_t                   bufferSize,                   /* In: Buffer size. */
    bool                    *pSerializeComplete,           /* Out: Set to true if serializing is complete. Otherwise, the buffer if full, call this again with a new buffer.  */
    size_t                  *pSerializedBytes              /* Out: The number of bytes that have been placed in the buffer. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    BDBG_ASSERT( hList );
    BDBG_OBJECT_ASSERT( hList, BIP_HttpHeaderList );

    B_Mutex_Lock( hList->hMutex );

    rc = BIP_HttpHeader_SerializeToBuffer_locked( hList, pBuffer, bufferSize, pSerializeComplete, pSerializedBytes );

    B_Mutex_Unlock( hList->hMutex );

    return( rc );
}

/*****************************************************************************
 *  API to determine the number of bytes required to completely serialize a
 *  specified BIP_HttpHeaderList of HttpHeaders.
 *****************************************************************************/
BIP_Status BIP_HttpHeaderList_GetSerializeBufferSize(
    BIP_HttpHeaderListHandle hList,
    size_t                  *pSerializeBufferSize
    )
{
    BIP_Status rc;

    BDBG_ASSERT( hList );
    BDBG_OBJECT_ASSERT( hList, BIP_HttpHeaderList );
    BDBG_ASSERT( pSerializeBufferSize );

    rc = BIP_HttpHeaderList_SerializeToBuffer( hList, NULL, 0, NULL, pSerializeBufferSize );

    return( rc );
}

/*****************************************************************************
 *  API to serialize a BIP_HttpHeaderList of HttpHeaders to an atom.
 *****************************************************************************/
BIP_Status BIP_HttpHeaderList_SerializeToAtom(
    BIP_HttpHeaderListHandle hList,                        /* In: handle of HeaderList to be serialized. */
    batom_factory_t          factory,
    batom_t                 *pAtom                         /* Out: Where to put the atom containing the serialized data. */
    )
{
    BIP_Status rc = BIP_SUCCESS;

    size_t  serializeLen      = 0;
    batom_t myAtom            = NULL;
    char   *pBuffer           = NULL;
    bool    serializeComplete = false;
    size_t  serializedBytes;

    BDBG_ASSERT( hList );
    BDBG_OBJECT_ASSERT( hList, BIP_HttpHeaderList );
    BDBG_ASSERT( pAtom );

    B_Mutex_Lock( hList->hMutex );

    /* Can't start serializing if we're in the middle of deserializing. */
    if (hList->deserializeState != deserializeState_eIdle) {rc = BIP_ERR_DESERIALIZE_IN_PROGRESS; goto error; }

    /*************************************************************************
     *  Start by counting up the sizes of all the headers so we know how
     *  much memory to malloc.
     *************************************************************************/
    rc = BIP_HttpHeader_SerializeToBuffer_locked( hList, NULL, 0, NULL, &serializeLen );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpHeader_SerializeToBuffer_locked failed." ), error, rc, rc );

    /*************************************************************************
     *  Malloc some memory, then create an atom out of it.
     *************************************************************************/
    pBuffer = B_Os_Malloc( serializeLen );
    BIP_CHECK_GOTO(( pBuffer ), ( "B_Os_Malloc (%zu bytes) failed.", serializeLen ), error, BIP_ERR_OUT_OF_SYSTEM_MEMORY, rc );

    myAtom = BIP_Atom_AtomFromBOsMallocRange( factory, pBuffer, serializeLen );

    /*************************************************************************
     *  Now serialize into the atom.
     *************************************************************************/
    rc = BIP_HttpHeader_SerializeToBuffer_locked( hList, pBuffer, serializeLen, &serializeComplete, &serializedBytes );
    BIP_CHECK_GOTO(( rc==BIP_SUCCESS ), ( "BIP_HttpHeader_SerializeToBuffer failed." ), error, rc, rc );

    BDBG_ASSERT( serializeComplete );                      /* The buffer was big enough, serialize should be complete. */
    BDBG_ASSERT( serializedBytes == serializeLen );        /* We should have filled the whole buffer. */

    *pAtom = myAtom;
    B_Mutex_Unlock( hList->hMutex );
    return( BIP_SUCCESS );

error:

    if (myAtom) {batom_release( myAtom ); }                /* This will free the buffer as well. */
    else if (pBuffer)
    {
        B_Os_Free( pBuffer );
    }

    B_Mutex_Unlock( hList->hMutex );

    return( rc );
}                                                          /* BIP_HttpHeaderList_SerializeToAtom */
