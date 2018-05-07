/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "lfmt_fmt_detail.h"

static void set_all_types(GFX_LFMT_FMT_DETAIL_T *fd, GFX_LFMT_TYPE_T type)
{
   for (uint32_t i = 0; i != GFX_LFMT_MAX_SLOTS; ++i)
      fd->slts[i].type = type;
}

void gfx_lfmt_fmt_detail(GFX_LFMT_FMT_DETAIL_T *fd, GFX_LFMT_T lfmt)
{
   /* BEGIN AUTO-GENERATED CODE (fmt_detail_base) */
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
      fd->num_slots = 4;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 32;
      fd->slts[1].shift = 32;
      fd->slts[1].bit_width = 32;
      fd->slts[2].shift = 64;
      fd->slts[2].bit_width = 32;
      fd->slts[3].shift = 96;
      fd->slts[3].bit_width = 32;
      break;
   case GFX_LFMT_BASE_C32_C32_C32:
      fd->num_slots = 3;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 32;
      fd->slts[1].shift = 32;
      fd->slts[1].bit_width = 32;
      fd->slts[2].shift = 64;
      fd->slts[2].bit_width = 32;
      break;
   case GFX_LFMT_BASE_C32_C32:
      fd->num_slots = 2;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 32;
      fd->slts[1].shift = 32;
      fd->slts[1].bit_width = 32;
      break;
   case GFX_LFMT_BASE_C32:
      fd->num_slots = 1;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 32;
      break;
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
      fd->num_slots = 4;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 16;
      fd->slts[1].shift = 16;
      fd->slts[1].bit_width = 16;
      fd->slts[2].shift = 32;
      fd->slts[2].bit_width = 16;
      fd->slts[3].shift = 48;
      fd->slts[3].bit_width = 16;
      break;
   case GFX_LFMT_BASE_C16_C16_C16:
      fd->num_slots = 3;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 16;
      fd->slts[1].shift = 16;
      fd->slts[1].bit_width = 16;
      fd->slts[2].shift = 32;
      fd->slts[2].bit_width = 16;
      break;
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
      fd->num_slots = 2;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 16;
      fd->slts[1].shift = 16;
      fd->slts[1].bit_width = 16;
      break;
   case GFX_LFMT_BASE_C16:
      fd->num_slots = 1;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 16;
      break;
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
      fd->num_slots = 4;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 8;
      fd->slts[1].shift = 8;
      fd->slts[1].bit_width = 8;
      fd->slts[2].shift = 16;
      fd->slts[2].bit_width = 8;
      fd->slts[3].shift = 24;
      fd->slts[3].bit_width = 8;
      break;
   case GFX_LFMT_BASE_C8_C8_C8:
      fd->num_slots = 3;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 8;
      fd->slts[1].shift = 8;
      fd->slts[1].bit_width = 8;
      fd->slts[2].shift = 16;
      fd->slts[2].bit_width = 8;
      break;
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
      fd->num_slots = 2;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 8;
      fd->slts[1].shift = 8;
      fd->slts[1].bit_width = 8;
      break;
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C8_2X2:
      fd->num_slots = 1;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 8;
      break;
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
      fd->num_slots = 1;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 4;
      break;
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
      fd->num_slots = 1;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 1;
      break;
   case GFX_LFMT_BASE_C10C10C10C2:
      fd->num_slots = 4;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 10;
      fd->slts[1].shift = 10;
      fd->slts[1].bit_width = 10;
      fd->slts[2].shift = 20;
      fd->slts[2].bit_width = 10;
      fd->slts[3].shift = 30;
      fd->slts[3].bit_width = 2;
      break;
   case GFX_LFMT_BASE_C11C11C10:
      fd->num_slots = 3;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 11;
      fd->slts[1].shift = 11;
      fd->slts[1].bit_width = 11;
      fd->slts[2].shift = 22;
      fd->slts[2].bit_width = 10;
      break;
   case GFX_LFMT_BASE_C5C6C5:
      fd->num_slots = 3;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 5;
      fd->slts[1].shift = 5;
      fd->slts[1].bit_width = 6;
      fd->slts[2].shift = 11;
      fd->slts[2].bit_width = 5;
      break;
   case GFX_LFMT_BASE_C5C5C5C1:
      fd->num_slots = 4;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 5;
      fd->slts[1].shift = 5;
      fd->slts[1].bit_width = 5;
      fd->slts[2].shift = 10;
      fd->slts[2].bit_width = 5;
      fd->slts[3].shift = 15;
      fd->slts[3].bit_width = 1;
      break;
   case GFX_LFMT_BASE_C1C5C5C5:
      fd->num_slots = 4;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 1;
      fd->slts[1].shift = 1;
      fd->slts[1].bit_width = 5;
      fd->slts[2].shift = 6;
      fd->slts[2].bit_width = 5;
      fd->slts[3].shift = 11;
      fd->slts[3].bit_width = 5;
      break;
   case GFX_LFMT_BASE_C4C4C4C4:
      fd->num_slots = 4;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 4;
      fd->slts[1].shift = 4;
      fd->slts[1].bit_width = 4;
      fd->slts[2].shift = 8;
      fd->slts[2].bit_width = 4;
      fd->slts[3].shift = 12;
      fd->slts[3].bit_width = 4;
      break;
   case GFX_LFMT_BASE_C32_C8X24:
      fd->num_slots = 2;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 32;
      fd->slts[1].shift = 32;
      fd->slts[1].bit_width = 8;
      break;
   case GFX_LFMT_BASE_C24C8:
      fd->num_slots = 2;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 24;
      fd->slts[1].shift = 24;
      fd->slts[1].bit_width = 8;
      break;
   case GFX_LFMT_BASE_C8C24:
      fd->num_slots = 2;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 8;
      fd->slts[1].shift = 8;
      fd->slts[1].bit_width = 24;
      break;
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
      fd->num_slots = 4;
      fd->slts[0].shift = 0;
      fd->slts[0].bit_width = 15;
      fd->slts[1].shift = 16;
      fd->slts[1].bit_width = 15;
      fd->slts[2].shift = 32;
      fd->slts[2].bit_width = 15;
      fd->slts[3].shift = 48;
      fd->slts[3].bit_width = 15;
      break;
   default:
      unreachable();
   }
   /* END AUTO-GENERATED CODE (fmt_detail_base) */

   /* BEGIN AUTO-GENERATED CODE (fmt_detail_type) */
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_UFLOAT:
      set_all_types(fd, GFX_LFMT_TYPE_UFLOAT);
      break;
   case GFX_LFMT_TYPE_FLOAT:
      set_all_types(fd, GFX_LFMT_TYPE_FLOAT);
      break;
   case GFX_LFMT_TYPE_UINT:
      set_all_types(fd, GFX_LFMT_TYPE_UINT);
      break;
   case GFX_LFMT_TYPE_INT:
      set_all_types(fd, GFX_LFMT_TYPE_INT);
      break;
   case GFX_LFMT_TYPE_UNORM:
      set_all_types(fd, GFX_LFMT_TYPE_UNORM);
      break;
   case GFX_LFMT_TYPE_SRGB:
      set_all_types(fd, GFX_LFMT_TYPE_SRGB);
      break;
   case GFX_LFMT_TYPE_SNORM:
      set_all_types(fd, GFX_LFMT_TYPE_SNORM);
      break;
   case GFX_LFMT_TYPE_FLOAT_UINT:
      assert(fd->num_slots == 2);
      fd->slts[0].type = GFX_LFMT_TYPE_FLOAT;
      fd->slts[1].type = GFX_LFMT_TYPE_UINT;
      break;
   case GFX_LFMT_TYPE_UINT_FLOAT:
      assert(fd->num_slots == 2);
      fd->slts[0].type = GFX_LFMT_TYPE_UINT;
      fd->slts[1].type = GFX_LFMT_TYPE_FLOAT;
      break;
   case GFX_LFMT_TYPE_FLOAT_UNORM:
      assert(fd->num_slots == 2);
      fd->slts[0].type = GFX_LFMT_TYPE_FLOAT;
      fd->slts[1].type = GFX_LFMT_TYPE_UNORM;
      break;
   case GFX_LFMT_TYPE_UNORM_FLOAT:
      assert(fd->num_slots == 2);
      fd->slts[0].type = GFX_LFMT_TYPE_UNORM;
      fd->slts[1].type = GFX_LFMT_TYPE_FLOAT;
      break;
   case GFX_LFMT_TYPE_UNORM_UINT:
      assert(fd->num_slots == 2);
      fd->slts[0].type = GFX_LFMT_TYPE_UNORM;
      fd->slts[1].type = GFX_LFMT_TYPE_UINT;
      break;
   case GFX_LFMT_TYPE_UINT_UNORM:
      assert(fd->num_slots == 2);
      fd->slts[0].type = GFX_LFMT_TYPE_UINT;
      fd->slts[1].type = GFX_LFMT_TYPE_UNORM;
      break;
   case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
      assert(fd->num_slots == 4);
      fd->slts[0].type = GFX_LFMT_TYPE_SRGB;
      fd->slts[1].type = GFX_LFMT_TYPE_SRGB;
      fd->slts[2].type = GFX_LFMT_TYPE_SRGB;
      fd->slts[3].type = GFX_LFMT_TYPE_UNORM;
      break;
   case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
      assert(fd->num_slots == 4);
      fd->slts[0].type = GFX_LFMT_TYPE_UNORM;
      fd->slts[1].type = GFX_LFMT_TYPE_SRGB;
      fd->slts[2].type = GFX_LFMT_TYPE_SRGB;
      fd->slts[3].type = GFX_LFMT_TYPE_SRGB;
      break;
   default:
      unreachable();
   }
   /* END AUTO-GENERATED CODE (fmt_detail_type) */

   /* BEGIN AUTO-GENERATED CODE (fmt_detail_channels) */
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
      assert(fd->num_slots == 1);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_R;
      break;
   case GFX_LFMT_CHANNELS_G:
      assert(fd->num_slots == 1);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_G;
      break;
   case GFX_LFMT_CHANNELS_B:
      assert(fd->num_slots == 1);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_B;
      break;
   case GFX_LFMT_CHANNELS_A:
      assert(fd->num_slots == 1);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_A;
      break;
   case GFX_LFMT_CHANNELS_RG:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_G;
      break;
   case GFX_LFMT_CHANNELS_GR:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_R;
      break;
   case GFX_LFMT_CHANNELS_RA:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_A;
      break;
   case GFX_LFMT_CHANNELS_AR:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_A;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_R;
      break;
   case GFX_LFMT_CHANNELS_RGB:
      assert(fd->num_slots == 3);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_B;
      break;
   case GFX_LFMT_CHANNELS_BGR:
      assert(fd->num_slots == 3);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_B;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_R;
      break;
   case GFX_LFMT_CHANNELS_RGBA:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_B;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_A;
      break;
   case GFX_LFMT_CHANNELS_BGRA:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_B;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_A;
      break;
   case GFX_LFMT_CHANNELS_ARGB:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_A;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_B;
      break;
   case GFX_LFMT_CHANNELS_ABGR:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_A;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_B;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_R;
      break;
   case GFX_LFMT_CHANNELS_X:
      assert(fd->num_slots == 1);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_X;
      break;
   case GFX_LFMT_CHANNELS_RX:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_X;
      break;
   case GFX_LFMT_CHANNELS_XR:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_R;
      break;
   case GFX_LFMT_CHANNELS_XG:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_G;
      break;
   case GFX_LFMT_CHANNELS_GX:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_X;
      break;
   case GFX_LFMT_CHANNELS_RGX:
      assert(fd->num_slots == 3);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_X;
      break;
   case GFX_LFMT_CHANNELS_XGR:
      assert(fd->num_slots == 3);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_R;
      break;
   case GFX_LFMT_CHANNELS_RGBX:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_B;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_X;
      break;
   case GFX_LFMT_CHANNELS_BGRX:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_B;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_X;
      break;
   case GFX_LFMT_CHANNELS_XRGB:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_B;
      break;
   case GFX_LFMT_CHANNELS_XBGR:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_B;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_G;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_R;
      break;
   case GFX_LFMT_CHANNELS_RXXX:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_X;
      break;
   case GFX_LFMT_CHANNELS_XXXR:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_R;
      break;
   case GFX_LFMT_CHANNELS_RAXX:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_R;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_A;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_X;
      break;
   case GFX_LFMT_CHANNELS_XXAR:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_A;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_R;
      break;
   case GFX_LFMT_CHANNELS_L:
      assert(fd->num_slots == 1);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_L;
      break;
   case GFX_LFMT_CHANNELS_LA:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_L;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_A;
      break;
   case GFX_LFMT_CHANNELS_AL:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_A;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_L;
      break;
   case GFX_LFMT_CHANNELS_LX:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_L;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_X;
      break;
   case GFX_LFMT_CHANNELS_XL:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_L;
      break;
   case GFX_LFMT_CHANNELS_D:
      assert(fd->num_slots == 1);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_D;
      break;
   case GFX_LFMT_CHANNELS_S:
      assert(fd->num_slots == 1);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_S;
      break;
   case GFX_LFMT_CHANNELS_DS:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_D;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_S;
      break;
   case GFX_LFMT_CHANNELS_SD:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_S;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_D;
      break;
   case GFX_LFMT_CHANNELS_DX:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_D;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_X;
      break;
   case GFX_LFMT_CHANNELS_XD:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_D;
      break;
   case GFX_LFMT_CHANNELS_SX:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_S;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_X;
      break;
   case GFX_LFMT_CHANNELS_XS:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_X;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_S;
      break;
   case GFX_LFMT_CHANNELS_Y:
      assert(fd->num_slots == 1);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_Y;
      break;
   case GFX_LFMT_CHANNELS_U:
      assert(fd->num_slots == 1);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_U;
      break;
   case GFX_LFMT_CHANNELS_V:
      assert(fd->num_slots == 1);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_V;
      break;
   case GFX_LFMT_CHANNELS_UV:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_U;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_V;
      break;
   case GFX_LFMT_CHANNELS_VU:
      assert(fd->num_slots == 2);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_V;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_U;
      break;
   case GFX_LFMT_CHANNELS_YUV:
      assert(fd->num_slots == 3);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_Y;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_U;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_V;
      break;
   case GFX_LFMT_CHANNELS_VUY:
      assert(fd->num_slots == 3);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_V;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_U;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_Y;
      break;
   case GFX_LFMT_CHANNELS_YUYV:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_Y;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_U;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_Y;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_V;
      break;
   case GFX_LFMT_CHANNELS_VYUY:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_V;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_Y;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_U;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_Y;
      break;
   case GFX_LFMT_CHANNELS_YYUV:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_Y;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_Y;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_U;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_V;
      break;
   case GFX_LFMT_CHANNELS_VUYY:
      assert(fd->num_slots == 4);
      fd->slts[0].channel = GFX_LFMT_CHANNELS_V;
      fd->slts[1].channel = GFX_LFMT_CHANNELS_U;
      fd->slts[2].channel = GFX_LFMT_CHANNELS_Y;
      fd->slts[3].channel = GFX_LFMT_CHANNELS_Y;
      break;
   default:
      unreachable();
   }
   /* END AUTO-GENERATED CODE (fmt_detail_channels) */
}
