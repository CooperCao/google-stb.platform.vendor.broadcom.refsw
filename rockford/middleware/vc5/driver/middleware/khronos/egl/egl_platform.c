/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "vcos.h"
#include "egl_platform.h"
#include "egl_thread.h"

bool egl_platform_init(void)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->init) return true;
   return fns->init();
}

bool egl_platform_supported(EGLenum platform)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->is_platform_supported) return true;
   return fns->is_platform_supported(platform);
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

EGLDisplay egl_platform_get_default_display(void)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->get_default_display)
      return (EGLDisplay) 1;
   return fns->get_default_display();
}

bool egl_platform_set_default_display(EGLNativeDisplayType display)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->set_default_display)
      return false;
   return fns->set_default_display(display);
}

void egl_platform_terminate(void)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->terminate) return;
   fns->terminate();
}

bool egl_platform_match_pixmap(EGLNativePixmapType pixmap,
      const EGL_CONFIG_T *config)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->match_pixmap) return false;
   return fns->match_pixmap(pixmap, config);
}

EGLint egl_platform_config_get_attrib(const EGL_CONFIG_T *config,
      EGLint attrib, bool *used)
{
   EGL_PLATFORM_FNS_T *fns = egl_platform_fns();
   if (!fns->config_get_attrib)
   {
      *used = false;
      return 0;
   }
   return fns->config_get_attrib(config, attrib, used);
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
