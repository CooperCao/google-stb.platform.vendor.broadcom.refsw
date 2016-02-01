/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 2.0 shader structure declaration.
=============================================================================*/

#ifndef GL20_SHADER_H
#define GL20_SHADER_H

#include "interface/khronos/include/GLES2/gl2.h"

#include "middleware/khronos/common/khrn_mem.h"

/*
   The state required per shader object consists of:

   - An unsigned integer specifying the shader object name.
   X An integer holding the value of SHADER TYPE.
   X A boolean holding the delete status, initially FALSE.
   X A boolean holding the status of the last compile, initially FALSE.
   U An array of type char containing the information log, initially empty.
   U An integer holding the length of the information log.
   X An array of type char containing the concatenated shader string, initially empty.
   X An integer holding the length of the concatenated shader string.
*/

#define SIG_SHADER 0x0054ade7

typedef struct {
   uint32_t sig;
   int32_t refs;
   int32_t name;

   GLboolean deleted;
   GLboolean compiled;

   GLenum type;

   MEM_HANDLE_T mh_sources_current;
   MEM_HANDLE_T mh_sources_compile;

   MEM_HANDLE_T mh_info;
} GL20_SHADER_T;

extern void gl20_shader_init(GL20_SHADER_T *shader, int32_t name, GLenum type);
extern void gl20_shader_term(void *v, uint32_t size);

extern void gl20_shader_sources_term(void *v, uint32_t size);

extern void gl20_shader_acquire(GL20_SHADER_T *shader);
extern void gl20_shader_release(GL20_SHADER_T *shader);

extern void gl20_shader_compile(GL20_SHADER_T *shader);

extern int gl20_is_shader(GL20_SHADER_T *shader);

#endif