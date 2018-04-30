/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "default_directfb.h"
#include "sched_abstract.h"

#ifdef __cplusplus
extern "C" {
#endif

struct BEGL_DisplayInterface;
struct BEGL_SchedInterface;

struct BEGL_DisplayInterface *CreateDirectFBDisplayInterface(IDirectFB *dfb,
                                                            BEGL_SchedInterface *schedIface);
void DestroyDirectFBDisplayInterface(struct BEGL_DisplayInterface *disp);

#ifdef __cplusplus
}
#endif
