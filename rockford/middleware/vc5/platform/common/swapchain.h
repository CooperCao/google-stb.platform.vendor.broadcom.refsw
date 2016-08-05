/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __SWAPCHAIN_H__
#define __SWAPCHAIN_H__

#include "fence_interface.h"
#include "surface_interface.h"
#include "queue.h"

#include <EGL/begl_displayplatform.h>

#include <stdbool.h>

struct SwapchainSurface
{
   BEGL_PixmapInfo   pixmap_info;
   bool              secure;
   unsigned          swap_interval;
   int               render_fence;
   int               display_fence;
   struct list       link;
   uint8_t           native_surface[0]; /* variable size buffer */
};

struct Swapchain
{
   const struct FenceInterface *fence_interface;
   const struct SurfaceInterface *surface_interface;

   struct queue render_queue;
   struct queue display_queue;

   struct SwapchainSurface *surfaces;
};

bool SwapchainCreate(struct Swapchain *swapchain,
      const struct FenceInterface *fence_interface,
      const struct SurfaceInterface *surface_interface,
      size_t swapchain_size);

void SwapchainDestroy(struct Swapchain *swapchain);

void SwapchainPoison(struct Swapchain *swapchain);

struct SwapchainSurface *SwapchainDequeueRenderSurface(
      struct Swapchain *swapchain, const BEGL_PixmapInfo *requested,
      bool secure);

void SwapchainEnqueueDisplaySurface(struct Swapchain *swapchain,
      struct SwapchainSurface *surface);

struct SwapchainSurface *SwapchainDequeueDisplaySurface(
      struct Swapchain *swapchain);

void SwapchainEnqueueRenderSurface(struct Swapchain *swapchain,
      struct SwapchainSurface *surface);

#endif /* __SWAPCHAIN_H__ */
