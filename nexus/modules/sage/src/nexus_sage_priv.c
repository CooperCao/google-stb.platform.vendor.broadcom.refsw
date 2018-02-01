/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "nexus_sage_module.h"
#include "priv/nexus_sage_priv.h"

#include "bsagelib.h"
#include "bsagelib_management.h"

BDBG_MODULE(nexus_sage);

void
NEXUS_Sage_GetSageLib_priv(
    BSAGElib_Handle *pSAGElibHandle)
{
    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(pSAGElibHandle);

    *pSAGElibHandle = g_NEXUS_sageModule.hSAGElib;
}

NEXUS_Error
NEXUS_Sage_WaitSage_priv(void)
{
    NEXUS_ASSERT_MODULE();

    if (NEXUS_Sage_P_CheckSageBooted()) {
        return NEXUS_SUCCESS;
    }
    return NEXUS_NOT_AVAILABLE;
}

NEXUS_Error
NEXUS_Sage_LoadImage_priv(NEXUS_SageImageHolder *holder)
{
    NEXUS_Error rc;
    void *img_context = NULL;
    BIMG_Interface img_interface;

    NEXUS_ASSERT_MODULE();

    if(!holder) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto EXIT;
    }

    rc = Nexus_SageModule_P_Img_Create(NEXUS_CORE_IMG_ID_SAGE, &img_context, &img_interface);
    if (rc != NEXUS_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    rc = NEXUS_SageModule_P_Load(holder, &img_interface, img_context);
    if (rc == NEXUS_NOT_AVAILABLE) {
        /* May not be an error, leave it to the caller to decide */
        BDBG_MSG(("%s - File not found (%s)", BSTD_FUNCTION, holder->name));
    } else if(rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - Cannot Load IMG %s ", BSTD_FUNCTION, holder->name));
    }

EXIT:
    if (img_context) {
        Nexus_SageModule_P_Img_Destroy(img_context);
    }

    return rc;
}

void * NEXUS_Sage_Malloc_priv(size_t size)
{
    NEXUS_ASSERT_MODULE();
    return NEXUS_Sage_P_Malloc(size);
}

void * NEXUS_Sage_MallocRestricted_priv(size_t size)
{
    NEXUS_ASSERT_MODULE();
    return NEXUS_Sage_P_MallocRestricted(size);
}

#if NEXUS_POWER_MANAGEMENT

/* The following is only used if Power Management is enabled */

static BSAGElib_eStandbyMode NEXUS_Sage_P_StandbyConvertMode(NEXUS_StandbyMode nexusMode);
static void NEXUS_Sage_P_ResponseStandbyCallback_isr(BSAGElib_RpcRemoteHandle handle, void *async_argument, uint32_t async_id, BERR_Code error);
static void NEXUS_Sage_P_StandbyResponseEventHandler(void *dummy);
static NEXUS_Error NEXUS_Sage_P_StandbyInit(void);
static void NEXUS_Sage_P_StandbyUninit(void);
static void NEXUS_Sage_P_StandbyClean(void);

static struct NEXUS_Sage_Standby_P_State {
    int count;
    BKNI_EventHandle event;
    NEXUS_EventCallbackHandle eventCallback;
    NEXUS_StandbyMode currMode;
    BSAGElib_ClientHandle hSAGElibClient;
} g_sage_standby = {0, NULL, NULL, NEXUS_StandbyMode_eOn, NULL};


/* Shared pattern between Host - Sage inside GlobalRam */
#define SAGE_SUSPENDVAL_RUN      0x4F4E4D4C
#define SAGE_SUSPENDVAL_SLEEP    0x4F567856
#define SAGE_SUSPENDVAL_RESUME   0x4E345678
#define SAGE_SUSPENDVAL_RESUMING 0x77347856
#define SAGE_SUSPENDVAL_S3READY  0x534E8C53
#define SAGE_SUSPENDVAL_S2READY  0x524E8C52


static BSAGElib_eStandbyMode
NEXUS_Sage_P_StandbyConvertMode(
    NEXUS_StandbyMode nexusMode)
{
    BSAGElib_eStandbyMode mode;

    switch (nexusMode) {
    case NEXUS_StandbyMode_ePassive: /* S2 : go to passive sleep */
        mode = BSAGElib_eStandbyModePassive;
        break;

    case NEXUS_StandbyMode_eDeepSleep: /* S3 : go to deep sleep */
        mode = BSAGElib_eStandbyModeDeepSleep;
        break;

    case NEXUS_StandbyMode_eOn: /* S0 : resuming */
        mode = BSAGElib_eStandbyModeOn;
        break;
    default:
    case NEXUS_StandbyMode_eMax:
    case NEXUS_StandbyMode_eActive:
        BDBG_ERR(("%s: cannot go to standby mode %u", BSTD_FUNCTION, nexusMode));
        mode = BSAGElib_eStandbyMax;
        break;
    }

    return mode;
}

static void
NEXUS_Sage_P_ResponseStandbyCallback_isr(
    BSAGElib_RpcRemoteHandle handle,
    void *async_argument,
    uint32_t async_id,
    BERR_Code error)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(async_argument);
    BSTD_UNUSED(async_id);
    BSTD_UNUSED(error);
    /* set registered event in order to trigger NEXUS_Sage_P_StandbyResponseEventHandler() */
    if (g_sage_standby.event) {
        BKNI_SetEvent_isr(g_sage_standby.event);
    }
    return;
}

static void
NEXUS_Sage_P_StandbyResponseEventHandler(
    void *dummy)
{
    BSTD_UNUSED(dummy);
    NEXUS_Sage_P_StandbyUninit();
}

static NEXUS_Error
NEXUS_Sage_P_StandbyInit(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    g_sage_standby.count++;
    if (g_sage_standby.event) {
        /* resources are already initialized */
        goto end;
    }

    {
        BERR_Code magnumRc = BKNI_CreateEvent(&g_sage_standby.event);
        if (magnumRc != BERR_SUCCESS) {
            BDBG_ERR(( "BKNI_CreateEvent(standby)" ));
            rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
            goto end;
        }
    }

    g_sage_standby.eventCallback = NEXUS_RegisterEvent(g_sage_standby.event,
                                                        NEXUS_Sage_P_StandbyResponseEventHandler, NULL);
    if (g_sage_standby.eventCallback == NULL) {
        BDBG_ERR(( "NEXUS_RegisterEvent(standby) failed!" ));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto end;
    }

    /* Open a client instance */
    {
        BSAGElib_ClientSettings SAGElibClientSettings;
        BERR_Code SAGElib_rc;

        NEXUS_Sage_P_GetSAGElib_ClientSettings(&SAGElibClientSettings);
        SAGElibClientSettings.i_rpc.response_isr = NEXUS_Sage_P_ResponseStandbyCallback_isr;
        SAGElib_rc = BSAGElib_OpenClient(g_NEXUS_sageModule.hSAGElib,
                                         &g_sage_standby.hSAGElibClient,
                                         &SAGElibClientSettings);
        if (SAGElib_rc != BERR_SUCCESS) {
            rc = BERR_TRACE(NEXUS_OS_ERROR);
            goto end;
        }
    }

end:
    if (rc != NEXUS_SUCCESS) {
        NEXUS_Sage_P_StandbyUninit();
    }
    return rc;
}

static void
NEXUS_Sage_P_StandbyUninit(void)
{
    /* check if no more client is waiting */

    if (g_sage_standby.count > 0) {
        g_sage_standby.count--;
    }
    if (g_sage_standby.count == 0) {
        NEXUS_Sage_P_StandbyClean();
    }
}

static void
NEXUS_Sage_P_StandbyClean(void)
{
    BDBG_MSG(("NEXUS_Sage_P_StandbyClean: cleanup BSAGElib resources"));

    if (g_sage_standby.eventCallback) {
        NEXUS_UnregisterEvent(g_sage_standby.eventCallback);
        g_sage_standby.eventCallback = NULL;
    }
    if (g_sage_standby.event) {
        BKNI_DestroyEvent(g_sage_standby.event);
        g_sage_standby.event = NULL;
    }
    if (g_sage_standby.hSAGElibClient) {
        BSAGElib_CloseClient(g_sage_standby.hSAGElibClient);
        g_sage_standby.hSAGElibClient = NULL;
    }
}

NEXUS_Error
NEXUS_SageModule_Standby_priv(
    bool enabled,
    const NEXUS_StandbySettings *pSettings)
{
    NEXUS_Error nexus_rc = NEXUS_SUCCESS;
    BERR_Code SAGElib_rc;

    BDBG_ENTER(NEXUS_Sage_Standby_priv);

    BDBG_MSG(("%s: enabled=%s, mode=%u", BSTD_FUNCTION, enabled ? "true" : "false", pSettings->mode));

    if (!pSettings) {
        BDBG_ERR(("%s: pSettings is not set.", BSTD_FUNCTION));
        nexus_rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    /* check if SAGE booted */
    if(!NEXUS_Sage_P_CheckSageBooted()) {
        BDBG_ERR(("%s: SAGE (re)boot failure", BSTD_FUNCTION));
        nexus_rc = BERR_TRACE(NEXUS_TIMEOUT);
        goto end;
    }

    if (!enabled) {
        /* refresh HSM handle on resume */
        nexus_rc = NEXUS_Sage_P_SAGElibUpdateHsm(1);
        if (nexus_rc != NEXUS_SUCCESS) {
            goto end;
        }
    }

    /* Open a client instance + prepare resources to handle PM transitions */
    nexus_rc = NEXUS_Sage_P_StandbyInit();
    if (nexus_rc != NEXUS_SUCCESS) {
        goto end;
    }

    /* Set the new mode in SAGElib */
    {
        BSAGElib_eStandbyMode SAGElibStandbyMode;
        /* Convert Nexus -- > BSAGElib standby mode */
        if (enabled) {
            SAGElibStandbyMode = NEXUS_Sage_P_StandbyConvertMode(pSettings->mode);
            if (SAGElibStandbyMode == BSAGElib_eStandbyMax) { goto end; }
        }
        else {
            SAGElibStandbyMode = BSAGElib_eStandbyModeOn;
        }

        if(SAGElibStandbyMode == BSAGElib_eStandbyModeDeepSleep)
        {
            /* Scrub and disable URR/XRR */
            NEXUS_Sage_P_SvpEnterS3();
            NEXUS_Sage_P_SvpStop(false);
            NEXUS_Sage_P_BP3Uninit();
            NEXUS_Sage_P_SecureLog_Uninit();
            NEXUS_Sage_P_ARUninit(SAGElibStandbyMode);
        }

        SAGElib_rc = BSAGElib_Standby(g_sage_standby.hSAGElibClient, SAGElibStandbyMode);
        if (SAGElib_rc != BERR_SUCCESS) {
            nexus_rc = BERR_TRACE(NEXUS_OS_ERROR);
            goto end;
        }
    }

    if (!enabled) {
        /* leaving standby */
        if (g_sage_standby.currMode == NEXUS_StandbyMode_eDeepSleep) {
            BSAGElib_Management_Reset(g_NEXUS_sageModule.hSAGElib);
            nexus_rc = NEXUS_SageModule_P_Start();
            if (nexus_rc != NEXUS_SUCCESS) {
                goto end;
            }
        }
        NEXUS_Sage_P_StandbyUninit();
    }
    else {
        /* entering standby */
        if (pSettings->mode == NEXUS_StandbyMode_eDeepSleep) {
            NEXUS_Sage_P_SAGELogUninit();
            NEXUS_Sage_P_StandbyUninit();
        }
        /* if g_sage_standby.currMode == NEXUS_StandbyMode_ePassive (S2):
           the resources are cleared inside the response callback process handling
           see NEXUS_Sage_P_ResponseStandbyCallback_isr */
    }

    g_sage_standby.currMode = pSettings->mode;

    if (enabled) {
        /* Clear hHSM handle on pause */
        NEXUS_Sage_P_SAGElibUpdateHsm(0);
    }

end:
    if (nexus_rc != NEXUS_SUCCESS) {
        NEXUS_Sage_P_StandbyUninit();
    }
    BDBG_LEAVE(NEXUS_Sage_Standby_priv);
    return nexus_rc;
}


#else /* NEXUS_POWER_MANAGEMENT */


/* in case NEXUS Power Management is disabled, do nothing */

NEXUS_Error
NEXUS_SageModule_Standby_priv(
    bool enabled,
    const NEXUS_StandbySettings *pSettings)
{
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
}


#endif /* NEXUS_POWER_MANAGEMENT */

void
NEXUS_SageModule_P_PrivClean(void)
{
    /* Clean all resources related to *_priv functions */
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Sage_P_StandbyClean();
#endif
    return;
}
