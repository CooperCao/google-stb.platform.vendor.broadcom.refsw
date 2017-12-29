/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "worker.h"
#include "windowstate.h"
#include "dispitem.h"
#include "../helpers/semaphore.h"

namespace nxpl
{

Worker::~Worker()
{
   m_done = true;
   auto windowState = static_cast<nxpl::WindowState *>(m_platformState);

   auto nw = static_cast<nxpl::NativeWindowInfo*>(windowState->GetWindowHandle());

   std::unique_ptr<nxpl::Bitmap> bitmap;
   std::unique_ptr<helper::Semaphore> sem;
   std::unique_ptr<nxpl::DispItem> dispItem(new nxpl::DispItem(std::move(bitmap), std::move(sem)));
   windowState->PushDispQ(std::move(dispItem));
   m_worker.join();

   NEXUS_SurfaceClient_Clear(nw->GetSurfaceClient());
}

Worker::Worker(void *platformState) :
   m_platformState(platformState),
   m_done(false)
{
   auto windowState = static_cast<nxpl::WindowState *>(m_platformState);

   auto nw = static_cast<nxpl::NativeWindowInfo*>(windowState->GetWindowHandle());

   NEXUS_SurfaceClientSettings clientSettings;
   NEXUS_SurfaceClient_GetSettings(nw->GetSurfaceClient(), &clientSettings);
   clientSettings.recycled.callback      = RecycleCallback;
   clientSettings.recycled.context       = this;
   clientSettings.vsync.callback         = VSyncCallback;
   clientSettings.vsync.context          = this;
   clientSettings.allowCompositionBypass = true;
   NEXUS_SurfaceClient_SetSettings(nw->GetSurfaceClient(), &clientSettings);

   m_worker = std::thread(&Worker::mainThread, this);
}

void Worker::VSyncCallback(void *ctx, int param __attribute__((unused)))
{
   static_cast<Worker *>(ctx)->VSyncHandler();
}

void Worker::VSyncHandler()
{
   m_vsync.notify();
}

void Worker::RecycleCallback(void *ctx, int param __attribute__((unused)))
{
   static_cast<Worker *>(ctx)->RecycleHandler();
}

void Worker::RecycleHandler()
{
   std::lock_guard<std::mutex> lock(m_displayMutex);

   auto windowState = static_cast<nxpl::WindowState *>(m_platformState);

   auto nw = static_cast<nxpl::NativeWindowInfo*>(windowState->GetWindowHandle());

   NEXUS_SurfaceHandle surface = NULL;
   size_t n = 0;
   NEXUS_SurfaceClient_RecycleSurface(nw->GetSurfaceClient(), &surface, 1, &n);
   if (!n)
      return;

   for (auto i = m_withNexus.begin(); i != m_withNexus.end();)
   {
      if ((*i)->GetSurface() == surface)
      {
         std::unique_ptr<nxpl::Bitmap> bitmap = std::move(*i);
         m_withNexus.erase(i);
         windowState->PushFreeQ(std::move(bitmap));
         break;
      }
   }
}

void Worker::SetupDisplay(const NativeWindowInfo &nw)
{
#ifdef NXCLIENT_SUPPORT
   NEXUS_SurfaceComposition   comp;

   NxClient_GetSurfaceClientComposition(nw.GetClientID(), &comp);

   /* TODO set comp.zorder to control z-order (0 = bottom layer) */
   if (!nw.GetStretch())
   {
      comp.virtualDisplay.width  = 0;
      comp.virtualDisplay.height = 0;
      comp.position.width  = nw.GetWidth();
      comp.position.height = nw.GetHeight();
   }
   else
   {
      comp.virtualDisplay.width  = nw.GetWidth();
      comp.virtualDisplay.height = nw.GetHeight();
      comp.position.width  = nw.GetWidth() - (2 * nw.GetX());
      comp.position.height = nw.GetHeight() - (2 * nw.GetY());
   }
   comp.position.x = nw.GetX();
   comp.position.y = nw.GetY();
   comp.zorder = nw.GetZOrder();
   comp.colorBlend = nw.GetColorBlend();
   comp.alphaBlend = nw.GetAlphaBlend();

   NxClient_SetSurfaceClientComposition(nw.GetClientID(), &comp);
#endif

   NEXUS_SurfaceClientSettings clientSettings;
   NEXUS_SurfaceClient_GetSettings(nw.GetSurfaceClient(), &clientSettings);
   switch (nw.GetType())
   {
   default:
   case NXPL_2D: clientSettings.orientation = NEXUS_VideoOrientation_e2D; break;
   case NXPL_3D_LEFT_RIGHT: clientSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight; break;
   case NXPL_3D_OVER_UNDER: clientSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder; break;
   }
   NEXUS_SurfaceClient_SetSettings(nw.GetSurfaceClient(), &clientSettings);
}

void Worker::mainThread(void)
{
   // buffer is pushed to fifo on swapbuffers()
   while (!m_done)
   {
      auto windowState = static_cast<nxpl::WindowState *>(m_platformState);

      auto nw = static_cast<nxpl::NativeWindowInfo*>(windowState->GetWindowHandle());

      auto dispItem = windowState->PopDispQ();

      if (!m_done)
      {
         // check if display parameters match
         if (dispItem->m_bitmap->GetWindowInfo() != windowState->GetWindowInfo())
         {
            SetupDisplay(dispItem->m_bitmap->GetWindowInfo());
            auto windowInfo = dispItem->m_bitmap->GetWindowInfo();
            windowState->UpdateWindowInfo(windowInfo);
         }

         // buffer is ready when mutex is triggered (replace with fence in final solution)
         dispItem->m_fence->wait();

         {
            std::lock_guard<std::mutex> lock(m_displayMutex);
            NEXUS_SurfaceClient_PushSurface(nw->GetSurfaceClient(), dispItem->m_bitmap->GetSurface(), NULL, true);
            m_withNexus.push_back(std::move(dispItem->m_bitmap));
         }

         m_vsync.reset();

         for (int i = 0; i < dispItem->m_swapInterval; i++)
            m_vsync.wait();

         RecycleHandler();
      }
   }
}

}
