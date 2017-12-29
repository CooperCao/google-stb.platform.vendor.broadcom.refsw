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

#include "bstd.h"
#include "blst_list.h"
#include "bkni.h"
#include "nexus_memory.h"

#include "keymaster_platform.h"
#include "keymaster_tags.h"
#include "keymaster_err.h"


BDBG_MODULE(keymaster_tags);


static BDBG_OBJECT_ID(KM_Tag_Context);
static BDBG_OBJECT_ID(KM_Tag_Item);

typedef struct KM_Tag_Item {
    BLST_D_ENTRY(KM_Tag_Item) link;
    BDBG_OBJECT(KM_Tag_Item)
    uint8_t tagValueData[];
} KM_Tag_Item;

/* Keymaster tag context */
struct KM_Tag_Context {
    BDBG_OBJECT(KM_Tag_Context)

    BLST_D_HEAD(KM_TagItemList, KM_Tag_Item) tagList;

    int32_t num_params;

    KM_Tag_Item *current;
    km_tag_t tag;
};


static inline bool km_tag_is_valid(km_tag_value_t *param)
{
    bool is_valid = false;

    switch (param->tag) {
        /* These will all just drop through as valid */
    case KM_TAG_PURPOSE :
    case KM_TAG_ALGORITHM :
    case KM_TAG_KEY_SIZE :
    case KM_TAG_BLOCK_MODE :
    case KM_TAG_DIGEST :
    case KM_TAG_PADDING :
    case KM_TAG_CALLER_NONCE :
    case KM_TAG_MIN_MAC_LENGTH :
    case KM_TAG_KDF :
    case KM_TAG_EC_CURVE :
    case KM_TAG_RSA_PUBLIC_EXPONENT :
    case KM_TAG_ECIES_SINGLE_HASH_MODE :
    case KM_TAG_INCLUDE_UNIQUE_ID :
    case KM_TAG_BLOB_USAGE_REQUIREMENTS :
    case KM_TAG_BOOTLOADER_ONLY :
    case KM_TAG_ACTIVE_DATETIME :
    case KM_TAG_ORIGINATION_EXPIRE_DATETIME :
    case KM_TAG_USAGE_EXPIRE_DATETIME :
    case KM_TAG_MIN_SECONDS_BETWEEN_OPS :
    case KM_TAG_MAX_USES_PER_BOOT :
    case KM_TAG_ALL_USERS :
    case KM_TAG_USER_ID :
    case KM_TAG_USER_SECURE_ID :
    case KM_TAG_NO_AUTH_REQUIRED :
    case KM_TAG_USER_AUTH_TYPE :
    case KM_TAG_AUTH_TIMEOUT :
    case KM_TAG_ALLOW_WHILE_ON_BODY :
    case KM_TAG_ALL_APPLICATIONS :
    case KM_TAG_APPLICATION_ID :
    case KM_TAG_EXPORTABLE :
    case KM_TAG_APPLICATION_DATA :
    case KM_TAG_CREATION_DATETIME :
    case KM_TAG_ORIGIN :
    case KM_TAG_ROLLBACK_RESISTANT :
    case KM_TAG_ROOT_OF_TRUST :
    case KM_TAG_OS_VERSION :
    case KM_TAG_OS_PATCHLEVEL :
    case KM_TAG_UNIQUE_ID :
    case KM_TAG_ATTESTATION_CHALLENGE :
    case KM_TAG_ASSOCIATED_DATA :
    case KM_TAG_NONCE :
    case KM_TAG_AUTH_TOKEN :
    case KM_TAG_MAC_LENGTH :
    case KM_TAG_RESET_SINCE_ID_ROTATION :
        is_valid = true;
        break;

    default:
        is_valid = false;
        break;
    }
    return is_valid;
}

static KM_Tag_Item* km_tag_make_tag_item(km_tag_value_t *tagValuePair, uint32_t *size)
{
    KM_Tag_Item *tag_value = NULL;
    km_tag_value_t *outTagValuePair;
    size_t allocSize = 0;
    uint32_t blobSize = 0;

    BDBG_ASSERT(tagValuePair);
    BDBG_ASSERT(size);

    allocSize = sizeof(KM_Tag_Item) + sizeof(km_tag_value_t);
    if ((km_tag_get_type(tagValuePair->tag) == KM_BIGNUM) || (km_tag_get_type(tagValuePair->tag) == KM_BYTES)) {
        if (tagValuePair->value.blob_data_length > KM_TAG_VALUE_BLOB_MAX_SIZE) {
            BDBG_ERR(("%s: Blob too large", BSTD_FUNCTION));
            goto err_done;
        }
        /* Round to nearest 4 bytes */
        blobSize = (tagValuePair->value.blob_data_length + 3ul) & ~3ul;
        allocSize += blobSize;
    }

    BDBG_MSG(("%s: allocating %lu", BSTD_FUNCTION, allocSize));
    if (NEXUS_Memory_Allocate(allocSize, NULL, (void **)&tag_value) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: Failed to allocate tag value", BSTD_FUNCTION));
        goto err_done;
    }
    BKNI_Memset(tag_value, 0, allocSize);
    BDBG_OBJECT_SET(tag_value, KM_Tag_Item);

    outTagValuePair = (km_tag_value_t *)tag_value->tagValueData;

    BDBG_MSG(("%s: adding 0x%x", BSTD_FUNCTION, tagValuePair->tag));

    switch (km_tag_get_type(tagValuePair->tag)) {
        /* Group drop through */
    case KM_ENUM:
    case KM_ENUM_REP:
    case KM_UINT:
    case KM_UINT_REP:
        *outTagValuePair = *tagValuePair;
        *size = sizeof(*tagValuePair);
        break;
        /* Group drop through */
    case KM_ULONG:
    case KM_ULONG_REP:
    case KM_DATE:
        *outTagValuePair = *tagValuePair;
        *size = sizeof(*tagValuePair);
        break;
    case KM_BOOL:
        *outTagValuePair = *tagValuePair;
        *size = sizeof(*tagValuePair);
        break;
    case KM_BIGNUM:
    case KM_BYTES:
        *outTagValuePair = *tagValuePair;
        BKNI_Memcpy(outTagValuePair->blob_data, tagValuePair->blob_data, tagValuePair->value.blob_data_length);
        *size = sizeof(*tagValuePair) + blobSize;
        break;

    default:
        BDBG_ERR(("%s: Invalid tag type (%x)", BSTD_FUNCTION, km_tag_get_type(tagValuePair->tag)));
        goto err_done;
    }

    goto done;

err_done:
    if (tag_value) {
        BDBG_OBJECT_UNSET(tag_value, KM_Tag_Item);
        NEXUS_Memory_Free((void *)tag_value);
        tag_value = NULL;
    }

done:
    return tag_value;
}

static KM_Tag_ContextHandle km_tag_dup_context(KM_Tag_ContextHandle handle)
{
    BERR_Code err;
    KM_Tag_ContextHandle new_handle = NULL;
    uint32_t size = 0;
    KM_Tag_Item *current = NULL;
    KM_Tag_Item *tag_item = NULL;
    KM_Tag_Item *prev = NULL;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    BDBG_MSG(("%s: allocating %lu", BSTD_FUNCTION, sizeof(struct KM_Tag_Context)));
    if (NEXUS_Memory_Allocate(sizeof(struct KM_Tag_Context), NULL, (void **)&new_handle) != NEXUS_SUCCESS) {
        err = BERR_OUT_OF_DEVICE_MEMORY;
        BDBG_ERR(("%s: Failed to allocate context", BSTD_FUNCTION));
        goto done;
    }

    BKNI_Memset(new_handle, 0, sizeof(struct KM_Tag_Context));
    BDBG_OBJECT_SET(new_handle, KM_Tag_Context);
    BLST_D_INIT(&new_handle->tagList);
    new_handle->num_params = 0;

    for (current = BLST_D_FIRST(&handle->tagList); current; current = BLST_D_NEXT(current, link)) {
        BDBG_OBJECT_ASSERT(current, KM_Tag_Item);
        tag_item = km_tag_make_tag_item((km_tag_value_t *)current->tagValueData, &size);
        if (!tag_item) {
            err = BERR_OUT_OF_DEVICE_MEMORY;
            BDBG_ERR(("%s: Failed to make tag item", BSTD_FUNCTION));
            goto done;
        }

        /* Keep the given order in case it is significant for repeatable items */
        if (!prev) {
            BLST_D_INSERT_HEAD(&new_handle->tagList, tag_item, link);
        } else {
            BLST_D_INSERT_AFTER(&new_handle->tagList, prev, tag_item, link);
        }
        new_handle->num_params += 1;
        prev = tag_item;
    }

    new_handle->current = NULL;
    new_handle->tag = KM_TAG_INVALID;
    err = BERR_SUCCESS;

done:
    if (err != BERR_SUCCESS && new_handle) {
        KM_Tag_DeleteContext(new_handle);
        new_handle = NULL;
    }
    return new_handle;
}

/* Function returns 0 if equal, -1 otherwise */
static int km_tag_cmp_tag_value_pair(km_tag_value_t *tagValuePair1, km_tag_value_t *tagValuePair2)
{
    int result = -1;

    if (tagValuePair1->tag == tagValuePair2->tag) {
        switch (km_tag_get_type(tagValuePair1->tag)) {
            /* Group drop through */
        case KM_ENUM:
        case KM_ENUM_REP:
        case KM_UINT:
        case KM_UINT_REP:
            if (tagValuePair1->value.integer == tagValuePair2->value.integer) {
                result = 0;
            }
            break;
            /* Group drop through */
        case KM_ULONG:
        case KM_ULONG_REP:
        case KM_DATE:
            if (tagValuePair1->value.long_integer == tagValuePair2->value.long_integer) {
                result = 0;
            }
            break;
        case KM_BOOL:
            if (tagValuePair1->value.boolean == tagValuePair2->value.boolean) {
                result = 0;
            }
            break;
        case KM_BIGNUM:
        case KM_BYTES:
            if ((tagValuePair1->value.blob_data_length == tagValuePair2->value.blob_data_length) &&
                (!BKNI_Memcmp(tagValuePair1->blob_data, tagValuePair2->blob_data, tagValuePair1->value.blob_data_length))) {
                result = 0;
            }
            break;

        default:
            BDBG_ERR(("%s: Invalid tag type (%x)", BSTD_FUNCTION, km_tag_get_type(tagValuePair1->tag)));
            break;
        }
    }
    return result;
}

BERR_Code KM_Tag_CreateContext(int32_t num_params, uint32_t len, uint8_t *data_block, KM_Tag_ContextHandle *ret_context)
{
    BERR_Code err;
    KM_Tag_ContextHandle handle = NULL;
    uint8_t *params;
    uint8_t *data_block_end;
    uint32_t size = 0;
    int32_t tag_index;
    KM_Tag_Item *tag_item = NULL;
    KM_Tag_Item *prev = NULL;

    BDBG_ASSERT(ret_context);

    if ((num_params < 0) || ((num_params > 0) && !data_block)) {
        err = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid parameter", BSTD_FUNCTION));
        goto done;
    }

    BDBG_MSG(("%s: allocating %lu", BSTD_FUNCTION, sizeof(struct KM_Tag_Context)));
    if (NEXUS_Memory_Allocate(sizeof(struct KM_Tag_Context), NULL, (void **)&handle) != NEXUS_SUCCESS) {
        err = BERR_OUT_OF_DEVICE_MEMORY;
        BDBG_ERR(("%s: Failed to allocate context", BSTD_FUNCTION));
        goto done;
    }

    BKNI_Memset(handle, 0, sizeof(struct KM_Tag_Context));
    BDBG_OBJECT_SET(handle, KM_Tag_Context);
    BLST_D_INIT(&handle->tagList);

    handle->num_params = 0;
    params = data_block;
    data_block_end = data_block + len;

    for (tag_index = 0; tag_index < num_params; tag_index++) {
        if (params >= data_block_end) {
            /* run out of data */
            err = BERR_INVALID_PARAMETER;
            BDBG_ERR(("%s: Tag/value data is invalid", BSTD_FUNCTION));
            goto done;
        }

        if (!km_tag_is_valid((km_tag_value_t *)params)) {
            err = BERR_INVALID_PARAMETER;
            BDBG_ERR(("%s: Invalid tag", BSTD_FUNCTION));
            goto done;
        }

        tag_item = km_tag_make_tag_item((km_tag_value_t *)params, &size);
        if (!tag_item) {
            err = BERR_OUT_OF_DEVICE_MEMORY;
            BDBG_ERR(("%s: Failed to make tag item", BSTD_FUNCTION));
            goto done;
        }

        /* Keep the given order in case it is significant for repeatable items */
        if (!prev) {
            BLST_D_INSERT_HEAD(&handle->tagList, tag_item, link);
        } else {
            BLST_D_INSERT_AFTER(&handle->tagList, prev, tag_item, link);
        }
        handle->num_params += 1;
        prev = tag_item;
        params += size;
    }

    handle->current = NULL;
    handle->tag = KM_TAG_INVALID;
    err = BERR_SUCCESS;

done:
    if (err != BERR_SUCCESS && handle) {
        KM_Tag_DeleteContext(handle);
        handle = NULL;
    }
    *ret_context = handle;
    return err;
}

BERR_Code KM_Tag_CreateContextFromTagValueSet(uint8_t *data_block, KM_Tag_ContextHandle *handle)
{
    BERR_Code err;
    km_tag_value_set_t *tag_value_set = (km_tag_value_set_t *)data_block;

    if (!tag_value_set) {
        *handle = NULL;
        err = BERR_SUCCESS;
        goto done;
    }

    err = KM_Tag_CreateContext(tag_value_set->num, tag_value_set->size, tag_value_set->params, handle);

done:
    return err;
}

KM_Tag_ContextHandle KM_Tag_DupContext(KM_Tag_ContextHandle handle)
{
    KM_Tag_ContextHandle dup_handle;

    dup_handle = km_tag_dup_context(handle);
    return dup_handle;
}

void KM_Tag_DeleteContext(KM_Tag_ContextHandle handle)
{
    KM_Tag_Item *tag_item = NULL;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    while ((tag_item = BLST_D_FIRST(&handle->tagList))) {
        BDBG_OBJECT_ASSERT(tag_item, KM_Tag_Item);
        BLST_D_REMOVE(&handle->tagList, tag_item, link);
        BDBG_OBJECT_UNSET(tag_item, KM_Tag_Item);
        NEXUS_Memory_Free((void *)tag_item);
    }
    BDBG_OBJECT_DESTROY(handle, KM_Tag_Context);
    NEXUS_Memory_Free((void *)handle);
}

int32_t KM_Tag_GetNumPairs(KM_Tag_ContextHandle handle)
{
    int32_t num = 0;
    if (!handle) {
        goto done;
    }
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);
    BDBG_ASSERT(handle->num_params >= 0);
    num = handle->num_params;

done:
    return num;
}

BERR_Code KM_Tag_CheckParameterConsitency(KM_Tag_ContextHandle handle)
{
    BERR_Code err;
    KM_Tag_Item *tag_item;
    km_tag_value_t *tagValuePair;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    /* Check that none of the non-repeatable types are repeated */
    for (tag_item = BLST_D_FIRST(&handle->tagList); tag_item; tag_item = BLST_D_NEXT(tag_item, link)) {
        BDBG_OBJECT_ASSERT(tag_item, KM_Tag_Item);
        tagValuePair = (km_tag_value_t *)tag_item->tagValueData;
        if (!km_tag_is_repeatable(tagValuePair->tag)) {
            KM_Tag_Item *tag_next;
            km_tag_value_t *nextTagValuePair;

            /* Search the list for another after the current */
            for (tag_next = BLST_D_NEXT(tag_item, link); tag_next; tag_next = BLST_D_NEXT(tag_next, link)) {
                BDBG_OBJECT_ASSERT(tag_next, KM_Tag_Item);
                nextTagValuePair = (km_tag_value_t *)tag_next->tagValueData;
                if (tagValuePair->tag == nextTagValuePair->tag) {
                    BDBG_ERR(("%s: param is repeated (%x)", BSTD_FUNCTION, tagValuePair->tag));
                    err = BERR_INVALID_PARAMETER;
                    goto done;
                }
            }
        }
    }

    err = BERR_SUCCESS;

done:
    return err;
}

BERR_Code KM_Tag_ValidateKeyParameters(KM_Tag_ContextHandle handle)
{
    BERR_Code err;
    KM_Tag_Item *tag_item;
    km_tag_value_t *tagValuePair;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    err = KM_Tag_CheckParameterConsitency(handle);
    if (err != BERR_SUCCESS) {
        goto done;
    }

    /* These tags cannot be specified in the key tags associated with a generate or import */
    for (tag_item = BLST_D_FIRST(&handle->tagList); tag_item; tag_item = BLST_D_NEXT(tag_item, link)) {
        BDBG_OBJECT_ASSERT(tag_item, KM_Tag_Item);
        tagValuePair = (km_tag_value_t *)tag_item->tagValueData;
        switch (tagValuePair->tag) {
        case KM_TAG_INVALID:
        case KM_TAG_AUTH_TOKEN:
        case KM_TAG_APPLICATION_DATA:
        case KM_TAG_ATTESTATION_CHALLENGE:
        case KM_TAG_ORIGIN:
        case KM_TAG_ROOT_OF_TRUST:
        case KM_TAG_OS_VERSION:
        case KM_TAG_OS_PATCHLEVEL:
            err = BERR_INVALID_PARAMETER;
            goto done;
        default:
            /* Everything else is acceptable */
            break;
        }
    }

    err = BERR_SUCCESS;

done:
    return err;
}

static KM_Tag_Item* km_tag_find_from_starting_point(KM_Tag_ContextHandle handle, km_tag_t tag, KM_Tag_Item *start, bool update_position)
{
    KM_Tag_Item *tag_item = NULL;
    km_tag_value_t *tagValuePair;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    if (!start) {
        /* At end of list so nothing to find */
        goto done;
    }

    BDBG_OBJECT_ASSERT(start, KM_Tag_Item);
    for (tag_item = start; tag_item; tag_item = BLST_D_NEXT(tag_item, link)) {
        BDBG_OBJECT_ASSERT(tag_item, KM_Tag_Item);
        tagValuePair = (km_tag_value_t *)tag_item->tagValueData;
        if (tagValuePair->tag == tag) {
            if (update_position) {
                handle->current = tag_item;
                handle->tag = tag;
            }
            break;
        }
    }

    if (!tag_item && update_position) {
        handle->current = NULL;
        handle->tag = KM_TAG_INVALID;
    }

done:
    return tag_item;
}

km_tag_value_t* KM_Tag_FindFirst(KM_Tag_ContextHandle handle, km_tag_t tag)
{
    KM_Tag_Item *tag_item = NULL;
    km_tag_value_t *tag_value = NULL;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);
    tag_item = km_tag_find_from_starting_point(handle, tag, (KM_Tag_Item *)BLST_D_FIRST(&handle->tagList), true);
    if (!tag_item) {
        goto done;
    }
    tag_value = (km_tag_value_t *)tag_item->tagValueData;

done:
    return tag_value;
}

km_tag_value_t* KM_Tag_FindNext(KM_Tag_ContextHandle handle)
{
    KM_Tag_Item *tag_item = NULL;
    km_tag_value_t *tag_value = NULL;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    if (!km_tag_is_repeatable(handle->tag)) {
        BDBG_ERR(("%s: tag not repeatable (%x)", BSTD_FUNCTION, handle->tag));
        handle->current = NULL;
        handle->tag = KM_TAG_INVALID;
        goto done;
    }
    if (!handle->current || (handle->tag == KM_TAG_INVALID)) {
        BDBG_ERR(("%s: no current search", BSTD_FUNCTION));
        handle->current = NULL;
        handle->tag = KM_TAG_INVALID;
        goto done;
    }

    tag_item = km_tag_find_from_starting_point(handle, handle->tag, (KM_Tag_Item *)BLST_D_NEXT(handle->current, link), true);
    if (!tag_item) {
        goto done;
    }
    tag_value = (km_tag_value_t *)tag_item->tagValueData;

done:
    return tag_value;
}

km_tag_value_t* KM_Tag_FindNth(KM_Tag_ContextHandle handle, km_tag_t tag, uint32_t n)
{
    KM_Tag_Item *tag_item = NULL;
    km_tag_value_t *tag_value = NULL;
    uint32_t found;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    if ((n > 0) && !km_tag_is_repeatable(tag)) {
        BDBG_ERR(("%s: tag not repeatable (%x)", BSTD_FUNCTION, tag));
        goto done;
    }

    /* Base 0 counting for n, so if we request item 0, it is the first item */
    for (found = 0; found <= n; found++) {
        if (!tag_item) {
            tag_item = km_tag_find_from_starting_point(handle, tag, BLST_D_FIRST(&handle->tagList), false);
        } else {
            tag_item = km_tag_find_from_starting_point(handle, tag, BLST_D_NEXT(tag_item, link), false);
        }
        if (!tag_item) {
            goto done;
        }
    }
    tag_value = (km_tag_value_t *)tag_item->tagValueData;

done:
    return tag_value;
}

km_tag_value_t* KM_Tag_First(KM_Tag_ContextHandle handle)
{
    km_tag_value_t *tag_value = NULL;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    handle->current = BLST_D_FIRST(&handle->tagList);
    handle->tag = KM_TAG_INVALID;

    if (handle->current) {
        BDBG_OBJECT_ASSERT(handle->current, KM_Tag_Item);
        tag_value = (km_tag_value_t *)handle->current->tagValueData;
    }

    return tag_value;
}

km_tag_value_t* KM_Tag_Next(KM_Tag_ContextHandle handle)
{
    km_tag_value_t *tag_value = NULL;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    if ((handle->tag != KM_TAG_INVALID) || !handle->current) {
        BDBG_ERR(("%s: no current search", BSTD_FUNCTION));
        handle->current = NULL;
        handle->tag = KM_TAG_INVALID;
        goto done;
    }

    handle->current = BLST_D_NEXT(handle->current, link);

    if (handle->current) {
        BDBG_OBJECT_ASSERT(handle->current, KM_Tag_Item);
        tag_value = (km_tag_value_t *)handle->current->tagValueData;
    }

done:
    return tag_value;
}

km_tag_value_t* KM_Tag_FindEnum(KM_Tag_ContextHandle handle, km_tag_t tag, uint32_t enumerated)
{
    KM_Tag_Item *tag_item = NULL;
    km_tag_value_t *tag_value = NULL;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    if ((km_tag_get_type(tag) != KM_ENUM) && (km_tag_get_type(tag) != KM_ENUM_REP)) {
        BDBG_ERR(("%s: tag not enum", BSTD_FUNCTION));
        goto done;
    }

    tag_item = km_tag_find_from_starting_point(handle, tag, BLST_D_FIRST(&handle->tagList), false);
    while (tag_item) {
        tag_value = (km_tag_value_t *)tag_item->tagValueData;
        if (tag_value->value.enumerated == enumerated) {
            break;
        }

        tag_item = km_tag_find_from_starting_point(handle, tag, BLST_D_NEXT(tag_item, link), false);
    }

done:
    return tag_item ? (tag_item->tagValueData) : NULL;
}

bool KM_Tag_ContainsEnum(KM_Tag_ContextHandle handle, km_tag_t tag, uint32_t enumerated)
{
    return (KM_Tag_FindEnum(handle, tag, enumerated) != NULL);
}

bool KM_Tag_ContainsInteger(KM_Tag_ContextHandle handle, km_tag_t tag, uint32_t integer)
{
    KM_Tag_Item *tag_item = NULL;
    km_tag_value_t *tag_value = NULL;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    if ((km_tag_get_type(tag) != KM_UINT) && (km_tag_get_type(tag) != KM_UINT_REP)) {
        BDBG_ERR(("%s: tag not integer", BSTD_FUNCTION));
        goto done;
    }

    tag_item = km_tag_find_from_starting_point(handle, tag, BLST_D_FIRST(&handle->tagList), false);
    while (tag_item) {
        tag_value = (km_tag_value_t *)tag_item->tagValueData;
        if (tag_value->value.integer == integer) {
            break;
        }

        tag_item = km_tag_find_from_starting_point(handle, tag, BLST_D_NEXT(tag_item, link), false);
    }

done:
    return (tag_item != NULL);
}

bool KM_Tag_ContainsLongInteger(KM_Tag_ContextHandle handle, km_tag_t tag, uint64_t long_integer)
{
    KM_Tag_Item *tag_item = NULL;
    km_tag_value_t *tag_value = NULL;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    if ((km_tag_get_type(tag) != KM_ULONG) && (km_tag_get_type(tag) != KM_ULONG_REP)) {
        BDBG_ERR(("%s: tag not long", BSTD_FUNCTION));
        goto done;
    }

    tag_item = km_tag_find_from_starting_point(handle, tag, BLST_D_FIRST(&handle->tagList), false);
    while (tag_item) {
        tag_value = (km_tag_value_t *)tag_item->tagValueData;
        if (tag_value->value.long_integer == long_integer) {
            break;
        }

        tag_item = km_tag_find_from_starting_point(handle, tag, BLST_D_NEXT(tag_item, link), false);
    }

done:
    return (tag_item != NULL);
}

uint32_t KM_Tag_Count(KM_Tag_ContextHandle handle, km_tag_t tag)
{
    KM_Tag_Item *tag_item = NULL;
    uint32_t count = 0;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    /* We don't bother trapping non-repeatable tags */

    tag_item = km_tag_find_from_starting_point(handle, tag, BLST_D_FIRST(&handle->tagList), false);
    while (tag_item) {
        count += 1;
        tag_item = km_tag_find_from_starting_point(handle, tag, BLST_D_NEXT(tag_item, link), false);
    }

    return count;
}

BERR_Code KM_Tag_Add(KM_Tag_ContextHandle handle, km_tag_value_t *tag_value_pair)
{
    BERR_Code err;
    KM_Tag_Item *tag_item = NULL;
    uint32_t size = 0;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);

    tag_item = km_tag_make_tag_item(tag_value_pair, &size);
    if (!tag_item) {
        err = BERR_OUT_OF_DEVICE_MEMORY;
        BDBG_ERR(("%s: Failed to make tag item", BSTD_FUNCTION));
        goto done;
    }

    BLST_D_INSERT_HEAD(&handle->tagList, tag_item, link);
    handle->num_params += 1;
    err = BERR_SUCCESS;

done:
    return err;
}

BERR_Code KM_Tag_AddEnum(KM_Tag_ContextHandle handle, const km_tag_t tag, const uint32_t enum_value)
{
    BERR_Code err;
    km_tag_value_t tagValuePair;

    BDBG_ASSERT((km_tag_get_type(tag) == KM_ENUM) || (km_tag_get_type(tag) == KM_ENUM_REP));
    tagValuePair.tag = tag;
    tagValuePair.value.enumerated = enum_value;
    /* coverity[uninit_use_in_call : FALSE] */
    err = KM_Tag_Add(handle, &tagValuePair);
    return err;
}

BERR_Code KM_Tag_AddInteger(KM_Tag_ContextHandle handle, const km_tag_t tag, const uint32_t integer_value)
{
    BERR_Code err;
    km_tag_value_t tagValuePair;

    BDBG_ASSERT((km_tag_get_type(tag) == KM_UINT) || (km_tag_get_type(tag) == KM_UINT_REP));
    tagValuePair.tag = tag;
    tagValuePair.value.integer = integer_value;
    /* coverity[uninit_use_in_call : FALSE] */
    err = KM_Tag_Add(handle, &tagValuePair);
    return err;
}

BERR_Code KM_Tag_AddLongInteger(KM_Tag_ContextHandle handle, const km_tag_t tag, const uint64_t long_int_value)
{
    BERR_Code err;
    km_tag_value_t tagValuePair;

    BDBG_ASSERT((km_tag_get_type(tag) == KM_ULONG) || (km_tag_get_type(tag) == KM_ULONG_REP));
    tagValuePair.tag = tag;
    tagValuePair.value.long_integer = long_int_value;
    /* coverity[uninit_use_in_call : FALSE] */
    err = KM_Tag_Add(handle, &tagValuePair);
    return err;
}

BERR_Code KM_Tag_AddDate(KM_Tag_ContextHandle handle, const km_tag_t tag, const uint64_t date_value)
{
    BERR_Code err;
    km_tag_value_t tagValuePair;

    BDBG_ASSERT(km_tag_get_type(tag) == KM_DATE);
    tagValuePair.tag = tag;
    tagValuePair.value.date_time = date_value;
    /* coverity[uninit_use_in_call : FALSE] */
    err = KM_Tag_Add(handle, &tagValuePair);
    return err;
}

BERR_Code KM_Tag_AddBool(KM_Tag_ContextHandle handle, const km_tag_t tag, const bool bool_value)
{
    BERR_Code err;
    km_tag_value_t tagValuePair;

    BDBG_ASSERT(km_tag_get_type(tag) == KM_BOOL);
    tagValuePair.tag = tag;
    tagValuePair.value.boolean = bool_value;
    /* coverity[uninit_use_in_call : FALSE] */
    err = KM_Tag_Add(handle, &tagValuePair);
    return err;
}

BERR_Code KM_Tag_AddBlob(KM_Tag_ContextHandle handle, const km_tag_t tag, const uint32_t blob_len, const uint8_t *blob_data)
{
    BERR_Code err;
    km_tag_value_t *tagValuePair;

    BDBG_ASSERT((km_tag_get_type(tag) == KM_BYTES) || (km_tag_get_type(tag) == KM_BIGNUM));
    BDBG_ASSERT(blob_data);
    if (NEXUS_Memory_Allocate(sizeof(km_tag_value_t) + blob_len, NULL, (void **)&tagValuePair) != NEXUS_SUCCESS) {
        err = BERR_OUT_OF_DEVICE_MEMORY;
        goto done;
    }
    tagValuePair->tag = tag;
    tagValuePair->value.blob_data_length = blob_len;
    BKNI_Memcpy(tagValuePair->blob_data, blob_data, blob_len);
    err = KM_Tag_Add(handle, tagValuePair);
    NEXUS_Memory_Free((void *)tagValuePair);

done:
    return err;
}

BERR_Code KM_Tag_Remove(KM_Tag_ContextHandle handle, km_tag_value_t *tag_value_pair)
{
    BERR_Code err = BERR_UNKNOWN;
    KM_Tag_Item *tag_item = NULL;
    KM_Tag_Item *cmp_item = NULL;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);
    BDBG_ASSERT(tag_value_pair);

    /* We don't just remove the tag, we search for it in case we get something that does not exist on the list */
    cmp_item = (KM_Tag_Item *)((uint8_t *)tag_value_pair - offsetof(KM_Tag_Item, tagValueData));
    BDBG_OBJECT_ASSERT(cmp_item, KM_Tag_Item);

    for (tag_item = BLST_D_FIRST(&handle->tagList); tag_item; tag_item = BLST_D_NEXT(tag_item, link)) {
        BDBG_OBJECT_ASSERT(tag_item, KM_Tag_Item);
        if (tag_item == cmp_item) {
            BDBG_MSG(("%s: removing 0x%x", BSTD_FUNCTION, tag_value_pair->tag));
            BLST_D_REMOVE(&handle->tagList, tag_item, link);
            handle->num_params -= 1;
            err = BERR_SUCCESS;
            break;
        }
    }

    if (err != BERR_SUCCESS) {
        BDBG_ERR(("%s: tag not found on list", BSTD_FUNCTION));
        err = BSAGE_ERR_INTERNAL;
    }

    return err;
}

void KM_Tag_Free(km_tag_value_t *tag_value_pair)
{
    KM_Tag_Item *tag_item = NULL;

    BDBG_ASSERT(tag_value_pair);

    tag_item = (KM_Tag_Item *)((uint8_t *)tag_value_pair - offsetof(KM_Tag_Item, tagValueData));
    BDBG_OBJECT_ASSERT(tag_item, KM_Tag_Item);
    BDBG_OBJECT_UNSET(tag_item, KM_Tag_Item);
    NEXUS_Memory_Free((void *)tag_item);
}

#if 0
#define DUMP_MAX 64
static void dump_buffer(uint8_t *buffer, uint32_t len)
{
    uint32_t i;
    uint8_t buf[DUMP_MAX * 2 + 1];
    uint8_t *ptr;

    ptr = buf;
    for (i = 0; i < len && i < DUMP_MAX; i++) {
        snprintf(ptr, sizeof(buf) - 2 * i, "%02X", buffer[i]);
        ptr += 2;
    }
    *ptr = 0;
    BDBG_LOG(("BUF: %s", buf));
}
#endif

BERR_Code KM_Tag_Serialize(KM_Tag_ContextHandle handle, uint8_t *buffer, uint32_t *buffer_size)
{
    KM_Tag_Item *tag_item = NULL;
    uint32_t size = 0;
    BERR_Code err;
    uint8_t *ptr;
    km_tag_value_t *tagValuePair;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);
    BDBG_ASSERT(buffer_size);

    /* Calculate the size required */
    for (tag_item = BLST_D_FIRST(&handle->tagList); tag_item; tag_item = BLST_D_NEXT(tag_item, link)) {
        BDBG_OBJECT_ASSERT(tag_item, KM_Tag_Item);
        tagValuePair = (km_tag_value_t *)tag_item->tagValueData;
        switch (km_tag_get_type(tagValuePair->tag)) {
            /* Group drop through */
        case KM_ENUM:
        case KM_ENUM_REP:
        case KM_UINT:
        case KM_UINT_REP:
        case KM_ULONG:
        case KM_ULONG_REP:
        case KM_DATE:
        case KM_BOOL:
            size += sizeof(km_tag_value_t);
            break;
        case KM_BIGNUM:
        case KM_BYTES:
            /* Store tag + data length + data */
            size += sizeof(km_tag_value_t);
            size += (tagValuePair->value.blob_data_length + 3ul) & ~3ul;
            break;
        default:
            BDBG_ERR(("%s: Invalid tag type (%x)", BSTD_FUNCTION, km_tag_get_type(tagValuePair->tag)));
            size = 0;
            err = BSAGE_ERR_KM_INVALID_TAG;
            goto done;
        }
    }

    if (!buffer) {
        /* If buffer is NULL, all we do is return the required size */
        err = BERR_SUCCESS;
        goto done;
    }
    if (*buffer_size < size) {
        BDBG_ERR(("%s: insufficient buffer space (got %d, need %d)", BSTD_FUNCTION,*buffer_size, size));
        err = BSAGE_ERR_KM_INSUFFICIENT_BUFFER_SPACE;
        goto done;
    }

    /* Now serialize the data into the given buffer */
    ptr = buffer;
    for (tag_item = BLST_D_FIRST(&handle->tagList); tag_item; tag_item = BLST_D_NEXT(tag_item, link)) {
        uint32_t data_size = 0;

        BDBG_OBJECT_ASSERT(tag_item, KM_Tag_Item);
        tagValuePair = (km_tag_value_t *)tag_item->tagValueData;
        switch (km_tag_get_type(tagValuePair->tag)) {
            /* Group drop through */
        case KM_ENUM:
        case KM_ENUM_REP:
        case KM_UINT:
        case KM_UINT_REP:
        case KM_ULONG:
        case KM_ULONG_REP:
        case KM_DATE:
        case KM_BOOL:
            data_size = sizeof(km_tag_value_t);
            BKNI_Memcpy(ptr, &tag_item->tagValueData, data_size);
            ptr += data_size;
            break;
        case KM_BIGNUM:
        case KM_BYTES:
            data_size = sizeof(km_tag_value_t);
            data_size += (tagValuePair->value.blob_data_length + 3ul) & ~3ul;
            /* It doesn't matter that we copy the extra padding bytes */
            BKNI_Memcpy(ptr, &tag_item->tagValueData, data_size);
            ptr += data_size;
            break;

        default:
            BDBG_ERR(("%s: Invalid tag type (%x)", BSTD_FUNCTION, km_tag_get_type(tagValuePair->tag)));
            size = 0;
            err = BSAGE_ERR_KM_INVALID_TAG;
            goto done;
        }
        BDBG_MSG(("%s: adding %x, size %d", BSTD_FUNCTION, tagValuePair->tag, data_size));
    }
    err = BERR_SUCCESS;

done:
    if (err == BERR_SUCCESS) {
        /* On success we update the incoming buffer size with the actual size used */
        *buffer_size = size;
    }
    return err;
}

BERR_Code KM_Tag_SerializeAsTagValueSet(KM_Tag_ContextHandle handle, uint8_t *buffer, uint32_t *buffer_size)
{
    uint32_t size = 0;
    BERR_Code err;
    uint8_t *ptr;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);
    BDBG_ASSERT(buffer_size);

    err = KM_Tag_Serialize(handle, NULL, &size);
    if (err != BERR_SUCCESS) {
        BDBG_ERR(("%s: failed to get buffer size", BSTD_FUNCTION));
        goto done;
    }
    size += 2 * sizeof(uint32_t);
    if (size > KM_TAG_VALUE_BLOCK_SIZE) {
        BDBG_ERR(("%s: tag value set too large", BSTD_FUNCTION));
        err = BSAGE_ERR_INTERNAL;
        goto done;
    }

    if (!buffer) {
        /* If buffer is NULL, all we do is return the required size */
        err = BERR_SUCCESS;
        goto done;
    }
    if (*buffer_size < size) {
        BDBG_ERR(("%s: insufficient buffer space (got %d, need %d)", BSTD_FUNCTION,*buffer_size, size));
        err = BSAGE_ERR_KM_INSUFFICIENT_BUFFER_SPACE;
        goto done;
    }

    ptr = buffer;
    BKNI_Memcpy(ptr, (uint8_t *)&handle->num_params, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    size -= 2 * sizeof(uint32_t);
    BKNI_Memcpy(ptr, (uint8_t *)&size, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    err = KM_Tag_Serialize(handle, ptr, &size);
    /* Just drop through with the result of serialize */

done:
    if (err == BERR_SUCCESS) {
        /* On success we update the incoming buffer size with the actual size used */
        *buffer_size = size + 2 * sizeof(uint32_t);
    }
    return err;
}

int KM_Tag_ContextCmp(KM_Tag_ContextHandle handle, KM_Tag_ContextHandle cmp_handle)
{
    KM_Tag_ContextHandle dup_handle = NULL;
    KM_Tag_Item *tag_item = NULL;
    KM_Tag_Item *cmp_item = NULL;
    km_tag_value_t *tagValuePair = NULL;
    km_tag_value_t *cmpValuePair = NULL;
    int result = -1;

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KM_Tag_Context);
    BDBG_ASSERT(cmp_handle);
    BDBG_OBJECT_ASSERT(cmp_handle, KM_Tag_Context);

    /* Fist the simple test that the handles have the same number of elements */
    if (handle->num_params != cmp_handle->num_params) {
        goto done;
    }

    /*
     * Compare the contexts by duplicating the second context, finding each element and deleting
     * them from the duplicate. If we have any left, then the context compare fails.
     */
    dup_handle = km_tag_dup_context(cmp_handle);
    if (!dup_handle) {
        goto done;
    }

    for (tag_item = BLST_D_FIRST(&handle->tagList); tag_item; tag_item = BLST_D_NEXT(tag_item, link)) {
        BDBG_OBJECT_ASSERT(tag_item, KM_Tag_Item);
        tagValuePair = (km_tag_value_t *)tag_item->tagValueData;
        for (cmp_item = BLST_D_FIRST(&dup_handle->tagList); cmp_item; cmp_item = BLST_D_NEXT(cmp_item, link)) {
            BDBG_OBJECT_ASSERT(cmp_item, KM_Tag_Item);
            cmpValuePair = (km_tag_value_t *)cmp_item->tagValueData;
            if (tagValuePair->tag == cmpValuePair->tag) {
                if (!km_tag_cmp_tag_value_pair(tagValuePair, cmpValuePair)) {
                    BLST_D_REMOVE(&dup_handle->tagList, cmp_item, link);
                    BDBG_OBJECT_UNSET(cmp_item, KM_Tag_Item);
                    NEXUS_Memory_Free((void *)cmp_item);
                    dup_handle->num_params -= 1;
                    break;
                } else {
                    /* Tags of the same type must be in the same order, so cmp failed */
                    goto done;
                }
            }
        }
    }
    if (BLST_D_FIRST(&dup_handle->tagList) == NULL) {
        /* All items removed so contexts are identical */
        result = 0;
    }

done:
    if (dup_handle) {
        KM_Tag_DeleteContext(dup_handle);
    }
    return result;
}

km_tag_value_t* KM_Tag_Dup(km_tag_value_t *tagValuePair)
{
    km_tag_value_t *new_tag = NULL;
    uint32_t size = sizeof(km_tag_value_t);

    BDBG_ASSERT(tagValuePair);
    if ((km_tag_get_type(tagValuePair->tag) == KM_BIGNUM) || (km_tag_get_type(tagValuePair->tag) == KM_BYTES)) {
        if (tagValuePair->value.blob_data_length > KM_TAG_VALUE_BLOB_MAX_SIZE) {
            BDBG_ERR(("%s: Blob too large", BSTD_FUNCTION));
            goto done;
        }
        size += tagValuePair->value.blob_data_length;
    }

    if (NEXUS_Memory_Allocate(size, NULL, (void **)&new_tag) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: Failed to allocate tag value", BSTD_FUNCTION));
        goto done;
    }
    *new_tag = *tagValuePair;
    if ((km_tag_get_type(tagValuePair->tag) == KM_BIGNUM) || (km_tag_get_type(tagValuePair->tag) == KM_BYTES)) {
        BKNI_Memcpy(new_tag->blob_data, tagValuePair->blob_data, tagValuePair->value.blob_data_length);
    }

done:
    return new_tag;
}
