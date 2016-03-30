/***************************************************************************
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
 *
 * Module Description:
 *
 **************************************************************************/
#include "nexus_video_decoder_module.h"
#include "nexus_still_decoder_impl.h"
#include "nexus_power_management.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_still_decoder);

#if NEXUS_NUM_STILL_DECODES
static BLST_S_HEAD(decoder_list, NEXUS_StillDecoder) g_decoders; /* global list of sw handles */
#define CURRENT(stillDecoder) ((stillDecoder)->hw->current == (stillDecoder))
static void NEXUS_StillDecoder_P_ReleaseStripedSurface( NEXUS_HwStillDecoderHandle stillDecoder, NEXUS_StripedSurfaceHandle stripedSurface );
#endif

BDBG_OBJECT_ID(NEXUS_HwStillDecoder);

void NEXUS_StillDecoder_GetDefaultOpenSettings( NEXUS_StillDecoderOpenSettings *pSettings )
{
#if NEXUS_NUM_STILL_DECODES
    NEXUS_VideoFormatInfo info;

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->fifoSize = 1 * 1024 * 1024;

    NEXUS_VideoFormat_GetInfo(g_NEXUS_videoDecoderModuleSettings.stillMemory[0].maxFormat, &info);
    pSettings->maxWidth = info.width;
    pSettings->maxHeight = info.height;

    /* copy over default codec support appropriate to stilldecoder */
    BKNI_Memcpy(pSettings->supportedCodecs, g_NEXUS_videoDecoderModuleSettings.stillMemory[0].supportedCodecs, sizeof(pSettings->supportedCodecs));
    pSettings->supportedCodecs[NEXUS_VideoCodec_eH264_Svc] = false; /* not supported by stilldecoder */
    pSettings->supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = false; /* not supported by stilldecoder */
#else
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#endif
}

#if NEXUS_NUM_STILL_DECODES
static NEXUS_Error nexus_stilldecoder_p_openchannel(NEXUS_HwStillDecoderHandle stillDecoder, const NEXUS_StillDecoderStartSettings *pSettings)
{
    NEXUS_Error rc;
    unsigned index;
    BXVD_ChannelSettings xvdChannelSettings;
    BAVC_VideoCompressionStd stVideoCompressionList[BAVC_VideoCompressionStd_eMax];

    index = NEXUS_NUM_VIDEO_DECODERS;

    rc = BXVD_GetChannelDefaultSettings(stillDecoder->device->xvd, index, &xvdChannelSettings);
    if (rc) return BERR_TRACE(rc);

    xvdChannelSettings.peVideoCmprStdList = stVideoCompressionList;
    if (pSettings) {
        bool supportedCodecs[NEXUS_VideoCodec_eMax];
        BKNI_Memset(supportedCodecs, 0, sizeof(supportedCodecs));
        supportedCodecs[pSettings->codec] = true;
        NEXUS_VideoDecoder_SetVideoCmprStdList_priv(supportedCodecs, &xvdChannelSettings, BAVC_VideoCompressionStd_eMax);
    }
    else {
        NEXUS_VideoDecoder_SetVideoCmprStdList_priv(stillDecoder->openSettings.supportedCodecs, &xvdChannelSettings, BAVC_VideoCompressionStd_eMax);
    }
    if (pSettings && pSettings->output.maxWidth && pSettings->output.maxHeight) {
        xvdChannelSettings.eDecodeResolution = NEXUS_VideoDecoder_GetDecodeResolution_priv(pSettings->output.maxWidth, pSettings->output.maxHeight);
    }
    else {
        xvdChannelSettings.eDecodeResolution = NEXUS_VideoDecoder_GetDecodeResolution_priv(stillDecoder->openSettings.maxWidth, stillDecoder->openSettings.maxHeight);
    }
    xvdChannelSettings.eChannelMode = BXVD_ChannelMode_eStill;
    if (g_NEXUS_videoDecoderModuleSettings.heapSize[stillDecoder->index].secondaryPicture &&
        ((pSettings && pSettings->codec == NEXUS_VideoCodec_eH265) || stillDecoder->openSettings.supportedCodecs[NEXUS_VideoCodec_eH265]))
    {
        xvdChannelSettings.bSplitPictureBuffersEnable = true;
    }

    xvdChannelSettings.b10BitBuffersEnable = (g_NEXUS_videoDecoderModuleSettings.stillMemory[stillDecoder->index].colorDepth >= 10);

    if (stillDecoder->buffer.mem) {
        xvdChannelSettings.uiChannelPictureBlockSize = stillDecoder->buffer.size;
        stillDecoder->buffer.pictureMemory = NULL;
        xvdChannelSettings.hChannelPictureBlock = stillDecoder->buffer.mem;
        xvdChannelSettings.uiChannelPictureBlockOffset = stillDecoder->buffer.memoryOffset;
    }
    else if (stillDecoder->openSettings.heap) {
        BXVD_FWMemConfig memConfig;

        rc = BXVD_GetChannelMemoryConfig(stillDecoder->device->xvd, &xvdChannelSettings, &memConfig);
        if (rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
        xvdChannelSettings.uiChannelPictureBlockSize = memConfig.uiPictureHeapSize;
        xvdChannelSettings.hChannelPictureBlock = BMMA_Alloc(NEXUS_Heap_GetMmaHandle(stillDecoder->openSettings.heap), memConfig.uiPictureHeapSize, 0, NULL);
        if(xvdChannelSettings.hChannelPictureBlock==NULL) {
            return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        }
        stillDecoder->buffer.pictureMemory = xvdChannelSettings.hChannelPictureBlock;
    }

    BDBG_MSG(("creating xvd channel %d for still decoder AVD%d", index, stillDecoder->index));
    /* The index of the still channel is one more than the regular. I don't believe adding index but it's an added check that index == 0. */
    rc = BXVD_OpenChannel(stillDecoder->device->xvd, &stillDecoder->xvdChannel, index, &xvdChannelSettings);
    if (rc) return BERR_TRACE(rc);

    return NEXUS_SUCCESS;
}

static void nexus_stilldecoder_p_closechannel(NEXUS_HwStillDecoderHandle stillDecoder)
{
    BDBG_ASSERT(stillDecoder->xvdChannel);
    BDBG_MSG(("closing xvd channel for still decoder AVD%d", stillDecoder->index));
    BXVD_CloseChannel(stillDecoder->xvdChannel);
    stillDecoder->xvdChannel = NULL;
    if(stillDecoder->buffer.pictureMemory) {
        BMMA_Free(stillDecoder->buffer.pictureMemory);
        stillDecoder->buffer.pictureMemory = NULL;
    }
    if (stillDecoder->buffer.block) {
        NEXUS_OBJECT_RELEASE(stillDecoder->current, NEXUS_MemoryBlock, stillDecoder->buffer.block);
        stillDecoder->buffer.block = NULL;
    }
}

static NEXUS_Error nexus_stilldecoder_p_outputbuffer(NEXUS_HwStillDecoderHandle stillDecoder, const NEXUS_StillDecoderStartSettings *pSettings)
{
    NEXUS_Error rc;
    bool has_output_buffer = pSettings && (pSettings->output.memory || pSettings->output.buffer);
    unsigned bufferSize = pSettings?pSettings->output.size:0;
    unsigned maxHeight = pSettings?pSettings->output.maxHeight:0;

    if (has_output_buffer && !bufferSize) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    else if (!has_output_buffer && bufferSize) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (stillDecoder->xvdChannel) {
        nexus_stilldecoder_p_closechannel(stillDecoder);
    }

    /* if no output buffer needed or present, short circuit. */
    if (!has_output_buffer && !stillDecoder->buffer.mem && stillDecoder->xvdChannel) {
        return 0;
    }

    BDBG_ASSERT(!stillDecoder->buffer.block);
    if (pSettings && pSettings->output.memory) {
        /* if more than one SW still decoder on this HW still decoder, do not allow external buffer. */
        if (stillDecoder->refcnt > 1) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        stillDecoder->buffer.block = pSettings->output.memory;
        NEXUS_OBJECT_ACQUIRE(stillDecoder->current, NEXUS_MemoryBlock, stillDecoder->buffer.block);
        stillDecoder->buffer.mem = NEXUS_MemoryBlock_GetBlock_priv(stillDecoder->buffer.block);
        stillDecoder->buffer.memoryOffset = pSettings->output.memoryOffset;
    }
    else if (pSettings && pSettings->output.buffer) {
        unsigned offset;
        /* if more than one SW still decoder on this HW still decoder, do not allow external buffer. */
        if (stillDecoder->refcnt > 1) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        NEXUS_Module_Lock(g_NEXUS_videoDecoderModuleSettings.core);
        rc = NEXUS_MemoryBlock_BlockAndOffsetFromRange_priv(pSettings->output.buffer, pSettings->output.size, &stillDecoder->buffer.block, &offset);
        NEXUS_Module_Unlock(g_NEXUS_videoDecoderModuleSettings.core);
        if (rc) return BERR_TRACE(rc);
        NEXUS_OBJECT_ACQUIRE(stillDecoder->current, NEXUS_MemoryBlock, stillDecoder->buffer.block);
        stillDecoder->buffer.mem = NEXUS_MemoryBlock_GetBlock_priv(stillDecoder->buffer.block);
        stillDecoder->buffer.memoryOffset = offset;
    }
    else {
        stillDecoder->buffer.mem = NULL;
        stillDecoder->buffer.memoryOffset = 0;
    }
    stillDecoder->buffer.size = bufferSize;
    stillDecoder->buffer.maxHeight = maxHeight;

    /* stillDecoder->rave is NULL, we are closing */
    if (stillDecoder->rave) {
        rc = nexus_stilldecoder_p_openchannel(stillDecoder, pSettings);
        if (rc) return BERR_TRACE(rc);
    }

    return NEXUS_SUCCESS;
}

static void NEXUS_HwStillDecoder_P_Close_Avd( NEXUS_HwStillDecoderHandle stillDecoder );
static NEXUS_HwStillDecoderHandle NEXUS_HwStillDecoder_P_Open_Avd(unsigned index, const NEXUS_StillDecoderOpenSettings *pSettings )
{
    NEXUS_HwStillDecoderHandle stillDecoder;
    NEXUS_RaveOpenSettings raveSettings;
    BERR_Code rc;
    int32_t pic_buf_length;

    stillDecoder = BKNI_Malloc(sizeof(*stillDecoder));
    if (!stillDecoder) {
        rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(stillDecoder, 0, sizeof(*stillDecoder));
    BDBG_OBJECT_SET(stillDecoder, NEXUS_HwStillDecoder);
    stillDecoder->openSettings = *pSettings;
    stillDecoder->index = index;
    stillDecoder->device = &g_NEXUS_videoDecoderXvdDevices[index];
    BDBG_ASSERT(stillDecoder->device->index == index);

    stillDecoder->endCodeFoundCallback = NEXUS_TaskCallback_Create(stillDecoder, NULL);
    if (!stillDecoder->endCodeFoundCallback) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error;}
    stillDecoder->stillPictureReadyCallback = NEXUS_IsrCallback_Create(stillDecoder, NULL);
    if (!stillDecoder->stillPictureReadyCallback) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error;}

    if (pSettings->maxWidth && pSettings->maxHeight) {
        rc = nexus_stilldecoder_p_openchannel(stillDecoder, NULL);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }

    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultOpenSettings_priv(&raveSettings);
    rc = BXVD_GetBufferConfig(stillDecoder->device->xvd, &raveSettings.config, &pic_buf_length);
    raveSettings.config.Cdb.Length = pSettings->fifoSize;
    raveSettings.config.Itb.Length = pSettings->fifoSize / 10;
    if (raveSettings.config.Itb.Length % 128) {
        raveSettings.config.Itb.Length -= raveSettings.config.Itb.Length % 128;
    }
    BKNI_Memcpy(raveSettings.supportedCodecs, pSettings->supportedCodecs, sizeof(raveSettings.supportedCodecs));
    raveSettings.heap = pSettings->cdbHeap;
    stillDecoder->rave = NEXUS_Rave_Open_priv(&raveSettings);
    UNLOCK_TRANSPORT();
    if (rc || !stillDecoder->rave) {rc = BERR_TRACE(NEXUS_UNKNOWN); goto error;}

    /* TODO: per device, not per channel */
    rc = BXVD_InstallDeviceInterruptCallback(stillDecoder->device->xvd, BXVD_DeviceInterrupt_eDecodedStillBufferReady, NEXUS_VideoDecoder_P_StillReady_isr, stillDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    return stillDecoder;

error:
    NEXUS_HwStillDecoder_P_Close_Avd(stillDecoder);
    return NULL;
}

static void NEXUS_HwStillDecoder_P_Close_Avd( NEXUS_HwStillDecoderHandle stillDecoder )
{
    BXVD_UnInstallDeviceInterruptCallback(stillDecoder->device->xvd, BXVD_DeviceInterrupt_eDecodedStillBufferReady);
    if (stillDecoder->rave) {
        LOCK_TRANSPORT();
        NEXUS_Rave_Close_priv(stillDecoder->rave);
        UNLOCK_TRANSPORT();
        stillDecoder->rave = NULL;
    }
    if (stillDecoder->xvdChannel) {
        nexus_stilldecoder_p_closechannel(stillDecoder);
    }
    (void)nexus_stilldecoder_p_outputbuffer(stillDecoder, NULL);

    NEXUS_StillDecoder_P_ReleaseStripedSurface(stillDecoder, NULL);

    if (stillDecoder->endCodeFoundCallback) {
        NEXUS_TaskCallback_Destroy(stillDecoder->endCodeFoundCallback);
    }
    if (stillDecoder->stillPictureReadyCallback) {
        NEXUS_IsrCallback_Destroy(stillDecoder->stillPictureReadyCallback);
    }

    BDBG_OBJECT_DESTROY(stillDecoder, NEXUS_HwStillDecoder);
    BKNI_Free(stillDecoder);
}

NEXUS_StillDecoderHandle NEXUS_StillDecoder_P_Open_Avd( NEXUS_VideoDecoderHandle parentDecoder, unsigned index, const NEXUS_StillDecoderOpenSettings *pSettings )
{
    NEXUS_StillDecoderHandle stillDecoder, d;
    NEXUS_StillDecoderOpenSettings defaultSettings;
    BERR_Code rc;

    if (parentDecoder) {
        BDBG_WRN(("parentDecoder in NEXUS_StillDecoder_Open is deprecated and unused. please change to NULL."));
    }
    if (index == NEXUS_ANY_ID) {
        index = 0;
    }
    if (index >= NEXUS_MAX_XVD_DEVICES) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    if (!g_NEXUS_videoDecoderXvdDevices[index].xvd) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    if (!pSettings) {
        NEXUS_StillDecoder_GetDefaultOpenSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    stillDecoder = BKNI_Malloc(sizeof(*stillDecoder));
    if (!stillDecoder) {
        rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_StillDecoder, stillDecoder);
    stillDecoder->intf = &NEXUS_StillDecoder_P_Interface_Avd;

    /* find existing HwStillDecoder */
    for (d = BLST_S_FIRST(&g_decoders); d; d = BLST_S_NEXT(d, link)) {
        if (d->hw->index == index && !d->hw->buffer.mem) {
            break;
        }
    }
    if (d) {
        stillDecoder->hw = d->hw;
    }
    else {
        /* if none found, create a HwStillDecoder */
        stillDecoder->hw = NEXUS_HwStillDecoder_P_Open_Avd(index, pSettings);
        if (!stillDecoder->hw) {
            BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
    }
    stillDecoder->hw->refcnt++;
    BLST_S_INSERT_HEAD(&g_decoders, stillDecoder, link);

    return stillDecoder;

error:
    NEXUS_StillDecoder_P_Close_Avd(stillDecoder);
    return NULL;
}

void NEXUS_StillDecoder_P_Close_Avd( NEXUS_StillDecoderHandle stillDecoder )
{
    if (stillDecoder->hw) {
        /* just in case there's a timer firing */
        NEXUS_StillDecoder_Stop(stillDecoder);

        BDBG_ASSERT(stillDecoder->hw->refcnt);
        if (--stillDecoder->hw->refcnt == 0) {
            NEXUS_HwStillDecoder_P_Close_Avd(stillDecoder->hw);
        }
        BLST_S_REMOVE(&g_decoders, stillDecoder, NEXUS_StillDecoder, link);
    }
    NEXUS_OBJECT_DESTROY(NEXUS_StillDecoder, stillDecoder);
    BKNI_Free(stillDecoder);
}
#endif

void NEXUS_StillDecoder_GetDefaultStartSettings( NEXUS_StillDecoderStartSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

#if NEXUS_NUM_STILL_DECODES
static NEXUS_Error NEXUS_HwStillDecoder_P_Start_Avd( NEXUS_StillDecoderHandle swStillDecoder )
{
    NEXUS_HwStillDecoderHandle stillDecoder = swStillDecoder->hw;
    const NEXUS_StillDecoderStartSettings *pSettings = &swStillDecoder->settings;
    NEXUS_RaveSettings raveSettings;
    NEXUS_PidChannelStatus pidChannelStatus;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(stillDecoder, NEXUS_HwStillDecoder);
    BDBG_ASSERT(!stillDecoder->current);

    rc = nexus_stilldecoder_p_outputbuffer(stillDecoder, pSettings);
    if (rc) {
        return BERR_TRACE(rc);
    }

    NEXUS_PidChannel_GetStatus(pSettings->pidChannel, &pidChannelStatus);

    LOCK_TRANSPORT();
    NEXUS_Rave_Disable_priv(stillDecoder->rave); /* must disable before reconfigure */
    NEXUS_Rave_Flush_priv(stillDecoder->rave);
    NEXUS_Rave_GetDefaultSettings_priv(&raveSettings);
    raveSettings.pidChannel = pSettings->pidChannel;
    raveSettings.bandHold = pidChannelStatus.playback;
    raveSettings.continuityCountEnabled = !pidChannelStatus.playback;
    rc = NEXUS_Rave_ConfigureVideo_priv(stillDecoder->rave, pSettings->codec, &raveSettings);
    NEXUS_Rave_Enable_priv(stillDecoder->rave);
    UNLOCK_TRANSPORT();
    if (rc) {return BERR_TRACE(rc);}

    NEXUS_PidChannel_ConsumerStarted(pSettings->pidChannel); /* needed to unpause playback & stuff like that */

    stillDecoder->settings = *pSettings;
    stillDecoder->status.endCodeFound = false;
    stillDecoder->status.stillPictureReady = false;
    NEXUS_StillDecoder_P_ReleaseStripedSurface(stillDecoder, NULL);

    NEXUS_TaskCallback_Set(stillDecoder->endCodeFoundCallback, &pSettings->endCodeFound);
    NEXUS_IsrCallback_Set(stillDecoder->stillPictureReadyCallback, &pSettings->stillPictureReady);

    NEXUS_Time_Get(&stillDecoder->startTime);

    /* start to poll on the ITB, looking for the endCode */
    if (!stillDecoder->timer) {
        stillDecoder->timer = NEXUS_ScheduleTimer(10, NEXUS_StillDecoder_P_CheckForEndCode, stillDecoder);
    }

    stillDecoder->current = swStillDecoder;
    return 0;
}

static void NEXUS_HwStillDecoder_P_Stop_Avd(NEXUS_StillDecoderHandle swStillDecoder)
{
    NEXUS_HwStillDecoderHandle stillDecoder = swStillDecoder->hw;
    bool never_started = false;

    BDBG_OBJECT_ASSERT(stillDecoder, NEXUS_HwStillDecoder);

    if (stillDecoder->timer) {
        NEXUS_CancelTimer(stillDecoder->timer);
        stillDecoder->timer = NULL;
        never_started = true;
    }

    LOCK_TRANSPORT();
    NEXUS_Rave_Disable_priv(stillDecoder->rave);
    NEXUS_Rave_Flush_priv(stillDecoder->rave);
    NEXUS_Rave_RemovePidChannel_priv(stillDecoder->rave);
    UNLOCK_TRANSPORT();

    /* if we never received a still, we should reset the XVD channel */
    if (!stillDecoder->status.stillPictureReady && !never_started) {
        BXVD_DecodeStillPictureReset(stillDecoder->xvdChannel);
    }

    stillDecoder->current = NULL;
}

NEXUS_Error NEXUS_StillDecoder_P_Start_Avd( NEXUS_StillDecoderHandle stillDecoder, const NEXUS_StillDecoderStartSettings *pSettings )
{
    NEXUS_Error rc;
    bool supportHd;

    BDBG_OBJECT_ASSERT(stillDecoder, NEXUS_StillDecoder);

    if (stillDecoder->started) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    /* Start not valid with NULL settings */
    if (!pSettings) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    supportHd =
        pSettings->output.maxHeight ? pSettings->output.maxHeight > 480 :
        (g_NEXUS_videoDecoderModuleSettings.stillMemory[0].maxFormat >= NEXUS_VideoFormat_e480p);

    switch (pSettings->codec) {
    case NEXUS_VideoCodec_eMpeg2:
        stillDecoder->endCode = 0xB7;
        stillDecoder->stillMode = supportHd?BXVD_DecodeModeStill_eMPEG_HD:BXVD_DecodeModeStill_eMPEG_SD;
        break;
    case NEXUS_VideoCodec_eH264:
        stillDecoder->endCode = 0x0A;
        stillDecoder->stillMode = supportHd?BXVD_DecodeModeStill_eAVC_HD:BXVD_DecodeModeStill_eAVC_SD;
        break;
    case NEXUS_VideoCodec_eH265:
        stillDecoder->endCode = 0x4A; /* Even though HEVC NAL is 16 bit (two bytes), nal_unit_type located in the first byte, see '7.3.1.2 NAL unit header syntax' */
        if (pSettings->output.maxHeight > 1088) {
            stillDecoder->stillMode = BXVD_DecodeModeStill_eHEVC_4K;
        } else {
            stillDecoder->stillMode = supportHd?BXVD_DecodeModeStill_eHEVC_HD:BXVD_DecodeModeStill_eHEVC_SD;
        }
        break;
    case NEXUS_VideoCodec_eVc1:
    case NEXUS_VideoCodec_eVc1SimpleMain:
        stillDecoder->endCode = 0x0A;
        stillDecoder->stillMode = supportHd?BXVD_DecodeModeStill_eVC1_HD:BXVD_DecodeModeStill_eVC1_SD;
        break;
    case NEXUS_VideoCodec_eMpeg4Part2:
    case NEXUS_VideoCodec_eDivx311:
        stillDecoder->endCode = 0xB1;
        stillDecoder->stillMode = supportHd?BXVD_DecodeModeStill_eMPEG4Part2_HD:BXVD_DecodeModeStill_eMPEG4Part2_SD;
        break;
    case NEXUS_VideoCodec_eVp8:
        stillDecoder->endCode = 0;
        stillDecoder->stillMode = supportHd?BXVD_DecodeModeStill_eVP8_SD:BXVD_DecodeModeStill_eVP8_HD;
        break;
    default:
        BDBG_WRN(("video codec %d not supported", pSettings->codec));
        return NEXUS_NOT_SUPPORTED;
    }

    stillDecoder->settings = *pSettings;

    if (!stillDecoder->hw->current) {
        rc = NEXUS_HwStillDecoder_P_Start_Avd(stillDecoder);
        if (rc) {
            return BERR_TRACE(rc);
        }
    }

    stillDecoder->started = true;

    return 0;
}

void NEXUS_StillDecoder_P_Stop_Avd(NEXUS_StillDecoderHandle stillDecoder)
{
    NEXUS_StillDecoderHandle d;

    BDBG_OBJECT_ASSERT(stillDecoder, NEXUS_StillDecoder);

    if (!stillDecoder->started) {
        return;
    }
    if (!CURRENT(stillDecoder)) {
        stillDecoder->started = false;
        return;
    }

    NEXUS_HwStillDecoder_P_Stop_Avd(stillDecoder);
    stillDecoder->started = false;

    /* TODO: a shortcoming in this virtualization is depending on the client stopping because we don't require per client storage for the still. */
    /* start the next pending decoder */
    for (d = BLST_S_FIRST(&g_decoders); d; d = BLST_S_NEXT(d, link)) {
        if (d->hw == stillDecoder->hw && d->started) {
            if (NEXUS_HwStillDecoder_P_Start_Avd(d)) {
                /* unable to start, so keep going */
                d->started = false;
            }
            else {
                break;
            }
        }
    }
}

NEXUS_Error NEXUS_StillDecoder_P_GetStatus_Avd( NEXUS_StillDecoderHandle swStillDecoder, NEXUS_StillDecoderStatus *pStatus )
{
    NEXUS_HwStillDecoderHandle stillDecoder;
    BERR_Code rc;
    BXVD_ChannelStatus channelStatus;

    BDBG_OBJECT_ASSERT(swStillDecoder, NEXUS_StillDecoder);
    stillDecoder = swStillDecoder->hw; /* does not have to be current */

    if (CURRENT(swStillDecoder)) {
        *pStatus = stillDecoder->status;

        if (stillDecoder->xvdChannel) {
            rc = BXVD_GetChannelStatus(stillDecoder->xvdChannel, &channelStatus);
            if (rc) {return BERR_TRACE(rc);}
            pStatus->avdStatusBlock = channelStatus.uiAVDStatusBlock;
        }
    }
    else {
        BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    }

    pStatus->rave.mainIndex = -1;
    pStatus->rave.mainSwRaveIndex = -1;
    if (stillDecoder->rave) {
        NEXUS_RaveStatus raveStatus;
        LOCK_TRANSPORT();
        rc = NEXUS_Rave_GetStatus_priv(stillDecoder->rave, &raveStatus);
        UNLOCK_TRANSPORT();
        if (!rc) {
            pStatus->rave.mainIndex = raveStatus.index;
            pStatus->rave.mainSwRaveIndex = raveStatus.swRaveIndex;
        }
    }


    return 0;
}

static void nexus_still_decoder_p_free_block(NEXUS_HwStillDecoderHandle stillDecoder, NEXUS_MemoryBlockHandle block)
{
    /* if block came from user, NEXUS_MemoryBlock_Free_driver will decrement refcnt and remove from objdb, making it unusable
    by the user again. so, only decrement refcnt in that case. */
    if (stillDecoder->buffer.block == block) {
        NEXUS_OBJECT_RELEASE(stillDecoder->current, NEXUS_MemoryBlock, stillDecoder->buffer.block);
    }
    else {
        NEXUS_MemoryBlock_Free_driver(block);
    }
}

NEXUS_Error NEXUS_StillDecoder_P_GetStripedSurface_Avd( NEXUS_StillDecoderHandle swStillDecoder, NEXUS_StripedSurfaceHandle *pStripedSurface )
{
    NEXUS_HwStillDecoderHandle stillDecoder;

    BDBG_OBJECT_ASSERT(swStillDecoder, NEXUS_StillDecoder);
    *pStripedSurface = NULL;
    if (!CURRENT(swStillDecoder)) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    stillDecoder = swStillDecoder->hw;
    if (stillDecoder->status.stillPictureReady) {
        if (stillDecoder->stripedSurface.recreate) {
            NEXUS_StripedSurfaceCreateSettings createSettings;
            BXVD_StillPictureBuffers buffer;

            NEXUS_StillDecoder_P_ReleaseStripedSurface(stillDecoder, NULL);

            /* get data out of critical section */
            BKNI_EnterCriticalSection();
            buffer = stillDecoder->stripedSurface.buffers;
            stillDecoder->stripedSurface.recreate = false;
            BKNI_LeaveCriticalSection();

            NEXUS_StripedSurface_GetDefaultCreateSettings(&createSettings);
            NEXUS_Module_Lock(g_NEXUS_videoDecoderModuleSettings.core);
            createSettings.lumaBuffer = NEXUS_MemoryBlock_FromMma_priv(buffer.hLuminanceFrameBufferBlock);
            NEXUS_Module_Unlock(g_NEXUS_videoDecoderModuleSettings.core);
            if(createSettings.lumaBuffer==NULL) {
                return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            }
            NEXUS_Module_Lock(g_NEXUS_videoDecoderModuleSettings.core);
            createSettings.chromaBuffer = NEXUS_MemoryBlock_FromMma_priv(buffer.hChrominanceFrameBufferBlock);
            NEXUS_Module_Unlock(g_NEXUS_videoDecoderModuleSettings.core);
            if(createSettings.chromaBuffer==NULL) {
                nexus_still_decoder_p_free_block(stillDecoder, createSettings.lumaBuffer);
                return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            }
            createSettings.lumaBufferOffset = buffer.ulLuminanceFrameBufferBlockOffset;
            createSettings.chromaBufferOffset = buffer.ulChrominanceFrameBufferBlockOffset;
            createSettings.pitch = buffer.ulStripedWidth;
            createSettings.imageWidth = buffer.ulImageWidth;
            createSettings.imageHeight = buffer.ulImageHeight;
            createSettings.stripedWidth = buffer.ulStripedWidth;
            createSettings.lumaStripedHeight = buffer.ulLumaStripedHeight;
            createSettings.chromaStripedHeight = buffer.ulChromaStripedHeight;
            if (buffer.eBitDepth == BAVC_VideoBitDepth_e10Bit) {
                createSettings.lumaPixelFormat = NEXUS_PixelFormat_eY10;
                createSettings.chromaPixelFormat = NEXUS_PixelFormat_eCb10_Cr10;
            }
            else {
                createSettings.lumaPixelFormat = NEXUS_PixelFormat_eY8;
                createSettings.chromaPixelFormat = NEXUS_PixelFormat_eCb8_Cr8;
            }
            BDBG_CWARNING(BXVD_BufferType_eMax == (BXVD_BufferType)NEXUS_VideoBufferType_eMax);
            createSettings.bufferType = buffer.eBufferType;

            stillDecoder->stripedSurface.handle = NEXUS_StripedSurface_Create(&createSettings);
            if(stillDecoder->stripedSurface.handle==NULL) {
                nexus_still_decoder_p_free_block(stillDecoder, createSettings.lumaBuffer);
                nexus_still_decoder_p_free_block(stillDecoder, createSettings.chromaBuffer);
                return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            }
            stillDecoder->lumaBlock = createSettings.lumaBuffer;
            stillDecoder->chromaBlock = createSettings.chromaBuffer;
        }

        *pStripedSurface = stillDecoder->stripedSurface.handle;
        return NEXUS_SUCCESS;
    }
    return BERR_TRACE(NEXUS_UNKNOWN);
}

static void NEXUS_StillDecoder_P_ReleaseStripedSurface( NEXUS_HwStillDecoderHandle stillDecoder, NEXUS_StripedSurfaceHandle stripedSurface )
{
    if (stillDecoder->buffer.block) {
        (void)nexus_stilldecoder_p_outputbuffer(stillDecoder, NULL);
    }

    if (stillDecoder->stripedSurface.handle) {
        /* allow internal null to return whatever is currently stored */
        if (!stripedSurface || stillDecoder->stripedSurface.handle == stripedSurface) {
            NEXUS_StripedSurface_Destroy(stillDecoder->stripedSurface.handle);
            stillDecoder->stripedSurface.handle = NULL;
        }
    }
    if (stillDecoder->lumaBlock) {
        nexus_still_decoder_p_free_block(stillDecoder, stillDecoder->lumaBlock);
        stillDecoder->lumaBlock = NULL;
    }
    if (stillDecoder->chromaBlock) {
        nexus_still_decoder_p_free_block(stillDecoder, stillDecoder->chromaBlock);
        stillDecoder->chromaBlock = NULL;
    }
}

void NEXUS_StillDecoder_P_ReleaseStripedSurface_Avd( NEXUS_StillDecoderHandle swStillDecoder, NEXUS_StripedSurfaceHandle stripedSurface )
{
    /* public API requires non-null handle */
    if (!stripedSurface || stripedSurface != swStillDecoder->hw->stripedSurface.handle) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    NEXUS_StillDecoder_P_ReleaseStripedSurface(swStillDecoder->hw, stripedSurface);
}
#endif /* NEXUS_NUM_STILL_DECODES */

void NEXUS_StillDecoder_GetDefaultMemoryRequest( NEXUS_StillDecoderMemoryRequest *pRequest )
{
    BKNI_Memset(pRequest, 0, sizeof(*pRequest));
    pRequest->width = 720;
    pRequest->height = 480;
    pRequest->codec = NEXUS_VideoCodec_eMpeg2;
}

NEXUS_Error NEXUS_StillDecoder_RequestMemorySize( NEXUS_StillDecoderHandle swStillDecoder, const NEXUS_StillDecoderMemoryRequest *pRequest, unsigned *pSize )
{
#if NEXUS_NUM_STILL_DECODES
    int rc;
    BXVD_ChannelSettings settings;
    BXVD_FWMemConfig config;
    BAVC_VideoCompressionStd stVideoCompressionList[1];
    NEXUS_HwStillDecoderHandle stillDecoder;

    BDBG_OBJECT_ASSERT(swStillDecoder, NEXUS_StillDecoder);
    stillDecoder = swStillDecoder->hw; /* does not have to be current */

    rc = BXVD_GetChannelDefaultSettings(stillDecoder->device->xvd, NEXUS_NUM_VIDEO_DECODERS, &settings);
    if (rc) return BERR_TRACE(rc);

    settings.eDecodeResolution = NEXUS_VideoDecoder_GetDecodeResolution_priv(pRequest->width, pRequest->height);
    /* hardcode NEXUS_TransportType_eTs because only DSS matters */
    stVideoCompressionList[0] = NEXUS_P_VideoCodec_ToMagnum(pRequest->codec, NEXUS_TransportType_eTs);
    settings.peVideoCmprStdList = stVideoCompressionList;
    settings.uiVideoCmprCount = 1;
    settings.eChannelMode = BXVD_ChannelMode_eStill;
    settings.b10BitBuffersEnable = (g_NEXUS_videoDecoderModuleSettings.stillMemory[stillDecoder->index].colorDepth >= 10);

    rc = BXVD_GetChannelMemoryConfig(stillDecoder->device->xvd, &settings, &config);
    if (rc) return BERR_TRACE(rc);

    *pSize = config.uiPictureHeapSize;
    return 0;
#else
    BSTD_UNUSED(swStillDecoder);
    BSTD_UNUSED(pRequest);
    BSTD_UNUSED(pSize);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}
