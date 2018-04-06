/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <string.h>

#include "nexus_memory.h"
#include "nexus_security_client.h"
#include "nexus_core_utils.h"
#include "keymaster_platform.h"
#include "keymaster_ids.h"
#include "gatekeeper_tl.h"
#include "keymaster_err.h"
#include "gatekeeper_cmd.h"

#ifdef ANDROID
#define GATEKEEPER_DRM_BIN_DEFAULT_FILEPATH "/dev/hwcfg/drm.bin"
#else
#define GATEKEEPER_DRM_BIN_DEFAULT_FILEPATH "./drm.bin"
#endif

BDBG_MODULE(gatekeeper_tl);

struct GatekeeperTl_Instance {
    BDBG_OBJECT(GatekeeperTl_Instance)
    SRAI_ModuleHandle moduleHandle;
};

BDBG_OBJECT_ID_DECLARE(GatekeeperTl_Instance);
BDBG_OBJECT_ID(GatekeeperTl_Instance);


#define GK_ALLOCATE_CONTAINER()                      inout = SRAI_Container_Allocate(); \
                                                     if (!inout) { err = BERR_OUT_OF_SYSTEM_MEMORY; goto done; }


#define GK_FREE_CONTAINER()                          if (inout) { SRAI_Container_Free(inout); }


#define GK_ALLOCATE_BLOCK(ptr, size)                 ptr = (uint8_t *)SRAI_Memory_Allocate(size, SRAI_MemoryType_Shared); \
                                                     if (!ptr) { err = BERR_OUT_OF_SYSTEM_MEMORY; goto done; }


#define GK_FREE_BLOCK(ptr)                           if (ptr) { SRAI_Memory_Free(ptr); ptr = NULL; }


#define GK_PROCESS_COMMAND(cmd_id)                   err = SRAI_Module_ProcessCommand(handle->moduleHandle, cmd_id, inout); \
                                                     if ((err != BERR_SUCCESS) || (inout->basicOut[0] != BERR_SUCCESS)) { \
                                                         BDBG_MSG(("%s: Command failed (%x/%x)", BSTD_FUNCTION, err, inout->basicOut[0])); \
                                                         if (err == BERR_SUCCESS) { err = inout->basicOut[0]; } \
                                                         goto done; \
                                                     }



static void _GatekeeperTl_ContextDelete(GatekeeperTl_Handle hGatekeeperTl);
static GatekeeperTl_Handle _GatekeeperTl_ContextNew(const char *bin_file_path);

void GatekeeperTl_GetDefaultInitSettings(GatekeeperTl_InitSettings *pModuleSettings)
{
    BDBG_ENTER(GatekeeperTl_GetDefaultInitSettings);

    BDBG_ASSERT(pModuleSettings);
    BKNI_Memset((uint8_t *)pModuleSettings, 0, sizeof(GatekeeperTl_InitSettings));
    BKNI_Memcpy(pModuleSettings->drm_binfile_path, GATEKEEPER_DRM_BIN_DEFAULT_FILEPATH, strlen(GATEKEEPER_DRM_BIN_DEFAULT_FILEPATH));

    BDBG_LEAVE(GatekeeperTl_GetDefaultInitSettings);
    return;
}

static GatekeeperTl_Handle _GatekeeperTl_ContextNew(const char *bin_file_path)
{
    BERR_Code err;
    NEXUS_Error nexus_rc;
    BSAGElib_InOutContainer *inout = NULL;
    GatekeeperTl_Handle handle = NULL;

    nexus_rc = NEXUS_Memory_Allocate(sizeof(*handle), NULL, (void **)&handle);
    if (nexus_rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: cannot allocate memory for context", BSTD_FUNCTION));
        err = BERR_OUT_OF_DEVICE_MEMORY;
        goto done;
    }
    handle->moduleHandle = NULL;

    BDBG_OBJECT_SET(handle, GatekeeperTl_Instance);

    GK_ALLOCATE_CONTAINER();

    err = Keymaster_ModuleInit(Keymaster_ModuleId_eGatekeeper,
                               bin_file_path,
                               inout,
                               &handle->moduleHandle,
                               NULL);
    if (err != BERR_SUCCESS) {
        BDBG_ERR(("%s: Error initializing module (0x%08x)", BSTD_FUNCTION, inout->basicOut[0]));
        goto done;
    }

done:
    GK_FREE_CONTAINER();

    if (err != BERR_SUCCESS && handle) {
        _GatekeeperTl_ContextDelete(handle);
        handle = NULL;
    }

    return handle;
}

static void _GatekeeperTl_ContextDelete(GatekeeperTl_Handle handle)
{
    if (handle) {
        if (handle->moduleHandle) {
            Keymaster_ModuleUninit(handle->moduleHandle, NULL);
            handle->moduleHandle = NULL;
        }

        BDBG_OBJECT_DESTROY(handle, GatekeeperTl_Instance);
        NEXUS_Memory_Free(handle);
    }
}

BERR_Code GatekeeperTl_Init(GatekeeperTl_Handle *pHandle, GatekeeperTl_InitSettings *pModuleSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    GatekeeperTl_InitSettings localSettings;

    BDBG_ENTER(GatekeeperTl_Init);

    if (!pModuleSettings) {
        GatekeeperTl_GetDefaultInitSettings(&localSettings);
        pModuleSettings = &localSettings;
    }

    if (!pHandle) {
        BDBG_ERR(("%s: Parameter pHandle is NULL", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto done;
    }

    *pHandle = _GatekeeperTl_ContextNew(pModuleSettings->drm_binfile_path);
    if (*pHandle == NULL) {
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto done;
    }

done:

    BDBG_LEAVE(GatekeeperTl_Init);
    return rc;
}

void GatekeeperTl_Uninit(GatekeeperTl_Handle handle)
{
    BDBG_ENTER(GatekeeperTl_Uninit);

    BDBG_OBJECT_ASSERT(handle, GatekeeperTl_Instance);
    _GatekeeperTl_ContextDelete(handle);

    BDBG_LEAVE(GatekeeperTl_Uninit);
    return;
}


BERR_Code GatekeeperTl_Enroll(
    GatekeeperTl_Handle handle,
    uint32_t in_user_id,
    gk_password_handle_t *in_password_handle,
    GatekeeperTl_Password *in_enroll_password,
    GatekeeperTl_Password *in_provided_password,
    uint32_t *out_retry_timeout,
    gk_password_handle_t *out_password_handle)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *in_handle = NULL;
    uint8_t *in_enroll_pass = NULL;
    uint8_t *in_provided_pass = NULL;
    uint8_t *out_handle = NULL;

    BDBG_ENTER(GatekeeperTl_Enroll);

    if (!handle || !in_provided_password || !in_provided_password->size || !in_provided_password->buffer ||
        (in_provided_password->size > GATEKEEPER_MAX_PASSWORD) ||!out_password_handle) {
        BDBG_ERR(("%s: Invalid parameter", BSTD_FUNCTION));
        err = BERR_INVALID_PARAMETER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, GatekeeperTl_Instance);

    GK_ALLOCATE_CONTAINER();

    if (in_password_handle) {
        if (!in_enroll_password || !in_enroll_password->size || !in_enroll_password->buffer ||
            (in_enroll_password->size > GATEKEEPER_MAX_PASSWORD)) {
            BDBG_ERR(("%s: Invalid password", BSTD_FUNCTION));
            err = BERR_INVALID_PARAMETER;
            goto done;
        }

        GK_ALLOCATE_BLOCK(in_handle, sizeof(gk_password_handle_t));
        BKNI_Memcpy(in_handle, in_password_handle, sizeof(gk_password_handle_t));
        GK_ALLOCATE_BLOCK(in_enroll_pass, in_enroll_password->size);
        BKNI_Memcpy(in_enroll_pass, in_enroll_password->buffer, in_enroll_password->size);
    }

    GK_ALLOCATE_BLOCK(in_provided_pass, in_provided_password->size);
    BKNI_Memcpy(in_provided_pass, in_provided_password->buffer, in_provided_password->size);
    GK_ALLOCATE_BLOCK(out_handle, sizeof(gk_password_handle_t));

    GK_CMD_ENROLL_IN_USER_ID = in_user_id;
    GK_CMD_ENROLL_IN_PASS_HANDLE_PTR = in_handle;
    GK_CMD_ENROLL_IN_PASS_HANDLE_LEN = in_handle ? sizeof(gk_password_handle_t) : 0;
    GK_CMD_ENROLL_IN_ENROLL_PASS_PTR = in_enroll_pass;
    GK_CMD_ENROLL_IN_ENROLL_PASS_LEN = in_enroll_pass ? in_enroll_password->size : 0;
    GK_CMD_ENROLL_IN_PROVIDED_PASS_PTR = in_provided_pass;
    GK_CMD_ENROLL_IN_PROVIDED_PASS_LEN = in_provided_password->size;
    GK_CMD_ENROLL_OUT_PASS_HANDLE_PTR = out_handle;
    GK_CMD_ENROLL_OUT_PASS_HANDLE_LEN = sizeof(gk_password_handle_t);

    GK_PROCESS_COMMAND(KM_CommandId_eGatekeeperEnroll);

    BKNI_Memcpy(out_password_handle, out_handle, sizeof(gk_password_handle_t));

done:
    if (out_retry_timeout) {
        *out_retry_timeout = GK_CMD_ENROLL_OUT_RETRY_TIMEOUT;
    }
    GK_FREE_CONTAINER();
    GK_FREE_BLOCK(in_handle);
    GK_FREE_BLOCK(in_enroll_pass);
    GK_FREE_BLOCK(in_provided_pass);
    GK_FREE_BLOCK(out_handle);
    BDBG_LEAVE(GatekeeperTl_Enroll);
    return err;
}

BERR_Code GatekeeperTl_Verify(
    GatekeeperTl_Handle handle,
    uint32_t in_user_id,
    uint64_t in_challenge,
    gk_password_handle_t *in_password_handle,
    GatekeeperTl_Password *in_provided_password,
    uint32_t *out_retry_timeout,
    km_hw_auth_token_t *out_auth_token)
{
    BERR_Code err;
    BSAGElib_InOutContainer *inout = NULL;
    uint8_t *challenge = NULL;
    uint8_t *in_handle = NULL;
    uint8_t *in_provided_pass = NULL;
    uint8_t *out_token = NULL;

    BDBG_ENTER(GatekeeperTl_Verify);

    if (!handle || !in_provided_password || !in_provided_password->size || !in_provided_password->buffer ||
        (in_provided_password->size > GATEKEEPER_MAX_PASSWORD) || !in_password_handle || !out_auth_token) {
        BDBG_ERR(("%s: Invalid parameter", BSTD_FUNCTION));
        err = BERR_INVALID_PARAMETER;
        goto done;
    }

    BDBG_OBJECT_ASSERT(handle, GatekeeperTl_Instance);

    GK_ALLOCATE_CONTAINER();

    GK_ALLOCATE_BLOCK(challenge, sizeof(uint64_t));
    BKNI_Memcpy(challenge, &in_challenge, sizeof(uint64_t));
    GK_ALLOCATE_BLOCK(in_handle, sizeof(gk_password_handle_t));
    BKNI_Memcpy(in_handle, in_password_handle, sizeof(gk_password_handle_t));
    GK_ALLOCATE_BLOCK(in_provided_pass, in_provided_password->size);
    BKNI_Memcpy(in_provided_pass, in_provided_password->buffer, in_provided_password->size);
    GK_ALLOCATE_BLOCK(out_token, sizeof(km_hw_auth_token_t));

    GK_CMD_VERIFY_IN_USER_ID = in_user_id;
    GK_CMD_VERIFY_IN_CHALLENGE_PTR = challenge;
    GK_CMD_VERIFY_IN_CHALLENGE_LEN = sizeof(uint64_t);
    GK_CMD_VERIFY_IN_PASS_HANDLE_PTR = in_handle;
    GK_CMD_VERIFY_IN_PASS_HANDLE_LEN = sizeof(gk_password_handle_t);
    GK_CMD_VERIFY_IN_PROVIDED_PASS_PTR = in_provided_pass;
    GK_CMD_VERIFY_IN_PROVIDED_PASS_LEN = in_provided_password->size;
    GK_CMD_VERIFY_OUT_AUTH_TOKEN_PTR = out_token;
    GK_CMD_VERIFY_OUT_AUTH_TOKEN_LEN = sizeof(km_hw_auth_token_t);

    GK_PROCESS_COMMAND(KM_CommandId_eGatekeeperVerify);

    BKNI_Memcpy(out_auth_token, out_token, sizeof(km_hw_auth_token_t));

done:
    if (out_retry_timeout) {
        *out_retry_timeout = GK_CMD_VERIFY_OUT_RETRY_TIMEOUT;
    }
    GK_FREE_CONTAINER();
    GK_FREE_BLOCK(challenge);
    GK_FREE_BLOCK(in_handle);
    GK_FREE_BLOCK(in_provided_pass);
    GK_FREE_BLOCK(out_token);
    BDBG_LEAVE(GatekeeperTl_Verify);
    return err;
}
