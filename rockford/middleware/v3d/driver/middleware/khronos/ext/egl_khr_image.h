/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Server-side implementation of the EGLImage extensions for EGL:
   EGL_KHR_image
   EGL_KHR_vg_parent_image
   EGL_KHR_gl_texture_2D_image
   EGL_KHR_gl_texture_cubemap_image
=============================================================================*/

#ifndef _EGL_KHR_IMAGE_H_
#define _EGL_KHR_IMAGE_H_

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"

#if KHRN_HW_TEX_SIZE_MAX <= 2048
typedef uint32_t tile_bits_t;
#elif KHRN_HW_TEX_SIZE_MAX <= 4096
typedef uint64_t tile_bits_t;
#else
#error Max texture size too big for this code
#endif

/* A structure with 1-bit per tile */
typedef struct {
   tile_bits_t  m_rowBits[KHRN_HW_TEX_SIZE_MAX / 64];
} EGL_IMAGE_TILE_DIRTY_BITS_T;

typedef struct EGL_IMAGE {
   uint64_t pid;

   MEM_HANDLE_T mh_image;

   /* used for platforms which may have reference counting on mapping */
   uint32_t buffer;

   /* BCG_EGLIMAGE_CONVERTER stuff */

   /* Shadow buffer actually holds the original RSO format
      image when we've done a t-format conversion on-the-fly. */
   MEM_HANDLE_T mh_shadow_image;

   /* Nominal frame count on which the conversion was done. Used to decide if the image is out of date */
   uint32_t     convertedFrame;

   /* True if the lockSemaphore, missedConvFlag and dirtyBitsMutex are allocated.  In error case the map
      removal of the eglimage removes the objects which have not been created */
   bool         update_control_initialized;

   /* True if explicit update control has been turned on for this image with the
      BRCM_image_update_control extension. */
   bool         explicit_updates;

   /* One dirty bit per tile. Primarily used for the BRCM_image_update_control extension. */
   volatile EGL_IMAGE_TILE_DIRTY_BITS_T dirtyBits;

   /* EGLimage locking mutex used to implement the lock/unlock functionality of the extension */
   VCOS_SEMAPHORE_T lockSemaphore;

   /* Flag used to detect if we had to cancel a conversion due to the image being locked */
   VCOS_SEMAPHORE_T  missedConvFlag;

   /* Mutex for dirty bits */
   VCOS_MUTEX_T  dirtyBitsMutex;

   /* Queued and completed conversion counts */
   volatile uint32_t queued;
   volatile uint32_t completed;

   /* Set during image lock to force a (potentially 0 region) conversion. This in turn will force a
      genuine update on the next application image lock, thus avoiding the 'every-other-frame-update' scenario. */
   volatile bool forceConversion;

} EGL_IMAGE_T;

extern void egl_image_term(void *v, uint32_t size);

#define egl_image_set_tile_dirty_bit(img, x, y)  (img->dirtyBits.m_rowBits[(y)] |= (1 << (x)))
#define egl_image_clr_tile_dirty_bit(img, x, y)  (img->dirtyBits.m_rowBits[(y)] &= ~(1 << (x)))
#define egl_image_clr_all_tile_dirty_bits(img)   (memset((void*)&((img)->dirtyBits), 0x00, sizeof(EGL_IMAGE_TILE_DIRTY_BITS_T)))
#define egl_image_set_all_tile_dirty_bits(img)   (memset((void*)&((img)->dirtyBits), 0xFF, sizeof(EGL_IMAGE_TILE_DIRTY_BITS_T)))

extern bool egl_image_any_tile_dirty(EGL_IMAGE_T *img);
extern bool egl_image_is_tile_dirty(EGL_IMAGE_T *img, uint32_t x, uint32_t y);
extern uint32_t egl_image_num_dirty_tiles(EGL_IMAGE_T *img);

#endif