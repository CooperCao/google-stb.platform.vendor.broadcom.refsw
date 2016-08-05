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
 *  stream out to network destination
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

BDBG_MODULE(ip_streamer);
#if 0
#define BDBG_MSG_FLOW(x)  BDBG_WRN( x );
#else
#define BDBG_MSG_FLOW(x)
#endif

/* initialize destination: ip */
int
initNexusIpDstList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    IpDst *ipDst;

    if (BKNI_CreateMutex(&ipStreamerGlobalCtx->ipDstMutex) != 0) {
        BDBG_ERR(("Failed to create BKNI mutex at %d", __LINE__));
        return -1;
    }

    /* half of destinations (rave ctx) are reserved ip streaming purpose and other half for local recording of live channels */
    for (i = 0; i < IP_STREAMER_NUM_RECPUMPS_FOR_STREAMING; i++) {
        ipDst = &ipStreamerGlobalCtx->ipDstList[i];
        memset(ipDst, 0, sizeof(IpDst));
        if (BKNI_CreateEvent(&ipDst->dataReadyEvent)) {
            BDBG_ERR(("Failed to psiAcquired event at %d", __LINE__));
            return -1;
        }
        /* coverity[missing_lock] */
        ipDst->inUse = false;
    }

    BDBG_MSG(("%d IP Destinations are initialized ", i));
    return 0;
}

void
unInitNexusIpDstList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    IpDst *ipDst;

    for (i = 0; i < IP_STREAMER_NUM_RECPUMPS_FOR_STREAMING; i++) {
        ipDst = &ipStreamerGlobalCtx->ipDstList[i];
        if (ipDst->dataReadyEvent)
            BKNI_DestroyEvent(ipDst->dataReadyEvent);
    }
    if (ipStreamerGlobalCtx->ipDstMutex)
        BKNI_DestroyMutex(ipStreamerGlobalCtx->ipDstMutex);
    BDBG_MSG(("%d IP Destinations are uninitialized ", i));
}

static void
prepareTimeshiftFileName(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    char *tmpPtr;
    struct timeval curTime;
    gettimeofday(&curTime, NULL);
    switch (ipStreamerCfg->srcType) {
#ifdef NEXUS_HAS_FRONTEND
    case IpStreamerSrc_eQam:
        snprintf(ipStreamerCfg->fileName, sizeof(ipStreamerCfg->fileName)-1,
            "%s/ts_file_qam%s_freq%d_ch%d_time_%d_%d.mpg",  /* mode freq sub-ch sec usec */
            ipStreamerCtx->globalCtx->globalCfg.timeshiftDirPath,
            ipStreamerCfg->qamMode == NEXUS_FrontendQamMode_e256 ? "256" : "64",
            ipStreamerCfg->frequency, ipStreamerCfg->subChannel, (int)curTime.tv_sec, (int)curTime.tv_usec
            );
        break;
    case IpStreamerSrc_eSat:
        snprintf(ipStreamerCfg->fileName, sizeof(ipStreamerCfg->fileName)-1,
            "%s/ts_file_sat%s_freq%d_ch%d_time_%d_%d.mpg",  /* mode freq sub-ch sec usec */
            ipStreamerCtx->globalCtx->globalCfg.timeshiftDirPath,
            ipStreamerCfg->satMode == NEXUS_FrontendSatelliteMode_eDss ? "dss" : "dvb",
            ipStreamerCfg->frequency, ipStreamerCfg->subChannel, (int)curTime.tv_sec, (int)curTime.tv_usec
            );
        break;
    case IpStreamerSrc_eVsb:
        snprintf(ipStreamerCfg->fileName, sizeof(ipStreamerCfg->fileName)-1,
            "%s/ts_file_vsb%s_freq%d_ch%d_time_%d_%d.mpg",  /* mode freq sub-ch sec usec */
            ipStreamerCtx->globalCtx->globalCfg.timeshiftDirPath,
            ipStreamerCfg->vsbMode == NEXUS_FrontendVsbMode_e8 ? "VSB8" : "VSB16",
            ipStreamerCfg->frequency, ipStreamerCfg->subChannel, (int)curTime.tv_sec, (int)curTime.tv_usec
            );
        break;
#endif /* NEXUS_HAS_FRONTEND */
    case IpStreamerSrc_eStreamer:
        snprintf(ipStreamerCfg->fileName, sizeof(ipStreamerCfg->fileName)-1,
            "%s/ts_file_streamer_ch%d_time_%d_%d.mpg",  /* mode freq sub-ch sec usec */
            ipStreamerCtx->globalCtx->globalCfg.timeshiftDirPath,
            ipStreamerCfg->subChannel, (int)curTime.tv_sec, (int)curTime.tv_usec
            );
        break;
    case IpStreamerSrc_eIp:
    case IpStreamerSrc_eFile:
        snprintf(ipStreamerCfg->fileName, sizeof(ipStreamerCfg->fileName)-1,
            "%s/ts_file_ip_proto%s_addr%d_port%d_ch%d_time_%d_%d.mpg",  /* proto ip port sub-ch sec usec */
            ipStreamerCtx->globalCtx->globalCfg.timeshiftDirPath,
            ipStreamerCfg->srcProtocol == B_PlaybackIpProtocol_eUdp ? "UDP" : ipStreamerCfg->srcProtocol == B_PlaybackIpProtocol_eRtp ? "RTP" : "RTPnoRTCP",
            inet_addr(ipStreamerCfg->srcIpAddress), ipStreamerCfg->srcPort, ipStreamerCfg->subChannel,
            (int)curTime.tv_sec, (int)curTime.tv_usec
            );
        break;
    default:
        BDBG_ERR(("Requested SourceType %d is not yet supported", ipStreamerCfg->srcType));
        return;
    }
    strncpy(ipStreamerCfg->indexFileName, ipStreamerCfg->fileName, strlen(ipStreamerCfg->fileName));
    tmpPtr = strstr(ipStreamerCfg->indexFileName, ".");
    if (tmpPtr)
        *tmpPtr = '\0';
    strncat(ipStreamerCfg->indexFileName, ".nav", sizeof(ipStreamerCfg->indexFileName)-1);
    BDBG_MSG(("Timeshift circular file name is %s, index name %s", ipStreamerCfg->fileName, ipStreamerCfg->indexFileName));
}

/* forward declaration */
int startNexusIpDst(IpStreamerCtx *ipStreamerCtx);
void stopNexusIpDst(IpStreamerCtx *ipStreamerCtx);

static void
errorCallbackDst(
    void *context,
    int param
    )
{
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)context;
    BDBG_WRN(("CTX %p: Got error during network streaming/recording: stopping ip/rec dst %d", (void *)ipStreamerCtx, param));
    if (ipStreamerCtx->ipDst)
        stopNexusIpDst(ipStreamerCtx);
}

static void
endOfStreamCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    BDBG_MSG(("%s: CTX: %p: End of stream reached for file playback", __FUNCTION__, context));
}

static void
dataReadyCallbackIpDst(
    void *context,
    int param
    )
{
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)context;
    IpStreamerSessionStatus status;
    BSTD_UNUSED(param);

    if (!ipStreamerCtx || !ipStreamerCtx->ipDst )
        return;
    B_IpStreamer_SessionStatus(ipStreamerCtx, &status);
    if (!status.active) {
        BDBG_ERR(("Session is not longer active, breaking out..."));
        return;
    }
    BKNI_SetEvent(ipStreamerCtx->ipDst->dataReadyEvent);
#if 0
    BDBG_MSG(("CTX %p: Genereated DataReady Event", ipStreamerCtx));
#endif
}

static void
overflowCallbackIpDst(
    void *context,
    int param
    )
{
    int rc;
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)context;
    BDBG_WRN(("CTX %p: Warning: Overflow during network streaming, param %d", (void *)ipStreamerCtx, param));
    ipStreamerCtx->overflow = true;

    /* stop & restart IP dst */
    stopNexusIpDst(ipStreamerCtx);
    rc = startNexusIpDst(ipStreamerCtx);
    if (rc) {
        BDBG_ERR(("Failed to start destinations"));
    }
    ipStreamerCtx->overflow = false;
    BDBG_MSG(("CTX %p: Restarted Streaming Buffer", (void *)ipStreamerCtx));
}

int
openNexusIpDstWithTimeshift(
    B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    int i;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    IpDst *ipDst;
    NEXUS_RecordSettings recordSettings;
    NEXUS_RecordPidChannelSettings pidSettings;
    B_PlaybackIpLiveStreamingOpenSettings liveStreamingSettings;
    NEXUS_FifoRecordSettings fifoRecordSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;
    NEXUS_PidChannelHandle patPidChannel;
    NEXUS_PidChannelHandle pmtPidChannel;
    NEXUS_VideoCodec videoCodec;
    int videoPid;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecpumpSettings recpumpSettings;
    int factor = 1;
    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_Timebase timebase;

    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->ipDstMutex);
    ipDst = NULL;
    for (i = 0; i < IP_STREAMER_NUM_RECPUMPS_FOR_STREAMING; i++) {
        if (!ipStreamerCtx->globalCtx->ipDstList[i].inUse) {
            ipDst = &ipStreamerCtx->globalCtx->ipDstList[i];
            ipDst->inUse = true;
            BDBG_MSG(("Found Free IP Dst entry %p, total IP dst entries %d", (void *)ipDst, IP_STREAMER_NUM_RECPUMPS_FOR_STREAMING));
            break;
        }
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ipDstMutex);
    if (!ipDst) {
        BDBG_ERR(("ERROR: No Free IP Dst entry, max %d", i));
        return -1;
    }

    /* setup the recpump for buffering the live channel */
    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    BDBG_MSG(("recpump (# %d): increasing by %d folds, data size %d, threshold %d, atom size %d, index size %d",
                i, factor, factor*recpumpOpenSettings.data.bufferSize, recpumpOpenSettings.data.dataReadyThreshold, recpumpOpenSettings.data.atomSize, factor*recpumpOpenSettings.index.bufferSize));
    /* increase the default record buffer size as IP streaming can introduce delays */
    recpumpOpenSettings.data.bufferSize = factor * recpumpOpenSettings.data.bufferSize;
    /* dataReadThreshold may have to be lowered if USE_TRANSPORT_TIMESTAMPSs are enabled */
    /* for non-TTS mode, this delay gets hidden in the client's jitter settings */
    recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.atomSize * 24;
    recpumpOpenSettings.index.bufferSize = factor * recpumpOpenSettings.index.bufferSize;
    ipDst->recpumpHandle = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
    if (!ipDst->recpumpHandle) {
        BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
        return -1;
    }
    NEXUS_Recpump_GetSettings(ipDst->recpumpHandle, &recpumpSettings);
    BDBG_MSG(("recpump (# %d): increased by %d folds, data size %d, threshold %d, atom size %d, index size %d",
                i, factor, factor*recpumpOpenSettings.data.bufferSize, recpumpOpenSettings.data.dataReadyThreshold, recpumpOpenSettings.data.atomSize, factor*recpumpOpenSettings.index.bufferSize));

    ipDst->recordHandle = NEXUS_Record_Create();
    if (!ipDst->recordHandle) {
        BDBG_ERR(("NEXUS Error at %d, failed to create record handle, returning..", __LINE__));
        return -1;
    }

    NEXUS_Record_GetSettings(ipDst->recordHandle, &recordSettings);
    recordSettings.recpump = ipDst->recpumpHandle;
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (ipStreamerCfg->pvrEncryptionEnabled) {
        ipStreamerCtx->pvrEncKeyHandle = _createKeyHandle(NEXUS_SecurityOperation_eEncrypt);
        if (!ipStreamerCtx->pvrEncKeyHandle) {
            BDBG_ERR(("%s: Failed to allocate enc keyslot", __FUNCTION__));
            return -1;
        }
        ipStreamerCtx->pvrDecKeyHandle = _createKeyHandle(NEXUS_SecurityOperation_eDecrypt);
        if (!ipStreamerCtx->pvrDecKeyHandle) {
            BDBG_ERR(("%s: Failed to allocate dec keyslot", __FUNCTION__));
            return -1;
        }
        recordSettings.recpumpSettings.securityDma = ipStreamerCtx->globalCtx->globalCfg.dmaHandle;
        recordSettings.recpumpSettings.securityContext = ipStreamerCtx->pvrEncKeyHandle;
        BDBG_MSG(("%s: PVR Encryption successfully setup", __FUNCTION__));
    }
    else {
        recordSettings.recpumpSettings.securityContext = NULL;
        recordSettings.recpumpSettings.securityDma = NULL;
    }
#endif
    if (ipStreamerCfg->enableTimestamps)
        recordSettings.recpumpSettings.timestampType = NEXUS_TransportTimestampType_e32_Binary;
    recordSettings.errorCallback.callback = errorCallbackDst;
    recordSettings.errorCallback.param = 0; /* used in the callback to determine streaming or recording context */
    recordSettings.errorCallback.context = ipStreamerCtx;
    recordSettings.overflowCallback.callback = overflowCallbackIpDst;
    recordSettings.overflowCallback.param = 0; /* used in the callback to determine streaming or recording context */
    recordSettings.overflowCallback.context = ipStreamerCtx;
    NEXUS_Record_SetSettings(ipDst->recordHandle, &recordSettings);

    /*
     * Streaming Live data to client: inorder to support timeshifting, need to do following:
     *  -Open FIFO file (live data is written to this circular file)
     *  -Pass PSI & FIFO file info to IP library via Live Streaming Open
     */
    prepareTimeshiftFileName(ipStreamerCfg, ipStreamerCtx);
    ipDst->fifoFileHandle = NEXUS_FifoRecord_Create(ipStreamerCfg->fileName, ipStreamerCfg->indexFileName);
    if (!ipDst->fifoFileHandle) {
        BDBG_ERR(("ERROR: Failed to create the Circular file for Timeshifting live content"));
        return -1;
    }
    NEXUS_FifoRecord_GetSettings(ipDst->fifoFileHandle, &fifoRecordSettings);
    fifoRecordSettings.interval = ipStreamerCtx->globalCtx->globalCfg.timeshiftBufferInterval;
    fifoRecordSettings.data.soft = fifoRecordSettings.interval * (off_t)(ipStreamerCtx->globalCtx->globalCfg.maxBitRate/8);
    fifoRecordSettings.data.hard = fifoRecordSettings.data.soft* 2;
    fifoRecordSettings.index.soft = fifoRecordSettings.interval * (off_t)(60 * 64 * 2); /* interval * content of max frame rate 60fps * max NAV entry size for AVC * margin of 2 */
    fifoRecordSettings.index.hard = fifoRecordSettings.index.soft* 2;
    BDBG_MSG(("Timeshift Buffer Size: data %lld, index %lld, interval %d", fifoRecordSettings.data.soft, fifoRecordSettings.index.soft, fifoRecordSettings.interval));
    rc = NEXUS_FifoRecord_SetSettings(ipDst->fifoFileHandle, &fifoRecordSettings);
    BDBG_ASSERT(!rc);

    ipDst->fileRecordHandle = NEXUS_FifoRecord_GetFile(ipDst->fifoFileHandle);

    if (ipStreamerCfg->usePlaybackForStreamingFiles) {
        NEXUS_PlaybackPidChannelSettings pidCfg0;
        NEXUS_PlaybackSettings playbackSettings;

        if ((ipStreamerCtx->fileSrc = BKNI_Malloc(sizeof(FileSrc))) == NULL) { BDBG_ERR(("BKNI_Malloc Failure at %d", __LINE__)); return -1;}
        BKNI_Memset(ipStreamerCtx->fileSrc, 0, sizeof(FileSrc));
        if ((ipStreamerCtx->fileSrc->playpumpHandle = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL)) == NULL) {
            BDBG_ERR(("%s: ERROR: Failed to Open a free Nexus Playpump idx", __FUNCTION__));
            return -1;
        }
        if ((ipStreamerCtx->fileSrc->playbackHandle = NEXUS_Playback_Create()) == NULL) {
            BDBG_ERR(("%s: ERROR: Failed to create a Nexus Playback instance", __FUNCTION__));
            return -1;
        }
        if ((ipStreamerCtx->fileSrc->timebase = NEXUS_Timebase_Open(NEXUS_ANY_ID)) == NEXUS_Timebase_eInvalid) {
            BDBG_ERR(("%s: ERROR: NEXUS_Timebase_Open Failed to open a free Timebase", __FUNCTION__));
            return -1;
        }
        timebase = ipStreamerCtx->fileSrc->timebase;
        NEXUS_Timebase_GetSettings(timebase, &timebaseSettings);
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
        timebaseSettings.freeze = true;
        timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e122ppm;
        if (NEXUS_Timebase_SetSettings(timebase, &timebaseSettings)) {
            BDBG_ERR(("%s: ERROR: NEXUS_Timebase_SetSettings Failed to open a free STC Channel %s", __FUNCTION__, ipStreamerCfg->fileName));
            return -1;
        }
        NEXUS_Playback_GetSettings(ipStreamerCtx->fileSrc->playbackHandle, &playbackSettings);
        playbackSettings.playpump = ipStreamerCtx->fileSrc->playpumpHandle;
        playbackSettings.playpumpSettings.transportType = psi->mpegType;
        playbackSettings.playpumpSettings.timestamp.pacing = true;
        playbackSettings.playpumpSettings.timestamp.pacingMaxError = 0xdad;
        playbackSettings.playpumpSettings.timestamp.pacingOffsetAdjustDisable = true;
        playbackSettings.playpumpSettings.timestamp.resetPacing = true;
        playbackSettings.playpumpSettings.timestamp.timebase = timebase;
        playbackSettings.playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_e32_Binary;
        playbackSettings.beginningOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
        playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePlay;
        playbackSettings.endOfStreamCallback.callback = NULL;
        playbackSettings.endOfStreamCallback.context = NULL;
        playbackSettings.errorCallback.callback = endOfStreamCallback;
        playbackSettings.errorCallback.context = ipStreamerCtx;
        playbackSettings.playpumpSettings.allPass=true;
        playbackSettings.timeshifting = true;
        if (NEXUS_Playback_SetSettings(ipStreamerCtx->fileSrc->playbackHandle, &playbackSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_Playback_SetSettings() Failed", __FUNCTION__));
            return -1;
        }
        NEXUS_Playback_GetDefaultPidChannelSettings(&pidCfg0);
        NEXUS_Playpump_GetAllPassPidChannelIndex(playbackSettings.playpump, &pidCfg0.pidSettings.pidSettings.pidChannelIndex );
        ipStreamerCtx->allPassPidChannel = NEXUS_Playback_OpenPidChannel(ipStreamerCtx->fileSrc->playbackHandle, 0x00, &pidCfg0);   /* PID is ignored in allPass mode */
        if (ipStreamerCtx->allPassPidChannel == NULL) {
            BDBG_ERR(("%s: NEXUS_Playback_OpenPidChannel() Failed", __FUNCTION__));
            return -1;
        }
        NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
        BDBG_MSG(("recpump: increasing by %d folds, data size %d, threshold %d, atom size %d, index size %d",
                    factor, factor*recpumpOpenSettings.data.bufferSize, recpumpOpenSettings.data.dataReadyThreshold, recpumpOpenSettings.data.atomSize, factor*recpumpOpenSettings.index.bufferSize));
        recpumpOpenSettings.data.bufferSize = factor * recpumpOpenSettings.data.bufferSize;
        recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.atomSize * 22;
        recpumpOpenSettings.index.bufferSize = factor * recpumpOpenSettings.index.bufferSize;
        ipDst->recpumpAllpassHandle = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
        if (!ipDst->recpumpAllpassHandle) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
        NEXUS_Recpump_GetSettings(ipDst->recpumpAllpassHandle, &recpumpSettings);
        BDBG_MSG(("CTX %p: recpump (ptr %p, # %d): increased by %d folds, data size %d, threshold %d, atom size %d, index size %d",
                    (void *)ipStreamerCtx, (void *)ipDst->recpumpAllpassHandle, i, factor, factor*recpumpOpenSettings.data.bufferSize, recpumpOpenSettings.data.dataReadyThreshold, recpumpOpenSettings.data.atomSize, factor*recpumpOpenSettings.index.bufferSize));
        recpumpSettings.data.dataReady.callback = dataReadyCallbackIpDst;
        recpumpSettings.data.dataReady.context = ipStreamerCtx;
        recpumpSettings.outputTransportType = psi->mpegType;
        recpumpSettings.timestampType = NEXUS_TransportTimestampType_e32_Mod300;
        recpumpSettings.bandHold = true;
        if (NEXUS_Recpump_SetSettings(ipDst->recpumpAllpassHandle, &recpumpSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to update recpump settings", __FUNCTION__));
            return -1;
        }
        rc = NEXUS_Recpump_AddPidChannel(ipDst->recpumpAllpassHandle, ipStreamerCtx->allPassPidChannel, NULL);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
    }
    /* call IP Applib to setup the streaming from Rave buffers */
    memset(&liveStreamingSettings, 0, sizeof(liveStreamingSettings));
    liveStreamingSettings.protocol = B_PlaybackIpProtocol_eHttp;
    liveStreamingSettings.streamingFd = ipStreamerCfg->streamingFd;
    liveStreamingSettings.enableTimeshifting = true;
    if (ipStreamerCfg->streamingFd <= 0)
        liveStreamingSettings.streamingDisabled = true;
    liveStreamingSettings.psi = psi;
    liveStreamingSettings.fifoFileHandle = ipDst->fifoFileHandle;
    memcpy(liveStreamingSettings.fileName, ipStreamerCfg->fileName, sizeof(liveStreamingSettings.fileName)-1);
    liveStreamingSettings.eventCallback = ipStreamerCfg->eventCallback;
    liveStreamingSettings.appCtx = ipStreamerCfg->appCtx;
    liveStreamingSettings.hlsSession = ipStreamerCfg->hlsSession;
    liveStreamingSettings.hlsSegmentSize = ipStreamerCfg->hlsSegmentSize;
    if (ipStreamerCfg->appHeader.valid) {
        if (ipStreamerCfg->appHeader.length > sizeof(liveStreamingSettings.appHeader.data)) {
            BDBG_ERR(("%s: ERROR: app header length (%d) is > data buffer in ip streamer (%d)", __FUNCTION__, ipStreamerCfg->appHeader.length, sizeof(liveStreamingSettings.appHeader.data)));
            return -1;
        }
        liveStreamingSettings.appHeader.valid = ipStreamerCfg->appHeader.valid;
        liveStreamingSettings.appHeader.length = ipStreamerCfg->appHeader.length;
        memcpy(liveStreamingSettings.appHeader.data, ipStreamerCfg->appHeader.data, ipStreamerCfg->appHeader.length);
    }
    if (ipStreamerCfg->usePlaybackForStreamingFiles)
        liveStreamingSettings.recpumpHandle = ipDst->recpumpAllpassHandle;
    else
        liveStreamingSettings.recpumpHandle = ipDst->recpumpHandle;
    liveStreamingSettings.dataReadyEvent = ipDst->dataReadyEvent;

    BDBG_MSG(("event callback appCtx %p", liveStreamingSettings.appCtx));
#ifdef B_HAS_DTCP_IP
    if (ipStreamerCfg->encryptionEnabled) {
        liveStreamingSettings.securitySettings.securityProtocol = B_PlaybackIpSecurityProtocol_DtcpIp;
        liveStreamingSettings.securitySettings.enableEncryption = true;
        liveStreamingSettings.securitySettings.initialSecurityContext = ipStreamerCtx->globalCtx->dtcpCtx;
        liveStreamingSettings.securitySettings.settings.dtcpIp.emiValue = ipStreamerCfg->emiValue;
    }
#endif
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (ipStreamerCfg->pvrEncryptionEnabled) {
        liveStreamingSettings.securitySettings.enableDecryption = true;
        liveStreamingSettings.securitySettings.dmaHandle = ipStreamerCtx->globalCtx->globalCfg.dmaHandle;
        liveStreamingSettings.securitySettings.pvrDecKeyHandle = ipStreamerCtx->pvrDecKeyHandle;
    }
#endif
    liveStreamingSettings.heapHandle = ipStreamerCtx->globalCtx->globalCfg.heapHandle;
    if ((ipDst->liveStreamingHandle = B_PlaybackIp_LiveStreamingOpen(&liveStreamingSettings)) == NULL) {
        BDBG_ERR(("%s: ERROR: Failed to open Network Streaming handle", __FUNCTION__));
        return -1;
    }

    videoPidChannel = ipStreamerCtx->videoPidChannel;
    audioPidChannel = ipStreamerCtx->audioPidChannel;
    pcrPidChannel = ipStreamerCtx->pcrPidChannel;
    patPidChannel = ipStreamerCtx->patPidChannel;
    pmtPidChannel = ipStreamerCtx->pmtPidChannel;
    videoCodec = psi->videoCodec;
    videoPid = psi->videoPid;
#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        /* transcoding session */
        BDBG_MSG(("Updated PidChannels for Transcoding case "));
        videoPidChannel = ipStreamerCtx->transcodeVideoPidChannel;
        audioPidChannel = ipStreamerCtx->transcodeAudioPidChannel;
        pcrPidChannel = ipStreamerCtx->transcodePcrPidChannel;
        patPidChannel = ipStreamerCtx->transcodePatPidChannel;
        pmtPidChannel = ipStreamerCtx->transcodePmtPidChannel;
        videoCodec = ipStreamerCtx->cfg.transcode.outVideoCodec;
        videoPid = ipStreamerCtx->cfg.transcode.outVideoPid;
    }
#endif
    /* configure the video pid for indexing */
    NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
    pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
    pidSettings.recpumpSettings.pidTypeSettings.video.codec = videoCodec;
    /* TODO: add option to insert timestamp into the stream */

    if (videoPidChannel) {
        rc = NEXUS_Record_AddPidChannel(ipDst->recordHandle, videoPidChannel, &pidSettings);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
    }

    if (audioPidChannel) {
        rc = NEXUS_Record_AddPidChannel(ipDst->recordHandle, audioPidChannel, NULL);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
    }

    if (pcrPidChannel) {
        rc = NEXUS_Record_AddPidChannel(ipDst->recordHandle, pcrPidChannel, NULL);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
    }

#ifdef STREAMER_CABLECARD_SUPPORT
#ifdef MPODXSIX
    if (ipStreamerCtx->caPidChannel) {
        rc = NEXUS_Record_AddPidChannel(ipDst->recordHandle, ipStreamerCtx->caPidChannel, NULL);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
    }
#endif
#endif

    rc = NEXUS_Record_AddPidChannel(ipDst->recordHandle, patPidChannel, NULL);
    if (rc) {
        BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
        return -1;
    }

    rc = NEXUS_Record_AddPidChannel(ipDst->recordHandle, pmtPidChannel, NULL);
    if (rc) {
        BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
        return -1;
    }

    ipStreamerCtx->ipDst = ipDst;
    BDBG_MSG(("CTX %p: IP Dst %p is opened", (void *)ipStreamerCtx, (void *)ipDst));
    return 0;
}

int
openNexusIpDstWithoutTimeshift(
    B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    int i;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    IpDst *ipDst;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpStatus recpumpStatus;
    NEXUS_RecpumpAddPidChannelSettings pidSettings;
    B_PlaybackIpLiveStreamingOpenSettings liveStreamingSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;
    NEXUS_PidChannelHandle patPidChannel;
    NEXUS_PidChannelHandle pmtPidChannel;
    NEXUS_VideoCodec videoCodec;
    int videoPid;
    int factor = 1; /* TODO: need to check if default rave buffer size is enough for streaming out */

    BDBG_MSG(("%s: ipStreamerCtx %p", __FUNCTION__, (void *)ipStreamerCtx ));
    BDBG_MSG(("%s: freq %u; pidListCount %u; ip (%s); port (%u); socket (%u)", __FUNCTION__, ipStreamerCtx->cfg.frequency,
              ipStreamerCtx->cfg.pidListCount, ipStreamerCtx->cfg.srcIpAddress, ipStreamerCtx->cfg.srcPort, ipStreamerCtx->cfg.streamingFd ));
    ipStreamerCtx->ipDst = NULL;
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->ipDstMutex);
    ipDst = NULL;
    for (i = 0; i < IP_STREAMER_NUM_RECPUMPS_FOR_STREAMING; i++) {
        if (!ipStreamerCtx->globalCtx->ipDstList[i].inUse) {
            ipDst = &ipStreamerCtx->globalCtx->ipDstList[i];
            ipDst->inUse = true;
            BDBG_MSG(("Found Free IP Dst entry %p, total IP dst entries %d", (void *)ipDst, IP_STREAMER_NUM_RECPUMPS_FOR_STREAMING));
            break;
        }
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ipDstMutex);
    if (!ipDst) {
        BDBG_ERR(("ERROR: No Free IP Dst entry, max %d", i));
        return -1;
    }

    /* setup the recpump for buffering the live channel */
    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    BDBG_MSG(("recpump %d: increasing by %d folds, data size %d, threshold %d, atom size %d, index size %d",
                i, factor, factor*recpumpOpenSettings.data.bufferSize, recpumpOpenSettings.data.dataReadyThreshold, recpumpOpenSettings.data.atomSize, factor*recpumpOpenSettings.index.bufferSize));
    /* increase the default record buffer size as IP streaming can introduce delays */
    recpumpOpenSettings.data.bufferSize = factor * recpumpOpenSettings.data.bufferSize;
    /* dataReadThreshold may have to be lowered if USE_TRANSPORT_TIMESTAMPSs are enabled */
    /* for non-TTS mode, this delay gets hidden in the client's jitter settings */
    if (ipStreamerCfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eRtp || ipStreamerCfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eUdp) {
        /* Todo: Further investigate this threshold and how it effect Macroblocking in Rtp.
              5 gives the balancing of not overly generating RAVE interrupts and macroblocking on client */
        recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.atomSize * 5;
    }
    else
    {
        recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.atomSize * 22;
    }
    recpumpOpenSettings.index.bufferSize = factor * recpumpOpenSettings.index.bufferSize;
    ipDst->recpumpHandle = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
    if (!ipDst->recpumpHandle) {
        BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
        return -1;
    }
    NEXUS_Recpump_GetSettings(ipDst->recpumpHandle, &recpumpSettings);
    BDBG_MSG(("CTX %p: recpump %d: (ptr %p): increased by %d folds, data size %-9d, threshold %-5d, atom size %-4d",
              (void *)ipStreamerCtx, i, (void *)ipDst->recpumpHandle, factor, factor*recpumpOpenSettings.data.bufferSize,
              recpumpOpenSettings.data.dataReadyThreshold, recpumpOpenSettings.data.atomSize ));
    BDBG_MSG(("CTX %p: recpump %d: (ptr %p): increased by %d folds, indx size %-9d, threshold %-5d, atom size %-4d",
              (void *)ipStreamerCtx, i, (void *)ipDst->recpumpHandle, factor, factor*recpumpOpenSettings.index.bufferSize,
              recpumpOpenSettings.index.dataReadyThreshold, recpumpOpenSettings.index.atomSize ));
    NEXUS_Recpump_GetStatus(ipDst->recpumpHandle, &recpumpStatus );

#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (ipStreamerCfg->pvrEncryptionEnabled) {
        ipStreamerCtx->pvrEncKeyHandle = _createKeyHandle(NEXUS_SecurityOperation_eEncrypt);
        if (!ipStreamerCtx->pvrEncKeyHandle) {
            BDBG_ERR(("%s: Failed to allocate enc keyslot", __FUNCTION__));
            return -1;
        }
        ipStreamerCtx->pvrDecKeyHandle = _createKeyHandle(NEXUS_SecurityOperation_eDecrypt);
        if (!ipStreamerCtx->pvrDecKeyHandle) {
            BDBG_ERR(("%s: Failed to allocate dec keyslot", __FUNCTION__));
            return -1;
        }
        recpumpSettings.securityDma = ipStreamerCtx->globalCtx->globalCfg.dmaHandle;
        recpumpSettings.securityContext = ipStreamerCtx->pvrEncKeyHandle;
        BDBG_MSG(("%s: PVR Encryption successfully setup", __FUNCTION__));
    }
    else {
        recpumpSettings.securityContext = NULL;
        recpumpSettings.securityDma = NULL;
    }
#endif

    if (ipStreamerCfg->enableTimestamps)
    {
        /* non timeshift case */
        recpumpSettings.timestampType = NEXUS_TransportTimestampType_e32_Binary;
    }
    recpumpSettings.data.dataReady.callback = dataReadyCallbackIpDst;
    recpumpSettings.data.dataReady.context = ipStreamerCtx;

    /* should always enable this once recpump thresholds are correctly set: SW7425-2663; this will avoid recpump overflow. */
    if (ipStreamerCfg->transcodeEnabled) {
        /* enable bandhold to allow rave to back-pressure playback which will backpressure encoder to drop frames in case of rave overflow */
        recpumpSettings.bandHold = true;
    }

    /* note: for non-timeshift case, we are not setting up the recpump overflow callback, instead live streaming thread is monitoring the buffer depths and carries out the fifo flush when overflow happens */
    if (NEXUS_Recpump_SetSettings(ipDst->recpumpHandle, &recpumpSettings) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: Failed to update recpump settings", __FUNCTION__));
        return -1;
    }

    /* call IP Applib to setup the streaming from Rave buffers */
    memset(&liveStreamingSettings, 0, sizeof(liveStreamingSettings));
    liveStreamingSettings.protocol = ipStreamerCfg->streamingCfg.streamingProtocol;
    liveStreamingSettings.enableTimeshifting = false;
    if (ipStreamerCfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eRtp || ipStreamerCfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eUdp) {
        if ( &liveStreamingSettings.rtpUdpSettings !=NULL )
        {
            BDBG_MSG(("%s: setting interface (%s)", __FUNCTION__, ipStreamerCtx->globalCtx->globalCfg.interfaceName )); fflush(stdout);
            liveStreamingSettings.rtpUdpSettings.interfaceName = ipStreamerCtx->globalCtx->globalCfg.interfaceName;
            BDBG_MSG(("%s: setting port streamingPort (%u); srcPort (%u); addr (%s)", __FUNCTION__,
                      ipStreamerCfg->streamingCfg.streamingPort ,ipStreamerCtx->cfg.srcPort, ipStreamerCfg->streamingCfg.streamingIpAddress  ));
            liveStreamingSettings.rtpUdpSettings.streamingPort = ipStreamerCfg->streamingCfg.streamingPort;
            strncpy(liveStreamingSettings.rtpUdpSettings.streamingIpAddress, ipStreamerCfg->streamingCfg.streamingIpAddress,
                    sizeof(liveStreamingSettings.rtpUdpSettings.streamingIpAddress)-1);
        }
    } else {
        /* For non-UDP, non-RTP protocols, socket connection is already there as client requests for the session via HTTP/TCP */
        liveStreamingSettings.streamingFd = ipStreamerCfg->streamingFd;
    }
    liveStreamingSettings.eventCallback = ipStreamerCfg->eventCallback;
    liveStreamingSettings.appCtx = ipStreamerCfg->appCtx;
    liveStreamingSettings.hlsSession = ipStreamerCfg->hlsSession;
    liveStreamingSettings.hlsSegmentSize = ipStreamerCfg->hlsSegmentSize;
    if (ipStreamerCfg->appHeader.valid) {
        if (ipStreamerCfg->appHeader.length > sizeof(liveStreamingSettings.appHeader.data)) {
            BDBG_ERR(("%s: ERROR: app header length (%d) is > data buffer in ip streamer (%d)", __FUNCTION__, ipStreamerCfg->appHeader.length, sizeof(liveStreamingSettings.appHeader.data)));
            return -1;
        }
        liveStreamingSettings.appHeader.valid = ipStreamerCfg->appHeader.valid;
        liveStreamingSettings.appHeader.length = ipStreamerCfg->appHeader.length;
        memcpy(liveStreamingSettings.appHeader.data, ipStreamerCfg->appHeader.data, ipStreamerCfg->appHeader.length);
    }

    BDBG_MSG(("event callback appCtx %p", liveStreamingSettings.appCtx));

    if (psi->psiValid)
    {
        liveStreamingSettings.psi = psi;
    }
    liveStreamingSettings.recpumpHandle = ipDst->recpumpHandle;
    liveStreamingSettings.dataReadyEvent = ipDst->dataReadyEvent;
    memcpy(liveStreamingSettings.fileName, ipStreamerCfg->fileName, sizeof(liveStreamingSettings.fileName)-1);
#ifdef B_HAS_DTCP_IP
    if (ipStreamerCfg->encryptionEnabled) {
        liveStreamingSettings.securitySettings.securityProtocol = B_PlaybackIpSecurityProtocol_DtcpIp;
        liveStreamingSettings.securitySettings.enableEncryption = true;
        liveStreamingSettings.securitySettings.initialSecurityContext = ipStreamerCtx->globalCtx->dtcpCtx;
        liveStreamingSettings.securitySettings.settings.dtcpIp.emiValue = ipStreamerCfg->emiValue;
    }
#endif
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (ipStreamerCfg->pvrEncryptionEnabled) {
        liveStreamingSettings.securitySettings.enableDecryption = true;
        liveStreamingSettings.securitySettings.dmaHandle = ipStreamerCtx->globalCtx->globalCfg.dmaHandle;
        liveStreamingSettings.securitySettings.pvrDecKeyHandle = ipStreamerCtx->pvrDecKeyHandle;
    }
#endif
    liveStreamingSettings.heapHandle = ipStreamerCtx->globalCtx->globalCfg.heapHandle;
#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled && ipStreamerCfg->hlsSession) {
        liveStreamingSettings.psiTables.patBufferPtr = (void *)ipStreamerCtx->transcoderDst->psi[0].pData;
        liveStreamingSettings.psiTables.patBufferSize = 188;
        liveStreamingSettings.psiTables.pmtBufferPtr = (void *)ipStreamerCtx->transcoderDst->psi[1].pData;
        liveStreamingSettings.psiTables.pmtBufferSize = 188;
        /* since we start the xcode before 1st segment request arrives, set this flag to indicate streaming is disabled */
        /* streaming is later enabled when 1st segment request comes. */
        liveStreamingSettings.streamingDisabled = true;
    }
#endif
    if ((ipDst->liveStreamingHandle = B_PlaybackIp_LiveStreamingOpen(&liveStreamingSettings)) == NULL) {
        BDBG_ERR(("%s: ERROR: Failed to open Network Streaming handle", __FUNCTION__));
        return -1;
    }

    if (ipStreamerCfg->enableAllpass) {
        rc = NEXUS_Recpump_AddPidChannel(ipDst->recpumpHandle, ipStreamerCtx->videoPidChannel, NULL);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
    }
    else if (ipStreamerCfg->pidListCount != 0) {
        /* client has provided the pid list, add pids channels using that */
        /* true for ASTRA's SAT/IP project */
        int i;
        NEXUS_PidChannelStatus pidChannelStatus;
        for (i=0; i< ipStreamerCfg->pidListCount; i++) {
            rc = NEXUS_Recpump_AddPidChannel(ipDst->recpumpHandle, ipStreamerCtx->pidChannelList[i], NULL);
            if (rc) {
                BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
                return -1;
            }
            NEXUS_PidChannel_GetStatus( ipStreamerCtx->pidChannelList[i], &pidChannelStatus );
            BDBG_MSG(("%s: Recpump adding PidChannel (%p); pid (%-3u); parserBand (%u); transportType (%u); contErrors (%u)",
                      __FUNCTION__, (void *)ipStreamerCtx->pidChannelList[i], pidChannelStatus.pid, pidChannelStatus.parserBand,
                      pidChannelStatus.transportType, pidChannelStatus.continuityCountErrors ));
        }
    }
    else {
        /* normal flow where pids were acquired using PSI discovery and we have full codec info about them */
        videoPidChannel = ipStreamerCtx->videoPidChannel;
        audioPidChannel = ipStreamerCtx->audioPidChannel;
        pcrPidChannel = ipStreamerCtx->pcrPidChannel;
        patPidChannel = ipStreamerCtx->patPidChannel;
        pmtPidChannel = ipStreamerCtx->pmtPidChannel;
        videoCodec = psi->videoCodec;
        videoPid = psi->videoPid;
#ifdef NEXUS_HAS_VIDEO_ENCODER
        if (ipStreamerCfg->transcodeEnabled) {
            /* transcoding session */
            BDBG_MSG(("Updated PidChannels for Transcoding case "));
            videoPidChannel = ipStreamerCtx->transcodeVideoPidChannel;
            audioPidChannel = ipStreamerCtx->transcodeAudioPidChannel;
            pcrPidChannel = ipStreamerCtx->transcodePcrPidChannel;
            patPidChannel = ipStreamerCtx->transcodePatPidChannel;
            pmtPidChannel = ipStreamerCtx->transcodePmtPidChannel;
            videoCodec = ipStreamerCtx->cfg.transcode.outVideoCodec;
            videoPid = ipStreamerCtx->cfg.transcode.outVideoPid;
        }
#endif
        /* configure the video pid for indexing */
        NEXUS_Recpump_GetDefaultAddPidChannelSettings(&pidSettings);
        pidSettings.pidType = NEXUS_PidType_eVideo;
        pidSettings.pidTypeSettings.video.index = false;
        pidSettings.pidTypeSettings.video.codec = videoCodec;
        pidSettings.pidTypeSettings.video.pid = videoPid;

        if (videoPidChannel) {
            NEXUS_RecpumpTpitFilter filter;
            rc = NEXUS_Recpump_AddPidChannel(ipDst->recpumpHandle, videoPidChannel, &pidSettings);
            if (rc) {
                BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
                return -1;
            }
            if (ipStreamerCfg->hlsSession) {
                /* enable TPIT filter for HLS Sessions so that IP library can segment the outgoing stream at the Random Access Indicator (RAI) boundary */
                BDBG_MSG(("%s: enable TPIT filter for HLS Sessions so that IP library can segment the outgoing stream at the Key packet boundary", __FUNCTION__));
                NEXUS_Recpump_GetDefaultTpitFilter(&filter);
                filter.config.mpeg.randomAccessIndicatorEnable = true;
                filter.config.mpeg.randomAccessIndicatorCompValue = true;
                filter.mpegMode = true;
                rc = NEXUS_Recpump_SetTpitFilter(ipDst->recpumpHandle, videoPidChannel, &filter);
                if (rc) { BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__)); return -1; }
            }
        }

        if (audioPidChannel) {
            rc = NEXUS_Recpump_AddPidChannel(ipDst->recpumpHandle, audioPidChannel, NULL);
            if (rc) {
                BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
                return -1;
            }
        }

        if (pcrPidChannel) {
            rc = NEXUS_Recpump_AddPidChannel(ipDst->recpumpHandle, pcrPidChannel, NULL);
            if (rc) {
                BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
                return -1;
            }
        }

#ifdef STREAMER_CABLECARD_SUPPORT
#ifdef MPODXSIX
        if (ipStreamerCtx->caPidChannel) {
            rc = NEXUS_Recpump_AddPidChannel(ipDst->recpumpHandle, ipStreamerCtx->caPidChannel, NULL);
            if (rc) {
                BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
                return -1;
            }
        }
#endif
#endif

        if (patPidChannel) {
            rc = NEXUS_Recpump_AddPidChannel(ipDst->recpumpHandle, patPidChannel, NULL);
            if (rc) {
                BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
                return -1;
            }
        }

#if 1
        if (pmtPidChannel) {
            rc = NEXUS_Recpump_AddPidChannel(ipDst->recpumpHandle, pmtPidChannel, NULL);
            if (rc) {
                BDBG_ERR(("NEXUS Error at %s:%d, returning..", __FILE__, __LINE__));
                return -1;
            }
        }
#endif
    }

    ipStreamerCtx->ipDst = ipDst;
    BDBG_MSG(("CTX %p: IP Dst %p is opened", (void *)ipStreamerCtx, (void *)ipDst));
    return 0;
}

void
closeNexusIpDstWithTimeshift(
    IpStreamerCtx *ipStreamerCtx
    )
{
    char *tmpPtr;

    if (!ipStreamerCtx->ipDst)
        return;
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->ipDstMutex);
    ipStreamerCtx->ipDst->inUse = false;
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ipDstMutex);
    if (!ipStreamerCtx->cfg.transcodeEnabled) {
        /* for transcoded sessions, pid channels need to be removed before stream mux is stopped otherwise, it frees the pid channels */
        NEXUS_Record_RemoveAllPidChannels(ipStreamerCtx->ipDst->recordHandle);
    }
    if (ipStreamerCtx->ipDst->recpumpHandle) {
        NEXUS_Recpump_RemoveAllPidChannels(ipStreamerCtx->ipDst->recpumpHandle);
    }
    if (ipStreamerCtx->cfg.usePlaybackForStreamingFiles) {
        if (ipStreamerCtx->fileSrc->timebase != NEXUS_Timebase_eInvalid)
            NEXUS_Timebase_Close(ipStreamerCtx->fileSrc->timebase);
        ipStreamerCtx->fileSrc->timebase = NEXUS_Timebase_eInvalid;
        if (ipStreamerCtx->ipDst->recpumpAllpassHandle) {
            NEXUS_Recpump_RemoveAllPidChannels(ipStreamerCtx->ipDst->recpumpAllpassHandle);
            NEXUS_Recpump_Close(ipStreamerCtx->ipDst->recpumpAllpassHandle);
        }
        if (ipStreamerCtx->fileSrc->playbackHandle)
            NEXUS_Playback_CloseAllPidChannels(ipStreamerCtx->fileSrc->playbackHandle);
        if (ipStreamerCtx->fileSrc->playpumpHandle)
            NEXUS_Playpump_Close(ipStreamerCtx->fileSrc->playpumpHandle);
        if (ipStreamerCtx->fileSrc->playbackHandle)
            NEXUS_Playback_Destroy(ipStreamerCtx->fileSrc->playbackHandle);
        ipStreamerCtx->fileSrc->playbackHandle = NULL;
        if (ipStreamerCtx->fileSrc->filePlayHandle)
            NEXUS_FilePlay_Close(ipStreamerCtx->fileSrc->filePlayHandle);
        BKNI_Free(ipStreamerCtx->fileSrc);
        ipStreamerCtx->fileSrc = NULL;
    }
    if (ipStreamerCtx->ipDst->liveStreamingHandle != NULL) {
        B_PlaybackIp_LiveStreamingClose(ipStreamerCtx->ipDst->liveStreamingHandle);
    }
    if (ipStreamerCtx->ipDst->fileRecordHandle != NULL) {
        NEXUS_FileRecord_Close(ipStreamerCtx->ipDst->fileRecordHandle);
    }
    if (ipStreamerCtx->ipDst->recordHandle != NULL)
        NEXUS_Record_Destroy(ipStreamerCtx->ipDst->recordHandle);
    if (ipStreamerCtx->ipDst->recpumpHandle)
        NEXUS_Recpump_Close(ipStreamerCtx->ipDst->recpumpHandle);

    /* now remove the timeshifted file buffer, nav & info files */
    if (remove(ipStreamerCtx->cfg.fileName) != 0) {
        BDBG_WRN(("Failed to remove timeshift file %s", ipStreamerCtx->cfg.fileName));
    }
    /* remove index file */
    if (remove(ipStreamerCtx->cfg.indexFileName) != 0) {
        BDBG_WRN(("Failed to remove timeshift file %s", ipStreamerCtx->cfg.indexFileName));
    }
    /* remove info file */
    tmpPtr = strstr(ipStreamerCtx->cfg.indexFileName, ".");
    if (tmpPtr)
        *tmpPtr = '\0';
    strncat(ipStreamerCtx->cfg.indexFileName, ".info", sizeof(ipStreamerCtx->cfg.indexFileName)-1);
    if (remove(ipStreamerCtx->cfg.indexFileName) != 0) {
        BDBG_WRN(("Failed to remove timeshift file %s", ipStreamerCtx->cfg.indexFileName));
    }
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (ipStreamerCtx->pvrEncKeyHandle)
        NEXUS_Security_FreeKeySlot(ipStreamerCtx->pvrEncKeyHandle);
    if (ipStreamerCtx->pvrDecKeyHandle && !ipStreamerCtx->cfg.pvrDecKeyHandle)
        NEXUS_Security_FreeKeySlot(ipStreamerCtx->pvrDecKeyHandle);
#endif
    BDBG_MSG(("%s: CTX %p: IP Dst %p is closed", __FUNCTION__, (void *)ipStreamerCtx, (void *)ipStreamerCtx->ipDst));
}

void
closeNexusIpDstWithoutTimeshift(
    IpStreamerCtx *ipStreamerCtx
    )
{
    if (!ipStreamerCtx->ipDst)
        return;
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->ipDstMutex);
    ipStreamerCtx->ipDst->inUse = false;
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->ipDstMutex);
    if (ipStreamerCtx->ipDst->recpumpHandle) {
        if (!ipStreamerCtx->cfg.transcodeEnabled)
            /* for transcoded sessions, pid channels are removed in stopping of transcoder pipe */
            NEXUS_Recpump_RemoveAllPidChannels(ipStreamerCtx->ipDst->recpumpHandle);
        NEXUS_Recpump_Close(ipStreamerCtx->ipDst->recpumpHandle);
    }
    if (ipStreamerCtx->ipDst->liveStreamingHandle != NULL) {
        B_PlaybackIp_LiveStreamingClose(ipStreamerCtx->ipDst->liveStreamingHandle);
    }
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (ipStreamerCtx->pvrEncKeyHandle)
        NEXUS_Security_FreeKeySlot(ipStreamerCtx->pvrEncKeyHandle);
    if (ipStreamerCtx->pvrDecKeyHandle && !ipStreamerCtx->cfg.pvrDecKeyHandle)
        NEXUS_Security_FreeKeySlot(ipStreamerCtx->pvrDecKeyHandle);
#endif
    BDBG_MSG(("%s: CTX %p: IP Dst %p is closed", __FUNCTION__, (void *)ipStreamerCtx, (void *)ipStreamerCtx->ipDst));
}

int
startNexusIpDst(
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    int count = 0;

    if (ipStreamerCtx->cfg.enableTimeshifting) {
        /* Even for IP Streaming, we record the live channel to Rave and stream it out from Rave buffers */
        if (NEXUS_Record_AddPlayback(ipStreamerCtx->ipDst->recordHandle, ipStreamerCtx->fileSrc->playbackHandle)) {
            BDBG_ERR(("%s: ERROR: CTX %p: NEXUS_Record_AddPlayback Failed", __FUNCTION__, (void *)ipStreamerCtx));
            return -1;
        }
        rc = NEXUS_Record_Start(ipStreamerCtx->ipDst->recordHandle, ipStreamerCtx->ipDst->fileRecordHandle);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
        if (ipStreamerCtx->cfg.usePlaybackForStreamingFiles) {
            rc = NEXUS_Recpump_Start(ipStreamerCtx->ipDst->recpumpAllpassHandle);
            if (rc) {
                BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
                return -1;
            }
            ipStreamerCtx->fileSrc->filePlayHandle = NEXUS_FifoPlay_Open(ipStreamerCtx->cfg.fileName, ipStreamerCtx->cfg.indexFileName, ipStreamerCtx->ipDst->fifoFileHandle);
            if (ipStreamerCtx->fileSrc->filePlayHandle == NULL) {
                BDBG_ERR(("%s: ERROR: NEXUS_FilePlay_OpenPosix Failed to open media file %s", __FUNCTION__, ipStreamerCtx->cfg.fileName));
                return -1;
            }
            BDBG_WRN(("TODO: Add a delay to avoid initial macroblocking: need to work on this bug: Sleeping 2 sec ............\n"));
            BKNI_Sleep(2000);
            while (count++ < 10) {
                rc = NEXUS_Playback_Start(ipStreamerCtx->fileSrc->playbackHandle, ipStreamerCtx->fileSrc->filePlayHandle, NULL);
                if (rc != NEXUS_SUCCESS) {
                    BDBG_ERR(("%s: ERROR: Failed to start Nexus Playback, retrying", __FUNCTION__));
                    BKNI_Sleep(100);
                    continue;
                }
                break;
            }
            if (rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: ERROR: Failed to start Nexus Playback", __FUNCTION__));
                return -1;
            }
        }
    }
    else {
        rc = NEXUS_Recpump_Start(ipStreamerCtx->ipDst->recpumpHandle);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
    }

    if (B_PlaybackIp_LiveStreamingStart(ipStreamerCtx->ipDst->liveStreamingHandle) != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: ERROR: Failed to start Live Streaming Session", __FUNCTION__));
        return -1;
    }
    ipStreamerCtx->ipStreamingInProgress = true;
    BDBG_MSG(("CTX %p: IP Dst %p is started", (void *)ipStreamerCtx, (void *)ipStreamerCtx->ipDst));
    return 0;
}

void
stopNexusIpDst(
    IpStreamerCtx *ipStreamerCtx
    )
{
    if (ipStreamerCtx->cfg.enableTimeshifting) {
        if (ipStreamerCtx->ipDst->recordHandle)
            NEXUS_Record_Stop(ipStreamerCtx->ipDst->recordHandle);
        if (ipStreamerCtx->cfg.usePlaybackForStreamingFiles) {
            if (ipStreamerCtx->ipDst->recpumpAllpassHandle) {
                NEXUS_StopCallbacks(ipStreamerCtx->ipDst->recpumpAllpassHandle);
                NEXUS_Recpump_Stop(ipStreamerCtx->ipDst->recpumpAllpassHandle);
            }
            if (ipStreamerCtx->fileSrc->playbackHandle) {
                NEXUS_StopCallbacks(ipStreamerCtx->fileSrc->playbackHandle);
                NEXUS_Playback_Stop(ipStreamerCtx->fileSrc->playbackHandle);
            }
            BDBG_MSG(("CTX %p: Streaming from a live Src using playback is stopped", (void *)ipStreamerCtx));
        }
        if (ipStreamerCtx->ipDst->liveStreamingHandle)
            B_PlaybackIp_LiveStreamingStop(ipStreamerCtx->ipDst->liveStreamingHandle);
    }
    else {
        if (ipStreamerCtx->ipDst->liveStreamingHandle)
            B_PlaybackIp_LiveStreamingStop(ipStreamerCtx->ipDst->liveStreamingHandle);
        if (ipStreamerCtx->overflow == false) {
            /* we only explictly stop recpump Callbacks if overflow flag is not set */
            /* otherwise, this function is invoked from the callback context and */
            /* calling StopCallbacks leads to a deadlock */
            if (ipStreamerCtx->ipDst->recpumpHandle)
                NEXUS_StopCallbacks(ipStreamerCtx->ipDst->recpumpHandle);
        }
        if (!ipStreamerCtx->cfg.transcodeEnabled && ipStreamerCtx->ipDst->recpumpHandle)
            NEXUS_Recpump_Stop(ipStreamerCtx->ipDst->recpumpHandle);
    }
    ipStreamerCtx->ipStreamingInProgress = false;
    BDBG_MSG(("CTX %p: IP dst %p is stopped", (void *)ipStreamerCtx, (void *)ipStreamerCtx->ipDst));
}
