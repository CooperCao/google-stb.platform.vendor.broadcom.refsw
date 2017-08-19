/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <memory.h>
#include <assert.h>
#include <semaphore.h>

#include "default_directfb.h"

#include "nexus_surface.h"
#include "nexus_memory.h"
#include "nexus_platform.h"

#include <bkni.h>

typedef struct
{
   IDirectFB               *dfb;
   BEGL_MemoryInterface    *memInterface;
} DBPL_Display;

typedef struct
{
   BEGL_BufferSettings     settings;
   IDirectFBSurface        *surface;
} DBPL_BufferData;

typedef struct
{
   pthread_mutex_t         mutex;

   /* keep everything in sync */
   pthread_cond_t          cond;
   bool                    inUse;
} DBPL_WindowState;

enum
{
   PIXMAP_INFO_MAGIC = 0x15EEB1A5
};

/*****************************************************************************
 * Display driver interface
 *****************************************************************************/

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
     {                                                                \
          int err = x;                                                \
          if (err != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
               DirectFBErrorFatal( #x, err );                         \
          }                                                           \
     }

static BEGL_Error DispBufferDisplay(void *context, BEGL_BufferDisplayState *state)
{
   DBPL_Display               *data = (DBPL_Display*)context;
   DBPL_BufferData *buffer = (DBPL_BufferData*)state->buffer;

   if (data)
   {
      DBPL_WindowState *windowState = (DBPL_WindowState *)state->windowState.platformState;

      DFBCHECK(buffer->surface->Unlock( buffer->surface ));

      /* call flip */
      DFBCHECK(buffer->surface->Flip( buffer->surface, NULL, DSFLIP_WAITFORSYNC ));

      /* cant be NULL in this implementation */
      assert(windowState);

      /* signal anything waiting on this that it can go */
      pthread_mutex_lock(&windowState->mutex);
      pthread_cond_broadcast(&windowState->cond);
      windowState->inUse = false;
      pthread_mutex_unlock(&windowState->mutex);

      return BEGL_Success;
   }

   return BEGL_Fail;
}

/* Flush pending displays until they are all done, then removes all buffers from display. Will block until complete. */
static BEGL_Error DispWindowUndisplay(void *context, BEGL_WindowState *windowState)
{
   DBPL_Display               *data = (DBPL_Display*)context;

   return BEGL_Success;
}

NEXUS_HeapHandle NXPL_MemHeap(BEGL_MemoryInterface *mem);

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
   DBPL_Display                  *data = (DBPL_Display*)context;
   DBPL_BufferData               *buffer = NULL;
   NEXUS_MemoryAllocationSettings memSettings;
   uint32_t                       bpp;
   DFBSurfaceDescription          desc;
   int                            pitch;
   NEXUS_MemoryStatus             memStatus;

   buffer = (DBPL_BufferData*)malloc(sizeof(DBPL_BufferData));
   memset(buffer, 0, sizeof(DBPL_BufferData));

   desc.flags = DSDESC_CAPS |
                DSDESC_WIDTH |
                DSDESC_HEIGHT |
                DSDESC_PIXELFORMAT;
   desc.caps = DSCAPS_GL;
   desc.width = settings->width;
   desc.height = settings->height;

   switch (settings->format)
   {
   case BEGL_BufferFormat_eA8B8G8R8 :
      desc.pixelformat = DSPF_ABGR;
      break;
   case BEGL_BufferFormat_eX8B8G8R8 :
      desc.pixelformat = DSPF_RGB32;
      break;
   case BEGL_BufferFormat_eR5G6B5 :
   case BEGL_BufferFormat_eR5G6B5_Texture :
      desc.pixelformat = DSPF_RGB16;
      break;
   case BEGL_BufferFormat_eYUV422_Texture :
      desc.pixelformat = DSPF_UYVY;
      break;
   case BEGL_BufferFormat_eVUY224_Texture :
      fprintf(stderr, "DirectFB does not currently support VUY244 surface formats which is required "
         "to support 3D graphics in big-endian mode\n");
      return NULL;
      break;
   case BEGL_BufferFormat_eA8B8G8R8_Texture :
      desc.pixelformat = DSPF_ABGR;
      break;
   case BEGL_BufferFormat_eR8G8B8A8_Texture :
      fprintf(stderr, "DirectFB does not currently support RGBA8888 surface formats which is required "
                     "to support 3D graphics in big-endian mode\n");
      return NULL;
      break;

   case BEGL_BufferFormat_eR8G8B8A8 :
   case BEGL_BufferFormat_eR8G8B8X8 :
      fprintf(stderr, "DirectFB does not currently support RGBA8888 surface formats which is required "
                      "to support 3D graphics in big-endian mode\n");
      return NULL;
      break;
   }

   DFBCHECK(data->dfb->CreateSurface( data->dfb, &desc, &buffer->surface ));

   /* These lock and unlocks are required --- DFB has some lazy surface creation */
   DFBCHECK(buffer->surface->Lock( buffer->surface, DSLF_READ, &settings->cachedAddr, &pitch));
   DFBCHECK(buffer->surface->Unlock( buffer->surface ));

   settings->physOffset = data->memInterface->ConvertCachedToPhysical(data->memInterface->context, settings->cachedAddr);
   buffer->settings = *settings;

   return (BEGL_BufferHandle)buffer;
}

static BEGL_BufferHandle DispBufferGet(void *context, BEGL_BufferSettings *settings)
{
   DBPL_Display                  *data = (DBPL_Display*)context;
   DBPL_BufferData               *buffer;

   buffer = (DBPL_BufferData*)malloc(sizeof(DBPL_BufferData));
   if (buffer)
   {
      DFBSurfacePixelFormat dfb_format;
      memset(buffer, 0, sizeof(DBPL_BufferData));

      /* window is the native window supplied during eglCreateWindowSurface */
      buffer->surface = (IDirectFBSurface*)settings->windowState.window;

      /* printf("DispBufferGet %d %d\n", settings->width, settings->height); */

      DFBCHECK(buffer->surface->GetPixelFormat ( buffer->surface, &dfb_format));

      if ((dfb_format != DSPF_ABGR) &&
          (dfb_format != DSPF_RGB16))
      {
         printf("==========================================================================\n");
         printf("Provided DFB window/surface is not a renderable type, aborting (%d)\n");
         printf("==========================================================================\n", dfb_format);
         assert(0);
      }

      /* structure copy -> grab the elements above (inputs) to the buffer structure */
      buffer->settings = *settings;

      /* printf("getting surface width = %d, height = %d, align = %d\n",
         buffer->settings.width, buffer->settings.height, buffer->settings.alignment); */
   }

   return (BEGL_BufferHandle)buffer;

error0:

   free(buffer);
   return NULL;
}

static BEGL_BufferHandle DispWindowStateCreate(void *context, BEGL_WindowHandle window)
{
   DBPL_WindowState *windowState;

   windowState = (DBPL_WindowState *)malloc(sizeof(DBPL_WindowState));

   if (windowState)
   {
      memset(windowState, 0, sizeof(DBPL_WindowState));

      /* create the mutex to stop the buffer being locked twice */
      pthread_mutex_init(&windowState->mutex, NULL);

      /* create the conditional for the buffer */
      pthread_cond_init(&windowState->cond, NULL);
   }

   return (BEGL_BufferHandle)windowState;
}

static BEGL_Error DispWindowStateDestroy(void *context, void *platformState)
{
   DBPL_WindowState *windowState = (DBPL_WindowState *)platformState;

   if (windowState)
   {
      pthread_mutex_destroy(&windowState->mutex);

      /* delete the condition */
      pthread_cond_destroy(&windowState->cond);
      free(windowState);

      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_Error DispBufferAccess(void *context, BEGL_BufferAccessState *bufferAccess)
{
   DBPL_Display               *data = (DBPL_Display*)context;

   if (data)
   {
      void * addr;
      int pitch;
      unsigned int res;
      DBPL_BufferData   *buffer = bufferAccess->buffer;
      DBPL_WindowState *windowState = (DBPL_WindowState *)bufferAccess->windowState.platformState;

      /* cant be NULL in this implementation */
      assert(windowState);

      pthread_mutex_lock(&windowState->mutex);
      if (windowState->inUse == true)
      {
         /* this releases the mutex and waits on the condition */
         pthread_cond_wait(&windowState->cond, &windowState->mutex);
         /* if we get here, we've satisfied the condition and got the mutex back */
      }
      windowState->inUse = true;
      pthread_mutex_unlock(&windowState->mutex);

      DFBCHECK(buffer->surface->Lock( buffer->surface, DSLF_WRITE, &addr, &pitch));

      /* update BEGL_BufferSettings with new information */
      buffer->settings.cachedAddr = addr;
      buffer->settings.pitchBytes = pitch;

      /* convert cached to physical */
      buffer->settings.physOffset = data->memInterface->ConvertCachedToPhysical(data->memInterface->context, buffer->settings.cachedAddr);

      /* get the size from the DFB surface */
      DFBCHECK(buffer->surface->GetSize ( buffer->surface, (int *)&buffer->settings.width, (int *)&buffer->settings.height));

      /* printf("locking surface %p pitch = %d, width = %d, height = %d, align = %d\n",
         addr, pitch, buffer->settings.width, buffer->settings.height, buffer->settings.alignment); */

      return BEGL_Success;
   }

   return BEGL_Fail;
}


/* Destroy a buffer previously created with BufferCreate */
static BEGL_Error DispBufferDestroy(void *context, BEGL_BufferDisplayState *bufferState)
{
   DBPL_BufferData *buffer = (DBPL_BufferData*)bufferState->buffer;

   if (buffer != NULL)
   {
      if ((buffer->settings.usage == BEGL_BufferUsage_ePixmap) && (buffer->surface))
      {
         DFBCHECK(buffer->surface->Release( buffer->surface ));
      }

      memset(buffer, 0, sizeof(DBPL_BufferData));
      free(buffer);
   }

   return BEGL_Success;
}



/* Get information about a created window buffer */
static BEGL_Error DispBufferGetCreateSettings(void *context, BEGL_BufferHandle bufHandle, BEGL_BufferSettings *settings)
{
   DBPL_BufferData *buffer = (DBPL_BufferData*)bufHandle;

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
   IDirectFBSurface *dst = (IDirectFBSurface*)window;
   BEGL_Error       ret  = BEGL_Fail;

   if (info != NULL)
   {
      /* Make sure we return something sensible */
      info->width  = 0;
      info->height = 0;
      info->swapchain_count = 0;

      if (dst != NULL)
      {
         DFBSurfaceCapabilities ret_caps;

         DFBCHECK(dst->GetSize( dst, &info->width, &info->height));

         DFBCHECK(dst->GetCapabilities( dst, &ret_caps ));
         if (ret_caps & DSCAPS_DOUBLE)
            info->swapchain_count = 2;
         else if (ret_caps & DSCAPS_TRIPLE)
            info->swapchain_count = 3;
         else
            info->swapchain_count = 1;

         ret = BEGL_Success;
      }
   }

   /* printf("[%s] - width = %d, height = %d, swapchain_count = %d, ret_format = %d\n",
      __FUNCTION__, info->width, info->height, info->swapchain_count, info->format); */

   return ret;
}

BEGL_DisplayInterface *DBPL_CreateDisplayInterface(IDirectFB *dfb,
                                                   BEGL_MemoryInterface *memIface,
                                                   BEGL_DisplayCallbacks *displayCallbacks)
{
   DBPL_Display *data;
   BEGL_DisplayInterface *disp = (BEGL_DisplayInterface*)malloc(sizeof(BEGL_DisplayInterface));

   if (disp != NULL)
   {
      data = (DBPL_Display *)malloc(sizeof(DBPL_Display));
      memset(disp, 0, sizeof(BEGL_DisplayInterface));

      if (data != NULL)
      {
         memset(data, 0, sizeof(DBPL_Display));
         disp->context = (void*)data;
         disp->BufferDisplay = DispBufferDisplay;
         disp->BufferCreate = DispBufferCreate;
         disp->BufferDestroy = DispBufferDestroy;
         disp->BufferGetCreateSettings = DispBufferGetCreateSettings;
         disp->BufferGet = DispBufferGet;
         disp->BufferAccess = DispBufferAccess;
         disp->WindowPlatformStateCreate = DispWindowStateCreate;
         disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
         disp->WindowGetInfo = DispWindowGetInfo;
         disp->WindowUndisplay = DispWindowUndisplay;

         /* stash the dfb */
         data->dfb = dfb;

         /* Store the memory interface */
         data->memInterface = memIface;
      }
      else
      {
         free(disp);
         return NULL;
      }
   }
   return disp;
}

void DBPL_DestroyDisplayInterface(BEGL_DisplayInterface *disp)
{
   if (disp)
   {
      DBPL_Display *data = (DBPL_Display *)disp->context;
      if (data)
         free(data);

      free(disp);
   }
}

bool DBPL_BufferGetRequirements(DBPL_PlatformHandle handle,
                                BEGL_PixmapInfoEXT *bufferRequirements,
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
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   assert(info->magic == PIXMAP_INFO_MAGIC);

   if (data != NULL && data->displayCallbacks.PixmapCreateCompatiblePixmap != NULL)
   {
      BEGL_BufferHandle buffer = data->displayCallbacks.PixmapCreateCompatiblePixmap(info);
      if (buffer != NULL)
      {
         *pixmapHandle = (void*)buffer;
         *surface = ((DBPL_BufferData*)buffer)->surface;
         return true;
      }
   }

   return false;
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
      BEGL_BufferDisplayState bufferState;
      memset(&bufferState, 0, sizeof(BEGL_BufferDisplayState));
      bufferState.buffer = (BEGL_BufferHandle)pixmapHandle;

      data->displayInterface->BufferDestroy(data->displayInterface->context, &bufferState);
   }
}
