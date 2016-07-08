/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION

=============================================================================*/

#include "../glxx/gl_public_api.h"

#include "../common/khrn_int_common.h"

#include "../glxx/glxx_server.h"

GL_API void* GL_APIENTRY glMapBufferOES(GLenum target, GLenum access)
{
   void *p = NULL;

   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   if (!state) return NULL;

   if (access != GL_WRITE_ONLY) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }
   p = glxx_map_buffer_range(state, target, 0, SIZE_MAX, GL_MAP_WRITE_BIT);

end:
   GLXX_UNLOCK_SERVER_STATE();
   return p;
}

GL_API GLboolean GL_APIENTRY glUnmapBufferOES(GLenum target)
{
   GLboolean res = GL_FALSE;

   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   if (!state) return GL_FALSE;

   res = glxx_unmap_buffer(state, target);
   GLXX_UNLOCK_SERVER_STATE();

   return res;
}

GL_API void GL_APIENTRY glGetBufferPointervOES(GLenum target, GLenum pname, GLvoid ** params)
{
   glxx_get_buffer_pointerv(target, pname, params);
}
