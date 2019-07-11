/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include <string.h>
#include <time.h>

#include "nexus_memory.h"
#include "nexus_security_client.h"
#include "nexus_core_utils.h"
#include "keymaster_platform.h"
#include "keymaster_ids.h"
#include "keymaster_tl.h"
#include "keymaster_err.h"
#include "keymaster_cmd_basic.h"
#include "keymaster_cmd_keys.h"
#include "keymaster_cmd_crypto.h"

/* When BSAGE_ERR_KM_AGAIN received, give up after this long */
#define KM_CMD_TIMEOUT_IN_SECONDS 20

#ifdef ANDROID
#define KEYMASTER_DRM_BIN_DEFAULT_FILEPATH "/dev/hwcfg/km.bin"
#define KEYMASTER_DRM_BIN_ALTERNATE_FILEPATH "/dev/hwcfg/drm.bin"
#else
#define KEYMASTER_DRM_BIN_DEFAULT_FILEPATH "./drm.bin"
#define KEYMASTER_DRM_BIN_ALTERNATE_FILEPATH ""
#endif

BDBG_MODULE(keymaster_tl);

struct KeymasterTl_Instance {
    BDBG_OBJECT(KeymasterTl_Instance)
    SRAI_ModuleHandle moduleHandle;
    SRAI_ModuleHandle ssdModuleHandle;
    uint32_t configuredKeymasterVersion;
};

BDBG_OBJECT_ID_DECLARE(KeymasterTl_Instance);
BDBG_OBJECT_ID(KeymasterTl_Instance);


#define KM_ALLOCATE_CONTAINER()                      inout = SRAI_Container_Allocate(); \
                                                     if (!inout) { err = BERR_OUT_OF_SYSTEM_MEMORY; goto done; }


#define KM_FREE_CONTAINER()                          if (inout) { SRAI_Container_Free(inout); }


#define KM_ALLOCATE_BLOCK(ptr, size)                 ptr = (uint8_t *)SRAI_Memory_Allocate(size, SRAI_MemoryType_Shared); \
                                                     if (!ptr) { err = BERR_OUT_OF_SYSTEM_MEMORY; goto done; }


#define KM_FREE_BLOCK(ptr)                           if (ptr) { SRAI_Memory_Free(ptr); ptr = NULL; }


#define KM_PROCESS_COMMAND(cmd_id)                   while (1) { \
                                                         time_t start, now; \
                                                         start = time(NULL); \
                                                         while (1) { \
                                                             now = time(NULL); \
                                                             if (now - start >= KM_CMD_TIMEOUT_IN_SECONDS) { \
                                                                 err = BERR_UNKNOWN; \
                                                                 BDBG_ERR(("%s: command %d timed out", BSTD_FUNCTION, cmd_id)); \
                                                                 goto done; \
                                                             } \
                                                             err = SRAI_Module_ProcessCommand(handle->moduleHandle, cmd_id, inout);\
                                                             if ((err != BERR_SUCCESS) || (inout->basicOut[0] != BERR_SUCCESS)) { \
                                                                 BDBG_MSG(("%s: Command failed (%x/%x)", BSTD_FUNCTION, err, inout->basicOut[0])); \
                                                                 if (err == BERR_SUCCESS) { err = inout->basicOut[0]; } \
                                                                 if (err == BSAGE_ERR_KM_AGAIN) { usleep(10000); continue; } \
                                                                 goto done;\
                                                             } else { break; } \
                                                         } \
                                                         break; \
                                                     }


static void _KeymasterTl_ContextDelete(KeymasterTl_Handle hKeymasterTl);
static BERR_Code _KeymasterTl_ContextNew(KeymasterTl_InitSettings *pModuleSettings, KeymasterTl_Handle *ret_handle);

void KeymasterTl_GetDefaultInitSettings(KeymasterTl_InitSettings *pModuleSettings)
{
    BDBG_ENTER(KeymasterTl_GetDefaultInitSettings);

    BDBG_ASSERT(pModuleSettings);
    BKNI_Memset((uint8_t *)pModuleSettings, 0, sizeof(KeymasterTl_InitSettings));
    if (!access(KEYMASTER_DRM_BIN_DEFAULT_FILEPATH, R_OK)) {
       BDBG_WRN(("%s: using keymaster drm bin file %s", BSTD_FUNCTION, KEYMASTER_DRM_BIN_DEFAULT_FILEPATH));
       BKNI_Memcpy(pModuleSettings->drm_binfile_path, KEYMASTER_DRM_BIN_DEFAULT_FILEPATH, strlen(KEYMASTER_DRM_BIN_DEFAULT_FILEPATH));
    } else if (strlen(KEYMASTER_DRM_BIN_ALTERNATE_FILEPATH) && !access(KEYMASTER_DRM_BIN_ALTERNATE_FILEPATH, R_OK)) {
       BDBG_WRN(("%s: using keymaster drm bin file %s", BSTD_FUNCTION, KEYMASTER_DRM_BIN_ALTERNATE_FILEPATH));
       BKNI_Memcpy(pModuleSettings->drm_binfile_path, KEYMASTER_DRM_BIN_ALTERNATE_FILEPATH, strlen(KEYMASTER_DRM_BIN_ALTERNATE_FILEPATH));
    } else {
       BDBG_ERR(("%s: no valid keybox data, service failure expected", BSTD_FUNCTION));
    }
    BDBG_LEAVE(KeymasterTl_GetDefaultInitSettings);
    return;
}

static BERR_Code _KeymasterTl_ContextNew(KeymasterTl_InitSettings *pModuleSettings, KeymasterTl_Handle *ret_handle)
{
    BERR_Code err;
    NEXUS_Error nexus_rc;
    BSAGElib_InOutContainer *inout = NULL;
    KeymasterTl_Handle handle = NULL;

    BDBG_ASSERT(ret_handle);

    nexus_rc = NEXUS_Memory_Allocate(sizeof(*handle), NULL, (void **)&handle);
    if (nexus_rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: cannot allocate memory for context", BSTD_FUNCTION));
        err = BERR_OUT_OF_DEVICE_MEMORY;
        goto done;
    }
    handle->moduleHandle = NULL;
    handle->ssdModuleHandle = NULL;

    BDBG_OBJECT_SET(handle, KeymasterTl_Instance);

    if (!pModuleSettings->version) {
        handle->configuredKeymasterVersion = SKM_VERSION_3;
    } else {
        handle->configuredKeymasterVersion = pModuleSettings->version;
    }
    if ((handle->configuredKeymasterVersion != SKM_VERSION_3) && (handle->configuredKeymasterVersion != SKM_VERSION_4)) {
        BDBG_ERR(("%s: Invalid version", BSTD_FUNCTION));
        err = BERR_INVALID_PARAMETER;
        goto done;
    }
    /* Rewrite the version to reflect the configured one */
    pModuleSettings->version = handle->configuredKeymasterVersion;

    KM_ALLOCATE_CONTAINER();

    err = Keymaster_ModuleInit(Keymaster_ModuleId_eKeymaster,
                               pModuleSettings,
                               inout,
                               &handle->moduleHandle,
                               &handle->ssdModuleHandle);
    if (err != BERR_SUCCESS) {
        BDBG_ERR(("%s: Error initializing module (err 0x%08x, inout 0x%08x)", BSTD_FUNCTION, err, inout->basicOut[0]));
        if ((err == BSAGE_ERR_INTERNAL) && (inout->basicOut[0] != BERR_SUCCESS))
            err = inout->basicOut[0];
        goto done;
    }

done:
    KM_FREE_CONTAINER();

    if (err != BERR_SUCCESS && handle) {
        _KeymasterTl_ContextDelete(handle);
        handle = NULL;
    }

    *ret_handle = handle;
    return err;
}

static void _KeymasterTl_ContextDelete(KeymasterTl_Handle handle)
{
    if (handle) {
        if (handle->moduleHandle || handle->ssdModuleHandle) {
            Keymaster_ModuleUninit(handle->moduleHandle, handle->ssdModuleHandle);
            handle->moduleHandle = NULL;
            handle->ssdModuleHandle = NULL;
        }

        BDBG_OBJECT_DESTROY(handle, KeymasterTl_Instance);
        NEXUS_Memory_Free(handle);
    }
}

BERR_Code KeymasterTl_Init(KeymasterTl_Handle *pHandle, KeymasterTl_InitSettings *pModuleSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    KeymasterTl_InitSettings localSettings;

    BDBG_ENTER(KeymasterTl_Init);

    if (!pModuleSettings) {
        KeymasterTl_GetDefaultInitSettings(&localSettings);
        pModuleSettings = &localSettings;
    }

    if (!pHandle) {
        BDBG_ERR(("%s: Parameter pHandle is NULL", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto done;
    }

    rc = _KeymasterTl_ContextNew(pModuleSettings, pHandle);
    if (rc != BERR_SUCCESS) {
        *pHandle = NULL;
        goto done;
    }

done:

    BDBG_LEAVE(KeymasterTl_Init);
    return rc;
}

/* Shutdown the processing in Keymaster */
static void _KeymasterTl_Shutdown(KeymasterTl_Handle handle)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;

    BDBG_ENTER(_KeymasterTl_Shutdown);

    BDBG_ASSERT(handle);
    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);
    KM_ALLOCATE_CONTAINER();
    KM_PROCESS_COMMAND(KM_CommandId_eShutdown);

done:
    KM_FREE_CONTAINER();
    BDBG_LEAVE(_KeymasterTl_Shutdown);
    return;
}

void KeymasterTl_Uninit(KeymasterTl_Handle handle)
{
    BDBG_ENTER(KeymasterTl_Uninit);

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);
    _KeymasterTl_Shutdown(handle);
    _KeymasterTl_ContextDelete(handle);

    BDBG_LEAVE(KeymasterTl_Uninit);
    return;
}


static BERR_Code _KeymasterTl_SerializeParams(KM_Tag_ContextHandle params, uint8_t **data, uint32_t *data_length, bool allow_empty)
{
    BERR_Code err;
    uint32_t length = 0;

    BDBG_ASSERT(data);
    BDBG_ASSERT(data_length);

    *data = NULL;
    *data_length = 0;

    if (!params) {
        if (allow_empty) {
            err = BERR_SUCCESS;
            goto done;
        }

        BDBG_ERR(("%s: Invalid params", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    /* Call serialize to calculate the buffer size required */
    err = KM_Tag_Serialize(params, NULL, &length);
    if (err != BERR_SUCCESS) {
        BDBG_ERR(("%s: Failed to calculate tag size", BSTD_FUNCTION));
        goto done;
    }
    BDBG_MSG(("%s length %d", BSTD_FUNCTION, length));
    if (!length || !KM_Tag_GetNumPairs(params)) {
        if (allow_empty) {
            err = BERR_SUCCESS;
            goto done;
        }

        BDBG_ERR(("%s: Invalid params", BSTD_FUNCTION));
        err = BERR_INVALID_PARAMETER;
        goto done;
    }
    KM_ALLOCATE_BLOCK(*data, length);

    /* Call it again to fill in the newly allocated buffer */
    err = KM_Tag_Serialize(params, *data, &length);
    if (err != BERR_SUCCESS) {
        BDBG_ERR(("%s: Failed to calculate tag size", BSTD_FUNCTION));
        goto done;
    }
    BDBG_MSG(("%s serialized block length %d", BSTD_FUNCTION, length));
    *data_length = length;

done:
    if (err != BERR_SUCCESS) {
        KM_FREE_BLOCK(*data);
    }
    return err;
}


BERR_Code KeymasterTl_Configure(
    KeymasterTl_Handle handle,
    uint32_t vendor_patchlevel,
    KM_Tag_ContextHandle in_params)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint32_t length = 0;
    uint8_t *params = NULL;

    BDBG_ENTER(KeymasterTl_Configure);

    if (!handle || !in_params) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    err = _KeymasterTl_SerializeParams(in_params, &params, &length, false);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    KM_CMD_BASIC_CONFIGURE_IN_PARAMS_PTR = params;
    KM_CMD_BASIC_CONFIGURE_IN_PARAMS_LEN = length;
    KM_CMD_BASIC_CONFIGURE_IN_NUM_PARAMS = KM_Tag_GetNumPairs(in_params);
    if (handle->configuredKeymasterVersion >= SKM_VERSION_4) {
        KM_CMD_BASIC_CONFIGURE_IN_VENDOR_PATCHLEVEL = vendor_patchlevel;
    }
    KM_PROCESS_COMMAND(KM_CommandId_eConfigure);

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(params);
    BDBG_LEAVE(KeymasterTl_Configure);
    return err;
}

BERR_Code KeymasterTl_AddRngEntropy(
    KeymasterTl_Handle handle,
    KeymasterTl_DataBlock *in_data)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *in_data_ptr = NULL;

    BDBG_ENTER(KeymasterTl_AddRngEntropy);

    if (!handle || !in_data || !in_data->size || !in_data->buffer) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    KM_ALLOCATE_BLOCK(in_data_ptr, in_data->size);
    BKNI_Memcpy(in_data_ptr, in_data->buffer, in_data->size);
    KM_CMD_BASIC_ADD_RNG_ENTROPY_IN_DATA_PTR = in_data_ptr;
    KM_CMD_BASIC_ADD_RNG_ENTROPY_IN_DATA_LEN = in_data->size;

    KM_PROCESS_COMMAND(KM_CommandId_eAddRngEntropy);

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(in_data_ptr);
    BDBG_LEAVE(KeymasterTl_AddRngEntropy);
    return err;
}

BERR_Code KeymasterTl_GenerateKey(
    KeymasterTl_Handle handle,
    KM_Tag_ContextHandle in_key_params,
    KeymasterTl_DataBlock *out_key_blob)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *key_params = NULL;
    uint32_t length = 0;
    uint8_t *nonce = NULL;
    uint32_t key_blob_len = 0;
    uint8_t *key_blob = NULL;

    BDBG_ENTER(KeymasterTl_GenerateKey);

    if (!handle || !in_key_params || !out_key_blob) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);
    BKNI_Memset(out_key_blob, 0, sizeof(KeymasterTl_DataBlock));

    KM_ALLOCATE_CONTAINER();

    err = _KeymasterTl_SerializeParams(in_key_params, &key_params, &length, false);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    KM_CMD_GENERATE_START_IN_KEY_PARAMS_PTR = key_params;
    KM_CMD_GENERATE_START_IN_KEY_PARAMS_LEN = length;
    KM_CMD_GENERATE_START_IN_NUM_KEY_PARAMS = KM_Tag_GetNumPairs(in_key_params);

    KM_ALLOCATE_BLOCK(nonce, sizeof(km_secure_nonce_t));
    KM_CMD_GENERATE_START_OUT_NONCE_PTR = nonce;
    KM_CMD_GENERATE_START_OUT_NONCE_LEN = sizeof(km_secure_nonce_t);

    KM_PROCESS_COMMAND(KM_CommandId_eGenerateKeyStart);

    key_blob_len = KM_CMD_GENERATE_START_OUT_KEY_BLOB_LEN;

    if (!key_blob_len) {
        BDBG_ERR(("%s: generate start returned key blob length of zero", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }

    /* Call complete */
    BKNI_Memset(inout, 0, sizeof(BSAGElib_InOutContainer));
    KM_CMD_GENERATE_COMPLETE_IN_NONCE_PTR = nonce;
    KM_CMD_GENERATE_COMPLETE_IN_NONCE_LEN = sizeof(km_secure_nonce_t);
    KM_CMD_GENERATE_COMPLETE_IN_KEY_PARAMS_PTR = key_params;
    KM_CMD_GENERATE_COMPLETE_IN_KEY_PARAMS_LEN = length;
    KM_CMD_GENERATE_COMPLETE_IN_NUM_KEY_PARAMS = KM_Tag_GetNumPairs(in_key_params);
    KM_ALLOCATE_BLOCK(key_blob, key_blob_len);
    KM_CMD_GENERATE_COMPLETE_OUT_KEY_BLOB_PTR = key_blob;
    KM_CMD_GENERATE_COMPLETE_OUT_KEY_BLOB_LEN = key_blob_len;

    KM_PROCESS_COMMAND(KM_CommandId_eGenerateKeyComplete);

    if ((uint32_t)KM_CMD_GENERATE_COMPLETE_OUT_RET_KEY_BLOB_LEN != key_blob_len) {
        BDBG_WRN(("%s: key blob changed size", BSTD_FUNCTION));
    }

    KM_ALLOCATE_BLOCK(out_key_blob->buffer, KM_CMD_GENERATE_COMPLETE_OUT_RET_KEY_BLOB_LEN);
    BKNI_Memcpy(out_key_blob->buffer, key_blob, KM_CMD_GENERATE_COMPLETE_OUT_RET_KEY_BLOB_LEN);
    out_key_blob->size = KM_CMD_GENERATE_COMPLETE_OUT_RET_KEY_BLOB_LEN;
    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(key_params);
    KM_FREE_BLOCK(nonce);
    KM_FREE_BLOCK(key_blob);
    BDBG_LEAVE(KeymasterTl_GenerateKey);
    return err;
}

void KeymasterTl_GetDefaultKeyCharacteristicsSettings(
    KeymasterTl_GetKeyCharacteristicsSettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
}

BERR_Code KeymasterTl_GetKeyCharacteristics(
    KeymasterTl_Handle handle,
    KeymasterTl_GetKeyCharacteristicsSettings *settings)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *key_blob = NULL;
    uint8_t *params = NULL;
    uint32_t length = 0;
    uint8_t *hw_ptr = NULL;
    uint8_t *sw_ptr = NULL;
    KM_Tag_ContextHandle hw_enforced = NULL;
    KM_Tag_ContextHandle sw_enforced = NULL;

    BDBG_ENTER(KeymasterTl_GetKeyCharacteristics);

    if (!handle || !settings || !settings->in_key_blob.size ||
        !settings->in_key_blob.buffer) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    KM_ALLOCATE_BLOCK(key_blob, settings->in_key_blob.size);
    BKNI_Memcpy(key_blob, settings->in_key_blob.buffer, settings->in_key_blob.size);
    KM_CMD_GET_CHARACTERISTICS_IN_KEY_BLOB_PTR = key_blob;
    KM_CMD_GET_CHARACTERISTICS_IN_KEY_BLOB_LEN = settings->in_key_blob.size;

    /* in_params can be NULL or empty with GetCharacteristics */
    err = _KeymasterTl_SerializeParams(settings->in_params, &params, &length, true);
    if (err != BERR_SUCCESS) {
        goto done;
    }

    KM_CMD_GET_CHARACTERISTICS_IN_PARAMS_PTR = params;
    KM_CMD_GET_CHARACTERISTICS_IN_PARAMS_LEN = length;
    KM_CMD_GET_CHARACTERISTICS_IN_NUM_PARAMS = (!settings->in_params) ? 0 : KM_Tag_GetNumPairs(settings->in_params);

    KM_ALLOCATE_BLOCK(hw_ptr, SKM_TAG_VALUE_BLOCK_SIZE);
    KM_CMD_GET_CHARACTERISTICS_OUT_HW_ENFORCED_PTR = hw_ptr;
    KM_CMD_GET_CHARACTERISTICS_OUT_HW_ENFORCED_LEN = SKM_TAG_VALUE_BLOCK_SIZE;
    KM_ALLOCATE_BLOCK(sw_ptr, SKM_TAG_VALUE_BLOCK_SIZE);
    KM_CMD_GET_CHARACTERISTICS_OUT_SW_ENFORCED_PTR = sw_ptr;
    KM_CMD_GET_CHARACTERISTICS_OUT_SW_ENFORCED_LEN = SKM_TAG_VALUE_BLOCK_SIZE;

    KM_PROCESS_COMMAND(KM_CommandId_eGetKeyCharacteristics);

    err = KM_Tag_CreateContextFromTagValueSet(hw_ptr, &hw_enforced);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    err = KM_Tag_CreateContextFromTagValueSet(sw_ptr, &sw_enforced);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    settings->out_hw_enforced = hw_enforced;
    settings->out_sw_enforced = sw_enforced;
    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(key_blob);
    KM_FREE_BLOCK(params);
    KM_FREE_BLOCK(hw_ptr);
    KM_FREE_BLOCK(sw_ptr);
    if (err != BERR_SUCCESS) {
        if (hw_enforced) {
            KM_Tag_DeleteContext(hw_enforced);
        }
        if (sw_enforced) {
            KM_Tag_DeleteContext(sw_enforced);
        }
    }
    BDBG_LEAVE(KeymasterTl_GetKeyCharacteristics);
    return err;
}

void KeymasterTl_GetDefaultImportKeySettings(
    KeymasterTl_ImportKeySettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
}

static BERR_Code kmtl_import_key_1_step(
    KeymasterTl_Handle handle,
    KeymasterTl_ImportKeySettings *settings)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *key_params = NULL;
    uint32_t length = 0;
    uint8_t *in_key_blob_ptr = NULL;
    uint8_t *key_blob = NULL;

    BDBG_ENTER(kmtl_import_key_1_step);

    if (!handle || !settings || !settings->in_key_params ||
        !settings->in_key_blob.size || !settings->in_key_blob.buffer) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    err = _KeymasterTl_SerializeParams(settings->in_key_params, &key_params, &length, false);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    KM_CMD_IMPORT_IN_KEY_PARAMS_PTR = key_params;
    KM_CMD_IMPORT_IN_KEY_PARAMS_LEN = length;
    KM_CMD_IMPORT_IN_NUM_KEY_PARAMS = KM_Tag_GetNumPairs(settings->in_key_params);
    KM_CMD_IMPORT_IN_KEY_FORMAT = (uint32_t)settings->in_key_format;

    KM_ALLOCATE_BLOCK(in_key_blob_ptr, settings->in_key_blob.size);
    KM_CMD_IMPORT_IN_KEY_BLOB_PTR = in_key_blob_ptr;
    KM_CMD_IMPORT_IN_KEY_BLOB_LEN = settings->in_key_blob.size;
    BKNI_Memcpy(in_key_blob_ptr, settings->in_key_blob.buffer, settings->in_key_blob.size);

    KM_ALLOCATE_BLOCK(key_blob, SKM_MAX_KEY_BLOB_SIZE);
    KM_CMD_IMPORT_OUT_KEY_BLOB_PTR = key_blob;
    KM_CMD_IMPORT_OUT_KEY_BLOB_LEN = SKM_MAX_KEY_BLOB_SIZE;

    KM_PROCESS_COMMAND(KM_CommandId_eImportKey);

    KM_ALLOCATE_BLOCK(settings->out_key_blob.buffer, KM_CMD_IMPORT_OUT_RET_KEY_BLOB_LEN);
    BKNI_Memcpy(settings->out_key_blob.buffer, key_blob, KM_CMD_IMPORT_OUT_RET_KEY_BLOB_LEN);
    settings->out_key_blob.size = KM_CMD_IMPORT_COMPLETE_OUT_RET_KEY_BLOB_LEN;
    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(key_params);
    KM_FREE_BLOCK(key_blob);
    if (in_key_blob_ptr) {
        /* Scrub the imported key blob */
        BKNI_Memset(in_key_blob_ptr, 0, settings->in_key_blob.size);
        SRAI_Memory_Free(in_key_blob_ptr);
    }
    BDBG_LEAVE(kmtl_import_key_1_step);
    return err;
}

BERR_Code KeymasterTl_ImportKey(
    KeymasterTl_Handle handle,
    KeymasterTl_ImportKeySettings *settings)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *key_params = NULL;
    uint32_t length = 0;
    uint8_t *nonce = NULL;
    uint8_t *in_key_blob_ptr = NULL;
    uint32_t key_blob_len = 0;
    uint8_t *key_blob = NULL;

    if (handle->configuredKeymasterVersion >= SKM_VERSION_4) {
        return kmtl_import_key_1_step(handle, settings);
    }

    BDBG_ENTER(KeymasterTl_ImportKey);

    if (!handle || !settings || !settings->in_key_params ||
        !settings->in_key_blob.size || !settings->in_key_blob.buffer) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    err = _KeymasterTl_SerializeParams(settings->in_key_params, &key_params, &length, false);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    KM_CMD_IMPORT_START_IN_KEY_PARAMS_PTR = key_params;
    KM_CMD_IMPORT_START_IN_KEY_PARAMS_LEN = length;
    KM_CMD_IMPORT_START_IN_NUM_KEY_PARAMS = KM_Tag_GetNumPairs(settings->in_key_params);
    KM_CMD_IMPORT_START_IN_KEY_FORMAT = (uint32_t)settings->in_key_format;

    KM_ALLOCATE_BLOCK(in_key_blob_ptr, settings->in_key_blob.size);
    KM_CMD_IMPORT_START_IN_KEY_BLOB_PTR = in_key_blob_ptr;
    KM_CMD_IMPORT_START_IN_KEY_BLOB_LEN = settings->in_key_blob.size;
    BKNI_Memcpy(in_key_blob_ptr, settings->in_key_blob.buffer, settings->in_key_blob.size);

    KM_ALLOCATE_BLOCK(nonce, sizeof(km_secure_nonce_t));
    KM_CMD_IMPORT_START_OUT_NONCE_PTR = nonce;
    KM_CMD_IMPORT_START_OUT_NONCE_LEN = sizeof(km_secure_nonce_t);

    KM_PROCESS_COMMAND(KM_CommandId_eImportKeyStart);

    key_blob_len = KM_CMD_IMPORT_START_OUT_KEY_BLOB_LEN;

    if (!key_blob_len) {
        BDBG_ERR(("%s: import start returned key blob length of zero", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }

    /* Call complete */
    BKNI_Memset(inout, 0, sizeof(BSAGElib_InOutContainer));
    KM_CMD_IMPORT_COMPLETE_IN_NONCE_PTR = nonce;
    KM_CMD_IMPORT_COMPLETE_IN_NONCE_LEN = sizeof(km_secure_nonce_t);
    KM_CMD_IMPORT_COMPLETE_IN_KEY_PARAMS_PTR = key_params;
    KM_CMD_IMPORT_COMPLETE_IN_KEY_PARAMS_LEN = length;
    KM_CMD_IMPORT_COMPLETE_IN_NUM_KEY_PARAMS = KM_Tag_GetNumPairs(settings->in_key_params);
    KM_CMD_IMPORT_COMPLETE_IN_KEY_FORMAT = (uint32_t)settings->in_key_format;
    KM_CMD_IMPORT_COMPLETE_IN_KEY_BLOB_PTR = in_key_blob_ptr;
    KM_CMD_IMPORT_COMPLETE_IN_KEY_BLOB_LEN = settings->in_key_blob.size;
    KM_ALLOCATE_BLOCK(key_blob, key_blob_len);
    KM_CMD_IMPORT_COMPLETE_OUT_KEY_BLOB_PTR = key_blob;
    KM_CMD_IMPORT_COMPLETE_OUT_KEY_BLOB_LEN = key_blob_len;

    KM_PROCESS_COMMAND(KM_CommandId_eImportKeyComplete);

    if ((uint32_t)KM_CMD_IMPORT_COMPLETE_OUT_RET_KEY_BLOB_LEN != key_blob_len) {
        BDBG_WRN(("%s: key blob changed size", BSTD_FUNCTION));
    }

    KM_ALLOCATE_BLOCK(settings->out_key_blob.buffer, KM_CMD_IMPORT_COMPLETE_OUT_RET_KEY_BLOB_LEN);
    BKNI_Memcpy(settings->out_key_blob.buffer, key_blob, KM_CMD_IMPORT_COMPLETE_OUT_RET_KEY_BLOB_LEN);
    settings->out_key_blob.size = KM_CMD_IMPORT_COMPLETE_OUT_RET_KEY_BLOB_LEN;
    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(key_params);
    KM_FREE_BLOCK(nonce);
    KM_FREE_BLOCK(key_blob);
    if (in_key_blob_ptr) {
        /* Scrub the imported key blob */
        BKNI_Memset(in_key_blob_ptr, 0, settings->in_key_blob.size);
        SRAI_Memory_Free(in_key_blob_ptr);
    }
    BDBG_LEAVE(KeymasterTl_ImportKey);
    return err;
}

void KeymasterTl_GetDefaultExportKeySettings(
    KeymasterTl_ExportKeySettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
}

BERR_Code KeymasterTl_ExportKey(
    KeymasterTl_Handle handle,
    KeymasterTl_ExportKeySettings *settings)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *key_blob = NULL;
    uint8_t *params = NULL;
    uint32_t length = 0;
    uint8_t *nonce = NULL;
    uint32_t key_blob_len = 0;
    uint8_t *ret_key_blob = NULL;

    BDBG_ENTER(KeymasterTl_ExportKey);

    if (!handle || !settings || !settings->in_key_blob.size ||
        !settings->in_key_blob.buffer) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    KM_CMD_EXPORT_START_IN_EXPORT_FORMAT = (uint32_t)settings->in_key_format;

    KM_ALLOCATE_BLOCK(key_blob, settings->in_key_blob.size);
    BKNI_Memcpy(key_blob, settings->in_key_blob.buffer, settings->in_key_blob.size);
    KM_CMD_EXPORT_START_IN_KEY_BLOB_PTR = key_blob;
    KM_CMD_EXPORT_START_IN_KEY_BLOB_LEN = settings->in_key_blob.size;

    /* in_params can be NULL or empty with ExportKey */
    err = _KeymasterTl_SerializeParams(settings->in_params, &params, &length, true);
    if (err != BERR_SUCCESS) {
        goto done;
    }

    KM_CMD_EXPORT_START_IN_PARAMS_PTR = params;
    KM_CMD_EXPORT_START_IN_PARAMS_LEN = length;
    KM_CMD_EXPORT_START_IN_NUM_PARAMS = (!settings->in_params) ? 0 : KM_Tag_GetNumPairs(settings->in_params);

    KM_ALLOCATE_BLOCK(nonce, sizeof(km_secure_nonce_t));
    KM_CMD_EXPORT_START_OUT_NONCE_PTR = nonce;
    KM_CMD_EXPORT_START_OUT_NONCE_LEN = sizeof(km_secure_nonce_t);

    KM_PROCESS_COMMAND(KM_CommandId_eExportKeyStart);

    key_blob_len = KM_CMD_EXPORT_START_OUT_KEY_BLOB_LEN;

    if (!key_blob_len) {
        BDBG_ERR(("%s: export start returned key blob length of zero", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }

    /* Call complete */
    BKNI_Memset(inout, 0, sizeof(BSAGElib_InOutContainer));
    KM_CMD_EXPORT_COMPLETE_IN_NONCE_PTR = nonce;
    KM_CMD_EXPORT_COMPLETE_IN_NONCE_LEN = sizeof(km_secure_nonce_t);
    KM_CMD_EXPORT_COMPLETE_IN_EXPORT_FORMAT = (uint32_t)settings->in_key_format;
    KM_CMD_EXPORT_COMPLETE_IN_KEY_BLOB_PTR = key_blob;
    KM_CMD_EXPORT_COMPLETE_IN_KEY_BLOB_LEN = settings->in_key_blob.size;
    KM_CMD_EXPORT_COMPLETE_IN_PARAMS_PTR = params;
    KM_CMD_EXPORT_COMPLETE_IN_PARAMS_LEN = length;
    KM_CMD_EXPORT_COMPLETE_IN_NUM_PARAMS = (!settings->in_params) ? 0 : KM_Tag_GetNumPairs(settings->in_params);
    KM_ALLOCATE_BLOCK(ret_key_blob, key_blob_len);
    KM_CMD_EXPORT_COMPLETE_OUT_KEY_BLOB_PTR = ret_key_blob;
    KM_CMD_EXPORT_COMPLETE_OUT_KEY_BLOB_LEN = key_blob_len;

    KM_PROCESS_COMMAND(KM_CommandId_eExportKeyComplete);

    if ((uint32_t)KM_CMD_EXPORT_COMPLETE_OUT_RET_KEY_BLOB_LEN != key_blob_len) {
        BDBG_WRN(("%s: key blob changed size", BSTD_FUNCTION));
    }

    KM_ALLOCATE_BLOCK(settings->out_key_blob.buffer, KM_CMD_EXPORT_COMPLETE_OUT_RET_KEY_BLOB_LEN);
    BKNI_Memcpy(settings->out_key_blob.buffer, ret_key_blob, KM_CMD_EXPORT_COMPLETE_OUT_RET_KEY_BLOB_LEN);
    settings->out_key_blob.size = KM_CMD_EXPORT_COMPLETE_OUT_RET_KEY_BLOB_LEN;
    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(key_blob);
    KM_FREE_BLOCK(params);
    KM_FREE_BLOCK(nonce);
    KM_FREE_BLOCK(ret_key_blob);
    BDBG_LEAVE(KeymasterTl_ExportKey);
    return err;
}

void KeymasterTl_GetDefaultAttestKeySettings(
    KeymasterTl_AttestKeySettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
}

BERR_Code KeymasterTl_AttestKey(
    KeymasterTl_Handle handle,
    KeymasterTl_AttestKeySettings *settings)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *params = NULL;
    uint32_t length = 0;
    uint8_t *nonce = NULL;
    uint8_t *in_key_blob_ptr = NULL;
    uint32_t cert_chain_buf_len = 0;
    uint8_t *cert_chain_buf = NULL;
    uint8_t *cert_chain = NULL;

    BDBG_ENTER(KeymasterTl_AttestKey);

    if (!handle || !settings || !settings->in_key_blob.size || !settings->in_key_blob.buffer) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    KM_ALLOCATE_BLOCK(in_key_blob_ptr, settings->in_key_blob.size);
    KM_CMD_ATTEST_START_IN_KEY_BLOB_PTR = in_key_blob_ptr;
    KM_CMD_ATTEST_START_IN_KEY_BLOB_LEN = settings->in_key_blob.size;
    BKNI_Memcpy(in_key_blob_ptr, settings->in_key_blob.buffer, settings->in_key_blob.size);

    /* in_params can be NULL or empty with AttestKey (only app_id and app_data is stored in it) */
    err = _KeymasterTl_SerializeParams(settings->in_params, &params, &length, true);
    if (err != BERR_SUCCESS) {
        goto done;
    }

    KM_CMD_ATTEST_START_IN_KEY_PARAMS_PTR = params;
    KM_CMD_ATTEST_START_IN_KEY_PARAMS_LEN = length;
    KM_CMD_ATTEST_START_IN_NUM_KEY_PARAMS = (!settings->in_params) ? 0 : KM_Tag_GetNumPairs(settings->in_params);

    KM_ALLOCATE_BLOCK(nonce, sizeof(km_secure_nonce_t));
    KM_CMD_ATTEST_START_OUT_NONCE_PTR = nonce;
    KM_CMD_ATTEST_START_OUT_NONCE_LEN = sizeof(km_secure_nonce_t);

    KM_PROCESS_COMMAND(KM_CommandId_eAttestKeyStart);

    cert_chain_buf_len = KM_CMD_ATTEST_START_OUT_CERT_CHAIN_BUFFER_LEN;

    if (!cert_chain_buf_len) {
        BDBG_ERR(("%s: attest start returned chain buf length of zero", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }

    /* Call complete */
    BKNI_Memset(inout, 0, sizeof(BSAGElib_InOutContainer));
    KM_CMD_ATTEST_COMPLETE_IN_NONCE_PTR = nonce;
    KM_CMD_ATTEST_COMPLETE_IN_NONCE_LEN = sizeof(km_secure_nonce_t);
    KM_CMD_ATTEST_COMPLETE_IN_KEY_BLOB_PTR = in_key_blob_ptr;
    KM_CMD_ATTEST_COMPLETE_IN_KEY_BLOB_LEN = settings->in_key_blob.size;
    KM_CMD_ATTEST_COMPLETE_IN_KEY_PARAMS_PTR = params;
    KM_CMD_ATTEST_COMPLETE_IN_KEY_PARAMS_LEN = length;
    KM_CMD_ATTEST_COMPLETE_IN_NUM_KEY_PARAMS = (!settings->in_params) ? 0 : KM_Tag_GetNumPairs(settings->in_params);
    KM_ALLOCATE_BLOCK(cert_chain, sizeof(km_cert_chain_t));
    KM_CMD_ATTEST_COMPLETE_OUT_CERT_CHAIN_PTR = cert_chain;
    KM_CMD_ATTEST_COMPLETE_OUT_CERT_CHAIN_LEN = sizeof(km_cert_chain_t);
    KM_ALLOCATE_BLOCK(cert_chain_buf, cert_chain_buf_len);
    KM_CMD_ATTEST_COMPLETE_OUT_CERT_CHAIN_BUFFER_PTR = cert_chain_buf;
    KM_CMD_ATTEST_COMPLETE_OUT_CERT_CHAIN_BUFFER_LEN = cert_chain_buf_len;

    KM_PROCESS_COMMAND(KM_CommandId_eAttestKeyComplete);

    if ((uint32_t)KM_CMD_ATTEST_COMPLETE_OUT_RET_CERT_CHAIN_BUFFER_LEN != cert_chain_buf_len) {
        BDBG_WRN(("%s: cert chain buf changed size", BSTD_FUNCTION));
    }

    BKNI_Memcpy(&settings->out_cert_chain, cert_chain, sizeof(km_cert_chain_t));
    KM_ALLOCATE_BLOCK(settings->out_cert_chain_buffer.buffer, KM_CMD_ATTEST_COMPLETE_OUT_RET_CERT_CHAIN_BUFFER_LEN);
    BKNI_Memcpy(settings->out_cert_chain_buffer.buffer, cert_chain_buf, KM_CMD_ATTEST_COMPLETE_OUT_RET_CERT_CHAIN_BUFFER_LEN);
    settings->out_cert_chain_buffer.size = KM_CMD_ATTEST_COMPLETE_OUT_RET_CERT_CHAIN_BUFFER_LEN;
    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(params);
    KM_FREE_BLOCK(in_key_blob_ptr);
    KM_FREE_BLOCK(nonce);
    KM_FREE_BLOCK(cert_chain_buf);
    KM_FREE_BLOCK(cert_chain);
    BDBG_LEAVE(KeymasterTl_AttestKey);
    return err;
}

BERR_Code KeymasterTl_UpgradeKey(
    KeymasterTl_Handle handle,
    KeymasterTl_DataBlock *in_key_blob,
    KM_Tag_ContextHandle in_params,
    KeymasterTl_DataBlock *out_key_blob)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *params = NULL;
    uint32_t length = 0;
    uint8_t *nonce = NULL;
    uint8_t *in_key_blob_ptr = NULL;
    uint32_t key_blob_len = 0;
    uint8_t *key_blob = NULL;

    BDBG_ENTER(KeymasterTl_UpgradeKey);

    if (!handle || !in_key_blob || !in_key_blob->size || !in_key_blob->buffer || !out_key_blob) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);
    BKNI_Memset(out_key_blob, 0, sizeof(KeymasterTl_DataBlock));

    KM_ALLOCATE_CONTAINER();

    KM_ALLOCATE_BLOCK(in_key_blob_ptr, in_key_blob->size);
    KM_CMD_UPGRADE_START_IN_KEY_BLOB_PTR = in_key_blob_ptr;
    KM_CMD_UPGRADE_START_IN_KEY_BLOB_LEN = in_key_blob->size;
    BKNI_Memcpy(in_key_blob_ptr, in_key_blob->buffer, in_key_blob->size);

    /* in_params can be NULL or empty with UpgradeKey (only app_id and app_data is stored in it) */
    err = _KeymasterTl_SerializeParams(in_params, &params, &length, true);
    if (err != BERR_SUCCESS) {
        goto done;
    }

    KM_CMD_UPGRADE_START_IN_KEY_PARAMS_PTR = params;
    KM_CMD_UPGRADE_START_IN_KEY_PARAMS_LEN = length;
    KM_CMD_UPGRADE_START_IN_NUM_KEY_PARAMS = (!in_params) ? 0 : KM_Tag_GetNumPairs(in_params);

    KM_ALLOCATE_BLOCK(nonce, sizeof(km_secure_nonce_t));
    KM_CMD_UPGRADE_START_OUT_NONCE_PTR = nonce;
    KM_CMD_UPGRADE_START_OUT_NONCE_LEN = sizeof(km_secure_nonce_t);

    KM_PROCESS_COMMAND(KM_CommandId_eUpgradeKeyStart);

    key_blob_len = KM_CMD_UPGRADE_START_OUT_KEY_BLOB_LEN;

    if (!key_blob_len) {
        BDBG_ERR(("%s: upgrade start returned key blob length of zero", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }

    /* Call complete */
    BKNI_Memset(inout, 0, sizeof(BSAGElib_InOutContainer));
    KM_CMD_UPGRADE_COMPLETE_IN_NONCE_PTR = nonce;
    KM_CMD_UPGRADE_COMPLETE_IN_NONCE_LEN = sizeof(km_secure_nonce_t);
    KM_CMD_UPGRADE_COMPLETE_IN_KEY_BLOB_PTR = in_key_blob_ptr;
    KM_CMD_UPGRADE_COMPLETE_IN_KEY_BLOB_LEN = in_key_blob->size;
    KM_CMD_UPGRADE_COMPLETE_IN_KEY_PARAMS_PTR = params;
    KM_CMD_UPGRADE_COMPLETE_IN_KEY_PARAMS_LEN = length;
    KM_CMD_UPGRADE_COMPLETE_IN_NUM_KEY_PARAMS = (!in_params) ? 0 : KM_Tag_GetNumPairs(in_params);
    KM_ALLOCATE_BLOCK(key_blob, key_blob_len);
    KM_CMD_UPGRADE_COMPLETE_OUT_KEY_BLOB_PTR = key_blob;
    KM_CMD_UPGRADE_COMPLETE_OUT_KEY_BLOB_LEN = key_blob_len;

    KM_PROCESS_COMMAND(KM_CommandId_eUpgradeKeyComplete);

    if ((uint32_t)KM_CMD_UPGRADE_COMPLETE_OUT_RET_KEY_BLOB_LEN != key_blob_len) {
        BDBG_WRN(("%s: key blob changed size", BSTD_FUNCTION));
    }

    if (KM_CMD_UPGRADE_COMPLETE_OUT_RET_KEY_BLOB_LEN) {
        KM_ALLOCATE_BLOCK(out_key_blob->buffer, KM_CMD_UPGRADE_COMPLETE_OUT_RET_KEY_BLOB_LEN);
        BKNI_Memcpy(out_key_blob->buffer, key_blob, KM_CMD_UPGRADE_COMPLETE_OUT_RET_KEY_BLOB_LEN);
    } else {
        /* If it returns 0, upgrade was not required */
        out_key_blob->buffer = NULL;
    }
    out_key_blob->size = KM_CMD_UPGRADE_COMPLETE_OUT_RET_KEY_BLOB_LEN;
    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(params);
    KM_FREE_BLOCK(in_key_blob_ptr);
    KM_FREE_BLOCK(nonce);
    KM_FREE_BLOCK(key_blob);
    BDBG_LEAVE(KeymasterTl_UpgradeKey);
    return err;
}

BERR_Code KeymasterTl_DeleteKey(
    KeymasterTl_Handle handle,
    KeymasterTl_DataBlock *in_key_blob)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *in_key_blob_ptr = NULL;

    BDBG_ENTER(KeymasterTl_DeleteKey);

    if (!handle || !in_key_blob || !in_key_blob->size || !in_key_blob->buffer) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    KM_ALLOCATE_BLOCK(in_key_blob_ptr, in_key_blob->size);
    KM_CMD_DELETE_KEY_IN_KEY_BLOB_PTR = in_key_blob_ptr;
    KM_CMD_DELETE_KEY_IN_KEY_BLOB_LEN = in_key_blob->size;
    BKNI_Memcpy(in_key_blob_ptr, in_key_blob->buffer, in_key_blob->size);

    KM_PROCESS_COMMAND(KM_CommandId_eDeleteKey);

    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(in_key_blob_ptr);
    BDBG_LEAVE(KeymasterTl_DeleteKey);
    return err;
}

BERR_Code KeymasterTl_DeleteAllKeys(KeymasterTl_Handle handle)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;

    BDBG_ENTER(KeymasterTl_DeleteAllKeys);

    if (!handle) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BERR_INVALID_PARAMETER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    KM_PROCESS_COMMAND(KM_CommandId_eDeleteAllKeys);

    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    BDBG_LEAVE(KeymasterTl_DeleteAllKeys);
    return err;
}

BERR_Code KeymasterTl_DestroyAttestationIds(KeymasterTl_Handle handle)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;

    BDBG_ENTER(KeymasterTl_DestroyAttestationIds);

    if (!handle) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BERR_INVALID_PARAMETER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    KM_PROCESS_COMMAND(KM_CommandId_eDestroyAttestationIds);

    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    BDBG_LEAVE(KeymasterTl_DestroyAttestationIds);
    return err;
}

void KeymasterTl_GetDefaultCryptoBeginSettings(
    KeymasterTl_CryptoBeginSettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
}

BERR_Code KeymasterTl_CryptoBegin(
    KeymasterTl_Handle handle,
    KeymasterTl_CryptoBeginSettings *settings,
    km_operation_handle_t *out_operation_handle)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *params = NULL;
    uint32_t length = 0;
    uint8_t *in_key_blob_ptr = NULL;
    uint8_t *ret_params = NULL;
    uint8_t *op_handle = NULL;
    KM_Tag_ContextHandle ret_context = NULL;
    bool command_completed = false;

    BDBG_ENTER(KeymasterTl_CryptoBegin);

    /* Key slot handle can be null, so not checked */
    if (!handle || !settings || !settings->in_key_blob.size ||
        !settings->in_key_blob.buffer || !out_operation_handle) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    *out_operation_handle = 0;

    KM_ALLOCATE_CONTAINER();

    KM_CMD_CRYPTO_BEGIN_IN_PURPOSE = (uint32_t)settings->purpose;
    KM_ALLOCATE_BLOCK(in_key_blob_ptr, settings->in_key_blob.size);
    KM_CMD_CRYPTO_BEGIN_IN_KEY_BLOB_PTR = in_key_blob_ptr;
    KM_CMD_CRYPTO_BEGIN_IN_KEY_BLOB_LEN = settings->in_key_blob.size;
    BKNI_Memcpy(in_key_blob_ptr, settings->in_key_blob.buffer, settings->in_key_blob.size);

    if (settings->hKeyslot) {
#if (NEXUS_SECURITY_API_VERSION==1)
        NEXUS_SecurityKeySlotInfo keyslotInfo;

        NEXUS_Security_GetKeySlotInfo(settings->hKeyslot, &keyslotInfo);
        if (keyslotInfo.keySlotEngine != NEXUS_SecurityEngine_eM2m) {
            BDBG_ERR(("%s: Key slot must be M2M", BSTD_FUNCTION));
            err = BERR_INVALID_PARAMETER;
            goto done;
        }

        KM_CMD_CRYPTO_BEGIN_IN_KEY_SLOT_NUMBER = keyslotInfo.keySlotNumber;
#else
        NEXUS_KeySlotInformation keyslotInfo;

        NEXUS_KeySlot_GetInformation(settings->hKeyslot, &keyslotInfo);
        if (keyslotInfo.slotType != NEXUS_KeySlotType_eIvPerSlot) {
            BDBG_ERR(("%s: Key slot must be IvPerSlot", BSTD_FUNCTION));
            err = BERR_INVALID_PARAMETER;
            goto done;
        }

        KM_CMD_CRYPTO_BEGIN_IN_KEY_SLOT_NUMBER = keyslotInfo.slotNumber;
#endif
    } else {
        KM_CMD_CRYPTO_BEGIN_IN_KEY_SLOT_NUMBER = 0xffffffff;
    }

    err = _KeymasterTl_SerializeParams(settings->in_params, &params, &length, true);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    KM_CMD_CRYPTO_BEGIN_IN_PARAMS_PTR = params;
    KM_CMD_CRYPTO_BEGIN_IN_PARAMS_LEN = length;
    KM_CMD_CRYPTO_BEGIN_IN_NUM_PARAMS = (!settings->in_params) ? 0 : KM_Tag_GetNumPairs(settings->in_params);

    KM_ALLOCATE_BLOCK(ret_params, SKM_TAG_VALUE_BLOCK_SIZE);
    KM_CMD_CRYPTO_BEGIN_OUT_PARAMS_PTR = ret_params;
    KM_CMD_CRYPTO_BEGIN_OUT_PARAMS_LEN = SKM_TAG_VALUE_BLOCK_SIZE;
    KM_ALLOCATE_BLOCK(op_handle, sizeof(km_operation_handle_t));
    KM_CMD_CRYPTO_BEGIN_OUT_OP_HANDLE_PTR = op_handle;
    KM_CMD_CRYPTO_BEGIN_OUT_OP_HANDLE_LEN = sizeof(km_operation_handle_t);

    KM_PROCESS_COMMAND(KM_CommandId_eCryptoBegin);
    command_completed = true;

    err = KM_Tag_CreateContextFromTagValueSet(ret_params, &ret_context);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    settings->out_params = ret_context;
    BKNI_Memcpy(out_operation_handle, KM_CMD_CRYPTO_BEGIN_OUT_OP_HANDLE_PTR, sizeof(km_operation_handle_t));
    err = BERR_SUCCESS;

done:
    if (command_completed && (err != BERR_SUCCESS)) {
        /* We need to abort because it failed after successful SAGE call */
        km_operation_handle_t tmp_handle;
        BKNI_Memcpy(&tmp_handle, KM_CMD_CRYPTO_BEGIN_OUT_OP_HANDLE_PTR, sizeof(km_operation_handle_t));
        (void)KeymasterTl_CryptoAbort(handle, tmp_handle);
    }
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(params);
    KM_FREE_BLOCK(in_key_blob_ptr);
    KM_FREE_BLOCK(ret_params);
    KM_FREE_BLOCK(op_handle);
    BDBG_LEAVE(KeymasterTl_CryptoBegin);
    return err;
}

void KeymasterTl_GetDefaultCryptoUpdateSettings(
    KeymasterTl_CryptoUpdateSettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
}

BERR_Code KeymasterTl_CryptoUpdate(
    KeymasterTl_Handle handle,
    km_operation_handle_t in_operation_handle,
    KeymasterTl_CryptoUpdateSettings *settings)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *params = NULL;
    uint32_t length = 0;
    uint8_t *ret_params = NULL;
    uint8_t *op_handle = NULL;
    KM_Tag_ContextHandle ret_context = NULL;

    BDBG_ENTER(KeymasterTl_CryptoUpdate);

    if (!handle || !settings) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    KM_ALLOCATE_BLOCK(op_handle, sizeof(km_operation_handle_t));
    BKNI_Memcpy(op_handle, &in_operation_handle, sizeof(km_operation_handle_t));
    KM_CMD_CRYPTO_UPDATE_IN_OP_HANDLE_PTR = op_handle;
    KM_CMD_CRYPTO_UPDATE_IN_OP_HANDLE_LEN = sizeof(km_operation_handle_t);

    err = _KeymasterTl_SerializeParams(settings->in_params, &params, &length, true);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    KM_CMD_CRYPTO_UPDATE_IN_PARAMS_PTR = params;
    KM_CMD_CRYPTO_UPDATE_IN_PARAMS_LEN = length;
    KM_CMD_CRYPTO_UPDATE_IN_NUM_PARAMS = (!settings->in_params) ? 0 : KM_Tag_GetNumPairs(settings->in_params);

    KM_CMD_CRYPTO_UPDATE_IN_DATA_PTR = settings->in_data.buffer;
    KM_CMD_CRYPTO_UPDATE_IN_DATA_LEN = settings->in_data.size;
    KM_ALLOCATE_BLOCK(ret_params, SKM_TAG_VALUE_BLOCK_SIZE);
    KM_CMD_CRYPTO_UPDATE_OUT_PARAMS_PTR = ret_params;
    KM_CMD_CRYPTO_UPDATE_OUT_PARAMS_LEN = SKM_TAG_VALUE_BLOCK_SIZE;
    KM_CMD_CRYPTO_UPDATE_OUT_DATA_PTR = settings->out_data.buffer;
    KM_CMD_CRYPTO_UPDATE_OUT_DATA_LEN = settings->out_data.size;

    KM_PROCESS_COMMAND(KM_CommandId_eCryptoUpdate);

    err = KM_Tag_CreateContextFromTagValueSet(ret_params, &ret_context);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    settings->out_params = ret_context;
    settings->out_input_consumed = KM_CMD_CRYPTO_UPDATE_OUT_INPUT_CONSUMED;
    settings->out_data_size = KM_CMD_CRYPTO_UPDATE_OUT_RET_DATA_LEN;
    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(op_handle);
    KM_FREE_BLOCK(params);
    KM_FREE_BLOCK(ret_params);
    BDBG_LEAVE(KeymasterTl_CryptoUpdate);
    return err;
}

void KeymasterTl_GetDefaultCryptoFinishSettings(
    KeymasterTl_CryptoFinishSettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
}

BERR_Code KeymasterTl_CryptoFinish(
    KeymasterTl_Handle handle,
    km_operation_handle_t in_operation_handle,
    KeymasterTl_CryptoFinishSettings *settings)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *op_handle = NULL;
    uint8_t *params = NULL;
    uint8_t *signature = NULL;
    uint32_t length = 0;
    uint8_t *ret_params = NULL;
    KM_Tag_ContextHandle ret_context = NULL;

    BDBG_ENTER(KeymasterTl_CryptoFinish);

    if (!handle || !settings) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    KM_ALLOCATE_BLOCK(op_handle, sizeof(km_operation_handle_t));
    BKNI_Memcpy(op_handle, &in_operation_handle, sizeof(km_operation_handle_t));
    KM_CMD_CRYPTO_FINISH_IN_OP_HANDLE_PTR = op_handle;
    KM_CMD_CRYPTO_FINISH_IN_OP_HANDLE_LEN = sizeof(km_operation_handle_t);

    err = _KeymasterTl_SerializeParams(settings->in_params, &params, &length, true);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    KM_CMD_CRYPTO_FINISH_IN_PARAMS_PTR = params;
    KM_CMD_CRYPTO_FINISH_IN_PARAMS_LEN = length;
    KM_CMD_CRYPTO_FINISH_IN_NUM_PARAMS = (!settings->in_params) ? 0 : KM_Tag_GetNumPairs(settings->in_params);

    if (settings->in_data.size && settings->in_data.buffer) {
        KM_CMD_CRYPTO_FINISH_IN_DATA_PTR = settings->in_data.buffer;
        KM_CMD_CRYPTO_FINISH_IN_DATA_LEN = settings->in_data.size;
    } else {
        KM_CMD_CRYPTO_FINISH_IN_DATA_PTR = NULL;
        KM_CMD_CRYPTO_FINISH_IN_DATA_LEN = 0;
    }
    if (settings->in_signature.size && settings->in_signature.buffer) {
        KM_ALLOCATE_BLOCK(signature, settings->in_signature.size);
        BKNI_Memcpy(signature, settings->in_signature.buffer, settings->in_signature.size);
        KM_CMD_CRYPTO_FINISH_IN_SIGNATURE_PTR = signature;
        KM_CMD_CRYPTO_FINISH_IN_SIGNATURE_LEN = settings->in_signature.size;
    } else {
        KM_CMD_CRYPTO_FINISH_IN_SIGNATURE_PTR = NULL;
        KM_CMD_CRYPTO_FINISH_IN_SIGNATURE_LEN = 0;
    }
    KM_ALLOCATE_BLOCK(ret_params, SKM_TAG_VALUE_BLOCK_SIZE);
    KM_CMD_CRYPTO_FINISH_OUT_PARAMS_PTR = ret_params;
    KM_CMD_CRYPTO_FINISH_OUT_PARAMS_LEN = SKM_TAG_VALUE_BLOCK_SIZE;
    if (settings->out_data.size && settings->out_data.buffer) {
        KM_CMD_CRYPTO_FINISH_OUT_DATA_PTR = settings->out_data.buffer;
        KM_CMD_CRYPTO_FINISH_OUT_DATA_LEN = settings->out_data.size;
    } else {
        KM_CMD_CRYPTO_FINISH_OUT_DATA_PTR = NULL;
        KM_CMD_CRYPTO_FINISH_OUT_DATA_LEN = 0;
    }

    KM_PROCESS_COMMAND(KM_CommandId_eCryptoFinish);

    err = KM_Tag_CreateContextFromTagValueSet(ret_params, &ret_context);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    settings->out_params = ret_context;
    settings->out_data_size = KM_CMD_CRYPTO_FINISH_OUT_RET_DATA_LEN;
    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(op_handle);
    KM_FREE_BLOCK(params);
    KM_FREE_BLOCK(signature);
    KM_FREE_BLOCK(ret_params);
    BDBG_LEAVE(KeymasterTl_CryptoFinish);
    return err;
}

BERR_Code KeymasterTl_CryptoAbort(
    KeymasterTl_Handle handle,
    km_operation_handle_t in_operation_handle)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *op_handle = NULL;

    BDBG_ENTER(KeymasterTl_CryptoAbort);

    if (!handle) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    KM_ALLOCATE_BLOCK(op_handle, sizeof(km_operation_handle_t));
    BKNI_Memcpy(op_handle, &in_operation_handle, sizeof(km_operation_handle_t));
    KM_CMD_CRYPTO_ABORT_IN_OP_HANDLE_PTR = op_handle;
    KM_CMD_CRYPTO_ABORT_IN_OP_HANDLE_LEN = sizeof(km_operation_handle_t);

    KM_PROCESS_COMMAND(KM_CommandId_eCryptoAbort);
    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(op_handle);
    BDBG_LEAVE(KeymasterTl_CryptoAbort);
    return err;
}

BERR_Code KeymasterTl_GetConfiguration(
    KeymasterTl_Handle handle,
    uint32_t *keymaster_version,
    bool *rpmbEnabled,
    bool *usingVms,
    uint32_t *hwKeysAvailable)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;

    BDBG_ENTER(KeymasterTl_GetConfiguration);

    if (!handle) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();
    KM_PROCESS_COMMAND(KM_CommandId_eGetConfiguration);
    if (keymaster_version) {
        *keymaster_version = handle->configuredKeymasterVersion;
    }
    if (rpmbEnabled) {
        *rpmbEnabled = (bool)KM_CMD_BASIC_GET_CONFIGURATION_OUT_RPMB_EN;
    }
    if (usingVms) {
        *usingVms = (bool)KM_CMD_BASIC_GET_CONFIGURATION_OUT_VMS_EN;
    }
    if (hwKeysAvailable) {
        *hwKeysAvailable = KM_CMD_BASIC_GET_CONFIGURATION_OUT_HW_KEY_TOTAL;
    }
    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    BDBG_LEAVE(KeymasterTl_GetConfiguration);
    return err;
}

BERR_Code KeymasterTl_CacheKey(
    KeymasterTl_Handle handle,
    uint32_t key_size,
    uint32_t exponent)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;

    BDBG_ENTER(KeymasterTl_CacheKey);

    if (!handle || !key_size || !exponent) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BERR_INVALID_PARAMETER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    KM_ALLOCATE_CONTAINER();

    KM_CMD_BASIC_CACHE_KEYS_IN_KEY_SIZE = key_size;
    KM_CMD_BASIC_CACHE_KEYS_IN_EXPONENT = exponent;

    KM_PROCESS_COMMAND(KM_CommandId_eCacheKey);

    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    BDBG_LEAVE(KeymasterTl_CacheKey);
    return err;
}

/* Wrapped key needs to be transformed into a block of memory of
   this structure:

    uint32_t version         # Always 0
    uint32_t transit_key_len
    uint8_t transit_key[]    # transit_key_len bytes
    uint32_t iv_len
    uint8_t iv[]             # iv_len bytes
    uint32_t key_format      # km_key_format_t
    uint32_t auth_count      # Entries in auth, below
    uint32_t auth_len        # Size of auth, below
    uint8_t auth[]           # Serial form of tag/value auth data
    uint32_t secure_key_len
    uint8_t secure_key[]     # secure_key_len bytes
    uint32_t tag_len
    uint8_t tag              # AES GCM tag data (tag_len bytes)
    uint32_t description_len
    uint8_t description[]    # DER encoded stream of KeyDescription

    This is done because there are insufficient block space in
    the container structure to transfer them as separate parameters,
    so we may as well stick to the format that X.509 uses.
*/

void KeymasterTl_GetDefaultImportWrappedKeySettings(
    KeymasterTl_ImportWrappedKeySettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
}

#define PUSH_UINT32(var)  if (length < sizeof(uint32_t)) goto done; \
                          BKNI_Memcpy(ptr, &var, sizeof(uint32_t)); \
                          ptr += sizeof(uint32_t); \
                          length -= sizeof(uint32_t)

#define PUSH_BUFFER(var, len)  if (length < len) goto done; \
                          BKNI_Memcpy(ptr, var, len); \
                          ptr += len; \
                          length -= len

BERR_Code KeymasterTl_ImportWrappedKey(
    KeymasterTl_Handle handle,
    KeymasterTl_ImportWrappedKeySettings *settings)
{
    BERR_Code err = BERR_UNKNOWN;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *params = NULL;
    uint32_t param_length = 0;
    uint32_t length = 0;
    uint32_t data = 0;
    uint8_t *ptr;
    uint8_t *in_wrapped_key_ptr = NULL;
    uint8_t *in_wrapping_key_ptr = NULL;
    uint8_t *in_masking_key_ptr = NULL;
    uint8_t *key_blob = NULL;

    BDBG_ENTER(KeymasterTl_ImportWrappedKey);

    if (!handle || !settings) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }
    if (!settings->in_transit_key.size || !settings->in_transit_key.buffer || (settings->in_transit_key.size > SKM_WRAPPED_BLOB_MAX_SIZE)) {
        BDBG_ERR(("%s: NULL transit key", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }
    if (!settings->in_iv.size || !settings->in_iv.buffer || (settings->in_iv.size > SKM_WRAPPED_BLOB_MAX_SIZE)) {
        BDBG_ERR(("%s: NULL IV", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }
    if (!settings->in_key_params) {
        BDBG_ERR(("%s: NULL key params", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }
    if (!settings->in_secure_key.size || !settings->in_secure_key.buffer || (settings->in_secure_key.size > SKM_WRAPPED_BLOB_MAX_SIZE)) {
        BDBG_ERR(("%s: NULL secure key", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }
    if (!settings->in_tag_data.size || !settings->in_tag_data.buffer || (settings->in_tag_data.size > SKM_WRAPPED_BLOB_MAX_SIZE)) {
        BDBG_ERR(("%s: NULL tag data", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }
    if (!settings->in_description.size || !settings->in_description.buffer || (settings->in_description.size > SKM_WRAPPED_BLOB_MAX_SIZE)) {
        BDBG_ERR(("%s: NULL description", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }
    if (!settings->in_wrapping_key.size || !settings->in_wrapping_key.buffer) {
        BDBG_ERR(("%s: NULL wrapping key", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }
    if (!settings->in_masking_key.size || !settings->in_masking_key.buffer) {
        BDBG_ERR(("%s: NULL masking key", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    if (handle->configuredKeymasterVersion < SKM_VERSION_4) {
        BDBG_ERR(("%s: function only supported in KM 4", BSTD_FUNCTION));
        err = BSAGE_ERR_MODULE_COMMAND_ID;
        goto done;
    }

    KM_ALLOCATE_CONTAINER();

    /* Work out the size of the wrapped key data - see function comment */
    err = KM_Tag_Serialize(settings->in_key_params, NULL, &param_length);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    length = param_length;
    length += sizeof(uint32_t) * 9;
    length += settings->in_transit_key.size;
    length += settings->in_iv.size;
    length += settings->in_secure_key.size;
    length += settings->in_tag_data.size;
    length += settings->in_description.size;
    if (length > SKM_WRAPPED_BLOB_MAX_SIZE) {
        BDBG_ERR(("%s: Wrapped data too large", BSTD_FUNCTION));
        err = BERR_INVALID_PARAMETER;
        goto done;
    }
    KM_ALLOCATE_BLOCK(in_wrapped_key_ptr, length);
    KM_CMD_IMPORT_WRAPPED_KEY_IN_WRAPPED_KEY_PTR = in_wrapped_key_ptr;
    KM_CMD_IMPORT_WRAPPED_KEY_IN_WRAPPED_KEY_LEN = length;
    ptr = in_wrapped_key_ptr;

    PUSH_UINT32(settings->version);
    PUSH_UINT32(settings->in_transit_key.size);
    PUSH_BUFFER(settings->in_transit_key.buffer, settings->in_transit_key.size);
    PUSH_UINT32(settings->in_iv.size);
    PUSH_BUFFER(settings->in_iv.buffer, settings->in_iv.size);
    data = (uint32_t)settings->in_key_format;
    PUSH_UINT32(data);
    data = KM_Tag_GetNumPairs(settings->in_key_params);
    PUSH_UINT32(data);
    PUSH_UINT32(param_length);

    data = length;
    err = KM_Tag_Serialize(settings->in_key_params, ptr, &data);
    if (err != BERR_SUCCESS) {
        BDBG_ERR(("%s: serialize failed", BSTD_FUNCTION));
        goto done;
    }
    if (data != param_length) {
        BDBG_ERR(("%s: param data changed size", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    ptr += param_length;
    length -= param_length;
    err = BERR_UNKNOWN;

    PUSH_UINT32(settings->in_secure_key.size);
    PUSH_BUFFER(settings->in_secure_key.buffer, settings->in_secure_key.size);
    PUSH_UINT32(settings->in_tag_data.size);
    PUSH_BUFFER(settings->in_tag_data.buffer, settings->in_tag_data.size);
    PUSH_UINT32(settings->in_description.size);
    PUSH_BUFFER(settings->in_description.buffer, settings->in_description.size);

    KM_ALLOCATE_BLOCK(in_wrapping_key_ptr, settings->in_wrapping_key.size);
    KM_CMD_IMPORT_WRAPPED_KEY_IN_WRAPPING_KEY_PTR = in_wrapping_key_ptr;
    KM_CMD_IMPORT_WRAPPED_KEY_IN_WRAPPING_KEY_LEN = settings->in_wrapping_key.size;
    BKNI_Memcpy(in_wrapping_key_ptr, settings->in_wrapping_key.buffer, settings->in_wrapping_key.size);

    KM_ALLOCATE_BLOCK(in_masking_key_ptr, settings->in_masking_key.size);
    KM_CMD_IMPORT_WRAPPED_KEY_IN_MASKING_KEY_PTR = in_masking_key_ptr;
    KM_CMD_IMPORT_WRAPPED_KEY_IN_MASKING_KEY_LEN = settings->in_masking_key.size;
    BKNI_Memcpy(in_masking_key_ptr, settings->in_masking_key.buffer, settings->in_masking_key.size);

    err = _KeymasterTl_SerializeParams(settings->in_params, &params, &length, true);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    KM_CMD_IMPORT_WRAPPED_KEY_IN_PARAMS_PTR = params;
    KM_CMD_IMPORT_WRAPPED_KEY_IN_PARAMS_LEN = length;
    KM_CMD_IMPORT_WRAPPED_KEY_IN_NUM_PARAMS = KM_Tag_GetNumPairs(settings->in_params);

    KM_ALLOCATE_BLOCK(key_blob, SKM_MAX_KEY_BLOB_SIZE);
    KM_CMD_IMPORT_WRAPPED_KEY_OUT_KEY_BLOB_PTR = key_blob;
    KM_CMD_IMPORT_WRAPPED_KEY_OUT_KEY_BLOB_LEN = SKM_MAX_KEY_BLOB_SIZE;

    KM_PROCESS_COMMAND(KM_CommandId_eImportWrappedKey);

    KM_ALLOCATE_BLOCK(settings->out_key_blob.buffer, KM_CMD_IMPORT_WRAPPED_KEY_OUT_RET_KEY_BLOB_LEN);
    BKNI_Memcpy(settings->out_key_blob.buffer, key_blob, KM_CMD_IMPORT_WRAPPED_KEY_OUT_RET_KEY_BLOB_LEN);
    settings->out_key_blob.size = KM_CMD_IMPORT_WRAPPED_KEY_OUT_RET_KEY_BLOB_LEN;
    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(params);
    KM_FREE_BLOCK(key_blob);
    KM_FREE_BLOCK(in_wrapping_key_ptr);
    KM_FREE_BLOCK(in_masking_key_ptr);
    KM_FREE_BLOCK(in_wrapped_key_ptr);
    BDBG_LEAVE(KeymasterTl_ImportWrappedKey);
    return err;
}

void KeymasterTl_GetDefaultHmacSharingParamsSettings(
    KeymasterTl_GetHmacSharingParamsSettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
}

BERR_Code KeymasterTl_GetHmacSharingParams(
    KeymasterTl_Handle handle,
    KeymasterTl_GetHmacSharingParamsSettings *settings)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *out_seed = NULL;
    uint8_t *out_nonce = NULL;

    BDBG_ENTER(KeymasterTl_GetHmacSharingParams);

    if (!handle || !settings) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    if (handle->configuredKeymasterVersion < SKM_VERSION_4) {
        BDBG_ERR(("%s: function only supported in KM 4", BSTD_FUNCTION));
        err = BSAGE_ERR_MODULE_COMMAND_ID;
        goto done;
    }

    KM_ALLOCATE_CONTAINER();

    KM_ALLOCATE_BLOCK(out_seed, SKM_SHA256_DIGEST_SIZE);
    KM_CMD_GET_HMAC_SHARING_OUT_SEED_PTR = out_seed;
    KM_CMD_GET_HMAC_SHARING_OUT_SEED_LEN = SKM_SHA256_DIGEST_SIZE;

    KM_ALLOCATE_BLOCK(out_nonce, SKM_SHA256_DIGEST_SIZE);
    KM_CMD_GET_HMAC_SHARING_OUT_NONCE_PTR = out_nonce;
    KM_CMD_GET_HMAC_SHARING_OUT_NONCE_LEN = SKM_SHA256_DIGEST_SIZE;

    KM_PROCESS_COMMAND(KM_CommandId_eGetHmacSharingParams);

    KM_ALLOCATE_BLOCK(settings->out_seed.buffer, KM_CMD_GET_HMAC_SHARING_OUT_SEED_RET_LEN);
    BKNI_Memcpy(settings->out_seed.buffer, out_seed, KM_CMD_GET_HMAC_SHARING_OUT_SEED_RET_LEN);
    settings->out_seed.size = KM_CMD_GET_HMAC_SHARING_OUT_SEED_RET_LEN;

    KM_ALLOCATE_BLOCK(settings->out_nonce.buffer, KM_CMD_GET_HMAC_SHARING_OUT_NONCE_LEN);
    BKNI_Memcpy(settings->out_nonce.buffer, out_nonce, KM_CMD_GET_HMAC_SHARING_OUT_NONCE_LEN);
    settings->out_nonce.size = KM_CMD_GET_HMAC_SHARING_OUT_NONCE_LEN;

    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(out_seed);
    KM_FREE_BLOCK(out_nonce);
    BDBG_LEAVE(KeymasterTl_GetHmacSharingParams);
    return err;
}

void KeymasterTl_GetDefaultComputeSharedHmacSettings(
    KeymasterTl_GetComputeSharedHmacSettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
}

BERR_Code KeymasterTl_ComputeSharedHmac(
    KeymasterTl_Handle handle,
    KeymasterTl_GetComputeSharedHmacSettings *settings)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *in_sharing_params = NULL;
    uint8_t *out_sharing_check = NULL;

    BDBG_ENTER(KeymasterTl_ComputeSharedHmac);

    if (!handle || !settings || (settings->in_num_params > SKM_MAX_SHARING_PARAMS)) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    if (handle->configuredKeymasterVersion < SKM_VERSION_4) {
        BDBG_ERR(("%s: function only supported in KM 4", BSTD_FUNCTION));
        err = BSAGE_ERR_MODULE_COMMAND_ID;
        goto done;
    }

    KM_ALLOCATE_CONTAINER();

    KM_ALLOCATE_BLOCK(in_sharing_params, sizeof(km_hmac_sharing_t) * settings->in_num_params);
    KM_CMD_COMPUTE_SHARED_HMAC_IN_NUM_PARAMS = settings->in_num_params;
    KM_CMD_COMPUTE_SHARED_HMAC_IN_PARAMS_PTR = in_sharing_params;
    KM_CMD_COMPUTE_SHARED_HMAC_IN_PARAMS_LEN = sizeof(km_hmac_sharing_t) * settings->in_num_params;
    BKNI_Memcpy(in_sharing_params, settings->in_sharing_params, KM_CMD_COMPUTE_SHARED_HMAC_IN_PARAMS_LEN);

    KM_ALLOCATE_BLOCK(out_sharing_check, SKM_SHA256_DIGEST_SIZE);
    KM_CMD_COMPUTE_SHARED_HMAC_OUT_SHARING_CHECK_PTR = out_sharing_check;
    KM_CMD_COMPUTE_SHARED_HMAC_OUT_SHARING_CHECK_LEN = SKM_SHA256_DIGEST_SIZE;

    KM_PROCESS_COMMAND(KM_CommandId_eComputeSharedHmac);

    KM_ALLOCATE_BLOCK(settings->out_sharing_check.buffer, SKM_SHA256_DIGEST_SIZE);
    BKNI_Memcpy(settings->out_sharing_check.buffer, out_sharing_check, SKM_SHA256_DIGEST_SIZE);
    settings->out_sharing_check.size = SKM_SHA256_DIGEST_SIZE;

    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(in_sharing_params);
    KM_FREE_BLOCK(out_sharing_check);
    BDBG_LEAVE(KeymasterTl_ComputeSharedHmac);
    return err;
}

void KeymasterTl_GetDefaultVerifyAuthorizationSettings(
    KeymasterTl_VerifyAuthorizationSettings *settings)
{
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));
}

BERR_Code KeymasterTl_VerifyAuthorization(
    KeymasterTl_Handle handle,
    KeymasterTl_VerifyAuthorizationSettings *settings)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *in_challenge = NULL;
    uint32_t length = 0;
    uint8_t *params = NULL;
    uint8_t *in_auth_token = NULL;
    uint8_t *out_verification_token = NULL;

    BDBG_ENTER(KeymasterTl_VerifyAuthorization);

    if (!handle || !settings || !settings->out_verification_token) {
        BDBG_ERR(("%s: Invalid input", BSTD_FUNCTION));
        err = BSAGE_ERR_KM_UNEXPECTED_NULL_POINTER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, KeymasterTl_Instance);

    if (handle->configuredKeymasterVersion < SKM_VERSION_4) {
        BDBG_ERR(("%s: function only supported in KM 4", BSTD_FUNCTION));
        err = BSAGE_ERR_MODULE_COMMAND_ID;
        goto done;
    }

    KM_ALLOCATE_CONTAINER();

    KM_ALLOCATE_BLOCK(in_challenge, sizeof(uint64_t));
    KM_CMD_VERIFY_AUTH_IN_CHALLENGE_PTR = in_challenge;
    KM_CMD_VERIFY_AUTH_IN_CHALLENGE_LEN = sizeof(uint64_t);
    BKNI_Memcpy(in_challenge, &settings->in_challenge, KM_CMD_VERIFY_AUTH_IN_CHALLENGE_LEN);

    err = _KeymasterTl_SerializeParams(settings->in_params, &params, &length, true);
    if (err != BERR_SUCCESS) {
        goto done;
    }
    KM_CMD_VERIFY_AUTH_IN_PARAMS_PTR = params;
    KM_CMD_VERIFY_AUTH_IN_PARAMS_LEN = length;
    KM_CMD_VERIFY_AUTH_IN_NUM_PARAMS = KM_Tag_GetNumPairs(settings->in_params);

    if (settings->in_auth_token) {
        KM_ALLOCATE_BLOCK(in_auth_token, sizeof(km_hw_auth_token_t));
        KM_CMD_VERIFY_AUTH_IN_AUTH_TOKEN_PTR = in_auth_token;
        KM_CMD_VERIFY_AUTH_IN_AUTH_TOKEN_LEN = sizeof(km_hw_auth_token_t);
        BKNI_Memcpy(in_auth_token, settings->in_auth_token, KM_CMD_VERIFY_AUTH_IN_AUTH_TOKEN_LEN);
    }

    KM_ALLOCATE_BLOCK(out_verification_token, sizeof(km_verification_token_t));
    KM_CMD_VERIFY_AUTH_OUT_VERIFY_TOKEN_PTR = out_verification_token;
    KM_CMD_VERIFY_AUTH_OUT_VERIFY_TOKEN_LEN = sizeof(km_verification_token_t);

    KM_PROCESS_COMMAND(KM_CommandId_eVerifyAuthorization);

    BKNI_Memcpy(settings->out_verification_token, out_verification_token, sizeof(km_verification_token_t));

    err = BERR_SUCCESS;

done:
    KM_FREE_CONTAINER();
    KM_FREE_BLOCK(in_challenge);
    KM_FREE_BLOCK(params);
    KM_FREE_BLOCK(in_auth_token);
    KM_FREE_BLOCK(out_verification_token);
    BDBG_LEAVE(KeymasterTl_VerifyAuthorization);
    return err;
}
