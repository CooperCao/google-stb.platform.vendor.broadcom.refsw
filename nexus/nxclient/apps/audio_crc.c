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
#include "nexus_audio_crc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"
#include "nxclient.h"
#include "wav_file.h"

BDBG_MODULE(audio_crc);

#define CRC_MAX_READ    100
#define MAX_CHAN_PAIRS  3

static void print_usage(void)
{
    printf(
        "Usage: audio_crc \n"
        " -multichannel (configures for multichannel output)\n"
        );
}

static void Crc_PrintCrcData (char * name, NEXUS_AudioCrcHandle handle, unsigned numChPairs);

FILE *file[MAX_CHAN_PAIRS];

int main(int argc, char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_AudioCrcHandle audioCrc;
    int rc;
    int curarg = 1;
    char filename[30];
    int channelPairs = 1;
    int i = 0;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-multichannel")) {
            channelPairs = 3;
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    memset(file, 0, sizeof(file));

    for (i = 0; i < channelPairs; i++)
    {
        sprintf(filename, "audioCrc%s_%d.txt",channelPairs==1?"stereo":"multichannel",i);
        file[i] = fopen(filename, "w+");
        if (!file[i]) {
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
    allocSettings.audioCrc = 1;
    allocSettings.audioCrcType.type = (channelPairs == 1 ? NxClient_AudioCrcType_eStereo : NxClient_AudioCrcType_eMultichannel);
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {rc = BERR_TRACE(rc); goto err_request;}

    audioCrc = NEXUS_AudioCrc_Open(allocResults.audioCrc.id, NULL);
    if (!audioCrc) {
        BDBG_ERR(("unable to acquire audio playback"));
        goto err_acquire;
    }

    while (1) {
        Crc_PrintCrcData(channelPairs == 1 ? "STEREO":"MULTICHANNEL", audioCrc, channelPairs);
        BKNI_Sleep(2000);
    }

    for (i = 0; i < channelPairs; i++)
    {
        if (file[i]) {
            fclose(file[i]);
        }
    }

    NEXUS_AudioCrc_Close(audioCrc);
err_acquire:
    NxClient_Free(&allocResults);
err_request:
    NxClient_Uninit();

    return 0;
}

NEXUS_AudioCrcEntry * Crc_GetEntryByIndex(
    const NEXUS_AudioCrcData * crcData,
    unsigned index
    )

{
    BDBG_ASSERT(crcData != NULL);

    switch ( index )
    {
        default:
        case 0:
            return (NEXUS_AudioCrcEntry*)&crcData->crc0;
            break;
        case 1:
            return (NEXUS_AudioCrcEntry*)&crcData->crc1;
            break;
        case 2:
            return (NEXUS_AudioCrcEntry*)&crcData->crc2;
            break;
        case 3:
            return (NEXUS_AudioCrcEntry*)&crcData->crc3;
            break;
    }

    return NULL;
}

static void Crc_PrintCrcData (char * name, NEXUS_AudioCrcHandle handle, unsigned numChPairs)
{
    NEXUS_AudioCrcData entries[CRC_MAX_READ];
    NEXUS_AudioCrcEntry * pEntry;
    unsigned numEntries = 0;
    NEXUS_Error err;
    unsigned i, j;

    err = NEXUS_AudioCrc_GetCrcData(handle, entries, CRC_MAX_READ, &numEntries);
    if ( err != BERR_SUCCESS || numEntries == 0 )
    {
        printf("%s - no data\n", __FUNCTION__);
        return;
    }

    printf("%s - NEXUS_AudioCrc_GetCrcData(%p) returned %d entries\n", name, (void*)handle, numEntries);
    for ( i = 0; i < numEntries; i++)
    {
        for ( j = 0; j < numChPairs; j++ )
        {
            char str[5];
            pEntry = Crc_GetEntryByIndex(&(entries[i]), j);
            printf("\t%s:chPair[%d] - seqNumber %d, value %04x\n", name, j,
                pEntry->seqNumber,
                pEntry->value);
            sprintf(str, "%04x", pEntry->value);
            fwrite(str, 1, sizeof(str)-1, file[j]);
            fflush(file[j]);
        }
    }
}

#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs audio)!\n");
    return 0;
}
#endif
