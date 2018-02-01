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

#ifndef KEYMASTER_KEY_PARAMS_H__
#define KEYMASTER_KEY_PARAMS_H__


#include "bstd.h"
#include "keymaster_types.h"

#define TEST_TAG_ADD_INTEGER(param_handle, tag, value)          err = KM_Tag_AddInteger(param_handle, tag, value); \
                                                                if (err != BERR_SUCCESS) { BDBG_ERR(("%s:%d  Tag add failed", BSTD_FUNCTION, __LINE__)); goto done; }

#define TEST_TAG_ADD_LONG_INTEGER(param_handle, tag, value)     err = KM_Tag_AddLongInteger(param_handle, tag, value); \
                                                                if (err != BERR_SUCCESS) { BDBG_ERR(("%s:%d  Tag add failed", BSTD_FUNCTION, __LINE__)); goto done; }

#define TEST_TAG_ADD_ENUM(param_handle, tag, value)             err = KM_Tag_AddEnum(param_handle, tag, (uint32_t)value); \
                                                                if (err != BERR_SUCCESS) { BDBG_ERR(("%s:%d  Tag add failed", BSTD_FUNCTION, __LINE__)); goto done; }

#define TEST_TAG_ADD_BOOL(param_handle, tag, value)             err = KM_Tag_AddBool(param_handle, tag, value); \
                                                                if (err != BERR_SUCCESS) { BDBG_ERR(("%s:%d  Tag add failed", BSTD_FUNCTION, __LINE__)); goto done; }

#define TEST_TAG_ADD_BYTES(param_handle, tag, len, data)        err = KM_Tag_AddBlob(param_handle, tag, len, data); \
                                                                if (err != BERR_SUCCESS) { BDBG_ERR(("%s:%d  Tag add failed", BSTD_FUNCTION, __LINE__)); goto done; }

#define TEST_TAG_REMOVE(param_handle, find_tag)                 do { km_tag_value_t *tag = KM_Tag_FindFirst(param_handle, find_tag); \
                                                                     if (tag) { EXPECT_SUCCESS(KM_Tag_Remove(param_handle, tag)); KM_Tag_Free(tag); } else { err = BERR_SUCCESS; } \
                                                                } while (0)

#define TEST_TAG_REMOVE_ENUM(param_handle, find_tag, value)     do { km_tag_value_t *tag = KM_Tag_FindEnum(param_handle, find_tag, value); \
                                                                     if (tag) { EXPECT_SUCCESS(KM_Tag_Remove(param_handle, tag)); KM_Tag_Free(tag); }  else { err = BERR_SUCCESS; } \
                                                                } while (0)


/* Fn pointer for key_params functions */
typedef BERR_Code (*km_key_param_fn)(KM_Tag_ContextHandle *key_params, uint32_t key_size);

typedef BERR_Code (*km_key_modifier_fn)(KM_Tag_ContextHandle key_params);


/* Remove the KM_TAG_KEY_SIZE */
BERR_Code km_test_remove_key_size(KM_Tag_ContextHandle key_params);

/* Remove the KM_TAG_EC_CURVE */
BERR_Code km_test_remove_curve(KM_Tag_ContextHandle key_params);

/* Add KM_EC_CURVE_P_256 for KM_TAG_EC_CURVE */
BERR_Code km_test_add_256_curve(KM_Tag_ContextHandle key_params);

/* Remove the KM_TAG_RSA_PUBLIC_EXPONENT */
BERR_Code km_test_remove_exponent(KM_Tag_ContextHandle key_params);

/* Remove the KM_TAG_ALL_APPLICATIONS */
BERR_Code km_test_remove_all_apps(KM_Tag_ContextHandle key_params);

/* Add default KM_TAG_APPLICATION_ID */
BERR_Code km_test_add_app_id(KM_Tag_ContextHandle key_params);

/* Add default KM_TAG_APPLICATION_DATA */
BERR_Code km_test_add_app_data(KM_Tag_ContextHandle key_params);

/* Add KM_DIGEST_SHA1 for KM_TAG_DIGEST */
BERR_Code km_test_add_sha1_digest(KM_Tag_ContextHandle key_params);

/* Add KM_DIGEST_NONE for KM_TAG_DIGEST */
BERR_Code km_test_add_none_digest(KM_Tag_ContextHandle key_params);

/* Remove KM_TAG_DIGEST */
BERR_Code km_test_remove_digest(KM_Tag_ContextHandle key_params);

/* Remove mac length */
BERR_Code km_test_remove_mac_length(KM_Tag_ContextHandle key_params);

/* Remove KM_TAG_MIN_MAC_LENGTH */
BERR_Code km_test_remove_min_mac_length(KM_Tag_ContextHandle key_params);

/* Add KM_TAG_MIN_MAC_LENGTH=48 */
BERR_Code km_test_add_min_mac_length_48(KM_Tag_ContextHandle key_params);

/* Add KM_TAG_MIN_MAC_LENGTH=130 */
BERR_Code km_test_add_min_mac_length_130(KM_Tag_ContextHandle key_params);

/* Add KM_TAG_MIN_MAC_LENGTH=384 */
BERR_Code km_test_add_min_mac_length_384(KM_Tag_ContextHandle key_params);

/* Add KM_TAG_KEY_SIZE=1024 */
BERR_Code km_test_add_key_size_1024(KM_Tag_ContextHandle key_params);

/* Add KM_TAG_KEY_SIZE=256 */
BERR_Code km_test_add_key_size_256(KM_Tag_ContextHandle key_params);

/* Add KM_TAG_RSA_PUBLIC_EXPONENT=3 */
BERR_Code km_test_add_exponent_3(KM_Tag_ContextHandle key_params);

BERR_Code km_test_remove_sign_verify(KM_Tag_ContextHandle key_params);

BERR_Code km_test_remove_encrypt_decrypt(KM_Tag_ContextHandle key_params);

/* Add AES key default parameters to tag context handle */
BERR_Code km_test_new_params_with_aes_defaults(KM_Tag_ContextHandle *key_params, uint32_t key_size);

/* Add HMAC key default parameters to tag context handle */
BERR_Code km_test_new_params_with_hmac_defaults(KM_Tag_ContextHandle *key_params, uint32_t key_size);

/* Add RSA key default parameters to tag context handle */
BERR_Code km_test_new_params_with_rsa_defaults(KM_Tag_ContextHandle *key_params, uint32_t key_size);

/* Add EC key default parameters to tag context handle */
BERR_Code km_test_new_params_with_ec_defaults(KM_Tag_ContextHandle *key_params, uint32_t key_size);


/* Update the tag context handle with secure ID tags (removes ALL_USERS and NO_AUTH tags) */
BERR_Code km_test_update_with_secure_id(KM_Tag_ContextHandle key_params);

/* Allocate a new params context and copy KM_TAG_APPLICATION_ID and KM_TAG_APPLICATION_DATA, if exists */
BERR_Code km_test_copy_app_id_and_data(KM_Tag_ContextHandle key_params, KM_Tag_ContextHandle *params);


#endif  /* KEYMASTER_KEY_PARAMS_H__ */
