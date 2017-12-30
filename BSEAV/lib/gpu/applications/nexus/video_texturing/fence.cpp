/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "fence.h"

#include <chrono>

namespace video_texturing
{

////////////////////////////////////////////////////////////////////////////////////////////////
// Helper class to wrap a GLES Fence object
////////////////////////////////////////////////////////////////////////////////////////////////
float    Fence::m_totalFenceWaitSecs = 0.0f;
uint32_t Fence::m_numFenceWaits = 0;

Fence::Fence() : m_fence(EGL_NO_SYNC_KHR)
{
   m_eglCreateSyncKHRFunc = (PFNEGLCREATESYNCKHRPROC)eglGetProcAddress("eglCreateSyncKHR");
   m_eglDestroySyncKHRFunc = (PFNEGLDESTROYSYNCKHRPROC)eglGetProcAddress("eglDestroySyncKHR");
   m_eglClientWaitSyncKHRFunc = (PFNEGLCLIENTWAITSYNCKHRPROC)eglGetProcAddress("eglClientWaitSyncKHR");
   m_eglGetSyncAttribSyncKHRFunc = (PFNEGLGETSYNCATTRIBKHRPROC)eglGetProcAddress("eglGetSyncAttribKHR");

   if (!m_eglCreateSyncKHRFunc || !m_eglDestroySyncKHRFunc ||
       !m_eglClientWaitSyncKHRFunc || !m_eglGetSyncAttribSyncKHRFunc)
      throw "EGL fences not supported. Cannot continue";
}

void Fence::Create()
{
   Destroy();
   m_fence = m_eglCreateSyncKHRFunc(eglGetDisplay(EGL_DEFAULT_DISPLAY), EGL_SYNC_FENCE_KHR, NULL);
}

void Fence::Destroy()
{
   if (m_fence != EGL_NO_SYNC_KHR)
   {
      m_eglDestroySyncKHRFunc(eglGetDisplay(EGL_DEFAULT_DISPLAY), m_fence);
      m_fence = EGL_NO_SYNC_KHR;
   }
}

void Fence::Wait()
{
   m_numFenceWaits++;

   if (m_fence != EGL_NO_SYNC_KHR)
   {
      // Record fence wait time for debug purposes
      std::chrono::time_point<std::chrono::steady_clock> start, end;
      start = std::chrono::steady_clock::now();

      // Wait for the fence to signal
      m_eglClientWaitSyncKHRFunc(eglGetDisplay(EGL_DEFAULT_DISPLAY), m_fence,
                                 EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, EGL_FOREVER_KHR);

      end = std::chrono::steady_clock::now();
      std::chrono::duration<float> elapsedSeconds = end - start;
      m_totalFenceWaitSecs += elapsedSeconds.count();
   }
}

bool Fence::Signaled() const
{
   EGLint value;
   m_eglGetSyncAttribSyncKHRFunc(eglGetDisplay(EGL_DEFAULT_DISPLAY), m_fence, EGL_SYNC_STATUS_KHR, &value);
   return value == EGL_SIGNALED_KHR ? true : false;
}

float Fence::AverageFenceWaitMS()
{
   if (m_numFenceWaits == 0)
      return 0.0f;

   return static_cast<float>(m_totalFenceWaitSecs * 1000.0 / m_numFenceWaits);
}

} // namespace
