/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#pragma once

#ifdef USE_DRM_MEMORY
BEGL_MemoryInterface *CreateDRMMemoryInterface(void);
void DestroyDRMMemoryInterface(BEGL_MemoryInterface *iface);
#else
static inline BEGL_MemoryInterface *CreateDRMMemoryInterface(void) { return NULL; }
static inline void DestroyDRMMemoryInterface(BEGL_MemoryInterface *iface) {}
#endif
