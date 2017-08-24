/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_SHARED_H
#define GLXX_SHARED_H
#ifdef __cplusplus
extern "C" {
#endif
#include "../common/khrn_resource.h"
#include "../common/khrn_map.h"
#include "glxx_texture.h"
#include "gl_public_api.h"
#include "glxx_fencesync.h"
#include "glxx_compute.h"

typedef struct {
   uint32_t next_pobject;
   uint32_t next_texture;
   uint32_t next_sampler;
   uint32_t next_buffer;
   uint32_t next_renderbuffer;
   uint32_t next_fencesync;

   khrn_map pobjects;

   /*
      Map of texture identifier to texture object
   */
   khrn_map textures;
   khrn_map samplers;

   /*
      Map of buffer identifier to buffer object
   */
   khrn_map buffers;

   khrn_map renderbuffers;
   khrn_map fencesyncs;

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

extern GLsync glxx_shared_create_fencesync(GLXX_SHARED_T *shared, const khrn_fence *kfence);
extern GLXX_FENCESYNC_T* glxx_shared_get_fencesync(GLXX_SHARED_T *shared, GLsync fencesync_id);

/* returns false if there is no object with id = fsync_id in the map;
 * returns true otherwise */
extern bool glxx_shared_delete_fencesync(GLXX_SHARED_T *shared, GLsync fsync_id);

#ifdef __cplusplus
}
#endif
#endif
