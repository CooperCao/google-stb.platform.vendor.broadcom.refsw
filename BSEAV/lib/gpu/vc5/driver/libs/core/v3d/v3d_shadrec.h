/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef V3D_SHADREC_H
#define V3D_SHADREC_H

#include "v3d_gen.h"
#include "libs/util/common.h"
#include "libs/util/assert_helpers.h"

EXTERN_C_BEGIN

typedef struct v3d_nv_shader_record_alloc_sizes
{
   size_t   packed_shader_rec_size;
   size_t   packed_shader_rec_align;
   size_t   defaults_size;
   size_t   defaults_align;
} V3D_NV_SHADER_RECORD_ALLOC_SIZES_T;

// Determine the sizes and alignments of memory needed to create an NV shader record
void v3d_get_nv_shader_record_alloc_sizes(V3D_NV_SHADER_RECORD_ALLOC_SIZES_T *sizes);

// Create NV shader record
void v3d_create_nv_shader_record(uint32_t *packed_shader_rec_ptr, v3d_addr_t packed_shader_rec_addr,
                                 uint32_t *defaults_ptr, v3d_addr_t defaults_addr, v3d_addr_t fshader_addr,
                                 v3d_addr_t funif_addr, v3d_addr_t vdata_addr,
                                 uint32_t vdata_max_index,
                                 bool does_z_writes,
                                 v3d_threading_t threading
                                 );

#if !V3D_VER_AT_LEAST(3,3,0,0)
void v3d_workaround_gfxh_1276(V3D_SHADREC_GL_MAIN_T *record);
#endif

static inline uint32_t v3d_attr_type_get_size_in_memory(v3d_attr_type_t type, unsigned num_elems)
{
   assert(num_elems <= 4u);
   switch (type)
   {
   case V3D_ATTR_TYPE_HALF_FLOAT:      return num_elems * 2;
   case V3D_ATTR_TYPE_FLOAT:           return num_elems * 4;
   case V3D_ATTR_TYPE_FIXED:           return num_elems * 4;
   case V3D_ATTR_TYPE_BYTE:            return num_elems * 1;
   case V3D_ATTR_TYPE_SHORT:           return num_elems * 2;
   case V3D_ATTR_TYPE_INT:             return num_elems * 4;
   case V3D_ATTR_TYPE_INT2_10_10_10:   return 4;
   default:                            unreachable();
   }
}

static inline uint32_t v3d_attr_get_num_elems(const V3D_SHADREC_GL_ATTR_T *attr)
{
   switch (attr->type)
   {
   case V3D_ATTR_TYPE_INT2_10_10_10:   return 4; /* attr->size is ignored */
   default:                            return attr->size;
   }
}

static inline uint32_t v3d_attr_get_size_in_memory(const V3D_SHADREC_GL_ATTR_T *attr)
{
   return v3d_attr_type_get_size_in_memory(attr->type, attr->size);
}

static inline const char *v3d_maybe_desc_shader_type_br(bool render, v3d_shader_type_t shader_type)
{
   switch (shader_type)
   {
   case V3D_SHADER_TYPE_VERTEX:  return render ? "vertex" : "coord";
   case V3D_SHADER_TYPE_TESSC:   return render ? "tessc_rdr" : "tessc_bin";
   case V3D_SHADER_TYPE_TESSE:   return render ? "tesse_rdr" : "tesse_bin";
   case V3D_SHADER_TYPE_GEOM:    return render ? "geom_rdr" : "geom_bin";
   case V3D_SHADER_TYPE_FRAG:    return "frag";
   case V3D_SHADER_TYPE_USER:    return "user";
#if V3D_VER_AT_LEAST(4,1,34,0)
   case V3D_SHADER_TYPE_COMPUTE: return "compute";
#endif
   default:                      return NULL;
   }
}

static inline const char *v3d_desc_shader_type_br(bool render, v3d_shader_type_t shader_type)
{
   const char *desc = v3d_maybe_desc_shader_type_br(render, shader_type);
   assert(desc);
   return desc;
}

#if V3D_VER_AT_LEAST(4,0,2,0)

static inline void v3d_shadrec_gl_tg_set_vpm_cfg(V3D_SHADREC_GL_TESS_OR_GEOM_T *sr,
   const V3D_VPM_CFG_TG_T cfg[2])
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   sr->bin = cfg[0];
   sr->render = cfg[1];
#else
   assert(cfg[0].tcs_batch_flush == cfg[1].tcs_batch_flush);
   assert(cfg[0].max_patches_per_tcs_batch == cfg[1].max_patches_per_tcs_batch);
   assert(cfg[0].max_patches_per_tes_batch == cfg[1].max_patches_per_tes_batch);
   sr->tcs_batch_flush = cfg[0].tcs_batch_flush;
   sr->per_patch_depth_bin = cfg[0].per_patch_depth;
   sr->per_patch_depth_render = cfg[1].per_patch_depth;
   sr->tcs_output_bin = cfg[0].tcs_output;
   sr->tcs_output_render = cfg[1].tcs_output;
   sr->tes_output_bin = cfg[0].tes_output;
   sr->tes_output_render = cfg[1].tes_output;
   sr->geom_output_bin = cfg[0].geom_output;
   sr->geom_output_render = cfg[1].geom_output;
   sr->max_patches_per_tcs_batch = cfg[0].max_patches_per_tcs_batch;
   sr->max_extra_vert_segs_per_tcs_batch_bin = cfg[0].max_extra_vert_segs_per_tcs_batch;
   sr->max_extra_vert_segs_per_tcs_batch_render = cfg[1].max_extra_vert_segs_per_tcs_batch;
   sr->min_tcs_segs_bin = cfg[0].min_tcs_segs;
   sr->min_tcs_segs_render = cfg[1].min_tcs_segs;
   sr->min_per_patch_segs_bin = cfg[0].min_per_patch_segs;
   sr->min_per_patch_segs_render = cfg[1].min_per_patch_segs;
   sr->max_patches_per_tes_batch = cfg[0].max_patches_per_tes_batch;
   sr->max_extra_vert_segs_per_tes_batch_bin = cfg[0].max_extra_vert_segs_per_tes_batch;
   sr->max_extra_vert_segs_per_tes_batch_render = cfg[1].max_extra_vert_segs_per_tes_batch;
   sr->max_tcs_segs_per_tes_batch_bin = cfg[0].max_tcs_segs_per_tes_batch;
   sr->max_tcs_segs_per_tes_batch_render = cfg[1].max_tcs_segs_per_tes_batch;
   sr->min_tes_segs_bin = cfg[0].min_tes_segs;
   sr->min_tes_segs_render = cfg[1].min_tes_segs;
   sr->max_extra_vert_segs_per_gs_batch_bin = cfg[0].max_extra_vert_segs_per_gs_batch;
   sr->max_extra_vert_segs_per_gs_batch_render = cfg[1].max_extra_vert_segs_per_gs_batch;
   sr->min_gs_segs_bin = cfg[0].min_gs_segs;
   sr->min_gs_segs_render = cfg[1].min_gs_segs;
#endif
}

static inline void v3d_shadrec_gl_tg_get_vpm_cfg(V3D_VPM_CFG_TG_T cfg[2],
   const V3D_SHADREC_GL_TESS_OR_GEOM_T *sr)
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   cfg[0] = sr->bin;
   cfg[1] = sr->render;
#else
   cfg[0].tcs_batch_flush = sr->tcs_batch_flush;
   cfg[0].per_patch_depth = sr->per_patch_depth_bin;
   cfg[0].tcs_output = sr->tcs_output_bin;
   cfg[0].tes_output = sr->tes_output_bin;
   cfg[0].geom_output = sr->geom_output_bin;
   cfg[0].max_patches_per_tcs_batch = sr->max_patches_per_tcs_batch;
   cfg[0].max_extra_vert_segs_per_tcs_batch = sr->max_extra_vert_segs_per_tcs_batch_bin;
   cfg[0].min_tcs_segs = sr->min_tcs_segs_bin;
   cfg[0].min_per_patch_segs = sr->min_per_patch_segs_bin;
   cfg[0].max_patches_per_tes_batch = sr->max_patches_per_tes_batch;
   cfg[0].max_extra_vert_segs_per_tes_batch = sr->max_extra_vert_segs_per_tes_batch_bin;
   cfg[0].max_tcs_segs_per_tes_batch = sr->max_tcs_segs_per_tes_batch_bin;
   cfg[0].min_tes_segs = sr->min_tes_segs_bin;
   cfg[0].max_extra_vert_segs_per_gs_batch = sr->max_extra_vert_segs_per_gs_batch_bin;
   cfg[0].min_gs_segs = sr->min_gs_segs_bin;
   cfg[1].tcs_batch_flush = sr->tcs_batch_flush;
   cfg[1].per_patch_depth = sr->per_patch_depth_render;
   cfg[1].tcs_output = sr->tcs_output_render;
   cfg[1].tes_output = sr->tes_output_render;
   cfg[1].geom_output = sr->geom_output_render;
   cfg[1].max_patches_per_tcs_batch = sr->max_patches_per_tcs_batch;
   cfg[1].max_extra_vert_segs_per_tcs_batch = sr->max_extra_vert_segs_per_tcs_batch_render;
   cfg[1].min_tcs_segs = sr->min_tcs_segs_render;
   cfg[1].min_per_patch_segs = sr->min_per_patch_segs_render;
   cfg[1].max_patches_per_tes_batch = sr->max_patches_per_tes_batch;
   cfg[1].max_extra_vert_segs_per_tes_batch = sr->max_extra_vert_segs_per_tes_batch_render;
   cfg[1].max_tcs_segs_per_tes_batch = sr->max_tcs_segs_per_tes_batch_render;
   cfg[1].min_tes_segs = sr->min_tes_segs_render;
   cfg[1].max_extra_vert_segs_per_gs_batch = sr->max_extra_vert_segs_per_gs_batch_render;
   cfg[1].min_gs_segs = sr->min_gs_segs_render;
#endif
}

#endif

EXTERN_C_END

#endif
