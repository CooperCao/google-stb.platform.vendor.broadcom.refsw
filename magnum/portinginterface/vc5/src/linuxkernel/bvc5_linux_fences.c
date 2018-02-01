/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bstd.h"

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/file.h>

#include "bvc5_fence_priv.h"

#include <linux/file.h>
#include <linux/fence.h>
#include <linux/sync_file.h>

BDBG_MODULE(BVC5_Fence);

//#define BVC5_FENCE_CHECK_DESTROY

#ifdef BVC5_FENCE_CHECK_DESTROY
/* list to check that we've signaled all the fences we've created and waited
 * for all the fences we were asked to wait for */
static LIST_HEAD(signal_data_list_head);
static DEFINE_SPINLOCK(signal_data_list_lock);

static LIST_HEAD(wait_data_list_head);
static DEFINE_SPINLOCK(wait_data_list_lock);
#endif

struct signal_data
{
   /* must be the first data member in the struct;
    * protected by lock;
    */
   struct fence fence;

   char name[32];

   spinlock_t      lock;
   /* protected by lock */
   u64         context;

   uint32_t clientId;
   struct list_head signal_data_list;
};

static const struct fence_ops bvc5_fence_ops;

#ifdef BVC5_FENCE_CHECK_DESTROY
static void signal_data_debug_add(struct signal_data *signal_data)
{
   unsigned long flags;
   spin_lock_irqsave(&signal_data_list_lock, flags);
   list_add_tail(&signal_data->signal_data_list, &signal_data_list_head);
   spin_unlock_irqrestore(&signal_data_list_lock, flags);
}

static void signal_data_debug_remove(struct signal_data *signal_data)
{
   unsigned long flags;
   spin_lock_irqsave(&signal_data_list_lock, flags);
   list_del(&signal_data->signal_data_list);
   spin_unlock_irqrestore(&signal_data_list_lock, flags);
}

static bool has_signaled_data(uint32_t uiClientId)
{
   struct list_head *pos;
   unsigned long flags;
   bool found = false;
   spin_lock_irqsave(&signal_data_list_lock, flags);
   list_for_each(pos, &signal_data_list_head)
   {
      struct signal_data *obj = container_of(pos, struct signal_data,
            signal_data_list);
      if (obj->clientId == uiClientId)
      {
         found = true;
         break;
      }
   }
   spin_unlock_irqrestore(&signal_data_list_lock, flags);
   return found;

}
#else
static void signal_data_debug_add(struct signal_data *signal_data) {}
static void signal_data_debug_remove(struct signal_data *signal_data) {}
static bool has_signaled_data(uint32_t uiClientId) { return false; }
#endif

struct fence *signal_data_create(uint32_t uiClientId)
{
   struct signal_data *obj = kzalloc(sizeof *obj, GFP_KERNEL);
   if (obj == NULL)
      return NULL;

   obj->context = fence_context_alloc(1);
   snprintf(obj->name, sizeof(obj->name), "bvc5_fence_%llu", obj->context);
   fence_init(&obj->fence, &bvc5_fence_ops, &obj->lock, obj->context, 1);
   spin_lock_init(&obj->lock);
   obj->clientId = uiClientId;

   signal_data_debug_add(obj);
   return &obj->fence;
}

static void signal_data_signal(struct signal_data *obj)
{
   unsigned long flags;

   spin_lock_irqsave(&obj->lock, flags);
   fence_signal_locked(&obj->fence);
   spin_unlock_irqrestore(&obj->lock, flags);
}


static inline struct signal_data *fence_parent(struct fence *fence)
{
   if (fence->ops != &bvc5_fence_ops)
      return NULL;

   return container_of(fence->lock, struct signal_data, lock);
}

static const char *bvc5_fence_get_driver_name(struct fence *fence)
{
   return "bvc5_fence";
}

static const char *bvc5_fence_get_signal_data_name(struct fence *fence)
{
   struct signal_data *parent = fence_parent(fence);
   return parent->name;
}

static void bvc5_fence_release(struct fence *fence)
{
   struct signal_data *parent = fence_parent(fence);

   //this is our last reference of signal_data
   signal_data_debug_remove(parent);
   fence_free(fence);
}


static bool bvc5_fence_enable_signaling(struct fence *fence)
{
   return true;
}

static void bvc5_fence_value_str(struct fence *fence,
      char *str, int size)
{
   struct signal_data *parent = fence_parent(fence);
   snprintf(str, size, "fence=%p context=%llu", fence, parent->context);
}

static const struct fence_ops bvc5_fence_ops = {
   .get_driver_name = bvc5_fence_get_driver_name,
   .get_timeline_name = bvc5_fence_get_signal_data_name,
   .enable_signaling = bvc5_fence_enable_signaling,
   .disable_signaling = NULL,
   .signaled = NULL,
   .wait = fence_default_wait,
   .release = bvc5_fence_release,
   .fence_value_str = bvc5_fence_value_str,
   .timeline_value_str = NULL,
};

BERR_Code BVC5_P_FenceArrayCreate(BVC5_FenceArrayHandle *handle)
{

   *handle = NULL;
   return BERR_SUCCESS;
}

void BVC5_P_FenceArrayDestroy(BVC5_FenceArrayHandle hFenceArr)
{

}

int BVC5_P_FenceCreate(BVC5_FenceArrayHandle hFenceArr, uint32_t uiClientId,
      void **dataToSignal)
{
   struct fence *fence;
   struct sync_file *sync_file;
   int fd = -1;


   *dataToSignal = NULL;
   fence = signal_data_create(uiClientId);
   if (!fence)
      return -1;

   fd = get_unused_fd_flags(0);
   if (fd < 0)
      goto err;

   sync_file = sync_file_create(fence);
   if (!sync_file)
      goto err;

   /* fd takes ownership of this sync_file; the sync_file has a reference count to fence;
    * when fd is close, sync_file_release is called and fence gets released */
   fd_install(fd, sync_file->file);

   *dataToSignal = fence;
   BDBG_MSG(("%s fence=%p\n", __FUNCTION__, fence));
   return fd;
err:
   BDBG_ERR(("%s:%d FAILED \n", __FUNCTION__, __LINE__));
   if (fd >= 0)
      put_unused_fd(fd);
   fence_put(fence);
   return -1;
}

void BVC5_P_FenceClose(BVC5_FenceArrayHandle hFenceArr, int fd)
{
   struct fence *fence = sync_file_get_fence(fd);
   if (!fence)
   {
      BDBG_ERR(("%s not a fence, fd = %d \n", __FUNCTION__, fd));
      return;
   }
   BDBG_MSG(("%s fence=%p\n", __FUNCTION__, fence));
   fence_put(fence);

   sys_close(fd);
}

void BVC5_P_FenceSignalAndCleanup(BVC5_FenceArrayHandle hFenceArr, void *dataToSignal)
{
   struct fence *fence = (struct fence*)dataToSignal;

   BDBG_MSG(("%s fence=%p\n", __FUNCTION__, fence));

   struct signal_data *parent = fence_parent(fence);
   signal_data_signal(parent);

   fence_put(fence);
}

struct wait_data
{
   struct kref kref;
   struct fence *fence;
   struct fence_cb cb;

   void (*callback)(void *, uint64_t);
   void *context;
   uint64_t param;
   uint32_t clientId;

   struct spinlock lock_cb_done;
   bool cb_done;

   struct list_head wait_data_list;
};

#ifdef BVC5_FENCE_CHECK_DESTROY
static void wait_data_debug_add(struct wait_data *obj)
{
   unsigned long flags;
   spin_lock_irqsave(&wait_data_list_lock, flags);
   list_add_tail(&obj->wait_data_list, &wait_data_list_head);
   spin_unlock_irqrestore(&wait_data_list_lock, flags);
}

static void wait_data_debug_remove(struct wait_data *obj)
{
   unsigned long flags;
   spin_lock_irqsave(&wait_data_list_lock, flags);
   list_del(&obj->wait_data_list);
   spin_unlock_irqrestore(&wait_data_list_lock, flags);
}

static bool has_wait_data(uint32_t uiClientId)
{
   unsigned long flags;
   struct list_head *pos;
   bool found = false;
   spin_lock_irqsave(&signal_data_list_lock, flags);
   list_for_each(pos, &signal_data_list_head)
   {
      struct signal_data *obj = container_of(pos, struct signal_data,
            signal_data_list);
      if (obj->clientId == uiClientId)
      {
         found = true;
         break;
      }
   }
   spin_unlock_irqrestore(&signal_data_list_lock, flags);
   return found;

}
#else
static void wait_data_debug_add(struct wait_data *obj) {}
static void wait_data_debug_remove(struct wait_data *obj) {}
static bool has_waited_data(uint32_t uiClientId) { return false;}
#endif

static struct wait_data *wait_data_create(struct fence *fence, void
      (*callback)(void *, uint64_t), void *context, uint64_t param, uint32_t
      uiClientId)
{
   struct wait_data *obj = kzalloc(sizeof(*obj),GFP_KERNEL);
   if (!obj)
      return NULL;

   kref_init(&obj->kref);
   obj->fence = fence;
   obj->callback = callback;
   obj->context = context;
   obj->param = param;
   spin_lock_init(&obj->lock_cb_done);
   obj->clientId = uiClientId;

   wait_data_debug_add(obj);
   return obj;
}

static void wait_data_free(struct kref *kref)
{
   struct wait_data *obj = container_of(kref, struct wait_data, kref);

   fence_put(obj->fence);
   wait_data_debug_remove(obj);

   kfree(obj);
}

static void wait_data_get(struct wait_data *obj)
{
   kref_get(&obj->kref);
}

static void wait_data_put(struct wait_data *obj)
{
   kref_put(&obj->kref, wait_data_free);
}

static void fence_cb_func(struct fence *fence, struct fence_cb *cb)
{
   unsigned long flags;
   struct wait_data *wait_data = container_of(cb, struct wait_data, cb);

   wait_data->callback(wait_data->context, wait_data->param);

   spin_lock_irqsave(&wait_data->lock_cb_done, flags);
   wait_data->cb_done = true;
   spin_unlock_irqrestore(&wait_data->lock_cb_done, flags);

   wait_data_put(wait_data);
}

int BVC5_P_FenceWaitAsync(BVC5_FenceArrayHandle hFenceArr, uint32_t uiClientId,
      int fd,
      void (*callback)(void *, uint64_t), void *context, uint64_t param,
      void **waitData)
{
   struct wait_data *wait_data;
   struct fence *fence;

   fence = sync_file_get_fence(fd);
   if (!fence)
   {
      BDBG_ERR(("%s not a fence, fd = %d \n", __FUNCTION__, fd));
      return -ENOMEM;
   }

   if (fence_is_signaled(fence))
   {
      BDBG_MSG(("%s fence=%p already signaled\n", __FUNCTION__, fence));
      fence_put(fence);
      return 1;
   }

   /* wait_data takes the ownership of the fence */
   wait_data = wait_data_create(fence, callback, context, param, uiClientId);
   if (!wait_data)
      return -ENOMEM;

   /* the callback will also also have reference to waitdata */
   wait_data_get(wait_data);
   if (fence_add_callback(wait_data->fence, &wait_data->cb, fence_cb_func))
   {
      /* fence already  signaled, callback was not added;
       * release both references */
      BDBG_MSG(("%s fence=%p already signaled\n", __FUNCTION__, wait_data->fence));
      wait_data_put(wait_data);
      wait_data_put(wait_data);
      *waitData = NULL;
      return 1;
   }
   else
   {
      /* callback was added, close the fd since we took ownership of the fence */
      sys_close(fd);
      *waitData = wait_data;
      BDBG_MSG(("%s fence=%p\n", __FUNCTION__, wait_data->fence));
      return 0;
   }
}

int BVC5_P_FenceWaitAsyncIsSignaled(BVC5_FenceArrayHandle hFenceArr, void *waitData)
{
   bool signaled;
   struct wait_data *wait_data = (struct wait_data*)waitData;

   signaled = fence_is_signaled(wait_data->fence);

   return (signaled == true);
}

void BVC5_P_FenceWaitAsyncCleanup( BVC5_FenceArrayHandle  hFenceArr,
      uint32_t uiClientId, void (*callback)(void *, uint64_t) /*unused*/, void *context /*unused*/,
      uint64_t param /*unused*/, void *waitData)
{
   int res;
   struct wait_data *wait_data = (struct wait_data*)waitData;

   res = fence_remove_callback(wait_data->fence, &wait_data->cb);
   if (res)
   {
      /* callback removed succeeded; put back the reference of wait_data that
       * belong to the callback */
      BDBG_MSG(("%s fence=%p callback removed\n", __FUNCTION__, wait_data->fence));
      wait_data_put(wait_data);
   }
   else
   {
      /* callback could not be removed (we lost the race to cancel it) */
      unsigned long flags;
      bool cb_done;
      spin_lock_irqsave(&wait_data->lock_cb_done, flags);
      cb_done = wait_data->cb_done;
      spin_unlock_irqrestore(&wait_data->lock_cb_done, flags);

      BDBG_MSG(("%s fence=%p cb_done=%d\n", __FUNCTION__, wait_data->fence, cb_done));

      if (!cb_done)
      {
         /* the callback was not called yet, but we lost the race for removing it */
         BDBG_ERR(("%s cancel_async failed but callback not called yet, fence=%p\n",
                  __FUNCTION__, wait_data->fence));
      }
   }
   // release our ref
   wait_data_put(wait_data);
}

void BVC5_P_FenceClientCheckDestroy(BVC5_FenceArrayHandle hFenceArr, uint32_t uiClientId)
{
#ifdef BVC5_FENCE_CHECK_DESTROY
   if (has_signaled_data(uiClientId))
      BDBG_ERR(("%sclient %d didn't signal all the fences", __FUNCTION__, uiClientId));

   if (has_wait_data(uiClientId))
      BDBG_ERR(("%s client %d didn't wait for all the fences", __FUNCTION__, uiClientId));
#endif
}

void BVC5_P_FenceClientDestroy(BVC5_FenceArrayHandle hFenceArr, uint32_t uiClientId)
{
}

/* These functions should not be called */
int BVC5_P_FenceCreateToSignalFromUser(BVC5_FenceArrayHandle hFenceArr, uint32_t uiClientId)
{
   WARN_ON(1);
   return -1;
}

int BVC5_P_FenceKeep(BVC5_FenceArrayHandle hFenceArr, int iFenceId)
{
   WARN_ON(1);
   return -1;
}

void BVC5_P_FenceSignalFromUser(BVC5_FenceArrayHandle hFenceArr, int iFenceId)
{
   WARN_ON(1);
}

void BVC5_P_FenceAddCallback(BVC5_FenceArrayHandle hFenceArr, int iFenceId,
      uint32_t uiClientId, void (*pfnCallback)(void *, uint64_t), void *pContext, uint64_t param)
{
   WARN_ON(1);
}

bool BVC5_P_FenceRemoveCallback( BVC5_FenceArrayHandle hFenceArr, int iFenceId,
      uint32_t uiClientId, void (*pfnCallback)(void *, uint64_t), void *pContext, uint64_t param)
{
   WARN_ON(1);
   return true;
}
