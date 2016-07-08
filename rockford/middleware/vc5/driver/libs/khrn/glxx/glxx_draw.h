/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
=============================================================================*/
#ifndef GLXX_DRAW_H
#define GLXX_DRAW_H

#include "glxx_enum_types.h"
#include "glxx_server_state.h"

/* Raw params as passed to glDraw* function by user */
typedef struct
{
   GLenum mode;

   GLuint min_index;
   GLuint max_index;

   GLsizei count; /* Unused if is_indirect */
   GLsizei instance_count; /* Unused if is_indirect */

   bool is_draw_arrays;
   /* is_draw_arrays only */
   GLint first; /* Unused if is_indirect */
   /* !is_draw_arrays only */
   GLenum index_type;
   const void *indices;

   GLint basevertex;
   GLuint baseinstance;

   bool is_indirect;
   /* is_indirect only */
   GLsizei num_indirect;
   GLsizei indirect_stride;
   const void *indirect;
} GLXX_DRAW_RAW_T;

#define GLXX_DRAW_RAW_DEFAULTS   \
   /* Must set mode */           \
   .min_index = 0,               \
   .max_index = (GLuint)-1,      \
   .count = 0,                   \
   .instance_count = 1,          \
   .is_draw_arrays = false,      \
   .first = 0,                   \
   .index_type = 0,              \
   .indices = NULL,              \
   .basevertex = 0,              \
   .baseinstance = 0,            \
   .is_indirect = false,         \
   .num_indirect = 1,            \
   .indirect_stride = 0,         \
   .indirect = NULL

/* Params after being checked by check_raw_draw_params(). This structure is
 * very similar to GLXX_DRAW_RAW_T, but the types reflect the constraints
 * checked by check_raw_draw_params(). eg count is unsigned here because it is
 * an error for it to be negative. */
typedef struct
{
   GLXX_PRIMITIVE_T mode;

   unsigned int min_index;
   unsigned int max_index;

   unsigned int count; /* Unused if is_indirect */
   unsigned int instance_count; /* Unused if is_indirect */

   bool is_draw_arrays;
   /* is_draw_arrays only */
   unsigned int first; /* Unused if is_indirect */
   /* !is_draw_arrays only */
   GLXX_INDEX_T index_type;
   const void *indices;

   int basevertex;
   unsigned int baseinstance;

   bool is_indirect;
   /* is_indirect only */
   unsigned int num_indirect;
   unsigned int indirect_stride;
   uintptr_t    indirect_offset;
} GLXX_DRAW_T;

typedef struct
{
   gmem_handle_t handle;
   size_t offset;
   bool needs_freeing;
   unsigned int size; /* size in bytes of the data allocated in handle */
} GLXX_STORAGE_T;

typedef struct
{
   unsigned int index; /* maximum index allowed across all enabled non-instance attributes */
   unsigned int instance; /* maximum instanced allowed across all enabled instance attributes */
} GLXX_ATTRIBS_MAX;

typedef struct
{
   uint32_t stride;
   uint32_t divisor;
} GLXX_VERTEX_BUFFER_CONFIG_T;

typedef struct
{
   GLXX_STORAGE_T array[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
}GLXX_VERTEX_POINTERS_T;

extern void glintDrawArraysOrElements(GLXX_SERVER_STATE_T *state, const GLXX_DRAW_RAW_T *draw);

extern bool glxx_drawtex(GLXX_SERVER_STATE_T *state, float Xs, float Ys, float
      Zw, float Ws, float Hs);

#endif
