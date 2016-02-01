/***************************************************************************
 *     (c)2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/

#include "nexus_scm_module.h"
#include "priv/nexus_core.h" /* get access to g_pCoreHandles */

#include "nexus_memory.h"

/* magnum basemodules */
#include "berr.h"
#include "bkni.h"

#include "nexus_scm.h"

BDBG_MODULE(nexus_scm);

/* Internal global context */
static struct NEXUS_Scm_P_State {
    int init;               /* init flag. 0 if module is not yet initialized. */
} g_scm;

/*
 * Local functions
 */
static NEXUS_Error NEXUS_Scm_P_LazyInit(void);
static void NEXUS_Scm_P_Cleanup(void);
static void NEXUS_Scm_P_Finalizer(NEXUS_ScmHandle scm);
static void NEXUS_ScmChannel_DumpScmStatus(void);
#if 0
static void  NEXUS_ScmChannel_DumpCommand(void);
#endif
/*
 * Private API between module funcs and Scm internals
 * Initialize/cleanup nexus_scm.c private vars
 */
NEXUS_Error NEXUS_Scm_P_VarInit(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BKNI_Memset(&g_scm, 0, sizeof(g_scm));

    BSTD_UNUSED(rc);
/*end:*/
    return rc;
}

void NEXUS_Scm_P_VarCleanup(void)
{
    NEXUS_Scm_P_Cleanup();
}

/*
 * Public API
 */

static void NEXUS_Scm_P_Cleanup(void)
{
    if (g_scm.init) {
        g_scm.init = 0;
    }
}

/* Initialize NEXUS Scm in a lazy maner i.e. postponned to when needed */
static NEXUS_Error NEXUS_Scm_P_LazyInit(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (g_scm.init) {
        goto end;
    }

    g_scm.init = 1;/* mark it now for cleanup error handling */

end:
    if (rc != NEXUS_SUCCESS) {
        NEXUS_Scm_P_Cleanup();
    }
    return rc;
}


NEXUS_OBJECT_CLASS_MAKE(NEXUS_Scm, NEXUS_Scm_Close);

/* Get default channel settings */
void NEXUS_Scm_GetDefaultOpenSettings(
    NEXUS_ScmOpenSettings *pSettings /* [out] */)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_CallbackDesc_Init(&pSettings->watchdogCallback);
}

/* Create a scm instance */
NEXUS_ScmHandle NEXUS_Scm_Open(
    const NEXUS_ScmOpenSettings *pSettings)
{
    NEXUS_ScmOpenSettings settings;
    NEXUS_ScmHandle scm = NULL;

    BDBG_ENTER(NEXUS_Scm_Open);

    if (pSettings == NULL) {
        NEXUS_Scm_GetDefaultOpenSettings(&settings);
        pSettings = &settings;
    }

    /* initialize module resources on first Open */
    if (NEXUS_Scm_P_LazyInit() != NEXUS_SUCCESS) {
        BDBG_ERR(("LazyInit failure"));
        goto end;
    }

    /* Init Scm instance */
    scm = BKNI_Malloc(sizeof(*scm));
    if (scm == NULL) {
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }

    /* success */

    NEXUS_OBJECT_INIT(NEXUS_Scm, scm);

    BKNI_EnterCriticalSection();
    g_NEXUS_scmModule.instance = scm;
    BKNI_LeaveCriticalSection();

    BDBG_MSG(("NEW scm=%p", (void *)scm));

end:
    BDBG_LEAVE(NEXUS_Scm_Open);
    return scm;
}

/* NEXUS_Scm_Close's destructor.
 * Called by NEXUS_Scm_Close upper API and
 *        by NEXUS garbage collecting */
static void NEXUS_Scm_P_Finalizer(NEXUS_ScmHandle scm)
{
    BDBG_ENTER(NEXUS_Scm_P_Finalizer);

    NEXUS_OBJECT_ASSERT(NEXUS_Scm, scm);

    BDBG_MSG(("DEL scm=%p", (void *)scm));

    /* NEXUS_OBJECT_ACQUIRE guarantees that no channels remain */
    BDBG_ASSERT((scm->channel.valid == false));

    BKNI_EnterCriticalSection();
    g_NEXUS_scmModule.instance = NULL;
    BKNI_LeaveCriticalSection();

    /* scm context is zeroed in NEXUS_OBJECT_DESTROY */
    NEXUS_OBJECT_DESTROY(NEXUS_Scm, scm);

    BKNI_Free(scm);

    BDBG_LEAVE(NEXUS_Scm_P_Finalizer);
}

/* Get default channel settings */
void NEXUS_ScmChannel_GetDefaultSettings(
    NEXUS_ScmChannelSettings *pSettings /* [out] */)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_CallbackDesc_Init(&pSettings->successCallback);
    NEXUS_CallbackDesc_Init(&pSettings->errorCallback);
}

/* Create a channel on a scm instance */
NEXUS_ScmChannelHandle NEXUS_Scm_CreateChannel(
    NEXUS_ScmHandle scm,
    const NEXUS_ScmChannelSettings *pSettings)
{
    NEXUS_ScmChannelHandle ret = NULL;
    NEXUS_ScmChannelHandle channel = NULL;

    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ENTER(NEXUS_Scm_CreateChannel);

    BDBG_ASSERT( pSettings );
    NEXUS_OBJECT_ASSERT(NEXUS_Scm, scm);

    if(true == scm->channel.valid){
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }
    channel = &scm->channel;

    BKNI_Memset(channel, 0, sizeof(NEXUS_ScmChannel));

    channel->scm = scm;
    channel->type = pSettings->type;

    rc = NEXUS_Scm_P_DtaModeSelect(channel->type);
    if(NEXUS_SUCCESS != rc){
        BDBG_ERR(("%s %d - %08x", __FUNCTION__, __LINE__,rc));
        goto end;
    }
    /* Load and start SCM  */
    rc = NEXUS_ScmModule_P_Start();
    if(rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - NEXUS_ScmModule_P_Start() fails %d", __FUNCTION__, rc));
        goto end;
    }

    ret = channel;
end:
    if (ret == NULL) {
        /* error cleanup */
        if (channel != NULL) {
            NEXUS_Scm_DestroyChannel(channel);
        }
    }
    BDBG_LEAVE(NEXUS_Scm_CreateChannel);
    return ret;
}

/* Destroy a channel created by NEXUS_Scm_CreateChannel */


void NEXUS_Scm_DestroyChannel(NEXUS_ScmChannelHandle channel)
{
    BDBG_ENTER(NEXUS_Scm_DestroyChannel);
    channel->valid = false;
    BDBG_LEAVE(NEXUS_Scm_DestroyChannel);
}

static NEXUS_Error NEXUS_ScmChannel_SageNotify(void);

/* Send a command to SCM-side through a channel */
NEXUS_Error NEXUS_ScmChannel_SendCommand(
    NEXUS_ScmChannelHandle channel,
    const NEXUS_ScmCommand *pCommand)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t * out;

    BDBG_ENTER(NEXUS_ScmChannel_SendCommand);

    BDBG_ASSERT(NULL != channel);

    if(channel->scm != g_NEXUS_scmModule.instance){
        rc = NEXUS_INVALID_PARAMETER;
        BDBG_ERR(("%s:%d %08x", __FILE__, __LINE__, rc));
        goto ExitFunc;
    }
    if(NULL == pCommand){
        rc = NEXUS_INVALID_PARAMETER;
        BDBG_ERR(("%s:%d %08x", __FILE__, __LINE__, rc));
        goto ExitFunc;
    }
    if((NULL == pCommand->cmd) || (NULL == pCommand->rsp)){
        rc = NEXUS_INVALID_PARAMETER;
        BDBG_ERR(("%s:%d %08x", __FILE__, __LINE__, rc));
        goto ExitFunc;
    }
    if((0 == pCommand->cmd_size) || (0 == pCommand->rsp_size)){
        rc = NEXUS_INVALID_PARAMETER;
        BDBG_ERR(("%s:%d %08x", __FILE__, __LINE__, rc));
        goto ExitFunc;
    }
    if(((SCM_CMD_SIZE - 4) < pCommand->cmd_size) || ((SCM_RSP_SIZE - 4) < pCommand->rsp_size)){
        rc = NEXUS_INVALID_PARAMETER;
        BDBG_ERR(("%s:%d %08x", __FILE__, __LINE__, rc));
        goto ExitFunc;
    }

    /* copy command in to the buffer */
    out = g_NEXUS_scmModule.sendBuf;
    out[0] = pCommand->scmCommandId;
    BKNI_Memcpy(&out[1], pCommand->cmd, pCommand->cmd_size);
    /* add 4 to account for command id */
    NEXUS_FlushCache(out, (pCommand->cmd_size + 4));
    /* flush output buffer before transaction */
    NEXUS_FlushCache(g_NEXUS_scmModule.recvBuf, pCommand->rsp_size);

    /* trigger SAGE and wait for response here */
    rc = NEXUS_ScmChannel_SageNotify();
    if(NEXUS_SUCCESS != rc){
        BDBG_ERR(("%s:%d %08x", __FILE__, __LINE__, rc));
        goto ExitFunc;
    }
    BKNI_Memcpy(pCommand->rsp, g_NEXUS_scmModule.recvBuf, pCommand->rsp_size);
ExitFunc:
    BDBG_LEAVE(NEXUS_ScmChannel_SendCommand);
    return rc;
}

#include "breg_mem.h"
#include "bchp_scpu_top_ctrl.h"
#include "bchp_cpu_ipi_intr2.h"
#include "bchp_scpu_host_intr2.h"
#include "bchp_scpu_globalram.h"

#define SCM_TIMEOUT 80
/* Trigger sage interaction and wait until response */
NEXUS_Error NEXUS_ScmChannel_SageNotify(void)
{
    uint32_t val;
    uint32_t timeout;
    NEXUS_Error rc = NEXUS_SUCCESS;

    val = BREG_Read32(g_pCoreHandles->reg, BCHP_SCPU_TOP_CTRL_SCPU_HOST_IRDY);
    /* check if sage is ready */
    if(0 == (val & BCHP_SCPU_TOP_CTRL_SCPU_HOST_IRDY_SCPU_HOST_IRDY_MASK)){
        BDBG_ERR(("IRDY1 %d", val));
        NEXUS_ScmChannel_DumpScmStatus();
        rc = NEXUS_NOT_AVAILABLE;
        goto ExitFunc;
    }

    /* notify sage */
    BREG_Write32(g_pCoreHandles->reg, BCHP_CPU_IPI_INTR2_CPU_SET, BCHP_CPU_IPI_INTR2_CPU_SET_HOST_SCPU_ILOAD_MASK);
    timeout = 0;
    do{
        val = BREG_Read32(g_pCoreHandles->reg, BCHP_SCPU_HOST_INTR2_CPU_STATUS);
        val &= BCHP_SCPU_HOST_INTR2_CPU_STATUS_SCPU_HOST_OLOAD_MASK;
        if(0 == val){
            BKNI_Sleep(5);
            timeout++;
        }
    }while((val == 0) && (timeout < SCM_TIMEOUT));
    if(timeout >= SCM_TIMEOUT){
        rc = NEXUS_TIMEOUT;
    }
    /* clear interrupt bit */
    BREG_Write32(g_pCoreHandles->reg, BCHP_SCPU_HOST_INTR2_CPU_CLEAR, BCHP_SCPU_HOST_INTR2_CPU_CLEAR_SCPU_HOST_OLOAD_MASK);

    val = BREG_Read32(g_pCoreHandles->reg, BCHP_SCPU_TOP_CTRL_SCPU_HOST_IRDY);
    if(0 == val){
        BDBG_ERR(("IRDY %d", val));
    }

ExitFunc:
    return rc;
}

void NEXUS_ScmChannel_DumpScmStatus(void)
{
    uint32_t addr, val, i;

    for(i = 0x120; i < 0x130; i+=4){
        addr = BCHP_SCPU_GLOBALRAM_DMEMi_ARRAY_BASE + i;
        val = BREG_Read32(g_pCoreHandles->reg, addr);
        BDBG_ERR(("SCM[%08x]: %08x", addr, val));
    }
}

#if 0
void  NEXUS_ScmChannel_DumpCommand(void)
{
    uint32_t *mem;
    uint32_t i;
    NEXUS_FlushCache(g_NEXUS_scmModule.sendBuf, 0x1000);
    mem = (uint32_t*)((uint32_t)g_NEXUS_scmModule.sendBuf);
    for(i = 0; i < 15; i++){
        BDBG_ERR(("BSP %08x:%08x", &mem[i], mem[i]));
    }
}
#endif

NEXUS_Error NEXUS_ScmGetChannelStatus(
    NEXUS_ScmChannelHandle channel,
    NEXUS_ScmChannelStatus *pStatus
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ENTER(NEXUS_ScmGetChannelStatus);

    BDBG_ASSERT(NULL != channel);

    if(channel->scm != g_NEXUS_scmModule.instance){
        rc = NEXUS_INVALID_PARAMETER;
        BDBG_ERR(("%s:%d %08x", __FILE__, __LINE__, rc));
        goto ExitFunc;
    }
    if(NULL == pStatus){
        rc = NEXUS_INVALID_PARAMETER;
        BDBG_ERR(("%s:%d %08x", __FILE__, __LINE__, rc));
        goto ExitFunc;
    }
    if(channel->type == NEXUS_ScmType_Arris){
        pStatus->otpVersion[0] = channel->otp_version[0];
        pStatus->otpVersion[1] = channel->otp_version[1];
        pStatus->otpVersion[2] = channel->otp_version[2];
        pStatus->otpVersion[3] = channel->otp_version[3];
    }else if(channel->type == NEXUS_ScmType_Cisco){
        pStatus->otpVersion[0] = channel->otp_version[0];
        pStatus->otpVersion[1] = 0;
        pStatus->otpVersion[2] = 0;
        pStatus->otpVersion[3] = 0;
    }else{
        pStatus->otpVersion[0] = 0;
        pStatus->otpVersion[1] = 0;
        pStatus->otpVersion[2] = 0;
        pStatus->otpVersion[3] = 0;
    }
    pStatus->imageVersion = channel->version;
    pStatus->type = channel->type;
ExitFunc:
    BDBG_LEAVE(NEXUS_ScmGetChannelStatus);
    return rc;
}
