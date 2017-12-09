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
 * Module Description: test program to demonstrate IP (HTTP/RTP/UDP) streaming functionality
 * of Live QAM, IP or Playback data from Disk.
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

#include "ip_streamer_lib.h"
#include "ip_streamer.h"
#include "ip_streamer_ip_src.h"
#include "ip_streamer_qam_src.h"
#include "ip_streamer_sat_src.h"
#include "ip_streamer_ofdm_src.h"
#include "ip_streamer_vsb_src.h"
#include "ip_streamer_file_src.h"
#include "ip_streamer_streamer_src.h"
#include "ip_streamer_ip_dst.h"
#include "ip_streamer_rec_dst.h"
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "ip_streamer_hdmi_src.h"
#include "ip_streamer_transcode.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_mux_output.h"
#endif
#include "ip_http.h"
#include "b_os_lib.h"
#ifndef DMS_CROSS_PLATFORMS
#include "ip_psi.h"
#endif

#ifdef STREAMER_CABLECARD_SUPPORT
#include "ip_strm_cablecard.h"
#endif

#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
#include "nexus_dma.h"
#endif
#if (defined(NEXUS_HAS_SECURITY) && (NEXUS_SECURITY_API_VERSION==1))
#include "nexus_security.h"
#endif

BDBG_MODULE(ip_streamer);

/*
 * Test App to demonstrate streaming Live content (either coming over RTP/UDP channels or QAM) to network clients
 */

#if 0
#define BDBG_MSG_FLOW(x)  BDBG_WRN( x );
#else
#define BDBG_MSG_FLOW(x)
#endif

void
setDefaultPsiSettings(B_PlaybackIpPsiInfo *psi)
{
    /* Note: the PSI info needs to be correctly set for the live channel to be streamed out */
    memset(psi, 0, sizeof(*psi));
    psi->psiValid = true;
    psi->videoCodec = NEXUS_VideoCodec_eMpeg2;
    psi->videoPid = 0x31;
    psi->audioCodec = NEXUS_AudioCodec_eAc3;
    psi->audioPid = 0x34;
    psi->pcrPid = 0x31;
    psi->pmtPid = 0x1;
    psi->mpegType = NEXUS_TransportType_eTs;
    BDBG_MSG(("Setting Default PSI Settings: vpid %d, vcodec %d, apid %d, acodec %d, container %d", psi->videoPid, psi->videoCodec, psi->audioPid, psi->audioCodec, psi->mpegType));
}

int
getEnvVariableValue(char *pName, unsigned long defaultValue)
{
    int value;
    char *pValue;
    char invalid = '\0';
    char *pInvalid = &invalid;

    value = defaultValue;
    pValue = getenv(pName);
    if (pValue != NULL) {
        value = strtoul(pValue, &pInvalid, 0);
        if (*pInvalid != '\0')
        {
            value = defaultValue;
        }
    }
    BDBG_MSG(("%s: %s = %d", BSTD_FUNCTION, pName, value));
    return value;
}

#ifndef DMS_CROSS_PLATFORMS
int
initNexusPlatform(
    IpStreamerGlobalCfg *ipStreamerGlobalCfg
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;

    /* Bring up all modules for a platform in a default configuration for this platform */
    if (!ipStreamerGlobalCfg->slaveMode) {
        NEXUS_PlatformSettings *platformSettings;

        if ((platformSettings = malloc(sizeof(NEXUS_PlatformSettings))) == NULL) { BDBG_ERR(("malloc Failure at %d", __LINE__)); return -1;}
        NEXUS_Platform_GetDefaultSettings(platformSettings);
        platformSettings->openFrontend = !ipStreamerGlobalCfg->disableFrontend;
#ifdef NEXUS_HAS_VIDEO_ENCODER
        platformSettings->audioModuleSettings.maxAudioDspTasks = platformSettings->audioModuleSettings.maxAudioDspTasks + NEXUS_NUM_VIDEO_ENCODERS;
#endif
        rc = NEXUS_Platform_Init(platformSettings);
        free(platformSettings);
        if (rc) {
            BDBG_ERR(("NEXUS_Platfrom_Init() failed, err %d", rc));
            return -1;
        }
    }
    else {
        /* true when some other app is runing that has already initialized the Nexus platform */
        NEXUS_Error rc;
        if ((rc = NEXUS_Platform_Join()) != NEXUS_SUCCESS) {
            BDBG_ERR(("NEXUS_Platfrom_Join() failed, err %d", rc));
            return -1;
        }
    }
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    if (!ipStreamerGlobalCfg->dmaHandle) {
        if ((ipStreamerGlobalCfg->dmaHandle = NEXUS_Dma_Open(0, NULL)) == NULL) { BDBG_ERR(("ERROR: NEXUS_Dma_Open failed")); return -1; }
        ipStreamerGlobalCfg->freeDmaHandle = true;
        BDBG_MSG(("%s: DMA Ctx is opened", BSTD_FUNCTION));
    }
#endif

    BDBG_MSG(("%s: NEXUS Platfrom is initialized", BSTD_FUNCTION));
    return 0;
}

#endif /* DMS_CROSS_PLATFORMS */

int
initLibs(void)
{
    B_Os_Init();

#ifdef STREAMER_CABLECARD_SUPPORT
    {
        cablecard_setting mpodSetting;

    mpodSetting.oob_mode = NEXUS_FrontendOutOfBandMode_ePod_Dvs178Qpsk;
    mpodSetting.us_mode = NEXUS_FrontendUpstreamMode_ePodDvs178;
    BDBG_ASSERT(cablecard_open(&mpodSetting) != NULL);
    cablecard_go();
    }
#endif
    return 0;
}

void
unInitLibs(void)
{
#ifdef STREAMER_CABLECARD_SUPPORT
    cablecard_close();
#endif
    B_Os_Uninit();
}

#if (NEXUS_HAS_DMA || NEXUS_HAS_XPT_DMA) && (NEXUS_HAS_SECURITY && NEXUS_SECURITY_API_VERSION==1)
NEXUS_KeySlotHandle
_createKeyHandle(NEXUS_SecurityOperation operationType)
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_SecurityAlgorithmSettings algConfig;
    NEXUS_SecurityClearKey key;
    static const uint8_t keys[16] = { 0x4e, 0x61, 0xbc, 0x00, 0xa8, 0x7f, 0x39, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
#ifdef ENABLE_CBC_MODE
    /* note: as off 09/22/2011, AES-CBC mode doesn't seem to be working on 7425 */
    unsigned char ivData[16]={0};
#endif
    NEXUS_KeySlotHandle keyHandle;

    NEXUS_Security_GetDefaultKeySlotSettings(&keySettings);
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    keyHandle = NEXUS_Security_AllocateKeySlot(&keySettings);
    if (!keyHandle) {
        BDBG_ERR(("%s: Failed to allocate enc keyslot", BSTD_FUNCTION));
        return NULL;
    }
    NEXUS_Security_GetDefaultAlgorithmSettings(&algConfig);
    algConfig.algorithm = NEXUS_SecurityAlgorithm_eAes;
    algConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
    algConfig.terminationMode = NEXUS_SecurityTerminationMode_eClear;
    algConfig.operation = operationType;
    if ( (rc = NEXUS_Security_ConfigAlgorithm(keyHandle, &algConfig)) != NEXUS_SUCCESS) {
        BDBG_ERR(("%s: Failed to Configure Security Algorithms, rc %d", BSTD_FUNCTION, rc));
        return NULL;
    }

    /* Load Encryption key */
    NEXUS_Security_GetDefaultClearKey(&key);
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    key.keySize = sizeof(keys);
    BKNI_Memcpy(key.keyData, keys, sizeof(keys));
    if (NEXUS_Security_LoadClearKey(keyHandle, &key) != 0) {
        BDBG_ERR(("%s: Load encryption key failed", BSTD_FUNCTION));
        return NULL;
    }
#ifdef ENABLE_CBC_MODE
    /* Load IV: use key as initial IV */
    NEXUS_Security_GetDefaultClearKey(&key);
    key.keyEntryType = NEXUS_SecurityKeyType_eIv;
    key.keySize = sizeof(ivData);
    BKNI_Memcpy(key.keyData, ivData, sizeof(ivData));
    if (NEXUS_Security_LoadClearKey(keyHandle, &key) != 0) {
        BDBG_ERR(("%s: Load encryption IV failed", BSTD_FUNCTION));
        return NULL;
    }
#endif
    BDBG_MSG(("%s: key handle %p successfully created", BSTD_FUNCTION, (void *)keyHandle));
    return keyHandle;
}
#endif

#ifndef DMS_CROSS_PLATFORMS
void
closeNexusLivePidChannels(
    IpStreamerCtx *ipStreamerCtx
    )
{
    if (ipStreamerCtx->cfg.enableAllpass) {
        if (ipStreamerCtx->videoPidChannel)
            NEXUS_PidChannel_Close(ipStreamerCtx->videoPidChannel);
    }
    else if (ipStreamerCtx->cfg.pidListCount != 0) {
        int i;
        /* client has provided the pid list, close those pid channels */
        /* true for ASTRA's SAT/IP project */
        for (i=0; i< ipStreamerCtx->cfg.pidListCount; i++) {
            if (ipStreamerCtx->pidChannelList[i])
                NEXUS_PidChannel_Close(ipStreamerCtx->pidChannelList[i]);
        }
    }
    else {
        if (ipStreamerCtx->pcrPidChannel)
            NEXUS_PidChannel_Close(ipStreamerCtx->pcrPidChannel);
        if (ipStreamerCtx->videoPidChannel)
            NEXUS_PidChannel_Close(ipStreamerCtx->videoPidChannel);
        if (ipStreamerCtx->audioPidChannel)
            NEXUS_PidChannel_Close(ipStreamerCtx->audioPidChannel);
        if (ipStreamerCtx->patPidChannel)
            NEXUS_PidChannel_Close(ipStreamerCtx->patPidChannel);
        if (ipStreamerCtx->pmtPidChannel)
            NEXUS_PidChannel_Close(ipStreamerCtx->pmtPidChannel);
    }
}

#ifndef DMS_CROSS_PLATFORMS
void
closeNexusPidChannels(
    IpStreamerCtx *ipStreamerCtx
    )
{
    BDBG_MSG(("%s: CTX %p: PID Channels are being closed", BSTD_FUNCTION, (void *)ipStreamerCtx));
    if (ipStreamerCtx->ipSrc) {
        closeNexusIpSrcPidChannels(ipStreamerCtx);
    }
    else if (ipStreamerCtx->fileSrc) {
        closeNexusFileSrcPidChannels(ipStreamerCtx);
    }
#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
    else if (ipStreamerCtx->hdmiSrc) {
        closeNexusTranscoderPidChannels(ipStreamerCtx);
    }
#endif
    else {
#ifdef STREAMER_CABLECARD_SUPPORT
        closeNexusPidChannelsCableCard(ipStreamerCtx);
#endif
        closeNexusLivePidChannels(ipStreamerCtx);
#ifdef NEXUS_HAS_VIDEO_ENCODER
        if (ipStreamerCtx->transcoderDst) {
            closeNexusTranscoderPidChannels(ipStreamerCtx);
        }
#endif
    }
    BDBG_MSG(("CTX %p: PID Channels are closed", (void *)ipStreamerCtx));
}
#endif /* DMS_CROSS_PLATFORMS */

int
openNexusLivePidChannels(
    B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    BSTD_UNUSED(ipStreamerCfg);

    BDBG_MSG(("%s: pidListCount (%u); enableAllpass %u; filename (%s)", BSTD_FUNCTION, ipStreamerCfg->pidListCount, ipStreamerCfg->enableAllpass, ipStreamerCfg->fileName ));
    if (ipStreamerCfg->pidListCount > 0) {
        char pidlist[128] = "pidList: ";
        char sPid[6];
        for (int idx=0; idx<ipStreamerCfg->pidListCount; idx++) {
            snprintf(sPid, sizeof(sPid)-1, "%u,", ipStreamerCfg->pidList[idx] );
            strncat(pidlist, sPid, sizeof(pidlist)-1 );
        }
        BDBG_MSG(("%s: %s", BSTD_FUNCTION, pidlist ));
    }
    if (ipStreamerCfg->enableAllpass) {
        NEXUS_PidChannelSettings pidCfg0;
        NEXUS_PidChannel_GetDefaultSettings(&pidCfg0) ;
        NEXUS_ParserBand_GetAllPassPidChannelIndex(ipStreamerCtx->parserBandPtr->parserBand, &pidCfg0.pidChannelIndex);
        /* The pid number is redundant here. */
        if ((ipStreamerCtx->videoPidChannel = NEXUS_PidChannel_Open(ipStreamerCtx->parserBandPtr->parserBand, 0x0, &pidCfg0)) == NULL) {
            BDBG_ERR(("%s: Failed to create pid channel for all pass case", BSTD_FUNCTION));
            return -1;
        }
    }
    else if (ipStreamerCfg->pidListCount != 0) {
        int i;
        /* client has provided the pid list, create pids channels using that */
        /* true for ASTRA's SAT/IP project */
        for (i=0; i< ipStreamerCfg->pidListCount; i++) {
            if ((ipStreamerCtx->pidChannelList[i] = NEXUS_PidChannel_Open(ipStreamerCtx->parserBandPtr->parserBand, ipStreamerCfg->pidList[i], NULL)) == NULL) {
                BDBG_ERR(("%s: Failed to create pid channel, i %d, pid 0x%x", BSTD_FUNCTION, i, ipStreamerCfg->pidList[i]));
                return -1;
            }

            /* CAD to force psi->videoPid to be valid */
            if (ipStreamerCfg->pidList[i] > 0 && psi->videoPid == 0) {
                psi->videoPid = ipStreamerCfg->pidList[i];
            } else if (ipStreamerCfg->pidList[i] > 0 && psi->audioPid == 0) {
                    psi->audioPid = ipStreamerCfg->pidList[i];
            }
            BDBG_MSG(("%s: NEXUS_PidChannel_Open pidList[%d] 0x%02x; chan %p", BSTD_FUNCTION, i, ipStreamerCfg->pidList[i], (void *)ipStreamerCtx->pidChannelList[i] ));
        }
    }
    else {
        /* allocate pid channels for non-xcode live case where client didn't provide the pid list */
        if (psi->videoPid) {
            ipStreamerCtx->videoPidChannel = NEXUS_PidChannel_Open(ipStreamerCtx->parserBandPtr->parserBand, psi->videoPid, NULL);
            if (ipStreamerCtx->videoPidChannel == NULL)
                return -1;
        }
        else
            ipStreamerCtx->videoPidChannel = NULL;

        if (psi->audioPid) {
            ipStreamerCtx->audioPidChannel = NEXUS_PidChannel_Open(ipStreamerCtx->parserBandPtr->parserBand, psi->audioPid, NULL);
            if (ipStreamerCtx->audioPidChannel == NULL)
                return -1;
        }
        else
            ipStreamerCtx->audioPidChannel = NULL;

        if (psi->pcrPid != psi->videoPid) {
            ipStreamerCtx->pcrPidChannel = NEXUS_PidChannel_Open(ipStreamerCtx->parserBandPtr->parserBand, psi->pcrPid, NULL);
            if (ipStreamerCtx->pcrPidChannel == NULL)
                return -1;
        }
        else
            ipStreamerCtx->pcrPidChannel = NULL;

        ipStreamerCtx->patPidChannel = NEXUS_PidChannel_Open(ipStreamerCtx->parserBandPtr->parserBand, 0 /* PAT is always PID 0 */, NULL);
        if (ipStreamerCtx->patPidChannel == NULL)
            return -1;

        ipStreamerCtx->pmtPidChannel = NEXUS_PidChannel_Open(ipStreamerCtx->parserBandPtr->parserBand, psi->pmtPid, NULL);
        if (ipStreamerCtx->pmtPidChannel == NULL)
            return -1;

    }
    return 0;
}
#endif /* DMS_CROSS_PLATFORMS */


int
openNexusPidChannels(
    B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    switch (ipStreamerCfg->srcType) {
#ifndef DMS_CROSS_PLATFORMS
#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
    case IpStreamerSrc_eHdmi:
        if (setupNexusTranscoderPidChannels(psi, ipStreamerCfg, ipStreamerCtx) < 0) {
            BDBG_ERR(("%s Failed", BSTD_FUNCTION));
            return -1;
        }
        break;
#endif /* NEXUS_HAS_VIDEO_ENCODER */
    case IpStreamerSrc_eIp:
        if (openNexusIpSrcPidChannels(psi, ipStreamerCfg, ipStreamerCtx) < 0) {
            BDBG_ERR(("%s: %s() Failed", BSTD_FUNCTION, "openNexusIpSrcPidChannels"));
            return -1;
        }
        break;
    case IpStreamerSrc_eQam:
    case IpStreamerSrc_eSat:
    case IpStreamerSrc_eVsb:
    case IpStreamerSrc_eStreamer:
    case IpStreamerSrc_eOfdm:
#ifdef NEXUS_HAS_VIDEO_ENCODER
        if (ipStreamerCtx->transcoderDst) {
            if (openNexusTranscoderPidChannels(psi, ipStreamerCfg, ipStreamerCtx) < 0) {
                BDBG_ERR(("%s: %s() Failed", BSTD_FUNCTION, "openNexusIpTranscoderChannels"));
                return -1;
            }
        }
        else
#endif /* NEXUS_HAS_VIDEO_ENCODER */
        {
            if (openNexusLivePidChannels(psi, ipStreamerCfg, ipStreamerCtx) < 0) {
                BDBG_ERR(("%s: %s() Failed", BSTD_FUNCTION, "openNexusLivePidChannels"));
                return -1;
            }
        }

#ifdef STREAMER_CABLECARD_SUPPORT
        if (openNexusPidChannelsCableCard(psi, ipStreamerCfg, ipStreamerCtx) < 0) {
            BDBG_ERR(("%s: %s() Failed", BSTD_FUNCTION, "openNexusPidChannelsCableCard"));
            return -1;
        }
#endif
        break;
#endif /* DMS_CROSS_PLATFORMS */
    case IpStreamerSrc_eFile:
        if (ipStreamerCfg->usePlaybackForStreamingFiles) {
            if (openNexusFileSrcPidChannels(psi, ipStreamerCfg, ipStreamerCtx) < 0) {
                BDBG_ERR(("%s: %s() Failed", BSTD_FUNCTION, "openNexusFileSrcPidChannels"));
                return -1;
            }
        }
        else {
            BDBG_MSG(("%s: no PID channel work is needed for direct file streaming (w/o using playback)", BSTD_FUNCTION));
            return 0;
        }
        break;
    default:
        BDBG_ERR(("src type %d is not yet supported", ipStreamerCfg->srcType));
        return -1;
    }
#ifndef DMS_CROSS_PLATFORMS
    if (ipStreamerCfg->srcType != IpStreamerSrc_eHdmi && !ipStreamerCfg->transcodeEnabled && !ipStreamerCfg->pidListCount) {
        if ((psi->videoPid && !ipStreamerCtx->videoPidChannel) ||
            (psi->audioPid && !ipStreamerCtx->audioPidChannel) ||
            ((psi->pcrPid != psi->videoPid) && !ipStreamerCtx->pcrPidChannel)) {
            BDBG_ERR(("Failed to open PID channels, vpid ch %p, apid ch %p, pcr pid ch %p, src type %d",
                (void *)ipStreamerCtx->videoPidChannel, (void *)ipStreamerCtx->audioPidChannel, (void *)ipStreamerCtx->pcrPidChannel, ipStreamerCfg->srcType));
            BDBG_ERR(("Failed to open PID channels, vpid %d, apid %d, pcr pid %d ",
                psi->videoPid, psi->audioPid, psi->pcrPid));
            return -1;
        }
    }
#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCfg->transcodeEnabled) {
        if ((ipStreamerCfg->transcode.outVideo && !ipStreamerCtx->transcodeVideoPidChannel) ||
            (ipStreamerCfg->transcode.outAudio && psi->audioPid && !ipStreamerCtx->transcodeAudioPidChannel)
           ) {
            BDBG_ERR(("Failed to open Nexus Playpump PID channels for transcoding session, pid channel: vpid %p, apid %p, pcr %p, pat %p, pmt %p",
                    (void *)ipStreamerCtx->transcodeVideoPidChannel, (void *)ipStreamerCtx->transcodeAudioPidChannel, (void *)ipStreamerCtx->transcodePcrPidChannel, (void *)ipStreamerCtx->transcodePatPidChannel, (void *)ipStreamerCtx->transcodePmtPidChannel));

            return -1;
        }
    }
#endif
    BDBG_MSG(("CTX %p: PID Channels are opened (vpid 0x%x, pid ch %p, pcr pid ch %p, apid 0x%x, audio pid ch %p, pmt pid %d)",
              (void *)ipStreamerCtx,
              psi->videoPid,
              (void *)ipStreamerCtx->videoPidChannel,
              (void *)ipStreamerCtx->pcrPidChannel,
              psi->audioPid,
              (void *)ipStreamerCtx->audioPidChannel,
              psi->pmtPid
              ));

#endif /* DMS_CROSS_PLATFORMS */
    return 0;
}

#ifndef DMS_CROSS_PLATFORMS
int
startNexusDst(
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;

    if (ipStreamerCtx->ipDst) {
        rc = startNexusIpDst(ipStreamerCtx);
        if (rc) {BDBG_ERR(("%s: failed at at %d, returning...", BSTD_FUNCTION, __LINE__)); return -1;}
    }

    if (ipStreamerCtx->recDst) {
        rc = startNexusRecDst(ipStreamerCtx);
        if (rc) {BDBG_ERR(("%s: failed at at %d, returning...", BSTD_FUNCTION, __LINE__)); return -1;}
    }
    return 0;
}

void
stopNexusDst(
    IpStreamerCtx *ipStreamerCtx
    )
{
    if (ipStreamerCtx->ipDst) {
        stopNexusIpDst(ipStreamerCtx);
    }

    if (ipStreamerCtx->recDst) {
        stopNexusRecDst(ipStreamerCtx);
    }
}
#endif /* DMS_CROSS_PLATFORMS */

int
startNexusSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    switch (ipStreamerCtx->srcType) {
#ifndef DMS_CROSS_PLATFORMS
    case IpStreamerSrc_eIp:
        rc = startNexusIpSrc(ipStreamerCtx);
        break;
#endif /* DMS_CROSS_PLATFORMS */
#if NEXUS_HAS_FRONTEND
    case IpStreamerSrc_eQam:
        rc = startNexusQamSrc(ipStreamerCtx);
        break;
    case IpStreamerSrc_eSat:
        rc = startNexusSatSrc(ipStreamerCtx);
        break;
    case IpStreamerSrc_eOfdm:
        rc = startNexusOfdmSrc(ipStreamerCtx);
        break;
    case IpStreamerSrc_eVsb:
        rc = startNexusVsbSrc(ipStreamerCtx);
        break;
#endif /* NEXUS_HAS_FRONTEND */
    case IpStreamerSrc_eFile:
        rc = startNexusFileSrc(ipStreamerCtx);
        break;
    case IpStreamerSrc_eHdmi:
    case IpStreamerSrc_eStreamer:
        /* nothing to start as HDMI input is already connected to encoder during open call */
        rc = NEXUS_SUCCESS;
        break;
    default:
        return -1;
    }
    return rc;
}

void
stopNexusSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
    switch (ipStreamerCtx->srcType) {
#ifndef DMS_CROSS_PLATFORMS
    case IpStreamerSrc_eIp:
        stopNexusIpSrc(ipStreamerCtx);
        break;
#endif /* DMS_CROSS_PLATFORMS */
#if NEXUS_HAS_FRONTEND
    case IpStreamerSrc_eQam:
        stopNexusQamSrc(ipStreamerCtx);
        break;
    case IpStreamerSrc_eSat:
        stopNexusSatSrc(ipStreamerCtx);
        break;
    case IpStreamerSrc_eOfdm:
        stopNexusOfdmSrc(ipStreamerCtx);
        break;
    case IpStreamerSrc_eVsb:
        stopNexusVsbSrc(ipStreamerCtx);
        break;
#endif /* NEXUS_HAS_FRONTEND */
    case IpStreamerSrc_eFile:
        stopNexusFileSrc(ipStreamerCtx);
        break;
    case IpStreamerSrc_eHdmi:
    case IpStreamerSrc_eStreamer:
        /* nothing to stop */
        break;
    default:
        return;
    }
}

void getNexusSrcDstStatus(
    IpStreamerCtx *ipStreamerCtx,
    uint64_t *bytesReceived
    )
{
    B_PlaybackIpFileStreamingStatus fileSrcStatus;
#if NEXUS_HAS_RECORD
    NEXUS_RecordStatus recDstStatus;
#endif
#ifndef DMS_CROSS_PLATFORMS
    /* prefer bytes being received by the IP dst over local recording */
    if (ipStreamerCtx->ipDst) {
        if (ipStreamerCtx->cfg.enableTimeshifting) {
            NEXUS_RecordStatus ipDstStatus;
            NEXUS_Record_GetStatus(ipStreamerCtx->ipDst->recordHandle, &ipDstStatus);
            *bytesReceived = ipDstStatus.recpumpStatus.data.bytesRecorded;
        }
        else {
            NEXUS_RecpumpStatus ipDstStatus;
            NEXUS_Recpump_GetStatus(ipStreamerCtx->ipDst->recpumpHandle, &ipDstStatus);
            *bytesReceived = ipDstStatus.data.bytesRecorded;
        }
    }
#endif /* DMS_CROSS_PLATFORMS */
#if NEXUS_HAS_RECORD
    if (ipStreamerCtx->recDst) {
        NEXUS_Record_GetStatus(ipStreamerCtx->recDst->recordHandle, &recDstStatus);
        *bytesReceived = recDstStatus.recpumpStatus.data.bytesRecorded;
    }
#endif /* NEXUS_HAS_RECORD */

    if (ipStreamerCtx->fileStreamingInProgress) {
        B_PlaybackIp_FileStreamingGetStatus(ipStreamerCtx->fileSrc->fileStreamingHandle, &fileSrcStatus);
        *bytesReceived = fileSrcStatus.bytesStreamed;
    }

}

int
printNexusStatus(
    IpStreamerCtx *ipStreamerCtx
    )
{
#ifndef DMS_CROSS_PLATFORMS
    NEXUS_Error rc = NEXUS_UNKNOWN;
    NEXUS_PlaypumpStatus ppStatus;
    B_PlaybackIpStatus playbackIpStatus;
    NEXUS_RecordStatus recDstStatus;

    BDBG_WRN(("%s: CTX %p: ----------------------", BSTD_FUNCTION, (void *)ipStreamerCtx));
    if (ipStreamerCtx->ipSrc && ipStreamerCtx->ipSrc->playpump) {
        rc = NEXUS_Playpump_GetStatus(ipStreamerCtx->ipSrc->playpump, &ppStatus);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
        rc = B_PlaybackIp_GetStatus(ipStreamerCtx->ipSrc->playbackIp, &playbackIpStatus);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }

        BDBG_WRN(("IP Ctx %p: Playback IP State %d, Playpump buffer depth %d, size %d, fullness %d%%, played bytes %lld",
            (void *)ipStreamerCtx, playbackIpStatus.ipState, ppStatus.fifoDepth, ppStatus.fifoSize, (ppStatus.fifoDepth*100)/ppStatus.fifoSize, ppStatus.bytesPlayed));
    }

#ifdef NEXUS_HAS_VIDEO_ENCODER
    if (ipStreamerCtx->cfg.transcodeEnabled && !ipStreamerCtx->channelChange) {
        NEXUS_PlaybackStatus pbStatus;
        NEXUS_VideoEncoderStatus status;
        NEXUS_VideoDecoderStatus videoDecodeStatus;
        NEXUS_AudioDecoderStatus audioDecodeStatus;
        NEXUS_AudioMuxOutputStatus audioMuxOutputStatus;

        if (ipStreamerCtx->fileSrc && ipStreamerCtx->fileSrc->playbackHandle)
        {
            if (NEXUS_Playpump_GetStatus(ipStreamerCtx->fileSrc->playpumpHandle, &ppStatus) == NEXUS_SUCCESS &&
                NEXUS_Playback_GetStatus(ipStreamerCtx->fileSrc->playbackHandle, &pbStatus) == NEXUS_SUCCESS )
            {
                BDBG_WRN(("%s: Playback[%d] Status:-->", BSTD_FUNCTION, ppStatus.index));
                BDBG_WRN(("%s: PB fifo depth/size = %u/%u", BSTD_FUNCTION, pbStatus.fifoDepth, pbStatus.fifoSize));
                BDBG_WRN(("%s: PP fifo depth/size = %u/%u", BSTD_FUNCTION, ppStatus.fifoDepth, ppStatus.fifoSize));
            }
        }
        if (NEXUS_VideoDecoder_GetStatus(ipStreamerCtx->transcoderDst->videoDecoder, &videoDecodeStatus) == NEXUS_SUCCESS)
        {
            BDBG_MSG(("%s: Video Decoder[%p] Status:-->", BSTD_FUNCTION, (void *)ipStreamerCtx->transcoderDst->videoDecoder));
            BDBG_MSG(("%s: data buffer depth/size= %u/%u", BSTD_FUNCTION, videoDecodeStatus.fifoDepth, videoDecodeStatus.fifoSize));
            BDBG_MSG(("%s: queued frames         = %u", BSTD_FUNCTION, videoDecodeStatus.queueDepth));
            BDBG_MSG(("%s: numDecoded count      = %u", BSTD_FUNCTION, videoDecodeStatus.numDecoded));
            BDBG_MSG(("%s: numDisplayed          = %u", BSTD_FUNCTION, videoDecodeStatus.numDisplayed));
            BDBG_MSG(("%s: numDecodeDrops        = %u", BSTD_FUNCTION, videoDecodeStatus.numDecodeDrops));
            BDBG_MSG(("%s: numDisplayDrops       = %u", BSTD_FUNCTION, videoDecodeStatus.numDisplayDrops));
            BDBG_MSG(("%s: numDisplayUnderflows  = %u", BSTD_FUNCTION, videoDecodeStatus.numDisplayUnderflows));
            BDBG_MSG(("%s: current PTS (45KHz)   = 0x%x", BSTD_FUNCTION, videoDecodeStatus.pts));
            BDBG_MSG(("%s: PTS error count       = %u", BSTD_FUNCTION, videoDecodeStatus.ptsErrorCount));
        }

        if (NEXUS_AudioDecoder_GetStatus(ipStreamerCtx->transcoderDst->audioDecoder, &audioDecodeStatus) == NEXUS_SUCCESS)
        {
            BDBG_MSG(("%s: Audio Decoder[%p] Status:-->", BSTD_FUNCTION, (void *)ipStreamerCtx->transcoderDst->audioDecoder));
            BDBG_MSG(("%s: data buffer depth/size= %u/%u", BSTD_FUNCTION, audioDecodeStatus.fifoDepth, audioDecodeStatus.fifoSize));
            BDBG_MSG(("%s: queued frames         = %u", BSTD_FUNCTION, audioDecodeStatus.queuedFrames));
            BDBG_MSG(("%s: numDecoded count      = %u", BSTD_FUNCTION, audioDecodeStatus.framesDecoded));
            BDBG_MSG(("%s: numDummyFrames        = %u", BSTD_FUNCTION, audioDecodeStatus.dummyFrames));
            BDBG_MSG(("%s: numFifoOverflows      = %u", BSTD_FUNCTION, audioDecodeStatus.numFifoOverflows));
            BDBG_MSG(("%s: numFifoUnderflows     = %u", BSTD_FUNCTION, audioDecodeStatus.numFifoUnderflows));
            BDBG_MSG(("%s: current PTS (45KHz)   = 0x%x", BSTD_FUNCTION, audioDecodeStatus.pts));
            BDBG_MSG(("%s: PTS error count       = %u", BSTD_FUNCTION, audioDecodeStatus.ptsErrorCount));
        }

        if (NEXUS_VideoEncoder_GetStatus(ipStreamerCtx->transcoderDst->videoEncoder, &status) == NEXUS_SUCCESS)
        {
            BDBG_MSG(("%s: Video Encoder[%p] Status:-->", BSTD_FUNCTION, (void *)ipStreamerCtx->transcoderDst->videoEncoder));
            BDBG_MSG(("%s: error flags                  = 0x%x", BSTD_FUNCTION, status.errorFlags));
            BDBG_MSG(("%s: error count                  = %u", BSTD_FUNCTION, status.errorCount));
            BDBG_MSG(("%s: picture drops due to error   = %u", BSTD_FUNCTION, status.picturesDroppedErrors));
            BDBG_MSG(("%s: picture drops due to FRC     = %u", BSTD_FUNCTION, status.picturesDroppedFRC));
            BDBG_MSG(("%s: pictures Encoded             = %u", BSTD_FUNCTION, status.picturesEncoded));
            BDBG_MSG(("%s: pictures Received            = %u", BSTD_FUNCTION, status.picturesReceived));
            BDBG_MSG(("%s: picture Id Last Encoded      = 0x%x", BSTD_FUNCTION, status.pictureIdLastEncoded));
#if 0
            NEXUS_VideoEncoderClearStatus clearStatus;
            NEXUS_VideoEncoder_GetDefaultClearStatus(&clearStatus);
            NEXUS_VideoEncoder_ClearStatus(ipStreamerCtx->transcoderDst->videoEncoder, &clearStatus);
#endif
        }

        if (NEXUS_AudioMuxOutput_GetStatus(ipStreamerCtx->transcoderDst->audioMuxOutput, &audioMuxOutputStatus) == NEXUS_SUCCESS)
        {
            BDBG_MSG(("%s: Audio mux output[%p] Status:-->", BSTD_FUNCTION, (void *)ipStreamerCtx->transcoderDst->audioMuxOutput));
            BDBG_MSG(("%s: numEncoded frames     = %u", BSTD_FUNCTION, audioMuxOutputStatus.numFrames));
            BDBG_MSG(("%s: numErrorFrames        = %u", BSTD_FUNCTION, audioMuxOutputStatus.numErrorFrames));
        }
    }
#endif
    if (ipStreamerCtx->ipDst) {
        if (ipStreamerCtx->cfg.enableTimeshifting) {
            NEXUS_RecordStatus ipDstStatus;
            NEXUS_FilePosition first, last;
            NEXUS_Record_GetStatus(ipStreamerCtx->ipDst->recordHandle, &ipDstStatus);
            NEXUS_FifoRecord_GetPosition(ipStreamerCtx->ipDst->fifoFileHandle, &first, &last);
            BDBG_MSG(("%s: IP Ctx %p: FIFO: first:last %lld:%lld, IP Destination: bytes streamed %lld, buffered: depth %d, size %d", BSTD_FUNCTION,
                        (void *)ipStreamerCtx, first.mpegFileOffset, last.mpegFileOffset, ipDstStatus.recpumpStatus.data.bytesRecorded, ipDstStatus.recpumpStatus.data.fifoDepth, ipDstStatus.recpumpStatus.data.fifoSize));
        }
        else {
            NEXUS_RecpumpStatus recStatus;
            NEXUS_Recpump_GetStatus(ipStreamerCtx->ipDst->recpumpHandle, &recStatus);
            BDBG_MSG(("%s: Recpump[%d] Status:-->", BSTD_FUNCTION, recStatus.rave.index));
            BDBG_MSG(("%s: bitrate = %u, fifo depth/size = %u/%u", BSTD_FUNCTION, recStatus.bitrate, recStatus.data.fifoDepth, recStatus.data.fifoSize));
        }
    }

#ifdef NEXUS_HAS_RECORD
    if (ipStreamerCtx->recDst) {
        NEXUS_Record_GetStatus(ipStreamerCtx->recDst->recordHandle, &recDstStatus);
        BDBG_WRN(("IP Ctx %p: Record Destination: bytes recorded %lld, buffered: depth %d, size %d",
            (void *)ipStreamerCtx, recDstStatus.recpumpStatus.data.bytesRecorded, recDstStatus.recpumpStatus.data.fifoDepth, recDstStatus.recpumpStatus.data.fifoSize));
    }
#endif
#endif /* DMS_CROSS_PLATFORMS */
    BDBG_MSG(("%s: ----------------------", BSTD_FUNCTION));
    return 0;
}

#ifndef DMS_CROSS_PLATFORMS
/* select free dst */
int
openNexusDst(
    B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;

    BDBG_MSG(("%s: ipDstEnabled %d; recDstEnabled %d", BSTD_FUNCTION, ipStreamerCfg->ipDstEnabled, ipStreamerCfg->recDstEnabled ));
    if (ipStreamerCfg->ipDstEnabled) {
        if (ipStreamerCfg->enableTimeshifting)
            rc = openNexusIpDstWithTimeshift(psi, ipStreamerCfg, ipStreamerCtx);
        else
            rc = openNexusIpDstWithoutTimeshift(psi, ipStreamerCfg, ipStreamerCtx);
        if (rc) {BDBG_ERR(("Failed to Open Ip dst")); return -1;}
    }
    if (ipStreamerCfg->recDstEnabled) {
        rc = openNexusRecDst(psi, ipStreamerCfg, ipStreamerCtx);
        if (rc) {BDBG_ERR(("Failed to Open Rec dst")); return -1;}
    }
    return 0;
}

void
closeNexusDst(
    IpStreamerCtx *ipStreamerCtx
    )
{
    if (ipStreamerCtx->ipDst) {
        if (ipStreamerCtx->cfg.enableTimeshifting)
            closeNexusIpDstWithTimeshift(ipStreamerCtx);
        else
            closeNexusIpDstWithoutTimeshift(ipStreamerCtx);
    }
    if (ipStreamerCtx->recDst) {
        closeNexusRecDst(ipStreamerCtx);
    }
}
#endif /* DMS_CROSS_PLATFORMS */


/* select a free src of requested type */
int
openNexusSrc(
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    ipStreamerCtx->srcType = ipStreamerCfg->srcType;
    struct sockaddr_in remoteAddr;
    int remoteAddrLen;

    BDBG_MSG(("%s - StreamerCfg (%p); StreamerCtx (%p); srcType (%u)", BSTD_FUNCTION, (void *)ipStreamerCfg, (void *)ipStreamerCtx, ipStreamerCfg->srcType ));
    if (ipStreamerCtx->globalCtx->globalCfg.disableFrontend &&
        ipStreamerCfg->srcType != IpStreamerSrc_eFile &&
        ipStreamerCfg->srcType != IpStreamerSrc_eSat ) {
        BDBG_ERR(("ERROR: Can't tune to live channel when frontend support is diabled"));
        return -1;
    }

    remoteAddrLen = sizeof(remoteAddr);
    memset(&remoteAddr, 0, sizeof(remoteAddr));
    if (ipStreamerCfg->streamingFd > 0 && ipStreamerCfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eHttp && ipStreamerCfg->ipDstEnabled) {
        if (getpeername(ipStreamerCfg->streamingFd, (struct sockaddr *)&remoteAddr, (socklen_t *)&remoteAddrLen) != 0) {
            BDBG_ERR(("ERROR: Failed to obtain remote IP address, errno: %d\n", errno));
            return -1;
        }
        BDBG_MSG(("%s: Request from: %s:%d", BSTD_FUNCTION, inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port)));
    }

    switch (ipStreamerCfg->srcType) {
#if NEXUS_HAS_FRONTEND
    case IpStreamerSrc_eQam:
        BDBG_WRN(("CTX %p: Client (%s:%d, streaming fd %d) is requesting to tune to Live QAM Channel: Frequency: %d, Qam Mode %s, Sub Channel %d",
            (void *)ipStreamerCtx, (ipStreamerCfg->streamingFd > 0 && ipStreamerCfg->ipDstEnabled)?inet_ntoa(remoteAddr.sin_addr):"", (ipStreamerCfg->streamingFd > 0 && ipStreamerCfg->ipDstEnabled)?htons(remoteAddr.sin_port):0,
            ipStreamerCfg->streamingFd, ipStreamerCfg->frequency, ipStreamerCfg->qamMode == NEXUS_FrontendQamMode_e256 ? "QAM256" : "QAM64", ipStreamerCfg->subChannel));

        /* select next free QAM source */
        rc = openNexusQamSrc(ipStreamerCfg, ipStreamerCtx);
        if (rc) {BDBG_ERR(("Failed to Open QAM Source")); return -1;}
        break;
    case IpStreamerSrc_eSat:
        BDBG_WRN(("CTX %p: Client (%s:%d) is requesting to tune to Live Sat Channel: Frequency: %d, Mode %s, Sub Channel %d",
            (void *)ipStreamerCtx, (ipStreamerCfg->streamingFd > 0 && ipStreamerCfg->ipDstEnabled)?inet_ntoa(remoteAddr.sin_addr):"", (ipStreamerCfg->streamingFd > 0 && ipStreamerCfg->ipDstEnabled)?htons(remoteAddr.sin_port):0,
            ipStreamerCfg->frequency, ipStreamerCfg->satMode == NEXUS_FrontendSatelliteMode_eDvb ? "DVB" : "DSS", ipStreamerCfg->subChannel));

        /* select next free Sat source */
        rc = openNexusSatSrc(ipStreamerCfg, ipStreamerCtx);
        if (rc) {BDBG_ERR(("Failed to Open Sat Source")); return -1;}
        break;
    case IpStreamerSrc_eOfdm:
        BDBG_WRN(("CTX %p: Client (%s:%d) is requesting to tune to Live OFDM Channel: Frequency: %d, Mode %s, Sub Channel %d",
            (void *)ipStreamerCtx, (ipStreamerCfg->streamingFd > 0 && ipStreamerCfg->ipDstEnabled)?inet_ntoa(remoteAddr.sin_addr):"", (ipStreamerCfg->streamingFd > 0 && ipStreamerCfg->ipDstEnabled)?htons(remoteAddr.sin_port):0,
            ipStreamerCfg->frequency, ipStreamerCfg->ofdmMode == NEXUS_FrontendOfdmMode_eIsdbt ? "ISDBT" : "DVBT", ipStreamerCfg->subChannel));

        /* select next free OFDM source */
        rc = openNexusOfdmSrc(ipStreamerCfg, ipStreamerCtx);
        if (rc) {BDBG_ERR(("Failed to Open Ofdm Source")); return -1;}
        break;
    case IpStreamerSrc_eVsb:
        BDBG_WRN(("Client is requesting to tune to Live VSB Channel: Frequency: %d, VSB Mode %s, Sub Channel %d",
            ipStreamerCfg->frequency, ipStreamerCfg->vsbMode == NEXUS_FrontendVsbMode_e8 ? "VSB8" : "VSB16", ipStreamerCfg->subChannel));

        /* select next free VSB source */
        rc = openNexusVsbSrc(ipStreamerCfg, ipStreamerCtx);
        if (rc) {BDBG_ERR(("Failed to to Open VSB Source")); return -1;}
        break;
#endif /* NEXUS_HAS_FRONTEND */
#ifndef DMS_CROSS_PLATFORMS
    case IpStreamerSrc_eStreamer:
        BDBG_WRN(("Client is requesting to %s  for %s from streamer source using %s protocol", ipStreamerCfg->mediaProbeOnly ? "Media Probe" : "Stream", ipStreamerCfg->fileName,
                    ipStreamerCfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eUdp ? "UDP": (ipStreamerCfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eRtp ? "RTP": "HTTP")));
        rc = openNexusStreamerSrc(ipStreamerCfg, ipStreamerCtx);
        if (rc) {BDBG_ERR(("Failed to to Open HDMI Source")); return -1;}
        break;
    case IpStreamerSrc_eIp:
        BDBG_WRN(("Client is requesting to tune to Live IP Channel: Host: %s, Port %d, Protocol %s, IpVersion %d",
            ipStreamerCfg->srcIpAddress, ipStreamerCfg->srcPort,
            ipStreamerCfg->srcProtocol == B_PlaybackIpProtocol_eUdp ? "UDP" :
            ipStreamerCfg->srcProtocol == B_PlaybackIpProtocol_eRtp ? "RTP" :
            ipStreamerCfg->srcProtocol == B_PlaybackIpProtocol_eHttp ? "HTTP" :
            "RTP no RTCP",
            ipStreamerCfg->iphVersion));

        /* select next free IP source */
        rc = openNexusIpSrc(ipStreamerCfg, ipStreamerCtx);
        if (rc) {BDBG_ERR(("Failed to to Open IP Source")); return -1;}
        break;
#endif /* DMS_CROSS_PLATFORMS */
    case IpStreamerSrc_eHdmi:
        BDBG_WRN(("Client is requesting to Encode & Stream from HDMI Input"));

#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
        if (ipStreamerCfg->mediaProbeOnly)
            break;
        rc = openNexusHdmiSrc(ipStreamerCfg, ipStreamerCtx);
        if (rc) {BDBG_ERR(("Failed to to Open HDMI Source")); return -1;}
#else
        BDBG_ERR(("ERROR: HDMI Source Input is not supported for this Platform")); return -1;
#endif /* defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT) */
        break;
    case IpStreamerSrc_eFile:
        BDBG_WRN(("Client is requesting to %s file '%s' using %s protocol", ipStreamerCfg->mediaProbeOnly ? "Media Probe" : "Stream", ipStreamerCfg->fileName,
                    ipStreamerCfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eUdp ? "UDP": (ipStreamerCfg->streamingCfg.streamingProtocol == B_PlaybackIpProtocol_eRtp ? "RTP": "HTTP")));

        rc = openNexusFileSrc(ipStreamerCfg, ipStreamerCtx);
        if (rc) {BDBG_ERR(("Failed to to Open Local File Source")); return -1;}
        break;
    default:
        BDBG_ERR(("Requested SourceType %d is not yet supported", ipStreamerCfg->srcType));
        return -1;
    }
    return 0;
}

/* select a free src of requested type */
void
closeNexusSrc(
    IpStreamerCtx *ipStreamerCtx
    )
{
#ifndef DMS_CROSS_PLATFORMS
    if (ipStreamerCtx->ipSrc)
        closeNexusIpSrc(ipStreamerCtx);
#if NEXUS_HAS_FRONTEND
    else if (ipStreamerCtx->vsbSrc)
        closeNexusVsbSrc(ipStreamerCtx);
    else if (ipStreamerCtx->qamSrc)
        closeNexusQamSrc(ipStreamerCtx);
    else if (ipStreamerCtx->satSrc)
        closeNexusSatSrc(ipStreamerCtx);
    else if (ipStreamerCtx->ofdmSrc)
        closeNexusOfdmSrc(ipStreamerCtx);
#endif /* NEXUS_HAS_FRONTEND */
    else if (ipStreamerCtx->streamerSrc)
        closeNexusStreamerSrc(ipStreamerCtx);
#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
    else if (ipStreamerCtx->hdmiSrc)
        closeNexusHdmiSrc(ipStreamerCtx);
#endif /* defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT) */
    else
#endif /* DMS_CROSS_PLATFORMS */
    if (ipStreamerCtx->fileSrc)
        closeNexusFileSrc(ipStreamerCtx);
}

int
B_IpStreamer_SessionAcquirePsiInfo(
    void *ctx,
    B_PlaybackIpPsiInfo *psiOut
    )
{
    int rc;
    IpStreamerConfig *ipStreamerCfg;
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)ctx;

    if (!ipStreamerCtx) {
        BDBG_ERR(("ERROR: Invalid session ctx %p", (void *)ipStreamerCtx));
        return -1;
    }

    BDBG_MSG(("%s: for CTX %p, pidListCount %d", BSTD_FUNCTION, ctx, ipStreamerCtx->cfg.pidListCount ));
    for (int idx=0; idx<ipStreamerCtx->cfg.pidListCount; idx++) {
        BDBG_MSG(("%s: pidList[%d] = %d", BSTD_FUNCTION, idx, ipStreamerCtx->cfg.pidList[idx] ));
    }

    if (ipStreamerCtx->cfg.pidListCount != 0 || ipStreamerCtx->cfg.enableAllpass ) {
        /* since client has already provided the list of pids it is interested in, we dont need to do the PSI discovery */
        BDBG_MSG(("%s: %s, so Skip PSI Acquistion for CTX %p", BSTD_FUNCTION, ipStreamerCtx->cfg.enableAllpass ? "Allpass mode":"Client specified PID list", (void *)ipStreamerCtx));
        memset(psiOut, 0, sizeof(B_PlaybackIpPsiInfo));
        return 0;
    }
    BDBG_MSG(("%s: Start PSI Acquistion for CTX %p", BSTD_FUNCTION, (void *)ipStreamerCtx));

    ipStreamerCfg = &ipStreamerCtx->cfg;
    switch (ipStreamerCfg->srcType) {
#if NEXUS_HAS_FRONTEND
    case IpStreamerSrc_eQam:
        rc = setupAndAcquirePsiInfoQamSrc(ipStreamerCfg, ipStreamerCtx, psiOut);
        if (rc) {
            BDBG_ERR(("PSI Acquistion failed from QAM Src"));
            return -1;
        }
        psiOut->liveChannel = true;
        break;
    case IpStreamerSrc_eOfdm:
        rc = setupAndAcquirePsiInfoOfdmSrc(ipStreamerCfg, ipStreamerCtx, psiOut);
        if (rc) {
            BDBG_ERR(("PSI Acquistion failed from Ofdm Src"));
            return -1;
        }
        psiOut->liveChannel = true;
        break;
    case IpStreamerSrc_eSat:
        rc = setupAndAcquirePsiInfoSatSrc(ipStreamerCfg, ipStreamerCtx, psiOut);
        if (rc) {
            BDBG_ERR(("PSI Acquistion failed from Sat Src"));
            return -1;
        }
        BDBG_MSG(("PSI Acquistion from Sat Src succeeded"));
        psiOut->liveChannel = true;
        break;
    case IpStreamerSrc_eVsb:
        rc = setupAndAcquirePsiInfoVsbSrc(ipStreamerCfg, ipStreamerCtx, psiOut);
        if (rc) {
            BDBG_ERR(("PSI Acquistion failed from VSB Src"));
            return -1;
        }
        psiOut->liveChannel = true;
        break;
#endif /* NEXUS_HAS_FRONTEND */
#ifndef DMS_CROSS_PLATFORMS
    case IpStreamerSrc_eStreamer:
        rc = setupAndAcquirePsiInfoStreamerSrc(ipStreamerCfg, ipStreamerCtx, psiOut);
        if (rc) {
            BDBG_ERR(("PSI Acquistion failed from Streamer Src"));
            return -1;
        }
        psiOut->liveChannel = true;
        break;
    case IpStreamerSrc_eIp:
        rc = setupAndAcquirePsiInfoIpSrc(ipStreamerCfg, ipStreamerCtx, psiOut);
        if (rc) {
            BDBG_ERR(("PSI Acquistion failed from IP Src"));
            return -1;
        }
        psiOut->liveChannel = true;
        break;
#endif /* DMS_CROSS_PLATFORMS */
#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
    case IpStreamerSrc_eHdmi:
        memset(psiOut, 0, sizeof(B_PlaybackIpPsiInfo));
        psiOut->videoPid = psiOut->pcrPid = 1;
        psiOut->mpegType = ipStreamerCfg->transcode.transportType;
        psiOut->liveChannel = true;
        psiOut->psiValid = true;
        if (ipStreamerCfg->transcode.outAudio)
            psiOut->audioPid = 2;
        return 0;
#endif /* defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT) */
    case IpStreamerSrc_eFile:
        rc = setupAndAcquirePsiInfoFileSrc(ipStreamerCtx, psiOut);
        if (rc) {
            BDBG_ERR(("PSI Acquistion failed from File Src"));
            return -1;
        }
        psiOut->liveChannel = false;
        break;
    default:
        BDBG_ERR(("Requested Frontend Src is not yet supported"));
        return -1;
    }
    BDBG_MSG(("PSI Video Pid %d, PSI Codec %d, PSI Audio Pid %d, PSI Audio Codec %d, PSI Pcr Pid %d, PSI Transport Type %d",
                psiOut->videoPid, psiOut->videoCodec, psiOut->audioPid, psiOut->audioCodec, psiOut->pcrPid, psiOut->mpegType));
    BDBG_MSG(("%s: PSI Acquistion complete for CTX %p", BSTD_FUNCTION, (void *)ipStreamerCtx));
    return 0;
}

void *
B_IpStreamer_SessionOpen(
    void *gCtx,
    IpStreamerOpenSettings *openSettings
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = (IpStreamerGlobalCtx *)gCtx;
    IpStreamerCtx *ipStreamerCtx;

    BDBG_MSG(("%s: global ctx %p, openSettings %p; streamingFd (%u)", BSTD_FUNCTION, (void *)ipStreamerGlobalCtx, (void *)openSettings, openSettings->streamingFd ));

    if (gCtx == NULL || openSettings == NULL) {
        BDBG_ERR(("%s: Invalid parameters: global ctx %p, openSettings %p", BSTD_FUNCTION, (void *)ipStreamerGlobalCtx, (void *)openSettings));
        return NULL;
    }

    if ((ipStreamerCtx = BKNI_Malloc(sizeof(IpStreamerCtx))) == NULL) { BDBG_ERR(("BKNI_Malloc Failure at %d", __LINE__)); goto out;}
    BDBG_MSG(("%s: ipStreamerCtx = malloc(%d) successful ", BSTD_FUNCTION, sizeof(IpStreamerCtx) ));
    memset(ipStreamerCtx, 0, sizeof(IpStreamerCtx));
    if (BKNI_CreateMutex(&ipStreamerCtx->lock) != 0) {
        BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
        goto out;
    }
    ipStreamerCtx->globalCtx = ipStreamerGlobalCtx;
    if (openSettings->eventCallback && openSettings->appCtx == NULL)
        /* Full DMS app specifies its own callback & app ctx, ip_streamer app can only specify the context here as it gets allocated in this function */
        openSettings->appCtx = ipStreamerCtx;

    /* parse incoming URL and determine what src & dst needs to be used for this live/file streaming session */
    if (getRequestInfoFromUrl(openSettings, &ipStreamerCtx->cfg, &ipStreamerCtx->globalCtx->globalCfg) < 0) {
        BDBG_ERR(("ERROR: Failed to parse/validate URL"));
        goto out;
    }

    if (ipStreamerCtx->cfg.hlsSession) {
        if (BKNI_CreateEvent(&ipStreamerCtx->statusEvent)) {
            BDBG_ERR(("%s: Failed to create event at %d", BSTD_FUNCTION, __LINE__));
            goto out;
        }
    }

    /* select src using requested sourceType */
    rc = openNexusSrc(&ipStreamerCtx->cfg, ipStreamerCtx);
    if (rc) {BDBG_ERR(("Failed to select a free src, go back to new listening")); goto errorClose;}

    BDBG_MSG(("%s: done ... session ctx %p", BSTD_FUNCTION, (void *)ipStreamerCtx));
    return (void *)ipStreamerCtx;

errorClose:
    closeNexusSrc(ipStreamerCtx);
out:
    BDBG_ERR(("%s: ERROR: ctx %p", BSTD_FUNCTION, (void *)ipStreamerCtx));
    if (ipStreamerCtx)
    {
        if (ipStreamerCtx->lock)
            BKNI_DestroyMutex(ipStreamerCtx->lock);
        if (ipStreamerCtx->statusEvent)
            BKNI_DestroyEvent(ipStreamerCtx->statusEvent);
        BKNI_Free(ipStreamerCtx);
    }
    return NULL;
}

void
B_IpStreamer_SessionClose(
    void *ctx
    )
{
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)ctx;

    if (!ipStreamerCtx)
        return;

    BDBG_MSG(("%s: closing session ctx %p", BSTD_FUNCTION, (void *)ipStreamerCtx));
    closeNexusSrc(ipStreamerCtx);
    if (ipStreamerCtx->statusEvent)
        BKNI_DestroyEvent(ipStreamerCtx->statusEvent);
    if (ipStreamerCtx->lock)
        BKNI_DestroyMutex(ipStreamerCtx->lock);
    BKNI_Free(ipStreamerCtx);
    BDBG_MSG(("%s: closed session ctx", BSTD_FUNCTION));
}

int
B_IpStreamer_SessionSetSettings(
    void *ctx,
    IpStreamerSettings *sessionSettings
    )
{
#ifndef DMS_CROSS_PLATFORMS
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)ctx;
    B_PlaybackIpLiveStreamingSettings settings;
    IpStreamerConfig *ipStreamerCfg;

    if (!ipStreamerCtx || !ipStreamerCtx->ipDst) {
        BDBG_ERR(("%s: referencing a stale Ip Streaming session...", BSTD_FUNCTION));
        return -1;
    }
    if (!ipStreamerCtx->ipStreamingInProgress) {
        BDBG_ERR(("%s: Ip Streaming session is NOT in progress, returning error...", BSTD_FUNCTION));
        return -1;
    }
    ipStreamerCfg = &ipStreamerCtx->cfg;
#ifdef NEXUS_HAS_VIDEO_ENCODER
#ifndef NEXUS_NUM_DSP_VIDEO_ENCODERS
    if (sessionSettings->xcodeModifyBitrate)
    {
        NEXUS_VideoEncoderSettings videoEncoderConfig;
        NEXUS_VideoEncoder_GetSettings(ipStreamerCtx->transcoderDst->videoEncoder, &videoEncoderConfig);
        videoEncoderConfig.bitrateMax = sessionSettings->xcodeBitrate;
        if (NEXUS_VideoEncoder_SetSettings(ipStreamerCtx->transcoderDst->videoEncoder, &videoEncoderConfig)) {
            BDBG_ERR(("%s: ERROR: Failed to set the Video Encoder Configuration", BSTD_FUNCTION));
            return -1;
        }
        BDBG_WRN(("%s: ################### modified bitrate to %d ###############", BSTD_FUNCTION, sessionSettings->xcodeBitrate));
    }
#endif
#endif
    if (sessionSettings->streamingEnabled || sessionSettings->resumeStreaming) {
        /* each new segment need to start w/ a PAT & PMT */
        memset(&settings, 0, sizeof(settings));
        ipStreamerCtx->cfg.streamingFd = sessionSettings->streamingFd;
#ifdef NEXUS_HAS_VIDEO_ENCODER
        if (ipStreamerCfg->transcodeEnabled && ipStreamerCfg->hlsSession) {
            preparePatPmt(ipStreamerCtx);
            settings.psiTables.patBufferPtr = (void *)ipStreamerCtx->transcoderDst->psi[0].pData;
            settings.psiTables.patBufferSize = 188;
            settings.psiTables.pmtBufferPtr = (void *)ipStreamerCtx->transcoderDst->psi[1].pData;
            settings.psiTables.pmtBufferSize = 188;
        }
#endif
        settings.streamingEnabled = sessionSettings->streamingEnabled;
        settings.resetStreaming = sessionSettings->resetStreaming;
        settings.resumeStreaming = sessionSettings->resumeStreaming;
        settings.streamingFd = sessionSettings->streamingFd;
        settings.hlsSegmentSize = sessionSettings->hlsSegmentSize;
        if (B_PlaybackIp_LiveStreamingSetSettings(ipStreamerCtx->ipDst->liveStreamingHandle, &settings) != B_ERROR_SUCCESS) {
            BDBG_ERR(("%s: ERROR: Failed to start File Streaming handle", BSTD_FUNCTION));
            return -1;
        }
        BDBG_MSG(("%s: enabled live Streaming settings", BSTD_FUNCTION));
    }
#endif /* DMS_CROSS_PLATFORMS */
    return 0;
}

int
B_IpStreamer_SessionStart(
    void *ctx,
    B_PlaybackIpPsiInfo *psi
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)ctx;
    IpStreamerConfig *ipStreamerCfg;

    BDBG_MSG(("%s: start streaming on session ctx %p", BSTD_FUNCTION, (void *)ipStreamerCtx));
    if (!ipStreamerCtx || !psi) {
        BDBG_ERR(("ERROR: Invalid parameters: session ctx %p, psi %p", (void *)ipStreamerCtx, (void *)psi));
        return -1;
    }
    ipStreamerCfg = &ipStreamerCtx->cfg;

    /* setup the pid filtering logic */
    rc = openNexusPidChannels(psi, ipStreamerCfg, ipStreamerCtx);
    if (rc) {BDBG_ERR(("Failed to open the PID Channels...")); goto errorClosePidChannels;}

    /* select free dst */
#ifndef DMS_CROSS_PLATFORMS
    rc = openNexusDst(psi, ipStreamerCfg, ipStreamerCtx);
    if (rc) {BDBG_ERR(("Failed to open dst handles")); goto errorCloseDst;}
#endif /* DMS_CROSS_PLATFORMS */

    /* start src */
    rc = startNexusSrc(ipStreamerCtx);
    if (rc) {BDBG_ERR(("Failed to start content sources")); goto errorStopSrc;}

    /* start dst */
#ifndef DMS_CROSS_PLATFORMS
    rc = startNexusDst(ipStreamerCtx);
    if (rc) {BDBG_ERR(("Failed to start destinations")); goto errorStopDst;}
#endif /* DMS_CROSS_PLATFORMS */

    BDBG_MSG(("CTX: %p: started %s streaming", BSTD_FUNCTION, ipStreamerCtx->fileSrc? "media file" : "live channel"));
    return 0;

errorStopDst:
#ifndef DMS_CROSS_PLATFORMS
    stopNexusDst(ipStreamerCtx);
#endif /* DMS_CROSS_PLATFORMS */
errorStopSrc:
#ifndef DMS_CROSS_PLATFORMS
    stopNexusSrc(ipStreamerCtx);
#endif /* DMS_CROSS_PLATFORMS */
errorCloseDst:
#ifndef DMS_CROSS_PLATFORMS
    if (!ipStreamerCtx->cfg.transcodeEnabled)
        /* for xcode path, dst & pid channels are closed in its closeNexusTranscoderPipe function */
        closeNexusDst(ipStreamerCtx);
#endif /* DMS_CROSS_PLATFORMS */
errorClosePidChannels:
#ifndef DMS_CROSS_PLATFORMS
    if (!ipStreamerCtx->cfg.transcodeEnabled)
        closeNexusPidChannels(ipStreamerCtx);
#endif /* DMS_CROSS_PLATFORMS */
    return -1;
}

void
B_IpStreamer_SessionStop(
    void *ctx
    )
{
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)ctx;
    BDBG_MSG(("%s: stopping streaming on session ctx %p", BSTD_FUNCTION, (void *)ipStreamerCtx));
    /* session specific cleanup */
    stopNexusSrc(ipStreamerCtx);
#ifndef DMS_CROSS_PLATFORMS
    if (!ipStreamerCtx->cfg.transcodeEnabled) {
        /* we only stop destination if it is a non-transcoded session */
        /* for transcoding session, we need to stop the transcoder pipeline before we can stop the destination */
        /* otherwise, transcoder can hang as its output is not consumed due to premature close of destination */
        /* transcoder pipeline is stopped/closed via the src close call as it was setup in the src open call */
        stopNexusDst(ipStreamerCtx);
        /* we close the dst & pidChannels here as they are opened part of the SessionStart call */
        closeNexusDst(ipStreamerCtx);
        closeNexusPidChannels(ipStreamerCtx);
    }
    else {
        BDBG_MSG(("%s: delaying nexus dst stop for transcoded sessions", BSTD_FUNCTION));
    }
#endif /* DMS_CROSS_PLATFORMS */
    BDBG_MSG(("%s: stopped streaming on session ctx %p", BSTD_FUNCTION, (void *)ipStreamerCtx));
}

void
B_IpStreamer_SessionStatus(
    void *ctx,
    IpStreamerSessionStatus *status
    )
{
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)ctx;
    IpStreamerConfig *ipStreamerCfg;
    uint64_t newBytesReceived;
    B_PlaybackIpFileStreamingStatus fileSrcStatus;
#ifndef DMS_CROSS_PLATFORMS
    B_PlaybackIpLiveStreamingStatus liveStreamingStatus;
#endif /* DMS_CROSS_PLATFORMS */

    memset(status, 0, sizeof(IpStreamerSessionStatus));
    status->active = false;
    if (!ipStreamerCtx) {
        BDBG_ERR(("ERROR: Invalid session ctx %p", (void *)ipStreamerCtx));
        return;
    }
    ipStreamerCfg = &ipStreamerCtx->cfg;

    if (!ipStreamerCtx->fileStreamingInProgress && !ipStreamerCtx->ipStreamingInProgress && !ipStreamerCtx->localRecordingInProgress && !ipStreamerCtx->overflow) {
        BDBG_MSG(("Stopping current ip streaming / local recording session: streaming: CTX %p: file %d, live %d, recording %d, overflow %d",
            (void *)ipStreamerCtx, ipStreamerCtx->fileStreamingInProgress, ipStreamerCtx->ipStreamingInProgress, ipStreamerCtx->localRecordingInProgress, ipStreamerCtx->overflow));
        return;
    }
    if (ipStreamerCtx->fileStreamingInProgress) {
        B_PlaybackIp_FileStreamingGetStatus(ipStreamerCtx->fileSrc->fileStreamingHandle, &fileSrcStatus);
        if (fileSrcStatus.connectionState == B_PlaybackIpConnectionState_eError || fileSrcStatus.connectionState == B_PlaybackIpConnectionState_eEof) {
            BDBG_MSG(("Stopping file streaming session: connectionState %d", fileSrcStatus.connectionState));
            return;
        }
    }
#ifndef DMS_CROSS_PLATFORMS
    else if (ipStreamerCtx->ipDst) {
        B_PlaybackIp_LiveStreamingGetStatus(ipStreamerCtx->ipDst->liveStreamingHandle, &liveStreamingStatus);
        if (liveStreamingStatus.connectionState == B_PlaybackIpConnectionState_eError || liveStreamingStatus.connectionState == B_PlaybackIpConnectionState_eEof) {
            BDBG_MSG(("Stopping live streaming session: connectionState %d", liveStreamingStatus.connectionState));
            return;
        }
    }
#endif /* DMS_CROSS_PLATFORMS */
    if (ipStreamerCtx->localRecordingInProgress) {
        uint64_t totalRecorded;
        getNexusSrcDstStatus(ipStreamerCtx, &totalRecorded);
        status->recordingDuration = (totalRecorded * 8) / ipStreamerCtx->globalCtx->globalCfg.maxBitRate;
        BDBG_MSG(("CTX %p: Recording duration %d, file size %lld, bit rate %d", (void *)ipStreamerCtx, status->recordingDuration, totalRecorded, ipStreamerCtx->globalCtx->globalCfg.maxBitRate));
    }

    getNexusSrcDstStatus(ipStreamerCtx, &newBytesReceived);
    if (ipStreamerCtx->globalCtx->globalCfg.printStats)
        printNexusStatus(ipStreamerCtx);
    status->active = true;
    return;
}

int
initNexusParserBandList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    unsigned i;
#ifndef DMS_CROSS_PLATFORMS
    for (i = 0; i < IP_STREAMER_NUM_PARSER_BANDS; i++) {
        memset(&ipStreamerGlobalCtx->parserBandList[i], 0, sizeof(ParserBand));

        if(ipStreamerGlobalCtx->globalCfg.multiProcessEnv)
        {
            ipStreamerGlobalCtx->parserBandList[i].parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
            if (!ipStreamerGlobalCtx->parserBandList[i].parserBand){
                BDBG_ERR(("Configuration Error: Not enough Parser bands (%lu) for available Frontends (%d)", (unsigned long)ipStreamerGlobalCtx->parserBandList[i].parserBand, NEXUS_NUM_PARSER_BANDS));
                return -1;
            }
        }
        else
        {
            ipStreamerGlobalCtx->parserBandList[i].parserBand = i;
            if (ipStreamerGlobalCtx->parserBandList[i].parserBand+1 >= NEXUS_NUM_PARSER_BANDS+1) {
                BDBG_ERR(("Configuration Error: Not enough Parser bands (%lu) for available Frontends (%d)", (unsigned long)ipStreamerGlobalCtx->parserBandList[i].parserBand, NEXUS_NUM_PARSER_BANDS));
                return -1;
            }

        }
        if (BKNI_CreateMutex(&ipStreamerGlobalCtx->parserBandList[i].lock) != 0) {
            BDBG_ERR(("BKNI_CreateMutex failed at %d", __LINE__));
            return -1;
        }
    }

    if (BKNI_CreateMutex(&ipStreamerGlobalCtx->parserBandMutex) != 0) {
        BDBG_ERR(("Failed to create BKNI mutex at %d", __LINE__));
        return -1;
    }
#endif /* DMS_CROSS_PLATFORMS */
    BDBG_MSG(("Initialized %d Parser Bands ", i));
    return 0;
}

void
unInitNexusParserBandList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
#ifndef DMS_CROSS_PLATFORMS
    for (i = 0; i < IP_STREAMER_NUM_PARSER_BANDS; i++) {
        if (ipStreamerGlobalCtx->parserBandList[i].lock)
            BKNI_DestroyMutex(ipStreamerGlobalCtx->parserBandList[i].lock);
    }
    if (ipStreamerGlobalCtx->parserBandMutex)
        BKNI_DestroyMutex(ipStreamerGlobalCtx->parserBandMutex);
#endif /* DMS_CROSS_PLATFORMS */
    BDBG_MSG(("ParserBand List is uninitialized"));
}

int
initDtcpIpLib(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
#ifdef B_HAS_DTCP_IP

    /* TODO: for VMS, we may need to pass in the nexus DMA handle here if it is already opened */
    if (DtcpInitHWSecurityParams(NULL) != BERR_SUCCESS) {
        BDBG_ERR(("ERROR: Failed to init DtcpHW Security parmas"));
        DtcpAppLib_Shutdown(ipStreamerGlobalCtx->dtcpCtx);
        ipStreamerGlobalCtx->dtcpCtx = NULL;
        return (-1);
    }

    if ((ipStreamerGlobalCtx->dtcpCtx = DtcpAppLib_Startup(B_DeviceMode_eSource, false,
                    ipStreamerGlobalCtx->globalCfg.dtcpKeyFormat,
                    ipStreamerGlobalCtx->globalCfg.ckc_check)) == NULL) {
        BDBG_ERR(("ERROR: DtcpAppLib_Startup failed"));
        return (-1);
    }

    if (DtcpAppLib_Listen(ipStreamerGlobalCtx->dtcpCtx, NULL /* src ip */, ipStreamerGlobalCtx->globalCfg.dtcpAkePort) != BERR_SUCCESS ) {
        BDBG_ERR(("ERROR: DtcpAppLib_listen failed"));
#ifdef B_DTCP_IP_HW_ENCRYPTION
        DtcpCleanupHwSecurityParams();
#endif
        DtcpAppLib_Shutdown(ipStreamerGlobalCtx->dtcpCtx);
        ipStreamerGlobalCtx->dtcpCtx = NULL;
        return (-1);
    }
    BDBG_MSG(("DTCP/IP Library is initialized"));
    return 0;
#else
    BSTD_UNUSED(ipStreamerGlobalCtx);
    return 0;
#endif
}

void
unInitDtcpIpLib(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
#ifdef B_HAS_DTCP_IP
    if (!ipStreamerGlobalCtx->dtcpCtx)
        return;
    DtcpAppLib_CancelListen(ipStreamerGlobalCtx->dtcpCtx);
#ifdef B_DTCP_IP_HW_ENCRYPTION
    DtcpCleanupHwSecurityParams();
#endif
    DtcpAppLib_Shutdown(ipStreamerGlobalCtx->dtcpCtx);
    ipStreamerGlobalCtx->dtcpCtx = NULL;
    BDBG_MSG(("DTCP/IP Library is un-initialized"));
#else
    BSTD_UNUSED(ipStreamerGlobalCtx);
#endif
}

void
B_IpStreamer_UnInit(void *ctx)
{
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = ctx;
    int i = 0;

    BDBG_MSG(("%s: ipStreamerGlobalCtx %p", BSTD_FUNCTION, (void *)ipStreamerGlobalCtx));
    if (!ipStreamerGlobalCtx) {
        BDBG_ERR(("ERROR: Failed to uninitialize IP streamer, NULL global context"));
        return;
    }

    unInitDtcpIpLib(ipStreamerGlobalCtx);
#ifdef NEXUS_HAS_VIDEO_ENCODER
    unInitNexusTranscoderDstList(ipStreamerGlobalCtx);
#endif
#ifndef DMS_CROSS_PLATFORMS
    unInitNexusRecDstList(ipStreamerGlobalCtx);
    unInitNexusIpDstList(ipStreamerGlobalCtx);
    unInitNexusParserBandList(ipStreamerGlobalCtx);
    unInitNexusIpSrcList(ipStreamerGlobalCtx);
    unInitNexusQamSrcList(ipStreamerGlobalCtx);
    unInitNexusStreamerSrcList(ipStreamerGlobalCtx);
    unInitNexusSatSrcList(ipStreamerGlobalCtx);
    unInitNexusOfdmSrcList(ipStreamerGlobalCtx);
    unInitNexusVsbSrcList(ipStreamerGlobalCtx);
#endif /* DMS_CROSS_PLATFORMS */
#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
    unInitNexusHdmiSrcList(ipStreamerGlobalCtx);
#endif /* defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT) */
    for(i = 0; i<ipStreamerGlobalCtx->globalCfg.numStreamingSessions; i++)
    {
        if (ipStreamerGlobalCtx->globalCfg.streamingCfg[i].streamingIpAddress)
            free(ipStreamerGlobalCtx->globalCfg.streamingCfg[i].streamingIpAddress);
        if (ipStreamerGlobalCtx->globalCfg.streamingCfg[i].url)
            free(ipStreamerGlobalCtx->globalCfg.streamingCfg[i].url);
        memset(&ipStreamerGlobalCtx->globalCfg.streamingCfg[i], 0, sizeof(IpStreamerStreamingOutCfg));
    }
#ifndef DMS_CROSS_PLATFORMS
#ifdef HAVE_DLNA
    if (!getenv("client_enable_dms") || strcmp(getenv("client_enable_dms"), "y"))
#endif
#if defined NEXUS_HAS_DMA || defined NEXUS_HAS_XPT_DMA
    if (ipStreamerGlobalCtx->globalCfg.dmaHandle && ipStreamerGlobalCtx->globalCfg.freeDmaHandle)
        NEXUS_Dma_Close(ipStreamerGlobalCtx->globalCfg.dmaHandle);
#endif
#endif /* DMS_CROSS_PLATFORMS */
    BKNI_Free(ipStreamerGlobalCtx);
#ifndef DMS_CROSS_PLATFORMS
#ifdef HAVE_DLNA
    if (!getenv("client_enable_dms") || strcmp(getenv("client_enable_dms"), "y"))
#endif
        NEXUS_Platform_Uninit();
#endif /* DMS_CROSS_PLATFORMS */
    BDBG_MSG(("NEXUS Platfrom is un-initialized"));
    unInitLibs();
    BDBG_MSG(("%s: done", BSTD_FUNCTION));
}

void *
B_IpStreamer_Init(
    IpStreamerGlobalCfg *ipStreamerGlobalCfg
   )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    IpStreamerGlobalCtx *ipStreamerGlobalCtx = NULL;

    BDBG_MSG(("%s: ipStreamerGlobalCfg %p", BSTD_FUNCTION, (void *)ipStreamerGlobalCfg));
    if (!ipStreamerGlobalCfg) {
        BDBG_ERR(("ERROR: Global IP Streamer Configuration needs to be specified"));
        return NULL;
    }

    /* ignore SIGPIPE otherwise abnormal termination on client can cause server to crash */
    signal(SIGPIPE, SIG_IGN);

#ifdef HAVE_DLNA
    if (!getenv("client_enable_dms") || strcmp(getenv("client_enable_dms"), "y"))
#endif
    {
#ifndef DMS_CROSS_PLATFORMS
        /* Initialize Nexus Platform */
        rc = initNexusPlatform(ipStreamerGlobalCfg);
        if (rc) {BDBG_ERR(("Failed to initialize Nexus Platform")); goto error;}
#endif /* DMS_CROSS_PLATFORMS */
    }

    if ((ipStreamerGlobalCtx = (IpStreamerGlobalCtx *)BKNI_Malloc(sizeof(IpStreamerGlobalCtx))) == NULL) {BDBG_ERR(("BKMI_Malloc Failure at %d", __LINE__)); return NULL;}
    memset(ipStreamerGlobalCtx, 0, sizeof(IpStreamerGlobalCtx));
    ipStreamerGlobalCtx->globalCfg = *ipStreamerGlobalCfg;

    /* Initialize various libs */
    rc = initLibs();
    if (rc) {BDBG_ERR(("Failed to initialize libs ")); goto error;}
#ifndef DMS_CROSS_PLATFORMS
    /* initialize sources: qam */
    rc = initNexusQamSrcList(ipStreamerGlobalCtx);
    if (rc) {BDBG_ERR(("Failed to build Qam Src List")); goto error;}

    /* initialize sources: streamer */
    rc = initNexusStreamerSrcList(ipStreamerGlobalCtx);
    if (rc) {BDBG_ERR(("Failed to build Streamer Src List")); goto error;}

    /* initialize sources: sat */
    rc = initNexusSatSrcList(ipStreamerGlobalCtx);
    if (rc) {BDBG_ERR(("Failed to build Sat Src List")); goto error;}

    /* initialize sources: ofdm */
    rc = initNexusOfdmSrcList(ipStreamerGlobalCtx);
    if (rc) {BDBG_ERR(("Failed to build OFDM Src List")); goto error;}

    /* initialize sources: vsb */
    rc = initNexusVsbSrcList(ipStreamerGlobalCtx);
    if (rc) {BDBG_ERR(("Failed to build Vsb Src List")); goto error;}

    /* initialize sources: ip */
    rc = initNexusIpSrcList(ipStreamerGlobalCtx);
    if (rc) {BDBG_ERR(("Failed to build IP Src List")); goto error;}

#if defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT)
    rc = initNexusHdmiSrcList(ipStreamerGlobalCtx);
    if (rc) {BDBG_ERR(("Failed to build Hdmi Src List")); goto error;}
#endif /* defined(NEXUS_HAS_VIDEO_ENCODER) && defined(NEXUS_HAS_HDMI_INPUT) */

    /* initialize parser bands */
    rc = initNexusParserBandList(ipStreamerGlobalCtx);
    if (rc) {BDBG_ERR(("Failed to initialize NEXUS Parser bands ")); goto error;}

    /* initialize destination: ip */
    rc = initNexusIpDstList(ipStreamerGlobalCtx);
    if (rc) {BDBG_ERR(("Failed to build IP Dst List")); goto error;}

    /* initialize destination: record */
    rc = initNexusRecDstList(ipStreamerGlobalCtx);
    if (rc) {BDBG_ERR(("Failed to build Record Dst List")); goto error;}

#ifdef NEXUS_HAS_VIDEO_ENCODER
    /* initialize transcoding destination:  av backend for decoding/encoding */
    rc = initNexusTranscoderDstList(ipStreamerGlobalCtx);
    if (rc) {BDBG_ERR(("Failed to build Transcoder Dst List")); goto error;}
#endif

    /* initialize dtcp-ip library */
    rc = initDtcpIpLib(ipStreamerGlobalCtx);
    if (rc) {BDBG_ERR(("Failed to initialize DTCP/IP Library ")); goto error;}
#endif /* DMS_CROSS_PLATFORMS */
    BDBG_MSG(("%s done, ipStreamerGlobalCtx %p", BSTD_FUNCTION, (void *)ipStreamerGlobalCtx));
    return (void *)ipStreamerGlobalCtx;

error:
    B_IpStreamer_UnInit(ipStreamerGlobalCtx);
    return NULL;
}
