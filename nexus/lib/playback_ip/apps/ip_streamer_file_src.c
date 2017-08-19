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
 *****************************************************************************/
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
#ifdef NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#include "nexus_parser_band.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_message.h"
#endif
#include "nexus_timebase.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#include "nexus_file_fifo.h"
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
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "ip_streamer_transcode.h"
extern void closeNexusTranscoderPipeNonRealTime( IpStreamerCtx *ipStreamerCtx, TranscoderDst *transcoderDst);
#endif

BDBG_MODULE(ip_streamer);
#if 0
#define BDBG_MSG_FLOW(x)  BDBG_WRN( x );
#else
#define BDBG_MSG_FLOW(x)
#endif

void
closeNexusFileSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    if (!ipStreamerCtx || !ipStreamerCtx->fileSrc)
        return;

    BDBG_MSG(("%s: CTX %p: closing file Src %p", BSTD_FUNCTION, (void *)ipStreamerCtx, (void *)ipStreamerCtx->fileSrc));
#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCtx->transcoderDst /*&& ipStreamerCtx->transcoderDst->refCount != 0*/) {
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        /* TODO: add a check for nonRealTime: for now assuming NRT is used for all file sources!!!! */
        closeNexusTranscoderPipe(ipStreamerCtx, ipStreamerCtx->transcoderDst);
        ipStreamerCtx->transcoderDst = NULL;
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
#endif
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (ipStreamerCtx->pvrDecKeyHandle)
        NEXUS_Security_FreeKeySlot(ipStreamerCtx->pvrDecKeyHandle);
#endif
    if (ipStreamerCtx->fileSrc->fileStreamingHandle)
        B_PlaybackIp_FileStreamingClose(ipStreamerCtx->fileSrc->fileStreamingHandle);
#ifdef NEXUS_HAS_PLAYBACK
    if (ipStreamerCtx->fileSrc->playpumpHandle)
        NEXUS_Playpump_Close(ipStreamerCtx->fileSrc->playpumpHandle);
    if (ipStreamerCtx->fileSrc->playbackHandle)
        NEXUS_Playback_Destroy(ipStreamerCtx->fileSrc->playbackHandle);
    if (ipStreamerCtx->fileSrc->filePlayHandle)
        NEXUS_FilePlay_Close(ipStreamerCtx->fileSrc->filePlayHandle);
    if (ipStreamerCtx->fileSrc->playbackStcChannel)
        NEXUS_StcChannel_Close(ipStreamerCtx->fileSrc->playbackStcChannel);
#endif
    BDBG_MSG(("CTX %p: File Src %p is closed", (void *)ipStreamerCtx, (void *)ipStreamerCtx->fileSrc));
    BKNI_Free(ipStreamerCtx->fileSrc);
    ipStreamerCtx->fileSrc = NULL;
}

extern char * buildInfoFileName(char *infoFilesDir, char *mediaFileName, unsigned programIndex);
int
openNexusFileSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    B_PlaybackIpFileStreamingOpenSettings fileStreamingSettings;
#ifndef DMS_CROSS_PLATFORMS
    NEXUS_ClientConfiguration clientConfig;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
#endif /* DMS_CROSS_PLATFORMS */
    if ((ipStreamerCtx->fileSrc = BKNI_Malloc(sizeof(FileSrc))) == NULL) { BDBG_ERR(("BKNI_Malloc Failure at %d", __LINE__)); return -1;}
    BKNI_Memset(ipStreamerCtx->fileSrc, 0, sizeof(FileSrc));
#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        BKNI_AcquireMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
        /* TODO: need to look into supporting streaming same file to multiple clients */
        /*
         * coverity[stack_use_local_overflow]
         * coverity[stack_use_overflow]
         */
        if ((ipStreamerCtx->transcoderDst = openNexusTranscoderPipe(ipStreamerCfg, ipStreamerCtx)) == NULL) {
            BDBG_ERR(("%s: Failed to open the transcoder pipe", BSTD_FUNCTION));
            BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
            goto error;
        }
        BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->transcoderDstMutex);
    }
#endif
#ifdef NEXUS_HAS_PLAYBACK
    if (ipStreamerCfg->usePlaybackForStreamingFiles) {
        char *tmpPtr;
        char *indexFileName;
        int indexFileLength;
        NEXUS_StcChannelSettings stcSettings;

        NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
        if(ipStreamerCtx->globalCtx->globalCfg.multiProcessEnv)
        {
            NEXUS_Platform_GetClientConfiguration(&clientConfig);
            playpumpOpenSettings.heap = clientConfig.heap[1]; /* playpump requires heap with eFull mapping */
        }
        BDBG_MSG(("%s: increasing the playpump FIFO from %d to %d", BSTD_FUNCTION, playpumpOpenSettings.fifoSize, playpumpOpenSettings.fifoSize*2));
        playpumpOpenSettings.fifoSize = playpumpOpenSettings.fifoSize*2;

        if ((ipStreamerCtx->fileSrc->playpumpHandle = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings)) == NULL) {
            BDBG_ERR(("%s: ERROR: Failed to Open a free Nexus Playpump idx", BSTD_FUNCTION));
            goto error;
        }
        if ((ipStreamerCtx->fileSrc->playbackHandle = NEXUS_Playback_Create()) == NULL) {
            BDBG_ERR(("%s: ERROR: Failed to create a Nexus Playback instance", BSTD_FUNCTION));
            goto error;
        }
        indexFileName = buildInfoFileName(ipStreamerCfg->mediaInfoFilesDir, ipStreamerCfg->fileName, 0);
        if (indexFileName == NULL) {
            BDBG_ERR(("%s: ERROR: Failed to build info file name %s for", BSTD_FUNCTION, ipStreamerCfg->fileName));
            goto error;
        }
        /* now replace the .info extension w/ .nav */
        tmpPtr = strstr(indexFileName, ".");
        indexFileLength = strlen(indexFileName);
        if (tmpPtr)
            *tmpPtr = '\0';
        strncat(indexFileName, ".nav", indexFileLength);
        BDBG_MSG(("index file name is %s", indexFileName));
        if ((ipStreamerCtx->fileSrc->filePlayHandle = NEXUS_FilePlay_OpenPosix(ipStreamerCfg->fileName, indexFileName)) == NULL) {
            BDBG_ERR(("%s: ERROR: NEXUS_FilePlay_OpenPosix Failed to open media file %s", BSTD_FUNCTION, ipStreamerCfg->fileName));
            goto error;
        }
        if (indexFileName)
            BKNI_Free(indexFileName);

        if (!ipStreamerCfg->transcodeEnabled) {
            /* transcode path already has this decode stc channel */
            NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
            stcSettings.timebase = NEXUS_Timebase_e0 + NEXUS_NUM_VIDEO_DECODERS;
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;
            if ((ipStreamerCtx->fileSrc->playbackStcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings)) == NULL) {
                BDBG_ERR(("%s: ERROR: NEXUS_StcChannel_Open Failed to open a free STC Channel %s", BSTD_FUNCTION, ipStreamerCfg->fileName));
                goto error;
            }
        }
    }
#endif

    /* call IP Applib to setup the streaming from Rave buffers */
    memset(&fileStreamingSettings, 0, sizeof(fileStreamingSettings));
    fileStreamingSettings.protocol = ipStreamerCfg->streamingCfg.streamingProtocol;
    fileStreamingSettings.beginFileOffset = ipStreamerCfg->beginFileOffset;
    fileStreamingSettings.endFileOffset = ipStreamerCfg->endFileOffset;
    fileStreamingSettings.playSpeed = ipStreamerCfg->playSpeed;
    fileStreamingSettings.beginTimeOffset = ipStreamerCfg->beginTimeOffset;
    fileStreamingSettings.endTimeOffset = ipStreamerCfg->endTimeOffset;
    fileStreamingSettings.mediaProbeOnly = ipStreamerCfg->mediaProbeOnly;
    if (ipStreamerCfg->appHeader.valid) {
        if (ipStreamerCfg->appHeader.length > sizeof(fileStreamingSettings.appHeader.data)) {
            BDBG_ERR(("%s: ERROR: app header length (%d) is > data buffer in ip streamer (%d)", BSTD_FUNCTION, ipStreamerCfg->appHeader.length, sizeof(fileStreamingSettings.appHeader.data)));
            goto error;
        }
        fileStreamingSettings.appHeader.valid = ipStreamerCfg->appHeader.valid;
        fileStreamingSettings.appHeader.length = ipStreamerCfg->appHeader.length;
        memcpy(fileStreamingSettings.appHeader.data, ipStreamerCfg->appHeader.data, ipStreamerCfg->appHeader.length);
    }
    fileStreamingSettings.autoRewind = ipStreamerCfg->autoRewind;
    BDBG_MSG(("settings: media probe only file: %d, %d", fileStreamingSettings.mediaProbeOnly, ipStreamerCfg->mediaProbeOnly));
    strncpy(fileStreamingSettings.fileName, ipStreamerCfg->fileName, sizeof(fileStreamingSettings.fileName)-1);
    strncpy(fileStreamingSettings.mediaInfoFilesDir, ipStreamerCfg->mediaInfoFilesDir, sizeof(fileStreamingSettings.mediaInfoFilesDir)-1);
    if (ipStreamerCfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eRtp || ipStreamerCfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eUdp) {
        strncpy(fileStreamingSettings.rtpUdpSettings.streamingIpAddress, ipStreamerCfg->streamingCfg.streamingIpAddress, sizeof(fileStreamingSettings.rtpUdpSettings.streamingIpAddress)-1);
        fileStreamingSettings.rtpUdpSettings.streamingPort = ipStreamerCfg->streamingCfg.streamingPort;
        fileStreamingSettings.rtpUdpSettings.interfaceName = ipStreamerCtx->globalCtx->globalCfg.interfaceName;
    }
    else {
        /* For non-UDP, non-RTP protocols, socket connection is already there as client requests for the session via HTTP/TCP */
        fileStreamingSettings.streamingFd = ipStreamerCfg->streamingFd;
    }
#ifdef B_HAS_DTCP_IP
    if (ipStreamerCfg->encryptionEnabled) {
        fileStreamingSettings.securitySettings.securityProtocol = B_PlaybackIpSecurityProtocol_DtcpIp;
        fileStreamingSettings.securitySettings.enableEncryption = true;
        fileStreamingSettings.securitySettings.initialSecurityContext = ipStreamerCtx->globalCtx->dtcpCtx;
        fileStreamingSettings.securitySettings.settings.dtcpIp.emiValue = ipStreamerCfg->emiValue;
        fileStreamingSettings.securitySettings.settings.dtcpIp.akeTimeoutInMs = 5000;
    }
#else
    BSTD_UNUSED(ipStreamerCfg);
#endif
#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && NEXUS_HAS_SECURITY
    if (!ipStreamerCfg->pvrDecryptionEnabled) {
        /* app hasn't enabled the PVR Decryption, check if it is set for testing purposes */
        ipStreamerCfg->pvrDecryptionEnabled = getEnvVariableValue("pvrDecEnabled", 0);
    }
    if (ipStreamerCfg->pvrDecryptionEnabled) {
        if (!ipStreamerCfg->pvrDecKeyHandle) {
            /* app didn't specify the pvrDecKeyHandle, create one */
            ipStreamerCtx->pvrDecKeyHandle = _createKeyHandle(NEXUS_SecurityOperation_eDecrypt);
            if (!ipStreamerCtx->pvrDecKeyHandle) {
                BDBG_ERR(("%s: Failed to allocate dec keyslot", BSTD_FUNCTION));
                goto error;
            }
            BDBG_MSG(("%s: PVR Decryption successfully setup", BSTD_FUNCTION));
        }
        fileStreamingSettings.securitySettings.enableDecryption = true;
        fileStreamingSettings.securitySettings.dmaHandle = ipStreamerCtx->globalCtx->globalCfg.dmaHandle;
        fileStreamingSettings.securitySettings.pvrDecKeyHandle = ipStreamerCtx->pvrDecKeyHandle;
    }
#else
    BSTD_UNUSED(getEnvVariableValue);
#endif
    fileStreamingSettings.heapHandle = ipStreamerCtx->globalCtx->globalCfg.heapHandle;

#ifdef DMS_CROSS_PLATFORMS
    if ( getenv("DLNA_CTT") && (strcmp(getenv("DLNA_CTT"),"y")==0) ) {
    fileStreamingSettings.disableIndexGeneration = false;
    }
    else /* disable index file generation for DMS_CROSS_PLATFORMS */
        fileStreamingSettings.disableIndexGeneration = true;
#else
    fileStreamingSettings.disableIndexGeneration = false;
#endif
    if ((ipStreamerCtx->fileSrc->fileStreamingHandle = B_PlaybackIp_FileStreamingOpen(&fileStreamingSettings)) == NULL) {
        BDBG_ERR(("%s: ERROR: Failed to open File Streaming handle", BSTD_FUNCTION));
        goto error;
    }

    BDBG_MSG(("CTX %p: File Src %p opened", (void *)ipStreamerCtx, (void *)ipStreamerCtx->fileSrc->fileStreamingHandle));
    return 0;

error:
    closeNexusFileSrc(ipStreamerCtx);
    return -1;
}

int
setupAndAcquirePsiInfoFileSrc(
    IpStreamerCtx *ipStreamerCtx,
    B_PlaybackIpPsiInfo *psiOut
    )
{
    memset(psiOut, 0, sizeof(B_PlaybackIpPsiInfo));
    if (B_PlaybackIp_FileStreamingGetMediaInfo(ipStreamerCtx->fileSrc->fileStreamingHandle, psiOut) != B_ERROR_SUCCESS) {
        BDBG_ERR(("Failed to obtain Media PSI info"));
        return -1;
    }
    return 0;
}

static void
endOfStreamCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    BDBG_MSG(("%s: CTX: %p: End of stream reached for file playback", BSTD_FUNCTION, context));
}

void
closeNexusFileSrcPidChannels(
    IpStreamerCtx *ipStreamerCtx
    )
{
    BDBG_MSG(("%s: Closing all pid ch", BSTD_FUNCTION));
#ifndef DMS_CROSS_PLATFORMS
    if (ipStreamerCtx->fileSrc->playbackHandle) {
        NEXUS_Playback_CloseAllPidChannels(ipStreamerCtx->fileSrc->playbackHandle);
    }
#endif /* DMS_CROSS_PLATFORMS */
#ifdef NEXUS_HAS_VIDEO_ENCODER
    /* these channels were created via setupNexusTranscoderPidChannels */
    if (ipStreamerCtx->transcodePatPidChannel) {
        NEXUS_Playpump_ClosePidChannel(ipStreamerCtx->transcoderDst->playpumpTranscodeSystemData, ipStreamerCtx->transcodePatPidChannel);
        ipStreamerCtx->transcodePatPidChannel = NULL;
    }
    if (ipStreamerCtx->transcodePmtPidChannel) {
        NEXUS_Playpump_ClosePidChannel(ipStreamerCtx->transcoderDst->playpumpTranscodeSystemData, ipStreamerCtx->transcodePmtPidChannel);
        ipStreamerCtx->transcodePmtPidChannel = NULL;
    }
    BDBG_MSG(("%s: Closed all pid ch", BSTD_FUNCTION));
#endif
}

int
openNexusFileSrcPidChannels(
    B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
#ifndef DMS_CROSS_PLATFORMS
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

    NEXUS_Playback_GetSettings(ipStreamerCtx->fileSrc->playbackHandle, &playbackSettings);
    playbackSettings.playpump = ipStreamerCtx->fileSrc->playpumpHandle;
    playbackSettings.playpumpSettings.transportType = psi->mpegType;
#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled)
    {
        if (ipStreamerCfg->transcode.nonRealTime)
            /* NRT mode won't use playback stc channel; set to NULL */
            playbackSettings.stcChannel = NULL;
        else
            playbackSettings.stcChannel = ipStreamerCtx->transcoderDst->videoStcChannel;
    }
    else
#endif
        playbackSettings.stcChannel = ipStreamerCtx->fileSrc->playbackStcChannel;

    if (ipStreamerCfg->enableAllpass)
        playbackSettings.playpumpSettings.allPass=true;
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
    playbackSettings.endOfStreamCallback.callback = endOfStreamCallback;
    playbackSettings.endOfStreamCallback.context = ipStreamerCtx;
    playbackSettings.errorCallback.callback = endOfStreamCallback;
    playbackSettings.errorCallback.context = ipStreamerCtx;
    if (ipStreamerCfg->beginTimeOffset)
        playbackSettings.startPaused = true;
    if (NEXUS_Playback_SetSettings(ipStreamerCtx->fileSrc->playbackHandle, &playbackSettings) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: NEXUS_Playback_SetSettings() Failed", BSTD_FUNCTION));
        return -1;
    }
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    if (ipStreamerCfg->enableAllpass) {
        NEXUS_Playpump_GetAllPassPidChannelIndex(playbackSettings.playpump, &playbackPidSettings.pidSettings.pidSettings.pidChannelIndex );
        ipStreamerCtx->videoPidChannel = NEXUS_Playback_OpenPidChannel(ipStreamerCtx->fileSrc->playbackHandle, 0x00, &playbackPidSettings);   /* PID is ignored in allPass mode */
        return 0;
    }

    if (psi->videoPid) {
#ifdef NEXUS_HAS_VIDEO_ENCODER
        if (ipStreamerCfg->transcodeEnabled) {
            if (!ipStreamerCfg->transcode.nonRealTime) {
                playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
                playbackPidSettings.pidTypeSettings.video.codec = psi->videoCodec;
                playbackPidSettings.pidTypeSettings.video.index = true;
                playbackPidSettings.pidTypeSettings.video.decoder = ipStreamerCtx->transcoderDst->videoDecoder;
            }
            ipStreamerCtx->transcoderDst->videoPidChannel = NEXUS_Playback_OpenPidChannel(ipStreamerCtx->fileSrc->playbackHandle, psi->videoPid, &playbackPidSettings);
        }
        else
        {
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            ipStreamerCtx->videoPidChannel = NEXUS_Playback_OpenPidChannel(ipStreamerCtx->fileSrc->playbackHandle, psi->videoPid, &playbackPidSettings);
        }
#else
        {
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            ipStreamerCtx->videoPidChannel = NEXUS_Playback_OpenPidChannel(ipStreamerCtx->fileSrc->playbackHandle, psi->videoPid, &playbackPidSettings);
        }
#endif
    }
    else
        ipStreamerCtx->videoPidChannel = NULL;

    ipStreamerCtx->audioPidChannel = NULL;
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    if (psi->audioPid) {
#ifdef NEXUS_HAS_VIDEO_ENCODER
        if (ipStreamerCfg->transcodeEnabled) {
            if (ipStreamerCtx->cfg.transcode.outAudio) {
                playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
                playbackPidSettings.pidTypeSettings.audio.primary = ipStreamerCtx->transcoderDst->audioDecoder;
                ipStreamerCtx->transcoderDst->audioPidChannel = NEXUS_Playback_OpenPidChannel(ipStreamerCtx->fileSrc->playbackHandle, psi->audioPid, &playbackPidSettings);
            }
        }
        else {
            /* audio enabled, but not a xcode session */
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            ipStreamerCtx->audioPidChannel = NEXUS_Playback_OpenPidChannel(ipStreamerCtx->fileSrc->playbackHandle, psi->audioPid, &playbackPidSettings);
        }
#else
        /* audio enabled and xcode is not compiled in */
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        ipStreamerCtx->audioPidChannel = NEXUS_Playback_OpenPidChannel(ipStreamerCtx->fileSrc->playbackHandle, psi->audioPid, &playbackPidSettings);
#endif
    }
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    if (psi->pcrPid != psi->videoPid) {
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
        ipStreamerCtx->pcrPidChannel = NEXUS_Playback_OpenPidChannel(ipStreamerCtx->fileSrc->playbackHandle, psi->pcrPid, &playbackPidSettings);
    }
    else
        ipStreamerCtx->pcrPidChannel = NULL;
    if (!ipStreamerCfg->transcodeEnabled) { /* only done for file streaming, pat/pmt are inserted via system data for transcoding session */
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
        ipStreamerCtx->patPidChannel = NEXUS_Playback_OpenPidChannel(ipStreamerCtx->fileSrc->playbackHandle, 0, &playbackPidSettings);
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        if (psi->pmtPid) {
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
            ipStreamerCtx->pmtPidChannel = NEXUS_Playback_OpenPidChannel(ipStreamerCtx->fileSrc->playbackHandle, psi->pmtPid, &playbackPidSettings);
        }
        BDBG_MSG(("%s: playback pid channels (v %p, a %p, pat %p, pmt %p) are created...", BSTD_FUNCTION,
                    (void *)ipStreamerCtx->videoPidChannel,
                    (void *)ipStreamerCtx->audioPidChannel,
                    (void *)ipStreamerCtx->patPidChannel,
                    (void *)ipStreamerCtx->pmtPidChannel));
    }
#ifdef NEXUS_HAS_VIDEO_ENCODER
    else {
        BDBG_MSG(("%s: playback pid channels (v %p, a %p) are created...", BSTD_FUNCTION, (void *)ipStreamerCtx->transcoderDst->videoPidChannel, (void *)ipStreamerCtx->transcoderDst->audioPidChannel));
    }
    if (ipStreamerCfg->transcodeEnabled) {
        if (setupNexusTranscoderPidChannels(psi, ipStreamerCfg, ipStreamerCtx) < 0) {
            BDBG_ERR(("%s: setupNexusTranscoderPidChannels() Failed", BSTD_FUNCTION));
            return -1;
        }
    }
#endif /* NEXUS_HAS_VIDEO_ENCODER */
#endif /* DMS_CROSS_PLATFORMS */
    return 0;
}

int
startNexusFileSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
#ifdef NEXUS_HAS_PLAYBACK
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackStartSettings startSettings;
#endif
    BDBG_MSG(("CTX %p: Start Streaming from File Src %p, !!!!! name %s", (void *)ipStreamerCtx, (void *)ipStreamerCtx->fileSrc, ipStreamerCtx->cfg.fileName));
#if 0
    if (strstr(ipStreamerCtx->cfg.fileName, "m3u8") == NULL)
        if (preparePatPmt(ipStreamerCtx) < 0)
            return -1;
#endif
#ifdef NEXUS_HAS_PLAYBACK
    if (ipStreamerCtx->fileSrc->playbackHandle) {
        NEXUS_Playback_GetSettings(ipStreamerCtx->fileSrc->playbackHandle, &playbackSettings);
#ifdef NEXUS_HAS_VIDEO_ENCODER
        if (ipStreamerCtx->cfg.transcodeEnabled && !ipStreamerCtx->cfg.transcode.nonRealTime)
            playbackSettings.stcChannel = ipStreamerCtx->transcoderDst->videoStcChannel;
        else
#endif
        playbackSettings.stcChannel = ipStreamerCtx->fileSrc->playbackStcChannel;
        if (NEXUS_Playback_SetSettings(ipStreamerCtx->fileSrc->playbackHandle, &playbackSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: NEXUS_Playback_SetSettings() Failed", BSTD_FUNCTION));
            return -1;
        }
        NEXUS_Playback_GetDefaultStartSettings(&startSettings);
        startSettings.mpeg2TsIndexType = NEXUS_PlaybackMpeg2TsIndexType_eNav;
        if (NEXUS_Playback_Start(ipStreamerCtx->fileSrc->playbackHandle, ipStreamerCtx->fileSrc->filePlayHandle, &startSettings) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: ERROR: Failed to start File Streaming handle", BSTD_FUNCTION));
            return -1;
        }
        if (ipStreamerCtx->cfg.beginTimeOffset) {
            int seekOffsetSec = (int)ipStreamerCtx->cfg.beginTimeOffset;
            BDBG_WRN(("Seeking Playback by %d msec\n", seekOffsetSec * 1000));
            if (NEXUS_Playback_Seek(ipStreamerCtx->fileSrc->playbackHandle, seekOffsetSec*1000) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: ERROR: Nexus Playback Seek failed to seek to %d msec offset", BSTD_FUNCTION, seekOffsetSec));
                return -1;
            }
            BDBG_WRN(("Seeked Playback by %d msec\n", seekOffsetSec*1000));
            if (NEXUS_Playback_Play(ipStreamerCtx->fileSrc->playbackHandle) != NEXUS_SUCCESS) {
                BDBG_ERR(("%s: ERROR: Nexus Playback Play failed after seeking to %f msec offset", BSTD_FUNCTION, ipStreamerCtx->cfg.beginTimeOffset));
                return -1;
            }
        }
        return 0;
    }
#endif
    if (B_PlaybackIp_FileStreamingStart(ipStreamerCtx->fileSrc->fileStreamingHandle) != B_ERROR_SUCCESS) {
        BDBG_ERR(("%s: ERROR: Failed to start File Streaming handle", BSTD_FUNCTION));
        return -1;
    }
    ipStreamerCtx->fileStreamingInProgress = true;
    return 0;
}

void
stopNexusFileSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
#ifdef NEXUS_HAS_PLAYBACK
    if (ipStreamerCtx->fileSrc->playbackHandle) {
        NEXUS_Playback_Stop(ipStreamerCtx->fileSrc->playbackHandle);
        BDBG_MSG(("CTX %p: Streaming from File Src %p using playback is stopped", (void *)ipStreamerCtx, (void *)ipStreamerCtx->fileSrc));
        return;
    }
#endif
    B_PlaybackIp_FileStreamingStop(ipStreamerCtx->fileSrc->fileStreamingHandle);
    ipStreamerCtx->fileStreamingInProgress = false;
    BDBG_MSG(("CTX %p: Streaming from File Src %p stopped", (void *)ipStreamerCtx, (void *)ipStreamerCtx->fileSrc));
}
