/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_image.h"
#include "middleware/khronos/common/2708/khrn_fmem_4.h"
#include "middleware/khronos/ext/egl_khr_image.h"
#include "middleware/khronos/glxx/glxx_server.h"

extern bool khrn_rso_to_tf_convert(GLXX_SERVER_STATE_T *state, KHRN_IMAGE_T *src, KHRN_IMAGE_T *dst);
extern void khrn_tfconvert_done(BEGL_HWCallbackRecord *cbRecord);
