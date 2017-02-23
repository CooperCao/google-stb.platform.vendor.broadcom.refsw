/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "vcos.h"
#include "egl_context_base.h"
#include "egl_surface_base.h"

void egl_context_base_init(EGL_CONTEXT_T *context,
      egl_api_t api, EGLConfig config,
      bool robustness, bool reset_notification,
      bool secure)
{
   context->api = api;
   context->config = config;
   context->robustness = robustness;
   context->reset_notification = reset_notification;
   context->secure = secure;
}

void egl_context_base_attach(EGL_CONTEXT_T *context,
      EGL_SURFACE_T *draw, EGL_SURFACE_T *read)
{
   context->draw = draw;
   context->read = read;

   if (draw)
      draw->context = context;

   if (read)
      read->context = context;
}

void egl_context_base_detach(EGL_CONTEXT_T *context)
{
   if (context->draw)
   {
      context->draw->context = NULL;
      egl_surface_try_delete(context->draw);
   }

   if (context->read && context->read != context->draw)
   {
      context->read->context = NULL;
      egl_surface_try_delete(context->read);
   }

   context->draw = NULL;
   context->read = NULL;
   context->bound_thread = NULL;
   egl_context_try_delete(context);
}
