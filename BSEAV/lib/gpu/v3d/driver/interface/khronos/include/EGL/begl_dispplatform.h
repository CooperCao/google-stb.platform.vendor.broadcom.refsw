/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BEGL_DEFAULT_PLATFORM 0

#define BEGL_SWAPCHAIN_BUFFER 0
#define BEGL_PIXMAP_BUFFER 1

typedef enum
{
   /* These formats are render target formats, but cannot be textured from */
   BEGL_BufferFormat_eA8B8G8R8,
   BEGL_BufferFormat_eR8G8B8A8,           /* For big-endian platforms */
   BEGL_BufferFormat_eX8B8G8R8,
   BEGL_BufferFormat_eR8G8B8X8,           /* For big-endian platforms */
   BEGL_BufferFormat_eR5G6B5,
   BEGL_BufferFormat_eYUV422,
   BEGL_BufferFormat_eYV12,

   BEGL_BufferFormat_INVALID
} BEGL_BufferFormat;

typedef enum
{
   BEGL_Success = 0,
   BEGL_Fail
} BEGL_Error;

typedef void  *BEGL_SwapchainBuffer;  /* Opaque as far as driver is concerned. Only the platform knows the actual type. */
typedef void  *BEGL_NativeBuffer;     /* Opaque as far as driver is concerned. Only the platform knows the actual type. */
typedef void  *BEGL_WindowHandle;     /* Opaque 'window' handle (required by EGL) */

typedef struct
{
   /* IN THE CASE OF MMA, EVERYTHING IS LOCKED/UNLOCKED BY THE PLATFORM LAYER */
   void               *cachedAddr;    /* Cached address of buffer memory. Set to NULL during create to tell app to allocate. */
   uint32_t            physOffset;    /* Physical memory offset of buffer memory. */

   uint32_t            width;         /* Visible width of buffer in pixels */
   uint32_t            height;        /* Visible height of buffer in pixels */
   uint32_t            pitchBytes;    /* Actual bytes per row of the buffer, including padding */
   uint32_t            totalByteSize; /* Actual bytes allocated for the entire image */
   uint32_t            secure;        /* used to signal whether the buffer is allocated in the secure heap or not */
   BEGL_BufferFormat   format;        /* Pixel format of the buffer */
} BEGL_BufferSettings;

typedef enum
{
   BEGL_WindowInfoWidth = 1,
   BEGL_WindowInfoHeight = 2,
   BEGL_WindowInfoFormat = 4,
   BEGL_WindowInfoSwapChainCount = 8,
   BEGL_WindowInfoRefreshRate = 16
} BEGL_WindowInfoFlags;

typedef struct
{
   uint32_t            width;             /* Visible width of window in pixels */
   uint32_t            height;            /* Visible height of window in pixels */
   uint32_t            swapchain_count;   /* Number of buffers in the swap chain, or 0 to take defaults from egl */
   BEGL_BufferFormat   format;            /* Pixel format of the buffer */
   uint32_t            refreshRateMilliHertz;       /* Refresh rate of display in hertz. Only used when swapInterval >= 2. */
} BEGL_WindowInfo;

typedef struct
{
   uint32_t            width;         /* Visible width of pixmap in pixels */
   uint32_t            height;        /* Visible height of pixmap in pixels */
   BEGL_BufferFormat   format;        /* Pixel format of the pixmap */
} BEGL_PixmapInfo;

#define BEGL_PIXMAPINFOEXT
typedef struct
{
   uint32_t            width;         /* Visible width of pixmap in pixels */
   uint32_t            height;        /* Visible height of pixmap in pixels */
   uint32_t            secure;        /* used to signal whether the buffer is allocated from secure heap or not */
   BEGL_BufferFormat   format;        /* Pixel format of the pixmap */
   uint32_t            magic;
} BEGL_PixmapInfoEXT;

typedef struct BEGL_DisplayInterface
{
   /* Context pointer - opaque to the 3d driver code, but passed out in all function pointer calls.
    * Prevents the client code needing to perform context lookups. */
   void *context;

   /* Request a buffer from a pre-allocated swap chain.  The swap chain must contain the correct number of
    * buffers to match the swap chain length.
    * Swap chains are always retrieved using this method, and are never created directly by the driver.
    * This is different to earlier versions of the driver.
    */
   BEGL_SwapchainBuffer (*BufferDequeue)(void *context, void *platformState,
         BEGL_BufferFormat format, int *fd);

   BEGL_Error (*BufferQueue)(void *context, void *platformState, BEGL_SwapchainBuffer buffer, int swapInterval, int fd);

   BEGL_Error (*BufferCancel)(void *context, void *platformState, BEGL_SwapchainBuffer buffer, int fd);

   /* Called prior to a swapchain being created for the given window, if you need to store any global state (per window/swapchain)
    * then do it in this structure. You can allocate a structure of your own creation which will be associated with the window/swapchain. */
   void * (*WindowPlatformStateCreate)(void *context, BEGL_WindowHandle window, bool secure);

   /* Called when a swapchain is destroyed */
   BEGL_Error (*WindowPlatformStateDestroy)(void *context, void *platformState);

   /* Return true if display requires default orientation (bottom up rasterization) */
   BEGL_Error (*DefaultOrientation)(void *context);

   /* Get a new reference to the underlying native buffer.
    *
    * For swapchain buffers the target is set to BEGL_SWAPCHAIN_BUFFER and
    * buffer is the BEGL_SwapchainBuffer obtained from BufferDequeue().
    * The plane is always 0.
    *
    * For pixmaps the target is set to BEGL_PIXMAP_BUFFER and buffer
    * is the platform-specific EGLNativePixmapType.
    *
    * For platform-specific EGL client buffers target and buffer parameters
    * are those of the corresponding eglCreateImage() call.
    *
    * On success it returns a new, non-NULL reference to the native buffer
    * and fills-in BEGL_BufferSettings structure. The returned reference must
    * remain valid until it's released by calling ReleaseNativeBuffer().
    * The destruction of the eglObject must not free or invalidate the native
    * buffer unless it was already released by the driver.
    */
   BEGL_NativeBuffer (*AcquireNativeBuffer)(void *context, uint32_t target,
         void *eglObject, BEGL_BufferSettings *outSettings);

   /* Release a reference obtained from AcquireNativeBuffer().
    *
    * After all references are released the buffer may be freed.
    */
   BEGL_Error (*ReleaseNativeBuffer)(void *context, uint32_t target, BEGL_NativeBuffer buffer);

   /* used for EGL_NATIVE_VISUAL_ID, returns a platform dependent visual ID from the config */
   BEGL_Error (*GetNativeFormat)(void *context, BEGL_BufferFormat bufferFormat, uint32_t *outNativeFormat);

   bool (*BindWaylandDisplay)(void *context, void *egl_display, void *wl_display);
   bool (*UnbindWaylandDisplay)(void *context, void *egl_display, void *wl_display);
   bool (*QueryBuffer)(void *context, void *display, void* buffer, int32_t attribute, int32_t *value);

} BEGL_DisplayInterface;

#ifdef __cplusplus
}
#endif
