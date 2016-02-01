/******************************************************************************
 *    (c)2015 Broadcom Corporation
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
 *****************************************************************************/

#include "nexus_platform.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include <stdio.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(remap);

/* the following define the input file and its characteristics -- these will vary by input file */
#define FILE_NAME "videos/cnnticker.mpg"
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_PID 0x21

static void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
}

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_Error rc;
    int i;
    NEXUS_PlaypumpOpenPidChannelSettings playpumpPidSettings;
    FILE *f;
    void *buffer;
    size_t size;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;

    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(playpump);

    f = fopen(FILE_NAME, "r");
    BDBG_ASSERT(f);

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&playpumpPidSettings);
    videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, VIDEO_PID, &playpumpPidSettings);

    playpumpPidSettings.pidSettings.remap.enabled = true;
    playpumpPidSettings.pidSettings.remap.pid = 0x123;
    rc = NEXUS_PidChannel_SetRemapSettings(videoPidChannel, &playpumpPidSettings.pidSettings.remap);
    BDBG_ASSERT(!rc);

    recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(recpump);

    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    rc = NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Recpump_AddPidChannel(recpump, videoPidChannel, NULL);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Recpump_Start(recpump);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Playpump_Start(playpump);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Playpump_GetBuffer(playpump, &buffer, &size);
    BDBG_ASSERT(!rc);

    i = fread(buffer, 1, size, f);
    NEXUS_Playpump_WriteComplete(playpump, 0, i);
    BDBG_WRN(("wrote %d bytes", i));

    BKNI_Sleep(100);

    rc = NEXUS_Recpump_GetDataBuffer(recpump, (const void **)&buffer, &size);
    BDBG_ASSERT(!rc);

    for (i=0;i*188<(int)size && i<10;i++) {
        unsigned char *packet = &((unsigned char *)buffer)[i*188];
        unsigned pid = (((unsigned)packet[1]&0x1f)<<8) + packet[2];
        BDBG_WRN(("packet %d: pid 0x%x", i, pid));
    }

    NEXUS_Playpump_Close(playpump);
    NEXUS_Recpump_Close(recpump);
    fclose(f);
    NEXUS_Platform_Uninit();
    return 0;
}
