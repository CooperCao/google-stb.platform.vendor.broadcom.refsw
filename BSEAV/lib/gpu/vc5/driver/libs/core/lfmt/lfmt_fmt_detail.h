/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "lfmt.h"
#include "libs/util/common.h"

EXTERN_C_BEGIN

#define GFX_LFMT_MAX_SLOTS 4
#define GFX_LFMT_FMT_DETAIL_INVALID_SLOT UINT32_MAX

struct gfx_lfmt_slot_detail
{
   uint32_t shift; /* Bit offset that this slot starts at */
   uint32_t bit_width; /* Bit width of slot */
   GFX_LFMT_TYPE_T type;
   GFX_LFMT_CHANNELS_T channel;
};

typedef struct
{
   uint32_t num_slots;
   struct gfx_lfmt_slot_detail slts[GFX_LFMT_MAX_SLOTS];
} GFX_LFMT_FMT_DETAIL_T;

extern void gfx_lfmt_fmt_detail(GFX_LFMT_FMT_DETAIL_T *fd, GFX_LFMT_T lfmt);

/* Returns GFX_LFMT_FMT_DETAIL_INVALID_SLOT if channel not found */
static inline uint32_t gfx_lfmt_fmt_detail_get_slot_by_channel_maybe(
   const GFX_LFMT_FMT_DETAIL_T *fd, GFX_LFMT_CHANNELS_T channel)
{
   for (uint32_t i = 0; i != fd->num_slots; ++i)
      if (fd->slts[i].channel == channel)
         return i;
   return GFX_LFMT_FMT_DETAIL_INVALID_SLOT;
}

static inline uint32_t gfx_lfmt_fmt_detail_get_slot_by_channel(
   const GFX_LFMT_FMT_DETAIL_T *fd, GFX_LFMT_CHANNELS_T channel)
{
   uint32_t i = gfx_lfmt_fmt_detail_get_slot_by_channel_maybe(fd, channel);
   assert(i != GFX_LFMT_FMT_DETAIL_INVALID_SLOT);
   return i;
}

EXTERN_C_END
