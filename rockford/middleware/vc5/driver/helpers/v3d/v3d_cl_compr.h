/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef V3D_CL_COMPR_H
#define V3D_CL_COMPR_H

#include "helpers/v3d/v3d_cl.h"
#include "helpers/v3d/v3d_gen.h"
#include "helpers/v3d/v3d_util.h"
#include "helpers/gfx/gfx_util.h"

/* Is encoding in indices mode the same for all prim types? */
extern bool v3d_cl_has_common_ind_encoding(v3d_cl_compr_type_t type);

extern uint32_t v3d_cl_compr_size(
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt,
   v3d_cl_compr_type_t compr_type);

#define V3D_CL_MAX_COMPR_SIZE \
   GFX_MAX(V3D_CL_COMPR_IND_COMMON_MAX_PACKED_SIZE, \
   GFX_MAX(V3D_CL_COMPR_IND_TRI_MAX_PACKED_SIZE, \
   GFX_MAX(V3D_CL_COMPR_IND_D3DPVSF_TRI_MAX_PACKED_SIZE, \
   GFX_MAX(V3D_CL_COMPR_IND_LINE_MAX_PACKED_SIZE, \
   GFX_MAX(V3D_CL_COMPR_IND_POINT_MAX_PACKED_SIZE, \
   V3D_CL_COMPR_XY_TRI_MAX_PACKED_SIZE)))))

union v3d_cl_compr_union
{
   int32_t rel_branch;
   v3d_addr_t branch;

   V3D_CL_COMPR_IND_COMMON_IID8_T ind_common_iid8;
   V3D_CL_COMPR_IND_COMMON_IID32_T ind_common_iid32;

   V3D_CL_COMPR_IND_TRI_C0_T ind_tri_c0;
   V3D_CL_COMPR_IND_TRI_C1_T ind_tri_c1;
   V3D_CL_COMPR_IND_TRI_C2_T ind_tri_c2;
   V3D_CL_COMPR_IND_TRI_C3_T ind_tri_c3;
   V3D_CL_COMPR_IND_TRI_C4_T ind_tri_c4;
   V3D_CL_COMPR_IND_TRI_C5_T ind_tri_c5;

   V3D_CL_COMPR_IND_D3DPVSF_TRI_C0_T ind_d3dpvsf_tri_c0;
   V3D_CL_COMPR_IND_D3DPVSF_TRI_C1_T ind_d3dpvsf_tri_c1;
   V3D_CL_COMPR_IND_D3DPVSF_TRI_C2_T ind_d3dpvsf_tri_c2;
   V3D_CL_COMPR_IND_D3DPVSF_TRI_C3_T ind_d3dpvsf_tri_c3;
   V3D_CL_COMPR_IND_D3DPVSF_TRI_C4_T ind_d3dpvsf_tri_c4;
   V3D_CL_COMPR_IND_D3DPVSF_TRI_C5_T ind_d3dpvsf_tri_c5;

   V3D_CL_COMPR_IND_LINE_C0_T ind_line_c0;
   V3D_CL_COMPR_IND_LINE_C1_T ind_line_c1;
   V3D_CL_COMPR_IND_LINE_C2_T ind_line_c2;
   V3D_CL_COMPR_IND_LINE_C3_T ind_line_c3;
   V3D_CL_COMPR_IND_LINE_C4_T ind_line_c4;
   V3D_CL_COMPR_IND_LINE_C5_T ind_line_c5;

   V3D_CL_COMPR_IND_POINT_C0_T ind_point_c0;
   V3D_CL_COMPR_IND_POINT_C1_T ind_point_c1;
   V3D_CL_COMPR_IND_POINT_C2_T ind_point_c2;
   V3D_CL_COMPR_IND_POINT_C4_T ind_point_c4;
   V3D_CL_COMPR_IND_POINT_C5_T ind_point_c5;

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

extern void v3d_cl_logc_compr(
   VCOS_LOG_CAT_T *log_cat, VCOS_LOG_LEVEL_T level, const char *line_prefix,
   const uint8_t *packed_compr, const struct v3d_cl_source *source,
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt);

#define v3d_cl_log_trace_compr(LINE_PREFIX, PACKED_COMPR, SOURCE, PLIST_FMT) \
   v3d_cl_logc_compr(VCOS_LOG_CATEGORY, VCOS_LOG_TRACE, LINE_PREFIX, PACKED_COMPR, SOURCE, PLIST_FMT)

static inline v3d_addr_t v3d_cl_compr_rel_branch_dst(
   const struct v3d_cl_source *source, int32_t rel_addr)
{
   return v3d_addr_align_down(source->parts[0].addr, 32) + rel_addr;
}

#endif
