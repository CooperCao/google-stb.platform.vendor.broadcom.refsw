/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
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
   MEMBER_PRIMITIVE_NONMATRIX,
   MEMBER_PRIMITIVE_MATRIX,
   MEMBER_ARRAY,
   MEMBER_STRUCT,
   MEMBER_BLOCK,
   MEMBER_FLAVOUR_COUNT
} MEMBER_FLAVOUR_T;

struct _MEMBER_LAYOUT_T
{
   /* offset from the beginning of the uniform block
      as determined by the packing rules */
   int offset;
   int base_alignment;

   /* If this member is contained in a struct or array,
      then 'level_offset' gives the offset from the
      beginning of the struct or array. */
   int level_offset;

   SymbolType *type;

   MEMBER_FLAVOUR_T flavour;

   union
   {
      struct
      {
         unsigned int member_count;
         int stride;
         MEMBER_LAYOUT_T *member_layouts;  /* Array of member layouts */
      } array_layout;

      struct
      {
         int stride;
         bool column_major;
      } matrix_layout;

      struct
      {
         int stride;
      } vector_layout;

      struct
      {
         unsigned int member_count;
         MEMBER_LAYOUT_T *member_layouts;
      } struct_layout;

      struct
      {
         unsigned size;      /* Total size, in bytes, of this uniform block */
         unsigned int member_count;
         MEMBER_LAYOUT_T *member_layouts;
      } block_layout;
   } u;
};


MEMBER_LAYOUT_T *construct_prim_vector_layout(int stride);
MEMBER_LAYOUT_T *construct_prim_scalar_layout();

int deep_member_count(const MEMBER_LAYOUT_T *layout);

int calculate_block_layout(MEMBER_LAYOUT_T *layout, SymbolType *type);
int calculate_non_block_layout(MEMBER_LAYOUT_T *layout, SymbolType *type);

#endif /* GLSL_UNIFORM_LAYOUT_H */
