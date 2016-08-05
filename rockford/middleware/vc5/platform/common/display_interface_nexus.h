/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __DISPLAY_INTERFACE_NEXUS_H__
#define __DISPLAY_INTERFACE_NEXUS_H__

#include "display_interface.h"
#include "../nexus/display_nexus_priv.h" /* NXPL_Display */
#include "fence_interface.h"

/* An implementation of display interface that uses provided NXPL_Display
 * and fence interface do show NXPL_Surface surfaces.
 */

bool DisplayInterface_InitNexus(struct DisplayInterface *di,
      NXPL_Display *display, const struct FenceInterface *fi,
      unsigned int numSurfaces);

#endif /* __DISPLAY_INTERFACE_NEXUS_H__ */
