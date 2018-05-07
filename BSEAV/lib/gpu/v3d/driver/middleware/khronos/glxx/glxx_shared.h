/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/common/khrn_map.h"
#include <GLES/gl.h>
#include "middleware/khronos/glxx/glxx_buffer.h"
#include "middleware/khronos/glxx/glxx_texture.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"

typedef struct {
   uint32_t next_pobject;
   uint32_t next_texture;
   uint32_t next_buffer;
   uint32_t next_renderbuffer;
   uint32_t next_framebuffer;

   khrn_map pobjects;

   /*
      Map of texture identifier to texture object

      Khronos state variable names:

      -

      Invariant:

      textures is a valid map and the elements are valid TEXTURE_Ts
   */

   khrn_map textures;

   /*
      Map of buffer identifier to buffer object

      Khronos state variable names:

      -

      Invariant:

      buffers is a valid map and the elements are valid BUFFER_Ts
   */

   khrn_map buffers;
   khrn_map renderbuffers;
   khrn_map framebuffers;
} GLXX_SHARED_T;

extern bool glxx_shared_init(GLXX_SHARED_T *shared);
extern void glxx_shared_term(void *p);

extern uint32_t glxx_shared_create_program(GLXX_SHARED_T *shared);
extern uint32_t glxx_shared_create_shader(GLXX_SHARED_T *shared, uint32_t type);

extern void *glxx_shared_get_pobject(GLXX_SHARED_T *shared, uint32_t pobject);
extern GLXX_BUFFER_T *glxx_shared_create_buffer(uint32_t buffer);
extern GLXX_BUFFER_T *glxx_shared_get_buffer(GLXX_SHARED_T *shared, uint32_t buffer, bool create);
extern GLXX_TEXTURE_T *glxx_shared_get_texture(GLXX_SHARED_T *shared, uint32_t texture);
extern GLXX_TEXTURE_T *glxx_shared_get_or_create_texture(GLXX_SHARED_T *shared, uint32_t texture, GLenum target, GLenum *error, bool *has_color, bool *has_alpha, bool *complete);
extern GLXX_RENDERBUFFER_T *glxx_shared_get_renderbuffer(GLXX_SHARED_T *shared, uint32_t renderbuffer, bool create);
extern GLXX_FRAMEBUFFER_T *glxx_shared_get_framebuffer(GLXX_SHARED_T *shared, uint32_t framebuffer, bool create);

extern void glxx_shared_delete_pobject(GLXX_SHARED_T *shared, uint32_t pobject);
extern void glxx_shared_delete_buffer(GLXX_SHARED_T *shared, uint32_t buffer);
extern void glxx_shared_delete_texture(GLXX_SHARED_T *shared, uint32_t texture);
extern void glxx_shared_delete_renderbuffer(GLXX_SHARED_T *shared, uint32_t renderbuffer);
extern void glxx_shared_delete_framebuffer(GLXX_SHARED_T *shared, uint32_t framebuffer);
