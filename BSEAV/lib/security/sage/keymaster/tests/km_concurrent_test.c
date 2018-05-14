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

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "keymaster_debug.h"
#include "bstd.h"
#include "bkni.h"

#include "openssl/x509.h"
#include "openssl/x509_vfy.h"
#include "openssl/md5.h"

#include "nexus_platform.h"
#include "nexus_platform_init.h"
#include "nexus_security_client.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include "keymaster_ids.h"
#include "keymaster_platform.h"
#include "keymaster_tl.h"
#include "keymaster_test.h"
#include "keymaster_keygen.h"
#include "keymaster_key_params.h"


BDBG_MODULE(km_concurrent_test);

#define USE_OS_VERSION     1
#define USE_OS_PATCHLEVEL  201603U

#define EXPECT_SUCCESS_UNLOCK_ON_ERR(op)  err = op; if (err != BERR_SUCCESS) { BDBG_ERR(("%s:%d  %s failed (err %x)", BSTD_FUNCTION, __LINE__, #op, err)); \
                                                                               pthread_mutex_unlock(state->mutex); goto done; }

#define NUM_CONCURRENT_OPS 10
#define TEST_KEY_SIZE      128
#define TEST_NUM_UPDATES   100
#define TEST_UPDATE_SIZE   4096
#define TEST_BLOCK_SIZE    (TEST_NUM_UPDATES * TEST_UPDATE_SIZE)

typedef struct thread_state_t {
    int i;                                  /* Index */
    BERR_Code result;                       /* End result of test */
    KeymasterTl_Handle handle;
    pthread_mutex_t *mutex;                 /* Generate key needs thread protection */
} thread_state_t;


#ifndef NXCLIENT_SUPPORT
static BERR_Code SAGE_app_join_nexus(void);
static void SAGE_app_leave_nexus(void);

static NEXUS_Error SAGE_app_join_nexus(void)
{
    NEXUS_Error err = 0;
    NEXUS_PlatformSettings platformSettings;

    BDBG_LOG(("\t*** Bringing up all Nexus modules for platform using default settings\n\n\n"));

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

    err = NEXUS_Platform_Init(&platformSettings);
    if (err != NEXUS_SUCCESS) {
        BDBG_ERR(("\t### Failed to bring up Nexus\n\n"));
    }
    return err;
}

static void SAGE_app_leave_nexus(void)
{
    NEXUS_Platform_Uninit();
}
#endif

static NEXUS_KeySlotHandle allocate_keyslot(void)
{
    NEXUS_KeySlotHandle keyslot = NULL;

#if (NEXUS_SECURITY_API_VERSION==1)
    NEXUS_SecurityKeySlotSettings keyslotSettings;

    NEXUS_Security_GetDefaultKeySlotSettings(&keyslotSettings);
    keyslotSettings.client = NEXUS_SecurityClientType_eSage;
    keyslotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    keyslot = NEXUS_Security_AllocateKeySlot(&keyslotSettings);
#else
    NEXUS_KeySlotAllocateSettings keyslotSettings;

    NEXUS_KeySlot_GetDefaultAllocateSettings(&keyslotSettings);
    keyslotSettings.useWithDma = true;
    keyslotSettings.owner = NEXUS_SecurityCpuContext_eSage;
    keyslotSettings.slotType = NEXUS_KeySlotType_eIvPerSlot;
    keyslot = NEXUS_KeySlot_Allocate(&keyslotSettings);
#endif

    if (!keyslot) {
        BDBG_ERR(("%s: Error allocating keyslot", BSTD_FUNCTION));
    }
    return keyslot;
}

static BERR_Code run_test(thread_state_t *state)
{
    BERR_Code err;
    KM_Tag_ContextHandle key_params;
    KeymasterTl_DataBlock key;
    KM_Tag_ContextHandle begin_params;
    KeymasterTl_CryptoBeginSettings beginSettings;
    KeymasterTl_CryptoUpdateSettings updateSettings;
    KeymasterTl_CryptoFinishSettings finishSettings;
    km_operation_handle_t op_handle = 0;
    NEXUS_KeySlotHandle keyslot = NULL;
    KeymasterTl_DataBlock in_data;
    KeymasterTl_DataBlock intermediate_data;
    KeymasterTl_DataBlock final_data;
    int mode;
    int block;
    uint32_t out_data_size;

    keyslot = allocate_keyslot();
    if (!keyslot) {
        err = BERR_OUT_OF_DEVICE_MEMORY;
        goto done;
    }

    EXPECT_SUCCESS(km_test_new_params_with_aes_defaults(&key_params, TEST_KEY_SIZE));

    /* API that are split into start/stop internally need to be mutex protected to call from threads */
    /* Perhaps a mutex should be added into the TL code to make it consistent */
    pthread_mutex_lock(state->mutex);
    EXPECT_SUCCESS_UNLOCK_ON_ERR(KeymasterTl_GenerateKey(state->handle, key_params, &key));
    pthread_mutex_unlock(state->mutex);

    EXPECT_SUCCESS(KM_Tag_NewContext(&begin_params));
    TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_PADDING, SKM_PAD_NONE);
    TEST_TAG_ADD_ENUM(begin_params, SKM_TAG_BLOCK_MODE, SKM_MODE_CBC);

    /* Allocate in-data and out-data */
    TEST_ALLOCATE_BLOCK(in_data, TEST_BLOCK_SIZE);
    TEST_ALLOCATE_BLOCK(intermediate_data, TEST_BLOCK_SIZE + TEST_KEY_SIZE / 8);
    TEST_ALLOCATE_BLOCK(final_data, TEST_BLOCK_SIZE + TEST_KEY_SIZE / 8);
    memset(in_data.buffer, 0x1c, in_data.size);
    memset(intermediate_data.buffer, 0xa2, intermediate_data.size);
    memset(final_data.buffer, 0x2d, final_data.size);

    /* Loop twice, once for encrypt and the second for decrypt */
    for (mode = 0; mode < 2; mode++) {
        out_data_size = 0;

        KeymasterTl_GetDefaultCryptoBeginSettings(&beginSettings);
        beginSettings.purpose = !mode ? SKM_PURPOSE_ENCRYPT : SKM_PURPOSE_DECRYPT;
        beginSettings.in_key_blob = key;
        beginSettings.hKeyslot = keyslot;
        beginSettings.in_params = begin_params;
        EXPECT_SUCCESS(KeymasterTl_CryptoBegin(state->handle, &beginSettings, &op_handle));
        if (!mode) {
            km_tag_value_t *tag = KM_Tag_FindFirst(beginSettings.out_params, SKM_TAG_NONCE);
            if (!tag) {
                BDBG_ERR(("%s: missing nonce", BSTD_FUNCTION));
                err = BERR_UNKNOWN;
                goto done;
            }
            /* Copy the tag into the begin params for the decrypt operation */
            TEST_TAG_ADD_BYTES(begin_params, SKM_TAG_NONCE, tag->value.blob_data_length, tag->blob_data);
        }
        TEST_DELETE_CONTEXT(beginSettings.out_params);

        for (block = 0; block < TEST_NUM_UPDATES; block++) {
            KeymasterTl_GetDefaultCryptoUpdateSettings(&updateSettings);

            if (!mode) {
                updateSettings.in_data.buffer = in_data.buffer + block * TEST_UPDATE_SIZE;
                updateSettings.in_data.size = TEST_UPDATE_SIZE;
                updateSettings.out_data.size = intermediate_data.size - block * TEST_UPDATE_SIZE;
                updateSettings.out_data.buffer = intermediate_data.buffer + block * TEST_UPDATE_SIZE;
            } else {
                updateSettings.in_data.buffer = intermediate_data.buffer + block * TEST_UPDATE_SIZE;
                updateSettings.in_data.size = TEST_UPDATE_SIZE;
                updateSettings.out_data.size = final_data.size - block * TEST_UPDATE_SIZE;
                updateSettings.out_data.buffer = final_data.buffer + block * TEST_UPDATE_SIZE;
            }
            EXPECT_SUCCESS(KeymasterTl_CryptoUpdate(state->handle, op_handle, &updateSettings));
            TEST_DELETE_CONTEXT(updateSettings.out_params);
            out_data_size += updateSettings.out_data_size;
        }

        KeymasterTl_GetDefaultCryptoFinishSettings(&finishSettings);
        EXPECT_SUCCESS(KeymasterTl_CryptoFinish(state->handle, op_handle, &finishSettings));
        TEST_DELETE_CONTEXT(finishSettings.out_params);
        out_data_size += finishSettings.out_data_size;
        op_handle = 0;

        /* Update the buffer to reflect the amount of data produced */
        if (!mode) {
            intermediate_data.size = out_data_size;
        } else {
            final_data.size = out_data_size;
        }
    }
    if (!memcmp(in_data.buffer, intermediate_data.buffer, in_data.size)) {
        BDBG_ERR(("%s: intermediate buffer compare failed", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    if ((in_data.size != final_data.size) || memcmp(in_data.buffer, final_data.buffer, in_data.size)) {
        BDBG_ERR(("%s: final buffer compare failed", BSTD_FUNCTION));
        err = BERR_UNKNOWN;
        goto done;
    }
    err = BERR_SUCCESS;

done:
    if (op_handle) {
        /* If crypto finish fails, this will throw an error too */
        (void)KeymasterTl_CryptoAbort(state->handle, op_handle);
    }
    TEST_FREE_KEYSLOT(keyslot);
    TEST_FREE_BLOCK(in_data);
    TEST_FREE_BLOCK(intermediate_data);
    TEST_FREE_BLOCK(final_data);
    TEST_FREE_BLOCK(key);
    TEST_DELETE_CONTEXT(key_params);
    TEST_DELETE_CONTEXT(begin_params);
    return err;
}

static void* test_thread(void *args)
{
    thread_state_t *state = (thread_state_t *)args;

    state->result = run_test(state);
    return NULL;
}

int main(int argc, char *argv[])
{
    KeymasterTl_Handle handle = NULL;
    KeymasterTl_InitSettings initSettings;
    NEXUS_Error err = NEXUS_SUCCESS;
    BERR_Code berr;
    int rc = 0;
    int i;
    KM_Tag_ContextHandle params = NULL;
    pthread_mutex_t mutex;
    thread_state_t thread_state[NUM_CONCURRENT_OPS];
    pthread_t threads[NUM_CONCURRENT_OPS];
#ifdef NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
#endif

    memset(&thread_state[0], 0, sizeof(thread_state));
    memset(&threads[0], 0, sizeof(threads));
    pthread_mutex_init(&mutex, NULL);

    if (argc > 2) {
        BDBG_ERR(("\tInvalid number of command line arguments (%u)\n", argc));
        return (-1);
    }

#ifdef NXCLIENT_SUPPORT
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "km_concurrent_test");
    joinSettings.ignoreStandbyRequest = true;
    err = NxClient_Join(&joinSettings);
    if (err) return (-1);
#else
    err = SAGE_app_join_nexus();
    if (err) return (-1);
#endif

    KeymasterTl_GetDefaultInitSettings(&initSettings);

    if (argv[1] != NULL) {
        if (strlen(argv[1]) > sizeof(initSettings.drm_binfile_path)) {
            BDBG_ERR(("\tString length of argument '%s' is too large (%u bytes)\n", argv[1], (uint32_t)strlen(argv[1])));
            rc = -1;
            goto done;
        }
        memcpy(initSettings.drm_binfile_path, argv[1], strlen(argv[1]));
    }

    berr = KeymasterTl_Init(&handle, &initSettings);
    if (berr != BERR_SUCCESS) {
        BDBG_ERR(("### Keymaster init failed (%x)\n", berr));
        rc = -1;
        goto done;
    }

    EXPECT_SUCCESS(KM_Tag_NewContext(&params));
    TEST_TAG_ADD_INTEGER(params, SKM_TAG_OS_VERSION, USE_OS_VERSION);
    TEST_TAG_ADD_INTEGER(params, SKM_TAG_OS_PATCHLEVEL, USE_OS_PATCHLEVEL);
    EXPECT_SUCCESS(KeymasterTl_Configure(handle, params));

    memset(&thread_state, 0, sizeof(thread_state));
    for (i = 0; i < NUM_CONCURRENT_OPS; i++) {
        thread_state[i].i = i;
        thread_state[i].result = BERR_UNKNOWN;
        thread_state[i].handle = handle;
        thread_state[i].mutex = &mutex;

        if (pthread_create(&threads[i], NULL, test_thread, (void *)&thread_state[i])) {
            BDBG_ERR(("%s: failed to spawn thread", BSTD_FUNCTION));
            err = BERR_UNKNOWN;
            goto done;
        }
    }

    for (i = 0; i < NUM_CONCURRENT_OPS; i++) {
        if (pthread_join(threads[i], NULL)) {
            BDBG_ERR(("%s: failed to join thread", BSTD_FUNCTION));
            err = BERR_UNKNOWN;
            goto done;
        }
        BDBG_LOG(("%s: thread %d finished with err 0x%x", BSTD_FUNCTION, i, thread_state[i].result));
        if (thread_state[i].result != BERR_SUCCESS) {
            BDBG_ERR(("%s: thread %d failed", BSTD_FUNCTION, i));
            err = thread_state[i].result;
            goto done;
        }
    }
    err = BERR_SUCCESS;

    BDBG_LOG(("%s: ***** ALL TESTS PASSED *****", BSTD_FUNCTION));

done:
    TEST_DELETE_CONTEXT(params);
    if (handle) {
        KeymasterTl_Uninit(handle);
    }
#ifdef NXCLIENT_SUPPORT
    NxClient_Uninit();
#else
    SAGE_app_leave_nexus();
#endif

    if (err != BERR_SUCCESS) {
        rc = -1;
    }

    if (rc) {
        BDBG_ERR(("Failure in SAGE Keymaster test\n"));
    }

    return rc;
}
