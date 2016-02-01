/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_BINARY_SHADER_H_INCLUDED
#define GLSL_BINARY_SHADER_H_INCLUDED

#include "glsl_common.h"

#include "middleware/khronos/glxx/glxx_shader_cache.h"

typedef enum {
   BINARY_SHADER_FRAGMENT,
   BINARY_SHADER_VERTEX,
   BINARY_SHADER_COORDINATE,
} glsl_binary_shader_flavour_t;

typedef struct {
   unsigned scalars_used[V3D_MAX_ATTR_ARRAYS]; /* how many scalars are read for each attrib */
   bool vertexid_used;
   bool instanceid_used;
} ATTRIBS_USED_T;

typedef struct {
   glsl_binary_shader_flavour_t flavour;

   uint32_t *code;
   size_t    code_size;
   uint32_t *unif;
   size_t    unif_count;
   unsigned  n_threads;
   bool      uses_control_flow;
   union {
      struct {
         bool discard;
         bool z_change;
      } fragment;
      struct {
         unsigned reads_total;
         ATTRIBS_USED_T attribs;
      } vertex;
   } u;
} BINARY_SHADER_T;

BINARY_SHADER_T *glsl_binary_shader_create       (glsl_binary_shader_flavour_t  flavour);
BINARY_SHADER_T *glsl_binary_shader_from_dataflow(glsl_binary_shader_flavour_t  flavour,
                                                  GLSL_VARY_MAP_T              *vary_map,
                                                  IR_PROGRAM_T                 *ir,
                                                  const GLXX_LINK_RESULT_KEY_T *key,
                                                  int                           v3d_version);
void             glsl_binary_shader_free         (BINARY_SHADER_T              *binary);

const char *glsl_binary_shader_flavour_name(glsl_binary_shader_flavour_t b);

#endif
