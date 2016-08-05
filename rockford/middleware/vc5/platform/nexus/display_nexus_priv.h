/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __DISPLAY_NEXUS_PRIV_H__
#define __DISPLAY_NEXUS_PRIV_H__

#include "default_nexus.h"
#include "private_nexus.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NXPL_PLATFORM_EXCLUSIVE
bool InitializeDisplayExclusive(NXPL_Display *priv,
      NXPL_DisplayType displayType, NEXUS_DISPLAYHANDLE display, int *bound,
      const NXPL_NativeWindowInfoEXT *windowInfo);
#else
bool InitializeDisplayMulti(NXPL_Display *priv,
      NXPL_DisplayType displayType,  unsigned int numSurfaces,
      uint32_t clientID,
      NEXUS_SurfaceClientHandle surfaceClient,
      const NXPL_NativeWindowInfoEXT *windowInfo);
#endif

void TerminateDisplay(NXPL_Display *priv);

void SetDisplayComposition(NXPL_Display *priv,
      const NXPL_NativeWindowInfoEXT *windowInfo);

void SetDisplayFinishedCallback(NXPL_Display *priv,
      NXPL_SurfaceOffDisplay callback, void *context);

bool DisplaySurface(NXPL_Display *priv, NEXUS_SurfaceHandle surface);

void WaitDisplaySync(NXPL_Display *priv);

#ifdef __cplusplus
}
#endif

#endif
