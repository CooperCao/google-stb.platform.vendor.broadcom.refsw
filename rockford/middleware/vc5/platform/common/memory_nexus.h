/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __MEMORY_NEXUS_H__
#define __MEMORY_NEXUS_H__

#ifdef __cplusplus
extern "C" {
#endif

BEGL_MemoryInterface *CreateMemoryInterface(void);
void DestroyMemoryInterface(BEGL_MemoryInterface *iface);

#ifdef __cplusplus
}
#endif

#endif
