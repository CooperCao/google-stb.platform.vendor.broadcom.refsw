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
#include "middleware/khronos/common/khrn_mem.h"

GLXX_BUFFER_T *glxx_shared_create_buffer(uint32_t buffer)
{
   GLXX_BUFFER_T *pbuffer = KHRN_MEM_ALLOC_STRUCT(GLXX_BUFFER_T);                  // check, glxx_buffer_term
   if (pbuffer != NULL) {
      khrn_mem_set_term(pbuffer, glxx_buffer_term);
      glxx_buffer_init(pbuffer, buffer);
   }
   return pbuffer;
}

GLXX_BUFFER_T *glxx_shared_get_buffer(GLXX_SHARED_T *shared, uint32_t buffer, bool create)
{
   GLXX_BUFFER_T *pbuffer = khrn_map_lookup(&shared->buffers, buffer);

   if (create && pbuffer == NULL) {

      assert(buffer);

      pbuffer = glxx_shared_create_buffer(buffer);
      if (pbuffer != NULL) {
         if (khrn_map_insert(&shared->buffers, buffer, pbuffer))
            khrn_mem_release(pbuffer);
         else
            KHRN_MEM_ASSIGN(pbuffer, NULL);
      }
   }

   return pbuffer;
}

void glxx_shared_delete_buffer(GLXX_SHARED_T *shared, uint32_t buffer)
{
   assert(buffer != 0);
   khrn_map_delete(&shared->buffers, buffer);
}

GLXX_TEXTURE_T *glxx_shared_get_texture(GLXX_SHARED_T *shared, uint32_t texture)
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

void glxx_shared_term(void *p)
{
   GLXX_SHARED_T *shared = p;
   khrn_map_term(&shared->pobjects);
   khrn_map_term(&shared->textures);
   khrn_map_term(&shared->buffers);
   khrn_map_term(&shared->renderbuffers);
   khrn_map_term(&shared->framebuffers);
}

uint32_t glxx_shared_create_program(GLXX_SHARED_T *shared)
{
   uint32_t result = 0;

   GL20_PROGRAM_T *program = KHRN_MEM_ALLOC_STRUCT(GL20_PROGRAM_T);     // check, gl20_program_term

   if (program != NULL) {
      khrn_mem_set_term(program, gl20_program_term);

      gl20_program_init(program, shared->next_pobject);

      if (khrn_map_insert(&shared->pobjects, shared->next_pobject, program))
         result = shared->next_pobject++;

      KHRN_MEM_ASSIGN(program, NULL);
   }

   return result;
}

uint32_t glxx_shared_create_shader(GLXX_SHARED_T *shared, uint32_t type)
{
   uint32_t result = 0;

   GL20_SHADER_T *shader = KHRN_MEM_ALLOC_STRUCT(GL20_SHADER_T);   // check, gl20_shader_term

   if (shader != NULL) {
      khrn_mem_set_term(shader, gl20_shader_term);

      gl20_shader_init(shader, shared->next_pobject, type);

      if (khrn_map_insert(&shared->pobjects, shared->next_pobject, shader))
         result = shared->next_pobject++;

      KHRN_MEM_ASSIGN(shader, NULL);
   }

   return result;
}

void *glxx_shared_get_pobject(GLXX_SHARED_T *shared, uint32_t pobject)
{
   return khrn_map_lookup(&shared->pobjects, pobject);
}

GLXX_RENDERBUFFER_T *glxx_shared_get_renderbuffer(GLXX_SHARED_T *shared, uint32_t renderbuffer, bool create)
{
   GLXX_RENDERBUFFER_T *glxx_renderbuffer = khrn_map_lookup(&shared->renderbuffers, renderbuffer);

   if (create && glxx_renderbuffer == NULL) {
      glxx_renderbuffer = KHRN_MEM_ALLOC_STRUCT(GLXX_RENDERBUFFER_T);       // check, glxx_renderbuffer_term

      if (glxx_renderbuffer != NULL) {
         khrn_mem_set_term(glxx_renderbuffer, glxx_renderbuffer_term);

         glxx_renderbuffer_init(glxx_renderbuffer, renderbuffer);

         if (khrn_map_insert(&shared->renderbuffers, renderbuffer, glxx_renderbuffer))
            khrn_mem_release(glxx_renderbuffer);
         else
            KHRN_MEM_ASSIGN(glxx_renderbuffer, NULL);
      }
   }

   return glxx_renderbuffer;
}

GLXX_FRAMEBUFFER_T *glxx_shared_get_framebuffer(GLXX_SHARED_T *shared, uint32_t framebuffer, bool create)
{
   GLXX_FRAMEBUFFER_T *glxx_framebuffer = khrn_map_lookup(&shared->framebuffers, framebuffer);

   if (create && glxx_framebuffer == NULL) {
      glxx_framebuffer = KHRN_MEM_ALLOC_STRUCT(GLXX_FRAMEBUFFER_T);        // check, glxx_framebuffer_term

      if (glxx_framebuffer != NULL) {
         khrn_mem_set_term(glxx_framebuffer, glxx_framebuffer_term);

         glxx_framebuffer_init(glxx_framebuffer, framebuffer);

         if (khrn_map_insert(&shared->framebuffers, framebuffer, glxx_framebuffer))
            khrn_mem_release(glxx_framebuffer);
         else
            KHRN_MEM_ASSIGN(glxx_framebuffer, NULL);
      }
   }

   return glxx_framebuffer;
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

GLXX_TEXTURE_T *glxx_shared_get_or_create_texture(GLXX_SHARED_T *shared, uint32_t texture, GLenum target, GLenum *error, bool *has_color, bool *has_alpha, bool *complete)
{
   GLXX_TEXTURE_T *glxx_texture = khrn_map_lookup(&shared->textures, texture);

   assert(texture);
   if (glxx_texture == NULL) {
      glxx_texture = KHRN_MEM_ALLOC_STRUCT(GLXX_TEXTURE_T);                 // check, glxx_texture_term

      if (glxx_texture != NULL) {
         khrn_mem_set_term(glxx_texture, glxx_texture_term);

         glxx_texture_init(glxx_texture, texture, target);

         if (khrn_map_insert(&shared->textures, texture, glxx_texture))
            khrn_mem_release(glxx_texture);
         else
            KHRN_MEM_ASSIGN(glxx_texture, NULL);
      }
      else
         *error = GL_OUT_OF_MEMORY;

      *complete = false;
   } else {
      bool fail = glxx_texture->target != target;
      glxx_texture_has_color_alpha(glxx_texture, has_color, has_alpha, complete);

      if (fail) {
         *error = GL_INVALID_OPERATION;
         glxx_texture = NULL;
      }
   }

   return glxx_texture;
}
