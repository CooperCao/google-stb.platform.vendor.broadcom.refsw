/******************************************************************************
* (c) 2013-4 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#ifdef SINGLE_PROCESS

/*=============================================================================
 The "quad exclusive display" is a specialised exclusive mode platform layer
 It supports a SINGLE native rendering window. Each frame submitted is considered
 to be one N x M of the display, so the final on-screen display is made from N x M
 frames arranged as:

 ...
 N+1 N+2 ...
 0 1 .. N

 There are two NMx buffers for the display and three 1x buffers for the application
 When a frame is submitted, it is copied into the back-buffer for the display.
 When the backbuffer is completed, it is swapped with the front buffer on the
 next vsync (or immediately in swap 0).
=============================================================================*/


#include "default_nexus.h"
#include "display_nexus.h"

#include "nexus_platform.h"

#include <nexus_graphics2d.h>

#include <EGL/egl.h>

#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <semaphore.h>

#define MAX_SWAP_BUFFERS      3
#define NUM_DISPLAY_BUFFERS   2

typedef void (*BufferGetRequirementsFunc)(BEGL_PixmapInfo *bufferRequirements, BEGL_BufferSettings *bufferConstrainedRequirements);

typedef struct
{
   BufferGetRequirementsFunc  bufferGetRequirementsFunc;
   NEXUS_DISPLAYHANDLE        display;
   NEXUS_HEAPHANDLE           heap;
   uint32_t                   winWidth;
   uint32_t                   winHeight;
   uint32_t                   winX;
   uint32_t                   winY;
   bool                       stretch;
   NXPL_DisplayType           displayType;
   BEGL_MemoryInterface      *memInterface;
   BKNI_EventHandle           vsyncEvent;
   uint32_t                   numPanelsX;
   uint32_t                   numPanelsY;
   uint32_t                   numPanels;
} NXPL_DisplayData;

/* There will be one WINPL_WindowState for each WINPL_SurfCompNativeWindow */
typedef struct
{
   NXPL_BufferData        *buffers[MAX_SWAP_BUFFERS];
   sem_t                  lockSemaphore[MAX_SWAP_BUFFERS];
   bool                   hasEverBeenDisplayed[MAX_SWAP_BUFFERS];
   uint32_t               actualSwapChainCount;

   NEXUS_Graphics2DHandle gfx2d;
   BKNI_EventHandle       blitEvent;
   NEXUS_SURFACEHANDLE    displaySurface[NUM_DISPLAY_BUFFERS];
   uint32_t               backBuffer;                          // 0 or 1
   uint32_t               bufferPosition;                      // 0, 1, 2, 3, .. NxM-1

} NXPL_WindowState;

/*****************************************************************************/

static void SemaphoreCreate(sem_t *sem, uint32_t initialVal)
{
   sem_init(sem, 0, initialVal);
}

static void SemaphoreDelete(sem_t *sem)
{
   sem_destroy(sem);
}

static void SemaphoreSignal(sem_t *sem)
{
   sem_post(sem);
}

static void SemaphoreWait(sem_t *sem)
{
   sem_wait(sem);
}

/*****************************************************************************
 * Display driver interface
 *****************************************************************************/
static void vsync_callback(void *context, int param)
{
   /* Simply trigger the event */
   BSTD_UNUSED(param);
   BKNI_SetEvent((BKNI_EventHandle)context);
}

static uint32_t GetBitsPerPixel(BEGL_BufferFormat format)
{
   uint32_t bpp = 0;

   switch (format)
   {
   case BEGL_BufferFormat_eA8B8G8R8         :
   case BEGL_BufferFormat_eR8G8B8A8         :
   case BEGL_BufferFormat_eX8B8G8R8         :
   case BEGL_BufferFormat_eR8G8B8X8         :
   case BEGL_BufferFormat_eA8B8G8R8_Texture :
   case BEGL_BufferFormat_eA8R8G8B8_Texture :
   case BEGL_BufferFormat_eR8G8B8A8_Texture :
   case BEGL_BufferFormat_eB8G8R8A8_Texture :
      bpp = 32;
      break;

   case BEGL_BufferFormat_eR5G6B5         :
   case BEGL_BufferFormat_eR5G6B5_Texture :
   case BEGL_BufferFormat_eYUV422_Texture :
   case BEGL_BufferFormat_eVUY224_Texture :
      bpp = 16;
      break;
   }

   return bpp;
}

static NEXUS_PixelFormat GetNexusFormat(BEGL_BufferFormat format)
{
   NEXUS_PixelFormat fmt;

   switch (format)
   {
   case BEGL_BufferFormat_eA8B8G8R8 :
      fmt = NEXUS_PixelFormat_eA8_B8_G8_R8;
      break;
   case BEGL_BufferFormat_eR8G8B8A8 :
      fmt = NEXUS_PixelFormat_eR8_G8_B8_A8;
      break;
   case BEGL_BufferFormat_eX8B8G8R8 :
      fmt = NEXUS_PixelFormat_eX8_B8_G8_R8;
      break;
   case BEGL_BufferFormat_eR8G8B8X8 :
      fmt = NEXUS_PixelFormat_eR8_G8_B8_X8;
      break;
   case BEGL_BufferFormat_eR5G6B5 :
   case BEGL_BufferFormat_eR5G6B5_Texture :
      fmt = NEXUS_PixelFormat_eR5_G6_B5;
      break;
   case BEGL_BufferFormat_eYUV422_Texture :
      fmt = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
      break;
   case BEGL_BufferFormat_eVUY224_Texture :
      fmt = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8;
      break;
   case BEGL_BufferFormat_eA8B8G8R8_Texture :
      fmt = NEXUS_PixelFormat_eA8_B8_G8_R8;
      break;
   case BEGL_BufferFormat_eA8R8G8B8_Texture :
      fmt = NEXUS_PixelFormat_eA8_R8_G8_B8;
      break;
   case BEGL_BufferFormat_eR8G8B8A8_Texture :
      fmt = NEXUS_PixelFormat_eR8_G8_B8_A8;
      break;
   case BEGL_BufferFormat_eB8G8R8A8_Texture :
      fmt = NEXUS_PixelFormat_eB8_G8_R8_A8;
      break;
   }

   return fmt;
}


static BEGL_Error Create4xDisplayBuffers(NEXUS_HEAPHANDLE heap, NXPL_WindowState *windowState, BEGL_BufferSettings *settings,
                                         uint32_t numPanelsX, uint32_t numPanelsY)
{
   uint32_t i;
   uint32_t bpp          = GetBitsPerPixel(settings->format);
   uint32_t scaledWidth  = settings->width  * numPanelsX;
   uint32_t scaledHeight = settings->height * numPanelsY;
   uint32_t pitch        = scaledWidth  * bpp / 8;
   uint32_t size         = scaledHeight * pitch;

   NEXUS_SurfaceCreateSettings    surfSettings;

   NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);

   surfSettings.pixelFormat = GetNexusFormat(settings->format);
   surfSettings.width       = scaledWidth;
   surfSettings.height      = scaledHeight;
   surfSettings.alignment   = settings->alignment;
   surfSettings.pitch       = pitch;
   surfSettings.heap        = heap;

   for (i = 0; i < NUM_DISPLAY_BUFFERS; ++i)
   {
      NEXUS_MemoryAllocationSettings memSettings;

      NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
      memSettings.alignment  = settings->alignment;
      memSettings.heap       = heap;

      if (windowState->displaySurface[i] != nullptr)
         NEXUS_Surface_Destroy(windowState->displaySurface[i]);

      NEXUS_Memory_Allocate(size, &memSettings, &surfSettings.pMemory);

      if (surfSettings.pMemory != nullptr)
      {
         windowState->displaySurface[i] = NEXUS_Surface_Create(&surfSettings);
      }
   }
}

static void BlitInto4xBuffer(NXPL_WindowState *windowState, NXPL_DisplayData *data, NEXUS_SURFACEHANDLE from, NEXUS_SURFACEHANDLE to,
                             uint32_t position)
{
   NEXUS_Error rc;
   NEXUS_Rect  rect;

   uint32_t numPanelsX = data->numPanelsX;
   uint32_t numPanelsY = data->numPanelsY;

   uint32_t offX = position % numPanelsX;
   uint32_t offY = numPanelsY - position / numPanelsX - 1;

   rect.x      = data->winWidth  * offX;
   rect.y      = data->winHeight * offY;
   rect.width  = data->winWidth;
   rect.height = data->winHeight;

   NEXUS_Graphics2D_FastBlit(windowState->gfx2d, to, &rect, from, nullptr, NEXUS_BlitColorOp_eCopySource, NEXUS_BlitAlphaOp_eCopySource, 0);

   do
   {
      rc = NEXUS_Graphics2D_Checkpoint(windowState->gfx2d, nullptr);
      if (rc == NEXUS_GRAPHICS2D_QUEUED)
         rc = BKNI_WaitForEvent(windowState->blitEvent, 1000);
   }
   while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);
}

static BEGL_Error DispBufferDisplay(void *context, BEGL_BufferDisplayState *state)
{
   NXPL_DisplayData        *data        = (NXPL_DisplayData *)     context;
   NXPL_BufferData         *buffer      = (NXPL_BufferData *)      state->buffer;
   NXPL_NativeWindowInfo   *nw          = (NXPL_NativeWindowInfo *)state->windowState.window;
   NXPL_WindowState        *windowState = (NXPL_WindowState *)     state->windowState.platformState;
   bool                    settingsChanged;
   uint32_t                numPanelsX = data->numPanelsX;
   uint32_t                numPanelsY = data->numPanelsY;

   uint32_t oldSwapChainIndex;

   BKNI_ResetEvent(data->vsyncEvent);

   /* Do we have a display buffer yet? */
   settingsChanged = nw->width   != data->winWidth || nw->height != data->winHeight ||
                     nw->x       != data->winX     || nw->y      != data->winY      ||
                     nw->stretch != data->stretch;

   if (settingsChanged || windowState->displaySurface[0] == nullptr)
   {
      Create4xDisplayBuffers(data->heap, windowState, &buffer->settings, numPanelsX, numPanelsY);
   }

   if (settingsChanged)
   {
      /* Window settings have changed */
      NEXUS_GraphicsSettings  graphicsSettings;
      NEXUS_Error             err;

      /* setup the display & callback */
      NEXUS_Display_GetGraphicsSettings(data->display, &graphicsSettings);
      graphicsSettings.enabled    = true;
      graphicsSettings.position.x = nw->x * numPanelsX;
      graphicsSettings.position.y = nw->y * numPanelsY;

      if (!nw->stretch)
      {
         graphicsSettings.position.width  = nw->width  * numPanelsX;
         graphicsSettings.position.height = nw->height * numPanelsY;
      }

      graphicsSettings.clip.width                   = nw->width  * numPanelsX;
      graphicsSettings.clip.height                  = nw->height * numPanelsY;

      graphicsSettings.frameBufferCallback.param    = 0;
      graphicsSettings.frameBufferCallback.context  = data->vsyncEvent;
      graphicsSettings.frameBufferCallback.callback = vsync_callback;

      err = NEXUS_Display_SetGraphicsSettings(data->display, &graphicsSettings);
      if (err)
         printf("NEXUS_Display_SetGraphicsSettings() failed\n");

      /* update width and height, etc. */
      data->winWidth  = nw->width;
      data->winHeight = nw->height;
      data->winX      = nw->x;
      data->winY      = nw->y;
      data->stretch   = nw->stretch;
   }

   // Blit into the buffer from surface into backBuffer
   BlitInto4xBuffer(windowState, data, buffer->surface, windowState->displaySurface[windowState->backBuffer], windowState->bufferPosition);

   // If this is the last buffer then swap.
   if (windowState->bufferPosition == (data->numPanels - 1))
   {
      NEXUS_GraphicsFramebuffer3D fb3d;

      windowState->bufferPosition = 0;

      NEXUS_Graphics_GetDefaultFramebuffer3D(&fb3d);

      if (data->displayType == NXPL_2D)
         fb3d.orientation = NEXUS_VideoOrientation_e2D;
      else if (data->displayType == NXPL_3D_LEFT_RIGHT)
         fb3d.orientation == NEXUS_VideoOrientation_e3D_LeftRight;
      else if (data->displayType == NXPL_3D_OVER_UNDER)
         fb3d.orientation == NEXUS_VideoOrientation_e3D_OverUnder;

      fb3d.main = windowState->displaySurface[windowState->backBuffer];

      NEXUS_Display_SetGraphicsFramebuffer3D(data->display, &fb3d);

      /* Do we need to wait for the vsync? */
      if (state->waitVSync)
         BKNI_WaitForEvent(data->vsyncEvent, BKNI_INFINITE);

      windowState->backBuffer = (windowState->backBuffer + 1) % 2;
   }
   else
   {
      windowState->bufferPosition += 1;
   }

   windowState->hasEverBeenDisplayed[buffer->settings.usage - BEGL_BufferUsage_eSwapChain0] = true;

   /* Now unlock the buffer that's just been undisplayed */
   oldSwapChainIndex = buffer->settings.usage - BEGL_BufferUsage_eSwapChain0;
   if (oldSwapChainIndex == 0)
      oldSwapChainIndex = windowState->actualSwapChainCount - 1;
   else
      oldSwapChainIndex--;

   if (windowState->hasEverBeenDisplayed[oldSwapChainIndex])
      SemaphoreSignal(&windowState->lockSemaphore[oldSwapChainIndex]);

   return BEGL_Success;
}

/* Flush pending displays until they are all done, then removes all buffers from display. Will block until complete. */
static BEGL_Error DispWindowUndisplay(void *context, BEGL_WindowState *windowState)
{
   NXPL_DisplayData        *data = (NXPL_DisplayData*)context;
   NXPL_NativeWindowInfo   *nw   = (NXPL_NativeWindowInfo*)windowState->window;
   NEXUS_GraphicsSettings   graphicsSettings;
   NEXUS_Error              err;

   NEXUS_Display_GetGraphicsSettings(data->display, &graphicsSettings);

   graphicsSettings.enabled = false;
   graphicsSettings.frameBufferCallback.callback = nullptr;

   err = NEXUS_Display_SetGraphicsSettings(data->display, &graphicsSettings);
   if (err)
      BKNI_Printf("NEXUS_Display_SetGraphicsSettings() failed during platform shutdown\n");

   /* Reset last seen data, so that the next display call will set them correctly */
   data->winWidth  = 0;
   data->winHeight = 0;
   data->winX      = 0;
   data->winY      = 0;
   data->stretch   = 0;

   if (nw && nw->destroying)
   {
      memset(nw, 0, sizeof(NXPL_NativeWindowInfo));
      free(nw);
   }

   return BEGL_Success;
}

/* Request creation of an appropriate display buffer. Only the 3D driver knows the size and alignment constraints, so the
 * buffer create request must come from the driver. settings->totalByteSize is the size of the memory that the driver needs.
 * We could have just requested a block of memory using the memory interface, but by having the platform layer create a 'buffer'
 * it can actually create whatever type it desires directly, and then only have to deal with that type. For example, in a Nexus
 * platform layer, this function might be implemented to create a NEXUS_Surface (with the correct memory constraints of course).
 * When the buffer handle is passed out during BufferDisplay, the platform layer can simply use it as a NEXUS_Surface. It
 * doesn't have to wrap the memory each time, or perform any lookups. Since the buffer handle is opaque to the 3d driver, the
 * platform layer has complete freedom. */
static BEGL_BufferHandle DispBufferCreate(void *context, BEGL_BufferSettings *settings)
{
   NXPL_DisplayData              *data       = (NXPL_DisplayData*)context;
   NXPL_BufferData               *buffer     = nullptr;

   NEXUS_SurfaceCreateSettings    surfSettings;
   NEXUS_MemoryAllocationSettings memSettings;
   NEXUS_MemoryStatus             memStatus;
   uint32_t                       bpp;

   buffer = (NXPL_BufferData*)malloc(sizeof(NXPL_BufferData));
   memset(buffer, 0, sizeof(NXPL_BufferData));

   NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);

   surfSettings.pixelFormat = GetNexusFormat(settings->format);
   surfSettings.width       = settings->width;
   surfSettings.height      = settings->height;
   surfSettings.alignment   = settings->alignment;
   surfSettings.pitch       = settings->pitchBytes;
   surfSettings.heap        = data->heap;

   NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
   memSettings.alignment  = settings->alignment;
   memSettings.heap       = data->heap;

   // Surface must be created in our heap, and always with a cached address
   NEXUS_Memory_Allocate(settings->totalByteSize, &memSettings, &surfSettings.pMemory);
   if (surfSettings.pMemory != nullptr)
      buffer->surface = NEXUS_Surface_Create(&surfSettings);

   // Note: these 3 lines are very deliberate, it means that FORCE_UNCACHED works correctly.
   // Be very careful if you change them. Think about uncached mode!
   NEXUS_Heap_GetStatus(data->heap, &memStatus);
   settings->physOffset = memStatus.offset + ((uintptr_t)surfSettings.pMemory - (uintptr_t)memStatus.addr);
   settings->cachedAddr = data->memInterface->ConvertPhysicalToCached(data->memInterface->context, settings->physOffset);

   buffer->settings = *settings;

   return (BEGL_BufferHandle)buffer;
}

/* Destroy a buffer previously created with BufferCreate */
static BEGL_Error DispBufferDestroy(void *context, BEGL_BufferDisplayState *bufferState)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   NXPL_BufferData *buffer = (NXPL_BufferData*)bufferState->buffer;

   if (buffer != nullptr)
   {
      NEXUS_SurfaceCreateSettings createSettings;

      if (buffer->surface)
      {
         NEXUS_Surface_GetCreateSettings(buffer->surface, &createSettings);
         NEXUS_Surface_Destroy(buffer->surface);

         // We do this using the create settings to ensure it's a cached ptr, even in FORCE_UNCACHED
         NEXUS_Memory_Free(createSettings.pMemory);
      }

      memset(buffer, 0, sizeof(NXPL_BufferData));
      free(buffer);
   }

   return BEGL_Success;
}

/* Get information about a created window buffer */
static BEGL_Error DispBufferGetCreateSettings(void *context, BEGL_BufferHandle bufHandle, BEGL_BufferSettings *settings)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   NXPL_BufferData *buffer = (NXPL_BufferData*)bufHandle;

   if (buffer != nullptr)
   {
      *settings = buffer->settings;
      return BEGL_Success;
   }

   return BEGL_Fail;
}

/* Called to determine current size of the window referenced by the opaque window handle.
 * This is needed by EGL in order to know the size of a native 'window'. */
static BEGL_Error DispWindowGetInfo(void *context,
                                    BEGL_WindowHandle window,
                                    BEGL_WindowInfoFlags flags,
                                    BEGL_WindowInfo *info)
{
   NXPL_NativeWindowInfo *nw = (NXPL_NativeWindowInfo*)window;

   if (nw != nullptr)
   {
      char *val;

      info->width  = nw->width;
      info->height = nw->height;

      val = getenv("V3D_DOUBLE_BUFFER");
      if (val && (val[0] == 't' || val[0] == 'T' || val[0] == '1'))
         info->swapchain_count = 2;
      else
         info->swapchain_count = MAX_SWAP_BUFFERS;

      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_BufferHandle DispBufferGet(void *context, BEGL_BufferSettings *settings)
{
   NXPL_DisplayData     *data        = (NXPL_DisplayData*)context;
   NXPL_BufferData      *buffer      = nullptr;
   NXPL_WindowState     *windowState = (NXPL_WindowState *)settings->windowState.platformState;

   buffer = (NXPL_BufferData*)DispBufferCreate(context, settings);

   if (buffer)
   {
      assert(windowState->buffers[settings->usage - BEGL_BufferUsage_eSwapChain0] == nullptr);
      windowState->buffers[settings->usage - BEGL_BufferUsage_eSwapChain0] = buffer;

      if (settings->usage - BEGL_BufferUsage_eSwapChain0 >= windowState->actualSwapChainCount)
         windowState->actualSwapChainCount = settings->usage - BEGL_BufferUsage_eSwapChain0 + 1;
   }

   return (BEGL_BufferHandle)buffer;
}

static BEGL_Error DispBufferAccess(void *context, BEGL_BufferAccessState *bufferAccess)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;

   if (data)
   {
      NXPL_WindowState *windowState = (NXPL_WindowState *)bufferAccess->windowState.platformState;
      NXPL_BufferData  *buffer      = (NXPL_BufferData  *)bufferAccess->buffer;

      SemaphoreWait(&windowState->lockSemaphore[buffer->settings.usage - BEGL_BufferUsage_eSwapChain0]);

      return BEGL_Success;
   }

   return BEGL_Fail;
}

static void BlitComplete(void *data, int unused)
{
   BSTD_UNUSED(unused);
   BKNI_SetEvent((BKNI_EventHandle)data);
}

static void *DispWindowStateCreate(void *context, BEGL_WindowHandle window)
{
   NXPL_WindowState *windowState = (NXPL_WindowState *)malloc(sizeof(NXPL_WindowState));
   uint32_t          i;

   memset(windowState, 0, sizeof(NXPL_WindowState));

   for (i = 0; i < MAX_SWAP_BUFFERS; i++)
      SemaphoreCreate(&windowState->lockSemaphore[i], 1);

   for (i = 0; i < NUM_DISPLAY_BUFFERS; ++i)
      windowState->displaySurface[i] = nullptr;

   windowState->backBuffer     = 0;
   windowState->bufferPosition = 0;

   BKNI_CreateEvent(&windowState->blitEvent);

   if (windowState->blitEvent == nullptr)
   {
      printf("\n\nNXPL : DispWindowStateCreate() ERROR.\nCould not create event\n");
   }

   windowState->gfx2d = NEXUS_Graphics2D_Open(0, nullptr);

   if (windowState->gfx2d == nullptr)
   {
      printf("\n\nNXPL : DispWindowStateCreate() ERROR.\nCould not start graphics 2D module\n");
   }
   else
   {
      NEXUS_Graphics2DSettings    gfxSettings;

      NEXUS_Graphics2D_GetSettings(windowState->gfx2d, &gfxSettings);
      gfxSettings.checkpointCallback.callback = BlitComplete;
      gfxSettings.checkpointCallback.context  = windowState->blitEvent;
      NEXUS_Graphics2D_SetSettings(windowState->gfx2d, &gfxSettings);
   }

   return (void*)windowState;
}

static BEGL_Error DispWindowStateDestroy(void *context, void *swapChainCtx)
{
   NXPL_WindowState *windowState = (NXPL_WindowState *)swapChainCtx;
   uint32_t          i;

   if (windowState)
   {
      for (i = 0; i < NUM_DISPLAY_BUFFERS; i++)
         if (windowState->displaySurface[i] != nullptr)
           NEXUS_Surface_Destroy(windowState->displaySurface[i]);

      if (windowState->gfx2d != nullptr)
         NEXUS_Graphics2D_Close(windowState->gfx2d);

      if (windowState->blitEvent != nullptr)
         BKNI_DestroyEvent(windowState->blitEvent);

      for (i = 0; i < MAX_SWAP_BUFFERS; i++)
         SemaphoreDelete(&windowState->lockSemaphore[i]);

      memset(windowState, 0, sizeof(NXPL_WindowState));
      free(windowState);
      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_DisplayInterface *CreateQuadDisplayInterface(BEGL_MemoryInterface *memIface,
                                                         NEXUS_DISPLAYHANDLE display,
                                                         BEGL_DisplayCallbacks *displayCallbacks,
                                                         uint32_t numPanelsX,
                                                         uint32_t numPanelsY)
{
   NXPL_DisplayData        *data;
   BEGL_DisplayInterface   *disp = (BEGL_DisplayInterface*)malloc(sizeof(BEGL_DisplayInterface));

   if (disp != nullptr)
   {
      data = (NXPL_DisplayData*)malloc(sizeof(NXPL_DisplayData));
      memset(disp, 0, sizeof(BEGL_DisplayInterface));

      if (data != nullptr)
      {
         NEXUS_MemoryStatus            memStatus;
         int topTwoBitsHeapStart;
         int topTwoBitsHeapEnd;

         memset(data, 0, sizeof(NXPL_DisplayData));

         disp->context                    = (void*)data;
         disp->BufferDisplay              = DispBufferDisplay;
         disp->BufferCreate               = DispBufferCreate;
         disp->BufferGet                  = DispBufferGet;
         disp->BufferDestroy              = DispBufferDestroy;
         disp->BufferGetCreateSettings    = DispBufferGetCreateSettings;
         disp->BufferAccess               = DispBufferAccess;
         disp->WindowUndisplay            = DispWindowUndisplay;
         disp->WindowGetInfo              = DispWindowGetInfo;
         disp->WindowPlatformStateCreate  = DispWindowStateCreate;
         disp->WindowPlatformStateDestroy = DispWindowStateDestroy;

         data->numPanelsX = numPanelsX;
         data->numPanelsY = numPanelsY;
         data->numPanels  = numPanelsX * numPanelsY;

         data->memInterface              = memIface;
         data->display                   = display;
         data->bufferGetRequirementsFunc = displayCallbacks->BufferGetRequirements;

         BKNI_CreateEvent(&data->vsyncEvent);

         /* Store the heap settings */
         data->heap = NXPL_MemHeap(memIface);

         NEXUS_Heap_GetStatus(data->heap, &memStatus);
         topTwoBitsHeapStart = memStatus.offset >> 30;
         topTwoBitsHeapEnd   = (memStatus.offset + memStatus.size - 1) >> 30;
         if (topTwoBitsHeapStart != topTwoBitsHeapEnd)
         {
            printf("\n\nCreateQuadDisplayInterface() ERROR.\nThe Heap you have selected in your platform layer straddles a 1GB boundary\n"
                   "Start %p, Size %p\n", memStatus.offset, memStatus.size);
            goto error1;
         }
      }
      else
      {
         goto error0;
      }
   }

   return disp;

error1:
   free(data);

error0:
   free(disp);
   return nullptr;
}

static void DestroyQuadDisplayInterface(BEGL_DisplayInterface *disp)
{
   if (disp != nullptr)
   {
      NXPL_DisplayData *data = (NXPL_DisplayData *)disp->context;

      if (data->vsyncEvent)
         BKNI_DestroyEvent(data->vsyncEvent);

      if (data != nullptr)
         free(data);

      memset(disp, 0, sizeof(BEGL_DisplayInterface));
      free(disp);
   }
}

void BSG_RegisterNexusQuadDisplayPlatform(NXPL_PlatformHandle *handle, NEXUS_DISPLAYHANDLE display,
                                          uint32_t numPanelsX, uint32_t numPanelsY)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)malloc(sizeof(BEGL_DriverInterfaces));
   memset(data, 0, sizeof(BEGL_DriverInterfaces));

   if (data != nullptr)
   {
      BEGL_GetDefaultDriverInterfaces(data);

      data->hwInterface      = NXPL_CreateHWInterface(&data->hardwareCallbacks);
      data->memInterface     = NXPL_CreateMemInterface(data->hwInterface);
      data->displayInterface = CreateQuadDisplayInterface(data->memInterface, display, &data->displayCallbacks,
                                                          numPanelsX, numPanelsY);

      *handle = (NXPL_PlatformHandle)data;

      BEGL_RegisterDriverInterfaces(data);
   }
}

void BSG_UnregisterNexusQuadDisplayPlatform(NXPL_PlatformHandle handle)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != nullptr)
   {
      /* Clear out all of our interface pointers and register the empty ones.
       * Do this before destroying the interfaces in case the driver needs to use them
       * to shutdown correctly */
      BEGL_DriverInterfaces nulliface;
      memset(&nulliface, 0, sizeof(BEGL_DriverInterfaces));
      BEGL_GetDefaultDriverInterfaces(&nulliface);
      BEGL_RegisterDriverInterfaces(&nulliface);

      DestroyQuadDisplayInterface(data->displayInterface);
      NXPL_DestroyMemInterface(data->memInterface);
      NXPL_DestroyHWInterface(data->hwInterface);

      memset(data, 0, sizeof(BEGL_DriverInterfaces));
      free(data);
   }
}

#endif
