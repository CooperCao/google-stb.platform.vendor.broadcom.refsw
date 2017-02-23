/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __DISPLAY_NEXUS_EXCLUSIVE_H__
#define __DISPLAY_NEXUS_EXCLUSIVE_H__

#include "display_interface.h"
#include "fence_interface.h"
#include "default_nexus.h"

/* An implementation of display interface that uses exclusive mode
 * Nexus display and a provided fence interface do show NXPL_Surface surfaces.
 */
bool DisplayInterface_InitNexusExclusive(DisplayInterface *di,
      const FenceInterface *fi,
      const NXPL_NativeWindowInfoEXT *windowInfo, NXPL_DisplayType displayType,
      NEXUS_DISPLAYHANDLE display, int *bound);

#endif /* __DISPLAY_NEXUS_EXCLUSIVE_H__ */
