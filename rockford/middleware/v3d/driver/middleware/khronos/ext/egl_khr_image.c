/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Server-side implementation of the EGLImage extensions for EGL:
   EGL_KHR_image
   EGL_KHR_vg_parent_image
   EGL_KHR_gl_texture_2D_image
   EGL_KHR_gl_texture_cubemap_image
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/ext/egl_khr_image.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/gl11/gl11_server.h"
#include "middleware/khronos/gl20/gl20_server.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#ifndef NO_OPENVG
#include "middleware/khronos/vg/vg_server.h"
#endif /* NO_OPENVG */
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/include/EGL/eglext_brcm.h"
#include "middleware/khronos/common/2708/khrn_tfconvert_4.h"

#ifdef ANDROID
#include <cutils/log.h>
#include <system/window.h>
#include "gralloc_priv.h"
#endif

/* */
void egl_image_term(void *v, uint32_t size)
{
   EGL_IMAGE_T *image = (EGL_IMAGE_T *)v;
   UNUSED(size);

   /* if we've run through the failure case, then these won't be created */
   if (image->update_control_initialized)
   {
      vcos_semaphore_delete(&image->lockSemaphore);
      vcos_semaphore_delete(&image->missedConvFlag);
      vcos_mutex_delete(&image->dirtyBitsMutex);
   }

   MEM_ASSIGN(image->mh_image, MEM_INVALID_HANDLE);

   /* BCG_EGLIMAGE_CONVERTER : clean up shadow buffer */
   MEM_ASSIGN(image->mh_shadow_image, MEM_INVALID_HANDLE);
}

/*
    int eglCreateImageKHR_impl (uint32_t glversion, EGL_CONTEXT_ID_T ctx, EGLenum target, EGLClientBuffer buffer, EGLint texture_level, EGLint *results)

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

int eglCreateImageKHR_impl (
   uint32_t glversion,
   EGL_CONTEXT_ID_T ctx,
   EGLenum target,
   EGLClientBuffer buffer,
   EGLint texture_level,
   EGLint *results)
{
   EGLint error = EGL_SUCCESS;
   EGL_SERVER_STATE_T *eglstate = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T heglimage = MEM_INVALID_HANDLE;
   MEM_HANDLE_T himage = MEM_INVALID_HANDLE;
   bool himage_acquired = false;
   EGLClientBuffer refcntbuffer = 0;

   /*
    * Preallocate the EGL_IMAGE_T struct and insert it into the map
    */

   heglimage = MEM_ALLOC_STRUCT_EX(EGL_IMAGE_T, MEM_COMPACT_DISCARD);

   if (!heglimage || !khrn_map_insert(&eglstate->eglimages, eglstate->next_eglimage, heglimage))
   {
      if (heglimage)
         mem_release(heglimage);

      results[0] = 0;
      results[1] = EGL_BAD_ALLOC;
      return 2;
   }

   mem_set_term(heglimage, egl_image_term);
   mem_release(heglimage);

   switch (target) {
   case EGL_NATIVE_PIXMAP_KHR:
   {
      /*
         if we get this far, we know we're dealing with a server-side pixmap
      */

      himage = egl_server_platform_create_pixmap_info((uint32_t)buffer, &error);
      himage_acquired = true;

      break;
   }
#ifndef NO_OPENVG
   case EGL_VG_PARENT_IMAGE_KHR:
   {
      MEM_HANDLE_T hcontext;
      MEM_HANDLE_T hvgimage;
      VG_SERVER_STATE_T *vgstate;

      hcontext = khrn_map_lookup(&eglstate->vgcontexts, ctx);
      vcos_assert(hcontext != MEM_INVALID_HANDLE);
      vgstate = (VG_SERVER_STATE_T *)mem_lock(hcontext, NULL);

      hvgimage = vg_get_image(vgstate, (VGImage)buffer, &error);

      if (hvgimage != MEM_INVALID_HANDLE) {
         if (vg_is_child_image(hvgimage))
            error = EGL_BAD_MATCH;
         else {
            VG_IMAGE_T *vgimage = (VG_IMAGE_T *)mem_lock(hvgimage, NULL);

            if (vg_image_leak(vgimage)) {
               himage = vgimage->image;
            } else {
               error = EGL_BAD_ALLOC;
            }

            mem_unlock(hvgimage);
         }
      }

      mem_unlock(hcontext);
      break;
   }
#endif
   case EGL_GL_TEXTURE_2D_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
   case EGL_GL_RENDERBUFFER_KHR:
   {
      MEM_HANDLE_T hcontext;
      MEM_HANDLE_T hshared = MEM_INVALID_HANDLE;

      hcontext = khrn_map_lookup(&eglstate->glcontexts, ctx);
      switch (glversion) {
      case EGL_SERVER_GL11:
      case EGL_SERVER_GL20:
      {
         GLXX_SERVER_STATE_T *glstate = (GLXX_SERVER_STATE_T *)mem_lock(hcontext, NULL);

         hshared = glstate->mh_shared;

         mem_unlock(hcontext);
         break;
      }
      default:
         error = EGL_BAD_MATCH;
      }
      if (hshared != MEM_INVALID_HANDLE) {
         GLXX_SHARED_T *shared = (GLXX_SHARED_T *)mem_lock(hshared, NULL);
         switch (target) {
         case EGL_GL_TEXTURE_2D_KHR:
         case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
         case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
         case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
         case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
         case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
         case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
         {
            MEM_HANDLE_T htexture = glxx_shared_get_texture(shared, (uint32_t)buffer);   /* returns invalid for default texture (0) */
            uint32_t buffer = convert_texture_target(target);
            bool is_cube = target != EGL_GL_TEXTURE_2D_KHR;

            if (htexture == MEM_INVALID_HANDLE)
               error = EGL_BAD_PARAMETER;
            else {
               GLXX_TEXTURE_T *texture = (GLXX_TEXTURE_T *)mem_lock(htexture, NULL);

               switch (texture->target) {
               case GL_TEXTURE_CUBE_MAP:
                   if (!is_cube)
                       error = EGL_BAD_PARAMETER;
                   break;
               case GL_TEXTURE_2D:
                   if (is_cube)
                       error = EGL_BAD_PARAMETER;
                   break;
               case GL_TEXTURE_EXTERNAL_OES:
                   /* You can't create eglImages from these */
                   error = EGL_BAD_PARAMETER;
                   break;
               default:
                   UNREACHABLE();
                   break;
               }

               if (error != EGL_SUCCESS) {
                   mem_unlock(htexture);
                   break;
               }

               switch (glxx_texture_check_complete(texture))
               {
               case COMPLETE:
               {
                  uint32_t mipmap_count = glxx_texture_get_mipmap_count(texture);
                  if (texture_level < 0 || (uint32_t)texture_level >= mipmap_count)
                     error = EGL_BAD_PARAMETER;
                  else if (texture->binding_type == TEXTURE_STATE_BOUND_TEXIMAGE || texture->binding_type == TEXTURE_STATE_BOUND_EGLIMAGE)
                     error = EGL_BAD_ACCESS;
                  else {
                     /* TODO: should we reject certain textures? (e.g. paletted, ETC1) */

                     /* At this point we should succeed */
                     himage = glxx_texture_share_mipmap(texture, buffer, texture_level);

                     /*
                      * TODO: should we set the IMAGE_FLAG_BOUND_EGLIMAGE flag
                      * of the other mipmaps?
                      */
                     texture->binding_type = TEXTURE_STATE_BOUND_TEXIMAGE;
                  }
                  break;
               }
               case INCOMPLETE:
                  error = EGL_BAD_PARAMETER;
                  break;
               case OUT_OF_MEMORY:
                  error = EGL_BAD_ALLOC;
                  break;
               default:
                  UNREACHABLE();
               }

               mem_unlock(htexture);
            }
            break;
         }
         case EGL_GL_RENDERBUFFER_KHR:
         {
            MEM_HANDLE_T hrenderbuffer = glxx_shared_get_renderbuffer(shared, (uint32_t)buffer, false);
            if (hrenderbuffer == MEM_INVALID_HANDLE)
               error = EGL_BAD_PARAMETER;
            else {
               GLXX_RENDERBUFFER_T *renderbuffer = (GLXX_RENDERBUFFER_T *)mem_lock(hrenderbuffer, NULL);
               if (renderbuffer->mh_storage == MEM_INVALID_HANDLE)
                  error = EGL_BAD_PARAMETER;
               else if (!glxx_renderbuffer_unmerge(renderbuffer))
                  error = EGL_BAD_ALLOC;
               else
                  himage = renderbuffer->mh_storage;
               mem_unlock(hrenderbuffer);
            }
            break;
         }
         default:
            UNREACHABLE();
         }
         mem_unlock(hshared);
      }
      break;
   }
#ifdef EGL_BRCM_image_wrap_bcg
   case EGL_IMAGE_WRAP_BRCM_BCG:
   {
      MEM_HANDLE_T handle;
      KHRN_IMAGE_FORMAT_T format;

      /* these are only used in the khrn_image_platform_fudge().  They get modified, so need to take a copy here */
      uint32_t width, height, align;
      width = ((EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *)buffer)->width;
      height = ((EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *)buffer)->height;

      handle = mem_wrap(((EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *)buffer)->storage,
         ((EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *)buffer)->offset,
         ((EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *)buffer)->height * ((EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *)buffer)->width,
         1, MEM_FLAG_DIRECT,
         "EGL_IMAGE_WRAP_BRCM_BCG");

      switch (((EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *)buffer)->format)
      {
      case BEGL_BufferFormat_eA8B8G8R8_TFormat:          format = ABGR_8888_TF;          break;
      case BEGL_BufferFormat_eX8B8G8R8_TFormat:          format = XBGR_8888_TF;          break;
      case BEGL_BufferFormat_eR5G6B5_TFormat:            format = RGB_565_TF;            break;
      case BEGL_BufferFormat_eR5G5B5A1_TFormat:          format = RGBA_5551_TF;          break;
      case BEGL_BufferFormat_eR4G4B4A4_TFormat:          format = RGBA_4444_TF;          break;
      default:
         break;
      }

      /* need to call fudge to adjust TF to LT if the size is such */
      khrn_image_platform_fudge(&format, &width, &height, &align, IMAGE_CREATE_FLAG_TEXTURE);

      himage = khrn_image_create_from_storage(format,
         ((EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *)buffer)->width,
         ((EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *)buffer)->height,
         ((EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *)buffer)->stride,
         MEM_INVALID_HANDLE, handle, 0, IMAGE_CREATE_FLAG_DISPLAY, false);
      himage_acquired = true;

      mem_release(handle);

      break;
   }
#endif
#ifdef ANDROID
   case EGL_NATIVE_BUFFER_ANDROID:
   {
      MEM_HANDLE_T handle;
      void * p;
      uint32_t offset, w, h, stride;
      KHRN_IMAGE_FORMAT_T format = ABGR_8888_RSO;
      android_native_buffer_t *android_buffer = (android_native_buffer_t *)buffer;

      khrn_platform_decode_native(buffer, &w, &h, &stride, &format, &offset, &p);

      handle = mem_wrap(p, offset,
                        h * stride,
                        1, MEM_FLAG_DIRECT,
                        "EGL_NATIVE_BUFFER_ANDROID");

      himage = khrn_image_create_from_storage(format,
         w, h, stride,
         MEM_INVALID_HANDLE, handle, 0, IMAGE_CREATE_FLAG_TEXTURE | IMAGE_CREATE_FLAG_RSO_TEXTURE, false);
      himage_acquired = true;

      mem_release(handle);

      refcntbuffer = buffer;

      android_buffer->common.incRef(&android_buffer->common);

      break;
   }
#endif

   default:
      error = EGL_BAD_PARAMETER;
   }

   vcos_assert(((himage == MEM_INVALID_HANDLE) && (error != EGL_SUCCESS)) ||
      ((himage != MEM_INVALID_HANDLE) && (error == EGL_SUCCESS)));

   if (himage != MEM_INVALID_HANDLE) {
      KHRN_IMAGE_T *image = (KHRN_IMAGE_T *)mem_lock(himage, NULL);
      if (image->flags & (IMAGE_FLAG_BOUND_CLIENTBUFFER|IMAGE_FLAG_BOUND_TEXIMAGE|IMAGE_FLAG_BOUND_EGLIMAGE)) {
         /*
          * Bound to an offscreen buffer, or has an offscreen buffer bound to
          * it, or is already an EGLImage sibling
          */
         mem_unlock(himage);
         error = EGL_BAD_ACCESS;
         himage = MEM_INVALID_HANDLE;
      } else {
         image->flags |= IMAGE_FLAG_BOUND_EGLIMAGE;
         mem_unlock(himage);
      }
   }

   if (error == EGL_SUCCESS) {
      EGL_IMAGE_T *eglimage;
      vcos_assert(himage != MEM_INVALID_HANDLE);

      eglimage = (EGL_IMAGE_T *)mem_lock(heglimage, NULL);
      eglimage->pid = eglstate->pid;
      eglimage->buffer = (uint32_t)refcntbuffer;

      /* BCG_EGLIMAGE_CONVERTER : create shadow and control list buffers */
      eglimage->mh_shadow_image = MEM_INVALID_HANDLE;
      eglimage->convertedFrame = ~0;      /* Ensures conversion first time out */

      vcos_mutex_create(&eglimage->dirtyBitsMutex, "eglImageMutex");
      vcos_semaphore_create(&eglimage->lockSemaphore, "eglImageLockSem", 1);
      vcos_semaphore_create(&eglimage->missedConvFlag, "eglImageMissedConv", 1);
      /* resources have been correctly allocated, so can be removed */
      eglimage->update_control_initialized = true;

      MEM_ASSIGN(eglimage->mh_image, himage);

      if (himage_acquired) { mem_release(himage); }
      mem_unlock(heglimage);

      results[0] = eglstate->next_eglimage++;
   } else {
      /*
       * Remove the entry we added to the map
       */
      khrn_map_delete(&eglstate->eglimages, eglstate->next_eglimage);
      results[0] = 0;
   }
   results[1] = error;

   return 2;
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
   uint32_t id = (uint32_t)image;
   MEM_HANDLE_T heglimage;

   heglimage = khrn_map_lookup(&state->eglimages, (uint32_t)id);
   if (heglimage) {
#ifdef ANDROID
      android_native_buffer_t *android_buffer;
#endif
      KHRN_IMAGE_T *image;
      EGL_IMAGE_T *eglimage = (EGL_IMAGE_T *)mem_lock(heglimage, NULL);
      MEM_HANDLE_T himage = eglimage->mh_image;

      image = (KHRN_IMAGE_T *)mem_lock(himage, NULL);

      khrn_interlock_write_immediate(&image->interlock);

#ifdef ANDROID
      /* If a regular texture is bound as an eglimage, then this is 0 */
      if (eglimage->buffer) {
         android_buffer = (android_native_buffer_t *)eglimage->buffer;
         /* if android creates any type of window other than EGL_NATIVE_BUFFER_ANDROID then it could be
            NULL.  The refsw implementation doesnt do this, so its probably not absolutely necessary */
         if (android_buffer != NULL) {
            vcos_assert(ANDROID_NATIVE_BUFFER_MAGIC == android_buffer->common.magic);
            android_buffer->common.decRef(&android_buffer->common);
         }
      }
#endif

      mem_unlock(heglimage);
      mem_unlock(himage);
   }

   return khrn_map_delete(&state->eglimages, id) ? EGL_TRUE : EGL_FALSE;
}

bool egl_image_any_tile_dirty(EGL_IMAGE_T *img)
{
   uint32_t r;

   if (!img->explicit_updates)
      return true;   // Always dirty when not using explicit updates

   for (r = 0; r < KHRN_HW_TEX_SIZE_MAX / 64; r++)
      if (img->dirtyBits.m_rowBits[r] != 0)
         return true;

   return false;
}

bool egl_image_is_tile_dirty(EGL_IMAGE_T *img, uint32_t x, uint32_t y)
{
   return (!img->explicit_updates) || ((img->dirtyBits.m_rowBits[y] & (1 << x)) != 0);
}

// Nice macro to set up a table of bit counts
static const uint8_t BitsSetTable256[256] =

{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
   B6(0), B6(1), B6(1), B6(2)
};

uint32_t egl_image_num_dirty_tiles(EGL_IMAGE_T *img)
{
   uint32_t c = 0;

   uint32_t r;

   if (!img->explicit_updates)
      return (KHRN_HW_TEX_SIZE_MAX / 64) * (KHRN_HW_TEX_SIZE_MAX / 64);

   for (r = 0; r < KHRN_HW_TEX_SIZE_MAX / 64; r++)
   {
      uint8_t *p = (uint8_t*)&img->dirtyBits.m_rowBits[r];

#if KHRN_HW_TEX_SIZE_MAX <= 2048
      c += BitsSetTable256[p[0]] + BitsSetTable256[p[1]] + BitsSetTable256[p[2]] + BitsSetTable256[p[3]];
#elif KHRN_HW_TEX_SIZE_MAX <= 4096
      c += BitsSetTable256[p[0]] + BitsSetTable256[p[1]] + BitsSetTable256[p[2]] + BitsSetTable256[p[3]] +
           BitsSetTable256[p[4]] + BitsSetTable256[p[5]] + BitsSetTable256[p[6]] + BitsSetTable256[p[7]];
#endif
   }

   return c;
}
