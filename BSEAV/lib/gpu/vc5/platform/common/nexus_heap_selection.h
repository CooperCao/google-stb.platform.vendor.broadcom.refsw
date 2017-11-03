/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include "nexus_platform.h"
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline NEXUS_HeapHandle GetDefaultHeap(void)
{
#ifdef SINGLE_PROCESS
   /* With refsw NEXUS_OFFSCREEN_SURFACE is the only heap guaranteed to be valid for v3d to use */
   return NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);
#else
   NEXUS_ClientConfiguration clientConfig;
   NEXUS_Platform_GetClientConfiguration(&clientConfig);

   /* With refsw NEXUS_OFFSCREEN_SURFACE is the only heap guaranteed to be valid for v3d to use */
   if (clientConfig.mode == NEXUS_ClientMode_eUntrusted)
      return clientConfig.heap[NXCLIENT_DEFAULT_HEAP];
   else
#ifdef NXCLIENT_SUPPORT
      return NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);
#else
      /* NEXUS_Platform_GetFramebufferHeap() is not callable under NEXUS_CLIENT_SUPPORT=y */
   return NULL;
#endif
#endif
}

static inline NEXUS_HeapHandle GetDynamicHeap(void)
{
#ifdef SINGLE_PROCESS
   return NULL;
#else
   NEXUS_ClientConfiguration clientConfig;
   NEXUS_Platform_GetClientConfiguration(&clientConfig);
   return clientConfig.heap[NXCLIENT_DYNAMIC_HEAP];
#endif
}

static inline NEXUS_HeapHandle GetSecureHeap(void)
{
#ifdef SINGLE_PROCESS
   return NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE);
#else
   NEXUS_ClientConfiguration clientConfig;
   NEXUS_Platform_GetClientConfiguration(&clientConfig);
   return clientConfig.heap[NXCLIENT_SECURE_GRAPHICS_HEAP];
#endif
}

static inline NEXUS_HeapHandle GetDisplayHeap(unsigned int display)
{
#ifdef SINGLE_PROCESS
   /* Framebuffer heap 0 is always GFD accessible */
   return NEXUS_Platform_GetFramebufferHeap(display);
#else
   /* previously a default heap was used in multi-process mode */
   BSTD_UNUSED(display);
   return GetDefaultHeap();
#endif
}

#ifdef __cplusplus
}
#endif
