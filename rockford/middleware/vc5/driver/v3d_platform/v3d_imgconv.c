/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Image format conversion

FILE DESCRIPTION
Sends jobs to the scheduler to convert between different image formats
=============================================================================*/

#include "v3d_imgconv.h"
#include "v3d_imgconv_internal.h"
#include "v3d_platform.h"

#include "helpers/v3d/v3d_align.h"
#include "helpers/gfx/gfx_util.h"

VCOS_LOG_CAT_T v3d_imgconv_log = VCOS_LOG_INIT("v3d_imgconv", VCOS_LOG_WARN);
#define VCOS_LOG_CATEGORY (&v3d_imgconv_log)

size_t v3d_imgconv_base_size(const struct v3d_imgconv_base_tgt *base)
{
   size_t size = 0;
   for (unsigned p = 0; p < base->desc.num_planes; p++)
      size = GFX_MAX(base->desc.planes[p].offset + base->plane_sizes[p],
            size);
   return size;
}

void* v3d_imgconv_gmem_tgt_to_ptr(gmem_cpu_sync_list* sync_list,
                                  const struct v3d_imgconv_gmem_tgt *tgt,
                                  unsigned int offset, bool write)
{
   offset += tgt->base.start_elem * tgt->base.array_pitch;

   void *ptr = gmem_map(tgt->handle);
   if (ptr)
   {
      /* sync per plane range */
      for (unsigned p = 0; p < tgt->base.desc.num_planes; p++)
      {
         gmem_cpu_sync_list_add_range(
            sync_list,
            tgt->handle,
            offset + tgt->base.desc.planes[p].offset,
            tgt->base.plane_sizes[p],
            (write ? GMEM_SYNC_CPU_WRITE : GMEM_SYNC_CPU_READ) | GMEM_SYNC_RELAXED);
      }

      ptr = (char*)ptr + offset;
   }
   return ptr;
}

static void init_base_tgt(struct v3d_imgconv_base_tgt *tgt,
      const GFX_BUFFER_DESC_T *desc,
      unsigned int x, unsigned int y, unsigned int z, unsigned int start_elem,
      unsigned int array_pitch)
{
   tgt->desc = *desc;
   tgt->x = x;
   tgt->y = y;
   tgt->z = z;
   tgt->start_elem = start_elem;
   tgt->array_pitch = array_pitch;

   for (unsigned p = 0; p < tgt->desc.num_planes; p++)
      tgt->plane_sizes[p] = gfx_buffer_size_plane(desc, p);
}

void v3d_imgconv_init_gmem_tgt(struct v3d_imgconv_gmem_tgt *tgt,
      gmem_handle_t handle, const v3d_scheduler_deps *deps,
      const GFX_BUFFER_DESC_T *desc,
      unsigned int x, unsigned int y, unsigned int z, unsigned int start_elem,
      unsigned int array_pitch)
{
   init_base_tgt(&tgt->base, desc, x, y, z, start_elem, array_pitch);
   tgt->handle = handle;
   tgt->deps = *deps;
}

void v3d_imgconv_init_ptr_tgt(struct v3d_imgconv_ptr_tgt *tgt, void *ptr,
      const GFX_BUFFER_DESC_T *desc,
      unsigned int x, unsigned int y, unsigned int z, unsigned int start_elem,
      unsigned int array_pitch)
{
   init_base_tgt(&tgt->base, desc, x, y, z, start_elem, array_pitch);
   tgt->data = ptr;
}

#define MAX_PATHS 6
static const v3d_imgconv_methods* conv_path[MAX_PATHS];
static const v3d_imgconv_methods* conv_path_sync[MAX_PATHS];
static VCOS_ONCE_T paths_initialized = VCOS_ONCE_INIT;

static void init_conv_paths(void)
{
   memset((void*)conv_path, 0, sizeof(conv_path));
   memset((void*)conv_path_sync, 0, sizeof(conv_path_sync));
   unsigned i;

   #define ADD_PATH(methods, i, path) \
   do {                               \
      (methods)[i] = (path);          \
      if ((methods)[i] != NULL)       \
         (i)++;                       \
   } while(0)

   /*
    * Priority list where either sync and async algorithm is allowed.
    * For methods defining both async and sync algorithm the async one is used.
    *
    * conv_path is used for gmem->gmem conversions
    */
   i = 0;
   ADD_PATH(conv_path, i, get_tfu_path());
   ADD_PATH(conv_path, i, get_yv12_tfu_path());  /* YV12->Y/CbCr + async */
   ADD_PATH(conv_path, i, get_neon_path());
   ADD_PATH(conv_path, i, get_extra_neon_path());
   ADD_PATH(conv_path, i, get_c_path());
   ADD_PATH(conv_path, i, get_gfx_blit_path());
   assert(i > 0 && i <= MAX_PATHS);

   /*
    *  Priority list where sync algorithm is required. The asynchronous
    *  algorithms on this list will either require an additional call to
    *  memcpy (cpu --> gmem) or will be skipped (gmem --> cpu). This may change
    *  the priority order.
    *
    * conv_path_sync is used for cpu->gmem conversions
    */
   i = 0;
   ADD_PATH(conv_path_sync, i, get_neon_path());
   ADD_PATH(conv_path_sync, i, get_extra_neon_path());
   ADD_PATH(conv_path_sync, i, get_memcpy_tfu_path()); /* memcpy + async */
   ADD_PATH(conv_path_sync, i, get_c_path());
   ADD_PATH(conv_path_sync, i, get_gfx_blit_path());
   assert(i > 0 && i <= MAX_PATHS);

   #undef ADD_PATH
}

static bool call_sync_func(convert_sync_t convert_func,
      const struct v3d_imgconv_gmem_tgt *dst, unsigned int dst_off,
      const struct v3d_imgconv_gmem_tgt *src, unsigned int src_off,
      v3d_scheduler_deps *deps, unsigned int width, unsigned int height,
      unsigned int depth)
{
   bool ok = false;
   void *src_ptr = NULL;
   void *dst_ptr = NULL;
   gmem_cpu_sync_list src_sync_list;
   gmem_cpu_sync_list dst_sync_list;

   v3d_scheduler_wait_jobs(deps, V3D_SCHED_DEPS_COMPLETED);

   gmem_cpu_sync_list_init(&src_sync_list);
   gmem_cpu_sync_list_init(&dst_sync_list);

   dst_ptr = v3d_imgconv_gmem_tgt_to_ptr(&dst_sync_list, dst, dst_off, true);
   if (dst_ptr == NULL)
      goto done;

   src_ptr = v3d_imgconv_gmem_tgt_to_ptr(&src_sync_list, src, src_off, false);
   if (src_ptr == NULL)
      goto done;

   gmem_sync_pre_cpu_access_list(&src_sync_list);
   gmem_sync_pre_cpu_access_list(&dst_sync_list);

   convert_func(&dst->base, dst_ptr, &src->base, src_ptr, width,
      height, depth);

   gmem_sync_post_cpu_write_list(&dst_sync_list);

   ok = true;

done:
   gmem_cpu_sync_list_destroy(&src_sync_list);
   gmem_cpu_sync_list_destroy(&dst_sync_list);

   if (src_ptr != NULL)
      gmem_unmap(src->handle);
   if (dst_ptr != NULL)
      gmem_unmap(dst->handle);

   return ok;
}

static bool call_prep_func_gmem(convert_prep_t prep_func,
                           struct v3d_imgconv_gmem_tgt *dst,
                           const struct v3d_imgconv_gmem_tgt *src, unsigned int src_off,
                           unsigned int width, unsigned int height,
                           unsigned int depth, v3d_sched_completion_fn *completion_fn,
                           void **completion_data)
{
   bool ok = false;

   /* Wait for any src buffer dependencies before reading it.
    * The destination is always a scratch buffer with no deps. */
   v3d_scheduler_deps src_deps;
   v3d_scheduler_copy_deps(&src_deps, &src->deps);
   v3d_scheduler_wait_jobs(&src_deps, V3D_SCHED_DEPS_COMPLETED);

   gmem_cpu_sync_list src_sync_list;
   gmem_cpu_sync_list_init(&src_sync_list);

   void *src_data = v3d_imgconv_gmem_tgt_to_ptr(&src_sync_list, src, src_off, false);
   if ((ok = (src_data != NULL)))
   {
      gmem_sync_pre_cpu_access_list(&src_sync_list);

      /* Run the prep function now. This will create the correct dst storage, and do the conversion.
       * We must remember to free the storage in dst when we're done using it. */
      ok = prep_func(dst, &src->base, (uint8_t*)src_data + src_off, width, height, depth,
                     completion_fn, completion_data);

      gmem_unmap(src->handle);
   }

   gmem_cpu_sync_list_destroy(&src_sync_list);

   return ok;
}

static bool call_prep_func_ptr(convert_prep_t prep_func,
                           struct v3d_imgconv_gmem_tgt *dst,
                           const struct v3d_imgconv_ptr_tgt *src, unsigned int src_off,
                           unsigned int width, unsigned int height,
                           unsigned int depth, v3d_sched_completion_fn *completion_fn,
                           void **completion_data)
{
   bool ok = false;
   void *src_ptr = NULL;

   src_ptr = src->data;
   if (src_ptr == NULL)
      goto done;

   /* Run the prep function now. This will create the correct dst storage, and do the conversion.
    * We must remember to free the storage in dst when we're done using it. */
   if (!prep_func(dst, &src->base, (uint8_t*)src_ptr + src_off, width, height, depth,
                  completion_fn, completion_data))
      goto done;

   ok = true;

done:
   return ok;
}

/* Convert an image from gmem to gmem (can use hardware) */
bool v3d_imgconv_copy(
      const struct v3d_imgconv_gmem_tgt *dst,
      const struct v3d_imgconv_gmem_tgt *src,
      uint64_t *job_id,
      unsigned int width,
      unsigned int height,
      unsigned int depth,
      unsigned int num_elems)
{
   v3d_scheduler_deps      out_deps;
   bool                    ok = false;

   /* Setup the output deps for the null job that depends on all other jobs */
   memset(&out_deps, 0, sizeof(out_deps));

   vcos_once(&paths_initialized, init_conv_paths);

   for (unsigned i = 0; i < num_elems; i++)
   {
      unsigned dst_off = dst->base.array_pitch * i;
      unsigned src_off = src->base.array_pitch * i;

      bool  conv_done = false;
      for (unsigned p = 0; !conv_done && p < MAX_PATHS; p++)
      {
         if (conv_path[p]->claim(&dst->base, &src->base, width, height, depth))
         {
            uint64_t cur_job = 0;
            if (conv_path[p]->convert_async != NULL)
            {
               v3d_sched_completion_fn     completion_fn = NULL;
               void                        *completion_data = NULL;

               if (conv_path[p]->convert_prep != NULL)
               {
                  /* Make a scratch buffer for our intermediate prep stage.
                   * call_prep_func will create the scratch buffer of the correct format
                   * for the following async conversion */
                  struct v3d_imgconv_gmem_tgt scratch;

                  if (!call_prep_func_gmem(conv_path[p]->convert_prep, &scratch, src, src_off,
                                           width, height, depth, &completion_fn, &completion_data))
                     continue;   // Try next algorithm

                  /* Set up the dest buffer deps to be used by the aync job */
                  v3d_scheduler_deps dst_deps;
                  v3d_scheduler_copy_deps(&dst_deps, &dst->deps);

                  /* start async task to convert the gmem copy */
                  src_off = 0;
                  if (!conv_path[p]->convert_async(dst, dst_off, &scratch, src_off,
                                                  &dst_deps, &cur_job, width, height, depth))
                  {
                     /* Failed, so cleanup immediately */
                     completion_fn(completion_data, 0, BCM_SCHED_JOB_ERROR);
                     completion_fn = NULL;
                     cur_job = 0;
                     continue;
                  }
               }
               else
               {
                  /* No intermediate step required, just fire the async conversion */
                  v3d_scheduler_deps src_and_dst_deps;
                  v3d_scheduler_copy_deps(&src_and_dst_deps, &src->deps);
                  v3d_scheduler_merge_deps(&src_and_dst_deps, &dst->deps);

                  if (!conv_path[p]->convert_async(dst, dst_off, src, src_off,
                     &src_and_dst_deps, &cur_job, width, height, depth))
                     goto end;
               }

               if (cur_job != 0)
                  v3d_scheduler_add_dep(&out_deps, cur_job);

               /* If we have a completion function we must have a prep stage that needs
                * to be cleaned up after the async conversion. We'll add a null job
                * with the completion handler to take care of that. */
               if (completion_fn != NULL)
                  v3d_scheduler_submit_null_job(&out_deps, completion_fn, completion_data);
            }
            else
            {
               v3d_scheduler_deps src_and_dst_deps;
               v3d_scheduler_copy_deps(&src_and_dst_deps, &src->deps);
               v3d_scheduler_merge_deps(&src_and_dst_deps, &dst->deps);

               assert(conv_path[p]->convert_sync); /* we must have one convert fct */
               if (!call_sync_func(conv_path[p]->convert_sync, dst, dst_off, src, src_off,
                                   &src_and_dst_deps, width, height, depth))
                  goto end;
            }
            conv_done = true;
         }
      }
      assert(conv_done);
   }
   ok = true;

end:
   /* Create a job to wait on all the created jobs */
   if (out_deps.n > 1)
      *job_id = v3d_scheduler_submit_null_job(&out_deps, NULL, NULL);
   else
      *job_id = out_deps.dependency[0];
   return ok;
}

static bool call_async_func(
      convert_async_t convert_func, convert_prep_t convert_prep,
      const struct v3d_imgconv_gmem_tgt *dst, unsigned int dst_off,
      const struct v3d_imgconv_ptr_tgt *src, unsigned int src_off,
      uint64_t *job_id, unsigned int width, unsigned int height, unsigned int depth)
{
   bool                    ok = false;
   v3d_scheduler_deps      dst_deps, out_deps;
   v3d_sched_completion_fn completion_fn = NULL;
   void                    *completion_data = NULL;

   memset(&out_deps, 0, sizeof(out_deps));

   v3d_scheduler_copy_deps(&dst_deps, &dst->deps);

   /* Must have a prep function to get the data ready for async conversion */
   assert(convert_prep != NULL);

   /* Make a scratch buffer for our intermediate prep stage.
   * call_prep_func will create the scratch buffer of the correct format
   * for the following async conversion */
   struct v3d_imgconv_gmem_tgt scratch;

   if (!call_prep_func_ptr(convert_prep, &scratch, src, src_off,
                           width, height, depth, &completion_fn, &completion_data))
      goto end;

   assert(scratch.handle != GMEM_HANDLE_INVALID);

   /* start async task to convert the gmem copy */
   src_off = 0; /* we only copied one element to scratch */
   uint64_t cur_job;
   src_off = 0;
   if (convert_func(dst, dst_off, &scratch, src_off,
                    &dst_deps, &cur_job, width, height, depth))
   {
      /* Create a null job to wait on the created job and free the scratch */
      assert(cur_job != 0);
      v3d_scheduler_add_dep(&out_deps, cur_job);
      *job_id = v3d_scheduler_submit_null_job(&out_deps, completion_fn, completion_data);
      ok = true;
   }
   else
   {
      completion_fn(completion_data, 0, BCM_SCHED_JOB_ERROR);
      *job_id = 0;
   }

end:
   return ok;
}

/* Convert an image in CPU memory into gmem
 * (cannot use hardware without an intermediate prep stage)
 */
bool v3d_imgconv_copy_from_ptr(
      const struct v3d_imgconv_gmem_tgt *dst,
      const struct v3d_imgconv_ptr_tgt *src,
      uint64_t *job_id,
      unsigned int width,
      unsigned int height,
      unsigned int depth,
      unsigned int num_elems)
{
   bool deps_completed = false;
   bool ok = false;
   uint64_t cur_job = 0;

   vcos_once(&paths_initialized, init_conv_paths);

   v3d_scheduler_deps out_deps;
   memset(&out_deps, 0, sizeof(out_deps));

   for (unsigned i = 0; i < num_elems; i++)
   {
      unsigned dst_off = dst->base.array_pitch * i;
      unsigned src_off = src->base.array_pitch * i;

      bool conv_done = false;
      for (unsigned p = 0; !conv_done && p < MAX_PATHS; p++)
      {
         if (conv_path_sync[p]->claim(&dst->base, &src->base, width, height, depth))
         {
            if (conv_path_sync[p]->convert_async != NULL)
            {
               /* make a gmem copy and start async task to convert it later */
               if (!call_async_func(conv_path_sync[p]->convert_async,
                                    conv_path_sync[p]->convert_prep,
                                    dst, dst_off, src, src_off,
                                    &cur_job, width, height, depth))
                  continue; /* try next algorithm if scratch can't be created */
               assert(cur_job != 0);
               v3d_scheduler_add_dep(&out_deps, cur_job);
            }
            else
            {
               assert(conv_path_sync[p]->convert_sync); /* we must have one convert fct */

               /* wait for pending jobs and start synchronous conversion */
               if (!deps_completed)
               {
                  v3d_scheduler_deps dst_deps;
                  v3d_scheduler_copy_deps(&dst_deps, &dst->deps);
                  v3d_scheduler_wait_jobs(&dst_deps, V3D_SCHED_DEPS_COMPLETED);
                  deps_completed = true;
               }

               gmem_cpu_sync_list dst_sync_list;
               gmem_cpu_sync_list_init(&dst_sync_list);

               void *dst_data = v3d_imgconv_gmem_tgt_to_ptr(&dst_sync_list, dst, dst_off, true);
               if ((ok = (dst_data != NULL)))
               {
                  gmem_sync_pre_cpu_access_list(&dst_sync_list);

                  ok = conv_path_sync[p]->convert_sync(&dst->base, dst_data, &src->base,
                     (uint8_t*)src->data + src_off, width, height, depth);

                  gmem_sync_post_cpu_write_list(&dst_sync_list);

                  gmem_unmap(dst->handle);
               }

               gmem_cpu_sync_list_destroy(&dst_sync_list);

               if (!ok)
                  goto end;
            }
            conv_done = true;
         }
      }
      assert(conv_done);
   }

   ok = true;
end:
   /* Create a job to wait on all the created jobs and free scratch */
   if (out_deps.n > 1)
      *job_id = v3d_scheduler_submit_null_job(&out_deps, NULL, NULL);
   else
      *job_id = cur_job;
   return ok;
}

/* Convert from gmem into CPU memory (cannot used hardware) */
bool v3d_imgconv_copy_to_ptr(
      const struct v3d_imgconv_ptr_tgt *dst,
      const struct v3d_imgconv_gmem_tgt *src,
      unsigned int width,
      unsigned int height,
      unsigned int depth,
      unsigned int num_elems)
{
   bool ok = false;

   vcos_once(&paths_initialized, init_conv_paths);

   v3d_scheduler_deps deps;
   v3d_scheduler_copy_deps(&deps, &src->deps);
   v3d_scheduler_wait_jobs(&deps, V3D_SCHED_DEPS_COMPLETED);

   for (unsigned i = 0; i < num_elems; i++)
   {
      unsigned dst_off = dst->base.array_pitch * i;
      unsigned src_off = src->base.array_pitch * i;

      bool  conv_done = false;
      for (unsigned p = 0; !conv_done && p < MAX_PATHS; p++)
      {
         if (conv_path_sync[p]->convert_sync == NULL)
            continue;
         if (conv_path_sync[p]->claim(&dst->base, &src->base, width, height, depth))
         {
            gmem_cpu_sync_list src_sync_list;
            gmem_cpu_sync_list_init(&src_sync_list);

            void *src_data = v3d_imgconv_gmem_tgt_to_ptr(&src_sync_list, src, src_off, false);
            if ((ok = (src_data != NULL)))
            {
               gmem_sync_pre_cpu_access_list(&src_sync_list);

               ok = conv_path_sync[p]->convert_sync(&dst->base, (uint8_t*)dst->data + dst_off,
                  &src->base, src_data, width, height, depth);

               gmem_unmap(src->handle);
            }

            gmem_cpu_sync_list_destroy(&src_sync_list);

            if (!ok)
               goto end;
            conv_done = true;
         }
      }
      assert(conv_done);
   }

   ok = true;
end:
   return ok;
}
