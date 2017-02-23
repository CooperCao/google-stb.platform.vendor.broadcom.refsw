/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *  main test app for ip_streamer
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
/* build a list of all available qam sources */
/* Note: for VMS configuration, local tuners (aka sources) == # of decoders are left for local decoding app */
int
initNexusQamSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    unsigned i, j;
    int qamTunerCount = 0;
    QamSrc *qamSrc;
    NEXUS_FrontendHandle frontendHandle=NULL;
    NEXUS_PlatformConfiguration platformConfig;

    /* global reset */
    memset(ipStreamerGlobalCtx->qamSrcList, 0, sizeof(ipStreamerGlobalCtx->qamSrcList));

    if (BKNI_CreateMutex(&ipStreamerGlobalCtx->qamSrcMutex) != 0) {
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
            settings.capabilities.qam = true;
            frontendHandle = NEXUS_Frontend_Acquire(&settings);
            if (!frontendHandle) {
                BDBG_MSG(("%s: NULL Frontend (# %d)", __FUNCTION__, i));
                continue;
            }
        } else {
            frontendHandle = platformConfig.frontend[i];
            if (!frontendHandle) {
                BDBG_MSG(("%s: NULL Frontend (# %d)", __FUNCTION__, i));
                continue;
            }
        }

        NEXUS_Frontend_GetCapabilities(frontendHandle, &capabilities);
        if (capabilities.qam) {
            qamTunerCount++;
            /* Found a QAM source */
            qamSrc = &ipStreamerGlobalCtx->qamSrcList[j];
            memset(qamSrc, 0, sizeof(QamSrc));
            qamSrc->frontendHandle = frontendHandle;
            if (BKNI_CreateEvent(&qamSrc->psiAcquiredEvent)) {
                BDBG_ERR(("Failed to psiAcquired event at %d", __LINE__));
                return -1;
            }
            if (BKNI_CreateEvent(&qamSrc->signalLockedEvent)) {
                BDBG_ERR(("Failed to signalLocked event at %d", __LINE__));
                return -1;
            }
            if (BKNI_CreateMutex(&qamSrc->lock) != 0) {
                BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
                return -1;
            }
            /* successfully setup a qam frontend src */
            j++;
        }
        else {
            /* skip other tuner types */
            continue;
        }
    }

    if (!j) {
        BDBG_MSG(("NO QAM Front End founds, check Nexus platform configuration"));
        return 0;
    }
    BDBG_MSG(("Initialized %d QAM Frontend Sources", j));
    return 0;
}

void
unInitNexusQamSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    QamSrc *qamSrc;
    for (i = 0; i < NEXUS_MAX_FRONTENDS; i++) {
        if (!ipStreamerGlobalCtx->qamSrcList[i].frontendHandle) {
            /* reached 1st NULL frontendHandle, so we dont have a free tuner to use */
            break;
        }
        qamSrc = &ipStreamerGlobalCtx->qamSrcList[i];
        if (qamSrc->psiAcquiredEvent)
            BKNI_DestroyEvent(qamSrc->psiAcquiredEvent);
        if (qamSrc->signalLockedEvent)
            BKNI_DestroyEvent(qamSrc->signalLockedEvent);
        if (qamSrc->lock)
            BKNI_DestroyMutex(qamSrc->lock);
    }
    /* global reset */
    memset(ipStreamerGlobalCtx->qamSrcList, 0, sizeof(ipStreamerGlobalCtx->qamSrcList));

    if (ipStreamerGlobalCtx->qamSrcMutex)
        BKNI_DestroyMutex(ipStreamerGlobalCtx->qamSrcMutex);
    BDBG_MSG(("QAM Frontend Sources are uninitialized"));
}

static void
qamLockCallback(void *context, int param)
{
    QamSrc *qamSrc = (QamSrc *)context;
    NEXUS_FrontendHandle frontendHandle;
    NEXUS_FrontendFastStatus fastStatus;
    NEXUS_Error statusResult;

    BDBG_ASSERT(qamSrc);
    BSTD_UNUSED(param);
    frontendHandle = qamSrc->frontendHandle;

    if ((statusResult = NEXUS_Frontend_GetFastStatus(frontendHandle, &fastStatus)) != NEXUS_SUCCESS) {

        if(BERR_NOT_SUPPORTED == statusResult ) {
            NEXUS_FrontendQamStatus qamStatus;

            if(NEXUS_Frontend_GetQamStatus(frontendHandle, &qamStatus) != NEXUS_SUCCESS ) {
                BDBG_ERR(("%s: Failed to get QAM status, frontend %p", __FUNCTION__, (void *)frontendHandle));
                return;
            }
            else
            {
                fastStatus.lockStatus=(qamStatus.receiverLock && qamStatus.fecLock) ? NEXUS_FrontendLockStatus_eLocked : NEXUS_FrontendLockStatus_eUnlocked;
            }

        }
        else
        {
            BDBG_ERR(("%s: unrecognized NEXUS_Frontend_GetFastStatus, frontend %p", __FUNCTION__, (void *)frontendHandle));
            return;
        }
    }
    if (fastStatus.lockStatus == NEXUS_FrontendLockStatus_eLocked) {
        BDBG_MSG(("%s: QAM frontent locked: frontend %p",  __FUNCTION__, (void *)frontendHandle));
        BKNI_SetEvent(qamSrc->signalLockedEvent);
    }
    else if (fastStatus.lockStatus == NEXUS_FrontendLockStatus_eUnlocked) {
        BDBG_MSG(("%s: frontend %p is unlocked", __FUNCTION__, (void *)frontendHandle));
    }
    else if (fastStatus.lockStatus == NEXUS_FrontendLockStatus_eNoSignal) {
        BDBG_ERR(("%s: no signal at this freq for frontend %p", __FUNCTION__, (void *)frontendHandle));
    }
    else {
        BDBG_ERR(("%s: unrecognized NEXUS_Frontend_GetFastStatus, frontend %p", __FUNCTION__, (void *)frontendHandle));
    }
}

#ifdef NEXUS_HAS_VIDEO_ENCODER
/* select a qam src based on the requested QAM frequency */
int
openNexusQamSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    unsigned i;
    QamSrc *qamSrc = NULL;
    QamSrc *qamSrcList = NULL;
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

    qamSrcList = &ipStreamerCtx->globalCtx->qamSrcList[0];
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->qamSrcMutex);
    /* reuse frontend & pb only if transcode session is enabled on the current & new session */
    /* copy psi info if freq is same */
    for (i = 0; i < NEXUS_MAX_FRONTENDS; i++) {
        if (!qamSrcList[i].frontendHandle) {
            /* reached 1st NULL frontendHandle, so we dont have a free tuner to use */
            break;
        }
        BDBG_MSG(("frequency %d, src entry freq %d, refCount %d, skipPsiAcquisition %d", ipStreamerCfg->frequency, qamSrcList[i].frequency, qamSrcList[i].refCount, ipStreamerCfg->skipPsiAcquisition));
#ifndef STREAMER_CABLECARD_SUPPORT
        if (qamSrcList[i].frequency == ipStreamerCfg->frequency) {
            /* a tuner is either currently or was already receiving streams for this frequency */
            if (!ipStreamerCfg->skipPsiAcquisition && qamSrcList[i].psiAcquiring) {
                /* check if some other thread is in the process of doing psi discovery on this qam channel */
                /* if so, let that thread finish getting PSI and then proceed */
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->qamSrcMutex);
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                BDBG_MSG(("CTX %p: Another thread is acquiring the PSI info, waiting for its completion...", (void *)ipStreamerCtx));
                if (BKNI_WaitForEvent(qamSrcList[i].psiAcquiredEvent, 30000)) {
                    BDBG_WRN(("%s: CTX %p: timed out while waiting for other thread to finish acquiring PSI info...", __FUNCTION__, (void *)ipStreamerCtx));
                    rc = -1;
                }
                if (ipStreamerCfg->transcodeEnabled && qamSrcList[i].transcodeEnabled) {
                    /* in xcode case, sleep to allow other thread finish setting up */
                    BDBG_MSG(("%s: delay runnig this thread ", __FUNCTION__));
                    BKNI_Sleep(200);
                }
                BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                BKNI_AcquireMutex(ipStreamerCtx->globalCtx->qamSrcMutex);
            }
            if (qamSrcList[i].numProgramsFound) {
                /* PSI info is available, set flag to skip it for this session and copy it from this */
                cachedPsi = qamSrcList[i].psi;
                numProgramsFound = qamSrcList[i].numProgramsFound;
                BDBG_MSG(("%s: freq matched to previously tuned channels, skip PSI acquisition and reuse it from cached copy", __FUNCTION__));
            }

            if (ipStreamerCfg->transcodeEnabled && qamSrcList[i].transcodeEnabled) {
                parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i % NEXUS_NUM_PARSER_BANDS];

                if (parserBandPtr->subChannel == ipStreamerCfg->subChannel &&
                parserBandPtr->transcode.outVideoCodec == ipStreamerCfg->transcode.outVideoCodec &&
                parserBandPtr->transcode.outAudioCodec == ipStreamerCfg->transcode.outAudioCodec &&
                parserBandPtr->transcode.outWidth == ipStreamerCfg->transcode.outWidth &&
                parserBandPtr->transcode.outHeight == ipStreamerCfg->transcode.outHeight
                ) {
                    /* we only reuse the frontend if this frontend already is being used for transcoding session and */
                    /* new session is also a transcoding session on the same frequency */
                    qamSrc = &qamSrcList[i];
                    frontendIndex = i; /* we use the same index for frontend, input band & parser band as IB & PB needs to have 1:1 mapping */
                    BDBG_WRN(("%s: reusing frontend index %d for xcode session", __FUNCTION__, i));
                    break;
                }
            }
            else {
                parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i % NEXUS_NUM_PARSER_BANDS];
                if (ipStreamerCfg->enableAllpass) {
                    /*we resue the fron end if allpass is enabled.*/
                    qamSrc = &qamSrcList[i];
                    frontendIndex = i; /* we use the same index for frontend, input band & parser band as IB & PB needs to have 1:1 mapping */
                    BDBG_WRN(("%s: reusing frontend index %d for xcode session", __FUNCTION__, i));
                    break;
                }
            }
        }
#endif
        if (!qamSrcList[i].refCount && !qamSrc) {
            /* save a pointer if we haven't yet found a free unused tuner */
            qamSrc = &qamSrcList[i];
            frontendIndex = i; /* we use the same index for frontend, input band & parser band as IB & PB needs to have 1:1 mapping */
        }
    }
    if (qamSrc) {
        BKNI_AcquireMutex(qamSrc->lock);
        qamSrc->refCount++;
        qamSrc->frequency = ipStreamerCfg->frequency;
        if (!ipStreamerCfg->skipPsiAcquisition) {
            if (!numProgramsFound) {
                /* force PSI acquisition as this src is being used to receive a new channel */
                qamSrc->numProgramsFound = 0;
                /* set flag to indicate this thread is getting PSI */
                qamSrc->psiAcquiring = true;
            }
            else {
                if (qamSrc->psi != cachedPsi)
                    /* copy only if cachedPsi is from a different qamSrc */
                    memcpy(qamSrc->psi, cachedPsi, sizeof(B_PlaybackIpPsiInfo)*MAX_PROGRAMS_PER_FREQUENCY);
                qamSrc->numProgramsFound = numProgramsFound;
                BDBG_MSG(("%s: PSI data reused from cached copy", __FUNCTION__));
            }
        }
        BKNI_ReleaseMutex(qamSrc->lock);
        BDBG_MSG(("CTX %p: Found a free Qam Frontend %p, index %d, qamSrc %p, use it for %dhz frequency, refCount %d", (void *)ipStreamerCtx, (void *)qamSrc->frontendHandle, frontendIndex, (void *)qamSrc, ipStreamerCfg->frequency, qamSrc->refCount));
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->qamSrcMutex);
    if (!qamSrc) {
        BDBG_WRN(("%s: No Free tuner available for this %d frequency all turners (# %d) busy", __FUNCTION__, ipStreamerCfg->frequency, i));
        goto error;
    }
    ipStreamerCtx->qamSrc = qamSrc;

    /* setup a callback to know when tuner is locked */
    NEXUS_Frontend_GetDefaultQamSettings(&ipStreamerCtx->qamSettings);
    ipStreamerCtx->qamSettings.frequency = ipStreamerCfg->frequency;
    ipStreamerCtx->qamSettings.mode = ipStreamerCfg->qamMode;
    ipStreamerCtx->qamSettings.annex = ipStreamerCfg->qamAnnex;
    ipStreamerCtx->qamSettings.symbolRate = ipStreamerCfg->symbolRate;
    ipStreamerCtx->qamSettings.lockCallback.callback = qamLockCallback;
    ipStreamerCtx->qamSettings.lockCallback.context = qamSrc;
    ipStreamerCtx->qamSettings.lockCallback.param = ipStreamerCfg->srcType;

    /* associate parser band to the input band associated with this tuner */
    ipStreamerCtx->parserBandPtr = NULL;
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->parserBandMutex);
    /* 1:1 mapping between IB & PB */
    parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[frontendIndex % NEXUS_NUM_PARSER_BANDS];

    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandMutex);
    if (parserBandPtr == NULL) {
        BDBG_ERR(("Failed to find a free parser band at %d", __LINE__));
        goto error;
    }

    NEXUS_Frontend_GetUserParameters(qamSrc->frontendHandle, &userParams);
    BKNI_AcquireMutex(parserBandPtr->lock);
    if (parserBandPtr->refCount == 0) {
        parserBandPtr->frequency = ipStreamerCfg->frequency;
        parserBandPtr->subChannel = ipStreamerCfg->subChannel;
        /* associate parser band to the input band associated with this tuner */
        /* Map this parser band to the demod's input band. */
        NEXUS_ParserBand_GetSettings(parserBandPtr->parserBand, &parserBandSettings);
        if (userParams.isMtsif) {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
            parserBandSettings.sourceTypeSettings.mtsif =  NEXUS_Frontend_GetConnector(qamSrc->frontendHandle); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
        }
        else {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
            parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
        }
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
#ifdef MPODXSIX
        parserBandSettings.allPass = false;
#endif
        rc = NEXUS_ParserBand_SetSettings(parserBandPtr->parserBand, &parserBandSettings);
        if (rc) {
            BDBG_ERR(("Failed to set the Nexus Parser band settings for band # %d", (int)parserBandPtr->parserBand));
            BKNI_ReleaseMutex(parserBandPtr->lock);
            goto error;
        }
    }
    else {
        BDBG_MSG(("Nexus Parser band band # %d is already setup (ref cnt %d)", (int)parserBandPtr->parserBand, parserBandPtr->refCount));
    }
    parserBandPtr->refCount++;
    BKNI_ReleaseMutex(parserBandPtr->lock);
    if (ipStreamerCfg->transcodeEnabled) {
        if (parserBandPtr->transcodeEnabled && parserBandPtr->transcoderDst->refCount) {
            if (parserBandPtr->subChannel != ipStreamerCfg->subChannel ||
                parserBandPtr->transcode.outVideoCodec != ipStreamerCfg->transcode.outVideoCodec ||
                parserBandPtr->transcode.outHeight != ipStreamerCfg->transcode.outHeight) {
                BDBG_WRN(("%s: ERROR: Limiting to one transcoding sesion: Transcoder pipe (# of users %d) is already opened/enabled for this Live Channel", __FUNCTION__, parserBandPtr->transcoderDst->refCount));
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                goto error;
            }
            BDBG_WRN(("%s: Transcoder pipe (# of users %d) is already opened/enabled for this Live Channel", __FUNCTION__, parserBandPtr->transcoderDst->refCount));
            parserBandPtr->transcoderDst->refCount++;
            ipStreamerCtx->transcoderDst = parserBandPtr->transcoderDst;

        }
        else {
            /* coverity[stack_use_local_overflow] */
            /* coverity[stack_use_overflow] */
            if ((parserBandPtr->transcoderDst = openNexusTranscoderPipe(ipStreamerCfg, ipStreamerCtx)) == NULL) {
                BDBG_ERR(("%s: Failed to open the transcoder pipe", __FUNCTION__));
                goto error;
            }
            parserBandPtr->transcodeEnabled = true;
            qamSrc->transcodeEnabled = true;
            parserBandPtr->transcode = ipStreamerCfg->transcode;
            parserBandPtr->subChannel = ipStreamerCfg->subChannel;
        }
        ipStreamerCtx->parserBandPtr = parserBandPtr;
    }
    else {
        /* no transcode case */
        ipStreamerCtx->parserBandPtr = parserBandPtr;
    }
    BDBG_MSG(("CTX %p: QAM Frontend Src %p (ref count %d), Input Band %d & Parser Band %d (ref count %d) are opened",
                (void *)ipStreamerCtx, (void *)qamSrc, qamSrc->refCount, userParams.param1, (int)ipStreamerCtx->parserBandPtr->parserBand, ipStreamerCtx->parserBandPtr->refCount));
    if (ipStreamerCfg->transcodeEnabled) {
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
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
    if (!qamSrc) return -1;
    BKNI_AcquireMutex(qamSrc->lock);
    qamSrc->refCount--;
    qamSrc->psiAcquiring = false;
    BKNI_ReleaseMutex(qamSrc->lock);
    return -1;
}

#else /* !NEXUS_HAS_VIDEO_ENCODER */

/* keep this function for backward compatibility w/ 7420 based SMS/VMS for one more release */
/* select a qam src based on the requested QAM frequency */
int
openNexusQamSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    unsigned i;
    bool matchFound = false;
    QamSrc *qamSrc = NULL;
    QamSrc *qamSrcList = NULL;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    ParserBand *parserBandPtr = NULL;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;

#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
#endif

    qamSrcList = &ipStreamerCtx->globalCtx->qamSrcList[0];
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->qamSrcMutex);
    for (i = 0; i < NEXUS_MAX_FRONTENDS; i++) {
        if (!qamSrcList[i].frontendHandle) {
            /* reached 1st NULL frontendHandle, so we dont have a free tuner to use */
            break;
        }
        BDBG_MSG(("frequency %d, src entry freq %d", ipStreamerCfg->frequency, qamSrcList[i].frequency));
#ifndef STREAMER_CABLECARD_SUPPORT
        if (qamSrcList[i].frequency == ipStreamerCfg->frequency /*&& ipStreamerCfg->transcodeEnabled*/) {
            /* a tuner is either currently or was already receiving streams for this frequency And */
            /* transcode is enabled for this stream */
            qamSrc = &qamSrcList[i];
            /* set a flag to indicate skipping PSI acquisition as it would have already been acquired during first tuning to this frequency */
            if (qamSrc->numProgramsFound)
                ipStreamerCtx->skipPsiAcquisition = true;
            matchFound = true;
            break;
        }
#endif
        if (!qamSrcList[i].refCount && !qamSrc)
            /* save a pointer if we haven't yet found a free unused tuner */
            qamSrc = &qamSrcList[i];
    }
    if (qamSrc) {
        BKNI_AcquireMutex(qamSrc->lock);
        qamSrc->refCount++;
        qamSrc->frequency = ipStreamerCfg->frequency;
        if (!matchFound) {
            /* force PSI acquisition as this src is being used to receive a new channel */
            ipStreamerCtx->skipPsiAcquisition = false;
            qamSrc->numProgramsFound = 0;
        }
        BKNI_ReleaseMutex(qamSrc->lock);
        BDBG_MSG(("Found a free Qam Frontend %p, use it for %dMhz frequency, refCount %d", qamSrc, ipStreamerCfg->frequency, qamSrc->refCount));
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->qamSrcMutex);
    if (!qamSrc) {
        BDBG_MSG(("No Free tuner available for this %d frequency all turners (# %d) busy", ipStreamerCfg->frequency, i));
        goto error;
    }
    ipStreamerCtx->qamSrc = qamSrc;

    /* setup a callback to know when tuner is locked */
    NEXUS_Frontend_GetDefaultQamSettings(&ipStreamerCtx->qamSettings);
    ipStreamerCtx->qamSettings.frequency = ipStreamerCfg->frequency;
    ipStreamerCtx->qamSettings.mode = ipStreamerCfg->qamMode;
    /* TODO: check if client needs to be specify this parameter as well */
    ipStreamerCtx->qamSettings.annex = ipStreamerCfg->qamAnnex;
    ipStreamerCtx->qamSettings.symbolRate = ipStreamerCfg->symbolRate;
    ipStreamerCtx->qamSettings.lockCallback.callback = qamLockCallback;
    ipStreamerCtx->qamSettings.lockCallback.context = qamSrc;
    ipStreamerCtx->qamSettings.lockCallback.param = ipStreamerCfg->srcType;

    /* associate parser band to the input band associated with this tuner */
    ipStreamerCtx->parserBandPtr = NULL;
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->parserBandMutex);
    for (i = 0; i < IP_STREAMER_NUM_PARSER_BANDS; i++) {
        /* pick an unused parser band */
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->parserBandList[i].lock);
#ifndef STREAMER_CABLECARD_SUPPORT
        if (ipStreamerCtx->globalCtx->parserBandList[i].frequency == ipStreamerCfg->frequency
#ifndef NEXUS_HAS_VIDEO_ENCODER
        && ipStreamerCtx->globalCtx->parserBandList[i].subChannel == ipStreamerCfg->subChannel
#endif
#ifdef NEXUS_HAS_7043_TRANSCODER
            &&
            !ipStreamerCtx->globalCtx->parserBandList[i].transcodeEnabled
#endif
            ) {
            /* this parser band is being used for same frequency and sub-channel (pids) */
            /* & not being used for transcoding (note: this check doesn't apply to new transcoder cause transcoded data is not available via IB5/PB channel, but via playback channel) */
            /* complete match, so reuse this parser band */
            parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i];
            BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandList[i].lock);
            break;
        }
#endif
        if (!parserBandPtr && ipStreamerCtx->globalCtx->parserBandList[i].refCount == 0) {
            /* found 1st unused entry */
            parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i];
        }
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandList[i].lock);
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandMutex);
    if (parserBandPtr == NULL) {
        BDBG_ERR(("Failed to find a free parser band at %d", __LINE__));
        goto error;
    }

    NEXUS_Frontend_GetUserParameters(qamSrc->frontendHandle, &userParams);
    BKNI_AcquireMutex(parserBandPtr->lock);
    if (parserBandPtr->refCount == 0) {
        parserBandPtr->frequency = ipStreamerCfg->frequency;
        parserBandPtr->subChannel = ipStreamerCfg->subChannel;
        /* associate parser band to the input band associated with this tuner */
        /* Map this parser band to the demod's input band. */
        NEXUS_ParserBand_GetSettings(parserBandPtr->parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
#ifdef MPODXSIX
        parserBandSettings.allPass = false;
#endif
        rc = NEXUS_ParserBand_SetSettings(parserBandPtr->parserBand, &parserBandSettings);
        if (rc) {
            BDBG_ERR(("Failed to set the Nexus Parser band settings for band # %d", parserBandPtr->parserBand));
            BKNI_ReleaseMutex(parserBandPtr->lock);
            goto error;
        }
    }
    else {
        BDBG_MSG(("Nexus Parser band band # %d is already setup (ref cnt %d)", parserBandPtr->parserBand, parserBandPtr->refCount));
    }
    parserBandPtr->refCount++;
    BKNI_ReleaseMutex(parserBandPtr->lock);
#ifdef NEXUS_HAS_7043_TRANSCODER
    if (ipStreamerCfg->transcodeEnabled) {
        ipStreamerCtx->remuxParserBandPtr = parserBandPtr;
        /* allocate another Parser band and tie it to IB5 as 7043 transcoder output is feed into the IB5 on SMS/VMS platforms */
        BDBG_MSG(("%s: initialize another input/parser band for transcoding", __FUNCTION__));
        parserBandPtr = NULL;
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->parserBandMutex);
        for (i = 0; i < IP_STREAMER_NUM_PARSER_BANDS; i++) {
            /* pick an unused parser band */
            BKNI_AcquireMutex(ipStreamerCtx->globalCtx->parserBandList[i].lock);
            if (ipStreamerCtx->globalCtx->parserBandList[i].frequency == ipStreamerCfg->frequency &&
                ipStreamerCtx->globalCtx->parserBandList[i].subChannel == ipStreamerCfg->subChannel &&
                ipStreamerCtx->globalCtx->parserBandList[i].transcodeEnabled) {
                /* this parser band is being used for same frequency and sub-channel (pids) & also being used for transcoding */
                /* xcd TODO: make sure if xcoding params also match. if so, reuse this parser band */
                parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i];
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandList[i].lock);
                /* reuse the remux dst for this duplicate session */
                /* we increment the remux use count here to atomically acquire both parserband & remux */
                ipStreamerCtx->remuxDst = &ipStreamerCtx->globalCtx->remuxDstList[0];
                if (ipStreamerCtx->remuxDst->inUse == false)
                    BDBG_ASSERT(NULL);
                ipStreamerCtx->remuxDst->refCount++;
                break;
            }
            if (!parserBandPtr && ipStreamerCtx->globalCtx->parserBandList[i].refCount == 0) {
                /* found 1st unused entry */
                parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i];
            }
            BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandList[i].lock);
        }
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandMutex);
        if (parserBandPtr == NULL) {
            BDBG_ERR(("%s: Failed to find a free parser band at %d", __FUNCTION__, __LINE__));
            /* free up the parser band */
            BKNI_AcquireMutex(ipStreamerCtx->remuxParserBandPtr->lock);
            ipStreamerCtx->remuxParserBandPtr->refCount--;
            BKNI_ReleaseMutex(ipStreamerCtx->remuxParserBandPtr->lock);
            goto error;
        }

        BKNI_AcquireMutex(parserBandPtr->lock);
        if (parserBandPtr->refCount == 0) {
            parserBandPtr->frequency = ipStreamerCfg->frequency;
            parserBandPtr->subChannel = ipStreamerCfg->subChannel;
            /* Map this parser band to the demod's input band. */
            NEXUS_ParserBand_GetSettings(parserBandPtr->parserBand, &parserBandSettings);
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
            /* associate parser band to the input band associated with this tuner: Note: IB5 is for SMS/VMS platforms */
            parserBandSettings.sourceTypeSettings.inputBand = NEXUS_InputBand_e5;
            parserBandSettings.transportType = NEXUS_TransportType_eTs;
            rc = NEXUS_ParserBand_SetSettings(parserBandPtr->parserBand, &parserBandSettings);
            if (rc) {
                BDBG_ERR(("Failed to set the Nexus Parser band settings for band # %d", parserBandPtr->parserBand));
                BKNI_ReleaseMutex(parserBandPtr->lock);
                /* free up the parser band */
                BKNI_AcquireMutex(ipStreamerCtx->remuxParserBandPtr->lock);
                ipStreamerCtx->remuxParserBandPtr->refCount--;
                BKNI_ReleaseMutex(ipStreamerCtx->remuxParserBandPtr->lock);
                goto error;
            }
            parserBandPtr->transcodeEnabled = true;
            parserBandPtr->transcode = ipStreamerCfg->transcode;
        }
        parserBandPtr->refCount++;
        BKNI_ReleaseMutex(parserBandPtr->lock);
        ipStreamerCtx->parserBandPtr = parserBandPtr;
    }
    else
#endif
#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        if (parserBandPtr->transcodeEnabled && parserBandPtr->transcoderDst->refCount) {
            if (parserBandPtr->subChannel != ipStreamerCfg->subChannel ||
                parserBandPtr->transcode.outVideoCodec != ipStreamerCfg->transcode.outVideoCodec ||
                parserBandPtr->transcode.outHeight != ipStreamerCfg->transcode.outHeight) {
                BDBG_WRN(("%s: ERROR: Limiting to one transcoding sesion: Transcoder pipe (# of users %d) is already opened/enabled for this Live Channel", __FUNCTION__, parserBandPtr->transcoderDst->refCount));
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                goto error;
            }
            BDBG_WRN(("%s: Transcoder pipe (# of users %d) is already opened/enabled for this Live Channel", __FUNCTION__, parserBandPtr->transcoderDst->refCount));
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
            parserBandPtr->transcode = ipStreamerCfg->transcode;
            parserBandPtr->subChannel = ipStreamerCfg->subChannel;
        }
        ipStreamerCtx->parserBandPtr = parserBandPtr;
    }
    else
#endif
    {
        /* no transcode case */
        ipStreamerCtx->parserBandPtr = parserBandPtr;
#ifdef NEXUS_HAS_7043_TRANSCODER
        ipStreamerCtx->remuxParserBandPtr = NULL;
#endif
    }
    BDBG_MSG(("CTX %p: QAM Frontend Src %p (ref count %d), Input Band %d & Parser Band %d (ref count %d) are opened",
                ipStreamerCtx, qamSrc, qamSrc->refCount, userParams.param1, ipStreamerCtx->parserBandPtr->parserBand, ipStreamerCtx->parserBandPtr->refCount));
#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
#endif
    return 0;

error:
    if (parserBandPtr) {
        BKNI_AcquireMutex(parserBandPtr->lock);
        parserBandPtr->refCount--;
        BKNI_ReleaseMutex(parserBandPtr->lock);
    }
#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
#endif
    if (!qamSrc) return -1;
    BKNI_AcquireMutex(qamSrc->lock);
    qamSrc->refCount--;
    BKNI_ReleaseMutex(qamSrc->lock);
    return -1;
}
#endif /* !NEXUS_HAS_VIDEO_ENCODER */

void
closeNexusQamSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    QamSrc *qamSrc = ipStreamerCtx->qamSrc;

#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCtx->transcoderDst) {
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        closeNexusTranscoderPipe(ipStreamerCtx, ipStreamerCtx->transcoderDst);
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        if (ipStreamerCtx->transcoderDst->refCount == 0) {
            /* clear the transcoding enabled flag in the parse band */
            ipStreamerCtx->parserBandPtr->transcodeEnabled = false;
            qamSrc->transcodeEnabled = false;
        }
    }
#endif
    /* free up the parser band */
    if (ipStreamerCtx->parserBandPtr) {
        BKNI_AcquireMutex(ipStreamerCtx->parserBandPtr->lock);
        ipStreamerCtx->parserBandPtr->refCount--;
        BKNI_ReleaseMutex(ipStreamerCtx->parserBandPtr->lock);
        BDBG_MSG(("CTX %p: Closed a parser band src %p, refCount %d", (void *)ipStreamerCtx, (void *)ipStreamerCtx->parserBandPtr, ipStreamerCtx->parserBandPtr->refCount));
    }

    if (!qamSrc || !qamSrc->refCount)
        return;

    ipStreamerCtx->qamSrc = NULL;
    BKNI_AcquireMutex(qamSrc->lock);
    qamSrc->refCount--;
    if (qamSrc->psiAcquiredEvent)
        BKNI_ResetEvent(qamSrc->psiAcquiredEvent);
    BKNI_ReleaseMutex(qamSrc->lock);
    BDBG_MSG(("CTX %p: Closed a qam src %p, refCount %d", (void *)ipStreamerCtx, (void *)qamSrc, qamSrc->refCount));
}

/* Obtain PSI info */
int
setupAndAcquirePsiInfoQamSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx,
    B_PlaybackIpPsiInfo *psiOut
    )
{
    int i;
    QamSrc *qamSrc;
    psiCollectionDataType  collectionData;

    memset(&collectionData, 0, sizeof(psiCollectionDataType));
    memset(psiOut, 0, sizeof(B_PlaybackIpPsiInfo));
    collectionData.srcType = ipStreamerCfg->srcType;
    collectionData.live = true;
    collectionData.parserBand = ipStreamerCtx->parserBandPtr->parserBand;
    collectionData.qamSettings = &ipStreamerCtx->qamSettings;
    collectionData.frontend = ipStreamerCtx->qamSrc->frontendHandle;
    collectionData.signalLockedEvent = ipStreamerCtx->qamSrc->signalLockedEvent;
    qamSrc = ipStreamerCtx->qamSrc;

    if (qamSrc->numProgramsFound == 0) {
        /* PSI hasn't yet been acquired */
        BDBG_MSG(("CTX %p: Acquire Psi Info...", (void *)ipStreamerCtx));

        /* get the PSI, this can take several seconds ... */
        acquirePsiInfo(&collectionData, &qamSrc->psi[0], &qamSrc->numProgramsFound);

        /* tell any other waiting thread that we are done acquiring PSI */
        BKNI_AcquireMutex(qamSrc->lock);
        qamSrc->psiAcquiring = false;
        BKNI_SetEvent(qamSrc->psiAcquiredEvent);
        BKNI_ReleaseMutex(qamSrc->lock);
        if (qamSrc->numProgramsFound == 0) {
            BDBG_WRN(("%s: ERROR: Unable to Acquire PSI!!", __FUNCTION__));
            return -1;
        }
    }
    else {
        BDBG_MSG(("CTX %p: Psi Info is already acquired...", (void *)ipStreamerCtx));
    }

    if (ipStreamerCfg->subChannel > qamSrc->numProgramsFound) {
        BDBG_MSG(("Requested sub-channel # (%d) is not found in the total channels (%d) acquired, defaulting to 1st sub-channel", ipStreamerCfg->subChannel, qamSrc->numProgramsFound));
        i = 0;
    }
    else {
        BDBG_MSG(("CTX %p: Requested sub-channel # (%d) is found in the total channels (%d) ", (void *)ipStreamerCtx, ipStreamerCfg->subChannel, qamSrc->numProgramsFound));
        i = ipStreamerCfg->subChannel - 1; /* sub-channels start from 1, where as psi table starts from 0 */
        if (i < 0) i = 0;
    }
    memcpy(psiOut, &qamSrc->psi[i], sizeof(B_PlaybackIpPsiInfo));
    BKNI_ResetEvent(qamSrc->signalLockedEvent);
    return 0;
}

int
startNexusQamSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    QamSrc *qamSrc = ipStreamerCtx->qamSrc;
    int rc = -1;

    BDBG_MSG(("%s: CTX %p: QAM Src %p, refCount %d", __FUNCTION__, (void *)ipStreamerCtx, (void *)qamSrc, qamSrc->refCount));

    BKNI_AcquireMutex(qamSrc->lock);
    if (qamSrc->started) {
        BDBG_MSG(("%s: CTX %p: QAM Src %p is already started, refCount %d", __FUNCTION__, (void *)ipStreamerCtx, (void *)qamSrc, qamSrc->refCount));
        rc = 0;
        goto out;
    }

    /* start frontend */
    BKNI_ResetEvent(qamSrc->signalLockedEvent);
    NEXUS_StartCallbacks(qamSrc->frontendHandle);
    if (NEXUS_Frontend_TuneQam(qamSrc->frontendHandle, &ipStreamerCtx->qamSettings) != NEXUS_SUCCESS) {
        BDBG_WRN(("CTX %p: QAM Src %p Failed to Tune QAM Frontend", (void *)ipStreamerCtx, (void *)qamSrc));
        rc = -1;
        goto out;
    }
#define QAM_LOCK_TIMEOUT 5000
    BDBG_MSG(("%s: waiting to lock frontend %p for %d msec", __FUNCTION__, (void *)qamSrc->frontendHandle, QAM_LOCK_TIMEOUT));
    if (BKNI_WaitForEvent(qamSrc->signalLockedEvent, QAM_LOCK_TIMEOUT)) {
        BDBG_WRN(("%s: CTX %p: failed to lock frontend %p...", __FUNCTION__, (void *)ipStreamerCtx, (void *)qamSrc->frontendHandle));
        rc = -1;
    }
    else {
        BDBG_MSG(("%s: frontend %p is locked", __FUNCTION__, (void *)qamSrc->frontendHandle));
        qamSrc->started = true;
        BDBG_MSG(("%s: CTX %p: QAM Src %p is started ...", __FUNCTION__, (void *)ipStreamerCtx, (void *)qamSrc));
        rc = 0;
    }
out:
    BKNI_ReleaseMutex(qamSrc->lock);
    return rc;
}

void
stopNexusQamSrc(
        IpStreamerCtx *ipStreamerCtx
    )
{
    QamSrc *qamSrc = ipStreamerCtx->qamSrc;
    int refCount;

    BKNI_AcquireMutex(qamSrc->lock);
    refCount = qamSrc->refCount;

    if (refCount == 1) {
        NEXUS_StopCallbacks(qamSrc->frontendHandle);
        NEXUS_Frontend_Untune(qamSrc->frontendHandle);
        qamSrc->started = false;
        BDBG_MSG(("CTX %p: QAM Frontend Src is stopped ...", (void *)ipStreamerCtx));
    }
    else {
        BDBG_MSG(("CTX %p: QAM Frontend Src %p is not stopped, ref count %d ...", (void *)ipStreamerCtx, (void *)qamSrc, refCount));
    }
    BKNI_ReleaseMutex(qamSrc->lock);
}
#else /* !NEXUS_HAS_FRONTEND */
/* stub functions */
int initNexusQamSrcList(IpStreamerGlobalCtx *ipStreamerGlobalCtx)
{
    BSTD_UNUSED(ipStreamerGlobalCtx);

    BDBG_ERR(("%s: Streaming from QAM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    /* note: we let the init call succeed, but we will fail the open call */
    /* this way app wont fail during startup, but will fail a client request to open/start a recording */
    return 0;
}
void unInitNexusQamSrcList(IpStreamerGlobalCtx *ipStreamerGlobalCtx)
{
    BSTD_UNUSED(ipStreamerGlobalCtx);

    BDBG_ERR(("%s: Streaming from QAM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
}
int openNexusQamSrc(IpStreamerConfig *ipStreamerCfg, IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCfg);
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from QAM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    return -1;
}
void closeNexusQamSrc(IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from QAM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
}
int setupAndAcquirePsiInfoQamSrc(IpStreamerConfig *ipStreamerCfg, IpStreamerCtx *ipStreamerCtx, B_PlaybackIpPsiInfo *psiOut)
{
    BSTD_UNUSED(psiOut);
    BSTD_UNUSED(ipStreamerCfg);
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from QAM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    return -1;
}
int startNexusQamSrc(IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from QAM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    return -1;
}
int stopNexusQamSrc(IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from QAM Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", __FUNCTION__));
    return -1;
}
#endif /* NEXUS_HAS_FRONTEND */
