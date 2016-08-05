/***************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 **************************************************************************/
#include "nexus_platform.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_audio_decoder.h"
static NEXUS_AudioDecoderHandle g_audioDecoders[2];
#include "nexus_audio_dac.h"
static NEXUS_AudioDacHandle g_audioDac;
static NEXUS_AudioDecoderStartSettings g_audioProgram;
static NEXUS_AudioCodec g_audioCodec;
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
#if NEXUS_NUM_RF_AUDIO_DECODERS
#include "nexus_rf_audio_decoder.h"
static NEXUS_RfAudioDecoderHandle g_rfDecoder;
#endif
#if NEXUS_NUM_ANALOG_AUDIO_DECODERS
#include "nexus_analog_audio_input.h"
#include "nexus_analog_audio_decoder.h"
static NEXUS_AnalogAudioInputHandle g_analogInput;
static NEXUS_AnalogAudioDecoderHandle g_analogDecoder;
#endif
#if NEXUS_DTV_PLATFORM
#include "nexus_auto_volume_level.h"
#include "nexus_audio_equalizer.h"
static NEXUS_AutoVolumeLevelHandle g_avl;
static bool g_avlEnabled = false;
#endif

#if BCHP_CHIP == 3563
#include "nexus_platform_avswitch.h"
#endif

#include "nexus_frontend.h"
static NEXUS_FrontendHandle g_frontend;
static NEXUS_FrontendVsbSettings g_vsbSettings;
static NEXUS_FrontendAnalogSettings g_analogSettings;
static bool g_useStreamer;

#include "nexus_parser_band.h"
#include "nexus_input_band.h"
#include "nexus_pid_channel.h"
#include "nexus_timebase.h"
#include "nexus_stc_channel.h"
static NEXUS_StcChannelHandle g_stcChannel;
static NEXUS_PidChannelHandle g_audioPid, g_pcrPid;

#include <assert.h>
#include <stdio.h>

#ifdef IP_STREAMER_SUBST
#include "ip_streamer_subst.h"
#endif

typedef enum TestInput
{
    TestInput_eNone,
    TestInput_eDecoder,
    TestInput_eSpdif,
    TestInput_eI2s,
    TestInput_eRf,
    TestInput_eAnalog,
    TestInput_eMax
} TestInput;

static TestInput g_input=TestInput_eNone;

static const char *g_inputNames[TestInput_eMax] = 
{
    "None",
    "Decoder",
    "SPDIF",
    "I2S",
    "RF",
    "ADC"
};

static void SelectInput(TestInput input);
static void RestartInput(void);

int main(int argc, char **argv)
{
    int tmp;
    char ctmp[256];
    bool running = true;
    NEXUS_Error errCode;
    NEXUS_PlatformConfiguration config;
    NEXUS_ParserBandSettings parserSettings;
    NEXUS_FrontendCapabilities frontendCaps;
    NEXUS_StcChannelSettings stcSettings;
#if NEXUS_DTV_PLATFORM
    NEXUS_AudioEqualizerHandle eq1=NULL, eq2=NULL;
#endif
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
#ifdef IP_STREAMER_SUBST
    IpsHandle ips;
    IpsOpenSettings ipsOpenSettings;
    int useIpStreamer = 0;
    IpsStartSettings ipsStartSettings;
#endif

    argc=argc;
    argv=argv;

    /* Boot Nexus */
    errCode = NEXUS_Platform_Init(NULL);
    assert(NEXUS_SUCCESS == errCode);

    /* Get outputs */
    NEXUS_Platform_GetConfiguration(&config);

    #if NEXUS_NUM_AUDIO_DACS > 1
    do
    {
        printf("Which audio DAC will be used? [0-%d]: ", NEXUS_NUM_AUDIO_DACS-1);
        scanf("%d", &tmp);
        fgets(ctmp, 256, stdin);    /* Discard CR */
    } while ( tmp < 0 || tmp >= NEXUS_NUM_AUDIO_DACS );
    g_audioDac = config.outputs.audioDacs[tmp];
    #else
    g_audioDac = config.outputs.audioDacs[0];
    #endif

    #if NEXUS_DTV_PLATFORM
    printf("Assigning equalizer 0 to DAC\n");
    eq1 = NEXUS_AudioEqualizer_Open(0, NULL);
    assert(NULL != eq1);
    NEXUS_AudioOutput_SetEqualizer(NEXUS_AudioDac_GetConnector(g_audioDac), eq1);
    #endif

    #if NEXUS_NUM_SPDIF_OUTPUTS
    printf("Use SPDIF output? [y/n]: ");
    fgets(ctmp, 256, stdin);
    if ( ctmp[0] == 'y' || ctmp[0] == 'Y' )
    {
        g_spdifOutput = config.outputs.spdif[0];
        printf("Use compressed SPDIF? [y/n]: ");
        fgets(ctmp, 256, stdin);
        if ( ctmp[0] == 'y' || ctmp[0] == 'Y' )
        {
            g_compressedSpdif = true;
            printf("Use simul mode? [y/n]: ");
            fgets(ctmp, 256, stdin);
            if ( ctmp[0] == 'y' || ctmp[0] == 'Y' )
            {
                g_simulMode = true;
            }
        }
        else
        {
            #if NEXUS_DTV_PLATFORM
            printf("Assign equalizer 1 to SPDIF? [y/n]: ");
            fgets(ctmp, 256, stdin);
            if ( ctmp[0] == 'y' || ctmp[0] == 'Y' )
            {
                eq2 = NEXUS_AudioEqualizer_Open(1, NULL);
                assert(NULL != eq2);
                NEXUS_AudioOutput_SetEqualizer(NEXUS_SpdifOutput_GetConnector(g_spdifOutput), eq2);
            }
            #endif
        }
    }
    #endif


    /* Open inputs */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
#ifdef IP_STREAMER_SUBST
    IpsAddAudioDecoderOpenSettings(ips, &audioDecoderOpenSettings);
#endif
    g_audioDecoders[0] = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
    assert(NULL != g_audioDecoders[0]);
    g_audioDecoders[1] = NEXUS_AudioDecoder_Open(1, &audioDecoderOpenSettings);
    assert(NULL != g_audioDecoders[1]);

    #if NEXUS_NUM_SPDIF_INPUTS
    g_spdifInput = NEXUS_SpdifInput_Open(0, NULL);
    assert(NULL != g_spdifInput);
    #endif

    #if NEXUS_NUM_I2S_INPUTS
    g_i2sInput = NEXUS_I2sInput_Open(0, NULL);
    assert(NULL != g_i2sInput);
    #endif

    #if NEXUS_NUM_RF_AUDIO_DECODERS
    g_rfDecoder = NEXUS_RfAudioDecoder_Open(0, NULL);
    assert(NULL != g_rfDecoder);
    #endif

    #if BCHP_CHIP == 3563
    {
        NEXUS_AvSwitchParams avSwitchParams;
        NEXUS_AvSwitchHandle avSwitch = NEXUS_AvSwitch_Open(3);
        assert(NULL != avSwitch);
        avSwitchParams.type = NEXUS_AvSwitchType_eSVIDEO;
        avSwitchParams.index = 0;
        NEXUS_AvSwitch_Switch(avSwitch, avSwitchParams);
        NEXUS_AvSwitch_Close(avSwitch);
    }
    #endif
    #if NEXUS_NUM_ANALOG_AUDIO_DECODERS
    {
        NEXUS_AnalogAudioDecoderSettings analogSettings;
        g_analogInput = NEXUS_AnalogAudioInput_Open(0, NULL);
        assert(NULL != g_analogInput);
        NEXUS_AnalogAudioDecoder_GetDefaultSettings(&analogSettings);
        analogSettings.input = NEXUS_AnalogAudioInput_GetConnector(g_analogInput);
        g_analogDecoder = NEXUS_AnalogAudioDecoder_Open(0, &analogSettings);
        assert(NULL != g_analogDecoder);
    }
    #endif
    #if NEXUS_DTV_PLATFORM
    g_avl = NEXUS_AutoVolumeLevel_Open(NULL);
    #endif

    g_stcChannel = NEXUS_StcChannel_Open(0, NULL);
    assert(NULL != g_stcChannel);

    /* Set up default digital channel for mummy stream */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&g_audioProgram);
    /* Get program settings at startup */
    g_audioProgram.stcChannel = NULL; /* g_stcChannel; */
    NEXUS_ParserBand_GetSettings(NEXUS_ParserBand_e0, &parserSettings);
    parserSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
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
    printf("Enter digital program settings\n");
    printf("Input type? [0=streamer, 1=tuner]: ");
    scanf("%d", &tmp);
    if ( 1 == tmp )
    {
        /* Tuner */
        if ( NULL == g_frontend )
        {
            printf("Unable to find VSB capable tuner.  Defaulting to streamer input.\n");
            NEXUS_Platform_GetStreamerInputBand(0, &parserSettings.sourceTypeSettings.inputBand);
            g_useStreamer = true;
        }
        else
        {
            NEXUS_FrontendUserParameters params;
            NEXUS_Frontend_GetUserParameters(g_frontend, &params);
            parserSettings.sourceTypeSettings.inputBand = (NEXUS_InputBand)params.param1;
            printf("Enter VSB Frequency (in Hz): ");
            scanf("%d", &tmp);
            NEXUS_Frontend_GetDefaultVsbSettings(&g_vsbSettings);
            g_vsbSettings.frequency = tmp;
        }
    }
    else
    {
        NEXUS_Platform_GetStreamerInputBand(0, &parserSettings.sourceTypeSettings.inputBand);
        g_useStreamer = true;
    }
    errCode = NEXUS_ParserBand_SetSettings(NEXUS_ParserBand_e0, &parserSettings);
#ifdef IP_STREAMER_SUBST
    if( 0 == tmp || ( 1 == tmp && NULL == g_frontend ) ) {
        useIpStreamer = 1;
        BSTD_UNUSED(g_analogSettings);
        ipsOpenSettings.parserBand=NEXUS_ParserBand_e0;
        ipsOpenSettings.playpumpIndex = 0;
        ipsOpenSettings.remuxIndex = 0;
        ips = IpsOpen(&ipsOpenSettings);
    }
#endif

    assert(NEXUS_SUCCESS == errCode);
    printf("Audio PID (in hex): ");
    scanf("%x", (unsigned *)&tmp);
    g_audioPid = NEXUS_PidChannel_Open(NEXUS_ParserBand_e0, tmp, NULL);
    assert(NULL != g_audioPid);
    do
    {
        printf("Audio Codec (1=MPEG, 3=AAC, 4=AAC+, 5=AC3, 6=AC3+): ");
        scanf("%d", &tmp);
        g_audioCodec = tmp;
    } while ( tmp != 1 && tmp < 3 && tmp > 6 );

    printf("PCR PID (in hex): ");
    scanf("%x", (unsigned *)&tmp);
    g_pcrPid = NEXUS_PidChannel_Open(NEXUS_ParserBand_e0, tmp, NULL);
    assert(NULL != g_pcrPid);

    NEXUS_StcChannel_GetSettings(g_stcChannel, &stcSettings);
    stcSettings.modeSettings.pcr.pidChannel = g_pcrPid;
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr;
    stcSettings.autoConfigTimebase = true;
#ifdef IP_STREAMER_SUBST
    if( useIpStreamer ) {
        IpsAddStcSettings(ips, &stcSettings);
    }
#endif
    errCode = NEXUS_StcChannel_SetSettings(g_stcChannel, &stcSettings);
    assert(NEXUS_SUCCESS == errCode);

    #if NEXUS_NUM_RF_AUDIO_DECODERS
    if ( g_frontend && frontendCaps.analog && frontendCaps.ifd )
    {
        printf("Analog tuning frequency (in Hz): ");
        scanf("%d", &tmp);
        NEXUS_Frontend_GetDefaultAnalogSettings(&g_analogSettings);
        g_analogSettings.frequency = tmp;
    }
    #endif

#ifdef IP_STREAMER_SUBST
    if( useIpStreamer ) {
        ipsStartSettings.vDecoder = NULL;
        ipsStartSettings.aDecoder = NULL;
        ipsStartSettings.stcChannel = NULL;
        IpsStart(ips, &ipsStartSettings);
    }
#endif
    while ( running )
    {
        printf("\nMain Menu\n");
        printf("\t1) Select Input [currently %s]\n", g_inputNames[g_input]);
        printf("\t2) Restart Input\n");
        printf("\t3) Set DAC volume\n");
        #if NEXUS_DTV_PLATFORM
        if ( g_avl )
        {
            printf("\t4) Toggle AVL [currently %s]\n", g_avlEnabled?"Enabled":"Disabled");
        }
        printf("\t5) Set Equalizer Settings\n");
        #endif
        printf("\t0) Quit\n");
        printf("\n\t Enter Selection: ");
        scanf("%d", &tmp);
        switch ( tmp )
        {
        case 0:
            printf("QUIT\n");
            running = false;
            break;
        case 1:
            printf("SELECT INPUT\n");
            printf("\t1) Decoder\n");
            #if NEXUS_NUM_SPDIF_INPUTS
            printf("\t2) SPDIF Input\n");
            #endif
            #if NEXUS_NUM_I2S_INPUTS
            printf("\t3) I2S Input\n");
            #endif
            #if NEXUS_NUM_RF_AUDIO_DECODERS
            printf("\t4) RF Decoder\n");
            #endif
            #if NEXUS_NUM_ANALOG_AUDIO_DECODERS
            printf("\t5) Analog Decoder\n");
            #endif
            printf("\t  Enter Selection: ");
            scanf("%d", &tmp);
            switch ( tmp )
            {
            case 1:
                SelectInput(TestInput_eDecoder);
                break;
            #if NEXUS_NUM_SPDIF_INPUTS
            case 2:
                SelectInput(TestInput_eSpdif);
                break;
            #endif
            #if NEXUS_NUM_I2S_INPUTS
            case 3:
                SelectInput(TestInput_eI2s);
                break;
            #endif
            #if NEXUS_NUM_RF_AUDIO_DECODERS
            case 4:
                SelectInput(TestInput_eRf);
                break;
            #endif
            #if NEXUS_NUM_ANALOG_AUDIO_DECODERS
            case 5:
                SelectInput(TestInput_eAnalog);
                break;
            #endif
            default:
                printf("Unrecognized input %d\n", tmp);
                break;
            }
        case 2:
            RestartInput();
            break;
        case 3:
            {
                NEXUS_AudioOutputSettings outputSettings;
                NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(g_audioDac), &outputSettings);
                printf("Select volume mode (0=dB, 1=linear): ");
                scanf("%d", &tmp);
                outputSettings.volumeType = (tmp==1)?NEXUS_AudioVolumeType_eLinear:NEXUS_AudioVolumeType_eDecibel;
                printf("Enter volume level: ");
                scanf("%d", &tmp);
                outputSettings.leftVolume = outputSettings.rightVolume = tmp;
                errCode = NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(g_audioDac), &outputSettings);
                assert(NEXUS_SUCCESS == errCode);
            }
            break;
        #if NEXUS_DTV_PLATFORM
        case 4:
            if ( g_avl )
            {
                g_avlEnabled = !g_avlEnabled;
                SelectInput(g_input);
            }
            break;
        case 5:
            {
                NEXUS_AudioEqualizerHandle eq;
                printf("Select Equalizer [0-1]: ");
                scanf("%d", &tmp);
                eq=(tmp==0)?eq1:eq2;
                if ( eq )
                {
                    float tmpFloat;
                    NEXUS_AudioEqualizerSettings eqSettings;
                    NEXUS_AudioEqualizer_GetSettings(eq, &eqSettings);
                    printf("Select mode [1=Tone, 2=5Band 3=7band(parametric)]: ");
                    scanf("%d", &tmp);

                    switch ( tmp )
                    {
                    case 1:
                        eqSettings.mode = NEXUS_AudioEqualizerMode_eTone;
                        printf("Select Bass Level [-10.0dB - +10.0dB]: ");
                        scanf("%f", &tmpFloat);
                        eqSettings.modeSettings.tone.bass = (unsigned int)(tmpFloat*100.0);
                        printf("Select Treble Level [-10.0dB - +10.0dB]: ");
                        scanf("%f", &tmpFloat);
                        eqSettings.modeSettings.tone.treble = (unsigned int)(tmpFloat*100.0);
                        break;
                    case 2:
                        eqSettings.mode = NEXUS_AudioEqualizerMode_eFiveBand;
                        printf("Select 100Hz Level [-10.0dB - +10.0dB]: ");
                        scanf("%f", &tmpFloat);
                        eqSettings.modeSettings.fiveBand.gain100Hz = (unsigned int)(tmpFloat*100.0);
                        printf("Select 300Hz Level [-10.0dB - +10.0dB]: ");
                        scanf("%f", &tmpFloat);
                        eqSettings.modeSettings.fiveBand.gain300Hz = (unsigned int)(tmpFloat*100.0);
                        printf("Select 1kHz Level [-10.0dB - +10.0dB]: ");
                        scanf("%f", &tmpFloat);
                        eqSettings.modeSettings.fiveBand.gain1000Hz = (unsigned int)(tmpFloat*100.0);
                        printf("Select 3kHz Level [-10.0dB - +10.0dB]: ");
                        scanf("%f", &tmpFloat);
                        eqSettings.modeSettings.fiveBand.gain3000Hz = (unsigned int)(tmpFloat*100.0);
                        printf("Select 10kHz Level [-10.0dB - +10.0dB]: ");
                        scanf("%f", &tmpFloat);
                        eqSettings.modeSettings.fiveBand.gain10000Hz = (unsigned int)(tmpFloat*100.0);
                        break;
                    case 3:
                        {
                            int tmp2, band_idx;
                            
                            eqSettings.mode = NEXUS_AudioEqualizerMode_eParametric;
                            printf("Enter Parametric setting for 7 bands of the equalizer, each band requires:\n");
                            printf("  Peak frequency in Hz\n");
                            printf("  Peak frequency Q gain in 1/100 (eg, for 0.33, enter 33)\n");
                            printf("  Gain in 1/100dB, range -12dB to +12dB, (eg, for 9.1dB, enter 9.1)\n");
                            for( band_idx = 0; band_idx < 7; band_idx++)
                            {
                                printf("Enter band[%d] Peak, Q, Gain values (separated by space): ", band_idx);
                                scanf("%d %d %f", &tmp, &tmp2, &tmpFloat);
                                eqSettings.modeSettings.parametric.bandSettings[band_idx].peak = tmp;
                                eqSettings.modeSettings.parametric.bandSettings[band_idx].q    = tmp2;
								/* 100.0 is used to keep the 0.xx value */
                                eqSettings.modeSettings.parametric.bandSettings[band_idx].gain = (unsigned int) (tmpFloat*100.0);
                            }
                        }
                    	break;
                    default:
                        break;
                    }

                    NEXUS_AudioEqualizer_SetSettings(eq, &eqSettings);
                }
            }
            break;
        #endif
        default:
            printf("Unrecognized option %d\n", tmp);
            break;
        }
    }

#ifdef IP_STREAMER_SUBST
    if( useIpStreamer ){
        IpsClose(ips);
    }
#endif

    return 0;
}

static void SelectInput(TestInput input)
{
    NEXUS_Error errCode;
    /* Untune current selection */
    switch ( g_input )
    {
    case TestInput_eNone:
        break;
    case TestInput_eDecoder:
    #if NEXUS_NUM_SPDIF_INPUTS
    case TestInput_eSpdif:
    #endif
        NEXUS_AudioDecoder_Stop(g_audioDecoders[0]);
        if ( g_spdifOutput && g_compressedSpdif && !g_simulMode && g_input == TestInput_eDecoder )
        {
            NEXUS_AudioDecoder_Stop(g_audioDecoders[1]);
        }
        #if NEXUS_DTV_PLATFORM
        if ( g_avl )
            NEXUS_AutoVolumeLevel_RemoveAllInputs(g_avl);
        #endif
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_audioDac));
        if ( g_spdifOutput )
        {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(g_spdifOutput));
        }
        break;
    #if NEXUS_NUM_I2S_INPUTS
    case TestInput_eI2s:
        NEXUS_I2sInput_Stop(g_i2sInput);
        #if NEXUS_DTV_PLATFORM
        if ( g_avl )
            NEXUS_AutoVolumeLevel_RemoveAllInputs(g_avl);
        #endif
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_audioDac));
        if ( g_spdifOutput )
        {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(g_spdifOutput));
        }
        break;
    #endif
    #if NEXUS_NUM_RF_AUDIO_DECODERS
    case TestInput_eRf:
        NEXUS_RfAudioDecoder_Stop(g_rfDecoder);
        #if NEXUS_DTV_PLATFORM
        if ( g_avl )
            NEXUS_AutoVolumeLevel_RemoveAllInputs(g_avl);
        #endif
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_audioDac));
        if ( g_spdifOutput )
        {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(g_spdifOutput));
        }
        break;
    #endif
    #if NEXUS_NUM_ANALOG_AUDIO_DECODERS
    case TestInput_eAnalog:
        NEXUS_AnalogAudioDecoder_Stop(g_analogDecoder);
        #if NEXUS_DTV_PLATFORM
        if ( g_avl )
            NEXUS_AutoVolumeLevel_RemoveAllInputs(g_avl);
        #endif
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(g_audioDac));
        if ( g_spdifOutput )
        {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(g_spdifOutput));
        }
        break;
    #endif
    default:
        printf("Unrecognized input %d\n", g_input);
        break;
    }

    g_input = TestInput_eNone;

    /* Setup new input */
    switch ( input )
    {
    case TestInput_eNone:
        /* Nothing to do */
        return;
    case TestInput_eDecoder:
        if ( g_frontend && !g_useStreamer )
        {
            errCode = NEXUS_Frontend_TuneVsb(g_frontend, &g_vsbSettings);
            assert(NEXUS_SUCCESS == errCode);
        }
        #if NEXUS_DTV_PLATFORM
        if ( g_avl && g_avlEnabled )
        {
            errCode = NEXUS_AutoVolumeLevel_AddInput(g_avl, NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0], NEXUS_AudioDecoderConnectorType_eStereo));
            assert(NEXUS_SUCCESS == errCode);
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_AutoVolumeLevel_GetConnector(g_avl));
            assert(NEXUS_SUCCESS == errCode);
        }
        else
        #endif
        {
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac),
                                                 NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0], NEXUS_AudioDecoderConnectorType_eStereo));
            assert(NEXUS_SUCCESS == errCode);
        }
        if ( g_spdifOutput )
        {
            NEXUS_AudioInputHandle decoderConnector;
            unsigned whichDecoder = (g_compressedSpdif&&!g_simulMode)?1:0;
            decoderConnector = NEXUS_AudioDecoder_GetConnector(g_audioDecoders[whichDecoder],
                                                               (g_compressedSpdif)?NEXUS_AudioDecoderConnectorType_eCompressed:
                                                                                   NEXUS_AudioDecoderConnectorType_eStereo);
            #if NEXUS_DTV_PLATFORM
            if ( g_avl && g_avlEnabled && !g_compressedSpdif )
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_spdifOutput), 
                                                     NEXUS_AutoVolumeLevel_GetConnector(g_avl));
                assert(NEXUS_SUCCESS == errCode);
            }
            else
            #endif
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_spdifOutput), decoderConnector);
                assert(NEXUS_SUCCESS == errCode);
            }
        }

        NEXUS_AudioDecoder_GetDefaultStartSettings(&g_audioProgram);
        g_audioProgram.stcChannel = NULL; /* g_stcChannel; */
        g_audioProgram.pidChannel = g_audioPid;
        errCode = NEXUS_AudioDecoder_Start(g_audioDecoders[0], &g_audioProgram);
        assert(NEXUS_SUCCESS == errCode);
        if ( g_spdifOutput && g_compressedSpdif && !g_simulMode )
        {
            errCode = NEXUS_AudioDecoder_Start(g_audioDecoders[1], &g_audioProgram);
            assert(NEXUS_SUCCESS == errCode);
        }
        break;
    #if NEXUS_NUM_SPDIF_INPUTS
    case TestInput_eSpdif:
        if ( g_avl && g_avlEnabled )
        {
            errCode = NEXUS_AutoVolumeLevel_AddInput(g_avl, NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0], NEXUS_AudioDecoderConnectorType_eStereo));
            assert(NEXUS_SUCCESS == errCode);
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_AutoVolumeLevel_GetConnector(g_avl));
            assert(NEXUS_SUCCESS == errCode);
        }
        else
        {
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac),
                                                 NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0], NEXUS_AudioDecoderConnectorType_eStereo));
            assert(NEXUS_SUCCESS == errCode);
        }
        if ( g_spdifOutput )
        {
            if ( g_avl && g_avlEnabled && !g_compressedSpdif )
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_AutoVolumeLevel_GetConnector(g_avl));
                assert(NEXUS_SUCCESS == errCode);
            }
            else
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_spdifOutput),
                                                     NEXUS_AudioDecoder_GetConnector(g_audioDecoders[0],
                                                                                     (g_compressedSpdif)?NEXUS_AudioDecoderConnectorType_eCompressed:
                                                                                                         NEXUS_AudioDecoderConnectorType_eStereo));
                assert(NEXUS_SUCCESS == errCode);
            }
        }

        NEXUS_AudioDecoder_GetDefaultStartSettings(&g_audioProgram);
        g_audioProgram.input = NEXUS_SpdifInput_GetConnector(g_spdifInput);
        errCode = NEXUS_AudioDecoder_Start(g_audioDecoders[0], &g_audioProgram);
        assert(NEXUS_SUCCESS == errCode);
        break;
    #endif
    #if NEXUS_NUM_I2S_INPUTS
    case TestInput_eI2s:
        #if NEXUS_DTV_PLATFORM
        if ( g_avl && g_avlEnabled )
        {
            errCode = NEXUS_AutoVolumeLevel_AddInput(g_avl, NEXUS_I2sInput_GetConnector(g_i2sInput));
            assert(NEXUS_SUCCESS == errCode);
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_AutoVolumeLevel_GetConnector(g_avl));
            assert(NEXUS_SUCCESS == errCode);
            if ( g_spdifOutput )
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_AutoVolumeLevel_GetConnector(g_avl));
                assert(NEXUS_SUCCESS == errCode);
            }
        }
        else
        #endif
        {
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_I2sInput_GetConnector(g_i2sInput));
            assert(NEXUS_SUCCESS == errCode);
            if ( g_spdifOutput )
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_spdifOutput), NEXUS_I2sInput_GetConnector(g_i2sInput));
                assert(NEXUS_SUCCESS == errCode);
            }
        }
        errCode = NEXUS_I2sInput_Start(g_i2sInput);
        break;
    #endif
    #if NEXUS_NUM_RF_AUDIO_DECODERS
    case TestInput_eRf:
        if ( g_frontend )
        {
            NEXUS_RfAudioDecoderSettings rfSettings;

            errCode = NEXUS_Frontend_TuneAnalog(g_frontend, &g_analogSettings);
            assert(NEXUS_SUCCESS == errCode);

            NEXUS_RfAudioDecoder_GetSettings(g_rfDecoder, &rfSettings);
            rfSettings.input = NEXUS_Frontend_GetAnalogAudioConnector(g_frontend);
            NEXUS_RfAudioDecoder_SetSettings(g_rfDecoder, &rfSettings);

            if ( g_avl && g_avlEnabled )
            {
                errCode = NEXUS_AutoVolumeLevel_AddInput(g_avl, NEXUS_RfAudioDecoder_GetConnector(g_rfDecoder));
                assert(NEXUS_SUCCESS == errCode);
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_AutoVolumeLevel_GetConnector(g_avl));
                assert(NEXUS_SUCCESS == errCode);
                if ( g_spdifOutput )
                {
                    errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_AutoVolumeLevel_GetConnector(g_avl));
                    assert(NEXUS_SUCCESS == errCode);
                }
            }
            else
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_RfAudioDecoder_GetConnector(g_rfDecoder));
                assert(NEXUS_SUCCESS == errCode);
                if ( g_spdifOutput )
                {
                    errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_spdifOutput), NEXUS_RfAudioDecoder_GetConnector(g_rfDecoder));
                    assert(NEXUS_SUCCESS == errCode);
                }
            }
            errCode = NEXUS_RfAudioDecoder_Start(g_rfDecoder);
            assert(NEXUS_SUCCESS == errCode);
        }
        else
        {
            return;
        }
        break;
    #endif
    #if NEXUS_NUM_ANALOG_AUDIO_DECODERS
    case TestInput_eAnalog:
        if ( g_avl && g_avlEnabled )
        {
            errCode = NEXUS_AutoVolumeLevel_AddInput(g_avl, NEXUS_AnalogAudioDecoder_GetConnector(g_analogDecoder));
            assert(NEXUS_SUCCESS == errCode);
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_AutoVolumeLevel_GetConnector(g_avl));
            assert(NEXUS_SUCCESS == errCode);
            if ( g_spdifOutput )
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_AutoVolumeLevel_GetConnector(g_avl));
                assert(NEXUS_SUCCESS == errCode);
            }
        }
        else
        {
            errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(g_audioDac), NEXUS_AnalogAudioDecoder_GetConnector(g_analogDecoder));
            assert(NEXUS_SUCCESS == errCode);
            if ( g_spdifOutput )
            {
                errCode = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(g_spdifOutput), NEXUS_AnalogAudioDecoder_GetConnector(g_analogDecoder));
                assert(NEXUS_SUCCESS == errCode);
            }
        }
        errCode = NEXUS_AnalogAudioDecoder_Start(g_analogDecoder);
        assert(NEXUS_SUCCESS == errCode);
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
    switch ( g_input )
    {
    case TestInput_eNone:
        /* Nothing to do */
        return;
    case TestInput_eDecoder:
        NEXUS_AudioDecoder_Stop(g_audioDecoders[0]);
        if ( g_spdifOutput && g_compressedSpdif && !g_simulMode && g_input == TestInput_eDecoder )
        {
            NEXUS_AudioDecoder_Stop(g_audioDecoders[1]);
        }
        NEXUS_AudioDecoder_Start(g_audioDecoders[0], &g_audioProgram);
        if ( g_spdifOutput && g_compressedSpdif && !g_simulMode && g_input == TestInput_eDecoder )
        {
            NEXUS_AudioDecoder_Start(g_audioDecoders[1], &g_audioProgram);
        }
        break;
    #if NEXUS_NUM_SPDIF_INPUTS
    case TestInput_eSpdif:
        NEXUS_AudioDecoder_Stop(g_audioDecoders[0]);
        NEXUS_AudioDecoder_Start(g_audioDecoders[0], &g_audioProgram);
        break;
    #endif
    #if NEXUS_NUM_RF_AUDIO_DECODERS
    case TestInput_eRf:
        if ( g_frontend )
        {
            NEXUS_RfAudioDecoder_Stop(g_rfDecoder);
            NEXUS_RfAudioDecoder_Start(g_rfDecoder);
        }
        break;
    #endif
    #if NEXUS_NUM_I2S_INPUTS
    case TestInput_eI2s:
        NEXUS_I2sInput_Stop(g_i2sInput);
        NEXUS_I2sInput_Start(g_i2sInput); 
        break;
    #endif
    #if NEXUS_NUM_ANALOG_AUDIO_DECODERS
    case TestInput_eAnalog:
        NEXUS_AnalogAudioDecoder_Stop(g_analogDecoder);
        NEXUS_AnalogAudioDecoder_Start(g_analogDecoder);
        break;
    #endif
    default:
        assert(false);
        return;
    }
}
