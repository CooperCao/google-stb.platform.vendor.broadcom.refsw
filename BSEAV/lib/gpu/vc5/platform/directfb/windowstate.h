/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "bitmap.h"

#include <EGL/egl.h>
#include <EGL/eglext_brcm.h>

 /* There will be one WINPL_WindowState for each WINPL_SurfCompNativeWindow */
class WindowState
{
public:
   WindowState(void *native) :
      m_native(native),
      m_bitmap(WrappedBitmap(static_cast<IDirectFBSurface*>(native))) {}
   ~WindowState() = default;

   void *GetWindowHandle() const
   {
      return m_native;
   }

   WrappedBitmap *GetBitmap()
   {
      return &m_bitmap;
   }

private:
   void           *m_native;
   WrappedBitmap   m_bitmap;
};
