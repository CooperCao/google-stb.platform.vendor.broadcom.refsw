/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "lfmt.h"

void gfx_lfmt_base_detail(GFX_LFMT_BASE_DETAIL_T *bd, GFX_LFMT_T lfmt)
{
   assert((lfmt & GFX_LFMT_BASE_MASK) != GFX_LFMT_BASE_NONE);

   /* BEGIN AUTO-GENERATED CODE (fill_bd_bpb_bpw) */
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
   case GFX_LFMT_BASE_BC2:
   case GFX_LFMT_BASE_BC3:
   case GFX_LFMT_BASE_BC5:
   case GFX_LFMT_BASE_EAC_EAC:
   case GFX_LFMT_BASE_ETC2_EAC:
   case GFX_LFMT_BASE_ASTC4X4:
   case GFX_LFMT_BASE_ASTC5X4:
   case GFX_LFMT_BASE_ASTC5X5:
   case GFX_LFMT_BASE_ASTC6X5:
   case GFX_LFMT_BASE_ASTC6X6:
   case GFX_LFMT_BASE_ASTC8X5:
   case GFX_LFMT_BASE_ASTC8X6:
   case GFX_LFMT_BASE_ASTC8X8:
   case GFX_LFMT_BASE_ASTC10X5:
   case GFX_LFMT_BASE_ASTC10X6:
   case GFX_LFMT_BASE_ASTC10X8:
   case GFX_LFMT_BASE_ASTC10X10:
   case GFX_LFMT_BASE_ASTC12X10:
   case GFX_LFMT_BASE_ASTC12X12:
      bd->bytes_per_block = 16;
      bd->bytes_per_word = 4;
      break;
   case GFX_LFMT_BASE_C32_C32_C32:
      bd->bytes_per_block = 12;
      bd->bytes_per_word = 4;
      break;
   case GFX_LFMT_BASE_C32_C32:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C32_C8X24:
   case GFX_LFMT_BASE_BC1:
   case GFX_LFMT_BASE_BC4:
   case GFX_LFMT_BASE_EAC:
   case GFX_LFMT_BASE_ETC2:
   case GFX_LFMT_BASE_ETC1:
   case GFX_LFMT_BASE_PUNCHTHROUGH_ETC2:
      bd->bytes_per_block = 8;
      bd->bytes_per_word = 4;
      break;
   case GFX_LFMT_BASE_C32:
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C10C10C10C2:
   case GFX_LFMT_BASE_C11C11C10:
   case GFX_LFMT_BASE_C24C8:
   case GFX_LFMT_BASE_C8C24:
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
      bd->bytes_per_block = 4;
      bd->bytes_per_word = 4;
      break;
   case GFX_LFMT_BASE_C16C16C16C16:
      bd->bytes_per_block = 8;
      bd->bytes_per_word = 8;
      break;
   case GFX_LFMT_BASE_C16_C16_C16_C16:
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
      bd->bytes_per_block = 8;
      bd->bytes_per_word = 2;
      break;
   case GFX_LFMT_BASE_C16_C16_C16:
      bd->bytes_per_block = 6;
      bd->bytes_per_word = 2;
      break;
   case GFX_LFMT_BASE_C16_C16:
      bd->bytes_per_block = 4;
      bd->bytes_per_word = 2;
      break;
   case GFX_LFMT_BASE_C16:
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C5C6C5:
   case GFX_LFMT_BASE_C5C5C5C1:
   case GFX_LFMT_BASE_C1C5C5C5:
   case GFX_LFMT_BASE_C4C4C4C4:
      bd->bytes_per_block = 2;
      bd->bytes_per_word = 2;
      break;
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
      bd->bytes_per_block = 4;
      bd->bytes_per_word = 1;
      break;
   case GFX_LFMT_BASE_C8_C8_C8:
      bd->bytes_per_block = 3;
      bd->bytes_per_word = 1;
      break;
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
      bd->bytes_per_block = 2;
      bd->bytes_per_word = 1;
      break;
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
   case GFX_LFMT_BASE_C8_2X2:
   case GFX_LFMT_BASE_P4BE_R8G8B8:
   case GFX_LFMT_BASE_P4BE_R8G8B8A8:
   case GFX_LFMT_BASE_P4BE_B5G6R5:
   case GFX_LFMT_BASE_P4BE_A4B4G4R4:
   case GFX_LFMT_BASE_P4BE_A1B5G5R5:
   case GFX_LFMT_BASE_P8_R8G8B8:
   case GFX_LFMT_BASE_P8_R8G8B8A8:
   case GFX_LFMT_BASE_P8_B5G6R5:
   case GFX_LFMT_BASE_P8_A4B4G4R4:
   case GFX_LFMT_BASE_P8_A1B5G5R5:
      bd->bytes_per_block = 1;
      bd->bytes_per_word = 1;
      break;
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
      bd->bytes_per_block = 32;
      bd->bytes_per_word = 1;
      break;
   default:
      unreachable();
   }
   /* END AUTO-GENERATED CODE (fill_bd_bpb_bpw) */

   /* BEGIN AUTO-GENERATED CODE (fill_bd_block_whd) */
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32:
   case GFX_LFMT_BASE_C32:
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
   case GFX_LFMT_BASE_C16_C16_C16:
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
   case GFX_LFMT_BASE_C16:
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8:
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C4X4:
   case GFX_LFMT_BASE_C1X7:
   case GFX_LFMT_BASE_C10C10C10C2:
   case GFX_LFMT_BASE_C11C11C10:
   case GFX_LFMT_BASE_C5C6C5:
   case GFX_LFMT_BASE_C5C5C5C1:
   case GFX_LFMT_BASE_C1C5C5C5:
   case GFX_LFMT_BASE_C4C4C4C4:
   case GFX_LFMT_BASE_C32_C8X24:
   case GFX_LFMT_BASE_C24C8:
   case GFX_LFMT_BASE_C8C24:
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
   case GFX_LFMT_BASE_P8_R8G8B8:
   case GFX_LFMT_BASE_P8_R8G8B8A8:
   case GFX_LFMT_BASE_P8_B5G6R5:
   case GFX_LFMT_BASE_P8_A4B4G4R4:
   case GFX_LFMT_BASE_P8_A1B5G5R5:
      bd->block_w = 1;
      bd->block_h = 1;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C8_C8_2X1:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
   case GFX_LFMT_BASE_P4BE_R8G8B8:
   case GFX_LFMT_BASE_P4BE_R8G8B8A8:
   case GFX_LFMT_BASE_P4BE_B5G6R5:
   case GFX_LFMT_BASE_P4BE_A4B4G4R4:
   case GFX_LFMT_BASE_P4BE_A1B5G5R5:
      bd->block_w = 2;
      bd->block_h = 1;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_C1:
      bd->block_w = 8;
      bd->block_h = 1;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X2:
      bd->block_w = 2;
      bd->block_h = 2;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_BC1:
   case GFX_LFMT_BASE_BC2:
   case GFX_LFMT_BASE_BC3:
   case GFX_LFMT_BASE_BC4:
   case GFX_LFMT_BASE_BC5:
   case GFX_LFMT_BASE_EAC:
   case GFX_LFMT_BASE_EAC_EAC:
   case GFX_LFMT_BASE_ETC2:
   case GFX_LFMT_BASE_ETC1:
   case GFX_LFMT_BASE_PUNCHTHROUGH_ETC2:
   case GFX_LFMT_BASE_ETC2_EAC:
   case GFX_LFMT_BASE_ASTC4X4:
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
      bd->block_w = 4;
      bd->block_h = 4;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC5X4:
      bd->block_w = 5;
      bd->block_h = 4;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC5X5:
      bd->block_w = 5;
      bd->block_h = 5;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC6X5:
      bd->block_w = 6;
      bd->block_h = 5;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC6X6:
      bd->block_w = 6;
      bd->block_h = 6;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC8X5:
      bd->block_w = 8;
      bd->block_h = 5;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC8X6:
      bd->block_w = 8;
      bd->block_h = 6;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC8X8:
      bd->block_w = 8;
      bd->block_h = 8;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC10X5:
      bd->block_w = 10;
      bd->block_h = 5;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC10X6:
      bd->block_w = 10;
      bd->block_h = 6;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC10X8:
      bd->block_w = 10;
      bd->block_h = 8;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC10X10:
      bd->block_w = 10;
      bd->block_h = 10;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC12X10:
      bd->block_w = 12;
      bd->block_h = 10;
      bd->block_d = 1;
      break;
   case GFX_LFMT_BASE_ASTC12X12:
      bd->block_w = 12;
      bd->block_h = 12;
      bd->block_d = 1;
      break;
   default:
      unreachable();
   }
   /* END AUTO-GENERATED CODE (fill_bd_block_whd) */

   switch (bd->bytes_per_block) {
   case 1:
      if ((lfmt & GFX_LFMT_BASE_MASK) == GFX_LFMT_BASE_C1)
      {
         bd->ut_w_in_blocks_1d = 64;
         bd->ut_w_in_blocks_2d = 4;
         bd->ut_h_in_blocks_2d = 16;
      }
      else
      {
         bd->ut_w_in_blocks_1d = 64;
         bd->ut_w_in_blocks_2d = 8;
         bd->ut_h_in_blocks_2d = 8;
      }
      break;
   case 2:
   case 4:
      bd->ut_w_in_blocks_1d = 64 / bd->bytes_per_block;
      bd->ut_w_in_blocks_2d = 16 / bd->bytes_per_block;
      bd->ut_h_in_blocks_2d = 4;
      break;
   case 8:
      bd->ut_w_in_blocks_1d = 8;
      bd->ut_w_in_blocks_2d = 4;
      bd->ut_h_in_blocks_2d = 2;
      break;
   case 16:
      bd->ut_w_in_blocks_1d = 4;
      bd->ut_w_in_blocks_2d = 2;
      bd->ut_h_in_blocks_2d = 2;
      break;
   default:
      /* base format incompatible with utiles: just set these fields to 0 */
      bd->ut_w_in_blocks_1d = 0;
      bd->ut_w_in_blocks_2d = 0;
      bd->ut_h_in_blocks_2d = 0;
   }
}

/* BEGIN AUTO-GENERATED CODE (misc_func_defns) */
bool gfx_lfmt_dims_and_swizzling_compatible(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_DIMS_MASK) {
   case GFX_LFMT_DIMS_1D:
      switch (lfmt & GFX_LFMT_SWIZZLING_MASK) {
      case GFX_LFMT_SWIZZLING_RSO:
         return true;
      case GFX_LFMT_SWIZZLING_LT:
      case GFX_LFMT_SWIZZLING_UIF:
      case GFX_LFMT_SWIZZLING_UIF_XOR:
      case GFX_LFMT_SWIZZLING_UIF_NOUTILE:
      case GFX_LFMT_SWIZZLING_UIF_NOUTILE_XOR:
      case GFX_LFMT_SWIZZLING_UBLINEAR:
      case GFX_LFMT_SWIZZLING_SAND_128:
      case GFX_LFMT_SWIZZLING_SAND_256:
         return false;
      default:
         unreachable();
         return false;
      }
      break;
   case GFX_LFMT_DIMS_2D:
      switch (lfmt & GFX_LFMT_SWIZZLING_MASK) {
      case GFX_LFMT_SWIZZLING_RSO:
      case GFX_LFMT_SWIZZLING_LT:
      case GFX_LFMT_SWIZZLING_UIF:
      case GFX_LFMT_SWIZZLING_UIF_XOR:
      case GFX_LFMT_SWIZZLING_UIF_NOUTILE:
      case GFX_LFMT_SWIZZLING_UIF_NOUTILE_XOR:
      case GFX_LFMT_SWIZZLING_UBLINEAR:
      case GFX_LFMT_SWIZZLING_SAND_128:
      case GFX_LFMT_SWIZZLING_SAND_256:
         return true;
      default:
         unreachable();
         return false;
      }
      break;
   case GFX_LFMT_DIMS_3D:
      switch (lfmt & GFX_LFMT_SWIZZLING_MASK) {
      case GFX_LFMT_SWIZZLING_RSO:
      case GFX_LFMT_SWIZZLING_LT:
      case GFX_LFMT_SWIZZLING_UIF:
      case GFX_LFMT_SWIZZLING_UIF_XOR:
      case GFX_LFMT_SWIZZLING_UIF_NOUTILE:
      case GFX_LFMT_SWIZZLING_UIF_NOUTILE_XOR:
      case GFX_LFMT_SWIZZLING_UBLINEAR:
         return true;
      case GFX_LFMT_SWIZZLING_SAND_128:
      case GFX_LFMT_SWIZZLING_SAND_256:
         return false;
      default:
         unreachable();
         return false;
      }
      break;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_is_uif_family(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_SWIZZLING_MASK) {
   case GFX_LFMT_SWIZZLING_RSO:
   case GFX_LFMT_SWIZZLING_LT:
   case GFX_LFMT_SWIZZLING_UBLINEAR:
   case GFX_LFMT_SWIZZLING_SAND_128:
   case GFX_LFMT_SWIZZLING_SAND_256:
      return false;
   case GFX_LFMT_SWIZZLING_UIF:
   case GFX_LFMT_SWIZZLING_UIF_XOR:
   case GFX_LFMT_SWIZZLING_UIF_NOUTILE:
   case GFX_LFMT_SWIZZLING_UIF_NOUTILE_XOR:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_is_uif_xor_family(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_SWIZZLING_MASK) {
   case GFX_LFMT_SWIZZLING_RSO:
   case GFX_LFMT_SWIZZLING_LT:
   case GFX_LFMT_SWIZZLING_UIF:
   case GFX_LFMT_SWIZZLING_UIF_NOUTILE:
   case GFX_LFMT_SWIZZLING_UBLINEAR:
   case GFX_LFMT_SWIZZLING_SAND_128:
   case GFX_LFMT_SWIZZLING_SAND_256:
      return false;
   case GFX_LFMT_SWIZZLING_UIF_XOR:
   case GFX_LFMT_SWIZZLING_UIF_NOUTILE_XOR:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_is_noutile_family(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_SWIZZLING_MASK) {
   case GFX_LFMT_SWIZZLING_RSO:
   case GFX_LFMT_SWIZZLING_LT:
   case GFX_LFMT_SWIZZLING_UIF:
   case GFX_LFMT_SWIZZLING_UIF_XOR:
   case GFX_LFMT_SWIZZLING_UBLINEAR:
   case GFX_LFMT_SWIZZLING_SAND_128:
   case GFX_LFMT_SWIZZLING_SAND_256:
      return false;
   case GFX_LFMT_SWIZZLING_UIF_NOUTILE:
   case GFX_LFMT_SWIZZLING_UIF_NOUTILE_XOR:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_is_sand_family(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_SWIZZLING_MASK) {
   case GFX_LFMT_SWIZZLING_RSO:
   case GFX_LFMT_SWIZZLING_LT:
   case GFX_LFMT_SWIZZLING_UIF:
   case GFX_LFMT_SWIZZLING_UIF_XOR:
   case GFX_LFMT_SWIZZLING_UIF_NOUTILE:
   case GFX_LFMT_SWIZZLING_UIF_NOUTILE_XOR:
   case GFX_LFMT_SWIZZLING_UBLINEAR:
      return false;
   case GFX_LFMT_SWIZZLING_SAND_128:
   case GFX_LFMT_SWIZZLING_SAND_256:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_is_compressed(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32:
   case GFX_LFMT_BASE_C32:
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
   case GFX_LFMT_BASE_C16_C16_C16:
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
   case GFX_LFMT_BASE_C16:
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8:
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
   case GFX_LFMT_BASE_C10C10C10C2:
   case GFX_LFMT_BASE_C11C11C10:
   case GFX_LFMT_BASE_C5C6C5:
   case GFX_LFMT_BASE_C5C5C5C1:
   case GFX_LFMT_BASE_C1C5C5C5:
   case GFX_LFMT_BASE_C4C4C4C4:
   case GFX_LFMT_BASE_C32_C8X24:
   case GFX_LFMT_BASE_C24C8:
   case GFX_LFMT_BASE_C8C24:
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
   case GFX_LFMT_BASE_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
      return false;
   case GFX_LFMT_BASE_BC1:
   case GFX_LFMT_BASE_BC2:
   case GFX_LFMT_BASE_BC3:
   case GFX_LFMT_BASE_BC4:
   case GFX_LFMT_BASE_BC5:
   case GFX_LFMT_BASE_EAC:
   case GFX_LFMT_BASE_EAC_EAC:
   case GFX_LFMT_BASE_ETC2:
   case GFX_LFMT_BASE_ETC1:
   case GFX_LFMT_BASE_PUNCHTHROUGH_ETC2:
   case GFX_LFMT_BASE_ETC2_EAC:
   case GFX_LFMT_BASE_ASTC4X4:
   case GFX_LFMT_BASE_ASTC5X4:
   case GFX_LFMT_BASE_ASTC5X5:
   case GFX_LFMT_BASE_ASTC6X5:
   case GFX_LFMT_BASE_ASTC6X6:
   case GFX_LFMT_BASE_ASTC8X5:
   case GFX_LFMT_BASE_ASTC8X6:
   case GFX_LFMT_BASE_ASTC8X8:
   case GFX_LFMT_BASE_ASTC10X5:
   case GFX_LFMT_BASE_ASTC10X6:
   case GFX_LFMT_BASE_ASTC10X8:
   case GFX_LFMT_BASE_ASTC10X10:
   case GFX_LFMT_BASE_ASTC12X10:
   case GFX_LFMT_BASE_ASTC12X12:
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
   case GFX_LFMT_BASE_P4BE_R8G8B8:
   case GFX_LFMT_BASE_P4BE_R8G8B8A8:
   case GFX_LFMT_BASE_P4BE_B5G6R5:
   case GFX_LFMT_BASE_P4BE_A4B4G4R4:
   case GFX_LFMT_BASE_P4BE_A1B5G5R5:
   case GFX_LFMT_BASE_P8_R8G8B8:
   case GFX_LFMT_BASE_P8_R8G8B8A8:
   case GFX_LFMT_BASE_P8_B5G6R5:
   case GFX_LFMT_BASE_P8_A4B4G4R4:
   case GFX_LFMT_BASE_P8_A1B5G5R5:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_is_paletted(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32:
   case GFX_LFMT_BASE_C32:
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
   case GFX_LFMT_BASE_C16_C16_C16:
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
   case GFX_LFMT_BASE_C16:
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8:
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
   case GFX_LFMT_BASE_C10C10C10C2:
   case GFX_LFMT_BASE_C11C11C10:
   case GFX_LFMT_BASE_C5C6C5:
   case GFX_LFMT_BASE_C5C5C5C1:
   case GFX_LFMT_BASE_C1C5C5C5:
   case GFX_LFMT_BASE_C4C4C4C4:
   case GFX_LFMT_BASE_C32_C8X24:
   case GFX_LFMT_BASE_C24C8:
   case GFX_LFMT_BASE_C8C24:
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
   case GFX_LFMT_BASE_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
   case GFX_LFMT_BASE_BC1:
   case GFX_LFMT_BASE_BC2:
   case GFX_LFMT_BASE_BC3:
   case GFX_LFMT_BASE_BC4:
   case GFX_LFMT_BASE_BC5:
   case GFX_LFMT_BASE_EAC:
   case GFX_LFMT_BASE_EAC_EAC:
   case GFX_LFMT_BASE_ETC2:
   case GFX_LFMT_BASE_ETC1:
   case GFX_LFMT_BASE_PUNCHTHROUGH_ETC2:
   case GFX_LFMT_BASE_ETC2_EAC:
   case GFX_LFMT_BASE_ASTC4X4:
   case GFX_LFMT_BASE_ASTC5X4:
   case GFX_LFMT_BASE_ASTC5X5:
   case GFX_LFMT_BASE_ASTC6X5:
   case GFX_LFMT_BASE_ASTC6X6:
   case GFX_LFMT_BASE_ASTC8X5:
   case GFX_LFMT_BASE_ASTC8X6:
   case GFX_LFMT_BASE_ASTC8X8:
   case GFX_LFMT_BASE_ASTC10X5:
   case GFX_LFMT_BASE_ASTC10X6:
   case GFX_LFMT_BASE_ASTC10X8:
   case GFX_LFMT_BASE_ASTC10X10:
   case GFX_LFMT_BASE_ASTC12X10:
   case GFX_LFMT_BASE_ASTC12X12:
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
      return false;
   case GFX_LFMT_BASE_P4BE_R8G8B8:
   case GFX_LFMT_BASE_P4BE_R8G8B8A8:
   case GFX_LFMT_BASE_P4BE_B5G6R5:
   case GFX_LFMT_BASE_P4BE_A4B4G4R4:
   case GFX_LFMT_BASE_P4BE_A1B5G5R5:
   case GFX_LFMT_BASE_P8_R8G8B8:
   case GFX_LFMT_BASE_P8_R8G8B8A8:
   case GFX_LFMT_BASE_P8_B5G6R5:
   case GFX_LFMT_BASE_P8_A4B4G4R4:
   case GFX_LFMT_BASE_P8_A1B5G5R5:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_is_etc_family(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32:
   case GFX_LFMT_BASE_C32:
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
   case GFX_LFMT_BASE_C16_C16_C16:
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
   case GFX_LFMT_BASE_C16:
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8:
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
   case GFX_LFMT_BASE_C10C10C10C2:
   case GFX_LFMT_BASE_C11C11C10:
   case GFX_LFMT_BASE_C5C6C5:
   case GFX_LFMT_BASE_C5C5C5C1:
   case GFX_LFMT_BASE_C1C5C5C5:
   case GFX_LFMT_BASE_C4C4C4C4:
   case GFX_LFMT_BASE_C32_C8X24:
   case GFX_LFMT_BASE_C24C8:
   case GFX_LFMT_BASE_C8C24:
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
   case GFX_LFMT_BASE_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
   case GFX_LFMT_BASE_BC1:
   case GFX_LFMT_BASE_BC2:
   case GFX_LFMT_BASE_BC3:
   case GFX_LFMT_BASE_BC4:
   case GFX_LFMT_BASE_BC5:
   case GFX_LFMT_BASE_ASTC4X4:
   case GFX_LFMT_BASE_ASTC5X4:
   case GFX_LFMT_BASE_ASTC5X5:
   case GFX_LFMT_BASE_ASTC6X5:
   case GFX_LFMT_BASE_ASTC6X6:
   case GFX_LFMT_BASE_ASTC8X5:
   case GFX_LFMT_BASE_ASTC8X6:
   case GFX_LFMT_BASE_ASTC8X8:
   case GFX_LFMT_BASE_ASTC10X5:
   case GFX_LFMT_BASE_ASTC10X6:
   case GFX_LFMT_BASE_ASTC10X8:
   case GFX_LFMT_BASE_ASTC10X10:
   case GFX_LFMT_BASE_ASTC12X10:
   case GFX_LFMT_BASE_ASTC12X12:
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
   case GFX_LFMT_BASE_P4BE_R8G8B8:
   case GFX_LFMT_BASE_P4BE_R8G8B8A8:
   case GFX_LFMT_BASE_P4BE_B5G6R5:
   case GFX_LFMT_BASE_P4BE_A4B4G4R4:
   case GFX_LFMT_BASE_P4BE_A1B5G5R5:
   case GFX_LFMT_BASE_P8_R8G8B8:
   case GFX_LFMT_BASE_P8_R8G8B8A8:
   case GFX_LFMT_BASE_P8_B5G6R5:
   case GFX_LFMT_BASE_P8_A4B4G4R4:
   case GFX_LFMT_BASE_P8_A1B5G5R5:
      return false;
   case GFX_LFMT_BASE_EAC:
   case GFX_LFMT_BASE_EAC_EAC:
   case GFX_LFMT_BASE_ETC2:
   case GFX_LFMT_BASE_ETC1:
   case GFX_LFMT_BASE_PUNCHTHROUGH_ETC2:
   case GFX_LFMT_BASE_ETC2_EAC:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_is_astc_family(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32:
   case GFX_LFMT_BASE_C32:
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
   case GFX_LFMT_BASE_C16_C16_C16:
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
   case GFX_LFMT_BASE_C16:
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8:
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
   case GFX_LFMT_BASE_C10C10C10C2:
   case GFX_LFMT_BASE_C11C11C10:
   case GFX_LFMT_BASE_C5C6C5:
   case GFX_LFMT_BASE_C5C5C5C1:
   case GFX_LFMT_BASE_C1C5C5C5:
   case GFX_LFMT_BASE_C4C4C4C4:
   case GFX_LFMT_BASE_C32_C8X24:
   case GFX_LFMT_BASE_C24C8:
   case GFX_LFMT_BASE_C8C24:
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
   case GFX_LFMT_BASE_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
   case GFX_LFMT_BASE_BC1:
   case GFX_LFMT_BASE_BC2:
   case GFX_LFMT_BASE_BC3:
   case GFX_LFMT_BASE_BC4:
   case GFX_LFMT_BASE_BC5:
   case GFX_LFMT_BASE_EAC:
   case GFX_LFMT_BASE_EAC_EAC:
   case GFX_LFMT_BASE_ETC2:
   case GFX_LFMT_BASE_ETC1:
   case GFX_LFMT_BASE_PUNCHTHROUGH_ETC2:
   case GFX_LFMT_BASE_ETC2_EAC:
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
   case GFX_LFMT_BASE_P4BE_R8G8B8:
   case GFX_LFMT_BASE_P4BE_R8G8B8A8:
   case GFX_LFMT_BASE_P4BE_B5G6R5:
   case GFX_LFMT_BASE_P4BE_A4B4G4R4:
   case GFX_LFMT_BASE_P4BE_A1B5G5R5:
   case GFX_LFMT_BASE_P8_R8G8B8:
   case GFX_LFMT_BASE_P8_R8G8B8A8:
   case GFX_LFMT_BASE_P8_B5G6R5:
   case GFX_LFMT_BASE_P8_A4B4G4R4:
   case GFX_LFMT_BASE_P8_A1B5G5R5:
      return false;
   case GFX_LFMT_BASE_ASTC4X4:
   case GFX_LFMT_BASE_ASTC5X4:
   case GFX_LFMT_BASE_ASTC5X5:
   case GFX_LFMT_BASE_ASTC6X5:
   case GFX_LFMT_BASE_ASTC6X6:
   case GFX_LFMT_BASE_ASTC8X5:
   case GFX_LFMT_BASE_ASTC8X6:
   case GFX_LFMT_BASE_ASTC8X8:
   case GFX_LFMT_BASE_ASTC10X5:
   case GFX_LFMT_BASE_ASTC10X6:
   case GFX_LFMT_BASE_ASTC10X8:
   case GFX_LFMT_BASE_ASTC10X10:
   case GFX_LFMT_BASE_ASTC12X10:
   case GFX_LFMT_BASE_ASTC12X12:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_is_bc_family(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32:
   case GFX_LFMT_BASE_C32:
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
   case GFX_LFMT_BASE_C16_C16_C16:
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
   case GFX_LFMT_BASE_C16:
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8:
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
   case GFX_LFMT_BASE_C10C10C10C2:
   case GFX_LFMT_BASE_C11C11C10:
   case GFX_LFMT_BASE_C5C6C5:
   case GFX_LFMT_BASE_C5C5C5C1:
   case GFX_LFMT_BASE_C1C5C5C5:
   case GFX_LFMT_BASE_C4C4C4C4:
   case GFX_LFMT_BASE_C32_C8X24:
   case GFX_LFMT_BASE_C24C8:
   case GFX_LFMT_BASE_C8C24:
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
   case GFX_LFMT_BASE_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
   case GFX_LFMT_BASE_EAC:
   case GFX_LFMT_BASE_EAC_EAC:
   case GFX_LFMT_BASE_ETC2:
   case GFX_LFMT_BASE_ETC1:
   case GFX_LFMT_BASE_PUNCHTHROUGH_ETC2:
   case GFX_LFMT_BASE_ETC2_EAC:
   case GFX_LFMT_BASE_ASTC4X4:
   case GFX_LFMT_BASE_ASTC5X4:
   case GFX_LFMT_BASE_ASTC5X5:
   case GFX_LFMT_BASE_ASTC6X5:
   case GFX_LFMT_BASE_ASTC6X6:
   case GFX_LFMT_BASE_ASTC8X5:
   case GFX_LFMT_BASE_ASTC8X6:
   case GFX_LFMT_BASE_ASTC8X8:
   case GFX_LFMT_BASE_ASTC10X5:
   case GFX_LFMT_BASE_ASTC10X6:
   case GFX_LFMT_BASE_ASTC10X8:
   case GFX_LFMT_BASE_ASTC10X10:
   case GFX_LFMT_BASE_ASTC12X10:
   case GFX_LFMT_BASE_ASTC12X12:
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
   case GFX_LFMT_BASE_P4BE_R8G8B8:
   case GFX_LFMT_BASE_P4BE_R8G8B8A8:
   case GFX_LFMT_BASE_P4BE_B5G6R5:
   case GFX_LFMT_BASE_P4BE_A4B4G4R4:
   case GFX_LFMT_BASE_P4BE_A1B5G5R5:
   case GFX_LFMT_BASE_P8_R8G8B8:
   case GFX_LFMT_BASE_P8_R8G8B8A8:
   case GFX_LFMT_BASE_P8_B5G6R5:
   case GFX_LFMT_BASE_P8_A4B4G4R4:
   case GFX_LFMT_BASE_P8_A1B5G5R5:
      return false;
   case GFX_LFMT_BASE_BC1:
   case GFX_LFMT_BASE_BC2:
   case GFX_LFMT_BASE_BC3:
   case GFX_LFMT_BASE_BC4:
   case GFX_LFMT_BASE_BC5:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_is_bstc_family(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32:
   case GFX_LFMT_BASE_C32:
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
   case GFX_LFMT_BASE_C16_C16_C16:
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
   case GFX_LFMT_BASE_C16:
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8:
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
   case GFX_LFMT_BASE_C10C10C10C2:
   case GFX_LFMT_BASE_C11C11C10:
   case GFX_LFMT_BASE_C5C6C5:
   case GFX_LFMT_BASE_C5C5C5C1:
   case GFX_LFMT_BASE_C1C5C5C5:
   case GFX_LFMT_BASE_C4C4C4C4:
   case GFX_LFMT_BASE_C32_C8X24:
   case GFX_LFMT_BASE_C24C8:
   case GFX_LFMT_BASE_C8C24:
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
   case GFX_LFMT_BASE_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
   case GFX_LFMT_BASE_BC1:
   case GFX_LFMT_BASE_BC2:
   case GFX_LFMT_BASE_BC3:
   case GFX_LFMT_BASE_BC4:
   case GFX_LFMT_BASE_BC5:
   case GFX_LFMT_BASE_EAC:
   case GFX_LFMT_BASE_EAC_EAC:
   case GFX_LFMT_BASE_ETC2:
   case GFX_LFMT_BASE_ETC1:
   case GFX_LFMT_BASE_PUNCHTHROUGH_ETC2:
   case GFX_LFMT_BASE_ETC2_EAC:
   case GFX_LFMT_BASE_ASTC4X4:
   case GFX_LFMT_BASE_ASTC5X4:
   case GFX_LFMT_BASE_ASTC5X5:
   case GFX_LFMT_BASE_ASTC6X5:
   case GFX_LFMT_BASE_ASTC6X6:
   case GFX_LFMT_BASE_ASTC8X5:
   case GFX_LFMT_BASE_ASTC8X6:
   case GFX_LFMT_BASE_ASTC8X8:
   case GFX_LFMT_BASE_ASTC10X5:
   case GFX_LFMT_BASE_ASTC10X6:
   case GFX_LFMT_BASE_ASTC10X8:
   case GFX_LFMT_BASE_ASTC10X10:
   case GFX_LFMT_BASE_ASTC12X10:
   case GFX_LFMT_BASE_ASTC12X12:
   case GFX_LFMT_BASE_P4BE_R8G8B8:
   case GFX_LFMT_BASE_P4BE_R8G8B8A8:
   case GFX_LFMT_BASE_P4BE_B5G6R5:
   case GFX_LFMT_BASE_P4BE_A4B4G4R4:
   case GFX_LFMT_BASE_P4BE_A1B5G5R5:
   case GFX_LFMT_BASE_P8_R8G8B8:
   case GFX_LFMT_BASE_P8_R8G8B8A8:
   case GFX_LFMT_BASE_P8_B5G6R5:
   case GFX_LFMT_BASE_P8_A4B4G4R4:
   case GFX_LFMT_BASE_P8_A1B5G5R5:
      return false;
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_is_std(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32:
   case GFX_LFMT_BASE_C32:
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
   case GFX_LFMT_BASE_C16_C16_C16:
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
   case GFX_LFMT_BASE_C16:
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8:
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
   case GFX_LFMT_BASE_C10C10C10C2:
   case GFX_LFMT_BASE_C11C11C10:
   case GFX_LFMT_BASE_C5C6C5:
   case GFX_LFMT_BASE_C5C5C5C1:
   case GFX_LFMT_BASE_C1C5C5C5:
   case GFX_LFMT_BASE_C4C4C4C4:
   case GFX_LFMT_BASE_C32_C8X24:
   case GFX_LFMT_BASE_C24C8:
   case GFX_LFMT_BASE_C8C24:
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
   case GFX_LFMT_BASE_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
      return true;
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
   case GFX_LFMT_BASE_BC1:
   case GFX_LFMT_BASE_BC2:
   case GFX_LFMT_BASE_BC3:
   case GFX_LFMT_BASE_BC4:
   case GFX_LFMT_BASE_BC5:
   case GFX_LFMT_BASE_EAC:
   case GFX_LFMT_BASE_EAC_EAC:
   case GFX_LFMT_BASE_ETC2:
   case GFX_LFMT_BASE_ETC1:
   case GFX_LFMT_BASE_PUNCHTHROUGH_ETC2:
   case GFX_LFMT_BASE_ETC2_EAC:
   case GFX_LFMT_BASE_ASTC4X4:
   case GFX_LFMT_BASE_ASTC5X4:
   case GFX_LFMT_BASE_ASTC5X5:
   case GFX_LFMT_BASE_ASTC6X5:
   case GFX_LFMT_BASE_ASTC6X6:
   case GFX_LFMT_BASE_ASTC8X5:
   case GFX_LFMT_BASE_ASTC8X6:
   case GFX_LFMT_BASE_ASTC8X8:
   case GFX_LFMT_BASE_ASTC10X5:
   case GFX_LFMT_BASE_ASTC10X6:
   case GFX_LFMT_BASE_ASTC10X8:
   case GFX_LFMT_BASE_ASTC10X10:
   case GFX_LFMT_BASE_ASTC12X10:
   case GFX_LFMT_BASE_ASTC12X12:
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
   case GFX_LFMT_BASE_P4BE_R8G8B8:
   case GFX_LFMT_BASE_P4BE_R8G8B8A8:
   case GFX_LFMT_BASE_P4BE_B5G6R5:
   case GFX_LFMT_BASE_P4BE_A4B4G4R4:
   case GFX_LFMT_BASE_P4BE_A1B5G5R5:
   case GFX_LFMT_BASE_P8_R8G8B8:
   case GFX_LFMT_BASE_P8_R8G8B8A8:
   case GFX_LFMT_BASE_P8_B5G6R5:
   case GFX_LFMT_BASE_P8_A4B4G4R4:
   case GFX_LFMT_BASE_P8_A1B5G5R5:
      return false;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_is_std_with_subsample(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32_C32:
   case GFX_LFMT_BASE_C32_C32:
   case GFX_LFMT_BASE_C32:
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
   case GFX_LFMT_BASE_C16_C16_C16:
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
   case GFX_LFMT_BASE_C16:
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8:
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
   case GFX_LFMT_BASE_C10C10C10C2:
   case GFX_LFMT_BASE_C11C11C10:
   case GFX_LFMT_BASE_C5C6C5:
   case GFX_LFMT_BASE_C5C5C5C1:
   case GFX_LFMT_BASE_C1C5C5C5:
   case GFX_LFMT_BASE_C4C4C4C4:
   case GFX_LFMT_BASE_C32_C8X24:
   case GFX_LFMT_BASE_C24C8:
   case GFX_LFMT_BASE_C8C24:
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
   case GFX_LFMT_BASE_BC1:
   case GFX_LFMT_BASE_BC2:
   case GFX_LFMT_BASE_BC3:
   case GFX_LFMT_BASE_BC4:
   case GFX_LFMT_BASE_BC5:
   case GFX_LFMT_BASE_EAC:
   case GFX_LFMT_BASE_EAC_EAC:
   case GFX_LFMT_BASE_ETC2:
   case GFX_LFMT_BASE_ETC1:
   case GFX_LFMT_BASE_PUNCHTHROUGH_ETC2:
   case GFX_LFMT_BASE_ETC2_EAC:
   case GFX_LFMT_BASE_ASTC4X4:
   case GFX_LFMT_BASE_ASTC5X4:
   case GFX_LFMT_BASE_ASTC5X5:
   case GFX_LFMT_BASE_ASTC6X5:
   case GFX_LFMT_BASE_ASTC6X6:
   case GFX_LFMT_BASE_ASTC8X5:
   case GFX_LFMT_BASE_ASTC8X6:
   case GFX_LFMT_BASE_ASTC8X8:
   case GFX_LFMT_BASE_ASTC10X5:
   case GFX_LFMT_BASE_ASTC10X6:
   case GFX_LFMT_BASE_ASTC10X8:
   case GFX_LFMT_BASE_ASTC10X10:
   case GFX_LFMT_BASE_ASTC12X10:
   case GFX_LFMT_BASE_ASTC12X12:
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
   case GFX_LFMT_BASE_P4BE_R8G8B8:
   case GFX_LFMT_BASE_P4BE_R8G8B8A8:
   case GFX_LFMT_BASE_P4BE_B5G6R5:
   case GFX_LFMT_BASE_P4BE_A4B4G4R4:
   case GFX_LFMT_BASE_P4BE_A1B5G5R5:
   case GFX_LFMT_BASE_P8_R8G8B8:
   case GFX_LFMT_BASE_P8_R8G8B8A8:
   case GFX_LFMT_BASE_P8_B5G6R5:
   case GFX_LFMT_BASE_P8_A4B4G4R4:
   case GFX_LFMT_BASE_P8_A1B5G5R5:
      return false;
   case GFX_LFMT_BASE_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_contains_srgb(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_UFLOAT:
   case GFX_LFMT_TYPE_FLOAT:
   case GFX_LFMT_TYPE_UINT:
   case GFX_LFMT_TYPE_INT:
   case GFX_LFMT_TYPE_UNORM:
   case GFX_LFMT_TYPE_SNORM:
   case GFX_LFMT_TYPE_FLOAT_UINT:
   case GFX_LFMT_TYPE_UINT_FLOAT:
   case GFX_LFMT_TYPE_FLOAT_UNORM:
   case GFX_LFMT_TYPE_UNORM_FLOAT:
   case GFX_LFMT_TYPE_UNORM_UINT:
   case GFX_LFMT_TYPE_UINT_UNORM:
      return false;
   case GFX_LFMT_TYPE_SRGB:
   case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
   case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_contains_int(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_UFLOAT:
   case GFX_LFMT_TYPE_FLOAT:
   case GFX_LFMT_TYPE_UNORM:
   case GFX_LFMT_TYPE_SRGB:
   case GFX_LFMT_TYPE_SNORM:
   case GFX_LFMT_TYPE_FLOAT_UNORM:
   case GFX_LFMT_TYPE_UNORM_FLOAT:
   case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
   case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
      return false;
   case GFX_LFMT_TYPE_UINT:
   case GFX_LFMT_TYPE_INT:
   case GFX_LFMT_TYPE_FLOAT_UINT:
   case GFX_LFMT_TYPE_UINT_FLOAT:
   case GFX_LFMT_TYPE_UNORM_UINT:
   case GFX_LFMT_TYPE_UINT_UNORM:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_contains_int_signed(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_UFLOAT:
   case GFX_LFMT_TYPE_FLOAT:
   case GFX_LFMT_TYPE_UINT:
   case GFX_LFMT_TYPE_UNORM:
   case GFX_LFMT_TYPE_SRGB:
   case GFX_LFMT_TYPE_SNORM:
   case GFX_LFMT_TYPE_FLOAT_UINT:
   case GFX_LFMT_TYPE_UINT_FLOAT:
   case GFX_LFMT_TYPE_FLOAT_UNORM:
   case GFX_LFMT_TYPE_UNORM_FLOAT:
   case GFX_LFMT_TYPE_UNORM_UINT:
   case GFX_LFMT_TYPE_UINT_UNORM:
   case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
   case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
      return false;
   case GFX_LFMT_TYPE_INT:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_contains_int_unsigned(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_UFLOAT:
   case GFX_LFMT_TYPE_FLOAT:
   case GFX_LFMT_TYPE_INT:
   case GFX_LFMT_TYPE_UNORM:
   case GFX_LFMT_TYPE_SRGB:
   case GFX_LFMT_TYPE_SNORM:
   case GFX_LFMT_TYPE_FLOAT_UNORM:
   case GFX_LFMT_TYPE_UNORM_FLOAT:
   case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
   case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
      return false;
   case GFX_LFMT_TYPE_UINT:
   case GFX_LFMT_TYPE_FLOAT_UINT:
   case GFX_LFMT_TYPE_UINT_FLOAT:
   case GFX_LFMT_TYPE_UNORM_UINT:
   case GFX_LFMT_TYPE_UINT_UNORM:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_contains_float(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_UFLOAT:
   case GFX_LFMT_TYPE_FLOAT:
   case GFX_LFMT_TYPE_FLOAT_UINT:
   case GFX_LFMT_TYPE_UINT_FLOAT:
   case GFX_LFMT_TYPE_FLOAT_UNORM:
   case GFX_LFMT_TYPE_UNORM_FLOAT:
      return true;
   case GFX_LFMT_TYPE_UINT:
   case GFX_LFMT_TYPE_INT:
   case GFX_LFMT_TYPE_UNORM:
   case GFX_LFMT_TYPE_SRGB:
   case GFX_LFMT_TYPE_SNORM:
   case GFX_LFMT_TYPE_UNORM_UINT:
   case GFX_LFMT_TYPE_UINT_UNORM:
   case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
   case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
      return false;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_contains_unorm(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_UFLOAT:
   case GFX_LFMT_TYPE_FLOAT:
   case GFX_LFMT_TYPE_UINT:
   case GFX_LFMT_TYPE_INT:
   case GFX_LFMT_TYPE_SRGB:
   case GFX_LFMT_TYPE_SNORM:
   case GFX_LFMT_TYPE_FLOAT_UINT:
   case GFX_LFMT_TYPE_UINT_FLOAT:
      return false;
   case GFX_LFMT_TYPE_UNORM:
   case GFX_LFMT_TYPE_FLOAT_UNORM:
   case GFX_LFMT_TYPE_UNORM_FLOAT:
   case GFX_LFMT_TYPE_UNORM_UINT:
   case GFX_LFMT_TYPE_UINT_UNORM:
   case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
   case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_contains_snorm(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_UFLOAT:
   case GFX_LFMT_TYPE_FLOAT:
   case GFX_LFMT_TYPE_UINT:
   case GFX_LFMT_TYPE_INT:
   case GFX_LFMT_TYPE_UNORM:
   case GFX_LFMT_TYPE_SRGB:
   case GFX_LFMT_TYPE_FLOAT_UINT:
   case GFX_LFMT_TYPE_UINT_FLOAT:
   case GFX_LFMT_TYPE_FLOAT_UNORM:
   case GFX_LFMT_TYPE_UNORM_FLOAT:
   case GFX_LFMT_TYPE_UNORM_UINT:
   case GFX_LFMT_TYPE_UINT_UNORM:
   case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
   case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
      return false;
   case GFX_LFMT_TYPE_SNORM:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_has_red(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
      return true;
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return false;
   default:
      unreachable();
      return false;
   }
}
uint32_t gfx_lfmt_red_bits(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_red(lfmt)) {
      return 0;
   }
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RG:
      case GFX_LFMT_CHANNELS_GR:
      case GFX_LFMT_CHANNELS_RA:
      case GFX_LFMT_CHANNELS_AR:
      case GFX_LFMT_CHANNELS_RX:
      case GFX_LFMT_CHANNELS_XR:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_R:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RG:
      case GFX_LFMT_CHANNELS_GR:
      case GFX_LFMT_CHANNELS_RA:
      case GFX_LFMT_CHANNELS_AR:
      case GFX_LFMT_CHANNELS_RX:
      case GFX_LFMT_CHANNELS_XR:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_R:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RG:
      case GFX_LFMT_CHANNELS_GR:
      case GFX_LFMT_CHANNELS_RA:
      case GFX_LFMT_CHANNELS_AR:
      case GFX_LFMT_CHANNELS_RX:
      case GFX_LFMT_CHANNELS_XR:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_R:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_R:
         return 4;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_R:
         return 1;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C10C10C10C2:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_RAXX:
         return 10;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C11C11C10:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_RGX:
         return 11;
      case GFX_LFMT_CHANNELS_BGR:
      case GFX_LFMT_CHANNELS_XGR:
         return 10;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C5C6C5:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 5;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C5C5C5C1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_RAXX:
         return 5;
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_XXAR:
         return 1;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C1C5C5C5:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_RAXX:
         return 1;
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_XXAR:
         return 5;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C4C4C4C4:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 4;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C8X24:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RG:
      case GFX_LFMT_CHANNELS_RA:
      case GFX_LFMT_CHANNELS_RX:
         return 32;
      case GFX_LFMT_CHANNELS_GR:
      case GFX_LFMT_CHANNELS_AR:
      case GFX_LFMT_CHANNELS_XR:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C24C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RG:
      case GFX_LFMT_CHANNELS_RA:
      case GFX_LFMT_CHANNELS_RX:
         return 24;
      case GFX_LFMT_CHANNELS_GR:
      case GFX_LFMT_CHANNELS_AR:
      case GFX_LFMT_CHANNELS_XR:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C24:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RG:
      case GFX_LFMT_CHANNELS_RA:
      case GFX_LFMT_CHANNELS_RX:
         return 8;
      case GFX_LFMT_CHANNELS_GR:
      case GFX_LFMT_CHANNELS_AR:
      case GFX_LFMT_CHANNELS_XR:
         return 24;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 15;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
         return 9;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   default:
      unreachable();
      return 0;
   }
}
bool gfx_lfmt_has_green(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return false;
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
      return true;
   default:
      unreachable();
      return false;
   }
}
uint32_t gfx_lfmt_green_bits(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_green(lfmt)) {
      return 0;
   }
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RG:
      case GFX_LFMT_CHANNELS_GR:
      case GFX_LFMT_CHANNELS_XG:
      case GFX_LFMT_CHANNELS_GX:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_G:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RG:
      case GFX_LFMT_CHANNELS_GR:
      case GFX_LFMT_CHANNELS_XG:
      case GFX_LFMT_CHANNELS_GX:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_G:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RG:
      case GFX_LFMT_CHANNELS_GR:
      case GFX_LFMT_CHANNELS_XG:
      case GFX_LFMT_CHANNELS_GX:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_G:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_G:
         return 4;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C10C10C10C2:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
         return 10;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C11C11C10:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 11;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C5C6C5:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 6;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C5C5C5C1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XBGR:
         return 5;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C1C5C5C5:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
         return 5;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C4C4C4C4:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
         return 4;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C8X24:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RG:
      case GFX_LFMT_CHANNELS_XG:
         return 8;
      case GFX_LFMT_CHANNELS_GR:
      case GFX_LFMT_CHANNELS_GX:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C24C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RG:
      case GFX_LFMT_CHANNELS_XG:
         return 8;
      case GFX_LFMT_CHANNELS_GR:
      case GFX_LFMT_CHANNELS_GX:
         return 24;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C24:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RG:
      case GFX_LFMT_CHANNELS_XG:
         return 24;
      case GFX_LFMT_CHANNELS_GR:
      case GFX_LFMT_CHANNELS_GX:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
         return 15;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
         return 9;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   default:
      unreachable();
      return 0;
   }
}
bool gfx_lfmt_has_blue(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return false;
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
      return true;
   default:
      unreachable();
      return false;
   }
}
uint32_t gfx_lfmt_blue_bits(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_blue(lfmt)) {
      return 0;
   }
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_B:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_B:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_B:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_B:
         return 4;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C10C10C10C2:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
         return 10;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C11C11C10:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
         return 10;
      case GFX_LFMT_CHANNELS_BGR:
         return 11;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C5C6C5:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
      case GFX_LFMT_CHANNELS_BGR:
         return 5;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C5C5C5C1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XBGR:
         return 5;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C1C5C5C5:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
         return 5;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C4C4C4C4:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
         return 4;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
         return 15;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGB:
         return 9;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   default:
      unreachable();
      return 0;
   }
}
bool gfx_lfmt_has_alpha(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return false;
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
      return true;
   default:
      unreachable();
      return false;
   }
}
uint32_t gfx_lfmt_alpha_bits(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_alpha(lfmt)) {
      return 0;
   }
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RA:
      case GFX_LFMT_CHANNELS_AR:
      case GFX_LFMT_CHANNELS_LA:
      case GFX_LFMT_CHANNELS_AL:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_A:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RA:
      case GFX_LFMT_CHANNELS_AR:
      case GFX_LFMT_CHANNELS_LA:
      case GFX_LFMT_CHANNELS_AL:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_A:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RA:
      case GFX_LFMT_CHANNELS_AR:
      case GFX_LFMT_CHANNELS_LA:
      case GFX_LFMT_CHANNELS_AL:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_A:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_A:
         return 4;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_A:
         return 1;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C10C10C10C2:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
         return 2;
      case GFX_LFMT_CHANNELS_RAXX:
         return 10;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C5C5C5C1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
         return 1;
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 5;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C1C5C5C5:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 5;
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
         return 1;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C4C4C4C4:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 4;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C8X24:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RA:
      case GFX_LFMT_CHANNELS_LA:
         return 8;
      case GFX_LFMT_CHANNELS_AR:
      case GFX_LFMT_CHANNELS_AL:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C24C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RA:
      case GFX_LFMT_CHANNELS_LA:
         return 8;
      case GFX_LFMT_CHANNELS_AR:
      case GFX_LFMT_CHANNELS_AL:
         return 24;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C24:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RA:
      case GFX_LFMT_CHANNELS_LA:
         return 24;
      case GFX_LFMT_CHANNELS_AR:
      case GFX_LFMT_CHANNELS_AL:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_ARGB:
      case GFX_LFMT_CHANNELS_ABGR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 15;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBA:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   default:
      unreachable();
      return 0;
   }
}
bool gfx_lfmt_has_depth(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return false;
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
      return true;
   default:
      unreachable();
      return false;
   }
}
uint32_t gfx_lfmt_depth_bits(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_depth(lfmt)) {
      return 0;
   }
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_DX:
      case GFX_LFMT_CHANNELS_XD:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_D:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_DX:
      case GFX_LFMT_CHANNELS_XD:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_D:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C8X24:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_DS:
      case GFX_LFMT_CHANNELS_DX:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C24C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_DS:
      case GFX_LFMT_CHANNELS_DX:
         return 24;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C24:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_SD:
      case GFX_LFMT_CHANNELS_XD:
         return 24;
      default:
         unreachable();
         return 0;
      }
      break;
   default:
      unreachable();
      return 0;
   }
}
bool gfx_lfmt_has_stencil(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return false;
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
      return true;
   default:
      unreachable();
      return false;
   }
}
uint32_t gfx_lfmt_stencil_bits(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_stencil(lfmt)) {
      return 0;
   }
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_SX:
      case GFX_LFMT_CHANNELS_XS:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_S:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C8X24:
   case GFX_LFMT_BASE_C24C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_DS:
      case GFX_LFMT_CHANNELS_XS:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C24:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_SD:
      case GFX_LFMT_CHANNELS_SX:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   default:
      unreachable();
      return 0;
   }
}
bool gfx_lfmt_has_y(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
      return false;
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return true;
   default:
      unreachable();
      return false;
   }
}
uint32_t gfx_lfmt_y_bits(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_y(lfmt)) {
      return 0;
   }
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_YUYV:
      case GFX_LFMT_CHANNELS_VYUY:
      case GFX_LFMT_CHANNELS_YYUV:
      case GFX_LFMT_CHANNELS_VUYY:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_YUV:
      case GFX_LFMT_CHANNELS_VUY:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_Y:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   default:
      unreachable();
      return 0;
   }
}
bool gfx_lfmt_has_u(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_V:
      return false;
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return true;
   default:
      unreachable();
      return false;
   }
}
uint32_t gfx_lfmt_u_bits(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_u(lfmt)) {
      return 0;
   }
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_YUYV:
      case GFX_LFMT_CHANNELS_VYUY:
      case GFX_LFMT_CHANNELS_YYUV:
      case GFX_LFMT_CHANNELS_VUYY:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_YUV:
      case GFX_LFMT_CHANNELS_VUY:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_UV:
      case GFX_LFMT_CHANNELS_VU:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C8_2X2:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_U:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   default:
      unreachable();
      return 0;
   }
}
bool gfx_lfmt_has_v(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
      return false;
   case GFX_LFMT_CHANNELS_V:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return true;
   default:
      unreachable();
      return false;
   }
}
uint32_t gfx_lfmt_v_bits(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_v(lfmt)) {
      return 0;
   }
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_YUYV:
      case GFX_LFMT_CHANNELS_VYUY:
      case GFX_LFMT_CHANNELS_YYUV:
      case GFX_LFMT_CHANNELS_VUYY:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_YUV:
      case GFX_LFMT_CHANNELS_VUY:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_UV:
      case GFX_LFMT_CHANNELS_VU:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C8_2X2:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_V:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   default:
      unreachable();
      return 0;
   }
}
bool gfx_lfmt_has_x(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return false;
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
      return true;
   default:
      unreachable();
      return false;
   }
}
uint32_t gfx_lfmt_x_bits(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_x(lfmt)) {
      return 0;
   }
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RX:
      case GFX_LFMT_CHANNELS_XR:
      case GFX_LFMT_CHANNELS_XG:
      case GFX_LFMT_CHANNELS_GX:
      case GFX_LFMT_CHANNELS_LX:
      case GFX_LFMT_CHANNELS_XL:
      case GFX_LFMT_CHANNELS_DX:
      case GFX_LFMT_CHANNELS_XD:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_X:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RX:
      case GFX_LFMT_CHANNELS_XR:
      case GFX_LFMT_CHANNELS_XG:
      case GFX_LFMT_CHANNELS_GX:
      case GFX_LFMT_CHANNELS_LX:
      case GFX_LFMT_CHANNELS_XL:
      case GFX_LFMT_CHANNELS_DX:
      case GFX_LFMT_CHANNELS_XD:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C16:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_X:
         return 16;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RX:
      case GFX_LFMT_CHANNELS_XR:
      case GFX_LFMT_CHANNELS_XG:
      case GFX_LFMT_CHANNELS_GX:
      case GFX_LFMT_CHANNELS_LX:
      case GFX_LFMT_CHANNELS_XL:
      case GFX_LFMT_CHANNELS_SX:
      case GFX_LFMT_CHANNELS_XS:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_X:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_X:
         return 4;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_X:
         return 1;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C10C10C10C2:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
         return 2;
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_RAXX:
         return 10;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C11C11C10:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGX:
         return 10;
      case GFX_LFMT_CHANNELS_XGR:
         return 11;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C5C6C5:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGX:
      case GFX_LFMT_CHANNELS_XGR:
         return 5;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C5C5C5C1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
         return 1;
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 5;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C1C5C5C5:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_RAXX:
         return 5;
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_XXAR:
         return 1;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C4C4C4C4:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 4;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C32_C8X24:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RX:
      case GFX_LFMT_CHANNELS_GX:
      case GFX_LFMT_CHANNELS_LX:
      case GFX_LFMT_CHANNELS_DX:
         return 8;
      case GFX_LFMT_CHANNELS_XR:
      case GFX_LFMT_CHANNELS_XG:
      case GFX_LFMT_CHANNELS_XL:
      case GFX_LFMT_CHANNELS_XS:
         return 32;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C24C8:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RX:
      case GFX_LFMT_CHANNELS_GX:
      case GFX_LFMT_CHANNELS_LX:
      case GFX_LFMT_CHANNELS_DX:
         return 8;
      case GFX_LFMT_CHANNELS_XR:
      case GFX_LFMT_CHANNELS_XG:
      case GFX_LFMT_CHANNELS_XL:
      case GFX_LFMT_CHANNELS_XS:
         return 24;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C8C24:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RX:
      case GFX_LFMT_CHANNELS_GX:
      case GFX_LFMT_CHANNELS_LX:
      case GFX_LFMT_CHANNELS_SX:
         return 24;
      case GFX_LFMT_CHANNELS_XR:
      case GFX_LFMT_CHANNELS_XG:
      case GFX_LFMT_CHANNELS_XL:
      case GFX_LFMT_CHANNELS_XD:
         return 8;
      default:
         unreachable();
         return 0;
      }
      break;
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
      switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
      case GFX_LFMT_CHANNELS_RGBX:
      case GFX_LFMT_CHANNELS_BGRX:
      case GFX_LFMT_CHANNELS_XRGB:
      case GFX_LFMT_CHANNELS_XBGR:
      case GFX_LFMT_CHANNELS_RXXX:
      case GFX_LFMT_CHANNELS_XXXR:
      case GFX_LFMT_CHANNELS_RAXX:
      case GFX_LFMT_CHANNELS_XXAR:
         return 15;
      default:
         unreachable();
         return 0;
      }
      break;
   default:
      unreachable();
      return 0;
   }
}
GFX_LFMT_TYPE_T gfx_lfmt_red_type(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_red(lfmt)) {
      return GFX_LFMT_TYPE_NONE;
   }
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SRGB:
         return GFX_LFMT_TYPE_SRGB;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_RG:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
      case GFX_LFMT_TYPE_FLOAT_UINT:
      case GFX_LFMT_TYPE_FLOAT_UNORM:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
      case GFX_LFMT_TYPE_UINT_FLOAT:
      case GFX_LFMT_TYPE_UINT_UNORM:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_UNORM_FLOAT:
      case GFX_LFMT_TYPE_UNORM_UINT:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SRGB:
         return GFX_LFMT_TYPE_SRGB;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_GR:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
      case GFX_LFMT_TYPE_UINT_FLOAT:
      case GFX_LFMT_TYPE_UNORM_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
      case GFX_LFMT_TYPE_FLOAT_UINT:
      case GFX_LFMT_TYPE_UNORM_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_FLOAT_UNORM:
      case GFX_LFMT_TYPE_UINT_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SRGB:
         return GFX_LFMT_TYPE_SRGB;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_RA:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
      case GFX_LFMT_TYPE_FLOAT_UINT:
      case GFX_LFMT_TYPE_FLOAT_UNORM:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
      case GFX_LFMT_TYPE_UINT_FLOAT:
      case GFX_LFMT_TYPE_UINT_UNORM:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_UNORM_FLOAT:
      case GFX_LFMT_TYPE_UNORM_UINT:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_AR:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
      case GFX_LFMT_TYPE_UINT_FLOAT:
      case GFX_LFMT_TYPE_UNORM_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
      case GFX_LFMT_TYPE_FLOAT_UINT:
      case GFX_LFMT_TYPE_UNORM_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_FLOAT_UNORM:
      case GFX_LFMT_TYPE_UINT_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
         return GFX_LFMT_TYPE_SRGB;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
         return GFX_LFMT_TYPE_SRGB;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   default:
      unreachable();
      return GFX_LFMT_TYPE_NONE;
   }
}
unsigned gfx_lfmt_red_index(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_RAXX:
      return 0;
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XRGB:
      return 1;
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_BGRX:
      return 2;
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_XXAR:
      return 3;
   default:
      unreachable();
      return 0;
   }
}
GFX_LFMT_TYPE_T gfx_lfmt_green_type(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_green(lfmt)) {
      return GFX_LFMT_TYPE_NONE;
   }
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SRGB:
         return GFX_LFMT_TYPE_SRGB;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_RG:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
      case GFX_LFMT_TYPE_UINT_FLOAT:
      case GFX_LFMT_TYPE_UNORM_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
      case GFX_LFMT_TYPE_FLOAT_UINT:
      case GFX_LFMT_TYPE_UNORM_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_FLOAT_UNORM:
      case GFX_LFMT_TYPE_UINT_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SRGB:
         return GFX_LFMT_TYPE_SRGB;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_GR:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
      case GFX_LFMT_TYPE_FLOAT_UINT:
      case GFX_LFMT_TYPE_FLOAT_UNORM:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
      case GFX_LFMT_TYPE_UINT_FLOAT:
      case GFX_LFMT_TYPE_UINT_UNORM:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_UNORM_FLOAT:
      case GFX_LFMT_TYPE_UNORM_UINT:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SRGB:
         return GFX_LFMT_TYPE_SRGB;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
         return GFX_LFMT_TYPE_SRGB;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
         return GFX_LFMT_TYPE_SRGB;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   default:
      unreachable();
      return GFX_LFMT_TYPE_NONE;
   }
}
unsigned gfx_lfmt_green_index(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_GX:
      return 0;
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
      return 1;
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
      return 2;
   default:
      unreachable();
      return 0;
   }
}
GFX_LFMT_TYPE_T gfx_lfmt_blue_type(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_blue(lfmt)) {
      return GFX_LFMT_TYPE_NONE;
   }
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SRGB:
         return GFX_LFMT_TYPE_SRGB;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
         return GFX_LFMT_TYPE_SRGB;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
         return GFX_LFMT_TYPE_SRGB;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   default:
      unreachable();
      return GFX_LFMT_TYPE_NONE;
   }
}
unsigned gfx_lfmt_blue_index(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_BGRX:
      return 0;
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_RGBX:
      return 2;
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_XRGB:
      return 3;
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_XBGR:
      return 1;
   default:
      unreachable();
      return 0;
   }
}
GFX_LFMT_TYPE_T gfx_lfmt_alpha_type(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_alpha(lfmt)) {
      return GFX_LFMT_TYPE_NONE;
   }
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_LA:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
      case GFX_LFMT_TYPE_UINT_FLOAT:
      case GFX_LFMT_TYPE_UNORM_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
      case GFX_LFMT_TYPE_FLOAT_UINT:
      case GFX_LFMT_TYPE_UNORM_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_FLOAT_UNORM:
      case GFX_LFMT_TYPE_UINT_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_AL:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
      case GFX_LFMT_TYPE_FLOAT_UINT:
      case GFX_LFMT_TYPE_FLOAT_UNORM:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
      case GFX_LFMT_TYPE_UINT_FLOAT:
      case GFX_LFMT_TYPE_UINT_UNORM:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_UNORM_FLOAT:
      case GFX_LFMT_TYPE_UNORM_UINT:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UFLOAT:
         return GFX_LFMT_TYPE_UFLOAT;
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      case GFX_LFMT_TYPE_INT:
         return GFX_LFMT_TYPE_INT;
      case GFX_LFMT_TYPE_UNORM:
      case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
         return GFX_LFMT_TYPE_UNORM;
      case GFX_LFMT_TYPE_SNORM:
         return GFX_LFMT_TYPE_SNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   default:
      unreachable();
      return GFX_LFMT_TYPE_NONE;
   }
}
unsigned gfx_lfmt_alpha_index(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_AL:
      return 0;
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_LA:
      return 1;
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
      return 3;
   case GFX_LFMT_CHANNELS_XXAR:
      return 2;
   default:
      unreachable();
      return 0;
   }
}
GFX_LFMT_TYPE_T gfx_lfmt_depth_type(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_depth(lfmt)) {
      return GFX_LFMT_TYPE_NONE;
   }
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_DS:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_FLOAT_UINT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UNORM_UINT:
         return GFX_LFMT_TYPE_UNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_SD:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UINT_FLOAT:
         return GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   default:
      unreachable();
      return GFX_LFMT_TYPE_NONE;
   }
}
unsigned gfx_lfmt_depth_index(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_DX:
      return 0;
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_XD:
      return 1;
   default:
      unreachable();
      return 0;
   }
}
GFX_LFMT_TYPE_T gfx_lfmt_stencil_type(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_stencil(lfmt)) {
      return GFX_LFMT_TYPE_NONE;
   }
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UINT:
         return GFX_LFMT_TYPE_UINT;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_DS:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_FLOAT_UINT:
      case GFX_LFMT_TYPE_UNORM_UINT:
         return GFX_LFMT_TYPE_UINT;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_SD:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UINT_FLOAT:
      case GFX_LFMT_TYPE_UINT_UNORM:
         return GFX_LFMT_TYPE_UINT;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   default:
      unreachable();
      return GFX_LFMT_TYPE_NONE;
   }
}
unsigned gfx_lfmt_stencil_index(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_SX:
      return 0;
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_XS:
      return 1;
   default:
      unreachable();
      return 0;
   }
}
GFX_LFMT_TYPE_T gfx_lfmt_y_type(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_y(lfmt)) {
      return GFX_LFMT_TYPE_NONE;
   }
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   default:
      unreachable();
      return GFX_LFMT_TYPE_NONE;
   }
}
unsigned gfx_lfmt_y_index(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_YYUV:
      return 0;
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_VUYY:
      return 2;
   case GFX_LFMT_CHANNELS_VYUY:
      return 1;
   default:
      unreachable();
      return 0;
   }
}
GFX_LFMT_TYPE_T gfx_lfmt_u_type(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_u(lfmt)) {
      return GFX_LFMT_TYPE_NONE;
   }
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UNORM:
         return GFX_LFMT_TYPE_UNORM;
      default:
         unreachable();
         return GFX_LFMT_TYPE_NONE;
      }
      break;
   default:
      unreachable();
      return GFX_LFMT_TYPE_NONE;
   }
}
unsigned gfx_lfmt_u_index(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_UV:
      return 0;
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VUYY:
      return 1;
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
      return 2;
   default:
      unreachable();
      return 0;
   }
}
GFX_LFMT_T gfx_lfmt_depth_to_x(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_depth(lfmt))
      return lfmt;
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_D:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_FLOAT:
      case GFX_LFMT_TYPE_UNORM:
         return (lfmt & ~(GFX_LFMT_CHANNELS_MASK | GFX_LFMT_TYPE_MASK)) | GFX_LFMT_CHANNELS_X | GFX_LFMT_TYPE_NONE;
      default:
         unreachable();
         return GFX_LFMT_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_DS:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_FLOAT_UINT:
      case GFX_LFMT_TYPE_UNORM_UINT:
         return (lfmt & ~(GFX_LFMT_CHANNELS_MASK | GFX_LFMT_TYPE_MASK)) | GFX_LFMT_CHANNELS_XS | GFX_LFMT_TYPE_UINT;
      default:
         unreachable();
         return GFX_LFMT_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_SD:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UINT_FLOAT:
      case GFX_LFMT_TYPE_UINT_UNORM:
         return (lfmt & ~(GFX_LFMT_CHANNELS_MASK | GFX_LFMT_TYPE_MASK)) | GFX_LFMT_CHANNELS_SX | GFX_LFMT_TYPE_UINT;
      default:
         unreachable();
         return GFX_LFMT_NONE;
      }
      break;
   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}
GFX_LFMT_T gfx_lfmt_stencil_to_x(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_stencil(lfmt))
      return lfmt;
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_S:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UINT:
         return (lfmt & ~(GFX_LFMT_CHANNELS_MASK | GFX_LFMT_TYPE_MASK)) | GFX_LFMT_CHANNELS_X | GFX_LFMT_TYPE_NONE;
      default:
         unreachable();
         return GFX_LFMT_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_DS:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_FLOAT_UINT:
         return (lfmt & ~(GFX_LFMT_CHANNELS_MASK | GFX_LFMT_TYPE_MASK)) | GFX_LFMT_CHANNELS_DX | GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UNORM_UINT:
         return (lfmt & ~(GFX_LFMT_CHANNELS_MASK | GFX_LFMT_TYPE_MASK)) | GFX_LFMT_CHANNELS_DX | GFX_LFMT_TYPE_UNORM;
      default:
         unreachable();
         return GFX_LFMT_NONE;
      }
      break;
   case GFX_LFMT_CHANNELS_SD:
      switch (lfmt & GFX_LFMT_TYPE_MASK) {
      case GFX_LFMT_TYPE_UINT_FLOAT:
         return (lfmt & ~(GFX_LFMT_CHANNELS_MASK | GFX_LFMT_TYPE_MASK)) | GFX_LFMT_CHANNELS_XD | GFX_LFMT_TYPE_FLOAT;
      case GFX_LFMT_TYPE_UINT_UNORM:
         return (lfmt & ~(GFX_LFMT_CHANNELS_MASK | GFX_LFMT_TYPE_MASK)) | GFX_LFMT_CHANNELS_XD | GFX_LFMT_TYPE_UNORM;
      default:
         unreachable();
         return GFX_LFMT_NONE;
      }
      break;
   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}
bool gfx_lfmt_has_color(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return true;
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
      return false;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_has_depth_stencil(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return false;
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
      return true;
   default:
      unreachable();
      return false;
   }
}
bool gfx_lfmt_has_repeated_channel(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
      return false;
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return true;
   default:
      unreachable();
      return false;
   }
}
GFX_LFMT_T gfx_lfmt_reverse_channels(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_R;
   case GFX_LFMT_CHANNELS_G:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_G;
   case GFX_LFMT_CHANNELS_B:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_B;
   case GFX_LFMT_CHANNELS_A:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_A;
   case GFX_LFMT_CHANNELS_RG:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_GR;
   case GFX_LFMT_CHANNELS_GR:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_RG;
   case GFX_LFMT_CHANNELS_RA:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_AR;
   case GFX_LFMT_CHANNELS_AR:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_RA;
   case GFX_LFMT_CHANNELS_RGB:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_BGR;
   case GFX_LFMT_CHANNELS_BGR:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_RGB;
   case GFX_LFMT_CHANNELS_RGBA:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_ABGR;
   case GFX_LFMT_CHANNELS_BGRA:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_ARGB;
   case GFX_LFMT_CHANNELS_ARGB:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_BGRA;
   case GFX_LFMT_CHANNELS_ABGR:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_RGBA;
   case GFX_LFMT_CHANNELS_X:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_X;
   case GFX_LFMT_CHANNELS_RX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XR;
   case GFX_LFMT_CHANNELS_XR:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_RX;
   case GFX_LFMT_CHANNELS_XG:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_GX;
   case GFX_LFMT_CHANNELS_GX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XG;
   case GFX_LFMT_CHANNELS_RGX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XGR;
   case GFX_LFMT_CHANNELS_XGR:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_RGX;
   case GFX_LFMT_CHANNELS_RGBX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XBGR;
   case GFX_LFMT_CHANNELS_BGRX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XRGB;
   case GFX_LFMT_CHANNELS_XRGB:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_BGRX;
   case GFX_LFMT_CHANNELS_XBGR:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_RGBX;
   case GFX_LFMT_CHANNELS_RXXX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XXXR;
   case GFX_LFMT_CHANNELS_XXXR:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_RXXX;
   case GFX_LFMT_CHANNELS_RAXX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XXAR;
   case GFX_LFMT_CHANNELS_XXAR:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_RAXX;
   case GFX_LFMT_CHANNELS_L:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_L;
   case GFX_LFMT_CHANNELS_LA:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_AL;
   case GFX_LFMT_CHANNELS_AL:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_LA;
   case GFX_LFMT_CHANNELS_LX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XL;
   case GFX_LFMT_CHANNELS_XL:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_LX;
   case GFX_LFMT_CHANNELS_D:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_D;
   case GFX_LFMT_CHANNELS_S:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_S;
   case GFX_LFMT_CHANNELS_DS:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_SD;
   case GFX_LFMT_CHANNELS_SD:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_DS;
   case GFX_LFMT_CHANNELS_DX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XD;
   case GFX_LFMT_CHANNELS_XD:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_DX;
   case GFX_LFMT_CHANNELS_SX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XS;
   case GFX_LFMT_CHANNELS_XS:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_SX;
   case GFX_LFMT_CHANNELS_Y:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_Y;
   case GFX_LFMT_CHANNELS_U:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_U;
   case GFX_LFMT_CHANNELS_V:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_V;
   case GFX_LFMT_CHANNELS_UV:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_VU;
   case GFX_LFMT_CHANNELS_VU:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_UV;
   case GFX_LFMT_CHANNELS_YUV:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_VUY;
   case GFX_LFMT_CHANNELS_VUY:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_YUV;
   case GFX_LFMT_CHANNELS_YUYV:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_VYUY;
   case GFX_LFMT_CHANNELS_VYUY:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_YUYV;
   case GFX_LFMT_CHANNELS_YYUV:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_VUYY;
   case GFX_LFMT_CHANNELS_VUYY:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_YYUV;
   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}
GFX_LFMT_T gfx_lfmt_reverse_type(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_UFLOAT:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_UFLOAT;
   case GFX_LFMT_TYPE_FLOAT:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_FLOAT;
   case GFX_LFMT_TYPE_UINT:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_UINT;
   case GFX_LFMT_TYPE_INT:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_INT;
   case GFX_LFMT_TYPE_UNORM:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_UNORM;
   case GFX_LFMT_TYPE_SRGB:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_SRGB;
   case GFX_LFMT_TYPE_SNORM:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_SNORM;
   case GFX_LFMT_TYPE_FLOAT_UINT:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_UINT_FLOAT;
   case GFX_LFMT_TYPE_UINT_FLOAT:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_FLOAT_UINT;
   case GFX_LFMT_TYPE_FLOAT_UNORM:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_UNORM_FLOAT;
   case GFX_LFMT_TYPE_UNORM_FLOAT:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_FLOAT_UNORM;
   case GFX_LFMT_TYPE_UNORM_UINT:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_UINT_UNORM;
   case GFX_LFMT_TYPE_UINT_UNORM:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_UNORM_UINT;
   case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB;
   case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM;
   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}
uint32_t gfx_lfmt_valid_chan_mask(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
      return 0x1;
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
      return 0x3;
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
      return 0x7;
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return 0xf;
   case GFX_LFMT_CHANNELS_X:
      return 0x0;
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_XS:
      return 0x2;
   case GFX_LFMT_CHANNELS_XGR:
      return 0x6;
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
      return 0xe;
   case GFX_LFMT_CHANNELS_XXXR:
      return 0x8;
   case GFX_LFMT_CHANNELS_XXAR:
      return 0xc;
   default:
      unreachable();
      return 0;
   }
}
uint32_t gfx_lfmt_num_slots_from_base(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C32_C32_C32_C32:
   case GFX_LFMT_BASE_C16C16C16C16:
   case GFX_LFMT_BASE_C16C16_C16C16:
   case GFX_LFMT_BASE_C16_C16_C16_C16:
   case GFX_LFMT_BASE_C8C8C8C8:
   case GFX_LFMT_BASE_C8_C8_C8_C8:
   case GFX_LFMT_BASE_C10C10C10C2:
   case GFX_LFMT_BASE_C5C5C5C1:
   case GFX_LFMT_BASE_C1C5C5C5:
   case GFX_LFMT_BASE_C4C4C4C4:
   case GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1:
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
   case GFX_LFMT_BASE_BC1:
   case GFX_LFMT_BASE_BC2:
   case GFX_LFMT_BASE_BC3:
   case GFX_LFMT_BASE_PUNCHTHROUGH_ETC2:
   case GFX_LFMT_BASE_ETC2_EAC:
   case GFX_LFMT_BASE_ASTC4X4:
   case GFX_LFMT_BASE_ASTC5X4:
   case GFX_LFMT_BASE_ASTC5X5:
   case GFX_LFMT_BASE_ASTC6X5:
   case GFX_LFMT_BASE_ASTC6X6:
   case GFX_LFMT_BASE_ASTC8X5:
   case GFX_LFMT_BASE_ASTC8X6:
   case GFX_LFMT_BASE_ASTC8X8:
   case GFX_LFMT_BASE_ASTC10X5:
   case GFX_LFMT_BASE_ASTC10X6:
   case GFX_LFMT_BASE_ASTC10X8:
   case GFX_LFMT_BASE_ASTC10X10:
   case GFX_LFMT_BASE_ASTC12X10:
   case GFX_LFMT_BASE_ASTC12X12:
   case GFX_LFMT_BASE_BSTC:
   case GFX_LFMT_BASE_BSTCYFLIP:
   case GFX_LFMT_BASE_P4BE_R8G8B8A8:
   case GFX_LFMT_BASE_P4BE_A4B4G4R4:
   case GFX_LFMT_BASE_P4BE_A1B5G5R5:
   case GFX_LFMT_BASE_P8_R8G8B8A8:
   case GFX_LFMT_BASE_P8_A4B4G4R4:
   case GFX_LFMT_BASE_P8_A1B5G5R5:
      return 4;
   case GFX_LFMT_BASE_C32_C32_C32:
   case GFX_LFMT_BASE_C16_C16_C16:
   case GFX_LFMT_BASE_C8_C8_C8:
   case GFX_LFMT_BASE_C11C11C10:
   case GFX_LFMT_BASE_C5C6C5:
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5:
   case GFX_LFMT_BASE_ETC2:
   case GFX_LFMT_BASE_ETC1:
   case GFX_LFMT_BASE_P4BE_R8G8B8:
   case GFX_LFMT_BASE_P4BE_B5G6R5:
   case GFX_LFMT_BASE_P8_R8G8B8:
   case GFX_LFMT_BASE_P8_B5G6R5:
      return 3;
   case GFX_LFMT_BASE_C32_C32:
   case GFX_LFMT_BASE_C16C16:
   case GFX_LFMT_BASE_C16_C16:
   case GFX_LFMT_BASE_C8C8:
   case GFX_LFMT_BASE_C8_C8:
   case GFX_LFMT_BASE_C32_C8X24:
   case GFX_LFMT_BASE_C24C8:
   case GFX_LFMT_BASE_C8C24:
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
   case GFX_LFMT_BASE_BC5:
   case GFX_LFMT_BASE_EAC_EAC:
      return 2;
   case GFX_LFMT_BASE_C32:
   case GFX_LFMT_BASE_C16:
   case GFX_LFMT_BASE_C8:
   case GFX_LFMT_BASE_C4:
   case GFX_LFMT_BASE_C4BE:
   case GFX_LFMT_BASE_C4X4:
   case GFX_LFMT_BASE_C1:
   case GFX_LFMT_BASE_C1X7:
   case GFX_LFMT_BASE_C8_2X2:
   case GFX_LFMT_BASE_BC4:
   case GFX_LFMT_BASE_EAC:
      return 1;
   default:
      unreachable();
      return 0;
   }
}
uint32_t gfx_lfmt_num_slots_from_type(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_UFLOAT:
   case GFX_LFMT_TYPE_FLOAT:
   case GFX_LFMT_TYPE_UINT:
   case GFX_LFMT_TYPE_INT:
   case GFX_LFMT_TYPE_UNORM:
   case GFX_LFMT_TYPE_SRGB:
   case GFX_LFMT_TYPE_SNORM:
      return 1;
   case GFX_LFMT_TYPE_FLOAT_UINT:
   case GFX_LFMT_TYPE_UINT_FLOAT:
   case GFX_LFMT_TYPE_FLOAT_UNORM:
   case GFX_LFMT_TYPE_UNORM_FLOAT:
   case GFX_LFMT_TYPE_UNORM_UINT:
   case GFX_LFMT_TYPE_UINT_UNORM:
      return 2;
   case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
   case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
      return 4;
   default:
      unreachable();
      return 0;
   }
}
uint32_t gfx_lfmt_num_slots_from_channels(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_R:
   case GFX_LFMT_CHANNELS_G:
   case GFX_LFMT_CHANNELS_B:
   case GFX_LFMT_CHANNELS_A:
   case GFX_LFMT_CHANNELS_X:
   case GFX_LFMT_CHANNELS_L:
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
   case GFX_LFMT_CHANNELS_Y:
   case GFX_LFMT_CHANNELS_U:
   case GFX_LFMT_CHANNELS_V:
      return 1;
   case GFX_LFMT_CHANNELS_RG:
   case GFX_LFMT_CHANNELS_GR:
   case GFX_LFMT_CHANNELS_RA:
   case GFX_LFMT_CHANNELS_AR:
   case GFX_LFMT_CHANNELS_RX:
   case GFX_LFMT_CHANNELS_XR:
   case GFX_LFMT_CHANNELS_XG:
   case GFX_LFMT_CHANNELS_GX:
   case GFX_LFMT_CHANNELS_LA:
   case GFX_LFMT_CHANNELS_AL:
   case GFX_LFMT_CHANNELS_LX:
   case GFX_LFMT_CHANNELS_XL:
   case GFX_LFMT_CHANNELS_DS:
   case GFX_LFMT_CHANNELS_SD:
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_SX:
   case GFX_LFMT_CHANNELS_XS:
   case GFX_LFMT_CHANNELS_UV:
   case GFX_LFMT_CHANNELS_VU:
      return 2;
   case GFX_LFMT_CHANNELS_RGB:
   case GFX_LFMT_CHANNELS_BGR:
   case GFX_LFMT_CHANNELS_RGX:
   case GFX_LFMT_CHANNELS_XGR:
   case GFX_LFMT_CHANNELS_YUV:
   case GFX_LFMT_CHANNELS_VUY:
      return 3;
   case GFX_LFMT_CHANNELS_RGBA:
   case GFX_LFMT_CHANNELS_BGRA:
   case GFX_LFMT_CHANNELS_ARGB:
   case GFX_LFMT_CHANNELS_ABGR:
   case GFX_LFMT_CHANNELS_RGBX:
   case GFX_LFMT_CHANNELS_BGRX:
   case GFX_LFMT_CHANNELS_XRGB:
   case GFX_LFMT_CHANNELS_XBGR:
   case GFX_LFMT_CHANNELS_RXXX:
   case GFX_LFMT_CHANNELS_XXXR:
   case GFX_LFMT_CHANNELS_RAXX:
   case GFX_LFMT_CHANNELS_XXAR:
   case GFX_LFMT_CHANNELS_YUYV:
   case GFX_LFMT_CHANNELS_VYUY:
   case GFX_LFMT_CHANNELS_YYUV:
   case GFX_LFMT_CHANNELS_VUYY:
      return 4;
   default:
      unreachable();
      return 0;
   }
}
GFX_LFMT_T gfx_lfmt_drop_subsample_size(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_is_std_with_subsample(lfmt)) {
      return lfmt;
   }
   switch (lfmt & GFX_LFMT_BASE_MASK) {
   case GFX_LFMT_BASE_C8_2X2:
      return (lfmt & ~GFX_LFMT_BASE_MASK) | GFX_LFMT_BASE_C8;
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_C8_2X1:
      return (lfmt & ~GFX_LFMT_BASE_MASK) | GFX_LFMT_BASE_C8_C8;
   case GFX_LFMT_BASE_C8_C8_C8_C8_2X1:
      return (lfmt & ~GFX_LFMT_BASE_MASK) | GFX_LFMT_BASE_C8_C8_C8_C8;
   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}
GFX_LFMT_T gfx_lfmt_ds_to_l(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_depth(lfmt) && !gfx_lfmt_has_stencil(lfmt)) {
      return lfmt;
   }
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_L;
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_SX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_LX;
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_XS:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XL;
   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}
GFX_LFMT_T gfx_lfmt_ds_to_red(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_depth(lfmt) && !gfx_lfmt_has_stencil(lfmt)) {
      return lfmt;
   }
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_D:
   case GFX_LFMT_CHANNELS_S:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_R;
   case GFX_LFMT_CHANNELS_DX:
   case GFX_LFMT_CHANNELS_SX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_RX;
   case GFX_LFMT_CHANNELS_XD:
   case GFX_LFMT_CHANNELS_XS:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XR;
   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}
GFX_LFMT_T gfx_lfmt_ds_to_rg(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_has_depth(lfmt) && !gfx_lfmt_has_stencil(lfmt)) {
      return lfmt;
   }
   switch (lfmt & GFX_LFMT_CHANNELS_MASK) {
   case GFX_LFMT_CHANNELS_D:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_R;
   case GFX_LFMT_CHANNELS_S:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_G;
   case GFX_LFMT_CHANNELS_DS:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_RG;
   case GFX_LFMT_CHANNELS_SD:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_GR;
   case GFX_LFMT_CHANNELS_DX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_RX;
   case GFX_LFMT_CHANNELS_XD:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XR;
   case GFX_LFMT_CHANNELS_SX:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_GX;
   case GFX_LFMT_CHANNELS_XS:
      return (lfmt & ~GFX_LFMT_CHANNELS_MASK) | GFX_LFMT_CHANNELS_XG;
   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}
GFX_LFMT_T gfx_lfmt_int_to_norm(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_contains_int(lfmt)) {
      return lfmt;
   }
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_UINT:
   case GFX_LFMT_TYPE_UNORM_UINT:
   case GFX_LFMT_TYPE_UINT_UNORM:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_UNORM;
   case GFX_LFMT_TYPE_INT:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_SNORM;
   case GFX_LFMT_TYPE_FLOAT_UINT:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_FLOAT_UNORM;
   case GFX_LFMT_TYPE_UINT_FLOAT:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_UNORM_FLOAT;
   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}
GFX_LFMT_T gfx_lfmt_snorm_to_unorm(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_contains_snorm(lfmt)) {
      return lfmt;
   }
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_SNORM:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_UNORM;
   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}
GFX_LFMT_T gfx_lfmt_srgb_to_unorm(GFX_LFMT_T lfmt)
{
   if (!gfx_lfmt_contains_srgb(lfmt)) {
      return lfmt;
   }
   switch (lfmt & GFX_LFMT_TYPE_MASK) {
   case GFX_LFMT_TYPE_SRGB:
   case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
   case GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_UNORM;
   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}
/* END AUTO-GENERATED CODE (misc_func_defns) */
