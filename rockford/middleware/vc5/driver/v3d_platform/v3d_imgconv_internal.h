/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Image format conversion

FILE DESCRIPTION
============================================================================*/
#pragma once

#include "v3d_imgconv.h"
#include "helpers/gfx/gfx_buffer.h"
#include "gmem.h"
#include "v3d_scheduler.h"
#include "helpers/gfx/gfx_lfmt_translate_v3d.h"
#include "helpers/gfx/gfx_lfmt.h"

typedef bool (*convert_async_t) (
      const struct v3d_imgconv_gmem_tgt *dst, unsigned int dst_off,
      const struct v3d_imgconv_gmem_tgt *src, unsigned int src_off,
      const v3d_scheduler_deps *deps, uint64_t *job_id,
      unsigned int width, unsigned int height,
      unsigned int depth);

typedef bool (*convert_sync_t) (
      const struct v3d_imgconv_base_tgt *dst, void *dst_data,
      const struct v3d_imgconv_base_tgt *src, void *src_data,
      unsigned int width, unsigned int height, unsigned int depth);

typedef bool (*convert_prep_t) (
      struct v3d_imgconv_gmem_tgt *dst,
      const struct v3d_imgconv_base_tgt *src, void *src_data,
      unsigned int width, unsigned int height, unsigned int depth,
      v3d_sched_completion_fn *cleanup_fn, void **cleanup_data);

typedef bool (*claim_t) (
      const struct v3d_imgconv_base_tgt *dst,
      const struct v3d_imgconv_base_tgt *src,
      unsigned int width, unsigned int height, unsigned int depth);

typedef struct
{
   claim_t           claim;
   convert_async_t   convert_async;
   convert_sync_t    convert_sync;
   convert_prep_t    convert_prep;

} v3d_imgconv_methods;

extern void *v3d_imgconv_gmem_tgt_to_ptr(gmem_cpu_sync_list* sync_list,
                                         const struct v3d_imgconv_gmem_tgt *tgt,
                                         unsigned int offset, bool write);

extern size_t v3d_imgconv_base_size(const struct v3d_imgconv_base_tgt *base);

const v3d_imgconv_methods* get_tlb_path(void);
const v3d_imgconv_methods* get_tfu_path(void);
const v3d_imgconv_methods* get_memcpy_tfu_path(void);
const v3d_imgconv_methods* get_yv12_tfu_path(void);
const v3d_imgconv_methods* get_neon_path(void);
const v3d_imgconv_methods* get_extra_neon_path(void);
const v3d_imgconv_methods* get_c_path(void);
const v3d_imgconv_methods* get_gfx_blit_path(void);
