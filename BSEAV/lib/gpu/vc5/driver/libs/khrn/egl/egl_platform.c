/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "egl_platform.h"
#include "egl_thread.h"

EGLint egl_platform_get_display(EGLenum platform, void *nativeDisplay,
      const void *attribList, EGL_AttribType attribType,
      void **handle)
{
   /* convert an empty attribute lists into a NULL list for easier handling */
   if (attribList && egl_attrib_list_item(&attribList, attribType,
         /*increment=*/false) == EGL_NONE)
      attribList = NULL;

   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->get_display)
   {
      /* only the default display is supported */
      if (platform == BEGL_DEFAULT_PLATFORM && !nativeDisplay && !attribList)
      {
         *handle = (EGLDisplay *)1;
         return EGL_SUCCESS;
      }
      else
      {
         *handle = EGL_NO_DISPLAY;
         return EGL_BAD_PARAMETER;
      }
   }
   else
   {
      return fns->get_display(platform, nativeDisplay, attribList,
            attribType == attrib_EGLAttrib, handle);
   }
}

bool egl_platform_initialize(void *handle)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->initialize) return true;
   return fns->initialize(handle);
}

void egl_platform_terminate(void *handle)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (fns->terminate)
      fns->terminate(handle);
}

bool egl_platform_color_format(GFX_LFMT_T lfmt, EGLint *platFormat)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->color_format)
   {
      *platFormat = lfmt;
      return true;
   }

   return fns->color_format(lfmt, platFormat);
}

const char *egl_platform_get_client_extensions(void)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->get_client_extensions) return NULL;
   return fns->get_client_extensions();
}

const char *egl_platform_get_display_extensions(void)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->get_display_extensions) return NULL;
   return fns->get_display_extensions();
}

__eglMustCastToProperFunctionPointerType egl_platform_get_proc_address(
      const char *procname)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->get_proc_address) return NULL;
   return fns->get_proc_address(procname);
}

bool egl_platform_match_pixmap(EGLNativePixmapType pixmap,
      const EGL_CONFIG_T *config)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->match_pixmap) return false;
   return fns->match_pixmap(pixmap, config);
}

bool egl_platform_config_get_attrib(const EGL_CONFIG_T *config,
      EGLint attrib, EGLint *value)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->config_get_attrib)
      return false;
   return fns->config_get_attrib(config, attrib, value);
}

bool egl_platform_config_check_attrib(EGLint attrib, EGLint value)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->config_check_attrib)
      return false;
   return fns->config_check_attrib(attrib, value);
}

bool egl_platform_config_match_attrib(EGLint attrib,
      EGLint requested, EGLint actual)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->config_match_attrib)
      return false;
   return fns->config_match_attrib(attrib, requested, actual);
}

bool egl_platform_wait_native(EGLint engine)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->wait_native)
      return true;
   return fns->wait_native(engine);
}

EGL_IMAGE_T *egl_platform_image_new(
        EGL_CONTEXT_T *ctx, EGLenum target, EGLClientBuffer buffer,
        const void *attrib_list, EGL_AttribType attrib_type)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->image_new)
   {
      egl_thread_set_error(EGL_BAD_PARAMETER);
      return NULL;
   }
   return fns->image_new(ctx, target, buffer, attrib_list, attrib_type);
}

EGL_SYNC_T *egl_platform_synckhr_new(EGL_CONTEXT_T *ctx, EGLenum type,
      const void *attrib_list, EGL_AttribType attrib_type)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->synckhr_new)
   {
      egl_thread_set_error(EGL_BAD_ATTRIBUTE);
      return NULL;
   }
   return fns->synckhr_new(ctx, type, attrib_list, attrib_type);
}
