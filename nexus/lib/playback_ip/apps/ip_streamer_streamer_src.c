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
 *  stream out from streamer src
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

int
initNexusStreamerSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    unsigned i;
    StreamerSrc *streamerSrc;

    /* global reset */
    memset(ipStreamerGlobalCtx->streamerSrcList, 0, sizeof(ipStreamerGlobalCtx->streamerSrcList));

    if (BKNI_CreateMutex(&ipStreamerGlobalCtx->streamerSrcMutex) != 0) {
        BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
        return -1;
    }

    for (i = 0; i < IP_STREAMER_NUM_STREAMER_INPUT; i++) {
        streamerSrc = &ipStreamerGlobalCtx->streamerSrcList[i];
        memset(streamerSrc, 0, sizeof(StreamerSrc));
        if (BKNI_CreateEvent(&streamerSrc->psiAcquiredEvent)) {
            BDBG_ERR(("Failed to psiAcquired event at %d", __LINE__));
            return -1;
        }
        if (BKNI_CreateEvent(&streamerSrc->signalLockedEvent)) {
            BDBG_ERR(("Failed to signalLocked event at %d", __LINE__));
            return -1;
        }
        if (BKNI_CreateMutex(&streamerSrc->lock) != 0) {
            BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
            return -1;
        }
        BDBG_MSG(("%s: streamerSrc %p, lock %p", BSTD_FUNCTION, (void *)streamerSrc, (void *)streamerSrc->lock));
            /* successfully setup a streamer src */
    }

    BDBG_MSG(("################################## Initialized %d streamer Sources", i));
    return 0;
}

void
unInitNexusStreamerSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    StreamerSrc *streamerSrc;

    for (i = 0; i < IP_STREAMER_NUM_STREAMER_INPUT; i++) {
        streamerSrc = &ipStreamerGlobalCtx->streamerSrcList[i];
        if (streamerSrc->psiAcquiredEvent)
            BKNI_DestroyEvent(streamerSrc->psiAcquiredEvent);
        if (streamerSrc->signalLockedEvent)
            BKNI_DestroyEvent(streamerSrc->signalLockedEvent);
        if (streamerSrc->lock)
            BKNI_DestroyMutex(streamerSrc->lock);
    }
    /* global reset */
    memset(ipStreamerGlobalCtx->streamerSrcList, 0, sizeof(ipStreamerGlobalCtx->streamerSrcList));

    if (ipStreamerGlobalCtx->streamerSrcMutex)
        BKNI_DestroyMutex(ipStreamerGlobalCtx->streamerSrcMutex);
    BDBG_MSG(("Streamer Sources are uninitialized"));
}

int
openNexusStreamerSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    unsigned i;
    StreamerSrc *streamerSrc = NULL;
    StreamerSrc *streamerSrcList = NULL;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    ParserBand *parserBandPtr = NULL;
    B_PlaybackIpPsiInfo *cachedPsi = NULL;
    int parserBandIndex = 0;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBandSettings parserBandSettings;
    int numProgramsFound = 0;

#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
#endif

    streamerSrcList = &ipStreamerCtx->globalCtx->streamerSrcList[0];
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->streamerSrcMutex);
    for (i = 0; i < IP_STREAMER_NUM_STREAMER_INPUT; i++) {
        {
            /* a tuner is either currently or was already receiving streams for this frequency */
            if (!ipStreamerCfg->skipPsiAcquisition && streamerSrcList[i].psiAcquiring) {
                /* check if some other thread is in the process of doing psi discovery on this qam channel */
                /* if so, let that thread finish getting PSI and then proceed */
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->streamerSrcMutex);
#ifdef NEXUS_HAS_VIDEO_ENCODER
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
#endif
                BDBG_MSG(("CTX %p: Another thread is acquiring the PSI info, waiting for its completion...", (void *)ipStreamerCtx));
                if (BKNI_WaitForEvent(streamerSrcList[i].psiAcquiredEvent, 30000)) {
                    BDBG_ERR(("%s: timeout while waiting for PSI acquisition by another thread", BSTD_FUNCTION));
                    return -1;
                }
#ifdef NEXUS_HAS_VIDEO_ENCODER
                if (ipStreamerCfg->transcodeEnabled && streamerSrcList[i].transcodeEnabled) {
                    /* in xcode case, sleep to allow other thread finish setting up */
                    BDBG_MSG(("%s: delay runnig this thread ", BSTD_FUNCTION));
                    BKNI_Sleep(200);
                }
                BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
#endif
                BKNI_AcquireMutex(ipStreamerCtx->globalCtx->streamerSrcMutex);
            }
            if (streamerSrcList[i].numProgramsFound) {
                /* PSI info is available, set flag to skip it for this session and copy it from this */
                cachedPsi = streamerSrcList[i].psi;
                numProgramsFound = streamerSrcList[i].numProgramsFound;
                BDBG_MSG(("%s: freq matched to previously tuned channels, skip PSI acquisition and reuse it from cached copy", BSTD_FUNCTION));
            }

#ifdef NEXUS_HAS_VIDEO_ENCODER
            if (ipStreamerCfg->transcodeEnabled && streamerSrcList[i].transcodeEnabled) {
                parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[i];
                        if (parserBandPtr->subChannel == ipStreamerCfg->subChannel &&
                        parserBandPtr->transcode.outVideoCodec == ipStreamerCfg->transcode.outVideoCodec &&
                        parserBandPtr->transcode.outAudioCodec == ipStreamerCfg->transcode.outAudioCodec &&
                        parserBandPtr->transcode.outWidth == ipStreamerCfg->transcode.outWidth &&
                        parserBandPtr->transcode.outHeight == ipStreamerCfg->transcode.outHeight
                        ) {
                    /* we only reuse the frontend if this frontend already is being used for transcoding session and */
                    /* new session is also a transcoding session on the same frequency */
                    streamerSrc = &streamerSrcList[i];
                    parserBandIndex = i; /* 1:1 mapping for streamer input index and parser band index */
                    BDBG_MSG(("%s: reusing frontend index %d for xcode session", BSTD_FUNCTION, i));
                    break;
                }
            }
#endif
        }
        if (!streamerSrcList[i].refCount && !streamerSrc) {
            /* save a pointer if we haven't yet found a free unused tuner */
            streamerSrc = &streamerSrcList[i];
            parserBandIndex = i; /* 1:1 mapping for streamer input index and parser band index */
        }
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->streamerSrcMutex);
    if (streamerSrc) {
        BKNI_AcquireMutex(streamerSrc->lock);
        streamerSrc->refCount++;
        if (!ipStreamerCfg->skipPsiAcquisition) {
            if (!streamerSrc->numProgramsFound) {
                /* force PSI acquisition as this src is being used to receive a new channel */
                streamerSrc->numProgramsFound = 0;
                /* set flag to indicate this thread is getting PSI */
                streamerSrc->psiAcquiring = true;
            }
            else if (cachedPsi) {
                memcpy(streamerSrc->psi, cachedPsi, sizeof(B_PlaybackIpPsiInfo)*MAX_PROGRAMS_PER_FREQUENCY);
                /* coverity[use] */
                streamerSrc->numProgramsFound = numProgramsFound;
                BDBG_MSG(("%s: PSI data reused from cached copy", BSTD_FUNCTION));
            }
        }
        BKNI_ReleaseMutex(streamerSrc->lock);
        BDBG_MSG(("CTX %p: Found a free index %d, sat src %p, refCount %d", (void *)ipStreamerCtx, parserBandIndex, (void *)streamerSrc, streamerSrc->refCount));
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->streamerSrcMutex);
    if (!streamerSrc) {
        BDBG_ERR(("%s: No Free streamer available all streamers (# %d) busy", BSTD_FUNCTION, i));
        goto error;
    }
    ipStreamerCtx->streamerSrc = streamerSrc;

    /* associate parser band to the input band associated with this tuner */
    ipStreamerCtx->parserBandPtr = NULL;
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->parserBandMutex);
    parserBandPtr = &ipStreamerCtx->globalCtx->parserBandList[parserBandIndex];
    BDBG_MSG(("%s: using parserBand # %d", BSTD_FUNCTION, i));
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->parserBandMutex);
    if (parserBandPtr == NULL) {
        BDBG_ERR(("Failed to find a free parser band at %d", __LINE__));
        goto error;
    }

    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);
    BKNI_AcquireMutex(parserBandPtr->lock);
    if (parserBandPtr->refCount == 0) {
        /* associate parser band to the input band associated with streamer */
        NEXUS_ParserBand_GetSettings(parserBandPtr->parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = inputBand;
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
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

#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        if (parserBandPtr->transcodeEnabled && parserBandPtr->transcoderDst->refCount) {
            if (parserBandPtr->subChannel != ipStreamerCfg->subChannel ||
                parserBandPtr->transcode.outVideoCodec != ipStreamerCfg->transcode.outVideoCodec ||
                parserBandPtr->transcode.outHeight != ipStreamerCfg->transcode.outHeight) {
                BDBG_ERR(("%s: ERROR: Limiting to one transcoding sesion: Transcoder pipe (# of users %d) is already opened/enabled for this Live Channel", BSTD_FUNCTION, parserBandPtr->transcoderDst->refCount));
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
                goto error;
            }
            parserBandPtr->transcodeEnabled = true;
            streamerSrc->transcodeEnabled = true;
            parserBandPtr->transcode = ipStreamerCfg->transcode;
            parserBandPtr->subChannel = ipStreamerCfg->subChannel;
        }
        ipStreamerCtx->parserBandPtr = parserBandPtr;
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
    else
#endif
    {
        /* no transcode case */
        ipStreamerCtx->parserBandPtr = parserBandPtr;
    }

    BDBG_MSG(("CTX %p: streamer Src %p (ref count %d), Parser Band %d (ref count %d) are opened",
                (void *)ipStreamerCtx, (void *)streamerSrc, streamerSrc->refCount, (int)ipStreamerCtx->parserBandPtr->parserBand, ipStreamerCtx->parserBandPtr->refCount));
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
    return -1;
}

void
closeNexusStreamerSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    StreamerSrc *streamerSrc = ipStreamerCtx->streamerSrc;

    if (!streamerSrc || !streamerSrc->refCount)
        return;

#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCtx->transcoderDst) {
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        closeNexusTranscoderPipe(ipStreamerCtx, ipStreamerCtx->transcoderDst);
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        if (ipStreamerCtx->transcoderDst->refCount == 0) {
            /* clear the transcoding enabled flag in the parse band */
            ipStreamerCtx->parserBandPtr->transcodeEnabled = false;
            streamerSrc->transcodeEnabled = false;
        }
    }
#endif
    if (ipStreamerCtx->parserBandPtr) {
        /* free up the parser band */
        BKNI_AcquireMutex(ipStreamerCtx->parserBandPtr->lock);
        ipStreamerCtx->parserBandPtr->refCount--;
        BKNI_ReleaseMutex(ipStreamerCtx->parserBandPtr->lock);
        BDBG_MSG(("CTX %p: Closed a parser band src %p, refCount %d", (void *)ipStreamerCtx, (void *)ipStreamerCtx->parserBandPtr, ipStreamerCtx->parserBandPtr->refCount));
    }

    ipStreamerCtx->streamerSrc = NULL;
    BKNI_AcquireMutex(streamerSrc->lock);
    streamerSrc->refCount--;
    if (streamerSrc->psiAcquiredEvent)
        BKNI_ResetEvent(streamerSrc->psiAcquiredEvent);
    BKNI_ReleaseMutex(streamerSrc->lock);
    BDBG_MSG(("CTX %p: Closed a streamer src %p, refCount %d", (void *)ipStreamerCtx, (void *)streamerSrc, streamerSrc->refCount));
}

int
setupAndAcquirePsiInfoStreamerSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx,
    B_PlaybackIpPsiInfo *psiOut
    )
{
    int i;
    StreamerSrc *streamerSrc;
    psiCollectionDataType  collectionData;

    memset(&collectionData, 0, sizeof(psiCollectionDataType));
    memset(psiOut, 0, sizeof(B_PlaybackIpPsiInfo));
    collectionData.srcType = ipStreamerCfg->srcType;
    collectionData.live = true;
    collectionData.parserBand = ipStreamerCtx->parserBandPtr->parserBand;
    streamerSrc = ipStreamerCtx->streamerSrc;

    if (streamerSrc->numProgramsFound == 0) {
        /* PSI hasn't yet been acquired */
        BDBG_MSG(("CTX %p: Acquire Psi Info..., PB band %d", (void *)ipStreamerCtx, (int)collectionData.parserBand));

        /* get the PSI, this can take several seconds ... */
        acquirePsiInfo(&collectionData, &streamerSrc->psi[0], &streamerSrc->numProgramsFound);

        /* tell any other waiting thread that we are done acquiring PSI */
        BDBG_MSG(("%s: streamerSrc %p, lock %p", BSTD_FUNCTION, (void *)streamerSrc, (void *)streamerSrc->lock));
        BKNI_AcquireMutex(streamerSrc->lock);
        streamerSrc->psiAcquiring = false;
        BKNI_SetEvent(streamerSrc->psiAcquiredEvent);
        BKNI_ReleaseMutex(streamerSrc->lock);
        if (streamerSrc->numProgramsFound == 0) {
            BDBG_ERR(("%s: ERROR: Unable to Acquire PSI!!", BSTD_FUNCTION));
            return -1;
        }
    }
    else {
        BDBG_MSG(("CTX %p: Psi Info is already acquired...", (void *)ipStreamerCtx));
    }

    if (ipStreamerCfg->subChannel > streamerSrc->numProgramsFound) {
        BDBG_MSG(("Requested sub-channel # (%d) is not found in the total channels (%d) acquired, defaulting to 1st sub-channel", ipStreamerCfg->subChannel, streamerSrc->numProgramsFound));
        i = 0;
    }
    else {
        BDBG_MSG(("CTX %p: Requested sub-channel # (%d) is found in the total channels (%d) ", (void *)ipStreamerCtx, ipStreamerCfg->subChannel, streamerSrc->numProgramsFound));
        i = ipStreamerCfg->subChannel - 1; /* sub-channels start from 1, where as psi table starts from 0 */
        if (i < 0) i = 0;
    }
    memcpy(psiOut, &streamerSrc->psi[i], sizeof(B_PlaybackIpPsiInfo));
    BKNI_ResetEvent(streamerSrc->signalLockedEvent);
    return 0;
}
