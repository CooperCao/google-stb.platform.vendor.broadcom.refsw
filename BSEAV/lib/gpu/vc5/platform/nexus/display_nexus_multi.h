/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#ifndef __DISPLAY_NEXUS_MULTI_H__
#define __DISPLAY_NEXUS_MULTI_H__

#include "display_interface.h"
#include "fence_interface.h"
#include "default_nexus.h"
#include "nexus_surface_client.h"
#include "private_nexus.h"
#include "../common/perf_event.h"

/* An implementation of display interface that uses multi-client mode
 * Nexus display and a provided fence interface do show NXPL_Surface surfaces.
 */
bool DisplayInterface_InitNexusMulti(DisplayInterface *di,
      const FenceInterface *fi,
      unsigned int numSurfaces, NXPL_NativeWindow *nw,
      EventContext *eventContext);

#endif /* __DISPLAY_NEXUS_MULTI_H__ */
