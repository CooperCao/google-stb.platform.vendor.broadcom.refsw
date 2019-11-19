/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "wl_client.h"
#include "dispitem.h"
#include "wlbitmap.h"
#include "display_wl_priv.h"
#include "../helpers/queue.h"

#include <memory>
#include <list>

#include "wayland_egl/wayland_egl_priv.h"

namespace wlpl
{

class WlWorker;

class WlWindow
{
public:
   WlWindow(struct WlClient *client, wl_egl_window *window,
         unsigned swapbuffers, bool secure);

   ~WlWindow()
   {
      DestroyWindow();
   }


   std::unique_ptr<WlBitmap> PopFreeQ(BEGL_BufferFormat format);

   void PushFreeQ(std::unique_ptr<WlBitmap> bitmap)
   {
      m_freeQ.Push(std::move(bitmap));
      m_wlEglWindow->attached_width = 0;
      m_wlEglWindow->attached_height = 0;
   }

   void PushDispQ(std::unique_ptr<DispItem<WlBitmap>> dispItem)
   {
      m_dispQ.Push(std::move(dispItem));
      m_wlEglWindow->attached_width = 0;
      m_wlEglWindow->attached_height = 0;
   }

private:
   friend class WlWorker;
   void DisplayNextBuffer();

private:
   static void BufferReleaseCb(void *data, struct wl_buffer *buffer)
   {
      WlWindow *self = static_cast<WlWindow *>(data);
      self->BufferRelease(buffer);
   }
   void BufferRelease(struct wl_buffer *buffer);
   int WaitRelease();

   static void ResizeCb(struct wl_egl_window *egl_window, void *data)
   {
      WlWindow *self = static_cast<WlWindow *>(data);
      self->Resize(egl_window);
   }
   void Resize(struct wl_egl_window *egl_window);

   static void DestroyWindowCb(void *data)
   {
      WlWindow *self = static_cast<WlWindow *>(data);
      self->DestroyWindow();
   }
   void DestroyWindow();

private:
   struct ::WlClient *m_client;
   struct ::wl_egl_window *m_wlEglWindow;
   const bool m_secure;

   struct ::WlWindow m_window;

   helper::MessageQueue<WlBitmap> m_freeQ;
   helper::MessageQueue<DispItem<WlBitmap>> m_dispQ;

   std::mutex m_displayMutex;
   std::list<std::unique_ptr<WlBitmap>> m_withWayland;
   helper::Extent2D m_windowSize;
   int m_windowDx;
   int m_windowDy;

   wlpl::WlWorker *m_worker;

};

}
