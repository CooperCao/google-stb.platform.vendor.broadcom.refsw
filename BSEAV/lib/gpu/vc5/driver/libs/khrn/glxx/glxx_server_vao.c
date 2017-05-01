/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_shared.h"
#include "glxx_server.h"
#include "vcos.h"

// Vertex array objects support.

// VAOs store all vertex attributes, plus buffer bindings for element arrays (BindBuffer(ELEMENT_ARRAY)) and vertex
// attrib arrays (BindBuffer(ARRAY_BUFFER)).  They don't store ArrayBuffer bindings or (obviously) VertexArray bindings.
// They are numbered from 0, where 0 is the "default" VAO which always exists. They are enabled when bound
// (BindVertexArrays), not when generated (GenVertexArrays).

// This is implemented by keeping a copy of the relevant server state in a separate structure, GLXX_VAO_T.
// Functions which alter server state update this structure, and switching the current structure (by binding a different
// VAO) alter the relevant server state.

static void glxx_vao_term(void *v, size_t size)
{
   GLXX_VAO_T *vao = v;
   vcos_unused(size);

   KHRN_MEM_ASSIGN(vao->element_array_binding.obj, NULL);
   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS; i++)
      KHRN_MEM_ASSIGN(vao->vbos[i].buffer, NULL);

   free(vao->debug_label);
   vao->debug_label = NULL;
}

static void vao_init(GLXX_SERVER_STATE_T *state, GLXX_VAO_T *vao, int32_t name)
{
   vao->name        = name;
   vao->enabled     = false;
   vao->debug_label = NULL;
}

static void vao_enable(GLXX_SERVER_STATE_T *state, GLXX_VAO_T *vao)
{
   assert(vao->enabled == false);

   vao->element_array_binding.buffer = 0;
   vao->element_array_binding.obj = NULL;

   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      GLXX_ATTRIB_CONFIG_T* attrib = &vao->attrib_config[i];
      attrib->gl_type = GL_FLOAT;
      attrib->v3d_type = V3D_ATTR_TYPE_FLOAT;
      attrib->is_signed = false;
      attrib->norm = 0;
      attrib->size = 4;
      attrib->enabled = false;
      attrib->total_size = 16;
      attrib->is_int = false;
      attrib->vbo_index = i;
      attrib->relative_offset = 0;
      attrib->stride = 16;
      attrib->original_stride = 0;
      attrib->pointer = NULL;
   }

   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIB_BINDINGS; i++)
   {
      vao->vbos[i].offset = 0;
      vao->vbos[i].stride = 16;
      vao->vbos[i].divisor = 0;
      vao->vbos[i].buffer = NULL;
   }

   vao->enabled = true;
}

GLXX_VAO_T *glxx_get_vao(GLXX_SERVER_STATE_T *state, uint32_t id)
{
   return khrn_map_lookup(&state->vao.objects, id);
}

static GLXX_VAO_T *glxx_allocate_vao(GLXX_SERVER_STATE_T *state, uint32_t id)
{
   GLXX_VAO_T *vao = glxx_get_vao(state, id);

   if (vao != NULL)
   {
      /* ID in use, couldn't allocate */
      return NULL;
   }

   vao = KHRN_MEM_ALLOC_STRUCT(GLXX_VAO_T);
   if (vao != NULL)
   {
      khrn_mem_set_term(vao, glxx_vao_term);

      vao_init(state, vao, id);

      if (khrn_map_insert(&state->vao.objects, id, vao))
      {
         khrn_mem_release(vao);
      }
      else
      {
         khrn_mem_release(vao);
         vao = NULL;
      }
   }

   return vao;
}

bool glxx_vao_initialise(GLXX_SERVER_STATE_T *state)
{
   GLXX_VAO_T *vao;

   if (!khrn_map_init(&state->vao.objects, 256))
      return false;

   vao = glxx_allocate_vao(state, 0);

   if (vao != NULL)
   {
      vao_enable(state, vao);
      KHRN_MEM_ASSIGN(state->vao.default_vao, vao);
      KHRN_MEM_ASSIGN(state->vao.bound, vao);
      state->vao.next = 1;

      return true;
   }
   return false;
}

void glxx_vao_uninitialise(GLXX_SERVER_STATE_T *state)
{
   KHRN_MEM_ASSIGN(state->vao.default_vao, NULL);
   KHRN_MEM_ASSIGN(state->vao.bound, NULL);
   khrn_map_term(&state->vao.objects);
}

void glintBindVertexArray(GLuint array)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   GLXX_VAO_T *vao = glxx_get_vao(state, array);
   if (vao == NULL)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   if (!vao->enabled)
   {
      // New object.
      vao_enable(state, vao);
   }

   KHRN_MEM_ASSIGN(state->vao.bound, vao);

end:
   glxx_unlock_server_state();
}

static void vao_delete_server_state(GLXX_SERVER_STATE_T *state, GLXX_VAO_T *vao)
{
   assert(vao != NULL);

   KHRN_MEM_ASSIGN(vao->element_array_binding.obj, NULL);
}

void glintDeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   if (n < 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   }
   else
   {
      int i;

      for (i = 0; i < n; i++)
      {
         GLuint name = arrays[i];

         if (name > 0)
         {
            GLXX_VAO_T *vao = glxx_get_vao(state, name);

            assert(vao != state->vao.default_vao);

            if (vao != NULL)
            {
               if (vao == state->vao.bound)
               {
                  KHRN_MEM_ASSIGN(state->vao.bound, state->vao.default_vao);
               }

               vao_delete_server_state(state, vao);
               khrn_map_delete(&state->vao.objects, name);
            }
         }
      }
   }

   glxx_unlock_server_state();
}

void glintGenVertexArrays(GLsizei n, GLuint* arrays)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (n < 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   }
   else
   {
      int i = 0;
      while (i < n)
      {
         GLXX_VAO_T *vao = glxx_allocate_vao(state, state->vao.next);

         if (vao != NULL)
         {
            arrays[i] = state->vao.next;
            i++;
         }

         state->vao.next ++;
      }
   }

   glxx_unlock_server_state();
}

GLboolean glintIsVertexArray(GLuint array)
{
   GLXX_VAO_T *vao;
   GLboolean result = GL_FALSE;
   GLXX_SERVER_STATE_T *state;

   state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return GL_FALSE;

   vao = glxx_get_vao(state, array);
   if (vao != NULL)
   {
      /* 0, being the default VAO returns false as a bizarre exception */
      result = array != 0 && vao->enabled;
   }

   glxx_unlock_server_state();
   return result;
}

// Vertex array objects, public API
//
GL_API void GL_APIENTRY glBindVertexArray(GLuint array)
{
   glintBindVertexArray(array);
}

GL_API void GL_APIENTRY glDeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
   glintDeleteVertexArrays(n, arrays);
}

GL_API void GL_APIENTRY glGenVertexArrays(GLsizei n, GLuint* arrays)
{
   glintGenVertexArrays(n, arrays);
}

GL_API GLboolean GL_APIENTRY glIsVertexArray(GLuint array)
{
   return glintIsVertexArray(array);
}
