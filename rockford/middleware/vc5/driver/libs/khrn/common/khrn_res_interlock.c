/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Reference counted object for memory handle protected by an interlock
=============================================================================*/
#include "khrn_res_interlock.h"
#include "khrn_process.h"
#include "libs/platform/gmem.h"
#include "vcos.h"

static void deferred_free_callback(void *data, uint64_t job_id, v3d_sched_job_error error)
{
   KHRN_RES_INTERLOCK_T *res_i = data;

   gmem_free(res_i->handle);
   free(res_i);
}

static KHRN_RES_INTERLOCK_T* create_with_handle(gmem_handle_t handle)
{
   KHRN_RES_INTERLOCK_T *res_i = calloc(1, sizeof(KHRN_RES_INTERLOCK_T));
   if(res_i)
   {
      res_i->handle = handle;
      res_i->ref_count = 1;
      khrn_interlock_init(&res_i->interlock);
   }
   return res_i;
}

// either allocate a new resource, or wait for non-stalling write
static KHRN_RES_INTERLOCK_T* try_realloc(
   KHRN_RES_INTERLOCK_T* res,
   size_t size,
   khrn_res_interlock_gmem_args const* args
   )
{
   while (true)
   {
      gmem_handle_t handle = gmem_alloc(size, args->align, args->usage, args->desc);
      if (handle != GMEM_HANDLE_INVALID)
      {
         KHRN_RES_INTERLOCK_T *new_res = create_with_handle(handle);
         if (new_res != NULL)
            return new_res;

         gmem_free(handle);
         return NULL;
      }

      // give up if no jobs completed
      if (!v3d_scheduler_wait_any())
         break;

      // stop if write would no longer cause a stall
      if (res != NULL && !khrn_interlock_write_now_would_stall(&res->interlock))
         break;
   }

   return NULL;
}

KHRN_RES_INTERLOCK_T* khrn_res_interlock_create(size_t size, size_t align, uint32_t usage, const char *desc)
{
   khrn_res_interlock_gmem_args args;
   args.align = align;
   args.usage = usage;
   args.desc = desc;
   return try_realloc(NULL, size, &args);
}

KHRN_RES_INTERLOCK_T* khrn_res_interlock_create_with_handle(gmem_handle_t handle)
{
   assert(handle != GMEM_HANDLE_INVALID);

   return create_with_handle(handle);
}

KHRN_RES_INTERLOCK_T* khrn_res_interlock_create_no_handle(void)
{
   return create_with_handle(GMEM_HANDLE_INVALID);
}

void khrn_res_interlock_set_handle(KHRN_RES_INTERLOCK_T *res_i,
      gmem_handle_t handle)
{
   assert(res_i->handle == GMEM_HANDLE_INVALID);
   res_i->handle = handle;
}

void khrn_res_interlock_refdec(KHRN_RES_INTERLOCK_T *res_i)
{
   if (!res_i)
      return;

   int before_dec = vcos_atomic_dec(&res_i->ref_count);
   assert(before_dec > 0);

   if (before_dec == 1)
   {
      if (res_i->handle != GMEM_HANDLE_INVALID)
      {
         assert(khrn_interlock_get_actions(&res_i->interlock, NULL) == ACTION_NONE);

         /* We cannot free the handle right now, we need to wait for the jobs
          * that are using this resource to complete and for their completion
          * callbacks to be run.
          */
         v3d_scheduler_deps *deps = khrn_interlock_get_sync(&res_i->interlock, true);

         if (!v3d_scheduler_jobs_reached_state(deps, V3D_SCHED_DEPS_FINALISED, false))
         {
            //TODO: this is temporary, till we implement v3d_scheduler_gmem_free;
            //Either way, we must have to find a better way how to deffer free
            //lots of resources
            v3d_scheduler_submit_null_job(deps, deferred_free_callback, res_i);
         }
         else
         {
            gmem_free(res_i->handle);
            free(res_i);
         }
      }
      else
      {
         free(res_i);
      }
   }
}

static bool khrn_res_interlock_write_data(KHRN_RES_INTERLOCK_T *res_i,
      size_t offset, const void *data, size_t size)
{
   assert(res_i->handle != GMEM_HANDLE_INVALID);

   khrn_interlock_write_now(&res_i->interlock);

   uint32_t sync_flags = GMEM_SYNC_CPU_WRITE | GMEM_SYNC_RELAXED;
   void *dst = gmem_map_and_begin_cpu_access_range(res_i->handle, offset, size, sync_flags);
   if (!dst)
      return false;

   memcpy(dst, data, size);

   gmem_end_cpu_access_range_and_unmap(res_i->handle, offset, size, sync_flags);
   return true;
}

KHRN_RES_INTERLOCK_T *khrn_res_interlock_from_data(const void  *src,
                                                   size_t       bytes,
                                                   size_t       padding,
                                                   size_t       align,
                                                   uint32_t     usage_flags,
                                                   const char  *desc) {
   KHRN_RES_INTERLOCK_T *res_i;

   res_i = khrn_res_interlock_create(bytes + padding, align, usage_flags, desc);
   if(!res_i) {
      goto failed;
   }

   if(!khrn_res_interlock_write_data(res_i, 0, src, bytes)) {
      goto failed;
   }

   return res_i;

 failed:
   khrn_res_interlock_refdec(res_i);
   return NULL;
}

void* khrn_res_interlock_map_range(
   KHRN_RES_INTERLOCK_T** res_ptr,
   size_t offset,
   size_t size,
   unsigned map_flags,
   const khrn_res_interlock_gmem_args* rename_args
)
{
   KHRN_RES_INTERLOCK_T* res = *res_ptr;
   assert(size > 0 && (offset + size) <= gmem_get_size(res->handle));

   if (map_flags & KHRN_MAP_WRITE_BIT)
   {
      if (rename_args != NULL)
      {
         size_t res_size = gmem_get_size(res->handle);

         // if not overwriting the whole resource, then we'll need to wait for CPU read
         bool discard_all = ((map_flags & KHRN_MAP_INVALIDATE_BUFFER_BIT) != 0)
                         || ((map_flags & KHRN_MAP_INVALIDATE_RANGE_BIT) != 0 && size == res_size);
         if (!discard_all)
         {
            khrn_interlock_read_now(&res->interlock);
         }

         // if a write would cause a stall, then better to rename
         if (khrn_interlock_write_now_would_stall(&res->interlock))
         {
            KHRN_RES_INTERLOCK_T* rename = try_realloc(res, res_size, rename_args);
            if (rename != NULL)
            {
               khrn_interlock_write_now(&rename->interlock);

               char* dst = gmem_map(rename->handle);
               if (dst != NULL)
               {
                  // need to copy some data to new resource
                  char* src = NULL;
                  if (!discard_all)
                  {
                     src = (char*)gmem_map_and_begin_cpu_access(res->handle, GMEM_SYNC_CPU_READ);
                     if (src != NULL)
                     {
                        gmem_sync_pre_cpu_access(rename->handle, GMEM_SYNC_CPU_WRITE | GMEM_SYNC_DISCARD);

                        if (map_flags & KHRN_MAP_INVALIDATE_RANGE_BIT)
                        {
                           // copy the data to rename resource except the data between offset and size.
                           size_t end = offset + size;
                           memcpy(dst, src, offset);
                           memcpy(dst + end, src + end, res_size - end);
                        }
                        else
                        {
                           // copy all the data to rename.
                           memcpy(dst, src, res_size);
                        }

                        gmem_sync_post_cpu_write(rename->handle, GMEM_SYNC_CPU_WRITE | GMEM_SYNC_DISCARD);
                        gmem_end_cpu_access_and_unmap(res->handle, GMEM_SYNC_CPU_READ);
                     }
                  }

                  // if not ok, then give up with rename and fallthrough
                  if (discard_all || src != NULL)
                  {
                     // update current resource object
                     khrn_res_interlock_refdec(res);
                     *res_ptr = rename;

                     // sync needs to match with values that will be passed into khrn_res_interlock_unmap_range.
                     gmem_sync_pre_cpu_access_range(rename->handle, offset, size, GMEM_SYNC_CPU_RW | GMEM_SYNC_RELAXED);
                     return dst + offset;
                  }

                  gmem_unmap(rename->handle);
               }

               // delete rename resource and attempt fall through
               khrn_res_interlock_refdec(rename);
            }
         }
      }

      khrn_interlock_write_now(&res->interlock);
   }
   else
   {
      khrn_interlock_read_now(&res->interlock);
   }

   return gmem_map_and_begin_cpu_access_range(
      res->handle,
      offset,
      size,
      (map_flags & KHRN_MAP_WRITE_BIT ? GMEM_SYNC_CPU_WRITE : 0) | GMEM_SYNC_CPU_READ | GMEM_SYNC_RELAXED);
}

void khrn_res_interlock_unmap_range(KHRN_RES_INTERLOCK_T* res, size_t offset, size_t size, unsigned map_flags)
{
   assert(size > 0 && (offset + size) <= gmem_get_size(res->handle));

   gmem_end_cpu_access_range_and_unmap(
      res->handle,
      offset,
      size,
      (map_flags & KHRN_MAP_WRITE_BIT ? GMEM_SYNC_CPU_WRITE : 0) | GMEM_SYNC_CPU_READ | GMEM_SYNC_RELAXED);
}
