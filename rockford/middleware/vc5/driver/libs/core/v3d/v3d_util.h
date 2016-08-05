/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include "v3d_common.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "libs/util/assert_helpers.h"

VCOS_EXTERN_C_BEGIN

static inline v3d_addr_t v3d_addr_min(v3d_addr_t a, v3d_addr_t b)
{
   return (a < b) ? a : b;
}

static inline v3d_addr_t v3d_addr_max(v3d_addr_t a, v3d_addr_t b)
{
   return (a > b) ? a : b;
}

static inline v3d_addr_t v3d_addr_offset(v3d_addr_t addr, size_t offset)
{
   v3d_addr_t offset_addr;
   assert(offset <= (v3d_addr_t)-1);
   offset_addr = (v3d_addr_t)(addr + offset);
   assert(offset_addr >= addr);
   return offset_addr;
}

static inline v3d_addr_t v3d_addr_align_down(v3d_addr_t addr, uint32_t align)
{
   assert(gfx_is_power_of_2(align));
   return addr & ~(v3d_addr_t)(align - 1);
}

static inline v3d_addr_t v3d_addr_align_up(v3d_addr_t addr, uint32_t align)
{
   assert(gfx_is_power_of_2(align));
   return v3d_addr_offset(addr, align - 1) & ~(v3d_addr_t)(align - 1);
}

static inline bool v3d_addr_aligned(v3d_addr_t addr, uint32_t align)
{
   assert(gfx_is_power_of_2(align));
   return !(addr & (align - 1));
}

#ifndef NDEBUG
#define v3d_assert_addr_range_within(BEGIN, SIZE, PERMITTED_BEGIN, PERMITTED_SIZE)                 \
   do                                                                                              \
   {                                                                                               \
      v3d_addr_t begin_ = (BEGIN), permitted_begin_ = (PERMITTED_BEGIN);                           \
      size_t size_ = (SIZE), permitted_size_ = (PERMITTED_SIZE);                                   \
      assert_msg(                                                                                  \
         (begin_ >= permitted_begin_) &&                                                           \
         (v3d_addr_offset(begin_, size_) <= v3d_addr_offset(permitted_begin_, permitted_size_)),   \
         "Address range 0x%08x..0x%08x outside permitted range 0x%08x..0x%08x",                    \
         begin_, v3d_addr_offset(begin_, size_),                                                   \
         permitted_begin_, v3d_addr_offset(permitted_begin_, permitted_size_));                    \
   } while (0)
#else
#define v3d_assert_addr_range_within(BEGIN, SIZE, PERMITTED_BEGIN, PERMITTED_SIZE)
#endif

VCOS_EXTERN_C_END
