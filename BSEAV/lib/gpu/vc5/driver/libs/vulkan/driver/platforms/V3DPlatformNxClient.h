/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#if defined(VK_USE_PLATFORM_DISPLAY_KHR) && !defined(SINGLE_PROCESS) && defined(NXCLIENT_SUPPORT)

#include <mutex>
#include <condition_variable>
#include <algorithm>

#include "V3DPlatformNexusCommon.h"
#include "SchedDependencies.h"

#include "nxclient.h"

namespace bvk
{

class V3DPlatformNxClient : public V3DPlatformNexusCommon
{
public:
   V3DPlatformNxClient();
   virtual ~V3DPlatformNxClient();

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

   // Display Extension Implementation
   VkResult GetDisplayPlaneProperties(
      uint32_t                      *pPropertyCount,
      VkDisplayPlanePropertiesKHR   *pProperties) override;

   VkResult GetDisplayPlaneCapabilities(
      VkDisplayModeKHR               mode,
      uint32_t                       planeIndex,
      VkDisplayPlaneCapabilitiesKHR *pCapabilities) override;

public:
   class NxClientSurface
   {
   public:
      NxClientSurface();
      ~NxClientSurface();

      NEXUS_SurfaceCompositorClientId ClientID() const { return m_allocResults.surfaceClient[0].id; }
      NEXUS_SurfaceClientHandle ClientHandle() const { return m_clientHandle; }
      uint32_t GetStackIndex() const { return m_composition.zorder; }

      void SetSwapchain(SwapchainKHR *chain, const VkExtent2D &visibleRegion);
      const SwapchainKHR *GetCurrentSwapchain() const { return m_currentSwapchain; }
      bool IsCurrentSwapchain(SwapchainKHR *chain) const { return chain == m_currentSwapchain; }
      void ClearCurrentSwapchain();

      NEXUS_SurfaceHandle AcquireNexusSurface(uint64_t timeout);
      VkResult DisplayNexusSurface(
         NEXUS_SurfaceHandle            nexusSurface,
         const SchedDependencies       &waitJobs,
         const VkDisplayPresentInfoKHR *presentInfo);

   private:
      // Asynchronous presentation support
      struct DeferredDisplayData
      {
         NxClientSurface        *nxClientSurface;
         NEXUS_SurfaceHandle     nexusSurface;
         VkDisplayPresentInfoKHR presentInfo;
         bool                    pushToHeadOfQueue;
         bool                    updateClientParams;
      };

      static void PresentationJobCallback(void *ctx);
      VkResult DisplayNexusSurface(const DeferredDisplayData *presentData);

      void SetSurfaceCompositionParameters();
      void SetPresentationParameters(const VkDisplayPresentInfoKHR &presentInfo);
      bool HavePresentationParametersChanged(const VkDisplayPresentInfoKHR &presentInfo);

      bool ValidSwapchainHandle(NEXUS_SurfaceHandle h)
      {
         return std::find(m_swapchainHandles.begin(),
                          m_swapchainHandles.end(),
                          h) != m_swapchainHandles.end();
      }

      static void RecycledCallback(void *ctx, int param);
      void Recycle();   // Takes lock and notifies condition variable if needed
      bool DoRecycle(); // Call with m_recycleMutex held, returns if notification required
      void ClearAcquireQueue(); // Call with m_recycleMutex held

      std::mutex                m_recycleMutex;
      std::condition_variable   m_recycled;

      NxClient_AllocResults     m_allocResults;
      NEXUS_SurfaceClientHandle m_clientHandle;
      NEXUS_SurfaceComposition  m_composition;

      // Tracking swapchain configuration changes and old swapchain
      // "out of date" presentation
      bool                      m_updateCompositionParams = true;
      bool                      m_swapchainImagePushed    = false;

      // A copy of the image handles that are owned by the current swapchain.
      // This removes any requirement for access synchronization with the
      // current swapchain object when it is being destroyed.
      std::vector<NEXUS_SurfaceHandle> m_swapchainHandles;

      // Handles owned by the current swapchain that can be acquired for use
      // by the application.
      std::queue<NEXUS_SurfaceHandle>  m_acquireQueue;

      // Handles either waiting to be or already pushed to the NEXUS surface
      // compositor and not yet recycled so we can cleanup images still in the
      // presentation queue (and on screen) during closedown.
      //
      // NOTE: m_presentedHandles may contain handles that are not
      //       owned by the current swapchain, i.e. they came from a previously
      //       associated swapchain but we still need to be able to clean them
      //       up at some point.
      std::set<NEXUS_SurfaceHandle>    m_presentedHandles;

      // Deferred presentation requests waiting for their dependencies
      // to complete. We have to hold a copy of the pointers to clean up
      // the data structure memory when closing down and not all presentations
      // have been completed.
      std::set<const DeferredDisplayData *> m_deferredPresentations;

      // The presentation queue has to internally make presentation requests
      // dependent on the previous one to ensure frames are displayed in order.
      JobID            m_lastPresentationJob = 0;

      SwapchainKHR    *m_currentSwapchain = nullptr;
      VkPresentModeKHR m_presentMode;

      // Display visible region at the point the current swapchain was set
      VkExtent2D       m_visibleRegion;
   };

   // Platform specific swapchain data. It is held in the swapchain.
   struct SwapchainData
   {
      // Have the swapchain hold a back reference to the
      // NxClientSurface object it is associated with, to avoid
      // looking it up through the swapchain create parameters
      // everywhere.
      NxClientSurface *nxClientSurface = nullptr;
   };

private:
   // Callback handling of display change notifications from NxServer
   static void NxClientCallback(void *ctx, int param);
   void DisplayChanged();

   void Cleanup(); // Must be called with m_displayMutex already held

   VkResult QueueNexusSurface(
      SwapchainKHR                  *chain,
      NEXUS_SurfaceHandle            nexusSurface,
      SchedDependencies             &deps,
      const VkDisplayPresentInfoKHR *presentInfo);

   NEXUS_SurfaceHandle DequeueNexusSurface(SwapchainKHR *chain, uint64_t timeout);

   NxClient_DisplaySettings       m_displaySettings;
   std::vector<NxClientSurface *> m_surfaces;
};

// Whenever V3DPlatform is seen, it means V3DPlatformNxClient
using V3DPlatform = V3DPlatformNxClient;

} // namespace bvk

#endif // EMBEDDED_SETTOP_BOX
