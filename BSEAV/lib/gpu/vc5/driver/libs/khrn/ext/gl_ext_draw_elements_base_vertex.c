/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../glxx/gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_draw.h"


static inline void draw_elements_base_vertex(
   GLenum mode,
   GLsizei count,
   GLenum index_type,
   const GLvoid *indices,
   GLint basevertex)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
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

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glDrawElementsBaseVertexEXT(GLenum mode,
      GLsizei count, GLenum index_type, const GLvoid *indices, GLint basevertex)
{
   draw_elements_base_vertex(mode, count, index_type, indices, basevertex);
}

GL_APICALL void GL_APIENTRY glDrawElementsBaseVertexOES(GLenum mode,
      GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
   draw_elements_base_vertex(mode, count, type, indices, basevertex);
}

#if KHRN_GLES32_DRIVER
GL_APICALL void GL_APIENTRY glDrawElementsBaseVertex(GLenum mode,
      GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
   draw_elements_base_vertex(mode, count, type, indices, basevertex);
}
#endif

static inline void draw_range_elements_base_vertex(
   GLenum mode,
   GLuint start,
   GLuint end,
   GLsizei count,
   GLenum index_type,
   const GLvoid *indices,
   GLint basevertex)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) {
      return;
   }

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .start = start,
      .end = end,
      .count = count,
      .index_type = index_type,
      .indices = indices,
      .basevertex = basevertex};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glDrawRangeElementsBaseVertexEXT(GLenum mode,
      GLuint start, GLuint end, GLsizei count, GLenum index_type,
      const GLvoid *indices, GLint basevertex)
{
   draw_range_elements_base_vertex(mode, start, end, count, index_type,
         indices, basevertex);
}

GL_APICALL void GL_APIENTRY glDrawRangeElementsBaseVertexOES(GLenum mode,
      GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices,
      GLint basevertex)
{
   draw_range_elements_base_vertex(mode, start, end, count, type, indices,
         basevertex);
}

#if KHRN_GLES32_DRIVER
GL_APICALL void GL_APIENTRY glDrawRangeElementsBaseVertex(GLenum mode,
      GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices,
      GLint basevertex)
{
   draw_range_elements_base_vertex(mode, start, end, count, type, indices,
         basevertex);
}
#endif

static inline void draw_elements_instanced_base_vertex(
   GLenum mode,
   GLsizei count,
   GLenum index_type,
   const GLvoid *indices,
   GLsizei instanceCount,
   GLint basevertex)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
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

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseVertexEXT(GLenum mode,
      GLsizei count, GLenum index_type, const GLvoid *indices,
      GLsizei instanceCount, GLint basevertex)
{
   draw_elements_instanced_base_vertex(mode, count, index_type, indices,
         instanceCount, basevertex);
}

GL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseVertexOES(GLenum mode,
      GLsizei count, GLenum type, const void *indices, GLsizei instancecount,
      GLint basevertex)
{
   draw_elements_instanced_base_vertex(mode, count, type, indices,
         instancecount, basevertex);
}

#if KHRN_GLES32_DRIVER
GL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseVertex(GLenum mode,
      GLsizei count, GLenum type, const void *indices, GLsizei instancecount,
      GLint basevertex)
{
   draw_elements_instanced_base_vertex(mode, count, type, indices,
         instancecount, basevertex);
}
#endif

// We only need the functions below if EXT_multi_draw_arrays is supported, which it isn't.
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
static inline void multi_draw_elements_base_vertex(
   GLenum mode,
   const GLsizei *count,
   GLenum type,
   const void *const*indices,
   GLsizei primcount,
   const GLint *basevertex)
{
   unused(mode);
   unused(count);
   unused(type);
   unused(indices);
   unused(primcount);
   unused(basevertex);
}

GL_APICALL void GL_APIENTRY glMultiDrawElementsBaseVertexEXT(GLenum mode,
      const GLsizei *count, GLenum type, const void * const *indices,
      GLsizei primcount, const GLint *basevertex)
{
   multi_draw_elements_base_vertex(mode, count, type, indices, primcount,
         basevertex);
}

// Note: The "OES" variant of this extension call this function
// glMultiDrawElementsBaseVertexEXT in the official Khronos extension spec and
// glMultiDrawElementsBaseVertexOES in the official Khronos gl.xml
// Providing both "EXT" and "OES" seems safer choice as auto-generated
// headers will include "OES" suffix (from gl.xml).
GL_APICALL void GL_APIENTRY glMultiDrawElementsBaseVertexOES(GLenum mode,
      const GLsizei *count, GLenum type, const void * const *indices,
      GLsizei primcount, const GLint *basevertex)
{
   multi_draw_elements_base_vertex(mode, count, type, indices, primcount,
         basevertex);
}
