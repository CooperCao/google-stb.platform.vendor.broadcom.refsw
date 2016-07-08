/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_uniform_layout.h"
#include "glsl_symbols.h"
#include "glsl_common.h"
#include "glsl_fastmem.h"
#include "glsl_primitive_types.auto.h"

#define BASIC_MACHINE_UNIT 4

static int calculate_prim_nonmatrix_layout(MEMBER_LAYOUT_T *layout, SymbolType *type, int offset, int level_start);
static int calculate_prim_matrix_layout(MEMBER_LAYOUT_T *layout, SymbolType *type, int offset, int level_start, LayoutQualifier *lq, int layout_spec);
static int calculate_struct_layout(MEMBER_LAYOUT_T *layout, SymbolType *type, int offset, int level_start, LayoutQualifier *lq, int layout_spec);
static int calculate_array_layout(MEMBER_LAYOUT_T *layout, SymbolType *type, int offset, int level_start, LayoutQualifier *lq, int layout_spec);

MEMBER_LAYOUT_T *construct_prim_vector_layout(int stride) {
   MEMBER_LAYOUT_T *layout = malloc_fast(sizeof(MEMBER_LAYOUT_T));
   layout->flavour = MEMBER_PRIMITIVE_NONMATRIX;
   layout->u.vector_layout.stride = stride;
   return layout;
}


/* ------------------------------- Layout tables ---------------------------- */
/* We use the std430 layout when asked for packed because it's most efficient */
#define STD140 0
#define STD430 1

static const int struct_alignment[2] = { 4, 1 };
static const int array_alignment[2]  = { 4, 1 };

static inline uint32_t basic_layout_align(PrimitiveTypeIndex t) {
   static const int aligns_by_scalar_count[5] = { 0, 1, 2, 4, 4 };
   assert(primitiveTypeFlags[t] & (PRIM_SCALAR_TYPE | PRIM_VECTOR_TYPE));
   assert(primitiveTypes[t].scalar_count > 0 && primitiveTypes[t].scalar_count <= 4);
   return aligns_by_scalar_count[primitiveTypes[t].scalar_count];
}
static inline uint32_t basic_layout_size(PrimitiveTypeIndex t) {
   assert(primitiveTypeFlags[t] & (PRIM_SCALAR_TYPE | PRIM_VECTOR_TYPE));
   return (uint32_t)primitiveTypes[t].scalar_count;
}

// ------------------------ Packing functions ------------------------

static inline int round_mult(int num, int multiple)
{
   if(multiple==0 || multiple==1) return num;

   int rem = num % multiple;
   if (rem == 0) return num;
   return (num + multiple - rem);
}

/* Called from outside for non-block uniforms and from calculate_block_type_layout */
static int calculate_non_block_type_layout(MEMBER_LAYOUT_T *layout, SymbolType *type, int offset, int level_start, LayoutQualifier *lq, int layout_spec)
{
   switch(type->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
         {
            PRIMITIVE_TYPE_FLAGS_T flags = primitiveTypeFlags[type->u.primitive_type.index];

            if(flags & PRIM_MATRIX_TYPE)
               offset = calculate_prim_matrix_layout(layout, type, offset, level_start, lq, layout_spec);
            else
               offset = calculate_prim_nonmatrix_layout(layout, type, offset, level_start);
         }
         break;
      case SYMBOL_ARRAY_TYPE:
         offset = calculate_array_layout(layout, type, offset, level_start, lq, layout_spec);
         break;
      case SYMBOL_STRUCT_TYPE:
         offset = calculate_struct_layout(layout, type, offset, level_start, lq, layout_spec);
         break;
      case SYMBOL_BLOCK_TYPE:    // blocks cannot be nested
      case SYMBOL_FUNCTION_TYPE: // functions can't be members of interface blocks
         UNREACHABLE();
         break;
   }

   return offset;
}

int calculate_non_block_layout(MEMBER_LAYOUT_T *layout, SymbolType *type) {
   return calculate_non_block_type_layout(layout, type, 0, 0, NULL, STD430);
}

static int get_max_member_alignment(int count, StructMember *member, LayoutQualifier *lq, int layout_spec) {
   int base_alignment = 0;
   for(int i=0; i<count; i++)
   {
      MEMBER_LAYOUT_T dummy;
      if (member[i].layout != NULL) lq = member[i].layout;
      calculate_non_block_type_layout(&dummy, member[i].type, 0, 0, lq, layout_spec);
      if(dummy.base_alignment > base_alignment) base_alignment = dummy.base_alignment;
   }
   return base_alignment;
}

static int get_layout_spec(LayoutQualifier *lq) {
   if( lq && (lq->qualified & UNIF_QUALED) && (lq->unif_bits & LAYOUT_STD140) )
      return STD140;
   else
      return STD430;
}

int calculate_block_layout(MEMBER_LAYOUT_T *layout, SymbolType *type)
{
   int offset = 0;

   assert(type->flavour==SYMBOL_BLOCK_TYPE);

   int layout_spec = get_layout_spec(type->u.block_type.lq);

   layout->type = type;
   layout->flavour = MEMBER_BLOCK;

   layout->u.block_layout.member_count = type->u.block_type.member_count;
   layout->u.block_layout.member_layouts = malloc_fast(type->u.block_type.member_count *
                                                       sizeof(MEMBER_LAYOUT_T));

   int base_alignment = get_max_member_alignment(type->u.block_type.member_count, type->u.block_type.member, NULL, layout_spec);

   /* Blocks are laid out as a struct with base offset 0 */
   base_alignment = round_mult(base_alignment, struct_alignment[layout_spec]*BASIC_MACHINE_UNIT);
   layout->base_alignment = base_alignment;
   layout->offset = 0;
   layout->level_offset = 0;

   for(unsigned i=0; i<type->u.block_type.member_count; i++)
   {
      SymbolType *member_type = type->u.block_type.member[i].type;
      LayoutQualifier *lq = type->u.block_type.member[i].layout;

      offset = calculate_non_block_type_layout( &layout->u.block_layout.member_layouts[i], member_type, offset, layout->offset, lq, layout_spec);
   }

   layout->u.block_layout.size = offset - layout->offset;
   return offset;
}

static int calculate_struct_layout(MEMBER_LAYOUT_T *layout, SymbolType *type, int offset, int level_start, LayoutQualifier *lq, int layout_spec)
{
   assert(type->flavour==SYMBOL_STRUCT_TYPE);
   int member_count = type->u.struct_type.member_count;

   layout->type = type;
   layout->flavour = MEMBER_STRUCT;

   layout->u.struct_layout.member_count = member_count;
   layout->u.struct_layout.member_layouts = (MEMBER_LAYOUT_T*)malloc_fast(member_count * sizeof(MEMBER_LAYOUT_T));

   int base_alignment = get_max_member_alignment(member_count, type->u.struct_type.member, lq, layout_spec);

   base_alignment = round_mult(base_alignment, struct_alignment[layout_spec]*BASIC_MACHINE_UNIT);
   layout->base_alignment = base_alignment;
   layout->offset = offset = round_mult(offset, base_alignment);
   layout->level_offset = layout->offset - level_start;

   for(int i=0; i<member_count; i++)
   {
      SymbolType *member_type = type->u.struct_type.member[i].type;

      offset = calculate_non_block_type_layout( &layout->u.struct_layout.member_layouts[i], member_type, offset, layout->offset, lq, layout_spec);
   }

   offset = round_mult(offset, layout->base_alignment);
   return offset;
}

static int calculate_array_layout(MEMBER_LAYOUT_T *layout, SymbolType *type, int offset, int level_start, LayoutQualifier *lq, int layout_spec)
{
   MEMBER_LAYOUT_T dummy;

   assert(type->flavour==SYMBOL_ARRAY_TYPE);

   layout->type = type;
   layout->flavour = MEMBER_ARRAY;

   unsigned n_member_layouts = type->u.array_type.member_count > 0 ? type->u.array_type.member_count : 1;
   layout->u.array_layout.member_count = type->u.array_type.member_count;
   layout->u.array_layout.member_layouts = malloc_fast(n_member_layouts * sizeof(MEMBER_LAYOUT_T));

   int member_size = calculate_non_block_type_layout(&dummy, type->u.array_type.member_type, 0, 0, lq, layout_spec);

   layout->base_alignment = round_mult(dummy.base_alignment, array_alignment[layout_spec]*BASIC_MACHINE_UNIT);
   layout->offset = round_mult(offset, layout->base_alignment);
   layout->level_offset = layout->offset - level_start;
   layout->u.array_layout.stride = round_mult(member_size, layout->base_alignment);

   offset = level_start = layout->offset;

   for(unsigned i=0; i<n_member_layouts; i++)
   {
      int new_offset = calculate_non_block_type_layout(&layout->u.array_layout.member_layouts[i],
                                                       type->u.array_type.member_type,
                                                       offset, level_start, lq, layout_spec);

      vcos_unused_in_release(new_offset);
      assert(new_offset == offset + member_size);
      offset += layout->u.array_layout.stride;
   }

   return round_mult(offset, layout->base_alignment);
}

static int calculate_prim_matrix_layout(MEMBER_LAYOUT_T *layout, SymbolType *type, int offset, int level_start, LayoutQualifier *lq, int layout_spec)
{
   int vectors;
   int vector_size;
   PrimitiveTypeIndex vector_type_index;
   MEMBER_LAYOUT_T dummy;

   assert(type->flavour==SYMBOL_PRIMITIVE_TYPE);

   layout->type = type;
   layout->flavour = MEMBER_PRIMITIVE_MATRIX;
   layout->u.matrix_layout.column_major = !glsl_is_row_major(lq);

   if(glsl_is_row_major(lq)) {
      // store as array of row vectors
      vectors = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 1);
      vector_type_index = glsl_prim_matrix_type_subscript_vector(type->u.primitive_type.index, 0);
   } else {
      // store as array of column vectors:
      vectors = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 0);
      vector_type_index = glsl_prim_matrix_type_subscript_vector(type->u.primitive_type.index, 1);
   }

   vector_size = calculate_prim_nonmatrix_layout(&dummy, &primitiveTypes[vector_type_index], 0, 0);

   layout->base_alignment = round_mult(dummy.base_alignment, array_alignment[layout_spec]*BASIC_MACHINE_UNIT);
   layout->offset = round_mult(offset, layout->base_alignment);
   layout->level_offset = layout->offset - level_start;
   layout->u.matrix_layout.stride = round_mult(vector_size, layout->base_alignment);

   return layout->offset + vectors * layout->u.matrix_layout.stride;
}

/* Layout are built up in terms of scalar and vector types */
static bool is_prim_type_basic(SymbolType *type) {
   assert(type->flavour==SYMBOL_PRIMITIVE_TYPE);
   return primitiveTypeFlags[type->u.primitive_type.index] &
                                  (PRIM_SCALAR_TYPE | PRIM_VECTOR_TYPE);
}

static int calculate_prim_nonmatrix_layout(MEMBER_LAYOUT_T *layout, SymbolType *type, int offset, int level_start)
{
   int size;
   layout->type = type;

   assert(type->flavour==SYMBOL_PRIMITIVE_TYPE);

   if (is_prim_type_basic(type)) {
      PrimitiveTypeIndex idx = type->u.primitive_type.index;
      layout->base_alignment = basic_layout_align(idx) * BASIC_MACHINE_UNIT;
      size                   = basic_layout_size (idx) * BASIC_MACHINE_UNIT;
   } else {
      /* A non-block uniform can still get a layout from here. */
      layout->base_alignment = BASIC_MACHINE_UNIT;
      size                   = BASIC_MACHINE_UNIT;
   }

   layout->flavour = MEMBER_PRIMITIVE_NONMATRIX;
   layout->offset = round_mult(offset, layout->base_alignment);
   layout->level_offset = layout->offset - level_start;
   /* TODO: This is a little dodgy for scalars, but fine. */
   /* TODO: Also, is it correct? */
   layout->u.vector_layout.stride = BASIC_MACHINE_UNIT;

   return layout->offset + size;
}
