/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __DISPLAY_NEXUS_MULTI_H__
#define __DISPLAY_NEXUS_MULTI_H__

#include "display_interface.h"
#include "fence_interface.h"
#include "default_nexus.h"
#include "nexus_surface_client.h"

/* An implementation of display interface that uses multi-client mode
 * Nexus display and a provided fence interface do show NXPL_Surface surfaces.
 */
bool DisplayInterface_InitNexusMulti(DisplayInterface *di,
      const FenceInterface *fi,
      const NXPL_NativeWindowInfoEXT *windowInfo, NXPL_DisplayType displayType,
      unsigned int numSurfaces, uint32_t clientID,
      NEXUS_SurfaceClientHandle surfaceClient);


#endif /* __DISPLAY_NEXUS_MULTI_H__ */
