/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "vcos.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>
#include "../common/khrn_image.h"
#include "../common/khrn_event_monitor.h"
#include "egl_image.h"
#include "egl_display.h"
#include "egl_thread.h"
#include "egl_image_texture.h"
#include "egl_image_renderbuffer.h"
#include "egl_image_framebuffer.h"
#include "egl_platform.h"
#include "egl_attrib_list.h"

static void destroy(EGL_IMAGE_T* egl_image)
{
   KHRN_MEM_ASSIGN(egl_image->image, NULL);
   free(egl_image);
}

static KHRN_IMAGE_T *get(EGL_IMAGE_T *egl_image)
{
   return egl_image->image;
}

EGL_IMAGE_T* egl_image_create(KHRN_IMAGE_T *image)
{
   assert(image != NULL);
   EGL_IMAGE_T *egl_image = calloc(1, sizeof(EGL_IMAGE_T));
   if (!egl_image)
      return NULL;

   egl_image_init(egl_image, image, destroy, get);
   return egl_image;
}

void egl_image_init(EGL_IMAGE_T* egl_image, KHRN_IMAGE_T *image,
   egl_image_destroy destroy, egl_image_get get)
{
   assert(egl_image != NULL);
   assert(image != NULL);
   assert(destroy != NULL);
   assert(get != NULL);

   egl_image->ref_count = 1;
   KHRN_MEM_ASSIGN(egl_image->image, image);
   egl_image->destroy = destroy;
   egl_image->get = get;
}

void egl_image_refinc(EGL_IMAGE_T *egl_image)
{
   vcos_atomic_inc(&egl_image->ref_count);
}

void egl_image_refdec(EGL_IMAGE_T *egl_image)
{
   int before_dec;

   if (!egl_image) return;

   before_dec = vcos_atomic_dec(&egl_image->ref_count);
   assert(before_dec);

   if (before_dec == 1)
   {
      assert(egl_get_image_handle(egl_image) == EGL_NO_IMAGE_KHR);
      egl_image->destroy(egl_image);
   }
}

KHRN_IMAGE_T *egl_image_get_image(EGL_IMAGE_T *egl_image)
{
   return egl_image->get(egl_image);
}

static EGLImageKHR egl_create_image_impl(EGLDisplay dpy,
       EGLContext ctx, EGLenum target, EGLClientBuffer buffer,
       const void *attrib_list, EGL_AttribType attrib_type)
{
   if (!egl_initialized(dpy, true))
      return EGL_NO_IMAGE_KHR;

   EGL_CONTEXT_T *context = NULL;
   if (ctx != EGL_NO_CONTEXT)
   {
      context = egl_get_context(ctx);
      if (!context)
      {
         egl_thread_set_error(EGL_BAD_CONTEXT);
         return EGL_NO_IMAGE_KHR;
      }
   }

   // Expect egl_thread_set_error to be called by these functions.
   EGL_IMAGE_T *image = NULL;
   switch (target)
   {
   case EGL_GL_TEXTURE_2D_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
   case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
      image = egl_image_texture_new(context, target, buffer, attrib_list, attrib_type);
      break;
   case EGL_GL_RENDERBUFFER_KHR:
      image = egl_image_renderbuffer_new(context, target, buffer);
      break;
   case EGL_GL_FRAMEBUFFER_BRCM:
      image = egl_image_framebuffer_new(context, target, buffer, attrib_list, attrib_type);
      break;
   default:
      image = egl_platform_image_new(context, target, buffer, attrib_list, attrib_type);
      break;
   }

   if (!image)
      return EGL_NO_IMAGE_KHR;;

   EGLImageKHR ret = egl_map_image(image);
   if (!ret)
   {
      egl_image_refdec(image);
      egl_thread_set_error(EGL_BAD_ALLOC);
      return EGL_NO_IMAGE_KHR;
   }

   return ret;
}

EGLAPI EGLImage EGLAPIENTRY eglCreateImage (EGLDisplay dpy,
      EGLContext ctx, EGLenum target, EGLClientBuffer buffer,
      const EGLAttrib *attrib_list)
{
   return egl_create_image_impl(dpy, ctx, target, buffer, attrib_list,
         attrib_EGLAttrib);
}

EGLAPI EGLImageKHR EGLAPIENTRY eglCreateImageKHR(EGLDisplay dpy,
       EGLContext ctx, EGLenum target, EGLClientBuffer buffer,
       const EGLint *attrib_list)
{
   return egl_create_image_impl(dpy, ctx, target, buffer, attrib_list,
         attrib_EGLint);
}

static EGLBoolean egl_destroy_image_impl(EGLDisplay dpy,
      EGLImageKHR image)
{
   EGL_IMAGE_T *egl_image;
   EGLint error = EGL_BAD_PARAMETER;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   egl_image = egl_unmap_image(image);
   if (!egl_image) goto end;

   egl_image_refdec(egl_image);

   error = EGL_SUCCESS;
end:
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroyImage (EGLDisplay dpy, EGLImage image)
{
   return egl_destroy_image_impl(dpy, image);
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroyImageKHR(EGLDisplay dpy,
      EGLImageKHR image)
{
   return egl_destroy_image_impl(dpy, image);
}
