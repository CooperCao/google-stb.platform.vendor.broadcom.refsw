/***************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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


BDBG_MODULE(playpump_descriptor_options);

static void play_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void event_callback(void *context, int param)
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
    printf("  RAVE %u, data %u bytes, index %u bytes.\r", status.rave.index, (unsigned)status.data.bytesRecorded, (unsigned)status.index.bytesRecorded);
}

/*
Summary:
Returns bits 'e'..'b' from word 'w',

Example:
    B_GET_BITS(0xDE,7,4)==0x0D
    B_GET_BITS(0xDE,3,0)=0x0E
*/
#define B_GET_BITS(w,e,b)  (((w)>>(b))&(((unsigned)(-1))>>((sizeof(unsigned))*8-(e+1-b))))

#define BTP_MODE_LAST                       0x82    /* Indicates last packet. */
#define TS_PKT_SIZE  (188)

static FILE *inputFile;
static BKNI_EventHandle playEvent;
static BKNI_EventHandle playDone;
static NEXUS_PlaypumpHandle playpump;
static NEXUS_ThreadHandle playThread;
static NEXUS_ThreadHandle playThread2;
static uint8_t lastCmdBtp[TS_PKT_SIZE];

#define BLOCK_SIZE	(TS_PKT_SIZE * 100)


typedef struct btpPkt
{
    uint32_t data[10];
} btpPkt;

static void buildBtpPacket(
    unsigned pid,
    const btpPkt *btp,
    unsigned timestampOffset,
    uint8_t *pkt
    )
{
    unsigned i;

    BKNI_Memset(pkt, 0, TS_PKT_SIZE+timestampOffset);
    pkt += timestampOffset;
    pkt[0] = 0x47;                      /* SYNC Byte */
    pkt[1] = B_GET_BITS(pid, 12, 8);    /* Upper 5 bits of videoPid */
    pkt[2] = B_GET_BITS(pid, 7, 0);     /* lower 8 bits of videoPid */
    pkt[3] = 0x20;                      /* Transport scrambling control, adaptation field control, continuity counter. */
    pkt[4] = 183;                       /* Adaptation field length is 183. This is the remainder of the packet. */
    pkt[5] = 0x02;                      /* Discard tailend of the packet. */
    pkt[6] = 45;                        /* Transport private data length (in decimal this is 45 bytes. 1 for the align byte, 4 for the signature, 40 for the control words). */
    pkt[7] = 0;                         /* Align Byte. */
    pkt[8 ] = 'B';                      /* BRCM Signature. */
    pkt[9 ] = 'R';
    pkt[10] = 'C';
    pkt[11] = 'M';

    for( i=0; i<sizeof(btp->data)/sizeof(btp->data[0]); i++)
    {
        pkt[12+4*i+0] = B_GET_BITS(btp->data[i], 31, 24);
        pkt[12+4*i+1] = B_GET_BITS(btp->data[i], 23, 16);
        pkt[12+4*i+2] = B_GET_BITS(btp->data[i], 15, 8);
        pkt[12+4*i+3] = B_GET_BITS(btp->data[i], 7, 0);
    }

}

static void buildLastBtp(
    unsigned pid,
    uint8_t *pkt
    )
{
    btpPkt btp;

    static uint32_t count = 1;

    BKNI_Memset(&btp, 0, sizeof(btp));

    btp.data[0] = BTP_MODE_LAST;
    btp.data[1] = count++;
    buildBtpPacket( pid, &btp, 0, pkt);
}

#define PID 0x11

static void playpumpThread(
	void *parm
	)
{
    void *buffer;
    int i = 10;
	size_t buffer_size;
	NEXUS_Error rc;
	NEXUS_PlaypumpWriteCompleteSettings settings;

    BSTD_UNUSED(parm);

    buildLastBtp(PID, lastCmdBtp);
	NEXUS_Playpump_GetDefaultWriteCompleteSettings(&settings);
	while(i--)
	{
        int n;
        rc = NEXUS_Playpump_GetBuffer(playpump, &buffer, &buffer_size);
        if(rc!=NEXUS_SUCCESS) {BDBG_ERR(("Get playpump buffer")); goto done;}
        if(buffer_size>0) {
            if(buffer_size > BLOCK_SIZE) buffer_size = BLOCK_SIZE;
            n = fread(buffer, 1, buffer_size, inputFile);
            if(n<=0) {BDBG_ERR(("fread failed")); goto done;}
            rc = NEXUS_Playpump_WriteCompleteWithSettings(playpump, 0, n, &settings);
            if(rc!=NEXUS_SUCCESS) {BDBG_ERR(("WriteComplete")); goto done;}
        } else {
            BKNI_WaitForEvent(playEvent, BKNI_INFINITE);
        }
	}

    /* Send the LAST_CMD BTP */
	NEXUS_Playpump_GetDefaultWriteCompleteSettings(&settings);
    while(1) {
        rc = NEXUS_Playpump_GetBuffer(playpump, &buffer, &buffer_size);
        if(rc!=NEXUS_SUCCESS) {BDBG_ERR(("Get playpump buffer for BTP")); goto done;}
        if(buffer_size >= TS_PKT_SIZE) {
            BKNI_Memcpy(buffer, lastCmdBtp, TS_PKT_SIZE);
            settings.descriptorOptions.transportOverride = true;
            rc = NEXUS_Playpump_WriteCompleteWithSettings(playpump, 0, TS_PKT_SIZE, &settings);
            if(rc!=NEXUS_SUCCESS) {BDBG_ERR(("WriteComplete for BTP"));}
            break;
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
    BKNI_EventHandle lastCmdRecvd;

	unsigned total = 0;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    /*inputFile = fopen("/root/Security/scratch/gf956276/video_only.ts", "rb");*/
    inputFile = fopen("cnnticker.video.es", "rb");
	BDBG_ASSERT(inputFile);

    /*dataFile = fopen("/root/sandbox/logs/stream.mpg", "wb");*/
	dataFile = fopen("stream.mpg", "wb");
	BDBG_ASSERT(dataFile);
    /*indexFile = fopen("/root/sandbox/logs/stream.idx", "wb");*/
	indexFile = fopen("stream.idx", "wb");
	BDBG_ASSERT(indexFile);

	NEXUS_Playpump_GetDefaultOpenSettings(&pumpOpenSettings);
	pumpOpenSettings.descriptorOptionsEnabled = true;
    playpump = NEXUS_Playpump_Open(0, &pumpOpenSettings);
	BDBG_ASSERT(playpump);

    BKNI_CreateEvent(&playEvent);
    BKNI_CreateEvent(&playDone);
    NEXUS_Playpump_GetSettings(playpump, &playpumpSettings);
    playpumpSettings.dataCallback.callback = play_callback;
    playpumpSettings.dataCallback.context = playEvent;
    playpumpSettings.transportType = NEXUS_TransportType_eEs;
    NEXUS_Playpump_SetSettings(playpump, &playpumpSettings);

    NEXUS_Playpump_GetDefaultOpenPidChannelSettings(&playpumpPidChannelSettings);
    /* Since we're playing back ES, the PID isn't used. */
    videoPidChannel = NEXUS_Playpump_OpenPidChannel(playpump, 1, &playpumpPidChannelSettings);

	NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpen);
	recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpen);
	BDBG_ASSERT(recpump);

    BKNI_CreateEvent(&recEvent);
    BKNI_CreateEvent(&lastCmdRecvd);
    NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
    recpumpSettings.data.dataReady.callback = event_callback;
    recpumpSettings.data.dataReady.context = recEvent;
    recpumpSettings.index.dataReady.callback = event_callback; /* same callback */
    recpumpSettings.index.dataReady.context = recEvent; /* same event */
    recpumpSettings.data.overflow.callback = overflow_callback;
    recpumpSettings.data.overflow.context = "data";
    recpumpSettings.index.overflow.callback = overflow_callback;
    recpumpSettings.index.overflow.context = "index";
    recpumpSettings.lastCmd.callback = event_callback;
    recpumpSettings.lastCmd.context = lastCmdRecvd;
    recpumpSettings.dropBtpPackets = false;
    rc = NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&addPidChannelSettings);
    addPidChannelSettings.pidType = NEXUS_PidType_eVideo;
    addPidChannelSettings.pidTypeSettings.video.index = true;
    /*addPidChannelSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eH264;*/
    addPidChannelSettings.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
    rc = NEXUS_Recpump_AddPidChannel(recpump, videoPidChannel, &addPidChannelSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Recpump_Start(recpump);
    BDBG_ASSERT(!rc);
    rc = NEXUS_Playpump_Start(playpump);
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
            BKNI_WaitForEvent(recEvent, 50);
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
        print_recpump_status(recpump);
    }
    printf("\n\n");

    printf("Waiting for first LAST_CMD\n");
    BKNI_WaitForEvent(lastCmdRecvd, BKNI_INFINITE);
    printf("Saw LAST_CMD\n");

    playThread2 = NEXUS_Thread_Create("playpumpThread2", playpumpThread, NULL, &threadSettings);

    while (BKNI_WaitForEvent(playDone, 0) != BERR_SUCCESS) {
        const void *data_buffer, *index_buffer;
        size_t data_buffer_size, index_buffer_size;
        int n;

        rc = NEXUS_Recpump_GetDataBuffer(recpump, &data_buffer, &data_buffer_size);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Recpump_GetIndexBuffer(recpump, &index_buffer, &index_buffer_size);
        BDBG_ASSERT(!rc);
        if (data_buffer_size == 0 && index_buffer_size == 0) {
            BKNI_WaitForEvent(recEvent, 50);
            continue;
        }

        if (data_buffer_size) {
            n = fwrite(data_buffer, 1, data_buffer_size, dataFile);
            if (n < 0) {BDBG_ERR(("fwrite error: %d\n", errno)); break;}
            rc = NEXUS_Recpump_DataReadComplete(recpump, n);
            BDBG_ASSERT(!rc);
            total += n;
        }
        fflush(dataFile);
        if (index_buffer_size) {
            n = fwrite(index_buffer, 1, index_buffer_size, indexFile);
            if (n < 0) {BDBG_ERR(("fwrite error: %d\n", errno)); break;}
            rc = NEXUS_Recpump_IndexReadComplete(recpump, n);
            BDBG_ASSERT(!rc);
        }
        fflush(indexFile);
        print_recpump_status(recpump);
    }
    printf("\n\n");

    printf("Waiting for second LAST_CMD\n");
    BKNI_WaitForEvent(lastCmdRecvd, BKNI_INFINITE);
    printf("Saw second LAST_CMD\n");

	NEXUS_Playpump_Stop(playpump);
	NEXUS_Playpump_Close(playpump);
	NEXUS_Recpump_Stop(recpump);
	NEXUS_Recpump_Close(recpump);
	NEXUS_Thread_Destroy(playThread);
    NEXUS_Thread_Destroy(playThread2);
	BKNI_DestroyEvent(playEvent);
	BKNI_DestroyEvent(recEvent);
	BKNI_DestroyEvent(playDone);
    BKNI_DestroyEvent( lastCmdRecvd);
	fclose(dataFile);
	fclose(indexFile);
	fclose(inputFile);
    NEXUS_Platform_Uninit();
	return 0;
}
