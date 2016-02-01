/******************************************************************************
 *    (c)2008-2011 Broadcom Corporation
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
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_recpump.h"

#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

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
    printf("status: \n");
    printf("  data:  %u total bytes \tfifo %u/%u\n", (unsigned)status.data.bytesRecorded, status.data.fifoDepth, status.data.fifoSize);
    printf("  index: %u total bytes \tfifo %u/%u\n", (unsigned)status.index.bytesRecorded, status.index.fifoDepth, status.index.fifoSize);
}

int main(void) {
    FILE *data_file;
    FILE *index_file;
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

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_Init(NULL);

    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    BKNI_CreateEvent(&event);

    recpump = NEXUS_Recpump_Open(0, NULL);

    data_file = fopen(data_fname, "wb");
    if (!data_file) {
        fprintf(stderr, "can't open data file:%s\n", data_fname);
        goto error;
    }
    index_file = fopen(index_fname, "wb");
    if (!index_file) {
        fprintf(stderr, "can't open index file:%s\n", index_fname);
        goto error;
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
    NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);

    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&addPidChannelSettings);
    addPidChannelSettings.pidType = NEXUS_PidType_eVideo;
    addPidChannelSettings.pidTypeSettings.video.index = true;
    addPidChannelSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
    pidChannel[0] = NEXUS_PidChannel_Open(parserBand, 0x11, NULL);
    NEXUS_Recpump_AddPidChannel(recpump, pidChannel[0], &addPidChannelSettings);

    pidChannel[1] = NEXUS_PidChannel_Open(parserBand, 0x14, NULL);
    NEXUS_Recpump_AddPidChannel(recpump, pidChannel[1], NULL);

    NEXUS_Recpump_Start(recpump);

    while (1) {
        const void *data_buffer[2], *index_buffer[2];
        size_t data_buffer_size[2], index_buffer_size[2];
        int n;

        if (NEXUS_Recpump_GetDataBufferWithWrap(recpump, &data_buffer[0], &data_buffer_size[0], &data_buffer[1], &data_buffer_size[1]))
            break;
        if (NEXUS_Recpump_GetIndexBufferWithWrap(recpump, &index_buffer[0], &index_buffer_size[0], &index_buffer[1], &index_buffer_size[1]))
            break;
        if (data_buffer_size[0] == 0 && index_buffer_size[0] == 0) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
        }

        if (data_buffer_size[0]) {
            ssize_t n2 = 0;
            n = fwrite(data_buffer[0], 1, data_buffer_size[0], data_file);
            if (n < 0) goto error;

            if (data_buffer_size[1]) {
                n2 = fwrite(data_buffer[1], 1, data_buffer_size[1], data_file);
                if (n2 < 0) goto error;
            }
            NEXUS_Recpump_DataReadComplete(recpump, n+n2);
        }
        if (index_buffer_size[0]) {
            ssize_t n2 = 0;
            n = fwrite(index_buffer[0], 1, index_buffer_size[0], index_file);
            if (n < 0) goto error;
            if (index_buffer_size[1]) {
                n2 = fwrite(index_buffer[1], 1, index_buffer_size[1], index_file);
                if (n2 < 0) goto error;
            }
            NEXUS_Recpump_IndexReadComplete(recpump, n+n2);
        }
        printf("wrote %d+%d data, %d+%d index\n", data_buffer_size[0], data_buffer_size[1], index_buffer_size[0], index_buffer_size[1]);
        print_status(recpump);
    }
    /* loops forever */

error:
    return 1;
}
