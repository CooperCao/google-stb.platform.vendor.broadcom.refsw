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

#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_display_vbi.h"
#include "nexus_video_window.h"
#include "nexus_video_decoder.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_playback.h"
#include "nexus_audio_decoder_trick.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_audio_capture.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_component_output.h"
#include "nexus_hdmi_output.h"
#include "nexus_hdmi_output_hdcp.h"
#include "nexus_audio_mixer.h"
#include "nexus_surface.h"
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif
#include "cmdline_args.h"

#include "nexus_ac3_encode.h"
#include "nexus_dolby_volume.h"
#include "nexus_dolby_digital_reencode.h"
#include "nexus_display.h"
#include "nexus_video_window.h"


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "bstd.h"
#include "bkni.h"
#include "fileio_custom.h"

BDBG_MODULE(dolby_ms12_dualplayback);

/* Primary, Secondary, Sound Effects, Application Audio */
#define NUM_DECODES      4
#define PRIMARY_DECODE   0
#define SECONDARY_DECODE 1
#define EFFECTS_DECODE   2
#define APPAUDIO_DECODE  3

#define OUTPUT_FADE_IDX  4

#define INVALID_FADE_LEVEL 0x7FFFFFFF
static unsigned defaultFadeLevel = 0x7FFFFFFF;

#define NUM_CAPTURES     4
#define CAPTURE_STEREO   0
#define CAPTURE_MULTICH  1
#define CAPTURE_COMP     2
#define CAPTURE_COMP4X   3

#define CAPTURE_DEINTERLEAVE_MULTICH    1
#define CAPTURE_LOCALBUFFER_SIZE        (64*1024)

#define AC4_MODE_SS_SD    0 /**< single stream, single decode */
#define AC4_MODE_SS_DD_SI 1 /**< single stream, dual decode, single instance */
#define AC4_MODE_SS_DD_DI 2 /**< single stream, dual decode, dual instance */
#define AC4_MODE_DS_DD    3 /**< dual stream, dual decode */

#define INVALID_PID     0x1fff

/***************************************************************************
Summary:
Certification mode parameters for DAPv2 Mixer
***************************************************************************/
#define DOLBY_DAP_CERT_MODE                        ((unsigned)1<<0)
#define DOLBY_DAP_CERT_DISABLE_MI                  ((unsigned)1<<1)
#define DOLBY_DAP_CERT_DISABLE_MI_SURROUNDCOMP     ((unsigned)1<<2)
#define DOLBY_DAP_CERT_DISABLE_MI_DIALOGENHANCER   ((unsigned)1<<3)
#define DOLBY_DAP_CERT_DISABLE_MI_VOLUMELIMITER    ((unsigned)1<<4)
#define DOLBY_DAP_CERT_DISABLE_MI_INTELLIGENTEQ    ((unsigned)1<<5)
#define DOLBY_DAP_CERT_ENABLE_SURROUND_DECODER     ((unsigned)1<<6)
/* |1111 1xxx xxxx xxxx| */
#define DOLBY_DAP_CERT_EQAMOUNT                    (0xf8000000)
#define DOLBY_DAP_CERT_EQAMOUNT_SHIFT              (27)

/***************************************************************************
Summary:
Certification mode parameters for UDC Decoder
***************************************************************************/
#define DOLBY_UDC_DECORRELATION                    ((unsigned)1<<0)
#define DOLBY_UDC_MDCT_BANDLIMIT                   ((unsigned)1<<1)
/* |1111 1xxx xxxx xxxx| */
#define DOLBY_UDC_OUTPUTMODE_CUSTOM_MASK           (0xf8000000)
#define DOLBY_UDC_OUTPUTMODE_CUSTOM_SHIFT          (27)

/***************************************************************************
Summary:
Certification mode parameters for AC4 Decoder
***************************************************************************/
/* |111x xxxx xxxx xxxx| */
#define DOLBY_AC4_DECODEMODE_MASK                  (0xe0000000)
#define DOLBY_AC4_DECODEMODE_SHIFT                 (29)

static bool hdmiHdcpEnabled = false;
static NEXUS_PlatformConfiguration platformConfig;

#if NEXUS_NUM_AUDIO_PLAYBACKS
typedef struct wave_header_t
{
    unsigned long riff;         /* 'RIFF' */
    unsigned long riffCSize;    /* size in bytes of file - 8 */
    unsigned long wave;         /* 'WAVE' */
    unsigned long fmt;          /* 'fmt ' */
    unsigned long headerLen;    /* header length (should be 16 for PCM) */
    unsigned short format;      /* 1 - pcm */
    unsigned short channels;    /* 1 - mono, 2 - stereo */
    unsigned long samplesSec;   /* samples / second */
    unsigned long bytesSec;     /* bytes / second */
    unsigned short chbits;      /* channels * bits/sample /8 */
    unsigned short bps;         /* bits per sample (8 or 16) */
    /* Extensible format */
    unsigned short cbSize;      /* 2 Size of the extension (0 or 22)  */
    unsigned short wValidBitsPerSample; /* 2 Number of valid bits  */
    unsigned short dwChannelMask; /* 4 Speaker position mask  */
    unsigned char SubFormat[16];  /* SubFormat */

    unsigned long dataSig;      /* 'data' */
    unsigned long dataLen;      /* length of data */
} wave_header_t;

/* 1KHz sine wave at 48 KHz */
static int16_t samples[48] =
{
    0,
    4276,
    8480,
    12539,
    16383,
    19947,
    23169,
    25995,
    28377,
    30272,
    31650,
    32486,
    32767,
    32486,
    31650,
    30272,
    28377,
    25995,
    23169,
    19947,
    16383,
    12539,
    8480,
    4276,
    0,
    -4277,
    -8481,
    -12540,
    -16384,
    -19948,
    -23170,
    -25996,
    -28378,
    -30273,
    -31651,
    -32487,
    -32767,
    -32487,
    -31651,
    -30273,
    -28378,
    -25996,
    -23170,
    -19948,
    -16384,
    -12540,
    -8481,
    -4277
};
#endif

unsigned preset_freq[3][20] = {
    { 32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 20, 1000, 1900, 2100, 6000, 7900, 8100, 15900, 16100, 20000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 20, 1000, 1900, 2100, 6000, 7900, 8100, 15900, 16100, 20000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
unsigned preset_gain[3][20] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 160, 160, 160, 0, 0, 0, 400, 400, -160, -160, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { -160, -160, -160, 400, 400, 400, 0, 0, 160, 160, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
void Intialize_values(unsigned *freq, unsigned *Default_freq, unsigned num)
{
    uint16_t k;
    for ( k = 0; k < num; k++ )
    {
        *freq++ = *Default_freq++;
    }
}

int presid_convert_to_ascii (char * hex, char * ascii)
{
    char * final = ascii;
    unsigned process = 0;
    bool integer = false;
    printf("convert hex string %s to ascii\n", hex);
    switch ( strlen(hex) )
    {
    case 32: process = 32; break;
    case 16: process = 0; break;
    case 5:
    case 4:
    case 3:
    case 2:
    case 1:  process = 0; integer = true; break;
    default:
        printf("invalid presid length %d\n", (int)strlen(hex));
        return -1;
    }

    if ( process == 0 )
    {
        if ( integer )
        {
            int tmp;
            tmp = atoi(hex);
            if ( tmp > 0xffff )
            {
                printf("invalid short presid value %d\n", tmp);
                return -1;
            }

            *ascii++ = (char)(tmp & 0xff);
            *ascii++ = (char)((tmp>>8) & 0xff);
        }
        else
        {
            strcpy(ascii, hex);
        }
    }
    else
    {
        unsigned i;
        for ( i = 0; i < process; i += 8 )
        {
            char tmpStr[9];
            unsigned length = strlen(hex+i);
            unsigned tmp;

            length = ( length > 8 ) ? 8 : length;
            memcpy(tmpStr, hex+i, length);
            tmpStr[8] = '\0';
            length /= 2;
            tmp = strtoul(tmpStr, NULL, 16);
            printf("current dword %x, length %d\n", tmp, length);
            while ( length > 0 )
            {
                #if 0
                *ascii++ = (char)(tmp&0xff);
                tmp = tmp >> 8;
                #else
                *ascii++ = (char)((tmp>>((length-1)*8))&0xff);
                #endif
                printf("%02x(%c)\n", *(ascii-1), *(ascii-1));
                --length;
            }
        }
        *ascii = '\0';
    }

    printf("final string \"%s\"\n", final);

    return 0;
}

#define MAX_FILENAME_LEN    1024
/*----------------------------------------------------------------------------------------DDP dual playback command line arguments------------------------------------------------------------------*/
struct dolby_digital_plus_command_args {
    const char *config_file;
    const char *primary_stream,*secondary_stream,*pcm_stream,*sound_effects_stream,*app_audio_stream;
    FILE *pPcmFile;

    unsigned secondary_substreamId;
    unsigned dialog_level;

    int loopback;
    NEXUS_AudioDecoderSettings decodeSettings;
    NEXUS_AudioCodec audioCodec;
    NEXUS_AudioDecoderCodecSettings ac3CodecSettings;
    NEXUS_AudioDecoderCodecSettings ac4CodecSettings;
    NEXUS_AudioDecoderCodecSettings aacCodecSettings;
    bool enable_dap;
    unsigned dvContentType;
    bool dvOverrideInbandVol;
    bool dv_enable;
    bool dde_enable;
    unsigned ddeDialogBoost;
    unsigned ddeContentCut;
    bool ieq_enable;
    unsigned deqNumBands;
    unsigned deqAmount;
    unsigned deqFrequency[20];
    unsigned deqPreset;
    int deqGain[20];
    bool dapEnaSurDecoder;
    bool miDisable;
    bool miSurCompDisable;
    bool miDdeDisable;
    bool miDvDisable;
    bool miDieqDisable;
    bool mdctBLEnable;
    bool multiCh71;
    bool certificationMode_ddre;
    bool certificationMode_dap;
    bool udcDecorrelation;
    int udcOutModeSpecial;
    bool externalPcm;
    bool externalPcmSpecified;
    bool targetSyncDisabled;
    bool completeFirstFrame;
    bool directMode;
    bool enableVideo;
    bool enableDdre;
    bool enableSpdif;
    bool enableHdmi;
    bool enableDac;
    struct {
        NEXUS_AudioOutputHandle spdif;
        NEXUS_AudioOutputHandle hdmi;
        NEXUS_AudioOutputHandle dac;
        NEXUS_AudioOutputHandle dummy;
    } connectors;
    bool enablePlayback;
    bool enableCompressed;
    bool enableDecode[NUM_DECODES];
    bool enableAtmos;
    bool dualMain;
    bool altStereo;
    NEXUS_DolbyDigitalReencodeProfile compression;
    int mixerBalance;
    unsigned cut;
    unsigned boost;
    unsigned fadeLevel[5];
    unsigned fadeDuration[5];
    unsigned fadeType[5];
    bool fixedEncFormat;
    struct {
        unsigned pid;
        NEXUS_AudioCodec codec;
        char filename[MAX_FILENAME_LEN];
    } decodeAttributes[NUM_DECODES];
    unsigned ac4PresIdxMain;
    unsigned ac4PresIdxAlt;
    unsigned ac4PresIdxAssoc;
    char ac4PresIdMain[NEXUS_AUDIO_AC4_PRESENTATION_ID_LENGTH];
    char ac4PresIdAlt[NEXUS_AUDIO_AC4_PRESENTATION_ID_LENGTH];
    char ac4PresIdAssoc[NEXUS_AUDIO_AC4_PRESENTATION_ID_LENGTH];
    unsigned ac4DecodeMode;
    struct {
        bool enabled;
        char filename[MAX_FILENAME_LEN];
    } audioCapture[NUM_CAPTURES];
};

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
static bool g_isVideoEs = true;


/*---------------------------------------------------------------------Monitoring playback----------------------------------------------------------------------------------------------------------*/
#define  B_HAS_PLAYBACK_MONITOR 0

#if B_HAS_PLAYBACK_MONITOR || NEXUS_NUM_AUDIO_PLAYBACKS
#include <pthread.h>
#include "bkni_multi.h"
#endif

#if B_HAS_PLAYBACK_MONITOR
typedef struct PlaybackMonitorState
{
    bool terminate;
    int iter;
    pthread_t thread;
    BKNI_MutexHandle lock;
    const struct util_opts_t *opts;
    NEXUS_PlaybackHandle playback;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderHandle secondaryDecoder;
    NEXUS_AudioDecoderHandle compressedDecoder;
    const NEXUS_AudioDecoderStartSettings *audioProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_FilePlayHandle customFile;
    NEXUS_FilePlayHandle stickyFile;
    const NEXUS_PlaybackStartSettings *playbackStartSettings;
} PlaybackMonitorState;

static void* monitor_thread(void *state_)
{
    const PlaybackMonitorState *state = state_;
    while ( !state->terminate )
    {
        NEXUS_PlaybackStatus status;
        NEXUS_PlaybackSettings playbackSettings;

        BERR_Code rc;
        bool failed;

        rc = NEXUS_Playback_GetStatus(state->playback, &status);
        BDBG_ASSERT(!rc);
        BKNI_Sleep(1000);
        FileIoSticky_GetFailBit(state->stickyFile, &failed);
        if ( !failed )
        {
            continue;
        }
        BDBG_WRN(("restarting from %u", status.position));
        BKNI_AcquireMutex(state->lock);
        NEXUS_Playback_Stop(state->playback);
        FileIoSticky_ClearFailBit(state->stickyFile);
        if ( state->customFile )
        {
            FileIoCustomProbabilities probabilities;
            FileIoCustom_GetProbabilities(state->customFile, NULL, &probabilities);
            probabilities.error = 0;
            probabilities.nodata = 0;
            probabilities.partial_read = 0;
            FileIoCustom_SetProbabilities(state->customFile, &probabilities, &probabilities);
        }

        /* stop decoder */

        /*  stop_audio(state->opts, state->audioDecoder,state->secondaryDecoder, state->compressedDecoder,capture,state->iter);*/

        NEXUS_Playback_GetSettings(state->playback, &playbackSettings);
        playbackSettings.startPaused = true;
        rc = NEXUS_Playback_SetSettings(state->playback, &playbackSettings);
        BDBG_ASSERT(!rc);

        /* Start decoders */

        /* start playback  */
        rc = NEXUS_Playback_Start(state->playback, state->file, state->playbackStartSettings);
        BDBG_ASSERT(!rc);

        /* seek into desired location */
        rc = NEXUS_Playback_Seek(state->playback, status.position);
        BDBG_ASSERT(!rc);

        /* start playing */
        rc = NEXUS_Playback_Play(state->playback);
        BDBG_ASSERT(!rc);
        BKNI_ReleaseMutex(state->lock);
    }
    return NULL;
}

static void monitor_thread_start(PlaybackMonitorState *state)
{
    int rc;
    BKNI_CreateMutex(&state->lock);
    state->terminate = false;
    rc = pthread_create(&state->thread, NULL, monitor_thread, state);
    BDBG_ASSERT(rc == 0);
    return;
}

static void monitor_thread_stop(PlaybackMonitorState *state)
{
    state->terminate = true;
    pthread_join(state->thread, NULL);
}
#endif /* B_HAS_PLAYBACK_MONITOR */

#if NEXUS_NUM_AUDIO_PLAYBACKS
typedef struct pcm_playback_t
{
    NEXUS_AudioPlaybackHandle playback;
    pthread_t thread;
    BKNI_EventHandle event;
    /* PCM WAV file info */
    wave_header_t wh;
    FILE *pPcmFile;
    unsigned pcmDataStart;
    unsigned pcmDataEnd;
    bool terminate;
} pcm_playback_t;

static void pcm_data_callback(void *pParam1, int param2)
{
    pcm_playback_t *pcm_pb = pParam1;
    /*
    printf("Data callback - channel 0x%08x\n", (unsigned)pParam1);
    */
    BSTD_UNUSED(param2);
    BKNI_SetEvent(pcm_pb->event);
}

static void* playback_thread_task(void *pParam)
{
    BERR_Code rc;
    int16_t *pBuffer;
    size_t bufferSize;
    unsigned offset = 0;
    unsigned i;
    pcm_playback_t *playback = pParam;

    printf("Starting Playback Thread playback %p, playback->playback %p\n", (void *)playback, (void *)playback->playback);

    do
    {
        bool loop = false;
        /* Manage PCM Playback buffer */
        /* Check available buffer space */
        rc = NEXUS_AudioPlayback_GetBuffer(playback->playback, (void **)&pBuffer, &bufferSize);
        /*printf("buffersize %d\n", bufferSize);*/
        if ( rc )
        {
            printf("Error getting playback buffer\n");
            bufferSize = 0;
            pBuffer = NULL;
        }
        if ( playback->pPcmFile != NULL )
        {
            if ( (ftell(playback->pPcmFile) + bufferSize) > playback->pcmDataEnd )
            {
                bufferSize = playback->pcmDataEnd - ftell(playback->pPcmFile);
                loop = true;
            }
            bufferSize = fread(pBuffer, 1, bufferSize, playback->pPcmFile);
            /*printf("read %d bytes from pcm file\n", bufferSize);*/

            if ( loop )
            {
                printf("Looping PCM file\n");
                fseek(playback->pPcmFile, playback->pcmDataStart, SEEK_SET);
            }
        }
        else
        {
            /* Copy samples into buffer */
            bufferSize /= 4;
            for ( i = 0; i < bufferSize; i++ )
            {
                pBuffer[2 * i] = pBuffer[(2 * i) + 1] = samples[offset];
                offset++;
                if ( offset >= 48 )
                {
                    offset = 0;
                }
            }
            bufferSize *= 4;
        }

        if ( bufferSize > 0 )
        {
            /*printf("writing buffersize %d\n", bufferSize);*/
            rc = NEXUS_AudioPlayback_WriteComplete(playback->playback, bufferSize);
            if ( rc )
            {
                printf("Error committing playback buffer\n");
            }
        }

        BKNI_WaitForEvent(playback->event, 5000);
    } while ( !playback->terminate );

    return NULL;
}

static void playback_thread_start(pcm_playback_t *playback)
{
    int rc;
    playback->terminate = false;
    rc = pthread_create(&playback->thread, NULL, playback_thread_task, playback);
    BDBG_ASSERT(rc == 0);
    return;
}

static void playback_thread_stop(pcm_playback_t *playback)
{
    playback->terminate = true;
    pthread_join(playback->thread, NULL);
    BKNI_DestroyEvent(playback->event);
    fclose(playback->pPcmFile);
    playback->event = NULL;
}
#endif

typedef struct capture_resource_t {
    NEXUS_AudioCaptureHandle audioCapture;
    FILE* file[4];
    NEXUS_AudioCaptureFormat format;
} capture_resource_t;

typedef struct app_resources {
    NEXUS_PlatformSettings platformSettings;
    NEXUS_StcChannelHandle stcChannel[NUM_DECODES];
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_PidChannelHandle pcrPidChannel;
    NEXUS_PidChannelHandle videoPidChannel;

    NEXUS_AudioDecoderHandle audioDecoders[NUM_DECODES];
    NEXUS_AudioDecoderHandle compressedDecoder;
    bool initialized[NUM_DECODES];
    bool started[NUM_DECODES];
    bool suspended[NUM_DECODES];
    NEXUS_AudioMixerHandle mixer;
    NEXUS_AudioMixerHandle directMixer;
    NEXUS_AudioMixerHandle directMchMixer;
    NEXUS_AudioDecoderStartSettings audioProgram[NUM_DECODES];
#if NEXUS_NUM_AUDIO_PLAYBACKS
    NEXUS_AudioPlaybackHandle audioPlayback;
    pcm_playback_t pcm_playbacks[1];
#endif
    NEXUS_FilePlayHandle file[NUM_DECODES];
    NEXUS_PlaypumpHandle playpump[NUM_DECODES];
    NEXUS_PlaybackHandle playback[NUM_DECODES];
    NEXUS_DolbyDigitalReencodeHandle ddre;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoWindowHandle window;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    capture_resource_t audioCapture[NUM_CAPTURES];
    BKNI_EventHandle captureEvent;
    pthread_t captureThread;
    bool captureDone;
} app_resources_t;

void audio_capture_callback (void *context, int param)
{
    BKNI_EventHandle event = (BKNI_EventHandle)context;

    /*printf("Capture callback\n");*/
    BKNI_SetEvent(event);
}

static void *capture_thread(void *pParam)
{
    app_resources_t* resources = (app_resources_t*)pParam;
    uint32_t* lbuffer[4] = {NULL,NULL,NULL,NULL};

    while ( !resources->captureDone )
    {
        unsigned c;

        BKNI_WaitForEvent(resources->captureEvent, 1000);
        for ( c = 0; c < NUM_CAPTURES; c++ )
        {
            if ( resources->audioCapture[c].audioCapture )
            {
                unsigned l;
                /* read twice to get as much data as possible */
                for ( l = 0; l < 2; l++)
                {
                    NEXUS_AudioCaptureSettings capSettings;
                    NEXUS_Error rc;
                    uint8_t * buffer = NULL;
                    uint32_t * pSrc;
                    uint32_t * pDst;
                    unsigned size = 0;
                    unsigned pairs = 1;
                    unsigned processed;

                    NEXUS_AudioCapture_GetSettings(resources->audioCapture[c].audioCapture, &capSettings);
                    rc = NEXUS_AudioCapture_GetBuffer(resources->audioCapture[c].audioCapture, (void**)&buffer, &size);
                    if ( rc != NEXUS_SUCCESS )
                    {
                        printf("NEXUS_AudioCapture_GetBuffer returned %u\n", rc);
                        continue;
                    }

                    if ( size > 0 )
                    {
                        switch ( capSettings.format )
                        {
                        case NEXUS_AudioCaptureFormat_e24Bit7_1:
                            pairs = 4;
                            break;
                        case NEXUS_AudioCaptureFormat_e24Bit5_1:
                            pairs = 3;
                            break;
                        default:
                            break;
                        }

                        #if CAPTURE_DEINTERLEAVE_MULTICH
                        if ( pairs > 1 )
                        {
                            unsigned p;
                            unsigned offset = 0;
                            processed = 0;

                            /* allocate local buffers if needed */
                            if ( lbuffer[0] == NULL )
                            {
                                for ( p = 0; p < pairs; p++ )
                                {
                                    lbuffer[p] = BKNI_Malloc(CAPTURE_LOCALBUFFER_SIZE);
                                }
                            }

                            /* limit the data to process to the local buffer size */
                            if ( size > (CAPTURE_LOCALBUFFER_SIZE*pairs) )
                            {
                                size = CAPTURE_LOCALBUFFER_SIZE*pairs;
                            }

                            while ( (size - processed) >= (pairs*8) )
                            {
                                for ( p = 0; p < pairs; p++ )
                                {
                                    pSrc = (uint32_t*)(buffer+processed);
                                    pDst = lbuffer[p]+offset;
                                    /* write 2 x 32bit samples to file */
                                    *pDst = *pSrc;
                                    /*printf("%d %d %08x <- %08x\n", p, offset, *pDst, *pSrc);*/
                                    *(pDst+1) = *(pSrc+1);
                                    /*printf("%d %d %08x <- %08x\n", p, offset+1, *(pDst+1), *(pSrc+1));*/
                                    processed += 8;
                                }
                                offset += 2;
                                /*printf("size %d processed %d\n", size, processed);*/
                            }
                            for ( p = 0; p < pairs; p++ )
                            {
                                /* write 2 x 32bit samples to file */
                                fwrite((void*)lbuffer[p], processed/pairs, 1, resources->audioCapture[c].file[p]);
                            }
                        }
                        else
                        #endif
                        if ( resources->audioCapture[c].file[0] )
                        {
                            fwrite((void*)buffer, size, 1, resources->audioCapture[c].file[0]);
                            processed = size;
                        }
                        else
                        {
                            processed = size;
                        }

                        /*printf("capture %d - wrote %d of %d bytes to file\n", c, processed, size);*/
                        rc = NEXUS_AudioCapture_ReadComplete(resources->audioCapture[c].audioCapture, processed);
                        if ( rc != NEXUS_SUCCESS )
                        {
                            printf("WARNING: NEXUS_AudioCapture_ReadComplete returned %u\n", rc);
                        }
                    }
                }
            }
        }
    }


    /* tear down local buffers */
    {
        unsigned p;
        for ( p = 0; p < 4; p++ )
        {
            if ( lbuffer[p] != NULL )
            {
                BKNI_Free(lbuffer[p]);
                lbuffer[p] = NULL;
            }
        }
    }

    return NULL;
}

typedef struct hotplugCallbackParameters
{
    NEXUS_HdmiOutputHandle hdmiOutput;
    NEXUS_DisplayHandle display;
} hotplugCallbackParameters;

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------Managing the run time change into the config parameters--------------------------------------------------------------------------------*/

static void set_config(char *input, struct dolby_digital_plus_command_args *dolby)
{
    unsigned valLen;
    char *value;
    value = strchr(input, '=');
    *value = 0;
    value++;

    valLen = strlen(value);
    while ( valLen > 0 &&
            (value[valLen-1] == ' ' || value[valLen-1] == '\r' || value[valLen-1] == '\n' ) )
    {
        /*printf("converting character number %d to 0\n", (int)value[valLen-1]);*/
        value[valLen-1] = '\0';
        valLen--;
    }

    if ( !strcmp(input, "OUTPUTMODE") )
    {
        dolby->decodeSettings.outputMode = atoi(value);
    }
    else if ( !strcmp(input, "OUTPUTLFECH_MODE") )
    {
        dolby->decodeSettings.outputLfeMode = atoi(value);
    }
    else if ( !strcmp(input, "DUALMONO_MODE") )
    {
        dolby->decodeSettings.dualMonoMode = atoi(value);
    }
    else if ( !strcmp(input, "DV") )
    {
        dolby->dv_enable = false;
        if ( atoi(value) == 1 )
        {
            dolby->dv_enable = true;
        }
    }
    else if ( !strcmp(input, "DAP_ENABLE") )
    {
        dolby->enable_dap = false;
        if ( atoi(value) == 1 )
        {
            dolby->enable_dap = true;
        }
    }
    else if ( !strcmp(input, "DVCONTENTTYPE") )
    {

        dolby->dvContentType = 7;
        if ( atoi(value) >= 0 && atoi(value) <= 10 )
        {
            dolby->dvContentType = (unsigned)atoi(value);
        }
    }
    else if ( !strcmp(input, "DVOVERRIDEINBAND") )
    {
        dolby->dvOverrideInbandVol = false;
        if ( atoi(value) == 1 )
        {
            dolby->dvOverrideInbandVol = true;
        }
    }
    else if ( !strcmp(input, "DE") )
    {
        dolby->dde_enable = false;
        if ( atoi(value) == 1 )
        {
            dolby->dde_enable = true;
        }
    }
    else if ( !strcmp(input, "DEBOOST") )
    {
        dolby->ddeDialogBoost = 0;
        if ( atoi(value) >= 0 && atoi(value) <= 16 )
        {
            dolby->ddeDialogBoost = atoi(value);
        }
    }
    else if ( !strcmp(input, "DECUT") )
    {
        dolby->ddeContentCut = 0;
        if ( atoi(value) >= 0 && atoi(value) <= 16 )
        {
            dolby->ddeContentCut = atoi(value);
        }
    }
    else if ( !strcmp(input, "IEQ") )
    {
        dolby->ieq_enable = false;
        if ( atoi(value) == 1 )
        {
            dolby->ieq_enable = true;
        }
    }
    else if ( !strcmp(input, "DEQAMOUNT") )
    {
        dolby->deqAmount = 0;
        if ( atoi(value) >= 0 && atoi(value) <= 16 )
        {
            dolby->deqAmount = atoi(value);
        }
    }
    else if ( !strcmp(input, "DEQBAND") )
    {
        dolby->deqNumBands = 10;
        if ( atoi(value) >= 0 && atoi(value) <= 20 )
        {
            dolby->deqNumBands = atoi(value);
        }
    }
    else if ( !strcmp(input, "IEQ_PRESET") )
    {
        dolby->deqPreset = atoi(value);

        Intialize_values(&dolby->deqFrequency[0], preset_freq[dolby->deqPreset], dolby->deqNumBands);
        Intialize_values((uint32_t *)&dolby->deqGain[0], preset_gain[dolby->deqPreset], dolby->deqNumBands);
    }
    else if ( !strcmp(input, "ENABLESURDEC") )
    {
        dolby->dapEnaSurDecoder = false;
        if ( atoi(value) == 1 )
        {
            dolby->dapEnaSurDecoder = true;
        }
    }
    else if ( !strcmp(input, "FADETYPEPRI") )
    {
        if ( atoi(value) >= 0 && atoi(value) <= 2 )
        {
            dolby->fadeType[PRIMARY_DECODE] = atoi(value);
        }
    }
    else if ( !strcmp(input, "FADEDURATIONPRI") )
    {
        if ( atoi(value) >= 0 && atoi(value) <= 60000 )
        {
            dolby->fadeDuration[PRIMARY_DECODE] = atoi(value);
        }
    }
    else if ( !strcmp(input, "FADELEVELPRI") )
    {
        if ( atoi(value) >= 0 && (unsigned)atoi(value) <= defaultFadeLevel )
        {
            dolby->fadeLevel[PRIMARY_DECODE] = (unsigned)atoi(value);
        }
    }
    else if ( !strcmp(input, "FADETYPESEC") )
    {
        if ( atoi(value) >= 0 && atoi(value) <= 2 )
        {
            dolby->fadeType[SECONDARY_DECODE] = atoi(value);
        }
    }
    else if ( !strcmp(input, "FADEDURATIONSEC") )
    {
        if ( atoi(value) >= 0 && atoi(value) <= 60000 )
        {
            dolby->fadeDuration[SECONDARY_DECODE] = atoi(value);
        }
    }
    else if ( !strcmp(input, "FADELEVELSEC") )
    {
        if ( atoi(value) >= 0 && (unsigned)atoi(value) <= defaultFadeLevel )
        {
            dolby->fadeLevel[SECONDARY_DECODE] = (unsigned)atoi(value);
        }
    }
    else if ( !strcmp(input, "FADETYPESE") )
    {
        if ( atoi(value) >= 0 && atoi(value) <= 2 )
        {
            dolby->fadeType[EFFECTS_DECODE] = atoi(value);
        }
    }
    else if ( !strcmp(input, "FADEDURATIONSE") )
    {
        if ( atoi(value) >= 0 && atoi(value) <= 60000 )
        {
            dolby->fadeDuration[EFFECTS_DECODE] = atoi(value);
        }
    }
    else if ( !strcmp(input, "FADELEVELSE") )
    {
        if ( atoi(value) >= 0 && (unsigned)atoi(value) <= defaultFadeLevel )
        {
            dolby->fadeLevel[EFFECTS_DECODE] = (unsigned)atoi(value);
        }
    }
    else if ( !strcmp(input, "FADETYPEAA") )
    {
        if ( atoi(value) >= 0 && atoi(value) <= 2 )
        {
            dolby->fadeType[APPAUDIO_DECODE] = atoi(value);
        }
    }
    else if ( !strcmp(input, "FADEDURATIONAA") )
    {
        if ( atoi(value) >= 0 && atoi(value) <= 60000 )
        {
            dolby->fadeDuration[APPAUDIO_DECODE] = atoi(value);
        }
    }
    else if ( !strcmp(input, "FADELEVELAA") )
    {
        if ( atoi(value) >= 0 && (unsigned)atoi(value) <= defaultFadeLevel )
        {
            dolby->fadeLevel[APPAUDIO_DECODE] = (unsigned)atoi(value);
        }
    }
    else if ( !strcmp(input, "FADETYPEOUT") )
    {
        if ( atoi(value) >= 0 && atoi(value) <= 2 )
        {
            dolby->fadeType[OUTPUT_FADE_IDX] = atoi(value);
        }
    }
    else if ( !strcmp(input, "FADEDURATIONOUT") )
    {
        if ( atoi(value) >= 0 && atoi(value) <= 60000 )
        {
            dolby->fadeDuration[OUTPUT_FADE_IDX] = atoi(value);
        }
    }
    else if ( !strcmp(input, "FADELEVELOUT") )
    {
        if ( atoi(value) >= 0 && (unsigned)atoi(value) <= defaultFadeLevel )
        {
            dolby->fadeLevel[OUTPUT_FADE_IDX] = (unsigned)atoi(value);
        }
    }
    else if ( !strcmp(input, "CERT_DDRE") )
    {
        dolby->certificationMode_ddre = false;
        if ( atoi(value) == 1 )
        {
            dolby->certificationMode_ddre = true;
        }
    }
    else if ( !strcmp(input, "CERT_DAP") )
    {
        dolby->certificationMode_dap = false;
        if ( atoi(value) == 1 )
        {
            dolby->certificationMode_dap = true;
        }
    }
    else if ( !strcmp(input, "UDC_OUTMODE") )
    {
        dolby->udcOutModeSpecial = 0;
        if ( atoi(value) >= -1 )
        {
            dolby->udcOutModeSpecial = atoi(value);
        }
    }
    else if ( !strcmp(input, "UDC_DECORRELATION") )
    {
        dolby->udcDecorrelation = false;
        if ( atoi(value) == 1 )
        {
            dolby->udcDecorrelation = true;
        }
    }
    else if ( !strcmp(input, "MIDISABLE") )
    {
        dolby->miDisable = false;
        if ( atoi(value) == 1 )
        {
            dolby->miDisable = true;
        }
    }
    else if ( !strcmp(input, "MISURCOMPDISABLE") )
    {
        dolby->miSurCompDisable = false;
        if ( atoi(value) == 1 )
        {
            dolby->miSurCompDisable = true;
        }
    }
    else if ( !strcmp(input, "MIDDVDISABLE") )
    {
        dolby->miDvDisable = false;
        if ( atoi(value) == 1 )
        {
            dolby->miDvDisable = true;
        }
    }
    else if ( !strcmp(input, "MIDDEDISABLE") )
    {
        dolby->miDdeDisable = false;
        if ( atoi(value) == 1 )
        {
            dolby->miDdeDisable = true;
        }
    }
    else if ( !strcmp(input, "MIDIEQDISABLE") )
    {
        dolby->miDieqDisable = false;
        if ( atoi(value) == 1 )
        {
            dolby->miDieqDisable = true;
        }
    }
    else if ( !strcmp(input, "MDCTBLENABLE") )
    {
        dolby->mdctBLEnable = false;
        if ( atoi(value) == 1 )
        {
            dolby->mdctBLEnable = true;
        }
    }
    else if ( !strcmp(input, "MULTICH") )
    {
        dolby->multiCh71 = false;
        if ( atoi(value) == 8 )
        {
            dolby->multiCh71 = true;
            dolby->decodeSettings.outputMode = NEXUS_AudioMode_e3_4;
        }
    }
    else if ( !strcmp(input, "DIRECT") )
    {
        dolby->directMode = false;
        if ( atoi(value) == 1 )
        {
            dolby->directMode = true;
        }
    }
    else if ( !strcmp(input, "SPDIF") )
    {
        dolby->enableSpdif = true;
        if ( atoi(value) == 0 )
        {
            dolby->enableSpdif = false;
        }
    }
    else if ( !strcmp(input, "HDMI") )
    {
        dolby->enableHdmi = true;
        if ( atoi(value) == 0 )
        {
            dolby->enableHdmi = false;
        }
    }
    else if ( !strcmp(input, "DAC") )
    {
        dolby->enableDac = true;
        if ( atoi(value) == 0 )
        {
            dolby->enableDac = false;
        }
    }
    else if ( !strcmp(input, "DDRE") )
    {
        dolby->enableDdre = true;
        if ( atoi(value) == 0 )
        {
            dolby->enableDdre = false;
        }
    }
    else if ( !strcmp(input, "FIXEDENCODE") )
    {
        dolby->fixedEncFormat = false;
        if ( atoi(value) == 1 )
        {
            dolby->fixedEncFormat = true;
        }
    }
    else if ( !strcmp(input, "PCMPLAYBACK") )
    {
        dolby->enablePlayback = false;
        if ( atoi(value) == 1 )
        {
            dolby->enablePlayback = true;
        }
    }
    else if ( !strcmp(input, "COMPRESSED") )
    {
        dolby->enableCompressed = false;
        if ( atoi(value) == 1 )
        {
            dolby->enableCompressed = true;
        }
    }
    else if ( !strcmp(input, "SECONDARY") )
    {
        dolby->enableDecode[SECONDARY_DECODE] = true;
        if ( atoi(value) == 0 )
        {
            dolby->enableDecode[SECONDARY_DECODE] = false;
        }
    }
    else if ( !strcmp(input, "SOUNDEFFECTS") )
    {
        dolby->enableDecode[EFFECTS_DECODE] = true;
        if ( atoi(value) == 0 )
        {
            dolby->enableDecode[EFFECTS_DECODE] = false;
        }
    }
    else if ( !strcmp(input, "APPAUDIO") )
    {
        dolby->enableDecode[APPAUDIO_DECODE] = true;
        if ( atoi(value) == 0 )
        {
            dolby->enableDecode[APPAUDIO_DECODE] = false;
        }
    }
    else if ( !strcmp(input, "ATMOS") )
    {
        dolby->enableAtmos = true;
        if ( atoi(value) == 0 )
        {
            dolby->enableAtmos = false;
        }
    }
    else if ( !strcmp(input, "COMPRESSION") )
    {
        if ( atoi(value) >= NEXUS_DolbyDigitalReencodeProfile_eMax ||
             atoi(value) < 0 )
        {
            dolby->compression = NEXUS_DolbyDigitalReencodeProfile_eFilmStandardCompression;
        }
        else
        {
            dolby->compression = (NEXUS_DolbyDigitalReencodeProfile)atoi(value);
        }
    }
    else if ( !strcmp(input, "TARGET_SYNC_DISABLED") )
    {
        dolby->targetSyncDisabled = false;
        if ( atoi(value) == 1 )
        {
            dolby->targetSyncDisabled = true;
        }
    }
    else if ( !strcmp(input, "COMPLETE_FIRST_FRAME") )
    {
        dolby->completeFirstFrame = false;
        if ( atoi(value) == 1 )
        {
            dolby->completeFirstFrame = true;
        }
    }
    else if ( !strcmp(input, "FILEPRIMARY") )
    {
        if ( strlen(value) > MAX_FILENAME_LEN )
        {
            printf("Increase MAX_FILENAME_LEN to be > %d\n", (int)strlen(value));
        }
        if ( strlen(value) <= 0 || strlen(value) > MAX_FILENAME_LEN )
        {
            strcpy(dolby->decodeAttributes[PRIMARY_DECODE].filename, "");
        }
        else
        {
            strcpy(dolby->decodeAttributes[PRIMARY_DECODE].filename, value);
        }
    }
    else if ( !strcmp(input, "CODECPRIMARY") )
    {
        if ( ((unsigned)atoi(value)) > NEXUS_AudioCodec_eMax )
        {
            dolby->decodeAttributes[PRIMARY_DECODE].codec = NEXUS_AudioCodec_eMax;
        }
        else
        {
            dolby->decodeAttributes[PRIMARY_DECODE].codec = lookup(g_audioCodecStrs, value);
        }
    }
    else if ( !strcmp(input, "PIDPRIMARY") )
    {
        if ( ((unsigned)atoi(value)) > INVALID_PID )
        {
            dolby->decodeAttributes[PRIMARY_DECODE].pid = INVALID_PID;
        }
        else
        {
            dolby->decodeAttributes[PRIMARY_DECODE].pid = atoi(value);
        }
    }
    else if ( !strcmp(input, "FILESECONDARY") )
    {
        if ( strlen(value) > MAX_FILENAME_LEN )
        {
            printf("Increase MAX_FILENAME_LEN to be > %d\n", (int)strlen(value));
        }
        if ( strlen(value) <= 0 || strlen(value) > MAX_FILENAME_LEN )
        {
            strcpy(dolby->decodeAttributes[SECONDARY_DECODE].filename, "");
        }
        else
        {
            strcpy(dolby->decodeAttributes[SECONDARY_DECODE].filename, value);
        }
    }
    else if ( !strcmp(input, "CODECSECONDARY") )
    {
        if ( ((unsigned)atoi(value)) > NEXUS_AudioCodec_eMax )
        {
            dolby->decodeAttributes[SECONDARY_DECODE].codec = NEXUS_AudioCodec_eMax;
        }
        else
        {
            dolby->decodeAttributes[SECONDARY_DECODE].codec = lookup(g_audioCodecStrs, value);
        }
    }
    else if ( !strcmp(input, "PIDSECONDARY") )
    {
        if ( ((unsigned)atoi(value)) > INVALID_PID )
        {
            dolby->decodeAttributes[SECONDARY_DECODE].pid = INVALID_PID;
        }
        else
        {
            dolby->decodeAttributes[SECONDARY_DECODE].pid = atoi(value);
        }
    }
    else if ( !strcmp(input, "FILEEFFECTS") )
    {
        if ( strlen(value) > MAX_FILENAME_LEN )
        {
            printf("Increase MAX_FILENAME_LEN to be > %d\n", (int)strlen(value));
        }
        if ( strlen(value) <= 0 || strlen(value) > MAX_FILENAME_LEN )
        {
            strcpy(dolby->decodeAttributes[EFFECTS_DECODE].filename, "");
        }
        else
        {
            strcpy(dolby->decodeAttributes[EFFECTS_DECODE].filename, value);
        }
    }
    else if ( !strcmp(input, "FILEAPPAUDIO") )
    {
        if ( strlen(value) > MAX_FILENAME_LEN )
        {
            printf("Increase MAX_FILENAME_LEN to be > %d\n", (int)strlen(value));
        }
        if ( strlen(value) <= 0 || strlen(value) > MAX_FILENAME_LEN )
        {
            strcpy(dolby->decodeAttributes[APPAUDIO_DECODE].filename, "");
        }
        else
        {
            strcpy(dolby->decodeAttributes[APPAUDIO_DECODE].filename, value);
        }
    }
    else if ( !strcmp(input, "CAPTURESTEREO") )
    {
        dolby->audioCapture[CAPTURE_STEREO].enabled = false;
        if ( atoi(value) == 1 )
        {
            dolby->audioCapture[CAPTURE_STEREO].enabled = true;
        }
    }
    else if ( !strcmp(input, "CAPTUREMULTICH") )
    {
        dolby->audioCapture[CAPTURE_MULTICH].enabled = false;
        if ( atoi(value) == 1 )
        {
            dolby->audioCapture[CAPTURE_MULTICH].enabled = true;
        }
    }
    else if ( !strcmp(input, "CAPTURECOMP") )
    {
        dolby->audioCapture[CAPTURE_COMP].enabled = false;
        if ( atoi(value) == 1 )
        {
            dolby->audioCapture[CAPTURE_COMP].enabled = true;
        }
    }
    else if ( !strcmp(input, "CAPTURECOMP4X") )
    {
        dolby->audioCapture[CAPTURE_COMP4X].enabled = false;
        if ( atoi(value) == 1 )
        {
            dolby->audioCapture[CAPTURE_COMP4X].enabled = true;
        }
    }
    else if ( !strcmp(input, "CAPTURESTEREOFILENAME") )
    {
        if ( strlen(value) > MAX_FILENAME_LEN )
        {
            printf("Increase MAX_FILENAME_LEN to be > %d\n", strlen(value));
        }
        strcpy(dolby->audioCapture[CAPTURE_STEREO].filename, value);
    }
    else if ( !strcmp(input, "CAPTUREMULTICHFILENAME") )
    {
        if ( strlen(value) > MAX_FILENAME_LEN )
        {
            printf("Increase MAX_FILENAME_LEN to be > %d\n", strlen(value));
        }
        strcpy(dolby->audioCapture[CAPTURE_MULTICH].filename, value);
    }
    else if ( !strcmp(input, "CAPTURECOMPFILENAME") )
    {
        if ( strlen(value) > MAX_FILENAME_LEN )
        {
            printf("Increase MAX_FILENAME_LEN to be > %d\n", strlen(value));
        }
        strcpy(dolby->audioCapture[CAPTURE_COMP].filename, value);
    }
    else if ( !strcmp(input, "CAPTURECOMP4XFILENAME") )
    {
        if ( strlen(value) > MAX_FILENAME_LEN )
        {
            printf("Increase MAX_FILENAME_LEN to be > %d\n", strlen(value));
        }
        strcpy(dolby->audioCapture[CAPTURE_COMP4X].filename, value);
    }
    else if ( dolby->audioCodec == NEXUS_AudioCodec_eAc3 || dolby->audioCodec == NEXUS_AudioCodec_eAc3Plus )
    {
        if ( !strcmp(input, "DOWNMIX_MODE") )
        {
            if ( atoi(value) == 0 )
            {
                dolby->ac3CodecSettings.codecSettings.ac3Plus.stereoDownmixMode = NEXUS_AudioDecoderDolbyStereoDownmixMode_eAutomatic; /*Lo/Ro always*/
            }
            else if ( atoi(value) == 1 )
            {
                dolby->ac3CodecSettings.codecSettings.ac3Plus.stereoDownmixMode = NEXUS_AudioDecoderDolbyStereoDownmixMode_eDolbySurroundCompatible; /*Lo/Ro always*/
            }
            else if ( atoi(value) == 2 )
            {
                dolby->ac3CodecSettings.codecSettings.ac3Plus.stereoDownmixMode = NEXUS_AudioDecoderDolbyStereoDownmixMode_eStandard; /*Lo/Ro always*/
            }
            else
            {
                printf("\n\n Only DOWNMIX_MODE=0,1,2 is allowed for this application. \n\n");
            }
        }
        else if ( !strcmp(input, "RFMODE") )
        {
            if ( atoi(value) == 0 )
            {
                dolby->ac3CodecSettings.codecSettings.ac3Plus.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            }
            else if ( atoi(value) == 1 )
            {
                dolby->ac3CodecSettings.codecSettings.ac3Plus.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            }
        }
        else if ( !strcmp(input, "RFMODE_DOWNMIX") )
        {
            if ( atoi(value) == 0 )
            {
                dolby->ac3CodecSettings.codecSettings.ac3Plus.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            }
            else if ( atoi(value) == 1 )
            {
                dolby->ac3CodecSettings.codecSettings.ac3Plus.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            }
        }
        else if ( !strcmp(input, "DRCCUT") )
        {
            dolby->ac3CodecSettings.codecSettings.ac3Plus.cut = atoi(value);
            /*dolby->ac3CodecSettings.codecSettings.ac3Plus.cutDownmix = atoi(value);*/
        }
        else if ( !strcmp(input, "DRCBOOST") )
        {
            dolby->ac3CodecSettings.codecSettings.ac3Plus.boost = atoi(value);
            /*dolby->ac3CodecSettings.codecSettings.ac3Plus.boostDownmix = atoi(value);*/
        }
        else if ( !strcmp(input, "DRCCUT_DOWNMIX") )
        {
            dolby->ac3CodecSettings.codecSettings.ac3Plus.cutDownmix = atoi(value);
        }
        else if ( !strcmp(input, "DRCBOOST_DOWNMIX") )
        {
            dolby->ac3CodecSettings.codecSettings.ac3Plus.boostDownmix = atoi(value);
        }
        else if ( !strcmp(input, "MIXERBALANCE") )
        {
            dolby->mixerBalance = atoi(value);
        }
        else if ( !strcmp(input, "SECONDARY_SUBSTREAMID") )
        {
            dolby->secondary_substreamId = atoi(value);
        }
        else if ( !strcmp(input, "DIALOG_LEVEL") )
        {
            dolby->dialog_level = atoi(value);
        }
    }
    else if ( dolby->audioCodec == NEXUS_AudioCodec_eAc4 )
    {
        if ( !strcmp(input, "DOWNMIX_MODE") )
        {
            dolby->ac4CodecSettings.codecSettings.ac4.stereoMode = NEXUS_AudioDecoderStereoDownmixMode_eLtRt; /*Lo/Ro always*/
            if ( atoi(value) == 1 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.stereoMode = NEXUS_AudioDecoderStereoDownmixMode_eLoRo; /*Lo/Ro always*/
            }
        }
        else if ( !strcmp(input, "RFMODE") )
        {
            if ( atoi(value) == 0 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            }
            else if ( atoi(value) == 1 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            }
        }
        else if ( !strcmp(input, "RFMODE_DOWNMIX") )
        {
            if ( atoi(value) == 0 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            }
            else if ( atoi(value) == 1 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            }
        }
        else if ( !strcmp(input, "AC4DRCSCALEHI") )
        {
            dolby->ac4CodecSettings.codecSettings.ac4.drcScaleHi = 100;
            if ( atoi(value) >= 0 && atoi(value) <= 100 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.drcScaleHi = atoi(value);
            }
        }
        else if ( !strcmp(input, "AC4DRCSCALELOW") )
        {
            dolby->ac4CodecSettings.codecSettings.ac4.drcScaleLow = 100;
            if ( atoi(value) >= 0 && atoi(value) <= 100 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.drcScaleLow = atoi(value);
            }
        }
        else if ( !strcmp(input, "AC4DRCSCALEHIDOWNMIX") )
        {
            dolby->ac4CodecSettings.codecSettings.ac4.drcScaleHiDownmix = 100;
            if ( atoi(value) >= 0 && atoi(value) <= 100 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.drcScaleHiDownmix = atoi(value);
            }
        }
        else if ( !strcmp(input, "AC4DRCSCALELOWDOWNMIX") )
        {
            dolby->ac4CodecSettings.codecSettings.ac4.drcScaleLowDownmix = 100;
            if ( atoi(value) >= 0 && atoi(value) <= 100 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.drcScaleLowDownmix = atoi(value);
            }
        }
        else if ( !strcmp(input, "AC4PROGSELECT") )
        {
            dolby->ac4CodecSettings.codecSettings.ac4.programSelection = 0;
            if ( atoi(value) <= 2 && atoi(value) >= 0 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.programSelection = atoi(value);
            }
        }
        else if ( !strcmp(input, "AC4DECODEMODE") )
        {
            dolby->ac4DecodeMode = 0;
            if ( atoi(value) <= 3 && atoi(value) >= 0 )
            {
                dolby->ac4DecodeMode = atoi(value) + 1; /* add 1 for certification mode masking */
            }
        }
        else if ( !strcmp(input, "AC4PROGBALANCE") )
        {
            dolby->ac4CodecSettings.codecSettings.ac4.programBalance = atoi(value);
            if ( atoi(value) > 32 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.programBalance = 32;
            }
            if ( atoi(value) < -32 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.programBalance = -32;
            }
        }
        else if ( !strcmp(input, "AC4DEAMOUNT") )
        {
            dolby->ac4CodecSettings.codecSettings.ac4.dialogEnhancerAmount = atoi(value);
            if ( atoi(value) > 12 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.dialogEnhancerAmount = 12;
            }
            if ( atoi(value) < -12 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.dialogEnhancerAmount = -12;
            }
        }
        else if ( !strcmp(input, "AC4PRESSELECT") )
        {
            dolby->ac4CodecSettings.codecSettings.ac4.selectionMode = NEXUS_AudioDecoderAc4PresentationSelectionMode_eAuto;
            if ( atoi(value) <= 2 && atoi(value) >= 0 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.selectionMode = (NEXUS_AudioDecoderAc4PresentationSelectionMode)atoi(value);
            }
        }
        else if ( !strcmp(input, "AC4ASSOCMIXING") )
        {
            dolby->ac4CodecSettings.codecSettings.ac4.enableAssociateMixing = false;
            if ( atoi(value) == 1 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.enableAssociateMixing = true;
            }
        }
        else if ( !strcmp(input, "AC4PRESINDEXMAIN") )
        {
            if ( atoi(value) <= 511 && atoi(value) >= 0 )
            {
                dolby->ac4PresIdxMain = atoi(value);
            }
        }
        else if ( !strcmp(input, "AC4PRESINDEXALT") )
        {
            if ( atoi(value) <= 511 && atoi(value) >= 0 )
            {
                dolby->ac4PresIdxAlt = atoi(value);
            }
        }
        else if ( !strcmp(input, "AC4PRESINDEXASSOC") )
        {
            if ( atoi(value) <= 511 && atoi(value) >= 0 )
            {
                dolby->ac4PresIdxAssoc = atoi(value);
            }
        }
        else if ( !strcmp(input, "AC4PRESIDMAIN") )
        {
            presid_convert_to_ascii(value, dolby->ac4PresIdMain);
        }
        else if ( !strcmp(input, "AC4PRESIDALT") )
        {
            presid_convert_to_ascii(value, dolby->ac4PresIdAlt);
        }
        else if ( !strcmp(input, "AC4PRESIDASSOC") )
        {
            presid_convert_to_ascii(value, dolby->ac4PresIdAssoc);
        }
        else if ( !strcmp(input, "AC4ASSOCTYPE") )
        {
            dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferredAssociateType = NEXUS_AudioAc4AssociateType_eNotSpecified;
            dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferredAssociateType = NEXUS_AudioAc4AssociateType_eNotSpecified;
            if ( atoi(value) <= 3 && atoi(value) >= 0 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferredAssociateType = (NEXUS_AudioAc4AssociateType)atoi(value);
                dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferredAssociateType = (NEXUS_AudioAc4AssociateType)atoi(value);
            }
        }
        else if ( !strcmp(input, "AC4PREFERLANG") )
        {
            dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferLanguageOverAssociateType = false;
            dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferLanguageOverAssociateType = false;
            if ( atoi(value) == 1 )
            {
                dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferLanguageOverAssociateType = true;
                dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferLanguageOverAssociateType = true;
            }
        }
        else if ( !strcmp(input, "AC4LANGPREF") )
        {
            if ( strlen(value) <= 1 && atoi(value) == 0 )
            {
                memset(dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[0].selection, 0, sizeof(char)*NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH);
                memset(dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[0].selection, 0, sizeof(char)*NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH);
            }
            else
            {
                strcpy(dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[0].selection, value);
                strcpy(dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[0].selection, value);
            }
        }
        else if ( !strcmp(input, "AC4LANGPREF2") )
        {
            if ( strlen(value) <= 1 && atoi(value) == 0 )
            {
                memset(dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[1].selection, 0, sizeof(char)*NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH);
                memset(dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[1].selection, 0, sizeof(char)*NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH);
            }
            else
            {
                strcpy(dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[1].selection, value);
                strcpy(dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[1].selection, value);
            }
        }
        else if ( !strcmp(input, "MIXERBALANCE") )
        {
            dolby->mixerBalance = atoi(value);
        }
        else if ( !strcmp(input, "SECONDARY_SUBSTREAMID") )
        {
            dolby->secondary_substreamId = atoi(value);
        }
        else if ( !strcmp(input, "DIALOG_LEVEL") )
        {
            dolby->dialog_level = atoi(value);
        }
    }
    else if ( dolby->audioCodec == NEXUS_AudioCodec_eAacAdts || dolby->audioCodec == NEXUS_AudioCodec_eAacLoas ||
              dolby->audioCodec == NEXUS_AudioCodec_eAacPlusAdts || dolby->audioCodec == NEXUS_AudioCodec_eAacPlusLoas )
    {
        if ( !strcmp(input, "DOWNMIX_MODE") )
        {
            if ( atoi(value) == 0 )
            {
                dolby->aacCodecSettings.codecSettings.aacPlus.downmixMode = NEXUS_AudioDecoderAacDownmixMode_eArib;
            }
            else if ( atoi(value) == 1 )
            {
                dolby->aacCodecSettings.codecSettings.aacPlus.downmixMode = NEXUS_AudioDecoderAacDownmixMode_eLtRt;
            }
            else if ( atoi(value) == 2 )
            {
                dolby->aacCodecSettings.codecSettings.aacPlus.downmixMode = NEXUS_AudioDecoderAacDownmixMode_eLoRo;
            }
            else
            {
                printf("\n\n Only DOWNMIX_MODE=0-Arib,1-LtRt,2-LoRo is allowed for this application. \n\n");
            }
        }
        else if ( !strcmp(input, "DRCCUT") )
        {
            dolby->aacCodecSettings.codecSettings.aacPlus.cut = atoi(value);
        }
        else if ( !strcmp(input, "DRCBOOST") )
        {
            dolby->aacCodecSettings.codecSettings.aacPlus.boost = atoi(value);
        }
        else if ( !strcmp(input, "DRCCUT_DOWNMIX") )
        {
            dolby->aacCodecSettings.codecSettings.aacPlus.cut = atoi(value);
        }
        else if ( !strcmp(input, "DRCBOOST_DOWNMIX") )
        {
            dolby->aacCodecSettings.codecSettings.aacPlus.boost = atoi(value);
        }
        else if ( !strcmp(input, "MIXERBALANCE") )
        {
            dolby->mixerBalance = atoi(value);
        }
        else if ( !strcmp(input, "RFMODE") )
        {
            if ( atoi(value) == 0 )
            {
                dolby->aacCodecSettings.codecSettings.aacPlus.drcMode = NEXUS_AudioDecoderDolbyPulseDrcMode_eLine;
            }
            else if ( atoi(value) == 1 )
            {
                dolby->aacCodecSettings.codecSettings.aacPlus.drcMode = NEXUS_AudioDecoderDolbyPulseDrcMode_eRf;
            }
        }
        else if ( !strcmp(input, "RFMODE_DOWNMIX") )
        {
            if ( atoi(value) == 0 )
            {
                dolby->aacCodecSettings.codecSettings.aacPlus.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
            }
            else if ( atoi(value) == 1 )
            {
                dolby->aacCodecSettings.codecSettings.aacPlus.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
            }
        }
        else if ( !strcmp(input, "DRC_DEFAULT_LEVEL") )
        {
            dolby->aacCodecSettings.codecSettings.aacPlus.drcDefaultLevel = atoi(value);
        }
    }
    else if ( !strcmp(input, "DRCCUT") )
    {
        /* This will be used only if the input is from non-Dolby codec */
        dolby->cut = atoi(value);
    }
    else if ( !strcmp(input, "DRCBOOST") )
    {
        /* This will be used only if the input is from non-Dolby codec */
        dolby->boost = atoi(value);
    }

    return;
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------Setting the configuration parameters through the config file------------------------------------------------------------*/
static void parse_config_file(struct dolby_digital_plus_command_args *dolby, bool print_line)
{
    char line[400];
    FILE *f = fopen(dolby->config_file, "r");

    if ( f )
    {
        while ( fgets(line, 400, f) )
        {
            if ( print_line )
            {
                printf("read line:%s", line);
            }
            set_config(line, dolby);
        }

        fclose(f);
    }
    else
    {
        fprintf(stderr, "Unable to parse config file, using defaults\n");
    }

    printf("done parsing config file\n");

    return;
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------Printing the current values of the config parameters--------------------------------------------------------------------------*/
static void print_settings(NEXUS_AudioDecoderSettings decodeSettings, NEXUS_AudioDecoderCodecSettings codecSettings, struct dolby_digital_plus_command_args *dolby)
{
    printf("\n\n------------- The current values of the configuration parameters are as follows: -------------- \n\n");

    printf("\t OUTPUTMODE = %d\n", decodeSettings.outputMode);
    printf("\t OUTPUTLFECH_MODE = %d\n", decodeSettings.outputLfeMode);
    printf("\t DUALMONO_MODE = %d\n", decodeSettings.dualMonoMode);
    printf("\t DOWNMIX_MODE = %d\n", (codecSettings.codecSettings.ac3Plus.stereoDownmixMode));
    if ( codecSettings.codec == NEXUS_AudioCodec_eAc3 || codecSettings.codec == NEXUS_AudioCodec_eAc3Plus )
    {
        printf("DDP/AC3 SETTINGS\n");

        printf("\t RFMODE = %d\n", codecSettings.codecSettings.ac3Plus.drcMode);
        printf("\t RFMODE_DOWNMIX = %d\n", codecSettings.codecSettings.ac3Plus.drcModeDownmix);
        printf("\t DRCCUT = %d\n", codecSettings.codecSettings.ac3Plus.cut);
        printf("\t DRCBOOST = %d\n", codecSettings.codecSettings.ac3Plus.boost);
        printf("\t DRCCUT_DOWNMIX = %d\n", codecSettings.codecSettings.ac3Plus.cutDownmix);
        printf("\t DRCBOOST_DOWNMIX = %d\n", codecSettings.codecSettings.ac3Plus.boostDownmix);
        printf("\t SECONDARY_SUBSTREAMID = %d\n", codecSettings.codecSettings.ac3Plus.substreamId);
        printf("\t ATMOS = %d\n", codecSettings.codecSettings.ac3Plus.enableAtmosProcessing);
    }
    else if ( codecSettings.codec == NEXUS_AudioCodec_eAacAdts || codecSettings.codec == NEXUS_AudioCodec_eAacLoas ||
              codecSettings.codec == NEXUS_AudioCodec_eAacPlusAdts || codecSettings.codec == NEXUS_AudioCodec_eAacPlusLoas )
    {
        printf("AAC SETTINGS\n");

        printf("\t RFMODE = %d\n", codecSettings.codecSettings.aacPlus.drcMode);
        printf("\t RFMODE_DOWNMIX = %d\n", codecSettings.codecSettings.aacPlus.drcMode);
        printf("\t DRCCUT = %d\n", codecSettings.codecSettings.aacPlus.cut);
        printf("\t DRCBOOST = %d\n", codecSettings.codecSettings.aacPlus.boost);
        printf("\t DRCCUT_DOWNMIX = %d\n", codecSettings.codecSettings.aacPlus.cut);
        printf("\t DRCBOOST_DOWNMIX = %d\n", codecSettings.codecSettings.aacPlus.boost);
    }
    else if ( codecSettings.codec == NEXUS_AudioCodec_eAc4 )
    {
        printf("\n\tAC4 SETTINGS:\n");

        printf("\t PRES INDEX MAIN = %d\n", dolby->ac4PresIdxMain);
        printf("\t PRES INDEX ALT = %d\n", dolby->ac4PresIdxAlt);
        printf("\t PRES INDEX ASSOC = %d\n", dolby->ac4PresIdxAssoc);
        printf("\t PRES ID MAIN = %s\n", dolby->ac4PresIdMain);
        printf("\t PRES ID ALT = %s\n", dolby->ac4PresIdAlt);
        printf("\t PRES ID ASSOC = %s\n", dolby->ac4PresIdAssoc);
        printf("\t STEREO MODE = %d\n", dolby->ac4CodecSettings.codecSettings.ac4.stereoMode);
        printf("\t DRC MODE = %d\n",dolby->ac4CodecSettings.codecSettings.ac4.drcMode);
        printf("\t DRC SCALEHI = %d\n",dolby->ac4CodecSettings.codecSettings.ac4.drcScaleHi);
        printf("\t DRC SCALELOW = %d\n",dolby->ac4CodecSettings.codecSettings.ac4.drcScaleLow);
        printf("\t DRC SCALEHI DOWNMIX = %d\n",dolby->ac4CodecSettings.codecSettings.ac4.drcScaleHiDownmix);
        printf("\t DRC SCALELOW DOWNMIX = %d\n",dolby->ac4CodecSettings.codecSettings.ac4.drcScaleLowDownmix);
        printf("\t DOWNMIX MODE = %d\n", dolby->ac4CodecSettings.codecSettings.ac4.drcModeDownmix);
        printf("\t DECODE MODE = %d\n",dolby->ac4DecodeMode);
        printf("\t PROGRAM SEL = %d\n", dolby->ac4CodecSettings.codecSettings.ac4.programSelection );
        printf("\t ASSOC MIXING = %d\n", dolby->ac4CodecSettings.codecSettings.ac4.enableAssociateMixing );
        printf("\t DE AMOUNT = %d\n",dolby->ac4CodecSettings.codecSettings.ac4.dialogEnhancerAmount);
        printf("\t MIXING BALANCE = %d\n",dolby->ac4CodecSettings.codecSettings.ac4.programBalance);
        printf("\t SELECTION MODE = %d\n", dolby->ac4CodecSettings.codecSettings.ac4.selectionMode);
        printf("\t PREFERRED LANG1 = %s\n",dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[0].selection);
        printf("\t PREFERRED LANG2 = %s\n\n",dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[1].selection);
        printf("\t PREFERRED LANG1 ALT = %s\n",dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[0].selection);
        printf("\t PREFERRED LANG2 ALT = %s\n\n",dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[1].selection);
    }
    printf("\t MIXERBALANCE = %d\n", dolby->mixerBalance);

    printf("\t DAP_ENABLE = %d\n", dolby->enable_dap);
    printf("\t DV = %d\n", dolby->dv_enable);
    printf("\t DE = %d\n", dolby->dde_enable);
    printf("\t DEBOOST = %d\n", dolby->ddeDialogBoost);
    printf("\t DECUT = %d\n", dolby->ddeContentCut);
    printf("\t IEQ = %d\n", dolby->ieq_enable);
    printf("\t DEQBAND = %d\n", dolby->deqNumBands);
    printf("\t ENABLESURDEC = %d\n", dolby->dapEnaSurDecoder);
    printf("\t CERT_DDRE = %d\n", dolby->certificationMode_ddre);
    printf("\t CERT_DAP = %d\n", dolby->certificationMode_dap);
    printf("\t UDC_OUTMODE = %d\n", dolby->udcOutModeSpecial);
    printf("\t UDC_DECORRELATION = %d\n", dolby->udcDecorrelation);
    printf("\t MIDISABLE = %d\n", dolby->miDisable);
    printf("\t MISURCOMPDISABLE = %d\n", dolby->miSurCompDisable);
    printf("\t MIDDVDISABLE = %d\n", dolby->miDvDisable);
    printf("\t MIDDEDISABLE = %d\n", dolby->miDdeDisable);
    printf("\t MIDIEQDISABLE = %d\n", dolby->miDieqDisable);
    printf("\t MDCTBLENABLE = %d\n", dolby->mdctBLEnable);

    printf("\t MULTICH = %d\n", dolby->multiCh71 ? 8 : 6);
    printf("\t FIXEDENCODE = %d\n", dolby->fixedEncFormat);
    printf("\t DIRECT = %d\n", dolby->directMode);
    printf("\t SPDIF = %d\n", dolby->enableSpdif);
    printf("\t HDMI = %d\n", dolby->enableHdmi);
    printf("\t DAC = %d\n", dolby->enableDac);
    printf("\t DDRE = %d\n", dolby->enableDdre);
    printf("\t PCMPLAYBACK = %d\n", dolby->enablePlayback);
    printf("\t COMPRESSED = %d\n", dolby->enableCompressed);
    printf("\t SECONDARY = %d\n", dolby->enableDecode[SECONDARY_DECODE]);
    printf("\t SOUNDEFFECTS = %d\n", dolby->enableDecode[EFFECTS_DECODE]);
    printf("\t APPAUDIO = %d\n", dolby->enableDecode[APPAUDIO_DECODE]);
    printf("\t FADEDURATIONPRI = %d\n", dolby->fadeDuration[PRIMARY_DECODE]);
    printf("\t FADETYPEPRI = %d\n", dolby->fadeType[PRIMARY_DECODE]);
    printf("\t FADELEVELPRI = %d\n", dolby->fadeLevel[PRIMARY_DECODE]);
    printf("\t FADEDURATIONSEC = %d\n", dolby->fadeDuration[SECONDARY_DECODE]);
    printf("\t FADETYPESEC = %d\n", dolby->fadeType[SECONDARY_DECODE]);
    printf("\t FADELEVELSEC = %d\n", dolby->fadeLevel[SECONDARY_DECODE]);
    printf("\t FADEDURATIONSE = %d\n", dolby->fadeDuration[EFFECTS_DECODE]);
    printf("\t FADETYPESE = %d\n", dolby->fadeType[EFFECTS_DECODE]);
    printf("\t FADELEVELSE = %d\n", dolby->fadeLevel[EFFECTS_DECODE]);
    printf("\t FADEDURATIONAA = %d\n", dolby->fadeDuration[APPAUDIO_DECODE]);
    printf("\t FADETYPEAA = %d\n", dolby->fadeType[APPAUDIO_DECODE]);
    printf("\t FADELEVELAA = %d\n", dolby->fadeLevel[APPAUDIO_DECODE]);
    printf("\t FADEDURATIONOUT = %d\n", dolby->fadeDuration[OUTPUT_FADE_IDX]);
    printf("\t FADETYPEOUT = %d\n", dolby->fadeType[OUTPUT_FADE_IDX]);
    printf("\t FADELEVELOUT = %d\n", dolby->fadeLevel[OUTPUT_FADE_IDX]);

    printf("\t ATMOS = %d\n", dolby->enableAtmos);
    printf("\t COMPRESSION = %d\n", dolby->compression);
    printf("\t TARGET_SYNC_DISABLED = %d\n", dolby->targetSyncDisabled);
    printf("\t COMPLETE_FIRST_FRAME = %d\n", dolby->completeFirstFrame);
    /*printf("\n(1)OUTPUTMODE = %d\n(2)OUTPUTLFECH_MODE = %d\n(3)DUALMONO_MODE = %d\n(4)DOWNMIX_MODE = %d\n(5)RFMODE = %d\n(6)DRCCUT = %d\n(7)DRCBOOST = %d\n(8)enableCompressed = %d\n(9)SECONDARY_SUBSTREAMID = %d\n(10)MIXERBALANCE = %d\n\n",decodeSettings.outputMode,decodeSettings.outputLfeMode,decodeSettings.dualMonoMode,(codecSettings.codecSettings.ac3Plus.stereoDownmixMode),codecSettings.codecSettings.ac3Plus.drcMode,codecSettings.codecSettings.ac3Plus.cut,codecSettings.codecSettings.ac3Plus.boost,dolby->enableCompressed,codecSettings.codecSettings.ac3Plus.substreamId, dolby->mixerBalance);*/

    printf("\n\n----------------------------------------------------------------------------------------------- \n\n");
    printf("\n\n To change the value of any config parameters, type the command as 'CONFIG PARAMETER NAME= VALUE' \n Type 'quit' to terminate the application. \n\n\n");
    return;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------Command line arguments parsing---------------------------------------------------------------------------------------------*/

int dolby_digital_plus_cmdline_parse(int argc, const char *argv[], struct util_opts_t *opts, struct dolby_digital_plus_command_args *dolby)
{
    int i;
    unsigned audioPid = INVALID_PID, secondaryAudioPid = INVALID_PID;
    NEXUS_AudioCodec audioCodec = NEXUS_AudioCodec_eMax;
    NEXUS_AudioCodec secondaryAudioCodec = NEXUS_AudioCodec_eMax;
    NEXUS_TransportType transportType = NEXUS_TransportType_eMax;

    dolby->loopback = 0;

    opts[PRIMARY_DECODE].common.transportType = NEXUS_TransportType_eMax;

    opts[PRIMARY_DECODE].common.audioCodec = NEXUS_AudioCodec_eMax;
    opts[PRIMARY_DECODE].common.contentMode = NEXUS_VideoWindowContentMode_eFull;
    opts[PRIMARY_DECODE].common.compressedAudio = false;
    opts[PRIMARY_DECODE].common.decodedAudio = true;
    opts[PRIMARY_DECODE].common.probe = true;
#if NEXUS_DTV_PLATFORM
    opts[PRIMARY_DECODE].common.displayType = NEXUS_DisplayType_eLvds;
    opts[PRIMARY_DECODE].common.displayFormat = NEXUS_VideoFormat_eCustom0;
#else
    opts[PRIMARY_DECODE].common.useCompositeOutput = true;
    opts[PRIMARY_DECODE].common.useComponentOutput = true;
    opts[PRIMARY_DECODE].common.displayFormat = NEXUS_VideoFormat_eNtsc;
    opts[PRIMARY_DECODE].common.displayType = NEXUS_DisplayType_eAuto;
#endif
    opts[PRIMARY_DECODE].stcChannelMaster = NEXUS_StcChannelAutoModeBehavior_eVideoMaster;
    opts[PRIMARY_DECODE].common.tsTimestampType = NEXUS_TransportTimestampType_eNone;
    opts[PRIMARY_DECODE].beginningOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    opts[PRIMARY_DECODE].endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    opts[PRIMARY_DECODE].common.playpumpTimestampReordering = true;
    opts[PRIMARY_DECODE].customFileIo = false;
    opts[PRIMARY_DECODE].playbackMonitor = false;
    opts[PRIMARY_DECODE].common.videoCodec = NEXUS_VideoCodec_eUnknown;

    for ( i = PRIMARY_DECODE; i < NUM_DECODES; i++ )
    {
        dolby->decodeAttributes[i].pid = INVALID_PID;
        dolby->decodeAttributes[i].codec = NEXUS_AudioCodec_eMax;
    }

    for ( i = 1; i < argc; i++ )
    {
        if ( !strcmp(argv[i], "-probe") )
        {
            /* Deprecated, ignore */
        }
        else if ( !strcmp(argv[i], "-config") )
        {
            dolby->config_file = argv[++i];
            printf("\n You entered the config file name :%s\n", dolby->config_file);
        }
        else if ( !strcmp(argv[i], "-loopback") )
        {
            dolby->loopback = 1;
            printf("\n You have enabled the loopback option\n");
        }
        else if ( !strcmp(argv[i], "-pcm") )
        {
            dolby->pcm_stream = argv[++i];
        }
        else if ( !strcmp(argv[i], "-primary") )
        {
            if ( strlen(argv[i+1]) > MAX_FILENAME_LEN )
            {
                printf("Increase MAX_FILENAME_LEN to be > %d\n", (int)strlen(argv[i+1]));
                return -1;
            }
            strcpy(dolby->decodeAttributes[PRIMARY_DECODE].filename, argv[i+1]);
            dolby->primary_stream = dolby->decodeAttributes[PRIMARY_DECODE].filename;
            ++i;
        }
        else if ( !strcmp(argv[i], "-secondary") )
        {
            if ( strlen(argv[i+1]) > MAX_FILENAME_LEN )
            {
                printf("Increase MAX_FILENAME_LEN to be > %d\n", (int)strlen(argv[i+1]));
                return -1;
            }
            strcpy(dolby->decodeAttributes[SECONDARY_DECODE].filename, argv[i+1]);
            dolby->secondary_stream = dolby->decodeAttributes[SECONDARY_DECODE].filename;
            ++i;
        }
        else if ( !strcmp(argv[i], "-sound_effects") )
        {
            if ( strlen(argv[i+1]) > MAX_FILENAME_LEN )
            {
                printf("Increase MAX_FILENAME_LEN to be > %d\n", (int)strlen(argv[i+1]));
                return -1;
            }
            strcpy(dolby->decodeAttributes[EFFECTS_DECODE].filename, argv[i+1]);
            dolby->sound_effects_stream = dolby->decodeAttributes[EFFECTS_DECODE].filename;
            ++i;
        }
        else if ( !strcmp(argv[i], "-app_audio") )
        {
            if ( strlen(argv[i+1]) > MAX_FILENAME_LEN )
            {
                printf("Increase MAX_FILENAME_LEN to be > %d\n", (int)strlen(argv[i+1]));
                return -1;
            }
            strcpy(dolby->decodeAttributes[APPAUDIO_DECODE].filename, argv[i+1]);
            dolby->app_audio_stream = dolby->decodeAttributes[APPAUDIO_DECODE].filename;
            ++i;
        }
        else if ( !strcmp(argv[i], "-audio") && i + 1 < argc )
        {
            audioPid = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-secondary_audio") && i + 1 < argc )
        {
            secondaryAudioPid = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-audio_codec") && i + 1 < argc )
        {
            audioCodec = lookup(g_audioCodecStrs, argv[++i]);
        }
        else if ( !strcmp(argv[i], "-secondary_audio_codec") && i + 1 < argc )
        {
            secondaryAudioCodec = lookup(g_audioCodecStrs, argv[++i]);
        }
        else if ( !strcmp(argv[i], "-pcr") && i + 1 < argc )
        {
            opts[PRIMARY_DECODE].common.pcrPid = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-external_pcm") && i + 1 < argc )
        {
            dolby->externalPcm = strtoul(argv[++i], NULL, 0) ? true : false;
            dolby->externalPcmSpecified = true;
        }
        else if ( !strcmp(argv[i], "-mpeg_type") && i + 1 < argc )
        {
            transportType = lookup(g_transportTypeStrs, argv[++i]);
        }
        else if ( !strcmp(argv[i], "-target_sync_disabled") )
        {
            dolby->targetSyncDisabled = true;
        }
        else if ( !strcmp(argv[i], "-complete_first_frame") )
        {
            dolby->completeFirstFrame = true;
        }
        else if ( !strcmp(argv[i], "-no_decoded_video") )
        {
            dolby->enableVideo = false;
        }
        else if ( !strcmp(argv[i], "-dual_main_audio") )
        {
            dolby->dualMain = true;
        }
        else if ( !strcmp(argv[i], "-alt_stereo") )
        {
            dolby->altStereo = true;
        }
        else
        {
            printf("unknown param %s\n", argv[i]);
            return -1;
        }
    }

    if ( dolby->secondary_stream == NULL && secondaryAudioPid != INVALID_PID )
    {
        dolby->secondary_stream = dolby->primary_stream;
    }

    /* replicate common settings to other decodes */
    for ( i = PRIMARY_DECODE+1; i < NUM_DECODES; i++ )
    {
        BKNI_Memcpy(&opts[i], &opts[PRIMARY_DECODE], sizeof(struct util_opts_t) );
    }

    if ( dolby->primary_stream )
    {
        opts[PRIMARY_DECODE].filename = dolby->primary_stream;
        dolby->decodeAttributes[PRIMARY_DECODE].pid = opts[PRIMARY_DECODE].common.audioPid = audioPid;
        dolby->decodeAttributes[PRIMARY_DECODE].codec = opts[PRIMARY_DECODE].common.audioCodec = audioCodec;
        opts[PRIMARY_DECODE].common.transportType = transportType;
        printf("\n You have selected the primary stream : %s, pid %u, codec %d \n\n", dolby->primary_stream, audioPid, audioCodec);
    }

    if ( secondaryAudioCodec == NEXUS_AudioCodec_eMax )
    {
        secondaryAudioCodec = audioCodec;
    }

    if ( dolby->secondary_stream )
    {
        /* if secondary stream enabled, make sure we either have a pid, or a separate file */
        if ( (dolby->primary_stream && strcmp(dolby->secondary_stream, dolby->primary_stream) == 0 && secondaryAudioPid != INVALID_PID) ||
             (dolby->primary_stream && strcmp(dolby->secondary_stream, dolby->primary_stream) != 0) || dolby->dualMain )
        {
            opts[SECONDARY_DECODE].filename = dolby->secondary_stream;
            opts[SECONDARY_DECODE].common.probe = true;
            dolby->decodeAttributes[SECONDARY_DECODE].pid = opts[SECONDARY_DECODE].common.audioPid = secondaryAudioPid;
            dolby->decodeAttributes[SECONDARY_DECODE].codec = opts[SECONDARY_DECODE].common.audioCodec = secondaryAudioCodec;
            opts[SECONDARY_DECODE].common.transportType = transportType;
            printf("\n You have selected the secondary stream : %s, pid %u, codec %d \n\n", dolby->secondary_stream, secondaryAudioPid, secondaryAudioCodec);
        }
    }

    if ( dolby->sound_effects_stream )
    {
        opts[EFFECTS_DECODE].filename = dolby->sound_effects_stream;
        printf("\n You have selected sound effects stream : %s \n\n", dolby->sound_effects_stream);
        opts[EFFECTS_DECODE].common.probe = true;
        opts[EFFECTS_DECODE].common.audioCodec = NEXUS_AudioCodec_eMax;
        opts[EFFECTS_DECODE].common.transportType = NEXUS_TransportType_eMax;
        opts[EFFECTS_DECODE].common.audioPid = INVALID_PID;
    }

    if ( dolby->app_audio_stream )
    {
        opts[APPAUDIO_DECODE].filename = dolby->app_audio_stream;
        printf("\n You have selected Application Audio stream : %s \n\n", dolby->app_audio_stream);
        opts[APPAUDIO_DECODE].common.probe = true;
        opts[APPAUDIO_DECODE].common.audioCodec = NEXUS_AudioCodec_eMax;
        opts[APPAUDIO_DECODE].common.transportType = NEXUS_TransportType_eMax;
        opts[APPAUDIO_DECODE].common.audioPid = INVALID_PID;
    }

    opts[PRIMARY_DECODE].common.displayType = NEXUS_DisplayType_eAuto;

    /* this allows the user to set: "-mpeg_type es -video_type mpeg" and forget the "-video 1" option */
    if ( opts[PRIMARY_DECODE].common.transportType == NEXUS_TransportType_eEs && !opts[PRIMARY_DECODE].common.videoPid && !opts[PRIMARY_DECODE].common.audioPid )
    {
        if ( g_isVideoEs )
        {
            opts[PRIMARY_DECODE].common.videoPid = 1;
        }
        else
        {
            opts[PRIMARY_DECODE].common.audioPid = 1;
        }
    }

    return 0;
}

static void translate_args_to_decoder_settings(unsigned idx, NEXUS_AudioDecoderHandle *decodes, struct dolby_digital_plus_command_args *dolby, NEXUS_AudioDecoderSettings * decoderSettings)
{
    BSTD_UNUSED(idx);
    BSTD_UNUSED(decodes);
    decoderSettings->outputMode = dolby->decodeSettings.outputMode;
    decoderSettings->outputLfeMode = dolby->decodeSettings.outputLfeMode;
    decoderSettings->dualMonoMode = dolby->decodeSettings.dualMonoMode;
}

void apply_mixer_settings(NEXUS_AudioMixerSettings *mixerSettings, struct dolby_digital_plus_command_args *dolby)
{
    if ( dolby->enable_dap )
    {
        unsigned i;
        mixerSettings->dolby.enablePostProcessing = true;
        mixerSettings->dolby.volumeLimiter.enableVolumeLimiting = dolby->dv_enable;
        mixerSettings->dolby.volumeLimiter.enableIntelligentLoudness = dolby->dvOverrideInbandVol ? false : true;
        mixerSettings->dolby.volumeLimiter.volumeLimiterAmount = dolby->dvContentType;
        mixerSettings->dolby.dialogEnhancer.enableDialogEnhancer = dolby->dde_enable;
        mixerSettings->dolby.dialogEnhancer.dialogEnhancerLevel = dolby->ddeDialogBoost;
        mixerSettings->dolby.dialogEnhancer.contentSuppressionLevel = dolby->ddeContentCut;
        /* if they have not specified num_bands ieq can be enabled and it will go with default freq and gains*/
        mixerSettings->dolby.intelligentEqualizer.enabled = dolby->ieq_enable;
        mixerSettings->dolby.intelligentEqualizer.numBands = dolby->deqNumBands;
        for ( i = 0; i < mixerSettings->dolby.intelligentEqualizer.numBands; i++ )
        {
            mixerSettings->dolby.intelligentEqualizer.band[i].frequency = dolby->deqFrequency[i];
            mixerSettings->dolby.intelligentEqualizer.band[i].gain = dolby->deqGain[i];
        }
    }
    else
    {
        mixerSettings->dolby.enablePostProcessing = false;
    }

    mixerSettings->dolby.certificationMode = 0;
    mixerSettings->dolby.certificationMode |= dolby->certificationMode_dap ? DOLBY_DAP_CERT_MODE : 0;
    if ( (mixerSettings->dolby.certificationMode & DOLBY_DAP_CERT_MODE) )
    {
        mixerSettings->dolby.certificationMode |= dolby->miDisable ? DOLBY_DAP_CERT_DISABLE_MI : 0;
        mixerSettings->dolby.certificationMode |= dolby->miSurCompDisable ? DOLBY_DAP_CERT_DISABLE_MI_SURROUNDCOMP : 0;
        mixerSettings->dolby.certificationMode |= dolby->miDvDisable ? DOLBY_DAP_CERT_DISABLE_MI_VOLUMELIMITER : 0;
        mixerSettings->dolby.certificationMode |= dolby->miDdeDisable ? DOLBY_DAP_CERT_DISABLE_MI_DIALOGENHANCER : 0;
        mixerSettings->dolby.certificationMode |= dolby->miDieqDisable ? DOLBY_DAP_CERT_DISABLE_MI_INTELLIGENTEQ : 0;
        mixerSettings->dolby.certificationMode |= (dolby->deqAmount << DOLBY_DAP_CERT_EQAMOUNT_SHIFT) & DOLBY_DAP_CERT_EQAMOUNT;
        mixerSettings->dolby.certificationMode |= dolby->dapEnaSurDecoder ? DOLBY_DAP_CERT_ENABLE_SURROUND_DECODER : 0;
    }
    printf("\nMS12 DAP Mixer settings:\n");
    printf("------------------------------------\n");
    printf("  certificationMode     %d\n", dolby->certificationMode_dap);
    if ( dolby->certificationMode_dap )
    {
        printf("    disableMi           %d\n", dolby->miDisable);
        printf("    disableMiSurComp    %d\n", dolby->miSurCompDisable);
        printf("    disableMiDV         %d\n", dolby->miDvDisable);
        printf("    disableMiDDE        %d\n", dolby->miDdeDisable);
        printf("    disableMiDieq       %d\n", dolby->miDieqDisable);
    }
    printf("  enableProcessing  %d\n", dolby->enable_dap);
    if ( dolby->enable_dap )
    {
        unsigned i;
        printf("    enableDV            %d\n", dolby->dv_enable);
        if ( dolby->dv_enable )
        {
            printf("      DvContentType     %d\n", dolby->dvContentType);
            printf("      DvOverrideInband  %d\n", dolby->dvOverrideInbandVol);
        }
        printf("    enableDE            %d\n", dolby->dde_enable);
        if ( dolby->dde_enable )
        {
            printf("      dialogBoost       %d\n", dolby->ddeDialogBoost);
            printf("      contentCut        %d\n", dolby->ddeContentCut);
        }
        printf("    enableDIEQ          %d\n", (dolby->ieq_enable));
        if ( dolby->ieq_enable > 0 )
        {
            printf("      eqAmount          %d\n", dolby->deqAmount);
            printf("      numEqBands        %d\n", dolby->deqNumBands);
            for ( i = 0; i < dolby->deqNumBands; i++ )
            {
                printf("      Band[%d]:\n", i);
                printf("        eqFrequency     %dHz\n", dolby->deqFrequency[i]);
                printf("        eqGain          %d\n", dolby->deqGain[i]);
            }
        }
    }
    printf("------------------------------------\n");
}
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

static void translate_args_to_codec_settings(unsigned idx, struct dolby_digital_plus_command_args *dolby, struct app_resources * recs, NEXUS_AudioDecoderCodecSettings *codecSettings)
{
    if ( codecSettings->codec == NEXUS_AudioCodec_eAc3Plus || codecSettings->codec == NEXUS_AudioCodec_eAc3 )
    {
        codecSettings->codecSettings.ac3Plus.certificationMode = 0;

        /* Set up Codec Specific params */
        if ( idx == PRIMARY_DECODE )
        {
            codecSettings->codecSettings.ac3Plus.substreamId = 0;
        }
        else if ( idx == SECONDARY_DECODE )
        {
            codecSettings->codecSettings.ac3Plus.substreamId = dolby->secondary_substreamId;
        }

        /* Only Enable Atmos processing if we don't have a Secondary decode */
        if ( idx == 0 && (!recs->initialized[SECONDARY_DECODE] || dolby->dualMain) )
        {
            codecSettings->codecSettings.ac3Plus.enableAtmosProcessing = dolby->enableAtmos;
        }
        else
        {
            codecSettings->codecSettings.ac3Plus.enableAtmosProcessing = false;
        }

        codecSettings->codecSettings.ac3Plus.stereoDownmixMode = dolby->ac3CodecSettings.codecSettings.ac3Plus.stereoDownmixMode;
        codecSettings->codecSettings.ac3Plus.drcMode = dolby->ac3CodecSettings.codecSettings.ac3Plus.drcMode;
        codecSettings->codecSettings.ac3Plus.drcModeDownmix = dolby->ac3CodecSettings.codecSettings.ac3Plus.drcModeDownmix;
        codecSettings->codecSettings.ac3Plus.cut = dolby->ac3CodecSettings.codecSettings.ac3Plus.cut;
        codecSettings->codecSettings.ac3Plus.boost = dolby->ac3CodecSettings.codecSettings.ac3Plus.boost;
        codecSettings->codecSettings.ac3Plus.cutDownmix = dolby->ac3CodecSettings.codecSettings.ac3Plus.cutDownmix;
        codecSettings->codecSettings.ac3Plus.boostDownmix = dolby->ac3CodecSettings.codecSettings.ac3Plus.boostDownmix;
        if ( dolby->udcDecorrelation )
        {
            codecSettings->codecSettings.ac3Plus.certificationMode |= DOLBY_UDC_DECORRELATION;
        }
        if ( dolby->mdctBLEnable )
        {
            codecSettings->codecSettings.ac3Plus.certificationMode |= DOLBY_UDC_MDCT_BANDLIMIT;
        }
        if ( dolby->udcOutModeSpecial )
        {
            if ( dolby->udcOutModeSpecial == -1 )
            {
                dolby->udcOutModeSpecial = 31;
            }
            codecSettings->codecSettings.ac3Plus.certificationMode |= (DOLBY_UDC_OUTPUTMODE_CUSTOM_MASK & (dolby->udcOutModeSpecial << DOLBY_UDC_OUTPUTMODE_CUSTOM_SHIFT));
        }
    }
    else if ( codecSettings->codec == NEXUS_AudioCodec_eAc4 )
    {
        codecSettings->codecSettings.ac4.certificationMode = 0;
        if ( dolby->ac4DecodeMode )
        {
            codecSettings->codecSettings.ac4.certificationMode |= (dolby->ac4DecodeMode << DOLBY_AC4_DECODEMODE_SHIFT) & DOLBY_AC4_DECODEMODE_MASK;
        }
        codecSettings->codecSettings.ac4.stereoMode = dolby->ac4CodecSettings.codecSettings.ac4.stereoMode;
        codecSettings->codecSettings.ac4.drcMode = dolby->ac4CodecSettings.codecSettings.ac4.drcMode;
        codecSettings->codecSettings.ac4.drcModeDownmix = dolby->ac4CodecSettings.codecSettings.ac4.drcModeDownmix;
        codecSettings->codecSettings.ac4.drcScaleHi = dolby->ac4CodecSettings.codecSettings.ac4.drcScaleHi;
        codecSettings->codecSettings.ac4.drcScaleLow = dolby->ac4CodecSettings.codecSettings.ac4.drcScaleLow;
        codecSettings->codecSettings.ac4.drcScaleHiDownmix = dolby->ac4CodecSettings.codecSettings.ac4.drcScaleHiDownmix;
        codecSettings->codecSettings.ac4.drcScaleLowDownmix = dolby->ac4CodecSettings.codecSettings.ac4.drcScaleLowDownmix;
        codecSettings->codecSettings.ac4.programSelection = dolby->ac4CodecSettings.codecSettings.ac4.programSelection;
        /* Set up Codec type Specific params */
        if ( idx == PRIMARY_DECODE )
        {
            /* certification decode mode overrides programSelection */
            if ( dolby->ac4DecodeMode != 0 )
            {
                codecSettings->codecSettings.ac4.programSelection = 0;
                if ( (dolby->ac4DecodeMode-1) != AC4_MODE_SS_DD_SI )
                {
                    codecSettings->codecSettings.ac4.programSelection = 1;
                }
            }
            dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationIndex = dolby->ac4PresIdxMain;
            dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].presentationIndex = dolby->ac4PresIdxAlt;
            strcpy(dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationId, dolby->ac4PresIdMain);
            strcpy(dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].presentationId, dolby->ac4PresIdAlt);
        }
        else if ( idx == SECONDARY_DECODE )
        {
            /* certification decode mode overrides programSelection */
            if ( dolby->ac4DecodeMode != 0 )
            {
                codecSettings->codecSettings.ac4.programSelection = 2;
                if( (dolby->ac4DecodeMode-1) == AC4_MODE_DS_DD)
                {
                    codecSettings->codecSettings.ac4.programSelection  = 0;
                }
            }
            dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationIndex = dolby->ac4PresIdxAssoc;
            strcpy(dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationId, dolby->ac4PresIdAssoc);
        }
        codecSettings->codecSettings.ac4.programBalance = dolby->ac4CodecSettings.codecSettings.ac4.programBalance;
        codecSettings->codecSettings.ac4.dialogEnhancerAmount = dolby->ac4CodecSettings.codecSettings.ac4.dialogEnhancerAmount;
        codecSettings->codecSettings.ac4.selectionMode = dolby->ac4CodecSettings.codecSettings.ac4.selectionMode;
        codecSettings->codecSettings.ac4.enableAssociateMixing = dolby->ac4CodecSettings.codecSettings.ac4.enableAssociateMixing;
        codecSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationIndex = dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationIndex;
        memcpy(codecSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationId, dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationId, sizeof(char) * NEXUS_AUDIO_AC4_PRESENTATION_ID_LENGTH);
        codecSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferLanguageOverAssociateType = dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferLanguageOverAssociateType;
        codecSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferredAssociateType = dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].preferredAssociateType;
        memcpy(codecSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[0].selection, dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[0].selection, sizeof(char) * NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH);
        memcpy(codecSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[1].selection, dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].languagePreference[1].selection, sizeof(char) * NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH);
        codecSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].presentationIndex = dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].presentationIndex;
        memcpy(codecSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].presentationId, dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].presentationId, sizeof(char) * NEXUS_AUDIO_AC4_PRESENTATION_ID_LENGTH);
        codecSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferLanguageOverAssociateType = dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferLanguageOverAssociateType;
        codecSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferredAssociateType = dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].preferredAssociateType;
        memcpy(codecSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[0].selection, dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[0].selection, sizeof(char) * NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH);
        memcpy(codecSettings->codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[1].selection, dolby->ac4CodecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eAlternate].languagePreference[1].selection, sizeof(char) * NEXUS_AUDIO_AC4_LANGUAGE_NAME_LENGTH);
    }
    else if ( codecSettings->codec == NEXUS_AudioCodec_eAacAdts || codecSettings->codec == NEXUS_AudioCodec_eAacLoas ||
              codecSettings->codec == NEXUS_AudioCodec_eAacPlusAdts || codecSettings->codec == NEXUS_AudioCodec_eAacPlusLoas )
    {
        codecSettings->codecSettings.aacPlus.downmixMode = dolby->aacCodecSettings.codecSettings.aacPlus.downmixMode;
        codecSettings->codecSettings.aacPlus.cut = dolby->aacCodecSettings.codecSettings.aacPlus.cut;
        codecSettings->codecSettings.aacPlus.boost = dolby->aacCodecSettings.codecSettings.aacPlus.boost;
        codecSettings->codecSettings.aacPlus.drcMode = dolby->aacCodecSettings.codecSettings.aacPlus.drcMode;
        codecSettings->codecSettings.aacPlus.drcDefaultLevel = dolby->aacCodecSettings.codecSettings.aacPlus.drcDefaultLevel;
    }
}

int decode_path_update_settings( int idx, struct dolby_digital_plus_command_args *dolby, struct app_resources * recs )
{
    if ( recs->audioDecoders[idx] )
    {
        NEXUS_AudioDecoderSettings decoderSettings;
        NEXUS_AudioDecoderCodecSettings codecSettings;

        NEXUS_AudioDecoder_GetSettings(recs->audioDecoders[idx], &decoderSettings);
        translate_args_to_decoder_settings(idx, recs->audioDecoders, dolby, &decoderSettings);
        NEXUS_AudioDecoder_SetSettings(recs->audioDecoders[idx], &decoderSettings);

        /* store start time settings (to be applied later) */
        recs->audioProgram[idx].targetSyncEnabled = !dolby->targetSyncDisabled;
        recs->audioProgram[idx].forceCompleteFirstFrame = dolby->completeFirstFrame;

        NEXUS_AudioDecoder_GetCodecSettings(recs->audioDecoders[idx], recs->audioProgram[idx].codec, &codecSettings);
        translate_args_to_codec_settings(idx, dolby, recs, &codecSettings);
        NEXUS_AudioDecoder_SetCodecSettings(recs->audioDecoders[idx], &codecSettings);

        if ( idx == PRIMARY_DECODE )
        {
            NEXUS_AudioMixerSettings mixerSettings;
            NEXUS_AudioMixer_GetSettings(recs->mixer, &mixerSettings);
            mixerSettings.dolby.multiStreamBalance = dolby->mixerBalance;
            apply_mixer_settings(&mixerSettings, dolby);
            if ( recs->audioProgram[PRIMARY_DECODE].codec == NEXUS_AudioCodec_eAc4 )
            {
                mixerSettings.dolby.enablePostProcessing = false;
            }
            NEXUS_AudioMixer_SetSettings(recs->mixer, &mixerSettings);
        }
    }

    return 0;
}

int decode_path_shutdown( int idx, struct app_resources * recs, bool complete )
{
    if ( recs->playback[idx] )
    {
        NEXUS_Playback_CloseAllPidChannels(recs->playback[idx]);
    }
    if ( recs->file[idx] )
    {
        NEXUS_FilePlay_Close(recs->file[idx]);
        recs->file[idx] = NULL;
    }
    if ( complete )
    {
        if ( recs->playback[idx] )
        {
            printf("destroy playback[%d] %p\n", idx, (void *)recs->playback[idx]);
            NEXUS_Playback_Destroy(recs->playback[idx]);
            recs->playback[idx] = NULL;
        }
        if ( recs->playpump[idx] )
        {
            printf("destroy playpump[%d] %p\n", idx, (void *)recs->playpump[idx]);
            NEXUS_Playpump_Close(recs->playpump[idx]);
            recs->playpump[idx] = NULL;
        }
    }

    recs->initialized[idx] = false;

    return 0;
}

int decode_path_initialize( int idx, struct util_opts_t *opts, struct dolby_digital_plus_command_args *dolby, struct app_resources * recs, bool init )
{
    BERR_Code rc;
    NEXUS_PlaybackHandle pb;
    NEXUS_PlaypumpHandle pp;
    NEXUS_StcChannelHandle stc;
    bool requiresPlaypump = true;
    NEXUS_AudioDecoderOpenSettings decoderOpenSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_FilePlayHandle customFile;
    NEXUS_FilePlayHandle stickyFile;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;

    NEXUS_AudioCodec audioCodec;
    unsigned audioPid, pcrPid;
    NEXUS_TransportType transportType;

    decode_path_shutdown(idx, recs, false);

    recs->initialized[idx] = false;

    if ( idx == PRIMARY_DECODE && init )
    {
        parse_config_file(dolby, true);
    }

    if ( !recs->audioDecoders[idx] )
    {
        NEXUS_AudioDecoder_GetDefaultOpenSettings(&decoderOpenSettings);
        decoderOpenSettings.multichannelFormat = dolby->multiCh71 ? NEXUS_AudioMultichannelFormat_e7_1 : NEXUS_AudioMultichannelFormat_e5_1;
        recs->audioDecoders[idx] = NEXUS_AudioDecoder_Open(idx, &decoderOpenSettings);
        BDBG_ASSERT(recs->audioDecoders[idx]);
    }

    /* explicitly disable secondary when requested */
    if ( idx == SECONDARY_DECODE && !dolby->enableDecode[SECONDARY_DECODE] )
    {
        opts[idx].filename = NULL;
    }
    if ( idx == EFFECTS_DECODE && !dolby->enableDecode[EFFECTS_DECODE] )
    {
        opts[idx].filename = NULL;
    }
    if ( idx == APPAUDIO_DECODE && !dolby->enableDecode[APPAUDIO_DECODE] )
    {
        opts[idx].filename = NULL;
    }

    if ( opts[idx].filename && strlen(opts[idx].filename) <= 0 )
    {
        opts[idx].filename = NULL;
    }

    if ( !opts[idx].filename )
    {
        return 0;
    }
    else if ( idx == SECONDARY_DECODE &&
              strcmp(opts[SECONDARY_DECODE].filename, opts[PRIMARY_DECODE].filename) == 0 &&
              !dolby->dualMain )
    {
        printf("SECONDARY is sharing PLAYBACK/PLAYPUMP with PRIMARY\n");
        requiresPlaypump = false;
    }

    if ( requiresPlaypump )
    {
        /* Bring up all modules for a platform in a default configuration for this platform */
        if ( !recs->playpump[idx] )
        {
            recs->playpump[idx] = NEXUS_Playpump_Open(idx, NULL);
            BDBG_ASSERT(recs->playpump[idx]);
        }
        pp = recs->playpump[idx];
        if ( !recs->playback[idx] )
        {
            recs->playback[idx] = NEXUS_Playback_Create();
            BDBG_ASSERT(recs->playback[idx]);
        }
        pb = recs->playback[idx];
        if ( !recs->stcChannel[idx] )
        {
            NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
            stcSettings.timebase = NEXUS_Timebase_e0;
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;
            stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eAudioMaster;
            recs->stcChannel[idx] = NEXUS_StcChannel_Open(idx, &stcSettings);
            BDBG_ASSERT(recs->stcChannel[idx]);
        }
        stc = recs->stcChannel[idx];
    }
    else
    {
        pp = recs->playpump[PRIMARY_DECODE];
        pb = recs->playback[PRIMARY_DECODE];
        stc = recs->stcChannel[PRIMARY_DECODE];
    }

    if ( !pp || !pb || !stc )
    {
        printf("ERROR: invalid configuration during initialize(%d), pp=%p, pb=%p, stc=%p\n", idx, (void*)pp, (void*)pb, (void*)stc);
        BDBG_ASSERT(pp && pb && stc);
    }

    if ( !recs->ddre )
    {
        NEXUS_DolbyDigitalReencodeSettings ddreSettings;
        NEXUS_DolbyDigitalReencode_GetDefaultSettings(&ddreSettings);
        ddreSettings.fixedEncoderFormat = dolby->fixedEncFormat;
        if ( dolby->multiCh71 )
        {
            ddreSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e7_1;
        }
        else
        {
            ddreSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;
        }
        recs->ddre = NEXUS_DolbyDigitalReencode_Open(&ddreSettings);
        BDBG_ASSERT(recs->ddre);
    }

    if ( dolby->loopback == 1 )
    {
        opts[idx].endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    }
    else
    {
        opts[idx].endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
    }

    /* save what we got from command line before we probe */
    printf("\n From command line - Stream[%d] audioCodec %d, pid %d, transportType %d\n", idx, opts[idx].common.audioCodec, opts[idx].common.audioPid, opts[idx].common.transportType);
    audioCodec = opts[idx].common.audioCodec;
    audioPid = opts[idx].common.audioPid;
    transportType = opts[idx].common.transportType;
    pcrPid = opts[idx].common.pcrPid;

    if ( cmdline_probe(&opts[idx].common, opts[idx].filename, &opts[idx].indexname) )
    {
        if (transportType != NEXUS_TransportType_eEs)
        {
            return 1;
        }
    }

    printf("\nPROBE of '%s' found:\n\t transport type %d, audio_codec %d, pid %d, pcr %d\n",
           opts[idx].filename, opts[idx].common.transportType, opts[idx].common.audioCodec, opts[idx].common.audioPid, opts[idx].common.pcrPid);

    /* restore what we got from command line after probe */
    if ( audioCodec != NEXUS_AudioCodec_eMax && audioCodec != opts[idx].common.audioCodec )
    {
        printf("Overriding probed audio codec %d with command line specified codec %d\n", opts[idx].common.audioCodec, audioCodec);
        opts[idx].common.audioCodec = audioCodec;
    }
    if ( audioPid != INVALID_PID && audioPid != opts[idx].common.audioPid )
    {
        printf("Overriding probed audio pid %d with command line specified pid %d\n", opts[idx].common.audioPid, audioPid);
        opts[idx].common.audioPid = audioPid;
    }
    if ( transportType != NEXUS_TransportType_eMax && transportType != opts[idx].common.transportType )
    {
        printf("Overriding probed transport type %d with command line specified type %d\n", opts[idx].common.transportType, transportType);
        opts[idx].common.transportType = transportType;
    }
    if ( pcrPid != 0 && pcrPid != opts[idx].common.pcrPid )
    {
        printf("Overriding probed pcr pid %d with command line specified pid %d\n", opts[idx].common.pcrPid, pcrPid);
        opts[idx].common.pcrPid = pcrPid;
    }

    printf("\nFinal Configuration for Decode[%d]:\n\t transport type %d, audio_codec %d, pid %d, pcr %d\n",
           idx, opts[idx].common.transportType, opts[idx].common.audioCodec, opts[idx].common.audioPid, opts[idx].common.pcrPid);

    /* We have to parse again (silently) now that we have the audio codec from probe */
    if ( idx == PRIMARY_DECODE )
    {
        /* store the primary audio codec here */
        dolby->audioCodec = opts[idx].common.audioCodec;
        if ( init )
        {
            parse_config_file(dolby, false);
        }
    }

    if ( !opts[idx].filename )
    {
        return -1;
    }

    if ( (opts[idx].indexname && !strcmp(opts[idx].indexname, "same")) ||
         opts[idx].common.transportType == NEXUS_TransportType_eMkv ||
         opts[idx].common.transportType == NEXUS_TransportType_eMp4
        )
    {
        opts[idx].indexname = opts[idx].filename;
    }

    if ( requiresPlaypump )
    {
        recs->file[idx] = NEXUS_FilePlay_OpenPosix(opts[idx].filename, opts[idx].indexname);
        if ( !recs->file[idx] )
        {
            fprintf(stderr, "can't open files:%s %s\n", opts[idx].filename, opts[idx].indexname);
            return -1;
        }
        if ( opts[idx].customFileIo )
        {
            customFile = recs->file[idx] = FileIoCustom_Attach(recs->file[idx]);
            BDBG_ASSERT(recs->file[idx]);
        }
        if ( opts[idx].playbackMonitor )
        {
            stickyFile = recs->file[idx] = FileIoSticky_Attach(recs->file[idx]);
            BDBG_ASSERT(recs->file[idx]);
        }

        NEXUS_Playback_GetSettings(pb, &playbackSettings);
        playbackSettings.playpump = pp;
        playbackSettings.playpumpSettings.transportType = opts[idx].common.transportType;
        playbackSettings.playpumpSettings.timestamp.pacing = false;
        playbackSettings.playpumpSettings.timestamp.type = opts[idx].common.tsTimestampType;

        playbackSettings.stcChannel = stc;
        playbackSettings.stcTrick = opts[idx].stcTrick;
        playbackSettings.beginningOfStreamAction = opts[idx].beginningOfStreamAction;
        playbackSettings.endOfStreamAction = opts[idx].endOfStreamAction;
        playbackSettings.enableStreamProcessing = opts[idx].streamProcessing;
        rc = NEXUS_Playback_SetSettings(pb, &playbackSettings);
        BDBG_ASSERT(!rc);
    }

    if ( opts[idx].common.audioCodec != NEXUS_AudioCodec_eUnknown )
    {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.primary = recs->audioDecoders[idx];
        playbackPidSettings.pidSettings.pidTypeSettings.audio.codec = opts[idx].common.audioCodec;
        recs->audioPidChannel = NEXUS_Playback_OpenPidChannel(pb, opts[idx].common.audioPid, &playbackPidSettings);
    }

    if ( idx == PRIMARY_DECODE && opts[idx].common.videoCodec != NEXUS_VideoCodec_eUnknown && dolby->enableVideo )
    {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.decoder = recs->videoDecoder;
        playbackPidSettings.pidTypeSettings.video.codec = opts[idx].common.videoCodec;
        recs->videoPidChannel = NEXUS_Playback_OpenPidChannel(pb, opts[idx].common.videoPid, &playbackPidSettings);

        NEXUS_VideoDecoder_GetDefaultStartSettings(&(recs->videoProgram));
        recs->videoProgram.codec = opts[idx].common.videoCodec;
        recs->videoProgram.pidChannel = recs->videoPidChannel;
        recs->videoProgram.stcChannel = stc;
    }
    else if ( idx == PRIMARY_DECODE )
    {
        if (recs->videoDecoder)
        {
            NEXUS_VideoWindow_RemoveAllInputs(recs->window);
            NEXUS_VideoDecoder_Close(recs->videoDecoder);
            recs->videoDecoder = NULL;
        }
    }

    if ( opts[idx].common.pcrPid && opts[idx].common.pcrPid != opts[idx].common.videoPid && opts[idx].common.pcrPid != opts[idx].common.audioPid )
    {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
        recs->pcrPidChannel = NEXUS_Playback_OpenPidChannel(pb, opts[idx].common.pcrPid, &playbackPidSettings);
    }

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&(recs->audioProgram[idx]));

    recs->audioProgram[idx].targetSyncEnabled = !dolby->targetSyncDisabled;
    recs->audioProgram[idx].forceCompleteFirstFrame = dolby->completeFirstFrame;
    recs->audioProgram[idx].codec = opts[idx].common.audioCodec;
    recs->audioProgram[idx].pidChannel = recs->audioPidChannel;
    recs->audioProgram[idx].stcChannel = stc;
    if ( dolby->dualMain &&
         (idx == PRIMARY_DECODE || idx == SECONDARY_DECODE) )
    {
        recs->audioProgram[idx].mixingMode = NEXUS_AudioDecoderMixingMode_eStandalone;
    }

    if ( idx == SECONDARY_DECODE )
    {
        recs->audioProgram[idx].secondaryDecoder = true;
    }
    else if ( idx == EFFECTS_DECODE )
    {
        recs->audioProgram[idx].mixingMode = NEXUS_AudioDecoderMixingMode_eSoundEffects;
    }
    else if ( idx == APPAUDIO_DECODE )
    {
        recs->audioProgram[idx].mixingMode = NEXUS_AudioDecoderMixingMode_eApplicationAudio;
    }

    decode_path_update_settings(idx, dolby, recs);
    recs->initialized[idx] = true;

    return 0;
}

int decode_path_start( int idx, struct util_opts_t *opts, struct dolby_digital_plus_command_args *dolby, struct app_resources * recs )
{
    BERR_Code rc;

    if ( idx == SECONDARY_DECODE )
    {
        printf("enable %d, filename %s\n", dolby->enableDecode[idx], opts[idx].filename ? opts[idx].filename:"");
    }
    if ( dolby->enableDecode[idx] && opts[idx].filename )
    {
        if ( recs->videoDecoder && idx == PRIMARY_DECODE && !recs->started[idx] )
        {
            NEXUS_VideoDecoder_Start(recs->videoDecoder, &(recs->videoProgram));
        }

        if ( recs->audioDecoders[idx] && !recs->started[idx] )
        {
            bool connected;
            NEXUS_AudioMixerInputSettings inputSettings;
            decode_path_update_settings(idx, dolby, recs);

            NEXUS_AudioInput_IsConnectedToInput(NEXUS_AudioDecoder_GetConnector(recs->audioDecoders[idx], NEXUS_AudioConnectorType_eMultichannel),
                                                NEXUS_AudioMixer_GetConnector(recs->mixer), &connected);

            if ( connected )
            {
                /* Apply Initial Fade Settings */
                NEXUS_AudioMixer_GetInputSettings(recs->mixer,
                                                  NEXUS_AudioDecoder_GetConnector(recs->audioDecoders[idx], NEXUS_AudioConnectorType_eMultichannel),
                                                  &inputSettings);
                inputSettings.fade.level = dolby->fadeLevel[idx];
                inputSettings.fade.type = dolby->fadeType[idx];
                inputSettings.fade.duration = dolby->fadeDuration[idx];
                NEXUS_AudioMixer_SetInputSettings(recs->mixer,
                                                  NEXUS_AudioDecoder_GetConnector(recs->audioDecoders[idx], NEXUS_AudioConnectorType_eMultichannel),
                                                  &inputSettings);
            }

            if ( idx == PRIMARY_DECODE || idx == SECONDARY_DECODE )
            {
                NEXUS_DolbyDigitalReencodeSettings ddreSettings;
                NEXUS_AudioDecoderCodecSettings codecSettings;
                NEXUS_AudioMixerSettings mixerSettings;

                NEXUS_AudioMixer_GetSettings(recs->mixer, &mixerSettings);
                mixerSettings.mainDecodeFade.level = dolby->fadeLevel[OUTPUT_FADE_IDX];
                mixerSettings.mainDecodeFade.type = dolby->fadeType[OUTPUT_FADE_IDX];
                mixerSettings.mainDecodeFade.duration = dolby->fadeDuration[OUTPUT_FADE_IDX];
                NEXUS_AudioMixer_SetSettings(recs->mixer, &mixerSettings);

                NEXUS_AudioDecoder_GetCodecSettings(recs->audioDecoders[idx], recs->audioProgram[idx].codec, &codecSettings);
                print_settings(dolby->decodeSettings, codecSettings, dolby);

                /* configure DDRE */
                NEXUS_DolbyDigitalReencode_GetSettings(recs->ddre, &ddreSettings);

                ddreSettings.profile = dolby->compression;
                ddreSettings.dialogLevel = dolby->dialog_level;
                if ( dolby->externalPcmSpecified )
                {
                    ddreSettings.externalPcmMode = dolby->externalPcm;
                }
                else
                {
                    unsigned i;
                    ddreSettings.externalPcmMode = false;
                    for ( i = 0; i < 2 && !ddreSettings.externalPcmMode; i++ )
                    {
                        switch ( opts[i].common.audioCodec )
                        {
                        case NEXUS_AudioCodec_eMax:
                        case NEXUS_AudioCodec_eAc3:
                        case NEXUS_AudioCodec_eAc3Plus:
                        case NEXUS_AudioCodec_eAacAdts:
                        case NEXUS_AudioCodec_eAacLoas:
                        case NEXUS_AudioCodec_eAacPlusAdts:
                        case NEXUS_AudioCodec_eAacPlusLoas:
                        case NEXUS_AudioCodec_eAc4:
                            break;
                        default:
                            BDBG_WRN(("%s file is a non-dolby codec %d.  Forcing external PCM mode.", i == 0 ? "Primary" : "Secondary", recs->audioProgram[i].codec));
                            ddreSettings.externalPcmMode = true;
                            break;
                        }
                    }
                }

                if ( ddreSettings.externalPcmMode == true )
                {
                    ddreSettings.cut = dolby->cut;
                    ddreSettings.boost = dolby->boost;
                    BDBG_WRN(("Cut:%d Boost:%d \n", ddreSettings.cut, ddreSettings.boost));
                }

                ddreSettings.certificationMode = dolby->certificationMode_ddre;
                NEXUS_DolbyDigitalReencode_SetSettings(recs->ddre, &ddreSettings);
            }

            if ( (dolby->enableDecode[SECONDARY_DECODE] && idx == SECONDARY_DECODE) ||
                 (dolby->enableDecode[EFFECTS_DECODE] && idx == EFFECTS_DECODE) ||
                 (dolby->enableDecode[APPAUDIO_DECODE] && idx == APPAUDIO_DECODE) ||
                 (idx == PRIMARY_DECODE) )
            {
                BERR_Code rc;
                printf("\n\n Starting decoder:%d \n\n", idx);
                rc = NEXUS_AudioDecoder_Start(recs->audioDecoders[idx], &(recs->audioProgram[idx]));
                BDBG_ASSERT(!rc);
                recs->started[idx] = true;
                recs->suspended[idx] = false;
            }

            if ( recs->compressedDecoder && idx == PRIMARY_DECODE )
            {
                printf("\n\n Starting Compressed decoder:%d \n\n", idx);
                rc = NEXUS_AudioDecoder_Start(recs->compressedDecoder, &recs->audioProgram[idx]);
                BDBG_ASSERT(!rc);
            }
        }
    }

    return 0;
}

int decode_path_stop( int idx, struct app_resources * recs )
{
    if ( idx == PRIMARY_DECODE && recs->started[idx] )
    {
        if ( recs->videoDecoder )
        {
            NEXUS_VideoDecoder_Stop(recs->videoDecoder);
        }
        if ( recs->compressedDecoder )
        {
            NEXUS_AudioDecoder_Stop(recs->compressedDecoder);
        }
    }

    if ( recs->audioDecoders[idx] && recs->started[idx] )
    {
        printf("stop audioDecoders[%d] %p\n", idx, (void *)recs->audioDecoders[idx]);
        NEXUS_AudioDecoder_Stop(recs->audioDecoders[idx]);
        recs->started[idx] = false;
        recs->suspended[idx] = false;
    }

    return 0;
}

int playback_start( int idx, struct util_opts_t *opts, struct app_resources * recs )
{
    BERR_Code rc;

    NEXUS_PlaybackStartSettings playbackStartSettings;
    if ( recs->playback[idx] && recs->started[idx] )
    {
        NEXUS_Playback_GetDefaultStartSettings(&playbackStartSettings);
        if ( opts[idx].autoBitrate )
        {
            playbackStartSettings.mode = NEXUS_PlaybackMode_eAutoBitrate;
        }
        rc = NEXUS_Playback_Start(recs->playback[idx], recs->file[idx], &playbackStartSettings);
        BDBG_ASSERT(!rc);
    }

    return 0;
}

int playback_stop( int idx, struct app_resources * recs )
{
    if ( recs->playback[idx] )
    {
        BERR_Code rc;
        NEXUS_PlaybackStatus pb_status;

        rc = NEXUS_Playback_GetStatus(recs->playback[idx], &pb_status);
        if ( rc == BERR_SUCCESS && pb_status.state != NEXUS_PlaybackState_eStopped )
        {
            printf("stop playback[%d] %p\n", idx, (void *)recs->playback[idx]);
            NEXUS_Playback_Stop(recs->playback[idx]);
        }
    }

    return 0;
}

#if NEXUS_NUM_HDMI_OUTPUTS > 0
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/* HDMI Support */
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define USE_PRODUCTION_KEYS 0

#if USE_PRODUCTION_KEYS

/*****************************/
/* INSERT PRODUCTION KeySet HERE */
/*****************************/

#else


/**************************************/
/* HDCP Specification Test Key Set    */
/*                                    */
/* NOTE: the default declared Test    */
/* KeySet below is from the HDCP Spec */
/* and it *IS NOT* compatible with    */
/* production devices                 */
/**************************************/


static NEXUS_HdmiOutputHdcpKsv hdcpTxAksv =
{ { 0x14, 0xF7, 0x61, 0x03, 0xB7 } };

static NEXUS_HdmiOutputHdcpKey encryptedTxKeySet[NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS] =
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x691e138f, 0x58a44d00 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x0950e658, 0x35821f00 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x0d98b9ab, 0x476a8a00 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xcac5cb52, 0x1b18f300 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xb4d89668, 0x7f14fb00 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x818f4878, 0xc98be000 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x412c11c8, 0x64d0a000 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x44202428, 0x5a9db300 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x6b56adbd, 0xb228b900 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xf6e46c4a, 0x7ba49100 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x589d5e20, 0xf8005600 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xa03fee06, 0xb77f8c00 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x28bc7c9d, 0x8c2dc000 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x059f4be5, 0x61125600 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xcbc1ca8c, 0xdef07400 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x6adbfc0e, 0xf6b83b00 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xd72fb216, 0xbb2ba000 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x98547846, 0x8e2f4800 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x38472762, 0x25ae6600 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xf2dd23a3, 0x52493d00 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x543a7b76, 0x31d2e200 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x2561e6ed, 0x1a584d00 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xf7227bbf, 0x82603200 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x6bce3035, 0x461bf600 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x6b97d7f0, 0x09043600 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xf9498d61, 0x05e1a100 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x063405d1, 0x9d8ec900 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x90614294, 0x67c32000 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xc34facce, 0x51449600 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x8a8ce104, 0x45903e00 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xfc2d9c57, 0x10002900 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x80b1e569, 0x3b94d700 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x437bdd5b, 0xeac75400 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xba90c787, 0x58fb7400 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xe01d4e36, 0xfa5c9300 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xae119a15, 0x5e070300 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x01fb788a, 0x40d30500 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xb34da0d7, 0xa5590000 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x409e2c4a, 0x633b3700 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x412056b4, 0xbb732500 }
};

#endif

/*
from HDCP Spec:
Table 51 gives the format of the HDCP SRM. All values are stored in big endian format.

Specify KSVs here in big endian;
*/
#define NUM_REVOKED_KSVS 3
static uint8_t NumRevokedKsvs = NUM_REVOKED_KSVS;
static const NEXUS_HdmiOutputHdcpKsv RevokedKsvs[NUM_REVOKED_KSVS] =
{
    /* MSB ... LSB */
    { { 0xa5, 0x1f, 0xb0, 0xc3, 0x72 } },
    { { 0x65, 0xbf, 0x04, 0x8a, 0x7c } },
    { { 0x65, 0x65, 0x1e, 0xd5, 0x64 } }
};

static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputSettings hdmiSettings;
    hotplugCallbackParameters *hotPlugCbParams ;

    hotPlugCbParams = (hotplugCallbackParameters *) pParam ;
    hdmi = hotPlugCbParams->hdmiOutput ;
    display = hotPlugCbParams->display ;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( !status.connected )
    {
        BDBG_WRN(("No RxDevice Connected"));
        return;
    }

    NEXUS_Display_GetSettings(display, &displaySettings);
    if ( !status.videoFormatSupported[displaySettings.format] )
    {
        BDBG_ERR(("Current format not supported by attached monitor. Switching to preferred format %d",
                  status.preferredVideoFormat));
        displaySettings.format = status.preferredVideoFormat;
    }
    NEXUS_Display_SetSettings(display, &displaySettings);

    /* force HDMI updates after a hotplug */
    NEXUS_HdmiOutput_GetSettings(hdmi, &hdmiSettings);
    NEXUS_HdmiOutput_SetSettings(hdmi, &hdmiSettings);

    /* restart HDCP if it was previously enabled */
    if ( hdmiHdcpEnabled )
    {
        NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
    }
}

static void hdmiOutputHdcpStateChanged(void *pContext, int param)
{
    bool success = false;
    NEXUS_HdmiOutputHandle handle = pContext;
    NEXUS_HdmiOutputHdcpStatus hdcpStatus;

    BSTD_UNUSED(param);

    NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdcpStatus);
    switch ( hdcpStatus.hdcpError )
    {
    case NEXUS_HdmiOutputHdcpError_eSuccess :
        BDBG_WRN(("HDCP Authentication Successful\n"));
        success = true;
        hdmiHdcpEnabled = true;
        break;

    case NEXUS_HdmiOutputHdcpError_eRxBksvError :
        BDBG_ERR(("HDCP Rx BKsv Error"));
        break;

    case NEXUS_HdmiOutputHdcpError_eRxBksvRevoked :
        BDBG_ERR(("HDCP Rx BKsv/Keyset Revoked"));
        break;

    case NEXUS_HdmiOutputHdcpError_eRxBksvI2cReadError :
    case NEXUS_HdmiOutputHdcpError_eTxAksvI2cWriteError :
        BDBG_ERR(("HDCP I2C Read Error"));
        break;

    case NEXUS_HdmiOutputHdcpError_eTxAksvError :
        BDBG_ERR(("HDCP Tx Aksv Error"));
        break;

    case NEXUS_HdmiOutputHdcpError_eReceiverAuthenticationError :
        BDBG_ERR(("HDCP Receiver Authentication Failure"));
        break;

    case NEXUS_HdmiOutputHdcpError_eRepeaterAuthenticationError :
    case NEXUS_HdmiOutputHdcpError_eRepeaterLinkFailure :    /* Repeater Error; unused */
        BDBG_ERR(("HDCP Repeater Authentication Failure"));
        break;

    case NEXUS_HdmiOutputHdcpError_eRxDevicesExceeded :
        BDBG_ERR(("HDCP Repeater MAX Downstram Devices Exceeded"));
        break;

    case NEXUS_HdmiOutputHdcpError_eRepeaterDepthExceeded :
        BDBG_ERR(("HDCP Repeater MAX Downstram Levels Exceeded"));
        break;

    case NEXUS_HdmiOutputHdcpError_eRepeaterFifoNotReady :
        BDBG_ERR(("Timeout waiting for Repeater"));
        break;

    case NEXUS_HdmiOutputHdcpError_eRepeaterDeviceCount0 : /* unused */
        break;

    case NEXUS_HdmiOutputHdcpError_eLinkRiFailure :
        BDBG_ERR(("HDCP Ri Integrity Check Failure"));
        break;

    case NEXUS_HdmiOutputHdcpError_eLinkPjFailure :
        BDBG_ERR(("HDCP Pj Integrity Check Failure"));
        break;

    case NEXUS_HdmiOutputHdcpError_eFifoUnderflow :
    case NEXUS_HdmiOutputHdcpError_eFifoOverflow :
        BDBG_ERR(("Video configuration issue"));
        break;

    case NEXUS_HdmiOutputHdcpError_eMultipleAnRequest : /* Should not reach here; but flag if occurs */
        BDBG_WRN(("Multiple Authentication Request... "));

    default :
        BDBG_WRN(("Unknown HDCP Authentication Error %d", hdcpStatus.hdcpError));
    }

    if ( !success )
    {
        fprintf(stderr, "\nHDCP Authentication Failed.  Current State %d\n", hdcpStatus.hdcpState);

        /* always retry */
        NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
    }
}

static void initializeHdmiOutputHdcpSettings(void)
{
    NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;

    NEXUS_HdmiOutput_GetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);

    /* copy the encrypted key set and its Aksv here  */
    BKNI_Memcpy(hdmiOutputHdcpSettings.encryptedKeySet, encryptedTxKeySet,
                NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS * sizeof(NEXUS_HdmiOutputHdcpKey));
    BKNI_Memcpy(&hdmiOutputHdcpSettings.aksv, &hdcpTxAksv,
                NEXUS_HDMI_OUTPUT_HDCP_KSV_LENGTH);

    /* install HDCP success  callback */
    hdmiOutputHdcpSettings.successCallback.callback = hdmiOutputHdcpStateChanged;
    hdmiOutputHdcpSettings.successCallback.context = platformConfig.outputs.hdmi[0];

    /* install HDCP failure callback */
    hdmiOutputHdcpSettings.failureCallback.callback = hdmiOutputHdcpStateChanged;
    hdmiOutputHdcpSettings.failureCallback.context = platformConfig.outputs.hdmi[0];

    NEXUS_HdmiOutput_SetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);

    /* install list of revoked KSVs from SRMs (System Renewability Message) if available */
    NEXUS_HdmiOutput_SetHdcpRevokedKsvs(platformConfig.outputs.hdmi[0],
                                        RevokedKsvs, NumRevokedKsvs);

}
#endif

int main(int argc, const char *argv[])
{
    #if NEXUS_HAS_PLAYBACK
    NEXUS_AudioMixerSettings mixerSettings;
    NEXUS_PlaybackStatus pstatus;
    #if NEXUS_NUM_HDMI_OUTPUTS > 0
    NEXUS_HdmiOutputStatus hdmiStatus;
    #endif

    int i, no_of_streams = NUM_DECODES;
    NEXUS_Error rc;

    struct app_resources resources;
    struct dolby_digital_plus_command_args dolby_args;
    struct util_opts_t opts[NUM_DECODES];

    FILE *file_params = NULL;

    NEXUS_AudioOutputEnabledOutputs outputs;
    NEXUS_AudioOutputClockConfig config;
    NEXUS_AudioOutputSettings outputSettings;
    unsigned timingResourceIndex = 1;

    bool exit;

    /* clear our structures */
    BKNI_Memset(&dolby_args, 0, sizeof(dolby_args));
    BKNI_Memset(&resources, 0, sizeof(resources));
    BKNI_Memset(opts, 0, sizeof(struct util_opts_t) * NUM_DECODES);

    NEXUS_Platform_GetDefaultSettings(&resources.platformSettings);
    resources.platformSettings.openFrontend = false;
    resources.platformSettings.audioModuleSettings.numPcmBuffers = 6;
    rc = NEXUS_Platform_Init(&resources.platformSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Populate Default algo parameters */
    {
        NEXUS_AudioDecoderHandle tempDecoder = NEXUS_AudioDecoder_Open(0, NULL);
        BDBG_ASSERT(tempDecoder);
        NEXUS_AudioDecoder_GetCodecSettings(tempDecoder, NEXUS_AudioCodec_eAc3Plus, &dolby_args.ac3CodecSettings);
        NEXUS_AudioDecoder_GetCodecSettings(tempDecoder, NEXUS_AudioCodec_eAacPlus, &dolby_args.aacCodecSettings);
        NEXUS_AudioDecoder_GetCodecSettings(tempDecoder, NEXUS_AudioCodec_eAc4, &dolby_args.ac4CodecSettings);
        NEXUS_AudioDecoder_Close(tempDecoder);
    }

    dolby_args.dialog_level = 31;
    dolby_args.enableVideo = true;
    dolby_args.enableDac = true;
    dolby_args.enableSpdif = true;
    dolby_args.enableHdmi = true;
    dolby_args.connectors.spdif = NULL;
    dolby_args.connectors.hdmi = NULL;
    dolby_args.connectors.dac = NULL;
    dolby_args.connectors.dummy = NULL;
    dolby_args.enableDdre = true;
    dolby_args.enableDecode[PRIMARY_DECODE] = true;
    dolby_args.enableDecode[SECONDARY_DECODE] = true;
    dolby_args.enableDecode[EFFECTS_DECODE] = true;
    dolby_args.enableDecode[APPAUDIO_DECODE] = true;
    dolby_args.enableAtmos = true;
    dolby_args.compression = NEXUS_DolbyDigitalReencodeProfile_eFilmStandardCompression;
    dolby_args.cut = 100;
    dolby_args.boost = 100;
    /* Dapv2 parameters */
    dolby_args.deqNumBands = 10;
    dolby_args.ac4PresIdxMain = 0xFFFFFFFF;
    dolby_args.ac4PresIdxAlt = 0xFFFFFFFF;
    dolby_args.ac4PresIdxAssoc = 0xFFFFFFFF;
    dolby_args.ac4PresIdMain[0] = '\0';
    dolby_args.ac4PresIdAlt[0] = '\0';
    dolby_args.ac4PresIdAssoc[0] = '\0';
    dolby_args.ac4DecodeMode = 0;

    Intialize_values(&dolby_args.deqFrequency[0], preset_freq[0], dolby_args.deqNumBands);
    Intialize_values((uint32_t *)&dolby_args.deqGain[0], preset_gain[0], dolby_args.deqNumBands);

    dolby_args.fadeLevel[PRIMARY_DECODE] = INVALID_FADE_LEVEL;
    dolby_args.fadeLevel[SECONDARY_DECODE] = INVALID_FADE_LEVEL;
    dolby_args.fadeLevel[EFFECTS_DECODE] = INVALID_FADE_LEVEL;
    dolby_args.fadeLevel[APPAUDIO_DECODE] = INVALID_FADE_LEVEL;
    dolby_args.fadeLevel[OUTPUT_FADE_IDX] = INVALID_FADE_LEVEL;

/*-------------------------------------------------------------------Initialization----------------------------------------------------------------------------------------------------------------*/

    NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
    mixerSettings.mixUsingDsp = true;
    resources.mixer = NEXUS_AudioMixer_Open(&mixerSettings);
    resources.directMixer = NEXUS_AudioMixer_Open(NULL);
    resources.directMchMixer = NEXUS_AudioMixer_Open(NULL);

    #if NEXUS_NUM_AUDIO_PLAYBACKS
    resources.audioPlayback = NEXUS_AudioPlayback_Open(0, NULL);
    if ( NULL == resources.audioPlayback )
    {
        fprintf(stderr, "Unable to open playback channel\n");
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        BDBG_ASSERT(NEXUS_INVALID_PARAMETER);
    }
    #endif

    resources.videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_Display_GetDefaultSettings(&resources.displaySettings);
    #if NEXUS_NUM_HDMI_OUTPUTS > 0
    NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( hdmiStatus.connected )
    {
        resources.displaySettings.format = hdmiStatus.preferredVideoFormat;
    }
    #endif
    resources.display = NEXUS_Display_Open(0, &resources.displaySettings);
    resources.window = NEXUS_VideoWindow_Open(resources.display, 0);
    NEXUS_VideoWindow_AddInput(resources.window, NEXUS_VideoDecoder_GetConnector(resources.videoDecoder));

    if ( dolby_digital_plus_cmdline_parse(argc, argv, opts, &dolby_args) )
    {
        return 0;
    }

    for ( i = 0; i < no_of_streams; i++ )
    {
        int ret = decode_path_initialize( i, opts, &dolby_args, &resources, true );
        if ( ret != 0 )
        {
            print_usage(argv[0]);
            return ret;
        }
    }

    defaultFadeLevel = 100;
    if ( dolby_args.certificationMode_dap )
    {
        defaultFadeLevel = 0x7FFFFFFF;
    }

    for ( i = 0; i < 5; i++ )
    {
        if ( dolby_args.fadeLevel[i] == INVALID_FADE_LEVEL )
        {
            dolby_args.fadeLevel[i] = defaultFadeLevel;
        }
    }

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    #if NEXUS_NUM_HDMI_OUTPUTS > 0
    /* Wait for HDMI to connect (up to 2 seconds) */
    for ( i = 0; i < 200; i++ )
    {
        NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
        if ( hdmiStatus.connected )
        {
            break;
        }
        BKNI_Sleep(10);
    }
    #endif

/*---------------------------------------------------------------------Configure Video / Display-----------------------------------------------------------------------------------------------------------------*/

    #if NEXUS_NUM_HDMI_OUTPUTS > 0
    NEXUS_Display_AddOutput(resources.display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    #endif
    #if NEXUS_NUM_COMPONENT_OUTPUTS
    if ( platformConfig.outputs.component[0] )
    {
        NEXUS_Display_AddOutput(resources.display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    }
    #endif
    #if NEXUS_NUM_COMPOSITE_OUTPUTS
    if ( resources.displaySettings.format == NEXUS_VideoFormat_eNtsc )
    {
        NEXUS_Display_AddOutput(resources.display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    }
    #endif
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------Make our Connections--------------------------------------------------------------------------------------------------------------------------*/

    #if 0
    /* if secondary wasn't enabled, don't connect it */
    if ( dolby_args.secondary_stream == NULL )
    {
        dolby_args.enableDecode[SECONDARY_DECODE] = false;
    }

    /* if sound effects wasn't enabled, don't connect it */
    if ( dolby_args.sound_effects_stream == NULL )
    {
        dolby_args.enableDecode[EFFECTS_DECODE] = false;
    }

    /* if sound effects wasn't enabled, don't connect it */
    if ( dolby_args.app_audio_stream == NULL )
    {
        dolby_args.enableDecode[APPAUDIO_DECODE] = false;
    }
    #endif


    if ( dolby_args.enableSpdif && platformConfig.outputs.spdif[0] )
    {
        dolby_args.connectors.spdif = NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]);
    }
    if ( dolby_args.enableHdmi && platformConfig.outputs.hdmi[0] )
    {
        dolby_args.connectors.hdmi = NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]);
    }
    if ( dolby_args.enableDac && platformConfig.outputs.audioDacs[0] )
    {
        dolby_args.connectors.dac = NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]);
    }
    if ( platformConfig.outputs.audioDummy[0] )
    {
        dolby_args.connectors.dummy = NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]);
    }

    if ( dolby_args.directMode )
    {
        /* Disable everything except primary and pcm playback in direct mode */
        dolby_args.enableDecode[SECONDARY_DECODE] = dolby_args.enableDecode[EFFECTS_DECODE] = dolby_args.enableDecode[APPAUDIO_DECODE] = false;

        printf("\n------------------------------------\n");
        printf("DIRECT Decoder->Output Mode connections:\n");
        printf("------------------------------------\n");
        if ( dolby_args.connectors.hdmi )
        {
            printf("  HDMI %s PCM\n", (resources.audioProgram[0].codec == NEXUS_AudioCodec_eAc3Plus && dolby_args.multiCh71) ? "7.1ch" : "5.1ch");
            rc = NEXUS_AudioOutput_AddInput(dolby_args.connectors.hdmi,
                                            NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[0], NEXUS_AudioConnectorType_eMultichannel));
            BDBG_ASSERT(NEXUS_SUCCESS == rc);
        }
        if ( dolby_args.enableCompressed &&
             (resources.audioProgram[0].codec == NEXUS_AudioCodec_eAc3Plus || resources.audioProgram[0].codec == NEXUS_AudioCodec_eAc3) )
        {
            if ( dolby_args.connectors.spdif )
            {
                printf("  SPDIF AC3 Compressed\n");
                rc = NEXUS_AudioOutput_AddInput(dolby_args.connectors.spdif,
                                                NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[0], NEXUS_AudioConnectorType_eCompressed));
                BDBG_ASSERT(NEXUS_SUCCESS == rc);
            }
            if ( !dolby_args.connectors.dac && !dolby_args.connectors.hdmi && dolby_args.connectors.dummy )
            {
                printf("  DUMMY PCM Stereo\n");
                rc = NEXUS_AudioOutput_AddInput(dolby_args.connectors.dummy,
                                                NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[0], NEXUS_AudioConnectorType_eStereo));
                BDBG_ASSERT(NEXUS_SUCCESS == rc);
            }
        }
        else
        {
            if ( dolby_args.connectors.spdif && dolby_args.altStereo )
            {
                printf("  SPDIF Ac4 Alt Stereo\n");
                rc = NEXUS_AudioOutput_AddInput(dolby_args.connectors.spdif,
                                                NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[0], NEXUS_AudioConnectorType_eAlternateStereo));
                BDBG_ASSERT(NEXUS_SUCCESS == rc);
            }
            else if ( dolby_args.connectors.spdif )
            {
                printf("  SPDIF PCM Stereo\n");
                rc = NEXUS_AudioOutput_AddInput(dolby_args.connectors.spdif,
                                                NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[0], NEXUS_AudioConnectorType_eStereo));
                BDBG_ASSERT(NEXUS_SUCCESS == rc);
            }
            if ( !dolby_args.connectors.spdif && !dolby_args.connectors.dac && !dolby_args.connectors.hdmi && dolby_args.connectors.dummy )
            {
                printf("  DUMMY PCM Stereo\n");
                rc = NEXUS_AudioOutput_AddInput(dolby_args.connectors.dummy,
                                                NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[0], NEXUS_AudioConnectorType_eStereo));
                BDBG_ASSERT(NEXUS_SUCCESS == rc);
            }
        }

        if ( dolby_args.connectors.dac )
        {
            printf("  DAC PCM Stereo\n");
            rc = NEXUS_AudioOutput_AddInput(dolby_args.connectors.dac,
                                            NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[0], NEXUS_AudioConnectorType_eStereo));
            BDBG_ASSERT(NEXUS_SUCCESS == rc);
        }
    }
    else
    {
        NEXUS_AudioMixer_AddInput(resources.mixer, NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[0], NEXUS_AudioConnectorType_eMultichannel));
        if ( dolby_args.enableDecode[SECONDARY_DECODE] )
        {
            NEXUS_AudioMixer_AddInput(resources.mixer, NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[1], NEXUS_AudioConnectorType_eMultichannel));
        }
        if ( dolby_args.enableDecode[EFFECTS_DECODE] )
        {
            NEXUS_AudioMixer_AddInput(resources.mixer, NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[2], NEXUS_AudioConnectorType_eMultichannel));
        }
        if ( dolby_args.enableDecode[APPAUDIO_DECODE] )
        {
            NEXUS_AudioMixer_AddInput(resources.mixer, NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[3], NEXUS_AudioConnectorType_eMultichannel));
        }
        #if NEXUS_NUM_AUDIO_PLAYBACKS
        if ( dolby_args.enablePlayback )
        {
            NEXUS_AudioMixer_AddInput(resources.mixer, NEXUS_AudioPlayback_GetConnector(resources.audioPlayback));
        }
        #endif

        /* Set the Mixer to use DSP mixing */
        NEXUS_AudioMixer_GetSettings(resources.mixer, &mixerSettings);
        mixerSettings.master = NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[0], NEXUS_AudioDecoderConnectorType_eMultichannel);
        apply_mixer_settings(&mixerSettings, &dolby_args);
        NEXUS_AudioMixer_SetSettings(resources.mixer, &mixerSettings);

        printf("\n------------------------------------\n");
        printf("Full MS12 Decoder->DspMixer->DDRE->Outputs Mode connections:\n");
        printf("------------------------------------\n");
        if ( dolby_args.enableDdre )
        {
            bool stereoConsumer = false;
            unsigned c;
            rc = NEXUS_DolbyDigitalReencode_AddInput(resources.ddre, NEXUS_AudioMixer_GetConnector(resources.mixer));
            BDBG_ASSERT(NEXUS_SUCCESS == rc);

            if ( dolby_args.connectors.dac )
            {
                printf("  DAC Stereo PCM\n");
                rc = NEXUS_AudioOutput_AddInput(dolby_args.connectors.dac,
                                                NEXUS_DolbyDigitalReencode_GetConnector(resources.ddre, NEXUS_AudioConnectorType_eStereo));
                BDBG_ASSERT(NEXUS_SUCCESS == rc);
                stereoConsumer = true;
            }

            if ( dolby_args.connectors.spdif )
            {
                if ( dolby_args.altStereo && resources.audioProgram[0].codec == NEXUS_AudioCodec_eAc4 )
                {
                    printf("  SPDIF Ac4 Alt Stereo\n");
                    rc = NEXUS_AudioOutput_AddInput(dolby_args.connectors.spdif,
                                                    NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[0], NEXUS_AudioConnectorType_eAlternateStereo));
                    BDBG_ASSERT(NEXUS_SUCCESS == rc);
                }
                else if ( dolby_args.enableCompressed )
                {
                    printf("  SPDIF Ac3 Compressed\n");
                    rc = NEXUS_AudioOutput_AddInput(dolby_args.connectors.spdif,
                                                    NEXUS_DolbyDigitalReencode_GetConnector(resources.ddre, NEXUS_AudioConnectorType_eCompressed));
                    BDBG_ASSERT(NEXUS_SUCCESS == rc);
                }
                else
                {
                    printf("  SPDIF Stereo PCM\n");
                    rc = NEXUS_AudioOutput_AddInput(dolby_args.connectors.spdif,
                                                    NEXUS_DolbyDigitalReencode_GetConnector(resources.ddre, NEXUS_AudioConnectorType_eStereo));
                    BDBG_ASSERT(NEXUS_SUCCESS == rc);
                    stereoConsumer = true;
                }
            }

            if ( dolby_args.connectors.hdmi )
            {
                if ( dolby_args.enableCompressed )
                {
                    if ( resources.audioProgram[0].codec == NEXUS_AudioCodec_eAc4 ||
                         resources.audioProgram[0].codec == NEXUS_AudioCodec_eAc3Plus || resources.audioProgram[0].codec == NEXUS_AudioCodec_eAc3 ||
                         resources.audioProgram[0].codec == NEXUS_AudioCodec_eAacAdts || resources.audioProgram[0].codec == NEXUS_AudioCodec_eAacLoas ||
                         resources.audioProgram[0].codec == NEXUS_AudioCodec_eAacPlusAdts || resources.audioProgram[0].codec == NEXUS_AudioCodec_eAacPlusLoas )
                    {
                        printf("  HDMI DDP Compressed\n");
                        NEXUS_AudioOutput_AddInput(dolby_args.connectors.hdmi,
                                                   NEXUS_DolbyDigitalReencode_GetConnector(resources.ddre, NEXUS_AudioConnectorType_eCompressed4x));
                    }
                    else
                    {
                        printf("  HDMI AC3 Compressed\n");
                        NEXUS_AudioOutput_AddInput(dolby_args.connectors.hdmi,
                                                   NEXUS_DolbyDigitalReencode_GetConnector(resources.ddre, NEXUS_AudioConnectorType_eCompressed));
                    }
                    BDBG_ASSERT(NEXUS_SUCCESS == rc);
                }
                else
                {
                    printf("  HDMI %s PCM\n", ((resources.audioProgram[0].codec == NEXUS_AudioCodec_eAc3Plus || resources.audioProgram[0].codec == NEXUS_AudioCodec_eAc4) && dolby_args.multiCh71) ? "7.1ch" : "5.1ch");
                    NEXUS_AudioOutput_AddInput(dolby_args.connectors.hdmi,
                                               NEXUS_DolbyDigitalReencode_GetConnector(resources.ddre, NEXUS_AudioConnectorType_eMultichannel));
                }
            }

            if ( !stereoConsumer && dolby_args.connectors.dummy )
            {
                printf("  DUMMY PCM Stereo\n");
                rc = NEXUS_AudioOutput_AddInput(dolby_args.connectors.dummy,
                                                NEXUS_DolbyDigitalReencode_GetConnector(resources.ddre, NEXUS_AudioConnectorType_eStereo));
                BDBG_ASSERT(NEXUS_SUCCESS == rc);
            }

            /* Setup output clocks */
            {
                NEXUS_AudioOutput_GetDefaultEnabledOutputs(&outputs);

                if ( dolby_args.connectors.spdif )
                {
                    outputs.spdif[0] = timingResourceIndex++;
                }
                if ( dolby_args.connectors.hdmi )
                {
                    outputs.hdmi[0] = timingResourceIndex++;
                }
                if ( dolby_args.connectors.dummy )
                {
                    outputs.audioDummy[0] = timingResourceIndex++;
                }

                for ( c = 0; c < NUM_CAPTURES; c++ )
                {
                    if ( dolby_args.audioCapture[c].enabled )
                    {
                        outputs.audioCapture[c] = timingResourceIndex++;
                    }
                }
                rc = NEXUS_AudioOutput_CreateClockConfig(&outputs, &config);
                BDBG_ASSERT(NEXUS_SUCCESS == rc);

                if ( dolby_args.connectors.spdif && config.spdif[0].pll != NEXUS_AudioOutputPll_eMax )
                {
                    NEXUS_AudioOutput_GetSettings(dolby_args.connectors.spdif, &outputSettings);
                    outputSettings.pll = config.spdif[0].pll;
                    /* spdif doesn't have nco */
                    NEXUS_AudioOutput_SetSettings(dolby_args.connectors.spdif, &outputSettings);
                }
                if ( dolby_args.connectors.hdmi &&
                     ( config.hdmi[0].pll != NEXUS_AudioOutputPll_eMax || config.hdmi[0].nco != NEXUS_AudioOutputNco_eMax ) )
                {
                    NEXUS_AudioOutput_GetSettings(dolby_args.connectors.hdmi, &outputSettings);
                    outputSettings.pll = config.hdmi[0].pll;
                    outputSettings.nco = config.hdmi[0].nco;
                    NEXUS_AudioOutput_SetSettings(dolby_args.connectors.hdmi, &outputSettings);
                }
                if ( dolby_args.connectors.dummy &&
                     ( config.audioDummy[0].pll != NEXUS_AudioOutputPll_eMax || config.audioDummy[0].nco != NEXUS_AudioOutputNco_eMax ) ) {
                    NEXUS_AudioOutput_GetSettings(dolby_args.connectors.dummy, &outputSettings);
                    outputSettings.pll = config.audioDummy[0].pll;
                    outputSettings.nco = config.audioDummy[0].nco;
                    NEXUS_AudioOutput_SetSettings(dolby_args.connectors.dummy, &outputSettings);
                }
            }

            /* Create/Attach captures */
            for ( c = 0; c < NUM_CAPTURES; c++ )
            {
                if ( dolby_args.audioCapture[c].enabled )
                {
                    unsigned numChPairs = 1;
                    NEXUS_AudioCaptureOpenSettings captureSettings;
                    NEXUS_AudioCaptureStartSettings captureStartSettings;

                    if ( !resources.captureEvent )
                    {
                        BKNI_CreateEvent(&resources.captureEvent);
                        BDBG_ASSERT(resources.captureEvent != NULL);
                    }

                    if ( !resources.captureThread )
                    {
                        pthread_create(&resources.captureThread, NULL, capture_thread, &resources);
                    }

                    printf("\n  %s Capture enabled\n", c==CAPTURE_COMP4X?"Compressed4x":c==CAPTURE_COMP?"Compressed":c==CAPTURE_MULTICH?"Multichannel":"Stereo");
                    /* open capture */
                    NEXUS_AudioCapture_GetDefaultOpenSettings(&captureSettings);
                    switch ( c )
                    {
                    default:
                    case CAPTURE_STEREO:
                        captureSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_eStereo;
                        captureSettings.format = NEXUS_AudioCaptureFormat_e24BitStereo;
                        break;
                    case CAPTURE_MULTICH:
                        if ( dolby_args.multiCh71 )
                        {
                            printf("    7.1ch\n");
                            captureSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e7_1;
                            captureSettings.format = NEXUS_AudioCaptureFormat_e24Bit7_1;
                            numChPairs = 4;
                        }
                        else
                        {
                            printf("    5.1ch\n");
                            captureSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;
                            captureSettings.format = NEXUS_AudioCaptureFormat_e24Bit5_1;
                            numChPairs = 3;
                        }
                        captureSettings.fifoSize *= numChPairs;
                        break;
                    case CAPTURE_COMP:
                        captureSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_eStereo;
                        captureSettings.format = NEXUS_AudioCaptureFormat_eCompressed;
                        break;
                    case CAPTURE_COMP4X:
                        captureSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_eStereo;
                        captureSettings.format = NEXUS_AudioCaptureFormat_eCompressed;
                        captureSettings.fifoSize *= 4;
                        break;
                    }
                    resources.audioCapture[c].audioCapture = NEXUS_AudioCapture_Open(c, &captureSettings);
                    BDBG_ASSERT(resources.audioCapture[c].audioCapture != NULL);

                    /* create file for capture */
                    if ( strlen(dolby_args.audioCapture[c].filename) > 0 )
                    {
                        unsigned p;
                        char *ext;
                        char base[MAX_FILENAME_LEN];
                        char filename[MAX_FILENAME_LEN+1];

                        strcpy(base, dolby_args.audioCapture[c].filename);
                        ext = strchr(base, '.');
                        *ext = 0;
                        ext++;

                        for ( p = 0; p < numChPairs; p++ )
                        {
                            sprintf(filename, "%s%d.%s", base, p, ext);
                            printf("    opening output capture file \"%s\"\n", filename);
                            resources.audioCapture[c].file[p] = fopen(filename, "wb");
                            BDBG_ASSERT(resources.audioCapture[c].file[p] != NULL);
                        }
                    }

                    /* connect capture */
                    switch ( c )
                    {
                    default:
                    case CAPTURE_STEREO:
                        NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(resources.audioCapture[c].audioCapture),
                                                   NEXUS_DolbyDigitalReencode_GetConnector(resources.ddre, NEXUS_AudioConnectorType_eStereo));
                        break;
                    case CAPTURE_MULTICH:
                        NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(resources.audioCapture[c].audioCapture),
                                                   NEXUS_DolbyDigitalReencode_GetConnector(resources.ddre, NEXUS_AudioConnectorType_eMultichannel));
                        break;
                    case CAPTURE_COMP:
                        NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(resources.audioCapture[c].audioCapture),
                                                   NEXUS_DolbyDigitalReencode_GetConnector(resources.ddre, NEXUS_AudioConnectorType_eCompressed));
                        break;
                    case CAPTURE_COMP4X:
                        NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(resources.audioCapture[c].audioCapture),
                                                   NEXUS_DolbyDigitalReencode_GetConnector(resources.ddre, NEXUS_AudioConnectorType_eCompressed4x));
                        break;
                    }

                    if (config.audioCapture[c].pll != NEXUS_AudioOutputPll_eMax || config.audioCapture[c].nco != NEXUS_AudioOutputNco_eMax )
                    {
                        NEXUS_AudioOutput_GetSettings(NEXUS_AudioCapture_GetConnector(resources.audioCapture[c].audioCapture), &outputSettings);
                        outputSettings.pll = config.audioCapture[c].pll;
                        outputSettings.nco = config.audioCapture[c].nco;
                        NEXUS_AudioOutput_SetSettings(NEXUS_AudioCapture_GetConnector(resources.audioCapture[c].audioCapture), &outputSettings);
                    }


                    /* start capture */
                    NEXUS_AudioCapture_GetDefaultStartSettings(&captureStartSettings);
                    captureStartSettings.dataCallback.callback = audio_capture_callback;
                    captureStartSettings.dataCallback.context = resources.captureEvent;
                    captureStartSettings.dataCallback.param = (int)c;
                    rc = NEXUS_AudioCapture_Start(resources.audioCapture[c].audioCapture, &captureStartSettings);
                    BDBG_ASSERT(NEXUS_SUCCESS == rc);
                    printf("    capture started.\n");
                }
            }
        }
        else
        {
            if ( dolby_args.connectors.hdmi )
            {
                printf("  HDMI %s PCM\n", ((resources.audioProgram[0].codec == NEXUS_AudioCodec_eAc3Plus || resources.audioProgram[0].codec == NEXUS_AudioCodec_eAc4) && dolby_args.multiCh71) ? "7.1ch" : "5.1ch");
                NEXUS_AudioOutput_AddInput(dolby_args.connectors.hdmi,
                                           NEXUS_AudioMixer_GetConnector(resources.mixer));
            }
        }
        printf("------------------------------------\n\n");
    }

    /* Nudge HDMI interface with some initial settings and a hotplug */
    if ( dolby_args.connectors.hdmi )
    {
        NEXUS_HdmiOutputSettings hdmiSettings;
        hotplugCallbackParameters hotPlugCbParams;
        NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
        hdmiSettings.hotplugCallback.callback = hotplug_callback;
        hotPlugCbParams.hdmiOutput = platformConfig.outputs.hdmi[0];
        hotPlugCbParams.display = resources.display;
        hdmiSettings.hotplugCallback.context = &hotPlugCbParams;
        NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);

        /* initalize HDCP settings, keys, etc. */
        initializeHdmiOutputHdcpSettings();

        /* Force a hotplug to switch to preferred format */
        hotplug_callback(&hotPlugCbParams, 0);
    }

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------Start decoders----------------------------------------------------------------------------------------------------*/

    for ( i = 0; i < no_of_streams; i++ )
    {
        decode_path_start(i, opts, &dolby_args, &resources);
    }

    if ( !dolby_args.directMode )
    {
        NEXUS_AudioMixer_GetSettings(resources.mixer, &mixerSettings);
        mixerSettings.dolby.multiStreamBalance = dolby_args.mixerBalance;
        NEXUS_AudioMixer_SetSettings(resources.mixer, &mixerSettings);
    }

    #if NEXUS_NUM_AUDIO_PLAYBACKS
    if ( dolby_args.enablePlayback )
    {
        NEXUS_AudioPlaybackStartSettings pbStartSettings;
        NEXUS_AudioPlaybackSettings pbSettings;

        BKNI_Memset(&(resources.pcm_playbacks[0]), 0, sizeof(resources.pcm_playbacks[0]));
        rc = BKNI_CreateEvent(&(resources.pcm_playbacks[0].event));
        BDBG_ASSERT(!rc);
        resources.pcm_playbacks[0].playback = resources.audioPlayback;

        /* If we have a wav file, get the sample rate from it */
        if ( dolby_args.pcm_stream != NULL )
        {
            FILE *pFile;
            unsigned long dword, bytesToPlay;

            pFile = resources.pcm_playbacks[0].pPcmFile = fopen(dolby_args.pcm_stream, "rb");
            if ( NULL == pFile )
            {
                printf("Unable to open '%s' for reading\n", dolby_args.pcm_stream);
            }

            fseek(pFile, 0, SEEK_END);
            bytesToPlay = ftell(pFile);
            fseek(pFile, 0, SEEK_SET);
            fread(&dword, 4, 1, pFile);
            if ( dword == 0x46464952 ) /* RIFF */
            { /* WAV */
                wave_header_t wh;

                fseek(pFile, 0, SEEK_SET);
                /* read in the wave file header */
                fread(&wh.riff, 4, 1, pFile);
                fread(&wh.riffCSize, 4, 1, pFile);
                fread(&wh.wave, 4, 1, pFile);
                fread(&wh.fmt, 4, 1, pFile);
                fread(&wh.headerLen, 4, 1, pFile);
                fread(&wh.format, 2, 1, pFile);
                fread(&wh.channels, 2, 1, pFile);
                fread(&wh.samplesSec, 4, 1, pFile);
                fread(&wh.bytesSec, 4, 1, pFile);
                fread(&wh.chbits, 2, 1, pFile);
                fread(&wh.bps, 2, 1, pFile);

                /* Verify that this is a PCM WAV file. */
                if ( wh.riff != 0x46464952 )
                {
                    fprintf(stderr, "RAW data file detected.\n");
                    fseek(pFile, 0, SEEK_SET);
                }
                else
                {
                    if ( wh.wave != 0x45564157 )
                    {
                        fprintf(stderr, "Not a WAV file.");
                        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                        BDBG_ASSERT(!rc);
                    }

                    if ( wh.headerLen == 40 && wh.format == 0xfffe ) /* WAVE_FORMAT_EXTENSIBLE */
                    {
                        fread(&wh.cbSize, 2, 1, pFile);                /* 2 Size of the extension (0 or 22)  */
                        fread(&wh.wValidBitsPerSample, 2, 1, pFile);   /* 2 Number of valid bits  */
                        fread(&wh.dwChannelMask, 4, 1, pFile);         /* 4 Speaker position mask  */
                        fread(&wh.SubFormat, 16, 1, pFile);            /* SubFormat GUID */
                    }
                    else if ( wh.headerLen == 18 && wh.format == 1 ) /* oddball WAVE format */
                    {
                        fread(&wh.cbSize, 2, 1, pFile);                /* 2 Size of the extension (0 or 22) ?*/
                    }
                    else if ( wh.headerLen != 16 && wh.format != 1 )
                    {
                        fprintf(stderr, "Not PCM data in WAV file. headerLen = %lu, Format 0x%x\n", wh.headerLen, wh.format);
                    }

                    for (;;)
                    {
                        if ( fread(&wh.dataSig, 4, 1, pFile) != 1 ) break;
                        if ( fread(&wh.dataLen, 4, 1, pFile) != 1 ) break;
                        /* looking for 'data' chunk */
                        if ( wh.dataSig == 0x61746164 )
                        {
                            break;
                        }
                        else if ( fseek(pFile, wh.dataLen, SEEK_CUR) )
                        {
                            break;
                        }
                    }
                    if ( wh.dataSig != 0x61746164 )
                    {
                        fprintf(stderr, "No 'data' chunk found in WAV file.\n");
                        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                        BDBG_ASSERT(!rc);
                    }
                    if ( wh.channels > 2 )
                    {
                        fprintf(stderr, "Invalid number of channels (%u) specified.\n", wh.channels);
                        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                        BDBG_ASSERT(!rc);
                    }
                    resources.pcm_playbacks[0].pcmDataStart = ftell(pFile);
                    resources.pcm_playbacks[0].pcmDataEnd = resources.pcm_playbacks[0].pcmDataStart + wh.dataLen;
                    BDBG_ASSERT(resources.pcm_playbacks[0].pcmDataEnd <= bytesToPlay);
                    pbStartSettings.sampleRate = wh.samplesSec;
                    pbStartSettings.bitsPerSample = wh.bps;
                    pbStartSettings.stereo = (wh.channels > 1);
                    pbStartSettings.signedData = (wh.bps != 8);    /* 8-bit .wav files are unsigned */
                    pbStartSettings.endian = NEXUS_EndianMode_eLittle;
                }

                resources.pcm_playbacks[0].wh = wh;
            }
        }
        else
        {
            NEXUS_AudioPlayback_GetDefaultStartSettings(&pbStartSettings);
            pbStartSettings.bitsPerSample = 16;
            pbStartSettings.sampleRate = 48000;
            pbStartSettings.stereo = true;
        }
        printf("PCM Settings - bitsPerSample %d, sampleRate %d, stereo %d, signed %d\n",
               pbStartSettings.bitsPerSample, pbStartSettings.sampleRate, pbStartSettings.stereo, pbStartSettings.signedData);
        pbStartSettings.dataCallback.callback = pcm_data_callback;
        pbStartSettings.dataCallback.context = &(resources.pcm_playbacks[0]);
        pbStartSettings.dataCallback.param = 0;
        rc = NEXUS_AudioPlayback_Start(resources.pcm_playbacks[0].playback, &pbStartSettings);
        BDBG_ASSERT(!rc);

        printf("Calling playback_thread_start(%p), audioPlayback %p\n", (void *)&(resources.pcm_playbacks[0]), (void *)resources.audioPlayback);
        playback_thread_start(&(resources.pcm_playbacks[0]));

        NEXUS_AudioPlayback_GetSettings(resources.audioPlayback, &pbSettings);
        pbSettings.leftVolume = pbSettings.rightVolume = 0x200000;
        rc = NEXUS_AudioPlayback_SetSettings(resources.audioPlayback, &pbSettings);
        BDBG_ASSERT(!rc);
    }
    #endif

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------Start Playback/Playpump-----------------------------------------------------------------------------------------------------*/
    for ( i = 0; i < no_of_streams; i++ )
    {
        playback_start(i, opts, &resources);
    }
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------Managing the runtime change of the config parameters------------------------------------------------------------------------*/
    if ( dolby_args.loopback == 1 )
    {
        for ( exit = false; !exit;)
        {
            char buffer[1024];
            char *buf;

            printf("MS12_Interactive>"); fflush(stdout);
            fgets(buffer, 1024, stdin);
            if ( feof(stdin) )
            {
                break;
            }

            buffer[strlen(buffer) - 1] = 0;

            buf = strtok(buffer, ";");
            if ( !buf ) continue;
            #if B_HAS_PLAYBACK_MONITOR
            if ( stickyFile )
            {
                BKNI_AcquireMutex(monitorState.lock);
            }
            #endif
            do
            {
                if ( !strcmp(buf, "?") )
                {
                    printf(
                        "Commands:\n"
                        "  s <index> - Toggle Suspend/Resume/Start for a decode\n"
                        "  S <index> - Toggle Stop/Start for a decode\n"
                        "  q - quit\n"
                        "  Parametername=value  - Change the value of any user configurable parameter\n"
                        );
                }
                else if ( (!strcmp(buf, "q")) || (!strcmp(buf, "quit")) )
                {
                    exit = true;
                    break;
                }
                else if ( strstr(buffer, "s ") != NULL )
                {
                    unsigned index;
                    char *value;
                    value = strchr(buffer, ' ');
                    *value = 0;
                    value++;

                    index = atoi(value);
                    printf("value '%s', %d\n", value, index);
                    if ((index < NUM_DECODES) && (resources.audioDecoders[index] != NULL))
                    {
                        printf("Toggle Start/Stop for Decode[%d] from %s to %s\n", index, resources.started[index] ? "STARTED" : "STOPPED", !resources.started[index] ? "STARTED" : "STOPPED");
                        if ( resources.started[index] && !resources.suspended[index] )
                        {
                            NEXUS_AudioDecoder_Suspend(resources.audioDecoders[index]);
                            resources.suspended[index] = true;
                        }
                        else if ( resources.suspended[index] )
                        {
                            NEXUS_AudioDecoder_Resume(resources.audioDecoders[index]);
                            resources.suspended[index] = false;
                        }
                        else /* !started && !suspended */
                        {
                            decode_path_start(index, opts, &dolby_args, &resources);
                            playback_start(index, opts, &resources);
                        }

                        /* assertion - we are either started or suspended here */
                        if ( index == SECONDARY_DECODE && resources.audioDecoders[index] &&
                            (resources.audioProgram[index].codec == NEXUS_AudioCodec_eAc3Plus || resources.audioProgram[index].codec == NEXUS_AudioCodec_eAc3) )
                        {
                            if ( resources.suspended[index] )
                            {
                                NEXUS_AudioDecoderCodecSettings codecSettings;
                                NEXUS_AudioDecoder_GetCodecSettings(resources.audioDecoders[PRIMARY_DECODE], resources.audioProgram[PRIMARY_DECODE].codec, &codecSettings);
                                codecSettings.codecSettings.ac3Plus.enableAtmosProcessing = dolby_args.enableAtmos;
                                printf("enable ATMOS flag (secondary off):%d\n", codecSettings.codecSettings.ac3Plus.enableAtmosProcessing);
                                NEXUS_AudioDecoder_SetCodecSettings(resources.audioDecoders[PRIMARY_DECODE], &codecSettings);
                            }
                            else
                            {
                                NEXUS_AudioDecoderCodecSettings codecSettings;
                                NEXUS_AudioDecoder_GetCodecSettings(resources.audioDecoders[PRIMARY_DECODE], resources.audioProgram[PRIMARY_DECODE].codec, &codecSettings);
                                if( !dolby_args.dualMain )
                                {
                                    codecSettings.codecSettings.ac3Plus.enableAtmosProcessing = false;
                                }
                                printf("disable ATMOS flag (secondary on):%d\n", codecSettings.codecSettings.ac3Plus.enableAtmosProcessing);
                                NEXUS_AudioDecoder_SetCodecSettings(resources.audioDecoders[PRIMARY_DECODE], &codecSettings);
                            }
                        }
                    }
                    else
                    {
                        printf("Invalid decode %d, or no active decoder %p\n", index, index < NUM_DECODES ? (void *)resources.audioDecoders[index] : NULL);
                    }
                }
                else if ( strstr(buffer, "S ") != NULL )
                {
                    unsigned index;
                    char *value;
                    value = strchr(buffer, ' ');
                    *value = 0;
                    value++;

                    index = atoi(value);
                    printf("value '%s', %d\n", value, index);
                    if ((index < NUM_DECODES) && (resources.audioDecoders[index] != NULL))
                    {
                        printf("Toggle Start/Stop for Decode[%d] from %s to %s\n", index, resources.started[index] ? "STARTED" : "STOPPED", !resources.started[index] ? "STARTED" : "STOPPED");
                        if ( resources.started[index] )
                        {
                            NEXUS_AudioDecoder_Stop(resources.audioDecoders[index]);
                            resources.suspended[index] = false;
                            resources.started[index] = false;
                        }
                        else /* !started && !suspended */
                        {
                            decode_path_start(index, opts, &dolby_args, &resources);
                            #if 0 /* TBD do we need to start the playback */
                            playback_start(index, opts, &resources);
                            #endif
                        }

                        /* assertion - we are either started or suspended here */
                        /* but we could be stopped now */
                        if ( index == SECONDARY_DECODE && resources.audioDecoders[index] &&
                            (resources.audioProgram[index].codec == NEXUS_AudioCodec_eAc3Plus || resources.audioProgram[index].codec == NEXUS_AudioCodec_eAc3) )
                        {
                            if ( resources.suspended[index] ||  !resources.started[index] )
                            {
                                NEXUS_AudioDecoderCodecSettings codecSettings;
                                NEXUS_AudioDecoder_GetCodecSettings(resources.audioDecoders[PRIMARY_DECODE], resources.audioProgram[PRIMARY_DECODE].codec, &codecSettings);
                                codecSettings.codecSettings.ac3Plus.enableAtmosProcessing = dolby_args.enableAtmos;
                                printf("enable ATMOS flag (secondary off):%d\n", codecSettings.codecSettings.ac3Plus.enableAtmosProcessing);
                                NEXUS_AudioDecoder_SetCodecSettings(resources.audioDecoders[PRIMARY_DECODE], &codecSettings);
                            }
                            else
                            {
                                NEXUS_AudioDecoderCodecSettings codecSettings;
                                NEXUS_AudioDecoder_GetCodecSettings(resources.audioDecoders[PRIMARY_DECODE], resources.audioProgram[PRIMARY_DECODE].codec, &codecSettings);
                                if( !dolby_args.dualMain )
                                {
                                    codecSettings.codecSettings.ac3Plus.enableAtmosProcessing = false;
                                }
                                printf("disable ATMOS flag (secondary on):%d\n", codecSettings.codecSettings.ac3Plus.enableAtmosProcessing);
                                NEXUS_AudioDecoder_SetCodecSettings(resources.audioDecoders[PRIMARY_DECODE], &codecSettings);
                            }
                        }
                    }
                    else
                    {
                        printf("Invalid decode %d, or no active decoder %p\n", index, index < NUM_DECODES ? (void *)resources.audioDecoders[index] : NULL);
                    }
                }
                else if ( strstr(buffer, "r ") != NULL )
                {
                    unsigned index;
                    char *value;
                    value = strchr(buffer, ' ');
                    *value = 0;
                    value++;

                    index = atoi(value);
                    printf("value '%s', %d\n", value, index);
                    if ((index < NUM_DECODES) && (resources.audioDecoders[index] != NULL))
                    {
                        int ret;
                        bool wasStarted = resources.started[index];
                        bool wasSuspended = resources.suspended[index];
                        printf("Reconfigure Decode[%d] (started %d)\n", index, resources.started[index]);

                        if ( wasStarted || wasSuspended )
                        {
                            playback_stop(index, &resources);
                            decode_path_stop(index, &resources);
                        }

                        decode_path_shutdown(index, &resources, false);

                        /* populate new file, codec, pid */
                        if ( opts[index].common.audioPid != dolby_args.decodeAttributes[index].pid )
                        {
                            opts[index].common.audioPid = dolby_args.decodeAttributes[index].pid;
                        }
                        if ( opts[index].common.audioCodec != dolby_args.decodeAttributes[index].codec )
                        {
                            opts[index].common.audioCodec = dolby_args.decodeAttributes[index].codec;
                        }
                        if ( !opts[index].filename || strcmp(opts[index].filename, dolby_args.decodeAttributes[index].filename) != 0 )
                        {
                            opts[index].filename = dolby_args.decodeAttributes[index].filename;
                        }

                        if ( index == SECONDARY_DECODE && strlen(opts[index].filename) <= 0 && opts[index].common.audioPid != INVALID_PID )
                        {
                            opts[index].filename = opts[PRIMARY_DECODE].filename;
                        }

                        ret = decode_path_initialize(index, opts, &dolby_args, &resources, false);

                        if ( wasStarted && !wasSuspended && ret == 0 )
                        {
                            decode_path_start(index, opts, &dolby_args, &resources);
                            playback_start(index, opts, &resources);
                        }
                    }
                    else
                    {
                        printf("Invalid decode %d, or no active decoder %p\n", index, index < NUM_DECODES ? (void *)resources.audioDecoders[index] : NULL);
                    }
                }
                else if ( (dolby_args.enable_dap == 1) || (strstr(buf, "DAP_ENABLE") != NULL) || (strstr(buf, "DV") != NULL) ||
                          (strstr(buf, "IEQ") != NULL) || (strstr(buf, "DE") != NULL)|| (strstr(buf, "OUTPUTMODE") != NULL) ||
                          (strstr(buf, "RFMODE_DOWNMIX") != NULL) || (strstr(buf, "SECONDARY_SUBSTREAMID") != NULL) ||
                          (strstr(buf, "SECONDARY") != NULL) || (strstr(buf, "OUTPUTLFECH_MODE") != NULL) ||
                          (strstr(buf, "DUALMONO_MODE") != NULL) || (strstr(buf, "DOWNMIX_MODE") != NULL) ||
                          (strstr(buf, "RFMODE") != NULL) || (strstr(buf, "DRCCUT") != NULL) || (strstr(buf, "DRCBOOST") != NULL) ||
                          (strstr(buf, "enableCompressed") != NULL) || (strstr(buf, "MIXERBALANCE") != NULL) ||
                          (strstr(buf, "AC4PROGSELECT") != NULL) || (strstr(buf, "AC4PROGBALANCE") != NULL) ||
                          (strstr(buf, "AC4PRESID") != NULL) || (strstr(buf, "AC4DEAMOUNT") != NULL) ||
                          (strstr(buf, "FADETYPEPRI") != NULL) || (strstr(buf, "FADEDURATIONPRI") != NULL) || (strstr(buf, "FADELEVELPRI") != NULL) ||
                          (strstr(buf, "FADETYPESEC") != NULL) || (strstr(buf, "FADEDURATIONSEC") != NULL) || (strstr(buf, "FADELEVELSEC") != NULL) ||
                          (strstr(buf, "FADETYPESE") != NULL) || (strstr(buf, "FADEDURATIONSE") != NULL) || (strstr(buf, "FADELEVELSE") != NULL) ||
                          (strstr(buf, "FADETYPEAA") != NULL) || (strstr(buf, "FADEDURATIONAA") != NULL) || (strstr(buf, "FADELEVELAA") != NULL) ||
                          (strstr(buf, "FADETYPEOUT") != NULL) || (strstr(buf, "FADEDURATIONOUT") != NULL) || (strstr(buf, "FADELEVELOUT") != NULL) ||
                          (strstr(buf, "FILEPRIMARY") != NULL) || (strstr(buf, "CODECPRIMARY") != NULL) || (strstr(buf, "PIDPRIMARY") != NULL) ||
                          (strstr(buf, "FILESECONDARY") != NULL) || (strstr(buf, "CODECSECONDARY") != NULL) || (strstr(buf, "PIDSECONDARY") != NULL) ||
                          (strstr(buf, "FILEEFFECTS") != NULL) || (strstr(buf, "FILEAPPAUDIO") != NULL)
                        )
                {
                    set_config(buf, &dolby_args);

                    decode_path_update_settings(PRIMARY_DECODE, &dolby_args, &resources);

                    if ( resources.audioDecoders[SECONDARY_DECODE] )
                    {
                        decode_path_update_settings(SECONDARY_DECODE, &dolby_args, &resources);
                    }

                    /* Apply Fade Settings */
                    {
                        NEXUS_AudioMixerSettings mixerSettings;
                        unsigned i;
                        for ( i = 0; i < NUM_DECODES; i++ )
                        {
                            if ( resources.audioDecoders[i] && resources.started[i] )
                            {
                                NEXUS_AudioMixerInputSettings inputSettings;
                                NEXUS_AudioMixer_GetInputSettings(resources.mixer,
                                                                  NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[i], NEXUS_AudioConnectorType_eMultichannel),
                                                                  &inputSettings);
                                if ( inputSettings.fade.level != dolby_args.fadeLevel[i] )
                                {
                                    inputSettings.fade.level = dolby_args.fadeLevel[i];
                                    inputSettings.fade.type = dolby_args.fadeType[i];
                                    inputSettings.fade.duration = dolby_args.fadeDuration[i];
                                    NEXUS_AudioMixer_SetInputSettings(resources.mixer,
                                                                      NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[i], NEXUS_AudioConnectorType_eMultichannel),
                                                                      &inputSettings);
                                }
                            }
                        }

                        NEXUS_AudioMixer_GetSettings(resources.mixer, &mixerSettings);
                        if ( mixerSettings.mainDecodeFade.level != dolby_args.fadeLevel[OUTPUT_FADE_IDX] )
                        {
                            mixerSettings.mainDecodeFade.level = dolby_args.fadeLevel[OUTPUT_FADE_IDX];
                            mixerSettings.mainDecodeFade.type = dolby_args.fadeType[OUTPUT_FADE_IDX];
                            mixerSettings.mainDecodeFade.duration = dolby_args.fadeDuration[OUTPUT_FADE_IDX];
                            NEXUS_AudioMixer_SetSettings(resources.mixer, &mixerSettings);
                        }
                    }

                    {
                        NEXUS_AudioDecoderCodecSettings codecSettings;
                        printf("\n\n After changing the values, the new values are:\n");
                        NEXUS_AudioDecoder_GetCodecSettings(resources.audioDecoders[PRIMARY_DECODE], resources.audioProgram[PRIMARY_DECODE].codec, &codecSettings);
                        print_settings(dolby_args.decodeSettings, codecSettings, &dolby_args);
                    }
                }
                else if ( !*buf )
                {
                    /* allow blank line */
                }
                else
                {
                    printf("unknown command: %s\n", buf);
                }
            } while ( (buf = strtok(NULL, ";")) );
            #if B_HAS_PLAYBACK_MONITOR
            if ( stickyFile )
            {
                BKNI_ReleaseMutex(monitorState.lock);
            }
            #endif
        }
    }
    else
    {
        #if B_HAS_PLAYBACK_MONITOR
        if ( stickyFile )
        {
            BKNI_AcquireMutex(monitorState.lock);
        }
        #endif

        rc = NEXUS_Playback_GetStatus(resources.playback[PRIMARY_DECODE], &pstatus);
        BDBG_ASSERT(!rc);
        while ( pstatus.state != 2 )
        {
            rc = NEXUS_Playback_GetStatus(resources.playback[PRIMARY_DECODE], &pstatus);
            BDBG_ASSERT(!rc);

            BKNI_Sleep(100);
        }

        #if B_HAS_PLAYBACK_MONITOR
        if ( stickyFile )
        {
            BKNI_ReleaseMutex(monitorState.lock);
        }
        #endif

    }
    #if B_HAS_PLAYBACK_MONITOR
    if ( stickyFile )
    {
        monitor_thread_stop(&monitorState);
    }

    #endif
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------Wait for Captures to drain--------------------------------------------------------------------------------------------------------*/

    if ( resources.captureThread )
    {
        printf("Sleeping to allow paths to capture to drain\n");
        BKNI_Sleep(5000);
        audio_capture_callback(resources.captureEvent, 0);
        resources.captureDone = true;
        pthread_join(resources.captureThread, NULL);
    }

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------Stopping,Closing and Removing the Resources---------------------------------------------------------------------------------------*/

    #if NEXUS_NUM_AUDIO_PLAYBACKS
    if ( dolby_args.enablePlayback )
    {
        NEXUS_AudioPlayback_Stop(resources.audioPlayback);
        playback_thread_stop(&(resources.pcm_playbacks[0]));
    }
    #endif
    for ( i = (no_of_streams - 1); i >= 0; i-- )
    {
        playback_stop( i, &resources );
        decode_path_stop( i, &resources );
    }

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------Tear Down Capture Resources-------------------------------------------------------------------------------------------------------*/

    {
        unsigned c,p;

        for ( c = 0; c < NUM_CAPTURES; c++ )
        {
            if ( resources.audioCapture[c].audioCapture )
            {
                printf("Stop/Close audio capture %d\n", c);
                NEXUS_AudioCapture_Stop(resources.audioCapture[c].audioCapture);
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioCapture_GetConnector(resources.audioCapture[c].audioCapture));
                NEXUS_AudioCapture_Close(resources.audioCapture[c].audioCapture);
                resources.audioCapture[c].audioCapture = NULL;
            }

            for ( p = 0; p < 4; p++ )
            {
                if ( resources.audioCapture[c].file[p] )
                {
                    printf("Close file %d, audio capture %d\n", p, c);
                    fclose(resources.audioCapture[c].file[p]);
                    resources.audioCapture[c].file[p] = NULL;
                }
            }
        }

        if ( resources.captureEvent )
        {
            printf("Destroy Capture Event\n");
            BKNI_DestroyEvent(resources.captureEvent);
            resources.captureEvent = NULL;
        }
    }

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    if ( dolby_args.connectors.hdmi )
    {
        NEXUS_AudioOutput_RemoveAllInputs(dolby_args.connectors.hdmi);
    }
    if ( dolby_args.connectors.spdif )
    {
        NEXUS_AudioOutput_RemoveAllInputs(dolby_args.connectors.spdif);
    }
    if ( dolby_args.connectors.dac )
    {
        NEXUS_AudioOutput_RemoveAllInputs(dolby_args.connectors.dac);
    }
    if ( dolby_args.connectors.dummy )
    {
        NEXUS_AudioOutput_RemoveAllInputs(dolby_args.connectors.dummy);
    }
    NEXUS_DolbyDigitalReencode_Close(resources.ddre);
    resources.ddre = NULL;

    NEXUS_AudioMixer_RemoveAllInputs(resources.mixer);
    NEXUS_AudioMixer_Close(resources.mixer);
    resources.mixer = NULL;
    NEXUS_AudioMixer_RemoveAllInputs(resources.directMixer);
    NEXUS_AudioMixer_Close(resources.directMixer);
    resources.directMixer = NULL;
    NEXUS_AudioMixer_RemoveAllInputs(resources.directMchMixer);
    NEXUS_AudioMixer_Close(resources.directMchMixer);
    resources.directMchMixer = NULL;

    for ( i = 0; i < no_of_streams; i++ )
    {
        decode_path_shutdown( i, &resources, true );
    }

    for ( i = 0; i < no_of_streams; i++ )
    {
        if ( resources.audioDecoders[i] )
        {
            NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(resources.audioDecoders[i], NEXUS_AudioConnectorType_eMultichannel));
            NEXUS_AudioDecoder_Close(resources.audioDecoders[i]);
            resources.audioDecoders[i] = NULL;
        }
    }
    if ( resources.compressedDecoder )
    {
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(resources.compressedDecoder, NEXUS_AudioConnectorType_eCompressed));
        NEXUS_AudioDecoder_Close(resources.compressedDecoder);
        resources.compressedDecoder = NULL;
    }
    #if NEXUS_NUM_AUDIO_PLAYBACKS
    NEXUS_AudioPlayback_Close(resources.audioPlayback);
    #endif

    if ( file_params )
    {
        fclose(file_params);
    }

    if (resources.videoDecoder)
    {
        NEXUS_VideoDecoder_Close(resources.videoDecoder);
        resources.videoDecoder = NULL;
    }
    NEXUS_VideoWindow_Close(resources.window);
    resources.window = NULL;

    /* stop/remove HDMI callbacks associated with display,
    so those callbacks do not access display once it is removed */
    #if NEXUS_NUM_HDMI_OUTPUTS > 0
    if ( platformConfig.outputs.hdmi[0] )
    {
        NEXUS_StopCallbacks(platformConfig.outputs.hdmi[0]);
    }
    #endif
    NEXUS_Display_Close(resources.display);

    NEXUS_Platform_Uninit();
    #endif
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    return 0;
}
