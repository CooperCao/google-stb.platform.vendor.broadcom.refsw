/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_common.h"
#include "glsl_layout.h"
#include "glsl_errors.h"
#include "glsl_fastmem.h"
#include "glsl_globals.h"

LayoutIDList *glsl_lq_id_list_new(LayoutID *id) {
   LayoutIDList *ret = malloc_fast(sizeof(LayoutIDList));
   ret->l = id;
   ret->next = NULL;
   return ret;
}

LayoutIDList *glsl_lq_id_list_append(LayoutIDList *l, LayoutID *id) {
   LayoutIDList *n = l;
   while (n->next != NULL) n = n->next;
   n->next = malloc_fast(sizeof(LayoutIDList));
   n = n->next;
   n->l = id;
   n->next = NULL;
   return l;
}

static bool lq_takes_argument(LQ q) {
   switch (q) {
      case LQ_LOCATION:
      case LQ_BINDING:
      case LQ_OFFSET:
      case LQ_SIZE_X:
      case LQ_SIZE_Y:
      case LQ_SIZE_Z:
         return true;
      default:
         return false;
   }
}

static FormatQualifier to_format_qualifier(LQ id)
{
   switch(id) {
      case LQ_RGBA32F:     return FMT_RGBA32F;
      case LQ_RGBA16F:     return FMT_RGBA16F;
      case LQ_R32F:        return FMT_R32F;
      case LQ_RGBA8:       return FMT_RGBA8;
      case LQ_RGBA8_SNORM: return FMT_RGBA8_SNORM;
      case LQ_RGBA32I:     return FMT_RGBA32I;
      case LQ_RGBA16I:     return FMT_RGBA16I;
      case LQ_RGBA8I:      return FMT_RGBA8I;
      case LQ_R32I:        return FMT_R32I;
      case LQ_RGBA32UI:    return FMT_RGBA32UI;
      case LQ_RGBA16UI:    return FMT_RGBA16UI;
      case LQ_RGBA8UI:     return FMT_RGBA8UI;
      case LQ_R32UI:       return FMT_R32UI;
      default: unreachable(); return FMT_RGBA8;
   }
}

AdvancedBlendQualifier glsl_lq_to_abq(LQ lq)
{
   switch (lq) {
      case LQ_BLEND_SUPPORT_MULTIPLY       : return ADV_BLEND_MULTIPLY;
      case LQ_BLEND_SUPPORT_SCREEN         : return ADV_BLEND_SCREEN;
      case LQ_BLEND_SUPPORT_OVERLAY        : return ADV_BLEND_OVERLAY;
      case LQ_BLEND_SUPPORT_DARKEN         : return ADV_BLEND_DARKEN;
      case LQ_BLEND_SUPPORT_LIGHTEN        : return ADV_BLEND_LIGHTEN;
      case LQ_BLEND_SUPPORT_COLORDODGE     : return ADV_BLEND_COLORDODGE;
      case LQ_BLEND_SUPPORT_COLORBURN      : return ADV_BLEND_COLORBURN;
      case LQ_BLEND_SUPPORT_HARDLIGHT      : return ADV_BLEND_HARDLIGHT;
      case LQ_BLEND_SUPPORT_SOFTLIGHT      : return ADV_BLEND_SOFTLIGHT;
      case LQ_BLEND_SUPPORT_DIFFERENCE     : return ADV_BLEND_DIFFERENCE;
      case LQ_BLEND_SUPPORT_EXCLUSION      : return ADV_BLEND_EXCLUSION;
      case LQ_BLEND_SUPPORT_HSL_HUE        : return ADV_BLEND_HSL_HUE;
      case LQ_BLEND_SUPPORT_HSL_SATURATION : return ADV_BLEND_HSL_SATURATION;
      case LQ_BLEND_SUPPORT_HSL_COLOR      : return ADV_BLEND_HSL_COLOR;
      case LQ_BLEND_SUPPORT_HSL_LUMINOSITY : return ADV_BLEND_HSL_LUMINOSITY;
      case LQ_BLEND_SUPPORT_ALL_EQUATIONS  : return ADV_BLEND_ALL_EQUATIONS;
      default:
         unreachable();return ADV_BLEND_NONE;
   }
}

LayoutQualifier *glsl_layout_create(const LayoutIDList *l) {
   if (!g_InGlobalScope)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "layouts must be at global scope");

   LayoutQualifier *ret = malloc_fast(sizeof(LayoutQualifier));
   ret->qualified = 0;
   ret->unif_bits = 0;

   uint32_t block_pack_bits = 0;
   uint32_t matrix_order_bits = 0;

   for ( ; l != NULL; l = l->next) {
      LQ id = l->l->id;
      if (lq_takes_argument(id) && l->l->flavour == LQ_FLAVOUR_PLAIN)
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "layout requires an argument but none given");
      if (!lq_takes_argument(id) && l->l->flavour != LQ_FLAVOUR_PLAIN)
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "layout does not take an argument");

      switch (id) {
         case LQ_LOCATION:
            ret->qualified |= LOC_QUALED;
            ret->location = l->l->argument;
            break;
         case LQ_BINDING:
            ret->qualified |= BINDING_QUALED;
            ret->binding = l->l->argument;
            break;
         case LQ_OFFSET:
            ret->qualified |= OFFSET_QUALED;
            ret->offset = l->l->argument;
            break;
         case LQ_RGBA32F:
         case LQ_RGBA16F:
         case LQ_R32F:
         case LQ_RGBA8:
         case LQ_RGBA8_SNORM:
         case LQ_RGBA32I:
         case LQ_RGBA16I:
         case LQ_RGBA8I:
         case LQ_R32I:
         case LQ_RGBA32UI:
         case LQ_RGBA16UI:
         case LQ_RGBA8UI:
         case LQ_R32UI:
            if (ret->qualified & FORMAT_QUALED)
               glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Only one format may be specified per declaration");
            ret->qualified |= FORMAT_QUALED;
            ret->format = to_format_qualifier(id);
            break;
         case LQ_SHARED: block_pack_bits = LAYOUT_SHARED; break;
         case LQ_PACKED: block_pack_bits = LAYOUT_PACKED; break;
         case LQ_STD140: block_pack_bits = LAYOUT_STD140; break;
         case LQ_STD430: block_pack_bits = LAYOUT_STD430; break;
         case LQ_ROW_MAJOR:    matrix_order_bits = LAYOUT_ROW_MAJOR;    break;
         case LQ_COLUMN_MAJOR: matrix_order_bits = LAYOUT_COLUMN_MAJOR; break;
         default:
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Layout qualifier not valid in this context");
      }
      if ((block_pack_bits | matrix_order_bits) != 0) {
         ret->qualified |= UNIF_QUALED;
         ret->unif_bits |= (block_pack_bits | matrix_order_bits);
      }
   }
   return ret;
}
