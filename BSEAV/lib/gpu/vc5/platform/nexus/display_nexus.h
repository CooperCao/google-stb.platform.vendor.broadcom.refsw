/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
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

/* NXPL_DisplayContext */
typedef struct
{
   NXPL_DisplayType     displayType;
   BEGL_SchedInterface *schedIface;

#ifdef NXPL_PLATFORM_EXCLUSIVE
   NEXUS_DISPLAYHANDLE  display;
   bool                 stretch;
#endif
} NXPL_DisplayContext;

struct BEGL_DisplayInterface;
struct BEGL_SchedInterface;

struct BEGL_DisplayInterface *CreateDisplayInterface(
      NEXUS_DISPLAYHANDLE display,
      NXPL_DisplayContext *ctx,
      struct BEGL_SchedInterface *schedIface);
void DestroyDisplayInterface(struct BEGL_DisplayInterface *disp);

#ifdef __cplusplus
}
#endif

#endif
