/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "display_framework.h"
#include "platform_common.h"

#include <string.h>
#include <assert.h>

/* Maximum display latency:
 * 0 - Won't work, it would be equivalent of a single-buffering where
 *     drawing is visible on screen immediately.
 * 1 - Start a new frame after the previous one is on screen, the result
 *     of drawing is visible at most 1 VSYNC later. After finishing a frame
 *     both CPU and GPU stall until the next VSYNC. This allows for minimal
 *     latency but at the cost of a significantly lower performance.
 * 2 - Start a new frame before the previous one is on screen, the result
 *     of drawing is visible 2 VSYNCs later. After finishing a frame
 *     CPU and triple-buffered GPU can start the next frame immediately,
 *     a double-buffered GPU always has to stall the rasterisation until
 *     the next VSYNC, when the other buffer becomes available.
 * 3 - More latency, same performance; the app has 1 VSYNC on average
 *     to complete a frame, the latency of 2 fames already compensates for
 *     software delay in VSYNC processing and gives plenty of room for
 *     occasional overshoots.
 */
#define MAX_DISPLAY_LATENCY 2

static unsigned wait_sync(DisplayFramework *df, unsigned swap_interval)
{
   while(swap_interval && DisplayInterface_WaitSync(df->display_interface))
   {
      --swap_interval;
   }
   return swap_interval;
}

static void *display_thread_func(void *p)
{
   DisplayFramework *df = (DisplayFramework *) p;

   /* main thread can proceed */
   pthread_barrier_wait(&df->barrier);

   SwapchainSurface *surface;
   while ((surface = SwapchainDequeueDisplaySurface(&df->swapchain)) != NULL)
   {
      /* ownership of the render fence is passed to display */
      int render_fence = surface->render_fence;
      surface->render_fence = df->fence_interface->invalid_fence;

      assert(surface->display_fence == df->fence_interface->invalid_fence);

      DisplayInterface_Display(df->display_interface, surface->native_surface,
            render_fence, (surface->swap_interval > 0), &surface->display_fence);

      wait_sync(df, surface->swap_interval);

      SwapchainEnqueueRenderSurface(&df->swapchain, surface);
      sem_post(&df->latency);
   }
   return NULL;
}

bool DisplayFramework_Start(DisplayFramework *df,
      const DisplayInterface *display_interface,
      const FenceInterface *fence_interface,
      const SurfaceInterface *surface_interface,
      uint32_t width, uint32_t height, uint32_t swapchain_count)
{
   /* sanity check */
   assert(df != NULL);
   assert(display_interface != NULL);
   assert(fence_interface != NULL);
   assert(surface_interface != NULL);
   assert(width > 0 && height > 0);
   assert(swapchain_count > 1);

   memset(df, 0, sizeof(*df));

   df->display_interface = display_interface;
   df->fence_interface = fence_interface;
   df->surface_interface = surface_interface;

   df->window_info.width = width;
   df->window_info.height = height;
   df->window_info.swapchain_count = swapchain_count;

   if (!SwapchainCreate(&df->swapchain, fence_interface, surface_interface, swapchain_count))
      goto no_swapchain;
   if (pthread_mutex_init(&df->window_mutex, NULL) != 0)
      goto no_mutex;
   if (sem_init(&df->latency, 0, MAX_DISPLAY_LATENCY) != 0)
      goto no_sem;
   if (pthread_barrier_init(&df->barrier, NULL, 2) != 0)
      goto no_barrier;
   if (pthread_create(&df->thread, NULL, display_thread_func, df) != 0)
      goto no_thread;

   /* Wait for thread to really start */
   pthread_barrier_wait(&df->barrier);
   pthread_barrier_destroy(&df->barrier);
   return true;

   no_thread: pthread_barrier_destroy(&df->barrier);
   no_barrier: sem_destroy(&df->latency);
   no_sem: pthread_mutex_destroy(&df->window_mutex);
   no_mutex: SwapchainDestroy(&df->swapchain);
   no_swapchain: return false;
}

void DisplayFramework_Stop(DisplayFramework *df)
{
   if (df)
   {
      SwapchainPoison(&df->swapchain);
      pthread_join(df->thread, NULL);

      /* The display thread has finished. Display queue is empty, render queue
       * contains cancelled surface(s) with render fence and/or surface(s)
       * sent to display with display fences. Ask display to stop - this should
       * lead to all display fences being eventually signalled.
       */
      DisplayInterface_Stop(df->display_interface);

      /* Destroy swapchain - swapchain waits on both render and display fence,
       * if present, prior to destroying each surface so we don't have to.
       */
      SwapchainDestroy(&df->swapchain);

      sem_destroy(&df->latency);
      pthread_mutex_destroy(&df->window_mutex);
   }
}

void DisplayFramework_GetSize(DisplayFramework *df,
      uint32_t *width, uint32_t *height)
{
   pthread_mutex_lock(&df->window_mutex);
   *width = df->window_info.width;
   *height = df->window_info.height;
   pthread_mutex_unlock(&df->window_mutex);
}

void DisplayFramework_SetSize(DisplayFramework *df,
      uint32_t width, uint32_t height)
{
   pthread_mutex_lock(&df->window_mutex);
   df->window_info.width = width;
   df->window_info.height = height;
   pthread_mutex_unlock(&df->window_mutex);
}

void *DisplayFramework_GetNextSurface(DisplayFramework *df,
      BEGL_BufferFormat format, bool secure, int *age, int *fence)
{
   assert(df != NULL);
   assert(format != BEGL_BufferFormat_INVALID);

   pthread_mutex_lock(&df->window_mutex);
   BEGL_PixmapInfo requested = {
         .width = df->window_info.width,
         .height = df->window_info.height,
         .format = format,
   };
   pthread_mutex_unlock(&df->window_mutex);

   sem_wait(&df->latency);

   SwapchainSurface *surface = SwapchainDequeueRenderSurface(
         &df->swapchain, &requested, secure);
   if (surface)
   {
      /* if this surface was cancelled the render fence may still be set */
      FenceInterface_WaitAndDestroy(df->fence_interface,
            &surface->render_fence);

      if (fence)
      {
         /* hand over the the display fence to the caller */
         *fence = surface->display_fence;
         surface->display_fence = df->fence_interface->invalid_fence; /* lost ownership */
      }
      else
      {
         /*the caller didn't want to handle the fence so wait here */
         FenceInterface_WaitAndDestroy(df->fence_interface,
               &surface->display_fence);
      }

      assert(surface->render_fence == df->fence_interface->invalid_fence);
      assert(surface->display_fence == df->fence_interface->invalid_fence);

      *age = surface->age;

      return surface->native_surface;
   }
   *age = 0;
   return NULL;
}

static void AgeSwapchain(Swapchain *swapchain, SwapchainSurface *cur_backbuffer)
{
   /* Old backbuffer gets age 1 */
   cur_backbuffer->age = 1;

   /* All other buffer ages are incremented if >0 */
   size_t surface_size = sizeof(SwapchainSurface) + swapchain->surface_interface->sizeof_surface;
   SwapchainSurface *p = swapchain->surfaces;
   for (size_t i = 0; i < swapchain->num_surfaces; i++)
   {
      if (p != cur_backbuffer && p->age > 0)
         p->age++;
      p = (SwapchainSurface *)((uintptr_t)p + surface_size);
   }
}

void DisplayFramework_DisplaySurface(DisplayFramework *df,
      void *surface, int fence, uint32_t swap_interval)
{
   assert(df != NULL);
   assert(surface != NULL);

   SwapchainSurface *s = container_of(surface, SwapchainSurface, native_surface);

   AgeSwapchain(&df->swapchain, s);

   assert(s->render_fence == df->fence_interface->invalid_fence);
   s->render_fence = fence;
   s->swap_interval = swap_interval;

   SwapchainEnqueueDisplaySurface(&df->swapchain, s);
}

void DisplayFramework_CancelSurface(DisplayFramework *df,
      void *surface, int fence)
{
   assert(df != NULL);
   assert(surface != NULL);

   SwapchainSurface *s = container_of(surface, SwapchainSurface, native_surface);

   assert(s->render_fence == df->fence_interface->invalid_fence);
   s->render_fence = fence;
   s->age = 0;

   SwapchainEnqueueRenderSurface(&df->swapchain, s);
}
