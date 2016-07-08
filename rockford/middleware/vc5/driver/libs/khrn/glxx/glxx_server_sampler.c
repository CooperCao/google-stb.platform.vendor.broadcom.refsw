/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glxx
Module   :

FILE DESCRIPTION
=============================================================================*/

/* Sampler objects.
 *
 * GL textures contain image parameters and sampling state. Sampler objects duplicate
 * the sampling state portion. When bound to a texture unit, sampler objects' state
 * overrides texture state for the texture bound to that TU.
 *
 * One SO can be bound to multiple TUs.
*/
#include "glxx_texture.h"
#include "glxx_shared.h"
#include "glxx_server.h"
#include "glxx_server_texture.h"
#include "glxx_server_internal.h"


static void glxx_sampler_init(GLXX_SERVER_STATE_T *state, GLXX_TEXTURE_SAMPLER_STATE_T *so, uint32_t id)
{
   so->id           = id;
   so->filter.min   = GL_NEAREST_MIPMAP_LINEAR;
   so->filter.mag   = GL_LINEAR;
   so->wrap.s       = GL_REPEAT;
   so->wrap.t       = GL_REPEAT;
   so->wrap.r       = GL_REPEAT;
   so->min_lod      = -1000.0f;
   so->max_lod      = 1000.0f;
   so->compare_mode = GL_NONE;
   so->compare_func = GL_LEQUAL;
   so->debug_label  = NULL;
}

static void glxx_sampler_term(void *v, size_t size)
{
   GLXX_TEXTURE_SAMPLER_STATE_T *so = (GLXX_TEXTURE_SAMPLER_STATE_T*)v;

   free(so->debug_label);
   so->debug_label = NULL;

   UNUSED(size);
}

static bool glxx_sampler_allocate(GLXX_SERVER_STATE_T *state, uint32_t id)
{
   GLXX_TEXTURE_SAMPLER_STATE_T *sampler = KHRN_MEM_ALLOC_STRUCT(GLXX_TEXTURE_SAMPLER_STATE_T);

   if (sampler == NULL)
      return false;

   khrn_mem_set_term(sampler, glxx_sampler_term);
   glxx_sampler_init(state, sampler, id);

   bool ok = glxx_shared_add_sampler(state->shared, sampler);
   khrn_mem_release(sampler);

   return ok;
}

GL_API void GL_APIENTRY glGenSamplers (GLsizei count, GLuint* samplers)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state) return;

   if (count < 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto out;
   }

   int i = 0;
   while (i < count)
   {
      if (glxx_shared_get_sampler(state->shared, state->shared->next_sampler) == NULL)
      {
         if (!glxx_sampler_allocate(state, state->shared->next_sampler))
         {
            glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
            goto out;
         }

         samplers[i] = state->shared->next_sampler;
         i++;
      }
      state->shared->next_sampler++;
   }

out:
   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glDeleteSamplers (GLsizei count, const GLuint* samplers)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state)
      return;

   if (count < 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto out;
   }

   {
      int i;

      for (i = 0; i < count; i++)
      {
         GLuint name = samplers[i];

         if (name > 0)
         {
            GLXX_TEXTURE_SAMPLER_STATE_T *so = glxx_shared_get_sampler(state->shared, name);
            if (so != NULL)
            {
               int j;

               /* If the doomed SO is bound to any TU, unbind it. */
               for (j = 0; j < GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS; j++)
               {
                  if (state->bound_sampler[j] == so)
                     KHRN_MEM_ASSIGN(state->bound_sampler[j], NULL);
               }

               glxx_shared_delete_sampler(state->shared, name);
            }
         }
      }
   }

out:
   GL30_UNLOCK_SERVER_STATE();
}

GL_API GLboolean GL_APIENTRY glIsSampler (GLuint sampler)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   GLboolean result = GL_FALSE;
   if (!state)
      return result;

   if (glxx_shared_get_sampler(state->shared, sampler) != NULL)
      result = GL_TRUE;

   GL30_UNLOCK_SERVER_STATE();
   return result;
}

GL_API void GL_APIENTRY glBindSampler (GLuint unit, GLuint sampler)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state)
      return;

   if (unit >= GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto out;
   }

   if (sampler == 0)
   {
      // Unbind.
      KHRN_MEM_ASSIGN(state->bound_sampler[unit], NULL);
   }
   else
   {
      GLXX_TEXTURE_SAMPLER_STATE_T *so = glxx_shared_get_sampler(state->shared, sampler);
      if (so == NULL)
      {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      }
      else
      {
         KHRN_MEM_ASSIGN(state->bound_sampler[unit], so);
      }
   }

out:
   GL30_UNLOCK_SERVER_STATE();
}

static bool glxx_sampler_getset_preamble(GLXX_SERVER_STATE_T *state, GLuint sampler, GLXX_TEXTURE_SAMPLER_STATE_T **so)
{
   *so = glxx_shared_get_sampler(state->shared, sampler);

   if (*so == NULL) {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return false;
   }

   return true;
}

GL_API void GL_APIENTRY glSamplerParameteri (GLuint sampler, GLenum pname, GLint param)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   GLXX_TEXTURE_SAMPLER_STATE_T *so;
   if (!state) return;

   if (glxx_sampler_getset_preamble(state, sampler, &so))
   {
      if (glxx_is_float_texparam(pname))
      {
         float fparam = (float)param;
         glxx_texparameterf_sampler_internal(state, so, pname, &fparam);
      }
      else
         glxx_texparameter_sampler_internal(state, 0, so, pname, &param);
   }

   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glSamplerParameteriv (GLuint sampler, GLenum pname, const GLint* param)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   GLXX_TEXTURE_SAMPLER_STATE_T *so;
   if (!state) return;

   if (glxx_sampler_getset_preamble(state, sampler, &so))
   {
      if (glxx_is_float_texparam(pname)) {
         float fparam = (float)param[0];
         glxx_texparameterf_sampler_internal(state, so, pname, &fparam);
      }
      else
         glxx_texparameter_sampler_internal(state, 0, so, pname, param);
   }

   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glSamplerParameterf (GLuint sampler, GLenum pname, GLfloat param)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   GLXX_TEXTURE_SAMPLER_STATE_T *so;
   if (!state) return;

   if (glxx_sampler_getset_preamble(state, sampler, &so))
   {
      if (glxx_is_float_texparam(pname))
         glxx_texparameterf_sampler_internal(state, so, pname, &param);
      else
      {
         int iparam = glxx_texparam_float_to_int(param);
         glxx_texparameter_sampler_internal(state, 0, so, pname, &iparam);
      }
   }

   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glSamplerParameterfv (GLuint sampler, GLenum pname, const GLfloat* param)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   GLXX_TEXTURE_SAMPLER_STATE_T *so;
   if (!state) return;

   if (glxx_sampler_getset_preamble(state, sampler, &so))
   {
      if (glxx_is_float_texparam(pname))
         glxx_texparameterf_sampler_internal(state, so, pname, param);
      else
      {
         // TODO: No format conversion here (ref glTexParameterfv.) Why not?
         GLint iparam = (GLint)param[0];
         glxx_texparameter_sampler_internal(state, 0, so, pname, &iparam);
      }
   }

   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glGetSamplerParameteriv (GLuint sampler, GLenum pname, GLint* params)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   GLXX_TEXTURE_SAMPLER_STATE_T *so;
   if (!state) return;

   if (glxx_sampler_getset_preamble(state, sampler, &so))
   {
      if (glxx_is_float_texparam(pname)) {
         GLfloat temp;
         uint32_t count;
         count = glxx_get_texparameterf_sampler_internal(state, so, pname, &temp);
         if (count)
         {
            assert(count == 1);
            params[0] = float_to_int(temp);
         }
      }
      else if (glxx_is_int_sampler_texparam(state, pname))
      {
         glxx_get_texparameter_sampler_internal(state, so, pname, params);
      }
      else
      {
          glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }

   }

   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glGetSamplerParameterfv (GLuint sampler, GLenum pname, GLfloat* params)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   GLXX_TEXTURE_SAMPLER_STATE_T *so;
   if (!state) return;

   if (glxx_sampler_getset_preamble(state, sampler, &so))
   {
      if (glxx_is_float_texparam(pname))
      {
         glxx_get_texparameterf_sampler_internal(state, so, pname, params);
      }
      else if(glxx_is_int_sampler_texparam(state, pname))
      {
         GLint temp;
         uint32_t count;
         count = glxx_get_texparameter_sampler_internal(state, so, pname, &temp);

         if (count)
         {
            assert(count == 1);
            params[0] = (GLfloat)temp;
         }
      }
      else
      {
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
   }

   GL30_UNLOCK_SERVER_STATE();
}
