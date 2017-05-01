/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "swapchain.h"

#include <string.h>
#include <assert.h>

static bool create_surfaces(Swapchain *swapchain, size_t num_surfaces)
{
   if (queue_init(&swapchain->render_queue)
         && queue_init(&swapchain->display_queue))
   {
      size_t surface_size = sizeof(SwapchainSurface)
            + swapchain->surface_interface->sizeof_surface;
      swapchain->surfaces = calloc(num_surfaces, surface_size);
      if (swapchain->surfaces)
      {
         SwapchainSurface *p = swapchain->surfaces;
         for (size_t i = 0; i < num_surfaces; i++)
         {
            p->pixmap_info.format = BEGL_BufferFormat_INVALID;
            p->display_fence = swapchain->fence_interface->invalid_fence;
            p->render_fence = swapchain->fence_interface->invalid_fence;
            p->swap_interval = 1;

            queue_enqueue(&swapchain->render_queue, &p->link);
            p = (SwapchainSurface *) ((uintptr_t) p + surface_size);
         }
      }
   }
   return swapchain->surfaces != NULL;
}

static void destroy_surfaces_from_queue(Swapchain *swapchain,
      struct queue *q)
{
   struct list *item;

   queue_poison(q);
   while ((item = queue_try_dequeue(q)) != NULL)
   {
      SwapchainSurface *surface = list_entry(item,
            SwapchainSurface, link);
      FenceInterface_WaitAndDestroy(swapchain->fence_interface,
            &surface->render_fence);
      FenceInterface_WaitAndDestroy(swapchain->fence_interface,
            &surface->display_fence);
      SurfaceInterface_Destroy(swapchain->surface_interface,
            surface->native_surface);
   }
}

static void destroy_surfaces(Swapchain *swapchain)
{
   destroy_surfaces_from_queue(swapchain, &swapchain->display_queue);
   destroy_surfaces_from_queue(swapchain, &swapchain->render_queue);
   queue_destroy(&swapchain->display_queue);
   queue_destroy(&swapchain->render_queue);
   free(swapchain->surfaces);
}

bool SwapchainCreate(Swapchain *swapchain,
      const FenceInterface *fence_interface,
      const SurfaceInterface *surface_interface, size_t size)
{
   /* sanity check */
   assert(swapchain != NULL);
   assert(fence_interface != NULL);
   assert(surface_interface != NULL);
   assert(size > 1);

   swapchain->fence_interface = fence_interface;
   swapchain->surface_interface = surface_interface;

   return create_surfaces(swapchain, size);
}

void SwapchainDestroy(Swapchain *swapchain)
{
   destroy_surfaces(swapchain);
}

void SwapchainPoison(Swapchain *swapchain)
{
   assert(swapchain != NULL);

   queue_poison(&swapchain->display_queue);
}

static bool resize_surface(Swapchain *swapchain,
      SwapchainSurface *surface, const BEGL_PixmapInfo *requested,
      bool secure)
{
   if (surface->pixmap_info.width > 0)
   {
      FenceInterface_WaitAndDestroy(swapchain->fence_interface,
            &surface->render_fence);
      FenceInterface_WaitAndDestroy(swapchain->fence_interface,
            &surface->display_fence);
      SurfaceInterface_Destroy(swapchain->surface_interface,
            surface->native_surface);
   }

   if (SurfaceInterface_Create(swapchain->surface_interface,
         surface->native_surface, requested->width, requested->height,
         requested->format, secure))
   {
      surface->pixmap_info.width = requested->width;
      surface->pixmap_info.height = requested->height;
      surface->pixmap_info.format = requested->format;
      surface->secure = secure;
   }
   else
   {
      surface->pixmap_info.width = 0;
      surface->pixmap_info.height = 0;
      surface->pixmap_info.format = BEGL_BufferFormat_INVALID;
      surface->secure = false;
   }

   return surface->pixmap_info.width > 0;
}

static SwapchainSurface *deque_surface(struct queue *queue)
{
   struct list * item = queue_dequeue(queue); /* block */
   SwapchainSurface *surface = item ? list_entry(item,
         SwapchainSurface, link) : NULL;
   return surface;
}

SwapchainSurface *SwapchainDequeueRenderSurface(
      Swapchain *swapchain, const BEGL_PixmapInfo *requested,
      bool secure)
{
   assert(swapchain != NULL);
   assert(requested != NULL);
   assert(requested->width > 0);
   assert(requested->height > 0);
   assert(requested->format != BEGL_BufferFormat_INVALID);

   SwapchainSurface *surface = deque_surface(&swapchain->render_queue);

   if (surface->pixmap_info.width != requested->width
         || surface->pixmap_info.height != requested->height
         || surface->pixmap_info.format != requested->format
         || surface->secure != secure)
   {
      resize_surface(swapchain, surface, requested, secure);
   }

   return surface;
}

void SwapchainEnqueueDisplaySurface(Swapchain *swapchain,
      SwapchainSurface *surface)
{
   assert(swapchain != NULL);
   assert(surface != NULL);

   queue_enqueue(&swapchain->display_queue, &surface->link);
}

SwapchainSurface *SwapchainDequeueDisplaySurface(
      Swapchain *swapchain)
{
   assert(swapchain != NULL);
   return  deque_surface(&swapchain->display_queue);
}

void SwapchainEnqueueRenderSurface(Swapchain *swapchain,
      SwapchainSurface *surface)
{
   assert(swapchain != NULL);
   assert(surface != NULL);

   queue_enqueue(&swapchain->render_queue, &surface->link);
}
