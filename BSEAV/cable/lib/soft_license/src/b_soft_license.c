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

#include "nexus_platform.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#endif
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#endif
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bstd_defs.h"
#include "bstd.h"
#include "b_os_lib.h"
#include "b_os_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "b_soft_license.h"

BDBG_MODULE(b_soft_license);
BDBG_OBJECT_ID(B_SoftLicense);
#define DEFAULT_POLLING_TIME 1000
struct B_SoftLicense
{
    BDBG_OBJECT(B_SoftLicense)
    B_SchedulerHandle scheduler;
    B_MutexHandle schedulerMutex;
    B_MutexHandle mutex;
    B_ThreadHandle thread;
    B_SoftLicense_Status status;
    B_SoftLicense_OpenSettings openSettings;
    B_SchedulerTimerId pollingTimerId;
};

static B_SoftLicenseHandle g_SoftLicense;


static void B_SoftLicense_P_Scheduler(void * param)
{
    B_SoftLicenseHandle hSoftLicense = (B_SoftLicenseHandle)param;
    BDBG_OBJECT_ASSERT(hSoftLicense,B_SoftLicense);
    B_Scheduler_Run(hSoftLicense->scheduler);
    return;
}

static void B_SoftLicense_P_Polling(void *param)
{
    B_SoftLicenseHandle hSoftLicense = (B_SoftLicenseHandle)param;
    B_SoftLicense_Status status;
#if NEXUS_HAS_FRONTEND
    NEXUS_FrontendModuleStatistics frontendStats;
#endif
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoDecoderModuleStatistics videoDecoderStats;
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    NEXUS_VideoEncoderModuleStatistics videoEncoderStats;
#endif

    BDBG_OBJECT_ASSERT(hSoftLicense,B_SoftLicense);
#if NEXUS_HAS_FRONTEND
    NEXUS_FrontendModule_GetStatistics(&frontendStats);
    status.numTuners = frontendStats.maxTunedFrontends;
#endif

#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoDecoderModule_GetStatistics(&videoDecoderStats);
    status.ultraHdDecode = (videoDecoderStats.maxDecodedHeight >= 2160 && videoDecoderStats.maxDecodedWidth >= 3840)?true:false;
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    NEXUS_VideoEncoderModule_GetStatistics(&videoEncoderStats);
    status.transcode = videoEncoderStats.numStarts > 0? true:false;
#endif

    if ( (status.transcode != hSoftLicense->status.transcode && status.transcode) ||
         (status.ultraHdDecode != hSoftLicense->status.ultraHdDecode && status.ultraHdDecode) ||
         (status.numTuners != hSoftLicense->status.numTuners && status.numTuners >= 8)
       )
    {
        B_Mutex_Lock(g_SoftLicense->mutex);
        g_SoftLicense->status.numTuners = status.numTuners;
        g_SoftLicense->status.transcode = status.transcode;
        g_SoftLicense->status.ultraHdDecode = status.ultraHdDecode;
        B_Mutex_Unlock(g_SoftLicense->mutex);
        hSoftLicense->openSettings.registeredCallback(hSoftLicense->openSettings.appContext);
    }

    B_Mutex_Lock(g_SoftLicense->mutex);
    g_SoftLicense->pollingTimerId = B_Scheduler_StartTimer(g_SoftLicense->scheduler,
                                                           g_SoftLicense->schedulerMutex,
                                                           g_SoftLicense->openSettings.pollingPeriod,
                                                           B_SoftLicense_P_Polling,
                                                           (void*)g_SoftLicense);
    B_Mutex_Unlock(g_SoftLicense->mutex);

    return;
}
void B_SoftLicense_GetDefaultOpenSettings(B_SoftLicense_OpenSettings *pOpenSettings)
{
    BKNI_Memset(pOpenSettings,0,sizeof(*pOpenSettings));
    pOpenSettings->pollingPeriod = DEFAULT_POLLING_TIME; /* in ms */
    return;
}


B_SoftLicenseHandle B_SoftLicense_Open(B_SoftLicense_OpenSettings *pOpenSettings)
{
    BDBG_ASSERT((pOpenSettings));
    BDBG_ASSERT((pOpenSettings->appContext));
    g_SoftLicense = BKNI_Malloc(sizeof (*g_SoftLicense));
    if(!g_SoftLicense) { return NULL; }
    BKNI_Memset(g_SoftLicense,0,sizeof(*g_SoftLicense));
    BDBG_OBJECT_SET(g_SoftLicense,B_SoftLicense);
    g_SoftLicense->scheduler = B_Scheduler_Create(NULL);
    if (!g_SoftLicense->scheduler) {goto error_open;    }
    g_SoftLicense->thread = B_Thread_Create("soft_license_scheduler", B_SoftLicense_P_Scheduler,g_SoftLicense, NULL);
    if (!g_SoftLicense->thread) {goto error_open;    }

    g_SoftLicense->schedulerMutex = B_Mutex_Create(NULL);
    if( !g_SoftLicense->schedulerMutex) {goto error_open; }

    g_SoftLicense->mutex = B_Mutex_Create(NULL);
    if( !g_SoftLicense->mutex) {goto error_open; }

    BKNI_Memcpy(&g_SoftLicense->openSettings,pOpenSettings,sizeof(*pOpenSettings));

    if (!g_SoftLicense->openSettings.pollingPeriod)
    {
        g_SoftLicense->openSettings.pollingPeriod = DEFAULT_POLLING_TIME;
    }

    g_SoftLicense->pollingTimerId = B_Scheduler_StartTimer(g_SoftLicense->scheduler,
                                                           g_SoftLicense->schedulerMutex,
                                                           g_SoftLicense->openSettings.pollingPeriod,
                                                           B_SoftLicense_P_Polling,
                                                           (void*)g_SoftLicense);
    return g_SoftLicense;
error_open:
    if (g_SoftLicense->pollingTimerId) { B_Scheduler_CancelTimer(g_SoftLicense->scheduler,g_SoftLicense->pollingTimerId); }
    if (g_SoftLicense->schedulerMutex) { B_Mutex_Destroy(g_SoftLicense->schedulerMutex); }
    if (g_SoftLicense->mutex) { B_Mutex_Destroy(g_SoftLicense->mutex); }
    if (g_SoftLicense->scheduler) { B_Scheduler_Stop(g_SoftLicense->scheduler); }
    if (g_SoftLicense->thread) {
        B_Scheduler_Stop(g_SoftLicense->scheduler);
        B_Thread_Destroy(g_SoftLicense->thread);
    }
    if (g_SoftLicense->scheduler) { B_Scheduler_Destroy(g_SoftLicense->scheduler); }
    BDBG_OBJECT_DESTROY(g_SoftLicense,B_SoftLicense);
    BKNI_Free(g_SoftLicense);
    return NULL;
}


NEXUS_Error B_SoftLicense_GetStatus(B_SoftLicenseHandle hSoftLicense,B_SoftLicense_Status *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BDBG_OBJECT_ASSERT(hSoftLicense,B_SoftLicense);
    B_Mutex_Lock(hSoftLicense->mutex);
    pStatus->numTuners = hSoftLicense->status.numTuners;
    pStatus->transcode = hSoftLicense->status.transcode;
    pStatus->ultraHdDecode = hSoftLicense->status.ultraHdDecode;
    B_Mutex_Unlock(hSoftLicense->mutex);
    return rc;
}

NEXUS_Error B_SoftLicense_Close(B_SoftLicenseHandle hSoftLicense)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BDBG_OBJECT_ASSERT(hSoftLicense,B_SoftLicense);
    B_Mutex_Lock(g_SoftLicense->mutex);
    if(hSoftLicense->pollingTimerId)
    {
        B_Scheduler_CancelTimer(hSoftLicense->scheduler,hSoftLicense->pollingTimerId);
    }
    B_Scheduler_Stop(hSoftLicense->scheduler);
    B_Mutex_Unlock(g_SoftLicense->mutex);

    B_Thread_Destroy(hSoftLicense->thread);
    B_Scheduler_Destroy(hSoftLicense->scheduler);
    B_Mutex_Destroy(hSoftLicense->mutex);
    B_Mutex_Destroy(hSoftLicense->schedulerMutex);
    BDBG_OBJECT_DESTROY(hSoftLicense,B_SoftLicense);
    BKNI_Free(hSoftLicense);
    return rc;
}
