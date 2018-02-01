/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "bstd.h"
#include "bkni.h"
#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_remux.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "nexus_playback.h"
#include "nexus_record.h"
#include "nexus_file.h"
#include "bchp_xpt_fe.h"
#include "bmmt.h"

BDBG_MODULE(tlv_isdb_s3_all_pass_record);

/**
  * tlv_isdb_s3_all_pass_record app detects the bit shift in TLV
  * stream and programs the BCM7278 transport with the bit shift
  * and records the tlv file in all pass mode with corrected bit
  * shifts.
  *
  * DekTek DTU-205 streamer is used to live stream a tlv file to
  * BCM9TS_DC stream (coax to serial) which in turn feeds into
  * serial transport input of BCM7278.
  *
  * live stream file brcm_mmt_1pgm_36mbps_hevc_aac.v2.tlv is a
  * broadcom generated stream with 38.8Mbps.
  *
  **/

static void recpump_dataready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
    return;
}

static void recpump_overflow_callback(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    BDBG_ERR(("Recpump overflow!"));
    return;
}

static void callbackHandler( void *context, int param )
{
	BSTD_UNUSED(param);
	BKNI_SetEvent((BKNI_EventHandle) context);
}

int main(int argc, char **argv)
{
    NEXUS_Error rc;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PidChannelHandle liveAllPassPidChannel;
    NEXUS_FileRecordHandle recFile;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecordHandle record;
    NEXUS_RecordSettings recordCfg;
    NEXUS_RecordPidChannelSettings recordPidSettings;
    NEXUS_PidChannelSettings pidChannelSettings;
    NEXUS_InputBandSettings inputBandSettings;
    uint32_t reg;
    unsigned bitShift;
    BKNI_EventHandle rec_event;
    const void *data_buffer;
    void *parse_buffer;
    size_t data_buffer_size;

    if(argc != 2)
    {
	    fprintf(stderr, "Usage: %s <recorded file>\n", argv[0]);
        return -1;
    }

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.transportModuleSettings.maxDataRate.parserBand[NEXUS_ParserBand_e0] = 108 * 1000 * 1000;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);
    recFile = NEXUS_FileRecord_OpenPosix(argv[1], NULL);
    if (!recFile)
    {
        fprintf(stderr, "can't open file:%s\n", argv[1]);
		return -1;
    }
    NEXUS_InputBand_GetSettings(NEXUS_InputBand_e0, &inputBandSettings);
    inputBandSettings.useInternalSync = false;
    inputBandSettings.validEnabled = false;
    NEXUS_InputBand_SetSettings(NEXUS_InputBand_e0, &inputBandSettings);
    NEXUS_InputBand_ArmSyncGeneration(NEXUS_InputBand_e0,0);
    NEXUS_ParserBand_GetSettings(NEXUS_ParserBand_e0, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.allPass = true;
    parserBandSettings.continuityCountEnabled = false;
    parserBandSettings.teiIgnoreEnabled = true;
    parserBandSettings.acceptNullPackets = true;
    parserBandSettings.acceptAdapt00 = true;
    NEXUS_ParserBand_SetSettings(NEXUS_ParserBand_e0, &parserBandSettings);

    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    rc = NEXUS_ParserBand_GetAllPassPidChannelIndex(NEXUS_ParserBand_e0, &pidChannelSettings.pidChannelIndex);
    BDBG_ASSERT(!rc);
    pidChannelSettings.continuityCountEnabled = false;      /* needs to be off for bulk mode. */
    liveAllPassPidChannel = NEXUS_PidChannel_Open(NEXUS_ParserBand_e0, 0, &pidChannelSettings);
    BDBG_ASSERT(liveAllPassPidChannel);

    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    recpumpOpenSettings.data.bufferSize = recpumpOpenSettings.data.bufferSize*2;
    recpumpOpenSettings.indexType = NEXUS_RecpumpIndexType_eNone;
    recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);

    BKNI_CreateEvent(&rec_event);
    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = recpump_dataready_callback;
    recpumpSettings.data.overflow.callback = recpump_overflow_callback;
    recpumpSettings.data.dataReady.context = rec_event;
    recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eDisable;
    recpumpSettings.outputTransportType = NEXUS_TransportType_eBulk;
    NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);
    NEXUS_Recpump_AddPidChannel(recpump,liveAllPassPidChannel,NULL);

    NEXUS_Recpump_Start(recpump);
readAgain:
    rc = NEXUS_Recpump_GetDataBuffer(recpump, &data_buffer, &data_buffer_size);
    BDBG_ASSERT(!rc);

    if (data_buffer_size < 128*1024)
    {
       BDBG_MSG((" data_buffer_size %u < 128*1024",data_buffer_size));
       BKNI_Sleep(10);
       goto readAgain;
    }
    else
    {
       parse_buffer = BKNI_Malloc(128*1024);
       BKNI_Memcpy(parse_buffer,data_buffer,128*1024);
       NEXUS_Recpump_Stop(recpump);
       NEXUS_Recpump_RemoveAllPidChannels(recpump);
       NEXUS_PidChannel_Close(liveAllPassPidChannel);
       bitShift =  bmmt_get_tlv_sync_byte_bitshift(parse_buffer,128*1024);
       BKNI_Free(parse_buffer);
       BDBG_WRN(("parsed bitShift %u",bitShift));
       if (bitShift >= 8)
       {
          BDBG_WRN(("consecutive sync bytes not detected with bit shift 1-8"));
          goto done;
       }
       else
       {
          BDBG_WRN(("parsed bitShift %u",bitShift));
       }
    }

    NEXUS_InputBand_ArmSyncGeneration(NEXUS_InputBand_e0,bitShift);

    NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
    rc = NEXUS_ParserBand_GetAllPassPidChannelIndex(NEXUS_ParserBand_e0, &pidChannelSettings.pidChannelIndex);
    BDBG_ASSERT(!rc);
    pidChannelSettings.continuityCountEnabled = false;      /* needs to be off for bulk mode. */
    liveAllPassPidChannel = NEXUS_PidChannel_Open(NEXUS_ParserBand_e0, 0, &pidChannelSettings);
    BDBG_ASSERT(liveAllPassPidChannel);

    record = NEXUS_Record_Create();
    NEXUS_Record_GetSettings(record, &recordCfg);
    recordCfg.recpump = recpump;
    recordCfg.recpumpSettings.bandHold = NEXUS_RecpumpFlowControl_eDisable;
    recordCfg.recpumpSettings.outputTransportType = NEXUS_TransportType_eBulk;
    recordCfg.writeAllTimeout = 200;
    recordCfg.indexFormat = NEXUS_RecordIndexType_eNone;
    NEXUS_Record_SetSettings(record, &recordCfg);

    NEXUS_Record_GetDefaultPidChannelSettings(&recordPidSettings);
    recordPidSettings.recpumpSettings.pidType = NEXUS_PidType_eOther;
    recordPidSettings.recpumpSettings.pidTypeSettings.video.index = false;
    NEXUS_Record_AddPidChannel(record, liveAllPassPidChannel, &recordPidSettings);
    NEXUS_Record_Start(record, recFile);

    printf("Press any key to stop recording.\n");
    (void) getchar();

    NEXUS_Record_Stop(record);
 done:
    NEXUS_FileRecord_Close(recFile);
    NEXUS_Recpump_Close(recpump);
    BKNI_DestroyEvent(rec_event);
    NEXUS_Platform_Uninit();
    return 0;
}
