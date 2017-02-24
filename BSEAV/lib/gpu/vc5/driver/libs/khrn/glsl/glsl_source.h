/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

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
