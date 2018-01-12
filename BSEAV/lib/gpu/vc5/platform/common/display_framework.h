/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __DISPLAY_FRAMEWORK_H__
#define __DISPLAY_FRAMEWORK_H__

#include "display_interface.h"
#include "fence_interface.h"
#include "surface_interface.h"
#include "swapchain.h"

#include <EGL/begl_displayplatform.h>

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

/*
 * DisplayFramework is a generic implementation of an animation loop
 * required by the BEGL_DisplayInterface. This allows common parts
 * of the abstractions to be reused by multiple platforms, much reducing
 * code duplication and associated maintenance burden
 *
 * The framework includes all necessary ingredients to drive display
 * and run buffer management but it intentionally lacks any knowledge
 * about the actual platform. In order to start display framework the caller
 * must implement 3 interfaces:
 *
 * - Fence interface that creates, destroys, waits and signals fences;
 *   a fence is expected to be accessed by an int descriptor. An implementation
 *   that uses Nexus fences is available in fence_interface_nexus.c
 *
 * - Surface interface that creates and destroys buffers. An implementation
 *   that creates and destroys NXPL_Surface surfaces is available
 *   in surface_interface_nexus.c
 *
 * - Display interface that can display a surface created by the surface
 *   interface and allows to synchronise with the display. There are two
 *   Nexus implementations available: one for exclusive access and one
 *   for multi-process mode using nxclient.
 *
 * The caller is expected to create implementations of required interfaces
 * (or reuse existing ones) and pass them to the DisplayFramework_Start()
 * function. This starts the display loop.
 *
 * In order to stop the display call DisplayFramework_Stop(). This will
 * request the display to stop, wait until all buffers are freed up,
 * destroy them and shut down the display framework.
 *
 * Please note that display framework does *NOT* take ownership
 * of the interfaces. This allows for more flexible implementation of required
 * interfaces that either can be reusable or must be destroyed and re-created
 * each time by the caller.
 */
typedef struct DisplayFramework
{
   const DisplayInterface   *display_interface;
   const FenceInterface     *fence_interface;
   const SurfaceInterface   *surface_interface;
   BEGL_WindowInfo           window_info;
   pthread_mutex_t           window_mutex;

   pthread_t                 thread;
   pthread_barrier_t         barrier;
   sem_t                     latency;
   Swapchain                 swapchain;
} DisplayFramework;

bool DisplayFramework_Start(DisplayFramework *df,
      const DisplayInterface *display_interface,
      const FenceInterface *fence_interface,
      const SurfaceInterface *surface_interface,
      uint32_t width, uint32_t height, uint32_t swapchain_count);

void DisplayFramework_Stop(DisplayFramework *df);

void DisplayFramework_GetSize(DisplayFramework *df,
      uint32_t *width, uint32_t *height);

void DisplayFramework_SetSize(DisplayFramework *df,
      uint32_t width, uint32_t height);

void *DisplayFramework_GetNextSurface(DisplayFramework *df,
      BEGL_BufferFormat format, bool secure, int *fence, unsigned *age);

void DisplayFramework_DisplaySurface(DisplayFramework *df,
      void *surface, int fence, uint32_t swap_interval);

void DisplayFramework_CancelSurface(DisplayFramework *df,
      void *surface, int fence);

#endif /* __DISPLAY_FRAMEWORK_H__ */
