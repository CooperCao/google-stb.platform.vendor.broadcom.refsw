/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/

#include "lfmt.h"
#include "lfmt_desc_maps.h"

#include "libs/util/assert_helpers.h"
#include "vcos_string.h"

size_t gfx_lfmt_sprint(char *buf, size_t buf_size, size_t offset, GFX_LFMT_T lfmt)
{
   size_t begin_offset = offset;

   #define APPEND(S) offset = vcos_safe_sprintf(buf, buf_size, offset, "%s%s", (offset == begin_offset) ? "" : "_", S)
   #define APPEND_FIELD(FIELD, field)                                               \
      do                                                                            \
      {                                                                             \
         GFX_LFMT_##FIELD##_T field_ = gfx_lfmt_get_##field(&lfmt);                 \
         if (field_)                                                                \
            APPEND(desc_map_value_to_desc(&gfx_lfmt_##field##_desc_map, field_));   \
      } while (0)

   GFX_LFMT_T base_channels = GFX_LFMT_NONE;
   gfx_lfmt_set_base(&base_channels, gfx_lfmt_get_base(&lfmt));
   gfx_lfmt_set_channels(&base_channels, gfx_lfmt_get_channels(&lfmt));
   if (base_channels)
   {
      const char *base_channels_desc = desc_map_try_value_to_desc(
         &gfx_lfmt_base_channels_desc_map, base_channels);
      if (base_channels_desc)
         APPEND(base_channels_desc);
      else
      {
         APPEND_FIELD(BASE, base);
         APPEND_FIELD(CHANNELS, channels);
      }
   }

   APPEND_FIELD(TYPE, type);
   APPEND_FIELD(PRE, pre);
   APPEND_FIELD(DIMS, dims);
   APPEND_FIELD(SWIZZLING, swizzling);
   APPEND_FIELD(YFLIP, yflip);

   #undef APPEND_FIELD
   #undef APPEND

   if (offset == begin_offset)
   {
      assert(lfmt == GFX_LFMT_NONE);
      offset = vcos_safe_sprintf(buf, buf_size, offset, "NONE");
   }

   return offset;
}

const char *gfx_lfmt_desc(GFX_LFMT_T lfmt)
{
   static GFX_LFMT_SPRINT(desc, lfmt);
   return desc;
}

static const struct
{
   const struct desc_map *map;
   uint32_t mask;
} PARTS[] = {
   {&gfx_lfmt_dims_desc_map,           GFX_LFMT_DIMS_MASK},
   {&gfx_lfmt_swizzling_desc_map,      GFX_LFMT_SWIZZLING_MASK},
   {&gfx_lfmt_yflip_desc_map,          GFX_LFMT_YFLIP_MASK},
   {&gfx_lfmt_base_desc_map,           GFX_LFMT_BASE_MASK},
   {&gfx_lfmt_type_desc_map,           GFX_LFMT_TYPE_MASK},
   {&gfx_lfmt_channels_desc_map,       GFX_LFMT_CHANNELS_MASK},
   {&gfx_lfmt_pre_desc_map,            GFX_LFMT_PRE_MASK},
   /* Try combos last, or we might end up with a pessimistic mask (and thus
    * refuse something like C8C8_RG) */
   {&gfx_lfmt_base_channels_desc_map,  GFX_LFMT_BASE_MASK | GFX_LFMT_CHANNELS_MASK}};

bool gfx_lfmt_maybe_from_desc(GFX_LFMT_T *lfmt_out, const char *desc)
{
   if (!strcmp(desc, "NONE"))
   {
      return GFX_LFMT_NONE;
   }

   /* Repeatedly chop the longest matching part off the start of desc and OR
    * the part into lfmt */
   GFX_LFMT_T lfmt = GFX_LFMT_NONE;
   for (;;)
   {
      /* Find longest matching part at start of desc */
      const char *longest_remainder = desc; /* desc after longest matching part */
      GFX_LFMT_T longest_lfmt = GFX_LFMT_NONE; /* lfmt of longest matching part */
      uint32_t longest_mask = 0; /* Mask of fields specified by longest matching part */
      for (size_t i = 0; i != vcos_countof(PARTS); ++i)
      {
         uint32_t part_lfmt;
         const char *part_remainder = desc_map_past_longest_prefix(
            &part_lfmt, PARTS[i].map, desc);
         if (part_remainder > longest_remainder)
         {
            longest_remainder = part_remainder;
            longest_lfmt = (GFX_LFMT_T)part_lfmt;
            longest_mask = PARTS[i].mask;
         }
      }

      if (longest_remainder == desc)
         /* No matching part */
         return false;

      assert(longest_lfmt != GFX_LFMT_NONE);
      assert(longest_mask != 0);

      assert(!(longest_lfmt & ~longest_mask));

      if (lfmt & longest_mask)
         /* Specified a field more than once */
         return false;

      desc = longest_remainder;
      lfmt |= longest_lfmt;

      if (!*desc)
         break;

      if (*desc != '_')
         /* We expect an underscore between parts */
         return false;
      ++desc;
   }

   *lfmt_out = lfmt;
   return true;
}

GFX_LFMT_T gfx_lfmt_from_desc(const char *desc)
{
   GFX_LFMT_T lfmt;
   if (gfx_lfmt_maybe_from_desc(&lfmt, desc))
      return lfmt;
   unreachable();
   return GFX_LFMT_NONE;
}
