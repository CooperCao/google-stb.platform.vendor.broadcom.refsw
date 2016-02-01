/*=============================================================================
Copyright (c); 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Format for dataflow binary objects
=============================================================================*/

#ifndef GLSL_IR_PROGRAM_H
#define GLSL_IR_PROGRAM_H

#include "helpers/v3d/v3d_limits.h"
#include "glsl_ir_shader.h"

#define DF_FNODE_R(n) (4*(n)+0) /* up to 8 of these */
#define DF_FNODE_G(n) (4*(n)+1)
#define DF_FNODE_B(n) (4*(n)+2)
#define DF_FNODE_A(n) (4*(n)+3)
/* DF_FNODE_A(7) == 31 */
#define DF_FNODE_DISCARD 32
#define DF_FNODE_DEPTH 33

#define DF_VNODE_X 0
#define DF_VNODE_Y 1
#define DF_VNODE_Z 2
#define DF_VNODE_W 3
#define DF_VNODE_POINT_SIZE 4
#define DF_VNODE_VARY(n) (5+(n)) /* up to 64 of these */
/* DF_VNODE_VARY(63) == 68 */

typedef struct {
   bool centroid;
   bool flat;
} VARYING_INFO_T;

typedef struct {
   int n;
   uint32_t entries[V3D_MAX_VARYING_COMPONENTS];
} GLSL_VARY_MAP_T;


typedef struct {
   IRShader *vshader;
   IRShader *fshader;

   LinkMap *vlink_map;
   LinkMap *flink_map;

   /* A bit-field showing which attributes are live */
   uint32_t live_attr_set;

   GLSL_VARY_MAP_T tf_vary_map;

   VARYING_INFO_T varying[V3D_MAX_VARYING_COMPONENTS];
} IR_PROGRAM_T;

LinkMap *glsl_link_map_alloc(int num_ins, int num_outs, int num_unifs, int num_buffers);
void glsl_link_map_free(LinkMap *l);

IR_PROGRAM_T *glsl_ir_program_create();
void          glsl_ir_program_free(IR_PROGRAM_T *bin);

#endif
