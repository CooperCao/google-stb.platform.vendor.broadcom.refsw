/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#if EMBEDDED_SETTOP_BOX && defined(VK_USE_PLATFORM_DISPLAY_KHR) && defined(SINGLE_PROCESS)

#ifndef NEXUS_HAS_DISPLAY
static_assert(false, "Vulkan Nexus single process platform requires display support");
#endif

#ifndef NEXUS_HAS_SURFACE
static_assert(false, "Vulkan Nexus single process platform requires surface support");
#endif

#include <cstring>
#include <dlfcn.h>

#include "V3DPlatformNexusExclusive.h"
#include "Common.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "DeviceMemory.h"
#include "Image.h"
#include "SwapchainKHR.h"

#include "libs/platform/v3d_scheduler.h"
#include "libs/platform/v3d_parallel.h"
#include "libs/util/profile/profile.h"

#include "nexus_memory.h"
#include "nexus_platform.h"
#include "nexus_core_utils.h"

// The design is that we have one display with one plane which is the
// Nexus display module's graphics "framebuffer".
//
// The Nexus display handle to be used can be provided to the platform via
// the globally visible symbol "bvk_nexus_display_handle". If this symbol
// exists and is non-null then is must be a valid handle. If the symbol
// does not exist or is set to NULL then the platform will attempt to open
// the Nexus display module itself with a default 720p main display
// configuration with HDMI enabled.
//
// The current display mode set when the display is opened or obtained
// from the application provided global symbol is the only
// display mode reported and no display mode changes are supported either
// through the Vulkan display interface or by the application changing
// the display configuration while the Vulkan platform object is in existence.
//
// Images are not finally pushed into the display queue until their semaphores
// have been signalled. This includes mailbox presentation mode, i.e.
// an image cannot replace an earlier image waiting to be displayed
// (and hence allow that earlier image to be re-aquired by the application)
// until it's semaphores have been signalled.
//
// Images are not made available for re-acquisition by the application until
// it is known that they are no longer being accessed by the display hardware
// or will never be accessed by the display hardware, i.e. the image was
// sucessfully replaced as the next image to be displayed before a vsync
// happened in mailbox mode.
//
// This avoids requiring any asynchronous fence operations to know when
// an image is no longer being presented, but means that the dequeue may
// potentially block for longer preventing work being done on the next
// set of command buffers.

LOG_DEFAULT_CAT("bvk::V3DPlatform");

// Set to 1 to log the graphics settings (src->dst rects and alpha) for each
// presented image.
#define LOG_GRAPHICS_SETTINGS 0

namespace bvk
{

static bool InitNexus()
{
   NEXUS_PlatformStatus platformStatus;
   NEXUS_Error err = NEXUS_Platform_GetStatus(&platformStatus);

   if (err == NEXUS_NOT_INITIALIZED)
   {
      NEXUS_PlatformSettings platformSettings;
      NEXUS_Platform_GetDefaultSettings(&platformSettings);
      platformSettings.openFrontend = false;

      NEXUS_Error err = NEXUS_Platform_Init(&platformSettings);
      if (err != NEXUS_SUCCESS)
         throw std::runtime_error("NEXUS_Platform_Init() failed\n");

      return true;
   }

   return false;
}

static inline NEXUS_DisplayHandle GetDisplay()
{
   auto displaySym = static_cast<NEXUS_DisplayHandle *>(dlsym(nullptr, "bvk_nexus_display_handle"));
   if (displaySym == nullptr || *displaySym == nullptr)
      return nullptr;

   NEXUS_DisplayStatus status;
   NEXUS_Error err = NEXUS_Display_GetStatus(*displaySym, &status);
   if (err != NEXUS_SUCCESS)
      throw std::runtime_error("Application provided NEXUS display handle is not valid");

   return *displaySym;
}

static NEXUS_DisplayHandle OpenDisplay()
{
   NEXUS_DisplayHandle display = nullptr;

   NEXUS_PlatformConfiguration platformConfig;
   NEXUS_Platform_GetConfiguration(&platformConfig);

   NEXUS_DisplaySettings displaySettings;
   NEXUS_Display_GetDefaultSettings(&displaySettings);
   // Fixed default config, the application must provide a configured display
   // handle to us if it wants anything different.
   displaySettings.format     = NEXUS_VideoFormat_e720p;
   displaySettings.background = 0xff000080;
   display = NEXUS_Display_Open(0, &displaySettings);

   if (display == nullptr)
   {
      printf("NEXUS_Display_Open() failed: running Vulkan \"headless\"\n");
   }
   else
   {
      if (NEXUS_NUM_COMPONENT_OUTPUTS > 0)
         NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));

      if (NEXUS_NUM_HDMI_OUTPUTS > 0)
         NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));

      // As we own the display, initially set the graphics output to be disabled
      NEXUS_GraphicsSettings settings;
      NEXUS_Display_GetGraphicsSettings(display, &settings);
      settings.enabled = false;
      settings.visible = false;
      settings.alpha   = 255u;
      NEXUS_Display_SetGraphicsSettings(display, &settings);
   }
   return display;
}

V3DPlatformNexusExclusive::V3DPlatformNexusExclusive()
{
   log_trace("%s (this = %p)", __FUNCTION__, this);

   // Defaults for headless bringup.
   m_numPlanes                              = 0;
   m_displayProperties.display              = VK_NULL_HANDLE;
   m_displayProperties.displayName          = "NO DISPLAY";
   m_displayProperties.physicalResolution   = { 0, 0 };
   m_displayProperties.physicalDimensions   = { 1, 1 };
   m_displayProperties.supportedTransforms  = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   m_displayProperties.planeReorderPossible = false;
   m_displayProperties.persistentContent    = false;

   m_displayModeProperties.parameters.visibleRegion = VkExtent2D { 0, 0 };
   m_displayModeProperties.parameters.refreshRate   = 0;

   m_uninitNexus = InitNexus();
   // First see if the application has made a display handle available to us
   m_display = GetDisplay();
   // If not then try and open the display ourselves
   if (m_display == nullptr)
   {
      m_display = OpenDisplay();
      m_closeDisplay = (m_display != nullptr);
   }

   if (m_display != nullptr)
   {
      m_numPlanes = 1;

      // NOTE: a deliberate C style cast here to cope with the different
      //       definitions of VkDisplayKHR in 32 and 64 bit builds.
      m_displayProperties.display = (VkDisplayKHR)m_display;
      m_displayProperties.displayName = "Main Display";

      NEXUS_DisplaySettings displaySettings;
      NEXUS_Display_GetSettings(m_display, &displaySettings);
      m_mainDisplayFormat = displaySettings.format;

      NEXUS_VideoFormatInfo info;
      NEXUS_VideoFormat_GetInfo(m_mainDisplayFormat, &info);

      NEXUS_DisplayCapabilities cap;
      NEXUS_GetDisplayCapabilities(&cap);
      m_displayCompressed = (cap.display[0].graphics.compression == NEXUS_GraphicsCompression_eRequired);
      log_trace("%s: Compressed display = %s", __FUNCTION__, TF(m_displayCompressed));

      auto extent = VkExtent2D { info.width, info.height };
      m_displayProperties.physicalResolution           = extent;
      m_displayModeProperties.parameters.visibleRegion = extent;

      // Nexus multiplies the vertical refresh by 100, Vulkan wants
      // it multiplied by 1000.
      m_displayModeProperties.parameters.refreshRate = info.verticalFreq * 10;
   }

   // NOTE: a deliberate C style cast here to cope with the different
   //       definitions of VkDisplayModeKHR in 32 and 64 bit builds.
   m_displayModeProperties.displayMode = (VkDisplayModeKHR)m_mainDisplayFormat;

   V3DPlatformNexusCommon::CommonInitialize();
}

V3DPlatformNexusExclusive::~V3DPlatformNexusExclusive()
{
   log_trace("%s (this = %p)", __FUNCTION__, this);
   std::lock_guard<std::mutex> lock { m_displayMutex };

   V3DPlatformNexusCommon::CommonTerminate();

   if (m_display != nullptr)
   {
      NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eAuto);
      if (m_currentSwapchain != nullptr)
         CleanupNexusFramebufferUsage();
      if (m_closeDisplay)
         NEXUS_Display_Close(m_display);
   }

   ClearCurrentSwapchain();

   for (auto d: m_deferredPresentations)
   {
      DestroySurfaceHandle(d->nexusSurface);
      ::delete d;
   }

   while (!m_displayQueue.empty())
   {
      auto d = m_displayQueue.front();
      m_displayQueue.pop();
      DestroySurfaceHandle(d->nexusSurface);
      ::delete d;
   }

   DestroySurfaceHandle(m_displayedSurface);
   DestroySurfaceHandle(m_pendingSurface);

   if (m_uninitNexus)
      NEXUS_Platform_Uninit();
}

void V3DPlatformNexusExclusive::ClearCurrentSwapchain()
{
   log_trace("%s: m_currentSwapchain = %p", __FUNCTION__, m_currentSwapchain);

   if (m_currentSwapchain != nullptr)
   {
      while(!m_acquireQueue.empty())
      {
         NEXUS_SurfaceHandle h = m_acquireQueue.front();
         m_acquireQueue.pop();

         uint32_t index = FindImageDataIndexFromHandle(m_currentSwapchain, h);
         DestroyImageData(m_currentSwapchain->PlatformImageData()[index]);
      }
   }
   else
   {
      demand(m_acquireQueue.empty());
   }

   m_swapchainHandles.clear();
   m_currentSwapchain = nullptr;
   m_swapchainImagePushed = false;
}

void V3DPlatformNexusExclusive::SetupNexusFramebufferUsage()
{
   log_trace("%s", __FUNCTION__);
   // Save the current graphics settings so we can restore them later.
   NEXUS_Display_GetGraphicsSettings(m_display, &m_initialGraphicsSettings);
   // If something else already has the Nexus framebuffer enabled we don't
   // try and use it.
   if (m_initialGraphicsSettings.enabled)
      throw VK_ERROR_NATIVE_WINDOW_IN_USE_KHR;

   m_graphicsSettings = m_initialGraphicsSettings;
   m_graphicsSettings.frameBufferCallback.callback = nullptr;
   m_graphicsSettings.enabled = true;
   m_graphicsSettings.visible = false;
   m_graphicsSettings.alpha   = 255u;

   if (NEXUS_Display_SetGraphicsSettings(m_display, &m_graphicsSettings) != NEXUS_SUCCESS)
      throw VK_ERROR_SURFACE_LOST_KHR;

   NEXUS_CallbackDesc vsync;
   NEXUS_CallbackDesc_Init(&vsync);
   vsync.callback = VSyncCallback;
   vsync.context  = xorContextPointer(this);

   if (NEXUS_Display_SetVsyncCallback(m_display, &vsync) != NEXUS_SUCCESS)
   {
      // Try and put things back as they were, do not worry if it fails as well
      NEXUS_Display_SetGraphicsSettings(m_display, &m_initialGraphicsSettings);
      throw VK_ERROR_SURFACE_LOST_KHR;
   }

   NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eManual);
}

void V3DPlatformNexusExclusive::CleanupNexusFramebufferUsage()
{
   NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eAuto);
   NEXUS_Display_SetVsyncCallback(m_display, nullptr);
   NEXUS_Display_SetGraphicsSettings(m_display, &m_initialGraphicsSettings);
}

void V3DPlatformNexusExclusive::AttachSwapchain(
   const VkAllocationCallbacks *pCallbacks,
   Device                      *pDevice,
   SwapchainKHR                *chain)
{
   std::lock_guard<std::mutex> lock { m_displayMutex };

   // If we have no swapchain then we will take over the Nexus display
   // vsync interrupt if the new chain creation succeeds. This allows a
   // degree of flexibility in switching between using an EGL window surface
   // and a Vulkan swapchain in the same app.
   bool needFramebufferSetup = (m_currentSwapchain == nullptr);

   ValidateNewSwapchain(chain, m_currentSwapchain);

   // Spec 1.0.32, section 29.6 WSI Swapchain:
   // Description of vkCreateSwapchainKHR states that if this calls fails
   // then as a consequence it is no longer possible to acquire any images
   // from oldSwapchain. Cleaning them up now frees memory in the Nexus
   // graphics heap to give us a chance of being able to allocate the images
   // for the new swapchain.
   ClearCurrentSwapchain();
   AllocateSwapchainImages(pCallbacks, pDevice, chain);

   try
   {
      for (auto i: chain->PlatformImageData())
      {
         m_swapchainHandles.push_back(i.handle);
         m_acquireQueue.push(i.handle);
      }

      if (needFramebufferSetup)
         SetupNexusFramebufferUsage();
   }
   catch(...)
   {
      m_swapchainHandles.clear();
      while(!m_acquireQueue.empty())
         m_acquireQueue.pop();

      throw; // The caller will catch this and clean up the actual images
   }

   m_surfaceAvailable.notify_one();
   m_currentSwapchain = chain;
}

void V3DPlatformNexusExclusive::DetachSwapchain(
   const VkAllocationCallbacks *pCallbacks,
   Device                      *pDevice,
   SwapchainKHR                *chain)
{
   std::lock_guard<std::mutex> lock { m_displayMutex };

   log_trace("DetachSwapchain: chain = %p", chain);

   DestroySwapchainImages(pCallbacks, chain);
   if (m_currentSwapchain == chain)
   {
      ClearCurrentSwapchain();
      CleanupNexusFramebufferUsage();
      // As we have released the use of the framebuffer and cleared
      // the vsync callback registration we need to cleanup the outstanding
      // display surfaces handles immediately.
      DestroySurfaceHandle(m_displayedSurface);
      m_displayedSurface = nullptr;
      DestroySurfaceHandle(m_pendingSurface);
      m_pendingSurface = nullptr;
   }
}

///////////////////////////////////////////////////////////////////////////
// Queue and Dequeue helpers called by the common Nexus base class swapchain
// implementation.
VkResult V3DPlatformNexusExclusive::QueueNexusSurface(
      SwapchainKHR                  *chain,
      NEXUS_SurfaceHandle            nexusSurface,
      SchedDependencies             &deps,
      const VkDisplayPresentInfoKHR *presentInfo)
{
   DeferredDisplayData *d;
   try
   {
      d = ::new DeferredDisplayData;
   }
   catch (const std::bad_alloc &)
   {
      // We should really put the handle back on the acquire queue here, but
      // that might run out of memory and throw again. So we are going to lose
      // track of this handle and the application will not be able to re-acquire
      // it; however, the consolation is that the app is about to die anyway.
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   d->platform          = this;
   d->nexusSurface      = nexusSurface;
   d->pushToHeadOfQueue = false;

   if (presentInfo != nullptr && presentInfo->sType == VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR)
      d->presentInfo = *presentInfo;
   else
      d->presentInfo.sType = VK_STRUCTURE_TYPE_MAX_ENUM;

   // NOTE: We must lock the context mutex first to match the locking
   //       order in the presentation callback/MoveToDisplayQueue path,
   //       otherwise we can deadlock.
   std::lock_guard<std::mutex> contextLock { s_validContextMutex };
   std::lock_guard<std::mutex> recycleLock { m_displayMutex };

   // NOTE: that we can legitimately be asked to display a Nexus surface
   //       related to an acquired image from an "old" swapchain as long
   //       as the current swapchain is a descendant. When the surface
   //       is recycled it will be immediately destroyed (as it can not be
   //       acquired again).
   if (ValidSwapchainHandle(nexusSurface))
   {
      if (!m_swapchainImagePushed)
      {
         // Update the surface present mode on the first image to be
         // presented from a swapchain (rather than when a swapchain is set),
         // so images presented from an old swapchain use their correct mode.
         m_presentMode = m_currentSwapchain->CreateInfo().presentMode;
      }
   }
   else
   {
      // If we have pushed an image from the current swapchain, pushing an image
      // from the old swapchain will now return out of date. It makes no sense
      // to allow images to be pushed out of order.
      if (m_swapchainImagePushed)
      {
         ::delete d;
         return VK_ERROR_OUT_OF_DATE_KHR;
      }
   }

   if (m_presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      d->pushToHeadOfQueue = true;

   m_deferredPresentations.insert(d);

   auto localDeps = deps;
   localDeps += m_lastPresentationJob;
   s_validContextPointers.insert(d);
   m_lastPresentationJob = v3d_scheduler_submit_usermode_job(
                              &localDeps,
                              PresentationJobCallback,
                              xorContextPointer(d));

   return VK_SUCCESS;
}

NEXUS_SurfaceHandle V3DPlatformNexusExclusive::DequeueNexusSurface(
   SwapchainKHR *chain,
   uint64_t      timeout)
{
   // We cannot acquire images from the surface unless it is associated with
   // the requested swapchain.
   //
   // vkCreateSwapchainKHR states that if an "oldSwapchain" is replaced, or an
   // attempt to replace it fails it is no longer possible to acquire images
   // from it. However vkAcquireNextImageKHR does not specify how this should
   // be handled if the application attempts it. Returning OUT_OF_DATE_KHR is
   // not the right answer because we can still present already acquired images
   // and we can only return OUT_OF_DATE_KHR if future presentation will fail.
   //
   // So our answer is to assert/crash/generally go badly wrong if the app
   // does this.
   assert(chain == m_currentSwapchain);

   std::unique_lock<std::mutex> lock { m_displayMutex };

   auto canAcquire = [this]{return !m_acquireQueue.empty();};

   if (timeout == UINT64_MAX)
   {
      m_surfaceAvailable.wait(lock, canAcquire);
   }
   else
   {
      std::chrono::nanoseconds t { timeout };
      if (!m_surfaceAvailable.wait_for(lock, t, canAcquire))
         return nullptr; // Timeout
   }

   assert(!m_acquireQueue.empty());

   NEXUS_SurfaceHandle h = m_acquireQueue.front();
   m_acquireQueue.pop();
   return h;
}

VkResult V3DPlatformNexusExclusive::GetDisplayPlaneProperties(
   uint32_t                      *pPropertyCount,
   VkDisplayPlanePropertiesKHR   *pProperties)
{
   log_trace("%s", __FUNCTION__);

   // Only one plane in exclusive mode.
   if (pProperties == nullptr)
   {
      *pPropertyCount = 1;
      return VK_SUCCESS;
   }

   if (*pPropertyCount == 0)
      return VK_INCOMPLETE;

   *pPropertyCount = 1;
   pProperties->currentDisplay    = m_displayProperties.display;
   pProperties->currentStackIndex = 0;
   return VK_SUCCESS;
}

VkResult V3DPlatformNexusExclusive::GetDisplayPlaneCapabilities(
   VkDisplayModeKHR               mode,
   uint32_t                       planeIndex,
   VkDisplayPlaneCapabilitiesKHR *pCapabilities)
{
   log_trace("%s", __FUNCTION__);

   assert(planeIndex == 0);

   NEXUS_VideoFormatInfo info;
   GetDisplayModeInfo(mode, &info);

   pCapabilities->supportedAlpha =
      VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR    |
      VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR    |
      VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR |
      VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR;

   pCapabilities->minSrcPosition = VkOffset2D { 0, 0 };
   // On compressed framebuffers (which is now the norm) we cannot move
   // the clip offset from (0,0). While we could make this conditional
   // on us detecting a compressed framebuffer system, it seems confusing
   // and not very helpful to the customer writing an app to provide
   // different behaviours in exclusive mode based on this.
   pCapabilities->maxSrcPosition = VkOffset2D { 0, 0 };
   pCapabilities->minSrcExtent   = VkExtent2D { 1, 1 };
   pCapabilities->maxSrcExtent   = VkExtent2D { info.width, info.height };

   pCapabilities->minDstPosition = VkOffset2D { 0, 0 };

   // From BVDC_Window_SetDstRect
   constexpr unsigned BVDC_P_WIN_DST_OUTPUT_H_MIN = 16u;
   constexpr unsigned BVDC_P_WIN_DST_OUTPUT_V_MIN = 10u; // Using the minimum for interlaced display modes

   pCapabilities->maxDstPosition = VkOffset2D {
         static_cast<int32_t>(info.width - (BVDC_P_WIN_DST_OUTPUT_H_MIN + 1)),
         static_cast<int32_t>(info.width - (BVDC_P_WIN_DST_OUTPUT_V_MIN + 1))
      };

   pCapabilities->minDstExtent   = VkExtent2D { BVDC_P_WIN_DST_OUTPUT_H_MIN,
                                                BVDC_P_WIN_DST_OUTPUT_V_MIN };
   pCapabilities->maxDstExtent   = VkExtent2D { info.width, info.height };
   return VK_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
// Deferred presentation and display Queue Management
//
// Called when a deferred display presentation job's dependencies (semaphores)
// are satisfied, at which point the display data is moved from the
// deferred set to the display queue.
void V3DPlatformNexusExclusive::PresentationJobCallback(void *ctx)
{
   std::lock_guard<std::mutex> lock { s_validContextMutex };

   ctx = xorContextPointer(ctx);
   auto itr = s_validContextPointers.find(ctx);
   if (itr != s_validContextPointers.end())
   {
      // Trusting that if ctx is still valid so is all of its contents.
      auto d = static_cast<DeferredDisplayData *>(ctx);
      (void)d->platform->MoveToDisplayQueue(d);
      s_validContextPointers.erase(itr);
   }
}

bool V3DPlatformNexusExclusive::RecycleSwapchainHandle(NEXUS_SurfaceHandle h)
{
#if LOG_QUEUE_DEQUEUE
   log_trace("%s: %p", __FUNCTION__, (void*)h);
#endif
   if (ValidSwapchainHandle(h))
   {
      m_acquireQueue.push(h);
      return true; // notify waiters on queue required
   }
   // The swapchain must have been replaced so destroy the surface now.
   DestroySurfaceHandle(h);
   return false;
}

void V3DPlatformNexusExclusive::MoveToDisplayQueue(const DeferredDisplayData *d)
{
#if LOG_QUEUE_DEQUEUE
   log_trace("%s: nexus handle %p", __FUNCTION__, d->nexusSurface);
#endif

   std::unique_lock<std::mutex> lock { m_displayMutex };

   assert(m_deferredPresentations.find(d) != m_deferredPresentations.end());
   m_deferredPresentations.erase(d);

   if (!d->pushToHeadOfQueue)
   {
      // FIFO presentation, push to the class display queue and let the
      // vsync handler do the display management.
      m_displayQueue.push(d);
      return;
   }

   // Mailbox mode, try to display the image in place of anything we have
   // so far pushed to the Nexus framebuffer code.
   bool notify = false;

   // If we are transitioning between swapchains configured in FIFO and mailbox
   // mode, or are on a compressed display, we need to clear the display queue.
   while (!m_displayQueue.empty())
   {
      auto i = m_displayQueue.front();
      m_displayQueue.pop();

      notify = notify || RecycleSwapchainHandle(i->nexusSurface);
      ::delete i;
   }

   // The Nexus auto compression of framebuffers, when the platform requires
   // it, does not handle queueing faster than the display refresh rate. So
   // we have to put the buffer on the queue and let the vsync handler do the
   // work. This will not recycle images back to the application as quickly
   // as when there is no compression step.
   if (m_displayCompressed)
   {
      m_displayQueue.push(d);
   }
   else
   {
      NEXUS_SurfaceHandle handleToRecycle = d->nexusSurface; // In case of failure to display
      if (DisplayNexusSurface(d) == NEXUS_SUCCESS)
      {
         // We have sucessfully pushed a handle for display to the Nexus
         // framebuffer code. Now we have to update our class state and
         // potentially recycle something back to be acquired by the application.
         if (m_pendingSurface != nullptr)
         {
            NEXUS_GraphicsFramebufferStatus status;
            NEXUS_Display_GetGraphicsFramebufferStatus(m_display, m_pendingSurface, &status);

            // If we had a previous handle waiting to go on the display then
            // when we pushed the new handle one of two things happened inside
            // Nexus.
            //
            // 1. The pending handle was still pending and has been replaced in
            //    the Nexus framebuffer queue by the new one, in which case it's
            //    state will have been changed to eUnused and we recycle it.
            //
            // 2. There had been a display update at some point and the pending
            //    handle is now actually on the display, in which case the handle
            //    we thought was still on the display should now be recycled.
            if (status.state == NEXUS_GraphicsFramebufferState_eUnused)
            {
               handleToRecycle = m_pendingSurface;
            }
            else
            {
               handleToRecycle = m_displayedSurface;
               m_displayedSurface = m_pendingSurface;
            }
         }

         m_pendingSurface = d->nexusSurface;
      }

      notify = notify || RecycleSwapchainHandle(handleToRecycle);
      ::delete d;
   }

   if (notify)
   {
      lock.unlock();
      m_surfaceAvailable.notify_one();
   }
}

/////////////////////////////////////////////////////////////////////////
// Display handling
void V3DPlatformNexusExclusive::VSyncCallback(void *ctx, int param)
{
   (void)param;

   std::lock_guard<std::mutex> lock { s_validContextMutex };

   ctx = xorContextPointer(ctx);
   if (s_validContextPointers.find(ctx) != s_validContextPointers.end())
      static_cast<V3DPlatformNexusExclusive *>(ctx)->VSyncHandler();
}

void V3DPlatformNexusExclusive::VSyncHandler()
{
   std::unique_lock<std::mutex> lock { m_displayMutex };

   bool notify = false;
   if (m_displayedSurface != nullptr)
   {
      NEXUS_GraphicsFramebufferStatus status;
      NEXUS_Display_GetGraphicsFramebufferStatus(m_display, m_displayedSurface, &status);

      if (status.state == NEXUS_GraphicsFramebufferState_eUnused)
      {
         // A handle that we previously had marked as on the display has been
         // replaced and is no longer in use, so recycle it now.
         notify = notify || RecycleSwapchainHandle(m_displayedSurface);
         m_displayedSurface = nullptr;
      }
   }

   if (m_pendingSurface != nullptr)
   {
      NEXUS_GraphicsFramebufferStatus status;
      NEXUS_Display_GetGraphicsFramebufferStatus(m_display, m_pendingSurface, &status);

      // If a handle we had previously queued for display with the Nexus
      // framebuffer code has now actually made it onto the display, update
      // the class state to reflect that.
      //
      // If we are on a display with automatic compression going on underneath
      // us, we need to return the handle immediately if we want mailbox
      // presentation with a triple buffered swapchain to not be limited to the
      // display framerate. We are assuming the M2MC blit has completed during
      // the last vsync interval, because the status doesn't tell us that,
      // otherwise we could potentially see visual artifacts.
      if (status.state == NEXUS_GraphicsFramebufferState_eDisplayed ||
          (m_displayCompressed && status.state == NEXUS_GraphicsFramebufferState_eUnused))
      {
         if (m_displayCompressed)
            notify = notify || RecycleSwapchainHandle(m_pendingSurface);
         else
            m_displayedSurface = m_pendingSurface;

         m_pendingSurface = nullptr;
      }
   }

   if (m_pendingSurface == nullptr && !m_displayQueue.empty())
   {
      // If there is not already a handle waiting to go onto the display
      // and we have one in the class display queue, then try to queue
      // it for display with the Nexus framebuffer code. This is the
      // implementation of FIFO presentation, where all images are presented
      // in order and are gated by the display vsync.
      auto d = m_displayQueue.front();
      m_displayQueue.pop();

      // It is possible, after detatching a swapchain which resets the
      // framebuffer and turns off the vsync, that we will still get
      // outstanding callbacks with a surface waiting in the
      // display queue. If so then we just want to immediately recycle it
      // and not turn the framebuffer back on again as part of the display call,
      // which would prevent another swapchain being bound (it would look like
      // the surface was in use by another API). This isn't theoretical,
      // vk_display_tests provoked this condition between test instances.
      if (m_currentSwapchain != nullptr && DisplayNexusSurface(d) == NEXUS_SUCCESS)
         m_pendingSurface = d->nexusSurface;
      else
         notify = notify || RecycleSwapchainHandle(d->nexusSurface);

      ::delete d;
   }

   if (notify)
   {
      lock.unlock();
      m_surfaceAvailable.notify_one();
   }
}

NEXUS_Error V3DPlatformNexusExclusive::DisplayNexusSurface(const DeferredDisplayData *d)
{
   NEXUS_Error ret;

   // Unlike in the NxClient platform implementation we can set the framebuffer
   // and the graphics configuration as a single operation. So in this case
   // we setup the graphics configuration parameters on each frame so we do
   // not have to have the complication of only doing the changes as the
   // swapchain changes. The additional work involved is not thought to be
   // an issue as the Nexus implementation tracks the current state and
   // does not make calls down to the Magnum VDC driver unless something
   // has actually changed since the last frame.
   m_graphicsSettings.visible = true;

   if (d->presentInfo.sType == VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR)
   {
      m_graphicsSettings.clip.x      = d->presentInfo.srcRect.offset.x;
      m_graphicsSettings.clip.y      = d->presentInfo.srcRect.offset.y;
      m_graphicsSettings.clip.width  = d->presentInfo.srcRect.extent.width;
      m_graphicsSettings.clip.height = d->presentInfo.srcRect.extent.height;

      m_graphicsSettings.position.x      = d->presentInfo.dstRect.offset.x;
      m_graphicsSettings.position.y      = d->presentInfo.dstRect.offset.y;
      m_graphicsSettings.position.width  = d->presentInfo.dstRect.extent.width;
      m_graphicsSettings.position.height = d->presentInfo.dstRect.extent.height;
   }
   else
   {
      // Default behaviour is to scale (up or down) to the full display size
      NEXUS_SurfaceStatus status;
      NEXUS_Surface_GetStatus(d->nexusSurface, &status);
      m_graphicsSettings.clip.x      = 0;
      m_graphicsSettings.clip.y      = 0;
      m_graphicsSettings.clip.width  = status.width;
      m_graphicsSettings.clip.height = status.height;

      m_graphicsSettings.position.x = 0;
      m_graphicsSettings.position.y = 0;

      auto visibleRegion = m_displayModeProperties.parameters.visibleRegion;
      m_graphicsSettings.position.width  = visibleRegion.width;
      m_graphicsSettings.position.height = visibleRegion.height;
   }

   // TODO: change scaling coefficients based on scale factor
   if (LOG_GRAPHICS_SETTINGS)
   {
      log_trace("Display (%u,%u,%u,%u) -> (%u,%u,%u,%u)",
         m_graphicsSettings.clip.x,
         m_graphicsSettings.clip.y,
         m_graphicsSettings.clip.width,
         m_graphicsSettings.clip.height,
         m_graphicsSettings.position.x,
         m_graphicsSettings.position.y,
         m_graphicsSettings.position.width,
         m_graphicsSettings.position.height);
   }
   // If we have a swapchain set the alpha, otherwise we must be displaying
   // the last images from a destroyed swapchain, so just leave the old
   // graphics settings as they were.
   if (m_currentSwapchain != nullptr)
   {
      auto swapCI = m_currentSwapchain->CreateInfo();
      const auto *displaySurface = FromSurfaceKHR<SurfaceDisplay>(swapCI.surface);

      auto alphaMode = GetAlphaMode(m_currentSwapchain);
      switch (alphaMode)
      {
         case VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR:
         {
            if (LOG_GRAPHICS_SETTINGS)
               log_trace("   Alpha Mode OPAQUE");

            m_graphicsSettings.constantAlpha     = 255u;
            m_graphicsSettings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eOne;
            m_graphicsSettings.destBlendFactor   = NEXUS_CompositorBlendFactor_eZero;
            break;
         }
         case VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR:
         {
            if (LOG_GRAPHICS_SETTINGS)
               log_trace("   Alpha Mode GLOBAL");

            uint32_t alpha = std::nearbyint(displaySurface->globalAlpha * 255.0f);
            m_graphicsSettings.constantAlpha     = static_cast<uint8_t>(std::min(255u, alpha));
            m_graphicsSettings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eConstantAlpha;
            m_graphicsSettings.destBlendFactor   = NEXUS_CompositorBlendFactor_eInverseConstantAlpha;
            break;
         }
         case VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR:
         {
            if (LOG_GRAPHICS_SETTINGS)
               log_trace("   Alpha Mode PER PIXEL");

            m_graphicsSettings.constantAlpha     = 255u;
            m_graphicsSettings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eSourceAlpha;
            m_graphicsSettings.destBlendFactor   = NEXUS_CompositorBlendFactor_eInverseSourceAlpha;
            break;
         }
         case VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR:
         {
            if (LOG_GRAPHICS_SETTINGS)
               log_trace("   Alpha Mode PREMULTIPLIED");

            m_graphicsSettings.constantAlpha     = 255u;
            m_graphicsSettings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eConstantAlpha;
            m_graphicsSettings.destBlendFactor   = NEXUS_CompositorBlendFactor_eInverseSourceAlpha;
            break;
         }
         default:
            unreachable();
      }
   }

   ret = NEXUS_Display_SetGraphicsSettings(m_display, &m_graphicsSettings);
   if (ret != NEXUS_SUCCESS)
      return ret;

   ret = NEXUS_Display_SetGraphicsFramebuffer(m_display, d->nexusSurface);
   if (ret != NEXUS_SUCCESS)
      return ret;

   ret = NEXUS_DisplayModule_SetUpdateMode(NEXUS_DisplayUpdateMode_eNow);
   return ret;
}

} // namespace bvk

#endif // EMBEDDED_SETTOP_BOX
