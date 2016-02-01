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
#include "middleware/khronos/common/khrn_interlock.h"
#include "middleware/khronos/common/khrn_map.h"
#include "middleware/khronos/glxx/glxx_texture.h"
#include "interface/khronos/glxx/gl_public_api.h"
#include "middleware/khronos/glxx/glxx_fencesync.h"

typedef struct {
   uint32_t next_pobject;
   uint32_t next_texture;
   uint32_t next_sampler;
   uint32_t next_buffer;
   uint32_t next_renderbuffer;
   uint32_t next_fencesync;

   KHRN_MAP_T pobjects;

   /*
      Map of texture identifier to texture object
   */
   KHRN_MAP_T textures;
   KHRN_MAP_T samplers;

   /*
      Map of buffer identifier to buffer object
   */
   KHRN_MAP_T buffers;

   KHRN_MAP_T renderbuffers;
   KHRN_MAP_T fencesyncs;

} GLXX_SHARED_T;

// TODO: Just include the headers?
struct GLXX_BUFFER_;
struct GLXX_RENDERBUFFER_T_;

extern bool glxx_shared_init(GLXX_SHARED_T *shared);
extern void glxx_shared_term(void *v, size_t size);

extern uint32_t glxx_shared_create_program(GLXX_SHARED_T *shared);
extern uint32_t glxx_shared_create_shader(GLXX_SHARED_T *shared, uint32_t type);

extern void *glxx_shared_get_pobject(GLXX_SHARED_T *shared, uint32_t pobject);
extern struct GLXX_BUFFER_ *glxx_shared_get_buffer(GLXX_SHARED_T *shared, uint32_t buffer);
extern struct GLXX_BUFFER_ *glxx_shared_allocate_buffer(GLXX_SHARED_T *shared, uint32_t buffer, bool *out_of_memory);
extern GLXX_TEXTURE_T *glxx_shared_get_texture(GLXX_SHARED_T *shared, uint32_t texture);
extern GLXX_TEXTURE_T *glxx_shared_get_or_create_texture(GLXX_SHARED_T *shared, uint32_t texture, GLenum target, GLenum *error);
extern struct GLXX_RENDERBUFFER_ *glxx_shared_get_renderbuffer(GLXX_SHARED_T *shared, uint32_t renderbuffer, bool create);

extern GLXX_TEXTURE_SAMPLER_STATE_T *glxx_shared_get_sampler(GLXX_SHARED_T *shared, uint32_t sampler);
extern bool glxx_shared_add_sampler(GLXX_SHARED_T *shared, GLXX_TEXTURE_SAMPLER_STATE_T *sampler);

extern void glxx_shared_delete_pobject(GLXX_SHARED_T *shared, uint32_t pobject);
extern void glxx_shared_delete_buffer(GLXX_SHARED_T *shared, uint32_t buffer);
extern void glxx_shared_delete_texture(GLXX_SHARED_T *shared, uint32_t texture);
extern void glxx_shared_delete_sampler(GLXX_SHARED_T *shared, uint32_t sampler);
extern void glxx_shared_delete_renderbuffer(GLXX_SHARED_T *shared, uint32_t renderbuffer);

extern GLsync glxx_shared_create_fencesync(GLXX_SHARED_T *shared, const KHRN_FENCE_T *kfence);
extern GLXX_FENCESYNC_T* glxx_shared_get_fencesync(GLXX_SHARED_T *shared, GLsync fencesync_id);

/* returns false if there is no object with id = fsync_id in the map;
 * returns true otherwise */
extern bool glxx_shared_delete_fencesync(GLXX_SHARED_T *shared, GLsync fsync_id);

#ifdef __cplusplus
}
#endif
#endif
