/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_INTRINSIC_TYPES_H_INCLUDED
#define GLSL_INTRINSIC_TYPES_H_INCLUDED

typedef enum glsl_intrinsic_index_e {
   INTRINSIC_TEXTURE,
   INTRINSIC_TEXTURE_SIZE,
   INTRINSIC_RSQRT,
   INTRINSIC_RCP,
   INTRINSIC_LOG2,
   INTRINSIC_EXP2,
   INTRINSIC_CEIL,
   INTRINSIC_FLOOR,
   INTRINSIC_TRUNC,
   INTRINSIC_NEAREST,
   INTRINSIC_MIN,
   INTRINSIC_MAX,
   INTRINSIC_ABS,
   INTRINSIC_FDX,
   INTRINSIC_FDY,
   INTRINSIC_CLZ,
   INTRINSIC_REINTERPF,
   INTRINSIC_REINTERPI,
   INTRINSIC_REINTERPU,
   INTRINSIC_FPACK,
   INTRINSIC_FUNPACKA,
   INTRINSIC_FUNPACKB,
   INTRINSIC_SIN,
   INTRINSIC_COS,
   INTRINSIC_TAN,
   INTRINSIC_ATOMIC_LOAD,
   INTRINSIC_ATOMIC_ADD,
   INTRINSIC_ATOMIC_SUB,
   INTRINSIC_ATOMIC_MIN,
   INTRINSIC_ATOMIC_MAX,
   INTRINSIC_ATOMIC_AND,
   INTRINSIC_ATOMIC_OR,
   INTRINSIC_ATOMIC_XOR,
   INTRINSIC_ATOMIC_XCHG,
   INTRINSIC_ATOMIC_CMPXCHG,
   INTRINSIC_IMAGE_STORE,
   INTRINSIC_IMAGE_SIZE,
   INTRINSIC_COUNT, // MARKER
} glsl_intrinsic_index_t;

typedef struct glsl_intrinsic_data_s {
   const char *name;
   glsl_intrinsic_index_t index;
} glsl_intrinsic_data_t;

#endif
