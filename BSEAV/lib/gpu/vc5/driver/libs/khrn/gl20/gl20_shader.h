/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GL20_SHADER_H
#define GL20_SHADER_H

#include "../glxx/gl_public_api.h"

#include "../common/khrn_mem.h"
#include "../glxx/glxx_shared.h"

#include "../glsl/glsl_compiler.h"

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

   GLenum type;

   char **source;
   int sourcec;

   CompiledShader *binary;

   char *info_log;

   char *debug_label;
} GL20_SHADER_T;

extern void gl20_shader_init(GL20_SHADER_T *shader, int32_t name, GLenum type);
extern void gl20_shader_term(void *v);

extern bool gl20_shader_set_source(GL20_SHADER_T *shader, unsigned count,
                                   const char *const *string, const int *length);

extern void gl20_shader_acquire(GL20_SHADER_T *shader);
extern void gl20_shader_release(GL20_SHADER_T *shader);

extern void gl20_shader_compile(GL20_SHADER_T *shader);

extern int gl20_is_shader(GL20_SHADER_T *shader);

extern void gl20_server_try_delete_shader(GLXX_SHARED_T *shared, GL20_SHADER_T *shader);

#endif
