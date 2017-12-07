/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_common.h"
#include "glsl_symbols.h"

EXTERN_C_BEGIN

typedef enum {
   LAYOUT_PRIM_NONMATRIX,
   LAYOUT_PRIM_MATRIX,
   LAYOUT_ARRAY,
   LAYOUT_STRUCT,
   LAYOUT_FLAVOUR_COUNT
} MemLayoutFlavour;

typedef enum {
   MEM_STD140  = 0,
   MEM_STD430  = 1,
   MEM_TMU_OPT = 2,
   MEM_PACKED  = 3
} LayoutSpec;

struct _MemLayout {
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

void glsl_mem_calculate_block_layout(MemLayout *layout, const SymbolType *type, bool for_tmu);
void glsl_mem_calculate_non_block_layout(MemLayout *layout, const SymbolType *type, LayoutSpec spec);

EXTERN_C_END
