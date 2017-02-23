/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#if NEXUS_HAS_AUDIO
#include "nexus_platform.h"
#include "nexus_platform_server.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_audio_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <sys/wait.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(audio_img_load);

static int client(void)
{
    unsigned cnt = 0;
    while (1) {
        int rc;
        BKNI_Printf("NEXUS_Platform_Join %d\n", ++cnt);
        rc = NEXUS_Platform_Join();
        if (rc) {
            BKNI_Printf("NEXUS_Platform_Join failed: %d. This test is only supported in kernel mode.\n", rc);
            return rc;
        }
        BKNI_Sleep(10);
        NEXUS_Platform_Uninit();
    }
    return 0;
}

static int server(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_AudioDecoderHandle pcmDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_Error rc;
    NEXUS_ParserBand parserBand;
    unsigned pid;
    NEXUS_PlatformStartServerSettings serverSettings;
    NEXUS_AudioCapabilities cap;
    NEXUS_AudioCodec audioCodec = 0;
    unsigned loops = 30 * 60; /* 1 minute of test*/

    srand((unsigned)time(NULL));

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    NEXUS_Platform_GetDefaultStartServerSettings(&serverSettings);
    serverSettings.allowUnauthenticatedClients = true;
    serverSettings.unauthenticatedConfiguration.mode = NEXUS_ClientMode_eProtected;
    serverSettings.unauthenticatedConfiguration.heap[0] = platformConfig.heap[0];
    rc = NEXUS_Platform_StartServer(&serverSettings);
    BDBG_ASSERT(!rc);

    pid = fork();
    if (!pid) {
        rc = execlp("audio_img_load", "audio_img_load", "-client", NULL);
        BDBG_ERR(("failure to exec: %d", rc));
        return rc;
    }

    parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);

    pcmDecoder = NEXUS_AudioDecoder_Open(0, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#elif NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_AddInput(
        NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
        NEXUS_AudioDecoder_GetConnector(pcmDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#else
#error test not supported
#endif

    NEXUS_GetAudioCapabilities(&cap);
    while (loops--) {
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
        audioProgram.codec = audioCodec;
        audioProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, 1, NULL);
        BDBG_WRN(("start decode %d", audioCodec));
        rc = NEXUS_AudioDecoder_Start(pcmDecoder, &audioProgram);
        if (rc) {
            cap.dsp.codecs[audioCodec].decode = false;
        }
        else {
            BKNI_Sleep(10);
            NEXUS_AudioDecoder_Stop(pcmDecoder);
        }

        NEXUS_PidChannel_Close(audioProgram.pidChannel);

        do {
            if (++audioCodec == NEXUS_AudioCodec_eMax) audioCodec = 0;
        } while (!cap.dsp.codecs[audioCodec].decode);
    }

    kill(pid, 9);
    waitpid(pid, 0, 0);
    NEXUS_ParserBand_Close(parserBand);
    NEXUS_AudioDecoder_Close(pcmDecoder);
    NEXUS_Platform_Uninit();
    return 0;
}

int main(int argc, char **argv)
{
    BSTD_UNUSED(argv);
    if (argc > 1) {
        client();
    }
    else {
        server();
    }
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
