/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

BEGL_MemoryInterface *CreateDRMMemoryInterface(void);
void DestroyDRMMemoryInterface(BEGL_MemoryInterface *iface);

#ifdef __cplusplus
}
#endif
