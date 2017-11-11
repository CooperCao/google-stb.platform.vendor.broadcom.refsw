/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_CONTEXT_GL_H
#define EGL_CONTEXT_GL_H

#include "egl_context_base.h"
#include "../glxx/glxx_server_state.h"

typedef enum
{
   OPENGL_ES_NONE = 0,
   OPENGL_ES_11   = 1 << 0,
   OPENGL_ES_3X   = 1 << 1,
   OPENGL_ES_ANY  = OPENGL_ES_11 | OPENGL_ES_3X,
}
gl_api_t;

/*
 * GL contexts are not thread-safe at all, so there is a global per-process
 * lock. Lock/unlock it with these functions.
 * The lock is created at the initialisation of the egl_process
 */
extern void egl_context_gl_create_lock(void);
extern void egl_context_gl_lock(void);
extern void egl_context_gl_unlock(void);

#ifndef NDEBUG
extern void egl_context_gl_assert_locked(void);
#else
#  define egl_context_gl_assert_locked() do {} while (0)
#endif

extern EGLint egl_context_gl_create(EGL_GL_CONTEXT_T **context, EGLConfig config,
      EGLContext share_ctx, const EGLint *attrib_list);

/*
 * If context is NULL, the current context from the current thread is used if
 * it's a GL context (if not NULL is returned). Requires you to have the lock.
 */
extern GLXX_SERVER_STATE_T *egl_context_gl_server_state(
      EGL_GL_CONTEXT_T *context);

/*
 * Returns context->gl_api & apis. If context is NULL, the current context from
 * the current thread is used provided it's a GL context (or OPENGL_ES_NONE is
 * returned)
 */
extern gl_api_t egl_context_gl_api(const EGL_GL_CONTEXT_T *context,
      gl_api_t apis);

/* Returns whether the context was created as a secure context */
extern bool egl_context_gl_secure(const EGL_GL_CONTEXT_T *context);

/* Returns whether the context was created as a robust context */
extern bool egl_context_gl_robustness(const EGL_GL_CONTEXT_T *context);

/* Returns whether the context was created as a debug context */
extern bool egl_context_gl_debug(const EGL_GL_CONTEXT_T *context);

/*
 * Returns GL versions of context->base.notification
 */
extern bool egl_context_gl_notification(const EGL_GL_CONTEXT_T *context);

/*
 * Set the reset notified to true (on the current context)
 */
extern void egl_context_gl_set_reset_notified();

/*
 * Returns true if the reset notification has already been returned
 * (on the current context)
 */
extern bool egl_context_gl_reset_notified();

#endif /* EGL_CONTEXT_GL_H */
