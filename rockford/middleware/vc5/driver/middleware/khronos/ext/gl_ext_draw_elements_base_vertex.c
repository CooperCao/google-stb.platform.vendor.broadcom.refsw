/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "interface/khronos/glxx/gl_public_api.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_draw.h"

GL_APICALL void GL_APIENTRY glDrawElementsBaseVertexEXT(
   GLenum mode,
   GLsizei count,
   GLenum index_type,
   const GLvoid *indices,
   GLint basevertex)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state) {
      return;
   }

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .count = count,
      .index_type = index_type,
      .indices = indices,
      .basevertex = basevertex};
   glintDrawArraysOrElements(state, &draw);

   GL30_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glDrawRangeElementsBaseVertexEXT(
   GLenum mode,
   GLuint start,
   GLuint end,
   GLsizei count,
   GLenum index_type,
   const GLvoid *indices,
   GLint basevertex)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state) {
      return;
   }

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .min_index = start,
      .max_index = end,
      .count = count,
      .index_type = index_type,
      .indices = indices,
      .basevertex = basevertex};
   glintDrawArraysOrElements(state, &draw);

   GL30_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseVertexEXT(
   GLenum mode,
   GLsizei count,
   GLenum index_type,
   const GLvoid *indices,
   GLsizei instanceCount,
   GLint basevertex)
{
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state) {
      return;
   }

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .count = count,
      .index_type = index_type,
      .indices = indices,
      .instance_count = instanceCount,
      .basevertex = basevertex};
   glintDrawArraysOrElements(state, &draw);

   GL30_UNLOCK_SERVER_STATE();
}

void GL_APIENTRY glMultiDrawElementsBaseVertexEXT(
   GLenum mode,
   const GLsizei *count,
   GLenum type,
   const void *const*indices,
   GLsizei primcount,
   const GLint *basevertex)
{
   // We only need this function if EXT_multi_draw_arrays is supported, which it isn't.
   // However, since it's part of the database from which we auto-generate the get_proc_address code,
   // we do need a symbol to reference, so we'll give it this empty function.
   //
   // Note: this function has not been exported from the DLL so can only be accessed via
   // get_proc_address. This is all allowed by the spec:
   //
   // "A non-NULL return value for eglGetProcAddress does not guarantee that a
   // function is actually supported at runtime.The client must also make a
   // corresponding query, such as glGetString(GL_EXTENSIONS) for OpenGL and
   // OpenGL ES extensions; vgGetString(VG_EXTENSIONS) for OpenVG extensions;
   // eglQueryString(dpy, EGL_EXTENSIONS) for EGL extensions; or query the
   // corresponding API's version for non-extension functions, to determine if
   // a function is supported by a particular client API context or display."

   vcos_unused(mode);
   vcos_unused(count);
   vcos_unused(type);
   vcos_unused(indices);
   vcos_unused(primcount);
   vcos_unused(basevertex);
}
