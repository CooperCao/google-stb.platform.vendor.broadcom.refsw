/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "display_nexus_priv.h"
#include "event.h"
#include "fatal_error.h"
#include "platform_common.h"

static void vsyncCallback(void *context, int param)
{
   NXPL_Display *priv = (NXPL_Display *)context;

   int terminating = __sync_fetch_and_and(&priv->terminating, 1);
   if (!terminating)
   {
      BKNI_AcquireMutex(priv->surface.mutex);
      if (!priv->surface.next)
      {
         /* This is the 2nd and subsequent vsync after DisplaySurface().
          * We have to skip the 1st one because frameBufferCallback may arrive
          * after vsyncCallback and we use the event to first wait until the
          * buffers are swapped and then to wait for subsequent vsyncs.
          *
          * The 1st vsync is signalled from frameBufferCallback().
          */
         SetEvent(priv->vsyncEvent);
      }
      BKNI_ReleaseMutex(priv->surface.mutex);
   }
}

static void frameBufferCallback(void *context, int param)
{
   NXPL_Display *priv = (NXPL_Display *)context;
   UNUSED(param);

   int terminating = __sync_fetch_and_and(&priv->terminating, 1);
   if (!terminating)
   {
      /* Buffers were swapped. The surface that was previously on screen is
       * off screen now. The surface that was waiting after the last call
       * to DisplaySurface is now on screen. The next call DisplaySurface
       * will initiate another buffer swap.
       */
      NEXUS_SurfaceHandle off;
      BKNI_AcquireMutex(priv->surface.mutex);
      off = priv->surface.current;
      priv->surface.current = priv->surface.next;
      priv->surface.next = NULL;

      /* that's the 1st "vsync" after DisplaySurface() */
      SetEvent(priv->vsyncEvent);

      BKNI_ReleaseMutex(priv->surface.mutex);

      if (priv->bufferOffDisplay.callback && off)
      {
         priv->bufferOffDisplay.callback(priv->bufferOffDisplay.context, off);
      }
   }
}

bool InitializeDisplayExclusive(NXPL_Display *priv,
      NXPL_DisplayType displayType, NEXUS_DISPLAYHANDLE display, int *bound,
      const NXPL_NativeWindowInfoEXT *windowInfo)
{
   priv->displayType = displayType;
   priv->bound = bound;
   priv->display = display;

   /* setup the display & callback */
   priv->vsyncEvent = CreateEvent();

   if (BKNI_CreateMutex(&priv->surface.mutex) != BERR_SUCCESS)
      FATAL_ERROR("BKNI_CreateMutex failed");
   priv->surface.current = NULL;
   priv->surface.next = NULL;

   if (priv->display != NULL)
   {
      __sync_fetch_and_add(priv->bound, 1);

      SetDisplayComposition(priv, windowInfo);
   }

   NEXUS_CallbackDesc vsync;
   /* vsync available, swap interval can be active */
   vsync.callback = vsyncCallback;
   vsync.context = priv;
   NEXUS_Error err = NEXUS_Display_SetVsyncCallback(priv->display, &vsync);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_Display_SetVsyncCallback failed");

   return true;
}

void TerminateDisplay(NXPL_Display *priv)
{
   /* mark as dying.  As callbacks are asynchronous in the nexus model, they can
      arrive after they have been uninstalled here, calling with destroyed resources */
   int res = __sync_fetch_and_or(&priv->terminating, 1);
   if (res != 0)
      FATAL_ERROR("TerminateDisplay called more than once");

   if (priv->display != NULL)
   {
      int res = __sync_sub_and_fetch(priv->bound, 1);
      /* don't remove the callbacks until we've counted everything done */
      if (res == 0)
      {
         NEXUS_GraphicsSettings  graphicsSettings;
         NEXUS_Display_GetGraphicsSettings(priv->display, &graphicsSettings);

         graphicsSettings.enabled = false;
         graphicsSettings.frameBufferCallback.callback = NULL;

         NEXUS_Error err = NEXUS_Display_SetGraphicsSettings(priv->display, &graphicsSettings);
         if (err != NEXUS_SUCCESS)
            FATAL_ERROR("NEXUS_Display_SetGraphicsSettings failed");
      }
   }

   NEXUS_Error err = NEXUS_Display_SetVsyncCallback(priv->display, NULL);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_Display_SetVsyncCallback failed");

   BKNI_DestroyMutex(priv->surface.mutex);
   priv->surface.current = NULL;
   priv->surface.next = NULL;

   DestroyEvent(priv->vsyncEvent);
}

void SetDisplayComposition(NXPL_Display *priv, const NXPL_NativeWindowInfoEXT *windowInfo)
{
   NEXUS_GraphicsSettings  graphicsSettings;

   NEXUS_Display_GetGraphicsSettings(priv->display, &graphicsSettings);

   graphicsSettings.enabled = true;
   graphicsSettings.position.x = windowInfo->x;
   graphicsSettings.position.y = windowInfo->y;
   if (!windowInfo->stretch)
   {
      graphicsSettings.position.width = windowInfo->width;
      graphicsSettings.position.height = windowInfo->height;
   }
   graphicsSettings.clip.width = windowInfo->width;
   graphicsSettings.clip.height = windowInfo->height;

   NEXUS_Error err = NEXUS_Display_SetGraphicsSettings(priv->display, &graphicsSettings);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_Display_SetGraphicsSettings failed");
}

void SetDisplayFinishedCallback(NXPL_Display *priv,
      NXPL_SurfaceOffDisplay callback, void *context)
{
   NEXUS_GraphicsSettings  graphicsSettings;
   NEXUS_Display_GetGraphicsSettings(priv->display, &graphicsSettings);

   priv->bufferOffDisplay.callback = callback;
   priv->bufferOffDisplay.context = context;

   graphicsSettings.frameBufferCallback.callback = frameBufferCallback;
   graphicsSettings.frameBufferCallback.context = priv;
   NEXUS_Error err = NEXUS_Display_SetGraphicsSettings(priv->display, &graphicsSettings);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("SetDisplayFrameBufferCallback failed");
}

bool DisplaySurface(NXPL_Display *priv, NEXUS_SurfaceHandle surface)
{
   BKNI_AcquireMutex(priv->surface.mutex);
   if (priv->surface.next)
   {
      BKNI_ReleaseMutex(priv->surface.mutex);
      return false;
   }
   priv->surface.next = surface;
   BKNI_ReleaseMutex(priv->surface.mutex);

   NEXUS_GraphicsFramebuffer3D fb3d;
   NEXUS_Graphics_GetDefaultFramebuffer3D(&fb3d);
   fb3d.main = surface;
   fb3d.right = surface;

   if (priv->displayType == NXPL_2D)
      fb3d.orientation = NEXUS_VideoOrientation_e2D;
   else if (priv->displayType == NXPL_3D_LEFT_RIGHT)
      fb3d.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
   else if (priv->displayType == NXPL_3D_OVER_UNDER)
      fb3d.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
   else
      FATAL_ERROR("Invalid displayType");

   ResetEvent(priv->vsyncEvent);

   NEXUS_Error err = NEXUS_Display_SetGraphicsFramebuffer3D(priv->display, &fb3d);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_Display_SetGraphicsFramebuffer3D failed");

   return true;
}

void WaitDisplaySync(NXPL_Display *priv)
{
   WaitEvent(priv->vsyncEvent);
}
