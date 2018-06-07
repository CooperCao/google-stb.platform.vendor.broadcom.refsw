/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <thread>
#include <atomic>
#include <list>

#include "nxwindowinfo.h"
#include "../helpers/semaphore.h"
#include "nxbitmap.h"

namespace wlpl
{

class NxWorker
{
public:
   NxWorker(void *platformState);
   ~NxWorker();

   static void VSyncCallback(void *ctx, int param);
   void VSyncHandler();

   static void RecycleCallback(void *ctx, int param);
   void RecycleHandler();

   void SetupDisplay(const NxWindowInfo &nw);
   void TermDisplay();

   void DisplayNexusSurface(NEXUS_DISPLAYHANDLE display, NEXUS_SurfaceHandle surface);

private:
   void *m_platformState;
   wlpl::NxWindowInfo m_info;
   std::thread m_worker;
   std::atomic<bool> m_done;

   helper::Semaphore m_vsync;

   std::mutex m_displayMutex;
   std::list<std::unique_ptr<NxBitmap>> m_withNexus;

   void mainThread(void);
};

}
