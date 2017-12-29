/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

/* #include <list> */
/* #include <mutex> */

/* #include "../helpers/queue.h" */
/* #include "worker.h" */
#include "bitmap.h"
/* #include "dfbcheck.h" */

#include "default_directfb.h"
#include "display_priv.h"

namespace dbpl
{

 /* There will be one WINPL_WindowState for each WINPL_SurfCompNativeWindow */
class WindowState
{
public:
   WindowState(void *context, BEGL_WindowHandle windowHandle) :
      m_windowHandle(windowHandle),
      m_bitmap(dbpl::WrappedBitmap(context, static_cast<IDirectFBSurface*>(windowHandle)))
   {}
   ~WindowState() = default;

   BEGL_WindowHandle GetWindowHandle() const
   {
      return m_windowHandle;
   }

   dbpl::WrappedBitmap *GetBitmap()
   {
      return &m_bitmap;
   }

private:
   BEGL_WindowHandle                                        m_windowHandle;
   dbpl::WrappedBitmap                                      m_bitmap;
};

}
