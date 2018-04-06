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
 *****************************************************************************/
#include "nexus_platform_client.h"
#include "nxclient.h"
#include "platform.h"
#include "platform_priv.h"
#include "platform_scheduler_priv.h"
#include "bdbg.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <string.h>

BDBG_MODULE(platform_scheduler);

void platform_scheduler_p_get_default_create_settings(PlatformSchedulerCreateSettings * pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->period = 100;
}

PlatformSchedulerHandle platform_scheduler_p_create(PlatformHandle platform, const PlatformSchedulerCreateSettings * pSettings)
{
    PlatformSchedulerHandle scheduler;
    scheduler = BKNI_Malloc(sizeof(*scheduler));
    BDBG_ASSERT(scheduler);
    BKNI_Memset(scheduler, 0, sizeof(*scheduler));
    if (pSettings)
    {
        BKNI_Memcpy(&scheduler->createSettings, pSettings, sizeof(*pSettings));
    }
    else
    {
        platform_scheduler_p_get_default_create_settings(&scheduler->createSettings);
    }

    BKNI_CreateMutex(&scheduler->mutex);

    BKNI_AcquireMutex(scheduler->mutex);
    BLST_Q_INIT(&scheduler->listeners);
    BLST_Q_INIT(&scheduler->free);
    scheduler->platform = platform;
    BKNI_ReleaseMutex(scheduler->mutex);

    BKNI_CreateEvent(&scheduler->wake);

    return scheduler;
}

void platform_scheduler_p_destroy(PlatformSchedulerHandle scheduler)
{
    PlatformListenerHandle l;

    if (!scheduler) return;
    if (scheduler->thread)
    {
        scheduler->state = PlatformSchedulerState_eDone;
        BKNI_SetEvent(scheduler->wake);
        pthread_join(scheduler->thread, NULL);
    }

    for (l = BLST_Q_FIRST(&scheduler->free); l; l = BLST_Q_FIRST(&scheduler->free))
    {
        BLST_Q_REMOVE_HEAD(&scheduler->free, link);
        BKNI_Free(l);
    }

    BKNI_DestroyEvent(scheduler->wake);
    scheduler->platform->schedulers[scheduler->createSettings.index] = NULL;
    BKNI_DestroyMutex(scheduler->mutex);
    BKNI_Free(scheduler);
}

PlatformListenerHandle platform_scheduler_add_listener(PlatformSchedulerHandle scheduler, PlatformCallback callback, void * pCallbackContext)
{
    PlatformListenerHandle listener;
    BDBG_ASSERT(scheduler);
    BDBG_ASSERT(callback);
    listener = BKNI_Malloc(sizeof(*listener));
    BDBG_ASSERT(listener);
    memset(listener, 0, sizeof(*listener));
    listener->callback = callback;
    listener->pContext = pCallbackContext;
    listener->deleted = false;
    BKNI_AcquireMutex(scheduler->mutex);
    BLST_Q_INSERT_TAIL(&scheduler->listeners, listener, link);
    BKNI_ReleaseMutex(scheduler->mutex);
    return listener;
}

void platform_scheduler_remove_listener(PlatformSchedulerHandle scheduler, PlatformListenerHandle listener)
{
    BDBG_ASSERT(scheduler);
    if (listener)
    {
        listener->deleted = true;
        BKNI_AcquireMutex(scheduler->mutex);
        BLST_Q_REMOVE(&scheduler->listeners, listener, link);
        BLST_Q_INSERT_TAIL(&scheduler->free, listener, link);
        BKNI_ReleaseMutex(scheduler->mutex);
    }
}

void platform_scheduler_wake(PlatformSchedulerHandle scheduler)
{
    BDBG_ASSERT(scheduler);
    BKNI_SetEvent(scheduler->wake);
}

void platform_scheduler_start(PlatformSchedulerHandle scheduler)
{
    BDBG_ASSERT(scheduler);

    if (scheduler->state != PlatformSchedulerState_eInit) return;

    scheduler->state = PlatformSchedulerState_eRunning;

    if (pthread_create(&scheduler->thread, NULL, &platform_scheduler_p_thread, scheduler))
    {
        BDBG_WRN(("Unable to create platform scheduler thread"));
        BDBG_ASSERT(0);
    }
}

void platform_scheduler_stop(PlatformSchedulerHandle scheduler)
{
    scheduler->state = PlatformSchedulerState_eDone;
    if (scheduler->thread) pthread_join(scheduler->thread, NULL);
    scheduler->thread = 0;
    scheduler->state = PlatformSchedulerState_eInit;
}

void * platform_scheduler_p_thread(void * context)
{
    PlatformSchedulerHandle scheduler = context;
    PlatformListenerHandle l;
    BERR_Code rc;

    BDBG_ASSERT(scheduler);

    while (scheduler->state == PlatformSchedulerState_eRunning)
    {
        BKNI_AcquireMutex(scheduler->mutex);
        for (l = BLST_Q_FIRST(&scheduler->free); l; l = BLST_Q_FIRST(&scheduler->free))
        {
            BLST_Q_REMOVE_HEAD(&scheduler->free, link);
            BKNI_Free(l);
        }
        for (l = BLST_Q_FIRST(&scheduler->listeners); l; l = BLST_Q_NEXT(l, link))
        {
            if (scheduler->state == PlatformSchedulerState_eDone) break;
            if (l->callback)
            {
                BKNI_ReleaseMutex(scheduler->mutex);
                if (!l->deleted)
                {
                    l->callback(l->pContext, 0);
                }
                BKNI_AcquireMutex(scheduler->mutex);
            }
        }
        BKNI_ReleaseMutex(scheduler->mutex);

        if (scheduler->state == PlatformSchedulerState_eDone) break;

        if (scheduler->wake)
        {
            rc = BKNI_WaitForEvent(scheduler->wake, scheduler->createSettings.period);
            if (rc == BERR_OS_ERROR)
            {
                BDBG_ERR(("Error waiting for scheduler wake event"));
                break;
            }
        }
    }

    return NULL;
}
