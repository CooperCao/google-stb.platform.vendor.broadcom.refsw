/******************************************************************************
 *    (c)2008-2010 Broadcom Corporation
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

#ifdef BDSP_FW_RBUF_CAPTURE
#include "rbuf_capture.h"
#endif
BDBG_MODULE(playback);

/* Primary, Secondary, Sound Effects, Application Audio */
#define NUM_DECODES      4
#define PRIMARY_DECODE   0
#define SECONDARY_DECODE 1
#define EFFECTS_DECODE   2
#define APPAUDIO_DECODE  3

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
    bool enableDdre;
    bool enableSpdif;
    bool enableHdmi;
    bool enableDac;
    bool enablePlayback;
    bool enableCompressed;
    bool enableSecondary;
    bool enableSoundEffects;
    bool enableAppAudio;
    bool enableAtmos;
    NEXUS_DolbyDigitalReencodeProfile compression;
    int mixerBalance;
    unsigned cut;
    unsigned boost;
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

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------Managing the run time change into the config parameters--------------------------------------------------------------------------------*/

static void set_config(char *input, struct dolby_digital_plus_command_args *dolby)
{
    char *value;
    value = strchr(input, '=');
    *value = 0;
    value++;

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
        dolby->enableSecondary = true;
        if ( atoi(value) == 0 )
        {
            dolby->enableSecondary = false;
        }
    }
    else if ( !strcmp(input, "SOUNDEFFECTS") )
    {
        dolby->enableSoundEffects = true;
        if ( atoi(value) == 0 )
        {
            dolby->enableSoundEffects = false;
        }
    }
    else if ( !strcmp(input, "APPAUDIO") )
    {
        dolby->enableAppAudio = true;
        if ( atoi(value) == 0 )
        {
            dolby->enableAppAudio = false;
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
    printf("\n\n---------------- The current values of the configuration parameters are as follows: ----------------- \n\n");

    printf("\t OUTPUTMODE = %d\n", decodeSettings.outputMode);
    printf("\t OUTPUTLFECH_MODE = %d\n", decodeSettings.outputLfeMode);
    printf("\t DUALMONO_MODE = %d\n", decodeSettings.dualMonoMode);
    printf("\t DOWNMIX_MODE = %d\n", (codecSettings.codecSettings.ac3Plus.stereoDownmixMode));
    if ( codecSettings.codec == NEXUS_AudioCodec_eAc3 || codecSettings.codec == NEXUS_AudioCodec_eAc3Plus )
    {
        printf("\t RFMODE = %d\n", codecSettings.codecSettings.ac3Plus.drcMode);
        printf("\t RFMODE_DOWNMIX = %d\n", codecSettings.codecSettings.ac3Plus.drcModeDownmix);
        printf("\t DRCCUT = %d\n", codecSettings.codecSettings.ac3Plus.cut);
        printf("\t DRCBOOST = %d\n", codecSettings.codecSettings.ac3Plus.boost);
        printf("\t DRCCUT_DOWNMIX = %d\n", codecSettings.codecSettings.ac3Plus.cutDownmix);
        printf("\t DRCBOOST_DOWNMIX = %d\n", codecSettings.codecSettings.ac3Plus.boostDownmix);
        printf("\t SECONDARY_SUBSTREAMID = %d\n", codecSettings.codecSettings.ac3Plus.substreamId);
        printf("\t ATMOS = %d\n", codecSettings.codecSettings.ac3Plus.enableAtmosProcessing);
    }
    else
    {
        printf("\t RFMODE = %d\n", codecSettings.codecSettings.aacPlus.drcMode);
        printf("\t RFMODE_DOWNMIX = %d\n", codecSettings.codecSettings.aacPlus.drcMode);
        printf("\t DRCCUT = %d\n", codecSettings.codecSettings.aacPlus.cut);
        printf("\t DRCBOOST = %d\n", codecSettings.codecSettings.aacPlus.boost);
        printf("\t DRCCUT_DOWNMIX = %d\n", codecSettings.codecSettings.aacPlus.cut);
        printf("\t DRCBOOST_DOWNMIX = %d\n", codecSettings.codecSettings.aacPlus.boost);
    }

    printf("\t MIXERBALANCE = %d\n", dolby->mixerBalance);

    printf("\t DAP_ENABLE = %d\n", dolby->enable_dap);
    printf("\t DV = %d\n", dolby->dv_enable);
    printf("\t DE = %d\n", dolby->dde_enable);
    printf("\t DEBOOST = %d\n", dolby->ddeDialogBoost);
    printf("\t DECUT = %d\n", dolby->ddeContentCut);
    printf("\t IEQ = %d\n", dolby->ieq_enable);
    printf("\t DEQBAND = %d\n", dolby->deqNumBands);
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
    printf("\t DIRECT = %d\n", dolby->directMode);
    printf("\t SPDIF = %d\n", dolby->enableSpdif);
    printf("\t HDMI = %d\n", dolby->enableHdmi);
    printf("\t DAC = %d\n", dolby->enableDac);
    printf("\t DDRE = %d\n", dolby->enableDdre);
    printf("\t PCMPLAYBACK = %d\n", dolby->enablePlayback);
    printf("\t COMPRESSED = %d\n", dolby->enableCompressed);
    printf("\t SECONDARY = %d\n", dolby->enableSecondary);
    printf("\t SOUNDEFFECTS = %d\n", dolby->enableSoundEffects);
    printf("\t APPAUDIO = %d\n", dolby->enableAppAudio);

    printf("\t ATMOS = %d\n", dolby->enableAtmos);
    printf("\t COMPRESSION = %d\n", dolby->compression);
    printf("\t TARGET_SYNC_DISABLED = %d\n", dolby->targetSyncDisabled);
    printf("\t COMPLETE_FIRST_FRAME = %d\n", dolby->completeFirstFrame);
    /*printf("\n(1)OUTPUTMODE = %d\n(2)OUTPUTLFECH_MODE = %d\n(3)DUALMONO_MODE = %d\n(4)DOWNMIX_MODE = %d\n(5)RFMODE = %d\n(6)DRCCUT = %d\n(7)DRCBOOST = %d\n(8)enableCompressed = %d\n(9)SECONDARY_SUBSTREAMID = %d\n(10)MIXERBALANCE = %d\n\n",decodeSettings.outputMode,decodeSettings.outputLfeMode,decodeSettings.dualMonoMode,(codecSettings.codecSettings.ac3Plus.stereoDownmixMode),codecSettings.codecSettings.ac3Plus.drcMode,codecSettings.codecSettings.ac3Plus.cut,codecSettings.codecSettings.ac3Plus.boost,dolby->enableCompressed,codecSettings.codecSettings.ac3Plus.substreamId, dolby->mixerBalance);*/

    printf("\n\n To change the value of any config parameters, type the command as 'CONFIG PARAMETER NAME= VALUE' \n Type 'quit' to terminate the application. \n\n\n");
    return;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------Command line arguments parsing---------------------------------------------------------------------------------------------*/

int dolby_digital_plus_cmdline_parse(int argc, const char *argv[], struct util_opts_t *opts, struct dolby_digital_plus_command_args *dolby, int j)
{
    int i;
    unsigned audioPid = INVALID_PID, secondaryAudioPid = INVALID_PID;
    NEXUS_AudioCodec audioCodec = NEXUS_AudioCodec_eMax;

    dolby->loopback = 0;

    memset(opts, 0, sizeof(*opts));
    opts->common.transportType = NEXUS_TransportType_eMax;

    opts->common.audioCodec = NEXUS_AudioCodec_eMax;
    opts->common.contentMode = NEXUS_VideoWindowContentMode_eFull;
    opts->common.compressedAudio = false;
    opts->common.decodedAudio = true;
    opts->common.probe = true;
#if NEXUS_DTV_PLATFORM
    opts->common.displayType = NEXUS_DisplayType_eLvds;
    opts->common.displayFormat = NEXUS_VideoFormat_eCustom0;
#else
    opts->common.useCompositeOutput = true;
    opts->common.useComponentOutput = true;
    opts->common.displayFormat = NEXUS_VideoFormat_eNtsc;
    opts->common.displayType = NEXUS_DisplayType_eAuto;
#endif
    opts->stcChannelMaster = NEXUS_StcChannelAutoModeBehavior_eVideoMaster;
    opts->common.tsTimestampType = NEXUS_TransportTimestampType_eNone;
    opts->beginningOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    opts->endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    opts->common.playpumpTimestampReordering = true;
    opts->customFileIo = false;
    opts->playbackMonitor = false;

    for ( i = 1; i < argc; i++ )
    {
        if ( !strcmp(argv[i], "-probe") )
        {
            /* Deprecated, ignore */
        }
        else if ( !strcmp(argv[i], "-config") )
        {
            dolby->config_file = argv[++i];
            if ( j == PRIMARY_DECODE )
            {
                printf("\n You entered the config file name :%s\n", dolby->config_file);
            }
        }
        else if ( !strcmp(argv[i], "-loopback") )
        {
            dolby->loopback = 1;
            if ( j == PRIMARY_DECODE )
            {
                printf("\n You have enabled the loopback option\n");
            }
        }
        else if ( !strcmp(argv[i], "-app_audio") )
        {
            dolby->app_audio_stream = argv[++i];
        }
        else if ( !strcmp(argv[i], "-sound_effects") )
        {
            dolby->sound_effects_stream = argv[++i];
        }
        else if ( !strcmp(argv[i], "-pcm") )
        {
            dolby->pcm_stream = argv[++i];
        }
        else if ( !strcmp(argv[i], "-secondary") )
        {
            dolby->secondary_stream = argv[++i];
        }
        else if ( !strcmp(argv[i], "-primary") )
        {
            dolby->primary_stream = argv[++i];
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
        else if ( !strcmp(argv[i], "-pcr") && i + 1 < argc )
        {
            opts->common.pcrPid = strtoul(argv[++i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-external_pcm") && i + 1 < argc )
        {
            dolby->externalPcm = strtoul(argv[++i], NULL, 0) ? true : false;
            dolby->externalPcmSpecified = true;
        }
        else if ( !strcmp(argv[i], "-mpeg_type") && i + 1 < argc )
        {
            opts->common.transportType = lookup(g_transportTypeStrs, argv[++i]);
        }
        else if ( !strcmp(argv[i], "-target_sync_disabled") )
        {
            dolby->targetSyncDisabled = true;
        }
        else if ( !strcmp(argv[i], "-complete_first_frame") )
        {
            dolby->completeFirstFrame = true;
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

    if ( j == PRIMARY_DECODE )
    {
        opts->filename = dolby->primary_stream;
        if ( NULL == opts->filename )
        {
            printf("No primary stream selected.  Use -primary <filemane> to specify the stream name\n");
            return -1;
        }
        printf("\n You have selected the primary stream : %s \n\n", dolby->primary_stream);
        opts->common.audioPid = audioPid;
        opts->common.audioCodec = audioCodec;
    }
    else if ( j == SECONDARY_DECODE && (dolby->secondary_stream != NULL || secondaryAudioPid != INVALID_PID) )
    {
        opts->filename = dolby->secondary_stream;
        if ( NULL == opts->filename )
        {
            printf("No secondary stream selected.  Use -secondary <filemane> to specify the stream name\n");
            return -1;
        }
        printf("\n You have selected the secondary stream : %s \n\n", dolby->secondary_stream);
        opts->common.audioPid = secondaryAudioPid;
        opts->common.audioCodec = audioCodec;
    }
    else if ( j == EFFECTS_DECODE && dolby->sound_effects_stream )
    {
        opts->filename = dolby->sound_effects_stream;
        if ( NULL == opts->filename )
        {
            printf("No sound effects stream selected.  Use -sound_effects <filemane> to specify the stream name\n");
            return -1;
        }
        printf("\n You have selected sound effects stream : %s \n\n", dolby->sound_effects_stream);
    }
    else if ( j == APPAUDIO_DECODE && dolby->app_audio_stream )
    {
        opts->filename = dolby->app_audio_stream;
        if ( NULL == opts->filename )
        {
            printf("No application audio stream selected.  Use -app_audio <filemane> to specify the stream name\n");
            return -1;
        }
        printf("\n You have selected Application Audio stream : %s \n\n", dolby->app_audio_stream);
    }

    opts->common.displayType = NEXUS_DisplayType_eAuto;

    /* this allows the user to set: "-mpeg_type es -video_type mpeg" and forget the "-video 1" option */
    if ( opts->common.transportType == NEXUS_TransportType_eEs && !opts->common.videoPid && !opts->common.audioPid )
    {
        if ( g_isVideoEs )
        {
            opts->common.videoPid = 1;
        }
        else
        {
            opts->common.audioPid = 1;
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
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputSettings hdmiSettings;

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

static void translate_args_to_codec_settings(unsigned idx, NEXUS_AudioDecoderHandle *decodes, struct dolby_digital_plus_command_args *dolby, NEXUS_AudioDecoderCodecSettings *codecSettings)
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
        if ( idx == 0 && !decodes[SECONDARY_DECODE] )
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

static void translate_args_to_decoder_settings(unsigned idx, NEXUS_AudioDecoderHandle *decodes, struct dolby_digital_plus_command_args *dolby, NEXUS_AudioDecoderSettings * decoderSettings)
{
    BSTD_UNUSED(idx);
    BSTD_UNUSED(decodes);
    decoderSettings->outputMode = dolby->decodeSettings.outputMode;
    decoderSettings->outputLfeMode = dolby->decodeSettings.outputLfeMode;
    decoderSettings->dualMonoMode = dolby->decodeSettings.dualMonoMode;
}

int main(int argc, const char *argv[])
{
#if NEXUS_HAS_PLAYBACK
    NEXUS_PlatformSettings platformSettings;
    NEXUS_StcChannelHandle stcChannel[NUM_DECODES];
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle audioPidChannel = NULL, pcrPidChannel = NULL, videoPidChannel;

    NEXUS_AudioDecoderHandle audioDecoders[NUM_DECODES], compressedDecoder = NULL;
    bool started[NUM_DECODES];
    NEXUS_AudioMixerSettings mixerSettings;
    NEXUS_AudioMixerHandle mixer, directMixer, directMchMixer;
    NEXUS_AudioDecoderStartSettings audioProgram[NUM_DECODES];
#if NEXUS_NUM_AUDIO_PLAYBACKS
    NEXUS_AudioPlaybackHandle audioPlayback;
    pcm_playback_t pcm_playbacks[1];
#endif
    NEXUS_FilePlayHandle file[NUM_DECODES], customFile = NULL, stickyFile = NULL;
    NEXUS_PlaypumpHandle playpump[NUM_DECODES];
    NEXUS_PlaybackStatus pstatus;
    NEXUS_PlaybackHandle playback[NUM_DECODES];
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_PlaybackStartSettings playbackStartSettings;
    NEXUS_DolbyDigitalReencodeHandle ddre = NULL;
#if NEXUS_NUM_HDMI_OUTPUTS > 0
    NEXUS_HdmiOutputStatus hdmiStatus;
#endif
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_VideoWindowHandle window;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;

    /* command line overrides */
    NEXUS_AudioCodec audioCodec;
    unsigned audioPid, pcrPid;
    NEXUS_TransportType transportType;

    int i, no_of_streams = NUM_DECODES;
    NEXUS_Error rc;


    struct dolby_digital_plus_command_args dolby_args;
    struct util_opts_t opts[NUM_DECODES];

    FILE *file_params = NULL;

    bool exit;
#ifdef BDSP_FW_RBUF_CAPTURE
    brbuf_init_ringbuf_capture();
#endif


    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    BDBG_ASSERT(!rc);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Populate our defaults */
    BKNI_Memset(&dolby_args, 0, sizeof(dolby_args));

    /* Populate Default algo parameters */
    {
        NEXUS_AudioDecoderHandle tempDecoder = NEXUS_AudioDecoder_Open(0, NULL);
        BDBG_ASSERT(tempDecoder);
        NEXUS_AudioDecoder_GetCodecSettings(tempDecoder, NEXUS_AudioCodec_eAc3Plus, &dolby_args.ac3CodecSettings);
        NEXUS_AudioDecoder_GetCodecSettings(tempDecoder, NEXUS_AudioCodec_eAacPlus, &dolby_args.aacCodecSettings);
        NEXUS_AudioDecoder_Close(tempDecoder);
    }

    dolby_args.dialog_level = 31;
    dolby_args.enableDac = true;
    dolby_args.enableSpdif = true;
    dolby_args.enableHdmi = true;
    dolby_args.enableDdre = true;
    dolby_args.enableSecondary = true;
    dolby_args.enableSoundEffects = true;
    dolby_args.enableAppAudio = true;
    dolby_args.enableAtmos = true;
    dolby_args.compression = NEXUS_DolbyDigitalReencodeProfile_eFilmStandardCompression;
    dolby_args.cut = 100;
    dolby_args.boost = 100;
    /* Dapv2 parameters */
    dolby_args.deqNumBands = 10;

    Intialize_values(&dolby_args.deqFrequency[0], preset_freq[0], dolby_args.deqNumBands);
    Intialize_values((uint32_t *)&dolby_args.deqGain[0], preset_gain[0], dolby_args.deqNumBands);
/*-------------------------------------------------------------------Initialization----------------------------------------------------------------------------------------------------------------*/

    NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
    mixerSettings.mixUsingDsp = true;
    mixer = NEXUS_AudioMixer_Open(&mixerSettings);
    directMixer = NEXUS_AudioMixer_Open(NULL);
    directMchMixer = NEXUS_AudioMixer_Open(NULL);

#if NEXUS_NUM_AUDIO_PLAYBACKS
    audioPlayback = NEXUS_AudioPlayback_Open(0, NULL);
    if ( NULL == audioPlayback )
    {
        fprintf(stderr, "Unable to open playback channel\n");
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        BDBG_ASSERT(NEXUS_INVALID_PARAMETER);
    }
#endif

    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
#if NEXUS_NUM_HDMI_OUTPUTS > 0
    NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( hdmiStatus.connected )
    {
        displaySettings.format = hdmiStatus.preferredVideoFormat;
    }
#endif
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    for ( i = 0; i < no_of_streams; i++ )
    {
        NEXUS_PlaybackHandle pb;
        NEXUS_PlaypumpHandle pp;
        NEXUS_StcChannelHandle stc;
        bool requiresPlaypump = true;
        NEXUS_AudioDecoderOpenSettings decoderOpenSettings;

        if ( dolby_digital_plus_cmdline_parse(argc, argv, &opts[i], &dolby_args, i) )
        {
            return 0;
        }

        if ( i == PRIMARY_DECODE )
        {
            parse_config_file(&dolby_args, true);
        }

        /* explicitly disable secondary when requested */
        if ( i == SECONDARY_DECODE && !dolby_args.enableSecondary )
        {
            opts[i].filename = NULL;
        }

        if ( !opts[i].filename )
        {
            audioDecoders[i] = NULL;
            file[i] = NULL;
            playpump[i] = NULL;
            playback[i] = NULL;
            stcChannel[i] = NULL;
            continue;
        }
        else if ( i == SECONDARY_DECODE && strcmp(opts[SECONDARY_DECODE].filename, opts[PRIMARY_DECODE].filename) == 0 )
        {
            printf("SECONDARY is sharing PLAYBACK/PLAYPUMP with PRIMARY\n");
            requiresPlaypump = false;
            file[i] = NULL;
            playpump[i] = NULL;
            playback[i] = NULL;
            stcChannel[i] = NULL;
        }

        if ( requiresPlaypump )
        {
            /* Bring up all modules for a platform in a default configuraiton for this platform */
            playpump[i] = NEXUS_Playpump_Open(i, NULL);
            BDBG_ASSERT(playpump[i]);
            pp = playpump[i];
            playback[i] = NEXUS_Playback_Create();
            BDBG_ASSERT(playback[i]);
            pb = playback[i];
            NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
            stcSettings.timebase = NEXUS_Timebase_e0;
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;
            stcChannel[i] = NEXUS_StcChannel_Open(i, &stcSettings);
            BDBG_ASSERT(stcChannel[i]);
            stc = stcChannel[i];
        }
        else
        {
            pp = playpump[PRIMARY_DECODE];
            pb = playback[PRIMARY_DECODE];
            stc = stcChannel[PRIMARY_DECODE];
        }

        NEXUS_AudioDecoder_GetDefaultOpenSettings(&decoderOpenSettings);
        decoderOpenSettings.multichannelFormat = dolby_args.multiCh71 ? NEXUS_AudioMultichannelFormat_e7_1 : NEXUS_AudioMultichannelFormat_e5_1;
        BDBG_ERR(("---decoderOpenSettings.multichannelFormat %d", decoderOpenSettings.multichannelFormat));
        audioDecoders[i] = NEXUS_AudioDecoder_Open(i, &decoderOpenSettings);
        BDBG_ASSERT(audioDecoders[i]);

        if ( dolby_args.loopback == 1 )
        {
            opts[i].endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
        }
        else
        {
            opts[i].endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
        }

        /* save what we got from command line before we probe */
        printf("\n From command line - Stream[%d] audioCodec %d, pid %d, transportType %d", i, opts[i].common.audioCodec, opts[i].common.audioPid, opts[i].common.transportType);
        audioCodec = opts[i].common.audioCodec;
        audioPid = opts[i].common.audioPid;
        transportType = opts[i].common.transportType;
        pcrPid = opts[i].common.pcrPid;

        if ( cmdline_probe(&opts[i].common, opts[i].filename, &opts[i].indexname) )
        {
            return 1;
        }

        printf("PROBE of '%s' found:\n\t transport type %d, audio_codec %d\n",
               opts[i].filename, opts[i].common.transportType, opts[i].common.audioCodec);

        /* restore what we got from command line after probe */
        if ( audioCodec != NEXUS_AudioCodec_eMax )
        {
            printf("Overriding probed audio codec %d with command line specified codec %d\n", opts[i].common.audioCodec, audioCodec);
            opts[i].common.audioCodec = audioCodec;
        }
        if ( audioPid != INVALID_PID )
        {
            printf("Overriding probed audio pid %d with command line specified pid %d\n", opts[i].common.audioPid, audioPid);
            opts[i].common.audioPid = audioPid;
        }
        if ( transportType != NEXUS_TransportType_eMax )
        {
            printf("Overriding probed transport type %d with command line specified type %d\n", opts[i].common.transportType, transportType);
            opts[i].common.transportType = transportType;
        }
        if ( pcrPid != 0 )
        {
            printf("Overriding probed pcr pid %d with command line specified pid %d\n", opts[i].common.pcrPid, pcrPid);
            opts[i].common.pcrPid = pcrPid;
        }

        /* We have to parse again (silently) now that we have the audio codec from probe */
        if ( i == PRIMARY_DECODE )
        {
            /* store the primary audio codec here */
            dolby_args.audioCodec = opts[i].common.audioCodec;
            parse_config_file(&dolby_args, false);
        }

        if ( !opts[i].filename )
        {
            print_usage(argv[0]);
            return 0;
        }

        if ( (opts[i].indexname && !strcmp(opts[i].indexname, "same")) ||
             opts[i].common.transportType == NEXUS_TransportType_eMkv ||
             opts[i].common.transportType == NEXUS_TransportType_eMp4
            )
        {
            opts[i].indexname = opts[i].filename;
        }

        if ( requiresPlaypump )
        {
            file[i] = NEXUS_FilePlay_OpenPosix(opts[i].filename, opts[i].indexname);
            if ( !file[i] )
            {
                fprintf(stderr, "can't open files:%s %s\n", opts[i].filename, opts[i].indexname);
                return -1;
            }
            if ( opts[i].customFileIo )
            {
                customFile = file[i] = FileIoCustom_Attach(file[i]);
                BDBG_ASSERT(file[i]);
            }
            if ( opts[i].playbackMonitor )
            {
                stickyFile = file[i] = FileIoSticky_Attach(file[i]);
                BDBG_ASSERT(file[i]);
            }

            NEXUS_Playback_GetSettings(pb, &playbackSettings);
            playbackSettings.playpump = pp;
            playbackSettings.playpumpSettings.transportType = opts[i].common.transportType;
            playbackSettings.playpumpSettings.timestamp.pacing = false;
            playbackSettings.playpumpSettings.timestamp.type = opts[i].common.tsTimestampType;

            playbackSettings.stcChannel = stc;
            playbackSettings.stcTrick = opts[i].stcTrick;
            playbackSettings.beginningOfStreamAction = opts[i].beginningOfStreamAction;
            playbackSettings.endOfStreamAction = opts[i].endOfStreamAction;
            playbackSettings.enableStreamProcessing = opts[i].streamProcessing;
            rc = NEXUS_Playback_SetSettings(pb, &playbackSettings);
            BDBG_ASSERT(!rc);
        }

        if ( opts[i].common.audioCodec != NEXUS_AudioCodec_eUnknown )
        {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidSettings.pidTypeSettings.audio.primary = audioDecoders[i];
            playbackPidSettings.pidSettings.pidTypeSettings.audio.codec = opts[i].common.audioCodec;
            audioPidChannel = NEXUS_Playback_OpenPidChannel(pb, opts[i].common.audioPid, &playbackPidSettings);
        }

        if ( i == PRIMARY_DECODE && opts[i].common.videoCodec != NEXUS_VideoCodec_eUnknown )
        {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
            playbackPidSettings.pidTypeSettings.video.codec = opts[i].common.videoCodec;
            videoPidChannel = NEXUS_Playback_OpenPidChannel(pb, opts[i].common.videoPid, &playbackPidSettings);

            NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
            videoProgram.codec = opts[i].common.videoCodec;
            videoProgram.pidChannel = videoPidChannel;
            videoProgram.stcChannel = stc;
        }

        if ( opts[i].common.pcrPid && opts[i].common.pcrPid != opts[i].common.videoPid && opts[i].common.pcrPid != opts[i].common.audioPid )
        {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
            pcrPidChannel = NEXUS_Playback_OpenPidChannel(pb, opts[i].common.pcrPid, &playbackPidSettings);
        }

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up
        the audio outputs. */
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram[i]);

        audioProgram[i].targetSyncEnabled = !dolby_args.targetSyncDisabled;
        audioProgram[i].forceCompleteFirstFrame = dolby_args.completeFirstFrame;
        audioProgram[i].codec = opts[i].common.audioCodec;
        audioProgram[i].pidChannel = audioPidChannel;
        audioProgram[i].stcChannel = stc;
        if ( i==SECONDARY_DECODE )
        {
            audioProgram[i].secondaryDecoder = true;
        }
        else if ( i == EFFECTS_DECODE )
        {
            audioProgram[i].mixingMode = NEXUS_AudioDecoderMixingMode_eSoundEffects;
        }
        else if ( i == APPAUDIO_DECODE )
        {
            audioProgram[i].mixingMode = NEXUS_AudioDecoderMixingMode_eApplicationAudio;
        }
    }

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

    for ( i = 0; i < no_of_streams; i++ )
    {
        if ( audioDecoders[i] )
        {
            NEXUS_AudioDecoderSettings decoderSettings;
            NEXUS_AudioDecoderCodecSettings codecSettings;

            NEXUS_AudioDecoder_GetSettings(audioDecoders[i], &decoderSettings);
            translate_args_to_decoder_settings(i, audioDecoders, &dolby_args, &decoderSettings);
            NEXUS_AudioDecoder_SetSettings(audioDecoders[i], &decoderSettings);

            /* store start time settings (to be applied later) */
            audioProgram[i].targetSyncEnabled = !dolby_args.targetSyncDisabled;
            audioProgram[i].forceCompleteFirstFrame = dolby_args.completeFirstFrame;

            NEXUS_AudioDecoder_GetCodecSettings(audioDecoders[i], audioProgram[i].codec, &codecSettings);
            translate_args_to_codec_settings(i, audioDecoders, &dolby_args, &codecSettings);
            NEXUS_AudioDecoder_SetCodecSettings(audioDecoders[i], &codecSettings);
        }
    }

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
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif
#if NEXUS_NUM_COMPONENT_OUTPUTS
    if ( platformConfig.outputs.component[0] )
    {
        NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
    }
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    if ( displaySettings.format == NEXUS_VideoFormat_eNtsc )
    {
        NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
    }
#endif

    /* Add remaining components */
    {
        NEXUS_DolbyDigitalReencodeSettings ddreSettings;
        NEXUS_DolbyDigitalReencode_GetDefaultSettings(&ddreSettings);
        if ( dolby_args.multiCh71 )
        {
            ddreSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e7_1;
        }
        ddreSettings.profile = dolby_args.compression;
        ddreSettings.dialogLevel = dolby_args.dialog_level;
        if ( dolby_args.externalPcmSpecified )
        {
            ddreSettings.externalPcmMode = dolby_args.externalPcm;
        }
        else
        {
            ddreSettings.externalPcmMode = false;
            for ( i = 0; i < 2 && !ddreSettings.externalPcmMode; i++ )
            {
                switch ( audioProgram[i].codec )
                {
                case NEXUS_AudioCodec_eAc3:
                case NEXUS_AudioCodec_eAc3Plus:
                case NEXUS_AudioCodec_eAacAdts:
                case NEXUS_AudioCodec_eAacLoas:
                case NEXUS_AudioCodec_eAacPlusAdts:
                case NEXUS_AudioCodec_eAacPlusLoas:
                    break;
                default:
                    if ( audioDecoders[i] )
                    {
                        BDBG_WRN(("%s file is a non-dolby codec.  Forcing external PCM mode.", i == 0 ? "Primary" : "Secondary"));
                        ddreSettings.externalPcmMode = true;
                    }
                    break;
                }
            }
        }

        if ( ddreSettings.externalPcmMode == true )
        {
            ddreSettings.cut = dolby_args.cut;
            ddreSettings.boost = dolby_args.boost;
            BDBG_WRN(("Cut:%d Boost:%d \n",dolby_args.cut,dolby_args.boost));
        }

        ddreSettings.certificationMode = dolby_args.certificationMode_ddre;
        ddre = NEXUS_DolbyDigitalReencode_Open(&ddreSettings);
        BDBG_ASSERT(NULL != ddre);
    }


/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------Make our Connections--------------------------------------------------------------------------------------------------------------------------*/

    /* if secondary wasn't enabled, don't connect it */
    if ( dolby_args.secondary_stream == NULL )
    {
        dolby_args.enableSecondary = false;
    }

    /* if sound effects wasn't enabled, don't connect it */
    if ( dolby_args.sound_effects_stream == NULL )
    {
        dolby_args.enableSoundEffects = false;
    }

    /* if sound effects wasn't enabled, don't connect it */
    if ( dolby_args.app_audio_stream == NULL )
    {
        dolby_args.enableAppAudio = false;
    }

    if ( dolby_args.directMode )
    {
        /* Disable everything except primary and pcm playback in direct mode */
        dolby_args.enableSecondary = dolby_args.enableSoundEffects = dolby_args.enableAppAudio = false;

        printf("\n------------------------------------\n");
        printf("DIRECT Decoder->Output Mode connections:\n");
        printf("------------------------------------\n");
        #if NEXUS_NUM_HDMI_OUTPUTS > 0
        if ( dolby_args.enableHdmi )
        {
            printf("  HDMI %s PCM\n", (audioProgram[0].codec == NEXUS_AudioCodec_eAc3Plus && dolby_args.multiCh71) ? "7.1ch" : "5.1ch");
            rc = NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                                            NEXUS_AudioDecoder_GetConnector(audioDecoders[0], NEXUS_AudioConnectorType_eMultichannel));
            BDBG_ASSERT(NEXUS_SUCCESS == rc);
        }
        #endif
        if ( dolby_args.enableCompressed &&
             (audioProgram[0].codec == NEXUS_AudioCodec_eAc3Plus || audioProgram[0].codec == NEXUS_AudioCodec_eAc3) )
        {
            #if NEXUS_NUM_SPDIF_OUTPUTS > 0
            if ( dolby_args.enableSpdif )
            {
                printf("  SPDIF AC3 Compressed\n");
                rc = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                                                NEXUS_AudioDecoder_GetConnector(audioDecoders[0], NEXUS_AudioConnectorType_eCompressed));
                BDBG_ASSERT(NEXUS_SUCCESS == rc);
            }
            #endif
            #if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS > 0
            if ( !dolby_args.enableDac && !dolby_args.enableHdmi )
            {
                printf("  DUMMY PCM Stereo\n");
                rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]),
                                                NEXUS_AudioDecoder_GetConnector(audioDecoders[0], NEXUS_AudioConnectorType_eStereo));
                BDBG_ASSERT(NEXUS_SUCCESS == rc);
            }
            #endif
        }
        #if NEXUS_NUM_AUDIO_DACS > 0
        if ( dolby_args.enableDac )
        {
            printf("  DAC PCM Stereo\n");
            rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                                            NEXUS_AudioDecoder_GetConnector(audioDecoders[0], NEXUS_AudioConnectorType_eStereo));
            BDBG_ASSERT(NEXUS_SUCCESS == rc);
        }
        #endif
    }
    else
    {
        NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(audioDecoders[0], NEXUS_AudioConnectorType_eMultichannel));
        if ( dolby_args.enableSecondary )
        {
            NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(audioDecoders[1], NEXUS_AudioConnectorType_eMultichannel));
        }
        if ( dolby_args.enableSoundEffects )
        {
            NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(audioDecoders[2], NEXUS_AudioConnectorType_eMultichannel));
        }
        if ( dolby_args.enableAppAudio )
        {
            NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(audioDecoders[3], NEXUS_AudioConnectorType_eMultichannel));
        }
#if NEXUS_NUM_AUDIO_PLAYBACKS
        if ( dolby_args.enablePlayback )
        {
            NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioPlayback_GetConnector(audioPlayback));
        }
#endif

        /* Set the Mixer to use DSP mixing */
        NEXUS_AudioMixer_GetSettings(mixer, &mixerSettings);
        mixerSettings.master = NEXUS_AudioDecoder_GetConnector(audioDecoders[0], NEXUS_AudioDecoderConnectorType_eMultichannel);
        apply_mixer_settings(&mixerSettings, &dolby_args);
        NEXUS_AudioMixer_SetSettings(mixer, &mixerSettings);

        printf("\n------------------------------------\n");
        printf("Full MS12 Decoder->DspMixer->DDRE->Outputs Mode connections:\n");
        printf("------------------------------------\n");
        if ( dolby_args.enableDdre )
        {
            rc = NEXUS_DolbyDigitalReencode_AddInput(ddre, NEXUS_AudioMixer_GetConnector(mixer));
            BDBG_ASSERT(NEXUS_SUCCESS == rc);

#if NEXUS_NUM_AUDIO_DACS
            if ( dolby_args.enableDac )
            {
                printf("  DAC PCM Stereo\n");
                rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                                                NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioConnectorType_eStereo));
                BDBG_ASSERT(NEXUS_SUCCESS == rc);
            }
#endif
            if ( dolby_args.enableCompressed == false )
            {
#if NEXUS_NUM_SPDIF_OUTPUTS > 0
                if ( dolby_args.enableSpdif )
                {
                    printf("  SPDIF Ac3 Compressed\n");
                    rc = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                                                    NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioConnectorType_eStereo));
                    BDBG_ASSERT(NEXUS_SUCCESS == rc);
                }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS > 0
                if ( dolby_args.enableHdmi )
                {
                    printf("  HDMI %s PCM\n", (audioProgram[0].codec == NEXUS_AudioCodec_eAc3Plus && dolby_args.multiCh71) ? "7.1ch" : "5.1ch");
                    NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                                               NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioConnectorType_eMultichannel));
                }
#endif
            }
            else if ( dolby_args.enableCompressed == true )
            {
#if NEXUS_NUM_SPDIF_OUTPUTS > 0
                if ( dolby_args.enableSpdif )
                {
                    printf("  SPDIF Ac3 Compressed\n");
                    rc = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                                                    NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioConnectorType_eCompressed));
                    BDBG_ASSERT(NEXUS_SUCCESS == rc);
                }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS > 0
                if ( dolby_args.enableHdmi )
                {
                    if ( audioProgram[0].codec == NEXUS_AudioCodec_eAc3Plus || audioProgram[0].codec == NEXUS_AudioCodec_eAc3 ||
                         audioProgram[0].codec == NEXUS_AudioCodec_eAacAdts || audioProgram[0].codec == NEXUS_AudioCodec_eAacLoas ||
                         audioProgram[0].codec == NEXUS_AudioCodec_eAacPlusAdts || audioProgram[0].codec == NEXUS_AudioCodec_eAacPlusLoas )
                    {
                        printf("  HDMI DDP Compressed\n");
                        NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                                                   NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioConnectorType_eCompressed4x));
                    }
                    else
                    {
                        printf("  HDMI AC3 Compressed\n");
                        NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                                                   NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioConnectorType_eCompressed));
                    }
                    BDBG_ASSERT(NEXUS_SUCCESS == rc);
                }
#endif
            }
        }
        else
        {
#if NEXUS_NUM_HDMI_OUTPUTS > 0
            if ( dolby_args.enableHdmi )
            {
                printf("  HDMI %s PCM\n", (audioProgram[0].codec == NEXUS_AudioCodec_eAc3Plus && dolby_args.multiCh71) ? "7.1ch" : "5.1ch");
                NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                                           NEXUS_AudioMixer_GetConnector(mixer));
            }
#endif
        }
        printf("------------------------------------\n\n");
    }
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------Start decoders----------------------------------------------------------------------------------------------------*/

    for ( i = 0; i < no_of_streams; i++ )
    {
        NEXUS_AudioDecoderCodecSettings codecSettings;
        if ( audioDecoders[i] )
        {
            if ( videoDecoder && i == PRIMARY_DECODE )
            {
                NEXUS_HdmiOutputSettings hdmiSettings;
                NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
                hdmiSettings.hotplugCallback.callback = hotplug_callback;
                hdmiSettings.hotplugCallback.context = platformConfig.outputs.hdmi[0];
                hdmiSettings.hotplugCallback.param = (int)display;
                NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);

                /* initalize HDCP settings, keys, etc. */
                initializeHdmiOutputHdcpSettings();

                /* Force a hotplug to switch to preferred format */
                hotplug_callback(platformConfig.outputs.hdmi[0], (int)display);
                NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
            }

            NEXUS_AudioDecoder_GetCodecSettings(audioDecoders[i], audioProgram[i].codec, &codecSettings);
            translate_args_to_codec_settings(i, audioDecoders, &dolby_args, &codecSettings);
            NEXUS_AudioDecoder_SetCodecSettings(audioDecoders[i], &codecSettings);

            if ( i == PRIMARY_DECODE )
            {
                print_settings(dolby_args.decodeSettings, codecSettings, &dolby_args);
            }

            if ( (dolby_args.enableSecondary && i == SECONDARY_DECODE) ||
                 (dolby_args.enableSoundEffects && i == EFFECTS_DECODE) ||
                 (dolby_args.enableAppAudio && i == APPAUDIO_DECODE) ||
                 (i == PRIMARY_DECODE) )
            {
                printf("\n\n Starting decoder:%d \n\n", i);
                rc = NEXUS_AudioDecoder_Start(audioDecoders[i], &audioProgram[i]);
                BDBG_ASSERT(!rc);
                started[i] = true;
            }
#ifdef BDSP_FW_RBUF_CAPTURE
            start_rbuf_channels_from_env();
#endif

            if ( compressedDecoder && i == PRIMARY_DECODE )
            {
                printf("\n\n Starting Compressed decoder:%d \n\n", i);
                rc = NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram[i]);
                BDBG_ASSERT(!rc);
            }
        }
    }

    if ( !dolby_args.directMode )
    {
        NEXUS_AudioMixer_GetSettings(mixer, &mixerSettings);
        mixerSettings.dolby.multiStreamBalance = dolby_args.mixerBalance;
        NEXUS_AudioMixer_SetSettings(mixer, &mixerSettings);
    }

#if NEXUS_NUM_AUDIO_PLAYBACKS
    if ( dolby_args.enablePlayback )
    {
        NEXUS_AudioPlaybackStartSettings pbStartSettings;
        NEXUS_AudioPlaybackSettings pbSettings;

        BKNI_Memset(&(pcm_playbacks[0]), 0, sizeof(pcm_playbacks[0]));
        rc = BKNI_CreateEvent(&(pcm_playbacks[0].event));
        BDBG_ASSERT(!rc);
        pcm_playbacks[0].playback = audioPlayback;

        /* If we have a wav file, get the sample rate from it */
        if ( dolby_args.pcm_stream != NULL )
        {
            FILE *pFile;
            unsigned long dword, bytesToPlay;

            pFile = pcm_playbacks[0].pPcmFile = fopen(dolby_args.pcm_stream, "rb");
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
                    pcm_playbacks[0].pcmDataStart = ftell(pFile);
                    pcm_playbacks[0].pcmDataEnd = pcm_playbacks[0].pcmDataStart + wh.dataLen;
                    BDBG_ASSERT(pcm_playbacks[0].pcmDataEnd <= bytesToPlay);
                    pbStartSettings.sampleRate = wh.samplesSec;
                    pbStartSettings.bitsPerSample = wh.bps;
                    pbStartSettings.stereo = (wh.channels > 1);
                    pbStartSettings.signedData = (wh.bps != 8);    /* 8-bit .wav files are unsigned */
                    pbStartSettings.endian = NEXUS_EndianMode_eLittle;
                }

                pcm_playbacks[0].wh = wh;
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
        pbStartSettings.dataCallback.context = &(pcm_playbacks[0]);
        pbStartSettings.dataCallback.param = 0;
        rc = NEXUS_AudioPlayback_Start(pcm_playbacks[0].playback, &pbStartSettings);
        BDBG_ASSERT(!rc);

        printf("Calling playback_thread_start(%p), audioPlayback %p\n", (void *)&(pcm_playbacks[0]), (void *)audioPlayback);
        playback_thread_start(&(pcm_playbacks[0]));

        NEXUS_AudioPlayback_GetSettings(audioPlayback, &pbSettings);
        pbSettings.leftVolume = pbSettings.rightVolume = 0x200000;
        rc = NEXUS_AudioPlayback_SetSettings(audioPlayback, &pbSettings);
        BDBG_ASSERT(!rc);
    }
#endif

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    for ( i = 0; i < no_of_streams; i++ )
    {
        if ( playback[i] )
        {
            NEXUS_Playback_GetDefaultStartSettings(&playbackStartSettings);
            if ( opts[i].autoBitrate )
            {
                playbackStartSettings.mode = NEXUS_PlaybackMode_eAutoBitrate;
            }
            rc = NEXUS_Playback_Start(playback[i], file[i], &playbackStartSettings);
            BDBG_ASSERT(!rc);
        }
    }

/*----------------------------------------------------------------------Managing the runtime change of the config parameters----------------------------------------------------------------------------------------------------------*/
    if ( dolby_args.loopback == 1 )
    {
        for ( exit = false; !exit;)
        {
            char buffer[256];
            char *buf;

            printf("MS12_Interactive>"); fflush(stdout);
            fgets(buffer, 256, stdin);
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
                        "  s <index> - Toggle Stop/Start for a decode\n"
                        "  q - quit\n"
                        "  Parametername=value  - Change the value of any user configurable parameter\n"
                        );
                }
                else if ( !strcmp(buf, "q") )
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
                    if ((index < NUM_DECODES) && (audioDecoders[index] != NULL))
                    {
                        printf("Toggle Start/Stop for Decode[%d] from %s to %s\n", index, started[index] ? "STARTED" : "STOPPED", !started[index] ? "STARTED" : "STOPPED");
                        if ( started[index] )
                        {
                            if ( audioProgram[index].codec == NEXUS_AudioCodec_eAc3Plus || audioProgram[index].codec == NEXUS_AudioCodec_eAc3 )
                            {
                                if ( index == SECONDARY_DECODE && audioDecoders[index] )
                                {
                                    NEXUS_AudioDecoderCodecSettings codecSettings;
                                    NEXUS_AudioDecoder_GetCodecSettings(audioDecoders[PRIMARY_DECODE], audioProgram[PRIMARY_DECODE].codec, &codecSettings);
                                    codecSettings.codecSettings.ac3Plus.enableAtmosProcessing = dolby_args.enableAtmos;
                                    printf("enable ATMOS flag (secondary off):%d\n", codecSettings.codecSettings.ac3Plus.enableAtmosProcessing);
                                    NEXUS_AudioDecoder_SetCodecSettings(audioDecoders[PRIMARY_DECODE], &codecSettings);
                                }
                            }
                            NEXUS_AudioDecoder_Suspend(audioDecoders[index]);
                        }
                        else
                        {
                            if ( audioProgram[index].codec == NEXUS_AudioCodec_eAc3Plus || audioProgram[index].codec == NEXUS_AudioCodec_eAc3 )
                            {
                                if ( index == SECONDARY_DECODE && audioDecoders[index] )
                                {
                                    NEXUS_AudioDecoderCodecSettings codecSettings;
                                    NEXUS_AudioDecoder_GetCodecSettings(audioDecoders[PRIMARY_DECODE], audioProgram[PRIMARY_DECODE].codec, &codecSettings);
                                    codecSettings.codecSettings.ac3Plus.enableAtmosProcessing = false;
                                    printf("disable ATMOS flag (secondary on):%d\n", codecSettings.codecSettings.ac3Plus.enableAtmosProcessing);
                                    NEXUS_AudioDecoder_SetCodecSettings(audioDecoders[PRIMARY_DECODE], &codecSettings);
                                }
                            }
                            NEXUS_AudioDecoder_Resume(audioDecoders[index]);
                        }
                        started[index] = !started[index];
                    }
                    else
                    {
                        printf("Invalid decode %d, or no active decoder %p\n", index, index < NUM_DECODES ? (void *)audioDecoders[index] : NULL);
                    }
                }
                else if ( (dolby_args.enable_dap == 1) || (strstr(buf, "DAP_ENABLE") != NULL) || (strstr(buf, "DV") != NULL) || (strstr(buf, "IEQ") != NULL) || (strstr(buf, "DE") != NULL)|| (strstr(buf, "OUTPUTMODE") != NULL) || (strstr(buf, "RFMODE_DOWNMIX") != NULL) || (strstr(buf, "SECONDARY_SUBSTREAMID") != NULL) || (strstr(buf, "SECONDARY") != NULL) || (strstr(buf, "OUTPUTLFECH_MODE") != NULL) || (strstr(buf, "DUALMONO_MODE") != NULL) || (strstr(buf, "DOWNMIX_MODE") != NULL) || (strstr(buf, "RFMODE") != NULL) || (strstr(buf, "DRCCUT") != NULL) || (strstr(buf, "DRCBOOST") != NULL) || (strstr(buf, "enableCompressed") != NULL) || (strstr(buf, "MIXERBALANCE") != NULL) )
                {
                    NEXUS_AudioDecoderSettings decoderSettings;
                    NEXUS_AudioDecoderCodecSettings codecSettings;

                    set_config(buf, &dolby_args);

                    NEXUS_AudioDecoder_GetSettings(audioDecoders[PRIMARY_DECODE], &decoderSettings);
                    translate_args_to_decoder_settings(PRIMARY_DECODE, audioDecoders, &dolby_args, &decoderSettings);
                    NEXUS_AudioDecoder_SetSettings(audioDecoders[PRIMARY_DECODE], &decoderSettings);
                    NEXUS_AudioDecoder_GetCodecSettings(audioDecoders[PRIMARY_DECODE], audioProgram[PRIMARY_DECODE].codec, &codecSettings);
                    translate_args_to_codec_settings(PRIMARY_DECODE, audioDecoders, &dolby_args, &codecSettings);
                    NEXUS_AudioDecoder_SetCodecSettings(audioDecoders[PRIMARY_DECODE], &codecSettings);

                    if ( audioDecoders[SECONDARY_DECODE] )
                    {
                        /* Apply new Secondary Settings */
                        NEXUS_AudioDecoder_GetSettings(audioDecoders[SECONDARY_DECODE], &decoderSettings);
                        translate_args_to_decoder_settings(SECONDARY_DECODE, audioDecoders, &dolby_args, &decoderSettings);
                        NEXUS_AudioDecoder_SetSettings(audioDecoders[SECONDARY_DECODE], &decoderSettings);
                        NEXUS_AudioDecoder_GetCodecSettings(audioDecoders[SECONDARY_DECODE], audioProgram[SECONDARY_DECODE].codec, &codecSettings);
                        translate_args_to_codec_settings(SECONDARY_DECODE, audioDecoders, &dolby_args, &codecSettings);
                        NEXUS_AudioDecoder_SetCodecSettings(audioDecoders[SECONDARY_DECODE], &codecSettings);
                    }

                    NEXUS_AudioMixer_GetSettings(mixer, &mixerSettings);
                    mixerSettings.dolby.multiStreamBalance = dolby_args.mixerBalance;
                    apply_mixer_settings(&mixerSettings, &dolby_args);
                    NEXUS_AudioMixer_SetSettings(mixer, &mixerSettings);

                    printf("\n\n After changing the values, the new values are:\n");
                    print_settings(dolby_args.decodeSettings, codecSettings, &dolby_args);
                }
                else if ( !*buf )
                {
                    /* allow blank line */
                }
                else
                {
                    printf("unknown command: %s\n", buf);
                }
            }while ( (buf = strtok(NULL, ";")) );
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

        rc = NEXUS_Playback_GetStatus(playback[PRIMARY_DECODE], &pstatus);
        BDBG_ASSERT(!rc);
        while ( pstatus.state != 2 )
        {
            rc = NEXUS_Playback_GetStatus(playback[PRIMARY_DECODE], &pstatus);
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

/*----------------------------------------------------------------Stopping,Closing and Removing the Resources---------------------------------------------------------------------------------------*/

#if NEXUS_NUM_AUDIO_PLAYBACKS
    if ( dolby_args.enablePlayback )
    {
        NEXUS_AudioPlayback_Stop(audioPlayback);
        playback_thread_stop(&(pcm_playbacks[0]));
    }
#endif
    for ( i = (no_of_streams - 1); i >= 0; i-- )
    {
        if ( playback[i] )
        {
            printf("stop playback[%d] %p\n", i, (void *)playback[i]);
            NEXUS_Playback_Stop(playback[i]);
        }
        if ( audioDecoders[i] )
        {
            printf("stop audioDecoders[%d] %p\n", i, (void *)audioDecoders[i]);
            NEXUS_AudioDecoder_Stop(audioDecoders[i]);
        }
    }

    NEXUS_VideoDecoder_Stop(videoDecoder);

    if ( compressedDecoder )
    {
        NEXUS_AudioDecoder_Stop(compressedDecoder);
    }

    for ( i = 0; i < no_of_streams; i++ )
    {
        if ( playback[i] )
        {
            NEXUS_Playback_CloseAllPidChannels(playback[i]);
        }
        if ( file[i] )
        {
            NEXUS_FilePlay_Close(file[i]);
        }
        if ( playback[i] )
        {
            printf("destroy playback[%d] %p\n", i, (void *)playback[i]);
            NEXUS_Playback_Destroy(playback[i]);
        }
        if ( playpump[i] )
        {
            printf("destroy playpump[%d] %p\n", i, (void *)playpump[i]);
            NEXUS_Playpump_Close(playpump[i]);
        }
    }

    #if NEXUS_NUM_HDMI_OUTPUTS > 0
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
    #endif
    #if NEXUS_NUM_SPDIF_OUTPUTS > 0
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
    #endif
    #if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
    #endif
    #if NEXUS_NUM_AUDIO_DUMMY_OUTPUTS > 0
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]));
    #endif
    NEXUS_DolbyDigitalReencode_Close(ddre);

    NEXUS_AudioMixer_RemoveAllInputs(mixer);
    NEXUS_AudioMixer_Close(mixer);
    NEXUS_AudioMixer_RemoveAllInputs(directMixer);
    NEXUS_AudioMixer_Close(directMixer);
    NEXUS_AudioMixer_RemoveAllInputs(directMchMixer);
    NEXUS_AudioMixer_Close(directMchMixer);

    for ( i = 0; i < no_of_streams; i++ )
    {
        if ( audioDecoders[i] )
        {
            NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(audioDecoders[i], NEXUS_AudioConnectorType_eMultichannel));
            NEXUS_AudioDecoder_Close(audioDecoders[i]);
        }
    }
    if ( compressedDecoder )
    {
        NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioConnectorType_eCompressed));
        NEXUS_AudioDecoder_Close(compressedDecoder);
    }
#ifdef BDSP_FW_RBUF_CAPTURE
    sleep(4);
#endif
#if NEXUS_NUM_AUDIO_PLAYBACKS
    NEXUS_AudioPlayback_Close(audioPlayback);
#endif

    if ( file_params )
    {
        fclose(file_params);
    }

    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_VideoWindow_Close(window);

    /* stop/remove HDMI callbacks associated with display,
    so those callbacks do not access display once it is removed */
    #if NEXUS_NUM_HDMI_OUTPUTS > 0
    NEXUS_StopCallbacks(platformConfig.outputs.hdmi[0]);
    #endif
    NEXUS_Display_Close(display);
#ifdef BDSP_FW_RBUF_CAPTURE
    brbuf_uninit_ringbuf_capture();
#endif

    NEXUS_Platform_Uninit();
#endif
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    return 0;
}

/* Ring buffer capture specific functions */
#ifdef BDSP_FW_RBUF_CAPTURE
/* ******************************** */

static
void
rbuf_capture_assert_fail(char *statement,
                         char *file,
                         int line,
                         ...)
{
    printf("\n--RBUF-- ASSERT FAILED! '%s' {%s:%d}\n", statement, file, line);
    return;
}
static
void
rbuf_capture_alert_fail(char *statement,
                        char *file,
                        int line,
                        ...)
{
    printf("\n--RBUF-- ALERT! '%s' {%s:%d}\n", statement, file, line);
    return;
}

static
/*inline*/
int32_t
rbuf_capture_device_open(int8_t *device_name)
{
    RBUF_ASSERT(NULL != device_name, "0x%08X", device_name);

    if ( rbuf_capture_app_player )
    {
        /* Create / append to file*/
        return open((char *)device_name,
                    (O_CREAT | O_WRONLY | O_APPEND),
                    ((S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH | S_IWOTH)));
    }
    else
    {
        /* Create / overwrite file*/
        return open((char *)device_name,
                    (O_CREAT | O_WRONLY | O_TRUNC | O_DIRECT),
                    ((S_IRUSR | S_IWUSR) | (S_IRGRP | S_IWGRP) | (S_IROTH | S_IWOTH)));
    }
}

static
/*inline*/
int32_t
rbuf_capture_device_write(int32_t device, void *buf, int32_t count)
{
    return write(device, buf, count);
}

static
/*inline*/
int32_t
rbuf_capture_device_close(int32_t device)
{
    return close(device);
}

static
char*
rbuf_capture_channel_get_type_string(uint32_t path,
                                     uint32_t channel,
                                     uint32_t rbuf_id,
                                     BDSP_P_RbufType type)
{
    static char buf[128];
    char *str;

    switch ( type )
    {
    case BDSP_P_RbufType_eDecode:
        {
            static char *dec = "DECODER CHANNEL";
            str = dec;
            break;
        }
    case BDSP_P_RbufType_ePassthru:
        {
            static char *pass = "PASSTHRU CHANNEL";
            str = pass;
            break;
        }
    case BDSP_P_RbufType_eMixer:
        {
            static char *mix = "MIXER CHANNEL";
            str = mix;
            break;
        }
    case BDSP_P_RbufType_eEncoder:
        {
            static char *enc = "ENCODER CHANNEL";
            str = enc;
            break;
        }
    case BDSP_P_RbufType_eTranscode:
        {
            static char *tran = "TRANSCODE CHANNEL";
            str = tran;
            break;
        }
    case BDSP_P_RbufType_eSM:
        {
            static char *spkr = "SPKR MGMT CHANNEL";
            str = spkr;
            break;
        }
    default:
        {
            RBUF_ASSERT(0, "");
            str = NULL;
            break;
        }
    }
    if ( rbuf_id != RBUF_ID_INVALID )
    {
        sprintf(buf, "PATH[%d]:%s[%d][%d]", path, str, channel, rbuf_id);
    }
    else
    {
        sprintf(buf, "PATH[%d]:%s[%d]", path, str, channel);
    }
    return buf;
}

static
char*
rbuf_capture_channel_get_info_string(uint32_t path,
                                     uint32_t channel,
                                     BDSP_P_RbufType type,
                                     uint32_t rbuf_id)
{
    static char buf[128];

    sprintf(buf, "%s",
            rbuf_capture_channel_get_type_string(path, channel, rbuf_id, type));

    return buf;
}

static
void
rbuf_capture_alert(rbuf_capture_channel_t *cap)
{
    RBUF_PRINT(("******************************************************************"));
    RBUF_PRINT(("*                     !!!! ALERT !!!!"));
    RBUF_PRINT(("* Error detected for %s",
                rbuf_capture_channel_get_info_string(cap->path,
                                                     cap->channel,
                                                     cap->type,
                                                     cap->rbuf_id)));

    /* Inactivate this channel*/
    cap->active = 0;

    RBUF_PRINT(("* Capture HAS NOW BEEN DISABLED for this ring buffer!!"));
    RBUF_PRINT(("******************************************************************"));

    return;
}

static
uint32_t
rbuf_capture_from_cap_q_to_disk(rbuf_capture_channel_t *cap)
{
    uint8_t *cap_q_ptr;
    uint32_t cap_q_r_idx = 0;
    uint32_t cap_q_w_idx = 0;
    uint32_t cap_q_num_entries = 0;
    uint32_t bytes_captured = 0;
    uint32_t aligned_entries = 0;
    RBUF_CAPTURE_CHANNEL_STATE cap_q_to_disk_state;

    cap_q_to_disk_state = cap->cap_q_to_disk_state;

    if ( !cap->active )
    {
        /* Channel is not active, nothing to do!*/
        bytes_captured = 0;
        return bytes_captured;
    }

    if ( (RBUF_CAPTURE_CHANNEL_STATE_INACTIVE == cap_q_to_disk_state) ||
         (RBUF_CAPTURE_CHANNEL_STATE_STOPPED == cap_q_to_disk_state) )
    {
        /* Nothing to do*/
        bytes_captured = 0;
        return bytes_captured;
    }

    if ( RBUF_CAPTURE_CHANNEL_STATE_PEND_STOP == cap_q_to_disk_state )
    {
        /* Transition to stopped state*/
        cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
        bytes_captured = 0;
        return bytes_captured;
    }

    cap_q_ptr = cap->q_ptr;
    cap_q_r_idx = cap->q_r_idx;
    cap_q_w_idx = cap->q_w_idx;
    cap_q_num_entries = cap->q_num_entries;

    if ( RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE == cap_q_to_disk_state )
    {
        /* If both the Capture Q to disk is empty and the RBUF to Captre Q is*/
        /* stopped, then the output is stopped*/
        if ( (Q_IS_EMPTY(cap_q_r_idx, cap_q_w_idx)) &&
             (RBUF_CAPTURE_CHANNEL_STATE_STOPPED == cap->rbuf_to_cap_q_state) )
        {
            /* Transition to stopped state*/
            cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;

            RBUF_DEBUG(("%s - DISK All data consumed",
                        rbuf_capture_channel_get_info_string(cap->path,
                                                             cap->channel,
                                                             cap->type,
                                                             cap->rbuf_id)));

            bytes_captured = 0;
            return bytes_captured;
        }
    }

    if ( (RBUF_CAPTURE_CHANNEL_STATE_ACTIVE == cap_q_to_disk_state) ||
         (RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE == cap_q_to_disk_state) )
    {
        if ( cap->q_w_idx > cap->q_r_idx )
        {
            if ( (cap->q_w_idx - cap->q_r_idx) >= RBUF_PAGE_SIZE )
            {
                aligned_entries = (((cap->q_w_idx - cap->q_r_idx) / RBUF_PAGE_SIZE) * RBUF_PAGE_SIZE);
                cap->rbuf_flush_onexit = false;
            }
            else
            {
                cap->rbuf_flush_onexit = true;
                bytes_captured = 0;
            }
        }
        else
        {
            if ( cap->q_w_idx != cap->q_r_idx )
            {
                aligned_entries = (RBUF_CAP_MAX_CHAN_ENTRIES - cap->q_r_idx);
            }
        }
        if ( !cap->rbuf_flush_onexit )
        {
            bytes_captured = rbuf_capture_device_write(cap->file, &cap_q_ptr[cap_q_r_idx], aligned_entries);
            cap->q_r_idx += aligned_entries;

            if ( cap->q_r_idx >= RBUF_CAP_MAX_CHAN_ENTRIES )
            {
                cap->q_r_idx = 0;
            }
        }
    }
    return bytes_captured;
}

static
void
rbuf_capture_rbuf_copy(rbuf_capture_channel_t *cap,
                       uint8_t *rbuf_ptr,
                       uint32_t *rbuf_w_idx,
                       uint32_t rbuf_num_entries,
                       uint32_t rbuf_copy_bytes,

                       uint8_t *cap_q_ptr,
                       uint32_t *cap_q_w_idx,
                       uint32_t cap_q_num_entries,
                       uint32_t *cap_q_avail_to_end_bytes,
                       uint32_t *cap_q_avail_from_start_bytes)
{
    uint32_t copy_bytes;
    uint32_t _cap_q_w_idx;
    uint32_t _rbuf_w_idx;

    _cap_q_w_idx = *cap_q_w_idx;
    _rbuf_w_idx = *rbuf_w_idx;

    /* Does the RBUF number of bytes fit into the capture Q's to end?*/
    if ( rbuf_copy_bytes <= *cap_q_avail_to_end_bytes )
    {
        /* RBUF segment will fit in capture Q to end (without wrap)*/
        copy_bytes = rbuf_copy_bytes;
        *cap_q_avail_to_end_bytes -= copy_bytes;
    }
    else
    {
        /* RBUF to end will NOT fit in capture Q to end (without wrap)*/

        if ( *cap_q_avail_to_end_bytes )
        {
            /* Fill to the capture Q wrap point*/
            copy_bytes = *cap_q_avail_to_end_bytes;

            RBUF_ASSERT((((_cap_q_w_idx + copy_bytes) <= cap_q_num_entries) && ((_rbuf_w_idx + copy_bytes) <= rbuf_num_entries)),
                        "%s - 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X",
                        rbuf_capture_channel_get_info_string(cap->path,
                                                             cap->channel,
                                                             cap->type,
                                                             cap->rbuf_id),
                        _cap_q_w_idx, cap_q_num_entries,
                        _rbuf_w_idx, rbuf_num_entries, copy_bytes);

            /* Invalidate the cache to cause burst read for memcpy*/
            /* NOTE:Data will never be written to the ring buffer by the MIPS*/
            /*      so the flush has no effect other than invalidate*/
            NEXUS_Memory_FlushCache(&rbuf_ptr[_rbuf_w_idx], copy_bytes);

            BKNI_Memcpy(&cap_q_ptr[_cap_q_w_idx], &rbuf_ptr[_rbuf_w_idx], copy_bytes);

            /* Should wrap to start since all copy to end bytes have been consumed*/
            RBUF_ASSERT((0 == Q_INC(_cap_q_w_idx, copy_bytes, 0, cap_q_num_entries)), "%s - 0x%08X 0x%08X 0x%08X",
                        rbuf_capture_channel_get_info_string(cap->path,
                                                             cap->channel,
                                                             cap->type,
                                                             cap->rbuf_id),
                        _cap_q_w_idx, copy_bytes, cap_q_num_entries);

            _cap_q_w_idx = 0;
            /* No more end bytes available*/
            *cap_q_avail_to_end_bytes = 0;

            /* Can just increment, RBUF guaranteed to not wrap*/
            _rbuf_w_idx += copy_bytes;
            /* Now setup copy of remaining RBUF end bytes to the beginning*/
            /* of the capture Q*/
            copy_bytes = rbuf_copy_bytes - copy_bytes;
            *cap_q_avail_from_start_bytes -= copy_bytes;
        }
        else
        {
            /* No space at to the end of the Q, simply copy at the start*/

            copy_bytes = rbuf_copy_bytes;
            RBUF_ASSERT((*cap_q_avail_from_start_bytes >= copy_bytes), "%s - 0x%08X 0x%08X",
                        rbuf_capture_channel_get_info_string(cap->path,
                                                             cap->channel,
                                                             cap->type,
                                                             cap->rbuf_id),
                        *cap_q_avail_from_start_bytes, copy_bytes);
            *cap_q_avail_from_start_bytes -= copy_bytes;
        }
    }
    RBUF_ASSERT((((_cap_q_w_idx + copy_bytes) <= cap_q_num_entries) && ((_rbuf_w_idx + copy_bytes) <= rbuf_num_entries)),
                "%s - 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X",
                rbuf_capture_channel_get_info_string(cap->path,
                                                     cap->channel,
                                                     cap->type,
                                                     cap->rbuf_id),
                _cap_q_w_idx, cap_q_num_entries,
                _rbuf_w_idx, rbuf_num_entries, copy_bytes);

    /* Invalidate the cache to cause burst read for memcpy*/
    /* NOTE:Data will never be written to the ring buffer by the MIPS*/
    /*      so the flush has no effect other than invalidate*/
    NEXUS_Memory_FlushCache(&rbuf_ptr[_rbuf_w_idx], copy_bytes);

    BKNI_Memcpy(&cap_q_ptr[_cap_q_w_idx], &rbuf_ptr[_rbuf_w_idx], copy_bytes);

    _cap_q_w_idx = Q_INC(_cap_q_w_idx, copy_bytes, 0, cap_q_num_entries);
    *cap_q_w_idx = _cap_q_w_idx;

    _rbuf_w_idx = Q_INC(_rbuf_w_idx, copy_bytes, 0, rbuf_num_entries);
    *rbuf_w_idx = _rbuf_w_idx;

    return;
}

static
void
rbuf_capture_from_rbuf_to_cap_q(rbuf_capture_channel_t *cap)
{
    uint8_t *rbuf_base_virt;
    uint32_t rbuf_base_phys;
    uint32_t rbuf_r_idx;
    uint32_t rbuf_w_idx;
    uint32_t rbuf_num_entries;
    uint32_t rbuf_copy_bytes;

    uint32_t rbuf_copy_to_end_bytes;
    uint32_t rbuf_copy_from_start_bytes;

    uint8_t *cap_q_ptr;
    uint32_t cap_q_r_idx;
    uint32_t cap_q_w_idx;
    uint32_t cap_q_num_entries;

    uint32_t cap_q_avail_to_end_bytes;
    uint32_t cap_q_avail_from_start_bytes;
    uint32_t cap_q_avail_total_bytes;

    uint32_t rbuf_cur_wraddr;
    uint32_t wr_cur_addr;
    uint32_t wr_prev_addr;
    uint32_t wr_cur_wrap;
    uint32_t wr_prev_wrap;

    uint32_t rbuf_cur_rdaddr;
    uint32_t rd_cur_wrap;
    uint32_t rd_prev_wrap;
    uint32_t rd_cur_addr;
    uint32_t rd_prev_addr;

    if ( !cap->active )
    {
        /* Channel is not active, nothing to do!*/
        return;
    }
    if ( (RBUF_CAPTURE_CHANNEL_STATE_INACTIVE == cap->rbuf_to_cap_q_state) ||
         (RBUF_CAPTURE_CHANNEL_STATE_STOPPED == cap->rbuf_to_cap_q_state) )
    {
        /* Nothing to do*/
        return;
    }
    if ( RBUF_CAPTURE_CHANNEL_STATE_PEND_STOP == cap->rbuf_to_cap_q_state )
    {
        /* Transition to stopped state*/
        cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
        return;
    }

    /* Check whether to initialize the ring buffer state.  Behavior is*/
    /* as follows:*/
    /*   Decoder / Passthru RBUFs:*/
    /*     1. Ring buffer registers are initialized with default values*/
    /*     2. Decoder adjusts write / end / start write addrss registers*/
    /*        NOTE::Start write must be greater than or equal to end address register*/
    /*     3. Decoder sets start write address register to less than end address*/
    /*     4. Initial write address shall be the start write address and the */
    /*        end address will be then be read to deterime size of RBUF.*/
    /*   Mixer RBUFs:*/
    /*     1. Ring buffer registers are initialized with default values*/
    /*     2. Write address WILL BE equal to base address register until*/
    /*        the final RBUF size is determined (end address register is written)*/
    /*     3. Once write address register does not equal base address, read*/
    /*        end address to determine size of RBUF.*/
    if ( !cap->rbuf_initialized )
    {
        uint32_t baseaddr;
        uint32_t endaddr;
        uint32_t freefull;
        uint32_t wrpoint;
        uint32_t entries;
        void *virt;
        int32_t result;

        /* If channel has never been initialized, it is still*/
        /* necessary to check for request to stop*/
        if ( RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE == cap->rbuf_to_cap_q_state )
        {
            cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
            RBUF_DEBUG(("%s - RBUF stopping before initialized!",
                        rbuf_capture_channel_get_info_string(cap->path,
                                                             cap->channel,
                                                             cap->type,
                                                             cap->rbuf_id)));
            return;
        }

        /* AUD_FMM_BF_CTRL_RINGBUF_X_RDADDR */
        rbuf_cur_rdaddr = (uint32_t)BREG_Read32(rbuf_reg_handle,
                                                cap->rbuf_reg_base + 0x00);
        /* AUD_FMM_BF_CTRL_RINGBUF_X_WRADDR */
        rbuf_cur_wraddr = (uint32_t)BREG_Read32(rbuf_reg_handle,
                                                cap->rbuf_reg_base + 0x04);
        /* AUD_FMM_BF_CTRL_RINGBUF_X_BASEADDR */
        baseaddr = (uint32_t)BREG_Read32(rbuf_reg_handle,
                                         cap->rbuf_reg_base + 0x08);
        /* AUD_FMM_BF_CTRL_RINGBUF_X_ENDADDR */
        endaddr = (uint32_t)BREG_Read32(rbuf_reg_handle,
                                        cap->rbuf_reg_base + 0x0C);
        /* AUD_FMM_BF_CTRL_RINGBUF_X_FREEFULL_MARK*/
        freefull = (uint32_t)BREG_Read32(rbuf_reg_handle,
                                         cap->rbuf_reg_base + 0x10);
        /* AUD_FMM_BF_CTRL_RINGBUF_X_START_WRPOINT*/
        wrpoint = (uint32_t)BREG_Read32(rbuf_reg_handle,
                                        cap->rbuf_reg_base + 0x14);

        RBUF_DEBUG(("%s - %08X r=%08X w=%08X b=%08X e=%08X f=%08X w=%08X",
                    rbuf_capture_channel_get_info_string(cap->path,
                                                         cap->channel,
                                                         cap->type,
                                                         cap->rbuf_id),
                    cap->rbuf_reg_base,
                    rbuf_cur_rdaddr,
                    rbuf_cur_wraddr,
                    baseaddr,
                    endaddr,
                    freefull,
                    wrpoint));

        /* Registers default to zero when uninitialized*/
        if ( (0 == rbuf_cur_rdaddr) || (0 == rbuf_cur_wraddr) || (0 == baseaddr) ||
             /*(0 == endaddr)         || (0 == freefull)        || (0 == wrpoint)) */
             (0 == endaddr) || (0 == wrpoint) )
        {
            return;
        }
        if ( (0x80000000 == rbuf_cur_rdaddr) || (0x80000000 == rbuf_cur_wraddr) || (0x80000000 == baseaddr) ||
             (0x80000000 == endaddr) || (0x80000000 == wrpoint) )
        {
            /* Not yet initialized*/
            return;
        }
        /* The initialized default end address will be maintained until write*/
        /* address starts to change. Until this change, no state can be initialized.*/
        if ( baseaddr == rbuf_cur_wraddr )
        {
            return;
        }

        /* Write address has changed*/
        if ( (BDSP_P_RbufType_eDecode == cap->type) || (BDSP_P_RbufType_eEncoder == cap->type) ||
             (BDSP_P_RbufType_ePassthru == cap->type) || (BDSP_P_RbufType_eTranscode == cap->type) )
        {
            /* Decoder / Passthru capture*/
            if ( wrpoint >= endaddr )
            {
                /* Final config is not yet complete*/
                return;
            }
            else
            {
                /* The start write address is now less than the end address,*/
                /* the end address must now be valid, fall through*/
                /* and compute RBUF parameters*/
            }
        }
        else
        {
            /* Mixer capture, write address is no longer equal to the*/
            /* base address, the end address must now be valid, fall through*/
            /* and compute RBUF parameters*/
            RBUF_ASSERT(((BDSP_P_RbufType_eMixer == cap->type) || (BDSP_P_RbufType_eSM == cap->type)), "%s",
                        rbuf_capture_channel_get_info_string(cap->path,
                                                             cap->channel,
                                                             cap->type,
                                                             cap->rbuf_id));
        }

        RBUF_ALERT((endaddr > baseaddr), "%s - 0x%08X 0x%08X",
                   rbuf_capture_channel_get_info_string(cap->path,
                                                        cap->channel,
                                                        cap->type,
                                                        cap->rbuf_id),
                   baseaddr, endaddr);

        entries = (endaddr - baseaddr) + 1;

        /* Use the physical base address to compute the uncached virtual address*/
        result = BMEM_ConvertOffsetToAddress(rbuf_heap, baseaddr, &virt);
        RBUF_ASSERT((BERR_SUCCESS == result), "%s - 0x%08X 0x%08X 0x%08X 0x%08X",
                    rbuf_capture_channel_get_info_string(cap->path,
                                                         cap->channel,
                                                         cap->type,
                                                         cap->rbuf_id),
                    result, rbuf_heap, baseaddr, virt);
        RBUF_DEBUG(("%s INITIALIZED:",
                    rbuf_capture_channel_get_info_string(cap->path,
                                                         cap->channel,
                                                         cap->type,
                                                         cap->rbuf_id)));

        RBUF_DEBUG(("rdaddr=0x%08X  wraddr=0x%08X  baseaddr=0x%08X (virt=0x%08X)",
                    rbuf_cur_rdaddr, rbuf_cur_wraddr, baseaddr,(uint32_t)virt));
        RBUF_DEBUG(("    endaddr=0x%08X (entries=%08X) freefull=0x%08X wrpoint=0x%08X",
                    endaddr, entries, freefull, wrpoint));

        cap->rbuf_num_entries = entries;
        cap->rbuf_base_phys = baseaddr;

        if ( (BDSP_P_RbufType_eDecode == cap->type) || (BDSP_P_RbufType_eEncoder == cap->type) ||
             (BDSP_P_RbufType_ePassthru == cap->type) || (BDSP_P_RbufType_eTranscode == cap->type) )
        {
            /* For decoder / passthru capture, use the start write address*/
            /* to see the capture start point*/
            cap->rbuf_prev_wraddr = wrpoint;
            cap->rbuf_prev_rdaddr = rbuf_cur_rdaddr;
        }
        else
        {
            /* For mixer capture, use the base address*/
            /* to see the capture start point*/
            cap->rbuf_prev_wraddr = baseaddr;
            cap->rbuf_prev_rdaddr = baseaddr;
        }
        cap->rbuf_base_virt_uncached = (uint8_t *)virt;
        cap->rbuf_initialized = 1;
        cap->rbuf_started = 1;
    }
    else
    {
        /* Already initialized, read the write / read address registers*/

        /* AUD_FMM_BF_CTRL_RINGBUF_X_WRADDR*/
        rbuf_cur_wraddr = (uint32_t)BREG_Read32(rbuf_reg_handle, cap->rbuf_reg_base + 0x04);
        /* AUD_FMM_BF_CTRL_RINGBUF_X_RDADDR*/
        rbuf_cur_rdaddr = (uint32_t)BREG_Read32(rbuf_reg_handle, cap->rbuf_reg_base + 0x00);
        if ( (rbuf_cur_wraddr == 0) || (rbuf_cur_wraddr == 0x80000000) || (rbuf_cur_rdaddr == 0) || (rbuf_cur_rdaddr == 0x80000000) ) return;
    }
    /* Extract the wrap bit*/
    wr_cur_wrap = rbuf_cur_wraddr & (1 << 31);
    wr_prev_wrap = cap->rbuf_prev_wraddr & (1 << 31);

    /* Mask the wrap bit*/
    wr_cur_addr = rbuf_cur_wraddr & (~(1 << 31));
    wr_prev_addr = cap->rbuf_prev_wraddr & (~(1 << 31));

    /* Extract the wrap bit*/
    rd_cur_wrap = rbuf_cur_rdaddr & (1 << 31);
    rd_prev_wrap = cap->rbuf_prev_rdaddr & (1 << 31);

    /* Mask the wrap bit*/
    rd_cur_addr = rbuf_cur_rdaddr & (~(1 << 31));
    rd_prev_addr = cap->rbuf_prev_rdaddr & (~(1 << 31));

    /* Verify the current WRADDR*/
    if ( (wr_cur_wrap != 0x00000000) && (wr_cur_wrap != 0x80000000) && (wr_prev_wrap != 0x00000000) && (wr_prev_wrap != 0x80000000) )
    {
        if ( wr_cur_wrap != wr_prev_wrap )
        {
            /* If wrapped, prev must be greater than cur*/
            /* else the RBUFs haven't been serviced in time*/
            if ( wr_cur_addr >= wr_prev_addr )
            {
                printf("\n%s - 0x%08X 0x%08X 0x%08X",
                       rbuf_capture_channel_get_info_string(cap->path,
                                                            cap->channel,
                                                            cap->type,
                                                            cap->rbuf_id));
                printf("\nwr_cur_wrap=0x%08X, wr_prev_wrap=0x%08X", wr_cur_wrap, wr_prev_wrap);
                printf("\nwr_cur_addr=0x%08X, wr_prev_addr=0x%08X", wr_cur_addr, wr_prev_addr);
            }
            RBUF_ALERT((wr_cur_addr < wr_prev_addr), "%s - 0x%08X 0x%08X 0x%08X",
                       rbuf_capture_channel_get_info_string(cap->path,
                                                            cap->channel,
                                                            cap->type,
                                                            cap->rbuf_id),
                       wr_cur_addr, wr_prev_addr, cap->rbuf_count);
            RBUF_ALERT((1 == cap->rbuf_started), "%s",
                       rbuf_capture_channel_get_info_string(cap->path,
                                                            cap->channel,
                                                            cap->type,
                                                            cap->rbuf_id));
        }
    }
    else
    {
        /* Wrap is unchanged, fall through and process the RBUF*/
        if ( wr_cur_addr == wr_prev_addr )
        {
            /* Nothing to do!*/
            if ( RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE == cap->rbuf_to_cap_q_state )
            {
                if ( rbuf_cur_rdaddr == cap->rbuf_prev_rdaddr )
                {
                    /* RBUF is stopped and no more data has arrived*/
                    /* Transition to stopped state*/
                    cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
                    RBUF_DEBUG(("%s - RBUF No longer being read",
                                rbuf_capture_channel_get_info_string(cap->path,
                                                                     cap->channel,
                                                                     cap->type,
                                                                     cap->rbuf_id)));
                }
                else
                {
                    cap->rbuf_prev_rdaddr = rbuf_cur_rdaddr;
                }
            }
            return;
        }
        /* Fall through and process the RBUF*/
        else
        {}
    }

    /* Verify the current RDADDR */
    if ( rd_cur_wrap != rd_prev_wrap )
    {
        /* Wrap changed*/
        RBUF_ALERT((rd_cur_addr < rd_prev_addr), "%s - 0x%08X 0x%08X",
                   rbuf_capture_channel_get_info_string(cap->path,
                                                        cap->channel,
                                                        cap->type,
                                                        cap->rbuf_id),
                   rd_cur_addr, rd_prev_addr);
    }
    else
    {
        /* Wrap unchanged*/
        RBUF_ALERT((rd_cur_addr >= rd_prev_addr), "%s - 0x%08X 0x%08X",
                   rbuf_capture_channel_get_info_string(cap->path,
                                                        cap->channel,
                                                        cap->type,
                                                        cap->rbuf_id),
                   rd_cur_addr, rd_prev_addr);
    }

    if ( rd_cur_wrap != wr_cur_wrap )
    {
        /* Read / Write wraps mismatch*/
        /* NOTE:: Can be equal, since wrap removes ambiguity of full/empty*/
        RBUF_ALERT((rd_cur_addr >= wr_cur_addr), "%s - 0x%08X 0x%08X 0x%08X 0x%08X",
                   rbuf_capture_channel_get_info_string(cap->path,
                                                        cap->channel,
                                                        cap->type,
                                                        cap->rbuf_id),
                   rd_cur_addr, wr_cur_addr, rbuf_cur_rdaddr, rbuf_cur_wraddr);
    }
    else
    {
        /* Read / Write wraps match*/
        RBUF_ALERT((rd_cur_addr <= wr_cur_addr), "%s - 0x%08X 0x%08X",
                   rbuf_capture_channel_get_info_string(cap->path,
                                                        cap->channel,
                                                        cap->type,
                                                        cap->rbuf_id),
                   rd_cur_addr, wr_cur_addr);
    }

    cap->rbuf_prev_rdaddr = rbuf_cur_rdaddr;

    /* Extract the RBUF information*/
    rbuf_base_virt = cap->rbuf_base_virt_cached;

    rbuf_base_phys = cap->rbuf_base_phys;
    rbuf_num_entries = cap->rbuf_num_entries;
    rbuf_w_idx = wr_cur_addr - rbuf_base_phys;
    rbuf_r_idx = wr_prev_addr - rbuf_base_phys;

    RBUF_ALERT((((wr_cur_addr >= rbuf_base_phys) && (wr_cur_addr < (rbuf_base_phys + rbuf_num_entries))) &&
                ((wr_prev_addr >= rbuf_base_phys) && (wr_prev_addr < (rbuf_base_phys + rbuf_num_entries)))),
               "%s - 0x%08X 0x%08X 0x%08X 0x%08X",
               rbuf_capture_channel_get_info_string(cap->path,
                                                    cap->channel,
                                                    cap->type,
                                                    cap->rbuf_id),
               wr_cur_addr, wr_prev_addr, rbuf_base_phys, (rbuf_base_phys + rbuf_num_entries));

    /* It is more efficient to memcpy a block(s) from the RBUF*/
    /* to the capture Q and then update the associated pointers once.*/
    /* For this reason, calculate the active RBUF linear sections*/
    /* and capture Q empty linear sections and then memcpy as required.*/

    /* Get the available bytes from the write pointer to the end*/
    rbuf_copy_to_end_bytes = Q_ENTRIES_END(rbuf_r_idx, rbuf_w_idx, rbuf_num_entries);
    /* Get the available bytes from the start to the write pointer*/
    rbuf_copy_from_start_bytes = Q_ENTRIES_START(rbuf_r_idx, rbuf_w_idx, rbuf_num_entries);
    rbuf_copy_bytes = rbuf_copy_to_end_bytes + rbuf_copy_from_start_bytes;

    RBUF_ALERT(((rbuf_w_idx < rbuf_num_entries) && (rbuf_r_idx < rbuf_num_entries) &&
                (rbuf_copy_to_end_bytes <= rbuf_num_entries) && (rbuf_copy_from_start_bytes <= rbuf_num_entries)),
               "%s - 0x%08X 0x%08X 0x%08X 0x%08X",
               rbuf_capture_channel_get_info_string(cap->path,
                                                    cap->channel,
                                                    cap->type,
                                                    cap->rbuf_id),
               rbuf_w_idx, rbuf_r_idx, rbuf_copy_to_end_bytes, rbuf_copy_from_start_bytes);

    /* Extract the capture Q information*/
    cap_q_ptr = cap->q_ptr;
    cap_q_r_idx = cap->q_r_idx;
    cap_q_w_idx = cap->q_w_idx;
    cap_q_num_entries = cap->q_num_entries;

    /* Get the available bytes from the write pointer to the end*/
    cap_q_avail_to_end_bytes = Q_AVAIL_END(cap_q_r_idx, cap_q_w_idx, cap_q_num_entries);
    /* Get the available bytes from the start to the write pointer*/
    cap_q_avail_from_start_bytes = Q_AVAIL_START(cap_q_r_idx, cap_q_w_idx, cap_q_num_entries);
    cap_q_avail_total_bytes = cap_q_avail_to_end_bytes + cap_q_avail_from_start_bytes;

    /* There must be space for the entire RBUF active bytes to be copied*/
    /* or else the file output is NOT keeping up!*/
    if ( rbuf_copy_bytes > cap_q_avail_total_bytes )
    {
        RBUF_PRINT(("%s:", rbuf_capture_channel_get_info_string(cap->path,
                                                                cap->channel,
                                                                cap->type,
                                                                cap->rbuf_id)));

        RBUF_PRINT(("RBUF w=0x%08X r=0x%08X copy=0x%08X start=0x%08X end=0x%08X",
                    rbuf_w_idx,
                    rbuf_r_idx,
                    rbuf_copy_bytes,
                    rbuf_copy_from_start_bytes,
                    rbuf_copy_to_end_bytes));
        RBUF_PRINT(("CAP  w=0x%08X r=0x%08X start=0x%08X end=0x%08X count=0x%08X",
                    cap_q_w_idx,
                    cap_q_r_idx,
                    cap_q_avail_from_start_bytes,
                    cap_q_avail_to_end_bytes,
                    cap->q_count));

        return;
    }

    /* Process active portions of the RBUF*/
    if ( rbuf_copy_to_end_bytes )
    {
        rbuf_capture_rbuf_copy(cap, rbuf_base_virt, &rbuf_r_idx, rbuf_num_entries, rbuf_copy_to_end_bytes,
                               cap_q_ptr, &cap_q_w_idx, cap_q_num_entries, &cap_q_avail_to_end_bytes, &cap_q_avail_from_start_bytes);
    }

    if ( rbuf_copy_from_start_bytes )
    {
        rbuf_capture_rbuf_copy(cap, rbuf_base_virt, &rbuf_r_idx, rbuf_num_entries, rbuf_copy_from_start_bytes,
                               cap_q_ptr, &cap_q_w_idx, cap_q_num_entries, &cap_q_avail_to_end_bytes, &cap_q_avail_from_start_bytes);
    }
    /* Save the current RBUF WADDR pointer*/
    cap->rbuf_prev_wraddr = rbuf_cur_wraddr;
    cap->rbuf_count += rbuf_copy_bytes;

    /* Now update the capture Q write pointer*/
    cap->q_count += rbuf_copy_bytes;

    /* NOTE:: Since the Capture Q to Disk task uses the write index*/
    /*        to determine available data, this MUST be the final step*/
    /*        to guarantee coherency of all other fields.*/
    cap->q_w_idx = cap_q_w_idx;

    return;
alert:
    rbuf_capture_alert(cap);
    return;
}

static
void*
rbuf_capture_from_rbuf_to_cap_q_task(void *unused)
{
    struct timespec sleep_time;
    uint32_t channel = 0;

    BSTD_UNUSED(unused);

    /* nanosleep setup (at least 1ms)*/
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = (1 * (1000 * 1000));

    /* Used to coordinate task shutdown*/
    rbuf_capture_from_rbuf_to_cap_q_active = 1;

    while ( rbuf_capture_from_rbuf_to_cap_q_active )
    {
        /* Yield*/
        nanosleep(&sleep_time, NULL);

        /* Process the RBUF for all active channel*/
        for ( channel = 0; channel < cap_channel_count; channel++ )
        {
            rbuf_capture_from_rbuf_to_cap_q(&rbuf_capture_channels[channel]);
        }
    }
    /* Signal this thread is terminated*/
    pthread_exit(NULL);
    return NULL;
}

static
void*
rbuf_capture_from_cap_q_to_disk_task(void *unused)
{
    uint32_t cur_bytes = 0, channel = 0;
    struct timespec sleep_time;

    BSTD_UNUSED(unused);

    /* nanosleep setup -- TWD better way to determine sleep time*/
    sleep_time.tv_sec = 0;
    if ( cap_channel_count <= 8 )
    {
        sleep_time.tv_nsec = 25 * (1000 * 1000);
    }
    else
    {
        sleep_time.tv_nsec = 12 * (1000 * 1000);
    }

    /* Used to coordinate task shutdown*/
    rbuf_capture_from_cap_q_to_disk_active = 1;

    /* NOTE:: This loop does not do an explicit yield.  This task*/
    /*        has low priority and will be scheduled based on its*/
    /*        priority.  This approach is taken since there should*/
    /*        always be data available for output.*/

    printf("cap_channel_count = %d\n", cap_channel_count);
    while ( rbuf_capture_from_cap_q_to_disk_active )
    {
        /* Process all active channels*/
        for ( channel = 0; channel < cap_channel_count; channel++ )
        {
            cur_bytes += rbuf_capture_from_cap_q_to_disk(&rbuf_capture_channels[channel]);
            nanosleep(&sleep_time, NULL);
        }
    }
    /* Signal this thread is terminated*/
    pthread_exit(NULL);
    return NULL;
}

static
void
rbuf_capture_channel_init(rbuf_capture_channel_t *cap)
{
    RBUF_ASSERT((NULL != cap), "0x%08X", cap);

    cap->active = 0;
    cap->path = RBUF_PATH_INVALID;
    cap->channel = RBUF_CHANNEL_INVALID;
    cap->index = RBUF_INDEX_INVALID;
    cap->rbuf_id = RBUF_ID_INVALID;
    cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_INACTIVE;
    cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_INACTIVE;

    cap->rbuf_count = 0;
    cap->rbuf_base_virt_uncached = NULL;
    cap->rbuf_base_virt_cached = NULL;
    cap->rbuf_base_phys = 0;
    cap->rbuf_prev_wraddr = 0;
    cap->rbuf_prev_rdaddr = 0;
    cap->rbuf_prev_endaddr = 0;
    cap->rbuf_started = 0;
    cap->rbuf_num_entries = 0;
    cap->rbuf_reg_base = 0;

    cap->q_count = 0;
    cap->q_ptr = NULL;
    cap->q_r_idx = 0;
    cap->q_w_idx = 0;
    cap->q_num_entries = 0;

    cap->file_name[0] = 0;
    cap->file = -1;
    return;
}

static
void
rbuf_capture_channel_uninit(rbuf_capture_channel_t *cap)
{
    int32_t result = 0;

    if ( !cap->active )
    {
        /* Channel is not active, nothing to do*/
        return;
    }
    if ( RBUF_CHANNEL_INVALID != cap->channel )
    {
        /* Unititialize this capture channel*/
        RBUF_DEBUG(("%s - Uninit",
                    rbuf_capture_channel_get_info_string(cap->path,
                                                         cap->channel,
                                                         cap->type,
                                                         cap->rbuf_id)));
        RBUF_ASSERT((NULL != cap->q_ptr),
                    "%s - 0x%08X",
                    rbuf_capture_channel_get_info_string(cap->path,
                                                         cap->channel,
                                                         cap->type,
                                                         cap->rbuf_id),
                    cap->q_ptr);

        /* BKNI_Free(cap->q_ptr); */
        free(cap->q_ptr);

        result = rbuf_capture_device_close(cap->file);
        if ( EBADF == errno )
        {
            RBUF_DEBUG(("Capture file not created!"));
            return;
        }

        /* If the copy path has been initialized, attempt to copy the capture*/
        /* file to the supplied output path*/
        if ( 0 != rbuf_copy_path[0] )
        {
            char cmd[2048];

            sprintf(cmd, "/bin/cp -f %s/%s %s/%s.tmp",
                    (char *)rbuf_capture_path,
                    (char *)cap->file_name,
                    (char *)rbuf_copy_path,
                    (char *)cap->file_name);

            RBUF_DEBUG(("'%s'", cmd));
            result = system(cmd);
            if ( result != 0 )
            {
                RBUF_PRINT(("Copy captured file failed!"));
            }

            sprintf(cmd, "/bin/mv -f %s/%s.tmp %s/%s",
                    (char *)rbuf_copy_path,
                    (char *)cap->file_name,
                    (char *)rbuf_copy_path,
                    (char *)cap->file_name);

            RBUF_DEBUG(("'%s'", cmd));
            result = system(cmd);
            if ( result != 0 )
            {
                RBUF_PRINT(("Rename captured file at copy path failed!"));
            }

            RBUF_DEBUG(("COPIED - '%s/%s' to '%s/%s'",
                        (char *)rbuf_capture_path,
                        (char *)cap->file_name,
                        (char *)rbuf_copy_path,
                        (char *)cap->file_name));
        }
    }
    cap->file_name[0] = 0;
    cap->path = RBUF_PATH_INVALID;
    cap->channel = RBUF_CHANNEL_INVALID;

    return;
}

static
int32_t
rbuf_capture_channels_index_get_free(void)
{
    int32_t index;

    RBUF_ASSERT((NULL != rbuf_capture_channels), "0x%08X", rbuf_capture_channels);

    for ( index = 0; index < RBUF_CAP_MAX_CHAN; index++ )
    {
        if ( !rbuf_capture_channels[index].active )
        {
            return index;
        }
    }
    return RBUF_INDEX_INVALID;
}

static
int32_t
rbuf_capture_channels_index_find(uint32_t path,
                                 uint32_t channel,
                                 uint32_t rbuf_id,
                                 BDSP_P_RbufType type)
{
    int32_t index;

    RBUF_ASSERT((NULL != rbuf_capture_channels), "0x%08X", rbuf_capture_channels);

    if ( rbuf_capture_app_player == 1 )
    {
        for ( index = 0; index < RBUF_CAP_MAX_CHAN; index++ )
        {
            if ( rbuf_capture_channels[index].active )
            {
                if ( (rbuf_capture_channels[index].path == path) &&
                     (rbuf_capture_channels[index].channel == channel) &&
                     (rbuf_capture_channels[index].rbuf_id == rbuf_id) &&
                     (rbuf_capture_channels[index].type == type) )
                {
                    return index;
                }
            }
        }
    }
    else
    {
        for ( index = 0; index < RBUF_CAP_MAX_CHAN; index++ )
        {
            if ( rbuf_capture_channels[index].active )
            {
                if ( (rbuf_capture_channels[index].path == path) &&
                     (rbuf_capture_channels[index].channel == channel) &&
                     (rbuf_capture_channels[index].type == type) )
                {
                    return index;
                }
            }
        }
    }
    return RBUF_INDEX_INVALID;
}

static
void
rbuf_capture_channels_init(void)
{
    uint32_t channel;

    rbuf_capture_channels = BKNI_Malloc(RBUF_CAP_MAX_CHAN * sizeof(*rbuf_capture_channels));
    RBUF_ASSERT((NULL != rbuf_capture_channels), "0x%08X", rbuf_capture_channels);

    for ( channel = 0; channel < RBUF_CAP_MAX_CHAN; channel++ )
    {
        rbuf_capture_channel_init(&rbuf_capture_channels[channel]);
    }

    return;
}

static
void
rbuf_capture_channels_uninit(void)
{
    uint32_t channel;

    /*uninitalize and free for all capture channels*/
    for ( channel = 0; channel < RBUF_CAP_MAX_CHAN; channel++ )
    {
        rbuf_capture_channel_uninit(&rbuf_capture_channels[channel]);
    }

    RBUF_ASSERT((NULL != rbuf_capture_channels), "0x%08X", rbuf_capture_channels);
    BKNI_Free(rbuf_capture_channels);
    return;
}

void
rbuf_capture_startup(void)
{
    int32_t result;
    /*struct sched_param sc_cfg;*/
    pthread_attr_t rbuf_thread_attr;

    rbuf_capture_channels_init();

    /*Create read & write threads with default priority */
    result = pthread_attr_init(&rbuf_thread_attr);
    RBUF_ASSERT((0 == result), "0x%08X", result);
    result = pthread_create(&rbuf_capture_from_cap_q_to_disk_thread,
                            &rbuf_thread_attr,
                            rbuf_capture_from_cap_q_to_disk_task,
                            NULL);
    RBUF_ASSERT((0 == result), "0x%08X", result);

    result = pthread_attr_init(&rbuf_thread_attr);
    RBUF_ASSERT((0 == result), "0x%08X", result);
    result = pthread_create(&rbuf_capture_from_rbuf_to_cap_q_thread,
                            &rbuf_thread_attr,
                            rbuf_capture_from_rbuf_to_cap_q_task,
                            NULL);
    RBUF_ASSERT((0 == result), "0x%08X", result);

    printf("RBUF Capture Initialized");
    rbuf_capture_initialized = 1;
    return;
}

static
void capture_filename(rbuf_capture_channel_t *cap)
{
    char *postfix;
    uint32_t chan, rbuf_id;
    static char file_name[512];

    switch ( cap->type )
    {
    case BDSP_P_RbufType_eDecode:
        {
            static char *dec = "decoder";
            postfix = dec;
            chan = cap->channel;
            rbuf_id = cap->rbuf_id;
            break;
        }
    case BDSP_P_RbufType_ePassthru:
        {
            static char *pass = "passthru";
            postfix = pass;
            chan = cap->channel;
            rbuf_id = cap->rbuf_id;
            break;
        }
    case BDSP_P_RbufType_eMixer:
        {
            static char *mix = "mixer";
            postfix = mix;
            chan = cap->channel;
            rbuf_id = cap->rbuf_id;
            break;
        }
    case BDSP_P_RbufType_eEncoder:
        {
            static char *enc = "encoder";
            postfix = enc;
            chan = cap->channel;
            rbuf_id = cap->rbuf_id;
            break;
        }
    case BDSP_P_RbufType_eTranscode:
        {
            static char *tran = "transcode";
            postfix = tran;
            chan = cap->channel;
            rbuf_id = cap->rbuf_id;
            break;
        }
    case BDSP_P_RbufType_eSM:
        {
            static char *spkr = "spkr_mgmt";
            postfix = spkr;
            chan = cap->channel;
            rbuf_id = cap->rbuf_id;
            break;
        }
    default:
        {
            RBUF_ASSERT((0), "");
            postfix = NULL;
            chan = RBUF_CHANNEL_INVALID;
            rbuf_id = RBUF_ID_INVALID;
            return;
        }
    }

    if ( rbuf_capture_multifile )
    {
        sprintf((char *)cap->file_name, "%d_%s_path_%d_%s_%d.cap",
                capture_count,
                rbuf_capture_filename,
                cap->path,
                postfix,
                chan);
    }
    /*If both multifile and app_player is enabled use multifile*/
    else if ( rbuf_capture_app_player && !rbuf_capture_multifile )
    {
        /*Use rbuf_id for capture filename with app_player*/
        sprintf((char *)cap->file_name, "%s_path_%d_%s_%d_%d.cap",
                rbuf_capture_filename,
                cap->path,
                postfix,
                chan,
                rbuf_id);
    }
    else
    {
        sprintf((char *)cap->file_name, "%s_path_%d_%s_%d.cap",
                rbuf_capture_filename,
                cap->path,
                postfix,
                chan);
    }
    sprintf((char *)file_name, "%s/%s", rbuf_capture_path, (char *)cap->file_name);
    cap->file = rbuf_capture_device_open((int8_t *)file_name);
    if ( cap->file < 0 )
    {
        RBUF_PRINT(("Unable to open capture file %s",(char *)cap->file_name));
    }
    return;
}

static
void
rbuf_capture_ringbuf_start(rbuf_capture_channel_t *cap)
{
    uint32_t baseaddr;
    uint32_t endaddr;
    uint32_t entries;
    void *virt_uncached;
    void *virt_cached;
    int32_t result;

    RBUF_DEBUG(("%s - Starting Ring Buffer ID %d FMM Base Address 0x%08X",
                rbuf_capture_channel_get_info_string(cap->path,
                                                     cap->channel,
                                                     cap->type,
                                                     cap->rbuf_id),
                cap->rbuf_id,
                cap->rbuf_reg_base));

    /*Set all pointers to ZERO on start*/
    cap->rbuf_num_entries = 0;
    cap->rbuf_base_phys = 0;
    cap->rbuf_prev_rdaddr = 0;
    cap->rbuf_prev_wraddr = 0;
    cap->rbuf_prev_endaddr = 0;
    cap->rbuf_started = 0;
    cap->rbuf_base_virt_uncached = NULL;
    cap->rbuf_initialized = 0;

    /* CACHED READS FROM RING BUFFER:*/
    /**/
    /*   Problem:*/
    /*   The MIPS receives access to DRAM essetially based on access count,*/
    /*   not bandwidth, in conjuntion with priority and round robin.  Therefore,*/
    /*   reading single 32bit quantities will result in one read per arbitration*/
    /*   schedule.  In a memory bound system, this will result in starvation*/
    /*   to the MIPS (it has many small accesses to perform and is high frequency).*/
    /*   Also, do to the nature of memory accesses, it is more efficient to */
    /*   burst blocks of contiguous data, rather incurring the setup/termination*/
    /*   overhead of individual accesses.*/
    /**/
    /*   Solution:*/
    /*   Cached accesses to the ring buffer shall be performed.  Using the*/
    /*   Linux cacheflush() system call, each ring buffer's address will be*/
    /*   cache flushed upon initialization.  From that point forward, when new*/
    /*   data is received at a given ring buffer, the address range corresponding*/
    /*   to that new data will be flushed.  Since no data will be written by*/
    /*   the MIPS to the ring buffer address, no data will be written to DRAM.*/
    /*   However, a side effect of cacheflush() is to also invalidate the*/
    /*   cached region.  This invalidate will cause a burst read from the memory*/
    /*   controller when the contents are read from ring buffer.*/

    /* AUD_FMM_BF_CTRL_RINGBUF_X_BASEADDR */
    baseaddr = (uint32_t)BREG_Read32(rbuf_reg_handle, cap->rbuf_reg_base + 0x08);

    /* AUD_FMM_BF_CTRL_RINGBUF_X_ENDADDR */
    endaddr = (uint32_t)BREG_Read32(rbuf_reg_handle, cap->rbuf_reg_base + 0x0C);

    if ( endaddr < baseaddr )
    {
        /*if endaddr is less than baseaddr, alert and disable capture on that channel */
        rbuf_capture_alert(cap);
        return;
    }

    entries = (endaddr - baseaddr) + 1;

    /* Use the physical base address to compute the uncached virtual address*/
    result = BMEM_ConvertOffsetToAddress(rbuf_heap, baseaddr, &virt_uncached);
    RBUF_ASSERT((BERR_SUCCESS == result), "0x%08X", result);

    /* Use the uncached address to compute the cached address*/
    result = BMEM_ConvertAddressToCached(rbuf_heap, virt_uncached, &virt_cached);
    RBUF_ASSERT((BERR_SUCCESS == result), "0x%08X", result);

    RBUF_DEBUG(("%s - Phys:0x%08X Uncached:0x%08X Cached:0x%08X Entries:0x%08X",
                rbuf_capture_channel_get_info_string(cap->path,
                                                     cap->channel,
                                                     cap->type,
                                                     cap->rbuf_id),
                baseaddr,
                (int32_t)virt_uncached,
                (int32_t)virt_cached,
                entries));

    /* Initial flush of this entire ring buffer.  It will be possible*/
    /* for the actual used number of bytes to be less than the total*/
    /* allocated size, but this flush will assume the max size to */
    /* put the cache into a known state (invalid).*/
    NEXUS_Memory_FlushCache(virt_cached, entries);
    cap->rbuf_base_virt_cached = (uint8_t *)virt_cached;

    /* Signal this channel's capture is ready*/
    cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_ACTIVE;
    cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_ACTIVE;

    RBUF_PRINT(("%s STARTED!",
                rbuf_capture_channel_get_info_string(cap->path,
                                                     cap->channel,
                                                     cap->type,
                                                     cap->rbuf_id)));

    capture_filename(cap);
    return;
}

static
void
rbuf_capture_channel_start_common(uint32_t channel,
                                  uint32_t path,
                                  BDSP_P_RbufType type,
                                  uint32_t rbuf_id)
{
    rbuf_capture_channel_t *cap;
    int32_t index;

    RBUF_PRINT(("%s START", rbuf_capture_channel_get_info_string(path, channel, type, rbuf_id)));

    if ( !rbuf_capture_enabled )
    {
        /*Capture not enabled, just return*/
        return;
    }
    else if ( !rbuf_capture_initialized )
    {
        RBUF_PRINT(("CAPTURE NOT INITIALIZED, START IGNORED"));
        return;
    }

    if ( rbuf_capture_app_player )
    {
        index = rbuf_capture_channels_index_find(path, channel, rbuf_id, type);
    }
    else
    {
        index = rbuf_capture_channels_index_find(path, channel, RBUF_INDEX_INVALID, type);
    }
    if ( RBUF_INDEX_INVALID == index )
    {
        RBUF_PRINT(("%s CAPTURE NOT CONFIGURED, IGNORING START!", rbuf_capture_channel_get_type_string(path, channel, rbuf_id, type)));
        return;
    }

    cap = &rbuf_capture_channels[index];

    /* Compute the ring buffer ID*/
    RBUF_ASSERT((64 > rbuf_id), "%s", rbuf_capture_channel_get_info_string(path, channel, type, rbuf_id));
    cap->rbuf_id = rbuf_id;

    /* Compute the base address based on the ID*/
#ifdef BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR
    cap->rbuf_reg_base = (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR +
                          (cap->rbuf_id * (BCHP_AUD_FMM_BF_CTRL_RINGBUF_1_RDADDR -
                                           BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR)));
#else
    cap->rbuf_reg_base = (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR +
                          (cap->rbuf_id * (BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_1_RDADDR -
                                           BCHP_AUD_FMM_BF_CTRL_SOURCECH_RINGBUF_0_RDADDR)));
#endif

    rbuf_capture_ringbuf_start(cap);
    return;
}

uint8_t
rbuf_capture_channel_start_dummy(uint32_t channel,
                                 uint32_t path,
                                 uint32_t rbuf_id,
                                 BDSP_P_RbufType CP_TYPE)
{
    BSTD_UNUSED(channel);
    BSTD_UNUSED(path);
    BSTD_UNUSED(rbuf_id);
    BSTD_UNUSED(CP_TYPE);

    return 0;
}

uint8_t
rbuf_capture_stop_channel_dummy(uint32_t channel,
                                uint32_t path,
                                uint32_t rbuf_id,
                                BDSP_P_RbufType CP_TYPE)
{
    BSTD_UNUSED(channel);
    BSTD_UNUSED(path);
    BSTD_UNUSED(rbuf_id);
    BSTD_UNUSED(CP_TYPE);

    return 0;
}

uint8_t
rbuf_capture_channel_start(uint32_t channel,
                           uint32_t path,
                           uint32_t rbuf_id,
                           BDSP_P_RbufType CP_TYPE)
{
    if ( rbuf_capture_enabled )
    {
        /*Counter for multifile capture*/
        if ( cap_count_flag == true )
        {
            cap_count_flag = false;
            if ( capture_count < 65536 )
            {
                capture_count += 1;
            }
            else
            {
                capture_count = 0;
            }
        }

        if ( CP_TYPE == BDSP_P_RbufType_eDecode )
        {
            rbuf_capture_channel_start_common(channel,
                                              path,
                                              BDSP_P_RbufType_eDecode,
                                              rbuf_id);
            return 0;
        }
        else if ( CP_TYPE == BDSP_P_RbufType_ePassthru )
        {
            rbuf_capture_channel_start_common(channel,
                                              path,
                                              BDSP_P_RbufType_ePassthru,
                                              rbuf_id);
            return 0;
        }
        else if ( CP_TYPE == BDSP_P_RbufType_eMixer )
        {
            rbuf_capture_channel_start_common(channel,
                                              path,
                                              BDSP_P_RbufType_eMixer,
                                              rbuf_id);
            return 0;
        }
        else if ( CP_TYPE == BDSP_P_RbufType_eEncoder )
        {
            rbuf_capture_channel_start_common(channel,
                                              path,
                                              BDSP_P_RbufType_eEncoder,
                                              rbuf_id);
            return 0;
        }
        else if ( CP_TYPE == BDSP_P_RbufType_eTranscode )
        {
            rbuf_capture_channel_start_common(channel,
                                              path,
                                              BDSP_P_RbufType_eTranscode,
                                              rbuf_id);
            return 0;
        }
        else if ( CP_TYPE == BDSP_P_RbufType_eSM )
        {
            rbuf_capture_channel_start_common(channel,
                                              path,
                                              BDSP_P_RbufType_eSM,
                                              rbuf_id);
            return 0;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

static
void
rbuf_capture_channel_stop(rbuf_capture_channel_t *cap)
{
    /*Flag to enable/disable count for multifile capture*/
    if ( cap_count_flag == false )
    {
        cap_count_flag = true;
    }

    if ( ((RBUF_CAPTURE_CHANNEL_STATE_INACTIVE == cap->rbuf_to_cap_q_state) &&
          (RBUF_CAPTURE_CHANNEL_STATE_INACTIVE == cap->cap_q_to_disk_state)) ||
         (RBUF_ID_INVALID == cap->rbuf_id) )
    {
        /* Channel was never started, ignore stop*/
        RBUF_PRINT(("%s CAPTURE NOT CONFIGURED, IGNORING STOP!",
                    rbuf_capture_channel_get_type_string(cap->path,
                                                         cap->channel,
                                                         cap->rbuf_id,
                                                         cap->type)));
        cap->rbuf_started = 0;
        cap->rbuf_initialized = 0;
        return;
    }
    else
    {
        struct timespec sleep_time;
        sleep_time.tv_sec = 0;
        sleep_time.tv_nsec = 25 * 1000 * 1000;

        /*  cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE;*/
        /*  cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_PEND_OUTPUT_DONE;*/

        do
        {
            /* Yield for read & write threads to complete the task*/
            nanosleep(&sleep_time, NULL);
            loop_break++;
            if ( loop_break > RBUF_CAP_MAX_CHAN )
            {
                cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
                cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_STOPPED;
            }
        } while ( (RBUF_CAPTURE_CHANNEL_STATE_STOPPED != cap->rbuf_to_cap_q_state) ||
                  (RBUF_CAPTURE_CHANNEL_STATE_STOPPED != cap->cap_q_to_disk_state) );

        if ( cap->rbuf_flush_onexit )
        {
            if ( cap->q_w_idx >= cap->q_r_idx )
            {
                rbuf_capture_device_write(cap->file, &cap->q_ptr[cap->q_r_idx], (cap->q_w_idx - cap->q_r_idx));
                cap->q_r_idx = cap->q_w_idx;
            }
            else
            {
                rbuf_capture_device_write(cap->file, &cap->q_ptr[cap->q_r_idx], (RBUF_CAP_MAX_CHAN_ENTRIES - cap->q_r_idx));
                rbuf_capture_device_write(cap->file, &cap->q_ptr[cap->q_r_idx], (cap->q_w_idx - 0));
                cap->q_r_idx = cap->q_w_idx;
            }
            cap->rbuf_flush_onexit = false;
        }
    }
    RBUF_PRINT(("STOPPED!"));

    /* RBUF is no longer active*/
    cap->rbuf_started = 0;
    cap->rbuf_initialized = 0;
    return;
}

static
void
rbuf_capture_channel_stop_common(uint32_t channel,
                                 uint32_t path,
                                 uint32_t rbuf_id,
                                 BDSP_P_RbufType type)
{
    rbuf_capture_channel_t *cap;
    int32_t index;



    RBUF_PRINT(("%s STOP",rbuf_capture_channel_get_type_string(path, channel, rbuf_id, type)));

    if ( !rbuf_capture_enabled )
    {
        /*capture not enabled, just return*/
        return;
    }
    else if ( !rbuf_capture_initialized )
    {
        /*Capture not initialized on this channel, ignore stop*/
        RBUF_PRINT(("CAPTURE NOT INITIALIZED, STOP IGNORED"));
        return;
    }

    index = rbuf_capture_channels_index_find(path, channel, rbuf_id, type);
    if ( RBUF_INDEX_INVALID == index )
    {
        RBUF_PRINT(("%s CAPTURE NOT CONFIGURED, IGNORING STOP!",rbuf_capture_channel_get_type_string(path, channel, rbuf_id, type)));
        return;
    }
    cap = &rbuf_capture_channels[index];

    rbuf_capture_channel_stop(cap);

    return;
}

uint8_t
rbuf_capture_stop_channel(uint32_t channel,
                          uint32_t path,
                          uint32_t rbuf_id,
                          BDSP_P_RbufType CP_TYPE)
{
    if ( rbuf_capture_enabled )
    {
        if ( CP_TYPE == BDSP_P_RbufType_eDecode )
        {
            rbuf_capture_channel_stop_common(channel,
                                             path,
                                             rbuf_id,
                                             BDSP_P_RbufType_eDecode);
            return 0;
        }
        else if ( CP_TYPE == BDSP_P_RbufType_ePassthru )
        {
            rbuf_capture_channel_stop_common(channel,
                                             path,
                                             rbuf_id,
                                             BDSP_P_RbufType_ePassthru);
            return 0;
        }
        else if ( CP_TYPE == BDSP_P_RbufType_eMixer )
        {
            rbuf_capture_channel_stop_common(channel,
                                             path,
                                             rbuf_id,
                                             BDSP_P_RbufType_eMixer);
            return 0;
        }
        else if ( CP_TYPE == BDSP_P_RbufType_eEncoder )
        {
            rbuf_capture_channel_stop_common(channel,
                                             path,
                                             rbuf_id,
                                             BDSP_P_RbufType_eEncoder);
            return 0;
        }
        else if ( CP_TYPE == BDSP_P_RbufType_eTranscode )
        {
            rbuf_capture_channel_stop_common(channel,
                                             path,
                                             rbuf_id,
                                             BDSP_P_RbufType_eTranscode);
            return 0;
        }
        else if ( CP_TYPE == BDSP_P_RbufType_eSM )
        {
            rbuf_capture_channel_stop_common(channel,
                                             path,
                                             rbuf_id,
                                             BDSP_P_RbufType_eSM);
            return 0;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

static
void
rbuf_capture_channel_add_cap(rbuf_capture_channel_t *cap)
{
    /* If this channel is being actively captured, must stop it first!*/
    if ( (RBUF_CAPTURE_CHANNEL_STATE_ACTIVE == cap->rbuf_to_cap_q_state) ||
         (RBUF_CAPTURE_CHANNEL_STATE_ACTIVE == cap->cap_q_to_disk_state) )
    {
        struct timespec sleep_time;

        cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_PEND_STOP;
        cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_PEND_STOP;
        /* Wait for capture tasks to indicate this channel is stopped*/
        do
        {
            /* nanosleep setup (sleep the minimal amount of time)*/
            sleep_time.tv_sec = 0;
            sleep_time.tv_nsec = 1;
            /* Yield*/
            nanosleep(&sleep_time, NULL);
        } while ( (RBUF_CAPTURE_CHANNEL_STATE_STOPPED != cap->rbuf_to_cap_q_state) ||
                  (RBUF_CAPTURE_CHANNEL_STATE_STOPPED != cap->cap_q_to_disk_state) );
    }

    cap->rbuf_to_cap_q_state = RBUF_CAPTURE_CHANNEL_STATE_INACTIVE;
    cap->cap_q_to_disk_state = RBUF_CAPTURE_CHANNEL_STATE_INACTIVE;

    cap->rbuf_count = 0;
    cap->rbuf_base_virt_uncached = NULL;
    cap->rbuf_base_phys = 0;
    cap->rbuf_num_entries = 0;
    cap->rbuf_prev_wraddr = 0;
    cap->rbuf_prev_rdaddr = 0;
    cap->rbuf_prev_endaddr = 0;
    cap->rbuf_started = 0;

    if ( NULL == cap->q_ptr )
    {
        cap->q_count = 0;
        cap->q_r_idx = 0;
        cap->q_w_idx = 0;
        cap->q_num_entries = RBUF_CAP_MAX_CHAN_ENTRIES;

        /*Create a page aligned buffer for capture*/
        posix_memalign((void **)&cap->q_ptr, RBUF_PAGE_SIZE, cap->q_num_entries);
        RBUF_ASSERT((NULL != cap->q_ptr), "0x%08X", (sizeof(*cap->q_ptr) * cap->q_num_entries));

        /*switch (cap->type)
        {
            case BDSP_P_RbufType_eDecode: {
            static char *dec= "decoder";
            break;
            }
            case BDSP_P_RbufType_ePassthru: {
            static char *pass= "passthru";
            break;
            }
            case BDSP_P_RbufType_eMixer: {
            static char *mix="mixer";
            break;
            }
            case BDSP_P_RbufType_eEncoder: {
            static char *enc="encoder";
            break;
            }
            case BDSP_P_RbufType_eTranscode: {
            static char *tran="transcode";
            break;
            }
            case BDSP_P_RbufType_eSM: {
            static char *spkr="spkr_mgmt";
            break;
            }
            default: {
            RBUF_ASSERT((0), "");
            break;
            }
        }*/
    }

    RBUF_PRINT(("ADDED!"));
    return;
}

static
void
rbuf_capture_channel_add(uint32_t path,
                         uint32_t channel,
                         uint32_t rbuf_id,
                         BDSP_P_RbufType type,
                         Rbuf_Callback_Info *cb_info)
{
    rbuf_capture_channel_t *cap;
    int32_t index;

    RBUF_PRINT(("%s ADD", rbuf_capture_channel_get_type_string(path, channel, rbuf_id, type)));

    if ( !rbuf_capture_enabled )
    {
        RBUF_PRINT(("CAPTURE DISABLED, ADD IGNORED"));
        return;
    }
    else if ( !rbuf_capture_initialized )
    {
        RBUF_PRINT(("CAPTURE NOT INITIALIZED, ADD IGNORED"));
        return;
    }

    /* Get a free channel data structure index*/
    index = rbuf_capture_channels_index_get_free();

    RBUF_ASSERT((RBUF_INDEX_INVALID != index),
                "%s",
                rbuf_capture_channel_get_type_string(path, channel, rbuf_id, type));

    cap = &rbuf_capture_channels[index];

    cap->index = index;
    cap->path = path;
    cap->channel = channel;
    cap->rbuf_id = rbuf_id;
    cap->type = type;
    cap->active = 1;
    cap->cb = NULL;
    cap->cb_context = NULL;

    /*Install the function pointer to send capture data*/
    if ( cb_info != NULL )
    {
        RBUF_PRINT(("Installing callback capture function"));
        cap->cb = cb_info->cb;
        cap->cb_context = cb_info->cb_context;
    }
    rbuf_capture_channel_add_cap(cap);

    return;
}

static
int32_t
rbuf_capture_command_execute(RBUF_CAPTURE_COMMAND_t *cmd)
{
    RBUF_ASSERT((NULL != cmd), "0x%08X", cmd);

    /*If rbuf_id is not required set it to default: RBUF_ID_INVALID*/
    if ( !rbuf_capture_app_player )
    {
        cmd->args[2] = RBUF_ID_INVALID;
    }

    switch ( cmd->command )
    {
    case RBUF_CAPTURE_COMMAND_INIT:
        {
            /*Start the read & write threads for capture*/
            rbuf_capture_startup();
            break;
        }
    case RBUF_CAPTURE_COMMAND_DECODER:
        {
            rbuf_capture_channel_add(cmd->args[0], cmd->args[1], cmd->args[2], BDSP_P_RbufType_eDecode, NULL);
            cap_channel_count += 1;
            break;
        }
    case RBUF_CAPTURE_COMMAND_PASSTHRU:
        {
            rbuf_capture_channel_add(cmd->args[0], cmd->args[1], cmd->args[2], BDSP_P_RbufType_ePassthru, NULL);
            cap_channel_count += 1;
            break;
        }
    case RBUF_CAPTURE_COMMAND_MIXER:
        {
            rbuf_capture_channel_add(cmd->args[0], cmd->args[1], cmd->args[2], BDSP_P_RbufType_eMixer, NULL);
            cap_channel_count += 1;
            break;
        }
    case RBUF_CAPTURE_COMMAND_ENCODER:
        {
            rbuf_capture_channel_add(cmd->args[0], cmd->args[1], cmd->args[2], BDSP_P_RbufType_eEncoder, NULL);
            cap_channel_count += 1;
            break;
        }
    case RBUF_CAPTURE_COMMAND_TRANSCODE:
        {
            rbuf_capture_channel_add(cmd->args[0], cmd->args[1], cmd->args[2], BDSP_P_RbufType_eTranscode, NULL);
            cap_channel_count += 1;
            break;
        }
    case RBUF_CAPTURE_COMMAND_SPKR_MGMT:
        {
            rbuf_capture_channel_add(cmd->args[0], cmd->args[1], cmd->args[2], BDSP_P_RbufType_eSM, NULL);
            cap_channel_count += 1;
            break;
        }
    case RBUF_CAPTURE_COMMAND_ECHO:
        {
            RBUF_PRINT(("SOCKET ECHO RECEIVED"));
            break;
        }
    default:
        {
            RBUF_ASSERT(0, "0x%08X", 0);
        }
    }
    return 0;
}

static
int8_t*
rbuf_capture_skip_whitespace(int8_t **str)
{
    int8_t *p1;
    int8_t *p2;

    /* Find the first non whitespace character*/
    p1 = *str;
    while ( 1 )
    {
        if ( 0 == *p1 )
        {
            return NULL;
        }
        if ( isspace(*p1) )
        {
            p1++;
        }
        else
        {
            break;
        }
    }

    /* Now find the first trailing whitespace character*/
    p2 = p1;
    while ( (0 != *p2) && (!isspace(*p2)) )
    {
        p2++;
    }

    /* Save the first non-whitespace character address*/
    *str = p1;

    /* Return the first trailing whitespace character address*/
    return p2;
}

static
int32_t
rbuf_capture_command_parse(int8_t *str,
                           RBUF_CAPTURE_COMMAND_t *command)
{
    int8_t *cmd;
    int32_t cmd_bytes;
    int8_t *ptr;

    RBUF_ASSERT((NULL != command), "0x%08X", command);

    cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_INIT];
    cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_INIT]);
    if ( 0 == strncmp((char *)cmd, (char *)str, cmd_bytes) )
    {
        command->command = RBUF_CAPTURE_COMMAND_INIT;
        command->args[0] = 0;
        return 0;
    }

    /*Get capture path from command configuration file*/
    cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_CAPTURE_PATH];
    cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_CAPTURE_PATH]);
    if ( 0 == strncmp((char *)cmd, (char *)str, cmd_bytes) )
    {
        int8_t *ptr;

        str += cmd_bytes;
        ptr = rbuf_capture_skip_whitespace(&str);
        if ( NULL != ptr )
        {
            uint32_t num_bytes;

            RBUF_ASSERT((ptr >= str), "0x%08X 0x%08X", ptr, str);
            num_bytes = (ptr - str);
            RBUF_ASSERT((num_bytes < sizeof(rbuf_capture_path)), "0x%08X 0x%08X", num_bytes, sizeof(rbuf_capture_path));
            strncpy((char *)rbuf_capture_path, (char *)str, num_bytes);
            rbuf_capture_path[num_bytes] = 0;
            RBUF_PRINT(("Capture Path '%s'", rbuf_capture_path));
            return 1;
        }
        else
        {
            return -1;
        }
    }

    cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_CAPTURE_FILENAME];
    cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_CAPTURE_FILENAME]);
    if ( 0 == strncmp((char *)cmd, (char *)str, cmd_bytes) )
    {
        int8_t *ptr;

        str += cmd_bytes;
        ptr = rbuf_capture_skip_whitespace(&str);
        if ( NULL != ptr )
        {
            uint32_t num_bytes;

            RBUF_ASSERT((ptr >= str), "0x%08X 0x%08X", ptr, str);
            num_bytes = (ptr - str);
            RBUF_ASSERT((num_bytes < sizeof(rbuf_capture_filename)), "0x%08X 0x%08X", num_bytes, sizeof(rbuf_capture_filename));
            strncpy((char *)rbuf_capture_filename, (char *)str, num_bytes);
            rbuf_capture_filename[num_bytes] = 0;
            RBUF_PRINT(("Capture Filename '%s'", rbuf_capture_filename));
            return 1;
        }
        else
        {
            return -1;
        }
    }

    cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_COPY_PATH];
    cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_COPY_PATH]);
    if ( 0 == strncmp((char *)cmd, (char *)str, cmd_bytes) )
    {
        int8_t *ptr;

        str += cmd_bytes;
        ptr = rbuf_capture_skip_whitespace(&str);
        if ( NULL != ptr )
        {
            uint32_t num_bytes;

            RBUF_ASSERT((ptr >= str), "0x%08X 0x%08X", ptr, str);
            num_bytes = (ptr - str);
            RBUF_ASSERT((num_bytes < sizeof(rbuf_copy_path)), "0x%08X 0x%08X", num_bytes, sizeof(rbuf_copy_path));
            strncpy((char *)rbuf_copy_path, (char *)str, num_bytes);
            rbuf_copy_path[num_bytes] = 0;
            RBUF_PRINT(("Copy Path '%s'", rbuf_copy_path));
            return 1;
        }
        else
        {
            return -1;
        }
    }

    cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_ECHO];
    cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_ECHO]);
    if ( 0 == strncmp((char *)cmd, (char *)str, cmd_bytes) )
    {
        command->command = RBUF_CAPTURE_COMMAND_ECHO;
        return 0;
    }

    cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_DECODER];
    cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_DECODER]);
    if ( 0 == strncmp((char *)cmd, (char *)str, cmd_bytes) )
    {
        command->command = RBUF_CAPTURE_COMMAND_DECODER;
        goto found;
    }

    cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_PASSTHRU];
    cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_PASSTHRU]);
    if ( 0 == strncmp((char *)cmd, (char *)str, cmd_bytes) )
    {
        command->command = RBUF_CAPTURE_COMMAND_PASSTHRU;
        goto found;
    }

    cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_MIXER];
    cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_MIXER]);
    if ( 0 == strncmp((char *)cmd, (char *)str, cmd_bytes) )
    {
        command->command = RBUF_CAPTURE_COMMAND_MIXER;
        goto found;
    }

    cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_ENCODER];
    cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_ENCODER]);
    if ( 0 == strncmp((char *)cmd, (char *)str, cmd_bytes) )
    {
        command->command = RBUF_CAPTURE_COMMAND_ENCODER;
        goto found;
    }

    cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_TRANSCODE];
    cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_TRANSCODE]);
    if ( 0 == strncmp((char *)cmd, (char *)str, cmd_bytes) )
    {
        command->command = RBUF_CAPTURE_COMMAND_TRANSCODE;
        goto found;
    }

    cmd = RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_SPKR_MGMT];
    cmd_bytes = strlen((char *)RBUF_CAPTURE_COMMAND_strings[RBUF_CAPTURE_COMMAND_SPKR_MGMT]);
    if ( 0 == strncmp((char *)cmd, (char *)str, cmd_bytes) )
    {
        command->command = RBUF_CAPTURE_COMMAND_SPKR_MGMT;
        goto found;
    }

    /*// Unknown Command*/
    return -1;

found:
    str += cmd_bytes;
    ptr = rbuf_capture_skip_whitespace(&str);
    if ( NULL == ptr )
    {
        /* Malformed command*/
        return -1;
    }
    command->args[0] = (uint32_t)strtol((char *)str, NULL, 0);
    str = ptr;
    ptr = rbuf_capture_skip_whitespace(&str);
    if ( NULL == ptr )
    {
        /* Malformed command*/
        return -1;
    }
    command->args[1] = (uint32_t)strtol((char *)str, NULL, 0);
    if ( rbuf_capture_app_player == 1 )
    {
        str = ptr;
        ptr = rbuf_capture_skip_whitespace(&str);
        if ( NULL == ptr )
        {
            /* Malformed command*/
            return -1;
        }
        command->args[2] = (uint32_t)strtol((char *)str, NULL, 0);
    }
    else
    {
        command->args[2] = RBUF_ID_INVALID;
    }

    /* Successful command / arguments*/
    return 0;
}

static
int32_t
rbuf_capture_parse_buffer(RBUF_CAPTURE_COMMAND_t *cmd,
                          uint32_t num_bytes)
{
    int32_t result = 0;
    uint32_t cur = 0;

    RBUF_ASSERT((NULL != cmd), "0x%08X", cmd);

    /* Parse each byte of the input buffer*/
    for ( cur = 0; cur < num_bytes; cur++ )
    {
        /* Command start delimiter is '#'*/
        if ( '#' == cmd->buffer[cur] )
        {
            /* Clear the arguments*/
            memset(cmd->args, 0, sizeof(cmd->args));
            /* Invalidate command type*/
            cmd->command = RBUF_CAPTURE_COMMAND_NUM_COMMANDS;
            result = rbuf_capture_command_parse(&cmd->buffer[cur], cmd);
            if ( result == 0 )
            {
                rbuf_capture_command_execute(cmd);
            }
            else
            {
                if ( -1 == result ) RBUF_PRINT(("Received Unknown Command"));
            }
        }
    }
    return 0;
}

/*
* Supplies a mechanism for the target to load a text file containing
* capture commands.  This file is specified via the environment variable
* 'rbuf_capture_socket_target_commands_filename' where its value contains
* the full target specific path and filename.  The commands file is
* evaluated once at startup ONLY.
*/
int32_t
rbuf_capture_target_commands_process(void)
{
    char *filename;
    FILE *file;
    /*int32_t  result;
    int8_t   buf[512];*/
    RBUF_CAPTURE_COMMAND_t cap_cmd;
    uint32_t eof;
    uint32_t bytes_read;
    static char *env = "rbuf_capture_socket_target_commands_filename";

    /* Get the environment variable*/
    filename = getenv(env);
    if ( NULL == filename )
    {
        /* Variable is not defined*/
        RBUF_PRINT(("*   %s=NULL", env));
        return 0;
    }

    RBUF_PRINT(("*   %s='%s'", env, filename));

    /* Open the specified file*/
    file = fopen(filename, "rt");
    if ( NULL == file )
    {
        /* File could not be opened*/
        RBUF_PRINT(("* !!!! ERROR: TARGET commands file '%s'"
                    " could NOT be opened, IGNORED!",
                    filename));
        return 0;
    }

    /* Read the input command file and process it*/
    eof = 0;
    while ( !eof )
    {
        bytes_read = fread(&cap_cmd.buffer[0], 1, sizeof(cap_cmd.buffer), file);
        if ( sizeof(cap_cmd.buffer) != bytes_read )
        {
            eof = 1;
        }
        rbuf_capture_parse_buffer(&cap_cmd, bytes_read);
    }
    return 0;
}

uint8_t
rbuf_capture_init(void *reg_handle,
                  void *heap)
{
    const char *config_string;

    printf("rbuf_init clled\n");
    /* Get environment variable settings*/
    config_string = getenv("rbuf_capture_enabled");
    rbuf_capture_enabled = (NULL != config_string) ?
        (uint32_t)strtol(config_string, NULL, 0) : 1;

    if ( !rbuf_capture_enabled )
    {
        /* Capture is not enabled, nothing else to do!*/
        rbuf_capture_debug_enabled = 0;
        rbuf_capture_app_player = 0;
        rbuf_capture_multifile = 0;
        return 0;
    }
    else
    {
        config_string = getenv("rbuf_capture_debug_enabled");
        rbuf_capture_debug_enabled = (NULL != config_string) ?
            (uint32_t)strtol(config_string, NULL, 0) : 0;

        config_string = getenv("rbuf_capture_multifile");
        rbuf_capture_multifile = (NULL != config_string) ?
            (uint32_t)strtol(config_string, NULL, 0) : 0;

        config_string = getenv("rbuf_capture_app_player");
        rbuf_capture_app_player = (NULL != config_string) ?
            (uint32_t)strtol(config_string, NULL, 0) : 0;

        /* Initialize the default path for captured files*/
        strcpy((char *)rbuf_capture_path, RBUF_CAPTURE_PATH_DEFAULT);
        /* Initialize the default filename for captured files*/
        strcpy((char *)rbuf_capture_filename, RBUF_CAPTURE_FILENAME_DEFAULT);
        /* Initialze the copy path to undefined (copy won't be done)*/
        rbuf_copy_path[0] = 0;

        RBUF_PRINT(("******************************************************************"));
        RBUF_PRINT(("*             *** RING BUFFER CAPTURE: VERSION 5.0 ***"));
        RBUF_PRINT(("*"));
        RBUF_DEBUG(("* Environment Variables:"));
        if ( rbuf_capture_debug_enabled )
        {
            RBUF_PRINT(("*   rbuf_capture_debug_enabled:    %d", rbuf_capture_debug_enabled));
        }
        if ( rbuf_capture_multifile )
        {
            RBUF_PRINT(("*   rbuf_capture_multifile:        %d", rbuf_capture_multifile));
        }
        if ( rbuf_capture_app_player )
        {
            RBUF_PRINT(("*   rbuf_capture_app_player:       %d", rbuf_capture_app_player));
        }
        RBUF_PRINT(("* Default Capture Path:            \"%s\"", rbuf_capture_path));
        RBUF_PRINT(("* Default Capture Filename:        \"%s\"", rbuf_capture_filename));
        RBUF_PRINT(("*"));
        RBUF_PRINT(("******************************************************************\n\n"));

        rbuf_reg_handle = reg_handle;
        rbuf_heap = heap;
        rbuf_capture_initialized = 0;

        rbuf_capture_target_commands_process();

        return 0;
    }
}

uint8_t
rbuf_capture_uninit(void)
{
    if ( rbuf_capture_enabled )
    {
        int32_t result;
        RBUF_PRINT(("Unintializing"));

        /* Remove the capture tasks*/
        if ( rbuf_capture_initialized )
        {
            /* Stop read from ring buffer to capture buffer thread*/
            if ( rbuf_capture_from_rbuf_to_cap_q_active )
            {
                RBUF_PRINT(("Stopping Read thread..."));
                rbuf_capture_from_rbuf_to_cap_q_active = 0;
                result = pthread_join(rbuf_capture_from_rbuf_to_cap_q_thread, NULL);
                RBUF_ASSERT((0 == result), "0x%08X", result);
                RBUF_PRINT(("[Stopped]"));
            }

            /* Stop write from capture buffer to file thread*/
            if ( rbuf_capture_from_cap_q_to_disk_active )
            {
                RBUF_PRINT(("Stopping write thread..."));
                rbuf_capture_from_cap_q_to_disk_active = 0;
                result = pthread_join(rbuf_capture_from_cap_q_to_disk_thread, NULL);
                RBUF_ASSERT((0 == result), "0x%08X", result);
                RBUF_PRINT(("[Stopped]"));
            }

            /* Uninitalize capture channels and free memory*/
            rbuf_capture_channels_uninit();
        }

        RBUF_PRINT(("******************************************************************"));
        RBUF_PRINT(("*             *** RING BUFFER CAPTURE DONE ***"));
        RBUF_PRINT(("******************************************************************\n\n"));

        return 0;
    }
    else
    {
        return 0;
    }
}

/* Intialize ring buffer capture interrupts if enabled*/
void brbuf_init_ringbuf_capture(void)
{
    if ( local_ringBufCB.rbuf_init == NULL || local_ringBufCB.rbuf_uninit == NULL )
    {
        int i, j;
        int count = 0;
        char buf[20];
        char *str_ptr = NULL;

        for ( i = 0; i < 2; i++ )
        {
            for ( j = 0; j < 8; j++ )
            {
                sprintf(&buf[0], "DECODER%1d%1d", i, j);
                str_ptr = getenv(&buf[0]);
                if ( NULL != str_ptr ) count++;
            }
        }

        for ( i = 0; i < 2; i++ )
        {
            for ( j = 0; j < 2; j++ )
            {
                sprintf(&buf[0], "PASSTHRU%1d%1d", i, j);
                str_ptr = getenv(&buf[0]);
                if ( NULL != str_ptr ) count++;
            }
        }

        local_ringBufCB.rbuf_init = rbuf_capture_init;
        local_ringBufCB.rbuf_uninit = rbuf_capture_uninit;

        if ( count == 0 )
        {
            local_ringBufCB.rbuf_capture_channel_start = rbuf_capture_channel_start;
            local_ringBufCB.rbuf_capture_stop_channel = rbuf_capture_stop_channel;
        }
        else
        {
            local_ringBufCB.rbuf_capture_channel_start = rbuf_capture_channel_start_dummy;
            local_ringBufCB.rbuf_capture_stop_channel = rbuf_capture_stop_channel_dummy;
        }

        BDSP_P_RbufSetup(local_ringBufCB);
        printf("RBUF_Setup Called\n");
    }

    return;
}

void start_rbuf_channels_from_env(void)
{
    /* Decoder Support only */
    int i, j, rbuf_id;
    char buf[20];
    char *str_ptr = NULL;
    int count = 0;

    for ( i = 0; i < 2; i++ )
    {
        for ( j = 0; j < 8; j++ )
        {
            sprintf(&buf[0], "DECODER%1d%1d", i, j);
            str_ptr = getenv(&buf[0]);
            if ( NULL != str_ptr )
            {
                sscanf(str_ptr, "%d", &rbuf_id);
                count++;
                /* Start All channels based on environment variables */
                /*printf ("Starting RBUF CAPTURE for DECODER [%d][%d] - rbuf id = %d\n\n", i, j, rbuf_id);*/
                rbuf_capture_channel_start(
                    i,
                    j,
                    rbuf_id,
                    BDSP_P_RbufType_eDecode);
            }
        }
    }

    for ( i = 0; i < 2; i++ )
    {
        for ( j = 0; j < 2; j++ )
        {
            sprintf(&buf[0], "PASSTHRU%1d%1d", i, j);
            str_ptr = getenv(&buf[0]);
            if ( NULL != str_ptr )
            {
                sscanf(str_ptr, "%d", &rbuf_id);
                count++;
                /*printf ("Starting RBUF CAPTURE for PASSTHRU [%d][%d] - rbuf id = %d\n\n", i, j, rbuf_id);*/
                /* Start All channels based on environment variables */
                rbuf_capture_channel_start(
                    i,
                    j,
                    rbuf_id,
                    BDSP_P_RbufType_ePassthru);
            }
        }
    }

    return;
}

/* Unintialize ring buffer capture interrupts if enabled*/
void brbuf_uninit_ringbuf_capture(void)
{
    rbuf_capture_uninit();
    if ( local_ringBufCB.rbuf_init != NULL || local_ringBufCB.rbuf_uninit != NULL )
    {
        local_ringBufCB.rbuf_init = NULL;
        local_ringBufCB.rbuf_uninit = NULL;
        local_ringBufCB.rbuf_capture_channel_start = NULL;
        local_ringBufCB.rbuf_capture_stop_channel = NULL;
        BDSP_P_RbufSetup(local_ringBufCB);
    }
    return;
}

int register_ringbuf_capture_callback(Rbuf_Channel_Info *ch_info, Rbuf_Callback_Info *cb_info)
{
    printf("Registering call back\n");
    if ( rbuf_capture_initialized == 0 )
    {
        rbuf_capture_startup();
    }
    rbuf_capture_channel_add(ch_info->path, ch_info->channel, ch_info->rbuf_id, ch_info->rb_type, cb_info);
    return 0;
}

#else

void brbuf_init_ringbuf_capture(void)
{
    return;
}

#endif /* BDSP_FW_RBUF_CAPTURE */
