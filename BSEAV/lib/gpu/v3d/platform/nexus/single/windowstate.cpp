/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "nexus_platform.h"

#if NEXUS_HAS_GRAPHICS2D
#include "nexus_graphics2d.h"
#endif

#include "windowstate.h"

namespace nxpl
{

WindowState::WindowState(void *context, BEGL_WindowHandle windowHandle, bool secure) :
   m_windowHandle(windowHandle),
   m_display(static_cast<NXPL_Display *>(context)->display),
   m_dispQ(),
   m_gfdbacking(),
   m_isBouncing(false),
   m_secure(secure),
   m_worker(std::unique_ptr<nxpl::Worker>(new nxpl::Worker(this,
         static_cast<NXPL_Display *>(context)->eventContext)))
{}

WindowState::~WindowState()
{
#if NEXUS_HAS_GRAPHICS2D
   if (m_gfx)
      NEXUS_Graphics2D_Close(m_gfx);
#endif
}

bool WindowState::Init(void *context, unsigned swapbuffers)
{
#if NEXUS_HAS_GRAPHICS2D
   /* get GFD0 heap and NEXUS_OFFSCREEN_SURFACE */
   NEXUS_HeapHandle gfd0Heap = NEXUS_Platform_GetFramebufferHeap(0);
   NEXUS_HeapHandle v3dHeap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);

   NEXUS_MemoryStatus gfd0Status, v3dStatus;
   NEXUS_Heap_GetStatus(gfd0Heap, &gfd0Status);
   NEXUS_Heap_GetStatus(v3dHeap, &v3dStatus);

   if (gfd0Status.memcIndex != v3dStatus.memcIndex)
   {
      auto nw = static_cast<nxpl::NativeWindowInfo*>(m_windowHandle);
      auto windowExtent = nw->GetExtent2D();

      for (int i = 0; i < 3; i++)
      {
         auto tmp = std::unique_ptr<nxpl::Bitmap>(
               new nxpl::Bitmap(context, gfd0Heap, windowExtent.GetWidth(),
                     windowExtent.GetHeight(), BEGL_BufferFormat_eA8B8G8R8));
         if (!tmp)
            return false;
         m_gfdbacking.Push(std::move(tmp));
      }

      m_gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
      if (m_gfx == NULL)
         return false;

      NEXUS_Graphics2DSettings gfxSettings;
      NEXUS_Graphics2D_GetSettings(m_gfx, &gfxSettings);
      gfxSettings.checkpointCallback.callback = CheckpointCallback;
      gfxSettings.checkpointCallback.context = this;
      gfxSettings.packetSpaceAvailable.callback = PacketSpaceAvailableCallback;
      gfxSettings.packetSpaceAvailable.context = this;
      NEXUS_Graphics2D_SetSettings(m_gfx, &gfxSettings);

      m_isBouncing = true;

      // double buffer only in bounce mode
      for (unsigned i = 0; i < 2; i++)
         m_freeQ.Push(std::unique_ptr<nxpl::Bitmap>{});
   }
   else
#endif
   {
      for (unsigned i = 0; i < swapbuffers; i++)
         m_freeQ.Push(std::unique_ptr<nxpl::Bitmap>{});
      m_gfx = NULL;
   }

   return true;
}

void WindowState::CheckpointCallback(void *ctx, int param __attribute__((unused)))
{
   static_cast<WindowState *>(ctx)->CheckpointHandler();
}

void WindowState::CheckpointHandler()
{
   m_checkpointEvent.notify();
}

void WindowState::PacketSpaceAvailableCallback(void *ctx, int param __attribute__((unused)))
{
   static_cast<WindowState *>(ctx)->PacketSpaceAvailableHandler();
}

void WindowState::PacketSpaceAvailableHandler()
{
   m_packetSpaceAvailableEvent.notify();
}

}
