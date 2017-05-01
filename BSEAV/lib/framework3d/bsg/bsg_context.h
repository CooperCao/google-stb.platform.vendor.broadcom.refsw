/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_CONTEXT_H__
#define __BSG_CONTEXT_H__

#include "bsg_common.h"
#include "bsg_gl.h"
#include <stdint.h>

namespace bsg
{
class ApplicationOptions;
class NativePixmap;

//! A Context represents an EGL rendering context and destination surface.
//! It manages the creation of the underlying EGL context based upon the ApplicationOptions.
class Context
{
public:
   Context();
   virtual ~Context();

   //! Initialize the context based upon the ApplicationOptions.
   //! Rendering using this context will send the output to nativewindow.
   virtual void Initialize(const ApplicationOptions &options, EGLNativeWindowType nativeWindow);

   //! Initialize the context based upon the ApplicationOptions.
   //! Rendering using this context will send the output to a pixmap.
   virtual void Initialize(const ApplicationOptions &options, NativePixmap *nativePixmap);

   virtual EGLConfig ChooseConfigForDisplay(EGLDisplay display, const ApplicationOptions &options);

   virtual void InitializeFromNativeWindow(const ApplicationOptions &options, EGLConfig config, EGLNativeWindowType nativeWindow);

   virtual void InitializeWithPixmap(const ApplicationOptions &options, EGLConfig config, NativePixmap *nativePixmap);

   //! Clean-up the context ready for termination
   virtual void Terminate();

   //! Make this context the current rendering context
   virtual void MakeCurrent();

   //! Present the current rendering to the display by swapping front and back buffers
   virtual void SwapBuffers();

   //! Sets the number of vsyncs between displayed frames. e.g.
   //! - 0 = un-synced - frames are displayed as quickly as possible. Tearing may result. Only really useful for benchmarking.
   //! - 1 = swap every vsync - maximum display rate of 60fps.
   //! - 2 = swap every 2 vsycns - maximum display rate of 30fps.
   //! - 5 = swap every 5 vsycns - maximum display rate of 12fps.
   void SetSwapInterval(int32_t interval);

   //! Return the EGL display
   EGLDisplay GetDisplay() const { return m_display; }

   //! Return the EGL surface
   EGLSurface GetSurface() const { return m_surface; }

   //! Return the EGL config used to create the EGL context
   EGLConfig GetConfig() const { return m_config; }

   //! Return the EGL context
   EGLContext GetContext() const { return m_context; }

private:

   void InitializeCreateContext(const ApplicationOptions &options, EGLConfig config);

   EGLDisplay m_display;
   EGLSurface m_surface;
   EGLContext m_context;
   EGLConfig  m_config;
};

}

#endif /* __BSG_CONTEXT_H__ */
