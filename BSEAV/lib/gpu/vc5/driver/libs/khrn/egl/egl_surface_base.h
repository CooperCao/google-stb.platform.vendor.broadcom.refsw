/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_SURFACE_BASE_H
#define EGL_SURFACE_BASE_H
#include <EGL/egl.h>
#include "egl_surface.h"
#include "egl_config.h"
#include "egl_attrib_list.h"
#include "../common/khrn_image.h"

#include "libs/util/gfx_util/gfx_util_rect.h"

typedef struct
{
   /* Get the buffer to draw to */
   khrn_image *(*get_back_buffer)(EGL_SURFACE_T *surface);

   /*
    * See eglSwapbuffers. May be NULL in which case back buffer is the same.
    */
   EGLint (*swap_buffers)(EGL_SURFACE_T *surface);

   /*
    * Set the swap interval (see eglSwapInterval). Can be NULL. It's up to the
    * implementation to clamp interval to whatever range it can support.
    */
   void (*swap_interval)(EGL_SURFACE_T *surface, int interval);

   /* These return an EGL error code. *value is not modified in case of error.
    * If not supplied, egl_surface_base_get_attrib is used. */
   EGLint (*get_attrib)(EGL_SURFACE_T *surface, EGLint attrib, EGLint *value);

   /*
    * init_attrib is for setting attributes during surface creation.
    * set_attrib is for setting attributes after surface creation (eglSurfaceAttrib).
    * These are separate functions as they accept completely different attribs.
    * These return an EGL error code (usually EGL_SUCCESS, EGL_BAD_MATCH,
    * EGL_BAD_ATTRIBUTE or EGL_BAD_PARAMETER).
    * If not supplied, egl_surface_base_init_attrib/egl_surface_base_set_attrib
    * are used.
    */
   EGLint (*init_attrib)(EGL_SURFACE_T *surface, EGLint attrib, EGLAttribKHR value);
   EGLint (*set_attrib)(EGL_SURFACE_T *surface, EGLint attrib, EGLAttribKHR value);

   /*
    * Destroys and frees the surface. If not supplied, ordinary free is called
    * instead.
    */
   void (*delete_fn)(EGL_SURFACE_T *surface);
}
EGL_SURFACE_METHODS_T;

typedef struct
{
   khrn_image      *image;
}
EGL_AUX_BUF_T;

/*
 * Make sure you keep these in order (or change egl_surface_base_init_aux_bufs)
 */
enum egl_aux_buf
{
   AUX_DEPTH = 0,
   AUX_STENCIL,
   AUX_MULTISAMPLE,
   AUX_MAX
};

enum egl_surface_type
{
   EGL_SURFACE_TYPE_NATIVE_WINDOW = 0x100,
   EGL_SURFACE_TYPE_PBUFFER,
   EGL_SURFACE_TYPE_PIXMAP,
};

typedef enum egl_buffer_count_mode
{
   MODE_REPORT_REAL_AGE_ZERO = 0,
   MODE_REPORT_REAL_BUFFER_AGE,
   MODE_REPORT_ZERO_BUFFER_AGE
} egl_buffer_count_mode_t;

typedef struct egl_buffer_age_damage_state
{
   gfx_rect               *damage_rects;       /* A malloc'ed list of damage rects, or NULL if none set */
   int                     num_damage_rects;   /* -1 indicates no regions have been set since last swap */
   int                     buffer_age;         /* The real buffer age of the surface                    */
   int                     buffer_age_override;/* The buffer age to report to the application           */
   bool                    buffer_age_queried; /* true if buffer age queried since last swap            */
   bool                    buffer_age_enabled; /* true if buffer age has ever been requested on surface */
   unsigned                big_damage_count;   /* How many large damage rectangles we've seen           */
   unsigned                age_override_count; /* How many times we've overridden the buffer age        */
   float                   damage_coverage;    /* How much of the surface is damaged when damage is     */
                                               /* valid and enabled? (0.0->1.0)                         */
   egl_buffer_count_mode_t mode;               /* The mode the heuristic state machine is in            */
}
EGL_BUFFER_AGE_DAMAGE_STATE_T;

struct egl_surface_base
{
   /*
    * Identify the type. This is mainly just for debugging and assertions etc.,
    * although some API calls like eglBindTexImage also need to check the type
    * of surface and generate errors.
    */
   egl_surface_type_t            type;

   VCOS_MUTEX_T                  lock;
   const EGL_CONFIG_T           *config;
   egl_surface_colorspace_t      gl_colorspace;
   bool                          secure;
   EGLenum                       multisample_resolve;
   unsigned                      width;
   unsigned                      height;
   EGLNativeWindowType           native_window;  /* If this surface is attached to a native window, which one? */
   EGLNativePixmapType           native_pixmap;  /* If this surface is attached to a native pixmap, which one? */

   EGL_BUFFER_AGE_DAMAGE_STATE_T age_damage_state;

   /*
    * The context that this surface is bound to. In other words, context->draw
    * == this or context->read == this, or both.
    */
   EGL_CONTEXT_T                 *context;

   /*
    * These may have NULL khrn_image * in them if they aren't allocated
    * (they often aren't all needed)
    */
   EGL_AUX_BUF_T                 aux_bufs[AUX_MAX];
   const EGL_SURFACE_METHODS_T   *fns;
};

/*
 * Common initialization for a surface. surface->fns must be set up before you
 * call this. Return EGL error codes.
 */
extern EGLint egl_surface_base_init(EGL_SURFACE_T *surface,
      const EGL_SURFACE_METHODS_T *fns,
      const EGL_CONFIG_T *config, const void *attrib_list,
      EGL_AttribType attrib_type,
      unsigned width, unsigned height,
      EGLNativeWindowType win, EGLNativePixmapType pix);

extern void egl_surface_base_destroy(EGL_SURFACE_T *surface);

/* Base get_attrib implementation */
extern EGLint egl_surface_base_get_attrib(EGL_SURFACE_T *surface,
      EGLint attrib, EGLint *value);

/* Base init_attrib implementation */
extern EGLint egl_surface_base_init_attrib(EGL_SURFACE_T *surface,
      EGLint attrib, EGLAttribKHR value);

/* Base set_attrib implementation */
extern EGLint egl_surface_base_set_attrib(EGL_SURFACE_T *surface,
      EGLint attrib, EGLAttribKHR value);

/* Assumes surface->config is already initialized. */
extern GFX_LFMT_T egl_surface_base_get_back_buffer_api_fmt(
      const EGL_SURFACE_T *surface);

/* Assumes surface->config is already initialized. */
extern GFX_LFMT_T egl_surface_base_get_aux_buffer_api_fmt(
      const EGL_SURFACE_T *surface, egl_aux_buf_t which);

/*
 * Check if the surface has been resized by the platform and reallocate
 * auxiliary buffers if necessary. Returns false for no memory.
 */
extern bool egl_surface_base_resize(EGL_SURFACE_T *surface,
      unsigned width, unsigned height);

extern khrn_image *egl_surface_base_get_aux_buffer(
      const EGL_SURFACE_T *surface, egl_aux_buf_t which);

extern void egl_surface_base_update_buffer_age_heuristics(
      EGL_SURFACE_T *surface, khrn_image *back_buffer, int age);

extern int egl_surface_base_query_buffer_age(EGL_SURFACE_T *surface);

#endif /* EGL_SURFACE_BASE_H */
