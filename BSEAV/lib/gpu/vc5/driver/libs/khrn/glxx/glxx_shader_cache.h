/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include "../common/khrn_resource.h"
#include "glxx_int_config.h"
#include "gl_public_api.h"
#include "../glsl/glsl_backend_cfg.h"
#include "../glsl/glsl_ir_program.h"
#include "../glsl/glsl_binary_program.h"
#include "../glsl/glsl_backend_uniforms.h"
#include "libs/core/v3d/v3d_gen.h"
#include "libs/core/v3d/v3d_vpm.h"
#include "libs/util/assert_helpers.h"
#include "libs/compute/compute.h"
#include "libs/linkres/linkres.h"

typedef struct glxx_shader_general_uniform
{
   BackendUniformFlavour type;
   uint32_t value;
} glxx_shader_general_uniform;

typedef struct glxx_shader_ubo_load_batch
{
   uint32_t index          : 8;  // UBO index.
   uint32_t load_end       : 24; // End of loads from this UBO.
   uint16_t range_start;
   uint16_t range_size;
} glxx_shader_ubo_load_batch;

typedef struct glxx_shader_ubo_load
{
   uint32_t dst20_src12;
} glxx_shader_ubo_load;
static_assrt(sizeof(glxx_shader_ubo_load) == sizeof(uint32_t));

#define GLXX_SHADER_COMPUTE_INDIRECT_BUFFER GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS
#define GLXX_SHADER_MAX_UNIFORM_BUFFERS (GLXX_SHADER_COMPUTE_INDIRECT_BUFFER+1)

static_assrt(GLXX_SHADER_MAX_UNIFORM_BUFFERS <= (1 << 8));
static_assrt(GLXX_CONFIG_MAX_UNIFORM_BLOCK_SIZE/4 <= (1 << 12));

typedef struct
{
   uint32_t num_uniforms;           // Number of uniforms in stream.
   uint32_t num_general_uniforms;   // Number of general uniforms, BACKEND_UNIFORM_UBO_LOAD entries advance the uniform write pointer.
   uint32_t num_ubo_load_batches;   // Number of ubo load batches.
   glxx_shader_general_uniform* general_uniforms;
   glxx_shader_ubo_load_batch* ubo_load_batches;
   glxx_shader_ubo_load* ubo_loads;
} GLXX_UNIFORM_MAP_T;

typedef struct
{
   v3d_size_t code_offset; // Offset of code for this shader in GLXX_LINK_RESULT_DATA_T.res
   GLXX_UNIFORM_MAP_T *uniform_map;
   v3d_threading_t threading;
#if V3D_VER_AT_LEAST(4,1,34,0)
   bool single_seg;
#endif
} GLXX_SHADER_DATA_T;

typedef enum glxx_vertex_pipe_stage
{
   GLXX_SHADER_VPS_VS,
#if V3D_VER_AT_LEAST(4,1,34,0)
   GLXX_SHADER_VPS_GS,     // order of T+G stages matches shader record
   GLXX_SHADER_VPS_TCS,
   GLXX_SHADER_VPS_TES,
#endif
   GLXX_SHADER_VPS_COUNT
} glxx_vertex_pipe_stage;

typedef struct glxx_link_result_data
{
   khrn_resource *res;
   GLXX_SHADER_DATA_T vps[GLXX_SHADER_VPS_COUNT][MODE_COUNT];
   GLXX_SHADER_DATA_T fs;

   linkres_t data;

#if !V3D_VER_AT_LEAST(3,3,0,0)
   bool bin_uses_control_flow;
   bool render_uses_control_flow;
#endif

#if V3D_VER_AT_LEAST(3,3,0,0)
   struct
   {
      uint8_t wgs_per_sg;        // number of work-groups per super-group.
    #if V3D_USE_CSD
      uint8_t max_sg_id;         // maximum number of concurrent super-groups minus 1.
    #else
      uint16_t max_wgs;          // maximum number of concurrent work-groups.
      bool has_barrier;
    #endif
      bool allow_concurrent_jobs;
   } cs;
#endif

   struct
   {
      struct
      {
         union
         {
#if V3D_VER_AT_LEAST(4,1,34,0)
            struct { uint8_t num_patch_vertices; } tg;
#endif
            struct { bool z_pre_pass; } v;
         };
         bool valid;
      } key;

      v3d_vpm_cfg_v vpm_cfg_v[2];
#if V3D_VER_AT_LEAST(4,1,34,0)
      uint32_t shadrec_tg_packed[V3D_SHADREC_GL_TESS_OR_GEOM_PACKED_SIZE/4];
#endif
   } cached_vpm_cfg;
} GLXX_LINK_RESULT_DATA_T;

typedef struct
{
   GLSL_BACKEND_CFG_T key;
   GLXX_LINK_RESULT_DATA_T data;
   bool used;
} GLXX_BINARY_CACHE_ENTRY_T;

#define GLXX_BINARY_CACHE_SIZE 16

typedef struct
{
   GLXX_BINARY_CACHE_ENTRY_T entry[GLXX_BINARY_CACHE_SIZE];
   uint32_t used;
   uint32_t next;
} GLXX_BINARY_CACHE_T;

extern GLXX_LINK_RESULT_DATA_T *glxx_binary_cache_get_shaders(
   GLXX_BINARY_CACHE_T *cache,
   const GLSL_BACKEND_CFG_T *key);

extern GLXX_LINK_RESULT_DATA_T *glxx_get_shaders_and_cache(
   GLXX_BINARY_CACHE_T *cache,
   IR_PROGRAM_T *ir,
   const GLSL_BACKEND_CFG_T *key);

extern void glxx_binary_cache_invalidate(GLXX_BINARY_CACHE_T *cache);
