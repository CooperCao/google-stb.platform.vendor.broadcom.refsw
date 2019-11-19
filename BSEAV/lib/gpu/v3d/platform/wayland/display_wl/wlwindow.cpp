/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/

#include "wlwindow.h"

#include "wlworker.h"
#include "default_wayland.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

namespace wlpl
{

WlWindow::WlWindow(struct WlClient *client, wl_egl_window *window, unsigned swapbuffers, bool secure) :
      m_client(client),
      m_wlEglWindow(window),
      m_secure(secure),
      m_windowDx(0),
      m_windowDy(0),
      m_worker(NULL)
{
   if (!CreateWlWindow(&m_window, m_client, m_wlEglWindow))
   {
      fprintf(stderr, "failed to create Wayland window\n");
      exit(1);
   }

   m_windowSize = helper::Extent2D(m_wlEglWindow->width, m_wlEglWindow->height);
   m_windowDx = m_wlEglWindow->dx;
   m_windowDy = m_wlEglWindow->dy;
   m_wlEglWindow->callback_private = this;
   m_wlEglWindow->resize_callback = ResizeCb;
   m_wlEglWindow->destroy_window_callback = DestroyWindowCb;

   for (unsigned i = 0; i < swapbuffers; i++)
      m_freeQ.Push(NULL);

   m_worker = new WlWorker(*this);
}

std::unique_ptr<WlBitmap> WlWindow::PopFreeQ(BEGL_BufferFormat format)
{
   helper::Extent2D bitmapExtent;

   while (m_freeQ.Empty() && WaitWlRelease(&m_window, true) != -1)
      continue;

   auto bitmap = m_freeQ.Pop();
   if (bitmap)
      bitmapExtent = bitmap->GetExtent2D();

   std::lock_guard<std::mutex> lock(m_displayMutex);
   // resize or create
   if (m_windowSize != bitmapExtent)
   {
      std::unique_ptr<WlBitmap> tmp(
            new WlBitmap(m_client, m_windowSize.GetWidth(),
                  m_windowSize.GetHeight(), format, m_secure,
                  BufferReleaseCb, this));
      // Copy requested window displacement to bitmap for later,
      // when the buffer is going to be displayed.
      tmp->SetWindowDisplacement(m_windowDx, m_windowDy);
      m_windowDx = m_windowDy = 0;
      m_wlEglWindow->attached_width = tmp->GetExtent2D().GetWidth();
      m_wlEglWindow->attached_height = tmp->GetExtent2D().GetHeight();
      bitmap = std::move(tmp);
   }
   return bitmap;
}

void WlWindow::DisplayNextBuffer()
{
   auto dispItem = m_dispQ.Pop();
   if (!dispItem->m_bitmap)
      return;

   if (dispItem->m_fence)
      dispItem->m_fence->wait();

   {
      std::lock_guard<std::mutex> lock(m_displayMutex);
      DisplayWlSharedBuffer(&m_window, &dispItem->m_bitmap->GetWlBuffer(),
            dispItem->m_bitmap->GetWindowDx(),
            dispItem->m_bitmap->GetWindowDy());
      dispItem->m_bitmap->SetWindowDisplacement(0, 0);

      m_withWayland.push_back(std::move(dispItem->m_bitmap));
   }

   for (int i = 0; i < dispItem->m_swapInterval; i++)
      WaitWlFrame(&m_window);
}

void WlWindow::BufferRelease(struct wl_buffer *buffer)
{
   std::lock_guard<std::mutex> lock(m_displayMutex);
   for (auto i = m_withWayland.begin(); i != m_withWayland.end(); ++i)
   {
      if ((*i)->GetWlBuffer().buffer == buffer)
      {
         std::unique_ptr<WlBitmap> bitmap = std::move(*i);
         m_withWayland.erase(i);
         PushFreeQ(std::move(bitmap));
         break;
      }
   }
}

void WlWindow::Resize(struct wl_egl_window *egl_window)
{
   assert(egl_window == m_wlEglWindow);

   std::lock_guard<std::mutex> lock(m_displayMutex);
   m_windowSize = helper::Extent2D(egl_window->width, egl_window->height);
   m_windowDx = egl_window->dx;
   m_windowDy = egl_window->dy;
}

void WlWindow::DestroyWindow()
{
   delete m_worker; //before the DestroyWlWindow()
   m_worker = NULL;

   DestroyWlWindow(&m_window);

   m_wlEglWindow->resize_callback = NULL;
   m_wlEglWindow->destroy_window_callback = NULL;
   m_wlEglWindow->callback_private = NULL;
}

} //namespace wlpl
