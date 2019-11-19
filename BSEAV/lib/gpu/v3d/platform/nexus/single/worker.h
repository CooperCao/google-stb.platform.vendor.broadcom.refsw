/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <thread>
#include <atomic>

#include "windowinfo.h"
#include "../helpers/semaphore.h"
#include "bitmap.h"

namespace nxpl
{

class Worker
{
public:
   Worker(void *platformState, EventContext *eventContext);
   ~Worker();

   void DisplayNexusSurface(NEXUS_DISPLAYHANDLE display, NEXUS_SurfaceHandle surface);

private:
   static void VSyncCallback(void *ctx, int param);
   void VSyncHandler();
   void SetupDisplay(const nxpl::NativeWindowInfo &nw, const helper::Extent2D &extent);
   void TermDisplay();

private:
   void                                      *m_platformState;
   EventContext                              *m_eventContext;
   nxpl::NativeWindowInfo                    m_info;
   std::thread                               m_worker;
   std::atomic<bool>                         m_done;

   NEXUS_GraphicsSettings                    m_graphicsSettings;

   helper::Semaphore                         m_vsync;

   // Handle of the surface that was on display the last time the vsync
   // handler callback was called.
   std::unique_ptr<nxpl::Bitmap>             m_displayedSurface;
   uint32_t                                  m_displayedSurfaceDebugId;
   // Handle of the surface that we had set to go on display next the
   // last time the presentation job (in Mailbox mode) or vsync handler
   // (in FIFO mode) was called
   std::unique_ptr<nxpl::Bitmap>             m_pendingSurface;
   uint32_t                                  m_pendingSurfaceDebugId;

   std::mutex                                m_displayMutex;

   void mainThread(void);
};

}
