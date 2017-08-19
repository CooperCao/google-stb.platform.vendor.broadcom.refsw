/******************************************************************************
 * Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#include "nexus_video_encoder_module.h"
#include "nexus_power_management.h"

#include "priv/nexus_stc_channel_priv.h"
#include "priv/nexus_display_priv.h"
#include "priv/nexus_core_img_id.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_video_encoder);
BDBG_FILE_MODULE(nexus_video_encoder_status);

NEXUS_VideoEncoder_P_State g_NEXUS_VideoEncoder_P_State;

#if NEXUS_NUM_DSP_VIDEO_ENCODERS

#define LOCK_TRANSPORT()    NEXUS_Module_Lock(g_NEXUS_VideoEncoder_P_State.config.transport)
#define UNLOCK_TRANSPORT()  NEXUS_Module_Unlock(g_NEXUS_VideoEncoder_P_State.config.transport)
#define LOCK_DISPLAY()    NEXUS_Module_Lock(g_NEXUS_VideoEncoder_P_State.config.display)
#define UNLOCK_DISPLAY()  NEXUS_Module_Unlock(g_NEXUS_VideoEncoder_P_State.config.display)
#define LOCK_AUDIO()    NEXUS_Module_Lock(g_NEXUS_VideoEncoder_P_State.config.audio)
#define UNLOCK_AUDIO()  NEXUS_Module_Unlock(g_NEXUS_VideoEncoder_P_State.config.audio)

#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
static NEXUS_Error NEXUS_VideoEncoder_P_EnqueueCb_isr(void * context, BAVC_EncodePictureBuffer *picture);
static NEXUS_Error NEXUS_VideoEncoder_P_DequeueCb_isr(void * context, BAVC_EncodePictureBuffer *picture);
#else
static NEXUS_Error NEXUS_VideoEncoder_P_EnqueueCb_isr(void * context, NEXUS_DisplayCapturedImage *image);
static NEXUS_Error NEXUS_VideoEncoder_P_DequeueCb_isr(void * context, NEXUS_DisplayCapturedImage *image);
#endif

static void NEXUS_VideoEncoder_P_Watchdog(void *context)
{
    unsigned i;
    NEXUS_VideoEncoder_P_Device *device = (NEXUS_VideoEncoder_P_Device *)context;
    NEXUS_VideoEncoderHandle encoder;
    bool restart_encode[NEXUS_NUM_DSP_VIDEO_ENCODERS];

    BDBG_WRN(("NEXUS_VideoEncoder_P_Watchdog"));

    for (i=0; i<NEXUS_NUM_DSP_VIDEO_ENCODERS; i++) {
        encoder = &device->channels[i];
        restart_encode[i] = encoder->started;

        if(encoder->encoder && restart_encode[i]) {
            NEXUS_VideoEncoder_Stop(encoder, NULL);
        }
    }

    NEXUS_DspVideoEncoder_Watchdog_priv();

    for (i=0; i<NEXUS_NUM_DSP_VIDEO_ENCODERS; i++) {
        encoder = &device->channels[i];

        if(encoder->encoder && restart_encode[i]) {
            NEXUS_VideoEncoder_Start(encoder, &encoder->startSettings);
        }
    }
}

void
NEXUS_VideoEncoderModule_GetDefaultSettings( NEXUS_VideoEncoderModuleSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

void
NEXUS_VideoEncoderModule_GetDefaultInternalSettings( NEXUS_VideoEncoderModuleInternalSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    return;
}

NEXUS_ModuleHandle
NEXUS_VideoEncoderModule_Init(
                            const NEXUS_VideoEncoderModuleInternalSettings *pInternalSettings,
                            const NEXUS_VideoEncoderModuleSettings *pSettings
                              )
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_VideoEncoder_P_Device *device = &g_NEXUS_VideoEncoder_P_State.device;
    NEXUS_CallbackDesc watchdog;

    BDBG_ASSERT(pSettings);

    BDBG_ASSERT(g_NEXUS_VideoEncoder_P_State.module==NULL);
    BDBG_ASSERT(pInternalSettings->display);
    BDBG_ASSERT(pInternalSettings->transport);
    BDBG_ASSERT(pInternalSettings->audio);
    BKNI_Memset(&g_NEXUS_VideoEncoder_P_State, 0, sizeof(g_NEXUS_VideoEncoder_P_State));
    g_NEXUS_VideoEncoder_P_State.config = *pInternalSettings;
    g_NEXUS_VideoEncoder_P_State.settings = *pSettings;

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow; /* encoder interface is slow */
    g_NEXUS_VideoEncoder_P_State.module = NEXUS_Module_Create("video_encoder", &moduleSettings);
    if(g_NEXUS_VideoEncoder_P_State.module == NULL) { BERR_TRACE(BERR_OS_ERROR); goto error; }

    NEXUS_CallbackHandler_Init(device->watchdogCallbackHandler, NEXUS_VideoEncoder_P_Watchdog, device);
    NEXUS_CallbackHandler_PrepareCallback(device->watchdogCallbackHandler, watchdog);
    NEXUS_DspVideoEncoder_SetWatchdogCallback_priv(&watchdog);

    return g_NEXUS_VideoEncoder_P_State.module;
error:
    return NULL;
}

void
NEXUS_VideoEncoderModule_Uninit(void)
{
    unsigned i;
    NEXUS_VideoEncoder_P_Device *device = &g_NEXUS_VideoEncoder_P_State.device;

    if(g_NEXUS_VideoEncoder_P_State.module==NULL) {return;}

    NEXUS_DspVideoEncoder_SetWatchdogCallback_priv(NULL);
    NEXUS_CallbackHandler_Shutdown(device->watchdogCallbackHandler);

    for(i=0;i<NEXUS_NUM_DSP_VIDEO_ENCODERS;i++ ) {
    if(device->channels[i].encoder) {
        NEXUS_VideoEncoder_Close(&device->channels[i]);
    }
    }
    NEXUS_Module_Destroy(g_NEXUS_VideoEncoder_P_State.module);
    g_NEXUS_VideoEncoder_P_State.module = NULL;
    return;
}

NEXUS_Error NEXUS_VideoEncoderModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);

    return NEXUS_SUCCESS;
}

void NEXUS_VideoEncoder_GetDefaultOpenSettings(NEXUS_VideoEncoderOpenSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_VideoEncoderHandle
NEXUS_VideoEncoder_Open(unsigned index, const NEXUS_VideoEncoderOpenSettings *pSettings)
{
    NEXUS_VideoEncoderOpenSettings settings;
    NEXUS_VideoEncoder_P_Device *device;
    NEXUS_VideoEncoderHandle encoder;
    NEXUS_RaveOpenSettings raveSettings;
    NEXUS_DspVideoEncoderOpenSettings dspOpenSettings;

    if(index>=NEXUS_NUM_DSP_VIDEO_ENCODERS) {BERR_TRACE(NEXUS_INVALID_PARAMETER);goto error;}

    device = &g_NEXUS_VideoEncoder_P_State.device;
    encoder = &device->channels[index];

    if(pSettings==NULL) {
        NEXUS_VideoEncoder_GetDefaultOpenSettings(&settings);
        pSettings=&settings;
    }

    NEXUS_OBJECT_SET(NEXUS_VideoEncoder, encoder);
    encoder->openSettings = *pSettings;
    encoder->device = device;
    encoder->index = index;

    /* Setup rave buffer */
    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultOpenSettings_priv(&raveSettings);
    UNLOCK_TRANSPORT();

    LOCK_AUDIO();
    NEXUS_DspVideoEncoder_GetRaveSettings_priv(&raveSettings);
    UNLOCK_AUDIO();

    if ( 0 != pSettings->data.fifoSize )
    {
        raveSettings.config.Cdb.Length = pSettings->data.fifoSize;
    }
    if ( 0 != pSettings->index.fifoSize )
    {
        raveSettings.config.Itb.Length = pSettings->index.fifoSize;
    }

    raveSettings.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;

    LOCK_TRANSPORT();
    encoder->raveContext = NEXUS_Rave_Open_priv(&raveSettings);
    UNLOCK_TRANSPORT();
    if ( NULL == encoder->raveContext )
    {
        (void)BERR_TRACE(BERR_UNKNOWN);
        goto error;
    }

    LOCK_AUDIO();
    NEXUS_DspVideoEncoder_GetDefaultOpenSettings_priv(&dspOpenSettings);
    encoder->encoder = NEXUS_DspVideoEncoder_Open_priv(index, &dspOpenSettings);
    UNLOCK_AUDIO();
    if(!encoder->encoder) {BERR_TRACE(NEXUS_NOT_SUPPORTED);goto error;}

    encoder->started = false;
    NEXUS_VideoEncoder_GetDefaultSettings(&encoder->settings);

    return encoder;

error:
    return NULL;
}

static void
NEXUS_VideoEncoder_P_Release(NEXUS_VideoEncoderHandle encoder)
{
    NEXUS_OBJECT_ASSERT(NEXUS_VideoEncoder, encoder);

    LOCK_AUDIO();
    NEXUS_DspVideoEncoder_Release_priv(encoder->encoder);
    UNLOCK_AUDIO();

    return;
}

static void
NEXUS_VideoEncoder_P_Finalizer(NEXUS_VideoEncoderHandle encoder)
{
    NEXUS_OBJECT_ASSERT(NEXUS_VideoEncoder, encoder);

    LOCK_AUDIO();
    NEXUS_DspVideoEncoder_Close_priv(encoder->encoder);
    UNLOCK_AUDIO();
    encoder->encoder = NULL;

    LOCK_TRANSPORT();
    NEXUS_Rave_Close_priv(encoder->raveContext);
    UNLOCK_TRANSPORT();

    NEXUS_OBJECT_UNSET(NEXUS_VideoEncoder, encoder);
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_VideoEncoder, NEXUS_VideoEncoder_Close);


void
NEXUS_VideoEncoder_GetDefaultStartSettings(NEXUS_VideoEncoderStartSettings *pSettings )
{
    NEXUS_DspVideoEncoderStartSettings startSettings;

    LOCK_AUDIO();
    NEXUS_DspVideoEncoder_GetDefaultStartSettings_priv(&startSettings);
    UNLOCK_AUDIO();

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    pSettings->nonRealTime = startSettings.nonRealTime;
    pSettings->codec = startSettings.codec;
    pSettings->profile = startSettings.profile;
    pSettings->level = startSettings.level;
    pSettings->bounds.outputFrameRate.min = startSettings.framerate;
    pSettings->bounds.outputFrameRate.max = startSettings.framerate;
    pSettings->bounds.inputDimension.max.width = startSettings.width;
    pSettings->bounds.inputDimension.max.height = startSettings.height;
    pSettings->encodeUserData = false;

    return;
}

void NEXUS_VideoEncoder_GetDefaultSettings(NEXUS_VideoEncoderSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->bitrateMax = 400000;
    pSettings->frameRate = NEXUS_VideoFrameRate_e29_97;
    /* Other settings are not used currently */
}

#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
NEXUS_Error NEXUS_VideoEncoder_P_EnqueueCb_isr(void * context, BAVC_EncodePictureBuffer *picture)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_VideoEncoderHandle encoder = (NEXUS_VideoEncoderHandle) context;

    BDBG_MSG(("Enqueue Picture %p %u", (void *)picture->hLumaBlock, picture->ulPictureId));
    if(encoder->started)
    {
        rc = NEXUS_DspVideoEncoder_EnqueuePicture_isr(encoder->encoder, picture);
    }
    return rc;
}
NEXUS_Error NEXUS_VideoEncoder_P_DequeueCb_isr(void * context, BAVC_EncodePictureBuffer *picture)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_VideoEncoderHandle encoder = (NEXUS_VideoEncoderHandle) context;

    if(encoder->started) {
        rc = NEXUS_DspVideoEncoder_DequeuePicture_isr(encoder->encoder, picture);
    } else {
        picture->hLumaBlock = NULL ;
        picture->ulPictureId = 0 ;
    }

    BDBG_MSG(("Dequeue Picture %p %u", (void *)picture->hLumaBlock, picture->ulPictureId));
    return rc;
}
#else /* !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT */
NEXUS_Error
NEXUS_VideoEncoder_P_EnqueueCb_isr(void * context, NEXUS_DisplayCapturedImage *image)
{
    NEXUS_VideoEncoderHandle encoder = (NEXUS_VideoEncoderHandle) context;
    NEXUS_DspVideoEncoderPicture picture;
    NEXUS_Error rc = NEXUS_UNKNOWN;

    picture.hImage = image->hImage;
    picture.offset = image->offset;
    picture.width = image->width;
    picture.height = image->height;
    picture.polarity = image->polarity;
    picture.origPts = image->origPts;
    picture.framerate = image->framerate;
    picture.stallStc = image->stallStc;
    picture.ignorePicture = image->ignorePicture;
    picture.sarHorizontal = image->aspectRatioX;
    picture.sarVertical = image->aspectRatioY;
    BDBG_MSG(("Enqueue Picture %p:%#x", (void *)image->hImage, image->offset));

    if (encoder->started) {
        rc = NEXUS_DspVideoEncoder_EnqueuePicture_isr(encoder->encoder, &picture);
    }

    return rc;
}

NEXUS_Error
NEXUS_VideoEncoder_P_DequeueCb_isr(void * context, NEXUS_DisplayCapturedImage *image)
{
    NEXUS_VideoEncoderHandle encoder = (NEXUS_VideoEncoderHandle) context;
    NEXUS_DspVideoEncoderPicture picture;
    NEXUS_Error rc = NEXUS_UNKNOWN;

    if (encoder->started) {
        rc = NEXUS_DspVideoEncoder_DequeuePicture_isr(encoder->encoder, &picture);

        image->hImage = picture.hImage ;
        image->offset = picture.offset ;
    } else {
        image->hImage = NULL ;
        image->offset = 0 ;
    }

    BDBG_MSG(("Dequeue Picture %p:%#x", (void *)image->hImage, image->offset));
    return rc;
}
#endif /* NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT */

#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
static void NEXUS_VideoEncoder_P_CloseStcSnapshot(
    NEXUS_VideoEncoderHandle encoder)
{
    if (encoder->snapshot)
    {
        LOCK_TRANSPORT();
        NEXUS_StcChannel_CloseSnapshot_priv(encoder->snapshot);
        UNLOCK_TRANSPORT();
    }
}

static NEXUS_Error NEXUS_VideoEncoder_P_OpenStcSnapshot(
    NEXUS_VideoEncoderHandle encoder,
    const NEXUS_VideoEncoderStartSettings *pSettings,
    NEXUS_DisplayEncoderSettings * pDisplaySettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelSnapshotSettings snapshotSettings;
    NEXUS_StcChannelSnapshotStatus snapshotStatus;
    unsigned stgIndex;

    LOCK_DISPLAY();
    stgIndex = NEXUS_Display_GetStgIndex_priv(pSettings->input);
    UNLOCK_DISPLAY();

    LOCK_TRANSPORT();
    encoder->snapshot = NEXUS_StcChannel_OpenSnapshot_priv(pSettings->stcChannel);
    if (!encoder->snapshot) {rc=BERR_TRACE(NEXUS_NOT_AVAILABLE); goto error;}

    NEXUS_StcChannel_GetSnapshotSettings_priv(encoder->snapshot, &snapshotSettings);
    snapshotSettings.triggerIndex = stgIndex;
    snapshotSettings.mode = NEXUS_StcChannelSnapshotMode_eLegacy;
    rc = NEXUS_StcChannel_SetSnapshotSettings_priv(encoder->snapshot, &snapshotSettings);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    rc = NEXUS_StcChannel_GetSnapshotStatus_priv(encoder->snapshot, &snapshotStatus);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    pDisplaySettings->stcSnapshotLoAddr = snapshotStatus.stcLoAddr;
    pDisplaySettings->stcSnapshotHiAddr = snapshotStatus.stcHiAddr;

    BDBG_MSG(("STCSNAPLO: %p; STCSNAPHI: %p", (void *)(pDisplaySettings->stcSnapshotLoAddr), (void *)(pDisplaySettings->stcSnapshotHiAddr)));

end:
    UNLOCK_TRANSPORT();
    return rc;

error:
    NEXUS_VideoEncoder_P_CloseStcSnapshot(encoder);
    goto end;
}
#endif /* NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT */

NEXUS_Error
NEXUS_VideoEncoder_Start(NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderStartSettings *pSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_DspVideoEncoderStartSettings startSettings;
    NEXUS_DisplayEncoderSettings displaySettings;

    LOCK_AUDIO();
    NEXUS_DspVideoEncoder_GetDefaultStartSettings_priv(&startSettings);
    NEXUS_DspVideoEncoder_GetUserDataSettings_priv(encoder->encoder, &startSettings.userDataSettings);
#if !NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    rc = NEXUS_DspVideoEncoder_GetExtInterruptInfo_priv(encoder->encoder, &startSettings.extIntInfo);
#endif
    UNLOCK_AUDIO();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}
    startSettings.nonRealTime = pSettings->nonRealTime;
    startSettings.codec = pSettings->codec;
    startSettings.profile = pSettings->profile;
    startSettings.level = pSettings->level;
    startSettings.width = pSettings->bounds.inputDimension.max.width;
    startSettings.height = pSettings->bounds.inputDimension.max.height;
    startSettings.stcChannel = pSettings->stcChannel;
    startSettings.framerate = encoder->settings.frameRate;
    startSettings.bitrate = encoder->settings.bitrateMax;
    startSettings.encoderDelay = encoder->settings.encoderDelay/27000; /* Convert from 27MHz to ms */
    startSettings.raveContext = encoder->raveContext;
    startSettings.userDataSettings.encodeUserData = pSettings->encodeUserData;

    BKNI_Memset(&displaySettings, 0, sizeof(NEXUS_DisplayEncoderSettings));
    displaySettings.enqueueCb_isr = NEXUS_VideoEncoder_P_EnqueueCb_isr;
    displaySettings.dequeueCb_isr = NEXUS_VideoEncoder_P_DequeueCb_isr;
    displaySettings.context = encoder;
    displaySettings.encodeRate = encoder->settings.frameRate;
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    rc = NEXUS_VideoEncoder_P_OpenStcSnapshot(encoder, pSettings, &displaySettings);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    displaySettings.vip.hHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mma;
    displaySettings.vip.stMemSettings.bSupportInterlaced = false;
    displaySettings.vip.stMemSettings.ulMemcId = 0; /* TODO: this is fishy */
    displaySettings.vip.stMemSettings.ulMaxHeight = pSettings->bounds.inputDimension.max.height;
    displaySettings.vip.stMemSettings.ulMaxWidth = pSettings->bounds.inputDimension.max.width;
#else
    displaySettings.extIntAddress = startSettings.extIntInfo.address;
    displaySettings.extIntBitNum = startSettings.extIntInfo.bit_num;
#endif

    LOCK_TRANSPORT();
    NEXUS_Rave_Disable_priv(encoder->raveContext);
    NEXUS_Rave_Flush_priv(encoder->raveContext);
    NEXUS_Rave_Enable_priv(encoder->raveContext);
    UNLOCK_TRANSPORT();

    LOCK_DISPLAY();
    rc = NEXUS_Display_SetEncoderCallback_priv(pSettings->input, pSettings->window, &displaySettings);
    UNLOCK_DISPLAY();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto connect_error;}

    LOCK_AUDIO();
    rc = NEXUS_DspVideoEncoder_Start_priv(encoder->encoder, &startSettings);
    UNLOCK_AUDIO();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto dsp_error;}
    BKNI_EnterCriticalSection();
    encoder->started = true;
    BKNI_LeaveCriticalSection();

    LOCK_DISPLAY();
    rc = NEXUS_Display_EnableEncoderCallback_priv(pSettings->input);
    UNLOCK_DISPLAY();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto display_error;}

    LOCK_DISPLAY();
    rc = NEXUS_DisplayModule_SetUserDataEncodeMode_priv(pSettings->input, pSettings->encodeUserData, &startSettings.userDataSettings.xudSettings, pSettings->window);
    UNLOCK_DISPLAY();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto userdata_error;}

    encoder->startSettings = *pSettings;

    return rc;

userdata_error:
    LOCK_DISPLAY();
    NEXUS_Display_SetEncoderCallback_priv(encoder->startSettings.input, pSettings->window, NULL);
    UNLOCK_DISPLAY();
display_error:
    BKNI_EnterCriticalSection();
    encoder->started = false;
    BKNI_LeaveCriticalSection();
    LOCK_AUDIO();
    NEXUS_DspVideoEncoder_Stop_priv(encoder->encoder);
    UNLOCK_AUDIO();
dsp_error:
    LOCK_DISPLAY();
    NEXUS_Display_DisableEncoderCallback_priv(pSettings->input);
    UNLOCK_DISPLAY();
connect_error:
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    NEXUS_VideoEncoder_P_CloseStcSnapshot(encoder);
#endif
error:
    return rc;
}

void NEXUS_VideoEncoder_GetDefaultStopSettings(NEXUS_VideoEncoderStopSettings *pSettings)
{
    BSTD_UNUSED(pSettings);

    return;
}

void
NEXUS_VideoEncoder_Stop( NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderStopSettings *pSettings)
{
    BERR_Code rc;

    BSTD_UNUSED(pSettings);

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);

    if(!encoder->started)
    return;

    LOCK_DISPLAY();
    rc = NEXUS_DisplayModule_SetUserDataEncodeMode_priv(encoder->startSettings.input, false, NULL, NULL);
    UNLOCK_DISPLAY();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}

    LOCK_DISPLAY();
    rc = NEXUS_Display_DisableEncoderCallback_priv(encoder->startSettings.input);
    UNLOCK_DISPLAY();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}

    LOCK_AUDIO();
    NEXUS_DspVideoEncoder_Stop_priv(encoder->encoder);
    UNLOCK_AUDIO();
    BKNI_EnterCriticalSection();
    encoder->started = false;
    BKNI_LeaveCriticalSection();

    LOCK_DISPLAY();
    rc = NEXUS_Display_SetEncoderCallback_priv(encoder->startSettings.input, encoder->startSettings.window, NULL);
    UNLOCK_DISPLAY();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}

#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    NEXUS_VideoEncoder_P_CloseStcSnapshot(encoder);
#endif

    return;
}

void
NEXUS_VideoEncoder_GetSettings(NEXUS_VideoEncoderHandle encoder, NEXUS_VideoEncoderSettings *pSettings)
{
    NEXUS_Error rc;
    NEXUS_DspVideoEncoderSettings dspSettings;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    BDBG_ASSERT(pSettings);

    *pSettings = encoder->settings;

    LOCK_AUDIO();
    rc = NEXUS_DspVideoEncoder_GetSettings_priv(encoder->encoder, &dspSettings);
    UNLOCK_AUDIO();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); return;}

    pSettings->bitrateMax = dspSettings.bitrate;
    pSettings->frameRate = dspSettings.frameRate;

    return;
}

NEXUS_Error
NEXUS_VideoEncoder_SetSettings( NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderSettings *pSettings)
{
    NEXUS_Error rc;
    NEXUS_DspVideoEncoderSettings dspSettings;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    BDBG_ASSERT(pSettings);

    dspSettings.bitrate = pSettings->bitrateMax;
    dspSettings.frameRate = pSettings->frameRate;

    LOCK_AUDIO();
    rc = NEXUS_DspVideoEncoder_SetSettings_priv(encoder->encoder, &dspSettings);
    UNLOCK_AUDIO();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); return rc;}

    encoder->settings = *pSettings;

    return rc;
}

NEXUS_Error
NEXUS_VideoEncoder_GetBuffer( NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderDescriptor **pBuffer, size_t *pSize, const NEXUS_VideoEncoderDescriptor **pBuffer2, size_t *pSize2)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pSize);
    BDBG_ASSERT(pBuffer2);
    BDBG_ASSERT(pSize2);

    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, flags, BAVC_VideoBufferDescriptor, stCommon.uiFlags);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, originalPts, BAVC_VideoBufferDescriptor, stCommon.uiOriginalPTS);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, pts, BAVC_VideoBufferDescriptor, stCommon.uiPTS);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, escr, BAVC_VideoBufferDescriptor, stCommon.uiESCR);

    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, ticksPerBit, BAVC_VideoBufferDescriptor, stCommon.uiTicksPerBit);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, shr, BAVC_VideoBufferDescriptor, stCommon.iSHR);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, offset, BAVC_VideoBufferDescriptor, stCommon.uiOffset);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, length, BAVC_VideoBufferDescriptor, stCommon.uiLength);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, dts, BAVC_VideoBufferDescriptor, uiDTS);
    NEXUS_ASSERT_FIELD(NEXUS_VideoEncoderDescriptor, dataUnitType, BAVC_VideoBufferDescriptor, uiDataUnitType);
    NEXUS_ASSERT_STRUCTURE(NEXUS_VideoEncoderDescriptor, BAVC_VideoBufferDescriptor);

    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_ORIGINALPTS_VALID ==  BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ORIGINALPTS_VALID);
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_PTS_VALID == BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID);
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_FRAME_START ==  BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_FRAME_START);
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EOS ==  BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_EOS );
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_METADATA == BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_METADATA );
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_DTS_VALID == BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DTS_VALID);
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_RAP == BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_RAP);
    BDBG_CASSERT(NEXUS_VIDEOENCODERDESCRIPTOR_VIDEOFLAG_DATA_UNIT_START == BAVC_VIDEOBUFFERDESCRIPTOR_FLAGS_DATA_UNIT_START);

    LOCK_AUDIO();
    rc = NEXUS_DspVideoEncoder_GetBuffer_priv(encoder->encoder, (void *)pBuffer, pSize, (void *)pBuffer2, pSize2);
    UNLOCK_AUDIO();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    return NEXUS_SUCCESS;
error:
    return rc;
}

NEXUS_Error
NEXUS_VideoEncoder_ReadComplete(NEXUS_VideoEncoderHandle encoder, unsigned descriptorsCompleted)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);

    LOCK_AUDIO();
    rc = NEXUS_DspVideoEncoder_ReadComplete_priv(encoder->encoder, descriptorsCompleted);
    UNLOCK_AUDIO();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    return NEXUS_SUCCESS;
error:
    return rc;
}

NEXUS_Error
NEXUS_VideoEncoder_GetStatus(NEXUS_VideoEncoderHandle encoder, NEXUS_VideoEncoderStatus *pStatus)
{
    return NEXUS_VideoEncoder_GetBufferStatus_priv(encoder, pStatus);
}

NEXUS_Error NEXUS_VideoEncoder_GetBufferBlocks_priv(NEXUS_VideoEncoderHandle encoder, BMMA_Block_Handle *phFrameBufferBlock, BMMA_Block_Handle *phMetadataBufferBlock)
{
    NEXUS_Error rc;
    LOCK_AUDIO();
    rc = NEXUS_DspVideoEncoder_GetBufferBlocks_priv(encoder->encoder, phFrameBufferBlock, phMetadataBufferBlock);
    UNLOCK_AUDIO();
    if(rc!=BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_VideoEncoder_GetBufferStatus_priv(NEXUS_VideoEncoderHandle encoder, NEXUS_VideoEncoderStatus *pStatus)
{
    NEXUS_Error rc;
    NEXUS_DspVideoEncoderStatus bufferStatus;

    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&bufferStatus, 0, sizeof(NEXUS_DspVideoEncoderStatus));

    LOCK_AUDIO();
    rc = NEXUS_DspVideoEncoder_GetStatus_priv(encoder->encoder, &bufferStatus);
    UNLOCK_AUDIO();
    if(rc!=BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }

    pStatus->bufferBlock = bufferStatus.bufferBlock;
    pStatus->metadataBufferBlock = bufferStatus.metadataBufferBlock;
    pStatus->picturesReceived = bufferStatus.picturesReceived;
    pStatus->picturesDroppedFRC = bufferStatus.picturesDroppedFRC;
    pStatus->picturesEncoded = bufferStatus.picturesEncoded;

    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_VideoEncoder_GetDelayRange (NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderSettings *pSettings, const NEXUS_VideoEncoderStartSettings *pStartSettings, NEXUS_VideoEncoderDelayRange *pDelayRange)
{
    NEXUS_Error rc;
    uint32_t delay;

    LOCK_AUDIO();
    rc = NEXUS_DspVideoEncoder_GetDelayRange_priv(encoder->encoder, pStartSettings->bounds.inputDimension.max.width, pStartSettings->bounds.inputDimension.max.height, pSettings->frameRate, pSettings->bitrateMax, &delay);
    UNLOCK_AUDIO();
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}

    /* Video encoder delay is in 27MHz ticks */
    pDelayRange->min = delay*27000;
    pDelayRange->max = delay*27000;

error:
    return rc;
}

void
NEXUS_VideoEncoder_GetSettingsOnInputChange(NEXUS_VideoEncoderHandle encoder, NEXUS_VideoEncoderSettingsOnInputChange *pSettings)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(pSettings);

    return;
}

NEXUS_Error
NEXUS_VideoEncoder_SetSettingsOnInputChange(NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderSettingsOnInputChange *pSettings)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(pSettings);

    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_VideoEncoder_InsertRandomAccessPoint(NEXUS_VideoEncoderHandle encoder)
{
    BSTD_UNUSED(encoder);

    return NEXUS_SUCCESS;
}

void
NEXUS_VideoEncoder_ClearStatus(NEXUS_VideoEncoderHandle handle, const NEXUS_VideoEncoderClearStatus *pClearStatus)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pClearStatus);

    return;
}

void
NEXUS_VideoEncoder_GetDefaultClearStatus(NEXUS_VideoEncoderClearStatus *pClearStatus)
{
    BSTD_UNUSED(pClearStatus);

    return;
}
#else /* !NEXUS_NUM_DSP_VIDEO_ENCODERS */

void
NEXUS_VideoEncoderModule_GetDefaultSettings( NEXUS_VideoEncoderModuleSettings *pSettings )
{
    BSTD_UNUSED(pSettings);
    return;
}

NEXUS_ModuleHandle
NEXUS_VideoEncoderModule_Init(const NEXUS_VideoEncoderModuleSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    return NULL;
}

void
NEXUS_VideoEncoderModule_Uninit(void)
{
    return;
}

NEXUS_Error NEXUS_VideoEncoderModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}


void NEXUS_VideoEncoder_GetDefaultOpenSettings(NEXUS_VideoEncoderOpenSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    return ;
}

NEXUS_VideoEncoderHandle
NEXUS_VideoEncoder_Open(unsigned index, const NEXUS_VideoEncoderOpenSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(index);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void
NEXUS_VideoEncoder_Close(NEXUS_VideoEncoderHandle encoder)
{
    BSTD_UNUSED(encoder);
    return;
}


void
NEXUS_VideoEncoder_GetDefaultStartSettings(NEXUS_VideoEncoderStartSettings *pSettings )
{
    BSTD_UNUSED(pSettings);
    return;
}

NEXUS_Error
NEXUS_VideoEncoder_Start(NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderStartSettings *pSettings)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void
NEXUS_VideoEncoder_Stop( NEXUS_VideoEncoderHandle encoder)
{
    BSTD_UNUSED(encoder);
    return;
}

void
NEXUS_VideoEncoder_GetSettings(NEXUS_VideoEncoderHandle encoder, NEXUS_VideoEncoderSettings *pSettings)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(pSettings);
    return;
}

NEXUS_Error
NEXUS_VideoEncoder_SetSettings( NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderSettings *pSettings)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error
NEXUS_VideoEncoder_GetBuffer( NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderDescriptor **pBuffer, size_t *pSize, const NEXUS_VideoEncoderDescriptor **pBuffer2, size_t *pSize2)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(pSize);
    BSTD_UNUSED(pBuffer2);
    BSTD_UNUSED(pSize2);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error
NEXUS_VideoEncoder_ReadComplete(NEXUS_VideoEncoderHandle encoder, unsigned descriptorsCompleted)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(descriptorsCompleted);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void
NEXUS_VideoEncoder_GetStatus(NEXUS_VideoEncoderHandle encoder, NEXUS_VideoEncoderStatus *pStatus)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(pStatus);
    return;
}

NEXUS_Error
NEXUS_VideoEncoder_GetDelayRange (NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderSettings *pSettings, const NEXUS_VideoEncoderStartSettings *pStartSettings, NEXUS_VideoEncoderDelayRange *pDelayRange)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(pStartSettings);
    BSTD_UNUSED(pDelayRange);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void
NEXUS_VideoEncoder_GetSettingsOnInputChange(NEXUS_VideoEncoderHandle encoder, NEXUS_VideoEncoderSettingsOnInputChange *pSettings)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(pSettings);
    return;
}

NEXUS_Error
NEXUS_VideoEncoder_SetSettingsOnInputChange(NEXUS_VideoEncoderHandle encoder, const NEXUS_VideoEncoderSettingsOnInputChange *pSettings)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error
NEXUS_VideoEncoder_InsertRandomAccessPoint(NEXUS_VideoEncoderHandle encoder)
{
    BSTD_UNUSED(encoder);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void
NEXUS_VideoEncoder_ClearStatus(NEXUS_VideoEncoderHandle handle, const NEXUS_VideoEncoderClearStatus *pClearStatus)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(pClearStatus);

    return;
}

void
NEXUS_VideoEncoder_GetDefaultClearStatus(NEXUS_VideoEncoderClearStatus *pClearStatus)
{
    BSTD_UNUSED(pClearStatus);

    return;
}
#endif /* NEXUS_NUM_DSP_VIDEO_ENCODERS */

void NEXUS_GetVideoEncoderCapabilities( NEXUS_VideoEncoderCapabilities *pCapabilities )
{
    int i, j;
    BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));
    /* DSP video encoder requires window on MEMC0 */
#if NEXUS_DSP_ENCODER_ACCELERATOR_SUPPORT
    BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));
    for (i=0;i<NEXUS_MAX_VIDEO_ENCODERS;i++) {
        for (j=0;j<BBOX_VDC_DISPLAY_COUNT;j++) {
            if (g_pCoreHandles->boxConfig->stVdc.astDisplay[j].stStgEnc.bAvailable)
            {
                pCapabilities->videoEncoder[i].displayIndex = j;
                break;
            }
        }
        if (j == BBOX_VDC_DISPLAY_COUNT) continue;
        pCapabilities->videoEncoder[i].supported = true;
        pCapabilities->videoEncoder[i].memory = g_NEXUS_VideoEncoder_P_State.config.videoEncoder[i].memory;
        BDBG_MSG(("Video encoder[%d] is supported with display[%d]", i, j));
        break; /* TODO: support multiple soft transcoders */
    }
#else
    BSTD_UNUSED(j);
    for (i=NEXUS_MAX_DISPLAYS-1;i>=0;i--) {
        unsigned memcIndex;
        int rc;
        rc = NEXUS_Display_P_GetWindowMemc_isrsafe(i, 0, &memcIndex);
        if (!rc && memcIndex == 0) break;
    }
    if (i >= 0) {
        pCapabilities->videoEncoder[0].supported = true;
        pCapabilities->videoEncoder[0].displayIndex = i;
        pCapabilities->videoEncoder[0].memory = g_NEXUS_VideoEncoder_P_State.config.videoEncoder[0].memory;
    }
#endif
}

unsigned NEXUS_VideoEncoder_GetIndex_isrsafe(NEXUS_VideoEncoderHandle encoder)
{
    BDBG_OBJECT_ASSERT(encoder, NEXUS_VideoEncoder);
    return encoder->index;
}

void NEXUS_VideoEncoderModule_GetStatistics( NEXUS_VideoEncoderModuleStatistics *pStats )
{
    BKNI_Memset(pStats, 0, sizeof(*pStats));
}

NEXUS_Error NEXUS_VideoEncoder_ReadIndex(NEXUS_VideoEncoderHandle encoder, NEXUS_VideoEncoderDescriptor *pBuffer, unsigned size, unsigned *pRead)
{
    BSTD_UNUSED(encoder);
    BSTD_UNUSED(pBuffer);
    BSTD_UNUSED(size);
    BSTD_UNUSED(pRead);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}
