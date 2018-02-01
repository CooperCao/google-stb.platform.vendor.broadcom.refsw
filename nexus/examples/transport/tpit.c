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
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>

BDBG_MODULE(tpit);

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
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpAddPidChannelSettings recpumpPidSettings;
    NEXUS_RecpumpTpitFilter filter;
    NEXUS_Error rc;

    rc = NEXUS_Platform_Init(NULL);
    BDBG_ASSERT(!rc);

    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    pidChannel = NEXUS_PidChannel_Open(parserBand, 0x11, NULL);

    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    recpump = NEXUS_Recpump_Open(0, &recpumpOpenSettings);

    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.index.dataReady.callback = dataready_callback; /* same callback */
    rc = NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&recpumpPidSettings);
#if 0
    /* you can simultaneously index with the SCD and TPIT indexers */
    recpumpPidSettings.pidType = NEXUS_PidType_eVideo;
    recpumpPidSettings.pidTypeSettings.video.index = true;
    recpumpPidSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
#endif
    rc = NEXUS_Recpump_AddPidChannel(recpump, pidChannel, &recpumpPidSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Recpump_GetDefaultTpitFilter(&filter);
    filter.config.mpeg.randomAccessIndicatorEnable = true;
    filter.config.mpeg.randomAccessIndicatorCompValue = true;
    filter.config.mpeg.payloadUnitStartEnable = true;
    filter.config.mpeg.payloadUnitStartCompValue = true;
    rc = NEXUS_Recpump_SetTpitFilter(recpump, pidChannel, &filter);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Recpump_Start(recpump);
    BDBG_ASSERT(!rc);

    while (1) {
        const void *buffer;
        size_t datasize, indexsize;

        /* dispose of CDB data */
        rc = NEXUS_Recpump_GetDataBuffer(recpump, &buffer, &datasize);
        BDBG_ASSERT(!rc);
        if (datasize) {
            rc = NEXUS_Recpump_DataReadComplete(recpump, datasize);
            BDBG_ASSERT(!rc);
        }

        /* ITB contains TPIT results */
        rc = NEXUS_Recpump_GetIndexBuffer(recpump, &buffer, &indexsize);
        BDBG_ASSERT(!rc);
        if (indexsize) {
            printf("%u tpit bytes\n", (unsigned)indexsize);
            rc = NEXUS_Recpump_IndexReadComplete(recpump, indexsize);
            BDBG_ASSERT(!rc);
        }

        if (!datasize && !indexsize) {
            BKNI_Sleep(10);
        }
    }

    NEXUS_Recpump_Stop(recpump);
    NEXUS_Recpump_Close(recpump);
    NEXUS_PidChannel_Close(pidChannel);

    NEXUS_Platform_Uninit();
    return 0;
}
