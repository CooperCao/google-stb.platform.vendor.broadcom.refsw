/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
