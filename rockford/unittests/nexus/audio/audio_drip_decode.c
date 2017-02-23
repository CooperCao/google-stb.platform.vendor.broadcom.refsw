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
 *
 * Module Description:
 *
 *****************************************************************************/
/* Nexus example app: playpump and decode */

#if NEXUS_HAS_AUDIO
#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_playpump.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#if NEXUS_DTV_PLATFORM
#include "nexus_platform_boardcfg.h"
#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

BDBG_MODULE(audio_drip_decode);

static void send_data(NEXUS_PlaypumpHandle playpump, const void *data, size_t len)
{
    while(len>0) {
        void *buffer;
        size_t buffer_size;

        if (NEXUS_Playpump_GetBuffer(playpump, &buffer, &buffer_size)!=NEXUS_SUCCESS) {
            BDBG_ASSERT(0);
        }
        /* BDBG_WRN(("buffer %#x %u", buffer, buffer_size)); */
        if(buffer == NULL || buffer_size==0) {
            BDBG_WRN(("buffer is full"));
            BKNI_Sleep(100);
            continue;
        }
        if(buffer_size>len) {
            buffer_size = len;
        }
        BKNI_Memcpy(buffer, data, buffer_size);
        if (NEXUS_Playpump_WriteComplete(playpump, 0, buffer_size)!=NEXUS_SUCCESS) {
            BDBG_ASSERT(0);
        }
        data = (uint8_t *)data + buffer_size;
        len -= buffer_size;
    }
    return;
}

int main(void)  {
    FILE *file;
    NEXUS_PlaypumpHandle playpump;
    const char *fname = "8k8bitpcm.wav.30ms.pes";
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_DisplayHandle display;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_PlaypumpSettings playpumpSettings;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);


    playpump = NEXUS_Playpump_Open(0, NULL);

    /* use stdio for file I/O to keep the example simple. */
    file = fopen(fname, "rb");
    if (!file) {
        fprintf(stderr, "can't open file:%s\n", fname);
        goto error;
    }

    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.transportType = NEXUS_TransportType_eMpeg2Pes;
    playpumpSettings.originalTransportType = NEXUS_TransportType_eMpeg2Pes;
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);

    NEXUS_Playpump_Start(playpump);

    stcChannel = NEXUS_StcChannel_Open(0, NULL);

    audioPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, 0xC0, NULL);

    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = NEXUS_AudioCodec_ePcmWav;
    audioProgram.pidChannel = audioPidChannel;

    /* Bring up video decoder and display. No audio to keep the example simple. */
    display = NEXUS_Display_Open(0, NULL);
#if NEXUS_DTV_PLATFORM
    NEXUS_Display_AddOutput(display, NEXUS_PanelOutput_GetConnector(platformConfig.outputs.panel[0]));
    NEXUS_BoardCfg_ConfigurePanel(true, true, true);
#else
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#endif
    audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
#if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_AddInput(
        NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
    NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

    for(;;) {
        int n;
        uint8_t frame[0x118];
        NEXUS_AudioDecoderStatus astatus;
        NEXUS_PlaypumpStatus pstatus;
        BERR_Code rc;

        n = fread(frame, 1, sizeof(frame), file);
        if (n < 0) goto error;
        if(n==0) {
            BDBG_WRN(("end of file"));
            fseek(file, 0, SEEK_SET);
            continue;
        }
        send_data(playpump, frame, n);
#if 1
        BKNI_Memset(frame, 0, sizeof(frame));
        frame[2] = 0x01;
        frame[3] = 0xEF;
        frame[6] = 0x81;
        send_data(playpump, frame, sizeof(frame));
#endif
#if 1
        BKNI_Sleep(100);
        rc = NEXUS_Playpump_GetStatus(playpump,&pstatus);
        BDBG_ASSERT(!rc);
        rc = NEXUS_AudioDecoder_GetStatus(audioDecoder, &astatus);
        BDBG_ASSERT(!rc);
        printf("playback: %lu/%lu audio %u/%u (%.1f) pts=%u, \n", (unsigned long)pstatus.fifoDepth, (unsigned long)pstatus.fifoSize, astatus.fifoDepth, astatus.fifoSize, astatus.fifoDepth/(float)sizeof(frame), astatus.pts);
#endif
#if 0
        BKNI_Sleep(100);
#endif
    }
    return 0;

error:
    return 1;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
