/***************************************************************************
*  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* API Description:
*   API name: AudioDsp
*    Specific APIs related to audio DSP control and debug.
*
***************************************************************************/

#include "nexus_audio_module.h"

#define RAAGA_DEBUG_LOG_CHANGES 1

#if BAPE_DSP_SUPPORT

#include "bdsp.h"

BDBG_MODULE(nexus_audio_dsp);

NEXUS_Error NEXUS_AudioDsp_GetDebugBuffer(
    unsigned dspIndex,
    NEXUS_AudioDspDebugType debugType,
    const void **pBuffer,   /* [out] attr{memory=cached} Pointer to debug data */
    size_t *pBufferSize     /* [out] size of buffer in bytes */
    )
{
    BERR_Code errCode;
    BDSP_MMA_Memory debugBuffer;
    size_t bufferSize;
    BDSP_Handle hDsp = NULL;

    if ( !BAPE_DEVICE_DSP_VALID(dspIndex) && !BAPE_DEVICE_ARM_VALID(dspIndex) )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ( debugType >= NEXUS_AudioDspDebugType_eMax )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_ASSERT(NULL != pBuffer);
    BDBG_ASSERT(NULL != pBufferSize);
    
    if ( g_NEXUS_audioModuleData.settings.dspDebugSettings.typeSettings[debugType].enabled == false )
    {
        BDBG_ERR(("Debug type %u is not enabled.  Please check the settings in NEXUS_AudioModuleSettings.dspDebugSettings.", debugType));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( g_NEXUS_audioModuleData.dspHandle && BAPE_DEVICE_DSP_VALID(dspIndex) )
    {
        hDsp = g_NEXUS_audioModuleData.dspHandle;
    }
    else if ( g_NEXUS_audioModuleData.armHandle && BAPE_DEVICE_ARM_VALID(dspIndex) )
    {
        hDsp = g_NEXUS_audioModuleData.armHandle;
    }

    if ( hDsp )
    {
        errCode = BDSP_GetDebugBuffer(hDsp, (BDSP_DebugType)debugType, dspIndex, &debugBuffer, &bufferSize);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        if ( bufferSize > 0 )
        {
            *pBuffer = debugBuffer.pAddr;
            *pBufferSize = bufferSize;

            BMMA_FlushCache(debugBuffer.hBlock, debugBuffer.pAddr, bufferSize);
        }
        else
        {
            *pBuffer = NULL;
            *pBufferSize = 0;

            /* After all of the core dump has been consumed for the DSP, restart */
            if ( debugType == NEXUS_AudioDspDebugType_eCoreDump && g_NEXUS_audioModuleData.watchdogDeferred )
            {
                /* Restart DSP now */
                g_NEXUS_audioModuleData.watchdogDeferred = false;
                NEXUS_AudioDecoder_P_Reset();
            }
        }
    }

    return BERR_SUCCESS;
}


NEXUS_Error NEXUS_AudioDsp_DebugReadComplete(
    unsigned dspIndex,
    NEXUS_AudioDspDebugType debugType,
    size_t bytesRead        /* Number of bytes consumed from the debug buffer */
    )
{
    BERR_Code errCode;
    BDSP_Handle hDsp = NULL;
    unsigned dspOffset;
    unsigned dspBase = 0;
    unsigned numCores = 0;
    NEXUS_AudioCapabilities caps;

    NEXUS_GetAudioCapabilities(&caps);

    if ( !BAPE_DEVICE_DSP_VALID(dspIndex) && !BAPE_DEVICE_ARM_VALID(dspIndex) )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ( debugType >= NEXUS_AudioDspDebugType_eMax )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( bytesRead == 0 )
    {
        return BERR_SUCCESS;
    }

    if ( g_NEXUS_audioModuleData.settings.dspDebugSettings.typeSettings[debugType].enabled == false )
    {
        BDBG_ERR(("Debug type %u is not enabled.  Please check the settings in NEXUS_AudioModuleSettings.dspDebugSettings.", debugType));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( g_NEXUS_audioModuleData.dspHandle && BAPE_DEVICE_DSP_VALID(dspIndex) )
    {
        hDsp = g_NEXUS_audioModuleData.dspHandle;
        dspBase = BAPE_DEVICE_DSP_FIRST;
        numCores = caps.numDsps;
    }
    else if ( g_NEXUS_audioModuleData.armHandle && BAPE_DEVICE_ARM_VALID(dspIndex) )
    {
        hDsp = g_NEXUS_audioModuleData.armHandle;
        dspBase = BAPE_DEVICE_ARM_FIRST;
        numCores = caps.numSoftAudioCores;
    }

    dspOffset = BAPE_DSP_DEVICE_INDEX(dspIndex, dspBase);
    if (dspOffset >= numCores) {
        BDBG_ERR(("Audio Core Index %u not supported(%u)", dspOffset, numCores));
    }

    if ( hDsp )
    {
        errCode = BDSP_ConsumeDebugData(hDsp, (BDSP_DebugType)debugType, dspOffset, bytesRead);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}
#else
NEXUS_Error NEXUS_AudioDsp_GetDebugBuffer(
    unsigned dspIndex,
    NEXUS_AudioDspDebugType debugType,
    const void **pBuffer,   /* [out] attr{memory=cached} Pointer to debug data */
    size_t *pBufferSize     /* [out] size of buffer in bytes */
    )
{
    BSTD_UNUSED(dspIndex);
    BSTD_UNUSED(debugType);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pBufferSize);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);

}

NEXUS_Error NEXUS_AudioDsp_DebugReadComplete(
    unsigned dspIndex,
    NEXUS_AudioDspDebugType debugType,
    size_t bytesRead        /* Number of bytes consumed from the debug buffer */
    )
{
    BSTD_UNUSED(dspIndex);
    BSTD_UNUSED(debugType);
    BSTD_UNUSED(bytesRead);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

#endif
