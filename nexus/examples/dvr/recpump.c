/******************************************************************************
 * Copyright (C) 2008-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_recpump.h"

#include <stdio.h>
#include <errno.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

BDBG_MODULE(recpump);

static void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void overflow_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    printf("overflow %s\n", (const char *)context);
}

static void print_status(NEXUS_RecpumpHandle recpump)
{
    NEXUS_RecpumpStatus status;
    NEXUS_Recpump_GetStatus(recpump, &status);
    printf("status: RAVE %d\n", status.rave.index);
    printf("  data:  %u total bytes \tfifo %u/%u\n", (unsigned)status.data.bytesRecorded, (unsigned)status.data.fifoDepth, (unsigned)status.data.fifoSize);
    printf("  index: %u total bytes \tfifo %u/%u\n", (unsigned)status.index.bytesRecorded, (unsigned)status.index.fifoDepth, (unsigned)status.index.fifoSize);
}

/* simple parsing of MPEG2 SCT
see BSEAV/lib/bcmplayer/src/bcmindexer for a complete example of SCT parsing */
struct sct6 { unsigned word[6]; };
static void parse_mpeg_sct(const void *buffer, unsigned size)
{
    const struct sct6 *sct;
    BDBG_ASSERT(size % sizeof(*sct) == 0); /* always process whole SCT's */
    for (sct = buffer; size; size -= sizeof(*sct), sct++) {
        uint8_t startCode = sct->word[2] >> 24;
        switch (startCode) {
        case 0xE0: 
            printf("0x%02x: PES header\n", startCode);
            break;
        case 0xFE: 
            printf("0x%02x: PTS %#x\n", startCode, sct->word[4]);
            break;
        case 0x00:
            {
            unsigned pic = (sct->word[2]>>11)&0x7;
            printf("0x%02x: picture %s\n", startCode, pic==1?"I":pic==2?"P":pic==3?"B":"unknown");
            }
            break;
        case 0x01:
            printf("0x%02x: first slice\n", startCode);
            break;
        default:
            printf("0x%02x: start code\n", startCode);
            break;
        }
    }
}

int main(void) {
    FILE *data_file = NULL;
    FILE *index_file = NULL;
    const char data_fname[] = "videos/stream.mpg";
    const char index_fname[] = "videos/stream.sct";
    BKNI_EventHandle event;
    NEXUS_InputBand inputBand;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpAddPidChannelSettings addPidChannelSettings;
    NEXUS_RecpumpHandle recpump;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PidChannelHandle pidChannel[2];
    NEXUS_Error rc;
    unsigned total = 0;

    /* Bring up all modules for a platform in a default configuration for this platform */
    rc = NEXUS_Platform_Init(NULL);
    if (rc) return -1;

    pidChannel[0]=NULL;
    pidChannel[1]=NULL;

    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    rc = NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    BDBG_ASSERT(!rc);

    BKNI_CreateEvent(&event);

    recpump = NEXUS_Recpump_Open(0, NULL);
    BDBG_ASSERT(recpump);

    data_file = fopen(data_fname, "wb");
    if (!data_file) {
        fprintf(stderr, "can't open data file:%s\n", data_fname);
        goto file_open_error;
    }
    index_file = fopen(index_fname, "wb");
    if (!index_file) {
        fprintf(stderr, "can't open index file:%s\n", index_fname);
        goto file_open_error;
    }

    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.data.dataReady.context = event;
    recpumpSettings.index.dataReady.callback = dataready_callback; /* same callback */
    recpumpSettings.index.dataReady.context = event; /* same event */
    recpumpSettings.data.overflow.callback = overflow_callback;
    recpumpSettings.data.overflow.context = "data";
    recpumpSettings.index.overflow.callback = overflow_callback;
    recpumpSettings.index.overflow.context = "index";
    rc = NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&addPidChannelSettings);
    addPidChannelSettings.pidType = NEXUS_PidType_eVideo;
    addPidChannelSettings.pidTypeSettings.video.index = true;
    addPidChannelSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
    pidChannel[0] = NEXUS_PidChannel_Open(parserBand, 0x11, NULL);
    rc = NEXUS_Recpump_AddPidChannel(recpump, pidChannel[0], &addPidChannelSettings);
    BDBG_ASSERT(!rc);

    pidChannel[1] = NEXUS_PidChannel_Open(parserBand, 0x14, NULL);
    rc = NEXUS_Recpump_AddPidChannel(recpump, pidChannel[1], NULL);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Recpump_Start(recpump);
    BDBG_ASSERT(!rc);

    print_status(recpump);
    while (total < 10*1024*1024) {
        const void *data_buffer, *index_buffer;
        size_t data_buffer_size, index_buffer_size;
        int n;

        rc = NEXUS_Recpump_GetDataBuffer(recpump, &data_buffer, &data_buffer_size);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Recpump_GetIndexBuffer(recpump, &index_buffer, &index_buffer_size);
        BDBG_ASSERT(!rc);
        if (data_buffer_size == 0 && index_buffer_size == 0) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
        }

        if (data_buffer_size) {
            n = fwrite(data_buffer, 1, data_buffer_size, data_file);
            if (n < 0) {BDBG_ERR(("fwrite error: %d\n", errno)); break;}
            rc = NEXUS_Recpump_DataReadComplete(recpump, n);
            BDBG_ASSERT(!rc);
            total += n;
        }
        if (index_buffer_size) {
            parse_mpeg_sct(index_buffer, index_buffer_size);
            n = fwrite(index_buffer, 1, index_buffer_size, index_file);
            if (n < 0) {BDBG_ERR(("fwrite error: %d\n", errno)); break;}
            rc = NEXUS_Recpump_IndexReadComplete(recpump, n);
            BDBG_ASSERT(!rc);
        }
        printf("wrote %u data, %u index\n", (unsigned)data_buffer_size, (unsigned)index_buffer_size);
        print_status(recpump);
    }
    
    NEXUS_Recpump_Stop(recpump);
file_open_error:
    NEXUS_Recpump_RemoveAllPidChannels(recpump);
    NEXUS_Recpump_Close(recpump);
    NEXUS_PidChannel_Close(pidChannel[0]);
    NEXUS_PidChannel_Close(pidChannel[1]);
    BKNI_DestroyEvent(event);
    if (index_file) fclose(index_file);
    if (data_file) fclose(data_file);
    NEXUS_Platform_Uninit();

    return 0;
}
