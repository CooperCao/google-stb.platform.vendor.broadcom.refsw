/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "v3d_imgconv.h"
#include "libs/core/gfx_buffer/gfx_buffer.h"
#include "gmem.h"
#include "v3d_scheduler.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/lfmt/lfmt.h"

typedef struct security_info_t
{
   bool  secure_context;
   bool  secure_src;
   bool  secure_dst;
} security_info_t;

typedef bool (*convert_async_t) (
      const struct v3d_imgconv_gmem_tgt *dst, unsigned int dst_off,
      const struct v3d_imgconv_gmem_tgt *src, unsigned int src_off,
      const v3d_scheduler_deps *deps, uint64_t *job_id,
      unsigned int width, unsigned int height,
      unsigned int depth);

typedef void (*convert_sync_t) (
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
      unsigned int width, unsigned int height, unsigned int depth,
      const security_info_t *sec_info);

typedef struct
{
   claim_t           claim;
   convert_async_t   convert_async;
   convert_sync_t    convert_sync;
   convert_prep_t    convert_prep;

} v3d_imgconv_methods;

extern size_t v3d_imgconv_base_size(const struct v3d_imgconv_base_tgt *base);

extern bool v3d_imgconv_valid_hw_sec_info(const security_info_t *sec_info);
extern bool v3d_imgconv_valid_cpu_sec_info(const security_info_t *sec_info);

const v3d_imgconv_methods* get_tfu_path(void);
const v3d_imgconv_methods* get_memcpy_tfu_path(void);
const v3d_imgconv_methods* get_yv12_tfu_path(void);
const v3d_imgconv_methods* get_neon_path(void);
const v3d_imgconv_methods* get_extra_neon_path(void);
const v3d_imgconv_methods* get_c_path(void);
const v3d_imgconv_methods* get_gfx_blit_path(void);
