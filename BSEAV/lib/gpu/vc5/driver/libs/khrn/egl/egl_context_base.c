/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "egl_context_base.h"
#include "egl_surface_base.h"
#include "egl_display.h"

void egl_context_base_init(EGL_CONTEXT_T *context, EGLDisplay dpy,
      egl_api_t api, const EGL_CONFIG_T *config, bool debug,
      bool robustness, bool reset_notification,
      bool secure)
{
   egl_display_refinc(dpy);

   context->display = dpy;
   context->api = api;
   context->config = config;
   context->debug = debug;
   context->robustness = robustness;
   context->reset_notification = reset_notification;
   context->secure = secure;
}

void egl_context_base_term(EGL_CONTEXT_T *context)
{
   egl_display_refdec(context->display);
   context->display = EGL_NO_DISPLAY;
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
