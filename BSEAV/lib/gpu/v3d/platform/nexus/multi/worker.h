/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <thread>
#include <atomic>
#include <list>

#include "windowinfo.h"
#include "../helpers/semaphore.h"
#include "bitmap.h"

namespace nxpl
{

class Worker
{
public:
   Worker(void *platformState);
   ~Worker();

   static void VSyncCallback(void *ctx, int param);
   void VSyncHandler();

   static void RecycleCallback(void *ctx, int param);
   void RecycleHandler();

   void SetupDisplay(const nxpl::NativeWindowInfo &nw);

   void DisplayNexusSurface(NEXUS_DISPLAYHANDLE display, NEXUS_SurfaceHandle surface);

private:
   void                                      *m_platformState;
   nxpl::NativeWindowInfo                    m_info;
   std::thread                               m_worker;
   std::atomic<bool>                         m_done;

   helper::Semaphore                         m_vsync;

   std::mutex                                m_displayMutex;
   std::list<std::unique_ptr<nxpl::Bitmap>>  m_withNexus;

   void mainThread(void);
};

}
