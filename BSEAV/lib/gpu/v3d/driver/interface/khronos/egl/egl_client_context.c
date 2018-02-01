/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"

#include "interface/khronos/egl/egl_client_context.h"
#include "interface/khronos/egl/egl_client_surface.h"
#include "interface/khronos/egl/egl_int_impl.h"
#include "middleware/khronos/common/khrn_mem.h"

#include <string.h>
#include <stdlib.h>

EGLint egl_context_check_attribs(const EGLint *attrib_list, EGLint max_version, EGLint *version, bool *secure)
{
   if (!attrib_list)
      return EGL_SUCCESS;

   while (1) {
      switch (*attrib_list++) {
      case EGL_CONTEXT_CLIENT_VERSION:
      {
         EGLint value = *attrib_list++;

         if (value < 1 || value > max_version)
            return EGL_BAD_CONFIG;
         else
            *version = value;

         break;
      }
      case EGL_NONE:
         return EGL_SUCCESS;
#if EGL_EXT_protected_content
      case EGL_PROTECTED_CONTENT_EXT:
      {
         EGLint value = *attrib_list++;

         if (value != EGL_TRUE && value != EGL_FALSE)
            return EGL_BAD_ATTRIBUTE;
         else
            *secure = value == EGL_TRUE;

         break;
      }
#endif
      default:
         return EGL_BAD_ATTRIBUTE;
      }
   }
}

static void egl_context_term(void *p)
{
   EGL_CONTEXT_T *context = p;
   /* If we're current then there should still be a reference to us */
   /* (if this wasn't the case we should call egl_context_release_surfaces here) */
   KHRN_MEM_ASSIGN(context->state, NULL);
}

EGL_CONTEXT_T *egl_context_create(EGL_CONTEXT_T *share_context, EGLContext name, EGLDisplay display,
   EGLConfig config, EGL_CONTEXT_TYPE_T type, bool secure)
{
   EGL_CONTEXT_T *context = KHRN_MEM_ALLOC_STRUCT(EGL_CONTEXT_T);

   if (context == NULL)
      return NULL;

   khrn_mem_set_term(context, egl_context_term);

   context->name = name;
   context->display = display;
   context->config = config;

   context->type = type;

   context->renderbuffer = EGL_NONE;

   context->state = NULL;

   switch (type) {
   case OPENGL_ES_11:
   case OPENGL_ES_20:
   {
      context->state = egl_create_glxx_server_state(share_context ? share_context->state : NULL,
         share_context ? share_context->type : type, secure);
      if (context->state == NULL)
         KHRN_MEM_ASSIGN(context, NULL);
      break;
   }
   default:
      UNREACHABLE();
      break;
   }

   return context;
}

EGLBoolean egl_context_get_attrib(EGL_CONTEXT_T *context, EGLint attrib, EGLint *value)
{
   switch (attrib) {
   case EGL_CONFIG_ID:
      *value = (int)(intptr_t)context->config;
      return EGL_TRUE;
   case EGL_CONTEXT_CLIENT_TYPE:
      switch (context->type) {
      case OPENGL_ES_11:
      case OPENGL_ES_20:
         *value = EGL_OPENGL_ES_API;
         break;
      default:
         UNREACHABLE();
         break;
      }
      return EGL_TRUE;
   case EGL_CONTEXT_CLIENT_VERSION:
      switch (context->type) {
      case OPENGL_ES_11:
         *value = 1;
         break;
      case OPENGL_ES_20:
         *value = 2;
         break;
      default:
         UNREACHABLE();
         break;
      }
      return EGL_TRUE;
   case EGL_RENDER_BUFFER:
   {
      /* TODO: GLES supposedly doesn't support single-buffered rendering. Should we take this into account? */
      *value = context->renderbuffer;
      return EGL_TRUE;
   }
   default:
      return EGL_FALSE;
   }
}
