/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/common/khrn_mem.h"
#include "middleware/khronos/gl20/2708/gl20_shader_4.h"
#include "middleware/khronos/glxx/glxx_server.h"

typedef struct {
   uint32_t index;
   char *name;
} GL20_BINDING_T;

typedef struct {
   int offset;
   unsigned type;
   char *name;
} GL20_ATTRIB_INFO_T;

#define SIG_PROGRAM 0x097067a8

typedef struct {
   uint32_t sig;
   int32_t  refs;
   int32_t  name;
   uint32_t debug_save_count;

   bool deleted;
   bool linked;
   bool validated;

   MEM_HANDLE_T mh_vertex;
   MEM_HANDLE_T mh_fragment;

   void *bindings;      /*GL20_BINDING_T*/
   unsigned num_bindings;

   GL20_LINK_RESULT_T result;

   int32_t attribs_live;

   void *samplers; /*GL20_SAMPLER_INFO_T*/
   unsigned num_samplers;

   void *uniforms; /*GL20_UNIFORM_INFO_T*/
   unsigned num_uniforms;

   void *uniform_data;
   unsigned num_uniform_data;

   void *attributes;    /*GL20_ATTRIB_INFO_T*/
   unsigned num_attributes;

   char *info_log;
} GL20_PROGRAM_T;

extern void gl20_program_init(GL20_PROGRAM_T *program, int32_t name);
extern void gl20_program_term(MEM_HANDLE_T handle);

extern bool gl20_program_bind_attrib(GL20_PROGRAM_T *program, unsigned index, const char *name);

extern void gl20_program_acquire(GL20_PROGRAM_T *program);
extern void gl20_program_release(GL20_PROGRAM_T *program);

extern void gl20_program_link(GL20_PROGRAM_T *program);

extern int gl20_is_program(GL20_PROGRAM_T *program);

extern GLboolean gl20_validate_current_program(GLXX_SERVER_STATE_T *state);
extern GLboolean gl20_validate_program(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_T *program);