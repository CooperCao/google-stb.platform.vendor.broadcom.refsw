/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
This is a default implementation of a Nexus platform layer used by V3D.
This illustrates one way in which the abstract display interface might be
implemented. You can replace this with your own custom version if preferred.
=============================================================================*/

#ifndef __DISPLAY_DIRECTFB_H__
#define __DISPLAY_DIRECTFB_H__

#include "default_directfb.h"
#include "sched_abstract.h"

#ifdef __cplusplus
extern "C" {
#endif

struct BEGL_DisplayInterface;
struct BEGL_SchedInterface;

struct BEGL_DisplayInterface *CreateDisplayInterface(IDirectFB *dfb,
                                                   BEGL_MemoryInterface *memIface);
void DestroyDisplayInterface(struct BEGL_DisplayInterface *disp);

#ifdef __cplusplus
}
#endif

#endif //__DISPLAY_DIRECTFB_H__
