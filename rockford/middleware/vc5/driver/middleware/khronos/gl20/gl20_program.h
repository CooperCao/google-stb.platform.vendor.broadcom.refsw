/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 2.0 program structure declaration.
=============================================================================*/

#ifndef GL20_PROGRAM_H
#define GL20_PROGRAM_H

#include "interface/khronos/glxx/gl_public_api.h"

#include "middleware/khronos/common/khrn_mem.h"

#include "interface/khronos/glxx/glxx_int_config.h"
#include "middleware/khronos/glxx/glxx_server.h"

#include "middleware/khronos/glsl/glsl_program.h"
#include "middleware/khronos/glsl/glsl_source.h"

#include "middleware/khronos/glxx/glxx_server.h"

#include "middleware/khronos/gl20/gl20_shader.h"

/*
   The state required per program object consists of:

   N An unsigned integer indicating the program object object name.
   X A boolean holding the delete status, initially FALSE.
   X A boolean holding the status of the last link attempt, initially FALSE.
   X A boolean holding the status of the last validation attempt, initally FALSE.
   X An integer holding the number of attached shader objects.
   X A list of unsigned integers to keep track of the names of the shader objects attached.
   U An array of type char containing the information log, initially empty.
   U An integer holding the length of the information log.
   - An integer holding the number of active uniforms.
   - For each active uniform, three integers, holding its location, size, and type,
     and an array of type char holding its name.
   - An array of words that hold the values of each active uniform.
   - An integer holding the number of active attributes.
   - For each active attbribute, three integers holding its location, size, and type,
     and an array of type char holding its name.
*/

#define SIG_PROGRAM 0x097067a8

typedef struct {
   struct {
      GLenum    buffer_mode;   // requested buffer mode
      uint32_t  varying_count; // requested varyings
      char     *name[GLXX_CONFIG_MAX_TF_INTERLEAVED_COMPONENTS];
   } pre_link;

   struct {
      GLenum            buffer_mode;   // See Table 6.19 GL_INTERLEAVED_ATTRIBS / GL_SEPARATE_ATTRIBS
      uint32_t          varying_count;
      uint32_t          addr_count;
      uint32_t          spec_count;
      V3D_TF_SPEC_T     spec[V3D_MAX_TF_SPECS];
   } post_link;
} GLXX_PROGRAM_TRANSFORM_FEEDBACK_T;

typedef struct GL20_PROGRAM_T_ {
   uint32_t            sig;
   int32_t             refs;
   int32_t             name;

   bool                deleted;
   bool                validated;
   bool                binary_hint;

   GL20_SHADER_T      *vertex;
   GL20_SHADER_T      *fragment;
   GL20_SHADER_T      *compute;
   KHRN_MEM_HANDLE_T   mh_info;
   char               *debug_label;

   GLSL_BINDING_T     *bindings;
   unsigned            num_bindings;
   unsigned           *ubo_binding_point;
   unsigned           *ssbo_binding_point;
   unsigned            num_scalar_uniforms;
   uint32_t           *uniform_data;

   GLSL_PROGRAM_T     *linked_glsl_program;

   GLXX_BINARY_CACHE_T cache;

   GLXX_PROGRAM_TRANSFORM_FEEDBACK_T transform_feedback;

} GL20_PROGRAM_T;

extern void gl20_program_init(GL20_PROGRAM_T *program, int32_t name);
extern void gl20_program_term(void *v, size_t size);

extern bool gl20_program_bind_attrib(GL20_PROGRAM_T *program, uint32_t index, const char *name);

extern void gl20_program_acquire(GL20_PROGRAM_T *program);
extern void gl20_program_release(GL20_PROGRAM_T *program);

extern void gl20_program_link(GL20_PROGRAM_T *program);

extern int gl20_is_program(GL20_PROGRAM_T *program);

extern bool gl20_validate_program(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_T *program);

extern void gl20_server_try_delete_program(GLXX_SHARED_T *shared, GL20_PROGRAM_T *program);

// Exposed for glxx_server_transform_feedback.c
extern GL20_PROGRAM_T *gl20_get_program(GLXX_SERVER_STATE_T *state, GLuint p);

extern GLSL_BLOCK_T *gl20_get_ubo_from_index (GLSL_PROGRAM_T *p, unsigned int index);
extern GLSL_BLOCK_T *gl20_get_ssbo_from_index(GLSL_PROGRAM_T *p, unsigned int index);

#endif
