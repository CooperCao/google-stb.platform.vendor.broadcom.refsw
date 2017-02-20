/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_SHADER_4_H
#define GLXX_SHADER_4_H

#include "../glsl/glsl_ir_program.h"
#include "glxx_shader_cache.h"
#include "../common/khrn_fmem.h"

typedef struct glxx_ustream_job
{
   uint32_t* dst_ptr;
   gmem_handle_t src_mem;
   glxx_ustream_fetch* fetches;
   uint32_t num_fetches;
   v3d_size_t src_offset;
   v3d_size_t src_size;
} glxx_ustream_job;

typedef struct glxx_ustream_job_block
{
   glxx_ustream_job jobs[64];
   struct glxx_ustream_job_block* next;
} glxx_ustream_job_block;

bool glxx_hw_emit_shaders(GLXX_LINK_RESULT_DATA_T  *data,
                          const GLSL_BACKEND_CFG_T *key,
                          IR_PROGRAM_T             *ir);

void glxx_hw_cleanup_shaders(GLXX_LINK_RESULT_DATA_T *data);

void glxx_shader_fill_ustream(
   uint32_t* dst,
   uint32_t const* src,
   glxx_ustream_fetch const* fetches,
   unsigned num_fetches);

void glxx_shader_process_ustream_jobs(glxx_ustream_job_block const* blocks, unsigned last_size);

#endif
