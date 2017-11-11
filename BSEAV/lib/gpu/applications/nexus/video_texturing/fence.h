/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/
#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <chrono>

namespace video_texturing
{

////////////////////////////////////////////////////////////////////////////////////////////////
// Helper class to wrap an EGL Fence object
////////////////////////////////////////////////////////////////////////////////////////////////
class Fence
{
public:
   Fence();
   ~Fence() { Destroy(); }

   void Create();
   void Destroy();
   void Wait();
   bool Valid() { return m_fence != EGL_NO_SYNC_KHR; }
   bool Signaled() const;

   static float AverageFenceWaitMS();

private:
   EGLSyncKHR m_fence;

   static float    m_totalFenceWaitSecs;
   static uint32_t m_numFenceWaits;

   std::chrono::time_point<std::chrono::steady_clock> m_createTime;

   // Extension function pointers
   PFNEGLCREATESYNCKHRPROC     m_eglCreateSyncKHRFunc;
   PFNEGLDESTROYSYNCKHRPROC    m_eglDestroySyncKHRFunc;
   PFNEGLCLIENTWAITSYNCKHRPROC m_eglClientWaitSyncKHRFunc;
   PFNEGLGETSYNCATTRIBKHRPROC  m_eglGetSyncAttribSyncKHRFunc;
};

} // namespace
