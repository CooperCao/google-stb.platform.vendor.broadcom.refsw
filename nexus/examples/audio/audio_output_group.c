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
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nexus_platform_features.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "nexus_pid_channel.h"
#include "nexus_stc_channel.h"
#include "nexus_parser_band.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_output_group.h"
#include "nexus_audio_input.h"
#include "nexus_core_utils.h"
#include <string.h>
#include <stdlib.h>

int main(void)
{
#if NEXUS_NUM_I2S_OUTPUTS > 1 || NEXUS_NUM_HDMI_OUTPUTS > 0
    NEXUS_PlatformConfiguration config;

    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_AudioDecoderHandle decoder;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_AudioDecoderStartSettings decoderSettings;
    NEXUS_AudioDecoderOpenSettings audioOpenSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_AudioOutputGroupCreateSettings outputGroupCreateSettings;
    NEXUS_AudioOutputGroupHandle outputGroup;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&config);

    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioOpenSettings);
    audioOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;
    decoder = NEXUS_AudioDecoder_Open(0, &audioOpenSettings);
    if ( NULL == decoder )
    {
        fprintf(stderr, "Unable to open decoder 0\n");
        return 0;
    }

    NEXUS_AudioOutputGroup_GetDefaultCreateSettings(&outputGroupCreateSettings);
#if NEXUS_NUM_SPDIF_OUTPUTS > 0
    outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eLeftRight] = NEXUS_SpdifOutput_GetConnector(config.outputs.spdif[0]);
#endif
#if NEXUS_NUM_I2S_OUTPUTS > 0
    outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eLeftRightSurround] = NEXUS_I2sOutput_GetConnector(config.outputs.i2s[0]);
#endif
#if NEXUS_NUM_I2S_OUTPUTS > 1
    outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eCenterLfe] = NEXUS_I2sOutput_GetConnector(config.outputs.i2s[1]);
#elif NEXUS_NUM_HDMI_OUTPUTS > 0
    outputGroupCreateSettings.outputs[NEXUS_AudioChannelPair_eCenterLfe] = NEXUS_HdmiOutput_GetAudioConnector(config.outputs.hdmi[0]);
#endif
    outputGroup = NEXUS_AudioOutputGroup_Create(&outputGroupCreateSettings);
    if ( NULL == outputGroup )
    {
        fprintf(stderr, "Unable to open output group\n");
        return 0;
    }

#if NEXUS_NUM_AUDIO_DACS > 0
    /* Connect DAC to decoder */
    NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(config.outputs.audioDacs[0]),
                               NEXUS_AudioDecoder_GetConnector(decoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    /* Connect group to decoder */
    NEXUS_AudioOutput_AddInput(NEXUS_AudioOutputGroup_GetConnector(outputGroup),
                               NEXUS_AudioDecoder_GetConnector(decoder, NEXUS_AudioDecoderConnectorType_eMultichannel));

    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    /* Map a parser band to the streamer input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the StcChannel to do TSM with the PCR */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = NEXUS_PidChannel_Open(parserBand, 0x11, NULL);
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /* Start the audio decoder */
    NEXUS_AudioDecoder_GetDefaultStartSettings(&decoderSettings);
    decoderSettings.codec = NEXUS_AudioCodec_eAc3;
    decoderSettings.pidChannel = NEXUS_PidChannel_Open(parserBand, 0x14, NULL);
    decoderSettings.stcChannel = stcChannel;
    printf("Starting Decoder\n");
    NEXUS_AudioDecoder_Start(decoder, &decoderSettings);
    printf("Done Starting Decoder\n");
    
    printf("Press Enter to Stop\n");
    getchar();

    printf("Stopping decoder\n");
    NEXUS_AudioDecoder_Stop(decoder);
    NEXUS_AudioOutputGroup_Destroy(outputGroup);
    NEXUS_AudioDecoder_Close(decoder);

    return 0;
#else
    fprintf(stderr, "Insufficient outputs to create output group");
    return 0;
#endif
}
#else /* NEXUS_HAS_AUDIO */
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
