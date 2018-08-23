/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include "libs/util/common.h"
#include "libs/core/v3d/v3d_ver.h"
#include "libs/core/v3d/v3d_limits.h"
#include "libs/core/v3d/v3d_vpm.h"

#include "../glsl/glsl_binary_program.h"

#include <stdint.h>

EXTERN_C_BEGIN

struct attr_rec {
   int idx;
   uint8_t c_scalars_used;
   uint8_t v_scalars_used;
};

typedef enum {
   LINKRES_FLAG_NONE                    = 0,
   LINKRES_FLAG_POINT_SIZE              = (1<<0),
   LINKRES_FLAG_VS_VERTEX_ID            = (1<<2),   // 2 bits (bin/render)
   LINKRES_FLAG_VS_INSTANCE_ID          = (1<<4),   // 2 bits (bin/render)
   LINKRES_FLAG_VS_BASE_INSTANCE        = (1<<6),   // 2 bits (bin/render)
   LINKRES_FLAG_VS_SEPARATE_VPM_BLOCKS  = (1<<8),   // 2 bits (bin/render)
   LINKRES_FLAG_FS_WRITES_Z             = (1<<10),
   LINKRES_FLAG_FS_EARLY_Z_DISABLE      = (1<<11),
   LINKRES_FLAG_FS_NEEDS_W              = (1<<12),
   LINKRES_FLAG_TLB_WAIT_FIRST_THRSW    = (1<<13),
   LINKRES_FLAG_PER_SAMPLE              = (1<<14),
   LINKRES_FLAG_TCS_BARRIERS            = (1<<15),
   LINKRES_FLAG_PRIM_ID_USED            = (1<<16),
   LINKRES_FLAG_PRIM_ID_TO_FS           = (1<<17),
#if V3D_VER_AT_LEAST(4,1,34,0)
   LINKRES_FLAG_DISABLE_IMPLICIT_VARYS  = (1<<18)
#endif
} linkres_flags_t;

typedef struct {
   uint32_t count;
#if !V3D_USE_CSD
   uint8_t  map[V3D_MAX_VARYING_COMPONENTS];
#endif

   uint32_t centroid[V3D_MAX_VARY_FLAG_WORDS];
   uint32_t flat[V3D_MAX_VARY_FLAG_WORDS];
#if V3D_VER_AT_LEAST(4,1,34,0)
   uint32_t noperspective[V3D_MAX_VARY_FLAG_WORDS];
#endif
} linkres_vary_data_t;

typedef struct {
   uint32_t flags;

   uint32_t        attr_count;
   struct attr_rec attr[V3D_MAX_ATTR_ARRAYS];

   uint8_t vs_input_words[MODE_COUNT];
   uint8_t vs_output_words[MODE_COUNT];

   uint32_t num_bin_qpu_instructions;

   linkres_vary_data_t vary;

#if V3D_VER_AT_LEAST(4,1,34,0)
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

   uint8_t has_tess          : 1;
   uint8_t has_geom          : 1;
#endif
} linkres_t;

void linkres_fill_data(const BINARY_PROGRAM_T *p, const IR_PROGRAM_T *ir, linkres_t *out);

#if V3D_VER_AT_LEAST(4,1,34,0)
void linkres_compute_tng_vpm_cfg(v3d_vpm_cfg_v cfg_v[2],
                                 uint32_t shadrec_tg_packed[],
                                 const linkres_t *link_data,
                                 unsigned num_patch_vertices, uint32_t vpm_size);
#endif

EXTERN_C_END
