/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../glxx/gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_server_internal.h"
#include "../glxx/glxx_hw.h"
#include "../glxx/glxx_framebuffer.h"
#include "../glxx/glxx_server_texture.h"
#include "../common/khrn_int_util.h"

static void glDrawTexfOES_impl(GLfloat x_s, GLfloat y_s, GLfloat z_s, GLfloat w_s, GLfloat h_s)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
   GLenum error = GL_NO_ERROR;
   GLfloat z_w;
   GLclampf n, f;

   if (!state)
      return;

   if (w_s <= 0.0f || h_s <= 0.0f)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (!glxx_fb_is_complete(state->bound_draw_framebuffer, &state->fences))
   {
      error = GL_INVALID_FRAMEBUFFER_OPERATION;
      goto end;
   }

   n = state->depth_range.z_near;
   f = state->depth_range.z_far;
   z_w = z_s <= 0 ? n : ( z_s >= 1 ? f : n + z_s * (f - n) );

   if (!glxx_drawtex(state, x_s, y_s, z_w, w_s, h_s))
   {
      error = GL_OUT_OF_MEMORY;
      goto end;
   }
end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glDrawTexfOES(GLfloat x_s, GLfloat y_s, GLfloat z_s, GLfloat w_s, GLfloat h_s)
{
   glDrawTexfOES_impl(x_s, y_s, z_s, w_s, h_s);
}

GL_API void GL_APIENTRY glDrawTexsOES (GLshort x, GLshort y, GLshort z, GLshort width, GLshort height)
{
   glDrawTexfOES_impl((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)width, (GLfloat)height);
}

GL_API void GL_APIENTRY glDrawTexiOES (GLint x, GLint y, GLint z, GLint width, GLint height)
{
   glDrawTexfOES_impl((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)width, (GLfloat)height);
}

GL_API void GL_APIENTRY glDrawTexxOES (GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height)
{
   glDrawTexfOES_impl(fixed_to_float(x), fixed_to_float(y), fixed_to_float(z), fixed_to_float(width), fixed_to_float(height));
}

GL_API void GL_APIENTRY glDrawTexsvOES (const GLshort *coords)
{
   glDrawTexfOES_impl((GLfloat)coords[0],(GLfloat)coords[1],(GLfloat)coords[2], (GLfloat)coords[3],(GLfloat)coords[4]);
}

GL_API void GL_APIENTRY glDrawTexivOES (const GLint *coords)
{
   glDrawTexfOES_impl((GLfloat)coords[0],(GLfloat)coords[1],(GLfloat)coords[2], (GLfloat)coords[3],(GLfloat)coords[4]);
}

GL_API void GL_APIENTRY glDrawTexxvOES (const GLfixed *coords)
{
   glDrawTexfOES_impl(fixed_to_float(coords[0]), fixed_to_float(coords[1]), fixed_to_float(coords[2]), fixed_to_float(coords[3]), fixed_to_float(coords[4]));
}

GL_API void GL_APIENTRY glDrawTexfvOES (const GLfloat *coords)
{
   glDrawTexfOES_impl((GLfloat)coords[0],(GLfloat)coords[1],(GLfloat)coords[2], (GLfloat)coords[3],(GLfloat)coords[4]);
}
