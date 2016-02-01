/***************************************************************************
 *     (c)2012-2014 Broadcom Corporation
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
#include "nexus_platform.h"
#include "nexus_audio_dac.h"
#include "nexus_i2s_input.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"
#include "nexus_audio_decoder.h"
#include <stdio.h>

#define INPUT_SAMPLE_RATE (48000)

int main(int argc, char **argv)
{
    NEXUS_Error errCode;
    NEXUS_I2sInputHandle i2sInput;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_AudioDecoderHandle decoder;
    NEXUS_AudioDecoderStartSettings startSettings;

	argc=argc;
	argv=argv;

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Set timebase 0 to I2S input */
    {
        NEXUS_TimebaseSettings timebaseSettings;
        NEXUS_Timebase_GetSettings(NEXUS_Timebase_e0, &timebaseSettings);

        timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eI2sIn;
        timebaseSettings.sourceSettings.i2s.index = 0;
        timebaseSettings.sourceSettings.i2s.sampleRate = INPUT_SAMPLE_RATE;

        NEXUS_Timebase_SetSettings(NEXUS_Timebase_e0, &timebaseSettings);
    }

    i2sInput = NEXUS_I2sInput_Open(0, NULL);
    if ( NULL == i2sInput )
    {
        fprintf(stderr, "Unable to open i2s input 0\n");
        return -1;
    }

    decoder = NEXUS_AudioDecoder_Open(0, NULL);
    if ( NULL == decoder )
    {
        fprintf(stderr, "Unable to open decoder 0\n");
        return -1;
    }

#if NEXUS_NUM_AUDIO_DACS
    errCode = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                                         NEXUS_AudioDecoder_GetConnector(decoder, NEXUS_AudioDecoderConnectorType_eStereo));

    if ( errCode )
    {
        fprintf(stderr, "Unable to connect DAC to decoder\n");
        return -1;
    }
#endif

    NEXUS_AudioDecoder_GetDefaultStartSettings(&startSettings);
    startSettings.input = NEXUS_I2sInput_GetConnector(i2sInput);
    startSettings.codec = NEXUS_AudioCodec_ePcm;
    errCode = NEXUS_AudioDecoder_Start(decoder, &startSettings);
    if ( errCode )
    {
        fprintf(stderr, "Unable to start decoder\n");
        return -1;
    }

    fprintf(stderr, "Press enter to exit\n");
    getchar();

    NEXUS_AudioDecoder_Stop(decoder);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
#endif
    NEXUS_AudioInput_Shutdown(NEXUS_I2sInput_GetConnector(i2sInput));
    NEXUS_I2sInput_Close(i2sInput);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(decoder, NEXUS_AudioDecoderConnectorType_eStereo));
    NEXUS_AudioDecoder_Close(decoder);

	return 0;
}
