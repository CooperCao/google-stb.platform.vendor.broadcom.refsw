/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GL20_PROGRAM_H
#define GL20_PROGRAM_H

#include "../glxx/gl_public_api.h"

#include "../common/khrn_mem.h"

#include "../glxx/glxx_int_config.h"

#include "../glsl/glsl_program.h"
#include "../glsl/glsl_source.h"

#include "gl20_shader.h"
#include "../glxx/glxx_server_state.h"
#include "../glxx/glxx_shader_cache.h"

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
   GLenum    buffer_mode;   // requested buffer mode
   uint32_t  varying_count; // requested varyings
   char     *name[GLXX_CONFIG_MAX_TF_INTERLEAVED_COMPONENTS];
} GLXX_PROGRAM_TFF_PRE_LINK_T;

typedef struct {
   GLenum            buffer_mode;   // See Table 6.19 GL_INTERLEAVED_ATTRIBS / GL_SEPARATE_ATTRIBS
   uint32_t          varying_count;
   uint32_t          addr_count;
   uint32_t          spec_count;
   V3D_TF_SPEC_T     spec[V3D_MAX_TF_SPECS];
} GLXX_PROGRAM_TFF_POST_LINK_T;

/* Data that is common between pipelines and normal programs */
typedef struct GL20_PROGRAM_COMMON_T_ {
   unsigned       *ubo_binding_point;
   unsigned       *ssbo_binding_point;
   unsigned        num_scalar_uniforms;
   uint32_t       *uniform_data;

   GLSL_PROGRAM_T *linked_glsl_program;

   // We need to be able to have a failed link leave the linked program
   // in place for SSOs -- failed link does not break the pipelines for
   // some reason.
   bool link_status;
   bool separable;

   GLXX_BINARY_CACHE_T          cache;
   GLXX_PROGRAM_TFF_POST_LINK_T transform_feedback;
} GL20_PROGRAM_COMMON_T;

typedef struct GL20_PROGRAM_T_ {
   uint32_t            sig;
   uint32_t            refs;
   int32_t             name;

   bool                deleted;
   bool                validated;
   bool                binary_hint;
   bool                separable;

   GL20_SHADER_T      *vertex;
   GL20_SHADER_T      *fragment;
#if GLXX_HAS_TNG
   GL20_SHADER_T      *tess_control;
   GL20_SHADER_T      *tess_evaluation;
   GL20_SHADER_T      *geometry;
#endif
   GL20_SHADER_T      *compute;
   khrn_mem_handle_t   mh_info;
   char               *debug_label;

   GLSL_BINDING_T     *bindings;
   unsigned            num_bindings;

   GLXX_PROGRAM_TFF_PRE_LINK_T transform_feedback;
   GL20_PROGRAM_COMMON_T       common;
} GL20_PROGRAM_T;

extern void gl20_program_init(GL20_PROGRAM_T *program, int32_t name);
extern void gl20_program_term(void *v, size_t size);

extern void gl20_program_common_init(GL20_PROGRAM_COMMON_T *common);
extern void gl20_program_common_term(GL20_PROGRAM_COMMON_T *common);

extern bool gl20_program_bind_attrib(GL20_PROGRAM_T *program, uint32_t index, const char *name);

extern void gl20_program_acquire(GL20_PROGRAM_T *program);
extern void gl20_program_release(GL20_PROGRAM_T *program);

extern void gl20_program_link(GL20_PROGRAM_T *program);

extern int gl20_is_program(GL20_PROGRAM_T *program);

extern bool gl20_validate_program(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_COMMON_T *program);

extern void gl20_server_try_delete_program(GLXX_SHARED_T *shared, GL20_PROGRAM_T *program);

extern bool gl20_attach_shader(GL20_PROGRAM_T *program, GL20_SHADER_T *shader);
extern bool gl20_detach_shader(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_T *program, GL20_SHADER_T *shader);

// Exposed for glxx_server_transform_feedback.c
extern GL20_PROGRAM_T *gl20_get_program(GLXX_SERVER_STATE_T *state, GLuint p);

extern const GLSL_BLOCK_T *gl20_get_ubo_from_index (const GLSL_PROGRAM_T *p, unsigned int index);
extern const GLSL_BLOCK_T *gl20_get_ssbo_from_index(const GLSL_PROGRAM_T *p, unsigned int index);

// Returns common components of used program, or bound pipeline object program
extern GL20_PROGRAM_COMMON_T *gl20_program_common_get(const GLXX_SERVER_STATE_T *state);            // Never returns NULL

// Returns the currently used program or vertex stage program of the
// current pipeline object
extern GL20_PROGRAM_T *gl20_get_tf_program(const GLXX_SERVER_STATE_T *state);

extern void gl20_program_save_error(GL20_PROGRAM_T *program, const char *error);

extern bool gl20_program_attach_shader(GL20_PROGRAM_T *program, GL20_SHADER_T *shader);
extern bool gl20_program_detach_shader(GL20_PROGRAM_T *program, GL20_SHADER_T *shader);

#endif
