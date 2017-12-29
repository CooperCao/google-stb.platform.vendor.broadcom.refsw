/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/glxx/glxx_buffer.h"
#include "middleware/khronos/glxx/glxx_texture.h"
#include "middleware/khronos/glxx/glxx_shared.h"
#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/gl20/gl20_shader.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"

MEM_HANDLE_T glxx_shared_get_buffer(GLXX_SHARED_T *shared, uint32_t buffer, bool create)
{
   MEM_HANDLE_T handle = khrn_map_lookup(&shared->buffers, buffer);

   if (create && handle == MEM_HANDLE_INVALID) {
      handle = MEM_ALLOC_STRUCT_EX(GLXX_BUFFER_T, MEM_COMPACT_DISCARD);                  // check, glxx_buffer_term

      assert(buffer);

      if (handle != MEM_HANDLE_INVALID) {
         mem_set_term(handle, glxx_buffer_term, NULL);

         glxx_buffer_init((GLXX_BUFFER_T *)mem_lock(handle, NULL), buffer);
         mem_unlock(handle);

         if (khrn_map_insert(&shared->buffers, buffer, handle))
            mem_release(handle);
         else {
            mem_release(handle);
            handle = MEM_HANDLE_INVALID;
         }
      }
   }

   return handle;
}

void glxx_shared_delete_buffer(GLXX_SHARED_T *shared, uint32_t buffer)
{
   assert(buffer != 0);
   khrn_map_delete(&shared->buffers, buffer);
}

MEM_HANDLE_T glxx_shared_get_texture(GLXX_SHARED_T *shared, uint32_t texture)
{
   return khrn_map_lookup(&shared->textures, texture);
}

void glxx_shared_delete_texture(GLXX_SHARED_T *shared, uint32_t texture)
{
   assert(texture != 0);
   khrn_map_delete(&shared->textures, texture);
}

bool glxx_shared_init(GLXX_SHARED_T *shared)
{
   shared->next_pobject = 1;
   shared->next_texture = 1;
   shared->next_buffer = 1;
   shared->next_renderbuffer = 1;
   shared->next_framebuffer = 1;

   if (!khrn_map_init(&shared->pobjects, 256))
      return false;
   if (!khrn_map_init(&shared->textures, 256))
      return false;
   if (!khrn_map_init(&shared->buffers, 256))
      return false;
   if (!khrn_map_init(&shared->renderbuffers, 256))
      return false;
   if (!khrn_map_init(&shared->framebuffers, 256))
      return false;

   return true;
}

void glxx_shared_term(MEM_HANDLE_T handle)
{
   GLXX_SHARED_T *shared = (GLXX_SHARED_T *)mem_lock(handle, NULL);

   khrn_map_term(&shared->pobjects);
   khrn_map_term(&shared->textures);
   khrn_map_term(&shared->buffers);
   khrn_map_term(&shared->renderbuffers);
   khrn_map_term(&shared->framebuffers);

   mem_unlock(handle);
}

uint32_t glxx_shared_create_program(GLXX_SHARED_T *shared)
{
   uint32_t result = 0;

   MEM_HANDLE_T handle = MEM_ALLOC_STRUCT_EX(GL20_PROGRAM_T, MEM_COMPACT_DISCARD);     // check, gl20_program_term

   if (handle != MEM_HANDLE_INVALID) {
      mem_set_term(handle, gl20_program_term, NULL);

      gl20_program_init((GL20_PROGRAM_T *)mem_lock(handle, NULL), shared->next_pobject);
      mem_unlock(handle);

      if (khrn_map_insert(&shared->pobjects, shared->next_pobject, handle))
         result = shared->next_pobject++;

      mem_release(handle);
   }

   return result;
}

uint32_t glxx_shared_create_shader(GLXX_SHARED_T *shared, uint32_t type)
{
   uint32_t result = 0;

   MEM_HANDLE_T handle = MEM_ALLOC_STRUCT_EX(GL20_SHADER_T, MEM_COMPACT_DISCARD);   // check, gl20_shader_term

   if (handle != MEM_HANDLE_INVALID) {
      mem_set_term(handle, gl20_shader_term, NULL);

      gl20_shader_init((GL20_SHADER_T *)mem_lock(handle, NULL), shared->next_pobject, type);
      mem_unlock(handle);

      if (khrn_map_insert(&shared->pobjects, shared->next_pobject, handle))
         result = shared->next_pobject++;

      mem_release(handle);
   }

   return result;
}

MEM_HANDLE_T glxx_shared_get_pobject(GLXX_SHARED_T *shared, uint32_t pobject)
{
   return khrn_map_lookup(&shared->pobjects, pobject);
}

MEM_HANDLE_T glxx_shared_get_renderbuffer(GLXX_SHARED_T *shared, uint32_t renderbuffer, bool create)
{
   MEM_HANDLE_T handle = khrn_map_lookup(&shared->renderbuffers, renderbuffer);

   if (create && handle == MEM_HANDLE_INVALID) {
      handle = MEM_ALLOC_STRUCT_EX(GLXX_RENDERBUFFER_T, MEM_COMPACT_DISCARD);       // check, glxx_renderbuffer_term

      if (handle != MEM_HANDLE_INVALID) {
         mem_set_term(handle, glxx_renderbuffer_term, NULL);

         glxx_renderbuffer_init((GLXX_RENDERBUFFER_T *)mem_lock(handle, NULL), renderbuffer);
         mem_unlock(handle);

         if (khrn_map_insert(&shared->renderbuffers, renderbuffer, handle))
            mem_release(handle);
         else {
            mem_release(handle);
            handle = MEM_HANDLE_INVALID;
         }
      }
   }

   return handle;
}

MEM_HANDLE_T glxx_shared_get_framebuffer(GLXX_SHARED_T *shared, uint32_t framebuffer, bool create)
{
   MEM_HANDLE_T handle = khrn_map_lookup(&shared->framebuffers, framebuffer);

   if (create && handle == MEM_HANDLE_INVALID) {
      handle = MEM_ALLOC_STRUCT_EX(GLXX_FRAMEBUFFER_T, MEM_COMPACT_DISCARD);        // check, glxx_framebuffer_term

      if (handle != MEM_HANDLE_INVALID) {
         mem_set_term(handle, glxx_framebuffer_term, NULL);

         glxx_framebuffer_init((GLXX_FRAMEBUFFER_T *)mem_lock(handle, NULL), framebuffer);
         mem_unlock(handle);

         if (khrn_map_insert(&shared->framebuffers, framebuffer, handle))
            mem_release(handle);
         else {
            mem_release(handle);
            handle = MEM_HANDLE_INVALID;
         }
      }
   }

   return handle;
}

void glxx_shared_delete_pobject(GLXX_SHARED_T *shared, uint32_t pobject)
{
   khrn_map_delete(&shared->pobjects, pobject);
}

void glxx_shared_delete_renderbuffer(GLXX_SHARED_T *shared, uint32_t renderbuffer)
{
   khrn_map_delete(&shared->renderbuffers, renderbuffer);
}

void glxx_shared_delete_framebuffer(GLXX_SHARED_T *shared, uint32_t framebuffer)
{
   khrn_map_delete(&shared->framebuffers, framebuffer);
}

MEM_HANDLE_T glxx_shared_get_or_create_texture(GLXX_SHARED_T *shared, uint32_t texture, GLenum target, GLenum *error, bool *has_color, bool *has_alpha, bool *complete)
{
   MEM_HANDLE_T handle = khrn_map_lookup(&shared->textures, texture);

   assert(texture);
   if (handle == MEM_HANDLE_INVALID) {
      handle = MEM_ALLOC_STRUCT_EX(GLXX_TEXTURE_T, MEM_COMPACT_DISCARD);                 // check, glxx_texture_term

      if (handle != MEM_HANDLE_INVALID) {
         mem_set_term(handle, glxx_texture_term, NULL);

         glxx_texture_init((GLXX_TEXTURE_T *)mem_lock(handle, NULL), texture, target);
         mem_unlock(handle);

         if (khrn_map_insert(&shared->textures, texture, handle))
            mem_release(handle);
         else {
            mem_release(handle);
            handle = MEM_HANDLE_INVALID;
         }
      }

      *complete = false;

      if (handle == MEM_HANDLE_INVALID)
         *error = GL_OUT_OF_MEMORY;
   } else {
      GLXX_TEXTURE_T *texture = (GLXX_TEXTURE_T *)mem_lock(handle, NULL);
      bool fail = texture->target != target;
      glxx_texture_has_color_alpha(texture, has_color, has_alpha, complete);
      mem_unlock(handle);

      if (fail) {
         *error = GL_INVALID_OPERATION;
         handle = MEM_HANDLE_INVALID;
      }
   }

   return handle;
}