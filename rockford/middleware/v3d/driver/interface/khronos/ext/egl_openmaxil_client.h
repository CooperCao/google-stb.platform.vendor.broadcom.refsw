/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Client-side function for notifying an OpenMAX IL component that an
EGLImage is available for rendering into.

=============================================================================*/

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"

extern int eglIntOpenMAXILDoneMarker (void* component_handle, EGLImageKHR egl_image);
