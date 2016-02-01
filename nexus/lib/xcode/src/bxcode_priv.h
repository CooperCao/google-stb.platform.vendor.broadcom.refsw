/***************************************************************************
 *     (c)2010-2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/
#ifndef BXCODE_PRIV_H__
#define BXCODE_PRIV_H__

#include "bcmplayer.h"
#include "tshdrbuilder.h"
#include "bdbg.h"
#include <stdio.h>
#include "b_os_lib.h"
#include "bfifo.h"
#include "bxcode.h"
#include "nexus_audio_mixer.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "nexus_file_fifo.h"
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "nexus_file_mux.h"
#include "nexus_stream_mux.h"
#endif
#ifdef NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif

/*-**************
Private API for bxcode.
****************/
#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(bxcode);

/*-**************
Private types, structs for bxcode.
****************/
/* 16 BXCode_P_OutputDescriptor FIFO */
#define BXCODE_P_OUTPUT_DESC_FIFO_SIZE     16
/* assume each segment/psi pair requires at least 2 output descriptors */
#define BXCODE_P_PSI_QUEUE_CNT             (BXCODE_P_OUTPUT_DESC_FIFO_SIZE/2)
#define BXCODE_P_TSPKT_LENGTH               188

/* FNRT chunk */
typedef struct BXCode_P_VideoChunk{
    uint32_t                   id;
    long                       startRapIndex;
    long                       endRapIndex;
    BNAV_Player_Position       startPosition;
    BNAV_Player_Position       endPosition;
    off_t                      startOffset;
    off_t                      endOffset;
} BXCode_P_VideoChunk;

/* video transcoder resource */
typedef struct BXCode_P_Video{
    unsigned                   id, bxcodeId;
    unsigned                   decoderId;

#ifdef NEXUS_HAS_VIDEO_ENCODER
    /* decoder/encoder handles */
    NEXUS_VideoEncoderHandle   encoder;
    NEXUS_DisplayHandle        display;
    NEXUS_VideoWindowHandle    window;
    NEXUS_StcChannelHandle     stcChannel;
    NEXUS_PlaypumpHandle       playpump;
    NEXUS_PidChannelHandle     pidChannel;
    NEXUS_VideoDecoderHandle   decoder;
    NEXUS_VideoImageInputHandle imageInput;

    /* FNRT context */
    BKNI_EventHandle           dataReadyEvent;
    BKNI_EventHandle           chunkDoneEvent;
    B_ThreadHandle             playpumpThread;
    B_MutexHandle              mutexStarted;
    int                        fd;
    NEXUS_VideoDecoderStartSettings   vidProgram;
    BXCode_P_VideoChunk        chunk;
    BXCode_P_VideoChunk        prevChunk;
    off_t                      fileOffset;

    /* internal state */
    bool                       started;
    void                      *pEncoderBufferBase; /* encoder output buffer base address */
    void                      *pEncoderMetadataBufferBase; /* encoder output metadata buffer base address */
#endif
} BXCode_P_Video;

/* audio transcoder resource */
typedef struct BXCode_P_Audio{
    unsigned                   id, bxcodeId;

#ifdef NEXUS_HAS_AUDIO_MUX_OUTPUT
    /* decoder/encoder handles */
    NEXUS_AudioMixerHandle     mixer;
    NEXUS_AudioEncoderHandle   encoder;
    NEXUS_AudioMuxOutputHandle muxOutput;
    NEXUS_StcChannelHandle     stcChannel;
    NEXUS_PlaypumpHandle       playpump;
    NEXUS_PidChannelHandle     pidChannel;
    NEXUS_AudioDecoderHandle   decoder;
    B_MutexHandle              mutexStarted;
    bool                       started;
    void                      *pEncoderBufferBase; /* encoder output buffer base address */
    void                      *pEncoderMetadataBufferBase; /* encoder output metadata buffer base address */

    /* FNRT context */
    BKNI_EventHandle           dataReadyEvent;
    B_ThreadHandle             playpumpThread;
    NEXUS_AudioDecoderStartSettings   audProgram;
    NEXUS_AudioMuxOutputStartSettings audioMuxStartSettings;
    NEXUS_AudioMuxOutputDelayStatus audioDelayStatus;
    int                        fd;
#endif
} BXCode_P_Audio;

typedef struct BXCode_P_PsiMessage{
        unsigned short         pid;
        NEXUS_MessageHandle    message;
        NEXUS_PidChannelHandle pidChannel;
        bool                   done;
} BXCode_P_PsiMessage;

/* BXCode context:
   Internally manage source playpump, playback, decoder, display, window, encoder, mux, recpump, record,
   sync channel, stc, pid channel etc orthorgonal system resources.
 */
typedef struct BXCode_P_Context {
    BDBG_OBJECT(bxcode)

    unsigned                   id;

#ifdef NEXUS_HAS_VIDEO_ENCODER
    /* bxcode settings */
    BXCode_OpenSettings        openSettings;
    BXCode_StartSettings       startSettings;
    BXCode_Settings            settings;

    /* a/v xcoder handles */
    BXCode_P_Video             video[NEXUS_MAX_VIDEO_ENCODERS];/* FNRT video pipes */
    BXCode_P_Audio             audio[BXCODE_MAX_AUDIO_PIDS];/* 6x audio PIDs passthru */
    unsigned                   numAudios;
    unsigned                   audioDummyId, audioDummyId2, audioDummyRefCnt; /* 6xaudio requires two dummy outputs */
    NEXUS_AudioMixerHandle     hwMixer; /* for 6xaudio */
    NEXUS_StcChannelHandle     stcChannelDecoder, stcChannelEncoder; /* only set in RT mode */
    BKNI_EventHandle           finishEvent;
    NEXUS_VideoEncoderDelayRange videoDelay;

    /* mp4 file mux handles/settings */
    NEXUS_FileMuxStartSettings mp4MuxConfig;
    NEXUS_FileMuxHandle        fileMux;
    NEXUS_MuxFileHandle        fileMuxOutput;

    /* ts stream mux/record handles/settings */
    NEXUS_PlaypumpHandle       playpumpMuxVideo;
    NEXUS_PlaypumpHandle       playpumpMuxAudio[BXCODE_MAX_AUDIO_PIDS];
    NEXUS_PlaypumpHandle       playpumpMuxSystem;
    NEXUS_StreamMuxHandle      streamMux;
    BKNI_EventHandle           recpumpEvent;
    int                        fdTsRec;
    FILE                      *fpIndexRec;
    B_ThreadHandle             recpumpThread;
    NEXUS_FifoRecordHandle     fifoRecord;
    NEXUS_FileRecordHandle     fileRecord;
    NEXUS_RecpumpHandle        recpump;
    NEXUS_RecordHandle         record;
    NEXUS_PidChannelHandle     pidChannelMuxVideo;
    NEXUS_PidChannelHandle     pidChannelMuxAudio[BXCODE_MAX_AUDIO_PIDS];
    NEXUS_PidChannelHandle     pidChannelMuxPcr;
    NEXUS_PidChannelHandle     pidChannelMuxPat;
    NEXUS_PidChannelHandle     pidChannelMuxPmt;
    NEXUS_MessageHandle        userDataMessage[NEXUS_MAX_MUX_PIDS];
    NEXUS_PidChannelHandle     pidChannelUserData[NEXUS_MAX_MUX_PIDS];
    NEXUS_PidChannelHandle     pidChannelMuxUserData[NEXUS_MAX_MUX_PIDS];
    NEXUS_StreamMuxStartSettings tsMuxConfig;
    /* segmented ts stream output descriptors */
    BFIFO_HEAD(OutputDescriptorFifo, BXCode_OutputTsDescriptor) outputDescFifo;
    BXCode_OutputTsDescriptor  outputDescs[BXCODE_P_OUTPUT_DESC_FIFO_SIZE];
    bool                       firstRai, firstRaiSeen;
    unsigned                   segmentsStartConsumed;
    void                      *lastIndexWp;
    void                      *lastDataWp;
    off_t                      totalRecordBytes;

    /* source handles */
#if NEXUS_HAS_HDMI_INPUT
    NEXUS_HdmiInputHandle      hdmiInput;
#endif
    NEXUS_FilePlayHandle       file;
    BKNI_EventHandle           dataReadyEvent;
    NEXUS_PlaypumpHandle       playpump;
    NEXUS_PlaybackHandle       playback;
    NEXUS_PidChannelHandle     pcrPidChannel;
    NEXUS_SyncChannelHandle    syncChannel;
    FILE                      *fpIndex;
    BNAV_Player_Handle         bcmplayer;

    /* FNRT variables */
    int                        videoXcoderId;
    int                        audioXcoderId;
    int                        activeXcoders;
    B_MutexHandle              mutexActiveXcoders;
    BXCode_P_VideoChunk        latestVideoChunk;
    B_MutexHandle              mutexChunk;

    /* PSI system data generation for TS mux */
    void                      *psiPkt;
    unsigned                   ccValue;
    NEXUS_StreamMuxSystemData  psi[2];
    B_MutexHandle              mutexSystemdata;
    B_SchedulerHandle          schedulerSystemdata;
    B_SchedulerTimerId         systemdataTimer;
    B_ThreadHandle             schedulerThread;
    bool                       systemdataTimerIsStarted;

    /* TS layer user data */
    unsigned                   numUserDataPids;
    bool                       userDataStreamValid[NEXUS_MAX_MUX_PIDS];
    unsigned                   userDataDescLen[NEXUS_MAX_MUX_PIDS];
    uint8_t                    userDataDescriptors[NEXUS_MAX_MUX_PIDS][188];
    TS_PMT_stream              userDataStream[NEXUS_MAX_MUX_PIDS];
    bool                       remapUserDataPid;
    BXCode_P_PsiMessage        psi_message[NEXUS_MAX_MUX_PIDS];

    /* xcode context level state */
    bool                       nonRealTime;
    bool                       lowDelay;
    BKNI_EventHandle           eofEvent;
    B_ThreadHandle             nrtEofHandle;
    B_MutexHandle              mutexStarted;
    bool                       started;
    bool                       toStop;
    NEXUS_Timebase             timebase;
#endif
} BXCode_P_Context;

/* system level global state */
typedef struct BXCode_P_State
{
    BXCode_Handle              handles[NEXUS_MAX_VIDEO_ENCODERS];
    /* bit masks to manage opened resources */
    unsigned                   openVideoDecoders;
    /* bit masks to manage used audio dummy outputs */
    unsigned                   usedAudioDummyOutputs;
} BXCode_P_State;

extern BXCode_P_State g_BXCode_P_State;

/*-**************
Private functions for bxcode.
****************/
void bxcode_init(BXCode_P_Context  *xcodeContext);
NEXUS_Error bxcode_open_video_transcode(BXCode_P_Context *xcodeContext, unsigned id);
void bxcode_close_video_transcode(BXCode_P_Context *xcodeContext, unsigned id);
void bxcode_stop_video_transcode(BXCode_P_Context *xcodeContext, unsigned id);
void bxcode_start_video_transcode(BXCode_P_Context *xcodeContext, unsigned id);
NEXUS_Error bxcode_open_audio_transcode(BXCode_P_Context *xcodeContext, unsigned id);
void bxcode_close_audio_transcode(BXCode_P_Context *xcodeContext, unsigned id);
void bxcode_stop_audio_transcode(BXCode_P_Context *xcodeContext, unsigned id);
void bxcode_start_audio_transcode(BXCode_P_Context *xcodeContext, unsigned id);
NEXUS_Error bxcode_open_mp4mux(	BXCode_P_Context *pContext );
NEXUS_Error bxcode_open_tsmux(BXCode_P_Context  *pContext );
NEXUS_Error bxcode_open_audio_mux(BXCode_P_Context  *pContext , unsigned id);
void bxcode_start_mux(BXCode_P_Context  *pContext );
void bxcode_finish_mux(	BXCode_P_Context  *pContext );
void bxcode_stop_mux(BXCode_P_Context  *pContext );
void bxcode_p_start_input(BXCode_P_Context  *bxcode);
void bxcode_p_stop_input(BXCode_P_Context  *bxcode);
void bxcode_p_post_start(BXCode_P_Context  *bxcode);
void bxcode_p_pre_stop(BXCode_P_Context  *bxcode);
void bxcode_p_pre_start(BXCode_P_Context  *bxcode);
void BXCode_P_UpdateSystemData(BXCode_P_Context *bxcode);
NEXUS_Error BXCode_P_SetVideoSettings(BXCode_P_Context *pContext, unsigned i, const BXCode_Settings *pSettings);
NEXUS_Error BXCode_P_SetAudioSettings(BXCode_P_Context *pContext, unsigned i, const BXCode_Settings *pSettings);

#ifdef __cplusplus
}
#endif

#endif
