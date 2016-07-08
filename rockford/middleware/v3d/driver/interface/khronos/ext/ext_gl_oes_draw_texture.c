/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Client-side implementation of the GL_OES_draw_texture extension.
http://www.khronos.org/registry/gles/extensions/OES/OES_draw_texture.txt
=============================================================================*/

#define GL_GLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_client_mangle.h"

#include "interface/khronos/common/khrn_int_common.h"

#include "interface/khronos/glxx/glxx_client.h"

#include "interface/khronos/glxx/glxx_int_impl.h"
#include "interface/khronos/glxx/gl11_int_impl.h"

#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/GLES/glext.h"

/* GL_OES_draw_texture */

GL_API void GL_APIENTRY glDrawTexsOES (GLshort x, GLshort y, GLshort z, GLshort width, GLshort height)
{
   glDrawTexfOES((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)width, (GLfloat)height);
}

GL_API void GL_APIENTRY glDrawTexiOES (GLint x, GLint y, GLint z, GLint width, GLint height)
{
   glDrawTexfOES((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)width, (GLfloat)height);
}

GL_API void GL_APIENTRY glDrawTexxOES (GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height)
{
   glDrawTexfOES(fixed_to_float(x), fixed_to_float(y), fixed_to_float(z), fixed_to_float(width), fixed_to_float(height));
}

GL_API void GL_APIENTRY glDrawTexsvOES (const GLshort *coords)
{
   glDrawTexfOES((GLfloat)coords[0], (GLfloat)coords[1], (GLfloat)coords[2], (GLfloat)coords[3], (GLfloat)coords[4]);
}

GL_API void GL_APIENTRY glDrawTexivOES (const GLint *coords)
{
   glDrawTexfOES((GLfloat)coords[0], (GLfloat)coords[1], (GLfloat)coords[2], (GLfloat)coords[3], (GLfloat)coords[4]);
}

GL_API void GL_APIENTRY glDrawTexxvOES (const GLfixed *coords)
{
   glDrawTexfOES(fixed_to_float(coords[0]), fixed_to_float(coords[1]), fixed_to_float(coords[2]), fixed_to_float(coords[3]), fixed_to_float(coords[4]));
}

GL_API void GL_APIENTRY glDrawTexfOES (GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   if (IS_OPENGLES_11(thread)) {
      glDrawTexfOES_impl_11(x, y, z, width, height, IS_OPENGLES_SECURE(thread));
   }
}

GL_API void GL_APIENTRY glDrawTexfvOES (const GLfloat *coords)
{
   glDrawTexfOES((GLfloat)coords[0], (GLfloat)coords[1], (GLfloat)coords[2], (GLfloat)coords[3], (GLfloat)coords[4]);
}
