/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_mem_layout.h"
#include "glsl_symbols.h"
#include "glsl_common.h"
#include "glsl_fastmem.h"
#include "glsl_primitive_types.auto.h"

#include "libs/util/gfx_util/gfx_util.h"

/* ------------------------------- Layout tables ---------------------------- */
/* We use the shared layout when asked for packed because it's most efficient */

#define BASIC_MACHINE_UNIT 4

typedef enum {
   STD140 = 0,
   STD430 = 1,
   SHARED = 2
} LayoutSpec;

static unsigned calculate_prim_nonmatrix_layout(MemLayout *layout, const SymbolType *type);
static unsigned calculate_prim_matrix_layout(MemLayout *layout, const SymbolType *type, const LayoutQualifier *lq, LayoutSpec s);
static unsigned calculate_struct_layout(MemLayout *layout, const SymbolType *type, const LayoutQualifier *lq, LayoutSpec s);
static unsigned calculate_array_layout(MemLayout *layout, const SymbolType *type, const LayoutQualifier *lq, LayoutSpec s);

static const unsigned struct_alignment[3] = { 4, 1, 1 };
static const unsigned  array_alignment[3] = { 4, 1, 1 };

static inline uint32_t basic_layout_align(PrimitiveTypeIndex t) {
   static const unsigned aligns_by_scalar_count[5] = { 0, 1, 2, 4, 4 };
   assert(primitiveTypeFlags[t] & (PRIM_SCALAR_TYPE | PRIM_VECTOR_TYPE));
   assert(primitiveTypes[t].scalar_count > 0 && primitiveTypes[t].scalar_count <= 4);
   return aligns_by_scalar_count[primitiveTypes[t].scalar_count];
}
static inline unsigned basic_layout_size(PrimitiveTypeIndex t) {
   assert(primitiveTypeFlags[t] & (PRIM_SCALAR_TYPE | PRIM_VECTOR_TYPE));
   return primitiveTypes[t].scalar_count;
}

/* ------------------------ Packing functions ------------------------ */

MemLayout *glsl_mem_prim_nonmatrix_layout(int stride) {
   MemLayout *layout = malloc_fast(sizeof(MemLayout));
   layout->flavour = LAYOUT_PRIM_NONMATRIX;
   layout->u.prim_nonmatrix_layout.stride = stride;
   return layout;
}

static unsigned calculate_non_block_type_layout(MemLayout *layout, const SymbolType *type, const LayoutQualifier *lq, LayoutSpec s)
{
   switch(type->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
      {
         PRIMITIVE_TYPE_FLAGS_T flags = primitiveTypeFlags[type->u.primitive_type.index];
         if(flags & PRIM_MATRIX_TYPE)
            return calculate_prim_matrix_layout(layout, type, lq, s);
         else
            return calculate_prim_nonmatrix_layout(layout, type);
      }
      case SYMBOL_ARRAY_TYPE:
         return calculate_array_layout(layout, type, lq, s);
      case SYMBOL_STRUCT_TYPE:
         return calculate_struct_layout(layout, type, lq, s);
      default:
         unreachable();
         return 0;
   }
}

unsigned glsl_mem_calculate_non_block_layout(MemLayout *layout, const SymbolType *type) {
   return calculate_non_block_type_layout(layout, type, NULL, SHARED);
}

static LayoutSpec get_layout_spec(const LayoutQualifier *lq) {
   if (!lq || !(lq->qualified & UNIF_QUALED)) return SHARED;

   if (lq->unif_bits & LAYOUT_STD140) return STD140;
   if (lq->unif_bits & LAYOUT_STD430) return STD430;
   else                               return SHARED;
}

static unsigned struct_layout(MemLayout *layout, unsigned member_count, const StructMember *members,
                              const LayoutQualifier *lq, LayoutSpec s)
{
   layout->flavour = LAYOUT_STRUCT;
   layout->u.struct_layout.member_count   = member_count;
   layout->u.struct_layout.member_layouts = malloc_fast(member_count * sizeof(MemLayout));
   layout->u.struct_layout.member_offsets = malloc_fast(member_count * sizeof(int));

   layout->base_alignment = 0; /* Built up as we go along */

   unsigned offset = 0;
   for(unsigned i=0; i<member_count; i++)
   {
      SymbolType *member_type = members[i].type;
      const LayoutQualifier *l = lq ? lq : members[i].layout;

      unsigned size = calculate_non_block_type_layout( &layout->u.struct_layout.member_layouts[i], member_type, l, s);
      unsigned member_alignment = layout->u.struct_layout.member_layouts[i].base_alignment;
      layout->base_alignment = gfx_umax(layout->base_alignment, member_alignment);
      offset = gfx_uround_up(offset, member_alignment);
      layout->u.struct_layout.member_offsets[i] = offset;
      offset += size;
      if (i+1 < member_count && (s == STD140 || s == STD430) &&
           (member_type->flavour == SYMBOL_STRUCT_TYPE || member_type->flavour == SYMBOL_ARRAY_TYPE))
      {
         offset = gfx_uround_up(offset, member_alignment);
      }
   }

   layout->base_alignment = gfx_uround_up(layout->base_alignment, struct_alignment[s]*BASIC_MACHINE_UNIT);
   layout->u.struct_layout.size = offset;

   return offset;
}

/* Blocks are laid out the same as structs */
unsigned glsl_mem_calculate_block_layout(MemLayout *layout, const SymbolType *type) {
   assert(type->flavour == SYMBOL_BLOCK_TYPE);
   LayoutSpec s = get_layout_spec(type->u.block_type.lq);
   return struct_layout(layout, type->u.block_type.member_count, type->u.block_type.member, NULL, s);
}

static unsigned calculate_struct_layout(MemLayout *layout, const SymbolType *type, const LayoutQualifier *lq, LayoutSpec s)
{
   return struct_layout(layout, type->u.struct_type.member_count, type->u.struct_type.member, lq, s);
}

static unsigned calculate_array_layout(MemLayout *layout, const SymbolType *type, const LayoutQualifier *lq, LayoutSpec s)
{
   layout->flavour = LAYOUT_ARRAY;
   layout->u.array_layout.member_count = type->u.array_type.member_count;
   layout->u.array_layout.member_layout = malloc_fast(sizeof(MemLayout));

   unsigned member_size = calculate_non_block_type_layout(layout->u.array_layout.member_layout,
                                                          type->u.array_type.member_type, lq, s);
   layout->base_alignment = gfx_uround_up(layout->u.array_layout.member_layout->base_alignment, array_alignment[s]*BASIC_MACHINE_UNIT);
   layout->u.array_layout.stride = gfx_uround_up(member_size, layout->base_alignment);

   unsigned n_member_layouts = type->u.array_type.member_count > 0 ? type->u.array_type.member_count : 1;
   return (n_member_layouts - 1) * layout->u.array_layout.stride + member_size;
}

static bool is_row_major(const LayoutQualifier *lq) {
   return lq && (lq->qualified & UNIF_QUALED) && (lq->unif_bits & LAYOUT_ROW_MAJOR);
}

static unsigned calculate_prim_matrix_layout(MemLayout *layout, const SymbolType *type, const LayoutQualifier *lq, LayoutSpec s)
{
   layout->flavour = LAYOUT_PRIM_MATRIX;
   layout->u.prim_matrix_layout.row_major = is_row_major(lq);

   int vectors;
   PrimitiveTypeIndex vector_type_index;
   if(is_row_major(lq)) {
      // store as array of row vectors
      vectors = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 1);
      vector_type_index = glsl_prim_matrix_type_subscript_vector(type->u.primitive_type.index, 0);
   } else {
      // store as array of column vectors:
      vectors = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 0);
      vector_type_index = glsl_prim_matrix_type_subscript_vector(type->u.primitive_type.index, 1);
   }

   MemLayout dummy;
   int vector_size = calculate_prim_nonmatrix_layout(&dummy, &primitiveTypes[vector_type_index]);

   layout->base_alignment = gfx_uround_up(dummy.base_alignment, array_alignment[s]*BASIC_MACHINE_UNIT);
   layout->u.prim_matrix_layout.stride = gfx_uround_up(vector_size, layout->base_alignment);

   return vectors * layout->u.prim_matrix_layout.stride;
}

/* Layout are built up in terms of scalar and vector types */
static bool is_prim_type_basic(const SymbolType *type) {
   assert(type->flavour==SYMBOL_PRIMITIVE_TYPE);
   return primitiveTypeFlags[type->u.primitive_type.index] &
                                  (PRIM_SCALAR_TYPE | PRIM_VECTOR_TYPE);
}

static unsigned calculate_prim_nonmatrix_layout(MemLayout *layout, const SymbolType *type)
{
   unsigned size;
   if (is_prim_type_basic(type)) {
      PrimitiveTypeIndex idx = type->u.primitive_type.index;
      layout->base_alignment = basic_layout_align(idx) * BASIC_MACHINE_UNIT;
      size                   = basic_layout_size (idx) * BASIC_MACHINE_UNIT;
   } else {
      /* A non-block uniform can still get a layout from here. */
      layout->base_alignment = BASIC_MACHINE_UNIT;
      size                   = BASIC_MACHINE_UNIT;
   }

   layout->flavour = LAYOUT_PRIM_NONMATRIX;
   layout->u.prim_nonmatrix_layout.stride = BASIC_MACHINE_UNIT;

   return size;
}
