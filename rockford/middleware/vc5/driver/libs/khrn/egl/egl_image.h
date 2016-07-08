/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#ifndef EGL_IMAGE_H
#define EGL_IMAGE_H
#include <EGL/egl.h>
#include "egl_types.h"
#include "../common/khrn_image.h"

struct egl_image
{
   KHRN_IMAGE_T         *image;
   volatile int         ref_count;
};

extern EGL_IMAGE_T* egl_image_create(KHRN_IMAGE_T *image);

/* increase ref count of egl_image */
extern void egl_image_refinc(EGL_IMAGE_T *egl_image);

/* decrease ref count on egl_image */
extern void egl_image_refdec(EGL_IMAGE_T *egl_image);

/* Get the KHRN_IMAGE_T * corresponding to egl_image's data */
extern KHRN_IMAGE_T *egl_image_get_image(EGL_IMAGE_T *egl_image);

#endif /* EGL_IMAGE_H */
