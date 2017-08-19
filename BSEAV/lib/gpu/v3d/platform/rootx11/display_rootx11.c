/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "default_rootx11.h"
#include "display_rootx11.h"

#include <xf86drm.h>
#include <X11/Xutil.h>
#include <X11/Xmd.h>
#include <X11/extensions/dri2proto.h>
#include <X11/extensions/dri2.h>

#include "nexus_surface.h"
#include "nexus_memory.h"
#include "nexus_platform.h"

#include <EGL/egl.h>

#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <semaphore.h>
#include <errno.h>

#define MAX_SWAP_BUFFERS 3

typedef void (*BufferGetRequirementsFunc)(BEGL_PixmapInfo *bufferRequirements, BEGL_BufferSettings *bufferConstrainedRequirements);

typedef struct
{
   BufferGetRequirementsFunc  bufferGetRequirementsFunc;
   Display                    *display;
   BEGL_MemoryInterface       *memInterface;
   void                       *rootOffset;
   unsigned int               rootWidth;
   unsigned int               rootHeight;
   unsigned int               rootChainCount;
   int                        initialized;
} RXPL_DisplayData;

typedef struct
{
   BEGL_BufferSettings     settings;
   NEXUS_SurfaceHandle     internalSurface; /* only used for pixmaps, not main windows */
   void                    *base;           /* used for main windows, not pixmaps */
} RXPL_BufferData;

/* There will be one WINPL_WindowState for each WINPL_SurfCompNativeWindow */
typedef struct
{
   sem_t                  lockSemaphore;
   Window                 window;
} RXPL_WindowState;

/*****************************************************************************/

extern NEXUS_HeapHandle NXPL_MemHeap(BEGL_MemoryInterface *mem);

static BEGL_Error DispBufferDisplay(void *context, BEGL_BufferDisplayState *state)
{
   RXPL_DisplayData *data = (RXPL_DisplayData*)context;
   RXPL_BufferData *buffer = (RXPL_BufferData*)state->buffer;
   RXPL_WindowState *windowState = (RXPL_WindowState *)state->windowState.platformState;
   CARD64 count;

   /* blocking call.  NSC will always return the previous buffer */
   DRI2SwapBuffers(data->display, windowState->window, 0, 0, 0, &count);

   sem_post(&windowState->lockSemaphore);

   return BEGL_Success;
}

/* Flush pending displays until they are all done, then removes all buffers from display. Will block until complete. */
static BEGL_Error DispWindowUndisplay(void *context, BEGL_WindowState *windowState)
{
   return BEGL_Success;
}

static BEGL_BufferHandle DispBufferCreate(void *context, BEGL_BufferSettings *settings)
{
   RXPL_DisplayData              *data = (RXPL_DisplayData*)context;
   RXPL_BufferData               *buffer = NULL;

   buffer = (RXPL_BufferData*)malloc(sizeof(RXPL_BufferData));

   if (buffer != NULL)
   {
      NEXUS_SurfaceCreateSettings    surfSettings;
      NEXUS_MemoryAllocationSettings memSettings;
      NEXUS_MemoryStatus             memStatus;
      uint32_t                       bpp;

      memset(buffer, 0, sizeof(RXPL_DisplayData));

      NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);

      switch (settings->format)
      {
         case BEGL_BufferFormat_eA8B8G8R8:
            bpp = 32;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eA8_B8_G8_R8;
            break;
         case BEGL_BufferFormat_eR8G8B8A8:
            bpp = 32;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_A8;
            break;
         case BEGL_BufferFormat_eX8B8G8R8:
            bpp = 32;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eX8_B8_G8_R8;
            break;
         case BEGL_BufferFormat_eR8G8B8X8:
            bpp = 32;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_X8;
            break;
         case BEGL_BufferFormat_eR5G6B5:
         case BEGL_BufferFormat_eR5G6B5_Texture:
            bpp = 16;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eR5_G6_B5;
            break;
         case BEGL_BufferFormat_eYUV422_Texture:
            bpp = 16;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
            break;
         case BEGL_BufferFormat_eVUY224_Texture:
            bpp = 16;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8;
            break;
         case BEGL_BufferFormat_eA8B8G8R8_Texture:
            bpp = 32;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eA8_B8_G8_R8;
            break;
         case BEGL_BufferFormat_eR8G8B8A8_Texture:
            bpp = 32;
            surfSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_A8;
            break;
         default:
            break;
      }

      surfSettings.width = settings->width;
      surfSettings.height = settings->height;
      surfSettings.alignment = settings->alignment;
      surfSettings.pitch = settings->pitchBytes;
      surfSettings.heap = NXPL_MemHeap(data->memInterface);

      NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
      memSettings.alignment = settings->alignment;
      memSettings.heap = surfSettings.heap;

      /* Surface must be created in our heap, and always with a cached address */
      NEXUS_Memory_Allocate(settings->totalByteSize, &memSettings, &surfSettings.pMemory);
      if (surfSettings.pMemory != NULL)
         buffer->internalSurface = NEXUS_Surface_Create(&surfSettings);

      NEXUS_Heap_GetStatus(memSettings.heap, &memStatus);
      settings->physOffset = memStatus.offset + ((uintptr_t)surfSettings.pMemory - (uintptr_t)memStatus.addr);
      settings->cachedAddr = surfSettings.pMemory;

      buffer->settings = *settings;
   }

   return (BEGL_BufferHandle)buffer;
}

/* Destroy a buffer previously created with BufferCreate */
static BEGL_Error DispBufferDestroy(void *context, BEGL_BufferDisplayState *bufferState)
{
   RXPL_DisplayData *data = (RXPL_DisplayData*)context;
   RXPL_BufferData *buffer = (RXPL_BufferData*)bufferState->buffer;

   if (buffer != NULL)
   {
      if (buffer->settings.usage == BEGL_BufferUsage_ePixmap)
      {
         /* for pixmap surfaces, the surface is always created via DispBufferCreate() */
         /* in this case we stashed the nexus handle in internalSurface */
         if (buffer->internalSurface && buffer->settings.cachedAddr)
         {
            NEXUS_Surface_Destroy(buffer->internalSurface);
            NEXUS_Memory_Free(buffer->settings.cachedAddr);
         }
      }
      /* for main root X window, this belongs to the server, so doesnt need clearing up */
      memset(buffer, 0, sizeof(RXPL_BufferData));
      free(buffer);
   }

   return BEGL_Success;
}

/* Get information about a created window buffer */
static BEGL_Error DispBufferGetCreateSettings(void *context, BEGL_BufferHandle bufHandle, BEGL_BufferSettings *settings)
{
   RXPL_DisplayData *data = (RXPL_DisplayData*)context;
   RXPL_BufferData *buffer = (RXPL_BufferData*)bufHandle;

   if (buffer != NULL)
   {
      *settings = buffer->settings;
      return BEGL_Success;
   }

   return BEGL_Fail;
}

static const DRI2EventOps ops = {
        .WireToEvent = NULL,
        .EventToWire = NULL,
};

/* Called to determine current size of the window referenced by the opaque window handle.
 * This is needed by EGL in order to know the size of a native 'window'. */
static BEGL_Error DispWindowGetInfo(void *context,
                                    BEGL_WindowHandle window,
                                    BEGL_WindowInfoFlags flags,
                                    BEGL_WindowInfo *info)
{
   RXPL_DisplayData *data = (RXPL_DisplayData*)context;

   if (!data->initialized)
   {
        char *device;
        char *driver;
        drm_magic_t magic;
        Window root;

        if (!DRI2InitDisplay(data->display, &ops))
        {
            printf("DRI2InitDisplay failed");
            return BEGL_Fail;
        }

        root = RootWindow(data->display, 0);

        if (!DRI2Connect(data->display, root, DRI2DriverDRI, &driver, &device))
        {
            printf("DRI2Connect failed");
            return BEGL_Fail;
        }

        printf("DRI2Connect: driver=%s, device=%s\n", (driver != NULL) ? driver : "", (device != NULL) ? device : "");

        sscanf(device, "%p %d %d %d",
        &data->rootOffset,
        &data->rootWidth,
        &data->rootHeight,
        &data->rootChainCount);

        magic = 0xB20ADC08;
        if (!DRI2Authenticate(data->display, root, magic))
        {
            printf("DRI2Authenticate failed");
            return BEGL_Fail;
        }

        data->initialized = 1;
   }

   if (flags & BEGL_WindowInfoWidth)
      info->width = data->rootWidth;
   if (flags & BEGL_WindowInfoHeight)
      info->height = data->rootHeight;

   if (flags & BEGL_WindowInfoSwapChainCount)
      info->swapchain_count = data->rootChainCount;

   if (flags & BEGL_WindowInfoRefreshRate)
      info->refreshRateMilliHertz = 16;

   return BEGL_Success;
}

static BEGL_BufferHandle DispBufferGet(void *context, BEGL_BufferSettings *settings)
{
   RXPL_DisplayData     *data = (RXPL_DisplayData*)context;
   RXPL_BufferData      *buffer = NULL;
   RXPL_WindowState     *windowState = (RXPL_WindowState *)settings->windowState.platformState;

   /* root X11 only provides one buffer in the underlying platform.  Our server implements
      three buffers from one display, so X can stay the same.

      +----------------+
      |                |
      | Buffer 1       |
      |                |
      +----------------+
      |                |
      | Buffer 2       |
      |                |
      +----------------+
      |                |
      | Buffer 3       |
      |                |
      +----------------+

      For the final solution we'll read the frame base pointer and parameters via DRI2Open, but
      this isn't implemented yet.  Just hack for now */

   buffer = (RXPL_BufferData*)malloc(sizeof(RXPL_BufferData));
   if (buffer)
   {
      void *sra;
      memset(buffer, 0, sizeof(RXPL_BufferData));

      sra = (void *)(uintptr_t)(data->rootOffset +
         ((settings->usage - BEGL_BufferUsage_eSwapChain0) *
            ((data->rootWidth * sizeof(unsigned int)) * data->rootHeight)));

      fprintf(stderr, "sra = %p\n", sra);

      /* in the final version, this will be mathematically offset from the usage (BEGL_BufferUsage_eSwapChain0) */
      buffer->base = sra;

      /* structure copy -> grab the elements above (inputs) to the buffer structure */
      buffer->settings = *settings;
   }

   return (BEGL_BufferHandle)buffer;
}

static BEGL_Error DispBufferAccess(void *context, BEGL_BufferAccessState *bufferAccess)
{
   RXPL_DisplayData *data = (RXPL_DisplayData*)context;

   if (data)
   {
      RXPL_WindowState *windowState = (RXPL_WindowState *)bufferAccess->windowState.platformState;
      RXPL_BufferData  *buffer = bufferAccess->buffer;

      while (sem_wait(&windowState->lockSemaphore) == -1 && errno == EINTR)
         continue;

      /* update BEGL_BufferSettings with new information */
      buffer->settings.physOffset = (unsigned int)buffer->base;
      buffer->settings.pitchBytes = data->rootWidth * sizeof(unsigned int);

      /* convert physical to cached */
      buffer->settings.cachedAddr = data->memInterface->ConvertPhysicalToCached(data->memInterface->context, buffer->settings.physOffset);

      buffer->settings.width = data->rootWidth;
      buffer->settings.height = data->rootHeight;

      buffer->settings.format = BEGL_BufferFormat_eA8B8G8R8;

      return BEGL_Success;
   }

   return BEGL_Fail;
}

static void *DispWindowStateCreate(void *context, BEGL_WindowHandle window)
{
   RXPL_WindowState *windowState = (RXPL_WindowState *)malloc(sizeof(RXPL_WindowState));
   RXPL_DisplayData *data = (RXPL_DisplayData*)context;
   uint32_t          i;
   int               nbufs, w, h;

   fprintf(stderr, "DispWindowStateCreate window=0x%x\n", window);

   static unsigned attachments[] = {
      DRI2BufferBackLeft,
   };

   memset(windowState, 0, sizeof(RXPL_WindowState));

   windowState->window = (Window)window;

   XMapWindow(data->display, (Window)window);

   DRI2CreateDrawable(data->display, (Window)window);

   DRI2GetBuffers(data->display, (Window)window, &w, &h, attachments, 1, &nbufs);

   sem_init(&windowState->lockSemaphore, 0, MAX_SWAP_BUFFERS - 1);

   return (void*)windowState;
}

static BEGL_Error DispWindowStateDestroy(void *context, void *swapChainCtx)
{
   RXPL_WindowState *windowState = (RXPL_WindowState *)swapChainCtx;

   if (windowState)
   {
      sem_destroy(&windowState->lockSemaphore);

      memset(windowState, 0, sizeof(RXPL_WindowState));
      free(windowState);
      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_Error DispSetDisplayID(void *context, unsigned int displayID, unsigned int *outEglDisplayID)
{
   RXPL_DisplayData *data = (RXPL_DisplayData*)context;

   if (outEglDisplayID != NULL)
   {
      /* X display ID may be required for DRI stuff... keep it */
      data->display = (Display *)displayID;
      /* only support root window X, so a single display */
      *outEglDisplayID = 1;
      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_Error DispGetNativeFormat(void *context,
                                      BEGL_BufferFormat bufferFormat,
                                      uint32_t * nativeFormat)
{
   BEGL_Error res = BEGL_Success;
   if (nativeFormat != NULL)
   {
      /* these were discovered by running xdpyinfo on the server
         I couldn't find them in the X11 sources */
      if (bufferFormat == BEGL_BufferFormat_eA8B8G8R8)
         *nativeFormat = 0x45;
      else if (bufferFormat == BEGL_BufferFormat_eX8B8G8R8)
         *nativeFormat = 0x21;
      else
         res = BEGL_Fail;
   }

   return res;
}

BEGL_DisplayInterface *RXPL_CreateDisplayInterface(BEGL_MemoryInterface *memIface,
                                                   Display *display, BEGL_DisplayCallbacks *displayCallbacks)
{
   RXPL_DisplayData        *data;
   BEGL_DisplayInterface   *disp = (BEGL_DisplayInterface*)malloc(sizeof(BEGL_DisplayInterface));

   if (disp != NULL)
   {
      data = (RXPL_DisplayData*)malloc(sizeof(RXPL_DisplayData));
      memset(disp, 0, sizeof(BEGL_DisplayInterface));

      if (data != NULL)
      {
         memset(data, 0, sizeof(RXPL_DisplayData));

         disp->context = (void*)data;
         disp->BufferDisplay = DispBufferDisplay;
         disp->BufferCreate = DispBufferCreate;
         disp->BufferGet = DispBufferGet;
         disp->BufferDestroy = DispBufferDestroy;
         disp->BufferGetCreateSettings = DispBufferGetCreateSettings;
         disp->BufferAccess = DispBufferAccess;
         disp->WindowUndisplay = DispWindowUndisplay;
         disp->WindowGetInfo = DispWindowGetInfo;
         disp->WindowPlatformStateCreate = DispWindowStateCreate;
         disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
         disp->SetDisplayID = DispSetDisplayID;
         disp->GetNativeFormat = DispGetNativeFormat;

         data->memInterface = memIface;
         printf("%s %d %p\n", __FUNCTION__, __LINE__, display);
         data->display = display;
         data->bufferGetRequirementsFunc = displayCallbacks->BufferGetRequirements;
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
   return NULL;
}

void RXPL_DestroyDisplayInterface(BEGL_DisplayInterface *disp)
{
   if (disp != NULL)
   {
      RXPL_DisplayData *data = (RXPL_DisplayData *)disp->context;

      if (data != NULL)
         free(data);

      memset(disp, 0, sizeof(BEGL_DisplayInterface));
      free(disp);
   }
}
