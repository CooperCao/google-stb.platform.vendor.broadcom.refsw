/******************************************************************************
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
 *****************************************************************************/
#include "nexus_base.h"
#include "nexus_display_module.h"
#if NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input_ext.h"
#include "priv/nexus_hdmi_input_priv.h"
#endif

#if NEXUS_NUM_VIDEO_DECODERS
#include "priv/nexus_video_decoder_priv.h"
#endif

BDBG_MODULE(nexus_video_input);

BDBG_OBJECT_ID(NEXUS_VideoInput_P_Link);

#define pVideo (&g_NEXUS_DisplayModule_State)

static void NEXUS_VideoInput_P_HdrInputInfoUpdated(void * context);
static NEXUS_VideoInput_P_Link *NEXUS_VideoInput_P_CreateLink_Init( NEXUS_VideoInput source, const NEXUS_VideoInput_P_LinkData *data, const NEXUS_VideoInput_P_Iface *iface );
static void NEXUS_VideoInput_P_DestroyLink_Uninit( NEXUS_VideoInput_P_Link *link );
static NEXUS_Error NEXUS_VideoInput_P_Create_VdcSource( NEXUS_VideoInput source, NEXUS_VideoInput_P_Link *link, const NEXUS_VideoInput_P_LinkData *data );
static void NEXUS_VideoInput_P_Destroy_VdcSource( NEXUS_VideoInput_P_Link *link );

static void NEXUS_VideoInput_P_SourceCallback_isr(void *pParam1, int pParam2, void *vdcData)
{
    NEXUS_VideoInput_P_Link *link = pParam1;
    const BVDC_Source_CallbackData *pSrcData = (const BVDC_Source_CallbackData*)vdcData;
    const BVDC_Source_CallbackMask *pMask = &pSrcData->stMask;
    NEXUS_VideoInputStatus *pStatus;

    BSTD_UNUSED(pParam2);
    BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);
    pStatus = &link->status;

    /* save all the data for later source calls */
    link->vdcSourceCallbackData = *pSrcData;

    if (link->id <= BAVC_SourceId_eMpegMax) {
        if (pMask->bCrcValue && link->crc.queue) {
            NEXUS_VideoInputCrcData *pData = &link->crc.queue[link->crc.wptr];
            pData->crc[0] = pSrcData->ulLumaCrc;
            pData->crc[1] = pSrcData->ulChromaCrc;
            pData->crc[2] = pSrcData->ulChroma1Crc;
            pData->crc[3] = pSrcData->ulLumaCrcR;
            pData->crc[4] = pSrcData->ulChromaCrcR;
            pData->crc[5] = pSrcData->ulChroma1CrcR;
            pData->idrPictureId = pSrcData->ulIdrPicId;
            pData->pictureOrderCount = pSrcData->lPicOrderCnt;
            pData->isField = pSrcData->eSourcePolarity != BAVC_Polarity_eFrame;

            if (++link->crc.wptr == link->crc.size) {
                link->crc.wptr = 0;
            }
            if (link->crc.wptr == link->crc.rptr) {
                BDBG_WRN(("crc capture overflow"));
            }
        }
    }
    else {
        /* For analog/hd-dvi sources, we need to capture the data passed into the callback. Calling BVDC_Source_GetInputStatus does
        not return reliable information.
        A change in bActive will always result in a sourceChanged callback. */
        pStatus->videoPresent = pSrcData->bActive;
        /* Based on current VDC impl, pSrcData->pFmtInfo will never be NULL for analog/hddvi. We are checking anyway for code safety.
        The main flag for knowing when video comes and goes is a change in bActive. */
        if (pSrcData->pFmtInfo) {
            if (pMask->bFmtInfo || pMask->bFrameRate) { /* save a couple cpu clock cycles */
                pStatus->width = pSrcData->pFmtInfo->ulWidth;
                pStatus->height = pSrcData->pFmtInfo->ulHeight;
                pStatus->format = NEXUS_P_VideoFormat_FromMagnum_isrsafe(pSrcData->pFmtInfo->eVideoFmt);
                pStatus->aspectRatio = NEXUS_P_AspectRatio_FromMagnum_isrsafe(pSrcData->pFmtInfo->eAspectRatio);
                pStatus->frameRate = NEXUS_P_FrameRate_FromMagnum_isrsafe(pSrcData->eFrameRateCode);
                pStatus->refreshRate = NEXUS_P_RefreshRate_FromFrameRate_isrsafe(pStatus->frameRate);
            }
        }
        else {
            pStatus->format = NEXUS_VideoFormat_eUnknown;
        }
    }

    switch (link->input->type) {
#if NEXUS_HAS_VIDEO_DECODER && NEXUS_NUM_VIDEO_DECODERS
    case NEXUS_VideoInputType_eDecoder:
        /* VDC reports that decoder refreshRate should change */
        if (pSrcData->stMask.bFrameRate && pSrcData->bMtgSrc) {
            BKNI_SetEvent(link->sourceChangedEvent);
        }
        break;
#endif
#if NEXUS_NUM_HDMI_INPUTS
    case NEXUS_VideoInputType_eHdmi:
        /* Allow HdmiInput to do ISR processing */
        NEXUS_VideoInput_P_HdmiSourceCallback_isr(link, pSrcData);

        /* HdmiInput needs a task callback to call BVDC_Source_GetInputStatus and get the remaining status information.
           VideoInput must do the work. */
        BKNI_SetEvent(link->sourceChangedEvent);
        break;
#endif
    default:
        break;
    }
}

static void NEXUS_VideoInput_P_SourceChanged(void *context)
{
    NEXUS_VideoInput input = context;
    NEXUS_VideoInput_P_Link *link = input->destination; /* do not call NEXUS_VideoInput_P_Get here. if the input has been shutdown, we can't recreate it here */

    if (!link) return;
    BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);

    switch (link->input->type) {
#if NEXUS_HAS_VIDEO_DECODER && NEXUS_NUM_VIDEO_DECODERS
    case NEXUS_VideoInputType_eDecoder:
    {
        /* it must be mtg src if sourceChangedEvent is set for a decorder input */
        NEXUS_VideoDecoder_DisplayInformation displayInformation;
        BKNI_Memset(&displayInformation, 0, sizeof(displayInformation));
        displayInformation.refreshRate = link->vdcSourceCallbackData.ulVertRefreshRate;
        BDBG_MSG(("mtg display refreshRate%d", displayInformation.refreshRate));
        NEXUS_Module_Lock(pVideo->modules.videoDecoder);
        NEXUS_VideoDecoder_UpdateDisplayInformation_priv((NEXUS_VideoDecoderHandle)link->input->source, &displayInformation);
        NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
        break;
    }
#endif
#if NEXUS_NUM_HDMI_INPUTS
    case NEXUS_VideoInputType_eHdmi:
        NEXUS_VideoInput_P_SetHdmiInputStatus(link);
        break;
#endif
    default:
        break;
    }
}

void
NEXUS_VideoInput_P_LinkData_Init(NEXUS_VideoInput_P_LinkData *data, BAVC_SourceId sourceId)
{
    BDBG_ASSERT(data);
    BKNI_Memset(data, 0, sizeof(*data));
    data->sourceId = sourceId;
    data->mtg = BVDC_Mode_eOff;
    return;
}

NEXUS_VideoInput_P_Link *
NEXUS_VideoInput_P_CreateLink(NEXUS_VideoInput source, const NEXUS_VideoInput_P_LinkData *data, const NEXUS_VideoInput_P_Iface *iface)
{
    NEXUS_VideoInput_P_Link *link = NULL;
    NEXUS_Error rc;

    link = NEXUS_VideoInput_P_CreateLink_Init(source, data, iface);
    if (!link) return NULL;

    rc = NEXUS_VideoInput_P_Create_VdcSource(source, link, data);
    if (NEXUS_SUCCESS != rc) {
        NEXUS_VideoInput_P_DestroyLink_Uninit(link);
        return NULL;
    }

    return link;
}

static void NEXUS_VideoInput_P_GetDefaultSettings(NEXUS_VideoInputSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->video3DSettings.overrideOrientation = false;
    pSettings->video3DSettings.orientation = NEXUS_VideoOrientation_e2D;
    NEXUS_CallbackDesc_Init(&pSettings->sourceChanged);
    return;
}

static NEXUS_VideoInput_P_Link *
NEXUS_VideoInput_P_CreateLink_Init(NEXUS_VideoInput source, const NEXUS_VideoInput_P_LinkData *data, const NEXUS_VideoInput_P_Iface *iface)
{
    NEXUS_VideoInput_P_Link *link = NULL;

    BDBG_ASSERT(source && source->source);
    BDBG_ASSERT(data);
    BDBG_ASSERT(iface);

    BDBG_MSG((">NEXUS_VideoInput_P_CreateLink_Init id %d, heap %p", data->sourceId, (void *)data->heap));

    link = BKNI_Malloc(sizeof(*link));
    if(!link) {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    BKNI_Memset(link, 0, sizeof(*link));
    BDBG_OBJECT_SET(link, NEXUS_VideoInput_P_Link);
    NEXUS_VideoInput_P_GetDefaultSettings(&link->cfg);
    link->id = data->sourceId;
    link->heap = data->heap;

    link->input = source;
    link->iface = *iface;
    link->input_info.type = source->type;
    link->input_info.source = source->source;
    link->sourceChangedCallback = NEXUS_IsrCallback_Create(source, NULL);
    BKNI_CreateEvent(&link->sourceChangedEvent);
    link->sourceChangedEventHandler = NEXUS_RegisterEvent(link->sourceChangedEvent, NEXUS_VideoInput_P_SourceChanged, source);
    link->resumeMode = NEXUS_VideoInputResumeMode_eAuto;

    BKNI_CreateEvent(&link->drm.inputInfoUpdatedEvent);
    link->drm.inputInfoUpdatedEventHandler = NEXUS_RegisterEvent(link->drm.inputInfoUpdatedEvent, NEXUS_VideoInput_P_HdrInputInfoUpdated, link);
    link->drm.inputInfoFrame.eotf = NEXUS_VideoEotf_eInvalid;
    link->timebase = NEXUS_Timebase_eInvalid;

#if NEXUS_VBI_SUPPORT
    /* Only default a CC buffer. All others require app to set buffer size. */
    link->vbiSettings.closedCaptionBufferSize = 30;
    /* VBI decode defaults copied from bvbi.c */
    link->vbiSettings.gemStar.baseLineTop = 10;
    link->vbiSettings.gemStar.lineMaskTop = 0x1f;
    link->vbiSettings.gemStar.baseLineBottom = 273;
    link->vbiSettings.gemStar.lineMaskBottom = 0x1f;
    link->vbiSettings.wss.vline576i = 23;
    link->vbiSettings.cc.ccLineTop      = 21;
    link->vbiSettings.cc.ccLineBottom   = 284;
    NEXUS_CallbackDesc_Init(&link->vbiSettings.wssChanged);
    NEXUS_CallbackDesc_Init(&link->vbiSettings.cgmsChanged);
    NEXUS_CallbackDesc_Init(&link->vbiSettings.vpsChanged);
    NEXUS_CallbackDesc_Init(&link->vbiSettings.closedCaptionDataReady);
    NEXUS_CallbackDesc_Init(&link->vbiSettings.teletextDataReady);
    NEXUS_CallbackDesc_Init(&link->vbiSettings.gemStarDataReady);
    /* no default buffer for teletext */
    link->vbi.wss.isrCallback = NEXUS_IsrCallback_Create(source, NULL);
    link->vbi.wss.data = 0xffff; /* no data */
    link->vbi.cgms.isrCallback = NEXUS_IsrCallback_Create(source, NULL);
    link->vbi.vps.isrCallback = NEXUS_IsrCallback_Create(source, NULL);
    link->vbi.cc.name = "closed caption";
    link->vbi.cc.isrCallback = NEXUS_IsrCallback_Create(source, NULL);
    link->vbi.cc.elementSize = sizeof(NEXUS_ClosedCaptionData);
    link->vbi.tt.name = "teletext";
    link->vbi.tt.isrCallback = NEXUS_IsrCallback_Create(source, NULL);
    link->vbi.tt.elementSize = sizeof(NEXUS_TeletextLine);
    link->vbi.gs.name = "gemstar";
    link->vbi.gs.isrCallback = NEXUS_IsrCallback_Create(source, NULL);
    link->vbi.gs.elementSize = sizeof(NEXUS_GemStarData);
#endif

    source->ref_cnt++;
    BLST_S_DICT_ADD(&pVideo->inputs, link, NEXUS_VideoInput_P_Link, input, link, err_duplicate);
    BDBG_MSG(("<NEXUS_VideoInput_P_CreateLink_Init %p", (void *)link));

    return link;

err_duplicate:
    NEXUS_VideoInput_P_DestroyLink_Uninit(link);
err_alloc:
    return NULL;
}

/* nexus_p_install_videoinput_cb is called on Open and SetSettings.
it is not evenly paired with nexus_p_uninstall_videoinput_cb */
static BERR_Code nexus_p_install_videoinput_cb(NEXUS_VideoInput_P_Link *link)
{
    BVDC_Source_CallbackSettings settings;
    BERR_Code rc;

    rc = BVDC_Source_GetCallbackSettings(link->sourceVdc, &settings);
    if (rc) return BERR_TRACE(rc);
    settings.stMask.bCrcValue = (link->cfg.crcQueueSize != 0);
    settings.stMask.bFrameRate = true;
    rc = BVDC_Source_SetCallbackSettings(link->sourceVdc, &settings);
    if (rc) return BERR_TRACE(rc);

    if (link->crc.size != link->cfg.crcQueueSize) {
        void *new_ptr = NULL, *old_ptr;

        /* defer the free until after critical section */
        old_ptr = link->crc.queue;
        /* queue size of 1 is treated same as 0 because it can't hold anything */
        if (link->cfg.crcQueueSize > 1) {
            new_ptr = BKNI_Malloc(link->cfg.crcQueueSize * sizeof(NEXUS_VideoInputCrcData));
            if (!new_ptr) {
                return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            }
        }

        /* must synchronize with ISR, so set state in CS */
        BKNI_EnterCriticalSection();
        link->crc.queue = new_ptr;
        link->crc.size = link->cfg.crcQueueSize>1?link->cfg.crcQueueSize:0;
        link->crc.wptr = link->crc.rptr = 0; /* flush */
        BKNI_LeaveCriticalSection();

        if (old_ptr) {
            BKNI_Free(old_ptr);
        }
    }

    rc = BVDC_Source_InstallCallback(link->sourceVdc, NEXUS_VideoInput_P_SourceCallback_isr, link, 0);
    if (rc) return BERR_TRACE(rc);

    return 0;
}

static void nexus_p_uninstall_videoinput_cb(NEXUS_VideoInput_P_Link *link)
{
    BVDC_Source_CallbackSettings settings;
    BERR_Code rc;

    rc = BVDC_Source_GetCallbackSettings(link->sourceVdc, &settings);
    settings.stMask.bCrcValue = 0;
    rc = BVDC_Source_SetCallbackSettings(link->sourceVdc, &settings);
    if (rc) rc = BERR_TRACE(rc);

    rc = BVDC_Source_InstallCallback(link->sourceVdc, (BVDC_CallbackFunc_isr)NULL, NULL, 0);
    if (rc) rc = BERR_TRACE(rc);

    if (link->crc.queue) {
        BKNI_Free(link->crc.queue);
        BKNI_EnterCriticalSection();
        link->crc.queue = NULL;
        link->crc.size = 0;
        BKNI_LeaveCriticalSection();
    }
}


static NEXUS_Error
NEXUS_VideoInput_P_Create_VdcSource(NEXUS_VideoInput source, NEXUS_VideoInput_P_Link *link, const NEXUS_VideoInput_P_LinkData *data)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BVDC_Source_Settings sourceCfg;

    BDBG_ASSERT(source && source->source);
    BDBG_ASSERT(link);
    BDBG_ASSERT(data);

    BDBG_MSG((">NEXUS_VideoInput_P_Create_VdcSource input %p, link %p, heap %p", (void *)source, (void *)link, (void *)data->heap));
    if (data->sourceVdc) {
        link->sourceVdc = data->sourceVdc;
        link->copiedSourceVdc = true;
    }
    else {
        rc = BVDC_Source_GetDefaultSettings(data->sourceId, &sourceCfg);
        if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_source;}

        if (data->heap && data->heap != pVideo->heap) {
            sourceCfg.hHeap = NEXUS_Display_P_CreateHeap(data->heap);
            if (!sourceCfg.hHeap) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_source;}

            link->vdcHeap = sourceCfg.hHeap;
        }

        /* This is set to true only for VideoImageInput without XDM */
        sourceCfg.bGfxSrc = data->gfxSource;
        link->mtg = data->mtg;
        sourceCfg.eMtgMode = data->mtg;

        rc = BVDC_Source_Create(pVideo->vdc, &link->sourceVdc, data->sourceId, &sourceCfg);
        if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_source;}

        rc = nexus_p_install_videoinput_cb(link);
        if (rc) {rc = BERR_TRACE(rc); goto err_install_videoinput_cb;}

        rc = NEXUS_Display_P_ApplyChanges();
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_apply_changes;}
    }

    BDBG_MSG(("<NEXUS_VideoInput_P_Create_VdcSource %s %p:%d", link->copiedSourceVdc?"copied":"created", (void *)link->sourceVdc, data->sourceId));

    return NEXUS_SUCCESS;

err_apply_changes:
    nexus_p_uninstall_videoinput_cb(link);
err_install_videoinput_cb:
    if (!link->copiedSourceVdc) {
        rc = BVDC_AbortChanges(pVideo->vdc);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
        rc = BVDC_Source_Destroy(link->sourceVdc);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}
    }
err_source:
    return rc;
}

void
NEXUS_VideoInput_P_DestroyLink(NEXUS_VideoInput_P_Link *link)
{
    NEXUS_VideoInput_P_Destroy_VdcSource(link);

    BKNI_Memset(&link->vbiSettings, 0, sizeof(link->vbiSettings));
#if NEXUS_VBI_SUPPORT
    NEXUS_VideoInput_P_SetVbiState(link->input);
#endif
    NEXUS_VideoInput_P_DestroyLink_Uninit(link);

    return;
}

static void
NEXUS_VideoInput_P_DestroyLink_Uninit(NEXUS_VideoInput_P_Link *link)
{
    BDBG_MSG((">NEXUS_VideoInput_P_DestroyLink_Uninit %p", (void *)link));
    BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);

#if NEXUS_VBI_SUPPORT
    NEXUS_IsrCallback_Destroy(link->vbi.gs.isrCallback);
    NEXUS_IsrCallback_Destroy(link->vbi.tt.isrCallback);
    NEXUS_IsrCallback_Destroy(link->vbi.cc.isrCallback);
    NEXUS_IsrCallback_Destroy(link->vbi.wss.isrCallback);
    NEXUS_IsrCallback_Destroy(link->vbi.cgms.isrCallback);
    NEXUS_IsrCallback_Destroy(link->vbi.vps.isrCallback);
#endif
    if(link->vbi.cc.data) {
        BKNI_Free(link->vbi.cc.data);
    }
    if(link->vbi.tt.data) {
        BKNI_Free(link->vbi.tt.data);
    }

    NEXUS_UnregisterEvent(link->drm.inputInfoUpdatedEventHandler);
    BKNI_DestroyEvent(link->drm.inputInfoUpdatedEvent);

    NEXUS_IsrCallback_Destroy(link->sourceChangedCallback);
    NEXUS_UnregisterEvent(link->sourceChangedEventHandler);
    BKNI_DestroyEvent(link->sourceChangedEvent);
    if ( link->checkFormatChangedEventHandler )
    {
        NEXUS_UnregisterEvent(link->checkFormatChangedEventHandler);
        BKNI_DestroyEvent(link->checkFormatChangedEvent);
    }

    link->input->ref_cnt--;
    BLST_S_DICT_REMOVE(&pVideo->inputs, link, link->input, NEXUS_VideoInput_P_Link, input, link);

    BDBG_OBJECT_DESTROY(link, NEXUS_VideoInput_P_Link);
    BKNI_Free(link);

    BDBG_MSG(("<NEXUS_VideoInput_P_DestroyLink_Uninit"));

    return;
}

static void
NEXUS_VideoInput_P_Destroy_VdcSource(NEXUS_VideoInput_P_Link *link)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);
    BDBG_MSG((">NEXUS_VideoInput_P_Destroy_VdcSource input %p, link %p", (void *)link->input, (void *)link));

    nexus_p_uninstall_videoinput_cb(link);

    if (!link->copiedSourceVdc && link->sourceVdc)
    {
        rc = BVDC_Source_Destroy(link->sourceVdc);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

        if(pVideo->updateMode != NEXUS_DisplayUpdateMode_eAuto) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);}

        rc = BVDC_ApplyChanges(pVideo->vdc);
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

        link->sourceVdc = NULL;
        nexus_videoadj_p_dnr_dealloc(link->id);

        if (link->vdcHeap) {
            NEXUS_Display_P_DestroyHeap(link->vdcHeap);
        }
   }

    BDBG_MSG(("<NEXUS_VideoInput_P_Destroy_VdcSource"));

    return;
}

void
NEXUS_VideoInput_GetSettings(NEXUS_VideoInput input, NEXUS_VideoInputSettings *pSettings)
{
    NEXUS_VideoInput_P_Link *link;

    BDBG_ASSERT(pSettings);
    BDBG_OBJECT_ASSERT(input, NEXUS_VideoInput);
    link = NEXUS_VideoInput_P_Get(input);
    if(!link) {
        NEXUS_VideoInput_P_GetDefaultSettings(pSettings);
        goto done;
    }
    BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);
    *pSettings  = link->cfg;
done:
    return;
}

/*
Summary:
Returns bits 'e'..'b' from word 'w',

Example:
    B_GET_BITS(0xDE,7,4)==0x0D
    B_GET_BITS(0xDE,3,0)=0x0E
*/
#define B_GET_BITS(w,e,b)  (((w)>>(b))&(((unsigned)(-1))>>((sizeof(unsigned))*8-(e+1-b))))


NEXUS_Error
NEXUS_VideoInput_SetSettings( NEXUS_VideoInput input, const NEXUS_VideoInputSettings *pSettings)
{
    BERR_Code rc = NEXUS_SUCCESS;
    NEXUS_VideoInput_P_Link *link;

    BDBG_ASSERT(pSettings);
    BDBG_OBJECT_ASSERT(input, NEXUS_VideoInput);
    link = NEXUS_VideoInput_P_Get(input);
    if(!link) { rc=BERR_TRACE(BERR_NOT_SUPPORTED); goto err_link;}
    if (!link->sourceVdc) {link->isDeferCfg = true; link->cfg = *pSettings; return NEXUS_SUCCESS;}

    if(pSettings->muteColor != link->cfg.muteColor) {
        rc = BVDC_Source_SetVideoMuteColor(link->sourceVdc, B_GET_BITS(pSettings->muteColor, 23, 16),
                                           B_GET_BITS(pSettings->muteColor, 15, 8), B_GET_BITS(pSettings->muteColor, 7, 0));
        if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_settings;}
    }
    if(pSettings->mute != link->cfg.mute || pSettings->repeat != link->cfg.repeat) {
        rc = BVDC_Source_SetMuteMode(link->sourceVdc,
            pSettings->mute ? BVDC_MuteMode_eConst :
            pSettings->repeat ? BVDC_MuteMode_eRepeat :
            BVDC_MuteMode_eDisable);
        if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_settings;}
    }
    if (pSettings->video3DSettings.overrideOrientation != link->cfg.video3DSettings.overrideOrientation ||
        pSettings->video3DSettings.orientation != link->cfg.video3DSettings.orientation)
    {
        rc = BVDC_Source_SetOrientation(link->sourceVdc, pSettings->video3DSettings.overrideOrientation, NEXUS_P_VideoOrientation_ToMagnum_isrsafe(pSettings->video3DSettings.orientation));
        if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_settings;}
    }

    link->cfg = *pSettings;

    rc = nexus_p_install_videoinput_cb(link);
    if (rc) {rc = BERR_TRACE(rc); goto err_settings;}

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    NEXUS_IsrCallback_Set(link->sourceChangedCallback, &pSettings->sourceChanged);

    return rc;

err_settings:
    {
        BERR_Code rc = BVDC_AbortChanges(pVideo->vdc);
        if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);}
    }
err_link:
    return rc;
}

NEXUS_Error
NEXUS_VideoInput_GetStatus(NEXUS_VideoInput input,NEXUS_VideoInputStatus *pStatus)
{
    NEXUS_VideoInput_P_Link *link;

    BDBG_ASSERT(pStatus);
    BDBG_OBJECT_ASSERT(input, NEXUS_VideoInput);
    link = NEXUS_VideoInput_P_Get(input);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if(!link) { goto done;}

    if (link->id <= BAVC_SourceId_eMpegMax) {
        if (link->info_valid) {
            BAVC_FrameRateCode frameRate;
            BFMT_AspectRatio aspectRatio;
            bool bStreamProgressive;

            /* the assignment of the BAVC_MFD_Picture structure is not atomic. therefore, protect. */
            BKNI_EnterCriticalSection();
            frameRate = link->info.mfd.eFrameRateCode;
            aspectRatio = link->info.mfd.eAspectRatio;
            bStreamProgressive = link->info.mfd.bStreamProgressive;
            pStatus->width = link->info.mfd.ulSourceHorizontalSize;
            pStatus->height = link->info.mfd.ulSourceVerticalSize;
            pStatus->videoPresent = !link->info.mfd.bMute;
            BKNI_LeaveCriticalSection();

            pStatus->frameRate = NEXUS_P_FrameRate_FromMagnum_isrsafe(frameRate);
            pStatus->format = NEXUS_P_VideoFormat_FromInfo_isrsafe(pStatus->height, pStatus->frameRate, !bStreamProgressive);
            pStatus->aspectRatio = NEXUS_P_AspectRatio_FromMagnum_isrsafe(aspectRatio);
            pStatus->refreshRate = NEXUS_P_RefreshRate_FromFrameRate_isrsafe(pStatus->frameRate);
        }
    }
    else {
        /* this status was already captured in NEXUS_VideoInput_P_SourceCallback_isr */
        *pStatus = link->status;
    }

    if (link->sourceVdc) {
        BVDC_Source_DebugStatus status;
        BVDC_Dbg_Source_GetDebugStatus(link->sourceVdc, &status);
        pStatus->numBvnErrors = status.ulNumErr;
    }

done:
    return 0;
}


static void
NEXUS_VideoInput_P_HdrInputInfoUpdated(void * context)
{
#if NEXUS_HAS_HDMI_OUTPUT && NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_VideoInput_P_Link *link = context;
    NEXUS_VideoWindowHandle windows[NEXUS_NUM_DISPLAYS];
    NEXUS_HdmiDynamicRangeMasteringInfoFrame drmInfoFrame ;
    unsigned count;
    unsigned i;

    BDBG_MSG(("Update HDR information using VideoInputType: %d",
        link->input->type)) ;

    if (link->input->type == NEXUS_VideoInputType_eDecoder)
    {
        BKNI_Memcpy(&drmInfoFrame, &link->drm.inputInfoFrame,
            sizeof(NEXUS_HdmiDynamicRangeMasteringInfoFrame)) ;
    }
#if NEXUS_HAS_HDMI_INPUT
    else if (link->input->type == NEXUS_VideoInputType_eHdmi)
    {
        NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput) ;
            NEXUS_HdmiInput_GetDrmInfoFrameData_priv(link->input->source, &drmInfoFrame) ;
        NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput) ;
    }
#endif
    else
    {
        BDBG_WRN(("Unsupported HDR Input Type: %d", link->input->type)) ;
        return ;
    }

    NEXUS_Display_P_GetWindows_priv(link->input, windows, NEXUS_NUM_DISPLAYS, &count);

    for (i = 0; i < count; i++)
    {
        if (windows[i]->display->hdmi.outputNotify)  /* HDMI Output is attached to this display */
        {
            NEXUS_VideoOutput_P_SetHdrSettings(windows[i]->display->hdmi.outputNotify,
                &drmInfoFrame) ;
        }
    }
#else
    BSTD_UNUSED(context);
#endif
}

#if NEXUS_NUM_VIDEO_DECODERS
static void
NEXUS_VideoInput_P_UpdateHdrInputInfo_isr(NEXUS_VideoInput_P_Link *link)
{
    NEXUS_TransferCharacteristics transferChars;
    NEXUS_TransferCharacteristics preferredTransferChars;
    NEXUS_HdmiDynamicRangeMasteringInfoFrame drmInfoFrame ;

    BKNI_Memset(&drmInfoFrame, 0 , sizeof(NEXUS_HdmiDynamicRangeMasteringInfoFrame)) ;

    transferChars = NEXUS_P_TransferCharacteristics_FromMagnum_isrsafe(link->info.mfd.eTransferCharacteristics);
    preferredTransferChars = NEXUS_P_TransferCharacteristics_FromMagnum_isrsafe(link->info.mfd.ePreferredTransferCharacteristics);

    drmInfoFrame.eotf =
        NEXUS_P_TransferCharacteristicsToEotf_isrsafe(transferChars, preferredTransferChars);

    if (drmInfoFrame.eotf == NEXUS_VideoEotf_eHdr10)
    {
        /* set Metadata type; only one type is currently supported */
        drmInfoFrame.metadata.type = NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1 ;
    }

    NEXUS_P_MasteringDisplayColorVolume_FromMagnum_isrsafe(
        &drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume,
        link->info.mfd.stDisplayPrimaries,
        &link->info.mfd.stWhitePoint,
        link->info.mfd.ulMaxDispMasteringLuma,
        link->info.mfd.ulMinDispMasteringLuma);

    NEXUS_P_ContentLightLevel_FromMagnum_isrsafe(
        &drmInfoFrame.metadata.typeSettings.type1.contentLightLevel,
        link->info.mfd.ulMaxContentLight,
        link->info.mfd.ulAvgContentLight);

    if (BKNI_Memcmp(&drmInfoFrame, &link->drm.inputInfoFrame, sizeof(NEXUS_HdmiDynamicRangeMasteringInfoFrame)))
    {
        BDBG_MSG(("Update DRM Packet to use EOTF: %d", drmInfoFrame.eotf)) ;
        BKNI_Memcpy(&link->drm.inputInfoFrame, &drmInfoFrame, sizeof(NEXUS_HdmiDynamicRangeMasteringInfoFrame)) ;
        link->drm.inputInfoFrame.eotf = drmInfoFrame.eotf ;
        BKNI_SetEvent_isr(link->drm.inputInfoUpdatedEvent);
    }
}

static void NEXUS_Display_P_VerifyDisplayTimebase_isr(NEXUS_VideoInput_P_Link *link)
{
    unsigned i, j;
    NEXUS_Timebase timebase;

    timebase = NEXUS_VideoDecoder_P_GetTimebase_isrsafe(link->input->source);
    if(timebase != link->timebase) {
        for (i=0;i<sizeof(pVideo->displays)/sizeof(pVideo->displays[0]);i++) {
            NEXUS_DisplayHandle display = pVideo->displays[i];
            if(!display) continue;
            for(j=0; j<sizeof(display->windows)/sizeof(display->windows[0]); j++) {
                NEXUS_VideoWindowHandle window = &display->windows[j];
                if (!window->open) continue;
                if((window->input == link->input)) {
                        BDBG_WRN(("Display %u uses Timebase %ld, Decoder uses Timebase %ld", i, display->cfg.timebase, timebase));
                }
            }
        }
        link->timebase = timebase;
    }
}

void
NEXUS_VideoInput_P_DecoderDataReady_isr(void *input_, const BAVC_MFD_Picture *pPicture)
{
    NEXUS_VideoInput_P_Link *link = input_;
    BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);
    BDBG_ASSERT(pPicture);
    link->info.mfd = *pPicture;
    link->info_valid = true;

#if NEXUS_NUM_MOSAIC_DECODES
    if (link->mosaic.backendMosaic) {
        unsigned i;
        for (i=0;i<NEXUS_NUM_DISPLAYS;i++) {
            NEXUS_VideoWindowHandle window = link->mosaic.parentWindow[i];
            if (window) {
                /* this takes a single picture and cuts it up into a linked list of mosaic pictures */
                pPicture = NEXUS_VideoWindow_P_CutBackendMosaic_isr(window, pPicture);
                BDBG_ASSERT(pPicture); /* if it can't cut, NEXUS_VideoWindow_P_CutBackendMosaic_isr should have returned the original pPicture */

                /* the first window gets to cut it. the other has to follow. */
                break;
            }
        }
    }
#endif

    NEXUS_VideoInput_P_UpdateHdrInputInfo_isr(link);

    if(pVideo->verifyTimebase)
        NEXUS_Display_P_VerifyDisplayTimebase_isr(link);

    BVDC_Source_MpegDataReady_isr(link->sourceVdc, 0 /* unused */, (void *)pPicture);
    return;
}

#if NEXUS_HAS_VIDEO_ENCODER
static void
NEXUS_VideoInput_P_UserDataCallback_isr(void *input_, const BAVC_USERDATA_info *userDataInfo)
{
    unsigned j;
    NEXUS_VideoInput_P_Link *link = input_;
    NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;

    BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);

    /* forward to each display connected to this input */
    for(j=0;j<sizeof(video->displays)/sizeof(video->displays[0]);j++) {
        NEXUS_DisplayHandle display = video->displays[j];
        if (display) {
            if (display->encodeUserData && (display->xudSource == link->input)) {
                if (BERR_SUCCESS != BXUDlib_UserDataHandler_isr(display->hXud, userDataInfo)) {
                    BDBG_ERR(("video input %p failed to feed user data to encode display %p", (void *)link->input, (void *)display));
                }
            }
        }
    }
}
#endif

static BERR_Code
NEXUS_VideoInput_P_ConnectVideoDecoder(NEXUS_VideoInput_P_Link *link)
{
    BERR_Code rc;
    NEXUS_VideoDecoderDisplayConnection decoderConnect;

    BDBG_ASSERT(link->input && link->input->source);

    if (!pVideo->modules.videoDecoder) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

#if NEXUS_NUM_MOSAIC_DECODES
    if (link->mosaic.backendMosaic) {
        /* the parent has already connected to the decoder */
        return 0;
    }
#endif

    NEXUS_Module_Lock(pVideo->modules.videoDecoder);
    NEXUS_VideoDecoder_GetDisplayConnection_priv(link->input->source, &decoderConnect);
    NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
    link->secureVideo = decoderConnect.secureVideo;

    rc = BVDC_Source_GetInterruptName(link->sourceVdc, BAVC_Polarity_eTopField, &decoderConnect.top.intId);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_intr_name;}
    rc = BVDC_Source_GetInterruptName(link->sourceVdc, BAVC_Polarity_eBotField, &decoderConnect.bottom.intId);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_intr_name;}
    rc = BVDC_Source_GetInterruptName(link->sourceVdc, BAVC_Polarity_eFrame, &decoderConnect.frame.intId);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_intr_name;}
    decoderConnect.callbackContext = link;
    decoderConnect.dataReadyCallback_isr = NEXUS_VideoInput_P_DecoderDataReady_isr;

    NEXUS_Module_Lock(pVideo->modules.videoDecoder);
    rc = NEXUS_VideoDecoder_SetDisplayConnection_priv(link->input->source, &decoderConnect);
    NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_connect;}

#if NEXUS_NUM_MOSAIC_DECODES
    link->mosaic.index = NEXUS_VideoDecoder_GetMosaicIndex_isrsafe(link->input->source);
#endif

    return rc;

err_connect:
err_intr_name:
    return rc;
}

static void
NEXUS_VideoInput_P_DisconnectVideoDecoder(NEXUS_VideoInput_P_Link *link)
{
    NEXUS_Error rc;
    NEXUS_VideoDecoderDisplayConnection decoderConnect;

    if (!pVideo->modules.videoDecoder) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return;
    }

#if NEXUS_NUM_MOSAIC_DECODES
    if (link->mosaic.backendMosaic) {
        /* the parent is the only one connected to the decoder */
        return;
    }
#endif

    BDBG_ASSERT(link->input && link->input->source);
    NEXUS_Module_Lock(pVideo->modules.videoDecoder);
    NEXUS_VideoDecoder_GetDisplayConnection_priv(link->input->source, &decoderConnect);
    decoderConnect.top.intId = 0;
    decoderConnect.bottom.intId = 0;
    decoderConnect.frame.intId = 0;
    decoderConnect.dataReadyCallback_isr = NULL;
    decoderConnect.userDataCallback_isr = NULL;
    rc = NEXUS_VideoDecoder_SetDisplayConnection_priv(link->input->source, &decoderConnect);
    NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); }
    return;
}

NEXUS_VideoInput_P_Link *
NEXUS_VideoInput_P_OpenDecoder(NEXUS_VideoInput input, NEXUS_VideoInput_P_Link *mosaicParent, NEXUS_VideoWindowHandle window)
{
    BAVC_SourceId sourceId;
    NEXUS_VideoInput_P_Iface iface;
    NEXUS_VideoInput_P_LinkData data;
    NEXUS_HeapHandle videoDecoderHeap;
    NEXUS_VideoInput_P_Link *link;

    if (!pVideo->modules.videoDecoder) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }

    BDBG_ASSERT(input->source);
    NEXUS_Module_Lock(pVideo->modules.videoDecoder);
    NEXUS_VideoDecoder_GetSourceId_priv(input->source, &sourceId);
    NEXUS_VideoDecoder_GetHeap_priv(input->source, &videoDecoderHeap);
    NEXUS_Module_Unlock(pVideo->modules.videoDecoder);

    iface.connect = NEXUS_VideoInput_P_ConnectVideoDecoder;
    iface.disconnect = NEXUS_VideoInput_P_DisconnectVideoDecoder;
    NEXUS_VideoInput_P_LinkData_Init(&data, sourceId);
    data.heap = videoDecoderHeap;
    data.mtg = nexus_p_window_alloc_mtg(window);
#if NEXUS_NUM_MOSAIC_DECODES
    if (mosaicParent) {
        data.sourceVdc = mosaicParent->sourceVdc;
    }
#else
    BSTD_UNUSED(mosaicParent);
#endif

    link = NEXUS_VideoInput_P_CreateLink(input, &data, &iface);

#if NEXUS_NUM_MOSAIC_DECODES
    if (link && mosaicParent) {
        link->mosaic.backendMosaic = mosaicParent->mosaic.backendMosaic;
    }
#endif

    return link;
}
#endif

BERR_Code
NEXUS_VideoInput_P_Connect(NEXUS_VideoInput input)
{
    BERR_Code rc;
    NEXUS_VideoInput_P_Link *link;
    bool created = false;

    /* this function is called on the first connection back to the VideoInput source */

    BDBG_OBJECT_ASSERT(input, NEXUS_VideoInput);

    link = input->destination;
    if (!link) {
        link = NEXUS_VideoInput_P_Get(input);
        if(!link) {rc=BERR_TRACE(BERR_NOT_SUPPORTED); goto err_get;}
        input->destination = link;
        created = true;
    }

    link->info_valid = false;

    rc = link->iface.connect(link);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc); goto err_connect;}

#if NEXUS_HAS_SYNC_CHANNEL
    if (input->sync.connectionChangedCallback_isr)
    {
        BKNI_EnterCriticalSection();
        input->sync.connectionChangedCallback_isr(input->sync.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }
#endif

    return rc;

err_connect:
    if (created) {
        /* reduce side-effects by cleaning up what we created */
        NEXUS_VideoInput_P_DestroyLink(link);
    }
    input->destination = NULL;
err_get:
    return rc;
}

void
NEXUS_VideoInput_P_Disconnect(NEXUS_VideoInput input) {
    NEXUS_VideoInput_P_Link *link;
    BERR_Code rc;
    BVDC_Dnr_Settings dnrSettings;

    BDBG_OBJECT_ASSERT(input, NEXUS_VideoInput);

#if NEXUS_HAS_SYNC_CHANNEL
    if (input->sync.connectionChangedCallback_isr)
    {
        BKNI_EnterCriticalSection();
        input->sync.connectionChangedCallback_isr(input->sync.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }
#endif

    link = NEXUS_VideoInput_P_Get(input);
    if(!link) {rc=BERR_TRACE(BERR_NOT_SUPPORTED); goto done;}
    link->iface.disconnect(link);

    /* DNR is window-based in Nexus, source-based in VDC. So, we disable DNR when the VideoInput is disconnected from its last window. */
    rc = BVDC_Source_GetDnrConfiguration(link->sourceVdc, &dnrSettings);
    if (!rc) {
        dnrSettings.eBnrMode = BVDC_FilterMode_eDisable;
        dnrSettings.eDcrMode = BVDC_FilterMode_eDisable;
        dnrSettings.eMnrMode = BVDC_FilterMode_eDisable;
        rc = BVDC_Source_SetDnrConfiguration(link->sourceVdc, &dnrSettings);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); }
    }

    link->info_valid = false;
    input->destination = NULL;

done:
    return;
}

NEXUS_VideoInput_P_Link *
NEXUS_VideoInput_P_GetExisting(NEXUS_VideoInput input)
{
    NEXUS_VideoInput_P_Link *link;
    BDBG_OBJECT_ASSERT(input, NEXUS_VideoInput);
    link = input->destination;
    if(!link) {
        BLST_S_DICT_FIND(&pVideo->inputs, link, input, input, link);
        /* If we find one, this is a disconnected VideoInput. We will allow SetSettings to succeed,
        but settings will not be applied until connected. */
    }

    if (link) {
        BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);
    }
    return link;
}

NEXUS_VideoInput_P_Link *
NEXUS_VideoInput_P_Get(NEXUS_VideoInput input)
{
    return NEXUS_VideoInput_P_GetForWindow(input, NULL);
}

NEXUS_VideoInput_P_Link *
NEXUS_VideoInput_P_GetForWindow(NEXUS_VideoInput input, NEXUS_VideoWindowHandle window)
{
    NEXUS_VideoInput_P_Link *inputLink;

    BDBG_OBJECT_ASSERT(input, NEXUS_VideoInput);
    BSTD_UNUSED(window);

    inputLink = input->destination;
    if(!inputLink) {
        BLST_S_DICT_FIND(&pVideo->inputs, inputLink, input, input, link);
        /* If we find one, this is a disconnected VideoInput. We will allow SetSettings to succeed,
        but settings will not be applied until connected. */
    }

    if(!inputLink) {
        BDBG_ASSERT(input->destination==NULL);
        switch(input->type) {
#if NEXUS_HAS_VIDEO_DECODER && NEXUS_NUM_VIDEO_DECODERS
        case NEXUS_VideoInputType_eDecoder:
            inputLink = NEXUS_VideoInput_P_OpenDecoder(input, NULL, window);
            break;
#endif
        case NEXUS_VideoInputType_eImage:
            inputLink = NEXUS_VideoImageInput_P_OpenInput(input);
            break;
#if NEXUS_NUM_HDMI_INPUTS
        case NEXUS_VideoInputType_eHdmi:
            inputLink = NEXUS_VideoInput_P_OpenHdmi(input);
            break;
#endif
#if BVBI_NUM_IN656
        case NEXUS_VideoInputType_eCcir656:
            inputLink = NEXUS_VideoInput_P_OpenCcir656(input);
            break;
#endif
#if NEXUS_NUM_HDDVI_INPUTS
        case NEXUS_VideoInputType_eHdDvi:
            inputLink = NEXUS_VideoInput_P_OpenHdDviInput(input);
            break;
#endif
        default:
            BDBG_WRN(("unknown VideoInput type: %d", input->type));
            BERR_TRACE(NEXUS_NOT_SUPPORTED);
            inputLink = NULL;
            break;
        }
    }
    else {
        switch(input->type) {
#if NEXUS_HAS_VIDEO_DECODER && NEXUS_NUM_VIDEO_DECODERS
        case NEXUS_VideoInputType_eDecoder:
            if (nexus_p_input_is_mtg(inputLink) && nexus_p_window_alloc_mtg(window) == BVDC_Mode_eOff) {
                /* TODO: If the source is MTG capable and is already created with eAuto, then all windows
                must be MTG capable...unless we destroy and recreate the BVDC_Source with MTG off,
                which would involved destroying and recreating windows. Note that calls like NEXUS_VideoInput_SetVbiSettings
                will create the BVDC_Source in eAuto mode and must be deferred when connecting to non-MTG windows. */
                BDBG_ERR(("cannot connect MTG source to MTG and non-MTG window paths"));
                return NULL;
            }
            break;
#endif
        default:
            break;
        }
    }

    if(inputLink) {
        BDBG_OBJECT_ASSERT(inputLink, NEXUS_VideoInput_P_Link);
        BDBG_ASSERT(inputLink->input == input);
        BDBG_ASSERT(input->destination == NULL || input->destination == inputLink);
        BDBG_ASSERT(input->type == inputLink->input_info.type && input->source == inputLink->input_info.source);
    }
    return inputLink;
}

void
NEXUS_VideoInput_Shutdown(NEXUS_VideoInput input)
{
    NEXUS_VideoInput_P_Link *inputLink;
    NEXUS_Error rc;
    unsigned int i;

    BSTD_UNUSED(rc);
    NEXUS_OBJECT_ASSERT(NEXUS_VideoInput, input);

    if (input->destination) {
        /* check if this input is connected to a window and auto-remove. */
        NEXUS_VideoWindowHandle window[3];
        const unsigned maxnum = sizeof(window)/sizeof(window[0]);
        unsigned num;
        do {
            rc = NEXUS_Display_P_GetWindows_priv(input, window, maxnum, &num);
            if (rc) {rc = BERR_TRACE(rc); goto err_connected;}
            for (i=0;i<num;i++) {
                /* remove in reverse order for faster BVDC_ApplyChanges (usually removes syncslip before synclock) */
                rc = NEXUS_VideoWindow_RemoveInput(window[num-i-1], input);
                if (rc) {rc = BERR_TRACE(rc); goto err_connected;}
            }
        } while (num == maxnum);

        /* if we're still connected, then this video input must be connected to another module. */
        if (input->destination) {
            BDBG_ERR(("Fatal application error. NEXUS_VideoInput_Shutdown %p called on connected input. You must call NEXUS_VideoWindow_RemoveInput before shutting down, otherwise system is likely to be in a corrupted state and will crash.", (void *)input));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); /* input is still connected */
#if 0
            /* Uncomment this BKNI_Fail to get a stack trace into your application. This is a serious app bug and should be corrected. */
            BKNI_Fail();
#endif
            goto err_connected;
        }
    }

#if NEXUS_VBI_SUPPORT
    for (i=0;i<NEXUS_NUM_DISPLAYS;i++) {
        if (pVideo->displays[i]) {
            NEXUS_DisplayVbiSettings dispvbiSettings;
            NEXUS_Display_GetVbiSettings(pVideo->displays[i], &dispvbiSettings);
            if (dispvbiSettings.vbiSource == input) {
                dispvbiSettings.vbiSource = NULL;
                NEXUS_Display_SetVbiSettings(pVideo->displays[i], &dispvbiSettings);
            }
        }
    }
#endif

    /* this is like calling NEXUS_VideoInput_P_Get, but without the possible CreateLink */
    inputLink = input->destination;
    if (!inputLink) {
        BLST_S_DICT_FIND(&pVideo->inputs, inputLink, input, input, link);
    }

    if (inputLink) {
        BDBG_OBJECT_ASSERT(inputLink, NEXUS_VideoInput_P_Link);
        BDBG_ASSERT(inputLink->input == input);
        BDBG_ASSERT(input->type == inputLink->input_info.type && input->source == inputLink->input_info.source);
        BDBG_ASSERT(input->ref_cnt > 0);
        NEXUS_VideoInput_P_DestroyLink(inputLink);
    }
err_connected:
    return;
}

void NEXUS_VideoInput_GetColorMatrix(NEXUS_VideoInput input, NEXUS_ColorMatrix *pColorMatrix)
{
    NEXUS_VideoInput_P_Link *link;

    BDBG_OBJECT_ASSERT(input, NEXUS_VideoInput);
    link = NEXUS_VideoInput_P_Get(input);
    if (link && link->sourceVdc) {
        if (link->bColorMatrixSet) {
            /* avoid VDC race conditions and return whatever was last set by the user. */
            *pColorMatrix = link->colorMatrix;
            return;
        }
        else {
            bool bOverride;
            NEXUS_Error rc;

            /* for video inputs, the BVDC_Source exists right away, so we can ask VDC for its default. */
            rc = BVDC_Source_GetColorMatrix(link->sourceVdc, &bOverride, pColorMatrix->coeffMatrix, &pColorMatrix->shift);
            if (!rc) return;
            /* else fall through */
        }
    }

    BKNI_Memset(pColorMatrix, 0, sizeof(*pColorMatrix));

    return;
}

NEXUS_Error NEXUS_VideoInput_SetColorMatrix(NEXUS_VideoInput input, const NEXUS_ColorMatrix *pColorMatrix)
{
    NEXUS_VideoInput_P_Link *link;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(input, NEXUS_VideoInput);
    link = NEXUS_VideoInput_P_Get(input);
    if (!link) return BERR_TRACE(NEXUS_UNKNOWN);
    if (!link->sourceVdc) {link->isDeferColorMatrix = true; link->colorMatrix = *pColorMatrix; return NEXUS_SUCCESS;}

    if (pColorMatrix) {
        link->colorMatrix = *pColorMatrix;
    }
    link->bColorMatrixSet = pColorMatrix != NULL;

    rc = BVDC_Source_SetColorMatrix(link->sourceVdc, link->bColorMatrixSet, link->colorMatrix.coeffMatrix, link->colorMatrix.shift);
    if (rc) return BERR_TRACE(rc);

    rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    return 0;
}

#if NEXUS_HAS_SYNC_CHANNEL
void NEXUS_VideoInput_GetSyncSettings_priv(NEXUS_VideoInput videoInput, NEXUS_VideoInputSyncSettings *pSyncSettings)
{
    BDBG_OBJECT_ASSERT(videoInput, NEXUS_VideoInput);
    NEXUS_ASSERT_MODULE();

    switch (videoInput->type) {
#if NEXUS_HAS_VIDEO_DECODER
    case NEXUS_VideoInputType_eDecoder:
        if (!pVideo->modules.videoDecoder) {
            BERR_TRACE(NEXUS_NOT_SUPPORTED);
            return;
        }
        NEXUS_Module_Lock(pVideo->modules.videoDecoder);
        NEXUS_VideoDecoder_GetSyncSettings_priv((NEXUS_VideoDecoderHandle)videoInput->source, pSyncSettings);
        NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
        break;
#else
        BSTD_UNUSED(pSyncSettings);
#endif
    default:
        BDBG_WRN(("SyncChannel not supported for this video input"));
        break;
    }

    /* two of the callbacks are handled by the input class, the rest are handled per input type */
    pSyncSettings->connectionChangedCallback_isr = videoInput->sync.connectionChangedCallback_isr;
    pSyncSettings->callbackContext = videoInput->sync.callbackContext;
}

NEXUS_Error NEXUS_VideoInput_SetSyncSettings_priv(NEXUS_VideoInput videoInput, const NEXUS_VideoInputSyncSettings *pSyncSettings)
{
    NEXUS_Error rc = 0;

    BDBG_OBJECT_ASSERT(videoInput, NEXUS_VideoInput);
    NEXUS_ASSERT_MODULE();

    /* two of the callbacks are handled by the input class, the rest are handled per input type */
    videoInput->sync.connectionChangedCallback_isr = pSyncSettings->connectionChangedCallback_isr;
    videoInput->sync.callbackContext = pSyncSettings->callbackContext;

    switch (videoInput->type) {
#if NEXUS_HAS_VIDEO_DECODER
    case NEXUS_VideoInputType_eDecoder:
        if (!pVideo->modules.videoDecoder) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        NEXUS_Module_Lock(pVideo->modules.videoDecoder);
        rc = NEXUS_VideoDecoder_SetSyncSettings_priv((NEXUS_VideoDecoderHandle)videoInput->source, pSyncSettings);
        NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
        break;
#else
        BSTD_UNUSED(pSyncSettings);
#endif
    default:
        BDBG_WRN(("SyncChannel not supported for this video input"));
        break;
    }
    return rc;
}

NEXUS_Error NEXUS_VideoInput_GetSyncStatus_isr(NEXUS_VideoInput videoInput, NEXUS_VideoInputSyncStatus *pSyncStatus)
{
    NEXUS_Error rc = 0;

    BDBG_OBJECT_ASSERT(videoInput, NEXUS_VideoInput);

    switch (videoInput->type) {
#if NEXUS_HAS_VIDEO_DECODER
    case NEXUS_VideoInputType_eDecoder:
        rc = NEXUS_VideoDecoder_GetSyncStatus_isr((NEXUS_VideoDecoderHandle)videoInput->source, pSyncStatus);
        break;
#else
        BSTD_UNUSED(pSyncStatus);
#endif
    default:
        BDBG_WRN(("SyncChannel not supported for this video input"));
        break;
    }
    return rc;
}
#endif /* NEXUS_HAS_SYNC_CHANNEL */

NEXUS_Error NEXUS_Display_P_GetWindows_priv(NEXUS_VideoInput videoInput, NEXUS_VideoWindowHandle *pWindowArray, unsigned arraySize, unsigned *numFilled)
{
    unsigned i,j;

    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(videoInput, NEXUS_VideoInput);

    *numFilled = 0;
    for (i=0;i<NEXUS_NUM_DISPLAYS;i++) {
        for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++) {
            if (*numFilled == arraySize) return 0;
            if (pVideo->displays[i] && pVideo->displays[i]->windows[j].open) {
                NEXUS_VideoWindowHandle window = &pVideo->displays[i]->windows[j];
#if NEXUS_NUM_MOSAIC_DECODES
                NEXUS_VideoWindowHandle mosaicWindow;
#endif
                if (window->input == videoInput) {
                    pWindowArray[(*numFilled)++] = window;
                }
#if NEXUS_NUM_MOSAIC_DECODES
                for (mosaicWindow = BLST_S_FIRST(&window->mosaic.children); mosaicWindow; mosaicWindow = BLST_S_NEXT(mosaicWindow, mosaic.link)) {
                    if (*numFilled == arraySize) return 0;
                    if (mosaicWindow->input == videoInput) {
                        pWindowArray[(*numFilled)++] = mosaicWindow;
                    }
                }
#endif
            }
        }
    }
    return 0;
}

void
NEXUS_Display_P_VideoInputDisplayUpdate(NEXUS_DisplayHandle display, NEXUS_VideoWindowHandle window, const NEXUS_DisplaySettings *pSettings)
{
    NEXUS_VideoInput videoInput = NULL;
    NEXUS_VideoInput_P_Link *link = NULL;
#if NEXUS_NUM_VIDEO_DECODERS
    bool bRefreshRateDriver = false;
    NEXUS_VideoDecoder_DisplayInformation displayInformation;
#if NEXUS_NUM_MOSAIC_DECODES
    BVDC_Window_Status vdcWindowStatus;
    NEXUS_VideoWindowHandle mosaicChild;
#endif
#endif

    if (!window) {
        display = pVideo->displayDrivingVideoDecoder;
        if (!display) return;
    }
    else {
        BDBG_OBJECT_ASSERT(window, NEXUS_VideoWindow);
        display = window->display;
        videoInput = window->input;
        if (videoInput)
            link = videoInput->destination;
        BDBG_OBJECT_ASSERT(videoInput, NEXUS_VideoInput);
    }
    BDBG_ASSERT(pSettings);

#if NEXUS_NUM_VIDEO_DECODERS
    BKNI_Memset(&displayInformation, 0, sizeof(displayInformation));
    if(display->timingGenerator == NEXUS_DisplayTimingGenerator_eEncoder) {
        displayInformation.stgIndex = display->stgIndex;
        BDBG_MSG(("display stg%d", displayInformation.stgIndex));
    }
    /* mtgSrc will not be syncLocked, and will update refreshRate with sourceChange */
    displayInformation.refreshRate = display->status.refreshRate / 10;
    if (link && !link->vdcSourceCallbackData.bMtgSrc) {
        if (link->vdcSourceCallbackData.ulVertRefreshRate == displayInformation.refreshRate) {
            /* this src might not be syncLocked to any display because connected displays are all syncLocked to other src */
            bRefreshRateDriver = true;
        }
#if NEXUS_NUM_MOSAIC_DECODES
        else if (window->mosaic.parent) {
            NEXUS_VideoInput_P_Link *parentLink;
            NEXUS_VideoWindowHandle parentWindow = window->mosaic.parent;
            NEXUS_VideoInput parentVideoInput = parentWindow->input;
            if (parentVideoInput && (parentLink = parentVideoInput->destination))
            {
                /* for mosaic window with MTG HW capable, vdc reports bMtgSrc as false in src callBack. and this src is never
                 * syncLocked to any display, but the refreshRate in src callBack data is always correct */
                displayInformation.refreshRate = parentLink->vdcSourceCallbackData.ulVertRefreshRate;
                bRefreshRateDriver = true;
            }
        }
#endif
    }
    BDBG_MSG(("display refreshRate %d", displayInformation.refreshRate));

    if (!videoInput) {
        NEXUS_Module_Lock(pVideo->modules.videoDecoder);
        /* the last displayInformation will be appiled to all gfd video decoder? */
        NEXUS_VideoDecoder_UpdateDefaultDisplayInformation_priv(&displayInformation);
        NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
        return;
    }

#if NEXUS_NUM_MOSAIC_DECODES
    if (window) {
        /* pass along to each mosaic child */
        for (mosaicChild = BLST_S_FIRST(&window->mosaic.children); mosaicChild; mosaicChild = BLST_S_NEXT(mosaicChild, mosaic.link)) {
            if (mosaicChild->input) {
                NEXUS_Display_P_VideoInputDisplayUpdate(NULL, mosaicChild, pSettings);
            }
        }
    }
#endif
#endif

    switch (videoInput->type) {
#if NEXUS_HAS_VIDEO_DECODER && NEXUS_NUM_VIDEO_DECODERS
    case NEXUS_VideoInputType_eDecoder:
        if (!pVideo->modules.videoDecoder) {
            BERR_TRACE(NEXUS_NOT_SUPPORTED);
            return;
        }

#if NEXUS_HAS_VIDEO_ENCODER
        {
            NEXUS_VideoDecoderDisplayConnection decoderConnect;

            NEXUS_Module_Lock(pVideo->modules.videoDecoder);
            NEXUS_VideoDecoder_GetDisplayConnection_priv(videoInput->source, &decoderConnect);
            decoderConnect.userDataCallback_isr = (display->encodeUserData || decoderConnect.userDataCallback_isr)
                ?NEXUS_VideoInput_P_UserDataCallback_isr:NULL;
            BDBG_MSG(("input[%p] -> display[%p] encode cc?%d, cb=%p", (void *)videoInput, (void *)display, display->encodeUserData, (void *)(unsigned long)decoderConnect.userDataCallback_isr));
            NEXUS_VideoDecoder_SetDisplayConnection_priv(videoInput->source, &decoderConnect);
            NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
        }
#endif

#if NEXUS_NUM_MOSAIC_DECODES
        if (window && window->mosaic.parent) {
            NEXUS_VideoWindowHandle parentWindow = window->mosaic.parent;

            /* mosaic windows have no VDC window and sync-locked is handled with BVDC_Window_SetMosaicTrack, so always pass the info along. */
            if (!bRefreshRateDriver && (!parentWindow->vdcState.window || BVDC_Window_GetStatus(parentWindow->vdcState.window, &vdcWindowStatus) || !vdcWindowStatus.bSyncLock )) {
                return;
            }
        }
        /* if the window is not sync-locked, don't feed the display info back to the video decoder input */
        else
#endif
        {
            /* this code exists to filter out which display's rate is communicated, not which decoder it's communicated to */
            if (!bRefreshRateDriver && window && (!window->vdcState.window || !window->status.isSyncLocked || link->vdcSourceCallbackData.bMtgSrc)) {
                return;
            }
        }

        NEXUS_Module_Lock(pVideo->modules.videoDecoder);
        NEXUS_VideoDecoder_UpdateDisplayInformation_priv((NEXUS_VideoDecoderHandle)videoInput->source, &displayInformation);
        NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
        break;
#endif
    case NEXUS_VideoInputType_eImage:
        NEXUS_VideoImageInput_P_UpdateDisplayInformation((NEXUS_VideoImageInputHandle)videoInput->source, pSettings);
        break;
    default:
        break;
    }
    return;
}

NEXUS_Error NEXUS_VideoInput_GetCrcData( NEXUS_VideoInput input, NEXUS_VideoInputCrcData *pData, unsigned numEntries, unsigned *numEntriesReturned )
{
    NEXUS_VideoInput_P_Link *link;
    BDBG_OBJECT_ASSERT(input, NEXUS_VideoInput);

    link = NEXUS_VideoInput_P_Get(input);

    *numEntriesReturned = 0;

    if (!link) return 0;

    if (pData == NULL) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return 0;
    }

    /* no critical section needed for this type of producer/consumer */
    while (*numEntriesReturned < numEntries && link->crc.wptr != link->crc.rptr && link->crc.queue) {
        pData[*numEntriesReturned] = link->crc.queue[link->crc.rptr];
        if (++link->crc.rptr == link->crc.size) {
            link->crc.rptr = 0;
        }
        (*numEntriesReturned)++;
    }
    return 0;
}

#if NEXUS_HAS_VIDEO_ENCODER && NEXUS_NUM_DSP_VIDEO_ENCODERS
NEXUS_Error NEXUS_VideoInput_P_ForceFrameCapture(
    NEXUS_VideoInput videoInput, bool force
    )
{
    NEXUS_VideoInput_P_Link *link;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(videoInput, NEXUS_VideoInput);
    link = NEXUS_VideoInput_P_Get(videoInput);
    if (!link) {return BERR_TRACE(NEXUS_UNKNOWN);}
    if (!link->sourceVdc) {return BERR_TRACE(NEXUS_UNKNOWN);}

    rc = BVDC_Source_ForceFrameCapture(link->sourceVdc, force);
    if (rc) return BERR_TRACE(rc);
    return NEXUS_SUCCESS;
}
#endif

void NEXUS_VideoInput_P_CheckFormatChange_isr(void *pParam)
{
    NEXUS_VideoInput_P_Link *pLink = pParam;

    BDBG_ASSERT(NULL != pLink);

    if ( pLink->checkFormatChangedEvent )
    {
        BKNI_SetEvent_isr(pLink->checkFormatChangedEvent);
    }
}

bool nexus_p_input_is_mtg(NEXUS_VideoInput_P_Link *link)
{
    return link->mtg == BVDC_Mode_eAuto && link->id <= BAVC_SourceId_eMpegMax && g_pCoreHandles->boxConfig->stVdc.astSource[link->id].bMtgCapable;
}
