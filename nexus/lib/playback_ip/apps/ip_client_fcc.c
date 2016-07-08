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
 * Module Description: client example code to show fast channel change over IP
 *
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_primer.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_stc_channel.h"
#include "nexus_timebase.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_ir_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "bstd.h"
#include "bkni.h"
#include "bdbg.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "b_os_lib.h"
#include "b_playback_ip_lib.h"

#if 0
#undef NEXUS_HAS_SYNC_CHANNEL
#endif
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif

BDBG_MODULE(ip_client_fcc);

#define NUM_PROGRAMS 5
#define NUM_PRIMERS 5
#if 1
#define AUDIO
#endif

#define IP_NETWORK_JITTER       300
typedef struct
{
    char *ipAddr;
    int port;
    int protocol;
    char *uri;
} IpChannel;

#if 0
IpChannel ipChannelList[NUM_PROGRAMS] = {
    {"192.168.1.102", 5000, B_PlaybackIpProtocol_eHttp, "/LiveChannel;Type=QAM;QamMode=QAM256;Freq=111;SubChannel=1;StreamingEnabled=Yes"},
    {"192.168.1.102", 5000, B_PlaybackIpProtocol_eHttp, "/LiveChannel;Type=QAM;QamMode=QAM256;Freq=147;SubChannel=1;StreamingEnabled=Yes"},
    {"192.168.1.102", 5000, B_PlaybackIpProtocol_eHttp, "/LiveChannel;Type=QAM;QamMode=QAM256;Freq=549;SubChannel=1;StreamingEnabled=Yes"}
#if 0
    {"192.168.1.102", 5000, B_PlaybackIpProtocol_eHttp, "/LiveChannel;Type=QAM;QamMode=QAM256;Freq=561;SubChannel=1;StreamingEnabled=Yes"}
    {"192.168.1.102", 5000, B_PlaybackIpProtocol_eHttp, "/LiveChannel;Type=QAM;QamMode=QAM256;Freq=111;SubChannel=5;StreamingEnabled=Yes"},
    {"192.168.1.102", 5000, B_PlaybackIpProtocol_eHttp, "/LiveChannel;Type=QAM;QamMode=QAM256;Freq=111;SubChannel=2;StreamingEnabled=Yes"}
#endif
};
#else
IpChannel ipChannelList[NUM_PROGRAMS] = {
    {"192.168.1.102", 4321, B_PlaybackIpProtocol_eHttp, "/cds/ContentDir/?id=923"},
    {"192.168.1.102", 4321, B_PlaybackIpProtocol_eHttp, "/cds/ContentDir/?id=924"},
    {"192.168.1.102", 4321, B_PlaybackIpProtocol_eHttp, "/cds/ContentDir/?id=925"},
    {"192.168.1.102", 4321, B_PlaybackIpProtocol_eHttp, "/cds/ContentDir/?id=926"},
    {"192.168.1.102", 4321, B_PlaybackIpProtocol_eHttp, "/cds/ContentDir/?id=927"},
#if 0
    {"192.168.1.102", 4321, B_PlaybackIpProtocol_eHttp, "/cds/ContentDir/?id=926"}
    {"192.168.1.102", 5000, B_PlaybackIpProtocol_eHttp, "/LiveChannel;Type=QAM;QamMode=QAM256;Freq=111;SubChannel=5;StreamingEnabled=Yes"},
    {"192.168.1.102", 5000, B_PlaybackIpProtocol_eHttp, "/LiveChannel;Type=QAM;QamMode=QAM256;Freq=111;SubChannel=2;StreamingEnabled=Yes"}
#endif
};
#endif

typedef struct _ipSession {
    B_PlaybackIpHandle playbackIp;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;
    NEXUS_VideoDecoderPrimerHandle primer;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelHandle stcChannel;
    B_PlaybackIpPsiInfo psi;
}IpSession;
typedef struct _ipSession *IpSessionHandle;

typedef struct _decodeSession {
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_Timebase timebaseStc;
    bool timebaseStcConfigured;
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelHandle syncChannel;
#endif
}DecodeSession;
typedef struct _decodeSession *DecodeSessionHandle;


static void firstPtsCallback(void *context, int param);
static void firstPtsPassedCallback(void *context, int param);


/* Making these declarations global as otherwise Coverity keeps flagging this example app as large stack overuse. */
NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
NEXUS_PlaypumpOpenPidChannelSettings pidChannelSettings;
B_PlaybackIpSessionOpenSettings ipSessionOpenSettings;
B_PlaybackIpSessionOpenStatus ipSessionOpenStatus;
B_PlaybackIpSessionSetupSettings ipSessionSetupSettings;
B_PlaybackIpSessionSetupStatus ipSessionSetupStatus;
B_PlaybackIpSettings playbackIpSettings;
NEXUS_PlaypumpSettings playpumpSettings;
B_PlaybackIpSessionStartSettings ipSessionStartSettings;
B_PlaybackIpSessionStartStatus ipSessionStartStatus;
NEXUS_VideoDecoderPrimerSettings primerSettings;
NEXUS_TimebaseSettings timebaseSettings;
IpSessionHandle ipSession;
NEXUS_StcChannelSettings stcSettings;
NEXUS_VideoDecoderSettings videoDecoderSettings;

IpSessionHandle
IpPrimerSessionStart(int sessionIndex, IpChannel *ipChannel, DecodeSessionHandle decodeSession)
{
    NEXUS_Error rc;

    ipSession = BKNI_Malloc(sizeof(IpSession));
    BDBG_ASSERT(ipSession);
    BKNI_Memset(ipSession, 0, sizeof(IpSession));

    /* open playpump */
    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpumpOpenSettings.fifoSize *= 2;
    playpumpOpenSettings.numDescriptors = 200;
    ipSession->playpump = NEXUS_Playpump_Open(sessionIndex, &playpumpOpenSettings);
    BDBG_ASSERT(ipSession->playpump);
    BDBG_WRN(("Playpump Idx %d", sessionIndex));

    /* Open IP Session */
    ipSession->playbackIp = B_PlaybackIp_Open(NULL);
    BDBG_ASSERT(ipSession->playbackIp);

    BKNI_Memset(&ipSessionOpenSettings, 0, sizeof(ipSessionOpenSettings));
    BKNI_Memset(&ipSessionOpenStatus, 0, sizeof(ipSessionOpenStatus));
    strncpy(ipSessionOpenSettings.socketOpenSettings.ipAddr, ipChannel->ipAddr, sizeof(ipSessionOpenSettings.socketOpenSettings.ipAddr)-1);
    ipSessionOpenSettings.socketOpenSettings.port = ipChannel->port;
    ipSessionOpenSettings.socketOpenSettings.protocol = ipChannel->protocol;
    ipSessionOpenSettings.socketOpenSettings.url = ipChannel->uri;
    ipSessionOpenSettings.ipMode = B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip;
    ipSessionOpenSettings.maxNetworkJitter = 300;
    ipSessionOpenSettings.networkTimeout = 1;  /* timeout in 1 sec during network outage events */
    rc = B_PlaybackIp_SessionOpen(ipSession->playbackIp, &ipSessionOpenSettings, &ipSessionOpenStatus);
    BDBG_ASSERT(!rc);

    /* do IP Session Setup so that PIDs are known */
    memset(&ipSessionSetupSettings, 0, sizeof(ipSessionSetupSettings));
    rc = B_PlaybackIp_SessionSetup(ipSession->playbackIp, &ipSessionSetupSettings, &ipSessionSetupStatus);
    BDBG_ASSERT(!rc);

    if(ipChannel->protocol == B_PlaybackIpProtocol_eUdp
        || ipChannel->protocol == B_PlaybackIpProtocol_eRtpNoRtcp
        || ipChannel->protocol == B_PlaybackIpProtocol_eRtp){
        ipSession->psi = ipSessionSetupStatus.u.udp.psi;
    }
    else if(ipChannel->protocol == B_PlaybackIpProtocol_eHttp
        || ipChannel->protocol == B_PlaybackIpProtocol_eHttps){
    ipSession->psi = ipSessionSetupStatus.u.http.psi;
    }
    else if(ipChannel->protocol == B_PlaybackIpProtocol_eRtsp){
        ipSession->psi = ipSessionSetupStatus.u.rtsp.psi;
    }

    BDBG_WRN(("vpid %d, vcodec %d, apid %d, acodec %d, psiPid %d", ipSession->psi.videoPid, ipSession->psi.videoCodec, ipSession->psi.audioPid, ipSession->psi.audioCodec, ipSession->psi.pcrPid));

    NEXUS_Playpump_GetSettings(ipSession->playpump, &playpumpSettings);
    playpumpSettings.transportType = ipSession->psi.mpegType;
    rc = NEXUS_Playpump_SetSettings(ipSession->playpump, &playpumpSettings);
    BDBG_ASSERT(!rc);

    /* open AV pid channels */
    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
    pidChannelSettings.pidType = NEXUS_PidType_eVideo;
    ipSession->videoPidChannel = NEXUS_Playpump_OpenPidChannel(ipSession->playpump, ipSession->psi.videoPid, &pidChannelSettings);
    BDBG_ASSERT(ipSession->videoPidChannel != NULL);

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
    pidChannelSettings.pidType = NEXUS_PidType_eAudio;
    pidChannelSettings.pidTypeSettings.audio.codec = ipSession->psi.audioCodec;
    ipSession->audioPidChannel = NEXUS_Playpump_OpenPidChannel(ipSession->playpump, ipSession->psi.audioPid, &pidChannelSettings);
    BDBG_ASSERT(ipSession->audioPidChannel != NULL);

    if (ipSession->psi.pcrPid == ipSession->psi.videoPid)
        ipSession->pcrPidChannel = ipSession->videoPidChannel;
    else if (ipSession->psi.pcrPid == ipSession->psi.audioPid)
        ipSession->pcrPidChannel = ipSession->audioPidChannel;
    else {
        NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
        pidChannelSettings.pidType = NEXUS_PidType_eOther;
        ipSession->pcrPidChannel = NEXUS_Playpump_OpenPidChannel(ipSession->playpump, ipSession->psi.pcrPid, &pidChannelSettings);
        BDBG_ASSERT(ipSession->pcrPidChannel != NULL);
    }

    /* configured the timebase */
    /* note: same timebase is used for all priming and the main decoding session */
    /* we configure it once here for the 1st priming session */
    /* later when the decode is started using this priming session, timebase is configured */
    /* again to include the pidChannel of the main decoding session */
    if (decodeSession->timebaseStcConfigured == false) {
        NEXUS_Timebase_GetSettings(decodeSession->timebaseStc, &timebaseSettings);
        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
        timebaseSettings.freeze = false;
        timebaseSettings.sourceSettings.pcr.maxPcrError = IP_NETWORK_JITTER * 183/2;    /* in milliseconds: based on 90Khz clock */
        timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e244ppm;
        timebaseSettings.sourceSettings.pcr.pidChannel = ipSession->pcrPidChannel;
        rc = NEXUS_Timebase_SetSettings(decodeSession->timebaseStc, &timebaseSettings);
        BDBG_ASSERT(!rc);
        decodeSession->timebaseStcConfigured = true;
    }

    /* configure STC channel: each priming session needs its own STC channel */
    /* all STC use the same Timebase */
    NEXUS_StcChannel_GetDefaultSettings(sessionIndex, &stcSettings);
    stcSettings.timebase = decodeSession->timebaseStc;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr;
    stcSettings.autoConfigTimebase = false;
    stcSettings.modeSettings.pcr.offsetThreshold = IP_NETWORK_JITTER * 183;
    stcSettings.modeSettings.pcr.maxPcrError = IP_NETWORK_JITTER * 183;    /* in milliseconds: based on 90Khz clock */
    stcSettings.modeSettings.pcr.disableJitterAdjustment = true;
    stcSettings.modeSettings.pcr.disableTimestampCorrection = true;
    stcSettings.modeSettings.pcr.pidChannel = ipSession->pcrPidChannel;
    stcSettings.stcIndex = 0;
    ipSession->stcChannel = NEXUS_StcChannel_Open(sessionIndex, &stcSettings);
    BDBG_WRN(("stc ch %d, ptr %p", sessionIndex, (void *)ipSession->stcChannel));
    BDBG_ASSERT(ipSession->stcChannel != NULL);

    /* setup the AV decoder start settings */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&ipSession->videoProgram);
    ipSession->videoProgram.codec = ipSession->psi.videoCodec;
    ipSession->videoProgram.pidChannel = ipSession->videoPidChannel;
    ipSession->videoProgram.stcChannel = ipSession->stcChannel;

#ifdef AUDIO
    NEXUS_AudioDecoder_GetDefaultStartSettings(&ipSession->audioProgram);
    ipSession->audioProgram.codec = ipSession->psi.audioCodec;
    ipSession->audioProgram.pidChannel = ipSession->audioPidChannel;
    ipSession->audioProgram.stcChannel = ipSession->stcChannel;
#endif

    /* Start playpump */
    rc = NEXUS_Playpump_Start(ipSession->playpump);
    BDBG_ASSERT(!rc);

    ipSession->primer = NEXUS_VideoDecoderPrimer_Open(decodeSession->videoDecoder);
    BDBG_ASSERT(ipSession->primer != NULL);
    NEXUS_VideoDecoderPrimer_GetSettings(ipSession->primer, &primerSettings);
    primerSettings.pastTolerance = 1500; /* amount of time willing to race */
    primerSettings.futureTolerance = 0;
    primerSettings.ptsStcDiffCorrectionEnabled = true;
    rc = NEXUS_VideoDecoderPrimer_SetSettings(ipSession->primer, &primerSettings);
    BDBG_ASSERT(!rc);
    /* start priming */
    rc = NEXUS_VideoDecoderPrimer_Start(ipSession->primer, &ipSession->videoProgram);
    BDBG_ASSERT(!rc);
    BDBG_WRN(("############## primer %p started: pastTolerance=%d, futureTolerance=%d\n", (void *)ipSession->primer, primerSettings.pastTolerance, primerSettings.futureTolerance));

    /* start IP session: all other modules (playpump, decoder, primer) must be started before this */
    /* update IP Session Settings */
    B_PlaybackIp_GetSettings(ipSession->playbackIp, &playbackIpSettings);
    playbackIpSettings.ipMode = B_PlaybackIpClockRecoveryMode_ePushWithPcrSyncSlip;
    /* coverity[stack_use_local_overflow] */
    /* coverity[stack_use_overflow] */
    rc = B_PlaybackIp_SetSettings(ipSession->playbackIp, &playbackIpSettings);
    BDBG_ASSERT(!rc);

    NEXUS_VideoDecoder_GetSettings(decodeSession->videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.firstPts.callback = firstPtsCallback;
    videoDecoderSettings.firstPts.context = decodeSession->videoDecoder;
    videoDecoderSettings.firstPtsPassed.callback = firstPtsPassedCallback;
    videoDecoderSettings.firstPtsPassed.context = decodeSession->videoDecoder;
    videoDecoderSettings.sourceChanged.callback = NULL;
    rc = NEXUS_VideoDecoder_SetSettings(decodeSession->videoDecoder, &videoDecoderSettings);
    BDBG_ASSERT(!rc);

    BKNI_Memset(&ipSessionStartSettings, 0, sizeof(ipSessionStartSettings));
    ipSessionStartSettings.nexusHandles.playpump = ipSession->playpump;
    ipSessionStartSettings.nexusHandles.videoDecoder = decodeSession->videoDecoder;
    ipSessionStartSettings.nexusHandles.stcChannel = ipSession->stcChannel;
    ipSessionStartSettings.nexusHandles.primaryAudioDecoder = decodeSession->audioDecoder;
    ipSessionStartSettings.nexusHandlesValid = true;
    rc = B_PlaybackIp_SessionStart(ipSession->playbackIp, &ipSessionStartSettings, &ipSessionStartStatus);
    BDBG_ASSERT(!rc);

    return ipSession;
}

void
IpPrimerSessionStop(IpSessionHandle ipSession, DecodeSessionHandle decodeSession)
{
    NEXUS_Error rc;
    NEXUS_VideoDecoderSettings videoDecoderSettings;

    rc = B_PlaybackIp_SessionStop(ipSession->playbackIp);
    BDBG_ASSERT(!rc);

    NEXUS_VideoDecoder_GetSettings(decodeSession->videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.firstPts.callback = NULL;
    videoDecoderSettings.firstPtsPassed.callback = NULL;
    videoDecoderSettings.sourceChanged.callback = NULL;
    rc = NEXUS_VideoDecoder_SetSettings(decodeSession->videoDecoder, &videoDecoderSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Playpump_Stop(ipSession->playpump);

    NEXUS_VideoDecoderPrimer_Stop(ipSession->primer);

    BDBG_WRN(("############## primer %p session stopped", (void *)ipSession->primer));
    NEXUS_VideoDecoderPrimer_Close(ipSession->primer);

    NEXUS_Playpump_CloseAllPidChannels(ipSession->playpump);
    NEXUS_Playpump_Close(ipSession->playpump);

    rc = B_PlaybackIp_SessionClose(ipSession->playbackIp);
    BDBG_ASSERT(!rc);
    B_PlaybackIp_Close(ipSession->playbackIp);

    NEXUS_StcChannel_Close(ipSession->stcChannel);
    BKNI_Free(ipSession);
}

int
IpSessionStartDecodeWithPrimer(IpSessionHandle ipSession, DecodeSessionHandle decodeSession)
{
    NEXUS_Error rc;
    NEXUS_TimebaseSettings timebaseSettings;
    /* update timebase to use the PCR from this decode session */
    NEXUS_Timebase_GetSettings(decodeSession->timebaseStc, &timebaseSettings);
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_ePcr;
    timebaseSettings.freeze = false;
    timebaseSettings.sourceSettings.pcr.maxPcrError = IP_NETWORK_JITTER * 183/2;    /* in milliseconds: based on 90Khz clock */
    timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e244ppm;
    timebaseSettings.sourceSettings.pcr.pidChannel = ipSession->videoProgram.pidChannel;
    rc = NEXUS_Timebase_SetSettings(decodeSession->timebaseStc, &timebaseSettings);
    BDBG_ASSERT(!rc);

    /* now start the decode with priming channel */
    rc = NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode(ipSession->primer,decodeSession->videoDecoder);
    BDBG_ASSERT(!rc);

#ifdef AUDIO
    rc = NEXUS_AudioDecoder_Start(decodeSession->audioDecoder, &ipSession->audioProgram);
    BDBG_ASSERT(!rc);
#endif

    BDBG_WRN((">>>>>>>>>>>>>>>>> starting program with primer: vpid 0x%x, apid 0x%x", ipSession->psi.videoPid, ipSession->psi.audioPid));
    return 0;
}

void IpsClose(IpSessionHandle ips)
{
    /* TODO: should we have an IpsStop() ? */
    if (ips->playbackIp) {
        B_PlaybackIp_SessionStop(ips->playbackIp);
        B_PlaybackIp_Close(ips->playbackIp);
        ips->playbackIp = NULL;
    }
    NEXUS_Playpump_Stop(ips->playpump);
    NEXUS_Playpump_ClosePidChannel(ips->playpump, ips->videoPidChannel);
    NEXUS_Playpump_Close(ips->playpump);
    BKNI_Free(ips);
}

static void
firstPtsCallback(void *context, int param)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)context;
    BSTD_UNUSED(param);
    if (videoDecoder) {
        NEXUS_VideoDecoderStatus status;
        if (NEXUS_VideoDecoder_GetStatus(videoDecoder, &status) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to get current vidoe currentPts\n", __FUNCTION__));
            return;
        }
        BDBG_WRN(("%s: first decoded pts 0x%x, ptsStcDifference 0x%x, type %d, tsm %d", __FUNCTION__, status.pts, status.ptsStcDifference, status.ptsType, status.tsm));
    }
    else {
        BDBG_WRN(("NOTHING >>>>>>>>>>>>>>>>>>>>"));
    }
}

static void
firstPtsPassedCallback(void *context, int param)
{
    BSTD_UNUSED(param);
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)context;
    if (videoDecoder) {
        NEXUS_VideoDecoderStatus status;
        if (NEXUS_VideoDecoder_GetStatus(videoDecoder, &status) != NEXUS_SUCCESS) {
            BDBG_ERR(("%s: Failed to get current vidoe currentPts\n", __FUNCTION__));
            return;
        }
        BDBG_WRN(("%s: first passed pts 0x%x, ptsStcDifference 0x%x, type %d, tsm %d\n", __FUNCTION__, status.pts, status.ptsStcDifference, status.ptsType, status.tsm));
    }
    else
        BDBG_WRN(("NOTHING >>>>>>>>>>>>>>>>>>>>"));
}


DecodeSessionHandle
decodeSessionInit(void)
{
    DecodeSessionHandle decodeSession = NULL;
    NEXUS_TimebaseSettings timebaseSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_Error rc;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    NEXUS_AudioDecoderSettings audioDecoderSettings;
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif
    NEXUS_AudioOutputSettings audioOutputSettings;
    NEXUS_AudioOutput audioOutput;
    NEXUS_Timebase timebaseOutput;

    NEXUS_Platform_GetConfiguration(&platformConfig);

    decodeSession = BKNI_Malloc(sizeof(DecodeSession));
    BDBG_ASSERT(decodeSession);
    BKNI_Memset(decodeSession, 0, sizeof(DecodeSession));

#if NEXUS_HAS_SYNC_CHANNEL
    /* create a sync channel */
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    syncChannelSettings.enablePrecisionLipsync = true;
    decodeSession->syncChannel = NEXUS_SyncChannel_Create(&syncChannelSettings);
    if (decodeSession->syncChannel == NULL) {printf("NEXUS Error at %d, Exiting...\n", __LINE__); exit(1);}
#endif

#ifdef AUDIO
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    audioDecoderOpenSettings.fifoSize = 512 * 1024; /* increased CDB for dejitter buffer */
    decodeSession->audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
    BDBG_ASSERT(decodeSession->audioDecoder);
    #if NEXUS_NUM_AUDIO_DACS
    rc = NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(decodeSession->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    BDBG_ASSERT(!rc);
    #endif
#if NEXUS_NUM_HDMI_OUTPUTS
    rc = NEXUS_AudioOutput_AddInput(
            NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
            NEXUS_AudioDecoder_GetConnector(decodeSession->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    BDBG_ASSERT(!rc);
#endif
#endif

    /* Bring up display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e1080i;
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    decodeSession->display = NEXUS_Display_Open(0, &displaySettings);
    BDBG_ASSERT(decodeSession->display);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    rc = NEXUS_Display_AddOutput(decodeSession->display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    BDBG_ASSERT(!rc);
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    rc = NEXUS_Display_AddOutput(decodeSession->display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    BDBG_ASSERT(!rc);
#endif

    decodeSession->window = NEXUS_VideoWindow_Open(decodeSession->display, 0);
    BDBG_ASSERT(decodeSession->window);
    /* Bring up decoder and connect to display */
    NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
    /* Increase the CDB size as it is used as the dejitter buffer */
    videoDecoderOpenSettings.fifoSize = 10 * 1024 * 1024;
    decodeSession->videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
    rc = NEXUS_VideoWindow_AddInput(decodeSession->window, NEXUS_VideoDecoder_GetConnector(decodeSession->videoDecoder));
    BDBG_ASSERT(!rc);

    NEXUS_VideoDecoder_GetSettings(decodeSession->videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.firstPts.callback = firstPtsCallback;
    videoDecoderSettings.firstPts.context = decodeSession->videoDecoder;
    videoDecoderSettings.firstPtsPassed.callback = firstPtsPassedCallback;
    videoDecoderSettings.firstPtsPassed.context = decodeSession->videoDecoder;
    videoDecoderSettings.ptsOffset = IP_NETWORK_JITTER * 45; /* in 45Khz clock */
    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
    rc = NEXUS_VideoDecoder_SetSettings(decodeSession->videoDecoder, &videoDecoderSettings);
    BDBG_ASSERT(!rc);

#ifdef AUDIO
    NEXUS_AudioDecoder_GetSettings(decodeSession->audioDecoder, &audioDecoderSettings);
    audioDecoderSettings.ptsOffset = IP_NETWORK_JITTER * 45;
    rc = NEXUS_AudioDecoder_SetSettings(decodeSession->audioDecoder, &audioDecoderSettings);
    BDBG_ASSERT(!rc);
#endif

#if NEXUS_HAS_SYNC_CHANNEL
    /* connect sync channel */
    NEXUS_SyncChannel_GetSettings(decodeSession->syncChannel, &syncChannelSettings);
    syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(decodeSession->videoDecoder);
    syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(decodeSession->audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);
    rc = NEXUS_SyncChannel_SetSettings(decodeSession->syncChannel, &syncChannelSettings);
    BDBG_ASSERT(!rc);
    BDBG_WRN(("SYNC Channel Settings are updated...."));
#endif

    /* one timebase (DPCR) block is shared for all priming & current decoding session */
    decodeSession->timebaseStc = NEXUS_Timebase_e0;
    timebaseOutput = NEXUS_Timebase_e1;

    /* configure output timebase to freerun */
    NEXUS_Timebase_GetSettings(timebaseOutput, &timebaseSettings);
    timebaseSettings.freeze = true;
    timebaseSettings.sourceSettings.pcr.trackRange = NEXUS_TimebaseTrackRange_e61ppm;
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
    rc = NEXUS_Timebase_SetSettings(timebaseOutput, &timebaseSettings);
    BDBG_ASSERT(!rc);

#ifdef AUDIO
    /* decouple display and audio output from input timebase */
    #if NEXUS_NUM_AUDIO_DACS
    audioOutput = NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]);
    NEXUS_AudioOutput_GetSettings(audioOutput, &audioOutputSettings);
    audioOutputSettings.timebase = timebaseOutput;
    rc = NEXUS_AudioOutput_SetSettings(audioOutput, &audioOutputSettings);
    BDBG_ASSERT(!rc);
    #endif
#if NEXUS_NUM_HDMI_OUTPUTS
    audioOutput = NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]);
    NEXUS_AudioOutput_GetSettings(audioOutput, &audioOutputSettings);
    audioOutputSettings.timebase = timebaseOutput;
    rc = NEXUS_AudioOutput_SetSettings(audioOutput, &audioOutputSettings);
    BDBG_ASSERT(!rc);
#endif
#endif

    NEXUS_Display_GetSettings(decodeSession->display, &displaySettings);
    displaySettings.timebase = timebaseOutput;
    rc = NEXUS_Display_SetSettings(decodeSession->display, &displaySettings);
    BDBG_ASSERT(!rc);

    return decodeSession;
}

void
IpStopDecoders(DecodeSessionHandle decodeSession)
{
#ifdef AUDIO
        NEXUS_AudioDecoder_Stop(decodeSession->audioDecoder);
#endif
        NEXUS_VideoDecoder_Stop(decodeSession->videoDecoder);
}

int main(int argc, char *argv[])
{
    unsigned nextProgramToDecode; /* next program that should be decoded */
    unsigned nextPrimerToDecode; /* next priming session that should be decoded */
    unsigned nextPrimerToUse; /* next primer index available for priming */
    unsigned nextProgramToPrime; /* next program to prime */
    NEXUS_Error rc;
    IpSessionHandle ipSessionList[NUM_PRIMERS];
    DecodeSessionHandle decodeSession = NULL;
    unsigned maxPrimers = NUM_PRIMERS;

    if (NEXUS_Platform_Init(NULL) != NEXUS_SUCCESS)
        BDBG_ASSERT(NULL);

    B_Os_Init();

    decodeSession = decodeSessionInit();
    BDBG_ASSERT(decodeSession);

    if (!((argc > 1) && argv[1] && (maxPrimers = atoi(argv[1]))<=NUM_PRIMERS))
    {
        maxPrimers = NUM_PRIMERS;
    }

    if((maxPrimers > 0) && (maxPrimers <= NUM_PRIMERS)) {
        for ( nextProgramToPrime = 0, nextPrimerToUse=0;
              nextPrimerToUse < maxPrimers;
              nextProgramToPrime++, nextPrimerToUse++)
        {
            ipSessionList[nextPrimerToUse] = IpPrimerSessionStart(nextPrimerToUse, &ipChannelList[nextProgramToPrime], decodeSession);
        }
    /* start decode on priming session 0 */
    nextProgramToDecode = 0;
    rc = IpSessionStartDecodeWithPrimer(ipSessionList[nextProgramToDecode], decodeSession);
    BDBG_ASSERT(!rc);

    /* since 1st priming session is now being decoded and all primers were being initially used, so next available primer is 0 */
    nextPrimerToUse = 0;
    nextPrimerToDecode = 1;
    nextProgramToPrime = maxPrimers % NUM_PROGRAMS;
    nextProgramToDecode = 1;

    while (1)
    {
        int ch;
#if 0
        BDBG_WRN(("Switching Channels every 4 sec\n"));
        BKNI_Sleep(4000);
#else
        BDBG_WRN(("Press Enter to Channel Up............"));
        ch = getchar();
        BSTD_UNUSED(ch);
#endif
        BDBG_WRN((">>>>>>>>>>>>>>>>> starting program %d decode with primer id %d: video pid 0x%x, audio pid 0x%x", nextProgramToDecode, nextPrimerToDecode, ipSessionList[nextPrimerToDecode]->psi.videoPid, ipSessionList[nextPrimerToDecode]->psi.audioPid));
        /* stop the current decoders */
        IpStopDecoders(decodeSession);

        /* start next decode session using next primer */
        rc = IpSessionStartDecodeWithPrimer(ipSessionList[nextPrimerToDecode], decodeSession);
        BDBG_ASSERT(!rc);
        if (++nextPrimerToDecode == maxPrimers) nextPrimerToDecode = 0;

        /* stop/close the unused primer before we can reuse it */
        IpPrimerSessionStop(ipSessionList[nextPrimerToUse], decodeSession);

#if 0
        BDBG_WRN(("sleeping extra 50 msec"));
        BKNI_Sleep(500);
#endif
        /* start priming next program on this primer now */
        ipSessionList[nextPrimerToUse] = IpPrimerSessionStart(nextPrimerToUse, &ipChannelList[nextProgramToPrime], decodeSession);

        if (++nextPrimerToUse == maxPrimers) nextPrimerToUse = 0;
        if (++nextProgramToPrime == NUM_PROGRAMS) nextProgramToPrime = 0;
        if (++nextProgramToDecode == NUM_PROGRAMS) nextProgramToDecode = 0;
    }

    IpsClose(ipSessionList[0]);
    }
    return 0;
}
