/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "bstd.h"
#include "nexus_parser_band.h"
#include "nexus_platform.h"
#include "nexus_platform_features.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "bkni.h"
#include "bkni_multi.h"

BDBG_MODULE(live_playback_records);

void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

typedef struct
{
	NEXUS_RecpumpHandle recpump;
	BKNI_EventHandle stopRequested;
	BKNI_EventHandle stopCompleted;
	BKNI_EventHandle dataReady;
	NEXUS_ThreadHandle thread;
}
recordResources;

typedef struct
{
	NEXUS_ParserBand parserBand;
	NEXUS_PidChannelHandle pidChannel;
	recordResources recResources;
}
liveRecordResources;

static void recpumpThread(
	void *parm
	)
{
	recordResources *r = parm;
	unsigned long data_total = 0;
	unsigned long index_total = 0;

    while (BKNI_WaitForEvent(r->stopRequested, 0) != BERR_SUCCESS)
	{
        const void *data_buffer, *index_buffer;
        size_t data_buffer_size, index_buffer_size;
        int n;
		NEXUS_Error rc;

        rc = NEXUS_Recpump_GetDataBuffer(r->recpump, &data_buffer, &data_buffer_size);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Recpump_GetIndexBuffer(r->recpump, &index_buffer, &index_buffer_size);
        BDBG_ASSERT(!rc);
        if (data_buffer_size == 0 && index_buffer_size == 0) {
            rc = BKNI_WaitForEvent(r->dataReady, BKNI_INFINITE);
			BDBG_ASSERT(!rc);
            continue;
        }

		/* They are just measuring power consumption, so throw the data away. */
        if (data_buffer_size) {
			data_total += data_buffer_size;
            rc = NEXUS_Recpump_DataReadComplete(r->recpump, data_buffer_size);
            BDBG_ASSERT(!rc);
        }
        if (index_buffer_size) {
			index_total += index_buffer_size;
            rc = NEXUS_Recpump_IndexReadComplete(r->recpump, index_buffer_size);
            BDBG_ASSERT(!rc);
        }
		BDBG_MSG(("recordThread %p: data %lu, index %lu", (void *) r->recpump, data_total, index_total));
    }
	BKNI_SetEvent(r->stopCompleted);
}

void setupRecord(
	recordResources *resources,
	NEXUS_PidChannelHandle pidChannel
	)
{
	NEXUS_Error rc;
    NEXUS_RecpumpOpenSettings recpumpOpen;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_ThreadSettings threadSettings;
    NEXUS_RecpumpAddPidChannelSettings addPidChannelSettings;

	BKNI_CreateEvent(&resources->stopRequested);
	BDBG_ASSERT(resources->stopRequested);
	BKNI_CreateEvent(&resources->stopCompleted);
	BDBG_ASSERT(resources->stopCompleted);
	BKNI_CreateEvent(&resources->dataReady);
	BDBG_ASSERT(resources->dataReady);

	NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpen);
	resources->recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpen);
	BDBG_ASSERT(resources->recpump);

    NEXUS_Recpump_GetSettings(resources->recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.data.dataReady.context = resources->dataReady;
    recpumpSettings.index.dataReady.callback = dataready_callback; /* same callback */
    recpumpSettings.index.dataReady.context = resources->dataReady; /* same event */
    rc = NEXUS_Recpump_SetSettings(resources->recpump, &recpumpSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&addPidChannelSettings);
    rc = NEXUS_Recpump_AddPidChannel(resources->recpump, pidChannel, &addPidChannelSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Recpump_Start(resources->recpump);
    BDBG_ASSERT(!rc);

    NEXUS_Thread_GetDefaultSettings(&threadSettings);
	resources->thread = NEXUS_Thread_Create("recThread", recpumpThread, resources, &threadSettings);
	BDBG_ASSERT(resources->thread);
}

void teardownRecord(
	recordResources *resources
	)
{
	NEXUS_Thread_Destroy(resources->thread);
	NEXUS_Recpump_Stop(resources->recpump);
	NEXUS_Recpump_RemoveAllPidChannels(resources->recpump);
	NEXUS_Recpump_Close(resources->recpump);
	BKNI_DestroyEvent(resources->dataReady);
	BKNI_DestroyEvent(resources->stopRequested);
	BKNI_DestroyEvent(resources->stopCompleted);
}

void startLiveRecording(
	liveRecordResources *resources,
	NEXUS_InputBand inputBand
	)
{
	NEXUS_Error rc;
	NEXUS_ParserBandSettings parserSettings;
	NEXUS_PidChannelSettings pidChannelSettings;

	resources->parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
	BDBG_ASSERT(resources->parserBand);
	NEXUS_ParserBand_GetSettings(resources->parserBand, &parserSettings);
    parserSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
	parserSettings.sourceTypeSettings.inputBand = inputBand;
    parserSettings.transportType = NEXUS_TransportType_eTs;
    parserSettings.allPass = true;
    parserSettings.acceptNullPackets = true;
	rc = NEXUS_ParserBand_SetSettings(resources->parserBand, &parserSettings);
	BDBG_ASSERT(!rc);

	NEXUS_PidChannel_GetDefaultSettings(&pidChannelSettings);
	pidChannelSettings.continuityCountEnabled = false;
	NEXUS_ParserBand_GetAllPassPidChannelIndex(resources->parserBand, &pidChannelSettings.pidChannelIndex);
	resources->pidChannel = NEXUS_PidChannel_Open(resources->parserBand, 0, &pidChannelSettings);
	BDBG_ASSERT(resources->pidChannel);

	setupRecord(&resources->recResources, resources->pidChannel);
}

void stopLiveRecording(
	liveRecordResources *resources
	)
{
	BERR_Code brc;

	BKNI_SetEvent(resources->recResources.stopRequested);
    brc = BKNI_WaitForEvent(resources->recResources.stopCompleted, BKNI_INFINITE);
	BDBG_ASSERT(!brc);

	teardownRecord(&resources->recResources);
	NEXUS_PidChannel_Close(resources->pidChannel);
	NEXUS_ParserBand_Close(resources->parserBand);
}

typedef struct
{
	FILE *inputFile;
	NEXUS_PlaypumpHandle playpump;
	NEXUS_PidChannelHandle pidChannel;
	BKNI_EventHandle stopRequested;
	BKNI_EventHandle stopCompleted;
	BKNI_EventHandle bufferAvailable;
	NEXUS_ThreadHandle thread;
	recordResources recResources;
}
pbRecordResources;

static void playpumpThread(
	void *parm
	)
{
	#define BLOCK_SIZE	(188 * 100)

    void *buffer;
	size_t buffer_size;
	NEXUS_Error rc;

	pbRecordResources *r = parm;
	unsigned long total = 0;

    while (BKNI_WaitForEvent(r->stopRequested, 0) != BERR_SUCCESS)
	{
		int n;

		rc = NEXUS_Playpump_GetBuffer(r->playpump, &buffer, &buffer_size);
		BDBG_ASSERT(!rc);
		if(!buffer_size)
		{
			rc = BKNI_WaitForEvent(r->bufferAvailable, BKNI_INFINITE);
			BDBG_ASSERT(!rc);
			continue;
		}

		if(buffer_size > BLOCK_SIZE)
			buffer_size = BLOCK_SIZE;
		n = fread(buffer, 1, buffer_size, r->inputFile);
		BDBG_ASSERT(n >= 0);

		if(!n)
		{
			fseek(r->inputFile, 0, SEEK_SET);
			NEXUS_Playpump_Flush(r->playpump);
		}
		else
		{
			rc = NEXUS_Playpump_WriteComplete(r->playpump, 0, n);
			BDBG_ASSERT(!rc);
			total += n;
			BDBG_MSG(("playpumpThread %p: total %lu", (void *) r->playpump, total));
		}
	}

	BKNI_SetEvent(r->stopCompleted);
}

void startPbRecording(
	pbRecordResources *resources,
	char *fileName
	)
{
	NEXUS_Error rc;
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpOpenPidChannelSettings pidChannelSettings;
    NEXUS_ThreadSettings threadSettings;
	NEXUS_PlaypumpOpenSettings playpumpOpen;

	resources->inputFile = fopen(fileName, "rb");
	BDBG_ASSERT(resources->inputFile);

	NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpen);
	playpumpOpen.numDescriptors *= 10;
	resources->playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpen);
	BDBG_ASSERT(resources->playpump);

	BKNI_CreateEvent(&resources->stopRequested);
	BDBG_ASSERT(resources->stopRequested);
	BKNI_CreateEvent(&resources->stopCompleted);
	BDBG_ASSERT(resources->stopCompleted);
	BKNI_CreateEvent(&resources->bufferAvailable);
	BDBG_ASSERT(resources->bufferAvailable);

    NEXUS_Playpump_GetSettings(resources->playpump, &playpumpSettings);
	playpumpSettings.continuityCountEnabled = false;
	playpumpSettings.allPass = true;
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = resources->bufferAvailable;
    playpumpSettings.transportType = NEXUS_TransportType_eTs;
    playpumpSettings.mode = NEXUS_PlaypumpMode_eFifo;
    rc = NEXUS_Playpump_SetSettings(resources->playpump, &playpumpSettings);
	BDBG_ASSERT(!rc);

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&pidChannelSettings);
	NEXUS_Playpump_GetAllPassPidChannelIndex(resources->playpump, &pidChannelSettings.pidSettings.pidChannelIndex);
	resources->pidChannel = NEXUS_Playpump_OpenPidChannel(resources->playpump, 0, &pidChannelSettings);
	BDBG_ASSERT(resources->pidChannel);

	setupRecord(&resources->recResources, resources->pidChannel);

    rc = NEXUS_Playpump_Start(resources->playpump);
	BDBG_ASSERT(!rc);
    NEXUS_Thread_GetDefaultSettings(&threadSettings);
	resources->thread = NEXUS_Thread_Create("playpumpThread", playpumpThread, resources, &threadSettings);
	BDBG_ASSERT(resources->thread);
}

void stopPbRecording(
	pbRecordResources *resources
	)
{
	BERR_Code brc;

	/* Stop the playpump thread */
	BKNI_SetEvent(resources->stopRequested);
    brc = BKNI_WaitForEvent(resources->stopCompleted, BKNI_INFINITE);
	BDBG_ASSERT(!brc);

	/* Stop the recpump thread and release the resources */
	BKNI_SetEvent(resources->recResources.stopRequested);
    brc = BKNI_WaitForEvent(resources->recResources.stopCompleted, BKNI_INFINITE);
	BDBG_ASSERT(!brc);
	teardownRecord(&resources->recResources);

	NEXUS_Thread_Destroy(resources->thread);
	NEXUS_Playpump_Stop(resources->playpump);
	NEXUS_Playpump_ClosePidChannel(resources->playpump, resources->pidChannel);
	NEXUS_Playpump_Close(resources->playpump);
	BKNI_DestroyEvent(resources->stopRequested);
	BKNI_DestroyEvent(resources->stopCompleted);
	BKNI_DestroyEvent(resources->bufferAvailable);
	fclose(resources->inputFile);
}

int main(
	int argc,
	char **argv
	)
{
	#define MAX_FILE_NAME_LEN	(80)

	int opt;
	unsigned i;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc;
	NEXUS_InputBand inputBand;

	unsigned numLive = 0;
	unsigned numPb = 0;
	char fileName[MAX_FILE_NAME_LEN] = "videos/cnnticker.mpg";
	liveRecordResources *liveRecs = NULL;
	pbRecordResources *pbRecs = NULL;

	if(argc < 2)
	{
		printf("Usage: %s -l <num live records> -p <num playback records> -f <playback file name>\n", argv[0]);
		printf("All playbacks will use the same input file. All records are all-pass.\n");
		printf("Defaults are %u live records, %u playback records, playback file %s\n", numLive, numPb, fileName);
		return 0;
	}

    while((opt = getopt(argc, argv, "l:p:f:")) != -1)
    {
        switch(opt)
        {
            case 'l':	/* number of live records */
			numLive = atoi(optarg);
            break;

			case 'p':	/* number of records from playback */
			numPb = atoi(optarg);
			break;

			case 'f':	/* file for playback */
			strncpy(fileName, optarg, MAX_FILE_NAME_LEN);
			break;

			default:
			printf("Unrecognized option %c\n", (char) opt);
			break;
		}
	}
	printf("Starting %u live record(s), %u playback record(s), playback file %s\n", numLive, numPb, fileName);

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.videoDecoderModuleSettings.deferInit = true;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);
	NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

	if(numLive)
	{
		liveRecs = BKNI_Malloc(numLive * sizeof(*liveRecs));
		BDBG_ASSERT(liveRecs);
		BKNI_Memset(liveRecs, 0, numLive * sizeof(*liveRecs));
		for(i = 0; i < numLive; i++)
			startLiveRecording(liveRecs + i, inputBand);
	}
	if(numPb)
	{
		pbRecs = BKNI_Malloc(numPb * sizeof(*pbRecs));
		BDBG_ASSERT(pbRecs);
		BKNI_Memset(pbRecs, 0, numLive * sizeof(*pbRecs));
		for(i = 0; i < numPb; i++)
			startPbRecording(pbRecs + i, fileName);
	}

	printf("Press any key to stop recording and exit\n");
	(void) getchar();

	if(numLive)
	{
		for(i = 0; i < numLive; i++)
			stopLiveRecording(liveRecs + i);
		BKNI_Free(liveRecs);
	}
	if(numPb)
	{
		for(i = 0; i < numPb; i++)
			stopPbRecording(pbRecs + i);
		BKNI_Free(pbRecs);
	}
    NEXUS_Platform_Uninit();
	return 0;
}
