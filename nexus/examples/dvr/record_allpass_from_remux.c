/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2008-2016 Broadcom. All rights reserved.
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
 *****************************************************************************/
/* Nexus example app: allpass record from playback through remux (ie. playback -> remux -> parser band -> record) */

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_RECORD && NEXUS_HAS_PLAYBACK
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_recpump.h"
#include "nexus_record.h"
#include "nexus_playback.h"
#include "nexus_file.h"

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <assert.h>

int main(void) {
    NEXUS_FileRecordHandle file;
    NEXUS_FilePlayHandle playfile;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecordHandle record;
    NEXUS_PidChannelHandle pidChannel[2];
    NEXUS_PlaybackPidChannelSettings playbackPidChannelSettings;
    NEXUS_PidChannelSettings pidChannelSettings;
    NEXUS_RecordSettings recordCfg;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    const char *recfname = "videos/remux_allpass.mpg";
    const char *playfname = "videos/cnnticker.mpg";
    NEXUS_PlatformSettings platformSettings;
    NEXUS_RemuxHandle remux;
    NEXUS_RemuxSettings remuxSettings;
    NEXUS_Error rc;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;    
    NEXUS_TransportCapabilities cap;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_GetTransportCapabilities(&cap);
    if (!cap.numRemux) {
        printf("This application is not supported on this chip. No remux.\n");
        return 0;
    }

    /* Set up the playback source */
    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    playfile = NEXUS_FilePlay_OpenPosix(playfname, NULL);
    if (!playfile) {
        fprintf(stderr, "can't open file:%s\n", playfname);
        return -1;
    }

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
    playbackSettings.playpumpSettings.allPass = true;
    playbackSettings.playpumpSettings.acceptNullPackets = true;
    NEXUS_Playback_SetSettings(playback, &playbackSettings);

    /* Open a pid channel from playback */
    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidChannelSettings);
    /* allpass requires that .pidChannelIndex be set to a special hardware-defined value. */
    NEXUS_Playpump_GetAllPassPidChannelIndex(playbackSettings.playpump, &playbackPidChannelSettings.pidSettings.pidSettings.pidChannelIndex );
    pidChannel[0] = NEXUS_Playback_OpenPidChannel(playback, 0x0, &playbackPidChannelSettings); /* pidNo is ignored for allpass */

    /* Configure remux. ie. Map playback => remux */
    NEXUS_Remux_GetDefaultSettings(&remuxSettings);
    remuxSettings.allPass = true;
    remux = NEXUS_Remux_Open(0, &remuxSettings);
    rc = NEXUS_Remux_AddPidChannel(remux, pidChannel[0]);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Remux_Start(remux); /* this registers remux as a playpump consumer */
    BDBG_ASSERT(!rc);

    /* Map remux output to a parser band via loopback route */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eRemux;
    parserBandSettings.sourceTypeSettings.remux = remux;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    parserBandSettings.allPass = true;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open a pid channel from parser band */
    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    /* allpass requires pidChannelIndex be set to a hardware-defined value. */
    NEXUS_ParserBand_GetAllPassPidChannelIndex(parserBand, &pidChannelSettings.pidChannelIndex);
    pidChannel[1] = NEXUS_PidChannel_Open(parserBand, 0x0, &pidChannelSettings); /* pidNo is ignored for allpass */

    /* Open recpump and record */
    recpump = NEXUS_Recpump_Open(0, NULL);
    record = NEXUS_Record_Create();
    NEXUS_Record_GetDefaultSettings(&recordCfg);
    recordCfg.recpump = recpump;
    NEXUS_Record_SetSettings(record, &recordCfg);

    file = NEXUS_FileRecord_OpenPosix(recfname, NULL);

    /* Start record */
    NEXUS_Record_AddPidChannel(record, pidChannel[1], NULL);
    NEXUS_Record_Start(record, file);

    /* Start playback */
    NEXUS_Playback_Start(playback, playfile, NULL);
    
    printf("press ENTER to stop record\n");
    getchar();

    NEXUS_Playback_Stop(playback);
    NEXUS_Record_Stop(record);
    NEXUS_Remux_Stop(remux);
    NEXUS_FileRecord_Close(file);

    return 0;
}
#else /* NEXUS_HAS_RECORD && NEXUS_HAS_PLAYBACK */
int main(void)
{
    printf("This application is not supported on this platform (needs record and playback)!\n");
    return 0;
}
#endif
