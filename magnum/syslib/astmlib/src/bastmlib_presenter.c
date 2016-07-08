/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "bstd.h"
#include "bastmlib_priv.h"
#include "bastmlib_presenter.h"
#include "bastmlib_presenter_priv.h"

BDBG_MODULE(astmlib);

#define BASTMLIB_DEBUG_QUEUE 0

#if BDBG_DEBUG_BUILD
static int BASTMlib_P_Strlen(const char * s)
{
    int len = 0;
    while (*s++) len++;
    return len;
}
#endif

void BASTMlib_Presenter_GetDefaultSettings(
    BASTMlib_Presenter_Settings * psSettings /* [out] */
)
{
    BDBG_ENTER(BASTMlib_Presenter_GetDefaultSettings);

    BDBG_ASSERT(psSettings);

    BKNI_Memset(psSettings, 0, sizeof(BASTMlib_Presenter_Settings));

    BDBG_LEAVE(BASTMlib_Presenter_GetDefaultSettings);
}

void BASTMlib_Presenter_Create(BASTMlib_Handle hAstm, BASTMlib_Presenter_Handle * phPresenter, const BASTMlib_Presenter_Settings * psSettings)
{
    BASTMlib_Presenter_Handle hPresenter;

    BDBG_ENTER(BASTMlib_Presenter_Create);

    hPresenter = (BASTMlib_Presenter_Handle)BKNI_Malloc(sizeof(struct BASTMlib_Presenter_Impl));
    BDBG_ASSERT(hPresenter);
    BKNI_Memset(hPresenter, 0, sizeof(struct BASTMlib_Presenter_Impl));
    hPresenter->uiId = hAstm->uiPresentersCreated++;

    BASTMlib_Presenter_GetDefaultConfig(&hPresenter->sConfig);

    BASTMlib_Presenter_SetSettings(hPresenter, psSettings);

    if (hPresenter->pcName)
    {
        BDBG_MSG(("Presenter %s initial configuration:", hPresenter->pcName));
    }
    else
    {
        BDBG_MSG(("Presenter %d initial configuration:", hPresenter->uiId));
    }
    BDBG_MSG(("  min time between events: %u ms", hPresenter->sConfig.uiMinimumTimeBetweenEvents));
    BDBG_MSG(("  pass event count threshold: %u events", hPresenter->sConfig.uiPassEventCountThreshold));
    BDBG_MSG(("  fail event count threshold: %u events", hPresenter->sConfig.uiFailEventCountThreshold));

    *phPresenter = hPresenter;

    BDBG_LEAVE(BASTMlib_Presenter_Create);
}

void BASTMlib_Presenter_GetSettings(
    const BASTMlib_Presenter_Handle hPresenter,
    BASTMlib_Presenter_Settings * psSettings /* out */
)
{
    BDBG_ENTER(BASTMlib_Presenter_GetSettings);
    BDBG_ASSERT(hPresenter);

    if (psSettings)
    {
        *psSettings = hPresenter->sSettings;
    }

    BDBG_LEAVE(BASTMlib_Presenter_GetSettings);
}

void BASTMlib_Presenter_SetSettings(
    BASTMlib_Presenter_Handle hPresenter,
    const BASTMlib_Presenter_Settings * psSettings
)
{
    BDBG_ENTER(BASTMlib_Presenter_SetSettings);
    BDBG_ASSERT(hPresenter);
    
    if (psSettings)
    {
#if BDBG_DEBUG_BUILD
        int len;
#endif
        hPresenter->sSettings = *psSettings;
#if BDBG_DEBUG_BUILD
        if (psSettings->pcName)
        {
            len = BASTMlib_P_Strlen(psSettings->pcName);
            hPresenter->pcName = (char *)BKNI_Malloc(len + 1);
            BKNI_Memset(hPresenter->pcName, 0, len + 1);
            BKNI_Memcpy(hPresenter->pcName, psSettings->pcName, len);
        }
        else
        {
            if (hPresenter->pcName)
            {
                BKNI_Free(hPresenter->pcName);
                hPresenter->pcName = NULL;
            }
        }
#else
        hPresenter->sSettings.pcName = NULL;
#endif
    }
    
    BDBG_LEAVE(BASTMlib_Presenter_SetSettings);
}

void BASTMlib_Presenter_Destroy(BASTMlib_Presenter_Handle hPresenter)
{
    BDBG_ENTER(BASTMlib_Presenter_Destroy);
    BDBG_ASSERT(hPresenter);

    if (hPresenter->sEventQueue.asEvents)
    {
        BKNI_Free(hPresenter->sEventQueue.asEvents);
    }

    if (hPresenter->pcName)
    {
        BKNI_Free(hPresenter->pcName);
    }
    
    BKNI_Free(hPresenter);
    
    BDBG_LEAVE(BASTMlib_Presenter_Destroy);
}

void BASTMlib_Presenter_GetDefaultConfig(BASTMlib_Presenter_Config * psConfig)
{
    BDBG_ENTER(BASTMlib_Presenter_P_GetDefaultConfig);

    BDBG_ASSERT(psConfig);

    BKNI_Memset(psConfig, 0, sizeof(BASTMlib_Presenter_Config));
    psConfig->uiPassEventCountThreshold = BASTMLIB_PRESENTER_P_DEFAULT_PASS_EVENT_COUNT_THRESHOLD;
    psConfig->uiFailEventCountThreshold = BASTMLIB_PRESENTER_P_DEFAULT_FAIL_EVENT_COUNT_THRESHOLD;

    BDBG_LEAVE(BASTMlib_Presenter_P_GetDefaultConfig);
}

void BASTMlib_Presenter_Flush_isr(
    BASTMlib_Presenter_Handle hPresenter
)
{
    BDBG_ENTER(BASTMlib_Presenter_Flush_isr);

    BDBG_ASSERT(hPresenter);

    hPresenter->sEventQueue.uiWrite = hPresenter->sEventQueue.uiCapacity / 2;
    hPresenter->sEventQueue.uiRead = hPresenter->sEventQueue.uiWrite - 1;
    hPresenter->sEventQueue.uiSize = 0;

    BDBG_LEAVE(BASTMlib_Presenter_Flush_isr);
}

void BASTMlib_Presenter_Reset_isr(
    BASTMlib_Presenter_Handle hPresenter
)
{
    BDBG_ENTER(BASTMlib_Presenter_Reset_isr);

    BDBG_ASSERT(hPresenter);

    hPresenter->uiPassEventCount = 0;
    hPresenter->uiFailEventCount = 0;

    BDBG_LEAVE(BASTMlib_Presenter_Reset_isr);
}

void BASTMlib_Presenter_GetConfig(
    const BASTMlib_Presenter_Handle hPresenter, 
    BASTMlib_Presenter_Config * psConfig
)
{
    BDBG_ENTER(BASTMlib_Presenter_GetConfig);

    BDBG_ASSERT(hPresenter);
    BDBG_ASSERT(psConfig);

    *psConfig = hPresenter->sConfig;
    
    BDBG_LEAVE(BASTMlib_Presenter_GetConfig);
}

void BASTMlib_Presenter_SetConfig(
    BASTMlib_Presenter_Handle hPresenter, 
    const BASTMlib_Presenter_Config * psConfig
)
{
    BDBG_ENTER(BASTMlib_Presenter_SetConfig);

    BDBG_ASSERT(hPresenter);
    BDBG_ASSERT(psConfig);

    hPresenter->sConfig = *psConfig;

    if (hPresenter->pcName)
    {
        BDBG_MSG(("Presenter %s reconfigured:", hPresenter->pcName));
    }
    else
    {
        BDBG_MSG(("Presenter %d reconfigured:", hPresenter->uiId));
    }
    BDBG_MSG(("  min time between events: %u ms", hPresenter->sConfig.uiMinimumTimeBetweenEvents));
    BDBG_MSG(("  pass event count threshold: %u events", hPresenter->sConfig.uiPassEventCountThreshold));
    BDBG_MSG(("  fail event count threshold: %u events", hPresenter->sConfig.uiFailEventCountThreshold));

	/* PR:50140 check for assignment before using hAstm */
    /* don't resize if already started, wait till next restart, or resize if unassigned */
	if ((hPresenter->hAstm && !hPresenter->hAstm->bStarted) || !hPresenter->hAstm)
	{
        BASTMlib_Presenter_ResizeEventQueue(hPresenter);
	}

    BDBG_LEAVE(BASTMlib_Presenter_SetConfig);
}

void BASTMlib_Presenter_EventHandler_isr(
    BASTMlib_Presenter_Handle hPresenter,
    const BASTMlib_Presenter_Event * psEvent
)
{
    BSYSlib_List_IteratorHandle hIterator = NULL;
    bool bFound = false;
    BASTMlib_Handle hAstm = NULL;

    BDBG_ENTER(BASTMlib_Presenter_EventHandler_isr);

    BDBG_ASSERT(hPresenter);
    BDBG_ASSERT(psEvent);

    hAstm = hPresenter->hAstm;

    if (hAstm)
    {
        if (hAstm->bStarted)
        {
            hIterator = BSYSlib_List_AcquireIterator_isr(hAstm->sPresentation.hPresenters);
            while (BSYSlib_List_HasNext_isr(hIterator))
            {
                BASTMlib_Presenter_Handle hTemp = NULL;
                hTemp = (BASTMlib_Presenter_Handle)BSYSlib_List_Next_isr(hIterator);

                if (hTemp == hPresenter)
                {
                    bFound = true;
                    break;
                }
            }

            BSYSlib_List_ReleaseIterator_isr(hIterator);

            if (bFound)
            {
                if (hAstm->sPresentation.bAcquire)
                {
                    if (hPresenter->sEventQueue.uiSize < hPresenter->sEventQueue.uiCapacity)
                    {
#if BASTMLIB_DEBUG_QUEUE
                        BDBG_MSG(("Adding event to presenter %p queue at %u", hPresenter, hPresenter->sEventQueue.uiWrite));
#endif
                        hPresenter->sEventQueue.asEvents[hPresenter->sEventQueue.uiWrite] = *psEvent;
                        hPresenter->sEventQueue.uiWrite++;
                        hPresenter->sEventQueue.uiWrite %= hPresenter->sEventQueue.uiCapacity;
                        hPresenter->sEventQueue.uiSize++;
                    }
                    else
                    {
                        BDBG_MSG(("Presenter %p event queue full.  Discarding events.", (void *)hPresenter));
                    }
                }
                else
                {
                    BDBG_MSG(("Presenter %p event received while not acquiring. Ignored", (void *)hPresenter));
                }
            }
            else
            {
                BDBG_MSG(("Unmanaged presenter (%p) event received", (void *)hPresenter));
            }
        }
        else
        {
            BDBG_MSG(("Presenter %p event received while stopped. Ignored", (void *)hPresenter));
        }
    }
    else
    {
        BDBG_MSG(("Unmanaged presenter (%p) event received", (void *)hPresenter));
    }

    BDBG_LEAVE(BASTMlib_Presenter_EventHandler_isr);
}

void BASTMlib_Presenter_ResizeEventQueue(
    BASTMlib_Presenter_Handle hPresenter
)
{
    unsigned int uiMinimumTimeBetweenEvents;
    unsigned int uiMaximumAcquisitionTime;
    unsigned int uiCapacity;

    BDBG_ASSERT(hPresenter);
    
    BDBG_ENTER(BASTMlib_Presenter_ResizeEventQueue);

    uiMinimumTimeBetweenEvents = hPresenter->sConfig.uiMinimumTimeBetweenEvents;
    uiMaximumAcquisitionTime = hPresenter->uiMaximumAcquisitionTime;

    if (uiMinimumTimeBetweenEvents && uiMaximumAcquisitionTime)
    {
        uiCapacity = uiMaximumAcquisitionTime / uiMinimumTimeBetweenEvents + 1;
        uiCapacity += uiCapacity / 10; /* 10% bigger than required */
    }
    else
    {
        uiCapacity = BASTMLIB_PRESENTER_P_DEFAULT_EVENT_QUEUE_CAPACITY;
    }

    if (hPresenter->sEventQueue.uiCapacity != uiCapacity)
    {
        if (hPresenter->sEventQueue.asEvents)
        {
            BKNI_Free(hPresenter->sEventQueue.asEvents);
        }

        hPresenter->sEventQueue.asEvents = (BASTMlib_Presenter_Event *)BKNI_Malloc(sizeof(BASTMlib_Presenter_Event) * uiCapacity);
        BDBG_ASSERT(hPresenter->sEventQueue.asEvents);
        BKNI_Memset(hPresenter->sEventQueue.asEvents, 0, sizeof(BASTMlib_Presenter_Event) * uiCapacity);

        hPresenter->sEventQueue.uiCapacity = uiCapacity;

        BKNI_EnterCriticalSection();
        BASTMlib_Presenter_Flush_isr(hPresenter);
        BKNI_LeaveCriticalSection();
    }

    BDBG_LEAVE(BASTMlib_Presenter_ResizeEventQueue);
}

