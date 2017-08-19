/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bvc5_fence_priv.h"

#include <stdbool.h>

BDBG_MODULE(BVC5_Fence);

#define MAX_FENCE_CALLBACKS 2

typedef struct BVC5_P_Fence
{
   bool               bSignaled;

   /* Client id is used when client dies to remove fences and callbacks belonging to it */
   struct Callback
   {
      uint32_t         uiClientId;                    /* owner of the callback         */
      void           (*pfnCallback)(void *, uint64_t);/* pfnCallback(pContext, uiParam)*/
      void            *pContext;                      /* hVC5 or Nexus module handle   */
      uint64_t         uiParam;                       /* Auxiliiary data e.g. hEvent   */
   } sCallback[MAX_FENCE_CALLBACKS];

   uint32_t            uiClientId;                     /* owner of the fence */
   unsigned            uiRefCount;
   unsigned            uiIndex;                        /* index in the fenceArray
                                                          equal with the iFenceId */
} BVC5_P_Fence;

/* Fence slots are allocated BVC5_P_FENCE_ARRAY_CHUNK at a time */
#define BVC5_P_FENCE_ARRAY_CHUNK   16

typedef struct BVC5_P_FenceArray
{
   BKNI_MutexHandle  hMutex;
   unsigned          uiCapacity;
   int               iFirstAvailable;
   BVC5_P_Fence      **pFences;
} BVC5_P_FenceArray;

BERR_Code BVC5_P_FenceArrayCreate(
   BVC5_FenceArrayHandle *phFenceArray
)
{
   BERR_Code               err = BERR_SUCCESS;
   BVC5_FenceArrayHandle   hFenceArr = NULL;
   size_t                  bytes;

   if (phFenceArray == NULL)
   {
      err = BERR_INVALID_PARAMETER;
      goto exit;
   }

   hFenceArr = (BVC5_P_FenceArray *)BKNI_Malloc(sizeof(BVC5_P_FenceArray));

   if (hFenceArr == NULL)
   {
      err = BERR_OUT_OF_SYSTEM_MEMORY;
      goto exit;
   }

   BKNI_Memset(hFenceArr, 0, sizeof(BVC5_P_FenceArray));

   err = BKNI_CreateMutex(&hFenceArr->hMutex);

   if (err != BERR_SUCCESS)
      goto exit;

   bytes = sizeof(BVC5_P_Fence*) * BVC5_P_FENCE_ARRAY_CHUNK;
   hFenceArr->pFences = (BVC5_P_Fence **)BKNI_Malloc(bytes);

   if (hFenceArr->pFences == NULL)
   {
      err = BERR_OUT_OF_SYSTEM_MEMORY;
      goto exit;
   }

   hFenceArr->uiCapacity      = BVC5_P_FENCE_ARRAY_CHUNK;
   hFenceArr->iFirstAvailable = 0;

   BKNI_Memset(hFenceArr->pFences, 0, bytes);

   *phFenceArray = hFenceArr;

exit:
   if (err != BERR_SUCCESS)
      BVC5_P_FenceArrayDestroy(hFenceArr);

   return err;
}

void BVC5_P_FenceArrayDestroy(
   BVC5_FenceArrayHandle hFenceArr
)
{
   if (hFenceArr == NULL)
      return;

   if (hFenceArr->hMutex != 0)
      BKNI_DestroyMutex(hFenceArr->hMutex);

   if (hFenceArr->pFences != NULL)
      BKNI_Free(hFenceArr->pFences);

   BKNI_Free(hFenceArr);
}

/* the caller must held the lock to hFenceArr */
static void BVC5_P_FenceRefCountInc(
   BVC5_P_Fence *pFence
)
{
   pFence->uiRefCount++;
}

/* the caller must held the lock to hFenceArr */
static void BVC5_P_FenceDestroy(
    BVC5_P_Fence *pFence,
    BVC5_FenceArrayHandle hFenceArr
)
{
   unsigned index;

   BDBG_ASSERT(pFence && pFence->uiIndex < hFenceArr->uiCapacity);
   BDBG_ASSERT(hFenceArr->pFences[pFence->uiIndex] == pFence);

   index = pFence->uiIndex;
   BDBG_MSG(("destroy fence pFence=%p fenceId=%d, refcount=%d", (void*)pFence,
         pFence->uiIndex, pFence->uiRefCount));
   hFenceArr->pFences[index] = NULL;
   BKNI_Free(pFence);

   if (hFenceArr->iFirstAvailable < 0 ||
         (index < (unsigned)hFenceArr->iFirstAvailable))
   {
      hFenceArr->iFirstAvailable = index;
   }
}

/* the caller must held the lock to hFenceArr */
static void BVC5_P_FenceRefCountDec(
    BVC5_P_Fence *pFence,
    BVC5_FenceArrayHandle hFenceArr
)
{
   BDBG_ASSERT(pFence);

   pFence->uiRefCount--;
   if (pFence->uiRefCount == 0)
      BVC5_P_FenceDestroy(pFence, hFenceArr);
}

int BVC5_P_FenceCreate(
   BVC5_FenceArrayHandle    hFenceArr,
   uint32_t                 uiClientId,
   void                   **dataToSignal
)
{
   bool          found = false;
   int           index = -1;
   unsigned      i;
   BVC5_P_Fence *pFence = NULL;

   BKNI_AcquireMutex(hFenceArr->hMutex);
   BDBG_MSG(("BVC5_P_FenceCreate uiClientId=%d", uiClientId));

   pFence = BKNI_Malloc(sizeof(BVC5_P_Fence));
   BKNI_Memset(pFence, 0, sizeof(BVC5_P_Fence));

   if (hFenceArr->iFirstAvailable == -1)
   {
      BVC5_P_Fence **newArray =
         (BVC5_P_Fence **)BKNI_Malloc(sizeof(BVC5_P_Fence*) * (hFenceArr->uiCapacity + BVC5_P_FENCE_ARRAY_CHUNK));

      if (newArray == NULL)
         goto exit;

      BKNI_Memcpy(newArray, hFenceArr->pFences, sizeof(BVC5_P_Fence*) * hFenceArr->uiCapacity);
      BKNI_Memset(&newArray[hFenceArr->uiCapacity], 0, sizeof(BVC5_P_Fence*) * BVC5_P_FENCE_ARRAY_CHUNK);

      hFenceArr->iFirstAvailable = hFenceArr->uiCapacity;
      hFenceArr->uiCapacity      = hFenceArr->uiCapacity + BVC5_P_FENCE_ARRAY_CHUNK;

      if (hFenceArr->pFences != NULL)
         BKNI_Free(hFenceArr->pFences);
      hFenceArr->pFences = newArray;
   }

   index = hFenceArr->iFirstAvailable;
   BDBG_ASSERT(index >= 0);
   BDBG_ASSERT(hFenceArr->pFences[index] == NULL);
   hFenceArr->pFences[index] = pFence;

   pFence->bSignaled = false;
   pFence->uiClientId = uiClientId;
   pFence->uiIndex   = (unsigned)index;

   /* start with 2 references - one for the signalling and one for the waiting end */
   pFence->uiRefCount = 2;

   BDBG_MSG(("Fence %d = %p", index, (void*)hFenceArr->pFences[index]));

   /* Find the next available fence */
   for (i = hFenceArr->iFirstAvailable + 1; i < hFenceArr->uiCapacity && !found; ++i)
   {
      if (hFenceArr->pFences[i] == NULL)
      {
         hFenceArr->iFirstAvailable = i;
         found = true;
      }
   }

   /* Run out? */
   if (!found)
      hFenceArr->iFirstAvailable = -1;

   *dataToSignal = &pFence->uiIndex;

exit:
   if (index < 0)
   {
      if (pFence)
         BKNI_Free(pFence);
      *dataToSignal = NULL;
   }
   BKNI_ReleaseMutex(hFenceArr->hMutex);
   return index;
}

/* return the fence corresponding to fence index;
   hFenceArr mutex must be held by the caller
 */
static BVC5_P_Fence *BVC5_P_FenceGet(
   BVC5_FenceArrayHandle hFenceArr,
   int iFenceId
)
{
   BDBG_MSG(("Searching for fence"));
   if (iFenceId < 0 || (unsigned int)iFenceId >= hFenceArr->uiCapacity)
   {
      BDBG_ERR(("Searching for wrong fenceid=%d\n", iFenceId));
      BDBG_ASSERT(0);
      return NULL;
   }

   if (hFenceArr->pFences[iFenceId] == NULL)
   {
      BDBG_ERR(("Searching for destroyed fenceid=%d\n", iFenceId));
      BDBG_ASSERT(0);
      return NULL;
   }

   BDBG_ASSERT(hFenceArr->pFences[iFenceId]->uiIndex == (unsigned) iFenceId);
   return hFenceArr->pFences[iFenceId];
}


int BVC5_P_FenceKeep(
   BVC5_FenceArrayHandle hFenceArr,
   int                   iFenceId
)
{
   BVC5_P_Fence *pFence = NULL;

   BKNI_AcquireMutex(hFenceArr->hMutex);

   BDBG_MSG(("BVC5_P_FenceDup %d", iFenceId));

   pFence = BVC5_P_FenceGet(hFenceArr, iFenceId);
   if (pFence == NULL)
   {
      goto end;
   }

   BVC5_P_FenceRefCountInc(pFence);

end:
   BKNI_ReleaseMutex(hFenceArr->hMutex);
   return pFence != NULL ? 0 : -1;
}

void BVC5_P_FenceClose(
   BVC5_FenceArrayHandle hFenceArr,
   int                   iFenceId
)
{
   BVC5_P_Fence *pFence = NULL;

   BKNI_AcquireMutex(hFenceArr->hMutex);

   BDBG_MSG(("BVC5_P_FenceClose %d", iFenceId));

   pFence = BVC5_P_FenceGet(hFenceArr, iFenceId);
   if (pFence == NULL)
      goto end;

   BVC5_P_FenceRefCountDec(pFence, hFenceArr);

end:
   BKNI_ReleaseMutex(hFenceArr->hMutex);
}

static struct Callback *BVC5_P_FenceCbEnd(BVC5_P_Fence *pFence)
{
   static const int count = sizeof(pFence->sCallback) / sizeof(pFence->sCallback[0]);
   return pFence->sCallback + count; /* one past the last item */
}

static struct Callback  *BVC5_P_FenceCbInit(
   struct Callback        *pCallback,
   uint32_t                uiClientId,
   void                  (*pfnCallback)(void *, uint64_t),
   void                   *pContext,
   uint64_t                uiParam
)
{
   pCallback->uiClientId = uiClientId;
   pCallback->pfnCallback = pfnCallback;
   pCallback->pContext = pContext;
   pCallback->uiParam = uiParam;
   return pCallback;
}

static bool BVC5_P_FenceCbMatch(
   const struct Callback *pLeft,
   const struct Callback *pRight
)
{
   return
      (pLeft->uiClientId  == pRight->uiClientId) &&
      (pLeft->pfnCallback == pRight->pfnCallback) &&
      (pLeft->pContext    == pRight->pContext) &&
      (pLeft->uiParam     == pRight->uiParam);
}

static struct Callback *BVC5_P_FenceFindCbMatch(
   BVC5_P_Fence           *pFence,
   const struct Callback  *pCallback
)
{
   struct Callback *cb = pFence->sCallback;
   struct Callback *end = BVC5_P_FenceCbEnd(pFence);
   struct Callback *found = NULL;
   while(cb < end)
   {
      if (BVC5_P_FenceCbMatch(cb, pCallback))
      {
         found = cb;
         break;
      }
      cb++;
   }
   return found;
}

static const struct Callback empty_cb = { 0, NULL, NULL, 0 };

static bool BVC5_P_FenceReplaceCb(
   BVC5_P_Fence           *pFence,
   const struct Callback  *pFind,
   const struct Callback  *pReplace
)
{
   struct Callback *found = BVC5_P_FenceFindCbMatch(pFence, pFind);
   if (found)
      *found = *pReplace;
   return found != NULL;
}

static bool BVC5_P_FenceAddCb(
   BVC5_P_Fence           *pFence,
   struct Callback        *pCallback
)
{
   return BVC5_P_FenceReplaceCb(pFence, &empty_cb, pCallback);
}

static bool BVC5_P_FenceDelCb(
   BVC5_P_Fence           *pFence,
   struct Callback        *pCallback
)
{
   return BVC5_P_FenceReplaceCb(pFence, pCallback, &empty_cb);
}

static void BVC5_P_FenceDelClientCb(
   BVC5_P_Fence           *pFence,
   uint32_t                uiClientId
)
{
   struct Callback *cb = pFence->sCallback;
   struct Callback *end = BVC5_P_FenceCbEnd(pFence);
   while(cb < end)
   {
      if (cb->uiClientId == uiClientId)
         *cb = empty_cb;
      cb++;
   }
}


static void BVC5_P_FenceCallback(
   BVC5_P_Fence   *pFence
)
{
   struct Callback *cb = pFence->sCallback;
   struct Callback *end = BVC5_P_FenceCbEnd(pFence);
   while(cb < end)
   {
      if (cb->pfnCallback)
      {
         cb->pfnCallback(cb->pContext, cb->uiParam);

         /* don't call more than once */
         *cb = empty_cb;
      }
      ++cb;
   }
}

void BVC5_P_FenceSignalAndCleanup(
   BVC5_FenceArrayHandle  hFenceArr,
   void                  *dataToSignal
)
{
   BVC5_P_Fence *pFence;
   int           fenceId;

   BKNI_AcquireMutex(hFenceArr->hMutex);

   fenceId = *(int*)dataToSignal;

   BDBG_MSG(("BVC5_P_FenceSignalAndCleanup fenceId=%d", fenceId));

   pFence = BVC5_P_FenceGet(hFenceArr, fenceId);
   if (pFence == NULL)
   {
      BDBG_MSG(("wrong data to signal fenceId=%d", fenceId));
      goto end;
   }

   BDBG_ASSERT(!pFence->bSignaled);
   pFence->bSignaled = true;
   BVC5_P_FenceCallback(pFence);

   BVC5_P_FenceRefCountDec(pFence, hFenceArr);

end:
   BKNI_ReleaseMutex(hFenceArr->hMutex);
}

int BVC5_P_FenceWaitAsync(
   BVC5_FenceArrayHandle   hFenceArr,
   uint32_t                uiClientId,
   int                     iFenceId,
   void                  (*pfnCallback)(void *, uint64_t),
   void                   *pContext,
   uint64_t                uiParam,
   void                  **waitData
)
{
   BVC5_P_Fence *pFence;
   int           res = -1;
   struct Callback cb;

   BKNI_AcquireMutex(hFenceArr->hMutex);
   *waitData = NULL;

   BDBG_MSG(("BVC5_P_FenceWaitAsync fenceId=%d", iFenceId));

   pFence = BVC5_P_FenceGet(hFenceArr, iFenceId);
   if (pFence == NULL)
   {
      BDBG_MSG(("Waiting on wrong fenceId=%d", iFenceId));
      goto end;
    };

   if (pFence->bSignaled)
   {
      res = 1;
      goto end;
   }

   BVC5_P_FenceCbInit(&cb, uiClientId, pfnCallback, pContext, uiParam);
   if (!BVC5_P_FenceAddCb(pFence, &cb))
   {
      res = -1;
      goto end;
   }

   /* we get ownership of iFenceId; it will be decremented when we
    * WaitAsyncCleanup gets called */
   *waitData = &pFence->uiIndex;
   res = 0;
end:
   BKNI_ReleaseMutex(hFenceArr->hMutex);
   return res;
}

int BVC5_P_FenceWaitAsyncIsSignaled(BVC5_FenceArrayHandle hFenceArr,
   void *waitData
)
{
   BVC5_P_Fence *pFence;
   int           fenceId;
   int           signaled = 0;

   BKNI_AcquireMutex(hFenceArr->hMutex);
   fenceId = *(int*)waitData;

   pFence = BVC5_P_FenceGet(hFenceArr, fenceId);
   if (pFence == NULL)
   {
      BDBG_MSG(("wrong waitData to check if fence is signaled fenceId=%d", fenceId));
      goto end;
   }

   if (pFence->bSignaled)
      signaled = 1;
end:
   BKNI_ReleaseMutex(hFenceArr->hMutex);
   return signaled;
}

void BVC5_P_FenceWaitAsyncCleanup(
   BVC5_FenceArrayHandle  hFenceArr,
   uint32_t               uiClientId,
   void                 (*pfnCallback)(void *, uint64_t),
   void                  *pContext,
   uint64_t               uiParam,
   void                  *waitData
)
{
   BVC5_P_Fence *pFence;
   int           fenceId;
   struct Callback cb;

   BKNI_AcquireMutex(hFenceArr->hMutex);
   fenceId = *(int*)waitData;

   BDBG_MSG(("BVC5_P_FenceWaitAsyncCleanup fenceId=%d", fenceId));

   pFence = BVC5_P_FenceGet(hFenceArr, fenceId);
   if (pFence == NULL)
   {
      BDBG_MSG(("wrong data to cancel async fenceId=%d", fenceId));
      goto end;
   }

   BVC5_P_FenceCbInit(&cb, uiClientId, pfnCallback, pContext, uiParam);
   BVC5_P_FenceDelCb(pFence, &cb);

   BVC5_P_FenceRefCountDec(pFence, hFenceArr);
end:
   BKNI_ReleaseMutex(hFenceArr->hMutex);
}

/* for addcallback/remove we do not increase/dec refcount because the caller
 * must hold a valid fenceId for the duration of the operations */
void BVC5_P_FenceAddCallback(
   BVC5_FenceArrayHandle  hFenceArr,
   int                    iFenceId,
   uint32_t               uiClientId,
   void                 (*pfnCallback)(void *, uint64_t),
   void                  *pContext,
   uint64_t               uiParam
)
{
   BVC5_P_Fence   *pFence = NULL;
   struct Callback cb;

   BDBG_MSG(("Add callback fenceId=%d", iFenceId));

   /* TODO: handle multiple callbacks */
   BKNI_AcquireMutex(hFenceArr->hMutex);

   pFence = BVC5_P_FenceGet(hFenceArr, iFenceId);
   if (pFence == NULL)
      goto end;

   if (pFence->bSignaled)
      pfnCallback(pContext, uiParam);
   else
   {
      BVC5_P_FenceCbInit(&cb, uiClientId, pfnCallback, pContext, uiParam);
      BVC5_P_FenceAddCb(pFence, &cb);
   }

end:
   BKNI_ReleaseMutex(hFenceArr->hMutex);
}

bool BVC5_P_FenceRemoveCallback(
   BVC5_FenceArrayHandle hFenceArr,
   int                   iFenceId,
   uint32_t              uiClientId,
   void                (*pfnCallback)(void *, uint64_t),
   void                 *pContext,
   uint64_t              uiParam
)
{
   BVC5_P_Fence   *pFence = NULL;
   struct Callback cb;
   bool signalled = false;

   BKNI_AcquireMutex(hFenceArr->hMutex);
   pFence = BVC5_P_FenceGet(hFenceArr, iFenceId);
   if (pFence == NULL)
      goto end;

   signalled = pFence->bSignaled;
   BVC5_P_FenceCbInit(&cb, uiClientId, pfnCallback, pContext, uiParam);
   BVC5_P_FenceDelCb(pFence, &cb);

end:
   BKNI_ReleaseMutex(hFenceArr->hMutex);
   return signalled;
}


void BVC5_P_FenceClientDestroy(
   BVC5_FenceArrayHandle hFenceArr,
   uint32_t              uiClientId
)
{
   uint32_t i;

   BDBG_MSG(("BVC5_P_FenceClientDestroy uiClientId=%d", uiClientId));

   BKNI_AcquireMutex(hFenceArr->hMutex);

   /* this gets called after we made sure that the scheduler had
    * signal/cleanup all the signal jobs and wait_for_fence jobs;
    * At this point, we should only have fences that were not closed and/or
    * signaled from user space */

   /* delete this client's callbacks from all fences */
   for (i = 0; i < hFenceArr->uiCapacity; ++i)
   {
      BVC5_P_Fence   *pFence = hFenceArr->pFences[i];
      if (pFence)
         BVC5_P_FenceDelClientCb(pFence, uiClientId);
   }

   /* signal and delete this client's fences - callbacks are already deleted */
   for (i = 0; i < hFenceArr->uiCapacity; ++i)
   {
      BVC5_P_Fence   *pFence = hFenceArr->pFences[i];
      if (pFence && (pFence->uiClientId == uiClientId))
      {
         BVC5_P_FenceCallback(pFence); /* de-block anyone waiting */
         BVC5_P_FenceDestroy(pFence, hFenceArr);
      }
   }

   BKNI_ReleaseMutex(hFenceArr->hMutex);
}

void BVC5_P_FenceClientCheckDestroy(
   BVC5_FenceArrayHandle hFenceArr,
   uint32_t              uiClientId
)
{
#ifdef NDEBUG
   BSTD_UNUSED(hFenceArr);
   BSTD_UNUSED(uiClientId);
#else
   uint32_t i;

   BKNI_AcquireMutex(hFenceArr->hMutex);

   /* we should not find any fences for this client */
   for (i = 0; i < hFenceArr->uiCapacity; ++i)
   {
      BVC5_P_Fence   *pFence = hFenceArr->pFences[i];

      if (pFence && pFence->uiClientId == uiClientId)
      {
         BDBG_ASSERT(0);
      }
   }

   BKNI_ReleaseMutex(hFenceArr->hMutex);
#endif
}

int BVC5_P_FenceCreateToSignalFromUser(BVC5_FenceArrayHandle hFenceArr,
   uint32_t uiClientId
)
{
   void *dataToSignal;
   int   iFenceId;
   int   signalId;

   iFenceId = BVC5_P_FenceCreate(hFenceArr, uiClientId, &dataToSignal);
   if (iFenceId == -1)
      return -1;

   /* *dataToSignal == iFenceId; close the fenceId; the caller must call
    * SignalFromUser and CloseFence
    */
   signalId = *(int*) dataToSignal;
   BDBG_ASSERT(iFenceId == signalId);
   return  signalId;
}

void BVC5_P_FenceSignalFromUser(BVC5_FenceArrayHandle hFenceArr,
   int iFenceId
)
{
   BVC5_P_FenceSignalAndCleanup(hFenceArr, &iFenceId);
}
