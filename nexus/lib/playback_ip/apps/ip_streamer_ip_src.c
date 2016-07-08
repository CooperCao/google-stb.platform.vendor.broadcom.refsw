/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

int
initNexusIpSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    IpSrc *ipSrc;

    /* create the per src list mutex */
    if (BKNI_CreateMutex(&ipStreamerGlobalCtx->ipSrcMutex) != 0) {
        BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
        return -1;
    }

    /* Open Nexus Playpump, IP Applib directly feeds the network data to the Nexus Playpump */
    for (i = 0; i < IP_STREAMER_NUM_PLAYPUMPS_FOR_LIVE_IP; i++) {
        ipSrc = &ipStreamerGlobalCtx->ipSrcList[i];
        memset(ipSrc, 0, sizeof(IpSrc));
        if (BKNI_CreateEvent(&ipSrc->psiAcquiredEvent)) {
            BDBG_ERR(("Failed to create psiAcquired event at %d", __LINE__));
            return -1;
        }
        if (BKNI_CreateMutex(&ipSrc->lock) != 0) {
            BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
            return -1;
        }
    }

    BDBG_MSG(("%d IP Frontend Sources Initialized", i));
    return 0;
}

void
unInitNexusIpSrcList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    IpSrc *ipSrc;
    for (i = 0; i < IP_STREAMER_NUM_PLAYPUMPS_FOR_LIVE_IP; i++) {
        ipSrc = &ipStreamerGlobalCtx->ipSrcList[i];
        if (ipSrc->psiAcquiredEvent)
            BKNI_DestroyEvent(ipSrc->psiAcquiredEvent);
        if (ipSrc->lock)
            BKNI_DestroyMutex(ipSrc->lock);
    }
    if (ipStreamerGlobalCtx->ipSrcMutex)
        BKNI_DestroyMutex(ipStreamerGlobalCtx->ipSrcMutex);
    BDBG_MSG(("Uninitialized IP Frontend Sources..."));
}

int
openNexusIpSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    int i;
    bool matchFound = false;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    IpSrc *ipSrcList;
    IpSrc *ipSrc = NULL;
    B_PlaybackIpStatus playbackIpStatus;
    B_PlaybackIpSessionOpenSettings ipSessionOpenSettings;
    B_PlaybackIpSessionOpenStatus ipSessionOpenStatus;
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;

#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
#endif

    ipSrcList = &ipStreamerCtx->globalCtx->ipSrcList[0];
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->ipSrcMutex);
    /* check if the requested IP frontend is already being used by another thread */
    ipSrc = NULL;
    for (i = 0; i < IP_STREAMER_NUM_PLAYPUMPS_FOR_LIVE_IP; i++) {
        BKNI_AcquireMutex(ipSrcList[i].lock);
        /* check if this entry matches our request: it may or may not be currently used */
        if (strcmp(ipSrcList[i].srcIpAddress, ipStreamerCfg->srcIpAddress) == 0 && ipSrcList[i].srcPort == ipStreamerCfg->srcPort && ipSrcList[i].srcProtocol == ipStreamerCfg->srcProtocol) {
            ipSrc = &ipSrcList[i];
            /* since this src entry is/was already used to receive on this IP Channel, PSI acquisition was already done, so set the skip flag */
            if (ipSrc->numProgramsFound)
                ipStreamerCtx->skipPsiAcquisition = true;
            if (ipSrc->refCount == 0) {
                /* even though we found a match, this src entry is not being currently shared, so continue w/ src setup below */
                BKNI_ReleaseMutex(ipSrcList[i].lock);
                matchFound = true;
                break;
            }
            else {
                /* found a match, check if xcode params also match if enabled for this session */
                if (!ipStreamerCfg->transcodeEnabled
#ifdef NEXUS_HAS_VIDEO_ENCODER
                        ||
                        (ipStreamerCfg->transcodeEnabled && ipSrc->transcodeEnabled &&
                         ipSrc->transcode.outVideoCodec == ipStreamerCfg->transcode.outVideoCodec &&
                         ipSrc->transcode.outAudioCodec == ipStreamerCfg->transcode.outAudioCodec &&
                         ipSrc->transcode.outWidth == ipStreamerCfg->transcode.outWidth &&
                         ipSrc->transcode.outHeight == ipStreamerCfg->transcode.outHeight
                        )
#endif
                        )
                        {
                    /* we only reuse the ipSrc if this ipSrc already is being used for transcoding session and */
                    /* new session is also a transcoding session on the same parameters */
                    /* found a match, and this src entry is being currently shared, increment refCount & return */
                    ipSrc->refCount++;
#ifdef NEXUS_HAS_VIDEO_ENCODER
                    if (ipStreamerCfg->transcodeEnabled) {
                        ipStreamerCtx->transcoderDst = ipSrc->transcoderDst;
                        ipSrc->transcoderDst->refCount++;
                    }
#endif
                    ipStreamerCtx->ipSrc = ipSrc;
                    BKNI_ReleaseMutex(ipSrcList[i].lock);
                    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ipSrcMutex);
#ifdef NEXUS_HAS_VIDEO_ENCODER
                    if (ipStreamerCfg->transcodeEnabled) {
                        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
                    }
#endif
                    BDBG_MSG(("IP Ctx %p: Found an existing IP Src entry for the requested IP channel: src entry %p, currently shared %s, for ip %s, port %d",
                                (void *)ipStreamerCtx, (void *)ipSrc, ipSrc->refCount? "Yes":"No", ipStreamerCfg->srcIpAddress, ipStreamerCfg->srcPort));
                    return 0;
                }
                else {
                    /* xcode params didn't match, so continue looking for next free ipSrc */
                    ipSrc = NULL;
                }
            }
        }
        if (!ipSrcList[i].refCount ) {
            /* unused src entry */
            if (!ipSrc)
                /* note this 1st free src entry */
                ipSrc = &ipSrcList[i];
        }
        BKNI_ReleaseMutex(ipSrcList[i].lock);
    }
    if (ipSrc) {
        BDBG_MSG(("Found Free IP Src entry %p, shared %s, for ip %s, port %d", (void *)ipSrc, ipSrc->refCount? "Yes":"No", ipStreamerCfg->srcIpAddress, ipStreamerCfg->srcPort));
        BKNI_AcquireMutex(ipSrc->lock);
        ipStreamerCtx->ipSrc = ipSrc;
        NEXUS_PlaypumpSettings playpumpSettings;
        ipSrc->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
        if (!ipSrc->playpump) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            BKNI_ReleaseMutex(ipSrc->lock);
            BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ipSrcMutex);
#ifdef NEXUS_HAS_VIDEO_ENCODER
            if (ipStreamerCfg->transcodeEnabled) {
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
            }
#endif
            return -1;
        }
        NEXUS_Playpump_GetSettings(ipSrc->playpump, &playpumpSettings);
        /* IP Applib currently uses the FIFO mode to feed data, so app needs to set that mode */
        /* TODO: this can now be done inside IP Applib */
        playpumpSettings.mode = NEXUS_PlaypumpMode_eFifo;

        if(ipStreamerCtx->globalCtx->globalCfg.multiProcessEnv)
        {
            NEXUS_Platform_GetClientConfiguration(&clientConfig);
            playpumpOpenSettings.heap = clientConfig.heap[1]; /* playpump requires heap with eFull mapping */
        }
        rc = NEXUS_Playpump_SetSettings(ipSrc->playpump, &playpumpSettings);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            BKNI_ReleaseMutex(ipSrc->lock);
            BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ipSrcMutex);
#ifdef NEXUS_HAS_VIDEO_ENCODER
            if (ipStreamerCfg->transcodeEnabled) {
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
            }
#endif
            return -1;
        }

        /* Initialize IP Applib */
        ipSrc->playbackIp = B_PlaybackIp_Open(NULL);
        if (!ipSrc->playbackIp) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            BKNI_ReleaseMutex(ipSrc->lock);
            BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ipSrcMutex);
#ifdef NEXUS_HAS_VIDEO_ENCODER
            if (ipStreamerCfg->transcodeEnabled) {
                BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
            }
#endif
            return -1;
        }
        ipSrc->srcPort = ipStreamerCfg->srcPort;
        ipSrc->srcProtocol = ipStreamerCfg->srcProtocol;
        memcpy(ipSrc->srcIpAddress, ipStreamerCfg->srcIpAddress, sizeof(ipSrc->srcIpAddress));
        ipSrc->refCount++;
        if (!matchFound) {
            /* force PSI acquisition as this src is being used to receive a new IP channel */
            ipStreamerCtx->skipPsiAcquisition = false;
            ipSrc->numProgramsFound = 0;
        }
        BKNI_ReleaseMutex(ipSrc->lock);
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ipSrcMutex);
    if (!ipSrc) {
        BDBG_MSG(("No Free IP Src available for this ip %s, port %d", ipStreamerCfg->srcIpAddress, ipStreamerCfg->srcPort));
#ifdef NEXUS_HAS_VIDEO_ENCODER
        if (ipStreamerCfg->transcodeEnabled) {
            BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        }
#endif
        return -1;
    }

    /* setup this src entry  to receive Live IP Channel */
    memset(&ipSessionOpenSettings, 0, sizeof(ipSessionOpenSettings));
    memset(&ipSessionOpenStatus, 0, sizeof(ipSessionOpenStatus));
    memcpy(ipSessionOpenSettings.socketOpenSettings.ipAddr, ipStreamerCfg->srcIpAddress, sizeof(ipSessionOpenSettings.socketOpenSettings.ipAddr));
    ipSessionOpenSettings.socketOpenSettings.port = ipStreamerCfg->srcPort;
    ipSessionOpenSettings.socketOpenSettings.protocol = ipStreamerCfg->srcProtocol;
    ipSessionOpenSettings.socketOpenSettings.interfaceName = ipStreamerCfg->interfaceName;
    ipSessionOpenSettings.maxNetworkJitter = DOCSIS_IP_NETWORK_JITTER;
    ipSessionOpenSettings.networkTimeout = 1;  /* timeout in 1 sec during network outage events */
    rc = B_PlaybackIp_SessionOpen(ipSrc->playbackIp, &ipSessionOpenSettings, &ipSessionOpenStatus);
    if (rc != B_ERROR_SUCCESS) {
        rc = B_PlaybackIp_GetStatus(ipSrc->playbackIp, &playbackIpStatus);
        if (rc) {BDBG_WRN(("NEXUS Error (%d) at %d, returning...", rc, __LINE__)); return (-1);}
        BDBG_ERR(("Socket Open call failed, IP state %d", playbackIpStatus.ipState));
        goto error;
    }
#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        /* coverity[stack_use_local_overflow] */
        /* coverity[stack_use_overflow] */
        if ((ipSrc->transcoderDst = openNexusTranscoderPipe(ipStreamerCfg, ipStreamerCtx)) == NULL) {
            BDBG_ERR(("%s: Failed to open the transcoder pipe", __FUNCTION__));
            goto error;
        }
        ipSrc->transcodeEnabled = true;
        ipSrc->transcode = ipStreamerCfg->transcode;
        ipStreamerCtx->transcoderDst = ipSrc->transcoderDst;
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
#endif
    BDBG_MSG(("CTX %p: IP Src %p opened, refCount %d", (void *)ipStreamerCtx, (void *)ipSrc, ipSrc->refCount));
    return 0;

error:
#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
#endif
    if (ipSrc) {
        BKNI_AcquireMutex(ipSrc->lock);
        ipSrc->refCount--;
        BKNI_ReleaseMutex(ipSrc->lock);
    }
    return -1;
}

void
closeNexusIpSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    IpSrc *ipSrc = ipStreamerCtx->ipSrc;

    if (!ipSrc || !ipSrc->refCount)
        return;

#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCtx->transcoderDst) {
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        closeNexusTranscoderPipe(ipStreamerCtx, ipStreamerCtx->transcoderDst);
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        if (ipStreamerCtx->transcoderDst->refCount == 0) {
            ipSrc->transcodeEnabled = false;
            ipSrc->transcoderDst = NULL;
        }
    }
#endif
    BKNI_AcquireMutex(ipSrc->lock);
    ipSrc->refCount--;
    if (ipSrc->refCount == 0) {
        if (ipSrc->playpump)
            NEXUS_Playpump_Close(ipSrc->playpump);
        ipSrc->playpump = NULL;
        /* Close Socket related resources */
        if (ipSrc->playbackIp) {
            B_PlaybackIp_SessionClose(ipSrc->playbackIp);
            B_PlaybackIp_Close(ipSrc->playbackIp);
        }
        ipSrc->playbackIp = NULL;
    }
    BKNI_ReleaseMutex(ipSrc->lock);
    /* coverity[missing_lock] */
    ipStreamerCtx->ipSrc = NULL;

    BDBG_MSG(("CTX %p: IP Src %p is closed (ref cnt %d)", (void *)ipStreamerCtx, (void *)ipSrc, ipSrc->refCount));
}

int
setupAndAcquirePsiInfoIpSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx,
    B_PlaybackIpPsiInfo *psiOut
    )
{
    int i;
    IpSrc *ipSrc;
    psiCollectionDataType  collectionData;

    memset(&collectionData, 0, sizeof(psiCollectionDataType));
    memset(psiOut, 0, sizeof(B_PlaybackIpPsiInfo));
    collectionData.srcType = ipStreamerCfg->srcType;
    collectionData.live = true;
    ipSrc = ipStreamerCtx->ipSrc;
    collectionData.playpump = ipSrc->playpump;
    collectionData.live = true;
    collectionData.playbackIp = ipSrc->playbackIp;

    if (!ipStreamerCtx->skipPsiAcquisition) {
        /* since skipPsi flag is *not* set, so this IP src is now being used for a new IP channel, so acquire PSI info */
        /* PSI hasn't yet been acquired, check if any other thread is not in the process of acquiring it */
        BDBG_MSG(("CTX %p: Acquire Psi Info...", (void *)ipStreamerCtx));
        /* now get the PSI, this can take several seconds ... */
        acquirePsiInfo(&collectionData, &ipSrc->psi[0], &ipSrc->numProgramsFound);
        BKNI_SetEvent(ipSrc->psiAcquiredEvent);
    }
    else {
        if (ipSrc->numProgramsFound == 0) {
            /* other thread hasn't yet acquired PSI, so wait for it to acquire it */
            BDBG_MSG(("CTX %p: Another thread sharing the IP Src (ref count %d) hasn't yet acquired PSI info, wait for its completion...", (void *)ipStreamerCtx, ipSrc->refCount));
            if (BKNI_WaitForEvent(ipSrc->psiAcquiredEvent, 60000)) {
                BDBG_ERR(("%s: timeout while waiting for PSI acquisition by another thread", __FUNCTION__));
                return -1;
            }
        }
        else {
            /* PSI is already available, so use it */
        }
    }

    if (ipSrc->numProgramsFound == 0) {
        BDBG_MSG(("Unable to parse PSI!!"));
        return -1;
    }

    if (ipStreamerCfg->subChannel > ipSrc->numProgramsFound) {
        BDBG_MSG(("Requested sub-channel # (%d) is not found in the total channels (%d) acquired, defaulting to 1st sub-channel", ipStreamerCfg->subChannel, ipSrc->numProgramsFound));
        i = 0;
    }
    else {
        BDBG_MSG(("CTX %p: Requested sub-channel # (%d) is found in the total channels (%d) ", (void *)ipStreamerCtx, ipStreamerCfg->subChannel, ipSrc->numProgramsFound));
        i = ipStreamerCfg->subChannel - 1; /* sub-channels start from 1, where as psi table starts from 0 */
        if (i < 0) i = 0;
    }
    memcpy(psiOut, &ipSrc->psi[i], sizeof(B_PlaybackIpPsiInfo));
    return 0;
}

void
closeNexusIpSrcPidChannels(
    IpStreamerCtx *ipStreamerCtx
    )
{
    IpSrc *ipSrc = ipStreamerCtx->ipSrc;
    NEXUS_Playpump_CloseAllPidChannels(ipSrc->playpump);
}

int
openNexusIpSrcPidChannels(
    B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    IpSrc *ipSrc;
    NEXUS_PlaypumpOpenPidChannelSettings pidChannelSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;

    BSTD_UNUSED(ipStreamerCfg);
    ipSrc = ipStreamerCtx->ipSrc;
    /* Open the video pid channel */
    if (psi->videoPid) {
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eVideo;
        videoPidChannel = NEXUS_Playpump_OpenPidChannel(ipSrc->playpump, psi->videoPid, &pidChannelSettings);
        if (videoPidChannel == NULL)
            return -1;
    }
    else
        videoPidChannel = NULL;

    /* Open the audio pid channel */
    if (psi->audioPid) {
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eAudio;
        pidChannelSettings.pidTypeSettings.audio.codec = psi->audioCodec;
        audioPidChannel = NEXUS_Playpump_OpenPidChannel(ipSrc->playpump, psi->audioPid, &pidChannelSettings);
        if (audioPidChannel == NULL)
            return -1;
    }
    else
        audioPidChannel = NULL;

    /* Open the pcr pid channel if PCRs are not on the video pid packets */
    if (psi->pcrPid != psi->videoPid) {
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eOther;
        pcrPidChannel = NEXUS_Playpump_OpenPidChannel(ipSrc->playpump, psi->pcrPid, &pidChannelSettings);
        if (pcrPidChannel == NULL)
            return -1;
    }
    else
        pcrPidChannel = NULL;

#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        ipStreamerCtx->transcoderDst->videoPidChannel = videoPidChannel;
        ipStreamerCtx->transcoderDst->audioPidChannel = audioPidChannel;
        ipStreamerCtx->transcoderDst->pcrPidChannel = pcrPidChannel;
        if (setupNexusTranscoderPidChannels(psi, ipStreamerCfg, ipStreamerCtx) < 0) {
            BDBG_ERR(("%s: setupNexusTranscoderPidChannels() Failed", __FUNCTION__));
            return -1;
        }
    }
    else
#endif
    {
        ipStreamerCtx->videoPidChannel = videoPidChannel;
        ipStreamerCtx->audioPidChannel = audioPidChannel;
        ipStreamerCtx->pcrPidChannel = pcrPidChannel;

        /* Open the pat pid channel */
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eOther;
        ipStreamerCtx->patPidChannel = NEXUS_Playpump_OpenPidChannel(ipSrc->playpump, 0 /* PAT is always PID 0 */, &pidChannelSettings);
        if (ipStreamerCtx->patPidChannel == NULL)
            return -1;

        /* Open the pmt pid channel */
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eOther;
        ipStreamerCtx->pmtPidChannel = NEXUS_Playpump_OpenPidChannel(ipSrc->playpump, psi->pmtPid, &pidChannelSettings);
        if (ipStreamerCtx->pmtPidChannel == NULL)
            return -1;
    }

    return 0;
}

int
startNexusIpSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    IpSrc *ipSrc = ipStreamerCtx->ipSrc;
    B_PlaybackIpSessionStartSettings ipSessionStartSettings;
    B_PlaybackIpSessionStartStatus ipSessionStartStatus;

    BKNI_AcquireMutex(ipSrc->lock);
    if (ipSrc->started) {
        BKNI_ReleaseMutex(ipSrc->lock);
        BDBG_MSG(("CTX %p: IP Src %p is already started, refCount %d", (void *)ipStreamerCtx, (void *)ipSrc, ipSrc->refCount));
        return 0;
    }

    if (ipSrc->refCount == 1) {
        /* start playpump */
        rc = NEXUS_Playpump_Start(ipSrc->playpump);
        if (rc) {
            BKNI_ReleaseMutex(ipSrc->lock);
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }

        /* Start IP Applib */
        memset(&ipSessionStartSettings, 0, sizeof(ipSessionStartSettings));

        if (ipSrc->playpump)
            ipSessionStartSettings.nexusHandles.playpump = ipSrc->playpump;
        ipSessionStartSettings.nexusHandlesValid = true;
        rc = B_PlaybackIp_SessionStart(ipSrc->playbackIp, &ipSessionStartSettings, &ipSessionStartStatus);
        if (rc != B_ERROR_SUCCESS) {
            BKNI_ReleaseMutex(ipSrc->lock);
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }

        ipSrc->started = true;
    }
    BKNI_ReleaseMutex(ipSrc->lock);
    BDBG_MSG(("CTX %p: IP Src %p is started", (void *)ipStreamerCtx, (void *)ipSrc));
    return 0;
}

void
stopNexusIpSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    IpSrc *ipSrc = ipStreamerCtx->ipSrc;
    int refCount;

    BKNI_AcquireMutex(ipSrc->lock);
    refCount = ipSrc->refCount;
    BKNI_ReleaseMutex(ipSrc->lock);
    if (refCount == 1) {
        B_PlaybackIp_SessionStop(ipSrc->playbackIp);
        NEXUS_Playpump_Stop(ipSrc->playpump);
        ipSrc->started = false;
        BDBG_MSG(("CTX %p: IP Src %p is stopped", (void *)ipStreamerCtx, (void *)ipSrc));
    }
    else {
        BDBG_MSG(("CTX %p: IP Src %p is not stopped, refCount %d", (void *)ipStreamerCtx, (void *)ipSrc, refCount));
    }
}
