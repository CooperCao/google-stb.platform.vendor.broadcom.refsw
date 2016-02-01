/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
Platform specific header for BCG's abstract platform support
=============================================================================*/

#ifndef __EGL_PLATFORM_ABSTRACT_H__
#define __EGL_PLATFORM_ABSTRACT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "interface/khronos/include/EGL/begl_displayplatform.h"

typedef struct bcg_platform_data
{
   BEGL_DisplayInterface displayInterface;
} BCG_PLATFORM_DATA_T;

extern BCG_PLATFORM_DATA_T  g_bcgPlatformData;

#ifdef __cplusplus
}
#endif

#endif /* __EGL_PLATFORM_ABSTRACT_H__ */
