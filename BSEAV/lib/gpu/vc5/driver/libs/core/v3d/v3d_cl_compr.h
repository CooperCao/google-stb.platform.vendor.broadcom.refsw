/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef V3D_CL_COMPR_H
#define V3D_CL_COMPR_H

#include "v3d_cl.h"
#include "v3d_gen.h"
#include "v3d_util.h"
#include "libs/util/gfx_util/gfx_util.h"

EXTERN_C_BEGIN

/* Is encoding in indices mode the same for all prim types? */
extern bool v3d_cl_has_common_ind_encoding(v3d_cl_compr_type_t type);

extern uint32_t v3d_cl_compr_size(
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt,
   v3d_cl_compr_type_t compr_type);

#define V3D_CL_MAX_COMPR_SIZE_SHARED \
   GFX_MAX(V3D_CL_COMPR_IND_COMMON_MAX_PACKED_SIZE, \
   GFX_MAX(V3D_CL_COMPR_IND_TRI_MAX_PACKED_SIZE, \
   GFX_MAX(V3D_CL_COMPR_IND_D3DPVSF_TRI_MAX_PACKED_SIZE, \
   GFX_MAX(V3D_CL_COMPR_IND_LINE_MAX_PACKED_SIZE, \
   GFX_MAX(V3D_CL_COMPR_IND_POINT_MAX_PACKED_SIZE, \
   V3D_CL_COMPR_XY_TRI_MAX_PACKED_SIZE)))))

#if V3D_VER_AT_LEAST(4,1,34,0)
#define V3D_CL_MAX_COMPR_SIZE GFX_MAX(V3D_CL_MAX_COMPR_SIZE_SHARED, V3D_CL_COMPR_IND_GENERIC_MAX_PACKED_SIZE)
#else
#define V3D_CL_MAX_COMPR_SIZE V3D_CL_MAX_COMPR_SIZE_SHARED
#endif

union v3d_cl_compr_union
{
   uint32_t id; /* iid8, iid32, prim_id8, prim_id32 */

#if V3D_VER_AT_LEAST(4,1,34,0)
   V3D_CL_COMPR_IND_COMMON_CLIPPED_PRIM_T ind_common_clipped_prim;
#else
   int32_t rel_branch;
   v3d_addr_t branch;
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
   V3D_CL_COMPR_IND_GENERIC_C0_T ind_generic_c0;
   V3D_CL_COMPR_IND_GENERIC_C1_T ind_generic_c1;
   V3D_CL_COMPR_IND_GENERIC_C2_T ind_generic_c2;
   V3D_CL_COMPR_IND_GENERIC_C3_T ind_generic_c3;
   V3D_CL_COMPR_IND_GENERIC_C4_T ind_generic_c4;
   V3D_CL_COMPR_IND_GENERIC_C5_T ind_generic_c5;
   V3D_CL_COMPR_IND_GENERIC_C6_T ind_generic_c6;
#endif

   V3D_CL_COMPR_IND_TRI_C0_T ind_tri_c0;
   V3D_CL_COMPR_IND_TRI_C1_T ind_tri_c1;
   V3D_CL_COMPR_IND_TRI_C2_T ind_tri_c2;
   V3D_CL_COMPR_IND_TRI_C3_T ind_tri_c3;
   V3D_CL_COMPR_IND_TRI_C4_T ind_tri_c4;
   V3D_CL_COMPR_IND_TRI_C5_T ind_tri_c5;
#if V3D_VER_AT_LEAST(4,1,34,0)
   V3D_CL_COMPR_IND_TRI_C6_T ind_tri_c6;
#endif

   V3D_CL_COMPR_IND_D3DPVSF_TRI_C0_T ind_d3dpvsf_tri_c0;
   V3D_CL_COMPR_IND_D3DPVSF_TRI_C1_T ind_d3dpvsf_tri_c1;
   V3D_CL_COMPR_IND_D3DPVSF_TRI_C2_T ind_d3dpvsf_tri_c2;
   V3D_CL_COMPR_IND_D3DPVSF_TRI_C3_T ind_d3dpvsf_tri_c3;
   V3D_CL_COMPR_IND_D3DPVSF_TRI_C4_T ind_d3dpvsf_tri_c4;
   V3D_CL_COMPR_IND_D3DPVSF_TRI_C5_T ind_d3dpvsf_tri_c5;
#if V3D_VER_AT_LEAST(4,1,34,0)
   V3D_CL_COMPR_IND_D3DPVSF_TRI_C6_T ind_d3dpvsf_tri_c6;
#endif

   V3D_CL_COMPR_IND_LINE_C0_T ind_line_c0;
   V3D_CL_COMPR_IND_LINE_C1_T ind_line_c1;
   V3D_CL_COMPR_IND_LINE_C2_T ind_line_c2;
   V3D_CL_COMPR_IND_LINE_C3_T ind_line_c3;
   V3D_CL_COMPR_IND_LINE_C4_T ind_line_c4;
   V3D_CL_COMPR_IND_LINE_C5_T ind_line_c5;
#if V3D_VER_AT_LEAST(4,1,34,0)
   V3D_CL_COMPR_IND_LINE_C6_T ind_line_c6;
#endif

   V3D_CL_COMPR_IND_POINT_C0_T ind_point_c0;
   V3D_CL_COMPR_IND_POINT_C1_T ind_point_c1;
   V3D_CL_COMPR_IND_POINT_C2_T ind_point_c2;
   V3D_CL_COMPR_IND_POINT_C4_T ind_point_c4;
   V3D_CL_COMPR_IND_POINT_C5_T ind_point_c5;
#if V3D_VER_AT_LEAST(4,1,34,0)
   V3D_CL_COMPR_IND_POINT_C6_T ind_point_c6;
#endif

   V3D_CL_COMPR_XY_TRI_C0_T xy_tri_c0;
   V3D_CL_COMPR_XY_TRI_C1_T xy_tri_c1;
   V3D_CL_COMPR_XY_TRI_C2_T xy_tri_c2;
   V3D_CL_COMPR_XY_TRI_C3_T xy_tri_c3;
};

struct v3d_cl_compr
{
   V3D_CL_PRIM_LIST_FORMAT_T plist_fmt;
   v3d_cl_compr_type_t compr_type;
   union v3d_cl_compr_union u;
};

static inline uint32_t v3d_cl_compr_size_2(const struct v3d_cl_compr *c)
{
   return v3d_cl_compr_size(&c->plist_fmt, c->compr_type);
}

extern void v3d_cl_unpack_compr(struct v3d_cl_compr *compr,
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt,
   const uint8_t *packed_compr);
extern void v3d_cl_pack_compr(uint8_t *packed_compr,
   const struct v3d_cl_compr *compr);

extern void v3d_cl_print_compr(const uint8_t *packed_compr,
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt,
   struct v3d_printer *printer);

extern void v3d_cl_log_cat_compr(
   struct log_cat *cat, log_level_t level, const char *line_prefix,
   const uint8_t *packed_compr, const struct v3d_cl_source *source,
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt);

#define v3d_cl_log_trace_compr(LINE_PREFIX, PACKED_COMPR, SOURCE, PLIST_FMT) \
   v3d_cl_log_cat_compr(&log_default_cat, LOG_TRACE, LINE_PREFIX, PACKED_COMPR, SOURCE, PLIST_FMT)

static inline v3d_addr_t v3d_cl_compr_rel_branch_dst(
   const struct v3d_cl_source *source, int32_t rel_addr)
{
   return v3d_addr_align_down(source->parts[0].addr, 32) + rel_addr;
}

EXTERN_C_END

#endif
