/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __DISPLAY_DIRECTFB_H__
#define __DISPLAY_DIRECTFB_H__

#include "default_directfb.h"
#include "sched_abstract.h"

#ifdef __cplusplus
extern "C" {
#endif

struct BEGL_DisplayInterface;
struct BEGL_SchedInterface;

struct BEGL_DisplayInterface *CreateDirectFBDisplayInterface(IDirectFB *dfb,
                                                   BEGL_MemoryInterface *memIface);
void DestroyDirectFBDisplayInterface(struct BEGL_DisplayInterface *disp);

#ifdef __cplusplus
}
#endif

#endif //__DISPLAY_DIRECTFB_H__
