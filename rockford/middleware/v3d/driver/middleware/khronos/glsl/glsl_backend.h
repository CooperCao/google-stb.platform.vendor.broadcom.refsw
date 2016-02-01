/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_BACKEND_H
#define GLSL_BACKEND_H

extern bool glsl_backend_create_shaders(
   slang_program *program,
   Dataflow *vertex_x,
   Dataflow *vertex_y,
   Dataflow *vertex_z,
   Dataflow *vertex_w,
   Dataflow *vertex_point_size,
   Dataflow **vertex_vary,
   uint32_t vary_count,
   Dataflow *frag_r,
   Dataflow *frag_g,
   Dataflow *frag_b,
   Dataflow *frag_a,
   Dataflow *frag_discard);

extern uint32_t glsl_backend_get_schedule_type(Dataflow *dataflow);

extern bool glsl_backend_schedule(Dataflow *root, uint32_t type, bool *allow_thread);
extern bool glsl_bcg_backend_schedule(Dataflow *root, uint32_t type, MEM_HANDLE_T *mh_code, MEM_HANDLE_T *mh_uniform_map, bool *allow_thread,
                                      uint32_t *vary_map, uint32_t *vary_count);

#define SCHEDULE_TYPE_INPUT (1<<0)
#define SCHEDULE_TYPE_OUTPUT (1<<1)
#define SCHEDULE_TYPE_ALU (1<<2)
#define SCHEDULE_TYPE_SIG (1<<4)

#endif