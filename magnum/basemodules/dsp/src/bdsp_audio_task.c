/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/


#include "bdsp.h"
#include "bdsp_priv.h"
#include "bdsp_raaga_fw_settings.h"


BDBG_MODULE(bdsp_audio_task);


BERR_Code BDSP_AudioTask_Pause(
    BDSP_TaskHandle task
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->pause )
    {
        return task->pause(task->pTaskHandle);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_AudioTask_Resume(
    BDSP_TaskHandle task
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->resume )
    {
        return task->resume(task->pTaskHandle);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_AudioTask_Advance(
    BDSP_TaskHandle task,
    unsigned ms
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    if ( task->advance )
    {
        return task->advance(task->pTaskHandle, ms);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}
/* PAUSE-UNPAUSE */
void BDSP_AudioTask_GetDefaultFreezeSettings(
    BDSP_AudioTaskFreezeSettings *pSettings /*[out]*/
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

void BDSP_AudioTask_GetDefaultUnFreezeSettings(
    BDSP_AudioTaskUnFreezeSettings *pSettings /*[out]*/
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

BERR_Code BDSP_AudioTask_Freeze(
    BDSP_TaskHandle htask,
    const BDSP_AudioTaskFreezeSettings *pFreezeSettings
    )
{
     BDBG_OBJECT_ASSERT(htask, BDSP_Task);
     BDBG_ASSERT(NULL != pFreezeSettings);

     BDBG_MSG(("%s", BSTD_FUNCTION));
     BDBG_MSG(("      Dummy addr %08x mask %08x value %08x",
     pFreezeSettings->fmmOutputAddress,
     pFreezeSettings->fmmOutputMask,
     pFreezeSettings->fmmOutputValue));

     if ( htask->freeze )
     {
         return htask->freeze(htask->pTaskHandle, pFreezeSettings);
     }
     else
     {
         return BERR_TRACE(BERR_NOT_SUPPORTED);
     }
}


BERR_Code BDSP_AudioTask_UnFreeze(
    BDSP_TaskHandle htask,
    const BDSP_AudioTaskUnFreezeSettings *pUnFreezeSettings
    )
{
     BDBG_OBJECT_ASSERT(htask, BDSP_Task);
     BDBG_ASSERT(NULL != pUnFreezeSettings);

     BDBG_MSG(("%s", BSTD_FUNCTION));
     BDBG_MSG(("      Dummy addr %08x mask %08x value %08x",
     pUnFreezeSettings->fmmOutputAddress,
     pUnFreezeSettings->fmmOutputMask,
     pUnFreezeSettings->fmmOutputValue));

     if ( htask->unfreeze )
     {
         return htask->unfreeze(htask->pTaskHandle, pUnFreezeSettings);
     }
     else
     {
         return BERR_TRACE(BERR_NOT_SUPPORTED);
     }

}
/* PAUSE-UNPAUSE */

/***************************************************************************
Summary:
Get Current Interrupt Handlers for a task
***************************************************************************/
void BDSP_AudioTask_GetInterruptHandlers_isr(
    BDSP_TaskHandle task,
    BDSP_AudioInterruptHandlers *pHandlers   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    BDBG_ASSERT(NULL != pHandlers);

    BKNI_ASSERT_ISR_CONTEXT();

    if ( task->getAudioInterruptHandlers_isr)
    {
         task->getAudioInterruptHandlers_isr(task->pTaskHandle,pHandlers);
    }
}

/***************************************************************************
Summary:
Set Current Interrupt Handlers for a task
***************************************************************************/
BERR_Code BDSP_AudioTask_SetInterruptHandlers_isr(
    BDSP_TaskHandle task,
    const BDSP_AudioInterruptHandlers *pHandlers   /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);
    BDBG_ASSERT(NULL != pHandlers);

    BKNI_ASSERT_ISR_CONTEXT();

    if ( task->setAudioInterruptHandlers_isr)
    {
        return task->setAudioInterruptHandlers_isr(task->pTaskHandle,pHandlers);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

BERR_Code BDSP_AudioTask_AudioGapFill(
    BDSP_TaskHandle task
    )
{
    BDBG_OBJECT_ASSERT(task, BDSP_Task);

    if ( task->audioGapFill )
    {
        return task->audioGapFill(task->pTaskHandle);
    }
    else
    {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
}

/***************************************************************************
Summary:
Get the codec copability status
***************************************************************************/
void BDSP_GetCodecCapabilities_isrsafe(
    BDSP_CodecCapabilities *pSetting
)
{
#ifdef BDSP_MS12_SUPPORT
    switch (BDSP_MS12_SUPPORT)
    {
        case 'A':
            BDBG_MSG(("BDSP detected BDSP_MS12_SUPPORT = 'A' "));
            pSetting->dolbyMs.dapv2 = true;
            pSetting->dolbyMs.ddEncode = true;
            pSetting->dolbyMs.ddpEncode51 = true;
            pSetting->dolbyMs.ddpEncode71 = true;
            pSetting->dolbyMs.pcm71 = true;
            break;
        case 'B':
            BDBG_MSG(("BDSP detected BDSP_MS12_SUPPORT = 'B' "));
            pSetting->dolbyMs.dapv2 = true;
            pSetting->dolbyMs.ddEncode = true;
            pSetting->dolbyMs.ddpEncode51 = true;
            pSetting->dolbyMs.ddpEncode71 = false;
            pSetting->dolbyMs.pcm71 = false;
            break;
        case 'C':
            BDBG_MSG(("BDSP detected BDSP_MS12_SUPPORT = 'C' "));
            pSetting->dolbyMs.dapv2 = false;
            pSetting->dolbyMs.ddEncode = true;
            pSetting->dolbyMs.ddpEncode51 = false;
            pSetting->dolbyMs.ddpEncode71 = false;
            pSetting->dolbyMs.pcm71 = false;
            break;
        case 'D':
            BDBG_MSG(("BDSP detected BDSP_MS12_SUPPORT = 'D' "));
            pSetting->dolbyMs.dapv2 = false;
            pSetting->dolbyMs.ddEncode = true;
            pSetting->dolbyMs.ddpEncode51 = true;
            pSetting->dolbyMs.ddpEncode71 = false;
            pSetting->dolbyMs.pcm71 = false;
            break;
        case 'E': /* MS11+ Mode */
            BDBG_MSG(("BDSP detected BDSP_MS12_SUPPORT = 'MS11+' "));
            pSetting->dolbyMs.dapv2 = false;
            pSetting->dolbyMs.ddEncode = true;
            pSetting->dolbyMs.ddpEncode51 = false;
            pSetting->dolbyMs.ddpEncode71 = false;
            pSetting->dolbyMs.pcm71 = false;
            break;
        default:
            BDBG_MSG(("BDSP detected BDSP_MS12_SUPPORT = 'C', Displaying MS12 Capabilities as: "));
            pSetting->dolbyMs.dapv2 = true;
            pSetting->dolbyMs.ddEncode = true;
            pSetting->dolbyMs.ddpEncode51 = false;
            pSetting->dolbyMs.ddpEncode71 = false;
            pSetting->dolbyMs.pcm71 = false;
            break;
    }
#else
            pSetting->dolbyMs.dapv2 = false;
#ifdef BDSP_AC3ENC_SUPPORT
            pSetting->dolbyMs.ddEncode = true;
#else
            pSetting->dolbyMs.ddEncode = false;
#endif
            pSetting->dolbyMs.ddpEncode51 = false;
            pSetting->dolbyMs.ddpEncode71 = false;
            pSetting->dolbyMs.pcm71 = false;
#endif

            BDBG_MSG(("pSetting->dolbyMs.dapv2 = %d", pSetting->dolbyMs.dapv2));
            BDBG_MSG(("pSetting->dolbyMs.ddEncode = %d", pSetting->dolbyMs.ddEncode));
            BDBG_MSG(("pSetting->dolbyMs.ddpEncode51 = %d", pSetting->dolbyMs.ddpEncode51));
            BDBG_MSG(("pSetting->dolbyMs.ddpEncode71 = %d", pSetting->dolbyMs.ddpEncode71));
            BDBG_MSG(("pSetting->dolbyMs.pcm71 = %d", pSetting->dolbyMs.pcm71));
}
