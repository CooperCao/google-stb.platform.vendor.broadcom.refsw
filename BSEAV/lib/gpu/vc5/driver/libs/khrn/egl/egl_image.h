/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_IMAGE_H
#define EGL_IMAGE_H
#include <EGL/egl.h>
#include "egl_types.h"
#include "../common/khrn_image.h"

typedef void  (*egl_image_destroy)(EGL_IMAGE_T *egl_image);
typedef khrn_image* (*egl_image_get)(EGL_IMAGE_T *egl_image);

struct egl_image
{
   khrn_image          *image;
   unsigned             num_mip_levels;
   volatile int         ref_count;
   egl_image_destroy    destroy;
   egl_image_get        get;
};

/* calloc() followed by egl_image_init() */
extern EGL_IMAGE_T* egl_image_create(khrn_image *image, unsigned num_mip_levels);

extern void egl_image_init(EGL_IMAGE_T* egl_image, khrn_image *image,
   unsigned num_mip_levels, egl_image_destroy destroy, egl_image_get get);

/* increase ref count of egl_image */
extern void egl_image_refinc(EGL_IMAGE_T *egl_image);

/* decrease ref count on egl_image */
extern void egl_image_refdec(EGL_IMAGE_T *egl_image);

/* Get the khrn_image * corresponding to egl_image's data */
extern khrn_image *egl_image_get_image(EGL_IMAGE_T *egl_image);

/* Get the number of miplevels in the egl_image's data */
extern unsigned egl_image_get_num_mip_levels(EGL_IMAGE_T *egl_image);
#endif /* EGL_IMAGE_H */
