/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "nxworker.h"

#include "dispitem.h"
#include "../helpers/semaphore.h"
#include "nxwindowstate.h"

namespace wlpl
{

NxWorker::~NxWorker()
{
   m_done = true;
   auto windowState = static_cast<NxWindowState *>(m_platformState);

   std::unique_ptr<NxBitmap> bitmap;
   std::unique_ptr<helper::Semaphore> sem;
   std::unique_ptr<DispItem<NxBitmap>> dispItem(
         new DispItem<NxBitmap>(std::move(bitmap), std::move(sem)));
   windowState->PushDispQ(std::move(dispItem));
   m_worker.join();
}

NxWorker::NxWorker(void *platformState) :
      m_platformState(platformState),
      m_done(false)
{
   auto windowState = static_cast<NxWindowState *>(m_platformState);

   auto nw = static_cast<NxWindowInfo*>(windowState->GetWindowHandle());

   NEXUS_SurfaceClientSettings clientSettings;
   NEXUS_SurfaceClient_GetSettings(nw->GetSurfaceClient(), &clientSettings);
   clientSettings.recycled.callback = RecycleCallback;
   clientSettings.recycled.context = this;
   clientSettings.vsync.callback = VSyncCallback;
   clientSettings.vsync.context = this;
   clientSettings.allowCompositionBypass = true;
   NEXUS_SurfaceClient_SetSettings(nw->GetSurfaceClient(), &clientSettings);

   m_worker = std::thread(&NxWorker::mainThread, this);
}

void NxWorker::VSyncCallback(void *ctx, int param [[gnu::unused]])
{
   static_cast<NxWorker *>(ctx)->VSyncHandler();
}

void NxWorker::VSyncHandler()
{
   m_vsync.notify();
}

void NxWorker::RecycleCallback(void *ctx, int param [[gnu::unused]])
{
   static_cast<NxWorker *>(ctx)->RecycleHandler();
}

void NxWorker::RecycleHandler()
{
   std::lock_guard<std::mutex> lock(m_displayMutex);

   auto windowState = static_cast<NxWindowState *>(m_platformState);

   auto nw = static_cast<NxWindowInfo*>(windowState->GetWindowHandle());

   const size_t numSurfaces = windowState->GetMaxSwapBuffers();
   NEXUS_SurfaceHandle surfaces[numSurfaces];
   memset(surfaces, 0, sizeof(surfaces));
   size_t n = 0;
   NEXUS_SurfaceClient_RecycleSurface(nw->GetSurfaceClient(), surfaces, numSurfaces, &n);

   for (size_t i = 0; i < n; i++)
   {
      for (auto j = m_withNexus.begin(); j != m_withNexus.end(); ++j)
      {
         if ((*j)->GetSurface() == surfaces[i])
         {
            std::unique_ptr<NxBitmap> bitmap = std::move(*j);
            m_withNexus.erase(j);
            windowState->PushFreeQ(std::move(bitmap));
            break;
         }
      }
   }
}

void NxWorker::SetupDisplay(const NxWindowInfo &nw)
{
#ifdef NXCLIENT_SUPPORT
   NEXUS_SurfaceComposition comp;

   NxClient_GetSurfaceClientComposition(nw.GetClientID(), &comp);

   /* TODO set comp.zorder to control z-order (0 = bottom layer) */
   if (!nw.GetStretch())
   {
      comp.virtualDisplay.width = 0;
      comp.virtualDisplay.height = 0;
      comp.position.width = nw.GetWidth();
      comp.position.height = nw.GetHeight();
   }
   else
   {
      comp.virtualDisplay.width = nw.GetWidth();
      comp.virtualDisplay.height = nw.GetHeight();
      comp.position.width = nw.GetWidth() - (2 * nw.GetX());
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
   case WLPL_2D:
      clientSettings.orientation = NEXUS_VideoOrientation_e2D;
      break;
   case WLPL_3D_LEFT_RIGHT:
      clientSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
      break;
   case WLPL_3D_OVER_UNDER:
      clientSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
      break;
   }
   NEXUS_SurfaceClient_SetSettings(nw.GetSurfaceClient(), &clientSettings);
}

void NxWorker::TermDisplay()
{
   auto windowState = static_cast<NxWindowState *>(m_platformState);

   auto nw = static_cast<NxWindowInfo*>(windowState->GetWindowHandle());

   NEXUS_SurfaceClientSettings clientSettings;
   NEXUS_SurfaceClient_GetSettings(nw->GetSurfaceClient(), &clientSettings);
   clientSettings.recycled.callback      = NULL;
   clientSettings.vsync.callback         = NULL;
   NEXUS_SurfaceClient_SetSettings(nw->GetSurfaceClient(), &clientSettings);

   NEXUS_SurfaceClient_Clear(nw->GetSurfaceClient());
}

void NxWorker::mainThread(void)
{
   // buffer is pushed to fifo on swapbuffers()
   while (!m_done)
   {
      auto windowState = static_cast<NxWindowState *>(m_platformState);

      auto nw = static_cast<NxWindowInfo*>(windowState->GetWindowHandle());

      auto dispItem = windowState->PopDispQ();

      if (!m_done)
      {
         // check if display parameters match
         auto windowInfo = dispItem->m_bitmap->GetWindowInfo();
         if (windowInfo != windowState->GetWindowInfo())
         {
            SetupDisplay(windowInfo);
            windowState->UpdateWindowInfo(windowInfo);
         }

         // buffer is ready when mutex is triggered (replace with fence in final solution)
         // if this is NULL, then its already signalled, so OK to display
         if (dispItem->m_fence)
            dispItem->m_fence->wait();

         {
            std::lock_guard<std::mutex> lock(m_displayMutex);
            NEXUS_SurfaceClient_PushSurface(nw->GetSurfaceClient(),
                  dispItem->m_bitmap->GetSurface(), NULL, true);
            m_withNexus.push_back(std::move(dispItem->m_bitmap));
         }

         m_vsync.reset();

         for (int i = 0; i < dispItem->m_swapInterval; i++)
            m_vsync.wait();

         RecycleHandler();
      }
   }
   TermDisplay();
}

}
