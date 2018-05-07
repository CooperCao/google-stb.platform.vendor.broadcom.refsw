/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <directfb_version.h>
#include <directfb.h>

#include <EGL/egl.h>
#include <EGL/begl_platform.h>

#ifdef __cplusplus
extern "C" {
#endif

bool DfbToBeglFormat(DFBSurfacePixelFormat dfb_format, BEGL_BufferFormat *result);
bool BeglToDfbFormat(BEGL_BufferFormat format, DFBSurfacePixelFormat *dfb_format);

#ifdef __cplusplus
}
#endif
