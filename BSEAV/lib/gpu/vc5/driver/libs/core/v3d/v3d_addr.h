/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdint.h>

typedef uint32_t v3d_addr_t;
typedef uint32_t v3d_size_t;

typedef struct v3d_addr_range
{
   v3d_addr_t begin;
   v3d_addr_t end;

#ifdef __cplusplus
   v3d_addr_range(v3d_addr_t begin, v3d_addr_t end) : begin(begin), end(end) {}

   bool overlaps(const v3d_addr_range &other) const
   {
      return (begin < other.end) && (other.begin < end);
   }

   bool contains(const v3d_addr_range &other) const
   {
      return (begin <= other.begin) && (other.end <= end);
   }
#endif
} v3d_addr_range;
