/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_TYPES_H
#define EGL_TYPES_H
#include <EGL/egl.h>

/*
 * This doesn't distinguish between different versions of GL apis (because nor
 * does most of EGL). There is a gl_api_t in an EGL_GL_CONTEXT_T to do that.
 * Note, these are used as array indexes so resist the temptation to give them
 * funny values.
 */
typedef enum
{
   API_OPENGL,
   API_OPENVG,
   API_COUNT,
}
egl_api_t;

typedef struct egl_display EGL_DISPLAY_T;
typedef struct egl_map EGL_MAP_T;
typedef struct egl_thread EGL_THREAD_T;

typedef struct egl_surface_base EGL_SURFACE_T;
typedef enum egl_aux_buf egl_aux_buf_t;
typedef enum egl_surface_type egl_surface_type_t;

/* Actual definition of egl_window_surface and egl_pixmap_surface comes from the platform */
typedef struct egl_window_surface EGL_WINDOW_SURFACE_T;
typedef struct egl_pixmap_surface EGL_PIXMAP_SURFACE_T;

typedef struct egl_context_base EGL_CONTEXT_T;
typedef struct egl_gl_context EGL_GL_CONTEXT_T;
typedef enum egl_context_type egl_context_type_t;

typedef struct egl_image EGL_IMAGE_T;
typedef struct egl_platform_fns EGL_PLATFORM_FNS_T;

typedef struct egl_sync EGL_SYNC_T;

#endif /* EGL_TYPES_H */
