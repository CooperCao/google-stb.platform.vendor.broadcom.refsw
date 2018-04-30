/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "../common/khrn_process.h"
#include "../common/khrn_record.h"
#include "libs/util/common.h"
#include "../glxx/glxx_server.h"
#include "egl_context_gl.h"
#include "../glxx/glxx_hw.h"
#include "egl_display.h"
#include "egl_surface_base.h"

static VCOS_REENTRANT_MUTEX_T gl_lock;

#ifndef NDEBUG
static thread_local int locked_count;
#endif

struct egl_gl_context
{
   EGL_CONTEXT_T           base;
   gl_api_t                api;

   /* Note: you must have the gl_lock before touching anything in this */
   GLXX_SERVER_STATE_T     server;
};

static EGL_CONTEXT_METHODS_T fns;

/*
 * GL locking strategy:
 *
 * 1. The lock is obtained on all GL entry-points.
 * 2. The lock is obtained in all completion callbacks back into the GL.
 * 3. Submitting jobs to the scheduler does not need the GL lock.
 * 4. Because completion callbacks can come from anywhere, in particular from
 *    either inside or outside the GL, the GL lock has to be recursive.
 */


void egl_context_gl_create_lock(void)
{
   if (vcos_reentrant_mutex_create(&gl_lock, "GLXX") != VCOS_SUCCESS)
      exit(EXIT_FAILURE);
}

static void flush(EGL_CONTEXT_T *context, bool wait)
{
   assert(context->type == EGL_CONTEXT_TYPE_GL);
   EGL_GL_CONTEXT_T *ctx = (EGL_GL_CONTEXT_T *) context;

   if (ctx->base.valid)
   {
      egl_context_gl_lock();
      glxx_server_state_flush(&ctx->server, wait);
      egl_context_gl_unlock();
   }
}

static EGL_GL_CONTEXT_T *get_context(EGLContext ctx)
{
   EGL_CONTEXT_T *c = egl_get_context(ctx);

   if (c == NULL || c->api != API_OPENGL)
      return NULL;

   assert(c->valid); // must be valid to be in context map.
   return (EGL_GL_CONTEXT_T *)c;
}

/*
 * Find or create share_ctx. Either way return a counted reference to it.
 */
static EGLint get_share_context(GLXX_SHARED_T **ret, EGLContext share_ctx, bool reset_notification)
{
   if (share_ctx != EGL_NO_CONTEXT)
   {
      EGL_GL_CONTEXT_T *share_context = get_context(share_ctx);
      if (share_context)
      {
         if (egl_context_gl_notification(share_context) == reset_notification)
         {
            *ret = share_context->server.shared;
            khrn_mem_acquire(*ret);
            return EGL_SUCCESS;
         }
         else
         {
            *ret = NULL;
            return EGL_BAD_MATCH;
         }
      }

      *ret = NULL;
      return EGL_BAD_CONTEXT;
   }

   GLXX_SHARED_T* shared = KHRN_MEM_ALLOC_STRUCT(GLXX_SHARED_T);
   if (shared)
   {
      khrn_mem_set_term(shared, glxx_shared_term);

      egl_context_gl_assert_locked();

      if (glxx_shared_init(shared))
      {
         *ret = shared;
          return EGL_SUCCESS;
      }

      khrn_mem_release(*ret);
   }

   *ret = NULL;
   return EGL_BAD_ALLOC;
}

typedef struct
{
   gl_api_t    api;
   bool        (*server_state_init)(GLXX_SERVER_STATE_T *, GLXX_SHARED_T *);

}
CLIENT_API_T;

static EGLint extract_attribs(const EGLint *attrib_list, const CLIENT_API_T **api,
      bool *robustness, bool *reset_notification, bool *want_debug, bool *secure)
{
   static const CLIENT_API_T es1x_api = {OPENGL_ES_11, gl11_server_state_init};
   static const CLIENT_API_T es3x_api = {OPENGL_ES_3X, gl20_server_state_init};

   const EGLint supported_flags = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR |
                                  EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR;
   const int max_supported_minor[] = { -1, 1, 0, KHRN_GLES32_DRIVER ? 2 : V3D_VER_AT_LEAST(3,3,0,0) ? 1 : 0 };

   /* Default values */
   EGLint major = 1;
   EGLint minor = 0;
   EGLint flags = 0;
   *robustness = false;
   *reset_notification = false;

   while (attrib_list != NULL && *attrib_list != EGL_NONE)
   {
      EGLint name  = *attrib_list++;
      EGLint value = *attrib_list++;

      switch(name) {
         case EGL_CONTEXT_MAJOR_VERSION:
            major = value;
            break;
         case EGL_CONTEXT_MINOR_VERSION:
            minor = value;
            break;
         case EGL_CONTEXT_FLAGS_KHR:
            if (value & ~supported_flags) return EGL_BAD_ATTRIBUTE;
            flags = value;
            break;
         case EGL_CONTEXT_OPENGL_ROBUST_ACCESS:
         case EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT:      /* These EXT versions have different numbers */
            if (value == EGL_TRUE || value == EGL_FALSE) *robustness = value;
            else return EGL_BAD_ATTRIBUTE;
            break;
         case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY:
         case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT:
            if (value == EGL_NO_RESET_NOTIFICATION_EXT) *reset_notification = false;
            else if (value == EGL_LOSE_CONTEXT_ON_RESET_EXT) *reset_notification = true;
            else return EGL_BAD_ATTRIBUTE;
            break;
         case EGL_PROTECTED_CONTENT_EXT:
            if (value == EGL_TRUE || value == EGL_FALSE) *secure = value;
            else return EGL_BAD_ATTRIBUTE;
            break;
         default:
            return EGL_BAD_ATTRIBUTE;
      }
   }

   /* Check that the version requested is a valid version */
   if (major <= 0 || major > 3) return EGL_BAD_MATCH;
   if (minor <  0 || minor > max_supported_minor[major]) return EGL_BAD_MATCH;

   /* Debug is supported (although we do nothing, it counts), robustness is not */
   if (flags & EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR) return EGL_BAD_ATTRIBUTE;

   *want_debug = (flags & EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR) != 0;

   /* Major version 1 gets an ES1.x API, while 2 and 3 are promoted to ES3 */
   if (major == 1) *api = &es1x_api;
   else            *api = &es3x_api;
   return EGL_SUCCESS;
}

void egl_context_gl_lock(void)
{
   vcos_reentrant_mutex_lock(&gl_lock);
#ifndef NDEBUG
   locked_count += 1;
#endif
}

void egl_context_gl_unlock(void)
{
#ifndef NDEBUG
   locked_count -= 1;
#endif
   vcos_reentrant_mutex_unlock(&gl_lock);
}

#ifndef NDEBUG
void egl_context_gl_assert_locked(void)
{
   assert(locked_count > 0);
}
#endif

EGLint egl_context_gl_create(EGL_GL_CONTEXT_T **context, EGLDisplay dpy,
      EGLConfig config_in, EGLContext share_ctx, const EGLint *attrib_list)
{
   EGL_GL_CONTEXT_T *ret = NULL;
   EGLint error = EGL_BAD_ALLOC;
   GLXX_SHARED_T *shared = NULL;
   const CLIENT_API_T *client_api;
   GLXX_SERVER_STATE_T *server = NULL;
   bool locked = false;
   bool robustness;
   bool reset_notification;
   bool debug = false;
   bool secure = false;

   const EGL_CONFIG_T *config = egl_config_validate(config_in);
   if (!config)
   {
      error = EGL_BAD_CONFIG;
      goto end;
   }

   error = extract_attribs(attrib_list, &client_api, &robustness,
      &reset_notification, &debug, &secure);
   if (error != EGL_SUCCESS)
      goto end;

   ret = calloc(1, sizeof *ret);
   if (!ret) goto end;

   ret->base.fns = &fns;
   ret->base.type = EGL_CONTEXT_TYPE_GL;

   egl_context_gl_lock();
   locked = true;

   egl_context_base_init(&ret->base, dpy, API_OPENGL, config, debug, robustness,
         reset_notification, secure);

   ret->api = client_api->api;
   ret->server.context = ret;

   error = get_share_context(&shared, share_ctx, reset_notification);
   if (error != EGL_SUCCESS) goto end;

   if (!client_api->server_state_init(&ret->server, shared))
      goto end;

   server = &ret->server;
   ret->base.valid = true;

   server->khr_debug.debug_output = debug;

   error = EGL_SUCCESS;
end:
   khrn_mem_release(shared);  /* It belongs to the server now */
   if (error != EGL_SUCCESS)
   {
      glxx_server_state_destroy(server);
      if (ret)
         egl_context_base_term(&ret->base);
      free(ret);
      ret = NULL;
   }

   if (locked)
      egl_context_gl_unlock();

   *context = ret;
   return error;
}

bool convert_image(EGL_CONTEXT_T *context, khrn_image *dst, khrn_image *src)
{
   egl_context_gl_lock();
   GLXX_SERVER_STATE_T *state = &((EGL_GL_CONTEXT_T *)context)->server;
   bool ok = khrn_image_convert(dst, src, &state->fences, context->secure);
   egl_context_gl_unlock();
   return ok;
}

void invalidate_draw(EGL_CONTEXT_T *context,
   bool color, bool color_ms, bool other_aux)
{
   egl_context_gl_lock();
   GLXX_SERVER_STATE_T *state = &((EGL_GL_CONTEXT_T *)context)->server;
   glxx_hw_invalidate_default_draw_framebuffer(state, color, color_ms, other_aux, other_aux);
   egl_context_gl_unlock();
}

static EGL_GL_CONTEXT_T *current_context(void)
{
   EGL_THREAD_T *thread = egl_thread_get();
   egl_api_t api = thread->current_api;

   assert(api >= 0 && api < API_COUNT);
   EGL_CONTEXT_T *context = thread->contexts[api];
   if (!context || context->api != API_OPENGL || !context->valid)
      return NULL;

   return (EGL_GL_CONTEXT_T *)context;
}

GLXX_SERVER_STATE_T *egl_context_gl_server_state(EGL_GL_CONTEXT_T *context)
{
   egl_context_gl_assert_locked();

   if (!context && !(context = current_context()))
      return NULL;

   return &context->server;
}

gl_api_t egl_context_gl_api(const EGL_GL_CONTEXT_T *context, gl_api_t apis)
{
   if (!context && !(context = current_context()))
      return OPENGL_ES_NONE;

   return context->api & apis;
}

bool egl_context_gl_debug(const EGL_GL_CONTEXT_T *context)
{
   if (!context && !(context = current_context()))
      return false;

   return context->base.debug;
}

bool egl_context_gl_secure(const EGL_GL_CONTEXT_T *context)
{
   if (!context && !(context = current_context()))
      return false;

   return context->base.secure;
}

bool egl_context_gl_robustness(const EGL_GL_CONTEXT_T *context)
{
   if (!context && !(context = current_context()))
      return EGL_FALSE;

   return context->base.robustness;
}

bool egl_context_gl_notification(const EGL_GL_CONTEXT_T *context)
{
   if (!context && !(context = current_context()))
      return false;

   return context->base.reset_notification;
}

bool egl_context_gl_reset_notified()
{
   EGL_GL_CONTEXT_T *context = current_context();
   if (!context)
      return false;

   return context->base.gpu_aborted_notified;
}

void egl_context_gl_set_reset_notified()
{
   EGL_GL_CONTEXT_T *context = current_context();
   if (!context)
      return;

   context->base.gpu_aborted_notified = true;
}

static void attach(EGL_CONTEXT_T *context, EGL_SURFACE_T *draw,
      EGL_SURFACE_T *read)
{
   assert(context->type == EGL_CONTEXT_TYPE_GL);
   EGL_GL_CONTEXT_T *ctx = (EGL_GL_CONTEXT_T *) context;

   egl_context_base_attach(context, draw, read);

   if (ctx->base.valid)
   {
      egl_context_gl_lock();
      glxx_server_attach_surfaces(&ctx->server, read, draw);
      egl_context_gl_unlock();
   }
}

static void detach(EGL_CONTEXT_T *context)
{
   assert(context->type == EGL_CONTEXT_TYPE_GL);
   EGL_GL_CONTEXT_T *ctx = (EGL_GL_CONTEXT_T *) context;

   if (ctx->base.valid)
   {
      egl_context_gl_lock();
      glxx_server_detach_surfaces(&ctx->server);
      egl_context_gl_unlock();
   }

   egl_context_base_detach(context);
}

static int client_version(const EGL_CONTEXT_T *context)
{
   EGL_GL_CONTEXT_T *ctx = (EGL_GL_CONTEXT_T *) context;

   assert(context->type == EGL_CONTEXT_TYPE_GL);

   switch (ctx->api)
   {
   case OPENGL_ES_11:
      return 1;
   case OPENGL_ES_3X:
      return 3;
   default:
      break;
   }

   assert(0);
   return 0;
}

static void term(EGL_CONTEXT_T *context)
{
   assert(context->type == EGL_CONTEXT_TYPE_GL);
   EGL_GL_CONTEXT_T *ctx = (EGL_GL_CONTEXT_T *) context;

   // khrn_record calls exit(0) with the driver in an undefined state
   // so don't both attempting a clean shutdown
   if (ctx->base.valid)
   {
      ctx->base.valid = false;
      egl_context_gl_lock();
      glxx_server_state_destroy(&ctx->server);
      egl_context_gl_unlock();
      egl_context_base_term(&ctx->base);
   }
}

static EGL_CONTEXT_METHODS_T fns =
{
   flush,
   convert_image,
   invalidate_draw,
   attach,
   detach,
   client_version,
   term
};
