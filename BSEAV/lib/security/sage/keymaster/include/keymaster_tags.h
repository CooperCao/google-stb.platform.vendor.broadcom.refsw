/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#ifndef KEYMASTER_TAGS_H__
#define KEYMASTER_TAGS_H__


#include "bstd.h"
#include "keymaster_types.h"


#ifdef __cplusplus
extern "C"
{
#endif


typedef struct KM_Tag_Context *KM_Tag_ContextHandle;



/***************************************************************************
Summary:
Create a context based on the input block for processing parameters

Returns:
Context handle, which must be deleted. Returns error or a new context handle.

See Also:
KM_Tag_DeleteContext()
***************************************************************************/
BERR_Code KM_Tag_CreateContext(int32_t num_params, uint32_t len, uint8_t *data_block, KM_Tag_ContextHandle *handle);

#define KM_Tag_NewContext(handle) KM_Tag_CreateContext(0, 0, NULL, handle);

/***************************************************************************
Summary:
Create a context based on a tag value set block

Returns:
Context handle, which must be deleted. Returns error or a new context handle.

See Also:
KM_Tag_DeleteContext()
***************************************************************************/
BERR_Code KM_Tag_CreateContextFromTagValueSet(uint8_t *data_block, KM_Tag_ContextHandle *handle);

/***************************************************************************
Summary:
Duplicate the whole tag context

Returns:
A new context with the same values as the original

See Also:
KM_Tag_DeleteContext()
***************************************************************************/
KM_Tag_ContextHandle KM_Tag_DupContext(KM_Tag_ContextHandle handle);

/***************************************************************************
Summary:
Delete context handle

See Also:
KM_Tag_CreateContext()
***************************************************************************/
void KM_Tag_DeleteContext(KM_Tag_ContextHandle handle);

/***************************************************************************
Summary:
Return the number of tag/value pairs in the context

See Also:
***************************************************************************/
int32_t KM_Tag_GetNumPairs(KM_Tag_ContextHandle handle);

/***************************************************************************
Summary:
Check that a parameter set is consistent. That there are no tags, which are
expected to be unique, but are repeated.

Returns:
BERR_INVALID_PARAMETER if tag is repeated when it should not be

See Also:
KM_Tag_CreateContext()
***************************************************************************/
BERR_Code KM_Tag_CheckParameterConsitency(KM_Tag_ContextHandle handle);

/***************************************************************************
Summary:
Validate the tags associated with with a key blob. There is a subset of tags
allowed, and this function will check that the tag context complies. Intended
for checking parameters when generating and importing keys, not decrypted key
blobs.

Returns:
BERR_INVALID_PARAMETER if tag is wrong or parameter passed in is incorrect
BSAGE_ERR_KM_UNSUPPORTED_TAG tag not found in list of supported tags

See Also:
KM_Tag_CreateContext()
***************************************************************************/
BERR_Code KM_Tag_ValidateKeyParameters(KM_Tag_ContextHandle handle);

/***************************************************************************
Summary:
Returns first item in the context (updates the internal current pointer)

Returns:
km_tag_value_t* - returns NULL empty

See Also:
KM_Tag_Next()
***************************************************************************/
km_tag_value_t* KM_Tag_First(KM_Tag_ContextHandle handle);

/***************************************************************************
Summary:
Return the next item in the context (updates the internal current pointer)

Returns:
km_tag_value_t* - returns NULL if no more tags

See Also:
KM_Tag_First()
***************************************************************************/
km_tag_value_t* KM_Tag_Next(KM_Tag_ContextHandle handle);

/***************************************************************************
Summary:
Find tag starting from the beginning of the list.

Returns:
km_tag_value_t* - returns NULL if not found or returns value

See Also:
KM_Tag_FindNext()
***************************************************************************/
km_tag_value_t* KM_Tag_FindFirst(KM_Tag_ContextHandle handle, km_tag_t tag);

/***************************************************************************
Summary:
Find next tag - only applicable to XXX_REP type tags. Must have called
KM_Tag_Find first.

Returns:
km_tag_value_t* - returns NULL if no more found or not a _REP type used in
previous KM_Tag_Find().

See Also:
KM_Tag_Find()
***************************************************************************/
km_tag_value_t* KM_Tag_FindNext(KM_Tag_ContextHandle handle);

/***************************************************************************
Summary:
Find Nth tag start at the beginning - only applicable to _REP type tags if n > 0.
Will not reset the state of an ongoing KM_Tag_Find/KM_Tag_FindNext.

Returns:
km_tag_value_t* - returns NULL if Nth tag does not exist

See Also:
KM_Tag_Find()
KM_Tag_FindNext()
***************************************************************************/
km_tag_value_t* KM_Tag_FindNth(KM_Tag_ContextHandle handle, km_tag_t tag, uint32_t n);

/***************************************************************************
Summary:
Returns true if the tag/value pair is found. Does not update the current
pointer, like find and find next.

See Also:
KM_Tag_Find()
***************************************************************************/
bool KM_Tag_ContainsEnum(KM_Tag_ContextHandle handle, km_tag_t tag, uint32_t enumerated);

/***************************************************************************
Summary:
Same as above except it returns the tag.

See Also:
KM_Tag_Find()
***************************************************************************/
km_tag_value_t* KM_Tag_FindEnum(KM_Tag_ContextHandle handle, km_tag_t tag, uint32_t enumerated);

/***************************************************************************
Summary:
Returns true if the tag/value pair is found. Does not update the current
pointer, like find and find next.

See Also:
KM_Tag_Find()
***************************************************************************/
bool KM_Tag_ContainsInteger(KM_Tag_ContextHandle handle, km_tag_t tag, uint32_t integer);

/***************************************************************************
Summary:
Returns true if the tag/value pair is found. Does not update the current
pointer, like find and find next.

See Also:
KM_Tag_Find()
***************************************************************************/
bool KM_Tag_ContainsLongInteger(KM_Tag_ContextHandle handle, km_tag_t tag, uint64_t long_integer);

/***************************************************************************
Summary:
Find the number of occurrences of a tag.

See Also:
***************************************************************************/
uint32_t KM_Tag_Count(KM_Tag_ContextHandle handle, km_tag_t tag);

/***************************************************************************
Summary:
Add an tag/value pair to a context. This makes copy of the data, not a
reference to the parameter provided.

See Also:
***************************************************************************/
BERR_Code KM_Tag_Add(KM_Tag_ContextHandle handle, km_tag_value_t *tag_value_pair);

/***************************************************************************
Summary:
Add an enum to a context. This makes a new entry in the tag context.

See Also:
***************************************************************************/
BERR_Code KM_Tag_AddEnum(KM_Tag_ContextHandle handle, const km_tag_t tag, const uint32_t enum_value);

/***************************************************************************
Summary:
Add an integer to a context. This makes a new entry in the tag context.

See Also:
***************************************************************************/
BERR_Code KM_Tag_AddInteger(KM_Tag_ContextHandle handle, const km_tag_t tag, const uint32_t integer_value);

/***************************************************************************
Summary:
Add a long integer to a context. This makes a new entry in the tag context.

See Also:
***************************************************************************/
BERR_Code KM_Tag_AddLongInteger(KM_Tag_ContextHandle handle, const km_tag_t tag, const uint64_t long_int_value);

/***************************************************************************
Summary:
Add a date to a context. This makes a new entry in the tag context.

See Also:
***************************************************************************/
BERR_Code KM_Tag_AddDate(KM_Tag_ContextHandle handle, const km_tag_t tag, const uint64_t date_value);

/***************************************************************************
Summary:
Add a bool to a context. This makes a new entry in the tag context.

See Also:
***************************************************************************/
BERR_Code KM_Tag_AddBool(KM_Tag_ContextHandle handle, const km_tag_t tag, const bool bool_value);

/***************************************************************************
Summary:
Add an enum to a context. This makes a new entry in the tag context.

See Also:
***************************************************************************/
BERR_Code KM_Tag_AddBlob(KM_Tag_ContextHandle handle, const km_tag_t tag, const uint32_t blob_len, const uint8_t *blob_data);

/***************************************************************************
Summary:
Remove a tag from the context. This works based on the result of
KM_Tag_FindFirst, KM_Tag_FindNext or KM_Tag_FindNth. NOT KM_Tag_Dup. Note
that this does not free the tag_value_pair. It will have to be freed
by the caller by calling KM_Tag_Free.

See Also:
KM_Tag_FindFirst()
KM_Tag_FindNext()
KM_Tag_FindNth()
KM_Tag_Free()
***************************************************************************/
BERR_Code KM_Tag_Remove(KM_Tag_ContextHandle handle, km_tag_value_t *tag_value_pair);

/***************************************************************************
Summary:
Free a tag. Note that the caller MUST call KM_Tag_Remove first, to remove
it from the context.

See Also:
KM_Tag_Remove()
KM_Tag_FindFirst()
KM_Tag_FindNext()
KM_Tag_FindNth()
KM_Tag_First()
KM_Tag_Next()
***************************************************************************/
void KM_Tag_Free(km_tag_value_t *tag_value_pair);

/***************************************************************************
Summary:
Serialize a set of data in a context to allow it to be stored offline. Storage
is compatible with loading data with a call to KM_Tag_CreateContext. If buffer
is NULL, buffer_size will be calculated and returned by the function. If
buffer is non-NULL, buffer_size must reflect the size of the
buffer passed in. The buffer must be >= the required size.

See Also:
KM_Tag_SerializeAsTagValueSet()
***************************************************************************/
BERR_Code KM_Tag_Serialize(KM_Tag_ContextHandle handle, uint8_t *buffer, uint32_t *buffer_size);

/***************************************************************************
Summary:
Serialize a set of data as a tag value set (km_tag_value_set_t). It's the
same as KM_Tag_Serialize with a prepended num and size. As with
KM_Tag_Serialize, if buffer is NULL, the function will return the buffer
space required to store the serialized data.

See Also:
KM_Tag_Serialize()
***************************************************************************/
BERR_Code KM_Tag_SerializeAsTagValueSet(KM_Tag_ContextHandle handle, uint8_t *buffer, uint32_t *buffer_size);

/***************************************************************************
Summary:
Compare two context tags. It will return 0 if they are identical otherwise it
will return -1.

See Also:
***************************************************************************/
int KM_Tag_ContextCmp(KM_Tag_ContextHandle handle, KM_Tag_ContextHandle cmp_handle);
/***************************************************************************
Summary:
Duplicate the tag, returning a new pointer. This new tag is not stored in
the context handle, so must be freed with a call to NEXUS_Memory_Free()

See Also:
***************************************************************************/
km_tag_value_t* KM_Tag_Dup(km_tag_value_t *tagValuePair);



/* inline helpers */
static inline km_tag_type_t km_tag_get_type(km_tag_t tag)
{
    return ((km_tag_type_t)KM_TAG_MASK_TYPE(tag));
}

static inline bool km_tag_is_repeatable(km_tag_t tag)
{
    bool is_repeatable = false;

    switch (km_tag_get_type(tag)) {
    case KM_ENUM_REP:
    case KM_UINT_REP:
    case KM_ULONG_REP:
        is_repeatable = true;
        break;

    default:
        is_repeatable = false;
        break;
    }
    return is_repeatable;
}

#ifdef __cplusplus
}
#endif


#endif  /* KEYMASTER_TAGS_H__ */
