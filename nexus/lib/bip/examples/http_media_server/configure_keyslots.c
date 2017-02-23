/******************************************************************************
* Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
******************************************************************************/

#include <stdio.h>
#ifdef USE_LOADABLE_TA
#include <fcntl.h>
#endif

#include "bstd.h"
#include "bkni.h"
#include "nexus_security.h"
#include "sage_srai.h"
#include "bsagelib_crypto_types.h"
#if 0
#include "test_ca_ids.h"
#endif
BDBG_MODULE(confiugre_keyslots);

/*Temp defines until figure out how to access test_ca_ids.h here */
#define SAGE_PLATFORM_ID_TEST_CA 0x88

#define ISOLATION_CONTEXT_SIZE 28


enum Test_Ca_module {
    TEST_CA_MODULE = 0x1
};

enum {
    TestCa_CommandId_LoadCaKey = 0x1,
    TestCa_CommandId_GetIsolationContext = 0x2,
    TestCa_CommandId_FreeKeySlot = 0x3
};




static SRAI_PlatformHandle hCaTa = NULL;
static SRAI_ModuleHandle hCaModule = NULL;

#ifdef USE_LOADABLE_TA
static uint8_t* pCaTaBuf = NULL;
static char caTaPath[] = "sage_ta_test_ca.bin";
#endif

static int _P_Init_Ta(uint32_t platformId);
static void _P_Uninit_Ta(uint32_t platformId);

static BERR_Code _P_GenerateLoadCaKey(NEXUS_KeySlotHandle *pKeySlotHandle, bool bIsolation, uint32_t exportTAId);
static void _P_CaFreeKeySlot(NEXUS_KeySlotHandle hKeySlot);

static int _P_Init_Ta(uint32_t platformId)
{
    int rc = 0;
    BERR_Code sage_rc;
    BSAGElib_State state;
    SRAI_PlatformHandle handle = NULL;
    BSAGElib_InOutContainer *container = NULL;

#ifdef USE_LOADABLE_TA
    int fd = -1;
    char* filePath;
    uint32_t taSize = 0;
    uint8_t* pBuf = NULL;

    switch (platformId) {
      case SAGE_PLATFORM_ID_TEST_CA:
        filePath = caTaPath;
        break;
      default:
        rc = -1;
        goto cleanup;
    }

    fd = open(filePath, O_RDONLY);
    if (fd == -1) {
        rc = -2;
        goto cleanup;
    }

    off_t pos = lseek(fd, 0, SEEK_END);
    if (pos == -1) {
        rc = -3;
        goto cleanup;
    }
    taSize = (uint32_t)pos;
    pBuf = SRAI_Memory_Allocate(taSize, SRAI_MemoryType_Shared);
    if (pBuf == NULL) {
        rc = -4;
        goto cleanup;
    }

    pos = lseek(fd, 0, SEEK_SET);
    if (pos != 0) {
        rc = -5;
        goto cleanup;
    }

    ssize_t readSize = read(fd, (void *)pBuf, (size_t)taSize);
    if (readSize != (ssize_t)taSize) {
        rc = -6;
        goto cleanup;
    }

cleanup:
    if (fd != -1) {
        close(fd);
        fd = -1;
    }

    if (rc) {
        if (pBuf) {
            SRAI_Memory_Free(pBuf);
        }
        fprintf(stderr, "%s ERROR #%d\n", __FUNCTION__, rc);
        return rc;
    }

    /* Install the platform first */
    sage_rc = SRAI_Platform_Install(platformId, pBuf, taSize);
    if (sage_rc != BERR_SUCCESS) {
        rc = -7;
    }
#else /* USE_LOADABLE_TA */
    container = SRAI_Container_Allocate();
    if (container == NULL) {
        rc = 1;
        goto handle_error;
    }
#endif /* USE_LOADABLE_TA */

    /* Open the platform first */
    sage_rc = SRAI_Platform_Open(platformId, &state, &handle);
    if (sage_rc != BERR_SUCCESS) {
        rc = -9;
        goto handle_error;
    }

    /* Check init state */
    if (state != BSAGElib_State_eInit) {
        /* Not yet initialized: send init command*/
        sage_rc = SRAI_Platform_Init(handle, NULL);
        if (sage_rc != BERR_SUCCESS) {
            rc = -10;
            goto handle_error;
        }
    }

    /* Initialize test module */
    switch (platformId) {
      case SAGE_PLATFORM_ID_TEST_CA:
        sage_rc = SRAI_Module_Init(handle,
            TEST_CA_MODULE, container, &hCaModule);
        hCaTa = handle;
        break;
    }
    if (sage_rc != BERR_SUCCESS) {
        rc = -11;
        goto handle_error;
    }
#ifndef USE_LOADABLE_TA
    if (container->basicOut[0] != 0) {
        rc = -12;
        goto handle_error;
    }
#endif

handle_error:

    if (rc) {
        fprintf(stderr, "%s ERROR #%d\n", __FUNCTION__, rc);
        /* error: cleanup */
        _P_Uninit_Ta(platformId);
    }

    if(container != NULL) {
        SRAI_Container_Free(container);
    }

    return rc;
}

static void _P_Uninit_Ta(uint32_t platformId)
{
    BERR_Code sage_rc = BERR_SUCCESS;
    SRAI_PlatformHandle handle = NULL;
    SRAI_ModuleHandle module = NULL;
#ifdef USE_LOADABLE_TA
    uint8_t* pBuf = NULL;
#endif

    switch (platformId) {
      case SAGE_PLATFORM_ID_TEST_CA:
#ifdef USE_LOADABLE_TA
        pBuf = pCaTaBuf;
#endif
        handle = hCaTa;
        module = hCaModule;
        break;
    }

    if (module) {
        SRAI_Module_Uninit(module);
        module = NULL;
    }

    if (handle) {
        SRAI_Platform_Close(handle);
        handle = NULL;
    }

#ifdef USE_LOADABLE_TA
    sage_rc = SRAI_Platform_UnInstall(platformId);
    if (sage_rc != BERR_SUCCESS) {
        fprintf(stderr, "%s ERROR #%d\n", __FUNCTION__, sage_rc);
    }

    if (pBuf) {
        SRAI_Memory_Free(pBuf);
        pBuf = NULL;
    }
#endif

    return;
}

static BERR_Code
_P_GenerateLoadCaKey(
    NEXUS_KeySlotHandle *pKeySlotHandle,
    bool bIsolation,
    uint32_t exportTAId)
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_SecurityKeySlotSettings keyslotSettings;
    NEXUS_SecurityKeySlotInfo keyslotInfo;
    NEXUS_KeySlotHandle keySlotHandle = NULL;

    BSAGElib_InOutContainer *container = NULL;

    container = SRAI_Container_Allocate();
    if (container == NULL) {
        BDBG_ERR(("%s - cannot allocate container", __FUNCTION__));
        rc = BSAGE_ERR_CONTAINER_REQUIRED;
        goto handle_error;
    }

    NEXUS_Security_GetDefaultKeySlotSettings(&keyslotSettings);
    keyslotSettings.client = NEXUS_SecurityClientType_eSage;
    keyslotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;

    keySlotHandle = NEXUS_Security_AllocateKeySlot(&keyslotSettings);
    if (keySlotHandle == NULL) {
        BDBG_ERR(("%s - Error allocating keyslot", __FUNCTION__));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }

    NEXUS_KeySlot_GetInfo(keySlotHandle, &keyslotInfo);

    BDBG_LOG(("%s - Keyslot index=%u, engine=%u",
              __FUNCTION__, keyslotInfo.keySlotNumber, keyslotInfo.keySlotEngine));

    container->basicIn[0] = keyslotInfo.keySlotNumber;

    /* Isolation flag must be set within Sage TA instead of host,
       but Broadcom's test TA is taking it from host for testing purpose
     */
    container->basicIn[1] = bIsolation;
    container->basicIn[2] = exportTAId;

    rc = SRAI_Module_ProcessCommand(hCaModule, TestCa_CommandId_LoadCaKey, container);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - Cannot process TestCa_CommandId_LoadCaKey command", __FUNCTION__));
        goto handle_error;
    }
    else if (container->basicOut[0] != BERR_SUCCESS) {
        BDBG_ERR(("%s - Error in TestCa_CommandId_LoadCaKey command: %s", __FUNCTION__, BSAGElib_Tools_ReturnCodeToString(container->basicOut[0])));
        rc = container->basicOut[0];
        goto handle_error;
    }

    BDBG_MSG(("%s - line = %u basic = %u ", __FUNCTION__, __LINE__, container->basicIn[0]));

    *pKeySlotHandle = keySlotHandle;
    keySlotHandle = NULL; /* eat-up keyslot on success */

handle_error:

    if (container != NULL) {
        SRAI_Container_Free(container);
        container = NULL;
    }

    if (keySlotHandle) {
        NEXUS_Security_FreeKeySlot(keySlotHandle);
        keySlotHandle = NULL;
    }

    return rc;
}

static void _P_CaFreeKeySlot(NEXUS_KeySlotHandle hKeySlot)
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_SecurityKeySlotInfo keyslotInfo;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_LOG(("%s enter", __FUNCTION__));

    if (hKeySlot == NULL) {
        BDBG_ERR(("%s - Nothing to do, keyslot handle is already NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto handle_error;
    }

    container = SRAI_Container_Allocate();
    if (container == NULL) {
        BDBG_ERR(("%s - cannot allocate container", __FUNCTION__));
        rc = BSAGE_ERR_CONTAINER_REQUIRED;
        goto handle_error;
    }

    /* pass the index again as a sanity check */
    NEXUS_KeySlot_GetInfo(hKeySlot, &keyslotInfo);
    BDBG_LOG(("%s - Keyslot index = '%u'", __FUNCTION__, keyslotInfo.keySlotNumber));
    container->basicIn[0] = keyslotInfo.keySlotNumber;
    container->basicIn[1] = keyslotInfo.keySlotEngine;

    rc = SRAI_Module_ProcessCommand(hCaModule, TestCa_CommandId_FreeKeySlot, container);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - Cannot process TestCa_CommandId_FreeKeySlot command", __FUNCTION__));
        goto handle_error;
    }
    else if (container->basicOut[0] != BERR_SUCCESS) {
        BDBG_ERR(("%s - Error in TestCa_CommandId_FreeKeySlot command: %s", __FUNCTION__, BSAGElib_Tools_ReturnCodeToString(container->basicOut[0])));
        rc = container->basicOut[0];
        goto handle_error;
    }

handle_error:

    /* free up the host keyslot handle after SAGE side returns */
    if (hKeySlot != NULL) {
        NEXUS_Security_FreeKeySlot(hKeySlot);
    }

    if (container != NULL) {
        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LOG(("%s leave", __FUNCTION__));
    return;
}

int prepare_keyslots(
    int playback,
    NEXUS_KeySlotHandle *pCaKeySlotHandle,
    uint32_t exportTAid,
    bool bIsolation
    )
{
    int ret_code = -1;
    BERR_Code rc = BERR_SUCCESS;

    if (playback == 0)
        rc = _P_Init_Ta(SAGE_PLATFORM_ID_TEST_CA);
    BDBG_ASSERT(rc == BERR_SUCCESS);

    /* Allocate and configure CA Key */
    if (pCaKeySlotHandle && !playback /* only decrypt CA streams on recorder app */)
    {
        rc = _P_GenerateLoadCaKey(pCaKeySlotHandle, bIsolation, exportTAid);
        if (rc != BERR_SUCCESS) {
            fprintf(stderr, "Generate and load CA key FAILED\n");
            goto error;
        }
    }

    ret_code = 0;

error:
    return ret_code;
}

void clean_keyslots(
    NEXUS_KeySlotHandle CaKeySlotHandle)
{
    if (hCaModule) {
        if (CaKeySlotHandle != NULL) {
            _P_CaFreeKeySlot(CaKeySlotHandle);
            CaKeySlotHandle = NULL;
        }

        _P_Uninit_Ta(SAGE_PLATFORM_ID_TEST_CA);
    }
    return;
}
