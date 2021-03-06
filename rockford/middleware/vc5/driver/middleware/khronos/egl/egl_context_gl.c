/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "vcos.h"
#include "middleware/khronos/common/khrn_process.h"
#include "middleware/khronos/common/khrn_record.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/egl/egl_context_gl.h"
#include "middleware/khronos/glxx/glxx_hw.h"
#include "middleware/khronos/egl/egl_display.h"
#include "middleware/khronos/egl/egl_surface_base.h"

static VCOS_REENTRANT_MUTEX_T gl_lock;
static bool gl_lock_created = false;
static VCOS_ONCE_T gl_once = VCOS_ONCE_INIT;

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
static void init_lock(void)
{
   if (vcos_reentrant_mutex_create(&gl_lock, "GLXX") != VCOS_SUCCESS)
      exit(EXIT_FAILURE);
   gl_lock_created = true;
}

static void flush(EGL_CONTEXT_T *context, bool wait)
{
   assert(context->type == EGL_CONTEXT_TYPE_GL);
   EGL_GL_CONTEXT_T *ctx = (EGL_GL_CONTEXT_T *) context;

   if (ctx->base.valid)
   {
      if (egl_context_gl_lock())
      {
         glxx_server_state_flush(&ctx->server, wait);
         egl_context_gl_unlock();
      }
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
static EGLint get_share_context(GLXX_SHARED_T **ret, EGLContext share_ctx)
{
   if (share_ctx != EGL_NO_CONTEXT)
   {
      EGL_GL_CONTEXT_T *share_context = get_context(share_ctx);
      if (share_context)
      {
         *ret = share_context->server.shared;
         khrn_mem_acquire(*ret);
         return EGL_SUCCESS;
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
      bool *robustness, bool *reset_notification, bool *want_debug)
{
   static const CLIENT_API_T es1x_api = {OPENGL_ES_11, gl11_server_state_init};
   static const CLIENT_API_T es3x_api =
#if KHRN_GLES31_DRIVER
      {OPENGL_ES_31, gl20_server_state_init};
#else
      {OPENGL_ES_30, gl20_server_state_init};
#endif

   const EGLint supported_flags = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR |
                                  EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR;
   const int max_supported_minor[] = { -1, 1, 0, KHRN_GLES31_DRIVER ? 1 : 0 };

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
         case EGL_CONTEXT_MAJOR_VERSION_KHR:
            if (value > 0 && value <= 3) major = value;
            else return EGL_BAD_MATCH;
            break;
         case EGL_CONTEXT_MINOR_VERSION_KHR:
         {
            if (value >= 0 && value <= 1) minor = value;
            else return EGL_BAD_MATCH;
            break;
         }
         case EGL_CONTEXT_FLAGS_KHR:
            if (value & ~supported_flags) return EGL_BAD_ATTRIBUTE;
            flags = value;
            break;
#if EGL_EXT_create_context_robustness
         case EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT:
            if (value == EGL_TRUE || value == EGL_FALSE) *robustness = value;
            else return EGL_BAD_ATTRIBUTE;
            break;
         case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT:
            if (value == EGL_NO_RESET_NOTIFICATION_EXT) *reset_notification = false;
            else if (value == EGL_LOSE_CONTEXT_ON_RESET_EXT) *reset_notification = true;
            else return EGL_BAD_ATTRIBUTE;
            break;
#endif
         default:
            return EGL_BAD_ATTRIBUTE;
      }
   }

   /* Check that the version requested is a valid version */
   if (minor > max_supported_minor[major]) return EGL_BAD_MATCH;

   /* Debug is supported (although we do nothing, it counts), robustness is not */
   if (flags & EGL_CONTEXT_OPENGL_ROBUST_ACCESS_BIT_KHR) return EGL_BAD_ATTRIBUTE;

   *want_debug = (flags & EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR) != 0;

   /* Major version 1 gets an ES1.x API, while 2 and 3 are promoted to ES3 */
   if (major == 1) *api = &es1x_api;
   else            *api = &es3x_api;
   return EGL_SUCCESS;
}

bool egl_context_gl_lock(void)
{
   if (gl_lock_created)
   {
      vcos_reentrant_mutex_lock(&gl_lock);
#ifndef NDEBUG
      EGL_THREAD_T *thread = egl_thread_get();
      thread->locked_count++;
#endif
   }
   return gl_lock_created;
}

void egl_context_gl_unlock(void)
{
#ifndef NDEBUG
   EGL_THREAD_T *thread = egl_thread_get();
   thread->locked_count--;
   assert(thread->locked_count >= 0);
#endif
   vcos_reentrant_mutex_unlock(&gl_lock);
}

#ifndef NDEBUG
void egl_context_gl_assert_locked(void)
{
   const EGL_THREAD_T *thread = egl_thread_get();
   assert(thread->locked_count > 0);
}
#endif

EGLint egl_context_gl_create(EGL_GL_CONTEXT_T **context, EGLConfig config,
   EGLContext share_ctx, const EGLint *attrib_list)
{
   EGL_GL_CONTEXT_T *ret = NULL;
   EGLint error = EGL_BAD_ALLOC;
   GLXX_SHARED_T *shared = NULL;
   const CLIENT_API_T *client_api;
   GLXX_SERVER_STATE_T *server = NULL;
   bool locked = false;
   bool robustness;
   bool reset_notification;
   bool want_debug = false;

   if (vcos_once(&gl_once, init_lock))
      goto end;

   if (!egl_config_is_valid(config))
   {
      error = EGL_BAD_CONFIG;
      goto end;
   }

   error = extract_attribs(attrib_list, &client_api, &robustness, &reset_notification, &want_debug);
   if (error != EGL_SUCCESS)
      goto end;

   ret = calloc(1, sizeof *ret);
   if (!ret) goto end;

   ret->base.fns = &fns;
   ret->base.type = EGL_CONTEXT_TYPE_GL;

   if (!egl_context_gl_lock())
      goto end;
   locked = true;

   egl_context_base_init(&ret->base, API_OPENGL, config, robustness, reset_notification);

   ret->api = client_api->api;
   ret->server.context = ret;

   error = get_share_context(&shared, share_ctx);
   if (error != EGL_SUCCESS) goto end;

   if (!client_api->server_state_init(&ret->server, shared))
      goto end;

   server = &ret->server;
   ret->base.valid = true;

   server->khr_debug.debug_output = want_debug;

   error = EGL_SUCCESS;
end:
   khrn_mem_release(shared);  /* It belongs to the server now */
   if (error != EGL_SUCCESS)
   {
      glxx_server_state_destroy(server);
      free(ret);
      ret = NULL;
   }

   if (locked)
      egl_context_gl_unlock();

   *context = ret;
   return error;
}

static v3d_scheduler_deps* flush_rendering(EGL_CONTEXT_T *context,
      EGL_SURFACE_T *surface)
{
   v3d_scheduler_deps *deps = NULL;

   assert(surface->context == context);

   if (egl_context_gl_lock())
   {
      KHRN_IMAGE_T *image = egl_surface_get_back_buffer(surface);
      if (image)
      {
         KHRN_RES_INTERLOCK_T *res_i = khrn_image_get_res_interlock(image);

         if (surface->context && (surface->context == egl_thread_get_context()) &&
            surface->context->draw == surface)
         {
            /* if this is the current draw surface, then default fb is using the
             * surface's back buffer, record that aux buffers become invalid after
             * this op, so we can save allocating/storing these */
            GLXX_SERVER_STATE_T *gl_state;
            gl_state = egl_context_gl_server_state((EGL_GL_CONTEXT_T*)surface->context);
            assert(gl_state);
            glxx_invalidate_default_draw_framebuffer(gl_state, false /* color*/,
               true /* multisample */, true /* depth */, true /* stencil */,
               NULL);
         }

         if (khrn_interlock_is_valid(&res_i->interlock))
            khrn_interlock_flush(&res_i->interlock);
         deps = khrn_interlock_get_sync(&res_i->interlock, false);
      }

      egl_context_gl_unlock();
   }
   return deps;
}

bool copy_surface(EGL_CONTEXT_T *context, EGL_SURFACE_T *surface, KHRN_IMAGE_T *dst)
{
   if (egl_context_gl_lock())
   {
      assert(surface->context == context);
      KHRN_IMAGE_T *src = egl_surface_get_back_buffer(surface);

      /* this is called from glCopyBuffer; the spec (egl1.4 and 1.5) says
       * that the surface must be bound to the calling thread's current context
       * and we report an error if this is not the case;
       * we need a context here so we record  the khrn_image copy operation on
       * the correct context */
      assert(surface->context && (surface->context == egl_thread_get_context()));
      GLXX_SERVER_STATE_T *gl_state;
      gl_state = egl_context_gl_server_state((EGL_GL_CONTEXT_T*)surface->context);
      assert(gl_state);

      if (surface->context->draw == surface)
      {
         /* copybuffers causes resolution of the ms color buffer to color buffer,
         * invalidate ms so we do not allocate/store multisample */
         glxx_invalidate_default_draw_framebuffer(gl_state, false /* color*/,
            true /* multisample */, false /* depth */, false /* stencil */,
            NULL);
      }
      bool ok = khrn_image_copy(dst, src, &gl_state->fences);
      egl_context_gl_unlock();

      return ok;
   }
   else
      return false;
}

static void add_fence(EGL_CONTEXT_T *context,
      const EGL_SURFACE_T *surface, int fence)
{
   KHRN_IMAGE_T *image;
   KHRN_RES_INTERLOCK_T *res_i = NULL;
   uint64_t job_id = v3d_scheduler_submit_wait_fence(fence);

   assert(surface->context == context);

   if (egl_context_gl_lock())
   {
      image = egl_surface_get_back_buffer(surface);
      res_i = khrn_image_get_res_interlock(image);
      khrn_interlock_job_add(&res_i->interlock, job_id, true);

      egl_context_gl_unlock();
   }
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

EGLBoolean egl_context_gl_robustness(const EGL_GL_CONTEXT_T *context)
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

static void attach(EGL_CONTEXT_T *context, EGL_SURFACE_T *draw,
      EGL_SURFACE_T *read)
{
   assert(context->type == EGL_CONTEXT_TYPE_GL);
   EGL_GL_CONTEXT_T *ctx = (EGL_GL_CONTEXT_T *) context;

   egl_context_base_attach(context, draw, read);

   if (ctx->base.valid)
   {
      if (egl_context_gl_lock())
      {
         glxx_server_attach_surfaces(&ctx->server, read, draw);
         egl_context_gl_unlock();
      }
   }
}

static void detach(EGL_CONTEXT_T *context)
{
   assert(context->type == EGL_CONTEXT_TYPE_GL);
   EGL_GL_CONTEXT_T *ctx = (EGL_GL_CONTEXT_T *) context;

   if (ctx->base.valid)
   {
      if (egl_context_gl_lock())
      {
         glxx_server_detach_surfaces(&ctx->server);
         egl_context_gl_unlock();
      }
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
   case OPENGL_ES_30:
   case OPENGL_ES_31:
      return 3;
   default:
      break;
   }

   assert(0);
   return 0;
}

static void invalidate(EGL_CONTEXT_T *context)
{
   assert(context->type == EGL_CONTEXT_TYPE_GL);
   EGL_GL_CONTEXT_T *ctx = (EGL_GL_CONTEXT_T *) context;

   // khrn_record calls exit(0) with the driver in an undefined state
   // so don't both attempting a clean shutdown
   if (ctx->base.valid)
   {
      ctx->base.valid = false;
      if (egl_context_gl_lock())
      {
         glxx_server_state_destroy(&ctx->server);
         egl_context_gl_unlock();
      }
   }
}

static EGL_CONTEXT_METHODS_T fns =
{
   flush,
   flush_rendering,
   copy_surface,
   add_fence,
   attach,
   detach,
   client_version,
   invalidate
};
