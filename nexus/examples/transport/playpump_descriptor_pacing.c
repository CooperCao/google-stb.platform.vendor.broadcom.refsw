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
#include <assert.h>
#include <errno.h>

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_playpump.h"
#include "nexus_recpump.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#define INPUT_FILE "videos/bugs_toys2_jurassic_q64_cd.mpg"
#define VIDEO_PID 	0x11

BDBG_MODULE(playpump_descriptor_pacing);

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

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

void print_recpump_status(NEXUS_RecpumpHandle recpump)
{
    NEXUS_RecpumpStatus status;

    NEXUS_Recpump_GetStatus(recpump, &status);
    printf("status: RAVE %d\n", status.rave.index);
    printf("  data:  %u total bytes \tfifo %u/%u\n", (unsigned)status.data.bytesRecorded, status.data.fifoDepth, status.data.fifoSize);
    printf("  index: %u total bytes \tfifo %u/%u\n", (unsigned)status.index.bytesRecorded, status.index.fifoDepth, status.index.fifoSize);
}

static FILE *inputFile;
static BKNI_EventHandle playEvent;
static BKNI_EventHandle playDone;
static NEXUS_PlaypumpHandle playpump;
static NEXUS_ThreadHandle playThread;

#define BLOCK_SIZE	(188 * 100)

static void playpumpThread(
	void *parm
	)
{
    void *buffer;
	size_t buffer_size;
	NEXUS_Error rc;
	NEXUS_PlaypumpWriteCompleteSettings settings;

	NEXUS_Playpump_GetDefaultWriteCompleteSettings(&settings);
	settings.descriptorPacing.pkt2pktDelta = 0x10;
	while(!feof(inputFile))
	{
	int n;

	rc = NEXUS_Playpump_GetBuffer(playpump, &buffer, &buffer_size);
	if(rc!=NEXUS_SUCCESS) {BDBG_ERR(("Get playpump buffer")); goto done;}
	if(buffer_size>0) {
		if(buffer_size > BLOCK_SIZE) {
		buffer_size = BLOCK_SIZE;
		}
		n = fread(buffer, 1, buffer_size, inputFile);
		if(n<=0) {BDBG_ERR(("fread failed")); goto done;}

			settings.descriptorPacing.timestamp += 0x1000;
		rc = NEXUS_Playpump_WriteCompleteWithSettings(playpump, 0, n, &settings);
		if(rc!=NEXUS_SUCCESS) {BDBG_ERR(("WriteComnplete")); goto done;}
	} else {
		BKNI_WaitForEvent(playEvent, BKNI_INFINITE);
	}
	}

	done:
	BKNI_SetEvent(playDone);
}

int main(void)
{
    NEXUS_PlaypumpSettings playpumpSettings;
    NEXUS_PlaypumpOpenPidChannelSettings playpumpPidChannelSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc;
    NEXUS_RecpumpOpenSettings recpumpOpen;
    NEXUS_RecpumpSettings recpumpSettings;
	NEXUS_RecpumpHandle recpump;
    NEXUS_RecpumpAddPidChannelSettings addPidChannelSettings;
    NEXUS_ThreadSettings threadSettings;
	NEXUS_PlaypumpOpenSettings pumpOpenSettings;
	FILE *dataFile;
	FILE *indexFile;
	BKNI_EventHandle recEvent;

	unsigned total = 0;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    inputFile = fopen(INPUT_FILE, "rb");
	BDBG_ASSERT(inputFile);

    dataFile = fopen("videos/stream.mpg", "wb");
	BDBG_ASSERT(dataFile);
    indexFile = fopen("videos/stream.idx", "wb");
	BDBG_ASSERT(indexFile);

	NEXUS_Playpump_GetDefaultOpenSettings(&pumpOpenSettings);
	pumpOpenSettings.descriptorPacingEnabled = true;
    playpump = NEXUS_Playpump_Open(0, &pumpOpenSettings);
	BDBG_ASSERT(playpump);

    BKNI_CreateEvent(&playEvent);
    BKNI_CreateEvent(&playDone);
    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = playEvent;
    playpumpSettings.transportType = NEXUS_TransportType_eTs;
    playpumpSettings.mode = NEXUS_PlaypumpMode_eFifo;
	playpumpSettings.timestamp.type = NEXUS_TransportTimestampType_eBinary;
	playpumpSettings.timestamp.pacing = true;
	playpumpSettings.timestamp.timebase = NEXUS_Timebase_eInvalid;	/* Use free-running for the sake of demo */
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);
    NEXUS_Playpump_Start(playpump);

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&playpumpPidChannelSettings);
    videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, VIDEO_PID, &playpumpPidChannelSettings);

	NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpen);
	recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpen);
	BDBG_ASSERT(recpump);

    BKNI_CreateEvent(&recEvent);
    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = dataready_callback;
    recpumpSettings.data.dataReady.context = recEvent;
    recpumpSettings.index.dataReady.callback = dataready_callback; /* same callback */
    recpumpSettings.index.dataReady.context = recEvent; /* same event */
    recpumpSettings.data.overflow.callback = overflow_callback;
    recpumpSettings.data.overflow.context = "data";
    recpumpSettings.index.overflow.callback = overflow_callback;
    recpumpSettings.index.overflow.context = "index";

	/* Setting recpumpSettings.timestampType will prepend 32-bit to each transport packet. This is done for the sake of debugging. */
	recpumpSettings.timestampType = NEXUS_TransportTimestampType_eBinary;

    rc = NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&addPidChannelSettings);
    addPidChannelSettings.pidType = NEXUS_PidType_eVideo;
    addPidChannelSettings.pidTypeSettings.video.index = true;
    addPidChannelSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
    rc = NEXUS_Recpump_AddPidChannel(recpump, videoPidChannel, &addPidChannelSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Recpump_Start(recpump);
    BDBG_ASSERT(!rc);

	/* Drive the playpump from a separate thread */
    NEXUS_Thread_GetDefaultSettings(&threadSettings);
	playThread = NEXUS_Thread_Create("playpumpThread", playpumpThread, NULL, &threadSettings);

    while (BKNI_WaitForEvent(playDone, 0) != BERR_SUCCESS) {
        const void *data_buffer, *index_buffer;
        size_t data_buffer_size, index_buffer_size;
        int n;

        rc = NEXUS_Recpump_GetDataBuffer(recpump, &data_buffer, &data_buffer_size);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Recpump_GetIndexBuffer(recpump, &index_buffer, &index_buffer_size);
        BDBG_ASSERT(!rc);
        if (data_buffer_size == 0 && index_buffer_size == 0) {
            BKNI_WaitForEvent(recEvent, BKNI_INFINITE);
            continue;
        }

        if (data_buffer_size) {
            n = fwrite(data_buffer, 1, data_buffer_size, dataFile);
            if (n < 0) {BDBG_ERR(("fwrite error: %d\n", errno)); break;}
            rc = NEXUS_Recpump_DataReadComplete(recpump, n);
            BDBG_ASSERT(!rc);
            total += n;
        }
        if (index_buffer_size) {
            n = fwrite(index_buffer, 1, index_buffer_size, indexFile);
            if (n < 0) {BDBG_ERR(("fwrite error: %d\n", errno)); break;}
            rc = NEXUS_Recpump_IndexReadComplete(recpump, n);
            BDBG_ASSERT(!rc);
        }
        printf("wrote %d data, %d index\n", data_buffer_size, index_buffer_size);
        print_recpump_status(recpump);
    }

	NEXUS_Playpump_Stop(playpump);
	NEXUS_Playpump_Close(playpump);
	NEXUS_Recpump_Stop(recpump);
	NEXUS_Recpump_Close(recpump);
	NEXUS_Thread_Destroy(playThread);
	BKNI_DestroyEvent(playEvent);
	BKNI_DestroyEvent(recEvent);
	BKNI_DestroyEvent(playDone);
	fclose(dataFile);
	fclose(indexFile);
	fclose(inputFile);
    NEXUS_Platform_Uninit();
	return 0;
}
