/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#if defined(VK_USE_PLATFORM_DISPLAY_KHR) && defined(SINGLE_PROCESS)

#include <condition_variable>
#include <algorithm>

#include "V3DPlatformNexusCommon.h"

#include "nexus_display.h"

namespace bvk
{

class V3DPlatformNexusExclusive : public V3DPlatformNexusCommon
{
public:
   V3DPlatformNexusExclusive();
   virtual ~V3DPlatformNexusExclusive();

   // Attach platform specific swapchain backing buffers to a set of images
   void AttachSwapchain(
      const VkAllocationCallbacks      *pCallbacks,
      Device                           *pDevice,
      SwapchainKHR                     *chain);

   // Detach platform specific swapchain stuff
   void DetachSwapchain(
      const VkAllocationCallbacks      *pCallbacks,
      Device                           *pDevice,
      SwapchainKHR                     *chain);

   VkResult GetDisplayPlaneProperties(
      uint32_t                      *pPropertyCount,
      VkDisplayPlanePropertiesKHR   *pProperties) override;

   VkResult GetDisplayPlaneCapabilities(
      VkDisplayModeKHR               mode,
      uint32_t                       planeIndex,
      VkDisplayPlaneCapabilitiesKHR *pCapabilities) override;

public:
   // Platform specific swapchain data. It is held in the swapchain.
   struct SwapchainData
   {
      // None Required
   };

private:
   struct DeferredDisplayData
   {
      V3DPlatformNexusExclusive *platform;
      NEXUS_SurfaceHandle        nexusSurface;
      VkDisplayPresentInfoKHR    presentInfo;
      bool                       pushToHeadOfQueue;
      bool                       updateGraphicsSettings;
   };

   // Implementation of step methods called by base class (De)QueueFrame
   VkResult QueueNexusSurface(
      SwapchainKHR                  *chain,
      NEXUS_SurfaceHandle            nexusSurface,
      SchedDependencies             &deps,
      const VkDisplayPresentInfoKHR *presentInfo);

   NEXUS_SurfaceHandle DequeueNexusSurface(SwapchainKHR *chain, uint64_t timeout);

   // Internal implementation helpers
   void ClearCurrentSwapchain();
   void SetupNexusFramebufferUsage();
   void CleanupNexusFramebufferUsage();

   static void PresentationJobCallback(void *ctx);
   void MoveToDisplayQueue(const DeferredDisplayData *d);

   static void VSyncCallback(void *ctx, int param);
   void VSyncHandler();

   NEXUS_Error DisplayNexusSurface(const DeferredDisplayData *d);

   bool ValidSwapchainHandle(NEXUS_SurfaceHandle h)
   {
      return std::find(m_swapchainHandles.begin(),
                        m_swapchainHandles.end(),
                        h) != m_swapchainHandles.end();
   }

   bool RecycleSwapchainHandle(NEXUS_SurfaceHandle h);

private:
   bool                   m_uninitNexus = false;
   bool                   m_closeDisplay = false;
   NEXUS_DisplayHandle    m_display = nullptr;
   NEXUS_GraphicsSettings m_initialGraphicsSettings;
   NEXUS_GraphicsSettings m_graphicsSettings;
   bool                   m_displayCompressed = false;

   JobID                  m_lastPresentationJob = 0;
   VkPresentModeKHR       m_presentMode;

   bool                   m_swapchainImagePushed = false;
   SwapchainKHR          *m_currentSwapchain = nullptr;

   // Copy of the surface handles in the current swapchain
   // TODO: review if this is as useful as it is in the NxClient platform
   std::vector<NEXUS_SurfaceHandle>        m_swapchainHandles;

   // Management of surfaces ready to be re-acquired by the application
   std::condition_variable                 m_surfaceAvailable;
   std::queue<NEXUS_SurfaceHandle>         m_acquireQueue;

   // Record of deferred structures not yet in the display queue so
   // they can be cleaned up if any are still outstanding on destruction.
   std::set<const DeferredDisplayData *>   m_deferredPresentations;
   // FIFO queue (in display order) of deferred presentations that have had
   // their semaphores all signalled and can now go on display.
   std::queue<const DeferredDisplayData *> m_displayQueue;
   // Handle of the surface that was on display the last time the vsync
   // handler callback was called.
   NEXUS_SurfaceHandle                     m_displayedSurface = nullptr;
   // Handle of the surface that we had set to go on display next the
   // last time the presentation job (in Mailbox mode) or vsync handler
   // (in FIFO mode) was called
   NEXUS_SurfaceHandle                     m_pendingSurface = nullptr;
};

// Whenever V3DPlatform is seen, it means V3DPlatformNexusExclusive
using V3DPlatform = V3DPlatformNexusExclusive;

} // namespace bvk

#endif // EMBEDDED_SETTOP_BOX
