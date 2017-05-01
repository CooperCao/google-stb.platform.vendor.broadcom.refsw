/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <iterator>
#include <set>

#include "GLIncludes.h"
#include "DeviceCaps.h"

static std::set<std::string> getExtensions()
{
   std::string eglExtension(eglQueryString(eglGetDisplay(EGL_DEFAULT_DISPLAY), EGL_EXTENSIONS));
   std::string glExtension((const char *)glGetString(GL_EXTENSIONS));
   std::istringstream availableExtensions(eglExtension + ' ' + glExtension);
   std::istream_iterator<std::string> begin(availableExtensions);
   std::istream_iterator<std::string> end;
   std::set<std::string> out(begin, end);
   return out;
}

static bool getExtensionSupported(const char *p)
{
   static auto s_extensions = getExtensions();
   auto search = s_extensions.find(std::string(p));
   return (search != s_extensions.end());
}

static unsigned getVersion(const char *p, const char *t)
{
   unsigned major, minor;
   std::sscanf(p, t, &major, &minor);
   return (major * 100) + minor;
}

static bool getVersionSupported(const char *p)
{
   static auto s_eglv = getVersion(eglQueryString(eglGetDisplay(EGL_DEFAULT_DISPLAY), EGL_VERSION), "%d.%d\n");
   static auto s_glv = getVersion((const char *)glGetString(GL_VERSION), "OpenGL ES %d.%d\n");

   std::string i(p);
   if (i == "EGL_VERSION_1_0")
      return (s_eglv >= 100);
   else if (i == "EGL_VERSION_1_1")
      return (s_eglv >= 101);
   else if (i == "EGL_VERSION_1_2")
      return (s_eglv >= 102);
   else if (i == "EGL_VERSION_1_3")
      return (s_eglv >= 103);
   else if (i == "EGL_VERSION_1_4")
      return (s_eglv >= 104);
   else if (i == "EGL_VERSION_1_5")
      return (s_eglv >= 105);
   else if (i == "GL_VERSION_ES_CM_1_0")
      return (s_glv >= 100);
   else if (i == "GL_ES_VERSION_2_0")
      return (s_glv >= 200);
   else if (i == "GL_ES_VERSION_3_0")
      return (s_glv >= 300);
   else if (i == "GL_ES_VERSION_3_1")
      return (s_glv >= 301);
   else if (i == "GL_ES_VERSION_3_2")
      return (s_glv >= 302);

   return false;
}

DeviceCaps::DeviceCaps()
{
   // egl needs to be up
   eglInitialize(eglGetDisplay(EGL_DEFAULT_DISPLAY), NULL, NULL);
   eglBindAPI(EGL_OPENGL_ES_API);

   EGLint configAttribs[] = { EGL_SURFACE_TYPE,
                                   EGL_WINDOW_BIT,
                                   EGL_RENDERABLE_TYPE,
                                   EGL_OPENGL_ES2_BIT,
                                   EGL_NONE };
   int numConfigs;
   EGLConfig eglConfig;
   eglChooseConfig(eglGetDisplay(EGL_DEFAULT_DISPLAY), configAttribs, &eglConfig, 1, &numConfigs);

   EGLSurface eglSurface;
   EGLint surfaceAttribs[] = { EGL_WIDTH , 1,
                               EGL_HEIGHT, 1,
                               EGL_NONE };
   eglSurface = eglCreatePbufferSurface(eglGetDisplay(EGL_DEFAULT_DISPLAY), eglConfig, surfaceAttribs);

   EGLContext eglContext;
   EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2,
                               EGL_NONE };
   eglContext = eglCreateContext(eglGetDisplay(EGL_DEFAULT_DISPLAY), eglConfig, NULL, contextAttribs);

   EGLSurface eglSurfaceCurrentDraw = eglGetCurrentSurface(EGL_DRAW);
   EGLSurface eglSurfaceCurrentRead = eglGetCurrentSurface(EGL_READ);
   EGLContext eglContextCurrent = eglGetCurrentContext();
   EGLDisplay eglDisplayCurrent = eglGetCurrentDisplay();

   eglMakeCurrent(eglGetDisplay(EGL_DEFAULT_DISPLAY), eglSurface, eglSurface, eglContext);

#include "ExtensionsMethods.inc"

   eglMakeCurrent(eglDisplayCurrent, eglSurfaceCurrentDraw, eglSurfaceCurrentRead, eglContextCurrent);

   eglDestroyContext(eglGetDisplay(EGL_DEFAULT_DISPLAY), eglContext);
   eglDestroySurface(eglGetDisplay(EGL_DEFAULT_DISPLAY), eglSurface);
};

DeviceCaps::~DeviceCaps()
{

};
