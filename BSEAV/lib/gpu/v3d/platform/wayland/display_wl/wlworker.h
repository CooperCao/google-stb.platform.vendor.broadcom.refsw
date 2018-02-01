/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "wlwindow.h"

#include <thread>
#include <atomic>

namespace wlpl
{

class WlWorker
{
public:
   WlWorker(WlWindow &window) :
         m_window(window),
         m_done(false)
   {
      m_worker = std::thread(&WlWorker::mainThread, this);
   }
   ~WlWorker()
   {
      m_done = true;

      std::unique_ptr<WlBitmap> bitmap;
      std::unique_ptr<helper::Semaphore> sem;
      std::unique_ptr<DispItem<WlBitmap>> dispItem(
            new DispItem<WlBitmap>(std::move(bitmap), std::move(sem)));
      m_window.PushDispQ(std::move(dispItem));

      m_worker.join();
   }

private:
   WlWindow &m_window;
   std::thread m_worker;
   std::atomic<bool> m_done;

   void mainThread(void)
   {
      while (!m_done)
         m_window.DisplayNextBuffer();
   }
};

}
