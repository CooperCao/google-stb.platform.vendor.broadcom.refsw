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

#ifndef __DISPLAY_NEXUS_H__
#define __DISPLAY_NEXUS_H__

#include "default_nexus.h"
#include "sched_abstract.h"

#ifdef __cplusplus
extern "C" {
#endif

struct BEGL_DisplayInterface;
struct BEGL_SchedInterface;

struct BEGL_DisplayInterface *CreateDisplayInterface(NEXUS_DISPLAYHANDLE display, struct BEGL_SchedInterface *schedIface);
void DestroyDisplayInterface(struct BEGL_DisplayInterface *disp);

#ifdef __cplusplus
}
#endif

#endif
