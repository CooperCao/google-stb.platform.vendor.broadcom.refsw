/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "gl_public_api.h"
#include "../common/khrn_render_state.h"
#include "glxx_server.h"
#include "glxx_compute.h"
#include "libs/util/profile/profile.h"

typedef struct
{
   GLXX_SERVER_STATE_T* state;
   GLbitfield barriers;
} memory_barrier_by_region_param;

static bool memory_barrier(glxx_hw_render_state* rs, void* param)
{
   GLXX_SERVER_STATE_T* state = (GLXX_SERVER_STATE_T*)param;
   if (rs->base.has_buffer_writes && rs->server_state == state)
      glxx_hw_render_state_flush(rs);

   return true;
}

GL_APICALL void GL_APIENTRY glMemoryBarrier(GLbitfield barriers)
{
   PROFILE_FUNCTION_MT("GL");

   /* Excluding: GL_PIXEL_BUFFER_BARRIER_BIT
                 GL_TEXTURE_UPDATE_BARRIER_BIT
                 GL_BUFFER_UPDATE_BARRIER_BIT */
   GLbitfield mask = 0
      | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT
      | GL_ELEMENT_ARRAY_BARRIER_BIT
      | GL_UNIFORM_BARRIER_BIT
      | GL_TEXTURE_FETCH_BARRIER_BIT
      | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
      | GL_COMMAND_BARRIER_BIT
      | GL_FRAMEBUFFER_BARRIER_BIT
      | GL_TRANSFORM_FEEDBACK_BARRIER_BIT
      | GL_ATOMIC_COUNTER_BARRIER_BIT
      | GL_SHADER_STORAGE_BARRIER_BIT;
   if ((barriers & mask) == 0)
      return;

   GLXX_SERVER_STATE_T* state = GL31_LOCK_SERVER_STATE();
   if (!state)
      return;

   // Flush all render-states with buffer writes belonging to this context.
   glxx_hw_render_state_foreach(memory_barrier, state);

   // Flush compute render-state belonging to this context.
   if (state->compute_render_state != NULL)
      glxx_compute_render_state_flush(state->compute_render_state);

   GL31_UNLOCK_SERVER_STATE();
}

// timh-todo: Implement without performing a flush.
GL_APICALL void GL_APIENTRY glMemoryBarrierByRegion(GLbitfield barriers)
{
   PROFILE_FUNCTION_MT("GL");

   GLbitfield const valid_bits =
         GL_ATOMIC_COUNTER_BARRIER_BIT
      |  GL_FRAMEBUFFER_BARRIER_BIT
      |  GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
      |  GL_SHADER_STORAGE_BARRIER_BIT
      |  GL_TEXTURE_FETCH_BARRIER_BIT
      |  GL_UNIFORM_BARRIER_BIT;

   GLXX_SERVER_STATE_T* state = GL31_LOCK_SERVER_STATE();
   if (!state)
      return;

   if (barriers != GL_ALL_BARRIER_BITS && (barriers & ~valid_bits) != 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return;
   }

   glxx_hw_render_state_foreach(memory_barrier, state);

   GL31_UNLOCK_SERVER_STATE();
}
