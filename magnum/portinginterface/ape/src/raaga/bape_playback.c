/***************************************************************************
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
 *
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bape_buffer.h"
#if BCHP_AUD_FMM_BF_CTRL_REG_START
#include "bchp_aud_fmm_bf_ctrl.h"
#endif

BDBG_MODULE(bape_playback);

#if BAPE_CHIP_MAX_PLAYBACKS > 0

BDBG_OBJECT_ID(BAPE_Playback);

#define BAPE_PLAYBACK_USE_TIMER_BASED_INTERRUPTS    0

#if BAPE_PLAYBACK_USE_TIMER_BASED_INTERRUPTS
#define BAPE_PLAYBACK_INTERRUPT_TIMER_DURATION 10000 /* microseconds */
static void BAPE_P_CheckPlaybackStatus_isr (void *pParam1, int param2);
#endif
static BERR_Code BAPE_Playback_P_ReArmWatermark(BAPE_PlaybackHandle hPlayback);
static BERR_Code BAPE_Playback_P_ConfigPathToOutput(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection);
static BERR_Code BAPE_Playback_P_StartPathToOutput(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection);
static void BAPE_Playback_P_StopPathToOutput(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection);

void BAPE_Playback_GetDefaultOpenSettings(
    BAPE_PlaybackOpenSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(BAPE_PlaybackOpenSettings));
    pSettings->numBuffers = 1;
    pSettings->bufferSize = 128*1024;
    pSettings->watermarkThreshold = 64*1024;
    pSettings->interleaved = true;
    pSettings->multichannelFormat = BAPE_MultichannelFormat_e2_0;
}

static void BAPE_Playback_P_FreeBuffer(BAPE_PlaybackHandle hPlayback, unsigned idx)
{
    if ( hPlayback->block[idx] )
    {
        if ( hPlayback->pBuffer[idx] )
        {
            BMMA_Unlock(hPlayback->block[idx], hPlayback->pBuffer[idx]);
            hPlayback->pBuffer[idx] = NULL;
        }
        if ( hPlayback->offset[idx] )
        {
            BMMA_UnlockOffset(hPlayback->block[idx], hPlayback->offset[idx]);
            hPlayback->offset[idx] = 0;
        }
        BMMA_Free(hPlayback->block[idx]);
        hPlayback->block[idx] = NULL;
    }
    if ( hPlayback->bufferInterfaceBlock[idx] )
    {
        if ( hPlayback->pBufferInterface[idx] )
        {
            BMMA_Unlock(hPlayback->bufferInterfaceBlock[idx], hPlayback->pBufferInterface[idx]);
            hPlayback->pBufferInterface[idx] = NULL;
        }
        if ( hPlayback->bufferInterfaceOffset[idx] )
        {
            BMMA_UnlockOffset(hPlayback->bufferInterfaceBlock[idx], hPlayback->bufferInterfaceOffset[idx]);
            hPlayback->bufferInterfaceOffset[idx] = 0;
        }
        BMMA_Free(hPlayback->bufferInterfaceBlock[idx]);
        hPlayback->bufferInterfaceBlock[idx] = NULL;
    }
}

static BERR_Code BAPE_Playback_P_CreateBufferGroup(BAPE_PlaybackHandle hPlayback, BAPE_MultichannelFormat multichannelFormat)
{
    BERR_Code errCode;
    unsigned numBuffers = 1, numChannelPairs;
    switch ( multichannelFormat )
    {
    default:
    case BAPE_MultichannelFormat_e2_0:
        numBuffers = 1;
        break;
    case BAPE_MultichannelFormat_e5_1:
        numBuffers = 3;
        break;
    case BAPE_MultichannelFormat_e7_1:
        numBuffers = 4;
        break;
    }

    numChannelPairs = numBuffers;

    if ( !hPlayback->interleaved )
    {
        numBuffers *= 2;
    }

    if ( numBuffers > hPlayback->numBuffers )
    {
        BDBG_ERR(("Requested %d buffers, but this playback channel has only %d buffers", (int)numBuffers, (int)hPlayback->numBuffers));
        return BERR_TRACE(BERR_NOT_AVAILABLE);
    }

    if ( hPlayback->bufferGroupHandle )
    {
        BAPE_BufferGroupStatus bufferGroupStatus;

        BAPE_BufferGroup_GetStatus_isrsafe(hPlayback->bufferGroupHandle, &bufferGroupStatus);
        if ( bufferGroupStatus.interleaved != hPlayback->interleaved || numBuffers != bufferGroupStatus.numChannels )
        {
            BAPE_BufferGroup_Close(hPlayback->bufferGroupHandle);
            hPlayback->bufferGroupHandle = NULL;
        }
    }

    if ( !hPlayback->bufferGroupHandle )
    {
        unsigned i;
        BAPE_BufferGroupOpenSettings bufferGroupSettings;
        BAPE_BufferGroup_GetDefaultOpenSettings(&bufferGroupSettings);
        for (i = 0; i < numChannelPairs; i++)
        {
            bufferGroupSettings.buffers[i*2].pInterface = hPlayback->pBufferInterface[i];
            bufferGroupSettings.buffers[i*2].interfaceBlock = hPlayback->bufferInterfaceBlock[i];
            bufferGroupSettings.buffers[i*2].interfaceOffset = hPlayback->bufferInterfaceOffset[i];
            bufferGroupSettings.buffers[i*2].pBuffer = hPlayback->pBuffer[i];
            if ( !hPlayback->interleaved )
            {
                bufferGroupSettings.buffers[i*2+1].pInterface = hPlayback->pBufferInterface[i+1];
                bufferGroupSettings.buffers[i*2+1].interfaceBlock = hPlayback->bufferInterfaceBlock[i+1];
                bufferGroupSettings.buffers[i*2+1].interfaceOffset = hPlayback->bufferInterfaceOffset[i+1];
                bufferGroupSettings.buffers[i*2+1].pBuffer = hPlayback->pBuffer[i+1];
            }
        }
        bufferGroupSettings.type = BAPE_BufferType_eReadWrite;
        bufferGroupSettings.bufferSize = hPlayback->bufferSize;
        bufferGroupSettings.interleaved = hPlayback->interleaved;
        bufferGroupSettings.numChannels = numBuffers;

        errCode = BAPE_BufferGroup_Open(hPlayback->hApe, &bufferGroupSettings, &hPlayback->bufferGroupHandle);
        if ( errCode != BERR_SUCCESS || hPlayback->bufferGroupHandle == NULL )
        {
            BDBG_ERR(("ERROR, unable to create buffer interface"));
            return BERR_TRACE(errCode);
        }

        /* Flush newly created group in case we had stale data in any of the buffers */
        BAPE_BufferGroup_Flush(hPlayback->bufferGroupHandle);
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_Playback_Open(
    BAPE_Handle hApe,
    unsigned index,
    const BAPE_PlaybackOpenSettings *pSettings,
    BAPE_PlaybackHandle *pHandle                    /* [out] */
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BAPE_FMT_Descriptor format;
    BAPE_PlaybackHandle hPlayback;
    BAPE_PlaybackOpenSettings defaultSettings;
    unsigned i;

    BDBG_OBJECT_ASSERT(hApe, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    *pHandle = NULL;

    if ( NULL == pSettings )
    {
        BAPE_Playback_GetDefaultOpenSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    if ( index >= BAPE_CHIP_MAX_PLAYBACKS )
    {
        BDBG_ERR(("This chip only supports %u playbacks.  Cannot open playback %u", BAPE_CHIP_MAX_PLAYBACKS, index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( hApe->playbacks[index] )
    {
        BDBG_ERR(("Playback %d already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* TODO: Extend for multichannel and non-interleved */
    if ( pSettings->numBuffers > BAPE_Channel_eMax )
    {
        BDBG_ERR(("Maximum num of buffers required for multichannel non-interleved audio playback cannot exceed %u. Requested num buffers is %u", BAPE_Channel_eMax, pSettings->numBuffers));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    hPlayback = BKNI_Malloc(sizeof(BAPE_Playback));
    if ( NULL == hPlayback )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(hPlayback, 0, sizeof(BAPE_Playback));
    hPlayback->hApe = hApe;
    hPlayback->hHeap = ( pSettings->heap ) ? pSettings->heap : hApe->memHandle;

    #if (BAPE_CHIP_MAX_SFIFOS == 0)
    hPlayback->bufferInterfaceType = BAPE_BufferInterfaceType_eDram;
    #else
    hPlayback->bufferInterfaceType = BAPE_BufferInterfaceType_eRdb;
    #endif

    if ( pSettings->watermarkThreshold >= pSettings->bufferSize )
    {
        BDBG_WRN(("playback watermark threshold %u is larger than the buffer size %u.  Reducing to %u.", pSettings->watermarkThreshold, pSettings->bufferSize, pSettings->bufferSize / 2));
        hPlayback->threshold = pSettings->bufferSize / 2;
    }
    else
    {
        hPlayback->threshold = pSettings->watermarkThreshold;
    }

    for (i = 0; i < pSettings->numBuffers; i++)
    {
        unsigned allocSize;

        allocSize = pSettings->bufferSize + BAPE_CHIP_SFIFO_PADDING;
        BAPE_SIZE_ALIGN(allocSize);
        hPlayback->block[i] = BMMA_Alloc(hPlayback->hHeap, allocSize, BAPE_ADDRESS_ALIGN, NULL);
        if ( NULL == hPlayback->block[i] )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_buffer_alloc;
        }
        hPlayback->pBuffer[i] = BMMA_Lock(hPlayback->block[i]);
        if ( NULL == hPlayback->pBuffer[i] )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            BAPE_Playback_P_FreeBuffer(hPlayback, i);
            goto err_buffer_alloc;
        }
        hPlayback->offset[i] = BMMA_LockOffset(hPlayback->block[i]);
        if ( 0 == hPlayback->offset[i] )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            BAPE_Playback_P_FreeBuffer(hPlayback, i);
            goto err_buffer_alloc;
        }

        BKNI_Memset(hPlayback->pBuffer[i], 0, sizeof(allocSize));
        BAPE_FLUSHCACHE_ISRSAFE(hPlayback->block[i], hPlayback->pBuffer[i], sizeof(allocSize));
        hPlayback->bufferSize = allocSize;

        if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
        {
            allocSize = sizeof(BAPE_BufferInterface);
            BAPE_SIZE_ALIGN(allocSize);
            hPlayback->bufferInterfaceBlock[i] = BMMA_Alloc(hPlayback->hHeap, allocSize, BAPE_ADDRESS_ALIGN, NULL);
            if ( NULL == hPlayback->bufferInterfaceBlock[i] )
            {
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                goto err_buffer_alloc;
            }
            hPlayback->pBufferInterface[i] = BMMA_Lock(hPlayback->bufferInterfaceBlock[i]);
            if ( NULL == hPlayback->pBufferInterface[i] )
            {
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BAPE_Playback_P_FreeBuffer(hPlayback, i);
                goto err_buffer_alloc;
            }
            hPlayback->bufferInterfaceOffset[i] = BMMA_LockOffset(hPlayback->bufferInterfaceBlock[i]);
            if ( 0 == hPlayback->bufferInterfaceOffset[i] )
            {
                errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                BAPE_Playback_P_FreeBuffer(hPlayback, i);
                goto err_buffer_alloc;
            }

            BKNI_Memset(hPlayback->pBufferInterface[i], 0, sizeof(BAPE_BufferInterface));
            BAPE_FLUSHCACHE_ISRSAFE(hPlayback->bufferInterfaceBlock[i], hPlayback->pBufferInterface[i], sizeof(BAPE_BufferInterface));

            hPlayback->pBufferInterface[i]->block = hPlayback->block[i];
            /*hPlayback->pBufferInterface[i]->pBuffer = hPlayback->pBuffer[i];*/
            hPlayback->pBufferInterface[i]->base = hPlayback->offset[i];
            hPlayback->pBufferInterface[i]->read = hPlayback->pBufferInterface[i]->base;
            hPlayback->pBufferInterface[i]->valid = hPlayback->pBufferInterface[i]->base;
            hPlayback->pBufferInterface[i]->end = hPlayback->pBufferInterface[i]->base + hPlayback->bufferSize;
            hPlayback->pBufferInterface[i]->watermark = hPlayback->threshold;
        }
    }

    hPlayback->index = index;
    hPlayback->numBuffers = pSettings->numBuffers;
    hPlayback->interleaved = pSettings->interleaved;
    hPlayback->settings.multichannelFormat = pSettings->multichannelFormat;
    hPlayback->settings.compressedData = false;
    hPlayback->settings.sampleRate = 44100;
    BAPE_P_InitPathNode(&hPlayback->node, BAPE_PathNodeType_ePlayback, 0, 1, hApe, hPlayback);
    BKNI_Snprintf(hPlayback->name, sizeof(hPlayback->name), "Playback %u", index);
    hPlayback->node.pName = hPlayback->name;

    BAPE_Connector_P_GetFormat(&hPlayback->node.connectors[0], &format);
    format.type = BAPE_DataType_ePcmStereo;
    format.source = BAPE_DataSource_eHostBuffer;
    format.sampleRate = 44100;
    errCode = BAPE_Connector_P_SetFormat(&hPlayback->node.connectors[0], &format);
    if ( errCode ) { errCode = BERR_TRACE(errCode); goto err_format; }

    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
    {
        errCode = BAPE_Playback_P_CreateBufferGroup(hPlayback, pSettings->multichannelFormat);
        if ( errCode )
        {
            BERR_TRACE(errCode);
            goto err_buffer_alloc;
        }
    }

    /* Init node callbacks */
    hPlayback->node.configPathToOutput = BAPE_Playback_P_ConfigPathToOutput;
    hPlayback->node.startPathToOutput = BAPE_Playback_P_StartPathToOutput;
    hPlayback->node.stopPathToOutput = BAPE_Playback_P_StopPathToOutput;
    BDBG_OBJECT_SET(hPlayback, BAPE_Playback);
    *pHandle = hPlayback;
    hApe->playbacks[index] = hPlayback;

    return errCode;

err_format:
err_buffer_alloc:
    for (i = 0; i < pSettings->numBuffers; i++)
    {
        BAPE_Playback_P_FreeBuffer(hPlayback, i);
    }

    BKNI_Free(hPlayback);

    return errCode;
}

void BAPE_Playback_Close(
    BAPE_PlaybackHandle hPlayback
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    if ( hPlayback->interruptTimer )
    {
        BKNI_EnterCriticalSection();
        BTMR_StopTimer_isr(hPlayback->interruptTimer);
        BKNI_LeaveCriticalSection();

        BTMR_DestroyTimer(hPlayback->interruptTimer);
        hPlayback->interruptTimer = NULL;
    }

    if ( hPlayback->running )
    {
        BDBG_WRN(("Stopping playback %p (%d) on close", (void *)hPlayback, hPlayback->index));
        BAPE_Playback_Stop(hPlayback);
    }

    /* Disconnect from all mixers, post-processors, and groups */
    BAPE_Connector_P_RemoveAllConnections(&hPlayback->node.connectors[0]);

    if ( hPlayback->bufferGroupHandle )
    {
        BAPE_BufferGroup_Close(hPlayback->bufferGroupHandle);
        hPlayback->bufferGroupHandle = NULL;
    }

    for (i = 0; i < hPlayback->numBuffers; i++)
    {
        BAPE_Playback_P_FreeBuffer(hPlayback, i);
    }

    hPlayback->hApe->playbacks[hPlayback->index] = NULL;
    BDBG_OBJECT_DESTROY(hPlayback, BAPE_Playback);
    BKNI_Free(hPlayback);
}

void BAPE_Playback_GetSettings(
    BAPE_PlaybackHandle hPlayback,
    BAPE_PlaybackSettings *pSettings    /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);
    BDBG_ASSERT(NULL != pSettings);

    *pSettings = hPlayback->settings;
}

BERR_Code BAPE_Playback_SetSettings(
    BAPE_PlaybackHandle hPlayback,
    const BAPE_PlaybackSettings *pSettings
    )
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);
    BDBG_ASSERT(NULL != pSettings);

    /* If we're already running, return error */
    if ( hPlayback->running )
    {
        if ( hPlayback->settings.multichannelFormat != pSettings->multichannelFormat ||
             hPlayback->settings.compressedData != pSettings->compressedData )
        {
            BDBG_ERR(("Playback %p (%d) is already running.  Cannot change data format.", (void *)hPlayback, hPlayback->index));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        if ( hPlayback->startSettings.sampleRate == 0 && hPlayback->settings.sampleRate != pSettings->sampleRate )
        {
            if ( pSettings->sampleRate == 0 )
            {
                BDBG_ERR(("Can not use sample rate of 0."));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            /* Update Sample Rate */
            BKNI_EnterCriticalSection();
            BAPE_Connector_P_SetSampleRate_isr(&hPlayback->node.connectors[0], pSettings->sampleRate);
            BKNI_LeaveCriticalSection();
        }
    }
    else
    {
        BAPE_FMT_Descriptor format;

        BAPE_Connector_P_GetFormat(&hPlayback->node.connectors[0], &format);
        if ( pSettings->compressedData == true )
        {
            format.type = BAPE_DataType_eIec61937;
        }
        else
        {
            switch ( pSettings->multichannelFormat )
            {
            case BAPE_MultichannelFormat_e2_0:
                format.type = BAPE_DataType_ePcmStereo;
                break;
            case BAPE_MultichannelFormat_e5_1:
                format.type = BAPE_DataType_ePcm5_1;
                break;
            case BAPE_MultichannelFormat_e7_1:
                format.type = BAPE_DataType_ePcm7_1;
                break;
            default:
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }
        errCode = BAPE_Connector_P_SetFormat(&hPlayback->node.connectors[0], &format);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
        }

        if ( hPlayback->settings.multichannelFormat != pSettings->multichannelFormat )
        {
            /* tear down, recreate, flush buffer group */
            errCode = BAPE_Playback_P_CreateBufferGroup(hPlayback, pSettings->multichannelFormat);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }

    }

    hPlayback->settings = *pSettings;
    return BERR_SUCCESS;
}

void BAPE_Playback_GetDefaultStartSettings(
    BAPE_PlaybackStartSettings *pSettings       /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    pSettings->sampleRate = 44100;
    pSettings->bitsPerSample = 16;
    pSettings->isStereo = true;
    pSettings->isSigned = true;
    pSettings->reverseEndian = false;
    pSettings->loopEnabled = false;
    pSettings->startThreshold = 0;
}

BERR_Code BAPE_Playback_Start(
    BAPE_PlaybackHandle hPlayback,
    const BAPE_PlaybackStartSettings *pSettings
    )
{
    BERR_Code errCode;
    unsigned numBuffers;
    unsigned sampleRate;
    bool hbr=false;
    BAPE_PathNode *pNode;
    unsigned numFound;
    bool mixedWithDsp = false;

    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);
    BDBG_ASSERT(NULL != pSettings);

    /* If we're already running, return error */
    if ( hPlayback->running )
    {
        BDBG_ERR(("Playback %p (%d) is already running.  Can't start.", (void *)hPlayback, hPlayback->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( hPlayback->suspended )
    {
        BDBG_ERR(("Playback %p (%d) is suspended.  Can't start.", (void *)hPlayback, hPlayback->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Sanity Checks */
    numBuffers = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&hPlayback->node.connectors[0].format);
    if ( !hPlayback->interleaved )
    {
        numBuffers *= 2;
    }
    if ( numBuffers > hPlayback->numBuffers )
    {
        BDBG_ERR(("%u buffers are required for the current data format but only %u have been allocated", numBuffers, hPlayback->numBuffers));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( 0 == pSettings->sampleRate )
    {
        sampleRate = hPlayback->settings.sampleRate;
        if ( 0 == sampleRate )
        {
            BDBG_ERR(("To use a variable sample rate, you must specify a valid rate in BAPE_PlaybackSettings.sampleRate."));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    else
    {
        sampleRate = pSettings->sampleRate;
    }

    switch ( pSettings->bitsPerSample )
    {
    case 8:
    case 24:
    case 32:
        if ( hPlayback->settings.compressedData )
        {
            BDBG_ERR(("Compressed Data must be 16-bit samples for IEC-61937 or 64-bit samples for HBR"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        /* Fall through */
    case 16:
        break;
    case 16*4:
        if ( hPlayback->settings.compressedData )
        {
            hbr = true;
        }
        break;
    default:
        BDBG_ERR(("Unsupported bits per sample.  Only 8/16/32 are supported."));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BDBG_MSG(("Playback %p (%d) Starting", (void *)hPlayback, hPlayback->index));

    /* Save start settings */
    hPlayback->startSettings = *pSettings;

    /* Update format if required */
    if ( hPlayback->settings.compressedData )
    {
        BAPE_FMT_Descriptor format;

        BAPE_Connector_P_GetFormat(&hPlayback->node.connectors[0], &format);
        if ( hbr )
        {
            format.type = BAPE_DataType_eIec61937x16;
            sampleRate = 192000;
        }
        else if ( sampleRate == 176400 || sampleRate == 192000 )
        {
            format.type = BAPE_DataType_eIec61937x4;
        }
        else if ( sampleRate == 32000 || sampleRate == 44100 || sampleRate == 48000 )
        {
            format.type = BAPE_DataType_eIec61937;
        }
        else
        {
            BDBG_ERR(("Compressed data is only supported at sample rates of 192, 176.4, 48, 44.1, and 32 kHz."));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        errCode = BAPE_Connector_P_SetFormat(&hPlayback->node.connectors[0], &format);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto err_set_format;
        }
    }

    /* Update data format and sample rate in connector */
    BKNI_EnterCriticalSection();
    BAPE_Connector_P_SetSampleRate_isr(&hPlayback->node.connectors[0], sampleRate);
    BKNI_LeaveCriticalSection();

    /* Build Paths  */
    errCode = BAPE_PathNode_P_AcquirePathResources(&hPlayback->node);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_acquire_resources;
    }

    /* Prepare network to start */
    errCode = BAPE_PathNode_P_ConfigurePathResources(&hPlayback->node);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_configure_resources;
    }

    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
    {
        if ( NULL == hPlayback->pMaster )
        {
            BDBG_ERR(("No outputs have been connected to this playback."));
            goto err_no_master;
        }
    }

    /* Install the interrupt handler */
    if ( hPlayback->interrupts.watermark.pCallback_isr )
    {
        if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
        {
            errCode = BAPE_SfifoGroup_P_SetFreemarkInterrupt(hPlayback->pMaster->sfifoGroup,
                                                             hPlayback->interrupts.watermark.pCallback_isr,
                                                             hPlayback->interrupts.watermark.pParam1,
                                                             hPlayback->interrupts.watermark.param2);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto err_watermark;
            }
        }
        else if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
        {
            if ( hPlayback->bufferGroupHandle )
            {
                BAPE_BufferGroupInterruptHandlers interrupts;

                BAPE_BufferGroup_GetInterruptHandlers(hPlayback->bufferGroupHandle, &interrupts);
                interrupts.freeAvailable.pCallback_isr = hPlayback->interrupts.watermark.pCallback_isr;
                interrupts.freeAvailable.pParam1 = hPlayback->interrupts.watermark.pParam1;
                interrupts.freeAvailable.param2 = hPlayback->interrupts.watermark.param2;
                errCode = BAPE_BufferGroup_SetInterruptHandlers(hPlayback->bufferGroupHandle, &interrupts);
                if ( errCode )
                {
                    return BERR_TRACE(errCode);
                }
            }
        }
    }

    #if BDBG_DEBUG_BUILD
        BAPE_Mixer_P_PrintMixers(hPlayback->hApe);
    #endif

    BAPE_PathNode_P_FindConsumersBySubtype_isrsafe(&hPlayback->node, BAPE_PathNodeType_eMixer, BAPE_MixerType_eDsp, 1, &numFound, &pNode);
    mixedWithDsp = (numFound > 0) ? true : false;
    if (mixedWithDsp)
    {
        if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
        {
            BAPE_PathConnection *pLink=NULL;

            for ( pLink = BLST_SQ_FIRST(&hPlayback->node.connectors[0].connectionList);
                  NULL != pLink;
                  pLink = BLST_SQ_NEXT(pLink, downstreamNode) )
            {
                if ( pLink->srcGroup )
                {
                    BKNI_EnterCriticalSection();
                    BAPE_SrcGroup_P_SetSampleRate_isr(pLink->srcGroup, sampleRate, 48000);
                    BKNI_LeaveCriticalSection();
                }
            }
        }
        else if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
        {
            /* TBD7211 - Do we need to explicitly notify the DSP mixer?? */
        }
    }

    /* Start */
    errCode = BAPE_PathNode_P_StartPaths(&hPlayback->node);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_start_paths;
    }

    #if BDBG_DEBUG_BUILD
        BAPE_Mixer_P_PrintDownstreamNodes(&hPlayback->node);
    #endif

    /* Success */
    BDBG_MSG(("Playback %p (%d) Start Successful", (void *)hPlayback, hPlayback->index));
    hPlayback->running = true;
    errCode = BERR_TRACE(BAPE_Playback_P_ReArmWatermark(hPlayback));
    if ( errCode )
    {
        BDBG_WRN(("Unable to arm playback interrupts"));
    }
    return BERR_SUCCESS;

err_start_paths:
    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
    {
        /* Remove the interrupt handler */
        (void)BAPE_SfifoGroup_P_SetFreemarkInterrupt(hPlayback->pMaster->sfifoGroup, NULL, NULL, 0);
    }
    else if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
    {
        if ( hPlayback->bufferGroupHandle )
        {
            BAPE_BufferGroupInterruptHandlers interrupts;

            BAPE_BufferGroup_GetInterruptHandlers(hPlayback->bufferGroupHandle, &interrupts);
            interrupts.freeAvailable.pCallback_isr = NULL;
            interrupts.freeAvailable.pParam1 = NULL;
            interrupts.freeAvailable.param2 = 0;
            (void)BAPE_BufferGroup_SetInterruptHandlers(hPlayback->bufferGroupHandle, &interrupts);
        }
    }
err_watermark:
err_no_master:
    BAPE_PathNode_P_ReleasePathResources(&hPlayback->node);
err_configure_resources:
err_acquire_resources:
err_set_format:
    return errCode;
}

void BAPE_Playback_Stop(
    BAPE_PlaybackHandle hPlayback
    )
{
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    if ( !hPlayback->running && !hPlayback->suspended)
    {
        BDBG_MSG(("Playback %p (%d) already stopped.", (void *)hPlayback, hPlayback->index));
        return;
    }

    if (hPlayback->running)
    {
        /* Stop Paths Downstream */
        BAPE_PathNode_P_StopPaths(&hPlayback->node);

        /* Remove the interrupt handler */
        if ( hPlayback->pMaster && hPlayback->interrupts.watermark.pCallback_isr )
        {
            if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
            {
                (void)BAPE_SfifoGroup_P_SetFreemarkInterrupt(hPlayback->pMaster->sfifoGroup, NULL, NULL, 0);
            }
            else if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
            {
                if ( hPlayback->bufferGroupHandle )
                {
                    BAPE_BufferGroupInterruptHandlers interrupts;

                    BAPE_BufferGroup_GetInterruptHandlers(hPlayback->bufferGroupHandle, &interrupts);
                    interrupts.freeAvailable.pCallback_isr = NULL;
                    interrupts.freeAvailable.pParam1 = NULL;
                    interrupts.freeAvailable.param2 = 0;
                    (void)BAPE_BufferGroup_SetInterruptHandlers(hPlayback->bufferGroupHandle, &interrupts);
                }
            }
        }
    }

    /* Clear the started flag and flush the buffer */
    hPlayback->running = false;
    hPlayback->suspended = false;
    hPlayback->bufferDepth = 0;
    hPlayback->pMaster = NULL;
}


BERR_Code BAPE_Playback_Resume(
    BAPE_PlaybackHandle hPlayback
    )
{
    BAPE_PlaybackStartSettings startSettings;
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    /* If we're already running, return error */
    if ( hPlayback->running )
    {
        BDBG_ERR(("Playback %p (%d) is already running.  Can't resume.", (void *)hPlayback, hPlayback->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( !hPlayback->suspended )
    {
        BDBG_ERR(("Playback %p (%d) is not suspended.  Can't resume.", (void *)hPlayback, hPlayback->index));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BKNI_Memcpy(&startSettings, &hPlayback->startSettings, sizeof(BAPE_PlaybackStartSettings));
    hPlayback->suspended = false;
    return BAPE_Playback_Start(hPlayback, &startSettings);
}

void BAPE_Playback_Suspend(
    BAPE_PlaybackHandle hPlayback
    )
{
    BERR_Code errCode;
    unsigned bufferDepth;
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    BDBG_MSG(("Playback %p (%d) Suspend", (void *)hPlayback, hPlayback->index));
    if ( !hPlayback->running )
    {
        BDBG_MSG(("Playback %p (%d) already stopped.", (void *)hPlayback, hPlayback->index));
        return;
    }
    hPlayback->running = false;
    hPlayback->suspending = true;
    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
    {
        BKNI_EnterCriticalSection();
        BAPE_SfifoGroup_P_Halt_isr(hPlayback->pMaster->sfifoGroup);
        BKNI_LeaveCriticalSection();

        errCode = BAPE_SfifoGroup_P_GetQueuedBytes(hPlayback->pMaster->sfifoGroup, &bufferDepth);
        if ( errCode )
        {
            BDBG_ERR(("Unable to suspend, stopping playback"));
            BAPE_Playback_Stop(hPlayback);
            hPlayback->suspending = false;
            return;
        }
        hPlayback->bufferDepth = bufferDepth;
        if ( bufferDepth > 0 )
        {
            BMMA_Block_Handle tmpBlock;
            BMMA_DeviceOffset baseAddr, rdAddr;
            unsigned length;
            unsigned chPairs, i;
            void *pBuffer, *pSrcCached, *pDstCached;
            BMMA_DeviceOffset preWrap;
            BMMA_DeviceOffset postWrap;
            BAPE_SfifoGroupSettings sfifoSettings;

            BAPE_SfifoGroup_P_GetSettings(hPlayback->pMaster->sfifoGroup, &sfifoSettings);
            length = hPlayback->bufferSize;
            tmpBlock = BMMA_Alloc(hPlayback->hHeap, length, 32, NULL);
            if ( NULL == tmpBlock )
            {
                BDBG_ERR(("Unable to suspend, stopping playback"));
                BAPE_Playback_Stop(hPlayback);
                hPlayback->suspending = false;
                return;
            }
            pBuffer = BMMA_Lock(tmpBlock);
            if ( NULL == pBuffer )
            {
                BDBG_ERR(("Unable to suspend, stopping playback"));
                BMMA_Free(tmpBlock);
                tmpBlock = NULL;
                BAPE_Playback_Stop(hPlayback);
                hPlayback->suspending = false;
                return;
            }
            pDstCached = pBuffer;

            /* copy the data*/
            chPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&hPlayback->node.connectors[0].format);
            /* Setup Buffers */
            for ( i = 0; i < chPairs; i++)
            {
                baseAddr = sfifoSettings.bufferInfo[i*2].base;
                BAPE_SfifoGroup_P_GetReadAddress(hPlayback->pMaster->sfifoGroup, i*2, 0, &rdAddr);

                pSrcCached = hPlayback->pBuffer[i*2];

                if (rdAddr+bufferDepth <= baseAddr+length) /* there is no wrap */
                {
                    BAPE_FLUSHCACHE_ISRSAFE(hPlayback->block[i*2], (uint8_t*)pSrcCached+(rdAddr-baseAddr), bufferDepth);
                    BKNI_Memcpy(pDstCached, (uint8_t*)pSrcCached+(rdAddr-baseAddr), bufferDepth);
                }
                else /* there is wrap */
                {
                    preWrap = baseAddr + length - rdAddr;
                    postWrap = bufferDepth - preWrap;

                    BAPE_FLUSHCACHE_ISRSAFE(hPlayback->block[i*2], (uint8_t*)pSrcCached+(rdAddr-baseAddr), preWrap);
                    BKNI_Memcpy(pDstCached, (uint8_t*)pSrcCached+(rdAddr-baseAddr), preWrap);
                    BAPE_FLUSHCACHE_ISRSAFE(hPlayback->block[i*2], pSrcCached, postWrap);
                    BKNI_Memcpy((uint8_t*)pDstCached+preWrap, pSrcCached, postWrap);
                }
                BKNI_Memcpy(pSrcCached, pDstCached, bufferDepth);
                BAPE_FLUSHCACHE_ISRSAFE(hPlayback->block[i*2], pSrcCached, bufferDepth);

                if (!sfifoSettings.interleaveData)
                {
                    baseAddr = sfifoSettings.bufferInfo[i*2+1].base;
                    BAPE_SfifoGroup_P_GetReadAddress(hPlayback->pMaster->sfifoGroup, i*2, 1, &rdAddr);

                    pSrcCached = hPlayback->pBuffer[i*2+1];

                    if (rdAddr+bufferDepth <= baseAddr+length) /* there is no wrap */
                    {
                        BAPE_FLUSHCACHE_ISRSAFE(hPlayback->block[i*2+1], (uint8_t*)pSrcCached+(rdAddr-baseAddr), bufferDepth);
                        BKNI_Memcpy(pDstCached, (uint8_t*)pSrcCached+(rdAddr-baseAddr), bufferDepth);
                    }
                    else /* there is wrap */
                    {
                        preWrap = baseAddr + length - rdAddr;
                        postWrap = bufferDepth - preWrap;

                        BAPE_FLUSHCACHE_ISRSAFE(hPlayback->block[i*2+1], (uint8_t*)pSrcCached+(rdAddr-baseAddr), preWrap);
                        BKNI_Memcpy(pDstCached, (uint8_t*)pSrcCached+(rdAddr-baseAddr), preWrap);
                        BAPE_FLUSHCACHE_ISRSAFE(hPlayback->block[i*2+1], pSrcCached, postWrap);
                        BKNI_Memcpy((uint8_t*)pDstCached+preWrap, pSrcCached, postWrap);
                    }
                    BKNI_Memcpy(pSrcCached, pDstCached, bufferDepth);
                    BAPE_FLUSHCACHE_ISRSAFE(hPlayback->block[i*2+1], pSrcCached, bufferDepth);
                }
            }
            BMMA_Unlock(tmpBlock, pBuffer);
            pBuffer = NULL;
            BMMA_Free(tmpBlock);
            tmpBlock = NULL;
        }
    }
    else if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
    {
        errCode = BAPE_BufferGroup_Enable(hPlayback->bufferGroupHandle, false);
        if ( errCode )
        {
            BDBG_WRN(("Failed to disable buffer group"));
            BERR_TRACE(errCode);
        }
    }

    /* Stop Paths Downstream */
    BAPE_PathNode_P_StopPaths(&hPlayback->node);

    /* Remove the interrupt handler */
    if ( hPlayback->interrupts.watermark.pCallback_isr )
    {
        if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
        {
            (void)BAPE_SfifoGroup_P_SetFreemarkInterrupt(hPlayback->pMaster->sfifoGroup, NULL, NULL, 0);
        }
        else if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
        {
            if ( hPlayback->bufferGroupHandle )
            {
                BAPE_BufferGroupInterruptHandlers interrupts;

                BAPE_BufferGroup_GetInterruptHandlers(hPlayback->bufferGroupHandle, &interrupts);
                interrupts.freeAvailable.pCallback_isr = NULL;
                interrupts.freeAvailable.pParam1 = NULL;
                interrupts.freeAvailable.param2 = 0;
                (void)BAPE_BufferGroup_SetInterruptHandlers(hPlayback->bufferGroupHandle, &interrupts);
            }
        }
    }
    hPlayback->suspended = true;
    hPlayback->pMaster = NULL;
    hPlayback->suspending = false;
}

void BAPE_Playback_Flush(
    BAPE_PlaybackHandle hPlayback
    )
{
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    if ( hPlayback->running )
    {
        BDBG_ASSERT(NULL != hPlayback->pMaster);
        if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
        {
            BAPE_SfifoGroup_P_Flush(hPlayback->pMaster->sfifoGroup);
        }
    }

    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
    {
        BAPE_BufferGroup_Flush(hPlayback->bufferGroupHandle);
    }
    hPlayback->bufferDepth = 0;
}

BERR_Code BAPE_Playback_GetBuffer(
    BAPE_PlaybackHandle hPlayback,
    BAPE_BufferDescriptor *pBuffers      /* [out] */
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned i, numChannelPairs;

    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);
    BDBG_ASSERT(NULL != pBuffers);

    BDBG_MSG(("Playback %p (%d) GetBuffer", (void *)hPlayback, hPlayback->index));

    BKNI_Memset(pBuffers, 0, sizeof(BAPE_BufferDescriptor));

    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
    {
        if ( (hPlayback->running) && !hPlayback->startSettings.loopEnabled )
        {
            BDBG_ASSERT(NULL != hPlayback->pMaster);
            errCode = BAPE_SfifoGroup_P_GetBuffer(hPlayback->pMaster->sfifoGroup, pBuffers);
            if ( errCode != BERR_SUCCESS ) { return BERR_TRACE(errCode); }
        }
        else
        {
            /* TODO: hPlayback non-interleaved and multichannel */
            numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&hPlayback->node.connectors[0].format);
            pBuffers->numBuffers = 2*numChannelPairs;
            pBuffers->interleaved = hPlayback->interleaved;
            if ( !hPlayback->suspending ) /* As long as we aren't in the middle of suspending */
            {
                if ( (hPlayback->bufferDepth + BAPE_CHIP_SFIFO_PADDING) > hPlayback->bufferSize ) {
                    BDBG_ERR(("BufferDepth + SFIFO padding is greater then bufferSize"));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }

                pBuffers->bufferSize = hPlayback->bufferSize - hPlayback->bufferDepth - BAPE_CHIP_SFIFO_PADDING; /* sfifo padding for master/slave configurations */
                if ( pBuffers->bufferSize )
                {
                    if ( pBuffers->interleaved )
                    {
                        for ( i = 0; i < numChannelPairs; i++)
                        {
                            pBuffers->buffers[i*2].pBuffer = (void *)((uint8_t*)hPlayback->pBuffer[i] + hPlayback->bufferDepth);
                            pBuffers->buffers[i*2].block = hPlayback->block[i];
                        }
                    }
                    else
                    {
                        for ( i = 0; i < numChannelPairs; i++)
                        {
                            /* wmp - is this correct? why are we using the same source buffer across two channels??  */
                            pBuffers->buffers[i*2].pBuffer = (void *)((uint8_t*)hPlayback->pBuffer[i] + hPlayback->bufferDepth);
                            pBuffers->buffers[i*2 + 1].pBuffer = (void *)((uint8_t*)hPlayback->pBuffer[i] + hPlayback->bufferDepth);
                            pBuffers->buffers[i*2].block = hPlayback->block[i];
                            pBuffers->buffers[i*2 + 1].block = hPlayback->block[i];
                        }
                    }
                }
            }
        }
    }
    else if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
    {
        if ( hPlayback->running || hPlayback->suspended )
        {
            errCode = BAPE_BufferGroup_GetWriteBuffers(hPlayback->bufferGroupHandle, pBuffers);
            if ( errCode )
            {
                BKNI_Memset(pBuffers, 0, sizeof(BAPE_BufferDescriptor));
                return BERR_TRACE(errCode);
            }
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_Playback_CommitData(
    BAPE_PlaybackHandle hPlayback,
    unsigned numBytes                   /* Number of bytes written into the buffer */
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    BDBG_MSG(("Playback %p (%d) CommitData", (void *)hPlayback, hPlayback->index));

    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
    {
        /* todo what should happen if we are suspending? */
        if (hPlayback->running)
        {
            BDBG_ASSERT(NULL != hPlayback->pMaster);
            errCode = BAPE_SfifoGroup_P_CommitData(hPlayback->pMaster->sfifoGroup, numBytes);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
        else
        {
            if ( (numBytes + hPlayback->bufferDepth) > (hPlayback->bufferSize - BAPE_CHIP_SFIFO_PADDING) ) /* sfifo padding for master/slave configurations */
            {
                BDBG_ERR(("Invalid number of bytes passed."));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            hPlayback->bufferDepth += numBytes;
        }

        if ( hPlayback->pMaster )
        {
            BAPE_SfifoGroup_P_RearmFreemarkInterrupt(hPlayback->pMaster->sfifoGroup);
        }
    }
    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
    {
        /* need to have valid start settings - if stopped or never started,
           num channel pairs, interleaved etc could change*/
        if ( hPlayback->running || hPlayback->suspended )
        {
            errCode = BAPE_BufferGroup_WriteComplete(hPlayback->bufferGroupHandle, (size_t)numBytes);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
        else
        {
            return BERR_NOT_AVAILABLE;
        }
    }

    return BERR_SUCCESS;
}

void BAPE_Playback_GetStatus(
    BAPE_PlaybackHandle hPlayback,
    BAPE_PlaybackStatus *pStatus    /* [out] */
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);
    BDBG_ASSERT(NULL != pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
    {
        pStatus->fifoSize = hPlayback->bufferSize - BAPE_CHIP_SFIFO_PADDING; /* sfifo padding for master/slave configurations */

        if ( !hPlayback->running )
        {
            pStatus->queuedBytes = hPlayback->bufferDepth;
            return;
        }

        /* If running, query the SFIFO */
        BDBG_ASSERT(NULL != hPlayback->pMaster);
        errCode = BAPE_SfifoGroup_P_GetQueuedBytes(hPlayback->pMaster->sfifoGroup, &pStatus->queuedBytes);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            return;
        }
    }
    else if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
    {
        pStatus->fifoSize = hPlayback->bufferSize;
        pStatus->queuedBytes = BAPE_BufferGroup_GetBufferDepth(hPlayback->bufferGroupHandle);
    }
}

void BAPE_Playback_GetConnector(
    BAPE_PlaybackHandle hPlayback,
    BAPE_Connector *pConnector /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);
    *pConnector = &hPlayback->node.connectors[0];
}

void BAPE_Playback_GetInterruptHandlers(
    BAPE_PlaybackHandle hPlayback,
    BAPE_PlaybackInterruptHandlers *pInterrupts     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);
    BDBG_ASSERT(NULL != pInterrupts);
    *pInterrupts = hPlayback->interrupts;
}

BERR_Code BAPE_Playback_SetInterruptHandlers(
    BAPE_PlaybackHandle hPlayback,
    const BAPE_PlaybackInterruptHandlers *pInterrupts
    )
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);
    BDBG_ASSERT(NULL != pInterrupts);

    /* store new settings */
    hPlayback->interrupts = *pInterrupts;

    if ( hPlayback->running )
    {
        if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
        {
            errCode = BAPE_SfifoGroup_P_SetFreemarkInterrupt(hPlayback->pMaster->sfifoGroup,
                                                             pInterrupts->watermark.pCallback_isr,
                                                             pInterrupts->watermark.pParam1,
                                                             pInterrupts->watermark.param2);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
        else if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
        {
            if ( hPlayback->bufferGroupHandle )
            {
                BAPE_BufferGroupInterruptHandlers interrupts;

                BAPE_BufferGroup_GetInterruptHandlers(hPlayback->bufferGroupHandle, &interrupts);
                interrupts.freeAvailable.pCallback_isr = pInterrupts->watermark.pCallback_isr;
                interrupts.freeAvailable.pParam1 = pInterrupts->watermark.pParam1;
                interrupts.freeAvailable.param2 = pInterrupts->watermark.param2;
                errCode = BAPE_BufferGroup_SetInterruptHandlers(hPlayback->bufferGroupHandle, &interrupts);
                if ( errCode )
                {
                    return BERR_TRACE(errCode);
                }
            }
        }
    }

    return BERR_TRACE(BAPE_Playback_P_ReArmWatermark(hPlayback));
}

#if BAPE_PLAYBACK_USE_TIMER_BASED_INTERRUPTS
static void BAPE_P_CheckPlaybackStatus_isr (void *pParam1, int param2)
{
    BAPE_PlaybackHandle handle = pParam1;
    unsigned free = 0;

    BDBG_OBJECT_ASSERT(handle, BAPE_Playback);

    if ( handle->bufferGroupHandle )
    {
        free = BAPE_BufferGroup_GetBufferFree_isr(handle->bufferGroupHandle);
    }

    if ( free > handle->threshold )
    {
        if (handle->interrupts.watermark.pCallback_isr)
        {
            /*BDBG_ERR(("%s Free threshold reached", BSTD_FUNCTION));*/
            handle->interrupts.watermark.pCallback_isr(handle->interrupts.watermark.pParam1, handle->interrupts.watermark.param2);
        }
    }

    #if BAPE_PLAYBACK_USE_TIMER_BASED_INTERRUPTS
    if (handle->interruptTimer)
    {
        BERR_TRACE(BTMR_StartTimer_isr(handle->interruptTimer, (unsigned)param2));
    }
    #endif
}
#endif

static BERR_Code BAPE_Playback_P_ReArmWatermark(BAPE_PlaybackHandle hPlayback)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    if ( hPlayback->running )
    {
        if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb && hPlayback->pMaster->sfifoGroup )
        {
            BDBG_ASSERT(NULL != hPlayback->pMaster);
            BAPE_SfifoGroup_P_RearmFreemarkInterrupt(hPlayback->pMaster->sfifoGroup);
        }
        else if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
        {
            #if BAPE_PLAYBACK_USE_TIMER_BASED_INTERRUPTS
            BTMR_TimerSettings timerSettings;

            if ( hPlayback->interruptTimer )
            {
                BKNI_EnterCriticalSection();
                BTMR_StopTimer_isr(hPlayback->interruptTimer);
                BKNI_LeaveCriticalSection();
            }

            if ( !hPlayback->interruptTimer )
            {
                errCode = BERR_TRACE(BTMR_GetDefaultTimerSettings(&timerSettings));
                if ( errCode )
                {
                    BDBG_WRN(("Get default timer settings failed"));
                }
                timerSettings.type = BTMR_Type_eCountDown;
                timerSettings.cb_isr = BAPE_P_CheckPlaybackStatus_isr;
                timerSettings.pParm1 = hPlayback;
                timerSettings.parm2 = BAPE_PLAYBACK_INTERRUPT_TIMER_DURATION;
                errCode |= BERR_TRACE(BTMR_CreateTimer(hPlayback->hApe->tmrHandle, &hPlayback->interruptTimer, &timerSettings));
                if ( errCode )
                {
                    BDBG_WRN(("Unable to create playback interrupt timer"));
                }
            }

            if ( hPlayback->interruptTimer )
            {
                errCode |= BERR_TRACE(BTMR_StartTimer(hPlayback->interruptTimer, BAPE_PLAYBACK_INTERRUPT_TIMER_DURATION));
                if ( errCode )
                {
                    BDBG_WRN(("Unable to start playback interrupt timer"));
                }
            }
            #endif
        }
    }

    return errCode;
}

static BERR_Code BAPE_Playback_P_ConfigPathToOutput(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    BAPE_PlaybackHandle hPlayback;
    unsigned chPairs;
    unsigned i;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_OBJECT_ASSERT(pConnection->pSource, BAPE_PathConnector);
    BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);

    hPlayback = pNode->pHandle;
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    chPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&hPlayback->node.connectors[0].format);

    pConnection->bufferGroupHandle = NULL;
    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
    {
        BAPE_SfifoGroupSettings sfifoSettings;

        /* Dropping into the FMM - Configure the SFIFO(s) */
        if ( NULL == hPlayback->pMaster )
        {
            hPlayback->pMaster = pConnection;
        }

        BAPE_SfifoGroup_P_GetSettings(pConnection->sfifoGroup, &sfifoSettings);
        sfifoSettings.highPriority = (hPlayback->node.connectors[0].format.sampleRate >= 96000)?1:0;
        if ( hPlayback->settings.compressedData )
        {
            sfifoSettings.dataWidth = 16;
        }
        else
        {
            sfifoSettings.dataWidth = hPlayback->startSettings.bitsPerSample;
        }
        sfifoSettings.reverseEndian = hPlayback->startSettings.reverseEndian;
        sfifoSettings.sampleRepeatEnabled = BAPE_FMT_P_RampingValid_isrsafe(&hPlayback->node.connectors[0].format);
        sfifoSettings.loopAround = hPlayback->startSettings.loopEnabled;
        sfifoSettings.wrpointEnabled = false;
        if ( hPlayback->startSettings.startThreshold > 0 )
        {
            if ( hPlayback->startSettings.loopEnabled )
            {
                BDBG_WRN(("Playback start threshold is not compatible with the loop setting.  Disabling startThreshold."));
            }
            else
            {
                sfifoSettings.wrpointEnabled = true;
            }
        }
        /* TODO: Add support for non-interleaved */
        sfifoSettings.interleaveData = hPlayback->interleaved;
        sfifoSettings.stereoData = hPlayback->startSettings.isStereo;
        sfifoSettings.signedData = hPlayback->startSettings.isSigned;
        if ( pConnection != hPlayback->pMaster )
        {
            BDBG_MSG(("Linking playback source channel group %p to master %p", (void *)pConnection->sfifoGroup, (void *)hPlayback->pMaster->sfifoGroup));
            sfifoSettings.master = hPlayback->pMaster->sfifoGroup;
        }
        else
        {
            sfifoSettings.master = NULL;
        }

        /* Setup Buffers */
        for ( i = 0; i < chPairs; i++)
        {
            if ( sfifoSettings.interleaveData)
            {
                sfifoSettings.bufferInfo[i].block = hPlayback->block[i*2];
                sfifoSettings.bufferInfo[i].pBuffer = hPlayback->pBuffer[i*2];
                sfifoSettings.bufferInfo[i].base = hPlayback->offset[i*2];
                sfifoSettings.bufferInfo[i].writeOffset = hPlayback->bufferDepth;
                sfifoSettings.bufferInfo[i].length = hPlayback->bufferSize;
                sfifoSettings.bufferInfo[i].watermark = hPlayback->threshold;
                if ( hPlayback->startSettings.startThreshold > hPlayback->bufferSize )
                {
                    BDBG_WRN(("Start threshold is too large.  Starting with buffer full."));
                    sfifoSettings.bufferInfo[i].wrpoint = hPlayback->offset[i*2] + hPlayback->bufferSize - 1;
                }
                else if ( hPlayback->startSettings.startThreshold > 0 )
                {
                    sfifoSettings.bufferInfo[i].wrpoint = hPlayback->offset[i*2] + hPlayback->startSettings.startThreshold-1;
                }
                else
                {
                    sfifoSettings.bufferInfo[i].wrpoint = 0;
                }
            }
            else
            {
                sfifoSettings.bufferInfo[i*2].base = hPlayback->offset[i*2];
                sfifoSettings.bufferInfo[i*2].writeOffset = hPlayback->bufferDepth;
                sfifoSettings.bufferInfo[i*2].length = hPlayback->bufferSize;
                sfifoSettings.bufferInfo[i*2].watermark = hPlayback->threshold;
                if ( hPlayback->startSettings.startThreshold > hPlayback->bufferSize )
                {
                    BDBG_WRN(("Start threshold is too large.  Starting with buffer full."));
                    sfifoSettings.bufferInfo[i*2].wrpoint = hPlayback->offset[i*2] + hPlayback->bufferSize - 1;
                }
                else if ( hPlayback->startSettings.startThreshold > 0 )
                {
                    sfifoSettings.bufferInfo[i*2].wrpoint = hPlayback->offset[i*2] + hPlayback->startSettings.startThreshold-1;
                }
                else
                {
                    sfifoSettings.bufferInfo[i*2].wrpoint = 0;
                }

                sfifoSettings.bufferInfo[i*2 + 1].block = hPlayback->block[(i*2) + 1];
                sfifoSettings.bufferInfo[i*2 + 1].pBuffer = hPlayback->pBuffer[(i*2) + 1];
                sfifoSettings.bufferInfo[i*2 + 1].base = hPlayback->offset[(i*2) + 1];
                sfifoSettings.bufferInfo[i*2 + 1].writeOffset = hPlayback->bufferDepth;
                sfifoSettings.bufferInfo[i*2 + 1].length = hPlayback->bufferSize;
                sfifoSettings.bufferInfo[i*2 + 1].watermark = hPlayback->threshold;
                if ( hPlayback->startSettings.startThreshold > hPlayback->bufferSize )
                {
                    BDBG_WRN(("Start threshold is too large.  Starting with buffer full."));
                    sfifoSettings.bufferInfo[i*2 + 1].wrpoint = hPlayback->offset[(i*2) + 1] + hPlayback->bufferSize - 1;
                }
                else if ( hPlayback->startSettings.startThreshold > 0 )
                {
                    sfifoSettings.bufferInfo[i*2 + 1].wrpoint = hPlayback->offset[(i*2) + 1] + hPlayback->startSettings.startThreshold-1;
                }
                else
                {
                    sfifoSettings.bufferInfo[i*2 + 1].wrpoint = 0;
                }
            }
        }
        BAPE_SfifoGroup_P_SetSettings(pConnection->sfifoGroup, &sfifoSettings);
    }
    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
    {
        /* non-destructive - can be called even if buffer group does not require changes */
        errCode = BAPE_Playback_P_CreateBufferGroup(hPlayback, hPlayback->settings.multichannelFormat);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        /* Configure buffer interfaces in Path Connection */
        /* TBD7211 - if we double buffer, we need to create another output buffer group and link it here */
        pConnection->bufferGroupHandle = hPlayback->bufferGroupHandle;
    }

    /* Other nodes don't require anything done here */

    return BERR_SUCCESS;
}

static BERR_Code BAPE_Playback_P_StartPathToOutput(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection)
{
    BAPE_PlaybackHandle hPlayback;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    hPlayback = pNode->pHandle;
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
    {
        if ( pConnection->sfifoGroup )
        {
            errCode = BAPE_SfifoGroup_P_Start(pConnection->sfifoGroup, false);
            if ( errCode )
            {
                return BERR_TRACE(errCode);
            }
        }
    }
    else if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
    {
        BAPE_BufferGroupFormat bgFormat;

        /* Set Format */
        bgFormat.bitsPerSample = hPlayback->startSettings.bitsPerSample;
        bgFormat.compressed = hPlayback->settings.compressedData;
        if ( hPlayback->interleaved && hPlayback->startSettings.bitsPerSample == 16 )
        {
            bgFormat.samplesPerDword = 2;
        }
        else
        {
            bgFormat.samplesPerDword = 1;
        }
        errCode = BAPE_BufferGroup_SetFormat(hPlayback->bufferGroupHandle, &bgFormat);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        /* Enable consumption */
        errCode = BAPE_BufferGroup_Enable(hPlayback->bufferGroupHandle, true);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }

    return BERR_SUCCESS;
}

static void BAPE_Playback_P_StopPathToOutput(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection)
{
    BAPE_PlaybackHandle hPlayback;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);

    hPlayback = pNode->pHandle;
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eRdb )
    {
        /* Stop slaves first */
        for ( pConnection = BLST_SQ_FIRST(&pNode->connectors[0].connectionList);
            NULL != pConnection;
            pConnection = BLST_SQ_NEXT(pConnection, downstreamNode) )
        {
            if ( pConnection != hPlayback->pMaster )
            {
                BAPE_SfifoGroup_P_Stop(pConnection->sfifoGroup);
            }
        }

        /* Stop master last */
        BAPE_SfifoGroup_P_Stop(hPlayback->pMaster->sfifoGroup);
    }
    else if ( hPlayback->bufferInterfaceType == BAPE_BufferInterfaceType_eDram )
    {
        /* TBD7211 - Handle master/slave - buffer group should really handle this */

        /* Disable consumption */
        errCode = BAPE_BufferGroup_Enable(hPlayback->bufferGroupHandle, false);
        if ( errCode )
        {
            BDBG_ERR(("Error disabling Buffer Group"));
            BERR_TRACE(errCode);
        }
        pConnection->bufferGroupHandle = NULL;
    }
}

#else /* stubs */

void BAPE_Playback_GetDefaultOpenSettings(
    BAPE_PlaybackOpenSettings *pSettings
    )
{
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_Playback_Open(
    BAPE_Handle hApe,
    unsigned index,
    const BAPE_PlaybackOpenSettings *pSettings,
    BAPE_PlaybackHandle *pHandle                    /* [out] */
    )
{
    BSTD_UNUSED(hApe);
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pHandle);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_Playback_Close(
    BAPE_PlaybackHandle hPlayback
    )
{
    BSTD_UNUSED(hPlayback);
}

void BAPE_Playback_GetSettings(
    BAPE_PlaybackHandle hPlayback,
    BAPE_PlaybackSettings *pSettings    /* [out] */
    )
{
    BSTD_UNUSED(hPlayback);
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_Playback_SetSettings(
    BAPE_PlaybackHandle hPlayback,
    const BAPE_PlaybackSettings *pSettings
    )
{
    BSTD_UNUSED(hPlayback);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_Playback_GetDefaultStartSettings(
    BAPE_PlaybackStartSettings *pSettings       /* [out] */
    )
{
    BSTD_UNUSED(pSettings);
}

BERR_Code BAPE_Playback_Start(
    BAPE_PlaybackHandle hPlayback,
    const BAPE_PlaybackStartSettings *pSettings
    )
{
    BSTD_UNUSED(hPlayback);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_Playback_Stop(
    BAPE_PlaybackHandle hPlayback
    )
{
    BSTD_UNUSED(hPlayback);
}

BERR_Code BAPE_Playback_Resume(
    BAPE_PlaybackHandle hPlayback
    )
{
    BSTD_UNUSED(hPlayback);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_Playback_Suspend(
    BAPE_PlaybackHandle hPlayback
    )
{
    BSTD_UNUSED(hPlayback);
}

void BAPE_Playback_Flush(
    BAPE_PlaybackHandle hPlayback
    )
{
    BSTD_UNUSED(hPlayback);
}

BERR_Code BAPE_Playback_GetBuffer(
    BAPE_PlaybackHandle hPlayback,
    BAPE_BufferDescriptor *pBuffers      /* [out] */
    )
{
    BSTD_UNUSED(hPlayback);
    BSTD_UNUSED(pBuffers);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

BERR_Code BAPE_Playback_CommitData(
    BAPE_PlaybackHandle hPlayback,
    unsigned numBytes                   /* Number of bytes written into the buffer */
    )
{
    BSTD_UNUSED(hPlayback);
    BSTD_UNUSED(numBytes);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BAPE_Playback_GetStatus(
    BAPE_PlaybackHandle hPlayback,
    BAPE_PlaybackStatus *pStatus    /* [out] */
    )
{
    BSTD_UNUSED(hPlayback);
    BSTD_UNUSED(pStatus);
}

void BAPE_Playback_GetConnector(
    BAPE_PlaybackHandle hPlayback,
    BAPE_Connector *pConnector /* [out] */
    )
{
    BSTD_UNUSED(hPlayback);
    BSTD_UNUSED(pConnector);
}

void BAPE_Playback_GetInterruptHandlers(
    BAPE_PlaybackHandle hPlayback,
    BAPE_PlaybackInterruptHandlers *pInterrupts     /* [out] */
    )
{
    BSTD_UNUSED(hPlayback);
    BSTD_UNUSED(pInterrupts);
}

BERR_Code BAPE_Playback_SetInterruptHandlers(
    BAPE_PlaybackHandle hPlayback,
    const BAPE_PlaybackInterruptHandlers *pInterrupts
    )
{
    BSTD_UNUSED(hPlayback);
    BSTD_UNUSED(pInterrupts);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

#endif
