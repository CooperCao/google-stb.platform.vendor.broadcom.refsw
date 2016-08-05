/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "display_interface_nexus.h"

#include "fence_queue.h"
#include "../nexus/private_nexus.h" /* NXPL_Surface */
#include "platform_common.h"

struct DisplayInterfaceNexus
{
   NXPL_Display *display;
   const struct FenceInterface *fence_interface;

   pthread_mutex_t mutex;
   struct fence_queue fence_queue;
};

static DisplayInterfaceResult display(void *context, void *s,
      int render_fence, int *display_fence)
{
   struct DisplayInterfaceNexus *din = (struct DisplayInterfaceNexus *)context;
   NXPL_Surface *surface = (NXPL_Surface *)s;

   FenceInterface_WaitAndDestroy(
         din->fence_interface, &render_fence);

   pthread_mutex_lock(&din->mutex);
   if (DisplaySurface(din->display, surface->surface))
   {
      FenceInterface_Create(
            din->fence_interface, display_fence);
      fence_queue_enqueue(&din->fence_queue, *display_fence);

      pthread_mutex_unlock(&din->mutex);
      return eDisplayPending;
   }
   else
   {
      pthread_mutex_unlock(&din->mutex);
      return eDisplayFailed;
   }
}

static void nexus_callback(void *context, NEXUS_SurfaceHandle surface)
{
   struct DisplayInterfaceNexus *din = (struct DisplayInterfaceNexus *)context;
   UNUSED(surface);
   int display_fence;

   pthread_mutex_lock(&din->mutex);
   if (fence_queue_dequeue(&din->fence_queue, &display_fence, false))
   {
      FenceInterface_Signal(din->fence_interface, display_fence);
   }
   pthread_mutex_unlock(&din->mutex);
}

static bool wait_sync(void *context)
{
   struct DisplayInterfaceNexus *din = (struct DisplayInterfaceNexus *)context;
   WaitDisplaySync(din->display);
   return true;
}

static void stop(void *context)
{
   struct DisplayInterfaceNexus *din = (struct DisplayInterfaceNexus *)context;
   TerminateDisplay(din->display);
   int display_fence;
   while (fence_queue_dequeue(&din->fence_queue, &display_fence, false))
      FenceInterface_Signal(din->fence_interface, display_fence);
}


void destroy(void *context)
{
   struct DisplayInterfaceNexus *din = (struct DisplayInterfaceNexus *)context;
   if (din)
   {
      pthread_mutex_destroy(&din->mutex);
      fence_queue_destroy(&din->fence_queue);
      free(din);
   }
}

bool DisplayInterface_InitNexus(struct DisplayInterface *di,
      NXPL_Display *nxpl_display, const struct FenceInterface *fi,
      unsigned int numSurfaces)
{
   struct DisplayInterfaceNexus *din = calloc(1, sizeof(*din));
   if (din)
   {
      din->display = nxpl_display;
      din->fence_interface = fi;
      fence_queue_init(&din->fence_queue, numSurfaces, din->fence_interface);
      pthread_mutex_init(&din->mutex, NULL);

      di->base.context = din;
      di->base.destroy = destroy;

      di->display = display;
      di->wait_sync = wait_sync;
      di->stop = stop;

      SetDisplayFinishedCallback(din->display, nexus_callback, din);
   }
   return din != NULL;
}
