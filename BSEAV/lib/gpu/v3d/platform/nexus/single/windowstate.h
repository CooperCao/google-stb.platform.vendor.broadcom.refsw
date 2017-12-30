/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <list>
#include <mutex>

#include "../helpers/queue.h"
#include "worker.h"
#include "bitmap.h"
#include "dispitem.h"

namespace nxpl
{

 /* There will be one WINPL_WindowState for each WINPL_SurfCompNativeWindow */
class WindowState
{
public:
   WindowState(void *context, BEGL_WindowHandle windowHandle, bool secure);
   ~WindowState();
   bool Init(void *context, unsigned swapbuffers);

   static void CheckpointCallback(void *ctx, int param);
   void CheckpointHandler();
   static void PacketSpaceAvailableCallback(void *ctx, int param);
   void PacketSpaceAvailableHandler();

   NEXUS_DISPLAYHANDLE GetDisplay() const
   {
      return m_display;
   }

   BEGL_WindowHandle GetWindowHandle() const
   {
      return m_windowHandle;
   }

   nxpl::NativeWindowInfo GetWindowInfo() const
   {
      return m_info;
   }

   void UpdateWindowInfo(nxpl::NativeWindowInfo &info)
   {
      m_info = info;
   }

   std::unique_ptr<nxpl::Bitmap> PopFreeQ()
   {
      return m_freeQ.Pop();
   }

   void PushFreeQ(std::unique_ptr<nxpl::Bitmap> bitmap)
   {
      m_freeQ.Push(std::move(bitmap));
   }

   std::unique_ptr<nxpl::DispItem> PopDispQ()
   {
      return m_dispQ.Pop();
   }

   void PushDispQ(std::unique_ptr<nxpl::DispItem> dispItem)
   {
      m_dispQ.Push(std::move(dispItem));
   }

   bool IsBouncing() const
   {
      return m_isBouncing;
   }

   std::unique_ptr<nxpl::Bitmap> PopGFDBackingQ()
   {
      return m_gfdbacking.Pop();
   }

   void PushGFDBackingQ(std::unique_ptr<nxpl::Bitmap> bitmap)
   {
      m_gfdbacking.Push(std::move(bitmap));
   }

   void CheckpointReset()
   {
      m_checkpointEvent.reset();
   }

   void CheckpointWait()
   {
      m_checkpointEvent.wait();
   }

   void PacketSpaceAvailableReset()
   {
      m_packetSpaceAvailableEvent.reset();
   }

   void PacketSpaceAvailableWait()
   {
      m_packetSpaceAvailableEvent.wait();
   }

   bool IsSecure() const
   {
      return m_secure;
   }

#if NEXUS_HAS_GRAPHICS2D
   NEXUS_Graphics2DHandle GetGFX() const
   {
      return m_gfx;
   }
#endif

private:
   BEGL_WindowHandle                                        m_windowHandle;
   NEXUS_DISPLAYHANDLE                                      m_display;
   nxpl::NativeWindowInfo                                   m_info;

   helper::MessageQueue<nxpl::Bitmap>                       m_freeQ;
   helper::MessageQueue<nxpl::DispItem>                     m_dispQ;

   helper::MessageQueue<nxpl::Bitmap>                       m_gfdbacking;
   bool                                                     m_isBouncing;

   helper::Semaphore                                        m_checkpointEvent;
   helper::Semaphore                                        m_packetSpaceAvailableEvent;

   bool                                                     m_secure;

#if NEXUS_HAS_GRAPHICS2D
   NEXUS_Graphics2DHandle                                   m_gfx;
#endif

   std::unique_ptr<nxpl::Worker>                            m_worker;
};

}
