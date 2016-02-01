/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
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
 *****************************************************************************/

/* Nexus example app: single live a/v descrambling and decode from an input band */
/* Test stream is spiderman_aes.ts */
/* Hardcoded for QAM-B 256QAM, 789MHz */
#if NEXUS_HAS_SECURITY && NEXUS_HAS_FRONTEND && defined(HSM_TEST_TO_BE_INTEGRATED)

#include "nexus_frontend.h"
#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "bstd.h"
#include "bkni.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif
#include "nexus_security.h"


/* From tune_qam lock_callback */

static void lock_callback(void *context, int param)
{
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle)context;
    NEXUS_FrontendQamStatus qamStatus;

    BSTD_UNUSED(param);

    fprintf(stderr, "Lock callback, frontend 0x%08x\n", (unsigned)frontend);

    NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
    fprintf(stderr, "QAM Lock callback, frontend 0x%08x - lock status %d, %d\n", (unsigned)frontend,
            qamStatus.fecLock, qamStatus.receiverLock);
}


int main(void)

{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend=NULL;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_FrontendAcquireSettings settings;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle pcmDecoder, compressedDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;


    /* The encryption keys are the same for Audio/Video EVEN/ODD keys in this example.  Those
       Keys can be different in a real application */
    unsigned char   VidEvenControlWord[] = {
                                            0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                                            0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef  };
    unsigned char VidOddControlWord[] = {
                                            0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                                            0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef  };
     unsigned char AudEvenControlWord[] = {
                                            0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                                            0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef  };
    unsigned char AudOddControlWord[] = {
                                            0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                                            0x12, 0x34, 0xfe, 0xed, 0xba, 0xbe, 0xbe, 0xef  };

    unsigned char iv[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned int videoPID, audioPID;
    unsigned freq = 789;
    NEXUS_KeySlotHandle videoKeyHandle = NULL;
    NEXUS_KeySlotHandle audioKeyHandle = NULL;
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    NEXUS_SecurityClearKey key;
    NEXUS_PidChannelStatus pidStatus;
    NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_FrontendQamMode qamMode;


    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = true;

    /* Added frontend fucntions here */
    qamMode = NEXUS_FrontendQamMode_e256;

    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
    settings.capabilities.qam = true;
    frontend = NEXUS_Frontend_Acquire(&settings);
    if (!frontend) {
        fprintf(stderr, "Unable to find QAM-capable frontend\n");
        return -1;
    }

    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = freq * 1000000;
    qamSettings.mode = qamMode;
    qamSettings.symbolRate = 5360537;
    qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
    qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    qamSettings.lockCallback.callback = lock_callback;
    qamSettings.lockCallback.context = frontend;

    NEXUS_Frontend_GetUserParameters(frontend, &userParams);

    /* End of frontend fucntions */

    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
    /* NEXUS_Platform_GetStreamerInputBand(0, &inputBand); */

    /* Map a parser band to the streamer input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the audio and video pid channels */
    videoPidChannel = NEXUS_PidChannel_Open(parserBand, 0x11, NULL);
    audioPidChannel = NEXUS_PidChannel_Open(parserBand, 0x14, NULL);

    /* Open the StcChannel to do lipsync with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up
    the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = NEXUS_VideoCodec_eMpeg2;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_eAc3;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;

    /* Bring up audio decoders and outputs */
    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
    compressedDecoder = NEXUS_AudioDecoder_Open(1, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

#if NEXUS_NUM_SPDIF_OUTPUTS
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eCompressed));

        NEXUS_AudioOutput_AddInput(
            NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
            NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
    else
    {
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

        NEXUS_AudioOutput_AddInput(
            NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
            NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));

    }
#endif


    /***************************************************************************************
        Config CA descrambler
    ***************************************************************************************/

    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;
    /* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if ( !videoKeyHandle)
    {
        printf("\nAllocate CA video keyslot failed \n");
        return 1;
    }
    audioKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if ( !audioKeyHandle)
    {
        printf("\nAllocate CA audio keyslot failed \n");
        return 1;
    }

    /* Config AV algorithms */
    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
    AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eAes;
    AlgConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eCbc;
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eBlock;

    /* ++++++++ */
    AlgConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
    AlgConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
    AlgConfig.caVendorID          = 0x1234;
    AlgConfig.askmModuleID        = NEXUS_SecurityAskmModuleID_eModuleID_4;
    AlgConfig.otpId               = NEXUS_SecurityOtpId_eOtpVal;
    printf(" ==========HSM_IS_40NM============\n");
    AlgConfig.key2Select          = NEXUS_SecurityKey2Select_eReserved1;

    /* ++++++++ */


    if ( NEXUS_Security_ConfigAlgorithm (videoKeyHandle, &AlgConfig) != 0 )
    {
        printf("\nConfig video CA Algorithm failed \n");
        return 1;
    }
     if ( NEXUS_Security_ConfigAlgorithm (audioKeyHandle, &AlgConfig) != 0 )
    {
        printf("\nConfig video CA Algorithm failed \n");
        return 1;
    }



    /* Load AV keys */
    key.keySize = sizeof(VidEvenControlWord);
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    /*--------*/
    key.keyIVType     = NEXUS_SecurityKeyIVType_eNoIV;
    /*--------*/
    memcpy(key.keyData,VidEvenControlWord,sizeof(VidEvenControlWord));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad video EVEN key failed \n");
        return 1;
    }
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy(key.keyData,VidOddControlWord,sizeof(VidOddControlWord));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad video ODD key failed \n");
        return 1;
    }

    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    memcpy(key.keyData,AudEvenControlWord,sizeof(AudEvenControlWord));
    if ( NEXUS_Security_LoadClearKey (audioKeyHandle, &key) != 0 )
    {
        printf("\nLoad audio EVEN key failed \n");
        return 1;
    }
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy(key.keyData,AudOddControlWord,sizeof(AudOddControlWord));
    if ( NEXUS_Security_LoadClearKey (audioKeyHandle, &key) != 0 )
    {
        printf("\nLoad audio ODD key failed \n");
        return 1;
    }

    /* Loading of IV values */
    key.keySize = sizeof(iv);

    /*--------*/
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    key.keyIVType     = NEXUS_SecurityKeyIVType_eIV;
    /*--------*/

    memcpy(key.keyData,iv,sizeof(iv));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad video IV failed \n");
        return 1;
    }
    memcpy(key.keyData,iv,sizeof(iv));
    if ( NEXUS_Security_LoadClearKey (audioKeyHandle, &key) != 0 )
    {
        printf("\nLoad audio IV failed \n");
        return 1;
    }

    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    key.keyIVType     = NEXUS_SecurityKeyIVType_eIV;

    memcpy(key.keyData,iv,sizeof(iv));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad video IV failed \n");
        return 1;
    }
    memcpy(key.keyData,iv,sizeof(iv));
    if ( NEXUS_Security_LoadClearKey (audioKeyHandle, &key) != 0 )
    {
        printf("\nLoad audio IV failed \n");
        return 1;
    }

    /* Add video PID channel to keyslot */
    NEXUS_PidChannel_GetStatus (videoProgram.pidChannel, &pidStatus);
    videoPID = pidStatus.pidChannelIndex;
    if ( NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle, videoPID)!= 0 )
    {
        printf("\nConfigPIDPointerTable failed \n");
        return 1;
    }

    /* Add audio PID channel to keyslot */
    NEXUS_PidChannel_GetStatus (audioProgram.pidChannel, &pidStatus);
    audioPID = pidStatus.pidChannelIndex;
    NEXUS_Security_AddPidChannelToKeySlot(audioKeyHandle, audioPID);

    printf ("\nSecurity Config OK\n");

    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);


        #if NEXUS_NUM_COMPONENT_OUTPUTS
        NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
        #endif
        #if NEXUS_NUM_SCART_INPUTS
        NEXUS_Display_AddOutput(display, NEXUS_ScartInput_GetVideoOutputConnector(platformConfig.inputs.scart[0]));
        #if NEXUS_NUM_SCART_INPUTS>1
        NEXUS_Display_AddOutput(display, NEXUS_ScartInput_GetVideoOutputConnector(platformConfig.inputs.scart[1]));
        #endif
        #endif
        #if NEXUS_NUM_COMPOSITE_OUTPUTS
        if ( platformConfig.outputs.composite[0] ) {
            NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
        }
        #endif
        if (platformConfig.outputs.hdmi[0]) {
            NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
        }

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));

    /* tune */
    NEXUS_Frontend_TuneQam(frontend, &qamSettings);
    printf("********************************** LOCK STATUS SHOULD PRINT *******************************\n");

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);
    NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
    if ( audioProgram.codec == NEXUS_AudioCodec_eAc3 )
    {
        /* Only pass through AC3 */
        NEXUS_AudioDecoder_Start(compressedDecoder, &audioProgram);
    }

#if 1
    /* Print status while decoding */
    for (;;) {
        NEXUS_VideoDecoderStatus status;
        NEXUS_AudioDecoderStatus audioStatus;
        uint32_t stc;

        NEXUS_VideoDecoder_GetStatus(vdecode, &status);
        NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
        printf("decode %.4dx%.4d, pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference, status.fifoSize?(status.fifoDepth*100)/status.fifoSize:0);
        NEXUS_AudioDecoder_GetStatus(pcmDecoder, &audioStatus);
        printf("audio0            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
            audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
        NEXUS_AudioDecoder_GetStatus(compressedDecoder, &audioStatus);
        if ( audioStatus.started )
        {
            printf("audio1            pts %#x, stc %#x (diff %d) fifo=%d%%\n",
                audioStatus.pts, stc, audioStatus.ptsStcDifference, audioStatus.fifoSize?(audioStatus.fifoDepth*100)/audioStatus.fifoSize:0);
        }
        BKNI_Sleep(1000);
    }
#else
    printf("Press ENTER to stop decode\n");
    getchar();

    /* example shutdown */
    NEXUS_Security_RemovePidChannelFromKeySlot(videoKeyHandle, videoPID);
    NEXUS_Security_RemovePidChannelFromKeySlot(audioKeyHandle, audioPID);
    NEXUS_Security_FreeKeySlot(videoKeyHandle);
    NEXUS_Security_FreeKeySlot(audioKeyHandle);

    NEXUS_AudioDecoder_Close(pcmDecoder);
    NEXUS_AudioDecoder_Close(compressedDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_VideoDecoder_Close(vdecode);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_PidChannel_Close(videoPidChannel);
    NEXUS_PidChannel_Close(audioPidChannel);
    NEXUS_Frontend_Release(frontend);
    NEXUS_Platform_Uninit();
#endif

    return 0;
}

#else /* NEXUS_HAS_SECURITY && NEXUS_HAS_FRONTEND */
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
