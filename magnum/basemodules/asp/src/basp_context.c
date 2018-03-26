/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "basp.h"
#include "basp_priv.h"
#include "bdbg.h"

BDBG_MODULE(BASP_CONTEXT);
BDBG_OBJECT_ID(BASP_P_Context);           /* BASP_ContextHandle */
BDBG_OBJECT_ID_DECLARE(BASP_P_Device);

/******************************************************************************/
void BASP_Context_GetDefaultCreateSettings(
    BASP_ContextCreateSettings *pSettings /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}


/******************************************************************************/
BERR_Code BASP_Context_Create(
  BASP_Handle hAsp,
  const BASP_ContextCreateSettings *pSettings,
  BASP_ContextHandle *pHandle /* [out] */
  )
{
    BERR_Code errCode = BERR_SUCCESS;
    BASP_ContextHandle hContext;

    BDBG_OBJECT_ASSERT(hAsp, BASP_P_Device);
                    BDBG_ASSERT(NULL != pHandle);
    BDBG_ASSERT(NULL != pSettings);

    *pHandle = NULL;

    /* Make sure settings are valid */
    switch ( pSettings->type )
    {
    case BASP_ContextType_eStreaming:
    case BASP_ContextType_eMux:
        break;

    default:
        BDBG_ERR(("Context BASP_ContextType %u is invalid or not supported", pSettings->type ));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto error;
        break;
    }

    hContext = BKNI_Malloc(sizeof(BASP_P_Context));

    BDBG_MSG(("hContext from malloc=%p", (void *)hContext));

    if ( NULL == hContext )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    BKNI_Memset(hContext, 0, sizeof(*hContext));
    BDBG_OBJECT_SET(hContext, BASP_P_Context);

    BKNI_EnterCriticalSection();
    BLST_S_INSERT_HEAD(&hAsp->contextList, hContext, nextContext);
    BKNI_LeaveCriticalSection();

    hContext->sCreateSettings = *pSettings;
    hContext->hAsp = hAsp;

    BLST_S_INIT(&hContext->channelList);

    *pHandle = hContext;

error:
    return errCode;
}


/******************************************************************************/

void BASP_Context_Destroy(
  BASP_ContextHandle hContext
  )
{
    BDBG_OBJECT_ASSERT(hContext, BASP_P_Context);

    /* Make sure no channels are running. */
    /* TODO: */

    /* Destroy all Channels that belong to this Context */
    /* TODO: */

    BDBG_LOG(("hContext entry =%p", (void *)hContext));

    /* Unlink from device's context list. */
    BKNI_EnterCriticalSection();
    BLST_S_REMOVE(&hContext->hAsp->contextList, hContext, BASP_P_Context, nextContext);
    BKNI_LeaveCriticalSection();
    BDBG_OBJECT_DESTROY(hContext, BASP_P_Context);

    /* Free memory. */
    BDBG_LOG(("hContext freeing =%p", (void *)hContext));
    BKNI_Free(hContext);
}


/******************************************************************************/
void BASP_Context_GetCallbacks(
  BASP_ContextHandle hContext,
  BASP_ContextCallbacks *pCallbacks /* [out] */
  )
{
    BSTD_UNUSED(hContext);
    BSTD_UNUSED(pCallbacks);

    /* TODO: */

    return;
}


/******************************************************************************/
BERR_Code BASP_Context_SetCallbacks(
  BASP_ContextHandle hContext,
  const BASP_ContextCallbacks *pCallbacks
  )
{

    BSTD_UNUSED(hContext);
    BSTD_UNUSED(pCallbacks);

    /* TODO: */

    return (BERR_SUCCESS);

}

/******************************************************************************/
BERR_Code BASP_Context_ProcessWatchdog(
  BASP_ContextHandle hContext
  )
{

    BSTD_UNUSED(hContext);

    /* TODO: */

    return (BERR_SUCCESS);

}
