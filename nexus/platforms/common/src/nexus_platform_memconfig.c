/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

#include "nexus_platform_priv.h"

#include "nexus_types.h"
#include "nexus_base.h"
#include "priv/nexus_core.h"
#if NEXUS_HAS_VIDEO_DECODER
#include "priv/nexus_video_decoder_priv.h"
#include "bxvd.h"
#endif
#if NEXUS_HAS_DISPLAY
#include "priv/nexus_display_priv.h"
#include "bvdc.h"
#if NEXUS_VBI_SUPPORT
#include "bvbi_cap.h" /* for BVBI_NUM_IN656 */
#endif
#endif
#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
#include "bvce.h"
#endif
#if NEXUS_HAS_SAGE
#include "bsagelib_types.h"
#endif
BDBG_MODULE(nexus_platform_settings); /* reuse same module as heap config */

/* TODO: move to nexus_platform.c once available in all platforms */
static struct NEXUS_PlatformSpecificOps g_platformSpecificOps;

#if NEXUS_HAS_VIDEO_DECODER || NEXUS_HAS_DISPLAY
static bool nexus_p_usepicbuf(NEXUS_SecureVideo secure, enum nexus_memconfig_picbuftype picbuftype)
{
    switch (secure) {
    case NEXUS_SecureVideo_eUnsecure: return picbuftype == nexus_memconfig_picbuftype_unsecure;
    case NEXUS_SecureVideo_eSecure:   return picbuftype == nexus_memconfig_picbuftype_secure;
    default:                          return true;
    }
}
#endif

struct NEXUS_MemoryLayout
{
    struct {
        unsigned main; /* There is one "main" heap which is used as the internal default heap. It is fully mapped and on MEMC0. */
        unsigned pictureBuffer[NEXUS_MAX_MEMC][nexus_memconfig_picbuftype_max]; /* used by the decoder and display for uncompressed pictures. sized with MemConfig */
        unsigned graphics[2]; /* Graphics heaps are large, application-only mapped heaps used for offscreen graphics.
            graphics[0] corresponds to NEXUS_Platform_GetFramebufferHeapIndex(NEXUS_OFFSCREEN_SURFACE). It is common to all platforms.
            graphics[1] corresponds to NEXUS_Platform_GetFramebufferHeapIndex(NEXUS_SECONDARY_OFFSCREEN_SURFACE). It only exists on a subset of platforms. */
    } heapIndex;
};

static void NEXUS_P_GetMemoryLayout(struct NEXUS_MemoryLayout *pLayout, const NEXUS_PlatformSettings *pPlatformSettings)
{
    unsigned i;

    BKNI_Memset(pLayout, 0, sizeof(*pLayout));
#if NEXUS_PLATFORM_DEFAULT_HEAP
    pLayout->heapIndex.main = NEXUS_PLATFORM_DEFAULT_HEAP;
#else
    pLayout->heapIndex.main = 0;
#endif
    for (i=0;i<NEXUS_MAX_MEMC;i++) {
        enum nexus_memconfig_picbuftype sec;
        for (sec=0;sec<nexus_memconfig_picbuftype_max;sec++) {
            pLayout->heapIndex.pictureBuffer[i][sec] = NEXUS_MAX_HEAPS;
        }
    }
    pLayout->heapIndex.pictureBuffer[0][nexus_memconfig_picbuftype_unsecure] = NEXUS_MEMC0_PICTURE_BUFFER_HEAP;
#if NEXUS_NUM_MEMC > 1
    pLayout->heapIndex.pictureBuffer[1][nexus_memconfig_picbuftype_unsecure] = NEXUS_MEMC1_PICTURE_BUFFER_HEAP;
#endif
#if NEXUS_NUM_MEMC > 2
    pLayout->heapIndex.pictureBuffer[2][nexus_memconfig_picbuftype_unsecure] = NEXUS_MEMC2_PICTURE_BUFFER_HEAP;
#endif
    pLayout->heapIndex.pictureBuffer[0][nexus_memconfig_picbuftype_secure] = NEXUS_MEMC0_SECURE_PICTURE_BUFFER_HEAP;
    pLayout->heapIndex.pictureBuffer[1][nexus_memconfig_picbuftype_secure] = NEXUS_MEMC1_SECURE_PICTURE_BUFFER_HEAP;
    pLayout->heapIndex.pictureBuffer[2][nexus_memconfig_picbuftype_secure] = NEXUS_MEMC2_SECURE_PICTURE_BUFFER_HEAP;

    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (!pPlatformSettings->heap[i].size) continue;
        if (pPlatformSettings->heap[i].heapType & NEXUS_HEAP_TYPE_GRAPHICS) {
            pLayout->heapIndex.graphics[0] = i;
        }
        if (pPlatformSettings->heap[i].heapType & NEXUS_HEAP_TYPE_SECONDARY_GRAPHICS) {
            pLayout->heapIndex.graphics[1] = i;
        }
    }
}

void NEXUS_GetDefaultMemoryConfigurationSettings_tagged( NEXUS_MemoryConfigurationSettings *pSettings, size_t size )
{
    const NEXUS_Core_PreInitState *preInitState;
    preInitState = NEXUS_Platform_P_PreInit();
    if (!preInitState) return; /* no BERR_TRACE possible */
    if(size == sizeof(*pSettings)) {
        NEXUS_P_GetDefaultMemoryConfigurationSettings(preInitState, pSettings);
    } else {
        BDBG_ERR(("NEXUS_GetDefaultMemoryConfigurationSettings: size mismatch %u != %u", (unsigned)sizeof(*pSettings), (unsigned)size));
        BKNI_Memset(pSettings, 0, size);
    }
    NEXUS_Platform_P_PreUninit();
    return;
}

/* move large structs off the stack */
struct NEXUS_P_GetDefaultMemoryConfigurationSettings_structs
{
#if NEXUS_HAS_VIDEO_DECODER
    struct {
        NEXUS_VideoDecoderModuleSettings moduleSettings;
    } xvd;
#endif
#if NEXUS_HAS_DISPLAY
    struct {
        BVDC_MemConfigSettings memConfigSettings;
    } vdc;
#endif
#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
    struct {
        BVCE_Channel_MemoryBoundsSettings channelSettings;
    } vce;
#endif
#if !NEXUS_HAS_VIDEO_DECODER && !NEXUS_HAS_DISPLAY
    unsigned unused;
#endif
};

#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
static int nexus_p_encoder_device_and_channel(const NEXUS_Core_PreInitState *preInitState, unsigned i, unsigned *pDevice, unsigned *pChannel)
{
    unsigned device, channel;
    for (device=0;device<BBOX_VCE_MAX_INSTANCE_COUNT;device++) {
        for (channel=0;channel<BBOX_VCE_MAX_CHANNEL_COUNT;channel++) {
            if (!(preInitState->boxConfig.stVce.stInstance[device].uiChannels & (1<<channel))) break;
            if (!i) {
                *pDevice = device;
                *pChannel = channel;
                return 0;
            }
            i--;
        }
    }
    return -1;
}
#endif

#if NEXUS_HAS_VIDEO_DECODER
static int nexus_p_xvd_device_channel(const NEXUS_Core_PreInitState *preInitState, unsigned index, unsigned *device, unsigned *channel)
{
    for (*device=0;*device<BBOX_XVD_MAX_DECODERS;(*device)++) {
        for (*channel=0;*channel<BBOX_XVD_MAX_CHANNELS && *channel<preInitState->boxConfig.stXvd.stInstance[*device].stDevice.numChannels;(*channel)++) {
            if (preInitState->boxConfig.stXvd.stInstance[*device].stDevice.stChannel[*channel].nexusIndex == index) {
                return 0;
            }
        }
    }
    return -1;
}

static NEXUS_VideoFormat nexus_p_get_format(BXVD_DecodeResolution res, unsigned framerate)
{
    switch (res) {
    default:
    case BXVD_DecodeResolution_eHD:
        switch (framerate) {
        case 25:
        case 30:
            return NEXUS_VideoFormat_e1080p30hz;
        case 50:
        default:
        case 60:
            return NEXUS_VideoFormat_e1080p;
        }
        break;
    case BXVD_DecodeResolution_eSD:
        switch (framerate) {
        case 25:
        case 50:
            return NEXUS_VideoFormat_ePal;
        case 30:
        default:
        case 60:
            return NEXUS_VideoFormat_eNtsc;
        }
        break;
    case BXVD_DecodeResolution_e4K:
        switch (framerate) {
        case 25:
        case 30:
            return NEXUS_VideoFormat_e3840x2160p30hz;
        case 50:
        default:
        case 60:
            return NEXUS_VideoFormat_e3840x2160p60hz;
        }
        break;
    }
}
#endif

void NEXUS_P_GetDefaultMemoryConfigurationSettings(const NEXUS_Core_PreInitState *preInitState, NEXUS_MemoryConfigurationSettings *pSettings)
{
    unsigned i;
    struct NEXUS_P_GetDefaultMemoryConfigurationSettings_structs *structs;

    structs = BKNI_Malloc(sizeof(*structs));
    if (!structs) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return;
    }

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

#if NEXUS_HAS_VIDEO_DECODER
    {
    NEXUS_VideoDecoderModule_GetDefaultSettings(&structs->xvd.moduleSettings);
    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++) {
        unsigned device, channel;

        if (preInitState->boxMode) {
            if (nexus_p_xvd_device_channel(preInitState, i, &device, &channel)) {
                continue;
            }
            pSettings->videoDecoder[i].used = true;
            pSettings->videoDecoder[i].maxFormat = nexus_p_get_format(
                preInitState->boxConfig.stXvd.stInstance[device].stDevice.stChannel[channel].DecoderUsage.maxResolution,
                preInitState->boxConfig.stXvd.stInstance[device].stDevice.stChannel[channel].DecoderUsage.framerate);
            pSettings->videoDecoder[i].colorDepth = preInitState->boxConfig.stXvd.stInstance[device].stDevice.stChannel[channel].DecoderUsage.bitDepth;
        }
        else {
            pSettings->videoDecoder[i].used = true;
            pSettings->videoDecoder[i].maxFormat = NEXUS_VideoFormat_e1080p;
            pSettings->videoDecoder[i].colorDepth = 8;
        }
        /* TODO: get codec support from BBOX_XVD */
        BKNI_Memcpy(pSettings->videoDecoder[i].supportedCodecs, structs->xvd.moduleSettings.supportedCodecs, sizeof(pSettings->videoDecoder[i].supportedCodecs));
#if NEXUS_NUM_MOSAIC_DECODES
        if (i == 0) {
            NEXUS_VideoFormatInfo info;
            NEXUS_VideoFormat_GetInfo_isrsafe(pSettings->videoDecoder[i].maxFormat, &info);
            if (info.height > 1088) {
                /* a 4K decoder can do 3 HD mosaics */
                pSettings->videoDecoder[i].mosaic.maxNumber = 3;
                pSettings->videoDecoder[i].mosaic.maxWidth = 1920;
                pSettings->videoDecoder[i].mosaic.maxHeight = 1080;
            }
            else {
                pSettings->videoDecoder[i].mosaic.maxNumber = NEXUS_NUM_MOSAIC_DECODES;
                pSettings->videoDecoder[i].mosaic.maxWidth = 352; /* CIF */
                pSettings->videoDecoder[i].mosaic.maxHeight = 288;
            }
            pSettings->videoDecoder[i].mosaic.colorDepth = 8;
        }
#endif
    }
#if NEXUS_NUM_STILL_DECODES
    /* default only one based on first decoder */
    pSettings->stillDecoder[0].used = true;
    BKNI_Memcpy(pSettings->stillDecoder[0].supportedCodecs, pSettings->videoDecoder[0].supportedCodecs, sizeof(pSettings->stillDecoder[0].supportedCodecs));
    pSettings->stillDecoder[0].maxFormat = pSettings->videoDecoder[0].maxFormat;
    pSettings->stillDecoder[0].colorDepth = pSettings->videoDecoder[0].colorDepth;
#endif
    }
#endif

#if NEXUS_HAS_DISPLAY
    {
        bool mtg = false;
        BVDC_GetDefaultMemConfigSettings(&structs->vdc.memConfigSettings);

#if NEXUS_MTG_DISABLED
        BDBG_WRN(("MTG disabled"));
#else
        for (i=0;i<BAVC_SourceId_eMax;i++) {
            if (preInitState->boxConfig.stVdc.astSource[i].bMtgCapable) {
                mtg = true;
                break;
            }
        }
#endif

        for (i=0;i<NEXUS_NUM_DISPLAYS;i++) {
            unsigned j;

            if (preInitState->boxMode) {
                if (!preInitState->boxConfig.stVdc.astDisplay[i].bAvailable) {
                    /* default of maxFormat = 0 means unused */
                    continue;
                }
                pSettings->display[i].maxFormat = NEXUS_P_VideoFormat_FromMagnum_isrsafe(preInitState->boxConfig.stVdc.astDisplay[i].eMaxVideoFmt);
            }
            else {
                switch (i) {
                case 0: pSettings->display[i].maxFormat = NEXUS_VideoFormat_e1080p; break;
#if NEXUS_NUM_DISPLAYS > 1
                case 1: pSettings->display[i].maxFormat = NEXUS_VideoFormat_ePal; break;
#endif
#if NEXUS_NUM_DISPLAYS > 2
                default: pSettings->display[i].maxFormat = NEXUS_VideoFormat_e720p; break;
#endif
                }
            }
            for (j=0;j<NEXUS_MAX_VIDEO_WINDOWS;j++) {
                if (preInitState->boxMode) {
                    pSettings->display[i].window[j].used = preInitState->boxConfig.stVdc.astDisplay[i].astWindow[j].bAvailable;
                }
                else {
                    pSettings->display[i].window[j].used = (j == 0); /* no PIP */
#if NEXUS_NUM_VIDEO_WINDOWS > 1
                    pSettings->display[i].window[j].used |= (i < 2); /* transcode displays have no PIP */
#endif
                }
                if (!pSettings->display[i].window[j].used) continue;
                if (preInitState->boxMode) {
                    if(preInitState->boxConfig.stVdc.astDisplay[i].astWindow[j].stResource.ulMad == BBOX_FTR_DISREGARD)
                        pSettings->display[i].window[j].deinterlacer = structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].eDeinterlacerMode;
                    else if(preInitState->boxConfig.stVdc.astDisplay[i].astWindow[j].stResource.ulMad == BBOX_FTR_INVALID)
                        pSettings->display[i].window[j].deinterlacer = BVDC_DeinterlacerMode_eNone;
                    else
                        pSettings->display[i].window[j].deinterlacer = BVDC_DeinterlacerMode_eBestQuality;
                }
                else
                {
                    pSettings->display[i].window[j].deinterlacer = structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].eDeinterlacerMode;
                }
                pSettings->display[i].window[j].smoothScaling = structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bSmoothScaling;
                pSettings->display[i].window[j].capture =
                    structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bSmoothScaling ||
                    structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bPsfMode ||
                    structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bSideBySide ||
                    structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bBoxDetect ||
                    structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bArbitraryCropping ||
                    structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bIndependentCropping;
                pSettings->display[i].window[j].support3d = i < 2; /* transcode displays have no 3D */
                pSettings->display[i].window[j].convertAnyFrameRate = true;
                pSettings->display[i].window[j].precisionLipSync = i < 2 && j == 0; /* only main window on HD/SD uses precision lipsync */
                pSettings->display[i].window[j].mtg = mtg && !preInitState->boxConfig.stVdc.astDisplay[i].stStgEnc.bAvailable;
            }
        }
    }
#else
    BSTD_UNUSED(i);
#endif

#if NEXUS_HAS_VIDEO_ENCODER
    {
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
        BSTD_UNUSED(preInitState);
        for (i=0;i<NEXUS_NUM_DSP_VIDEO_ENCODERS;i++) {
            pSettings->videoEncoder[i].used = true;
            pSettings->videoEncoder[i].maxWidth = 416;
            pSettings->videoEncoder[i].maxHeight = 224;
            pSettings->videoEncoder[i].interlaced = false;
        }
#else
        if (preInitState->boxMode) {
            unsigned encoderIdx = 0;
            for (i=0;i<NEXUS_NUM_VIDEO_ENCODERS;i++) {
               unsigned device, channel;
               int rc;
               pSettings->videoEncoder[i].used = false;
               rc = nexus_p_encoder_device_and_channel(preInitState, i, &device, &channel);
               if (rc) continue;
               /* collapse enabled encoder mapping table */
               if (preInitState->boxConfig.stVce.stInstance[device].uiChannels & (1<<channel)) {
                   NEXUS_VideoFormatInfo info;
                   NEXUS_VideoFormat_GetInfo_isrsafe(NEXUS_P_VideoFormat_FromMagnum_isrsafe(preInitState->boxConfig.stVce.stInstance[device].eVideoFormat), &info);
                   pSettings->videoEncoder[encoderIdx].used = true;
                   pSettings->videoEncoder[encoderIdx].maxWidth = NEXUS_P_SizeAlign( info.width, preInitState->boxConfig.stVce.stInstance[device].uiPixelAlignment );
                   pSettings->videoEncoder[encoderIdx].maxHeight = NEXUS_P_SizeAlign( info.height, preInitState->boxConfig.stVce.stInstance[device].uiPixelAlignment );
                   NEXUS_P_FrameRate_FromRefreshRate_isrsafe(info.verticalFreq * 10, &pSettings->videoEncoder[encoderIdx].maxFrameRate);
                   pSettings->videoEncoder[encoderIdx].interlaced = ( ( pSettings->videoEncoder[encoderIdx].maxHeight >= 1080 ) && ( pSettings->videoEncoder[encoderIdx].maxHeight <= 1088) ) ? true : false;
                   encoderIdx++;
               }
           }
        }
        else {
           BVCE_Channel_GetDefaultMemoryBoundsSettings(preInitState->hBox, &structs->vce.channelSettings); /* not per channel */
            for (i=0;i<NEXUS_NUM_VIDEO_ENCODERS;i++) {
               pSettings->videoEncoder[i].used = true;
               pSettings->videoEncoder[i].maxWidth = structs->vce.channelSettings.stDimensions.stMax.uiWidth;
               pSettings->videoEncoder[i].maxHeight = structs->vce.channelSettings.stDimensions.stMax.uiHeight;
               pSettings->videoEncoder[i].maxFrameRate = NEXUS_VideoFrameRate_e60;
               pSettings->videoEncoder[i].interlaced = (structs->vce.channelSettings.eInputType == BAVC_ScanType_eInterlaced);
           }
       }
#endif
    }
#else
    BSTD_UNUSED(preInitState);
#endif

#if NEXUS_HAS_AUDIO
    NEXUS_AudioModule_GetDefaultUsageSettings(&pSettings->audio);
#endif

#if NEXUS_HAS_HDMI_INPUT
    pSettings->videoInputs.hdDvi = true;
#endif

    NEXUS_Platform_P_SetSpecificOps(&g_platformSpecificOps);
    if (g_platformSpecificOps.modifyDefaultMemoryConfigurationSettings) {
        (g_platformSpecificOps.modifyDefaultMemoryConfigurationSettings)(pSettings);
    }

#if NEXUS_HAS_VIDEO_DECODER && NEXUS_NUM_STILL_DECODES
    /* after codecs have been set, update still decoder based on first (highest capability) decoder */
    BKNI_Memcpy(pSettings->stillDecoder[0].supportedCodecs, pSettings->videoDecoder[0].supportedCodecs, sizeof(pSettings->stillDecoder[0].supportedCodecs));
    pSettings->stillDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Svc] = false;
    pSettings->stillDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = false;
#endif

    BKNI_Free(structs);
    return;
}

#if NEXUS_HAS_VIDEO_DECODER
void NEXUS_P_SupportVideoDecoderCodec( NEXUS_MemoryConfigurationSettings *pSettings, NEXUS_VideoCodec codec )
{
    unsigned i;
    for (i=0; i<NEXUS_NUM_VIDEO_DECODERS; i++) {
        pSettings->videoDecoder[i].supportedCodecs[codec] = true;
    }
}
#endif

void NEXUS_P_GetDefaultMemoryRtsSettings(const NEXUS_Core_PreInitState *preInitState, NEXUS_MemoryRtsSettings *pRtsSettings)
{
    unsigned i;

    BKNI_Memset(pRtsSettings, 0, sizeof(*pRtsSettings));
    BSTD_UNUSED(i);
    pRtsSettings->boxMode = preInitState->boxMode;
#if NEXUS_HAS_VIDEO_DECODER
    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++) {
        unsigned device, channel;
        if (preInitState->boxMode) {
            if (!nexus_p_xvd_device_channel(preInitState, i, &device, &channel)) {
                pRtsSettings->videoDecoder[i].mfdIndex = preInitState->boxConfig.stXvd.stInstance[device].stDevice.stChannel[channel].mfdIndex;
                pRtsSettings->videoDecoder[i].avdIndex = device;
                pRtsSettings->avd[device].memcIndex = preInitState->boxConfig.stXvd.stInstance[device].stDevice.memcIndex;
                if (preInitState->boxConfig.stXvd.stInstance[device].stDevice.secondaryMemcIndex != BBOX_XVD_UNUSED) {
                    pRtsSettings->avd[device].secondaryMemcIndex = preInitState->boxConfig.stXvd.stInstance[device].stDevice.secondaryMemcIndex;
                    pRtsSettings->avd[device].splitBufferHevc = true;
                }
            }
        }
        else {
            pRtsSettings->videoDecoder[i].mfdIndex = i;
            pRtsSettings->videoDecoder[i].avdIndex = i / 2;
        }
    }
#endif
    if (g_platformSpecificOps.modifyDefaultMemoryRtsSettings) {
        (g_platformSpecificOps.modifyDefaultMemoryRtsSettings)(pRtsSettings);
    }
}

#if NEXUS_HAS_DISPLAY || (NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS)
/* return a driver-mapped heap for a MEMC */
static NEXUS_Error nexus_p_get_driver_heap(const NEXUS_PlatformSettings *pPlatformSettings, unsigned memcIndex, bool allowMemc0DriverHeap, unsigned *pHeap)
{
    switch (memcIndex) {
    case 0:
        if (allowMemc0DriverHeap && pPlatformSettings->heap[NEXUS_MEMC0_DRIVER_HEAP].size) {
            *pHeap = NEXUS_MEMC0_DRIVER_HEAP;
            return NEXUS_SUCCESS;
        }
        *pHeap = NEXUS_MEMC0_MAIN_HEAP;
        return NEXUS_SUCCESS;

    case 1:
        *pHeap = NEXUS_MEMC1_DRIVER_HEAP;
        return NEXUS_SUCCESS;
        break;
    case 2:
        *pHeap = NEXUS_MEMC2_DRIVER_HEAP;
        return NEXUS_SUCCESS;
        break;
    default:
        break;
    }
    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
}
#endif

/* move large structs off the stack */
struct NEXUS_P_GetMemoryConfiguration_structs
{
#if NEXUS_HAS_VIDEO_DECODER
    struct {
        BXVD_ChannelSettings channelSettings;
        BXVD_FWMemConfigSettings settings;
        BXVD_FWMemConfig memConfig;
        BXVD_FWMemConfig mosaicMemConfig;
        BAVC_VideoCompressionStd stVideoCompressionList[BAVC_VideoCompressionStd_eMax];
        bool supportedCodecs[NEXUS_VideoCodec_eMax]; /* copy so we can remove MVC */
    } xvd;
#endif
#if NEXUS_HAS_DISPLAY
    struct {
        BVDC_MemConfigSettings memConfigSettings;
        BVDC_MemConfig memConfig;
    } vdc;
#endif
#if NEXUS_HAS_VIDEO_ENCODER && !NEXUS_NUM_DSP_VIDEO_ENCODERS
    struct {
       BVCE_Channel_MemoryBoundsSettings channelSettings;
       BVCE_Channel_MemorySettings memSettings;
       BVCE_MemoryConfig memConfig;
    } vce;
#endif
    unsigned unused;
};

static void nexus_p_add_display_buffers(NEXUS_DisplayHeapSettings *pHeapSettings, const BVDC_Heap_Settings *pVdcHeapSettings)
{
    pHeapSettings->quadHdBuffers.count += pVdcHeapSettings->ulBufferCnt_4HD;
    pHeapSettings->quadHdBuffers.pipCount += pVdcHeapSettings->ulBufferCnt_4HD_Pip;
    pHeapSettings->fullHdBuffers.count += pVdcHeapSettings->ulBufferCnt_2HD;
    pHeapSettings->fullHdBuffers.pipCount += pVdcHeapSettings->ulBufferCnt_2HD_Pip;
    pHeapSettings->hdBuffers.count += pVdcHeapSettings->ulBufferCnt_HD;
    pHeapSettings->hdBuffers.pipCount += pVdcHeapSettings->ulBufferCnt_HD_Pip;
    pHeapSettings->sdBuffers.count += pVdcHeapSettings->ulBufferCnt_SD;
    pHeapSettings->sdBuffers.pipCount += pVdcHeapSettings->ulBufferCnt_SD_Pip;
}

static unsigned nexus_p_get_window_heap(const struct NEXUS_MemoryLayout *memoryLayout, unsigned memcIndex, enum nexus_memconfig_picbuftype picbuftype, NEXUS_SecureVideo secure)
{
    if ((picbuftype == nexus_memconfig_picbuftype_unsecure && secure != NEXUS_SecureVideo_eSecure) ||
        (picbuftype == nexus_memconfig_picbuftype_secure && secure != NEXUS_SecureVideo_eUnsecure)) {
        return memoryLayout->heapIndex.pictureBuffer[memcIndex][picbuftype];
    }
    else {
        /* not allowed */
        return NEXUS_MAX_HEAPS;
    }
}

static NEXUS_Error NEXUS_P_GetMemoryConfiguration(const NEXUS_Core_PreInitState *preInitState, const NEXUS_MemoryConfigurationSettings *pSettings, const NEXUS_MemoryRtsSettings *pRtsSettings, NEXUS_MemoryConfiguration *pConfig,
    const NEXUS_PlatformSettings *pPlatformSettings)
{
    int rc = 0;
    unsigned i, memcIndex;
    enum nexus_memconfig_picbuftype sec;
    BCHP_MemoryInfo memInfo;
#if NEXUS_HAS_VIDEO_DECODER
    unsigned exclusiveDecoder[NEXUS_MAX_XVD_DEVICES];
#endif
    struct NEXUS_P_GetMemoryConfiguration_structs *structs;
    struct NEXUS_MemoryLayout memoryLayout;
    unsigned num_mosaic_decoders = 0;

    if (0) goto err_getsettings; /* prevent warning */
    BSTD_UNUSED(pRtsSettings);
    BSTD_UNUSED(pSettings);

    NEXUS_P_GetMemoryLayout(&memoryLayout, pPlatformSettings);

    structs = BKNI_Malloc(sizeof(*structs));
    if (!structs) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(pConfig, 0, sizeof(*pConfig));
    BKNI_Memset(&g_NEXUS_platformHandles.estimatedMemory, 0, sizeof(g_NEXUS_platformHandles.estimatedMemory));
#if NEXUS_HAS_VIDEO_DECODER
    pConfig->videoDecoder = pPlatformSettings->videoDecoderModuleSettings;
#endif
#if NEXUS_HAS_DISPLAY
    pConfig->display = pPlatformSettings->displayModuleSettings;
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    NEXUS_VideoEncoderModule_GetDefaultInternalSettings(&pConfig->videoEncoder);
#endif

    BCHP_GetMemoryInfo_PreInit(preInitState->hReg, &memInfo);

#if NEXUS_HAS_VIDEO_DECODER
    BKNI_Memset(&pConfig->videoDecoder.heapSize, 0, sizeof(pConfig->videoDecoder.heapSize));
    BKNI_Memset(pConfig->videoDecoder.avdEnabled, 0, sizeof(pConfig->videoDecoder.avdEnabled));
    BKNI_Memset(pConfig->videoDecoder.supportedCodecs, 0, sizeof(pConfig->videoDecoder.supportedCodecs));

    /* if 4K or triple HD mosaic, this channel is exclusive to all other channels on the device. so only allocate memory for it,
    assuming it is >= the sum of all channels in non-exclusive mode */
    for (i=0;i<NEXUS_MAX_XVD_DEVICES;i++) {
        exclusiveDecoder[i] = NEXUS_NUM_VIDEO_DECODERS;
    }
#if ((BCHP_CHIP == 7268) ||  (BCHP_CHIP == 7271))
    /* 7268/71 box mode 6 upwards allows UHD + 1080p on a single decoder, so need a temporary special case */
    if (preInitState->boxMode < 6)
#endif
    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++) {
        NEXUS_VideoFormatInfo info;
        if (!pSettings->videoDecoder[i].used) continue;
        NEXUS_VideoFormat_GetInfo_isrsafe(pSettings->videoDecoder[i].maxFormat, &info);
        if (NEXUS_VideoDecoder_GetDecodeResolution_priv(info.width, info.height) == BXVD_DecodeResolution_e4K ||
            (pSettings->videoDecoder[i].mosaic.maxNumber >= 3 && pSettings->videoDecoder[i].mosaic.maxHeight > 720))
        {
            unsigned avdIndex = pRtsSettings->videoDecoder[i].avdIndex;
            if (exclusiveDecoder[avdIndex] != NEXUS_NUM_VIDEO_DECODERS) {
                /* two exclusive decoders not supported */
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }
            exclusiveDecoder[avdIndex] = i;
        }
    }

    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS + NEXUS_NUM_STILL_DECODES;i++) {
        unsigned avdIndex;
        NEXUS_VideoFormatInfo info;
        bool still = false;
        unsigned index, secondaryMemcIndex;
        const NEXUS_VideoDecoderMemory *mem;

#if NEXUS_NUM_STILL_DECODES
        if (i >= NEXUS_NUM_VIDEO_DECODERS) {
            index = i - NEXUS_NUM_VIDEO_DECODERS;
            still = true;
            mem = &pSettings->stillDecoder[index];
            if (!mem->used) {
                pConfig->videoDecoder.stillMemory[index].used = false;
                continue;
            }
            avdIndex = index; /* assumption based on one StillDecoder per AVD */
        }
        else
#endif
        {
            index = i;
            mem = &pSettings->videoDecoder[index];
            if (!mem->used) {
                pConfig->videoDecoder.avdMapping[index] = NEXUS_MAX_XVD_DEVICES; /* unused */
                pConfig->videoDecoder.memory[index].used = false;
                continue;
            }
            avdIndex = pRtsSettings->videoDecoder[index].avdIndex;
            if (mem->mosaic.maxNumber) {
                num_mosaic_decoders++;
            }
        }

        (void)BXVD_GetChannelDefaultSettings(NULL, index, &structs->xvd.channelSettings);

        memcIndex = pRtsSettings->avd[avdIndex].memcIndex;
        if (memcIndex >= NEXUS_NUM_MEMC) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        secondaryMemcIndex = pRtsSettings->avd[avdIndex].secondaryMemcIndex;
        if (secondaryMemcIndex >= NEXUS_NUM_MEMC) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

        BXVD_GetFWMemConfigDefaultSettings(&structs->xvd.settings);
        structs->xvd.settings.uiAVDInstance = avdIndex;
        structs->xvd.settings.MemcIndex = memcIndex;
        structs->xvd.settings.MemcIndexExtended = secondaryMemcIndex;
        structs->xvd.settings.pInfo = &memInfo;

        BKNI_Memcpy(structs->xvd.supportedCodecs, mem->supportedCodecs, sizeof(structs->xvd.supportedCodecs));
        NEXUS_VideoFormat_GetInfo_isrsafe(mem->maxFormat, &info);
        structs->xvd.channelSettings.bAVC51Enable = mem->avc51Supported;
        structs->xvd.channelSettings.eChannelMode = still ? BXVD_ChannelMode_eStill : BXVD_ChannelMode_eVideo;
        structs->xvd.channelSettings.b10BitBuffersEnable = mem->colorDepth >= 10;
        structs->xvd.channelSettings.uiExtraPictureMemoryAtoms = mem->extraPictureBuffers;
        structs->xvd.channelSettings.peVideoCmprStdList = structs->xvd.stVideoCompressionList;
        NEXUS_VideoDecoder_SetVideoCmprStdList_priv(structs->xvd.supportedCodecs, &structs->xvd.channelSettings, BAVC_VideoCompressionStd_eMax);
        structs->xvd.channelSettings.eDecodeResolution = NEXUS_VideoDecoder_GetDecodeResolution_priv(info.width, info.height);
        if (structs->xvd.supportedCodecs[NEXUS_VideoCodec_eH265] && pRtsSettings->avd[avdIndex].splitBufferHevc) {
            structs->xvd.channelSettings.bSplitPictureBuffersEnable = true;
        }
        rc = BXVD_GetChannelMemoryParameters(&structs->xvd.channelSettings, &structs->xvd.settings, &structs->xvd.memConfig);
        if (rc) {rc = BERR_TRACE(rc); goto err_getsettings;}

        if (!still && mem->mosaic.maxNumber) {
            unsigned general, secure, picture, secondaryPicture;

            /* remove MVC for mosaic */
            structs->xvd.supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = false;
            NEXUS_VideoDecoder_SetVideoCmprStdList_priv(structs->xvd.supportedCodecs, &structs->xvd.channelSettings, BAVC_VideoCompressionStd_eMax);

            /* sum all mosaics and see if it's greater than single decode */
            structs->xvd.channelSettings.eDecodeResolution = NEXUS_VideoDecoder_GetDecodeResolution_priv(mem->mosaic.maxWidth, mem->mosaic.maxHeight);
            if (structs->xvd.channelSettings.eDecodeResolution == BXVD_DecodeResolution_e4K) {
                rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto err_getsettings;
            }
            structs->xvd.channelSettings.b10BitBuffersEnable = mem->mosaic.colorDepth >= 10;
            rc = BXVD_GetChannelMemoryParameters(&structs->xvd.channelSettings, &structs->xvd.settings, &structs->xvd.mosaicMemConfig);
            if (rc) {rc = BERR_TRACE(rc); goto err_getsettings;}
            general = structs->xvd.mosaicMemConfig.uiGeneralHeapSize * mem->mosaic.maxNumber;
            secure = structs->xvd.mosaicMemConfig.uiCabacHeapSize * mem->mosaic.maxNumber;
            picture = structs->xvd.mosaicMemConfig.uiPictureHeapSize * mem->mosaic.maxNumber;
            secondaryPicture = structs->xvd.mosaicMemConfig.uiPictureHeap1Size * mem->mosaic.maxNumber;

            if (general > structs->xvd.memConfig.uiGeneralHeapSize) {
                structs->xvd.memConfig.uiGeneralHeapSize = general;
            }
            if (secure > structs->xvd.memConfig.uiCabacHeapSize) {
                structs->xvd.memConfig.uiCabacHeapSize = secure;
            }
            if (picture > structs->xvd.memConfig.uiPictureHeapSize) {
                structs->xvd.memConfig.uiPictureHeapSize = picture;
            }
            if (secondaryPicture > structs->xvd.memConfig.uiPictureHeap1Size) {
                structs->xvd.memConfig.uiPictureHeap1Size = secondaryPicture;
            }
        }

        if (exclusiveDecoder[avdIndex] == NEXUS_NUM_VIDEO_DECODERS || exclusiveDecoder[avdIndex] == index) {
            for (sec=0;sec<nexus_memconfig_picbuftype_max;sec++) {
                const char *name;
                if (!nexus_p_usepicbuf(mem->secure, sec)) {
                    continue;
                }
                name = (sec==nexus_memconfig_picbuftype_unsecure)?"picture":"secure picture",
                pConfig->pictureBuffer[memcIndex][sec].size += structs->xvd.memConfig.uiPictureHeapSize;
                BDBG_MSG(("XVD MEMC%u HVD%u idx%u: %s %u", memcIndex, avdIndex, i, name, structs->xvd.memConfig.uiPictureHeapSize));
                if (structs->xvd.memConfig.uiPictureHeap1Size) {
                    pConfig->pictureBuffer[secondaryMemcIndex][sec].size += structs->xvd.memConfig.uiPictureHeap1Size;
                    BDBG_MSG(("XVD MEMC%u HVD%u idx%u: secondary %s %u", secondaryMemcIndex, avdIndex, i, name, structs->xvd.memConfig.uiPictureHeap1Size));
                }
            }

            pConfig->videoDecoder.heapSize[avdIndex].general += structs->xvd.memConfig.uiGeneralHeapSize;
            g_NEXUS_platformHandles.estimatedMemory.memc[memcIndex].videoDecoder.general += structs->xvd.memConfig.uiGeneralHeapSize;
            pConfig->videoDecoder.heapSize[avdIndex].secure += structs->xvd.memConfig.uiCabacHeapSize;
            g_NEXUS_platformHandles.estimatedMemory.memc[memcIndex].videoDecoder.secure += structs->xvd.memConfig.uiCabacHeapSize;

            if (!mem->dynamicPictureBuffers &&
                pConfig->pictureBuffer[memcIndex][nexus_memconfig_picbuftype_unsecure].size) {
                pConfig->videoDecoder.heapSize[avdIndex].picture += structs->xvd.memConfig.uiPictureHeapSize;
                pConfig->videoDecoder.heapSize[avdIndex].secondaryPicture += structs->xvd.memConfig.uiPictureHeap1Size;
            }
            else {
                /* use special value of 1 to communicate if RTS requires split buffer for this HVD, even though to we runtime allocation */
                pConfig->videoDecoder.heapSize[avdIndex].secondaryPicture = structs->xvd.memConfig.uiPictureHeap1Size ? 1 : 0;
            }
        }
#if NEXUS_NUM_STILL_DECODES
        if (still) {
            BKNI_Memcpy(&pConfig->videoDecoder.stillMemory[index], mem, sizeof(pConfig->videoDecoder.stillMemory[index]));
        }
        else
#endif
        {
            pConfig->videoDecoder.avdEnabled[avdIndex] = true;
            pConfig->videoDecoder.avdMapping[index] = avdIndex;
            pConfig->videoDecoder.mfdMapping[index] = pRtsSettings->videoDecoder[index].mfdIndex;
            pConfig->videoDecoder.avdHeapIndex[avdIndex] = memoryLayout.heapIndex.pictureBuffer[memcIndex][nexus_memconfig_picbuftype_unsecure];
            pConfig->videoDecoder.secure.avdHeapIndex[avdIndex] = memoryLayout.heapIndex.pictureBuffer[memcIndex][nexus_memconfig_picbuftype_secure];
            pConfig->videoDecoder.secondaryPictureHeapIndex[avdIndex] = memoryLayout.heapIndex.pictureBuffer[pRtsSettings->avd[avdIndex].secondaryMemcIndex][nexus_memconfig_picbuftype_unsecure];
            pConfig->videoDecoder.secure.secondaryPictureHeapIndex[avdIndex] = memoryLayout.heapIndex.pictureBuffer[pRtsSettings->avd[avdIndex].secondaryMemcIndex][nexus_memconfig_picbuftype_secure];
            BKNI_Memcpy(&pConfig->videoDecoder.memory[index], mem, sizeof(pConfig->videoDecoder.memory[index]));
        }
    }

    pConfig->videoDecoder.hostAccessibleHeapIndex = NEXUS_MEMC0_MAIN_HEAP;
#endif

#if NEXUS_HAS_DISPLAY
    {
        for (i=0;i<NEXUS_MAX_HEAPS;i++) {
            NEXUS_DisplayHeapSettings *pHeapSettings = &pConfig->display.displayHeapSettings[i];
            pHeapSettings->quadHdBuffers.count = 0;
            pHeapSettings->quadHdBuffers.pipCount = 0;
            pHeapSettings->fullHdBuffers.count = 0;
            pHeapSettings->fullHdBuffers.pipCount = 0;
            pHeapSettings->hdBuffers.count = 0;
            pHeapSettings->hdBuffers.pipCount = 0;
            pHeapSettings->sdBuffers.count = 0;
            pHeapSettings->sdBuffers.pipCount = 0;
        }

        BDBG_CASSERT(BVDC_MAX_DISPLAYS >= NEXUS_MAX_DISPLAYS);
        BDBG_CASSERT(BVDC_MAX_VIDEO_WINDOWS >= NEXUS_MAX_VIDEO_WINDOWS);
        BVDC_GetDefaultMemConfigSettings(&structs->vdc.memConfigSettings);
        for (i=0;i<BVDC_MAX_DISPLAYS;i++) {
            unsigned j;
            if (i >= NEXUS_NUM_DISPLAYS || !pSettings->display[i].maxFormat) continue;
            for (j=0;j<BVDC_MAX_VIDEO_WINDOWS;j++) {
                bool bPip = j>0;
                if (preInitState->boxMode) {
                    if((preInitState->boxConfig.stVdc.astDisplay[i].astWindow[j].stSizeLimits.ulHeightFraction == 1) ||
                       (preInitState->boxConfig.stVdc.astDisplay[i].astWindow[j].stSizeLimits.ulWidthFraction == 1))
                       bPip = false;
                }
                if (j>=NEXUS_MAX_VIDEO_WINDOWS || !pSettings->display[i].window[j].used) continue;

                structs->vdc.memConfigSettings.stDisplay[i].bUsed = true;
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bUsed = true;

                rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(pSettings->display[i].maxFormat, &structs->vdc.memConfigSettings.stDisplay[i].eMaxDisplayFormat);
                if (rc) {rc = BERR_TRACE(rc); goto err_getsettings;}

                memcIndex = preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j];
                if (memcIndex >= BBOX_MemcIndex_Invalid || memcIndex >= NEXUS_MAX_MEMC) { rc = BERR_TRACE(NEXUS_INVALID_PARAMETER); goto err_getsettings;}

                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].ulMemcIndex = memcIndex;
                memcIndex = preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[i].aulVidWinMadMemcIndex[j];
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].ulMadMemcIndex = memcIndex;
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].b5060Convert = pSettings->display[i].window[j].convertAnyFrameRate;
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bSlave_24_25_30_Display =
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bLipsync = pSettings->display[i].window[j].precisionLipSync;
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bSlave_24_25_30_Display &= (1 == i); /* only display 1 */
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bPip = bPip;
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].eDeinterlacerMode = pSettings->display[i].window[j].deinterlacer;
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bSmoothScaling = pSettings->display[i].window[j].capture || pSettings->display[i].window[j].smoothScaling;
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].b3DMode = pSettings->display[i].window[j].support3d;
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bPsfMode =
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bSideBySide =
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bBoxDetect =
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bArbitraryCropping =
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bIndependentCropping = pSettings->display[i].window[j].capture;
                if (num_mosaic_decoders == 0) {
                    structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bMosaicMode = false;
                }
                if (pSettings->videoInputs.hdDvi || pSettings->videoInputs.ccir656) {
                    /* Only set true for non-mfd; never set false. VDC may default to true for other reasons. */
                    structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bSyncSlip = true;
                }
                else {
                    bool stg = false;
                    if (preInitState->boxMode) {
                        stg = preInitState->boxConfig.stVdc.astDisplay[i].stStgEnc.bAvailable;
                    }
                    if (((i == 0) && (j==0)) || stg)
                        structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bSyncSlip = false;
                }
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].bNonMfdSource = pSettings->videoInputs.hdDvi || pSettings->videoInputs.ccir656 || pSettings->display[i].window[j].mtg;
                structs->vdc.memConfigSettings.stDisplay[i].stWindow[j].ulAdditionalBufCnt = pSettings->display[i].window[j].userCaptureBufferCount;

                structs->vdc.memConfigSettings.stRdc.ulMemcIndex = preInitState->boxConfig.stMemConfig.stVdcMemcIndex.ulRdcMemcIndex;
            }
        }

        rc = BVDC_GetMemoryConfiguration(&structs->vdc.memConfigSettings, &structs->vdc.memConfig);
        if (rc) {rc = BERR_TRACE(rc); goto err_getsettings;}

        BDBG_CASSERT(NEXUS_MAX_MEMC <= BVDC_MAX_MEMC);
        for (memcIndex=0;memcIndex<NEXUS_MAX_MEMC;memcIndex++) {
            /* capture RDC/CFC heap use */
            g_NEXUS_platformHandles.estimatedMemory.memc[memcIndex].display.general += structs->vdc.memConfig.stMemc[memcIndex].ulRulSize;
            g_NEXUS_platformHandles.estimatedMemory.memc[memcIndex].display.general += structs->vdc.memConfig.stMemc[memcIndex].ulCfcLutSize;
        }

        for (i=0;i<BVDC_MAX_DISPLAYS;i++) {
            unsigned j;
            if (i >= NEXUS_NUM_DISPLAYS || !pSettings->display[i].maxFormat) continue;
            for (j=0;j<BVDC_MAX_VIDEO_WINDOWS;j++) {
                if (j>=NEXUS_MAX_VIDEO_WINDOWS || !pSettings->display[i].window[j].used) continue;
                for (sec=0;sec<nexus_memconfig_picbuftype_max;sec++) {
                    unsigned heapIndex;
                    const char *name = (sec==nexus_memconfig_picbuftype_unsecure)?"picture":"secure picture";

                    if (!nexus_p_usepicbuf(pSettings->display[i].window[j].secure, sec)) continue;

                    /* CAP */
                    if (structs->vdc.memConfig.stDisplay[i].stWindow[j].ulCapSize) {
                        memcIndex = preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j];
                        BDBG_ASSERT(memcIndex < NEXUS_MAX_MEMC);
                        heapIndex = memoryLayout.heapIndex.pictureBuffer[memcIndex][sec];
                        BDBG_ASSERT(heapIndex < NEXUS_MAX_HEAPS);
                        pConfig->pictureBuffer[memcIndex][sec].size += structs->vdc.memConfig.stDisplay[i].stWindow[j].ulCapSize;
                        BDBG_MSG(("VDC MEMC%d CMP%u V%u CAP: %s %d", memcIndex, i, j, name, structs->vdc.memConfig.stDisplay[i].stWindow[j].ulCapSize));
                        nexus_p_add_display_buffers(&pConfig->display.displayHeapSettings[heapIndex], &structs->vdc.memConfig.stDisplay[i].stWindow[j].stCapHeapSettings);
                    }

                    /* MAD */
                    if (structs->vdc.memConfig.stDisplay[i].stWindow[j].ulMadSize) {
                        memcIndex = preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[i].aulVidWinMadMemcIndex[j];
                        BDBG_ASSERT(memcIndex < NEXUS_MAX_MEMC);
                        heapIndex = memoryLayout.heapIndex.pictureBuffer[memcIndex][sec];
                        BDBG_ASSERT(heapIndex < NEXUS_MAX_HEAPS);
                        pConfig->pictureBuffer[memcIndex][sec].size += structs->vdc.memConfig.stDisplay[i].stWindow[j].ulMadSize;
                        BDBG_MSG(("VDC MEMC%d CMP%u V%u MAD: %s %d", memcIndex, i, j, name, structs->vdc.memConfig.stDisplay[i].stWindow[j].ulMadSize));
                        nexus_p_add_display_buffers(&pConfig->display.displayHeapSettings[heapIndex], &structs->vdc.memConfig.stDisplay[i].stWindow[j].stMadHeapSettings);
                    }
                }
            }
        }

        NEXUS_Display_P_SetVdcMemConfig(&structs->vdc.memConfigSettings);
    }

    nexus_p_get_driver_heap(pPlatformSettings, preInitState->boxConfig.stMemConfig.stVdcMemcIndex.ulRdcMemcIndex, true, &pConfig->display.rdcHeapIndex);
    for (i=0;i<BBOX_VDC_HDMI_DISPLAY_COUNT;i++) {
        if(BBOX_MemcIndex_Invalid != preInitState->boxConfig.stMemConfig.stVdcMemcIndex.aulHdmiDisplayCfcMemcIndex[i]) {
            BDBG_CASSERT(BBOX_VDC_HDMI_DISPLAY_COUNT==NEXUS_MAX_HDMI_OUTPUTS);
            nexus_p_get_driver_heap(pPlatformSettings, preInitState->boxConfig.stMemConfig.stVdcMemcIndex.aulHdmiDisplayCfcMemcIndex[i], true, &pConfig->display.cfc.vecHeapIndex[i]);
        }
    }
    for (i=0;i<BBOX_VDC_DISPLAY_COUNT;i++) {
        BDBG_CASSERT(BBOX_VDC_DISPLAY_COUNT==NEXUS_MAX_DISPLAYS);
        if(BBOX_MemcIndex_Invalid != preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[i].ulCmpCfcMemcIndex) {
            nexus_p_get_driver_heap(pPlatformSettings, preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[i].ulCmpCfcMemcIndex, true, &pConfig->display.cfc.cmpHeapIndex[i]);
        }
        if(BBOX_MemcIndex_Invalid != preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[i].ulGfdCfcMemcIndex) {
            nexus_p_get_driver_heap(pPlatformSettings, preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[i].ulGfdCfcMemcIndex, true, &pConfig->display.cfc.gfdHeapIndex[i]);
        }
    }

#else
    BSTD_UNUSED(i);
    BSTD_UNUSED(num_mosaic_decoders);
#endif

#if NEXUS_HAS_VIDEO_ENCODER
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
    {
       for (i=0;i<NEXUS_NUM_VIDEO_ENCODERS;i++) {
           pConfig->videoEncoder.videoEncoder[i].memory = pSettings->videoEncoder[i];
       }
    }
#else
    {
       bool deviceMemory[BBOX_VCE_MAX_INSTANCE_COUNT];
       BKNI_Memset(deviceMemory, 0, sizeof(deviceMemory));
       BKNI_Memset(pConfig->videoEncoder.heapSize, 0, sizeof(pConfig->videoEncoder.heapSize));

       for (i=0;i<NEXUS_MAX_VIDEO_ENCODERS;i++) {
           pConfig->videoEncoder.vceMapping[i].device = -1;
           pConfig->videoEncoder.vceMapping[i].channel = -1;
       }

       for (i=0;i<NEXUS_NUM_VIDEO_ENCODERS;i++) {
           unsigned vceIndex, channel;
           unsigned memcIndex, mainMemcIndex;
           struct {
              unsigned firmware, output, secure, system;
           } heap = {0,0,0,0};

            if (!pSettings->videoEncoder[i].used) continue;
            rc = nexus_p_encoder_device_and_channel(preInitState, i, &vceIndex, &channel);
            if (rc) continue;

            if (vceIndex >= BBOX_VCE_MAX_INSTANCE_COUNT) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            mainMemcIndex = preInitState->boxConfig.stVce.stInstance[vceIndex].uiMemcIndex;
            if (mainMemcIndex >= NEXUS_NUM_MEMC) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
            nexus_p_get_driver_heap(pPlatformSettings, 0, true, &heap.output);
#if NEXUS_HAS_SAGE
#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
            /* must have compile time definition and run time use */
            if (pPlatformSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size) {
                heap.secure = NEXUS_VIDEO_SECURE_HEAP;
            }
            else
#endif
#endif
            {
                nexus_p_get_driver_heap(pPlatformSettings, 0, false, &heap.secure);
            }

            rc = nexus_p_get_driver_heap(pPlatformSettings, mainMemcIndex, false, &heap.firmware);
            if (rc) return BERR_TRACE(rc);
            heap.system = heap.firmware;

           if (!deviceMemory[vceIndex]) {
                /* do per-device memory */
                BVCE_PlatformSettings platformSettings;
                BVCE_MemoryBoundsSettings memoryBoundsSettings;
                BVCE_GetDefaultPlatformSettings(&platformSettings);
                platformSettings.hBox = preInitState->hBox;
                platformSettings.uiInstance = vceIndex;
                BVCE_GetDefaultMemoryBoundsSettings(&platformSettings, &memoryBoundsSettings);
                BVCE_GetMemoryConfig(&platformSettings, &memoryBoundsSettings, &structs->vce.memConfig);

                g_NEXUS_platformHandles.estimatedMemory.memc[mainMemcIndex].videoEncoder.secure += structs->vce.memConfig.uiSecureMemSize;
                memcIndex = pPlatformSettings->heap[heap.system].memcIndex;
                g_NEXUS_platformHandles.estimatedMemory.memc[memcIndex].videoEncoder.general += structs->vce.memConfig.uiGeneralMemSize;
                memcIndex = pPlatformSettings->heap[heap.firmware].memcIndex;
                g_NEXUS_platformHandles.estimatedMemory.memc[memcIndex].videoEncoder.firmware += structs->vce.memConfig.uiFirmwareMemSize;
                memcIndex = pPlatformSettings->heap[heap.output].memcIndex;
                g_NEXUS_platformHandles.estimatedMemory.memc[memcIndex].videoEncoder.index += structs->vce.memConfig.uiIndexMemSize;
                g_NEXUS_platformHandles.estimatedMemory.memc[memcIndex].videoEncoder.data += structs->vce.memConfig.uiDataMemSize;

                pConfig->videoEncoder.heapSize[vceIndex].general += structs->vce.memConfig.uiGeneralMemSize;
                pConfig->videoEncoder.heapSize[vceIndex].picture += structs->vce.memConfig.uiPictureMemSize;
                pConfig->videoEncoder.heapSize[vceIndex].secure += structs->vce.memConfig.uiSecureMemSize;
                pConfig->videoEncoder.heapSize[vceIndex].firmware += structs->vce.memConfig.uiFirmwareMemSize;
                pConfig->videoEncoder.heapSize[vceIndex].index += structs->vce.memConfig.uiIndexMemSize;
                pConfig->videoEncoder.heapSize[vceIndex].data += structs->vce.memConfig.uiDataMemSize;

                deviceMemory[vceIndex] = true;
           }
           BVCE_Channel_GetDefaultMemoryBoundsSettings(preInitState->hBox, &structs->vce.channelSettings);
           structs->vce.channelSettings.stDimensions.stMax.uiWidth = pSettings->videoEncoder[i].maxWidth;
           structs->vce.channelSettings.stDimensions.stMax.uiHeight = pSettings->videoEncoder[i].maxHeight;
           structs->vce.channelSettings.eInputType = (pSettings->videoEncoder[i].interlaced? BAVC_ScanType_eInterlaced : BAVC_ScanType_eProgressive);

           structs->vce.memSettings.uiInstance = vceIndex;
           structs->vce.memSettings.memcIndex.uiPicture = mainMemcIndex;
           structs->vce.memSettings.memcIndex.uiSecure = mainMemcIndex;
           structs->vce.memSettings.pstMemoryInfo = &memInfo;
           BVCE_Channel_GetMemoryConfig(preInitState->hBox, &structs->vce.memSettings, &structs->vce.channelSettings, &structs->vce.memConfig);

           pConfig->pictureBuffer[mainMemcIndex][nexus_memconfig_picbuftype_unsecure].size += structs->vce.memConfig.uiPictureMemSize;
           pConfig->videoEncoder.heapSize[vceIndex].general += structs->vce.memConfig.uiGeneralMemSize;
           pConfig->videoEncoder.heapSize[vceIndex].picture += structs->vce.memConfig.uiPictureMemSize;
           pConfig->videoEncoder.heapSize[vceIndex].secure += structs->vce.memConfig.uiSecureMemSize;
           pConfig->videoEncoder.heapSize[vceIndex].firmware += structs->vce.memConfig.uiFirmwareMemSize;
           pConfig->videoEncoder.heapSize[vceIndex].index += structs->vce.memConfig.uiIndexMemSize;
           pConfig->videoEncoder.heapSize[vceIndex].data += structs->vce.memConfig.uiDataMemSize;
           g_NEXUS_platformHandles.estimatedMemory.memc[mainMemcIndex].videoEncoder.secure += structs->vce.memConfig.uiSecureMemSize;

           /* coverity[CONSTANT_EXPRESSION_RESULT: FALSE] */
           BDBG_MSG(("VCE MEMC%d VCE%u idx%u %ux%u%c, picture "BDBG_UINT64_FMT", cabac "BDBG_UINT64_FMT, mainMemcIndex, vceIndex, i,
            structs->vce.channelSettings.stDimensions.stMax.uiWidth, structs->vce.channelSettings.stDimensions.stMax.uiHeight, pSettings->videoEncoder[i].interlaced? 'i' : 'p',
            BDBG_UINT64_ARG((uint64_t)structs->vce.memConfig.uiPictureMemSize), BDBG_UINT64_ARG((uint64_t)structs->vce.memConfig.uiSecureMemSize)));

           memcIndex = pPlatformSettings->heap[heap.system].memcIndex;
           g_NEXUS_platformHandles.estimatedMemory.memc[memcIndex].videoEncoder.general += structs->vce.memConfig.uiGeneralMemSize;
           memcIndex = pPlatformSettings->heap[heap.firmware].memcIndex;
           g_NEXUS_platformHandles.estimatedMemory.memc[memcIndex].videoEncoder.firmware += structs->vce.memConfig.uiFirmwareMemSize;
           memcIndex = pPlatformSettings->heap[heap.output].memcIndex;
           g_NEXUS_platformHandles.estimatedMemory.memc[memcIndex].videoEncoder.index += structs->vce.memConfig.uiIndexMemSize;
           g_NEXUS_platformHandles.estimatedMemory.memc[memcIndex].videoEncoder.data += structs->vce.memConfig.uiDataMemSize;

           pConfig->videoEncoder.vceMapping[i].device = vceIndex;
           pConfig->videoEncoder.vceMapping[i].channel = channel;
           pConfig->videoEncoder.videoEncoder[i].memory = pSettings->videoEncoder[i];
           pConfig->videoEncoder.heapIndex[vceIndex].firmware[0] =
           pConfig->videoEncoder.heapIndex[vceIndex].firmware[1] = heap.firmware;
           pConfig->videoEncoder.heapIndex[vceIndex].output = heap.output;
           pConfig->videoEncoder.heapIndex[vceIndex].secure  = heap.secure;
           pConfig->videoEncoder.heapIndex[vceIndex].system  = heap.system;
           pConfig->videoEncoder.heapIndex[vceIndex].picture = memoryLayout.heapIndex.pictureBuffer[mainMemcIndex][nexus_memconfig_picbuftype_unsecure];
       }
    }
#endif
#else
    BSTD_UNUSED(pPlatformSettings);
#endif

#if NEXUS_HAS_DISPLAY
    for (i=0;i<NEXUS_NUM_DISPLAYS;i++) {
        unsigned j;
        pConfig->display.memConfig[i] = pSettings->display[i];
        for (j=0;j<NEXUS_MAX_VIDEO_WINDOWS;j++) {
            /* convert per-window memc to heap index */
            unsigned memcIndex;
            if (pSettings->display[i].maxFormat > 0 && pSettings->display[i].window[j].used) {
                memcIndex = preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j];
                pConfig->display.videoWindowHeapIndex[i][j] = nexus_p_get_window_heap(&memoryLayout, memcIndex, nexus_memconfig_picbuftype_unsecure, pSettings->display[i].window[j].secure);
                pConfig->display.secure.videoWindowHeapIndex[i][j] = nexus_p_get_window_heap(&memoryLayout, memcIndex, nexus_memconfig_picbuftype_secure, pSettings->display[i].window[j].secure);
            }
            memcIndex = preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[i].aulVidWinMadMemcIndex[j];
            if (memcIndex != BBOX_MemcIndex_Invalid) {
                pConfig->display.deinterlacerHeapIndex[i][j] = nexus_p_get_window_heap(&memoryLayout, memcIndex, nexus_memconfig_picbuftype_unsecure, pSettings->display[i].window[j].secure);
                pConfig->display.secure.deinterlacerHeapIndex[i][j] = nexus_p_get_window_heap(&memoryLayout, memcIndex, nexus_memconfig_picbuftype_secure, pSettings->display[i].window[j].secure);
            }
        }
    }
#endif
#if NEXUS_HAS_VIDEO_DECODER && NEXUS_HAS_DISPLAY
    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++) { /* i is MFD index */
        unsigned j;
        for (j=0;j<NEXUS_NUM_VIDEO_DECODERS;j++) {
            if (pRtsSettings->videoDecoder[j].mfdIndex == i) break;
        }
        if (j<NEXUS_NUM_VIDEO_DECODERS) {
            pConfig->display.videoImageInput[i].mfd.memcIndex = pRtsSettings->avd[pRtsSettings->videoDecoder[j].avdIndex].memcIndex;
            pConfig->display.videoImageInput[i].mfd.secondaryMemcIndex = pRtsSettings->avd[pRtsSettings->videoDecoder[j].avdIndex].splitBufferHevc?
                pRtsSettings->avd[pRtsSettings->videoDecoder[j].avdIndex].secondaryMemcIndex : pRtsSettings->avd[pRtsSettings->videoDecoder[j].avdIndex].memcIndex;
            /* TODO: vfd.memcIndex */
        }
    }
#endif

#if NEXUS_HAS_AUDIO
    {
        NEXUS_AudioModuleMemoryEstimate audioEstimate;
        rc = NEXUS_AudioModule_GetMemoryEstimate(preInitState, &pSettings->audio, &audioEstimate);
        if (!rc) {
            for (i=0;i<NEXUS_MAX_MEMC;i++) {
                g_NEXUS_platformHandles.estimatedMemory.memc[i].audio.general = audioEstimate.memc[i].general;
            }
        }
    }
#endif

    for (memcIndex=0;memcIndex<NEXUS_MAX_MEMC;memcIndex++) {
        for (sec=0;sec<nexus_memconfig_picbuftype_max;sec++) {
            pConfig->pictureBuffer[memcIndex][sec].heapIndex = memoryLayout.heapIndex.pictureBuffer[memcIndex][sec];
            if (pConfig->pictureBuffer[memcIndex][sec].heapIndex == NEXUS_MAX_HEAPS) continue;
            if (pConfig->pictureBuffer[memcIndex][sec].size == 0) continue;
            pConfig->pictureBuffer[memcIndex][sec].size += 0x2000; /* add 2 more pages for front&back guardband overhead in case of page alignment */
            /* size must be increased to be page aligned */
            if (pConfig->pictureBuffer[memcIndex][sec].size & 0xfff) {
                pConfig->pictureBuffer[memcIndex][sec].size = (pConfig->pictureBuffer[memcIndex][sec].size + 0xfff) & ~0xfff;
            }
        }
    }

err_getsettings:
    BKNI_Free(structs);
    return rc;
}

NEXUS_Error NEXUS_P_ApplyMemoryConfiguration(const NEXUS_Core_PreInitState *preInitState, const NEXUS_MemoryConfigurationSettings *pMemConfig, const NEXUS_MemoryRtsSettings *pRtsSettings, NEXUS_PlatformSettings *pSettings, NEXUS_P_PlatformInternalSettings *pInternalSettings)
{
    unsigned i;
    NEXUS_Error rc;
    NEXUS_MemoryConfiguration *pConfig;

    pConfig = BKNI_Malloc(sizeof(*pConfig));
    if (!pConfig) return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);

    rc = NEXUS_P_GetMemoryConfiguration(preInitState, pMemConfig, pRtsSettings, pConfig, pSettings);
    if (rc) {rc = BERR_TRACE(rc); goto done;}

#if NEXUS_MEMC0_PICTURE_BUFFER_HEAP
    {
    for (i=0;i<NEXUS_MAX_MEMC;i++) {
        enum nexus_memconfig_picbuftype sec;
        for (sec=0;sec<nexus_memconfig_picbuftype_max;sec++) {
            unsigned heapIndex = pConfig->pictureBuffer[i][sec].heapIndex;
            if (heapIndex != NEXUS_MAX_HEAPS) {
                if (pSettings->heap[heapIndex].heapType & NEXUS_HEAP_TYPE_DTU) {
                    continue;
                }
                /* only check when called from NEXUS_Platform_GetDefaultSettings */
                if (pSettings != &g_NEXUS_platformSettings && pSettings->heap[heapIndex].size) {
                    BDBG_ERR(("Overwriting heap[%d].size %d with memconfig value", heapIndex, pSettings->heap[heapIndex].size));
                }
                pSettings->heap[heapIndex].memcIndex = i;
                pSettings->heap[heapIndex].size = pConfig->pictureBuffer[i][sec].size;
                pSettings->heap[heapIndex].heapType |= NEXUS_HEAP_TYPE_PICTURE_BUFFERS;
#if !BMMA_USE_STUB
                pSettings->heap[heapIndex].memoryType |= NEXUS_MEMORY_TYPE_MANAGED;
#if NEXUS_NUM_SOFT_VIDEO_DECODERS
                pSettings->heap[heapIndex].memoryType |= NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED;
#else
                pSettings->heap[heapIndex].memoryType |= NEXUS_MEMORY_TYPE_NOT_MAPPED;
#endif
#endif

#if NEXUS_HAS_SAGE
                if (sec == nexus_memconfig_picbuftype_secure) {
                    pSettings->heap[heapIndex].memoryType |= NEXUS_MEMORY_TYPE_SECURE;
                }
#endif
            }
        }
    }
    }
#else
    BSTD_UNUSED(i);
#endif
#if NEXUS_HAS_VIDEO_DECODER
    pSettings->videoDecoderModuleSettings = pConfig->videoDecoder;
#if  NEXUS_NUM_SOFT_VIDEO_DECODERS
    /* Don't allocate memory for the first decoder at init time */
    pSettings->videoDecoderModuleSettings.heapSize[0].picture = 0;
    pSettings->videoDecoderModuleSettings.heapSize[0].secondaryPicture = 0;
    pSettings->videoDecoderModuleSettings.memory[0].dynamicPictureBuffers = true;
#endif
#endif
#if NEXUS_HAS_DISPLAY
    pSettings->displayModuleSettings = pConfig->display;
    pSettings->displayModuleSettings.memconfig.ccir656 = pMemConfig->videoInputs.ccir656;
    pSettings->displayModuleSettings.memconfig.hdDvi = pMemConfig->videoInputs.hdDvi;
#if NEXUS_HAS_VIDEO_DECODER
    pSettings->displayModuleSettings.memconfig.mosaic = pMemConfig->videoDecoder[0].mosaic.maxNumber > 0;
#endif
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    if(pInternalSettings) {
        pInternalSettings->videoEncoderSettings = pConfig->videoEncoder;
    }
#else
    BSTD_UNUSED(pInternalSettings);
#endif

done:
    BKNI_Free(pConfig);
    return rc;
}

bool nexus_p_has_secure_decoder_on_memc(const NEXUS_Core_PreInitState *preInitState, const NEXUS_MemoryRtsSettings *pRtsSettings, const NEXUS_MemoryConfigurationSettings *pMemConfig, unsigned memcIndex)
{
    unsigned i;
#if NEXUS_HAS_VIDEO_DECODER
    for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
        if (pMemConfig->videoDecoder[i].used && pMemConfig->videoDecoder[i].secure > NEXUS_SecureVideo_eUnsecure) {
            if (pRtsSettings->avd[pRtsSettings->videoDecoder[i].avdIndex].memcIndex == memcIndex) {
                return true;
            }
        }
    }
#else
    BSTD_UNUSED(i);
    BSTD_UNUSED(pRtsSettings);
#endif
#if NEXUS_HAS_DISPLAY
    for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
        unsigned j;
        for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++) {
            if (pMemConfig->display[i].window[j].used && pMemConfig->display[i].window[j].secure > NEXUS_SecureVideo_eUnsecure) {
                if (preInitState->boxConfig.stMemConfig.stVdcMemcIndex.astDisplay[i].aulVidWinCapMemcIndex[j] == memcIndex) {
                    return true;
                }
            }
        }
    }
#else
    BSTD_UNUSED(i);
    BSTD_UNUSED(pMemConfig);
    BSTD_UNUSED(memcIndex);
    BSTD_UNUSED(preInitState);
#endif
    return false;
}
