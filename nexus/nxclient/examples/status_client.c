/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#include "nxclient.h"
#include "nexus_platform.h"
#if NEXUS_HAS_TRANSPORT
#include "nexus_playpump.h"
#endif
#if NEXUS_HAS_AUDIO
#include "nexus_audio_decoder.h"
#endif
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(status_client);

#if NEXUS_HAS_TRANSPORT
static void print_playpump(NEXUS_PlaypumpHandle playpump)
{
    NEXUS_PlaypumpStatus status;
    int rc;
    rc = NEXUS_Playpump_GetStatus(playpump, &status);
    if (rc) {BERR_TRACE(rc); return;}

    printf(
        "playpump%u: %s\n"
        "fifo %u/%u (%u%%)\n"
        "descfifo %u/%u (%u%%)\n"
        "KB played %u\n",
        status.index,status.started?"started":"stopped",
        (unsigned)status.fifoDepth, (unsigned)status.fifoSize, status.fifoSize?(unsigned)(status.fifoDepth*100/status.fifoSize):0,
        (unsigned)status.descFifoDepth, (unsigned)status.descFifoSize, status.descFifoSize?(unsigned)(status.descFifoDepth*100/status.descFifoSize):0,
        (unsigned)(status.bytesPlayed/1024));
    printf("\n");
}
#endif

#if NEXUS_HAS_AUDIO
static void print_audio_decoder(NEXUS_AudioDecoderHandle audioDecoder)
{
    NEXUS_AudioDecoderStatus status;
    int rc;
    rc = NEXUS_AudioDecoder_GetStatus(audioDecoder, &status);
    if (rc) {BERR_TRACE(rc); return;}

    printf(
        "audio_decoder: %s %s\n"
        "fifo %u/%u (%u%%)\n"
        "PTS %#x, PTS/STC diff %d\n",
        status.started?"started":"stopped",status.tsm?"tsm":"vsync",
        status.fifoDepth,status.fifoSize,status.fifoSize?status.fifoDepth*100/status.fifoSize:0,
        status.pts, status.ptsStcDifference);
    printf("\n");
}
#endif

#if NEXUS_HAS_VIDEO_DECODER
static void print_video_decoder(NEXUS_VideoDecoderHandle videoDecoder)
{
    NEXUS_VideoDecoderStatus status;
    int rc;
    rc = NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
    if (rc) {BERR_TRACE(rc); return;}

    printf(
        "video_decoder: %s %s\n"
        "fifo %u/%u (%u%%)\n"
        "PTS %#x, PTS/STC diff %d\n",
        status.started?"started":"stopped",status.tsm?"tsm":"vsync",
        status.fifoDepth,status.fifoSize,status.fifoSize?status.fifoDepth*100/status.fifoSize:0,
        status.pts, status.ptsStcDifference);
    printf("\n");
}
#endif

int main(void)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc;
#define MAX_OBJECTS 16
    NEXUS_InterfaceName interfaceName;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
    size_t num;
    unsigned i;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    joinSettings.mode = NEXUS_ClientMode_eVerified;
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    /* print status for every Playpump, AudioDecoder and VideoDecoder handle */

#if NEXUS_HAS_TRANSPORT
    NEXUS_Platform_GetDefaultInterfaceName(&interfaceName);
    strcpy(interfaceName.name, "NEXUS_Playpump");
    rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);
    BDBG_ASSERT(!rc);
    for (i=0;i<num;i++) {
        print_playpump(objects[i].object);
    }
#endif

#if NEXUS_HAS_AUDIO
    NEXUS_Platform_GetDefaultInterfaceName(&interfaceName);
    strcpy(interfaceName.name, "NEXUS_AudioDecoder");
    rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);
    BDBG_ASSERT(!rc);
    for (i=0;i<num;i++) {
        print_audio_decoder(objects[i].object);
    }
#endif

#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_Platform_GetDefaultInterfaceName(&interfaceName);
    strcpy(interfaceName.name, "NEXUS_VideoDecoder");
    rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);
    BDBG_ASSERT(!rc);
    for (i=0;i<num;i++) {
        print_video_decoder(objects[i].object);
    }
#endif

    NxClient_Uninit();
    return 0;
}
