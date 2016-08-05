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
      SetEvent(priv->vsyncEvent);
}

static void bufferOffDisplayCallback(void *context, int param)
{
   NXPL_Display *priv = (NXPL_Display *)context;
   UNUSED(param);

   int terminating = __sync_fetch_and_and(&priv->terminating, 1);
   if (!terminating)
   {
      size_t numRecycled = 1;
      NEXUS_SurfaceHandle surface_list[priv->numSurfaces];
      NEXUS_SurfaceClient_RecycleSurface(priv->surfaceClient, surface_list, priv->numSurfaces, &numRecycled);
      if (priv->bufferOffDisplay.callback)
         for (size_t i = 0; i < numRecycled; i++)
            priv->bufferOffDisplay.callback(priv->bufferOffDisplay.context, surface_list[i]);
   }
}

bool InitializeDisplayMulti(NXPL_Display *priv,
      NXPL_DisplayType displayType, unsigned int numSurfaces,
      uint32_t clientID,
      NEXUS_SurfaceClientHandle surfaceClient,
      const NXPL_NativeWindowInfoEXT *windowInfo)
{
   priv->displayType = displayType;
   priv->numSurfaces = numSurfaces;
   priv->clientID = clientID;
   priv->surfaceClient = surfaceClient;
   priv->display = NULL;

   /* setup the display & callback */
   priv->vsyncEvent = CreateEvent();

   if (priv->surfaceClient != NULL)
   {
      SetDisplayComposition(priv, windowInfo);

      NEXUS_SurfaceClientSettings clientSettings;
      NEXUS_SurfaceClient_GetSettings(priv->surfaceClient, &clientSettings);

      clientSettings.vsync.callback = vsyncCallback;
      clientSettings.vsync.context = priv;

      NEXUS_Error err = NEXUS_SurfaceClient_SetSettings(priv->surfaceClient,
            &clientSettings);
      if (err != NEXUS_SUCCESS)
         FATAL_ERROR("InitializeDisplayMulti failed");
   }
   return true;
}

void TerminateDisplay(NXPL_Display *priv)
{
   /* mark as dying.  As callbacks are asynchronous in the nexus model, they can
      arrive after they have been uninstalled here, calling with destroyed resources */
   int res = __sync_fetch_and_or(&priv->terminating, 1);
   if (res != 0)
      FATAL_ERROR("TerminateDisplay called more than once");

   if (priv->surfaceClient != NULL)
   {
      NEXUS_SurfaceClientSettings clientSettings;
      NEXUS_SurfaceClient_GetSettings(priv->surfaceClient, &clientSettings);

      clientSettings.vsync.callback = NULL;
      clientSettings.vsync.context = NULL;

      NEXUS_SurfaceClient_SetSettings(priv->surfaceClient, &clientSettings);
   }

   DestroyEvent(priv->vsyncEvent);
}

void SetDisplayComposition(NXPL_Display *priv, const NXPL_NativeWindowInfoEXT *windowInfo)
{
#if defined(NXCLIENT_SUPPORT)
   NEXUS_SurfaceComposition comp;

   NxClient_GetSurfaceClientComposition(priv->clientID, &comp);

   if (!windowInfo->stretch)
   {
      comp.virtualDisplay.width = 0;
      comp.virtualDisplay.height = 0;
      comp.position.width = windowInfo->width;
      comp.position.height = windowInfo->height;
   }
   else
   {
      comp.virtualDisplay.width = windowInfo->width;
      comp.virtualDisplay.height = windowInfo->height;
      comp.position.width = windowInfo->width - (2 * windowInfo->x);
      comp.position.height = windowInfo->height - (2 * windowInfo->y);
   }
   comp.position.x = windowInfo->x;
   comp.position.y = windowInfo->y;
   comp.zorder = windowInfo->zOrder;
   comp.colorBlend = windowInfo->colorBlend;
   comp.alphaBlend = windowInfo->alphaBlend;

   NxClient_SetSurfaceClientComposition(priv->clientID, &comp);
#endif
}

void SetDisplayFinishedCallback(NXPL_Display *priv,
      NXPL_SurfaceOffDisplay callback, void *context)
{
   NEXUS_SurfaceClientSettings clientSettings;
   NEXUS_SurfaceClient_GetSettings(priv->surfaceClient, &clientSettings);

   /* this is *NOT* thread safe */
   priv->bufferOffDisplay.callback = callback;
   priv->bufferOffDisplay.context = context;

   clientSettings.recycled.callback = bufferOffDisplayCallback;
   clientSettings.recycled.context = priv;
   NEXUS_Error err = NEXUS_SurfaceClient_SetSettings(priv->surfaceClient,
         &clientSettings);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("SetDisplayFrameBufferCallback failed");

}

bool DisplaySurface(NXPL_Display *priv, NEXUS_SurfaceHandle surface)
{
   NEXUS_SurfaceClientSettings clientSettings;
   NEXUS_SurfaceClient_GetSettings(priv->surfaceClient, &clientSettings);
   if (priv->displayType == NXPL_2D)
      clientSettings.orientation = NEXUS_VideoOrientation_e2D;
   else if (priv->displayType == NXPL_3D_LEFT_RIGHT)
      clientSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
   else if (priv->displayType == NXPL_3D_OVER_UNDER)
      clientSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
   else
      FATAL_ERROR("Invalid displayType");
   clientSettings.allowCompositionBypass = true;

   NEXUS_Error err = NEXUS_SurfaceClient_SetSettings(priv->surfaceClient, &clientSettings);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_SurfaceClient_SetSettings failed");

   ResetEvent(priv->vsyncEvent);

   err = NEXUS_SurfaceClient_PushSurface(priv->surfaceClient, surface, NULL, false);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_SurfaceClient_PushSurface failed");
   return true;
}

void WaitDisplaySync(NXPL_Display *priv)
{
   WaitEvent(priv->vsyncEvent);
}
