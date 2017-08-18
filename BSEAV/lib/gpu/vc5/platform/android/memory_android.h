/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __MEMORY_ANDROID_H__
#define __MEMORY_ANDROID_H__

#ifdef __cplusplus
extern "C" {
#endif

BEGL_MemoryInterface *CreateAndroidMemoryInterface(void);
void DestroyAndroidMemoryInterface(BEGL_MemoryInterface *iface);

#ifdef __cplusplus
}
#endif

#endif
