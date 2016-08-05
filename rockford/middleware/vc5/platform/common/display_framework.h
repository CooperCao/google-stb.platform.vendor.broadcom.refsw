/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __DISPLAY_FRAMEWORK_H__
#define __DISPLAY_FRAMEWORK_H__

#include "display_interface.h"
#include "fence_interface.h"
#include "surface_interface.h"
#include "swapchain.h"

#include <EGL/begl_displayplatform.h>

#include <stdbool.h>

struct DisplayFramework
{
   const struct DisplayInterface   *display_interface;
   const struct FenceInterface     *fence_interface;
   const struct SurfaceInterface   *surface_interface;
   BEGL_WindowInfo                  window_info;

   pthread_t                        thread;
   pthread_barrier_t                barrier;
   struct Swapchain                 swapchain;
};

bool DisplayFramework_Start(struct DisplayFramework *df,
      const struct DisplayInterface *display_interface,
      const struct FenceInterface *fence_interface,
      const struct SurfaceInterface *surface_interface,
      uint32_t width, uint32_t height, uint32_t swapchain_count);

void DisplayFramework_Stop(struct DisplayFramework *df);

void *DisplayFramework_GetNextSurface(struct DisplayFramework *df,
      BEGL_BufferFormat format, bool secure, int *fence);

void DisplayFramework_DisplaySurface(struct DisplayFramework *df,
      void *surface, int fence, uint32_t swap_interval);

void DisplayFramework_CancelSurface(struct DisplayFramework *df,
      void *surface, int fence);

#endif /* __DISPLAY_FRAMEWORK_H__ */
