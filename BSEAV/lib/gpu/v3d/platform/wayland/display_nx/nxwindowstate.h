/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <list>
#include <mutex>

#include "nxbitmap.h"
#include "nxworker.h"
#include "../helpers/queue.h"
#include "dispitem.h"

namespace wlpl
{

/* There will be one WINPL_WindowState for each WINPL_SurfCompNativeWindow */
class NxWindowState
{
public:
   NxWindowState(void *context [[gnu::unused]], BEGL_WindowHandle windowHandle,
         bool secure) :
         m_windowHandle(windowHandle),
         m_dispQ(),
         m_secure(secure),
         m_worker(std::unique_ptr<NxWorker>(new NxWorker(this)))
   {
   }

   ~NxWindowState() = default;

   bool Init(void *context [[gnu::unused]], unsigned swapbuffers)
   {
      for (unsigned i = 0; i < swapbuffers; i++)
         m_freeQ.Push(NULL);
      m_maxSwapBuffers = swapbuffers;
      return true;
   }

   BEGL_WindowHandle GetWindowHandle() const
   {
      return m_windowHandle;
   }

   NxWindowInfo GetWindowInfo() const
   {
      return m_info;
   }

   void UpdateWindowInfo(NxWindowInfo &info)
   {
      m_info = info;
   }

   std::unique_ptr<NxBitmap> PopFreeQ()
   {
      return m_freeQ.Pop();
   }

   void PushFreeQ(std::unique_ptr<NxBitmap> bitmap)
   {
      m_freeQ.Push(std::move(bitmap));
   }

   std::unique_ptr<DispItem<NxBitmap>> PopDispQ()
   {
      return m_dispQ.Pop();
   }

   void PushDispQ(std::unique_ptr<DispItem<NxBitmap>> dispItem)
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
   BEGL_WindowHandle m_windowHandle;
   NxWindowInfo m_info;

   helper::MessageQueue<NxBitmap> m_freeQ;
   helper::MessageQueue<DispItem<NxBitmap>> m_dispQ;

   bool m_secure;

   std::unique_ptr<NxWorker> m_worker;

   unsigned m_maxSwapBuffers;
};

}
