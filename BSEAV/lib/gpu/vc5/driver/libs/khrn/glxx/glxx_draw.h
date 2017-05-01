/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_DRAW_H
#define GLXX_DRAW_H

#include "glxx_enum_types.h"
#include "glxx_server_state.h"
#include "../glsl/glsl_program.h"

/* Raw params as passed to glDraw* function by user */
typedef struct
{
   GLenum mode;

   GLuint start;
   GLuint end;

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
   .start = 0,                   \
   .end = (GLuint)-1,            \
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

extern void glintDrawArraysOrElements(GLXX_SERVER_STATE_T *state, const GLXX_DRAW_RAW_T *draw);

extern bool glxx_drawtex(GLXX_SERVER_STATE_T *state, float Xs, float Ys, float
      Zw, float Ws, float Hs);

/* Return the draw mode at the input to the rasteriser */
extern GLXX_PRIMITIVE_T glxx_get_rast_draw_mode(const GLSL_PROGRAM_T *p,
      GLXX_PRIMITIVE_T input_mode);

#endif
