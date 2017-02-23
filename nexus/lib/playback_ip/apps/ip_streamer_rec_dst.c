/******************************************************************************
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
 *
 * Module Description: 
 *  rec destination => record to local disk
 *
 ******************************************************************************/
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>

#include "bstd.h"
#include "bkni.h"
#include "blst_list.h"
#include "blst_queue.h"

#ifndef DMS_CROSS_PLATFORMS                                                                                              
#include "nexus_platform.h"                                                                                              
#if NEXUS_HAS_FRONTEND                                                                                                   
#include "nexus_frontend.h"                                                                                              
#endif                                                                                                                   
#include "nexus_parser_band.h"                                                                                           
#include "nexus_pid_channel.h"                                                                                           
#include "nexus_playpump.h"                                                                                              
#include "nexus_message.h"                                                                                               
#include "nexus_timebase.h"                                                                                              
#include "nexus_recpump.h"                                                                                               
#ifdef NEXUS_HAS_RECORD                                                                                                  
#include "nexus_record.h"                                                                                                
#endif                                                                                                                   
#include "nexus_file_fifo.h"                                                                                             
#include "nexus_core_utils.h"                                                                                            
#endif /* DMS_CROSS_PLATFORMS */                                                                                         
#include "b_playback_ip_lib.h"                                                                                           

#ifdef B_HAS_DTCP_IP
#include "b_dtcp_applib.h"
#include "b_dtcp_constants.h"
#endif

#include "nexus_core_utils.h"
#include "ip_streamer_lib.h"
#include "ip_streamer.h"
#include "ip_http.h"
#include "b_os_lib.h"

BDBG_MODULE(ip_streamer);
#if 0
#define BDBG_MSG_FLOW(x)  BDBG_WRN( x );
#else
#define BDBG_MSG_FLOW(x)  
#endif

#ifdef NEXUS_HAS_RECORD
/* change to buildRecDstList */
int
initNexusRecDstList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    RecDst *recDst;

    if (BKNI_CreateMutex(&ipStreamerGlobalCtx->recDstMutex) != 0) {
        BDBG_ERR(("Failed to create BKNI mutex at %d", __LINE__));
        return -1;
    }
    for (i = 0; i < IP_STREAMER_NUM_RECPUMPS_FOR_RECORD; i++) {
        recDst = &ipStreamerGlobalCtx->recDstList[i];
        memset(recDst, 0, sizeof(RecDst));
        /* coverity[missing_lock] */
        recDst->inUse = false;
    }

    BDBG_MSG(("%d Record Destinations are initialized ", i));
    return 0;
}

void
unInitNexusRecDstList(
    IpStreamerGlobalCtx *ipStreamerGlobalCtx
    )
{
    int i;
    RecDst *recDst;

    for (i = 0; i < IP_STREAMER_NUM_RECPUMPS_FOR_RECORD; i++) {
        recDst = &ipStreamerGlobalCtx->recDstList[i];
    }
    if (ipStreamerGlobalCtx->recDstMutex)
        BKNI_DestroyMutex(ipStreamerGlobalCtx->recDstMutex);
    BDBG_MSG(("%d Record Destinations are uninitialized ", i));
}

/* forward declaration */
int startNexusRecDst(IpStreamerCtx *ipStreamerCtx);
void stopNexusRecDst(IpStreamerCtx *ipStreamerCtx);

static void
errorCallbackDst(
    void *context,
    int param
    )
{
    IpStreamerCtx *ipStreamerCtx = (IpStreamerCtx *)context;
    BDBG_WRN(("CTX %p: Got error during network streaming/recording: stopping ip/rec dst %d", (void *)ipStreamerCtx, param));
    if (ipStreamerCtx->recDst)
        stopNexusRecDst(ipStreamerCtx);
}

int
openNexusRecDst(
    B_PlaybackIpPsiInfo *psi,
    IpStreamerConfig *ipStreamerCfg,
    IpStreamerCtx *ipStreamerCtx
    )
{
    int i;
    NEXUS_Error rc = NEXUS_UNKNOWN;
    RecDst *recDst;
    NEXUS_RecordSettings recordSettings;
    NEXUS_RecordPidChannelSettings pidSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    int factor = 2;

    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->recDstMutex);
    recDst = NULL;
    for (i = 0; i < IP_STREAMER_NUM_RECPUMPS_FOR_RECORD; i++) {
        if (!ipStreamerCtx->globalCtx->recDstList[i].inUse) {
            recDst = &ipStreamerCtx->globalCtx->recDstList[i];
            BDBG_MSG(("Found Free Record Dst entry %p", (void *)recDst));
            ipStreamerCtx->recDst = recDst;
            recDst->inUse = true;
            break;
        }
    }
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->recDstMutex);
    if (!recDst) {
        BDBG_ERR(("ERROR: No Free Record Dst entry"));
        return -1;
    }

    /* recording logic */
    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    BDBG_MSG(("recpump (# %d): data size %d, threshold %d, atom size %d, index size %d",
                i, factor*recpumpOpenSettings.data.bufferSize, recpumpOpenSettings.data.dataReadyThreshold, recpumpOpenSettings.data.atomSize, factor*recpumpOpenSettings.index.bufferSize));
    /* TODO: need to determine the actual size needed */
    recpumpOpenSettings.data.bufferSize = factor * recpumpOpenSettings.data.bufferSize;
    recpumpOpenSettings.index.bufferSize = factor * recpumpOpenSettings.index.bufferSize;
    /* we start from the other half of the remaining destinations (rave ctx) */
    recDst->recpumpHandle = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
    if (!recDst->recpumpHandle) {
        BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
        return -1;
    }

    recDst->recordHandle = NEXUS_Record_Create();
    if (!recDst->recordHandle) {
        BDBG_ERR(("NEXUS Error at %d, failed to create record handle, returning..", __LINE__));
        return -1;
    }

    NEXUS_Record_GetSettings(recDst->recordHandle, &recordSettings);
    recordSettings.recpump = recDst->recpumpHandle;
    recordSettings.errorCallback.callback = errorCallbackDst;
    recordSettings.errorCallback.param = 0; /* used in the callback to determine streaming or recording context */
    recordSettings.errorCallback.context = ipStreamerCtx;
    NEXUS_Record_SetSettings(recDst->recordHandle, &recordSettings);

    ipStreamerCtx->recDst->fileRecordHandle = NEXUS_FileRecord_OpenPosix(ipStreamerCfg->fileName, ipStreamerCfg->indexFileName);
    if (ipStreamerCtx->recDst->fileRecordHandle == NULL) {
        BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
        return -1;
    }

    /* configure the video pid for indexing */
    NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
    pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
    pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
    pidSettings.recpumpSettings.pidTypeSettings.video.codec = psi->videoCodec;
    if (ipStreamerCtx->videoPidChannel) {
        rc = NEXUS_Record_AddPidChannel(ipStreamerCtx->recDst->recordHandle, ipStreamerCtx->videoPidChannel, &pidSettings);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
    }

    if (ipStreamerCtx->audioPidChannel) {
        rc = NEXUS_Record_AddPidChannel(ipStreamerCtx->recDst->recordHandle, ipStreamerCtx->audioPidChannel, NULL);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
    }

    if (ipStreamerCtx->pcrPidChannel) {
        rc = NEXUS_Record_AddPidChannel(ipStreamerCtx->recDst->recordHandle, ipStreamerCtx->pcrPidChannel, NULL);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
    }

#ifdef STREAMER_CABLECARD_SUPPORT
#ifdef MPODXSIX
    if (ipStreamerCtx->caPidChannel) {
        rc = NEXUS_Record_AddPidChannel(ipStreamerCtx->recDst->recordHandle, ipStreamerCtx->caPidChannel, NULL);
        if (rc) {
            BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
            return -1;
        }
    }
#endif
#endif

    rc = NEXUS_Record_AddPidChannel(ipStreamerCtx->recDst->recordHandle, ipStreamerCtx->patPidChannel, NULL);
    if (rc) {
        BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
        return -1;
    }

    rc = NEXUS_Record_AddPidChannel(ipStreamerCtx->recDst->recordHandle, ipStreamerCtx->pmtPidChannel, NULL);
    if (rc) {
        BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
        return -1;
    }

    BDBG_MSG(("CTX %p: Record Dst %p (file %s) is opened", (void *)ipStreamerCtx, (void *)recDst, ipStreamerCfg->fileName));
    return 0;
}

void
closeNexusRecDst(
    IpStreamerCtx *ipStreamerCtx
    )
{
    if (!ipStreamerCtx->recDst)
        return;
    BKNI_AcquireMutex(ipStreamerCtx->globalCtx->recDstMutex);
    ipStreamerCtx->recDst->inUse = false;
    BKNI_ReleaseMutex(ipStreamerCtx->globalCtx->recDstMutex);
    NEXUS_Record_RemoveAllPidChannels(ipStreamerCtx->recDst->recordHandle);
    if (ipStreamerCtx->recDst->fileRecordHandle != NULL) {
        NEXUS_FileRecord_Close(ipStreamerCtx->recDst->fileRecordHandle);
    }
    if (ipStreamerCtx->recDst->recordHandle != NULL)
        NEXUS_Record_Destroy(ipStreamerCtx->recDst->recordHandle);
    if (ipStreamerCtx->recDst->recpumpHandle)
        NEXUS_Recpump_Close(ipStreamerCtx->recDst->recpumpHandle);
    BDBG_MSG(("CTX %p: Rec Dst %p is closed", (void *)ipStreamerCtx, (void *)ipStreamerCtx->recDst));
}

int
startNexusRecDst(
    IpStreamerCtx *ipStreamerCtx
    )
{
    NEXUS_Error rc = NEXUS_UNKNOWN;
    rc = NEXUS_Record_Start(ipStreamerCtx->recDst->recordHandle, ipStreamerCtx->recDst->fileRecordHandle);
    if (rc) {
        BDBG_ERR(("NEXUS Error at %d, returning..", __LINE__));
        return -1;
    }
    ipStreamerCtx->localRecordingInProgress = true;
    BDBG_MSG(("CTX %p: Record Dst %p is started", (void *)ipStreamerCtx, (void *)ipStreamerCtx->recDst));
    return 0;
}

void
stopNexusRecDst(
    IpStreamerCtx *ipStreamerCtx
    )
{
    if (ipStreamerCtx->localRecordingInProgress) {
        NEXUS_Record_Stop(ipStreamerCtx->recDst->recordHandle);
        ipStreamerCtx->localRecordingInProgress = false;
        BDBG_MSG(("CTX %p: Rec dst %p is stopped", (void *)ipStreamerCtx, (void *)ipStreamerCtx->recDst));
    }
}

#else /* !NEXUS_HAS_RECORD */
int initNexusRecDstList(IpStreamerGlobalCtx *ipStreamerGlobalCtx)
{
    BSTD_UNUSED(ipStreamerGlobalCtx);

    BDBG_ERR(("%s: IP Record can't be enabled on platforms w/o NEXUS_HAS_RECORD support", __FUNCTION__));
    /* note: we let the init call succeed, but we will fail the open call */
    /* this way app wont fail during startup, but will fail a client request to open/start a recording */
    return 0; 
}
void unInitNexusRecDstList(IpStreamerGlobalCtx *ipStreamerGlobalCtx)
{
    BSTD_UNUSED(ipStreamerGlobalCtx);

    BDBG_ERR(("%s: IP Record Can't be enabled on platforms w/o NEXUS_HAS_RECORD support", __FUNCTION__));
}
int openNexusRecDst(B_PlaybackIpPsiInfo *psi, IpStreamerConfig *ipStreamerCfg, IpStreamerCtx *ipStreamerCtx);
{
    BSTD_UNUSED(psi);
    BSTD_UNUSED(ipStreamerCfg);
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: IP Record Can't be enabled on platforms w/o NEXUS_HAS_RECORD support", __FUNCTION__));
    return -1; 
}
void closeNexusRecDst(IpStreamerCtx *ipStreamerCtx);
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: IP Record Can't be enabled on platforms w/o NEXUS_HAS_RECORD support", __FUNCTION__));
}
int setupAndAcquirePsiInfoRecDst(IpStreamerConfig *ipStreamerCfg, IpStreamerCtx *ipStreamerCtx, B_PlaybackIpPsiInfo *psiOut);
{
    BSTD_UNUSED(psiOut);
    BSTD_UNUSED(ipStreamerCfg);
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: IP Record Can't be enabled on platforms w/o NEXUS_HAS_RECORD support", __FUNCTION__));
    return -1; 
}
int startNexusRecDst(IpStreamerCtx *ipStreamerCtx);
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: IP Record Can't be enabled on platforms w/o NEXUS_HAS_RECORD support", __FUNCTION__));
    return -1; 
}
int stopNexusRecDst(IpStreamerCtx *ipStreamerCtx);
{
    BSTD_UNUSED(ipStreamerCtx);

    BDBG_ERR(("%s: IP Record Can't be enabled on platforms w/o NEXUS_HAS_RECORD support", __FUNCTION__));
    return -1; 
}
#endif
