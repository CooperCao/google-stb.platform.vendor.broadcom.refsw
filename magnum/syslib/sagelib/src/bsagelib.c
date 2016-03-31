/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "bkni.h"

#include "blst_slist.h"

#include "bsagelib.h"
#include "bsagelib_client.h"
#include "bsagelib_priv.h"
#include "bsagelib_shared_types.h"

#include "bhsm_otpmsp.h"
#include "bhsm_keyladder_enc.h"

BDBG_MODULE(BSAGElib);


BDBG_OBJECT_ID(BSAGElib_P_Instance);
BDBG_OBJECT_ID(BSAGElib_P_Client);


/* Local functions */
static BERR_Code BSAGElib_P_TimerInit(BSAGElib_Handle hSAGElib);
static void BSAGElib_P_TimerUninit(BSAGElib_Handle hSAGElib);
static void BSAGElib_P_Close(BSAGElib_Handle hSAGElib);
static void BSAGElib_P_CloseClient(BSAGElib_ClientHandle hSAGElibClient, int collector);
static BERR_Code BSAGElib_P_GetChipsetType(BSAGElib_Handle hSAGElib);
static void BSAGElib_P_FreeSageVkl(BSAGElib_Handle hSAGElib, BCMD_VKLID_e vklId);
static BCMD_VKLID_e BSAGElib_P_AllocSageVkl(BSAGElib_Handle hSAGElib);


static BERR_Code
BSAGElib_P_TimerInit(
    BSAGElib_Handle hSAGElib)
{
    BERR_Code rc;
    BTMR_TimerSettings timerSettings = { BTMR_Type_eSharedFreeRun, NULL, NULL, 0, false };
    rc = BTMR_CreateTimer(hSAGElib->core_handles.hTmr, &hSAGElib->hTimer, &timerSettings);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BTMR_CreateTimer failure %d", __FUNCTION__, rc));
    }
    return rc;
}

static void
BSAGElib_P_TimerUninit(
    BSAGElib_Handle hSAGElib)
{
    if (hSAGElib->hTimer) {
        BTMR_DestroyTimer(hSAGElib->hTimer);
        hSAGElib->hTimer = NULL;
    }
}

static BCMD_VKLID_e
BSAGElib_P_AllocSageVkl(
    BSAGElib_Handle hSAGElib)
{
    BERR_Code rc;
    BCMD_VKLID_e vklId;
    BHSM_AllocateVKLIO_t allocateVKLIO;

    BKNI_Memset(&allocateVKLIO, 0, sizeof(BHSM_AllocateVKLIO_t));
    allocateVKLIO.client                  = BHSM_ClientType_eSAGE;
    allocateVKLIO.customerSubMode         = BCMD_CustomerSubMode_eGeneric_CA_64_4;
    allocateVKLIO.bNewVKLCustSubModeAssoc = false;

    BSAGElib_iLockHsm();
    rc = BHSM_AllocateVKL(hSAGElib->core_handles.hHsm, &allocateVKLIO);
    BSAGElib_iUnlockHsm();
    if(rc != BERR_SUCCESS) {
        vklId = BCMD_VKL_eMax;
        BDBG_ERR(("%s - BHSM_AllocateVKL() fails", __FUNCTION__));
    }
    else {
        vklId = allocateVKLIO.allocVKL;
        BDBG_MSG(("%s - allocate vkl id=%u", __FUNCTION__, vklId));
    }

    return vklId;
}

static void
BSAGElib_P_FreeSageVkl(
    BSAGElib_Handle hSAGElib,
    BCMD_VKLID_e vklId)
{
    BSAGElib_iLockHsm();
    BHSM_FreeVKL(hSAGElib->core_handles.hHsm, vklId);
    BSAGElib_iUnlockHsm();
}

void
BSAGElib_P_SageVklsUninit(
    BSAGElib_Handle hSAGElib)
{
    if (hSAGElib->vkl1 != BCMD_VKL_eMax) {
        BSAGElib_P_FreeSageVkl(hSAGElib, hSAGElib->vkl1);
        hSAGElib->vkl1 = BCMD_VKL_eMax;
    }
    if (hSAGElib->vkl2 != BCMD_VKL_eMax) {
        BSAGElib_P_FreeSageVkl(hSAGElib, hSAGElib->vkl2);
        hSAGElib->vkl2 = BCMD_VKL_eMax;
    }
}

BERR_Code
BSAGElib_P_SageVklsInit(
    BSAGElib_Handle hSAGElib)
{
    BERR_Code rc = BERR_SUCCESS;

    hSAGElib->vkl1 = BSAGElib_P_AllocSageVkl(hSAGElib);
    hSAGElib->vkl2 = BSAGElib_P_AllocSageVkl(hSAGElib);
    if (hSAGElib->vkl1 == BCMD_VKL_eMax ||
        hSAGElib->vkl2 == BCMD_VKL_eMax) {
        BDBG_ERR(("%s - cannot initialize VKLs for SAGE", __FUNCTION__));
        rc = BERR_OS_ERROR;
        goto end;
    }

end:
    return rc;
}

void
BSAGElib_GetDefaultSettings(
    BSAGElib_Settings *pSettings)
{
    BDBG_ENTER(BSAGElib_GetDefaultSettings);
    BDBG_ASSERT(pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    BDBG_LEAVE(BSAGElib_GetDefaultSettings);
}

BERR_Code
BSAGElib_GetDynamicSettings(
    BSAGElib_Handle hSAGElib,
    BSAGElib_DynamicSettings *settings)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BSAGElib_GetDynamicSettings);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(settings);

    settings->hHsm = hSAGElib->core_handles.hHsm;

    BDBG_LEAVE(BSAGElib_GetDynamicSettings);
    return rc;
}
BERR_Code
BSAGElib_SetDynamicSettings(
    BSAGElib_Handle hSAGElib,
    BSAGElib_DynamicSettings *settings)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BSAGElib_SetDynamicSettings);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(settings);

    hSAGElib->core_handles.hHsm = settings->hHsm;

    BDBG_LEAVE(BSAGElib_SetDynamicSettings);
    return rc;
}

BERR_Code
BSAGElib_Open(
    BSAGElib_Handle *pSAGElibHandle,
    BSAGElib_Settings *settings /* [in] */)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_Handle instance = NULL;

    BDBG_ENTER(BSAGElib_Open);

    if (!pSAGElibHandle || !settings) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid input parameter [handle=%p, settings=%p]",
                  __FUNCTION__, pSAGElibHandle, settings));
        goto end;
    }

    if (!settings->hReg | !settings->hChp | !settings->hInt | !settings->hTmr | !settings->hHsm) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid core handle", __FUNCTION__));
        goto end;
    }

    if (!settings->i_memory_alloc.malloc |
        !settings->i_memory_alloc.malloc_restricted |
        !settings->i_memory_alloc.free) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid memory allocation interface", __FUNCTION__));
        goto end;
    }

    if (!settings->i_memory_map.offset_to_addr | !settings->i_memory_map.addr_to_offset) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid memory map interface", __FUNCTION__));
        goto end;
    }

    if (!settings->i_memory_sync.flush | !settings->i_memory_sync.invalidate) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid memory sync interface", __FUNCTION__));
        goto end;
    }

    if (!settings->i_memory_sync_isrsafe.flush | !settings->i_memory_sync_isrsafe.invalidate) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid memory sync isrsafe interface", __FUNCTION__));
        goto end;
    }

    if (!settings->i_sync_sage.lock | !settings->i_sync_sage.unlock |
        !settings->i_sync_hsm.lock | !settings->i_sync_hsm.unlock) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid sync sage or HSM interface", __FUNCTION__));
        goto end;
    }

    instance = BKNI_Malloc(sizeof(*instance));
    if (!instance) {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        BDBG_ERR(("%s: cannot allocate instance context", __FUNCTION__));
        goto end;
    }

    BKNI_Memset(instance, 0, sizeof(*instance));
    instance->vkl1 = instance->vkl2 = BCMD_VKL_eMax;

    BDBG_OBJECT_SET(instance, BSAGElib_P_Instance);

    instance->core_handles.hReg = settings->hReg;
    instance->core_handles.hChp = settings->hChp;
    instance->core_handles.hInt = settings->hInt;
    instance->core_handles.hTmr = settings->hTmr;
    instance->core_handles.hHsm = settings->hHsm;

    /* Common interfaces */
    instance->i_memory_map = settings->i_memory_map;
    instance->i_memory_sync = settings->i_memory_sync;
    instance->i_memory_sync_isrsafe = settings->i_memory_sync_isrsafe;
    instance->i_memory_alloc = settings->i_memory_alloc;
    instance->i_sync_hsm = settings->i_sync_hsm;

    /* instance specifics */
    instance->defaultClientSettings.i_sync_sage = settings->i_sync_sage;
    instance->defaultClientSettings.i_sync_hsm = settings->i_sync_hsm;

        /*acquire SAGE power resources */
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CLK
    BCHP_PWR_AcquireResource(instance->core_handles.hChp, BCHP_PWR_RESOURCE_HDMI_TX_CLK);
#endif
#ifdef BCHP_PWR_RESOURCE_BVN
    BCHP_PWR_AcquireResource(instance->core_handles.hChp, BCHP_PWR_RESOURCE_BVN);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC
    BCHP_PWR_AcquireResource(instance->core_handles.hChp, BCHP_PWR_RESOURCE_VDC_VEC);
#endif

    rc = BSAGElib_P_GetChipsetType(instance);
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGElib_P_Rpc_Init(instance);
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGElib_P_TimerInit(instance);
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGElib_P_Management_Initialize(instance);
    if (rc != BERR_SUCCESS) { goto end; }

    if (settings->enablePinmux) {
        instance->enablePinmux = 1;
        rc = BSAGElib_P_Init_Serial(instance);
        if (rc != BERR_SUCCESS) { goto end; }
    }

    rc = BSAGElib_P_SageVklsInit(instance);
    if (rc != BERR_SUCCESS) { goto end; }

    BLST_S_INIT(&instance->clients);

    BDBG_MSG(("%s add hSAGElib=%p", __FUNCTION__, instance));
    *pSAGElibHandle = instance;

end:
    if (rc != BERR_SUCCESS) {
        BSAGElib_P_Close(instance);
    }
    BDBG_LEAVE(BSAGElib_Open);
    return rc;
}

static void
BSAGElib_P_Close(
    BSAGElib_Handle hSAGElib)
{
    if (!hSAGElib) {
        goto end;
    }

    BDBG_MSG(("%s remove hSAGElib=%p", __FUNCTION__, hSAGElib));

    for (;;)
    {
        BSAGElib_ClientHandle hSAGElibClient = BLST_S_FIRST(&hSAGElib->clients);
        if (hSAGElibClient == NULL) {
            break;
        }
        BDBG_ERR(("%s: leaked hSAGElib=%p hSAGElibClient=%p. Forcing close.",
                  __FUNCTION__, hSAGElib, hSAGElibClient));
        BSAGElib_P_CloseClient(hSAGElibClient, 1);
    }

    BSAGElib_P_Management_Uninitialize(hSAGElib);
    BSAGElib_P_TimerUninit(hSAGElib);
    BSAGElib_P_Rpc_Uninit(hSAGElib);

    /*release SAGE power resources */
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CLK
    BCHP_PWR_ReleaseResource(hSAGElib->core_handles.hChp, BCHP_PWR_RESOURCE_HDMI_TX_CLK);
#endif
#ifdef BCHP_PWR_RESOURCE_BVN
    BCHP_PWR_ReleaseResource(hSAGElib->core_handles.hChp, BCHP_PWR_RESOURCE_BVN);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC
    BCHP_PWR_ReleaseResource(hSAGElib->core_handles.hChp, BCHP_PWR_RESOURCE_VDC_VEC);
#endif

    BDBG_OBJECT_DESTROY(hSAGElib, BSAGElib_P_Instance);

    BKNI_Free(hSAGElib);

end:
    return;
}


void
BSAGElib_Close(
    BSAGElib_Handle hSAGElib)
{
    BDBG_ENTER(BSAGElib_Close);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);

    BSAGElib_P_Close(hSAGElib);

    BDBG_LEAVE(BSAGElib_Close);
}

void
BSAGElib_GetDefaultClientSettings(
    BSAGElib_Handle hSAGElib,
    BSAGElib_ClientSettings *pSettings /* [in/out] */)
{
    BDBG_ENTER(BSAGElib_GetDefaultClientSettings);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(pSettings);

    *pSettings = hSAGElib->defaultClientSettings;

    BDBG_LEAVE(BSAGElib_GetDefaultClientSettings);
}

BERR_Code
BSAGElib_OpenClient(
    BSAGElib_Handle hSAGElib,
    BSAGElib_ClientHandle *pSAGElibClientHandle, /* [out] SAGElib instance handle */
    BSAGElib_ClientSettings *settings)
{
    BSAGElib_ClientHandle hSAGElibClient = NULL;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BSAGElib_OpenClient);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);

    if (!settings | !pSAGElibClientHandle) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid input parameter [handle=%p, settings=%p]",
                  __FUNCTION__, pSAGElibClientHandle, settings));
        goto end;
    }

    hSAGElibClient = BKNI_Malloc(sizeof(*hSAGElibClient));
    if (!hSAGElibClient) {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        BDBG_ERR(("%s: cannot allocate client context", __FUNCTION__));
        goto end;
    }

    BKNI_Memset(hSAGElibClient, 0, sizeof(*hSAGElibClient));

    BDBG_OBJECT_SET(hSAGElibClient, BSAGElib_P_Client);

    hSAGElibClient->settings = *settings;
    hSAGElibClient->hSAGElib = hSAGElib;

    BLST_S_INIT(&hSAGElibClient->remotes);
    BLST_SQ_INIT(&hSAGElibClient->responseCallbacks);
    BLST_SQ_INIT(&hSAGElibClient->responseCallbackCache);

    BSAGElib_iLockSage;
    BLST_S_INSERT_HEAD(&hSAGElib->clients, hSAGElibClient, link);
    BSAGElib_iUnlockSage;

    *pSAGElibClientHandle = hSAGElibClient;

    BDBG_MSG(("%s add hSAGElib=%p hSAGElibClient=%p", __FUNCTION__, hSAGElib, hSAGElibClient));

end:
    if (rc != BERR_SUCCESS) {
        BSAGElib_P_CloseClient(hSAGElibClient, 0);
    }

    BDBG_LEAVE(BSAGElib_OpenClient);
    return rc;
}

/* Needs of a collector param is due to:
 - needs access to i_sync_sage interface to lock sage down on regular flow
 - BUT when the client is garbage collected, then running code is in Within 'main' context,
   i.e. the 'owner' of SAGElib and then shall not lock/unlock */
static void
BSAGElib_P_CloseClient(
    BSAGElib_ClientHandle hSAGElibClient,
    int collector)
{
    BSAGElib_Handle hSAGElib;

    if (hSAGElibClient == NULL) {
        goto end;
    }

    hSAGElib = hSAGElibClient->hSAGElib;

    BDBG_MSG(("%s remove hSAGElib=%p hSAGElibClient=%p", __FUNCTION__, hSAGElib, hSAGElibClient));

    /* if collector is set, lock/unlock callback could be set to NULL */
    if (!collector) {
        BSAGElib_iLockSage;
    }
    BLST_S_REMOVE(&hSAGElib->clients, hSAGElibClient, BSAGElib_P_Client, link);
    if (!collector) {
        BSAGElib_iUnlockSage;
    }

    for (;;)
    {
        BSAGElib_RpcRemoteHandle remote = BLST_S_FIRST(&hSAGElibClient->remotes);
        if (remote == NULL) {
            break;
        }
        if (remote == hSAGElib->hStandbyRemote) {
            hSAGElib->hStandbyRemote = NULL;
            BDBG_MSG(("%s remove hSAGElib=%p hSAGElibClient=%p Standby remote=%p", __FUNCTION__, hSAGElib, hSAGElibClient, remote));
        }
        else {
            BDBG_ERR(("%s: leaked hSAGElib=%p hSAGElibClient=%p remote=%p. Forcing close.",
                      __FUNCTION__, hSAGElib, hSAGElibClient, remote));
        }
        BSAGElib_P_Rpc_RemoveRemote(remote);
    }

    /* dequeue any pending responses */
    BSAGElib_P_Rpc_ConsumeResponseCallbacks(hSAGElibClient, 0/* do NOT fire the callbacks */);
    /* free callbackItem cache */
    BSAGElib_P_Rpc_ResponseCallbackCacheFree(hSAGElibClient);

    if (hSAGElibClient->hContainerCache) {
        BSAGElib_Tools_ContainerCache_Close(hSAGElibClient->hContainerCache);
        hSAGElibClient->hContainerCache = NULL;
    }

    BDBG_OBJECT_DESTROY(hSAGElibClient, BSAGElib_P_Client);
    BKNI_Free(hSAGElibClient);

end:
    return;
}

void
BSAGElib_CloseClient(
    BSAGElib_ClientHandle hSAGElibClient /* [in] SAGElib instance handle */)
{
    BDBG_ENTER(BSAGElib_CloseClient);

    BDBG_OBJECT_ASSERT(hSAGElibClient, BSAGElib_P_Client);

    BSAGElib_P_CloseClient(hSAGElibClient, 0);

    BDBG_LEAVE(BSAGElib_CloseClient);
}

BERR_Code
BSAGElib_Client_DispatchResponseCallbacks(
    BSAGElib_ClientHandle hSAGElibClient)
{
    BERR_Code rc;

    BDBG_ENTER(BSAGElib_Client_DispatchResponseCallbacks);

    BDBG_OBJECT_ASSERT(hSAGElibClient, BSAGElib_P_Client);

    rc = BSAGElib_P_Rpc_ConsumeResponseCallbacks(hSAGElibClient, 1/* fire the callbacks */);

    BDBG_LEAVE(BSAGElib_Client_DispatchResponseCallbacks);
    return rc;
}

BERR_Code
BSAGElib_Client_GetResponse(
    BSAGElib_ClientHandle hSAGElibClient,
    struct BSAGElib_ResponseData *data)
{
    BDBG_OBJECT_ASSERT(hSAGElibClient, BSAGElib_P_Client);
    return BSAGElib_P_Rpc_GetResponse(hSAGElibClient, data);
}

BERR_Code
BSAGElib_GetChipInfo(
    BSAGElib_Handle hSAGElib,
    BSAGElib_ChipInfo *pChipInfo)
{
    BERR_Code rc = BERR_SUCCESS;
    BDBG_ENTER(BSAGElib_GetChipInfo);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(pChipInfo);
    *pChipInfo = hSAGElib->chipInfo;

    BDBG_LEAVE(BSAGElib_GetChipInfo);
    return rc;
}

BERR_Code
BSAGElib_P_GetOtp(
    BSAGElib_Handle hSAGElib,
    BCMD_Otp_CmdMsp_e msp_enum,
    uint32_t *out,
    const char *dbg_name)
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_ReadMspIO_t ReadMspParm;

    BKNI_Memset(&ReadMspParm, 0, sizeof(BHSM_ReadMspIO_t));
    ReadMspParm.readMspEnum = msp_enum;
    BSAGElib_iLockHsm();
    rc = BHSM_ReadMSP(hSAGElib->core_handles.hHsm, &ReadMspParm);
    BSAGElib_iUnlockHsm();
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error calling BHSM_ReadMSP() for '%s'", dbg_name));
        goto end;
    }
    if (ReadMspParm.unStatus)
    {
        BDBG_ERR(("Error reading OTP MSP for '%s'", dbg_name));
        rc = BERR_NOT_AVAILABLE;
        goto end;
    }
    *out = _EndianSwap(ReadMspParm.aucMspData);

end:
    return rc;
}

#define OTP_SWIZZLE0A_MSP0_VALUE_ZS (0x02)
#define OTP_SWIZZLE0A_MSP1_VALUE_ZS (0x02)
#define OTP_SWIZZLE0A_MSP0_VALUE_ZB (0x3E)
#define OTP_SWIZZLE0A_MSP1_VALUE_ZB (0x3F)
#define OTP_SWIZZLE0A_MSP0_VALUE_CUST1 (0x03)
#define OTP_SWIZZLE0A_MSP1_VALUE_CUST1 (OTP_SWIZZLE0A_MSP0_VALUE_CUST1)
/* Retrieve chipset type */
static BERR_Code
BSAGElib_P_GetChipsetType(
    BSAGElib_Handle hSAGElib)
{
    BERR_Code rc = BERR_SUCCESS;
    uint32_t otp_swizzle0a_msp0;
    uint32_t otp_swizzle0a_msp1;

    rc = BSAGElib_P_GetOtp(hSAGElib, BCMD_Otp_CmdMsp_eReserved233, &otp_swizzle0a_msp0, "msp0");
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGElib_P_GetOtp(hSAGElib, BCMD_Otp_CmdMsp_eReserved234, &otp_swizzle0a_msp1, "msp1");
    if (rc != BERR_SUCCESS) { goto end; }

    BDBG_MSG(("%s - OTP [MSP0: %d, MSP1: %d]",
              __FUNCTION__, otp_swizzle0a_msp0, otp_swizzle0a_msp1));

    if ((otp_swizzle0a_msp0 == OTP_SWIZZLE0A_MSP0_VALUE_ZS) &&
       (otp_swizzle0a_msp1 == OTP_SWIZZLE0A_MSP1_VALUE_ZS)) {
        BDBG_LOG(("%s - Chip Type: ZS", __FUNCTION__));
        hSAGElib->chipInfo.chipType = BSAGElib_ChipType_eZS;
    }
    else if ((otp_swizzle0a_msp0 == OTP_SWIZZLE0A_MSP0_VALUE_ZB) &&
             (otp_swizzle0a_msp1 == OTP_SWIZZLE0A_MSP1_VALUE_ZB)) {
        BDBG_LOG(("%s - Chip Type: ZB", __FUNCTION__));
        hSAGElib->chipInfo.chipType = BSAGElib_ChipType_eZB;
    }
    else if ((otp_swizzle0a_msp0 == OTP_SWIZZLE0A_MSP0_VALUE_CUST1) &&
             (otp_swizzle0a_msp1 == OTP_SWIZZLE0A_MSP1_VALUE_CUST1)) {
        BDBG_LOG(("%s - Chip Type: Customer1", __FUNCTION__));
        hSAGElib->chipInfo.chipType = BSAGElib_ChipType_eCustomer1;
    }
    else
    {
        BDBG_LOG(("%s - Chip Type: Customer specific chip", __FUNCTION__));
        hSAGElib->chipInfo.chipType = BSAGElib_ChipType_eCustomer;
    }

end:
    return rc;
}

BERR_Code
BSAGElib_GetStatus(
    BSAGElib_Handle hSAGElib,
    BSAGElib_Status *pStatus)
{
    uint32_t addr, value;
    BERR_Code rc = BERR_SUCCESS;
    BDBG_ENTER(BSAGElib_GetStatus);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(pStatus);

    addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eSageStatusFlags);
    value = BREG_Read32(hSAGElib->core_handles.hReg, addr);
    pStatus->urr.secured = (value & BSAGElibSageStatusFlags_eURRSecureStatus_Mask) ? true : false;

    BDBG_LEAVE(BSAGElib_GetStatus);
    return rc;
}

BERR_Code
BSAGElib_GetLogWriteCount(
    BSAGElib_Handle hSAGElib,
    uint64_t *size)
{
    uint32_t addr, value;
    BERR_Code rc = BERR_SUCCESS;
    BDBG_ENTER(BSAGElib_GetLogWriteCount);

    BDBG_OBJECT_ASSERT(hSAGElib, BSAGElib_P_Instance);
    BDBG_ASSERT(size);

    addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eSageLogWriteCountMSB);
    value = BREG_Read32(hSAGElib->core_handles.hReg, addr);
    *size = value;
    *size = (*size<<32);

    addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eSageLogWriteCountLSB);
    value = BREG_Read32(hSAGElib->core_handles.hReg, addr);
    *size |= (value);


    BDBG_LEAVE(BSAGElib_GetLogWriteCount);
    return rc;
}
