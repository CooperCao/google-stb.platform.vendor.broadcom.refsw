/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_PLATFORM_H
#define EGL_PLATFORM_H
#include <EGL/egl.h>
#include "egl_config.h"
#include "egl_image.h"
#include "egl_attrib_list.h"
#include "libs/core/lfmt/lfmt.h"

struct wl_display;

struct egl_platform_fns
{
   /*
    * One-off platform initialization that happens right at the start when
    * everything else is initialized in a thread-safe way. Can be NULL.
    */
   bool        (*init)(void);

   /*
    * Check if EGL_EXT_platform_* is supported. If unimplemented the assumed
    * answer is always true.
    */
   bool        (*is_platform_supported)(EGLenum platform);

   /* Convert an internal format to a platform colour format, can be NULL */
   bool        (*color_format)(GFX_LFMT_T lfmt, EGLint *platFormat);

   /*
    * Anything to add to result of eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS)
    * for this platform. Can be NULL.
    */
   const char  *(*get_client_extensions)(void);

   /*
    * Anything to add to result of eglQueryString(valid_display, EGL_EXTENSIONS) for this
    * platform. Can be NULL.
    */
   const char  *(*get_display_extensions)(void);

   /*
    * Anything to add to result of eglGetProcAddress(procname) for this
    * platform. Can be NULL.
    */
   __eglMustCastToProperFunctionPointerType (*get_proc_address)(
         const char *procname);

   /* Return a default display. Can be NULL. */
   EGLDisplay  (*get_default_display)(void);

   /*
    * Set the default display. If this is called after the first call to
    * get_default_display, then it should return false (and whatever else it
    * does is undefined).
    */
   bool        (*set_default_display)(EGLNativeDisplayType display);

   /*
    * Any cleanup the platform needs to do when the whole process quits. Can be
    * NULL.
    */
   void        (*terminate)(void);

   /*
    * Return true if config can be used to render to pixmap. Can be NULL, in
    * which case false is assumed
    */
   bool        (*match_pixmap)(EGLNativePixmapType pixmap,
                  const EGL_CONFIG_T *config);

   /*
    * Return the value of attrib in config, or 0 if attrib is not recognized as
    * something specific to this platform.
    *
    * Sets *used to true if attrib was recognized.
    *
    * Can be NULL.
    *
    * The uses of return value and pointer arg are swapped compared with
    * eglGetConfigAttrib because it's more useful this way around-- often you
    * already know the attrib is valid so can just pass NULL to valid.
    *
    * Matches egl_config_get_attrib()
    */
   bool      (*config_get_attrib)(const EGL_CONFIG_T *config,
                  EGLint attrib, EGLint *value);

   /*
    * Return true if attrib is an attribute specific to this platform and value
    * is a valid value of it. Can be NULL.
    */
   bool        (*config_check_attrib)(EGLint attrib, EGLint value);

   /*
    * Return true if a value of requested in an attrib list for attrib matches
    * an actual value (as returned by config_get_attrib). Guaranteed only to be
    * called for attribs and values for which config_check_attrib returns true,
    * i.e. ones that are specific to this platform. Can be NULL.
    */
   bool        (*config_match_attrib)(EGLint attrib, EGLint requested,
                  EGLint actual);

   /* See eglWaitNative. Can be NULL (which means no waiting) */
   bool        (*wait_native)(EGLint engine);

   /*
    * Implements the platform specific part of eglCreateImageKHR
    * Create a new EGL Image type specific to your platform.
    *
    * Returns a valid platform-specific EGL_IMAGE_T * on success. Note that
    * ctx may be NULL, as not all image require contexts. If you do require
    * one, and ctx is NULL, then you should call egl_thread_set_error with
    * EGL_BAD_CONTEXT.
    * In case of failure, returns NULL and the function is responsible for
    * setting any egl error.
    *
    * Can be NULL.
    */
   EGL_IMAGE_T *(*image_new)(EGL_CONTEXT_T *ctx, EGLenum target,
                  EGLClientBuffer buffer, const void *attrib_list,
                  EGL_AttribType attrib_type);

   /* Implements the platform specific part of eglCreateSyncKHR
    * Create a new EGLSyncKHR specific to your platform.
    *
    * Returns an EGL_SYNC_T* on success. ctx is not NULL. You need to check if
    * your platform supports the requested type and the corresponding
    * attrrib_list.
    * In case of failure, returns NULL and the function is responsible for
    * setting any egl error.
    *
    * Can be NULL.
    */
    EGL_SYNC_T* (*synckhr_new)(EGL_CONTEXT_T *ctx, EGLenum type,
          const void *attrib_list, EGL_AttribType attrib_type);
};

/* Platform defines this. */
extern EGL_PLATFORM_FNS_T *egl_platform_fns(void);

/* These all call into the fns returned by egl_platform_fns */
extern bool egl_platform_init(void);

extern bool egl_platform_supported(EGLenum platform);
extern bool egl_platform_color_format(GFX_LFMT_T lfmt, EGLint *platFormat);
extern const char *egl_platform_get_client_extensions(void);
extern const char *egl_platform_get_display_extensions(void);
extern __eglMustCastToProperFunctionPointerType egl_platform_get_proc_address(
      const char *procname);
extern EGLDisplay egl_platform_get_default_display(void);
extern bool egl_platform_set_default_display(EGLNativeDisplayType display);
extern void egl_platform_terminate(void);
extern bool egl_platform_match_pixmap(EGLNativePixmapType pixmap,
      const EGL_CONFIG_T *config);

/* See eglWaitNative */
extern bool egl_platform_wait_native(EGLint engine);

/*
 * The uses of return value and pointer arg are swapped compared with
 * eglGetConfigAttrib because it's more useful this way around-- often you
 * already know the attrib is valid so can just pass NULL to valid.
 *
 * Matches egl_config_get_attrib()
*/
extern bool egl_platform_config_get_attrib(const EGL_CONFIG_T *config,
      EGLint attrib, EGLint *value);
extern bool egl_platform_config_check_attrib(EGLint attrib, EGLint value);
extern bool egl_platform_config_match_attrib(EGLint attrib,
      EGLint requested, EGLint actual);

/*
 * In case of failure, returns NULL and the function is responsible for setting
 * any egl error corresponding to the platform specific eglCreateImageKHR.
 */
extern EGL_IMAGE_T *egl_platform_image_new(EGL_CONTEXT_T *ctx, EGLenum target,
      EGLClientBuffer buffer, const void *attrib_list,
      EGL_AttribType attrib_type);
/*
 * In case of failure, returns NULL and the function is responsible for setting
 * any egl error corresponding to the platform specific eglCreateSyncKHR.
 */
extern EGL_SYNC_T *egl_platform_synckhr_new(EGL_CONTEXT_T *ctx, EGLenum type,
      const void *attrib_list, EGL_AttribType attrib_type);

#endif /* EGL_PLATFORM_H */
