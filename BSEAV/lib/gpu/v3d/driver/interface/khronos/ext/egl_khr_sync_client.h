/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_common.h"

#include "interface/khronos/common/khrn_client.h"
#include "middleware/khronos/egl/egl_server.h"
#include "interface/vcos/vcos.h"

typedef struct {
   unsigned type;
   int32_t condition;
   int32_t status;

   VCOS_SEMAPHORE_T sem;
} EGL_SYNC_T;