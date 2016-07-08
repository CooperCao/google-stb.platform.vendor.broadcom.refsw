/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Server-side implementation of the GL_OES_EGL_image extension for GLES 1.1/2.0.
=============================================================================*/
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#include "middleware/khronos/egl/egl_server.h"

#include "middleware/khronos/ext/egl_khr_image.h"
#include "interface/khronos/include/GLES2/gl2.h"
#include "interface/khronos/include/GLES2/gl2ext.h"

/*
   void glEGLImageTargetTexture2DOES_impl (GLenum target, GLeglImageOES image)

   Khronos documentation:
Another way of specifying two-dimensional texture images is to
    reference them from existing EGLImage objects.  Images specified
    this way will be EGLImage siblings with the original EGLImage
    source and any other EGLImage targets.

    The command

        void EGLImageTargetTexture2DOES(enum target, eglImageOES image);

    defines an entire two-dimensional texture array.  All properties
    of the texture images (including width, height, format, border, mipmap
    levels of detail, and image data) are taken from the specified
    eglImageOES <image>, rather than from the client or the framebuffer.
    Any existing image arrays associated with any mipmap levels in the texture
    object are freed (as if TexImage was called for each, with an image of
    zero size).  As a result of this referencing operation, all of the pixel
    data in the <buffer> used as the EGLImage source resource (i.e., the
    <buffer> parameter passed to the CreateImageOES command that returned
    <image>) will become undefined.

    Currently, <target> must be TEXTURE_2D.  <image> must be the
    handle of a valid EGLImage resource, cast into the type eglImageOES.
    Assuming no errors are generated in EGLImageTargetTexture2DOES, the
    newly specified texture object will be an EGLImage target of the
    specified  eglImageOES.  If an application later respecifies any image
    array in the texture object (through mechanisms such as calls to
    TexImage2D and/or GenerateMipmapOES, or setting the
    SGIS_GENERATE_MIPMAP parameter to TRUE), implementations should allocate
    additional space for all specified (and respecified) image arrays,
    and copy any existing image data to the newly (re)specified texture
    object (as if TexImage was called for every level-of-detail in the
    texture object).  The respecified texture object will not be an
    EGLImage target.

    If the GL is unable to specify a texture object using the supplied
    eglImageOES <image> (if, for example, <image> refers to a multisampled
    eglImageOES), the error INVALID_OPERATION is generated.

    If <target> is not TEXTURE_2D, the error INVALID_ENUM is generated.
*/

void glEGLImageTargetTexture2DOES_impl (GLenum target, GLeglImageOES image)
{
   GLenum error = GL_NONE;
   MEM_HANDLE_T htexture;
   GLXX_TEXTURE_T *texture;
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   EGL_SERVER_STATE_T *eglstate = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T heglimage = khrn_map_lookup(&eglstate->eglimages, (uint32_t)image);

   if (heglimage == MEM_INVALID_HANDLE)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   switch (target)
   {
   case GL_TEXTURE_2D:
   case GL_TEXTURE_EXTERNAL_OES:
      break;
   default:
       error = GL_INVALID_ENUM;
       goto end;
   }

   texture = glxx_server_state_get_texture(state, target, GL_FALSE, &htexture);

   if (texture) {
      EGL_IMAGE_T * egl_image = (EGL_IMAGE_T *)mem_lock(heglimage, NULL);
      KHRN_IMAGE_T * image;

      MEM_HANDLE_T himage = egl_image->mh_image;

      image = (KHRN_IMAGE_T *)mem_lock(himage, NULL);

      vcos_assert(texture->target != GL_TEXTURE_CUBE_MAP);
      if (glxx_texture_is_valid_image(image))
      {
         glxx_texture_bind_images(texture, 1, &himage, TEXTURE_STATE_BOUND_EGLIMAGE, MEM_INVALID_HANDLE, 0);
         /* The bind above removes all previous bindings, so assign the external image here */
         if (khrn_image_is_rso(image->format))
            MEM_ASSIGN(texture->external_image, heglimage);
      }
      else
      {
#ifdef ANDROID
         /* HACK, don't return an error for 4k images.  These should be handled by HWC, so if you
            try to texture with them you'll only get black */
         if ((image->width > MAX_TEXTURE_SIZE && image->width < 4096) ||
             (image->height > MAX_TEXTURE_SIZE && image->height < 4096))
            error = GL_NONE;
         else
#endif
         error = GL_INVALID_OPERATION;
      }

      mem_unlock(himage);
      mem_unlock(heglimage);
      mem_unlock(htexture);
   }

end:
   if (error != GL_NONE)
   {
      glxx_server_state_set_error(state, error);
   }

   GLXX_UNLOCK_SERVER_STATE();
}

/*
   void glEGLImageTargetRenderbufferStorageOES_impl_20 (GLenum target, GLeglImageOES image)

   Khronos documentation:

   The command

        void EGLImageTargetRenderbufferStorageOES(enum target,
                                                  eglImageOES image)

    establishes the data storage, format, and dimensions of a
    renderbuffer object's image, using the parameters and storage
    associated with the eglImageOES <image>.  Assuming no errors
    are generated in this process, the resulting renderbuffer
    will be an EGLImage target of the specifed eglImageOES <image>.
    The renderbuffer will remain an EGLImage target until it is
    deleted or respecified by a call to
    {Reference}RenderbufferStorageOES.  As a result of this referencing
    operation, all of the pixel data in the <buffer> used as the
    EGLImage source resource (i.e., the <buffer> parameter passed to
    the CreateImageOES command that returned <image>) will become
    undefined.

    <target> must be RENDERBUFFER_OES, and <image> must be the
    handle of a valid EGLImage resource, cast into the type
    eglImageOES.

    If the GL is unable to create a renderbuffer using the specified
    eglImageOES, the error INVALID_OPERATION is generated.  If <image>
    does not refer to a valid eglImageOES object, the error
    INVALID_VALUE is generated.

    Dependencies on OES_framebuffer_object

    If the OES_framebuffer_object extension is not supported, all
    discussion of renderbuffers should be ignored, and all calls to
    EGLImageTargetRenderbufferStorageOES should generate the error
    INVALID_OPERATION.
*/

void glEGLImageTargetRenderbufferStorageOES_impl_20 (GLenum target, GLeglImageOES image)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   EGL_SERVER_STATE_T *eglstate = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T heglimage = khrn_map_lookup(&eglstate->eglimages, (uint32_t)image);

   if (heglimage == MEM_INVALID_HANDLE)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else if (state->mh_bound_renderbuffer == MEM_INVALID_HANDLE)
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
   else if (target != GL_RENDERBUFFER)
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else {
      GLXX_RENDERBUFFER_T *renderbuffer;
      MEM_HANDLE_T himage = ((EGL_IMAGE_T *)mem_lock(heglimage, NULL))->mh_image;
      mem_unlock(heglimage);

      renderbuffer = (GLXX_RENDERBUFFER_T *)mem_lock(state->mh_bound_renderbuffer, NULL);

      if (!glxx_renderbuffer_bind_image(renderbuffer, himage))
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);

      mem_unlock(state->mh_bound_renderbuffer);
   }

   GLXX_UNLOCK_SERVER_STATE();
}
