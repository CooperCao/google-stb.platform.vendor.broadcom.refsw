/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/common/khrn_int_image.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/common/abstract/khrn_client_platform_abstract.h"
#include "middleware/khronos/common/khrn_image.h"

#include <malloc.h>
#include <assert.h>

BEGL_BufferHandle BEGLint_PixmapCreateCompatiblePixmap(const BEGL_PixmapInfoEXT *pixmapInfo)
{
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();

   return driverInterfaces->displayInterface->BufferCreate(driverInterfaces->displayInterface->context, pixmapInfo);
}
