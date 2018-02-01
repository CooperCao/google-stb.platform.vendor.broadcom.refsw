/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_common.h"
#include "glsl_ir_program.h"
#include "glsl_backend_uniforms.h"
#include "libs/core/v3d/v3d_limits.h"

EXTERN_C_BEGIN

struct glsl_backend_cfg;

typedef struct {
   unsigned scalars_used[V3D_MAX_ATTR_ARRAYS]; /* how many scalars are read for each attrib */
   bool vertexid_used;
   bool instanceid_used;
   bool baseinstance_used;
} ATTRIBS_USED_T;

typedef struct {
   ShaderFlavour flavour;

   uint64_t   *code;
   size_t      code_size;
   umap_entry *unif;
   size_t      unif_count;
   unsigned n_threads;
#if V3D_VER_AT_LEAST(4,1,34,0)
   bool single_seg;
#endif
#if !V3D_VER_AT_LEAST(3,3,0,0)
   bool uses_control_flow;
#endif
   union {
      struct {
         bool writes_z;
         bool ez_disable;
         bool needs_w; // Need (non-centroid) W for anything other than varyings?
         bool tlb_wait_first_thrsw;
         bool per_sample;
         bool reads_prim_id;
#if V3D_VER_AT_LEAST(4,1,34,0)
         bool reads_implicit_varys;
#endif
#if !V3D_HAS_SRS_CENTROID_FIX
         bool ignore_centroids;
#endif
         bool barrier;
      } fragment;
      struct {
         bool     prim_id_used;
         bool     has_point_size;
         unsigned output_words;
      } geometry;
      struct {
         bool     prim_id_used;
         bool     has_point_size;
         unsigned output_words;
      } tess_e;
      struct {
         bool     prim_id_used;
         bool     barrier;
         unsigned output_words;
         unsigned output_words_patch;
      } tess_c;
      struct {
         ATTRIBS_USED_T attribs;
         unsigned       input_words;
         unsigned       output_words;
         bool           has_point_size;
         bool           combined_seg_ok;
      } vertex;
   } u;
} BINARY_SHADER_T;

BINARY_SHADER_T *glsl_binary_shader_create(ShaderFlavour    flavour);
void             glsl_binary_shader_free  (BINARY_SHADER_T *binary);

bool glsl_binary_shader_from_dataflow(
   BINARY_SHADER_T              **ret_binary,
   ShaderFlavour                  flavour,
   bool                           bin_mode,
   GLSL_VARY_MAP_T               *vary_map,
   IR_PROGRAM_T                  *ir,
   const struct glsl_backend_cfg *key);

const char *glsl_shader_flavour_name(ShaderFlavour f);

EXTERN_C_END
