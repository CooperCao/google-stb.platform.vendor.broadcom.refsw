/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef V3D_PLATFORM_H
#define V3D_PLATFORM_H
#include "vcos.h"
#include <stdbool.h>
#include "libs/core/v3d/v3d_common.h"
#include "libs/core/v3d/v3d_debug.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize the v3d platform, including the scheduler session
 * (we only use one per process). Return true for success.
 */
extern bool v3d_platform_init(void);
extern void v3d_platform_shutdown(void);

extern bool v3d_platform_explicit_sync(void);

/* Debug functions. These may be implemented as no-ops! */
extern void v3d_platform_set_debug_callback(v3d_debug_callback_t callback, void *p);

#ifdef __cplusplus
}
#endif

#endif /* V3D_PLATFORM_H */
