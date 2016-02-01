/***************************************************************************
 *     (c)2007-2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#include "nexus_base.h"
#include "nexus_display_module.h"
#include "priv/nexus_surface_priv.h"
#include "nexus_power_management.h"
#if NEXUS_HAS_VIDEO_DECODER
#include "bxdm_pp.h"
#include "bxdm_decoder.h"
#include "bxdm_picture.h"
#endif
#include "blst_squeue.h"
#include "priv/nexus_stc_channel_priv.h"

BDBG_MODULE(nexus_video_image_input);

#define BDBG_MSG_ISR(X) /* BDBG_MSG(X)  */
#define pVideo (&g_NEXUS_DisplayModule_State)

#if NEXUS_HAS_VIDEO_DECODER
typedef struct NEXUS_VideoImageInputPicture
{
    BXDM_Picture xdmPicture;
    BLST_SQ_ENTRY(NEXUS_VideoImageInputPicture) node;
    NEXUS_SurfaceHandle hSurface;
} NEXUS_VideoImageInputPicture;
#endif

struct NEXUS_VideoImageInput {
    NEXUS_OBJECT(NEXUS_VideoImageInput);
    NEXUS_VideoInputObject input;
    NEXUS_TaskCallbackHandle  imageCallback;
    unsigned index;
    NEXUS_VideoImageInputSettings settings;
    bool connected;

    int curPic, quePic;
    int lastFree;
    int waitingPics;               /* keep tabs on pics waiting to be displayed */
    BFMT_VideoFmt displayFormat;

    struct tagfeeder {
        BAVC_SourceId sourceId;
        struct {
           const BPXL_Plane *frame;
        } surface;
        unsigned   age;            /* how long since pic was displayed */
        unsigned   primed;         /* set to Vsync display count when an image is loaded into queue, not cleared until image removed from queue */
        unsigned   loaded;         /* set to Vsync display count when pic is displayed, decremented until =1 while displayed */
        bool       displayed;      /* Set to true while pic is actively displayed */
        bool       freed;          /* Set to true when pic has transitioned from displayed to non-displayed and can be freed */
        NEXUS_SurfaceHandle usrSurface;
        NEXUS_SurfaceHandle freedSurface;
        BAVC_Gfx_Picture pic;
        BAVC_MFD_Picture mfdPic;
        BMMA_DeviceOffset lumaOffset;
        BMMA_DeviceOffset chromaOffset;
    } *feeder;
    int fifoSize;

    BVDC_Source_Handle source;
    BKNI_EventHandle event;
    NEXUS_EventCallbackHandle eventHandler;
    NEXUS_VideoInput_P_Link *pLink;

#if NEXUS_HAS_VIDEO_DECODER
    struct
    {
        BXDM_PictureProvider_Handle provider;
        BXDM_DisplayInterruptHandler_Handle dih;
        bool eos;
        BLST_SQ_HEAD(ReadyQueue, NEXUS_VideoImageInputPicture) readyQueue;
        BLST_SQ_HEAD(DispQueue, NEXUS_VideoImageInputPicture) displayQueue;
        BLST_SQ_HEAD(RetQueue, NEXUS_VideoImageInputPicture) returnQueue;
        BLST_SQ_HEAD(RecQueue, NEXUS_VideoImageInputPicture) recycleQueue;
        BLST_SQ_HEAD(FreeList, NEXUS_VideoImageInputPicture) freeList;
        NEXUS_VideoImageInputPicture *pPictures;
        NEXUS_IsrCallbackHandle ptsError;
        NEXUS_IsrCallbackHandle firstPtsPassed;
        NEXUS_IsrCallbackHandle frameReturned;
        NEXUS_IsrCallbackHandle eosDone;
        unsigned serialNum;
    } xdm;
#endif
    struct
    {
        NEXUS_StcChannelDecoderConnectionHandle connector;
        unsigned priority;
    } stc;
};

#if NEXUS_HAS_VIDEO_DECODER
/* Returns the number of unique base pictures available in the decoder's picture delivery queue */
static BERR_Code NEXUS_VideoImageInput_P_GetPictureCount_isr(void *pvHandle, uint32_t *puiPictureCount);

/* Returns a pointer to the specified unified picture on the decoder's picture delivery queue,
* but DOES NOT remove it from the queue */
static BERR_Code NEXUS_VideoImageInput_P_PeekAtPicture_isr(void *pvHandle, uint32_t uiIndex, const BXDM_Picture **pUnifiedPicture);

/* Returns a pointer to the next unified picture on the decoder's picture delivery queue
* and DOES remove it from the queue */
static BERR_Code NEXUS_VideoImageInput_P_GetNextPicture_isr(void *pvHandle, const BXDM_Picture **pUnifiedPicture);

/* Adds unified picture to the decoder's release queue */
static BERR_Code NEXUS_VideoImageInput_P_ReleasePicture_isr(void *pvHandle, const BXDM_Picture *pUnifiedPicture, const BXDM_Decoder_ReleasePictureInfo *pReleasePictureInfo);

/* Adds released unified picture to the apps's release queue */
static BERR_Code NEXUS_VideoImageInput_P_DisplayInterruptEvent_isr(void *pvHandle);

static NEXUS_Error NEXUS_VideoImageInput_P_PushSurface_xdm(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle image,
    const NEXUS_VideoImageInputSurfaceSettings *pSettings
    );

static NEXUS_Error NEXUS_VideoImageInput_P_RecycleSurface_xdm(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle *recycled,
    size_t num_entries,
    size_t *num_returned
    );

static NEXUS_Error NEXUS_VideoImageInput_P_CheckSurfaceCompletion_xdm(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle image
    );

static NEXUS_Error NEXUS_VideoImageInput_P_SetNextSurface_xdm(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle image
    );

static NEXUS_Error NEXUS_VideoImageInput_P_SetSurface_xdm(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle image
    );

static void NEXUS_VideoImageInput_P_FirstPtsReady_isr(void *pContext, int32_t unused, const BXDM_PictureProvider_PTSInfo *pPTSInfo);
static void NEXUS_VideoImageInput_P_FirstPtsPassed_isr(void *pPrivateContext, int32_t unused, const BXDM_PictureProvider_PTSInfo *pstPTSInfo);
static void NEXUS_VideoImageInput_P_PtsError_isr(void *pContext, int32_t unused, const BXDM_PictureProvider_PTSInfo *pPTSInfo );
static BERR_Code NEXUS_VideoImageInput_P_GetPts_isr(void *pContext, BAVC_PTSInfo *pPTSInfo);
static BERR_Code NEXUS_VideoImageInput_P_GetCdbLevel_isr(void *pContext, unsigned *pCdbLevel);
static BERR_Code NEXUS_VideoImageInput_P_StcValid_isr(void *pContext);
static BERR_Code NEXUS_VideoImageInput_P_SetPcrOffset_isr(void *pContext, uint32_t pcrOffset);
static BERR_Code NEXUS_VideoImageInput_P_GetPcrOffset_isr(void *pContext, uint32_t *pPcrOffset);
static void NEXUS_VideoImageInput_P_ApplyDisplayInformation(NEXUS_VideoImageInputHandle imageInput);

static NEXUS_Error NEXUS_VideoImageInput_P_SendEos(NEXUS_VideoImageInputHandle imageInput);

static BERR_Code NEXUS_VideoImageInput_P_Picture_isr(void *pContext, int32_t unused, BAVC_MFD_Picture *pFieldData);
#endif

static NEXUS_Error NEXUS_VideoImageInput_P_CleanUpSurfaces( NEXUS_VideoImageInputHandle imageInput );

void
NEXUS_Display_P_ImageInputNext_isr(void  *disp, int unused, BAVC_Polarity  polarity, BAVC_SourceState  state, void **picture)
{
    NEXUS_VideoImageInputHandle imageInput = disp;

    BSTD_UNUSED(unused);
    BSTD_UNUSED(state);
    BSTD_UNUSED(polarity);
    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);
    BDBG_ASSERT(picture);

    *picture = NULL;

    if ( imageInput->feeder[imageInput->curPic].primed  ) {
        BAVC_Gfx_Picture pic;
        BERR_Code rc;

        rc = BVDC_Source_GetSurface_isr(imageInput->source, &pic);
        if (BERR_SUCCESS==rc) {
            if ((pic.pSurface && pic.pSurface == imageInput->feeder[imageInput->curPic].surface.frame) ||
                (pic.pstMfdPic && pic.pstMfdPic == imageInput->feeder[imageInput->curPic].pic.pstMfdPic)) {
                int i,freed=0;
                imageInput->feeder[imageInput->curPic].age++;
                imageInput->feeder[imageInput->curPic].displayed = true;
                if ( imageInput->feeder[imageInput->curPic].loaded ) {
                    imageInput->feeder[imageInput->curPic].loaded--;
                }


                for ( i=0; i < imageInput->fifoSize ; i++ ) {
                    if ( i != imageInput->curPic && imageInput->feeder[i].displayed ) {
                        imageInput->feeder[i].displayed = false;
                        imageInput->feeder[i].freed     = true;
                        freed++;
                    }
                }

                if ( imageInput->waitingPics || freed ) {
                    /* Need to fire an event for new pic to be queued out of isr time */
                    /* and it should be ready in time for the next v sysnc */
                    BKNI_SetEvent(imageInput->event);
                }

            }
        }
    }
    else if ( imageInput->feeder[imageInput->curPic].age && imageInput->feeder[imageInput->curPic].usrSurface==NULL ) {
        imageInput->feeder[imageInput->curPic].age++;
    }

    return;
}

static void
NEXUS_ImageInput_P_EventHandler(void *cntx)
{
    NEXUS_VideoImageInputHandle imageInput = cntx;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;
    BERR_Code rc;

    if ( imageInput->feeder[imageInput->curPic].usrSurface ) {
        /* advance to next picture if available */
        if ( imageInput->feeder[imageInput->curPic].displayed ) {
            BDBG_MSG_ISR(("EvntHdlr, waitPics=%d curPic=%d.loaded=%d" , imageInput->waitingPics, imageInput->curPic, imageInput->feeder[imageInput->curPic].loaded ));
            if ( imageInput->feeder[imageInput->curPic].loaded <= 1  ) { /* count of one or less, means vsync count satisfied */
                BKNI_EnterCriticalSection();

                if ( imageInput->waitingPics ) {
                    imageInput->waitingPics--;
                    /* If anything else is waiting advance to it */
                    imageInput->curPic++;
                    if ( imageInput->curPic >= imageInput->fifoSize ) imageInput->curPic = 0;
                    imageInput->feeder[imageInput->curPic].loaded = imageInput->feeder[imageInput->curPic].primed;

                    BKNI_LeaveCriticalSection();

                    rc = BVDC_Source_SetSurface( imageInput->source , &imageInput->feeder[imageInput->curPic].pic );
                    if (rc!=BERR_SUCCESS) { BERR_TRACE(rc);}

                    rc = BVDC_ApplyChanges(video->vdc);
                    if (rc!=BERR_SUCCESS) { BERR_TRACE(rc);}


                    NEXUS_TaskCallback_Fire(imageInput->imageCallback);
                }
                else
                    BKNI_LeaveCriticalSection();
            }
        }
    }
    else {
        /* No surface is set kick start the process */
        BKNI_EnterCriticalSection();

        if ( imageInput->waitingPics ) {
            imageInput->waitingPics--;
            BDBG_MSG_ISR(("EvntHdlr(no usr surface), waitPics=%d curPic=%d.loaded=%d" , imageInput->waitingPics, imageInput->curPic, imageInput->feeder[imageInput->curPic].loaded ));
            /* If anything else is waiting advance to it */
            imageInput->curPic++;
            if ( imageInput->curPic >= imageInput->fifoSize ) imageInput->curPic = 0;
            imageInput->feeder[imageInput->curPic].loaded = imageInput->feeder[imageInput->curPic].primed;

            BKNI_LeaveCriticalSection();

            rc = BVDC_Source_SetSurface( imageInput->source , &imageInput->feeder[imageInput->curPic].pic );
            if (rc!=BERR_SUCCESS) { BERR_TRACE(rc);}

            rc = BVDC_ApplyChanges(video->vdc);
            if (rc!=BERR_SUCCESS) { BERR_TRACE(rc);}
        }
        else
            BKNI_LeaveCriticalSection();

    }
}

static void NEXUS_ImageInput_P_GetDefaultGfxPic(BAVC_Gfx_Picture *pGfxPic)
{
    BDBG_ASSERT(pGfxPic);
    BKNI_Memset(pGfxPic, 0, sizeof(BAVC_Gfx_Picture));
    pGfxPic->pSurface = NULL;
    pGfxPic->pstMfdPic = NULL;
    pGfxPic->eInOrientation = NEXUS_P_VideoOrientation_ToMagnum_isrsafe(NEXUS_VideoOrientation_e2D);
}

void
NEXUS_VideoImageInput_GetDefaultSurfaceSettings(NEXUS_VideoImageInputSurfaceSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->displayVsyncs = 1;
    pSettings->sourceFormat = NEXUS_PictureScan_eProgressive;
    pSettings->sequence = NEXUS_PictureScan_eProgressive;
    return;
}

void
NEXUS_VideoImageInput_GetDefaultSettings(NEXUS_VideoImageInputSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_CALLBACKDESC_INIT(&pSettings->imageCallback);
#if defined NEXUS_VIDEO_IMAGE_INPUT_1080P_FEED || BCHP_CHIP == 3548 || BCHP_CHIP == 3556
    /* macro and BCHP_CHIP support for backward compatibility */
    pSettings->progressiveScan1080 = true;
#endif
    pSettings->fifoSize = 16;   /* Worst case for xdm mode */
    pSettings->type = NEXUS_VideoImageInput_eMfd;
    pSettings->lowDelayMode = true;
    pSettings->tsmEnabled = true;
    return;
}

NEXUS_VideoImageInputHandle
NEXUS_VideoImageInput_Open(unsigned index, const NEXUS_VideoImageInputSettings *pSettings)
{
    NEXUS_VideoImageInputHandle  imageInput;
    NEXUS_VideoImageInputSettings settings;
#if NEXUS_HAS_VIDEO_DECODER
    BXDM_PictureProvider_Settings providerSettings;
#endif
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;
    NEXUS_Error rc;
    int i,checkedFifoSize;
    unsigned srcIndex;

    if(!pSettings) {
        NEXUS_VideoImageInput_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    switch (pSettings->type) {
    case NEXUS_VideoImageInput_eVfd:
        if (index >= video->vdcCapabilities.ulNumVfd) {
            BDBG_ERR(("VFD%d is not available.", index));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto err_index;
        }
        srcIndex = BAVC_SourceId_eVfd0 + index;
        break;
    case NEXUS_VideoImageInput_eMfd:
        if (index >= video->vdcCapabilities.ulNumMfd) {
            BDBG_ERR(("MFD%d is not available.", index));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto err_index;
        }
        srcIndex = BAVC_SourceId_eMpeg0 + index;
        break;
    default:
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_index;
    }

    imageInput = BKNI_Malloc(sizeof(*imageInput));
    if(!imageInput) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_alloc;
    }
    NEXUS_OBJECT_INIT(NEXUS_VideoImageInput, imageInput);
    imageInput->index = index;
    NEXUS_VIDEO_INPUT_INIT(&imageInput->input, NEXUS_VideoInputType_eImage, imageInput);

    checkedFifoSize = (int)pSettings->fifoSize;
    if( (int)pSettings->fifoSize < 2 ) {
        BDBG_MSG(("Overriding fifosize to min of 2 !"));
        checkedFifoSize = 2;    /* minimum queue size */
    }

    imageInput->imageCallback = NEXUS_TaskCallback_Create( imageInput, NULL);
    if ( NULL == imageInput->imageCallback )
    {
        rc = BERR_TRACE(BERR_OS_ERROR);
        goto err_callback;
    }


    rc = BKNI_CreateEvent(&imageInput->event);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_event;}
    imageInput->eventHandler = NEXUS_RegisterEvent(imageInput->event, NEXUS_ImageInput_P_EventHandler, imageInput );
    if(!imageInput->eventHandler) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_event_handler;}

    imageInput->feeder = BKNI_Malloc(sizeof(struct tagfeeder) * checkedFifoSize );

    if(!imageInput->feeder) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_feeder;
    }
    BKNI_Memset(imageInput->feeder, 0, sizeof(struct tagfeeder) * checkedFifoSize );

    imageInput->fifoSize = checkedFifoSize;

    for ( i=0; i < imageInput->fifoSize ; i++ ) {
        imageInput->feeder[i].sourceId = srcIndex;
        imageInput->feeder[i].usrSurface   = NULL;
        imageInput->feeder[i].freedSurface = NULL;
    }

    imageInput->settings = *pSettings;

#if NEXUS_HAS_VIDEO_DECODER
    /* Initialize components for XDM display path */
    imageInput->xdm.ptsError = NEXUS_IsrCallback_Create(imageInput, NULL);
    if ( NULL == imageInput->xdm.ptsError )
    {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_pts_error;
    }
    imageInput->xdm.firstPtsPassed = NEXUS_IsrCallback_Create(imageInput, NULL);
    if ( NULL == imageInput->xdm.firstPtsPassed )
    {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_first_pts_passed;
    }
    imageInput->xdm.frameReturned = NEXUS_IsrCallback_Create(imageInput, NULL);
    if ( NULL == imageInput->xdm.frameReturned )
    {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto frame_returned;
    }
    imageInput->xdm.eosDone = NEXUS_IsrCallback_Create(imageInput, NULL);
    if ( NULL == imageInput->xdm.eosDone )
    {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto eos_done;
    }

    BXDM_PictureProvider_GetDefaultSettings(&providerSettings);
    rc = BXDM_PictureProvider_Create(&imageInput->xdm.provider, &providerSettings);
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        goto err_provider;
    }
    BKNI_EnterCriticalSection();
    {
        BXDM_Decoder_Interface decInterface;
        BKNI_Memset(&decInterface, 0, sizeof(decInterface));
        decInterface.getPictureCount_isr = NEXUS_VideoImageInput_P_GetPictureCount_isr;
        decInterface.peekAtPicture_isr = NEXUS_VideoImageInput_P_PeekAtPicture_isr;
        decInterface.getNextPicture_isr = NEXUS_VideoImageInput_P_GetNextPicture_isr;
        decInterface.releasePicture_isr = NEXUS_VideoImageInput_P_ReleasePicture_isr;
        decInterface.displayInterruptEvent_isr = NEXUS_VideoImageInput_P_DisplayInterruptEvent_isr;
        rc = BXDM_PictureProvider_SetDecoderInterface_isr(imageInput->xdm.provider, &decInterface, imageInput);
    }
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        BKNI_LeaveCriticalSection();
        goto err_dec_interface;
    }
    BXDM_PictureProvider_Callback_Install_FirstCodedPTSReady_isr(imageInput->xdm.provider, NEXUS_VideoImageInput_P_FirstPtsReady_isr, imageInput, 0);
    BXDM_PictureProvider_Callback_Install_FirstPTSPassed_isr(imageInput->xdm.provider, NEXUS_VideoImageInput_P_FirstPtsPassed_isr, imageInput, 0);
    BXDM_PictureProvider_Callback_Install_PTSError_isr(imageInput->xdm.provider, NEXUS_VideoImageInput_P_PtsError_isr, imageInput, 0);
    BXDM_PictureProvider_SetPictureDropMode_isr(imageInput->xdm.provider, BXDM_PictureProvider_PictureDropMode_eFrame);
    {
        BXDM_PictureProvider_TSMThresholdSettings threshold;
        threshold.uiTooEarlyThreshold = 0;
        threshold.uiTooLateThreshold = 0;
        threshold.uiDeltaStcPtsDiffThreshold = 10;
        BXDM_PictureProvider_SetTSMThresholdSettings_isr(imageInput->xdm.provider, &threshold);
    }
    BXDM_PictureProvider_SetMonitorRefreshRate_isr(imageInput->xdm.provider, BXDM_PictureProvider_MonitorRefreshRate_e60Hz);
    BXDM_PictureProvider_Set1080pScanMode_isr(imageInput->xdm.provider, BXDM_PictureProvider_1080pScanMode_eDefault);
    BXDM_PictureProvider_SetFreeze_isr(imageInput->xdm.provider, false);
    BXDM_PictureProvider_SetPTSOffset_isr(imageInput->xdm.provider, 0);
    BXDM_PictureProvider_SetMuteMode_isr(imageInput->xdm.provider, false);
    BXDM_PictureProvider_SetSTCValid_isr(imageInput->xdm.provider, true);
    BXDM_PictureProvider_SetSTCIndex_isr(imageInput->xdm.provider, 0);
    BXDM_PictureProvider_SetPlaybackMode_isr(imageInput->xdm.provider, true);
    BXDM_PictureProvider_SetFrameRateDetectionMode_isr(imageInput->xdm.provider, BXDM_PictureProvider_FrameRateDetectionMode_eStable);
    BXDM_PictureProvider_SetASTMMode_isr(imageInput->xdm.provider, false);
    BXDM_PictureProvider_SetVirtualTSMOnPCRDiscontinuityMode_isr(imageInput->xdm.provider, true);
    BXDM_PictureProvider_SetChannelSyncMode_isr(imageInput->xdm.provider, true);
    BXDM_PictureProvider_SetErrorHandlingMode_isr(imageInput->xdm.provider, BXDM_PictureProvider_ErrorHandlingMode_eOff);
    BXDM_PictureProvider_Set480pPulldownMode_isr(imageInput->xdm.provider, BXDM_PictureProvider_PulldownMode_eUseEncodedFormat);
    BXDM_PictureProvider_SetJitterToleranceImprovementMode_isr(imageInput->xdm.provider, false);
    BKNI_LeaveCriticalSection();

    imageInput->xdm.pPictures = BKNI_Malloc(sizeof(NEXUS_VideoImageInputPicture) * imageInput->fifoSize);
    if ( NULL == imageInput->xdm.pPictures )
    {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_pictures;
    }
    BKNI_Memset(imageInput->xdm.pPictures, 0, sizeof(NEXUS_VideoImageInputPicture) * imageInput->fifoSize);
    BLST_SQ_INIT(&imageInput->xdm.readyQueue);
    BLST_SQ_INIT(&imageInput->xdm.displayQueue);
    BLST_SQ_INIT(&imageInput->xdm.returnQueue);
    BLST_SQ_INIT(&imageInput->xdm.recycleQueue);
    BLST_SQ_INIT(&imageInput->xdm.freeList);
    imageInput->xdm.serialNum = 0;
    for ( i = 0; i < imageInput->fifoSize; i++ )
    {
        BLST_SQ_INSERT_TAIL(&imageInput->xdm.freeList, &imageInput->xdm.pPictures[i], node);
    }
#endif

    imageInput->curPic = imageInput->quePic = imageInput->lastFree = 0;
    imageInput->feeder[0].primed = 1;  /* for kickstart */
    NEXUS_OBJECT_REGISTER(NEXUS_VideoInput, &imageInput->input, Open);

    /* Make sure all settings are sync'ed */
    (void)NEXUS_VideoImageInput_SetSettings(imageInput, pSettings);

    return imageInput;

#if NEXUS_HAS_VIDEO_DECODER
err_pictures:
err_dec_interface:
    (void)BXDM_PictureProvider_Destroy(imageInput->xdm.provider);
err_provider:
    NEXUS_IsrCallback_Destroy(imageInput->xdm.eosDone);
eos_done:
    NEXUS_IsrCallback_Destroy(imageInput->xdm.frameReturned);
frame_returned:
    NEXUS_IsrCallback_Destroy(imageInput->xdm.firstPtsPassed);
err_first_pts_passed:
    NEXUS_IsrCallback_Destroy(imageInput->xdm.ptsError);
err_pts_error:
    BKNI_Free(imageInput->feeder);
#endif
err_feeder:
    NEXUS_UnregisterEvent(imageInput->eventHandler);
err_event_handler:
    BKNI_DestroyEvent(imageInput->event);
err_event:
    NEXUS_TaskCallback_Destroy(imageInput->imageCallback);
err_callback:
    BKNI_Free(imageInput);
err_alloc:
err_index:
    return NULL;
}

static NEXUS_Error NEXUS_VideoImageInput_P_CleanUpSurfaces( NEXUS_VideoImageInputHandle imageInput )
{
    NEXUS_Error rc=NEXUS_SUCCESS;
    int i,j;

    j = imageInput->curPic; /* work backwards from current pic which can't be freed */
    for ( i=0; i < imageInput->fifoSize; i++ ) {
        j--;
        if ( j < 0 ) j = imageInput->fifoSize-1;
        if ( imageInput->feeder[j].freed && j != imageInput->curPic ) {
            const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;

            if (imageInput->feeder[j].usrSurface && NULL==imageInput->feeder[j].pic.pstMfdPic) {
                NEXUS_Module_Lock(video->modules.surface);
                NEXUS_Surface_GetPixelPlane_priv(imageInput->feeder[j].usrSurface);
                NEXUS_Module_Unlock(video->modules.surface);
            }
            if(imageInput->feeder[j].pic.pstMfdPic) {/* unlock device memory before free */
                BMMA_UnlockOffset(imageInput->feeder[j].pic.pstMfdPic->hLuminanceFrameBufferBlock, imageInput->feeder[j].lumaOffset);
                BMMA_UnlockOffset(imageInput->feeder[j].pic.pstMfdPic->hChrominanceFrameBufferBlock, imageInput->feeder[j].chromaOffset);
            }
            BKNI_EnterCriticalSection();
            imageInput->feeder[j].freedSurface   = imageInput->feeder[j].usrSurface;
            imageInput->feeder[j].usrSurface     = NULL;
            imageInput->feeder[j].surface.frame  = NULL;
            imageInput->feeder[j].primed = 0 ;
            imageInput->feeder[j].freed  = false;
            imageInput->feeder[j].pic.pstMfdPic  = NULL;
            BKNI_LeaveCriticalSection();
            imageInput->feeder[j].age = 0;
        }
    }
    return rc;
}

static void
NEXUS_VideoImageInput_P_ShutdownFeeder(NEXUS_VideoImageInputHandle imageInput)
{
    BERR_Code rc;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;
    int i;

    /* Before letting go of the surface, verify it's still allocated. Users should not destroy the surface
    before either calling NEXUS_VideoImageInput_SetSurface(imageInput, NULL) or closing VideoImageInput. */
    if (imageInput->feeder[imageInput->curPic].usrSurface && NULL==imageInput->feeder[imageInput->curPic].pic.pstMfdPic) {
        NEXUS_Module_Lock(video->modules.surface);
        NEXUS_Surface_GetPixelPlane_priv(imageInput->feeder[imageInput->curPic].usrSurface);
        NEXUS_Module_Unlock(video->modules.surface);
    }

    /* first clean mfd_picture and only then destroy surfaces */
    BKNI_EnterCriticalSection();
    imageInput->waitingPics = 0;                   /* Disable loading of any pics in Isr */
    imageInput->feeder[imageInput->curPic].age = 0;

    imageInput->feeder[imageInput->curPic].surface.frame  = NULL;
    imageInput->feeder[imageInput->curPic].usrSurface = NULL;
    imageInput->feeder[imageInput->curPic].pic.pstMfdPic  = NULL;
    BKNI_LeaveCriticalSection();
    if (imageInput->connected) {
        unsigned watchdog = 0;
        /* We need to see 2 vsyncs occur to make sure this surface is not being read by the MFD */
        BDBG_ASSERT(imageInput->feeder[imageInput->curPic].age == 0);
        BKNI_EnterCriticalSection();
        imageInput->feeder[imageInput->curPic].age    = 1;
        imageInput->feeder[imageInput->curPic].loaded = 1;
        imageInput->feeder[imageInput->curPic].primed = 0;
        BKNI_LeaveCriticalSection();
        while (imageInput->feeder[imageInput->curPic].age <= 2 && watchdog++ < 100) {
            BKNI_Sleep(1);
        }
        imageInput->feeder[imageInput->curPic].age    = 0;
        imageInput->feeder[imageInput->curPic].loaded = 0;
        imageInput->feeder[imageInput->curPic].displayed = 0;
        /* Now the surface is unused */
    }

    /* Now VDC isn't holding any surface, so mark for cleanup any other surfaces left over */
    for ( i=0; i < imageInput->fifoSize; i++ ) {
        imageInput->feeder[i].primed = 0;
    }
    /* Do the clean up */
    rc = NEXUS_VideoImageInput_P_CleanUpSurfaces(imageInput);
    if ( rc ) rc = BERR_TRACE(rc);

    return;
}

static void
NEXUS_VideoImageInput_P_Finalizer(NEXUS_VideoImageInputHandle imageInput)
{
    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    if(imageInput->input.destination!=NULL  || imageInput->input.ref_cnt!=0) {
        BDBG_WRN(("NEXUS_VideoImageInput_Destroy: %#lx videoInput (%#lx) is in use", (unsigned long)imageInput, (unsigned long)&imageInput->input));
        NEXUS_VideoInput_Shutdown(NEXUS_VideoImageInput_GetConnector(imageInput));
    }
    NEXUS_VideoImageInput_P_ShutdownFeeder(imageInput);
    NEXUS_TaskCallback_Destroy(imageInput->imageCallback);

    BKNI_Free(imageInput->feeder);
#if NEXUS_HAS_VIDEO_DECODER
    (void)BXDM_PictureProvider_Destroy(imageInput->xdm.provider);
    NEXUS_IsrCallback_Destroy(imageInput->xdm.firstPtsPassed);
    NEXUS_IsrCallback_Destroy(imageInput->xdm.ptsError);
    NEXUS_IsrCallback_Destroy(imageInput->xdm.eosDone);
    NEXUS_IsrCallback_Destroy(imageInput->xdm.frameReturned);
    BKNI_Free(imageInput->xdm.pPictures);
#endif
    NEXUS_UnregisterEvent(imageInput->eventHandler);
    BKNI_DestroyEvent(imageInput->event);

    NEXUS_OBJECT_DESTROY(NEXUS_VideoImageInput, imageInput);
    BKNI_Free(imageInput);
    return;
}

static void
NEXUS_VideoImageInput_P_Release(NEXUS_VideoImageInputHandle imageInput)
{
    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    NEXUS_OBJECT_UNREGISTER(NEXUS_VideoInput, &imageInput->input, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_VideoImageInput, NEXUS_VideoImageInput_Close);

NEXUS_Error NEXUS_VideoImageInput_RecycleSurface( NEXUS_VideoImageInputHandle imageInput, NEXUS_SurfaceHandle *recycled, size_t num_entries, size_t *num_returned )
{
    NEXUS_Error rc;
    int i,j,nrFreeable;

    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);

#if NEXUS_HAS_VIDEO_DECODER
    if ( !imageInput->settings.lowDelayMode )
    {
        return NEXUS_VideoImageInput_P_RecycleSurface_xdm(imageInput, recycled, num_entries, num_returned);
    }
#endif

    for ( i=0; i < (int)num_entries ; i++ ) {
        recycled[i] = NULL;
    }

    /* clean up anything changed since last time */
    rc = NEXUS_VideoImageInput_P_CleanUpSurfaces( imageInput );
    if ( rc ) rc = BERR_TRACE(rc);

    /* First go through list and see which ones are free, note their surface handles so we can pass back to caller */
    j = imageInput->lastFree;
    nrFreeable=0;
    for ( i=0; i < imageInput->fifoSize ; i++ ) {
        j++;
        if ( j >= imageInput->fifoSize ) j = 0;
        if ( imageInput->feeder[j].freedSurface ) {
            if ( nrFreeable < (int)num_entries ) {
                recycled[nrFreeable++] = imageInput->feeder[j].freedSurface;
                BDBG_MSG(("xx_Recyc recycled[nrFreeable=%d]=%p feeder[j=%d].freedSurface=%p" , nrFreeable-1, (void *)recycled[nrFreeable-1], j, (void *)imageInput->feeder[j].freedSurface ));
                BKNI_EnterCriticalSection();
                imageInput->feeder[j].freedSurface = NULL;
                imageInput->lastFree = j;
                BKNI_LeaveCriticalSection();
                if ( nrFreeable >= (int)num_entries ) break;
            }
        }
    }
    *num_returned = nrFreeable;
    if ( nrFreeable ) {
        /* Now release them from imageinput queue system */
        rc = NEXUS_VideoImageInput_P_CleanUpSurfaces( imageInput );
        if ( rc ) rc = BERR_TRACE(rc);

    }
    return rc;
}

NEXUS_Error NEXUS_VideoImageInput_CheckSurfaceCompletion( NEXUS_VideoImageInputHandle imageInput, NEXUS_SurfaceHandle image )
{
    NEXUS_Error rc;
    int i;

    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);

#if NEXUS_HAS_VIDEO_DECODER
    if ( !imageInput->settings.lowDelayMode )
    {
        return NEXUS_VideoImageInput_P_CheckSurfaceCompletion_xdm(imageInput, image);
    }
#endif

    rc = NEXUS_VideoImageInput_P_CleanUpSurfaces( imageInput );
    if ( rc ) rc = BERR_TRACE(rc);

    for ( i=0; i < imageInput->fifoSize ; i++ ) {
        /* find image to check for */
        if ( imageInput->feeder[i].usrSurface == image ) break;
    }

    if ( (i < imageInput->fifoSize) && imageInput->feeder[i].primed ) {
       return NEXUS_IMAGEINPUT_BUSY;
    }
    else {
        return BERR_SUCCESS;
    }
}

NEXUS_Error
NEXUS_VideoImageInput_PushSurface(NEXUS_VideoImageInputHandle imageInput, NEXUS_SurfaceHandle image, const NEXUS_VideoImageInputSurfaceSettings *pSettings ) {

    int i,j,availableSlots=0;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;
    const BPXL_Plane *frame=NULL;
    BERR_Code rc;
    unsigned width;
    unsigned height;
    NEXUS_SurfaceCreateSettings surfaceCfg;
    NEXUS_VideoImageInputSurfaceSettings defaultSettings;
    NEXUS_VideoInput_P_Link *link;
    BVDC_Source_Capabilities vdcCapabilities;
    BPXL_Format magnumPixelFormat;
    NEXUS_StripedSurfaceCreateSettings stripeSettings;

    BAVC_Gfx_Picture pic;

    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);

#if NEXUS_HAS_VIDEO_DECODER
    if ( !imageInput->settings.lowDelayMode )
    {
        return NEXUS_VideoImageInput_P_PushSurface_xdm(imageInput, image, pSettings);
    }
#endif

    if(image==NULL && pSettings->stripedSurface==NULL) {
        /* Passing in NULL, flushes the pipeline, first set current feeder to NULL */
        NEXUS_VideoImageInput_P_ShutdownFeeder(imageInput);
        goto done; /* shortcut */
    }

    if(!pSettings) {
        NEXUS_VideoImageInput_GetDefaultSurfaceSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* clean up any surfaces newly freed */
    rc = NEXUS_VideoImageInput_P_CleanUpSurfaces( imageInput );
    if ( rc ) rc = BERR_TRACE(rc);

    /* add new surface to queue */
    if(image) {
        NEXUS_Surface_GetCreateSettings(image, &surfaceCfg);
        width = surfaceCfg.width;
        height = surfaceCfg.height;
        if (width < 64 || height < 64) {
            BDBG_ERR(("minimum surface size for VideoImageInput is 64x64. surface %p is %dx%d", (void *)image, width, height));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto err_surface;
        }
        link = NEXUS_VideoInput_P_Get( NEXUS_VideoImageInput_GetConnector(imageInput) );
        if (!link) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);
        BDBG_ASSERT(link->sourceVdc);
        if (!link->ref_cnt) {
            BDBG_ERR(("image input not connected to window"));
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto err_surface;
        }

        rc = NEXUS_P_PixelFormat_ToMagnum_isrsafe( surfaceCfg.pixelFormat, &magnumPixelFormat);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_surface;}

        BVDC_Source_GetCapabilities(link->sourceVdc, &vdcCapabilities) ;
        if ( !vdcCapabilities.pfIsPxlfmtSupported(magnumPixelFormat) ) {
            BDBG_WRN(("pixelFormat %d of image=%p is NOT supported", surfaceCfg.pixelFormat, (void *)image ));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); /* unsupported  pixelFormat */
            goto err_surface;
        }
    } else {/* striped surface */
        BDBG_ASSERT(pSettings->stripedSurface);
        NEXUS_StripedSurface_GetCreateSettings(pSettings->stripedSurface, &stripeSettings);
    }

    if ( pSettings->infront ) {
        /* de-queue all queued surfaces except current active, add new surface in now cleared list */
        j = imageInput->curPic + 1;
        if ( j >= imageInput->fifoSize) j = 0;

        BKNI_EnterCriticalSection();
        for ( i=0; i < imageInput->fifoSize ; i++ ) {
            if ( imageInput->waitingPics ) {
                imageInput->feeder[j].freed  = true;
                imageInput->waitingPics--;
                j++;
                if ( j >= imageInput->fifoSize) j = 0;
            }
            else {
                break;
            }
        }
        BKNI_LeaveCriticalSection();

        /* clean up any surfaces newly freed */
        rc = NEXUS_VideoImageInput_P_CleanUpSurfaces( imageInput );
        if ( rc ) rc = BERR_TRACE(rc);

        BKNI_EnterCriticalSection();
        imageInput->quePic = imageInput->curPic+1;
        if ( imageInput->quePic >= imageInput->fifoSize ) imageInput->quePic = 0;
        BKNI_LeaveCriticalSection();
        availableSlots = imageInput->fifoSize - 1;
        goto doQueue;
    }
    else {
        /* check if can add to queue and do so if able */
        for ( i=0; i < imageInput->fifoSize ; i++ ) {
            if ( !imageInput->feeder[i].usrSurface && !imageInput->feeder[i].primed ) {
                availableSlots++;
                continue ;
            }
            else if ( (image && image == imageInput->feeder[i].usrSurface) ||
                (pSettings->stripedSurface && (NEXUS_SurfaceHandle)pSettings->stripedSurface == imageInput->feeder[i].usrSurface)) {
                /* passed in surface is in list, abort and return busy */
                return NEXUS_IMAGEINPUT_BUSY;
            }
        }
        if ( availableSlots == 0 ) {
            return NEXUS_IMAGEINPUT_QUEUE_FULL;
        }
        else {
            BDBG_MSG(("NEXUS_VideoImageInput_PushSurface image=%p curPic=%d available slots=%d", (void *)image, imageInput->curPic, availableSlots ));
        }
        imageInput->quePic++;
        if ( imageInput->quePic >= imageInput->fifoSize ) imageInput->quePic = 0;
    }
doQueue:

    NEXUS_ImageInput_P_GetDefaultGfxPic(&pic);
    if(image) { /* gfx surface */
        NEXUS_Module_Lock(video->modules.surface);

        frame = NEXUS_Surface_GetPixelPlane_priv(image);
        BDBG_ASSERT(frame);

        pic.pSurface = frame;
        pic.ulOrigPTS = pSettings->pts;

        NEXUS_Module_Unlock(video->modules.surface);
    }

    BKNI_EnterCriticalSection();
    if(!image) { /* striped video surface */
        BDBG_MSG(("prep the striped surface parameters..."));
        pic.pstMfdPic = &(imageInput->feeder[imageInput->quePic].mfdPic);
        pic.pstMfdPic->ePxlFmt = BPXL_INVALID;
        pic.pstMfdPic->eSourcePolarity = BAVC_Polarity_eFrame;
        pic.pstMfdPic->ulDisplayHorizontalSize = stripeSettings.imageWidth;
        pic.pstMfdPic->ulDisplayVerticalSize   = stripeSettings.imageHeight;
        pic.pstMfdPic->ulSourceHorizontalSize  = stripeSettings.imageWidth;
        pic.pstMfdPic->ulSourceVerticalSize    = stripeSettings.imageHeight;
        pic.pstMfdPic->eStripeWidth = NEXUS_P_StripeWidth_ToMagnum_isrsafe(stripeSettings.stripedWidth);
        pic.pstMfdPic->eYCbCrType = BAVC_YCbCrType_e4_2_0;
        pic.pstMfdPic->eBitDepth  = (stripeSettings.lumaPixelFormat == NEXUS_PixelFormat_eY10)?
            BAVC_VideoBitDepth_e10Bit : BAVC_VideoBitDepth_e8Bit;
        pic.pstMfdPic->ulLuminanceNMBY = stripeSettings.lumaStripedHeight/16;
        pic.pstMfdPic->ulChrominanceNMBY = stripeSettings.chromaStripedHeight/16;
        pic.pstMfdPic->hLuminanceFrameBufferBlock = NEXUS_MemoryBlock_GetBlock_priv(stripeSettings.lumaBuffer);
        pic.pstMfdPic->hChrominanceFrameBufferBlock = NEXUS_MemoryBlock_GetBlock_priv(stripeSettings.chromaBuffer);
        pic.pstMfdPic->ulLuminanceFrameBufferBlockOffset = stripeSettings.lumaBufferOffset;
        pic.pstMfdPic->ulChrominanceFrameBufferBlockOffset = stripeSettings.chromaBufferOffset;
    }
    imageInput->feeder[imageInput->quePic].usrSurface = image? image : (NEXUS_SurfaceHandle)pSettings->stripedSurface;
    imageInput->feeder[imageInput->quePic].surface.frame  = frame;
    imageInput->feeder[imageInput->quePic].freed  = false;
    imageInput->feeder[imageInput->quePic].primed = pSettings->displayVsyncs;
    imageInput->feeder[imageInput->quePic].age = 1;

    imageInput->feeder[imageInput->quePic].pic = pic;
    BDBG_MSG(("feeder[%u].pMfd=  %p", imageInput->quePic, imageInput->feeder[imageInput->quePic].pic.pstMfdPic));

    imageInput->waitingPics++;
    BKNI_LeaveCriticalSection();

    /* lock device memory before passing into VDC */
    if(!image) { /* striped video surface */
        imageInput->feeder[imageInput->quePic].lumaOffset = BMMA_LockOffset(pic.pstMfdPic->hLuminanceFrameBufferBlock);
        imageInput->feeder[imageInput->quePic].chromaOffset = BMMA_LockOffset(pic.pstMfdPic->hChrominanceFrameBufferBlock);
    }
    BDBG_MSG(("xx_PushSurf image=%p timestamp=0x%8x curPic=%d quePic=%d waitPics=%d", (void *)image , pSettings->pts, imageInput->curPic, imageInput->quePic, imageInput->waitingPics ));
    BKNI_SetEvent(imageInput->event); /* synchronise adding pictures with isr */

done:
    return BERR_SUCCESS;
err_surface:
    return rc;
}

NEXUS_Error
NEXUS_VideoImageInput_SetNextSurface(NEXUS_VideoImageInputHandle imageInput, NEXUS_SurfaceHandle image )
{
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;
    const BPXL_Plane *frame;
    BERR_Code rc;
    unsigned width;
    unsigned height;
    NEXUS_SurfaceCreateSettings surfaceCfg;
    NEXUS_VideoInput_P_Link *link;
    BVDC_Source_Capabilities vdcCapabilities;
    BPXL_Format magnumPixelFormat;
    BAVC_Gfx_Picture pic;

    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);

#if NEXUS_HAS_VIDEO_DECODER
    if ( !imageInput->settings.lowDelayMode )
    {
        return NEXUS_VideoImageInput_P_SetNextSurface_xdm(imageInput, image);
    }
#endif

    if( image && imageInput->feeder[imageInput->curPic].usrSurface==image) {
        return NEXUS_IMAGEINPUT_BUSY;
    }

    if(image==NULL) {
        /* Passing in NULL, flushes the pipeline, first set current feeder to NULL */
        NEXUS_VideoImageInput_P_ShutdownFeeder(imageInput);
        goto done; /* shortcut */
    }

    rc = NEXUS_VideoImageInput_P_CleanUpSurfaces( imageInput );
    if ( rc ) rc = BERR_TRACE(rc);

    if ( imageInput->waitingPics >= imageInput->fifoSize - 2 ) {
        return NEXUS_IMAGEINPUT_QUEUE_FULL;
    }

    NEXUS_Surface_GetCreateSettings(image, &surfaceCfg);
    width = surfaceCfg.width;
    height = surfaceCfg.height;
    if (width < 64 || height < 64) {
        BDBG_ERR(("minimum surface size for VideoImageInput is 64x64. surface %p is %dx%d", (void *)image, width, height));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_surface;
    }
    link = NEXUS_VideoInput_P_Get( NEXUS_VideoImageInput_GetConnector(imageInput) );
    if (!link) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);
    BDBG_ASSERT(link->sourceVdc);

    rc = NEXUS_P_PixelFormat_ToMagnum_isrsafe( surfaceCfg.pixelFormat, &magnumPixelFormat);
    if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_surface;}

    BVDC_Source_GetCapabilities(link->sourceVdc, &vdcCapabilities) ;
    if ( !vdcCapabilities.pfIsPxlfmtSupported(magnumPixelFormat) ) {
        BDBG_WRN(("pixelFormat %d of image=%p is NOT supported", surfaceCfg.pixelFormat, (void *)image ));
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); /* unsupported  pixelFormat */
        goto err_surface;
    }
    imageInput->quePic++;
    if ( imageInput->quePic >= imageInput->fifoSize ) imageInput->quePic = 0;

    NEXUS_Module_Lock(video->modules.surface);

    frame = NEXUS_Surface_GetPixelPlane_priv(image);
    BDBG_ASSERT(frame);

    NEXUS_Module_Unlock(video->modules.surface);

    imageInput->feeder[imageInput->quePic].usrSurface     = image;
    imageInput->feeder[imageInput->quePic].surface.frame  = frame;

    imageInput->source = link->sourceVdc;

    NEXUS_ImageInput_P_GetDefaultGfxPic(&pic);
    pic.pSurface = frame;

    BKNI_EnterCriticalSection();

    imageInput->feeder[imageInput->quePic].freed  = false;
    imageInput->feeder[imageInput->quePic].primed = 1;  /* legacy api, no Vsync count */
    imageInput->feeder[imageInput->quePic].age = 1;
    imageInput->feeder[imageInput->quePic].pic = pic;
    imageInput->waitingPics++;

    BKNI_LeaveCriticalSection();

    BKNI_SetEvent(imageInput->event); /* Let handler start new picture */

done:
    return BERR_SUCCESS;

err_surface:
    return rc;
}

NEXUS_Error
NEXUS_VideoImageInput_SetSurface(NEXUS_VideoImageInputHandle imageInput, NEXUS_SurfaceHandle image )
{
    BERR_Code rc;
    int i,cnt=6;

#if NEXUS_HAS_VIDEO_DECODER
    if ( !imageInput->settings.lowDelayMode )
    {
        return NEXUS_VideoImageInput_P_SetSurface_xdm(imageInput, image);
    }
#endif

    for(i=0;;i++) {
        /* if image_input queue is full, wait for a slot opening up */
        rc = NEXUS_VideoImageInput_SetNextSurface(imageInput, image );
        if(rc==NEXUS_SUCCESS) {
            break;
        } else if(rc==NEXUS_IMAGEINPUT_QUEUE_FULL || rc==NEXUS_IMAGEINPUT_BUSY) {
            if(i>cnt) {
                return BERR_TRACE(rc);
            }
            BKNI_Sleep(8);  /* wait half a vsync if queue full */
        } else {
            return BERR_TRACE(rc);
        }
    }

    if ( image != NULL ) {
        i = imageInput->curPic; /* "snapshot" current picture value so we're not following a moving target */
        do {
            /* spin half vsyncs until current picture removed from display - ie a new pic has been displayed */
            BKNI_Sleep(8);
        } while ( imageInput->feeder[i].displayed && cnt-- );
    }
    return NEXUS_SUCCESS;
}

NEXUS_VideoInputHandle
NEXUS_VideoImageInput_GetConnector(NEXUS_VideoImageInputHandle imageInput)
{
    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);
    return &imageInput->input;
}

#if NEXUS_HAS_VIDEO_DECODER
static void
NEXUS_VideoImageInput_P_DisconnectStcChannel(NEXUS_VideoImageInputHandle imageInput)
{
    NEXUS_Module_Lock(pVideo->modules.transport);
    if (imageInput->stc.connector)
    {
        NEXUS_StcChannel_DisconnectDecoder_priv(imageInput->stc.connector);
        imageInput->stc.connector = NULL;
    }
    NEXUS_Module_Unlock(pVideo->modules.transport);
}

static NEXUS_Error
NEXUS_VideoImageInput_P_ConnectStcChannel(NEXUS_VideoImageInputHandle imageInput, unsigned * pStcIndex)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelDecoderConnectionSettings stcChannelConnectionSettings;

    NEXUS_Module_Lock(pVideo->modules.transport);

    NEXUS_StcChannel_GetDefaultDecoderConnectionSettings_priv(&stcChannelConnectionSettings);
    stcChannelConnectionSettings.type = NEXUS_StcChannelDecoderType_eVideo;
    stcChannelConnectionSettings.priority = imageInput->stc.priority;
    stcChannelConnectionSettings.getPts_isr = NEXUS_VideoImageInput_P_GetPts_isr;
    stcChannelConnectionSettings.getCdbLevel_isr = NEXUS_VideoImageInput_P_GetCdbLevel_isr;
    stcChannelConnectionSettings.stcValid_isr = NEXUS_VideoImageInput_P_StcValid_isr;
    stcChannelConnectionSettings.setPcrOffset_isr = NEXUS_VideoImageInput_P_SetPcrOffset_isr;
    stcChannelConnectionSettings.getPcrOffset_isr = NEXUS_VideoImageInput_P_GetPcrOffset_isr;
    stcChannelConnectionSettings.pCallbackContext = imageInput;
    imageInput->stc.connector = NEXUS_StcChannel_ConnectDecoder_priv(imageInput->settings.stcChannel, &stcChannelConnectionSettings);
    if (!imageInput->stc.connector) { NEXUS_Module_Unlock(pVideo->modules.transport); rc = BERR_TRACE(rc); goto err; }

    NEXUS_StcChannel_GetIndex_priv(imageInput->settings.stcChannel, pStcIndex);

    NEXUS_Module_Unlock(pVideo->modules.transport);

err:
    return rc;
}
#endif

static BERR_Code
NEXUS_VideoImageInput_P_Connect(NEXUS_VideoInput_P_Link *link)
{
    BERR_Code rc;
    NEXUS_VideoImageInputHandle imageInput;

    BDBG_ASSERT(link->input && link->input->source);
    imageInput = link->input->source;
    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);

    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eImageInput, true);

    imageInput->pLink = link;
    imageInput->source = link->sourceVdc;
    if ( imageInput->settings.lowDelayMode )
    {
        rc = BVDC_Source_InstallPictureCallback( imageInput->source , NEXUS_Display_P_ImageInputNext_isr, imageInput , 0);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_source_cfg;}
    }
#if NEXUS_HAS_VIDEO_DECODER
    else
    {
        unsigned stcIndex = 0;
        BINT_Id top, bottom, frame;
        rc = BVDC_Source_GetInterruptName(link->sourceVdc, BAVC_Polarity_eTopField, &top);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_intr_name;}
        rc = BVDC_Source_GetInterruptName(link->sourceVdc, BAVC_Polarity_eBotField, &bottom);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_intr_name;}
        rc = BVDC_Source_GetInterruptName(link->sourceVdc, BAVC_Polarity_eFrame, &frame);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc); goto err_intr_name;}

        NEXUS_Module_Lock(pVideo->modules.videoDecoder);
        rc = NEXUS_VideoDecoderModule_InstallXdmProvider_priv(imageInput->index, imageInput->xdm.provider, NEXUS_VideoImageInput_P_Picture_isr, imageInput, top, bottom, frame,
            &imageInput->xdm.dih);
        NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
        if (rc!=BERR_SUCCESS) { rc = BERR_TRACE(rc);goto err_source_cfg;}

        if (imageInput->settings.stcChannel)
        {
            rc = NEXUS_VideoImageInput_P_ConnectStcChannel(imageInput, &stcIndex);
            if (rc) { rc = BERR_TRACE(rc); goto err_set_tsm; }
        }

        NEXUS_VideoImageInput_P_ApplyDisplayInformation(imageInput);

        BKNI_EnterCriticalSection();
        BDBG_MSG(("Using STC %u", stcIndex));
        BXDM_PictureProvider_SetSTCIndex_isr(imageInput->xdm.provider, stcIndex);
        BXDM_PictureProvider_Callback_SetEnable_isr(imageInput->xdm.provider, BXDM_PictureProvider_Callback_eFirstPTSPassed, true);
        BXDM_PictureProvider_Callback_SetEnable_isr(imageInput->xdm.provider, BXDM_PictureProvider_Callback_eFirstCodedPTSReady, true);
        BXDM_PictureProvider_Callback_SetEnable_isr(imageInput->xdm.provider, BXDM_PictureProvider_Callback_ePTSError, true);
        rc = BXDM_PictureProvider_StartDecode_isr(imageInput->xdm.provider);
        BKNI_LeaveCriticalSection();
        if (rc) {
            rc = BERR_TRACE(rc);
            goto err_start_decode;
        }
    }
#endif

    imageInput->connected = true;

    return rc;

#if NEXUS_HAS_VIDEO_DECODER
err_start_decode:
    if (imageInput->settings.stcChannel && !imageInput->settings.lowDelayMode)
    {
        NEXUS_VideoImageInput_P_DisconnectStcChannel(imageInput);
    }
err_set_tsm:
    if ( !imageInput->settings.lowDelayMode )
    {
        NEXUS_Module_Lock(pVideo->modules.videoDecoder);
        NEXUS_VideoDecoderModule_RemoveXdmProvider_priv(imageInput->xdm.dih, imageInput->xdm.provider);
        NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
    }
err_intr_name:
#endif
err_source_cfg:
    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eImageInput, false);
    return rc;
}

static void
NEXUS_VideoImageInput_P_Disconnect(NEXUS_VideoInput_P_Link *link)
{
    NEXUS_VideoImageInputHandle imageInput;
    BERR_Code rc;

    BDBG_ASSERT(link->input && link->input->source);
    imageInput = link->input->source;
    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);

    if ( imageInput->settings.lowDelayMode )
    {
        rc = BVDC_Source_InstallPictureCallback(link->sourceVdc, NULL, NULL, 0);
        if (rc) rc = BERR_TRACE(rc);
    }
#if NEXUS_HAS_VIDEO_DECODER
    else
    {
        if (imageInput->settings.stcChannel)
        {
            NEXUS_VideoImageInput_P_DisconnectStcChannel(imageInput);
        }

        BKNI_EnterCriticalSection();
        BXDM_PictureProvider_Callback_SetEnable_isr(imageInput->xdm.provider, BXDM_PictureProvider_Callback_eFirstPTSPassed, false);
        BXDM_PictureProvider_Callback_SetEnable_isr(imageInput->xdm.provider, BXDM_PictureProvider_Callback_eFirstCodedPTSReady, false);
        BXDM_PictureProvider_Callback_SetEnable_isr(imageInput->xdm.provider, BXDM_PictureProvider_Callback_ePTSError, false);
        rc = BXDM_PictureProvider_StopDecode_isr(imageInput->xdm.provider);
        BKNI_LeaveCriticalSection();
        if (rc) {rc = BERR_TRACE(rc);}

        NEXUS_Module_Lock(pVideo->modules.videoDecoder);
        NEXUS_VideoDecoderModule_RemoveXdmProvider_priv(imageInput->xdm.dih, imageInput->xdm.provider);
        NEXUS_Module_Unlock(pVideo->modules.videoDecoder);
    }
#endif

    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eImageInput, false);

    imageInput->connected = false;
}


NEXUS_VideoInput_P_Link *
NEXUS_VideoImageInput_P_OpenInput(NEXUS_VideoInput input)
{
    NEXUS_VideoInput_P_Iface iface;
    NEXUS_VideoInput_P_LinkData data;
    NEXUS_VideoImageInputHandle imageInput = input->source;

    BDBG_ASSERT(input->source);
    BDBG_ASSERT(input->type == NEXUS_VideoInputType_eImage);
    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);

    iface.connect = NEXUS_VideoImageInput_P_Connect;
    iface.disconnect = NEXUS_VideoImageInput_P_Disconnect;
    NEXUS_VideoInput_P_LinkData_Init(&data, imageInput->feeder[imageInput->curPic].sourceId);
    data.heap = imageInput->settings.heap;
    data.gfxSource = imageInput->settings.lowDelayMode;
    return NEXUS_VideoInput_P_CreateLink(input, &data, &iface);
}

void
NEXUS_VideoImageInput_GetSettings(NEXUS_VideoImageInputHandle imageInput, NEXUS_VideoImageInputSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);
    BDBG_ASSERT(pSettings);
    *pSettings = imageInput->settings;
    return;
}

NEXUS_Error
NEXUS_VideoImageInput_SetSettings(NEXUS_VideoImageInputHandle imageInput, const NEXUS_VideoImageInputSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);
    BDBG_ASSERT(pSettings);

    if ( imageInput->connected )
    {
        /* These params can not be changed while connected */
        if ( pSettings->stcChannel != imageInput->settings.stcChannel ||
             pSettings->lowDelayMode != imageInput->settings.lowDelayMode )
        {
            BDBG_ERR(("STC Channel and lowDelayModes can not be changed while connected to a window"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
    }

    if(pSettings->qScale>127) { return BERR_TRACE(BERR_INVALID_PARAMETER); }

    NEXUS_Module_TaskCallback_Set(imageInput->imageCallback, &pSettings->imageCallback);
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_Module_IsrCallback_Set(imageInput->xdm.frameReturned, &pSettings->imageCallback);
    NEXUS_Module_IsrCallback_Set(imageInput->xdm.ptsError, &pSettings->ptsError);
    NEXUS_Module_IsrCallback_Set(imageInput->xdm.firstPtsPassed, &pSettings->firstPtsPassed);
    {
    bool tsmEnabled=false;
    BERR_Code errCode;
    BDBG_MSG(("lowDelay %u stcChannel %p tsmEnabled %u", pSettings->lowDelayMode, (void *)pSettings->stcChannel, pSettings->tsmEnabled));
    if ( pSettings->lowDelayMode == false && NULL != pSettings->stcChannel && pSettings->tsmEnabled )
    {
        tsmEnabled = true;
    }

    BKNI_EnterCriticalSection();
    errCode = BXDM_PictureProvider_SetDisplayMode_isr(imageInput->xdm.provider, tsmEnabled ? BXDM_PictureProvider_DisplayMode_eTSM : BXDM_PictureProvider_DisplayMode_eVirtualTSM);
    BKNI_LeaveCriticalSection();
    if ( errCode ) { (void)BERR_TRACE(errCode); }
    }
#endif

    imageInput->settings = *pSettings;
    return NEXUS_SUCCESS;
}

void NEXUS_VideoImageInput_P_UpdateDisplayInformation(NEXUS_VideoImageInputHandle imageInput, const NEXUS_DisplaySettings *pSettings)
{
    BFMT_VideoFmt fmt;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);
    rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(pSettings->format, &fmt);
    if (!rc) {
        BKNI_EnterCriticalSection();
        imageInput->displayFormat = fmt;
        BKNI_LeaveCriticalSection();
    }

#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoImageInput_P_ApplyDisplayInformation(imageInput);
#endif
}

NEXUS_Error NEXUS_VideoImageInput_GetStatus( NEXUS_VideoImageInputHandle imageInput, NEXUS_VideoImageInputStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(imageInput, NEXUS_VideoImageInput);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
#if NEXUS_NUM_VIDEO_DECODERS
    if (imageInput->settings.type == NEXUS_VideoImageInput_eMfd) {
        pStatus->memcIndex = g_NEXUS_DisplayModule_State.moduleSettings.videoImageInput[imageInput->index].mfd.memcIndex;
        pStatus->secondaryMemcIndex = g_NEXUS_DisplayModule_State.moduleSettings.videoImageInput[imageInput->index].mfd.secondaryMemcIndex;
    }
    else {
        pStatus->memcIndex = g_NEXUS_DisplayModule_State.moduleSettings.videoImageInput[imageInput->index].vfd.memcIndex;
    }
#endif
    return 0;
}

#if NEXUS_HAS_VIDEO_DECODER
/* Returns the number of unique base pictures available in the decoder's picture delivery queue */
static BERR_Code NEXUS_VideoImageInput_P_GetPictureCount_isr(void *pvHandle, uint32_t *puiPictureCount)
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pvHandle;
    NEXUS_VideoImageInputPicture *pPicture;
    uint32_t count=0;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    BDBG_ASSERT(NULL != puiPictureCount);
    BKNI_ASSERT_ISR_CONTEXT();

    for ( pPicture=BLST_SQ_FIRST(&imageInput->xdm.readyQueue);
          NULL != pPicture;
          pPicture = BLST_SQ_NEXT(pPicture, node) )
    {
        count++;
    }
    #if 0
    BDBG_MSG(("Count %u", count));
    #endif
    *puiPictureCount = count;
    return BERR_SUCCESS;
}

/* Returns a pointer to the specified unified picture on the decoder's picture delivery queue,
* but DOES NOT remove it from the queue */
static BERR_Code NEXUS_VideoImageInput_P_PeekAtPicture_isr(void *pvHandle, uint32_t uiIndex, const BXDM_Picture **pUnifiedPicture)
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pvHandle;
    NEXUS_VideoImageInputPicture *pPicture;
    uint32_t count=0;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    BDBG_ASSERT(NULL != pUnifiedPicture);
    BKNI_ASSERT_ISR_CONTEXT();

    for ( pPicture=BLST_SQ_FIRST(&imageInput->xdm.readyQueue);
          NULL != pPicture;
          pPicture = BLST_SQ_NEXT(pPicture, node) )
    {
        if ( count++ == uiIndex )
        {
            *pUnifiedPicture = &pPicture->xdmPicture;
            #if 0
            BDBG_MSG(("Peek %u (%.03u %08x)", uiIndex, pPicture->xdmPicture.uiSerialNumber, pPicture->xdmPicture.stPTS.uiValue));
            #endif
            return BERR_SUCCESS;
        }
    }

    /* If we get here, the index is out of range */
    *pUnifiedPicture = NULL;
    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

/* Returns a pointer to the next unified picture on the decoder's picture delivery queue
* and DOES remove it from the queue */
static BERR_Code NEXUS_VideoImageInput_P_GetNextPicture_isr(void *pvHandle, const BXDM_Picture **pUnifiedPicture)
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pvHandle;
    NEXUS_VideoImageInputPicture *pPicture;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    BDBG_ASSERT(NULL != pUnifiedPicture);
    BKNI_ASSERT_ISR_CONTEXT();

    pPicture = BLST_SQ_FIRST(&imageInput->xdm.readyQueue);
    if ( NULL == pPicture )
    {
        *pUnifiedPicture = NULL;
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else
    {
        *pUnifiedPicture = &pPicture->xdmPicture;
        BLST_SQ_REMOVE_HEAD(&imageInput->xdm.readyQueue, node);
        BLST_SQ_INSERT_TAIL(&imageInput->xdm.displayQueue, pPicture, node);
        BDBG_MSG(("Deliver %.03u %08x %s", pPicture->xdmPicture.uiSerialNumber, pPicture->xdmPicture.stPTS.uiValue, pPicture->xdmPicture.stPictureType.bLastPicture?"EOS":""));
        return BERR_SUCCESS;
    }
}

/* Adds unified picture to the decoder's release queue */
static BERR_Code NEXUS_VideoImageInput_P_ReleasePicture_isr(void *pvHandle, const BXDM_Picture *pUnifiedPicture, const BXDM_Decoder_ReleasePictureInfo *pReleasePictureInfo)
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pvHandle;
    NEXUS_VideoImageInputPicture *pPicture = (NEXUS_VideoImageInputPicture *)pUnifiedPicture;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    BDBG_ASSERT(NULL != pUnifiedPicture);
    BSTD_UNUSED(pReleasePictureInfo);
    BKNI_ASSERT_ISR_CONTEXT();

    BLST_SQ_REMOVE(&imageInput->xdm.displayQueue, pPicture, NEXUS_VideoImageInputPicture, node);
    if ( pPicture->xdmPicture.stBufferInfo.bValid )
    {
        /* Return picture to application */
        BDBG_MSG(("Release %.03u %08x", pPicture->xdmPicture.uiSerialNumber, pPicture->xdmPicture.stPTS.uiValue));
        BLST_SQ_INSERT_TAIL(&imageInput->xdm.returnQueue, pPicture, node);
    }
    else
    {
        /* Dummy EOS picture just goes directly into the free list */
        NEXUS_IsrCallback_Fire_isr(imageInput->xdm.eosDone);
        BLST_SQ_INSERT_TAIL(&imageInput->xdm.freeList, pPicture, node);
        imageInput->xdm.eos = false;
    }
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_VideoImageInput_P_DisplayInterruptEvent_isr(void *pvHandle)
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pvHandle;
    NEXUS_VideoImageInputPicture *pPicture;
    bool fireCallback=false;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    BKNI_ASSERT_ISR_CONTEXT();

    /* Frames must be delayed by one display event before returning to the application to avoid tearing */
    while ( NULL != (pPicture=BLST_SQ_FIRST(&imageInput->xdm.returnQueue)) )
    {
        BLST_SQ_REMOVE_HEAD(&imageInput->xdm.returnQueue, node);
        BLST_SQ_INSERT_TAIL(&imageInput->xdm.recycleQueue, pPicture, node);
        fireCallback = true;
    }
    if ( fireCallback )
    {
        NEXUS_IsrCallback_Fire_isr(imageInput->xdm.frameReturned);
    }

    return BERR_SUCCESS;
}


static NEXUS_Error NEXUS_VideoImageInput_P_PushSurface_xdm(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle image,
    const NEXUS_VideoImageInputSurfaceSettings *pSettings
    )
{
    NEXUS_VideoImageInputPicture *pPicture;
    BXDM_Picture xdmPicture;
    NEXUS_VideoImageInputSurfaceSettings defaults;
    BVDC_Source_Capabilities vdcCapabilities;
    NEXUS_VideoInput_P_Link *link;
    BPXL_Format magnumPixelFormat;
    NEXUS_StripedSurfaceCreateSettings stripeSettings;
    BERR_Code rc;

    if ( NULL == pSettings )
    {
        NEXUS_VideoImageInput_GetDefaultSurfaceSettings(&defaults);
        pSettings = &defaults;
    }

    /* Handle EOS */
    if ( pSettings->endOfStream )
    {
        if ( image || pSettings->stripedSurface )
        {
            BDBG_ERR(("You must pass a NULL image to generate end of stream"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        BDBG_MSG(("EOS request"));
        return NEXUS_VideoImageInput_P_SendEos(imageInput);
    }
    /* Handle Flush */
    if ( NULL == image && NULL == pSettings->stripedSurface )
    {
        /* Flush the surface queue */
        BKNI_EnterCriticalSection();
        while ( NULL != (pPicture = BLST_SQ_FIRST(&imageInput->xdm.readyQueue)) )
        {
            BLST_SQ_REMOVE_HEAD(&imageInput->xdm.readyQueue, node);
            if ( pPicture->xdmPicture.stBufferInfo.bValid )
            {
                BLST_SQ_INSERT_TAIL(&imageInput->xdm.recycleQueue, pPicture, node);
                NEXUS_IsrCallback_Fire_isr(imageInput->xdm.frameReturned);
            }
            else
            {
                /* Don't flush EOS from the queue */
                break;
            }
        }
        BKNI_LeaveCriticalSection();
        return NEXUS_SUCCESS;
    }

    /* Rest is queuing surfaces */
    link = NEXUS_VideoInput_P_Get( NEXUS_VideoImageInput_GetConnector(imageInput) );
    if (!link) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    BDBG_OBJECT_ASSERT(link, NEXUS_VideoInput_P_Link);
    BDBG_ASSERT(link->sourceVdc);
    if (!link->ref_cnt) {
        BDBG_ERR(("image input not connected to window"));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    /* Prepare XDM picture structure at task time */
    BKNI_Memset(&xdmPicture, 0, sizeof(BXDM_Picture));

    if(image) { /* regular surface */
        NEXUS_SurfaceStatus surfaceStatus;
        NEXUS_SurfaceMemoryProperties surfaceMemory;

        NEXUS_Surface_GetStatus(image, &surfaceStatus);
        NEXUS_Surface_GetMemoryProperties(image, &surfaceMemory);

        rc = NEXUS_P_PixelFormat_ToMagnum_isrsafe( surfaceStatus.pixelFormat, &magnumPixelFormat);
        if (rc!=BERR_SUCCESS) { return BERR_TRACE(rc); }

        BVDC_Source_GetCapabilities(link->sourceVdc, &vdcCapabilities) ;
        if ( !vdcCapabilities.pfIsPxlfmtSupported(magnumPixelFormat) ) {
            BDBG_WRN(("pixelFormat %d of image=%p is NOT supported", surfaceStatus.pixelFormat, (void *)image ));
            return BERR_TRACE(NEXUS_INVALID_PARAMETER); /* unsupported  pixelFormat */
        }
        xdmPicture.stBufferInfo.hLuminanceFrameBufferBlock = NEXUS_MemoryBlock_GetBlock_priv(surfaceMemory.pixelMemory);
        xdmPicture.stBufferInfo.ulLuminanceFrameBufferBlockOffset = surfaceMemory.pixelMemoryOffset;
        xdmPicture.stBufferInfo.stPixelFormat.ePixelFormat = magnumPixelFormat;
        xdmPicture.stBufferInfo.stPixelFormat.bValid = true;
        xdmPicture.stBufferInfo.uiRowStride = surfaceStatus.pitch;
        xdmPicture.stBufferInfo.stSource.bValid = true;
        xdmPicture.stBufferInfo.stSource.uiHeight = surfaceStatus.height;
        xdmPicture.stBufferInfo.stSource.uiWidth = surfaceStatus.width;
    } else { /* striped surface */
        BDBG_ASSERT(pSettings->stripedSurface);
        NEXUS_StripedSurface_GetCreateSettings(pSettings->stripedSurface, &stripeSettings);
        rc = NEXUS_P_PixelFormat_ToMagnum_isrsafe( stripeSettings.lumaPixelFormat, &magnumPixelFormat);
        xdmPicture.stBufferInfo.hLuminanceFrameBufferBlock = NEXUS_MemoryBlock_GetBlock_priv(stripeSettings.lumaBuffer);
        xdmPicture.stBufferInfo.ulLuminanceFrameBufferBlockOffset = stripeSettings.lumaBufferOffset;
        xdmPicture.stBufferInfo.hChrominanceFrameBufferBlock = NEXUS_MemoryBlock_GetBlock_priv(stripeSettings.chromaBuffer);
        xdmPicture.stBufferInfo.ulChrominanceFrameBufferBlockOffset = stripeSettings.chromaBufferOffset;
        xdmPicture.stBufferInfo.uiLumaStripeHeight   = stripeSettings.lumaStripedHeight;
        xdmPicture.stBufferInfo.uiChromaStripeHeight = stripeSettings.chromaStripedHeight;
        xdmPicture.stBufferInfo.eStripeWidth = NEXUS_P_StripeWidth_ToMagnum_isrsafe(stripeSettings.stripedWidth);
    }

    xdmPicture.stBufferInfo.eBufferFormat = BXDM_Picture_BufferFormat_eSingle;
    xdmPicture.stBufferInfo.eBufferHandlingMode = BXDM_Picture_BufferHandlingMode_eNormal;
    xdmPicture.stBufferInfo.eLumaBufferType = xdmPicture.stBufferInfo.eChromaBufferType =
        (BPXL_IS_YCbCr422_10BIT_FORMAT(magnumPixelFormat) || BPXL_IS_YCbCr420_10BIT_FORMAT(magnumPixelFormat))? BXDM_Picture_BufferType_e10Bit : BXDM_Picture_BufferType_e8Bit;
    xdmPicture.stBufferInfo.stDisplay = xdmPicture.stBufferInfo.stSource;
    /* Omit display size and striping info for now */
    xdmPicture.stBufferInfo.stChromaLocation[BAVC_Polarity_eFrame].bValid = true;
    xdmPicture.stBufferInfo.stChromaLocation[BAVC_Polarity_eFrame].eMpegType = BAVC_MpegType_eMpeg2;
    xdmPicture.stBufferInfo.stChromaLocation[BAVC_Polarity_eTopField].bValid = true;
    xdmPicture.stBufferInfo.stChromaLocation[BAVC_Polarity_eTopField].eMpegType= BAVC_MpegType_eMpeg2;
    xdmPicture.stBufferInfo.stChromaLocation[BAVC_Polarity_eBotField].bValid = true;
    xdmPicture.stBufferInfo.stChromaLocation[BAVC_Polarity_eBotField].eMpegType= BAVC_MpegType_eMpeg2;
    xdmPicture.stBufferInfo.eYCbCrType = BPXL_IS_YCbCr420_FORMAT(magnumPixelFormat) ? BAVC_YCbCrType_e4_2_0 : BAVC_YCbCrType_e4_2_2;
    switch (pSettings->pullDown) {
    case NEXUS_PicturePullDown_eFrame:
        xdmPicture.stBufferInfo.ePulldown = BXDM_Picture_PullDown_eFrameX1;
        break;
    default:
        xdmPicture.stBufferInfo.ePulldown = pSettings->pullDown;
        break;
    }
    xdmPicture.stBufferInfo.eSourceFormat = pSettings->sourceFormat;
    xdmPicture.stBufferInfo.bValid = true;
    xdmPicture.uiSerialNumber = imageInput->xdm.serialNum++;
    BDBG_MSG(("Push %u pts %08x %s", xdmPicture.uiSerialNumber,pSettings->pts,pSettings->ptsValid?"valid":"invalid"));
    switch ( pSettings->pictureCoding )
    {
    default:
        xdmPicture.stPictureType.eCoding = BXDM_Picture_Coding_eUnknown;
        break;
    case NEXUS_PictureCoding_eI:
        xdmPicture.stPictureType.eCoding = BXDM_Picture_Coding_eI;
        xdmPicture.stPictureType.bReference = true;
        break;
    case NEXUS_PictureCoding_eP:
        xdmPicture.stPictureType.eCoding = BXDM_Picture_Coding_eP;
        xdmPicture.stPictureType.bReference = true;
        break;
    case NEXUS_PictureCoding_eB:
        xdmPicture.stPictureType.eCoding = BXDM_Picture_Coding_eB;
        break;
    }
    xdmPicture.stPictureType.eSequence = pSettings->sequence;
    xdmPicture.stPTS.bValid = pSettings->ptsValid;
    xdmPicture.stPTS.uiValue = pSettings->pts;
    if ( pSettings->frameRate == NEXUS_VideoFrameRate_eUnknown )
    {
        xdmPicture.stFrameRate.eType = BXDM_Picture_FrameRateType_eUnknown;
    }
    else
    {
        unsigned frameRate = NEXUS_P_RefreshRate_FromFrameRate_isrsafe(pSettings->frameRate);
        xdmPicture.stFrameRate.eType = BXDM_Picture_FrameRateType_eFixed;
        xdmPicture.stFrameRate.stRate.uiNumerator = frameRate;
        xdmPicture.stFrameRate.stRate.uiDenominator = 1000;
    }
    xdmPicture.stFrameRate.bValid = true;

    xdmPicture.stAspectRatio.eAspectRatio = NEXUS_P_AspectRatio_ToMagnum_isrsafe(pSettings->aspectRatio);
    xdmPicture.stAspectRatio.uiSampleAspectRatioX = pSettings->sampleAspectRatio.x;
    xdmPicture.stAspectRatio.uiSampleAspectRatioY = pSettings->sampleAspectRatio.y;
    xdmPicture.stAspectRatio.bValid = true;

    /* Add frame to ready queue in isr context */
    BKNI_EnterCriticalSection();
    {
        pPicture = BLST_SQ_FIRST(&imageInput->xdm.freeList);
        if ( pPicture )
        {
            BLST_SQ_REMOVE_HEAD(&imageInput->xdm.freeList, node);
            BLST_SQ_INSERT_TAIL(&imageInput->xdm.readyQueue, pPicture, node);
            pPicture->hSurface = image? image : (NEXUS_SurfaceHandle)pSettings->stripedSurface;
            pPicture->xdmPicture = xdmPicture;
        }
    }
    BKNI_LeaveCriticalSection();

    if ( pPicture )
    {
        return NEXUS_SUCCESS;
    }
    else
    {
        return BERR_TRACE(NEXUS_IMAGEINPUT_QUEUE_FULL);
    }
}

static NEXUS_Error NEXUS_VideoImageInput_P_RecycleSurface_xdm(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle *recycled,
    size_t num_entries,
    size_t *num_returned
    )
{
    size_t num=0;
    NEXUS_VideoImageInputPicture *pPicture;

    BKNI_Memset(recycled, 0, num_entries * sizeof(*recycled));

    BKNI_EnterCriticalSection();
    {
        while ( NULL != (pPicture = BLST_SQ_FIRST(&imageInput->xdm.recycleQueue)) &&
                num < num_entries )
        {
            /* move picture from return queue to free list and then return surface to app */
            BLST_SQ_REMOVE_HEAD(&imageInput->xdm.recycleQueue, node);
            BLST_SQ_INSERT_TAIL(&imageInput->xdm.freeList, pPicture, node);
            BDBG_MSG(("Recycle %u", pPicture->xdmPicture.uiSerialNumber));
            recycled[num++] = pPicture->hSurface;
            pPicture->hSurface = NULL;
        }
    }
    BKNI_LeaveCriticalSection();

    *num_returned = num;
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_VideoImageInput_P_CheckSurfaceCompletion_xdm(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle image
    )
{
    NEXUS_VideoImageInputPicture *pPicture;

    /* Completed surfaces will be found in the return queue */
    BKNI_EnterCriticalSection();
    {
        for ( pPicture = BLST_SQ_FIRST(&imageInput->xdm.recycleQueue);
              NULL != pPicture;
              pPicture = BLST_SQ_NEXT(pPicture,node) )
        {
            if ( pPicture->hSurface == image )
            {
                break;
            }
        }
    }
    BKNI_LeaveCriticalSection();

    return (NULL != pPicture) ? NEXUS_SUCCESS : NEXUS_IMAGEINPUT_BUSY;  /* BERR_TRACE intentionally omitted */
}

static NEXUS_Error NEXUS_VideoImageInput_P_SetNextSurface_xdm(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle image
    )
{
    BSTD_UNUSED(imageInput);
    BSTD_UNUSED(image);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

static NEXUS_Error NEXUS_VideoImageInput_P_SetSurface_xdm(
    NEXUS_VideoImageInputHandle imageInput,
    NEXUS_SurfaceHandle image
    )
{
    BSTD_UNUSED(imageInput);
    BSTD_UNUSED(image);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

static NEXUS_Error NEXUS_VideoImageInput_P_SendEos(
    NEXUS_VideoImageInputHandle imageInput
    )
{
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    if ( imageInput->settings.lowDelayMode )
    {
        BDBG_ERR(("EOS is not available if NEXUS_VideoImageInputSettings.lowDelayMode = false"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if ( imageInput->xdm.eos )
    {
        BDBG_ERR(("EOS has already been requested"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if ( imageInput->connected )
    {
        NEXUS_VideoImageInputPicture *pPicture;
        BKNI_EnterCriticalSection();
        {
            pPicture = BLST_SQ_FIRST(&imageInput->xdm.freeList);
            if ( pPicture )
            {
                BLST_SQ_REMOVE_HEAD(&imageInput->xdm.freeList, node);
                BKNI_Memset(&pPicture->xdmPicture, 0, sizeof(BXDM_Picture));
                pPicture->xdmPicture.stPictureType.bLastPicture = true;
                pPicture->hSurface = NULL;
                BLST_SQ_INSERT_TAIL(&imageInput->xdm.readyQueue, pPicture, node);
                imageInput->xdm.eos = true;
                BDBG_MSG(("EOS Sent"));
            }
            else
            {
                /* No picture buffer available for EOS */
                BDBG_MSG(("Queue full, no EOS"));
                errCode = NEXUS_IMAGEINPUT_QUEUE_FULL;
            }
        }
        BKNI_LeaveCriticalSection();
    }
    else
    {
        BDBG_ERR(("ImageInput is not connected, can't raise EOS"));
        errCode = BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    return errCode;
}

static void NEXUS_VideoImageInput_P_FirstPtsReady_isr(void *pContext, int32_t unused, const BXDM_PictureProvider_PTSInfo *pPTSInfo )
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pContext;
    BAVC_PTSInfo pts;
    BERR_Code rc;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    BSTD_UNUSED(unused);

    pts.ui32CurrentPTS = pPTSInfo->ui32RunningPTS;
    pts.ePTSType = BAVC_PTSType_eCoded;
    BDBG_MSG(("imageInput:%d Request STC %#x", (unsigned)imageInput->index, pts.ui32CurrentPTS));

    if (imageInput->settings.stcChannel && imageInput->stc.connector) {
       rc = NEXUS_StcChannel_RequestStc_isr(imageInput->stc.connector, &pts);
       if (rc) {rc=BERR_TRACE(rc);} /* keep going */
    }
}

static void NEXUS_VideoImageInput_P_FirstPtsPassed_isr(
         void *pPrivateContext,
         int32_t unused,
         const BXDM_PictureProvider_PTSInfo *pstPTSInfo)
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pPrivateContext;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    BSTD_UNUSED(unused);
    BSTD_UNUSED(pstPTSInfo);

    BDBG_MSG(("First PTS passed"));

    NEXUS_IsrCallback_Fire_isr(imageInput->xdm.firstPtsPassed);
}

static void NEXUS_VideoImageInput_P_PtsError_isr(void *pContext, int32_t unused, const BXDM_PictureProvider_PTSInfo *pPTSInfo )
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pContext;
    BAVC_PTSInfo pts;
    BERR_Code rc;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    BSTD_UNUSED(unused);

    pts.ui32CurrentPTS = pPTSInfo->ui32RunningPTS;
    pts.ePTSType = BAVC_PTSType_eCoded;
    BDBG_MSG(("imageInput:%d PTS error %#x", (unsigned)imageInput->index, pts.ui32CurrentPTS));

    if (imageInput->settings.stcChannel && imageInput->stc.connector)
    {
        rc = NEXUS_StcChannel_PtsError_isr(imageInput->stc.connector, &pts);
        if (rc) {rc=BERR_TRACE(rc);} /* keep going */
    }

    NEXUS_IsrCallback_Fire_isr(imageInput->xdm.ptsError);
}

static BERR_Code NEXUS_VideoImageInput_P_Picture_isr(void *pContext, int32_t unused, BAVC_MFD_Picture *pFieldData)
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pContext;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    BSTD_UNUSED(unused);

    NEXUS_VideoInput_P_DecoderDataReady_isr(imageInput->pLink, pFieldData);

    return BERR_SUCCESS;
}

static BERR_Code NEXUS_VideoImageInput_P_GetPts_isr(void *pContext, BAVC_PTSInfo *pPTSInfo)
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pContext;
    BXDM_PictureProvider_PTSInfo ptsInfo;
    BERR_Code rc;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    rc = BXDM_PictureProvider_GetCurrentPTSInfo_isr(imageInput->xdm.provider, &ptsInfo);

    /* map data structures */
    pPTSInfo->ui32CurrentPTS = ptsInfo.ui32RunningPTS;
    pPTSInfo->ePTSType = ptsInfo.ePTSType;
    return rc;
}

static BERR_Code NEXUS_VideoImageInput_P_GetCdbLevel_isr(void *pContext, unsigned *pCdbLevel)
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pContext;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    *pCdbLevel = 128*1024;  /* TODO: What should this really do? */
    return 0;
}

static BERR_Code NEXUS_VideoImageInput_P_StcValid_isr(void *pContext)
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pContext;
    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    BDBG_MSG(("STC Valid"));
    return BXDM_PictureProvider_SetSTCValid_isr(imageInput->xdm.provider, true);
}

static BERR_Code NEXUS_VideoImageInput_P_SetPcrOffset_isr(void *pContext, uint32_t pcrOffset)
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pContext;
    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    BDBG_MSG(("Set PCR Offset %u", pcrOffset));
    return BXDM_PictureProvider_SetSoftwarePCROffset_isr(imageInput->xdm.provider, pcrOffset);
}

static BERR_Code NEXUS_VideoImageInput_P_GetPcrOffset_isr(void *pContext, uint32_t *pPcrOffset)
{
    NEXUS_VideoImageInputHandle imageInput = (NEXUS_VideoImageInputHandle)pContext;
    NEXUS_OBJECT_ASSERT(NEXUS_VideoImageInput, imageInput);
    return BXDM_PictureProvider_GetSoftwarePCROffset_isr(imageInput->xdm.provider, pPcrOffset);
}

static void NEXUS_VideoImageInput_P_ApplyDisplayInformation(NEXUS_VideoImageInputHandle imageInput)
{
    BERR_Code rc;
    bool nrt=false;
    unsigned n, stgIndex=0;
    NEXUS_VideoWindowHandle window;
    NEXUS_DisplayHandle display;
    BXDM_PictureProvider_MonitorRefreshRate refreshRate;
    unsigned dispRefreshRate;

    rc = NEXUS_Display_P_GetWindows_priv(NEXUS_VideoImageInput_GetConnector(imageInput), &window, 1, &n);
    if (!rc && n == 1) {
        display = window->display;
        BDBG_ASSERT(NULL != display);
        nrt = display->stgSettings.nonRealTime;
        stgIndex = display->stgIndex;
    } else {
        /* No display connection */
        return;
    }

    dispRefreshRate = display->status.refreshRate / 10;

    BDBG_MSG(("Applying display info to XDM (%.02d.%.02d Hz) fmt %u %sreal-time", dispRefreshRate/100, dispRefreshRate%100, (unsigned)display->cfg.format,nrt?"non-":""));

    refreshRate = NEXUS_VideoDecoder_P_GetXdmMonitorRefreshRate_isrsafe(dispRefreshRate);
    BKNI_EnterCriticalSection();
    rc = BXDM_PictureProvider_SetMonitorRefreshRate_isr(imageInput->xdm.provider, refreshRate);
    BKNI_LeaveCriticalSection();
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);}

    if(imageInput->settings.stcChannel) {
        NEXUS_StcChannelNonRealtimeSettings stcNonRealtimeSettings;
        NEXUS_Module_Lock(pVideo->modules.transport);
        {
            NEXUS_StcChannel_GetDefaultNonRealtimeSettings_priv( &stcNonRealtimeSettings);
            stcNonRealtimeSettings.externalTrigger = 0;
            if(nrt) {
                stcNonRealtimeSettings.triggerMode = NEXUS_StcChannelTriggerMode_eExternalTrig;
                stcNonRealtimeSettings.externalTrigger = stgIndex;
                if(dispRefreshRate) {
                    bool b1001Slip;
                    unsigned vsyncRate;
                    NEXUS_VideoDecoder_P_GetVsyncRate_isrsafe(dispRefreshRate, &vsyncRate, &b1001Slip);
                    stcNonRealtimeSettings.stcIncrement = ((uint64_t)27000000 * (b1001Slip?1001:1000) / 10) / vsyncRate; /* convert from 1/100Hz to 27MHz ticks */
                } else {
                    stcNonRealtimeSettings.stcIncrement = 0;
                }
            } else {
                stcNonRealtimeSettings.triggerMode = NEXUS_StcChannelTriggerMode_eTimebase;
            }
            /* reset NRT STC only when starting */
            rc = NEXUS_StcChannel_SetNonRealtimeConfig_priv(imageInput->settings.stcChannel, &stcNonRealtimeSettings, !imageInput->connected);
        }
        NEXUS_Module_Unlock(pVideo->modules.transport);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); /* keep going */}
    }
}
#endif
