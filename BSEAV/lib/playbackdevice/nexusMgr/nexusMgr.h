/******************************************************************************
 *    (c)2008-2011 Broadcom Corporation
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
 * 
 *****************************************************************************/

#ifndef __BCMNEXUS_MGR_H__
#define __BCMNEXUS_MGR_H__

#include <sys/time.h>
#include "nexus_graphics2d.h"
#include "nexus_audio_playback.h"
#include "nexus_video_decoder.h"
#include "nexus_video_window.h"
#include "nexus_stc_channel.h"
#include "nexus_audio_decoder.h"
#include "nexus_video_window.h"
#include "nexus_playpump.h"
#include "nexus_video_input.h"
#include "nexus_ir_input.h"
#include "TestHarness.h"
#ifdef NEXUS_HAS_PICTURE_DECODER
#include "nexus_picture_decoder.h"
#endif

#define BRCM_COPY_PROTECTION_ENABLED 1
#if BRCM_COPY_PROTECTION_ENABLED
#include "nexus_hdmi_output.h"
#include "nexus_component_output.h"
#include "nexus_hdmi_output_hdcp.h"
#endif 
#ifdef __cplusplus
extern "C" 
{
#endif

bool BcmNexus_Platform_Init(void);
void BcmNexus_Platform_Uninit(void);

/*#define _DFB_TEST_HARNESS_*/

/* Graphics 2D Resource */
typedef struct BcmNexus_Graphics_Resources_Config {
    unsigned unused;
} BcmNexus_Graphics_Resources_Config;

typedef struct BcmNexus_Graphics_Window {
	NEXUS_SurfaceHandle surface;
} BcmNexus_Graphics_Window;

typedef struct BcmNexus_Graphics_Resources {
    BcmNexus_Graphics_Resources_Config	config;
    NEXUS_Graphics2DHandle				graphics2d;
    void (*graphicsSync)(void);
    const BcmNexus_Graphics_Window *(*createWindow)(const struct BcmNexus_Graphics_Resources *resources, NEXUS_Rect *rect, bool defaultPosition, NEXUS_PixelFormat format, const char *title);
    void (*destroyWindow)(const BcmNexus_Graphics_Window *window);
    void (*windowUpdated)(const BcmNexus_Graphics_Window *window, const NEXUS_Rect *rect);
} BcmNexus_Graphics_Resources;

const BcmNexus_Graphics_Resources *
BcmNexus_Graphics_Resources_Acquire(BcmNexus_Graphics_Resources_Config *config);
void 
BcmNexus_Graphics_Resources_Release(const BcmNexus_Graphics_Resources *resources);


/* Sound Resource */
typedef struct BcmNexus_SoundOutput_Resources_Config {
    unsigned fifoSize;
	NEXUS_AudioPlaybackStartSettings startSettings;
} BcmNexus_SoundOutput_Resources_Config;

/* size of the single audio frame in bytes */
#define B_AUDIO_FRAME_SIZE  512
/* 32 * 512 buffer allows to buffer up-to 371 msec of 44.1KHz audio data in HW */
#define B_AUDIO_BUFFERED_FRAMES 32

typedef enum  BcmNexus_SoundOutput_Volume {
    BcmNexus_SoundOutput_Volume_ePcm,
    BcmNexus_SoundOutput_Volume_eDecoder
} BcmNexus_SoundOutput_Volume;

NEXUS_Error BcmNexus_SoundOutput_SetVolume(BcmNexus_SoundOutput_Volume volume, unsigned level);

typedef struct BcmNexus_SoundOutput_Resources{
    NEXUS_AudioPlaybackHandle pcmPlayback;
	void (*pcmStatusUpdate) (const struct BcmNexus_SoundOutput_Resources *resources, bool started);
    BcmNexus_SoundOutput_Resources_Config config;
} BcmNexus_SoundOutput_Resources;

const BcmNexus_SoundOutput_Resources *
BcmNexus_SoundOutput_Resources_Acquire(const BcmNexus_SoundOutput_Resources_Config  *config);
void 
BcmNexus_SoundOutput_Resources_Release(const BcmNexus_SoundOutput_Resources *resources);


/* Stream Player Resource */
typedef struct BcmNexus_StreamPlayer_Resources_Config {
	NEXUS_VideoCodec videoCodec;
	NEXUS_AudioCodec audioCodec;
	uint16_t audioPid;
	uint16_t videoPid;
	uint16_t numPlaypumps;
	NEXUS_TransportType transportType;
	NEXUS_CallbackDesc videoDataCallback;            
	NEXUS_CallbackDesc audioDataCallback;
	NEXUS_KeySlotHandle decryptContext;
	NEXUS_CallbackDesc videoSourceCallback; 
	NEXUS_CallbackDesc fifoEmptyCallback;	
	void *stageWindow;
	bool enableHDCP;
} BcmNexus_StreamPlayer_Resources_Config;

typedef struct BcmNexus_StreamPlayer_Resources {
    BcmNexus_StreamPlayer_Resources_Config  config;
    NEXUS_StcChannelHandle                  stcChannel;
    NEXUS_VideoDecoderHandle                videoDecoder;
    NEXUS_AudioDecoderHandle                audioDecoder[2];
	#if BRCM_COPY_PROTECTION_ENABLED
    NEXUS_DisplayHandle                     display;
	#endif
	
    NEXUS_VideoWindowHandle                 videoWindow;
    NEXUS_PlaypumpHandle                    playpump[2];
    NEXUS_PidChannelHandle                  videoPidChannel;
    NEXUS_PidChannelHandle                  audioPidChannel;
    void                                    (*audioStatusUpdate) (const struct BcmNexus_StreamPlayer_Resources *resources, bool started);
	NEXUS_Error								(*setVideoRegion) (const struct BcmNexus_StreamPlayer_Resources *resources, NEXUS_Rect *planeRect, NEXUS_Rect *rect, NEXUS_Rect *clipRect);
    NEXUS_DmaHandle                         dmaHandle;
    uint32_t                                scaleX;
    uint32_t                                scaleY;
	void									*stageWindow;
} BcmNexus_StreamPlayer_Resources;

const BcmNexus_StreamPlayer_Resources *
BcmNexus_StreamPlayer_Resources_Acquire( const BcmNexus_StreamPlayer_Resources_Config *config);
void 
BcmNexus_StreamPlayer_Resources_Release(const BcmNexus_StreamPlayer_Resources *resources_);

struct timeval BcmNexus_Key_Last_Key_Timestamp(void);

/* IR Resource */
typedef struct BcmNexus_Input_Resources_Config {
	NEXUS_CallbackDesc	key_down_callback;
	NEXUS_CallbackDesc	key_up_callback;
    bool				enable_tty;
} BcmNexus_Input_Resources_Config;


typedef struct BcmNexus_Input_Resources {
    BcmNexus_Input_Resources_Config	config;
} BcmNexus_Input_Resources;

const BcmNexus_Input_Resources *
BcmNexus_Input_Resources_Acquire(const BcmNexus_Input_Resources_Config *config);

void 
BcmNexus_Input_Resources_Release(const BcmNexus_Input_Resources *resources);


#ifdef NEXUS_HAS_PICTURE_DECODER
/* Image Decoder Resource */
typedef struct BcmNexus_ImageDecoder_Resources_Config {
    size_t imageMaxSize;
} BcmNexus_ImageDecoder_Resources_Config;

typedef struct BcmNexus_ImageDecoder_Resources {
    BcmNexus_ImageDecoder_Resources_Config config;
    NEXUS_Graphics2DHandle graphics;
    NEXUS_PictureDecoderHandle pictureDecoder;
    void (*graphics_sync)(void);
} BcmNexus_ImageDecoder_Resources;

const BcmNexus_ImageDecoder_Resources *
BcmNexus_ImageDecoder_Resources_Acquire( const BcmNexus_ImageDecoder_Resources_Config *config );

extern void 
BcmNexus_ImageDecoder_Resources_Release(const BcmNexus_ImageDecoder_Resources *resources);
#endif

#ifdef _DFB_TEST_HARNESS_
void	
BcmNexus_DFBWindowUpdate(void * pFrameBuffer, 
                         int x,int y, 
                         int h,int w,
                         int pitch,
                         STG_KEY_UPDATE pfnKeyUpdateFn,
                         void * keyUpdateCnxt);
void 
BcmNexus_DFBKeyEvent(U32 keyCode, 
					 U32 direction);
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
