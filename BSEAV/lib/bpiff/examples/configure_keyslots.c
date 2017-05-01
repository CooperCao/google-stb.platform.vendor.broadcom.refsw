/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include <stdio.h>
#ifdef USE_LOADABLE_TA
#include <fcntl.h>
#include "nexus_otpmsp.h"

#define OTP_MSP0_VALUE_ZS (0x02)
#define OTP_MSP1_VALUE_ZS (0x02)
#endif

#include "bstd.h"
#include "bkni.h"
#include "nexus_security.h"
#include "sage_srai.h"
#include "bsagelib_crypto_types.h"
#include "test_ca_ids.h"
#include "test_sink_ids.h"

BDBG_MODULE(confiugre_keyslots);

static SRAI_PlatformHandle hCaTa = NULL;
static SRAI_ModuleHandle hCaModule = NULL;

static SRAI_PlatformHandle hSinkTa = NULL;
static SRAI_ModuleHandle hSinkModule = NULL;

#ifdef USE_LOADABLE_TA
static uint8_t* pCaTaBuf = NULL;
static uint8_t* pSinkTaBuf = NULL;
static char caTaPath[] = "sage_ta_test_ca.bin";
static char sinkTaPath[] = "sage_ta_test_sink.bin";
static char caTaDevPath[] = "sage_ta_test_ca_dev.bin";
static char sinkTaDevPath[] = "sage_ta_test_sink_dev.bin";
#endif

static int _P_Init_Ta(uint32_t platformId);
static void _P_Uninit_Ta(uint32_t platformId);

static BERR_Code _P_GenerateLoadCaKey(
    NEXUS_KeySlotHandle *pKeySlotHandle,
    uint32_t exportTAId);

static BERR_Code _P_CaGetIsolationContext(
    BSAGElib_SharedBlock *block);

static BERR_Code _P_SinkLoadKey(
    NEXUS_KeySlotHandle *pKeySlotHandle,
    BSAGElib_Crypto_Operation_e operation,
    bool bIsPacketMode,
    BSAGElib_SharedBlock *block);

static void _P_CaFreeKeySlot(NEXUS_KeySlotHandle hKeySlot);
static void _P_SinkFreeKeySlot(NEXUS_KeySlotHandle hKeySlot);

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
    bool isZS = false;
    NEXUS_ReadMspParms readMspParms;
    NEXUS_ReadMspIO readMsp0;
    NEXUS_ReadMspIO readMsp1;

    readMspParms.readMspEnum = NEXUS_OtpCmdMsp_eReserved233;
    NEXUS_Security_ReadMSP(&readMspParms,&readMsp0);

    readMspParms.readMspEnum = NEXUS_OtpCmdMsp_eReserved234;
    NEXUS_Security_ReadMSP(&readMspParms,&readMsp1);

    if ((readMsp0.mspDataBuf[3] == OTP_MSP0_VALUE_ZS)
        && (readMsp1.mspDataBuf[3] == OTP_MSP1_VALUE_ZS)) {
        isZS = true;
    }

    switch (platformId) {
      case SAGE_PLATFORM_ID_TEST_CA:
        if (isZS)
        {
            filePath = caTaDevPath;
        }
        else
        {
            filePath = caTaPath;
        }
        break;
      case SAGE_PLATFORM_ID_TEST_SINK:
        if (isZS)
        {
            filePath = sinkTaDevPath;
        }
        else
        {
            filePath = sinkTaPath;
        }
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
      case SAGE_PLATFORM_ID_TEST_SINK:
        sage_rc = SRAI_Module_Init(handle,
            TEST_SINK_MODULE, container, &hSinkModule);
        hSinkTa = handle;
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
      case SAGE_PLATFORM_ID_TEST_SINK:
#ifdef USE_LOADABLE_TA
        pBuf = pSinkTaBuf;
#endif
        handle = hSinkTa;
        module = hSinkModule;
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
    keyslotSettings.keySlotType = NEXUS_SecurityKeySlotType_eType3;

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

    container->basicIn[1] = exportTAId;

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

static BERR_Code _P_CaGetIsolationContext(BSAGElib_SharedBlock *block)
{
    BERR_Code rc = BERR_SUCCESS;

    BSAGElib_InOutContainer *container = NULL;

    container = SRAI_Container_Allocate();
    if (container == NULL) {
        BDBG_ERR(("%s - cannot allocate container", __FUNCTION__));
        rc = BSAGE_ERR_CONTAINER_REQUIRED;
        goto handle_error;
    }

    container->blocks[0].len = block->len;
    container->blocks[0].data.ptr = block->data.ptr;

    rc = SRAI_Module_ProcessCommand(hCaModule, TestCa_CommandId_GetIsolationContext, container);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - Cannot process TestCa_CommandId_GetIsolationContext command", __FUNCTION__));
        goto handle_error;
    }
    else if (container->basicOut[0] != BERR_SUCCESS) {
        BDBG_ERR(("%s - Error in TestCa_CommandId_GetIsolationContext command: %s", __FUNCTION__, BSAGElib_Tools_ReturnCodeToString(container->basicOut[0])));
        rc = container->basicOut[0];
        goto handle_error;
    }

    BDBG_MSG(("%s - line = %u", __FUNCTION__, __LINE__));

handle_error:

    if (container != NULL) {
        SRAI_Container_Free(container);
        container = NULL;
    }

    return rc;
}

static BERR_Code _P_SinkLoadKey(
    NEXUS_KeySlotHandle *pKeySlotHandle,
    BSAGElib_Crypto_Operation_e operation,
    bool bIsPacketMode,
    BSAGElib_SharedBlock *block)
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
    keyslotSettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;

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
    container->basicIn[1] = operation;
    container->basicIn[2] = bIsPacketMode;

    if (block != NULL) {
        container->blocks[0].len = block->len;
        container->blocks[0].data.ptr = block->data.ptr;
    }

    rc = SRAI_Module_ProcessCommand(hSinkModule, TestSink_CommandId_LoadKey, container);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - Cannot process TestSink_CommandId_LoadKey command", __FUNCTION__));
        goto handle_error;
    }
    else if (container->basicOut[0] != BERR_SUCCESS) {
        BDBG_ERR(("%s - Error in TestSink_CommandId_LoadKey command: %s", __FUNCTION__, BSAGElib_Tools_ReturnCodeToString(container->basicOut[0])));
        rc = container->basicOut[0];
        goto handle_error;
    }

    BDBG_LOG(("%s - line = %u basic = %u ", __FUNCTION__, __LINE__, container->basicIn[0]));

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

static void _P_SinkFreeKeySlot(NEXUS_KeySlotHandle hKeySlot)
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

    rc = SRAI_Module_ProcessCommand(hSinkModule, TestSink_CommandId_FreeKeySlot, container);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - Cannot process TestSink_CommandId_FreeKeySlot: command", __FUNCTION__));
        goto handle_error;
    }
    else if (container->basicOut[0] != BERR_SUCCESS) {
        BDBG_ERR(("%s - Error in TestSink_CommandId_FreeKeySlot: command: %s", __FUNCTION__, BSAGElib_Tools_ReturnCodeToString(container->basicOut[0])));
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
    NEXUS_KeySlotHandle *pM2mKeySlotHandle,
    NEXUS_KeySlotHandle *pCaKeySlotHandle,
    uint32_t exportTAid,
    bool bIsPacketMode,
    BSAGElib_SharedBlock *block)
{
    int ret_code = -1;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_Crypto_Operation_e operation;

    if (playback == 0) {
        rc = _P_Init_Ta(SAGE_PLATFORM_ID_TEST_CA);
        if (rc != BERR_SUCCESS) {
            fprintf(stderr, "Failed to initialize TestCa\n");
            goto error;
        }
    }
    if (pM2mKeySlotHandle != NULL) {
        rc = _P_Init_Ta(SAGE_PLATFORM_ID_TEST_SINK);
        if (rc != BERR_SUCCESS) {
            fprintf(stderr, "Failed to initialize TestSink\n");
            goto error;
        }
    }

    /* Allocate and configure CA Key */
    if (pCaKeySlotHandle && !playback /* only decrypt CA streams on recorder app */)
    {
        rc = _P_GenerateLoadCaKey(pCaKeySlotHandle, exportTAid);
        if (rc != BERR_SUCCESS) {
            fprintf(stderr, "Generate and load CA key FAILED\n");
            goto error;
        }
    }

    /* Load Key for Sink TA */
    if (playback) {
        operation = BSAGElib_Crypto_Operation_eDecrypt;
    }
    else {
        operation = BSAGElib_Crypto_Operation_eEncrypt;
        rc = _P_CaGetIsolationContext(block);
    }

    if (pM2mKeySlotHandle != NULL) {
        rc = _P_SinkLoadKey(pM2mKeySlotHandle, operation, bIsPacketMode, block);
        if (rc != BERR_SUCCESS) {
            fprintf(stderr, "Load Key FAILED\n");
            goto error;
        }
    }

    ret_code = 0;

error:
    return ret_code;
}

void clean_keyslots(
    NEXUS_KeySlotHandle m2mKeySlotHandle,
    NEXUS_KeySlotHandle CaKeySlotHandle)
{
    if (hCaModule) {
        if (CaKeySlotHandle != NULL) {
            _P_CaFreeKeySlot(CaKeySlotHandle);
            CaKeySlotHandle = NULL;
        }

        _P_Uninit_Ta(SAGE_PLATFORM_ID_TEST_CA);
    }

    if (m2mKeySlotHandle != NULL) {
        _P_SinkFreeKeySlot(m2mKeySlotHandle);
        m2mKeySlotHandle = NULL;

        _P_Uninit_Ta(SAGE_PLATFORM_ID_TEST_SINK);
    }

    return;
}
