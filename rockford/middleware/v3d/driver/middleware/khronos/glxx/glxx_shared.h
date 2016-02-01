/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES shared state object.
=============================================================================*/

#ifndef GLXX_SHARED_H
#define GLXX_SHARED_H
#ifdef __cplusplus
extern "C" {
#endif
#include "middleware/khronos/common/khrn_map.h"
#include "interface/khronos/include/GLES/gl.h"

typedef struct {
   uint32_t next_pobject;
   uint32_t next_texture;
   uint32_t next_buffer;
   uint32_t next_renderbuffer;
   uint32_t next_framebuffer;

   KHRN_MAP_T pobjects;

   /*
      Map of texture identifier to texture object

      Khronos state variable names:

      -

      Invariant:

      textures is a valid map and the elements are valid TEXTURE_Ts
   */

   KHRN_MAP_T textures;

   /*
      Map of buffer identifier to buffer object

      Khronos state variable names:

      -

      Invariant:

      buffers is a valid map and the elements are valid BUFFER_Ts
   */

   KHRN_MAP_T buffers;
   KHRN_MAP_T renderbuffers;
   KHRN_MAP_T framebuffers;
} GLXX_SHARED_T;

extern bool glxx_shared_init(GLXX_SHARED_T *shared);
extern void glxx_shared_term(void *v, uint32_t size);

extern uint32_t glxx_shared_create_program(GLXX_SHARED_T *shared);
extern uint32_t glxx_shared_create_shader(GLXX_SHARED_T *shared, uint32_t type);

extern MEM_HANDLE_T glxx_shared_get_pobject(GLXX_SHARED_T *shared, uint32_t pobject);
extern MEM_HANDLE_T glxx_shared_get_buffer(GLXX_SHARED_T *shared, uint32_t buffer, bool create);
extern MEM_HANDLE_T glxx_shared_get_texture(GLXX_SHARED_T *shared, uint32_t texture);
extern MEM_HANDLE_T glxx_shared_get_or_create_texture(GLXX_SHARED_T *shared, uint32_t texture, GLenum target, GLenum *error, bool *has_color, bool *has_alpha, bool *complete);
extern MEM_HANDLE_T glxx_shared_get_renderbuffer(GLXX_SHARED_T *shared, uint32_t renderbuffer, bool create);
extern MEM_HANDLE_T glxx_shared_get_framebuffer(GLXX_SHARED_T *shared, uint32_t framebuffer, bool create);

extern void glxx_shared_delete_pobject(GLXX_SHARED_T *shared, uint32_t pobject);
extern void glxx_shared_delete_buffer(GLXX_SHARED_T *shared, uint32_t buffer);
extern void glxx_shared_delete_texture(GLXX_SHARED_T *shared, uint32_t texture);
extern void glxx_shared_delete_renderbuffer(GLXX_SHARED_T *shared, uint32_t renderbuffer);
extern void glxx_shared_delete_framebuffer(GLXX_SHARED_T *shared, uint32_t framebuffer);
#ifdef __cplusplus
 }
#endif
#endif
