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
#include "nexus_picture_decoder_module.h"
#include "nexus_picture_decoder_init.h"
#include "nexus_power_management.h"
#include "priv/nexus_core.h"
#include "nexus_client_resources.h"
#if NEXUS_HAS_SECURITY
#include "priv/nexus_security_regver_priv.h"
#endif

#include "priv/nexus_core_img.h"
#include "priv/nexus_core_img_id.h"

BDBG_MODULE(nexus_picture_decoder);
#define BDBG_MSG_TRACE(x)  /* BDBG_MSG(x) */


#if NEXUS_HAS_SECURITY
#define LOCK_SECURITY() NEXUS_Module_Lock(g_NEXUS_PictureDecoder_P_ModuleState.settings.security);
#define UNLOCK_SECURITY() NEXUS_Module_Unlock(g_NEXUS_PictureDecoder_P_ModuleState.settings.security);
#endif

typedef enum {
  ebufFree = 0,
  ebufAllocd,
  ebufFilled
} sidBuf_eState;

struct NEXUS_PictureDecoder {
    NEXUS_OBJECT(NEXUS_PictureDecoder);
    BLST_S_ENTRY(NEXUS_PictureDecoder) link;
    BSID_ChannelHandle  sid;
    int index;
    void *fifo; /* cached pointer */
    unsigned wr_offset;
    enum {
        NEXUS_PictureDecoder_P_eStopped,
        NEXUS_PictureDecoder_P_eWaitingData,
        NEXUS_PictureDecoder_P_eStreamInfo,       /* BSID_GetStreamInfo called */
        NEXUS_PictureDecoder_P_eStreamInfoDone ,  /* BSID_GetStreamInfo completed */
        NEXUS_PictureDecoder_P_eDecoding,
        NEXUS_PictureDecoder_P_eSegmentDone,
        NEXUS_PictureDecoder_P_eFailed,
        NEXUS_PictureDecoder_P_eMoreInfoData,
        NEXUS_PictureDecoder_P_eMoreImageData
    } state;
    NEXUS_PictureDecoderOpenSettings openSettings;
    NEXUS_PictureDecoderStartSettings startSettings;
    BKNI_EventHandle event;
    NEXUS_EventCallbackHandle eventHandler;
    NEXUS_TaskCallbackHandle pictureParsedCallback;
    NEXUS_TaskCallbackHandle segmentDecodedCallback;

    struct {
        BSID_LinDataMap dataMap;
        BSID_ImageFormat imageFormat;
        struct {
            BSID_NotificationEvent eventType;
        } event;
        NEXUS_PictureDecoderStatus status;
        unsigned nextRow;
        struct {
            NEXUS_Rect rect;
            NEXUS_SurfaceHandle surface;
        } segment;
    } decodeState; /* SID PI could write to user supplied pointers after function have returned */
    void *memFifo; /* pointer from bmem */
    void *multiScanBuffer;
    uint32_t multiScanBufOffset;
    BMEM_Heap_Handle heap;   /* heap used for SID compressed data buffer, cpu access required */
    BMEM_Heap_Handle multiscanHeap; /* heap used for multiscan buffer, cpu access not required */
    unsigned remainingBytes; /* When using small cdb, shows remaining bytes to decode */
    unsigned bufferDivide;
    int                           chBusy;        /* bufr being processed, set when loaded, zerod after isr received */
                                                 /* Read complete uses it to know when it can start chain again */
    BSID_ImageHeader           imageHeader;
    uint32_t crc;
    uint32_t errorCode;
    uint32_t firmwareVersion;
};

NEXUS_PictureDecoder_P_ModuleState g_NEXUS_PictureDecoder_P_ModuleState;

void
NEXUS_PictureDecoderModule_GetDefaultSettings(NEXUS_PictureDecoderModuleSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->surface = NULL;
    return;
}


static BERR_Code secureFirmwareSid( void* pData, const BSID_BootInfo *pBootInfo )
{
#if NEXUS_HAS_SECURITY
    NEXUS_Error rc;

    BSTD_UNUSED( pData );

    NEXUS_FlushCache( pBootInfo->pStartAddress, pBootInfo->uiSize );

    LOCK_SECURITY();
    rc = NEXUS_Security_RegionVerifyEnable_priv( NEXUS_SecurityRegverRegionID_eSid0,
                                                 NEXUS_AddrToOffset( pBootInfo->pStartAddress ),
                                                 pBootInfo->uiSize );
    UNLOCK_SECURITY();
    if (rc) { return BERR_TRACE(rc); }

    return rc;
#else
    BSTD_UNUSED( pData );
    BSTD_UNUSED( pBootInfo );
    return NEXUS_SUCCESS;
#endif
}


static void
NEXUS_P_PictureDecoder_WatchdogCallback_isr(const BSID_WatchdogCallbackData *pCallbackData)
{
    struct NEXUS_PictureDecoder_P_HWState *hwState = &g_NEXUS_PictureDecoder_P_ModuleState.hwState;
    NEXUS_PictureDecoderHandle decoder = hwState->acquired;

    /* if PictureDecoder never started, hwState->acquired may not be set */
    if (!hwState->acquired) return;

    BSTD_UNUSED(pCallbackData);
    BDBG_OBJECT_ASSERT(decoder, NEXUS_PictureDecoder);
    BDBG_WRN(("watchdog_isr:(dec=%#lx) idx=%d %#lx Fw ver=0x%8x", (unsigned long)decoder, decoder->index, (unsigned long)decoder->sid , hwState->firmwareVersion ));
    BKNI_SetEvent_isr(hwState->watchdogEvent);

    return;
}

static void NEXUS_PictureDecoder_P_WatchdogHandler(void *cntx);

NEXUS_ModuleHandle
NEXUS_PictureDecoderModule_Init(const NEXUS_PictureDecoderModuleSettings *pSettings)
{
    NEXUS_Error rc;
    NEXUS_ModuleSettings moduleSettings;
    struct NEXUS_PictureDecoder_P_HWState *hwState = &g_NEXUS_PictureDecoder_P_ModuleState.hwState;
    BSID_Settings sidSettings;
    BSID_RevisionInfo st_RevisionInfo;
    #if NEXUS_HAS_SECURITY
    NEXUS_SecurityRegionConfiguration regionConfig;
    #endif

    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pSettings->surface);
    g_NEXUS_PictureDecoder_P_ModuleState.settings = *pSettings;
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow; /* picture decoder interface is slow */
    g_NEXUS_PictureDecoder_P_ModuleState.module = NEXUS_Module_Create("picture_decoder", &moduleSettings);
    if(!g_NEXUS_PictureDecoder_P_ModuleState.module) { rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);goto err_module;}

    NEXUS_LockModule();

    BKNI_Memset(hwState, 0, sizeof(*hwState));

    (void)BSID_GetDefaultSettings(&sidSettings);
    sidSettings.ui32_SidInstance = 0;


    /* Image download */
    if (Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_SID,&hwState->img_context,&hwState->img_interface )== NEXUS_SUCCESS)
    {
        sidSettings.pImgInterface = (BIMG_Interface*)&hwState->img_interface;
        sidSettings.pImgContext   = hwState->img_context;
    }
    else
    {
        hwState->img_context= NULL;
    }
    #if NEXUS_HAS_SECURITY
    LOCK_SECURITY();
    NEXUS_Security_RegionGetDefaultConfig_priv( NEXUS_SecurityRegverRegionID_eSid0, &regionConfig ); /* use defaults */
    rc = NEXUS_Security_RegionConfig_priv( NEXUS_SecurityRegverRegionID_eSid0, &regionConfig );
    UNLOCK_SECURITY();
    if( rc != BERR_SUCCESS ) { rc=BERR_TRACE(rc); goto err_sid; }
    #endif


    sidSettings.p_ExternalBootCallback = secureFirmwareSid;
    sidSettings.p_WatchdogFunc_isr = NEXUS_P_PictureDecoder_WatchdogCallback_isr;

    rc = BSID_Open(&hwState->sid, g_pCoreHandles->chp, g_pCoreHandles->reg, NULL, g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].mma, g_pCoreHandles->bint, &sidSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_sid;}

    rc = BKNI_CreateEvent(&hwState->watchdogEvent);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_watchdog_event;}

    hwState->watchdogEventHandler = NEXUS_RegisterEvent(hwState->watchdogEvent, NEXUS_PictureDecoder_P_WatchdogHandler, hwState->sid);
    if(!hwState->watchdogEventHandler) {rc=BERR_TRACE(rc);goto err_watchdog_event_handler;}

    (void)BSID_GetRevisionInfo( hwState->sid , &st_RevisionInfo );
    hwState->firmwareVersion = st_RevisionInfo.ui32_FWVersion;

    BDBG_MSG(("SID PI version=%d FW version=%d" , st_RevisionInfo.ui32_PIVersion , st_RevisionInfo.ui32_FWVersion ));
    BDBG_MSG(("    RleLegacyMaxWidth     =%4d RleHdDvdRleMaxWidth=%4d" , st_RevisionInfo.ui32_RleLegacyMaxWidth  ,st_RevisionInfo.ui32_RleHdDvdRleMaxWidth ));
    BDBG_MSG(("    RleBdMaxWidth         =%4d JpegMaxWidth       =%4d" , st_RevisionInfo.ui32_RleBdMaxWidth      ,st_RevisionInfo.ui32_JpegMaxWidth ));
    BDBG_MSG(("    PngPaletteMaxWidth    =%4d PngRgbaMaxWidth    =%4d" , st_RevisionInfo.ui32_PngPaletteMaxWidth ,st_RevisionInfo.ui32_PngRgbaMaxWidth ));
    BDBG_MSG(("    Png16bppMaxWidth      =%4d GifMaxWidth        =%4d" , st_RevisionInfo.ui32_Png16BppMaxWidth   ,st_RevisionInfo.ui32_GifMaxWidth ));

    NEXUS_UnlockModule();
    return g_NEXUS_PictureDecoder_P_ModuleState.module;

err_watchdog_event_handler:
    BKNI_DestroyEvent(hwState->watchdogEvent);
err_watchdog_event:
err_sid:
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_PictureDecoder_P_ModuleState.module);
    g_NEXUS_PictureDecoder_P_ModuleState.module = NULL;
err_module:
    return NULL;
}

void
NEXUS_PictureDecoderModule_Uninit(void)
{
    struct NEXUS_PictureDecoder_P_HWState *hwState = &g_NEXUS_PictureDecoder_P_ModuleState.hwState;

    BDBG_ASSERT(g_NEXUS_PictureDecoder_P_ModuleState.module);

    NEXUS_LockModule();
    NEXUS_UnregisterEvent(hwState->watchdogEventHandler);
    BKNI_DestroyEvent(hwState->watchdogEvent);

    (void)BSID_Close(hwState->sid);
    NEXUS_UnlockModule();

    #if NEXUS_HAS_SECURITY
    LOCK_SECURITY();
    NEXUS_Security_RegionVerifyDisable_priv( NEXUS_SecurityRegverRegionID_eSid0 );
    UNLOCK_SECURITY();
    #endif

    NEXUS_Module_Destroy(g_NEXUS_PictureDecoder_P_ModuleState.module);
    g_NEXUS_PictureDecoder_P_ModuleState.module = NULL;
    return;
}

void
NEXUS_PictureDecoder_GetDefaultStartSettings(NEXUS_PictureDecoderStartSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->format = NEXUS_PictureFormat_eJpeg;
    NEXUS_CallbackDesc_Init(&pSettings->pictureParsed);
    NEXUS_CallbackDesc_Init(&pSettings->segmentDecoded);
    return;
}

void
NEXUS_PictureDecoder_GetDefaultOpenSettings(NEXUS_PictureDecoderOpenSettings *pSettings)
{
    struct NEXUS_PictureDecoder_P_HWState *hwState = &g_NEXUS_PictureDecoder_P_ModuleState.hwState;
    BSID_RevisionInfo st_RevisionInfo;

    BDBG_ASSERT(pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    (void)BSID_GetRevisionInfo( hwState->sid , &st_RevisionInfo );

    pSettings->jpeg.maxWidth              = st_RevisionInfo.ui32_JpegMaxWidth;
    pSettings->jpeg.chromaVerticalShift   = 0;
    pSettings->jpeg.chromaHorizontalShift = 0;
    pSettings->png.maxWidthRgb            = st_RevisionInfo.ui32_PngRgbaMaxWidth;
    pSettings->png.maxWidthPalette        = st_RevisionInfo.ui32_PngPaletteMaxWidth;
    pSettings->png.maxWidth16Bpp          = st_RevisionInfo.ui32_Png16BppMaxWidth;
    pSettings->gif.maxWidth               = st_RevisionInfo.ui32_GifMaxWidth;
    pSettings->rle.maxWidthDvd            = st_RevisionInfo.ui32_RleLegacyMaxWidth;
    pSettings->rle.maxWidthBluRay         = st_RevisionInfo.ui32_RleBdMaxWidth;
    pSettings->bufferSize                 = 2*1024*1024;
    pSettings->multiScanBufferSize        = 0;
    pSettings->bufferPtr                  = NULL;
    pSettings->multiScanBufferPtr         = NULL;

    return;
}

static void
NEXUS_PictureDecoder_P_Reset(NEXUS_PictureDecoderHandle decoder)
{

    decoder->wr_offset = 0;
    decoder->chBusy = ebufFree;
    decoder->state = NEXUS_PictureDecoder_P_eStopped;
    BKNI_Memset(&decoder->decodeState, 0, sizeof(decoder->decodeState));
    decoder->decodeState.status.state = NEXUS_PictureDecoderState_eIdle;
    decoder->decodeState.status.headerValid = false;
    decoder->decodeState.segment.surface = NULL;
    decoder->startSettings.imageSize = 0;  /* Need to reset before every GetBuffer / Decoder_Start */

    return;
}

static void
NEXUS_PictureDecoder_P_Callback_isr(BSID_ChannelHandle hSid, const BSID_EventInfo *eventInfo, void *pCallbackData)
{
    struct NEXUS_PictureDecoder_P_HWState *hwState = &g_NEXUS_PictureDecoder_P_ModuleState.hwState;
    NEXUS_PictureDecoderHandle decoder = hwState->acquired;
    unsigned eEventType = eventInfo->e_EventType;

    /* if PictureDecoder never started, hwState->acquired may not be set */
    if (!hwState->acquired) return;

    BSTD_UNUSED(pCallbackData);
    BDBG_OBJECT_ASSERT(decoder, NEXUS_PictureDecoder);
    BDBG_MSG(("xx_isr:(dec=%#lx) idx=%d %#lx event:%#x ", (unsigned long)decoder, decoder->index, (unsigned long)hSid, (unsigned)eEventType ));

    BDBG_ASSERT(decoder->sid == hSid);
    decoder->decodeState.event.eventType |= eEventType; /* eventType is really a bitmask */

    decoder->crc = eventInfo->ui32_imageCheckSum;
    decoder->errorCode = eventInfo->ui32_errorCode;

    /* see NEXUS_PictureDecoder_P_EventHandler */
    BKNI_SetEvent(decoder->event);

    return;
}


void NEXUS_PictureDecoder_P_ProcessSidError( NEXUS_PictureDecoderHandle decoder ) {

    unsigned imgIdx = 0;
    const char imageType[][6] = { "UKWN" , "JPEG" , "PNG " , "GIF " , "RLE " };
    struct NEXUS_PictureDecoder_P_HWState *hwState = &g_NEXUS_PictureDecoder_P_ModuleState.hwState;
    BSID_RevisionInfo st_RevisionInfo;

    (void)BSID_GetRevisionInfo( hwState->sid , &st_RevisionInfo );

    switch( decoder->decodeState.imageFormat ) {
        case BSID_ImageFormat_eJPEG: imgIdx=1; break;
        case BSID_ImageFormat_ePNG:  imgIdx=2; break;
        case BSID_ImageFormat_eGIF:  imgIdx=3; break;
        case BSID_ImageFormat_eRLE_BD:
        case BSID_ImageFormat_eRLE_HD_DVD:
        case BSID_ImageFormat_eRLE_Legacy_DVD :
            imgIdx=4; break;
        default : break; /* empty */
    }

    switch( decoder->errorCode ) {
        case BSID_ERR_JPEG_UNSUPP_TYPE:
            BDBG_WRN(("Unsupported jpeg file , cannot decode" ));
            break;
        case BSID_ERR_PNG_ILLEGAL_SIZE:
        case BSID_ERR_PNG_TOO_WIDE:
        case BSID_ERR_GIF_ILLEGAL_SIZE:
        case BSID_ERR_JPEG_BAD_SIZE:
            decoder->decodeState.status.header.width  = decoder->imageHeader.ui32_ImgWidth;
            decoder->decodeState.status.header.height = decoder->imageHeader.ui32_ImgHeight;
            BDBG_WRN(("%s dimensions w=%d h=%d exceeds maximum decodes w=%d h=%d, cannot decode" , imageType[imgIdx] ,
                decoder->decodeState.status.header.width, decoder->decodeState.status.header.height ,st_RevisionInfo.ui32_JpegMaxWidth ,st_RevisionInfo.ui32_JpegMaxWidth ));
            break;
    }
}

static void
NEXUS_PictureDecoder_P_Failed(NEXUS_PictureDecoderHandle decoder)
{
    unsigned multiplier=( decoder->decodeState.status.header.multiscan == true) ?  3*2 : ( decoder->decodeState.status.header.rotated == true ? 4:1);
    unsigned multiScanBufferSize = decoder->decodeState.status.header.surface.width * decoder->decodeState.status.header.surface.height * multiplier;

    decoder->state = NEXUS_PictureDecoder_P_eFailed;
    decoder->chBusy = ebufFree;

    if ( decoder->errorCode ) {
        NEXUS_PictureDecoder_P_ProcessSidError( decoder );
    }

    BDBG_WRN(("%#lx decode failed stream: %ux%u format:%u surface:%ux%u %s %s %s multiScanBuffer:%u", (unsigned long)decoder, (unsigned)decoder->decodeState.status.header.width, (unsigned)decoder->decodeState.status.header.height, (unsigned)decoder->decodeState.status.header.format, (unsigned)decoder->decodeState.status.header.surface.width, (unsigned)decoder->decodeState.status.header.surface.height, decoder->decodeState.status.header.multiscan?"multiscan":"", decoder->decodeState.status.header.grayscale?"grayscale":"", decoder->decodeState.status.header.rotated?"rotated":"", (unsigned)decoder->openSettings.multiScanBufferSize ));
    if( (decoder->decodeState.status.header.multiscan || decoder->decodeState.status.header.rotated)  && decoder->openSettings.multiScanBufferSize<multiScanBufferSize) {
        BDBG_WRN(("%#lx stream: requires multiscan buffer of at least %u KBytes (%u)", (unsigned long)decoder,
            (unsigned)multiScanBufferSize/1024, (unsigned)multiScanBufferSize ));
    }
    if ( decoder->openSettings.bufferSize < 64*1024 && !decoder->decodeState.status.header.width && !decoder->decodeState.status.header.height ) {
        BDBG_WRN(("SID could get no header info, increase openSettings.bufferSize to greater than 64*1024 (now=%d)" , decoder->openSettings.bufferSize ));
    }
    return;
}


static NEXUS_Error
NEXUS_PictureDecoder_P_DecodeSegment(NEXUS_PictureDecoderHandle decoder)
{
    BERR_Code rc=BERR_SUCCESS;
    NEXUS_SurfaceMemory memory;
    NEXUS_SurfaceCreateSettings surfaceSettings;

    BDBG_ASSERT(decoder->decodeState.segment.surface);

    NEXUS_Surface_GetMemory(decoder->decodeState.segment.surface, &memory);
    NEXUS_Surface_GetCreateSettings(decoder->decodeState.segment.surface, &surfaceSettings);

    if(decoder->decodeState.segment.rect.width == 0 && decoder->decodeState.segment.rect.height==0) {
        decoder->decodeState.segment.rect.width = decoder->decodeState.status.header.surface.width;
        decoder->decodeState.segment.rect.height = decoder->decodeState.status.header.surface.height;
    } else if(decoder->decodeState.segment.rect.width != decoder->decodeState.status.header.surface.width ||
              (unsigned)decoder->decodeState.segment.rect.y != decoder->decodeState.nextRow ||
              decoder->decodeState.segment.rect.y + decoder->decodeState.segment.rect.height > decoder->decodeState.status.header.surface.height) {
        /* only sequential horizontal stripes are supported */
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if ( decoder->startSettings.imageSize > decoder->openSettings.bufferSize ) {
        decoder->decodeState.dataMap.ui32_Size = decoder->bufferDivide;
    }
    else {
        decoder->decodeState.dataMap.ui32_Size = decoder->wr_offset;
    }
    decoder->decodeState.dataMap.ui32_Offset = NEXUS_AddrToOffset(decoder->fifo);

    if(decoder->decodeState.segment.rect.width == decoder->decodeState.status.header.surface.width &&
        decoder->decodeState.segment.rect.height == decoder->decodeState.status.header.surface.height &&
        decoder->decodeState.segment.rect.x == 0 &&  decoder->decodeState.segment.rect.y == 0) {

        unsigned multiplier=( decoder->decodeState.status.header.multiscan == true) ?  3*2 : ( decoder->decodeState.status.header.rotated == true ? 4:1);
        /* Full decode of image */
        BSID_StartDecodeSettings startDecodeSettings;
        BDBG_MSG(("Full image decode=%p idx=%d" , (void *)decoder, decoder->index ));

        BSID_GetDefaultStartDecodeSettings(BSID_DecodeMode_eStillImage, &startDecodeSettings);
        startDecodeSettings.uDecodeSettings.stImage.s_ImageBuffer.ui32_Pitch   = memory.pitch;
        startDecodeSettings.uDecodeSettings.stImage.s_ImageBuffer.ui32_Width   = surfaceSettings.width;
        startDecodeSettings.uDecodeSettings.stImage.s_ImageBuffer.ui32_Height  = surfaceSettings.height;
        startDecodeSettings.uDecodeSettings.stImage.s_ImageBuffer.ui32_Offset  = NEXUS_AddrToOffset(memory.buffer);
        startDecodeSettings.uDecodeSettings.stImage.e_ImageFormat   = decoder->decodeState.imageFormat;
        startDecodeSettings.uDecodeSettings.stImage.e_DataMapType   = BSID_DataMapType_eLinear;
        startDecodeSettings.uDecodeSettings.stImage.pv_InputDataMap = &decoder->decodeState.dataMap;
        startDecodeSettings.uDecodeSettings.stImage.ui32_MultiScanBufSize = decoder->openSettings.multiScanBufferSize;
        startDecodeSettings.uDecodeSettings.stImage.ui32_MultiScanBufAddr = decoder->multiScanBufOffset;
        startDecodeSettings.uDecodeSettings.stImage.ui32_DecodeSequenceID = 0x10;

        BKNI_EnterCriticalSection();
        decoder->state  = NEXUS_PictureDecoder_P_eDecoding;
        decoder->chBusy = ebufFilled;
        BKNI_LeaveCriticalSection();

        if ( decoder->startSettings.imageSize < decoder->openSettings.bufferSize ) {
            /* Using cdb large enough to hold image, so flag this to SID */
            startDecodeSettings.uDecodeSettings.stImage.b_LastDmaChunk = 1;
        }

        if ( ( decoder->decodeState.status.header.multiscan || decoder->decodeState.status.header.rotated ) &&
                        ( decoder->openSettings.multiScanBufferSize < (startDecodeSettings.uDecodeSettings.stImage.s_ImageBuffer.ui32_Width*startDecodeSettings.uDecodeSettings.stImage.s_ImageBuffer.ui32_Height*multiplier) )  ) {
                /* TODO , use api to get multiscanBuf size when available */
                BDBG_WRN(("For %s jpeg , need multiScanBufferSize (%d) >= width %d x height %d x %d (%d)",
                    (decoder->decodeState.status.header.multiscan == true) ? "multiscan" : "rotated" , decoder->openSettings.multiScanBufferSize,
                    startDecodeSettings.uDecodeSettings.stImage.s_ImageBuffer.ui32_Width,
                    startDecodeSettings.uDecodeSettings.stImage.s_ImageBuffer.ui32_Height, multiplier,
                    startDecodeSettings.uDecodeSettings.stImage.s_ImageBuffer.ui32_Width * startDecodeSettings.uDecodeSettings.stImage.s_ImageBuffer.ui32_Height * multiplier));
                goto error;
        }

        rc = BSID_StartDecode( decoder->sid, &startDecodeSettings);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    } else {
        /* Segmented decode of image */
        BSID_StartDecodeSettings startDecodeSettings;
        BDBG_MSG(("Segmented decode=%p idx=%d of image", (void *)decoder, decoder->index));
        BSID_GetDefaultStartDecodeSettings(BSID_DecodeMode_eStillSegment, &startDecodeSettings);
        startDecodeSettings.uDecodeSettings.stSegment.s_ImageBuffer.ui32_Pitch   = memory.pitch;
        startDecodeSettings.uDecodeSettings.stSegment.s_ImageBuffer.ui32_Width   = surfaceSettings.width;
        startDecodeSettings.uDecodeSettings.stSegment.s_ImageBuffer.ui32_Height  = surfaceSettings.height;
        startDecodeSettings.uDecodeSettings.stSegment.s_ImageBuffer.ui32_Offset  = NEXUS_AddrToOffset(memory.buffer);
        startDecodeSettings.uDecodeSettings.stSegment.e_ImageFormat   = decoder->decodeState.imageFormat;
        startDecodeSettings.uDecodeSettings.stSegment.e_DataMapType   = BSID_DataMapType_eLinear;
        startDecodeSettings.uDecodeSettings.stSegment.pv_InputDataMap = &decoder->decodeState.dataMap;
        startDecodeSettings.uDecodeSettings.stSegment.b_FirstSegment  = decoder->decodeState.segment.rect.y == 0;
        startDecodeSettings.uDecodeSettings.stSegment.ui32_SegmentHeight    = decoder->decodeState.segment.rect.height;
        startDecodeSettings.uDecodeSettings.stSegment.ui32_MultiScanBufSize = decoder->openSettings.multiScanBufferSize;
        startDecodeSettings.uDecodeSettings.stSegment.ui32_MultiScanBufAddr = decoder->multiScanBufOffset;
        startDecodeSettings.uDecodeSettings.stSegment.ui32_DecodeSequenceID = 0x5;

        if ( decoder->decodeState.status.header.multiscan  || decoder->decodeState.status.header.rotated ) {
            BDBG_ERR(("Segmented output decode of multiscan/rotated jpeg not supported!"));
            /* purposefully disable multiscan, to generate error and abort process */
            /* as Segmented output decode of multiscan/rotated jpeg not supported  */
            startDecodeSettings.uDecodeSettings.stSegment.ui32_MultiScanBufSize = 0;
            startDecodeSettings.uDecodeSettings.stSegment.ui32_MultiScanBufAddr = 0;
        }

        decoder->decodeState.nextRow += decoder->decodeState.segment.rect.height;
        BKNI_EnterCriticalSection();
        decoder->state = NEXUS_PictureDecoder_P_eDecoding;
        decoder->chBusy = ebufFilled;
        BKNI_LeaveCriticalSection();
        if ( decoder->startSettings.imageSize < decoder->openSettings.bufferSize ) {
            /* buffer is large enough to hold image, so flag this to SID */
            startDecodeSettings.uDecodeSettings.stSegment.b_LastDmaChunk = 1;
        }
        BDBG_MSG(("NEXUS_PictureDecoder_P_DecodeSegment: %#lx %u %s", (unsigned long)decoder,
          (unsigned)startDecodeSettings.uDecodeSettings.stSegment.ui32_SegmentHeight,
          startDecodeSettings.uDecodeSettings.stSegment.b_FirstSegment?"first":""));
        rc = BSID_StartDecode(decoder->sid, &startDecodeSettings);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    }

    return NEXUS_SUCCESS;

error:
    NEXUS_PictureDecoder_P_Failed(decoder);
    return rc;
}


static void
NEXUS_PictureDecoder_P_EventHandler(void *cntx)
{
    NEXUS_PictureDecoderHandle decoder = cntx;
    BERR_Code rc;
    unsigned eventType;
    BSID_StopDecodeSettings settings;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_PictureDecoder);
    BDBG_MSG(("event:(dec=%#lx) idx=%d state:%d event:%#x", (unsigned long)decoder, decoder->index, decoder->state, (unsigned)decoder->decodeState.event.eventType));
    BKNI_EnterCriticalSection();
    eventType = decoder->decodeState.event.eventType;
    decoder->decodeState.event.eventType = 0;
    BKNI_LeaveCriticalSection();

    if(decoder->state==NEXUS_PictureDecoder_P_eStopped) {
        BKNI_EnterCriticalSection();
        decoder->chBusy = ebufFree;          /* Allow Stop */
        BKNI_LeaveCriticalSection();
        goto done;
    }
    if( (eventType&(BSID_NotificationEvent_eDecodeError | BSID_NotificationEvent_eGetInfoError))!=0) {
        /* see bsid_err.h to reference value in errorCode */
        BDBG_WRN(("NEXUS_PictureDecoder_P_EventHandler: %#lx eventType error %x SID error=0x%8x", (unsigned long)decoder, eventType , decoder->errorCode));
        BKNI_EnterCriticalSection();
        decoder->chBusy = ebufFree;          /* Allow Stop */
        BKNI_LeaveCriticalSection();
        goto error;
    }
    if ( !(eventType&(BSID_NotificationEvent_eGetInfoDone)) && eventType&(BSID_NotificationEvent_eWaitingForMoreInputData) ) {
        /* fire callback to get more data */
        switch( decoder->state ) {
            case NEXUS_PictureDecoder_P_eStreamInfo:
            case NEXUS_PictureDecoder_P_eMoreInfoData:
                if ( decoder->chBusy > ebufAllocd ) {
                    BKNI_EnterCriticalSection();
                    decoder->chBusy = ebufFree;
                    BKNI_LeaveCriticalSection();
                }
                break;
            case NEXUS_PictureDecoder_P_eDecoding:
                decoder->state = NEXUS_PictureDecoder_P_eMoreImageData;
                if ( decoder->chBusy > ebufAllocd ) {
                    BKNI_EnterCriticalSection();
                    decoder->chBusy = ebufFree;
                    BKNI_LeaveCriticalSection();
                }
                break;
            default:
                /* Everything else don't change state */
                break;
        } /* end of switch decoder->state */
        NEXUS_TaskCallback_Fire(decoder->segmentDecodedCallback);
        goto done;

    }

    BSID_GetDefaultStopSettings(&settings);
    switch(decoder->state) {
    case NEXUS_PictureDecoder_P_eStreamInfo:
        if( (eventType&BSID_NotificationEvent_eGetInfoDone)!=BSID_NotificationEvent_eGetInfoDone) {
            break;
        }
        /* Stream Info Done - Stop decode */
        BSID_StopDecode(decoder->sid, &settings);
        decoder->decodeState.status.header.format = NEXUS_P_PixelFormat_FromMagnum_isrsafe(decoder->imageHeader.e_PixelFormat);
        decoder->decodeState.status.header.width  = decoder->imageHeader.ui32_ImgWidth;
        decoder->decodeState.status.header.height = decoder->imageHeader.ui32_ImgHeight;
        decoder->decodeState.status.header.multiscan = decoder->imageHeader.ui32_MultiScan != 0;
        decoder->decodeState.status.header.grayscale = decoder->imageHeader.ui32_GrayScale != 0;
        decoder->decodeState.status.header.palette = decoder->imageHeader.ui32_NumClutEntries!=0;
        decoder->decodeState.status.header.rotated = decoder->imageHeader.e_JpegSubtype == BSID_JpegSubType_e422r;
        decoder->decodeState.status.header.hasTransparentData = decoder->imageHeader.ui32_HasTransparentData;
        decoder->decodeState.status.header.transparentColorIndex = decoder->imageHeader.ui32_TransparentColorIndex;
        decoder->decodeState.status.header.transparentColorRgb = decoder->imageHeader.ui32_TransparentColorRGB;
        decoder->decodeState.status.header.surface.x = 0;
        decoder->decodeState.status.header.surface.y = 0;
        decoder->decodeState.status.header.surface.width = decoder->imageHeader.ui32_SurWidth;
        decoder->decodeState.status.header.surface.height = decoder->imageHeader.ui32_SurHeight;
        decoder->decodeState.status.headerValid = true;
        decoder->state = NEXUS_PictureDecoder_P_eStreamInfoDone;
        NEXUS_TaskCallback_Fire(decoder->pictureParsedCallback);
        BDBG_MSG(("NEXUS_PictureDecoder_P_EventHandler: %#lx stream: %ux%u format:%u surface:%ux%u %s %s %s", (unsigned long)decoder, (unsigned)decoder->decodeState.status.header.width, (unsigned)decoder->decodeState.status.header.height, (unsigned)decoder->decodeState.status.header.format, (unsigned)decoder->decodeState.status.header.surface.width, (unsigned)decoder->decodeState.status.header.surface.height, decoder->decodeState.status.header.multiscan?"multiscan":"", decoder->decodeState.status.header.grayscale?"grayscale":"", decoder->decodeState.status.header.rotated?"rotated":""));

        if(decoder->decodeState.segment.surface) {
            rc = NEXUS_PictureDecoder_P_DecodeSegment(decoder);
            if(rc!=NEXUS_SUCCESS) {rc=BERR_TRACE(rc); goto error;}
        }
        break;
    case NEXUS_PictureDecoder_P_eDecoding:
        BSID_StopDecode(decoder->sid, &settings);
        decoder->state = NEXUS_PictureDecoder_P_eSegmentDone;
        NEXUS_TaskCallback_Fire(decoder->segmentDecodedCallback);
        break;
    case NEXUS_PictureDecoder_P_eSegmentDone:
        decoder->state = NEXUS_PictureDecoder_P_eSegmentDone;
        NEXUS_TaskCallback_Fire(decoder->segmentDecodedCallback);
        break;
    default:
        break;
    }
    BKNI_EnterCriticalSection();
    decoder->chBusy = ebufFree;
    BKNI_LeaveCriticalSection();
    return;
error:
    NEXUS_PictureDecoder_P_Failed(decoder);
    NEXUS_TaskCallback_Fire(decoder->pictureParsedCallback);
    NEXUS_TaskCallback_Fire(decoder->segmentDecodedCallback);
done:
    return;
}


static void
NEXUS_PictureDecoder_P_WatchdogHandler(void *cntx)
{
    BSID_Handle sid = (BSID_Handle)cntx;
    BERR_Code rc;
    /* Anything else to cleanup here? */
    BDBG_WRN(("Picture Decoder Watchdog"));
    rc = BSID_ProcessWatchdog(sid);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); /* keep going */}
    /* Any channel/display cleanup required? */
    return;
}


NEXUS_Error
NEXUS_PictureDecoder_P_ParseHeader(NEXUS_PictureDecoderHandle decoder)
{
    BSID_StartDecodeSettings startDecodeSettings;
    BERR_Code rc;

    BSID_GetDefaultStartDecodeSettings(BSID_DecodeMode_eStillInfo, &startDecodeSettings);

    decoder->decodeState.dataMap.ui32_Size   = decoder->wr_offset;
    decoder->decodeState.dataMap.ui32_Offset = NEXUS_AddrToOffset(decoder->fifo);

    startDecodeSettings.uDecodeSettings.stStreamInfo.e_ImageFormat   = decoder->decodeState.imageFormat;
    startDecodeSettings.uDecodeSettings.stStreamInfo.e_DataMapType   = BSID_DataMapType_eLinear;
    startDecodeSettings.uDecodeSettings.stStreamInfo.pv_InputDataMap = &decoder->decodeState.dataMap;
    startDecodeSettings.uDecodeSettings.stStreamInfo.ps_OutImageInfo = &decoder->imageHeader;
    startDecodeSettings.uDecodeSettings.stStreamInfo.b_LastDmaChunk  = true;
    startDecodeSettings.uDecodeSettings.stStreamInfo.ui32_DecodeSequenceID = 0x3;
    decoder->state  = NEXUS_PictureDecoder_P_eStreamInfo;
    BKNI_EnterCriticalSection();
    decoder->chBusy = ebufFilled;
    BKNI_LeaveCriticalSection();

    rc = BSID_StartDecode( decoder->sid, &startDecodeSettings);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto error;}
    return NEXUS_SUCCESS;

error:
    decoder->chBusy  = ebufFree;
    decoder->state = NEXUS_PictureDecoder_P_eFailed;
    return rc;
}

static NEXUS_Error nexus_picturedecoder_verify(NEXUS_PictureDecoderHandle decoder, bool checkOnly )
{
    struct NEXUS_PictureDecoder_P_HWState *hwState = &g_NEXUS_PictureDecoder_P_ModuleState.hwState;

    BSTD_UNUSED(checkOnly);

    if (!hwState->acquired) {
        /* the first user of an unacquired decoder acquires it */
        BDBG_MSG(("%p acquired", (void *)decoder));
        hwState->acquired = decoder;
    }
    else if (hwState->acquired != decoder) {
        /* already acquired. no not use BERR_TRACE because this will eventually pass when FW is multicontext */
        return NEXUS_NOT_AVAILABLE;
    }
    /* acquired */
    return 0;
}

NEXUS_Error
NEXUS_PictureDecoder_Start(NEXUS_PictureDecoderHandle decoder, const NEXUS_PictureDecoderStartSettings *pSettings)
{
    NEXUS_Error rc;
    NEXUS_PictureDecoderStartSettings settings;
    BDBG_OBJECT_ASSERT(decoder, NEXUS_PictureDecoder);

    if (nexus_picturedecoder_verify(decoder,false)) {
        return NEXUS_NOT_AVAILABLE; /* currently, not an error. */
    }

    if(decoder->state != NEXUS_PictureDecoder_P_eStopped) { return BERR_TRACE(BERR_NOT_SUPPORTED); }

    if ( decoder->wr_offset == 0 ) return BERR_TRACE(BERR_INVALID_PARAMETER);

    if(!pSettings) {
        NEXUS_PictureDecoder_GetDefaultStartSettings(&settings);
        pSettings = &settings;
    }

    switch(pSettings->format) {
#if 0
    case NEXUS_PictureFormat_eRleDvd:
        decoder->decodeState.imageFormat = BSID_ImageFormat_eRLE_Legacy_DVD;
        break;
    case NEXUS_PictureFormat_eRleBluRay:
        decoder->decodeState.imageFormat = BSID_ImageFormat_eRLE_BD;
        break;
#endif
    case NEXUS_PictureFormat_eJpeg:
        decoder->decodeState.imageFormat = BSID_ImageFormat_eJPEG;
        break;
    case NEXUS_PictureFormat_ePng:
        decoder->decodeState.imageFormat = BSID_ImageFormat_ePNG;
        break;
    case NEXUS_PictureFormat_eGif:
        decoder->decodeState.imageFormat = BSID_ImageFormat_eGIF;
        break;
    case NEXUS_PictureFormat_eMpeg:
        decoder->decodeState.imageFormat = BSID_ImageFormat_eMPEG_IFRAME;
        break;
    case NEXUS_PictureFormat_eRleDvd:
        /* coverity[unterminated_case] */
    case NEXUS_PictureFormat_eRleBluRay:
        decoder->state = NEXUS_PictureDecoder_P_eFailed;
        /* coverity[fallthrough] */
    default:
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }


    decoder->startSettings = *pSettings;
    if ( decoder->startSettings.imageSize > decoder->openSettings.bufferSize ) {
        decoder->remainingBytes = decoder->startSettings.imageSize;
        decoder->bufferDivide   = decoder->openSettings.bufferSize;
        BDBG_MSG(("imageSize(%d) > bufferSize(%d), small buffer decode enabled" , decoder->startSettings.imageSize, decoder->openSettings.bufferSize ));
    }

    NEXUS_Module_TaskCallback_Set(decoder->pictureParsedCallback, &decoder->startSettings.pictureParsed);
    NEXUS_Module_TaskCallback_Set(decoder->segmentDecodedCallback, &decoder->startSettings.segmentDecoded);
    decoder->state = NEXUS_PictureDecoder_P_eWaitingData;

    if(decoder->wr_offset>0) {
        rc = NEXUS_PictureDecoder_P_ParseHeader(decoder);
        if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    }
    return NEXUS_SUCCESS;
}

void
NEXUS_PictureDecoder_Stop(NEXUS_PictureDecoderHandle decoder)
{
    struct NEXUS_PictureDecoder_P_HWState *hwState = &g_NEXUS_PictureDecoder_P_ModuleState.hwState;
    BSID_StopDecodeSettings settings;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_PictureDecoder);

    if (decoder->state == NEXUS_PictureDecoder_P_eStopped) {
        return;
    }

    if (nexus_picturedecoder_verify(decoder,true)) {
        return; /* currently, not an error. */
    }

    NEXUS_Module_TaskCallback_Set(decoder->pictureParsedCallback, NULL);
    NEXUS_Module_TaskCallback_Set(decoder->segmentDecodedCallback, NULL);

    if ( decoder->state == NEXUS_PictureDecoder_P_eDecoding ||
         decoder->state == NEXUS_PictureDecoder_P_eStreamInfo ||
         decoder->state == NEXUS_PictureDecoder_P_eMoreInfoData ||
         decoder->state == NEXUS_PictureDecoder_P_eMoreImageData ) {
        BSID_GetDefaultStopSettings(&settings);
        settings.bForceStop = true;
        BSID_StopDecode(decoder->sid, &settings);
    }

    NEXUS_PictureDecoder_P_Reset(decoder);
    hwState->acquired = NULL;

    return ;
}

NEXUS_Error
NEXUS_PictureDecoder_GetBuffer(NEXUS_PictureDecoderHandle decoder, void **pBuffer, size_t *pSize)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_PictureDecoder);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pSize);

    if (nexus_picturedecoder_verify(decoder,false)) {
        *pBuffer = NULL;
        *pSize = 0;
        return NEXUS_NOT_AVAILABLE; /* currently, not an error. */
    }

    switch(decoder->state) {
    case NEXUS_PictureDecoder_P_eStopped:
    case NEXUS_PictureDecoder_P_eWaitingData:
    case NEXUS_PictureDecoder_P_eMoreInfoData:
    case NEXUS_PictureDecoder_P_eMoreImageData:
    case NEXUS_PictureDecoder_P_eStreamInfoDone:
    case NEXUS_PictureDecoder_P_eSegmentDone:
    case NEXUS_PictureDecoder_P_eDecoding:
        break;
    default:
        *pBuffer = NULL;
        *pSize = 0;
        BDBG_WRN(("Dec->state=%d " , decoder->state ));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( decoder->startSettings.imageSize > decoder->openSettings.bufferSize ) {
        /* Small buffer mode */
        if ( decoder->chBusy ) {
            *pBuffer = NULL;
            *pSize = 0;
            return BERR_UNKNOWN;  /* SID is still processing data, so can't return buffer until it has finished */
        }
        *pSize = decoder->openSettings.bufferSize;
        {
            *pBuffer = (uint8_t *)decoder->fifo;
            BDBG_MSG_TRACE(("Frnt dec->fifo=0x%p physDecFifo=0x%p pBuffer=0x%8x pSize=%d dec->bufDiv=%d" , decoder->fifo ,NEXUS_AddrToOffset(*pBuffer),
                                                                                                        *pBuffer, *pSize, decoder->bufferDivide ));
            decoder->chBusy = ebufAllocd;
        }
    }
    else {
        /* full size buffer mode */
        *pBuffer = (uint8_t *)decoder->fifo + decoder->wr_offset;
        *pSize = decoder->openSettings.bufferSize - decoder->wr_offset;
         BDBG_MSG(("FULL dec=%p dec->fifo=%p pBuffer=%p pSize=%u", (void *)decoder, (void *)decoder->fifo ,*pBuffer, (unsigned)*pSize ));
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_PictureDecoder_ReadComplete(NEXUS_PictureDecoderHandle decoder, size_t skip, size_t amountUsed)
{
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(decoder, NEXUS_PictureDecoder);

    if (nexus_picturedecoder_verify(decoder,false)) {
        return NEXUS_NOT_AVAILABLE; /* currently, not an error. */
    }
    BDBG_MSG(("ReadComplete: state=%d, skip=%d, amount_used=%d", decoder->state, (unsigned)skip, (unsigned)amountUsed));

    switch(decoder->state) {
    case NEXUS_PictureDecoder_P_eMoreInfoData:
    case NEXUS_PictureDecoder_P_eMoreImageData:
        break;
    case NEXUS_PictureDecoder_P_eStopped:
    case NEXUS_PictureDecoder_P_eWaitingData:
        break;
    case NEXUS_PictureDecoder_P_eStreamInfoDone:
    case NEXUS_PictureDecoder_P_eSegmentDone:
        break;
    case NEXUS_PictureDecoder_P_eFailed:
        return NEXUS_NOT_AVAILABLE; /* decode failed, there's nothing to be done */
        break;
    case NEXUS_PictureDecoder_P_eStreamInfo:
        BDBG_WRN(("SID decode buffer(=%d kByte) is probably too small to decode header, increase buffer size, decoder->state=%d" ,
            (decoder->openSettings.bufferSize/1024), decoder->state ));
        break;
    default:
        BDBG_WRN(("decoder->state=%d Idx=%d" , decoder->state, decoder->index ));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    if(skip!=0) { return BERR_TRACE(BERR_NOT_SUPPORTED);}
    if( !decoder->startSettings.imageSize && amountUsed > (decoder->openSettings.bufferSize - decoder->wr_offset)) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( decoder->startSettings.imageSize > decoder->openSettings.bufferSize ) {
        BSID_DmaChunkInfo DMAChunkInfo;

        if ( decoder->remainingBytes > decoder->bufferDivide ) {
            DMAChunkInfo.ui32_Size = decoder->bufferDivide;
        }
        else {
            DMAChunkInfo.ui32_Size = decoder->remainingBytes;
        }
        decoder->remainingBytes   -= DMAChunkInfo.ui32_Size;

        DMAChunkInfo.ui32_Offset = NEXUS_AddrToOffset(decoder->fifo);
        NEXUS_FlushCache((uint8_t *)decoder->fifo, decoder->openSettings.bufferSize ); /* just flush part of buffer */

        DMAChunkInfo.ui32_LastDmaChunk = (decoder->remainingBytes<= 0);

        BKNI_EnterCriticalSection();
        if ( decoder->chBusy == ebufAllocd && decoder->state != NEXUS_PictureDecoder_P_eSegmentDone && decoder->state != NEXUS_PictureDecoder_P_eStreamInfoDone ) {
            decoder->chBusy  = ebufFilled;
            decoder->state   = NEXUS_PictureDecoder_P_eDecoding;
            BKNI_LeaveCriticalSection();
            /* Start SID now */
            BSID_SetDmaChunkInfo( decoder->sid, &DMAChunkInfo);

        }
        else {
            BKNI_LeaveCriticalSection();
        }
    } /* end of small buffer model */
    else {
        /* large buffer model */
        NEXUS_FlushCache((uint8_t *)decoder->fifo, amountUsed );
    }
    decoder->wr_offset += amountUsed;

    if(decoder->state == NEXUS_PictureDecoder_P_eWaitingData && decoder->wr_offset>0) {
        rc = NEXUS_PictureDecoder_P_ParseHeader(decoder);
        if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    }

    return NEXUS_SUCCESS;
}


NEXUS_Error
NEXUS_PictureDecoder_GetStatus(NEXUS_PictureDecoderHandle decoder, NEXUS_PictureDecoderStatus *pStatus)
{
    struct NEXUS_PictureDecoder_P_HWState *hwState = &g_NEXUS_PictureDecoder_P_ModuleState.hwState;

    BDBG_OBJECT_ASSERT(decoder, NEXUS_PictureDecoder);
    BDBG_ASSERT(pStatus);

    if (nexus_picturedecoder_verify(decoder,false)) {
        return NEXUS_NOT_AVAILABLE; /* currently, not an error. */
    }

    *pStatus = decoder->decodeState.status;
    BKNI_EnterCriticalSection();
    switch(decoder->state) {
    case NEXUS_PictureDecoder_P_eSegmentDone:
        pStatus->state = NEXUS_PictureDecoderState_eSegmentDone;
        pStatus->crc = decoder->crc;
        break;
    case NEXUS_PictureDecoder_P_eStopped:
        pStatus->state = NEXUS_PictureDecoderState_eIdle;
        break;
    case NEXUS_PictureDecoder_P_eFailed:
        pStatus->state = NEXUS_PictureDecoderState_eError;
        break;
    case NEXUS_PictureDecoder_P_eMoreInfoData:
        pStatus->state = NEXUS_PictureDecoderState_eMoreData;
        break;
    case NEXUS_PictureDecoder_P_eMoreImageData:
        pStatus->state = NEXUS_PictureDecoderState_eMoreData;
        break;
    default:
        pStatus->state = NEXUS_PictureDecoderState_eDecoding;
        break;
    }
    BKNI_LeaveCriticalSection();

    pStatus->version.firmware = hwState->firmwareVersion;

    BDBG_MSG(("NEXUS_PictureDecoder_GetStatus:%#lx index=%d state:%u", (unsigned long)decoder, decoder->index , (unsigned)pStatus->state));
    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_PictureDecoder_GetPalette(NEXUS_PictureDecoderHandle decoder, NEXUS_PictureDecoderPalette *pPalette)
{
    BDBG_OBJECT_ASSERT(decoder, NEXUS_PictureDecoder);
    BDBG_ASSERT(pPalette);
    if (nexus_picturedecoder_verify(decoder,false)) {
        return NEXUS_NOT_AVAILABLE; /* currently, not an error. */
    }
    if(!decoder->decodeState.status.headerValid) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    pPalette->numPaletteEntries = decoder->imageHeader.ui32_NumClutEntries;

    BDBG_CASSERT(sizeof(pPalette->palette) == sizeof(decoder->imageHeader.aui32_Clut));
    BKNI_Memcpy(pPalette->palette, decoder->imageHeader.aui32_Clut, sizeof(pPalette->palette));
    return NEXUS_SUCCESS;
}

NEXUS_Error
NEXUS_PictureDecoder_DecodeSegment(NEXUS_PictureDecoderHandle decoder, NEXUS_SurfaceHandle destination, const NEXUS_Rect *segment)
{
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(decoder, NEXUS_PictureDecoder);
    BDBG_ASSERT(destination);

    if (nexus_picturedecoder_verify(decoder,false)) {
        return NEXUS_NOT_AVAILABLE; /* currently, not an error. */
    }
    decoder->decodeState.segment.surface = destination;
    if(segment) {
        if(segment->x != 0  || segment->width <= 0 || segment->height<=0 || segment->y<0) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
        decoder->decodeState.segment.rect = *segment;
    } else {
        decoder->decodeState.segment.rect.x = 0;
        decoder->decodeState.segment.rect.y = 0;
        decoder->decodeState.segment.rect.width = 0;
        decoder->decodeState.segment.rect.height = 0;
    }

    if(decoder->state == NEXUS_PictureDecoder_P_eStreamInfoDone || decoder->state == NEXUS_PictureDecoder_P_eSegmentDone)
    {
        rc = NEXUS_PictureDecoder_P_DecodeSegment(decoder);
        if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
    }

    return NEXUS_SUCCESS;
}

NEXUS_PictureDecoderHandle
NEXUS_PictureDecoder_Open(unsigned index, const NEXUS_PictureDecoderOpenSettings *pSettings)
{
    NEXUS_PictureDecoderHandle decoder;
    NEXUS_PictureDecoderOpenSettings settings;
    BERR_Code rc;
    NEXUS_HeapHandle heap, multiscanHeap;
    struct NEXUS_PictureDecoder_P_HWState *hwState = &g_NEXUS_PictureDecoder_P_ModuleState.hwState;

    if(!pSettings) {
        NEXUS_PictureDecoder_GetDefaultOpenSettings(&settings);
        pSettings = &settings;
    }

    heap = NEXUS_P_DefaultHeap(pSettings->heap, NEXUS_DefaultHeapType_eFull);
    if (!heap) heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    if (!NEXUS_P_CpuAccessibleHeap(heap)) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }
    /* multiscan buffer does not need to be cpu accessible */
    /* TODO: may need pSettings->multiscanHeap */
    multiscanHeap = NEXUS_P_DefaultHeap(pSettings->heap, NEXUS_DefaultHeapType_eAny);
    if (!multiscanHeap) multiscanHeap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;

    if (pSettings->bufferPtr) {
        if (!NEXUS_P_CpuAccessibleAddress(pSettings->bufferPtr)) {
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            return NULL;
        }
    }

    rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(pictureDecoder,Count,NEXUS_ANY_ID);
    if (rc) { rc = BERR_TRACE(rc); return NULL; }

    decoder = BKNI_Malloc(sizeof(*decoder));
    if(!decoder) {
        NEXUS_CLIENT_RESOURCES_RELEASE(pictureDecoder,Count,NEXUS_ANY_ID);
        rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_PictureDecoder, decoder);
    decoder->heap = NEXUS_Heap_GetMemHandle(heap);
    decoder->multiscanHeap = NEXUS_Heap_GetMemHandle(multiscanHeap);
    decoder->index = index;
    decoder->openSettings = *pSettings;
    NEXUS_PictureDecoder_GetDefaultStartSettings(&decoder->startSettings);

    /* alloc multiscan buffer */
    decoder->multiScanBuffer = NULL;
    decoder->multiScanBufOffset = 0;
    if(pSettings->multiScanBufferSize) {
        if(pSettings->multiScanBufferPtr==NULL) {
            decoder->multiScanBuffer = BMEM_Alloc(decoder->multiscanHeap, pSettings->multiScanBufferSize);
            if(!decoder->multiScanBuffer) {rc=BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);goto err_multiscanbuffer;}
            decoder->multiScanBufOffset = NEXUS_AddrToOffset(decoder->multiScanBuffer);
        } else {
            decoder->multiScanBufOffset = NEXUS_AddrToOffset(pSettings->multiScanBufferPtr);
            if(decoder->multiScanBufOffset==0) {rc=BERR_TRACE(NEXUS_INVALID_PARAMETER);goto err_multiscanbuffer;}
            decoder->multiScanBuffer = NEXUS_OffsetToUncachedAddr(decoder->multiScanBufOffset);
        }
    }
    decoder->remainingBytes = 0;

    if(pSettings->bufferPtr==NULL) {
        decoder->memFifo = BMEM_Alloc(decoder->heap, pSettings->bufferSize);
        if(!decoder->memFifo) {rc=BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);goto err_fifo;}
        rc = BMEM_Heap_ConvertAddressToCached(decoder->heap, decoder->memFifo, &decoder->fifo);
        if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_fifo_cached;}
    } else {
        decoder->memFifo = NULL;
        decoder->fifo = pSettings->bufferPtr;
    }

    rc = BKNI_CreateEvent(&decoder->event);
    if(rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc);goto err_event;}
    decoder->eventHandler = NEXUS_RegisterEvent(decoder->event, NEXUS_PictureDecoder_P_EventHandler, decoder);
    if(!decoder->eventHandler) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_event_handler;}

    decoder->pictureParsedCallback = NEXUS_TaskCallback_Create(decoder, NULL);
    if(!decoder->pictureParsedCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_picture_header_callback;}

    decoder->segmentDecodedCallback = NEXUS_TaskCallback_Create(decoder, NULL);
    if(!decoder->segmentDecodedCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto err_segment_decoded_callback;}

    NEXUS_PictureDecoder_P_Reset(decoder);

    if ( !hwState->sidCh ) {
        BSID_OpenChannelSettings sidChOpenSettings;
        /* decoder handle gets a unique sid channel */
        BSID_GetDefaultOpenChannelSettings( BSID_ChannelType_eStill, &sidChOpenSettings  );

        sidChOpenSettings.e_ChannelType = BSID_ChannelType_eStill;
        sidChOpenSettings.ui32_ChannelPriority = 0;
        sidChOpenSettings.u_ChannelSpecific.still.ui32_QueueDepth = 5;
        sidChOpenSettings.u_ChannelSpecific.still.p_CallbackFunc_isr  = (BSID_CallbackFunc)NEXUS_PictureDecoder_P_Callback_isr;
        sidChOpenSettings.u_ChannelSpecific.still.ui32_CallbackEvents =   (BSID_NotificationEvent_eGetInfoDone |
                                                          BSID_NotificationEvent_eDecodeDone |
                                                          BSID_NotificationEvent_eGetInfoError |
                                                          BSID_NotificationEvent_eDecodeError |
                                                          BSID_NotificationEvent_eWaitingForMoreInputData);
        /* Setting these means no memory bounds check is performed */
        sidChOpenSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Offset = 0x0;
        sidChOpenSettings.u_ChannelSpecific.still.s_MemoryBounds.ui32_Size   = 0x0;

        /* open sid channel*/
        rc = BSID_OpenChannel( hwState->sid, &decoder->sid, 0 /* index */ , &sidChOpenSettings);
        if (rc != BERR_SUCCESS) {
            BDBG_WRN(("app_main: BSID_OpenChannel decoder=%p idx=%d failed err=%#x", (void *)decoder, 0 /*index */, rc));
            goto err_segment_decoded_callback;
        }
        hwState->sidCh = decoder->sid;
    }
    else {
        /* Copy settings we previously opened with */
        decoder->sid = hwState->sidCh; /* copy sid channel to new decoder context */
    }

    BLST_S_INSERT_HEAD(&hwState->list, decoder, link);

    return decoder;

err_segment_decoded_callback:
    NEXUS_TaskCallback_Destroy(decoder->pictureParsedCallback);
err_picture_header_callback:
    NEXUS_UnregisterEvent(decoder->eventHandler);
err_event_handler:
    BKNI_DestroyEvent(decoder->event);
err_event:
err_fifo_cached:
    if(pSettings->bufferPtr==NULL) {
        BMEM_Free(decoder->heap, decoder->memFifo);
    }
err_fifo:
    if(pSettings->multiScanBufferPtr==NULL && decoder->multiScanBuffer) {
        BMEM_Free(decoder->multiscanHeap, decoder->multiScanBuffer);
    }
err_multiscanbuffer:
    NEXUS_CLIENT_RESOURCES_RELEASE(pictureDecoder,Count,NEXUS_ANY_ID);
    BDBG_OBJECT_DESTROY(decoder, NEXUS_PictureDecoder);
    BKNI_Free(decoder);
    return NULL;
}

static void
NEXUS_PictureDecoder_P_Release( NEXUS_PictureDecoderHandle decoder)
{
    NEXUS_OBJECT_ASSERT(NEXUS_PictureDecoder, decoder);
    NEXUS_CLIENT_RESOURCES_RELEASE(pictureDecoder,Count,NEXUS_ANY_ID);
}

static void
NEXUS_PictureDecoder_P_Finalizer( NEXUS_PictureDecoderHandle decoder)
{
    struct NEXUS_PictureDecoder_P_HWState *hwState = &g_NEXUS_PictureDecoder_P_ModuleState.hwState;

    NEXUS_OBJECT_ASSERT(NEXUS_PictureDecoder, decoder);

    NEXUS_PictureDecoder_Stop(decoder);

    if(decoder->openSettings.multiScanBufferPtr==NULL && decoder->multiScanBuffer) {
        BMEM_Free(decoder->multiscanHeap, decoder->multiScanBuffer);
    }
    if(decoder->openSettings.bufferPtr==NULL) {
        BMEM_Free(decoder->heap, decoder->memFifo);
    }

    BLST_S_REMOVE(&hwState->list, decoder, NEXUS_PictureDecoder, link);

    /* close sid channel */
    if (!BLST_S_FIRST(&hwState->list)) {
        BDBG_ASSERT(decoder->sid);
        BSID_CloseChannel(decoder->sid);
        hwState->sidCh = NULL;
    }

    NEXUS_TaskCallback_Destroy(decoder->segmentDecodedCallback);
    NEXUS_TaskCallback_Destroy(decoder->pictureParsedCallback);
    NEXUS_UnregisterEvent(decoder->eventHandler);
    BKNI_DestroyEvent(decoder->event);

    NEXUS_OBJECT_DESTROY(NEXUS_PictureDecoder, decoder);
    BKNI_Free(decoder);
    if ( hwState->acquired == decoder ) {
        hwState->acquired = NULL;
    }

    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_PictureDecoder, NEXUS_PictureDecoder_Close);

NEXUS_Error NEXUS_PictureDecoderModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct NEXUS_PictureDecoder_P_HWState *hwState = &g_NEXUS_PictureDecoder_P_ModuleState.hwState;
    NEXUS_PictureDecoderHandle decoder;
    BSTD_UNUSED(pSettings);

    if (enabled) {
        for (decoder = BLST_S_FIRST(&hwState->list); decoder; decoder = BLST_S_NEXT(decoder, link)) {
            /* If all channels are stopped, that ensures that the SID clock is released */
            if(decoder->state != NEXUS_PictureDecoder_P_eStopped) {
                BDBG_WRN(("Picture decoder %u:%p needs to be stopped before entering standby", decoder->index, (void *)decoder));
                return NEXUS_NOT_SUPPORTED;
            }
        }
        rc = BSID_Standby(hwState->sid);
        if (rc!=BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
        }

        #if NEXUS_HAS_SECURITY
        LOCK_SECURITY();
        NEXUS_Security_RegionVerifyDisable_priv( NEXUS_SecurityRegverRegionID_eSid0 );
        UNLOCK_SECURITY();
        #endif
    }
    else {
        rc = BSID_Resume(hwState->sid);
        if (rc!=BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
        }
    }
    return rc;
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);

    return NEXUS_SUCCESS;
#endif
}
