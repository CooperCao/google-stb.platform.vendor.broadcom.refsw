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
#define BEGL_DEFAULT_BUFFER 0

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

   BEGL_BufferFormat_eA8B8G8R8_sRGB_PRE,
   BEGL_BufferFormat_eR8G8B8A8_sRGB_PRE,  /* For big-endian platforms */
   BEGL_BufferFormat_eX8B8G8R8_sRGB_PRE,
   BEGL_BufferFormat_eR8G8B8X8_sRGB_PRE,  /* For big-endian platforms */
   BEGL_BufferFormat_eR5G6B5_sRGB_PRE,

   BEGL_BufferFormat_eA8B8G8R8_LIN_NON,
   BEGL_BufferFormat_eR8G8B8A8_LIN_NON,   /* For big-endian platforms */
   BEGL_BufferFormat_eX8B8G8R8_LIN_NON,
   BEGL_BufferFormat_eR8G8B8X8_LIN_NON,   /* For big-endian platforms */
   BEGL_BufferFormat_eR5G6B5_LIN_NON,

   BEGL_BufferFormat_eA8B8G8R8_LIN_PRE,
   BEGL_BufferFormat_eR8G8B8A8_LIN_PRE,   /* For big-endian platforms */
   BEGL_BufferFormat_eX8B8G8R8_LIN_PRE,
   BEGL_BufferFormat_eR8G8B8X8_LIN_PRE,   /* For big-endian platforms */
   BEGL_BufferFormat_eR5G6B5_LIN_PRE,

   BEGL_BufferFormat_INVALID
} BEGL_BufferFormat;

typedef enum
{
   BEGL_Success = 0,
   BEGL_Fail
} BEGL_Error;

typedef enum
{
   BEGL_Increment = 0,
   BEGL_Decrement
} BEGL_RefCountMode;

typedef void  *BEGL_BufferHandle;     /* Opaque as far as driver is concerned. Only the app/lib knows the actual type. */
typedef void  *BEGL_DisplayHandle;    /* Opaque 'display' handle (required by EGL) */
typedef void  *BEGL_WindowHandle;     /* Opaque 'window' handle (required by EGL) */

typedef struct
{
   void                *platformState;/* abstract state created by the platform layer */
   BEGL_WindowHandle   window;        /* The native window handle */
} BEGL_WindowState;

typedef struct
{
   /* IN THE CASE OF MMA, EVERYTHING IS LOCKED/UNLOCKED BY THE PLATFORM LAYER */
   void               *cachedAddr;    /* Cached address of buffer memory. Set to NULL during create to tell app to allocate. */
   uint32_t            physOffset;    /* Physical memory offset of buffer memory. */

   uint32_t            width;         /* Visible width of buffer in pixels */
   uint32_t            height;        /* Visible height of buffer in pixels */
   uint32_t            pitchBytes;    /* Actual bytes per row of the buffer, including padding */
   uint32_t            totalByteSize; /* Actual bytes allocated for the entire image */
   uint32_t            alignment;     /* Buffer alignment specified as a power of 2. 1 = 2 bytes, 2 = 4 bytes etc. */
   uint32_t            secure;        /* used to signal whether the buffer is allocated in the secure heap or not */
   BEGL_BufferFormat   format;        /* Pixel format of the buffer */
   BEGL_WindowState    windowState;   /* State relating to the window to which this buffer is related */
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
   uint32_t            openvg;        /* used to signal whether the buffer is used by VG or not */
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

typedef struct
{
   BEGL_BufferHandle   buffer;         /* The buffer to be/having been displayed */
   BEGL_WindowState    windowState;    /* State for this buffer's window */
   bool                waitVSync;      /* Should we wait for a vSync after displaying the buffer? */
} BEGL_BufferDisplayState;

typedef struct
{
   /* Context pointer - opaque to the 3d driver code, but passed out in all function pointer calls.
    * Prevents the client code needing to perform context lookups. */
   void *context;

   /* Request creation of an appropriate buffer (for pixmaps, FBOs or pbuffers, but NOT swap-chain buffers).
    * settings->totalByteSize is the size of the memory that the driver needs.
    * We could have just requested a block of memory using the memory interface, but by having the platform layer create a 'buffer'
    * it can actually create whatever type it desires directly, and then only have to deal with that type. For example, in a Nexus
    * platform layer, this function might be implemented to create a NEXUS_Surface (with the correct memory constraints of course).
    */
   BEGL_BufferHandle (*BufferCreate)(void *context, const BEGL_PixmapInfoEXT *bufferRequirements);

   /* Destroy a buffer previously created with BufferCreate */
   BEGL_Error (*BufferDestroy)(void *context, BEGL_BufferDisplayState *bufferState);

   /* Request a buffer from a pre-allocated swap chain.  The swap chain must contain the correct number of
    * buffers to match the swap chain length.
    * Swap chains are always retrieved using this method, and are never created directly by the driver.
    * This is different to earlier versions of the driver.
    */
   BEGL_BufferHandle (*BufferDequeue)(void *context, void *platformState, BEGL_BufferFormat format, int *fd);

   BEGL_Error (*BufferQueue)(void *context, void *platformState, BEGL_BufferHandle buffer, int swapInterval, int fd);

   BEGL_Error (*BufferCancel)(void *context, void *platformState, BEGL_BufferHandle buffer, int fd);

   /* Called prior to a swapchain being created for the given window, if you need to store any global state (per window/swapchain)
    * then do it in this structure. You can allocate a structure of your own creation which will be associated with the window/swapchain. */
   void * (*WindowPlatformStateCreate)(void *context, BEGL_WindowHandle window, bool secure);

   /* Called when a swapchain is destroyed */
   BEGL_Error (*WindowPlatformStateDestroy)(void *context, void *platformState);

   /* Return true if display requires default orientation (bottom up rasterization) */
   BEGL_Error (*DefaultOrientation)(void *context);

   /* Called to get access to an underlying native buffer. */
   BEGL_Error (*GetNativeBuffer)(void *context, uint32_t eglTarget, void *eglClientBuffer, void **buffer);

   /*
    * Used to decode a native format when you have an eglImage provided by the system.
    * For buffers obtained via BufferCreate() or BufferDeque() target is set to
    * BEGL_DEFAULT_BUFFER and buffer is BEGL_BufferHandle.
    * For platform-specific EGL client buffers target and buffer parameters
    * are those of the corresponding eglCreateImage() call. The target is passed
    * down directly, buffer is obtained from GetNativeBuffer(), if available,
    * otherwise passed down directly.
    */
   BEGL_Error (*SurfaceGetInfo)(void *context, uint32_t target, void *buffer, BEGL_BufferSettings *outSettings);

   /* used for EGL_NATIVE_VISUAL_ID, returns a platform dependent visual ID from the config */
   BEGL_Error (*GetNativeFormat)(void *context, BEGL_BufferFormat bufferFormat, uint32_t *outNativeFormat);

   /* called when eglGetDisplay() is called.  Used in cases such as X11, so the X display can be passed into the
      driver.  Could also be used in NEXUS to remove the need for an app to explicitly call it's platform
      registration */
   BEGL_Error (*SetDisplay)(void *context, uint32_t platform, void *nativeDisplay);

   /* some platforms require resources to be locked/unlocked on taking references */
   BEGL_Error (*SurfaceChangeRefCount)(void *context, uint32_t target, void *buffer, BEGL_RefCountMode incOrDec);

} BEGL_DisplayInterface;

/* The client application, or default platform library must register valid versions of each
   of these interfaces before any EGL or GL functions are invoked, using the following functions
   provided by the 3D driver.
*/
typedef struct
{
   /* Called by app/lib to create a pixmap buffer with the correct alignment/size constraints for EGL to use.
    * This will call back into the application's BufferCreate() function to actually allocate the buffer, after
    * calculating the appropriate alignment/size constraints.
    *
    * There should be no real need to use pixmap rendering in EGL, since the swap chain is exposed via this API anyway.
    * However, we must still keep pixmap rendering functional, and thus need this API call.
    *
    * The buffer should be destroyed using BEGL_DisplayInterface->BufferDestroy() */
   BEGL_BufferHandle (*PixmapCreateCompatiblePixmap)(BEGL_PixmapInfoEXT *pixmapInfo);

   /* Function to return the requirements of a given buffer size, so the application can make its own swap
    * chain and provide it back to GL */
   void (*BufferGetRequirements)(const BEGL_PixmapInfoEXT *bufferRequirements, BEGL_BufferSettings *bufferConstrainedRequirements);

} BEGL_DisplayCallbacks;

#ifdef __cplusplus
}
#endif
