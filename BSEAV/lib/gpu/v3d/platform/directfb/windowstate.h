/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "bitmap.h"

#include "default_directfb.h"
#include "display_priv.h"

#include <EGL/egl.h>
#include <EGL/begl_platform.h>

 /* There will be one WINPL_WindowState for each WINPL_SurfCompNativeWindow */
class WindowState
{
public:
   WindowState(BEGL_WindowHandle windowHandle) :
      m_windowHandle(windowHandle),
      m_bitmap(WrappedBitmap(static_cast<IDirectFBSurface*>(windowHandle)))
   {}
   ~WindowState() = default;

   BEGL_WindowHandle GetWindowHandle() const
   {
      return m_windowHandle;
   }

   WrappedBitmap *GetBitmap()
   {
      return &m_bitmap;
   }

private:
   BEGL_WindowHandle    m_windowHandle;
   WrappedBitmap        m_bitmap;
};
