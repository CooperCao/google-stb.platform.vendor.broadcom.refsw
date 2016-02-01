/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
DESC
=============================================================================*/

#include "default_nexus.h"

#include "nexus_platform.h"

#include <EGL/egl.h>

#include <malloc.h>
#include <memory.h>
#include <semaphore.h>

#include "png.h"

#define MAX_SWAP_BUFFERS 3

typedef void (*BufferGetRequirementsFunc)(BEGL_PixmapInfo *bufferRequirements, BEGL_BufferSettings *bufferConstrainedRequirements);

typedef struct
{
   BufferGetRequirementsFunc  bufferGetRequirementsFunc;
   NEXUS_DisplayHandle     display;
   NEXUS_HeapHandle        heap;
   uint32_t                winWidth;
   uint32_t                winHeight;
   uint32_t                winX;
   uint32_t                winY;
   bool                    stretch;
   BEGL_MemoryInterface    *memInterface;
} NXPL_DisplayData;

typedef struct
{
   BEGL_BufferSettings     settings;
   NEXUS_SURFACEHANDLE     surface;
} NXPL_BufferData;

/* There will be one WINPL_WindowState for each WINPL_SurfCompNativeWindow */
typedef struct
{
   sem_t                  lockSemaphore;
} NXPL_WindowState;

/*****************************************************************************
 * Display driver interface
 *****************************************************************************/
static BEGL_Error DispBufferDisplay(void *context, BEGL_BufferDisplayState *state)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   NXPL_WindowState *windowState = (NXPL_WindowState *)state->windowState.platformState;

#ifdef PNG_OUT
   NXPL_BufferData *buffer = (NXPL_BufferData*)state->buffer;
   NXPL_NativeWindowInfo *nw = (NXPL_NativeWindowInfo*)state->windowState.window;
   FILE *fp;
   char fname[256];
   png_structp png_ptr;
   png_infop info_ptr = NULL;
   bool hasAlpha = true;
   png_bytepp row_pointers;
   int y, res = 0;

   sprintf(fname, "dump%d.png", nw->x++);
   /* create file */
   fp = fopen(fname, "wb");
   if (!fp)
      return 0;

   /* initialize stuff */
   png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
      goto error0;

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
      goto error0;

   if (setjmp(png_jmpbuf(png_ptr)))
      goto error0;

   png_init_io(png_ptr, fp);

   /* write header */
   if (setjmp(png_jmpbuf(png_ptr)))
      goto error0;

   png_set_IHDR(png_ptr, info_ptr, buffer->settings.width, buffer->settings.height,
            8, hasAlpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

   png_write_info(png_ptr, info_ptr);

   row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * buffer->settings.height);
   if (!row_pointers)
      goto error0;

   /* initialize the rowpointers for the image */
   row_pointers[nw->height-1] = (png_bytep)buffer->settings.cachedAddr;
   for (y=nw->height-1; y>0; y--)
      row_pointers[y-1] = (png_bytep)(row_pointers[y] + buffer->settings.pitchBytes);

   /* write bytes */
   if (setjmp(png_jmpbuf(png_ptr)))
      goto error1;

   png_write_image(png_ptr, row_pointers);

   /* end write */
   if (setjmp(png_jmpbuf(png_ptr)))
      goto error1;

   png_write_end(png_ptr, NULL);

   /* success */
   res = 1;

error1:
   /* cleanup heap allocation */
   free(row_pointers);

error0:
   png_destroy_write_struct(&png_ptr, &info_ptr);

   fclose(fp);

#endif

   sem_post(&windowState->lockSemaphore);

   return BEGL_Success;
}

/* Flush pending displays until they are all done, then removes all buffers from display. Will block until complete. */
static BEGL_Error DispWindowUndisplay(void *context, BEGL_WindowState *windowState)
{
   NXPL_NativeWindowInfo   *nw = (NXPL_NativeWindowInfo*)windowState->window;

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
   NXPL_DisplayData              *data = (NXPL_DisplayData*)context;
   NXPL_BufferData               *buffer = NULL;
   NEXUS_MemoryAllocationSettings memSettings;
   NEXUS_MemoryStatus             memStatus;
   uint32_t                       bpp;

   buffer = (NXPL_BufferData*)malloc(sizeof(NXPL_BufferData));
   if (buffer != NULL)
   {
      memset(buffer, 0, sizeof(NXPL_BufferData));

      NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
      memSettings.alignment = settings->alignment;
      memSettings.heap = data->heap;

      NEXUS_Memory_Allocate(settings->totalByteSize, &memSettings, &settings->cachedAddr);

      NEXUS_Heap_GetStatus(data->heap, &memStatus);
      settings->physOffset = memStatus.offset + ((uintptr_t)settings->cachedAddr - (uintptr_t)memStatus.addr);

      buffer->settings = *settings;
   }

   return (BEGL_BufferHandle)buffer;
}

/* Destroy a buffer previously created with BufferCreate */
static BEGL_Error DispBufferDestroy(void *context, BEGL_BufferDisplayState *bufferState)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   NXPL_BufferData *buffer = (NXPL_BufferData*)bufferState->buffer;

   if (buffer != NULL)
   {
      NEXUS_Memory_Free(buffer->settings.cachedAddr);

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

   if (buffer != NULL)
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

   if (nw != NULL)
   {
      info->width = nw->width;
      info->height = nw->height;
      info->swapchain_count = MAX_SWAP_BUFFERS;
      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_BufferHandle DispBufferGet(void *context, BEGL_BufferSettings *settings)
{
   NXPL_DisplayData     *data = (NXPL_DisplayData*)context;
   NXPL_BufferData      *buffer = NULL;

   buffer = (NXPL_BufferData*)DispBufferCreate(context, settings);

   return (BEGL_BufferHandle)buffer;
}

static BEGL_Error DispBufferAccess(void *context, BEGL_BufferAccessState *bufferAccess)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;

   if (data)
   {
      NXPL_WindowState *windowState = (NXPL_WindowState *)bufferAccess->windowState.platformState;
      NXPL_BufferData  *buffer = bufferAccess->buffer;

      sem_wait(&windowState->lockSemaphore);

      return BEGL_Success;
   }

   return BEGL_Fail;
}

static void *DispWindowStateCreate(void *context, BEGL_WindowHandle window)
{
   NXPL_WindowState *windowState = (NXPL_WindowState *)malloc(sizeof(NXPL_WindowState));
   if (windowState != NULL)
   {
      memset(windowState, 0, sizeof(NXPL_WindowState));
      sem_init(&windowState->lockSemaphore, 0, MAX_SWAP_BUFFERS);
   }

   return (void*)windowState;
}

static BEGL_Error DispWindowStateDestroy(void *context, void *swapChainCtx)
{
   NXPL_WindowState *windowState = (NXPL_WindowState *)swapChainCtx;

   if (windowState)
   {
      sem_destroy(&windowState->lockSemaphore);

      memset(windowState, 0, sizeof(NXPL_WindowState));
      free(windowState);
      return BEGL_Success;
   }

   return BEGL_Fail;
}

BEGL_DisplayInterface *NXPL_CreateDisplayInterface(BEGL_MemoryInterface *memIface,
                                                   NEXUS_DisplayHandle display, BEGL_DisplayCallbacks *displayCallbacks)
{
   NXPL_DisplayData *data;
   BEGL_DisplayInterface *disp = (BEGL_DisplayInterface*)malloc(sizeof(BEGL_DisplayInterface));

   if (disp != NULL)
   {
      data = (NXPL_DisplayData*)malloc(sizeof(NXPL_DisplayData));
      memset(disp, 0, sizeof(BEGL_DisplayInterface));

      if (data != NULL)
      {
         NEXUS_MemoryStatus            memStatus;
         int topTwoBitsHeapStart;
         int topTwoBitsHeapEnd;

         memset(data, 0, sizeof(NXPL_DisplayData));

         disp->context = (void*)data;
         disp->BufferDisplay = DispBufferDisplay;
         disp->WindowUndisplay = DispWindowUndisplay;
         disp->BufferCreate = DispBufferCreate;
         disp->BufferGet = DispBufferGet;
         disp->BufferDestroy = DispBufferDestroy;
         disp->BufferGetCreateSettings = DispBufferGetCreateSettings;
         disp->BufferAccess = DispBufferAccess;
         disp->WindowGetInfo = DispWindowGetInfo;
         disp->WindowPlatformStateCreate = DispWindowStateCreate;
         disp->WindowPlatformStateDestroy = DispWindowStateDestroy;

         data->memInterface = memIface;
         data->display = display;
         data->bufferGetRequirementsFunc = displayCallbacks->BufferGetRequirements;

         /* Store the heap settings */
         data->heap = (NEXUS_HeapHandle)NXPL_MemHeap(memIface);

         NEXUS_Heap_GetStatus(data->heap, &memStatus);
         topTwoBitsHeapStart = memStatus.offset >> 30;
         topTwoBitsHeapEnd = (memStatus.offset + memStatus.size - 1) >> 30;
         if (topTwoBitsHeapStart != topTwoBitsHeapEnd)
         {
            printf("\n\nNXPL : NXPL_CreateDisplayInterface() ERROR.\nThe Heap you have selected in your platform layer straddles a 1GB boundary\n"
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
   return NULL;
}

void NXPL_DestroyDisplayInterface(BEGL_DisplayInterface *mem)
{
   if (mem != NULL)
   {
      if (mem->context != NULL)
         free(mem->context);

      memset(mem, 0, sizeof(BEGL_DisplayInterface));
      free(mem);
   }
}

bool NXPL_BufferGetRequirements(NXPL_PlatformHandle handle,
                                BEGL_PixmapInfo *bufferRequirements,
                                BEGL_BufferSettings * bufferConstrainedRequirements)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != NULL && data->displayCallbacks.BufferGetRequirements != NULL)
   {
      data->displayCallbacks.BufferGetRequirements(bufferRequirements, bufferConstrainedRequirements);
      return true;
   }

   return false;
}

bool NXPL_CreateCompatiblePixmap(NXPL_PlatformHandle handle, void **pixmapHandle, NEXUS_SURFACEHANDLE *surface, BEGL_PixmapInfo *info)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != NULL && data->displayCallbacks.PixmapCreateCompatiblePixmap != NULL)
   {
      BEGL_BufferHandle buffer = data->displayCallbacks.PixmapCreateCompatiblePixmap(info);
      if (buffer != NULL)
      {
         *pixmapHandle = (void*)buffer;
         *surface = ((NXPL_BufferData*)buffer)->surface;
         return true;
      }
   }

   return false;
}

void NXPL_DestroyCompatiblePixmap(NXPL_PlatformHandle handle, void *pixmapHandle)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != NULL &&
       data->displayInterface != NULL &&
       data->displayInterface->BufferDestroy)
   {
      BEGL_BufferDisplayState bufferState;
      memset(&bufferState, 0, sizeof(BEGL_BufferDisplayState));
      bufferState.buffer = (BEGL_BufferHandle)pixmapHandle;

      data->displayInterface->BufferDestroy(data->displayInterface->context, &bufferState);
   }
}

void *NXPL_CreateNativeWindow(const NXPL_NativeWindowInfo *info)
{
   NXPL_NativeWindowInfo *nw = (NXPL_NativeWindowInfo*)malloc(sizeof(NXPL_NativeWindowInfo));
   memset(nw, 0, sizeof(NXPL_NativeWindowInfo));
   if (nw != NULL && info != NULL)
      *nw = *info;

   return nw;
}

void NXPL_UpdateNativeWindow(void *native, const NXPL_NativeWindowInfo *info)
{
   if (info != NULL && native != NULL)
      *((NXPL_NativeWindowInfo*)native) = *info;
}

void NXPL_DestroyNativeWindow(void *nativeWin)
{
   if (nativeWin != NULL)
   {
      NXPL_NativeWindowInfo *nw = (NXPL_NativeWindowInfo*)nativeWin;
      memset(nw, 0, sizeof(NXPL_NativeWindowInfo));
      free(nw);
   }
}
