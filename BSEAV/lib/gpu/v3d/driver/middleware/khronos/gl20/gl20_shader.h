/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/include/GLES2/gl2.h"

#include "middleware/khronos/common/khrn_mem.h"

#define SIG_SHADER 0x0054ade7

typedef struct {
   uint32_t sig;
   int32_t refs;
   int32_t name;

   bool deleted;
   bool compiled;

   unsigned type;

   char **sourcev;
   unsigned sourcec;

   char *info_log;
} GL20_SHADER_T;

extern void gl20_shader_init(GL20_SHADER_T *shader, int32_t name, unsigned type);
extern void gl20_shader_term(MEM_HANDLE_T handle);

extern void gl20_shader_acquire(GL20_SHADER_T *shader);
extern void gl20_shader_release(GL20_SHADER_T *shader);

extern void gl20_shader_compile(GL20_SHADER_T *shader);

extern int gl20_is_shader(GL20_SHADER_T *shader);

extern bool gl20_shader_set_source(GL20_SHADER_T *shader, unsigned count, const char * const *string, const int *length);