/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_context.h"
#include "bsg_exception.h"
#include "bsg_application_options.h"
#include "bsg_platform.h"

#include <stdio.h>
#include <vector>

namespace bsg
{

Context::Context() :
   m_display(EGL_NO_DISPLAY),
   m_surface(EGL_NO_SURFACE),
   m_context(EGL_NO_CONTEXT)
{

}

Context::~Context()
{

}


static void ColorBits(uint32_t *r, uint32_t *g, uint32_t *b, uint32_t *a, int bpp, int alphaBits)
{
   *r = 0;
   *g = 0;
   *b = 0;
   *a = 0;

   if (bpp == 16)
   {
      if (alphaBits == 0)
      {
         *r = 5;
         *g = 6;
         *b = 5;
         *a = 0;
      }
      else if (alphaBits == 1)
      {
         *r = 5;
         *g = 5;
         *b = 5;
         *a = 1;
      }
      else if (alphaBits > 1)
      {
         *r = 4;
         *g = 4;
         *b = 4;
         *a = 4;
      }
   }
   else if (bpp > 16)
   {
      *r = 8;
      *g = 8;
      *b = 8;
      if (bpp == 32)
         *a = 8;
      else
         *a = 0;
   }
}


EGLConfig Context::ChooseConfigForDisplay(EGLDisplay dpy, const ApplicationOptions &options)
{
   m_display = dpy;

   /*
      Step 2 - Initialize EGL.
      EGL has to be initialized with the display obtained in the
      previous step. We cannot use other EGL functions except
      eglGetDisplay and eglGetError before eglInitialize has been
      called.
      If we're not interested in the EGL version number we can just
      pass NULL for the second and third parameters.
   */
   EGLint     majorVersion;
   EGLint     minorVersion;

   if (!eglInitialize(m_display, &majorVersion, &minorVersion))
      BSG_THROW("eglInitialize() failed");

   /*
    Bind the API used by the application
   */
   if (options.GetApiVersion() == eOPVG_1_1)
   {
      if (!eglBindAPI(EGL_OPENVG_API))
      {
         BSG_THROW("eglBindAPI(EGL_OPENVG_API) failed\n");
      }
   }

   /*
      Step 3 - Specify the required configuration attributes.
      An EGL "configuration" describes the pixel format and type of
      surfaces that can be used for drawing.
      For now we just want to use a 16 bit RGB surface that is a
      Window surface, i.e. it will be visible on screen. The list
      has to contain key/value pairs, terminated with EGL_NONE.
   */

   uint32_t bpp = options.GetColorBits();
   uint32_t alpha = options.GetAlphaBits();
   uint32_t r, g, b, a;

   ColorBits(&r, &g, &b, &a, bpp, alpha);

   std::vector<EGLint>     confAttribs;

   confAttribs.push_back(EGL_RED_SIZE);
   confAttribs.push_back(r);
   confAttribs.push_back(EGL_GREEN_SIZE);
   confAttribs.push_back(g);
   confAttribs.push_back(EGL_BLUE_SIZE);
   confAttribs.push_back(b);
   confAttribs.push_back(EGL_ALPHA_SIZE);
   confAttribs.push_back(a);

   if (options.GetApiVersion() == eOPVG_1_1)
   {
      uint32_t depthBits = options.GetDepthBits();
      if ((depthBits > 0) && (depthBits <= 8))
      {
         confAttribs.push_back(EGL_ALPHA_MASK_SIZE);
         confAttribs.push_back(options.GetDepthBits());
      }
   }
   else
   {
      confAttribs.push_back(EGL_DEPTH_SIZE);
      confAttribs.push_back(options.GetDepthBits());

      confAttribs.push_back(EGL_STENCIL_SIZE);
      confAttribs.push_back(options.GetStencilBits());
   }

   if (options.GetMultisample())
   {
      confAttribs.push_back(EGL_SAMPLE_BUFFERS);
      confAttribs.push_back(1);
      confAttribs.push_back(EGL_SAMPLES);
      confAttribs.push_back(4);
   }

   confAttribs.push_back(EGL_SURFACE_TYPE);

   EGLint surfaceBits = EGL_WINDOW_BIT;

#ifndef EMULATED
   if (options.GetPreserve())
      surfaceBits |= EGL_SWAP_BEHAVIOR_PRESERVED_BIT;
#endif
   confAttribs.push_back(surfaceBits);

   confAttribs.push_back(EGL_RENDERABLE_TYPE);

   if (options.GetApiVersion() == eGLES_2_0)
      confAttribs.push_back(EGL_OPENGL_ES2_BIT);
   else if (options.GetApiVersion() == eGLES_3_0)
      confAttribs.push_back(EGL_OPENGL_ES2_BIT);
   else if (options.GetApiVersion() == eGLES_1_1)
      confAttribs.push_back(EGL_OPENGL_ES_BIT);
   else if (options.GetApiVersion() == eOPVG_1_1)
      confAttribs.push_back(EGL_OPENVG_BIT);
   confAttribs.push_back(EGL_NONE);

   /*
      Step 3.5 - Get the number of configurations to correctly size the array
      used in step 4
   */
   int        numConfigs;

   if (!eglGetConfigs(m_display, NULL, 0, &numConfigs))
      BSG_THROW("eglGetConfigs() failed");

   std::vector<EGLConfig> eglConfig(numConfigs);

   /*
      Step 4 - Find a config that matches all requirements.
      eglChooseConfig provides a list of all available configurations
      that meet or exceed the requirements given as the second
      argument.
   */
   if (!eglChooseConfig(m_display, &confAttribs[0], &eglConfig[0], numConfigs, &numConfigs) || (numConfigs == 0))
      BSG_THROW("eglChooseConfig() failed");

   int i;

   for (i = 0; i < numConfigs; i++)
   {
      /*
         From the EGL spec - 3.4.1
         This rule places configs with deeper color buffers first in the list returned by eglChooseConfig.
         Applications may find this counterintuitive, and need to perform additional processing on the list of
         configs to find one best matching their requirements. For example, specifying RGBA depths of 5651
         could return a list whose first config has a depth of 8888.
      */

      /* We asked for a 565, but all the 8888 match first, we need to pick an actual 565 buffer */

      /*
         Step 4.5 - Check the output.
         Just my debug, I had a problem with the PVR ES2.0 emulator which didn't allow me to
         specify EGL_RED_SIZE or the config failed.  I removed the offending lines and use
         the code below to check the defaults
      */
      EGLint red_size;
      EGLint green_size;
      EGLint blue_size;
      EGLint alpha_size;
      EGLint depth_size;
      EGLint stencil_size;
      EGLint conformant;

      eglGetConfigAttrib(m_display, eglConfig[i], EGL_RED_SIZE,         &red_size);
      eglGetConfigAttrib(m_display, eglConfig[i], EGL_GREEN_SIZE,       &green_size);
      eglGetConfigAttrib(m_display, eglConfig[i], EGL_BLUE_SIZE,        &blue_size);
      eglGetConfigAttrib(m_display, eglConfig[i], EGL_ALPHA_SIZE,       &alpha_size);
      eglGetConfigAttrib(m_display, eglConfig[i], EGL_DEPTH_SIZE,       &depth_size);
      eglGetConfigAttrib(m_display, eglConfig[i], EGL_STENCIL_SIZE,     &stencil_size);
      eglGetConfigAttrib(m_display, eglConfig[i], EGL_CONFORMANT,       &conformant);

      // Does this exactly match the color bits that we requested and conformance?
      if (red_size == (EGLint)r &&
          green_size == (EGLint)g &&
          blue_size == (EGLint)b &&
          alpha_size == (EGLint)a &&
          !!conformant == options.GetConformant())
         break;
   }

   // non conformant configs may not exactly match, so just take whatever
   if (i >= numConfigs)
   {
      for (i = 0; i < numConfigs; i++)
      {
         EGLint conformant;
         eglGetConfigAttrib(m_display, eglConfig[i], EGL_CONFORMANT, &conformant);
         if (!!conformant == options.GetConformant())
            break;
      }
   }

   return eglConfig[i];
}

void Context::InitializeFromNativeWindow(const ApplicationOptions &options, EGLConfig config, EGLNativeWindowType nativeWindow)
{
   /*
      Step 5 - Create a surface to draw to.
      Use the config picked in the previous step and the native window
      handle when available to create a window surface. A window surface
      is one that will be visible on screen inside the native display (or
      fullscreen if there is no windowing system).
      Pixmaps and pbuffers are surfaces which only exist in off-screen
      memory.
   */
   std::vector<EGLint>     ctxAttribList;
   if (options.GetPreserve())
   {
      ctxAttribList.push_back(EGL_SWAP_BEHAVIOR);
      ctxAttribList.push_back(EGL_BUFFER_PRESERVED);
   }
   if (options.GetSecure())
   {
      ctxAttribList.push_back(EGL_PROTECTED_CONTENT_EXT);
      ctxAttribList.push_back(EGL_TRUE);
   }
   ctxAttribList.push_back(EGL_NONE);

   m_surface = eglCreateWindowSurface(m_display, config, nativeWindow, &ctxAttribList[0]);

   if (options.GetPreserve())
      eglSurfaceAttrib(m_display, m_surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);

   InitializeCreateContext(options, config);
}

void Context::InitializeWithPixmap(const ApplicationOptions &options, EGLConfig config, NativePixmap *nativePixmap)
{
   /*
      Step 5 - Create a surface to draw to.
      Use the config picked in the previous step and the native window
      handle when available to create a window surface. A window surface
      is one that will be visible on screen inside the native display (or
      fullscreen if there is no windowing system).
      Pixmaps and pbuffers are surfaces which only exist in off-screen
      memory.
   */

   m_surface = eglCreatePixmapSurface(m_display, config, nativePixmap->EGLPixmap(), NULL);

   InitializeCreateContext(options, config);
}

static GLint GetESApiVersion(const ApplicationOptions &options)
{
   GLint version = 0;

   switch (options.GetApiVersion())
   {
   case eGLES_1_1:
      version = 1;
      break;
   case eGLES_2_0:
      version = 2;
      break;
   case eGLES_3_0:
      version = 3;
      break;
   default:
      break;
   }

   return version;
}

void Context::InitializeCreateContext(const ApplicationOptions &options, EGLConfig config)
{
   /*
      Step 6 - Create a context.
      EGL has to create a context for OpenGL ES. Our OpenGL ES resources
      like textures will only be valid inside this context
      (or shared contexts)
   */
   std::vector<EGLint>     ctxAttribList;

   if (options.GetSecure())
   {
      ctxAttribList.push_back(EGL_PROTECTED_CONTENT_EXT);
      ctxAttribList.push_back(EGL_TRUE);
   }
   ctxAttribList.push_back(EGL_CONTEXT_CLIENT_VERSION);
   ctxAttribList.push_back(GetESApiVersion(options));
   ctxAttribList.push_back(EGL_NONE);

   m_config = config;
   m_context = eglCreateContext(m_display, m_config, EGL_NO_CONTEXT, (options.GetApiVersion() == eOPVG_1_1) ? NULL : &ctxAttribList[0]);
   if (m_context == EGL_NO_CONTEXT)
      BSG_THROW("eglCreateContext() failed");

   /*
      Step 7 - Bind the context to the current thread and use our
      window surface for drawing and reading.
      Contexts are bound to a thread. This means you don't have to
      worry about other threads and processes interfering with your
      OpenGL ES application.
      We need to specify a surface that will be the target of all
      subsequent drawing operations, and one that will be the source
      of read operations. They can be the same surface.
   */
   eglMakeCurrent(m_display, m_surface, m_surface, m_context);

   if (options.GetSwapInterval() != 1)
      eglSwapInterval(m_display, options.GetSwapInterval());
}

// This initialization routine works for most platforms, but X11 is more
// convoluted.
void Context::Initialize(EGLDisplay display, const ApplicationOptions &options, EGLNativeWindowType nativeWindow)
{
   /*
      Step 1 - Get the display.
      EGL uses the concept of a "display" which in most environments
      corresponds to a single physical screen. Since we want
      to draw to the main screen or only have a single screen to begin
      with, we let EGL pick the default display.
      Querying other displays is platform specific.
   */
   EGLConfig  config  = ChooseConfigForDisplay(display, options);

   InitializeFromNativeWindow(options, config, nativeWindow);
}

void Context::Initialize(EGLDisplay display, const ApplicationOptions &options, NativePixmap *nativePixmap)
{
   /*
      Step 1 - Get the display.
      EGL uses the concept of a "display" which in most environments
      corresponds to a single physical screen. Since we want
      to draw to the main screen or only have a single screen to begin
      with, we let EGL pick the default display.
      Querying other displays is platform specific.
   */
   EGLConfig  config  = ChooseConfigForDisplay(display, options);

   InitializeWithPixmap(options, config, nativePixmap);
}

void Context::Terminate()
{
   if (m_display != EGL_NO_DISPLAY)
   {
      eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      if (m_context != EGL_NO_CONTEXT)
         eglDestroyContext(m_display, m_context);
      if (m_surface != EGL_NO_SURFACE)
         eglDestroySurface(m_display, m_surface);
      eglTerminate(m_display);

      m_display = EGL_NO_DISPLAY;
      m_surface = EGL_NO_SURFACE;
      m_context = EGL_NO_CONTEXT;
   }
}

void Context::MakeCurrent()
{
   if (m_display != EGL_NO_DISPLAY)
      eglMakeCurrent(m_display, m_surface, m_surface, m_context);
}

void Context::SwapBuffers()
{
   if (m_display != EGL_NO_DISPLAY)
   {
      eglSwapBuffers(m_display, m_surface);
   }
}

void Context::SetSwapInterval(int32_t interval)
{
   if (m_display != EGL_NO_DISPLAY)
      eglSwapInterval(m_display, interval);
}


}
