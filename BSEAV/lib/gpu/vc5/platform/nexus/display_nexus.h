/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#ifndef __DISPLAY_NEXUS_H__
#define __DISPLAY_NEXUS_H__

#include "default_nexus.h"
#include "sched_abstract.h"
#include "berr.h"
#include "bkni.h"

#include "../common/sched_nexus.h"  // For EventContext

#ifdef __cplusplus
extern "C" {
#endif

/* NXPL_DisplayContext */
typedef struct
{
   BEGL_SchedInterface    *schedIface;
   EventContext           *eventContext;

#ifdef NXPL_PLATFORM_EXCLUSIVE
   NEXUS_DISPLAYHANDLE  display;
#endif
} NXPL_DisplayContext;

struct BEGL_DisplayInterface;
struct BEGL_SchedInterface;

struct BEGL_DisplayInterface *CreateDisplayInterface(
      NEXUS_DISPLAYHANDLE display,
      NXPL_DisplayContext *ctx,
      struct BEGL_SchedInterface *schedIface,
      EventContext *eventContext);

void DestroyDisplayInterface(struct BEGL_DisplayInterface *disp);

#ifdef __cplusplus
}
#endif

#endif
