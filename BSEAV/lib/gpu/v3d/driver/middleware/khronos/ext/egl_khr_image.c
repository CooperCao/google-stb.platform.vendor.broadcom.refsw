/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/ext/egl_khr_image.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/gl11/gl11_server.h"
#include "middleware/khronos/gl20/gl20_server.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/include/EGL/eglext_brcm.h"
#include "middleware/khronos/common/2708/khrn_tfconvert_4.h"
#include "middleware/khronos/common/khrn_mem.h"

/* */
static void egl_image_term(void *p)
{
   EGL_IMAGE_T *eglimage = p;
   KHRN_MEM_ASSIGN(eglimage->image, NULL);
}

/*
    int eglCreateImageKHR_impl (EGL_CONTEXT_TYPE_T glversion, EGL_CONTEXT_ID_T ctx, EGLenum target, EGLClientBuffer buffer, EGLint texture_level, EGLint *results)

    EGL_BAD_CONTEXT     ctx is not a valid EGLContext handle
    EGL_BAD_PARAMETER   target is not one of the values in Table aaa
    EGL_BAD_PARAMETER   attr_list includes an attribute not listed in Table bbb
    EGL_BAD_ACCESS      Resource bound with eglBindTexImage, eglCreatePbufferFromClientBuffer or is already an EGLImage sibling
    EGL_BAD_ALLOC       Out of memory
    EGL_BAD_MATCH       Type of object does not match type of context
    EGL_BAD_PARAMETER   Buffer is not the name of a valid object of the specified target type in the specified context
    EGL_BAD_PARAMETER   Buffer refers to default object (0)
    EGL_BAD_PARAMETER   Buffer refers to an incomplete texture or a renderbuffer without storage (TODO: should we forbid depth/stencil renderbuffers at this stage?)
    EGL_BAD_MATCH       VGImage is a child image
    EGL_BAD_MATCH       Invalid mipmap level specified

   results
   0  EGLImageKHR       Handle to new EGLImage (0 on failure)
   1  EGLInt            Error code (EGL_SUCCESS on success)
   We always return 2.

   Khronos documentation:
   [mostly EGL_KHR_image but other extensions amend it]

   The command

        EGLImageKHR eglCreateImageKHR(
                                EGLDisplay dpy,
                                EGLContext ctx,
                                EGLenum target,
                                EGLClientBuffer buffer,
                                EGLint* attr_list)

    is used to create an EGLImage from an existing image resource <buffer>.
    <dpy> specifies the EGL display used for this operation, or EGL_NO_DISPLAY
    if a display is not required.  <ctx> specifies the EGL client API context
    used for this operation, or EGL_NO_CONTEXT if a client API context is not
    required.  <target> specifies the type of resource being used as the
    EGLImage source (examples include two-dimensional textures in OpenGL ES
    contexts and VGImage objects in OpenVG contexts).  <buffer> is the name
    (or handle) of a resource to be used as the EGLImage source, cast into the
    type EGLClientBuffer.  <attr_list> is an list of attribute-value pairs
    which is used to select sub-sections of <buffer> for use as the EGLImage
    source, such as mipmap levels for OpenGL ES texture map resources.  If
    <attr_list> is non-NULL, the last attribute specified in the list should
    be EGL_NONE.

    The resource specified by <dpy>, <ctx>, <target>, <buffer>, and
    <attr_list> must not itself be an EGLImage sibling, or bound to an EGL
    PBuffer resource (eglBindTexImage, eglCreatePbufferFromClientBuffer).

    Values accepted for <target> are listed in Table aaa, below.

      +-----------------------------------------+------------------------------------+
      |  <target>                               |  Notes                             |
      +-----------------------------------------+------------------------------------+
      |  EGL_NATIVE_PIXMAP_KHR                  |Used for EGLNativePixmapType objects|    [EGL_KHR_image_pixmap]
      +-----------------------------------------+------------------------------------+
      |  EGL_VG_PARENT_IMAGE_KHR                |  Used for OpenVG VGImage objects   |    [EGL_KHR_vg_parent_image]
      +-----------------------------------------+------------------------------------+
      |  EGL_GL_TEXTURE_2D_KHR                  |      Used for OpenGL ES            |    [EGL_KHR_gl_texture_2D_image]
      |                                         |      2D texture images.            |
      +-----------------------------------------+------------------------------------+
      |  EGL_GL_TEXTURE_CUBEMAP_POSITIVE_X_KHR  |  Used for the +X face of           |    [EGL_KHR_gl_texture_cubemap_image]
      |                                         |  OpenGL-ES cubemap texture         |
      |                                         |  images                            |
      +-----------------------------------------+------------------------------------+
      |  EGL_GL_TEXTURE_CUBEMAP_NEGATIVE_X_KHR  |  Used for the -X face of           |
      |                                         |  OpenGL-ES cubemap texture         |
      |                                         |  images                            |
      +-----------------------------------------+------------------------------------+
      |  EGL_GL_TEXTURE_CUBEMAP_POSITIVE_Y_KHR  |  Used for the +Y face of           |
      |                                         |  OpenGL-ES cubemap texture         |
      |                                         |  images                            |
      +-----------------------------------------+------------------------------------+
      |  EGL_GL_TEXTURE_CUBEMAP_NEGATIVE_Y_KHR  |  Used for the -Y face of           |
      |                                         |  OpenGL-ES cubemap texture         |
      |                                         |  images                            |
      +-----------------------------------------+------------------------------------+
      |  EGL_GL_TEXTURE_CUBEMAP_POSITIVE_Z_KHR  |  Used for the +Z face of           |
      |                                         |  OpenGL-ES cubemap texture         |
      |                                         |  images                            |
      +-----------------------------------------+------------------------------------+
      |  EGL_GL_TEXTURE_CUBEMAP_NEGATIVE_Z_KHR  |  Used for the -Z face of           |
      |                                         |  OpenGL-ES cubemap texture         |
      |                                         |  images                            |
      +-----------------------------------------+------------------------------------+
      |  EGL_GL_RENDERBUFFER_KHR                |  Used for OpenGL ES                |    [EGL_KHR_gl_texture_renderbuffer_image]
      |                                         |  renderbuffer images               |
      +-----------------------------------------+------------------------------------+
       Table aaa.  Legal values for eglCreateImageKHR <target> parameter

    Attributes accepted in the attribute-value list <attr_list> are listed
    in Table bbb, below.

      +------------------------+-----------------------------------------+---------------+
      | Attribute              | Description                             | Default Value |
      +------------------------+-----------------------------------------+---------------+
      |  EGL_NONE              |Marks the end of the attribute-value list| N/A           |
      +------------------------+-----------------------------------------+---------------+
      |EGL_GL_TEXTURE_LEVEL_KHR|  Specifies the mipmap level             |   0           |    [EGL_KHR_gl_*]
      |                        |  used as the EGLImage source.           |               |
      |                        |  Must be part of the complete           |               |
      |                        |  texture object <buffer>                |               |
      +-----------+------------------------------------------------------+---------------+
       Table bbb.  Legal attributes for eglCreateImageKHR <attr_list> parameter

    If <target> is EGL_NATIVE_PIXMAP_KHR, <dpy> must be a valid display, <ctx>
    must be EGL_NO_CONTEXT; <buffer> must be a handle to a valid
    NativePixmapType object, cast into the type EGLClientBuffer, and
    <attr_list> is ignored.

    [EGL_KHR_vg_parent_image]
    If <target> is EGL_VG_PARENT_IMAGE_KHR, <dpy> must be a valid EGLDisplay,
    <ctx> must be a valid OpenVG API context on that display, and <buffer>
    must be a handle of a VGImage object valid in the specified context, cast
    into the type EGLClientBuffer.  Furthermore, the specified VGImage
    <buffer> must not be a child image (i.e. the value returned by
    vgGetParent(<buffer>) must be <buffer>).  If the specified VGImage
    <buffer> has any child images (i.e., vgChildImage has been previously
    called with the parent parameter set to <buffer>), all child images will
    be treated as EGLImage siblings after CreateImageKHR returns.  Any values
    specified in <attr_list> are ignored.

    [EGL_KHR_gl_*]
    If <target> is EGL_GL_TEXTURE_2D_KHR,
    EGL_GL_RENDERBUFFER_KHR,
    EGL_GL_TEXTURE_CUBEMAP_POSITIVE_X_KHR,
    EGL_GL_TEXTURE_CUBEMAP_NEGATIVE_X_KHR,
    EGL_GL_TEXTURE_CUBEMAP_POSITIVE_Y_KHR,
    EGL_GL_TEXTURE_CUBEMAP_NEGATIVE_Y_KHR,
    EGL_GL_TEXTURE_CUBEMAP_POSITIVE_Z_KHR, or
    EGL_GL_TEXTURE_CUBEMAP_NEGATIVE_Z_KHR,
    <dpy> must be a valid EGLDisplay,
    and <ctx> must be a valid OpenGL ES API context on that display.

    [EGL_KHR_gl_texture_2D_image]
    If <target> is EGL_GL_TEXTURE_2D_KHR, <buffer> must be the name of a
    complete, non-default EGL_GL_TEXTURE_2D target texture object, cast into
    the type EGLClientBuffer.  <attr_list> should specify the mipmap level
    which will be used as the EGLImage source (EGL_GL_TEXTURE_LEVEL_KHR); the
    specified mipmap level must be part of <buffer>.  If not specified, the
    default value listed in Table bbb will be used, instead.  Additional
    values specified in <attr_list> are ignored.

    [EGL_KHR_gl_texture_cubemap_image]
    If <target> is one of the EGL_GL_TEXTURE_CUBEMAP_* enumerants,
    <buffer> must be the name of a cube-complete, non-default
    EGL_GL_TEXTURE_CUBEMAP_OES target texture object, cast into the type
    EGLClientBuffer.  <attr_list> should specify the mipmap level which will
    be used as the EGLImage source (EGL_GL_TEXTURE_LEVEL_KHR); the specified
    mipmap level must be part of <buffer>.  If not specified, the default
    value listed in Table bbb will be used, instead.  Additional values
    specified in <attr_list> are ignored.

    [EGL_KHR_gl_renderbuffer_image]
    If <target> is EGL_GL_RENDERBUFFER_KHR, <buffer> must be the name of a
    complete, non-default EGL_GL_RENDERBUFFER_OES object, cast into the type
    EGLClientBuffer.  Values specified in <attr_list> are ignored.

    This command returns an EGLImageKHR object corresponding to the image
    data specified by <dpy>, <ctx>, <target>, <buffer> and <attr_list> which
    may be referenced by client API operations, or EGL_NO_IMAGE_KHR in the
    event of an error.  After this function returns, the pixel data in
    <buffer> will be undefined (this applies to all sub-objects which may be
    included in <buffer>, not just those which are specified by <attr_list>).
    If the <attr_list> contains attributes from table bbb which are not
    applicable to the buffer specified by <target> then those extra attributes
    are ignored.

       * If <dpy> is not the handle of a valid EGLDisplay object or
         EGL_NO_DISPLAY, the error EGL_BAD_DISPLAY is generated.

       * If <ctx> is not the handle of a valid EGLContext object on <dpy> or
         EGL_NO_CONTEXT, the error EGL_BAD_CONTEXT
         is generated.

       * If <target> is not one of the values in Table aaa, the error
         EGL_BAD_PARAMETER is generated.

       * If <target> is EGL_NATIVE_PIXMAP_KHR and <buffer> is not a
         valid native pixmap handle, or if there is no EGLConfig
         supporting native pixmaps whose color buffer format matches the
         format of <buffer>, the error EGL_BAD_PARAMETER is generated.

       * If an attribute specified in <attr_list> is not one of the
         attributes listed in Table bbb, the error EGL_BAD_PARAMETER is
         generated.

       * If the resource specified by <dpy>, <ctx>, <target>, <buffer> and
         <attr_list> has an off-screen buffer bound to it (e.g., by a
         previous call to eglBindTexImage), the error EGL_BAD_ACCESS is
         generated.

       * If the resource specified by <dpy>, <ctx>, <target>, <buffer> and
         <attr_list> is bound to an off-screen buffer (e.g., by a previous
         call to eglCreatePbufferFromClientBuffer), the error
         EGL_BAD_ACCESS is generated.

       * If the resource specified by <dpy>, <ctx>, <target>, <buffer> and
         <attr_list> is itself an EGLImage sibling, the error
         EGL_BAD_ACCESS is generated.

       * If <target> is EGL_NATIVE_PIXMAP_KHR, and <dpy> is not a valid
         EGLDisplay object the error EGL_BAD_DISPLAY is generated.

       * If <target> is EGL_NATIVE_PIXMAP_KHR, and <ctx> is not EGL_NO_CONTEXT,
         the error EGL_BAD_PARAMETER is generated.

       * If <target> is NATIVE_PIXMAP_KHR, and <buffer> is not a handle
         to a valid NativePixmapType object, the error EGL_BAD_PARAMETER
         is generated.

       * If insufficient memory is available to complete the specified
         operation, the error EGL_BAD_ALLOC is generated.

       * If the call to eglCreateImageKHR fails for multiple reasons, the
         generated error must be appropriate for one of the reasons,
         although the specific error returned is undefined.

    [EGL_KHR_vg_parent_image]
        * If <target> is EGL_VG_PARENT_IMAGE_KHR, and <dpy> is not a
          valid EGLDisplay, the error EGL_BAD_DISPLAY is generated.

        * If <target> is EGL_VG_PARENT_IMAGE_KHR and <ctx> is not a
          valid EGLContext, the error EGL_BAD_CONTEXT is generated.

        * If <target> is EGL_VG_PARENT_IMAGE_KHR and <ctx> is not a valid
          OpenVG context, the error EGL_BAD_MATCH is returned.

        * If <target> is EGL_VG_PARENT_IMAGE_KHR and <buffer> is not a handle
          to a VGImage object in the specified API context <ctx>, the error
          EGL_BAD_PARAMETER is generated.

        * If <target> is EGL_VG_PARENT_IMAGE_KHR, and the VGImage specified by
          <buffer> is a child image (i.e., vgGetParent(<buffer>) returns
          a different handle), the error EGL_BAD_ACCESS is generated.

    [EGL_KHR_gl_*]
        * If <target> is EGL_GL_TEXTURE_2D_KHR, EGL_GL_TEXTURE_CUBEMAP_*_KHR,
          EGL_GL_RENDERBUFFER_KHR or EGL_GL_TEXTURE_3D_KHR, and <dpy> is not a
          valid EGLDisplay, the error EGL_BAD_DISPLAY is generated.

        * If <target> is EGL_GL_TEXTURE_2D_KHR, EGL_GL_TEXTURE_CUBEMAP_*_KHR,
          EGL_GL_RENDERBUFFER_KHR or EGL_GL_TEXTURE_3D_KHR, and <ctx> is not a
          valid EGLContext, the error EGL_BAD_CONTEXT is generated.

        * If <target> is EGL_GL_TEXTURE_2D_KHR, EGL_GL_TEXTURE_CUBEMAP_*_KHR,
          EGL_GL_RENDERBUFFER_KHR or EGL_GL_TEXTURE_3D_KHR, and <ctx> is not a
          valid OpenGL ES context, or does not match the <dpy>, the error
          EGL_BAD_MATCH is generated.

        * If <target> is EGL_GL_TEXTURE_2D_KHR, EGL_GL_TEXTURE_CUBEMAP_*_KHR
          or EGL_GL_TEXTURE_3D_KHR and <buffer> is not the name of a complete
          texture object of type <target>, the error EGL_BAD_PARAMETER
          is generated.

        * If <target> is EGL_GL_TEXTURE_2D_KHR, EGL_GL_TEXTURE_CUBEMAP_*_KHR,
          EGL_GL_RENDERBUFFER_KHR or EGL_GL_TEXTURE_3D_KHR and <buffer> refers
          to the default object (0), the error EGL_BAD_PARAMETER is
          generated.

        * If <target> is EGL_GL_TEXTURE_2D_KHR, EGL_GL_TEXTURE_CUBEMAP_*_KHR,
          or EGL_GL_TEXTURE_3D_KHR, and the value specified in <attr_list>
          for EGL_GL_TEXTURE_LEVEL_KHR is not a valid mipmap level for
          the specified texture object <buffer>, the error EGL_BAD_MATCH
          is generated.


    Note that the success or failure of eglCreateImageKHR should not affect the
    ability to use <buffer> in its original API context (or context share
    group) (although the pixel data will be undefined).

*/

static uint32_t convert_texture_target(EGLenum target)
{
   switch (target) {
   case EGL_GL_TEXTURE_2D_KHR: return TEXTURE_BUFFER_TWOD;
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR: return TEXTURE_BUFFER_POSITIVE_X;
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR: return TEXTURE_BUFFER_NEGATIVE_X;
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR: return TEXTURE_BUFFER_POSITIVE_Y;
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR: return TEXTURE_BUFFER_NEGATIVE_Y;
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR: return TEXTURE_BUFFER_POSITIVE_Z;
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR: return TEXTURE_BUFFER_NEGATIVE_Z;
   default:
      UNREACHABLE();
      return 0;
   }
}

static KHRN_IMAGE_T *egl_image_texture_new(GLXX_SHARED_T *shared, EGLenum target, EGLClientBuffer buffer, EGLint texture_level, EGLint *error)
{
   GLXX_TEXTURE_T *texture = glxx_shared_get_texture(shared, (uint32_t)(uintptr_t)buffer);   /* returns invalid for default texture (0) */

   if (texture == NULL)
   {
      *error = EGL_BAD_PARAMETER;
      return NULL;
   }

   bool is_cube = target != EGL_GL_TEXTURE_2D_KHR;
   switch (texture->target) {
   case GL_TEXTURE_CUBE_MAP:
      if (!is_cube)
         *error = EGL_BAD_PARAMETER;
      break;
   case GL_TEXTURE_2D:
      if (is_cube)
         *error = EGL_BAD_PARAMETER;
      break;
   case GL_TEXTURE_EXTERNAL_OES:
      /* You can't create eglImages from these */
      *error = EGL_BAD_PARAMETER;
      break;
   default:
      UNREACHABLE();
      break;
   }

   if (*error != EGL_SUCCESS)
      return NULL;

   KHRN_IMAGE_T *image = NULL;

   bool base_complete = false;
   GLXX_TEXTURE_COMPLETENESS_T status = glxx_texture_check_complete_levels(texture, base_complete);
   if (status != COMPLETE) {
      base_complete = true;
      status = glxx_texture_check_complete_levels(texture, base_complete);
   }

   switch (status) {
   case COMPLETE:
      {
         uint32_t mipmap_count = glxx_texture_get_mipmap_count(texture);
         if (texture_level < 0 || (uint32_t)texture_level >= mipmap_count)
            *error = EGL_BAD_PARAMETER;
         else if (texture->binding_type == TEXTURE_STATE_BOUND_TEXIMAGE || texture->binding_type == TEXTURE_STATE_BOUND_EGLIMAGE)
            *error = EGL_BAD_ACCESS;
         else {
            /* TODO: should we reject certain textures? (e.g. paletted, ETC1) */

            /* make sure there are no other mipmap levels outside base_level and
            * num_levels */
            unsigned num_levels = base_complete ? 1 : glxx_texture_get_mipmap_count(texture);
            if (!glxx_texture_has_images_outside_range(texture, num_levels)) {
               /* At this point we should succeed */
               KHRN_MEM_ASSIGN(image, glxx_texture_share_mipmap(texture, convert_texture_target(target), texture_level));

               /*
               * TODO: should we set the IMAGE_FLAG_BOUND_EGLIMAGE flag
               * of the other mipmaps?
               */
               texture->binding_type = TEXTURE_STATE_BOUND_TEXIMAGE;
            }
            else
               *error = EGL_BAD_PARAMETER;
         }
      }
      break;
   case INCOMPLETE:
      *error = EGL_BAD_PARAMETER;
      break;
   case OUT_OF_MEMORY:
      *error = EGL_BAD_ALLOC;
      break;
   default:
      UNREACHABLE();
   }

   return image;
}

static KHRN_IMAGE_T *egl_image_renderbuffer_new(GLXX_SHARED_T *shared, EGLClientBuffer buffer, EGLint *error)
{
   GLXX_RENDERBUFFER_T *renderbuffer = glxx_shared_get_renderbuffer(shared, (uint32_t)(uintptr_t)buffer, false);
   if (renderbuffer == NULL)
   {
      *error = EGL_BAD_PARAMETER;
      return NULL;
   }

   KHRN_IMAGE_T *image = NULL;
   if (renderbuffer->storage == NULL)
      *error = EGL_BAD_PARAMETER;
   else if (!glxx_renderbuffer_unmerge(renderbuffer))
      *error = EGL_BAD_ALLOC;
   else
      KHRN_MEM_ASSIGN(image, renderbuffer->storage);

   return image;
}

EGLImageKHR eglCreateImageKHR_impl (
   EGLenum target,
   EGLClientBuffer egl_buffer,
   EGLint texture_level,
   EGLint *error)
{
   EGL_SERVER_STATE_T *eglstate = EGL_GET_SERVER_STATE();
   KHRN_IMAGE_T *image = NULL;
   *error = EGL_SUCCESS;

   /*
    * Preallocate the EGL_IMAGE_T struct and insert it into the map
    */

   EGL_IMAGE_T *eglimage = KHRN_MEM_ALLOC_STRUCT(EGL_IMAGE_T);

   if (!eglimage || !khrn_map_insert(&eglstate->eglimages, eglstate->next_eglimage, eglimage))
   {
      if (eglimage)
         KHRN_MEM_ASSIGN(eglimage, NULL);

      *error = EGL_BAD_ALLOC;
      return 0;
   }

   khrn_mem_set_term(eglimage, egl_image_term);

   void *native_buffer = NULL;
   bool platform_client_buffer = false;
   switch (target) {
   case EGL_NATIVE_PIXMAP_KHR:
   {
      /*
         if we get this far, we know we're dealing with a server-side pixmap
      */

      image = egl_server_platform_create_pixmap_info((void *)egl_buffer, false);

      break;
   }
   case EGL_GL_TEXTURE_2D_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
   case EGL_GL_RENDERBUFFER_KHR:
   {
      GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
      if (state == NULL)
      {
         *error = EGL_BAD_MATCH;
         break;
      }

      GLXX_SHARED_T *shared = state->shared;
      if (shared != NULL) {
         switch (target) {
         case EGL_GL_TEXTURE_2D_KHR:
         case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
         case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
         case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
         case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
         case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
         case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
            image = egl_image_texture_new(shared, target, egl_buffer, texture_level, error);
            break;
         case EGL_GL_RENDERBUFFER_KHR:
            image = egl_image_renderbuffer_new(shared, egl_buffer, error);
            break;
         default:
            UNREACHABLE();
         }
      }

      glxx_unlock_server_state(OPENGL_ES_ANY);

      break;
   }
   default:
      native_buffer = egl_server_platform_get_native_buffer(target, egl_buffer);
      image = egl_server_platform_image_new(target, native_buffer, error);
      if ((image != NULL) && (*error == EGL_SUCCESS))
         platform_client_buffer = true;
      break;
   }

   assert(((image == NULL) && (*error != EGL_SUCCESS)) ||
      ((image != NULL) && (*error == EGL_SUCCESS)));

   if (image != NULL) {
      if (image->flags & (IMAGE_FLAG_BOUND_CLIENTBUFFER|IMAGE_FLAG_BOUND_TEXIMAGE|IMAGE_FLAG_BOUND_EGLIMAGE)) {
         /*
          * Bound to an offscreen buffer, or has an offscreen buffer bound to
          * it, or is already an EGLImage sibling
          */
         *error = EGL_BAD_ACCESS;
         image = NULL;
      } else
         image->flags |= IMAGE_FLAG_BOUND_EGLIMAGE;
   }

   if (*error == EGL_SUCCESS) {
      assert(image != NULL);

      eglimage->target = target;
      eglimage->native_buffer = native_buffer;
      eglimage->platform_client_buffer = platform_client_buffer;

      KHRN_MEM_ASSIGN(eglimage->image, image);
      KHRN_MEM_ASSIGN(image, NULL);
      KHRN_MEM_ASSIGN(eglimage, NULL);

      return (EGLImageKHR)(uintptr_t)eglstate->next_eglimage++;
   } else {
      /*
       * Remove the entry we added to the map
       */
      khrn_map_delete(&eglstate->eglimages, eglstate->next_eglimage);
      KHRN_MEM_ASSIGN(eglimage, NULL);
      return 0;
   }
}

/*
   Khronos documentation:

   The command

           EGLBoolean eglDestroyImageKHR(
                               EGLDisplay dpy,
                               EGLImageKHR image)

    is used to destroy the specified EGLImageKHR object <image>.  Once
    destroyed, <image> may not be used to create any additional EGLImage
    target resources within any client API contexts, although existing
    EGLImage siblings may continue to be used.  EGL_TRUE is returned
    if DestroyImageKHR succeeds, EGL_FALSE indicates failure.

       * If <dpy> is not the handle of a valid EGLDisplay object or
         EGL_NO_DISPLAY, the error EGL_BAD_DISPLAY is generated.

       * If <dpy> is not the same as the <dpy> used to create the <image>,
         then EGL_BAD_MATCH is generated.

      * If <image> is not a valid EGLImageKHR object, the error
        EGL_BAD_PARAMETER is generated."
*/

EGLBoolean eglDestroyImageKHR_impl (EGLImageKHR image)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   EGL_IMAGE_T *eglimage = khrn_map_lookup(&state->eglimages, (uint32_t)(uintptr_t)image);

   if (eglimage != NULL) {
      KHRN_IMAGE_T *image = eglimage->image;
      khrn_interlock_write_immediate(&image->interlock);
   }

   return khrn_map_delete(&state->eglimages, (uint32_t)(uintptr_t)image) ? EGL_TRUE : EGL_FALSE;
}
