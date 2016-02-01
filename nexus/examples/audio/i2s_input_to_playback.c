/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
#if NEXUS_HAS_AUDIO && NEXUS_NUM_AUDIO_CAPTURES && NEXUS_NUM_I2S_INPUTS
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "nexus_i2s_input.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_gpio.h"
#include "nexus_audio_input_capture.h"
#include "nexus_audio_capture.h"
#include "nexus_core_utils.h"
#include "nexus_audio_playback.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

BDBG_MODULE(i2s_input_to_playback);

#define USE_OUTPUT_CAPTURE      1
#define SAMPLERATE              48000
#define CAPTURE_BUFFER_SIZE     (1*128*1024) /* 128KB per PCM channel pair */

char temp_buff[CAPTURE_BUFFER_SIZE];
static struct timeval  __mclock_start;

NEXUS_AudioPlaybackHandle g_playbackHandle = NULL;
static bool enable_capture_thread = true;
BKNI_EventHandle event = NULL;

void mclock_init( void )
{
    gettimeofday( &__mclock_start, NULL );
}

long int mclock( void )
{
    struct timeval timecurrent;
    struct timeval timeresult;

    gettimeofday( &timecurrent, NULL );

    timeresult.tv_sec  = timecurrent.tv_sec  - __mclock_start.tv_sec;
    timeresult.tv_usec = timecurrent.tv_usec - __mclock_start.tv_usec;

    if( timeresult.tv_usec < 0 )
    {
        timeresult.tv_sec--;
        timeresult.tv_usec += 1000000;
    }

    return timeresult.tv_sec * 1000 + ( timeresult.tv_usec / 1000 );
}

static void capture_callback(void *pParam1, int param2)
{
    /**
    printf("Data callback - channel 0x%08x\n", (unsigned)pParam1);
    **/
    pParam1=pParam1;    /*unused*/
    BKNI_SetEvent((BKNI_EventHandle)param2);
}

static void* capture_thread(void *pParam);

static void* capture_thread(void *pParam)
{
    void *cap_buffer;
    void *play_buffer;
    size_t cap_buff_size, play_buff_size, wsize=0;
    size_t byteToPlay=0;
    #if USE_OUTPUT_CAPTURE
    NEXUS_AudioCaptureHandle capture = pParam;
    #else /* Input Capture Only */
    NEXUS_AudioInputCaptureHandle capture = pParam;
    #endif
    /**FILE *pFile = (FILE *)param;**/
    NEXUS_Error errCode;
    long elapse_msec;

    while ( enable_capture_thread )
    {
        elapse_msec=mclock(); /* print elapse time */

        if (elapse_msec > 7) {
            fprintf(stderr, "elapse_msec : %ld m Sec \n", elapse_msec);
        }

        #if USE_OUTPUT_CAPTURE
        NEXUS_AudioCapture_GetBuffer(capture, (void **)&cap_buffer, &cap_buff_size);
        #else
        NEXUS_AudioInputCapture_GetBuffer(capture, (void **)&cap_buffer, &cap_buff_size);
        #endif

        if(cap_buff_size > 0) {
            /*printf("received %d bytes from capture\n", cap_buff_size);*/
            byteToPlay = cap_buff_size;
            while (byteToPlay > 0) {
                errCode = NEXUS_AudioPlayback_GetBuffer(g_playbackHandle, (void **)&play_buffer, &play_buff_size);
                if ( errCode ) {
                    printf("Error getting playback buffer\n");
                    break;
                }

                if (play_buff_size == 0) continue;

                if ( play_buff_size > byteToPlay ) {
                    wsize = byteToPlay;
                    byteToPlay = 0;
                }
                else { /* if play_buff_size < byteToPlay */
                    wsize = play_buff_size;
                    byteToPlay -= play_buff_size;
                }

                BKNI_Memcpy(play_buffer, cap_buffer, wsize);

                /*printf("  added %d bytes to playback\n", wsize);*/
                errCode = NEXUS_AudioPlayback_WriteComplete(g_playbackHandle, wsize);
                if ( errCode ) {
                    printf("Error committing playback buffer\n");
                    break;
                }
            }

            #if USE_OUTPUT_CAPTURE
            errCode=NEXUS_AudioCapture_ReadComplete(capture, cap_buff_size);
            #else
            errCode=NEXUS_AudioInputCapture_ReadComplete(capture, cap_buff_size);
            #endif
            if ( errCode ) {
                fprintf(stderr, "Error committing capture buffer\n");
                #if USE_OUTPUT_CAPTURE
                NEXUS_AudioCapture_Stop(capture);
                #else
                NEXUS_AudioInputCapture_Stop(capture);
                #endif
                enable_capture_thread = false;
            }
            /*printf("completed %d bytes from capture\n", cap_buff_size);*/
        }
        mclock_init();  /* Save Current Time */

        /* Wait for data callback */
        errCode = BKNI_WaitForEvent(event, 1000);
        if ( errCode )
        {
            printf("error waiting for event\n");
        }
    }

    return NULL;
}

void SleepMsec(long a_nMSec)
{
    struct timeval stTimeVal;

    stTimeVal.tv_sec = a_nMSec / 1000;
    a_nMSec -= stTimeVal.tv_sec * 1000;
    stTimeVal.tv_usec = a_nMSec * 1000;

    select(0, 0, 0, 0, &stTimeVal);
}

int main(int argc, char *argv[])
{
    NEXUS_Error errCode;
    NEXUS_I2sInputHandle i2sInput;
    NEXUS_PlatformConfiguration platformConfig;
    pthread_t captureThread;

    #if !USE_OUTPUT_CAPTURE
    NEXUS_AudioInputCaptureHandle inputCapture;
    NEXUS_AudioInputCaptureOpenSettings icOpenSettings;
    NEXUS_AudioInputCaptureStartSettings icStartSettings;
    NEXUS_AudioInputCaptureSettings icSettings;
    #endif

    #if USE_OUTPUT_CAPTURE
    NEXUS_AudioCaptureHandle outputCapture;
    NEXUS_AudioCaptureOpenSettings ocOpenSettings;
    NEXUS_AudioCaptureStartSettings ocStartSettings;
    #endif

    NEXUS_AudioPlaybackOpenSettings pbOpenSettings;
    NEXUS_AudioPlaybackStartSettings settings;

    NEXUS_AudioOutputSettings outputSettings;

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    BSTD_UNUSED(argv);
    BSTD_UNUSED(argc);

    BKNI_CreateEvent(&event);

    /* Set timebase 0 to I2S input */
    {
        NEXUS_TimebaseSettings timebaseSettings;
        NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0, &timebaseSettings);

        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eI2sIn;
        timebaseSettings.sourceSettings.i2s.index = 0;
        timebaseSettings.sourceSettings.i2s.sampleRate = SAMPLERATE;
        NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0, &timebaseSettings);
    }

    i2sInput = NEXUS_I2sInput_Open(0, NULL);
    if ( NULL == i2sInput ) {
        fprintf(stderr, "Unable to open i2s input 0\n");
        return -1;
    }

    #if USE_OUTPUT_CAPTURE
    NEXUS_AudioCapture_GetDefaultOpenSettings(&ocOpenSettings);
    ocOpenSettings.fifoSize = CAPTURE_BUFFER_SIZE;
    ocOpenSettings.format = NEXUS_AudioCaptureFormat_e16BitStereo;
    ocOpenSettings.threshold = ocOpenSettings.fifoSize / 1024;
    outputCapture = NEXUS_AudioCapture_Open(0, &ocOpenSettings);
    if ( NULL == outputCapture ) {
        fprintf(stderr, "Unable to open outputCapture channel\n");
        return 0;
    }
    BDBG_ERR(("Opened in OutputCapture->Memory"));
    BDBG_ERR((">>>> capture format is %u", ocOpenSettings.format));
    BDBG_ERR((">>>> capture fifoSize is %u", ocOpenSettings.fifoSize));
    BDBG_ERR((">>>> capture threshold is %u", ocOpenSettings.threshold));
    #else
    NEXUS_AudioInputCapture_GetDefaultOpenSettings(&icOpenSettings);
    icOpenSettings.fifoSize = CAPTURE_BUFFER_SIZE;
    icOpenSettings.format = NEXUS_AudioCaptureFormat_e16BitStereo;
    icOpenSettings.threshold = icOpenSettings.fifoSize / 1024;
    inputCapture = NEXUS_AudioInputCapture_Open(0, &icOpenSettings);
    if ( NULL == inputCapture ) {
        fprintf(stderr, "Unable to open inputCapture channel\n");
        return 0;
    }
    BDBG_ERR(("Opened in InputCapture->Memory"));
    BDBG_ERR((">>>> capture format is %u", icOpenSettings.format));
    BDBG_ERR((">>>> capture fifoSize is %u", icOpenSettings.fifoSize));
    BDBG_ERR((">>>> capture threshold is %u", icOpenSettings.threshold));
    #endif

    NEXUS_AudioPlayback_GetDefaultOpenSettings(&pbOpenSettings);
    pbOpenSettings.fifoSize = CAPTURE_BUFFER_SIZE*2;
    g_playbackHandle = NEXUS_AudioPlayback_Open(0, &pbOpenSettings);
    if ( NULL == g_playbackHandle )
    {
        fprintf(stderr, "Unable to open playback channel\n");
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        /** goto done; **/
    }

    /* Connect Playback -> Outputs */
    #if NEXUS_NUM_AUDIO_DACS > 0
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioPlayback_GetConnector(g_playbackHandle));
    #endif

    #if NEXUS_NUM_I2S_OUTPUTS > 0
    NEXUS_AudioOutput_AddInput(
	NEXUS_I2sOutput_GetConnector(platformConfig.outputs.i2s[0]),
        NEXUS_AudioPlayback_GetConnector(g_playbackHandle));
    #endif

    #if NEXUS_NUM_SPDIF_OUTPUTS > 0
    NEXUS_AudioOutput_AddInput(
        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
        NEXUS_AudioPlayback_GetConnector(g_playbackHandle));
    #endif

    /* Set Timebase for outputs */
    #if NEXUS_NUM_AUDIO_DACS > 0
    NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), &outputSettings);
    outputSettings.timebase = NEXUS_Timebase_e0;
    NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), &outputSettings);
    #endif

    #if NEXUS_NUM_I2S_OUTPUTS > 0
    NEXUS_AudioOutput_GetSettings(NEXUS_I2sOutput_GetConnector(platformConfig.outputs.i2s[0]), &outputSettings);
    outputSettings.timebase = NEXUS_Timebase_e0;
    outputSettings.pll = NEXUS_AudioOutputPll_e0;
    NEXUS_AudioOutput_SetSettings(NEXUS_I2sOutput_GetConnector(platformConfig.outputs.i2s[0]), &outputSettings);
    #endif

    #if NEXUS_NUM_SPDIF_OUTPUTS > 0
    NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), &outputSettings);
    outputSettings.timebase = NEXUS_Timebase_e0;
    outputSettings.pll = NEXUS_AudioOutputPll_e0;
    NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), &outputSettings);
    #endif

    /* Start thread for playback */
    #if USE_OUTPUT_CAPTURE
    pthread_create(&captureThread, NULL, capture_thread, outputCapture);
    #else
    pthread_create(&captureThread, NULL, capture_thread, inputCapture);
    #endif

    #if USE_OUTPUT_CAPTURE
    /* set audio outputCapture to use Timebase e0 */
    NEXUS_AudioOutput_GetSettings(NEXUS_AudioCapture_GetConnector(outputCapture), &outputSettings);
    outputSettings.timebase = NEXUS_Timebase_e0;
    outputSettings.nco = NEXUS_AudioOutputNco_e0;
    NEXUS_AudioOutput_SetSettings(NEXUS_AudioCapture_GetConnector(outputCapture), &outputSettings);

    /* Connect I2sInput -> AudioCapture */
    NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(outputCapture), NEXUS_I2sInput_GetConnector(i2sInput));

    /** start Audio Capture **/
    NEXUS_AudioCapture_GetDefaultStartSettings(&ocStartSettings);
    ocStartSettings.dataCallback.callback = capture_callback;
    ocStartSettings.dataCallback.context = outputCapture;
    ocStartSettings.dataCallback.param = (int)event;
    NEXUS_AudioCapture_Start(outputCapture, &ocStartSettings);

    /** Start I2s Input **/
    errCode = NEXUS_I2sInput_Start(i2sInput);
    if ( errCode ) {
        fprintf(stderr, "Unable to start I2S Capture\n");
        return -1;
    }
    #else
    NEXUS_AudioInputCapture_GetSettings(inputCapture, &icSettings);
    icSettings.dataCallback.callback = capture_callback;
    icSettings.dataCallback.context = inputCapture;
    icSettings.dataCallback.param = (int)event;
    NEXUS_AudioInputCapture_SetSettings(inputCapture, &icSettings);

    NEXUS_AudioInputCapture_GetDefaultStartSettings(&icStartSettings);
    icStartSettings.input = NEXUS_I2sInput_GetConnector(i2sInput);
    NEXUS_AudioInputCapture_Start(inputCapture, &icStartSettings);
    #endif

    /** start playback **/
    NEXUS_AudioPlayback_GetDefaultStartSettings(&settings);
    settings.bitsPerSample = 16;
    settings.sampleRate = SAMPLERATE;
    settings.signedData = true;
    errCode = NEXUS_AudioPlayback_Start(g_playbackHandle, &settings);
    BDBG_ASSERT(!errCode);

    fprintf(stderr, "Press ENTER to quit\n");
    getchar();

    enable_capture_thread = false;
    BKNI_SetEvent(event);
    pthread_join(captureThread, NULL);

    fprintf(stderr, "Stopping capture\n");
    #if USE_OUTPUT_CAPTURE
    NEXUS_I2sInput_Stop(i2sInput);
    NEXUS_AudioCapture_Stop(outputCapture);
    #else
    NEXUS_AudioInputCapture_Stop(inputCapture);
    #endif
    NEXUS_AudioPlayback_Stop(g_playbackHandle);

    #if NEXUS_NUM_I2S_OUTPUTS > 0
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_I2sOutput_GetConnector(platformConfig.outputs.i2s[0]));
    #endif
    #if NEXUS_NUM_AUDIO_DACS > 0
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
    #endif
    #if NEXUS_NUM_SPDIF_OUTPUTS > 0
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
    #endif
    NEXUS_AudioInput_Shutdown(NEXUS_I2sInput_GetConnector(i2sInput));

    #if USE_OUTPUT_CAPTURE
    NEXUS_AudioCapture_Close(outputCapture);
    #else
    NEXUS_AudioInputCapture_Close(inputCapture);
    #endif
    NEXUS_AudioPlayback_Close(g_playbackHandle);

    NEXUS_I2sInput_Close(i2sInput);

    BKNI_DestroyEvent(event);
    NEXUS_Platform_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform.\n");
    return 0;
}
#endif
