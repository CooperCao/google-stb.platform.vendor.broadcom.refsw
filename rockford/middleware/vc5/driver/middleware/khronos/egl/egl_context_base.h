/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#ifndef EGL_CONTEXT_BASE_H
#define EGL_CONTEXT_BASE_H

#include "interface/khronos/include/EGL/egl.h"
#include "middleware/khronos/egl/egl_context.h"

typedef struct
{
   /* Flush all rendering; wait = true --> wait for jobs to finish; required
    * (see eglClientWait) */
   void        (*flush)(EGL_CONTEXT_T *context, bool wait);

   /*
    * Flush all rendering destined for surface's back buffer, and return a
    * pointer to dependecies to wait for for rendering to complete.
    * If context is a gl api and surface is the current drawing surface,
    * multisample buffer gets resolved to the color buffer and all ancillary
    * buffers are invalid after this flush.
    */
   v3d_scheduler_deps *(*flush_rendering)(EGL_CONTEXT_T *context,
         EGL_SURFACE_T *surface);

   /*
    * Copy the content of the surface's back buffer to the destination image;
    * If context is a gl api and surface is the current drawing surface, the
    * multisample buffer gets resolved to the color buffer and multisample
    * buffer content is invalid after this operation
    */
   bool (*copy_surface)(EGL_CONTEXT_T *context,
         EGL_SURFACE_T *surface, KHRN_IMAGE_T *dst);

   /*
    * Add a fence on the surface; any reading/writing writing to the surfaces's
    * back buffer will wait for this fence before proceeding
    */
   void        (*add_fence)(EGL_CONTEXT_T *context,
                  const EGL_SURFACE_T *surface, int fence);

   /*
    * Must call egl_context_base_attach. If NULL, base_attach is called
    * instead.
    */
   void        (*attach)(EGL_CONTEXT_T *context,
                  EGL_SURFACE_T *draw, EGL_SURFACE_T *read);

   /*
    * Must call egl_context_base_detach. If NULL, base_detach is called
    * instead.
    */
   void        (*detach)(EGL_CONTEXT_T *context);

   /* Return client API version. Required. */
   int         (*client_version)(const EGL_CONTEXT_T *context);

   /*
    * Destroy any resources inside the context (but don't free it-- that's
    * done in the base class along with allocation).
    */
   void        (*invalidate)(EGL_CONTEXT_T *context);
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
   bool                    robustness;
   bool                    reset_notification;

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
      egl_api_t api, EGLConfig config,
      bool robustness, bool reset_notification);

/* Sets context's draw and read surfaces. */
extern void egl_context_base_attach(EGL_CONTEXT_T *context,
      EGL_SURFACE_T *draw, EGL_SURFACE_T *read);

/* Sets context's draw and read surfaces back to NULL. */
extern void egl_context_base_detach(EGL_CONTEXT_T *context);

#endif /* EGL_CONTEXT_BASE_H */
