/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BEGL_DISPLAYPLATFORM_H__
#define __BEGL_DISPLAYPLATFORM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <EGL/begl_memplatform.h>

#include <stdint.h>
#include <stdlib.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

/*****************************************************************************
 *****************************************************************************
 *****                       DISPLAY INTERFACE                           *****
 *****************************************************************************
 *****************************************************************************/

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

typedef enum
{
   BEGL_WindowInfoWidth = 1,
   BEGL_WindowInfoHeight = 2,
   BEGL_WindowInfoFormat = 4,
   BEGL_WindowInfoSwapChainCount = 8,
} BEGL_WindowInfoFlags;

typedef enum
{
   /* These formats are render target formats */
   BEGL_BufferFormat_eA8B8G8R8,
   BEGL_BufferFormat_eR8G8B8A8,
   BEGL_BufferFormat_eX8B8G8R8,
   BEGL_BufferFormat_eR8G8B8X8,
   BEGL_BufferFormat_eR5G6B5,

   BEGL_BufferFormat_eR4G4B4A4,
   BEGL_BufferFormat_eA4B4G4R4,
   BEGL_BufferFormat_eR4G4B4X4,
   BEGL_BufferFormat_eX4B4G4R4,

   BEGL_BufferFormat_eR5G5B5A1,
   BEGL_BufferFormat_eA1B5G5R5,
   BEGL_BufferFormat_eR5G5B5X1,
   BEGL_BufferFormat_eX1B5G5R5,

   /* non renderable input formats */
   BEGL_BufferFormat_eYV12,                  /* 3 planes layed out in Google format */
   BEGL_BufferFormat_eYUV422,                /* Single plane YUYV */

   /* renderable, but can only be used by the display and not re-read */
   BEGL_BufferFormat_eBSTC,

   /* A format which can be bound directly to the texture target without
    * requiring internal format conversions by the OpenGL driver.
    *
    * This is available only on Nexus based platform implementations and is
    * mapped to NEXUS_PixelFormat_eUIF_R8_G8_B8_A8, which has been coded in
    * Nexus and Magnum to produce memory layouts directly compatible
    * with V3D, the standard M2MC and mipmap M2MC hardware on supported SoCs.
    *
    * - Pixmaps of this type may be created with or without a mipchain.
    * - Only level 0 will be renderable by V3D
    * - Only pixmaps without a mipchain will be readable by the standard M2MC,
    *   i.e. via the Nexus 2D graphics blit APIs or Nexus surface compositor
    * - pixmaps with a mipchain can have the mipchain populated using the
    *   mipmap specific variant of the M2MC on SoCs where that is available
    * - pixmaps with a mipchain can not be read by any of the M2MCs
    * - Not all level 0 image sizes are supported to ensure compatibility
    *   between the two sides, this currently means the minimum width
    *   and height are both limited to 64 pixels.
    */
   BEGL_BufferFormat_eTILED,

   /* Can be used to return back an invalid format */
   BEGL_BufferFormat_INVALID
} BEGL_BufferFormat;

typedef struct
{
   uint32_t            width;                 /* Visible width of window in pixels */
   uint32_t            height;                /* Visible height of window in pixels */
   uint32_t            swapchain_count;       /* Number of buffers in the swap chain, or 0 to take defaults from egl */
} BEGL_WindowInfo;

typedef struct BEGL_PixmapInfo
{
   uint32_t            width;                 /* Visible width of pixmap in pixels */
   uint32_t            height;                /* Visible height of pixmap in pixels */
   BEGL_BufferFormat   format;
} BEGL_PixmapInfo;

typedef struct BEGL_PixmapInfoEXT
{
   uint32_t            magic;
   uint32_t            width;                 /* Visible width of pixmap in pixels */
   uint32_t            height;                /* Visible height of pixmap in pixels */
   uint32_t            miplevels;             /* Number of miplevels required, eTILED format only */
                                              /* must be 1 (default) for all other buffer formats */
   BEGL_BufferFormat   format;
   bool                secure;                /* Create pixmap in secure heap */
} BEGL_PixmapInfoEXT;

typedef struct
{
   uint32_t            width;                 /* Visible width of surface in pixels                     */
   uint32_t            height;                /* Visible height of surface in pixels                    */
   uint32_t            pitchBytes;            /* Bytes per row, or HW specific pitch for eTILED format  */
   uint64_t            physicalOffset;        /* Physical address                                       */
   void                *cachedAddr;           /* Cached address mapping                                 */
   uint32_t            byteSize;              /* Size of buffer in bytes                                */
   uint32_t            miplevels;             /* Number of miplevels contained in the surface,          */
                                              /* this will be 1 for formats that do not support mipmaps */
   BEGL_BufferFormat   format;
} BEGL_SurfaceInfo;

typedef struct BEGL_DisplayInterface
{
   /* Context pointer - opaque to the 3d driver code, but passed out in all function pointer calls.
    * Prevents the client code needing to perform context lookups. */
   void *context;

   bool (*Init)(void *context);
   void (*Terminate)(void *context);

   /* Called to determine current size of the window referenced by the opaque window handle.
    * Also fills in the number of pre-allocated swap-chain buffers, which must be > 0.
    * Set to the number of buffers in your pre-allocated chain. See BufferGet().
    * This is needed by EGL in order to know the size of a native 'window'. */
   BEGL_Error (*WindowGetInfo)(void *context, void *opaqueNativeWindow, BEGL_WindowInfoFlags flags, BEGL_WindowInfo *info);

   /* Called to get access to an underlying native surface.
    * Can be NULL if creating EGL images from native buffers isn't required.
    */
   BEGL_Error (*GetNativeSurface)(void *context, uint32_t eglTarget, void *eglClientBuffer, void **opaqueNativeSurface);

   BEGL_Error (*SurfaceGetInfo)(void *context, void *opaqueNativeSurface, BEGL_SurfaceInfo *info);
   BEGL_Error (*SurfaceChangeRefCount)(void *context, void *opaqueNativeSurface, BEGL_RefCountMode inOrDec);

   /* Return the next render buffer surface in the swap chain (in opaqueNativeSurface)
    * with a fence to wait on before accesing the buffer surface.
    * A surface obtained this way must be returned to the display system with a call to
    * DisplaySurface or CancelSurface.
    * All these 3 functions must be implemented;
    */
   BEGL_Error (*GetNextSurface)(void *context, void *opaqueNativeWindow, BEGL_BufferFormat format,
                               BEGL_BufferFormat *actualFormat, void **opaqueNativeSurface, bool secure, int *fence);

   BEGL_Error (*DisplaySurface)(void *context, void *nativeWindow, void *nativeSurface, int fence, int interval);

   BEGL_Error (*CancelSurface)(void *context, void *nativeWindow, void *nativeSurface, int fence);

   bool  (*PlatformSupported)(void *context, uint32_t platform);

   bool  (*SetDefaultDisplay)(void *context, void *display);

   void *(*GetDefaultDisplay)(void *context);

   void *(*WindowPlatformStateCreate)(void *context, void *nativeWindow);

   BEGL_Error (*WindowPlatformStateDestroy)(void *context, void *windowState);

   BEGL_Error (*GetNativeFormat)(void *context, BEGL_BufferFormat format, uint32_t *nativeformat);

   const char *(*GetClientExtensions)(void *context);
   const char *(*GetDisplayExtensions)(void *context);

   bool (*BindWaylandDisplay)(void *context, void *egl_display, void *wl_display);

   bool (*UnbindWaylandDisplay)(void *context, void *egl_display, void *wl_display);

   bool (*QueryBuffer)(void *context, void *display, void* buffer, int32_t attribute, int32_t *value);

} BEGL_DisplayInterface;

extern void BEGL_RegisterDisplayInterface(BEGL_DisplayInterface *iface);
extern void BEGL_PlatformAboutToShutdown(void);

typedef void (*PFN_BEGL_RegisterDisplayInterface)(BEGL_DisplayInterface *);
typedef void (*PFN_BEGL_PlatformAboutToShutdown)(void);

#ifdef __cplusplus
}
#endif

#endif /* __BEGL_DISPLAYPLATFORM_H__ */
