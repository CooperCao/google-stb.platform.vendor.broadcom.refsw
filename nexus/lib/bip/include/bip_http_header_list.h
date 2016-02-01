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

#ifndef BIP_HTTP_HEADER_H
#define BIP_HTTP_HEADER_H

#include "bip.h"
#include "bioatom.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup bip_http_header_list
 *
 * BIP_HttpHeader Interface Definition.
 *
 * This class is responsible for creating, manipulating, and accessing HTTP Headers
 * that are used in HTTP Request and Response messages.
 *
 * These BIP_HttpHeader objects are contained within a BIP_HttpHeaderList, which
 * is a simple one-dimensional ordered list.  The BIP_HttpHeader APIs provide for
 * manipulating headers within a HeaderList.
 */

/**
Summary:
Datatype to hold a reference to a BIP_HttpHeader object.
**/

typedef struct BIP_HttpHeaderList *BIP_HttpHeaderListHandle; /*!< HttpHeaderList object */
typedef struct BIP_HttpHeader     *BIP_HttpHeaderHandle;     /*!< HttpHeader object */

/**
Summary:
Create an empty HttpHeaderList

Description:
Since HttpHeaders must always belong to a HttpHeaderList, an HttpHeaderList must
be created before an HttpHeader can be created.

Return:
    non-NULL :        The handle of the new HttpHeaderList to be used for calling subsequent HttpHeader related APIs
Return:
    NULL     :        Failure, an HttpHeaderList instance could not be created.

See Also:
BIP_HttpHeaderList_Destroy
**/
BIP_HttpHeaderListHandle BIP_HttpHeaderList_Create(
    void *ownerContext                                     /*!< Any pointer, but the same pointer will be needed for destroying the HeaderList. */
    );

/**
Summary:
Destroy an HttpHeaderList

Description:
Destroys the specified HttpHeaderList after removing and destroying all HttpHeaders that it contains.

Return:
    void :

See Also:
BIP_HttpHeaderList_Create
**/
void BIP_HttpHeaderList_Destroy(
    BIP_HttpHeaderListHandle hHeaderList,                  /*!< Handle of the HeaderList to be destroyed. */
    void                    *ownerContext                  /*!< Same ownerContext used to Create the HeaderList. */
    );

/**
Summary:
Clear an HttpHeaderList

Description:
Removes all headers from an HttpHeaderList and returns it to its initial state.

Return:
    BIP_SUCCESS           : The handle of the first HttpHeader has been placed in the hHeader variable.

See Also:
BIP_HttpHeaderList_Create
**/
void BIP_HttpHeaderList_Clear(
    BIP_HttpHeaderListHandle hHeaderList );                /*!< Handle of the HeaderList to be cleared. */

/**
Summary:
Create an HttpHeader

Description:
Since HttpHeaders must always belong to a HttpHeaderList, a HttpHeaderListHandle
must be specified during Header creation.

Return:
    non-NULL :        The handle of the new HttpHeader to be used for calling subsequent HttpHeader related APIs
Return:
    NULL     :        Failure, an HttpHeader instance could not be created.

See Also:
BIP_HttpHeader_Destroy
**/
BIP_HttpHeaderHandle BIP_HttpHeaderList_CreateHeader(
    BIP_HttpHeaderListHandle hHeaderList,                  /*!< [in]  Handle of the HeaderList that will hold the header. */
    const char              *pName,                        /*!< [in]  Pointer to a null-terminated string with the header's name. */
    const char              *pValue,                       /*!< [in]  Optional pointer to a null-terminated string will be copied to the header's value. */
                                                           /*!<       If NULL, then the header will be created without a value. */
    BIP_HttpHeaderHandle     hAddBeforeThisHeader          /*!< [in]  If a BIP_HeaderHandle is passed, and it exists in the specified headerList, */
                                                           /*!<       the newly created header will be positioned immediately in front of it. */
                                                           /*!<       If NULL, the new header will be placed at the end of the HeaderList. */
    );

/**
Summary:
Destroy a BIP_HttpHeader

Description:
Destroys the HttpHeader after removing it from its HttpHeaderList.

Return:
    void :

See Also:
BIP_HttpHeader_Create
**/
BIP_Status BIP_HttpHeaderList_DestroyHeader(
    BIP_HttpHeaderHandle hHttpHeader                       /*!< Handle of the Header to be destroyed. */
    );

/**
Summary:
Get the first or next HttpHeader following a given Header in the specified
HttpHeaderList.  Optionally, a header name can be specified to return the
next header with that (case-insensitive) name.

Description:
Starting from a given HttpHeader, or from the start of the HttpHeaderList,
return the handle of the next Header, optionally, with a specified name.

RFC7230 Sec 3.2 states that header names are case-insensitive.

Return:
    BIP_SUCCESS           : The handle of the next HttpHeader has been placed in the hNextHeader variable.
Return:
    BIP_INF_NOT_AVAILABLE : There are no Headers with the specified name following hCurrentHeader.
                            The variable referenced by phNextHeader has been set to NULL.
**/
BIP_Status BIP_HttpHeaderList_GetNextHeader(
    BIP_HttpHeaderListHandle hHeaderList,                  /*!< [in]  Handle of the HeaderList to be accessed. */
    BIP_HttpHeaderHandle     hCurrentHeader,               /*!< [in]  The header from which to start. Pass NULL to start from the beginning of the list. */
    const char              *pName,                        /*!< [in]  If non-NULL, then return the next Header with a matching name. */
    BIP_HttpHeaderHandle    *phNextHeader,                 /*!< [out] Optional: Caller's variable to receive the handle of the next header. */
                                                           /*!<       This can also be the address of the hCurrentHeader variable. */
                                                           /*!<       Pass NULL if not interested in the header's handle. */
    const char             **ppNextHeaderValue             /*!< [out] Optional address of pointer that will be point to the header's value. */
                                                           /*!<       Pass NULL if not interested in the header's value. */
    );

/**
Summary:
Reposition a Header to be in front of some other Header.

Description:
Moves a header to a different position in the HeaderList, by placing it in front of another
specified header, or at the end of the list.

Return:
    BIP_SUCCESS           : The Header was moved as requested.
**/
BIP_Status BIP_HttpHeaderList_MoveHeader(
    BIP_HttpHeaderHandle hHeader,                          /*!< [in]  The header to be moved. */
    BIP_HttpHeaderHandle hPutBeforeThisHeader              /*!< [in]  The Header that hHeader will moved in front of. */
    );                                                     /*!<       If NULL, then hHeader will be placed at the end of the HeaderList. */

/**
Summary:
Get a pointer to a Header's name.

Description:
Returns a pointer the header name associated with a HeaderHandle.

Return:
    BIP_SUCCESS           : The address of the Header's name was returned.

See Also:
BIP_HttpHeader_GetValue
BIP_HttpHeader_SetValue
**/
BIP_Status BIP_HttpHeaderList_GetHeaderName(
    BIP_HttpHeaderHandle hHeader,                          /*!< [in]  The header of interest. */
    const char         **pName                             /*!< [out] The address of a pointer that will be updated to point the Header's name as a null-terminated string. */
    );                                                     /*!<       This pointer will become invalid when it's Header is destroyed. */

/**
Summary:
Get a pointer to a Header's value.

Description:
Returns a pointer the header value associated with a HeaderHandle.

Return:
    BIP_SUCCESS           : The address of the Header's value string was returned.

See Also:
BIP_HttpHeader_SetValue
**/
BIP_Status BIP_HttpHeaderList_GetHeaderValue(
    BIP_HttpHeaderHandle hHeader,                          /*!< [in]  The header of interest. */
    const char         **pValue                            /*!< [out] The address of a pointer that will be updated to point the Header's value as a null-terminated string. */
    );                                                     /*!<       This pointer will become invalid when it's Header is destroyed. */

/**
Summary:
Set a Header's value.

Description:
Replaces a Header's value with a copy of a specified character string.

Return:
    BIP_SUCCESS       : The Header's value string has been modified as requested.

See Also:
BIP_HttpHeader_GetValue
**/
BIP_Status BIP_HttpHeaderList_SetHeaderValue(
    BIP_HttpHeaderHandle hHeader,                          /*!< [in]  The header that will have its value changed. */
    const char          *pValue                            /*!< [in]  A pointer to a null-terminated string containing the Header's new value. */
                                                           /*!<       The new Header value does not include the null-terminator. */
                                                           /*!<       Passing NULL will result in a header without a value.  */
    );

BIP_Status BIP_HttpHeaderList_DeserializeFromAtom(
    BIP_HttpHeaderListHandle hList,
    batom_cursor            *cursor,                       /*!< [in/out] Cursor that contains incoming data */
    bool                    *pParseComplete );             /*!< [in]  Set to true if the http message is complete, ie it has <CRNL><CRNL or <NL><NL>. */

BIP_Status BIP_HttpHeaderList_DeserializeFromBuffer(
    BIP_HttpHeaderListHandle hList,                        /*!< [in]  The HttpHeaderList to be populated with the deserialized Headers. */
    const char              *buffer,                       /*!< [in]  Buffer that contains incoming serial data. */
    size_t                   bufferSize,                   /*!< [in]  Input buffer size. */
    bool                    *pParseComplete,               /*!< [out] Set to true if the http message is complete, ie it has end of header(<CRNL><CRNL or <NL><NL>). */
    size_t                  *pConsumedBytes );             /*!< [out] If request is complete then this specifies the consumed data size.If message is not complete then this must be equal to bufferSize. */

BIP_Status BIP_HttpHeaderList_GetSerializeBufferSize(
    BIP_HttpHeaderListHandle hList,                        /*!< [in]  Handle of HeaderList to be serialized. */
    size_t                  *pSerializeBufferSize          /*!< [out] Address of variable to receive the number of bytes required for serializing the HeaderList. */
    );

BIP_Status BIP_HttpHeaderList_SerializeToBuffer(
    BIP_HttpHeaderListHandle hList,                        /*!< [in]  Handle of HeaderList to be serialized. */
    char                    *pBuffer,                      /*!< [in]  Buffer to receive the serialized output. */
    size_t                   bufferSize,                   /*!< [in]  Buffer size. */
    bool                    *pSerializeComplete,           /*!< [out] Set to true if serialization is complete. Otherwise, the buffer is full, call this again with a new buffer.  */
    size_t                  *pDeserializedBytes            /*!< [out] The number of bytes that have been placed in the buffer. */
    );

BIP_Status BIP_HttpHeaderList_SerializeToAtom(
    BIP_HttpHeaderListHandle hList,                        /*!< [in]  Handle of HeaderList to be serialized. */
    batom_factory_t          factory,                      /*!< [in]  Factory to use for creating atom .   */
    batom_t                 *atom );                       /*!< [out] Where to put the atom containing the serialized data. */

#ifdef __cplusplus
}
#endif

#endif /* BIP_HTTP_HEADER_H */
