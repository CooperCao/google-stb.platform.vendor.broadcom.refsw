/***************************************************************************
 *     Broadcom Proprietary and Confidential. (c)2015 Broadcom.  All rights reserved.
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
 *
 **************************************************************************/

#include "bstd.h"

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/file.h>

/* Android Sync Framework */
#include "sync.h"
#include "sw_sync.h"
#include "bvc5_fence_priv.h"

#define pr_info_fence pr_debug

/* these two lists are only for debug purposes, to be able to check that we've
 * signaled all created fences and cleaned up all fences we were asked to wait
 * for */
static LIST_HEAD(signal_data_list_head);
static DEFINE_SPINLOCK(signal_data_list_lock);

static LIST_HEAD(wait_data_list_head);
static DEFINE_SPINLOCK(wait_data_list_lock);

struct signal_data
{
   struct sw_sync_timeline *timeline;

   uint32_t clientId;
   struct list_head signal_data_list;
};

struct wait_data
{
   struct sync_fence *sync_fence;
   void (*callback)(void *, void *);
   void *context;
   void *param;

   struct spinlock lock_signaled;
   int signaled; /* 1 = signaled */

   struct spinlock lock_cb_done;
   bool cb_done; /* 1 = callback called */

   struct sync_fence_waiter waiter;

   uint32_t clientId;
   struct list_head wait_data_list;
};

BERR_Code BVC5_P_FenceArrayCreate(BVC5_FenceArrayHandle *handle)
{

   *handle = NULL;
   return BERR_SUCCESS;
}

void BVC5_P_FenceArrayDestroy(BVC5_FenceArrayHandle hFenceArr)
{

}

/* a new fence is created on its own timeline */
int BVC5_P_FenceCreate(BVC5_FenceArrayHandle hFenceArr, uint32_t uiClientId,
      void **dataToSignal)
{
   char name[64];
   struct sync_pt *sync_point = NULL;
   struct sync_fence *sync_fence = NULL;
   struct signal_data *signal_data = NULL;
   int fd;
   unsigned long flags;

   fd = get_unused_fd_flags(0);
   if (fd < 0)
      return -1;

   signal_data = kmalloc(sizeof *signal_data, GFP_KERNEL);
   if (signal_data == NULL)
      goto err;

   snprintf(name, sizeof(name), "bvc5_fence_%p", signal_data);
   signal_data->timeline = sw_sync_timeline_create(name);
   if (signal_data->timeline == NULL)
      goto err;

   sync_point = sw_sync_pt_create(signal_data->timeline, 1);
   if (sync_point == NULL)
      goto err;

   /* sync_fence_create takes ownership pf sync_point */
   sync_fence =  sync_fence_create(name, sync_point);
   if (sync_fence == NULL)
      goto err;
   sync_point = NULL;

   /* fd takes ownership of this fence; when fd is close, sync_fence will be released` */
   sync_fence_install(sync_fence, fd);

   *dataToSignal = signal_data;
   pr_info_fence("%s: fd=%d sync_fence=%p name=%s timeline=%p \n",
         __FUNCTION__, fd, sync_fence, sync_fence->name, signal_data->timeline);

   spin_lock_irqsave(&signal_data_list_lock, flags);
   signal_data->clientId = uiClientId;
   list_add_tail(&signal_data->signal_data_list, &signal_data_list_head);
   spin_unlock_irqrestore(&signal_data_list_lock, flags);

   return fd;
err:
   pr_err("%s:%d FAILED \n", __FUNCTION__, __LINE__);

   if (sync_point)
      sync_pt_free(sync_point);

   if (signal_data)
   {
      if (signal_data->timeline)
         sync_timeline_destroy(&signal_data->timeline->obj);

      kfree(signal_data);
   }
   put_unused_fd(fd);
   *dataToSignal = NULL;
   return -1;
}

void BVC5_P_FenceClose(BVC5_FenceArrayHandle hFenceArr, int fd)
{
   struct sync_fence *sync_fence = sync_fence_fdget(fd);

   if (!sync_fence)
   {
      pr_err("%s not a sync_fence fd, fd - %d \n", __FUNCTION__, fd);
      return;
   }
   pr_info_fence("%s fd=%d sync_fence=%p name=%s\n", __FUNCTION__, fd,
         sync_fence, sync_fence->name);
   sync_fence_put(sync_fence);
   sys_close(fd);
}

void BVC5_P_FenceSignalAndCleanup(BVC5_FenceArrayHandle hFenceArr, void *dataToSignal)
{
   struct signal_data *signal_data = (struct signal_data*)dataToSignal;
   unsigned long flags;

   pr_info_fence("%s timeline=%p \n", __FUNCTION__, signal_data->timeline);
   sw_sync_timeline_inc(signal_data->timeline, 1);

   /* we are done, destroy timeline and free signal_data */
   sync_timeline_destroy(&signal_data->timeline->obj);

   spin_lock_irqsave(&signal_data_list_lock, flags);
   list_del(&signal_data->signal_data_list);
   spin_unlock_irqrestore(&signal_data_list_lock, flags);

   kfree(signal_data);
}

static void sync_fence_callback(struct sync_fence *sync_fence, struct sync_fence_waiter *waiter)
{
   unsigned long flags;
   struct wait_data *wait_data = container_of(waiter, typeof(*wait_data), waiter);

   pr_info_fence("%s sync_fence=%p fence_name=%s\n", __FUNCTION__, sync_fence, sync_fence->name);

   spin_lock_irqsave(&wait_data->lock_signaled, flags);
   wait_data->signaled = 1;
   spin_unlock_irqrestore(&wait_data->lock_signaled, flags);

   wait_data->callback(wait_data->context, wait_data->param);

   spin_lock_irqsave(&wait_data->lock_cb_done, flags);
   wait_data->cb_done = 1;
   spin_unlock_irqrestore(&wait_data->lock_cb_done, flags);
}

int BVC5_P_FenceWaitAsync(BVC5_FenceArrayHandle hFenceArr, uint32_t uiClientId,
      int fd,
      void (*callback)(void *, void *), void *context, void *param,
      void **waitData)
{
   int status = -1;
   struct wait_data *wait_data;
   unsigned long flags;

   wait_data = kmalloc(sizeof *wait_data,GFP_KERNEL);

   if (!wait_data)
      return -ENOMEM;

   wait_data->sync_fence = sync_fence_fdget(fd);
   if (!wait_data->sync_fence)
   {
      pr_err("%s not a sync_fence fd, fd - %d \n", __FUNCTION__, fd);
      goto end;
   }

   wait_data->callback = callback;
   wait_data->context = context;
   wait_data->param = param;
   spin_lock_init(&wait_data->lock_signaled);
   wait_data->signaled = 0;

   spin_lock_init(&wait_data->lock_cb_done);
   wait_data->cb_done= 0;

   sync_fence_waiter_init(&wait_data->waiter, sync_fence_callback);
   status = sync_fence_wait_async(wait_data->sync_fence, &wait_data->waiter);
   /* status indicates that the fence:
      < 0: is in error state
        0: is not yet signalled, callback will be made when signalled
      > 0: was already signalled, callback will not be made
   */
   pr_info_fence("%s sync_fence=%p name=%s, status=%s\n", __FUNCTION__,
         wait_data->sync_fence, wait_data->sync_fence->name,
         status > 0 ? "already_signaled" : ( status==0 ? "callback installed" : "error installing callback"));

   if (status != 0)
      goto end;

   /* we've got a sync_fence reference, close the fd */
   sys_close(fd);
   *waitData = wait_data;

   spin_lock_irqsave(&wait_data_list_lock, flags);
   wait_data->clientId = uiClientId;
   list_add_tail(&wait_data->wait_data_list, &wait_data_list_head);
   spin_unlock_irqrestore(&wait_data_list_lock, flags);

   return 0;

end:
   if (wait_data->sync_fence)
      sync_fence_put(wait_data->sync_fence);
   kfree(wait_data);
   *waitData = NULL;
   return status;
}

int BVC5_P_FenceWaitAsyncIsSignaled(BVC5_FenceArrayHandle hFenceArr, void *waitData)
{
   unsigned long flags;
   int signaled = 0;
   struct wait_data *wait_data = (struct wait_data*)waitData;

   pr_info_fence("%s sync_fence=%p fence_name=%s \n", __FUNCTION__,
         wait_data->sync_fence, wait_data->sync_fence->name);

   spin_lock_irqsave(&wait_data->lock_signaled, flags);
   signaled = wait_data->signaled;
   spin_unlock_irqrestore(&wait_data->lock_signaled, flags);
   return signaled;
}

void BVC5_P_FenceWaitAsyncCleanup(BVC5_FenceArrayHandle hFenceArr, void *waitData)
{
   int status;
   unsigned long flags;
   struct wait_data *wait_data = (struct wait_data*)waitData;

   pr_info_fence("%s sync_fence=%p fence_name=%s \n", __FUNCTION__,
         wait_data->sync_fence, wait_data->sync_fence->name);

   /* returns 0 if waiter was removed from fence's async waiter list.
    * returns -ENOENT if waiter was not found on fence's async waiter list */
   status = sync_fence_cancel_async(wait_data->sync_fence, &wait_data->waiter);
   if (status == -ENOENT)
   {
      unsigned long flags;

      while(1)
      {
         spin_lock_irqsave(&wait_data->lock_cb_done, flags);
         if (wait_data->cb_done)
         {
            spin_unlock_irqrestore(&wait_data->lock_cb_done, flags);
            break;
         }
         spin_unlock_irqrestore(&wait_data->lock_cb_done, flags);

         /* the callback was not called yet, but we lost the race for removing the async waiter;
          *  give the callback a chance to be called ? how ? wait for an event ?
          */
         pr_err("%s cancel_async failed but callback not called yet sync_fence=%p name=%s\n",
               __FUNCTION__, wait_data->sync_fence, wait_data->sync_fence->name);
         /*  for the moment , just leak the waitData */
         return;
      }
   }
   sync_fence_put(wait_data->sync_fence);

   spin_lock_irqsave(&wait_data_list_lock, flags);
   list_del(&wait_data->wait_data_list);
   spin_unlock_irqrestore(&wait_data_list_lock, flags);

   kfree(wait_data);
}

void BVC5_P_FenceClientDestroy(BVC5_FenceArrayHandle hFenceArr, uint32_t uiClientId)
{
}

void BVC5_P_FenceClientCheckDestroy(BVC5_FenceArrayHandle hFenceArr, uint32_t uiClientId)
{
   unsigned long flags;
   struct list_head *pos;
   int have_signal_data  = 0;
   int have_wait_data  = 0;

   spin_lock_irqsave(&signal_data_list_lock, flags);
   list_for_each(pos, &signal_data_list_head)
   {
      struct signal_data *obj = container_of(pos, struct signal_data,
            signal_data_list);
      if (obj->clientId == uiClientId)
         have_signal_data = 1;
   }
   spin_unlock_irqrestore(&signal_data_list_lock, flags);

   if (have_signal_data == 1)
      pr_err("%s we still have unsignaled data for this client uiClientId=%d", __FUNCTION__, uiClientId);

   spin_lock_irqsave(&wait_data_list_lock, flags);
   list_for_each(pos, &wait_data_list_head)
   {
      struct wait_data *obj = container_of(pos, struct wait_data,
            wait_data_list);
      if (obj->clientId == uiClientId)
         have_wait_data = 1;

   }
   spin_unlock_irqrestore(&wait_data_list_lock, flags);

   if (have_wait_data == 1)
      pr_err("%s we still have wait_data for this client uiClientId=%d", __FUNCTION__, uiClientId);
}

/* These functions should not be called */
int BVC5_P_FenceCreateToSignalFromUser(BVC5_FenceArrayHandle hFenceArr, uint32_t uiClientId)
{
   BUG_ON(1);
}

void BVC5_P_FenceSignalFromUser(BVC5_FenceArrayHandle hFenceArr, int iFenceId)
{
   BUG_ON(1);
}

void BVC5_P_FenceAddCallback(BVC5_FenceArrayHandle hFenceArr, int iFenceId,
      void (*pfnCallback)(void *, void *), void *pContext, void *pParam)
{
   BUG_ON(1);
}

void BVC5_P_FenceRemoveCallback( BVC5_FenceArrayHandle hFenceArr, int iFenceId)
{
   BUG_ON(1);
}
