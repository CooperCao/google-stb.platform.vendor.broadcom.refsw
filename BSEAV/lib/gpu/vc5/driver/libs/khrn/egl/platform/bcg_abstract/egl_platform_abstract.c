/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "vcos.h"
#include <EGL/eglplatform.h>
#include <EGL/egl.h>

/* Expose prototypes added by the platform-specific extensions */
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>
#include <EGL/eglext_wayland.h>
#undef EGL_EGLEXT_PROTOTYPES

#include "../../egl_platform.h"
#include "../../egl_thread.h"
#include "../../../common/khrn_process.h"
#include "../../egl_display.h"
#include "../../egl_thread.h"

#include "libs/platform/v3d_platform.h"

#include "egl_platform_abstract.h"
#include "egl_image_native_buffer_abstract.h"
#include "egl_surface_common_abstract.h"

#ifdef ANDROID
#include "egl_native_fence_sync_android.h"
#include <log/log.h>
#endif

/* Our platform data, shared only by other BCG platform code */
BCG_PLATFORM_DATA_T  g_bcgPlatformData;

#ifdef ANDROID
static bool s_expose_fences = false;
static VCOS_ONCE_T s_expose_fences_inited;

void init_expose_fences(void)
{
   s_expose_fences = property_get_bool("ro.v3d.fence.expose", false);
   ALOGE("%s v3d EGL_ANDROID_native_fence_sync extension %s",
            __FUNCTION__, s_expose_fences ? "enabled": "disabled");
}
#endif

EGLint get_display(EGLenum platform, void *nativeDisplay,
      const void *attribList, bool isEglAttrib, void **handle)
{
   if (g_bcgPlatformData.initInterface.GetDisplay)
   {
      return g_bcgPlatformData.initInterface.GetDisplay(
            g_bcgPlatformData.initInterface.context, platform, nativeDisplay,
            attribList, isEglAttrib, handle);
   }
   else if (platform == BEGL_DEFAULT_PLATFORM && !nativeDisplay && !attribList)
   {
      *handle = (void *)1;
      return EGL_SUCCESS;
   }
   else
      return EGL_BAD_PARAMETER;
}

static bool initialize(void * handle)
{
   if (!g_bcgPlatformData.initInterface.Initialize)
      return true;
   return g_bcgPlatformData.initInterface.Initialize(
         g_bcgPlatformData.initInterface.context,
         (BEGL_DisplayHandle)handle) == BEGL_Success;
}

static void terminate(void * handle)
{
   if (g_bcgPlatformData.initInterface.Terminate)
      g_bcgPlatformData.initInterface.Terminate(
         g_bcgPlatformData.initInterface.context,
         (BEGL_DisplayHandle)handle);
}

/* Convert an internal format to a platform colour format. */
static bool color_format(GFX_LFMT_T lfmt, EGLint *platFormat)
{
   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;
   BEGL_BufferFormat begl = 0;
   EGLint retFmt = lfmt;
   *platFormat = retFmt;

   switch (gfx_lfmt_fmt(lfmt))
   {
   case GFX_LFMT_R8_G8_B8_A8_UNORM  : begl = BEGL_BufferFormat_eA8B8G8R8;  break;
   case GFX_LFMT_R8_G8_B8_X8_UNORM  : begl = BEGL_BufferFormat_eX8B8G8R8;  break;
   case GFX_LFMT_B5G6R5_UNORM       : begl = BEGL_BufferFormat_eR5G6B5;    break;
   case GFX_LFMT_A4B4G4R4_UNORM     : begl = BEGL_BufferFormat_eA4B4G4R4;  break;
   case GFX_LFMT_A1B5G5R5_UNORM     : begl = BEGL_BufferFormat_eA1B5G5R5;  break;
   case GFX_LFMT_R8_G8_B8_UNORM     : begl = BEGL_BufferFormat_eR8G8B8;    break;
   case GFX_LFMT_R16_G16_B16_A16_FLOAT : begl = BEGL_BufferFormat_eA16B16G16R16_FP; break;
   case GFX_LFMT_R10G10B10A2_UNORM  : begl = BEGL_BufferFormat_eA2B10G10R10;     break;
#if V3D_VER_AT_LEAST(3, 3, 0, 0)
   case GFX_LFMT_BSTC_RGBA_UNORM    : begl = BEGL_BufferFormat_eBSTC;      break;
#else
   case GFX_LFMT_BSTC_RGBA_UNORM    : return false; /* Not supported before 3.3 */
#endif
   default                          : return false;
   }

   if (platform && platform->GetNativeFormat)
     if (platform->GetNativeFormat(platform->context, begl, (uint32_t*)&retFmt) != BEGL_Success)
        return false;

   *platFormat = retFmt;

   return true;
}

static const char *get_client_extensions(void)
{
   BEGL_InitInterface *platform = &g_bcgPlatformData.initInterface;
   return platform->GetClientExtensions ?
         platform->GetClientExtensions(platform->context) : NULL;
}

static const char *get_display_extensions(void)
{
   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;
   return platform->GetDisplayExtensions ?
         platform->GetDisplayExtensions(platform->context) : NULL;
}

static __eglMustCastToProperFunctionPointerType get_proc_address(const char *procname)
{
#define RETURN_PROC_ADDRESS(api) \
   if (strcmp(procname, #api) == 0) \
      return (__eglMustCastToProperFunctionPointerType)api;

   /* Add API added by platform-specific extensions here */
   RETURN_PROC_ADDRESS(eglBindWaylandDisplayWL);
   RETURN_PROC_ADDRESS(eglUnbindWaylandDisplayWL);
   RETURN_PROC_ADDRESS(eglQueryWaylandBufferWL);

   return NULL;

#undef RETURN_PROC_ADDRESS
}

/* Return true if config can be used to render to pixmap. */
static bool match_pixmap(EGLNativePixmapType pixmap, const EGL_CONFIG_T *config)
{
   /* TODO : return true if the given config can be used to render into the pixmap */
   return false;
}

static bool is_valid_android_framebuffer_target(const EGL_CONFIG_T *config, EGLint attrib)
{
   return (config->color_api_fmt        == GFX_LFMT_R8_G8_B8_A8_UNORM ||
           ((config->color_api_fmt      == GFX_LFMT_R8_G8_B8_UNORM) && config->x_padded)) &&
          config->depth_stencil_api_fmt == GFX_LFMT_NONE &&
          config->stencil_api_fmt       == GFX_LFMT_NONE;
}

static bool is_valid_android_recordable(const EGL_CONFIG_T *config, EGLint attrib)
{
   return config->color_api_fmt == GFX_LFMT_R8_G8_B8_A8_UNORM;
}

/* Return the value of attrib in config, or 0 if attrib is not recognized as
 * something specific to this platform.
 * Sets *used to true if attrib was recognized. */
static bool config_get_attrib(const EGL_CONFIG_T *config, EGLint attrib, EGLint *value)
{
   switch (attrib)
   {
   case EGL_FRAMEBUFFER_TARGET_ANDROID:
      *value = is_valid_android_framebuffer_target(config, attrib) ? EGL_TRUE : EGL_FALSE;
      return true;
   case EGL_RECORDABLE_ANDROID:
      *value = is_valid_android_recordable(config, attrib) ? EGL_TRUE : EGL_FALSE;
      return true;
   }

   return false;
}

/* Return true if attrib is an attribute specific to this platform and value
 * is a valid value of it. */
static bool config_check_attrib(EGLint attrib, EGLint value)
{
   switch (attrib)
   {
   case EGL_MATCH_NATIVE_PIXMAP :
      {
         EGLNativePixmapType   pixmap = (EGLNativePixmapType)(uintptr_t)value;
         if (sizeof(uintptr_t) == 8)
         {
            // TODO : fixme
            printf("EGL_MATCH_NATIVE_PIXMAP & EGL_NATIVE_BUFFER_ANDROID queries will not work on 64-bit platforms.\n");
            printf("See SWVC5-526 for details. The EGL specification cannot allow this to work.\n");
            return false;
         }

         /* Check that value is a valid pixmap */
         if (get_pixmap_format(pixmap) != BEGL_BufferFormat_INVALID)
            return true;
      }
      break;
   case EGL_FRAMEBUFFER_TARGET_ANDROID:
      if (value == EGL_DONT_CARE || value == EGL_FALSE || value == EGL_TRUE)
         return true;
      break;
   case EGL_RECORDABLE_ANDROID:
      if (value == EGL_DONT_CARE || value == EGL_FALSE || value == EGL_TRUE)
         return true;
      break;
   }

   return false;
}

/* Return true if a value of requested in an attrib list for attrib matches
 * an actual value (as returned by config_get_attrib). Guaranteed only to be
 * called for attribs and values for which config_check_attrib returns true,
 * i.e. ones that are specific to this platform. */
static bool config_match_attrib(EGLint attrib, EGLint requested, EGLint actual)
{
   switch (attrib)
   {
      case EGL_FRAMEBUFFER_TARGET_ANDROID:
         if (requested == EGL_DONT_CARE || requested == actual)
            return true;
         break;
      case EGL_RECORDABLE_ANDROID:
         if (requested == EGL_DONT_CARE || requested == actual)
            return true;
         break;
   }
   return false;
}

/* See eglWaitNative. */
static bool wait_native(EGLint engine)
{
   if (engine != EGL_CORE_NATIVE_ENGINE)
   {
      egl_thread_set_error(EGL_BAD_PARAMETER);
      return false;
   }
   /* TODO : This should probably wait for all 'platform native rendering' to complete
             This might be an M2MC checkpoint on Nexus platforms for example */
   egl_thread_set_error(EGL_SUCCESS);
   return true;
}

static EGL_PLATFORM_FNS_T fns =
{
   get_display,
   initialize,
   terminate,
   color_format,
   get_client_extensions,
   get_display_extensions,
   get_proc_address,
   match_pixmap,
   config_get_attrib,
   config_check_attrib,
   config_match_attrib,
   wait_native,
   egl_image_native_buffer_abstract_new,
   NULL     /* synckhr_new */
};

#ifdef ANDROID
EGL_PLATFORM_FNS_T *egl_platform_fns(void)
{
   /* make sure s_expose_fences is set */
   vcos_once(&s_expose_fences_inited, init_expose_fences);

   if (s_expose_fences)
   {
      fns.synckhr_new =  egl_native_fence_sync_android_new;
   }
   else
   {
      fns.synckhr_new =  NULL;
   }
   return &fns;
}
#else
EGL_PLATFORM_FNS_T *egl_platform_fns(void)
{
   return &fns;
}
#endif

__attribute__((visibility("default")))
void BEGL_RegisterInitInterface(BEGL_InitInterface *iface)
{
   if (iface != NULL)
      g_bcgPlatformData.initInterface = *iface;
   else
      memset(&g_bcgPlatformData.initInterface, 0, sizeof(BEGL_InitInterface));
}

__attribute__((visibility("default")))
void BEGL_RegisterDisplayInterface(BEGL_DisplayInterface *iface)
{
   if (iface != NULL)
      g_bcgPlatformData.displayInterface = *iface;
   else
      memset(&g_bcgPlatformData.displayInterface, 0, sizeof(BEGL_DisplayInterface));
}

__attribute__((visibility("default")))
void BEGL_PlatformAboutToShutdown(void)
{
   /* we need to do this here to shutdown resources prior to removing the
      platform layer, otherwise we get left with resource leaks and zombie
      processes */
   egl_terminate();
   egl_release_thread();
}
