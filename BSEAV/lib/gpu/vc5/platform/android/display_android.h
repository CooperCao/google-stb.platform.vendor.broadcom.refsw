/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
