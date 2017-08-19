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
/* build a list of all available sat sources */
/* Note: for VMS configuration, local tuners (aka sources) == # of decoders are left for local decoding app */
int
initNexusSatSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    unsigned i, j;
    int satTunerCount = 0;
    SatSrc *satSrc;
    NEXUS_FrontendHandle frontendHandle=NULL;
    NEXUS_PlatformConfiguration platformConfig;

    /* global reset */
    memset(ipStreamerGlobalCtx->satSrcList, 0, sizeof(ipStreamerGlobalCtx->satSrcList));

    if (BKNI_CreateMutex(&ipStreamerGlobalCtx->satSrcMutex) != 0) {
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
            settings.capabilities.satellite = true;
            frontendHandle = NEXUS_Frontend_Acquire(&settings);
        } else {
            frontendHandle = platformConfig.frontend[i];
        }

        if (!frontendHandle) {
            BDBG_MSG(("%s: NULL Frontend (# %d)", BSTD_FUNCTION, i));
            continue;
        }
        NEXUS_Frontend_GetCapabilities(frontendHandle, &capabilities);
        if (capabilities.satellite) {
            satTunerCount++;
            /* Found a sat source */
            satSrc = &ipStreamerGlobalCtx->satSrcList[j];
            memset(satSrc, 0, sizeof(SatSrc));
            satSrc->frontendHandle = frontendHandle;
            if (BKNI_CreateEvent(&satSrc->psiAcquiredEvent)) {
                BDBG_ERR(("Failed to psiAcquired event at %d", __LINE__));
                return -1;
            }
            if (BKNI_CreateEvent(&satSrc->signalLockedEvent)) {
                BDBG_ERR(("Failed to signalLocked event at %d", __LINE__));
                return -1;
            }
            if (BKNI_CreateMutex(&satSrc->lock) != 0) {
                BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
                return -1;
            }
            BDBG_MSG(("%s: satSrc %p, lock %p", BSTD_FUNCTION, (void *)satSrc, (void *)satSrc->lock));
            /* successfully setup a sat frontend src */
            j++;
        }
        else {
            /* skip other tuner types */
            BDBG_MSG(("!!!!!!!!!!!!!!!!!!!!!! Satellite Capabilities not set for this frontend %p", (void *)frontendHandle));
            continue;
        }
    }

    if (!j) {
        BDBG_MSG(("NO sat Front End founds, check Nexus platform configuration"));
        return 0;
    }
    BDBG_MSG(("################################## Initialized %d sat Frontend Sources", j));
    return 0;
}

void
unInitNexusSatSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    SatSrc *satSrc;
    for (i = 0; i < NEXUS_MAX_FRONTENDS; i++) {
        if (!ipStreamerGlobalCtx->satSrcList[i].frontendHandle) {
            /* reached 1st NULL frontendHandle, so we dont have a free tuner to use */
            break;
        }
        satSrc = &ipStreamerGlobalCtx->satSrcList[i];
        if (satSrc->psiAcquiredEvent)
            BKNI_DestroyEvent(satSrc->psiAcquiredEvent);
        if (satSrc->signalLockedEvent)
            BKNI_DestroyEvent(satSrc->signalLockedEvent);
        if (satSrc->lock)
            BKNI_DestroyMutex(satSrc->lock);
    }
    /* global reset */
    memset(ipStreamerGlobalCtx->satSrcList, 0, sizeof(ipStreamerGlobalCtx->satSrcList));

    if (ipStreamerGlobalCtx->satSrcMutex)
        BKNI_DestroyMutex(ipStreamerGlobalCtx->satSrcMutex);
    BDBG_MSG(("sat Frontend Sources are uninitialized"));
}

static void
satLockCallback(void *context, int param)
{
    SatSrc *satSrc = (SatSrc *)context;
    NEXUS_FrontendHandle frontendHandle;
    NEXUS_FrontendSatelliteStatus satStatus;
    NEXUS_FrontendDiseqcStatus disqecStatus;

    BSTD_UNUSED(param);
    frontendHandle = satSrc->frontendHandle;
    if (NEXUS_Frontend_GetSatelliteStatus(frontendHandle, &satStatus) != NEXUS_SUCCESS) {
        BDBG_WRN(("%s: NEXUS_Frontend_GetSatelliteStatus() failed", BSTD_FUNCTION ));
        return;
    }

    BDBG_MSG(("sat Lock callback, frontend %p - demod lock status %s, symbolRate %d",
                (void *)frontendHandle, (satStatus.demodLocked==0)?"UNLOCKED":"LOCKED", satStatus.symbolRate));
    if (NEXUS_Frontend_GetDiseqcStatus(frontendHandle, &disqecStatus) != NEXUS_SUCCESS) {
        BDBG_WRN(("%s: NEXUS_Frontend_GetDiseqcStatus() failed", BSTD_FUNCTION));
        return;
    }
    BDBG_MSG(("diseqc tone = %d, voltage = %u", disqecStatus.toneEnabled, disqecStatus.voltage));

    if (satStatus.demodLocked)
      BKNI_SetEvent(satSrc->signalLockedEvent);
}

#ifdef NEXUS_HAS_VIDEO_ENCODER
/* select a sat src based on the requested Sat frequency: w/ xcode */
int
openNexusSatSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    unsigned i;
    SatSrc *satSrc = NULL;
    SatSrc *satSrcList = NULL;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    ParserBand *parserBandPtr = NULL;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    B_PlaybackIpPsiInfo *cachedPsi = NULL;
    int numProgramsFound = 0;
    NEXUS_FrontendDiseqcSettings diseqcSettings;
    int frontendIndex = 0;
    unsigned maxNexusFrontends = NEXUS_MAX_FRONTENDS;
    unsigned firstNexusFrontend = 0;

    BDBG_MSG(("%s - StreamerCfg (%p); StreamerCtx (%p)", BSTD_FUNCTION, (void *)ipStreamerCfg, (void *)ipStreamerCtx ));
    BDBG_MSG(("%s - StreamerCtx->globalCtx (%p)", BSTD_FUNCTION, (void *)ipStreamerCtx->globalCtx ));
    if (ipStreamerCfg==NULL)
    {
        BDBG_ERR(("%s: ipStreamerCfg cannot be NULL", BSTD_FUNCTION));
        return -1;
    }

    if (ipStreamerCtx==NULL)
    {
        BDBG_ERR(("%s: ipStreamerCtx cannot be NULL", BSTD_FUNCTION));
        return -1;
    }

    if (ipStreamerCfg->transcodeEnabled) {
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }

    satSrcList = &ipStreamerCtx->globalCtx->satSrcList[0];
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->satSrcMutex);
    /* reuse frontend & pb only if transcode session is enabled on the current & new session */
    /* copy psi info if freq is same */
#ifdef NEXUS_USE_7425_SATIPSW
    if (ipStreamerCfg->verticalPolarization) {
        /* vertical polarization is supported by 1st 4828 tuner */
        firstNexusFrontend = 0;
        maxNexusFrontends = NEXUS_MAX_FRONTENDS/2;
    }
    else {
        /* all other polarizations are supported by 2nd 4828 tuner */
        firstNexusFrontend = NEXUS_MAX_FRONTENDS/2;
        maxNexusFrontends = NEXUS_MAX_FRONTENDS;
    }
#endif
    for (i = firstNexusFrontend; i < maxNexusFrontends; i++) {
        if (!satSrcList[i].frontendHandle) {
            /* reached 1st NULL frontendHandle, so we dont have a free tuner to use */
            break;
        }
        BDBG_MSG(("%s: satSrc[%u] - CTX %p; new frequency %u, satSrc freq %u, skipPsiAcquisition %d; liveStreaming (%p)",
                  BSTD_FUNCTION, i, (void *)ipStreamerCtx, ipStreamerCfg->frequency, satSrcList[i].frequency, ipStreamerCfg->skipPsiAcquisition,
                   (void *)ipStreamerCtx->cfg.liveStreamingCtx ));
        if (satSrcList[i].frequency == ipStreamerCfg->frequency) {
            /* a tuner is either currently or was already receiving streams for this frequency */
            if (!ipStreamerCfg->skipPsiAcquisition && satSrcList[i].psiAcquiring) {
                /* check if some other thread is in the process of doing psi discovery on this sat channel */
                /* if so, let that thread finish getting PSI and then proceed */
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->satSrcMutex);
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                BDBG_MSG(("CTX %p: Another thread is acquiring the PSI info, waiting for its completion...", (void *)ipStreamerCtx));
                if (BKNI_WaitForEvent(satSrcList[i].psiAcquiredEvent, 30000)) {
                    BDBG_ERR(("%s: timeout while waiting for PSI acquisition by another thread", BSTD_FUNCTION));
                    return -1;
                }
                if (ipStreamerCfg->transcodeEnabled && satSrcList[i].transcodeEnabled) {
                    /* in xcode case, sleep to allow other thread finish setting up */
                    BDBG_MSG(("%s: delay runnig this thread ", BSTD_FUNCTION));
                    BKNI_Sleep(200);
                }
                BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                BKNI_AcquireMutex(ipStreamerCtx->globalCtx->satSrcMutex);
            }
            if (satSrcList[i].numProgramsFound) {
                /* PSI info is available, set flag to skip it for this session and copy it from this */
                cachedPsi = satSrcList[i].psi;
                numProgramsFound = satSrcList[i].numProgramsFound;
                BDBG_MSG(("%s: freq matched to previously tuned channels, skip PSI acquisition and reuse it from cached copy", BSTD_FUNCTION));
            }

            /* if transcoding or joining a live conference stream */
            if ( (ipStreamerCfg->transcodeEnabled && satSrcList[i].transcodeEnabled) || (ipStreamerCtx->cfg.liveStreamingCtx != NULL) ) {
                parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i% NEXUS_NUM_PARSER_BANDS];

                if (parserBandPtr->subChannel == ipStreamerCfg->subChannel &&
                parserBandPtr->transcode.outVideoCodec == ipStreamerCfg->transcode.outVideoCodec &&
                parserBandPtr->transcode.outAudioCodec == ipStreamerCfg->transcode.outAudioCodec &&
                parserBandPtr->transcode.outWidth == ipStreamerCfg->transcode.outWidth &&
                parserBandPtr->transcode.outHeight == ipStreamerCfg->transcode.outHeight
                ) {
                    /* we only reuse the frontend if this frontend already is being used for transcoding session and */
                    /* new session is also a transcoding session on the same frequency */
                    satSrc = &satSrcList[i];
                    frontendIndex = i; /* we use the same index for frontend, input band & parser band as IB & PB needs to have 1:1 mapping */
                    BDBG_MSG(("%s: reusing frontend index %d for xcode session", BSTD_FUNCTION, i));
                    break;
                }
            }
            else{
                parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i % NEXUS_NUM_PARSER_BANDS];

                if (ipStreamerCfg->enableAllpass) {
                    /*we resue the fron end if allpass is enabled.*/
                    satSrc = &satSrcList[i];
                    frontendIndex = i; /* we use the same index for frontend, input band & parser band as IB & PB needs to have 1:1 mapping */
                    BDBG_MSG(("%s: reusing frontend index %d for live channel decode session", BSTD_FUNCTION, i));
                    break;
                }

            }
        }
        if (!satSrcList[i].refCount && !satSrc) {
            /* save a pointer if we haven't yet found a free unused tuner */
            satSrc = &satSrcList[i];
            frontendIndex = i; /* we use the same index for frontend, input band & parser band as IB & PB needs to have 1:1 mapping */
        }
    }
    if (satSrc) {
        BKNI_AcquireMutex(satSrc->lock);
        satSrc->refCount++;
        satSrc->frequency = ipStreamerCfg->frequency;
        if (!ipStreamerCfg->skipPsiAcquisition) {
            if (!numProgramsFound) {
                /* force PSI acquisition as this src is being used to receive a new channel */
                satSrc->numProgramsFound = 0;
                /* set flag to indicate this thread is getting PSI */
                satSrc->psiAcquiring = true;
            }
            else {
                memcpy(satSrc->psi, cachedPsi, sizeof(B_PlaybackIpPsiInfo)*MAX_PROGRAMS_PER_FREQUENCY);
                satSrc->numProgramsFound = numProgramsFound;
                BDBG_MSG(("%s: PSI data reused from cached copy", BSTD_FUNCTION));
            }
        }
        BKNI_ReleaseMutex(satSrc->lock);
        BDBG_MSG(("CTX %p: Found a free sat Frontend %p, index %d, sat src %p, use it for %uMhz frequency, refCount %d", (void *)ipStreamerCtx, (void *)satSrc->frontendHandle, frontendIndex, (void *)satSrc, ipStreamerCfg->frequency, satSrc->refCount));
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->satSrcMutex);
    if (!satSrc) {
        BDBG_WRN(("%s: No Free tuner available for this %u frequency all turners (# %u) busy", BSTD_FUNCTION, ipStreamerCfg->frequency, i));
        goto error;
    }
    ipStreamerCtx->satSrc = satSrc;

    /* setup a callback to know when tuner is locked */
    NEXUS_Frontend_GetDefaultSatelliteSettings(&ipStreamerCtx->satSettings);
    ipStreamerCtx->satSettings.frequency = ipStreamerCfg->frequency;
    ipStreamerCtx->satSettings.symbolRate = ipStreamerCfg->symbolRate;
    ipStreamerCtx->satSettings.mode = ipStreamerCfg->satMode;
    ipStreamerCtx->satSettings.lockCallback.callback = satLockCallback;
    ipStreamerCtx->satSettings.lockCallback.context = satSrc;
    ipStreamerCtx->satSettings.lockCallback.param = ipStreamerCfg->srcType;
    if (ipStreamerCtx->satSettings.mode == NEXUS_FrontendSatelliteMode_eQpskLdpc ||
        ipStreamerCtx->satSettings.mode == NEXUS_FrontendSatelliteMode_e8pskLdpc) {
        /* for DVBS2, update the pilotTone settings */
        if (ipStreamerCfg->pilotToneSpecified) {
            /* client has provided pilotTone settings of either on or off */
            ipStreamerCtx->satSettings.ldpcPilot = ipStreamerCfg->pilotTone;
            ipStreamerCtx->satSettings.ldpcPilotPll = ipStreamerCfg->pilotTone;
            ipStreamerCtx->satSettings.ldpcPilotScan = false;
        }
        else {
            bool eightpsk = ipStreamerCtx->satSettings.mode == NEXUS_FrontendSatelliteMode_e8pskLdpc? 1:0;
            ipStreamerCtx->satSettings.ldpcPilot = eightpsk;
            ipStreamerCtx->satSettings.ldpcPilotPll = eightpsk;
            ipStreamerCtx->satSettings.ldpcPilotScan = !eightpsk;
        }
        BDBG_MSG(("%s: ldpcPilot %d, ldpcPilotPll %d ldpcPilotScan %d", BSTD_FUNCTION, ipStreamerCtx->satSettings.ldpcPilot, ipStreamerCtx->satSettings.ldpcPilotPll, ipStreamerCtx->satSettings.ldpcPilotScan));
    }
    if (ipStreamerCfg->fec) {
        ipStreamerCtx->satSettings.codeRate.numerator = ipStreamerCfg->fec == 910 ? 9 : ipStreamerCfg->fec / 10;
        ipStreamerCtx->satSettings.codeRate.denominator = ipStreamerCfg->fec == 910 ? 10 : ipStreamerCfg->fec % 10;
    }

    BDBG_MSG(("%s: then NEXUS_HAS_VIDEO_ENCODER Sat Freq %u, symbolRate %u, mode %d; sock %d", BSTD_FUNCTION,
              ipStreamerCtx->satSettings.frequency, ipStreamerCtx->satSettings.symbolRate, (int)ipStreamerCtx->satSettings.mode,
              ipStreamerCtx->cfg.streamingFd ));

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

    BDBG_MSG(("%s: using parserBand %d; freq (%u); symbolRate (%u)" ,BSTD_FUNCTION ,
              frontendIndex  ,ipStreamerCtx->satSettings.frequency ,
              ipStreamerCtx->satSettings.symbolRate ));
    NEXUS_Frontend_GetUserParameters(satSrc->frontendHandle, &userParams);
    BKNI_AcquireMutex(parserBandPtr->lock);
    if (parserBandPtr->refCount == 0) {
        parserBandPtr->frequency = ipStreamerCfg->frequency;
        parserBandPtr->subChannel = ipStreamerCfg->subChannel;
        /* associate parser band to the input band associated with this tuner */
        /* Map this parser band to the demod's input band. */
        NEXUS_ParserBand_GetSettings(parserBandPtr->parserBand, &parserBandSettings);
        if (userParams.isMtsif) {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
            parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(satSrc->frontendHandle); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
        }
        else {
            parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
            parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
        }
        if (ipStreamerCfg->satMode == NEXUS_FrontendSatelliteMode_eDss)
            parserBandSettings.transportType = NEXUS_TransportType_eDssEs; /* assumes DSS SD */
        else
            parserBandSettings.transportType = NEXUS_TransportType_eTs;
        if (ipStreamerCfg->enableAllpass) {
            parserBandSettings.allPass=true;
            parserBandSettings.acceptNullPackets=false;
        }
        rc = NEXUS_ParserBand_SetSettings(parserBandPtr->parserBand, &parserBandSettings);
        if (rc) {
            BDBG_ERR(("Failed to set the Nexus Parser band settings for band # %d", (int)parserBandPtr->parserBand));
            BKNI_ReleaseMutex(parserBandPtr->lock);
            goto error;
        }
        NEXUS_Frontend_GetDiseqcSettings(satSrc->frontendHandle, &diseqcSettings);
        diseqcSettings.toneEnabled = ipStreamerCfg->toneEnabled;
        diseqcSettings.voltage = ipStreamerCfg->diseqcVoltage;
        if (NEXUS_Frontend_SetDiseqcSettings(satSrc->frontendHandle, &diseqcSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to set the Nexus Diseqc settings", BSTD_FUNCTION));
            BKNI_ReleaseMutex(parserBandPtr->lock);
            goto error;
        }
        BDBG_MSG(("%s: Updated Nexus Diseqc settings: toneEnabled %d, voltage %d", BSTD_FUNCTION, diseqcSettings.toneEnabled, diseqcSettings.voltage));
    }
    else {
        BDBG_MSG(("Nexus Parser band # %d is already setup (ref cnt %d)", (int)parserBandPtr->parserBand, parserBandPtr->refCount));
    }
    parserBandPtr->refCount++;
    BKNI_ReleaseMutex(parserBandPtr->lock);
    if (ipStreamerCfg->transcodeEnabled) {
        if (parserBandPtr->transcodeEnabled && parserBandPtr->transcoderDst->refCount) {
            if (parserBandPtr->subChannel != ipStreamerCfg->subChannel ||
                parserBandPtr->transcode.outVideoCodec != ipStreamerCfg->transcode.outVideoCodec ||
                parserBandPtr->transcode.outHeight != ipStreamerCfg->transcode.outHeight) {
                BDBG_ERR(("%s: ERROR: Limiting to one transcoding sesion: Transcoder pipe (# of users %d) is already opened/enabled for this Live Channel", BSTD_FUNCTION, parserBandPtr->transcoderDst->refCount));
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                goto error;
            }
            BDBG_MSG(("%s: Transcoder pipe (# of users %d) is already opened/enabled for this Live Channel", BSTD_FUNCTION, parserBandPtr->transcoderDst->refCount));
            parserBandPtr->transcoderDst->refCount++;
            ipStreamerCtx->transcoderDst = parserBandPtr->transcoderDst;

        }
        else {
            /* coverity[stack_use_local_overflow] */
            /* coverity[stack_use_overflow] */
            if ((parserBandPtr->transcoderDst = openNexusTranscoderPipe(ipStreamerCfg, ipStreamerCtx)) == NULL) {
                BDBG_ERR(("%s: Failed to open the transcoder pipe", BSTD_FUNCTION));
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                goto error;
            }
            parserBandPtr->transcodeEnabled = true;
            satSrc->transcodeEnabled = true;
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
    BDBG_MSG(("CTX %p: sat Frontend Src %p (ref count %d), Input Band %d & Parser Band %d (ref count %d) are opened",
                (void *)ipStreamerCtx, (void *)satSrc, satSrc->refCount, userParams.param1, (int)ipStreamerCtx->parserBandPtr->parserBand, ipStreamerCtx->parserBandPtr->refCount));

    /* add new logic for  new Adc for demods */
    if (ipStreamerCfg->isNewAdc) {
        NEXUS_FrontendSatelliteRuntimeSettings settings;
        NEXUS_Frontend_GetSatelliteRuntimeSettings(satSrc->frontendHandle, &settings);
        settings.selectedAdc = ipStreamerCfg->newAdc;
        NEXUS_Frontend_SetSatelliteRuntimeSettings(satSrc->frontendHandle, &settings);

        BDBG_MSG(("%s: New Adc for Demod is  %d", BSTD_FUNCTION, settings.selectedAdc));
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
    if (!satSrc) return -1;
    BKNI_AcquireMutex(satSrc->lock);
    satSrc->refCount--;
    satSrc->psiAcquiring = false;
    BKNI_ReleaseMutex(satSrc->lock);
    return -1;
}
#else /* !NEXUS_HAS_VIDEO_ENCODER */
/* select a sat src based on the requested Sat frequency: w/o xcode */
int
openNexusSatSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    unsigned i;
    SatSrc *satSrc = NULL;
    SatSrc *satSrcList = NULL;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    ParserBand *parserBandPtr = NULL;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBandSettings parserBandSettings;
    B_PlaybackIpPsiInfo *cachedPsi = NULL;
    int numProgramsFound = 0;
    NEXUS_FrontendDiseqcSettings diseqcSettings;
    int frontendIndex = 0;

    satSrcList = &ipStreamerCtx->globalCtx->satSrcList[0];
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->satSrcMutex);
    /* reuse frontend & pb only if transcode session is enabled on the current & new session */
    /* copy psi info if freq is same */
    for (i = 0; i < NEXUS_MAX_FRONTENDS; i++) {
        if (!satSrcList[i].frontendHandle) {
            /* reached 1st NULL frontendHandle, so we dont have a free tuner to use */
            break;
        }
        BDBG_MSG(("NEW: frequency %lu, src entry freq %d, skipPsiAcquisition %d", ipStreamerCfg->frequency, satSrcList[i].frequency, ipStreamerCfg->skipPsiAcquisition));
        if (satSrcList[i].frequency == ipStreamerCfg->frequency) {
            /* a tuner is either currently or was already receiving streams for this frequency */
            if (!ipStreamerCfg->skipPsiAcquisition && satSrcList[i].psiAcquiring) {
                /* check if some other thread is in the process of doing psi discovery on this sat channel */
                /* if so, let that thread finish getting PSI and then proceed */
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->satSrcMutex);
                BDBG_MSG(("CTX %p: Another thread is acquiring the PSI info, waiting for its completion...", ipStreamerCtx));
                if (BKNI_WaitForEvent(satSrcList[i].psiAcquiredEvent, 30000)) {
                    BDBG_ERR(("%s: timeout while waiting for PSI acquisition by another thread", BSTD_FUNCTION));
                    return -1;
                }
                BKNI_AcquireMutex(ipStreamerCtx->globalCtx->satSrcMutex);
            }
            if (satSrcList[i].numProgramsFound) {
                /* PSI info is available, set flag to skip it for this session and copy it from this */
                cachedPsi = satSrcList[i].psi;
                numProgramsFound = satSrcList[i].numProgramsFound;
                BDBG_MSG(("%s: freq matched to previously tuned channels, skip PSI acquisition and reuse it from cached copy", BSTD_FUNCTION));
            }
        }
        if (!satSrcList[i].refCount && !satSrc) {
            /* save a pointer if we haven't yet found a free unused tuner */
            satSrc = &satSrcList[i];
            frontendIndex = i; /* we use the same index for frontend, input band & parser band as IB & PB needs to have 1:1 mapping */
        }
    }
    if (satSrc) {
        BKNI_AcquireMutex(satSrc->lock);
        satSrc->refCount++;
        satSrc->frequency = ipStreamerCfg->frequency;
        if (!ipStreamerCfg->skipPsiAcquisition) {
        if (!numProgramsFound) {
            /* force PSI acquisition as this src is being used to receive a new channel */
            satSrc->numProgramsFound = 0;
            /* set flag to indicate this thread is getting PSI */
            satSrc->psiAcquiring = true;
        }
        else {
            memcpy(satSrc->psi, cachedPsi, sizeof(B_PlaybackIpPsiInfo)*MAX_PROGRAMS_PER_FREQUENCY);
            satSrc->numProgramsFound = numProgramsFound;
            BDBG_MSG(("%s: PSI data reused from cached copy", BSTD_FUNCTION));
            }
        }
        BKNI_ReleaseMutex(satSrc->lock);
        BDBG_MSG(("CTX %p: Found a free sat Frontend %p, index %d, sat src %p, use it for %luMhz frequency, refCount %d", ipStreamerCtx, satSrc->frontendHandle, frontendIndex, satSrc, ipStreamerCfg->frequency, satSrc->refCount));
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->satSrcMutex);
    if (!satSrc) {
        BDBG_ERR(("%s: No Free tuner available for this %lu frequency all turners (# %d) busy", BSTD_FUNCTION, ipStreamerCfg->frequency, i));
        goto error;
    }
    ipStreamerCtx->satSrc = satSrc;

    /* setup a callback to know when tuner is locked */
    NEXUS_Frontend_GetDefaultSatelliteSettings(&ipStreamerCtx->satSettings);
    ipStreamerCtx->satSettings.frequency = ipStreamerCfg->frequency;
    ipStreamerCtx->satSettings.symbolRate = ipStreamerCfg->symbolRate;
    ipStreamerCtx->satSettings.mode = ipStreamerCfg->satMode;
    ipStreamerCtx->satSettings.lockCallback.callback = satLockCallback;
    ipStreamerCtx->satSettings.lockCallback.context = satSrc;
    ipStreamerCtx->satSettings.lockCallback.param = ipStreamerCfg->srcType;
    if (ipStreamerCtx->satSettings.mode == NEXUS_FrontendSatelliteMode_eQpskLdpc ||
        ipStreamerCtx->satSettings.mode == NEXUS_FrontendSatelliteMode_e8pskLdpc) {
        /* for DVBS2, update the pilotTone settings */
        if (ipStreamerCfg->pilotToneSpecified) {
            /* client has provided pilotTone settings of either on or off */
            ipStreamerCtx->satSettings.ldpcPilot = ipStreamerCfg->pilotTone;
            ipStreamerCtx->satSettings.ldpcPilotPll = ipStreamerCfg->pilotTone;
            ipStreamerCtx->satSettings.ldpcPilotScan = false;
        }
        else {
            bool eightpsk = ipStreamerCtx->satSettings.mode == NEXUS_FrontendSatelliteMode_e8pskLdpc? 1:0;
            ipStreamerCtx->satSettings.ldpcPilot = eightpsk;
            ipStreamerCtx->satSettings.ldpcPilotPll = eightpsk;
            ipStreamerCtx->satSettings.ldpcPilotScan = !eightpsk;
        }
        BDBG_MSG(("%s: ldpcPilot %d, ldpcPilotPll %d ldpcPilotScan %d", BSTD_FUNCTION, ipStreamerCtx->satSettings.ldpcPilot, ipStreamerCtx->satSettings.ldpcPilotPll, ipStreamerCtx->satSettings.ldpcPilotScan));
    }
    if (ipStreamerCfg->fec) {
        ipStreamerCtx->satSettings.codeRate.numerator = ipStreamerCfg->fec == 910 ? 9 : ipStreamerCfg->fec / 10;
        ipStreamerCtx->satSettings.codeRate.denominator = ipStreamerCfg->fec == 910 ? 10 : ipStreamerCfg->fec % 10;
    }
    BDBG_MSG(("%s: else NEXUS_HAS_VIDEO_ENCODER Sat Freq %lu, symbolRate %d, mode %d", BSTD_FUNCTION, ipStreamerCtx->satSettings.frequency, ipStreamerCtx->satSettings.symbolRate, ipStreamerCtx->satSettings.mode));

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
    BDBG_MSG(("%s: using parserBand # %d", BSTD_FUNCTION, i));
#else
    /* pick an unused parser band */
    parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[frontendIndex % NEXUS_NUM_PARSER_BANDS];
#endif
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandMutex);
    if (parserBandPtr == NULL) {
        BDBG_ERR(("Failed to find a free parser band at %d", __LINE__));
        goto error;
    }

    NEXUS_Frontend_GetUserParameters(satSrc->frontendHandle, &userParams);
    BKNI_AcquireMutex(parserBandPtr->lock);
    if (parserBandPtr->refCount == 0) {
        parserBandPtr->frequency = ipStreamerCfg->frequency;
        parserBandPtr->subChannel = ipStreamerCfg->subChannel;
        /* associate parser band to the input band associated with this tuner */
        /* Map this parser band to the demod's input band. */
        NEXUS_ParserBand_GetSettings(parserBandPtr->parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
        if (ipStreamerCfg->satMode == NEXUS_FrontendSatelliteMode_eDss)
            parserBandSettings.transportType = NEXUS_TransportType_eDssEs; /* assumes DSS SD */
        else
            parserBandSettings.transportType = NEXUS_TransportType_eTs;
        if (ipStreamerCfg->enableAllpass) {
            parserBandSettings.allPass=true;
            parserBandSettings.acceptNullPackets=false;
        }
        rc = NEXUS_ParserBand_SetSettings(parserBandPtr->parserBand, &parserBandSettings);
        if (rc) {
            BDBG_ERR(("Failed to set the Nexus Parser band settings for band # %d", parserBandPtr->parserBand));
            BKNI_ReleaseMutex(parserBandPtr->lock);
            goto error;
        }
        NEXUS_Frontend_GetDiseqcSettings(satSrc->frontendHandle, &diseqcSettings);
        diseqcSettings.toneEnabled = ipStreamerCfg->toneEnabled;
        diseqcSettings.voltage = ipStreamerCfg->diseqcVoltage;
        if (NEXUS_Frontend_SetDiseqcSettings(satSrc->frontendHandle, &diseqcSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to set the Nexus Diseqc settings", BSTD_FUNCTION));
            BKNI_ReleaseMutex(parserBandPtr->lock);
            goto error;
        }
        BDBG_MSG(("%s: Updated Nexus Diseqc settings: toneEnabled %d, voltage %d", BSTD_FUNCTION, diseqcSettings.toneEnabled, diseqcSettings.voltage));
    }
    else {
        BDBG_MSG(("Nexus Parser band band # %d is already setup (ref cnt %d)", parserBandPtr->parserBand, parserBandPtr->refCount));
    }
    parserBandPtr->refCount++;
    ipStreamerCtx->parserBandPtr = parserBandPtr;
    BKNI_ReleaseMutex(parserBandPtr->lock);
    BDBG_MSG(("CTX %p: sat Frontend Src %p (ref count %d), Input Band %d & Parser Band %d (ref count %d) are opened",
                ipStreamerCtx, satSrc, satSrc->refCount, userParams.param1, ipStreamerCtx->parserBandPtr->parserBand, ipStreamerCtx->parserBandPtr->refCount));

    /* add new logic for  new Adc for demods */
    if (ipStreamerCfg->isNewAdc) {
        NEXUS_FrontendSatelliteRuntimeSettings settings;
        NEXUS_Frontend_GetSatelliteRuntimeSettings(satSrc->frontendHandle, &settings);
        settings.selectedAdc = ipStreamerCfg->newAdc;
        NEXUS_Frontend_SetSatelliteRuntimeSettings(satSrc->frontendHandle, &settings);

        BDBG_MSG(("%s: New Adc for Demod is  %d", BSTD_FUNCTION, settings.selectedAdc));
    }

    return 0;

error:
    if (parserBandPtr) {
        BKNI_AcquireMutex(parserBandPtr->lock);
        parserBandPtr->refCount--;
        BKNI_ReleaseMutex(parserBandPtr->lock);
    }
    if (!satSrc) return -1;
    BKNI_AcquireMutex(satSrc->lock);
    satSrc->refCount--;
    satSrc->psiAcquiring = false;
    BKNI_ReleaseMutex(satSrc->lock);
    return -1;
}
#endif

void
closeNexusSatSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    SatSrc *satSrc = ipStreamerCtx->satSrc;

#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCtx->transcoderDst) {
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        closeNexusTranscoderPipe(ipStreamerCtx, ipStreamerCtx->transcoderDst);
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        if (ipStreamerCtx->transcoderDst->refCount == 0) {
            /* clear the transcoding enabled flag in the parse band */
            ipStreamerCtx->parserBandPtr->transcodeEnabled = false;
            satSrc->transcodeEnabled = false;
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

    if (!satSrc || !satSrc->refCount)
        return;

    ipStreamerCtx->satSrc = NULL;
    BKNI_AcquireMutex(satSrc->lock);
    satSrc->refCount--;
    if (satSrc->psiAcquiredEvent)
        BKNI_ResetEvent(satSrc->psiAcquiredEvent);
    BKNI_ReleaseMutex(satSrc->lock);
    BDBG_MSG(("CTX %p: Closed a sat src %p, refCount %d", (void *)ipStreamerCtx, (void *)satSrc, satSrc->refCount));
}

int
setupAndAcquirePsiInfoSatSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx,
    B_PlaybackIpPsiInfo *psiOut
    )
{
    int i;
    SatSrc *satSrc;
    psiCollectionDataType  collectionData;

    memset(&collectionData, 0, sizeof(psiCollectionDataType));
    memset(psiOut, 0, sizeof(B_PlaybackIpPsiInfo));
    collectionData.srcType = ipStreamerCfg->srcType;
    collectionData.live = true;
    collectionData.parserBand = ipStreamerCtx->parserBandPtr->parserBand;
    collectionData.satSettings = &ipStreamerCtx->satSettings;
    collectionData.frontend = ipStreamerCtx->satSrc->frontendHandle;
    collectionData.signalLockedEvent = ipStreamerCtx->satSrc->signalLockedEvent;
    satSrc = ipStreamerCtx->satSrc;

    if (satSrc->numProgramsFound == 0) {
        /* PSI hasn't yet been acquired */
        BDBG_MSG(("CTX %p: Acquire Psi Info..., PB band %d", (void *)ipStreamerCtx, (int)collectionData.parserBand));

        /* get the PSI, this can take several seconds ... */
        acquirePsiInfo(&collectionData, &satSrc->psi[0], &satSrc->numProgramsFound);

        /* tell any other waiting thread that we are done acquiring PSI */
        BDBG_MSG(("%s: satSrc %p, lock %p", BSTD_FUNCTION, (void *)satSrc, (void *)satSrc->lock));
        BKNI_AcquireMutex(satSrc->lock);
        satSrc->psiAcquiring = false;
        BKNI_SetEvent(satSrc->psiAcquiredEvent);
        BKNI_ReleaseMutex(satSrc->lock);
        if (satSrc->numProgramsFound == 0) {
            BDBG_ERR(("%s: ERROR: Unable to Acquire PSI!!", BSTD_FUNCTION));
            return -1;
        }
    }
    else {
        BDBG_MSG(("CTX %p: Psi Info is already acquired...", (void *)ipStreamerCtx));
    }

    if (ipStreamerCfg->subChannel > satSrc->numProgramsFound) {
        BDBG_MSG(("Requested sub-channel # (%d) is not found in the total channels (%d) acquired, defaulting to 1st sub-channel", ipStreamerCfg->subChannel, satSrc->numProgramsFound));
        i = 0;
    }
    else {
        BDBG_MSG(("CTX %p: Requested sub-channel # (%d) is found in the total channels (%d) ", (void *)ipStreamerCtx, ipStreamerCfg->subChannel, satSrc->numProgramsFound));
        i = ipStreamerCfg->subChannel - 1; /* sub-channels start from 1, where as psi table starts from 0 */
        if (i < 0) i = 0;
    }
    memcpy(psiOut, &satSrc->psi[i], sizeof(B_PlaybackIpPsiInfo));
    BKNI_ResetEvent(satSrc->signalLockedEvent);
    return 0;
}


int
startNexusSatSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    SatSrc *satSrc = ipStreamerCtx->satSrc;

    BKNI_AcquireMutex(satSrc->lock);
    if (satSrc->started) {
        BKNI_ReleaseMutex(satSrc->lock);
        BDBG_MSG(("CTX %p: Sat Src %p is already started, refCount %d", (void *)ipStreamerCtx, (void *)satSrc, satSrc->refCount));
        return 0;
    }

    /* start frontend */
    BKNI_ResetEvent(satSrc->signalLockedEvent);
    NEXUS_StartCallbacks(satSrc->frontendHandle);
    BDBG_MSG(("CTX %p: TuneSatellite; sock %u; srcPos %u; srcFe %u; freq %u; tone %u; vpol %u; satMode %u; fec %u; sr %u; pids %d"
              ,(void *)ipStreamerCtx ,ipStreamerCtx->cfg.streamingFd, ipStreamerCtx->cfg.srcPosition, ipStreamerCtx->cfg.srcFe,
              ipStreamerCtx->cfg.frequency,  ipStreamerCtx->cfg.toneEnabled, ipStreamerCtx->cfg.verticalPolarization,
              ipStreamerCtx->cfg.satMode, ipStreamerCtx->cfg.fec, ipStreamerCtx->cfg.symbolRate, ipStreamerCtx->cfg.pidListCount ));
    rc = NEXUS_Frontend_TuneSatellite(satSrc->frontendHandle, &ipStreamerCtx->satSettings);
    if (rc) {
        BKNI_ReleaseMutex(satSrc->lock);
        BDBG_ERR(("Failed to Tune to Sat Frontend"));
        return -1;
    }
    if (BKNI_WaitForEvent(satSrc->signalLockedEvent, 5000)) {
        BKNI_ReleaseMutex(satSrc->lock);
        BDBG_MSG(("CTX %p: Sat Src %p FAILED TO LOCK THE SIGNAL ...", (void *)ipStreamerCtx, (void *)satSrc));
        return -1;
    }

    satSrc->started = true;
    BKNI_ReleaseMutex(satSrc->lock);
    BDBG_MSG(("CTX %p: Sat Src %p is started ...", (void *)ipStreamerCtx, (void *)satSrc));
    return 0;
}

void
stopNexusSatSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    SatSrc *satSrc = ipStreamerCtx->satSrc;
    int refCount;

    BKNI_AcquireMutex(satSrc->lock);
    refCount = satSrc->refCount;
    BKNI_ReleaseMutex(satSrc->lock);

    if (refCount == 1) {
        NEXUS_StopCallbacks(satSrc->frontendHandle);
        NEXUS_Frontend_Untune(satSrc->frontendHandle);
        satSrc->started = false;
        BDBG_MSG(("CTX %p: Sat Frontend Src is stopped ...", (void *)ipStreamerCtx));
    }
    else {
        BDBG_MSG(("CTX %p: Sat Frontend Src %p is not stopped, ref count %d ...", (void *)ipStreamerCtx, (void *)satSrc, refCount));
    }
}
#else /* !NEXUS_HAS_FRONTEND */
/* stub functions */
int initNexusSatSrcList(IpStreamerGlobalCtx *ipStreamerGlobalCtx)
{
    BSTD_UNUSED(ipStreamerGlobalCtx);

    BDBG_ERR(("%s: Streaming from Satellite Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", BSTD_FUNCTION));
    /* note: we let the init call succeed, but we will fail the open call */
    /* this way app wont fail during startup, but will fail a client request to open/start a recording */
    return 0;
}
void unInitNexusSatSrcList(IpStreamerGlobalCtx *ipStreamerGlobalCtx)
{
    BSTD_UNUSED(ipStreamerGlobalCtx);

    BDBG_ERR(("%s: Streaming from Satellite Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", BSTD_FUNCTION));
}
int openNexusSatSrc(IpStreamerConfig *ipStreamerCfg, IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCfg);
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from Satellite Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", BSTD_FUNCTION));
    return -1;
}
void closeNexusSatSrc(IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from Satellite Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", BSTD_FUNCTION));
}
int setupAndAcquirePsiInfoSatSrc(IpStreamerConfig *ipStreamerCfg, IpStreamerCtx *ipStreamerCtx, B_PlaybackIpPsiInfo *psiOut)
{
    BSTD_UNUSED(psiOut);
    BSTD_UNUSED(ipStreamerCfg);
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from Satellite Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", BSTD_FUNCTION));
    return -1;
}
int startNexusSatSrc(IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from Satellite Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", BSTD_FUNCTION));
    return -1;
}
int stopNexusSatSrc(IpStreamerCtx *ipStreamerCtx)
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: Streaming from Satellite Src can't be enabled on platforms w/o NEXUS_HAS_FRONTEND support", BSTD_FUNCTION));
    return -1;
}
#endif /* NEXUS_HAS_FRONTEND */
