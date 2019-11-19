/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#ifndef __SWAPCHAIN_H__
#define __SWAPCHAIN_H__

#include "fence_interface.h"
#include "surface_interface.h"
#include "queue.h"
#include "windowinfo.h"

#include <EGL/begl_displayplatform.h>

#include <stdbool.h>

typedef struct SwapchainSurface
{
   BEGL_PixmapInfo   pixmap_info;
   bool              secure;
   unsigned          swap_interval;
   int               age;
   int               render_fence;
   int               display_fence;
   WindowInfo        windowInfo;
   struct list       link;
   uint8_t           native_surface[0]; /* variable size buffer */
} SwapchainSurface;

enum
{
   MAX_SWAP_BUFFERS = 3
};

typedef struct Swapchain
{
   const FenceInterface *fence_interface;
   const SurfaceInterface *surface_interface;

   struct queue render_queue;
   struct queue display_queue;

   unsigned num_surfaces;
   SwapchainSurface *surfaces;
} Swapchain;

bool SwapchainCreate(Swapchain *swapchain,
      const FenceInterface *fence_interface,
      const SurfaceInterface *surface_interface,
      size_t swapchain_size);

void SwapchainDestroy(Swapchain *swapchain);

void SwapchainPoison(Swapchain *swapchain);

SwapchainSurface *SwapchainDequeueRenderSurface(
      Swapchain *swapchain, const BEGL_PixmapInfo *requested,
      bool secure);

void SwapchainEnqueueDisplaySurface(Swapchain *swapchain,
      SwapchainSurface *surface);

SwapchainSurface *SwapchainDequeueDisplaySurface(
      Swapchain *swapchain);

void SwapchainEnqueueRenderSurface(Swapchain *swapchain,
      SwapchainSurface *surface);

#endif /* __SWAPCHAIN_H__ */
