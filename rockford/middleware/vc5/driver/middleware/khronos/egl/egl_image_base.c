/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "vcos.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "middleware/khronos/egl/egl_image_base.h"

void egl_image_base_init(EGL_IMAGE_T *egl_image, KHRN_IMAGE_T *image)
{
   /*
    * OK to cast away this const here (and and in try_delete) because this is
    * where we initialize egl_image->image.
    */
   KHRN_IMAGE_T **p = (KHRN_IMAGE_T **) &egl_image->image;
   KHRN_MEM_ASSIGN(*p, image);

   assert(egl_image->ref_count > 0);

   /*
    * If we didn't get the KHRN_IMAGE_T * now, require that a function has
    * been provided to get it later.
    */
   if (image == NULL)
      assert(egl_image->fns && egl_image->fns->create_image);
}

void egl_image_base_destroy(EGL_IMAGE_T *egl_image)
{
   KHRN_IMAGE_T **p = (KHRN_IMAGE_T **) &egl_image->image;
   KHRN_MEM_ASSIGN(*p, NULL);
}
