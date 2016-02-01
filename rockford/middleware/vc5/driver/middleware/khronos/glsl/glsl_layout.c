/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
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
         case LQ_SHARED: block_pack_bits = LAYOUT_SHARED; break;
         case LQ_PACKED: block_pack_bits = LAYOUT_PACKED; break;
         case LQ_STD140: block_pack_bits = LAYOUT_STD140; break;
         case LQ_STD430: block_pack_bits = LAYOUT_STD430; break;
         case LQ_ROW_MAJOR:    matrix_order_bits = LAYOUT_ROW_MAJOR;    break;
         case LQ_COLUMN_MAJOR: matrix_order_bits = LAYOUT_COLUMN_MAJOR; break;
         default:
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Invalid layout qualifier");
      }
      if (block_pack_bits != 0) {
         ret->qualified |= UNIF_QUALED;
         ret->unif_bits |= block_pack_bits;
      }
      if (matrix_order_bits != 0) {
         ret->qualified |= UNIF_QUALED;
         ret->unif_bits |= matrix_order_bits;
      }
   }
   return ret;
}
