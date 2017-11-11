/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/util/common.h"

EXTERN_C_BEGIN

// Choose sensible supertile size
void v3d_choose_supertile_sizes(
   unsigned tiles_x, unsigned tiles_y,
   unsigned min_supertile_w, unsigned min_supertile_h, unsigned max_supertiles,
   unsigned *supertile_w, unsigned *supertile_h);

// Calculate supertile range
void v3d_supertile_range(
   uint32_t *morton_flags, uint32_t *begin_supertile, uint32_t *end_supertile,
   uint32_t num_cores, uint32_t core,
   unsigned int num_supertiles_x, unsigned int num_supertiles_y,
   bool partition_supertiles_in_sw, bool all_cores_same_st_order);

EXTERN_C_END
