/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "worker.h"
#include "windowstate.h"
#include "dispitem.h"
#include "../helpers/semaphore.h"
#include "nexus_core_utils.h"

namespace nxpl
{

Worker::~Worker()
{
   m_done = true;
   auto windowState = static_cast<nxpl::WindowState *>(m_platformState);

   std::unique_ptr<nxpl::Bitmap> bitmap;
   std::unique_ptr<helper::Semaphore> sem;
   std::unique_ptr<nxpl::DispItem> dispItem(new nxpl::DispItem(std::move(bitmap), std::move(sem), 0));
   windowState->PushDispQ(std::move(dispItem));
   m_vsync.notify();
   m_worker.join();
}

Worker::Worker(void *platformState, EventContext *eventContext) :
   m_platformState(platformState),
   m_eventContext(eventContext),
   m_done(false),
   m_displayedSurfaceDebugId(0),
   m_pendingSurfaceDebugId(0)
{
   auto windowState = static_cast<nxpl::WindowState *>(m_platformState);

   NEXUS_CallbackDesc vsync;
   NEXUS_CallbackDesc_Init(&vsync);
   vsync.callback = VSyncCallback;
   vsync.context  = this;

   if (NEXUS_Display_SetVsyncCallback(windowState->GetDisplay(), &vsync) != NEXUS_SUCCESS)
   {
      printf("Unable to set Nexus vsync callback");
      exit(0);
   }

   NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eManual);

   m_worker = std::thread(&Worker::mainThread, this);
}

void Worker::VSyncCallback(void *ctx, int param __attribute__((unused)))
{
   reinterpret_cast<Worker *>(ctx)->VSyncHandler();
}

void Worker::VSyncHandler()
{
   std::unique_lock<std::mutex> lock { m_displayMutex };

   auto windowState = static_cast<nxpl::WindowState *>(m_platformState);

   if (m_displayedSurface)
   {
      NEXUS_GraphicsFramebufferStatus status;
      NEXUS_Display_GetGraphicsFramebufferStatus(windowState->GetDisplay(), m_displayedSurface->GetSurface(), &status);

      if (status.state == NEXUS_GraphicsFramebufferState_eUnused)
      {
         PerfAddEvent(m_eventContext, PERF_EVENT_TRACK_DISPLAY, PERF_EVENT_ON_DISPLAY, m_displayedSurfaceDebugId,
                      BCM_EVENT_END, (uintptr_t)m_displayedSurface->GetSurface());

         if (windowState->IsBouncing())
            windowState->PushGFDBackingQ(std::move(m_displayedSurface));
         else
            windowState->PushFreeQ(std::move(m_displayedSurface));
      }
   }

   if (m_pendingSurface)
   {
      NEXUS_GraphicsFramebufferStatus status;
      NEXUS_Display_GetGraphicsFramebufferStatus(windowState->GetDisplay(), m_pendingSurface->GetSurface(), &status);

      if (status.state == NEXUS_GraphicsFramebufferState_eDisplayed)
      {
         m_displayedSurface = std::move(m_pendingSurface);
         m_displayedSurfaceDebugId = m_pendingSurfaceDebugId;
         m_pendingSurfaceDebugId++;

         PerfAddEvent(m_eventContext, PERF_EVENT_TRACK_DISPLAY, PERF_EVENT_ON_DISPLAY, m_displayedSurfaceDebugId,
                      BCM_EVENT_BEGIN, (uintptr_t)m_displayedSurface->GetSurface());
      }
   }

   m_vsync.notify();
}

void Worker::SetupDisplay(const NativeWindowInfo &nw, const helper::Extent2D &extent)
{
   auto windowState = static_cast<nxpl::WindowState *>(m_platformState);

   /* setup the display & callback */
   NEXUS_Display_GetGraphicsSettings(windowState->GetDisplay(), &m_graphicsSettings);
   m_graphicsSettings.enabled = true;

   if (!nw.GetStretch())
   {
      m_graphicsSettings.position.x = nw.GetX();
      m_graphicsSettings.position.y = nw.GetY();

      m_graphicsSettings.position.width = extent.GetWidth();
      m_graphicsSettings.position.height = extent.GetHeight();
   }
   else
   {
      float scaledPosX = nw.GetX() * ((float)m_graphicsSettings.position.width / extent.GetWidth());
      float scaledPosY = nw.GetY() * ((float)m_graphicsSettings.position.height / extent.GetHeight());

      m_graphicsSettings.position.x = (uint16_t)scaledPosX;
      m_graphicsSettings.position.y = (uint16_t)scaledPosY;

      m_graphicsSettings.position.width -= (2 * scaledPosX);
      m_graphicsSettings.position.height -= (2 * scaledPosY);
   }
   m_graphicsSettings.clip.width = extent.GetWidth();
   m_graphicsSettings.clip.height = extent.GetHeight();
}

void Worker::TermDisplay()
{
   auto windowState = static_cast<nxpl::WindowState *>(m_platformState);
   if (windowState->GetDisplay() != NULL)
   {
      NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eAuto);
      NEXUS_Display_SetVsyncCallback(windowState->GetDisplay(), NULL);

      // Clear callbacks that are 'in-flight'. From Nexus description of
      // the NEXUS_StopCallbacks():
      //
      // "Description:
      // This waits for any running callbacks to finish and ensures that
      // no other callbacks for this handle arrive."
      NEXUS_StopCallbacks(windowState->GetDisplay());
      NEXUS_StartCallbacks(windowState->GetDisplay());

      NEXUS_GraphicsSettings settings;
      NEXUS_Display_GetGraphicsSettings(windowState->GetDisplay(), &settings);
      settings.enabled = false;
      NEXUS_Display_SetGraphicsSettings(windowState->GetDisplay(), &settings);
   }
}

void Worker::mainThread(void)
{
   // buffer is pushed to fifo on swapbuffers()
   while (!m_done)
   {
      auto windowState = static_cast<nxpl::WindowState *>(m_platformState);
      auto dispItem = windowState->PopDispQ();

      if (!m_done)
      {
         auto windowInfo = dispItem->m_bitmap->GetWindowInfo();
         // check if display parameters match
         if (windowInfo != windowState->GetWindowInfo())
         {
            SetupDisplay(windowInfo, dispItem->m_bitmap->GetExtent2D());
            windowState->UpdateWindowInfo(windowInfo);
         }

         // buffer is ready when mutex is triggered (replace with fence in final solution)
         // if this is NULL, then its already signalled, so OK to display
         if (dispItem->m_fence)
            dispItem->m_fence->wait();

         if (NEXUS_Display_SetGraphicsSettings(windowState->GetDisplay(), &m_graphicsSettings) != NEXUS_SUCCESS)
         {
            printf("NEXUS_Display_SetGraphicsSettings() failed\n");
            exit(0);
         }

#if NEXUS_HAS_GRAPHICS2D
         std::unique_ptr<nxpl::Bitmap> bitmap;
         if (windowState->IsBouncing())
         {
            NEXUS_Graphics2DBlitSettings blitSettings;
            NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);

            /* blit from v3d surface to GFD accessible version (will block until surface off display) */
            bitmap = windowState->PopGFDBackingQ();

            blitSettings.source.surface = dispItem->m_bitmap->GetSurface();
            blitSettings.output.surface = bitmap->GetSurface();
            blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
            blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

            while (1)
            {
               windowState->PacketSpaceAvailableReset();
               NEXUS_Error rc = NEXUS_Graphics2D_Blit(windowState->GetGFX(), &blitSettings);
               if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL)
                  windowState->PacketSpaceAvailableWait();
               else
                  break;
            }

            windowState->CheckpointReset();
            NEXUS_Error rc = NEXUS_Graphics2D_Checkpoint(windowState->GetGFX(), NULL); /* require to execute queue */
            if (rc == NEXUS_GRAPHICS2D_QUEUED)
               windowState->CheckpointWait();

            windowState->PushFreeQ(std::move(dispItem->m_bitmap));
         }
         else
#endif
            bitmap = std::move(dispItem->m_bitmap);

         NEXUS_GraphicsFramebuffer3D fb3d;
         NEXUS_Graphics_GetDefaultFramebuffer3D(&fb3d);

         fb3d.main = bitmap->GetSurface();
         fb3d.orientation = bitmap->GetOrientation();

         {
            std::unique_lock<std::mutex> lock { m_displayMutex };

            if (NEXUS_Display_SetGraphicsFramebuffer3D(windowState->GetDisplay(), &fb3d) != NEXUS_SUCCESS)
            {
               printf("NEXUS_Display_SetGraphicsFramebuffer3D() failed\n");
               exit(0);
            }

            if (NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eNow) != NEXUS_SUCCESS)
            {
               printf("NEXUS_DisplayModule_SetUpdateMode() failed\n");
               exit(0);
            }

            if (m_pendingSurface)
            {
               std::unique_ptr<nxpl::Bitmap> handleToRelease;
               NEXUS_GraphicsFramebufferStatus status;
               NEXUS_Display_GetGraphicsFramebufferStatus(windowState->GetDisplay(), m_pendingSurface->GetSurface(), &status);

               if (status.state == NEXUS_GraphicsFramebufferState_eUnused)
                  handleToRelease = std::move(m_pendingSurface);
               else
               {
                  handleToRelease = std::move(m_displayedSurface);
                  m_displayedSurface = std::move(m_pendingSurface);
               }

               if (handleToRelease)
               {
                  if (windowState->IsBouncing())
                     windowState->PushGFDBackingQ(std::move(handleToRelease));
                  else
                     windowState->PushFreeQ(std::move(handleToRelease));
               }
            }
            m_pendingSurface = std::move(bitmap);

            m_vsync.reset();
         }

         for (int i = 0; i < dispItem->m_swapInterval && !m_done; i++)
            m_vsync.wait();
      }
   }
   TermDisplay();
}

}
