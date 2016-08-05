/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_video_decoder_private.h"
#include "nexus_power_management.h"
#include "priv/nexus_stc_channel_priv.h"
#include "nexus_transport_capabilities.h"
#include "priv/nexus_pid_channel_priv.h"
#include "priv/nexus_rave_priv.h"
#include "priv/nexus_core.h"
#include "priv/nexus_core_img_id.h"
#include "priv/nexus_core_img.h"

#include "bchp_xpt_rave.h"
#include "bxvd_pvr.h"
#include "bxvd.h"
#include "bxvd_decoder.h"
#include "nexus_surface.h"
#include "bgrc.h"
#include "bgrc_packet.h"

#include "nexus_video_decoder_security.h"

BDBG_MODULE(nexus_video_decoder);
BDBG_FILE_MODULE(nexus_flow_video_decoder);

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

BTRC_MODULE(ChnChange_Tune, ENABLE);
BTRC_MODULE(ChnChange_TuneLock, ENABLE);
BTRC_MODULE(ChnChange_DecodeStopVideo, ENABLE);
BTRC_MODULE(ChnChange_DecodeStartVideo, ENABLE);
BTRC_MODULE(ChnChange_DecodeStartVideoXVD, ENABLE);
BTRC_MODULE(ChnChange_DecodeFirstVideo, ENABLE);
BTRC_MODULE(ChnChange_DecodeFirstvPTSPassed, ENABLE);
BTRC_MODULE(ChnChange_SyncTsmLock, ENABLE); /* TODO: add when sync_channel support is available */
BTRC_MODULE(ChnChange_SyncUnmuteVideo, ENABLE); /* TODO: add when sync_channel support is available */
BTRC_MODULE(ChnChange_Total_Video, ENABLE);

/*
NEXUS_NUM_XVD_DEVICES = # of AVD cores to use. can be overridden in nexus_platform_features.h.
NEXUS_MAX_XVD_DEVICES = # of AVD cores that can be supported by the API. must be >= NEXUS_NUM_XVD_DEVICES.
*/
#ifndef NEXUS_NUM_XVD_DEVICES
#if BCHP_CHIP == 7400 || BCHP_CHIP == 7425 || BCHP_CHIP == 7420 || BCHP_CHIP == 7435
#define NEXUS_NUM_XVD_DEVICES 2
#else
#define NEXUS_NUM_XVD_DEVICES 1
#endif
#endif

/* non-static global module data */
NEXUS_ModuleHandle g_NEXUS_videoDecoderModule = NULL;
NEXUS_VideoDecoderModuleSettings g_NEXUS_videoDecoderModuleSettings;
NEXUS_VideoDecoderModuleInternalSettings g_NEXUS_videoDecoderModuleInternalSettings;
struct NEXUS_VideoDecoderDevice g_NEXUS_videoDecoderXvdDevices[NEXUS_MAX_XVD_DEVICES];

/* static global module data */
static NEXUS_VideoDecoderHandle g_videoDecoders[NEXUS_NUM_VIDEO_DECODERS] = {NULL,}; /* decoder usage */
static NEXUS_VideoDecoderCapabilities g_cap;

static void NEXUS_P_SetVideoDecoderCapabilities(void);

static NEXUS_Error NEXUS_VideoDecoderModule_P_PostInit(void);
static NEXUS_Error NEXUS_VideoDecoder_P_SetSettings(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderSettings *pSettings, bool force);
static NEXUS_Error NEXUS_VideoDecoder_P_InitializeQueue(NEXUS_VideoDecoderHandle videoDecoder);
static NEXUS_Error DisplayManagerLite_isr( void* pPrivateContext, const BXDM_DisplayInterruptInfo *pstDisplayInterruptInfo, BAVC_MVD_Picture **pDispMgrToVDC);
static void NEXUS_VideoDecoder_P_ReturnOutstandingFrames_Avd(NEXUS_VideoDecoderHandle videoDecoder);
static void NEXUS_VideoDecoder_P_StcChannelStatusChangeEventHandler(void *context);

/* Please refer to the XVD memory spreadsheet for more details on these values. */

/* CDB and ITB sizes */
#define B_XVD_VIDEO_CDB_SIZE (3*1024*1024/2) /* 1.5 MB for main video decode CDB */
#define B_XVD_VIDEO_ITB_SIZE (512*1024)      /* 512 KB for main video decode ITB */

/**************************************
* Module functions
**************************************/
void NEXUS_VideoDecoderModule_GetDefaultInternalSettings(NEXUS_VideoDecoderModuleInternalSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

void NEXUS_VideoDecoderModule_GetDefaultSettings(NEXUS_VideoDecoderModuleSettings *pSettings)
{
    unsigned i;
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->watchdogEnabled = true;

    /* default is one heap per memc. if your platform requires a custom setting, that could should be put into the platform. */
    for (i=0;i<NEXUS_NUM_XVD_DEVICES;i++) {
        pSettings->avdHeapIndex[i] = i;
    }

    BDBG_CASSERT(NEXUS_MAX_XVD_DEVICES >= NEXUS_NUM_XVD_DEVICES);

    /* for instance, 7405 has one AVD and two MFD's. each MFD gets a separate BAVC_SourceId for VDC */
    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++) {
        pSettings->mfdMapping[i] = i;
        /* for backward compatible, non-7405 orthorgonal chips have one decode on each core; all future non-orthorgonal chips
           should really override the default in platform code. */
#if NEXUS_NUM_XVD_DEVICES == NEXUS_NUM_VIDEO_DECODERS
        pSettings->avdMapping[i] = i; /* set orthorgonal default */
#endif
        pSettings->memory[i].colorDepth = 8;
    }
#if NEXUS_MAX_VIDEO_DECODERS > NEXUS_NUM_VIDEO_DECODERS
    for (;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
        pSettings->avdMapping[i] = NEXUS_NUM_XVD_DEVICES; /* this means the decoder is unused */
    }
#endif

    for (i=0; i<NEXUS_NUM_XVD_DEVICES; i++) {
        pSettings->avdEnabled[i] = true;
    }

    for (i=0;i<NEXUS_VideoCodec_eMax;i++) {
        switch (i) {
        case NEXUS_VideoCodec_eH264_Mvc:
        case NEXUS_VideoCodec_eH265:
        case NEXUS_VideoCodec_eVp9:
            /* default off because of memory. platform must default on. */
            break;
        case NEXUS_VideoCodec_eNone:
        case NEXUS_VideoCodec_eH264_Svc:
        case NEXUS_VideoCodec_eMotionJpeg:
        case NEXUS_VideoCodec_eVp7:
            /* unsupported */
            break;
        default:
            pSettings->supportedCodecs[i] = true;
        }
    }
    pSettings->maxDecodeFormat = NEXUS_VideoFormat_e1080p30hz;
    pSettings->maxStillDecodeFormat = NEXUS_VideoFormat_e1080p30hz;
    pSettings->numDecodes = NEXUS_NUM_VIDEO_DECODERS;
    return;
}

void NEXUS_VideoDecoder_SetVideoCmprStdList_priv( const bool *supportedCodecs, BXVD_ChannelSettings *channelSettings, unsigned videoCmprStdListSize)
{
    unsigned i;
    BDBG_ASSERT(videoCmprStdListSize == BAVC_VideoCompressionStd_eMax);
    channelSettings->uiVideoCmprCount = 0;
    for (i=0;i<BAVC_VideoCompressionStd_eMax;i++) {
        switch (i) {
#if NEXUS_CPU_ARM
        /* allow XVD to configure for VP6/8 memory */
#else
        case BAVC_VideoCompressionStd_eVP6:
        case BAVC_VideoCompressionStd_eVP8:
            /* Don't tell XVD to configure for 65/40nm, but allow start decode anyway. Keeps backward compatibility. */
            break;
#endif
        case BAVC_VideoCompressionStd_eH261:
        case BAVC_VideoCompressionStd_eSVC_BL:
            break;
        default:
            if (supportedCodecs[NEXUS_P_VideoCodec_FromMagnum(i)]) {
                channelSettings->peVideoCmprStdList[channelSettings->uiVideoCmprCount++] = i;
            }
            break;
        }
    }
}

/* XVD doesn't alloc based on actual sizes, but on BXVD_DecodeResolution. To avoid false errors,
convert to BXVD_DecodeResolution, then compare. BXVD_DecodeResolution is not sorted, so apply this mapping:
   BXVD_DecodeResolution_eHD   3
   BXVD_DecodeResolution_eSD   2
   BXVD_DecodeResolution_eCIF  1
   BXVD_DecodeResolution_eQCIF 0
   BXVD_DecodeResolution_e4K   4
*/
static unsigned g_decodeResolutionSortOrder[BXVD_DecodeResolution_eMaxModes] = {3,2,1,0,4};

BXVD_DecodeResolution NEXUS_VideoDecoder_GetDecodeResolution_priv( unsigned maxWidth, unsigned maxHeight)
{
    if (maxHeight == 0 || maxWidth == 0 ) {
        BDBG_ERR(("zero height or width specified"));
        return BXVD_DecodeResolution_eHD;
    }
    if(maxHeight > 1088 || maxWidth > 1920) {
        return BXVD_DecodeResolution_e4K;
    }
    else if (maxHeight <= 144 && maxWidth <= 176) {
        return BXVD_DecodeResolution_eQCIF;
    }
    else if (maxHeight <= 288 && maxWidth <= 352) {
        return BXVD_DecodeResolution_eCIF;
    }
    else if (maxHeight <= 576 && maxWidth <= 720) {
        return BXVD_DecodeResolution_eSD;
    }
    else if(maxHeight <= 1088 && maxWidth <= 1920){
        return BXVD_DecodeResolution_eHD;
    }
    else {
        BDBG_ERR(("Invalid height %d or width %d specified",maxHeight,maxWidth));
        return BXVD_DecodeResolution_eHD;
    }
}

static void NEXUS_VideoDecoderModule_P_Print_Avd(void)
{
#if BDBG_DEBUG_BUILD
    unsigned i;

    BDBG_LOG(("VideoDecoderModule:"));
    for (i=0; i<NEXUS_NUM_XVD_DEVICES; i++) {
        unsigned j;
        char str[32];
        if (!g_NEXUS_videoDecoderXvdDevices[i].xvd) continue;
        if (g_NEXUS_videoDecoderModuleSettings.watchdogEnabled) {
            BKNI_Snprintf(str, sizeof(str), "%u", g_NEXUS_videoDecoderXvdDevices[i].numWatchdogs);
        }
        else {
            BKNI_Snprintf(str, sizeof(str), "%s", "disabled");
        }
        BDBG_LOG(("AVD%u: (%p) general:%2uMB secure:%2uMB picture:%3uMB watchdog:%s", i, (void *)g_NEXUS_videoDecoderXvdDevices[i].xvd,
            g_NEXUS_videoDecoderModuleSettings.heapSize[i].general/1024/1024,
            g_NEXUS_videoDecoderModuleSettings.heapSize[i].secure/1024/1024,
            g_NEXUS_videoDecoderModuleSettings.heapSize[i].picture/1024/1024,
            str));
        for (j=0; j < NEXUS_NUM_XVD_CHANNELS; j++) {
            NEXUS_VideoDecoderHandle videoDecoder = g_NEXUS_videoDecoderXvdDevices[i].channel[j];
            if (videoDecoder) {
                NEXUS_VideoDecoderStatus status;
                NEXUS_VideoDecoderTrickState trickState;
                NEXUS_PidChannelStatus pidChannelStatus;
                unsigned itbDepth, itbSize;
                NEXUS_Error rc;
                long ptsOffset;
                unsigned refreshRate;
                #if !BDBG_NO_LOG
                static const char *g_decodeModeStr[NEXUS_VideoDecoderDecodeMode_eMax] = {"All", "IP", "I"};
                #endif

                rc = NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
                if (rc) continue;
                NEXUS_VideoDecoder_GetTrickState(videoDecoder, &trickState);

                if (videoDecoder->started) {
                    NEXUS_PidChannel_GetStatus(videoDecoder->startSettings.pidChannel, &pidChannelStatus);
                }
                NEXUS_Rave_GetItbBufferInfo(videoDecoder->rave, &itbDepth, &itbSize);
                BKNI_EnterCriticalSection();
                if (videoDecoder->dec) {
                    BXVD_GetDisplayOffset_isr(videoDecoder->dec, &ptsOffset);
                }
                else {
                    ptsOffset = 0;
                }
                BKNI_LeaveCriticalSection();

                if (videoDecoder->mosaicMode) {
                    BKNI_Snprintf(str, sizeof(str), "nexus[%u.%u]", videoDecoder->parentIndex, videoDecoder->index);
                }
                else {
                    BKNI_Snprintf(str, sizeof(str), "nexus[%u]", videoDecoder->parentIndex);
                }
                BDBG_LOG((" ch%u(%p): %s videoInput=%p max=%ux%up%u %u bit", j, (void *)videoDecoder->dec, str, (void*)(&videoDecoder->input),
                    videoDecoder->settings.maxWidth, videoDecoder->settings.maxHeight,
                    (videoDecoder->memconfig.refreshRate+500)/1000, videoDecoder->settings.colorDepth));
                if (videoDecoder->started) {
                    BDBG_LOG(("  started: codec=%d, pid=%#x, pidCh=%p, stcCh=%p",
                       videoDecoder->startSettings.codec, pidChannelStatus.pid, (void *)videoDecoder->startSettings.pidChannel, (void *)videoDecoder->startSettings.stcChannel));
                }
                else {
                    BDBG_LOG(("  stopped"));
                }
                BDBG_LOG(("  CDB: %d/%d (%d%%), ITB: %d/%d (%d%%), pic_queue=%d",
                    status.fifoDepth, status.fifoSize, status.fifoSize?status.fifoDepth*100/status.fifoSize:0,
                    itbDepth, itbSize, itbSize?itbDepth*100/itbSize:0,
                    status.queueDepth));
                if (videoDecoder->dec) {
                    BDBG_LOG(("  TSM: %s pts=%#x pts_stc_diff=%d pts_offset=%#x errors=%d", videoDecoder->tsm?"enabled":"disabled", status.pts, status.ptsStcDifference,
                        (unsigned)ptsOffset, status.ptsErrorCount));
                    BDBG_LOG(("  Decode: decoded=%d drops=%d errors=%d overflows=%d", status.numDecoded, status.numDecodeDrops,
                        status.numDecodeErrors, status.numDecodeOverflows));
                    BDBG_LOG(("  Display: displayed=%d drops=%d errors=%d underflows=%d", status.numDisplayed, status.numDisplayDrops,
                        status.numDisplayErrors, status.numDisplayUnderflows));
                    BDBG_LOG(("  Trick: rate=%d %s brcm=%c host=%c stc=%c top=%c",
                        trickState.rate, g_decodeModeStr[trickState.decodeMode],
                        trickState.brcmTrickModesEnabled?'y':'n',
                        trickState.hostTrickModesEnabled?'y':'n',
                        trickState.stcTrickEnabled?'y':'n',
                        trickState.topFieldOnly?'y':'n'));
                    refreshRate = (NEXUS_P_RefreshRate_FromFrameRate_isrsafe(status.frameRate)+500) / 1000;
                    BDBG_LOG(("  Source: %ux%u%c%u %u bit", status.source.width, status.source.height,
                        status.interlaced?'i':'p', refreshRate, videoDecoder->streamInfo.colorDepth));
                    if ( status.source.width && (status.source.width != status.coded.width || status.source.height != status.coded.height)) {
                        BDBG_LOG(("  Coded: %ux%u", status.coded.width, status.coded.height));
                    }
                }
            }
        }
    }
#endif
}

NEXUS_ModuleHandle NEXUS_VideoDecoderModule_P_Init_Avd(const NEXUS_VideoDecoderModuleInternalSettings *pModuleSettings, const NEXUS_VideoDecoderModuleSettings *pSettings)
{
    BERR_Code rc;
    NEXUS_ModuleSettings moduleSettings;
    unsigned i;

    BDBG_ASSERT(!g_NEXUS_videoDecoderModule);

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow; /* decoder interface is slow */
    moduleSettings.dbgPrint = NEXUS_VideoDecoderModule_P_Print_Avd;
    moduleSettings.dbgModules = "nexus_video_decoder";
    g_NEXUS_videoDecoderModule = NEXUS_Module_Create("video_decoder", &moduleSettings);
    if (!g_NEXUS_videoDecoderModule) {
        return NULL;
    }

    /* init global data */
    BKNI_Memset(&g_NEXUS_videoDecoderXvdDevices, 0, sizeof(g_NEXUS_videoDecoderXvdDevices));
    g_NEXUS_videoDecoderModuleSettings = *pSettings;
    g_NEXUS_videoDecoderModuleInternalSettings = *pModuleSettings;

    /* touch up settings for non-memconfig platforms. everything should be uniform after this. */
    for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
        if (g_NEXUS_videoDecoderModuleSettings.memory[i].used) break;
    }
    if (i == NEXUS_MAX_VIDEO_DECODERS) {
        for (i=0;i<(unsigned)pSettings->numDecodes;i++) {
            g_NEXUS_videoDecoderModuleSettings.memory[i].used = true;
            BKNI_Memcpy(g_NEXUS_videoDecoderModuleSettings.memory[i].supportedCodecs, pSettings->supportedCodecs, sizeof(g_NEXUS_videoDecoderModuleSettings.memory[i].supportedCodecs));
            g_NEXUS_videoDecoderModuleSettings.memory[i].maxFormat = pSettings->maxDecodeFormat;
#if (BCHP_CHIP == 7405)
            if (i == 1) {
                /* forcing 7405 PIP to SD resolution */
                g_NEXUS_videoDecoderModuleSettings.memory[i].maxFormat = NEXUS_VideoFormat_eNtsc;
            }
#endif
        }
        if (pSettings->maxStillDecodeFormat) {
            g_NEXUS_videoDecoderModuleSettings.stillMemory[0].used = true;
            BKNI_Memcpy(g_NEXUS_videoDecoderModuleSettings.stillMemory[0].supportedCodecs, pSettings->supportedCodecs, sizeof(g_NEXUS_videoDecoderModuleSettings.stillMemory[0].supportedCodecs));
            g_NEXUS_videoDecoderModuleSettings.stillMemory[0].maxFormat = pSettings->maxStillDecodeFormat;
        }
    }

    if (!pSettings->deferInit) {
        NEXUS_LockModule();
        rc = NEXUS_VideoDecoderModule_P_PostInit();
        NEXUS_UnlockModule();
        if (rc) {
            NEXUS_Module_Destroy(g_NEXUS_videoDecoderModule);
            g_NEXUS_videoDecoderModule = NULL;
        }
    }

    return g_NEXUS_videoDecoderModule;
}


static NEXUS_Error NEXUS_VideoDecoderModule_P_PostInit(void)
{
    BERR_Code rc;
    unsigned i,j;
    const NEXUS_VideoDecoderModuleSettings *pSettings = &g_NEXUS_videoDecoderModuleSettings;

    /* open XVD device, not channels */
    for (i=0; i<NEXUS_NUM_XVD_DEVICES; i++) {
        BXVD_Settings xvdSettings;
        struct NEXUS_VideoDecoderDevice *vDevice = &g_NEXUS_videoDecoderXvdDevices[i];

        if (!pSettings->avdEnabled[i]) {
            continue;
        }

        BDBG_CASSERT(NEXUS_NUM_XVD_DEVICES <= NEXUS_MAX_HEAPS);
        if(pSettings->avdHeapIndex[i]==NEXUS_MAX_HEAPS || !g_pCoreHandles->heap[pSettings->avdHeapIndex[i]].mma)
        {
            vDevice->mem = g_pCoreHandles->heap[pSettings->secure.avdHeapIndex[i]].mma;
        }
        else
        {
            vDevice->mem = g_pCoreHandles->heap[pSettings->avdHeapIndex[i]].mma;
        }
        BXVD_GetDefaultSettings(&xvdSettings);

        if ( 0 == pSettings->heapSize[i].general )
        {
            BDBG_ERR(("XVD general heap size is 0. Please refer set correct XVD heap size according to XVD Memory Worksheet"));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error;
        }
        xvdSettings.stFWMemConfig.uiGeneralHeapSize = pSettings->heapSize[i].general;  /* Context memory (not included FW, see below) */
        xvdSettings.stFWMemConfig.uiCabacHeapSize =   pSettings->heapSize[i].secure;   /* SVP secure memory */
        xvdSettings.stFWMemConfig.uiPictureHeapSize = pSettings->heapSize[i].picture;  /* picture memory */
        if (pSettings->heapSize[i].secondaryPicture > 1) {
            xvdSettings.stFWMemConfig.uiPictureHeap1Size = pSettings->heapSize[i].secondaryPicture; /* for split picture buffer systems */
        }
        /* TODO: replace pSettings->hostAccessibleHeapIndex with g_pCoreHandles->defaultHeapIndex */
        BDBG_MSG(("BXVD_Open %d: fw heap=%d, avd heap=%d", i, pSettings->hostAccessibleHeapIndex, pSettings->avdHeapIndex[i]));
        xvdSettings.uiAVDInstance = i;
        xvdSettings.hFirmwareHeap = g_pCoreHandles->heap[pSettings->hostAccessibleHeapIndex].mma; /* FW always in main heap */
        xvdSettings.hGeneralHeap = xvdSettings.hFirmwareHeap; /* XXX new */
        if(xvdSettings.stFWMemConfig.uiPictureHeapSize) {
            xvdSettings.hPictureHeap = vDevice->mem;
        }
        if (xvdSettings.stFWMemConfig.uiPictureHeap1Size) {
            if(pSettings->secondaryPictureHeapIndex[i]==NEXUS_MAX_HEAPS || !g_pCoreHandles->heap[pSettings->secondaryPictureHeapIndex[i]].mma)
            {
                xvdSettings.hPictureHeap1 = g_pCoreHandles->heap[pSettings->secure.secondaryPictureHeapIndex[i]].mma;
            }
            else
            {
                xvdSettings.hPictureHeap1 = g_pCoreHandles->heap[pSettings->secondaryPictureHeapIndex[i]].mma;
            }
        }
        if (xvdSettings.stFWMemConfig.uiCabacHeapSize) {
            if (g_NEXUS_videoDecoderModuleInternalSettings.secureHeap) {
                xvdSettings.hCabacHeap = NEXUS_Heap_GetMmaHandle(g_NEXUS_videoDecoderModuleInternalSettings.secureHeap);
            }
        }
        xvdSettings.hTimerDev = g_pCoreHandles->tmr;
        xvdSettings.uiDecoderDebugLogBufferSize = pSettings->debugLogBufferSize;

        /* Image download */
        if (Nexus_Core_P_Img_Create(NEXUS_CORE_IMG_ID_XVD,&vDevice->img_context,&vDevice->img_interface )== NEXUS_SUCCESS)
        {
            BDBG_MSG(("FW download used"));
            xvdSettings.pImgInterface = (BIMG_Interface*)&vDevice->img_interface;
            xvdSettings.pImgContext   = vDevice->img_context;
        }
        else
        {
            BDBG_MSG(("FW download not used"));
            vDevice->img_context= NULL;
        }

        NEXUS_VideoDecoder_P_GetSecurityCallbacks(&xvdSettings, i /*deviceId*/);

        rc = BXVD_Open(&vDevice->xvd, g_pCoreHandles->chp, g_pCoreHandles->reg,
            g_pCoreHandles->bint, &xvdSettings);
        if (rc) {rc=BERR_TRACE(rc); goto error;}
        vDevice->index = i;
        BXVD_GetHardwareCapabilities(vDevice->xvd, &vDevice->cap);

        /* register event for task-time execution of xvd watchdog. the watchdog event and isr is device-level, even
        though the BXVD_Interrupt is channel-level. */
        BKNI_CreateEvent(&vDevice->watchdog_event);
        vDevice->watchdogEventHandler = NEXUS_RegisterEvent(vDevice->watchdog_event, NEXUS_VideoDecoder_P_WatchdogHandler, vDevice);

        /* install XVD device interrupts */
        if (g_NEXUS_videoDecoderModuleSettings.watchdogEnabled && !NEXUS_GetEnv("no_watchdog")) {
            rc = BXVD_InstallDeviceInterruptCallback(vDevice->xvd, BXVD_DeviceInterrupt_eWatchdog, NEXUS_VideoDecoder_P_Watchdog_isr, vDevice, 0);
            if (rc) {rc=BERR_TRACE(rc); goto error;}
        }
        for (j = 0; j < BXVD_DisplayInterrupt_eMax; j++)
        {
           rc = BXDM_DisplayInterruptHandler_Create(&vDevice->hXdmDih[j]);
            if (rc!=BERR_SUCCESS) {rc=BERR_TRACE(rc); goto error;}
        }
    }

    NEXUS_P_SetVideoDecoderCapabilities();

    return 0;

error:
    for (i=0; i<NEXUS_NUM_XVD_DEVICES; i++) {
        struct NEXUS_VideoDecoderDevice *vDevice = &g_NEXUS_videoDecoderXvdDevices[i];
        for (j = 0; j < BXVD_DisplayInterrupt_eMax; j++) {
            if (vDevice->hXdmDih[j]) {
                BXDM_DisplayInterruptHandler_Destroy(vDevice->hXdmDih[j]);
            }
        }
        if (vDevice->watchdogEventHandler) {
            NEXUS_UnregisterEvent(vDevice->watchdogEventHandler);
        }
        if (vDevice->watchdog_event) {
            BKNI_DestroyEvent(vDevice->watchdog_event);
        }
        if (vDevice->xvd) {
            BXVD_Close(vDevice->xvd);
        }
        if (vDevice->img_context) {
            Nexus_Core_P_Img_Destroy(vDevice->img_context);
        }
    }
    return rc;
}

void NEXUS_VideoDecoderModule_P_Uninit_Avd(void)
{
    unsigned i, j;

    NEXUS_LockModule();

    for (i=0;i<NEXUS_NUM_XVD_DEVICES;i++) {
        struct NEXUS_VideoDecoderDevice *vDevice = &g_NEXUS_videoDecoderXvdDevices[i];

        if (!vDevice->xvd) continue;

        for (j=0;j<NEXUS_NUM_XVD_CHANNELS;j++) {
            if (vDevice->channel[j]) {
                NEXUS_VideoDecoder_Close(vDevice->channel[j]);
            }
        }

        for (j = 0; j < BXVD_DisplayInterrupt_eMax; j++)
        {
           BXDM_DisplayInterruptHandler_Destroy(vDevice->hXdmDih[j]);
           vDevice->hXdmDih[j] = NULL;
        }
        if (g_NEXUS_videoDecoderModuleSettings.watchdogEnabled) {
            BXVD_UnInstallDeviceInterruptCallback(vDevice->xvd, BXVD_DeviceInterrupt_eWatchdog);
        }

        NEXUS_UnregisterEvent(vDevice->watchdogEventHandler);
        BKNI_DestroyEvent(vDevice->watchdog_event);
        BXVD_Close(vDevice->xvd);
        Nexus_Core_P_Img_Destroy(vDevice->img_context);

        NEXUS_VideoDecoder_P_DisableFwVerification(i);
    }

    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_videoDecoderModule);
    g_NEXUS_videoDecoderModule = NULL;
}

/**************************************
* Interface functions
**************************************/
void NEXUS_VideoDecoder_P_GetRaveSettings(unsigned avdIndex, NEXUS_RaveOpenSettings *pRaveSettings, const NEXUS_VideoDecoderOpenMosaicSettings *pOpenSettings)
{
    int32_t pic_buf_length;

    LOCK_TRANSPORT();
    NEXUS_Rave_GetDefaultOpenSettings_priv(pRaveSettings);
    UNLOCK_TRANSPORT();

    (void)BXVD_GetBufferConfig(g_NEXUS_videoDecoderXvdDevices[avdIndex].xvd, &pRaveSettings->config, &pic_buf_length);
    pRaveSettings->config.Cdb.Length = B_XVD_VIDEO_CDB_SIZE;
    pRaveSettings->config.Itb.Length = B_XVD_VIDEO_ITB_SIZE;

    if (pOpenSettings->openSettings.fifoSize) {
        if (pOpenSettings->openSettings.itbFifoSize) {
            /* user-specific ITB */
            pRaveSettings->config.Itb.Length = pOpenSettings->openSettings.itbFifoSize;
        }
        else {
            /* change ITB size relative to the fifoSize change */
            pRaveSettings->config.Itb.Length = pRaveSettings->config.Itb.Length * (pOpenSettings->openSettings.fifoSize/10000) / (pRaveSettings->config.Cdb.Length/10000);
            /* ITB length must be 128 byte aligned */
            pRaveSettings->config.Itb.Length -= (pRaveSettings->config.Itb.Length % 128);
        }

        /* If the user has specified a CDB size, use it...
         * but only after we've resized the ITB based on the requested new CDB
         * size compared to the old CDB size. */
        pRaveSettings->config.Cdb.Length = pOpenSettings->openSettings.fifoSize;
    }
    pRaveSettings->heap = pOpenSettings->openSettings.cdbHeap;
}

NEXUS_Error NEXUS_VideoDecoder_P_Init_Generic(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_RaveOpenSettings *raveSettings, const NEXUS_VideoDecoderOpenMosaicSettings *pOpenSettings)
{
    NEXUS_Error rc;
    bool openEnhancementRave;
    NEXUS_VideoFormatInfo videoFormatInfo;
    NEXUS_RaveOpenSettings copyRaveSettings;

    BDBG_ASSERT(!videoDecoder->rave);
    BDBG_ASSERT(!videoDecoder->enhancementRave);

    /* verify API macros aren't below nexus_platform_features.h */
    BDBG_CASSERT(NEXUS_MAX_VIDEO_DECODERS >= NEXUS_NUM_VIDEO_DECODERS);

    videoDecoder->otfPvr.wasActive = false;
    videoDecoder->errorHandlingOverride = false;
    videoDecoder->errorHandlingMode = NEXUS_VideoDecoderErrorHandling_ePicture;

    openEnhancementRave = pOpenSettings->openSettings.enhancementPidChannelSupported;

    NEXUS_VIDEO_INPUT_INIT(&videoDecoder->input, NEXUS_VideoInputType_eDecoder, videoDecoder);
    NEXUS_OBJECT_REGISTER(NEXUS_VideoInput, &videoDecoder->input, Open);

    videoDecoder->openSettings = *pOpenSettings;
    NEXUS_OBJECT_SET(NEXUS_VideoDecoder, videoDecoder);
    videoDecoder->displayInformation.refreshRate = 5994; /* 0.01 Hz */

#if NEXUS_HAS_ASTM
    BKNI_Memset(&videoDecoder->astm.settings, 0, sizeof(NEXUS_VideoDecoderAstmSettings));
    BKNI_Memset(&videoDecoder->astm.status, 0, sizeof(NEXUS_VideoDecoderAstmStatus));
#endif

    videoDecoder->userdataCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->userdataCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->sourceChangedCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->sourceChangedCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->streamChangedCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->streamChangedCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->private.streamChangedCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->private.streamChangedCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->ptsErrorCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->ptsErrorCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->firstPtsCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->firstPtsCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->firstPtsPassedCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->firstPtsPassedCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->dataReadyCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->dataReadyCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->fifoEmpty.callback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->fifoEmpty.callback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->afdChangedCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->afdChangedCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->decodeErrorCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->decodeErrorCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->s3DTVChangedCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->s3DTVChangedCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->fnrtChunkDoneCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->fnrtChunkDoneCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->stateChangedCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->stateChangedCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}

    videoDecoder->playback.firstPtsCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->playback.firstPtsCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}
    videoDecoder->playback.firstPtsPassedCallback = NEXUS_IsrCallback_Create(videoDecoder, NULL);
    if(!videoDecoder->playback.firstPtsPassedCallback) {rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);goto error;}

    videoDecoder->userdata.mem = BMMA_Alloc(g_pCoreHandles->heap[g_NEXUS_videoDecoderModuleSettings.hostAccessibleHeapIndex].mma, pOpenSettings->openSettings.userDataBufferSize, 0, NULL);
    if (!videoDecoder->userdata.mem) {rc = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto error;}
    videoDecoder->userdata.buf = BMMA_Lock(videoDecoder->userdata.mem);
    if (!videoDecoder->userdata.buf) {rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto error;}

    rc = BKNI_CreateEvent(&videoDecoder->source_changed_event);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    rc = BKNI_CreateEvent(&videoDecoder->channelChangeReportEvent);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    videoDecoder->channelChangeReportEventHandler = NEXUS_RegisterEvent(videoDecoder->channelChangeReportEvent, NEXUS_VideoDecoder_P_ChannelChangeReport, videoDecoder);
    if (videoDecoder->channelChangeReportEventHandler == NULL) {rc=BERR_TRACE(rc); goto error;}

    rc= BKNI_CreateEvent(&videoDecoder->s3DTVStatusEvent);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    videoDecoder->s3DTVStatusEventHandler = NEXUS_RegisterEvent(videoDecoder->s3DTVStatusEvent, NEXUS_VideoDecoder_P_3DTVTimer, videoDecoder);
    if (videoDecoder->s3DTVStatusEventHandler == NULL) {rc=BERR_TRACE(rc); goto error;}

    rc = BKNI_CreateEvent(&videoDecoder->stc.statusChangeEvent);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    videoDecoder->stc.statusChangeEventHandler = NEXUS_RegisterEvent(videoDecoder->stc.statusChangeEvent, &NEXUS_VideoDecoder_P_StcChannelStatusChangeEventHandler, videoDecoder);
    if (videoDecoder->stc.statusChangeEventHandler == NULL) {rc=BERR_TRACE(rc); goto error;}

    videoDecoder->stc.priority = 0; /* highest priority video */

    /* convert from module settings max format to OpenMosaicSettings max width/height */
    NEXUS_VideoFormat_GetInfo(g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].maxFormat, &videoFormatInfo);
    videoDecoder->memconfig.maxWidth = videoFormatInfo.width;
    videoDecoder->memconfig.maxHeight = videoFormatInfo.height;
    videoDecoder->memconfig.refreshRate = videoFormatInfo.verticalFreq * 10;

    if (!videoDecoder->mosaicMode && !pOpenSettings->openSettings.fifoSize && videoDecoder->memconfig.maxWidth > 1920) {
        /* default to larger CDB for 4K2K, same ITB */
        copyRaveSettings = *raveSettings;
        copyRaveSettings.config.Cdb.Length = 6*1024*1024;
        raveSettings = &copyRaveSettings;
    }
    videoDecoder->cdbLength = raveSettings->config.Cdb.Length;
    videoDecoder->itbLength = raveSettings->config.Itb.Length;

    LOCK_TRANSPORT();
    videoDecoder->rave = NEXUS_Rave_Open_priv(raveSettings);
    UNLOCK_TRANSPORT();
    if (!videoDecoder->rave) {rc=BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto error;}
    LOCK_TRANSPORT();
    if(openEnhancementRave) {
        videoDecoder->enhancementRave = NEXUS_Rave_Open_priv(raveSettings);
    }
    UNLOCK_TRANSPORT();
    if (openEnhancementRave && !videoDecoder->enhancementRave) {rc=BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto error;}

    /* set default settings */
    BKNI_Memset(&videoDecoder->settings, 0, sizeof(videoDecoder->settings));
    videoDecoder->settings.dropFieldMode = false;
    BKNI_Memcpy(videoDecoder->settings.supportedCodecs, raveSettings->supportedCodecs, sizeof(videoDecoder->settings.supportedCodecs));

    BKNI_Memset(&videoDecoder->extendedSettings.lowLatencySettings, 0, sizeof(NEXUS_VideoDecoderLowLatencySettings));
    videoDecoder->extendedSettings.lowLatencySettings.controlPeriod = 3000;
    videoDecoder->extendedSettings.lowLatencySettings.latency = 100;
    videoDecoder->extendedSettings.lowLatencySettings.minLatency = 0;
    videoDecoder->extendedSettings.lowLatencySettings.maxLatency = 200;

    if (videoDecoder->memconfig.maxWidth > 1920) {
        /* don't propagate 4K2K from init-time to run-time settings. require app to set. */
        videoDecoder->settings.maxWidth = 1920;
        videoDecoder->settings.maxHeight = 1080;
    }
    else {
        videoDecoder->settings.maxWidth = videoFormatInfo.width;
        videoDecoder->settings.maxHeight = videoFormatInfo.height;
    }
    videoDecoder->settings.maxFrameRate = NEXUS_VideoFrameRate_e60;
    videoDecoder->settings.colorDepth = 8; /* require app to select 10 bit */
    videoDecoder->settings.userDataFilterThreshold = 60; /* 2 seconds */

    return NEXUS_SUCCESS;

error:
    /* TODO: full unwind */
    if (videoDecoder->userdata.buf) {
        BMMA_Unlock(videoDecoder->userdata.mem, videoDecoder->userdata.buf);
        videoDecoder->userdata.buf = NULL;
    }
    if (videoDecoder->userdata.mem) {
        BMMA_Free(videoDecoder->userdata.mem);
        videoDecoder->userdata.mem = NULL;
    }
    LOCK_TRANSPORT();
    if (videoDecoder->rave) {
        NEXUS_Rave_Close_priv(videoDecoder->rave);
        videoDecoder->rave = NULL;
    }
    if (videoDecoder->enhancementRave) {
        NEXUS_Rave_Close_priv(videoDecoder->enhancementRave);
        videoDecoder->enhancementRave = NULL;
    }
    UNLOCK_TRANSPORT();
    BDBG_ASSERT(rc);
    return rc;
}

NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_P_Open(int parentIndex, unsigned index, const NEXUS_VideoDecoderOpenMosaicSettings *pOpenSettings)
{
    BERR_Code rc;
    struct NEXUS_VideoDecoderDevice *pDevice;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_RaveOpenSettings raveSettings;
    unsigned mfdIndex, channelIndex, avdIndex;
    unsigned i;

    if (!g_NEXUS_videoDecoderModule) {
        BDBG_ERR(("Module not initialized"));
        return NULL;
    }

    for (i=0; i<NEXUS_NUM_XVD_DEVICES; i++) {
        if (g_NEXUS_videoDecoderModuleSettings.avdEnabled[i]) {
            if (g_NEXUS_videoDecoderXvdDevices[i].xvd) {
                break;
            }
        }
    }
    if (i==NEXUS_NUM_XVD_DEVICES) {
        rc = NEXUS_VideoDecoderModule_P_PostInit();
        if (rc) {
            BERR_TRACE(rc);
            return NULL;
        }
    }

    /**
    7405 has one AVD outputting to two MFD's. g_NEXUS_videoDecoderXvdDevices[] represents the AVD. mainIndex represents the MFD.
    The MFD0 is used for main and mosaic. mainIndex = 0, channelIndex = 0,2,3,4,... (i.e. skip 1).
    The MFD1 is used for PIP. In that case, mainIndex = 1, channelIndex = 1.
    7422/25 has two AVDs outputting to three MFD's. AVD1 drives MFD1&2, while AVD0 (SVC capable) drives MFD0.
    **/
    /* mfdIndex is the MFD index; avdIndex is the device index; channelIndex is the channel num on that device. */
    if (parentIndex == -1) {
        if (g_NEXUS_videoDecoderModuleSettings.avdMapping[index] >= NEXUS_NUM_XVD_DEVICES) {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            return NULL;
        }
        mfdIndex = g_NEXUS_videoDecoderModuleSettings.mfdMapping[index];
        avdIndex  = g_NEXUS_videoDecoderModuleSettings.avdMapping[index];
        if(g_videoDecoders[index]) {
            BDBG_WRN(("VideoDecoder %d already open", index));
            return NULL;
        }
    }
    else {
        if (g_NEXUS_videoDecoderModuleSettings.avdMapping[parentIndex] >= NEXUS_NUM_XVD_DEVICES) {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            return NULL;
        }
        mfdIndex = g_NEXUS_videoDecoderModuleSettings.mfdMapping[parentIndex];
        avdIndex  = g_NEXUS_videoDecoderModuleSettings.avdMapping[parentIndex];
    }
    if (avdIndex >= NEXUS_NUM_XVD_DEVICES) {
        BDBG_ERR(("AVD%d not supported. %d AVD's supported.", avdIndex, NEXUS_NUM_XVD_DEVICES));
        return NULL;
    }
    pDevice = &g_NEXUS_videoDecoderXvdDevices[avdIndex];

    /* find available channel */
    for(channelIndex = 0; channelIndex < NEXUS_NUM_XVD_CHANNELS; channelIndex++)
    {
        /* search for unused channel per the AVD device */
        if(!pDevice->channel[channelIndex])
        {
            break;
        }
    }
    BDBG_MSG(("NEXUS_VideoDecoder_P_Open %u.%u, AVD%d, MFD%d, channel%d, %ux%u",
        (parentIndex != -1)?(unsigned)parentIndex:index,
        index, avdIndex, mfdIndex, channelIndex, pOpenSettings->maxWidth, pOpenSettings->maxHeight));
    if (channelIndex >= NEXUS_NUM_XVD_CHANNELS) {
        BDBG_ERR(("XVD channel %d not supported. %d channels supported.", channelIndex, NEXUS_NUM_XVD_CHANNELS));
        return NULL;
    }

    videoDecoder = BKNI_Malloc(sizeof(*videoDecoder));
    if (!videoDecoder) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(videoDecoder, 0, sizeof(*videoDecoder));
    pDevice->channel[channelIndex] = videoDecoder;
    videoDecoder->device = pDevice;
    videoDecoder->channelIndex = channelIndex;
    videoDecoder->index = index;
    videoDecoder->parentIndex = (parentIndex != -1)?(unsigned)parentIndex:index;
    videoDecoder->mosaicMode = (parentIndex != -1);
    videoDecoder->mfdIndex = mfdIndex;

    /* Note, the global flag should be set after all previous error checking passed to avoid side effect. */
    /* avoid re-open an opened decoder */
    if (!videoDecoder->mosaicMode) {
        g_videoDecoders[index] = videoDecoder;
    }

    /* copy from open to runtime settings. they may change. */
    videoDecoder->mosaicSettings.maxWidth = pOpenSettings->maxWidth;
    videoDecoder->mosaicSettings.maxHeight = pOpenSettings->maxHeight;

    NEXUS_VideoDecoder_P_GetRaveSettings(pDevice->index, &raveSettings, pOpenSettings);
    videoDecoder->intf = &NEXUS_VideoDecoder_P_Interface_Avd;

    /* set supportedCodecs[] based on module supportedCodecs[] and XVD's report of HW capabilities */
    {
        NEXUS_VideoCodec i;
        for (i=0;i<NEXUS_VideoCodec_eMax;i++) {
            BAVC_VideoCompressionStd xvdCodec = NEXUS_P_VideoCodec_ToMagnum(i, NEXUS_TransportType_eTs);
            if (i == NEXUS_VideoCodec_eH264_Mvc || i == NEXUS_VideoCodec_eH264_Svc) {
                /* don't propagate MVC/SVC from init-time to run-time settings. require app to set. */
                raveSettings.supportedCodecs[i] = false;
            }
            else if (g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].supportedCodecs[i] && videoDecoder->device->cap.bCodecSupported[xvdCodec]) {
                raveSettings.supportedCodecs[i] = true;
            }
            else {
                raveSettings.supportedCodecs[i] = false;
            }
        }
    }

    rc = NEXUS_VideoDecoder_P_Init_Generic(videoDecoder, &raveSettings, pOpenSettings);
    if(rc!=NEXUS_SUCCESS) { rc=BERR_TRACE(rc);goto error; }

    if (videoDecoder->mosaicMode) {
        videoDecoder->settings.colorDepth = g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].mosaic.colorDepth;
    }
    else {
        videoDecoder->settings.colorDepth = g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].colorDepth;
    }

#if NEXUS_HAS_SYNC_CHANNEL
    BKNI_Memset(&videoDecoder->sync.status, 0, sizeof(videoDecoder->sync.status));
    videoDecoder->sync.status.digital = true;
#endif

    return videoDecoder;

error:
    /* this will close whatever has been opened */
    NEXUS_VideoDecoder_Close(videoDecoder);
    return NULL;
}

static BERR_Code NEXUS_VideoDecoder_P_PictureDataReady_isr(void *context, int32_t param, BAVC_MFD_Picture *picture)
{
    NEXUS_VideoDecoder_P_DataReady_isr(context, param, picture);
    return BERR_SUCCESS;
}

#define MS_2_45K(M) ((M) * 45)
#define NEXUS_LOW_LATENCY_P_MAX_PICTURES_TO_EXAMINE 60     /* Will retrieve the information for N pictures. */
#if 0
#define NEXUS_LOW_LATENCY_P_DEFAULT_DESIRED_AVG_PIX_ON_QUEUE 3
#define NEXUS_LOW_LATENCY_P_DEFAULT_CONTROL_PERIOD 3000 /* ms */
#endif

#define LOW_LATENCY_TRACE(x) /*BDBG_MSG(x)*/

static BXVD_PictureHandlingMode NEXUS_VideoDecoder_P_DoFastLowLatency_isr(
    NEXUS_VideoDecoderHandle videoDecoder,
    BXVD_TSMResult tsmResult,
    const BXVD_PPBParameterInfo * lastPicture,
    unsigned pictureDuration,
    unsigned queueDepth,
    unsigned targetDepth,
    int32_t * pPtsOffset
)
{
#if 0
    BERR_Code rc = BERR_SUCCESS;
#endif
    BXVD_PictureHandlingMode phMode = BXVD_PictureHandlingMode_eDefault;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoDecoder, videoDecoder);
    BDBG_ASSERT(lastPicture);
    BDBG_ASSERT(pPtsOffset);

    LOW_LATENCY_TRACE(("DoFastLowLatency_isr: tsm = %u; lastpic = %p; picdur = %u; depth = %u; target = %u; offset = %d",
        tsmResult,
        lastPicture,
        pictureDuration,
        queueDepth,
        targetDepth,
        *pPtsOffset));

#if 0
    /* All this does is force the DM to drain the queue, it doesn't hit a latency target */
    if (tsmResult == BXVD_TSMResult_eWait)
    {
        bool advanceToEnd;

        if (targetDepth)
        {
            advanceToEnd = (queueDepth > targetDepth);
        }
        else
        {
            /* This logic is from the Wifi display group.  It sets the drop threshold based
            * on the framerate.
            */
            if (pictureDuration > 900)
            {
                advanceToEnd = (queueDepth > 2);
            }
            else
            {
                advanceToEnd = (queueDepth > 3);
            }
        }

        if (advanceToEnd)
        {
            /* If the last picture on the queue has a valid PTS, adjust the PTS
            * offset to eat up the pix between current and last. If it does not
            * have a valid PTS, set the pts offset to -n pic times, which will
            * drop N pix from the queue.
            */
            if (lastPicture->stPTS.bValid)
            {
                BXVD_PTSInfo pts;

                /* get the current PTS */
                rc = BXVD_GetPTS_isr(videoDecoder->dec, &pts);
                if (rc) { BERR_TRACE(rc); goto error; }

                *pPtsOffset -= (signed)(lastPicture->stPTS.uiValue - pts.ui32RunningPTS);
            }
            else
            {
                *pPtsOffset -= (signed)(pictureDuration * queueDepth);
            }

            /* Force this picture to be evaluated in vsync mode.
            * It will be displayed on this vsync.
            */
            phMode = BXVD_PictureHandlingMode_eIgnorePTS;
        }
    }
#else
    BSTD_UNUSED(tsmResult);
    if (queueDepth > targetDepth)
    {
        *pPtsOffset -= (signed)(pictureDuration * (queueDepth - targetDepth));
    }
    else if (targetDepth > queueDepth)
    {
        *pPtsOffset += (signed)(pictureDuration * (targetDepth - queueDepth));
    }
#endif

#if 0
error:
#endif
    return phMode;
}

#define NEXUS_LOW_LATENCY_P_TAKE_QD_SAMPLE(LL, QD, TIME) \
    (LL)->sumQueueDepth += (QD); \
    (LL)->sampleCount++; \
    (LL)->lastSampleTime = (TIME);

#define NEXUS_LOW_LATENCY_P_RESET_QD_SAMPLE_DATA(LL) \
    (LL)->sumQueueDepth = 0; \
    (LL)->sampleCount = 0; \
    (LL)->lastSampleTime = 0;

static BXVD_PictureHandlingMode NEXUS_VideoDecoder_P_DoAverageLowLatency_isr(
    NEXUS_VideoDecoderHandle videoDecoder,
    unsigned pictureDuration,
    uint32_t currentTime,
    unsigned queueDepth,
    unsigned targetDepth,
    int32_t * pPtsOffset
)
{
    unsigned avgQueueDepth;
    BXVD_PictureHandlingMode phMode = BXVD_PictureHandlingMode_eDefault;
    NEXUS_VideoDecoderLowLatencySettings * pSettings;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoDecoder, videoDecoder);
    BDBG_ASSERT(pPtsOffset);

    LOW_LATENCY_TRACE(("DoAverageLowLatency_isr: pic = %u; time = %u; depth = %u; target = %u; offset = %d",
        pictureDuration,
        currentTime,
        queueDepth,
        targetDepth,
        *pPtsOffset));

    pSettings = &videoDecoder->extendedSettings.lowLatencySettings;

    /* this will be zero first time around and each time after we apply control */
    if (videoDecoder->lowLatency.sampleCount)
    {
        int32_t sampleElapsed = currentTime - videoDecoder->lowLatency.lastSampleTime;
        int32_t controlElapsed = currentTime - videoDecoder->lowLatency.lastControlTime;

        if (sampleElapsed > 0 && controlElapsed > 0)
        {
            if (pictureDuration && sampleElapsed >= (signed)pictureDuration) /* sampling period is ~1 frame */
            {
                if (controlElapsed > (signed)MS_2_45K(pSettings->controlPeriod))
                {
                    avgQueueDepth = videoDecoder->lowLatency.sumQueueDepth /
                        videoDecoder->lowLatency.sampleCount;

                    LOW_LATENCY_TRACE(("samples: %u", videoDecoder->lowLatency.sampleCount));

                    /* reset for next measurement cycle */
                    NEXUS_LOW_LATENCY_P_RESET_QD_SAMPLE_DATA(&videoDecoder->lowLatency);
                    videoDecoder->lowLatency.lastControlTime = currentTime;

                    if (avgQueueDepth < targetDepth)
                    {
                        *pPtsOffset += ((signed)(pictureDuration)) * (signed)(targetDepth - avgQueueDepth);
                    }
                    else if (avgQueueDepth > targetDepth)
                    {
                        *pPtsOffset -= ((signed)(pictureDuration)) * (signed)(avgQueueDepth - targetDepth);
                    }

                    LOW_LATENCY_TRACE(("queue depth: %u/%u/%u", queueDepth, avgQueueDepth, targetDepth));
                }

                NEXUS_LOW_LATENCY_P_TAKE_QD_SAMPLE(&videoDecoder->lowLatency, queueDepth, currentTime);
            }
            /* else not enough time has elapsed, or no valid picture time, do nothing but wait */
        }
        else if (sampleElapsed < 0 || controlElapsed < 0)
        {
            BDBG_WRN(("Time discontinuity detected, resetting low latency measurements"));
            NEXUS_LOW_LATENCY_P_RESET_QD_SAMPLE_DATA(&videoDecoder->lowLatency);
            videoDecoder->lowLatency.lastControlTime = currentTime;
        }
        /* else no time has elapsed, do nothing but wait */
    }
    else
    {
        /* this grabs the first sample, all other samples we want at least a frame period between */
        NEXUS_LOW_LATENCY_P_TAKE_QD_SAMPLE(&videoDecoder->lowLatency, queueDepth, currentTime);
    }

    return phMode;
}

static bool NEXUS_VideoDecoder_P_QualitySucks_isr(
    NEXUS_VideoDecoderHandle videoDecoder,
    unsigned pictureDuration,
    uint32_t currentTime,
    unsigned queueDepth,
    unsigned targetDepth
)
{
    /* TODO */
    BSTD_UNUSED(videoDecoder);
    BSTD_UNUSED(pictureDuration);
    BSTD_UNUSED(currentTime);
    return queueDepth < targetDepth;
}

static bool NEXUS_VideoDecoder_P_LatencyCouldBeBetter_isr(
    NEXUS_VideoDecoderHandle videoDecoder,
    unsigned pictureDuration,
    uint32_t currentTime,
    unsigned queueDepth,
    unsigned targetDepth
)
{
    /* TODO */
    BSTD_UNUSED(videoDecoder);
    BSTD_UNUSED(pictureDuration);
    BSTD_UNUSED(currentTime);
    BSTD_UNUSED(queueDepth);
    BSTD_UNUSED(targetDepth);
    return false;
}

static BXVD_PictureHandlingMode NEXUS_VideoDecoder_P_DoAdaptiveLowLatency_isr(
    NEXUS_VideoDecoderHandle videoDecoder,
    unsigned pictureDuration,
    uint32_t currentTime,
    unsigned queueDepth,
    unsigned targetDepth,
    unsigned minDepth,
    unsigned maxDepth,
    int32_t * pPtsOffset
)
{
    int32_t controlElapsed;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoDecoder, videoDecoder);
    BDBG_ASSERT(pPtsOffset);

    LOW_LATENCY_TRACE(("DoAdaptiveLowLatency_isr: pic = %u; time = %u; depth = %u; target = %u; min = %u; max = %u; offset = %d",
        pictureDuration,
        currentTime,
        queueDepth,
        targetDepth,
        minDepth,
        maxDepth,
        *pPtsOffset));

    controlElapsed = currentTime - videoDecoder->lowLatency.lastControlTime;

    if (controlElapsed > (signed)MS_2_45K(videoDecoder->extendedSettings.lowLatencySettings.controlPeriod))
    {
        if (NEXUS_VideoDecoder_P_QualitySucks_isr(videoDecoder,
            pictureDuration,
            currentTime,
            queueDepth,
            targetDepth))
        {
            targetDepth++;
            if (targetDepth > maxDepth)
            {
                targetDepth = maxDepth;
                LOW_LATENCY_TRACE(("hit max latency target, quality cannot improve"));
            }
        }
        else if (NEXUS_VideoDecoder_P_LatencyCouldBeBetter_isr(videoDecoder,
            pictureDuration,
            currentTime,
            queueDepth,
            targetDepth))
        {
            targetDepth--;
            if (targetDepth < minDepth)
            {
                targetDepth = minDepth;
                LOW_LATENCY_TRACE(("hit min quality target, latency cannot improve"));
            }
        }
        else
        {
            /* do not change the target*/
        }
    }

    return NEXUS_VideoDecoder_P_DoAverageLowLatency_isr(
        videoDecoder,
        pictureDuration,
        currentTime,
        queueDepth,
        targetDepth,
        pPtsOffset);
}

typedef struct NEXUS_VideoDecoder_P_FrameRateRatio
{
    unsigned numerator;
    unsigned denominator;
} NEXUS_VideoDecoder_P_FrameRateRatio;

static void NEXUS_VideoDecoder_P_GetFrameRateRatio_isr(
    NEXUS_VideoDecoderHandle videoDecoder,
    const BXVD_PPBParameterInfo * firstPicture,
    NEXUS_VideoDecoder_P_FrameRateRatio * ratio
)
{
    NEXUS_OBJECT_ASSERT(NEXUS_VideoDecoder, videoDecoder);
    BDBG_ASSERT(ratio);

    if (videoDecoder->streamInfo.valid)
    {
        ratio->numerator = NEXUS_P_RefreshRate_FromFrameRate_isrsafe(videoDecoder->streamInfo.frameRate);
        ratio->denominator = 1000;
        if (ratio->numerator)
        {
            LOW_LATENCY_TRACE(("stream info valid: ratio = %u/%u", ratio->numerator, ratio->denominator));
        }
        else
        {
            ratio->numerator = 0;
            ratio->denominator = 1;
            LOW_LATENCY_TRACE(("stream info valid; unknown frame rate: ratio = %u/%u", ratio->numerator, ratio->denominator));
        }
    }
    else if (firstPicture)
    {
        /* Use the frame rate of the first picture on the queue. (The frame rate
        * should be the same for all the pictures on the queue.)
        */
        ratio->numerator = firstPicture->stFrameRate.stRate.uiNumerator;
        ratio->denominator = firstPicture->stFrameRate.stRate.uiDenominator;
        if (ratio->numerator && ratio->denominator)
        {
            LOW_LATENCY_TRACE(("stream info invalid: ratio = %u/%u", ratio->numerator, ratio->denominator));
        }
        else
        {
            ratio->numerator = 0;
            ratio->denominator = 1;
            LOW_LATENCY_TRACE(("stream info invalid; rate on queue unknown: ratio = %u/%u", ratio->numerator, ratio->denominator));
        }
    }
    else
    {
        ratio->numerator = 0;
        ratio->denominator = 1;
        LOW_LATENCY_TRACE(("stream info invalid: no pix on queue; ratio = %u/%u", ratio->numerator, ratio->denominator));
    }
}

static unsigned NEXUS_VideoDecoder_P_ComputeSourcePictureDuration_isr(
    NEXUS_VideoDecoderHandle videoDecoder,
    const NEXUS_VideoDecoder_P_FrameRateRatio * ratio
)
{
    unsigned pictureDuration;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoDecoder, videoDecoder);
    BDBG_ASSERT(ratio);

    if (ratio->numerator)
    {
        /* want 45 KHz ticks, frame rate ratio is in mHz */
        pictureDuration = (45000 * ratio->denominator) / ratio->numerator;

        /* If the frame rate is 59.94, 29.97... bump the picture time.
        * We may not need to be this accurate.
        */
        if (ratio->denominator != 1000)
        {
            pictureDuration += 1;
        }
    }
    else
    {
        pictureDuration = 0;
    }

    return pictureDuration;
}

static void NEXUS_VideoDecoder_P_BoundLowLatencyPtsOffset_isr(
    NEXUS_VideoDecoderHandle videoDecoder,
    int32_t * pPtsOffset
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BXVD_DisplayThresholds thresholds;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoDecoder, videoDecoder);
    BDBG_ASSERT(pPtsOffset);

    rc = BXVD_GetDisplayThresholds_isr(videoDecoder->dec, &thresholds);
    if (!rc && thresholds.ui32DiscardThreshold)
    {
        if (*pPtsOffset > (signed)thresholds.ui32DiscardThreshold)
        {
            *pPtsOffset = (signed)thresholds.ui32DiscardThreshold - MS_2_45K(2);
            LOW_LATENCY_TRACE(("Low latency PTS offset hit upper bound of %d\n", thresholds.ui32DiscardThreshold));
        }
        else if (*pPtsOffset < -(signed)thresholds.ui32DiscardThreshold)
        {
            *pPtsOffset = -(signed)thresholds.ui32DiscardThreshold + MS_2_45K(2);
            LOW_LATENCY_TRACE(("Low latency PTS offset hit lower bound of %d\n", -(signed)thresholds.ui32DiscardThreshold));
        }
    }
    else
    {
        /* just use +/- 1 second */
        if (*pPtsOffset > MS_2_45K(1000))
        {
            *pPtsOffset = MS_2_45K(1000);
            LOW_LATENCY_TRACE(("Low latency PTS offset hit upper bound of %d\n", MS_2_45K(1000)));
        }
        else if (*pPtsOffset < -(signed)thresholds.ui32DiscardThreshold)
        {
            *pPtsOffset = MS_2_45K(-1000);
            LOW_LATENCY_TRACE(("Low latency PTS offset hit lower bound of %d\n", MS_2_45K(-1000)));
        }
    }
}

static void NEXUS_VideoDecoder_P_DoLowLatency_isr(
    NEXUS_VideoDecoderHandle videoDecoder,
    BXVD_TSMInfo * pTSMInfo
)
{
    static const BXVD_PPBParameterInfo * pictures[NEXUS_LOW_LATENCY_P_MAX_PICTURES_TO_EXAMINE];
    BERR_Code rc = BERR_SUCCESS;
    unsigned queueDepth = 0;
    unsigned pictureDuration = 0;
    NEXUS_VideoDecoder_P_FrameRateRatio ratio;
    int32_t ptsOffset;
    uint32_t currentTime = 0;
    unsigned targetDepth = 0;
    unsigned minDepth = 0;
    unsigned maxDepth = 0;
    NEXUS_StcChannelHandle stcChannel = NULL;
    const NEXUS_VideoDecoderLowLatencySettings * pSettings;
    BXVD_PictureHandlingMode phMode = BXVD_PictureHandlingMode_eDefault;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoDecoder, videoDecoder);
    BDBG_ASSERT(pTSMInfo);

    pSettings = &videoDecoder->extendedSettings.lowLatencySettings;
    stcChannel = videoDecoder->started ? videoDecoder->startSettings.stcChannel : NULL;

    /* copy the old value */
    ptsOffset = videoDecoder->lowLatency.ptsOffset;

    /* Get the parameters for the pix on the queue. */
    BXVD_GetPPBParameterQueueInfo_isr(
        videoDecoder->dec,
        pictures,
        NEXUS_LOW_LATENCY_P_MAX_PICTURES_TO_EXAMINE,
        &queueDepth);

    NEXUS_VideoDecoder_P_GetFrameRateRatio_isr(videoDecoder,
        queueDepth ? pictures[0] : NULL,
        &ratio);

    if (ratio.numerator && ratio.denominator)
    {
        pictureDuration = NEXUS_VideoDecoder_P_ComputeSourcePictureDuration_isr(videoDecoder, &ratio);

        if (stcChannel)
        {
            NEXUS_StcChannel_GetStc_isr(stcChannel, &currentTime);

            /* round up to nearest whole pic */
            targetDepth = (pSettings->latency * ratio.numerator / ratio.denominator + 500) / 1000;

            switch (videoDecoder->extendedSettings.lowLatencySettings.mode)
            {
                case NEXUS_VideoDecoderLowLatencyMode_eFast:
                    if (queueDepth > 0)
                    {
                    phMode = NEXUS_VideoDecoder_P_DoFastLowLatency_isr(
                        videoDecoder,
                        pTSMInfo->eTSMResult,
                        pictures[queueDepth - 1],
                        pictureDuration,
                        queueDepth,
                        targetDepth,
                        &ptsOffset);
                    }
                    break;
                case NEXUS_VideoDecoderLowLatencyMode_eAverage:
                    phMode = NEXUS_VideoDecoder_P_DoAverageLowLatency_isr(
                        videoDecoder,
                        pictureDuration,
                        currentTime,
                        queueDepth,
                        targetDepth,
                        &ptsOffset);
                    break;
                case NEXUS_VideoDecoderLowLatencyMode_eAdaptive:
                    /* round up */
                    minDepth = (pSettings->minLatency * ratio.numerator / ratio.denominator + 500) / 1000;
                    maxDepth = (pSettings->maxLatency * ratio.numerator / ratio.denominator + 500) / 1000;
                    phMode = NEXUS_VideoDecoder_P_DoAdaptiveLowLatency_isr(
                        videoDecoder,
                        pictureDuration,
                        currentTime,
                        queueDepth,
                        targetDepth,
                        minDepth,
                        maxDepth,
                        &ptsOffset);
                    break;
                default:
                    /* do nothing */
                    phMode = BXVD_PictureHandlingMode_eDefault;
                    break;
            }

            pTSMInfo->ePictureHandlingMode = phMode;

            NEXUS_VideoDecoder_P_BoundLowLatencyPtsOffset_isr(videoDecoder, &ptsOffset);

            /* don't bother reapplying if it hasn't changed */
            if (videoDecoder->lowLatency.ptsOffset != ptsOffset)
            {
                LOW_LATENCY_TRACE(("Applying new low latency pts offset: %d", ptsOffset));
                videoDecoder->lowLatency.ptsOffset = ptsOffset;

                rc = NEXUS_VideoDecoder_P_SetPtsOffset_isr(videoDecoder);
                if (rc) { BERR_TRACE(rc); }
            }
        }
        else
        {
            /* TODO: what do we do with no STC? (like ES streams) */
        }
    }
}

static void NEXUS_VideoDecoder_P_TsmResult_isr( void *pParm1, int iParm2, void *pData )
{
    BERR_Code rc;
    BXVD_TSMInfo *pTSMInfo = (BXVD_TSMInfo *)pData;
    NEXUS_VideoDecoderHandle videoDecoder = pParm1;
    const BXVD_PPBParameterInfo * pstPPBParameterInfo;
    uint32_t n;

    BSTD_UNUSED(iParm2);

#if 0
    {
        BXVD_ChannelStatus status;
        rc = BXVD_GetChannelStatus_isr(videoDecoder->dec, &status);
        if (!rc)
        {
            BDBG_ERR(("queue depth: %u", status.ulPictureDeliveryCount));
        }
    }
#endif

    if (videoDecoder->extendedSettings.lowLatencySettings.mode != NEXUS_VideoDecoderLowLatencyMode_eOff)
    {
        NEXUS_VideoDecoder_P_DoLowLatency_isr(videoDecoder, pTSMInfo);
        return;
    }  /* end of if ( low latency mode is enabled ) */

    if (!videoDecoder->trickState.maxFrameRepeat || videoDecoder->trickState.tsmEnabled != NEXUS_TsmMode_eSimulated) {
        return;
    }

    /* Get the parameters of the next picture on the delivery queue. */
    rc = BXVD_GetPPBParameterQueueInfo_isr(videoDecoder->dec, &pstPPBParameterInfo, 1, &n);
    if (rc || !n) {
        return;
    }

    /* If the same picture is sitting at the head of the queue, bump the display count.
    * Note: this is not the picture being displayed, but the easiest way to detect that a picture is being repeated.
    */
    if ( videoDecoder->maxFrameRepeat.pictureId == pstPPBParameterInfo->uiSerialNumber ) {
        videoDecoder->maxFrameRepeat.pictureDisplayCount++;
        if (videoDecoder->maxFrameRepeat.pictureDisplayCount >= videoDecoder->trickState.maxFrameRepeat ) {
            BDBG_MSG(("maxFrameRepeat: id=%d max=%d,%d", pstPPBParameterInfo->uiSerialNumber, videoDecoder->maxFrameRepeat.pictureDisplayCount, videoDecoder->trickState.maxFrameRepeat));
            pTSMInfo->ePictureHandlingMode = BXVD_PictureHandlingMode_eIgnorePTS;
            videoDecoder->maxFrameRepeat.pictureDisplayCount = 0;
        }
    }
    else {
        /* A new picture, reset the values. */
        videoDecoder->maxFrameRepeat.pictureId = pstPPBParameterInfo->uiSerialNumber;
        videoDecoder->maxFrameRepeat.pictureDisplayCount = 0;
    }
}

static void NEXUS_VideoDecoderModule_P_PrintDecoders(NEXUS_VideoDecoderHandle videoDecoder)
{
    unsigned i;
    for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
        NEXUS_VideoFormatInfo info;
        unsigned d, j;
        const NEXUS_VideoDecoderMemory *pmemory = &g_NEXUS_videoDecoderModuleSettings.memory[i];

        if (!pmemory->used) continue;
        NEXUS_VideoFormat_GetInfo(pmemory->maxFormat, &info);
        BDBG_LOG(("VideoDecoder[%d]: AVD%d, maxFormat %4dx%4d, %2d bit decode, %2d bit feeder, AVC51? %c, MVC? %c, %d mosaics at %dx%d",
            i,
            g_NEXUS_videoDecoderModuleSettings.avdMapping[i],
            info.width, info.height,
            pmemory->colorDepth,
            g_cap.videoDecoder[i].feeder.colorDepth,
            pmemory->avc51Supported?'y':'n',
            pmemory->supportedCodecs[NEXUS_VideoCodec_eH264_Mvc]?'y':'n',
            pmemory->mosaic.maxNumber, pmemory->mosaic.maxWidth, pmemory->mosaic.maxHeight));

        for (d=0;d<NEXUS_MAX_XVD_DEVICES;d++) {
            if (!g_NEXUS_videoDecoderXvdDevices[d].xvd) continue;
            for (j=0;j<NEXUS_NUM_XVD_CHANNELS;j++) {
                NEXUS_VideoDecoderHandle vdec = g_NEXUS_videoDecoderXvdDevices[d].channel[j];
                if (!vdec || vdec->parentIndex!=i) continue;
                BDBG_LOG(("  %s as: maxFormat %4dx%4d, %2d bit, AVC51? %c, MVC? %c",
                    vdec == videoDecoder?"FAILED":"opened",
                    vdec->settings.maxWidth, vdec->settings.maxHeight,
                    vdec->settings.colorDepth,
                    vdec->openSettings.openSettings.avc51Enabled?'y':'n',
                    vdec->settings.supportedCodecs[NEXUS_VideoCodec_eH264_Mvc]?'y':'n'
                    ));
            }
        }
    }
}

void NEXUS_VideoDecoderModule_PrintDecoders(void)
{
    NEXUS_VideoDecoderModule_P_PrintDecoders(NULL);
}

static NEXUS_Error NEXUS_VideoDecoder_P_SetMosaicIndex(NEXUS_VideoDecoderHandle videoDecoder)
{
    unsigned avdIndex, i;
    bool used[NEXUS_NUM_XVD_CHANNELS];
    BKNI_Memset(used, 0, sizeof(used));
    /* TODO: rewrite to only search videoDecoder->device and (optionally) videoDecoder->device->slaveLinkedDevice.
    Or, rewrite to do bookkeeping of used mosaicIndex on the master device. */
    for (avdIndex=0; avdIndex<NEXUS_NUM_XVD_DEVICES; avdIndex++) {
        for (i=0;i<NEXUS_NUM_XVD_CHANNELS;i++) {
            NEXUS_VideoDecoderHandle v = g_NEXUS_videoDecoderXvdDevices[avdIndex].channel[i];
            if (!v || !v->mosaicMode) continue;
            if (v->device != videoDecoder->device && v->device != videoDecoder->linkedDevice) continue;
            if (v->dec) used[v->mosaicIndex] = true;
        }
    }
    for (i=0;i<NEXUS_NUM_XVD_CHANNELS;i++) {
        if (!used[i]) break;
    }
    if (i == NEXUS_NUM_XVD_CHANNELS) return BERR_TRACE(NEXUS_UNKNOWN);
    videoDecoder->mosaicIndex = i;
    return NEXUS_SUCCESS;
}

/* we use secure picture buffers if told to, or if that's all we have */
bool nexus_p_use_secure_picbuf(NEXUS_VideoDecoderHandle videoDecoder)
{
    if (videoDecoder->openSettings.openSettings.secureVideo) {
        return true;
    }
    else {
        return (g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].secure == NEXUS_SecureVideo_eSecure);
    }
}

/**
NEXUS_VideoDecoder_P_OpenChannel is called when the VideoDecoder is connected to a Display.
This must happen before Start.
This allows for power down during analog video decode.
It also allows for XVD/VDC memory sharing during analog video decode.
It's important that no BMEM_Alloc occur in NEXUS_VideoDecoder_P_OpenChannel to prevent memory fragmentation.
**/
NEXUS_Error NEXUS_VideoDecoder_P_OpenChannel(NEXUS_VideoDecoderHandle videoDecoder)
{
    BXVD_ChannelSettings channelSettings;
    BERR_Code rc;
    BAVC_VideoCompressionStd magnum_formats[BAVC_VideoCompressionStd_eMax];
    BXVD_DecodeResolution memConfigMaxDecodeResolution;

    BDBG_ASSERT(!videoDecoder->dec);

    rc = BXVD_GetChannelDefaultSettings(videoDecoder->device->xvd, videoDecoder->channelIndex, &channelSettings);
    if (rc) return BERR_TRACE(rc);

    if (videoDecoder->transportType == NEXUS_TransportType_eEs) {
        const char *str = NEXUS_GetEnv("xvd_removal_delay");
        if (str) {
            channelSettings.ulRemovalDelay = NEXUS_atoi(str);
        }
    }

    channelSettings.eChannelMode = BXVD_ChannelMode_eVideo;
    channelSettings.bAVC51Enable = videoDecoder->openSettings.openSettings.avc51Enabled;
    channelSettings.bExcessDirModeEnable = videoDecoder->openSettings.openSettings.excessDirModeEnabled;
    channelSettings.peVideoCmprStdList = magnum_formats;
    NEXUS_VideoDecoder_SetVideoCmprStdList_priv(videoDecoder->settings.supportedCodecs, &channelSettings, BAVC_VideoCompressionStd_eMax);
    /* Swapping width and height turns out to be a common app bug, and with 4K support it kind of works but with undesired memory usage or loss of other features, so flag it. */
    if (videoDecoder->settings.maxHeight == 1920 || videoDecoder->settings.maxHeight == 1280) {
        BDBG_WRN(("unusual width,height of %u,%u resulting in 4K decode resolution", videoDecoder->settings.maxWidth, videoDecoder->settings.maxHeight));
    }
    channelSettings.eDecodeResolution = NEXUS_VideoDecoder_GetDecodeResolution_priv(videoDecoder->settings.maxWidth, videoDecoder->settings.maxHeight);
    memConfigMaxDecodeResolution = NEXUS_VideoDecoder_GetDecodeResolution_priv(videoDecoder->memconfig.maxWidth,videoDecoder->memconfig.maxHeight);
    if (g_decodeResolutionSortOrder[channelSettings.eDecodeResolution] > g_decodeResolutionSortOrder[memConfigMaxDecodeResolution]) {
        BDBG_ERR(("runtime max %dx%d exceeds init memconfig %dx%d", videoDecoder->settings.maxWidth, videoDecoder->settings.maxHeight, videoDecoder->memconfig.maxWidth, videoDecoder->memconfig.maxHeight));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    /* if not dynamicPictureBuffers and not mosaic, and memconfig max is not 4K, don't go less than memconfig max to avoid fragmentation failures */
    if (!g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].dynamicPictureBuffers &&
        g_decodeResolutionSortOrder[memConfigMaxDecodeResolution] < g_decodeResolutionSortOrder[BXVD_DecodeResolution_e4K] &&
        !videoDecoder->mosaicMode)
    {
        if (g_decodeResolutionSortOrder[channelSettings.eDecodeResolution] < g_decodeResolutionSortOrder[memConfigMaxDecodeResolution]) {
            channelSettings.eDecodeResolution = memConfigMaxDecodeResolution;
        }
    }


    /* prevent dual decode if a channel is in exclusive mode: 4K, MVC or 3 or 4 HD mosaic. */
    {
        unsigned num4K = 0, numHD = 0, numMvc = 0, num = 0, i;
        char problem[16] = "";
        for (i=0;i<NEXUS_NUM_XVD_CHANNELS;i++) {
            NEXUS_VideoDecoderHandle v = videoDecoder->device->channel[i];
            BXVD_DecodeResolution res;
            if (!v || (v != videoDecoder && !v->dec)) continue;
            if (v->settings.supportedCodecs[NEXUS_VideoCodec_eH264_Mvc]) numMvc++;
            res = NEXUS_VideoDecoder_GetDecodeResolution_priv(v->settings.maxWidth, v->settings.maxHeight);
            if (res == BXVD_DecodeResolution_e4K) num4K++;
            if (res == BXVD_DecodeResolution_eHD) numHD++;
            num++;
        }
        if (g_NEXUS_videoDecoderModuleSettings.memory[0].mosaic.maxNumber &&
            numHD >= g_NEXUS_videoDecoderModuleSettings.memory[0].mosaic.maxNumber && num > numHD) {
            BKNI_Snprintf(problem, sizeof(problem), "%u HD mosaic", g_NEXUS_videoDecoderModuleSettings.memory[0].mosaic.maxNumber);
        }
        else if ((num4K||numMvc) && num > 1) {
            BKNI_Snprintf(problem, sizeof(problem), num4K?"4K":"MVC");
        }
        if (problem[0]){
            BDBG_ERR(("cannot allocate another decoder on this device while in %s exclusive mode", problem));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }

    if (g_NEXUS_videoDecoderModuleSettings.heapSize[videoDecoder->device->index].secondaryPicture &&
        videoDecoder->settings.supportedCodecs[NEXUS_VideoCodec_eH265])
    {
        channelSettings.bSplitPictureBuffersEnable = true;
    }
    {
        /* if user passes in zero, revert to default */
        unsigned colorDepth = videoDecoder->settings.colorDepth;
        if (videoDecoder->mosaicMode) {
            if (colorDepth > g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].mosaic.colorDepth) {
                colorDepth = g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].mosaic.colorDepth;
            }
        }
        else {
            if (colorDepth > g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].colorDepth) {
                colorDepth = g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].colorDepth;
            }
        }
        channelSettings.b10BitBuffersEnable = colorDepth >= 10;
    }

    {
        BXVD_FWMemConfig memConfig;
        BMMA_Heap_Handle heap;

        rc = BXVD_GetChannelMemoryConfig(videoDecoder->device->xvd, &channelSettings, &memConfig);
        if (rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}

        if (videoDecoder->openSettings.openSettings.pictureHeap) {
            heap = NEXUS_Heap_GetMmaHandle(videoDecoder->openSettings.openSettings.pictureHeap);
        }
        else if (g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].dynamicPictureBuffers || nexus_p_use_secure_picbuf(videoDecoder)) {
            unsigned deviceHeapIndex;
            if (nexus_p_use_secure_picbuf(videoDecoder)) {
                deviceHeapIndex = g_NEXUS_videoDecoderModuleSettings.secure.avdHeapIndex[videoDecoder->device->index];
            }
            else {
                deviceHeapIndex = g_NEXUS_videoDecoderModuleSettings.avdHeapIndex[videoDecoder->device->index];
            }
            heap = g_pCoreHandles->heap[deviceHeapIndex].mma;
            if (!heap) {
                BDBG_ERR(("no picture buffer heap[%u]", deviceHeapIndex));
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }
        }
        else {
            heap = NULL;
        }
        if (heap) {
            channelSettings.uiChannelPictureBlockOffset = 0;
            channelSettings.uiChannelPictureBlockSize = memConfig.uiPictureHeapSize;
            channelSettings.hChannelPictureBlock = BMMA_Alloc(heap, memConfig.uiPictureHeapSize,0,NULL);
            if(channelSettings.hChannelPictureBlock==NULL) {
                return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
            }
        }

        if (memConfig.uiPictureHeap1Size) {
            if(videoDecoder->openSettings.openSettings.secondaryPictureHeap) {
                heap = NEXUS_Heap_GetMmaHandle(videoDecoder->openSettings.openSettings.secondaryPictureHeap);
            }
            else if (g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].dynamicPictureBuffers || nexus_p_use_secure_picbuf(videoDecoder)) {
                unsigned deviceHeapIndex;
                if (nexus_p_use_secure_picbuf(videoDecoder)) {
                    deviceHeapIndex = g_NEXUS_videoDecoderModuleSettings.secure.secondaryPictureHeapIndex[videoDecoder->device->index];
                }
                else {
                    deviceHeapIndex = g_NEXUS_videoDecoderModuleSettings.secondaryPictureHeapIndex[videoDecoder->device->index];
                }
                heap = g_pCoreHandles->heap[deviceHeapIndex].mma;
                if (!heap) {
                    if(channelSettings.hChannelPictureBlock) {
                        BMMA_Free(channelSettings.hChannelPictureBlock);
                    }
                    BDBG_ERR(("no secondary picture buffer heap[%u]", deviceHeapIndex));
                    return BERR_TRACE(NEXUS_NOT_AVAILABLE);
                }
            }
            else {
                heap = NULL;
            }
            if (heap) {
                channelSettings.uiChannelPictureBlockOffset1 = 0;
                channelSettings.uiChannelPictureBlockSize1 = memConfig.uiPictureHeap1Size;
                channelSettings.hChannelPictureBlock1 = BMMA_Alloc(heap, memConfig.uiPictureHeap1Size,0,NULL);
                if(channelSettings.hChannelPictureBlock1==NULL) {
                    if(channelSettings.hChannelPictureBlock) {
                        BMMA_Free(channelSettings.hChannelPictureBlock);
                    }
                    return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
                }
            }
        }
        if( g_NEXUS_videoDecoderModuleSettings.heapSize[videoDecoder->device->index].secure == 0 && g_NEXUS_videoDecoderModuleInternalSettings.secureHeap) {
            channelSettings.uiChannelCabacBlockOffset = 0;
            channelSettings.uiChannelCabacBlockSize = memConfig.uiCabacHeapSize;
            channelSettings.hChannelCabacBlock = BMMA_Alloc(NEXUS_Heap_GetMmaHandle(g_NEXUS_videoDecoderModuleInternalSettings.secureHeap), memConfig.uiCabacHeapSize,0,NULL);
            if(channelSettings.hChannelCabacBlock==NULL) {
                if(channelSettings.hChannelPictureBlock) {
                    BMMA_Free(channelSettings.hChannelPictureBlock);
                }
                if(channelSettings.hChannelPictureBlock1) {
                    BMMA_Free(channelSettings.hChannelPictureBlock1);
                }
                return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
            }
        }
    }
    videoDecoder->decoder.xvd.pictureMemory =  channelSettings.hChannelPictureBlock;
    videoDecoder->decoder.xvd.secondaryPictureMemory =  channelSettings.hChannelPictureBlock1;
    videoDecoder->decoder.xvd.cabacMemory =  channelSettings.hChannelCabacBlock;

    channelSettings.bSVC3DModeEnable = videoDecoder->openSettings.openSettings.svc3dSupported;

    rc = NEXUS_VideoDecoder_P_SetMosaicIndex(videoDecoder);
    if (rc) return BERR_TRACE(rc);

    BDBG_MODULE_MSG(nexus_flow_video_decoder, ("open %p, XVD channel %d", (void *)videoDecoder, videoDecoder->channelIndex));
    rc = BXVD_OpenChannel(videoDecoder->device->xvd, &videoDecoder->dec, videoDecoder->channelIndex, &channelSettings);
    if (rc) {
        if (channelSettings.hChannelCabacBlock) {
            BMMA_Free(channelSettings.hChannelCabacBlock);
        }
        if (channelSettings.hChannelPictureBlock) {
            BMMA_Free(channelSettings.hChannelPictureBlock);
        }
        if (channelSettings.hChannelPictureBlock1) {
            BMMA_Free(channelSettings.hChannelPictureBlock1);
        }
        if (rc == BERR_OUT_OF_DEVICE_MEMORY || rc == BERR_INVALID_PARAMETER) {
            BDBG_LOG(("Cannot open. Memory configuration is the following:"));
            NEXUS_VideoDecoderModule_P_PrintDecoders(videoDecoder);
        }
        return BERR_TRACE(rc);
    }
    rc = BXVD_Userdata_Open(videoDecoder->dec, &videoDecoder->userdata.handle, NULL);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_Userdata_InstallInterruptCallback(videoDecoder->userdata.handle, (BINT_CallbackFunc)NEXUS_VideoDecoder_P_UserdataReady_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    if (videoDecoder->mosaicMode) {
        videoDecoder->device->mosaicCount++;
    }
    if (!videoDecoder->mosaicMode || videoDecoder->device->mosaicCount == 1) {
        BXDM_DisplayInterruptHandler_Handle hXdmDih = NULL;
        unsigned xdmIndex = 0;
        /* supports scalable XDM mapping */
        for(xdmIndex = 0; xdmIndex < BXVD_DisplayInterrupt_eMax; xdmIndex++) {
            if(!videoDecoder->device->xdmInUse[xdmIndex]) {
                /* mark the XDM usage flag when found */
                videoDecoder->device->xdmInUse[xdmIndex] = true;
                videoDecoder->xdmIndex = xdmIndex;
                break;
            }
        }
        if(xdmIndex >= BXVD_DisplayInterrupt_eMax) {
            BDBG_ERR(("MFD %d runs out of XDM!", videoDecoder->mfdIndex));
            rc = BERR_INVALID_PARAMETER;
            goto error;
        }

        hXdmDih = videoDecoder->device->hXdmDih[videoDecoder->xdmIndex];
        if ( NULL == hXdmDih )
        {
           rc = BXVD_InstallDeviceInterruptCallback(videoDecoder->device->xvd,
               BXVD_DeviceInterrupt_ePictureDataReady0 + videoDecoder->xdmIndex,
               NEXUS_VideoDecoder_P_DataReady_isr, videoDecoder, 0);
        }
        else
        {
           rc = BXDM_DisplayInterruptHandler_InstallCallback_PictureDataReadyInterrupt(hXdmDih,
                    NEXUS_VideoDecoder_P_PictureDataReady_isr, videoDecoder, 0);
        }
        if (rc) {rc=BERR_TRACE(rc); goto error;}

        videoDecoder->isInterruptChannel = true;
    }
    else if (videoDecoder->mosaicMode) {
        /* this isn't the first mosaic, so look up the xdmIndex */
        unsigned i;
        for (i=0;i<NEXUS_NUM_XVD_CHANNELS;i++) {
            NEXUS_VideoDecoderHandle v = videoDecoder->device->channel[i];
            if (v && v->mosaicMode && (v != videoDecoder) && v->isInterruptChannel) {
                videoDecoder->xdmIndex = v->xdmIndex;
            }
        }
    }

    rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eRequestSTC, NEXUS_VideoDecoder_P_RequestStc_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_ePTSError, NEXUS_VideoDecoder_P_PtsError_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_ePictureParameters, NEXUS_VideoDecoder_P_PictureParams_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eFirstPTSReady, NEXUS_VideoDecoder_P_FirstPtsReady_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eFirstPTSPassed, NEXUS_VideoDecoder_P_FirstPtsPassed_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eTSMPassInASTMMode, NEXUS_VideoDecoder_P_TsmPass_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_ePtsStcOffset, NEXUS_VideoDecoder_P_PtsStcOffset_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eDecodeError, NEXUS_VideoDecoder_P_DecodeError_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_EnableInterrupt(videoDecoder->dec, BXVD_Interrupt_eDecodeError, true);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eTSMResult, NEXUS_VideoDecoder_P_TsmResult_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_EnableInterrupt(videoDecoder->dec, BXVD_Interrupt_eTSMResult, true);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eClipStart,NEXUS_VideoDecoder_P_FirstPtsPassed_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_ePictureExtensionData, NEXUS_VideoDecoder_P_PictureExtensionData_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eChunkDone, NEXUS_VideoDecoder_P_FnrtChunkDone_isr, videoDecoder, 0);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    /* BXVD_Interrupt_ePictureExtensionData interrupt is enabled in NEXUS_VideoDecoder_P_Start_priv */
    /* we must apply all settings which may have been deferred here */
    rc = NEXUS_VideoDecoder_P_SetSettings(videoDecoder, &videoDecoder->settings, true);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
    rc = NEXUS_VideoDecoder_P_SetCrcFifoSize(videoDecoder, false);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
#if NEXUS_HAS_SYNC_CHANNEL
    rc = NEXUS_VideoDecoder_SetSyncSettings_priv(videoDecoder, &videoDecoder->sync.settings);
    if (rc) {rc=BERR_TRACE(rc); goto error;}
#endif
    NEXUS_VideoDecoder_P_ApplyDisplayInformation(videoDecoder);

    return 0;

error:
    NEXUS_VideoDecoder_P_CloseChannel(videoDecoder);
    BDBG_ASSERT(rc);
    return rc;
}

NEXUS_MemoryBlockHandle NEXUS_VideoDecoder_P_MemoryBlockFromMma(NEXUS_VideoDecoderHandle decoder, BMMA_Block_Handle mma)
{
    unsigned i;
    int available = -1;
    NEXUS_MemoryBlockHandle block;

    if(mma==NULL) {
        return NULL;
    }
    for(i=0;i<sizeof(decoder->memoryBlockHeap)/sizeof(decoder->memoryBlockHeap[0]);i++) {
        if(decoder->memoryBlockHeap[i].mma==mma) {
            return decoder->memoryBlockHeap[i].block;
        } else if(available<0 && decoder->memoryBlockHeap[i].block==NULL) {
            available = (int)i;
        }
    }
    if(available<0) { (void)BERR_TRACE(NEXUS_NOT_AVAILABLE); return NULL; }

    NEXUS_Module_Lock(g_NEXUS_videoDecoderModuleInternalSettings.core);
    block = NEXUS_MemoryBlock_FromMma_priv(mma);
    NEXUS_Module_Unlock(g_NEXUS_videoDecoderModuleInternalSettings.core);
    if(block==NULL) { (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);return NULL; }
    NEXUS_OBJECT_REGISTER(NEXUS_MemoryBlock, block, Acquire);
    decoder->memoryBlockHeap[available].block = block;
    decoder->memoryBlockHeap[available].mma = mma;
    return block;
}

static void NEXUS_VideoDecoder_P_MemoryBlock_FreeAll(NEXUS_VideoDecoderHandle decoder)
{
    unsigned i;

    for(i=0;i<sizeof(decoder->memoryBlockHeap)/sizeof(decoder->memoryBlockHeap[0]);i++) {
        NEXUS_MemoryBlockHandle block=decoder->memoryBlockHeap[i].block;
        if(block) {
            decoder->memoryBlockHeap[i].block=NULL;
            decoder->memoryBlockHeap[i].mma=NULL;
            NEXUS_OBJECT_UNREGISTER(NEXUS_MemoryBlock, block, Release);
            NEXUS_MemoryBlock_Free(block);
        }
    }
    return;
}



void NEXUS_VideoDecoder_P_CloseChannel(NEXUS_VideoDecoderHandle videoDecoder)
{
    BDBG_ASSERT(videoDecoder->dec);

    if (videoDecoder->userdata.handle) {
        BXVD_Userdata_UninstallInterruptCallback(videoDecoder->userdata.handle, (BINT_CallbackFunc)NEXUS_VideoDecoder_P_UserdataReady_isr);
        BXVD_Userdata_Close(videoDecoder->userdata.handle);
        videoDecoder->userdata.handle = NULL;
    }

    (void)NEXUS_VideoDecoder_P_SetCrcFifoSize(videoDecoder, true);

    BXVD_UnInstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_ePTSError);
    BXVD_UnInstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eRequestSTC);
    BXVD_UnInstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_ePictureParameters);
    BXVD_UnInstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eFirstPTSReady);
    BXVD_UnInstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eFirstPTSPassed);
    BXVD_UnInstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_ePtsStcOffset);
    BXVD_UnInstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eDecodeError);
    BXVD_UnInstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eTSMResult);
    BXVD_UnInstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eClipStart);
    BXVD_UnInstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_ePictureExtensionData);
    BXVD_UnInstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_eChunkDone);
    if (videoDecoder->mosaicMode) {
        --videoDecoder->device->mosaicCount;
    }
    if (videoDecoder->isInterruptChannel) {
        BXDM_DisplayInterruptHandler_Handle hXdmDih = NULL;

        hXdmDih = videoDecoder->device->hXdmDih[videoDecoder->xdmIndex];

        if ( NULL == hXdmDih )
        {
           BXVD_UnInstallDeviceInterruptCallback(videoDecoder->device->xvd,
               BXVD_DeviceInterrupt_ePictureDataReady0 + videoDecoder->xdmIndex);
        }
        else
        {
           BXDM_DisplayInterruptHandler_UnInstallCallback_PictureDataReadyInterrupt(hXdmDih);
        }
        videoDecoder->isInterruptChannel = false;

        /* was this a mosaic and are there other mosaics still active? */
        if (videoDecoder->mosaicMode && videoDecoder->device->mosaicCount) {
            unsigned i;
            /* reassign to another mosaic */
            for (i=0;i<NEXUS_NUM_XVD_CHANNELS;i++) {
                NEXUS_VideoDecoderHandle v = videoDecoder->device->channel[i];
                if (v && v->mosaicMode && (v != videoDecoder) && v->dec) {
                    BERR_Code rc;
                    if  ( NULL == hXdmDih ) {
                        rc = BXVD_InstallDeviceInterruptCallback(v->device->xvd,
                            BXVD_DeviceInterrupt_ePictureDataReady0 + videoDecoder->xdmIndex,
                            NEXUS_VideoDecoder_P_DataReady_isr, v, 0);
                    }
                    else {
                        rc = BXDM_DisplayInterruptHandler_InstallCallback_PictureDataReadyInterrupt(hXdmDih,
                            (BXDM_DisplayInterruptHandler_PictureDataReady_isr) NEXUS_VideoDecoder_P_DataReady_isr,
                            v, 0);
                    }
                    if (rc) {rc=BERR_TRACE(rc);} /* fall through */
                    v->isInterruptChannel = true;
                    break;
                }
            }
        }
        else { /* if XDM is uninstalled, clear its usage flag */
            videoDecoder->device->xdmInUse[videoDecoder->xdmIndex] = false;
        }
    }
    NEXUS_VideoDecoder_P_MemoryBlock_FreeAll(videoDecoder);
    BDBG_MODULE_MSG(nexus_flow_video_decoder, ("close %p XVD channel", (void *)videoDecoder));
    BXVD_CloseChannel(videoDecoder->dec);
    videoDecoder->dec = NULL;
    if(videoDecoder->decoder.xvd.pictureMemory) {
        BMMA_Free(videoDecoder->decoder.xvd.pictureMemory);
        videoDecoder->decoder.xvd.pictureMemory=NULL;
    }
    if(videoDecoder->decoder.xvd.secondaryPictureMemory) {
        BMMA_Free(videoDecoder->decoder.xvd.secondaryPictureMemory);
        videoDecoder->decoder.xvd.secondaryPictureMemory=NULL;
    }
    if(videoDecoder->decoder.xvd.cabacMemory) {
        BMMA_Free(videoDecoder->decoder.xvd.cabacMemory);
        videoDecoder->decoder.xvd.cabacMemory=NULL;
    }
}

void NEXUS_VideoDecoder_GetDefaultOpenSettings(NEXUS_VideoDecoderOpenSettings *pOpenSettings)
{
    BKNI_Memset(pOpenSettings, 0, sizeof(*pOpenSettings));
    pOpenSettings->userDataBufferSize = 16 * 1024;
    if (g_NEXUS_videoDecoderModuleSettings.supportedCodecs[NEXUS_VideoCodec_eH264_Svc] ||
        g_NEXUS_videoDecoderModuleSettings.supportedCodecs[NEXUS_VideoCodec_eH264_Mvc])
        {
            pOpenSettings->enhancementPidChannelSupported = true;
    }
}

NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_P_Open_Avd(unsigned index, const NEXUS_VideoDecoderOpenSettings *pOpenSettings)
{
    NEXUS_VideoDecoderOpenMosaicSettings mosaicSettings;
    BKNI_Memset(&mosaicSettings, 0, sizeof(mosaicSettings)); /* these are unused if non-mosaic */
    if (pOpenSettings) {
        mosaicSettings.openSettings = *pOpenSettings;
    }
    else {
        NEXUS_VideoDecoder_GetDefaultOpenSettings(&mosaicSettings.openSettings);
    }
    return NEXUS_VideoDecoder_P_Open(-1, index, &mosaicSettings);
}

void NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(NEXUS_VideoDecoderOpenMosaicSettings *pOpenSettings)
{
    BKNI_Memset(pOpenSettings, 0, sizeof(*pOpenSettings));
    NEXUS_VideoDecoder_GetDefaultOpenSettings(&pOpenSettings->openSettings);
    /* leave maxWidth and maxHeight 0 */
    pOpenSettings->openSettings.enhancementPidChannelSupported = false;
}

NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_P_OpenMosaic_Avd(unsigned parentIndex, unsigned index, const NEXUS_VideoDecoderOpenMosaicSettings *pOpenSettings)
{
    NEXUS_VideoDecoderHandle videoDecoder;
    BERR_Code rc=0;
    if (!pOpenSettings) {
        /* no meaningful default. params required. */
        rc=BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    videoDecoder = NEXUS_VideoDecoder_P_Open(parentIndex, index, pOpenSettings);
    if (videoDecoder) {
        /* deprecated. only set if specified by app. */
        if (pOpenSettings->maxWidth || pOpenSettings->maxHeight) {
            videoDecoder->settings.maxWidth = pOpenSettings->maxWidth;
            videoDecoder->settings.maxHeight = pOpenSettings->maxHeight;
        }
        if (pOpenSettings->linkedDevice.enabled) {
            if (pOpenSettings->linkedDevice.avdIndex >= NEXUS_MAX_XVD_DEVICES) {
                BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto err_link;
            }
            videoDecoder->linkedDevice = &g_NEXUS_videoDecoderXvdDevices[pOpenSettings->linkedDevice.avdIndex];
            if (!videoDecoder->linkedDevice) {
                BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto err_link;
            }
            /* because the XVD is not runtime opened/closed, we can link and never unlink. */
            if (videoDecoder->linkedDevice->slaveLinkedDevice && videoDecoder->linkedDevice->slaveLinkedDevice != videoDecoder->device) {
                BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto err_link;
            }
            videoDecoder->linkedDevice->slaveLinkedDevice = videoDecoder->device;
            BXVD_LinkDecoders(videoDecoder->linkedDevice->xvd, videoDecoder->device->xvd, BXVD_DisplayInterrupt_eZero);
            BDBG_MSG(("linking videoDecoder %u on AVD%u to AVD%u", videoDecoder->index, videoDecoder->device->index, videoDecoder->linkedDevice->index));
        }
    }
    return videoDecoder;

err_link:
    NEXUS_VideoDecoder_Close(videoDecoder);
    return NULL;
}

void NEXUS_VideoDecoder_P_Close_Generic(NEXUS_VideoDecoderHandle videoDecoder)
{
    if (videoDecoder->started) {
        NEXUS_VideoDecoder_Stop(videoDecoder);
    }

    if (videoDecoder->input.destination) {
        BDBG_ERR(("NEXUS_VideoDecoder_Close: closing decoder while still using it as input. You must call NEXUS_VideoWindow_RemoveInput and NEXUS_VideoInput_Shutdown before Close."));
        BDBG_ASSERT(0);
    }
    else if (videoDecoder->input.ref_cnt) {
        BDBG_ERR(("NEXUS_VideoDecoder_Close: closing decoder with active input. You must call NEXUS_VideoInput_Shutdown before Close."));
        BDBG_ASSERT(0);
    }


    (void)NEXUS_VideoDecoder_SetPowerState(videoDecoder, false);

    if (videoDecoder->source_changed_event) {
        BKNI_DestroyEvent(videoDecoder->source_changed_event);
        videoDecoder->source_changed_event = NULL;
    }
    if (videoDecoder->channelChangeReportEventHandler) {
        NEXUS_UnregisterEvent(videoDecoder->channelChangeReportEventHandler);
        videoDecoder->channelChangeReportEventHandler = NULL;
    }
    if (videoDecoder->channelChangeReportEvent) {
        BKNI_DestroyEvent(videoDecoder->channelChangeReportEvent);
        videoDecoder->channelChangeReportEvent = NULL;
    }
    if (videoDecoder->s3DTVStatusEventHandler) {
        NEXUS_UnregisterEvent(videoDecoder->s3DTVStatusEventHandler);
        videoDecoder->s3DTVStatusEventHandler = NULL;
    }
    if (videoDecoder->s3DTVStatusEvent) {
        BKNI_DestroyEvent(videoDecoder->s3DTVStatusEvent);
        videoDecoder->s3DTVStatusEvent = NULL;
    }
    if (videoDecoder->stc.statusChangeEventHandler) {
        NEXUS_UnregisterEvent(videoDecoder->stc.statusChangeEventHandler);
        videoDecoder->stc.statusChangeEventHandler = NULL;
    }
    if (videoDecoder->stc.statusChangeEvent) {
        BKNI_DestroyEvent(videoDecoder->stc.statusChangeEvent);
        videoDecoder->stc.statusChangeEvent = NULL;
    }

    if(videoDecoder->enhancementRave) {
        LOCK_TRANSPORT();
        NEXUS_Rave_Close_priv(videoDecoder->enhancementRave);
        UNLOCK_TRANSPORT();
        videoDecoder->enhancementRave = NULL;
    }

    if (videoDecoder->rave) {
        LOCK_TRANSPORT();
        NEXUS_Rave_Close_priv(videoDecoder->rave);
        UNLOCK_TRANSPORT();
        videoDecoder->rave = NULL;
    }

    if (videoDecoder->userdataCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->userdataCallback);
        videoDecoder->userdataCallback = NULL;
    }
    if (videoDecoder->sourceChangedCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->sourceChangedCallback);
        videoDecoder->sourceChangedCallback = NULL;
    }
    if (videoDecoder->streamChangedCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->streamChangedCallback);
        videoDecoder->streamChangedCallback = NULL;
    }
    if (videoDecoder->private.streamChangedCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->private.streamChangedCallback);
        videoDecoder->private.streamChangedCallback = NULL;
    }
    if (videoDecoder->ptsErrorCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->ptsErrorCallback);
        videoDecoder->ptsErrorCallback = NULL;
    }
    if (videoDecoder->firstPtsCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->firstPtsCallback);
        videoDecoder->firstPtsCallback = NULL;
    }
    if (videoDecoder->firstPtsPassedCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->firstPtsPassedCallback);
        videoDecoder->firstPtsPassedCallback = NULL;
    }
    if (videoDecoder->dataReadyCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->dataReadyCallback);
        videoDecoder->dataReadyCallback = NULL;
    }
    if (videoDecoder->fifoEmpty.callback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->fifoEmpty.callback);
        videoDecoder->fifoEmpty.callback = NULL;
    }
    if (videoDecoder->afdChangedCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->afdChangedCallback);
        videoDecoder->afdChangedCallback = NULL;
    }
    if (videoDecoder->decodeErrorCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->decodeErrorCallback);
        videoDecoder->decodeErrorCallback = NULL;
    }
    if (videoDecoder->s3DTVChangedCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->s3DTVChangedCallback);
        videoDecoder->s3DTVChangedCallback = NULL;
    }
    if (videoDecoder->userdata.buf) {
        BMMA_Unlock(videoDecoder->userdata.mem, videoDecoder->userdata.buf);
        videoDecoder->userdata.buf = NULL;
    }
    if (videoDecoder->userdata.mem) {
        BMMA_Free(videoDecoder->userdata.mem);
        videoDecoder->userdata.mem = NULL;
    }

    if (videoDecoder->playback.firstPtsCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->playback.firstPtsCallback);
        videoDecoder->playback.firstPtsCallback = NULL;
    }
    if (videoDecoder->playback.firstPtsPassedCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->playback.firstPtsPassedCallback);
        videoDecoder->playback.firstPtsPassedCallback = NULL;
    }
    if (videoDecoder->fnrtChunkDoneCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->fnrtChunkDoneCallback);
        videoDecoder->fnrtChunkDoneCallback = NULL;
    }
    if (videoDecoder->stateChangedCallback) {
        NEXUS_IsrCallback_Destroy(videoDecoder->stateChangedCallback);
        videoDecoder->stateChangedCallback = NULL;
    }

    return;
}

void NEXUS_VideoDecoder_P_Close_Avd(NEXUS_VideoDecoderHandle videoDecoder)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    NEXUS_VideoDecoder_P_Close_Generic(videoDecoder);
    videoDecoder->device->channel[videoDecoder->channelIndex] = NULL;

    /* clear the decoder usage status if this is the last reference of that decoder. */
    if(!videoDecoder->mosaicMode) {
        BDBG_ASSERT(g_videoDecoders[videoDecoder->index] == videoDecoder);
        g_videoDecoders[videoDecoder->index] = NULL;
    }

    NEXUS_OBJECT_DESTROY(NEXUS_VideoDecoder, videoDecoder);
    BKNI_Free(videoDecoder);
}

NEXUS_Error NEXUS_VideoDecoder_P_SetSettings_Avd(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    return NEXUS_VideoDecoder_P_SetSettings(videoDecoder, pSettings, false);
}

NEXUS_Error NEXUS_VideoDecoder_P_SetChannelChangeMode(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoder_ChannelChangeMode channelChangeMode)
{
    BXVD_ChannelChangeMode mode;
    BERR_Code rc;

    switch (channelChangeMode) {
    case NEXUS_VideoDecoder_ChannelChangeMode_eMute:
        mode = BXVD_ChannelChangeMode_eMute;
        break;
    case NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock:
        mode = BXVD_ChannelChangeMode_eLastFramePreviousChannel;
        break;
    case NEXUS_VideoDecoder_ChannelChangeMode_eMuteUntilFirstPicture:
        mode = BXVD_ChannelChangeMode_eMuteWithFirstPicturePreview;
        break;
    case NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture:
        mode = BXVD_ChannelChangeMode_eLastFramePreviousWithFirstPicturePreview;
        break;
    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    rc = BXVD_SetChannelChangeMode(videoDecoder->dec, mode);
    if (rc) return BERR_TRACE(rc);
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_VideoDecoder_P_SetSettings(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderSettings *pSettings, bool force)
{
    BERR_Code rc = 0;
    bool setMute = false;
    bool setFreeze = false;
    bool setUserdata = false;
    bool setPtsOffset = false;
    bool setDiscardThreshold = false;
    unsigned i;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    /* i=1 to skip eUnknown */
    for (i=1;i<NEXUS_VideoCodec_eMax;i++) {
        if (pSettings->supportedCodecs[i] && !g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].supportedCodecs[i]) {
            BDBG_ERR(("decoder %d(%d) was not configured at init-time for codec %d", videoDecoder->index, videoDecoder->parentIndex, i));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }

    if (!videoDecoder->dec) {
        goto skip;
    }

    if (force || (pSettings->fnrtSettings.enable != videoDecoder->settings.fnrtSettings.enable) || (pSettings->fnrtSettings.preChargeCount != videoDecoder->settings.fnrtSettings.preChargeCount)) {
        BXVD_PVR_FNRTSettings settings;
        BXVD_PVR_GetDefaultFNRTSettings(videoDecoder->dec, &settings);
        settings.bEnabled = pSettings->fnrtSettings.enable;
        settings.uiPreChargeCount = pSettings->fnrtSettings.preChargeCount;
        rc = BXVD_PVR_SetFNRTSettings(videoDecoder->dec, &settings);
        if (rc) return BERR_TRACE(rc);
    }

    /* convert to single XVD calls. this requires that Nexus init in the same configuration as XVD, or that it asserts its configuration elsewhere */
    if (force || pSettings->freeze != videoDecoder->settings.freeze) {
        setFreeze = true; /* defer to bottom */
    }
    if (pSettings->errorHandling != videoDecoder->settings.errorHandling) {
        BDBG_CASSERT(BXVD_ErrorHandling_ePrognostic == (BXVD_Picture_ErrorHandling)NEXUS_VideoDecoderErrorHandling_ePrognostic);
        rc = BXVD_SetErrorHandlingMode(videoDecoder->dec, (BXVD_Picture_ErrorHandling)pSettings->errorHandling);
        if (rc) return BERR_TRACE(rc);
    }
    if (pSettings->mute != videoDecoder->settings.mute) {
        setMute = true; /* defer to bottom */
    }
    if (force || pSettings->channelChangeMode != videoDecoder->settings.channelChangeMode) {
#if NEXUS_HAS_SYNC_CHANNEL
        /* TODO: currently this only can be changed before start, should be any time */
        videoDecoder->sync.status.lastPictureHeld =
            (pSettings->channelChangeMode == NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture)
            ||
            (pSettings->channelChangeMode == NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock);
        /* TODO: insert sync channel update status callback */
#endif
        rc = NEXUS_VideoDecoder_P_SetChannelChangeMode(videoDecoder, pSettings->channelChangeMode);
        if (rc) return BERR_TRACE(rc);
    }
    if (pSettings->userDataEnabled != videoDecoder->settings.userDataEnabled) {
        setUserdata = true;
    }
    if (force || pSettings->ptsOffset != videoDecoder->settings.ptsOffset) {
        setPtsOffset = true;
    }
    if (videoDecoder->started && (force || (pSettings->discardThreshold != videoDecoder->settings.discardThreshold))) {
        setDiscardThreshold = true;
    }
    if (force || pSettings->dropFieldMode != videoDecoder->settings.dropFieldMode) {
        rc = BXVD_SetPictureDropMode(videoDecoder->dec, pSettings->dropFieldMode ? BXVD_PictureDropMode_eField : BXVD_PictureDropMode_eFrame);
        if (rc) return BERR_TRACE(rc);
    }
    if (force || pSettings->stillContentInterpolationMode != videoDecoder->settings.stillContentInterpolationMode) {
        BDBG_CWARNING(NEXUS_StillContentInterpolationMode_eMax == (NEXUS_StillContentInterpolationMode)BXVD_StillContentInterpolationMode_eMaxModes);
        rc = BXVD_SetInterpolationModeForStillContent(videoDecoder->dec, pSettings->stillContentInterpolationMode);
        if (rc) return BERR_TRACE(rc);
    }
    if (force || pSettings->movingContentInterpolationMode != videoDecoder->settings.movingContentInterpolationMode) {
        BDBG_CWARNING(NEXUS_MovingContentInterpolationMode_eMax == (NEXUS_MovingContentInterpolationMode)BXVD_MovingContentInterpolationMode_eMaxModes);
        rc = BXVD_SetInterpolationModeForMovingContent(videoDecoder->dec, pSettings->movingContentInterpolationMode);
        if (rc) return BERR_TRACE(rc);
    }
    if (force || pSettings->scanMode != videoDecoder->settings.scanMode) {
        BDBG_CWARNING(NEXUS_VideoDecoderScanMode_eMax == (NEXUS_VideoDecoderScanMode)BXVD_1080pScanMode_eMax);
        rc = BXVD_Set1080pScanMode(videoDecoder->dec, pSettings->scanMode);
        if (rc) return BERR_TRACE(rc);
    }
    if(force || pSettings->sourceOrientation != videoDecoder->settings.sourceOrientation || pSettings->customSourceOrientation != videoDecoder->settings.customSourceOrientation) {
        BXVD_3DSetting setting3d;
        BDBG_CASSERT(NEXUS_VideoDecoderSourceOrientation_e2D == (NEXUS_VideoDecoderSourceOrientation)BXVD_Orientation_e2D);
        BDBG_CASSERT(NEXUS_VideoDecoderSourceOrientation_e3D_LeftRight == (NEXUS_VideoDecoderSourceOrientation)BXVD_Orientation_eLeftRight);
        BDBG_CASSERT(NEXUS_VideoDecoderSourceOrientation_e3D_OverUnder == (NEXUS_VideoDecoderSourceOrientation)BXVD_Orientation_eOverUnder);
        BDBG_CASSERT(NEXUS_VideoDecoderSourceOrientation_e3D_LeftRightFullFrame == (NEXUS_VideoDecoderSourceOrientation)BXVD_Orientation_eLeftRightFullFrame);
        BDBG_CASSERT(NEXUS_VideoDecoderSourceOrientation_e3D_RightLeftFullFrame == (NEXUS_VideoDecoderSourceOrientation)BXVD_Orientation_eRightLeftFullFrame);
        BDBG_CASSERT(NEXUS_VideoDecoderSourceOrientation_e3D_LeftRightEnhancedResolution == (NEXUS_VideoDecoderSourceOrientation)BXVD_Orientation_eLeftRightEnhancedResolution);
        BDBG_CWARNING(NEXUS_VideoDecoderSourceOrientation_eMax == (NEXUS_VideoDecoderSourceOrientation)BXVD_Orientation_eMax);
        setting3d.bOverrideOrientation = pSettings->customSourceOrientation;
        setting3d.eOrientation = pSettings->sourceOrientation;
        setting3d.bSetNextPointer = false;
        rc = BXVD_Set3D(videoDecoder->dec,  &setting3d);
        if (rc) return BERR_TRACE(rc);
    }
    if (force || pSettings->horizontalOverscanMode != videoDecoder->settings.horizontalOverscanMode) {
        BDBG_CWARNING(NEXUS_VideoDecoderHorizontalOverscanMode_eMax == (NEXUS_VideoDecoderHorizontalOverscanMode)BXVD_HorizontalOverscanMode_eMax);
        rc = BXVD_SetHorizontalOverscanMode(videoDecoder->dec, pSettings->horizontalOverscanMode);
        if (rc) return BERR_TRACE(rc);
    }
skip:
    NEXUS_IsrCallback_Set(videoDecoder->userdataCallback, &pSettings->appUserDataReady);
    NEXUS_IsrCallback_Set(videoDecoder->sourceChangedCallback, &pSettings->sourceChanged);
    NEXUS_IsrCallback_Set(videoDecoder->streamChangedCallback, &pSettings->streamChanged);
    NEXUS_IsrCallback_Set(videoDecoder->ptsErrorCallback, &pSettings->ptsError);
    NEXUS_IsrCallback_Set(videoDecoder->firstPtsCallback, &pSettings->firstPts);
    NEXUS_IsrCallback_Set(videoDecoder->firstPtsPassedCallback, &pSettings->firstPtsPassed);
    NEXUS_IsrCallback_Set(videoDecoder->fifoEmpty.callback, &pSettings->fifoEmpty);
    NEXUS_IsrCallback_Set(videoDecoder->afdChangedCallback, &pSettings->afdChanged);
    NEXUS_IsrCallback_Set(videoDecoder->decodeErrorCallback, &pSettings->decodeError);
    NEXUS_IsrCallback_Set(videoDecoder->fnrtChunkDoneCallback, &pSettings->fnrtSettings.chunkDone);
    NEXUS_IsrCallback_Set(videoDecoder->stateChangedCallback, &pSettings->stateChanged.callback);

    if ( pSettings->fifoThreshold != videoDecoder->settings.fifoThreshold )
    {
        LOCK_TRANSPORT();

        rc = NEXUS_Rave_SetCdbThreshold_priv(videoDecoder->rave, pSettings->fifoThreshold);

        UNLOCK_TRANSPORT();

        if (rc) return BERR_TRACE(rc);
    }

    videoDecoder->settings = *pSettings;

    if (setMute) {
        /* this setting is shared with SyncChannel, and depends on ch chg mode */
        rc = NEXUS_VideoDecoder_P_SetMute(videoDecoder);
        if (rc) return BERR_TRACE(rc);
    }
    if (setFreeze) {
        /* this setting is shared with SyncChannel, and depends on ch chg mode */
        rc = NEXUS_VideoDecoder_P_SetFreeze(videoDecoder);
        if (rc) return BERR_TRACE(rc);
    }
    if (setUserdata) {
        /* this setting is shared with VideoInput's priv interface */
        rc = NEXUS_VideoDecoder_P_SetUserdata(videoDecoder);
        if (rc) return BERR_TRACE(rc);
    }
    if (setPtsOffset) {
        rc = NEXUS_VideoDecoder_P_SetPtsOffset(videoDecoder);
        if (rc) return BERR_TRACE(rc);
    }
    if (setDiscardThreshold) {
        rc = NEXUS_VideoDecoder_P_SetDiscardThreshold_Avd(videoDecoder);
        if (rc) return BERR_TRACE(rc);
    }

    return NEXUS_SUCCESS;
}

void NEXUS_VideoDecoder_GetDefaultStartSettings(NEXUS_VideoDecoderStartSettings *pStartSettings)
{
    BKNI_Memset(pStartSettings, 0, sizeof(*pStartSettings));
    pStartSettings->codec = NEXUS_VideoCodec_eMpeg2;
    pStartSettings->enhancementPidChannel = NULL;
    pStartSettings->nonRealTime = false;
    return;
}

#define NEXUS_VIDEO_DECODER_P_FIFO_WATCHDOG_MAX_FIFO_STATIC_COUNT 4

static void NEXUS_VideoDecoder_P_UpdateFifoStaticCount_isr(
    NEXUS_VideoDecoderHandle videoDecoder
)
{
    BERR_Code rc;
    BXDM_PictureProvider_PTSInfo ptsInfo;
    unsigned cdbValidPointer, cdbReadPointer;

    /* We can tell if the source has run dry by looking at the RAVE CDB VALID pointer. If it stops moving, then
    either there's no source, or the XVD DM has throttled it (e.g. pause). If XVD DM is throttling, then there must
    be pictures in the queue. If there are no pictures in the queue, then the source has run dry. */
    NEXUS_Rave_GetCdbPointers_isr(videoDecoder->rave, &cdbValidPointer, &cdbReadPointer);
    if (cdbValidPointer == videoDecoder->fifoWatchdog.lastCdbValidPointer
        && cdbReadPointer == videoDecoder->fifoWatchdog.lastCdbReadPointer
        && videoDecoder->pictureDeliveryCount == videoDecoder->fifoWatchdog.lastPictureDeliveryCount)
    {
        if (videoDecoder->fifoWatchdog.staticCount < NEXUS_VIDEO_DECODER_P_FIFO_WATCHDOG_MAX_FIFO_STATIC_COUNT)
        {
            videoDecoder->fifoWatchdog.staticCount++;
        }
    }
    else
    {
        videoDecoder->fifoWatchdog.staticCount = 0;
    }

    if ( videoDecoder->dec )
    {
        rc = BXVD_GetPTS_isr(videoDecoder->dec, &ptsInfo);
    }
    else if ( videoDecoder->xdm.pictureProvider )
    {
        rc = BXDM_PictureProvider_GetCurrentPTSInfo_isr(videoDecoder->xdm.pictureProvider, &ptsInfo);
    }
    else
    {
        rc = BERR_NOT_SUPPORTED;
    }

    if (!rc)
    {
        /* reset static count if PTS is still moving */
        if (videoDecoder->fifoWatchdog.lastPtsValid && ptsInfo.ui32RunningPTS != videoDecoder->fifoWatchdog.lastPts)
        {
            videoDecoder->fifoWatchdog.staticCount = 0;
        }

        videoDecoder->fifoWatchdog.lastPts = ptsInfo.ui32RunningPTS;
        videoDecoder->fifoWatchdog.lastPtsValid = true;
    }

    videoDecoder->fifoWatchdog.lastPictureDeliveryCount = videoDecoder->pictureDeliveryCount;
    videoDecoder->fifoWatchdog.lastCdbValidPointer = cdbValidPointer;
    videoDecoder->fifoWatchdog.lastCdbReadPointer = cdbReadPointer;
}

#define NEXUS_VIDEO_DECODER_P_FIFO_WATCHDOG_EMPTY_THRESHOLD 5
#define NEXUS_VIDEO_DECODER_P_FIFO_WATCHDOG_FULL_THRESHOLD 95
#define NEXUS_VIDEO_DECODER_P_FIFO_WATCHDOG_TIMEOUT 125

#if BDBG_DEBUG_BUILD
static void NEXUS_VideoDecoder_P_CheckStatus(void *context)
{
    NEXUS_VideoDecoderHandle videoDecoder = context;
    BXVD_ChannelStatus channelStatus;
    BERR_Code rc;

    if (++videoDecoder->status.cnt * NEXUS_VIDEO_DECODER_P_FIFO_WATCHDOG_TIMEOUT < 2000) {
        return;
    }
    videoDecoder->status.cnt = 0;

    rc = BXVD_GetChannelStatus(videoDecoder->dec, &channelStatus);
    if (rc) {rc = BERR_TRACE(rc);} /* fall through */
    if (!rc && channelStatus.uiAVDStatusBlock != videoDecoder->status.avdStatusBlock) {
        unsigned i;
        unsigned value;
        static const struct { unsigned value; const char *name; } g_values[] = {
#define MAKE_ENTRY(MACRO) {MACRO,#MACRO}
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_STALLED_ON_INPUT),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_STALLED_ON_PIF),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_STALLED_ON_CABAC_WORKLIST),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_STALLED_ON_IL_WORKLIST),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_STALLED_ON_PPB),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_STALLED_ON_CABAC_BINBUFFER),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_DECODE_STALLED),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_STALLED_ON_USERDATA_BUFFER),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_RAP_NOT_DETECTED),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_UNSUPPORTED_FEATURE_DETECTED),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_IMAGE_SIZE_TOO_BIG),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_BAD_STREAM),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_LESS_MEM_RESTRICT_BUFF),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_DECODE_WARNING),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_INPUT_OVERFLOW),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_DECODE_STEREO_SEQ_ERROR),
            MAKE_ENTRY(BXVD_CHANNELSTATUS_AVD_STATE_DROP_UNSUPP_TEMP_DM)};
        value = videoDecoder->status.avdStatusBlock = channelStatus.uiAVDStatusBlock;
        /* only some avdStatusBlock bits are considered errors. so we only print if at least one is an error.
        but, when we print, we print all bits. */
        if (value & (BXVD_CHANNELSTATUS_AVD_ERROR_MASK)) {
            BDBG_ERR(("decoder %d (%p) has avdStatusBlock errors: %#x", videoDecoder->index, (void *)videoDecoder, videoDecoder->status.avdStatusBlock));
            for (i=0;i<sizeof(g_values)/sizeof(g_values[0]);i++) {
                if (value & g_values[i].value) {
                    if (g_values[i].value & (BXVD_CHANNELSTATUS_AVD_ERROR_MASK)) {
                        BDBG_ERR(("  %s(%#x)", g_values[i].name, g_values[i].value));
                    }
                    else {
                        BDBG_WRN(("  %s(%#x)", g_values[i].name, g_values[i].value));
                    }
                    value &= ~g_values[i].value;
                }
            }
            if (value) {
                BDBG_ERR(("  unknown(%#x)", value));
            }
        }
    }

    /* detect 1080p60 content on a 1080p30 channel */
    if (g_NEXUS_videoDecoderModuleSettings.memory[videoDecoder->parentIndex].maxFormat == NEXUS_VideoFormat_e1080p30hz) {
        /* for interlaced, use same info as NEXUS_VideoDecoderStatus.interlaced */
        if (videoDecoder->last_field.ulSourceHorizontalSize >= 1080 && videoDecoder->last_field.bStreamProgressive) {
            unsigned refreshRate = NEXUS_P_RefreshRate_FromFrameRate_isrsafe(videoDecoder->last_field.eFrameRateCode);
            if (refreshRate >= 59940) {
                BDBG_WRN(("decoder %u has 1080p60 source which exceeds its 1080p30 capability", videoDecoder->index));
            }
        }
    }
}
#endif

/* TODO: hardcoded from 7.5 Fps drop frame rate 1.001/7.5*45000 */
#define NEXUS_VIDEO_DECODER_P_MAX_SOURCE_FRAME_DURATION_45K 6006

#if NEXUS_VIDEO_DECODER_GARBAGE_FIFO_WATCHDOG_SUPPORT
#define NEXUS_VIDEO_DECODER_P_GARBAGE_FIFO_WATCHDOG_ITB_EMPTY_THRESHOLD 3
static void NEXUS_VideoDecoder_P_DetectGarbageFifoDeadlock(NEXUS_VideoDecoderHandle videoDecoder, bool isCdbFull)
{
    bool isItbEmpty;

    LOCK_TRANSPORT();
    isItbEmpty = !NEXUS_Rave_CompareVideoStartCodeCount_priv(videoDecoder->rave,
        NEXUS_VIDEO_DECODER_P_GARBAGE_FIFO_WATCHDOG_ITB_EMPTY_THRESHOLD);
    UNLOCK_TRANSPORT();

    if (videoDecoder->fifoWatchdog.status.isHung && isCdbFull && isItbEmpty)
    {
        BDBG_WRN(("Video garbage watchdog flush"));
        videoDecoder->fifoWatchdog.staticCount = 0;
        NEXUS_VideoDecoder_Flush(videoDecoder);
    }
}
#endif

static void NEXUS_VideoDecoder_P_FifoWatchdog(void *context)
{
    bool decoderRateConsumptionCompatible;
    bool decoderNotConsuming;
    bool isCdbFull;
    bool isItbFull;
    unsigned percentCdbFull, percentItbFull;
    unsigned cdbDepth, cdbSize;
    unsigned itbDepth, itbSize;
    NEXUS_VideoDecoderHandle videoDecoder = context;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_ASSERT(videoDecoder->dec);
    videoDecoder->fifoWatchdog.timer = NULL;

#if BDBG_DEBUG_BUILD
    NEXUS_VideoDecoder_P_CheckStatus(context);
#endif

    /* the decoder should consume at a rate that is compatible with our counter threshold and
    watchdog task period (NRT mode is not normal consumption rate compatible) */
    decoderRateConsumptionCompatible =
        videoDecoder->started
        && videoDecoder->trickState.rate >= NEXUS_NORMAL_DECODE_RATE;

    /* don't bother incrementing if we aren't supposed to be consuming */
    if (decoderRateConsumptionCompatible)
    {
        BKNI_EnterCriticalSection();
        /* checks valid and read pointers and display queue for staticicity */
        NEXUS_VideoDecoder_P_UpdateFifoStaticCount_isr(videoDecoder);
        BKNI_LeaveCriticalSection();
    }

    decoderNotConsuming = videoDecoder->fifoWatchdog.staticCount >= NEXUS_VIDEO_DECODER_P_FIFO_WATCHDOG_MAX_FIFO_STATIC_COUNT;

    NEXUS_StcChannel_GetDefaultDecoderFifoWatchdogStatus_priv(&videoDecoder->fifoWatchdog.status);

    /* hung = not consuming when it should be */
    videoDecoder->fifoWatchdog.status.isHung = decoderRateConsumptionCompatible && decoderNotConsuming;

    BKNI_EnterCriticalSection();
    /* decoder hang detection requires knowledge of buffer depths */
    NEXUS_Rave_GetCdbBufferInfo_isr(videoDecoder->rave, &cdbDepth, &cdbSize);
    BKNI_LeaveCriticalSection();
    NEXUS_Rave_GetItbBufferInfo(videoDecoder->rave, &itbDepth, &itbSize);

    /* fifo threshold can override max fill depth, so we must use it to compare fullness */
    if (videoDecoder->settings.fifoThreshold && videoDecoder->settings.fifoThreshold < cdbSize)
    {
        cdbSize = videoDecoder->settings.fifoThreshold;
    }

    percentCdbFull = cdbSize ? cdbDepth * 100 / cdbSize : 0;
    isCdbFull = percentCdbFull > NEXUS_VIDEO_DECODER_P_FIFO_WATCHDOG_FULL_THRESHOLD;
    percentItbFull = itbSize ? itbDepth * 100 / itbSize : 0;
    isItbFull = percentItbFull > NEXUS_VIDEO_DECODER_P_FIFO_WATCHDOG_FULL_THRESHOLD;
    LOCK_TRANSPORT();
    percentCdbFull = NEXUS_Rave_IsConsumableVideoElementAvailable_priv(videoDecoder->rave) ? percentCdbFull : 0;
    UNLOCK_TRANSPORT();

    videoDecoder->fifoWatchdog.status.percentFull = percentCdbFull;
    videoDecoder->fifoWatchdog.status.frameSyncUnlocked = false;

    /* no point in flushing if we can still take some more data; if either buf is "full", new data will be blocked */
    videoDecoder->fifoWatchdog.isFull = isCdbFull || isItbFull;

    BDBG_MSG_TRACE(("FIFO Watchdog: CDB: %u; ITB: %u", percentCdbFull, percentItbFull));

/*
 * the methods for determining video is locked up due to garbage data in
 * the FIFO break host trick modes, therefore we require a compile time
 * flag to enable them for customers with no such trick modes
 */
#if NEXUS_VIDEO_DECODER_GARBAGE_FIFO_WATCHDOG_SUPPORT
    NEXUS_VideoDecoder_P_DetectGarbageFifoDeadlock(videoDecoder, isCdbFull);
#endif

    if (videoDecoder->startSettings.stcChannel) {
        bool stcRateConsumptionCompatible;
        bool stcFrozen;
        unsigned num, den;

        /* the STC should be running at a rate that is >= 1.0x
        otherwise it might be too slow and cause a false positive */
        NEXUS_StcChannel_GetRate_priv(videoDecoder->startSettings.stcChannel, &num, &den);
        stcFrozen = NEXUS_StcChannel_IsFrozen_priv(videoDecoder->startSettings.stcChannel);
        stcRateConsumptionCompatible = !stcFrozen && (num >= den + 1);

        /*
         * if stc rate is not compatible, we can't say we are hung
         */
        videoDecoder->fifoWatchdog.status.isHung =
            videoDecoder->fifoWatchdog.status.isHung
            &&
            stcRateConsumptionCompatible;

        videoDecoder->fifoWatchdog.status.tsmWait =
            videoDecoder->ptsStcDifference > NEXUS_VIDEO_DECODER_P_MAX_SOURCE_FRAME_DURATION_45K;

        if (!stcRateConsumptionCompatible)
        {
            videoDecoder->fifoWatchdog.staticCount = 0;
        }

        if (videoDecoder->stc.connector)
        {
            LOCK_TRANSPORT();
            NEXUS_StcChannel_ReportDecoderHang_priv(videoDecoder->stc.connector, &videoDecoder->fifoWatchdog.status);
            UNLOCK_TRANSPORT();
        }
    }

    videoDecoder->fifoWatchdog.timer = NEXUS_ScheduleTimer(NEXUS_VIDEO_DECODER_P_FIFO_WATCHDOG_TIMEOUT, NEXUS_VideoDecoder_P_FifoWatchdog, videoDecoder);
}

static void NEXUS_VideoDecoder_P_StcChannelStatusChangeEventHandler(void *context)
{
    NEXUS_VideoDecoderHandle videoDecoder = context;
    NEXUS_StcChannelDecoderConnectionStatus status;

    if (!videoDecoder->stc.connector) return;

    LOCK_TRANSPORT();
    NEXUS_StcChannel_GetDecoderConnectionStatus_priv(videoDecoder->stc.connector, &status);
    UNLOCK_TRANSPORT();

    if (status.flush || status.zeroFill)
    {
        /* reset count if we are doing something about it */
        videoDecoder->fifoWatchdog.staticCount = 0;
    }

    /* if both flush and zero-fill are set, flush first */

    if (status.flush) {
        BDBG_WRN(("Video FIFO watchdog flush"));
        NEXUS_VideoDecoder_Flush(videoDecoder);
        LOCK_TRANSPORT();
        NEXUS_StcChannel_ReportDecoderFlush_priv(videoDecoder->stc.connector);
        UNLOCK_TRANSPORT();
    }

    /* this is a one-shot, like a flush */
    if (status.zeroFill)
    {
        BDBG_WRN(("Video FIFO watchdog zero-fill"));
        BKNI_EnterCriticalSection();
        NEXUS_VideoDecoder_P_SetUnderflowMode_isr(videoDecoder, true);
        BKNI_LeaveCriticalSection();
        LOCK_TRANSPORT();
        NEXUS_StcChannel_ReportDecoderZeroFill_priv(videoDecoder->stc.connector);
        UNLOCK_TRANSPORT();
    }
}

NEXUS_Error NEXUS_VideoDecoder_P_Start_Generic_Prologue(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *pStartSettings)
{

    if (videoDecoder->started) {
        BDBG_ERR(("VideoDecoder already started"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (!pStartSettings->pidChannel) {
        BDBG_ERR(("NEXUS_VideoDecoder_Start requires pid channel"));
        return NEXUS_INVALID_PARAMETER;
    }
    if(pStartSettings->enhancementPidChannel && !videoDecoder->enhancementRave) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (videoDecoder->raveDetached || (pStartSettings->enhancementPidChannel && videoDecoder->enhancementRaveDetached)) {
        BDBG_ERR(("NEXUS_VideoDecoder_Start requires RAVE contexts to be attached"));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    if (!videoDecoder->settings.supportedCodecs[pStartSettings->codec]) {
        BDBG_ERR(("video codec %d not supported. See NEXUS_VideoDecoderSettings.supportedCodecs[].", pStartSettings->codec));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    videoDecoder->startSettings = *pStartSettings;
    videoDecoder->last_field_flag = false;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_VideoDecoder_P_Start_Avd(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *pStartSettings)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BDBG_ASSERT(pStartSettings);

    if (!videoDecoder->dec) {
        if (!videoDecoder->device->defaultConnection.top.intId) {
            BDBG_ERR(("Must call NEXUS_VideoWindow_AddInput before starting VideoDecoder"));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        rc = NEXUS_VideoDecoder_SetPowerState(videoDecoder, true);
        if (rc) return BERR_TRACE(rc);
    }
    if (!videoDecoder->displayConnection.top.intId || videoDecoder->videoAsGraphics) {
        bool skip = false;
        if (!videoDecoder->device->defaultConnection.top.intId) {
            BDBG_ERR(("Must call NEXUS_Display_DriveVideoDecoder before starting video-as-graphics VideoDecoder"));
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        if (videoDecoder->mosaicMode) {
            /* if any other mosaic is started, don't call this again. we could have a combination
            of BVN and videoAsGraphics mosaics. */
            unsigned i;
            for (i=0;i<NEXUS_NUM_XVD_CHANNELS;i++) {
                NEXUS_VideoDecoderHandle v = videoDecoder->device->channel[i];
                if (v && v != videoDecoder && v->parentIndex == videoDecoder->parentIndex && v->dec) {
                    skip = true;
                    break;
                }
            }
        }
        if (!skip) {
            rc = NEXUS_VideoDecoder_SetDisplayConnection_priv_Avd(videoDecoder, NULL);
            if (rc) return BERR_TRACE(rc);
        }
    }

    rc = NEXUS_VideoDecoder_P_Start_Generic_Prologue(videoDecoder, pStartSettings);
    if(rc!=NEXUS_SUCCESS) { return BERR_TRACE(rc); }

    rc = NEXUS_VideoDecoder_P_Start_priv(videoDecoder, pStartSettings, false);
    if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto err_start; }

    rc = NEXUS_VideoDecoder_P_Start_Generic_Epilogue(videoDecoder, pStartSettings);
    if(rc!=NEXUS_SUCCESS) { rc = BERR_TRACE(rc); goto err_start; }

    return NEXUS_SUCCESS;

err_start:
    BKNI_Memset(&videoDecoder->startSettings, 0, sizeof(videoDecoder->startSettings));
    return rc;
}

#if NEXUS_HAS_SYNC_CHANNEL
void NEXUS_VideoDecoder_P_SetCustomSyncPtsOffset(NEXUS_VideoDecoderHandle videoDecoder)
{
    uint32_t customPtsOffset;

    customPtsOffset = /* expected in milliseconds */
        (videoDecoder->primerPtsOffset + videoDecoder->additionalPtsOffset)
        / (NEXUS_IS_DSS_MODE(videoDecoder->transportType) ? 27000 : 45);

    /* TODO: generate a change event */
    videoDecoder->sync.status.customPtsOffset = customPtsOffset;
}
#endif

static void NEXUS_VideoDecoder_P_StopStcChannel(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings * pStartSettings)
{
    NEXUS_StcChannelNonRealtimeSettings stcNonRealtimeSettings;
    NEXUS_Error rc;

    LOCK_TRANSPORT();

    /* set STC to freerunning mode on stop */
    NEXUS_StcChannel_GetDefaultNonRealtimeSettings_priv( &stcNonRealtimeSettings);
    stcNonRealtimeSettings.triggerMode = NEXUS_StcChannelTriggerMode_eTimebase;
    rc = NEXUS_StcChannel_SetNonRealtimeConfig_priv(videoDecoder->startSettings.stcChannel, &stcNonRealtimeSettings, false);
    if (rc) { rc = BERR_TRACE(rc); /* keep going */ }

    if (pStartSettings->pidChannel) {
        NEXUS_StcChannel_DisablePidChannel_priv(pStartSettings->stcChannel, pStartSettings->pidChannel);
    }
    if (pStartSettings->enhancementPidChannel) {
        NEXUS_StcChannel_DisablePidChannel_priv(pStartSettings->stcChannel, pStartSettings->enhancementPidChannel);
    }

    if (videoDecoder->stc.connector) {
        NEXUS_StcChannel_DisconnectDecoder_priv(videoDecoder->stc.connector);
        videoDecoder->stc.connector = NULL;
    }

    UNLOCK_TRANSPORT();
}

static NEXUS_Error NEXUS_VideoDecoder_P_StartStcChannel(
    NEXUS_VideoDecoderHandle videoDecoder,
    const NEXUS_VideoDecoderStartSettings *pStartSettings,
    const NEXUS_StcChannelDecoderConnectionSettings *pStcChannelConnectionSettings,
    bool *pPlayback,
    unsigned *pStcChannelIndex)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_StcChannelSettings stcChannelSettings;

    LOCK_TRANSPORT();

    videoDecoder->stc.connector = NEXUS_StcChannel_ConnectDecoder_priv(pStartSettings->stcChannel, pStcChannelConnectionSettings);
    if (!videoDecoder->stc.connector) { UNLOCK_TRANSPORT(); rc = BERR_TRACE(NEXUS_NOT_SUPPORTED); goto err; }

    if (!videoDecoder->savedRave) {
        if (pStartSettings->pidChannel)
        {
            NEXUS_StcChannel_EnablePidChannel_priv(pStartSettings->stcChannel, pStartSettings->pidChannel);
        }
        if (pStartSettings->enhancementPidChannel) {
            NEXUS_StcChannel_EnablePidChannel_priv(pStartSettings->stcChannel, pStartSettings->enhancementPidChannel);
        }
    }

    NEXUS_StcChannel_GetIndex_priv(pStartSettings->stcChannel, pStcChannelIndex); /* this allows the StcChannels to be swapped */

    UNLOCK_TRANSPORT();

    NEXUS_StcChannel_GetSettings(pStartSettings->stcChannel, &stcChannelSettings);
    /* tell the decoder what type of TSM to do based on NEXUS_StcChannelMode, not transport source */
    *pPlayback = (stcChannelSettings.mode != NEXUS_StcChannelMode_ePcr);

    if (NEXUS_StcChannel_VerifyPidChannel(pStartSettings->stcChannel, pStartSettings->pidChannel)) {
        BDBG_WRN(("StcChannel and PidChannel transportType are not compatible"));
    }

err:
    return rc;
}

NEXUS_Error NEXUS_VideoDecoder_P_Start_Generic_Body(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *pStartSettings, bool otfPvr, BAVC_VideoCompressionStd *pVideoCmprStd, NEXUS_RaveStatus *raveStatus, const NEXUS_StcChannelDecoderConnectionSettings *pStcChannelConnectionSettings, bool *pPlayback, unsigned *pStcChannelIndex)
{
    BERR_Code rc;
    NEXUS_RaveSettings raveSettings;
    NEXUS_PidChannelStatus pidChannelStatus;
    BTRC_TRACE(ChnChange_DecodeStartVideo, START);

    /* clear stuff out */
    BKNI_Memset(&videoDecoder->last_field, 0, sizeof(videoDecoder->last_field));
    videoDecoder->last_field.bMute = true;
    videoDecoder->last_field.ulDecodePictureId = 0xFFFFFFFF;
    videoDecoder->last_getStripedSurfaceSerialNumber = 0xFFFFFFFF;
    BKNI_Memset(&videoDecoder->s3DTVStatus, 0, sizeof(videoDecoder->s3DTVStatus));
    videoDecoder->lastStreamInfo = videoDecoder->streamInfo;
    BKNI_Memset(&videoDecoder->streamInfo, 0, sizeof(videoDecoder->streamInfo));
    videoDecoder->streamInfo.eotf = NEXUS_VideoEotf_eMax;
    BKNI_Memset(&videoDecoder->status, 0, sizeof(videoDecoder->status));
    videoDecoder->dataReadyCount = 0;
    videoDecoder->numPicturesReceivedToFlush = 0;
    videoDecoder->firstPtsPassed = false;
    videoDecoder->firstPtsReady = false;

    NEXUS_PidChannel_GetStatus(pStartSettings->pidChannel, &pidChannelStatus);
    videoDecoder->transportType = pidChannelStatus.originalTransportType; /* saved for later ISR processing */
    videoDecoder->currentUserDataFormat = BUDP_DCCparse_Format_Unknown;
    videoDecoder->useUserDataFormat = false;
    videoDecoder->ptsStcDifference = 0;

    NEXUS_VideoDecoder_P_Trick_Reset_Generic(videoDecoder);

    videoDecoder->pts_error_cnt = 0;
    NEXUS_VideoDecoder_P_FlushFifoEmpty(videoDecoder);
    videoDecoder->fifoWatchdog.staticCount = 0;
    videoDecoder->fifoWatchdog.lastCdbValidPointer = 0;
    videoDecoder->fifoWatchdog.lastCdbReadPointer = 0;
    videoDecoder->overflowCount = 0;


    *pVideoCmprStd = NEXUS_P_VideoCodec_ToMagnum(pStartSettings->codec, videoDecoder->transportType);

    rc = NEXUS_VideoDecoder_P_SetTsm(videoDecoder);
    if (rc) {rc = BERR_TRACE(rc); goto err_set_tsm;}

    *pStcChannelIndex = videoDecoder->mfdIndex; /* this is the default */

    if (pStartSettings->stcChannel) {
        rc = NEXUS_VideoDecoder_P_StartStcChannel(videoDecoder, pStartSettings, pStcChannelConnectionSettings, pPlayback, pStcChannelIndex);
        if (rc) { rc = BERR_TRACE(rc); goto err_set_tsm; }
    }
    else {
        *pPlayback = pidChannelStatus.playback; /* this enabled band hold on vsync playback */
    }

    NEXUS_VideoDecoder_P_SetLowLatencySettings(videoDecoder);

    if (videoDecoder->savedRave) {
        /* the rave context is already configured */
        LOCK_TRANSPORT();
        rc = NEXUS_Rave_GetStatus_priv(videoDecoder->rave, raveStatus);
        UNLOCK_TRANSPORT();
        if (rc) {rc = BERR_TRACE(rc); goto err_get_rave_status;}
    }
    else {
        LOCK_TRANSPORT();
        NEXUS_Rave_GetDefaultSettings_priv(&raveSettings);
        raveSettings.pidChannel = pStartSettings->pidChannel;
        /* 20120725 bandrews - bandHold cannot be enabled if pid channel is from live band */
        raveSettings.bandHold = *pPlayback && pidChannelStatus.playback; /* playback needs band hold, unless it's doing PCR TSM */
        raveSettings.continuityCountEnabled = !*pPlayback;
        raveSettings.disableReordering = (pStartSettings->timestampMode == NEXUS_VideoDecoderTimestampMode_eDecode);
        raveSettings.numOutputBytesEnabled = true; /* rave will poll every 250 msec. this is no worse than SW rave, so we enable always. */
        raveSettings.nonRealTime = pStartSettings->nonRealTime;
        raveSettings.includeRepeatedItbStartCodes = videoDecoder->device->cap.bIncludeRepeatedItbStartCodes;
    #if NEXUS_OTFPVR
        if(otfPvr) {
            raveSettings.otfPvr = true;
        }
    #else
        BSTD_UNUSED(otfPvr);
    #endif
        rc = NEXUS_Rave_ConfigureVideo_priv(videoDecoder->rave, pStartSettings->codec, &raveSettings);
        if (rc==BERR_SUCCESS) {
            rc = NEXUS_Rave_GetStatus_priv(videoDecoder->rave, raveStatus);
        } else {
            rc = BERR_TRACE(rc);
        }
        if(rc==BERR_SUCCESS) {
            if(pStartSettings->enhancementPidChannel) {
                BDBG_ASSERT(videoDecoder->enhancementRave); /* should have verified prior */
                raveSettings.pidChannel = pStartSettings->enhancementPidChannel;
                rc = NEXUS_Rave_ConfigureVideo_priv(videoDecoder->enhancementRave, pStartSettings->codec, &raveSettings);
                if (rc!=BERR_SUCCESS) {
                    rc = BERR_TRACE(rc);
                }
            }
        }
        UNLOCK_TRANSPORT();
        if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc); goto err_config_rave;}
    }

    /* PR42265 20080610 bandrews */
    if (NEXUS_GetEnv("sarnoff_video_delay_workaround") && pStartSettings->codec == NEXUS_VideoCodec_eMpeg2)
    {
        BDBG_MSG(("Applying additional 120 ms to video decoder for MPEG-2 streams"));
        videoDecoder->additionalPtsOffset =
            NEXUS_IS_DSS_MODE(videoDecoder->transportType) ? (120 * 27000) : (120 * 45); /* 120 ms @ 27Mhz (DSS) or 45KHz (MPEG2 TS) */
    }
    else {
        videoDecoder->additionalPtsOffset = 0;
    }

#if NEXUS_HAS_SYNC_CHANNEL
    /* this is for letting sync know about the change to additionalPtsOffset above */
    NEXUS_VideoDecoder_P_SetCustomSyncPtsOffset(videoDecoder);

    /* some (but not all) members of sync status must be reset on start */
    videoDecoder->sync.status.started = true;
    videoDecoder->sync.status.nonRealTime = pStartSettings->nonRealTime;
    /* these come when picparam is available */
    videoDecoder->sync.status.height = 0;
    videoDecoder->sync.status.interlaced = false;
    videoDecoder->sync.status.frameRate = BAVC_FrameRateCode_eUnknown;
    /* these come from ptsStcDiff callback */
    videoDecoder->sync.status.delayValid = false;
    videoDecoder->sync.status.delay = 0;

    /* notify sync - this must be called before BXVD_StartDecode so that SyncChannel is ready for the
    ISR-driven callbacks, which may occur before BXVD_StartDecode returns. */
    if (videoDecoder->sync.settings.startCallback_isr) {
        BKNI_EnterCriticalSection();
        (*videoDecoder->sync.settings.startCallback_isr)(videoDecoder->sync.settings.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }

    if (videoDecoder->sync.startMuted)
    {
        /* re-arm mute if sync asks us to */
        videoDecoder->sync.mute = true;
        BDBG_MSG(("Sync requested to start %p muted", (void *)videoDecoder));
    }

    if (!pStartSettings->nonRealTime && videoDecoder->extendedSettings.lowLatencySettings.mode == NEXUS_VideoDecoderLowLatencyMode_eOff)
    {
        videoDecoder->sync.delay = videoDecoder->sync.startDelay;
        BDBG_MSG(("%p: using sync start delay %u", (void *)videoDecoder, videoDecoder->sync.delay));
        NEXUS_VideoDecoder_P_SetPtsOffset(videoDecoder);
    }

    NEXUS_VideoDecoder_P_SetMute(videoDecoder);
#endif

    return NEXUS_SUCCESS;

err_config_rave:
    LOCK_TRANSPORT();
    NEXUS_Rave_RemovePidChannel_priv(videoDecoder->rave);
    if(pStartSettings->enhancementPidChannel) {
        NEXUS_Rave_RemovePidChannel_priv(videoDecoder->enhancementRave);
    }
    UNLOCK_TRANSPORT();
err_get_rave_status:
err_set_tsm:
    if (pStartSettings->stcChannel) {
        NEXUS_VideoDecoder_P_StopStcChannel(videoDecoder, pStartSettings);
    }
    return rc;
}

NEXUS_Error NEXUS_VideoDecoder_P_Start_Priv_Generic_Epilogue(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *pStartSettings)
{
    NEXUS_VideoDecoder_FlushUserData(videoDecoder);

    LOCK_TRANSPORT();
    NEXUS_Rave_Enable_priv(videoDecoder->rave);
    if(pStartSettings->enhancementPidChannel) {
        BDBG_ASSERT(videoDecoder->enhancementRave);
        NEXUS_Rave_Enable_priv(videoDecoder->enhancementRave);
    }
    UNLOCK_TRANSPORT();

#if NEXUS_HAS_ASTM
    videoDecoder->astm.status.started = true;

    if (videoDecoder->astm.settings.lifecycle_isr)
    {
        BDBG_MSG(("Video channel %p is notifying ASTM of its start action", (void *)videoDecoder));
        BKNI_EnterCriticalSection();
        (*videoDecoder->astm.settings.lifecycle_isr)(videoDecoder->astm.settings.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }
#endif

    BTRC_TRACE(ChnChange_DecodeFirstVideo, START);
    videoDecoder->started = true;
    videoDecoder->maxFrameRepeat.pictureDisplayCount = 0;
    BTRC_TRACE(ChnChange_DecodeStartVideo, STOP);

    /* need to see if preroll -> unmute */
    NEXUS_VideoDecoder_P_SetMute(videoDecoder);

    return NEXUS_SUCCESS;
}

#if !NEXUS_OTFPVR
#define NEXUS_VideoDecoder_P_OtfPvr_Activate(videoDecoder, raveStatus, cfg) (NEXUS_SUCCESS)
#endif

NEXUS_Error NEXUS_VideoDecoder_P_Start_Generic_Epilogue(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *pStartSettings)
{
    NEXUS_OBJECT_ACQUIRE(videoDecoder, NEXUS_PidChannel, pStartSettings->pidChannel);
    NEXUS_PidChannel_ConsumerStarted(pStartSettings->pidChannel); /* needed to unpause playback & stuff like that */

    if(pStartSettings->enhancementPidChannel) {
        NEXUS_OBJECT_ACQUIRE(videoDecoder, NEXUS_PidChannel, pStartSettings->enhancementPidChannel);
        NEXUS_PidChannel_ConsumerStarted(pStartSettings->enhancementPidChannel); /* needed to unpause playback & stuff like that */
    }
    if(pStartSettings->stcChannel) {
        NEXUS_OBJECT_ACQUIRE(videoDecoder, NEXUS_StcChannel, pStartSettings->stcChannel);
    }

    return 0;
}

NEXUS_Error NEXUS_VideoDecoder_P_Start_priv(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *pStartSettings, bool otfPvr)
{
    NEXUS_Error rc;
    BXVD_DecodeSettings cfg;
    NEXUS_StcChannelDecoderConnectionSettings stcChannelConnectionSettings;
    NEXUS_RaveStatus raveStatus;
    NEXUS_RaveStatus enhancementRaveStatus;
    BXVD_DisplayThresholds thresholds;
    unsigned stcChannelIndex;

    BXVD_GetDecodeDefaultSettings(videoDecoder->dec, &cfg);
    LOCK_TRANSPORT();
    NEXUS_StcChannel_GetDefaultDecoderConnectionSettings_priv(&stcChannelConnectionSettings);
    stcChannelConnectionSettings.type = NEXUS_StcChannelDecoderType_eVideo;
    stcChannelConnectionSettings.priority = videoDecoder->stc.priority;
    stcChannelConnectionSettings.getPts_isr = NEXUS_VideoDecoder_P_GetPtsCallback_isr;
    stcChannelConnectionSettings.getCdbLevel_isr = NEXUS_VideoDecoder_P_GetCdbLevelCallback_isr;
    stcChannelConnectionSettings.stcValid_isr = NEXUS_VideoDecoder_P_StcValidCallback_isr;
    if (videoDecoder->mosaicMode || videoDecoder->startSettings.nonRealTime) {
        stcChannelConnectionSettings.setPcrOffset_isr = NEXUS_VideoDecoder_P_SetPcrOffset_isr;
        stcChannelConnectionSettings.getPcrOffset_isr = NEXUS_VideoDecoder_P_GetPcrOffset_isr;
    }
    stcChannelConnectionSettings.pCallbackContext = videoDecoder;
    stcChannelConnectionSettings.statusUpdateEvent = videoDecoder->stc.statusChangeEvent;
    UNLOCK_TRANSPORT();

        /* NRT mode disables XDM Jitter Tolerance Improvement (JTI) since the NRT TSM may have very small dStcPts jitter */
    rc = BXVD_SetJitterToleranceImprovementEnable(videoDecoder->dec, !videoDecoder->startSettings.nonRealTime);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);} /* fall through */

    rc = NEXUS_VideoDecoder_P_Start_Generic_Body(videoDecoder, pStartSettings, otfPvr, &cfg.eVideoCmprStd, &raveStatus, &stcChannelConnectionSettings, &cfg.bPlayback, &stcChannelIndex);
    if(rc!=NEXUS_SUCCESS) {rc = BERR_TRACE(rc); goto err_start_generic_body;}

    /* this needs to happen some time before BXVD_Start */
    videoDecoder->validOutputPic = false;

    /* enable interrupt for 3DTV AVC SEI messages */
    rc = BXVD_EnableInterrupt(videoDecoder->dec, BXVD_Interrupt_ePictureExtensionData, true);
    if (rc) {rc=BERR_TRACE(rc); goto err_enable_interrupt;}

    /* reset internal override of 3D orientation, if and only if app did not specify an orientation */
    if (videoDecoder->settings.customSourceOrientation==false) {
        BXVD_3DSetting xvd3dSetting;
        BXVD_Get3D(videoDecoder->dec, &xvd3dSetting);
        xvd3dSetting.bOverrideOrientation = false;
        xvd3dSetting.eOrientation = BXDM_PictureProvider_Orientation_e2D;
        BXVD_Set3D(videoDecoder->dec, &xvd3dSetting);
    }

    /* PR49215 this needs to be reset on restart */
    rc = BXVD_SetHwPcrOffsetEnable(videoDecoder->dec, !cfg.bPlayback);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc); goto err_hwpcroffset;}

    /* SW7425-4465/SW7425-4682: need to reset SW PCR offset on restart */
    rc = BXVD_SetSwPcrOffset(videoDecoder->dec, 0);
    if (rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc); goto err_swpcroffset;}

    videoDecoder->fifoWatchdog.lastPtsValid = false;
    videoDecoder->fifoWatchdog.timer = NEXUS_ScheduleTimer(NEXUS_VIDEO_DECODER_P_FIFO_WATCHDOG_TIMEOUT, NEXUS_VideoDecoder_P_FifoWatchdog, videoDecoder);

    rc = NEXUS_VideoDecoder_P_SetPtsOffset(videoDecoder);
    if (rc) {rc = BERR_TRACE(rc);goto err_setptsoffset;}

    cfg.eSTC = BXVD_STC_eZero + stcChannelIndex;
    if(cfg.eSTC >= BXVD_STC_eMax) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_invalid_stc;
    }

    /* map the XDM with decoder */
    cfg.eDisplayInterrupt = videoDecoder->xdmIndex + BXVD_DisplayInterrupt_eZero;

#if NEXUS_HAS_ASTM
    /*
     * ASTM mode must be decided before we start, otherwise we won't see any TSM
     * pass interrupts, so we base it on whether TSM is enabled or not on start.
     */
    BDBG_MSG(("%s ASTM mode for video channel %p", videoDecoder->tsm ? "Enabling" : "Disabling", (void *)videoDecoder));
    cfg.bAstmMode = videoDecoder->tsm;
#endif

    NEXUS_VideoDecoder_P_ApplyDisplayInformation(videoDecoder);

    BDBG_CASSERT(BXVD_ErrorHandling_ePrognostic == (BXVD_Picture_ErrorHandling)NEXUS_VideoDecoderErrorHandling_ePrognostic);
    cfg.eErrorHandling = (BXVD_Picture_ErrorHandling)(pStartSettings->errorHandling);
    cfg.uiErrorThreshold = pStartSettings->errorHandlingThreshold;
    videoDecoder->settings.errorHandling = pStartSettings->errorHandling;
    if (pStartSettings->prerollRate > 10)
    {
        if (pStartSettings->prerollRate <= 99)
        {
            cfg.uiPreRollNumerator = pStartSettings->prerollRate;
            cfg.uiPreRollRate = 100;
        }
        else
        {
            BDBG_ERR(("Preroll rates > 99%% are not supported; disabling preroll"));
        }
    }
    else
    {
        cfg.uiPreRollRate = pStartSettings->prerollRate;
    }
    cfg.pContextMap = &raveStatus.xptContextMap;
#if NEXUS_CRC_CAPTURE
    cfg.bCrcMode = true;
#endif
    cfg.bIgnoreDPBOutputDelaySyntax = videoDecoder->extendedSettings.ignoreDpbOutputDelaySyntax;
    cfg.bZeroDelayOutputMode = videoDecoder->extendedSettings.zeroDelayOutputMode;
    cfg.bEarlyPictureDeliveryMode = videoDecoder->extendedSettings.earlyPictureDeliveryMode;
    cfg.bSVC3DModeEnable = videoDecoder->extendedSettings.svc3dEnabled;
    cfg.bIFrameAsRAP = videoDecoder->extendedSettings.treatIFrameAsRap;
    cfg.bIgnoreNumReorderFramesEqZero = videoDecoder->extendedSettings.ignoreNumReorderFramesEqZero;

    if ( pStartSettings->appDisplayManagement )
    {
        cfg.bExternalPictureProviderMode = pStartSettings->appDisplayManagement;
        rc = NEXUS_VideoDecoder_P_InitializeQueue(videoDecoder);
        if ( rc ) { rc=BERR_TRACE(rc); goto err_init_queue; }
    }
    if (videoDecoder->extendedSettings.svc3dEnabled && !videoDecoder->openSettings.openSettings.svc3dSupported) {
        BDBG_ERR(("You must set NEXUS_VideoDecoderOpenSettings.svc3dSupported before setting NEXUS_VideoDecoderExtendedSettings.svc3dEnabled"));
        cfg.bSVC3DModeEnable = false;
    }

    /* XVD frame rate handling is managed in the following priority scheme:
       1. use the frame rate coded in the stream
       2. If that's unknown, use eDefaultFrameRate
       3. If that's unspecified, use FRD logic to determine frame rate
       4. If frame rate cannot be determined, report "unknown" */
    cfg.eFrameRateDetectionMode = BXVD_FrameRateDetectionMode_eStable;
    rc = NEXUS_P_FrameRate_ToMagnum_isrsafe(pStartSettings->frameRate, &cfg.eDefaultFrameRate);
    if(rc!=NEXUS_SUCCESS) {rc = BERR_TRACE(rc); goto err_framerate_tomagnum;}

    if(otfPvr) {
        rc = NEXUS_VideoDecoder_P_OtfPvr_Activate(videoDecoder, &raveStatus, &cfg);
        if(rc!=NEXUS_SUCCESS) {rc = BERR_TRACE(rc); goto err_otfpvr_activate;}
    }

    BDBG_CWARNING(NEXUS_VideoDecoderProgressiveOverrideMode_eMax == (NEXUS_VideoDecoderProgressiveOverrideMode)BXVD_ProgressiveOverrideMode_eMax);
    cfg.eProgressiveOverrideMode = pStartSettings->progressiveOverrideMode;

    BDBG_CWARNING(NEXUS_VideoDecoderTimestampMode_eMax == (NEXUS_VideoDecoderTimestampMode)BXVD_TimestampMode_eMaxModes);
    /* The enums in XVD and FW are swapped, e.g.
       NEXUS_NEXUS_VideoDecoderTimestampMode_eDecode maps to BXVD_TimestampMode_eDisplay.
       The Nexus enums are correct. As long as we're assigning a number to a number, we are ok. */
    cfg.eTimestampMode = pStartSettings->timestampMode;
    if(pStartSettings->enhancementPidChannel) {
        BDBG_ASSERT(videoDecoder->enhancementRave);
        LOCK_TRANSPORT();
        rc = NEXUS_Rave_GetStatus_priv(videoDecoder->enhancementRave, &enhancementRaveStatus);
        UNLOCK_TRANSPORT();
        if(rc!=BERR_SUCCESS) {rc = BERR_TRACE(rc);goto err_rave_status;}
        cfg.aContextMapExtended[0] = &enhancementRaveStatus.xptContextMap;
        cfg.uiContextMapExtNum = 1;
    }
    cfg.bUserDataBTPModeEnable = true;
    cfg.bNRTModeEnable = videoDecoder->startSettings.nonRealTime;
    cfg.uiVDCRectangleNum = videoDecoder->mosaicIndex;

    BDBG_MODULE_MSG(nexus_flow_video_decoder, ("start %p, pidChannel %p, stcChannel %p, codec %d", (void *)videoDecoder, (void *)pStartSettings->pidChannel, (void *)pStartSettings->stcChannel, pStartSettings->codec));
    BTRC_TRACE(ChnChange_DecodeStartVideoXVD, START);
    rc = BXVD_StartDecode(videoDecoder->dec, &cfg);
    BTRC_TRACE(ChnChange_DecodeStartVideoXVD, STOP);
    if (rc) {rc = BERR_TRACE(rc); goto err_startdecode;}

    /* XVD gets a default discard threshold per codec. Capture is here so we can restore it after a trick mode. */
    rc = BXVD_GetDisplayThresholds(videoDecoder->dec, &thresholds);
    if (rc) {rc = BERR_TRACE(rc); goto err_getdisplaythresholds;}
    videoDecoder->defaultDiscardThreshold = thresholds.ui32DiscardThreshold;
    videoDecoder->defaultVeryLateThreshold = thresholds.ui32VeryLateThreshold;

    (void)NEXUS_VideoDecoder_P_SetDiscardThreshold_Avd(videoDecoder);

    rc = NEXUS_VideoDecoder_P_Start_Priv_Generic_Epilogue(videoDecoder, pStartSettings);
    if(rc!=NEXUS_SUCCESS) {rc = BERR_TRACE(rc); goto start_priv_generic_epilogue;}

    NEXUS_VideoDecoder_P_SetUserdata(videoDecoder);

    return rc;

start_priv_generic_epilogue:
err_getdisplaythresholds:
    BXVD_StopDecode(videoDecoder->dec);
err_startdecode:
err_rave_status:
err_otfpvr_activate:
err_framerate_tomagnum:
    if ( pStartSettings->appDisplayManagement )
    {
        void *pPrivateContext = videoDecoder;
        BXDM_DisplayInterruptHandler_Handle displayInterrupt = videoDecoder->device->hXdmDih[videoDecoder->xdmIndex + BXVD_DisplayInterrupt_eZero];
        BXDM_DisplayInterruptHandler_RemovePictureProviderInterface(displayInterrupt, DisplayManagerLite_isr, pPrivateContext);
    }
err_init_queue:
err_invalid_stc:
err_setptsoffset:
    NEXUS_CancelTimer(videoDecoder->fifoWatchdog.timer);
    videoDecoder->fifoWatchdog.timer = NULL;
err_swpcroffset:
err_hwpcroffset:
    BXVD_EnableInterrupt(videoDecoder->dec, BXVD_Interrupt_ePictureExtensionData, false);
err_enable_interrupt:
    NEXUS_VideoDecoder_P_Stop_Priv_Generic_Prologue(videoDecoder);
    NEXUS_VideoDecoder_P_Stop_Priv_Generic_Epilogue(videoDecoder);
    if (pStartSettings->stcChannel) {
        NEXUS_VideoDecoder_P_StopStcChannel(videoDecoder, pStartSettings);
    }
err_start_generic_body:
    return rc;
}

NEXUS_Error NEXUS_VideoDecoder_P_Stop_Generic_Prologue(NEXUS_VideoDecoderHandle videoDecoder)
{
    if (!videoDecoder->started) {
        BDBG_WRN(("video decoder not started"));
        return NEXUS_UNKNOWN;
    }
    return NEXUS_SUCCESS;
}

void NEXUS_VideoDecoder_P_Stop_Generic_Epilogue(NEXUS_VideoDecoderHandle videoDecoder)
{
    BDBG_MSG(("NEXUS_VideoDecoder_P_Stop_Generic_Epilogue:%p stcChannel:%p pidChannel:%p enhancementPidChannel:%p", (void *)videoDecoder, (void *)videoDecoder->startSettings.stcChannel, (void *)videoDecoder->startSettings.pidChannel, (void *)videoDecoder->startSettings.enhancementPidChannel));
    videoDecoder->started = false;
    if(videoDecoder->startSettings.stcChannel) {
        NEXUS_VideoDecoder_P_StopStcChannel(videoDecoder, &videoDecoder->startSettings);
        NEXUS_OBJECT_RELEASE(videoDecoder, NEXUS_StcChannel, videoDecoder->startSettings.stcChannel);
    }
    NEXUS_OBJECT_RELEASE(videoDecoder, NEXUS_PidChannel, videoDecoder->startSettings.pidChannel);
    if(videoDecoder->startSettings.enhancementPidChannel) {
        NEXUS_OBJECT_RELEASE(videoDecoder, NEXUS_PidChannel, videoDecoder->startSettings.enhancementPidChannel);
    }
    BKNI_Memset(&videoDecoder->startSettings, 0, sizeof(videoDecoder->startSettings));
    return;
}

void NEXUS_VideoDecoder_P_Stop_Avd(NEXUS_VideoDecoderHandle videoDecoder)
{
    NEXUS_Error rc;
    bool skipFlushOnStop;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    skipFlushOnStop = videoDecoder->skipFlushOnStop;

    rc = NEXUS_VideoDecoder_P_Stop_Generic_Prologue(videoDecoder);
    if(rc!=NEXUS_SUCCESS) {return;}

    if (!videoDecoder->dec) {
        BDBG_ERR(("NEXUS_VideoDecoder_Stop must be called before NEXUS_VideoWindow_RemoveInput. Please reverse the order of calls in your application."));
        return;
    }
    if(videoDecoder->otfPvr.wasActive) {
        videoDecoder->otfPvr.wasActive = false;
        NEXUS_VideoDecoder_P_SetChannelChangeMode(videoDecoder, videoDecoder->settings.channelChangeMode); /* reapply the channel change mode */
    }
    NEXUS_VideoDecoder_P_Stop_priv(videoDecoder);
#if NEXUS_OTFPVR
    if(videoDecoder->otfPvr.active)  {
        videoDecoder->otfPvr.active = false;
        NEXUS_VideoDecoder_P_OtfPvr_Stop(videoDecoder);
    }
#endif

    /* The primer interface may have replaced the main RAVE context with a primed context. this reverses that. */
    if (videoDecoder->savedRave) {
        NEXUS_VideoDecoderPrimerHandle primer = videoDecoder->primer;
        NEXUS_VideoDecoderPrimer_P_Unlink(videoDecoder->primer, videoDecoder);
        if (!skipFlushOnStop) {
            NEXUS_VideoDecoderPrimer_Stop(primer);
        }

    }
    NEXUS_VideoDecoder_P_Stop_Generic_Epilogue(videoDecoder);
    return;
}


void NEXUS_VideoDecoder_P_Stop_Priv_Generic_Prologue(NEXUS_VideoDecoderHandle videoDecoder)
{
    BTRC_TRACE(ChnChange_Total_Video, START);
    BTRC_TRACE(ChnChange_DecodeStopVideo, START);

    if (videoDecoder->fifoWatchdog.timer) {
        NEXUS_CancelTimer(videoDecoder->fifoWatchdog.timer);
        videoDecoder->fifoWatchdog.timer = NULL;
    }
    if (videoDecoder->s3DTVStatusTimer) {
        NEXUS_CancelTimer(videoDecoder->s3DTVStatusTimer);
        videoDecoder->s3DTVStatusTimer = NULL;
    }
    return;
}

void NEXUS_VideoDecoder_P_Stop_Priv_Generic_Epilogue(NEXUS_VideoDecoderHandle videoDecoder)
{
    LOCK_TRANSPORT();
    NEXUS_Rave_Disable_priv(videoDecoder->rave);
    if (!videoDecoder->skipFlushOnStop) {
        NEXUS_Rave_Flush_priv(videoDecoder->rave);
        NEXUS_Rave_RemovePidChannel_priv(videoDecoder->rave);
    }
    if (videoDecoder->startSettings.enhancementPidChannel) {
        NEXUS_Rave_Disable_priv(videoDecoder->enhancementRave);
        if (!videoDecoder->skipFlushOnStop) {
            NEXUS_Rave_Flush_priv(videoDecoder->enhancementRave);
            NEXUS_Rave_RemovePidChannel_priv(videoDecoder->enhancementRave);
        }
    }
    UNLOCK_TRANSPORT();
    videoDecoder->skipFlushOnStop = false;

#if NEXUS_HAS_ASTM
    videoDecoder->astm.status.started = false;

    if (videoDecoder->astm.settings.lifecycle_isr)
    {
        BDBG_MSG(("Video channel %p is notifying ASTM of its stop action", (void *)videoDecoder));
        BKNI_EnterCriticalSection();
        (*videoDecoder->astm.settings.lifecycle_isr)(videoDecoder->astm.settings.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }
#endif

    videoDecoder->primerPtsOffset = 0;

#if NEXUS_HAS_SYNC_CHANNEL
    videoDecoder->sync.status.started = false;

    /*
     * TODO: This is very bad!! However, we have no way to clear this from within
     * sync in a timely fashion to prevent video from using it when sync attempts
     * to mute on startup.  Sync does have equivalent state info, it's just late.
     */
    videoDecoder->sync.settings.delay = 0;

    /* notify sync */
    if (videoDecoder->sync.settings.startCallback_isr)
    {
        BKNI_EnterCriticalSection();
        (*videoDecoder->sync.settings.startCallback_isr)(videoDecoder->sync.settings.callbackContext, 0);
        BKNI_LeaveCriticalSection();
    }
#endif
    BTRC_TRACE(ChnChange_DecodeStopVideo, STOP);
}

void NEXUS_VideoDecoder_P_Stop_priv(NEXUS_VideoDecoderHandle videoDecoder)
{
    BERR_Code rc;

    NEXUS_VideoDecoder_P_Stop_Priv_Generic_Prologue(videoDecoder);

    NEXUS_VideoDecoder_P_SetUserdata(videoDecoder);

    if ( videoDecoder->startSettings.appDisplayManagement )
    {
        videoDecoder->externalTsm.stopped = true;
        NEXUS_VideoDecoder_P_ReturnOutstandingFrames_Avd(videoDecoder);
    }

    BDBG_MODULE_MSG(nexus_flow_video_decoder, ("stop %p", (void *)videoDecoder));
    rc = BXVD_StopDecode(videoDecoder->dec);
    if (rc) {rc=BERR_TRACE(rc); /* keep going */}

    if ( videoDecoder->startSettings.appDisplayManagement )
    {
        void *pPrivateContext = videoDecoder;
        BXDM_DisplayInterruptHandler_Handle displayInterrupt = videoDecoder->device->hXdmDih[videoDecoder->xdmIndex + BXVD_DisplayInterrupt_eZero];

        BXDM_DisplayInterruptHandler_RemovePictureProviderInterface(displayInterrupt, DisplayManagerLite_isr, pPrivateContext);
    }

    NEXUS_VideoDecoder_P_Stop_Priv_Generic_Epilogue(videoDecoder);

    return;
}

void NEXUS_VideoDecoder_P_GetStatus_Generic(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderStatus *pStatus)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if (videoDecoder->rave) {
        unsigned depth, size;
        NEXUS_RaveStatus raveStatus;
        int rc;

        BKNI_EnterCriticalSection();
        NEXUS_Rave_GetCdbBufferInfo_isr(videoDecoder->rave, &depth, &size);
        BKNI_LeaveCriticalSection();
        pStatus->fifoDepth = depth;
        pStatus->fifoSize = size;
#if 0
        if(size != videoDecoder->cdbLength) { /* number returned by Rave is off by 1 */
            BDBG_WRN(("NEXUS_VideoDecoder_GetStatus:%#x mismatch in CDB size %u:%u", (unsigned)videoDecoder, (unsigned)size, (unsigned)videoDecoder->cdbLength));
        }
#endif
        if(videoDecoder->startSettings.enhancementPidChannel) {
            BKNI_EnterCriticalSection();
            NEXUS_Rave_GetCdbBufferInfo_isr(videoDecoder->enhancementRave, &depth, &size);
            BKNI_LeaveCriticalSection();
            pStatus->enhancementFifoDepth = depth;
            pStatus->enhancementFifoSize = size;
        }

        LOCK_TRANSPORT();
        rc = NEXUS_Rave_GetStatus_priv(videoDecoder->rave, &raveStatus);
        UNLOCK_TRANSPORT();
        if (rc == NEXUS_SUCCESS) {
            if(raveStatus.numOutputBytes >= pStatus->fifoDepth) {
                pStatus->numBytesDecoded = raveStatus.numOutputBytes - pStatus->fifoDepth;
            } else {
                pStatus->numBytesDecoded = 0;
            }
        }
    }

    pStatus->started = videoDecoder->started;
    pStatus->muted = videoDecoder->last_field.bMute;
    pStatus->source.width = videoDecoder->last_field.ulSourceHorizontalSize;
    pStatus->source.height = videoDecoder->last_field.ulSourceVerticalSize;
    if (videoDecoder->firstPtsPassed ||
        !videoDecoder->tsm ||
        (videoDecoder->settings.channelChangeMode != NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture &&
         videoDecoder->settings.channelChangeMode != NEXUS_VideoDecoder_ChannelChangeMode_eMuteUntilFirstPicture))
    {
        /* streamInfo now matches the last_field, so we can mix them. */
        pStatus->coded.width = videoDecoder->streamInfo.codedSourceHorizontalSize;
        pStatus->coded.height = videoDecoder->streamInfo.codedSourceVerticalSize;
    }
    else {
        /* last_field is still reporting the old picture, so we can't mix with current streamInfo. */
        pStatus->coded.width = videoDecoder->lastStreamInfo.codedSourceHorizontalSize;
        pStatus->coded.height = videoDecoder->lastStreamInfo.codedSourceVerticalSize;
    }
    pStatus->display.width = videoDecoder->last_field.ulDisplayHorizontalSize;
    pStatus->display.height = videoDecoder->last_field.ulDisplayVerticalSize;

    pStatus->interlaced = !videoDecoder->last_field.bStreamProgressive;
    pStatus->aspectRatio = NEXUS_P_AspectRatio_FromMagnum_isrsafe(videoDecoder->last_field.eAspectRatio);
    BDBG_CWARNING(BAVC_FrameRateCode_eMax == (BAVC_FrameRateCode)NEXUS_VideoFrameRate_eMax);
    pStatus->frameRate = videoDecoder->last_field.eFrameRateCode;
    pStatus->ptsStcDifference = videoDecoder->ptsStcDifference;
    pStatus->format = NEXUS_P_VideoFormat_FromInfo_isrsafe(pStatus->source.height, pStatus->frameRate, pStatus->interlaced);

    pStatus->ptsErrorCount = videoDecoder->pts_error_cnt;
    pStatus->tsm = videoDecoder->tsm;
    pStatus->firstPtsPassed = videoDecoder->firstPtsPassed;

    switch (videoDecoder->pictureParameterInfo.pictureCoding) {
    case BXVD_PictureCoding_eI: pStatus->pictureCoding = NEXUS_PictureCoding_eI; break;
    case BXVD_PictureCoding_eP: pStatus->pictureCoding = NEXUS_PictureCoding_eP; break;
    case BXVD_PictureCoding_eB: pStatus->pictureCoding = NEXUS_PictureCoding_eB; break;
    default: pStatus->pictureCoding = NEXUS_PictureCoding_eUnknown; break;
    }

    pStatus->fifoEmptyEvents = videoDecoder->fifoEmpty.emptyCount;
    pStatus->fifoNoLongerEmptyEvents = videoDecoder->fifoEmpty.noLongerEmptyCount;

    return;
}

NEXUS_Error NEXUS_VideoDecoder_P_GetStatus_Avd(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderStatus *pStatus)
{
    BXVD_PTSInfo ptsInfo;
    BXVD_GopTimeCode gopTimeCode;
    BXVD_ChannelStatus channelStatus;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    NEXUS_VideoDecoder_P_GetStatus_Generic(videoDecoder, pStatus);

    if (!videoDecoder->dec) return NEXUS_SUCCESS;

    BKNI_EnterCriticalSection();
    rc |= BXVD_GetPTS_isr(videoDecoder->dec, &ptsInfo);
    BKNI_LeaveCriticalSection();
    rc |= BXVD_GetChannelStatus(videoDecoder->dec, &channelStatus);
    rc |= BXVD_GetGopTimeCode(videoDecoder->dec, &gopTimeCode);
    rc |= BXVD_GetPictureTag(videoDecoder->dec, &pStatus->pictureTag);
    /* if a status function failed, we'll fail in the end. but return what we can. */

    pStatus->pts = ptsInfo.ui32RunningPTS;
    pStatus->ptsType = (NEXUS_PtsType)ptsInfo.ePTSType;

    pStatus->queueDepth = channelStatus.ulPictureDeliveryCount;
    pStatus->cabacBinDepth = channelStatus.uiCabacBinDepth;

    pStatus->timeCode.hours = gopTimeCode.ulTimeCodeHours;
    pStatus->timeCode.minutes = gopTimeCode.ulTimeCodeMinutes;
    pStatus->timeCode.seconds = gopTimeCode.ulTimeCodeSeconds;
    pStatus->timeCode.pictures = gopTimeCode.ulTimeCodePictures;

    if ( videoDecoder->started && videoDecoder->startSettings.appDisplayManagement ) {
        pStatus->numDecoded = videoDecoder->externalTsm.numDecoded;
        pStatus->numDisplayed = videoDecoder->externalTsm.numDisplayed;
        pStatus->numIFramesDisplayed = videoDecoder->externalTsm.numIFramesDisplayed;
    }
    else {
    pStatus->numDecoded = channelStatus.uiPicturesDecodedCount;
    pStatus->numDisplayed = channelStatus.uiDisplayedCount;
    pStatus->numIFramesDisplayed = channelStatus.uiIFrameCount;
    }
    pStatus->numDecodeErrors = channelStatus.uiDecodeErrorCount;
    pStatus->numDecodeOverflows = channelStatus.uiDecoderInputOverflow;
    pStatus->numDisplayErrors = channelStatus.uiDisplayedParityFailureCount;
    pStatus->numDecodeDrops = channelStatus.uiDecoderDroppedCount;
    pStatus->avdStatusBlock = channelStatus.uiAVDStatusBlock;
    pStatus->numPicturesReceived = channelStatus.uiPicturesReceivedCount + videoDecoder->numPicturesReceivedToFlush;
    pStatus->numDisplayDrops = channelStatus.uiDisplayManagerDroppedCount;
    pStatus->numDisplayUnderflows = channelStatus.uiVsyncUnderflowCount;
    pStatus->numWatchdogs = videoDecoder->device->numWatchdogs;

    BDBG_CASSERT(BXVD_Video_Protocol_eLevel_MaxLevel == (BXVD_Video_Protocol_Level)NEXUS_VideoProtocolLevel_eMax);
    pStatus->protocolLevel = channelStatus.eProtocolLevel;
    BDBG_CASSERT(BXVD_Video_Protocol_eProfile_MaxProfile == (BXVD_Video_Protocol_Profile)NEXUS_VideoProtocolProfile_eMax);
    pStatus->protocolProfile = channelStatus.eProtocolProfile;

#if NEXUS_OTFPVR
    if(videoDecoder->otfPvr.active)  {
        NEXUS_VideoDecoder_P_OtfPvr_UpdateStatus(videoDecoder, pStatus);
    }
#endif
    return BERR_TRACE(rc);
}

void NEXUS_VideoDecoder_P_Flush_Avd(NEXUS_VideoDecoderHandle videoDecoder)
{
    BERR_Code rc;
    BXVD_ChannelStatus channelStatus;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    if (!videoDecoder->started) return;
    BDBG_ASSERT(videoDecoder->dec);

    if ( videoDecoder->startSettings.appDisplayManagement )
    {
        /* Synchronize with isr */
        BKNI_EnterCriticalSection();
        videoDecoder->externalTsm.stopped = true;
        BKNI_LeaveCriticalSection();
    }

    BDBG_MSG(("flush"));
    rc = BXVD_DisableForFlush(videoDecoder->dec);
    if (rc) {rc=BERR_TRACE(rc);} /* fall through */

    /* BXVD_FlushDecode will clear the uiPicturesReceivedCount. Nexus needs to accumulate this
    total since Start. */
    rc = BXVD_GetChannelStatus(videoDecoder->dec, &channelStatus);
    if (!rc) {
        videoDecoder->numPicturesReceivedToFlush += channelStatus.uiPicturesReceivedCount;
    }

#if NEXUS_OTFPVR
    if(videoDecoder->otfPvr.active)  {
        NEXUS_VideoDecoder_P_OtfPvr_DisableForFlush(videoDecoder);
    }
#endif

    LOCK_TRANSPORT();
    NEXUS_Rave_Disable_priv(videoDecoder->rave);
    NEXUS_Rave_Flush_priv(videoDecoder->rave);
    if(videoDecoder->startSettings.enhancementPidChannel) {
        NEXUS_Rave_Disable_priv(videoDecoder->enhancementRave);
        NEXUS_Rave_Flush_priv(videoDecoder->enhancementRave);
    }
    UNLOCK_TRANSPORT();

    rc |= BXVD_FlushDecode(videoDecoder->dec);
    if (rc) {rc=BERR_TRACE(rc);} /* fall through */
    LOCK_TRANSPORT();
    NEXUS_Rave_Enable_priv(videoDecoder->rave);
    if (videoDecoder->startSettings.enhancementPidChannel) {
        NEXUS_Rave_Enable_priv(videoDecoder->enhancementRave);
    }
    UNLOCK_TRANSPORT();

#if NEXUS_OTFPVR
    if(videoDecoder->otfPvr.active)  {
        NEXUS_VideoDecoder_P_OtfPvr_Flush(videoDecoder);
    }
#endif

    NEXUS_VideoDecoder_FlushUserData(videoDecoder);
    videoDecoder->maxFrameRepeat.pictureDisplayCount = 0;
    BKNI_Memset(&videoDecoder->status, 0, sizeof(videoDecoder->status));
    NEXUS_VideoDecoder_P_FlushFifoEmpty(videoDecoder);

    if ( videoDecoder->startSettings.appDisplayManagement )
    {
        videoDecoder->externalTsm.stopped = false;
    }
}

NEXUS_Error NEXUS_VideoDecoder_P_GetStreamInformation_Avd(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderStreamInformation *pStreamInformation)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    *pStreamInformation = videoDecoder->streamInfo;
    return 0;
}

void NEXUS_VideoDecoder_P_Reset_Avd( NEXUS_VideoDecoderHandle videoDecoder )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_VideoDecoder_P_WatchdogHandler(videoDecoder->device);
}

NEXUS_Error NEXUS_VideoDecoder_P_SetStartPts_Avd( NEXUS_VideoDecoderHandle videoDecoder, uint32_t pts )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    if (videoDecoder->dec && videoDecoder->startSettings.stcChannel) {
        BERR_Code rc;

        BDBG_WRN(("NEXUS_VideoDecoder_SetStartPts %#x", pts));
        rc =BXVD_SetClipTime(videoDecoder->dec, BXVD_ClipTimeType_eClipStartOnly, pts, 0xFFFFFFFF);
        if (rc) return BERR_TRACE(rc);
        rc = BXVD_SetTSMWaitForValidSTC(videoDecoder->dec);
        if (rc) return BERR_TRACE(rc);
    }
    return 0;
}

void NEXUS_VideoDecoder_P_GetPlaybackSettings_Common( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderPlaybackSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    *pSettings = videoDecoder->playback.videoDecoderPlaybackSettings;
}

NEXUS_Error NEXUS_VideoDecoder_P_SetPlaybackSettings_Common( NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderPlaybackSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    NEXUS_IsrCallback_Set(videoDecoder->playback.firstPtsCallback, &pSettings->firstPts);
    NEXUS_IsrCallback_Set(videoDecoder->playback.firstPtsPassedCallback, &pSettings->firstPtsPassed);
    videoDecoder->playback.videoDecoderPlaybackSettings = *pSettings;
    return 0;
}

void NEXUS_VideoDecoder_P_IsCodecSupported_Generic( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoCodec codec, bool *pSupported )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    if (codec >= NEXUS_VideoCodec_eMax) {
        *pSupported = false;
        return;
    }

    *pSupported = videoDecoder->settings.supportedCodecs[codec];
}

#include "bxvd_dbg.h"
NEXUS_Error NEXUS_VideoDecoderModule_SetDebugLog(unsigned avdCore, bool enabled)
{
    NEXUS_Error rc;
    NEXUS_VideoDecoderHandle videoDecoder;

    if (avdCore >= NEXUS_NUM_XVD_DEVICES) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    /* If not connected, then it's not powered up yet, so we can't start the debug log. Don't use BERR_TRACE because this is normal. */
    videoDecoder = g_NEXUS_videoDecoderXvdDevices[avdCore].channel[0];
    if (!videoDecoder || !videoDecoder->dec) {
        return NEXUS_UNKNOWN;
    }

    rc = BXVD_DBG_ControlDecoderDebugLog(g_NEXUS_videoDecoderXvdDevices[avdCore].xvd, enabled?BXVD_DBG_DebugLogging_eStart:BXVD_DBG_DebugLogging_eStop);
    return BERR_TRACE(rc);
}

NEXUS_Error NEXUS_VideoDecoderModule_ReadDebugLog(unsigned avdCore, void *buffer, unsigned bufferSize, unsigned *pAmountRead)
{
    NEXUS_Error rc;
    NEXUS_VideoDecoderHandle videoDecoder;

    if (avdCore >= NEXUS_NUM_XVD_DEVICES || buffer==NULL || pAmountRead==NULL) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    *pAmountRead = 0;

    /* If not connected, then it's not powered up yet, so we can't start the debug log. Don't use BERR_TRACE because this is normal. */
    videoDecoder = g_NEXUS_videoDecoderXvdDevices[avdCore].channel[0];
    if (!videoDecoder || !videoDecoder->dec) {
        return 0;
    }
    rc = BXVD_DBG_ReadDecoderDebugLog(g_NEXUS_videoDecoderXvdDevices[avdCore].xvd, buffer, bufferSize, pAmountRead);
    return BERR_TRACE(rc);
}

NEXUS_Error NEXUS_VideoDecoderModule_SendDebugCommand( unsigned avdCore, const NEXUS_VideoDecoderModuleDebugCommand *pCommand )
{
    NEXUS_Error rc;
    if (avdCore >= NEXUS_NUM_XVD_DEVICES) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    rc = BXVD_DBG_SendDecoderDebugCommand(g_NEXUS_videoDecoderXvdDevices[avdCore].xvd, (char *)pCommand->command);
    return BERR_TRACE(rc);
}

NEXUS_Error NEXUS_VideoDecoder_P_SetPowerState_Avd( NEXUS_VideoDecoderHandle videoDecoder, bool powerUp )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    /* internally, powering up means opening the XVD channel. XVD takes care of the actual power up/down. */

    if (powerUp) {
        NEXUS_Error rc;

        if (!videoDecoder->dec) {
            /* if the user has not call NEXUS_VideoWindow_AddInput at least once, this function will fail.
            however, once they have called it once, it's ok to power up even if not connected. This is quirky and could
            easily change. For that reason, nexus is justing making the call and will allow XVD to fail if it must. */
            rc = NEXUS_VideoDecoder_P_OpenChannel(videoDecoder);
            if (rc) return BERR_TRACE(rc);
        }
    }
    else {
        /* if started, we can automatically stop. */
        if (videoDecoder->started) {
            NEXUS_VideoDecoder_Stop(videoDecoder);
        }

        if (videoDecoder->dec) {
            NEXUS_VideoDecoder_P_CloseChannel(videoDecoder);
        }
    }
    return 0;
}

void NEXUS_VideoDecoder_GetMosaicSettings( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderMosaicSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    *pSettings = videoDecoder->mosaicSettings;
}

NEXUS_Error NEXUS_VideoDecoder_SetMosaicSettings( NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderMosaicSettings *pSettings )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    if (pSettings->maxWidth != videoDecoder->mosaicSettings.maxWidth  ||
        pSettings->maxHeight != videoDecoder->mosaicSettings.maxHeight)
    {
        /* must set before calling NEXUS_VideoDecoder_P_OpenChannel */
        videoDecoder->mosaicSettings = *pSettings;
        videoDecoder->settings.maxWidth = pSettings->maxWidth;
        videoDecoder->settings.maxHeight = pSettings->maxHeight;

        if (videoDecoder->dec) {
            if (videoDecoder->input.destination || videoDecoder->input.ref_cnt) {
                BDBG_ERR(("Cannot change maxWidth/Height while still connected as an input. You must call NEXUS_VideoWindow_RemoveInput and NEXUS_VideoInput_Shutdown beforehand."));
                return BERR_TRACE(NEXUS_NOT_SUPPORTED);
            }

            /* must close and reopen XVD channel */
            NEXUS_VideoDecoder_P_CloseChannel(videoDecoder);
            rc = NEXUS_VideoDecoder_P_OpenChannel(videoDecoder);
            if (rc) return BERR_TRACE(rc);
        }
    }

    /* this redundant set is here in case more settings than maxWidth/Height are added */
    videoDecoder->mosaicSettings = *pSettings;

    return 0;
}

NEXUS_Error NEXUS_VideoDecoderModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    unsigned avdIndex;
    NEXUS_Error rc;

    BSTD_UNUSED(pSettings);

    /* We only check whether any video decoder is started. This forces application to stop decoder
       before entering standby.
       The actual power down is done from display when we remove the input */
    for (avdIndex=0; avdIndex<NEXUS_NUM_XVD_DEVICES; avdIndex++) {
        unsigned channelIndex;
        if (!g_NEXUS_videoDecoderModuleSettings.avdEnabled[avdIndex]) {
            continue;
        }
        for (channelIndex=0; channelIndex<NEXUS_NUM_XVD_CHANNELS; channelIndex++) {
            NEXUS_VideoDecoderHandle videoDecoder;
            videoDecoder = g_NEXUS_videoDecoderXvdDevices[avdIndex].channel[channelIndex];

            if(!videoDecoder) {
                continue;
            }


            if(videoDecoder->started) {
                BDBG_WRN(("Forcing Video Decoder %d:%d Stop for Standby", avdIndex, channelIndex));
                NEXUS_VideoDecoder_Stop(videoDecoder);
            }
        }
    }

    for (avdIndex=0; avdIndex<NEXUS_NUM_XVD_DEVICES; avdIndex++) {
        if (!g_NEXUS_videoDecoderModuleSettings.avdEnabled[avdIndex]) {
            continue;
        }
        if(enabled) {
            rc = BXVD_Standby(g_NEXUS_videoDecoderXvdDevices[avdIndex].xvd);

            NEXUS_VideoDecoder_P_DisableFwVerification( avdIndex ); /* disable region verification */

        } else {
            rc = BXVD_Resume(g_NEXUS_videoDecoderXvdDevices[avdIndex].xvd);
        }
        if (rc) return BERR_TRACE(rc);
    }
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
#endif
    return NEXUS_SUCCESS;
}

NEXUS_RaveHandle NEXUS_VideoDecoder_DetachRaveContext( NEXUS_VideoDecoderHandle videoDecoder )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    if (!videoDecoder->raveDetached) {
        NEXUS_OBJECT_REGISTER(NEXUS_Rave, videoDecoder->rave, Acquire);
        videoDecoder->raveDetached = true;
        return videoDecoder->rave;
    }
    else {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        return NULL;
    }
}

NEXUS_RaveHandle NEXUS_VideoDecoder_DetachEnhancementRaveContext( NEXUS_VideoDecoderHandle videoDecoder )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    if (!videoDecoder->enhancementRaveDetached) {
        NEXUS_OBJECT_REGISTER(NEXUS_Rave, videoDecoder->enhancementRave, Acquire);
        videoDecoder->enhancementRaveDetached = true;
        return videoDecoder->enhancementRave;
    }
    else {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        return NULL;
    }
}

NEXUS_Error NEXUS_VideoDecoder_AttachRaveContext( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_RaveHandle rave )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
    if (videoDecoder->raveDetached && rave == videoDecoder->rave) {
        NEXUS_OBJECT_UNREGISTER(NEXUS_Rave, videoDecoder->rave, Release);
        videoDecoder->raveDetached = false;
        return 0;
    }
    else if (videoDecoder->enhancementRaveDetached && rave == videoDecoder->enhancementRave) {
        NEXUS_OBJECT_UNREGISTER(NEXUS_Rave, videoDecoder->enhancementRave, Release);
        videoDecoder->enhancementRaveDetached = false;
        return 0;
    }
    else {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);;
    }
}

#include "bxvd_ppb.h"

#if BXVD_PPB_EXTENDED
static void NEXUS_VideoDecoder_P_AddCrc_isr( NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderCrc *entry )
{
    if (!videoDecoder->crc.size) return;

    videoDecoder->crc.data[videoDecoder->crc.wptr] = *entry;
    if (++videoDecoder->crc.wptr == videoDecoder->crc.size) {
        videoDecoder->crc.wptr = 0;
    }
    if (videoDecoder->crc.rptr == videoDecoder->crc.wptr) {
        BDBG_WRN(("crc overflow"));
    }
}
#endif

static void NEXUS_VideoDecoder_P_PPBReceived_isr(void *context, int unused, void *pp_ppb)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)context;


#if BXVD_PPB_EXTENDED
    const BXVD_PPB *pPPB = *(BXVD_PPB **)pp_ppb;
    NEXUS_VideoDecoderCrc crc;
#endif

    BSTD_UNUSED(unused);
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);
#if BXVD_PPB_EXTENDED
    crc.top.luma = pPPB->crc.crc_luma;
    crc.top.cr = pPPB->crc.crc_cr;
    crc.top.cb = pPPB->crc.crc_cb;
    crc.bottom.luma = pPPB->crc.crc_luma_bot;
    crc.bottom.cr = pPPB->crc.crc_cr_bot;
    crc.bottom.cb = pPPB->crc.crc_cb_bot;
    NEXUS_VideoDecoder_P_AddCrc_isr(videoDecoder, &crc);
#else
    BSTD_UNUSED(pp_ppb);
#endif
}

NEXUS_Error NEXUS_VideoDecoder_P_SetCrcFifoSize(NEXUS_VideoDecoderHandle videoDecoder, bool forceDisable)
{
    unsigned size = forceDisable?0:videoDecoder->extendedSettings.crcFifoSize;
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    if (size != videoDecoder->crc.size && videoDecoder->crc.size) {
        BKNI_EnterCriticalSection();
        videoDecoder->crc.size = 0;
        BKNI_LeaveCriticalSection();
        BKNI_Free(videoDecoder->crc.data);
    }
    if (size && !videoDecoder->crc.size) {
        videoDecoder->crc.data = BKNI_Malloc(size*sizeof(NEXUS_VideoDecoderCrc));
        if (!videoDecoder->crc.data) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
        BKNI_EnterCriticalSection();
        videoDecoder->crc.size = size;
        videoDecoder->crc.rptr = videoDecoder->crc.wptr = 0;
        BKNI_LeaveCriticalSection();
    }

    if (videoDecoder->dec) {
        if (videoDecoder->crc.size) {
            int rc;
            rc = BXVD_InstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_ePPBReceived, NEXUS_VideoDecoder_P_PPBReceived_isr, videoDecoder, 0);
            if (rc) return BERR_TRACE(rc);
            rc = BXVD_EnableInterrupt(videoDecoder->dec, BXVD_Interrupt_ePPBReceived, true);
            if (rc) return BERR_TRACE(rc);
        }
        else {
            BXVD_UnInstallInterruptCallback(videoDecoder->dec, BXVD_Interrupt_ePPBReceived);
        }
    }

    return 0;
}

NEXUS_Error NEXUS_VideoDecoder_GetCrcData( NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderCrc *pEntries, unsigned numEntries, unsigned *pNumReturned )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    *pNumReturned = 0;
    BKNI_EnterCriticalSection();
    while (numEntries--) {
        if (videoDecoder->crc.rptr != videoDecoder->crc.wptr) {
            pEntries[*pNumReturned] = videoDecoder->crc.data[videoDecoder->crc.rptr];
            (*pNumReturned)++;
            if (++videoDecoder->crc.rptr == videoDecoder->crc.size) {
                videoDecoder->crc.rptr = 0;
            }
        }
    }
    BKNI_LeaveCriticalSection();
    return 0;
}

NEXUS_Error NEXUS_VideoDecoder_P_SetLowLatencySettings(NEXUS_VideoDecoderHandle videoDecoder)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    /* TODO: move as much pre-computation here as possible to reduce time in isr later */
    BSTD_UNUSED(videoDecoder);

    return rc;
}

static void get_stc_count(NEXUS_VideoDecoderCapabilities *pCapabilities)
{
    unsigned stcCount = 0xffffffff;
    unsigned i;

    for (i = 0; i < NEXUS_NUM_XVD_DEVICES; i++)
    {
        if (g_NEXUS_videoDecoderXvdDevices[i].xvd)
        {
            if (g_NEXUS_videoDecoderXvdDevices[i].cap.uiSTC_Count < stcCount)
            {
                stcCount = g_NEXUS_videoDecoderXvdDevices[i].cap.uiSTC_Count;
            }
        }
    }

    if (stcCount == 0xffffffff)
    {
        /* no open AVD hardware to determine caps, revert to xpt count */
        NEXUS_TransportCapabilities tCaps;
        NEXUS_GetTransportCapabilities(&tCaps);
        stcCount = tCaps.numStcs;
    }

    pCapabilities->numStcs = stcCount;
}

void NEXUS_GetVideoDecoderCapabilities( NEXUS_VideoDecoderCapabilities *pCapabilities )
{
    *pCapabilities = g_cap;
}

/* called after module init */
static void NEXUS_P_SetVideoDecoderCapabilities(void)
{
    unsigned i;
    NEXUS_VideoDecoderCapabilities *pCapabilities = &g_cap;

    BKNI_Memset(pCapabilities, 0, sizeof(*pCapabilities));

    for (i=0;i<NEXUS_NUM_VIDEO_DECODERS;i++) {
        if (g_NEXUS_videoDecoderModuleSettings.memory[i].used) {
            ++pCapabilities->numVideoDecoders;
            pCapabilities->videoDecoder[i].avdIndex = g_NEXUS_videoDecoderModuleSettings.avdMapping[i];
            pCapabilities->videoDecoder[i].feeder.index = g_NEXUS_videoDecoderModuleSettings.mfdMapping[i];
            switch (g_pCoreHandles->boxConfig->stVdc.astSource[g_NEXUS_videoDecoderModuleSettings.mfdMapping[i]].eBpp) {
            case BBOX_Vdc_Bpp_e12bit:
                pCapabilities->videoDecoder[i].feeder.colorDepth = 12;
                break;
            case BBOX_Vdc_Bpp_e10bit:
                pCapabilities->videoDecoder[i].feeder.colorDepth = 10;
                break;
            default:
                pCapabilities->videoDecoder[i].feeder.colorDepth = 8;
                break;
            }
        }
    }
    get_stc_count(pCapabilities);
#if NEXUS_NUM_DSP_VIDEO_DECODERS
    pCapabilities->dspVideoDecoder.total = NEXUS_NUM_DSP_VIDEO_DECODERS;
    pCapabilities->dspVideoDecoder.baseIndex = NEXUS_NUM_VIDEO_DECODERS;
    pCapabilities->dspVideoDecoder.useForVp6 = true;
#endif
#if NEXUS_NUM_SID_VIDEO_DECODERS
    pCapabilities->sidVideoDecoder.total = NEXUS_NUM_SID_VIDEO_DECODERS;
    pCapabilities->sidVideoDecoder.baseIndex = NEXUS_NUM_VIDEO_DECODERS;
#if NEXUS_NUM_DSP_VIDEO_DECODERS
    pCapabilities->sidVideoDecoder.baseIndex += NEXUS_NUM_DSP_VIDEO_DECODERS;
#endif
    pCapabilities->sidVideoDecoder.useForMotionJpeg = true;
#endif

#if NEXUS_NUM_SOFT_VIDEO_DECODERS
    pCapabilities->softVideoDecoder.total = NEXUS_NUM_SOFT_VIDEO_DECODERS;
    pCapabilities->softVideoDecoder.baseIndex = NEXUS_NUM_VIDEO_DECODERS;
#if NEXUS_NUM_DSP_VIDEO_DECODERS
    pCapabilities->softVideoDecoder.baseIndex += NEXUS_NUM_DSP_VIDEO_DECODERS;
#endif
#if NEXUS_NUM_SID_VIDEO_DECODERS
    pCapabilities->softVideoDecoder.baseIndex += NEXUS_NUM_SID_VIDEO_DECODERS;
#endif
    pCapabilities->softVideoDecoder.useForVp9 = true;
#endif /* NEXUS_NUM_SOFT_VIDEO_DECODERS */

    BKNI_Memcpy(pCapabilities->memory, g_NEXUS_videoDecoderModuleSettings.memory, sizeof(pCapabilities->memory));
    BKNI_Memcpy(pCapabilities->stillMemory, g_NEXUS_videoDecoderModuleSettings.stillMemory, sizeof(pCapabilities->stillMemory));
}


#define XVD_FILTER

static void DML_P_Prepare_MFD_Struct_isr(NEXUS_VideoDecoderExternalTsmData *pVideoDecoderLite, const BXDM_DisplayInterruptInfo *pstDisplayInterruptInfo, BAVC_MFD_Picture **pMFDPicture, BXDM_Picture *pstDispPicture, BAVC_Polarity SrcPolarity)
{
    BAVC_MFD_Picture *pCurrentMFDPicture;
    *pMFDPicture = &pVideoDecoderLite->pMFDPicture[0];
    pCurrentMFDPicture = *pMFDPicture;

    if (pstDispPicture)
    {
        pCurrentMFDPicture->bFrameProgressive = ( pstDispPicture->stBufferInfo.eSourceFormat == BXDM_Picture_SourceFormat_eProgressive);
        pCurrentMFDPicture->bStreamProgressive = ( BXDM_Picture_Sequence_eProgressive == pstDispPicture->stPictureType.eSequence );

        if (pstDispPicture->stAspectRatio.bValid) {
            pCurrentMFDPicture->eAspectRatio = pstDispPicture->stAspectRatio.eAspectRatio;
        }
        else {
            pCurrentMFDPicture->eAspectRatio = NEXUS_AspectRatio_eUnknown;
        }
        if (!pstDispPicture->stFrameRate.bValid || pstDispPicture->stFrameRate.stRate.uiNumerator == 0 || pstDispPicture->stFrameRate.stRate.uiDenominator == 0) {
            pCurrentMFDPicture->eFrameRateCode = NEXUS_VideoFrameRate_eUnknown;
        }
        else {
            NEXUS_VideoFrameRate nxFrameRate;
            NEXUS_P_FrameRate_FromRefreshRate_isrsafe((1000 * pstDispPicture->stFrameRate.stRate.uiNumerator) / pstDispPicture->stFrameRate.stRate.uiDenominator, &nxFrameRate);
            pCurrentMFDPicture->eFrameRateCode = nxFrameRate;
        }

        pCurrentMFDPicture->eInterruptPolarity = pstDisplayInterruptInfo->eInterruptPolarity;
        pCurrentMFDPicture->eSourcePolarity = SrcPolarity;

        pCurrentMFDPicture->uiSampleAspectRatioX = pstDispPicture->stAspectRatio.uiSampleAspectRatioX;
        pCurrentMFDPicture->uiSampleAspectRatioY = pstDispPicture->stAspectRatio.uiSampleAspectRatioY;

        pCurrentMFDPicture->ulSourceHorizontalSize = pstDispPicture->stBufferInfo.stSource.uiWidth;
        pCurrentMFDPicture->ulSourceVerticalSize = pstDispPicture->stBufferInfo.stSource.uiHeight;

        if ( true == pstDispPicture->stClipping.bValid )
        {
            pCurrentMFDPicture->ulSourceHorizontalSize -= (pstDispPicture->stClipping.uiLeft +pstDispPicture->stClipping.uiRight);
            pCurrentMFDPicture->ulSourceVerticalSize -= (pstDispPicture->stClipping.uiTop + pstDispPicture->stClipping.uiBottom);

            pCurrentMFDPicture->ulSourceClipTop = pstDispPicture->stClipping.uiTop;
            pCurrentMFDPicture->ulSourceClipLeft = pstDispPicture->stClipping.uiLeft;
        }
        else
        {

            pCurrentMFDPicture->ulSourceClipTop = 0;
            pCurrentMFDPicture->ulSourceClipLeft = 0;
        }

        pCurrentMFDPicture->i32_HorizontalPanScan = 0;
        pCurrentMFDPicture->i32_VerticalPanScan = 0;


        pCurrentMFDPicture->ulDisplayHorizontalSize = pCurrentMFDPicture->ulSourceHorizontalSize; /* needs to be the post cropped value */
        pCurrentMFDPicture->ulDisplayVerticalSize = pCurrentMFDPicture->ulSourceVerticalSize; /* needs to be the post cropped value */


        if (pstDispPicture->stPOC.bValid == true)
        {
            pCurrentMFDPicture->ulIdrPicID = pstDispPicture->stPOC.uiPictureId;
            /*    pCurrentMFDPicture->int32_PicOrderCnt;  this line has no effect */

            if ( true == pstDispPicture->stPOC.stPictureOrderCount[pCurrentMFDPicture->eSourcePolarity].bValid )
            {
                pCurrentMFDPicture->int32_PicOrderCnt = pstDispPicture->stPOC.stPictureOrderCount[pCurrentMFDPicture->eSourcePolarity].iValue;
            }
            else if ( true == pstDispPicture->stPOC.stPictureOrderCount[BAVC_Polarity_eFrame].bValid )
            {
                pCurrentMFDPicture->int32_PicOrderCnt = pstDispPicture->stPOC.stPictureOrderCount[BAVC_Polarity_eFrame].iValue;
            }
            else if ( true == pstDispPicture->stPOC.stPictureOrderCount[BAVC_Polarity_eTopField].bValid )
            {
                pCurrentMFDPicture->int32_PicOrderCnt = pstDispPicture->stPOC.stPictureOrderCount[BAVC_Polarity_eTopField].iValue;
            }
            else if ( true == pstDispPicture->stPOC.stPictureOrderCount[BAVC_Polarity_eBotField].bValid )
            {
                pCurrentMFDPicture->int32_PicOrderCnt = pstDispPicture->stPOC.stPictureOrderCount[BAVC_Polarity_eBotField].iValue;
            }
        }


        /* Defaults */
        pCurrentMFDPicture->bIgnoreCadenceMatch = false;
        pCurrentMFDPicture->bMute = false;
        pCurrentMFDPicture->bCaptureCrc = false;
        pCurrentMFDPicture->pNext = NULL;
        pCurrentMFDPicture->ulChannelId = 0;
        pCurrentMFDPicture->bPictureRepeatFlag = 0;


        pCurrentMFDPicture->bValidAfd = pstDispPicture->stActiveFormatDescription.bValid;
        pCurrentMFDPicture->ulAfd = pstDispPicture->stActiveFormatDescription.uiValue;

        pCurrentMFDPicture->ulAdjQp = pstDispPicture->stDigitalNoiseReduction.uiAdjustedQuantizationParameter;

        if ( BAVC_Polarity_eFrame == pCurrentMFDPicture->eSourcePolarity)
        {
            pCurrentMFDPicture->eChrominanceInterpolationMode  = BAVC_InterpolationMode_eFrame;
        }
        else
        {
            pCurrentMFDPicture->eChrominanceInterpolationMode  = BAVC_InterpolationMode_eField;
        }

        pCurrentMFDPicture->eColorPrimaries = pstDispPicture->stDisplayInfo.eColorPrimaries;
        pCurrentMFDPicture->eMatrixCoefficients = pstDispPicture->stDisplayInfo.eMatrixCoefficients;
        pCurrentMFDPicture->eTransferCharacteristics = pstDispPicture->stDisplayInfo.eTransferCharacteristics;


        if ( true == pstDispPicture->stBufferInfo.stChromaLocation[pCurrentMFDPicture->eSourcePolarity].bValid )
        {
            pCurrentMFDPicture->eMpegType = pstDispPicture->stBufferInfo.stChromaLocation[pCurrentMFDPicture->eSourcePolarity].eMpegType;
        }
        else if ( true ==pstDispPicture->stBufferInfo.stChromaLocation[BAVC_Polarity_eFrame].bValid )
        {
            pCurrentMFDPicture->eMpegType = pstDispPicture->stBufferInfo.stChromaLocation[BAVC_Polarity_eFrame].eMpegType;
        }
        else if ( true == pstDispPicture->stBufferInfo.stChromaLocation[BAVC_Polarity_eTopField].bValid )
        {
            pCurrentMFDPicture->eMpegType = pstDispPicture->stBufferInfo.stChromaLocation[BAVC_Polarity_eTopField].eMpegType;
        }
        else if ( true == pstDispPicture->stBufferInfo.stChromaLocation[BAVC_Polarity_eBotField].bValid )
        {
            pCurrentMFDPicture->eMpegType = pstDispPicture->stBufferInfo.stChromaLocation[BAVC_Polarity_eBotField].eMpegType;
        }
        else
        {
            pCurrentMFDPicture->eMpegType = BAVC_MpegType_eMpeg2;
        }

        pCurrentMFDPicture->eStripeWidth = pstDispPicture->stBufferInfo.eStripeWidth;
        pCurrentMFDPicture->eYCbCrType = pstDispPicture->stBufferInfo.eYCbCrType;


        pCurrentMFDPicture->hLuminanceFrameBufferBlock = pstDispPicture->stBufferInfo.hLuminanceFrameBufferBlock;
        pCurrentMFDPicture->ulLuminanceFrameBufferBlockOffset = pstDispPicture->stBufferInfo.ulLuminanceFrameBufferBlockOffset;
        pCurrentMFDPicture->hChrominanceFrameBufferBlock = pstDispPicture->stBufferInfo.hChrominanceFrameBufferBlock;
        pCurrentMFDPicture->ulChrominanceFrameBufferBlockOffset = pstDispPicture->stBufferInfo.ulChrominanceFrameBufferBlockOffset;

        pCurrentMFDPicture->ulLumaRangeRemapping = 0x08;
        pCurrentMFDPicture->ulChromaRangeRemapping = 0x08;



        if (pstDispPicture->stRangeRemapping.bValid)
        {
            pCurrentMFDPicture->ulChromaRangeRemapping = pstDispPicture->stRangeRemapping.uiChroma;
        }

        pCurrentMFDPicture->ulChrominanceNMBY = pstDispPicture->stBufferInfo.uiChromaStripeHeight >> 4;

        if (pstDispPicture->stRangeRemapping.bValid)
        {
            pCurrentMFDPicture->ulLumaRangeRemapping = pstDispPicture->stRangeRemapping.uiLuma;
        }

        pCurrentMFDPicture->ulLuminanceNMBY = pstDispPicture->stBufferInfo.uiLumaStripeHeight >> 4;
        pCurrentMFDPicture->eBitDepth = pstDispPicture->stBufferInfo.eLumaBufferType == BXDM_Picture_BufferType_e10Bit ? BAVC_VideoBitDepth_e10Bit : BAVC_VideoBitDepth_e8Bit;

        /* Unused Parameters */
        pCurrentMFDPicture->ulRowStride = 0;

#if 0
        pCurrentMFDPicture->hFgtHeap = NULL;
        pCurrentMFDPicture->pFgtSeiBufferAddress = NULL;
#endif

        BDBG_MSG(("DisplayManagerLite_isr Disp () -> %d (%d) - (%d) ",pstDispPicture->uiSerialNumber, pCurrentMFDPicture->eSourcePolarity, pCurrentMFDPicture->eInterruptPolarity));

    }
    else
    {
        pCurrentMFDPicture->bMute = true;
        pCurrentMFDPicture->eInterruptPolarity = pstDisplayInterruptInfo->eInterruptPolarity;
        BDBG_MSG(("DisplayManagerLite_isr Disp Mute "));

    }


}

static bool DML_P_CheckPolarity_isr(uint32_t SrcPolarity, uint32_t DisPolarity)
{

    if (SrcPolarity != DisPolarity
        && DisPolarity != BAVC_Polarity_eFrame
        && SrcPolarity != BAVC_Polarity_eFrame)
    {
        return false;
    }
    else
    {
        return true;
    }

}

static BAVC_Polarity DML_P_GetInterruptPolarity_isr( const BXDM_DisplayInterruptInfo *pstDisplayInterruptInfo)
{
    uint32_t Polarity;
    Polarity = pstDisplayInterruptInfo->eInterruptPolarity;

    return Polarity;
}

static bool DML_P_PeekNextPic_isr(NEXUS_VideoDecoderExternalTsmData * pVideoDecoderLite, DML_DispPicStruct *EvalPic)
{
    BXDM_Picture * UnifiedPicture;
    uint32_t SerialNumber;

    EvalPic->pDispPicture = NULL;
    EvalPic->valid = false;

    while ( BFIFO_READ_PEEK(&pVideoDecoderLite->displayPictureQueue.displayFifo) )
    {
        NEXUS_VideoDisplayPictureContext *pContext;

        pContext = BFIFO_READ(&pVideoDecoderLite->displayPictureQueue.displayFifo);
        BDBG_ASSERT(NULL != pContext);
        UnifiedPicture = pContext->pUnifiedPicture;
        SerialNumber = pContext->serialNumber;

        /* Release it if not ment for Display*/
        if (pContext->dropPicture || UnifiedPicture->stPictureType.bLastPicture)
        {
            if ((pVideoDecoderLite->pDecoderPrivateContext!=NULL))
            {
                pVideoDecoderLite->decoderInterface.releasePicture_isr(pVideoDecoderLite->pDecoderPrivateContext,  UnifiedPicture, NULL);
                BDBG_MSG(("Drop () -> %d",UnifiedPicture->uiSerialNumber));
            }
            BFIFO_READ_COMMIT(&pVideoDecoderLite->displayPictureQueue.displayFifo, 1);
        }
        else
        {
            /* Put the Popped picture into NewPic */
            EvalPic->pDispPicture = UnifiedPicture;
            EvalPic->valid = true;
            BDBG_MSG(("DML_P_PeekNextPic_isr Q %p", (void *)pContext));
            break;
        }
    }

    return EvalPic->valid;

}

static void DML_P_ReleasePic_isr(NEXUS_VideoDecoderExternalTsmData * pVideoDecoderLite, DML_DispPicStruct *RelPic)
{
    /* Release Pictures to Filter after display*/
    if (RelPic->valid)
    {
        if ((pVideoDecoderLite->pDecoderPrivateContext!=NULL) && (RelPic->pDispPicture!=NULL))
        {
            pVideoDecoderLite->decoderInterface.releasePicture_isr(pVideoDecoderLite->pDecoderPrivateContext,  RelPic->pDispPicture, NULL);
            if (0) BDBG_WRN(("Rel () -> %d",RelPic->pDispPicture->uiSerialNumber));
        }
        RelPic->valid = false;
    }
}

static uint32_t DML_P_GetSrcPolarity_isr(DML_DispPicStruct *DispPic, uint32_t DisPolarity)
{
    uint32_t SrcPolarity;

    BDBG_MSG((" DML_P_GetSrcPolarity_isr %d %d %d", DispPic->pDispPicture->uiSerialNumber,DispPic->first_pic,DispPic->repeat_field));
    switch (DispPic->pDispPicture->stBufferInfo.ePulldown)
    {
    case BXDM_Picture_PullDown_eBottom:
    case BXDM_Picture_PullDown_eBottomTop:

        SrcPolarity = DispPic->first_pic ? BAVC_Polarity_eBotField: BAVC_Polarity_eTopField;
        break;

    case BXDM_Picture_PullDown_eTop:
    case BXDM_Picture_PullDown_eTopBottom:

        SrcPolarity = DispPic->first_pic ? BAVC_Polarity_eTopField: BAVC_Polarity_eBotField;
        break;

    case BXDM_Picture_PullDown_eBottomTopBottom:

        if (DispPic->first_pic)
        {
            DispPic->repeat_field = 0;
            SrcPolarity = BAVC_Polarity_eBotField;
        }
        else if (!DispPic->repeat_field)
        {
            DispPic->first_pic= 0;
            DispPic->repeat_field = 1;
            SrcPolarity = BAVC_Polarity_eTopField;
        }
        else if (DispPic->repeat_field)
        {
            DispPic->first_pic= 0;
            DispPic->repeat_field = 1;
            SrcPolarity = BAVC_Polarity_eBotField;
        }
        break;
    case BXDM_Picture_PullDown_eTopBottomTop:

        if (DispPic->first_pic)
        {
            DispPic->repeat_field = 0;
            SrcPolarity = BAVC_Polarity_eTopField;
        }
        else if (!DispPic->repeat_field)
        {
            DispPic->first_pic= 0;
            DispPic->repeat_field = 1;
            SrcPolarity = BAVC_Polarity_eBotField;
        }
        else if (DispPic->repeat_field)
        {
            DispPic->first_pic= 0;
            DispPic->repeat_field = 1;
            SrcPolarity = BAVC_Polarity_eTopField;
        }

        break;

    case BXDM_Picture_PullDown_eFrameX2:
    case BXDM_Picture_PullDown_eFrameX3:
    case BXDM_Picture_PullDown_eFrameX1:
    case BXDM_Picture_PullDown_eFrameX4:

        SrcPolarity = BAVC_Polarity_eFrame;
        break;

    default:
        SrcPolarity= BAVC_Polarity_eTopField;
        break;


    }
/*
    if((DisPolarity == BAVC_Polarity_eFrame) && (SrcPolarity != BAVC_Polarity_eFrame))
    {
            SrcPolarity = DisPolarity;
    }
*/
    BDBG_MSG((" Pic %d: SF %d, P %d  O/p %d %d", DispPic->pDispPicture->uiSerialNumber,
              DispPic->pDispPicture->stBufferInfo.eSourceFormat,
              DispPic->pDispPicture->stBufferInfo.ePulldown,
              SrcPolarity,DisPolarity));

    return SrcPolarity;
}

static BAVC_Polarity DML_P_PrepareRepeatPicture_isr(NEXUS_VideoDecoderExternalTsmData * pVideoDecoderLite, BAVC_Polarity DisPolarity)
{
    BAVC_Polarity SrcPolarity = 0;
    if (pVideoDecoderLite->displayPic.valid)
    {
        pVideoDecoderLite->displayPic.first_pic = false;
        SrcPolarity = DML_P_GetSrcPolarity_isr(&pVideoDecoderLite->displayPic,DisPolarity);
        BDBG_MSG(("DML_P_PrepareRepeatPicture_isr %d %d %d", pVideoDecoderLite->displayPic.pDispPicture->uiSerialNumber,pVideoDecoderLite->displayPic.first_pic,pVideoDecoderLite->displayPic.repeat_field));

    }

    return SrcPolarity;
}

static bool DML_P_VerifyNewPicture_isr(BAVC_Polarity DisPolarity,DML_DispPicStruct *EvalPic)
{

    bool new_pic_good;
    BAVC_Polarity SrcPolarity = 0;

    EvalPic->first_pic = true;
    SrcPolarity = DML_P_GetSrcPolarity_isr(EvalPic,DisPolarity);

    new_pic_good = DML_P_CheckPolarity_isr(SrcPolarity, DisPolarity);
    BDBG_MSG(("DML_P_VerifyNewPicture_isr %d?", new_pic_good));

    return new_pic_good;

}

static void DML_P_MakeNewPicCurrent_isr(NEXUS_VideoDecoderExternalTsmData * pVideoDecoderLite, DML_DispPicStruct *EvalPic, DML_DispPicStruct *DispPic)
{
    NEXUS_VideoDisplayPictureContext *pContext;
    BXDM_Picture * UnifiedPicture;
    uint32_t SerialNumber;

    pContext = BFIFO_READ(&pVideoDecoderLite->displayPictureQueue.displayFifo);
    BDBG_ASSERT(NULL != pContext);

    UnifiedPicture = pContext->pUnifiedPicture;
    SerialNumber = pContext->serialNumber;

    /* Pop the picure from the Queue */
    BFIFO_READ_COMMIT(&pVideoDecoderLite->displayPictureQueue.displayFifo, 1);

    BDBG_MSG(("DML_P_MakeNewPicCurrent_isr Q %p %p", (void *)EvalPic->pDispPicture,(void *)UnifiedPicture));

    /* Verify if the peeked Picture is the one on the TOP of the Queue */
    BDBG_ASSERT(UnifiedPicture == EvalPic->pDispPicture);

    pVideoDecoderLite->displayPic.pDispPicture = EvalPic->pDispPicture;
    pVideoDecoderLite->displayPic.valid = EvalPic->valid;
    pVideoDecoderLite->displayPic.first_pic = true;
    pVideoDecoderLite->displayPic.repeat_field= false;
    BSTD_UNUSED(DispPic);

    BDBG_MSG(("DML_P_MakeNewPicCurrent_isr New %d %d %d", pVideoDecoderLite->displayPic.pDispPicture->uiSerialNumber,pVideoDecoderLite->displayPic.first_pic, pVideoDecoderLite->displayPic.repeat_field));
}

static BAVC_Polarity DML_P_PrepareNewPicture_isr(NEXUS_VideoDecoderExternalTsmData * pVideoDecoderLite, BAVC_Polarity DisPolarity, DML_DispPicStruct *EvalPic)
{
    BAVC_Polarity SrcPolarity = 0;
    if (pVideoDecoderLite->displayPic.valid)
    {
        DML_P_ReleasePic_isr(pVideoDecoderLite,&pVideoDecoderLite->displayPic);
    }
    DML_P_MakeNewPicCurrent_isr(pVideoDecoderLite, EvalPic, &pVideoDecoderLite->displayPic);
    BDBG_MSG(("DML_P_PrepareNewPicture_isr %d %p",pVideoDecoderLite->displayPic.valid, (void *)pVideoDecoderLite->displayPic.pDispPicture));

    SrcPolarity = DML_P_GetSrcPolarity_isr(&pVideoDecoderLite->displayPic,DisPolarity);

    BDBG_MSG(("DML_P_PrepareNewPicture_isr"));

    return SrcPolarity;
}

static NEXUS_Error DisplayManagerLite_isr( void* pPrivateContext, const BXDM_DisplayInterruptInfo *pstDisplayInterruptInfo, BAVC_MVD_Picture **pDispMgrToVDC)
{
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderExternalTsmData * pVideoDecoderLite;
    BAVC_Polarity SrcPolarity;
    BAVC_Polarity DisPolarity;
    unsigned PictureCount=0;

    bool new_pic_good;

    DML_DispPicStruct EvalPic;
    videoDecoder = pPrivateContext;
    pVideoDecoderLite = &videoDecoder->externalTsm;

    BKNI_Memset( &EvalPic, 0, sizeof (DML_DispPicStruct));

    DisPolarity = DML_P_GetInterruptPolarity_isr(pstDisplayInterruptInfo);

    /* Don't call any decoder functions when we're about to or already have stopped it */
    if ( videoDecoder->externalTsm.stopped == false )
    {
        /* Call the Decoder Display Interrupt Event callback if one is registered */
        if ( NULL !=pVideoDecoderLite->decoderInterface.displayInterruptEvent_isr)
        {
            pVideoDecoderLite->decoderInterface.displayInterruptEvent_isr(pVideoDecoderLite->pDecoderPrivateContext);
        }

        /* Check for new frames from the decoder and space to hold them */
        pVideoDecoderLite->decoderInterface.getPictureCount_isr(videoDecoder->dec, &PictureCount);
        if ( PictureCount > 0 && BFIFO_WRITE_LEFT(&pVideoDecoderLite->decoderPictureQueue.decodeFifo) > 0 )
        {
            NEXUS_IsrCallback_Fire_isr(videoDecoder->dataReadyCallback);
        }

        if (DML_P_PeekNextPic_isr(pVideoDecoderLite,&EvalPic) == true )
        {
            new_pic_good = DML_P_VerifyNewPicture_isr(DisPolarity,&EvalPic);
            if (new_pic_good)
            {
                SrcPolarity = DML_P_PrepareNewPicture_isr(pVideoDecoderLite,DisPolarity,&EvalPic);
            }
            else
            {
                SrcPolarity = DML_P_PrepareRepeatPicture_isr(pVideoDecoderLite,DisPolarity);
            }
        }
        else
        {
            SrcPolarity = DML_P_PrepareRepeatPicture_isr(pVideoDecoderLite,DisPolarity);
        }
    }
    else
    {
        /* Stopped - just prepare a repeat */
        SrcPolarity = DML_P_PrepareRepeatPicture_isr(pVideoDecoderLite,DisPolarity);
    }

    DML_P_Prepare_MFD_Struct_isr(pVideoDecoderLite, pstDisplayInterruptInfo, pDispMgrToVDC, pVideoDecoderLite->displayPic.pDispPicture, SrcPolarity);
    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_VideoDecoder_P_InitializeQueue(NEXUS_VideoDecoderHandle videoDecoder)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    void * pContext;
    BXDM_DisplayInterruptHandler_Handle displayInterrupt;
    BXDM_DisplayInterruptHandler_AddPictureProviderInterface_Settings addPictureProviderSettings;
    static void *pPrivateContext;

    videoDecoder->externalTsm.stopped = false;
    videoDecoder->externalTsm.pMFDPicture = &videoDecoder->externalTsm.MFDPicture;
    BKNI_Memset( videoDecoder->externalTsm.pMFDPicture, 0, sizeof ( BAVC_MFD_Picture ));

    BDBG_MSG(("NEXUS_VideoDecoder_Initialize_Queue "));

    BFIFO_INIT(&videoDecoder->externalTsm.decoderPictureQueue.decodeFifo, videoDecoder->externalTsm.decoderPictureQueue.pictureContexts, NEXUS_P_MAX_ELEMENTS_IN_VIDEO_DECODER_PIC_QUEUE);
    BFIFO_INIT(&videoDecoder->externalTsm.displayPictureQueue.displayFifo, videoDecoder->externalTsm.displayPictureQueue.pictureContexts, NEXUS_P_MAX_ELEMENTS_IN_VIDEO_DECODER_PIC_QUEUE);
    videoDecoder->externalTsm.numDecoded = 0;
    videoDecoder->externalTsm.numDisplayed = 0;
    videoDecoder->externalTsm.numIFramesDisplayed = 0;

    videoDecoder->externalTsm.pDecoderPrivateContext = videoDecoder->dec;
    pPrivateContext = &videoDecoder->externalTsm;
    displayInterrupt = videoDecoder->device->hXdmDih[videoDecoder->xdmIndex + BXVD_DisplayInterrupt_eZero];

    if ( videoDecoder->dec ) {
        /* Must close/re-open the XVD channel to release its picture provider TODO: Optimize this */
        NEXUS_VideoDecoder_P_CloseChannel(videoDecoder);
        rc = NEXUS_VideoDecoder_P_OpenChannel(videoDecoder);
        if ( rc ) { return BERR_TRACE(rc); }
    }

    rc = BXDM_DIH_AddPictureProviderInterface_GetDefaultSettings (&addPictureProviderSettings);
    if(rc!=BERR_SUCCESS) {BDBG_ERR(("BXDM_DIH_AddPictureProviderInterface_GetDefaultSettings Failed "));}

    rc = BXDM_DisplayInterruptHandler_AddPictureProviderInterface(displayInterrupt, DisplayManagerLite_isr, videoDecoder, &addPictureProviderSettings);
    if(rc!=BERR_SUCCESS) {BDBG_ERR(("BXDM_DisplayInterruptHandler_AddPictureProviderInterface Failed "));}

    /* TODO: There should be an extern for this... */
    rc = BXVD_Decoder_GetDMInterface(videoDecoder->dec, &(videoDecoder->externalTsm.decoderInterface), &pContext );
    if(rc!=BERR_SUCCESS) {BDBG_ERR(("BXVD_Decoder_GetDMInterface Failed "));}

    return rc;
}

#if 0
{
    /* Update the Data Structures reqired by application framework*/

}
#endif

static NEXUS_Error NEXUS_VideoDecoder_P_GetDecodedFrame(
    NEXUS_VideoDecoderHandle videoDecoder
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned PictureCount = 0;
    uint32_t SerialNumber;
    const BXDM_Picture * UnifiedPicture;
    NEXUS_VideoDecoderPictureContext *pContext;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoDecoder, videoDecoder);

    if ( false == videoDecoder->started || !videoDecoder->startSettings.appDisplayManagement )
    {
        return NEXUS_NOT_AVAILABLE;
    }

    BKNI_EnterCriticalSection();

    if(videoDecoder->dec)
    {
        videoDecoder->externalTsm.decoderInterface.getPictureCount_isr(videoDecoder->dec, &PictureCount);
    }

    BDBG_MSG(("XVD Decoder Pictures %d",PictureCount));

    if((PictureCount!=0) && BFIFO_WRITE_PEEK(&videoDecoder->externalTsm.decoderPictureQueue.decodeFifo))
    {
        videoDecoder->externalTsm.decoderInterface.getNextPicture_isr( videoDecoder->dec, &UnifiedPicture);

        SerialNumber = ++videoDecoder->externalTsm.decoderPictureQueue.pictureCounter;

        pContext = BFIFO_WRITE(&videoDecoder->externalTsm.decoderPictureQueue.decodeFifo);
        pContext->pUnifiedPicture = (BXDM_Picture *) UnifiedPicture;
        pContext->serialNumber= SerialNumber;
        pContext->pUnifiedPicture->uiSerialNumber = SerialNumber;

        BFIFO_WRITE_COMMIT(&videoDecoder->externalTsm.decoderPictureQueue.decodeFifo, 1);
        if ( UnifiedPicture->stPictureType.bLastPicture && !videoDecoder->last_field_flag )
        {
            videoDecoder->last_field_flag = true;
            NEXUS_IsrCallback_Fire_isr(videoDecoder->sourceChangedCallback);
        }
        else
        {
            videoDecoder->externalTsm.numDecoded++;
        }

        /* Update last_field information with most recently decoded picture for appDisplayManagement */
        videoDecoder->last_field.bMute = false;
        if ( UnifiedPicture->stBufferInfo.stDisplay.bValid )
        {
            videoDecoder->last_field.ulDisplayHorizontalSize = UnifiedPicture->stBufferInfo.stDisplay.uiWidth;
            videoDecoder->last_field.ulDisplayVerticalSize = UnifiedPicture->stBufferInfo.stDisplay.uiHeight;
        }
        else
        {
            videoDecoder->last_field.ulDisplayHorizontalSize = UnifiedPicture->stBufferInfo.stSource.uiWidth;
            videoDecoder->last_field.ulDisplayVerticalSize = UnifiedPicture->stBufferInfo.stSource.uiHeight;
            if ( true == UnifiedPicture->stClipping.bValid )
            {
                videoDecoder->last_field.ulDisplayHorizontalSize -= (UnifiedPicture->stClipping.uiLeft + UnifiedPicture->stClipping.uiRight);
                videoDecoder->last_field.ulDisplayVerticalSize -= (UnifiedPicture->stClipping.uiTop + UnifiedPicture->stClipping.uiBottom);
            }
        }
        videoDecoder->last_field.ulSourceHorizontalSize = UnifiedPicture->stBufferInfo.stSource.uiWidth;
        videoDecoder->last_field.ulSourceVerticalSize = UnifiedPicture->stBufferInfo.stSource.uiHeight;
        switch (UnifiedPicture->stBufferInfo.ePulldown) {
        case BXDM_Picture_PullDown_eTop:
        case BXDM_Picture_PullDown_eBottom:
        case BXDM_Picture_PullDown_eTopBottom:
        case BXDM_Picture_PullDown_eBottomTop:
        case BXDM_Picture_PullDown_eTopBottomTop:
        case BXDM_Picture_PullDown_eBottomTopBottom:
            videoDecoder->last_field.bStreamProgressive = false;
            break;
        default:
            videoDecoder->last_field.bStreamProgressive = true;
            break;
        }
        if (UnifiedPicture->stAspectRatio.bValid) {
            videoDecoder->last_field.eAspectRatio = UnifiedPicture->stAspectRatio.eAspectRatio;
        }
        else {
            videoDecoder->last_field.eAspectRatio = NEXUS_AspectRatio_eUnknown;
        }
        if (!UnifiedPicture->stFrameRate.bValid || UnifiedPicture->stFrameRate.stRate.uiNumerator == 0 || UnifiedPicture->stFrameRate.stRate.uiDenominator == 0) {
            videoDecoder->last_field.eFrameRateCode = NEXUS_VideoFrameRate_eUnknown;
        }
        else {
            NEXUS_VideoFrameRate nxFrameRate;
            NEXUS_P_FrameRate_FromRefreshRate_isrsafe((1000 * UnifiedPicture->stFrameRate.stRate.uiNumerator) / UnifiedPicture->stFrameRate.stRate.uiDenominator, &nxFrameRate);
            videoDecoder->last_field.eFrameRateCode = nxFrameRate;
        }

        {
            BXVD_PictureParameterInfo info;
            BXVD_SetPictureParameterInfo_isrsafe(&info, &videoDecoder->last_field, UnifiedPicture);
            NEXUS_VideoDecoder_P_PictureParams_isr(videoDecoder, 0, &info);
        }
    }
    else
    {
        /* No decoded pictures */
        rc = NEXUS_UNKNOWN;
    }

    BKNI_LeaveCriticalSection();

    return rc;
}

static void NEXUS_VideoDecoder_P_ContextToStatus(NEXUS_VideoDecoderHandle decoder, NEXUS_VideoDecoderPictureContext *pContext, NEXUS_VideoDecoderFrameStatus *pStatus)
{
    BXDM_Picture *UnifiedPicture;

    UnifiedPicture = pContext->pUnifiedPicture;
    BDBG_ASSERT(NULL != UnifiedPicture);

    NEXUS_StripedSurface_GetDefaultCreateSettings(&pStatus->surfaceCreateSettings);
    pStatus->lastPicture = UnifiedPicture->stPictureType.bLastPicture;
    if ( false == pStatus->lastPicture )
    {
        pStatus->surfaceCreateSettings.imageWidth = UnifiedPicture->stBufferInfo.stSource.uiWidth;
        pStatus->surfaceCreateSettings.imageHeight = UnifiedPicture->stBufferInfo.stSource.uiHeight;
        if ( true == UnifiedPicture->stClipping.bValid )
        {
            pStatus->surfaceCreateSettings.imageWidth = pStatus->surfaceCreateSettings.imageWidth - (UnifiedPicture->stClipping.uiLeft +UnifiedPicture->stClipping.uiRight);
            pStatus->surfaceCreateSettings.imageHeight = pStatus->surfaceCreateSettings.imageHeight - (UnifiedPicture->stClipping.uiTop + UnifiedPicture->stClipping.uiBottom);
        }
        pStatus->surfaceCreateSettings.lumaBuffer = NEXUS_VideoDecoder_P_MemoryBlockFromMma(decoder, UnifiedPicture->stBufferInfo.hLuminanceFrameBufferBlock);
        pStatus->surfaceCreateSettings.chromaBuffer = NEXUS_VideoDecoder_P_MemoryBlockFromMma(decoder, UnifiedPicture->stBufferInfo.hChrominanceFrameBufferBlock);
        pStatus->surfaceCreateSettings.bottomFieldLumaBuffer = NULL;
        pStatus->surfaceCreateSettings.bottomFieldChromaBuffer = NULL;

        pStatus->surfaceCreateSettings.lumaBufferOffset = UnifiedPicture->stBufferInfo.ulLuminanceFrameBufferBlockOffset;
        pStatus->surfaceCreateSettings.chromaBufferOffset = UnifiedPicture->stBufferInfo.ulChrominanceFrameBufferBlockOffset;
        pStatus->surfaceCreateSettings.bottomFieldLumaBufferOffset = 0;
        pStatus->surfaceCreateSettings.bottomFieldChromaBufferOffset = 0;
        pStatus->surfaceCreateSettings.pitch = NEXUS_P_StripeWidth_FromMagnum_isrsafe(UnifiedPicture->stBufferInfo.eStripeWidth);
        pStatus->surfaceCreateSettings.stripedWidth = pStatus->surfaceCreateSettings.pitch;
        pStatus->surfaceCreateSettings.lumaStripedHeight = UnifiedPicture->stBufferInfo.uiLumaStripeHeight;
        pStatus->surfaceCreateSettings.chromaStripedHeight = UnifiedPicture->stBufferInfo.uiChromaStripeHeight;\
    }

    pStatus->serialNumber = pContext->serialNumber;
    pStatus->ptsValid = UnifiedPicture->stPTS.bValid;
    pStatus->pts = UnifiedPicture->stPTS.uiValue;
    switch ( UnifiedPicture->stPictureType.eCoding )
    {
    default:
        pStatus->pictureCoding = NEXUS_PictureCoding_eUnknown;
        break;
    case BXDM_Picture_Coding_eI:
        pStatus->pictureCoding = NEXUS_PictureCoding_eI;
        break;
    case BXDM_Picture_Coding_eP:
        pStatus->pictureCoding = NEXUS_PictureCoding_eP;
        break;
    case BXDM_Picture_Coding_eB:
        pStatus->pictureCoding = NEXUS_PictureCoding_eB;
        break;
    }

    pStatus->surfaceCreateSettings.bufferType = NEXUS_VideoBufferType_eFrame;
    if (!UnifiedPicture->stFrameRate.bValid || UnifiedPicture->stFrameRate.stRate.uiNumerator == 0 || UnifiedPicture->stFrameRate.stRate.uiDenominator == 0) {
        pStatus->frameRate = NEXUS_VideoFrameRate_eUnknown;
    }
    else {
        NEXUS_P_FrameRate_FromRefreshRate_isrsafe((1000 * UnifiedPicture->stFrameRate.stRate.uiNumerator) / UnifiedPicture->stFrameRate.stRate.uiDenominator, &pStatus->frameRate);
    }
    switch (UnifiedPicture->stBufferInfo.ePulldown) {
    case BXDM_Picture_PullDown_eTop: pStatus->pullDown = NEXUS_PicturePullDown_eTop; break;
    case BXDM_Picture_PullDown_eBottom: pStatus->pullDown = NEXUS_PicturePullDown_eBottom; break;
    case BXDM_Picture_PullDown_eTopBottom: pStatus->pullDown = NEXUS_PicturePullDown_eTopBottom; break;
    case BXDM_Picture_PullDown_eBottomTop: pStatus->pullDown = NEXUS_PicturePullDown_eBottomTop; break;
    case BXDM_Picture_PullDown_eTopBottomTop: pStatus->pullDown = NEXUS_PicturePullDown_eTopBottomTop; break;
    case BXDM_Picture_PullDown_eBottomTopBottom: pStatus->pullDown = NEXUS_PicturePullDown_eBottomTopBottom; break;
    default: pStatus->pullDown = NEXUS_PicturePullDown_eFrame; break;
    }

    if (UnifiedPicture->stAspectRatio.bValid) {
        pStatus->aspectRatio = UnifiedPicture->stAspectRatio.eAspectRatio;
        if (pStatus->aspectRatio == NEXUS_AspectRatio_eSar) {
            pStatus->sampleAspectRatio.x = UnifiedPicture->stAspectRatio.uiSampleAspectRatioX;
            pStatus->sampleAspectRatio.y = UnifiedPicture->stAspectRatio.uiSampleAspectRatioY;
        }
    }
    else {
        pStatus->aspectRatio = NEXUS_AspectRatio_eUnknown;
    }

    pStatus->sourceFormat = UnifiedPicture->stBufferInfo.eSourceFormat;
    pStatus->sequence = UnifiedPicture->stPictureType.eSequence;

    if (BAVC_YCbCrType_e4_2_0 == UnifiedPicture->stBufferInfo.eYCbCrType)
    {
        if (BXDM_Picture_BufferType_e10Bit == UnifiedPicture->stBufferInfo.eLumaBufferType)
        {
            pStatus->surfaceCreateSettings.lumaPixelFormat = BM2MC_PACKET_PixelFormat_eY10;
            pStatus->surfaceCreateSettings.chromaPixelFormat = BM2MC_PACKET_PixelFormat_eCb10_Cr10;
        }
        else
        {
            pStatus->surfaceCreateSettings.lumaPixelFormat = BM2MC_PACKET_PixelFormat_eY8;
            pStatus->surfaceCreateSettings.chromaPixelFormat = BM2MC_PACKET_PixelFormat_eCb8_Cr8;
        }
    }
    else
    {
        BM2MC_PACKET_PixelFormat m2mcPxlFmt;
        BGRC_Packet_ConvertPixelFormat(&m2mcPxlFmt, UnifiedPicture->stBufferInfo.stPixelFormat.ePixelFormat);
        pStatus->surfaceCreateSettings.lumaPixelFormat = m2mcPxlFmt;
    }
}

static NEXUS_Error NEXUS_VideoDecoder_P_GetDecodedFrameStatus(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderFrameStatus *pStatus,
    unsigned numEntries,
    unsigned *pNumEntries
    )
{
    unsigned i, num, left;
    NEXUS_VideoDecoderPictureContext *pContext;

    /* Return contiguous frames */
    num = BFIFO_READ_PEEK(&handle->externalTsm.decoderPictureQueue.decodeFifo);
    if ( num > numEntries )
    {
        num = numEntries;
    }
    pContext = BFIFO_READ(&handle->externalTsm.decoderPictureQueue.decodeFifo);
    for ( i = 0; i < num; i++,pContext++,pStatus++ )
    {
        NEXUS_VideoDecoder_P_ContextToStatus(handle, pContext, pStatus);
    }

    *pNumEntries = i;

    /* Get entries after wrap */
    left = BFIFO_READ_LEFT(&handle->externalTsm.decoderPictureQueue.decodeFifo) -
            BFIFO_READ_PEEK(&handle->externalTsm.decoderPictureQueue.decodeFifo);
    if ( left > (numEntries-num) )
    {
        left = numEntries - num;
    }
    pContext = handle->externalTsm.decoderPictureQueue.pictureContexts;
    for ( i = 0; i < left; i++,pContext++,pStatus++ )
    {
        NEXUS_VideoDecoder_P_ContextToStatus(handle, pContext, pStatus);
    }

    *pNumEntries = *pNumEntries + i;

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_VideoDecoder_GetDecodedFrames_Avd(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderFrameStatus *pStatus,  /* attr{nelem=numEntries;nelem_out=pNumEntriesReturned} [out] */
    unsigned numEntries,
    unsigned *pNumEntriesReturned /* [out] */
    )
{
    unsigned i=0;
    NEXUS_Error errCode = NEXUS_SUCCESS;

    if ( NULL == pStatus || numEntries == 0 || NULL == pNumEntriesReturned )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( handle->started )
    {
        if ( handle->startSettings.appDisplayManagement )
        {
            /* Pop any decoded frames from the XVD queue */
            do
            {
                errCode = NEXUS_VideoDecoder_P_GetDecodedFrame(handle);
            } while ( errCode == NEXUS_SUCCESS );
            errCode = NEXUS_VideoDecoder_P_GetDecodedFrameStatus(handle, pStatus, numEntries, pNumEntriesReturned);
            if ( errCode )
            {
                *pNumEntriesReturned = 0;
                return BERR_TRACE(errCode);
            }

            return NEXUS_SUCCESS;
        }
        else
        {
            for ( i = 0; i < numEntries; i++ )
            {
                errCode = NEXUS_VideoDecoder_P_GetStripedSurfaceCreateSettings(handle, &handle->last_field, &pStatus[i].surfaceCreateSettings);
                if ( errCode )
                {
                    break;
                }
                if (pStatus[i].surfaceCreateSettings.lumaBuffer) {
                    NEXUS_OBJECT_REGISTER(NEXUS_MemoryBlock, pStatus[i].surfaceCreateSettings.lumaBuffer, Acquire);
                }
                if (pStatus[i].surfaceCreateSettings.chromaBuffer) {
                    NEXUS_OBJECT_REGISTER(NEXUS_MemoryBlock, pStatus[i].surfaceCreateSettings.chromaBuffer, Acquire);
                }
                pStatus[i].serialNumber = handle->last_field.ulDecodePictureId;
                pStatus[i].pts = handle->last_field.ulOrigPTS;
                pStatus[i].ptsValid = true;
                switch ( handle->last_field.ePictureType )
                {
                default:
                    pStatus[i].pictureCoding = NEXUS_PictureCoding_eUnknown;
                    break;
                case BAVC_PictureCoding_eI:
                    pStatus[i].pictureCoding = NEXUS_PictureCoding_eI;
                    break;
                case BAVC_PictureCoding_eP:
                    pStatus[i].pictureCoding = NEXUS_PictureCoding_eP;
                    break;
                case BAVC_PictureCoding_eB:
                    pStatus[i].pictureCoding = NEXUS_PictureCoding_eB;
                    break;
                }
            }
        }
    }

    *pNumEntriesReturned = i;
    return NEXUS_SUCCESS;
}

static void NEXUS_VideoDecoder_P_ReturnOutstandingFrames_Avd(
    NEXUS_VideoDecoderHandle videoDecoder
    )
{
    unsigned NumEntries;

    if ( videoDecoder->startSettings.appDisplayManagement )
    {
        /* Purge all pending frames from the app queue */
        const unsigned entries = sizeof(videoDecoder->functionData.returnOutstandingFrames_Avd.status) / sizeof(videoDecoder->functionData.returnOutstandingFrames_Avd.status[0]);
        while ( NEXUS_SUCCESS == NEXUS_VideoDecoder_GetDecodedFrames_Avd(videoDecoder, videoDecoder->functionData.returnOutstandingFrames_Avd.status, entries, &NumEntries) &&
                NumEntries > 0 )
        {
            NEXUS_VideoDecoder_ReturnDecodedFrames_Avd(videoDecoder, NULL, NumEntries);
        }

        BKNI_EnterCriticalSection();

        /* Return all pictures pushed into the "display" fifo above */
        while ( BFIFO_READ_PEEK(&videoDecoder->externalTsm.displayPictureQueue.displayFifo) )
        {
            NEXUS_VideoDisplayPictureContext *pContext;

            pContext = BFIFO_READ(&videoDecoder->externalTsm.displayPictureQueue.displayFifo);
            BDBG_ASSERT(NULL != pContext);
            BFIFO_READ_COMMIT(&videoDecoder->externalTsm.displayPictureQueue.displayFifo, 1);
            videoDecoder->externalTsm.decoderInterface.releasePicture_isr(videoDecoder->externalTsm.pDecoderPrivateContext, pContext->pUnifiedPicture, NULL);
        }
        /* Return active picture */
        DML_P_ReleasePic_isr(&videoDecoder->externalTsm,&videoDecoder->externalTsm.displayPic);

        BKNI_LeaveCriticalSection();
    }
}

NEXUS_Error NEXUS_VideoDecoder_ReturnDecodedFrames_Avd(
    NEXUS_VideoDecoderHandle videoDecoder,
    const NEXUS_VideoDecoderReturnFrameSettings *pSettings, /* attr{null_allowed=y, nelem=numFrames} Settings for each returned frame.  Pass NULL for defaults. */
    unsigned numFrames                                      /* Number of frames to return to the decoder */
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i;

    if ( false == videoDecoder->started )
    {
        return NEXUS_SUCCESS;
    }
    if ( false == videoDecoder->startSettings.appDisplayManagement )
    {
        /* Nothing to do - surface creation is now up to the caller */
        return NEXUS_SUCCESS;
    }

    BKNI_EnterCriticalSection();

    if ( numFrames > (unsigned)BFIFO_READ_LEFT(&videoDecoder->externalTsm.decoderPictureQueue.decodeFifo) )
    {
        BKNI_LeaveCriticalSection();
        BDBG_ERR(("Attempt to return more decoded frames than frames available"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* This should never happen.  The queues are the same size. */
    BDBG_ASSERT(numFrames <= (unsigned)BFIFO_WRITE_LEFT(&videoDecoder->externalTsm.displayPictureQueue.displayFifo));

    for ( i = 0; i < numFrames; i++ )
    {
        NEXUS_VideoDecoderPictureContext *pDecoderPicture;
        NEXUS_VideoDisplayPictureContext *pDisplayPicture;

        pDecoderPicture = BFIFO_READ(&videoDecoder->externalTsm.decoderPictureQueue.decodeFifo);
        pDisplayPicture = BFIFO_WRITE(&videoDecoder->externalTsm.displayPictureQueue.displayFifo);

        pDisplayPicture->pUnifiedPicture = pDecoderPicture->pUnifiedPicture;
        pDisplayPicture->serialNumber = pDecoderPicture->serialNumber;
        pDisplayPicture->dropPicture = (pSettings != NULL) ? !pSettings->display : false;

        BFIFO_READ_COMMIT(&videoDecoder->externalTsm.decoderPictureQueue.decodeFifo, 1);
        BFIFO_WRITE_COMMIT(&videoDecoder->externalTsm.displayPictureQueue.displayFifo, 1);
        videoDecoder->externalTsm.numDisplayed++;
        if ( pDisplayPicture->pUnifiedPicture->stPictureType.eCoding == BXDM_Picture_Coding_eI )
        {
            videoDecoder->externalTsm.numIFramesDisplayed++;
        }

        if (pSettings != NULL)
        {
            ++pSettings;
        }
    }

    BKNI_LeaveCriticalSection();

    return rc;
}

void NEXUS_VideoDecoder_GetDefaultReturnFrameSettings(
    NEXUS_VideoDecoderReturnFrameSettings *pSettings    /* [out] */
    )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->display = true;
}

NEXUS_Error NEXUS_VideoDecoder_GetFifoStatus(
    NEXUS_VideoDecoderHandle videoDecoder,
    NEXUS_VideoDecoderFifoStatus *pStatus /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    if ( NULL == pStatus )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if ( NULL == videoDecoder->rave )
    {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    else
    {
        int rc;

        LOCK_TRANSPORT();
        rc = NEXUS_Rave_GetPtsRange_priv(videoDecoder->rave, &pStatus->pts.mostRecent, &pStatus->pts.leastRecent);
        UNLOCK_TRANSPORT();
        if ( NEXUS_SUCCESS == rc )
        {
            pStatus->pts.valid = true;
        }

        if ( videoDecoder->dec )
        {
            BXVD_PTSInfo ptsInfo;
            BXVD_ChannelStatus channelStatus;

            BKNI_EnterCriticalSection();
            (void)BXVD_GetPTS_isr(videoDecoder->dec, &ptsInfo);
            BKNI_LeaveCriticalSection();
            (void)BXVD_GetChannelStatus(videoDecoder->dec, &channelStatus);

            /* PTS 0 is valid and also means "no PTS". so, if queueDepth is non-zero, we regard PTS 0 as valid, otherwise as "no PTS". */
            if (pStatus->pts.valid && (ptsInfo.ui32RunningPTS || channelStatus.ulPictureDeliveryCount)) {
                /* check for backwards PTS discontinuity */
                if (pStatus->pts.mostRecent > ptsInfo.ui32RunningPTS) {
                    pStatus->bufferLatency = (pStatus->pts.mostRecent - ptsInfo.ui32RunningPTS) / (NEXUS_IS_DSS_MODE(videoDecoder->transportType) ? 27000 : 45);
                    BDBG_MSG_TRACE(("pts calc: %#x %#x --> %d", pStatus->pts.mostRecent, ptsInfo.ui32RunningPTS, pStatus->bufferLatency));
                }
                else {
                    /* we can't know (even if we scanned entire ITB) until discont passes through. */
                    pStatus->bufferLatency = 0;
                }
            }
            else {
                /* no PTS found, so we can measure time the display queue, assuming constant source frame rate */
                unsigned refreshRate = NEXUS_P_RefreshRate_FromFrameRate_isrsafe(videoDecoder->streamInfo.frameRate);
                pStatus->bufferLatency = channelStatus.ulPictureDeliveryCount * 1000000 / (refreshRate?refreshRate:30000);
                BDBG_MSG_TRACE(("queue calc: %d pic, %d refresh --> %d", channelStatus.ulPictureDeliveryCount, refreshRate, pStatus->bufferLatency));
            }
        }
    }
    return NEXUS_SUCCESS;
}

/* this does not reserve the MFD. it requires the app to not open the conflicting
VideoDecoder. the safest policy is to not open another VideoDecoder while an extension video decoder is open. */
NEXUS_Error NEXUS_VideoDecoder_P_getUnusedMfd(bool dsp, BAVC_SourceId *pId, unsigned *pHeapIndex)
{
    int i;
    int rc;
    unsigned heapIndex = dsp ? g_NEXUS_videoDecoderModuleSettings.videoDecoderExtension.dsp.heapIndex :
                               g_NEXUS_videoDecoderModuleSettings.videoDecoderExtension.sid.heapIndex;
    unsigned memcIndex;

    rc = NEXUS_Core_HeapMemcIndex_isrsafe(heapIndex, &memcIndex);
    if (rc) return BERR_TRACE(rc);

    /* find the first available MFD on the required MEMC.
    search in reverse order to minimize collisions with simple code. for more complex code, better management is required. */
    for (i=NEXUS_NUM_VIDEO_DECODERS-1;i>=0;i--) {
        unsigned index;
        if (g_videoDecoders[i] || !g_NEXUS_videoDecoderModuleSettings.memory[i].used) continue;
        rc = NEXUS_Core_HeapMemcIndex_isrsafe(g_NEXUS_videoDecoderModuleSettings.avdHeapIndex[g_NEXUS_videoDecoderModuleSettings.avdMapping[i]], &index);
        if (rc) continue;
        if (index == memcIndex) {
            *pHeapIndex = heapIndex;
            *pId = g_NEXUS_videoDecoderModuleSettings.mfdMapping[i] + BAVC_SourceId_eMpeg0;
            return NEXUS_SUCCESS;
        }
    }
    return NEXUS_NOT_AVAILABLE;
}

void NEXUS_VideoDecoder_GetPrivateSettings(NEXUS_VideoDecoderHandle videoDecoder, NEXUS_VideoDecoderPrivateSettings * pSettings)
{
    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    if (pSettings)
    {
        BKNI_Memcpy(pSettings, &videoDecoder->private.settings, sizeof(NEXUS_VideoDecoderPrivateSettings));
    }
}

NEXUS_Error NEXUS_VideoDecoder_SetPrivateSettings(NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderPrivateSettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(videoDecoder, NEXUS_VideoDecoder);

    NEXUS_IsrCallback_Set(videoDecoder->private.streamChangedCallback, &pSettings->streamChanged);

    return rc;
}
