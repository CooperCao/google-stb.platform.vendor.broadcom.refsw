/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_BINARY_SHADER_H_INCLUDED
#define GLSL_BINARY_SHADER_H_INCLUDED

#include "glsl_common.h"

#include "../glxx/glxx_shader_cache.h"

typedef struct {
   unsigned scalars_used[V3D_MAX_ATTR_ARRAYS]; /* how many scalars are read for each attrib */
   bool vertexid_used;
   bool instanceid_used;
} ATTRIBS_USED_T;

typedef struct {
   ShaderFlavour flavour;

   uint32_t *code;
   size_t    code_size;
   uint32_t *unif;
   size_t    unif_count;
   unsigned  n_threads;
   bool      uses_control_flow;
   union {
      struct {
         bool writes_z;
         bool ez_disable;
         bool tlb_wait_first_thrsw;
      } fragment;
      struct {
         unsigned reads_total;
         ATTRIBS_USED_T attribs;
      } vertex;
   } u;
} BINARY_SHADER_T;

BINARY_SHADER_T *glsl_binary_shader_create(ShaderFlavour    flavour);
void             glsl_binary_shader_free  (BINARY_SHADER_T *binary);

BINARY_SHADER_T *glsl_binary_shader_from_dataflow(ShaderFlavour                 flavour,
                                                  bool                          bin_mode,
                                                  GLSL_VARY_MAP_T              *vary_map,
                                                  IR_PROGRAM_T                 *ir,
                                                  const GLXX_LINK_RESULT_KEY_T *key);

const char *glsl_shader_flavour_name(ShaderFlavour f);

#endif
