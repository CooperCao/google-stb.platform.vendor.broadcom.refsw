/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <memory>
#include <cassert>

#include "default_directfb.h"

#include "nexus_surface.h"
#include "nexus_memory.h"
#include "nexus_platform.h"

#include "windowstate.h"
#include "bitmap.h"
#include "../helpers/semaphore.h"

enum
{
   PIXMAP_INFO_MAGIC = 0x15EEB1A5
};

#define MAX_SWAP_BUFFERS 3

/*****************************************************************************
 * Display driver interface
 *****************************************************************************/

static BEGL_BufferHandle DispBufferCreate(void *context, BEGL_PixmapInfoEXT const *bufferRequirements)
{
   auto data = static_cast<DBPL_Display *>(context);
   auto bitmap = new dbpl::Bitmap(context, bufferRequirements);
   return static_cast<BEGL_BufferHandle>(bitmap);
}

/* Destroy a buffer previously created with BufferCreate */
static BEGL_Error DispBufferDestroy(void *context __attribute__((unused)), BEGL_BufferDisplayState *bufferState)
{
   auto bitmap = static_cast<dbpl::Bitmap *>(bufferState->buffer);
   delete bitmap;
   return BEGL_Success;
}

static BEGL_BufferHandle DispBufferDequeue(void *context __attribute__((unused)), void *platformState, BEGL_BufferFormat format __attribute__((unused)), int *fd)
{
   auto windowState = static_cast<dbpl::WindowState *>(platformState);

   // not needed on directfb
   *fd = -1;

   auto bitmap = windowState->GetBitmap();
   bitmap->Lock();

   return static_cast<BEGL_BufferHandle>(bitmap);
}

static BEGL_Error DispBufferQueue(void *context, void *platformState, BEGL_BufferHandle buffer, int swap_interval, int fd)
{
   if (buffer != NULL)
   {
      auto data = static_cast<DBPL_Display *>(context);
      auto windowState = static_cast<dbpl::WindowState *>(platformState);
      std::unique_ptr<helper::Semaphore> fence;
      if (fd != -1)
         fence = std::move(std::unique_ptr<helper::Semaphore>(static_cast<helper::Semaphore*>(data->hwInterface->FenceGet(data->hwInterface->context, fd))));
      else
         fence = std::move(std::unique_ptr<helper::Semaphore>(new helper::Semaphore(1)));

      if (fence)
         fence->wait();

      auto bitmap = static_cast<dbpl::WrappedBitmap*>(buffer);
      bitmap->Unlock();

      auto surface = bitmap->GetSurface();
      DFBCHECK( surface->Flip( surface, NULL, swap_interval > 0 ? DSFLIP_WAITFORSYNC : DSFLIP_NONE ));
   }

   return BEGL_Success;
}

static BEGL_Error DispBufferCancel(void *context __attribute__((unused)), void *platformState __attribute__((unused)),
                                   BEGL_BufferHandle buffer __attribute__((unused)), int fd __attribute__((unused)))
{
   return BEGL_Success;
}

static void *DispWindowStateCreate(void *context, BEGL_WindowHandle window, bool secure __attribute__((unused)))
{
   std::unique_ptr<dbpl::WindowState> windowState(new dbpl::WindowState(context, window));

   return static_cast<void*>(windowState.release());
}

static BEGL_Error DispWindowStateDestroy(void *context __attribute__((unused)), void *swapChainCtx)
{
   auto windowState = static_cast<dbpl::WindowState *>(swapChainCtx);

   delete windowState;
   return BEGL_Success;
}

static BEGL_Error DispSurfaceGetInfo(void *context __attribute__((unused)),
   uint32_t target __attribute__((unused)),
   void *buffer,
   BEGL_BufferSettings *settings)
{
   if (settings && buffer)
   {
      *settings = static_cast<dbpl::WrappedBitmap*>(buffer)->GetCreateSettings();
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

typedef struct DBPL_DisplayInterface : BEGL_DisplayInterface
{
   DBPL_Display data;
} DBPL_DisplayInterface;

extern "C" BEGL_DisplayInterface *DBPL_CreateDisplayInterface(IDirectFB *dfb,
                                                              BEGL_HWInterface     *hwIface,
                                                              BEGL_MemoryInterface *memIface)
{
   std::unique_ptr<DBPL_DisplayInterface> disp(new DBPL_DisplayInterface());
   if (!disp)
      return NULL;

   disp->context = &disp->data;
   disp->BufferCreate = DispBufferCreate;
   disp->BufferDestroy = DispBufferDestroy;
   disp->BufferDequeue = DispBufferDequeue;
   disp->BufferQueue = DispBufferQueue;
   disp->BufferCancel = DispBufferCancel;
   disp->WindowPlatformStateCreate = DispWindowStateCreate;
   disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
   disp->SurfaceGetInfo = DispSurfaceGetInfo;

   disp->data.dfb = dfb;
   disp->data.memInterface = memIface;
   disp->data.hwInterface = hwIface;

   return disp.release();
}

extern "C" void DBPL_DestroyDisplayInterface(BEGL_DisplayInterface *p)
{
   auto disp = std::unique_ptr<DBPL_DisplayInterface>(static_cast<DBPL_DisplayInterface *>(p));
}

bool DBPL_BufferGetRequirements(DBPL_PlatformHandle handle,
                                BEGL_PixmapInfoEXT *bufferRequirements,
                                BEGL_BufferSettings * bufferConstrainedRequirements)
{
   auto data = static_cast<BEGL_DriverInterfaces*>(handle);

   if (data != NULL && data->displayCallbacks.BufferGetRequirements != NULL)
   {
      data->displayCallbacks.BufferGetRequirements(bufferRequirements, bufferConstrainedRequirements);
      return true;
   }

   return false;
}

void DBPL_GetDefaultPixmapInfoEXT(BEGL_PixmapInfoEXT *info)
{
   if (info != NULL)
   {
      memset(info, 0, sizeof(BEGL_PixmapInfoEXT));

      info->format = BEGL_BufferFormat_INVALID;
      info->magic = PIXMAP_INFO_MAGIC;
   }
}

bool DBPL_CreateCompatiblePixmapEXT(DBPL_PlatformHandle handle, void **pixmapHandle, IDirectFBSurface **surface,
   BEGL_PixmapInfoEXT *info)
{
   auto data = static_cast<BEGL_DriverInterfaces*>(handle);

   assert(info->magic == PIXMAP_INFO_MAGIC);

   if (!(data != NULL && data->displayCallbacks.PixmapCreateCompatiblePixmap != NULL))
      return false;

   if (surface == NULL)
      return false;

   BEGL_BufferHandle buffer = data->displayCallbacks.PixmapCreateCompatiblePixmap(info);
   if (buffer == NULL)
      return false;

   *pixmapHandle = static_cast<void*>(buffer);
   *surface = static_cast<dbpl::Bitmap*>(buffer)->GetSurface();
   return true;
}

bool DBPL_CreateCompatiblePixmap(DBPL_PlatformHandle handle, void **pixmapHandle, IDirectFBSurface **surface,
   BEGL_PixmapInfo *info)
{
   BEGL_PixmapInfoEXT   infoEXT;

   DBPL_GetDefaultPixmapInfoEXT(&infoEXT);

   infoEXT.width = info->width;
   infoEXT.height = info->height;
   infoEXT.format = info->format;

   return DBPL_CreateCompatiblePixmapEXT(handle, pixmapHandle, surface, &infoEXT);
}

void DBPL_DestroyCompatiblePixmap(DBPL_PlatformHandle handle, void *pixmapHandle)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != NULL &&
       data->displayInterface != NULL &&
       data->displayInterface->BufferDestroy != NULL)
   {
      BEGL_BufferDisplayState bufferState = {};
      bufferState.buffer = static_cast<BEGL_BufferHandle>(pixmapHandle);
      data->displayInterface->BufferDestroy(data->displayInterface->context, &bufferState);
   }
}
