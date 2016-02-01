/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef V3D_VPM_ALLOC_H
#define V3D_VPM_ALLOC_H

#include "helpers/gfx/gfx_util.h"

#define V3D_VPM_ROWS_PER_BLOCK 8

static inline uint32_t v3d_cvs_input_size(
   bool vertex_id, bool instance_id,
   uint32_t num_attr_rows)
{
   return gfx_udiv_round_up(
      (vertex_id ? 1 : 0) + (instance_id ? 1 : 0) + num_attr_rows,
      V3D_VPM_ROWS_PER_BLOCK);
}

static inline uint32_t v3d_cs_output_size(
   bool point_size, uint32_t num_tf_values)
{
   return gfx_udiv_round_up(
      /* 6 = clip header + x/y */
      6 + (point_size ? 1 : 0) + num_tf_values,
      V3D_VPM_ROWS_PER_BLOCK);
}

static inline uint32_t v3d_vs_output_size(
   bool point_size, uint32_t num_varys)
{
   return gfx_udiv_round_up(
      /* 4 = x/y + z + 1/w */
      4 + (point_size ? 1 : 0) + num_varys,
      V3D_VPM_ROWS_PER_BLOCK);
}

#endif
