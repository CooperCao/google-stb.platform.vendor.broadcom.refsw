/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef V3D_SHADREC_H
#define V3D_SHADREC_H

#include "v3d_gen.h"
#include "vcos_types.h"

VCOS_EXTERN_C_BEGIN

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
                                 v3d_addr_t funif_addr, v3d_addr_t vdata_addr, bool does_z_writes,
                                 v3d_threading_t threading);

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

static inline v3d_threading_t v3d_translate_threading(uint32_t threadability)
{
   switch (threadability) {
   case 1:  return V3D_THREADING_T1;
   case 2:  return V3D_THREADING_T2;
   case 4:  return V3D_THREADING_T4;
   default: unreachable(); return V3D_THREADING_INVALID;
   }
}

static inline uint32_t v3d_get_threadability(v3d_threading_t threading)
{
   switch (threading) {
   case V3D_THREADING_T1:  return 1;
   case V3D_THREADING_T2:  return 2;
   case V3D_THREADING_T4:  return 4;
   default: unreachable(); return 0;
   }
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
   default:                      return NULL;
   }
}

static inline const char *v3d_desc_shader_type_br(bool render, v3d_shader_type_t shader_type)
{
   const char *desc = v3d_maybe_desc_shader_type_br(render, shader_type);
   assert(desc);
   return desc;
}

VCOS_EXTERN_C_END

#endif
