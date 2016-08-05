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
 * Module Description:
 *
 *****************************************************************************/
/* automated audio module unittest. mostly based on audio_test.c unittest, but automated */
#include "nexus_platform.h"
#if NEXUS_NUM_SPDIF_OUTPUTS && NEXUS_NUM_AUDIO_DACS
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_audio_decoder.h"
static NEXUS_AudioDecoderHandle g_audioDecoders[2];
#include "nexus_audio_dac.h"
static NEXUS_AudioDacHandle g_audioDac;
static NEXUS_AudioDecoderStartSettings g_audioProgram;
static NEXUS_AudioDecoderStatus g_audioStatus;
#if NEXUS_NUM_SPDIF_OUTPUTS
#include "nexus_spdif_output.h"
static NEXUS_SpdifOutputHandle g_spdifOutput;
static bool g_compressedSpdif = false;
static bool g_simulMode = false;
#endif
#if NEXUS_NUM_SPDIF_INPUTS
#include "nexus_spdif_input.h"
static NEXUS_SpdifInputHandle g_spdifInput;
#endif
#if NEXUS_NUM_I2S_INPUTS
#include "nexus_i2s_input.h"
static NEXUS_I2sInputHandle g_i2sInput;
#endif
#include "nexus_auto_volume_level.h"
static NEXUS_AutoVolumeLevelHandle g_avl;
static bool g_avlEnabled = false;

#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
static NEXUS_FrontendHandle g_frontend;
#endif

static bool g_useStreamer;

#include "nexus_parser_band.h"
#include "nexus_input_band.h"
#include "nexus_pid_channel.h"
#include "nexus_timebase.h"
#include "nexus_stc_channel.h"
static NEXUS_StcChannelHandle g_stcChannel;
static NEXUS_PidChannelHandle g_audioPid, g_pcrPid;

#include <stdio.h>

#ifdef IP_STREAMER_SUBST
#include "ip_streamer_subst.h"
#endif

BDBG_MODULE(audio_test_auto);

typedef enum TestInput
{
    TestInput_eNone,
    TestInput_eDecoder,
    TestInput_eSpdif,
    TestInput_eI2s,
    /* TODO: add hdmi input */
    TestInput_eMax
} TestInput;

static TestInput g_input=TestInput_eNone;

static const char *g_inputNames[TestInput_eMax] =
{
    "None",
    "Decoder",
    "SPDIF",
    "I2S",
};

static void SelectInput(TestInput input);
static void RestartInput(void);
static void SetOutputSettings(void);

#define STOP_DECODER_AND_CHECK(decoderNum) \
    NEXUS_AudioDecoder_Stop(g_audioDecoders[decoderNum]); \
    errCode = NEXUS_AudioDecoder_GetStatus(g_audioDecoders[decoderNum], &g_audioStatus); \
    BDBG_ASSERT(NEXUS_SUCCESS == errCode && g_audioStatus.started == false);

#define START_DECODER_AND_CHECK(decoderNum) \
    errCode = NEXUS_AudioDecoder_Start(g_audioDecoders[decoderNum], &g_audioProgram); \
    BDBG_ASSERT(NEXUS_SUCCESS == errCode); \
    errCode = NEXUS_AudioDecoder_GetStatus(g_audioDecoders[decoderNum], &g_audioStatus); \
    BDBG_ASSERT(NEXUS_SUCCESS == errCode && g_audioStatus.started);

#define STOP_RF_DECODER_AND_CHECK() \
    NEXUS_RfAudioDecoder_Stop(g_rfDecoder); \
    NEXUS_RfAudioDecoder_GetStatus(g_rfDecoder, &g_rfAudioStatus); \
    BDBG_ASSERT(g_rfAudioStatus.started == false);

#define START_RF_DECODER_AND_CHECK() \
    errCode = NEXUS_RfAudioDecoder_Start(g_rfDecoder); \
    BDBG_ASSERT(NEXUS_SUCCESS == errCode); \
    NEXUS_RfAudioDecoder_GetStatus(g_rfDecoder, &g_rfAudioStatus); \
    BDBG_ASSERT(g_rfAudioStatus.started);

#define STOP_ANALOG_DECODER_AND_CHECK() \
    NEXUS_AnalogAudioDecoder_Stop(g_analogDecoder); \
    NEXUS_AnalogAudioDecoder_GetStatus(g_analogDecoder, &g_analogAudioStatus); \
    BDBG_ASSERT(g_analogAudioStatus.started == false); \

#define START_ANALOG_DECODER_AND_CHECK() \
    errCode = NEXUS_AnalogAudioDecoder_Start(g_analogDecoder); \
    BDBG_ASSERT(NEXUS_SUCCESS == errCode); \
    NEXUS_AnalogAudioDecoder_GetStatus(g_analogDecoder, &g_analogAudioStatus); \
    BDBG_ASSERT(g_analogAudioStatus.started);

#define INPUT_PCR_PID 0x14
#define INPUT_AUD_PID 0x11

int main(int argc, char **argv)
{
    NEXUS_Error errCode;
    NEXUS_PlatformConfiguration config;
    NEXUS_ParserBandSettings parserSettings;
#if NEXUS_HAS_FRONTEND
    int tmp;
    NEXUS_FrontendCapabilities frontendCaps;
#endif
    NEXUS_StcChannelSettings stcSettings;
    int i,j,k;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
#ifdef IP_STREAMER_SUBST
    IpsHandle ips;
    IpsOpenSettings ipsOpenSettings;
    IpsStartSettings ipsStartSettings;
#endif
    argc=argc;
    argv=argv;

    /* Boot Nexus */
    errCode = NEXUS_Platform_Init(NULL);
    BDBG_ASSERT(NEXUS_SUCCESS == errCode);

    /* Get outputs */
    NEXUS_Platform_GetConfiguration(&config);

    /* Open inputs */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
#ifdef IP_STREAMER_SUBST
    IpsAddAudioDecoderOpenSettings(ips, &audioDecoderOpenSettings);
#endif
    g_audioDecoders[0] = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
    BDBG_ASSERT(NULL != g_audioDecoders[0]);
    g_audioDecoders[1] = NEXUS_AudioDecoder_Open(1, &audioDecoderOpenSettings);
    BDBG_ASSERT(NULL != g_audioDecoders[1]);

    #if NEXUS_NUM_SPDIF_INPUTS
    g_spdifInput = NEXUS_SpdifInput_Open(0, NULL);
    BDBG_ASSERT(NULL != g_spdifInput);
    #endif

    #if NEXUS_NUM_I2S_INPUTS
    g_i2sInput = NEXUS_I2sInput_Open(0, NULL);
    BDBG_ASSERT(NULL != g_i2sInput);
    #endif

    g_avl = NEXUS_AutoVolumeLevel_Open(NULL);

    g_stcChannel = NEXUS_StcChannel_Open(0, NULL);
    BDBG_ASSERT(NULL != g_stcChannel);

    /* Set up default digital channel */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&g_audioProgram);
    /* Get program settings at startup */
    g_audioProgram.stcChannel = NULL; /* g_stcChannel; */
    NEXUS_ParserBand_GetSettings(NEXUS_ParserBand_e0, &parserSettings);
    parserSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;

#if NEXUS_HAS_FRONTEND
    /* Find VSB tuner */
    for ( tmp = 0; config.frontend[tmp] != NULL; tmp++ )
    {
        NEXUS_Frontend_GetCapabilities(config.frontend[tmp], &frontendCaps);
        if ( frontendCaps.vsb )
        {
            g_frontend = config.frontend[tmp];
            break;
        }
    }
#endif

    /* default to streamer ... */
    NEXUS_Platform_GetStreamerInputBand(0, &parserSettings.sourceTypeSettings.inputBand);
    g_useStreamer = true;

    errCode = NEXUS_ParserBand_SetSettings(NEXUS_ParserBand_e0, &parserSettings);
    BDBG_ASSERT(NEXUS_SUCCESS == errCode);
#ifdef IP_STREAMER_SUBST
    ipsOpenSettings.parserBand=NEXUS_ParserBand_e0;
    ipsOpenSettings.playpumpIndex = 0;
    ipsOpenSettings.remuxIndex = 0;
    ips = IpsOpen(&ipsOpenSettings);
#endif

    g_audioPid = NEXUS_PidChannel_Open(NEXUS_ParserBand_e0, INPUT_AUD_PID, NULL);
    BDBG_ASSERT(NULL != g_audioPid);

    g_pcrPid = NEXUS_PidChannel_Open(NEXUS_ParserBand_e0, INPUT_PCR_PID, NULL);
    BDBG_ASSERT(NULL != g_pcrPid);

    NEXUS_StcChannel_GetSettings(g_stcChannel, &stcSettings);
    stcSettings.modeSettings.pcr.pidChannel = g_pcrPid;
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr;
    stcSettings.autoConfigTimebase = true;
#ifdef IP_STREAMER_SUBST
    IpsAddStcSettings(ips, &stcSettings);
#endif
    errCode = NEXUS_StcChannel_SetSettings(g_stcChannel, &stcSettings);
    BDBG_ASSERT(NEXUS_SUCCESS == errCode);

#ifdef IP_STREAMER_SUBST
    ipsStartSettings.vDecoder = NULL;
    ipsStartSettings.aDecoder = g_audioDecoders[0];
    ipsStartSettings.stcChannel = NULL;
    IpsStart(ips, &ipsStartSettings);
#endif

    for (i=0; i<NEXUS_NUM_AUDIO_DACS; i++) { /* cycle through audio DAC outputs */
#if NEXUS_NUM_AUDIO_DACS
        g_audioDac = config.outputs.audioDacs[i];
#endif

        for (j=1; j<NEXUS_NUM_SPDIF_OUTPUTS*3+1; j++) { /* cycle through SPDIF outputs and settings */
            g_spdifOutput = NULL;
            g_compressedSpdif = NULL;
            g_simulMode = NULL;

            if (j) {
                g_spdifOutput = config.outputs.spdif[(j-1)/3];
            }
            if (j%3==2) {
                g_compressedSpdif = true;
            }
            if (j&&(j%3==0)) {
                /* compressed & simul only works for AC3 or WMAPro.
                g_compressedSpdif = true;
                */
                g_simulMode = true;
            }

            SetOutputSettings();
            for (k=0; k<TestInput_eMax; k++) { /* cycle through test inputs */
                printf("test DAC(%d), SPDIF(%d,compr=%d,simul=%d), input: %s\n", i,
                    j?(j-1)/3:-1, g_compressedSpdif?1:0, g_simulMode?1:0, g_inputNames[k]);
                SelectInput((TestInput)k);
                BKNI_Sleep(1000);
                printf("restart input: %s\n", g_inputNames[k]);
                RestartInput();
                BKNI_Sleep(1000);

                if ( g_avl )
                {
                    g_avlEnabled = true;
                    printf("select input: %s with avlEnabled\n", g_inputNames[k]);
                    SelectInput(k);
                    BKNI_Sleep(1000);
                    g_avlEnabled = false;
                }
            }
        }
    }
    printf("test completed\n");

    /* shutdown */
    STOP_DECODER_AND_CHECK(0);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0], NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0], NEXUS_AudioDecoderConnectorType_eMultichannel));
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0], NEXUS_AudioDecoderConnectorType_eCompressed));
    NEXUS_AudioDecoder_Close(g_audioDecoders[0]);

    STOP_DECODER_AND_CHECK(1);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(g_audioDecoders[1], NEXUS_AudioDecoderConnectorType_eCompressed));
    NEXUS_AudioDecoder_Close(g_audioDecoders[1]);

    #if NEXUS_NUM_I2S_INPUTS
    NEXUS_I2sInput_Stop(g_i2sInput);
    NEXUS_I2sInput_Close(g_i2sInput);
    #endif

    if ( g_avl )
    {
        NEXUS_AutoVolumeLevel_RemoveAllInputs(g_avl);
        NEXUS_AutoVolumeLevel_Close(g_avl);
    }

    NEXUS_PidChannel_Close(g_audioPid);
    NEXUS_PidChannel_Close(g_pcrPid);
#ifdef IP_STREAMER_SUBST
    IpsClose(ips);
#endif
    NEXUS_Platform_Uninit();

    /* TODO: despite the above, the shutdown doesn't seem to happen cleanly.
    if you run this app twice in a row, it'll behave strangely the second time around and fail the first assert. */
    return 0;

}

static void SelectInput(TestInput input)
{
    NEXUS_Error errCode;
    printf("enter SelectInput with %d\n", input);

    /* Untune current selection */
    switch ( g_input )
    {
    case TestInput_eNone:
        break;
    case TestInput_eDecoder:
    #if NEXUS_NUM_SPDIF_INPUTS
    case TestInput_eSpdif:
    #endif
        STOP_DECODER_AND_CHECK(0);
#if NEXUS_NUM_SPDIF_OUTPUTS
        if ( g_spdifOutput && g_compressedSpdif && !g_simulMode && g_input == TestInput_eDecoder )
        {
            STOP_DECODER_AND_CHECK(1);
        }
#endif
        break;
    #if NEXUS_NUM_I2S_INPUTS
    case TestInput_eI2s:
        NEXUS_I2sInput_Stop(g_i2sInput);
        break;
    #endif
    default:
        printf("Unrecognized input %d\n", g_input);
        break;
    }

    if ( g_avl )
        NEXUS_AutoVolumeLevel_RemoveAllInputs(g_avl);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_audioDac));
#endif

#if NEXUS_NUM_SPDIF_OUTPUTS
    if ( g_spdifOutput )
    {
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(g_spdifOutput));
    }
#endif
    g_input = TestInput_eNone;

    /* Setup new input */
    switch ( input )
    {
    case TestInput_eNone:
        /* Nothing to do */
        return;
    case TestInput_eDecoder:
        if ( g_avl && g_avlEnabled )
        {
            errCode = NEXUS_AutoVolumeLevel_AddInput(g_avl, NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0], NEXUS_AudioDecoderConnectorType_eStereo));
            BDBG_ASSERT(NEXUS_SUCCESS == errCode);
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_AutoVolumeLevel_GetConnector(g_avl));
            BDBG_ASSERT(NEXUS_SUCCESS == errCode);
        }
        else
        {
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac),
                                                 NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0], NEXUS_AudioDecoderConnectorType_eStereo));
            BDBG_ASSERT(NEXUS_SUCCESS == errCode);
        }
#if NEXUS_NUM_SPDIF_OUTPUTS
        if ( g_spdifOutput )
        {
            NEXUS_AudioInputHandle decoderConnector;
            unsigned whichDecoder = (g_compressedSpdif&&!g_simulMode)?1:0;
            decoderConnector = NEXUS_AudioDecoder_GetConnector(g_audioDecoders[whichDecoder],
                                                               (g_compressedSpdif)?NEXUS_AudioDecoderConnectorType_eCompressed:
                                                                                   NEXUS_AudioDecoderConnectorType_eStereo);
            if ( g_avl && g_avlEnabled && !g_compressedSpdif )
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_spdifOutput),
                                                     NEXUS_AutoVolumeLevel_GetConnector(g_avl));
                BDBG_ASSERT(NEXUS_SUCCESS == errCode);
            }
            else
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_spdifOutput), decoderConnector);
                BDBG_ASSERT(NEXUS_SUCCESS == errCode);
            }
        }
#endif
        NEXUS_AudioDecoder_GetDefaultStartSettings(&g_audioProgram);
        g_audioProgram.stcChannel = NULL; /* g_stcChannel; */
        g_audioProgram.pidChannel = g_audioPid;
        START_DECODER_AND_CHECK(0);
#if NEXUS_NUM_SPDIF_OUTPUTS
        if ( g_spdifOutput && g_compressedSpdif && !g_simulMode )
        {
            START_DECODER_AND_CHECK(1);
        }
#endif
        break;
    #if NEXUS_NUM_SPDIF_INPUTS
    case TestInput_eSpdif:
        if ( g_avl && g_avlEnabled )
        {
            errCode = NEXUS_AutoVolumeLevel_AddInput(g_avl, NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0], NEXUS_AudioDecoderConnectorType_eStereo));
            BDBG_ASSERT(NEXUS_SUCCESS == errCode);
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_AutoVolumeLevel_GetConnector(g_avl));
            BDBG_ASSERT(NEXUS_SUCCESS == errCode);
        }
        else
        {
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac),
                                                 NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0], NEXUS_AudioDecoderConnectorType_eStereo));
            BDBG_ASSERT(NEXUS_SUCCESS == errCode);
        }
#if NEXUS_NUM_SPDIF_OUTPUTS
        if ( g_spdifOutput )
        {
            if ( g_avl && g_avlEnabled && !g_compressedSpdif )
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_spdifOutput), NEXUS_AutoVolumeLevel_GetConnector(g_avl));
                BDBG_ASSERT(NEXUS_SUCCESS == errCode);
            }
            else
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_spdifOutput),
                                                     NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0],
                                                                                     (g_compressedSpdif)?NEXUS_AudioDecoderConnectorType_eCompressed:
                                                                                                         NEXUS_AudioDecoderConnectorType_eStereo));
                BDBG_ASSERT(NEXUS_SUCCESS == errCode);
            }
        }
#endif

        NEXUS_AudioDecoder_GetDefaultStartSettings(&g_audioProgram);
        g_audioProgram.input = NEXUS_SpdifInput_GetConnector(g_spdifInput);
        START_DECODER_AND_CHECK(0);
        break;
    #endif
    #if NEXUS_NUM_I2S_INPUTS
    case TestInput_eI2s:
        errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_I2sInput_GetConnector(g_i2sInput));
        BDBG_ASSERT(NEXUS_SUCCESS == errCode);
#if NEXUS_NUM_SPDIF_OUTPUTS
        if ( g_spdifOutput )
        {
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_spdifOutput), NEXUS_I2sInput_GetConnector(g_i2sInput));
            BDBG_ASSERT(NEXUS_SUCCESS == errCode);
        }
#endif
        errCode = NEXUS_I2sInput_Start(g_i2sInput);
        BDBG_ASSERT(NEXUS_SUCCESS == errCode);
        break;
    #endif
    default:
        printf("Unrecognized input %d\n", input);
        break;
    }

    g_input = input;
}

static void RestartInput(void)
{
    NEXUS_Error errCode;
    switch ( g_input )
    {
    case TestInput_eNone:
        /* Nothing to do */
        return;
    case TestInput_eDecoder:
        STOP_DECODER_AND_CHECK(0);
#if NEXUS_NUM_SPDIF_OUTPUTS
        if ( g_spdifOutput && g_compressedSpdif && !g_simulMode && g_input == TestInput_eDecoder )
        {
            STOP_DECODER_AND_CHECK(1);
        }
#endif
        START_DECODER_AND_CHECK(0);
#if NEXUS_NUM_SPDIF_OUTPUTS
        if ( g_spdifOutput && g_compressedSpdif && !g_simulMode && g_input == TestInput_eDecoder )
        {
            START_DECODER_AND_CHECK(1);
        }
#endif
        break;
    #if NEXUS_NUM_SPDIF_INPUTS
    case TestInput_eSpdif:
        STOP_DECODER_AND_CHECK(0);
        START_DECODER_AND_CHECK(0);
        break;
    #endif
    #if NEXUS_NUM_I2S_INPUTS
    case TestInput_eI2s:
        NEXUS_I2sInput_Stop(g_i2sInput);
        errCode = NEXUS_I2sInput_Start(g_i2sInput);
        BDBG_ASSERT(NEXUS_SUCCESS == errCode);
        break;
    #endif
    default:
        printf("Unrecognized input %d\n", g_input);
        return;
    }
}

static void SetOutputSettings(void)
{
    NEXUS_Error errCode;
    NEXUS_AudioOutputSettings outputSettings;
    NEXUS_AudioOutput output;

    output = NEXUS_AudioDac_GetConnector(g_audioDac);
    NEXUS_AudioOutput_GetSettings(output, &outputSettings);
    /* TODO: could try changing various settings here */
    errCode = NEXUS_AudioOutput_SetSettings(output, &outputSettings);
    BDBG_ASSERT(NEXUS_SUCCESS == errCode);

#if NEXUS_NUM_SPDIF_OUTPUTS
    if ( g_spdifOutput ) {
        output = NEXUS_SpdifOutput_GetConnector(g_spdifOutput);
        NEXUS_AudioOutput_GetSettings(output, &outputSettings);
        errCode = NEXUS_AudioOutput_SetSettings(output, &outputSettings);
        BDBG_ASSERT(NEXUS_SUCCESS == errCode);
    }
#endif

    return;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return -1;
}
#endif
