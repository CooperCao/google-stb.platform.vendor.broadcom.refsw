/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#ifndef WLPL_EXPORT
#define WLPL_EXPORT __attribute__((visibility("default")))
#endif

#ifdef SINGLE_PROCESS
#error Wayland requires multi-process support
#endif

#include "nexus_graphics2d.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WLPL_NexusWindowInfoEXT
{
   uint32_t width;
   uint32_t height;
   uint32_t x;
   uint32_t y;
   bool stretch;
   uint32_t clientID;
   uint32_t zOrder;
   NEXUS_BlendEquation colorBlend;
   NEXUS_BlendEquation alphaBlend;
   uint32_t magic;
} WLPL_NexusWindowInfoEXT;

/* Generate a default WLPL_NexusWindowInfoEXT */
WLPL_EXPORT void WLPL_GetDefaultNexusWindowInfoEXT(
      WLPL_NexusWindowInfoEXT *info);

/* Create a 'native window' of the given size */
WLPL_EXPORT void *WLPL_CreateNexusWindowEXT(
      const WLPL_NexusWindowInfoEXT *info);

/* Destroy a 'native window' */
WLPL_EXPORT void WLPL_DestroyNexusWindow(void *nativeWin);

#ifdef __cplusplus
}
#endif
