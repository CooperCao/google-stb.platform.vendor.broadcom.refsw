/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#ifndef EGL_IMAGE_H
#define EGL_IMAGE_H
#include "interface/khronos/include/EGL/egl.h"
#include "middleware/khronos/egl/egl_types.h"
#include "middleware/khronos/common/khrn_image.h"

typedef void  (*egl_image_destroy)(EGL_IMAGE_T *egl_image);
typedef KHRN_IMAGE_T* (*egl_image_get)(EGL_IMAGE_T *egl_image);

struct egl_image
{
   KHRN_IMAGE_T         *image;
   volatile int         ref_count;
   egl_image_destroy    destroy;
   egl_image_get        get;
};

/* calloc() followed by egl_image_init() */
extern EGL_IMAGE_T* egl_image_create(KHRN_IMAGE_T *image);

extern void egl_image_init(EGL_IMAGE_T* egl_image, KHRN_IMAGE_T *image,
   egl_image_destroy destroy, egl_image_get get);

/* increase ref count of egl_image */
extern void egl_image_refinc(EGL_IMAGE_T *egl_image);

/* decrease ref count on egl_image */
extern void egl_image_refdec(EGL_IMAGE_T *egl_image);

/* Get the KHRN_IMAGE_T * corresponding to egl_image's data */
extern KHRN_IMAGE_T *egl_image_get_image(EGL_IMAGE_T *egl_image);

#endif /* EGL_IMAGE_H */
