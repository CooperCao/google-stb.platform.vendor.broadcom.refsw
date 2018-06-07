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
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#include "bvc5.h"
#include "bvc5_scheduler_event_priv.h"

#include <stdbool.h>

BDBG_MODULE(BVC5_SCHED_EVENT);

typedef struct BVC5_P_Event
{
   bool        bSignalled;

   uint64_t    uiEventId;          /* index in the eventArray
                                  * equal with the uiEventId */

   bool        bWaitedOn;        /* True if the event is waited on */
} BVC5_P_Event;

/* Event slots are allocated BVC5_P_EVENT_ARRAY_CHUNK at a time */
#define BVC5_P_EVENT_ARRAY_CHUNK   16

typedef struct BVC5_P_EventArray
{
   BKNI_MutexHandle  hMutex;
   uint64_t          uiCapacity;
   int64_t           iFirstAvailable;
   BVC5_P_Event      **pEvents;
} BVC5_P_EventArray;

BERR_Code BVC5_P_EventArrayCreate(
   BVC5_EventArrayHandle *phEventArray
)
{
   BERR_Code               err = BERR_SUCCESS;
   BVC5_EventArrayHandle   hEventArr = NULL;
   size_t                  bytes;

   if (phEventArray == NULL)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   BDBG_CASSERT(sizeof(BVC5_P_EventArray) <= CPU_PAGE_SIZE);
   hEventArr = (BVC5_P_EventArray *)BKNI_Malloc(sizeof(BVC5_P_EventArray));

   if (hEventArr == NULL)
   {
      err = BERR_OUT_OF_SYSTEM_MEMORY;
      goto exit;
   }

   BKNI_Memset(hEventArr, 0, sizeof(BVC5_P_EventArray));

   err = BKNI_CreateMutex(&hEventArr->hMutex);

   if (err != BERR_SUCCESS)
      goto exit;

   bytes = sizeof(BVC5_P_Event*) * BVC5_P_EVENT_ARRAY_CHUNK;
   hEventArr->pEvents = (BVC5_P_Event **)BKNI_Malloc(bytes);

   if (hEventArr->pEvents == NULL)
   {
      err = BERR_OUT_OF_SYSTEM_MEMORY;
      goto exit;
   }

   hEventArr->uiCapacity      = BVC5_P_EVENT_ARRAY_CHUNK;
   hEventArr->iFirstAvailable = 0;

   BKNI_Memset(hEventArr->pEvents, 0, bytes);

   *phEventArray = hEventArr;

exit:
   if (err != BERR_SUCCESS)
      BVC5_P_EventArrayDestroy(hEventArr);

   return err;
}

void BVC5_P_EventArrayDestroy(
   BVC5_EventArrayHandle hEventArr
)
{
   if (hEventArr == NULL)
      return;

   if (hEventArr->hMutex != 0)
      BKNI_DestroyMutex(hEventArr->hMutex);

   if (hEventArr->pEvents != NULL)
      BKNI_Free(hEventArr->pEvents);

   BKNI_Free(hEventArr);
}


bool BVC5_P_NewSchedEvent(
      BVC5_EventArrayHandle   hEventArr,
      uint64_t                *pSchedEventId)
{
   bool          found = false;
   int64_t       index = -1;
   uint64_t      i;
   BVC5_P_Event  *pEvent = NULL;

   BKNI_AcquireMutex(hEventArr->hMutex);

   BDBG_CASSERT(sizeof(BVC5_P_Event) <= CPU_PAGE_SIZE);
   pEvent = BKNI_Malloc(sizeof(BVC5_P_Event));
   BKNI_Memset(pEvent, 0, sizeof(BVC5_P_Event));

   /* If there is not more space allocate a new array and copy the data across*/
   if (hEventArr->iFirstAvailable == -1)
   {
      BVC5_P_Event **newArray =
         (BVC5_P_Event **)BKNI_Malloc(sizeof(BVC5_P_Event*) * (hEventArr->uiCapacity + BVC5_P_EVENT_ARRAY_CHUNK));

      if (newArray == NULL)
      {
         BDBG_ERR(("BVC5_P_NewSchedEvent -- Out of memory"));
         goto exit;
      }

      BKNI_Memcpy(newArray, hEventArr->pEvents, sizeof(BVC5_P_Event*) * hEventArr->uiCapacity);
      BKNI_Memset(&newArray[hEventArr->uiCapacity], 0, sizeof(BVC5_P_Event*) * BVC5_P_EVENT_ARRAY_CHUNK);

      hEventArr->iFirstAvailable = hEventArr->uiCapacity;
      hEventArr->uiCapacity      = hEventArr->uiCapacity + BVC5_P_EVENT_ARRAY_CHUNK;

      if (hEventArr->pEvents != NULL)
         BKNI_Free(hEventArr->pEvents);
      hEventArr->pEvents = newArray;
   }

   index = hEventArr->iFirstAvailable;
   BDBG_ASSERT(index >= 0);
   BDBG_ASSERT(hEventArr->pEvents[index] == NULL);
   hEventArr->pEvents[index] = pEvent;

   pEvent->bSignalled   = false;
   pEvent->uiEventId    = (uint64_t)index;
   pEvent->bWaitedOn    = false;
   *pSchedEventId       = pEvent->uiEventId;

   BDBG_MSG(("BVC5_P_NewSchedEvent -- pEvent:  %p, eventid: " BDBG_UINT64_FMT, (void *)pEvent, BDBG_UINT64_ARG(pEvent->uiEventId)));

   /* Find the next available event */
   for (i = hEventArr->iFirstAvailable + 1; i < hEventArr->uiCapacity && !found; ++i)
   {
      if (hEventArr->pEvents[i] == NULL)
      {
         hEventArr->iFirstAvailable = i;
         found = true;
      }
   }

   /* Run out? */
   if (!found)
      hEventArr->iFirstAvailable = -1;

exit:
   if (index < 0)
   {
      if (pEvent)
         BKNI_Free(pEvent);
   }

   BKNI_ReleaseMutex(hEventArr->hMutex);
   return  index >= 0;

}

/* return the event corresponding to event index;
   hEventArr mutex must be held by the caller
 */
static BVC5_P_Event *BVC5_P_EventGet(
   BVC5_EventArrayHandle hEventArr,
   int uiEventId
)
{
   BDBG_MSG(("Searching for Event"));
   if (uiEventId < 0 || (uint64_t)uiEventId >= hEventArr->uiCapacity)
   {
      BDBG_ERR(("Searching for wrong eventid=%d\n", uiEventId));
      BDBG_ASSERT(0);
      return NULL;
   }

   if (hEventArr->pEvents[uiEventId] == NULL)
   {
      BDBG_ERR(("Searching for destroyed eventid=%d\n", uiEventId));
      BDBG_ASSERT(0);
      return NULL;
   }

   BDBG_ASSERT(hEventArr->pEvents[uiEventId]->uiEventId == (unsigned) uiEventId);
   return hEventArr->pEvents[uiEventId];
}

void BVC5_P_DeleteSchedEvent(
    BVC5_EventArrayHandle hEventArr,
    uint64_t             uiSchedEventId
)
{
   unsigned index;
   BVC5_P_Event *pEvent;

   BDBG_MSG(("Deleting Event id: " BDBG_UINT64_FMT, BDBG_UINT64_ARG(uiSchedEventId)));

   BKNI_AcquireMutex(hEventArr->hMutex);

   pEvent = BVC5_P_EventGet(hEventArr, uiSchedEventId);

   BDBG_ASSERT(pEvent && pEvent->uiEventId < hEventArr->uiCapacity);
   BDBG_ASSERT(hEventArr->pEvents[pEvent->uiEventId] == pEvent);

   index = pEvent->uiEventId;
   BDBG_MSG(("destroy Event pEvent=%p eventId= " BDBG_UINT64_FMT, (void*)pEvent,
         BDBG_UINT64_ARG(pEvent->uiEventId)));
   hEventArr->pEvents[index] = NULL;
   BKNI_Free(pEvent);

   if (hEventArr->iFirstAvailable < 0 ||
         (index < (unsigned)hEventArr->iFirstAvailable))
   {
      hEventArr->iFirstAvailable = index;
   }

   BKNI_ReleaseMutex(hEventArr->hMutex);
}

bool BVC5_P_WaitOnSchedEventDone(
    BVC5_EventArrayHandle hEventArr,
    uint64_t             uiSchedEventId
)
{
   bool signalled = false;
   BVC5_P_Event *pEvent;

   BKNI_AcquireMutex(hEventArr->hMutex);

   pEvent = BVC5_P_EventGet(hEventArr, uiSchedEventId);

   if (pEvent)
   {
      BDBG_ASSERT(pEvent->uiEventId < hEventArr->uiCapacity);
      BDBG_ASSERT(hEventArr->pEvents[pEvent->uiEventId] == pEvent);

      signalled = pEvent->bSignalled;

      /* Flag that the event is waited on */
      if (!signalled)
         pEvent->bWaitedOn = true;
      else
         pEvent->bWaitedOn = false;
   }
   else
      signalled = true; /* if the event has not been found */
                        /* let's behave as it has already  */
                        /* been signalled.                 */

   BKNI_ReleaseMutex(hEventArr->hMutex);

   return signalled;
}

void BVC5_P_SetSchedEvent(
    BVC5_EventArrayHandle hEventArr,
    uint64_t             uiSchedEventId
)
{
   BVC5_P_Event *pEvent;

   BDBG_MSG(("Setting Event id: " BDBG_UINT64_FMT, BDBG_UINT64_ARG(uiSchedEventId)));

   BKNI_AcquireMutex(hEventArr->hMutex);

   pEvent = BVC5_P_EventGet(hEventArr, uiSchedEventId);

   if (pEvent)
   {
      BDBG_ASSERT(pEvent && pEvent->uiEventId < hEventArr->uiCapacity);
      BDBG_ASSERT(hEventArr->pEvents[pEvent->uiEventId] == pEvent);

      pEvent->bSignalled = true;
   }
   else
      BDBG_ERR(("Event id: " BDBG_UINT64_FMT " doesn't exist", BDBG_UINT64_ARG(uiSchedEventId)));

   BKNI_ReleaseMutex(hEventArr->hMutex);
}

void BVC5_P_ResetSchedEvent(
    BVC5_EventArrayHandle hEventArr,
    uint64_t             uiSchedEventId
)
{
   BVC5_P_Event *pEvent;

   BDBG_MSG(("Resetting Event id: " BDBG_UINT64_FMT, BDBG_UINT64_ARG(uiSchedEventId)));

   BKNI_AcquireMutex(hEventArr->hMutex);

   pEvent = BVC5_P_EventGet(hEventArr, uiSchedEventId);

   BDBG_ASSERT(pEvent && pEvent->uiEventId < hEventArr->uiCapacity);
   BDBG_ASSERT(hEventArr->pEvents[pEvent->uiEventId] == pEvent);

   if (pEvent->bWaitedOn)
      BDBG_ERR(("Resetting Event id: " BDBG_UINT64_FMT " when there is a waiter", BDBG_UINT64_ARG(uiSchedEventId)));

   pEvent->bSignalled = false;

   BKNI_ReleaseMutex(hEventArr->hMutex);
}

bool BVC5_P_QuerySchedEvent(
    BVC5_EventArrayHandle hEventArr,
    uint64_t             uiSchedEventId
)
{
   BVC5_P_Event *pEvent;
   bool signalled = false;

   BDBG_MSG(("Querying Event id: " BDBG_UINT64_FMT, BDBG_UINT64_ARG(uiSchedEventId)));

   BKNI_AcquireMutex(hEventArr->hMutex);

   pEvent = BVC5_P_EventGet(hEventArr, uiSchedEventId);

   BDBG_ASSERT(pEvent && pEvent->uiEventId < hEventArr->uiCapacity);
   BDBG_ASSERT(hEventArr->pEvents[pEvent->uiEventId] == pEvent);

   signalled = pEvent->bSignalled;

   BKNI_ReleaseMutex(hEventArr->hMutex);

   return signalled;
}

void BVC5_P_DeleteAllSchedEvent(
      BVC5_EventArrayHandle hEventArr
)
{
   uint64_t i;

   BDBG_MSG(("BVC5_P_DeleteAllSchedEvent"));

   BKNI_AcquireMutex(hEventArr->hMutex);

   for (i = 0; i < hEventArr->uiCapacity; ++i)
   {
      BVC5_P_Event   *pEvent = hEventArr->pEvents[i];
      if (pEvent)
      {
         hEventArr->pEvents[i] = NULL;
         BKNI_Free(pEvent);
      }
   }

   BKNI_ReleaseMutex(hEventArr->hMutex);
}
