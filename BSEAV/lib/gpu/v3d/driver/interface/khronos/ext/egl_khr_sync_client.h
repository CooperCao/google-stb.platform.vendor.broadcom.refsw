/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_common.h"

#include "interface/khronos/common/khrn_client.h"
#include "middleware/khronos/egl/egl_server.h"

typedef struct {
   EGLenum type;

   EGL_SYNC_ID_T serversync;
} EGL_SYNC_T;

void egl_sync_term(EGL_SYNC_T *sync);