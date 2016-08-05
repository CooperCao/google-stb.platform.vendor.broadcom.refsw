/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 *****************************************************************************/

#ifndef STANDBY_H__
#define STANDBY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "bstd.h"
#include "bkni.h"
#include "nexus_platform.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_core_utils.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_audio_dummy_output.h"
#include "nexus_spdif_output.h"
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#include "nexus_stream_mux.h"
#endif
#include "nexus_audio_encoder.h"
#include "nexus_audio_mixer.h"
#include "nexus_playback.h"
#include "nexus_recpump.h"
#if NEXUS_HAS_RECORD
#include "nexus_record.h"
#endif
#include "nexus_file.h"
#include "nexus_ir_input.h"
#if NEXUS_HAS_UHF_INPUT
#include "nexus_uhf_input.h"
#endif
#if NEXUS_HAS_KEYPAD
#include "nexus_keypad.h"
#endif
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_parser_band.h"
#include "nexus_transport_wakeup.h"
#include "nexus_message.h"
#if NEXUS_HAS_PICTURE_DECODER
#include "nexus_picture_decoder.h"
#endif
#if PLAYBACK_IP_SUPPORT
#include "b_playback_ip_lib.h"
#endif


/* These codes correspond to keys on the OneForAll Remote */
#define EXIT_IR_CODE 0xD012
#define S0_IR_CODE 0x600A /*master power*/
#define S1_IR_CODE 0xF001 /*1*/
#define S2_IR_CODE 0xE002 /*2*/
#define S3_IR_CODE 0xD003 /*3*/
#define S5_IR_CODE 0xB005 /*5*/
#define SO_IR_CODE 0xE011 /*ok*/

#define S0_UHF_CODE 0x00220373

#define S0_KEYPAD_CODE


#if (BCHP_CHIP == 7425) || (BCHP_CHIP == 7435) || (BCHP_CHIP == 7420)
#define MEMC1_SUPPORT 1
#endif

#define MAX_CONTEXTS 2

typedef struct B_StandbyNexusHandles {
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_DisplayHandle displayHD, displaySD;
    NEXUS_VideoWindowHandle windowHD[MAX_CONTEXTS];
    NEXUS_VideoWindowHandle windowSD[MAX_CONTEXTS];
    NEXUS_Graphics2DHandle gfx2d;
    NEXUS_SurfaceHandle framebufferHD, framebufferSD, offscreenHD, offscreenSD;
    NEXUS_StcChannelHandle stcChannel[MAX_CONTEXTS];
    NEXUS_PidChannelHandle videoPidChannel[MAX_CONTEXTS], audioPidChannel[MAX_CONTEXTS];
    NEXUS_VideoDecoderHandle videoDecoder[MAX_CONTEXTS];
    NEXUS_VideoDecoderStartSettings videoProgram[MAX_CONTEXTS];
    NEXUS_AudioDecoderHandle audioDecoder[MAX_CONTEXTS];
    NEXUS_AudioDecoderStartSettings audioProgram[MAX_CONTEXTS];
    NEXUS_ParserBand parserBand[MAX_CONTEXTS];
#if NEXUS_HAS_FRONTEND
    NEXUS_FrontendHandle frontend[MAX_CONTEXTS];
#endif
    NEXUS_FilePlayHandle filePlay[MAX_CONTEXTS];
    NEXUS_PlaypumpHandle playpump[MAX_CONTEXTS];
    NEXUS_PlaybackHandle playback[MAX_CONTEXTS];
#if NEXUS_HAS_RECORD
    NEXUS_FileRecordHandle fileRec[MAX_CONTEXTS];
    NEXUS_RecpumpHandle recpump[MAX_CONTEXTS];
    NEXUS_RecordHandle record[MAX_CONTEXTS];
#endif
#if PLAYBACK_IP_SUPPORT
    B_PlaybackIpHandle playbackIp[MAX_CONTEXTS];
#endif
#if NEXUS_HAS_PICTURE_DECODER
    NEXUS_PictureDecoderHandle pictureDecoder;
    NEXUS_SurfaceHandle picFrameBuffer, picSurface;
    NEXUS_PictureDecoderStatus imgStatus;
    FILE *picFileHandle;
#endif
#ifdef NEXUS_HAS_VIDEO_ENCODER
    NEXUS_DisplayHandle displayTranscode;
    NEXUS_VideoWindowHandle windowTranscode;
    NEXUS_StcChannelHandle stcChannelTranscode;
    NEXUS_AudioMixerHandle audioMixer;
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_AudioEncoderHandle audioEncoder;
    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_StreamMuxHandle streamMux;
    NEXUS_PlaypumpHandle playpumpTranscodeAudio;
    NEXUS_PlaypumpHandle playpumpTranscodeVideo;
    NEXUS_PlaypumpHandle playpumpTranscodePcr;
    NEXUS_PidChannelHandle pidChannelTranscodeAudio;
    NEXUS_PidChannelHandle pidChannelTranscodeVideo;
    NEXUS_PidChannelHandle pidChannelTranscodePcr;
    NEXUS_FileRecordHandle fileTranscode;
    NEXUS_RecpumpHandle recpumpTranscode;
    NEXUS_RecordHandle recordTranscode;
    BKNI_EventHandle finishEvent;
#endif

    NEXUS_InputBand wakeupInputBand;

    NEXUS_IrInputHandle irHandle;
#if NEXUS_HAS_UHF_INPUT
    NEXUS_UhfInputHandle uhfHandle;
#endif
#if NEXUS_HAS_KEYPAD
    NEXUS_KeypadHandle keypadHandle;
#endif
#if NEXUS_HAS_CEC
    NEXUS_CecHandle hCec;
#endif
    BKNI_EventHandle event, s1Event;
    BKNI_EventHandle checkpointEvent, spaceAvailableEvent;
    BKNI_EventHandle signalLockedEvent;
} B_StandbyNexusHandles;

struct opts_t {
    NEXUS_TransportType transportType;
    unsigned short videoPid, pcrPid, audioPid, pmtPid;
    NEXUS_VideoCodec videoCodec;
    NEXUS_AudioCodec audioCodec;
    unsigned width, height;
};

struct url_t {
    char scheme[32];
    char domain[128];
    unsigned port;
    char path[256]; /* contains "/path?query_string#fragment_id" */
};

typedef enum B_InputSource {
    eInputSourceNone,
    eInputSourceQam,
    eInputSourceSat,
    eInputSourceOfdm,
    eInputSourceStreamer,
    eInputSourceFile,
    eInputSourceHdmi
} B_InputSource;

typedef enum B_DevicePowerMode {
    ePowerModeS0,
    ePowerModeS1,
    ePowerModeS2,
    ePowerModeS3,  /* S3 warm boot */
    ePowerModeS5   /* S3 cold boot */
} B_DevicePowerMode;

typedef struct B_DeviceState {
    bool hdmi_connected;
    bool component_connected;
    bool composite_connected;
    bool rfm_connected;
    bool dac_connected[MAX_CONTEXTS];
    bool spdif_connected[MAX_CONTEXTS];
    bool frontend_tuned[MAX_CONTEXTS];
    bool decoder_connected[MAX_CONTEXTS];
    bool decode_started[MAX_CONTEXTS];
    bool playback_started[MAX_CONTEXTS];
    bool record_started[MAX_CONTEXTS];
    bool encode_started;
    bool graphics2d_started;
    bool stop_graphics2d;
    bool openfe;
    B_InputSource source[MAX_CONTEXTS];
    struct {
        unsigned freq, qammode, satmode, ofdmmode, adc;
    } frontend [MAX_CONTEXTS];
    char *playfile[MAX_CONTEXTS];
    char *picfile;
    B_DevicePowerMode power_mode;
    bool exit_app;
    bool timer_wake;
    bool cecDeviceReady;
#if PLAYBACK_IP_SUPPORT
    bool playbackIpActive[MAX_CONTEXTS];
    bool playbackIpLiveMode[MAX_CONTEXTS];
    B_PlaybackIpPsiInfo playbackIpPsi[MAX_CONTEXTS];
    struct url_t url;
#endif
    struct opts_t opts[MAX_CONTEXTS];
    unsigned num_contexts;
} B_DeviceState;


/* this is a simple wrapper around pmlib to make the sample app simpler */
typedef struct pmlib_state_t
{
    bool usb;
    bool enet;
    bool moca;
    bool sata;
    bool tp1;
    bool tp2;
    bool tp3;
    bool memc1;
    bool cpu;
    bool ddr;
    int flags;
} pmlib_state_t;

int start_app(void);
void stop_app(void);
void ir_open(void);
void ir_close(void);
void ir_last_key(unsigned *code, unsigned *codeHigh);
void uhf_open(void);
void uhf_close(void);
void keypad_open(void);
void keypad_close(void);
void display_open(unsigned id);
void display_close(unsigned id);
void window_open(unsigned window_id, unsigned display_id);
void window_close(unsigned window_id, unsigned display_id);
void add_hdmi_output(void);
void remove_hdmi_output(void);
void add_component_output(void);
void remove_component_output(void);
void add_composite_output(void);
void remove_composite_output(void);
void add_rfm_output(void);
void remove_rfm_output(void);
void add_dac_output(unsigned id);
void remove_dac_output(unsigned id);
void add_spdif_output(unsigned id);
void remove_spdif_output(unsigned id);
void add_outputs(void);
void remove_outputs(void);
void playback_open(unsigned id);
void playback_close(unsigned id);
void record_open(unsigned id);
void record_close(unsigned id);
void stc_channel_open(unsigned id);
void stc_channel_close(unsigned id);
void graphics2d_open(void);
void graphics2d_close(void);
void graphics2d_setup(void);
void graphics2d_destroy(void);
void graphics3d_open(void);
void graphics3d_close(void);
void decoder_open(unsigned id);
void decoder_close(unsigned id);
void picture_decoder_open(void);
void picture_decoder_close(void);
void encoder_open(unsigned id);
void encoder_close(unsigned decoder_id);
void add_window_input(unsigned id);
void remove_window_input(unsigned decoder_id);
int live_setup(unsigned id);
int playback_setup(unsigned id);
int decode_start(unsigned id);
void decode_stop(unsigned id);
int picture_decode_start(void);
void picture_decode_stop(void);
int picture_decode_display(void);
int encode_start(unsigned id);
void encode_stop(unsigned id);
int graphics2d_start(void);
void graphics2d_stop(void);
int playback_start(unsigned id);
void playback_stop(unsigned id);
int record_start(unsigned id);
void record_stop(unsigned id);
void untune_frontend(unsigned id);
int tune_qam(unsigned id);
void untune_qam(unsigned id);
int tune_ofdm(unsigned id);
void untune_ofdm(unsigned id);
int tune_sat(unsigned id);
void untune_sat(unsigned id);
int streamer_start(unsigned id);
void streamer_stop(unsigned id);
int start_live_context(unsigned id);
void stop_live_context(unsigned id);
int start_play_context(unsigned id);
void stop_play_context(unsigned id);
void stop_decodes(void);
#endif
