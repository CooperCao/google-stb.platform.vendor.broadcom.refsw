/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"
#include "bkni.h"

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/file.h>
#include <linux/atomic.h>
#include <linux/random.h>

/* Android Sync Framework */
#include "sync.h"
#include "sw_sync.h"
#include "bv3d_fence_priv.h"
#include "../bv3d_priv.h"
#include "../bv3d_worker_priv.h"

BDBG_MODULE(BV3D_Fence);

#define DEBUG_MARKER 0

#define pr_info_fence pr_debug

typedef struct BV3D_P_FenceArray
{
   spinlock_t hSpinLock;
   struct list_head  sWaitData;
   struct list_head  sSignalData;

   int iWaitDataCount;
   int iSignalDataCount;
} BV3D_P_FenceArray;

typedef struct BV3D_P_SignalData
{
   struct sw_sync_timeline   *psTimeline;

   uint32_t                   uiClientId;
#if DEBUG_MARKER
   uint16_t                   uiMarker;
#endif

   struct list_head           sNext;
} BV3D_P_SignalData;

typedef struct BV3D_P_WaitData
{
   struct sync_fence         *psSyncFence;
   struct sync_fence_waiter   sWaiter;

   atomic_t                   uiSignaled;

   uint32_t                   uiClientId;
   BV3D_Handle                hV3d;

   struct list_head           sNext;
} BV3D_P_WaitData;

/***************************************************************************/
BERR_Code BV3D_P_FenceArrayCreate(
   BV3D_FenceArrayHandle *phFenceArray
)
{
   BERR_Code               err = BERR_SUCCESS;
   BV3D_FenceArrayHandle   hFenceArr = NULL;

   hFenceArr = kzalloc(sizeof(BV3D_P_FenceArray), GFP_KERNEL);
   if (hFenceArr == NULL)
   {
      err = BERR_OUT_OF_SYSTEM_MEMORY;
      goto exit;
   }

   *phFenceArray = hFenceArr;

   spin_lock_init(&hFenceArr->hSpinLock);
   INIT_LIST_HEAD(&hFenceArr->sWaitData);
   INIT_LIST_HEAD(&hFenceArr->sSignalData);

   hFenceArr->iWaitDataCount = 0;
   hFenceArr->iSignalDataCount = 0;

exit:
   if (err != BERR_SUCCESS)
      BV3D_P_FenceArrayDestroy(hFenceArr);

   return err;
}

/***************************************************************************/
static void BV3D_P_FenceWaitAsyncCleanup(
   BV3D_FenceArrayHandle hFenceArr,
   BV3D_P_WaitData *psWaitData
)
{
   if (psWaitData->psSyncFence)
   {
      /* returns 0 if waiter was removed from fence's async waiter list.
       * returns -ENOENT if waiter was not found on fence's async waiter list */
      sync_fence_cancel_async(psWaitData->psSyncFence, &psWaitData->sWaiter);
      sync_fence_put(psWaitData->psSyncFence);
   }
}

/***************************************************************************/
static void BV3D_P_FenceSignal(
   BV3D_FenceArrayHandle hFenceArr,
   void *dataToSignal
)
{
   struct BV3D_P_SignalData *psSignalData = (struct BV3D_P_SignalData*)dataToSignal;

   pr_info_fence("%s timeline=%p\n", __FUNCTION__, psSignalData->psTimeline);
   sw_sync_timeline_inc(psSignalData->psTimeline, 1);

   /* we are done, destroy timeline */
   sync_timeline_destroy(&psSignalData->psTimeline->obj);
}

/***************************************************************************/
static void BV3D_P_FenceArrayWalkAndDestroy(
   BV3D_FenceArrayHandle hFenceArr,
   uint32_t              uiClientId,
   bool                  bAllClients
)
{
   unsigned long ulFlags;
   struct list_head *pos, *q;
   spin_lock_irqsave(&hFenceArr->hSpinLock, ulFlags);
   list_for_each_safe(pos, q, &hFenceArr->sWaitData)
   {
      BV3D_P_WaitData *psWaitData = list_entry(pos, BV3D_P_WaitData, sNext);
      if (bAllClients || (psWaitData->uiClientId == uiClientId))
      {
         BV3D_P_FenceWaitAsyncCleanup(hFenceArr, psWaitData);
         list_del(pos);
         hFenceArr->iWaitDataCount--;
         kfree(psWaitData);
      }
   }

   list_for_each_safe(pos, q, &hFenceArr->sSignalData)
   {
      BV3D_P_SignalData *psSignalData = list_entry(pos, BV3D_P_SignalData, sNext);
      if (bAllClients || (psSignalData->uiClientId == uiClientId))
      {
         BV3D_P_FenceSignal(hFenceArr, psSignalData);
         list_del(pos);
         hFenceArr->iSignalDataCount--;
         kfree(psSignalData);
      }
   }
   spin_unlock_irqrestore(&hFenceArr->hSpinLock, ulFlags);
}

/***************************************************************************/
void BV3D_P_FenceArrayDestroy(
   BV3D_FenceArrayHandle hFenceArr
)
{
   BV3D_P_FenceArrayWalkAndDestroy(hFenceArr, 0, true);

   kfree(hFenceArr);
}

/***************************************************************************/
int BV3D_P_FenceOpen(
   BV3D_FenceArrayHandle   hFenceArr,
   uint32_t                uiClientId,
   void                  **dataToSignal,
   char                    cType,
   int                     iPid
)
{
   char cName[32];
   struct sync_pt *psSyncPoint = NULL;
   struct sync_fence *psSyncFence = NULL;
   struct BV3D_P_SignalData *psSignalData = NULL;
   int fd;
   unsigned long ulFlags;

   fd = get_unused_fd_flags(0);
   if (fd < 0)
      goto end;

   psSignalData = kzalloc(sizeof(BV3D_P_SignalData), GFP_KERNEL);
   if (psSignalData == NULL)
      goto end;

#if DEBUG_MARKER
   uint16_t uiMarker = 0;
   get_random_bytes(&uiMarker, sizeof(uint16_t));
   snprintf(cName, sizeof(cName), "bv3d_f_%c_%04d_0x%04x_%p", cType, (uint16_t)iPid, uiMarker, psSignalData);
#else
   snprintf(cName, sizeof(cName), "bv3d_fence_%c_%04d_%p", cType, (uint16_t)iPid, psSignalData);
#endif

   psSignalData->psTimeline = sw_sync_timeline_create(cName);
   if (psSignalData->psTimeline == NULL)
      goto end;

   /* BKNI_Printf("%s psSignalData=%p\n", __FUNCTION__, psSignalData); */

   psSyncPoint = sw_sync_pt_create(psSignalData->psTimeline, 1);
   if (psSyncPoint == NULL)
      goto end;

   /* sync_fence_create takes ownership pf sync_point */
   psSyncFence =  sync_fence_create(cName, psSyncPoint);
   if (psSyncFence == NULL)
      goto end;
   psSyncPoint = NULL;

   /* fd takes ownership of this fence; when fd is close, sync_fence will be released */
   sync_fence_install(psSyncFence, fd);

   psSignalData->uiClientId = uiClientId;
#if DEBUG_MARKER
    psSignalData->uiMarker = uiMarker;
#endif

   *dataToSignal = psSignalData;

#if 0
   BKNI_Printf("%s: fd=%d sync_fence=%p name=%s timeline=%p client %d\n",
         __FUNCTION__, fd, psSyncFence, psSyncFence->name, psSignalData->psTimeline, psSignalData->uiClientId);
#endif

   spin_lock_irqsave(&hFenceArr->hSpinLock, ulFlags);
   list_add_tail(&psSignalData->sNext, &hFenceArr->sSignalData);
   hFenceArr->iSignalDataCount++;

   if (hFenceArr->iSignalDataCount > 50)
   {
      struct list_head *pos, *q;
      BKNI_Printf("> 50 outstanding fences open\n");
      list_for_each_safe(pos, q, &hFenceArr->sSignalData)
      {
         BV3D_P_SignalData *psSignalData = list_entry(pos, BV3D_P_SignalData, sNext);
         BKNI_Printf("---> bv3d_fence_%p\n", psSignalData);
      }
   }

   spin_unlock_irqrestore(&hFenceArr->hSpinLock, ulFlags);

   return fd;
end:
   pr_err("%s:%d FAILED\n", __FUNCTION__, __LINE__);

   if (psSyncPoint)
      sync_pt_free(psSyncPoint);

   if (psSignalData)
   {
      if (psSignalData->psTimeline)
         sync_timeline_destroy(&psSignalData->psTimeline->obj);

      kfree(psSignalData);
   }
   put_unused_fd(fd);
   *dataToSignal = NULL;
   return -1;
}

/***************************************************************************/
void BV3D_P_FenceSignalAndCleanup(
   BV3D_FenceArrayHandle hFenceArr,
   void *dataToSignal,
   const char* function
)
{
   struct BV3D_P_SignalData *psSignalData = (struct BV3D_P_SignalData*)dataToSignal;
   if (psSignalData)
   {
      unsigned long ulFlags;

#if 0
#if DEBUG_MARKER
      BKNI_Printf("%s: psSignalData=%p, marker=0x%04x (from %s)\n",
         __FUNCTION__, psSignalData, psSignalData->uiMarker, function);
#else
      BKNI_Printf("%s: psSignalData=%p (from %s)\n",
         __FUNCTION__, psSignalData, function);
#endif
#endif

      BV3D_P_FenceSignal(hFenceArr, dataToSignal);

      spin_lock_irqsave(&hFenceArr->hSpinLock, ulFlags);
      list_del(&psSignalData->sNext);
      hFenceArr->iSignalDataCount--;
      spin_unlock_irqrestore(&hFenceArr->hSpinLock, ulFlags);

      kfree(psSignalData);
   }
}

/***************************************************************************/
void BV3D_P_FenceFree(
   BV3D_FenceArrayHandle   hFenceArr,
   void                   *pWaitData
)
{
#if 0
BKNI_Printf("BV3D_P_FenceFree %p\n", pWaitData);
#endif

   /* if initially signalled without callback, then psWaitData was never allocated,
      just return */
   if (pWaitData)
   {
      unsigned long ulFlags;
      BV3D_P_WaitData *psWaitData = (BV3D_P_WaitData *)pWaitData;

      BV3D_P_FenceWaitAsyncCleanup(hFenceArr, psWaitData);

      spin_lock_irqsave(&hFenceArr->hSpinLock, ulFlags);
      list_del(&psWaitData->sNext);
      hFenceArr->iWaitDataCount--;
      spin_unlock_irqrestore(&hFenceArr->hSpinLock, ulFlags);

      kfree(psWaitData);
   }
}

static void sync_fence_callback(struct sync_fence *sync_fence, struct sync_fence_waiter *waiter)
{
   BV3D_P_WaitData *psWaitData = container_of(waiter, BV3D_P_WaitData, sWaiter);

   atomic_set(&psWaitData->uiSignaled, 1);

/* BKNI_Printf("Signal callback on client %d, psWaitData %p\n", psWaitData->uiClientId, psWaitData); */

   /* kick worker to advance the pipeline if nothing follows (shutdown) */
   BV3D_P_WakeWorkerThread_isr(psWaitData->hV3d);
}

/***************************************************************************/
BERR_Code BV3D_P_FenceWaitAsync(
      BV3D_Handle           hV3d,
      uint32_t              uiClientId,
      int                   iFenceId,
      void                **ppWaitData)
{
   BERR_Code err = BERR_SUCCESS;
   unsigned long ulFlags;
   int status;
   BV3D_FenceArrayHandle hFenceArr = hV3d->hFences;
   BV3D_P_WaitData *psWaitData = kzalloc(sizeof(BV3D_P_WaitData), GFP_KERNEL);
   if (!psWaitData)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   psWaitData->psSyncFence = sync_fence_fdget(iFenceId);
   if (!psWaitData->psSyncFence)
   {
      BKNI_Printf("%s not a sync_fence fd, fd - %d\n", __FUNCTION__, iFenceId);
      err = BERR_INVALID_PARAMETER;
      goto end;
   }

   atomic_set(&psWaitData->uiSignaled, 0);
   psWaitData->uiClientId = uiClientId;
   psWaitData->hV3d = hV3d;

   sync_fence_waiter_init(&psWaitData->sWaiter, sync_fence_callback);
   status = sync_fence_wait_async(psWaitData->psSyncFence, &psWaitData->sWaiter);
   /* status indicates that the fence:
      < 0: is in error state
        0: is not yet signalled, callback will be made when signalled
      > 0: was already signalled, callback will not be made
   */
#if 0
   BKNI_Printf("%s sync_fence=%p name=%s, status=%s\n", __FUNCTION__,
         psWaitData->psSyncFence, psWaitData->psSyncFence->name,
         status > 0 ? "already_signaled" : ( status==0 ? "callback installed" : "error installing callback"));
#endif

   if (status != 0)
   {
      err = BERR_INVALID_PARAMETER;
      goto end;
   }

   sys_close(iFenceId);

   /* BKNI_Printf("Installing callback on client %d, psWaitData %p\n", psWaitData->uiClientId, psWaitData); */

   *ppWaitData = psWaitData;

   spin_lock_irqsave(&hFenceArr->hSpinLock, ulFlags);
   list_add_tail(&psWaitData->sNext, &hFenceArr->sWaitData);
   hFenceArr->iWaitDataCount++;

   if (hFenceArr->iWaitDataCount > 50)
   {
      struct list_head *pos, *q;
      BKNI_Printf("> 50 outstanding async wait open\n");
      list_for_each_safe(pos, q, &hFenceArr->sWaitData)
      {
         BV3D_P_WaitData *psWaitData = list_entry(pos, BV3D_P_WaitData, sNext);
         BKNI_Printf("---> %p, signalled = %s\n", psWaitData, !!atomic_read(&psWaitData->uiSignaled) ? "true" : "false");
      }
   }

   spin_unlock_irqrestore(&hFenceArr->hSpinLock, ulFlags);

   return err;

end:
   /* we've got a sync_fence reference (or error) */
   if (psWaitData->psSyncFence)
      sync_fence_put(psWaitData->psSyncFence);
   kfree(psWaitData);
   *ppWaitData = NULL;
   sys_close(iFenceId);
   return err;
}

/***************************************************************************/
bool BV3D_P_FenceIsSignalled(
   BV3D_FenceArrayHandle   hFenceArr,
   void                   *pWaitData
)
{
   if (pWaitData)
   {
      BV3D_P_WaitData *psWaitData = (BV3D_P_WaitData *)pWaitData;
      return !!atomic_read(&psWaitData->uiSignaled);
   }
   else
      return true;
}

/***************************************************************************/
void BV3D_P_FenceClientDestroy(
   BV3D_FenceArrayHandle hFenceArr,
   uint32_t              uiClientId
)
{
   BV3D_P_FenceArrayWalkAndDestroy(hFenceArr, uiClientId, false);
}
