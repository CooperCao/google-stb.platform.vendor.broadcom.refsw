/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_client_mangle.h"
#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/glxx/gl11_int_impl.h"
#include "interface/khronos/glxx/glxx_client.h"

static void draw_texf(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   if (IS_OPENGLES_11(thread)) {
      glDrawTexfOES_impl_11(x, y, z, width, height, IS_OPENGLES_SECURE(thread));
   }
}

GL_API void GL_APIENTRY glDrawTexsOES(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height)
{
   draw_texf((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)width, (GLfloat)height);
}

GL_API void GL_APIENTRY glDrawTexiOES(GLint x, GLint y, GLint z, GLint width, GLint height)
{
   draw_texf((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)width, (GLfloat)height);
}

GL_API void GL_APIENTRY glDrawTexxOES(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height)
{
   draw_texf(fixed_to_float(x), fixed_to_float(y), fixed_to_float(z), fixed_to_float(width), fixed_to_float(height));
}

GL_API void GL_APIENTRY glDrawTexsvOES(const GLshort *coords)
{
   draw_texf((GLfloat)coords[0], (GLfloat)coords[1], (GLfloat)coords[2], (GLfloat)coords[3], (GLfloat)coords[4]);
}

GL_API void GL_APIENTRY glDrawTexivOES(const GLint *coords)
{
   draw_texf((GLfloat)coords[0], (GLfloat)coords[1], (GLfloat)coords[2], (GLfloat)coords[3], (GLfloat)coords[4]);
}

GL_API void GL_APIENTRY glDrawTexxvOES(const GLfixed *coords)
{
   draw_texf(fixed_to_float(coords[0]), fixed_to_float(coords[1]), fixed_to_float(coords[2]), fixed_to_float(coords[3]), fixed_to_float(coords[4]));
}

GL_API void GL_APIENTRY glDrawTexfOES(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height)
{
   draw_texf(x, y, z, width, height);
}

GL_API void GL_APIENTRY glDrawTexfvOES(const GLfloat *coords)
{
   draw_texf((GLfloat)coords[0], (GLfloat)coords[1], (GLfloat)coords[2], (GLfloat)coords[3], (GLfloat)coords[4]);
}