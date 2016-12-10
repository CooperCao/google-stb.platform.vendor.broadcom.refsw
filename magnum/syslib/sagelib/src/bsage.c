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
#include "priv/bsagelib_shared_types.h"

#include "bhsm_otpmsp.h"
#include "bhsm_keyladder_enc.h"

#include "bsage.h"
#include "bsage_priv.h"
#include "bsage_management.h"

BDBG_MODULE(BSAGE);

BSAGE_Handle hSAGE_Global = NULL;

/* Local functions */
static BERR_Code BSAGE_P_TimerInit(BSAGE_Handle hSAGE);
static void BSAGE_P_TimerUninit(BSAGE_Handle hSAGE);


static BERR_Code
BSAGE_P_TimerInit(
    BSAGE_Handle hSAGE)
{
    BERR_Code rc;
    BTMR_TimerSettings timerSettings = { BTMR_Type_eSharedFreeRun, NULL, NULL, 0, false };
    rc = BTMR_CreateTimer(hSAGE->hTmr, &hSAGE->hTimer, &timerSettings);
    if(rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - BTMR_CreateTimer failure %d", __FUNCTION__, rc));
    }
    return rc;
}

static void
BSAGE_P_TimerUninit(
    BSAGE_Handle hSAGE)
{
    if (hSAGE->hTimer) {
        BTMR_DestroyTimer(hSAGE->hTimer);
        hSAGE->hTimer = NULL;
    }
}

BERR_Code
BSAGE_P_GetOtp(
    BSAGE_Handle hSAGE,
    BHSM_Handle hHsm,
    BCMD_Otp_CmdMsp_e msp_enum,
    uint32_t *out,
    const char *dbg_name)
{
    BERR_Code rc = BERR_SUCCESS;
    BHSM_ReadMspIO_t ReadMspParm;

    BKNI_Memset(&ReadMspParm, 0, sizeof(BHSM_ReadMspIO_t));
    ReadMspParm.readMspEnum = msp_enum;
    BSAGE_iLockHsm();
    rc = BHSM_ReadMSP(hHsm, &ReadMspParm);
    BSAGE_iUnlockHsm();
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

void
BSAGE_Close(
    BSAGE_Handle hSAGE)
{
    if (!hSAGE) {
        goto end;
    }

    if(hSAGE_Global != hSAGE)
    {
        BDBG_ERR(("%s: invalid SAGE handle %p, hSAGE_Global %p",__FUNCTION__,hSAGE,hSAGE_Global));
        hSAGE = hSAGE_Global;
    }

    BDBG_MSG(("%s remove hSAGE=%p", __FUNCTION__, (void *)hSAGE));

    for (;;)
    {
        BSAGE_RpcRemoteHandle remote = BLST_S_FIRST(&hSAGE->remotes);
        if (remote == NULL) {
            break;
        }
        BSAGE_Rpc_RemoveRemote(remote);
    }

    BSAGE_P_Management_Uninitialize(hSAGE);

    BSAGE_P_Rpc_Uninit(hSAGE);

    BSAGE_P_TimerUninit(hSAGE);

    BKNI_Free(hSAGE);

    hSAGE_Global = NULL;
end:
    return;
}

BERR_Code
BSAGE_Open(
    BSAGE_Handle *pSAGEHandle,
    BSAGE_Settings *settings)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGE_Handle instance = NULL;

    BDBG_ENTER(BSAGE_Open);

    if (!pSAGEHandle ) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid input parameter NULL handle pointer",__FUNCTION__));
        goto end;
    }

    if(hSAGE_Global != NULL)
    {
        BDBG_ERR(("%s: BSAGE already openned, return handle",__FUNCTION__));
        instance = hSAGE_Global;
        *pSAGEHandle = hSAGE_Global;
    }

    if (!settings) {
        BDBG_ERR(("%s: NULL settings, return NULL SAGElib handle",__FUNCTION__));
        *pSAGEHandle = hSAGE_Global;
        goto end;
    }

    if (!settings->hReg | !settings->hInt | !settings->hTmr) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid core handle", __FUNCTION__));
        goto end;
    }

    if (!settings->i_memory_map.offset_to_addr | !settings->i_memory_map.addr_to_offset) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid memory map interface", __FUNCTION__));
        goto end;
    }

    if (!settings->i_memory_sync.flush | !settings->i_memory_sync.invalidate) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid memory sync isrsafe interface", __FUNCTION__));
        goto end;
    }

    if (!settings->i_sync_hsm.lock | !settings->i_sync_hsm.unlock) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid sync HSM interface", __FUNCTION__));
        goto end;
    }

    if(instance == NULL){
    instance = BKNI_Malloc(sizeof(*instance));
    if (!instance) {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        BDBG_ERR(("%s: cannot allocate instance context", __FUNCTION__));
        goto end;
    }
    BKNI_Memset(instance, 0, sizeof(*instance));
    }
    instance->hReg = settings->hReg;
    instance->hInt = settings->hInt;
    instance->hTmr = settings->hTmr;

    instance->i_memory_map = settings->i_memory_map;
    instance->i_memory_sync = settings->i_memory_sync;
    instance->i_sync_sage = settings->i_sync_sage;
    instance->i_sync_hsm = settings->i_sync_hsm;

    BLST_S_INIT(&instance->remotes);

    if (settings->enablePinmux) {
        instance->enablePinmux = 1;
        rc = BSAGE_P_Init_Serial(instance);
        if (rc != BERR_SUCCESS) { goto end; }
    }

    rc = BSAGE_P_TimerInit(instance);
    if (rc != BERR_SUCCESS) { goto end; }

    rc = BSAGE_P_Management_Initialize(instance);
    if (rc != BERR_SUCCESS) { goto end; }

    BDBG_MSG(("%s add hSAGE=%p", __FUNCTION__, (void *)instance));
    *pSAGEHandle = instance;
    hSAGE_Global = instance;

end:
    if (rc != BERR_SUCCESS) {
        BSAGE_Close(instance);
    }
    BDBG_LEAVE(BSAGE_Open);
    return rc;
}

BERR_Code
BSAGE_RegisterCallback(
    SAGE_Event event,
    BSAGE_Callback callback,
    void *context,
    BSAGE_RpcRemoteHandle remote)
{
    BSAGE_ManagementInterface watchdog;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BSAGE_RegisterEvent);

    if (!callback) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_MSG(("%s: event %d,NULL callback function", __FUNCTION__,event));
        goto end;
    }

    if (remote == NULL && event != BSAGE_Event_watchdog) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s:  event %d,NULL remote handle", __FUNCTION__,event));
        goto end;
    }

    switch(event)
    {
    case BSAGE_Event_msgIndication:
        remote->rpc.indicationRecv_isr = (BSAGElib_Rpc_IndicationRecvCallback)callback;
        remote->rpc.indicationContext = context;
        break;
    case BSAGE_Event_msgResponse:
        remote->rpc.response_isr = (BSAGE_Rpc_DispatchResponse_isr) callback;
        remote->rpc.responseContext = context;
        break;
    case BSAGE_Event_msgCallbackRequest:
        remote->rpc.callbackRequest_isr = (BSAGElib_Rpc_CallbackRequestISRCallback) callback;
        remote->rpc.callbackRequestContext = context;
        break;
    case BSAGE_Event_msgTATermination:
        remote->rpc.taTerminate_isr = (BSAGElib_Rpc_TATerminateCallback) callback;
        remote->rpc.taTerminationContext = context;
        break;
    case BSAGE_Event_watchdog:
        watchdog.watchdog_isr = (BSAGE_Management_WatchdogCallback)callback;
        watchdog.context = NULL;
        rc = BSAGE_Management_Register(&watchdog);
        break;
    default:
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid event %d", __FUNCTION__,event));
        goto end;
    }
end:
    BDBG_LEAVE(BSAGE_RegisterEvent);
    return rc;
}

BERR_Code
BSAGE_UnRegisterCallback(
    SAGE_Event event,
    BSAGE_RpcRemoteHandle remote)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(BSAGE_UnRegisterEvent);

    if (remote == NULL && event != BSAGE_Event_watchdog) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s:  event %d,NULL remote handle", __FUNCTION__,event));
        goto end;
    }

    switch(event)
    {
    case BSAGE_Event_msgIndication:
        remote->rpc.indicationRecv_isr = NULL;
        break;
    case BSAGE_Event_msgResponse:
        remote->rpc.response_isr = NULL;
        break;
    case BSAGE_Event_msgCallbackRequest:
        remote->rpc.callbackRequest_isr = NULL;
        break;
    case BSAGE_Event_msgTATermination:
        remote->rpc.taTerminate_isr = NULL;
        break;
    case BSAGE_Event_watchdog:
        BSAGE_Management_Unregister((BSAGE_ManagementInterface *)remote);
        break;
    default:
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid event %d", __FUNCTION__,event));
        goto end;
    }
end:
    BDBG_LEAVE(BSAGE_UnRegisterEvent);
    return rc;
}

BERR_Code
BSAGE_GetStatus(
    BSAGElib_Status *pStatus)
{
    uint32_t addr, value;
    BERR_Code rc = BERR_SUCCESS;
    BSAGE_Handle hSAGE = hSAGE_Global;

    BDBG_ENTER(BSAGE_GetStatus);

    if (!hSAGE) {
        rc = BERR_NOT_INITIALIZED;
        BDBG_ERR(("%s: SAGE is not open yet, hSAGE is NULL", __FUNCTION__));
        goto err;
    }

    BDBG_ASSERT(pStatus);

    addr = BSAGElib_GlobalSram_GetRegister(BSAGElib_GlobalSram_eSageStatusFlags);
    value = BREG_Read32(hSAGE->hReg, addr);
    pStatus->urr.secured = (value & BSAGElibSageStatusFlags_eURRSecureStatus_Mask) ? true : false;
err:
    BDBG_LEAVE(BSAGE_GetStatus);
    return rc;
}
