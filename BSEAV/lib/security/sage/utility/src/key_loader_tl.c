/******************************************************************************
 * (c) 2014 Broadcom Corporation
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
 ******************************************************************************/

#include "blst_list.h"
#include "utility_platform.h"
#include "utility_ids.h"
#include "key_loader_tl.h"
#include "nexus_memory.h"

BDBG_MODULE(key_loader_tl);


struct KeyLoaderTl_P_Instance
{
    BDBG_OBJECT(KeyLoaderTl_P_Instance)
    SRAI_ModuleHandle moduleHandle;
};

BDBG_OBJECT_ID_DECLARE(KeyLoaderTl_P_Instance);
BDBG_OBJECT_ID(KeyLoaderTl_P_Instance);

static void _KeyLoaderTl_ContextDelete(KeyLoaderTl_Handle hKeyLoaderTl);
static KeyLoaderTl_Handle _KeyLoaderTl_ContextNew(void);

void KeyLoaderTl_GetDefaultSettings(KeyLoaderTlSettings *pKeyLoaderModuleSettings)
{
    BDBG_ENTER(KeyLoaderTl_GetDefaultSettings);

    BDBG_ASSERT(pKeyLoaderModuleSettings);
    BKNI_Memset((uint8_t *)pKeyLoaderModuleSettings, 0x00, sizeof(KeyLoaderTlSettings));

    BDBG_LEAVE(KeyLoaderTl_GetDefaultSettings);
    return;
}

static KeyLoaderTl_Handle _KeyLoaderTl_ContextNew(void)
{
    BERR_Code magnum_rc;
    NEXUS_Error nexus_rc;
    BSAGElib_InOutContainer *container = NULL;
    KeyLoaderTl_Handle hKeyLoaderTl = NULL;

    nexus_rc = NEXUS_Memory_Allocate(sizeof(*hKeyLoaderTl), NULL, (void **)&hKeyLoaderTl);
    if (nexus_rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - cannot allocate memory for KeyLoaderTl context", __FUNCTION__));
        magnum_rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto ErrorExit;
    }
    hKeyLoaderTl->moduleHandle = NULL;

    BDBG_OBJECT_SET(hKeyLoaderTl, KeyLoaderTl_P_Instance);

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        magnum_rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto ErrorExit;
    }

    /* Initialize SAGE KeyLoader module */
    magnum_rc = Utility_ModuleInit(Utility_ModuleId_eKeyLoader,
                                   NULL,
                                   container,
                                   &hKeyLoaderTl->moduleHandle);
    if(magnum_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error initializing KeyLoader TL module (0x%08x)", __FUNCTION__, container->basicOut[0]));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL) {
        SRAI_Container_Free(container);
        container = NULL;
    }

    if (magnum_rc != BERR_SUCCESS && hKeyLoaderTl) {
        _KeyLoaderTl_ContextDelete(hKeyLoaderTl);
        hKeyLoaderTl = NULL;
    }

    return hKeyLoaderTl;
}

static void _KeyLoaderTl_ContextDelete(KeyLoaderTl_Handle hKeyLoaderTl)
{
    if (hKeyLoaderTl)
    {

        if (hKeyLoaderTl->moduleHandle)
        {
            Utility_ModuleUninit(hKeyLoaderTl->moduleHandle);
            hKeyLoaderTl->moduleHandle = NULL;
        }

        BDBG_OBJECT_DESTROY(hKeyLoaderTl, KeyLoaderTl_P_Instance);
        NEXUS_Memory_Free(hKeyLoaderTl);
        hKeyLoaderTl = NULL;
    }
}

BERR_Code KeyLoaderTl_Init(KeyLoaderTl_Handle *pKeyLoaderTlHandle, KeyLoaderTlSettings *pKeyLoaderModuleSettings)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(KeyLoaderTl_Init);

    if (pKeyLoaderTlHandle == NULL)
    {
        BDBG_ERR(("%s - Parameter pHandle is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    if(pKeyLoaderModuleSettings == NULL)
    {
        BDBG_ERR(("%s - Parameter settings are NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    *pKeyLoaderTlHandle = _KeyLoaderTl_ContextNew();
    if (*pKeyLoaderTlHandle == NULL)
    {
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto ErrorExit;
    }

ErrorExit:

    BDBG_LEAVE(KeyLoaderTl_Init);
    return rc;
}

void KeyLoaderTl_Uninit(KeyLoaderTl_Handle hKeyLoaderTl)
{
    BDBG_ENTER(KeyLoaderTl_Uninit);

    BDBG_OBJECT_ASSERT(hKeyLoaderTl, KeyLoaderTl_P_Instance);
    _KeyLoaderTl_ContextDelete(hKeyLoaderTl);

    BDBG_LEAVE(KeyLoaderTl_Uninit);
    return;
}


void KeyLoader_GetDefaultConfigKeySlotSettings(KeyLoader_KeySlotConfigSettings *pKeySlotConfigSettings)
{
    BDBG_ENTER(KeyLoader_GetDefaultConfigKeySlotSettings);

    if(pKeySlotConfigSettings != NULL){
        BKNI_Memset((uint8_t *)pKeySlotConfigSettings, 0x00, sizeof(KeyLoader_KeySlotConfigSettings));
    }

    pKeySlotConfigSettings->aesCounterSize = BSAGElib_Crypto_AesCounterSize_e64;

    BDBG_LEAVE(KeyLoader_GetDefaultConfigKeySlotSettings);
    return;
}


BERR_Code KeyLoader_AllocAndConfigKeySlot(KeyLoaderTl_Handle hKeyLoaderTl,
                                          NEXUS_KeySlotHandle *pKeySlotHandle,
                                          KeyLoader_KeySlotConfigSettings *pKeySlotConfigSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_SecurityKeySlotSettings keyslotSettings;
    NEXUS_SecurityKeySlotInfo keyslotInfo;
    NEXUS_KeySlotHandle keySlotHandle = NULL;

    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(KeyLoader_AllocAndConfigKeySlot);

    BDBG_OBJECT_ASSERT(hKeyLoaderTl, KeyLoaderTl_P_Instance);
    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - cannot allocate container", __FUNCTION__));
        rc = BSAGE_ERR_CONTAINER_REQUIRED;
        goto handle_error;
    }

    NEXUS_Security_GetDefaultKeySlotSettings(&keyslotSettings);
    keyslotSettings.client = NEXUS_SecurityClientType_eSage;
    keyslotSettings.keySlotEngine = pKeySlotConfigSettings->engine;

    keySlotHandle = NEXUS_Security_AllocateKeySlot(&keyslotSettings);
    if(keySlotHandle == NULL)
    {
        BDBG_ERR(("%s - Error allocating keyslot", __FUNCTION__));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }

    NEXUS_KeySlot_GetInfo(keySlotHandle, &keyslotInfo);

    BDBG_LOG(("%s - Keyslot index=%u, engine=%u",
              __FUNCTION__, keyslotInfo.keySlotNumber, keyslotInfo.keySlotEngine));

    container->basicIn[0] = keyslotInfo.keySlotNumber;

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(sizeof(KeyLoader_KeySlotConfigSettings), SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate '%u' bytes of memory ^^^^^^^^^^^^^^^^^^^", __FUNCTION__, sizeof(KeyLoader_KeySlotConfigSettings)));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }
    container->blocks[0].len = sizeof(KeyLoader_KeySlotConfigSettings);

    BKNI_Memcpy(container->blocks[0].data.ptr, (uint8_t*)pKeySlotConfigSettings, sizeof(KeyLoader_KeySlotConfigSettings));

    rc = SRAI_Module_ProcessCommand(hKeyLoaderTl->moduleHandle, KeyLoader_CommandId_eAllocAndConfig, container);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Cannot process eAllocAndConfig command", __FUNCTION__));
        goto handle_error;
    }
    else if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error in eAllocAndConfig command: %s", __FUNCTION__, BSAGElib_Tools_ReturnCodeToString(container->basicOut[0])));
        rc = container->basicOut[0];
        goto handle_error;
    }
    else if (container->basicOut[1] == KEY_LOADER_INVALID_KEYSLOT_INDEX)
    {
        BDBG_ERR(("%s - eAllocAndConfig returned an invalid key slot", __FUNCTION__));
        rc = BSAGE_ERR_INTERNAL;
        goto handle_error;
    }

    BDBG_MSG(("%s - line = %u basic = %u ", __FUNCTION__, __LINE__, container->basicIn[0]));

    *pKeySlotHandle = keySlotHandle;
    keySlotHandle = NULL; /* eat-up keyslot on success */

handle_error:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    if (keySlotHandle)
    {
        NEXUS_Security_FreeKeySlot(keySlotHandle);
        keySlotHandle = NULL;
    }

    BDBG_LEAVE(KeyLoader_AllocAndConfigKeySlot);
    return rc;
}


void KeyLoader_FreeKeySlot(KeyLoaderTl_Handle hKeyLoaderTl, NEXUS_KeySlotHandle hKeySlot)
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_SecurityKeySlotInfo keyslotInfo;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(KeyLoader_FreeKeySlot);

    BDBG_OBJECT_ASSERT(hKeyLoaderTl, KeyLoaderTl_P_Instance);
    if (hKeySlot == NULL)
    {
        BDBG_ERR(("%s - Nothing to do, keyslot handle is already NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto handle_error;
    }

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - cannot allocate container", __FUNCTION__));
        rc = BSAGE_ERR_CONTAINER_REQUIRED;
        goto handle_error;
    }

    /* pass the index again as a sanity check */
    NEXUS_KeySlot_GetInfo(hKeySlot, &keyslotInfo);
    BDBG_LOG(("%s - Keyslot index = '%u'", __FUNCTION__, keyslotInfo.keySlotNumber));
    container->basicIn[0] = keyslotInfo.keySlotNumber;
    container->basicIn[1] = keyslotInfo.keySlotEngine;

    rc = SRAI_Module_ProcessCommand(hKeyLoaderTl->moduleHandle, KeyLoader_CommandId_eFreeKeyslot, container);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Cannot process eFreeKeyslot command", __FUNCTION__));
        goto handle_error;
    }
    else if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error in eFreeKeyslot command: %s", __FUNCTION__, BSAGElib_Tools_ReturnCodeToString(container->basicOut[0])));
        rc = container->basicOut[0];
        goto handle_error;
    }
    else if (container->basicOut[1] == KEY_LOADER_INVALID_KEYSLOT_INDEX)
    {
        BDBG_ERR(("%s - eFreeKeyslot returned an invalid key slot", __FUNCTION__));
        rc = BSAGE_ERR_INTERNAL;
        goto handle_error;
    }

handle_error:

    /* free up the host keyslot handle after SAGE side returns */
    if (hKeySlot != NULL){
        NEXUS_Security_FreeKeySlot(hKeySlot);
    }

    if(container != NULL)
    {
        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(KeyLoader_FreeKeySlot);
    return;
}



void KeyLoader_GetDefaultWrappedKeySettings(KeyLoader_WrappedKeySettings *pWrappedKeySettings)
{
    BDBG_ENTER(KeyLoader_GetDefaultWrappedKeySettings);

    if(pWrappedKeySettings != NULL){
        BKNI_Memset((uint8_t *)pWrappedKeySettings, 0x00, sizeof(KeyLoader_WrappedKeySettings));

        pWrappedKeySettings->keyLength = 16;
    }

    BDBG_LEAVE(KeyLoader_GetDefaultWrappedKeySettings);
    return;
}


BERR_Code KeyLoader_LoadWrappedKey(KeyLoaderTl_Handle hKeyLoaderTl,
                                   NEXUS_KeySlotHandle hKeySlot,
                                   KeyLoader_WrappedKeySettings *pWrappedKeySettings)
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_SecurityKeySlotInfo keyslotInfo;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(KeyLoader_LoadWrappedKey);

    BDBG_OBJECT_ASSERT(hKeyLoaderTl, KeyLoaderTl_P_Instance);
    if (pWrappedKeySettings == NULL)
    {
        BDBG_ERR(("%s - Settings structure pointer is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto handle_error;
    }

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - cannot allocate container", __FUNCTION__));
        rc = BSAGE_ERR_CONTAINER_REQUIRED;
        goto handle_error;
    }

    /* pass the index */
    NEXUS_KeySlot_GetInfo(hKeySlot, &keyslotInfo);
    BDBG_LOG(("%s - Keyslot index=%u, engine=%u",
              __FUNCTION__, keyslotInfo.keySlotNumber, keyslotInfo.keySlotEngine));
    container->basicIn[0] = keyslotInfo.keySlotNumber;
    container->basicIn[1] = keyslotInfo.keySlotEngine;


    container->blocks[0].data.ptr = SRAI_Memory_Allocate(sizeof(KeyLoader_WrappedKeySettings), SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate '%u' bytes of memory ^^^^^^^^^^^^^^^^^^^", __FUNCTION__, sizeof(KeyLoader_WrappedKeySettings)));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }
    container->blocks[0].len = sizeof(KeyLoader_WrappedKeySettings);

    BKNI_Memcpy(container->blocks[0].data.ptr, (uint8_t*)pWrappedKeySettings, sizeof(KeyLoader_WrappedKeySettings));

    rc = SRAI_Module_ProcessCommand(hKeyLoaderTl->moduleHandle, KeyLoader_CommandId_eLoadWrappedKey, container);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Cannot process eLoadWrappedKey command", __FUNCTION__));
        goto handle_error;
    }
    else if (container->basicOut[0] != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error in eLoadWrappedKey command: %s", __FUNCTION__, BSAGElib_Tools_ReturnCodeToString(container->basicOut[0])));
        rc = container->basicOut[0];
        goto handle_error;
    }
    else if (container->basicOut[1] == KEY_LOADER_INVALID_KEYSLOT_INDEX)
    {
        BDBG_ERR(("%s - eLoadWrappedKey returned an invalid key slot", __FUNCTION__));
        rc = BSAGE_ERR_INTERNAL;
        goto handle_error;
    }

handle_error:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(KeyLoader_LoadWrappedKey);
    return rc;
}



void KeyLoader_GetDefaultUpdateIvSettings(KeyLoader_UpdateIvSettings *pUpdateIvSettings)
{
    BDBG_ENTER(KeyLoader_GetDefaultUpdateIvSettings);

    if(pUpdateIvSettings != NULL){
        BKNI_Memset((uint8_t *)pUpdateIvSettings, 0x00, sizeof(KeyLoader_UpdateIvSettings));
    }

    BDBG_LEAVE(KeyLoader_GetDefaultUpdateIvSettings);
    return;
}

BERR_Code KeyLoader_UpdateIv(KeyLoaderTl_Handle hKeyLoaderTl,
                                   NEXUS_KeySlotHandle hKeySlot,
                                   KeyLoader_UpdateIvSettings *pUpdateIvSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_SecurityKeySlotInfo keyslotInfo;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(KeyLoader_UpdateIv);

    BDBG_OBJECT_ASSERT(hKeyLoaderTl, KeyLoaderTl_P_Instance);
    if (pUpdateIvSettings == NULL)
    {
        BDBG_ERR(("%s - Settings structure pointer is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto handle_error;
    }

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - cannot allocate container", __FUNCTION__));
        rc = BSAGE_ERR_CONTAINER_REQUIRED;
        goto handle_error;
    }

    /* pass the index */
    NEXUS_KeySlot_GetInfo(hKeySlot, &keyslotInfo);
    BDBG_LOG(("%s - Keyslot index=%u, engine=%u",
              __FUNCTION__, keyslotInfo.keySlotNumber, keyslotInfo.keySlotEngine));
    container->basicIn[0] = keyslotInfo.keySlotNumber;
    container->basicIn[1] = keyslotInfo.keySlotEngine;


    container->blocks[0].data.ptr = SRAI_Memory_Allocate(sizeof(KeyLoader_UpdateIvSettings), SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate '%u' bytes of memory ^^^^^^^^^^^^^^^^^^^", __FUNCTION__, sizeof(KeyLoader_UpdateIvSettings)));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }
    container->blocks[0].len = sizeof(KeyLoader_UpdateIvSettings);

    BKNI_Memcpy(container->blocks[0].data.ptr, (uint8_t*)pUpdateIvSettings, sizeof(KeyLoader_UpdateIvSettings));

    rc = SRAI_Module_ProcessCommand(hKeyLoaderTl->moduleHandle, KeyLoader_CommandId_eUpdateIv, container);
    if ((rc != BERR_SUCCESS) || (container->basicOut[0] != BERR_SUCCESS))
    {
        BDBG_ERR(("%s - Error sending update IV to SAGE Key Loader module.  Returned keyslot index '0x%08x'", __FUNCTION__, container->basicOut[1]));
        goto handle_error;
    }

handle_error:

    if(container != NULL)
    {
        if(container->basicOut[0] != BERR_SUCCESS){
            BDBG_ERR(("%s", BSAGElib_Tools_ReturnCodeToString(container->basicOut[0])));
        }

        if(container->blocks[0].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(KeyLoader_UpdateIv);
    return rc;
}
