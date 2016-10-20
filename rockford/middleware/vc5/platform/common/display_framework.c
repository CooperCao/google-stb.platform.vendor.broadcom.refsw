/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "display_framework.h"
#include "platform_common.h"

#include <string.h>
#include <assert.h>

static unsigned wait_sync(DisplayFramework *df, unsigned swap_interval)
{
   while(swap_interval && DisplayInterface_WaitSync(df->display_interface))
   {
      --swap_interval;
   }
   return swap_interval;
}

static void display_surface(DisplayFramework *df,
      SwapchainSurface *surface)
{
   void *native_surface = surface->native_surface;

   switch (DisplayInterface_Display(df->display_interface, native_surface,
         surface->render_fence, &surface->display_fence))
   {
   case eDisplayFailed:
      /* display fence shouldn't be set */
      assert(surface->display_fence == df->fence_interface->invalid_fence);
      break;
   case eDisplaySuccessful:
   case eDisplayPending:
      /* display fence may be set */
      break;
   }

   /* render fence is now owned by the display */
   surface->render_fence = df->fence_interface->invalid_fence;
}

static void *display_thread_func(void *p)
{
   DisplayFramework *df = (DisplayFramework *) p;

   /* main thread can proceed */
   pthread_barrier_wait(&df->barrier);

   SwapchainSurface *surface;
   while ((surface = SwapchainDequeueDisplaySurface(&df->swapchain)) != NULL)
   {
      if (FenceInterface_Wait(df->fence_interface,
            surface->display_fence, 0))
      {
         FenceInterface_Destroy(df->fence_interface,
            &surface->display_fence);
         display_surface(df, surface);
         SwapchainEnqueueRenderSurface(&df->swapchain, surface);
         wait_sync(df, surface->swap_interval);
      }
      else /* don't display this one - it's still on display */
      {
         SwapchainEnqueueRenderSurface(&df->swapchain, surface);
      }

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

   if (SwapchainCreate(&df->swapchain, fence_interface, surface_interface, swapchain_count))
   {
      if (pthread_barrier_init(&df->barrier, NULL, 2) == 0)
      {
         if (pthread_create(&df->thread, NULL, display_thread_func, df) == 0)
         {
            /* Wait for thread to really start */
            pthread_barrier_wait(&df->barrier);

            pthread_barrier_destroy(&df->barrier);
            return true;
         }
         pthread_barrier_destroy(&df->barrier);
      }
      SwapchainDestroy(&df->swapchain);
   }
   return false;
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
   }
}

void *DisplayFramework_GetNextSurface(DisplayFramework *df,
      BEGL_BufferFormat format, bool secure, int *fence)
{
   assert(df != NULL);
   assert(format != BEGL_BufferFormat_INVALID);

   BEGL_PixmapInfo requested = {
         .width = df->window_info.width,
         .height = df->window_info.height,
         .format = format,
   };

   SwapchainSurface *surface = SwapchainDequeueRenderSurface(
         &df->swapchain, &requested, secure);
   if (surface)
   {
      /* if this surface was cancelled the render fence may still be set */
      FenceInterface_WaitAndDestroy(df->fence_interface,
            &surface->render_fence);

      /* hand over display fence to the caller */
      if (fence)
      {
         if (surface->swap_interval > 0)
         {
            /* retain our own reference to the display fence */
            FenceInterface_Keep(df->fence_interface, surface->display_fence);

            /* hand over the other reference to the renderer */
            *fence = surface->display_fence;
         }
         else
         {
            /* let renderer ignore the display fence */
            *fence = df->fence_interface->invalid_fence;
         }
      }

      return surface->native_surface;
   }
   return NULL;
}

void DisplayFramework_DisplaySurface(DisplayFramework *df,
      void *surface, int fence, uint32_t swap_interval)
{
   assert(df != NULL);
   assert(surface != NULL);

   SwapchainSurface *s = container_of(surface,
         SwapchainSurface, native_surface);

   s->render_fence = fence;
   s->swap_interval = swap_interval;

   SwapchainEnqueueDisplaySurface(&df->swapchain, s);
}

void DisplayFramework_CancelSurface(DisplayFramework *df,
      void *surface, int fence)
{
   assert(df != NULL);
   assert(surface != NULL);

   SwapchainSurface *s = container_of(surface,
         SwapchainSurface, native_surface);

   s->render_fence = fence;

   SwapchainEnqueueRenderSurface(&df->swapchain, s);
}
