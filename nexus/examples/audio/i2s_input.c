/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nexus_audio_dac.h"
#include "nexus_i2s_input.h"
#include "nexus_audio_input.h"
#include "nexus_audio_output.h"

int main(int argc, char **argv)
{
    NEXUS_Error errCode;
    NEXUS_I2sInputHandle i2sInput;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_AudioCapabilities audioCapabilities;
    NEXUS_AudioOutputHandle audioDacHandle = NULL;

    argc=argc;
    argv=argv;

    NEXUS_Platform_Init(NULL);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (audioCapabilities.numInputs.i2s == 0)
    {
        printf("This application is not supported on this platform.\n");
        return 0;
    }

    if (audioCapabilities.numOutputs.dac > 0)
    {
        audioDacHandle = NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]);
    }

    i2sInput = NEXUS_I2sInput_Open(0, NULL);
    if ( NULL == i2sInput )
    {
        fprintf(stderr, "Unable to open i2s input 0\n");
        return -1;
    }

    if (audioDacHandle) {
        errCode = NEXUS_AudioOutput_AddInput(audioDacHandle,
                                             NEXUS_I2sInput_GetConnector(i2sInput));
        if ( errCode )
        {
            fprintf(stderr, "Unable to connect DAC to I2S Input\n");
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "No DAC Output\n");
        return -1;
    }

    errCode = NEXUS_I2sInput_Start(i2sInput);
    if ( errCode )
    {
        fprintf(stderr, "Unable to start I2S Capture\n");
        return -1;
    }

    fprintf(stderr, "Press enter to exit\n");
    getchar();

    NEXUS_I2sInput_Stop(i2sInput);
    if (audioCapabilities.numOutputs.dac > 0) {
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
    }

    NEXUS_AudioInput_Shutdown(NEXUS_I2sInput_GetConnector(i2sInput));
    NEXUS_I2sInput_Close(i2sInput);

    return 0;
}
#else /* NEXUS_HAS_AUDIO */
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
