/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  GL Broadcom multisample render to texture extension

FILE DESCRIPTION
Client-side implementation of GL_EXT_multisampled_render_to_texture.
=============================================================================*/

#define GL_GLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_client_mangle.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/glxx/glxx_client.h"
#include "interface/khronos/glxx/glxx_int_impl.h"
#include "interface/khronos/glxx/gl11_int_impl.h"
#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/GLES/glext.h"

#if GL_EXT_multisampled_render_to_texture

extern GL_API void GL_APIENTRY glxx_client_RenderbufferStorageMultisample(GLenum target, GLsizei samples,
   GLenum internalformat, GLsizei width, GLsizei height);
extern GL_API void GL_APIENTRY glxx_client_FramebufferTexture2DMultisample(GLenum target, GLenum attachment,
   GLenum textarget, GLuint texture, GLint level, GLsizei samples);

GL_API void GL_APIENTRY glRenderbufferStorageMultisampleEXT(GLenum target, GLsizei samples,
   GLenum internalformat, GLsizei width, GLsizei height)
{
   glxx_client_RenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

GL_API void GL_APIENTRY glFramebufferTexture2DMultisampleEXT(GLenum target, GLenum attachment,
   GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
   glxx_client_FramebufferTexture2DMultisample(target, attachment, textarget, texture, level, samples);
}

#endif