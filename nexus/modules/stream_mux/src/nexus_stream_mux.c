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
#include "nexus_stream_mux_module.h"
#include "priv/nexus_playpump_priv.h"
#include "nexus_video_encoder_output.h"
#include "priv/nexus_video_encoder_priv.h"
#include "priv/nexus_audio_mux_output_priv.h"
#include "priv/nexus_pid_channel_priv.h"

BDBG_MODULE(nexus_stream_mux);

#define BDBG_MSG_TRACE(x)   /* BDBG_MSG(x) */

NEXUS_StreamMux_P_State g_NEXUS_StreamMux_P_State;

void
NEXUS_StreamMuxModule_GetDefaultSettings( NEXUS_StreamMuxModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

NEXUS_ModuleHandle
NEXUS_StreamMuxModule_Init( const NEXUS_StreamMuxModuleSettings *pSettings)
{
    NEXUS_Error rc;

    NEXUS_ModuleSettings moduleSettings;
    BDBG_ASSERT(g_NEXUS_StreamMux_P_State.module==NULL);
    BDBG_ASSERT(pSettings->transport);
    BDBG_ASSERT(pSettings->videoEncoder);
    BDBG_ASSERT(pSettings->audio);
    BDBG_ASSERT(pSettings->core);
    BKNI_Memset(&g_NEXUS_StreamMux_P_State, 0, sizeof(g_NEXUS_StreamMux_P_State));
    g_NEXUS_StreamMux_P_State.config = *pSettings;
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eHigh;
    g_NEXUS_StreamMux_P_State.module = NEXUS_Module_Create("stream_mux", &moduleSettings);
    if(g_NEXUS_StreamMux_P_State.module == NULL) { rc = BERR_TRACE(BERR_OS_ERROR); goto error; }

    return g_NEXUS_StreamMux_P_State.module;

error:
    return NULL;
}

void
NEXUS_StreamMuxModule_Uninit(void)
{
    if(g_NEXUS_StreamMux_P_State.module==NULL) {return;}

    NEXUS_Module_Destroy(g_NEXUS_StreamMux_P_State.module);
    g_NEXUS_StreamMux_P_State.module = NULL;
    return;
}

void
NEXUS_StreamMux_GetDefaultCreateSettings( NEXUS_StreamMuxCreateSettings *pSettings )
{
    BMUXlib_TS_CreateSettings muxCreateSettings;
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BMUXlib_TS_GetDefaultCreateSettings(&muxCreateSettings);
    pSettings->memoryConfiguration.systemBufferSize = muxCreateSettings.stMemoryConfig.uiSystemBufferSize;
    pSettings->memoryConfiguration.sharedBufferSize = muxCreateSettings.stMemoryConfig.uiSharedBufferSize;
    NEXUS_CallbackDesc_Init(&pSettings->finished);
    return;
}

static void
NEXUS_P_StreamMux_ManagedPtr_Init(NEXUS_P_StreamMux_ManagedPtr *managed)
{
    managed->block = NULL;
    managed->ptr = NULL;
    return;
}

static void
NEXUS_P_StreamMux_ManagedPtr_Release(NEXUS_P_StreamMux_ManagedPtr *managed)
{
    if(managed->ptr) {
        BDBG_ASSERT(managed->block);
        NEXUS_MemoryBlock_Unlock(managed->block);
        managed->ptr = NULL;
        managed->block = NULL;
    }
    return;
}

NEXUS_StreamMuxHandle
NEXUS_StreamMux_Create( const NEXUS_StreamMuxCreateSettings *pSettings )
{
    NEXUS_StreamMuxHandle  mux;
    NEXUS_StreamMuxCreateSettings createSettings;
    NEXUS_Error rc;
    BMUXlib_TS_CreateSettings muxCreateSettings;
    NEXUS_TsMuxSettings tsMuxSettings;
    unsigned i;

    if(pSettings==NULL) {
        NEXUS_StreamMux_GetDefaultCreateSettings(&createSettings);
        pSettings = &createSettings;
    }
    mux = BKNI_Malloc(sizeof(*mux));
    if(mux==NULL) {rc=BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto error;}
    NEXUS_OBJECT_INIT(NEXUS_StreamMux, mux);
    mux->createSettings = *pSettings;
    mux->muxTimer = NULL;
    mux->started = false;
    for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        mux->videoState[i].mux = mux;
        NEXUS_P_StreamMux_ManagedPtr_Init(&mux->videoState[i].frame);
        NEXUS_P_StreamMux_ManagedPtr_Init(&mux->videoState[i].meta);
    }
    for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        mux->audioState[i].mux = mux;
        NEXUS_P_StreamMux_ManagedPtr_Init(&mux->audioState[i].frame);
        NEXUS_P_StreamMux_ManagedPtr_Init(&mux->audioState[i].meta);
    }

    mux->finishedCallback = NEXUS_TaskCallback_Create(mux, NULL);
    if(!mux->finishedCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    NEXUS_TaskCallback_Set(mux->finishedCallback, &pSettings->finished);

    NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
    NEXUS_TsMux_GetDefaultSettings_priv(&tsMuxSettings);
    mux->tsMux = NEXUS_TsMux_Create_priv();
    NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);
    if(mux->tsMux==NULL) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}

    BMUXlib_TS_GetDefaultCreateSettings(&muxCreateSettings);
    muxCreateSettings.hMma = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mma;
    muxCreateSettings.stMemoryConfig.uiSystemBufferSize = pSettings->memoryConfiguration.systemBufferSize;
    muxCreateSettings.stMemoryConfig.uiSharedBufferSize = pSettings->memoryConfiguration.sharedBufferSize;
    rc = BMUXlib_TS_Create(&mux->mux, &muxCreateSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    return mux;

error:
    return NULL;
}

static void
NEXUS_StreamMux_P_Finalizer(NEXUS_StreamMuxHandle mux)
{
    NEXUS_OBJECT_ASSERT(NEXUS_StreamMux, mux);
    if(mux->started) {
        NEXUS_StreamMux_Stop(mux);
    }
    NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
    NEXUS_TsMux_Destroy_priv(mux->tsMux);
    NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);
    BMUXlib_TS_Destroy(mux->mux);
    NEXUS_TaskCallback_Destroy(mux->finishedCallback);
    NEXUS_OBJECT_DESTROY(NEXUS_StreamMux, mux);
    BKNI_Free(mux);
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_StreamMux, NEXUS_StreamMux_Destroy);

void
NEXUS_StreamMux_GetDefaultStartSettings(NEXUS_StreamMuxStartSettings *pSettings)
{
    unsigned i;
    BMUXlib_TS_StartSettings *muxStartSettings = &g_NEXUS_StreamMux_P_State.functionData.NEXUS_StreamMux_GetDefaultStartSettings.muxStartSettings;

    g_NEXUS_StreamMux_P_State.functionData.NEXUS_StreamMux_GetDefaultStartSettings.cookie = NEXUS_StreamMux_GetDefaultStartSettings;

    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BMUXlib_TS_GetDefaultStartSettings(muxStartSettings);

    for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        pSettings->video[i].pesId = 0xE0;
        pSettings->video[i].pidChannelIndex = -1;/* default to -1 */
        pSettings->audio[i].pesId = 0xC0;
        pSettings->audio[i].pidChannelIndex = -1;/* default to -1 */
    }
    pSettings->latencyTolerance = muxStartSettings->uiServiceLatencyTolerance;
    pSettings->nonRealTime = false;
    pSettings->nonRealTimeRate = 2 * NEXUS_NORMAL_PLAY_SPEED;
    pSettings->servicePeriod = muxStartSettings->uiServicePeriod;
    pSettings->muxDelay = muxStartSettings->uiA2PDelay;
    pSettings->supportTts = muxStartSettings->bSupportTTS;
    pSettings->interleaveMode = muxStartSettings->eInterleaveMode;
    pSettings->insertPtsOnlyOnFirstKeyFrameOfSegment = muxStartSettings->bInsertPtsOnlyOnFirstKeyFrameOfSegment;
    pSettings->useInitialPts = muxStartSettings->stNonRealTimeSettings.bInitialVideoPTSValid;
    pSettings->initialPts = ( muxStartSettings->stNonRealTimeSettings.uiInitialVideoPTS >> 1 );
    BDBG_ASSERT(g_NEXUS_StreamMux_P_State.functionData.NEXUS_StreamMux_GetDefaultStartSettings.cookie == NEXUS_StreamMux_GetDefaultStartSettings);
    return;
}

static BERR_Code
NEXUS_StreamMux_P_AddTransportDescriptors(void *context, const BMUXlib_TS_TransportDescriptor *descriptors, size_t count, size_t *queuedCount)
{
    NEXUS_PlaypumpHandle playpump = context;
    unsigned i;
    NEXUS_Error rc;

    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(descriptors);
    BDBG_ASSERT(queuedCount);
    BDBG_MSG_TRACE(("AddTransportDescriptors: %#x count:%u", (unsigned)playpump, count));
    *queuedCount=0;
    for(;count>0;) {
        unsigned nconvert = count<=NEXUS_STREAM_MUX_P_MAX_DESCRIPTORS?count:NEXUS_STREAM_MUX_P_MAX_DESCRIPTORS;
        BPVRlib_Feed_ExtendedOffsetEntry *entries = g_NEXUS_StreamMux_P_State.entries;
        size_t nconsumed;
        for(i=0;i<nconvert;i++,entries++,descriptors++) {
            entries->baseEntry.offset = descriptors->uiBufferOffset;
            entries->baseEntry.len = descriptors->uiBufferLength;
            entries->flags = descriptors->stTsMuxDescriptorConfig;
        }
        NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
        rc = NEXUS_Playpump_AddExtendedOffsetEntries_priv(playpump, g_NEXUS_StreamMux_P_State.entries, nconvert, &nconsumed);
        NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);
        if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
        BDBG_ASSERT(nconsumed<=count);
        count -= nconsumed;
        *queuedCount += nconsumed;
        if(nconsumed!=nconvert) {
            break;
        }
    }
    BDBG_MSG_TRACE(("AddTransportDescriptors: %#x queue:%u", (unsigned)playpump, *queuedCount));
    return BERR_SUCCESS;
}

static BERR_Code
NEXUS_StreamMux_P_GetCompletedTransportDescriptors( void *context, size_t *completedCount)
{
    NEXUS_PlaypumpHandle playpump = context;
    BERR_Code rc;

    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(completedCount);
    NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
    rc = NEXUS_Playpump_GetCompleted_priv(playpump, completedCount);
    NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);
    BDBG_MSG_TRACE(("GetCompletedTransportDescriptors: %#x count:%u", (unsigned)playpump, *completedCount));
    return BERR_TRACE(rc);
}

static BERR_Code
NEXUS_StreamMux_P_GetVideoBufferDescriptors( void *context, const BAVC_VideoBufferDescriptor *descriptors0[], size_t *numDescriptors0, const BAVC_VideoBufferDescriptor *descriptors1[], size_t *numDescriptors1)
{
    BERR_Code rc;
    NEXUS_P_StreamMux_VideoEncoderState *state = context;

    NEXUS_ASSERT_MODULE();
    /* full validation is in the NEXUS_VideoEncoder_GetBuffer */
    NEXUS_ASSERT_STRUCTURE(NEXUS_VideoEncoderDescriptor, BAVC_VideoBufferDescriptor);
    rc = NEXUS_VideoEncoder_GetBuffer(state->videoEncoder, (const NEXUS_VideoEncoderDescriptor**)descriptors0, numDescriptors0, (const NEXUS_VideoEncoderDescriptor**)descriptors1, numDescriptors1);
    BDBG_MSG_TRACE(("GetVideoBufferDescriptors:%#x numDescriptors:%u:%u", (unsigned)context, *numDescriptors0, *numDescriptors1));
    return BERR_TRACE(rc);
}

static BERR_Code
NEXUS_StreamMux_P_ConsumeVideoBufferDescriptors( void *context, size_t numDescriptors)
{
    BERR_Code rc;
    NEXUS_P_StreamMux_VideoEncoderState *state = context;

    NEXUS_ASSERT_MODULE();
    rc = NEXUS_VideoEncoder_ReadComplete(state->videoEncoder, numDescriptors);
    return BERR_TRACE(rc);
}

static BERR_Code
NEXUS_StreamMux_P_GetVideoBufferStatus( void *context, BMUXlib_CompressedBufferStatus *status)
{
    NEXUS_P_StreamMux_VideoEncoderState *state = context;
    NEXUS_VideoEncoderStatus encoderStatus;

    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(status);
    NEXUS_VideoEncoder_GetBufferStatus_priv(state->videoEncoder, &encoderStatus);
    BKNI_Memset(status,0,sizeof(*status));
    status->hFrameBufferBlock = NEXUS_MemoryBlock_GetBlock_priv(encoderStatus.bufferBlock);
    status->hMetadataBufferBlock = NEXUS_MemoryBlock_GetBlock_priv(encoderStatus.metadataBufferBlock);

    return BERR_SUCCESS;
}

static BERR_Code
NEXUS_StreamMux_P_GetAudioBufferDescriptors(
   void *context,
   const BAVC_AudioBufferDescriptor *astDescriptors0[], /* Can be NULL if uiNumDescriptors0=0. */
   size_t *puiNumDescriptors0,
   const BAVC_AudioBufferDescriptor *astDescriptors1[], /* Needed to handle FIFO wrap. Can be NULL if uiNumDescriptors1=0. */
   size_t *puiNumDescriptors1
   )
{
    BERR_Code rc;
    NEXUS_P_StreamMux_AudioEncoderState *state = context;

    NEXUS_ASSERT_MODULE();
    NEXUS_ASSERT_STRUCTURE(NEXUS_AudioMuxOutputFrame, BAVC_AudioBufferDescriptor);
    rc = NEXUS_AudioMuxOutput_GetBuffer(state->audioMuxOutput, (const NEXUS_AudioMuxOutputFrame**)astDescriptors0, puiNumDescriptors0, (const NEXUS_AudioMuxOutputFrame**)astDescriptors1, puiNumDescriptors1);
    BDBG_MSG_TRACE(("GetAudioBufferDescriptors:%#x numDescriptors:%u:%u", (unsigned)context, *puiNumDescriptors0, *puiNumDescriptors1));
    return BERR_TRACE(rc);
}

static BERR_Code
NEXUS_StreamMux_P_ConsumeAudioBufferDescriptors(
   void *context,
   size_t uiNumDescriptors
   )
{
    BERR_Code rc;
    NEXUS_P_StreamMux_AudioEncoderState *state = context;

    NEXUS_ASSERT_MODULE();
    rc = NEXUS_AudioMuxOutput_ReadComplete(state->audioMuxOutput, uiNumDescriptors);
    return BERR_TRACE(rc);
}

static BERR_Code
NEXUS_StreamMux_P_GetAudioBufferStatus(
   void *context,
   BMUXlib_CompressedBufferStatus *status
   )
{
    NEXUS_AudioMuxOutputStatus encoderStatus;
    NEXUS_P_StreamMux_AudioEncoderState *state = context;

    NEXUS_ASSERT_MODULE();
    BDBG_ASSERT(status);
    NEXUS_AudioMuxOutput_GetBufferStatus_priv(state->audioMuxOutput, &encoderStatus);
    BKNI_Memset(status,0,sizeof(*status));
    status->hFrameBufferBlock = NEXUS_MemoryBlock_GetBlock_priv(encoderStatus.bufferBlock);
    status->hMetadataBufferBlock = NEXUS_MemoryBlock_GetBlock_priv(encoderStatus.metadataBufferBlock);
    return BERR_SUCCESS;
}

static BERR_Code
NEXUS_StreamMux_P_TS_GetTransportStatus( void *context, BMUXlib_TS_TransportStatus *status)
{
    NEXUS_StreamMuxHandle mux=context;
    NEXUS_TsMuxStatus tsMuxStatus;

    BDBG_OBJECT_ASSERT(mux, NEXUS_StreamMux);

    BKNI_Memset(status, 0, sizeof(*status));

    NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
    NEXUS_TsMux_GetStatus_priv(mux->tsMux, &tsMuxStatus);

    status->uiSTC = tsMuxStatus.uiSTC;
    status->uiESCR = tsMuxStatus.uiESCR;

    NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);
    return BERR_SUCCESS;
}


static BERR_Code
NEXUS_StreamMux_P_TS_GetTransportSettings(void *context, BMUXlib_TS_TransportSettings *settings)
{
    NEXUS_StreamMuxHandle mux=context;
    NEXUS_TsMuxSettings tsMuxSettings;

    BDBG_OBJECT_ASSERT(mux, NEXUS_StreamMux);
    BDBG_ASSERT(settings);
    BKNI_Memset(settings, 0, sizeof(*settings));

    NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
    NEXUS_TsMux_GetSettings_priv(mux->tsMux, &tsMuxSettings);
    settings->uiMuxDelay = tsMuxSettings.uiMuxDelay;
    settings->stNonRealTimeSettings.uiPacingCounter = tsMuxSettings.AFAPSettings.uiPacingCounter;
    settings->bNonRealTimeMode = tsMuxSettings.bAFAPMode;

    NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);
    return BERR_SUCCESS;
}

static BERR_Code
NEXUS_StreamMux_P_TS_SetTransportSettings( void *context, const BMUXlib_TS_TransportSettings *settings)
{
    NEXUS_StreamMuxHandle mux=context;
    NEXUS_TsMuxSettings tsMuxSettings;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(mux, NEXUS_StreamMux);
    BDBG_ASSERT(settings);

    NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
    NEXUS_TsMux_GetSettings_priv(mux->tsMux, &tsMuxSettings);
    tsMuxSettings.uiMuxDelay = settings->uiMuxDelay;
    tsMuxSettings.AFAPSettings.uiPacingCounter = settings->stNonRealTimeSettings.uiPacingCounter;
    tsMuxSettings.bAFAPMode = settings->bNonRealTimeMode;
    tsMuxSettings.AFAPSettings.bEnablePacingPause = settings->bNonRealTimeMode;
    tsMuxSettings.AFAPSettings.uiPacingSpeed = mux->startSettings.nonRealTimeRate / NEXUS_NORMAL_PLAY_SPEED;
    rc = NEXUS_TsMux_SetSettings_priv(mux->tsMux, &tsMuxSettings, mux->startSettings.stcChannel);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto done; }

done:
    NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);
    return rc;
}


#define B_CAPTURE_USER_DATA 0
#if B_CAPTURE_USER_DATA
#include <stdio.h>
#endif

static BERR_Code
NEXUS_StreamMux_P_TS_GetUserDataBuffer(void *pvContext, BMMA_Block_Handle *phBlock, size_t *puiBlockOffset0, size_t *puiLength0, size_t *puiBlockOffset1, size_t *puiLength1)
{
    BERR_Code rc;
    void *pBuffer0,*pBuffer1;
    NEXUS_MemoryBlockHandle block;

    rc = NEXUS_Message_GetBufferWithWrap(pvContext, (const void**)&pBuffer0, puiLength0, (const void**)&pBuffer1, puiLength1);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto error; }

    NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.core);
    if (*puiLength0) {
        unsigned offset;
        rc = NEXUS_MemoryBlock_BlockAndOffsetFromRange_priv(pBuffer0, *puiLength0, &block, &offset);
        if (!rc) {
            *puiBlockOffset0 = offset;
            *phBlock = NEXUS_MemoryBlock_GetBlock_priv(block);
        }
    }
    else {
        *phBlock = NULL;
        *puiBlockOffset0 = 0;
    }
    if (!rc ) {
        if (*puiLength1) {
            unsigned offset;
            BDBG_ASSERT(*phBlock); /* we never get length1 unless we got length0 and its phBlock */
            rc = NEXUS_MemoryBlock_BlockAndOffsetFromRange_priv(pBuffer1, *puiLength1, &block, &offset);
            if (!rc) {
                *puiBlockOffset1 = offset;
                if (*phBlock != NEXUS_MemoryBlock_GetBlock_priv(block)) {
                    rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                }
            }
        }
        else {
            *puiBlockOffset1 = 0;
        }
    }
    NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.core);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto error; }

#if B_CAPTURE_USER_DATA
    if(rc==NEXUS_SUCCESS && *puiLength0) {
        static int n=0;
        char fname[64];
        FILE *fout;
        BKNI_Snprintf(fname, sizeof(fname), "user_data_%u.ts", n);
        n++;
        fout = fopen(fname,"wb");
        if(fout) {
            fwrite(*pBuffer0, 1, *puiLength0, fout);
            if(*puiLength1) {
                fwrite(*pBuffer1, 1, *puiLength1, fout);
            }
            fclose(fout);
        }
    }
#endif /* B_CAPTURE_USER_DATA */
    return BERR_SUCCESS;
error:
   *puiLength0 = 0;
   *puiLength1 = 0;
   return rc;
}

static BERR_Code NEXUS_StreamMux_P_TS_ConsumeUserDataBuffer(void *pvContext, size_t uiNumBytesConsumed)
{
    return NEXUS_Message_ReadComplete(pvContext, uiNumBytesConsumed);
}



static NEXUS_Error
NEXUS_StreamMux_P_AddPid(BMUXlib_TS_StartSettings *muxStartSettings, NEXUS_PlaypumpHandle playpump, unsigned *channel, NEXUS_Timebase timebase, NEXUS_PidChannelHandle *pidChannel, uint16_t pesId, int pidChannelIndex, NEXUS_Playpump_OpenPidChannelSettings_priv *pidSettings_priv)
{
    NEXUS_PlaypumpSettings settings;
    BMUXlib_TS_TransportChannelInterface *ch;
    BERR_Code rc;

    if(*channel>=BMUXLIB_TS_MAX_TRANSPORT_INSTANCES) { return BERR_TRACE(NEXUS_NOT_SUPPORTED); }
    if(playpump==NULL) {return BERR_TRACE(NEXUS_INVALID_PARAMETER);}

    NEXUS_Playpump_GetSettings(playpump, &settings);
    if (settings.transportType != NEXUS_TransportType_eEs) {
       settings.transportType = pidChannel ? NEXUS_TransportType_eMpeg2Pes : NEXUS_TransportType_eTs;
    }
    settings.originalTransportType = settings.transportType;
    settings.timestamp.timebase = timebase;
    settings.timestamp.pacing = true;

    rc = NEXUS_Playpump_SetSettings(playpump, &settings);
    if(rc) {rc=BERR_TRACE(rc);goto error;}
    if(pidChannel) {
        NEXUS_PlaypumpOpenPidChannelSettings openSettings;
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&openSettings);
        openSettings.pidSettings.pidChannelIndex = pidChannelIndex;

        NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
        *pidChannel = NEXUS_Playpump_OpenPidChannel_priv(playpump, pesId, &openSettings, pidSettings_priv );
        NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);
        if(*pidChannel==NULL) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error; }

        NEXUS_OBJECT_REGISTER(NEXUS_PidChannel, *pidChannel, Open);
    }

    ch = muxStartSettings->transport.stChannelInterface+(*channel);
    if (settings.transportType != NEXUS_TransportType_eEs) {
       (*channel)++;
    }

    ch->fAddTransportDescriptors = NEXUS_StreamMux_P_AddTransportDescriptors;
    ch->fGetCompletedTransportDescriptors = NEXUS_StreamMux_P_GetCompletedTransportDescriptors;
    ch->pContext = playpump;
    return NEXUS_SUCCESS;

error:
    return rc;
}

static void
NEXUS_StreamMux_P_MuxTimer(void *context)
{
    NEXUS_StreamMuxHandle mux=context;
    BERR_Code rc;
    BMUXlib_DoMux_Status muxStatus;
    unsigned nextExecutionTime;

    BDBG_OBJECT_ASSERT(mux, NEXUS_StreamMux);

    mux->muxTimer = NULL;
    rc = BMUXlib_TS_DoMux(mux->mux, &muxStatus);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto error; }
    mux->completedDuration = muxStatus.uiCompletedDuration;
    nextExecutionTime = muxStatus.uiNextExecutionTime;
    BDBG_MSG(("MuxTimer:%p nextExecutionTime:%u state:%u", (void*)mux, nextExecutionTime, (unsigned)muxStatus.eState));
    if(muxStatus.eState!=BMUXlib_State_eFinished && muxStatus.eState!=BMUXlib_State_eStopped) {
        mux->muxTimer = NEXUS_ScheduleTimer(nextExecutionTime, NEXUS_StreamMux_P_MuxTimer, mux);
        if(mux->muxTimer==NULL) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error; }
    } else if(muxStatus.eState==BMUXlib_State_eFinished) {
        BDBG_MSG(("MuxTimer:%p finished", (void*)mux));
        NEXUS_TaskCallback_Fire(mux->finishedCallback);
    }

    return;
error:
    return;
}

static NEXUS_Error
NEXUS_StreamMux_P_ControlOnePlaypump(NEXUS_StreamMuxHandle mux, NEXUS_PlaypumpHandle playpump, bool start, bool es)
{
    NEXUS_Error rc;
    NEXUS_PlaypumpSettings settings;

    NEXUS_Playpump_GetSettings(playpump, &settings);
    if (settings.transportType == NEXUS_TransportType_eEs)
    {
       if(mux->mcpbStarted && start) return NEXUS_SUCCESS;
    }

    if(start) {
        NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
        rc = NEXUS_TsMux_AddPlaypump_priv(mux->tsMux, playpump);
        rc = BERR_TRACE(rc);
        if(rc==BERR_SUCCESS) {
            rc = NEXUS_Playpump_StartMuxInput_priv(playpump);
        }
        NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);
        mux->mcpbStarted = true;
        return BERR_TRACE(rc);
    } else {
        NEXUS_Playpump_Stop(playpump);
        NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
        NEXUS_TsMux_RemovePlaypump_priv(mux->tsMux, playpump);
        NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);
        if(es) {
            /* NEXUS_Playpump_ClosePidChannel will call nexus_unregister */
            NEXUS_Playpump_CloseAllPidChannels(playpump);
        }
        mux->mcpbStarted = false;
        return NEXUS_SUCCESS;
    }
}

static NEXUS_Error
NEXUS_StreamMux_P_ControlAllPlaypumps(NEXUS_StreamMuxHandle mux, const NEXUS_StreamMuxStartSettings *pSettings, bool start)
{
    unsigned i;
    NEXUS_Error rc;

    for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        if(pSettings->video[i].encoder==NULL) {
            break;
        }
        rc = NEXUS_StreamMux_P_ControlOnePlaypump(mux, pSettings->video[i].playpump,start,true);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    }
    for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        if(pSettings->audio[i].muxOutput==NULL) {
            break;
        }
        rc = NEXUS_StreamMux_P_ControlOnePlaypump(mux, pSettings->audio[i].playpump,start,true);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    }
    if(pSettings->pcr.playpump!=NULL) {
        rc = NEXUS_StreamMux_P_ControlOnePlaypump(mux, pSettings->pcr.playpump,start,false);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    }
    return NEXUS_SUCCESS;
error:
    return rc;
}


NEXUS_Error
NEXUS_StreamMux_Start( NEXUS_StreamMuxHandle mux, const NEXUS_StreamMuxStartSettings *pSettings, NEXUS_StreamMuxOutput *pMuxOutput)
{
    NEXUS_Error rc;
    unsigned channel=0;
    unsigned i;
    NEXUS_Timebase timebase = NEXUS_Timebase_eInvalid;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_TsMuxSettings tsMuxSettings;
    NEXUS_Playpump_OpenPidChannelSettings_priv pidSettings_priv;

    BDBG_OBJECT_ASSERT(mux, NEXUS_StreamMux);
    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pMuxOutput);

    BKNI_Memset(pMuxOutput, 0, sizeof(*pMuxOutput));
    if(pSettings->stcChannel==NULL) { rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;}

    NEXUS_StcChannel_GetSettings(pSettings->stcChannel, &stcSettings);
    timebase = stcSettings.timebase;
    if(!pSettings->nonRealTime) {/* NRT mode could share the decoder STC which should be in MOD300 mode */
        stcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;
    }
    rc = NEXUS_StcChannel_SetSettings(pSettings->stcChannel, &stcSettings);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
    NEXUS_Playpump_GetDefaultOpenPidChannelSettings_priv(&pidSettings_priv);
    NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);


    BMUXlib_TS_GetDefaultStartSettings(&mux->muxStartSettings);
    mux->muxStartSettings.uiServiceLatencyTolerance = pSettings->latencyTolerance;
    mux->muxStartSettings.transport.stDeviceInterface.pContext = mux;
    mux->muxStartSettings.transport.stDeviceInterface.fGetTransportSettings = NEXUS_StreamMux_P_TS_GetTransportSettings;
    mux->muxStartSettings.transport.stDeviceInterface.fSetTransportSettings = NEXUS_StreamMux_P_TS_SetTransportSettings;
    mux->muxStartSettings.transport.stDeviceInterface.fGetTransportStatus = NEXUS_StreamMux_P_TS_GetTransportStatus;
    mux->muxStartSettings.bNonRealTimeMode = pSettings->nonRealTime;
    mux->muxStartSettings.uiServicePeriod = pSettings->servicePeriod;
    mux->muxStartSettings.uiA2PDelay = pSettings->muxDelay;
    mux->muxStartSettings.bSupportTTS = pSettings->supportTts;
    mux->muxStartSettings.eInterleaveMode = pSettings->interleaveMode;
    mux->muxStartSettings.bInsertPtsOnlyOnFirstKeyFrameOfSegment = pSettings->insertPtsOnlyOnFirstKeyFrameOfSegment;
    mux->muxStartSettings.stNonRealTimeSettings.bInitialVideoPTSValid = pSettings->useInitialPts;
    mux->muxStartSettings.stNonRealTimeSettings.uiInitialVideoPTS = ( ( (uint64_t) pSettings->initialPts ) << 1 );

    channel=0;
    mux->muxStartSettings.uiNumValidVideoPIDs = 0;
    mux->muxStartSettings.uiNumValidAudioPIDs = 0;
    mux->muxStartSettings.uiNumValidUserdataPIDs = 0;

    {
#if BXPT_SW7425_4528_WORKAROUND
        bool remapping=false;
        uint16_t basePesId=0xC0;

        for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
            unsigned j;
            const NEXUS_StreamMuxAudioPid *audio= pSettings->audio+i;
            if(audio->muxOutput==NULL) {
                break;
            }
            for(j=0;j<i;j++) {
                if(pSettings->audio[j].pesId == audio->pesId) {
                    remapping = true;
                }
            }
        }
#endif
        for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
            const NEXUS_StreamMuxVideoPid *video = pSettings->video+i;
            if(video->encoder==NULL) {
                break;
            }
            if(i>=BMUXLIB_TS_MAX_VIDEO_PIDS) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error;}
            mux->muxStartSettings.video[i].uiTransportChannelIndex = channel;
            BDBG_MSG(("NEXUS_StreamMux_Start:%p using channel %u for video[%u]=%#x", (void*)mux, channel, i, video->pid));
            pidSettings_priv.tsPid = video->pid;
            rc = NEXUS_StreamMux_P_AddPid(&mux->muxStartSettings, video->playpump, &channel, timebase, &pMuxOutput->video[i], video->pesId, video->pidChannelIndex, &pidSettings_priv);
            if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
            mux->muxStartSettings.video[i].uiPID = video->pid;
            mux->muxStartSettings.video[i].uiPESStreamID = video->pesId;
            mux->videoState[i].videoEncoder = video->encoder;
            mux->muxStartSettings.video[i].stInputInterface.pContext = &mux->videoState[i];
            mux->muxStartSettings.video[i].stInputInterface.fGetBufferDescriptors = NEXUS_StreamMux_P_GetVideoBufferDescriptors;
            mux->muxStartSettings.video[i].stInputInterface.fConsumeBufferDescriptors = NEXUS_StreamMux_P_ConsumeVideoBufferDescriptors;
            mux->muxStartSettings.video[i].stInputInterface.fGetBufferStatus = NEXUS_StreamMux_P_GetVideoBufferStatus;
            {
               NEXUS_PidChannelStatus status;

               NEXUS_PidChannel_GetStatus(
                  pMuxOutput->video[i],
                  &status );

               mux->muxStartSettings.video[i].uiPIDChannelIndex = status.pidChannelIndex;
            }
            mux->muxStartSettings.uiNumValidVideoPIDs = i+1;
        }
        for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
            const NEXUS_StreamMuxAudioPid *audio= pSettings->audio+i;
            if(audio->muxOutput==NULL) {
                break;
            }
            if(i>=BMUXLIB_TS_MAX_AUDIO_PIDS) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error;}
            mux->muxStartSettings.audio[i].uiTransportChannelIndex = channel;
            BDBG_MSG(("NEXUS_StreamMux_Start:%p using channel %u for audio[%u]=%#x", (void*)mux, channel, i, audio->pid));
            pidSettings_priv.tsPid = audio->pid;
#if BXPT_SW7425_4528_WORKAROUND
            if(remapping) {
                pidSettings_priv.remapping = true;
                pidSettings_priv.remappedPesId = audio->pesId;
                rc = NEXUS_StreamMux_P_AddPid(&mux->muxStartSettings, audio->playpump, &channel, timebase, &pMuxOutput->audio[i], basePesId, audio->pidChannelIndex, &pidSettings_priv);
                mux->muxStartSettings.audio[i].uiPESStreamID = basePesId;/* to be remapped later by rave */
                basePesId++;
                pidSettings_priv.remapping = false; /* clear for consecutive users of pidSettings_priv */
            } else {
                rc = NEXUS_StreamMux_P_AddPid(&mux->muxStartSettings, audio->playpump, &channel, timebase, &pMuxOutput->audio[i], audio->pesId, audio->pidChannelIndex, &pidSettings_priv);
                mux->muxStartSettings.audio[i].uiPESStreamID = audio->pesId;
            }
#else
            rc = NEXUS_StreamMux_P_AddPid(&mux->muxStartSettings, audio->playpump, &channel, timebase, &pMuxOutput->audio[i], audio->pesId, audio->pidChannelIndex, &pidSettings_priv);
            mux->muxStartSettings.audio[i].uiPESStreamID = audio->pesId;
#endif
            if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
            mux->muxStartSettings.audio[i].uiPID = audio->pid;
            mux->audioState[i].audioMuxOutput = audio->muxOutput;
            mux->muxStartSettings.audio[i].stInputInterface.pContext = &mux->audioState[i];
            mux->muxStartSettings.audio[i].stInputInterface.fGetBufferDescriptors = NEXUS_StreamMux_P_GetAudioBufferDescriptors;
            mux->muxStartSettings.audio[i].stInputInterface.fConsumeBufferDescriptors = NEXUS_StreamMux_P_ConsumeAudioBufferDescriptors;
            mux->muxStartSettings.audio[i].stInputInterface.fGetBufferStatus = NEXUS_StreamMux_P_GetAudioBufferStatus;
            {
               NEXUS_PidChannelStatus status;

               NEXUS_PidChannel_GetStatus(
                  pMuxOutput->audio[i],
                  &status );

               mux->muxStartSettings.audio[i].uiPIDChannelIndex = status.pidChannelIndex;
            }
            mux->muxStartSettings.audio[i].bEnablePESPacking = audio->pesPacking;
            mux->muxStartSettings.uiNumValidAudioPIDs=i+1;
        }
    }

    if(pSettings->pcr.playpump!=NULL) {
        NEXUS_PlaypumpSettings settings;
        NEXUS_Playpump_GetSettings(pSettings->pcr.playpump, &settings);
        mux->muxStartSettings.stPCRData.uiTransportChannelIndex = channel;
        BDBG_MSG(("NEXUS_StreamMux_Start:%p using channel %u for pcr %#x", (void*)mux, channel,  pSettings->pcr.pid));
        pidSettings_priv.tsPid = 0;
        pidSettings_priv.preserveCC = true;
        rc = NEXUS_StreamMux_P_AddPid(&mux->muxStartSettings, pSettings->pcr.playpump, &channel, timebase, (settings.transportType == NEXUS_TransportType_eEs)?&pMuxOutput->pcr:NULL, 0, -1, &pidSettings_priv);
        if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
        mux->muxStartSettings.stPCRData.uiPID = pSettings->pcr.pid;
        mux->muxStartSettings.stPCRData.uiInterval = pSettings->pcr.interval;

        if (settings.transportType == NEXUS_TransportType_eEs) {
           NEXUS_PidChannelStatus status;

           NEXUS_PidChannel_GetStatus(
              pMuxOutput->pcr,
              &status );

           mux->muxStartSettings.stPCRData.uiPIDChannelIndex = status.pidChannelIndex;
        }
    }
    for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        if(pSettings->userdata[i].message==NULL) {
            break;
        }
        if(i>=BMUXLIB_TS_MAX_USERDATA_PIDS) {
            BDBG_ERR(("only %u userdata pids is supported", BMUXLIB_TS_MAX_USERDATA_PIDS));
            rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;
        }
        BDBG_MSG(("NEXUS_StreamMux_Start:%p using message %p for userdata %u", (void *)mux, (void *)pSettings->userdata[i].message, i));
        mux->muxStartSettings.userdata[i].stUserDataInterface.pContext = pSettings->userdata[i].message;
        mux->muxStartSettings.userdata[i].stUserDataInterface.fGetUserDataBuffer = NEXUS_StreamMux_P_TS_GetUserDataBuffer;
        mux->muxStartSettings.userdata[i].stUserDataInterface.fConsumeUserDataBuffer = NEXUS_StreamMux_P_TS_ConsumeUserDataBuffer;
        mux->muxStartSettings.uiNumValidUserdataPIDs = i+1;
    }

    mux->startSettings = *pSettings;

    rc = NEXUS_StreamMux_P_ControlAllPlaypumps(mux, &mux->startSettings, true);
    if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
    BXPT_TsMux_GetDefaultSettings(&tsMuxSettings);
    tsMuxSettings.bAFAPMode = pSettings->nonRealTime;
    rc = NEXUS_TsMux_SetSettings_priv(mux->tsMux, &tsMuxSettings, pSettings->stcChannel);
    NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto error; }


    rc = BMUXlib_TS_Start(mux->mux, &mux->muxStartSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    mux->muxTimer = NEXUS_ScheduleTimer(0, NEXUS_StreamMux_P_MuxTimer, mux);
    if(mux->muxTimer==NULL) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error; }
    mux->started = true;
    mux->completedDuration = 0;

    return NEXUS_SUCCESS;
error:
    return rc;
}

void NEXUS_StreamMux_GetSettings( NEXUS_StreamMuxHandle mux, NEXUS_StreamMuxSettings *pSettings)
{
   BERR_Code rc;
   BMUXlib_TS_MuxSettings stMuxSettings;
   unsigned i;

   BDBG_OBJECT_ASSERT(mux, NEXUS_StreamMux);
   BDBG_ASSERT(pSettings);

   BKNI_Memset(&stMuxSettings, 0, sizeof(stMuxSettings));
   rc = BMUXlib_TS_GetMuxSettings( mux->mux, &stMuxSettings);
   if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

   for ( i = 0; (i < NEXUS_MAX_MUX_PIDS) && (i < BMUXLIB_TS_MAX_VIDEO_PIDS); i++ ) {
      pSettings->enable.video[i] = stMuxSettings.stInputEnable.bVideo[i];
   }

   for ( i = 0; (i < NEXUS_MAX_MUX_PIDS) && (i < BMUXLIB_TS_MAX_AUDIO_PIDS); i++ ) {
      pSettings->enable.audio[i] = stMuxSettings.stInputEnable.bAudio[i];
   }

   return;

   error:
   return;
}

NEXUS_Error NEXUS_StreamMux_SetSettings( NEXUS_StreamMuxHandle mux, const NEXUS_StreamMuxSettings *pSettings)
{
   BERR_Code rc;
   BMUXlib_TS_MuxSettings stMuxSettings;
   unsigned i;

   BDBG_OBJECT_ASSERT(mux, NEXUS_StreamMux);
   BDBG_ASSERT(pSettings);

   rc = BMUXlib_TS_GetMuxSettings( mux->mux, &stMuxSettings);
   if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

   for ( i = 0; (i < NEXUS_MAX_MUX_PIDS) && (i < BMUXLIB_TS_MAX_VIDEO_PIDS); i++ ) {
      stMuxSettings.stInputEnable.bVideo[i] = pSettings->enable.video[i];
   }

   for ( i = 0; (i < NEXUS_MAX_MUX_PIDS) && (i < BMUXLIB_TS_MAX_AUDIO_PIDS); i++ ) {
      stMuxSettings.stInputEnable.bAudio[i] = pSettings->enable.audio[i];
   }

   rc = BMUXlib_TS_SetMuxSettings( mux->mux, &stMuxSettings);
   if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

   return NEXUS_SUCCESS;

   error:
   return rc;
}

void
NEXUS_StreamMux_Finish(NEXUS_StreamMuxHandle mux)
{
    BERR_Code rc;
    BMUXlib_TS_FinishSettings muxFinishSettings;

    BDBG_OBJECT_ASSERT(mux, NEXUS_StreamMux);
    BMUXlib_TS_GetDefaultFinishSettings(&muxFinishSettings);
    rc = BMUXlib_TS_Finish(mux->mux, &muxFinishSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    return;

error:
    return;
}

void
NEXUS_StreamMux_Stop(NEXUS_StreamMuxHandle mux)
{
    BERR_Code rc;
    unsigned i;

    BDBG_OBJECT_ASSERT(mux, NEXUS_StreamMux);
    if(!mux->started) {
        return;
    }
    rc = BMUXlib_TS_Stop(mux->mux);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    if(mux->muxTimer) {
        NEXUS_CancelTimer(mux->muxTimer);
        mux->muxTimer = NULL;
    }
    rc = NEXUS_StreamMux_P_ControlAllPlaypumps(mux, &mux->startSettings, false);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.transport);
    NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.transport);

    for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        NEXUS_P_StreamMux_ManagedPtr_Release(&mux->videoState[i].frame);
        NEXUS_P_StreamMux_ManagedPtr_Release(&mux->videoState[i].meta);
    }
    for(i=0;i<NEXUS_MAX_MUX_PIDS;i++) {
        NEXUS_P_StreamMux_ManagedPtr_Release(&mux->audioState[i].frame);
        NEXUS_P_StreamMux_ManagedPtr_Release(&mux->audioState[i].meta);
    }

    mux->started = false;
    return;

error:
    return;
}

void
NEXUS_StreamMux_GetDefaultSystemData(NEXUS_StreamMuxSystemData *pSystemDataBuffer)
{
    BDBG_ASSERT(pSystemDataBuffer);
    BKNI_Memset(pSystemDataBuffer, 0, sizeof(*pSystemDataBuffer));
    return;
}

NEXUS_Error
NEXUS_StreamMux_AddSystemDataBuffer(NEXUS_StreamMuxHandle mux, const NEXUS_StreamMuxSystemData *pSystemDataBuffer)
{
    BERR_Code rc;
    size_t queuedCount;
    BMUXlib_TS_SystemData astSystemDataBuffer[1];
    NEXUS_MemoryBlockHandle block;
    unsigned offset;

    BDBG_OBJECT_ASSERT(mux, NEXUS_StreamMux);
    BDBG_ASSERT(pSystemDataBuffer);

    BKNI_Memset( &astSystemDataBuffer[0], 0, sizeof( BMUXlib_TS_SystemData ) );
    astSystemDataBuffer[0].uiTimestampDelta = pSystemDataBuffer->timestampDelta;
    astSystemDataBuffer[0].uiSize = pSystemDataBuffer->size;

    NEXUS_Module_Lock(g_NEXUS_StreamMux_P_State.config.core);
    rc = NEXUS_MemoryBlock_BlockAndOffsetFromRange_priv((void*)pSystemDataBuffer->pData, pSystemDataBuffer->size, &block, &offset);
    if (!rc )
    {
        astSystemDataBuffer[0].uiBlockOffset = offset;
        astSystemDataBuffer[0].hBlock = NEXUS_MemoryBlock_GetBlock_priv(block);
    }
    NEXUS_Module_Unlock(g_NEXUS_StreamMux_P_State.config.core);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto error; }

    queuedCount = 0;
    rc = BMUXlib_TS_AddSystemDataBuffers(mux->mux, astSystemDataBuffer, 1, &queuedCount);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    if(queuedCount==0) {rc=BERR_TRACE(NEXUS_NOT_AVAILABLE); goto error;}

    return NEXUS_SUCCESS;
error:
    return rc;
}

void
NEXUS_StreamMux_GetCompletedSystemDataBuffers(NEXUS_StreamMuxHandle mux, size_t *pCompletedCount)
{
    BERR_Code rc;


    BDBG_OBJECT_ASSERT(mux, NEXUS_StreamMux);
    BDBG_ASSERT(pCompletedCount);

    rc = BMUXlib_TS_GetCompletedSystemDataBuffers(mux->mux, pCompletedCount);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}

    return;

error:
    return;
}

NEXUS_Error
NEXUS_StreamMux_GetStatus( NEXUS_StreamMuxHandle mux, NEXUS_StreamMuxStatus *pStatus )
{
    BMUXlib_TS_Status status;
    unsigned i;

    BDBG_OBJECT_ASSERT(mux, NEXUS_StreamMux);
    BDBG_ASSERT(pStatus);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->duration = mux->completedDuration;

    BMUXlib_TS_GetStatus(mux->mux, &status);
    for ( i = 0; (i < BMUXLIB_TS_MAX_VIDEO_PIDS) && (i < NEXUS_MAX_MUX_PIDS); i++ )
    {
       pStatus->video.pid[i].currentTimestamp = status.stVideo[i].uiCurrentTimestamp/2;
    }
    for ( i = 0; (i < BMUXLIB_TS_MAX_AUDIO_PIDS) && (i < NEXUS_MAX_MUX_PIDS); i++ )
    {
       pStatus->audio.pid[i].currentTimestamp = status.stAudio[i].uiCurrentTimestamp/2;
    }
    pStatus->systemdata.pid[0].currentTimestamp = status.stSystem.uiCurrentTimestamp/2;
    for ( i = 0; (i < BMUXLIB_TS_MAX_USERDATA_PIDS) && (i < NEXUS_MAX_MUX_PIDS); i++ )
    {
       pStatus->userdata.pid[i].currentTimestamp = status.stUserData[i].uiCurrentTimestamp/2;
    }
    pStatus->video.averageBitrate = status.stAverageBitrate.uiVideo;
    pStatus->audio.averageBitrate = status.stAverageBitrate.uiAudio;
    pStatus->systemdata.averageBitrate = status.stAverageBitrate.uiSystemData;
    pStatus->userdata.averageBitrate = status.stAverageBitrate.uiUserData;
    pStatus->efficiency = status.uiEfficiency;
    pStatus->totalBytes = status.uiTotalBytes;

    return NEXUS_SUCCESS;
}

void NEXUS_StreamMux_GetDefaultConfiguration(NEXUS_StreamMuxConfiguration *pConfiguration)
{
    BMUXlib_TS_StartSettings *startSettings = &g_NEXUS_StreamMux_P_State.functionData.NEXUS_StreamMux_GetDefaultConfiguration.startSettings;
    BMUXlib_TS_MuxSettings *muxSettings = &g_NEXUS_StreamMux_P_State.functionData.NEXUS_StreamMux_GetDefaultConfiguration.muxSettings;

    g_NEXUS_StreamMux_P_State.functionData.NEXUS_StreamMux_GetDefaultConfiguration.cookie = NEXUS_StreamMux_GetDefaultConfiguration;

    BMUXlib_TS_GetDefaultStartSettings(startSettings);
    BMUXlib_TS_GetDefaultMuxSettings(muxSettings);
    BKNI_Memset(pConfiguration, 0, sizeof(*pConfiguration));
    pConfiguration->servicePeriod = startSettings->uiServicePeriod;
    pConfiguration->latencyTolerance = startSettings->uiServiceLatencyTolerance;
    pConfiguration->nonRealTime = startSettings->bNonRealTimeMode;
    pConfiguration->videoPids = 1;
    pConfiguration->audioPids = 1;
    pConfiguration->userDataPids = 0;
    pConfiguration->pcrPidinterval = startSettings->stPCRData.uiInterval;
    pConfiguration->systemDataBitRate = muxSettings->uiSystemDataBitRate;
    pConfiguration->muxDelay = startSettings->uiA2PDelay;
    pConfiguration->supportTts = startSettings->bSupportTTS;

    BDBG_ASSERT(g_NEXUS_StreamMux_P_State.functionData.NEXUS_StreamMux_GetDefaultConfiguration.cookie == NEXUS_StreamMux_GetDefaultConfiguration);
    return;
}

void NEXUS_StreamMux_GetMemoryConfiguration(const NEXUS_StreamMuxConfiguration *pConfiguration, NEXUS_StreamMuxMemoryConfiguration *pMemoryConfiguration)
{
    BMUXlib_TS_MuxConfig *muxConfig = &g_NEXUS_StreamMux_P_State.functionData.NEXUS_StreamMux_GetMemoryConfiguration.muxConfig;
    BMUXlib_TS_MemoryConfig *memoryConfig = &g_NEXUS_StreamMux_P_State.functionData.NEXUS_StreamMux_GetMemoryConfiguration.memoryConfig;

    g_NEXUS_StreamMux_P_State.functionData.NEXUS_StreamMux_GetMemoryConfiguration.cookie = NEXUS_StreamMux_GetMemoryConfiguration;
    BDBG_ASSERT(pConfiguration);
    BDBG_ASSERT(pMemoryConfiguration);
    BMUXlib_TS_GetDefaultStartSettings(&muxConfig->stMuxStartSettings);
    BMUXlib_TS_GetDefaultMuxSettings(&muxConfig->stMuxSettings);

    muxConfig->stMuxStartSettings.uiServicePeriod = pConfiguration->servicePeriod;
    muxConfig->stMuxStartSettings.uiServiceLatencyTolerance = pConfiguration->latencyTolerance;
    muxConfig->stMuxStartSettings.bNonRealTimeMode = pConfiguration->nonRealTime;
    muxConfig->stMuxStartSettings.uiNumValidVideoPIDs = pConfiguration->videoPids;
    muxConfig->stMuxStartSettings.uiNumValidAudioPIDs = pConfiguration->audioPids;
    muxConfig->stMuxStartSettings.uiNumValidUserdataPIDs = pConfiguration->userDataPids;
    muxConfig->stMuxStartSettings.stPCRData.uiInterval = pConfiguration->pcrPidinterval;
    muxConfig->stMuxStartSettings.uiA2PDelay = pConfiguration->muxDelay;
    muxConfig->stMuxStartSettings.bSupportTTS = pConfiguration->supportTts;
    muxConfig->stMuxSettings.uiSystemDataBitRate = pConfiguration->systemDataBitRate;
    BMUXlib_TS_GetMemoryConfig(muxConfig, memoryConfig);
    BKNI_Memset(pMemoryConfiguration, 0, sizeof(*pMemoryConfiguration));
    pMemoryConfiguration->systemBufferSize = memoryConfig->uiSystemBufferSize;
    pMemoryConfiguration->sharedBufferSize = memoryConfig->uiSharedBufferSize;
    BDBG_ASSERT(g_NEXUS_StreamMux_P_State.functionData.NEXUS_StreamMux_GetMemoryConfiguration.cookie == NEXUS_StreamMux_GetMemoryConfiguration);
    return;
}

