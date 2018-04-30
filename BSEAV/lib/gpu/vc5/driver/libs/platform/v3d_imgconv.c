/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_imgconv.h"
#include "v3d_imgconv_internal.h"
#include "v3d_platform.h"

#include "libs/core/v3d/v3d_align.h"
#include "libs/util/gfx_util/gfx_util.h"

typedef struct gmem_to_gmem_job_data
{
   v3d_imgconv_gmem_tgt src;
   v3d_imgconv_gmem_tgt dst;
   void                 *src_ptr;
   void                 *dst_ptr;
   unsigned int         src_offset;
   unsigned int         dst_offset;
   unsigned int         width;
   unsigned int         height;
   unsigned int         depth;
   unsigned int         path;
} gmem_to_gmem_job_data;

static void build_conv_info(bool secure_context, const gmem_handle_t src[GFX_BUFFER_MAX_PLANES],
                            const gmem_handle_t dst[GFX_BUFFER_MAX_PLANES], conversion_info_t *info)
{
   assert(info != NULL);

   info->secure_context = secure_context;
   info->secure_src     = false;
   info->secure_dst     = false;
   info->contiguous_src = true;
   info->contiguous_dst = true;

   for (uint32_t p = 0; p < GFX_BUFFER_MAX_PLANES; p++)
   {
      if (src != NULL && src[p] != GMEM_HANDLE_INVALID)
      {
         info->secure_src     = info->secure_src     || (gmem_get_usage(src[p]) & GMEM_USAGE_SECURE);
         info->contiguous_src = info->contiguous_src && (gmem_get_usage(src[p]) & GMEM_USAGE_CONTIGUOUS);
      }

      if (dst != NULL && dst[p] != GMEM_HANDLE_INVALID)
      {
         info->secure_dst     = info->secure_dst     || (gmem_get_usage(dst[p]) & GMEM_USAGE_SECURE);
         info->contiguous_dst = info->contiguous_dst && (gmem_get_usage(dst[p]) & GMEM_USAGE_CONTIGUOUS);
      }
   }
}

bool v3d_imgconv_valid_hw_conv_info(const conversion_info_t *info)
{
   if (info->secure_context)
   {
      if (!info->secure_dst)
         return false;
   }
   else
   {
      if (info->secure_dst || info->secure_src)
         return false;
   }

   return true;
}

bool v3d_imgconv_valid_cpu_conv_info(const conversion_info_t *info)
{
   return !(info->secure_dst || info->secure_src);
}

size_t v3d_imgconv_base_size(const struct v3d_imgconv_base_tgt *base)
{
   size_t size = 0;
   for (unsigned p = 0; p < base->desc.num_planes; p++)
      size = GFX_MAX(base->desc.planes[p].offset + base->plane_sizes[p],
            size);
   return size;
}

static void *v3d_imgconv_map_pre_cpu_access(
   const v3d_imgconv_gmem_tgt *tgt,
   size_t offset)
{
   assert(gfx_buffer_single_region(&tgt->base.desc));

   void *ptr = gmem_map_and_get_ptr(tgt->handles[0]);
   if (!ptr)
      return NULL;

   offset += tgt->offset + tgt->base.start_elem*tgt->base.array_pitch;

   return (char*)ptr + offset;
}

static void v3d_imgconv_sync_pre_cpu_access(
   const v3d_imgconv_gmem_tgt *tgt,
   size_t offset)
{
   offset += tgt->offset + tgt->base.start_elem*tgt->base.array_pitch;

   /* sync per plane range */
   for (unsigned p = 0; p < tgt->base.desc.num_planes; p++)
   {
      const GFX_BUFFER_DESC_PLANE_T *plane = &tgt->base.desc.planes[p];
      assert(plane->region < countof(tgt->handles));
      gmem_invalidate_mapped_range(
         tgt->handles[plane->region],
         plane->offset + offset,
         tgt->base.plane_sizes[p]);
   }
}

static void *v3d_imgconv_map_and_sync_pre_cpu_access(
   const v3d_imgconv_gmem_tgt *tgt,
   size_t offset)
{
   void *ptr = v3d_imgconv_map_pre_cpu_access(tgt, offset);
   if (!ptr)
      return NULL;

   v3d_imgconv_sync_pre_cpu_access(tgt, offset);

   return ptr;
}

static void v3d_imgconv_sync_post_cpu_write(
   const v3d_imgconv_gmem_tgt *tgt,
   size_t offset)
{
   offset += tgt->offset + tgt->base.start_elem*tgt->base.array_pitch;

   /* sync per plane range */
   for (unsigned p = 0; p < tgt->base.desc.num_planes; p++)
   {
      const GFX_BUFFER_DESC_PLANE_T *plane = &tgt->base.desc.planes[p];
      assert(plane->region < countof(tgt->handles));
      gmem_flush_mapped_range(
         tgt->handles[plane->region],
         plane->offset + offset,
         tgt->base.plane_sizes[p]);
   }
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
   tgt->conversion = V3D_IMGCONV_CONVERSION_FORMAT;

   for (unsigned p = 0; p < tgt->desc.num_planes; p++)
      tgt->plane_sizes[p] = gfx_buffer_size_plane(desc, p);
}

void v3d_imgconv_init_gmem_tgt_multi_region(struct v3d_imgconv_gmem_tgt *tgt,
      unsigned num_handles, const gmem_handle_t *handles, size_t offset,
      const v3d_scheduler_deps *deps, const GFX_BUFFER_DESC_T *desc,
      unsigned int x, unsigned int y, unsigned int z, unsigned int start_elem,
      unsigned int array_pitch)
{
   init_base_tgt(&tgt->base, desc, x, y, z, start_elem, array_pitch);
   assert(num_handles <= countof(tgt->handles));
   for (unsigned i = 0; i != countof(tgt->handles); ++i)
      tgt->handles[i] = (i < num_handles) ? handles[i] : GMEM_HANDLE_INVALID;
   tgt->offset = offset;
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

static void add_path(const v3d_imgconv_methods **list, bool is_sync_list,
      unsigned *pos, const v3d_imgconv_methods *path)
{
   if (path != NULL)
   {
#ifndef NDEBUG
      /* pointer functions that need to be filled */
      assert(path->claim);
      assert(path->convert_async || path->convert_sync);
      if (is_sync_list)
      {
         assert((path->convert_async && path->convert_prep) ||
                 path->convert_sync);
      }
#endif
      list[*pos] = path;
      *pos += 1;
   }
}

#define MAX_PATHS 8
static const v3d_imgconv_methods* conv_path[MAX_PATHS];
static const v3d_imgconv_methods* conv_path_sync[MAX_PATHS];
static VCOS_ONCE_T paths_initialized = VCOS_ONCE_INIT;

static void init_conv_paths(void)
{
   memset((void*)conv_path, 0, sizeof(conv_path));
   memset((void*)conv_path_sync, 0, sizeof(conv_path_sync));
   unsigned i;

   /*
    * Priority list where either sync and async algorithm is allowed.
    * For methods defining both async and sync algorithm the async one is used.
    *
    * conv_path is used for gmem->gmem conversions
    */
   i = 0;
   add_path(conv_path, false, &i, get_tfu_path());
   add_path(conv_path, false, &i, get_m2mc_path());
   add_path(conv_path, false, &i, get_sand_m2mc_tfu_path()); /* SAND->RGBA   + async */
   add_path(conv_path, false, &i, get_yv12_tfu_path());      /* YV12->Y/CbCr + async */
   add_path(conv_path, false, &i, get_neon_path());
   add_path(conv_path, false, &i, get_extra_neon_path());
   add_path(conv_path, false, &i, get_c_path());
   add_path(conv_path, false, &i, get_gfx_blit_path());
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
   add_path(conv_path_sync, true, &i, get_neon_path());
   add_path(conv_path_sync, true, &i, get_extra_neon_path());
   add_path(conv_path_sync, true, &i, get_memcpy_tfu_path()); /* memcpy + async */
   add_path(conv_path_sync, true, &i, get_c_path());
   add_path(conv_path_sync, true, &i, get_gfx_blit_path());
   assert(i > 0 && i <= MAX_PATHS);
}

static bool call_prep_func_gmem(convert_prep_t prep_func,
                           struct v3d_imgconv_gmem_tgt *dst,
                           const struct v3d_imgconv_gmem_tgt *src, unsigned int src_off,
                           unsigned int width, unsigned int height,
                           unsigned int depth, v3d_sched_completion_fn *completion_fn,
                           void **completion_data)
{
   /* Wait for any src buffer dependencies before reading it.
    * The destination is always a scratch buffer with no deps. */
   v3d_scheduler_deps src_deps;
   v3d_scheduler_copy_deps(&src_deps, &src->deps);
   v3d_scheduler_wait_jobs(&src_deps, V3D_SCHED_DEPS_COMPLETED);

   void *src_data = v3d_imgconv_map_and_sync_pre_cpu_access(src, src_off);
   if (!src_data)
      return false;

   /* Run the prep function now. This will create the correct dst storage, and do the conversion.
      * We must remember to free the storage in dst when we're done using it. */
   return prep_func(
      dst,
      &src->base, (uint8_t*)src_data + src_off,
      width, height, depth,
      completion_fn, completion_data);
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

/* Helper for cpu based gmem to gmem conversion.
 * Will be called from a user-mode callback on another thread.
 */
static void cpu_convert_gmem_to_gmem_now(void *data)
{
   gmem_to_gmem_job_data *d = (gmem_to_gmem_job_data*)data;

   v3d_imgconv_sync_pre_cpu_access(&d->src, d->src_offset);
   v3d_imgconv_sync_pre_cpu_access(&d->dst, d->dst_offset);

   conv_path[d->path]->convert_sync(&d->dst.base, d->dst_ptr, &d->src.base, d->src_ptr,
                                    d->width, d->height, d->depth);

   v3d_imgconv_sync_post_cpu_write(&d->dst, d->dst_offset);

   free(data);
}

/* Convert from gmem to gmem using the CPU.
 * Sets a job_id to wait on for the conversion to complete.
 * It will always schedule a user-mode job and do it asynchronously to avoid
 * blocking here.
 */
static bool cpu_convert_gmem_to_gmem(
   uint64_t *job_id,
   const v3d_imgconv_gmem_tgt *dst,
   const v3d_imgconv_gmem_tgt *src,
   unsigned int dst_off,
   unsigned int src_off,
   unsigned int width,
   unsigned int height,
   unsigned int depth,
   unsigned int path_to_use)
{
   /* Make a deep copy of the job parameters */
   gmem_to_gmem_job_data *job_data = malloc(sizeof(gmem_to_gmem_job_data));
   if (job_data == NULL)
      return false;

   v3d_scheduler_deps src_and_dst_deps;
   v3d_scheduler_copy_deps(&src_and_dst_deps, &src->deps);
   v3d_scheduler_merge_deps(&src_and_dst_deps, &dst->deps);

   job_data->src        = *src;
   job_data->src_offset = src_off;
   job_data->dst        = *dst;
   job_data->dst_offset = dst_off;
   job_data->width      = width;
   job_data->height     = height;
   job_data->depth      = depth;
   job_data->path       = path_to_use;

   job_data->dst_ptr = v3d_imgconv_map_pre_cpu_access(dst, dst_off);
   if (job_data->dst_ptr == NULL)
      goto end;

   job_data->src_ptr = v3d_imgconv_map_pre_cpu_access(src, src_off);
   if (job_data->src_ptr == NULL)
      goto end;

   *job_id = v3d_scheduler_submit_usermode_job(&src_and_dst_deps, cpu_convert_gmem_to_gmem_now, job_data);
   return true;

end:
   free(job_data);
   return false;
}

/* Determine gmem usage flags for a potential conversion */
gmem_usage_flags_t v3d_imgconv_calc_dst_gmem_usage(
   const v3d_imgconv_base_tgt *dst_base,
   const v3d_imgconv_base_tgt *dst_base_contiguous,
   const v3d_imgconv_gmem_tgt *src,
   bool secure_context)
{
   vcos_once(&paths_initialized, init_conv_paths);

   gmem_handle_t dst_handles[GFX_BUFFER_MAX_PLANES] = { 0 };

   conversion_info_t info;
   build_conv_info(secure_context, src->handles, dst_handles, &info);
   info.secure_dst = info.secure_src;

   gmem_usage_flags_t ret = info.secure_src ? GMEM_USAGE_SECURE : GMEM_USAGE_NONE;

   /* Run over all the claims, favoring non-contiguous memory first, then contiguous */
   for (unsigned p = 0; p < MAX_PATHS && conv_path[p]; p++)
   {
      /* Can we claim with non-contiguous dst memory? */
      info.contiguous_dst = false;
      if (conv_path[p]->claim(dst_base, &src->base, src->base.desc.width, src->base.desc.height,
                              src->base.desc.depth, &info))
      {
         return ret;
      }
      else
      {
         /* Does a contiguous memory claim work? */
         info.contiguous_dst = true;
         if (conv_path[p]->claim(dst_base_contiguous, &src->base, src->base.desc.width, src->base.desc.height,
                                 src->base.desc.depth, &info))
            return ret | GMEM_USAGE_CONTIGUOUS;
      }
   }
   return ret;
}

/* Convert an image from gmem to gmem (can use hardware) */
bool v3d_imgconv_convert(
      const struct v3d_imgconv_gmem_tgt *dst,
      const struct v3d_imgconv_gmem_tgt *src,
      uint64_t *job_id,
      unsigned int width,
      unsigned int height,
      unsigned int depth,
      unsigned int num_elems,
      bool secure_context)
{
   v3d_scheduler_deps      out_deps;
   bool                    ok = false;

   /* Setup the output deps for the null job that depends on all other jobs */
   memset(&out_deps, 0, sizeof(out_deps));

   vcos_once(&paths_initialized, init_conv_paths);

   conversion_info_t info;
   build_conv_info(secure_context, src->handles, dst->handles, &info);

   for (unsigned i = 0; i < num_elems; i++)
   {
      unsigned dst_off = dst->base.array_pitch * i;
      unsigned src_off = src->base.array_pitch * i;

      bool conv_done = false;
      for (unsigned p = 0; !conv_done && p < MAX_PATHS && conv_path[p]; p++)
      {
         if (!conv_path[p]->claim(&dst->base, &src->base, width, height, depth, &info))
            continue;

         uint64_t cur_job = 0;
         if (conv_path[p]->convert_async != NULL)
         {
            v3d_sched_completion_fn     completion_fn = NULL;
            void                        *completion_data = NULL;

            if (conv_path[p]->convert_prep != NULL)
            {
               if (!gfx_buffer_single_region(&src->base.desc))
                  continue; // TODO Multi-region support

               /* Make a scratch buffer for our intermediate prep stage.
                * call_prep_func will create the scratch buffer of the correct format
                * for the following async conversion */
               struct v3d_imgconv_gmem_tgt scratch;

               if (!call_prep_func_gmem(conv_path[p]->convert_prep, &scratch, src, src_off,
                                          width, height, depth, &completion_fn, &completion_data))
                  continue;   // Try next algorithm

               /* Set up the dest buffer deps to be used by the async job */
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
            /* No direct async path available, schedule a CPU conversion */
            assert(conv_path[p]->convert_sync); /* we must have one convert fct */

            if (!gfx_buffer_single_region(&src->base.desc) || !gfx_buffer_single_region(&dst->base.desc))
               continue; // TODO Multi-region support

            uint64_t job = 0;
            if (!cpu_convert_gmem_to_gmem(&job, dst, src, dst_off, src_off, width, height, depth, p))
               goto end;

            if (job != 0)
               v3d_scheduler_add_dep(&out_deps, job);
         }
         conv_done = true;
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

static void adjust_for_memcpy(
      struct v3d_imgconv_base_tgt *dst,
      struct v3d_imgconv_base_tgt *src,
      unsigned int *src_width,
      unsigned int *src_height,
      unsigned int depth,
      unsigned int num_elems)
{
   for (uint32_t plane = 0; plane < src->desc.num_planes; plane++)
   {
      GFX_BUFFER_DESC_PLANE_T *src_plane = &src->desc.planes[plane];
      GFX_BUFFER_DESC_PLANE_T *dst_plane = &dst->desc.planes[plane];

      assert(gfx_lfmt_bytes_per_block(src_plane->lfmt) ==
             gfx_lfmt_bytes_per_block(dst_plane->lfmt));

      const bool src_compressed = gfx_lfmt_is_compressed(src_plane->lfmt);
      const bool dst_compressed = gfx_lfmt_is_compressed(dst_plane->lfmt);
      struct v3d_imgconv_base_tgt  *keep;
      struct v3d_imgconv_base_tgt  *modify;
      if (dst_compressed)
      {
         keep = dst;
         modify = src;
      }
      else
      {
         keep = src;
         modify = dst;
      }

      modify->desc.planes[plane].lfmt = gfx_lfmt_set_format(modify->desc.planes[plane].lfmt,
                                                            keep->desc.planes[plane].lfmt);

      /* if only one format is compressed, adjust the copy area accordingly */
      if (src_compressed != dst_compressed)
      {
         GFX_LFMT_BASE_DETAIL_T bd;
         gfx_lfmt_base_detail(&bd, keep->desc.planes[plane].lfmt);

         modify->x *= bd.block_w;
         modify->y *= bd.block_h;
         modify->z *= bd.block_d;
         modify->desc.width *= bd.block_w;
         modify->desc.height *= bd.block_h;

         if (modify == src)
         {
            /* there are no multi-planar compressed formats
             * so this will be executed only once */
            assert(src->desc.num_planes == 1);
            *src_width *= bd.block_w;
            *src_height *= bd.block_h;
         }
      }
   }

   /* after adjustment src_width == dst_width and src_height == dst_height */
   assert(*src_width <= src->desc.width);
   assert(*src_width <= dst->desc.width);
   assert(*src_height <= src->desc.height);
   assert(*src_height <= dst->desc.height);
}

/* Copy an image from gmem to gmem (can use hardware) */
bool v3d_imgconv_memcpy(
      const struct v3d_imgconv_gmem_tgt *dst,
      const struct v3d_imgconv_gmem_tgt *src,
      uint64_t *job_id,
      unsigned int src_width,
      unsigned int src_height,
      unsigned int depth,
      unsigned int num_elems,
      bool secure_context)
{
   assert(src->base.desc.num_planes == dst->base.desc.num_planes);

   struct v3d_imgconv_gmem_tgt dst_copy = *dst;
   struct v3d_imgconv_gmem_tgt src_copy = *src;
   adjust_for_memcpy(&dst_copy.base, &src_copy.base, &src_width,
         &src_height, depth, num_elems);

   return v3d_imgconv_convert(&dst_copy, &src_copy, job_id, src_width,
               src_height, depth, num_elems, secure_context);
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

   assert(scratch.handles[0] != GMEM_HANDLE_INVALID);

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
 * NOTE : Asynchronous conversion is not guaranteed.
 * You should not call this function from the Vulkan driver.
 * There is a potential event deadlock if the dependencies aren't
 * satisfied and a CPU conversion path is chosen.
 */
bool v3d_imgconv_convert_from_ptr(
   const v3d_imgconv_gmem_tgt *dst,
   const v3d_imgconv_ptr_tgt *src,
   uint64_t *job_id,
   unsigned int width,
   unsigned int height,
   unsigned int depth,
   unsigned int num_elems,
   bool secure_context)
{
   bool deps_completed = false;
   uint64_t cur_job = 0;

   vcos_once(&paths_initialized, init_conv_paths);

   conversion_info_t info;
   build_conv_info(secure_context, NULL, dst->handles, &info);

   v3d_scheduler_deps out_deps;
   memset(&out_deps, 0, sizeof(out_deps));

   bool ok = true;
   for (unsigned i = 0; i < num_elems; i++)
   {
      unsigned dst_off = dst->base.array_pitch * i;
      unsigned src_off = src->base.array_pitch * i;

      bool conv_done = false;
      for (unsigned p = 0; !conv_done && p < MAX_PATHS && conv_path_sync[p]; p++)
      {
         if (!conv_path_sync[p]->claim(&dst->base, &src->base, width, height, depth, &info))
            continue;

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

            if (!gfx_buffer_single_region(&dst->base.desc))
               continue; // TODO Multi-region support

            /* wait for pending jobs and start synchronous conversion */
            if (!deps_completed)
            {
               v3d_scheduler_deps dst_deps;
               v3d_scheduler_copy_deps(&dst_deps, &dst->deps);
               v3d_scheduler_wait_jobs(&dst_deps, V3D_SCHED_DEPS_COMPLETED);
               deps_completed = true;
            }

            void* dst_data = v3d_imgconv_map_and_sync_pre_cpu_access(dst, dst_off);
            if (!dst_data)
            {
               ok = false;
               goto end;
            }

            conv_path_sync[p]->convert_sync(&dst->base, dst_data, &src->base,
               (uint8_t*)src->data + src_off, width, height, depth);

            v3d_imgconv_sync_post_cpu_write(dst, dst_off);
         }
         conv_done = true;
      }
      assert(conv_done);
   }

end:
   /* Create a job to wait on all the created jobs and free scratch */
   if (out_deps.n > 1)
      *job_id = v3d_scheduler_submit_null_job(&out_deps, NULL, NULL);
   else
      *job_id = cur_job;
   return ok;
}

/* Convert from gmem into CPU memory (cannot use hardware) */
bool v3d_imgconv_convert_to_ptr(
      const struct v3d_imgconv_ptr_tgt *dst,
      const struct v3d_imgconv_gmem_tgt *src,
      unsigned int width,
      unsigned int height,
      unsigned int depth,
      unsigned int num_elems,
      bool secure_context)
{
   vcos_once(&paths_initialized, init_conv_paths);

   conversion_info_t info;
   build_conv_info(secure_context, src->handles, NULL, &info);

   v3d_scheduler_deps deps;
   v3d_scheduler_copy_deps(&deps, &src->deps);
   v3d_scheduler_wait_jobs(&deps, V3D_SCHED_DEPS_COMPLETED);

   for (unsigned i = 0; i < num_elems; i++)
   {
      unsigned dst_off = dst->base.array_pitch * i;
      unsigned src_off = src->base.array_pitch * i;

      bool conv_done = false;
      for (unsigned p = 0; !conv_done && p < MAX_PATHS && conv_path_sync[p]; p++)
      {
         if (conv_path_sync[p]->convert_sync == NULL)
            continue;

         if (!conv_path_sync[p]->claim(&dst->base, &src->base, width, height, depth, &info))
            continue;

         void *src_data = v3d_imgconv_map_and_sync_pre_cpu_access(src, src_off);
         if (!src_data)
            return false;

         conv_path_sync[p]->convert_sync(&dst->base, (uint8_t*)dst->data + dst_off,
            &src->base, src_data, width, height, depth);

         conv_done = true;
      }
      assert(conv_done);
   }

   return true;
}
