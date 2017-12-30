/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/v3d/v3d_ver.h"

#define GLXX_HAS_COMPUTE      V3D_VER_AT_LEAST(3,3,0,0)
#define GLXX_HAS_TNG (KHRN_GLES32_DRIVER || V3D_VER_AT_LEAST(4,0,2,0))
