/******************************************************************************
 *    (c)2010-2014 Broadcom Corporation
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

#if NEXUS_HAS_AUDIO
#include "nexus_platform_client.h"
#include "nexus_audio_capture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "nxclient.h"
#include "wav_file.h"

BDBG_MODULE(audio_capture);

static void print_usage(void)
{
    printf(
        "Usage: audio_capture [FILE]\n"
        );
}

int main(int argc, char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_AudioCaptureHandle audioCapture;
    NEXUS_AudioCaptureStartSettings startSettings;
    int rc;
    int curarg = 1;
    const char *filename = NULL;
    FILE *file = NULL;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!filename) {
            filename = argv[curarg];
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    if (filename) {
        file = fopen(filename, "w+");
        if (!file) {
            fprintf(stderr, "### unable to open %s\n", filename);
            return -1;
        }
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.audioCapture = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {rc = BERR_TRACE(rc); goto err_request;}

    audioCapture = NEXUS_AudioCapture_Open(allocResults.audioCapture.id, NULL);
    if (!audioCapture) {
        BDBG_ERR(("unable to acquire audio playback"));
        goto err_acquire;
    }

    NEXUS_AudioCapture_GetDefaultStartSettings(&startSettings);
    rc = NEXUS_AudioCapture_Start(audioCapture, &startSettings);
    if (rc) {rc = BERR_TRACE(rc);goto err_start;}

    if (file) {
        /* write a WAV header */
        NEXUS_AudioCaptureSettings settings;
        NEXUS_AudioCaptureStatus status;
        struct wave_header wave_header;

        get_default_wave_header(&wave_header);
        NEXUS_AudioCapture_GetSettings(audioCapture, &settings);
        wave_header.riffCSize = 0xfffffff0;
        wave_header.dataLen = 0xffffffcc;
        switch (settings.format) {
        default:
        case NEXUS_AudioCaptureFormat_e16BitStereo:
            wave_header.bps = 16;
            wave_header.channels = 2;
            break;
        case NEXUS_AudioCaptureFormat_e24BitStereo:
            wave_header.bps = 32;  /* We are actually capturing the data as 32 bit */
            wave_header.channels = 2;
            break;
        case NEXUS_AudioCaptureFormat_e24Bit5_1:
            wave_header.bps = 32;  /* We are actually capturing the data as 32 bit */
            wave_header.channels = 6;
            break;
        case NEXUS_AudioCaptureFormat_e16BitMonoLeft:
        case NEXUS_AudioCaptureFormat_e16BitMonoRight:
        case NEXUS_AudioCaptureFormat_e16BitMono:
            wave_header.bps = 16;
            wave_header.channels = 1;
            break;
        }

        BDBG_WRN(("Capture format %u, bits per sample %u, num chs %u", settings.format, wave_header.bps, wave_header.channels));
        rc = NEXUS_AudioCapture_GetStatus(audioCapture, &status);
        BDBG_ASSERT(!rc);
        wave_header.samplesSec = status.sampleRate;
        wave_header.bytesSec = wave_header.samplesSec * wave_header.channels * wave_header.bps / 8;
        wave_header.chbits = wave_header.channels * wave_header.bps / 8;
        write_wave_header(file, &wave_header);
    }

    while (1) {
        void *buffer;
        size_t size;
        NEXUS_Error rc;
        rc = NEXUS_AudioCapture_GetBuffer(audioCapture, &buffer, &size);
        if ( rc != NEXUS_SUCCESS )
        {
            BDBG_ERR(("Capture has been stopped!!"));
        }
        else if (!size) {
            BKNI_Sleep(100); /* could register for callback and wait for event */
            continue;
        }

        if ( file )
        {
            fwrite(buffer, size, 1, file);
        }
        BDBG_WRN(("read %d bytes", size));
        NEXUS_AudioCapture_ReadComplete(audioCapture, size);
    }

    NEXUS_AudioCapture_Stop(audioCapture);
    if (file) {
        fclose(file);
    }

err_start:
    NEXUS_AudioCapture_Close(audioCapture);
err_acquire:
    NxClient_Free(&allocResults);
err_request:
    NxClient_Uninit();

    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
