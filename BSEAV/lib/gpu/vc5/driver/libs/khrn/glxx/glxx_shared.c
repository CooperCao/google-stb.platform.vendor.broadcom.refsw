/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_int_config.h"

#include "glxx_texture.h"
#include "glxx_shared.h"
#include "glxx_renderbuffer.h"
#include "glxx_framebuffer.h"
#include "../gl20/gl20_program.h"
#include "../gl20/gl20_shader.h"

GLXX_BUFFER_T *glxx_shared_get_buffer(GLXX_SHARED_T *shared, uint32_t buffer)
{
   return  khrn_map_lookup(&shared->buffers, buffer);
}

/*
   Obtains a new buffer object pointer, given a shared state object.
*/
GLXX_BUFFER_T *glxx_shared_allocate_buffer(GLXX_SHARED_T *shared, uint32_t buffer, bool *out_of_memory)
{
   assert(buffer != 0);

   GLXX_BUFFER_T *buffer_obj = khrn_map_lookup(&shared->buffers, buffer);

   if (buffer_obj != NULL)
   {
      // The buffer name is already in use
      *out_of_memory = false;
      return NULL;
   }

   buffer_obj = KHRN_MEM_ALLOC_STRUCT(GLXX_BUFFER_T); // check, glxx_buffer_term

   if (buffer_obj == NULL)
   {
      *out_of_memory = true;
      return NULL;
   }

   khrn_mem_set_term(buffer_obj, glxx_buffer_term);

   glxx_buffer_init(buffer_obj, buffer);

   if (!khrn_map_insert(&shared->buffers, buffer, buffer_obj))
   {
      khrn_mem_release(buffer_obj);
      *out_of_memory = true;
      return NULL;
   }

   khrn_mem_release(buffer_obj);
   *out_of_memory = false;

   return buffer_obj;
}

void glxx_shared_delete_buffer(GLXX_SHARED_T *shared, uint32_t buffer)
{
   assert(buffer != 0);
   khrn_map_delete(&shared->buffers, buffer);
}

/*
   Obtains a texture object, given a shared state object
   Returns either a valid pointer, or NULL.  NULL means the texture did not exist.
*/
GLXX_TEXTURE_T *glxx_shared_get_texture(GLXX_SHARED_T *shared, uint32_t texture)
{
   return khrn_map_lookup(&shared->textures, texture);
}


/*
   Removes the specified texture from the shared object.
*/
void glxx_shared_delete_texture(GLXX_SHARED_T *shared, uint32_t texture)
{
   assert(texture != 0);
   khrn_map_delete(&shared->textures, texture);
}

/* Initialise the shared object pool. */
/* *Please* remember to check shared_term, below, when you add items here */
bool glxx_shared_init(GLXX_SHARED_T *shared)
{
   shared->next_pobject = 1;
   shared->next_texture = 1;
   shared->next_sampler = 1;
   shared->next_buffer = 1;
   shared->next_renderbuffer = 1;
   shared->next_fencesync = 1;

   if (!khrn_map_init(&shared->pobjects, 256))
      return false;
   if (!khrn_map_init(&shared->textures, 256))
      return false;
   if (!khrn_map_init(&shared->samplers, 256))
      return false;
   if (!khrn_map_init(&shared->buffers, 256))
      return false;
   if (!khrn_map_init(&shared->renderbuffers, 256))
      return false;
   if (!khrn_map_init(&shared->fencesyncs, 256))
      return false;

   bool *gpu_aborted = khrn_mem_alloc(sizeof(bool), "gpu aborted flag");
   *gpu_aborted = false;
   KHRN_MEM_ASSIGN(shared->gpu_aborted, gpu_aborted);
   khrn_mem_release(gpu_aborted);

   return true;
}

static void destroy_pobject_callback(khrn_map *map, uint32_t key, void *pobject, void *shared)
{
   assert(pobject != NULL);

   if (gl20_is_program(pobject))
   {
      GL20_PROGRAM_T *program = (GL20_PROGRAM_T *)pobject;
      program->deleted = true;
      gl20_server_try_delete_program((GLXX_SHARED_T *)shared, program);
   }
   else if (gl20_is_shader(pobject))
   {
      GL20_SHADER_T *shader = (GL20_SHADER_T *)pobject;
      shader->deleted = true;
      gl20_server_try_delete_shader((GLXX_SHARED_T *)shared, shader);
   }
   else
   {
      unreachable();
   }
}

void glxx_shared_term(void *v, size_t size)
{
   GLXX_SHARED_T *shared = (GLXX_SHARED_T *)v;

   vcos_unused(size);

   khrn_map_iterate(&shared->pobjects, destroy_pobject_callback, shared);
   khrn_map_term(&shared->pobjects);
   khrn_map_term(&shared->textures);
   khrn_map_term(&shared->samplers);
   khrn_map_term(&shared->buffers);
   khrn_map_term(&shared->renderbuffers);
   khrn_map_term(&shared->fencesyncs);

   KHRN_MEM_ASSIGN(shared->gpu_aborted, NULL);
}

uint32_t glxx_shared_create_program(GLXX_SHARED_T *shared)
{
   uint32_t result = 0;

   GL20_PROGRAM_T *program = KHRN_MEM_ALLOC_STRUCT(GL20_PROGRAM_T);     // check, gl20_program_term

   if (program != NULL)
   {
      khrn_mem_set_term(program, gl20_program_term);

      gl20_program_init(program, shared->next_pobject);

      if (khrn_map_insert(&shared->pobjects, shared->next_pobject, program))
         result = shared->next_pobject++;

      khrn_mem_release(program);
   }

   return result;
}

uint32_t glxx_shared_create_shader(GLXX_SHARED_T *shared, uint32_t type)
{
   uint32_t result = 0;

   GL20_SHADER_T *shader = KHRN_MEM_ALLOC_STRUCT(GL20_SHADER_T);   // check, gl20_shader_term

   if (shader != NULL)
   {
      khrn_mem_set_term(shader, gl20_shader_term);

      gl20_shader_init(shader, shared->next_pobject, type);

      if (khrn_map_insert(&shared->pobjects, shared->next_pobject, shader))
         result = shared->next_pobject++;

      khrn_mem_release(shader);
   }

   return result;
}

void *glxx_shared_get_pobject(GLXX_SHARED_T *shared, uint32_t pobject)
{
   return khrn_map_lookup(&shared->pobjects, pobject);
}


GLXX_RENDERBUFFER_T *glxx_shared_get_renderbuffer(GLXX_SHARED_T *shared, uint32_t name, bool create)
{
   GLXX_RENDERBUFFER_T *rb_obj = khrn_map_lookup(&shared->renderbuffers, name);

   if (create && rb_obj == NULL)
   {
      rb_obj = glxx_renderbuffer_create(name);

      if (rb_obj != NULL)
      {
         bool ok;
         ok = khrn_map_insert(&shared->renderbuffers, name, rb_obj);

         khrn_mem_release(rb_obj);
         if (!ok)
            rb_obj = NULL;
      }
   }

   return rb_obj;
}

void glxx_shared_delete_pobject(GLXX_SHARED_T *shared, uint32_t pobject)
{
   khrn_map_delete(&shared->pobjects, pobject);
}

void glxx_shared_delete_renderbuffer(GLXX_SHARED_T *shared, uint32_t renderbuffer)
{
   khrn_map_delete(&shared->renderbuffers, renderbuffer);
}

/*
   Obtains a texture object, given a shared state object, creating a new one
   should it not already exist.
   The target parameter controls what sort of texture is created. It also
   generates an error if there is an existing texture with that name of the
   wrong type.

   Returns either a valid GLXX_TEXTURE_T pointer, or NULL.  NULL indicates an error.
   These errors are distinguished by the error parameter:
      GL_OUT_OF_MEMORY      Texture object does not exist, and insufficient memory to create a new one
      GL_INVALID_OPERATION  Texture object does exist but is of the wrong type
*/

GLXX_TEXTURE_T *glxx_shared_get_or_create_texture(GLXX_SHARED_T *shared,
      uint32_t texture, GLenum target, GLenum *error)
{
   GLXX_TEXTURE_T *texture_obj = khrn_map_lookup(&shared->textures, texture);

   assert(texture);

   if (texture_obj == NULL)
   {
      bool ok = false;
      texture_obj = glxx_texture_create(target, texture);
      if (texture_obj)
         ok = khrn_map_insert(&shared->textures, texture, texture_obj);
      khrn_mem_release(texture_obj);
      if (!ok)
      {
         texture_obj = NULL;
         *error = GL_OUT_OF_MEMORY;
      }
   }
   else
   {
      if (texture_obj->target != target)
      {
         *error = GL_INVALID_OPERATION;
         texture_obj = NULL;
      }
   }

   return texture_obj;
}

GLsync glxx_shared_create_fencesync(GLXX_SHARED_T *shared, const khrn_fence *fence)
{
   GLXX_FENCESYNC_T *fsync = NULL;
   uint32_t id = 0;

   fsync = glxx_fencesync_create(shared->next_fencesync, fence);
   if (!fsync)
      return 0;

   if (khrn_map_insert(&shared->fencesyncs, shared->next_fencesync, fsync))
   {
      id = shared->next_fencesync;
      shared->next_fencesync++;
   }
   khrn_mem_release(fsync);
   return (GLsync)(uintptr_t)id;
}

GLXX_FENCESYNC_T* glxx_shared_get_fencesync(GLXX_SHARED_T *shared, GLsync fsync_id)
{
   if ((uintptr_t)fsync_id > UINT32_MAX)
      return NULL;
   return (GLXX_FENCESYNC_T*) khrn_map_lookup(&shared->fencesyncs, (uint32_t)(uintptr_t)fsync_id);
}

bool glxx_shared_delete_fencesync(GLXX_SHARED_T *shared, GLsync fsync_id)
{
   if ((uintptr_t)fsync_id > UINT32_MAX)
      return false;
   return khrn_map_delete(&shared->fencesyncs, (uint32_t)(uintptr_t)fsync_id);
}

GLXX_TEXTURE_SAMPLER_STATE_T *glxx_shared_get_sampler(GLXX_SHARED_T *shared, uint32_t sampler)
{
   return khrn_map_lookup(&shared->samplers, sampler);
}

void glxx_shared_delete_sampler(GLXX_SHARED_T *shared, uint32_t sampler)
{
   khrn_map_delete(&shared->samplers, sampler);
}

bool glxx_shared_add_sampler(GLXX_SHARED_T *shared, GLXX_TEXTURE_SAMPLER_STATE_T *sampler)
{
   return khrn_map_insert(&shared->samplers, sampler->id, sampler);
}
