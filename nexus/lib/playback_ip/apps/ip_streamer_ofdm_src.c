/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELYn
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *  ofdm source setup file
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ******************************************************************************/
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>

#include "bstd.h"
#include "bkni.h"
#include "blst_list.h"
#include "blst_queue.h"

#ifndef DMS_CROSS_PLATFORMS
#include "nexus_platform.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_parser_band.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_message.h"
#include "nexus_timebase.h"
#include "nexus_recpump.h"
#ifdef NEXUS_HAS_RECORD
#include "nexus_record.h"
#endif
#include "nexus_file_fifo.h"
#include "b_psip_lib.h"
#include "nexus_core_utils.h"
#endif /* DMS_CROSS_PLATFORMS */
#include "b_playback_ip_lib.h"

#ifdef B_HAS_DTCP_IP
#include "b_dtcp_applib.h"
#include "b_dtcp_constants.h"
#endif

#include "nexus_core_utils.h"
#include "ip_streamer_lib.h"
#include "ip_streamer.h"
#include "ip_http.h"
#include "b_os_lib.h"
#ifndef DMS_CROSS_PLATFORMS
#include "ip_psi.h"
#endif
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "ip_streamer_transcode.h"
#endif

BDBG_MODULE(ip_streamer);
#if 0
#define BDBG_MSG_FLOW(x)  BDBG_WRN( x );
#else
#define BDBG_MSG_FLOW(x)
#endif

#ifdef NEXUS_HAS_FRONTEND
/* build a list of all available OFDM sources */
/* Note: for VMS configuration, local tuners (aka sources) == # of decoders are left for local decoding app */
int
initNexusOfdmSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    unsigned i, j;
    int ofdmTunerCount = 0;
    OfdmSrc *ofdmSrc;
    NEXUS_FrontendHandle frontendHandle=NULL;
    NEXUS_PlatformConfiguration platformConfig;

    /* global reset */
    memset(ipStreamerGlobalCtx->ofdmSrcList, 0, sizeof(ipStreamerGlobalCtx->ofdmSrcList));

    if (BKNI_CreateMutex(&ipStreamerGlobalCtx->ofdmSrcMutex) != 0) {
        BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
        return -1;
    }

    NEXUS_Platform_GetConfiguration(&platformConfig);
    for (i = 0, j = 0; i < NEXUS_MAX_FRONTENDS; i++) {
        NEXUS_FrontendCapabilities capabilities;
        if(ipStreamerGlobalCtx->globalCfg.multiProcessEnv) {
            NEXUS_FrontendAcquireSettings settings;
            NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
            settings.mode = NEXUS_FrontendAcquireMode_eByCapabilities;
            settings.capabilities.ofdm = true;
            frontendHandle = NEXUS_Frontend_Acquire(&settings);
        } else {
            frontendHandle = platformConfig.frontend[i];
        }

        if (!frontendHandle) {
            BDBG_MSG(("%s: NULL Frontend (# %d)", __FUNCTION__, i));
            continue;
        }
        NEXUS_Frontend_GetCapabilities(frontendHandle, &capabilities);
        if (capabilities.ofdm) {
            ofdmTunerCount++;
            /* Found a ofdm source */
            ofdmSrc = &ipStreamerGlobalCtx->ofdmSrcList[j];
            memset(ofdmSrc, 0, sizeof(OfdmSrc));
            ofdmSrc->frontendHandle = frontendHandle;
            if (BKNI_CreateEvent(&ofdmSrc->psiAcquiredEvent)) {
                BDBG_ERR(("Failed to psiAcquired event at %d", __LINE__));
                return -1;
            }
            if (BKNI_CreateEvent(&ofdmSrc->signalLockedEvent)) {
                BDBG_ERR(("Failed to signalLocked event at %d", __LINE__));
                return -1;
            }
            if (BKNI_CreateMutex(&ofdmSrc->lock) != 0) {
                BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
                return -1;
            }
            BDBG_MSG(("%s: ofdmSrc %p, lock %p", __FUNCTION__, ofdmSrc, ofdmSrc->lock));
            /* successfully setup a OFDM frontend src */
            j++;
        }
        else {
            /* skip other tuner types */
            BDBG_MSG(("!!!!!!!!!!!!!!!!!!!!!! OFDM Capabilities not set for this frontend %p", frontendHandle));
            continue;
        }
    }

    if (!j) {
        BDBG_MSG(("NO OFDM Front End founds, check Nexus platform configuration"));
        return 0;
    }
    BDBG_MSG(("################################## Initialized %d OFDM Frontend Sources", j));
    return 0;
}

void
unInitNexusOfdmSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    OfdmSrc *ofdmSrc;
    for (i = 0; i < NEXUS_MAX_FRONTENDS; i++) {
        if (!ipStreamerGlobalCtx->ofdmSrcList[i].frontendHandle) {
            /* reached 1st NULL frontendHandle, so we dont have a free tuner to use */
            break;
        }
        ofdmSrc = &ipStreamerGlobalCtx->ofdmSrcList[i];
        if (ofdmSrc->psiAcquiredEvent)
            BKNI_DestroyEvent(ofdmSrc->psiAcquiredEvent);
        if (ofdmSrc->signalLockedEvent)
            BKNI_DestroyEvent(ofdmSrc->signalLockedEvent);
        if (ofdmSrc->lock)
            BKNI_DestroyMutex(ofdmSrc->lock);
    }
    /* global reset */
    memset(ipStreamerGlobalCtx->ofdmSrcList, 0, sizeof(ipStreamerGlobalCtx->ofdmSrcList));

    if (ipStreamerGlobalCtx->ofdmSrcMutex)
        BKNI_DestroyMutex(ipStreamerGlobalCtx->ofdmSrcMutex);
    BDBG_MSG(("OFDM Frontend Sources are uninitialized"));
}

static void
ofdmLockCallback(void *context, int param)
{
    OfdmSrc *ofdmSrc = (OfdmSrc *)context;
    NEXUS_FrontendHandle frontendHandle;
    NEXUS_FrontendFastStatus fastStatus;
    NEXUS_Error statusResult;

    BDBG_ASSERT(ofdmSrc);
    BSTD_UNUSED(param);
    frontendHandle = ofdmSrc->frontendHandle;

    if ((statusResult = NEXUS_Frontend_GetFastStatus(frontendHandle, &fastStatus)) != NEXUS_SUCCESS) {
        if (BERR_NOT_SUPPORTED == statusResult) {
            /* coverity[stack_use_local_overflow] */
            NEXUS_FrontendOfdmStatus ofdmStatus;
            if (NEXUS_Frontend_GetOfdmStatus(frontendHandle, &ofdmStatus) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: Failed to get OFDM status, frontend %p", __FUNCTION__, frontendHandle));
                return;
            }
            else {
                fastStatus.lockStatus = ofdmStatus.receiverLock ? NEXUS_FrontendLockStatus_eLocked : NEXUS_FrontendLockStatus_eUnlocked;
            }
        }
        else
        {
            BDBG_ERR(("%s: unrecognized NEXUS_Frontend_GetFastStatus, frontend %p", __FUNCTION__, frontendHandle));
            return;
        }
    }
    if (fastStatus.lockStatus == NEXUS_FrontendLockStatus_eLocked) {
        BDBG_MSG(("%s: OFDM frontent locked: frontend %p",  __FUNCTION__, frontendHandle));
        BKNI_SetEvent(ofdmSrc->signalLockedEvent);
    }
    else if (fastStatus.lockStatus == NEXUS_FrontendLockStatus_eUnlocked) {
        BDBG_MSG(("%s: frontend %p is unlocked", __FUNCTION__, frontendHandle));
    }
    else if (fastStatus.lockStatus == NEXUS_FrontendLockStatus_eNoSignal) {
        BDBG_ERR(("%s: no signal at this freq for frontend %p", __FUNCTION__, frontendHandle));
    }
    else {
        BDBG_ERR(("%s: unrecognized NEXUS_Frontend_GetFastStatus, frontend %p", __FUNCTION__, frontendHandle));
    }
}

#ifdef NEXUS_HAS_VIDEO_ENCODER
/* select a OFDM src based on the requested frequency: w/ xcode */
int
openNexusOfdmSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    unsigned i;
    OfdmSrc *ofdmSrc = NULL;
    OfdmSrc *ofdmSrcList = NULL;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    ParserBand *parserBandPtr = NULL;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    B_PlaybackIpPsiInfo *cachedPsi = NULL;
    int numProgramsFound = 0;
    int frontendIndex = 0;

    if (ipStreamerCfg->transcodeEnabled) {
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }

    ofdmSrcList = &ipStreamerCtx->globalCtx->ofdmSrcList[0];
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->ofdmSrcMutex);
    /* reuse frontend & pb only if transcode session is enabled on the current & new session */
    /* copy psi info if freq is same */
    for (i = 0; i < NEXUS_MAX_FRONTENDS; i++) {
        if (!ofdmSrcList[i].frontendHandle) {
            /* reached 1st NULL frontendHandle, so we dont have a free tuner to use */
            break;
        }
        BDBG_MSG(("frequency %lu, src entry freq %d, skipPsiAcquisition %d", ipStreamerCfg->frequency, ofdmSrcList[i].frequency, ipStreamerCfg->skipPsiAcquisition));
        if (ofdmSrcList[i].frequency == ipStreamerCfg->frequency) {
            /* a tuner is either currently or was already receiving streams for this frequency */
            if (!ipStreamerCfg->skipPsiAcquisition && ofdmSrcList[i].psiAcquiring) {
                /* check if some other thread is in the process of doing psi discovery on this OFDM channel */
                /* if so, let that thread finish getting PSI and then proceed */
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ofdmSrcMutex);
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                BDBG_MSG(("CTX %p: Another thread is acquiring the PSI info, waiting for its completion...", ipStreamerCtx));
                if (BKNI_WaitForEvent(ofdmSrcList[i].psiAcquiredEvent, 30000)) {
                    BDBG_ERR(("%s: timeout while waiting for PSI acquisition by another thread", __FUNCTION__));
                    return -1;
                }
                if (ipStreamerCfg->transcodeEnabled && ofdmSrcList[i].transcodeEnabled) {
                    /* in xcode case, sleep to allow other thread finish setting up */
                    BDBG_MSG(("%s: delay runnig this thread ", __FUNCTION__));
                    BKNI_Sleep(200);
                }
                BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                BKNI_AcquireMutex(ipStreamerCtx->globalCtx->ofdmSrcMutex);
            }
            if (ofdmSrcList[i].numProgramsFound) {
                /* PSI info is available, set flag to skip it for this session and copy it from this */
                cachedPsi = ofdmSrcList[i].psi;
                numProgramsFound = ofdmSrcList[i].numProgramsFound;
                BDBG_MSG(("%s: freq matched to previously tuned channels, skip PSI acquisition and reuse it from cached copy", __FUNCTION__));
            }

            if (ipStreamerCfg->transcodeEnabled && ofdmSrcList[i].transcodeEnabled) {
                parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i% NEXUS_NUM_PARSER_BANDS];

                if (parserBandPtr->subChannel == ipStreamerCfg->subChannel &&
                parserBandPtr->transcode.outVideoCodec == ipStreamerCfg->transcode.outVideoCodec &&
                parserBandPtr->transcode.outAudioCodec == ipStreamerCfg->transcode.outAudioCodec &&
                parserBandPtr->transcode.outWidth == ipStreamerCfg->transcode.outWidth &&
                parserBandPtr->transcode.outHeight == ipStreamerCfg->transcode.outHeight
                ) {
                    /* we only reuse the frontend if this frontend already is being used for transcoding session and */
                    /* new session is also a transcoding session on the same frequency */
                    ofdmSrc = &ofdmSrcList[i];
                    frontendIndex = i; /* we use the same index for frontend, input band & parser band as IB & PB needs to have 1:1 mapping */
                    BDBG_MSG(("%s: reusing frontend index %d for xcode session", __FUNCTION__, i));
                    break;
                }
            }
        }
        if (!ofdmSrcList[i].refCount && !ofdmSrc) {
            /* save a pointer if we haven't yet found a free unused tuner */
            ofdmSrc = &ofdmSrcList[i];
            frontendIndex = i; /* we use the same index for frontend, input band & parser band as IB & PB needs to have 1:1 mapping */
        }
    }
    if (ofdmSrc) {
        BKNI_AcquireMutex(ofdmSrc->lock);
        ofdmSrc->refCount++;
        ofdmSrc->frequency = ipStreamerCfg->frequency;
        if (!ipStreamerCfg->skipPsiAcquisition) {
            if (!numProgramsFound) {
                /* force PSI acquisition as this src is being used to receive a new channel */
                ofdmSrc->numProgramsFound = 0;
                /* set flag to indicate this thread is getting PSI */
                ofdmSrc->psiAcquiring = true;
            }
            else {
                memcpy(ofdmSrc->psi, cachedPsi, sizeof(B_PlaybackIpPsiInfo)*MAX_PROGRAMS_PER_FREQUENCY);
                ofdmSrc->numProgramsFound = numProgramsFound;
                BDBG_MSG(("%s: PSI data reused from cached copy", __FUNCTION__));
            }
        }
        BKNI_ReleaseMutex(ofdmSrc->lock);
        BDBG_MSG(("CTX %p: Found a free OFDM Frontend %p, index %d, OFDM src %p, use it for %luMhz frequency, refCount %d", ipStreamerCtx, ofdmSrc->frontendHandle, frontendIndex, ofdmSrc, ipStreamerCfg->frequency, ofdmSrc->refCount));
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ofdmSrcMutex);
    if (!ofdmSrc) {
        BDBG_WRN(("%s: No Free OFDM tuner available for this %d frequency all turners (# %lu) busy", __FUNCTION__, ipStreamerCfg->frequency, i));
        goto error;
    }
    ipStreamerCtx->ofdmSrc = ofdmSrc;

    /* setup a callback to know when tuner is locked */
    NEXUS_Frontend_GetDefaultOfdmSettings(&ipStreamerCtx->ofdmSettings);
    ipStreamerCtx->ofdmSettings.frequency = ipStreamerCfg->frequency;
    ipStreamerCtx->ofdmSettings.bandwidth = ipStreamerCfg->bandwidth;
    ipStreamerCtx->ofdmSettings.mode = ipStreamerCfg->ofdmMode;
    ipStreamerCtx->ofdmSettings.cciMode = ipStreamerCfg->cciMode;
    ipStreamerCtx->ofdmSettings.priority = ipStreamerCfg->ofdmPriority;
    switch (ipStreamerCfg->ofdmMode) {
        case NEXUS_FrontendOfdmMode_eDvbt:
            ipStreamerCtx->ofdmSettings.manualTpsSettings = false;
            ipStreamerCtx->ofdmSettings.pullInRange = NEXUS_FrontendOfdmPullInRange_eWide;
            ipStreamerCtx->ofdmSettings.cciMode = NEXUS_FrontendOfdmCciMode_eNone;;
            break;
        case NEXUS_FrontendOfdmMode_eDvbt2:
            ipStreamerCtx->ofdmSettings.dvbt2Settings.plpMode = false;
            ipStreamerCtx->ofdmSettings.dvbt2Settings.plpId = 0;
            break;
        default:
            break;
    }
    ipStreamerCtx->ofdmSettings.acquisitionMode = NEXUS_FrontendOfdmAcquisitionMode_eAuto;
    ipStreamerCtx->ofdmSettings.terrestrial = ipStreamerCfg->terrestrial;
    ipStreamerCtx->ofdmSettings.spectrum = NEXUS_FrontendOfdmSpectrum_eAuto;
#if 0
    /* TODO add async callback later */
    ipStreamerCtx->ofdmSettings.asyncStatusReadyCallback.callback = async_status_ready_callback;
    ipStreamerCtx->ofdmSettings.asyncStatusReadyCallback.context = statusEvent;
#endif
    ipStreamerCtx->ofdmSettings.lockCallback.callback = ofdmLockCallback;
    ipStreamerCtx->ofdmSettings.lockCallback.context = ofdmSrc;
    ipStreamerCtx->ofdmSettings.lockCallback.param = ipStreamerCfg->srcType;

    BDBG_MSG(("%s: OFDM Freq %lu, bandwidth %d, mode %d", __FUNCTION__, ipStreamerCtx->ofdmSettings.frequency, ipStreamerCtx->ofdmSettings.bandwidth, ipStreamerCtx->ofdmSettings.mode));

    /* associate parser band to the input band associated with this tuner */
    ipStreamerCtx->parserBandPtr = NULL;
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->parserBandMutex);
    /* 1:1 mapping between IB & PB */
    parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[frontendIndex% NEXUS_NUM_PARSER_BANDS];

    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandMutex);
    if (parserBandPtr == NULL) {
        BDBG_ERR(("Failed to find a free parser band at %d", __LINE__));
        goto error;
    }

    NEXUS_Frontend_GetUserParameters(ofdmSrc->frontendHandle, &userParams);
    BKNI_AcquireMutex(parserBandPtr->lock);
    if (parserBandPtr->refCount == 0) {
        parserBandPtr->frequency = ipStreamerCfg->frequency;
        parserBandPtr->subChannel = ipStreamerCfg->subChannel;
        /* associate parser band to the input band associated with this tuner */
        /* Map this parser band to the demod's input band. */
        NEXUS_ParserBand_GetSettings(parserBandPtr->parserBand, &parserBandSettings);
        if (userParams.isMtsif) {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
            parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(ofdmSrc->frontendHandle); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
        }
        else {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
            parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
        }
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        rc = NEXUS_ParserBand_SetSettings(parserBandPtr->parserBand, &parserBandSettings);
        if (rc) {
            BDBG_ERR(("Failed to set the Nexus Parser band settings for band # %d", parserBandPtr->parserBand));
            BKNI_ReleaseMutex(parserBandPtr->lock);
            goto error;
        }
        BDBG_MSG(("Nexus Parser band band # %d is setup (ref cnt %d)", parserBandPtr->parserBand, parserBandPtr->refCount));
    }
    else {
        BDBG_MSG(("Nexus Parser band band # %d is already setup (ref cnt %d)", parserBandPtr->parserBand, parserBandPtr->refCount));
    }
    parserBandPtr->refCount++;
    BKNI_ReleaseMutex(parserBandPtr->lock);
    if (ipStreamerCfg->transcodeEnabled) {
        if (parserBandPtr->transcodeEnabled && parserBandPtr->transcoderDst->refCount) {
            if (parserBandPtr->subChannel != ipStreamerCfg->subChannel ||
                parserBandPtr->transcode.outVideoCodec != ipStreamerCfg->transcode.outVideoCodec ||
                parserBandPtr->transcode.outHeight != ipStreamerCfg->transcode.outHeight) {
                BDBG_ERR(("%s: ERROR: Limiting to one transcoding sesion: Transcoder pipe (# of users %d) is already opened/enabled for this Live Channel", __FUNCTION__, parserBandPtr->transcoderDst->refCount));
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                goto error;
            }
            BDBG_MSG(("%s: Transcoder pipe (# of users %d) is already opened/enabled for this Live Channel", __FUNCTION__, parserBandPtr->transcoderDst->refCount));
            parserBandPtr->transcoderDst->refCount++;
            ipStreamerCtx->transcoderDst = parserBandPtr->transcoderDst;

        }
        else {
            if ((parserBandPtr->transcoderDst = openNexusTranscoderPipe(ipStreamerCfg, ipStreamerCtx)) == NULL) {
                BDBG_ERR(("%s: Failed to open the transcoder pipe", __FUNCTION__));
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                goto error;
            }
            parserBandPtr->transcodeEnabled = true;
            ofdmSrc->transcodeEnabled = true;
            parserBandPtr->transcode = ipStreamerCfg->transcode;
            parserBandPtr->subChannel = ipStreamerCfg->subChannel;
        }
        ipStreamerCtx->parserBandPtr = parserBandPtr;
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
    else {
        /* no transcode case */
        ipStreamerCtx->parserBandPtr = parserBandPtr;
    }
    BDBG_MSG(("CTX %p: OFDM Frontend Src %p (ref count %d), Input Band %d & Parser Band %d (ref count %d) are opened",
                ipStreamerCtx, ofdmSrc, ofdmSrc->refCount, userParams.param1, ipStreamerCtx->parserBandPtr->parserBand, ipStreamerCtx->parserBandPtr->refCount));
    return 0;

error:
    if (parserBandPtr) {
        BKNI_AcquireMutex(parserBandPtr->lock);
        parserBandPtr->refCount--;
        BKNI_ReleaseMutex(parserBandPtr->lock);
    }
    if (ipStreamerCfg->transcodeEnabled) {
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
    if (!ofdmSrc) return -1;
    BKNI_AcquireMutex(ofdmSrc->lock);
    ofdmSrc->refCount--;
    ofdmSrc->psiAcquiring = false;
    BKNI_ReleaseMutex(ofdmSrc->lock);
    return -1;
}
#else /* !NEXUS_HAS_VIDEO_ENCODER */
/* select a OFDM src based on the requested frequency: w/o xcode */
int
openNexusOfdmSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    unsigned i;
    OfdmSrc *ofdmSrc = NULL;
    OfdmSrc *ofdmSrcList = NULL;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    ParserBand *parserBandPtr = NULL;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    B_PlaybackIpPsiInfo *cachedPsi = NULL;
    int numProgramsFound = 0;
    int frontendIndex = 0;

    ofdmSrcList = &ipStreamerCtx->globalCtx->ofdmSrcList[0];
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->ofdmSrcMutex);
    /* reuse frontend & pb only if transcode session is enabled on the current & new session */
    /* copy psi info if freq is same */
    for (i = 0; i < NEXUS_MAX_FRONTENDS; i++) {
        if (!ofdmSrcList[i].frontendHandle) {
            /* reached 1st NULL frontendHandle, so we dont have a free tuner to use */
            break;
        }
        BDBG_MSG(("frequency %lu, src entry freq %d, skipPsiAcquisition %d", ipStreamerCfg->frequency, ofdmSrcList[i].frequency, ipStreamerCfg->skipPsiAcquisition));
        if (ofdmSrcList[i].frequency == ipStreamerCfg->frequency) {
            /* a tuner is either currently or was already receiving streams for this frequency */
            if (!ipStreamerCfg->skipPsiAcquisition && ofdmSrcList[i].psiAcquiring) {
                /* check if some other thread is in the process of doing psi discovery on this OFDM channel */
                /* if so, let that thread finish getting PSI and then proceed */
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ofdmSrcMutex);
                BDBG_MSG(("CTX %p: Another thread is acquiring the PSI info, waiting for its completion...", ipStreamerCtx));
                if (BKNI_WaitForEvent(ofdmSrcList[i].psiAcquiredEvent, 30000)) {
                    BDBG_ERR(("%s: timeout while waiting for PSI acquisition by another thread", __FUNCTION__));
                    return -1;
                }
                BKNI_AcquireMutex(ipStreamerCtx->globalCtx->ofdmSrcMutex);
            }
            if (ofdmSrcList[i].numProgramsFound) {
                /* PSI info is available, set flag to skip it for this session and copy it from this */
                cachedPsi = ofdmSrcList[i].psi;
                numProgramsFound = ofdmSrcList[i].numProgramsFound;
                BDBG_MSG(("%s: freq matched to previously tuned channels, skip PSI acquisition and reuse it from cached copy", __FUNCTION__));
            }
        }
        if (!ofdmSrcList[i].refCount && !ofdmSrc) {
            /* save a pointer if we haven't yet found a free unused tuner */
            ofdmSrc = &ofdmSrcList[i];
            frontendIndex = i; /* we use the same index for frontend, input band & parser band as IB & PB needs to have 1:1 mapping */
        }
    }
    if (ofdmSrc) {
        BKNI_AcquireMutex(ofdmSrc->lock);
        ofdmSrc->refCount++;
        ofdmSrc->frequency = ipStreamerCfg->frequency;
        if (!ipStreamerCfg->skipPsiAcquisition) {
        if (!numProgramsFound) {
            /* force PSI acquisition as this src is being used to receive a new channel */
            ofdmSrc->numProgramsFound = 0;
            /* set flag to indicate this thread is getting PSI */
            ofdmSrc->psiAcquiring = true;
        }
        else {
            memcpy(ofdmSrc->psi, cachedPsi, sizeof(B_PlaybackIpPsiInfo)*MAX_PROGRAMS_PER_FREQUENCY);
            ofdmSrc->numProgramsFound = numProgramsFound;
            BDBG_MSG(("%s: PSI data reused from cached copy", __FUNCTION__));
            }
        }
        BKNI_ReleaseMutex(ofdmSrc->lock);
        BDBG_MSG(("CTX %p: Found a free OFDM Frontend %p, index %d, OFDM src %p, use it for %luMhz frequency, refCount %d", ipStreamerCtx, ofdmSrc->frontendHandle, frontendIndex, ofdmSrc, ipStreamerCfg->frequency, ofdmSrc->refCount));
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ofdmSrcMutex);
    if (!ofdmSrc) {
        BDBG_ERR(("%s: No Free tuner available for this %lu frequency all turners (# %d) busy", __FUNCTION__, ipStreamerCfg->frequency, i));
        goto error;
    }
    ipStreamerCtx->ofdmSrc = ofdmSrc;

    /* setup a callback to know when tuner is locked */
    NEXUS_Frontend_GetDefaultOfdmSettings(&ipStreamerCtx->ofdmSettings);
    ipStreamerCtx->ofdmSettings.frequency = ipStreamerCfg->frequency;
    ipStreamerCtx->ofdmSettings.bandwidth = ipStreamerCfg->bandwidth;
    ipStreamerCtx->ofdmSettings.mode = ipStreamerCfg->ofdmMode;
    switch (ipStreamerCfg->ofdmMode) {
        case NEXUS_FrontendOfdmMode_eDvbt:
            ipStreamerCtx->ofdmSettings.manualTpsSettings = false;
            ipStreamerCtx->ofdmSettings.pullInRange = NEXUS_FrontendOfdmPullInRange_eWide;
            ipStreamerCtx->ofdmSettings.cciMode = NEXUS_FrontendOfdmCciMode_eNone;;
            break;
        case NEXUS_FrontendOfdmMode_eDvbt2:
            ipStreamerCtx->ofdmSettings.dvbt2Settings.plpMode = false;
            ipStreamerCtx->ofdmSettings.dvbt2Settings.plpId = 0;
            break;
        default:
            break;
    }
    ipStreamerCtx->ofdmSettings.acquisitionMode = NEXUS_FrontendOfdmAcquisitionMode_eAuto;
    ipStreamerCtx->ofdmSettings.terrestrial = ipStreamerCfg->terrestrial;
    ipStreamerCtx->ofdmSettings.spectrum = NEXUS_FrontendOfdmSpectrum_eAuto;
#if 0
    /* TODO add async callback later */
    ipStreamerCtx->ofdmSettings.asyncStatusReadyCallback.callback = async_status_ready_callback;
    ipStreamerCtx->ofdmSettings.asyncStatusReadyCallback.context = statusEvent;
#endif
    ipStreamerCtx->ofdmSettings.lockCallback.callback = ofdmLockCallback;
    ipStreamerCtx->ofdmSettings.lockCallback.context = ofdmSrc;
    ipStreamerCtx->ofdmSettings.lockCallback.param = ipStreamerCfg->srcType;

    BDBG_MSG(("%s: OFDM Freq %lu, bandwidth %d, mode %d", __FUNCTION__, ipStreamerCtx->ofdmSettings.frequency, ipStreamerCtx->ofdmSettings.bandwidth, ipStreamerCtx->ofdmSettings.mode));

    /* associate parser band to the input band associated with this tuner */
    ipStreamerCtx->parserBandPtr = NULL;
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->parserBandMutex);
#define OOB_TUNER_SUPPORT
#ifdef OOB_TUNER_SUPPORT
    /* only the last 2 parser bands are available on 7346 in this case, first 8 are reserved for MTSIF input */
    for (i = IP_STREAMER_NUM_PARSER_BANDS-2; i < IP_STREAMER_NUM_PARSER_BANDS; i++) {
        /* pick an unused parser band */
        if (!parserBandPtr && ipStreamerCtx->globalCtx->parserBandList[i].refCount == 0) {
            /* found 1st unused entry */
            parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i];
            break;
        }
    }
    BDBG_MSG(("%s: using parserBand # %d", __FUNCTION__, i));
#else
    /* pick an unused parser band */
    parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[frontendIndex % NEXUS_NUM_PARSER_BANDS];

#endif
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandMutex);
    if (parserBandPtr == NULL) {
        BDBG_ERR(("Failed to find a free parser band at %d", __LINE__));
        goto error;
    }

    NEXUS_Frontend_GetUserParameters(ofdmSrc->frontendHandle, &userParams);
    BKNI_AcquireMutex(parserBandPtr->lock);
    if (parserBandPtr->refCount == 0) {
        parserBandPtr->frequency = ipStreamerCfg->frequency;
        parserBandPtr->subChannel = ipStreamerCfg->subChannel;
        /* associate parser band to the input band associated with this tuner */
        /* Map this parser band to the demod's input band. */
        NEXUS_ParserBand_GetSettings(parserBandPtr->parserBand, &parserBandSettings);
        if (userParams.isMtsif) {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
            parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(ofdmSrc->frontendHandle); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
        }
        else {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
            parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
        }
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        rc = NEXUS_ParserBand_SetSettings(parserBandPtr->parserBand, &parserBandSettings);
        if (rc) {
            BDBG_ERR(("Failed to set the Nexus Parser band settings for band # %d", parserBandPtr->parserBand));
            BKNI_ReleaseMutex(parserBandPtr->lock);
            goto error;
        }
        BDBG_MSG(("Nexus Parser band band # %d is setup (ref cnt %d)", parserBandPtr->parserBand, parserBandPtr->refCount));
    }
    else {
        BDBG_MSG(("Nexus Parser band band # %d is already setup (ref cnt %d)", parserBandPtr->parserBand, parserBandPtr->refCount));
    }
    parserBandPtr->refCount++;
    ipStreamerCtx->parserBandPtr = parserBandPtr;
    BKNI_ReleaseMutex(parserBandPtr->lock);
    BDBG_MSG(("CTX %p: OFDM Frontend Src %p (ref count %d), Input Band %d & Parser Band %d (ref count %d) are opened",
                ipStreamerCtx, ofdmSrc, ofdmSrc->refCount, userParams.param1, ipStreamerCtx->parserBandPtr->parserBand, ipStreamerCtx->parserBandPtr->refCount));
    return 0;

error:
    if (parserBandPtr) {
        BKNI_AcquireMutex(parserBandPtr->lock);
        parserBandPtr->refCount--;
        BKNI_ReleaseMutex(parserBandPtr->lock);
    }
    if (!ofdmSrc) return -1;
    BKNI_AcquireMutex(ofdmSrc->lock);
    ofdmSrc->refCount--;
    ofdmSrc->psiAcquiring = false;
    BKNI_ReleaseMutex(ofdmSrc->lock);
    return -1;
}
#endif

void
closeNexusOfdmSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    OfdmSrc *ofdmSrc = ipStreamerCtx->ofdmSrc;

#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCtx->transcoderDst) {
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        closeNexusTranscoderPipe(ipStreamerCtx, ipStreamerCtx->transcoderDst);
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        if (ipStreamerCtx->transcoderDst->refCount == 0) {
            /* clear the transcoding enabled flag in the parse band */
            ipStreamerCtx->parserBandPtr->transcodeEnabled = false;
            ofdmSrc->transcodeEnabled = false;
        }
    }
#endif
    /* free up the parser band */
    if (ipStreamerCtx->parserBandPtr) {
        BKNI_AcquireMutex(ipStreamerCtx->parserBandPtr->lock);
        ipStreamerCtx->parserBandPtr->refCount--;
        BKNI_ReleaseMutex(ipStreamerCtx->parserBandPtr->lock);
        BDBG_MSG(("CTX %p: Closed a parser band src %p, refCount %d", ipStreamerCtx, ipStreamerCtx->parserBandPtr, ipStreamerCtx->parserBandPtr->refCount));
    }

    if (!ofdmSrc || !ofdmSrc->refCount)
        return;
    BDBG_WRN(("%s: refCount %d", __FUNCTION__, ofdmSrc->refCount));

    ipStreamerCtx->ofdmSrc = NULL;
    BKNI_AcquireMutex(ofdmSrc->lock);
    ofdmSrc->refCount--;
    if (ofdmSrc->psiAcquiredEvent)
        BKNI_ResetEvent(ofdmSrc->psiAcquiredEvent);
    BKNI_ReleaseMutex(ofdmSrc->lock);
    BDBG_MSG(("CTX %p: Closed a OFDM src %p, refCount %d", ipStreamerCtx, ofdmSrc, ofdmSrc->refCount));
}

int
setupAndAcquirePsiInfoOfdmSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx,
    B_PlaybackIpPsiInfo *psiOut
    )
{
    int i;
    OfdmSrc *ofdmSrc;
    psiCollectionDataType  collectionData;

    memset(&collectionData, 0, sizeof(psiCollectionDataType));
    memset(psiOut, 0, sizeof(B_PlaybackIpPsiInfo));
    collectionData.srcType = ipStreamerCfg->srcType;
    collectionData.live = true;
    collectionData.parserBand = ipStreamerCtx->parserBandPtr->parserBand;
    collectionData.ofdmSettings = &ipStreamerCtx->ofdmSettings;
    collectionData.frontend = ipStreamerCtx->ofdmSrc->frontendHandle;
    collectionData.signalLockedEvent = ipStreamerCtx->ofdmSrc->signalLockedEvent;
    ofdmSrc = ipStreamerCtx->ofdmSrc;

    if (ofdmSrc->numProgramsFound == 0) {
        /* PSI hasn't yet been acquired */
        BDBG_MSG(("CTX %p: Acquire Psi Info..., PB band %d", ipStreamerCtx, collectionData.parserBand));

        /* get the PSI, this can take several seconds ... */
        acquirePsiInfo(&collectionData, &ofdmSrc->psi[0], &ofdmSrc->numProgramsFound);

        /* tell any other waiting thread that we are done acquiring PSI */
        BDBG_MSG(("%s: ofdmSrc %p, lock %p", __FUNCTION__, ofdmSrc, ofdmSrc->lock));
        BKNI_AcquireMutex(ofdmSrc->lock);
        ofdmSrc->psiAcquiring = false;
        BKNI_SetEvent(ofdmSrc->psiAcquiredEvent);
        BKNI_ReleaseMutex(ofdmSrc->lock);
        if (ofdmSrc->numProgramsFound == 0) {
            BDBG_ERR(("%s: ERROR: Unable to Acquire PSI!!", __FUNCTION__));
            return -1;
        }
    }
    else {
        BDBG_MSG(("CTX %p: Psi Info is already acquired...", ipStreamerCtx));
    }

    if (ipStreamerCfg->subChannel > ofdmSrc->numProgramsFound) {
        BDBG_MSG(("Requested sub-channel # (%d) is not found in the total channels (%d) acquired, defaulting to 1st sub-channel", ipStreamerCfg->subChannel, ofdmSrc->numProgramsFound));
        i = 0;
    }
    else {
        BDBG_MSG(("CTX %p: Requested sub-channel # (%d) is found in the total channels (%d) ", ipStreamerCtx, ipStreamerCfg->subChannel, ofdmSrc->numProgramsFound));
        i = ipStreamerCfg->subChannel - 1; /* sub-channels start from 1, where as psi table starts from 0 */
        if (i < 0) i = 0;
    }
    memcpy(psiOut, &ofdmSrc->psi[i], sizeof(B_PlaybackIpPsiInfo));
    BKNI_ResetEvent(ofdmSrc->signalLockedEvent);
    return 0;
}


int
startNexusOfdmSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    OfdmSrc *ofdmSrc = ipStreamerCtx->ofdmSrc;

    BKNI_AcquireMutex(ofdmSrc->lock);
    if (ofdmSrc->started) {
        BKNI_ReleaseMutex(ofdmSrc->lock);
        BDBG_MSG(("CTX %p: OFDM Src %p is already started, refCount %d", ipStreamerCtx, ofdmSrc, ofdmSrc->refCount));
        return 0;
    }

    /* start frontend */
    BKNI_ResetEvent(ofdmSrc->signalLockedEvent);
    NEXUS_StartCallbacks(ofdmSrc->frontendHandle);
    rc = NEXUS_Frontend_TuneOfdm(ofdmSrc->frontendHandle, &ipStreamerCtx->ofdmSettings);
    if (rc) {
        BKNI_ReleaseMutex(ofdmSrc->lock);
        BDBG_ERR(("Failed to Tune to OFDM Frontend"));
        return -1;
    }
    if (BKNI_WaitForEvent(ofdmSrc->signalLockedEvent, 5000)) {
        BKNI_ReleaseMutex(ofdmSrc->lock);
        BDBG_MSG(("CTX %p: OFDM Src %p failed to lock the signal ...", ipStreamerCtx, ofdmSrc));
        return -1;
    }

    ofdmSrc->started = true;
    BKNI_ReleaseMutex(ofdmSrc->lock);
    BDBG_MSG(("CTX %p: OFDM Src %p is started ...", ipStreamerCtx, ofdmSrc));
    return 0;
}

void
stopNexusOfdmSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    OfdmSrc *ofdmSrc = ipStreamerCtx->ofdmSrc;
    int refCount;

    BKNI_AcquireMutex(ofdmSrc->lock);
    refCount = ofdmSrc->refCount;
    BKNI_ReleaseMutex(ofdmSrc->lock);

    if (refCount == 1) {
        NEXUS_StopCallbacks(ofdmSrc->frontendHandle);
        NEXUS_Frontend_Untune(ofdmSrc->frontendHandle);
        ofdmSrc->started = false;
        BDBG_MSG(("CTX %p: OFDM Frontend Src is stopped ...", ipStreamerCtx));
    }
    else {
        BDBG_MSG(("CTX %p: OFDM Frontend Src %p is not stopped, ref count %d ...", ipStreamerCtx, ofdmSrc, refCount));
    }
}
#else /* !NEXUS_HAS_FRONTEND */
/* stub functions */
int initNexusOfdmSrcList(IpStreamerGlobalCtx *ipStreamerGlobalCtx)
{
    BSTD_UNUSED(ipStreamerGlobalCtx);

    BDBG_ERR(("%s: Streaming from OFDM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    /* note: we let the init call succeed, but we will fail the open call */
    /* this way app wont fail during startup, but will fail a client request to open/start a recording */
    return 0;
}
void unInitNexusOfdmSrcList(IpStreamerGlobalCtx *ipStreamerGlobalCtx)
{
    BSTD_UNUSED(ipStreamerGlobalCtx);

    BDBG_ERR(("%s: Streaming from OFDM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
}
int openNexusOfdmSrc(IpStreamerConfig *ipStreamerCfg, IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCfg);
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from OFDM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    return -1;
}
void closeNexusOfdmSrc(IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from OFDM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
}
int setupAndAcquirePsiInfoOfdmSrc(IpStreamerConfig *ipStreamerCfg, IpStreamerCtx *ipStreamerCtx, B_PlaybackIpPsiInfo *psiOut)
{
    BSTD_UNUSED(psiOut);
    BSTD_UNUSED(ipStreamerCfg);
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from OFDM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    return -1;
}
int startNexusOfdmSrc(IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from OFDM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    return -1;
}
int stopNexusOfdmSrc(IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from OFDM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    return -1;
}
#endif /* NEXUS_HAS_FRONTEND */
