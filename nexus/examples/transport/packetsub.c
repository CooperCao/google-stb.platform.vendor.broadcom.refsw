/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "nexus_platform.h"
#include "nexus_packetsub.h"
#include "nexus_parser_band.h"
#include "nexus_recpump.h" /* for verification */

#include "bstd.h"
#include "bkni.h"
#include <stdio.h>

BDBG_MODULE(packetsub);

void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
}

int main(void) {
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PidChannelHandle pidChannel;
    NEXUS_PidChannelSettings pidChannelSettings;
    NEXUS_PacketSubHandle packetSub;
    NEXUS_PacketSubOpenSettings packetSubOpenSettings;
    NEXUS_PacketSubSettings packetSubSettings;
    unsigned pid = 0x0;
    uint8_t *pkt;
    unsigned size;
    NEXUS_Error rc;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecpumpSettings recpumpSettings;
    unsigned total_data = 0;
    FILE *data_file;

    NEXUS_Platform_Init(NULL);

    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.continuityCountEnabled = false; /* disable CC checking since our PAT below does not have valid CCs */
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    pidChannelSettings.pidChannelIndex = NEXUS_PID_CHANNEL_OPEN_INJECTION; /* use a HW pid channel for packetsub injection */
    pidChannel = NEXUS_PidChannel_Open(parserBand, 0 /* don't care */, &pidChannelSettings);

    NEXUS_PacketSub_GetDefaultOpenSettings(&packetSubOpenSettings);
    packetSubOpenSettings.fifoSize = 1024;

    packetSub = NEXUS_PacketSub_Open(0, &packetSubOpenSettings);

    NEXUS_PacketSub_GetSettings(packetSub, &packetSubSettings);
    packetSubSettings.pidChannel = pidChannel;
    packetSubSettings.loop = true;
    rc = NEXUS_PacketSub_SetSettings(packetSub, &packetSubSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_PacketSub_GetBuffer(packetSub, (void **)&pkt, &size);
    BDBG_ASSERT(!rc);
    BDBG_ASSERT(size >= 188);

    /* create new packet to replace the PAT */
    pkt[0] = 0x47;   /* SYNC BYTE */
    pkt[1] = (pid >> 8) & 0x1f;
    pkt[2] = pid & 0xff;  /* PID */
    pkt[3] = 0x30; /* not scrambled, adaptation_field then payload, 0 continuity counter */
    pkt[4] = 188-5-4; /* leave only 4 bytes of payload */
    BKNI_Memset(&pkt[5], 0, 188-5-4); /* zero out adaptation field */
    pkt[184] = 0xde;
    pkt[185] = 0xad;
    pkt[186] = 0xbe;
    pkt[187] = 0xef;

    rc = NEXUS_PacketSub_WriteComplete(packetSub, 188);
    BDBG_ASSERT(!rc);

    rc = NEXUS_PacketSub_Start(packetSub);
    BDBG_ASSERT(!rc);

    /**
    Use a recpump to record the result for verification
    **/
    recpump = NEXUS_Recpump_Open(0, NULL);
    data_file = fopen("packetsub_results.ts", "wb");
    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);
    rc = NEXUS_Recpump_AddPidChannel(recpump, pidChannel, NULL);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Recpump_Start(recpump);
    BDBG_ASSERT(!rc);
    while (total_data < 32 * 1024) {
        const void *buffer;
        size_t size;

        rc = NEXUS_Recpump_GetDataBuffer(recpump, &buffer, &size);
        BDBG_ASSERT(!rc);
        if (!size) {
            BKNI_Sleep(1);
            continue;
        }
        fwrite(buffer, size, 1, data_file);
        rc = NEXUS_Recpump_DataReadComplete(recpump, size);
        BDBG_ASSERT(!rc);
        printf("writing %d bytes\n", size);
        total_data += size;
    }
    NEXUS_Recpump_Stop(recpump);
    NEXUS_Recpump_RemoveAllPidChannels(recpump);
    NEXUS_Recpump_Close(recpump);
    fclose(data_file);

    NEXUS_PacketSub_Stop(packetSub);
    NEXUS_PacketSub_Close(packetSub);
    NEXUS_PidChannel_Close(pidChannel);
    NEXUS_Platform_Uninit();
    return 0;
}
