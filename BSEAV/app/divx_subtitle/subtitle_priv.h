/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *****************************************************************************/
/*
A collection of functions to make the example apps smaller and easier to understand.
These are not NEXUS api's and are subject to change.
*/

#ifndef EXAMPLES_LIB_H__
#define EXAMPLES_LIB_H__

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#endif
#include "nexus_video_types.h"
#include "nexus_audio_types.h"
#include "nexus_stc_channel.h"
#include "nexus_pid_channel.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_video_input.h"
#include "nexus_video_output.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_spdif_output.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_graphics2d.h"
#include "nexus_message.h"
#include "nexus_core_utils.h"
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif

#include "nexus_security.h"
#include "nexus_keyladder.h"
#include "nexus_dma.h"
#ifdef B_HAS_DIVX_DRM
#include "nexus_divxdrm.h"
#endif

#include "subtitle_control.h"

#include "b_os_lib.h"
#include "bkni.h"
#include "bkni_multi.h"

#include "bwin.h"


typedef struct Subtitle_OutputProtection_Data{
      uint32_t cgmsaBit;
      uint32_t cgmsa_data;
      uint32_t cgmsb_data[5];
      uint16_t wss_data;
      uint32_t macrovision_data;
      NEXUS_ClosedCaptionData ccData;
      uint8_t ictBit;  
      bool enabled;   
      B_ThreadHandle ccHandle[2];      
}Subtitle_OutputProtection_Data;

typedef struct Subtitle_DecodeSettings {
    NEXUS_ParserBand parserBand;
    NEXUS_TransportType transportType;
    NEXUS_PlaypumpHandle playpump; /* if set, open playpump pid channels */
#if NEXUS_HAS_PLAYBACK
    NEXUS_PlaybackHandle playback; /* if set, open playback pid channels */
#endif
    uint16_t videoPid[MAX_TRACKS];
    NEXUS_VideoCodec videoCodec[MAX_TRACKS];
    uint16_t audioPid[MAX_TRACKS];
    NEXUS_AudioCodec audioCodec[MAX_TRACKS];
    uint16_t subtitlePid[MAX_TRACKS];	  
} Subtitle_DecodeSettings;

typedef struct Subtitle_Handles {
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram[MAX_TRACKS];
#if NEXUS_HAS_AUDIO
    NEXUS_AudioDecoderHandle audioDecoder, audioPassthrough;
    NEXUS_AudioDecoderStartSettings audioProgram[MAX_TRACKS];
#endif
    NEXUS_MessageHandle subtitle[MAX_TRACKS];
    NEXUS_MessageStartSettings subtitleProgram[MAX_TRACKS];
    unsigned totalSubtitleTracks;

    NEXUS_PlaypumpHandle playpump;
#if NEXUS_HAS_PLAYBACK
    NEXUS_PlaybackHandle playback;
#endif
    NEXUS_FilePlayHandle file;	  
    NEXUS_FilePlayHandle orig_filehandle;

    NEXUS_DisplayHandle displayHD;
    NEXUS_DisplayHandle displaySD;
    Subtitle_OutputProtection_Data outputProtectionData;
    NEXUS_Graphics2DHandle graphics;
    BKNI_EventHandle checkpointEvent, packetSpaceAvailableEvent;
	  
    NEXUS_SurfaceHandle surfaceHD, surfaceSD, offscreen;
    NEXUS_SurfaceCreateSettings createSettings; /* Cache Create setting for offscreen surface */
	  
    NEXUS_PidChannelHandle videoPid;
    NEXUS_PidChannelHandle audioPid;
    NEXUS_PidChannelHandle subtitlePid;
    NEXUS_VideoWindowHandle windowHD;
    NEXUS_VideoWindowHandle windowSD;
    NEXUS_StcChannelHandle stcChannel;
    /* NEXUS_SurfaceHandle fb; */
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelHandle syncChannel;
#endif
    
#ifdef B_HAS_DIVX_DRM
    NEXUS_DivxDrmHandle divxDrm;
    NEXUS_KeySlotHandle genericKeyHandle;
#endif

    bwin_engine_t win_engine;
    bwin_engine_settings win_engine_settings;
    bwin_framebuffer_t win_fb;
    bwin_framebuffer_settings win_fb_settings;
    bwin_font_t win_font_default;
    bwin_font_t win_font_attached[MAX_TRACKS];
	  
    bool subtitle_started;
    bool wait_for_first_pts_passed;
	  
    uint32_t videoWidth, videoHeight;
    uint32_t displayHDWidth, displayHDHeight;
    uint32_t displaySDWidth, displaySDHeight;
    NEXUS_AspectRatio aspect_ratio;
} Subtitle_Handles;



typedef struct Subtitle_Scheduler_data
{
    B_ThreadHandle thread;
    B_MutexHandle mutex;
    B_SchedulerHandle scheduler;
    B_SchedulerTimerId sync_timer; 
    B_SchedulerCallbackHandle callback; 
    BKNI_EventHandle event;
    B_ThreadHandle subtitle_thread;
    BKNI_EventHandle subtitle_event;
} Subtitle_Scheduler_data;

/* Single decode of audio/video. */
void Decode_GetDefaultSettings(Subtitle_DecodeSettings *pSettings);

/* Callback for ictBit, This will be called if ictBit is set to 0 */
void ictDisplay (void * cnxt, uint8_t ictBit );

/* Callback for Macrovision Bit and CGMSA Bit, */
void outputProtection (void * cnxt, uint32_t macBit, uint32_t cgmsaBit );

/* Function to set the Display format and CGMS configuration */
void Display_SetOutput(NEXUS_DisplaySettings *displayCfg, int display, int pal );

/* Init display */
NEXUS_Error InitDisplay(void);

/* Start a decode */
void SetVideoProgram(Subtitle_DecodeSettings *pSettings, int  totalVideoTracks);
NEXUS_Error InitVideoDecode(Subtitle_DecodeSettings *pSettings, int  totalVideoTracks);
NEXUS_Error StartVideoDecode(Subtitle_DecodeSettings *pSettings, unsigned track, bool tsm);
void StopVideoDecode(void);

void SetAudioProgram(Subtitle_DecodeSettings *pSettings, int  totalVideoTracks);
NEXUS_Error InitAudioDecode(Subtitle_DecodeSettings *pSettings, int  totalAudioTracks);
NEXUS_Error StartAudioDecode(Subtitle_DecodeSettings *pSettings, unsigned track, bool tsm);
void StopAudioDecode(void);

NEXUS_Error InitSubtitleDecode(Subtitle_DecodeSettings *pSettings, int  totalSubtitleTracks, BKNI_EventHandle event);
NEXUS_Error StartSubtitleDecode(Subtitle_DecodeSettings *pSettings);
void StopSubtitleDecode(void);

void StopAllDecode(void);

void UninitDisplay(void);

/* Dump status */
void PrintDecodeStatus(void);

/* Get time in milliseconds */
unsigned b_get_time(void);

/* Convert settop to Nexus transport type */
NEXUS_TransportType GetTransportType(bstream_mpeg_type mpeg_type);

/* Convert settop to Nexus audio codec */
NEXUS_AudioCodec GetAudioCodec(baudio_format settopCodec);

/* Convert settop to Nexus video codec */
NEXUS_VideoCodec GetVideoCodec(bvideo_codec settopCodec);

void video_request_stc(void *context, int param);

#endif
