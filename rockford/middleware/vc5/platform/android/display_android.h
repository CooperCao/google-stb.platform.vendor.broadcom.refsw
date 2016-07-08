/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  Default Android platform API for EGL driver
Module   :  Android platform

FILE DESCRIPTION
=============================================================================*/

#ifndef __DISPLAY_ANDROID_H__
#define __DISPLAY_ANDROID_H__

#include "default_android.h"
#include "sched_abstract.h"

#ifdef __cplusplus
extern "C" {
#endif

struct BEGL_DisplayInterface;
struct BEGL_SchedInterface;

struct BEGL_DisplayInterface *CreateAndroidDisplayInterface(struct BEGL_SchedInterface *schedIface);
void DestroyAndroidDisplayInterface(struct BEGL_DisplayInterface *disp);

#ifdef __cplusplus
}
#endif

#endif
