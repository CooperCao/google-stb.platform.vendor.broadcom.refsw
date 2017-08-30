/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_CONTEXT_BASE_H
#define EGL_CONTEXT_BASE_H

#include <EGL/egl.h>
#include "egl_context.h"

typedef struct
{
   /* Flush all rendering; wait = true --> wait for jobs to finish; required
    * (see eglClientWait) */
   void (*flush)(EGL_CONTEXT_T *context, bool wait);

   /* Copy the source image to the destination image. The copy may not complete
    * before the function returns. flush() can be called with wait=true to wait
    * for the copy. */
   bool (*convert_image)(EGL_CONTEXT_T *context,
         khrn_image *dst, khrn_image *src);

   /* Invalidate the specified draw surface buffers.
    * color implies color_ms (if color is set, color_ms is ignored), but not
    * the other way around. Invalidating just color_ms means we want to throw
    * away the multisample color information but keep the downsampled color. */
   void (*invalidate_draw)(EGL_CONTEXT_T *context,
      bool color, bool color_ms, bool other_aux);

   /* Must call egl_context_base_attach. If NULL, base_attach is called
    * instead. */
   void (*attach)(EGL_CONTEXT_T *context,
                  EGL_SURFACE_T *draw, EGL_SURFACE_T *read);

   /* Must call egl_context_base_detach. If NULL, base_detach is called
    * instead. */
   void (*detach)(EGL_CONTEXT_T *context);

   /* Return client API version. Required. */
   int (*client_version)(const EGL_CONTEXT_T *context);

   /* Destroy any resources inside the context (but don't free it-- that's
    * done in the base class along with allocation). */
   void (*invalidate)(EGL_CONTEXT_T *context);
}
EGL_CONTEXT_METHODS_T;

enum egl_context_type
{
   EGL_CONTEXT_TYPE_GL = 0x200,
   EGL_CONTEXT_TYPE_VG,
};

struct egl_context_base
{
   /* Identify the type. This is just for debugging and assertions etc. */
   egl_context_type_t      type;

   EGLConfig               config;
   EGLDisplay              display;
   egl_api_t               api;
   bool                    valid;
   bool                    debug;
   bool                    robustness;
   bool                    secure;
   bool                    reset_notification;
   /*
    * Used to only notify the reset once with glGetGraphicsResetStatusEXT
    * and then return no error so the application can delete the contexts
   */
   bool gpu_aborted_notified;

   /*
    * Thread this context is bound to. Only used for binding/unbinding and
    * checking binding of context. All other operations should use the current
    * thread.
    */
   EGL_THREAD_T            *bound_thread;

   /* Surfaces that are bound to this context */
   EGL_SURFACE_T           *draw;
   EGL_SURFACE_T           *read;

   EGL_CONTEXT_METHODS_T   *fns;
};

/* Common initialization for a context. */
extern void egl_context_base_init(EGL_CONTEXT_T *context,
      egl_api_t api, EGLConfig config, bool debug,
      bool robustness, bool reset_notification,
      bool secure);

/* Sets context's draw and read surfaces. */
extern void egl_context_base_attach(EGL_CONTEXT_T *context,
      EGL_SURFACE_T *draw, EGL_SURFACE_T *read);

/* Sets context's draw and read surfaces back to NULL. */
extern void egl_context_base_detach(EGL_CONTEXT_T *context);

#endif /* EGL_CONTEXT_BASE_H */
