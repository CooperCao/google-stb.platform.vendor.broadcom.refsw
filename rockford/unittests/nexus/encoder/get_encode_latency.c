/***************************************************************************
 *     (c)2003-2014 Broadcom Corporation
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
#if NEXUS_HAS_VIDEO_ENCODER && NEXUS_HAS_HDMI_INPUT && NEXUS_HAS_PICTURE_DECODER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include <linux/videodev2.h>
#include <linux/input.h>
#include <linux/soundcard.h>

#include <netinet/in.h>
#include <net/if.h>
#include <linux/sockios.h>

#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bkni.h"           /* kernel interface */
#include "bkni_multi.h"     /* kernel interface */
#include "bdbg.h"

#include "b_os_lib.h"

/* Nexus example app: show jpeg image */
#include "nexus_platform.h"
#include "nexus_graphics2d.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_video_image_input.h"
#include "nexus_video_decoder.h"
#include "nexus_video_encoder.h"
#include "nexus_video_encoder_output.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "nexus_playback.h"
#include "nexus_record.h"
#include "nexus_parser_band.h"
#include "nexus_stream_mux.h"
#include "nexus_timebase.h"

#include "tshdrbuilder.h"
#include "b_os_lib.h"

#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#include "nexus_picture_decoder.h"
#include "nexus_hdmi_input.h"
#include "nexus_hdmi_input_hdcp.h"


#define RECORD_TO_FILE 1

/***********************************
 *  USB Video Camera
 */
#define DEFAULT_VIDEO_CAMERA_DEVICE      "/dev/video0"
#define DEFAULT_VIDEO_CAMERA_WIDTH        1280
#define DEFAULT_VIDEO_CAMERA_HEIGHT       720
#define DEFAULT_VIDEO_CAMERA_FRAMERATE    30
#define DEFAULT_AUDIO_NUM_CHANNELS        1
#define DEFAULT_AUDIO_FORMAT              AFMT_S16_LE   /* use big endian instead of little endian */
#define DEFAULT_AUDIO_NEWFRAG             0x7fff0006    /* Unlimited number of 256(2^8) fragments */
#define DEFAULT_AUDIO_SPEED               8000
#define DEFAULT_AUDIO_DEVICE             "/dev/dsp"
#define DEFAULT_AUDIO_MAXDATALENGTH       320
#define VIDEO_CLOCK_RATE                 (45000)


/***********************************
 * STC channels
 */
#define ENCODER_STC_IDX                   1
#define LOOPBACK_STC_IDX                  (NEXUS_NUM_STC_CHANNELS - 1)

/***********************************
 * Mux
 */
/* file i/o page size */
#define RECORD_PAGE_SIZE                 (4096)
#define RECORD_ATOM_SIZE                 (47*RECORD_PAGE_SIZE)
/* nexus_record limits the per file write transaction size */
#define RECORD_WRITE_SIZE_LIMIT          (3*RECORD_ATOM_SIZE)
/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
#define TS_HEADER_BUF_LENGTH              189
/* multi buffer to allow the background PSI packet to update CC while foreground one is waiting for TS mux pacing */
#define PSI_QUEUE_CNT                     4 /* every second insert a PSI, so it takes 4 secs to circle; hopefully its transfer won't be delay that far */

/* PID assignments */
#define MUX_PCR_PID                      (0x11)
#define MUX_VIDEO_PID                    (0x12)
#define MUX_PMT_PID                      (0x55)
#define MUX_PAT_PID                      (0x0)

/* Playpump assignments */
#define LOOPBACK_PLAYPUMP_IDX             0
#define STREAM_MUX_VIDEO_PLAYPUMP_IDX     1
#define STREAM_MUX_AUDIO_PLAYPUMP_IDX     2
#define STREAM_MUX_PCR_PLAYPUMP_IDX       3
#define SOURCE_PLAYPUMP_IDX               4

/* Recpump assignment */
#define STREAM_MUX_RECPUMP_IDX            0
#define INPUT_RECPUMP_IDX                 1

/* HTTP Streaming */
#define HTTP_STREAMING_DEFAULT_SERVER_PORT 5000

#define USE_APPLICATION_BUFFER 0
#define DEBUG 0

/* keystroke handler */
#define INFINITE_WAIT (0xffffffff)



BDBG_MODULE(video_conf);
BDBG_FILE_MODULE(contcounter);


static uint8_t SampleEDID[] =
{
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x08, 0x6D, 0x74, 0x22, 0x05, 0x01, 0x11, 0x20,
	0x00, 0x14, 0x01, 0x03, 0x80, 0x00, 0x00, 0x78, 0x0A, 0xDA, 0xFF, 0xA3, 0x58, 0x4A, 0xA2, 0x29,
	0x17, 0x49, 0x4B, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
	0x45, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16, 0x20,
	0x58, 0x2C, 0x25, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x9E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x42,
	0x43, 0x4D, 0x37, 0x34, 0x32, 0x32, 0x2F, 0x37, 0x34, 0x32, 0x35, 0x0A, 0x00, 0x00, 0x00, 0xFD,
	0x00, 0x17, 0x3D, 0x0F, 0x44, 0x0F, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x89,

	0x02, 0x03, 0x3C, 0x71, 0x7F, 0x03, 0x0C, 0x00, 0x40, 0x00, 0xB8, 0x2D, 0x2F, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xE3, 0x05, 0x1F, 0x01, 0x49, 0x90, 0x05, 0x20, 0x04, 0x03, 0x02, 0x07,
	0x06, 0x01, 0x29, 0x09, 0x07, 0x01, 0x11, 0x07, 0x00, 0x15, 0x07, 0x00, 0x01, 0x1D, 0x00, 0x72,
	0x51, 0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x1E, 0x8C, 0x0A,
	0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0xBA, 0x88, 0x21, 0x00, 0x00, 0x18,
	0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0x0B, 0x88, 0x21, 0x00,
	0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9D
};

void source_changed(void *context, int param)
{
#if OUTPUT_VIDEO_FORMAT_FOLLOWS_INPUT
    NEXUS_HdmiInputHandle hdmiInput ;
    NEXUS_HdmiInputStatus hdmiInputStatus ;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)param;

    hdmiInput = (NEXUS_HdmiInputHandle) context ;
    NEXUS_HdmiInput_GetStatus(hdmiInput, &hdmiInputStatus) ;

    NEXUS_Display_GetSettings(display, &displaySettings);
    if ( displaySettings.format != hdmiInputStatus.originalFormat )
    {
        BDBG_WRN(("Video Format Change - Updating to %u", hdmiInputStatus.originalFormat));

        displaySettings.format = hdmiInputStatus.originalFormat;
        NEXUS_Display_SetSettings(display, &displaySettings);
    }
#else
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
#endif
}

void avmute_changed(void *context, int param)
{
    NEXUS_HdmiInputHandle hdmiInput ;
    NEXUS_HdmiInputStatus hdmiInputStatus ;
    BSTD_UNUSED(param);

    hdmiInput = (NEXUS_HdmiInputHandle) context ;
    NEXUS_HdmiInput_GetStatus(hdmiInput, &hdmiInputStatus) ;

    if (!hdmiInputStatus.validHdmiStatus)
    {
        printf("avmute_changed callback: Unable to get hdmiInput status\n") ;
    }
    else
    {
        printf("avmute_changed callback: %s\n",
            hdmiInputStatus.avMute ? "Set_AvMute" : "Clear_AvMute") ;
    }
}

static unsigned char JPGHdr[0x14] =
{
    0xff,0xd8,                  /*SOI*/
    0xff,0xe0,                  /* APP0*/
    0x00,0x10,                  /* APP0 Hdr size*/
    0x4a,0x46,0x49,0x46,0x00,   /* ID string*/
    0x01,0x01,                  /* Version*/
    0x00,                       /* Bits per type*/
    0x00, 0x00,                 /* X density*/
    0x00, 0x00,                 /* Y density*/
    0x00,                       /* X Thumbnail size*/
    0x00                        /* Y Thumbnail size*/
};

static unsigned char MJPGDHTSeg[0x1A4] =
{
     /* JPEG DHT Segment for YCrCb omitted from MJPG data */
    0xFF,0xC4,0x01,0xA2,
    0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x01,0x00,0x03,0x01,0x01,0x01,0x01,
    0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,0x10,0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,0x04,0x00,
    0x00,0x01,0x7D,0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,
    0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xA1,0x08,0x23,0x42,0xB1,0xC1,0x15,0x52,0xD1,0xF0,0x24,
    0x33,0x62,0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,0x1A,0x25,0x26,0x27,0x28,0x29,0x2A,0x34,
    0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,
    0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
    0x79,0x7A,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,
    0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,
    0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,
    0xDA,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
    0xF8,0xF9,0xFA,0x11,0x00,0x02,0x01,0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01,
    0x02,0x77,0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,
    0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xA1,0xB1,0xC1,0x09,0x23,0x33,0x52,0xF0,0x15,0x62,
    0x72,0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,0x17,0x18,0x19,0x1A,0x26,0x27,0x28,0x29,0x2A,
    0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,0x55,0x56,
    0x57,0x58,0x59,0x5A,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,0x75,0x76,0x77,0x78,
    0x79,0x7A,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,0x94,0x95,0x96,0x97,0x98,
    0x99,0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,
    0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,
    0xD9,0xDA,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,
    0xF9,0xFA
};

struct VideoCamera_Buffer {
    void *            start;
    size_t            length;
};

typedef struct VideoCamera_FrameInfo
{
    uint8_t          *data;               /* Frame data */
    int               size;               /* The size of frame data */
    int               index;              /* The index of frame */
    struct timeval	  timeStamp;          /* Time Stamp */
} VideoCamera_FrameInfo;

typedef struct VideoCameraSettings
{
	struct
	{
		char             *videoDevName;
    	unsigned int      width;
    	unsigned int      height;
    	unsigned int      framerate;
	} video;

    struct
    {
		char             *audioDevName;
    	int               format;
    	int               channels;
    	int               speed;
    	int               newFrag;
    	int               maxDataLength;
	} audio;
} VideoCameraSettings;

typedef struct VideoCamera
{
	struct
	{
		int                         videoFd;
		struct VideoCamera_Buffer  *videoBuffers;
		unsigned int                numBuffers;
		NEXUS_SurfaceHandle         picture;
	} video;

	struct
	{
    	int                         audioFd;
    	int8_t                     *readPacketBuffer;
	} audio;

	VideoCameraSettings             settings;
} VideoCamera;


typedef struct TranscoderSettings
{
    bool                            variableFrameRate;
    NEXUS_VideoCodec                codec;
    NEXUS_VideoFrameRate            frameRate;
    NEXUS_VideoCodecProfile         profile;
    NEXUS_VideoCodecLevel           level;
    unsigned                        bitrateMax;
    unsigned                        framesP;
    unsigned                        framesB;
    NEXUS_VideoFormat               displayFormat;
    unsigned                        displayRefreshRate;
    bool                            isDisplayCustomized;
    unsigned                        encoderWidth;
    unsigned                        encoderHeight;
	bool                            bLoopback;
	bool                            bRecord;
	bool                            bHttpStreamOut;
} TranscoderSettings;

typedef struct Transcoder
{
	/* Transcoder */
	TranscoderSettings                transcoderSettings;
	NEXUS_VideoEncoderOpenSettings    encoderOpenSettings;
    NEXUS_VideoEncoderHandle          videoEncoder;
    NEXUS_StcChannelHandle            encoderStcChannel;
    NEXUS_StcChannelSettings          encoderStcSettings;
	NEXUS_DisplayHandle               encoderDisplay;
    NEXUS_VideoWindowHandle           encoderWindow;
    uint8_t                          *encoderBufferBase;

	/* Stream Mux */
	struct {
		NEXUS_StreamMuxHandle             streamMux;
		NEXUS_StreamMuxStartSettings      config;
		BKNI_EventHandle                  event;
		NEXUS_PlaypumpHandle              videoPlaypump;
		NEXUS_PidChannelHandle            videoPidChannel;
#if 0
		NEXUS_PlaypumpHandle              audioPlaypump;
		NEXUS_PidChannelHandle            audioPidChannel;
		NEXUS_PidChannelHandle            patPidChannel;
		NEXUS_PidChannelHandle            pmtPidChannel;
#endif
		NEXUS_PlaypumpHandle              pcrPlaypump;
		NEXUS_PidChannelHandle            pcrPidChannel;
		NEXUS_RecpumpHandle               recpump;
	} mux;

	/* PSI system data */
	void                             *pat[PSI_QUEUE_CNT];
	void                             *pmt[PSI_QUEUE_CNT];
	unsigned                          ccValue;
	NEXUS_StreamMuxSystemData         psi[2];
	B_MutexHandle                     mutexSystemdata;
	B_SchedulerHandle                 schedulerSystemdata;
	B_SchedulerTimerId                systemdataTimer;
	B_ThreadHandle                    schedulerThread;
	bool                              systemdataTimerIsStarted;

 	/* Loopback */
	struct {
		NEXUS_DisplayHandle           display;
    	NEXUS_VideoWindowHandle       window;
		NEXUS_StcChannelHandle        stcChannel;
		NEXUS_VideoDecoderHandle      videoDecoder;
        NEXUS_PidChannelHandle        videoPidChannel, audioPidChannel;
		NEXUS_PidChannelHandle        pcrPidChannel, pmtPidChannel, patPidChannel;
        NEXUS_PlaypumpHandle          playpump;
        pthread_t                     thread;
        BKNI_EventHandle              event;
        bool                          done;
    } loopback;

#if RECORD_TO_FILE
	struct {
		NEXUS_RecordHandle            record;
		NEXUS_FileRecordHandle        transcodeFile;
		char                          indexfname[13];
		char                          fname[12];
	} record;
#endif

	struct {
		int                           exitThread;
		pthread_t                     thread;
		int                           listeningPort;  /* port to receive HTTP requests on */
	} httpStreaming;

} Transcoder;

typedef enum Input
{
	Input_eCamera = 0,
	Input_eHdmi,
	Input_eStreamer
} Input;


/* Keystroke handler */
static B_EventHandle g_eotEvent = NULL;/* end of test event */

static void keyHandler( void )
{
	char key[256];
	int rcInput;

	for(;;)
	{
		printf("\nEnter 'q' to quit\n\n");
		rcInput = scanf("%s", key);

		if(!strcmp(key, "q"))
		{
			break;
		}
	}

	if(g_eotEvent) {
		B_Event_Set(g_eotEvent);
	}
}

static NEXUS_HeapHandle GetImageInputHeap(unsigned imageInputIndex)
{
    NEXUS_PlatformSettings platformSettings;
    unsigned videoDecoderIndex, heapIndex;
    NEXUS_PlatformConfiguration platformConfig;

    /* The imageInput index is the same as the MPEG feeder (MFD) index. we need to find a heap that is RTS-accessible by this MFD.
    This can be learned from the VideoDecoderModule because the MFD normally reads from AVD picture buffers. */

    /* first, get platform settings, which gives us VideoDecoderModuleSettings */
    NEXUS_Platform_GetSettings(&platformSettings);

    /* next, find the VideoDecoder index that uses this MFD */
    for (videoDecoderIndex=0;videoDecoderIndex<NEXUS_NUM_VIDEO_DECODERS;videoDecoderIndex++) {
        if (platformSettings.videoDecoderModuleSettings.mfdMapping[videoDecoderIndex] == imageInputIndex) {
            /* we've found a video decoder that uses this MFD */
            break;
        }
    }
    if (videoDecoderIndex == NEXUS_NUM_VIDEO_DECODERS) {
        /* this MFD is unused by VideoDecoder, so we can't know the heap */
        return NULL;
    }

    /* now, get the heap index for this video decoder. the MFD must be able to read from this heap
    if it's able to read AVD decoded pictures placed into this heap. */
    heapIndex = platformSettings.videoDecoderModuleSettings.avdHeapIndex[videoDecoderIndex];

    /* then return the heap */
    NEXUS_Platform_GetConfiguration(&platformConfig);
    return platformConfig.heap[heapIndex];
}

static void GfxBlitComplete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void VideoImageInputBufferCallback( void *context, int param )
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static int MjpegParser
    (const unsigned char              *picBuffer,
     const uint32_t                    picSize,
     unsigned char                    *decoderBuffer,
     uint32_t                          decoderBufferSize,
     uint32_t                         *pReadDataSize)
{
    uint32_t  count = 0;
    uint32_t  imageSize = 0;
    uint32_t  offset = 0;
    uint32_t  header = 0;

    BDBG_ASSERT(NULL != picBuffer);
    BDBG_ASSERT(NULL != pReadDataSize);
    BDBG_ASSERT(NULL != decoderBuffer);

    if(decoderBufferSize < picSize + sizeof(JPGHdr) + sizeof(MJPGDHTSeg))
    {
        fprintf(stderr, "(%s,%d): JPEG file size[%u] is larger then buffer size[%u]\n",
            __FUNCTION__, __LINE__, picSize, decoderBufferSize);
        return -1;
    }

    count = picSize - 1;

    if(count == picSize - 1)
    {
#if DEBUG
        fprintf(stderr, "(%s,%d): Huffman table doesn't exist! .v4l2_buf.usedbytes[%d]\n",
            __FUNCTION__, __LINE__, picSize);
#endif
        if(picSize == 0)
        {
            fprintf(stderr, "(%s,%d): v4l2_buf.bytesused is 0; returning error\n",__FUNCTION__, __LINE__);
            return -1;
        }

        imageSize = picSize;
        offset = 0;
#if DEBUG
        fprintf(stderr, "(%s,%d): fwrite all end; write success\n",__FUNCTION__, __LINE__);
#endif
    }
    else
    {
        for(count = 0; count < (picSize - 1); count++)
        {
            if((picBuffer[count] == 0x000000FF) && (picBuffer[count+1] == 0x000000D8))
            break;
        }
#if DEBUG
        fprintf(stderr, "(%s,%d): Huffman table exists! test_i = %d,FF = %02x,D8 = %02x\n",
            __FUNCTION__, __LINE__,count,picBuffer[count],picBuffer[count+1]);
#endif
        imageSize = picSize - count - 1;
        if(imageSize == 0)
        {
            fprintf(stderr, "(%s,%d): usedbytes == 0, return directly\n",__FUNCTION__, __LINE__);
            return -1;
        }

        /*imageBuf = buffer + count + 1;*/
        offset = count + 1;

    }

    BKNI_Memcpy(decoderBuffer,JPGHdr,sizeof(JPGHdr));
    BKNI_Memcpy(decoderBuffer + sizeof(JPGHdr),MJPGDHTSeg,sizeof(MJPGDHTSeg));

    /*removing avi header*/
    header =  *((picBuffer + offset) + 4);
    header <<= 8;
    header += *((picBuffer + offset) + 5) + 4;

    if((sizeof (JPGHdr) + sizeof (MJPGDHTSeg) + imageSize) >= header )
    {
#if DEBUG
        fprintf(stderr, "(%s,%d): Parsing mjpeg original size[%d], avi header size[%d]\n",
             __FUNCTION__, __LINE__,sizeof (JPGHdr) + sizeof (MJPGDHTSeg) + imageSize, header);
#endif
        BKNI_Memcpy((char*)(decoderBuffer) + sizeof (JPGHdr)+ sizeof (MJPGDHTSeg),(char*)(picBuffer + offset) + header,imageSize - header);

        *pReadDataSize = sizeof (JPGHdr) + sizeof (MJPGDHTSeg) + imageSize - header;
    }
    else
    {
        *pReadDataSize = 0;
#if DEBUG
        fprintf(stderr, "(%s,%d): Mjpeg header too large! Parsing mjpeg size[%d], header[%d]\n",
            __FUNCTION__, __LINE__, sizeof(JPGHdr) + sizeof(MJPGDHTSeg) + imageSize, header);
#endif
		return -1;
    }

    return 0;
}

static int VideoCamera_ReleaseVideoFrame
	( VideoCamera *pVideoCamera,
	  VideoCamera_FrameInfo *frame )
{
    struct v4l2_buffer buf;
    int ret = 0;

    if (frame->data == NULL) {
        printf("Frame data is NULL!\n");
        ret = -1;
        goto fail;
    }

    BKNI_Memset(&buf, 0, sizeof(struct v4l2_buffer));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = frame->index;

    ret = ioctl (pVideoCamera->video.videoFd, VIDIOC_QBUF, &buf);
    if (-1 == ret) {
        perror("ReleaseFrame: VIDIOC_QBUF failed");
        goto fail;
    }
    frame->data = NULL;
    frame->size = 0;
    frame->index = -1;

fail:
    return ret;
}

static int VideoCamera_CaptureVideoFrame
	( VideoCamera *pVideoCamera,
	  VideoCamera_FrameInfo *frame )
{
    struct v4l2_buffer buf;
    int ret = 0;

    BKNI_Memset(&buf, 0, sizeof(struct v4l2_buffer));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    ret = ioctl(pVideoCamera->video.videoFd, VIDIOC_DQBUF, &buf);
    if(-1 == ret)
    {
        perror("VIDIOC_DQBUF failed");
        goto fail;
    }

    if(buf.index >= pVideoCamera->video.numBuffers)
    {
        printf("Buffer index(%d) invalid, we only have %d buffers\n", buf.index, pVideoCamera->video.numBuffers);
        ret = -1;
        goto fail;
    }


    frame->data= pVideoCamera->video.videoBuffers[buf.index].start;

    frame->size = buf.bytesused;
    frame->index = buf.index;
    frame->timeStamp= buf.timestamp;
fail:
	return ret;
}


static int VideoCamera_StartVideoCapture
	( VideoCamera *pVideoCamera )
{
    unsigned int i;
    int ret;
    struct v4l2_buffer buf;
    enum v4l2_buf_type type;

    for (i = 0; i < pVideoCamera->video.numBuffers; ++i) {

        BKNI_Memset(&buf, 0, sizeof(struct v4l2_buffer));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        ret = ioctl(pVideoCamera->video.videoFd, VIDIOC_QBUF, &buf);
        if (-1 == ret)
        {
            perror("VIDIOC_QBUF failed");
            goto fail;
        }
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = ioctl (pVideoCamera->video.videoFd, VIDIOC_STREAMON, &type);
    if (-1 == ret)
    {
        perror("VIDIOC_STREAMON failed");
        goto fail;
    }

    sleep(2);
fail:
    return ret;

}


static int VideoCamera_StopVideoCapture
	( VideoCamera *pVideoCamera )
{
    int ret = 0;
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ret = ioctl(pVideoCamera->video.videoFd, VIDIOC_STREAMOFF, &type);

    return ret;
}


/*
 * Open VideoCamera's video module
 * Return: 0 - success; other - fail.
 */
static int VideoCamera_OpenVideo
	( VideoCamera *pVideoCamera )
{
    int ret = 0;
	char *dev_name = pVideoCamera->settings.video.videoDevName;
    unsigned int width = pVideoCamera->settings.video.width;
    unsigned int height = pVideoCamera->settings.video.height;
    unsigned int framerate = pVideoCamera->settings.video.framerate;

	int videoFd;
    struct VideoCamera_Buffer *videoBuffers;
    unsigned int numBuffers;

    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    struct v4l2_streamparm parm;

    videoFd = open(dev_name, O_RDWR, 0);
    if (videoFd < 0)
	{
        fprintf (stderr, "Cannot open '%s': %d, %s\n",
            dev_name, errno, strerror (errno));
        return -1;
    }

    ret = ioctl(videoFd, VIDIOC_QUERYCAP, &cap);
    if (ret)
	{
        perror("VIDIOC_QUERYCAP failed");
        goto fail;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
        fprintf(stderr, "%s is no video capture device\n", dev_name);
        ret = -1;
        goto fail;
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
    	fprintf(stderr, "%s does not support streaming i/o\n", dev_name);
        ret = -1;
        goto fail;
    }

    BKNI_Memset(&fmt, 0, sizeof(struct v4l2_format));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    fprintf(stderr, "Video Camera: Capture size: %d x %d\n", width, height);
    ret = ioctl(videoFd, VIDIOC_S_FMT, &fmt);
    if(ret)
    {
        perror("VIDIOC_S_FMT failed");
        goto fail;
    }

    if ((width != fmt.fmt.pix.width) || (height != fmt.fmt.pix.height))
	{
        fprintf(stderr, "Video Camera: The V4L2 driver changed the video from %dx%d to %dx%d\n", width, height, fmt.fmt.pix.width, fmt.fmt.pix.height);
    }
    fprintf(stderr, "Video Camera: Set %d x %d format successfully\n", fmt.fmt.pix.width, fmt.fmt.pix.height);

    BKNI_Memset(&parm, 0, sizeof(parm));
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = framerate;

    if(ioctl(videoFd, VIDIOC_S_PARM, &parm) == -1)
	{
        perror("VIDIOC_S_PARM  error");
    }

    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(videoFd, VIDIOC_G_PARM, &parm) == -1)
	{
        fprintf(stderr, "VIDIOC_G_PARM failed 1\n");
    }

    if(parm.parm.capture.timeperframe.denominator != framerate)
	{
		fprintf(stderr, "Video Camera: Try to set frame rate %d fps, but device only support %d fps\n", framerate, parm.parm.capture.timeperframe.denominator);
    }

    fprintf(stderr, "Video Camera: Framerate: %d fps\n", parm.parm.capture.timeperframe.denominator);

    /* Init the mmap */
    memset(&req, 0, sizeof(struct v4l2_requestbuffers));
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    ret = ioctl (videoFd, VIDIOC_REQBUFS, &req);
    if (-1 == ret)
    {
        perror("VIDIOC_REQBUFS failed");
        goto fail;
    }

    if (req.count < 2) {
        printf("Insufficient buffer memory");
        goto fail;
    }

    videoBuffers = calloc(req.count, sizeof(struct VideoCamera_Buffer));
    if(videoBuffers == NULL)
    {
        perror("No enought memory");
        ret = -1;
        goto fail;
    }

    for (numBuffers= 0; numBuffers < req.count; ++numBuffers)
    {
        struct v4l2_buffer buf;

        BKNI_Memset(&buf, 0, sizeof(struct v4l2_buffer));
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = numBuffers;

        ret = ioctl (videoFd, VIDIOC_QUERYBUF, &buf);
        if(-1 == ret)
        {
            perror("VIDIOC_QUERYBUF failed");
            goto fail;
        }

        videoBuffers[numBuffers].length = buf.length;
        videoBuffers[numBuffers].start =
            mmap (NULL,
            buf.length,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            videoFd, buf.m.offset);

        if (MAP_FAILED == videoBuffers[numBuffers].start)
        {
            perror("mmap failed");
            ret = -1;
            goto fail;
        }
#if DEBUG
        fprintf(stderr, "buffer[%d]: start = %p, length = %d\n",
            numBuffers,
            videoBuffers[numBuffers].start,
            videoBuffers[numBuffers].length
            );
#endif
    }

	pVideoCamera->video.videoFd = videoFd;
	pVideoCamera->video.videoBuffers = videoBuffers;
	pVideoCamera->video.numBuffers = numBuffers;

    return ret;

fail:
    close(pVideoCamera->video.videoFd);

    return ret;
}

/*
 * Close the VideoCamera's video module.
 * Return: 0 - success; other - fail.
*/
static int VideoCamera_CloseVideo
	( VideoCamera *pVideoCamera )
{
    int ret = 0;
    unsigned int i;

    /* Unmap all the buffers */
    for (i = 0; i < pVideoCamera->video.numBuffers; ++i)
    {
        if (-1 == munmap (pVideoCamera->video.videoBuffers[i].start, pVideoCamera->video.videoBuffers[i].length))
        {
            fprintf(stderr, "munmap buffer %d failed. errno = %d(%s)\n", i, errno, strerror(errno));
            ret--;
        }
    }

    /* free g_videoBuffers */
    free(pVideoCamera->video.videoBuffers);

    /* Close the video device */
    if( -1 == close(pVideoCamera->video.videoFd))
    {
        perror("Close video device failed");
        ret--;
    }

    return ret;
}

/*
 * Decode video from video camera.
 * Return: 0 - success; other - fail.
*/
static int VideoCamera_StartVideoDecode
	 ( VideoCamera *pVideoCamera,
	   void *capFrameBuffer,
	   size_t capFrameBufSize,
	   /*NEXUS_SurfaceHandle *picture,*/
	   NEXUS_PictureDecoderStatus *decoderStatus,
       NEXUS_PictureDecoderStartSettings *decoderStartSettings,
       NEXUS_PictureDecoderHandle pictureDecoder )
{
    void *buffer;
    uint32_t bufferSize;
	uint32_t jpegDataSize;
	int rc;
    NEXUS_SurfaceCreateSettings createSettings;

	BSTD_UNUSED(pVideoCamera);

    NEXUS_PictureDecoder_GetBuffer(pictureDecoder, &buffer, &bufferSize); /* get location and size of the decoder's buffer */

	BKNI_Memcpy(buffer, capFrameBuffer, capFrameBufSize);      /* copy captured frame into the decoder's buffer */

	if( capFrameBufSize < bufferSize )
    {
        rc = MjpegParser((unsigned char *)capFrameBuffer, capFrameBufSize,
			                     (unsigned char *)buffer, bufferSize,
                                  &jpegDataSize);
         if (rc)
         {
             fprintf(stderr, "(%s,%d): MjpegParser failed!\n",__FUNCTION__, __LINE__);
             return rc;
         }
    }
    else if( capFrameBufSize >= bufferSize )
    {
         /* Decoding larger buffers requires larger buffers. Modify NEXUS_PictureDecoderOpenSettings * bufferSize */
    	fprintf(stderr, "(%s,%d): Captured frame size[%u] is larger then buffer size[%u], not supported in DecodeJpeg\n",
             __FUNCTION__, __LINE__, capFrameBufSize, (unsigned)bufferSize);
        return -1;
    }

	decoderStartSettings->imageSize = capFrameBufSize;

    NEXUS_PictureDecoder_ReadComplete(pictureDecoder, 0, jpegDataSize); /* commit data to the decoder */

    NEXUS_PictureDecoder_Start(pictureDecoder, decoderStartSettings);

    do {
        NEXUS_PictureDecoder_GetStatus(pictureDecoder, decoderStatus);
        if( decoderStatus->state == NEXUS_PictureDecoderState_eError)
        {
            fprintf(stderr, "(%s,%d): Picture decoding failed !",__FUNCTION__, __LINE__);
            NEXUS_PictureDecoder_Stop(pictureDecoder);
            return -1;
        }
        BKNI_Sleep(5);
    } while(!decoderStatus->headerValid); /* wait for picture dimensions */

	/* create picture that could handle complete picture */
	NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
	createSettings.pixelFormat = decoderStatus->header.format;
	createSettings.width = decoderStatus->header.surface.width;
	createSettings.height = decoderStatus->header.surface.height;
	createSettings.alignment = 2;
    {
        NEXUS_PixelFormatInfo pixelFormatInfo;
        NEXUS_PixelFormat_GetInfo(createSettings.pixelFormat, &pixelFormatInfo);
        /* can't rely on default pitch 0 because we must round up to 4 byte alignment for SID */
        createSettings.pitch = (createSettings.width*pixelFormatInfo.bpp + 7) / 8;
        createSettings.pitch = ((createSettings.pitch + 3) & ~3);
        BDBG_ASSERT(createSettings.pitch % 4 == 0);
    }

	/* create surface to be used by the picture decoder */
	pVideoCamera->video.picture = NEXUS_Surface_Create(&createSettings);
	if (!pVideoCamera->video.picture)
	{
		fprintf(stderr, "Failed to create surface for picutre decoder (SID) use.\n");
		return -1;
	}

    /* copy palette if needed */
    if (NEXUS_PIXEL_FORMAT_IS_PALETTE(createSettings.pixelFormat)) {
        NEXUS_PictureDecoderPalette sidPalette;
        NEXUS_SurfaceMemory mem;

        rc = NEXUS_PictureDecoder_GetPalette(pictureDecoder, &sidPalette);
        BDBG_ASSERT(!rc);
        NEXUS_Surface_GetMemory(pVideoCamera->video.picture, &mem),
        BKNI_Memcpy(mem.palette, sidPalette.palette, mem.numPaletteEntries*sizeof(NEXUS_PixelFormat));
        NEXUS_Surface_Flush(pVideoCamera->video.picture);
    }

    /* start decoding and put into picture */
    NEXUS_PictureDecoder_DecodeSegment(pictureDecoder, pVideoCamera->video.picture, NULL);
    do {
        NEXUS_PictureDecoder_GetStatus(pictureDecoder, decoderStatus);
        if(decoderStatus->state == NEXUS_PictureDecoderState_eError)
        {
            fprintf(stderr, "(%s,%d): Picture decoding failed !",__FUNCTION__, __LINE__);
            NEXUS_PictureDecoder_Stop(pictureDecoder);
            return -1;

        }
        BKNI_Sleep(2);
    } while(decoderStatus->state != NEXUS_PictureDecoderState_eSegmentDone); /* wait for picture to decode */

    return 0;
}

/*
 * Stop SID video decoding
 * Return: 0 - success; other - fail.
 */
static int VideoCamera_StopVideoDecode
	 ( VideoCamera                *pVideoCamera,
	   NEXUS_PictureDecoderHandle  pictureDecoder)
{
    NEXUS_PictureDecoder_Stop(pictureDecoder);

	NEXUS_Surface_Destroy(pVideoCamera->video.picture);

	return 0;
}

#if 0
/*
 * Open VideoCamera's audio module
 * Return: 0 - success; other - fail.
 */
static int VideoCamera_OpenAudio
    ( VideoCamera *pVideoCamera )
{
    int fragSize = 0;
	int audioFd;
	int8_t *pReadPacketBuffer;
	VideoCameraSettings settings = pVideoCamera->settings;

    pReadPacketBuffer = (int8_t *)BKNI_Malloc(settings.audio.maxDataLength * sizeof(char));
    if(pReadPacketBuffer == NULL)
    {
        fprintf(stderr, "(%s,%d): Malloc memory failed!",__FUNCTION__, __LINE__);
        goto fail;
    }
	else
	{
		pVideoCamera->audio.readPacketBuffer = pReadPacketBuffer;
	}

    audioFd = open(settings.audio.audioDevName, O_RDONLY);
    if (audioFd < 0)
    {
        fprintf(stderr, "(%s,%d): Open audio device failed",__FUNCTION__, __LINE__);
        goto fail1;
    }
	else
	{
		pVideoCamera->audio.audioFd = audioFd;
	}

    if (ioctl(audioFd, SNDCTL_DSP_SETFMT, &settings.audio.format) == -1)
    {
        fprintf(stderr, "(%s,%d): SNDCTL_DSP_SETFMT set format failed",__FUNCTION__, __LINE__);
        goto fail2;
    }

    if (ioctl(audioFd, SNDCTL_DSP_CHANNELS, &settings.audio.channels) == -1)
    {
        fprintf(stderr, "(%s,%d):SNDCTL_DSP_CHANNELS set channel failed",__FUNCTION__, __LINE__);
        goto fail2;
    }

    if (ioctl(audioFd, SNDCTL_DSP_SPEED, &settings.audio.speed) == -1)
    {
        fprintf(stderr, "(%s,%d):SNDCTL_DSP_SPEED set speed failed",__FUNCTION__, __LINE__);
        goto fail2;
    }

    if (ioctl(audioFd, SNDCTL_DSP_GETBLKSIZE, &fragSize) == -1)
    {
        fprintf(stderr, "(%s,%d):SNDCTL_DSP_GETBLKSIZE get blksize failed ",__FUNCTION__, __LINE__);
    }

    if (ioctl(audioFd, SNDCTL_DSP_SETFRAGMENT, &settings.audio.newFrag) == -1)
    {
        fprintf(stderr, "(%s,%d):SNDCTL_DSP_SETFRAGMENT set fragment failed ",__FUNCTION__, __LINE__);
    }

	return 0;

fail2:
	close(audioFd);
fail1:
    BKNI_Free(pReadPacketBuffer);
fail:
    return -1;
}

/*
 * Close the VideoCamera's audio module.
 * Return: 0 - success; other - fail.
*/
static int VideoCamera_CloseAudio
	( VideoCamera *pVideoCamera)
{
    int ret = 0;

    BKNI_Free(pVideoCamera->audio.readPacketBuffer);
    pVideoCamera->audio.readPacketBuffer = NULL;

    if (close(pVideoCamera->audio.audioFd) < 0)
    {
    	fprintf(stderr, "(%s,%d):close audio device failed", __FUNCTION__, __LINE__);
        ret = -1;
    }
    else
    {
        pVideoCamera->audio.audioFd = -1;
    }

    return ret;
}

/*
 * Capture a PCM frame.
 * Return: 0 - success; other - fail.
*/

static int VideoCamera_CaptureAudioFrame
    ( VideoCamera *pVideoCamera,
      int8_t **pBuffer,
      int *pSize )
{
    int ret;
    int size = 0;

    BDBG_ASSERT( NULL != pVideoCamera);

    if (pVideoCamera->audio.audioFd < 0 || pVideoCamera->audio.readPacketBuffer == NULL )
    {
        fprintf(stderr, "(%s,%d): Must confirm if device open successfully", __FUNCTION__, __LINE__);
        return -1;
    }

   *pBuffer = pVideoCamera->audio.readPacketBuffer;

    size = read(pVideoCamera->audio.audioFd, *pBuffer, pVideoCamera->settings.audio.maxDataLength);
    if(size < 0)
    {
        *pSize = 0;
        if(errno == EAGAIN)
        {
            ret = 0;
        }
        else
        {
            fprintf(stderr, "(%s,%d): Read device failed",__FUNCTION__, __LINE__);
            ret = -1;
        }
    }
    else
    {
        *pSize = size;
        ret = 0;
    }

    return ret;
}
#endif


static int VideoCamera_Open
	( VideoCamera          **pVideoCamera,
	  VideoCameraSettings   *pVideoCameraSettings )
{
	int ret;
	VideoCamera *pVidCam;

	pVidCam = (VideoCamera *)BKNI_Malloc(sizeof(VideoCamera));
	if (!pVidCam)
	{
		fprintf(stderr, "Failed to allocate memory for VideoCamera object.\n");
		return -1;
	}

	pVidCam->settings = *pVideoCameraSettings;

	ret = VideoCamera_OpenVideo(pVidCam);
	if (ret != 0)
	{
		fprintf(stderr, "Failed to open video device\n");
		return ret;
	}

#if 0
	ret = VideoCamera_OpenAudio(pVidCam);
	if (ret != 0)
	{
		fprintf(stderr, "Failed to open audio device\n");
		VideoCamera_CloseVideo(pVidCam);
		return ret;
	}
#endif

	*pVideoCamera = pVidCam;

	return 0;
}

static int VideoCamera_Close
	( VideoCamera *pVideoCamera )
{
	int ret = 0;

	ret = VideoCamera_CloseVideo(pVideoCamera);
	if (ret != 0)
	{
		fprintf(stderr, "Failed to close video device\n");
		return ret;
	}

#if 0
	ret = VideoCamera_CloseAudio(pVideoCamera);
	if (ret != 0)
	{
		fprintf(stderr, "Failed to close audio device\n");
		return ret;
	}
#endif
	BKNI_Free(pVideoCamera);

	return ret;
}

void HttpStreaming_WaitForNetworkEvent
	( Transcoder *pTranscoder,
	  int         sd )
{
	fd_set rfds;
	struct timeval tv;

	while (!pTranscoder->httpStreaming.exitThread) {
		FD_ZERO(&rfds);
		FD_SET(sd, &rfds);
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		if ( select(sd +1, &rfds, NULL, NULL, &tv) < 0 ) {
			perror("ERROR: select(): exiting...");
			break;
		}

		if (!FD_ISSET(sd, &rfds))
			/* No request from Client yet, go back to select loop */
			continue;
		break;
	}

    return;
}

static void * HttpStreaming_Output
	( void *context )
{
    Transcoder *pTranscoder = (Transcoder *)context;
    int listeningFd, sockFd;
    struct sockaddr_in localAddr;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    int reuse_flag = 1;
	const void *rec_buffer;
    size_t rec_size;
    NEXUS_Error rc;

    BDBG_MSG(("Starting HTTP send "));
    if ((listeningFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        /* Socket Create Error */
        perror("Socket Open Err");
        goto error;
    }
    if (setsockopt(listeningFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_flag, sizeof(reuse_flag) ) < 0 ) {
        BDBG_ERR(("REUSE Socket Error"));
        goto error;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(pTranscoder->httpStreaming.listeningPort);
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listeningFd, (struct sockaddr *) &localAddr, sizeof(localAddr))) {
        perror("Socket Bind Err to bind to actual i/f");
        goto error;
    }
    if (listen(listeningFd, 8)) {
        perror("Socket listen Err");
        goto error;
    }
    /* Set stdin to non-blocking */
    if (fcntl(listeningFd, F_SETFL, fcntl(listeningFd, F_GETFL)|O_NONBLOCK) < 0)
        BDBG_WRN(("Failed to set non-blocking mode on listening socket"));

    while (!pTranscoder->httpStreaming.exitThread)
	{
        HttpStreaming_WaitForNetworkEvent(pTranscoder, listeningFd);
        /* accept connection */
        if ((sockFd = accept(listeningFd, NULL, NULL)) < 0)
		{
            perror("ERROR: accept():");
            goto error;
        }

        if (getsockname(sockFd, (struct sockaddr *)&addr, &addrlen))
		{
            perror("getsockname():");
            close(sockFd);
            goto error;
        }

        rc = NEXUS_Recpump_GetDataBuffer(pTranscoder->mux.recpump, &rec_buffer, &rec_size);
        BDBG_ASSERT(!rc);

        if (send(sockFd, (void*)rec_buffer, rec_size, MSG_NOSIGNAL) < 0)
		{
            BDBG_MSG(("Failed to send %d bytes of channel map data, errno %d", rec_size, errno));
        }
        BDBG_WRN(("%s: send %d bytes of transcoded data", __FUNCTION__, rec_size));

        /* calling NEXUS_Recpump_DataReadComplete before the XPT HW has actually read the data. this
        is ok because the recpump buffer will generally be empty (wrap around unlikely) */
        rc = NEXUS_Recpump_DataReadComplete(pTranscoder->mux.recpump, rec_size);
        BDBG_ASSERT(!rc);

        close(sockFd);
    }

error:
    BDBG_MSG(("Exiting Channel Map Thread"));
    close(listeningFd);
    pthread_exit(NULL);
}

#if 0
static void *HttpStreaming_Input
	( void * context )
{

}
#endif

static void
TranscoderMux_FinishCallback(void *context, int param)
{
	BKNI_EventHandle finishEvent = (BKNI_EventHandle)context;

	BSTD_UNUSED(param);
	BDBG_WRN(("Finish callback invoked, now stop the stream mux."));
	BKNI_SetEvent(finishEvent);
}

void TranscoderMux_Add_Pat_Pmt
    ( Transcoder *pTranscoder,
      uint16_t    pcrPid,
      uint16_t    vidPid,
      uint16_t    audPid,
      uint8_t     vidStreamType,
      uint8_t     audStreamType )
{
    uint8_t pat_pl_buf[TS_HEADER_BUF_LENGTH], pmt_pl_buf[TS_HEADER_BUF_LENGTH];
    size_t pat_pl_size, pmt_pl_size;
    size_t buf_used = 0;
    size_t payload_pked = 0;
    unsigned streamNum;
    int i;

    TS_PAT_state patState;
    TS_PSI_header psi_header;
    TS_PMT_state pmtState;
    TS_PAT_program program;
    TS_PMT_stream pmt_stream;

    TS_PID_info pidInfo;
    TS_PID_state pidState;

    /* == CREATE PSI TABLES == */
    TS_PSI_header_Init(&psi_header);
    TS_PAT_Init(&patState, &psi_header, pat_pl_buf, TS_HEADER_BUF_LENGTH);

    TS_PAT_program_Init(&program, 1, MUX_PMT_PID);
    TS_PAT_addProgram(&patState, &pmtState, &program, pmt_pl_buf, TS_HEADER_BUF_LENGTH);

    TS_PMT_setPcrPid(&pmtState, pcrPid);

    TS_PMT_stream_Init(&pmt_stream, vidStreamType, vidPid);
    TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);

    if(audPid)
    {
        TS_PMT_stream_Init(&pmt_stream, audStreamType, audPid);
        TS_PMT_addStream(&pmtState, &pmt_stream, &streamNum);
    }

    TS_PAT_finalize(&patState, &pat_pl_size);
    TS_PMT_finalize(&pmtState, &pmt_pl_size);
    BDBG_MSG(("\nOutput PMT section:"));
    for(i=0; i < (int)pmtState.size; i+=8) {
        BDBG_MSG(("%02x %02x %02x %02x %02x %02x %02x %02x", pmtState.buf[i], pmtState.buf[i+1], pmtState.buf[i+2], pmtState.buf[i+3],
            pmtState.buf[i+4], pmtState.buf[i+5], pmtState.buf[i+6], pmtState.buf[i+7]));
    }

    /* == CREATE TS HEADERS FOR PSI INFORMATION == */
    TS_PID_info_Init(&pidInfo, 0x0, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pTranscoder->pat[0], TS_HEADER_BUF_LENGTH, &buf_used, patState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pTranscoder->pat[0] + buf_used, pat_pl_buf, pat_pl_size);

    TS_PID_info_Init(&pidInfo, MUX_PMT_PID, 1);
    pidInfo.pointer_field = 1;
    TS_PID_state_Init(&pidState);
    TS_buildTSHeader(&pidInfo, &pidState, pTranscoder->pmt[0], TS_HEADER_BUF_LENGTH, &buf_used, pmtState.size, &payload_pked, 1);
    BKNI_Memcpy((uint8_t*)pTranscoder->pmt[0] + buf_used, pmt_pl_buf, pmt_pl_size);
    BDBG_MSG(("\nOutput PMT packet:"));
    for(i=0; i < TS_HEADER_BUF_LENGTH; i+=8) {
        BDBG_MSG(("%02x %02x %02x %02x %02x %02x %02x %02x",
            *((uint8_t*)pTranscoder->pmt[0]+i), *((uint8_t*)pTranscoder->pmt[0]+i+1), *((uint8_t*)pTranscoder->pmt[0]+i+2), *((uint8_t*)pTranscoder->pmt[0]+i+3),
            *((uint8_t*)pTranscoder->pmt[0]+i+4), *((uint8_t*)pTranscoder->pmt[0]+i+5), *((uint8_t*)pTranscoder->pmt[0]+i+6), *((uint8_t*)pTranscoder->pmt[0]+i+7)));
    }
}

#if 0
static void TranscoderMux_InsertSystemDataTimer(void *context)
{
	Transcoder *pTranscoder = context;
	uint8_t ccByte;

	++pTranscoder->ccValue;/* increment CC synchronously with PAT/PMT */
	ccByte = *((uint8_t*)pTranscoder->pat[pTranscoder->ccValue % PSI_QUEUE_CNT] + 4); /* the 1st byte of pat/pmt arrays is for TSheader builder use */

	/* need to increment CC value for PAT/PMT packets */
	ccByte = (ccByte & 0xf0) | (pTranscoder->ccValue & 0xf);
	*((uint8_t*)pTranscoder->pat[pTranscoder->ccValue % PSI_QUEUE_CNT] + 4) = ccByte;
	/* need to flush the cache before set to TS mux hw */
	NEXUS_Memory_FlushCache((void*)((uint8_t*)pTranscoder->pat[pTranscoder->ccValue % PSI_QUEUE_CNT] + 4), 1);
	/* ping pong PAT pointer */
	pTranscoder->psi[0].pData = (void*)((uint8_t*)pTranscoder->pat[pTranscoder->ccValue % PSI_QUEUE_CNT] + 1);

	ccByte = *((uint8_t*)pTranscoder->pmt[pTranscoder->ccValue % PSI_QUEUE_CNT] + 4);
	ccByte = (ccByte & 0xf0) | (pTranscoder->ccValue & 0xf);
	*((uint8_t*)pTranscoder->pmt[pTranscoder->ccValue % PSI_QUEUE_CNT] + 4) = ccByte;
	NEXUS_Memory_FlushCache((void*)((uint8_t*)pTranscoder->pmt[pTranscoder->ccValue % PSI_QUEUE_CNT] + 4), 1);
	/* ping pong PMT pointer */
	pTranscoder->psi[1].pData = (void*)((uint8_t*)pTranscoder->pmt[pTranscoder->ccValue % PSI_QUEUE_CNT] + 1);

	NEXUS_StreamMux_AddSystemDataBuffer(pTranscoder->mux.streamMux, &pTranscoder->psi[0]);
	NEXUS_StreamMux_AddSystemDataBuffer(pTranscoder->mux.streamMux, &pTranscoder->psi[1]);
	BDBG_MODULE_MSG(contcounter,("Insert PAT&PMT... ccPAT = %x ccPMT=%x", *((uint8_t*)pTranscoder->pat[pTranscoder->ccValue % PSI_QUEUE_CNT] + 4) & 0xf,
		*((uint8_t*)pTranscoder->pmt[pTranscoder->ccValue  % PSI_QUEUE_CNT] + 4) & 0xf));
	if(pTranscoder->systemdataTimerIsStarted)
	{
		pTranscoder->systemdataTimer = B_Scheduler_StartTimer(
			pTranscoder->schedulerSystemdata,pTranscoder->mutexSystemdata, 1000, TranscoderMux_InsertSystemDataTimer, pTranscoder);
		if(pTranscoder->systemdataTimer==NULL) {BDBG_ERR(("schedule timer error %d", NEXUS_OUT_OF_SYSTEM_MEMORY));}
	}
	return;
}

/*******************************
 * Add system data to stream_mux
 */
static void TranscoderMux_Create_SystemData( Transcoder *pTranscoder )
{
	uint8_t vidStreamType, audStreamType = 0;
	uint16_t audPid = 0;
	unsigned i;

	for(i=0; i<PSI_QUEUE_CNT; i++)
	{
		NEXUS_Memory_Allocate(TS_HEADER_BUF_LENGTH, NULL, &pTranscoder->pat[i]);
		NEXUS_Memory_Allocate(TS_HEADER_BUF_LENGTH, NULL, &pTranscoder->pmt[i]);
	}

	/* decide the stream type to set in PMT */
	switch(pTranscoder->transcoderSettings.codec)
	{
		case NEXUS_VideoCodec_eMpeg2:         vidStreamType = 0x2; break;
		case NEXUS_VideoCodec_eMpeg4Part2:    vidStreamType = 0x10; break;
		case NEXUS_VideoCodec_eH264:          vidStreamType = 0x1b; break;
		case NEXUS_VideoCodec_eVc1SimpleMain: vidStreamType = 0xea; break;
		default:
			BDBG_ERR(("Video encoder codec %d is not supported!\n", pTranscoder->transcoderSettings.codec));
			BDBG_ASSERT(0);
	}

	TranscoderMux_Add_Pat_Pmt(pTranscoder, MUX_PCR_PID, MUX_VIDEO_PID, audPid, vidStreamType, audStreamType);
	for(i=0; i<PSI_QUEUE_CNT; i++)
	{
		if(i > 0)
		{
			BKNI_Memcpy(pTranscoder->pat[i], pTranscoder->pat[0], TS_HEADER_BUF_LENGTH);
			BKNI_Memcpy(pTranscoder->pmt[i], pTranscoder->pmt[0], TS_HEADER_BUF_LENGTH);
		}
		NEXUS_Memory_FlushCache(pTranscoder->pat[i], TS_HEADER_BUF_LENGTH);
		NEXUS_Memory_FlushCache(pTranscoder->pmt[i], TS_HEADER_BUF_LENGTH);
	}
	BKNI_Memset(pTranscoder->psi, 0, sizeof(pTranscoder->psi));
	pTranscoder->psi[0].size = 188;
	/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
	pTranscoder->psi[0].pData = (void*)((uint8_t*)pTranscoder->pat[0] + 1);
	pTranscoder->psi[0].timestampDelta = 0;
	pTranscoder->psi[1].size = 188;
	/* TSHDRBUILDER has one extra byte at the beginning to describe the variable length TS header buffer */
	pTranscoder->psi[1].pData = (void*)((uint8_t*)pTranscoder->pmt[0] + 1);
	pTranscoder->psi[1].timestampDelta = 0;
	NEXUS_StreamMux_AddSystemDataBuffer(pTranscoder->mux.streamMux, &pTranscoder->psi[0]);
	NEXUS_StreamMux_AddSystemDataBuffer(pTranscoder->mux.streamMux, &pTranscoder->psi[1]);
	BDBG_MSG(("insert PAT&PMT... ccPAT = %x ccPMT=%x", *((uint8_t*)pTranscoder->pat[0] + 4) & 0xf,
		*((uint8_t*)pTranscoder->pmt[0] + 4) & 0xf));

}

static void TranscoderMux_Start_SystemData( Transcoder *pTranscoder )
{
	if(!pTranscoder->systemdataTimerIsStarted) {
		/* schedule a periodic timer to insert PAT/PMT */
		B_ThreadSettings settingsThread;
		pTranscoder->mutexSystemdata = B_Mutex_Create(NULL);
		pTranscoder->schedulerSystemdata = B_Scheduler_Create(NULL);
		/* create thread to run scheduler */
		B_Thread_GetDefaultSettings(&settingsThread);
		pTranscoder->schedulerThread = B_Thread_Create("systemdata_Scheduler",
			(B_ThreadFunc)B_Scheduler_Run,
			pTranscoder->schedulerSystemdata,
			&settingsThread);
		if (NULL == pTranscoder->schedulerThread)
		{
			BDBG_ERR(("failed to create scheduler thread"));
		}
		pTranscoder->systemdataTimer = B_Scheduler_StartTimer(
			pTranscoder->schedulerSystemdata,pTranscoder->mutexSystemdata, 1000, TranscoderMux_InsertSystemDataTimer, pTranscoder);
		if(pTranscoder->systemdataTimer==NULL) {BDBG_ERR(("schedule timer error"));}
		pTranscoder->systemdataTimerIsStarted = true;
	}
}

static void TranscoderMux_Stop_SystemData( Transcoder  *pTranscoder )
{
	/* cancel system data timer */
	if(pTranscoder->systemdataTimerIsStarted)
	{
		B_Scheduler_CancelTimer(pTranscoder->schedulerSystemdata, pTranscoder->systemdataTimer);
		B_Scheduler_Stop(pTranscoder->schedulerSystemdata);
		B_Scheduler_Destroy(pTranscoder->schedulerSystemdata);
		if (pTranscoder->schedulerThread)
		{
			B_Thread_Destroy(pTranscoder->schedulerThread);
			pTranscoder->schedulerThread = NULL;
		}
		B_Mutex_Destroy(pTranscoder->mutexSystemdata);
		pTranscoder->systemdataTimer = NULL;
		pTranscoder->systemdataTimerIsStarted = false;
	}
}

static void TranscoderMux_Release_SystemData( Transcoder  *pTranscoder )
{
	unsigned i;

	for(i=0; i<PSI_QUEUE_CNT; i++)
	{
		NEXUS_Memory_Free(pTranscoder->pat[i]);
		NEXUS_Memory_Free(pTranscoder->pmt[i]);
	}
}
#endif

/***************************************************************************
Summary: Open stream mux
    None.

Description:
    None.

See Also:
    None.
**************************************************************************/
static void TranscoderMux_Open( Transcoder  *pTranscoder )
{
	NEXUS_StreamMuxCreateSettings muxCreateSettings;
	NEXUS_StreamMuxStartSettings muxConfig;

	NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
	NEXUS_RecpumpOpenSettings recpumpOpenSettings;

	NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
	playpumpOpenSettings.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
	playpumpOpenSettings.numDescriptors = 64; /* set number of descriptors */
	playpumpOpenSettings.streamMuxCompatible = true;
	pTranscoder->mux.videoPlaypump = NEXUS_Playpump_Open(STREAM_MUX_VIDEO_PLAYPUMP_IDX, &playpumpOpenSettings);
	BDBG_ASSERT(pTranscoder->mux.videoPlaypump);
	BDBG_MSG(("Transcoder opened TS mux video PES playpump %d [%p].", STREAM_MUX_VIDEO_PLAYPUMP_IDX, (void*)pTranscoder->mux.videoPlaypump));
#if 0
	NEXUS_Playpump_SuspendPacing(pTranscoder->mux.videoPlaypump, true);
#endif

	pTranscoder->mux.pcrPlaypump = NEXUS_Playpump_Open(STREAM_MUX_PCR_PLAYPUMP_IDX, &playpumpOpenSettings);
	BDBG_MSG(("Transcoder opened TS mux PCR playpump %d [%p].", STREAM_MUX_PCR_PLAYPUMP_IDX, (void*)pTranscoder->mux.pcrPlaypump));
	BDBG_ASSERT(pTranscoder->mux.pcrPlaypump);

	BKNI_CreateEvent(&pTranscoder->mux.event);
	NEXUS_StreamMux_GetDefaultCreateSettings(&muxCreateSettings);
	muxCreateSettings.finished.callback = TranscoderMux_FinishCallback;
	muxCreateSettings.finished.context = pTranscoder->mux.event;
	pTranscoder->mux.streamMux = NEXUS_StreamMux_Create(&muxCreateSettings);
	BDBG_ASSERT(pTranscoder->mux.streamMux);

	NEXUS_StreamMux_GetDefaultStartSettings(&muxConfig);
	muxConfig.transportType = NEXUS_TransportType_eTs;
	muxConfig.stcChannel = pTranscoder->encoderStcChannel;
	muxConfig.nonRealTime = false;
	muxConfig.nonRealTimeRate = 32 * NEXUS_NORMAL_PLAY_SPEED; /* AFAP */

	muxConfig.video[0].pid = MUX_VIDEO_PID;
	muxConfig.video[0].encoder = pTranscoder->videoEncoder;
	muxConfig.video[0].playpump = pTranscoder->mux.videoPlaypump;

#if 0
	/* audio playpump here is for ts muxer */
	NEXUS_Playpump_GetDefaultOpenSettings(&playpumpConfig);
	playpumpOpenSettings.fifoSize = 16384; /* reduce FIFO size allocated for playpump */
	playpumpOpenSettings.numDescriptors = 64; /* set number of descriptors */
	playpumpOpenSettings.streamMuxCompatible = true;
	pTranscoder->mux.audioPlaypump = NEXUS_Playpump_Open(STREAM_MUX_AUDIO_PLAYPUMP_IDX, &playpumpOpenSettings);
	BDBG_ASSERT(pTranscoder->mux.audioPlaypump);
	BDBG_MSG(("Transcoder opened TS mux audio PES playpump%d [%p].", STREAM_MUX_AUDIO_PLAYPUMP_IDX, (void*)pTranscoder->mux.audioPlaypump));

	muxConfig.audio[0].pid = MUX_AUDIO_PID;
	muxConfig.audio[0].muxOutput = pTranscoder->audioMuxOutput;
	muxConfig.audio[0].playpump = pTranscoder->mux.audioPlaypump;
#endif

	muxConfig.pcr.pid = MUX_PCR_PID;
	muxConfig.pcr.playpump = pTranscoder->mux.pcrPlaypump;
	muxConfig.pcr.interval = 50;

	/******************************************
	 * Set up transcoder recpump output
	 */
	NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);

#if 1
	/******************************************
	 *  NRT workaround for the XPT band hold and data ready events sharing the same threshold: avoid band hold from occuring,
	 * otherwise, video stutter would happen!
	 * 1) The nexus_record timer fires at 250 ms interval to service record data as bottom line;
	 * 2) whenever nexus record timer fires, it'll consume up to Nx(3x(47x4096)) cdb data;
	 * 3) so if file i/o can keep up, band hold threshold = 2x(3x(47x4096)) = 1.1MB can sustain record bit rate up to
	 *       2 * 3 * 188K * 8 / 250ms = 36 Mbps without reaching band hold;
	 * 4) larger band hold threshold can sustain higher record bit rate throughput.
	 */

	/* CDB dataReadyThreshold is page and atom size aligned */
	recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.bufferSize * 50/100;
	recpumpOpenSettings.data.dataReadyThreshold =
		(recpumpOpenSettings.data.dataReadyThreshold < 2 * RECORD_WRITE_SIZE_LIMIT)
		? (2 * RECORD_WRITE_SIZE_LIMIT) : recpumpOpenSettings.data.dataReadyThreshold;

	/* ITB dataready threshold is 90% of ITB buffer size */
	recpumpOpenSettings.index.dataReadyThreshold = recpumpOpenSettings.index.bufferSize * 50/100;

	/* round down CDB dataready threshold to atom aligned for DirectIO and TS packets record */
	recpumpOpenSettings.data.dataReadyThreshold -= recpumpOpenSettings.data.dataReadyThreshold % RECORD_PAGE_SIZE;

	BDBG_ERR(("To open recpump with dataReadyThreshold = %d indexReadyThreshold=%d",
		recpumpOpenSettings.data.dataReadyThreshold, recpumpOpenSettings.index.dataReadyThreshold));
	BDBG_ERR(("        recpump with data fifo size     = %d index fifo size    =%d",
		recpumpOpenSettings.data.bufferSize, recpumpOpenSettings.index.bufferSize));
	BDBG_ERR(("        recpump with atom size     = %d",
		recpumpOpenSettings.data.atomSize));
#endif

	pTranscoder->mux.recpump = NEXUS_Recpump_Open(STREAM_MUX_RECPUMP_IDX, &recpumpOpenSettings);
	BDBG_ASSERT(pTranscoder->mux.recpump);
	BDBG_MSG(("Transcoder opened TS mux recpump%d [%p].",  STREAM_MUX_RECPUMP_IDX, (void*)pTranscoder->mux.recpump));

#if 0
	/*******************************
	 * create system data PAT/PMT
	 */
	TranscoderMux_Create_SystemData(pTranscoder);
#endif

	/* store the mux config */
	pTranscoder->mux.config = muxConfig;

	/* open PidChannels */
	pTranscoder->mux.pcrPidChannel = NEXUS_Playpump_OpenPidChannel(pTranscoder->mux.pcrPlaypump, muxConfig.pcr.pid, NULL);

	BDBG_ASSERT(pTranscoder->mux.pcrPidChannel);

#if 0
	pTranscoder->mux.pmtPidChannel = NEXUS_Playpump_OpenPidChannel(pTranscoder->mux.pcrPlaypump, MUX_PMT_PID, NULL);
	BDBG_ASSERT(pTranscoder->mux.pmtPidChannel);
	pTranscoder->mux.patPidChannel = NEXUS_Playpump_OpenPidChannel(pTranscoder->mux.pcrPlaypump, MUX_PAT_PID, NULL);
	BDBG_ASSERT(pTranscoder->mux.patPidChannel);
#endif

}

/***************************************************************************
Summary: Close stream mux
    None.

Description:
    None.

See Also:
    None.
**************************************************************************/
static void TranscoderMux_Close( Transcoder  *pTranscoder )
{
	NEXUS_Playpump_ClosePidChannel(pTranscoder->mux.pcrPlaypump, pTranscoder->mux.pcrPidChannel);
	NEXUS_Recpump_Close(pTranscoder->mux.recpump);
	NEXUS_StreamMux_Destroy(pTranscoder->mux.streamMux);
	NEXUS_Playpump_Close(pTranscoder->mux.pcrPlaypump);
	NEXUS_Playpump_Close(pTranscoder->mux.videoPlaypump);
}

#if 0
static void overflow_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    printf("overflow %s\n", (const char *)context);
}
#endif
static void *TranscoderLoopback_Pump_Data( void *context )
{
    Transcoder *pTranscoder = context;
	NEXUS_RecpumpStatus rStatus;
	NEXUS_PlaypumpStatus pStatus;

    while (!pTranscoder->loopback.done)
	{
        const void *rec_buffer;
        NEXUS_PlaypumpScatterGatherDescriptor desc;
        size_t rec_size;
        unsigned num;
        NEXUS_Error rc;

        rc = NEXUS_Recpump_GetDataBuffer(pTranscoder->mux.recpump, &rec_buffer, &rec_size);
        BDBG_ASSERT(!rc);

        if (!rec_size)
		{
			printf("waiting ...");
            if (BKNI_WaitForEvent(pTranscoder->loopback.event, 1000) == BERR_TIMEOUT)
            {
				printf("time out\n");
            }
			else
			{
			    printf(" got event\n");
			}
			NEXUS_Recpump_GetStatus(pTranscoder->mux.recpump, &rStatus);
			printf("recpump status: \n");
		    printf("  data:  %u total bytes \tfifo %u/%u\n", (unsigned)rStatus.data.bytesRecorded, rStatus.data.fifoDepth, rStatus.data.fifoSize);
    		printf("  index: %u total bytes \tfifo %u/%u\n", (unsigned)rStatus.index.bytesRecorded, rStatus.index.fifoDepth, rStatus.index.fifoSize);

			NEXUS_Playpump_GetStatus(pTranscoder->loopback.playpump, &pStatus);
			printf("playpump status: \n");
		    printf("  data:  %u total bytes \tfifo %u/%u\n", (unsigned)pStatus.bytesPlayed, pStatus.fifoDepth, pStatus.fifoSize);
			printf("  descriptor:  %u depth \tfifo %u\n",  pStatus.descFifoDepth, pStatus.descFifoSize);

            continue;
        }

        /* use scatter/gather to go from record buffer directly to playback HW */
        desc.addr = (void*)rec_buffer;
        desc.length = rec_size;

        rc = NEXUS_Playpump_SubmitScatterGatherDescriptor(pTranscoder->loopback.playpump, &desc, 1, &num);
        BDBG_ASSERT(!rc);
        if (!num) {
            BKNI_WaitForEvent(pTranscoder->loopback.event, 1000);
            continue;
        }

        /* calling NEXUS_Recpump_DataReadComplete before the XPT HW has actually read the data. this
        is ok because the recpump buffer will generally be empty (wrap around unlikely) */
        rc = NEXUS_Recpump_DataReadComplete(pTranscoder->mux.recpump, rec_size);
        BDBG_ASSERT(!rc);
    }
    return NULL;
}

static void TranscoderLoopback_Dataready_Callback
	( void *context,
	  int param )
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}


void TranscoderLoopback_Playback_Start
	( Transcoder *pTranscoder )
{
    NEXUS_Error rc;
    NEXUS_RecpumpSettings recpumpSettings;
	NEXUS_RecpumpAddPidChannelSettings recPumpAddPidChannelSettings;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;

    BKNI_CreateEvent(&pTranscoder->loopback.event);

	NEXUS_Recpump_GetDefaultAddPidChannelSettings(&recPumpAddPidChannelSettings);
	recPumpAddPidChannelSettings.pidType = NEXUS_PidType_eVideo;
	recPumpAddPidChannelSettings.pidTypeSettings.video.index = /* RAVE only support mpeg2&avc indexing */
			(pTranscoder->transcoderSettings.codec==NEXUS_VideoCodec_eMpeg2) ||
			(pTranscoder->transcoderSettings.codec==NEXUS_VideoCodec_eH264);
	recPumpAddPidChannelSettings.pidTypeSettings.video.codec = pTranscoder->transcoderSettings.codec;
	rc = NEXUS_Recpump_AddPidChannel(pTranscoder->mux.recpump, pTranscoder->mux.videoPidChannel, &recPumpAddPidChannelSettings);
	BDBG_ASSERT(!rc);

	recPumpAddPidChannelSettings.pidType = NEXUS_PidType_eOther;
	rc = NEXUS_Recpump_AddPidChannel(pTranscoder->mux.recpump, pTranscoder->mux.pcrPidChannel, NULL);
	BDBG_ASSERT(!rc);
#if 0
	NEXUS_Recpump_AddPidChannel(pTranscoder->mux.recpump, pTranscoder->mux.pmtPidChannel, &recPumpAddPidChannelSettings);
	NEXUS_Recpump_AddPidChannel(pTranscoder->mux.recpump, pTranscoder->mux.patPidChannel, &recPumpAddPidChannelSettings);
	NEXUS_Recpump_AddPidChannel(pTranscoder->mux.recpump, pTranscoder->mux.audioPidChannel, NULL);
#endif

    NEXUS_Recpump_GetSettings(pTranscoder->mux.recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = TranscoderLoopback_Dataready_Callback;
    recpumpSettings.data.dataReady.context = pTranscoder->loopback.event;
#if 0
    recpumpSettings.data.overflow.callback = overflow_callback;
    recpumpSettings.data.overflow.context = "data";
    recpumpSettings.index.overflow.callback = overflow_callback;
    recpumpSettings.index.overflow.context = "index";
#endif
 	recpumpSettings.pcrPidChannel = pTranscoder->mux.pcrPidChannel;
	recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
    rc = NEXUS_Recpump_SetSettings(pTranscoder->mux.recpump, &recpumpSettings);
	BDBG_ASSERT(!rc);

	rc = NEXUS_Recpump_Start(pTranscoder->mux.recpump);
    BDBG_ASSERT(!rc);

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpumpOpenSettings.fifoSize = 1024; /* only playing descriptors */
    pTranscoder->loopback.playpump = NEXUS_Playpump_Open(LOOPBACK_PLAYPUMP_IDX, &playpumpOpenSettings);
	BDBG_ASSERT(pTranscoder->loopback.playpump);

	NEXUS_Playpump_GetSettings(pTranscoder->loopback.playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = TranscoderLoopback_Dataready_Callback;
    playpumpSettings.dataCallback.context = pTranscoder->loopback.event;
    NEXUS_Playpump_SetSettings(pTranscoder->loopback.playpump, &playpumpSettings);

	rc = NEXUS_Playpump_Start(pTranscoder->loopback.playpump);
    BDBG_ASSERT(!rc);

    pTranscoder->loopback.videoPidChannel = NEXUS_Playpump_OpenPidChannel(pTranscoder->loopback.playpump, MUX_VIDEO_PID, NULL);
    pTranscoder->loopback.pcrPidChannel =  NEXUS_Playpump_OpenPidChannel(pTranscoder->loopback.playpump, MUX_PCR_PID, NULL);
#if 0
	pTranscoder->loopback.pmtPidChannel =  NEXUS_Playpump_OpenPidChannel(pTranscoder->loopback.playpump, MUX_PMT_PID, NULL);
    pTranscoder->loopback.patPidChannel =  NEXUS_Playpump_OpenPidChannel(pTranscoder->loopback.playpump, MUX_PAT_PID, NULL);
	pTranscoder->loopback.audioPidChannel = NEXUS_Playpump_OpenPidChannel(pTranscoder->loopback.playpump, g_channels[channel].audioPid, NULL);*/
#endif

    /* start dedicated thread for pumping data */
    pTranscoder->loopback.done = false;
    rc = pthread_create(&pTranscoder->loopback.thread, NULL, TranscoderLoopback_Pump_Data, pTranscoder);
    BDBG_ASSERT(!rc);

}

void TranscoderLoopback_Playback_Stop
	( Transcoder *pTranscoder )
{
    pTranscoder->loopback.done = true;
    BKNI_SetEvent(pTranscoder->loopback.event);
    pthread_join(pTranscoder->loopback.thread, NULL);

    NEXUS_Playpump_ClosePidChannel(pTranscoder->loopback.playpump, pTranscoder->loopback.videoPidChannel);
    NEXUS_Playpump_ClosePidChannel(pTranscoder->loopback.playpump, pTranscoder->loopback.pcrPidChannel);

#if 0
	NEXUS_Playpump_ClosePidChannel(pTranscoder->loopback.playpump, pTranscoder->loopback.audioPidChannel);
	NEXUS_Playpump_ClosePidChannel(pTranscoder->loopback.playpump, pTranscoder->loopback.patPidChannel);
	NEXUS_Playpump_ClosePidChannel(pTranscoder->loopback.playpump, pTranscoder->loopback.pmtPidChannel);
#endif

    NEXUS_Recpump_RemoveAllPidChannels(pTranscoder->mux.recpump);
	NEXUS_Recpump_Stop(pTranscoder->mux.recpump);
#if 0
    NEXUS_PidChannel_Close(pTranscoder->loopback.audioPidChannel);
#endif

    NEXUS_Playpump_Close(pTranscoder->loopback.playpump);
    BKNI_DestroyEvent(pTranscoder->loopback.event);
}

#if RECORD_TO_FILE

void TranscoderRecord_Start
	( Transcoder *pTranscoder )
{
    NEXUS_Error rc;
	NEXUS_RecordPidChannelSettings recordPidSettings;
    NEXUS_RecordSettings recordSettings;

	if((pTranscoder->transcoderSettings.codec==NEXUS_VideoCodec_eMpeg2) ||
	   (pTranscoder->transcoderSettings.codec==NEXUS_VideoCodec_eH264))
	{
		strcpy(pTranscoder->record.indexfname, "encode.nav");
	}

	pTranscoder->record.record = NEXUS_Record_Create();
	BDBG_ASSERT(pTranscoder->record.record);

	NEXUS_Record_GetSettings(pTranscoder->record.record, &recordSettings);
	recordSettings.recpump = pTranscoder->mux.recpump;
	/* NOTE: enable band hold to allow recpump to stall TS mux in NRT mode in case file i/o is slower! */
	recordSettings.recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eEnable;
	recordSettings.pollingTimer = 50;
	NEXUS_Record_SetSettings(pTranscoder->record.record, &recordSettings);

	{
		NEXUS_RecpumpStatus status;
        NEXUS_Recpump_GetStatus(recordSettings.recpump, &status);
	}

	/* configure the video pid for indexing */
	NEXUS_Record_GetDefaultPidChannelSettings(&recordPidSettings);
	recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
	recordPidSettings.recpumpSettings.pidTypeSettings.video.index = /* RAVE only support mpeg2&avc indexing */
			(pTranscoder->transcoderSettings.codec==NEXUS_VideoCodec_eMpeg2) ||
			(pTranscoder->transcoderSettings.codec==NEXUS_VideoCodec_eH264);
	recordPidSettings.recpumpSettings.pidTypeSettings.video.codec = pTranscoder->transcoderSettings.codec;
	rc = NEXUS_Record_AddPidChannel(pTranscoder->record.record, pTranscoder->mux.videoPidChannel, &recordPidSettings);
	BDBG_ASSERT(!rc);

	recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eOther;
	rc = NEXUS_Record_AddPidChannel(pTranscoder->record.record, pTranscoder->mux.pcrPidChannel, &recordPidSettings);
	BDBG_ASSERT(!rc);

    pTranscoder->record.transcodeFile = NEXUS_FileRecord_OpenPosix(pTranscoder->record.fname,
			pTranscoder->record.indexfname[0]? pTranscoder->record.indexfname : NULL);

	BDBG_ASSERT(pTranscoder->record.transcodeFile);

	/* Start record of stream mux output */
	NEXUS_Record_Start(pTranscoder->record.record, pTranscoder->record.transcodeFile);
}

void TranscoderRecord_Stop
	( Transcoder *pTranscoder )
{
	NEXUS_Record_Stop(pTranscoder->record.record);
	NEXUS_FileRecord_Close(pTranscoder->record.transcodeFile);

    NEXUS_Record_RemoveAllPidChannels(pTranscoder->record.record);

	NEXUS_Record_Destroy(pTranscoder->record.record);
}
#endif

/***************************************************************************
Summary: Playback transcoded video
    None.

Description:
    None.

See Also:
    None.
**************************************************************************/
static int TranscoderLoopback_Start
	( Transcoder *pTranscoder )
{
    NEXUS_Error                        rc = NEXUS_UNKNOWN;
    NEXUS_VideoWindowSettings          windowSettings;
    NEXUS_DisplaySettings              displaySettings;
    NEXUS_VideoFormatInfo              displayFormatInfo;
	NEXUS_VideoDecoderSettings         vidDecodeSettings;
	NEXUS_VideoDecoderStartSettings    videoDecodeStartSettings;
    NEXUS_StcChannelSettings           stcSettings;

	/* open PIP window in the same display as the video capture display */
    pTranscoder->loopback.window = NEXUS_VideoWindow_Open(pTranscoder->loopback.display, 1);
    if (pTranscoder->loopback.window == NULL)
	{
		BDBG_ERR(("Failed to open loopback window."));
		return -1;
    }

    NEXUS_Display_GetSettings(pTranscoder->loopback.display, &displaySettings);
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &displayFormatInfo);
    NEXUS_VideoWindow_GetSettings(pTranscoder->loopback.window, &windowSettings);
    windowSettings.position.x = (displayFormatInfo.width/24 + (displayFormatInfo.width*7/25) + displayFormatInfo.width/48);
    windowSettings.position.width = displayFormatInfo.width*3/5;
    windowSettings.position.height = windowSettings.position.width*3/4;
    windowSettings.position.y = (displayFormatInfo.height-windowSettings.position.height)/2;
	windowSettings.visible = true;
    rc = NEXUS_VideoWindow_SetSettings(pTranscoder->loopback.window, &windowSettings);


	BDBG_MSG(("Starting playback."));
 	/* start loopback playback of transcoded video */
	/*TranscoderLoopback_Playback_Start(pTranscoder);*/


    NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    /*stcSettings.mode = NEXUS_StcChannelMode_eAuto;*/
	stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
#if 0
    stcSettings.modeSettings.pcr.pidChannel = pTranscoder->loopback.pcrPidChannel;
#else
	stcSettings.modeSettings.pcr.pidChannel = pTranscoder->mux.pcrPidChannel;
#endif
    /*stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eVideoMaster;*/
    /*stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eFirstAvailable;*/
    pTranscoder->loopback.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
    if (pTranscoder->loopback.stcChannel == NULL)
	{
		BDBG_ERR(("Failed to open loopback window."));
		return -1;
    }

    pTranscoder->loopback.videoDecoder = NEXUS_VideoDecoder_Open(1, NULL);
    if (pTranscoder->loopback.videoDecoder == NULL)
	{
		BDBG_ERR(("Failed to open loopback video decoder."));
		return -1;
    }

	NEXUS_VideoDecoder_GetSettings(pTranscoder->loopback.videoDecoder, &vidDecodeSettings);
	BKNI_Memset(vidDecodeSettings.supportedCodecs, 0, sizeof(vidDecodeSettings.supportedCodecs));
	vidDecodeSettings.supportedCodecs[NEXUS_VideoCodec_eH264] = true; /* it's for regular HD decode heap allocation; it covers mpeg2/h264/mpeg4 HD size */
	NEXUS_VideoDecoder_SetSettings(pTranscoder->loopback.videoDecoder, &vidDecodeSettings);

	/* Set up decoder Start structures now. */
	NEXUS_VideoDecoder_GetDefaultStartSettings(&videoDecodeStartSettings);
	videoDecodeStartSettings.codec = pTranscoder->transcoderSettings.codec;
#if 0
	videoDecodeStartSettings.pidChannel = pTranscoder->loopback.videoPidChannel;
#else
	videoDecodeStartSettings.pidChannel = pTranscoder->mux.videoPidChannel;
#endif
	videoDecodeStartSettings.stcChannel = pTranscoder->loopback.stcChannel;

    NEXUS_VideoWindow_AddInput(pTranscoder->loopback.window,
		NEXUS_VideoDecoder_GetConnector(pTranscoder->loopback.videoDecoder));

	/* Start video loopback decoder */
	NEXUS_VideoDecoder_Start(pTranscoder->loopback.videoDecoder, &videoDecodeStartSettings);

	/* delay the loopback until A2P passed to avoid stutter since playback jams PTS to STC instead of locking PCR so the encode buffer
	   model is not enforced at loopback decoder plus the O_DIRECT timeshift buffer seems to have some latency. */
	BKNI_Sleep(3000);


	BDBG_MSG(("Started loopback."));
	return 0;
}

/***************************************************************************
Summary:
    None.

Description:
    None.

See Also:
    None.
**************************************************************************/
static void TranscoderLoopback_Stop
	( Transcoder *pTranscoder )
{
	NEXUS_VideoDecoder_Stop(pTranscoder->loopback.videoDecoder);
    NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(pTranscoder->loopback.videoDecoder));
    NEXUS_VideoWindow_RemoveAllInputs(pTranscoder->loopback.window);
    NEXUS_VideoDecoder_Close(pTranscoder->loopback.videoDecoder);
    NEXUS_StcChannel_Close(pTranscoder->loopback.stcChannel);
 	/* stop loopback playback of transcoded video */
	/*TranscoderLoopback_Playback_Stop(pTranscoder);*/
    NEXUS_VideoWindow_Close(pTranscoder->loopback.window);
}



/***************************************************************************
Summary:
    Start transcode

Description:
    None.

See Also:
    None.
**************************************************************************/

static int Transcoder_Start
	( Transcoder *pTranscoder )
{
    NEXUS_Error                        rc;
	NEXUS_VideoEncoderHandle           videoEncoder = pTranscoder->videoEncoder;
	TranscoderSettings                *pTranscoderSettings  = &pTranscoder->transcoderSettings;
	NEXUS_StcChannelHandle             encoderStcChannel = pTranscoder->encoderStcChannel;
	NEXUS_DisplayHandle                encoderDisplay  = pTranscoder->encoderDisplay;
	NEXUS_VideoWindowHandle            encoderWindow = pTranscoder->encoderWindow;
	NEXUS_StreamMuxOutput              muxOutput;
    NEXUS_VideoEncoderStartSettings    videoEncoderStartConfig;
    NEXUS_VideoEncoderSettings         videoEncoderConfig;
    NEXUS_VideoEncoderDelayRange       videoDelay;
    NEXUS_VideoWindowSettings          windowSettings;
#if 0
    NEXUS_DisplayCustomFormatSettings  customFormatSettings;
#endif
    BDBG_ASSERT(NULL != videoEncoder);

#if 0
	/*******************************
	 * Add system data to stream_mux
	 */
	TranscoderMux_Start_SystemData(pTranscoder);
#endif

	/* start mux */
	NEXUS_StreamMux_Start(pTranscoder->mux.streamMux, &pTranscoder->mux.config, &muxOutput);
	pTranscoder->mux.videoPidChannel = muxOutput.video[0];
	/*pTranscoder->mux.audioPidChannel = muxOutput.audio[0];*/


	if (pTranscoder->transcoderSettings.bLoopback)
	{
		TranscoderLoopback_Start(pTranscoder);
	}
	else if (pTranscoder->transcoderSettings.bHttpStreamOut)/* http stream out */
	{
		pTranscoder->httpStreaming.exitThread = 0;
		pTranscoder->httpStreaming.listeningPort = HTTP_STREAMING_DEFAULT_SERVER_PORT;
    	if (pthread_create(&pTranscoder->httpStreaming.thread, NULL, HttpStreaming_Output, (void *)pTranscoder))
		{
        	BDBG_ERR(("Failed to create HttpStreaming_Output pthread, errno %d", errno));
        	return -1;
    	}
    }
	else /* http stream out or record */
	{
#if RECORD_TO_FILE
		if (pTranscoder->transcoderSettings.bRecord)
		{
			BDBG_ERR(("Starting record."));
			strcpy(pTranscoder->record.fname, "encode.ts");
			strcpy(pTranscoder->record.indexfname, "");
#if 0
			BKNI_Snprintf(pTranscoder->record.fname, 12, "./encode.ts");
			BKNI_Snprintf(pTranscoder->record.indexfname, 13, "");
#endif
			TranscoderRecord_Start(pTranscoder);
		}
#endif
    }
#if 0
	{
		pContext->pidChannelTranscodeAudio = muxOutput.audio[0];
		NEXUS_Record_AddPidChannel(pContext->record, pContext->pidChannelTranscodeAudio, NULL);
	}
#endif

    printf("============ Video Encoder Settings ============\n");
    printf("Video Codec = [%s]\n", pTranscoderSettings->codec == NEXUS_VideoCodec_eH264 ? "H264" : "Unknown");
    printf("Video Codec Profile = [%s]\n",
        pTranscoderSettings->profile == NEXUS_VideoCodecProfile_eBaseline ? "Base Line" :
        pTranscoderSettings->profile == NEXUS_VideoCodecProfile_eMain ? "Main" : "Unknown");

    printf("Video Codec Profile-Level = [%d]\n", pTranscoderSettings->level);

    printf("Video Encoder Max Bit Rate = [%d]\n", pTranscoderSettings->bitrateMax);

    printf("Video Encoder Resolution = [%d x %d]\n", pTranscoderSettings->encoderWidth, pTranscoderSettings->encoderHeight);

    printf("=================================================\n");

#if 0
    NEXUS_Display_GetDefaultCustomFormatSettings(&customFormatSettings);
    if(pTranscoderSettings->isDisplayCustomized == true)
    {
        customFormatSettings.width = pTranscoderSettings->encoderWidth;
        customFormatSettings.height = pTranscoderSettings->encoderHeight;
    }
    customFormatSettings.refreshRate = pTranscoderSettings->displayRefreshRate;
    customFormatSettings.interlaced = false;
    customFormatSettings.aspectRatio = NEXUS_DisplayAspectRatio_e4x3;
    customFormatSettings.dropFrameAllowed = true;
    NEXUS_Display_SetCustomFormatSettings(encoderDisplay, NEXUS_VideoFormat_eCustom2, &customFormatSettings);
#endif

    NEXUS_VideoEncoder_GetSettings(videoEncoder, &videoEncoderConfig);
    videoEncoderConfig.variableFrameRate = pTranscoderSettings->variableFrameRate;
    videoEncoderConfig.frameRate = pTranscoderSettings->frameRate;
    videoEncoderConfig.bitrateMax = pTranscoderSettings->bitrateMax;
    videoEncoderConfig.streamStructure.framesP = pTranscoderSettings->framesP;
    videoEncoderConfig.streamStructure.framesB = pTranscoderSettings->framesB;
    /* videoEncoderConfig.delayMode = NEXUS_VideoEncoderDelayMode_eLow; */
    /* videoEncoderConfig.lowDelayMode = true; */

    NEXUS_VideoWindow_GetSettings(encoderWindow, &windowSettings);
    windowSettings.visible = true;
    rc = NEXUS_VideoWindow_SetSettings(encoderWindow, &windowSettings);
    BDBG_ASSERT(!rc);

    NEXUS_VideoEncoder_GetDefaultStartSettings(&videoEncoderStartConfig);

    videoEncoderStartConfig.codec = pTranscoderSettings->codec;
    videoEncoderStartConfig.profile = pTranscoderSettings->profile;
    videoEncoderStartConfig.level = pTranscoderSettings->level;

    videoEncoderStartConfig.interlaced = false;
    videoEncoderStartConfig.input = encoderDisplay;
    videoEncoderStartConfig.stcChannel = encoderStcChannel;

    videoEncoderStartConfig.lowDelayPipeline = true;
    videoEncoderStartConfig.nonRealTime = false;
	videoEncoderStartConfig.encodeUserData = false;

    /******************************************
    * add configurable delay to video path */
    /* NOTE: ITFP is encoder feature to detect and lock on 3:2/2:2 cadence in the video content to help
    * efficient coding for interlaced formats; disabling ITFP will impact the bit efficiency but reduce the encode delay. */
    videoEncoderConfig.enableFieldPairing = false;

    /* 0 to use default 750ms rate buffer delay; TODO: allow user to adjust it to lower encode delay at cost of quality reduction! */
    videoEncoderStartConfig.rateBufferDelay = 0;

    /* to allow 23.976p passthru; TODO: allow user to configure minimum framerate to achieve lower delay!
    * Note: lower minimum framerate means longer encode delay */
    videoEncoderStartConfig.bounds.inputFrameRate.min = NEXUS_VideoFrameRate_e23_976;

    /* to allow 24 ~ 60p dynamic frame rate coding TODO: allow user to config higher minimum frame rate for lower delay! */
    videoEncoderStartConfig.bounds.outputFrameRate.min = NEXUS_VideoFrameRate_e23_976;
    videoEncoderStartConfig.bounds.outputFrameRate.max = NEXUS_VideoFrameRate_e60;

    /* max encode size allows 1080p encode; TODO: allow user to choose lower max resolution for lower encode delay */
    videoEncoderStartConfig.bounds.inputDimension.max.width = 1920;
    videoEncoderStartConfig.bounds.inputDimension.max.height = 1088;

    NEXUS_VideoEncoder_GetDelayRange(videoEncoder, &videoEncoderConfig, &videoEncoderStartConfig, &videoDelay);
    fprintf(stderr, "Video encoder end-to-end delay = [%u ~ %u] ms", videoDelay.min/27000, videoDelay.max/27000);
    videoEncoderConfig.encoderDelay = videoDelay.min;
	videoEncoderConfig.frameRate = pTranscoder->transcoderSettings.frameRate;
	videoEncoderConfig.bitrateMax = pTranscoder->transcoderSettings.bitrateMax;

    rc = NEXUS_VideoEncoder_SetSettings(videoEncoder, &videoEncoderConfig);
    BDBG_ASSERT(!rc);


    rc = NEXUS_VideoEncoder_Start(videoEncoder, &videoEncoderStartConfig);

#if 0
    NEXUS_VideoEncoder_GetStatus(videoEncoder, &videoEncoderStatus);
	printf("\nVideo Encoder[%d] Status:\n", pContext->contextId);
				printf("----------------------\n");
				printf("data buffer base address     = %p\n", videoEncodeStatus.bufferBase);
				printf("metadata buffer base address = %p\n", videoEncodeStatus.metadataBufferBase);
				printf("error flags                  = 0x%x\n", videoEncodeStatus.errorFlags);
				printf("error count                  = %u\n", videoEncodeStatus.errorCount);
				printf("picture drops due to error   = %u\n", videoEncodeStatus.picturesDroppedErrors);
				printf("picture drops due to FRC     = %u\n", videoEncodeStatus.picturesDroppedFRC);
				printf("pictures Encoded             = %u\n", videoEncodeStatus.picturesEncoded);
				printf("pictures Received            = %u\n", videoEncodeStatus.picturesReceived);
				printf("picture Id Last Encoded      = 0x%x\n", videoEncodeStatus.pictureIdLastEncoded);
				printf("----------------------\n\n");

    encoderBufferBase = (uint8_t *)videoEncoderStatus.bufferBase;
#endif

    fprintf(stderr, "Video Encoder start is completed\n");
    return 0;
}

/***************************************************************************
Summary:
    Stop transcode

Description:
    None.

See Also:
    None.
**************************************************************************/
static int Transcoder_Stop
	( Transcoder *pTranscoder )
{
    NEXUS_Error                rc;
    NEXUS_VideoWindowSettings  windowSettings;

    BDBG_ASSERT(NULL != pTranscoder->videoEncoder);

#if 0
	TranscoderMux_Stop_SystemData(pTranscoder);
#endif

    NEXUS_VideoWindow_GetSettings(pTranscoder->encoderWindow, &windowSettings);
    windowSettings.visible = false;
    rc = NEXUS_VideoWindow_SetSettings(pTranscoder->encoderWindow, &windowSettings);
    BDBG_ASSERT(!rc);

    NEXUS_VideoEncoder_Stop(pTranscoder->videoEncoder, NULL);

	if (pTranscoder->transcoderSettings.bLoopback)
	{
		TranscoderLoopback_Stop(pTranscoder);
	}
	else if (pTranscoder->transcoderSettings.bHttpStreamOut)/* http stream out */
	{
        pTranscoder->httpStreaming.exitThread = 1;
        BKNI_Sleep(1000);
		pthread_join(pTranscoder->httpStreaming.thread, NULL);
    }
	else /* http stream out */
	{
#if RECORD_TO_FILE
		if (pTranscoder->transcoderSettings.bRecord)
		{
			TranscoderRecord_Stop(pTranscoder);
		}
#endif
    }
	NEXUS_StreamMux_Stop(pTranscoder->mux.streamMux);

    return 0;
}

static int Transcoder_Open
	( Transcoder                     **pTranscoder,
	  TranscoderSettings              *pTranscoderSettings,
	  NEXUS_VideoInput                 videoInput )
{
	Transcoder                        *pLocalXcoder;
	NEXUS_VideoEncoderHandle           videoEncoder;
	NEXUS_VideoEncoderOpenSettings     encoderOpenSettings;
	NEXUS_StcChannelHandle             encoderStcChannel;
	NEXUS_StcChannelSettings           encoderStcSettings;
	NEXUS_DisplayHandle                encoderDisplay;
	NEXUS_DisplaySettings              displaySettings;
	NEXUS_VideoWindowHandle            encoderWindow;
#if 0
	NEXUS_VideoWindowSettings          windowSettings;
#endif

	BDBG_ASSERT(NULL != videoInput);

	pLocalXcoder = (Transcoder *)BKNI_Malloc(sizeof(Transcoder));
	if (pLocalXcoder == NULL)
	{
		fprintf(stderr, "Failed to create Transcoder object.\n");
		return -1;
	}

	/* Initialize encoder settings */
	pTranscoderSettings->variableFrameRate = false;                              /*encode parameter*/
    pTranscoderSettings->codec = NEXUS_VideoCodec_eH264;
    pTranscoderSettings->frameRate = NEXUS_VideoFrameRate_e30;                  /*encode parameter*/
    pTranscoderSettings->profile = NEXUS_VideoCodecProfile_eBaseline;
    pTranscoderSettings->level = NEXUS_VideoCodecLevel_e31;
    pTranscoderSettings->bitrateMax = 2000000;
    pTranscoderSettings->framesP = 29;                                          /*encode parameter*/
    pTranscoderSettings->framesB = 0;                                           /*encode parameter*/
    pTranscoderSettings->displayFormat = NEXUS_VideoFormat_e480p;               /*Display parameter */
    pTranscoderSettings->displayRefreshRate = 60000;                            /*Display Parameter*/
    pTranscoderSettings->isDisplayCustomized = false;
    pTranscoderSettings->encoderWidth = 1280;
    pTranscoderSettings->encoderHeight = 720;

    /* Open a STC channel for video encoder */
    NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &encoderStcSettings);
    encoderStcSettings.timebase = NEXUS_Timebase_e0;
    encoderStcSettings.mode = NEXUS_StcChannelMode_eAuto;
	encoderStcSettings.pcrBits = NEXUS_StcChannel_PcrBits_eFull42;/* ViCE2 requires 42-bit STC broadcast */
    encoderStcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &encoderStcSettings);
    if (!encoderStcChannel)
    {
		fprintf(stderr, "Can't open STC channel.\n");
		return -1;
    }

	/* NOTE: must open video encoder before display; otherwise open will init ViCE2 core
	* which might cause encoder display GISB error since encoder display would
	* trigger RDC to program mailbox registers in ViCE2;
	*/
	NEXUS_VideoEncoder_GetDefaultOpenSettings(&encoderOpenSettings);
	encoderOpenSettings.type = NEXUS_VideoEncoderType_eSingle;
    videoEncoder = NEXUS_VideoEncoder_Open(0, &encoderOpenSettings);
    if (!videoEncoder)
    {
		fprintf(stderr, "Can't open ViCE.\n");
		NEXUS_StcChannel_Close(encoderStcChannel);
		return -1;
    }

    /* Bring up video encoder display */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eEncoder;
    displaySettings.format = NEXUS_VideoFormat_e720p;
	displaySettings.frameRateMaster = NULL;/* disable frame rate tracking for now */

    encoderDisplay = NEXUS_Display_Open(NEXUS_ENCODER0_DISPLAY_IDX, &displaySettings);
    if (!encoderDisplay)
    {
		fprintf(stderr, "Can't open encoder display.\n");
		NEXUS_VideoEncoder_Close(videoEncoder);
		NEXUS_StcChannel_Close(encoderStcChannel);
		return -1;
    }

    encoderWindow = NEXUS_VideoWindow_Open(encoderDisplay, 0);
    if (!encoderWindow)
    {
		fprintf(stderr, "Can't open encoder window.\n");
		NEXUS_VideoEncoder_Close(videoEncoder);
		NEXUS_StcChannel_Close(encoderStcChannel);
		NEXUS_Display_Close(encoderDisplay);
		return -1;
    }

#if 0
    NEXUS_VideoWindow_GetSettings(encoderWindow, &windowSettings);
    windowSettings.forceCapture = false;
    NEXUS_VideoWindow_SetSettings(encoderWindow, &windowSettings);
#endif

    NEXUS_VideoWindow_AddInput(encoderWindow, videoInput);

	pLocalXcoder->videoEncoder = videoEncoder;
	pLocalXcoder->encoderStcChannel = encoderStcChannel;
	pLocalXcoder->encoderDisplay = encoderDisplay;
	pLocalXcoder->encoderWindow  = encoderWindow;
	pLocalXcoder->transcoderSettings = *pTranscoderSettings;
	*pTranscoder = pLocalXcoder;

	/* Setup Stream Mux */
	TranscoderMux_Open(*pTranscoder);

    return 0;
}

static int Transcoder_Close
	( Transcoder *pTranscoder )
{
    NEXUS_VideoWindowSettings  windowSettings;

    BDBG_ASSERT(NULL != pTranscoder->videoEncoder);

#if 0
	TranscoderMux_Release_SystemData(pTranscoder);
#endif
	TranscoderMux_Close(pTranscoder);
    NEXUS_VideoWindow_GetSettings(pTranscoder->encoderWindow, &windowSettings);
    windowSettings.visible = false;
    NEXUS_VideoWindow_SetSettings(pTranscoder->encoderWindow, &windowSettings);
	NEXUS_VideoWindow_RemoveAllInputs(pTranscoder->encoderWindow);
	NEXUS_VideoWindow_Close(pTranscoder->encoderWindow);
	NEXUS_Display_Close(pTranscoder->encoderDisplay);
	NEXUS_StcChannel_Close(pTranscoder->encoderStcChannel);
	NEXUS_VideoEncoder_Close(pTranscoder->videoEncoder);
	BKNI_Free(pTranscoder);

	return 0;
}

static void usage (int argc, char **argv)
{
	BSTD_UNUSED(argc);

    fprintf (stderr, "Usage: %s [options]\n\n", argv[0]);
    fprintf (stderr, "Options:\n");
	fprintf (stderr, "-camera                 - usb camera input (default)\n");
	fprintf (stderr, "-hdmi                   - hdmi input\n");
	fprintf (stderr, "-streamer               - streamer input\n");
	fprintf (stderr, "-record_input           - record input. Only applies to streamer.\n");
    fprintf (stderr, "-d <video device name>  - For usb camera input only. Default is /dev/video0\n");
    fprintf (stderr, "-s <width> <height>     - For usb camera input only. Frame size. Default is 1280x720\n");
    fprintf (stderr, "-f                      - For usb camera input only. Frame rate. Default is 30.\n");
    fprintf (stderr, "-l                      - loopback transcoded video\n");
    fprintf (stderr, "                          Cannot be used with http streaming option.\n");
#ifdef RECORD_TO_FILE
    fprintf (stderr, "-record_xcode           - record transcoded video.\n");
    fprintf (stderr, "                          Cannot be used with loopback option.\n");
#endif
    fprintf (stderr, "-o                      - send transcoded video out via http streaming\n");
    fprintf (stderr, "                          Cannot be used with loopback option.\n");
    fprintf (stderr, "-h                      - prints this message\n");
}


/* Get PTS info needed for identifying frames */
static void PrintEncoderPts(Transcoder *pTranscoder, int k)
{
	struct timeval timeStamp;
	size_t size[2];
	const NEXUS_VideoEncoderDescriptor *desc[2];
	unsigned i,j;
	unsigned descs;

	if (k != 0)
	{
		uint32_t t0;

		gettimeofday(&timeStamp, 0);
		/* in ms */
		t0 = timeStamp.tv_sec * 1000 + timeStamp.tv_usec/1000;
	    printf("Pic %d: t0 = %d\n", k, t0);
	}

	/* Get encoded buffer PTS thrice - once in the earlier part, once in the middle and another in the latter part of the stream */
	NEXUS_VideoEncoder_GetBuffer(pTranscoder->videoEncoder, &desc[0], &size[0], &desc[1], &size[1]);

	for(descs=0,j=0;j<2;j++)
	{
		descs+=size[j];
		for(i=0;i<size[j];i++)
		{
			if(desc[j][i].length > 0)
			{
				BDBG_MSG(("original PTS: %x, PTS: %x%08x", desc[j][i].originalPts,
					(uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff)));
				printf("original PTS: %x, PTS: %x%08x\n", desc[j][i].originalPts,
					(uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff));
				if(desc[j][i].length > 0x100000)
				{
					BDBG_ERR(("++++ desc[%d][%d] length = 0x%x, offset=0x%x", j,i, desc[j][i].length, desc[j][i].offset));
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
	Input                        input = Input_eCamera;
	bool                         bRecordInput = false;

	/* VideoCamera driver */
    VideoCamera_FrameInfo        frame;

	/* VideoCamera object */
	VideoCamera                 *pVideoCamera;
	VideoCameraSettings          videoCameraSettings;

	/* Nexus */
    NEXUS_PlatformSettings       platformSettings;
    NEXUS_PlatformConfiguration  platformConfig;
	NEXUS_DisplayHandle          display;
    NEXUS_DisplaySettings        displaySettings;
	NEXUS_VideoFormatInfo        videoFormatInfo;

	NEXUS_VideoWindowHandle      window;
    NEXUS_VideoWindowSettings    windowCfg;

    NEXUS_SurfaceCreateSettings  surfaceCreateSettings;

    NEXUS_Graphics2DHandle       gfx;
    NEXUS_Graphics2DSettings     gfx2DSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;
	BKNI_EventHandle             gfxEvent;

	/*NEXUS_SurfaceHandle          picture = NULL;*/
    NEXUS_SurfaceHandle          img[3];   /* triple buffering for image update every FRAME and a full queue */

	/* Picture Decoder (SID) */
    void                        *pPictureBuffer;
    void                        *pMultiScanBuffer;
    int                          multiScanBufferSize = 16*1024*1024;
    int                          pictureBufferSize = 4*1024*1024;

    NEXUS_PictureDecoderStartSettings decoderStartSettings;
    NEXUS_PictureDecoderStatus        decoderStatus;
    NEXUS_PictureDecoderOpenSettings  decoderSettings;
    NEXUS_PictureDecoderHandle        pictureDecoder = NULL;

	/* Video Image Input */
	NEXUS_VideoImageInputHandle          imageInput;
    NEXUS_VideoImageInputSettings        imageInputSetting;
	NEXUS_VideoImageInputSurfaceSettings surfaceSettings;
    NEXUS_VideoInput                     videoInput;
	BKNI_EventHandle                     imageEvent;
	struct timeval                       timeBase, timeStamp;

	NEXUS_TimebaseSettings            timebaseSettings;

	/* TODO: Audio */

	/* Transcoder object */
	Transcoder                       *pTranscoder;
	TranscoderSettings	              transcoderSettings;

	/* Picture counter */
	int                               picCntr;

	int                               argIndex;

#if USE_APPLICATION_BUFFER
    NEXUS_MemoryAllocationSettings allocationSettings;
#endif

	/* keystroke handler */
	B_ThreadSettings threadSettings;
	B_ThreadHandle keyHandle;

	B_Os_Init();

	g_eotEvent = B_Event_Create(NULL);
	B_Event_Reset(g_eotEvent);
	B_Thread_GetDefaultSettings(&threadSettings);

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0, &timebaseSettings);
    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
    NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0, &timebaseSettings);

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = NEXUS_DisplayType_eAuto;
    displaySettings.format = NEXUS_VideoFormat_eNtsc;
    displaySettings.timebase = NEXUS_Timebase_e0;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif

	/* Set default transcode settings */
	transcoderSettings.bLoopback = false;
	transcoderSettings.bRecord = false;
	transcoderSettings.bHttpStreamOut = false;

	/* Get user provided options for VideoCamera driver */
    for (argIndex = 1; argIndex < argc; argIndex++)
    {
        if (!strcmp(argv[argIndex], "-d"))
        {
             videoCameraSettings.video.videoDevName = strdup(argv[++argIndex]);
        }
        else if (!strcmp(argv[argIndex], "-s"))
        {
			videoCameraSettings.video.width = strtol(argv[++argIndex], NULL, 0);
			videoCameraSettings.video.height = strtol(argv[++argIndex], NULL, 0);
        }
        else if (!strcmp(argv[argIndex], "-f"))
        {
            videoCameraSettings.video.framerate = strtol(argv[++argIndex], NULL, 0);
        }
        else if (!strcmp(argv[argIndex], "-l"))
        {
            transcoderSettings.bLoopback = true;
        }
        else if (!strcmp(argv[argIndex], "-record_xcode"))
        {
            transcoderSettings.bRecord = true;
        }
		else if (!strcmp(argv[argIndex], "-o"))
        {
            transcoderSettings.bHttpStreamOut = true;
        }
		else if (!strcmp(argv[argIndex], "-streamer"))
        {
            input = Input_eStreamer;
        }
        else if (!strcmp(argv[argIndex], "-hdmi"))
        {
            input = Input_eHdmi;
        }
        else if (!strcmp(argv[argIndex], "-camera"))
        {
            input = Input_eCamera;
        }
		else if (!strcmp(argv[argIndex], "-record_input"))
        {
            bRecordInput = true;
        }
		else
        {
			printf("argIndex = %d\n", argIndex);
            usage(argc, argv);
            return -1;
        }
    }

	if (input == Input_eCamera)
	{
		/* Set default video camera settings */
		videoCameraSettings.video.videoDevName = DEFAULT_VIDEO_CAMERA_DEVICE;
		videoCameraSettings.video.width = DEFAULT_VIDEO_CAMERA_WIDTH;
		videoCameraSettings.video.height = DEFAULT_VIDEO_CAMERA_HEIGHT;
		videoCameraSettings.video.framerate = DEFAULT_VIDEO_CAMERA_FRAMERATE;

		videoCameraSettings.audio.audioDevName = DEFAULT_AUDIO_DEVICE;
		videoCameraSettings.audio.format = DEFAULT_AUDIO_FORMAT;
		videoCameraSettings.audio.channels = DEFAULT_AUDIO_NUM_CHANNELS;
		videoCameraSettings.audio.speed = DEFAULT_AUDIO_SPEED;
		videoCameraSettings.audio.newFrag = DEFAULT_AUDIO_NEWFRAG;
		videoCameraSettings.audio.maxDataLength = DEFAULT_AUDIO_MAXDATALENGTH;
	}

    window = NEXUS_VideoWindow_Open(display, 0);
    if(!window)
	{
        fprintf(stderr, "Can't open window\n");
		return -1;
    }

	NEXUS_VideoWindow_GetSettings(window, &windowCfg);
    fprintf(stderr, "windowCfg: width=%d hght=%d x=%d, y=%d\n",
		windowCfg.position.width, windowCfg.position.height,
        windowCfg.position.x, windowCfg.position.y);

    windowCfg.visible = true;

    if (NEXUS_VideoWindow_SetSettings(window, &windowCfg) != NEXUS_SUCCESS)
    {
        fprintf(stderr, "Can't change window settings\n");
		return -1;
    }

	/* For calculating PTS */
	gettimeofday(&timeBase, 0);

	if (input == Input_eHdmi)
	{
	    NEXUS_HdmiInputHandle hdmiInput;
	    NEXUS_HdmiInputSettings hdmiInputSettings;
	    unsigned hdmiInputIndex = 0;

	    NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0, &timebaseSettings);
	    timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eHdDviIn;
	    NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0, &timebaseSettings);

	    NEXUS_HdmiInput_GetDefaultSettings(&hdmiInputSettings);

		hdmiInputSettings.timebase = NEXUS_Timebase_e0;
	    hdmiInputSettings.frontend.hpdDisconnected = false ;
	    hdmiInputSettings.useInternalEdid = true ;

	    /* Load EDID RAM with the internal declared EDID. */
	    hdmiInput = NEXUS_HdmiInput_OpenWithEdid(hdmiInputIndex, &hdmiInputSettings,
	            &SampleEDID[0], (uint16_t) sizeof(SampleEDID));
	    if (!hdmiInput)
	    {
	        fprintf(stderr, "Can't get hdmi input\n");
	        return -1;
	    }

	    NEXUS_HdmiInput_GetSettings(hdmiInput, &hdmiInputSettings) ;
	    hdmiInputSettings.avMuteChanged.callback = avmute_changed;
	    hdmiInputSettings.avMuteChanged.context = hdmiInput ;
	    hdmiInputSettings.sourceChanged.callback = source_changed;
	    hdmiInputSettings.sourceChanged.context = hdmiInput ;
	    hdmiInputSettings.sourceChanged.param = (int)display;
	    NEXUS_HdmiInput_SetSettings(hdmiInput, &hdmiInputSettings) ;

		videoInput = NEXUS_HdmiInput_GetVideoConnector(hdmiInput);
	    NEXUS_VideoWindow_AddInput(window, videoInput);

		if (Transcoder_Open(&pTranscoder, &transcoderSettings, videoInput) != 0)
		{
			fprintf(stderr, "Failed to open Transcoder object\n");
			return -1;
		}

		pTranscoder->loopback.display = display;
		if (Transcoder_Start(pTranscoder) != 0)
		{
			fprintf(stderr, "Failed to start transcode\n");
			return -1;
		}

		picCntr = 0;
		/* Running loop */
		keyHandle = B_Thread_Create("key handler", (B_ThreadFunc)keyHandler, NULL, &threadSettings);
		while(B_Event_Wait(g_eotEvent, 10)!=B_ERROR_SUCCESS)
		{
			picCntr++;
			PrintEncoderPts(pTranscoder, picCntr);
		}
		B_Thread_Destroy(keyHandle);

		Transcoder_Stop(pTranscoder);
		Transcoder_Close(pTranscoder);

	    NEXUS_VideoWindow_RemoveAllInputs(window);
	    NEXUS_VideoInput_Shutdown(videoInput);
		NEXUS_VideoWindow_Close(window);
	    NEXUS_Display_RemoveAllOutputs(display);
	    NEXUS_Display_Close(display);
	    NEXUS_Platform_Uninit();
	}
	else if (input == Input_eStreamer)
	{
		NEXUS_Error rc;
	    NEXUS_InputBand inputBand;
	    NEXUS_FileRecordHandle file;
	    NEXUS_RecpumpHandle recpump;
	    NEXUS_RecordHandle record;
	    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
	    NEXUS_ParserBandSettings parserBandSettings;
	    NEXUS_RecordPidChannelSettings pidSettings;
	    NEXUS_RecordSettings recordSettings;
	    NEXUS_PidChannelHandle pidChannel[2];
	    NEXUS_VideoDecoderHandle vdecode;
	    NEXUS_VideoDecoderStartSettings videoProgram;
	    NEXUS_StcChannelSettings stcSettings;
		NEXUS_VideoInput videoInput;

	    const char *fname = "stream.tts";
		const char *index = "stream.nav";

	    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

	    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
	    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
	    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
	    parserBandSettings.transportType = NEXUS_TransportType_eTs;
	    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

	    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    	videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
    	videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, 0x31, NULL);


	    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
	    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
	    stcSettings.timebase = NEXUS_Timebase_e0;
	    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
	    stcSettings.modeSettings.pcr.pidChannel = videoProgram.pidChannel; /* PCR on video pid */
	    videoProgram.stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);

	    /* bring up decoder and connect to display */
	    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
		videoInput = NEXUS_VideoDecoder_GetConnector(vdecode);
	    NEXUS_VideoWindow_AddInput(window, videoInput);

	    /* Start Decoder */
	    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);

		/* Record */
		if (bRecordInput)
		{
		    recpump = NEXUS_Recpump_Open(INPUT_RECPUMP_IDX, NULL);
		    record = NEXUS_Record_Create();

		    NEXUS_Record_GetSettings(record, &recordSettings);
		    recordSettings.recpump = recpump;
			recordSettings.recpumpSettings.timestampType = NEXUS_TransportTimestampType_eMod300;
		    NEXUS_Record_SetSettings(record, &recordSettings);

		    file = NEXUS_FileRecord_OpenPosix(fname,  index);

			/* configure the video pid for indexing */
		    NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
		    pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
		    pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
		    pidSettings.recpumpSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
		    pidChannel[0] = NEXUS_PidChannel_Open(parserBand, 0x31, NULL);
		    rc = NEXUS_Record_AddPidChannel(record, pidChannel[0], &pidSettings);
		    BDBG_ASSERT(!rc);

		    rc = NEXUS_Record_Start(record, file);
		    BDBG_ASSERT(!rc);
		}

        /* Nexus is now recording to disk */

		/* Transcode */
		if (Transcoder_Open(&pTranscoder, &transcoderSettings, videoInput) != 0)
		{
			fprintf(stderr, "Failed to open Transcoder object\n");
			return -1;
		}

		pTranscoder->loopback.display = display;
		if (Transcoder_Start(pTranscoder) != 0)
		{
			fprintf(stderr, "Failed to start transcode\n");
			return -1;
		}


		picCntr=0;
		/* Running loop */
		keyHandle = B_Thread_Create("key handler", (B_ThreadFunc)keyHandler, NULL, &threadSettings);

		while(B_Event_Wait(g_eotEvent, 10)!=B_ERROR_SUCCESS)
		{
			picCntr++;

			PrintEncoderPts(pTranscoder, picCntr);
		}

		B_Thread_Destroy(keyHandle);


		Transcoder_Stop(pTranscoder);
		Transcoder_Close(pTranscoder);

		NEXUS_VideoDecoder_Stop(vdecode);

	    printf("press ENTER to stop record\n");
	    getchar();

		if (bRecordInput)
		{
		    NEXUS_Record_Stop(record);
		    NEXUS_Record_RemoveAllPidChannels(record);
		    NEXUS_PidChannel_Close(pidChannel[0]);
		    NEXUS_FileRecord_Close(file);
		    NEXUS_Record_Destroy(record);
		    NEXUS_Recpump_Close(recpump);
		}

	    NEXUS_VideoWindow_RemoveAllInputs(window);
	    NEXUS_VideoInput_Shutdown(videoInput);
		NEXUS_VideoWindow_Close(window);
	    NEXUS_Display_RemoveAllOutputs(display);
	    NEXUS_Display_Close(display);
	    NEXUS_Platform_Uninit();
	}
	else /* video camera */
	{
	    imageInput = NEXUS_VideoImageInput_Open(0, NULL);
	    if(!imageInput)
		{
	        fprintf(stderr, "Can't create imageInput\n");
			return -1;
	    }

	    videoInput = NEXUS_VideoImageInput_GetConnector(imageInput);
	    if(!videoInput)
		{
	        fprintf(stderr, "Can't create videoInput\n");
			return -1;
	    }

	    if (NEXUS_VideoWindow_AddInput(window, videoInput) != NEXUS_SUCCESS)
	    {
			fprintf(stderr, "Failed to add videoInput to window\n");
			return -1;
	    }

	    BKNI_CreateEvent(&imageEvent);
	    NEXUS_VideoImageInput_GetSettings( imageInput, &imageInputSetting );
	    imageInputSetting.imageCallback.callback = VideoImageInputBufferCallback;
	    imageInputSetting.imageCallback.context  = imageEvent;

	    if (NEXUS_VideoImageInput_SetSettings(imageInput, &imageInputSetting ) != NEXUS_SUCCESS)
	    {
			fprintf(stderr, "Failed to set video image input settings.\n");
			return -1;
	    }

		if (Transcoder_Open(&pTranscoder, &transcoderSettings, videoInput) != 0)
		{
			fprintf(stderr, "Failed to open Transcoder object\n");
			return -1;
		}

	    BKNI_CreateEvent(&gfxEvent);
	    gfx = NEXUS_Graphics2D_Open(0, NULL);
		if(!gfx)
		{
	        fprintf(stderr, "Can't open GFX.\n");
			return -1;
	    }

	    NEXUS_Graphics2D_GetSettings(gfx, &gfx2DSettings);
	    gfx2DSettings.checkpointCallback.callback = GfxBlitComplete;
	    gfx2DSettings.checkpointCallback.context = gfxEvent;
	    NEXUS_Graphics2D_SetSettings(gfx, &gfx2DSettings);

	    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);

	    /* Create surfaces that will be used with VideoImageInput. The image input surface
	    must be from the MEMC that MFD has access too. Image input 0 uses MFD0.
	    We have to use mfdMapping[] to learn which AVD, then learn the heap for that AVD.
	    NEXUS_VideoDecoderModuleSettings has that information after NEXUS_Platform_Init completes.
	    Hence the multiple levels of indirection.
	    */
	    NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
	    surfaceCreateSettings.width = videoFormatInfo.width; /* TODO: use encoder settings */
	    surfaceCreateSettings.height = videoFormatInfo.height;  /* TODO: use encoder settings */
	    surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
		surfaceCreateSettings.heap = GetImageInputHeap(0);

	    img[0] = NEXUS_Surface_Create(&surfaceCreateSettings);
	    img[1] = NEXUS_Surface_Create(&surfaceCreateSettings);
	    img[2] = NEXUS_Surface_Create(&surfaceCreateSettings);

	    if (!img[0] || !img[1] || !img[2])
		{
	        fprintf(stderr, "Can't create surfaces\n");
			return -1;
	    }

		if (VideoCamera_Open(&pVideoCamera, &videoCameraSettings) != 0)
		{
			fprintf(stderr, "Failed to open video camera.\n");
			return -1;
		}

	    VideoCamera_StartVideoCapture(pVideoCamera);

		pTranscoder->loopback.display = display;
		if (Transcoder_Start(pTranscoder) != 0)
		{
			fprintf(stderr, "Failed to start transcode\n");
			return -1;
		}

	    /* Let PictureDecoder allocate memory */
	    printf(" Using PictureDecoder provided buffers...\n");
	    pMultiScanBuffer = NULL;
	    pPictureBuffer = NULL;

		/* Setup Picture Decoder */
	    NEXUS_PictureDecoder_GetDefaultOpenSettings(&decoderSettings);
	    decoderSettings.multiScanBufferSize = multiScanBufferSize;
	    decoderSettings.bufferSize = pictureBufferSize;
	    decoderSettings.multiScanBufferPtr = pMultiScanBuffer;
	    decoderSettings.bufferPtr = pPictureBuffer;
	    pictureDecoder = NEXUS_PictureDecoder_Open(0, &decoderSettings);

	    NEXUS_PictureDecoder_GetDefaultStartSettings(&decoderStartSettings);
	    decoderStartSettings.format = NEXUS_PictureFormat_eJpeg;

	    for (picCntr=0;picCntr<500;picCntr++)
	    {
			NEXUS_Error          rc;
		    NEXUS_SurfaceHandle  surfaceBuf = img[picCntr%3];

	        do
			{
	            /* Because we re-use buffers, must check it is not in use, before filling */
	            rc = NEXUS_VideoImageInput_CheckSurfaceCompletion( imageInput, surfaceBuf );
	            if (rc == NEXUS_IMAGEINPUT_BUSY)
				{
	                BKNI_WaitForEvent(imageEvent, BKNI_INFINITE);
	            }
	        } while (rc);

			/* Get the captured frame */
	        if (VideoCamera_CaptureVideoFrame(pVideoCamera, &frame) == -1)
	        {
				fprintf(stderr, "Video capture frame failed!\n");

	            if(VideoCamera_ReleaseVideoFrame(pVideoCamera, &frame) == -1)
	            {
	                fprintf(stderr, "Video release frame failed!\n");
					return -1;
	            }
				else
				{
					continue;
				}
	        }

		    /* Enable picture decoder */
		    if (VideoCamera_StartVideoDecode(pVideoCamera, frame.data, frame.size,
				/*&picture,*/ &decoderStatus, &decoderStartSettings, pictureDecoder) != 0)
			{
				printf("Error is starting video decode\n");
			    goto error;
		    }

	    	NEXUS_Surface_Flush(pVideoCamera->video.picture);

	 	    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);

		    blitSettings.source.surface = pVideoCamera->video.picture;
		    blitSettings.source.rect.x = 0;
		    blitSettings.source.rect.y = 0;
		    blitSettings.source.rect.width = decoderStatus.header.width;
		    if(NEXUS_PIXEL_FORMAT_IS_YCRCB(decoderStatus.header.format)) {
		        blitSettings.source.rect.width += blitSettings.source.rect.width%2; /* YCrCb single pixel has width of 2 */
		    }

		    blitSettings.source.rect.height = decoderStatus.header.height;
		    blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
		    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

		    blitSettings.output.surface = surfaceBuf;
		    blitSettings.output.rect.x = 0;
		    blitSettings.output.rect.y = 0;
		    blitSettings.output.rect.width = videoFormatInfo.width; /* fill to fit entire screen */
		    blitSettings.output.rect.height = videoFormatInfo.height;

		    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
		    /* blit can fail because an internal queue is full. wait for all blits to complete, then continue. */
			rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
			if(rc == NEXUS_GRAPHICS2D_QUEUED)
	        {
	            BKNI_WaitForEvent(gfxEvent, BKNI_INFINITE);
	        }

	        do
			{
				size_t size[2];
				const NEXUS_VideoEncoderDescriptor *desc[2];

				/* if image_input queue is full, wait for a slot opening up */
#if 0
	            rc = NEXUS_VideoImageInput_SetNextSurface(imageInput, surfaceBuf);
#else
	        	NEXUS_VideoImageInput_GetDefaultSurfaceSettings( &surfaceSettings );

				/* original PTS in ms */
				surfaceSettings.pts = (frame.timeStamp.tv_sec - timeBase.tv_sec)*VIDEO_CLOCK_RATE + ((frame.timeStamp.tv_usec - timeBase.tv_usec)/1000)*(VIDEO_CLOCK_RATE/1000);

				if (picCntr != 0)
				{
					uint32_t t0;

					gettimeofday(&timeStamp, 0);
					/* in ms */
					t0 = timeStamp.tv_sec * 1000 + timeStamp.tv_usec/1000;
				    printf("Pic %d: Surface PTS = 0x%x, T0 = %d\n", picCntr, surfaceSettings.pts, t0);
				}

				rc = NEXUS_VideoImageInput_PushSurface(imageInput, surfaceBuf, &surfaceSettings );
#endif
	            if (rc == NEXUS_IMAGEINPUT_QUEUE_FULL)
				{
	                BKNI_WaitForEvent(imageEvent, BKNI_INFINITE);
	            }

				/* Get encoded buffer PTS thrice - once in the earlier part, once in the middle and another in the latter part of the stream */
				if (picCntr == 150 || picCntr == 250 || picCntr == 350)
				{
					unsigned i,j;
					unsigned descs;

					NEXUS_VideoEncoder_GetBuffer(pTranscoder->videoEncoder, &desc[0], &size[0], &desc[1], &size[1]);

					if(size[0]==0 && size[1]==0)
					{
						NEXUS_PictureDecoderStatus pstatus;

						NEXUS_PictureDecoder_GetStatus(pictureDecoder, &pstatus);
						BKNI_Sleep(30);
						continue;
					}

					for(descs=0,j=0;j<2;j++)
					{
						descs+=size[j];
						for(i=0;i<size[j];i++)
						{
							if(desc[j][i].length > 0)
							{
								BDBG_MSG(("original PTS: %x, PTS: %x%08x", desc[j][i].originalPts,
									(uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff)));
								printf("original PTS: %x, PTS: %x%08x\n", desc[j][i].originalPts,
									(uint32_t)(desc[j][i].pts>>32), (uint32_t)(desc[j][i].pts & 0xffffffff));
								if(desc[j][i].length > 0x100000)
								{
									BDBG_ERR(("++++ desc[%d][%d] length = 0x%x, offset=0x%x", j,i, desc[j][i].length, desc[j][i].offset));
								}
							}
						}
					}
				}
	        } while (rc);

			VideoCamera_StopVideoDecode(pVideoCamera, pictureDecoder);

			/* Release the captured frame */
	        VideoCamera_ReleaseVideoFrame(pVideoCamera, &frame);

	    }

error:
	    if (pPictureBuffer) {
	        NEXUS_Memory_Free(pPictureBuffer);
	    }
	    if (pMultiScanBuffer) {
	        NEXUS_Memory_Free(pMultiScanBuffer);
	    }
	    if (pictureDecoder) {
	        NEXUS_PictureDecoder_Close(pictureDecoder);
	    }

		Transcoder_Stop(pTranscoder);
		Transcoder_Close(pTranscoder);
	    VideoCamera_StopVideoCapture(pVideoCamera);

		NEXUS_VideoImageInput_SetSurface(imageInput, NULL);

	    VideoCamera_Close(pVideoCamera);
	    NEXUS_Surface_Destroy(img[2]);
	    NEXUS_Surface_Destroy(img[1]);
	    NEXUS_Surface_Destroy(img[0]);
	    NEXUS_Graphics2D_Close(gfx);
		BKNI_DestroyEvent(gfxEvent);
		BKNI_DestroyEvent(imageEvent);
	    NEXUS_VideoWindow_RemoveAllInputs(window);
	    NEXUS_VideoInput_Shutdown(videoInput);
	    NEXUS_VideoImageInput_Close(imageInput);
		NEXUS_VideoWindow_Close(window);
	    NEXUS_Display_RemoveAllOutputs(display);
	    NEXUS_Display_Close(display);
	    NEXUS_Platform_Uninit();

		BKNI_Free(videoCameraSettings.video.videoDevName);
	}

	B_Event_Destroy(g_eotEvent);
	B_Os_Uninit();

    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
