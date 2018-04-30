/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "lfmt_block.h"

#include "libs/core/v3d/v3d_common.h"
#include "libs/core/v3d/v3d_gen.h"
#include "libs/util/gfx_util/gfx_util_conv.h"

void gfx_lfmt_block_set_slot_bits(
   union gfx_lfmt_block_data *block,
   const GFX_LFMT_FMT_DETAIL_T *fd, uint32_t slot_idx, uint32_t bits)
{
   const struct gfx_lfmt_slot_detail *slot = &fd->slts[slot_idx];
   uint32_t word  = slot->shift / 32;
   uint32_t shift = slot->shift & 31;
   uint32_t mask  = gfx_mask(slot->bit_width) << shift;
   assert(shift + slot->bit_width <= 32);  // Slots can't straddle a 32-bit boundary

   block->ui32[word] = (block->ui32[word] & ~mask) | (bits << shift);
}

uint32_t gfx_lfmt_block_get_slot_bits(
   const union gfx_lfmt_block_data *block,
   const GFX_LFMT_FMT_DETAIL_T *fd, uint32_t slot_idx)
{
   const struct gfx_lfmt_slot_detail *slot = &fd->slts[slot_idx];
   uint32_t word  = slot->shift / 32;
   uint32_t shift = slot->shift & 31;
   uint32_t mask  = gfx_mask(slot->bit_width) << shift;
   assert(shift + slot->bit_width <= 32);  // Slots can't straddle a 32-bit boundary

   return (block->ui32[word] & mask) >> shift;
}

static size_t sprint_slot_desc(char *buf, size_t buf_size, size_t offset,
   const struct gfx_lfmt_slot_detail *slot, uint32_t slot_bits)
{
   offset = gfx_lfmt_sprint(buf, buf_size, offset, (GFX_LFMT_T)slot->channel);
   /* Ignore type of X channels -- always show hex */
   switch ((slot->channel == GFX_LFMT_CHANNELS_X) ? GFX_LFMT_TYPE_UINT : slot->type)
   {
   case GFX_LFMT_TYPE_FLOAT:
      switch (slot->bit_width)
      {
      case 16: return vcos_safe_sprintf(buf, buf_size, offset, "=%1.8e", gfx_float16_to_float(slot_bits));
      case 32: return vcos_safe_sprintf(buf, buf_size, offset, "=%1.8e", gfx_float_from_bits(slot_bits));
      default: unreachable(); return 0;
      }
   case GFX_LFMT_TYPE_UFLOAT:
      switch (slot->bit_width)
      {
      case 10: return vcos_safe_sprintf(buf, buf_size, offset, "=%1.8e", gfx_ufloat10_to_float(slot_bits));
      case 11: return vcos_safe_sprintf(buf, buf_size, offset, "=%1.8e", gfx_ufloat11_to_float(slot_bits));
      default: unreachable(); return 0;
      }
   case GFX_LFMT_TYPE_UNORM:
   case GFX_LFMT_TYPE_SNORM:
   case GFX_LFMT_TYPE_SRGB:
   case GFX_LFMT_TYPE_UINT:
   case GFX_LFMT_TYPE_INT:
      return vcos_safe_sprintf(buf, buf_size, offset, "=0x%0*" PRIx32, gfx_udiv_round_up(slot->bit_width, 4), slot_bits);
   default:
      unreachable();
      return 0;
   }
}

size_t gfx_lfmt_sprint_block(char *buf, size_t buf_size, size_t offset,
   const GFX_LFMT_BLOCK_T *block)
{
   if (gfx_lfmt_is_std(block->fmt))
   {
      GFX_LFMT_FMT_DETAIL_T fd;
      gfx_lfmt_fmt_detail(&fd, block->fmt);

      for (uint32_t slot_idx = 0; slot_idx != fd.num_slots; ++slot_idx)
      {
         struct gfx_lfmt_slot_detail *slot = &fd.slts[slot_idx];
         offset = sprint_slot_desc(buf, buf_size, offset,
            slot, gfx_lfmt_block_get_slot_bits(&block->u, &fd, slot_idx));
         if (slot_idx != fd.num_slots - 1)
            offset = vcos_safe_sprintf(buf, buf_size, offset, " ");
      }

      offset = vcos_safe_sprintf(buf, buf_size, offset, "\n");
   }
   else if (gfx_lfmt_is_bstc_family(block->fmt))
   {
      struct v3d_basic_string_printer printer;
      v3d_basic_string_printer_init(&printer, buf, buf_size, offset);
      v3d_print_bstc_block(block->u.ui8, &printer.base.base);
      offset = printer.offset;
   }
   else
      offset = gfx_lfmt_sprint_block_raw(buf, buf_size, offset, block, /*compact=*/false);

   return offset;
}

size_t gfx_lfmt_sprint_block_raw(char *buf, size_t buf_size, size_t offset,
   const GFX_LFMT_BLOCK_T *block, bool compact)
{
   uint32_t bytes_per_block = gfx_lfmt_bytes_per_block(block->fmt);
   for (uint32_t i = 0; i < bytes_per_block; ++i)
   {
      offset = vcos_safe_sprintf(buf, buf_size, offset, "%02x", (unsigned int)block->u.ui8[i]);

      bool last = i == bytes_per_block - 1;
      if (!compact && (((i & 7) == 7) || last))
         offset = vcos_safe_sprintf(buf, buf_size, offset, "\n");
      else if (!last)
         offset = vcos_safe_sprintf(buf, buf_size, offset, " ");
   }

   return offset;
}

const char *gfx_lfmt_desc_block(const GFX_LFMT_BLOCK_T *block)
{
   static GFX_LFMT_SPRINT_BLOCK(desc, block);
   return desc;
}

const char *gfx_lfmt_desc_block_raw(const GFX_LFMT_BLOCK_T *block, bool compact)
{
   static GFX_LFMT_SPRINT_BLOCK_RAW(desc, block, compact);
   return desc;
}
