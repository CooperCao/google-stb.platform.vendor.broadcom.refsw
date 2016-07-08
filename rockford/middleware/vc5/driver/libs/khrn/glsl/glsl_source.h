/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_SOURCE_H_INCLUDED
#define GLSL_SOURCE_H_INCLUDED

#include "../glxx/gl_public_api.h"

typedef struct {
   /* Inputs to compile */
   const char * const *sourcev;
   int  sourcec;
   int  name;
} GLSL_SHADER_SOURCE_T;

typedef struct {
   char *name;
   int   index;
} GLSL_BINDING_T;

typedef struct {
   /* Inputs to link */
   unsigned              num_bindings;
   GLSL_BINDING_T       *bindings;
   unsigned              num_tf_varyings;
   const char          **tf_varyings;
   int                   name;
} GLSL_PROGRAM_SOURCE_T;

void glsl_shader_source_dump(const GLSL_SHADER_SOURCE_T *src);
void glsl_program_source_dump(const GLSL_PROGRAM_SOURCE_T *p, unsigned vertex_name, unsigned fragmnt_name);

#endif
