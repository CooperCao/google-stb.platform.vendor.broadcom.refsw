/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/v3d/v3d_limits.h"
#include "libs/core/v3d/v3d_gen.h"
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
   bool noperspective;
} VARYING_INFO_T;

typedef struct {
   int n;
   uint32_t entries[V3D_MAX_VARYING_COMPONENTS];
} GLSL_VARY_MAP_T;

enum gs_in_type {
   GS_IN_POINTS,
   GS_IN_LINES,
   GS_IN_TRIANGLES,
   GS_IN_LINES_ADJ,
   GS_IN_TRIS_ADJ,
   GS_IN_INVALID
};

typedef struct {
   struct {
      IRShader *ir;        /* NULL if stage not present */
      LinkMap  *link_map;  /* NULL if stage not present */
   } stage[SHADER_FLAVOUR_COUNT];

   uint32_t live_attr_set; /* A bit-field showing which attributes are live */

   unsigned                   tess_vertices;
   v3d_cl_tess_type_t         tess_mode;
   v3d_cl_tess_edge_spacing_t tess_spacing;
   bool                       tess_point_mode;
   bool                       tess_cw;

   enum gs_in_type         gs_in;
   v3d_cl_geom_prim_type_t gs_out;
   unsigned                gs_n_invocations;
   unsigned                gs_max_vertices;

   GLSL_VARY_MAP_T tf_vary_map;

   bool                   early_fragment_tests;
   AdvancedBlendQualifier abq;
   bool                   varyings_per_sample;
   VARYING_INFO_T         varying[V3D_MAX_VARYING_COMPONENTS];
   unsigned               max_known_layers;

   unsigned cs_wg_size[3];
   unsigned cs_shared_block_size;
} IR_PROGRAM_T;

EXTERN_C_BEGIN

bool glsl_wg_size_requires_barriers(const unsigned wg_size[3]);

LinkMap *glsl_link_map_alloc(int num_ins, int num_outs, int num_unifs, int num_buffers);
void glsl_link_map_free(LinkMap *l);

IR_PROGRAM_T *glsl_ir_program_create();
void          glsl_ir_program_free(IR_PROGRAM_T *bin);

EXTERN_C_END
