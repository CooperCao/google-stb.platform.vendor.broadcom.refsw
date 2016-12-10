/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bape.h"
#include "bape_priv.h"
#include "bchp_aud_fmm_bf_ctrl.h"

BDBG_MODULE(bape_playback);

BDBG_OBJECT_ID(BAPE_Playback);

static void BAPE_Playback_P_ReArmWatermark(BAPE_PlaybackHandle hPlayback);
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

    for (i = 0; i < pSettings->numBuffers; i++) 
    {
        hPlayback->pBuffer[i] = BMEM_AllocAligned(hPlayback->hHeap, pSettings->bufferSize, 5, 0);
        if ( NULL == hPlayback->pBuffer[i] )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto err_buffer_alloc;
        }
    }

    hPlayback->bufferSize = pSettings->bufferSize;
    if ( pSettings->watermarkThreshold >= pSettings->bufferSize )
    {
        BDBG_WRN(("playback watermark threshold %u is larger than the buffer size %u.  Reducing to %u.", pSettings->watermarkThreshold, pSettings->bufferSize, pSettings->bufferSize / 2));
        hPlayback->threshold = pSettings->bufferSize / 2;
    }
    else
    {
        hPlayback->threshold = pSettings->watermarkThreshold;
    }
    hPlayback->index = index;
    hPlayback->numBuffers = pSettings->numBuffers;
    hPlayback->settings.multichannelFormat = BAPE_MultichannelFormat_e2_0;
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
        if ( NULL != hPlayback->pBuffer[i] )
        {
            BMEM_Free(hPlayback->hHeap, hPlayback->pBuffer[i]);
        }
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
    if ( hPlayback->running )
    {
        BDBG_WRN(("Stopping playback %p (%d) on close", (void *)hPlayback, hPlayback->index));
        BAPE_Playback_Stop(hPlayback);
    }

    /* Disconnect from all mixers, post-processors, and groups */
    BAPE_Connector_P_RemoveAllConnections(&hPlayback->node.connectors[0]);

    for (i = 0; i < hPlayback->numBuffers; i++)
    {
        if (hPlayback->pBuffer[i]) 
        {
            BMEM_Free(hPlayback->hHeap, hPlayback->pBuffer[i]);
        }
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
    pSettings->interleaved = true;
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
    bool mixedWithDsp;

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
    if ( !pSettings->interleaved )
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

    if ( NULL == hPlayback->pMaster )
    {
        BDBG_ERR(("No outputs have been connected to this input capture."));
        goto err_no_master;
    }

    /* Install the interrupt handler */
    if ( hPlayback->interrupts.watermark.pCallback_isr )
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

    #if BDBG_DEBUG_BUILD
        BAPE_Mixer_P_PrintMixers(hPlayback->hApe);
    #endif

    BAPE_PathNode_P_FindConsumersBySubtype(&hPlayback->node, BAPE_PathNodeType_eMixer, BAPE_MixerType_eDsp, 1, &numFound, &pNode);
    mixedWithDsp = (numFound > 0) ? true : false;
    if (mixedWithDsp)
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
    BAPE_Playback_P_ReArmWatermark(hPlayback);
    return BERR_SUCCESS;

err_start_paths:
    /* Remove the interrupt handler */
    (void)BAPE_SfifoGroup_P_SetFreemarkInterrupt(hPlayback->pMaster->sfifoGroup, NULL, NULL, 0);
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
            (void)BAPE_SfifoGroup_P_SetFreemarkInterrupt(hPlayback->pMaster->sfifoGroup, NULL, NULL, 0);
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
    BKNI_EnterCriticalSection();
    /* Start */
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
        uint32_t baseAddr, rdAddr, length;
        unsigned chPairs, i;
        void *pBuffer, *pSrcCached, *pDstCached;
        unsigned preWrap;
        unsigned postWrap;
        BAPE_SfifoGroupSettings sfifoSettings;

        BAPE_SfifoGroup_P_GetSettings(hPlayback->pMaster->sfifoGroup, &sfifoSettings);
        length = hPlayback->bufferSize;
        pBuffer = BMEM_AllocAligned(hPlayback->hHeap, length, 5, 0);
        if ( NULL == pBuffer )
        {
            BDBG_ERR(("Unable to suspend, stopping playback"));
            BAPE_Playback_Stop(hPlayback);
            hPlayback->suspending = false;
            return;
        }
        BMEM_ConvertAddressToCached(hPlayback->hHeap, pBuffer, &pDstCached);

        /* copy the data*/
        chPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&hPlayback->node.connectors[0].format);
        /* Setup Buffers */
        for ( i = 0; i < chPairs; i++)
        {
            baseAddr = sfifoSettings.bufferInfo[i*2].base;
            BAPE_SfifoGroup_P_GetReadAddress(hPlayback->pMaster->sfifoGroup, i*2, 0, &rdAddr);

            BMEM_ConvertAddressToCached(hPlayback->hHeap, hPlayback->pBuffer[i*2], &pSrcCached);

            if (rdAddr+bufferDepth <= baseAddr+length) /* there is no wrap */
            {
                BKNI_Memcpy(pDstCached, (uint8_t*)pSrcCached+(rdAddr-baseAddr), bufferDepth);
            }
            else /* there is wrap */
            {
                preWrap = baseAddr + length - rdAddr;
                postWrap = bufferDepth - preWrap;

                BKNI_Memcpy(pDstCached, (uint8_t*)pSrcCached+(rdAddr-baseAddr), preWrap);
                BKNI_Memcpy((uint8_t*)pDstCached+preWrap, pSrcCached, postWrap);
            }
            BKNI_Memcpy(pSrcCached, pDstCached, bufferDepth);
            BMEM_Heap_FlushCache(hPlayback->hHeap, pSrcCached, bufferDepth);

            if (!sfifoSettings.interleaveData)
            {
                baseAddr = sfifoSettings.bufferInfo[i*2+1].base;
                BAPE_SfifoGroup_P_GetReadAddress(hPlayback->pMaster->sfifoGroup, i*2, 1, &rdAddr);

                BMEM_ConvertAddressToCached(hPlayback->hHeap, hPlayback->pBuffer[i*2+1], &pSrcCached);

                if (rdAddr+bufferDepth <= baseAddr+length) /* there is no wrap */
                {
                    BKNI_Memcpy(pDstCached, (uint8_t*)pSrcCached+(rdAddr-baseAddr), bufferDepth);
                }
                else /* there is wrap */
                {
                    preWrap = baseAddr + length - rdAddr;
                    postWrap = bufferDepth - preWrap;

                    BKNI_Memcpy(pDstCached, (uint8_t*)pSrcCached+(rdAddr-baseAddr), preWrap);
                    BKNI_Memcpy((uint8_t*)pDstCached+preWrap, pSrcCached, postWrap);
                }
                BKNI_Memcpy(pSrcCached, pDstCached, bufferDepth);
                BMEM_Heap_FlushCache(hPlayback->hHeap, pSrcCached, bufferDepth);
            }
        }
        BMEM_Free(hPlayback->hHeap, pBuffer);
    }

    /* Stop Paths Downstream */
    BAPE_PathNode_P_StopPaths(&hPlayback->node);

    /* Remove the interrupt handler */
    if ( hPlayback->interrupts.watermark.pCallback_isr )
    {
        (void)BAPE_SfifoGroup_P_SetFreemarkInterrupt(hPlayback->pMaster->sfifoGroup, NULL, NULL, 0);
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
        BAPE_SfifoGroup_P_Flush(hPlayback->pMaster->sfifoGroup);
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
    
    if ( (hPlayback->running) && !hPlayback->startSettings.loopEnabled )
    {
        BDBG_ASSERT(NULL != hPlayback->pMaster);
        BAPE_SfifoGroup_P_GetBuffer(hPlayback->pMaster->sfifoGroup, pBuffers);
    }
    else
    {
        /* TODO: hPlayback non-interleaved and multichannel */
        numChannelPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&hPlayback->node.connectors[0].format);
        pBuffers->numBuffers = 2*numChannelPairs;
        pBuffers->interleaved = hPlayback->startSettings.interleaved;
        if ( !hPlayback->suspending ) /* As long as we aren't in the middle of suspending */
        {
            pBuffers->bufferSize = hPlayback->bufferSize - hPlayback->bufferDepth;
            if ( pBuffers->bufferSize )
            {
                if ( pBuffers->interleaved )
                {
                    for ( i = 0; i < numChannelPairs; i++)
                    {
                        pBuffers->buffers[i*2].pBuffer = (void *)((unsigned long)hPlayback->pBuffer[i] + hPlayback->bufferDepth);
                    }
                }
                else
                {
                    for ( i = 0; i < numChannelPairs; i++)
                    {
                        pBuffers->buffers[i*2].pBuffer = (void *)((unsigned long)hPlayback->pBuffer[i] + hPlayback->bufferDepth);
                        pBuffers->buffers[i*2 + 1].pBuffer = (void *)((unsigned long)hPlayback->pBuffer[i] + hPlayback->bufferDepth);
                    }
                }
            }
        }
    }

    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    return errCode;
}

BERR_Code BAPE_Playback_CommitData(
    BAPE_PlaybackHandle hPlayback,
    unsigned numBytes                   /* Number of bytes written into the buffer */
    )
{
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    BDBG_MSG(("Playback %p (%d) CommitData", (void *)hPlayback, hPlayback->index));

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
        if ( numBytes + hPlayback->bufferDepth > hPlayback->bufferSize )
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
    pStatus->fifoSize = hPlayback->bufferSize;
    
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

    if ( hPlayback->running )
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

    /* These aren't actually used by the interrupt directly, so no need for critical section here. */
    hPlayback->interrupts = *pInterrupts;

    return BERR_SUCCESS;
}

static void BAPE_Playback_P_ReArmWatermark(BAPE_PlaybackHandle hPlayback)
{
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    if ( hPlayback->running )
    {
        BDBG_ASSERT(NULL != hPlayback->pMaster);
        BAPE_SfifoGroup_P_RearmFreemarkInterrupt(hPlayback->pMaster->sfifoGroup);
    }
}

static BERR_Code BAPE_Playback_P_ConfigPathToOutput(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection)
{
    BAPE_PlaybackHandle hPlayback;
    BERR_Code errCode;
    unsigned offset;
    unsigned chPairs;
    unsigned i;

    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    BDBG_OBJECT_ASSERT(pConnection->pSource, BAPE_PathConnector);
    BDBG_OBJECT_ASSERT(pConnection->pSink, BAPE_PathNode);

    hPlayback = pNode->pHandle;
    BDBG_OBJECT_ASSERT(hPlayback, BAPE_Playback);

    if ( pConnection->sfifoGroup )
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
        sfifoSettings.interleaveData = hPlayback->startSettings.interleaved;
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

        chPairs = BAPE_FMT_P_GetNumChannelPairs_isrsafe(&hPlayback->node.connectors[0].format);
        /* Setup Buffers */
        for ( i = 0; i < chPairs; i++) 
        {
            if ( sfifoSettings.interleaveData) 
            {
                errCode = BMEM_ConvertAddressToOffset(hPlayback->hHeap, hPlayback->pBuffer[i*2], &offset);
                BDBG_ASSERT(BERR_SUCCESS == errCode);
                sfifoSettings.bufferInfo[i].base = offset;
                sfifoSettings.bufferInfo[i].writeOffset = hPlayback->bufferDepth;
                sfifoSettings.bufferInfo[i].length = hPlayback->bufferSize;
                sfifoSettings.bufferInfo[i].watermark = hPlayback->threshold;
                if ( hPlayback->startSettings.startThreshold > hPlayback->bufferSize )
                {
                    BDBG_WRN(("Start threshold is too large.  Starting with buffer full."));
                    sfifoSettings.bufferInfo[i].wrpoint = offset + hPlayback->bufferSize - 1;
                }
                else if ( hPlayback->startSettings.startThreshold > 0 )
                {
                    sfifoSettings.bufferInfo[i].wrpoint = offset + hPlayback->startSettings.startThreshold-1;
                }
                else
                {
                    sfifoSettings.bufferInfo[i].wrpoint = 0;
                }
            }
            else
            {
                errCode = BMEM_ConvertAddressToOffset(hPlayback->hHeap, hPlayback->pBuffer[i*2], &offset);
                BDBG_ASSERT(BERR_SUCCESS == errCode);
                sfifoSettings.bufferInfo[i*2].base = offset;
                sfifoSettings.bufferInfo[i*2].writeOffset = hPlayback->bufferDepth;
                sfifoSettings.bufferInfo[i*2].length = hPlayback->bufferSize;
                sfifoSettings.bufferInfo[i*2].watermark = hPlayback->threshold;
                if ( hPlayback->startSettings.startThreshold > hPlayback->bufferSize )
                {
                    BDBG_WRN(("Start threshold is too large.  Starting with buffer full."));
                    sfifoSettings.bufferInfo[i*2].wrpoint = offset + hPlayback->bufferSize - 1;
                }
                else if ( hPlayback->startSettings.startThreshold > 0 )
                {
                    sfifoSettings.bufferInfo[i*2].wrpoint = offset + hPlayback->startSettings.startThreshold-1;
                }
                else
                {
                    sfifoSettings.bufferInfo[i*2].wrpoint = 0;
                }

                errCode = BMEM_ConvertAddressToOffset(hPlayback->hHeap, hPlayback->pBuffer[(i*2) + 1], &offset);
                BDBG_ASSERT(BERR_SUCCESS == errCode);
                sfifoSettings.bufferInfo[i*2 + 1].base = offset;
                sfifoSettings.bufferInfo[i*2 + 1].writeOffset = hPlayback->bufferDepth;
                sfifoSettings.bufferInfo[i*2 + 1].length = hPlayback->bufferSize;
                sfifoSettings.bufferInfo[i*2 + 1].watermark = hPlayback->threshold;
                if ( hPlayback->startSettings.startThreshold > hPlayback->bufferSize )
                {
                    BDBG_WRN(("Start threshold is too large.  Starting with buffer full."));
                    sfifoSettings.bufferInfo[i*2 + 1].wrpoint = offset + hPlayback->bufferSize - 1;
                }
                else if ( hPlayback->startSettings.startThreshold > 0 )
                {
                    sfifoSettings.bufferInfo[i*2 + 1].wrpoint = offset + hPlayback->startSettings.startThreshold-1;
                }
                else
                {
                    sfifoSettings.bufferInfo[i*2 + 1].wrpoint = 0;
                }
            }
        }
        BAPE_SfifoGroup_P_SetSettings(pConnection->sfifoGroup, &sfifoSettings);
    }
    /* Other nodes don't require anything done here */

    return BERR_SUCCESS;    
}

static BERR_Code BAPE_Playback_P_StartPathToOutput(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection)
{
    BERR_Code errCode;
    BDBG_OBJECT_ASSERT(pNode, BAPE_PathNode);
    BDBG_OBJECT_ASSERT(pConnection, BAPE_PathConnection);
    if ( pConnection->sfifoGroup )
    {
        errCode = BAPE_SfifoGroup_P_Start(pConnection->sfifoGroup, false);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }
    }
    return BERR_SUCCESS;
}

static void BAPE_Playback_P_StopPathToOutput(BAPE_PathNode *pNode, BAPE_PathConnection *pConnection)
{
    BAPE_PlaybackHandle hPlayback = pNode->pHandle;

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
