/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Format for dataflow binary objects
=============================================================================*/

#ifndef GLSL_IR_PROGRAM_H
#define GLSL_IR_PROGRAM_H

#include "libs/core/v3d/v3d_limits.h"
#include "glsl_ir_shader.h"

typedef enum {
   SHADER_VERTEX,
   SHADER_TESS_CONTROL,
   SHADER_TESS_EVALUATION,
   SHADER_GEOMETRY,
   SHADER_FRAGMENT,
   SHADER_COMPUTE,
   SHADER_FLAVOUR_COUNT
} ShaderFlavour;

#define DF_BLOB_FRAGMENT_COUNT 35
#define DF_BLOB_VERTEX_COUNT   69

#define DF_FNODE_R(n) (4*(n)+0) /* up to 8 of these */
#define DF_FNODE_G(n) (4*(n)+1)
#define DF_FNODE_B(n) (4*(n)+2)
#define DF_FNODE_A(n) (4*(n)+3)
/* DF_FNODE_A(7) == 31 */
#define DF_FNODE_DISCARD     32
#define DF_FNODE_DEPTH       33
#define DF_FNODE_SAMPLE_MASK 34

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

enum tess_mode {
   TESS_ISOLINES,
   TESS_TRIANGLES,
   TESS_QUADS,
   TESS_INVALID,
};

enum tess_spacing {
   TESS_SPACING_EQUAL,
   TESS_SPACING_FRACT_EVEN,
   TESS_SPACING_FRACT_ODD,
   TESS_SPACING_INVALID,
};

enum gs_out_type {
   GS_OUT_POINTS,
   GS_OUT_LINE_STRIP,
   GS_OUT_TRI_STRIP,
   GS_OUT_INVALID
};

typedef struct {
   struct {
      IRShader *ir;        /* NULL if stage not present */
      LinkMap  *link_map;  /* NULL if stage not present */
   } stage[SHADER_FLAVOUR_COUNT];

   /* A bit-field showing which attributes are live */
   uint32_t live_attr_set;
   GLSL_VARY_MAP_T tf_vary_map;

   bool early_fragment_tests;
   bool varyings_per_sample;
   VARYING_INFO_T varying[V3D_MAX_VARYING_COMPONENTS];

   unsigned          tess_vertices;
   enum tess_mode    tess_mode;
   enum tess_spacing tess_spacing;
   bool              tess_point_mode;
   bool              tess_cw;

   enum gs_out_type  gs_out;
   unsigned          gs_n_invocations;
   unsigned          gs_max_vertices;
} IR_PROGRAM_T;

LinkMap *glsl_link_map_alloc(int num_ins, int num_outs, int num_unifs, int num_buffers);
void glsl_link_map_free(LinkMap *l);

IR_PROGRAM_T *glsl_ir_program_create();
void          glsl_ir_program_free(IR_PROGRAM_T *bin);

#endif
