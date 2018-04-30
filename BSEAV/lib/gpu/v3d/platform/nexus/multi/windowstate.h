/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <list>
#include <mutex>

#include "../helpers/queue.h"
#include "worker.h"
#include "bitmap.h"
#include "dispitem.h"

namespace nxpl
{

 /* There will be one WINPL_WindowState for each WINPL_SurfCompNativeWindow */
class WindowState
{
public:
   WindowState(void *context __attribute__((unused)), BEGL_WindowHandle windowHandle, bool secure) :
      m_windowHandle(windowHandle),
      m_dispQ(),
      m_secure(secure),
      m_worker(std::unique_ptr<nxpl::Worker>(new nxpl::Worker(this,
            static_cast<NXPL_Display *>(context)->eventContext))) {}
   ~WindowState() = default;

   bool Init(void *context __attribute__((unused)), unsigned swapbuffers)
   {
      for (unsigned i = 0; i < swapbuffers; i++)
         m_freeQ.Push(std::unique_ptr<nxpl::Bitmap>{});
      m_maxSwapBuffers = swapbuffers;
      return true;
   }

   BEGL_WindowHandle GetWindowHandle() const
   {
      return m_windowHandle;
   }

   nxpl::NativeWindowInfo GetWindowInfo() const
   {
      return m_info;
   }

   void UpdateWindowInfo(nxpl::NativeWindowInfo &info)
   {
      m_info = info;
   }

   std::unique_ptr<nxpl::Bitmap> PopFreeQ()
   {
      return m_freeQ.Pop();
   }

   void PushFreeQ(std::unique_ptr<nxpl::Bitmap> bitmap)
   {
      m_freeQ.Push(std::move(bitmap));
   }

   std::unique_ptr<nxpl::DispItem> PopDispQ()
   {
      return m_dispQ.Pop();
   }

   void PushDispQ(std::unique_ptr<nxpl::DispItem> dispItem)
   {
      m_dispQ.Push(std::move(dispItem));
   }

   bool IsSecure() const
   {
      return m_secure;
   }

   unsigned GetMaxSwapBuffers() const
   {
      return m_maxSwapBuffers;
   }

private:
   BEGL_WindowHandle                                        m_windowHandle;
   nxpl::NativeWindowInfo                                   m_info;

   helper::MessageQueue<nxpl::Bitmap>                       m_freeQ;
   helper::MessageQueue<nxpl::DispItem>                     m_dispQ;

   bool                                                     m_secure;

   std::unique_ptr<nxpl::Worker>                            m_worker;

   unsigned                                                 m_maxSwapBuffers;
};

}
