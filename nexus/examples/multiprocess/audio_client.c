/******************************************************************************
 *    (c)2010-2012 Broadcom Corporation
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
#include "nexus_platform_client.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO
#include "nexus_simple_audio_playback.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

BDBG_MODULE(audio_client);

static void complete(void *context, int param)
{
    BSTD_UNUSED(param);
    BDBG_WRN(("pcm callback"));
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv)  {
    NEXUS_ClientConfiguration platformConfig;
    NEXUS_SimpleAudioPlaybackHandle audioPlayback;
    NEXUS_SimpleAudioPlaybackStartSettings startSettings;
    FILE *file;
    BKNI_EventHandle event;
    int rc;
    unsigned timeout = 0;
    int curarg = 1;
    bool loop = false;
    const char *filename = "audio.pcm";

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-loop")) {
            loop = true;
        }
        else {
            filename = argv[curarg];
        }
        curarg++;
    }

    rc = NEXUS_Platform_AuthenticatedJoin(NULL);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }
    
    NEXUS_Platform_GetClientConfiguration(&platformConfig);

    audioPlayback = NEXUS_SimpleAudioPlayback_Acquire(0);
    BDBG_ASSERT(audioPlayback);
    
    BKNI_CreateEvent(&event);
    file = fopen(filename, "r");
    if (!file) {
        BDBG_ERR(("unable to open %s", filename));
        return -1;
    }

    NEXUS_SimpleAudioPlayback_GetDefaultStartSettings(&startSettings);
    startSettings.dataCallback.callback = complete;
    startSettings.dataCallback.context = event;
    /* TODO: support WAV files for dynamic config */
    startSettings.sampleRate = 44100;
    startSettings.bitsPerSample = 16;
    startSettings.stereo = true;
    startSettings.loopAround = false;
    rc = NEXUS_SimpleAudioPlayback_Start(audioPlayback, &startSettings);
    if (rc) {rc = BERR_TRACE(rc);goto err_start;}

    while (1) {
        void *buffer;
        size_t size;
        int n;

        rc = NEXUS_SimpleAudioPlayback_GetBuffer(audioPlayback, &buffer, &size);
        BDBG_ASSERT(!rc);
        if (size == 0) {
            BKNI_WaitForEvent(event, 1000);
            continue;
        }
        n = fread(buffer, 1, size, file);
        if (n < 0) {
            break;
        }
        else if (n == 0) {
            if (loop) {
                fseek(file, 0, SEEK_SET);
                continue;
            }
            else {
                /* eof */
                break;
            }
        }

        rc = NEXUS_SimpleAudioPlayback_WriteComplete(audioPlayback, n);
        BDBG_ASSERT(!rc);
        printf("play %d bytes\n", n);
    }

    NEXUS_SimpleAudioPlayback_Stop(audioPlayback);
    
err_start:
    fclose(file);
    BKNI_DestroyEvent(event);    

    NEXUS_SimpleAudioPlayback_Release(audioPlayback);
    NEXUS_Platform_Uninit();

    return 0;
}
#else /* NEXUS_HAS_AUDIO */
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
