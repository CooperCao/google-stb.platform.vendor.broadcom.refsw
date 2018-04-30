/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#if EMBEDDED_SETTOP_BOX && defined(VK_USE_PLATFORM_DISPLAY_KHR) && !defined(SINGLE_PROCESS) && defined(NXCLIENT_SUPPORT)

#ifndef NEXUS_HAS_SURFACE
static_assert(false, "Vulkan NxClient platform requires Nexus surface support");
#endif

#include <cstring>
#include <climits>

#include "V3DPlatformNxClient.h"
#include "Common.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "DeviceMemory.h"
#include "Image.h"
#include "SwapchainKHR.h"
#include "Options.h"

#include "libs/platform/v3d_scheduler.h"
#include "libs/platform/v3d_parallel.h"
#include "libs/util/profile/profile.h"

#include "nexus_core_utils.h"
#include "nexus_surface_client.h"
#include "nexus_platform.h"

// The design is that we have one display with one (or possibly more)
// planes which are actually NEXUS surface clients. The SD secondary
// display is not being considered in this implementation.
//
// The current display mode set by the server is the only display mode
// reported and no display mode changes are supported through this
// interface.
//
// Images are not pushed to the NXClientSurface until their semaphores have
// been signalled. This includes mailbox presentation mode, i.e.
// an image cannot replace an earlier image waiting to be displayed
// (and hence allow that earlier image to be re-aquired by the application)
// until it's semaphores have been signalled.
//
// The dequeue and image re-acquisition by the application is driven
// entirely by the NxClientSurface recycle mechanism. That is, we do not
// return images back to the application which have not yet been recycled to us.
//
// This avoids requiring any asynchronous fence operations to know when
// an image is no longer being presented, but means that the dequeue may
// potentially block for longer preventing work being done on the next
// set of command buffers.
//
// In mailbox box we force the recycle (rather than waiting for a callback)
// when a new image is pushed to the front of the NxClientSurface's queue,
// to make an image available to the application as soon as possible.

// NOTE: a deliberate C style cast here to cope with the different
// definitions of VkDisplayKHR in 32 and 64 bit builds.
constexpr auto BVkMainDisplayHandle = (VkDisplayKHR)1u;

LOG_DEFAULT_CAT("bvk::V3DPlatform");

namespace bvk
{
//////////////////////////////////////////////////////////////////////////
// Some basic overloaded helpers for NEXUS/Vulkan rectangles, region, extents

static inline bool operator!=(const VkExtent2D &a, const VkExtent2D &b)
{
   return (a.width != b.width || a.height != b.height);
}

static inline bool operator!=(const NEXUS_Rect &a, const VkRect2D &b)
{
   return a.x      != b.offset.x
       || a.y      != b.offset.y
       || a.width  != b.extent.width
       || a.height != b.extent.height;
}

static inline bool operator!=(const NEXUS_Rect &a, const VkExtent2D &b)
{
   return a.x      != 0
       || a.y      != 0
       || a.width  != b.width
       || a.height != b.height;
}

// NOTE: you cannot override operator= outside of class scope
static inline void Assign(NEXUS_Rect &a, const VkRect2D &b)
{
   a.x      = b.offset.x;
   a.y      = b.offset.y;
   a.width  = b.extent.width;
   a.height = b.extent.height;
}

static inline void Assign(NEXUS_Rect &a, const VkExtent2D &b)
{
   a.x      = 0;
   a.y      = 0;
   a.width  = b.width;
   a.height = b.height;
}

static inline void Assign(NEXUS_SurfaceRegion &a, const VkExtent2D &b)
{
   a.width  = b.width;
   a.height = b.height;
}

//////////////////////////////////////////////////////////////////////////
// Implementation of NxClientSurface class which abstracts the presentation
// of swapchain images on a NEXUS surface compositor surface. There is one
// of these created for each Vulkan "plane" we report through the platform
// class.
void V3DPlatformNxClient::NxClientSurface::RecycledCallback(void *ctx, int param)
{
   (void)param;
   assert(ctx != nullptr);

   std::lock_guard<std::mutex> lock { s_validContextMutex };

   ctx = xorContextPointer(ctx);
   if (s_validContextPointers.find(ctx) != s_validContextPointers.end())
      static_cast<V3DPlatformNxClient::NxClientSurface*>(ctx)->Recycle();
}

V3DPlatformNxClient::NxClientSurface::NxClientSurface()
{
   log_trace("%s (this = %p)", __FUNCTION__, this);

   NxClient_AllocSettings allocSettings;
   NxClient_GetDefaultAllocSettings(&allocSettings);
   allocSettings.surfaceClient = 1;

   NEXUS_Error err = NxClient_Alloc(&allocSettings, &m_allocResults);
   if (err != NEXUS_SUCCESS)
     throw std::bad_alloc();

   m_clientHandle = NEXUS_SurfaceClient_Acquire(ClientID());

   NxClient_GetSurfaceClientComposition(ClientID(), &m_composition);

   NEXUS_SurfaceClientSettings clientSettings;
   NEXUS_SurfaceClient_GetSettings(m_clientHandle, &clientSettings);

   clientSettings.orientation            = NEXUS_VideoOrientation_e2D;

   // We cannot really allow composition bypass as it will result in
   // perpixel/post multiplied alpha blending, as set in the graphics
   // display, of the surface and we have no control over it. As we
   // cannot really know when the bypass will actually happen we cannot
   // sensibly report the right alpha mode capabilities back to the
   // application.
   clientSettings.allowCompositionBypass = false;
   clientSettings.recycled.callback      = RecycledCallback;
   clientSettings.recycled.context       = xorContextPointer(this);

   err = NEXUS_SurfaceClient_SetSettings(m_clientHandle, &clientSettings);

   if (err != NEXUS_SUCCESS)
      throw std::runtime_error("Cannot configure NEXUS SurfaceClient settings\n");
}

V3DPlatformNxClient::NxClientSurface::~NxClientSurface()
{
   std::lock_guard<std::mutex> lock { m_recycleMutex };

   log_trace("%s (this = %p)", __FUNCTION__, this);

   NEXUS_SurfaceClientSettings clientSettings;
   NEXUS_SurfaceClient_GetSettings(m_clientHandle, &clientSettings);

   // Stop the recycling
   clientSettings.recycled.callback = NULL;
   clientSettings.recycled.context  = NULL;

   NEXUS_SurfaceClient_SetSettings(m_clientHandle, &clientSettings);

   // Do a recycle now to get back anything currently on the recycle queue
   DoRecycle();

   // Destroy recycled images
   ClearAcquireQueue();

   // This will release the surface clients ref count on NEXUS_Surface handles
   // that are still in the "push" queue.
   NEXUS_SurfaceClient_Release(m_clientHandle);

   // Current surface must be cleared from nxserver
   NEXUS_SurfaceClient_Clear(m_clientHandle);

   // Destroy images that were still pushed for presentation on release
   for (auto h: m_presentedHandles)
      DestroySurfaceHandle(h);

   NxClient_Free(&m_allocResults);

   // Note: at the point this object is being destroyed our owner has already
   //       removed all entries from the valid context set, so no callbacks
   //       can call into any objects during the destruction process. Hence
   //       there is no need to remove these pointers from it here.
   for (auto d: m_deferredPresentations)
      ::delete d;
}

void V3DPlatformNxClient::NxClientSurface::SetSwapchain(
      SwapchainKHR     *chain,
      const VkExtent2D &visibleRegion)
{
   std::lock_guard<std::mutex> lock { m_recycleMutex };

   // No images from the old swapchain can be acquired again, so we release
   // them and their backing memory now.
   ClearAcquireQueue();

   // After this any images recycled for the old swapchain will be destroyed
   // by the recycle code.
   m_swapchainHandles.clear();

   // If we are switching to a new swapchain unrelated to anything previously
   // seen on the surface then force the composition parameters to be updated
   m_updateCompositionParams = (m_currentSwapchain == nullptr && chain != nullptr);

   // If we are changing to a child swapchain (which must be created from the
   // same ICD surface as the current one) then only update the composition if
   // the chain create params that effect the composition are different from the
   // parent chain.
   if (m_currentSwapchain != nullptr && chain != nullptr)
   {
      if (m_currentSwapchain->CreateInfo().compositeAlpha != chain->CreateInfo().compositeAlpha
       || m_currentSwapchain->CreateInfo().imageExtent != chain->CreateInfo().imageExtent
       || m_visibleRegion != visibleRegion)
         m_updateCompositionParams = true;
   }

   m_currentSwapchain     = chain;
   m_visibleRegion        = visibleRegion;
   m_swapchainImagePushed = false;

   for (auto i: chain->PlatformImageData())
   {
      m_swapchainHandles.push_back(i.handle);
      m_acquireQueue.push(i.handle);
   }
   m_recycled.notify_one();
}

void V3DPlatformNxClient::NxClientSurface::ClearCurrentSwapchain()
{
   std::lock_guard<std::mutex> lock { m_recycleMutex };

   // No images from the current swapchain can be acquired again, so we release
   // them and their backing memory now.
   ClearAcquireQueue();

   // After this any images recycled for the current swapchain will be destroyed
   // by the recycle code.
   m_swapchainHandles.clear();
   m_currentSwapchain = nullptr;
}

void V3DPlatformNxClient::NxClientSurface::PresentationJobCallback(void *ctx)
{
   std::lock_guard<std::mutex> lock { s_validContextMutex };

   ctx = xorContextPointer(ctx);
   auto itr = s_validContextPointers.find(ctx);
   if (itr != s_validContextPointers.end())
   {
      // Trusting that if ctx is still valid so is all of its contents.
      auto d = static_cast<DeferredDisplayData *>(ctx);
      (void)d->nxClientSurface->DisplayNexusSurface(d);
      s_validContextPointers.erase(itr);
      ::delete d;
   }
}

VkResult V3DPlatformNxClient::NxClientSurface::DisplayNexusSurface(
      NEXUS_SurfaceHandle            nexusSurface,
      const SchedDependencies       &waitJobs,
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
      // it; however, the consolation is that the app is about to die anyway
      // and NxServer will clean up the resources when that happens.
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   d->nxClientSurface    = this;
   d->nexusSurface       = nexusSurface;
   d->pushToHeadOfQueue  = false;
   d->updateClientParams = false;

   if (presentInfo != nullptr && presentInfo->sType == VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR)
      d->presentInfo = *presentInfo;
   else
      d->presentInfo.sType = VK_STRUCTURE_TYPE_MAX_ENUM;

   // NOTE: We must lock the context mutex first to match the locking
   //       order in the presentation callback/DisplaySurface path,
   //       otherwise we can deadlock. This does mean we hold on to
   //       the context lock for longer than we might like here.
   std::unique_lock<std::mutex> contextLock { s_validContextMutex };
   std::unique_lock<std::mutex> recycleLock { m_recycleMutex };

   // NOTE: that we can legitimately be asked to display a Nexus surface
   //       related to an acquired image from an "old" swapchain as long
   //       as the current swapchain is a descendant. When the surface
   //       is recycled it will be immediately destroyed (as it can not be
   //       acquired again).
   if (ValidSwapchainHandle(nexusSurface))
   {
      // If we need to change the composition parameters from the previous
      // swapchain and this is the first image pushed from the new swapchain,
      // make the composition changes and push the image to the head of the
      // surface compositor's queue. This will immediately recycle any of the
      // old swapchain's images pushed to the Nexus surface but not yet on
      // display.
      if (!m_swapchainImagePushed)
      {
         if (m_updateCompositionParams)
         {
            SetSurfaceCompositionParameters();
            m_updateCompositionParams = false;
            d->updateClientParams     = true;
            d->pushToHeadOfQueue      = true;
         }

         // Update the surface present mode on the first image to be
         // presented from a swapchain (rather than when a swapchain is set),
         // so images presented from an old swapchain use their correct mode.
         m_presentMode = m_currentSwapchain->CreateInfo().presentMode;
      }
   }
   else
   {
      // If we have pushed an image from the current swapchain, pushing an image
      // from the old swapchain will now return out of date, regardless of the
      // compositor configuration. It makes no sense to allow images to be pushed
      // out of order.
      if (m_swapchainImagePushed)
      {
         ::delete d;
         return VK_ERROR_OUT_OF_DATE_KHR;
      }
   }

   if (m_presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      d->pushToHeadOfQueue = true;

   // Record the presentation for cleanup purposes
   m_presentedHandles.insert(nexusSurface);
   m_deferredPresentations.insert(d);

   auto deps = waitJobs;
   deps += m_lastPresentationJob;
   if (!v3d_scheduler_jobs_reached_state(&deps, V3D_SCHED_DEPS_COMPLETED, true))
   {
      s_validContextPointers.insert(d);
      m_lastPresentationJob = v3d_scheduler_submit_usermode_job(
                                 &deps,
                                 PresentationJobCallback,
                                 xorContextPointer(d));

      return VK_SUCCESS;
   }

   // All dependencies already satisfied so just do the display immediately.
   recycleLock.unlock();
   contextLock.unlock();

   VkResult ret = DisplayNexusSurface(d);
   ::delete d;

   return ret;
}

VkResult V3DPlatformNxClient::NxClientSurface::DisplayNexusSurface(const DeferredDisplayData *d)
{
   std::unique_lock<std::mutex> lock { m_recycleMutex };

   // First some housekeeping, whatever happens we need to forget about the
   // data pointer as it will be destroyed after this call and we do not
   // want the surface class destroying it again on closedown.
   m_deferredPresentations.erase(d);

   // The position and source clipping should really be pushed down with the
   // image handle so they only take effect when this specific image gets displayed.
   //
   // TODO: Talk to David Erickson about this.
   bool updateClient = d->updateClientParams;
   if (HavePresentationParametersChanged(d->presentInfo))
   {
      SetPresentationParameters(d->presentInfo);
      updateClient = true;
   }

   if (updateClient)
      NxClient_SetSurfaceClientComposition(ClientID(), &m_composition);

   NEXUS_Error err = NEXUS_SurfaceClient_PushSurface(m_clientHandle,
                                                     d->nexusSurface,
                                                     NULL,
                                                     d->pushToHeadOfQueue);
   if (err != NEXUS_SUCCESS)
   {
      m_presentedHandles.erase(d->nexusSurface);

      if (ValidSwapchainHandle(d->nexusSurface))
      {
         // In this case the image can get re-acquired although if the surface
         // is in a bad state that may not be useful; however by pushing it on
         // the acquire queue we keep hold of it so it will get cleaned up if
         // the swapchain gets replaced or the instance and hence the platform
         // is destroyed.
         m_acquireQueue.push(d->nexusSurface);
         m_recycled.notify_one();
         return VK_ERROR_SURFACE_LOST_KHR;
     }
     else
     {
        // When we return out of date the image cannot be re-acquired by the app
        // We need to clean this up now.
        DestroySurfaceHandle(d->nexusSurface);
        return VK_ERROR_OUT_OF_DATE_KHR;
     }
   }

   m_swapchainImagePushed = true;

   // Recycle any image replaced by this call in the queue [in mailbox mode]
   // immediately rather than waiting for the recycle callback.
   if (d->pushToHeadOfQueue)
   {
      bool notify = DoRecycle();

      // Done this way to avoid the waiter waking up and immediately blocking
      // on the mutex again.
      lock.unlock();
      if (notify)
         m_recycled.notify_one();
   }

   return VK_SUCCESS;
}

NEXUS_SurfaceHandle V3DPlatformNxClient::NxClientSurface::AcquireNexusSurface(uint64_t timeout)
{
   std::unique_lock<std::mutex> lock { m_recycleMutex };

   auto canAcquire = [this]{return !m_acquireQueue.empty();};

   if (timeout == UINT64_MAX)
   {
      m_recycled.wait(lock, canAcquire);
   }
   else
   {
      std::chrono::nanoseconds t { timeout };
      if (!m_recycled.wait_for(lock, t, canAcquire))
         return nullptr; // Timeout
   }

   assert(!m_acquireQueue.empty());

   NEXUS_SurfaceHandle h = m_acquireQueue.front();
   m_acquireQueue.pop();
   return h;
}

bool V3DPlatformNxClient::NxClientSurface::DoRecycle()
{
   size_t numRecycled = std::max(static_cast<size_t>(1u), m_swapchainHandles.size());

   NEXUS_SurfaceHandle surfaceList[numRecycled];

   NEXUS_Error err = NEXUS_SurfaceClient_RecycleSurface(m_clientHandle,
      surfaceList,
      numRecycled,
      &numRecycled);

   // If we have never actually pushed anything to the surface then we get an error
   // this can happen after running non-display based tests.
   if (err != NEXUS_SUCCESS)
      return false;

   bool notify = false;
   for (size_t i = 0; i < numRecycled; i++)
   {
     m_presentedHandles.erase(surfaceList[i]);

     // If we get callbacks for surfaces belonging to old swapchains we
     // destroy the surface handles and their backing memory here.
     if (ValidSwapchainHandle(surfaceList[i]))
     {
        m_acquireQueue.push(surfaceList[i]);
        notify = true;
     }
     else
     {
        DestroySurfaceHandle(surfaceList[i]);
     }
   }

   return notify;
}

void V3DPlatformNxClient::NxClientSurface::Recycle()
{
   std::unique_lock<std::mutex> lock { m_recycleMutex };

   bool notify = DoRecycle();

   // Done this way to avoid the waiter waking up and immediately blocking
   // on the mutex again.
   lock.unlock();
   if (notify)
      m_recycled.notify_one();
}

void V3DPlatformNxClient::NxClientSurface::ClearAcquireQueue()
{
   log_trace("%s", __FUNCTION__);
   while(!m_acquireQueue.empty())
   {
      NEXUS_SurfaceHandle h = m_acquireQueue.front();
      DestroySurfaceHandle(h);
      m_acquireQueue.pop();
   }
}

bool V3DPlatformNxClient::NxClientSurface::HavePresentationParametersChanged(
      const VkDisplayPresentInfoKHR &presentInfo)
{
   if (presentInfo.sType == VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR)
   {
      return (m_composition.position != presentInfo.dstRect
           || m_composition.clipRect != presentInfo.srcRect);
   }
   else if (m_currentSwapchain != nullptr)
   {
      // NOTE: even if we are presenting an image from an old swapchain this is
      //       still valid as descendant swapchains must be created from the same
      //       surface as their parent.
      auto swapCI = m_currentSwapchain->CreateInfo();
      const auto *displaySurface = FromSurfaceKHR<SurfaceDisplay>(swapCI.surface);

      return (m_composition.position != displaySurface->imageExtent
           || m_composition.clipRect != VkRect2D { 0, 0, 0, 0 });
   }

   // Force update to something sensible if the swapchain has been destroyed
   return true;
}

void V3DPlatformNxClient::NxClientSurface::SetPresentationParameters(
      const VkDisplayPresentInfoKHR &presentInfo)
{
   if (presentInfo.sType == VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR)
   {
      Assign(m_composition.position, presentInfo.dstRect);
      Assign(m_composition.clipRect, presentInfo.srcRect);
   }
   else if (m_currentSwapchain)
   {
      auto swapCI = m_currentSwapchain->CreateInfo();
      const auto *displaySurface = FromSurfaceKHR<SurfaceDisplay>(swapCI.surface);

      Assign(m_composition.position, displaySurface->imageExtent);
      Assign(m_composition.clipRect, VkRect2D { 0, 0, 0, 0 });
   }
   else
   {
      // If the swapchain has been destroyed do something sensible for already
      // queued images.
      Assign(m_composition.position, VkRect2D { 0, 0,
                                                m_composition.virtualDisplay.width,
                                                m_composition.virtualDisplay.height});
      Assign(m_composition.clipRect, VkRect2D { 0, 0, 0, 0 });
   }
}

void V3DPlatformNxClient::NxClientSurface::SetSurfaceCompositionParameters()
{
   auto swapCI = m_currentSwapchain->CreateInfo();
   const auto *displaySurface = FromSurfaceKHR<SurfaceDisplay>(swapCI.surface);

   Assign(m_composition.virtualDisplay, m_visibleRegion);
   Assign(m_composition.clipBase,       swapCI.imageExtent);
   Assign(m_composition.position,       displaySurface->imageExtent);
   Assign(m_composition.clipRect,       VkRect2D { 0, 0, 0, 0 });

   log_trace("%s:", __FUNCTION__);
   log_trace("   vdisp (%ux%u) cbase (%ux%u)",
         m_composition.virtualDisplay.width, m_composition.virtualDisplay.height,
         m_composition.clipBase.width, m_composition.clipBase.height);

   m_composition.zorder = displaySurface->planeStackIndex;

   uint32_t alpha = std::nearbyint(displaySurface->globalAlpha * 255.0f);
   alpha = std::min(255u, alpha);
   m_composition.constantColor = (alpha << 24) | 0xffffff;

   m_composition.alphaPremultiplySourceEnabled = false;

   auto alphaMode = GetAlphaMode(m_currentSwapchain);
   switch (alphaMode)
   {
      case VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR:
      {
         log_trace("   Alpha Mode OPAQUE");
         static NEXUS_BlendEquation color = {
               NEXUS_BlendFactor_eOne,
               NEXUS_BlendFactor_eSourceColor,
               /* false / NEXUS_BlendFactor_eZero for the rest */
         };
         static NEXUS_BlendEquation alpha = {
               NEXUS_BlendFactor_eOne,
               NEXUS_BlendFactor_eOne,
               /* false / NEXUS_BlendFactor_eZero for the rest */
         };
         m_composition.colorBlend = color;
         m_composition.alphaBlend = alpha;
         break;
      }
      case VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR:
      {
         log_trace("   Alpha Mode GLOBAL");
         static NEXUS_BlendEquation color = {
               NEXUS_BlendFactor_eConstantAlpha,
               NEXUS_BlendFactor_eSourceColor,
               false, // +
               NEXUS_BlendFactor_eInverseConstantAlpha,
               NEXUS_BlendFactor_eDestinationColor,
               /* false / NEXUS_BlendFactor_eZero for the rest */
         };
         static NEXUS_BlendEquation alpha = {
               NEXUS_BlendFactor_eOne,
               NEXUS_BlendFactor_eConstantAlpha,
               false, // +
               NEXUS_BlendFactor_eInverseConstantAlpha,
               NEXUS_BlendFactor_eDestinationAlpha,
               /* false / NEXUS_BlendFactor_eZero for the rest */
         };
         m_composition.colorBlend = color;
         m_composition.alphaBlend = alpha;
         break;
      }
      case VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR:
         // NOTE: README FIRST IF YOU THINK THE ALPHA BLENDING IS WRONG
         //       You are correct, sort of...
         //
         //       Nexus surface compositor gives you no control over the final
         //       blend equation of the composed framebuffer with the video and
         //       background. The NEXUS_Display graphics window color blend
         //       equations default to AsCs + (1-As)Cd. So the final composed
         //       framebuffer will have its color components multiplied by alpha
         //       again as it is displayed.
         //
         //       The entire surface compositor view of blending, exposing the
         //       full blitter functionality to clients, seems broken. It should have
         //       only exposed non-premult, premult and global alpha with "src over"
         //       blend equations and then the entire final framebuffer contents would
         //       always be pre-multiplied. In that case the final display blend
         //       equations could be set appropriately.
         //
         //       However the Nexus APIs seem far too ingrained now to make any kind
         //       of substantial change to the surface compositor behaviour, so what
         //       do we do? The composition set here is correct, anything else would
         //       produce garbage in the framebuffer and we have no ability to
         //       de-multiply the final color result of the blend.
         log_trace("   Alpha Mode PER PIXEL");
         m_composition.alphaPremultiplySourceEnabled = true;
         // Fall through
      case VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR:
      {
         log_trace("   Alpha Mode PREMULTIPLIED");
         static NEXUS_BlendEquation color = {
               NEXUS_BlendFactor_eOne,
               NEXUS_BlendFactor_eSourceColor,
               false, // +
               NEXUS_BlendFactor_eInverseSourceAlpha,
               NEXUS_BlendFactor_eDestinationColor,
               /* false / NEXUS_BlendFactor_eZero for the rest */
         };
         static NEXUS_BlendEquation alpha = {
               NEXUS_BlendFactor_eOne,
               NEXUS_BlendFactor_eSourceAlpha,
               false, // +
               NEXUS_BlendFactor_eInverseSourceAlpha,
               NEXUS_BlendFactor_eDestinationAlpha,
               /* false / NEXUS_BlendFactor_eZero for the rest */
         };
         m_composition.colorBlend = color;
         m_composition.alphaBlend = alpha;
         break;
      }
      default:
         unreachable();
   }
}

//////////////////////////////////////////////////////////////////////////
// Platform class implementation for NxClient
void V3DPlatformNxClient::NxClientCallback(void *ctx, int param)
{
   (void)param;

   std::lock_guard<std::mutex> lock { s_validContextMutex };

   ctx = xorContextPointer(ctx);
   if (s_validContextPointers.find(ctx) != s_validContextPointers.end())
      static_cast<V3DPlatformNxClient *>(ctx)->DisplayChanged();
}

V3DPlatformNxClient::V3DPlatformNxClient()
{
   log_trace("%s", __FUNCTION__);

   NxClient_JoinSettings joinSettings;
   NxClient_GetDefaultJoinSettings(&joinSettings);

   snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "Vulkan Display");
   joinSettings.mode = NEXUS_ClientMode_eVerified;

   NEXUS_Error err = NxClient_Join(&joinSettings);
   if (err != NEXUS_SUCCESS)
      throw std::runtime_error("NxClient_Join() failed\n");

   m_displayProperties.display = BVkMainDisplayHandle;
   m_displayProperties.displayName = "Main Display";

   // We could potentially get real dimensions from a HDMI connected display
   // but those EDID values are typically garbage anyway.
   m_displayProperties.physicalDimensions = { 1, 1 };

   // Again on HDMI this could be set to the native supported display mode
   // dimensions. But as we are not allowing the display mode to change
   // through Vulkan there is no point, this will get set to the current
   // display mode size instead later.
   m_displayProperties.physicalResolution   = { 0, 0 };
   m_displayProperties.supportedTransforms  = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   m_displayProperties.planeReorderPossible = true;
   m_displayProperties.persistentContent    = false;

   // Set display settings change callback and get current display settings
   NxClient_CallbackThreadSettings settings;
   NxClient_GetDefaultCallbackThreadSettings(&settings);

   settings.displaySettingsChanged.callback = NxClientCallback;
   settings.displaySettingsChanged.context  = xorContextPointer(this);
   settings.displaySettingsChanged.param    = 0;

   err = NxClient_StartCallbackThread(&settings);
   if (err != NEXUS_SUCCESS)
      throw std::runtime_error("NxClient_StartCallbackThread() failed\n");

   // But get the actual current settings now
   DisplayChanged(); // NOTE: locks m_displayMutex

   try
   {
      // We must do this before we try and use Options
      V3DPlatformNexusCommon::CommonInitialize();

      m_numPlanes = Options::numNxClientSurfaces;
      log_trace("   Creating %u planes", m_numPlanes);
      m_surfaces.resize(m_numPlanes, nullptr);
      for (auto &s: m_surfaces)
      {
         s = ::new NxClientSurface();

         std::lock_guard<std::mutex> contextLock { s_validContextMutex };
         s_validContextPointers.insert(s);
      }

   }
   catch (...)
   {
      std::lock_guard<std::mutex> displayLock { m_displayMutex };
      Cleanup();
      throw;
   }
}

V3DPlatformNxClient::~V3DPlatformNxClient()
{
   log_trace("%s (this = %p)", __FUNCTION__, this);
   std::lock_guard<std::mutex> lock { m_displayMutex };
   Cleanup();
}

void V3DPlatformNxClient::Cleanup()
{
   V3DPlatformNexusCommon::CommonTerminate();
   NxClient_StopCallbackThread();

   for (auto s : m_surfaces)
      ::delete s;

   NxClient_Uninit();
}

void V3DPlatformNxClient::DisplayChanged()
{
   std::lock_guard<std::mutex> lock { m_displayMutex };

   log_trace("Display Changed Notification");

   NxClient_GetDisplaySettings(&m_displaySettings);

   // TODO: should any change to the display format cause queue/dequeue
   //       to report VK_ERROR_OUT_OF_DATE or VK_SUBOPTIMAL_KHR?
   m_mainDisplayFormat = m_displaySettings.format;

   NEXUS_VideoFormatInfo info;
   NEXUS_VideoFormat_GetInfo(m_mainDisplayFormat, &info);

   auto extent = VkExtent2D { info.width, info.height };
   m_displayProperties.physicalResolution           = extent;
   m_displayModeProperties.parameters.visibleRegion = extent;

   // Nexus multiplies the vertical refresh by 100, Vulkan wants it multiplied
   // by 1000.
   m_displayModeProperties.parameters.refreshRate = info.verticalFreq * 10;

   // NOTE: a deliberate C style cast here to cope with the different
   // definitions of VkDisplayModeKHR in 32 and 64 bit builds.
   m_displayModeProperties.displayMode = (VkDisplayModeKHR)m_mainDisplayFormat;
}

void V3DPlatformNxClient::AttachSwapchain(
   const VkAllocationCallbacks *pCallbacks,
   Device                      *pDevice,
   SwapchainKHR                *chain)
{
   std::lock_guard<std::mutex> lock { m_displayMutex };

   const auto ci = chain->CreateInfo();

   const auto *displaySurface = FromSurfaceKHR<SurfaceDisplay>(ci.surface);
   assert(displaySurface->planeIndex < m_numPlanes);

   auto nxClientSurface = m_surfaces[displaySurface->planeIndex];
   ValidateNewSwapchain(chain, nxClientSurface->GetCurrentSwapchain());

   // Spec 1.0.32, section 29.6 WSI Swapchain:
   // Description of vkCreateSwapchainKHR states that if this calls fails
   // then as a consequence it is no longer possible to acquire any images
   // from an oldswapchain. So clean up anything waiting to be acquired now,
   // freeing up memory in the Nexus graphics heap to give us a chance of
   // being able to allocate the images for the new swapchain.
   //
   // NOTE: in this platform we do not just call ClearCurrentSwapchain() as
   //       that currently has other side effects with regards to reprogramming
   //       the surface configuration as we transition between images still
   //       being displayed from the old swapchain and the new swapchain.
   //       We might want to revisit this later.
   log_trace("%s: Releasing unacquired images from oldSwapchain", __FUNCTION__);

   auto *oldChain = fromHandle<SwapchainKHR>(ci.oldSwapchain);
   while (oldChain != nullptr)
   {
      NEXUS_SurfaceHandle h = nxClientSurface->AcquireNexusSurface(0);
      if (h == nullptr)
         break;

      uint32_t index = FindImageDataIndexFromHandle(oldChain, h);
      DestroyImageData(oldChain->PlatformImageData()[index]);
   }

   try
   {
      AllocateSwapchainImages(pCallbacks, pDevice, chain);
   }
   catch(...)
   {
      // Now we really have to clear out the old swapchain state
      nxClientSurface->ClearCurrentSwapchain();
      throw;
   }

   // Store a reference to the NEXUS surface in the chain
   chain->PlatformSwapchainData().nxClientSurface = nxClientSurface;

   // And tell the NEXUS surface client wrapper about them so we can
   // acquire them for use.
   nxClientSurface->SetSwapchain(chain, m_displayModeProperties.parameters.visibleRegion);
}

void V3DPlatformNxClient::DetachSwapchain(
   const VkAllocationCallbacks *pCallbacks,
   Device                      *pDevice,
   SwapchainKHR                *chain)
{
   // NOTE: This is called by the Swapchain class in two circumstances.
   //  (1) When a valid swapchain is really being destroyed, so we have to
   //      destroy things.
   //  (2) When an exception happens during Swapchain creation, in which case
   //      we shouldn't be doing anything as we should have already cleaned up
   //      ourselves.
   std::lock_guard<std::mutex> lock { m_displayMutex };

   log_trace("DetachSwapchain: chain = %p", chain);

   DestroySwapchainImages(pCallbacks, chain);

   // If this chain is still the current chain for the surface then clear it
   // which will release any images waiting to be acquired and will cause any
   // future recycled images from the chain to be released at that point.
   //
   // TODO: this will not force the destruction of the image(s) currently being
   //       presented, which will continue to be pushed through the NEXUS
   //       surface compositor to the display. Is this necessary in this case
   //       or should we force them to be destroyed as well?
   auto nxClientSurface = chain->PlatformSwapchainData().nxClientSurface;

   if (nxClientSurface && nxClientSurface->IsCurrentSwapchain(chain))
      nxClientSurface->ClearCurrentSwapchain();
}

VkResult V3DPlatformNxClient::QueueNexusSurface(
      SwapchainKHR                  *chain,
      NEXUS_SurfaceHandle            nexusSurface,
      SchedDependencies             &deps,
      const VkDisplayPresentInfoKHR *presentInfo)
{
   std::lock_guard<std::mutex> lock { m_displayMutex };

   auto nxClientSurface = chain->PlatformSwapchainData().nxClientSurface;
   return nxClientSurface->DisplayNexusSurface(nexusSurface, deps, presentInfo);
}

NEXUS_SurfaceHandle V3DPlatformNxClient::DequeueNexusSurface(
   SwapchainKHR *chain,
   uint64_t      timeout)
{
   std::lock_guard<std::mutex> lock { m_displayMutex };

   auto nxClientSurface = chain->PlatformSwapchainData().nxClientSurface;

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
   assert(nxClientSurface->IsCurrentSwapchain(chain));

   return nxClientSurface->AcquireNexusSurface(timeout);
}

VkResult V3DPlatformNxClient::GetDisplayPlaneProperties(
   uint32_t                      *pPropertyCount,
   VkDisplayPlanePropertiesKHR   *pProperties)
{
   std::lock_guard<std::mutex> lock { m_displayMutex };

   log_trace("%s: m_numPlanes = %u", __FUNCTION__, m_numPlanes);

   if (pProperties == nullptr)
   {
      *pPropertyCount = m_surfaces.size();
      return VK_SUCCESS;
   }

   uint32_t idx = 0;
   for(auto s: m_surfaces)
   {
      if (*pPropertyCount <= idx)
         return VK_INCOMPLETE;

      pProperties[idx].currentDisplay = m_displayProperties.display;
      pProperties[idx].currentStackIndex = s->GetStackIndex();
      idx++;
   }

   *pPropertyCount = idx;
   return VK_SUCCESS;
}

VkResult V3DPlatformNxClient::GetDisplayPlaneCapabilities(
   VkDisplayModeKHR               mode,
   uint32_t                       planeIndex,
   VkDisplayPlaneCapabilitiesKHR *pCapabilities)
{
   log_trace("%s", __FUNCTION__);

   assert(planeIndex < m_numPlanes);

   NEXUS_VideoFormatInfo info;
   GetDisplayModeInfo(mode, &info);

   VkExtent2D maxSrcExtent;

   // Pick some arbitrary source limits based on the mode so we don't end
   // up with unsupported/bandwidth limited downscales.
   if (info.width <= 960)
      maxSrcExtent = VkExtent2D { 960, 540 };
   else if (info.width <= 1920)
      maxSrcExtent = VkExtent2D { 1920, 1080 };
   else
      maxSrcExtent = VkExtent2D { 4096, 2160 };

   // The NEXUS surface compositor allows us the full range
   // of alpha blending options with other graphics surface clients.
   pCapabilities->supportedAlpha = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR
                                 | VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR
                                 | VK_DISPLAY_PLANE_ALPHA_GLOBAL_BIT_KHR
                                 | VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;

   pCapabilities->minSrcPosition = VkOffset2D { 0, 0 };
   pCapabilities->maxSrcPosition = VkOffset2D {
      static_cast<int32_t>(maxSrcExtent.width)  - 1,
      static_cast<int32_t>(maxSrcExtent.height) - 1
   };

   pCapabilities->minSrcExtent   = VkExtent2D { 1, 1 };
   pCapabilities->maxSrcExtent   = maxSrcExtent;
   pCapabilities->minDstPosition = VkOffset2D { 0, 0 }; // TODO can this be negative in NSC?
   pCapabilities->maxDstPosition = VkOffset2D {
      static_cast<int32_t>(info.width)  - 1,
      static_cast<int32_t>(info.height) - 1
   };

   pCapabilities->minDstExtent   = VkExtent2D { 1, 1 };
   pCapabilities->maxDstExtent   = VkExtent2D { info.width, info.height };
   return VK_SUCCESS;
}

} // namespace bvk

#endif // EMBEDDED_SETTOP_BOX
