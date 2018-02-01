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

#define GLXX_SHADER_FLAGS_POINT_SIZE_SHADED_VERTEX_DATA  (1<<0)
#define GLXX_SHADER_FLAGS_VS_READS_VERTEX_ID             (1<<2)   // 2 bits (bin/render)
#define GLXX_SHADER_FLAGS_VS_READS_INSTANCE_ID           (1<<4)   // 2 bits (bin/render)
#define GLXX_SHADER_FLAGS_VS_READS_BASE_INSTANCE         (1<<6)   // 2 bits (bin/render)
#define GLXX_SHADER_FLAGS_VS_SEPARATE_I_O_VPM_BLOCKS     (1<<8)   // 2 bits (bin/render)
#define GLXX_SHADER_FLAGS_FS_WRITES_Z                    (1<<10)
#define GLXX_SHADER_FLAGS_FS_EARLY_Z_DISABLE             (1<<11)
#define GLXX_SHADER_FLAGS_FS_NEEDS_W                     (1<<12)
#define GLXX_SHADER_FLAGS_TLB_WAIT_FIRST_THRSW           (1<<13)
#define GLXX_SHADER_FLAGS_PER_SAMPLE                     (1<<14)
#define GLXX_SHADER_FLAGS_TCS_BARRIERS                   (1<<15)
#define GLXX_SHADER_FLAGS_PRIM_ID_USED                   (1<<16)
#define GLXX_SHADER_FLAGS_PRIM_ID_TO_FS                  (1<<17)
#if V3D_VER_AT_LEAST(4,1,34,0)
#define GLXX_SHADER_FLAGS_DISABLE_IMPLICIT_VARYS         (1<<18)
#endif

struct attr_rec {
   int idx;
   uint8_t c_scalars_used;
   uint8_t v_scalars_used;
};

typedef enum glxx_vertex_pipe_stage
{
   GLXX_SHADER_VPS_VS,
#if GLXX_HAS_TNG
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
   uint32_t num_varys;
   uint8_t vary_map[GLXX_CONFIG_MAX_VARYING_SCALARS];

#if !V3D_VER_AT_LEAST(3,3,0,0)
   bool bin_uses_control_flow;
   bool render_uses_control_flow;
#endif

   uint32_t num_bin_qpu_instructions;

   uint32_t        attr_count;
   struct attr_rec attr[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];

   uint32_t flags;
   uint8_t vs_input_words[MODE_COUNT];
   uint8_t vs_output_words[MODE_COUNT];
#if GLXX_HAS_TNG
   uint8_t tcs_output_vertices_per_patch;             // set to 0 if TCS doesn't write vertex data, glxx_hw will create 1 invocation for the patch.
   uint8_t tcs_output_words_per_patch[MODE_COUNT];
   uint8_t tcs_output_words[MODE_COUNT];
   uint8_t tes_output_words[MODE_COUNT];
   uint16_t gs_output_words[MODE_COUNT];

   uint8_t geom_invocations;
   uint8_t geom_prim_type    : 2; // v3d_cl_geom_prim_type_t

   uint8_t tess_type         : 2; // v3d_cl_tess_type_t
   uint8_t tess_point_mode   : 1; // bool
   uint8_t tess_edge_spacing : 2; // v3d_cl_tess_edge_spacing_t
   uint8_t tess_clockwise    : 1; // bool

   union
   {
      struct
      {
         uint8_t has_tess : 1;
         uint8_t has_geom : 1;
      };
      uint8_t has_tng;
   };
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

   uint32_t varying_centroid[V3D_MAX_VARY_FLAG_WORDS];
   uint32_t varying_flat[V3D_MAX_VARY_FLAG_WORDS];
#if V3D_VER_AT_LEAST(4,1,34,0)
   uint32_t varying_noperspective[V3D_MAX_VARY_FLAG_WORDS];
#endif

   struct
   {
      struct
      {
         union
         {
#if GLXX_HAS_TNG
            struct { uint8_t num_patch_vertices; } tg;
#endif
            struct { bool z_pre_pass; } v;
         };
         bool valid;
      } key;

      v3d_vpm_cfg_v vpm_cfg_v[2];
#if GLXX_HAS_TNG
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
