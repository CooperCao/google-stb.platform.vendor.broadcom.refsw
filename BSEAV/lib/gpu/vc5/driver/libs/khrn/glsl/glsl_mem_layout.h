/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_UNIFORM_LAYOUT_H
#define GLSL_UNIFORM_LAYOUT_H

#include "glsl_common.h"
#include "glsl_symbols.h"

typedef enum
{
   LAYOUT_PRIM_NONMATRIX,
   LAYOUT_PRIM_MATRIX,
   LAYOUT_ARRAY,
   LAYOUT_STRUCT,
   LAYOUT_FLAVOUR_COUNT
} MemLayoutFlavour;

struct _MemLayout
{
   unsigned base_alignment;

   MemLayoutFlavour flavour;

   union {
      struct {
         unsigned   member_count;
         MemLayout *member_layout;
         unsigned   stride;
      } array_layout;

      struct {
         int  stride;
         bool row_major;
      } prim_matrix_layout;

      struct {
         int stride;
      } prim_nonmatrix_layout;

      /* Blocks are layed out as structs */
      struct {
         unsigned   member_count;
         MemLayout *member_layouts;
         unsigned  *member_offsets;
         unsigned   size;      /* Total size, in bytes, of this block */
      } struct_layout;
   } u;
};

MemLayout *glsl_mem_prim_nonmatrix_layout(int stride);

unsigned glsl_mem_calculate_block_layout(MemLayout *layout, const SymbolType *type);
unsigned glsl_mem_calculate_non_block_layout(MemLayout *layout, const SymbolType *type);

#endif /* GLSL_UNIFORM_LAYOUT_H */
